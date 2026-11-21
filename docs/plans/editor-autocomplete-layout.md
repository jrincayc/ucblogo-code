# Editor Autocomplete Layout

## Goal

Replace the built-in editor's overlapping autocomplete popup with a reserved
suggestion area. The textarea must remain usable at its full visible size,
including when suggestions are empty. Keep the existing textarea and completion
engine; avoid caret-position measurement and floating-popup positioning.

Terminal-mode autosuggest must remain unchanged, including its rich descriptions,
layout, matching, and keyboard behavior.

## Agreed Design

- Portrait and narrow landscape: one horizontally scrollable suggestion row below
  the textarea and above the editor actions.
- Landscape with sufficient width: a vertically scrollable rail to the right of
  the textarea. The current preview uses landscape orientation and a minimum
  viewport width of 48rem.
- Desktop suggestions: word name, primary usage signature, and a short summary.
  Truncate long content within each pill rather than expanding the reserved area.
- Compact suggestions: word-name-only pills with touch targets at least 44 CSS
  pixels high. The preview selects compact content below 48rem or with a coarse
  primary pointer.
- Reserve stable dimensions even with no matches or after dismissal. Suggestions
  must not overlap the textarea or resize it as the user types.
- Let suggestions scroll independently of the textarea. Keep editor actions and
  punctuation keys outside the suggestion scroller.
- Preserve existing themes, text-size settings, and completion enable/disable
  behavior.

## Current Status

As of 2026-11-16:

- Layout and styling are implemented in [shell.html](../../shell.html), scoped to
  the editor instance through `presentation="editor"`.
- The empty horizontal bar's width-collapse bug is fixed with an explicit
  `minmax(0, 1fr)` grid column.
- Completion logic and focus handling have not been changed for this work.
- Regression coverage is in the existing
  [autocomplete fixture](../../tests/acceptance/autocomplete.testcafe.js).
- Desktop Chrome 152: 25 autocomplete tests passed, including terminal cases.
- Desktop Firefox 155: 25 autocomplete tests passed, including terminal cases.
- Row and rail tests now use deterministic editor-only suggestion providers and
  check selection after scrolling, inserted text, caret position, and focus.
  Providers are restored after each test.
- Chrome phone emulation at 390 by 844 with mobile/touch enabled: 22 passed;
  three resize-dependent cases were filtered out, not passed.
- No dedicated flashing test is planned, per maintainer direction. The reserved
  layout removes the overlap implicated in the original report; this is not a
  claim that all possible causes of flicker have been diagnosed.

## Remaining Work

1. Automated layout and completion checks are complete in desktop Chrome and
  Firefox. No additional application changes were needed during execution.
  Do not replace the textarea, refactor the shared completion engine, or change
  terminal suggestions merely to finish this plan.
2. Verify native mouse/touch scrolling and selection manually. Automated tests
  use programmatic scrolling and the fixture's established DOM-dispatched
  Shadow DOM clicks; they do not establish native gesture correctness.
  An attempted TestCafe simulated click lost textarea focus without inserting
  text in both layouts. The DOM-click path passes; native input remains an
  explicit coverage gap rather than a diagnosed application defect.
3. Check real mobile keyboard opening, closing, and rotation. Confirm the editor,
   actions, and suggestion area fit the visible viewport and remain usable.
   Avoid repeated row/rail switching caused solely by keyboard visibility. If a
   defect is found, make the smallest editor-only change with a focused TestCafe
   regression where automation can exercise the behavior.
4. Coordinate an actual iPad Safari session last; phone emulation does not
  establish real keyboard, touch, or Safari coverage. Desktop Safari is also
  untested locally because no Mac is available.
5. Treat any additional focus, selection, IME, or suggestion-update changes as
   separately justified work, not assumed requirements of this layout change.

## Test Contract

Maintain coverage for:

- Empty, populated, and dismissed suggestions preserving textarea dimensions.
- Full-width horizontal layout and non-overlapping landscape rail placement.
- Rich desktop content versus compact word-name-only pills.
- Independently scrollable suggestions and minimum touch-target height.
- Existing insertion, caret preservation, keyboard acceptance, dismissal, and
  completion-setting behavior in the editor.
- Unchanged terminal suggestions, including usage signatures and summaries.

Use the existing TestCafe suite and shared runner. Do not substitute Puppeteer
or Playwright. Do not install anything except TestCafe without explicit
permission and a concrete justification.

```sh
node tests/acceptance/run-testcafe.js --fixture 'Logo word autocompletion'
npm run test:autocomplete:phone
TESTCAFE_BROWSER=firefox node tests/acceptance/run-testcafe.js --fixture 'Logo word autocompletion'
```

Rebuild generated web artifacts before browser checks whenever application
sources change. Test-only edits can use the current verified build. Do not
hand-edit generated outputs. Preserve the maintainer's running browser session
and unsaved Logo work.