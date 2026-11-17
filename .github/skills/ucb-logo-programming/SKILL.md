---
name: ucb-logo-programming
description: "Write, test, review, explain, and document idiomatic UCB Logo programs for wxWidgets and Web Edition, including Logo/JavaScript integration. Use for live workspace development, arity and dialect semantics, test coverage, CSLS, runjs, Web APIs, HTML terminal output, usage patterns, demos, tutorials for developers of all levels, and backward-compatible improvements."
argument-hint: "Describe the Logo program, JavaScript integration, teaching material, usage pattern, or improvement you want."
---

# UCB Logo Programming

Collaborate with an expert UCB Logo developer on clear, idiomatic Logo programs, Logo/JavaScript integration, usage patterns, and learning materials for developers at all levels, targeting wxWidgets and Web Edition as appropriate. Respect Logo's interactive, exploratory character while applying useful ideas from functional and declarative programming. Preserve backward compatibility by default; make platform-specific capabilities explicit instead of disguising them as portable Logo.

Most users run Logo in a browser with access to JavaScript but no C development environment. When the target is unspecified, default to Web Edition and favor examples and extensions that work in the existing browser runtime without rebuilding the interpreter. Support wxWidgets and portable Logo when requested; clearly label browser-only capabilities and prerequisites.

Implement extensions in Logo or JavaScript, not C. Read C implementation code when needed to verify behavior. If a desired capability cannot reasonably be implemented in Logo or JavaScript, suggest the C or interpreter change needed, explaining the motivation, desired behavior, and compatibility implications, then leave its implementation to the user and the separate [interpreter-development skill](../ucb-logo-interpreter-development/SKILL.md).

For overlapping JavaScript work, distinguish user-facing Logo/JS libraries and examples from implementation of the interpreter's runtime bridge: the latter belongs to interpreter development. Logo-level regression tests and documentation may accompany a handoff without implementing the runtime fix.

Treat the user as an expert collaborator: discuss implementation constraints, semantics, tradeoffs, and evidence directly without introductory explanations unless requested. Adapt the depth and prerequisites of generated materials to their intended audience, which may range from beginners to experienced Logo developers; do not assume that audience is the user.

Support both traditional live-system development and source-file development. Logo's interactive workspace is valuable for exploration and pedagogy; the user also often prefers VS Code for syntax highlighting, Git, and editing source. Choose the workflow appropriate to the task and audience, and allow movement between them without treating either as obsolete.

## Workflow

1. **Identify the target and outcome.** Determine whether the request concerns a portable Logo program, wxWidgets behavior, Logo/JavaScript integration, Web Edition feature, tutorial/demo, usage pattern, extensibility improvement, or language-design proposal. For materials, identify the intended audience and prerequisites. Ask only when the target materially changes the design.
2. **Check the local dialect and implementation.** Search nearby `.lg` examples, CSLS programs, help files, tests, and the relevant implementation when needed to verify behavior. UCB Logo has its own syntax and semantics; do not assume Logo-like pseudocode or another dialect's primitives work here. Check whether a word is primitive, library-defined, or web-only.
3. **Choose the simplest fitting model.** Prefer small named procedures with clear inputs and `output`/`stop` behavior. Use lists as the ordinary sequence, arrays when indexed mutation or stable random access is useful, and higher-order tools such as `map`, `map.se`, `filter`, `reduce`, and `apply` when they make the transformation clearer. Use templates for higher-order operations and macros when caller-context evaluation or code generation is needed. Keep state local where practical; use mutation when it expresses the problem more clearly than forcing a functional form.
4. **Preserve Logo's strengths.** Favor readable procedure names, meaningful inputs, interactive experimentation, composable procedures, and examples that can be pasted into the Logo prompt. Avoid needless punctuation-heavy encodings, opaque one-liners, global state, and clever macro machinery without a concrete benefit. Respect established Logo conventions and explain any deliberate departure.
5. **Keep platform boundaries visible.** Start with a portable core if both wx and Web Edition are targets. Put browser APIs, DOM work, `runjs`, and HTML terminal output behind clearly named web-specific procedures. Use wx APIs only after checking the actual wx implementation. Do not claim a web-only word or behavior is portable; state any required runtime or browser support. For Unicode text, check wx and WebAssembly separately: wx input/output must convert explicitly between `wxString` and UTF-8, while JS/Wasm interop should use the existing Emscripten UTF-8 conversions. Do not assume UTF-8 bytes are Logo characters or that one code point is one displayed grapheme.
6. **For JavaScript interop,** keep the JavaScript expression trusted and stable, and pass Logo data through `runjs` arguments (`arg0`, `arg1`, etc.) rather than interpolating values into source text. Convert returned values deliberately: the web bridge serializes JS values into Logo-readable text, so verify how strings, arrays, objects, `null`, `undefined`, booleans, and promises arrive before relying on them. Check the current implementation and `webedition.md` for ASCII/Unicode and runtime constraints. `runjs` evaluates JavaScript and is not a security boundary; do not execute untrusted source.
7. **Use parenthesized vararg calls when needed.** In this dialect, variadic primitive calls in prefix form can require parenthesized invocation to pass nondefault arity reliably (for example, `(runjs "|expr| :a :b :c)`). Verify arity-sensitive call sites after refactors.
8. **Keep code/data channels separate in wrappers.** Web helper procedures in `webstartup.lg` should pass values as bound args to `runjs` (`arg0`, `arg1`, ...) instead of constructing JavaScript by quoting or concatenating Logo data.
9. **Preserve the existing runtime bridge contract.** Do not alter writer semantics from Logo wrappers; side-effect JS calls should still use `ignore` around `runjs` results when the return value is irrelevant.
10. **For HTML terminal output,** use the `htmlstring` primitive to mark intentional markup. The Web Edition terminal renders output as HTML, so treat markup as executable presentation: use trusted/static markup, escape untrusted text, or construct DOM nodes and assign untrusted values with `textContent`. Verify behavior in the actual web terminal and do not assume wx renders HTML the same way.
11. **Validate in the target runtime.** Run the narrowest relevant Logo test or demo. For shared programs, check both wx and Web Edition where available. For browser interop or rendered HTML, exercise the browser UI, not only Node; check console errors, input/output behavior, and mobile layout when relevant. Avoid unrelated fixes.
12. **Finish the learning surface.** When relevant, add or update a runnable example and explain its inputs, outputs, and assumptions at the intended audience's level. Put user-facing primitive documentation in the established help/manual locations when needed; keep Web Edition limitations and exclusive features accurate in `webedition.md`. Prefer a small tutorial that teaches a reusable idea over a feature tour.

## Desktop Iteration

- Create or edit the Logo source in VS Code. Put new demos and examples in `contrib/`, tests in `tests/`, and shared Web Edition platform helpers in `webstartup.lg` when appropriate; follow existing project conventions and avoid unrelated changes.
- Serve the existing web build with `emrun --no-browser --port PORT ucblogo.html`, selecting an available port, and open the URL in an external Chrome (never the VS Code embedded browser), or drive it headlessly and show screenshots. Reuse an existing suitable server when possible. Rebuild the app when needed to validate changes or obtain a usable build; inspect the build procedure and preserve existing work in progress. If `emrun` or the build tools are unavailable, state the blocker and use an available alternative where appropriate.
- After rebuilding, distinguish stale service-worker/browser content from source failures with the [web-testing skill's stale-artifacts checklist](../ucb-logo-web-testing/references/stale-artifacts.md). Do not edit generated `ucblogo.html` or `sw.js` directly to fix source behavior.
- Wait until Logo is initialized and the `logoInputText` terminal input is enabled. Send commands to Logo's stdin via that input or the existing `runLogo` JavaScript entry point, then run the relevant procedures and inspect Logo output, rendered HTML or graphics, and browser errors. For multiline source, the app's upload control copies a selected Logo file into the virtual filesystem and issues `load`; verify the current build's behavior before relying on another loading mechanism.
- Iterate interactively: debug at the Logo prompt and, when useful, edit definitions in Logo. The web editor saves to the browser's virtual filesystem, which does not automatically mirror workspace files. Assume generated Logo and JavaScript code, including live experiments, is potentially worth keeping; preserve it in suitable Git-ready workspace files by default, creating source files when needed. Reconcile live changes without overwriting unrelated edits, then reload or rerun the synchronized source to verify it. Trivial diagnostic probes or code the user explicitly does not want retained need not become repository files.
- Add focused tests for the behavior and report what was actually checked. Commit checkpoints per the repo commit policy.

## Repository Pointers

- Portable idioms and examples: `csls/`, `logolib/`, and `tests/UnitTests*.lg`.
- Web interop and startup helpers: `webedition.md`, `webstartup.lg`, `weblib.c`, and `pre.js`.
- HTML terminal example and output handling: `htmldemo.lg`, `term.c`, and `shell.html`.
- User-facing primitive documentation: `helpfiles/` and `docs/ucblogo.texi`; `usermanual` provides historical context and may lag current editions.
- Existing demos: `animationdemo.lg`, `animationdemo2.lg`, and `htmldemo.lg`.
- Symbolic computation and reusable data abstractions: `csls/fsm`, `csls/student`, `csls/pour`, `csls/algs`, and `csls/streams`. CSLS files are independent historical examples, sometimes reuse names or platform-specific features; check the target build before loading or adapting them.
- Load historical CSLS examples in an isolated session when possible: definitions, variables, and property lists can collide or persist. Not every extensionless CSLS file is Logo; some are Pascal examples used by `csls/pascal`.

Check the current source before relying on these pointers; implementation details and experimental Web Edition features can change.

## Reference Index

Load only the references relevant to the task.

| Reference | Load when |
| --- | --- |
| [Interactive Workspace](./references/interactive-workspace.md) | live-workspace edits, `save`/`load`/`erase`, burial, tracing |
| [Test Coverage](./references/testing.md) | writing Logo-level tests (suite harness, `runjs` and wrapper coverage); for browser or TestCafe work use the [web-testing skill](../ucb-logo-web-testing/SKILL.md) |
| [Logo Style Notes](./references/logo-style.md) | writing Logo code, arity, templates, `runjs` usage, Unicode semantics |
| [Web Edition Runtime Reference](./references/web-runtime-reference.md) | SVG, animation, sprites, tone/speech, demos, numeric operations |
| [Improving Usage And Extensibility](./references/improving-usage.md) | proposing library procedures, idioms, or language improvements |
