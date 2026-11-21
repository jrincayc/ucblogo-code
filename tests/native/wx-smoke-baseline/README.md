# wx Smoke Screenshot Baseline

Baselines must come from the GitHub Actions wx smoke job, not a developer
workstation. Native wx window geometry and rendering can differ across host
themes and environments. All six baselines are from
[run 43966352257](https://github.com/n1ywb-1/ucblogo-code/actions/runs/43966352257),
using `ubuntu-latest`, `libwxgtk3.2-dev`, and Xvfb at 1024x768.

The saved PNGs are byte-for-byte copies of that run's screenshots:

- `text-before-clear.png`
- `text-after-clear.png`
- `graphics-shapes.png`
- `graphics-background.png`
- `fileio-roundtrip.png`
- `htmlstring-markup.png`

The four earlier references were also captured by
[run 43959056904](https://github.com/jrincayc/ucblogo-code/actions/runs/43959056904),
which stopped each scene at its first visual mismatch against workstation
baselines. The driver now retains visual failures while continuing to capture
later stages, so the subsequent run supplied the two remaining references.
No workstation captures remain in the baseline set. To refresh references,
review a CI artifact's screenshots and copy only the intended baseline PNGs;
normal test runs never update references automatically.

Independent verification on commit `85aa4cf` in
[run 43967048973](https://github.com/n1ywb-1/ucblogo-code/actions/runs/43967048973)
passed all four scenes: every screenshot differed from its baseline by `0.000%`.
The same wx job passed all 105 Logo unit tests.

`wx_smoke.py` compares all six stages against the saved PNGs. A pixel is
counted as changed when any RGB channel differs by more than 16; the test fails
when more than 0.25% of pixels in the nonblack app bounds differ. An amplified
`*-diff.png` is saved beside the actual screenshot on failure.

The screenshots in `diagnostics/` are historical captures from when the later
text and graphics stages timed out. They show the previous application state,
not the expected `text-after-clear` or `graphics-background` output, so they are
retained only as diagnostics and are not baseline images.