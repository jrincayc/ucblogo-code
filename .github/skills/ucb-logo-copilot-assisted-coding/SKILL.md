---
name: ucb-logo-copilot-assisted-coding
description: "Rapidly prototype and iterate on UCB Logo programs with Copilot in the Web Edition, from live experiments to saved source files. Use when: prototyping Logo programs, quick browser checks of Logo code, iterating on a demo or procedure, turning a live Logo session into repository files, choosing between prompt-level experiments and formal gates, or deciding when to switch from rapid prototyping to full validation."
argument-hint: "Describe the Logo program or idea you want to prototype, and whether it should be kept."
---

# UCB Logo Copilot-Assisted Coding

Support fast, interactive Logo prototyping with Copilot. Optimize for a short loop: write or change a procedure, run it in the Web Edition, look at the result, and adjust. Keep formal gates (workflow run records, stage approvals, QA passes) out of the loop until the user asks to harden the work.

Language semantics, arity, `runjs`, and Unicode rules belong to the [programming skill](../ucb-logo-programming/SKILL.md). Build, deploy, and test commands belong to the [build-deploy](../ucb-logo-build-deploy/SKILL.md) and [web-testing](../ucb-logo-web-testing/SKILL.md) skills. This skill covers the workflow between them.

Before relying on a primitive’s syntax or usage in a prototype, look it up in the repository help files. Run the bundled Logo word lookup script from the repository root:

```sh
python3 .github/skills/ucb-logo-help/scripts/get_help.py -- "print"
```

Replace `print` with the Logo word being checked. The lookup is case-insensitive and reads `helpfiles/<word>` directly; report that no help file exists if the script does so. Use the full text returned by the script rather than inferring or summarizing the word’s behavior.

For syntax, parsing, evaluation, or language-behavior questions that the help file does not answer, search the repository manuals before drafting Logo code:

```sh
rg -n -i -C 4 "topic or exact syntax" usermanual docs/ucblogo.texi
```

Replace the search expression with the relevant word or phrase. Start with `usermanual` for the historical/dialect description and `docs/ucblogo.texi` for the maintained Texinfo source; use both for comparison. Attribute findings to the specific manual section or passage. If the sources disagree or a manual appears to lag current behavior, verify against the current implementation and tests rather than silently choosing one. The repository manual text is documentation, not executable authority over the current interpreter.

For advanced Logo problems, use Brian Harvey’s *Computer Science Logo Style* books as additional conceptual references:

- [Volume 1: Symbolic Computing](https://people.eecs.berkeley.edu/~bh/v1-toc2.html) — exploration, procedures, functional programming, recursion, planning, and debugging.
- [Volume 2: Advanced Techniques](https://people.eecs.berkeley.edu/~bh/v2-toc2.html) — iteration and control structures, extensibility, and advanced programming techniques.
- [Volume 3: Beyond Programming](https://people.eecs.berkeley.edu/~bh/v3-toc2.html) — programming-language design tradeoffs and broader uses of Logo.

Treat these books as public educational and historical sources, not as a replacement for the repository manual or current implementation. Cite the relevant volume and chapter or section for a substantial design claim, and distinguish a historical preference or example from a documented requirement of the current dialect.

## Modes

- **Rapid prototyping** (default when the user is exploring): the rules in [project instructions](../../copilot-instructions.md) apply, including the prototyping exception. Default to the VS Code embedded/integrated browser for the live Logo Web Edition. Use an external browser only when the user requests it or embedded browser access is unavailable. Skip workflow-run records, stage approvals, and QA passes for prototype edits and browser checks.
- **Hardening**: switch when the user asks for tests, a PR, or a release. Then apply the normal browser priority, test matrix, and commit policy.
- Stop prototyping when the user says so. Do not carry prototype shortcuts into hardened work without re-running the relevant checks.

## Using the VS Code Embedded Browser

For manual rapid iteration, open the target URL in VS Code's built-in **Simple Browser** rather than launching a separate Chrome window:

1. Start or reuse the existing Web Edition server and record its URL.
2. Open the Command Palette and run **Simple Browser: Show** (`simpleBrowser.show`).
3. Enter the full `http://localhost:PORT/ucblogo.html` URL.
4. Reuse that Simple Browser tab while iterating; reload only for stale-runtime recovery or after a rebuild.

VS Code's Simple Browser is the embedded/manual-browser mechanism. Do not substitute Chrome DevTools, CDP, or an external browser merely because they are technically available when embedded-browser access is available. Use external Chrome or headless checks only when the user explicitly requests it or the embedded browser cannot be opened or controlled. Automated browser acceptance work belongs to [the web-testing skill](../ucb-logo-web-testing/SKILL.md), not this live iteration loop.

The shared Browser Pages attachment is the other way the host may expose an already-open page. Reuse its URL and keep that page open between iterations. If browser controls or page automation are unavailable, report that as an embedded-browser limitation and ask for the required host/browser access rather than silently falling back to a new external-browser workflow.

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

## Practical Iteration Notes

- Prefer the live browser's accessibility snapshot refs for command input and controls; do not guess selectors for hidden editor elements.
- Hide the turtle cursor with `ht` during drawing and restore it with `st` only if the final cursor state should be visible.
- Use polygon exterior angles explicitly: a regular hexagon turns 60 degrees per side.
- Center geometry by accounting for the turtle's starting state; reset or restore position and heading after helper procedures.
- Live procedure definitions survive browser/runtime sessions but not necessarily a reboot or reload. Treat prompt-only work as session-local until saved to a Logo source file and re-run from that file.
- After restarting a machine or runtime, verify the server process and served URL, then reload Logo and re-establish prompt readiness before resuming live definitions.
- The repository web server is JavaScript: start it with Node, not Python. Reuse a running server and keep the browser/server process owned and identified during iteration.

## Crash and Recovery

- At the start of rapid prototyping, enable `dribble` with a stable session filename (for example, `dribble "rapid-prototype-session.log`) and keep it enabled until the prototype is stopped or saved. `nodribble` closes it; do not close it during normal iteration.
- Keep the dribble transcript because it preserves keyboard input and terminal interaction for crash recovery. Because Logo's Web Edition starts in `/d`, relative dribble names persist in its virtual/IDBFS session storage.
- Mirror every live Logo edit into a workspace source file under `contrib/rapid-prototypes/` (or a closer feature-specific location) immediately after the edit. Mirror procedure definitions in valid Logo form and rerun `load` or re-define from the file as a stale-state check.
- On crash or reboot, preserve the existing browser transcript where possible, restore the server and embedded browser, verify prompt readiness, then recover the workspace mirror and relevant dribble transcript instead of reconstructing edits from memory.

## Stalled or Crashed Runtime Recovery

- If the Logo prompt stops accepting commands or the evaluator appears hung/crashed, do not reload or close the page before capturing the dribble file. Logo's Web Edition can retain an operational JavaScript/DOM context even when the Wasm evaluator is stalled.
- From the browser page, use the Emscripten filesystem API to persist and download the active transcript. The Web Edition stores its working directory in `/d`; for the default transcript use `/d/rapid-prototype-session.log`.
- First run `FS.syncfs(false, callback)` to flush the IDBFS-backed `/d` filesystem, then read `FS.readFile(path)` and create a downloaded text `Blob`. If `FS.syncfs` is unavailable or fails, preserve a fresh `FS.readFile` snapshot in memory and attempt direct download.
- Only after the dribble snapshot is secured should you reload, restart the task-owned runtime, or recover the workspace mirror. Keep the original browser/profile state when possible and do not clear IndexedDB or other site data merely to diagnose a hang.
