# Project Plan: Pure-JS Logo Object Inspection API

## Problem

The current inspection API (`Module.logoGC` in [pre.js](../../pre.js)) calls
twelve `logo_gc_debug_*` C exports ([mem.c](../../mem.c),
[Makefile.am](../../Makefile.am)). The C runtime is non-reentrant: calling a
Wasm export while the interpreter is suspended in Asyncify, or from an
`EM_ASM`/`runjs` callback with a live C frame, can corrupt evaluator state. The
API is therefore unsafe exactly when it is needed: debugging a live program.
Its output is also `console.table`/`console.log` text, not data, and it
exposes only raw GC nodes, not Logo objects.

## Goals

- Inspect live Logo state (words, lists, arrays, procedures, variables,
  property lists, registers, variable frames, heap) from JavaScript with no
  call into Wasm.
- All results are JSON-serializable and returned as values (no console output).
- New file separate from `pre.js`; `pre.js` loses `Module.logoGC`.
- Read-only. Never write Wasm memory.

Non-goals: stepping, breakpoints, mutation, stable addresses across GC, wx
Edition support.

## Design

### Reentrancy rule

The inspector reads Wasm linear memory only (a `Uint8Array`/`DataView` over the
heap, re-acquired on every API call so `ALLOW_MEMORY_GROWTH` cannot leave a
stale view). It never calls `Module._*` functions, `ccall`, `cwrap`, or
`Module.run_evaluator`. JS is single-threaded, so while the inspector runs the
interpreter cannot execute; a read is a consistent snapshot of whatever state
the runtime was suspended in. This is the same pattern already used for C
globals (`getValue(Module._Unbound, '*')` in [liblogo.js](../../liblogo.js)).

Reads bypass ASAN poison, which removes the need for the
`__attribute__((no_sanitize_address))` accessors.

### C-side changes (data and deletions only, no new code)

- Delete the `logo_gc_debug_*` functions from `mem.c` and their entries from
  both `EXPORTED_FUNCTIONS` lists in `Makefile.am`.
- Export data symbols needed as roots: `_hash_table`, `_segment_list`,
  `_regs`, `_var_stack`, `_the_generation`; `_Unbound`, `_inside_gc`, and
  `_LOGO_GIL` are already exported. JS reads these as addresses, like
  `Module._Unbound` today.
- Add `HEAPU8` to `-sEXPORTED_RUNTIME_METHODS` for both targets (or use
  `wasmMemory.buffer` if that is already reachable); verify against the pinned
  Emscripten version.
- Add a `__EMSCRIPTEN__`-guarded `_Static_assert` block in `mem.c` for
  `sizeof(NODE)`, the `NODE` and `struct segment` field offsets, and the
  `struct registers` offsets the JS layout table depends on. A layout change
  then fails the build instead of silently misreading memory.
- `eval.c` ASAN hook: call the new pure-JS `Module.logoInspect.node($0)`
  instead of `Module.logoGC.inspect`.

### New file: `logo-inspect.js`

Same shape as [storage-status.js](../../storage-status.js): a factory
`createLogoInspector({ memory, symbols, layout })` that takes injected memory
and root addresses, so Node tests can drive it with a synthetic heap. At the
bottom it exports via `module.exports` and `globalThis`. In the Web/Node builds
it is linked with `--pre-js logo-inspect.js` after `pre.js`, and attaches
`Module.logoInspect` once the runtime is initialized. It adds no deployed
asset, since `--pre-js` code is inlined into `ucblogo.js`.

Layout table: a single `LAYOUT` object (wasm32: 4-byte pointers and `FIXNUM`,
`NODE` fields, `NT_*` flag values, `struct segment`, `struct registers`, object
cell offsets). At attach time the inspector self-checks the layout (`Unbound`
node type, first segment `size`, node stride, first non-empty `hash_table`
bucket is a `CASEOBJ` with a readable name). On mismatch every call returns a
`layout-mismatch` error rather than guessing. This covers `SERIALIZE_OBJECTS`
and `OBJECTS` variants not matched by the table.

### JSON contract

Every call returns a plain object, never throws on bad input or stale
addresses, and contains no `BigInt`, `undefined`, `NaN`, `Infinity`, `Map`,
or functions:

```json
{ "ok": true, "schema": 1, "...": "..." }
{ "ok": false, "schema": 1, "error": { "code": "bad-address", "message": "..." } }
```

Logo values:

| Kind | JSON |
| --- | --- |
| word | `{"t":"word","s":"foo"}` |
| integer | `{"t":"int","v":3}` |
| float | `{"t":"float","v":1.5}` (non-finite: `"v":null,"repr":"inf"`) |
| list | `{"t":"list","items":[...]}` |
| array | `{"t":"array","origin":1,"items":[...]}` |
| shared or cyclic node | first visit carries `"addr"`; later visits are `{"t":"ref","addr":N}` |
| limit hit | `{"t":"truncated","reason":"depth"}` or `"count"` |

Each value also optionally carries `"text"`, a Logo-printable rendering, such as
`[a b [c]]`, for quick reading. Addresses are numbers. Limits default to
`depth: 8`, `count: 1000`, `bytes: 65536` and can be overridden per call.

### API surface

All methods return the contract above.

| Method | Purpose |
| --- | --- |
| `status()` | schema, layout check result, `safe` snapshot flags (`LOGO_GIL`, Asyncify state, `inside_gc`), heap counters |
| `workspace()` | procedures (user, macro, primitive, arity, flags), variables, property-list names; buried/traced/stepped flags from `obflags` |
| `procedure(name)` | formals, body lines, text, min/default/max arity, flags |
| `variable(name)`, `plist(name)` | global value; `[{prop, value}]` |
| `value(addrOrName, opts)` | any node as the Logo value model |
| `node(addr)` | raw node: decoded type flags, `car`/`cdr`/`obj`, generation, mark, id |
| `graph(addr, opts)` | replaces `pcons`: `{root, nodes:{addr:node}}` adjacency map |
| `nodes({offset, limit})` | replaces `dumpNodes`: paged heap walk by segment |
| `registers()` | evaluator `regs` as values (`proc`, `ufun`, `this_line`, ...) |
| `frames()` | `var_stack` dynamic variable frames (phase 3) |
| `json(method, ...args)` | `JSON.stringify` helper with indentation option |

`Module.logoGC` and `globalThis.logoGC` become a deprecated alias over
`node`/`nodes`/`graph` for one release, then are removed. `gdb.rc` is
unchanged.

### Address safety

An address is accepted only if it falls inside a known segment and is aligned
to the node stride. A free node (`NTFREE`) is returned as `{"freed":true}`.
Walks use a visited set and the count/depth/byte limits above. Strings are
read by `n_str`/`n_len` with bounds checked against heap size. Addresses are
snapshots: the collector can free or reuse a node afterward, and a JS-held
address is not a GC root.

### Mid-evaluation caveat

When called from `runjs` or the ASAN hook, in-flight values live in C locals
and the Asyncify stack, not `regs`, so `registers()` and `frames()` can show
transient state. `status().safe` reports the conditions and every result
includes the `status` flags it was read under.

## Phases

1. **Layout and memory access.** Add the `LAYOUT` table and static asserts,
   heap-view helper, data-symbol exports, and `status()`/`node()`. Confirm
   offsets on a real wasm32 build, including `sizeof(NODE)`.
2. **Logo value model.** Read words (confirm parity-bit/`clearparity` and
   UTF-8 decoding against `intern.c`, `parse.c`, and `print.c`), numbers,
   lists, arrays, `text`, `graph`, `nodes`. Walk `hash_table` for
   `workspace()`, `procedure()`, `variable()`, `plist()`.
3. **Registers and frames.** Study the `eval.c` save/restore stack discipline
   and `var_stack` layout before defining the `registers()` and `frames()`
   schema; document what is exact and what is transient.
4. **Removal.** Delete the C debug exports and `Module.logoGC` body, switch the
   `eval.c` ASAN hook, update docs. Build both targets with all export lists.
5. **Optional Logo access.** A `runjs` wrapper can call the inspector and
   queue the JSON string with `Module.FS_runjs_getChar_buffer_append_str`. It
   is read-only, so reentrancy is not an issue. Decide after phase 2.

Commit after each phase per the commit policy.

## Validation

- Node unit tests (added to `npm run test:source`, new
  `tests/acceptance/logo-inspect.test.js`): synthetic heap builder covering
  words, ints, floats, nested lists, arrays, shared and cyclic structures,
  limits, freed nodes, misaligned and out-of-range addresses, layout mismatch,
  and `JSON.parse(JSON.stringify(x))` round-trip equality for every method.
- Source guard test: `logo-inspect.js` contains no `Module._` calls, `ccall`,
  `cwrap`, `run_evaluator`, or heap writes; allowed `Module._*` references are
  an explicit data-symbol whitelist. Also assert no `logo_gc_debug` remains in
  `mem.c` or `Makefile.am`.
- Node/Wasm: build `ucblogo-node.js`, run `tests/test.lg` (65 Ok baseline), and
  a small script that defines procedures and variables via Logo and compares
  `workspace()`/`procedure()`/`variable()` to `po`/`pots` output.
- Browser: one TestCafe case in the existing web suite that defines a
  procedure, variable, list, and array, then calls `Module.logoInspect` while
  idle and while blocked in `readword`, asserting JSON content and no browser
  errors. Mid-evaluation via `runjs` in a separate case.
- ASAN build: confirm the `eval.c` hook now returns JSON for a poisoned node
  instead of calling C.
- Stale-artifact check before trusting results: rebuild both targets, restart
  the runtime, bypass the service worker.

## Risks and Open Decisions

- **Hard-coded layout.** JS mirrors C struct offsets. Mitigated by static
  asserts plus the runtime self-check; the alternative is a C-emitted constant
  layout table (data only), which removes duplication at the cost of a larger C
  change. Recommend the JS table with static asserts first.
- **Heap view export** may need `HEAPU8` in `EXPORTED_RUNTIME_METHODS`;
  confirm it does not change the size/behavior of the Asyncify build.
- **`Module._` data exports** must resolve to numeric addresses with
  `STRICT_JS` on the pinned Emscripten (as `_Unbound` does today); check after
  any toolchain upgrade (see [emscripten-upgrade.md](emscripten-upgrade.md)).
- **Transient state** (see above) cannot be eliminated in pure JS.
- **Compatibility alias** for `logoGC`: keep for one release or drop now.
  The skills docs ([gc-memory.md](../../.github/skills/ucb-logo-interpreter-development/references/gc-memory.md),
  [project-map.md](../../.github/skills/ucb-logo-interpreter-development/references/project-map.md))
  and [webedition.md](../../webedition.md) need updating either way.

## Attribution

Co-authored with GitHub Copilot.
