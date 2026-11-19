# Test Coverage

- Future work: improve the Logo language unit tests and their harness, including coverage, reliable failure reporting, and ease of adding tests. Treat this as a separate task, not an instruction to change the harness during unrelated work.
- Identify gaps and add focused unit or regression tests for pure Logo and Logo/JavaScript hybrid behavior, including existing behavior as well as new features. Cover representative values, boundary cases, errors, and compatibility where relevant; follow the existing test framework and conventions in `tests/`.
- Logo suite tests output `true` or `false`. Add each test to its suite's test list; for a new suite, follow `tests/_suite-template.lg`, install it with `InstallSuite`, and ensure the harness loads it. A defined but unlisted test does not run automatically.
- Use Logo tests for language-level behavior and browser acceptance tests when behavior depends on the JavaScript bridge, DOM, events, HTML output, or browser APIs. For manual Logo rapid iteration, use the VS Code embedded browser by default; use external Chrome only when the user requests it or embedded access is unavailable. Automated browser acceptance runs continue to use the TestCafe runner and external browser engines.
- Browser acceptance, TestCafe commands and flags, the platform order, remote iPad sessions, and Lighthouse live in the [web-testing skill](../../ucb-logo-web-testing/SKILL.md). Browser tests exercise automated browsers; mobile emulation is not real Safari coverage.
- Include command-level wrapper coverage for `webstartup.lg` helpers and refactor tests toward bound-arg use, replacing quote-based interpolation where possible.
- Use individual Logo tests for rapid iteration and run the relevant suite for broader validation. The browser build currently packages the suite under `/tests`; from the Logo prompt, `ignore runjs "|FS.chdir('/')| load "tests/test.lg` runs the full entry point. It ends with `bye`, so the browser session terminates; report any subsequent runtime error separately from test results.
- Inspect individual `Ok`, `Failed`, and `Error` results, not just the final completion message: the underlying Logo harness does not set its failure flag for every false result, so use the Node Logo runner and browser Logo fixture's result checks. Logo OOP is broken and omitted from the web build; do not rely on it or count its skipped tests as passing coverage.
- If a failing test exposes an interpreter defect requiring C changes, retain or propose a focused regression test and explain the failure and suggested C-level handoff; do not implement the C fix under this skill. Report skipped tests, environment limitations, and untested behavior accurately.

## Lessons Learned: Repro Discipline During Runtime Debugging

- When the bug appears only under optimized web/Node builds, keep the exact reproducer command stable while collecting evidence; avoid changing test parameters until the failure mechanism is understood.
- Ensure any helper script that runs tests after building cannot execute stale outputs from a failed compile; require build success before test launch.
- Prefer additive diagnostics over behavioral changes while triaging nondeterministic runtime failures: capture IDs, object lifecycle evidence, and exact failing operation context first.
- If evidence points to interpreter memory lifetime (for example, nodes being swept before use), escalate to interpreter-development fixes rather than masking symptoms in Logo-level programs or tests.
