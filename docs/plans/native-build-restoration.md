# Project Plan: Restore wx And Terminal Builds

Status: Native restoration is complete with one accepted known failure: `Tests.File.LiteralDribble` is reported as missing by the Node/browser graders because dribble output is concatenated with its `Ok` status. Fresh Chrome reports an Asyncify shutdown abort after the suite has printed all results. This test is intentionally left unchanged per maintainer direction. Native CI/docs and isolated production Web validation are complete; Node terminal split and wx HTML rendering remain deferred.
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
  maintainer's suggested choice is MAI Code 2 Flash. If unavailable, mark agent work BLOCKED rather than using a fallback model. Verify the actual
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

- [x] Recheck native compiler, wxWidgets, terminal capability library, Autotools,
  and Xvfb prerequisites.
- [x] Inspect `/home/jeff/ucblogo-code-c` and establish the intended native source
  revision there without disturbing existing changes. Use additional isolated
  source copies for conflicting configurations unless out-of-tree support has
  been verified for all relevant custom recipes. Do not use the active
  `/home/jeff/ucblogo-code` workspace for implementation or build/test execution.
- [x] Configure wx explicitly and confirm that wx was actually selected; do not
  mistake a fallback build for wx coverage.
- [x] Configure terminal-only explicitly with wx and X11 disabled.
- [x] Capture each initial compile/link failure with effective configuration and
  compiler flags. Do not test stale executables after a failed build.

### 2. Repair Shared Compile And Link Boundaries

- [x] Fix conditional Emscripten includes and export annotations.
- [x] Make HTML type guards consistent across definitions and consumers, testing
  whether wx can retain support before restricting HTML strings to web builds.
- [x] Separate web event consumption and scheduler state from native execution.
- [x] Fix the native GC call signature and verify its stack-root contract.
- [x] Compile the touched translation units first, then link each native target.
  After each focused edit, run the cheapest relevant check before expanding scope.

### 3. Restore Native Text And Terminal Behavior

- [x] Rename the current [term.c](../../term.c) implementation to `webterm.c`,
  preserving intentional Web Edition behavior during the move.
- [x] Identify and record the appropriate pre-port revision, then restore legacy
  `term.c` for non-web builds that use the ordinary terminal backend, including
  the native X11 configuration. Reconcile necessary shared API changes rather
  than blindly replacing the file. wx continues to use [wxterm.c](../../wxterm.c).
- [x] Update source selection in [Makefile.am](../../Makefile.am) and any other
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
requirements are identical. The current source selection shares `webterm.c` for
browser and Node because both targets use the same Wasm objects. `print.c` now
checks for the browser output element once before escaping stdout; Node batch
stdout remains literal, while the browser keeps HTML formatting. This is a
partial decision, not a completed host-operation inventory: Node interactive
TTY behavior and DOM-dependent operations in the remaining web terminal words
still need review and validation.

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

- [x] Exercise default and object-enabled native builds. Do not carry the web OOP
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
- [x] Exercise allocation-heavy and repeated-GC cases in native `-O2` builds.
  `-O2` matches GitHub CI's native Autoconf default; the maintainer explicitly
  limited optimization validation to O2. No O3 claim is made.
- [ ] Rebuild Web Edition after shared C changes and verify artifact freshness
  before running browser checks. Preserve Asyncify and browser non-reentrancy.
- [x] Record precise tested configurations and remaining OS/browser coverage gaps.
  Node success does not establish browser or wx GUI correctness.

#### Local wx Build And Tests Matching GitHub CI

Running the Logo unit tests is mandatory for wx build validation. Use the freshly
built native wx executable to run [UnitTests.lg](../../tests/UnitTests.lg) under
Xvfb with the CI invocation below, and grade its individual results. Compilation,
GUI startup, manual smoke checks, or passing Node/browser tests cannot substitute
for this native wx Logo unit-test run. If it cannot run, report wx unit-test
validation as blocked or untested, not passed.

- [x] Run the local equivalent of the existing
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
LOGOLIB="$PROJECT_ROOT/logolib" \
  timeout 120s xvfb-run -e /dev/stdout --auto-servernum \
  "$PROJECT_ROOT/ucblogo" "$PROJECT_ROOT/tests/UnitTests.lg" - -f "$RESULTS" -x

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

- [x] Record local compile/link and graded-test outcomes, configuration, and
  remaining failures. These commands are planned validation, not evidence that
  the current broken sources pass.
- [x] Perform a maintainer-reviewed wx GUI smoke at the user desktop. The
  maintainer manually exercised `doctor` (short ELIZA conversation), `basic`
  (entered and ran a short BASIC program), `mines` (several losses), and
  `dotgame` (several losses), and `cleartext`; the wx session was confirmed
  usable. A separate scripted Xvfb screenshot harness is inconclusive: it
  captured the first stage of each scene, but its timeout screenshots do not
  establish whether later commands ran. Some scenes also remained open after
  `bye`, which the harness incorrectly treats as a failure even though wx stays
  open. This does not block the manually reviewed GUI smoke. The maintainer
  installed `xdotool`; scripted key delivery is available if later needed.

#### Mandatory Unix Native Terminal Logo Unit Tests

- [x] Run [UnitTests.lg](../../tests/UnitTests.lg) locally with the freshly built
  Unix native terminal executable, configured with
  `--disable-wasm --disable-wx --disable-x11 --disable-docs`. Exercise both default
  and object-enabled configurations, selecting the suite's appropriate cases
  without silently omitting failures.
- [x] Reuse the Logo unit-test harness, working directory, `LOGOLIB`, result-file
  options, and per-result grading described above, but invoke the terminal
  executable directly without Xvfb. Verify the harness's invocation and shutdown
  behavior on the terminal target rather than assuming wx and terminal input
  lifecycles are identical.
- [x] Require a successful test process and a fresh, nonempty graded results
  report containing actual test results. Reject failures, errors, timeouts, and
  missing reports; retain the report and logs. Report blocked or unrun tests as
  untested, never passed.
- [x] Treat this as mandatory terminal-build validation. Compilation, interactive
  smoke checks, or passing wx/Node/browser tests cannot substitute for running
  the Logo unit tests on the native terminal executable. Run the planned PTY and
  redirected-stream checks in addition to the unit suite.

### 5. Add Lasting CI And Documentation

- [x] Extend native CI to cover relevant `wasm` development pushes/PRs; the audited
  [workflow](../../.github/workflows/ci.yaml#L3) only triggers for `master`.
- [x] Add terminal-only compile and regression coverage alongside wx, not instead
  of wx. Add a focused PTY regression where practical.
- [ ] If wx HTML rendering is implemented, retain a dependency-light wx CI build
  with rendering disabled and add a separate enabled-renderer check. Document the
  configure switch, extra dependencies, supported markup, and disabled fallback.
- [ ] Keep web regression coverage alongside native checks; coordinate with the
  [browser and Node CI plan](browser-node-ci.md) rather than duplicating it.
- [x] Retain compiler logs and graded results and propagate failures through exit
  status. Keep unsupported/unavailable checks visibly untested.
- [x] Document prerequisites, isolated build commands, supported native terminal
  behavior, and any verified platform limitations.

### 6. Final Browser And Node.js Revalidation In A Fresh Checkout

Run this task last, after all preceding implementation, native validation, CI,
documentation, and any pursued stretch-goal work is complete. Earlier Web Edition
checks do not replace this final clean-checkout validation.

- [x] Prepare an isolated fresh source snapshot containing the exact validated
  working-tree changes. It corresponds to base revision `19a008b` plus the
  staged/unstaged changes in `/home/jeff/ucblogo-code-c`; it is not a Git clone
  because the work remains uncommitted. No work was committed or discarded to
  prepare it.
- [x] Install the project's declared dependencies and configure/build both the
  browser and Node.js targets from source using the supported Emscripten workflow.
  Do not copy native objects, generated configuration, staged resources, loaders,
  Wasm files, or other build outputs from existing working trees. Verify even
  tracked generated outputs are freshly rebuilt before using them.
- [x] Run and grade the Logo unit tests against the newly built Node.js target,
  plus the applicable Node terminal/input/output regressions. Confirm execution
  does not depend on a browser DOM. Report skips and failures explicitly. The
  suite produced 75 `Ok` results with OOP skipped, but the runner exits 1 because
  it cannot recognize `Tests.File.LiteralDribble` when its dribble payload is
  concatenated directly with the `Ok` status. This is the already-known Node
  issue; the test was not modified.
- [x] Run source checks and the functional browser suite, including its graded
  Logo unit tests, against the fresh browser build. Start with desktop Chrome;
  follow the project's broader browser validation order for additional coverage
  and record unavailable or human-coordinated device checks honestly. Source
  checks passed 43 with 4 skipped. Chrome 152 on Linux completed the functional
  suite with 56/57 fixtures passing; the bundled Logo fixture has the same
  `LiteralDribble` grading failure, followed by the expected final-`bye` shutdown
  diagnostic becoming an unhandled Asyncify abort. No source/runtime patch was
  made for this known issue.
- [x] Verify the server root/URL and loaded loader/Wasm/resources belong to this
  checkout. Use a fresh browser origin/profile or bypass HTTP and service-worker
  caches for diagnosis without clearing the user's persistent Logo data.
- [x] Retain build/test logs, graded reports, tool/browser versions, build profile,
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
- The Node.js/browser terminal-sharing decision is recorded; its implementation
  remains deferred. Node/browser operation equivalence is not claimed.
- Native text, workspace persistence, and optimized GC tests pass.
- Fresh Web Edition browser/Node artifacts build successfully. Source checks pass;
  the Chrome functional suite has one accepted known failure, `LiteralDribble`,
  shared with the Node runner. Broader Safari and device coverage is untested.
- CI protects both native targets and the development branch, with web coverage
  retained and remaining platform gaps documented.
- After all other work, browser and Node.js builds have been rebuilt from a fresh
  isolated source snapshot containing the exact working-tree changes. The Node
  Logo runner reports 75 `Ok` results but exits 1 for the known `LiteralDribble`
  result-format issue; Chrome passes 56/57 fixtures, with that same test causing
  the single failure. Artifacts, profile, browser, and hashes were recorded
  during validation. No Safari/device coverage is claimed.

## Validation Status

Source revision `19a008b` was validated in the isolated worktree
`/tmp/ucblogo-native-wx-19a008b`; no native build/configuration was run in the
active Web Edition checkout. The maintainer installed the documented system
prerequisites. The native source checkout was initially clean before edits.

- Linux wx, object-enabled CI profile: build/link passed; Xvfb Logo suite graded
  95/95 passing, including HTML-string construction/equality and repeated GC.
- Linux wx, default non-object profile: build/link passed; Xvfb Logo suite graded
  66/66 passing.
- Linux terminal-only, object-enabled profile: build/link passed; Logo suite
  graded 95/95 passing.
- Linux terminal-only, default profile: build/link passed; Logo suite graded
  66/66 passing.
- Linux native X11 profile: configure selection and build/link passed before the
  final HTML-string constructor regression was added.
- Earlier WebAssembly build using `--enable-wasm --disable-docs --disable-service-worker`:
  passed. Its Node suite predates the new `LiteralDribble` regression and is not
  the final Node result. Earlier desktop Chrome suite: 57/57 passed on that older
  artifact set.
- Native redirected stdout and PTY probes print `<&>` literally; the PTY restores
  terminal attributes after `readchar` and normal exit. Node batch output also
  remains literal; browser output routing retains the existing escaping path.
- Workflow YAML parses successfully; `actionlint` is unavailable.
- Known pre-existing issue: Ctrl-C during `forever [make "x 1]` crashes the
  terminal build with SIGSEGV in roughly 5-10% of PTY runs (6/60 on this branch,
  3/60 on upstream master `3170933`, same failure). Not wx-specific; reported as
  [jrincayc/ucblogo-code#265](https://github.com/jrincayc/ucblogo-code/issues/265).
  The PTY harness retries this one check on signal 11 and notes it on stderr.

The Node runner emits nonfatal `localStorage.getItem` warnings in this
environment. No claim is made that wx HTML is rendered.

### Final Fresh Web Validation (2026-11-14)

The exact uncommitted source snapshot based on revision `19a008b` was synchronized
to `/tmp/ucblogo-web-final-20261114` (not a Git clone; no changes were committed
or discarded). With Emscripten 4.0.5, Node 25.7.0, and Chrome 152 on Linux Mint
22.3, the production Web profile (`-O3`, Asyncify, service worker enabled) built
fresh browser HTML/Wasm, Node JS/Wasm, and Workbox service-worker artifacts.
Because generated help indexes are absent in a fresh source snapshot,
`make regenerate-help-index` was run before completing the build.

- Source checks: 43 passed, 4 skipped, 0 failed.
- Node Logo runner: 75 results printed `Ok`, with OOP skipped, but exits 1 because
  `LiteralDribble` prints `line1a<&>"'zline3Ok` without a line boundary. The
  grader therefore reports `Missing result for Tests.File.LiteralDribble`.
- Desktop Chrome functional suite: 56/57 fixtures passed. The bundled Logo
  fixture reports the same missing `LiteralDribble` result; after `bye`, Chrome
  also reports an unhandled Asyncify rewind abort during the shutdown path.
- Per maintainer direction, `LiteralDribble` is accepted as a known-failing Node
  and browser test for this restoration. No test or runtime behavior was changed
  to mask it. The Node localStorage warnings remain nonfatal.
- Browser artifact hashes: `ucblogo.html` `3c46ac3c1b2fefac32ddadad0c656cbfcc8b54df9d834ab9bdeb632c488a9d52`;
  `ucblogo-node.js` `3fbffa92cebf340ec14972c47f4683a8f563ca8849b21d60b5729d72e47359b0`;
  `ucblogo.wasm` `528c9cb49dd10dd32bb47ab423eed8436a33cff70efa7d3031de1ea9ad070d6f`;
  `ucblogo-node.wasm` `fb91efd38400b098aa767c4989e3f5fb8bfaf476e2b34f0209379e5699fc7dfa`;
  `sw.js` `01ad9150a049f0f5de086d6af4ff1281d07574f473a524496764419238286a36`.
- Firefox, Safari, physical-device, and remote iPad coverage were not run.

### Latest Native O2 Validation (2026-11-14)

Fresh sources were synchronized from `/home/jeff/ucblogo-code-c` into isolated
`/tmp` trees and rebuilt with the CI native optimization level `-O2`. GitHub CI
does not set native optimization flags; Autoconf supplies `-g -O2`. The
maintainer limited optimization checks to O2; O3 was not tested.

- wx object-enabled build: `/tmp/ucblogo-wx-o2`, configured with
  `--disable-wasm --enable-wx --enable-objects --enable-gitid --disable-docs`;
  `HAVE_WX`, `OBJECTS`, and `GITID` verified. Build/link passed. The full current
  Logo suite exited 0 with 105 results and 0 non-Ok results, including
  `Tests.MemMgr.LongList` and `Tests.MemMgr.Garbage`.
- Native terminal object-enabled build: `/tmp/ucblogo-term`, configured with
  `--disable-wasm --disable-wx --disable-x11 --enable-objects --enable-gitid
  --disable-docs`; `WASM_MISSING`, `OBJECTS`, and `GITID` verified. A clean
  rebuild/link passed at `-O2`. The full current Logo suite exited 0 with 105
  results and 0 non-Ok results, including both memory-manager tests.
- An additional 100,000-item allocation/explicit-GC script exited 0 on both
  targets. The terminal printed the expected count `100000`. The wx capture
  contained only Xvfb keymap warnings and no Logo count, so its stress-script
  behavior is inconclusive; the wx memory-manager suite itself passed.
- wx manual smoke was reviewed by the maintainer on the user desktop: `doctor`
  responded to a short conversation, `basic` ran a short program, and `mines`
  and `dotgame` were played through several losses, and `cleartext` was checked.
  The earlier scripted Xvfb screenshot run is inconclusive, not a wx failure:
  its timeout screenshots do not establish whether commands after the first
  marker ran; the later `bye` exit expectation also conflicts with wx remaining
  open.
- The archived master-only wx suite invocation timed out without a report. The
  passing current suite contains the legacy master tests plus the newer tests;
  it is used as the passing superset result, not a separately passing master-only
  invocation.
- The test-owned Xvfb `:109` process was stopped. The maintainer's desktop wx
  session was left untouched.

### Node target: known minor issues (not fixed; low priority)

Found 2026-11-14 with the Node build from the synced tree
(`/tmp/ucblogo-wasm`, `ucblogo-node.js` built 16:01, shared `webterm.c`). The
maintainer ranked the Node build as low priority; these are recorded, not
repaired, and none has a confirmed root cause.

1. `node tests/acceptance/run-logo-node.js` exits 1: 75 passed, 0 failed, 0
   errors, but the runner reports `Missing result for Tests.File.LiteralDribble`.
   Not investigated; the other nine new `Literal*` cases pass on Node. The Node
   target therefore does not currently pass the Logo suite cleanly.
2. A Node batch run that reads stdin (`readword`, `readchar`, `eofp`) from a
   pipe exits with status 1 even when the script ends with `bye`; output is
   otherwise correct. Runs that never read stdin exit 0, and the status was the
   same with and without a trailing newline.
3. A `0xFF` byte on stdin ends a `readword` early (input `a\xffb\n` gave a first
   word of length 1, then `b`), while `c3 bf` read correctly. This is consistent
   with the `char` return type of `em_getc` in `files.c`, but the cause was not
   isolated and the browser path was not tested.
4. The suspected hang at EOF in the Node branch of `em_getc` did not reproduce:
   piped stdin without a trailing newline and `</dev/null` both terminated and
   reported end of input. The mechanism was not traced.
5. Startup prints nonfatal `localStorage.getItem is not a function` traces.

The Node link still uses `webterm.c`; the `nodeterm.c` split is deferred with
this work.

## Handoff

State at handoff (2026-11-14, updated after native O2 validation). All edits are uncommitted in
`/home/jeff/ucblogo-code-c` (staged and unstaged, plus untracked `tests/native/`).
Isolated build trees live under `/tmp`: `ucblogo-term` (fresh native terminal
O2 build, 17:17), `ucblogo-wx-o2` (fresh wx O2 build, 17:20), `ucblogo-wx`
(earlier wx build), `ucblogo-wasm`, `ucblogo-native-wx-19a008b` (older,
Wasm-configured), and `ucblogo-legacy-term-3170933` (upstream master terminal
build, used only for the Ctrl-C comparison). Re-sync a tree with
`/tmp/sync-tree.sh DEST` (copies tracked and untracked non-ignored files; excludes
the nested `ucblogo-code/` clone) and rebuild before trusting a binary.

Decisions made with the maintainer:

- wx HTML: retention only. Document that wx retains `HTML_STRING` and prints
  tagged HTML as markup text. Do not add `--enable-wx-html` or a renderer; mark
  the rendering checklist items deferred, not done.
- wx GUI smoke: the maintainer installed `xdotool` and manually reviewed the
  desktop wx session after exercising `doctor`, `basic`, `mines`, and `dotgame`.
  `cleartext` is also marked complete per maintainer direction. The separate
  Xvfb screenshot/acknowledgement script was inconclusive: timeout screenshots
  captured no evidence that later commands ran, and expecting the wx app to exit
  after `bye` is not a valid success criterion. `keyact` delivery was not
  separately measured.
- Node versus browser terminal: split into `webterm.c` (browser) and a new
  `nodeterm.c`. `__EMSCRIPTEN__` cannot tell the hosts apart, and the Node and
  browser links share `$(ucblogo_OBJECTS)`, so the Node rule in `Makefile.am`
  must swap `ucblogo-webterm.o` for `ucblogo-nodeterm.o`. Shared objects
  (`print.c`, `files.c`, `pre.js`) keep their runtime `typeof document` guards.
  Record the operation-to-backend map here before implementing.
- Scope: remaining implementation includes CI/docs and Task 6. `/tmp` is open for
  isolated work; never touch `/home/jeff/ucblogo-code`. No commits.

- Optimization validation: native GitHub CI uses Autoconf's `-O2` defaults.
  The maintainer authorized O2 only; do not add O3 testing without renewed
  direction.

Verified this session (terminal build in `/tmp/ucblogo-term`; the sources it was
built from match the canonical `main.c`, `print.c`, `term.c`, `files.c` and
`tests/UnitTests-Files.lg`):

- `tests/native/pty_terminal.py`: 13 checks, 0 failed. Covers character-mode and
  normal-exit termios restore, Ctrl-C return to the prompt, xterm `cleartext`,
  `setcursor`, cursor report and `setmargins` sequences, `TERM=dumb` and unset,
  redirected literal stdout, and redirected EOF handling.
- Logo unit suite on that terminal build: 105 results, all `Ok`. The added
  `Tests.File.Literal*` cases cover write/read, list read, multiline read,
  print-to-buffer, dribble, constructors, workspace save/load and a multiline
  procedure round trip; `HtmlStringPrintsMarkup` covers HTML strings.
- Issue [#265](https://github.com/jrincayc/ucblogo-code/issues/265) is open for
  the Ctrl-C SIGSEGV. No root cause is confirmed. A likely cause, not verified, is
  that `logo_stop` in `main.c` runs `err_logo` (allocating and printing) inside a
  real signal handler.
- Unrelated finding: `unblock_input` in `main.c` is guarded by `HAVE_IOCTRL_H`,
  which `config.h` does not define, and the old `longjmp` fallback was removed in
  `e68d709`. It is a no-op on native builds. Not a regression from this work;
  not yet confirmed to matter.

The newer `Literal*` cases and HTML-string test have now passed in the full wx
O2 suite. Node-specific minor issues and unverified host behavior are recorded
above; they remain low priority and are not to be fixed in this scope.

Remaining work, in order:

1. Done: the two `files.c` suspicions were probed on Node; results are in
   "Node target: known minor issues" above. Not fixed, by maintainer decision.
2. Done: fresh native wx and terminal O2 builds; full current Logo suites both
  graded 105/105, including the legacy master cases and both memory-manager
  tests. The wx GUI/CSLS workflows were manually reviewed by the maintainer.
  A master-only wx suite attempt timed out without a report; the current full
  suite passed as the superset. The additional wx 100,000-item stress run is
  inconclusive because its GUI log omitted the expected count.
3. Deferred (low priority): Node terminal split: operation inventory and decision
  record, `nodeterm.c`, `Makefile.am` Node link change, symbol-set comparison
  with `nm`, Node batch and PTY tests. Record minor Node issues, but do not fix
  them unless scope changes.
4. Done: add native PTY and default non-object terminal CI coverage, require
  nonempty graded wx results, document native builds, and update project-map
  anchors. Workflow YAML parses; `actionlint` is unavailable.
5. Done with accepted known failure: build fresh Web/Node artifacts and run source,
  Node, and desktop Chrome checks. Node and Chrome both flag `LiteralDribble`;
  no Safari or physical-device checks were run.

Process rules carry over: three-attempt limit per blocker (all counts are 0 now),
low-cost model policy, stop task-owned processes before finishing, and do not run
`configure` in a tree whose current Makefile still needs `make clean`. The prior
task-owned Xvfb `:109` display has been stopped; no task-owned test process remains.