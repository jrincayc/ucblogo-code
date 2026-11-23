#!/bin/sh
# Usage: scripts/ci-run.sh {list|watch|failed|errors|log} [branch] [repo]
# Defaults: current branch, the push-run repo (origin). PR runs live on upstream.
# Quiet on success: `watch` prints nothing and exits 0 when the run passes; on failure
# it prints failing "job: step" lines and the first N (default 15) distinct errors.
set -eu
cmd=${1:-list}
branch=${2:-$(git rev-parse --abbrev-ref HEAD)}
repo=${3:-$(git remote get-url origin | sed -E 's#.*github.com[:/]##; s#\.git$##')}
n=${N:-15}

run=$(gh run list --branch "$branch" -R "$repo" --limit 1 --json databaseId,status -q '.[0] | "\(.databaseId) \(.status)"')
[ -n "$run" ] || { echo "no runs for $branch in $repo" >&2; exit 2; }
id=${run% *}
status=${run#* }

# Completed runs are immutable: cache their output in /tmp (CI_CACHE=0 disables).
cache=${TMPDIR:-/tmp}/ci-cache
cached() {
  key=$1; shift
  if [ "$status" != completed ] || [ "${CI_CACHE:-1}" = 0 ]; then
    "$@"
    return
  fi
  mkdir -p "$cache"
  f=$cache/$(echo "$repo" | tr / _)-$id-$key
  [ -s "$f" ] || { "$@" > "$f.tmp" && mv "$f.tmp" "$f"; }
  cat "$f"
}

# Drop the "job<TAB>step<TAB>timestamp " log prefix.
body() { awk -F'\t' 'NF>=3 { sub(/^[^ ]+ /, "", $3); print $3 }' | cut -c1-260; }

failing_steps() {
  cached jobs gh run view "$id" -R "$repo" --json jobs | jq -r \
    '.jobs[] | select(.conclusion=="failure") | .name as $j | .steps[] | select(.conclusion=="failure") | "\($j): \(.name)"'
}

errors() {
  cached log-failed gh run view "$id" -R "$repo" --log-failed | body |
    grep -E 'error:|##\[error\]|✖|ERR!|failed \(' | awk '!seen[$0]++' | head -n "$n" || true
}

case "$cmd" in
  list)
    gh run list --branch "$branch" -R "$repo" --limit "${N:-3}" \
      --json databaseId,conclusion,headSha,displayTitle \
      -q '.[] | [.databaseId, (if .conclusion == "" then "running" else .conclusion end), .headSha[0:7], .displayTitle] | @tsv'
    echo "HEAD	$(git rev-parse --short HEAD)"
    ;;
  watch)
    gh run watch "$id" -R "$repo" --exit-status >/dev/null 2>&1 && exit 0
    echo "run $id failed" >&2
    failing_steps
    errors
    exit 1
    ;;
  failed)
    failing_steps
    errors
    ;;
  errors)
    errors
    ;;
  log)
    out=${TMPDIR:-/tmp}/ci-$id.log
    cached log-failed gh run view "$id" -R "$repo" --log-failed > "$out"
    echo "$out"
    ;;
  *)
    echo "unknown command: $cmd" >&2
    exit 2
    ;;
esac
