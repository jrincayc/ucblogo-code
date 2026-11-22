'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort unavailable).

const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const test = require('node:test');

const repo = path.resolve(__dirname, '..', '..');

function tempRepo(t) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ucblogo-accounting-hook-'));
  for (const command of [
    ['init', '--quiet'],
    ['config', 'user.name', 'Accounting Hook Test'],
    ['config', 'user.email', 'accounting-hook@example.invalid'],
  ]) {
    const result = spawnSync('git', ['-C', root, ...command], { encoding: 'utf8' });
    assert.equal(result.status, 0, result.stderr);
  }
  fs.mkdirSync(path.join(root, '.github', 'git-hooks'), { recursive: true });
  fs.mkdirSync(path.join(root, 'scripts'), { recursive: true });
  for (const name of ['pre-commit', 'commit-msg']) {
    fs.copyFileSync(path.join(repo, '.github', 'git-hooks', name), path.join(root, '.github', 'git-hooks', name));
  }
  fs.copyFileSync(path.join(repo, 'scripts', 'copilot-accounting.js'), path.join(root, 'scripts', 'copilot-accounting.js'));
  const hooks = spawnSync('git', ['-C', root, 'config', 'core.hooksPath', '.github/git-hooks'], { encoding: 'utf8' });
  assert.equal(hooks.status, 0, hooks.stderr);
  t.after(() => fs.rmSync(root, { recursive: true, force: true }));
  return root;
}

function git(root, args, options = {}) {
  const result = spawnSync('git', ['-C', root, ...args], {
    encoding: 'utf8',
    env: { ...process.env, ...(options.env || {}) },
  });
  if (options.allowFailure) return result;
  assert.equal(result.status, 0, result.stderr || result.stdout);
  return result;
}

test('normal commit and amend create exactly one accounting block', (t) => {
  const root = tempRepo(t);
  fs.writeFileSync(path.join(root, 'file.txt'), 'one\n');
  git(root, ['add', 'file.txt']);
  git(root, ['commit', '--quiet', '-m', 'normal commit']);

  let message = git(root, ['show', '-s', '--format=%B', 'HEAD']).stdout;
  assert.equal((message.match(/^COPILOT-ACCOUNTING-BEGIN v1$/gm) || []).length, 1);
  assert.equal((message.match(/^COPILOT-ACCOUNTING-END$/gm) || []).length, 1);

  git(root, ['commit', '--quiet', '--amend', '--no-edit']);
  message = git(root, ['show', '-s', '--format=%B', 'HEAD']).stdout;
  assert.equal((message.match(/^COPILOT-ACCOUNTING-BEGIN v1$/gm) || []).length, 1);
  assert.equal((message.match(/^COPILOT-ACCOUNTING-END$/gm) || []).length, 1);
});

test('merge commit receives one accounting block', (t) => {
  const root = tempRepo(t);
  fs.writeFileSync(path.join(root, 'base.txt'), 'base\n');
  git(root, ['add', 'base.txt']);
  git(root, ['commit', '--quiet', '-m', 'base']);

  git(root, ['switch', '--quiet', '-c', 'feature']);
  fs.writeFileSync(path.join(root, 'feature.txt'), 'feature\n');
  git(root, ['add', 'feature.txt']);
  git(root, ['commit', '--quiet', '-m', 'feature']);

  git(root, ['switch', '--quiet', '-']);
  git(root, ['merge', '--quiet', '--no-ff', 'feature', '-m', 'merge feature']);
  const message = git(root, ['show', '-s', '--format=%B', 'HEAD']).stdout;
  assert.equal((message.match(/^COPILOT-ACCOUNTING-BEGIN v1$/gm) || []).length, 1);
  assert.equal((message.match(/^COPILOT-ACCOUNTING-END$/gm) || []).length, 1);
});

test('malformed file-backed candidate blocks commit safely', (t) => {
  const root = tempRepo(t);
  fs.writeFileSync(path.join(root, 'file.txt'), 'one\n');
  git(root, ['add', 'file.txt']);
  const candidate = path.join(root, 'malformed.json');
  fs.writeFileSync(candidate, '{bad json', 'utf8');
  const result = git(root, ['commit', '--quiet', '-m', 'blocked'], {
    allowFailure: true,
    env: { COPILOT_ACCOUNTING_CANDIDATE: candidate },
  });
  assert.notEqual(result.status, 0);
  assert.match(result.stderr, /Copilot accounting pre-commit failed|Copilot accounting commit-msg failed/);
});

test('dry-run mode does not mutate commit messages', (t) => {
  const root = tempRepo(t);
  fs.writeFileSync(path.join(root, 'file.txt'), 'dry run\n');
  git(root, ['add', 'file.txt']);
  git(root, ['commit', '--quiet', '-m', 'dry run commit'], {
    env: { COPILOT_ACCOUNTING_DRY_RUN: '1' },
  });
  const message = git(root, ['show', '-s', '--format=%B', 'HEAD']).stdout;
  assert.doesNotMatch(message, /^COPILOT-ACCOUNTING-BEGIN v1$/m);
});
