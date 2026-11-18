---
description: "Use when editing or running browser acceptance tests, TestCafe fixtures, or the shared runner under tests/acceptance."
applyTo: "tests/acceptance/**"
---
# Acceptance Test Rules

- Run tests through `node tests/acceptance/run-testcafe.js` with `--test 'exact test name'` or `--test-grep 'regex'`. Do not use raw `testcafe --app` commands or fixed startup delays.
- Synchronize on runtime and input readiness or observable output, never arbitrary sleeps.
- Assert observable results (value, focus, `aria-*`, computed style), not only internal state.
- Built-only commands do not rebuild. Rebuild after changes to `shell.html`, `pre.js`, `liblogo.js`, or C sources before trusting a browser result.
- In `ClientFunction` callbacks, iterate DOM nodes with `for...of` instead of array callbacks. Under touch emulation, dispatch a DOM `.click()` for Shadow DOM options.
- Skipped tests (including OOP) are not passing coverage.

See the [web-testing skill](../skills/ucb-logo-web-testing/SKILL.md) for commands, lessons, and the platform matrix.
