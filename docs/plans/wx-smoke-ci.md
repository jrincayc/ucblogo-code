# Fix wx Smoke Tests And Add CI Coverage

Status: driver fix and six-stage visual-diff coverage completed on 2026-11-16
in commit `5f59d17` (`Complete wx smoke visual baseline coverage`). CI integration
is enabled, and all six workstation baselines have been replaced with CI
captures. Independent CI verification passes all six baseline comparisons and
all 105 native wx unit tests.

## Current Evidence

- Independent CI verification of commit `85aa4cf` in
  [run 43967048973](https://github.com/n1ywb-1/ucblogo-code/actions/runs/43967048973)
  passed all four smoke scenes, with `0.000%` changed pixels for all six stages,
  and all 105 native wx Logo unit tests. Both terminal jobs also passed.
- Native wx CI run
  [43966352257](https://github.com/n1ywb-1/ucblogo-code/actions/runs/43966352257)
  passed all 105 Logo unit tests and captured all six smoke stages. Four earlier
  CI baselines matched at `0.000%`; only the two remaining workstation baselines
  failed. Those two PNGs have now been copied unchanged from this run's artifact.
- The screenshot smoke driver in
  [tests/native/wx_smoke.py](../../tests/native/wx_smoke.py) now passes all four
  scenes and captures all six stages with the normal timeouts. The verified
  configuration is native Linux, GCC 13.3, wxWidgets 3.2.4, and Xvfb at 1024x768.
- Isolated probes confirmed that `stop` in the acknowledgement helper prevented
  subsequent command-file stages from running. Replacing `readword` with
  `readchars` still failed; retaining `readword` and letting the bounded loop
  finish without `stop` made both two-stage scenes pass.
- The driver now initializes a local acknowledgement flag, polls only while it
  is false, and completes the loop normally. The existing atomic replacement of
  `pending` acknowledgement files with `ready` is retained; that change alone
  had not resolved the original failure.
- The maintainer approved the new `text-after-clear` and `graphics-background`
  local screenshots on 2026-11-16. Their CI replacements were inspected for the
  same expected states: positioned text after clearing and a white graphics
  background with the red drawing. All six saved references now match the CI
  capture at `0.000%`. See the
  [baseline README](../../tests/native/wx-smoke-baseline/README.md).
- Before the driver fix, unmodified upstream master at `f0ca245` reproduced the
  same two-stage handshake failure as the branch's native build at `238e466`,
  using GCC 13.3 and wxWidgets 3.2.4. Both reached the first marker but not the
  second, with screenshots showing a Logo prompt rather than a frozen GUI.
  The corrected helper has not yet been rechecked against upstream master.
- The maintainer considers behavior shared with upstream master intended.
  The interpreter and active WASM build configuration were left unchanged.
- The driver treats an app still open after its post-capture exit wait as a
  successful run and cleans it up. It has no explicit script-completion marker,
  so a passing result alone does not prove that `bye` ran or the app exited on
  its own. CI sets `TMPDIR` inside the archived report directory, preserving
  generated scripts, logs, and acknowledgement files even on failure.
- The maintainer observed that the first interactive `cleartext` and
  `setbackground` entries appeared ineffective while the second worked. A
  causal link to the helper's `stop` has not been independently established.
- Commit `1a2f38a` enabled the screenshot driver in CI. Its first run exposed
  host-dependent window geometry and rendering in the workstation baselines.
- Commit `7ad7bec` accumulates visual mismatch and missing-baseline errors while
  acknowledging each capture, allowing later stages to run without suppressing
  test failures. Focused mocked checks verify matching and mismatching two-stage
  scenes; the CI artifact confirms that both later stages are now captured.

## Finish Plan

1. **Completed: isolate why scene execution stops.** The two-stage reproducer
   distinguished the `stop`-based helper from a normally completing bounded
   loop. Both formerly failing scenes now reach their second markers.

2. **Completed: fix only the driver and approve visual coverage.** Commit
   `5f59d17` contains the helper fix, two approved PNGs, six-stage comparison
   coverage, and baseline documentation. Timeouts and interpreter semantics
  were not changed. CI integration followed separately in `1a2f38a`.

3. **Partially complete: make failures diagnosable.** Visual failures no longer
  prevent later captures, and CI archives generated scripts and logs. Still
  pending: distinguish startup, capture,
   acknowledgement, script-completion, and process-exit failures. Make exhausted
  acknowledgement polling an explicit failure. Add structured stage results and an
   explicit completion marker before making claims about reaching `bye`, and
   document the cleanup policy when the GUI remains open afterward.

4. **Partially complete: validate CI coverage.** All four scenes pass locally;
  CI captures all six stages and its 105 native wx unit tests pass. All six
  baselines pass independent CI verification in run `43967048973`. Still
  pending: exercise withheld-acknowledgement and early-process-exit failures to
  verify nonzero results and cleanup, and recheck the corrected handshake
  against upstream master. Verify script completion separately from
  driver-enforced GUI cleanup.

5. **Completed: add the verified command to the existing wx CI job.**
  [.github/workflows/ci.yaml](../../.github/workflows/ci.yaml) installs Xvfb,
  `xauth`, `x11-apps`, `x11-utils`, and `python3-pil`; runs the driver with
  `LOGOLIB` set; and archives screenshots, scripts, logs, and results using
  `if: always()`. The driver owns Xvfb and is not wrapped in another `xvfb-run`.

6. **Completed: document and checkpoint the CI baselines.** The verified driver fix and
  CI integration were committed separately. The maintainer requested the CI
  capture rerun and replacement of the remaining workstation baselines. Their
  provenance and independent passing verification are recorded in the baseline
  README. Broader diagnostic hardening remains separate work.

## Completion Criteria

- [x] All four smoke scenes pass locally with the normal timeouts.
- [x] CI captures all six stages, including after a visual mismatch.
- [x] All six baseline PNGs come from CI; no workstation references remain.
- [x] An independent CI run passes all six baseline comparisons (`43967048973`).
- [x] Driver fix and approved baselines are committed as `5f59d17`.
- [x] The interpreter and active WASM build configuration remain unchanged.
- [ ] Script completion and post-`bye` cleanup are explicitly distinguished.
- [ ] Negative-path checks demonstrate failure detection and process cleanup.
- [ ] The corrected handshake is verified against upstream master.
- [x] All 105 native wx Logo unit tests pass in CI run `43966352257`.
- [x] CI runs the smoke driver and retains diagnostics even on failure.
- [x] Final process-ownership check confirms no temporary test processes remain,
    except preview sessions the maintainer still wants open.