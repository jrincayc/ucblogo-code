# Plan: HELP-FONT CI Failures

Status: Diagnosis in progress; failure screenshots and diagnostics are in place
(pushed in `b5f9aae`) and waiting for the next CI failure.
Date: 2026-11-16

## Problem

The `HELP-FONT-<size>` tests in `tests/acceptance/help-font-size.testcafe.js`
fail intermittently in the "Build and Test WebAssembly" job. TestCafe cannot
find `#helpCatList h3 button` with text "Constructors" at the first click in
`openEntry`, after the 10s selector timeout.

## Established (CI logs of 33 runs, 2026-11-15 onward)

| Test | Failed | Rate |
| --- | --- | --- |
| HELP-FONT-16 | 0 / 33 | 0% |
| HELP-FONT-24 | 7 / 33 | 21% |
| HELP-FONT-40 | 5 / 33 | 15% |

- About 10 of 33 runs (30%) had at least one font failure; three runs failed
  both 24 and 40. Every failure is the same missing-selector error.
- Runs in the jrincayc repo (PR #268 `pull_request` runs of the same SHAs) failed
  more often (24px 5/13, 40px 3/13) than the fork's push runs (2/20 each). The
  difference is the CI context, not the code; the sample is small.
- Run 44007224888 (`505e221`) has one `HELP-FONT-24` failure on the fork; its
  jrincayc run of the same SHA passed. An earlier note that it "failed twice"
  was wrong.
- Not reproduced locally: 8/8 idle and 12/12 on 2 pinned cores under CPU load.

## Not established

- Whether the cause is size-specific or an ordering effect: 16px always runs
  first, so the data cannot separate them.
- Whether the Help dialog opened at all. `build()` is synchronous, so an empty
  list on a successful open is unlikely. Candidates: the click on the Help
  button missed or was lost; `build()` threw and the "Help is not available"
  alert was suppressed by the test's dialog handler; `isolateSettings` calls
  `reload()` and `ready()` may pass on the old page before the reload takes
  effect.

## Done

- Failed tests now write screenshots to `screenshots/failures/` (runner default
  `takeOnFails`), uploaded in the "WebAssembly Test Reports" artifact.
- `restoreSettings` captures `Logo state:` before its reload and includes
  `helpOpen`, `helpCategories`, `activeElement`, `fontSize`.
- `scripts/ci-screenshots.sh [run-id|branch] [repo]` fetches and caches the
  screenshots in `/tmp/ci-screenshots/`.

## Follow-up steps

1. When a run fails, fetch it with `scripts/ci-screenshots.sh`, view the
   screenshot, and read `Logo state:` from `scripts/ci-run.sh log`. Use the
   `helpOpen` and `helpCategories` values to pick a cause: dialog not open (click
   or build problem), open with zero categories (data), or open with categories
   but no "Constructors" (title text).
2. To separate size from order, run the fixture with the order changed (for
   example 40, 24, 16) or repeat one size, in CI or under load.
3. Fix the cause. Prefer fixing an application race or the `reload()`/`ready()`
   synchronization in `isolateSettings` over adding sleeps; if `build()` can
   fail, surface the error instead of an alert the test cannot see.
4. Run the focused fixture many times, then the full web suite once.
5. Record the lesson in the web-testing skill and remove the "Intermittent"
   note from
   [ci-and-deploy.md](../../.github/skills/ucb-logo-build-deploy/references/ci-and-deploy.md).

## Out Of Scope

- Rerunning CI until it passes without a diagnosis.
- Changing other Help dialog behavior.

<!-- Co-authored by GitHub Copilot. Scope: CI failure investigation plan. Energy (kWh) and CO2: unavailable. -->
