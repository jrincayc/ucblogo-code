---
name: ucb-logo-turtle-graphics
description: "Write and explain UCB Logo turtle drawings, with state-first setup, primitives, reusable motion and pen patterns, and Web Edition versus wxWidgets portability notes."
argument-hint: "Describe the drawing, reusable turtle primitive, or Web/wx portability question."
---

# UCB Logo Turtle Graphics

Use this skill for user-facing Logo drawing, procedural geometry, pen and turtle state, labels and fills, and Web Edition or wxWidgets platform boundaries. Use `ucb-logo-programming` for general Logo syntax and `ucb-logo-web-testing` for browser test execution.

## Start state-first

Before a drawing, establish every mutable setting it depends on. Do not rely on `clearscreen`, a startup file, or earlier work for reproducible state:

- Set position and heading with `setpos`/`setxy` and `setheading` when absolute placement matters.
- Set pen size and color explicitly. Set mode and up/down independently; `penpaint` sets paint mode and puts the pen down, while `pendown` changes only position state.
- Set background, turtle visibility, shape, screen mode, wrap/fence/window mode, and scrunch when relevant.
- Treat `clean` as erase without moving or changing other state. Treat `clearscreen`/`cs` as erase plus home to `[0 0]` heading `0`. Neither tool is a universal reset.
- Do not use `pen`/`setpen` as a complete drawing-state snapshot: they cover a pen bundle, not position, heading, visibility, background, shape, wrap/fence/window mode, scrunch, or palette.

Use the Web Edition references as the default target and keep wx differences explicit.

## Load focused guidance

- [Primitive names and classifications](./references/turtle-primitives.md)
- [Reusable drawing patterns](./references/turtle-patterns.md)
- [Web and wx platform notes](./references/platform-notes.md)

## Workflow

1. Identify the portable drawing core and edition-specific needs.
2. Check names and aliases in the primitive reference before teaching or using them.
3. Establish state in the procedure or complete example rather than assuming a pristine session.
4. Prefer small named procedures, local variables, and state saved/restored around reusable procedures.
5. Use `clean` or `clearscreen` deliberately; label examples where layout may change.
6. Mark complete standalone Logo examples as fenced `logo` blocks. Put syntax fragments in another fence language or label them as non-runnable.

For complete runnable examples, see [Reusable drawing patterns](./references/turtle-patterns.md). For source-backed availability and unverified rendering behavior, see [Platform notes](./references/platform-notes.md).

<!-- Co-authored by GitHub Copilot. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
