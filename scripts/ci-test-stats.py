#!/usr/bin/env python3
"""Per-test failure rates across CI runs, for every job in a run.

Usage: scripts/ci-test-stats.py [-R REPO]... [--limit N] [--job REGEX] [--workflow FILE]
                               [--grep REGEX] [--all] [--runs] [--no-steps] [--refresh]

Sources, per run: test result lines in every job's log (TestCafe and node:test `✓`/`✔`/`✖`, Logo
suite `Tests.X... Ok`) and the pass/fail of every job step from `gh run view --json jobs`, so jobs
that print no per-test lines (native terminal, wx) still show build and grading failures as
"step: ..." rows. A test is a (job, name) pair; it fails in a run if any result for it failed.

Defaults: origin repo, workflow ci.yaml, last 30 runs, all jobs, only rows with a failure. Repeat -R
to compare repos (for example origin and jrincayc/ucblogo-code, whose runs are pull_request runs of
the same SHAs). --job filters jobs by regex.

Completed runs are immutable, so the raw full log (gzip) and the jobs JSON are cached in
${TMPDIR:-/tmp}/ci-cache, the directory scripts/ci-run.sh uses (it reads the same `-jobs` file);
parser changes never refetch. A first run over N uncached runs downloads N logs (about 100 KB
each); the count is printed first. --refresh refetches. Runs without results are counted
separately and excluded from the rates. Only the latest attempt of a run is seen.
"""
import argparse
import collections
import gzip
import json
import os
import re
import subprocess
import sys

RESULT = re.compile(r'^\s*(✓|✔|✖) (.+?)(?: \((?:\d|screenshots:).*)?$')
LOGO = re.compile(r'^([A-Za-z][\w.]*)\.\.\. +(Ok|Failed|Error)\b')
NOT_TESTS = {'failing tests:', 'failing tests'}


def gh(*args):
    done = subprocess.run(['gh', *args], capture_output=True, text=True, errors='replace')
    if done.returncode:
        sys.exit(f'gh {" ".join(args[:3])} failed: {done.stderr.strip()[:300]}')
    return done.stdout


def origin_repo():
    url = subprocess.run(['git', 'remote', 'get-url', 'origin'], capture_output=True, text=True).stdout
    return re.sub(r'\.git$', '', re.sub(r'.*github\.com[:/]', '', url.strip()))


def cache_path(cache_dir, repo, run_id, key):
    return os.path.join(cache_dir, f'{repo.replace("/", "_")}-{run_id}-{key}')


def fetch_cached(path, refresh, args, compress=False):
    opener = gzip.open if compress else open
    if not refresh and os.path.exists(path):
        with opener(path, 'rt', errors='replace') as f:
            return f.read()
    text = gh(*args)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with opener(path + '.tmp', 'wt') as f:
        f.write(text)
    os.replace(path + '.tmp', path)
    return text


def log_results(text):
    """{job: {test: passed}} from the result lines of every job in a full run log."""
    out = collections.defaultdict(dict)
    for line in text.splitlines():
        parts = line.split('\t', 2)
        if len(parts) < 3:
            continue
        content = re.sub(r'^\S+\s?', '', parts[2])
        match = RESULT.match(content)
        if match and match.group(2).strip() not in NOT_TESTS:
            name, passed = match.group(2).strip(), match.group(1) != '✖'
        else:
            match = LOGO.match(content)
            if not match:
                continue
            name, passed = match.group(1), match.group(2) == 'Ok'
        tests = out[parts[0]]
        tests[name] = tests.get(name, True) and passed
    return out


def step_results(jobs_json):
    """{job: {"step: name": passed}} for steps that ran to success or failure."""
    out = {}
    for job in json.loads(jobs_json).get('jobs', []):
        steps = out.setdefault(job['name'], {})
        for step in job.get('steps') or []:
            if step.get('conclusion') in ('success', 'failure'):
                steps[f'step: {step["name"]}'] = step['conclusion'] == 'success'
    return out


def short(job):
    return re.sub(r'^Build and Test ', '', job)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split('\n')[0])
    ap.add_argument('-R', '--repo', action='append', help='owner/name (repeatable; default origin)')
    ap.add_argument('--limit', type=int, default=30)
    ap.add_argument('--job', help='only jobs whose name matches this regex')
    ap.add_argument('--workflow', default='ci.yaml')
    ap.add_argument('--grep', help='only tests whose name matches this regex')
    ap.add_argument('--all', action='store_true', help='include tests that never failed')
    ap.add_argument('--runs', action='store_true', help='list each run with its failing tests')
    ap.add_argument('--no-steps', action='store_true', help='omit step results')
    ap.add_argument('--refresh', action='store_true', help='ignore the cache')
    args = ap.parse_args()
    repos = args.repo or [origin_repo()]
    cache_dir = os.path.join(os.environ.get('TMPDIR', '/tmp'), 'ci-cache')
    name_filter = re.compile(args.grep) if args.grep else None
    job_filter = re.compile(args.job) if args.job else None

    seen = collections.defaultdict(collections.Counter)    # (job, test) -> repo -> runs with a result
    failed = collections.defaultdict(collections.Counter)  # (job, test) -> repo -> runs with a failure
    run_rows = []
    for repo in repos:
        runs = json.loads(gh('run', 'list', '-R', repo, '--workflow', args.workflow, '--limit', str(args.limit),
                             '--json', 'databaseId,status,createdAt,headSha,event'))
        done = sorted((r for r in runs if r['status'] == 'completed'), key=lambda r: r['createdAt'])
        missing = sum(not os.path.exists(cache_path(cache_dir, repo, r['databaseId'], 'log.gz')) for r in done)
        if missing and not args.refresh:
            print(f'{repo}: downloading {missing} of {len(done)} logs (cached afterwards)', file=sys.stderr)
        with_tests = 0
        for run in done:
            run_id = str(run['databaseId'])
            log = fetch_cached(cache_path(cache_dir, repo, run_id, 'log.gz'), args.refresh,
                               ['run', 'view', run_id, '-R', repo, '--log'], compress=True)
            results = collections.defaultdict(dict)
            for job, tests in log_results(log).items():
                results[job].update(tests)
            if not args.no_steps:
                jobs = fetch_cached(cache_path(cache_dir, repo, run_id, 'jobs'), args.refresh,
                                    ['run', 'view', run_id, '-R', repo, '--json', 'jobs'])
                for job, steps in step_results(jobs).items():
                    results[job].update(steps)
            outcome = {(job, name): passed for job, tests in results.items()
                       if not job_filter or job_filter.search(job) for name, passed in tests.items()}
            if not outcome:
                continue
            with_tests += 1
            for key, passed in outcome.items():
                seen[key][repo] += 1
                if not passed:
                    failed[key][repo] += 1
            bad = [f'{short(job)}: {name}' for (job, name), passed in outcome.items() if not passed]
            run_rows.append((run['createdAt'][:16], repo, run_id, run['headSha'][:7], run['event'], bad))
        print(f'{repo}: {len(runs)} runs, {len(done)} completed, {with_tests} with results', file=sys.stderr)

    if args.runs:
        for when, repo, run_id, sha, event, bad in run_rows:
            print(f'{when} {repo} {run_id} {sha} {event:12} ' + (', '.join(n[:60] for n in bad) or '-'))
        print()
    print(f'{"failed/runs":>12} {"rate":>5}  {"job":22} test' +
          ''.join(f'  | {r.split("/")[0]}' for r in repos if len(repos) > 1))
    for key in sorted(seen, key=lambda k: (-sum(failed[k].values()) / sum(seen[k].values()), k)):
        job, name = key
        total_failed, total_seen = sum(failed[key].values()), sum(seen[key].values())
        if (not total_failed and not args.all) or (name_filter and not name_filter.search(name)):
            continue
        per_repo = ''.join(f'  | {failed[key][r]}/{seen[key][r]}' for r in repos) if len(repos) > 1 else ''
        print(f'{f"{total_failed}/{total_seen}":>12} {100 * total_failed / total_seen:4.0f}%  {short(job)[:22]:22} '
              f'{name[:80]}{per_repo}')


if __name__ == '__main__':
    main()
