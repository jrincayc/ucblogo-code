# Later Research Plan: Turtle Graphics Report Reconciliation

## Objective

Determine why the experimental turtle graphics accounting report did not establish a passing Web artifact freshness check for revision `1052788e27471d213754d5009ba043aad6e8851b`.

This is diagnosis-only. Do not change application behavior, tests, build configuration, or reconciliation logic unless a separate scope is approved.

## Source Evidence

- CI run: https://github.com/jrincayc/ucblogo-code/actions/runs/45083454833
- Job: https://github.com/jrincayc/ucblogo-code/actions/runs/45083454833/job/137396610032
- Revision: `1052788e27471d213754d5009ba043aad6e8851b`
- Failing assertion: `Report does not establish a passing Web artifact freshness check`
- Test: `tests/acceptance/turtle-graphics-examples.testcafe.js`
- Security test: `ACC-URL-001` passed
- Suite: 119/120 Web tests passed
- Classification: reconciliation/reporting defect, intentionally deferred

## Preserve Evidence

Before rebuilding or rerunning:

1. Download the complete job logs, generated reports, Git metadata, and upload artifacts.
2. Record original filenames, paths, timestamps, sizes, and SHA-256 hashes.
3. Record whether each artifact came from the job checkout or was generated during the build.
4. Preserve the original CI environment rather than treating a later local checkout as equivalent evidence.

## Investigation Areas

### 1. Artifact mtimes

Compare filesystem modification times for `ucblogo.html`, `ucblogo.js`, `ucblogo.wasm`, source inputs, the report file, and uploaded copies.

Determine whether mtimes reflect checkout timing, build start or completion, source compilation, report generation, upload, or coarse filesystem timestamp behavior.

### 2. Checkout dirty state

Capture exact `HEAD`, branch or detached-HEAD status, staged and unstaged changes, untracked files, generated files, and `git describe` output before and after build/report generation.

Do not conflate generated or report files with source modifications that affect the Web build.

### 3. Embedded Git description

Compare the embedded artifact description, report description, CI checkout ref, local `git describe` output, and any dirty suffix.

Distinguish exact commit identity, commit plus dirty metadata, stale metadata, absent metadata, and an invalid fallback value.

### 4. Build timing

Establish an ordered timeline for runner acquisition, checkout, build configuration, source mtimes, build start, artifact generation, build completion, report generation, upload, test execution, and reconciliation.

### 5. Report write timing

Identify whether the report is written before or after generated artifact completion and whether test execution rewrites or finalizes it after its freshness snapshot.

### 6. Upload coverage

Inventory artifacts and reports uploaded by the job. Verify that the report and browser-tested artifacts correspond by path and hash, and that the report directory is included in upload coverage.

### 7. Reconciliation discrimination checks

| Check | Passing evidence | Failure evidence |
|---|---|---|
| Revision identity | Report and artifacts consistently identify the tested revision | Missing or contradictory revision identity |
| Checkout cleanliness | Pre-build tracked and untracked state is known | Generated files are misclassified as source modifications |
| Artifact freshness | Artifact hashes/mtimes follow a completed build | Artifact predates checkout/build or came from another target |
| Build timing | Build completion precedes report and artifact snapshot | Report snapshots stale or intermediate state |
| Report timing | Report observes final tested artifact state | Report observes pre-build or predicted state |
| Upload coverage | Tested artifact/report hashes match uploaded objects | Report or artifact set is missing, replaced, or wrong |
| Embedded metadata | Description matches checkout without contradictory state | Metadata is stale or invalid |
| Test jurisdiction | Report compares the exact artifact served to the browser | Report compares local/uploaded artifacts from another target |

## Proposed conclusion format

Possible outcomes:

1. True stale-artifact defect.
2. Report false negative.
3. Upload mismatch.
4. Measurement ambiguity.
5. Environment defect.

Retain failure/BLOCKED status until the discrimination checks identify a defensible passing or failing reconciliation result.
