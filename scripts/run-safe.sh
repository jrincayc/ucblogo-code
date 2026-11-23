#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

if [[ $# -lt 1 ]]; then
    printf 'Usage: %s <script.sh> [args...]\n' "$0" >&2
    exit 2
fi

TARGET_SCRIPT="$1"
shift

if [[ ! -f "$TARGET_SCRIPT" ]]; then
    printf 'ERROR: Script not found: %s\n' "$TARGET_SCRIPT" >&2
    exit 1
fi

# 1. Fail closed if shellcheck or python3 is missing
for tool in shellcheck python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        printf 'BLOCKED BY RUN-SAFE: %s is required but not installed in PATH.\n' "$tool" >&2
        exit 1
    fi
done

# 2. Validate passed CLI arguments (reject unexpected multi-line payloads)
for arg in "$@"; do
    if [[ "$arg" == *$'\n'* ]]; then
        printf 'BLOCKED BY RUN-SAFE: Multi-line argument passed via CLI; pass via file or stdin instead.\n' >&2
        exit 1
    fi
done

# 3. Run ShellCheck AST + policy checks (skips heredoc bodies & quoted literals properly)
TARGET_PATH="$TARGET_SCRIPT" python3 - <<'EOF'
import json, os, subprocess, sys

target_path = os.environ["TARGET_PATH"]
with open(target_path, "r", encoding="utf-8") as f:
    code = f.read()

# 1. Run ShellCheck at -S style on the target script
res = subprocess.run(
    ["shellcheck", "-s", "bash", "-S", "style", "-e", "SC2016", "-f", "json", "--", target_path],
    text=True,
    capture_output=True,
)
if res.returncode != 0:
    try:
        for iss in json.loads(res.stdout):
            sys.stderr.write(f"SC{iss['code']} ({target_path}:{iss['line']}): {iss['message']}\n")
    except Exception:
        sys.stderr.write(res.stderr or res.stdout)
    sys.stderr.write("BLOCKED BY RUN-SAFE: Fix the ShellCheck quoting/style errors above.\n")
    sys.exit(1)

# 2. Detect real here-strings (<<<) via ShellCheck POSIX sh AST check
sh_res = subprocess.run(
    ["shellcheck", "-s", "sh", "-f", "json", "--", target_path],
    text=True,
    capture_output=True,
)
if sh_res.returncode != 0:
    try:
        for iss in json.loads(sh_res.stdout):
            if iss["code"] in (3003, 2039) and "here-string" in iss["message"].lower():
                sys.stderr.write(
                    f"BLOCKED BY RUN-SAFE ({target_path}:{iss['line']}): Here-strings (<<<) are forbidden. "
                    "Use printf '%s\\n' \"$var\" | cmd.\n"
                )
                sys.exit(1)
    except Exception:
        pass

# 3. Walk lines while properly skipping heredoc bodies and quoted string literals
in_heredoc = None
for idx, line in enumerate(code.splitlines(), start=1):
    if in_heredoc is not None:
        if line.strip() == in_heredoc:
            in_heredoc = None
        continue

    if line.lstrip().startswith("#"):
        continue

    in_sq = False
    in_dq = False
    esc = False
    unquoted = []
    i = 0
    n = len(line)

    while i < n:
        ch = line[i]
        if esc:
            esc = False
            unquoted.append(" ")
            i += 1
            continue
        if ch == "\\" and not in_sq:
            esc = True
            unquoted.append(" ")
            i += 1
            continue
        if ch == "'" and not in_dq:
            in_sq = not in_sq
            unquoted.append(" ")
            i += 1
            continue
        if ch == '"' and not in_sq:
            in_dq = not in_sq
            unquoted.append(" ")
            i += 1
            continue

        if not in_sq and not in_dq:
            if ch == "#":
                break
            if line[i : i + 2] == "<<" and line[i : i + 3] != "<<<":
                j = i + 2
                if j < n and line[j] == "-":
                    j += 1
                while j < n and line[j] in (" ", "\t"):
                    j += 1
                if j < n and line[j] == "'":
                    end_q = line.find("'", j + 1)
                    if end_q != -1:
                        in_heredoc = line[j + 1 : end_q]
                        unquoted.append(" ")
                        i = end_q + 1
                        continue
                sys.stderr.write(
                    f"BLOCKED BY RUN-SAFE ({target_path}:{idx}): Heredoc delimiter must be single-quoted (<<'EOF').\n"
                )
                sys.exit(1)
            unquoted.append(ch)
        else:
            unquoted.append(" ")
        i += 1

    clean_cmd = "".join(unquoted)
    segments = clean_cmd.replace(";", "\n").replace("|", "\n").replace("&", "\n").splitlines()
    keyword_boundaries = {"then", "do", "else", "elif", "if", "while", "until", "for", "case", "select"}
    for seg in segments:
        words = seg.split()
        command_start = True
        for word in words:
            if word in keyword_boundaries:
                command_start = True
                continue
            if command_start and word in ("ruby", "eval"):
                sys.stderr.write(
                    f"BLOCKED BY RUN-SAFE ({target_path}:{idx}): Prohibited command '{word}'. "
                    "Use yq, jq, or python3 - <<'EOF'.\n"
                )
                sys.exit(1)
            command_start = False
EOF

# 4. Auto-load emsdk if emcc is not in PATH
ensure_emsdk() {
    if command -v emcc >/dev/null 2>&1; then
        return 0
    fi

    local candidates=(
        "${EMSDK:-}/emsdk_env.sh"
        "$HOME/emsdk/emsdk_env.sh"
        "/opt/emsdk/emsdk_env.sh"
        "/usr/local/emsdk/emsdk_env.sh"
    )

    local env_script
    for env_script in "${candidates[@]}"; do
        if [[ -n "$env_script" && -f "$env_script" ]]; then
            # shellcheck source=/dev/null
            EMSDK_QUIET=1 source "$env_script" >/dev/null 2>&1
            return 0
        fi
    done

    printf 'ERROR: emcc not found in PATH and emsdk_env.sh could not be located.\n' >&2
    return 1
}

ensure_emsdk
exec bash -- "$TARGET_SCRIPT" "$@"
