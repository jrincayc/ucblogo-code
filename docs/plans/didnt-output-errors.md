# "X didn't output to Y" Error Attribution Plan

Status: Planned; investigation has not started. Source claims reviewed against
the tree on 2026-11-15.
Date: 2026-11-15

## Objective

Brian Harvey's wish: the evaluator reports every `X didn't output to Y in Z`
error with the correct `X`, `Y`, and `Z`, in all cases, without breaking tail
call elimination and without the current pile of special cases.

This plan covers investigation only. No evaluator change is proposed until
Phases 0-3 produce a written specification and a failing-case corpus.

## Message Anatomy

`DIDNT_OUTPUT` (message 5) formats as `%p didn't output to %p`. `err_logo`
records `ufun` and `this_line` in `err_mesg`; `err_print` appends the
`in <ufun> <line>` suffix only when printing to the terminal
([error.c](../../error.c#L88)).

- `X`: the procedure that failed to produce a value. Today taken from
  `didnt_output_name`, else `last_call`
  ([error.c](../../error.c#L120-L131)).
- `Y`: the consumer that demanded a value, passed as `error_desc` or taken from
  `car(didnt_get_output)`.
- `Z`: `ufun` and `this_line`, overwritten from `didnt_get_output` when
  `error_desc` is `NIL`. This overwrites the live evaluator registers, not just
  the message fields.

`ERROR` ([lerror](../../error.c#L239-L251)) outputs
`[5 |X didn't output to Y| ufun line]`: item 2 is the formatted message word
without the `in ...` suffix. Changing which names fill `X` or `Y` therefore
changes item 2.

## Current Mechanism (to verify against source, not assumed)

Three overlapping pieces of state decide the message:

1. `didnt_get_output` ([eval.c](../../eval.c)): `UNBOUND` ("command context, no
   value wanted"), `NIL` ("value or no value OK", the `runresult` case),
   `the_generation` (macro call: value wanted, but not as an actual argument;
   [eval.c](../../eval.c#L577-L578)), or a list `(consumer ufun line)` built by
   `cons_list` at arg evaluation, default expression evaluation, and
   `OUTPUT`/`.MAYBEOUTPUT` tail transitions. In the default-argument path the
   third element is the default expression, not `this_line`
   ([eval.c](../../eval.c#L803)). It is saved and restored with the registers
   (`eval_save` copies the whole struct), and it is also used as a mode flag at
   compound procedure entry to compute `val_status`
   ([eval.c](../../eval.c#L905-L911)).
2. `didnt_output_name`: a "culprit" register. It is cleared to `NIL` at the
   start of each argument and default evaluation, at `OUTPUT`/`.MAYBEOUTPUT`
   tail transitions, and by `err_logo` after use. It is set to `fun` at compound
   procedure entry only if `NIL`, in `loutput`/`lstop`, in the `OP` with
   `VALUE_OK` path, and in `macro_return`; the `STOP_PRIORITY` branch sets it
   to `car(expresn)`. Set-only-if-`NIL` deliberately names the outermost
   procedure entered for an operand, but the `OUTPUT` tail path clears it
   ([eval.c](../../eval.c#L984)), so that path names the innermost callee.
3. `last_call`: a global, not a register, so it is not saved or restored. It
   is assigned from `fun` in `accumulate_arg` and in the default-argument path
   immediately before the check, and `err_logo` copies `didnt_output_name` into
   it. As a fallback it names whatever `fun` held when the callee returned,
   which after a tail call is the innermost primitive, not the procedure the
   user called.

Detection happens at several independent sites, each with its own rules:

- `accumulate_arg` and `set_args_continue`: `STOPPING`/`OUTPUT` flag mismatch,
  or `val == UNBOUND`. The flag-mismatch call `err_logo(DIDNT_OUTPUT, fun)`
  runs before `eval_restore`, right after `last_call = fun` and
  `didnt_output_name = NIL`, so `X` and `Y` are the same `fun`
  ([eval.c](../../eval.c#L503-L513), [eval.c](../../eval.c#L845-L855)).
  The `val == UNBOUND` path retries `err_logo` in a loop while `erract`
  returns no value.
- `loutput` and `lstop` ([coms.c](../../coms.c#L61-L99)): `val_status` says
  output or stop is not permitted. `loutput` carries the comment "Not quite
  right; could be .maybeoutput". `after_maybeoutput` calls both directly from
  C ([eval.c](../../eval.c#L1081-L1087)), so `fun` there is not necessarily
  `stop`/`output`.
- The `STOP_PRIORITY` branch of `eval_sequence`
  ([eval.c](../../eval.c#L1018-L1047)): the `OUTPUT_OK` sub-case raises no
  error at all when `didnt_get_output` is `NIL` or `UNBOUND` ("actually can
  happen: PRINT FOREACH ..."), `ERR_MACRO` for macros, else `DIDNT_OUTPUT` with
  whatever `didnt_output_name` holds; the final sub-case names `stop` and
  reports `AT_TOPLEVEL` during `PAUSE`.
- `fall_off_want_output` and `macro_return`: end-of-body paths with their own
  conditions. `catch_followup` and `withobject_followup` only save/restore
  `didnt_output_name` and `didnt_get_output` and raise `DK_WHAT`
  ([eval.c](../../eval.c#L1366), [eval.c](../../eval.c#L1406));
  `withobject` exists only under `OBJECTS`.

Tail calls interact because a tail call discards the caller's frame:
`fun`, `ufun`, `this_line`, `didnt_output_name`, and `didnt_get_output` are
rewritten in place (`tail_eval_dispatch` and compound procedure entry), so the
original callee name and the consumer must survive in registers rather than on
the stack. (`begin_apply` is the `apply` entry point, not procedure entry.)

## Hypotheses To Test

Each is falsifiable with one Logo snippet; none is assumed true.

- H1: Tail-call attribution is inconsistent by path. A chain reached by
  falling off the end or by `stop` blames the outermost procedure (set only if
  `NIL` at entry); a chain through tail `OUTPUT` blames the innermost callee
  (cleared at the transition); the `last_call` fallback can name a primitive.
- H2 (moved to Phase 0 as a spec question): at a tail `OUTPUT`, `Y` becomes
  `output` rather than the original consumer. This is what keeps tail calls
  constant-space, and "g didn't output to output in f" is defensible.
- H3: `lstop` and `loutput` use `fun` as `X`. That is the `stop`/`output`
  primitive only when they are reached as primitives; from
  `after_maybeoutput` it is whatever the evaluated expression left in `fun`.
- H4: Error text differs between interpreted contexts that should behave alike:
  `catch`, `run`/`runresult`, `apply`/`foreach`/`map` templates, macros and
  `.macro` expansion, default-value expressions, `withobject` (`OBJECTS` builds
  only).
- H5: `err_logo` overwrites the live `ufun`/`this_line` registers from
  `didnt_get_output`; because `DIDNT_OUTPUT` is recoverable, execution can
  continue with them clobbered after `erract` or `pause`/`co` supplies a value.
  In the default-argument path `Z`'s line is the default expression.
- H6: In the `val == UNBOUND` retry loop, the first `err_logo` clears
  `didnt_output_name`, so later iterations fall back to `last_call`, a global
  that the `erract` code itself updates through `accumulate_arg`; a retry can
  name an unrelated procedure. Also check whether `catch "error` or
  `errorfunc` leaves `didnt_output_name` set for the next error.
- H7: The flag-mismatch path in `accumulate_arg`/`set_args_continue` reports
  "F didn't output to F" (same `fun` for `X` and `Y`).
- H8: The `STOP_PRIORITY` `OUTPUT_OK` sub-case with `didnt_get_output` `NIL` or
  `UNBOUND` reports nothing at the failing site and relies on an outer check.

## Phase 0: Specification and Oracle

1. Write the intended meaning of `X`, `Y`, `Z` in one paragraph per case class,
   and get Brian Harvey's (or the maintainer's) confirmation before coding.
   Open questions to settle first:
   - For `to f stop end  print f`: is `X` `f` or `stop`?
   - For a tail-call chain `a -> b -> c` where `c` fails: is `X` `c` (innermost
     procedure) or `a` (what the user called)? Tail-call elimination makes the
     answer a design choice, not a given. Today the answer depends on whether
     the chain goes through tail `OUTPUT` (H1).
   - At a tail `OUTPUT`, is `Y` `output` or the original consumer (H2)?
   - Is `Y` the primitive/procedure name or the operand position text? What
     should `Y` show for an infix operand such as `1 + f`?
2. Record current behavior on this tree and, if available, on stock UCBLogo
   6.x and the upstream tree, as a baseline. Do not treat any of them as
   correct. Record the current `tests/test.lg` pass count at the same time
   rather than relying on a remembered number.

## Phase 1: Failing-Case Corpus

Build a table-driven set of Logo snippets crossing:

| Axis | Values |
| --- | --- |
| Failing callee | user procedure ending in a command; `stop` in body; falls off end after `if`/`ifelse`/`repeat`/`foreach` with no output; primitive command used as operand; macro returning non-list or calling `stop`; `.maybeoutput` with no value |
| Position | direct operand; tail of `output`; tail of procedure body (tail call); non-tail in sequence; nested operand; infix operand; parenthesized variadic call; default-value expression |
| Consumer `Y` | primitive (`print`), user procedure, `output`, `apply`/`map`/`foreach` template, `run`/`runresult`/`catch`, `make`, `if` predicate |
| Depth | direct; one tail call; chain of 3 tail calls; tail call through `if`/`ifelse`/`run`/`catch` |
| Context | toplevel, inside procedure, inside `pause`, with `erract`/`catch "error` active, `erract` that runs code and returns no value (H6) |

Include named rows for H7 ("F didn't output to F") and H8 (no error at the
failing site). Mark `withobject` rows as `OBJECTS`-only.

For each row record `ERROR` output (`[5 |X didn't output to Y| ufun line]`;
parse `X`/`Y` from item 2) and the printed message.

Harness constraints:

- `RunTests` in [tests/UnitTests.lg](../../tests/UnitTests.lg) runs every test
  inside `RUN`, `CATCH "Error`, and `FOREACH`, which are themselves H4
  contexts. Toplevel, `pause`, and printed-message rows need a separate
  stdin-driven run of `ucblogo-node.js` that compares transcript text.
- The harness reports only Ok, Failed, or Error, and the acceptance grader
  allows exactly one hard-coded XFAIL (`Tests.File.LiteralDribble`). There is
  no expected-failure mechanism. Write characterization tests that assert
  current output, with the intended output in a comment, and flip each
  assertion in the commit that fixes it; or keep the file out of the default
  run until fixed.
- A new `tests/UnitTests-DidntOutput.lg` runs only if a `LOAD` line is added
  to `tests/UnitTests.lg`. Follow the pattern in
  [tests/UnitTests-Macros.lg](../../tests/UnitTests-Macros.lg), whose existing
  case checks only error number 5 and so is unaffected by `X`/`Y` changes.

## Phase 2: Instrumentation

Add a temporary, compile-time-guarded trace logging, at each detection site
and each write to `didnt_output_name`, `didnt_get_output`, `last_call`,
`ufun`: site label, old and new values, `val_status`, `tailcall`, and
`stopping_flag`.

The existing `debprint` hook (`DEBUGGING` in eval.c, runtime-gated by
`redefp`) already dumps `didnt_get_output`, `didnt_output_name`,
`val_status`, and `stopping_flag` at every label via `do_debug`
([eval.c](../../eval.c#L75-L77)); add `last_call`, `ufun`, `this_line`, and
`tailcall`. [tests/test.lg](../../tests/test.lg#L1) sets `redefp` true, so
enabling `DEBUGGING` floods the whole suite: use a separate guard for the new
trace or run the corpus without `redefp`.

Run the corpus and attach traces to the failing rows. Remove the trace or keep
it only behind a debug guard before merging.

## Phase 3: Classify

For each failing row identify the root cause category:

- stale or wrongly cleared `didnt_output_name`;
- `last_call` fallback naming an inner primitive;
- `didnt_get_output` rebuilt with the wrong consumer at a tail transition;
- a detection site that omits attribution (`err_logo(DIDNT_OUTPUT, NIL)` with
  no culprit set);
- wrapper paths (`catch`, macros, `withobject`) restoring registers before or
  after the check in the wrong order;
- `err_logo` side effects (`ufun`/`this_line` overwrite, clearing the name).

Count sites per category. If most failures share one cause, a local fix is
viable; if causes are spread across detection sites, a structural change is
needed.

## Phase 4: Design Candidates

Evaluate against these invariants:

- Tail calls remain O(1) in control stack, `var_stack`, and heap per
  iteration. Any added state is a fixed number of registers overwritten at
  each tail transition, never a growing list.
- Attribution is computed in one place from explicit state, replacing the
  `didnt_output_name`/`last_call` fallback chain.
- GC safety: any new node register belongs in `struct registers`
  ([logo.h](../../logo.h)) before `r_val_status`
  ([logo.h](../../logo.h#L620)), so `eval_save` copies it and
  `num_saved_nodes` marks saved copies; the live register also needs an
  explicit `mark()` in [mem.c](../../mem.c#L784-L785). Follow
  [gc-memory.instructions.md](../../.github/instructions/gc-memory.instructions.md).
- Preserve the `val_status` flag semantics and the macro-caller context.
- No change to the `ERROR` list shape `[5 |message| ufun line]`, error number,
  or message template. Item 2 text is expected to change for corrected rows;
  the existing `tests/UnitTests-Macros.lg` case checks only the error number.

Candidates:

1. Minimal: a single `expect` record `(consumer, consumer-ufun, consumer-line)`
   (today's `didnt_get_output` list) plus a single `blame` register holding the
   procedure name chosen by the Phase 0 decision (innermost: overwritten at
   compound procedure entry unconditionally; outermost: set at argument start
   and never touched by tail transitions), and at primitive entry for
   `stop`/`output`. Remove `didnt_output_name` and the `last_call` use in
   `DIDNT_OUTPUT`. This is constant size under tail calls.
2. Separate the two roles of `didnt_get_output`: a small integer or flag for
   `val_status` computation, and the `expect` record used only for messages.
3. Defer message construction: record only the failing site and register
   snapshot at detection, build `X`/`Y`/`Z` in one helper.

Prototype the smallest candidate that makes the corpus pass, then re-evaluate
for simplification.

## Phase 5: Validation

- Narrow: `node ./ucblogo-node.js tests/test.lg` after
  `source ~/emsdk/emsdk_env.sh && emmake make ucblogo-node.js` (the pass count
  recorded in Phase 0 plus the new file).
- Tail-call regression: a tail-recursive loop of at least several million
  iterations through `if`, `ifelse`, `run`, `catch`, and a 3-procedure
  mutual-recursion cycle; assert it completes and that `NODES` sampled before
  and after (after `gc`) shows no growth proportional to iteration count,
  comparing old and new builds. Also a failing-output error raised after deep
  tail recursion to confirm attribution without stack growth.
- GC: run the corpus under the ASAN profile ([address-sanitizer.txt](../../address-sanitizer.txt))
  and at `-O1` per the interpreter skill's GC guidance.
- Native: build wx/native target (or confirm shared-source compile) since
  `eval.c`, `coms.c`, and `error.c` are shared.
- Browser: one TestCafe smoke run of the Web Edition with a rebuilt artifact;
  stale-artifact checks first.

## Risks

- `didnt_get_output` doubles as control state; separating it risks changing
  `val_status` and therefore which forms are tail calls.
- User programs and libraries may rely on current (wrong) messages via
  `ERROR` item 2; keep the template and list structure, and list each changed
  item 2 text in the corpus so the change is deliberate.
- The `STOP_PRIORITY` branch contains cases documented as "actually can
  happen", including one that raises no error at the failing site (H8); they
  reflect real unsolved attribution and must be enumerated, not deleted.
- The `DIDNT_OUTPUT` path mutates live `ufun`/`this_line` and is recoverable,
  so fixing attribution can change where execution resumes after `erract`.
- Behavior differs from other Logo implementations; do not conform to them
  without the Phase 0 decision.

## Deliverables

1. Specification paragraphs and decisions on the Phase 0 questions.
2. Corpus and baseline table (current output vs. expected).
3. Root-cause classification from Phase 3.
4. Chosen design with the tail-call and GC invariants checked.
5. Implementation in small commits, each leaving the corpus at least as green
   as before.
