---
name: ucb-logo-copilot-assisted-coding
description: "Rapidly prototype and iterate on UCB Logo programs with Copilot in the Web Edition, from live experiments to saved source files. Use when: prototyping Logo programs, quick browser checks of Logo code, iterating on a demo or procedure, turning a live Logo session into repository files, choosing between prompt-level experiments and formal gates, or deciding when to switch from rapid prototyping to full validation."
argument-hint: "Describe the Logo program or idea you want to prototype, and whether it should be kept."
---

# UCB Logo Copilot-Assisted Coding

Support fast, interactive Logo prototyping with Copilot. Optimize for a short loop: write or change a procedure, run it in the Web Edition, look at the result, and adjust. Keep formal gates (workflow run records, stage approvals, QA passes) out of the loop until the user asks to harden the work.

Language semantics, arity, `runjs`, and Unicode rules belong to the [programming skill](../ucb-logo-programming/SKILL.md). Build, deploy, and test commands belong to the [build-deploy](../ucb-logo-build-deploy/SKILL.md) and [web-testing](../ucb-logo-web-testing/SKILL.md) skills. This skill covers the workflow between them.

## Modes

- **Rapid prototyping** (default when the user is exploring): the rules in [project instructions](../../copilot-instructions.md) apply, including the prototyping exception. The embedded browser is allowed for quick Logo checks. Skip workflow-run records, stage approvals, and QA passes for prototype edits and browser checks.
- **Hardening**: switch when the user asks for tests, a PR, or a release. Then apply the normal browser priority, test matrix, and commit policy.
- Stop prototyping when the user says so. Do not carry prototype shortcuts into hardened work without re-running the relevant checks.

## Iteration Loop

1. **Reuse the environment.** Check for a running `emrun` server and an open browser page on the target build before starting new ones. Track terminal IDs and ports for anything started.
2. **Wait for readiness.** Confirm Logo is initialized and the command input is enabled before sending commands.
3. **Try the smallest change.** Define or redefine one procedure, run it at the prompt, and inspect output, rendered HTML or graphics, and console errors.
4. **Check for stale state.** If a result contradicts the source, use the [stale-artifacts checklist](../ucb-logo-web-testing/references/stale-artifacts.md) before editing again. Live definitions persist in the session; reload or restart the runtime when they may be stale.
5. **Keep what matters.** Experiments that the user will want later go into a Git-tracked source file. Throwaway probes do not need to be saved.

## Keeping Prototypes

- Prototype code is worth keeping by default. Save it to a workspace file (for example, under `contrib/` for demos or `tests/` for tests) and note what it does.
- Reconcile live Logo edits with the file without overwriting unrelated changes. Reload or rerun the saved source to confirm it still works.
- Commit each meaningful checkpoint locally, per the commit policy. Stage only the files you touched.
- Do not edit generated `ucblogo.html`, `sw.js`, or `ucblogo.js` to fix source behavior.

## Asking Copilot for Help

- Give the goal, the expected output, and the audience or dialect constraints. Mention whether the code should run in Web Edition only or also in wxWidgets.
- Ask for a small first version, then extend it. Prefer named procedures with clear inputs.
- When a request needs a C change, describe the desired behavior and hand off to the [interpreter-development skill](../ucb-logo-interpreter-development/SKILL.md) rather than changing C during prototyping.

## Handoff

When the prototype works and the user wants it kept, summarize:

- What the program does and how to run it in the Web Edition.
- Which files hold the source and which checks were run, in which browser.
- Anything left unverified (for example, mobile Safari or wxWidgets).

Then ask whether to move to hardening.
