# wx Smoke Screenshot Baseline

Baselines must come from the GitHub Actions wx smoke job, not a developer
workstation. Native wx window geometry and rendering can differ across host
themes and environments. The first verified CI capture is from
[run 43959056904](https://github.com/jrincayc/ucblogo-code/actions/runs/43959056904),
using `ubuntu-latest`, `libwxgtk3.2-dev`, and Xvfb at 1024x768.

These four baselines are byte-for-byte copies of that run's screenshots:

- `text-before-clear.png`
- `graphics-shapes.png`
- `fileio-roundtrip.png`
- `htmlstring-markup.png`

That run stopped each scene at its first visual mismatch against workstation
baselines, so it did not capture `text-after-clear.png` or
`graphics-background.png`. The driver now retains visual failures while
continuing to capture later stages. Those two files remain provisional
workstation captures until a CI rerun provides their replacements.

`wx_smoke.py` compares all six stages against the saved PNGs. A pixel is
counted as changed when any RGB channel differs by more than 16; the test fails
when more than 0.25% of pixels in the nonblack app bounds differ. An amplified
`*-diff.png` is saved beside the actual screenshot on failure.

The screenshots in `diagnostics/` are historical captures from when the later
text and graphics stages timed out. They show the previous application state,
not the expected `text-after-clear` or `graphics-background` output, so they are
retained only as diagnostics and are not baseline images.