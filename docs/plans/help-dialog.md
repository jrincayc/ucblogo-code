# Project Plan: Web Edition Help Dialog

Status: Phase 1 wireframe approved and frozen. Phase 2 steps 2.1 to 2.4 are
implemented (2026-11-15); step 2.6 is implemented except pending accessibility
review; step 2.5 (tutorial) needs the maintainer's authorship.
Date: 2026-11-14

## Objective

Add a modal Help dialog to the Web Edition. It is mobile-first and accessible
(target WCAG 2.2 AA, see [web accessibility plan](web-accessibility.md)).

- A side panel lists Logo words grouped semantically, as in
  [usermanual](../../usermanual) and the chapter menu of
  [ucblogo.texi](../ucblogo.texi). The main panel shows the documentation for
  the selected word.
- The user can insert the selected word into the command input, or into the
  editor when the editor dialog is open.
- Tabs switch between the word browser, a beginner tutorial (with advice on
  where to go next), the user manual, and `webedition.md`.
- Full-text search covers words and documents.

## Baseline

- [shell.html](../../shell.html) owns dialog markup and CSS:
  `dialog.dialogWithTitle`, `h2.dialogTitle` as a direct child, a scrolling
  `.dialogBody`, `--theme-*` variables, toolbar buttons with
  `aria-haspopup="dialog"`, and the editor dialog. There are no width media
  queries today.
- [pre.js](../../pre.js): `Module.softKeyDown` inserts at the caret into
  `#editorTextArea` when `#editorDialog` is open, otherwise into
  `#logoInputText`. `Module.completion.help(name)` reads
  `/share/ucblogo/helpfiles/<name>` (embedded).
- Help file format ([helpfiles/fput](../../helpfiles/fput)): usage lines, a
  blank line, then body text.
- Only `helpfiles` and `logolib` are embedded in the Wasm filesystem.
  `usermanual` and `webedition.md` are not available at runtime.

## Phase 1: Wireframe (approved)

Format: static HTML/CSS mockup at `docs/plans/help-dialog-wireframe/index.html`,
not wired to Logo and not part of the build. Viewports: phone portrait, phone
landscape, tablet/desktop.

- [ ] Content model sketch: category tree and about 15 sample words, entry
  layout (usage, description, Insert button), tab set (Words, Tutorial, User
  manual, Web Edition) and a persistent search field above the tabs.
- [ ] Phone portrait (base styles): full-screen dialog, fixed title bar with
  Close, search field, horizontally scrolling tablist, single-pane drill-down
  (categories, words, detail with Back), sticky bottom action bar.
- [ ] Phone landscape and tablet/desktop: two panes (tree about 14em, detail),
  independent scrolling, fixed title bar and tabs, action bar in the detail
  header.
- [ ] Entry points: toolbar Help button next to Settings, Help button in the
  editor dialog (stacked modal), and F1.
- [ ] States: empty search, grouped results with highlighted snippets, no
  results, loading, word not found, polite live region "Inserted `word`".
- [ ] Annotations: role, accessible name, keyboard behavior, and touch-target
  size for each region.
- [ ] Accessibility requirements shown in the mockup: `showModal()` with
  `aria-labelledby`; focus to the search field on open and back to the opener
  on close; Escape closes; WAI-ARIA tabs pattern (arrow keys, Home, End);
  native `ul` of `button`s for the word list; labelled search with a
  `role=status` result count; native `click` for insertion; 44px touch
  targets; no horizontal scroll at 200% zoom except the tablist; 4.5:1
  contrast in light and dark; `prefers-reduced-motion`.
- [ ] Insertion rule: same target selection as `Module.softKeyDown`; insert at
  the caret with a trailing space and place the caret after it. Show both
  "Insert" and "Insert and close" to settle the close and focus behavior.
- [ ] Review in desktop Chrome, then Chrome device emulation (about 360x640,
  640x360, 820x1180): accessibility tree, keyboard-only use, 200% zoom, dark
  mode, stacked modal over the editor. Report emulation as emulation; real
  iPad Safari coverage happens during implementation with the maintainer.

Exit: reviewed wireframe frozen as the specification for implementation.
Approved mockup: [help-dialog-wireframe/index.html](help-dialog-wireframe/index.html).
The checklist items above were not individually ticked; the maintainer approved
the mockup as is. The tutorial, manual, and Web Edition panels are placeholders
and the word set is sample data, to be specified in Phase 2.

## Phase 2: Implementation

Constraints from the codebase: the Wasm build embeds `$(pkgdatadir)` (helpfiles
and logolib), `tests`, and `web.lgo` ([Makefile.am](../../Makefile.am)
`--embed-file`); [workbox-config.js](../../workbox-config.js) precaches only
`ucblogo.html`, `ucblogo.js`, and `ucblogo.wasm`, with `NetworkFirst` runtime
caching for everything else; the Settings button lives in the toolbar group in
[shell.html](../../shell.html). Help runs in JS only and does not call into the
Logo runtime.

### Step 2.1: Content generation (build time, Node script in `scripts/`)

- [x] `scripts/build-help.js` writes `help/index.json`: categories from the
  `@section` structure of [ucblogo.texi](../ucblogo.texi), word to category
  map, and usage lines; per-word body text comes from `helpfiles/<name>`.
- [x] Same script renders the user manual and `webedition.md` to sanitized
  static HTML (chapter files) plus a compact search index (`help/search.json`).
- [x] Tests for the script against fixture inputs (category coverage: every
  file in `helpfiles/` appears in exactly one category or in "Other").

### Step 2.2: Delivery (decided 2026-11-15: eager, lowest priority, not in the FS)

Considered and rejected: a separate Emscripten data package (`file_packager`
`--preload`, mounted under `/share/ucblogo/help`). It works (the tool ships with
the SDK) but adds a build step, a second loader script and a `.data` file to
stage, verify and keep in step with the Wasm build, and the only gain is reading
the documents with `FS.readFile`. Plain `fetch` of static files needs none of
that, and the stage, cache and failure handling are the same in both options.
Revisit only if a Logo-side `help` command must read these documents.

- [x] Word entries: read from the embedded `/share/ucblogo/helpfiles` and the
  small category index `help-index.json` (about 27 KB, already embedded), since
  the Words tab needs them at once.
- [x] Manual and `webedition.md` (combined in `help/docs.json`, about 214 KB raw
  and 60 KB gzipped): fetched eagerly but last, after Logo is ready and the page
  has loaded, during idle time (`requestIdleCallback`, low-priority `fetch`).
  Never embedded in the Wasm filesystem. Tabs show a loading state if opened
  before the fetch completes. The tutorial is not included until authored.
- [x] Offline: rely on the existing `NetworkFirst` runtime caching, which stores
  the idle fetches. Do not add them to the Workbox precache: precaching runs at
  service-worker install and competes with startup. Consequence: Help documents
  work offline only after one complete online session.
- [x] Deploy: `scripts/stage-web-deploy.js` copies only an explicit file list, so
  it must include the `help/` files (and `verify` must check them) or the live
  site will return 404 for them. Add a test in `stage-web-deploy.test.js`.
- [x] CI must generate the `help/` files before staging (the Makefile rule that
  already builds `help/index.json` should produce them too).

### Step 2.3: Dialog markup and behavior

- [x] Copy markup and CSS from the frozen wireframe into `shell.html`; reuse
  `--theme-*` variables and the `dialogWithTitle` structure.
- [x] Add the toolbar Help button beside Settings, a Help button in the editor
  dialog header, and F1.
- [x] `Module.help` service in `pre.js`: open/close with focus return, tab
  state, drill-down state, search, and insertion.
- [x] Refactor insertion out of `Module.softKeyDown` into a shared helper so
  both use the same target selection; add trailing space and caret
  placement.
- [x] Close the open decision on "Insert" vs "Insert and close" (default
  recommendation: keep both).

### Step 2.4: Search

- [x] Search uses the embedded category/word index and lazily read helpfiles;
  word names and usage rank above bodies. Document search scans the fetched
  sections at runtime; results are grouped with highlighted snippets and a
  `role=status` count (resolves Open Decision 2).

### Step 2.5: Tutorial

- [ ] Author the beginner tutorial (Markdown source in the repo, built by
  Step 2.1) with a "where next" ending. Content review by the maintainer.

### Step 2.6: Tests and docs

- [x] TestCafe acceptance tests in [tests/acceptance](../../tests/acceptance),
  coordinated with the [TestCafe consolidation plan](testcafe-consolidation.md):
  open/close and focus return, tab keyboard behavior, search states, insertion
  into the command input and editor, stacked modal, document-load failure/retry,
  and light/dark contrast. Offline delivery is additionally covered by the
  existing `NetworkFirst` policy and source checks.
- [x] Unit tests for the build script and search ranking.
- [ ] Accessibility checks per [web-accessibility.md](web-accessibility.md);
  desktop Chrome, phone Chrome emulation, Firefox, and light/dark contrast
  checks are automated and pass. Pending: a real phone and iPad Safari with the
  maintainer, and a screen-reader pass.
- [x] Update `webedition.md` (Help dialog section). The user manual is unchanged.

## Open Decisions

1. Decided (2026-11-15): the manual and `webedition.md` are fetched eagerly at
   lowest priority, not embedded in the Wasm filesystem; see Step 2.2.
2. Resolved (2026-11-15): word search uses the embedded index and document
  search scans the fetched sections at runtime; no separate full-text index.
3. Resolved (2026-11-15): keep both Insert actions; plain Insert leaves Help
  open, while Insert and close closes it.
4. Open: tutorial authorship and scope (beginner programmers, plus advice on
  going further); maintainer authorship and review are needed.
