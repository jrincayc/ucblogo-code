# Project Plan: Restore wx And Terminal Builds

Status: Proposed; implementation has not started.
Date: 2026-11-14

## Objective

Restore native wxWidgets and terminal-only UCB Logo builds on the `wasm` branch
while preserving Web Edition behavior. Completion requires working native
behavior and regression coverage, not just successful compilation.

This document records the agreed plan. Saving it does not authorize application
changes, dependency installation, configuration, builds, or tests.

## Baseline And Scope

The initial investigation was a read-only source audit, not a reproduced native
build. The findings below are source-level blockers and risks; additional errors
may appear when native builds run.

- [Makefile.am](../../Makefile.am) already separates native and web entry points,
  adapters, graphics, and terminal backends. Preserve that structure rather than
  rewriting the platform layer.
- Restore both wx and terminal-only builds as part of this project. Terminal
  restoration is not a deferred follow-up.
- Preserve Logo semantics, native event/signal handling, GC rooting, and the web
  Asyncify contract. Do not introduce blanket native web stubs to hide accidental
  dependencies.
- Keep native and Wasm configuration, objects, executables, and staged resources
  isolated. Do not overwrite the current working Web Edition build casually.
- Do not perform this project's implementation, configuration, builds, or runtime
  tests in `/home/jeff/ucblogo-code`, the maintainer's active Web Edition workspace.
  Use `/home/jeff/ucblogo-code-c`, the maintainer's usual native/wx checkout, for
  native work. Check its branch, revision, local changes, and build state before
  use; it is user-owned, not a disposable directory. Preserve existing work and
  do not reset, clean, replace, or synchronize it destructively. Use additional
  isolated checkouts for conflicting configurations and the final fresh-checkout
  browser/Node validation, also outside the active Web Edition workspace.
- At the initial audit, GCC/G++ were available, but `wx-config` and `xvfb-run`
  were not found. Recheck prerequisites before implementation and coordinate any
  installation requiring elevated privileges.
- Rebase, commits, publishing, unrelated refactors, and a GC redesign are outside
  this plan. Native X11 graphics is not a substitute for terminal-only coverage;
  retain its existing path and record any untested coverage.

## Findings And Repair Direction

| Area | Source Evidence | Repair Direction |
| --- | --- | --- |
| Emscripten dependencies | [init.c](../../init.c#L25) includes Emscripten unconditionally; [init.c](../../init.c#L67) and [eval.c](../../eval.c#L42) use unguarded export annotations. | Guard headers and make annotations conditional without removing shared globals or requiring native Emscripten headers. |
| HTML node types | [logo.h](../../logo.h#L125) guards `NT_HTML`, but [logo.h](../../logo.h#L171) uses it in an unconditional `HTML_STRING` enum member. | Test retaining HTML-string support on wx rather than assuming it is web-only. Make definitions and conversion, printing, and collector consumers consistent with the capability decision. Actual wx HTML rendering is a stretch goal and should be investigated after the wx build is working again. |
| Web event queue | [eval.c](../../eval.c#L380) and [eval.c](../../eval.c#L926) unconditionally consume web events; implementations are in [liblogo.js](../../liblogo.js) and [webgraphics.c](../../webgraphics.c). | Guard web queue access while preserving native `eval_buttonact` callbacks and wx event processing. |
| Web scheduler state | [main.c](../../main.c#L133) writes `yield_enabled`, defined only in [weblib.c](../../weblib.c#L27). | Remove or guard web-only state changes in the native entry point; adding an extern alone is insufficient. |
| Native stack scanning | [mem.c](../../mem.c#L558) defines one-argument `mark_stack`, but [mem.c](../../mem.c#L810) passes two arguments on the native path. | Reconcile the native call with the current scanner contract, retaining native register spilling and stack-root initialization. |
| Text semantics | [print.c](../../print.c#L116) HTML-escapes characters and ordinary word printing uses the helper without a platform distinction. | Preserve literal ordinary text, file output, and string construction while evaluating intentional HTML rendering on wx. Select escaping by output contract rather than treating all native output as plain or all wx output as HTML. |
| Terminal backend | [term.c](../../term.c) has unguarded Emscripten dependencies and browser operations replacing or commenting out native terminal functionality. wx excludes this file, but terminal-only builds require it. | Rename the current implementation to `webterm.c` and restore legacy `term.c` for non-web, non-wx builds. Evaluate whether Node.js and browsers should share the web backend or use separate terminal files. |

Line references describe the audit snapshot and should be rechecked when work
starts. Shared HTML references include [logodata.c](../../logodata.c#L399),
[print.c](../../print.c#L341), and [mem.c](../../mem.c#L967).

## Subagent Execution Strategy

Parallelize bounded work, not shared build state. The coordinating agent owns
integration, shared interface decisions, and the final validation gate. The roles
below describe suggested task assignments, not pre-existing named agent modes or
authorization to begin implementation now.

### Low-Cost Model And Three-Attempt Limit

- Use a low-cost agent/model for implementation and subagent assignments. The
  maintainer's suggested choice is GPT-6 Luna if available. Verify the actual
  selectable model and current usage multiplier/pricing before starting; this
  plan does not assert that the suggested name is available or low-cost. If it
  cannot be selected, check with the maintainer before substituting a model.
- Do not automatically escalate to a more expensive model, delegate the same
  unresolved problem to additional agents, or expand the investigation merely
  to continue past the retry limit. A model change requires maintainer approval.
- Allow at most three unsuccessful fix attempts for the same blocker. One
  attempt is a concrete proposed repair followed by the focused build/test check
  that determines whether it resolved the issue. Record the hypothesis, changed
  files, command, and result for each attempt; unrelated tool calls are not
  separate fix attempts.
- After the third unsuccessful attempt, the subagent must stop repair work and
  notify the coordinator. The coordinator pauses implementation and checks in
  with the maintainer before further work. Safely stop owned processes as needed,
  preserve patches/logs and user work, and do not leave other agents making
  speculative changes while awaiting direction.
- The check-in must summarize the blocker/reproducer, all three attempted fixes
  and their outcomes, current patch/build state, and recommended next choices.
  Ask whether to continue, change approach/model, defer the task, or stop. Do not
  make a fourth attempt without explicit authorization.
- Track the count per blocker across coordinator/subagent handoffs, model changes,
  and resumed sessions. Renaming or reassigning the issue does not reset it.
  Missing prerequisites or an unsafe/unclear next step may require an earlier
  check-in; three attempts are a ceiling, not a requirement to spend them all.

### Suggested Roles

| Subagent | Assignment And File Ownership | Required Handoff |
| --- | --- | --- |
| Shared Core Compatibility | Repair platform guards, evaluator event boundaries, native scheduler references, HTML representation consistency, GC call signatures, and ordinary text semantics. Own shared C/headers such as `logo.h`, `globals.h`, `init.c`, `eval.c`, `main.c`, `mem.c`, `logodata.c`, and `print.c`. | Focused patch, native compile/link evidence, interface decisions, and GC/output risks. Do not remove candidate wx HTML support merely to unblock compilation. |
| Terminal Backend Restoration | Rename current `term.c` to `webterm.c`, restore the legacy native terminal implementation, and evaluate Node/browser sharing. Own terminal backend files; propose shared-header and build-recipe changes to the coordinator rather than editing another owner's files. | Legacy source revision, backend patch, operation/host mapping, Node/browser recommendation, and focused IO/PTY checks when buildable. |
| Native CI And Test Harness | Prepare wx and terminal unit-test execution/grading and native CI coverage. Own test-runner additions and native workflow edits; do not independently change shared build configuration. | Reproducible commands, timeout/failure handling, report checks, CI patch, and target-specific limitations. Preserve the existing Logo unit-test harness. |
| wx Validation | After integration, build wx in its assigned native checkout, run and grade Logo unit tests under Xvfb, then perform relevant GUI smoke checks. Validation-only unless a repair is reassigned explicitly. | Exact source/configuration, commands, graded report, GUI results, and reproducible failures. |
| Unix Terminal Validation | After integration, build the Unix terminal target in a separate checkout, run and grade Logo unit tests, then test redirected streams and PTY behavior. Validation-only unless reassigned explicitly. | Exact source/configuration, commands, graded report, terminal-state results, and reproducible failures. |
| wx HTML Feasibility And Optional Rendering | Only after wx works, evaluate type retention and live rendering; if pursued, own wx renderer/adapter changes. Submit Autoconf requirements to the coordinator. | Representation-versus-rendering findings, dependency and disabled-fallback contract, optional renderer patch, and enabled/disabled tests. |

The coordinator owns [configure.ac](../../configure.ac),
[Makefile.am](../../Makefile.am), shared build recipes, generated build-system
updates, and this plan. Shared-core ownership includes final application of
HTML-type/printing changes requested by the wx rendering agent. Reassign file
ownership explicitly before a subagent edits outside its assignment.

### Execution Order

1. The maintainer manually updates `/home/jeff/ucblogo-code-c`. The coordinator
  verifies its branch, revision, local changes, prerequisites, and source
  baseline, then assigns directories and file ownership. Do not pull or
  synchronize the native checkout automatically.
2. Launch Shared Core Compatibility, Terminal Backend Restoration, and Native CI
  And Test Harness as independent assignments. Agree on HTML representation and
  terminal symbol contracts before dependent edits. Read-only wx HTML research
  may occur here, but renderer implementation waits until wx recovery.
3. Integrate their patches one at a time and run focused checks after each.
  Resolve shared-header and build-selection changes centrally. Full terminal
  validation waits for both shared-core repairs and backend restoration; wx
  validation waits for the shared repairs and applicable integrated build edits.
4. Run wx Validation and Unix Terminal Validation in parallel in separate build
  trees against the same integrated source snapshot. Route failures back to the
  owning implementation agent; do not let validators independently patch shared
  code. Revalidate affected targets after fixes.
5. After wx recovery, pursue the optional rendering task if feasible, then finish
  its affected native tests, CI, and documentation. Do not let stretch-goal work
  bypass or delay reporting the mandatory native Logo unit-test outcomes.
6. Only after all other changes are integrated, perform Task 6's fresh-checkout
  browser/Node validation. Node and browser tests may run concurrently after
  both builds succeed, using separate processes and nonconflicting resources.
  Do not run competing configure/clean/build commands in one tree. Any resulting
  fixes require repeating the final fresh-checkout gate.

### Isolation And Reporting

- Give each concurrent build/configuration its own directory outside
  `/home/jeff/ucblogo-code`. Protect user-owned `/home/jeff/ucblogo-code-c`; use
  additional isolated copies for other agents rather than reconfiguring it
  concurrently. Do not create Git branches, commit, or cherry-pick without
  authorization; integrate reviewed patches when those operations are not agreed.
- Every agent prompt must state the target directory, source snapshot, allowed
  files, dependencies on other work, focused validation, and explicit prohibitions
  on touching the active web workspace or reverting unrelated changes. Include
  the approved low-cost model policy, each blocker's existing attempt count, and
  the mandatory three-attempt stop-and-check-in rule.
- Agents return patches or findings plus actual commands/results, reports,
  blockers, and owned process IDs/ports. The coordinator reviews each handoff and
  distinguishes proposed checks from executed checks. Limit concurrent builds
  and browser runs to available CPU/memory, and clean up task-owned processes.
- Parallel subagent success does not establish integration success. Mandatory
  native unit tests and final browser/Node revalidation must exercise the combined
  changes, not separate partial implementations.

## Implementation Sequence

### 1. Establish Isolated Native Baselines

On Debian/Ubuntu releases providing wxWidgets 3.2, install the native build and
test prerequisites with:

```sh
sudo apt update && sudo apt install \
  build-essential autoconf automake autoconf-archive pkg-config \
  libncurses-dev libwxgtk3.2-dev libgtk-3-dev libnotify-dev \
  xvfb xauth
```

`libncurses-dev` supplies terminal capability headers/libraries; `xvfb` and `xauth`
support the headless wx test harness. These packages target the `--disable-docs`
configurations below, not manual generation or the Emscripten SDK. Optional wx
HTML-renderer dependencies are intentionally excluded until a renderer is chosen.
Check package availability on older distributions. This command is documentation
for the maintainer to run; no installation has been performed as part of planning.

- [ ] Recheck native compiler, wxWidgets, terminal capability library, Autotools,
  and Xvfb prerequisites.
- [ ] Inspect `/home/jeff/ucblogo-code-c` and establish the intended native source
  revision there without disturbing existing changes. Use additional isolated
  source copies for conflicting configurations unless out-of-tree support has
  been verified for all relevant custom recipes. Do not use the active
  `/home/jeff/ucblogo-code` workspace for implementation or build/test execution.
- [ ] Configure wx explicitly and confirm that wx was actually selected; do not
  mistake a fallback build for wx coverage.
- [ ] Configure terminal-only explicitly with wx and X11 disabled.
- [ ] Capture each initial compile/link failure with effective configuration and
  compiler flags. Do not test stale executables after a failed build.

### 2. Repair Shared Compile And Link Boundaries

- [ ] Fix conditional Emscripten includes and export annotations.
- [ ] Make HTML type guards consistent across definitions and consumers, testing
  whether wx can retain support before restricting HTML strings to web builds.
- [ ] Separate web event consumption and scheduler state from native execution.
- [ ] Fix the native GC call signature and verify its stack-root contract.
- [ ] Compile the touched translation units first, then link each native target.
  After each focused edit, run the cheapest relevant check before expanding scope.

### 3. Restore Native Text And Terminal Behavior

- [ ] Rename the current [term.c](../../term.c) implementation to `webterm.c`,
  preserving intentional Web Edition behavior during the move.
- [ ] Identify and record the appropriate pre-port revision, then restore legacy
  `term.c` for non-web builds that use the ordinary terminal backend, including
  the native X11 configuration. Reconcile necessary shared API changes rather
  than blindly replacing the file. wx continues to use [wxterm.c](../../wxterm.c).
- [ ] Update source selection in [Makefile.am](../../Makefile.am) and any other
  active build recipes: native terminal builds select restored `term.c`, wx
  selects its existing backend, and Wasm selects the web backend subject to the
  Node.js/browser decision below. Ensure each executable links exactly one
  terminal implementation and includes the required sources in distributions.
- [ ] Ensure ordinary native text displays literal punctuation and multiline
  text without visible HTML entities or unintended markup interpretation.
  Distinguish intentionally tagged HTML from ordinary words. Cover stdout,
  redirected writers, dribble output, string construction, and workspace
  save/load round trips; GUI rendering must not alter stored text.
- [ ] Restore native terminal capability lookup, character-mode transitions,
  screen clearing, cursor positioning, and margins. Consult the pre-port native
  implementation/history where behavior was replaced or commented out.
- [ ] Keep DOM calls, `new_output_context`, and web export annotations on the web
  path. Evaluate sharing HTML-string construction/formatting with wx separately
  from browser-only rendering APIs.
- [ ] Verify input adapter selection in [files.c](../../files.c),
  [parse.c](../../parse.c), and [globals.h](../../globals.h): stdio for terminal,
  wx input for wx, asynchronous input only for web.
- [ ] Test EOF, unget, line/character reads, redirected input, pause/stop, waits,
  and exit behavior. Do not narrow EOF to a byte at an input boundary.
- [ ] Preserve native callbacks and signal handling. Verify `buttonact`, `keyact`,
  and graphics where supported rather than assuming web events replace them.

#### wx HTML Strings And Rendering Stretch Goal

Test whether wx can retain the HTML-string representation without Emscripten
dependencies. As a bonus, aim to render explicitly tagged HTML as formatted
content in the live wx terminal, not merely display markup or export HTML.
Rendering is a non-blocking stretch goal: do not make a substantial GUI redesign
a prerequisite for native build restoration.

- [ ] Make wx HTML rendering an explicit Autoconf option in
  [configure.ac](../../configure.ac), proposed as `--enable-wx-html` /
  `--disable-wx-html`, disabled by default. Do not silently enable it merely
  because optional renderer dependencies happen to be installed.
- [ ] Check renderer headers/libraries or wx components only when rendering is
  enabled. Fail with an actionable diagnostic if explicitly enabled without wx
  or required dependencies, including on Wasm configurations. Show the effective
  setting in configure output and document prerequisites in `--help`/build docs.
- [ ] Use a dedicated rendering feature define and Automake conditional where
  needed. When disabled, exclude optional renderer sources, headers, link flags,
  and runtime dependencies. Keep this option independent of the decision to
  retain the HTML-string type, conversion, and GC support in wx.
- [ ] Trace the live output path through [wxterm.c](../../wxterm.c),
  [wxMain.cpp](../../wxMain.cpp), and [wxTerminal.cpp](../../wxTerminal.cpp).
  Identify the actual rendering control and existing HTML capabilities; HTML
  export alone is not evidence that the terminal renders HTML.
- [ ] Test creation, conversion, copying, printing, and repeated GC of tagged HTML
  strings in native wx without browser helpers. Retain representation support
  when tests pass, keeping `NT_HTML`, `HTML_STRING`, conversion cases, and
  collector handling consistent on supported targets.
- [ ] Probe actual GUI rendering with tagged HTML beside the same markup as an
  ordinary word. Verify formatting, entities, multiline and mixed plain/HTML
  output, and malformed markup. Record the supported subset rather than assuming
  browser HTML/CSS/JavaScript parity.
- [ ] If the existing control cannot render HTML, evaluate a small wx-native
  adapter or rendering path. Record dependencies, portability, and effects on
  selection/copy, scrolling, input focus, history, clearing, and performance
  before choosing it. Keep markup inert; do not execute scripts or introduce
  automatic external-resource loading as part of this stretch goal.
- [ ] Verify ordinary text remains literal, escaping happens exactly once where
  needed, and redirected writers, dribbling, string construction, and workspace
  save/load retain their defined text semantics independently of GUI rendering.
- [ ] Document separately whether wx retains the type and whether it renders
  HTML. Specify non-rendering behavior for plain terminal/Node targets and wx
  fallback behavior if rendering is unavailable; none may require a browser DOM.

#### Node.js And Browser Backend Decision

Do not assume that compiling both targets with Emscripten means their terminal
requirements are identical. Evaluate the following before finalizing source
selection; sharing `webterm.c` is a candidate, not an already approved outcome.

- [ ] Inventory terminal operations in the renamed implementation and its JS
  bridges. Separate genuinely shared Wasm behavior from DOM rendering, browser
  input events, Node stdin/stdout, and host-terminal operations.
- [ ] Compare Node.js interactive TTY and redirected/batch behavior with browser
  behavior: character mode, EOF/unget, prompts, clearing, cursor/margins,
  formatting, signals, and shutdown. Specify unsupported operations explicitly.
- [ ] Evaluate one shared `webterm.c` with narrow host-specific adapters versus
  separate browser and Node.js terminal files. Prefer sharing when host
  differences remain small and testable; prefer separate files when sharing
  would scatter DOM/Node conditionals or compromise either terminal contract.
  Extract common helpers only where they remove meaningful duplication.
- [ ] Record the decision, tradeoffs, and operation-to-backend mapping in this
  plan before implementing the final arrangement. Preserve Asyncify and GC
  requirements in the Node.js build regardless of terminal-file selection.
- [ ] If separate files are selected, update browser and Node.js object/link
  lists independently. `__EMSCRIPTEN__` alone cannot distinguish the hosts;
  verify the build-time or runtime selection mechanism and avoid duplicate
  terminal symbols or browser-only imports on the Node.js path.
- [ ] Validate both Node.js batch and interactive/PTY use where supported, plus
  browser terminal workflows, before accepting either arrangement. Node.js
  output must not require a DOM or accidentally emit browser HTML formatting.

### 4. Validate All Three Targets

| Target | Configuration | Required Validation |
| --- | --- | --- |
| Linux wx | `./configure --disable-wasm --enable-wx --disable-docs` | Compile/link, startup, GUI input/events/graphics, Logo regressions, file round trips, repeated GC. |
| Linux terminal-only | `./configure --disable-wasm --disable-wx --disable-x11 --disable-docs` | Compile/link, Logo regressions, piped input/output, real-terminal or PTY checks, terminal-state restoration. |
| Web Edition | Existing `emconfigure ./configure --enable-wasm` workflow, preserving the intended profile | Fresh browser/Node artifacts, Node Logo regressions, focused desktop Chrome output/event/GC checks, then relevant broader regressions. |

- [ ] Exercise default and object-enabled native builds. Do not carry the web OOP
  exclusion into native validation without establishing an actual native failure.
- [ ] If the rendering stretch goal is implemented, test wx with
  `--disable-wx-html` in an environment without optional renderer dependencies
  and with `--enable-wx-html` where those dependencies are available. Test clear
  configure failures for missing dependencies and incompatible target options,
  and verify documented non-rendering behavior when the feature is disabled.
- [ ] Reuse the native regression harness in
  [ci.yaml](../../.github/workflows/ci.yaml), adapting its invocation for terminal
  execution as needed. Grade individual `Ok`, `Failed`, and `Error` results rather
  than trusting completion text alone.
- [ ] Use a real terminal or PTY for character-mode, cursor, and signal tests.
  Batch tests alone cannot establish interactive correctness.
- [ ] Verify normal exit and interrupts leave terminal echo/canonical settings
  usable, including after character reads and error recovery.
- [ ] Exercise allocation-heavy and repeated-GC cases in optimized native builds.
- [ ] Rebuild Web Edition after shared C changes and verify artifact freshness
  before running browser checks. Preserve Asyncify and browser non-reentrancy.
- [ ] Record precise tested configurations and remaining OS/browser coverage gaps.
  Node success does not establish browser or wx GUI correctness.

#### Local wx Build And Tests Matching GitHub CI

Running the Logo unit tests is mandatory for wx build validation. Use the freshly
built native wx executable to run [UnitTests.lg](../../tests/UnitTests.lg) under
Xvfb with the CI invocation below, and grade its individual results. Compilation,
GUI startup, manual smoke checks, or passing Node/browser tests cannot substitute
for this native wx Logo unit-test run. If it cannot run, report wx unit-test
validation as blocked or untested, not passed.

- [ ] Run the local equivalent of the existing
  [CI build/test job](../../.github/workflows/ci.yaml#L42), not a newly invented
  wx test harness. Keep objects and git IDs enabled for this CI-equivalent run;
  test the default non-object configuration separately.

Run the following from the root of a fresh, isolated native source copy containing
the intended changes, after installing the prerequisites above. Do not run it in
the configured Web Edition tree or an Emscripten compiler environment. No existing
build objects or results should be copied into this native tree.

```sh
set -eu
PROJECT_ROOT="$PWD"
RESULTS="$PROJECT_ROOT/logo-unit-test-results.txt"

autoreconf --install
CC=gcc CXX=g++ ./configure \
  --disable-wasm --enable-wx --enable-objects --enable-gitid --disable-docs
grep -Eq '^#define HAVE_WX 1$' config.h
make

rm -f "$RESULTS"
cd "$PROJECT_ROOT/tests"
LOGOLIB="$PROJECT_ROOT/logolib" \
  timeout 120s xvfb-run -e /dev/stdout --auto-servernum \
  "$PROJECT_ROOT/ucblogo" UnitTests.lg - -f "$RESULTS" -x

test -s "$RESULTS"
awk '/\.\.\./ && ! /Ok$/{print "::error::"$0}' "$RESULTS"
awk '/\.\.\./ {count++; if ($0 !~ /Ok$/) failed=1}
     END {if (count == 0 || failed) exit 1}' "$RESULTS"
```

The configure options explicitly select native wx instead of relying on CI's
defaults. `timeout 120s` mirrors the CI test step's two-minute limit; `set -e`
prevents tests after a failed build and rejects a failed or timed-out test run.
The working directory, `LOGOLIB`, Logo arguments, and per-result grading follow
CI. The local recipe additionally rejects a missing, empty, or result-free report
so stale/absent output cannot appear to pass. Retain the report and build/test
logs, including on failure. A failed/timed-out run must not be reported as passing
even if a partial results file contains only `Ok` lines.

- [ ] Record local compile/link and graded-test outcomes, configuration, and
  remaining failures. These commands are planned validation, not evidence that
  the current broken sources pass.
- [ ] After automated tests, perform the planned visible wx GUI smoke checks;
  a Logo suite under Xvfb does not establish every interactive GUI workflow.

#### Mandatory Unix Native Terminal Logo Unit Tests

- [ ] Run [UnitTests.lg](../../tests/UnitTests.lg) locally with the freshly built
  Unix native terminal executable, configured with
  `--disable-wasm --disable-wx --disable-x11 --disable-docs`. Exercise both default
  and object-enabled configurations, selecting the suite's appropriate cases
  without silently omitting failures.
- [ ] Reuse the Logo unit-test harness, working directory, `LOGOLIB`, result-file
  options, and per-result grading described above, but invoke the terminal
  executable directly without Xvfb. Verify the harness's invocation and shutdown
  behavior on the terminal target during implementation rather than assuming wx
  and terminal input lifecycles are identical.
- [ ] Require a successful test process and a fresh, nonempty graded results
  report containing actual test results. Reject failures, errors, timeouts, and
  missing reports; retain the report and logs. Report blocked or unrun tests as
  untested, never passed.
- [ ] Treat this as mandatory terminal-build validation. Compilation, interactive
  smoke checks, or passing wx/Node/browser tests cannot substitute for running
  the Logo unit tests on the native terminal executable. Run the planned PTY and
  redirected-stream checks in addition to the unit suite.

### 5. Add Lasting CI And Documentation

- [ ] Extend native CI to cover relevant `wasm` development pushes/PRs; the audited
  [workflow](../../.github/workflows/ci.yaml#L3) only triggers for `master`.
- [ ] Add terminal-only compile and regression coverage alongside wx, not instead
  of wx. Add a focused PTY regression where practical.
- [ ] If wx HTML rendering is implemented, retain a dependency-light wx CI build
  with rendering disabled and add a separate enabled-renderer check. Document the
  configure switch, extra dependencies, supported markup, and disabled fallback.
- [ ] Keep web regression coverage alongside native checks; coordinate with the
  [browser and Node CI plan](browser-node-ci.md) rather than duplicating it.
- [ ] Retain compiler logs and graded results and propagate failures through exit
  status. Keep unsupported/unavailable checks visibly untested.
- [ ] Document prerequisites, isolated build commands, supported native terminal
  behavior, and any verified platform limitations.

### 6. Final Browser And Node.js Revalidation In A Fresh Checkout

Run this task last, after all preceding implementation, native validation, CI,
documentation, and any pursued stretch-goal work is complete. Earlier Web Edition
checks do not replace this final clean-checkout validation.

- [ ] Prepare a fresh checkout containing the exact completed changes. Record
  its revision and any explicitly applied uncommitted patch; do not accidentally
  validate an older committed baseline. Do not commit or discard work solely to
  prepare the checkout without authorization.
- [ ] Install the project's declared dependencies and configure/build both the
  browser and Node.js targets from source using the supported Emscripten workflow.
  Do not copy native objects, generated configuration, staged resources, loaders,
  Wasm files, or other build outputs from existing working trees. Verify even
  tracked generated outputs are freshly rebuilt before using them.
- [ ] Run and grade the Logo unit tests against the newly built Node.js target,
  plus the applicable Node terminal/input/output regressions. Confirm execution
  does not depend on a browser DOM. Report skips and failures explicitly.
- [ ] Run source checks and the functional browser suite, including its graded
  Logo unit tests, against the fresh browser build. Start with desktop Chrome;
  follow the project's broader browser validation order for additional coverage
  and record unavailable or human-coordinated device checks honestly.
- [ ] Verify the server root/URL and loaded loader/Wasm/resources belong to this
  checkout. Use a fresh browser origin/profile or bypass HTTP and service-worker
  caches for diagnosis without clearing the user's persistent Logo data.
- [ ] Retain build/test logs, graded reports, tool/browser versions, build profile,
  and checkout/artifact provenance. Report browser and Node.js outcomes separately.
  If a defect requires further source or build changes, repeat this final task
  from a fresh checkout containing those fixes before declaring completion.

## Completion Criteria

- Both native targets compile and link without Emscripten headers, JS imports, or
  web-only state dependencies.
- Native regression results are graded and passing, with any unrelated baseline
  failures explicitly recorded rather than silently skipped.
- The Logo unit tests have been run locally using the freshly built wx executable
  and the CI-equivalent Xvfb invocation, with individual results graded and the
  report retained. Build success or GUI smoke checks alone do not satisfy this.
- wx starts and its affected GUI/input/event/graphics workflows work.
- wx HTML-string retention has been tested and its outcome documented, with
  consistent type, conversion, and GC handling. Actual formatted HTML rendering
  in the live wx terminal is a separately reported stretch goal, not a condition
  for completing native restoration. Ordinary text and file semantics remain
  correct regardless of rendering capability.
- If implemented, wx HTML rendering is opt-in through Autoconf and disabled by
  default. Disabled builds require no optional renderer dependencies; enabled
  builds validate prerequisites explicitly. HTML-string representation support
  is not unnecessarily tied to the rendering option.
- Terminal-only Logo works both interactively and with redirected streams;
  terminal settings are restored after exit and interruption.
- The Logo unit tests have also been run locally with the freshly built Unix
  native terminal executable, independently of wx, with individual results graded
  and the report retained. Compilation and terminal smoke checks alone do not
  satisfy this requirement.
- The current terminal implementation has been moved to `webterm.c`, legacy
  `term.c` has been restored for native terminal builds, and wx retains its own
  backend. Build recipes select exactly one appropriate terminal implementation.
- The Node.js/browser terminal-sharing decision is documented and validated
  against their distinct host requirements, including Node.js without a DOM.
- Native text, workspace persistence, and optimized GC tests pass.
- Fresh Web Edition builds retain language, output, event, and GC behavior.
- CI protects both native targets and the development branch, with web coverage
  retained and remaining platform gaps documented.
- After all other work, browser and Node.js builds have been rebuilt and tested
  in a fresh checkout containing the completed changes, with their Logo unit
  tests graded and results/provenance retained. Reused artifacts or earlier web
  checks do not satisfy this final validation gate.

## Validation Status

No implementation, native build reproduction, dependency installation, or runtime
validation has been performed for this plan. All implementation tasks are pending.