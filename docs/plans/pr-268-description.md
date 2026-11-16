## Web Edition (WASM): impact on the wx and Unix terminal builds

Base is `f0ca245`, the current `master` tip. The Web Edition itself is described in `webedition.md`. This description covers only what the branch changes for the legacy builds and how that was checked.

### Legacy files touched

Fourteen shared C files that native builds compile have changed: `coms.c eval.c files.c graphics.c init.c intern.c lists.c logodata.c main.c math.c mem.c parse.c print.c wrksp.c`, plus `term.c`, `logo.h`, `globals.h`, `nographics.c` and `makehelp.c`. No C++ files, wx sources, `logolib/` or `csls/` are modified. The only data-file change is the new `helpfiles/setdarkmode`.

**Compiled out on native builds (preprocessor-guarded):**
- All `__EMSCRIPTEN__` blocks, including the web-only primitives (`runjs`, `animationstart`, `animationstop`, `italic`, `underline`, `deviceorientation`) and the UI event queue in `eval.c`.
- `YIELD` expands to `((void)0)`, so the roughly 20 yield points added to `eval.c`, `print.c` and `graphics.c` vanish.
- `POISON_NODE` and `UNPOISON_NODE` are no-ops without ASan.
- `SEG_SIZE` stays 16000 natively. The larger value is Emscripten-only.
- `EMSCRIPTEN_KEEPALIVE` is empty natively.
- `SERIALIZE_OBJECTS` is off, so `NODE` layout is unchanged.

**Compiled into native builds (please read these):**

| Area | Change | Native effect |
|---|---|---|
| `term.c` | Native terminal backend restored. The web terminal moved to `webterm.c`. `Makefile.am` links exactly one terminal backend. | Terminal-only builds use `term.c`. wx still uses `wxterm.c`. |
| `print.c` | Ordinary word output goes through `html_print_char()`. | Escaping happens only for `stdout` when a browser DOM exists. Otherwise it is `print_char`, unchanged. |
| `logo.h`, `logodata.c`, `mem.c` | New `HTML_STRING` node type (`NT_HTML` bit) with a GC sweep case. | The type exists on every build. Output is not rendered as HTML on wx. |
| `init.c` | New primitive `htmlstring`, not guarded. | A new Logo word on all platforms. |
| `mem.c` | Stack scan moved into `mark_stack()` and `set_bottom_stack()`. `valid_pointer` uses `size_t` and drops `volatile`. | Same scan range and roots as before. Extra code applies only under ASan or Emscripten. |
| `eval.c` | `eval_driver` ignores `NIL`/`UNBOUND`. | Defensive guard only. |
| `parse.c`, `files.c` | Input goes through a `sysGetC` indirection (`getc` natively, the wx reader under wx). A null prompt is tolerated and stdout is flushed after a prompt. | Same behaviour. |
| `main.c` | Added `--version`/`-version`. `bottom_stack` is set via `set_bottom_stack()`. | New flag. |
| Small fixes | `strnzcpy` bound, `makehelp.c` buffers (100 to 1024 bytes), `srandom` cast, `nop` prototype, unused variable in `lists.c`. | Warning and overflow fixes. |
| Build system | `configure.ac` and `Makefile.am`. | See open items. |
| `tests/UnitTests.lg` | Loads suites as `tests/UnitTests-*.lg`. It skips OOP when `kindof` is absent, tracks failures, and prints the error text. | Run it from the repo root. |

### How the risk was retired

CI (`.github/workflows/ci.yaml`) triggers on PRs to `master` and runs three native jobs. Each grades every individual result and fails on a missing report.
1. **wx** (`--enable-wx --enable-objects --enable-gitid`, under Xvfb):
   - It checks that `HAVE_WX` was actually selected.
   - It runs the full Logo unit suite.
   - It runs `tests/native/wx_smoke.py`, which has four scenes and six stages compared pixel-by-pixel against approved screenshots.
2. **Terminal with objects** (`--disable-wx --disable-x11 --enable-objects`):
   - It runs the full Logo unit suite.
   - It checks that literal terminal output has no HTML entities.
   - It runs `tests/native/pty_terminal.py`, which has 13 checks:
     - termios restored after char-mode reads and on normal exit
     - Ctrl-C returns to the prompt
     - xterm `cleartext`, `setcursor` and `setmargins` sequences
     - `TERM=dumb` and unset
     - redirected stdout and EOF
3. **Terminal without objects** (`--disable-objects`): builds and runs the full suite, checking the object-free default.

Local results (GCC 13.3, `-O2`, which is CI's native default):

| Build | Result |
|---|---|
| wx, objects | 105/105 Ok, including both MemMgr tests |
| terminal, objects | 105/105 Ok |
| wx default | 66/66 (earlier run on the same profile) |
| terminal default | 66/66 (earlier run on the same profile) |
| PTY harness | 13 checks, 0 failed |

- A 100,000-item allocation and explicit-GC stress run passed on the terminal build. The wx run was inconclusive because its capture contained no count.
- The maintainer manually exercised wx on a real desktop with `doctor`, `basic`, `mines`, `dotgame` and `cleartext`.
- **Failures that also occur on upstream `master`:**
  - Ctrl-C during `forever [make "x 1]` crashes the terminal build with SIGSEGV in about 5-10% of runs: 6/60 on this branch, 3/60 on `master` `3170933`. This is #265. The PTY harness retries that one check.
  - The original wx smoke handshake failed identically on `master` and on this branch, before the harness was fixed. Reports are in `reports/wx-upstream-comparison/`.

### Not covered

- macOS, Windows, BSD, and a native X11 run on the final code. X11 build and link passed before the last HTML-string test was added.
- A run of the master-only wx test list. The attempt timed out without a report, so the 105-test superset result stands in for it.
- The smoke baselines were generated on this branch, so they guard against future drift but do not prove pixel parity with `master`. The corrected smoke driver has not been rechecked on `master`.
- wx renders HTML-string content as literal markup. Only retention of the type is tested.
- `tests/Makefile.am` `EXTRA_DIST` does not yet list `UnitTests-Files.lg`, so `make dist` omits it.

### Open items

- `keyp` regression: the `#endif /* wx */` move in `files.c` `lkeyp` removes the file-stream path for wx and web.
- `term.c` is the pre-#213 file. It restores the WIN32 and sgtty branches that #213 removed, along with the termios path.
- The package version string is `6.2.5-WASM` for every build.
- `make` no longer runs `regenerate-help-index`, so run it explicitly (CI does).
- `--enable-gitid` now uses `git describe --tags --always --dirty`.
