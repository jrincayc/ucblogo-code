---
name: safe-shell-streams
description: "Use when composing shell commands, pipelines, redirections, command substitution, process substitution, here-documents, here-strings, or temporary files; route untrusted data through file descriptors instead of evaluating or interpolating command text."
---

# Safe Shell Streams and Substitution

Treat shell data and shell syntax as separate channels. Pipes, redirection, and file descriptors carry data as byte streams; shell parsing determines executable syntax. Never concatenate untrusted values into command strings for `eval`, `sh -c`, or later reparsing.

## Core Rules

- Prefer pipelines and redirection for large or binary data.
- Quote literal variable expansions when used as arguments, such as `"$path"`.
- Use `--` before positional data when the receiving command supports it.
- Enable `set -o pipefail` in Bash when every pipeline stage must succeed.
- Treat process substitution and pipes as interfaces, not sanitizers. The receiving command still must not evaluate stream content as code.
- Prefer NUL-delimited records for path lists when all participating commands support them.
- Avoid command substitution when output can be large, binary, or contain trailing newlines.
- Use `mktemp` when a seekable on-disk file is actually required.

## Pipes

A pipe connects the standard output (`stdout`) of one process to the standard input (`stdin`) of the next process through a kernel-managed buffer.

```bash
cmd1 | cmd2
```

Pass both `stdout` and `stderr` with Bash's combined form:

```bash
cmd1 |& cmd2
```

The explicit equivalent is:

```bash
cmd1 2>&1 | cmd2
```

Use `pipefail` when the shell must report failure from any pipeline component:

```bash
set -o pipefail
producer | transformer | consumer
```

## Redirection and File Descriptors

Standard descriptors are `0` for `stdin`, `1` for `stdout`, and `2` for `stderr`.

| Syntax | Behavior |
|---|---|
| `cmd > file` | Write `stdout` to `file`, replacing existing content |
| `cmd >> file` | Append `stdout` to `file` |
| `cmd < file` | Read `stdin` from `file` |
| `cmd 2> err.log` | Write `stderr` to `err.log` |
| `cmd &> all.log` | Write `stdout` and `stderr` to `all.log` in Bash |
| `cmd > all.log 2>&1` | Portable-order redirection of both streams to one file |
| `cmd > /dev/null 2>&1` | Discard both output streams |
| `cmd 3<> file` | Open `file` for reading and writing on descriptor `3` |

Redirection order matters. `cmd 2>&1 > file` sends `stderr` to the current `stdout`, then redirects only `stdout` to `file`; use `cmd > file 2>&1` to send both to the file.

## Command Substitution

Command substitution captures a command's `stdout` for use as a shell value or argument.

```bash
today=$(date)
printf '%s\n' "$today"
```

Prefer `$(...)` over legacy backticks because it nests clearly and is easier to read. Quoting the expansion preserves it as one argument.

Command substitution has important limits:

- Bash variables and command arguments cannot contain NUL bytes.
- Command substitution removes trailing newline characters.
- Large output consumes shell memory and can exceed `ARG_MAX` when passed as arguments.
- Never feed untrusted substituted output back into an `eval`, shell parser, or executable command template.

For output intended to be processed as records, stream it directly or store it in a file instead of capturing it in a scalar variable.

## Process Substitution

Process substitution exposes a command's stream as a file descriptor path. It is useful for tools that require path arguments rather than `stdin`.

Read from generated paths:

```bash
diff <(sort -- file1) <(sort -- file2)
```

Write to multiple generated consumers:

```bash
tee >(grep 'ERROR' > err.log) >(grep 'WARN' > warn.log) < app.log
```

Process substitution is Bash-specific. It also does not propagate the substituted command's asynchronous exit status to the outer command; validate that status separately when correctness depends on it.

## Here-Documents and Here-Strings

A here-document sends a literal block to `stdin`. Quote the delimiter to disable parameter, command, and arithmetic expansion inside the body.

```bash
cat <<'EOF' > config.txt
This body is literal; shell expansions are disabled.
EOF
```

A here-string supplies one string plus a newline to `stdin`:

```bash
grep 'pattern' <<< "$value"
```

Here-documents and here-strings are convenient for modest in-memory input, but still pass their bytes to the receiving program. They are not command sanitizers.

## Explicit Temporary Files

Use `mktemp` for a unique file or directory when a process requires a real, seekable path or a stable lifetime.

```bash
tmpfile=$(mktemp)
trap 'rm -f -- "$tmpfile"' EXIT

cmd < input > "$tmpfile"
processor -- "$tmpfile"
```

Keep `tmpfile` trusted, quoted, and out of interpolation into shell source. For multiple related artifacts, prefer a unique temporary directory and remove only that owned directory.

## Safe Refactoring Patterns

### Unsafe evaluated string

Do not do this:

```bash
eval "grep $user_pattern $filename"
```

Pass the values as fixed arguments and use a supported option terminator:

```bash
grep -- "$user_pattern" "$filename"
```

### Unsafe large argument

Do not capture a large file merely to pass it back as one argument:

```bash
processor "$(cat -- huge-file)"
```

Stream or redirect the file:

```bash
processor < huge-file
```

If the processor requires a pathname, pass the pathname directly:

```bash
processor -- huge-file
```

### Unsafe word splitting

Do not use raw unquoted expansion:

```bash
rm $filename
```

Preserve the value as one argument:

```bash
rm -- "$filename"
```

If several paths must be transported, prefer a NUL-delimited stream:

```bash
find -- "$root" -type f -name '*.tmp' -print0 |
  xargs -0 -r -- rm --
```

## Why Streams Are Safer

1. **Command injection:** Data flowing through `stdin` is not automatically reparsed as shell syntax. Do not weaken that property with `eval`, `sh -c`, template engines, or command-string construction.
2. **Word splitting and globbing:** Quoted argument expansion avoids splitting on `IFS` and pathname expansion. Pipelines avoid carrying shell fields through a string representation.
3. **Arbitrary byte sequences:** Pipes and files can transport bytes, including embedded NUL bytes. Shell variables, command substitutions, and command arguments cannot preserve NUL bytes.
4. **Memory and argument limits:** Streams move data incrementally through bounded kernel buffers. Command substitution or oversized argument lists can consume shell/process memory or exceed `ARG_MAX`.

## Review Checklist

Before accepting shell code, verify that it:

- Never evaluates data as shell source.
- Quotes expansions used as arguments.
- Uses `--` where untrusted values could be interpreted as options.
- Streams large data instead of materializing it in arguments.
- Sets `pipefail` when partial pipeline failure must be detected.
- Uses NUL delimiters for arbitrary filenames when tools support them.
- Uses `mktemp` and scoped cleanup for required disk-backed temporaries.
- Keeps Bash-specific features, including `|&`, process substitution, and here-strings, out of shells that do not support them.

# Limit shell pipeline depth
- Very long pipelines are wasteful because if one command fails the whole thing fails and has to be restarted and this failure can and often does happen when it's almost finished.
- Break up long pipelines into a sequence of shorter ones and save intermediate results to temp files.
- This also permits spot checking intermediate results to detect defects in the pipeline or data.
- This is a judgement call so think about the amount of data and the risk and the length and complexity of the pipeline command and consider how to decompose it into smaller steps.
- Andon if you can't figure it out; that probably means we need a script.
<!-- Co-authored by GitHub Copilot. Scope: safe shell stream and substitution skill. Model: MAI Code 2; reasoning-effort setting unavailable. Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs. -->
