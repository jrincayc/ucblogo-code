# Turtle Graphics Skill Checkpoint

Date: 2026-11-20
Task: TGS-2026-11-20-R1
Stage: Research, paused; no independent QA or human stage approval recorded.
Base revision: 9a5023b259783e6e80d698ddf2e7b307d1c23fa1 (`wasm`).

## Saved Artifacts

- [Draft plan](../plans/turtle-graphics-skill.md): implementation not started.
- [Drawing-state research](../research/turtle-drawing-state-2026-11-20.md): author labels it Complete; independent QA pending.
- [Primitive and pattern research](../research/turtle-primitives-patterns-2026-11-20.md): Partial; known alias typo remains.
- [Platform research](../research/turtle-platform-2026-11-20.md): Partial; duplicate section remains.

## Requirements To Preserve

Many turtle commands mutate state. Before drawing, inspect or explicitly set
every mutable drawing setting the drawing relies on. Do not assume current
pen color, size, up/down, mode, turtle visibility, background, or other state
unless it is known from a pristine session or was deliberately established
recently without intervening mutation.

`clearscreen` erases graphics and homes position and heading; it is not a
general drawing-state reset. The drawing-state report contains source-backed
claims about retained settings, still awaiting independent review.

The current Web logical canvas is 640x480, whereas wx uses host-dependent
dimensions. The user expects dimensions may become variable in the future:
describe the current distinction without presenting it as permanent or
spending further research time on it. No dimension changes are authorized.

## Open Items

- Correct the primitive report's `pen?`/`pendown?` prose to match registered `pendownp`/`pendown?`; its alias table already uses the correct names.
- Consolidate the platform report's two `Options and Trade-offs` sections without losing useful content.
- Include the current-versus-future dimensions caveat when revising the platform guidance.
- Verify all three reports and obtain independent Research-stage QA, then explicit human approval before advancing to Plan.

The user requested GPT-6 Luna maximum-effort subagents for the two repairs.
Those repairs were not confirmed complete; timeout/network difficulties were
reported. No fallback model or successful repair is claimed here.

Workers may propose independent follow-on tasks to the orchestrator, who
assigns fresh workers. Workers must not launch agents themselves. Preserve
one file owner per parallel task and the active-instance cap.

## Checkpoint Authorization

The user's latest direction was `checkpoint; save state and commit`, following
a request to wrap up quickly because of time constraints and flaky network.
This local commit is an explicitly requested unfinished checkpoint, not a
QA-cleared deliverable. The normal independent pre/post-commit QA checks
have not been completed; this departure is recorded rather than claimed
as a pass. No stage advancement or push is authorized.

No application sources, generated assets, skill files, or tests were changed
for this checkpoint. No builds or browser checks were run. No task-owned
server or browser process is recorded; do not stop user-owned terminals.
The temporary ledger remains under `/tmp/copilot/TGS-2026-11-20-R1/`.

<!-- Co-authored by GitHub Copilot. Model identity and reasoning-effort setting unavailable. -->