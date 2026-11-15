# Project Plan: Web Edition PWA Installability And Offline Hardening

Status: Proposed; implementation has not started.
Date: 2026-11-14

## Objective

Make the Web Edition reliably installable as a PWA (home screen, desktop
install) from any deployment path, keep offline use and updates predictable,
and make claims about persistence and installation in the docs verifiable.

Source: the 2026-11-14 PWA audit. The live GitHub Pages deployment at
`/ucblogo-code/` is installable today; these items remove deployment-path
fragility and polish the install, offline, update, and storage behavior.

Companion plans: [Supported browser and Node.js CI](browser-node-ci.md) (PWA
checks can become a scheduled job there) and
[Web accessibility](web-accessibility.md) (new UI below must meet its
conventions).

## Constraints

- Preserve installed-app identity. Existing installs use the id
  `https://n1ywb-1.github.io/ucblogo-code/ucblogo.html`. Any manifest change
  must resolve to the same id and scope on that origin.
- Never auto-activate a new service worker (SW) while a session is running;
  it could reload the page and lose in-memory Logo definitions.
- Do not weaken the stale-artifact rules in
  [copilot-instructions.md](../../.github/copilot-instructions.md#artifact-freshness):
  local test runs stay SW-free unless a test explicitly opts in.
- Generated outputs (`ucblogo.html`, `sw.js`, `web-config.js`) are fixed in
  their sources ([shell.html](../../shell.html), [Makefile.am](../../Makefile.am),
  [workbox-config.js](../../workbox-config.js)), not by hand.
- Publishing remains a manual maintainer action; this plan adds staging and
  verification only.
- Desktop Safari and real iOS are not verifiable locally. Report that gap;
  iPad checks are a human checkpoint per
  [tests/acceptance/README.md](../../tests/acceptance/README.md).

## Issue 1: Hard-coded `/ucblogo-code/` paths

Problem: [manifest.json](../../manifest.json) `start_url` and
[workbox-config.js](../../workbox-config.js) `navigateFallback` and
`modifyURLPrefix` embed `/ucblogo-code/`. Any other host path makes precache
URLs 404, SW install fails, and offline and install break.

Plan:
1. Set manifest `start_url` to `./ucblogo.html` and add `scope` `./`. Resolved
   against the manifest URL, `start_url` equals the current absolute URL, so
   identity is unchanged on Pages.
2. Do not add an explicit `id` (see Issue 4); the default derives from
   `start_url`.
3. In workbox-config.js, remove `modifyURLPrefix` and use a relative
   `navigateFallback` (`ucblogo.html`). Workbox resolves precache and
   `createHandlerBoundToURL` URLs against the SW location; confirm that in the
   generated `sw.js` and in the browser.
4. Regenerate `sw.js` and verify entries are relative.

Validation: the PWA test (Issue 11) serves the build under two prefixes (root
and a nested path) and checks that the manifest resolves, the SW installs, and
the app loads offline in both. A source test asserts no absolute
`/ucblogo-code/` remains in the manifest or Workbox config.

Risk: low. If Workbox cannot resolve the relative fallback key, use a
build-time `navigateFallback` derived from the precache manifest instead.

## Issue 2: Service worker never registers on localhost

Problem: [shell.html](../../shell.html#L915-L927) skips registration on
`localhost`, `127.0.0.1`, and `[::1]`, so the install and offline flows cannot
be tested locally. The skip protects local diagnosis from stale caches.

Plan:
1. Keep the default skip on loopback hosts.
2. Add an explicit opt-in: registration on loopback when the URL has
   `?serviceworker=1`. Without it, behavior is unchanged, so existing
   TestCafe runs stay SW-free.
3. Keep the `Module.serviceWorker === false` path that unregisters existing
   workers.
4. Update the registration test in
   [tests/acceptance/source.test.js](../../tests/acceptance/source.test.js)
   with loopback opt-in and opt-out cases.
5. Document the flag in [webedition.md](../../webedition.md) next to the
   stale-artifact guidance.

Validation: source test plus the PWA test, which uses the opt-in.

## Issue 3: Icons lack `type`/`purpose`; no maskable icon

Problem: the 512px artwork puts "UCB Logo" near the bottom edge, so adaptive or
circular masks crop it. Icons declare no `type` or `purpose`.

Plan:
1. Add `"type": "image/png"` and `"purpose": "any"` to the existing 192, 256,
   and 512 icons. Do not mark them maskable.
2. Create `ucblogo-maskable-512.png`: the artwork scaled into the central 80%
   safe zone on an opaque background taken from the logo. Commit the PNG; no
   generator infrastructure.
3. Add it with `"purpose": "maskable"`, as a separate entry (do not combine
   `any maskable`).
4. Add `manifest.json`, `ucblogo-192.png`, `ucblogo-512.png`, and the new icons
   to `EXTRA_DIST` in [Makefile.am](../../Makefile.am); `make dist` currently
   omits them.

Validation: Chrome DevTools Application > Manifest shows no icon warnings and
the maskable preview keeps the text visible. Source test checks each listed
icon exists with its declared dimensions.

## Issue 4: Missing `theme_color`, `background_color`, `id`, `screenshots`

Plan:
1. Add `background_color` and `theme_color` matching `--theme-background` in
   [shell.html](../../shell.html). A manifest holds one value, so use the light
   value there and add a pair of `<meta name="theme-color" media="(prefers-color-scheme: ...)">`
   tags for light and dark title bars. Accept a possible light splash in dark
   mode; document it as a platform limit.
2. `id`: omit by default (see Constraints). If an explicit `id` is later
   wanted, it must equal the current absolute id; add a source or PWA-test
   assertion that the computed id for the Pages URL is unchanged. Record this
   rationale in a manifest-adjacent note in [webedition.md](../../webedition.md).
3. `screenshots` (optional, last): one wide and one narrow capture produced from
   the existing TestCafe screenshot support, with `form_factor` set. Keep
   them small; they are not precached.

Validation: DevTools Manifest pane; PWA test asserts no `id`/scope drift.

## Issue 5: No `apple-touch-icon` or iOS metadata

Plan:
1. Add an opaque 180x180 `ucblogo-180.png` (iOS fills transparency with
   black).
2. Add `<link rel="apple-touch-icon" href="ucblogo-180.png">` and
   `<meta name="apple-mobile-web-app-title" content="UCB Logo">` to
   [shell.html](../../shell.html).
3. Add the icon to the precache list (Issue 6).

Validation: source test for the tags and file; human checkpoint on the
maintainer's iPad (Add to Home Screen icon and title). Desktop Safari is not
covered.

## Issue 6: Incomplete precache; overly broad runtime caching

Problem: the precache covers only `ucblogo.html`, `.js`, `.wasm`. The manifest
and icons are cached only if fetched through the SW. The catch-all
`NetworkFirst` for `/.*/` also caches every GET, including cross-origin
requests made by user programs via `runjs`/`fetch`, with no size bound and no
timeout.

Plan:
1. Use an explicit file list (no wildcards) in
   [workbox-config.js](../../workbox-config.js): the three build files,
   `manifest.json`, the icons used by the manifest and iOS, and `logologo.ico`.
   Do not use `*.png` (it would include `chromeflagsjspi.png`).
2. Make `sw.js` depend on the same files in [Makefile.am](../../Makefile.am)
   so changes regenerate it.
3. Restrict the runtime route to same-origin GET and add `networkTimeoutSeconds`
   (about 3) so poor networks fall back to cache. Add an expiration plugin
   (`maxEntries`) if the generator supports it without extra build steps;
   otherwise bound by the same-origin restriction alone.
4. Enable `cleanupOutdatedCaches: true` so older precaches from previous
   Workbox runtimes are removed.

Validation: PWA test inspects the cache storage list, goes offline, reloads the
installed start URL, and checks the manifest and icons are served. It also
checks a cross-origin fetch from the page is not stored.

## Issue 7: No service-worker update path

Problem: no `skipWaiting`/`clientsClaim` and nothing sends the existing
`SKIP_WAITING` message, so a long-lived installed window can stay on an old
build indefinitely.

Plan:
1. After registration, call `registration.update()` on `visibilitychange`
   (when the document becomes visible) and at a modest interval for long-lived
   windows.
2. When a worker reaches `waiting` (via `registration.waiting` and
   `updatefound`/`statechange`), show a non-modal status notice with an
   "Update" action, following the status and dialog conventions in
   [web-accessibility.md](web-accessibility.md) (`role="status"`, native button,
   focus not stolen).
3. On user action post `{type: 'SKIP_WAITING'}` and reload after
   `controllerchange`. The existing `beforeunload` prompt still guards unsaved
   work; no automatic activation.
4. Keep `clientsClaim` off to avoid a mid-session controller swap.

Validation: source test with a fake registration for the state machine; PWA
test serves build A, then build B, and asserts the notice appears, the old
page is not reloaded without action, and the new version loads after the
action. Wait for real readiness before asserting.

## Issue 8: Persistence claim is unverified

Problem: [webedition.md](../../webedition.md#L271) says installing "ensures"
persistence, but nothing calls `navigator.storage.persist()`.

Plan:
1. Find the existing IDBFS ready hook in [pre.js](../../pre.js) and, after it,
   record `navigator.storage.persisted()` and `estimate()` for display.
2. Call `persist()` only when the app runs standalone (Issue 10) or after a
   user-initiated save. Firefox prompts for it; never prompt at first load.
3. Surface the result (persistent or best-effort, usage versus quota) in the
   existing help or settings UI. Do not add a new dialog if one fits.
4. Reword the docs: Chrome and Edge grant persistence by heuristic, including
   for installed apps; Firefox asks; Safari evicts tab data after 7 days
   without use but exempts Home Screen web apps. State that backups through
   "Download Logo Session" remain recommended.

Validation: browser test with `navigator.storage` stubbed asserts no `persist()`
call before standalone or a save, exactly one call afterward, and correct
status text. Real eviction behavior is not testable here; say so.

## Issue 9: Deploy copy step over-publishes and includes stale files

Problem: the `cp {*.js,*.wasm,...}` step in
[webedition.md](../../webedition.md#L831) also publishes `webserver.js`,
`pre.js`, `liblogo.js`, `package.json`, `chromeflagsjspi.png`, and every
`workbox-*.js` including stale ones. Old Workbox runtimes (`workbox-84839ebf.js`,
`workbox-915e8d08.js`, `workbox-c89c8001.js`) are tracked in git, while the
generated runtime for the current `sw.js` is untracked.

Plan:
1. Add `scripts/stage-web-deploy.js` (Node, no dependencies) that takes a target
   directory and copies exactly: `ucblogo.html`, `.js`, `.wasm`, their `.map`
   files if present, `sw.js`, `sw.js.map`, the single `workbox-*.js` named in
   `sw.js`, `manifest.json`, every icon the manifest or HTML references, and
   `logologo.ico`. Fail if any is missing.
2. After copying, verify every precache URL in `sw.js` and every manifest icon
   exists in the target, and that `ucblogo.html`, `.js`, and `.wasm` revisions match
   the staged bytes (so the SW cannot be from a different build).
3. Add `npm run stage:web -- <dir>` and replace the manual `cp` in the
   Deployment section.
4. Stop tracking stale `workbox-*.js` runtimes (`git rm --cached`) and ignore
   `workbox-????????.js`, `web-config.js`, and `sw.js.map` in `.gitignore`;
   `workbox-config.js` stays tracked (its name does not match the pattern). Have
   the `sw.js` rule in [Makefile.am](../../Makefile.am) remove older
   `workbox-????????.js` before generating, and add the runtime to `CLEANFILES`.
5. Do not retain previous runtimes in the deploy directory: browsers refetch
   imports when `sw.js` changes.

Validation: run the staging script into a temp dir after a production build;
serve the temp dir and run the PWA test against it. Negative case: delete a
file and confirm the script fails. Removal of tracked files is a repo change
for the maintainer to confirm in review.

## Issue 10: Smaller gaps

Plan:
1. Standalone detection: set `data-display-mode` on `<html>` from
   `matchMedia('(display-mode: standalone)')` and `navigator.standalone`,
   updated on change. Used by Issue 8 and by tests; no visual change.
2. Install affordance: capture `beforeinstallprompt`, expose an "Install app"
   button in the existing help or menu surface only when the event fired and
   the app is not standalone; hide on `appinstalled`. Keyboard and touch
   operable, native button. Where no event exists (Safari, Firefox), show
   help text with the browser's install route instead of a button.
3. `beforeunload`: keep the protection (in-memory definitions are lost on
   close), but remove the duplicate registration (`window.onbeforeunload` and
   `addEventListener` both exist) and document the behavior in the PWA section.
4. Docs: rewrite the PWA section in [webedition.md](../../webedition.md#L314-L320)
   to cover desktop Chrome/Edge install, iOS Share > Add to Home Screen,
   Android, offline scope, the update notice, persistence caveats, and the
   `?serviceworker=1` localhost flag.

Validation: TestCafe cases with `beforeinstallprompt` dispatched synthetically
(labeled as simulated, not a real install); display-mode emulation through the
browser provider where available.

## Issue 11: No automated installability check

Plan:
1. Static checks added to `npm run test:source`: manifest fields and relative
   URLs, icon existence and dimensions, precache config includes the manifest and
   icons, and no hard-coded prefix.
2. `tests/acceptance/pwa.test.js` (Mocha/Chai like the Lighthouse test; uses
   a locally served built tree and the Chrome that Lighthouse already
   depends on): with `?serviceworker=1`, read CDP `Page.getInstallabilityErrors` and
   `Page.getAppManifest`, wait for the SW to activate and control the page, go
   offline and reload, and repeat under a nested prefix. Lighthouse 13 has no PWA
   category, so it is not used for this.
3. Add `npm run test:pwa:built` against current artifacts. Initially manual;
   promote to a scheduled job in the browser CI plan once stable.

## Sequencing

1. Phase A (portability and test harness): Issues 1, 2, 6, 11. This is the
   foundation; the PWA test must exist before later changes.
2. Phase B (identity and icons): Issues 3, 4, 5.
3. Phase C (lifecycle): Issues 7, 8, 10.
4. Phase D (deployment and docs): Issue 9 and the remaining doc updates.

Each phase: rebuild with `npm run build:web`, run `npm run test:source`,
then the focused PWA and browser checks on fresh artifacts (verify the served
files are the just-built ones, with no stale SW).

## Out Of Scope

- Changing the deployment host, adding a publish workflow, or any push.
- Migrating Asyncify or caching strategy for the Wasm beyond the precache above.
- Web Push, background sync, file handlers, and share targets.
- Physical-device coverage beyond the iPad human checkpoint.
