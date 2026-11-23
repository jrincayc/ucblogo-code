# Project Plan: Investigate DEP0169 In The Test Harness

Status: Proposed; investigation has not started.
Date: 2026-11-22

## Objective

Determine the exact source and package ownership of the Node.js `DEP0169`
deprecation warning emitted while the shared TestCafe runner executes the
`ACC-URL-001` URL-isolation acceptance test. The investigation is limited to
diagnosis and evidence: it does not authorize source edits, dependency upgrades,
lockfile changes, suppression of the warning, or implementation changes.

The warning text states:

> `url.parse()` behavior is not standardized and prone to errors that have
> security implications. Use the WHATWG URL API instead.

This warning is a deprecation and compatibility signal, not evidence that a
CVE exists or that UCB Logo application code is vulnerable. CVEs are not issued
for `url.parse()` vulnerabilities; ownership and reachability must be proven
before remediation.

## Scope And Baseline

- Reproduce the warning with the exact focused `ACC-URL-001` TestCafe command.
- Capture Node's complete deprecation stack and preserve the terminal/browser
  identities involved.
- Trace the call chain from the project runner into TestCafe, Puppeteer, proxy,
  URL, or utility dependencies.
- Determine whether legacy parsing is project-owned, directly TestCafe-owned,
  transitive, or inherited from the browser automation stack.
- Record whether the legacy behavior is justified for compatibility or whether
  it has a safe WHATWG URL replacement.
- Keep Web Edition application behavior, generated artifacts, TestCafe fixtures,
  and package manifests unchanged during the investigation.

Observed baseline:

- The warning appears only in the Node process reported as PID `348733`.
- Chrome 152.0.0.0 / Linux Mint 22.3 passes `ACC-URL-001` after the warning.
- The test harness may have a legitimate reason for legacy `url.parse()` use.
- No conclusion should be drawn until the originating stack identifies the
  package and call site.

## Milestones

### 1. Preserve And Reproduce The Stack

- [ ] Run the exact focused test with
  `NODE_OPTIONS=--trace-deprecation`.
- [ ] If `--trace-deprecation` does not expose a useful call chain, rerun once
  with `NODE_OPTIONS=--throw-deprecation` in an isolated terminal.
- [ ] Record the Node.js, npm, TestCafe, Chrome, Linux, and relevant package
  versions.
- [ ] Preserve the relevant warning and stack, excluding unrelated terminal
  output or credentials.
- [ ] Confirm that the focused test still passes and that the warning's PID
  corresponds to the test-harness process rather than browser content.

Exit criterion: the trace reaches a repository or dependency function and the
runtime versions and reproduction command are recorded.

### 2. Map Direct And Transitive Ownership

- [ ] Search tracked project JavaScript for `url.parse(...)`, `require('url')`,
  `require("url")`, `import * as url`, and equivalent legacy URL API use.
- [ ] Search the call chain and test-harness dependencies for the exact call
  site.
- [ ] Use `npm ls` or a lockfile query to identify direct owners and the shortest
  dependency path to the package containing the call.
- [ ] Inspect the package implementation and tests to understand the URL data and
  legacy semantics it handles.
- [ ] Distinguish project-owned code from TestCafe, browser automation, proxy,
  or unrelated transitive tooling.

Exit criterion: each call-site candidate has a package owner, dependency path,
version, and source evidence, or is explicitly unresolved.

### 3. Evaluate Compatibility And Security Impact

- [ ] Determine whether the call handles absolute URLs, relative URLs,
  malformed URLs, credentials, redirects, proxies, file URLs, or browser-facing
  request URLs.
- [ ] Identify differences that may exist between legacy `url.parse()` and
  WHATWG `new URL(...)`, including base resolution and malformed-input behavior.
- [ ] Determine whether the package has an existing WHATWG API path or option.
- [ ] Check whether the installed package version has a compatible upgrade that
  removes the legacy call.
- [ ] If no upgrade exists, determine whether replacement belongs in project
  code, a parent dependency, or upstream tooling.
- [ ] Assess whether the call accepts attacker-controlled input and crosses a
  trust boundary.

Exit criterion: there is an evidence-backed disposition of project-owned,
dependency-owned, compatibility-sensitive, or not-reachable, plus the security
impact supported by that evidence.

### 4. Select A Remediation Or Record A Justified Disposition

Possible outcomes include:

- Project-owned legacy use: replace only the demonstrated call with WHATWG APIs,
  with focused tests for equivalent behavior.
- Direct dependency: upgrade to a compatible version if that version removes
  the call without breaking the harness.
- Transitive dependency: upgrade or replace the direct parent dependency,
  or document a narrowly scoped compatibility resolution after evidence is
  available.
- Legitimate dependency compatibility behavior: do not suppress or patch it;
  record the owner, tested behavior, upgrade path, and residual risk.
- Tool-only or unreachable: document that it does not execute in UCB Logo
  application code and does not create a project CVE obligation.
- No safe disposition: stop remediation and escalate with the exact package,
  stack, and reproduction evidence.

A resolution does not authorize edits until it is separately approved as a
project implementation task.

## Validation Plan

If a remediation is later approved:

- Run the exact `ACC-URL-001` focused test.
- Run relevant shared-runner source tests and focused URL/application-server
  tests.
- Confirm the warning is absent or remains dependency-owned and documented.
- Verify package and lockfile diffs against only the approved dependency scope.
- Run `git diff --check` and the repository's focused whitespace check.
- Preserve browser, runtime, build, and package identities in the result.

## Evidence Artifact

Record the completed investigation under `docs/research/` with:

- Reproduction command and environment.
- Full relevant deprecation stack.
- Project/dependency/package ownership and `npm ls` path.
- Compatible WHATWG APIs or the reason none are appropriate.
- Upgrade, replacement, suppression, and no-change alternatives.
- Rejected alternatives and compatibility or security consequences.
- Residual risks and a separate implementation recommendation if remediation is
  warranted.

No temporary trace, package-tree output, or credential-bearing data should be
copied into repository prose.

<!-- Co-authored by GitHub Copilot. Scope: DEP0169 test-harness investigation plan. Model and reasoning-effort metadata unavailable. Elapsed wall-clock time and environmental accounting unavailable. -->
