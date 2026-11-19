---
name: parallel-xargs
description: "Use when mapping independent shell tasks with xargs; choose bounded process concurrency, isolate worker side effects and outputs, and reduce results safely."
---

# Parallel Shell Mapping with xargs

Use this skill when repeated, independent shell work can be expressed as one
worker invocation per input. `xargs` schedules the map phase; an explicit
follow-up command performs the reduce phase.

## Decide Whether to Map

- Map only independent work. If tasks depend on one another or mutate the same
  state, keep them ordered or create isolation first.
- Set a deliberate positive concurrency limit. Account for memory, I/O, CPU,
  and any parallelism already running at the agent-orchestration level. Do not
  use `-P0` as a default; it can start as many workers as the implementation
  allows.
- Do not run builds concurrently against the same checkout when they rewrite
  shared Makefiles, generated files, or object outputs. Use separate build
  directories or worktrees, or run the builds serially.

## Map Inputs Safely

For file paths, use NUL-delimited input so spaces, quotes, and newlines in
names are preserved. Avoid parsing `ls` output or splitting paths on whitespace.
For example, `printf '%s\0' "${items[@]}"` can pass a Bash array safely.

Pair `-P` with `-n` or `-L` so `xargs` launches multiple worker invocations;
`-P` alone may still result in a single invocation. Use `-n1` when each worker
should receive one item. The example below uses GNU `xargs` options (`-r` and
`--`); check option support before using it on a different implementation.

```bash
#!/usr/bin/env bash
set -euo pipefail

jobs=${JOBS:?Set JOBS to a positive worker limit}
[[ $jobs =~ ^[1-9][0-9]*$ ]] || {
  printf '%s\n' 'JOBS must be a positive integer.' >&2
  exit 2
}

results_dir=${RESULTS_DIR:?Set RESULTS_DIR to a unique output directory}
mkdir -p "$results_dir"
export RESULTS_DIR="$results_dir"

# run-one-test.sh accepts one input path and writes to a unique result/log file.
if find tests -type f -name '*.lg' -print0 |
  xargs -0 -r -n1 -P "$jobs" -- ./run-one-test.sh
then
  ./reduce-test-results.sh "$results_dir"
else
  printf '%s\n' 'One or more workers failed; reduction skipped.' >&2
  exit 1
fi
```

## Isolate and Reduce Results

- Assume parallel workers can write to stdout and stderr in any order; output
  may interleave. Prefer a unique result and log file per item, using a stable,
  collision-free key. Do not rely on `xargs` to preserve input or output order.
- Run the reducer only after the map command succeeds. Keep `pipefail` enabled
  for pipelines so failures from input generation are not hidden, and do not
  mask worker failures with `|| true`.
- Have the reducer consume per-item outputs in an explicit deterministic
  order. Define what an empty input set means before running the map.
- If a shared resource cannot be isolated, a lock can preserve correctness,
  but it may remove the performance benefit. Prefer separate resources when
  practical.

<!-- Co-authored by GitHub Copilot. Scope: bounded parallel shell mapping skill. Model identity and reasoning effort unavailable. Measured task interval: 2026-11-19 04:04:10 to 04:13:54 UTC (9m 44s). Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->