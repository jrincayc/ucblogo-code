# Quality Assurance Process

## Purpose

This document defines the quality process for UCBLogo. It applies to source code, tests, build scripts, documentation, Web Edition assets, native targets, CI, releases, and process changes.

Quality means correct behavior, reproducible builds, trustworthy evidence, clear scope, and no hidden regressions. Quality is a shared responsibility, not a final test phase.

## Principles

- Preserve correct behavior unless a documented requirement or approval changes it.
- Prefer the narrowest check that discriminates the changed behavior.
- Rebuild when source or embedded assets change; never validate stale artifacts.
- Test observable behavior at the lowest effective layer.
- Record exact revisions, configurations, commands, results, and coverage limits.
- Treat skipped, filtered, unavailable, and human-only checks as uncovered, not passing.
- Stop work when a defect affects a passed gate or reveals a systemic issue.
- Keep changes small, reviewable, reversible, and tied to acceptance criteria.
- Avoid retries or timing changes as substitutes for deterministic synchronization.
- Do not clear user storage or interfere with unsaved work for a test run.

## Scope

### Source and runtime targets

Select validation according to the affected target:

- Logo language, primitives, files, memory, and garbage collection: Logo regressions and the native or Node harnesses.
- Web runtime bridge, DOM, events, storage, UI, SVG, and reentrancy: fresh Web artifacts plus the shared TestCafe suite.
- Node/WebAssembly behavior without browser APIs: the Node Logo and Node/Wasm suites.
- wxWidgets, Unix terminal, and PTY behavior: the native CI jobs and available local smoke checks.
- Build recipes, service workers, packaging, and deployment: source/build validation plus a fresh served artifact.

Changes affecting multiple targets require coverage for each materially affected target. An isolated passing result does not prove cross-target correctness.

### Test layers

Use tests in this preference order:

1. A narrow unit or source check for deterministic logic.
2. A Logo regression for interpreter-language behavior.
3. A Node/Wasm check for runtime behavior without browser dependencies.
4. A browser acceptance test for DOM, events, persistence, Web APIs, or visual behavior.
5. A native target or device check for platform-specific behavior.
6. A manual human checkpoint for native keyboard, touch, rotation, hardware, or physical device behavior.

Do not run browser tests solely to repeat lower-layer language logic. Conversely, Node success does not establish browser behavior.

## Development Workflow

```mermaid
flowchart TD
    A[Change or defect identified] --> B[Research and reproduction]
    B --> C[Plan and target-impact analysis]
    C --> D[Test Plan]
    D --> E[Implement tests and baseline]
    E --> F[Implement changes]
    F --> G[Focused target validation]
    G --> H[Independent code review]
    H --> I[Aggregate QA and exact-revision check]
    I --> J{Human stage acceptance}
    J -->|Approved| K[Final integration or release gate]
    J -->|Not approved| C
    G -->|Unexpected failure| L[Diagnose and repair]
    L --> G
    H -->|Findings| F
    I -->|FAIL or BLOCKED| F

    G -->|Defect in already passed gate| X[ANDON: stop affected work]
    H -->|Escaped defect| X
    I -->|Escaped defect| X
    X --> Y[Preserve evidence and reopen gate]

    subgraph TG[Target Gates]
        T1[Web HTML and Wasm]
        T2[Node and Wasm]
        T3[wxWidgets - Linux Unix macOS Windows]
        T4[Unix terminal objects and no-objects]
    end

    F -.->|affected targets| TG
```

Every stage produces a deliverable, independent evidence or review as applicable, and a human checkpoint where the contract requires one. Material scope, acceptance, or target changes reopen the earliest affected stage.

### 1. Define the change

Before implementation:

- state the defect or user-visible requirement;
- identify affected targets and ownership;
- define observable acceptance criteria;
- identify regressions and unresolved risks;
- determine whether visual or accessibility review is required.

Material ambiguity that prevents a bounded implementation shall stop the affected line and reopen the earliest affected planning gate.

### 2. Reproduce and plan tests

When a behavior is absent or broken:

- establish the reproduction with exact inputs and target revision;
- write or define a discriminating test that fails for the intended reason;
- separate setup failures and syntax defects from product failures;
- identify fixture data, setup, teardown, environment, and cleanup.

Behavior-preserving work requires a baseline and an explanation of the relevant regression protection.

### 3. Implement tests

Add regressions to the existing suite and shared harness. Prefer deterministic observable assertions over implementation details, arbitrary delays, and incidental formatting.

A test is implementation-ready only when it:

- is compatible with existing fixtures and capability filtering;
- waits for real readiness or observable state;
- restores settings or runtime state it modifies;
- preserves unrelated user work; and
- reports individual failures rather than relying on completion text alone.

### 4. Implement the change

Keep implementation substeps small and independently reviewable. Each substep shall run the agreed focused check, inspect the complete diff, and preserve unrelated changes.

Build and test results shall use exact source and artifacts. Reconfigure or clean targets before switching profiles; do not use stale objects or generated output as evidence.

### 5. Review and integrate

Inspect the complete stage diff and exact final revision. Code and test work shall have independent review where required by the repository workflow. Documentation, configuration, and process substeps follow the relevant QA and commit checks.

Material changes invalidate affected review and QA verdicts. Repairs require a fresh focused check and any affected independent review.

### 6. Final acceptance

Before reporting completion:

- review the exact commit range or revision;
- verify that requested checks passed against fresh artifacts;
- confirm coverage and record skips or unavailable platforms;
- ensure test reports and residual risks correspond to the tested revision;
- obtain the human approval required for the stage or publication.

## Test and Build Commands

Use the repository package scripts and CI inventory as the source of truth. Common Web Edition gates are:

| Goal | Command | Result boundary |
| --- | --- | --- |
| Build and run the complete Web suite | `npm test` | Build, source checks, and functional browser acceptance |
| Source checks only | `npm run test:source` | Existing source and Node-side checks; no rebuild |
| Full browser suite against existing artifacts | `npm run test:browser:built` | All discovered functional fixtures; may include capability filtering |
| Browser suite with a fresh build | `npm run test:browser` | Fresh Web build followed by functional browser acceptance |
| Node/Wasm Logo grading | `npm run test:logo:node` | Existing Node/Wasm build; inspect individual results |
| Focused browser test | `node tests/acceptance/run-testcafe.js --test 'exact test name'` | One exact test name |
| TestCafe pattern | `node tests/acceptance/run-testcafe.js --test-grep 'regex'` | Matching tests |
| Lighthouse | `npm run test:lighthouse` | Separate audits, not functional or language coverage |
| Whitespace verification | `node scripts/format-whitespace.js --check --changed` | Markdown and prose formatting |

Run focused checks during iteration. Broaden browser, native, and device coverage before integration, publication, or reporting release readiness. Do not run the entire platform matrix after every edit unless required by the change or an approved gate.

## Browser and Device Coverage

The Web Edition shall be validated through the shared TestCafe platform matrix. Broader validation runs in this order:

| Priority | Platform | Validation type | Evidence boundary |
| --- | --- | --- | --- |
| 1 | Desktop Chrome / Chromium | Headless and affected headed runs | Primary desktop regression platform; record browser and engine versions |
| 2 | Mobile Chrome / Chrome emulation | Emulated phone viewport and touch behavior | Responsive evidence, but not a physical phone pass |
| 3 | Firefox | Desktop and relevant mobile-emulated coverage | Distinguishes Chromium-family behavior and Web API differences |
| 4 | Real iPadOS Safari | Human-in-the-loop remote Safari and PWA session | Native keyboard, physical touch, rotation, storage, lifecycle, and offline evidence |
| 5 | Desktop Safari | macOS CI or approved hosted macOS browser | Supported but unavailable locally; do not infer it from iPad Safari or WebKit automation |

Additional mobile Firefox coverage applies when relevant or explicitly requested. Broaden before pushing, opening a PR, or reporting final readiness; checkpoint commits do not require the complete matrix unless a platform-specific failure or approved risk trigger requires it.

### Web platform evidence rules

- Use Chrome for the default focused desktop iteration. Use Chrome device emulation for mobile-first layout or touch iteration without a physical device.
- Firefox follows mobile Chrome/emulation in the default broader-validation order. Prioritize it earlier when diagnosing a Firefox-specific failure.
- Desktop Safari requires actual Safari on macOS or an approved macOS CI/hosted Safari service. Automated WebKit is not a desktop Safari pass.
- iPad Safari requires a coordinated human session on the maintainer's iPad. Synthetic Visual Viewport changes, dispatched orientation events, proxy clicks, and in-page `.click()` calls are not native keyboard, physical touch, or physical rotation evidence.
- TestCafe capability filtering is an environment limit, not a passing result for excluded cases. Record exact capability and resize exclusions.
- iPad Safari does not establish iPhone or desktop Safari coverage. Mobile Chrome emulation does not establish a physical Android or iPhone pass.
- Report engine, browser, OS/device, profile, emulation versus real-device status, viewport, build, and exact test selection.

### Real iPad session

A real iPad session shall use a dedicated fresh test runtime, preserve unsaved work, verify the served build, and record:

- device and browser versions;
- Safari tab or installed-PWA mode;
- commit, build configuration, and served artifact;
- runner settings and exact filters;
- automated results and capability exclusions;
- native keyboard and rotation observations;
- touch, background/resume, storage, reload, and offline behavior;
- unavailable or pending checks.

Desktop Safari is not claimed locally without actual evidence. Human iPad signoff is pending rather than inferred from automated emulation or remote automation.

## Artifact Freshness

Before trusting a result:

- identify the target and profile;
- confirm the source revision and dirty state;
- rebuild after changes to embedded files, C sources, shell markup, runtime JavaScript, Logo startup files, or generated assets;
- verify the HTML, loader, Wasm, packaged files, and server root belong to the intended build;
- verify a reused browser page is on the intended origin and has bypassed stale HTTP or service-worker state;
- verify the server is owned or reused by the operator and will be cleaned up correctly.

A build on disk does not update an already-running runtime. When behavior contradicts source or a previous result, diagnose freshness first. Do not clear all site data by default.

## Stop-the-Line and Escalation

A normal failed test or rejected implementation is not automatically an Andon. Continue bounded diagnosis within the approved scope.

Stop and report an Andon when:

- a defect is found in an artifact, commit, decision, or approval that already passed a gate;
- a defect reveals a likely systemic or cross-target issue;
- a failed gate blocks advancement;
- a required review or evidence is unavailable;
- a requirement becomes too ambiguous to proceed safely; or
- three consecutive focused fix-and-check attempts fail on the same issue.

On an Andon:

1. stop the affected work immediately;
2. preserve exact revision and evidence;
3. notify the human and affected owners;
4. identify the affected scope and plausible consequences;
5. reopen the earliest invalidated gate;
6. do not resume until the human or governing process authorizes a resolution.

A current-gate FAIL or BLOCKED permits bounded correction; it is not automatically an escaped-defect Andon unless work already passed a gate.

## Evidence and Reporting

Every quality report shall state:

- purpose and scope;
- exact revision, commit range, branch, and dirty state;
- target, build profile, runtime, browser, and device where applicable;
- commands or methods;
- actual pass, fail, blocked, skipped, and unavailable results;
- individual test results rather than only overall exit status;
- residual risks and coverage gaps;
- temporary processes or servers retained after the task.

For formal workflow checks, append structured results under `docs/workflow-runs/<task-id>.md` in `## QA and Check Results`. For checks with no formal workflow record, append a canonical entry to `docs/qa-result-log.md`. Both logs shall preserve prior failures and use `Supersedes` for later corrections. Never rewrite history to conceal a missed gate or failed result.

A result shall use only:

- `PASS`: evidence satisfies the exact acceptance criteria;
- `FAIL`: evidence demonstrates a missed criterion or defect;
- `BLOCKED`: required evidence, review, toolchain, network, hardware, or scope is unavailable;
- `N/A`: the result is explicitly reasoned and tied to a non-applicable criterion.

Missing evidence is `BLOCKED`, never an assumed pass.

## Git and Commit Discipline

- Make frequent small commits after cleared semantic blocks.
- Stage only task-owned files and inspect the staged diff.
- Run focused validation and record its actual result before committing.
- Keep intermediate commits on a private or unshared branch; do not imply unpushed work is integrated.
- Do not push or rewrite pushed history without fresh explicit human approval for the exact remote, branch, and commits.
- Stop and disclose a process failure if required review or QA evidence is missing for a commit.

## CI as an Inventory

GitHub Actions is the inventory and baseline for automated cross-target coverage. At minimum, Web changes should account for:

- source and runner Node tests;
- fresh Web build and browser acceptance;
- Node Logo grading;
- browser runner behavior;
- Lighthouse audits;
- native terminal and PTY behavior;
- wxWidgets unit and smoke checks where affected or available.

CI coverage does not replace focused local diagnosis or independent review. A CI green mark shall not be used to claim browser, device, platform, or visual coverage that the job did not execute.

## Reviews

- The implementation owner owns correctness of the focused change and focused evidence.
- Independent review prioritizes correctness, security, regressions, scope, and unintended behavior.
- Development QA independently verifies stage acceptance and exact revision identity.
- Visual work requires independent proposed-design review and independent rendered-output review when the gate applies.
- Human signoff is required before stage advancement or publication.
- No author or reviewer shall approve work they authored.

## Process Changes

Process changes require:

- a concrete defect or improvement;
- affected scope and owner;
- risk and rollback analysis;
- revised acceptance and evidence;
- explicit human approval;
- documentation of approved deviations and their limits;
- a follow-up check that the revised process works.

A deviation is not approval from urgency, silence, role consensus, or an earlier task request. If the process proves harmful or wasteful, improve the document rather than silently bypassing it.

### Supported build targets

Maintain all four supported major targets continuously:

| Target | Configuration | Primary runtime and role | Required quality coverage |
| --- | --- | --- | --- |
| Web HTML/Wasm | Emscripten with `emconfigure ./configure --enable-wasm` | Browser UI, SVG, DOM, storage, events, PWA, and Web APIs | Fresh build; source/runner checks; TestCafe browser acceptance; Lighthouse for audited web health |
| Node/Wasm | Emscripten Node environment from the same Web/Wasm toolchain | Headless Node subprocess and filesystem runtime | Fresh Node/Wasm artifact; Node Logo grading with individual `Ok`, `Failed`, and `Error` results |
| wxWidgets | Native configure with `--enable-wx` | Desktop GUI, graphics, input, and platform integration | Independent native build under Xvfb when needed; Logo unit suite; wx smoke tests and visual baselines |
| Unix terminal | Native configure with `--disable-wasm --disable-wx --disable-x11` | Pipe, PTY, curses, redirected-I/O, and literal terminal behavior | Objects and object-free builds as applicable; Logo unit suite; literal-output and PTY checks |

The Web HTML and Node targets share Wasm source and build machinery but are not interchangeable. A passing Node Logo run does not prove browser behavior. Conversely, passing browser tests do not prove the Node subprocess contract or Unix filesystem behavior.

Native and Emscripten builds shall not be reconfigured concurrently in the same checkout or object tree. Use isolated worktrees or build directories for target matrices, and follow `make clean` and target-switching rules before trusting a result.

### Target-specific expectations

#### Web HTML/Wasm

- Rebuild after shell markup, runtime JavaScript, C sources, Logo startup files, packaged Logo tests, help, or Web assets change.
- Verify the served HTML, `.js`, `.wasm`, startup files, service worker, and origin represent the intended build.
- Use the shared TestCafe runner rather than a second browser assertion framework.
- Wait for HTTP, welcome-output, idle-evaluator, and input readiness; do not replace readiness with fixed sleeps.
- Cover desktop Chrome and affected browser/device targets. Record capability filtering, emulation, real-device status, and excluded cases.
- Keep Lighthouse as a separate audit. Never count audit thresholds as language or functional browser coverage.

#### Node/Wasm

- Build or relink the Node environment separately and verify it contains the exact source revision.
- Grade the individual Logo result lines and require final `All tests completed.` plus successful process exit.
- Omitted or skipped OOP coverage is not passing coverage.
- Verify process errors, signals, timeout handling, runtime crashes, stdout/stderr, stdin, and raw filesystem behavior as applicable.
- Add Node regressions for bridge, raw-filesystem, stdin, evaluator, and non-DOM runtime behavior that does not belong in TestCafe.

#### wxWidgets

- Configure explicitly with `--disable-wasm --enable-wx`; verify `HAVE_WX` is selected and the intended object/profile options are active.
- Maintain wxWidgets coverage on Linux, Unix, macOS, and Windows; it is not a Linux-only desktop edition.
- Build and run Logo unit tests in each available target environment; report missing GUI dependencies and unavailable visual evidence rather than treating compilation as runtime validation.
- Run the wx smoke harness for GUI startup, input, events, graphics, and repeated state checks when the change affects those surfaces.
- Compare GUI screenshots only against approved CI baselines or explicitly approved baselines for the exact operating system and environment.
- Verify native file round trips and repeated GC where memory, graphics, or object changes are involved.
- Preserve compatibility with headless Xvfb CI and avoid claiming an interactive desktop pass from a headless-only run.

##### wx platform matrix

| Platform | Required evidence | Platform limit |
| --- | --- | --- |
| Linux | Native configure/build, Logo unit suite, and wx GUI startup/input/affected behavior under Xvfb or an approved desktop session | Xvfb proves the automated harness, not an interactive desktop session |
| Unix | Native configure/build and platform-appropriate Logo, terminal, file, graphics, and smoke coverage | Document the Unix variant, display system, toolchain, wxWidgets version, and unavailable graphical or input checks |
| macOS | Native configure/build, Logo unit suite, and wx startup/graphics/input smoke through macOS CI or human desktop validation | Do not infer macOS behavior from Linux GTK/Xvfb, Windows, or Unix terminal builds |
| Windows | Native configure/build, Logo unit suite, and wx startup/graphics/input smoke through Windows CI or human desktop validation | Do not infer Windows behavior from Linux, Unix, or macOS wx builds; record MinGW/MSVC or another toolchain identity |

Cross-platform wx changes shall identify an available CI or human gate for every affected wx platform. If a platform cannot execute, mark that platform `BLOCKED` or explicitly exclude it by approved scope; never relabel Linux GTK evidence as macOS or Windows coverage.

#### Unix terminal

- Configure explicitly with `--disable-wasm --disable-wx --disable-x11`, and test both object-enabled and object-free variants when object support or default behavior can change.
- Run and grade the full Logo unit suite, including individual result lines and missing-report detection.
- Verify literal redirected output and the absence of HTML entity substitution.
- Exercise PTY behavior, character-mode and termios state, Ctrl-C, xterm cursor/control sequences, `TERM=dumb`, unset `TERM`, redirected stdout, EOF, and terminal-state restoration when terminal code changes.
- Test piped stdin and stdout where I/O behavior changes. Avoid treating an interactive terminal pass as equivalent to pipe or EOF coverage.

### Cross-target review rule

A shared C, header, parser, evaluator, memory, file, graphics, build, or test-harness change requires a target-impact analysis. At minimum, identify whether it affects the Web HTML, Node/Wasm, wxWidgets, and Unix terminal targets. Run or plan the affected target gates; a target shall not be marked applicable/green unless its exact build and focused evidence exist.
