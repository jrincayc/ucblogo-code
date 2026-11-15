---
name: ucb-logo-interpreter-development
description: "Develop, debug, and review the UCB Logo interpreter's C/C++ internals and Web Edition runtime, preserving wxWidgets Edition. Use for the mini-Scheme explicit-control evaluator, plm, parsing and primitives, garbage collection and memory management, Emscripten/Wasm/Asyncify lifecycle, the C/JS runtime bridge and runjs, IO, virtual filesystem and IDBFS persistence. For UI, accessibility, settings, and SVG graphics use the web-frontend skill; for tests and browser validation use the web-testing skill; for builds, CI, and deployment use the build-deploy skill."
argument-hint: "Describe the interpreter, runtime bridge, or compatibility change."
---

# UCB Logo Interpreter Development

Investigate and implement interpreter and runtime changes. Default to Web Edition; keep wxWidgets Edition building and preserve its shared language behavior. Treat the user as an expert collaborator and discuss evidence, invariants, and tradeoffs directly.

Companion skills own adjacent work: [web frontend](../ucb-logo-web-frontend/SKILL.md) for `shell.html`, `pre.js` UI, accessibility, settings, and SVG graphics; [web testing](../ucb-logo-web-testing/SKILL.md) for tests, browser validation, and stale-artifact diagnosis; [build and deploy](../ucb-logo-build-deploy/SKILL.md) for Autotools, profiles, CI, packaging, and releases.

This skill permits C/C++ interpreter changes. The companion [Logo programming skill](../ucb-logo-programming/SKILL.md) covers idiomatic Logo programs, semantics, examples, and JavaScript extensions without owning C implementation. Use its language guidance as needed; its C-edit restriction does not apply to interpreter development under this skill.

Load the [project reference](./references/project-map.md) for source ownership and documentation pointers. Do not survey the whole repository on every task. Source pointers and known limitations must be checked against current code.

## Working Procedure

1. Identify the observable behavior and target editions. Start at a failing command, primitive, test, entry point, or owning source; follow forwarding code to the actual decision point.
2. Read nearby implementation and tests. Form a falsifiable hypothesis and choose the cheapest discriminating check before editing. Use `plm` for the conceptual model, then verify against source.
3. Choose the smallest coherent layer: Logo library, shared C interpreter, platform adapter, JavaScript runtime, HTML/CSS, or build system. Do not hide shared semantic defects in web-only wrappers.
4. State relevant invariants: Logo semantics, NODE lifetime, evaluator state, platform guards, suspend/resume boundaries, IO ordering, or persistence readiness. Preserve backward compatibility unless a breaking change is explicitly approved.
5. Implement a focused change and immediately run its narrow regression test or executable validation. Repair that slice before broadening scope. Scale coverage with risk, especially for GC, evaluator, IO, and cross-platform contracts.
6. Rebuild affected artifacts and validate in the actual target runtime. Browser-only behavior needs browser checks; Node success does not establish DOM, IndexedDB, rendering, or event-loop correctness. Use the [web-testing skill](../ucb-logo-web-testing/SKILL.md) to choose and run checks; if behavior contradicts source, load its [stale-artifacts checklist](../ucb-logo-web-testing/references/stale-artifacts.md) before editing again.
7. Update affected docs and report exactly what passed, failed, or could not be checked. Preserve unrelated work; do not commit, regenerate unrelated outputs, or overwrite build configurations casually.

## Reference Index

Load only the references relevant to the task.

| Reference | Load when |
| --- | --- |
| [Project Map](./references/project-map.md) | locating the owning source file, or finding the docs a change must update |
| [Implementation Language, Language Model, And Evaluator](./references/language-and-evaluator.md) | adding a Logo word, touching the evaluator, parsing, primitives, or Unicode strings |
| [Garbage Collection And Memory](./references/gc-memory.md) | touching NODE lifetime, roots, GC, Whippet, memory bugs, or debugging `-O1`/Asyncify failures |
| [Web Runtime And Asyncify](./references/web-runtime-asyncify.md) | startup, yielding, Asyncify, UI event queue, or JS-to-interpreter signaling |
| [IO, Filesystems, And JavaScript](./references/io-filesystem-js.md) | input/output, IDBFS persistence, uploads, or `runjs` |
| [Logo Words Across C, JS, And Adapters](./references/primitives-and-bridge.md) | adding a Logo word backed by `liblogo.js`, an adapter, or a wx/web shared primitive |
