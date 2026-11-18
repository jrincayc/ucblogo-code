# Cloud Handoff Plan

## Context

The handoff describes an orchestrator implementation and testing of worker
limits, but the current checkout does not contain identifiable orchestrator
files or the referenced `.github/copilot-instructions.md`. `term.c` is noted
as an active file, but the handoff does not specify the relevant changes there.

## Plan

1. Reconcile the current branch and its files with the pushed orchestrator work
   and identify any missing instructions or implementation context.
2. Confirm the intended orchestrator behavior before changing code:
   - Use a flat worker pool; do not create nested worker agents.
   - Limit simultaneous sub-agents to eight.
   - Use parallel map/reduce processing when it is useful.
3. Inspect the relevant implementation and existing build/test setup.
4. Continue implementation only after confirming the intended scope, then
   validate the changes with the repository's existing checks.

## Current status

Implementation is not continued in this checkout because the handoff's
orchestrator context is not present in the files inspected so far.
