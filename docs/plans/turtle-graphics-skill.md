# Project Plan: Turtle Graphics Skill

Status: Draft; implementation not started.
Date: 2026-11-20

Checkpoint: Research paused; independent QA and stage approval pending.
See [resume state](../workflow-runs/turtle-graphics-skill-checkpoint-2026-11-20.md).

## Objective

Create a repository skill for user-facing UCB Logo turtle graphics work, with
practical guidance for programs, demos, primitives, and common pitfalls on both
the Web Edition and wxWidgets Edition.

## Scope

The skill will cover:

- Turtle motion and heading primitives.
- Pen position, mode, size, pattern, color, and palette use.
- Turtle, window, screen, shape, label, fill, and mouse-related operations.
- Saving and restoring turtle state in procedures and demos.
- Animation redraw patterns and Web Edition sprite basics.
- Platform differences that affect Logo programs and demonstrations.

## Platform Targets

- Web Edition
- wxWidgets Edition

## Overlap With Existing Skills

The draft will reuse and link to existing material rather than duplicate it:

- `ucb-logo-copilot-assisted-coding` for rapid Logo prototyping and live checks.
- `ucb-logo-programming` for Logo style and Web runtime conventions.
- `ucb-logo-web-frontend` for SVG and graphics rendering internals.
- `ucb-logo-interpreter-development` for shared and platform graphics internals.

The new skill will not repeat SVG callback or C/JS implementation details.
Those remain the responsibility of the existing specialized skills.

## Implementation Steps

1. Create `.github/skills/ucb-logo-turtle-graphics/SKILL.md` with YAML
   frontmatter, trigger phrases, a scope statement, and links to references.
2. Add `references/turtle-primitives.md` for motion, pen, color, shape, window
   modes, labels, fills, and turtle/window queries.
3. Add `references/turtle-patterns.md` for demo idioms, state restoration,
   cleanup, animation redraw, and sprite basics.
4. Add `references/platform-notes.md` for user-level Web and wxWidgets
   differences, with internal implementation links deferred to existing skills.
5. Verify primitive names against `helpfiles/` and `usermanual` before writing.

## Verification

- Check that all relative skill and reference links resolve.
- Confirm primitive names and spellings against `helpfiles/` and the manual.
- Validate the YAML frontmatter and skill file structure.
- No application build or browser test is required for a documentation-only
  draft.

## Excluded From This Draft

- Editing existing skills.
- Changing shared graphics or rendering internals.
- Build or test execution.
- Application-code changes.
- Commit or push operations.

Exception: the user explicitly requested a local checkpoint commit on
2026-11-20. This saves unfinished work; it does not approve implementation
or authorize a push.

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash. Reasoning effort: unavailable. -->
