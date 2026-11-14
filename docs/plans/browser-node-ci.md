# Project Plan: Supported Browser And Node.js CI

Status: Proposed; implementation has not started.
Date: 2026-11-13

## Objective

Run the Web Edition's functional and Logo regression tests on GitHub Actions for
all documented browser brands and Node.js. Automate routine compatibility work
while retaining periodic human-in-the-loop tests on the maintainer's real iPad.

Companion project: [TestCafe consolidation](testcafe-consolidation.md). Capability
probes and build preparation can start before migration completes; full browser
jobs consume its shared TestCafe fixtures and runner contract.

## Baseline And Scope

- [webedition.md](../../webedition.md) lists current Chrome, Firefox, Edge,
  Safari, and Node.js; it also describes Android and iOS mobile support and a
  Node.js prerequisite of version 20 or newer. Resolve runtime version policy
  explicitly rather than silently dropping the documented minimum.
- [ci.yaml](../../.github/workflows/ci.yaml) builds/tests native Linux Logo on
  pushes and pull requests targeting `master`. It has no Web Edition browser
  or Node.js jobs, and currently does not cover `wasm` pushes.
- [cd.yaml](../../.github/workflows/cd.yaml) owns existing native distribution
  builds. Preserve these jobs; Web Edition validation is not a wx replacement.
- `emconfigure ./configure --enable-wasm` plus `emmake make` / `make check`
  (wrapped by `npm run build:web`) configures and builds browser and Node
  outputs, runs the Node Logo suite, and generates the Workbox service worker for
  production builds. Debug info, sanitizers and Emscripten runtime checks are
  configure options; see [webedition.md](../../webedition.md#build-options).
- [package.json](../../package.json) currently mixes Selenium, TestCafe, and
  Lighthouse scripts. The companion plan consolidates functional browser tests.

This project adds CI, deterministic test/result handling, artifact packaging,
and the minimum build interfaces needed to support it. It does not authorize
paid device services, new self-hosted infrastructure, deployment, or broad native
build repair. Native CI remains intact and its failures remain visible.

## Coverage Matrix

The required desktop matrix covers every documented brand, not every possible
browser/OS combination. Record precise versions instead of promising all historic
versions. Mobile emulation supplements, but never substitutes for, actual mobile
engines and device checks.

| Target | Runner / Mechanism | Proposed Cadence And Gate |
| --- | --- | --- |
| Google Chrome stable | Ubuntu; TestCafe, headless or Xvfb as required | Every relevant PR/push; required after proof |
| Firefox stable | Ubuntu; TestCafe, headless or Xvfb as required | Every relevant PR/push; required after proof |
| Microsoft Edge stable | Windows; TestCafe | Every relevant PR/push; required after proof |
| Safari supplied by pinned macOS image | macOS; TestCafe `safari`, GUI, proxy automation | Every relevant PR/push; required only after capability proof |
| Mobile Chrome emulation | Ubuntu Chrome; small portrait/landscape touch profiles | Every relevant PR/push; required responsive checks, labeled emulation |
| Node.js application runtime | Ubuntu; direct generated Node/Wasm execution, no browser | Every relevant PR/push; required version matrix |
| Additional desktop OS coverage | Chrome/Firefox on Windows/macOS; Edge on Linux where useful | Scheduled/manual first; expand for platform-specific regressions |
| Actual Android Chrome and Firefox | Android emulator/device with a proven TestCafe connection/provider | Scheduled/manual after a capability probe; otherwise explicitly untested |
| iOS/iPadOS Safari simulator | macOS Xcode simulator plus a proven remote/custom TestCafe launcher | Optional scheduled/manual automation after proof; not physical-device coverage |
| Physical iPad Safari | Maintainer's Linux host and LAN TestCafe remote connection | Periodic and release/device-sensitive signoff; human-in-the-loop, not an unattended hosted job |
| GNOME Web/Epiphany (WebKitGTK) | Ubuntu; custom TestCafe path/provider and virtual display | Optional supplemental engine probe, not Safari coverage or a substitute for its required job |
| Lighthouse | Ubuntu Chrome; existing mobile/desktop audit tool | Scheduled/manual initially; promote stable agreed budgets later |

Desktop Safari is the primary automated WebKit target. TestCafe officially lists
Safari, and GitHub macOS images include it, but this repository has not yet proved
the integration. Browser presence alone is not proof of working GUI automation.
TestCafe has no built-in `webkit` alias or Safari headless mode. Do not require
SafariDriver for TestCafe's proxy path unless a selected provider actually uses it.

Actual mobile coverage is a separate milestone. If hosted simulators/emulators
cannot run the required flows reliably, record the gap and propose an approved
device service or self-hosted solution. Do not label desktop touch/viewport
emulation as automated iOS or Android coverage. iPad results do not prove iPhone
or desktop Safari behavior.

## Implementation Milestones

### 1. Prove Browser And Runtime Capabilities

- [ ] Add small manually triggered probe jobs for Chrome, Firefox, Edge, and
  Safari using the installed TestCafe version and pinned runner OS labels.
- [ ] Start with browser launch and one assertion; then load a fresh Logo build,
  wait for interpreter readiness, execute a Logo command, check SVG/DOM output,
  and verify persistence across reload.
- [ ] For Safari, check GUI session availability, first-launch prompts, launch
  permissions, localhost/proxy access, screenshots, and clean shutdown. Capture
  exact failures; do not silently replace Safari with Playwright WebKit.
- [ ] Investigate Android emulator and iOS simulator launch/connect paths
  separately. Prove proxy-port reachability, secure-context requirements, and
  shared tests before assigning them a scheduled job.
- [ ] Define supported Node runtime versions: test the documented minimum
  separately if retained, plus supported LTS lines at implementation time.
  Distinguish EOL compatibility from actively supported Node releases.
- [ ] Select one supported tooling Node version for TestCafe/Lighthouse/build
  dependencies. Do not assume their engine requirements match the application
  minimum; runtime-only jobs should not install incompatible browser tooling.

Exit criterion: each initial required desktop target has an observed successful
probe or an explicit blocker. Unsupported/mobile targets remain visible gaps,
not green skips or evidence-free compatibility claims.

### 2. Establish A Reproducible Build Artifact

- [ ] Pin a compatible Emscripten SDK, tooling Node version, and package lockfile;
  use `npm ci`. Verify native build utilities and resource staging, including
  the host-built help generator, instead of relying on local generated files.
- [ ] Introduce a fail-fast build interface with explicit outputs and separate
  test invocations. Keep the existing developer entry point usable. Preserve
  Asyncify and GC-root-scanning requirements; do not substitute JSPI.
- [ ] Build browser and Node artifacts from the checked-out revision. Publish
  a manifest with commit, effective flags/profile, toolchain versions, output
  hashes, and the expected packaged Logo/library/help resources.
- [ ] Package actual loader dependencies, Wasm, required data/maps, HTML, and
  production worker/Workbox assets based on emitted outputs, not loose globs of
  unrelated tracked artifacts. Never publish artifacts after build failure.
- [ ] Prove that the Node loader and `NODERAWFS` resource paths are relocatable
  before reusing Linux-built output on Windows/macOS. Stage resources explicitly
  or build on the target if necessary; do not assume absolute paths transfer.
- [ ] Build once per profile and fan out immutable artifacts to test jobs. Cache
  dependencies with lockfile/toolchain keys, not unverified generated runtimes.
- [ ] Start with the existing optimized production configuration. The
  `prod`/`dev`/`debug` distinctions now map to configure options (default;
  `--with-debug-info=...`; plus `--enable-asan` and runtime checks). Do not block
  the first browser probes on all three or silently disable failing sanitizers.

Exit criterion: a clean CI checkout builds successfully and a downstream job
serves/runs only manifest-verified outputs from that revision and configuration.

### 3. Add Browser And Node Test Jobs

- [ ] Download the matching artifact, provision the selected browser, log its
  binary/version, and invoke the companion plan's built-artifact TestCafe runner.
  Use the shared proxy-mode baseline for portability.
- [ ] Use one isolated server/origin/profile per job. Wait for readiness and
  verify the build identity from the served application, not just timestamps.
- [ ] Run the shared functional suite and the embedded Logo suite in every
  required desktop browser; record capability-specific exclusions with reasons.
- [ ] Run mobile portrait/landscape responsive fixtures separately, including
  settings, editor/autocomplete, focus, scroll, and simulated-viewport checks.
- [ ] For Node, run [test.lg](../../tests/test.lg) through the generated Node
  loader, plus applicable non-DOM bridge, IO/filesystem, and GC regressions.
  Do not treat Node success as browser DOM/persistence coverage.
- [ ] Grade Node and browser Logo results, not just process exit: reject test
  failures/errors, crashes, missing completion, and empty/no-test runs. Report
  known exclusions such as web OOP omissions. Test the grader using failing,
  incomplete, and multiline result samples, including expected-error cases.
- [ ] Preserve the current native result grading and add separate source-only
  JavaScript checks retained by the consolidation project.
- [ ] Verify nonzero status propagation through wrappers and pipelines. Set
  action, run, and job limits; terminate owned browser/server processes on failure
  and cancellation without killing unrelated processes.

Exit criterion: all required jobs pass on a known-good revision, and intentional
assertion failure, Logo failure, and timeout each make their job fail with evidence.

### 4. Wire Triggers, Reporting, And Required Checks

- [ ] Add a dedicated Web Edition workflow or clearly separated jobs. Cover
  pushes to `wasm` and `master`, pull requests to supported target branches,
  `workflow_dispatch`, and scheduled broader checks. Preserve native CI/CD triggers.
- [ ] Let PRs validate their checked-out revision/merge without credentials.
  Use least-privilege permissions; do not run untrusted PR code with deployment
  secrets or privileged `pull_request_target` execution.
- [ ] Use change filters only with an explicit skipped/not-applicable reporting
  strategy so required checks do not remain pending. Include shared interpreter,
  build, packaged Logo resources, tests, and dependency changes in web relevance.
- [ ] Cancel superseded PR runs with concurrency groups. During matrix bring-up,
  retain other browsers' results when one fails so cross-browser evidence survives.
- [ ] Upload machine-readable results, human summaries, console/startup errors,
  failure screenshots where supported, build/sanitizer logs, and version/manifest
  metadata with unique browser/profile names and bounded retention. Preserve
  reports on failure and distinguish passed, failed, skipped, and unavailable.
- [ ] Promote desktop/browser/Node checks to required only after stable successful
  runs and maintainer approval for branch protection. Temporary nonblocking
  probes must not become a permanent way to hide a supported-browser failure.
- [ ] Keep Lighthouse budgets separate from correctness gates until runner
  variance is measured. Retain mobile/desktop HTML and JSON audit reports.

### 5. Add Production, Debug, And Real-Device Coverage

- [ ] Test production worker installation, reload/update and offline behavior
  explicitly. Ordinary proxy acceptance tests alone do not prove an unmodified
  PWA's cache behavior; validate TestCafe proxy effects and use an appropriate
  unproxied/manual check if that behavior cannot be faithfully automated.
- [ ] Keep no-worker dev/debug origins separate; skipping worker generation is
  not enough to release an already controlled page. Never clear user data as
  a blanket stale-artifact remedy.
- [ ] Schedule compatible debug/ASAN runs and GC/lifecycle stress tests; retain
  optimized production coverage because rooting bugs can depend on optimization.
- [ ] Schedule proven Android/simulator jobs and optional WebKitGTK checks with
  their actual engine/OS labels. Do not add nominal jobs that never run tests.
- [ ] Publish a real-iPad runbook: artifact/commit selection, LAN server and both
  TestCafe ports, URL/QR connection, data preservation, filtered/full suite runs,
  and human keyboard/rotation/touch/background-resume/PWA checkpoints.
- [ ] Record periodic iPad results against the same revision tested by CI.
  Trigger human signoff for device-sensitive changes and releases, not every edit.
  GitHub-hosted CI cannot directly reach a private LAN iPad without separately
  approved networking/infrastructure; the default run remains on the LAN host.

## Dependencies, Risks, And Completion

Deliver in order: capability probes, trustworthy artifact/result grading,
initial required desktop and Node jobs, then extended profiles/mobile/audits.
The complete browser suite follows TestCafe consolidation; Node grading and
Safari proof can be developed independently.

Major risks are Safari GUI/permissions on hosted macOS, TestCafe compatibility
with changing browsers and proxy-sensitive APIs, Node dependency-engine mismatch,
absolute resource paths, and flaky performance/device tests. Pin reproducible
inputs, retain failure evidence, and resolve capability gaps before strengthening
coverage claims. Get approval before spending money or provisioning infrastructure.

Completion requires observed CI runs for Chrome, Firefox, Edge, Safari, and the
agreed Node versions; preserved native CI; tested failure/timeout propagation;
fresh artifact provenance; and a documented, demonstrated real-iPad workflow.
Mobile emulator/simulator gaps must be explicitly resolved or accepted as manual
coverage by the maintainer. Optional WebKitGTK and extra OS jobs are supplemental,
not substitutes for a missing required supported-browser check.

## Capability References

- [TestCafe browsers, custom paths, and headless support](https://testcafe.io/documentation/402828/guides/intermediate-guides/browsers)
- [TestCafe remote devices and emulation](https://testcafe.io/documentation/403584/guides/intermediate-guides/mobile-devices-cloud-browsers-and-emulation)
- [GitHub-hosted runner capabilities](https://docs.github.com/en/actions/reference/runners/github-hosted-runners)
- [macOS runner image contents](https://github.com/actions/runner-images/blob/main/images/macos/macos-15-arm64-Readme.md)

Recheck runner availability, browser versions, and tool support at implementation
time; these references establish candidates, not completed project validation.
