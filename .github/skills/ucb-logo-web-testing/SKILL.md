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
