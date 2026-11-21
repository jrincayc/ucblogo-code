# Executive Summary: Integrating the Web Edition into `master`

> **Status:** Approved on 2026-11-21. This is the maintainer-facing summary of `wasm-upstream-integration.md`, the planning source of truth on the `wasm` branch. Implementation remains delayed pending explicit upstream maintainer approval.

## Goal

Move the intentional work from the `wasm` branch into `jrincayc/ucblogo-code:master` through small, independently reviewable PRs. Splitting the branch is a means to complete upstream adoption—not the goal itself.

## Approach

### 1. Establish a controlled baseline

Record specific `master` and `wasm` commit IDs, inventory the diff by meaningful hunk, and map dependencies. New commits on `wasm` will be tracked while the PR series is underway, preventing the integration backlog from changing invisibly.

### 2. Restore and protect legacy builds first

On a private branch based on `master`, repair the wx and Unix terminal builds and restore their regression coverage:

- terminal and wx unit suites;
- terminal PTY checks;
- wx smoke tests with screenshots and visual diffs;
- native CI with explicit result grading.

Files not already present on `master`, particularly the PTY and wx smoke infrastructure and screenshot baselines, will require explicit approval.

Native validation and integration are limited to wx and Unix terminal only.

The complete legacy integration branch must pass in a fresh checkout before any PR is opened.

### 3. Derive small PRs from the validated result

The private integration branch serves as the source of truth, but it will not become a PR. It will be divided into independently valid units such as:

1. Native test infrastructure and CI
2. Platform guards and build selection
3. Terminal backend restoration
4. Native I/O compatibility
5. GC and stack-root contracts
6. Small independent defensive fixes

Each PR will target `master` or a named predecessor, contain one coherent review question, and build and pass its relevant tests by itself.

### 4. Add the Web Edition incrementally

After the native baseline is accepted, the remaining work will be introduced in reviewable vertical slices:

- shared interpreter and build compatibility;
- a minimal browser and Node WebAssembly build;
- asynchronous runtime/input lifecycle;
- Logo/JavaScript bridge;
- Web graphics;
- filesystem and persistence;
- terminal, editor, Help, settings, accessibility, and other UI features;
- Web tests, CI, packaging, PWA, deployment, and documentation.

Shared-interpreter PRs will rerun native tests. Web PRs will validate their actual browser and Node runtimes; build success alone will not count.

### 5. Preserve ongoing development and prove convergence

Every new `wasm` commit will be classified as part of a pending PR, a follow-up to merged work, a future feature, or an intentional exclusion. At the final checkpoint, the remaining branch diff will be audited so every intentional change is accounted for.

## Completion Criteria

The effort is complete when:

- `master` reliably builds and tests wx, Unix terminal, browser WebAssembly, and Node WebAssembly targets;
- intended Web features, tests, documentation, tooling, and deployment support have landed;
- no unexplained `wasm` delta remains;
- PR #268 is retired or marked superseded by the accepted PR series;
- `master` becomes the canonical integrated source.

This approach gives the maintainer small, testable decisions while treating complete upstream adoption—not temporary branch splitting—as the endpoint.

<!-- Co-authored by GitHub Copilot. Scope: approved executive summary of the WASM upstream integration roadmap. Model: MAI Code 2; reasoning-effort setting unavailable. Task wall-clock time: 4m 10s partial measured interval (2026-11-21T16:49:45Z to 2026-11-21T16:53:55Z). Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->
