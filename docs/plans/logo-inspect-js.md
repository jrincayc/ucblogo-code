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

Under ASAN, these raw reads do not detect poisoned nodes. They also must not
go through Emscripten's instrumented accessors (`getValue`, `_asan_js_load_*`),
which call C and would trap on a poisoned read. Core scope reads raw memory and
does not report poison; reporting it is the ASAN stretch goal below, so the
data model leaves room for it (an optional `asan` field on nodes, never a
thrown error).

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
  instead of `Module.logoGC.inspect`. The hook already reports poison itself
  through `__asan_address_is_poisoned` in C.

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

## Test-Driven Development

No implementation work begins until the full suite below is written, run, and
committed in a failing state, and you have reviewed it. Tests define the API
contract; implementation phases only turn named groups of them green.

### Rules

- Red first: before implementation, run every new test and record that each
  fails for the intended reason (missing `logo-inspect.js`, no
  `Module.logoInspect`, `logo_gc_debug` still present), not from a typo or
  harness error. Save the red run output path in the commit message.
- Tests are not weakened to pass. A change to an expected value needs a stated
  reason in the commit message. A bug found later gets a failing regression
  test before its fix.
- No silent skips. Real-runtime tests fail when the build artifact is missing;
  they are only excluded from `test:source`, not skipped inside a run.
- Commit the red suite as its own commit (intermediate commits need not pass).

### Test layers

1. **Contract tests** (Node, no Wasm; `tests/acceptance/logo-inspect.test.js`,
   added to `npm run test:source`). Written against `createLogoInspector` with
   injected memory and symbols:
   - every method exists, is synchronous, and returns the `{ok, schema}` or
     `{ok:false, error}` shape;
   - every result round-trips through `JSON.parse(JSON.stringify(x))` with
     deep equality and contains no `BigInt`, `undefined`, `NaN`, `Infinity`,
     `Map`, or function;
   - bad input (unknown name, `null`, non-number address, negative, huge) never
     throws and returns a coded error;
   - the value-model table above, limits (`depth`, `count`, `bytes`) with the
     `truncated` forms, cycle and sharing as `ref`, and `text` rendering;
   - address safety: out-of-segment, misaligned, and `NTFREE` addresses;
   - layout self-check failure returns `layout-mismatch` from every method;
   - the heap view is re-read on every call (swap the backing buffer between
     calls to simulate memory growth);
   - read-only: the synthetic memory is frozen or hashed before and after
     every call.
2. **Synthetic heap builder** (`tests/acceptance/helpers/logo-heap.js`, test
   code only): builds segments, nodes, words, lists, arrays, `CASEOBJ` object
   cells, and `hash_table` buckets in a `Uint8Array`. To avoid testing the
   layout against itself, it takes offsets from an independently written
   `tests/fixtures/logo-layout.json`; a source test asserts that fixture, the
   `LAYOUT` table in `logo-inspect.js`, and the `_Static_assert` values in
   `mem.c` all agree.
3. **Golden fixtures** (`tests/fixtures/logo-inspect/*.json`): expected JSON
   for each value kind, a procedure, a variable, a property list, and a
   registers sample, reviewed by hand once and then frozen.
4. **Source guard tests** (same file): `logo-inspect.js` contains no
   `Module._` calls, `ccall`, `cwrap`, `run_evaluator`, or heap writes; allowed
   `Module._*` references are an explicit data-symbol whitelist; no
   `logo_gc_debug` remains in `mem.c`, `Makefile.am`, `pre.js`, or `eval.c`;
   `Module.logoGC`, if kept, is only an alias; `pre.js` does not define the
   inspector.
5. **Real-runtime oracle tests** (Node/Wasm; new
   `tests/acceptance/logo-inspect-node.test.js` in `npm run test:built`,
   which fails if `ucblogo-node.js` is missing or older than its sources). A
   fixed Logo program defines procedures, variables, lists, arrays, words with
   special characters and non-ASCII text, a property list, buried and traced
   items, then calls the inspector through `runjs` and prints the JSON between
   marker lines. The test parses it and compares against Logo's own answers:
   `contents`/`buried`, `arity`, `text`, `thing`, `plist`, `procedurep`,
   `primitivep`. The Logo output is the oracle, so a mistake in the synthetic
   builder cannot hide a real layout error. Includes a forced-GC case that
   checks results stay valid and `NTFREE` addresses are reported, and a case
   where the Wasm memory has grown.
6. **Browser tests** (TestCafe, existing web suite): define a procedure,
   variable, list, and array; call `Module.logoInspect` while idle and while
   blocked in `readword`; assert JSON content, `status().safe`, and no
   console or page errors; one case from `runjs` mid-evaluation. Verify that no
   Wasm export is invoked by wrapping the exports object in the test page and
   counting calls during the inspector call (expected: zero).
7. **Build checks:** both targets link with the new export lists and
   `HEAPU8`; the `_Static_assert` block compiles; the ASAN build links and its
   `eval.c` hook returns JSON for a poisoned node (raw contents; poison
   reporting is the stretch goal).

### Test ownership by phase

| Phase | Turns green |
| --- | --- |
| 1 Layout and memory | layer 2 agreement test, layer 1 layout, address-safety, read-only, heap-refresh, `status`/`node` cases, build checks |
| 2 Value model | layer 1 value-model and limit cases, layer 3 value fixtures, layer 5 value, `workspace`, `procedure`, `variable`, `plist`, GC and memory-growth cases |
| 3 Registers and frames | `registers`/`frames` fixtures and layer 5/6 mid-evaluation cases, written after the study step defines the schema (see below) |
| 4 Removal | layer 4 source guards, ASAN build link and hook check |
| 5 Logo access | cases added only if the phase is approved |
| 6 ASAN poison (stretch) | ASAN-only cases listed in the stretch goal |

Phase 3 schema is unknown until `eval.c` is studied, so its tests are written
and committed red at the start of phase 3, before its implementation.

## Phases

0. **Test suite (red).** Write layers 1 to 6 for phases 1, 2, and 4 plus the
   build checks; commit red with the failure evidence. Review with the
   maintainer before continuing.
1. **Layout and memory access.** Add the `LAYOUT` table and static asserts,
   heap-view helper, data-symbol exports, and `status()`/`node()`. Confirm
   offsets on a real wasm32 build, including `sizeof(NODE)`. Make the phase 1
   tests green.
2. **Logo value model.** Read words (confirm parity-bit/`clearparity` and
   UTF-8 decoding against `intern.c`, `parse.c`, and `print.c`), numbers,
   lists, arrays, `text`, `graph`, `nodes`. Walk `hash_table` for
   `workspace()`, `procedure()`, `variable()`, `plist()`. Make the phase 2
   tests green.
3. **Registers and frames.** Study the `eval.c` save/restore stack discipline
   and `var_stack` layout, document what is exact and what is transient, write
   the red tests for the resulting schema, then implement.
4. **Removal.** Delete the C debug exports and `Module.logoGC` body, switch the
   `eval.c` ASAN hook, update docs. Build both targets with all export lists.
   Make the layer 4 guards green.
5. **Optional Logo access.** A `runjs` wrapper can call the inspector and
   queue the JSON string with `Module.FS_runjs_getChar_buffer_append_str`. It
   is read-only, so reentrancy is not an issue. Decide after phase 2; tests
   first if approved.

6. **Stretch: ASAN poison reporting.** See below. Only after phases 0 to 4
   are green; not required for the core API.

Commit after each phase per the commit policy. Run the full affected suite
(`test:source`, Node/Wasm, focused TestCafe) after each phase, and the
stale-artifact check (rebuild both targets, restart the runtime, bypass the
service worker) before trusting any real-runtime result.

## Stretch Goal: ASAN Poison Reporting

Under ASAN the inspector should report poisoned nodes instead of hiding
corruption: a poisoned node is still returned with its raw contents, flagged,
and never causes a throw or an ASAN abort.

Why it is a stretch: it depends on ASAN internals of the pinned Emscripten
rather than on Logo's own layout.

- Emscripten's wasm mapping is `shadow = address >> 3` with no offset
  (`asan_mapping_emscripten.h`), so the shadow bytes live in the same linear
  memory and JS can read them with the same raw view. Confirm on the pinned
  version and on `ALLOW_MEMORY_GROWTH`; a toolchain upgrade can change it.
- Shadow decoding: `0` means all 8 bytes addressable, `1..7` means only the
  first k bytes are, and negative values are poison kinds (heap freed,
  redzone, user-poisoned, and so on). Map the codes the Logo build produces.
- Logo poisons only `nunion` of free nodes (`POISON_NODE` in
  [globals.h](../../globals.h)), so the check covers that region and the
  header separately.
- ASAN fake stack frames and quarantine are out of scope.
- The inspector must detect an ASAN build (a data export such as
  `__asan_shadow_memory_dynamic_address` or a build-time flag exposed as data)
  and stay inert otherwise, without calling any export.

Output when active: each node adds
`"asan": {"poisoned": true, "kind": "heap-freed", "shadow": "0xfd", "region": "nunion"}`;
`status()` reports `"asan": {"active": true}`; every traversal result lists
`"poisonedNodes": [addr, ...]` so corruption is visible from the top-level
result. A value read from a poisoned node is returned as raw data inside a
`{"t":"poisoned", ...}` wrapper, not interpreted further.

Tests, written red before the stretch implementation:

- Node: synthetic heap with an injected shadow region; each shadow code maps
  to the expected `kind`, partial granules are handled, and no method throws on
  poisoned or partially poisoned nodes.
- Source guard: the inspector does not use `getValue`, `HEAP*` macros that the
  ASAN build rewrites, or any `_asan_*`/`__asan_*` call.
- ASAN build (Node/Wasm): free a list, collect, then inspect its old address and
  assert `poisoned: true` and a `poisonedNodes` entry; inspect a live node and
  assert no flag; deliberately corrupt via a test-only Logo path and confirm
  the report, with the process still alive afterward.
- A non-ASAN build reports `asan.active: false` and never reads the shadow
  region.

If the shadow check proves unreliable on the pinned Emscripten, drop it and
document that the C-side hook (`__asan_address_is_poisoned` in `eval.c`) is the
supported ASAN diagnostic.

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
