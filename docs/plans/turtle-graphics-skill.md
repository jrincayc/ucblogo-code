# Plan: UCB Logo Turtle Graphics Skill

Status: Revised Plan draft; prior Plan approval reopened by the user for browser execution of runnable documentation examples. Independent Plan QA and human approval pending.
Task: TGS-2026-11-20-P1
Date: 2026-11-20
Base revision: `6a35fb2c3ed110ac7b1e25a07858ceff629a3370` on `wasm`.
Current stage: Plan (reopened after user-directed acceptance-criteria change).
This assignment authorizes this plan edit only; it does not authorize test
implementation, documentation authoring, commits, or a stage advance.

## Research Approval

Research was approved by the user on 2026-11-20. These exact approved reports
are the evidence base for this plan:

- [Turtle drawing state](../research/turtle-drawing-state-2026-11-20.md), SHA-256
   `1a2894ebe71a10bf54e528d72a4c9414226dd7025204ddd579a859f69d445387`.
- [Turtle primitives and patterns](../research/turtle-primitives-patterns-2026-11-20.md),
   SHA-256 `9c38af4ff6ad25dae33a12660343b3f5537e57531a70c935f2e66d97333dc69c`.
- [Turtle platform findings](../research/turtle-platform-2026-11-20.md), SHA-256
   `dcda205b538090f980a49ef9c55412c593998a2e6f0a547efcf6b86333563e85`.

The [workflow checkpoint](../workflow-runs/turtle-graphics-skill-checkpoint-2026-11-20.md)
records the Research approval and the metadata-verification deviation. It does
not record approval of this Plan; independent Plan QA and human approval remain
pending.

## Objective

Create a focused, user-facing skill for writing and understanding UCB Logo
turtle drawings. Default to the Web Edition when the user's target is
unspecified, teach the shared Logo drawing core first, and provide concise
wxWidgets portability notes. Deliver one skill entry point and three focused
references; do not change the interpreter, application, or existing skills.

## Scope And Ownership

The new skill owns practical Logo drawing guidance: primitive lookup, explicit
state setup, reusable drawing patterns, and user-observable edition notes. It
complements rather than replaces existing ownership:

- `ucb-logo-programming` retains general Logo syntax, style, and programming
   guidance.
- `ucb-logo-copilot-assisted-coding` retains rapid live prototyping workflows.
- `ucb-logo-web-testing` retains browser/runtime testing procedures.
- `ucb-logo-web-frontend` retains Web UI, SVG, and rendering implementation
   guidance.
- `ucb-logo-interpreter-development` retains interpreter, C/JS bridge, and
   graphics implementation guidance.

The documentation package has four files. A separate TestCafe fixture is also
authorized to execute the actual runnable Logo examples in those documents;
its ownership and validation are staged separately:

| Deliverable | Sole future file owner | Responsibility |
| --- | --- | --- |
| `.github/skills/ucb-logo-turtle-graphics/references/turtle-primitives.md` | Primitive-reference author | Current turtle, motion, pen, palette, screen, shape, label, fill, and related primitive names, aliases, and classifications. |
| `.github/skills/ucb-logo-turtle-graphics/references/turtle-patterns.md` | Patterns-reference author | State-first drawing setup, composition, cleanup, state scope, and carefully bounded pattern guidance. |
| `.github/skills/ucb-logo-turtle-graphics/references/platform-notes.md` | Platform-reference author | Web-default and wxWidgets user-visible differences, current dimensions caveat, and explicit runtime unknowns. |
| `.github/skills/ucb-logo-turtle-graphics/SKILL.md` | Skill-entry author | Concise discovery surface and workflow integrating the three accepted references. |
| `tests/acceptance/turtle-graphics-examples.testcafe.js` | Example-test author | Extract actual runnable `logo` fences from the four documents and execute each in the Web Edition browser runtime. |

The Orchestrator creates fresh direct assignments and owns branch integration
and commits. The proposed authors do not edit one another's files, launch or
delegate to agents, stage, or commit. The three reference tasks may run in
parallel after the later Test Plan and Implement Tests gates. The Orchestrator
must keep all roles and workstreams under the hard cap of eight active agent
instances total; eight is a ceiling, not a target. Each reference is an
independent non-code artifact and requires its own Development QA PASS before
dependent integration. The SKILL.md task depends on all three accepted
references and their QA PASS; it owns only SKILL.md. No assignments are made
by this Plan task.


## Shared Content Contract

All four authors use the same approved state and naming contract so that the
reference files and entry point agree:

1. Before drawing, inspect or explicitly establish every mutable setting the
    drawing depends on. Rely on a value only when the session is known pristine
    or the value was deliberately set recently with no intervening mutation.
    Cover, as relevant, pen color and size; pen mode and pen up/down position;
    turtle visibility; background; absolute position and heading; wrap, fence,
    or window mode; scrunch; shape; and palette changes.
2. Teach pen mode separately from pen up/down: `PENDOWN` sets the pen down but
    does not choose paint mode; `PENPAINT` sets paint mode and also sets the pen
    down. Examples that require both must establish both deliberately.
3. Describe `CLEARSCREEN` (`CS`) as erasing and homing position and heading,
    not as a global drawing-state reset. It preserves pen mode and up/down,
    color, size, background, visibility, shape, wrap/scrunch, and other relevant
    settings. Describe `CLEAN` as erasing without changing position or heading;
    it is not a full reset either.
4. Describe `PEN` and `SETPEN` as a pen-state bundle, not a complete turtle,
    screen, or palette snapshot. They do not by themselves preserve position,
    heading, visibility, background, shape, wrap/scrunch, screen mode, or palette.
5. Say that the current Web logical canvas is 640 x 480 and wx canvas bounds
    follow the host window. Mark this as current implementation behavior that
    may change, not a permanent coordinate contract. Do no additional dimension
    research for this work.
6. Use names supported by current registrations, help, library definitions, or
    Web startup wrappers, and label their source/classification. Do not teach
    stale or unregistered spellings such as `setturtle`, `setturtleshape`,
    `setshaperendering`, `animationframact`, or `animationdelay` as Logo words.
7. Distinguish source-level availability from runtime-tested appearance. Do
    not claim Web/wx rendering parity for shape, fill, erase/reverse, labels,
    palettes, scrunch, mouse input, or animation when the evidence is
    unverified or conflicting. No runnable sprite demo is a deliverable; do not
    promise one without a separately approved validation plan and evidence.
8. Mark each complete, standalone example intended for execution with a fenced
   `logo` code block. The browser test harness extracts and runs those exact
   blocks from the documentation; do not duplicate example source in the
   harness. Label non-runnable fragments distinctly and exclude them from the
   runnable-example set.

## Proposed Work Breakdown

These are bounded future assignments, not work authorized in the current Plan
stage. The shared content contract above is included in each assignment to
prevent conflicting claims while preserving separate file ownership.

### Primitive Reference

Own only `references/turtle-primitives.md`. Organize canonical spellings and
aliases by drawing task, distinguish primitive words from library procedures
and Web startup facilities, and cite the appropriate local help/manual/source
evidence. Include state mutators and queries, palette caveats, and the
`CLEAN`/`CLEARSCREEN` distinction. Acceptance: every taught name is source- or
help-backed and correctly classified; unsupported names and parity claims are
excluded or explicitly identified as unknown.

### Patterns Reference

Own only `references/turtle-patterns.md`. Explain repeatable setup before
drawing, coordinate/heading and pen-up movement patterns, procedure-local
state, cleanup choice, and what a pen snapshot omits. Keep any Web animation or
sprite discussion clearly edition-specific and evidence-bounded. Acceptance:
guidance observes the shared content contract, does not rely on an implicit
pristine session, and contains no unvalidated runnable sprite promise.

### Platform Reference

Own only `references/platform-notes.md`. Present the Web Edition as the default
target and wx as a portability comparison. Include the current-but-variable
canvas distinction and source/documentation conflicts without attempting to
resolve them through presumed visual parity. State runtime unknowns directly;
do not spend additional research on dimensions. Acceptance: current facts,
documentation conflicts, and unverified rendering behavior are distinguishable
to the reader.

### Skill Entry And Reference Integration

After all three references pass independent QA, own only
`.github/skills/ucb-logo-turtle-graphics/SKILL.md`. Add valid YAML frontmatter
whose `name` matches the folder and whose concise description contains useful
drawing-related discovery phrases. Keep the entry point procedural and
progressively loaded: when to use it, Web default/wx note, state-first
workflow, and relative links to the three references. Follow the loaded skill
authoring guidance: one-level relative reference links and fewer than 500
lines. Acceptance: all links resolve, the state contract is visible without
duplicating the reference articles, and existing skill ownership remains
clear.

## Dependencies And Integration Order

1. Complete this Plan deliverable, obtain independent Plan QA, then wait for
    explicit human approval before entering Test Plan.
2. In Test Plan, define exact static content checks, commands, expected results,
    regression scope, and whether any reusable fixture is appropriate. No
    application behavior changes are planned; the Test Plan should explicitly
    justify why no fixture data is needed if none is useful. Do not implement
    tests in that stage.
3. Complete the approved Implement Tests stage before documentation authoring.
   Implement only the separately authorized TestCafe fixture at
   `tests/acceptance/turtle-graphics-examples.testcafe.js`; the documentation
   itself remains the sole source of Logo example code.
4. In Implement Changes, directly assign the three independent reference
    authors as fresh tasks after checking the active-instance cap. Each writes
    only its owned file. Run independent Development QA on each non-code
    reference; do not start dependent integration until all three pass.
5. Directly assign a fresh SKILL.md author after the reference QA passes. QA
    the entry point as its own non-code deliverable. The Orchestrator integrates
    accepted files and owns any commit; no worker commits under this plan.
6. Submit the exact four-file documentation diff and static validation evidence
    for the applicable aggregate stage QA and human checkpoint. No stage is
    approved by this Plan.

## Acceptance Criteria

The Plan is ready for its gate when it defines the exact deliverables, bounded
file ownership, dependencies, integration/QA order, acceptance checks,
verification strategy, rollback, exclusions, risks, and visual-impact status.
The future skill package is acceptable only when all of the following hold:

- Exactly the three reference files and `SKILL.md` are created for the skill;
   the only additional task-owned file is the separately authorized TestCafe
   fixture. No application, other test, existing skill, manual, or research
   file is changed.
- The skill is for user-facing Logo drawing, defaults to Web when unspecified,
   and supplies wx portability notes without taking over neighboring skills'
   responsibilities.
- The references consistently cover explicit state setup, pristine/recently
   set assumptions, independent pen mode and up/down state, `PENPAINT` setting
   the pen down, relevant color/size/visibility/background/position/heading/
   wrap/palette/scrunch/shape, and the actual `CLEAN`/`CLEARSCREEN` distinction.
- The references state that `PEN`/`SETPEN` do not capture all turtle, screen,
   and palette state; no example relies on them as a full snapshot.
- The platform notes present 640 x 480 Web and host-dependent wx bounds only as
   current behavior that could change; no further dimension research is done.
- Names, aliases, and platform classifications are checked against the
   approved research and local evidence; stale/unregistered names are not
   presented as Logo commands.
- Source-backed feature availability is not described as cross-platform visual
   equivalence. Runtime unknowns and stale documentation conflicts remain
   explicit; no runnable sprite demo is promised without later approval and
   validation.
- YAML frontmatter and all relative links are valid; the skill body stays
   within the skill authoring length guidance.
- Every complete runnable example is fenced as `logo`, extracted from its
   owning document at test time, and actually executed in a headless Chrome
   browser session using the Web Edition runtime. Each example runs in a fresh
   runtime to prevent state leakage; no Logo commands are executed by Node.
- A browser test report identifies each example by source document and fence,
   reports its result, and references a captured PNG screenshot for every
   example. Failed examples retain diagnostic output and failure screenshots.
- Existing cross-browser functional coverage is not repeated by this focused
   test; the executable-document-example fixture runs only in desktop headless
   Chrome. Mobile Chrome, device emulation, and other browsers are excluded.
- Static checks and the focused browser example test pass against fresh Web
   Edition artifacts. No wx runtime or visual-design change is in scope.

## Verification Strategy

This Plan stage uses static checks only: verify required plan sections and
approved report links/hashes, and run `git diff --check` scoped to this file.
The later documentation package uses local static checks for frontmatter,
links, primitive names, and state/portability statements. It also executes all
complete fenced `logo` examples extracted directly from the four documents in
headless Chrome through the existing Web Edition acceptance runner. Capture a
screenshot after every example and include the screenshot path in a per-example
report entry. Keep the existing browser runner's readiness, watchdog, and
owned-process cleanup; do not launch raw Chrome or run Logo source in Node.
Use a known-fresh Web build and report its identity. The Test Plan stage owns
the exact fixture protocol, commands, completion/error assertions, screenshot
paths, report schema, and expected results. Existing cross-browser graphics
coverage is sufficient; this focused fixture does not repeat it.

## Visual Impact

No product UI, layout, typography, color, imagery, animation, responsive
behavior, or interaction changes are proposed. This plan adds browser
execution and screenshots solely as evidence that the actual documented Logo
examples run and produce turtle output; screenshots are linked in the test
report. A visual-design review is not applicable because no interface design
changes are planned. Runtime screenshots are not evidence of wx or other
browser parity.

## Exclusions

- No changes to application/interpreter code, generated assets (except ignored
   test report artifacts), unrelated tests, help, manuals, demos, existing
   skills, or research reports.
- No Node Logo execution, wx runtime testing, mobile Chrome, device emulation,
   other-browser rerun, product UI visual review, install, network, or
   dependency operation. The only Logo execution is in the desktop headless
   Chrome Web Edition acceptance fixture.
- No claims that unverified Web and wx rendering behavior is equivalent.
- No further canvas-dimension research and no promise of a runnable sprite demo.
- No agent launch or assignment by this task, staging, commit, or push.

## Risks And Mitigations

- **Source is not visual proof:** feature registration does not establish
   rendering parity. Keep availability, source observations, and runtime
   unknowns distinct; avoid equivalence claims.
- **Startup and prior session state:** startup files or earlier commands can
   mutate defaults. Teach state inspection/setup before drawing and avoid
   unqualified pristine-session assumptions.
- **Conflicting or stale names and docs:** use the approved name inventory and
   current source evidence; call out unresolved discrepancies instead of
   repeating stale manual or demo spellings.
- **Cross-reference drift:** separate authors could use inconsistent names or
   reset assumptions. Give each the Shared Content Contract, isolate file
   ownership, QA references separately, and integrate SKILL.md only after they
   pass.
- **Canvas dimensions may evolve:** label the current Web/wx distinction as
   implementation-specific and avoid hard-coding it as a long-term guarantee.
- **Unobservable model metadata:** requested model/effort verification remains
   unverified as recorded in the checkpoint; report this deviation without
   claiming runtime identity or treating it as evidence of actual availability.

## Rollback

This Plan revision edits only the plan and performs no commit. If Plan QA
requests changes, revise only this file. During Implement Tests, the owner may
correct only the new fixture and remove only its task-owned report artifacts
after checking for subsequent edits. During documentation work, each owner may
correct or remove only its own unintegrated file. Preserve unrelated user
changes. The Orchestrator owns integrated rollback and must target only the
accepted test fixture, documentation, and generated report artifacts. Do not
reset or revert the whole worktree. A defect discovered after QA must stop
dependent integration and return the affected artifact to QA.

## Open Decisions

No additional research question blocks this Plan. Initial Web pen-size output,
exact visual parity for several graphics operations, and behavior across all
browser/wx environments remain unverified research/runtime limits, not claims
to settle here. The detailed example extraction protocol, browser readiness and
error grading, screenshot/report schema, exact commands, and static cases
belong to the Test Plan stage. The approved runnable-example scope excludes
new sprite demos; any additional deliverable reopens scope with the
Orchestrator.

<!-- Co-authored by GitHub Copilot. Requested model: GPT-6 Luna; active model and reasoning-effort metadata: unverified. -->
