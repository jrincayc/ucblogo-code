---
description: "Use when editing the garbage collector, NODE allocation, evaluator registers, or other C code where object lifetime and GC roots matter."
applyTo: "mem.c,logodata.c,intern.c,lists.c,eval.c,logo.h"
---
# GC And Memory Checklist

- Every live NODE must stay reachable from a root across any allocation, GC, error, yield, or callback. Check C locals, globals, evaluator registers, interned symbols, and arrays separately.
- Do not nest allocators in one expression. Preallocate the wrapper cell, then fill it with `setcar`/`setcdr`.
- Mutate existing objects with `setcar`, `setcdr`, or `setobject` so write barriers and old-to-young tracking stay correct.
- A JavaScript-held Wasm address does not keep a NODE alive.
- Do not disable `-O1` or Asyncify to hide a lifetime bug. Prove the cause with ASAN/UBSAN, stable NODE IDs, and forced GC tests.
- Keep test parameters fixed while diagnosing a temporal GC bug, and run tests only after a successful build.

See the [GC reference](../skills/ucb-logo-interpreter-development/references/gc-memory.md) before changing collector behavior.
