# Project Plan: Web Edition Accessibility

Status: Proposed; implementation and assistive-technology validation are not
claimed by this plan.
Date: 2026-11-13

## Objective

Make the complete Web Edition usable by people with a range of visual, motor,
and other access needs. Support existing third-party screen readers; develop an
optional built-in screen reader more capable than the current spoken output;
make turtle drawings understandable without relying on color; and assess
refreshable braille and tactile-graphics workflows with users.

The current [Web Edition roadmap](../../webedition.md#accessibility-status-and-roadmap)
is the summary. This document proposes work, dependencies, and acceptance gates;
it does not authorize implementation, a schedule, or procurement by itself.

## Baseline And Scope

- The maintainer reports that terminal output can be read by a screen reader.
  As a web app, the rest of the interface is expected to work with third-party
  screen readers, but end-to-end compatibility and specific combinations have
  not yet been verified. Do not equate HTML/ARIA markup with successful use.
- Current text-to-speech can speak output; it is not the proposed full built-in
  reader. External-reader interoperability and the built-in reader are separate
  workstreams, and neither replaces the other.
- [shell.html](../../shell.html) owns the web interface, command input, editor,
  controls, settings, and SVG drawing container. Its drawing is exposed as an
  image, not a verified navigable description of its contents.
- [pre.js](../../pre.js), [liblogo.js](../../liblogo.js), and
  [webgraphics.c](../../webgraphics.c) are starting points for runtime, speech,
  input/output, and graphics behavior. Verify current ownership and generated
  SVG behavior before choosing implementation boundaries.
- [tests/acceptance](../../tests/acceptance) contains browser checks; coordinate
  new tests with the [TestCafe consolidation plan](testcafe-consolidation.md)
  and the [browser and Node CI plan](browser-node-ci.md). Neither plan implies
  that real screen-reader or braille-device testing is automated.
- Work is web-first. Preserve existing Logo semantics, files, drawing output,
  performance, Node behavior, and wx compatibility where shared code is touched.
  Desktop, mobile, keyboard, touch, low vision, and assistive technology are in
  scope. Replacing OS-wide assistive technology or guaranteeing arbitrary
  third-party Logo/JavaScript-created UI is not.

## Priorities And Dependencies

| Priority | Work Package | Dependencies / Parallel Work |
| --- | --- | --- |
| P0 | Audit, user involvement, and repeatable baseline | First; informs all other packages |
| P1 | External-reader, keyboard, input, editor, and terminal workflows | Starts with audit; ship fixes incrementally |
| P1 | Drawing access model and visual-mode design | Research in parallel; coordinate shared graphics representation |
| P2 | Color-vision, grayscale, and black-and-white pattern modes | Prototype after rendering/export inventory; independent of built-in reader |
| P2 | Drawing descriptions and braille assessment | Initial braille assessment can start with existing text workflows; richer drawing review depends on description prototype |
| P3 | Optional built-in screen reader | Design early; implementation builds on stable UI and drawing access models |
| Ongoing | Automated regression, human validation, documentation | Begin with baseline and continue through every release |

Priorities indicate sequencing, not fixed dates. Assign maintainers, user
participants, and estimates after the audit. Ship independently useful changes
without waiting for a full built-in reader or a complete tactile solution.

## 0. Audit And User Research

- [ ] Inventory startup, terminal input/output, Logo reads, history,
  autocomplete, editor, dialogs, file actions, settings, turtle graphics,
  animation, and error/recovery paths. Capture keyboard and touch workflows.
- [ ] Record existing speech controls and current behavior rather than assuming
  how output, voices, cancellation, or settings persistence work.
- [ ] Test representative external screen reader/browser combinations with
  keyboard and, where available, refreshable braille. Record versions, device,
  steps, observed behavior, severity, and whether a human actually tested it.
- [ ] Invite blind, low-vision, color-vision-deficient, braille, and
  keyboard-only users into prioritization and acceptance. Identify participation
  and hardware gaps rather than treating simulation as user validation.
- [ ] Set an explicit accessibility target, initially WCAG 2.2 AA where
  applicable, and record exceptions, non-web content, and unmet criteria. A
  checklist or automated score alone does not establish conformance.
- [ ] Establish a reproducible fresh-build baseline and collect focus, output,
  and accessibility-tree evidence without disturbing user files/settings.

Exit: a dated issue inventory, baseline test matrix, named decision owners, and
prioritized blockers exist. Unavailable combinations are marked untested.

## 1. External Readers, Input, And Editing

- [ ] Audit native names, roles, values, grouping, states, focus order, and
  keyboard activation for all controls; use native semantics when possible.
- [ ] Verify command entry and Logo-requested input, including character and
  line reads, multiline definitions, submission, cancellation, and errors.
  Users can review the entered text, caret, selection, and pending input before
  submitting it; typing echo follows their screen reader preferences.
- [ ] Make history, autocomplete, and soft-key insertion usable with keyboard,
  touch, and external readers. Announce relevant selection changes without
  double-speaking; preserve focus, text selection, and caret position.
- [ ] Exercise the editor's multiline navigation, selection, autocomplete,
  apply/discard, and error recovery. Provide meaningful status and restore focus
  after dialogs; do not trap focus outside a modal or lose edited text.
- [ ] Make output and prompts reviewable in order, including partial lines,
  long output, errors, busy/ready states, and stop. Manage live announcements
  so rapid or repeated output does not drown out input or interrupt indefinitely.
- [ ] Verify file upload/download, settings, focus visibility, zoom/reflow,
  contrast, high-contrast/forced-colors modes, reduced motion, and non-color
  status cues. Preserve existing text-to-speech and Logo-selected text colors.
- [ ] Add targeted browser regressions for focus, names, keyboard interaction,
  output order, and state. Test real screen readers for speech and navigation;
  DOM assertions are not a substitute.

Exit: a user can start Logo, enter and review a command, read results/errors,
edit/apply a procedure, use history, stop execution, and navigate dialogs and
files without a mouse in the agreed tested combinations. Document failures and
coverage gaps; no automatic claim of full conformance follows from this gate.

## 2. Accessible Turtle Drawing Model

- [ ] Inventory SVG creation, labels, strokes, fills, backgrounds, sprites,
  clipping/wrapping, masks/erase, clearing, redraw, animation, and export.
  Determine which Logo semantic information survives in the generated SVG.
- [ ] Prototype a concise summary plus an on-demand navigable view of drawing
  contents: labels, geometry/position, color or pattern, order, and turtle state
  where reliably known. Distinguish observed primitives from inferred shapes;
  do not claim to understand the artistic intent of arbitrary drawings.
- [ ] Compare deriving descriptions from SVG with retaining structured drawing
  events. Decide how changes, masking, clearing, arbitrary `runjs` SVG edits,
  and generated sprites affect accuracy. Mark unsupported or stale information
  explicitly rather than presenting it as complete.
- [ ] Bound memory, CPU, and announcement volume for large drawings and
  animation; offer snapshots/on-demand inspection and avoid narrating every
  frame. Preserve drawing, interpreter, and event-queue semantics.
- [ ] Evaluate descriptions with blind and low-vision users. Verify keyboard and
  third-party reader navigation and reuse the same representation for the
  future built-in reader and braille where appropriate.

Exit: an accepted prototype and decision record describe the supported drawing
operations, fidelity limitations, lifecycle, performance budget, and user-tested
navigation. Complex drawings must not be represented solely by a generic image
label; unsupported content remains clearly identified.

## 3. Accessible Color And Pattern Modes

- [ ] Define a user-selectable default, multiple color-vision-friendly options,
  grayscale, and black-and-white pattern mode. Specify palette mappings and a
  legend or inspectable mapping; no single palette can distinguish every color
  for every person or guarantee contrast against all backgrounds.
- [ ] Design grayscale for distinguishability, not just automatic luminance
  conversion. Test small adjacent regions, thin strokes, overlapping marks,
  background, labels, and print output.
- [ ] Prototype black-and-white SVG patterns for fills and distinguishable
  stroke encodings such as patterns, dashes, or markers. Specify scale,
  spacing, line width, and legend at different zoom and print sizes. Include
  masks/erase, clipping, transparency, sprites, and drawing order in tests.
- [ ] Decide and document how modes affect indexed versus arbitrary RGB colors,
  colors assigned by Logo programs, existing versus new marks, theme changes,
  backgrounds, and content injected through JavaScript. Avoid implying that
  arbitrary RGB values can always be mapped to uniquely identifiable patterns.
- [ ] Keep default rendering and Logo color values/queries compatible. Make
  preview transformations reversible and separate from underlying drawing data;
  explicitly define whether mode changes redraw existing marks. Persist the
  user's chosen setting where storage is available and handle storage failure.
- [ ] Specify default/original versus displayed-mode SVG export. Ensure selected
  accessible exports contain self-contained SVG pattern definitions and IDs,
  work after reload/opening elsewhere, and preserve the declared appearance.
- [ ] Test representative artwork, collisions and edge cases with color-vision
  simulations, grayscale and black-and-white printing, and users with relevant
  access needs. Provide keyboard and screen-reader access to palette settings.

Exit: documented modes and export policy are available without changing Logo
color semantics; representative visual and export regressions pass, and user
evaluation and unresolved limitations are recorded.

## 4. Braille And Tactile Assessment

- [ ] Test refreshable braille with external screen readers for input, caret and
  selection, history, editor, terminal output, alerts, and long lines. Record
  contracted/uncontracted or computer-braille settings where relevant, and
  confirm punctuation important to Logo is distinguishable.
- [ ] Test drawing summaries and navigation with braille users once a prototype
  exists. Assess whether ordering, spatial information, and concise descriptions
  remain understandable on the tested display and screen reader.
- [ ] Consult users about tactile diagrams and evaluate possible SVG or other
  export workflows with actual embossers/printers where available. Treat SVG
  patterns, refreshable braille, and tactile graphics as distinct technologies;
  a downloadable SVG or an accessible text description is not proof of tactile
  compatibility.
- [ ] Publish tested equipment, software, methods, results, blockers, and a
  recommendation: supported workflows, further prototype, or explicit gap.
  Identify hardware/participant needs before committing to a tactile feature.

Exit: an evidence-based compatibility report and prioritized follow-up exist.
If hardware or participants are unavailable, record the assessment as blocked,
not passed; a tactile export is not a prerequisite for reporting text results.

## 5. Optional Built-In Screen Reader

- [ ] Define supported scope and user workflows separately from third-party
  screen-reader support. The built-in reader works within the app, not across
  the operating system, and is not a replacement for braille or native readers.
- [ ] Inventory current speech APIs and browser limitations, voice availability,
  permissions/user-gesture restrictions, offline behavior, and failures. Prototype
  a speech queue with cancellation, prioritization, and bounded output; preserve
  the current spoken-output feature and Logo speech commands.
- [ ] Add optional reading of input, caret/selection and editing, command history,
  editor content, output, controls/focus/state, errors, and available drawing
  descriptions. Provide on-demand review rather than relying only on live speech.
- [ ] Design discoverable keyboard and touch operation, including enable/disable,
  interrupt, navigation, repeat, and help. Resolve conflicts with browser,
  operating-system, screen-reader, and Logo shortcuts; avoid trapping focus.
- [ ] Provide voice, language, rate, volume, verbosity, punctuation/typing echo,
  and announcement settings where supported; save preferences with graceful
  fallback when storage or voices are unavailable. Define safe defaults that do
  not duplicate third-party speech; do not depend on detecting a screen reader.
- [ ] Handle long/rapid output, partial lines, program errors, stop, animation,
  page changes, and shutdown without stale or unbounded speech. Do not reenter
  an active Logo runtime from speech callbacks; use existing safe event paths.
- [ ] Test with blind users across input, editor, drawing, settings, recovery,
  and third-party-reader coexistence. Document unavailable voices, offline
  behavior, platform-specific limitations, and the supported navigation model.

Exit: an opt-in reader supports the agreed end-to-end workflow without disabling
external readers; speech controls, interruption, failure behavior, and regression
coverage pass, and user testing and remaining gaps are documented.

## Validation And Release Gates

- [ ] Maintain automated checks for accessible names/roles/states, focus,
  keyboard operations, input/caret handling, output ordering, settings persistence,
  speech queue behavior with stubs, drawing state, and SVG export. Add an
  accessibility audit as a supplemental signal, not the sole gate.
- [ ] Run focused desktop Chrome tests during ordinary web development; expand
  to mobile Chrome or emulation, Firefox, then a coordinated real iPad Safari
  session for broader validation. Emulation is not a physical-device test.
- [ ] Record actual OS, browser, screen reader, version, device, and input method
  in a matrix. Candidate combinations include NVDA/Firefox and NVDA/Chrome on
  Windows, Orca/Firefox on Linux, and VoiceOver/Safari on iPadOS; availability
  and support commitments require confirmation. Test braille with actual
  hardware when available.
- [ ] Desktop Safari is supported but unavailable to the maintainer locally;
  use actual Safari on an available Mac/CI or approved service when feasible,
  otherwise record a gap. iPad Safari and automated WebKit are not desktop Safari.
- [ ] Before release, repeat agreed task-based tests with assistive-technology
  users, log unresolved severity and workarounds, and update the support matrix
  and user documentation. Accessibility claims describe tested combinations,
  not inferred universal compatibility.

## Decisions And Risks

| Decision / Risk | Owner / Resolution Required |
| --- | --- |
| Supported assistive-technology combinations and WCAG scope | Maintainer with accessibility users after baseline |
| Speech queue ownership, keyboard model, and browser speech limitations | Reader design before implementation; test coexistence and offline fallback |
| Drawing semantic source, partial coverage, and animation scale | Graphics prototype and user evaluation before promising completeness |
| Indexed/RGB mapping, retroactive rendering, palette collisions, and export | Graphics design before palette implementation |
| Braille hardware, screen-reader access, and tactile production | Recruit participants and secure equipment; record blocked testing |
| Generated artifacts, caching, and preserved user data | Verify source/build/runtime freshness; do not clear user storage or change production caching to pass tests |
| Schedule and staffing | Estimate after audit; no completion date or funding is assumed |

## Immediate Next Steps

1. Assign an audit owner and identify participants and available assistive tools.
2. Record current behavior for terminal output, command input, editor, and a
   simple turtle drawing in at least one real screen-reader/browser combination.
3. Inventory the SVG and speech paths, capture failing workflows as issues, and
   agree on the first input/output regression and human acceptance scenario.
4. Prototype one simple drawing description and one color/pattern mapping in
   parallel; document tradeoffs before expanding either implementation.


# Logo special speech features
1. Speek characters as entered. Optional. Disabled by default.
2. Speek text in logo input after user stops typing for a short period and after autocomplete suggestions have been spoken. Optional, disabled by default.
3. Spoken output should include punctuation (semantic interpretation?)
4. Spoken output should use semantic prompts, not "question mark"
5. Spoken output should include autocomplete suggestions, after a short delay. Just speak the word name.

# help dialog
help dialog should have screen reader features

# settings dialog
settings dialog should have screen reader features

# turtle graphics
it's unclear how turtle graphics can be spoken

# terminal help command
terminal help command shoudl work well
with screen reader

question: How to handle setcursor generated output markup?