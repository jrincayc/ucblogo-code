---
name: ucb-logo-build-deploy
description: "Build, configure, package, and deploy the UCB Logo Web Edition and native builds. Use for Autotools (configure.ac, Makefile.am), emconfigure and emmake, npm build scripts, prod/dev/debug/ASAN profiles, Emscripten toolchain upgrades, GitHub Actions CI and CD workflows, the Workbox service worker and PWA manifest, stage-web-deploy, GitHub Pages release tags, live-site verification, wx/native build restoration, and upstream rebase or PR planning."
argument-hint: "Describe the build, CI, packaging, release, or toolchain change."
---

# UCB Logo Build And Deploy

Make builds reproducible, easy to debug, and fast to iterate while preserving both Web Edition and wxWidgets builds. Fix source recipes, not generated outputs. Interpreter changes belong to the [interpreter skill](../ucb-logo-interpreter-development/SKILL.md); test selection and browser validation belong to the [web-testing skill](../ucb-logo-web-testing/SKILL.md).

## Procedure

1. **Identify the target and profile.** Web (`prod`, `dev`, `debug`), Node, or wx. Check the active configuration before running `make`.
2. **Switch targets safely.** Follow the Artifact Freshness rule in [project instructions](../../copilot-instructions.md#artifact-freshness): `make clean` under the current configuration first, then reconfigure. If the current Makefile cannot clean safely, use an isolated build tree.
3. **Read before editing.** Read `configure.ac`, `Makefile.am`, `package.json`, and the relevant workflow; see [build system](./references/build-system.md).
4. **Build, then test.** Chain tests after a successful build (`&&`); never run tests against leftovers from a failed build. Verify the intended sources and macros were selected.
5. **Deploy only on request.** Publishing is a release tag push; get confirmation first. Follow [CI and deploy](./references/ci-and-deploy.md) and verify the live bytes afterward.
6. **Report.** State which configuration, toolchain versions, and checks were actually used, and what could not be checked (for example wx GUI, macOS, Windows).

## Reference Index

| Reference | Load when |
| --- | --- |
| [Build system](./references/build-system.md) | changing configure options, Makefile recipes, profiles, npm scripts, or toolchain flags |
| [CI and deploy](./references/ci-and-deploy.md) | editing workflows, releasing to GitHub Pages, staging, or verifying the live site |
| [Upstream roadmap](./references/upstream-roadmap.md) | planning rebase, wx repair, or the upstream PR |
