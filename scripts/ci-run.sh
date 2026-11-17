#!/bin/sh
# Usage: scripts/ci-run.sh {list|watch|failed|errors} [branch] [repo]
# Defaults: current branch, the push-run repo (origin). PR runs live on upstream.
set -eu
cmd=${1:-list}
branch=${2:-$(git rev-parse --abbrev-ref HEAD)}
repo=${3:-$(git remote get-url origin | sed -E 's#.*github.com[:/]##; s#\.git$##')}
sha=$(git rev-parse HEAD)

latest_id() {
  gh run list --branch "$branch" -R "$repo" --limit 1 --json databaseId -q '.[0].databaseId'
}

case "$cmd" in
  list)
    gh run list --branch "$branch" -R "$repo" --limit 5 \
      --json databaseId,conclusion,event,headSha,displayTitle | cat
    echo "HEAD=$sha"
    ;;
  watch)
    gh run watch "$(latest_id)" -R "$repo" --exit-status
    ;;
  failed)
    gh run view "$(latest_id)" -R "$repo"
    gh run view "$(latest_id)" -R "$repo" --log-failed
    ;;
  errors)
    gh run view "$(latest_id)" -R "$repo" --log-failed | grep -n 'error\|Error' | cut -c1-300
    ;;
  *)
    echo "unknown command: $cmd" >&2
    exit 2
    ;;
esac
