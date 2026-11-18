# "X didn't output to Y" Error Attribution Plan

Status: Phase 0 working specification recorded and backed by the PLM; Phase 1
corpus (32 cases) captured on this tree and upstream. Remaining before
implementation: open sub-questions, `pause`/`erract` rows, and Brian Harvey's
confirmation.
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

H1 and H3-H8 remain falsifiable hypotheses; H2 is resolved in Phase 0 below.

- H1: Attribution along value-forwarding chains through `output` is
  inconsistent. Refuted for user procedures: the `OUTPUT`-tail chain blames the
  innermost callee, `output`-then-fall-off (case 05) blames the procedure that
  fell off, and `commandtaila` correctly blames `commandtaila`, because
  falling off the end with `OUTPUT_OK` pushes `fall_off_want_output` and calls
  `commandtailb` as a command. The inconsistency is in value-forwarding
  primitives instead (H4).
- H3: `lstop` and `loutput` use `fun` as `X`. That is the `stop`/`output`
  primitive only when they are reached as primitives; from
  `after_maybeoutput` it is whatever the evaluated expression left in `fun`.
  Untested: the `directstop` probe's `stop` was handled by the
  `STOP_PRIORITY` branch of `eval_sequence`, not by `lstop`.
- H4: Error text differs between interpreted contexts that should behave alike.
  Confirmed by the corpus: `run` is transparent as an operand (case 19 blames
  the callee) but not under `output` (case 28 blames `run`); `apply`, `catch`,
  and `map` blame themselves or library internals (cases 16, 18, 21, 29);
  `.maybeoutput` reports a callee as failing to output to `.maybeoutput`,
  which accepts no value (case 24); an anonymous `apply` template reports
  error 29 (case 17). `withobject` remains untested (`OBJECTS` builds only).
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
  "F didn't output to F" (same `fun` for `X` and `Y`). Not reproduced by any
  corpus case; reachability still needs source analysis.
- H8: The `STOP_PRIORITY` `OUTPUT_OK` sub-case with `didnt_get_output` `NIL` or
  `UNBOUND` reports nothing at the failing site and relies on an outer check.
  Not reproduced: `print foreach [1 2] [stop]` (case 08b), the shape named in
  the source comment, reports error 29 "Macro returned [] instead of a list".

## Phase 0: Specification and Oracle

1. Use this working specification for corpus expectations; confirm it with
   Brian Harvey before coding:
   - `X` is the innermost procedure in the value-forwarding chain (callees
     whose value becomes their caller's value through `output` or
     `.maybeoutput`) that fails to return a value. Thus `to f stop end` used
     as an operation blames `f`; `a: output b`, `b: output c`, `c: stop`
     blames `c`; `a: b` (falls off the end after calling `b` as a command)
     blames `a`; and `a: output b`, `b: c`, `c: stop` blames `b`.
   - `X` may be a primitive: a primitive command used as an operand blames the
     primitive (`print ignore 3` -> `ignore`), as the PLM oracle below
     requires. Still open: an anonymous template (`apply [[x] [stop]] [1]`;
     proposed: the template text, as the `%p` format prints it), whether
     value-forwarding primitives (`apply`, `run`, `catch`, `map`, `foreach`)
     are transparent so that `X` is the callee inside them, and macros (which
     report `ERR_MACRO`, error 29, rather than `DIDNT_OUTPUT`).
   - `Y` is the immediate operation requiring the missing value. At a tail
     `OUTPUT`, it is `output`; for `1 + f`, it is `+`, not an operand position.
   - `Z` is the user procedure and source expression containing that immediate
     consumer, where one exists. A top-level consumer has no user procedure or
     source line.
   Confirmation is still required before implementation; in particular,
   confirm innermost attribution within the value-forwarding chain.
2. Record current behavior on this tree and, if available, on stock UCBLogo
   6.x and the upstream tree, as a baseline. Do not treat any of them as
   correct. Record the current `tests/test.lg` pass count at the same time
   rather than relying on a remembered number.

Initial local evidence (2026-11-15): on Node v25.7.0,
`npm run test:logo:node` reported 75 passed, 0 failed, 0 errors; OOP was
skipped and `Tests.File.LiteralDribble` was reported as its known XFAIL. The
runner also emitted Node-only `localStorage.getItem` diagnostics after the
Logo suite completed; it did not grade them as a runtime crash.

Four direct-run probes captured the current `ERROR` values; they are cases
01-04 of [tests/didnt-output-probes.lg](../../tests/didnt-output-probes.lg),
whose full results are in the Phase 1 corpus table.

| Snippet shape | Current `ERROR` output | Observation |
| --- | --- | --- |
| `to directstop` with `stop`; `print directstop` | `[5 directstop didn't output to print [] []]` | `X` is the user procedure, not `stop`; `Y` is `print`; no user-procedure `Z` at top level. |
| `outputtaila -> outputtailb -> outputtailc`, each forwarding with `output`; final procedure stops; called by `print` | `[5 outputtailc didn't output to output outputtailb [output outputtailc]]` | `X` is innermost `outputtailc`; `Y` is `output`; `Z` is the forwarding procedure and its `output` expression. |
| `commandtaila -> commandtailb`, final procedure stops; called by `print` | `[5 commandtaila didn't output to print [] []]` | `commandtaila` falls off its end after calling `commandtailb` as a command; blaming `commandtaila` matches the working specification. |
| `print 1 + directstop` | `[5 directstop didn't output to + [] []]` | Infix `Y` is the operation `+`, not an operand-position description. |

Upstream baseline (2026-11-15): upstream `jrincayc/ucblogo-code` at
`f0ca245` (2026-10-16), built natively with
`./configure --disable-wx --disable-docs` in a separate clone and run with
`LOGOLIB` pointing at its `logolib`, produced output identical to this tree's
Node build for all 32 corpus cases. Attribution is unchanged since the 2007
initial import (`b539a1c`): every commit touching `didnt_output_name`,
`didnt_get_output`, `last_call`, or `DIDNT_OUTPUT` is that import, and
`changes.txt` has no related entries.

Oracle from the Program Logic Manual ([plm](../../plm#L340-L420), section
3.2a): `to foo output print 3 end` called by `show foo` should report
"print didn't output to output in foo", and `to foo print 3 end` called by
`show foo` should report "foo didn't output to show in baz". This agrees
with the working specification and settles that `X` may be a primitive.

## Phase 1: Failing-Case Corpus

Build a table-driven set of Logo snippets crossing:

| Axis | Values |
| --- | --- |
| Failing callee | user procedure ending in a command; `stop` in body; falls off end after `if`/`ifelse`/`repeat`/`foreach` with no output; primitive command used as operand; macro returning non-list or calling `stop`; `.maybeoutput` with no value |
| Position | direct operand; tail of `output`; tail of procedure body (tail call); non-tail in sequence; nested operand; infix operand; parenthesized variadic call; default-value expression |
| Consumer `Y` | primitive (`print`), user procedure, `output`, `apply`/`map`/`foreach` template, `run`/`runresult`/`catch`, `make`, `if` predicate |
| Depth | direct; one tail call; chain of 3 tail calls; tail call through `if`/`ifelse`/`run`/`catch` |
| Context | toplevel, inside procedure, inside `pause`, with `erract`/`catch "error` active, `erract` that runs code and returns no value (H6) |

Corpus results: [tests/didnt-output-probes.lg](../../tests/didnt-output-probes.lg),
run with `node ./ucblogo-node.js tests/didnt-output-probes.lg`. The file is a
direct-run script, not a suite file. Turtle primitives crash under Node, so
case 09 uses `ignore` instead of `fd`. "Spec" compares against the Phase 0
working specification; `?` marks rows that depend on an open sub-question.

| Case | Call | Current `ERROR` output | Spec |
| --- | --- | --- | --- |
| 05 | `a: output b`, `b: c`, `c: stop`; `print a` | `[5 fallchainb didn't output to output fallchaina [output fallchainb]]` | yes |
| 06 | falls off after `if "true [print 1]` | `[5 falloffif didn't output to print [] []]` | yes |
| 07 | falls off after `repeat 2 [print 1]` | `[5 falloffrepeat didn't output to print [] []]` | yes |
| 08 | body `foreach [1 2] [stop]` | `[5 falloffforeach didn't output to print [] []]` | yes |
| 08b | `print foreach [1 2] [stop]` | `[29 Macro returned [] instead of a list [] []]` | ? (macro) |
| 09 | `print ignore 3` | `[5 ignore didn't output to print [] []]` | yes |
| 10 | `show1 directstop` (user consumer) | `[5 directstop didn't output to show1 [] []]` | yes |
| 11 | `make "v directstop` | `[5 directstop didn't output to make [] []]` | yes |
| 12 | `if directstop [print 1]` | `[5 directstop didn't output to if [] []]` | yes |
| 13 | `print first directstop` | `[5 directstop didn't output to first [] []]` | yes |
| 14 | `print (sum 1 2 directstop)` | `[5 directstop didn't output to sum [] []]` | yes |
| 15 | `(defaultuser)` with default `[:x directstop]` | `[5 directstop didn't output to defaultuser [] []]` | yes |
| 16 | `print apply "directstop []` | `[5 apply didn't output to print [] []]` | ? (transparency) |
| 17 | `print apply [[x] [stop]] [1]` | `[29 Macro returned nothing instead of a list apply [stop]]` | no |
| 18 | `print map [directstop] [1 2]` | `[5 apply didn't output to combine map1 [output combine (apply :map.template firsts :template.lists) (map1 bfs :template.lists :template.number+1)]]` | no |
| 19 | `print run [directstop]` | `[5 directstop didn't output to print [] []]` | yes |
| 20 | `print runresult [directstop]` | no error (`[]`) | yes |
| 21 | `print catch "foo [directstop]` | `[5 catch didn't output to print [] []]` | ? (transparency) |
| 22 | `.macro macstop stop end`; `print macstop` | `[29 Macro returned nothing instead of a list macstop [stop]]` | ? (macro) |
| 23 | `.macro macword output "abc end`; `print macword` | `[29 Macro returned abc instead of a list macword [output "abc]]` | yes (not message 5) |
| 24 | `.maybeoutput directstop` in a procedure used by `print` | `[5 directstop didn't output to .maybeoutput maybeempty [.maybeoutput directstop]]` | no |
| 25 | `print output 3` in a procedure | `[5 output didn't output to print opinarg [print output 3]]` | yes |
| 26 | `print stop` in a procedure | `[5 stop didn't output to print stopinarg [print stop]]` | yes |
| 27 | `ifelse "true [output directstop] [output 1]` | `[5 directstop didn't output to output viaifelse [ifelse "true [output directstop] [output 1]]]` | yes |
| 28 | `output run [directstop]` | `[5 run didn't output to output viarun [output run [directstop]]]` | no (inconsistent with 19) |
| 29 | `output catch "x [directstop]` | `[5 catch didn't output to output viacatch [output catch "x [directstop]]]` | ? (transparency) |
| 30 | `print directstop` inside a procedure | `[5 directstop didn't output to print wrapper [print directstop]]` | yes |
| 31 | same, non-tail in a sequence | `[5 directstop didn't output to print seqcase [print directstop]]` | yes |

Context rows under Node (file-argument runs):

- A top-level error while loading the file argument is silent: the load
  stops and no message is printed, so printed-message rows cannot be checked
  under Node. `catch "error` rows work, and a later unrelated error reports
  its own `ERROR` list (no stale `didnt_output_name` observed).
- `erract [output 99]` printed neither `99` nor a message; `erract [noisy]`
  printed `noisy` once with no repeated message.
- `erract [pause]` prints "directstop didn't output to print" and
  "Pausing...", then loops on prompts with stdin at EOF; piping `co 5`
  re-enters the same error and pause. `pause`/`co`/`erract` rows (H5, H6)
  need the native build or the browser.

For each row record `ERROR` output (`[5 |X didn't output to Y| ufun line]`;
parse `X`/`Y` from item 2) and the printed message.

Harness constraints:

- `RunTests` in [tests/UnitTests.lg](../../tests/UnitTests.lg) runs every test
  inside `RUN`, `CATCH "Error`, and `FOREACH`, which are themselves H4
  contexts. Toplevel and printed-message rows need a separate direct run.
- Direct runs must pass a `.lg` file argument to `ucblogo-node.js`; piping
  commands to its stdin fails with `ReferenceError: document is not defined`.
  Do not use hyphens in procedure names; `-` parses as minus.
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
   innermost procedure in the value-forwarding chain: set at compound
   procedure entry when the procedure's value is wanted (the `OUTPUT_OK`
   cases) and left alone when a callee is invoked as a command, so `stop` or
   `output` in a body blames the containing procedure. Remove
   `didnt_output_name` and the `last_call` use in `DIDNT_OUTPUT`. This is
   constant size under tail calls.
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

  Baseline on the current Node build (2026-11-15), peak (`max`) from `NODES`
  after `gc`; current nodes returned to about 4,800 in every run:

  | Loop | N = 1M peak | N = 3M peak | Constant? |
  | --- | --- | --- | --- |
  | `output` self-tail countdown | 2,516,530 | 2,516,530 | yes |
  | command self-tail with `stop` | 2,516,530 | 2,516,530 | yes |
  | `output` through `ifelse` | 11,743,990 | 33,554,350 | no |
  | `output run [...]` | 2,516,530 | 2,516,530 | yes |
  | `output catch "x [...]` | 8,388,550 | 24,326,890 | no |
  | 3-procedure mutual `output` recursion | 2,516,530 | 2,516,530 | yes |
  | deep `output` chain ending in `stop` | 2,516,530 | 2,516,530 | yes; error `[5 deepfail didn't output to output deepfail [output deepfail :n - 1]]` |

  `ifelse` and `catch` paths already grow with depth, so the regression check
  is "no worse than baseline", not "constant", for those two rows.
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
  item 2 text in the corpus so the change is deliberate. Known consumers only
  forward or print item 2 ([logolib/cond](../../logolib/cond#L17),
  [logolib/popl](../../logolib/popl#L8), [logolib/pon](../../logolib/pon#L8),
  [csls/pascal](../../csls/pascal#L10)); none parse the names. The
  `logolib/Messages*` templates and the code tables in
  [docs/ucblogo.texi](../../docs/ucblogo.texi#L7574) and
  [usermanual](../../usermanual#L3912) need no change.
- The `STOP_PRIORITY` branch contains cases documented as "actually can
  happen", including one that raises no error at the failing site (H8); they
  reflect real unsolved attribution and must be enumerated, not deleted.
- The `DIDNT_OUTPUT` path mutates live `ufun`/`this_line` and is recoverable,
  so fixing attribution can change where execution resumes after `erract`.
- Behavior differs from other Logo implementations; do not conform to them
  without the Phase 0 decision.

## Deliverables

1. The Phase 0 working specification, its open sub-questions resolved, and
   Brian Harvey's confirmation.
2. Corpus and baseline table (current output vs. expected).
3. Root-cause classification from Phase 3.
4. Chosen design with the tail-call and GC invariants checked.
5. Implementation in small commits, each leaving the corpus at least as green
   as before.
