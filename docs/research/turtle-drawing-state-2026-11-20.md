# Research Findings: Turtle Drawing State and `CLEARSCREEN`

**Status:** Complete
**Researched:** 2026-11-20 (UTC)
**Question:** What mutable turtle, pen, and screen state must UCB Logo drawing guidance establish explicitly, and what does `CLEARSCREEN` actually preserve or reset?
**Scope and assumptions:** Repository-local evidence only. The scope is user-visible drawing state in shared C, Web Edition, and wxWidgets Edition. It excludes browser or wx runtime rendering checks, external documentation, source changes, tests, and builds. “Pristine” means a session that has not been changed by prior drawing code or a user/startup procedure.

## Key Findings

1. **[Documented fact — High]** `CLEARSCREEN` (`CS`) erases the graphics content and moves the turtle to position `[0 0]` with heading `0`. Its help and manual entries say it is equivalent to `HOME` plus `CLEAN`; the implementation performs the clear and then the center/reset in `cs_helper(TRUE)`. Evidence: [E1], [E2], [E3].
2. **[Documented fact — High]** `CLEARSCREEN` does not reset the pen color, pen size, pen up/down position, pen mode, or background. It also does not reset turtle visibility or shape. The shared C implementation has no assignments to these fields in `cs_helper`; it only conditionally records existing non-default state for graphics refresh. Evidence: [E3], [E4], [E5].
3. **[Documented fact/direct observation — High]** `CLEAN` is more strictly non-mutating than `CLEARSCREEN`: it calls `cs_helper(FALSE)`, clears the graphics backend, and leaves position and heading alone. Its help and manual agree that its turtle state is unchanged. Evidence: [E1], [E2], [E3].
4. **[Documented fact/direct observation — High]** Visibility is preserved at the user-visible `SHOWNP` level. `HIDETURTLE` sets `user_turtle_shown` false and `SHOWTURTLE` sets it true; `lclearscreen` and `lclean` neither call those primitives nor assign the flag. The Web backend's clear also does not alter the turtle DOM display style. Evidence: [E5], [E6].
5. **[Documented fact/direct observation — High]** Background is preserved and used for clearing. `SETBACKGROUND` alone changes the stored background; neither shared clear helper calls it. Web `clear_screen` leaves `logoBackground` styling unchanged, and wx `realClearScreen` clears with the current stored background brush. Evidence: [E4], [E6], [E7].
6. **[Documented fact — High]** Pen position and mode are independent mutables: `PENDOWN` sets down without changing mode, `PENUP` sets up without changing mode, and `PENPAINT`, `PENERASE`, and `PENREVERSE` set both down and a mode. `CLEARSCREEN`/`CLEAN` contain no corresponding state assignments. Evidence: [E4], [E8].
7. **[Inference — High basis]** Reproducible drawing code should explicitly establish every relevant pen/turtle/screen state instead of assuming a pristine session or that `CLEARSCREEN` reset it. A setup should at minimum choose pen color, pen size, pen mode/up-down state, visibility, and background when those affect the drawing. Position and heading should be set explicitly if the drawing relies on absolute placement. Evidence: [E3], [E4], [E5], [E9].
8. **[Documented fact/direct observation — Medium]** A pristine session is only partly cross-platform in the source. Shared C globals establish position `[0 0]`, heading `0`, scale `1/1`, wrap mode, paint mode, and a user-visible turtle; current Web and wx source establish pen color `7`, down pen state, and background `0`. Initial pen size is not established consistently enough to be assumed; one Web C/JS startup pair even appears to disagree until a primitive sets size. Evidence: [E10], [E11], [E12].
9. **[Direct observation — Medium]** `CLEARSCREEN` and `CLEAN` can have a platform layout side effect because shared graphics code calls `prepare_to_draw` before clearing. Web `prepare_to_draw` requests split-screen, and wx `wxPrepare`/`wx_clear` can enter split-screen if graphics mode is not active. This is separate from clearing and does not prove that every Logo `SCREENMODE`/layout state is identical across editions. Evidence: [E13], [E14].

## State Matrix

| State | `CLEARSCREEN` / `CS` | `CLEAN` | Basis |
| --- | --- | --- | --- |
| Position and heading | Sets position to `[0 0]` and heading to `0` | Unchanged | [E1], [E2], [E3] |
| Pen up/down (`PENDOWNP`) | Unchanged | Unchanged | [E3], [E4], [E8] |
| Pen mode (`PENMODE`) | Unchanged | Unchanged | [E1], [E3], [E4], [E8] |
| Pen color (`PENCOLOR`) | Unchanged | Unchanged | [E3], [E4], [E8] |
| Pen size (`PENSIZE`) | Unchanged | Unchanged | [E3], [E4], [E12] |
| Pen pattern | No reset assignment in shared clear helper | No reset assignment in shared clear helper | [E3], [E4] |
| Background (`BACKGROUND`) | Unchanged; used as the clear surface | Unchanged; used as the clear surface | [E4], [E6], [E7] |
| Turtle visibility (`SHOWNP`) | Unchanged | Unchanged | [E5], [E6] |
| Turtle shape | Unchanged | Unchanged | [E3], [E15] |
| Turtle wrap/fence/window mode | Unchanged | Unchanged | [E3], [E10], [E15] |
| Scrunch/`X_SCALE`/`Y_SCALE` | Unchanged | Unchanged | [E3], [E10], [E15] |
| Screen layout | No direct reset; `prepare_to_draw` may enter graphics/split layout | Same | [E13], [E14] |

## Evidence Ledger

| ID | Finding | Type | Confidence and basis | Source and date | Planning relevance |
| --- | --- | --- | --- | --- | --- |
| E1 | Bundled help distinguishes `CLEAN` and `CLEARSCREEN`: `CLEAN` says state is unchanged; `CLEARSCREEN` says it erases and sends the turtle to initial position/heading. | Documented fact | High; direct file text. | [`helpfiles/clean:1-5`](../../helpfiles/clean#L1-L5), [`helpfiles/clearscreen:1-5`](../../helpfiles/clearscreen#L1-L5), latest commit 2008-09-13. | The skill can use the help as a first-party distinction rather than inferring preservation from the word “clean.” |
| E2 | The maintained and historical manuals give the same `CLEAN` and `CLEARSCREEN` descriptions. | Documented fact | High; independent manual passages agree. | [`docs/ucblogo.texi:3785-3827`](../../docs/ucblogo.texi#L3785-L3827), [`usermanual:1759-1769`](../../usermanual#L1759-L1769), latest commit 2026-08-30. | Independent documentation corroboration for position/heading and unchanged clean state. |
| E3 | `cs_helper(FALSE)` clears only, while `cs_helper(TRUE)` additionally zeroes and moves position/heading. Neither helper assigns pen color, size, up/down, mode, background, visibility, or shape. | Direct observation | High; exact shared-C control flow. | [`graphics.c:776-817`](../../graphics.c#L776-L817), latest commit 2026-10-17. | Establishes the precise shared behavior and why neither word acts as a universal reset. |
| E4 | Pen mutation/query functions change only their named state. `lpendown`, `lpenup`, `lpenpaint`, `lpenerase`, `lpenreverse`, `lsetpencolor`, `lsetbackground`, and `lsetpensize` assign their own fields; `CLEARSCREEN` calls none of them. | Direct observation | High; exact setter and query functions. | [`graphics.c:1055-1165`](../../graphics.c#L1055-L1165), [`graphics.c:1246-1264`](../../graphics.c#L1246-L1264), latest commit 2026-10-17. | Supports explicit pen setup and the pen-state matrix. |
| E5 | Visibility is represented by `user_turtle_shown`, while `SHOWNP` reads that flag. `HIDETURTLE`/`SHOWTURTLE` change it and `cs_helper`/backends do not. | Documented fact/direct observation | High; separate primitives and flag reads. | [`graphics.c:563-617`](../../graphics.c#L563-L617), latest commit 2026-10-17; [`helpfiles/hideturtle:1-6`](../../helpfiles/hideturtle#L1-L6), [`helpfiles/showturtle:1-4`](../../helpfiles/showturtle#L1-L4), latest commit 2008-09-13. | A clear must not be documented as changing turtle visibility. |
| E6 | Web clearing replaces the SVG drawing group and empties `logoDefs`; it does not set the background or call hide/show, and hide/show operate on the turtle's separate display style. | Direct observation | High for source; Medium for actual browser rendering without a browser run. | [`pre.js:592-606`](../../pre.js#L592-L606), [`pre.js:664-700`](../../pre.js#L664-L700), [`pre.js:878-882`](../../pre.js#L878-L882), latest commit 2026-11-16; [`webgraphics.c:41-43`](../../webgraphics.c#L41-L43). | Corroborates Web preservation of background and visibility. |
| E7 | wx `realClearScreen` creates a brush from `TurtleFrame::back_ground` and clears with it; `wx_clear` also does layout work but no background reset. | Direct observation | High for source; browser/wx rendering remains unrun. | [`wxTurtleGraphics.cpp:523-532`](../../wxTurtleGraphics.cpp#L523-L532), [`wxTurtleGraphics.cpp:891-905`](../../wxTurtleGraphics.cpp#L891-L905), [`wxGraphics.h:63-84`](../../wxGraphics.h#L63-L84), latest commit 2026-08-30/2026-09-04. | Distinguishes clearing to the current background from changing background state. |
| E8 | Help and manual describe pen up/down, paint/erase/reverse, color, size, and background semantics explicitly. | Documented fact | High; repeated in local help/manual. | [`helpfiles/pendown:1-4`](../../helpfiles/pendown#L1-L4), [`helpfiles/penup:1-4`](../../helpfiles/penup#L1-L4), [`helpfiles/penpaint:1-3`](../../helpfiles/penpaint#L1-L3), [`helpfiles/penerase:1-3`](../../helpfiles/penerase#L1-L3), [`helpfiles/penreverse:1-4`](../../helpfiles/penreverse#L1-L4); [`docs/ucblogo.texi:4153-4385`](../../docs/ucblogo.texi#L4153-L4385), latest commit 2026-08-30. | The skill should teach pen mode and position independently. |
| E9 | Mutable-state setup is a safer portable pattern: `SETPENCOLOR`, `SETPENSIZE`, `PENPAINT`/`PENDOWN`, `SHOWTURTLE`, and `SETBACKGROUND` are registered shared primitives. | Documented fact/direct observation | High; shared primitive registration plus documented arity. | [`init.c:328-341`](../../init.c#L328-L341), [`init.c:410-432`](../../init.c#L410-L432), [`init.c:456-464`](../../init.c#L456-L464), latest commit 2026-11-14. | Planning relevance for setup and primitives to check. |
| E10 | Shared C starts with paint mode, `[0 0]`, heading `0`, scale `1/1`, wrap mode, and no shape change from the static triangle shape; it also starts with `user_turtle_shown = TRUE`. | Direct observation | High; explicit global initializers and static shape zero. | [`graphics.c:85-94`](../../graphics.c#L85-L94), [`turtleshape.c:81-88`](../../turtleshape.c#L81-L88), [`turtleshape.c:105-106`](../../turtleshape.c#L105-L106), latest commit 2026-10-17/2026-08-30. | Establishes a shared pristine core while avoiding an unsupported claim for pen size. |
| E11 | Current Web and wx source establish several pen defaults: Web graphics init sets pen color `7` and down state `0`, sets background `0`, and wx initializes pen color `7`, width `1`, down visibility `0`, and background `0`. | Direct observation | High for source initializers; Medium for runtime startup with user `startup.lg`. | [`webgraphics.c:299-303`](../../webgraphics.c#L299-L303), [`pre.js:519-527`](../../pre.js#L519-L527), [`pre.js:508-516`](../../pre.js#L508-L516); [`wxTurtleGraphics.cpp:60-65`](../../wxTurtleGraphics.cpp#L60-L65), [`wxTurtleGraphics.cpp:232-245`](../../wxTurtleGraphics.cpp#L232-L245), latest commit 2026-11-12/2026-11-16/2026-08-30. | Provides default corroboration but still argues for explicit setup. |
| E12 | Initial pen size is not portable from current source. wx explicitly initializes width `1` and has equal height; Web's JS `pen_info.sz` starts `1`, but Web C `xgr_pen.pw/ph` is not initialized by `graphics_init`, and `PENSIZE` reads C fields. | Direct observation/inference | Medium; source discrepancy is clear, actual PENSIZE output is not runtime-confirmed. | [`pre.js:508-516`](../../pre.js#L508-L516), [`pre.js:680-688`](../../pre.js#L680-L688); [`webgraphics.c:299-303`](../../webgraphics.c#L299-L303), [`webgraphics.h:69-73`](../../webgraphics.h#L69-L73); [`wxTurtleGraphics.cpp:232-245`](../../wxTurtleGraphics.cpp#L232-L245), latest commit 2026-11-16/2026-11-12/2026-08-30. | Explicit `SETPENSIZE` is the reliable skill pattern; initial-size semantics need runtime follow-up. |
| E13 | Web `prepare_to_draw` calls `split_screen`, and `clear_screen` is reached after `prepare_to_draw`; wx `prepare_to_draw` calls `wxPrepare`, which calls `wxSplitScreen` as needed. | Direct observation | High for source; Medium for layout outcome across viewports. | [`pre.js:582-584`](../../pre.js#L582-L584), [`pre.js:772-797`](../../pre.js#L772-L797); [`wxGraphics.h:52-55`](../../wxGraphics.h#L52-L55), [`wxTurtleGraphics.cpp:891-925`](../../wxTurtleGraphics.cpp#L891-L925), latest commit 2026-11-16/2026-08-30. | Do not interpret clearing alone as a complete screen-layout reset; platform layout differences remain. |
| E14 | The shared C `lsplitscreen`, `ltextscreen`, and `lfullscreen` functions set Logo screen mode, but `lclearscreen`/`lclean` call the shared clear helper and do not call those mode primitives. | Direct observation | High; exact shared function bodies. | [`graphics.c:1020-1050`](../../graphics.c#L1020-L1050), latest commit 2026-10-17. | Screen layout is separate mutable state and is not a universal clear-screen reset. |
| E15 | Shape and turtle-mode state have dedicated mutators and are not touched by clearing. The current shared `current_shape` starts at `0` and wrap mode is the shared initial mode. | Direct observation | High; exact mutator/control flow. | [`graphics.c:620-654`](../../graphics.c#L620-L654), [`graphics.c:928-974`](../../graphics.c#L928-L974), [`turtleshape.c:105-106`](../../turtleshape.c#L105-L106), latest commit 2026-10-17/2026-08-30. | Extra state should not be assumed reset by cleanup. |

## Options and Trade-offs

- **Explicit setup vs pristine assumptions.** Explicit setup adds primitives, but makes procedures and demos reproducible after other work or startup state. A pristine-only assumption is shorter, but is fragile even before considering user `startup.lg` mutations.
- **Setup each frame vs save/restore state.** Establishing state before a procedure is simplest for examples. Saving/restoring state (using the library `PEN`/`SETPEN` and turtle queries) preserves an existing session but requires explicit handling of shape, visibility, position, heading, background, and platform-specific pattern information.
- **Reset helpers vs cleanup helpers.** `CLEARSCREEN` combines clearing with position/heading reset. `CLEAN` is useful when the drawing position and heading must remain unchanged. Neither is a full reset for pen, screen, or visibility settings.
- **Portable pen values vs palette fidelity.** Standard slot `0` is black and slot `7` is white in the maintained palette, but custom/past palette changes and RGB appearance remain edition-dependent. Setup can choose slot values for portability, while exact appearance still needs backend validation.

## Conflicts and Counterevidence

- The help for `CLEAN` says position, heading, pen mode, “etc.” are unchanged. It is corroborated by the C implementation. The help for `CLEARSCREEN` is brief and could be misread as changing no other turtle state; this report's implementation-backed distinction is the safer description.
- The first 16 pen/background color assignments are consistent in the local documentation, and current Web/wx source uses slot `7` for pen and `0` for background at initialization. The exact pen-size initial state has a source discrepancy: Web JavaScript starts a stroke at size `1`, while C `PENSIZE` reads C fields that `webgraphics_init` does not set. This is not evidence that `PENSIZE` is safely predictable without setup.
- `startup.lg` is loaded by native startup code and could mutate graphics state; the Web startup file in this source contains procedures and messages rather than a verified drawing-state setup. A truly pristine session must therefore exclude prior Logo or startup mutations.
- Runtime and visual equivalence were not checked. Static code can prove state assignments but cannot prove that every backend paints, clears, or displays those states identically.

## Constraints and Dependencies

- This task used only local repository files and read-only Git metadata. It ran no Logo program, build, browser, TestCafe, or wx session.
- The Web Edition depends on SVG/CSS, viewport, browser zoom, and palette variables. The wx/native editions depend on host window and display state.
- Pen patterns and reverse/erase behavior are more platform-dependent than the basic pen state fields. The report does not assert pixel-identical erase or reverse behavior.
- Startup files and prior procedures can invalidate any inferred pristine state. The plan should treat “default” values as initialization sources, not as session invariants.

## Unknowns and Unverified Items

- Actual Web, Node, X11, and wx startup values for `PENSIZE` were not executed. The source-backed C/JS size discrepancy remains the principal initialization unknown.
- Actual browser/wx `CLEARSCREEN` behavior after hiding/showing the turtle was not visually observed.
- The exact effect of custom Web turtle sprites, pen patterns, RGB palettes, and printer/background combinations was not runtime-tested.
- The report did not establish whether Web `SCREENMODE`/layout state remains consistent after `prepare_to_draw` in every browser viewport; source evidence shows a possible split-layout side effect.
- No independent external UCB Logo implementation was used; external lookup was outside scope.

## Search Coverage and Limitations

Coverage included the bundled help index/files, `usermanual`, `docs/ucblogo.texi`, primitive registrations, shared graphics implementation, Web adapter/SVG source, wx adapter source, startup loading, and the existing `logolib/pen` and `logolib/setpen` state bundles. Principal terms were `clearscreen`, `clean`, `pencolor`, `pensize`, `penup`, `pendown`, `penmode`, `background`, `hideturtle`, `showturtle`, `shownp`, `position`, `heading`, `shape`, `screenmode`, and platform clear functions.

The 2008 help text and 2026 implementation are intentionally cross-checked rather than treated as equally current. Source commits were recorded through local `git log`. No external sources or runtime-generated evidence were available in this task.

## Planning Handoff

The planning agent should ensure the future turtle-graphics skill states:

- `CLEAN` leaves position, heading, pen state, visibility, and background unchanged.
- `CLEARSCREEN` adds position/heading reset to the clear, but is not a global graphics-state reset.
- `CLEARSCREEN` and `CLEAN` preserve pen up/down, pen mode, pen color, pen size, background, and user-selected visibility; they do not change these fields.
- Drawing setup should explicitly set every relevant state; a sample setup decision should cover color, size, down/up and mode, visibility, background, and, if needed, position/heading.
- Pristine-session examples should note that startup files or prior procedures can mutate state and that a Web/wx initial pen-size claim needs runtime corroboration.
- Platform notes should separate graphics clearing from background clearing and screen layout actions.

No implementation, plan edit, test work, commit, or visual gate was performed. Runtime claims remain dependent on later browser/wx validation.

## Sources

- [`helpfiles/clearscreen`](../../helpfiles/clearscreen), [`helpfiles/clean`](../../helpfiles/clean), [`helpfiles/setpencolor`](../../helpfiles/setpencolor), [`helpfiles/setpensize`](../../helpfiles/setpensize), [`helpfiles/pendown`](../../helpfiles/pendown), [`helpfiles/penup`](../../helpfiles/penup), [`helpfiles/penpaint`](../../helpfiles/penpaint), [`helpfiles/penerase`](../../helpfiles/penerase), [`helpfiles/penreverse`](../../helpfiles/penreverse), [`helpfiles/setbackground`](../../helpfiles/setbackground), [`helpfiles/hideturtle`](../../helpfiles/hideturtle), [`helpfiles/showturtle`](../../helpfiles/showturtle) — local primitive help.
- [`usermanual`](../../usermanual) and [`docs/ucblogo.texi`](../../docs/ucblogo.texi) — historical and maintained primitive documentation.
- [`graphics.c`](../../graphics.c), [`init.c`](../../init.c), [`turtleshape.c`](../../turtleshape.c) — shared state, primitive setters, clearing, and shape/mode defaults.
- [`webgraphics.c`](../../webgraphics.c), [`webgraphics.h`](../../webgraphics.h), [`pre.js`](../../pre.js) — Web C/JS clearing, background, visibility, and pen initialization.
- [`wxGraphics.h`](../../wxGraphics.h), [`wxTurtleGraphics.cpp`](../../wxTurtleGraphics.cpp) — wx clearing, layout, background, and pen initialization.
- [`main.c`](../../main.c) and [`webstartup.lg`](../../webstartup.lg) — startup loading context.
- [`logolib/pen`](../../logolib/pen), [`logolib/setpen`](../../logolib/setpen) — library-level state capture/restoration patterns.

<!-- Co-authored by GitHub Copilot. Scope: turtle drawing-state research. Model: MAI Code 2 Flash. Reasoning effort: max. Runtime metadata beyond the assignment: unverified. -->
