# Project Plan: Web Edition Editor Syntax Highlighting

Status: Proposed; implementation has not started.
Date: 2026-11-15 (amended: new `<logo-editor>` web component, TDD, milestones)

## Objective

Add Logo syntax highlighting to the Web Edition editor dialog. Instead of
modifying the existing `<textarea>` in place, build a new vanilla-JS web
component, `<logo-editor>`, that owns the editing surface and the highlighting,
develop it test-first in isolation, and then swap it into the editor dialog.

Logo is hard to tokenize, so the tokens come from a table-driven
cellular-automaton tokenizer adapted from the Lhogho compiler
(`src/core/parser.c`, "tokenization uses microprogrammable cell automata"),
compiled to a separate standalone Wasm module, not rewritten from scratch.

## Constraints

- The editor opens while the interpreter holds `LOGO_GIL`, and Asyncify forbids
  reentering the C runtime. The tokenizer therefore lives in its own standalone
  Wasm module (`logohl.wasm`), never in `ucblogo.wasm`.
- The wxWidgets edition is out of scope except that it must not break. Every
  build change sits inside `if WASM`; `ucblogo_SOURCES` and `SUBDIRS` are
  untouched. See the standing WX gate below.
- No framework, bundler, or npm runtime dependency: plain ES2020 in a classic
  script, custom elements, and Shadow DOM, matching the existing
  `logo-autocomplete` and `logo-soft-keys` elements.
- Lhogho is GPLv2-or-later; this repository is GPLv3
  ([LICENSE](../../LICENSE)), so reuse is compatible. Keep an attribution
  header.
- Follow the artifact-freshness rules in
  [copilot-instructions.md](../../.github/copilot-instructions.md#artifact-freshness).
  `sw.js` and other generated outputs are never hand-edited.

## Decisions

- **New component, not an in-place edit.** `<logo-editor>` lives in a new
  source file, `logo-editor.js` (classic script, loaded with `defer` from
  [shell.html](../../shell.html)). It is developed against a standalone harness
  page and touches the real editor dialog only at the integration milestone.
  A separate file keeps `shell.html` from growing and lets the harness load the
  exact production code. Cost: one more file to precache and stage (M1 covers
  it).
- **The component does not reference `Module`.** It takes the tokenizer URL and
  settings through attributes and properties. `pre.js` keeps only `FS` file I/O
  and settings persistence. This is what makes the harness possible.
- **Shadow DOM** with an internal `<textarea>` as the input surface and a
  `<pre aria-hidden="true">` highlight layer behind it. The native textarea
  keeps IME, mobile keyboards, selection, and screen-reader editing semantics;
  the component never implements its own caret or selection.
- **Textarea-compatible API** on the host (see below) so `logo-autocomplete`,
  `Module.insertAtCaret`, and existing tests keep working with minimal change.
- Separate standalone Wasm module, no Emscripten JS glue, fetched and
  instantiated by the component.
- The module embeds primitive names generated from `helpfiles/ALL_NAMES`; user
  procedures come from `to` headers in the buffer.
- Spans use UTF-16 code-unit indices, so JS needs no UTF-8 offset conversion.
  The module uses no libc and has no imports.
- Out of scope: wx/native editor, semantic checks (arity, undefined
  procedures), incremental tokenization, line numbers, and changes to
  `ucblogo.wasm`.

## Current State

- The editor is `<textarea id="editorTextArea">` inside `.editorTextContainer`
  in [shell.html](../../shell.html), with a `<logo-autocomplete for=...>`
  sibling.
- `Module.editor` in [pre.js](../../pre.js) opens, applies, and discards edits
  through `editorTextArea.value`.
- `LogoAutocomplete` ([shell.html](../../shell.html)) uses its input's `id`,
  `value`, `selectionStart`, `setSelectionRange`, `focus`, `setAttribute`
  (`role`, `aria-*`), and `input`, `blur`, `click`, `keydown` listeners.
  `accept()` and `Module.insertAtCaret` set `value` programmatically, which
  does not fire `input`.
- Existing tests reference `#editorTextArea` directly:
  [autocomplete.testcafe.js](../../tests/acceptance/autocomplete.testcafe.js),
  [settings-persistence.testcafe.js](../../tests/acceptance/settings-persistence.testcafe.js),
  [help-dialog.testcafe.js](../../tests/acceptance/help-dialog.testcafe.js),
  and the mocks in [source.test.js](../../tests/acceptance/source.test.js).
- [workbox-config.js](../../workbox-config.js) precaches three named files;
  [scripts/stage-web-deploy.js](../../scripts/stage-web-deploy.js) has a
  `REQUIRED` list.
- The Lhogho automaton is a nested function in `tokenize()`, driven by
  `MODE_*` x character-class tables of `PAT_*` microprogram bits and coupled to
  its atom and error types, so it is reimplemented, not copied.

## Component Contract

`<logo-editor>`:

- Properties: `value`, `selectionStart`, `selectionEnd`, `setSelectionRange()`,
  `select()`, `focus()`, `blur()`, `readOnly`, `highlighting` (boolean
  attribute, default on), `tokenizerUrl` (default `logohl.wasm`).
- Events: native `input`, `keydown`, `click`, and `blur` observable on the host.
  Programmatic `value` assignment does not fire `input` (as with a textarea) but
  still re-renders the highlight. New events: `highlightready` after each
  render with `detail.spans` (the test synchronization point, so tests never
  sleep) and `highlighterror` when the module cannot load.
- Accessibility: the host forwards `role` and `aria-*` attributes and the label
  (`aria-label` or `aria-labelledby`) to the inner textarea, delegates focus,
  and marks the highlight layer `aria-hidden`.
- Styling: font, color, and background inherit; token colors are CSS custom
  properties (`--logo-editor-comment`, and so on) with per-theme defaults;
  `::part(input)` and `::part(highlight)` are exposed.
- Fallback: if the module fails to load, or the input exceeds the size cap, the
  component behaves as a plain textarea with visible text and no overlay.

## Test-Driven Development Rules

1. **Red first.** For every behavior, write the failing test, run it, and
   confirm it fails for the intended reason before writing production code.
2. **Smallest green.** Write only enough code to pass, then refactor with the
   tests green.
3. **Commit the checkpoints.** Commit the red test, then the green
   implementation, as separate local commits where practical (see the commit
   policy in [copilot-instructions.md](../../.github/copilot-instructions.md)).
   Intermediate commits need not build.
4. **Pick the cheapest level that can fail.** Tokenizer behavior is tested in
   Node against `logohl.wasm` alone. Component behavior is tested in the browser
   against the harness page, with no Logo runtime. Only integration behavior
   uses the full app.
5. **Deterministic waits.** Wait on `highlightready`, never on timers.
6. **Test IDs.** Each test gets an ID listed in
   [tests/acceptance/COVERAGE.md](../../tests/acceptance/COVERAGE.md) in the
   same commit that adds it.
7. **Rebuild before browser tests** and check artifact freshness; run focused
   tests while iterating and the broader matrix only at milestone exit.

Test levels and files:

| Level | Runner | File | Needs |
| --- | --- | --- | --- |
| Tokenizer | `node --test` | `tests/acceptance/highlight.test.js` | `logohl.wasm` only |
| Component | TestCafe | `tests/acceptance/logo-editor.testcafe.js` on `tests/acceptance/fixtures/logo-editor-harness.html` | `logo-editor.js`, `logohl.wasm`, static server |
| Integration | TestCafe | `tests/acceptance/editor-highlight.testcafe.js` | full app build |
| Build and deploy | `node --test` | `stage-web-deploy.test.js`, `source.test.js` | built artifacts |

New npm scripts: `test:highlight` (tokenizer, also added to `test:source`),
`test:editor` (component), and `test:editor:integration`.

## Milestones

Each milestone ends with its exit criteria met, the standing gates passed, and
a local commit. Do not start a milestone until the previous one is green.

### M1: Walking skeleton (pipeline first)

Prove the riskiest infrastructure with the thinnest possible feature.

- **Tests first (red):**
  - `highlight.test.js`: `logohl.wasm` exists, instantiates with no imports,
    and tokenizes `abc` into one word span.
  - `stage-web-deploy.test.js`: `logohl.wasm` and `logo-editor.js` are required
    and precached.
  - `logo-editor.testcafe.js`: `logo-editor` is defined; `value` round-trips;
    typed text appears in the highlight layer.
- **Build:** `highlight/logohl.c` stub (single class); `logohl.wasm` rule in
  [Makefile.am](../../Makefile.am) under `if WASM` (no entry point, standalone,
  no filesystem, exported functions only); `WEB_TARGETS`, `CLEANFILES`, `sw.js`
  dependency, `EXTRA_DIST`; `logohl.wasm` and `logo-editor.js` in `globPatterns`
  and `REQUIRED`; harness page; stub `logo-editor.js` with a textarea plus a
  mirrored `<pre>`.
- **Exit:** all three tests green; `npm run build:web` still passes; the staged
  deploy verifies; the WX gate passes.

### M2: Tokenizer core (lexical classes)

- **Tests first (red):** golden-span cases, one per lexical rule, derived from
  [parse.c](../../parse.c): words, quoted words (`"x`, including `"[`),
  variables (`:x`), numbers (integer, decimal, exponent), infix operators
  (`+ - * / = < > <= >= <>`), parens, list brackets, array braces, comments
  (`;`), continuations (`~`), `|barred|` words, backslash escapes, LF, CR, and
  CRLF line ends, and an unterminated `|` as an error span. Property checks over
  random input: spans stay in bounds, are ordered and non-overlapping, and no
  input crashes or hangs. Edge cases: empty input, over-cap input, astral
  characters (UTF-16 offsets).
- **Implement:** the `mode x char-class -> microprogram` tables from the Lhogho
  automaton (entry, space, word, barred, backslashed, tilde, semitilde,
  semicolon, tildespace, less, greater and the `PAT_*` actions), emitting spans
  and keeping the comments and continuations that Lhogho discards. Add the
  attribution header.
- **Exit:** tokenizer suite green; tokenizing is linear time (a 1 MB input
  completes within a stated budget); the module still has no imports.

### M3: Structure and primitives

- **Tests first (red):** `to NAME :a :b ... end` marks the procedure name and
  parameters; bare words that are primitives (case-insensitive, including
  aliases) get the primitive class and other words do not; user procedures
  defined in the buffer are marked at their call sites; unbalanced `[`, `]`,
  `(`, `)`, `{`, `}` and a stray `end` get the error class; every entry in
  `helpfiles/ALL_NAMES` classifies as a primitive. Where practical, add a
  differential check of word boundaries against the interpreter's own parse
  using `ucblogo-node.js`.
- **Implement:** `scripts/build-highlight-names.js` generating the sorted
  `highlight/primitives.h` from `ALL_NAMES` (after `regenerate-help-index`),
  binary search in the module, and the structural pass.
- **Exit:** suite green; the generated header is a build product, not committed.

### M4: Component rendering

- **Tests first (red), in the harness:** each token class renders with its class
  and a distinct computed color; overlay and textarea line boxes align for long
  lines, wrapping, tabs, a trailing empty line, and non-ASCII text; scroll
  position mirrors in both directions, including after resize; `highlighting`
  off or over-cap input shows plain visible text; a failed module load emits
  `highlighterror` and leaves a working plain editor; `forced-colors` shows
  native text; light and dark themes and the `logo-text-colors-set` custom
  colors meet AA contrast.
- **Implement:** a DOM builder that turns spans into `<span class="hl-...">`
  using `textContent` only (never `innerHTML`), `requestAnimationFrame`
  debouncing, `ResizeObserver`, matched metrics and `scrollbar-gutter`, a
  trailing sentinel, and the `editor-highlighting` state that makes textarea
  text transparent only after a successful render.
- **Exit:** component suite green in desktop Chrome.

### M5: Textarea-compatible contract and accessibility

- **Tests first (red), in the harness:** the exact API surface
  `logo-autocomplete` and `Module.insertAtCaret` use (`value`, the selection
  getters and `setSelectionRange`, `focus`, `id`, attribute forwarding, and
  `input`, `blur`, `click`, and capturing `keydown` listeners on the host);
  programmatic `value` updates re-render without firing `input`; the accessible
  name, role, and focus order are correct (focus delegation, forwarded `aria-*`,
  highlight layer hidden from assistive technology); typing, paste, undo, and
  IME composition do not lose or duplicate text or move the caret.
- **Implement:** the host API, attribute forwarding through a
  `MutationObserver`, focus delegation, and a single `refresh()` path used by
  every setter.
- **Exit:** suite green; an accessibility checklist is recorded in the test
  file.

### M6: Integration into the editor dialog

- **Tests first (red):** update the existing tests that reference
  `#editorTextArea` (autocomplete, settings persistence, help dialog, the
  `source.test.js` mocks) to the component contract, and add
  `editor-highlight.testcafe.js` covering: opening the editor from Logo while
  the runtime is busy (the original `LOGO_GIL` constraint); highlighted spans and
  distinct colors in the real app; the autocomplete accept path and
  `insertAtCaret` re-highlighting; Apply and Discard preserving behavior; the
  persisted "Editor syntax highlighting" setting acting as a kill switch; and
  fallback when `logohl.wasm` is blocked.
- **Implement:** replace the textarea in [shell.html](../../shell.html) with
  `<logo-editor id="editorTextArea">` (keeping the id so selectors and
  `<logo-autocomplete for>` keep working), add
  `<script src="logo-editor.js" defer>`, point `Module.editor` in
  [pre.js](../../pre.js) at the component, and add the setting through the
  existing single-setter pattern (default on). Remove the now-dead textarea CSS.
- **Exit:** full integration suite green in desktop Chrome and phone emulation;
  all pre-existing editor and autocomplete tests pass, changed only for the new
  element.

### M7: Cross-browser, performance, and offline hardening

- **Tests first (red):** a latency budget for tokenize plus render on a 100 KB
  buffer; behavior at and above the size cap; offline use through the service
  worker with `logohl.wasm` and `logo-editor.js` precached; Lighthouse
  accessibility and performance not regressed.
- **Validate** in the project's browser order: desktop Chrome, mobile Chrome (or
  device emulation, reported as emulation), Firefox, then iPad Safari with the
  maintainer. Note the desktop Safari gap. Fix the alignment and metric drift
  found per browser, with a regression test for each.
- **Exit:** the matrix is recorded in
  [tests/acceptance/COVERAGE.md](../../tests/acceptance/COVERAGE.md); stale-
  artifact checks are done (HTTP cache disabled, service worker bypassed, fresh
  profile, full rebuild).

### M8: Documentation and release readiness

- Document the component, the setting, and the Lhogho attribution in
  [webedition.md](../../webedition.md) and the test docs.
- Verify the staged deploy, and the live site after release, with the
  build-deploy skill's checks.
- **Exit:** docs reviewed; no open TODOs in the new files.

## Standing Gates (every milestone that touches build files)

- **WX safety:** `git diff` shows no changes to `*.cpp`, `wx*`, `TextEditor.*`,
  `ucblogo_SOURCES`, or `SUBDIRS`, and every [Makefile.am](../../Makefile.am)
  change sits inside `if WASM`. Configure and build the native non-wasm target in
  a separate isolated build tree and confirm it builds unchanged. Do not
  reconfigure the in-tree Makefile.
- **Freshness:** `npm run build:web`, restart the server on the rebuilt files,
  and bypass the service worker and HTTP cache before trusting a browser result.
- **Security:** no `innerHTML` with user text; the module reads only its own
  buffer; the size cap bounds work.

## Open Questions

1. Does the setting ship on by default? Recommendation: on.
2. What buffer cap skips highlighting? Recommendation: 256 KB, tuned in M7.
3. Should `<logo-editor>` later replace the command-line input too? Out of scope
   here; the contract is designed so it could.

## Risks

- Overlay and textarea metric drift (fonts, wrapping, scrollbars, mobile
  Safari). Mitigation: alignment tests in M4 and M7, the kill switch, and the
  `forced-colors` fallback.
- Cross-shadow ARIA: `aria-activedescendant` and label association do not cross
  shadow roots. Mitigation: forwarding and focus delegation in M5, verified with
  accessibility-tree assertions; revisit light-DOM rendering if it cannot be
  made to work.
- Native undo history is lost on programmatic `value` assignment. This matches
  the current editor; M5 records the behavior rather than changing it.
- Tokenizer drift from `parse.c`. Mitigation: golden tests derived from its
  lexical rules and the differential check in M3.
- Two more shipped artifacts in the service-worker cache. Mitigation: precache,
  staging, and freshness steps in M1 and M7.
