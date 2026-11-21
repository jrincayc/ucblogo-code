---
name: ucb-logo-web-testing
description: "Run, write, debug, and report tests and browser validation for the UCB Logo Web Edition. Use for TestCafe acceptance tests, the shared runner and its TESTCAFE_* options, focused or hung browser runs, the Node Logo suite, Lighthouse, desktop and mobile Chrome, Firefox, remote iPad Safari sessions, the platform coverage matrix, stale-artifact or service-worker cache diagnosis, and deciding which checks to run before reporting results."
argument-hint: "Describe the behavior to test, or the failing or hung test."
---

# UCB Logo Web Testing

Validate Web Edition behavior in the real target runtime and report exactly what ran. Browser-only behavior needs browser checks; Node success does not establish DOM, IndexedDB, rendering, or event-loop correctness. Logo-language test authoring (suite harness, `runjs` coverage) belongs to the [Logo programming skill](../ucb-logo-programming/references/testing.md); C changes belong to the [interpreter skill](../ucb-logo-interpreter-development/SKILL.md).

Global rules live in [project instructions](../../copilot-instructions.md): browser priority order, Safari policy, artifact freshness, and process ownership. Do not restate them here.

## Procedure

1. **Establish freshness.** Identify the artifact, URL, and runtime under test. Rebuild after changes to `shell.html`, `pre.js`, `liblogo.js`, or C sources; built-only commands do not rebuild. If behavior contradicts source, load the [stale-artifacts checklist](./references/stale-artifacts.md) before changing code again.
2. **Write the failing test first.** Assert observable results (value, focus, `aria-*`, computed style), not only internal state. Choose the test layer with [test strategy](./references/test-strategy.md).
   If a test is difficult to write or flaky, first ask whether a less brittle test (lower layer, deterministic seam, state or attribute assertion) satisfies the same requirement; see [test strategy](./references/test-strategy.md).
3. **Run the narrowest case.** Use the shared runner with `--test 'exact name'` or `--test-grep`, readiness waits, and its Node watchdog; see [commands](./references/commands.md). Keep the reproducer stable while diagnosing.
4. **Read individual results.** Review each `Ok`, `Failed`, and `Error`; a completion message or exit status alone is not evidence. Skipped tests (including OOP) are not passes.
5. **Broaden deliberately.** Expand to other browsers and devices in the [platform matrix](./references/platform-matrix.md) order only before pushing, opening a PR, or reporting final results, when asked, or when the failure is browser-specific. Do not run the full matrix after every edit.
6. **Report precisely.** State the browser/engine and version, emulation versus real device, artifact build, and what was skipped, unavailable, or pending a human pass (mobile Safari).

## Reference Index

| Reference | Load when |
| --- | --- |
| [Validation commands](./references/commands.md) | choosing an npm script, runner flag, or `TESTCAFE_*` variable |
| [Test strategy](./references/test-strategy.md) | deciding which layer (Logo, Node, browser, wx) covers a change |
| [Browser test lessons](./references/browser-test-lessons.md) | writing UI tests, bisecting a hang, iPad or viewport issues, keyboard handling |
| [Platform matrix](./references/platform-matrix.md) | planning broader validation or CI coverage |
| [Stale artifacts](./references/stale-artifacts.md) | results contradict source, or before trusting a build or browser result |

# Browser Test Priority

- Run test cases in a headless browser by default
- If the user requests a visible headed browser then use an external browser
- Do not run tests in the vscode embedded browser unless asked.
- Default iteration uses focused desktop Chrome checks. For mobile-first design features or mobile-specific issues, use mobile Chrome, Chrome device emulation, or the affected mobile browser as the primary iteration target, according to what best exercises the feature or reproduces the issue.
- For broader validation, the default browser-test order is desktop Chrome, mobile Chrome (or Chrome device emulation), Firefox, then mobile Safari on the maintainer's iPad. Follow explicit user scope and prioritize the affected browser when diagnosing a browser-specific failure; do not run the whole matrix after every edit. Report emulation as emulation, not real-device coverage.
- Mobile Safari testing requires a human-in-the-loop session on the maintainer's iPad with the current no-Mac setup. Because device connection and interaction require coordination, do it last in broader validation, after the automated checks pass. Coordinate with the maintainer rather than assuming an unattended run; test earlier when specifically diagnosing a mobile Safari issue or explicitly requested.
- Desktop Safari is supported, but the maintainer cannot test it locally because there is no Mac. Report that coverage gap; use actual Safari through available macOS CI or an approved hosted service when feasible, without treating automated WebKit or iPad Safari as desktop Safari validation. See the web-testing skill's [platform matrix](./references/platform-matrix.md).

# Multi-Target Test Runs

- wx tests exist only to detect regressions caused by Web Edition changes. Pre-existing wx failures and a janky harness are out of scope; take screenshot baselines from the CI environment, not a local machine.
- When asked to run all tests across the main targets, delegate wxWidgets, Unix terminal, WebAssembly browser, and Node/WebAssembly validation to separate subagents and run them in parallel.
- Give each target its own detached worktree or isolated build directory. Native and Emscripten configurations rewrite shared generated build files and object outputs; do not reconfigure a shared checkout concurrently or reuse objects across targets.
- Use `.github/workflows/ci.yaml` as the test inventory and command source of truth. Include each relevant job and grading step, report skipped or unavailable coverage, and do not install missing dependencies without user approval.
- Treat wx unit and smoke tests, both Unix terminal configurations and PTY/literal-output checks, Web build and browser/source/audit checks, and a separately built Node/Wasm suite as distinct coverage. Avoid duplicating target work unless a build dependency requires it.