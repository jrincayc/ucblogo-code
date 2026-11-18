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

The inspector reads Wasm linear memory only (see Memory access). It never calls `Module._*` functions, `ccall`, `cwrap`, or
`Module.run_evaluator`. JS is single-threaded, so while the inspector runs the
interpreter cannot execute; a read is a consistent snapshot of whatever state
the runtime was suspended in. This is the same pattern already used for C
globals (`getValue(Module._Unbound, '*')` in [liblogo.js](../../liblogo.js)).

ASAN is out of scope for now (see the deferred section at the end).

### Memory access

Checked against the pinned Emscripten 4.0.5 and the current `ucblogo.js` and
`ucblogo-node.js`. All of these options are pure JS; none calls into Wasm.

| Option | Verdict |
| --- | --- |
| Typed views `HEAPU8`, `HEAP16`, `HEAP32`, `HEAPU32`, `HEAPF64` | **Use.** One indexed load per field, no per-call type dispatch, and the view type documents the C type. Bulk heap walks read thousands of nodes. |
| `getValue(ptr, type)` | Acceptable for one-off console use, not for the inspector. In this build it is a `switch` over the same views (`'*'` uses `HEAPU32`, `'i32'` is signed, `'i64'` returns a `BigInt`, which is not JSON). It adds nothing over the views and has no unsigned 8/16/32-bit integer types. |
| `setValue` | Never: the inspector is read-only. |
| `UTF8ToString(ptr)` | **Do not use for Logo words.** Words are `n_str`/`n_len` slices into shared `string_block`s, not NUL-terminated, and `ecma` (always on in `logo.h`) stores backslashed special characters as bytes `3..` (`ecma_set`, `special_chars` in `logodata.c`). Decode `HEAPU8.subarray(n_str, n_str + n_len)` with `TextDecoder`, then map those bytes back through `special_chars`, read from memory rather than copied into JS. |
| `DataView` | Not needed. Wasm is little-endian and NODE fields are naturally aligned. |

Rules for the views:

- Use the module-scope `HEAP*` variables. `logo-inspect.js` is a `--pre-js`, so
  it shares the module scope, as `pre.js` already does with `getValue` and
  `UTF8ToString`. Never cache a view in a variable or closure: on memory growth
  `updateMemoryViews()` replaces every view, and a cached one goes stale.
  `Module.HEAP*` is not exported and is not needed.
- `ptr >> 2` (or `>> 3`) silently floors a misaligned pointer onto the wrong
  field. Check alignment first: NODE addresses must be 8-byte aligned (the
  union holds a `double`), and 32-bit fields 4-byte aligned. A misaligned
  address returns a `misaligned` error, not a read.
- Pointers, `FIXNUM`/`long`, and `int` are 32-bit on wasm32. Read pointers with
  `HEAPU32` and signed fields (`nint`, `my_gen`, `n_len`) with `HEAP32`. Read the
  `nprim` shorts with `HEAP16`, `nfloat` with `HEAPF64`, and the
  `SERIALIZE_OBJECTS` `id` as two `HEAPU32` words, combined into a number only
  when it is below 2^53, otherwise as a hex string.
- `nprim_fun` is a function-table index, not a memory address. Report it as
  a number and never follow it.

### C-side changes (data and deletions only, no new code)

- Delete the `logo_gc_debug_*` functions from `mem.c` and their entries from
  both `EXPORTED_FUNCTIONS` lists in `Makefile.am`.
- Export data symbols needed as roots: `_hash_table`, `_segment_list`,
  `_regs`, `_var_stack`, `_the_generation`; `_Unbound`, `_inside_gc`, and
  `_LOGO_GIL` are already exported. JS reads these as addresses, like
  `Module._Unbound` today.
- Export `_special_chars` (data) for word decoding. No
  `EXPORTED_RUNTIME_METHODS` change is needed, since the views are in scope.
- Add a `__EMSCRIPTEN__`-guarded `_Static_assert` block in `mem.c` for
  `sizeof(NODE)`, the `NODE` and `struct segment` field offsets, and the
  `struct registers` offsets the JS layout table depends on. A layout change
  then fails the build instead of silently misreading memory.
- `eval.c` ASAN hook: call the new pure-JS `Module.logoInspect.node($0)`
  instead of `Module.logoGC.inspect`. The hook already reports poison itself
  through `__asan_address_is_poisoned` in C.

### New file: `logo-inspect.js`

Same shape as [storage-status.js](../../storage-status.js): a factory
`createLogoInspector({ memory, symbols, layout })` that takes the heap view
and root addresses as inputs. At the
bottom it exports via `module.exports` and `globalThis`. In the Web/Node builds
it is linked with `--pre-js logo-inspect.js` after `pre.js`, and attaches
`Module.logoInspect` once the runtime is initialized. It adds no deployed
asset, since `--pre-js` code is inlined into `ucblogo.js`.

Layout table: a single `LAYOUT` object (wasm32: 4-byte pointers and `FIXNUM`,
`NODE` fields, `NT_*` flag values, `struct segment`, `struct registers`, object
cell offsets). At attach time the inspector self-checks the layout (`Unbound`
node type, first segment `size`, node stride, and an object cell in the
`hash_table` CONS chains with a readable nonempty canonical string). On mismatch
every call returns a `layout-mismatch` error rather than guessing. This covers
`SERIALIZE_OBJECTS` and `OBJECTS` variants not matched by the table.

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

Each value also carries `"addr"` so a caller can drill down to `node()`
or `tree()`. Each value also optionally carries `"text"`, a Logo-printable rendering, such as
`[a b [c]]`, for quick reading. Addresses are numbers. Limits default to
`depth: 8`, `count: 1000`, `bytes: 65536` and can be overridden per call.

### Raw NODE

`node(addr)` returns the struct exactly as stored, plus decoding. This is the
lowest level, and the other methods are built on it:

```json
{
  "addr": 1052744, "segment": {"addr": 1048576, "index": 104},
  "node_type": {"value": 768, "octal": "01400", "name": "INTT",
                "flags": ["NT_WORD", "NT_NUMBER"]},
  "id": null, "my_gen": 0, "gen_age": 3, "mark_gc": 12,
  "next": 1052784, "oldyoung_next": 0,
  "union": {"view": "nint", "nint": 42},
  "words": ["0x00000300", "0x00000000", "..."],
  "bytes": "0003000000000000..."
}
```

- `node_type.name` is the matching `NODETYPES` composite (`CONS`, `STRING`,
  `CASEOBJ`, `ARRAY`, `LINE`, ...), or `null` if none matches. `flags` always
  lists the individual `NT_*` bits. `NTFREE` (`-1`) gives `name: "NTFREE"`.
- `union.view` is chosen from the type: `ncons` (`ncar`, `ncdr`, `nobj`),
  `nstring` (`nstring_ptr`, `nstring_head`, `nstring_len`, plus `refcnt` from
  the string block and the decoded `text` and raw hex `strbytes`), `nprim`
  (`fun`, `priority`, `min`, `dflt`, `max`), `nint`, `nfloat`, or `narray`
  (`dim`, `origin`, `data`, the element pointer list). For an unknown type,
  `union.view` is `"unknown"`.
- `words` (32-bit hex) and `bytes` (hex) dump the whole `sizeof(NODE)` struct,
  so nothing depends on the decoding being right.
- Pointer fields are numbers. `0` is `NIL`. Known globals are annotated:
  `{"addr": N, "sym": "Unbound"}`.

### NODE tree

`tree(root, opts)` walks from `root` and returns the full NODE tree as nested
raw nodes. `root` can be an address or a name. Each pointer field is replaced
by the child's raw node. A node already emitted becomes `{"ref": addr}` (this
handles shared structure and cycles). Limits give `{"truncated": "depth"}` or
`"count"`. Children: `ncar`, `ncdr`, `nobj` for cons-like nodes, every `narray`
element, and `nstring_head` as a raw string block, `{refcnt, bytes}`.

Given a name, the root is the Logo object found by scanning `hash_table`:
the cons list `[canonical procnode valnode plist obflags caselist...]` from
the `__object` accessors in `logo.h`. With `opts.roles` (default on), each
child also gets a `role` from those accessors. Object cells get `canonical`,
`procnode`, `valnode`, `plist`, `obflags`, and `caselist`. Procnode cells get
`text`, `formals`, `bodywords`, `minargs`, `dfltargs`, and `maxargs`. Case
objects get `strnode` and `object`. Roles are labels only; the raw fields are
unchanged. `tree` replaces `pcons`/`graph`.

### API surface

All methods return the contract above.

| Method | Purpose |
| --- | --- |
| `status()` | schema, layout check result, `safe` snapshot flags (`LOGO_GIL`, Asyncify state, `inside_gc`), heap counters |
| `workspace()` | procedures (user, macro, primitive, arity, flags), variables, property-list names; buried/traced/stepped flags from `obflags` |
| `procedure(name)` | formals, body lines, text, min/default/max arity, flags |
| `variable(name)`, `plist(name)` | global value; `[{prop, value}]` |
| `value(addrOrName, opts)` | any node as the Logo value model |
| `node(addr)` | one raw NODE (see Raw NODE) |
| `tree(addrOrName, opts)` | full raw NODE tree from an address or a Logo object (see NODE tree) |
| `nodes({offset, limit})` | replaces `dumpNodes`: paged heap walk by segment, raw nodes |
| `registers()` | evaluator `regs` as values (`proc`, `ufun`, `this_line`, ...) |
| `frames()` | `var_stack` dynamic variable frames (phase 3) |
| `json(method, ...args)` | `JSON.stringify` helper with indentation option |

`Module.logoGC` and `globalThis.logoGC` become a deprecated alias over
`node`/`nodes`/`tree` for one release, then are removed. `gdb.rc` is
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

The inspector is a debugging aid, so the suite stays small: run Logo code in
the live Web Edition, call the inspector, and check that it found what the
code created. Tests are written and run red (no `Module.logoInspect` yet)
before any implementation, committed on their own, and reviewed.

### Harness

One TestCafe fixture, `tests/acceptance/logo-inspect.testcafe.js`, which the
shared runner discovers automatically. It uses the existing `ready`,
`runLogo`, and idle helpers in
[testcafe-helpers.js](../../tests/acceptance/testcafe-helpers.js). One local
helper:

```js
const inspect = ClientFunction((method, args) =>
    JSON.stringify(Module.logoInspect[method].apply(Module.logoInspect, args)));
```

(`apply`, not spread: TestCafe client functions lack the Babel helpers for it.)

The test parses the string in Node. That alone checks the JSON requirement.
Every case also asserts `JSON.parse(JSON.stringify(r))` deep-equals `r`. Each
case runs Logo through `runLogo`, waits for idle, inspects, and compares
against values written in the test. Anything after `bye` is out of scope.

The Node/Wasm build is not used: its `main` reads stdin synchronously and has
no `/dev/runjs`, so there is no point at which JS can inspect a live program.

### Test cases

| # | Logo | Inspect | Expect |
| --- | --- | --- | --- |
| 1 | (none) | `status()` | `ok`, `schema: 1`, layout check passed, idle flags |
| 2 | `make "n 42` | `variable("n")` | `{t:"int", v:42}` |
| 3 | `make "f 1.5` | `variable("f")` | `{t:"float", v:1.5}` |
| 4 | `make "w "hello  make "v "\|a b\|` | `variable("w")`, `variable("v")` | words `hello`, `a b` |
| 5 | `make "l [a [b c] 3]  make "e []` | `variable("l")`, `variable("e")` | nested list items and `text` `[a [b c] 3]`; empty list |
| 6 | `make "a {1 2 3}@0` | `variable("a")` | `{t:"array", origin:0}`, 3 items |
| 7 | `make "c [x y]  .setbf :c :c` | `variable("c")` | completes; a `{t:"ref"}` closes the cycle |
| 8 | `make "d [[[[[[[[[[1]]]]]]]]]]` | `value("d", {depth:3})` | `{t:"truncated", reason:"depth"}` at level 3 |
| 9 | `define "sq [[x] [output :x * :x]]` | `procedure("sq")` | formals `[x]`, arity 1/1/1, body line `output :x * :x` |
| 10 | `define "opt [[a [b 2] [c] 1]]` | `procedure("opt")` | arity min 1, default 1, max -1 |
| 11 | (none) | `procedure("sum")` | primitive, arity 0/2/-1 |
| 12 | `pprop "p "k 7` | `plist("p")` | `[{prop:"k", value:{t:"int", v:7}}]` |
| 13 | `bury "sq` | `workspace()` | lists `sq` (buried), `n`, `p` |
| 14 | (none) | `variable("nosuch")`, `node(0)`, `node(1)`, `node(2**32)` | `ok:false` with an error code; nothing throws |
| 15 | `make "r readword` (submitted without `runLogo`; see below) | `variable("n")`, `status()` | works while Logo is suspended in Asyncify; `status` shows not idle; after a line is typed, `:r` has that text and Logo returns to idle |
| 16 | `ignore runjs "\|window.__i = JSON.stringify(Module.logoInspect.variable('n'))\|` (normal `runLogo`) | read `window.__i` | inspector called from inside a C call returns `{t:"int", v:42}` and Logo continues |
| 17 | `gc` after cases 2 to 6 | repeat cases 2 to 6 | values unchanged after collection |
| 18 | grow the heap with a large list until `HEAPU8.length` increases | `variable("n")` | still correct after memory growth |
| 19 | (case 2) | `node(addr of :n's value)` | `node_type` `{value:768, octal:"01400", name:"INTT"}`, `union.nint` 42, `words.length` = `sizeof(NODE)/4`, `bytes` length matches |
| 20 | (case 2) | `node(addr + 4)` | `ok:false`, code `misaligned` |
| 21 | (case 5) | `tree("l")` | root is `l`'s object cell; the `valnode` child is `CONS`; three `ncdr` cells end in `0`; the second `ncar` is `CONS`; leaf `nstring.text` values are `a`, `b`, `c`, `3` |
| 22 | (case 7) | `tree("c")` | terminates; contains `{ref: <first cell addr>}` |
| 23 | (case 9) | `tree("sq")` | has a `procnode` child with `formals` and `minargs` roles; `minargs` is `nint` 1 |
| 24 | (case 21) | `node(a)` for each `a` in the tree | equals that node in the tree, ignoring nested children |
| 25 | `make "s "a\ b` | `tree("s")` value word | `nstring.text` `a b`; `strbytes` `612062` (the quoted-word path clears parity before storing the space) |

Cases 9 and 10 cover the arity decoding that is easy to get wrong. Cases 15
and 16 are the reason the inspector exists: they show it is safe at the two
points where calling C was not. Cases 19 to 25 check the raw layer that the
other cases are built on.

Case 15 cannot use `runLogo`, whose final step waits for idle, and idle
requires `!Module.blockedOnGetc`; it would time out. Follow the existing
pattern in
[autocomplete.testcafe.js](../../tests/acceptance/autocomplete.testcafe.js)
(`print [`): wait for idle, type the command and press enter directly, wait
for `Module.blockedOnGetc`, inspect, then type a line and wait for idle. Case
16 uses plain `runLogo`: the `runjs` call finishes inside the command, so Logo
is idle afterward. The runjs result is an operation, so the call needs
`ignore`, as in `logo-suite.testcafe.js`.

Phase 3 adds two cases once the schema is defined. One calls `registers()` and
`frames()` while `w` (defined by `define "w [[] [local "x make "x 5 make "r readword]]`) is waiting, and
expects `x = 5` in a frame. The other checks the same frame from `runjs`.

One source check is added to
[source.test.js](../../tests/acceptance/source.test.js): `logo-inspect.js`
contains no `Module._` function calls, `ccall`, `cwrap`, or `getValue`, and no
`logo_gc_debug` remains anywhere. This enforces the pure-JS rule without a
runtime spy.

### Running

`node tests/acceptance/run-testcafe.js --test-grep 'Logo inspector'` against
a fresh build. Desktop Chrome only; other browsers are not needed for a
debugging aid.

## Phases

0. **Tests (red).** Write cases 1 to 25 and the source check. Confirm each
   fails because `Module.logoInspect` is missing, not from a harness error.
   Commit and review.
1. **Layout and memory access.** `LAYOUT` table, static asserts, heap view,
   data-symbol exports, `status()`, `node()`. Green: 1, 14, 19, 20.
2. **Logo value model.** Words, numbers, lists, arrays, limits, `text`,
   `tree`, `nodes`, `workspace`, `procedure`, `variable`, `plist`. Green: 2 to
   13, 15 to 18, and 21 to 25.
3. **Registers and frames.** Study `eval.c` and `var_stack`, add the two
   phase 3 cases red, then implement.
4. **Removal.** Delete the C debug exports and `Module.logoGC`, switch the
   `eval.c` ASAN hook, update docs. Green: the source check.
5. **Optional Logo access.** Decide after phase 2; tests first if approved.
6. **Deferred: ASAN poison reporting.** See below.

Commit after each phase. Rebuild and bypass the service worker before
trusting a browser result.

## Deferred: ASAN Poison Reporting

Out of scope for now; kept for reference.

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

Tests, written red first, in an ASAN web build with the same fixture: case
17's collection frees an erased list (`ern "l gc`), and inspecting its old
address reports `poisoned: true` without throwing. A live node reports no
flag. A non-ASAN build reports `asan.active: false`.

If the shadow check proves unreliable on the pinned Emscripten, drop it and
document that the C-side hook (`__asan_address_is_poisoned` in `eval.c`) is the
supported ASAN diagnostic.

## Risks and Open Decisions

- **Hard-coded layout.** JS mirrors C struct offsets. Mitigated by static
  asserts plus the runtime self-check; the alternative is a C-emitted constant
  layout table (data only), which removes duplication at the cost of a larger C
  change. Recommend the JS table with static asserts first.
- **Heap views in scope** depend on `--pre-js` sharing the module scope and on
  `updateMemoryViews` naming; recheck after any toolchain upgrade.
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
