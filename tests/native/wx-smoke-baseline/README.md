# wx Smoke Screenshot Baseline

Captured from the Linux native wx build under Xvfb at 1024x768 on 2026-11-16
(GCC 13.3, wxWidgets 3.2.4).

The root PNGs are the four stages reached during the smoke run:

- `text-before-clear.png`
- `graphics-shapes.png`
- `fileio-roundtrip.png`
- `htmlstring-markup.png`

The screenshots in `diagnostics/` were captured after the later text and graphics
stages timed out. They show the previous application state, not the expected
`text-after-clear` or `graphics-background` output, so they are retained only as
diagnostics and are not baseline images.

This is a manual visual reference from a partial run, not a fully validated
six-stage baseline or an automated pixel-diff test.