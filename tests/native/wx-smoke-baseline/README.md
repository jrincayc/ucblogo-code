# wx Smoke Screenshot Baseline

Captured from the Linux native wx build under Xvfb at 1024x768 on 2026-11-16
(GCC 13.3, wxWidgets 3.2.4).

The root PNGs are the six approved visual baselines:

- `text-before-clear.png`
- `text-after-clear.png`
- `graphics-shapes.png`
- `graphics-background.png`
- `fileio-roundtrip.png`
- `htmlstring-markup.png`

`wx_smoke.py` compares all six stages against the saved PNGs. A pixel is
counted as changed when any RGB channel differs by more than 16; the test fails
when more than 0.25% of pixels in the nonblack app bounds differ. An amplified
`*-diff.png` is saved beside the actual screenshot on failure.

The screenshots in `diagnostics/` are historical captures from when the later
text and graphics stages timed out. They show the previous application state,
not the expected `text-after-clear` or `graphics-background` output, so they are
retained only as diagnostics and are not baseline images.

With `stop` removed from the acknowledgement helper, current smoke runs reach
all six stages. The `text-after-clear` and `graphics-background` captures were
visually approved on 2026-11-16 and added to the baseline set.