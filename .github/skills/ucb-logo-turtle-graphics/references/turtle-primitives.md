# Turtle Graphics Primitives

This reference separates C primitives, library procedures, and Web startup facilities. Check local help or source before adding a name to an example.

## Motion and position

- `forward`/`fd`, `back`/`bk`, `left`/`lt`, `right`/`rt`: C primitives.
- `setheading`/`seth`, `setpos`, `setxy`, `setx`, `sety`, `home`, `arc`: C primitives.
- `pos`, `heading`, `towards`, `xcor`, `ycor`: position queries. `xcor` and `ycor` are library procedures built from `pos`.
- Heading is clockwise from positive Y; positive X is right and positive Y is up.

## Pen and palette

- `pendown`/`pd`, `penup`/`pu`: C primitives for pen position only.
- `penpaint`/`ppt`, `penerase`/`pe`, `penreverse`/`px`: C primitives that select mode and also put the pen down.
- `pencolor`/`pc`, `setpencolor`/`setpc`, `pensize`, `setpensize`, `penmode`, `penpattern`, `setpalette`, `palette`, and `setbackground`/`setbg`: C primitives or queries. Classifications and exact palette limits vary by edition.
- `pendownp` and `pendown?` are alternate query spellings.
- `pen` and `setpen` are library procedures. They reconstruct a pen-state bundle containing up/down, mode, size, color, and pattern. They are not a complete turtle, screen, shape, background, wrap, or palette snapshot.

## Visibility, shape, and screen state

- `showturtle`/`st`, `hideturtle`/`ht`, `shownp`/`shown?`, `setshape`, `shape`: C primitives. Current built-in shape names are `"triangle` and `"turtle`, with numeric shapes `0` and `1`.
- `wrap`, `window`, `fence`, `setscrunch`, `scrunch`, `screenmode`, `textscreen`/`ts`, `splitscreen`/`ss`, `fullscreen`/`fs`: C primitives or queries.
- `clean`: erase without changing position or heading.
- `clearscreen`/`cs`: erase and move to `[0 0]` heading `0`. It preserves pen state, size, color, background, visibility, shape, wrap/scrunch, and screen settings; layout preparation may enter graphics/split layout.

## Drawing operations

- `label`, `filled`, `fill`, `setlabelheight`, and `labelsize`: source availability varies. `filled` is usable in source for wx and Web, while flood-fill `fill` is explicitly non-portable and is not implemented in the Web adapter.
- `savepict`, `loadpict`, and `epspict`: C primitives with portability and format caveats.
- `mousepos`, `buttonp`/`button?`, `button`, and guarded `clickpos`: C/adapter input queries. Button variables and event handlers are edition-dependent.

Exclude names absent from registrations or startup definitions. The implemented Web startup and animation facilities are listed only in [Platform notes](./platform-notes.md).

## State and procedure tools

- `repeat` is primitive. `for`, `foreach`, `iseq`, `rseq`, `map`, `filter`, `reduce`, and `localmake` are library or other user-level facilities.
- For reproducibility, use `localmake` and named procedures rather than relying on user or startup variables.

The exact appearance of fill, erase/reverse, palettes, labels, shapes, and mouse/touch behavior is unverified without an edition/runtime test. Source availability is not evidence of visual parity.

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
