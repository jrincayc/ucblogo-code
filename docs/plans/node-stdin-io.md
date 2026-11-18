# Node Target stdin IO Plan

Status: Planned. Not started. Node is a low-priority target; the browser contract does not change.

## Problem

On every Wasm target, `em_getc` and `em_ungetc` in [files.c](../../files.c) bypass libc stdio. Consequences on Node:

- `feof(stdin)` never becomes true. This affects `lreadword`, `lreadchar`, `lreadrawline`, `lkeyp` and `llinep` in [files.c](../../files.c), and the batch-exit check in [main.c](../../main.c).
- Only `leofp` observes EOF, by comparing `sysGetC` to `EOF`.
- `lreadchar` and `lreadchars` narrow the result to `char` before the EOF check.
- The Node branch polls `process.stdin.read(1)` every 20 ms and uses `Buffer` unshift, with no EOF flag.

In the browser, stdin EOF has no meaning. `em_getc` returns -1 there only when woken with an empty buffer (stop or pause via `getcharready`); that is an interrupt, not EOF, and stays as is.

Known Node symptoms (see "Node target: known minor issues" in [native-build-restoration.md](native-build-restoration.md)):

1. `node tests/acceptance/run-logo-node.js` exits 1 with `Missing result for Tests.File.LiteralDribble`.
2. A batch run that reads stdin exits 1 even when the script ends with `bye`. Cause unknown.
3. A `0xFF` byte on stdin ends `readword` early. Cause unknown.
4. Startup prints nonfatal `localStorage.getItem is not a function` traces.

## Approach

Node already links with `-s NODERAWFS=1`, so libc `getc(stdin)` reaches `fs.readSync(0)`. Use real libc stdin on Node and keep `em_getc` browser-only. Detect Node at runtime (`typeof document === 'undefined'`), because browser and Node share the same Wasm objects.

Fallback if libc stdin proves unreliable (for example `EAGAIN` on non-blocking pipes or TTYs): keep a Node-only JS reader with an `EAGAIN` retry and an explicit stdin-EOF flag, and route `feof(stdin)` through a small `sysFeof()` shim.

## Steps

### Phase 0: reproduce and isolate (no edits)

1. Re-sync `/tmp/ucblogo-wasm` with `/tmp/sync-tree.sh` and rebuild `ucblogo-node.js`. Run a probe table: `readword`, `readchar`, `readchars`, `eofp`, `keyp`, `linep` against piped input, input without a trailing newline, empty input, `</dev/null`, a `0xFF` byte, and a TTY via the approach in `tests/native/pty_terminal.py`.
2. Diagnose the exit status 1 after a stdin read followed by `bye`: log `process.exitCode`, the `exit` event and the Emscripten exit status; compare with a `fs.readSync(0)` probe.
3. Isolate the `0xFF` truncation through `rd_getc`, `ecma_clear`, `into_line` and `strchr(special_chars, c)` in [parse.c](../../parse.c).
4. Run `Tests.File.LiteralDribble` alone and inspect raw stdout. Hypothesis: `TYPE` output without a trailing newline breaks the `^name...` pattern in [tests/acceptance/logo-results.js](../../tests/acceptance/logo-results.js).

### Phase 1: Node stdin through libc

5. In [files.c](../../files.c), add a one-time Node check. Make `sysGetC` use `getc(stdin)` on Node and `em_getc()` only in the browser; likewise `sysUnGetC` uses `ungetc` on Node. Update the macros in [globals.h](../../globals.h). Remove the Node branches from `em_getc` and `em_ungetc`.
6. Comment in `em_getc` that returning -1 means a wake without data, not EOF.
7. In `lreadchar` and `lreadchars`, hold `int` and check EOF before narrowing to `char`.
8. Confirm [main.c](../../main.c) sees real `feof(stdin)` on Node and exits cleanly for piped input; keep the browser unaffected.
9. Spike risk: if `readSync(0)` throws `EAGAIN`, apply the fallback above.

### Phase 2: small Node-only cleanups (parallel with Phase 1)

10. Guard `localStorage` use in [pre.js](../../pre.js) with a `typeof localStorage?.getItem === 'function'` check. Browser behavior is unchanged.
11. Fix `Tests.File.LiteralDribble` per step 4, preferably in the test (newlines or dribble-file-only output) rather than the runner. It must still pass on wx and the terminal build.

### Phase 3: tests and docs

12. Add a Node stdin harness beside [tests/acceptance/run-logo-node.js](../../tests/acceptance/run-logo-node.js): spawn `ucblogo-node.js` with scripted stdin and assert output, exit code, EOF handling (`readword` and `readchar` give `[]`, `eofp` gives true) and a `0xFF` round trip. Add a PTY case where practical.
13. Replace the known-issues section in [native-build-restoration.md](native-build-restoration.md) with the results. Document the Node IO contract (libc stdin, no browser EOF) in [webedition.md](../../webedition.md) and [browser-node-ci.md](browser-node-ci.md).

## Verification

1. After re-sync and rebuild in `/tmp/ucblogo-wasm`: `node tests/acceptance/run-logo-node.js` gives 76/76 with exit 0, then the new stdin harness passes.
2. Rebuild the browser target and run the desktop Chrome suite with the service worker bypassed and cache disabled. Browser input and output must be unchanged because `files.c` is shared.
3. Re-run the native terminal and wx unit suites and the PTY harness to confirm the `__EMSCRIPTEN__` guards leave those builds untouched.
4. Re-run the Phase 0 probe table: every row exits 0 with correct EOF behavior.

## Scope

- In scope: Node stdin and EOF correctness, the `0xFF` issue, the exit status, `LiteralDribble`, `localStorage` noise.
- Out of scope: the `nodeterm.c` split and Node interactive character mode (still deferred).
- The browser keeps `em_getc` and event-driven input, with no EOF concept.

## Open decisions

1. Commit to libc stdin on Node (recommended) versus the JS reader with an explicit EOF flag from the start.
2. Fix `LiteralDribble` in the test (recommended) versus making the runner tolerate unterminated output.
