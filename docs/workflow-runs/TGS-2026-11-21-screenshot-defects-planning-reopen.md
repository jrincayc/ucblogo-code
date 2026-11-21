# TGS-2026-11-20-T1 — Screenshot Defects Planning Reopen

Date: 2026-11-21
UTC timestamp: `2026-11-21T20:30:14Z`
Task: `TGS-2026-11-20-T1`
Current gate: **Planning — reopened/pending**
Branch: `wasm`
Affected partial-repair revision: `6604d03e44d7be8ddee50886e52beb8c0d11f5b3`
Approved plan baseline: SHA-256 `6b10fabda17a78bf9b9b7aac2953b99b599153e7a83b4c0fd6da4e1b1bf8a2eb`
Approved Test Plan revision before reopen: SHA-256 `b0c9e65e4bd29a28099a93a93716c5f0a8c8e87583867f31560a31e256ea83bf`

## Andon

An independent Development Reviewer found two escaped defects in the partial
screenshot-base repair after it had passed the Implement Tests stage and reached
Implement Changes:

1. **Disabled screenshot CLI mode is not recognized.**
   `tests/acceptance/run-testcafe.js` does not detect `--disable-screenshots`.
   It can resolve `screenshotsPath` to `screenshots` even though screenshots are
   disabled, and export that misleading value to the child fixture.

2. **PNG evidence is size-only.**
   `tests/acceptance/turtle-graphics-examples.testcafe.js` accepts a screenshot
   when it is a file and has nonzero size. It does not verify the PNG signature,
   IHDR marker, or otherwise establish that the file is a PNG.

## Reopened Scope

The defects affect the following partial implementation files:

- `tests/acceptance/run-testcafe.js`
- `tests/acceptance/runner.test.js`
- `tests/acceptance/turtle-graphics-examples.testcafe.js`

The partial implementation and dependent browser/static evidence are not a
passed Implement Changes deliverable. The review invalidates downstream
Implement Changes approval and any later assumptions based on it.

## Planning Requirements

The top-level Turtle Graphics Plan and this screenshot repair's bounded scope
must be revised to require:

- Effective screenshot base resolution for default, explicit `path=`, disabled,
  and conflicting CLI modes.
- A disabled mode that produces no effective base and prevents/diagnoses fixture
  screenshot capture rather than reporting a false base.
- A real PNG signature and format marker check, not nonzero size alone.
- Current-run UUID isolation and preservation of prior screenshot directories.
- Report and Markdown paths derived from the actual effective base and exact
  absolute files.
- Mismatch and stale-file discrimination in reconciliation.
- Focused unit/static coverage before browser execution.

## Gate Sequence

1. Revise and independently QA the Plan.
2. Obtain explicit human approval of the exact revised Plan.
3. Reopen Test Planning and produce a revised Test Plan for the expanded scope.
4. Independently QA and obtain human approval of the exact revised Test Plan.
5. Enter Implement Tests and add the discriminating red tests.
6. Obtain aggregate Implement Tests QA and human approval before implementation.
7. Implement only the cleared scope.
8. Run focused static and browser validation in both screenshot-root modes.
9. Independent review, aggregate Implement Changes QA, and human approval.

No implementation edit, build, browser rerun, commit, push, or later gate advance
is authorized now.

## Preservation

No implementation or plan file was changed to create this record. Existing
partial test/fixture changes remain untouched for later review. Pre-existing
ledger, accounting, formatting, upstream, and research work is preserved and
excluded from this reopen record.

<!-- Co-authored by GitHub Copilot. Scope: escaped screenshot-defect Planning reopen. Model: MAI Code 2 Flash; reasoning-effort setting unavailable. -->
