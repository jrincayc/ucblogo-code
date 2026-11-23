#!/usr/bin/env bash
set -euo pipefail
IFS=$'\n\t'

require_tools() {
    local tool
    for tool in shellcheck python3; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            printf 'BLOCKED BY REPO GATE: %s is required but not installed in PATH.\n' "$tool" >&2
            exit 1
        fi
    done
}

validate_command_string() {
    local cmd="$1"

    require_tools

    CMD_STR="$cmd" python3 - <<'EOF'
import json, os, subprocess, sys

code = os.environ["CMD_STR"]

# 1. Run ShellCheck's real Bash AST parser (blocks SC2086 unquoted vars, syntax errors, etc.)
res = subprocess.run(
    ["shellcheck", "-s", "bash", "-S", "style", "-e", "SC2016", "-f", "json", "-"],
    input=code,
    text=True,
    capture_output=True,
)
if res.returncode != 0:
    try:
        for iss in json.loads(res.stdout):
            sys.stderr.write(f"SC{iss['code']} (line {iss['line']}): {iss['message']}\n")
    except Exception:
        sys.stderr.write(res.stderr or res.stdout)
    sys.stderr.write("BLOCKED BY REPO GATE: Fix the ShellCheck quoting/style errors above and retry.\n")
    sys.exit(1)

# 2. Use ShellCheck POSIX sh AST check to detect real here-strings (<<< = SC3003 / SC2039)
sh_res = subprocess.run(
    ["shellcheck", "-s", "sh", "-f", "json", "-"],
    input=code,
    text=True,
    capture_output=True,
)
if sh_res.returncode != 0:
    try:
        for iss in json.loads(sh_res.stdout):
            if iss["code"] in (3003, 2039) and "here-string" in iss["message"].lower():
                sys.stderr.write(
                    f"BLOCKED BY REPO GATE (line {iss['line']}): Here-strings (<<<) are forbidden. "
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
            # Detect heredoc operator << (excluding <<< which was already caught above)
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
                    f"BLOCKED BY REPO GATE (line {idx}): Heredoc delimiter must be single-quoted (<<'EOF').\n"
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
                    f"BLOCKED BY REPO GATE (line {idx}): Prohibited command '{word}'. "
                    "Use yq, jq, or python3 - <<'EOF'.\n"
                )
                sys.exit(1)
            command_start = False
EOF
}

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
}

if [[ "${1:-}" == "-c" && -n "${2:-}" ]]; then
    validate_command_string "$2"
    ensure_emsdk
fi

exec /bin/bash "$@"
