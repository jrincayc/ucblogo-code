# Project Plan: Web Edition Storage Persistence And Eviction Awareness

Status: Implemented; browser and device verification remains pending.
Date: 2026-11-15

## Objective

Make the Web Edition's claims about saved-file durability accurate and
verifiable. Request persistent storage at an appropriate time, show users the
real storage state, surface write failures instead of logging them, and make
backup guidance say exactly what each backup path covers.

This plan was split out of Issue 8 of
[pwa-installability.md](pwa-installability.md). The implementation now calls
the Storage API after a successful IDBFS save or an explicit user request, and
shows its reported state in Settings. Browser-specific behavior still requires
the manual checks listed in Phase 0.

## Implementation Status

- IDBFS readiness, save, completion, failure, and unavailable-signal states
   are implemented in `pre.js`.
- The injected, browser-global-free status and request policy is implemented in
   `storage-status.js` and covered by Node source tests.
- Settings UI, failure notices, request cooldown, standalone/install retries,
   and corrected backup guidance are implemented.
- The active Emscripten 4.0.5 source confirms `IDBFS.queuePersist` calls
   `IDBFS.syncfs(mount, false, callback)`, while its completion callback ignores
   the error argument. Wrapping `IDBFS.syncfs` is therefore required to expose
   write failures.
- Desktop Chrome 152 passed the focused Logo `save` and Settings test against
   fresh artifacts. The runner serves `http://127.0.0.1`, which reported
   `isSecureContext === false` and no `navigator.storage`; this verified the
   unknown-API UI path, not browser persistence grants or estimates.
- Real Chrome profile variations, Firefox prompt behavior, Safari behavior,
   real eviction, and the throwaway-profile download check remain unverified.

Related plans: [Dropbox backup](dropbox-backup.md) and
[Local folder backup](local-folder-backup.md) reuse the IDBFS signals and the
status text defined here; [Web accessibility](web-accessibility.md) conventions
apply to the new UI.

## Constraints

- Never prompt at first load. Request persistence only after the user has
  saved something, or on an explicit button press.
- Do not weaken the stale-artifact rules or change service-worker behavior;
  this plan adds no caching.
- Do not report a guarantee the browser has not given. The status distinguishes
  "persistent" (granted), "best-effort" (not granted), and "unknown" (API
  absent or errored).
- Preserve existing user data. No migration, rename, or clearing of `/d`,
  `localStorage`, or IndexedDB.
- Real eviction (storage pressure, Safari's inactivity cap) cannot be triggered
  in a test. Reports say so and never claim eviction behavior was verified.
- Desktop Safari and real iOS are not verifiable locally; the iPad checks are a
  human checkpoint per [tests/acceptance/README.md](../../tests/acceptance/README.md).

## Code facts this plan depends on

- [pre.js](../../pre.js) (`Module.onRuntimeInitialized`) mounts IDBFS at `/d`
   with `autoPersist: true` and now exposes populate readiness and write status.
- The active SDK's `IDBFS.queuePersist` runs on close of modified files and
   filesystem mutations. It calls `IDBFS.syncfs` with a completion callback,
   but that callback discards errors; `pre.js` wraps the lower-level completion
   callback to capture them.
- `Module.downloadSessionClicked` in pre.js runs Logo `save` to
  `/tmp/ucblogo_session.lgo` and downloads that file. That serializes the
  in-memory workspace; it does not export the files stored under `/d`.
- `Module.uploadFileButtonClicked` imports files; there is no per-file or
  whole-folder download.
- Settings persist in `localStorage` under `ucblogo.settings`; settings are
  normalized on read, so persistence bookkeeping uses a separate key.
- [shell.html](../../shell.html) has a Settings dialog (`id="settings"`) and
  status live regions; PWA Issue 10 adds standalone detection
  (`data-display-mode`, `appinstalled`).

## Browser behavior

Documented (web.dev, "Persistent storage", checked 2026-11-15):

- `persist()` and `persisted()` are in Chrome 55+, Edge 79+, Firefox 57+, and
  Safari 15.2+.
- Chromium shows no prompt. It grants or silently denies by heuristics: site
  engagement, installed or bookmarked, notification permission. A denied request
  can be retried and is re-evaluated.
- Firefox prompts the user.
- Granted persistence protects IndexedDB, Cache API, `localStorage`, service
  workers, and OPFS from eviction under storage pressure. Only the user can
  remove it; there is no API to relinquish it.
- Guidance: request when the user saves critical data, ideally inside a user
  gesture; do not retry immediately after a denial.

Unverified (settle in Phase 0):

- Safari's `persist()` result in a tab versus a Home Screen web app, and the
  7-day inactivity cap for tab data and exemption for Home Screen web apps. The
  webedition.md text already asserts these; do not repeat them as fact until
  the cited source is recorded in the doc.
- Whether Firefox shows its prompt when `persist()` is called outside a user
  gesture.
- Whether `persisted()` stays true across Chrome restarts for an installed app
  and for a bookmarked tab.

## Issue 1: No IDBFS ready, saved, or error signals

Problem: the populate callback only logs, so nothing can tell when `/d` is
loaded, that loading failed (for example IndexedDB blocked in a private window),
or that a background write failed (for example `QuotaExceededError`).

Status: Implemented. Node source tests cover populate success/failure, queued
saves, sync success/failure, and missing hooks. Desktop Chrome verifies Logo
`save` and sync completion on fresh Web artifacts. Injected quota failures are
source-tested; real quota exhaustion is not reproduced.

Plan:
1. In pre.js, resolve a `Module.storage.ready` promise when the populate
   completes and reject or flag it with the error otherwise. Keep the existing
   console logging.
2. Wrap `IDBFS.queuePersist` through a property lookup, guarded by a feature
   check, to dispatch a `saved` event with a timestamp when a persist starts, a
   `persisted` event when it completes, and a `savefailed` event with the error
   when it fails. If Phase 0 shows autoPersist swallows errors, wrap the
   internal `syncfs` completion instead. If the lookup fails after an
   Emscripten upgrade, emit a single `signals-unavailable` state rather than
   silently doing nothing.
3. This is the shared foundation: the Dropbox plan's dirty signal and readiness
   promise come from here. Do not implement them twice.

Validation: source test with a fake `IDBFS` and `FS` covering ready, populate
error, persist success, persist failure, and missing `queuePersist`. A browser
test makes a Logo `save` and observes the events. The `savefailed` path is
tested with an injected failure; real quota exhaustion is not reproduced.

## Issue 2: Storage status model

Status: Implemented in `storage-status.js`, with a DOM adapter in `pre.js`.
Node tests cover absent/rejecting APIs, granted state, zero estimates, and
request events. The TestCafe proxy's HTTP origin does not expose
`navigator.storage`, so real browser estimates remain pending on a suitable
secure origin.

Plan:
1. Add `Module.storage.status()` returning
   `{ api, persisted, usage, quota, populated, lastSaved, lastError }` from
   `navigator.storage.persisted()` and `estimate()`, tolerating absent or
   rejecting APIs ("unknown"). `estimate()` values are approximate and
   browsers may round them; show them as approximate.
2. Refresh on ready, on each `persisted` or `savefailed` event, on
   `appinstalled`, on display-mode change, and when the status UI opens. Do not
   poll.
3. Keep this logic in a small module with injected `navigator.storage` and
   events so Node can test it. It loads no browser globals at import, as with
   the Dropbox plan's sync core.

Validation: source tests with stubbed storage: persisted true or false,
`estimate()` missing or rejecting, large and zero quotas, and event-driven
refresh.

## Issue 3: When to request persistence
Status: Implemented and source-tested for no request before save, one ordinary
automatic request per session, a 30-day denial cooldown, install/standalone
retries, and explicit requests. Firefox prompt/gesture behavior remains a
Phase 0 browser check; no automatic request is made at load.

Plan:
1. Request `persist()` at most once per session, after the first `persisted`
   event (the user's first successful save), and only if `persisted()` is false
   and the API exists. Never at load.
2. If the first request is denied, record the time and result under a separate
   `localStorage` key (for example `ucblogo.persist`) and do not auto-request
   again for a cool-down (default 30 days). Re-request automatically on
   `appinstalled` and on the first save in standalone mode, because Chromium
   re-evaluates heuristics then.
3. The status UI (Issue 4) offers a "Request persistent storage" button that
   ignores the cool-down, since it is an explicit gesture and Firefox needs one.
4. If Phase 0 shows Firefox suppresses or mishandles the prompt without a
   gesture, defer the automatic request to the next user gesture after the save
   and keep one call per session.
5. A save made by a user program (`runjs` or a program loop writing files)
   counts as the same save; the rule is per session, not per file, so a program
   cannot trigger repeated prompts.

Validation: browser test with `navigator.storage` stubbed asserts no
`persist()` call before a save, exactly one after the first save, no second
call after a denial within the cool-down, one more after a simulated
`appinstalled`, and a button press always calls it. Mark install events as
simulated.

## Issue 4: Status UI
Status: Implemented in the existing Settings dialog with a polite status line,
approximate usage, last completed save, last write error, and an explicit
request button. Desktop Chrome verifies accessible Settings and IDBFS status;
alternate API states are covered by injected source tests. A full real-browser
matrix remains pending.

Plan:
1. Add a Storage section to the existing Settings dialog in
   [shell.html](../../shell.html); no new dialog. If the Dropbox plan's Backup
   dialog ships, it shows the same line from the same status object.
2. Contents: one-sentence state ("Saved files are protected by the browser",
   "The browser may delete saved files if the device runs low on space", or
   "Browser storage is unavailable; files are lost when this tab closes"),
   approximate usage versus quota, last saved time, the last write error if
   any, and the "Request persistent storage" button when `persisted()` is false
   and the API exists. After a denial, the text says the browser declined and
   that backups are the safeguard; it does not blame the user.
3. Accessibility per [web-accessibility.md](web-accessibility.md): native
   button with a visible label, a `role="status"` polite live region that
   announces state changes only (not every usage refresh), no focus movement,
   theme-aware styling, and touch targets matching the settings controls.
4. On a write failure, also show a non-modal notice through the existing
   status/live-region mechanism with a pointer to the backup options, without
   stealing focus.

Validation: TestCafe asserts accessible names, button visibility rules, the
text for each state through the stub, and that no live region updates on a
usage-only refresh.

## Issue 5: Startup and write failure handling
Status: Implemented. Populate failures and distinct write errors appear in a
non-modal live region and in Settings. Source tests inject failures; real
IndexedDB denial and quota exhaustion remain unverified.

Problem: if IndexedDB is unavailable or blocked, the app runs on an in-memory
`/d` and the user finds out only when files vanish.

Plan:
1. On populate failure or unavailable IndexedDB, set the status to
   "unavailable", show the notice from Issue 4, and keep running.
2. On `savefailed`, keep the most recent error in the status and notify once
   per distinct error, not per write.
3. Do not retry in a loop. Let the next Logo write trigger the next attempt, as
   autoPersist already does.

Validation: browser test with an IDB open failure and a quota failure injected
through the Issue 1 fakes.

## Issue 6: Backup guidance matches what exists
Status: Documentation now describes that Download session serializes Logo's
workspace and does not export `/d` files. Source checks confirm the code path;
the planned throwaway-profile download-content browser check is still pending.

Problem: webedition.md tells users "Always back up your data. Use the
'Download Logo Session' button." That button exports the in-memory workspace as
a Logo `save` file, not the files under `/d`. A user who keeps data files, saved
pictures, or several program files would not have them in that download.

Plan:
1. Phase 0 verifies exactly what a session download contains and what is
   missing, using a throwaway profile.
2. Reword the docs to describe what it covers, point to the upload button for
   restoring, and name the real gap (no whole-folder export) until a backup
   provider ships. Do not add a new export feature in this plan; the
   [Local folder backup](local-folder-backup.md) and
   [Dropbox backup](dropbox-backup.md) plans own that.
3. If Phase 0 finds the gap serious, raise a separate small plan for a
   per-file or all-files download rather than widening this one.

Validation: a docs and source check that the wording matches the behavior found
in step 1, and a test that a session download includes the Logo workspace
definitions it claims to.

## Issue 7: Documentation
Status: Implemented for the available evidence. Durability claims and backup
coverage are corrected, Settings states and retry behavior are documented, and
Safari-specific eviction/install behavior is explicitly left unverified rather
than repeated as fact.

Plan:
1. Rewrite the persistence warnings in [webedition.md](../../webedition.md)
   (Virtual filesystem and the "progressive web app" paragraph): installing
   improves the odds on Chromium, it is not a guarantee; Firefox asks; Safari's
   behavior and 7-day cap with their sources once Phase 0 verifies them. Say
   where to see the status and how to request persistence. Remove "ensure".
2. Document the Settings Storage section and its states, the cool-down, and
   what "best-effort" means.
3. Update the persistence references in
   [dropbox-backup.md](dropbox-backup.md) and
   [local-folder-backup.md](local-folder-backup.md) to this plan (done with this
   split).

## Phase 0: Spikes

1. **Complete locally:** Read the active emsdk's `IDBFS` source. The active
   4.0.5 implementation exposes `queuePersist`, but its callback ignores
   background sync errors, so the app wraps `IDBFS.syncfs` completion instead.
2. Chrome desktop: `persisted()` in a normal tab, a bookmarked tab, and an
   installed app, before and after calling `persist()`. Record results.
3. Firefox: prompt behavior with and without a gesture; denied and granted
   outcomes; retry behavior.
4. iPad Safari: tab versus Home Screen app; `persist()` result; `estimate()`
   values. Maintainer-coordinated human checkpoint. Desktop Safari is a stated
   coverage gap.
5. Session download contents (Issue 6). Source inspection confirms the handler
   runs Logo `save` to `/tmp/ucblogo_session.lgo` and downloads that file; the
   planned throwaway-profile confirmation remains pending.

Exit criteria: documented behavior per browser with sources or recorded
observations; unknowns carried into the docs as unknowns.

## Production Verification After Deployment

Run this checklist only after the tagged Web Edition deploy workflow succeeds.
The production site is currently unused and has no user data, so these checks
may create and clear test data in disposable browser profiles. Reconfirm that
the site is still unused immediately before testing. Do not use a personal or
shared browser profile. These checks do not attempt to cause storage eviction
or fill the browser's storage quota.

### 1. Confirm the deployed release

- Record the release tag, deploy workflow run, published `gh_pages` commit,
   production URL (including any path prefix), UTC test time, and browser
   versions.
- Download the live `ucblogo.html`, `ucblogo.js`, `ucblogo.wasm`, `sw.js`,
   manifest, linked assets, and the Workbox runtime named by `sw.js`, using
   cache-busting query strings. Run the staging verifier against the temporary
   download with the expected release tag. Confirm the bundled JavaScript
   contains the storage controller and the HTML contains the Storage UI.
- Record the asset hashes and confirm superseded Workbox runtime filenames
   return 404. Remove only the temporary downloaded files after verification.
- Use disposable browser profiles and navigate directly to the production
   HTTPS URL, not through TestCafe's HTTP proxy; the proxy can change secure-
   context behavior and hide `navigator.storage`. Clear the production origin's
   site data in these disposable profiles when a clean first-run state is
   needed. Do not clear data from any personal or shared profile. Confirm the
   loaded page and service worker belong to the recorded release before
   interpreting any result.

### 2. Desktop Chrome production smoke

- In a clean disposable profile, open the production URL directly over HTTPS.
   Confirm `isSecureContext`, `navigator.storage`, and the app's startup. In
   **Settings > Browser storage**, check that status, approximate usage/quota,
   populated state, and last-save state are coherent. Compare with
   `await Module.storage.status()` and the browser's
   `await navigator.storage.persisted()` / `await navigator.storage.estimate()`
   in DevTools. Estimates are approximate; do not require exact values.
- Before saving, confirm there is no persistence request and no
   `ucblogo.persist` record in this new profile. Attach listeners to
   `Module.storage` for `saved`, `persisted`, and `savefailed`, then define a
   uniquely named temporary Logo procedure and save the workspace to a unique
   file under `/d`. Confirm the save signals, last-saved time, absence of an
   error, the `ucblogo.persist` result record, and the current value returned by
   `navigator.storage.persisted()`.
- Reload the app, load the unique file, and verify the procedure is present.
   Close and reopen the disposable profile once and repeat the load check. This
   verifies ordinary persistence across reload/relaunch, not eviction
   protection. Remove the temporary file and procedure after the checks.
- Test both request outcomes where the browser permits it. In Firefox, deny
   the first request in a disposable profile; verify the denial is recorded,
   the message is neutral, a later save within cooldown makes no automatic
   second request, and the explicit Settings button can request again. In
   Chromium, record the actual silent grant/denial; its decision is heuristic
   and may not be forceable. A profile that already has persistent storage
   cannot be made best-effort through the API, so use a separate disposable
   profile rather than trying to revoke the grant.
- Test the startup failure presentation in another disposable profile by
   blocking IndexedDB for the production origin with a supported, reversible
   browser control, then reloading. Confirm the app remains usable, `/d` is
   reported unavailable, and the non-modal notice appears. Restore storage
   access and reload. If the browser has no origin-level control, leave this
   production check pending; do not modify deployed code to simulate it. Do not
   simulate quota exhaustion or leave the profile with storage blocked.
- Repeat the start/save/reload checks in a standalone installation if
   available. Record whether standalone display mode and `appinstalled` are
   observed; do not assume every browser emits the install event. Clear only
   this disposable profile's origin data after recording results.

### 3. Other browsers and devices

- After desktop Chrome passes, repeat the start, status, unique save, and
   reload/load check in desktop Firefox. Record the permission prompt, its
   timing and granted/denied result, and complete the denial/cooldown check
   above if Firefox permits it.
- Run mobile Chrome emulation as a layout and control-visibility check only;
   label it as emulation, not real-device persistence coverage.
- Last, coordinate the maintainer's human iPad Safari checkpoint. Use a fresh
   disposable test profile/session; clear only the production origin's test data
   there. Check Safari tab and installed Home Screen app separately where
   available. Record `persisted()` and `estimate()` results, plus
   background/resume and relaunch behavior. Do not claim desktop Safari coverage
   from this check.

### 4. Limits and failure handling

- The site is currently test-only, but still never fill production storage or
   attempt to trigger real eviction. Block IndexedDB only in a disposable test
   profile, restore it immediately afterward, and never inject quota errors into
   a shared profile. Quota/write failures remain covered by local injected tests.
- A successful reload proves only that the test file survived that reload; it
   does not prove long-term durability or resistance to eviction. Report browser
   protection state separately from observed file survival.
- If live asset validation fails, stop behavioral testing and resolve the
   deployment/cache mismatch first. If a browser result contradicts the
   release, verify service-worker control and loaded response hashes before
   changing code or clearing data outside the disposable profile.
- Capture the browser/version, release tag, status object, console error,
   service-worker state, and exact reproduction steps. Do not include private
   file contents in reports.

### 5. Production test record

```text
UTC date / tester:
Production site still unused / no user data confirmed:
Release tag / deploy run / gh_pages commit:
Production URL / response asset hashes / staging verifier result:
Browser + version / OS / profile type (dedicated fresh profile):
Service-worker controller + release identity:
Storage API / persisted result / approximate usage and quota:
First-load prompt absent / save signal / completed save / write error:
Unique save file survived reload and loaded correctly:
Explicit request shown / clicked / browser result (or hidden because granted):
Firefox prompt behavior / mobile emulation result:
iPad Safari tab / installed app human checkpoint:
Failures, exclusions, or pending checks:
Eviction and real quota exhaustion: not tested:
Cleanup of temporary Logo file/procedure and downloaded assets:
```

## Sequencing

1. Phase 0 spikes.
2. Issue 1 (signals), then Issue 2 (status model). Source tests first.
3. Issues 3 and 5 (policy and failure handling), then Issue 4 (UI).
4. Issues 6 and 7 (docs) after Phase 0 results.

Dependencies: PWA Issue 10 supplies standalone detection and `appinstalled`;
until it lands, Issue 3 treats the display-mode media query as the only
standalone signal. Each phase ends with `npm run build:web`, `npm run
test:source`, and focused desktop Chrome checks on fresh artifacts (verify the
served files are the just-built ones, with no stale service worker), then
mobile Chrome emulation, Firefox, and the iPad checkpoint last.

## Out Of Scope

- Cloud or folder backup, and recovery from eviction: see the backup plans.
- Whole-folder export or import features (see Issue 6, step 3).
- Quota-management UI, per-file size display, or clearing storage.
- Changing the IDBFS mount, schema, or `/d` layout.
- Claiming real eviction or Safari inactivity behavior is tested.
