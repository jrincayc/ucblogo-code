# Test Plan: UCB Logo Turtle Graphics Skill

Status: Draft; Test Plan QA and human approval pending.
Task: TGS-2026-11-20-T1
Date: 2026-11-20
Approved Plan: [turtle graphics skill plan](./turtle-graphics-skill.md)
Plan SHA-256: `6b10fabda17a78bf9b9b7aac2953b99b599153e7a83b4c0fd6da4e1b1bf8a2eb`
Target branch: `wasm`

## Purpose And Scope

Specify static-document checks and browser execution checks for the four
skill documents authorized by the approved Plan:

- `.github/skills/ucb-logo-turtle-graphics/SKILL.md`
- `.github/skills/ucb-logo-turtle-graphics/references/turtle-primitives.md`
- `.github/skills/ucb-logo-turtle-graphics/references/turtle-patterns.md`
- `.github/skills/ucb-logo-turtle-graphics/references/platform-notes.md`

The documents are the test suite for user-facing examples. The browser fixture
must parse each fenced block whose language is exactly `logo` (case-sensitive),
use the block text as the Logo source, and execute it without copying the code
into a checked-in fixture. Fragments such as syntax illustrations must be
fenced with another language or explicitly marked non-runnable; only complete
standalone examples belong in `logo` fences.

The test target covers frontmatter, links, primitive naming/classification,
the shared drawing-state contract, platform caveats, scope, attribution, and
execution of every complete `logo` example. It does not test Node Logo behavior,
wx, mobile Chrome, device emulation, or another browser.

## Test Artifacts And Fixtures

During Implement Tests, create one persistent TestCafe fixture:

`tests/acceptance/turtle-graphics-examples.testcafe.js`

The fixture owns four concerns:

1. Parse the actual four Markdown documents with the repository's Markdown
   parser, identifying each fence token whose `info` is exactly `logo`.
2. Write each fence body verbatim to a task-owned temporary `.lg` file under
   `/tmp/copilot/TGS-2026-11-20-T1/examples/`; no checked-in source copy is made.
3. Load that file through the Web Edition upload control in a fresh page/runtime,
   wait on observable Logo readiness and completion, and evaluate browser/Logo
   diagnostics.
4. Write `reports/turtle-graphics-examples/report.json` and
   `reports/turtle-graphics-examples/report.md`, linking each document/fence
   identity to its PNG screenshot and result. Report files and screenshots are
   generated artifacts, not source fixtures.

Use one deterministic example test per fence generated at fixture load time. If
there are no runnable `logo` fences, the fixture must fail: a zero-example pass
is not acceptable. The source documents are the reusable fixture data. No mock
Logo source, duplicated sample, external payload, service, or secret is needed.
A compact in-memory manifest may contain only document paths, fence index, and
generated temporary-file/screenshot paths.

Static checks use a temporary Node test file at:

`/tmp/copilot/TGS-2026-11-20-T1/turtle-graphics-skill.test.js`

Use Node's built-in `node:test` and `node:assert/strict`, the locally available
`markdown-it`, and Ruby Psych for safe YAML parsing. Do not install dependencies.
The static harness remains temporary; the browser TestCafe fixture is persistent.

## Environment And Setup

Run from `/home/jeff/ucblogo-code` on branch `wasm`. Before a run, record:

- Git HEAD and dirty state;
- Web artifact identity and freshness (including whether generated artifacts
  correspond to current source);
- Node, Ruby, TestCafe, Chrome version/user agent, runner options, and report
  paths.

Use the shared runner only:

```sh
TESTCAFE_BROWSER=chrome TESTCAFE_HEADLESS=1 \
  node tests/acceptance/run-testcafe.js \
  --fixture 'Turtle graphics documentation examples' \
  --screenshots path=reports/turtle-graphics-examples/screenshots \
  --reporter 'spec,json:reports/testcafe.json'
```

The fixture must enforce `TESTCAFE_BROWSER=chrome` and headless operation rather
than accept an inherited mobile/emulation/Firefox target. It must not use raw
TestCafe `--app` commands or fixed startup delays. Use `ready`/observable idle
state and the shared runner's server, readiness, watchdog, and process cleanup.
Do not run Logo commands in Node.

Mobile Chrome, device emulation, headed Chrome, wx, and non-Chrome browsers are
outside this focused fixture. Existing broader coverage is not repeated.

For every generated example test, navigate to `ucblogo.html` in a fresh TestCafe
page context. Wait for the existing welcome output and idle predicate. Capture
output and diagnostics before upload, select the exact generated `.lg` file
through `#uploadFileInput`, then wait until Logo has loaded and returned to
idle. A fixed timeout is only a failure bound, never a readiness delay.

At teardown, preserve the report and screenshots, then remove only the temporary
extracted `.lg` files and task-owned `/tmp/copilot/TGS-2026-11-20-T1/` contents.
Do not remove unrelated `reports/` data or pre-existing screenshots.

## Cases And Expected Results

| ID | Input / procedure | Expected result |
| --- | --- | --- |
| TP-01 Package inventory | Enumerate `.github/skills/ucb-logo-turtle-graphics/`. | Exactly `SKILL.md` and the three approved references exist; the only separate task-owned test source is `tests/acceptance/turtle-graphics-examples.testcafe.js`. |
| TP-02 Skill frontmatter | Extract and safely parse initial YAML. | One valid mapping; exact skill name; nonempty <=1024-character discovery-oriented description; optional argument hint is valid. |
| TP-03 Entry point workflow | Inspect `SKILL.md` and links. | Under 500 lines; states Web default, wx note, state-first workflow, and links to all references with one-level relative paths. |
| TP-04 Markdown and links | Parse all four documents and traverse link tokens. | All relative paths resolve; external schemes are approved; fences are structurally valid. |
| TP-05 Primitive inventory audit | Audit taught words and classifications against current local evidence. | Every taught primitive/alias/library/Web word is evidenced and correctly classified; `pendownp`/`pendown?` is correct; `pen?` and stale/unregistered names are not taught as working commands. Return the name/source audit to QA. |
| TP-06 State-first setup | Inspect setup prose and every runnable example's setup. | Every relied-on mutable state is established: relevant color, size, mode, up/down, visibility, background, position/heading, wrap/fence/window, palette, scrunch, and shape. State is assumed only in a pristine session or after a deliberate recent setting without intervening mutation. |
| TP-07 Clear and pen snapshot semantics | Inspect content and compare with approved state report. | `CLEARSCREEN` erases and homes position/heading but preserves the other drawing state; `CLEAN` erases without moving the turtle; neither is a full reset. `PEN`/`SETPEN` are not full turtle/screen/palette snapshots. Possible split-screen/layout side effect is labeled source-backed and not runtime-verified. |
| TP-08 Platform note | Inspect platform reference. | Web is default; 640 x 480 is current implementation behavior, not permanent; wx bounds are host-dependent; no additional dimension promise/research; source availability is not treated as visual parity. |
| TP-09 Contract consistency | Compare all four documents and the approved Plan. | No contradiction in state, aliases, cleanup, dimensions, scope, or rendering claims. |
| TP-10 Runnable fence inventory | Parse all four documents. | At least one complete `logo` fence exists. Each is identified by repository path and one-based fence ordinal; no complete executable example is hidden in a non-`logo` fence; non-runnable snippets are distinctly labeled/fenced. |
| TP-11 Exact extraction | For every `logo` fence, compare temporary `.lg` bytes with the Markdown token content. | Temporary source equals the fence content exactly (after the parser-defined trailing newline behavior), contains no prefixed test wrapper, and is attributable to one document/fence identity. |
| TP-12 Fresh browser runtime | For every generated example, load a fresh `ucblogo.html`, establish welcome/idle, and upload the extracted file. | New context prevents prior pen/turtle/background/palette state leakage; setup succeeds; browser is desktop headless Chrome, not emulation/mobile. |
| TP-13 Browser Logo grading | For each uploaded example, wait for upload/load completion and idle. | No uncaught JS error, `javascriptError` content, Logo runtime failure/error marker attributable to the example, unexpected loader failure, timeout, or failure to return idle. Diagnostic output is retained on failure. An intentionally empty/no-op or incomplete fence is not acceptable. |
| TP-14 Screenshot per example | After execution reaches a terminal pass or failure state, capture a PNG. | Exactly one linked screenshot exists for every fence. Pass screenshots show terminal and graphics state; failure screenshots retain the visible diagnostic state. Missing/unreadable/zero-byte screenshots fail. |
| TP-15 Report integrity | Inspect `report.json` and `report.md` after all examples. | One entry per parsed fence, stable document/fence identity, source path, result, browser/Chrome version, artifact identity, execution/diagnostic summary, and relative screenshot path. Counts reconcile: entries = fences = screenshots; no copied Logo body is required in the report. |
| TP-16 Runner/artifact discipline | Inspect runner configuration and report metadata. | Shared runner owns server/process cleanup; desktop `chrome:headless` is enforced; no raw TestCafe `--app`, Node Logo invocation, fixed startup sleep, wx run, or other browser. Report identifies the Web build revision/artifact source and whether it was fresh. |
| TP-17 Scope, attribution, and diff | Inspect source/report inventory and footer; run scoped whitespace checks. | Four approved skill documents, one authorized TestCafe fixture, generated report/screenshot outputs only. Each source artifact has one Copilot attribution; no unrelated change; scoped `git diff --check` exits 0. |

## Validation Commands

### Implement Tests Baseline

Before skill documents exist, create the persistent fixture and temporary static
harness. Run:

```sh
node --test /tmp/copilot/TGS-2026-11-20-T1/turtle-graphics-skill.test.js
```

Expected red baseline: the static harness starts correctly and fails for the
missing four-document package. For the browser fixture, run only a collection or
fixture-startup check through the shared runner. Expected result: it clearly
fails because there are no runnable `logo` fences; it must not silently pass a
zero-example suite. A parser, TestCafe, server, screenshot, or setup failure is
not an acceptable intended-red result.

### Final Validation

After all skill documents and the fixture are integrated, run the narrow static
harness, then the focused browser fixture:

```sh
node --test /tmp/copilot/TGS-2026-11-20-T1/turtle-graphics-skill.test.js
TESTCAFE_BROWSER=chrome TESTCAFE_HEADLESS=1 \
  node tests/acceptance/run-testcafe.js \
  --fixture 'Turtle graphics documentation examples' \
  --screenshots path=reports/turtle-graphics-examples/screenshots \
  --reporter 'spec,json:reports/testcafe.json'
git diff --check -- \
  .github/skills/ucb-logo-turtle-graphics \
  tests/acceptance/turtle-graphics-examples.testcafe.js
```

Expected: all static tests and all generated example tests pass; report entry,
fence, and screenshot counts reconcile; both report formats exist; the runner
cleans up owned processes; `diff --check` exits 0. Inspect the TestCafe JSON and
the custom report rather than trusting process exit alone.

### Test Plan Artifact Validation

Validate this plan's local links/headings and run:

```sh
git diff --check -- docs/plans/turtle-graphics-skill-test-plan.md
```

This stage does not run the fixture or execute examples. Those are explicitly
reserved for Implement Tests and later approved validation stages.

## Baseline And Regression Scope

The package and browser fixture do not exist at baseline. Implement Tests must
show an intended red result caused by absent package/examples, not harness
syntax or browser setup. Once the documents exist, unchanged criteria must pass.

Regression coverage is the four documents and the mechanism that executes them
from source. Static checks protect discovery, links, names, state semantics,
platform caveats, and scope. Browser checks protect actual UCB Logo execution,
state isolation, rendering of turtle output, diagnostics, and screenshot/report
coverage. The full existing suite remains responsible for other-browser graphics
and unrelated regression coverage; this focused fixture does not duplicate it.

## Risks And Limitations

- A screenshot establishes only captured evidence for that run; it does not prove
  rendering parity with wx or another browser.
- Logo source ending in interactive `read` commands, infinite loops, `bye`, or
  other process-ending/state-dependent behavior is unsuitable as a complete
  example unless the test protocol defines and verifies it. Prefer standalone
  finite drawing examples with visible terminal/graphics completion.
- A page/runtime is fresh per example, but each example still must establish the
  mutable state it relies on; the test does not excuse weak documentation.
- Uploading a `.lg` file exercises the Web Edition loader and graphics runtime,
  not Node. Loader/output error text must be distinguished from successful Logo
  execution; missing completion evidence is a failure.
- The custom report is generated by the fixture. If the browser or TestCafe
  crashes before finalization, the failure screenshot/runner JSON must remain
  available, and the missing custom report is itself a reported failure.
- Phrase assertions are brittle. Pair them with the primitive source audit and
  human review of example setup/state semantics.
- No dependency installation is authorized. Missing parser, TestCafe, Chrome,
  or usable Web artifacts is a blocker, not permission to substitute tools.
- A change that adds a sprite demo, Node/wx execution, another browser, or
  additional source file reopens the earliest affected stage.

## Gate State

This is a Test Plan deliverable only. It defines, but does not implement, the
TestCafe fixture, static harness, browser report, or screenshots. Independent
Test Plan QA and explicit human approval are required before Implement Tests.
The visual design gate remains N/A: screenshots are runtime test evidence, not
a proposed product visual change.

<!-- Co-authored by GitHub Copilot. Requested model: GPT-6 Luna; active model and reasoning-effort metadata: unverified. -->