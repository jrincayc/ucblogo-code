# Plan: HELP-FONT-24 CI Failure

Status: Proposed; investigation has not started.
Date: 2026-11-16

## Problem

`HELP-FONT-24` in `tests/acceptance/help-font-size.testcafe.js` failed twice on
one commit (GitHub run 44007224888, SHA `505e221`, job "Build and Test
WebAssembly", Ubuntu 26.04, Chrome 157). TestCafe could not find
`#helpCatList h3 button` with text "Constructors" (89/90 tests passed; the 16px
and 40px variants passed). The previous commit (`46bd2cb`, run 44006458712)
passed, and the two commits differ only in a script and a doc line, so this is
intermittent or timing/environment dependent rather than caused by the diff.

## Facts

- `openEntry` clicks the Help button, then immediately clicks the category
  button. `build()` in [shell.html](../../shell.html) fills `#helpCatList` from
  `Module.help.load()` when the dialog opens; the docs index loads separately
  and asynchronously.
- The test sets the text size (24px) before opening Help.
- Only the first selector in `openEntry` failed; the dialog itself opened.

## Steps

1. Reproduce locally: run only the help-font fixture repeatedly (focused run per
   the web-testing skill, desktop Chrome, fresh profile and no stale service
   worker) at 24px and at all three sizes; record pass rate.
2. If it does not reproduce, fetch the failing job's TestCafe screenshots and
   reports (artifact "Web Test Reports") and inspect `#helpCatList` contents and
   dialog state at the failure.
3. Decide the cause: help data not ready when the dialog opens, category title
   text different from "Constructors" under some state, or the first click
   landing before the dialog is interactive at the larger size.
4. Fix the cause: wait for the category list (or a ready signal) in
   `openEntry`, or fix `build()` / `Module.help.load()` if the application can
   show an empty list. Prefer fixing an application race over adding sleeps.
5. Run the focused fixture many times, then the full web suite once.
6. Record the lesson in the web-testing skill and remove the "Intermittent"
   note from
   [ci-and-deploy.md](../../.github/skills/ucb-logo-build-deploy/references/ci-and-deploy.md).

## Out Of Scope

- Rerunning CI until it passes without a diagnosis.
- Changing other Help dialog behavior.

<!-- Co-authored by GitHub Copilot. Scope: CI failure investigation plan. Energy (kWh) and CO2: unavailable. -->
