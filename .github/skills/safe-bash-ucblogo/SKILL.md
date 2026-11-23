---
name: safe-bash-ucblogo
description: Strict, zero-interpolation Bash rules for AI agents building and testing UCBLogo (Legacy GCC and Emscripten emcc/emmake, Binaryen, Node, TestCafe). Enforces filename quoting, ShellCheck validation, and automatic emsdk loading.
---

# Safe Bash for UCBLogo (Legacy GCC + Emscripten Wasm)

**CRITICAL RULE:** Never embed source code (UCBLogo, C, WAT, JS, HTML, CSS, Python), YAML/JSON payloads, or multi-line strings inside Bash quotes (`"..."` or `'...'`). Always pass code and data via **files** or **unexpanded Heredocs (`<<'EOF'`)**.

---

## 1. Execution & Emscripten Auto-Loading

* **Always run multi-line scripts via `./scripts/run-safe.sh`:**
  ```bash
  ./scripts/run-safe.sh "scripts/my_task.sh"
  ```
  This wrapper automatically runs `shellcheck -S warning` (blocking unquoted variables) and sources `emsdk_env.sh` if `emcc` is not preloaded in `PATH`.
* **For direct one-off Emscripten commands** when `emcc` might not be preloaded:
  ```bash
  command -v emcc >/dev/null 2>&1 || EMSDK_QUIET=1 source "${EMSDK:-$HOME/emsdk}/emsdk_env.sh"
  emmake make -C "${WASM_BUILD_DIR}" -j"$(nproc)"
  ```

---

## 2. Mandatory Filename & Variable Quoting

Lightweight models frequently fail by leaving filenames or variables unquoted (`emcc $FILE`, `rm my file.wasm`). **Assume every filename contains spaces, parentheses, or leading hyphens (`-`).**

* **Quote EVERY variable expansion:** Write `"$file"`, `"${build_dir}/ucblogo.js"`, and `"${c_files[@]}"`. Never write bare `$file`.
* **Quote EVERY command substitution:** Write `OUT="$(cmd)"`. Never write `for f in $(find ...)`.
* **Use Bash arrays for file lists & compiler flags:** Never store multiple paths or `-s` flags in a plain string.
* **Prefix dynamic paths with `--`:** Prevents filenames starting with `-` from acting as flags (`rm -- "$file"`, `cp -- "$src" "$dst"`).

| Task | FORBIDDEN (Breaks on Spaces/Globbing) | REQUIRED (Safe for Any Filename) |
| :--- | :--- | :--- |
| **Compile with `emcc`** | `emcc $SRC -o $OUT $FLAGS` | `emcc -- "$SRC" -o "$OUT" "${FLAGS[@]}"` |
| **Build with `emmake`** | `emmake make -j$(nproc)` | `emmake make -j"$(nproc)"` |
| **Loop over `find`** | `for f in $(find src -name "*.c"); do` | `find src -name '*.c' -print0 \| while IFS= read -r -d '' f; do` |
| **List of source files** | `SRCS="a.c b.c"; gcc $SRCS` | `SRCS=("a.c" "b.c"); gcc -- "${SRCS[@]}"` |
| **Optimize Wasm** | `wasm-opt -O3 $IN_WASM -o $OUT_WASM` | `wasm-opt -O3 -- "$IN_WASM" -o "$OUT_WASM"` |

### Safe Emscripten Flag Arrays
Emscripten `-s` settings contain brackets and quotes that break when escaped inside strings. Always put them in a Bash array:
```bash
EM_FLAGS=(
    -O2
    '-sWASM=1'
    '-sMODULARIZE=1'
    '-sALLOW_MEMORY_GROWTH=1'
    '-sEXPORTED_FUNCTIONS=["_main","_eval_logo"]'
    '-sEXPORTED_RUNTIME_METHODS=["ccall","cwrap"]'
)
emcc -- "${C_SOURCES[@]}" "${EM_FLAGS[@]}" -o "${WASM_DIST_DIR}/ucblogo.js"
```

---

## 3. Zero-Interpolation Rule & Approved Tools

Do not concatenate strings or interpolate variables into `bash -c`, `python3 -c`, `node -e`, `ruby -e`, or `ucblogo`.

* **YAML / JSON:** Use `yq` (YAML) or `jq` (JSON). **NEVER call `ruby` to parse YAML.** If complex logic is needed, pipe a single-quoted Heredoc to `python3 - <<'EOF'` and pass paths via environment variables.
* **WebAssembly Inspection:** Use `wasm-validate`, `wasm2wat`, and `wasm-opt` directly on quoted `.wasm` files.
* **Writing Source/Test Files (`.lg`, `.c`, `.h`, `.wat`, `.js`, `.html`, `.css`):** Always use a single-quoted Heredoc (`cat <<'EOF' > "path/to/file"`).

```bash
# GOOD: Zero escaping needed for JS, HTML, WAT, or UCBLogo syntax
cat <<'EOF' > "tests/wasm_bridge_test.js"
import { Selector } from 'testcafe';

fixture('UCBLogo Wasm REPL')
    .page('http://localhost:8080/index.html');

test('Evaluates quoted words and nested brackets', async t => {
    const prompt = Selector('#logo-input');
    await t
        .typeText(prompt, 'print [hello "world [1 2]]')
        .pressKey('enter');
});
EOF

# GOOD: Parse YAML safely with yq
yq -r '.builds.emscripten.flags[]' -- "$CONFIG_FILE"
```

---

## 4. Dual-Target QA & TestCafe Workflows

### Legacy UCBLogo CLI Tests (Prevent Interactive Hangs)
UCBLogo hangs on `? ` prompts if a script omits `bye` or hits an error. Always wrap runs in `timeout` and append `bye` via stream:
```bash
TMP_DIR="$(mktemp -d)"
trap 'rm -rf -- "$TMP_DIR"' EXIT

{ cat -- "$TEST_FILE"; printf '\nbye\n'; } | timeout 10s ./build-legacy/ucblogo > "$TMP_DIR/actual.txt" 2>&1
```

### Emscripten WebAssembly + TestCafe Headless Runs
```bash
wasm-validate -- "${WASM_DIST_DIR}/ucblogo.wasm"

npx http-server -- "$WASM_DIST_DIR" -p 8080 -s &
SERVER_PID=$!
trap 'kill -- "$SERVER_PID" 2>/dev/null || true' EXIT

npx testcafe "chromium:headless" -- "$TESTCAFE_SPEC"
```

---

## 5. Security & Pre-Execution Audit

1. **Guard every `rm -rf` with `:?`:** `rm -rf -- "${BUILD_DIR:?}/"*`
2. **Use `grep -F --` for literal code searches:** Prevents Logo `[brackets]`, C `*pointers`, and WAT `(s-expressions)` from being parsed as regex or flags.
3. **Never use `eval` or backticks (`` `...` ``):** Always use `$(...)` and quote it (`RES="$(cmd)"`).
4. **Use `printf '%s\n'` instead of `echo`:** `echo "$VAR"` corrupts strings starting with `-n`/`-e` or containing backslashes (`\`).

Before outputting **any** shell tool call, write a 3-line `<bash_audit>` check:
```text
<bash_audit>
1. Quoted Paths/Vars: Every VAR and (...) is wrapped in "..." (e.g., "file", "{arr[@]}"). [YES/NO]
2. Zero Interpolation: Code/Logo/JS/WAT uses <<'EOF' (single-quoted); Emscripten flags use arrays; YAML/JSON uses yq/jq (no Ruby). [YES/NO]
3. Safe Toolchain/Cleanup: Dynamic paths follow --, rm -rf uses \${VAR:?}, UCBLogo has timeout + bye, servers use trap cleanup. [YES/NO]
</bash_audit>
```
