# Project Plan: Offline Coding Agent For The Web Edition

Status: Proposed; implementation has not started.
Date: 2026-11-15

## Objective

Add an on-device coding agent to the Web Edition that writes, explains, and
repairs UCB Logo code using a small open-weight model (candidate:
[Qwen2.5-Coder-3B-Instruct-GGUF](https://huggingface.co/Qwen/Qwen2.5-Coder-3B-Instruct-GGUF))
post-trained on UCB Logo. It must:

- work offline on lower-end phones after a one-time explicit download;
- be 100% usable from the Logo command line, with no GUI beyond HTML-formatted
  terminal output;
- send no prompts, code, or telemetry off the device.

Companion plans: [PWA installability](pwa-installability.md) (offline storage
and service worker rules this plan depends on) and
[Web accessibility](web-accessibility.md) (streamed terminal output must meet
its conventions).

## Constraints

- No new UI: no dialogs, panels, buttons, settings pages. All control is Logo
  procedures; all output is terminal text, optionally HTML via `htmlstring`.
- The model and inference engine are never precached or embedded in the Wasm
  filesystem; they are fetched only by an explicit command. See Issue 3.
- Model output is untrusted data. It is displayed escaped, never executed
  without an explicit user command, and never interpolated into Logo or
  JavaScript source (use `runjs` bound args; see the
  [trust boundary](../../.github/skills/ucb-logo-programming/SKILL.md)).
- Do not break the Asyncify yield contract: Logo must yield at least every
  50 ms ([webedition.md](../../webedition.md#asyncify)). Inference runs in a
  Web Worker, never on the main thread.
- No C changes unless a spike proves the Logo/JS bridge cannot do the job.
  wxWidgets Edition is unaffected.
- Generated outputs (`ucblogo.html`, `sw.js`) are fixed at their sources, not by
  hand. CI never downloads the model.
- Desktop Safari and real iOS cannot be verified locally; report that gap and
  treat iPad as a human checkpoint.

## Issue 1: Model choice and size tiers

Problem: "lower-end phone" and "3B" conflict. A Q4_K_M 3B GGUF is about 2 GB of
weights plus KV cache (Qwen2.5-3B: 36 layers, 2 KV heads, head dim 128, about
36 KB per token in f16, so about 150 MB at 4096 tokens). Wasm has no `mmap`, so
weights are copied into the Wasm heap; browsers on 3-4 GB phones will kill the
tab. Throughput on phone CPUs through Wasm is likely a few tokens per second
for 3B. These are estimates to be measured in Spike C.

Plan:
1. Train the same dataset into three sizes and ship tiers selected by
   `agent.install`:
   - `"small`: Qwen2.5-Coder-0.5B-Instruct (about 0.4 GB), very low-end only.
   - `"medium` (default): Qwen2.5-Coder-1.5B-Instruct (about 1.1 GB).
   - `"large`: Qwen2.5-Coder-3B-Instruct (about 2 GB), opt-in, 6 GB+ devices.
2. Expect a Logo-tuned 1.5B to beat an untuned 3B on Logo; the eval harness
   (Issue 6) decides the default, not assumption.
3. Check licenses before committing. As I recall, the 0.5B/1.5B Qwen2.5-Coder
   models are Apache 2.0 but the 3B uses the Qwen Research License, which
   restricts commercial use. Verify on each model card; if that is unacceptable
   for 3B, use Qwen3-4B or SmolLM3-3B (both Apache 2.0) as the large tier.
4. Quantization: Q4_K_M by default; Q4_0 is worth testing for ARM speed. Compute
   an importance matrix (`llama-imatrix`) over Logo text so low-bit rounding
   preserves Logo-specific tokens, then re-run the eval on the quantized file.

Validation: eval pass rate and tokens/second per tier on at least one real
low-end Android phone and the maintainer's iPad.

## Issue 2: Inference engine

Problem: GGUF means llama.cpp. Options:

| Option | Fit | Concern |
| --- | --- | --- |
| [wllama](https://github.com/ngxson/wllama) (llama.cpp to Wasm, worker) | Runs GGUF as is, CPU, works on any phone | Multi-thread needs `SharedArrayBuffer` |
| Own llama.cpp Wasm build in this repo | Full control, pinned Emscripten | Large maintenance cost; second toolchain |
| WebLLM / MLC (WebGPU) | Fast on capable GPUs | Needs MLC-format weights, not GGUF; weak on low-end phones |
| llama.cpp WebGPU backend | Future | Not mature enough to depend on |

Plan:
1. Start with wllama (or its pinned llama.cpp Wasm build) in a dedicated
   Worker. Single-thread is the guaranteed baseline.
2. Multi-thread requires cross-origin isolation (COOP/COEP headers). GitHub
   Pages cannot set headers; the usual workaround is a service-worker header
   injector (`coi-serviceworker`), which would have to merge with the existing
   Workbox worker. Treat this as Spike D; ship single-thread if it risks the
   PWA or the Hugging Face CORS fetches (`COEP: require-corp` affects
   cross-origin requests).
3. Load the engine lazily: the first `agent*` call fetches `agent.js` and the
   engine Wasm, which are not in the Workbox precache list but are cached for
   offline use by the install step (Issue 3).
4. Keep WebGPU as a later optional backend behind the same JS interface.

Validation: Spike C measures tokens/second, peak memory, and tab survival on
real devices before Issue 4 starts.

## Issue 3: Model download, storage, and service-worker safety

Problem: GitHub Pages rejects files over 100 MB, so the model cannot ship with
the site. Also
[workbox-config.js](../../workbox-config.js) has a catch-all runtime route
(`NetworkFirst`, `/.*/`) that would try to cache a multi-GB model response in
Cache Storage.

Plan:
1. Host GGUF files on the Hugging Face Hub (CORS-enabled `resolve` URLs) in a
   repo owned by the project. Pin by commit hash, not `main`.
2. Exclude model and engine hosts from the Workbox runtime route (a `urlPattern`
   that rejects them, or a bypass route). Add a source test that asserts it.
3. Store in the Origin Private File System, not IDBFS `/d` (too large, and sync
   would copy it) and not Cache Storage. Download in ranged chunks with resume,
   verify SHA-256 against a manifest pinned in `agent.js`, and write
   incrementally so peak memory stays near one chunk.
4. Call `navigator.storage.persist()` and report the result. Safari evicts
   non-persistent script storage after inactivity unless the app is installed,
   so `agent.status` must say plainly when the model could be evicted.
5. Service-worker updates must never delete the model; it lives outside Cache
   Storage so Workbox cleanup cannot touch it. Add a test.
6. Show size, free quota, and connection type, and require confirmation, before
   downloading. `agent.install` is the only network access the agent performs.

Validation: acceptance test with a tiny fixture GGUF served locally: install,
reload, go offline, and run.

## Issue 4: Logo command-line surface

All commands are Logo procedures defined in a new `agentstartup.lg` loaded on
demand (kept out of the default workspace to avoid name clutter), with thin
`runjs` wrappers using bound args per
[webstartup.lg](../../webstartup.lg).

```logo
agent.install [tier]     ; download and verify the model, print progress lines
agent.status             ; installed tier, size, persistence, engine, context size
agent.remove             ; delete the model from OPFS
agent "|task text|       ; one-shot: print short plan and proposed code, store as last proposal
agent.show               ; reprint last proposal
agent.run                ; evaluate the last proposal after the lint check (explicit user act)
agent.save "name         ; define the proposal's procedures without running them
agent.explain "procname  ; explain a workspace procedure
agent.fix "procname      ; repair using the last error message
agent.cancel             ; abort generation
```

Decisions:
1. Names use dotted forms like existing `.`-prefixed Logo conventions; confirm
   no collision with primitives or `logolib` (Open Decision 3).
2. `agent` blocks the Logo prompt until generation ends, like `wait`, and the
   existing break key aborts it through `AbortSignal`. Streaming tokens print
   to the terminal. How a Logo procedure waits on a promise, given that `runjs`
   promise results are delivered through the getchar buffer
   ([pre.js](../../pre.js#L335)), is Spike A.
3. Never auto-run. The agent proposes; `agent.run` or `agent.save` acts.
4. Workspace context is gathered in Logo (`contents`, `text`, `po`) and passed to
   JS as bound args. The agent never reads files other than through Logo.
5. Output is HTML only through `htmlstring`, built from escaped text. Code
   blocks use `<pre>`; model text is never inserted as markup.
6. Streamed output is chunked by line or sentence so screen readers on the
   terminal live region are not flooded token by token (see
   [web-accessibility.md](web-accessibility.md)).

## Issue 5: Agent loop, context, and safety

Plan:
1. Context budget: 4096 tokens max on phones. Fixed system prompt of about 100
   tokens (fine-tuning removes the need for a long dialect cheat sheet; this is
   the biggest prefill saving), the task, and at most N lines of workspace
   context. Keep the system prompt identical across calls so its KV cache can
   be reused.
2. Retrieval: build a small lexical (BM25) index over
   [help/docs.json](../../help/docs.json) on first use and inject the top two or
   three primitive entries (about 300 tokens). No embedding model.
3. Output envelope: one short plan sentence then one fenced Logo block. If the
   engine supports grammar-constrained sampling, constrain the envelope (and
   balanced `[]`/`()`), but never the Logo body.
4. Static lint before display, no execution: bracket balance, `to`/`end`
   pairing, and unknown-word detection by tokenizing against the primitive list
   (`helpfiles/ALL_NAMES`), library names, and workspace names. Unknown words
   trigger at most two bounded repair rounds that feed the lint messages back
   to the model; remaining findings are printed as warnings.
5. Denylist: proposals containing `runjs`, file and directory primitives,
   `load`, `erasefile`, or `dribble` are shown but `agent.run` refuses them
   unless the user passes an explicit override argument.
6. Prompt injection: workspace text (comments, words, loaded files) is data in
   the prompt. Because the agent cannot act without a user command and the
   denylist applies, an injection can at worst produce bad advice.
7. Cancellation and limits: cap output tokens, wall-clock time, and repair
   rounds; free the context between calls so memory returns to baseline.

## Issue 6: Evaluation harness (build this before training)

Problem: without a measurement, neither prompting nor fine-tuning can be
judged, and a phone-sized model will otherwise look plausible while writing
wrong Logo.

Plan:
1. Task set of about 200 to 300 tasks across categories: arithmetic and words
   and lists, recursion, `foreach`/`map`/`filter`/`reduce` templates, `for` and
   `repeat`, turtle geometry, property lists and arrays, macros, error repair,
   and Web Edition words (`runjs`, `htmlstring`, SVG, speech), plus dedicated
   categories for infix precedence and unary minus, dynamic scope, and
   quoting and escaping.
2. Each task has a prompt and a hidden Logo check (`output` assertions in the
   style of [tests/UnitTests.lg](../../tests/UnitTests.lg)). Metric: pass@1 on
   execution, plus a lint-clean rate.
3. Execute candidates with the headless Node build
   (`node ./ucblogo-node.js`), one process per candidate, in a throwaway
   working directory with a timeout and no network, because model output can
   include file primitives. Prefer a container for bulk runs.
4. Split by task family, not by example, so the held-out set tests
   generalization. Keep the existing `tests/*.lg` suites out of training data.
5. Report baselines in order: stock model with plain prompt; stock with
   few-shot plus retrieval; tuned; tuned plus lint-repair; tuned and quantized.

## Issue 7: Post-training on UCB Logo (primer and recipe)

What this is and why: a small model has seen little UCB Logo. It mixes in
MSWLogo, LogoWriter, Python, and JavaScript habits (`print(x)`, `return`,
`else`, missing `end`, `:` vs `"` confusion). Post-training means continuing to
train the existing model on examples of correct UCB Logo so those habits are
replaced. For a model this size the practical method is supervised fine-tuning
(SFT) with LoRA, which trains a small adapter instead of all weights. It needs
one GPU, not a cluster.

### Step 1: Collect seed material

Use what the repo already has:
[helpfiles/](../../helpfiles) (per-primitive docs and examples),
[docs/ucblogo.texi](../../docs/ucblogo.texi), [logolib/](../../logolib),
[csls/](../../csls), [contrib/](../../contrib), `tests/*.lg`, the demos, and
[webedition.md](../../webedition.md). Check each source's license before
training and before publishing a dataset (CSLS and the user manual are
Berkeley/Harvey material; Rosetta Code Logo is CC BY-SA/GFDL; public GitHub
Logo is mostly other dialects). Keep a provenance field on every record.

### Step 2: Generate verified training data

A few thousand high-quality examples beat a large noisy set.

1. Use a stronger teacher model (one whose license allows training on its
   output, such as Qwen2.5-Coder-32B or a DeepSeek model) to write tasks and
   solutions, giving it the relevant help entries and a few real programs in
   context.
2. Run every solution in the Node build against a test (the teacher also writes
   the test). Keep only candidates that pass. Failed ones that the teacher can
   repair become "error then fix" examples. This execution filter is the key
   quality lever, and the repo is unusually well placed for it.
3. Cover the traps deliberately: `pr` vs `print`, `make "x` vs `:x`,
   `output`/`stop`, `if`/`ifelse`, `?` templates, `for [i 1 10 1]`, infix
   precedence, parenthesized vararg calls, procedure `to ... end` layout, dynamic
   scope, quoting and escaping (`"word`, `|...|`, backslash), and Web words.
   Fixtures come from the verified semantics TODO below, not from memory. Add examples of refusing or translating other dialects.
4. Format as chat JSONL using exactly the runtime prompt and the model's chat
   template (ChatML for Qwen), so training and inference match:
   ```json
   {"messages":[
     {"role":"system","content":"UCB Logo coding agent."},
     {"role":"user","content":"Define square :size and draw four rotated squares."},
     {"role":"assistant","content":"Four squares, each turned 90 degrees.\n```logo\nto square :size\n  repeat 4 [fd :size rt 90]\nend\nrepeat 4 [square 50 rt 90]\n```"}],
    "source":"synthetic","verified":true}
   ```
5. Include 10-20% general code or instruction data as replay so the model does
   not forget how to follow instructions.

### Step 3: Fine-tune (SFT with LoRA/QLoRA)

1. Tools: [Unsloth](https://github.com/unslothai/unsloth),
   [Axolotl](https://github.com/axolotl-ai-cloud/axolotl), or Hugging Face
   [TRL](https://github.com/huggingface/trl). The 0.5B/1.5B/3B sizes fit QLoRA on
   a free Colab or Kaggle T4, or a single rented 24 GB GPU.
2. Starting hyperparameters (adjust on eval): LoRA rank 16 to 64 on all linear
   layers, learning rate 1e-4 to 2e-4, 2 to 3 epochs, loss on assistant tokens
   only, packed sequences up to 2048.
3. Train the three sizes from the same data; evaluate each (Issue 6).

### Step 4: Optional improvement rounds

1. Rejection-sampling fine-tuning: sample 8 answers per task from the SFT model,
   keep only those that pass, add to data, retrain. No RL infrastructure.
2. Preference tuning (DPO in TRL) on pairs of passing vs failing or
   wrong-dialect answers.
3. Reinforcement learning with the Node test runner as the verifier (GRPO in
   TRL) is possible but needs more GPU time; defer until the above plateaus.
4. Distill the agent behavior too: include multi-turn lint-repair and
   error-fix traces so the model learns the loop format, not just one-shot code.

### Step 5: Export and ship

1. Merge the LoRA into the base weights, convert with llama.cpp
   `convert_hf_to_gguf.py`, quantize with `llama-quantize` using the Logo
   importance matrix (Issue 1), and rerun the full eval on the quantized file.
2. Publish GGUFs, the dataset, scripts, and a model card (license, training data
   provenance, eval numbers, known weaknesses) on Hugging Face.
3. Pin file hashes in `agent.js` and record the training commit and eval score
   per release.

### Where things live

- Eval tasks and runner: `tests/agent-eval/` in this repo (depends on
  `ucblogo-node.js`).
- Data-generation and training scripts: `training/agent/` (scripts and configs
  only).
- Datasets, adapters, and GGUFs: Hugging Face Hub, never git.

## Issue 8: Testing

1. Source tests: Workbox runtime route excludes model hosts, no model or engine
   in precache, manifest hashes present.
2. JS unit tests with a mock engine for command parsing, chunked output,
   lint, repair bounds, denylist, and escaping.
3. Logo tests for `agentstartup.lg` wrappers against the mock engine, runnable in
   the Node suite.
4. TestCafe acceptance with a tiny fixture GGUF served locally: install, offline
   run, cancel, remove. Order follows the repo's browser priority; no real
   model download in CI.
5. Manual device matrix (low-end Android Chrome, iPad Safari) for memory and
   speed, run as human checkpoints.

## Issue 9: Documentation

Document commands, tiers, storage and eviction behavior, privacy, and
safety/denylist in [webedition.md](../../webedition.md) and a help topic.
State clearly that suggestions can be wrong and that nothing runs without a
user command.

## TODO: Finish verifying UCB Logo semantics

The assistant drafting this plan has made mistakes with infix precedence,
dynamic scope, and quoting/escaping. The model will make the same mistakes
unless training data and eval tasks are verified against the interpreter, not
written from memory. An initial probe pass is recorded below; finish the
browser-target and remaining language checks before building the dataset
(Issue 7) and task set (Issue 6).

Investigate by running each case in the Node build and the browser build, and
record actual output:

1. Precedence and arity: verify infix precedence and associativity, comparisons,
   prefix calls with infix expressions (`sum 2 3 * 4`,
   `first [1 2 3] + 10`, `list 1 + 2 3`), unary minus spacing, and
   parenthesized vararg calls. The initial Node probe found that prefix-call
   boundaries matter: `sum 2 3 * 4` outputs `14`, while
   `first [1 2 3] + 10` errors because `+` is applied to the list; wrapping the
   selector, `(first [1 2 3]) + 10`, outputs `11`. `2 + 3 * 4` outputs `14`
   and `10 - 2 - 3` outputs `5`.
2. Dynamic scope: a callee sees and modifies the caller's `local`/input
   variables ([LOCAL](../../docs/ucblogo.texi#L4840) states this); `make` on a
   name with no local binding creates a global; `localmake`; input shadowing;
   `?` template slots and `:input` references inside `foreach`/`map`/`run`
   bodies; `thing`/`:"x`.
3. Quoting and escaping: after an initial quote, word delimiters are spaces,
   brackets, and parentheses; infix characters are not delimiters. Verify
   backslash and `|...|` forms, `;` comments and `~` continuation, quoted vs.
   list-literal contents (`[a "b :c]`), `parse`, `runparse`, `word`, and
   `count` on escaped text, and how `runjs` strings are quoted from Logo
   (`"|...|`) per the existing [runjs notes](../../webedition.md).
4. Continue checking Node and browser behavior with small probe files and
   capture errors explicitly (`catch "Error` followed by `error`). The earlier
   apparent cutoff was the Logo error `+ doesn't like [1 2 3] as input`, not
   stdin failure. The repo's `node tests/acceptance/run-logo-node.js` suite
   passed 75 tests with 0 failures (one known `LiteralDribble` XFAIL).

Outputs of this TODO:

- Initial WebAssembly/Node findings (2026-11-15): `10 - 2 - 3` outputs `5`;
   `minus 3 + 4` outputs `-7`; `-3 + 4` and `- 3 + 4` output `1`;
   `runresult [ - 3 + 4]` returns `1`; `print (- 3 + 4)` prints `1`;
   `3 - 4` outputs `-1`; `3 - -4` outputs `7`; and `2 * -3` outputs `-6`.
   Avoid `3 -4`, which prints `3` then errors on `-4`. Unparenthesized
   `print - 3 + 4` errors because of the prefix parsing context.
- Dynamic-scope probe: a callee reads a caller's local `x=5`, its `make "x 20`
   changes that binding, and the caller sees `20`; a global `x=99` remains
   unchanged. A callee also reads an input local to its caller.
- Quoting probe: `print "a+b` outputs `a+b`, `count "a+b` is `3`,
   `count parse "|a b|` is `1`, and `vbarredp` distinguishes the `+` in
   `|a+b|` from the plus quoted with a backslash. Backslash quoting does not
   retain the vertical-bar metadata through `parse`.
- The [UCB Logo programming skill style notes](../../.github/skills/ucb-logo-programming/references/logo-style.md)
   now include these verified WebAssembly/Node findings.

- Still to verify in the browser build: comparisons, `list`/vararg boundaries,
   `foreach`/`map` templates, `run`/`runparse` behavior, more complex quoted
   strings, and the Web Edition `runjs` quoting examples. The Node observations
   should not be presented as cross-target validation until these checks pass.

- A verified set of fixtures (expression, expected output, notes) under
  `tests/agent-eval/` that doubles as Logo unit tests and training seeds.
- Dedicated eval categories and training-data coverage for precedence,
  dynamic scope, and quoting, with adversarial examples that look like other
  dialects.

## Spikes (do first, in order)

- A. Prove a Logo procedure can block on a JS promise, stream output, and be
  aborted, using the existing `runjs`/getchar path (no C change if possible).
- B. Hand-measure stock Qwen2.5-Coder 0.5B/1.5B/3B Q4_K_M in the wllama demo on
  the target phones: load success, tokens/second, peak memory.
- C. Same under the real Web Edition page, to confirm no yield or memory
  regression to Logo.
- D. Cross-origin isolation feasibility with the existing service worker and
  Hugging Face fetches.

## Milestones

0. Complete the Logo semantics TODO above.
1. Spikes A to D; decide engine, threading, and default tier.
2. Eval harness and baselines on stock models (Issue 6).
3. Dataset v1, first SFT, eval (Issue 7).
4. Runtime MVP with the stock model: install, `agent`, `agent.show`,
   `agent.cancel`; storage and service-worker safety (Issues 2 to 4).
5. Tuned GGUF, retrieval, lint and repair, `agent.run`/`agent.save`/`explain`/`fix`
   (Issue 5).
6. Tests, docs, and the device matrix (Issues 8 and 9).
7. Opt-in release behind the explicit install command; no default-path change.

## Open Decisions

1. Default tier and whether 3B ships at all, pending the license check and the
   eval (Issue 1).
2. Single-thread only vs cross-origin-isolated multi-thread (Spike D).
3. Command names and whether the dotted prefix collides with anything.
4. Blocking vs asynchronous generation at the Logo prompt (Spike A).
5. Hosting org and repo for GGUFs and dataset, and dataset license.
6. Whether to fund a rented GPU for the later RL stage or stop at SFT plus
   rejection sampling.
