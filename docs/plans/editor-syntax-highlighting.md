# Project Plan: Web Edition Editor Syntax Highlighting

Status: Proposed; implementation has not started.
Date: 2026-11-15

## Objective

Highlight Logo source in the Web Edition's built-in editor dialog. Logo is
hard to tokenize, so reuse the table-driven cellular-automaton tokenizer from
the Lhogho compiler (`src/core/parser.c`, "tokenization uses microprogrammable
cell automata") instead of writing a new parser.

## Constraints

- The editor opens while the interpreter holds `LOGO_GIL`, and Asyncify forbids
  reentering the C runtime. The tokenizer therefore lives in a separate,
  standalone Wasm module, not in `ucblogo.wasm`.
- The wxWidgets edition is out of scope except that it must not break. Every
  build change sits inside `if WASM`; `ucblogo_SOURCES` and `SUBDIRS` are
  untouched.
- Lhogho is GPLv2-or-later; this repository is GPLv3 ([LICENSE](../../LICENSE)),
  so reuse is compatible. Keep an attribution header.
- Follow the artifact-freshness rules in
  [copilot-instructions.md](../../.github/copilot-instructions.md#artifact-freshness).
  `sw.js` and other generated outputs are never hand-edited.

## Decisions

- Separate standalone Wasm module (`logohl.wasm`) with no Emscripten JS glue,
  fetched and instantiated from `pre.js`.
- Highlight layer is a `<pre>` behind a transparent `<textarea>`, which keeps
  native input, IME, mobile behavior, and `logo-autocomplete`.
- The module embeds primitive names generated from `helpfiles/ALL_NAMES`; user
  procedures come from `to` headers in the buffer.
- Spans use UTF-16 code-unit indices, so JS needs no UTF-8 offset conversion.
  The module uses no libc and has no imports.
- Out of scope: wx/native editor, semantic checks (arity, undefined
  procedures), incremental tokenization, and changes to `ucblogo.wasm`.

## Current State

- The editor is `<textarea id="editorTextArea">` inside `.editorTextContainer`
  in [shell.html](../../shell.html), with a `<logo-autocomplete>` sibling.
- `Module.editor` in [pre.js](../../pre.js) opens, applies, and discards edits.
- `LogoAutocomplete.accept` and `Module.insertAtCaret` set `value`
  programmatically, so they do not fire `input`.
- [workbox-config.js](../../workbox-config.js) precaches three named files;
  [scripts/stage-web-deploy.js](../../scripts/stage-web-deploy.js) has a
  `REQUIRED` list.
- The Lhogho automaton is a nested function in `tokenize()`, driven by
  `MODE_*` x character-class tables of `PAT_*` microprogram bits and coupled to
  its atom and error types, so it is reimplemented, not copied.

## Steps

### Phase 1: Tokenizer (C)

1. Create `highlight/logohl.c`: static UTF-16 input and span buffers and a
   small ABI (buffer pointer, `hl_tokenize(len)` returning the span count, span
   pointer). Spans are `(start, end, class)`. Inputs over a fixed cap are
   rejected so JS can skip highlighting.
2. Reimplement the Lhogho automaton as `mode x char-class -> microprogram`
   tables. Keep the modes (entry, space, word, barred, backslashed, tilde,
   semitilde, semicolon, tildespace, less, greater) and the `PAT_*` actions.
   Drop the buffer, atom, and error-atom coupling and emit spans, including
   comments and continuations that Lhogho discards. Depends on 1.
3. Adapt the rules to UCBLogo using [parse.c](../../parse.c) as the lexical
   reference. Token classes: word, quoted word (`"x`), variable (`:x`), number,
   infix operator, paren, list bracket, array brace, comment (`;`),
   continuation (`~`), barred or backslash escape, and an error class for an
   unterminated `|`. Add a structural pass for `to NAME :params ... end` and
   unbalanced brackets. Depends on 2.
4. Add `scripts/build-highlight-names.js` to turn
   [helpfiles/ALL_NAMES](../../helpfiles/ALL_NAMES) into a sorted header
   `highlight/primitives.h`; the module classifies bare words by
   case-insensitive binary search. Runs after `regenerate-help-index`.
   Parallel with 2-3.
5. Put a Lhogho attribution and GPL notice in the file header.

### Phase 2: Build, cache, and deploy (all under `if WASM`)

6. In [Makefile.am](../../Makefile.am), add a `logohl.wasm` rule built with
   `emcc` (no entry point, standalone Wasm, no filesystem, exported functions
   only), independent of `$(ucblogo_OBJECTS)`. Add it to `WEB_TARGETS`,
   `CLEANFILES`, and the `sw.js` dependency, and the highlight sources to
   `EXTRA_DIST`. Depends on 1-4.
7. Add `logohl.wasm` to `globPatterns` in
   [workbox-config.js](../../workbox-config.js) so it is precached, to
   `REQUIRED` in [scripts/stage-web-deploy.js](../../scripts/stage-web-deploy.js),
   and update
   [tests/acceptance/stage-web-deploy.test.js](../../tests/acceptance/stage-web-deploy.test.js).
   Depends on 6.

### Phase 3: Editor integration

8. In [pre.js](../../pre.js), extend `Module.editor` with a highlighter that
   lazily runs `fetch('logohl.wasm')` and `WebAssembly.instantiateStreaming`
   (falling back to `instantiate` on a wrong MIME type), writes
   `charCodeAt` values into the module's `Uint16Array`, reads spans back, and
   builds `<span class="hl-...">` nodes with `textContent` (never `innerHTML`).
   It re-tokenizes the whole buffer with `requestAnimationFrame` debouncing. On
   load failure highlighting stays off and the native textarea text remains
   visible. Depends on 1.
9. In [shell.html](../../shell.html), add
   `<pre id="editorHighlight" aria-hidden="true">` inside
   `.editorTextContainer` before the textarea. Make textarea text transparent
   with a visible `caret-color` only under an `editor-highlighting` class that
   JS adds after a successful load. Match font, line height, padding, border,
   `tab-size`, wrapping, and `scrollbar-gutter` between the layers; mirror
   scroll on `scroll`, resize via `ResizeObserver`, and add a trailing sentinel
   for a final empty line. Disable the overlay under `forced-colors: active`.
   Parallel with 8.
10. Define token colors as CSS variables per theme and dark mode, meeting AA
    contrast, and make them coexist with the `logo-text-colors-set` setting.
    Depends on 9.
11. Refresh the overlay on `input` and on every programmatic value change
    (`Module.editor.edit`, `Module.insertAtCaret`, `discardClicked`,
    `LogoAutocomplete.accept`) through one `refreshHighlight()`. Depends on
    8-9.
12. Add a persisted "Editor syntax highlighting" setting, default on, through
    the existing single-setter pattern. It is a kill switch for overlay
    misalignment on specific browsers. Depends on 8-9.

### Phase 4: Verification

13. Node unit tests: `tests/acceptance/highlight.test.js`, added to
    `test:source` in [package.json](../../package.json). Load `logohl.wasm`
    directly in Node. Golden-span cases per lexical rule, empty input,
    over-cap input, and astral or emoji text for UTF-16 offsets. Assert zero
    imports, and that every `ALL_NAMES` entry classifies as a primitive.
    Depends on 6.
14. TestCafe: `tests/acceptance/editor-highlight.testcafe.js`, modeled on
    [autocomplete.testcafe.js](../../tests/acceptance/autocomplete.testcafe.js).
    Cover opening the editor while the runtime is busy (the original
    constraint), span classes and distinct computed colors, overlay and
    textarea alignment, scroll sync, wrapping, the autocomplete accept path, the
    kill switch, and fallback when `logohl.wasm` is blocked. Run desktop Chrome,
    then phone emulation, then Firefox; iPad Safari last with the maintainer.
    Report emulation as emulation and note the desktop Safari gap. Depends on
    8-12.
15. WX safety: `git diff` shows no changes to `*.cpp`, `wx*`, `TextEditor.*`,
    `ucblogo_SOURCES`, or `SUBDIRS`, and every `Makefile.am` change sits inside
    `if WASM`. Configure and build the native non-wasm target in a separate
    isolated build tree and confirm it builds unchanged.
16. Before reporting results, run `npm run build:web`, restart the server on the
    rebuilt files, and bypass the service worker and HTTP cache so stale
    `ucblogo.html`, `sw.js`, or `logohl.wasm` is ruled out.

## Open Questions

1. Does the setting ship on by default? Recommendation: on.
2. What buffer cap skips highlighting? Recommendation: 256 KB, tuned after
   profiling.

## Risks

- Overlay and textarea metric drift (fonts, wrapping, scrollbars, mobile
  Safari); mitigated by the kill switch, `forced-colors` fallback, and
  alignment tests.
- Tokenizer drift from `parse.c`; mitigated by golden tests derived from its
  lexical rules.
- A second generated artifact in the service worker cache; mitigated by the
  precache, staging, and freshness steps above.
