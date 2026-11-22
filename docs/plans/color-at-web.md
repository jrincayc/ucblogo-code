# Plan: Web Edition `color.at`

## Objective and Scope

Implement issue #228's `color.at` operation in the Web Edition only. Given a Logo position, identify the topmost SVG drawing element at that logical point, read its computed color, and report a palette number when the color exactly matches a palette entry or an RGB list otherwise.

This packet implements only `color.at`. `color.under` is deferred. It also defers accurate polygon/text stroke-versus-fill hit classification. Native wx/X11/no-graphics behavior is unchanged; final broader acceptance testing may detect shared-code regressions.

The direct SVG/style inspection path is the only proposed mechanism. If it cannot satisfy an approved case, stop at Andon and request human review; do not add an alternative sensing mechanism inside this task.

## Approved Decisions and Assumptions to Confirm

- `COLOR.AT position` has arity 1.
- Position uses normal Logo turtle coordinates and follows `SETPOS`/`POS` conversion and validation behavior.
- Web logical drawing coordinates are the existing 640 by 480 SVG coordinate space.
- A palette-matching result uses the lowest matching palette index.
- A non-palette result uses the existing Logo RGB percentage representation.
- Out-of-bounds behavior remains a Plan-approval decision; the proposal is a normal Logo error rather than `-1`.
- The turtle overlay must not affect the result, and all turtle/display state must be preserved.
- A line element reports computed `stroke`; other supported drawing elements report computed `fill`. Accurate stroke-versus-fill hit classification for polygon/text geometry is explicitly out of scope.

## Execution Constraints

- Research is skipped by the user's task-scoped approval.
- The implementation worker pool is capped at one active worker.
- One worker owns the serial combined-TDD packet in one private worktree: tests, production code, documentation, refactoring, and focused validation.
- Independent review and aggregate QA remain required at the normal gates.
- No shell, build-system, script, or source-code string interpolation is permitted. Any necessary exception requires explicit human approval and a tested, approved quoting/escaping mechanism; otherwise raise Andon.
- Prefer file tools, fixed arguments, stdin, temporary files, and pipes over interpolated command or code strings.

## Test Plan Inventory

The later Test Plan must define exact inputs, expected values, setup, commands, artifact identity, and teardown for all following cases.

### Fast-fail fuse: first planned and first implemented

1. Create the smallest deterministic interior of a `FILLED` polygon in the real Web SVG drawing, query one interior point with `color.at`, and expect the configured palette index.
2. The test must also prove that the SVG element identified at the logical point exposes the intended computed fill color.
3. The initial red result must be specifically that `color.at` is undefined or unimplemented. A harness, build, browser, parser, stale-artifact, or setup failure is not valid red evidence.
4. Once the direct mechanism is wired, this single case must go green before any broader test or production work proceeds.
5. If the direct element/style path cannot identify the `FILLED` polygon interior, stop after preserving evidence and raise Andon. Do not continue by adding a speculative fallback.

### Remaining planned cases

- Background: a point with no drawing element returns the current background palette color.
- Line: a point on a line returns the line's computed stroke color.
- Arbitrary coordinates: x/y orientation and center conversion match normal Logo coordinates.
- Palette match: changed palette entries and duplicate RGB values return the lowest matching index.
- RGB fallback: a computed color absent from the palette returns the three-item RGB list.
- Overlap: the topmost supported drawing element wins.
- Turtle exclusion: a visibly shown turtle over the sampled point does not affect the result.
- State preservation: position, heading, pen, palette, screen mode, and turtle visibility remain unchanged.
- Malformed input: invalid nonnumeric or incorrectly shaped positions use normal Logo argument errors.
- Bounds: coordinates just inside and just outside the logical drawing area produce the approved result or error.
- Element rule: line uses stroke; non-line supported elements use fill; no polygon/text stroke classification is asserted.
- Repetition: repeated queries do not mutate the DOM drawing or Logo state.
- Documentation: primitive availability, limitations, and Web-only scope are correctly described.

## Combined-TDD Implementation Order

The Test Plan shall preserve the complete case inventory above before implementation starts. During Implement Changes under the approved combined-TDD deviation:

1. Implement and run only the fast-fail fuse first.
2. Preserve intended red evidence from the missing primitive.
3. Make the smallest C/JS bridge and direct SVG/style change needed to reach green.
4. Refactor only while that focused fuse remains green.
5. After the fuse passes, add the remaining planned cases and implementation in the smallest practical increments.
6. Run the focused check after each substantive edit.
7. Stop after three unsuccessful attempts on the same unexpected failure, and raise Andon rather than making a fourth attempt.
8. Record `Implement Tests: not run under approved deviation` at the workflow checkpoint; do not claim that stage passed.

The combined-TDD packet is a candidate bootstrap: implementation is test-first and the direct-SVG candidate strategy is fixed. Workers may not change strategy on the fly. Strategy changes require planning or explicit human direction. After the third unsuccessful focused attempt on the same issue, stop, preserve evidence, and ask for direction rather than changing strategy.

## Implementation Map

1. `init.c`: register only the Web Edition `color.at` primitive with arity 1 under `__EMSCRIPTEN__`.
2. `globals.h`: add the Web-only `lcolorat` declaration; do not add `lcolorunder`.
3. `webgraphics.c`: validate the position, convert Logo coordinates to logical SVG coordinates, bridge to JavaScript, match palette entries, and construct the final Logo NODE result.
4. `pre.js`: add one synchronous `Module.graphics` resolver that selects the topmost supported SVG element, excludes the turtle, reads computed stroke/fill according to the documented element rule, resolves current palette CSS variables, and returns primitive RGB/status data.
5. `tests/acceptance/`: add one focused browser fixture, beginning with the fast-fail `FILLED` polygon-interior case and then the remaining approved cases.
6. `webedition.md` and/or the authoritative help/manual source: document `color.at`, Web-only availability, the element-color rule, limitations, bounds behavior, and deferred `color.under`/stroke classification work.

No `shell.html` change is planned. Modify it only if a focused implementation review proves a direct-vector sensing seam is necessary and the change remains in scope.

## Validation

- Prove the fast-fail test fails only because `color.at` is missing.
- Prove the fast-fail test passes against a fresh Web build before broadening implementation.
- Run the complete focused Web fixture in desktop headless Chrome through the shared TestCafe runner.
- Rebuild C/JS assets before trusting browser evidence and record artifact/source identity.
- Run the agreed focused Web graphics/bridge regression scope.
- Do not run task-specific native builds; normal final acceptance remains responsible for broader cross-build breakage discovery.
- Run scoped prose/source whitespace checks and inspect the final diff.
- Obtain independent Reviewer coverage and aggregate Development QA against the exact final revision.

## Handoff Requirements

Before implementation, the approved packet must name the task ID, exact base revision, approved Plan and Test Plan revisions/hashes, private worktree path, allowed files, exact focused and regression commands, integration/review owners, and the task-specific combined-TDD approval.

No push is authorized. No implementation is authorized by this plan alone.

## QA Questions

The Plan QA review shall verify:

- all semantic and negative cases are planned before implementation;
- the first implementation case is the fast-fail `FILLED` polygon-interior/direct-style fuse;
- direct SVG/style inspection is the only planned mechanism;
- `color.under`, offscreen alternatives, and polygon/text stroke classification remain out of scope;
- the one-worker cap and no-interpolation policy are explicit;
- acceptance expectations, state preservation, bounds behavior, and artifact freshness are testable;
- no unnecessary native work or scope expansion is included.

<!-- Co-authored by GitHub Copilot. Scope: Web Edition color.at implementation plan. Model and reasoning-effort setting unavailable. Wall-clock time, energy, and CO2 unavailable for this planning-only draft. -->
