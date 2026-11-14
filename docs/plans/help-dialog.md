# Project Plan: Web Edition Help Dialog

Status: Proposed; wireframe and implementation are not started.
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

## Phase 1: Wireframe (current step)

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

## Phase 2: Implementation (not yet planned in detail)

Proposed after wireframe approval; to be split into its own plan.

- Build-time conversion of `usermanual`, `webedition.md`, and the tutorial to
  HTML or JSON, plus a prebuilt search index.
- Dialog markup and CSS in `shell.html`; behavior as a `Module.help` service
  in `pre.js` (JS-only, no new C-to-JS reentry into the Logo runtime).
- Tests in [tests/acceptance](../../tests/acceptance), coordinated with the
  [TestCafe consolidation plan](testcafe-consolidation.md).
- Update `webedition.md` and the usermanual references.

## Open Decisions

1. How to ship the manual and `webedition.md`: embed in the Wasm filesystem, or
   fetch and cache through the service worker. Recommendation: generate at
   build time and decide the delivery path before Phase 2.
2. Search implementation: prebuilt index versus a runtime scan; index size
   against load time on mobile.
3. Whether insertion closes the dialog by default.
4. Tutorial authorship and scope (beginner programmers, plus advice on going
   further).
