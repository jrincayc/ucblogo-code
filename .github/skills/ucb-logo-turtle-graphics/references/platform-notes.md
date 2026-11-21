# Turtle Graphics Platform Notes

## Web Edition as the default

When no edition is specified, write shared turtle primitives and describe Web Edition behavior only with source-backed caveats. The current Web logical SVG canvas is `640 x 480`; its implementation may become variable in future work. Do not promise a fixed coordinate or pixel contract.

## wxWidgets

wxWidgets uses a host widget/canvas size and tracks resize. Turtle coordinates remain turtle units, while visible bounds follow the host window. Stateful rendering, fonts, mouse coordinates, printer output, and palette appearance remain edition-dependent.

## Shared core

The portable core includes one turtle, positive X right, positive Y up, heading measured clockwise from positive Y, `forward`/`back`/`left`/`right`, `setpos`/`setxy`/`setheading`, `home`, pen primitives, `clean`, and `clearscreen`/`cs`.

Standard palette slots `0` through `7` are the safer portable starting point; exact RGB appearance and custom palette ranges remain host-dependent.

## Cleanup and layout

`clean` leaves position, heading, pen state, visibility, background, shape, wrap/scrunch, and palette unchanged. `clearscreen` adds position `[0 0]` and heading `0` to the clear. Neither is a complete reset.

A clear can trigger graphics/split-screen preparation. Web `prepare_to_draw` requests split-screen behavior, while wx native layout also responds to its active graphics mode. Screen layout should be established explicitly when it matters.

## Source availability versus rendering

Some maintained manual annotations are stale:

- `filled` is currently guarded and implemented for wx and Web, even though older text calls it wx-only.
- `setlabelheight` and `labelsize` have Web and wx guards, but exact browser/wx sizing is not runtime-proven.
- `clickpos` has current Web and wx guard/source support, while older manual text labels it wx-only.
- `scrunch` and `setscrunch` remain registered in shared C despite Web prose calling them missing.

Conversely, source availability does not prove visual parity. Do not claim pixel-equivalent palette, erase/reverse, fill, labels, scrunch, shapes, pen patterns, or screen layout. Browser and wx visual behavior remain unverified without runtime tests.

## Web-only facilities

The Web startup provides `makesprite` and `setturtlesprite` as Logo wrappers. The SVG mechanism supports one turtle/sprite; extra independent objects require separate Web DOM/JS work.

The implemented animation names are `animationstart`, `animationstop`, `animationact`, `animationtime`, and `animationdelta`. Older prose names for animation are absent from source implementations. Do not teach absent names as Logo words.

Web input uses mouse handlers and lacks a demonstrated touch implementation. Do not claim touch equivalence. Web export replaces some native picture workflows with generated SVG.

## Features to keep edition-specific

- `fill` flood-fill is not portable and has no Web adapter implementation.
- Picture save/load/EPS behavior varies or is unavailable; do not present it as a portable graphics workflow.
- Custom palette, pen pattern, reverse/erase, label sizing, and printer/export appearance remain host-dependent.
- Full/split/text screen layout can differ across responsive Web viewports and wx native frames.
- Animation, sprite DOM, SVG export, and `runjs` bridge facilities are Web Edition facilities.

No visual parity, sprite demo, or platform-wide rendering matrix is established by this reference alone.

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
