# Graphics And Demo Iteration

- Keep shared turtle semantics in the shared graphics layer and rendering in platform backends. Verify affected coordinates/scaling, heading, pen modes/colors, wrapping/clipping, labels, fills, visibility, and redraw.
- SVG label sizing lesson: CSS font-size rules can override SVG presentation attributes. A global rule such as `* { font-size: ... }` can force `LABEL` text size and break drawing-relative behavior. For turtle labels, prefer explicit inline style on SVG text nodes (for example `el.style.fontSize = "${pen_info.fntsz}px"`) so label size follows drawing units and SVG scaling rather than UI text controls.
- For browser-facing animation work, require visible evidence before calling a change successful: run the scene in-browser, verify animation state and SVG output shape/color signals, and capture a screenshot checkpoint.
- Treat command submission as asynchronous/queued in the web terminal path. After sending load/run commands, allow settlement before reading state, otherwise diagnostic reads can reflect stale pre-command conditions.
- Prefer stabilizing standalone scene files first, then compose them into a combined runner. Cross-scene regressions are commonly init/order/palette leakage issues, not proof that the standalone scene math is wrong.
