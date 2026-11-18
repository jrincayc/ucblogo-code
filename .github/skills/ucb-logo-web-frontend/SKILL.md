---
name: ucb-logo-web-frontend
description: "Develop and review the UCB Logo Web Edition user interface: shell.html markup and CSS, pre.js Module settings and handlers, dialogs, accessibility and ARIA, themes and dark mode, terminal and editor controls, mobile and touch input, web settings persistence, BEL cues, and SVG turtle graphics rendering, labels, and animation demos. For C/JS bridge and Logo word plumbing use the interpreter skill; for browser tests use the web-testing skill."
argument-hint: "Describe the UI, accessibility, settings, or graphics change."
---

# UCB Logo Web Frontend

Change the browser UI and SVG graphics of Web Edition while keeping the interpreter's runtime contracts intact. Interpreter and bridge changes belong to the [interpreter skill](../ucb-logo-interpreter-development/SKILL.md); tests and browser validation belong to the [web-testing skill](../ucb-logo-web-testing/SKILL.md). Logo-author documentation of the SVG DOM, animation loop, and sprites is in the [programming skill](../ucb-logo-programming/references/web-runtime-reference.md).

## Procedure

1. **Find the source.** Markup, CSS, and inline script live in `shell.html`; settings state and handlers in `pre.js`; turtle rendering in `webgraphics.c` and the SVG callbacks. `ucblogo.html` and `sw.js` are generated and must not be edited.
2. **Use native semantics first.** Native labels, `fieldset`/`legend`, `click` for buttons, and `showModal()` for modal dialogs; add ARIA only when native semantics fall short. Follow existing UI/CSS conventions.
3. **Route settings through one setter.** UI controls, Logo words, and persistence share `Module` state; guard `localStorage` and `document`.
4. **Keep Logo values in C.** If a Logo primitive needs the value, validate it in C and make one-way C-to-JS calls. JavaScript must not call back into the active runtime; see the interpreter skill's [primitives and bridge](../ucb-logo-interpreter-development/references/primitives-and-bridge.md).
5. **Write the failing test first, then rebuild.** Assert computed styles, accessible names, focus, and layout in a real browser. Rebuild before browser tests and check artifact freshness.
6. **Report evidence.** Name the browser, viewport, and what was observed; mobile emulation is not real-device coverage.

## Reference Index

| Reference | Load when |
| --- | --- |
| [UI, accessibility, and settings](./references/ui-and-settings.md) | editing dialogs, controls, themes, terminal output, BEL, or settings and persistence |
| [Graphics and demo iteration](./references/graphics-and-demos.md) | changing turtle rendering, SVG labels, or iterating on animation demos |
