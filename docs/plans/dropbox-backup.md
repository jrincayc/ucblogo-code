# Project Plan: Dropbox Backup And Sync For The Web Edition

Status: Proposed; implementation has not started.
Date: 2026-11-15

## Objective

Let users back up and restore the Web Edition virtual filesystem (`/d`,
IDBFS) to their own Dropbox, so browser storage eviction (notably Safari's
7-day cap on tab data) cannot lose work. Dropbox is the first provider because
it has the simplest static-site auth story. Other providers are investigated in
[backup-provider-investigations.md](backup-provider-investigations.md).

## Constraints

- No backend, ever. The project must not receive, proxy, or store user data or
  credentials (FERPA/COPPA exposure). Data flows only between the user's browser
  and Dropbox. Legal wording is a maintainer and counsel decision.
- Least privilege: Dropbox "App Folder" access only (`Apps/<app name>/`).
- Trust boundary (maintainer decision, 2026-11-15): loaded Logo programs are
  trusted, and avoiding malicious ones is the user's responsibility. Logo and the
  sync code must still resist malicious third-party data (file names, file
  contents, Dropbox responses) so it cannot cause Logo or JavaScript injection;
  that is why `runjs` takes bound parameters instead of interpolated source.
  Rules for this feature:
  - never build Logo or JavaScript source from remote or file-derived strings;
    pass them as bound `runjs` arguments, or do not generate commands at all;
  - render names and messages with `textContent` or text nodes, never `innerHTML`;
  - validate remote JSON shapes and types; reject paths with `.`/`..` segments,
    absolute or backslash segments, NUL, control or bidirectional-override
    characters, and excessive length or depth; cap file size and count;
  - pulled files are inert: nothing is loaded or run automatically.
- Tokens stay out of web storage and out of pre.js scope (the direct `eval` in
  `Module.runjs` can see pre.js top-level variables). Since hostile Logo programs
  are out of scope, the token Worker is defense in depth against injection or
  XSS regressions, not a boundary against Logo programs; keep it unless its cost
  proves disproportionate during Phase 1.
- The Asyncify Logo runtime is non-reentrant. Sync code reads and writes the
  Emscripten `FS` from JS only and never calls C exports.
- Never navigate away from a live session silently; unsaved Logo definitions
  would be lost (see PWA plan Issue 7).
- Generated outputs (`ucblogo.html`, `sw.js`, `web-config.js`) are fixed in
  their sources, not by hand.
- Publishing stays on the tag-triggered Deploy Web workflow; this plan never
  pushes or releases.
- Desktop Safari and real iOS cannot be verified locally; iPad checks are a
  human checkpoint ([tests/acceptance/README.md](../../tests/acceptance/README.md)).

## Verified facts (Dropbox docs, 2026-11-15)

- Code flow with PKCE needs no client secret; Dropbox recommends it for
  pure-JavaScript apps, with short-lived access tokens and no refresh tokens.
  If the user is signed in and has approved the app, re-authorization needs no
  input. The official SDK is MIT and has a browser PKCE example and Worker
  support.
- Code flow without `redirect_uri` displays the code on dropbox.com for the
  user to paste into the app (candidate fallback; confirm with PKCE).
- Redirect URIs are registered exactly in the developer console; "Allow public
  clients" covers PKCE.
- A new app is in development status: your account plus up to 500 enabled
  users. When 50 users have linked, a two-week clock starts to obtain production
  approval or linking freezes. Review is not performed before 50 linked users
  unless early review is requested. Review needs a privacy policy, branding
  compliance, and minimal permissions.
- Terms: users must be 13+ (US) or 16+ elsewhere; free accounts can be
  terminated after 12 months without access; new terms take effect 2027-01-01.

Unverified (settle in Phase 0): single-upload size cap, rate limits and
headers, the `content_hash` algorithm, COOP behavior of Dropbox auth pages,
and CORS preflight behavior from the live origin.

## Code facts this plan depends on

- [pre.js](../../pre.js#L8-L18) mounts IDBFS with `autoPersist` and calls
  `FS.syncfs(true, ...)` whose callback only logs; nothing signals when the
  populate is done.
- Emscripten's `IDBFS.queuePersist` is called, through a property lookup, on
  close of a modified file, `mkdir`, `rmdir`, `unlink`, `rename`, and `symlink`;
  wrapping it yields a precise dirty signal.
- [workbox-config.js](../../workbox-config.js) sets `navigateFallback` to
  `ucblogo.html`, which would answer `dropbox-callback.html?code=...`
  navigations with the Logo app, and its runtime route caches every GET
  (PWA plan Issue 6 restricts it).
- `scripts/stage-web-deploy.js` publishes a fixed file list, and
  `npm run test:source` lists test files explicitly.
- [web-config.js](../../Makefile.am) is generated from a configure
  substitution (`WEB_SERVICE_WORKER`); a Dropbox app key can use the same path.
- There is no Content-Security-Policy anywhere in the repository.

## Related PWA plan items

Reuse [pwa-installability.md](pwa-installability.md): Issue 1 (relative paths,
so the redirect URI works under any prefix), Issue 6 (same-origin runtime
caching), Issue 8 (`storage.persist()` after a user-initiated save), Issue 10
(standalone detection and `beforeunload` cleanup), Issue 11 (PWA test harness).
Issue 6 is a prerequisite or co-change: Dropbox traffic must never be cached.

## Phase 0: Gating spikes

1. Register a development-status Dropbox app (App Folder, "Allow public
   clients") with exact redirect URIs for localhost and the live path.
2. Popup auth on desktop Chrome and Firefox: does Dropbox's
   Cross-Origin-Opener-Policy sever `window.opener`? Return the code over a
   `BroadcastChannel` regardless.
3. iPad Safari in a tab and as a Home Screen app: does the callback share
   storage and channels with the standalone app?
4. Paste-code fallback with PKCE and no `redirect_uri`.
5. CORS preflights to `api.dropboxapi.com` and `content.dropboxapi.com` with
   `Authorization` and `Dropbox-API-Arg` from localhost and the github.io origin.
   Record the upload size cap, `content_hash` algorithm, and rate-limit headers.

Exit criteria: one working auth path per supported browser, or a documented
gap. Spike pages are throwaway and not shipped.

## Phase 1: Implementation

1. **Foundations in pre.js.** Resolve a readiness promise when the IDBFS
   populate finishes (and expose failure). Wrap `IDBFS.queuePersist` to emit a
   dirty event, guarded by a feature check and a periodic size/mtime scan fallback
   so an Emscripten upgrade cannot silently disable syncing.
2. **`backup-sync.js` (pure core, dependency-injected).** Manifest of per-path
   `{localHash, remoteRev, remoteContentHash}` in a separate IndexedDB (not under
   `/d`); three-way planner; conflict copies named `name.conflict-<time>`.
   Safeguards:
   - never sync before the populate promise resolves;
   - confirmation for mass deletes;
   - detect case-insensitive name collisions (Dropbox ignores case) and illegal
     names;
   - skip files modified within a few seconds or flagged dirty;
   - size cap and 429 backoff;
   - single-flight with `navigator.locks`;
   - recovery from eviction: local empty, manifest gone, remote non-empty pulls
     everything and deletes nothing.
   No browser globals at load, so Node can test it. Include it with an extra
   `--pre-js` for the web HTML target only.
3. **`backup-worker.js`, the only code that touches tokens.** Generates PKCE,
   receives the code over a `BroadcastChannel`, exchanges it, and keeps the
   access token in memory only. No refresh token (Dropbox guidance); on expiry
   the UI shows a "Reconnect" button because popups need a user gesture. Narrow
   RPC: connect, disconnect, list, get, put (with `rev`), delete. An injection bug
   elsewhere in the page still cannot read the token, and the token reaches only
   the App Folder. Stage and precache the worker file.
4. **`dropbox-callback.html`.** A flat, dependency-free page that forwards
   `location.search` over the channel and closes itself. Add it to the staging
   script's required list and its test, `EXTRA_DIST`, and a
   `navigateFallbackDenylist` in workbox-config. Do not precache it.
5. **UI in [shell.html](../../shell.html).** A `Backup` button next to
   Settings and a `dialogWithTitle` dialog with connect/disconnect, "Sync now",
   status in a `role="status"` live region, last-sync time, a conflicts list, the
   persistence status line (PWA Issue 8), the 13+/16+ note, and a privacy link.
   Follow the frontend skill: native labels, focus entry and restore, touch
   targets, theme-aware borders.
6. **Auth flow.** Call `window.open` synchronously in the click handler and set
   its URL after the async PKCE work (Safari drops the gesture otherwise). Choose
   popup, full-page redirect, or paste-code from standalone detection and the
   Phase 0 results.
7. **Triggers.** Sync on connect, debounced after a dirty event, on
   `visibilitychange`, on `online`, and on "Sync now". Call `storage.persist()`
   after the first successful connect.
8. **Config.** `--with-dropbox-app-key` in [configure.ac](../../configure.ac),
   emitted into `web-config.js`. With no key the Dropbox option is hidden so forks
   still work. Derive the redirect URI from
   `new URL('dropbox-callback.html', location.href)`; each deployment path must
   be registered.

## Phase 2: Tests (failing tests first)

- **Source (`node --test`, added to `test:source`):** the three-way matrix (new
  local, new remote, both changed, deleted either side), empty-local guard, mass
  delete guard, eviction recovery, case collisions, backoff, expired token; the
  RFC 7636 PKCE vector; static checks for no CDN script, no hard-coded token or
  secret, callback staged and denylisted. Hostile-data cases: remote and local
  names containing `..`, `/`, backslash, NUL, newlines, `]`, `|`, quotes,
  `<img onerror=...>`, bidirectional overrides, and overlong or deeply nested
  paths; malformed or wrongly typed Dropbox JSON. Assert nothing reaches
  `runjs`, `eval`, or `innerHTML`, and add static checks that the backup code
  builds no Logo or JavaScript source from such strings.
- **TestCafe fixture `backup-sync.testcafe.js`:** replace `window.Worker` with an
  in-page fake Dropbox (pattern of `installMocks` in `bridge.testcafe.js`).
  Assert accessible names and focus behavior, the connect flow with a stubbed
  popup, one `persist()` call, no token in any storage, a file saved by Logo
  reaching the fake remote, and a remote file appearing locally and loading.
  Add a remote file with a hostile name and assert it appears as inert text and
  nothing executes. Update `tests/acceptance/COVERAGE.md`.
- **Manual against real Dropbox**, in platform-matrix order: desktop Chrome,
  mobile Chrome emulation, Firefox, then the iPad in a tab and as a Home Screen
  app. Use a throwaway profile; never clear the maintainer's real site data.
  Report emulation as emulation and the Safari gaps as gaps.

## Phase 3: Docs and production approval

- Backup section in [webedition.md](../../webedition.md); reword the persistence
  claims per PWA Issue 8.
- Privacy policy stating nothing is sent to the project, hosted at a stable URL.
- App icon, branding-compliant naming, and the production application prepared
  before the 50-linked-user clock starts.

## Related hardening (existing code)

`Module.uploadFileButtonClicked` in [pre.js](../../pre.js) builds
``load "|<file name>|`` by string interpolation: `replace('|', '\\|')` replaces
only the first pipe, and `runLogo`'s echo wraps the text in `pr [...]`, so a `]`
or extra `|` in a file name appears able to break out and inject Logo. File names
are untrusted data, so fix this as a separate small change before the sync code
reuses the pattern (escape fully or use bound arguments), with a regression test
using hostile names. Verify the exploit with a test first.

## Decisions and assumptions

- Decided: one active provider at a time; switching requires disconnect, and
  manifests are keyed by provider id.
- Bidirectional sync with deletions propagated but guarded (alternative:
  add/update only).
- Hand-written Dropbox client instead of the SDK (no CDN, small footprint).
- A new Backup dialog rather than a Settings section; if chosen, amend the PWA
  plan's "no new dialog" note in Issue 8.

## Out of scope

- Other providers (see the investigations plan).
- Logo words for sync control, background sync, and multi-tab IDBFS divergence
  (a pre-existing hazard).
