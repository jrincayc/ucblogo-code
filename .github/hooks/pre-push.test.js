'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const test = require('node:test');

const hook = path.resolve(__dirname, '..', 'git-hooks', 'pre-push');

function tempEnvironment(t) {
  const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ucblogo-pre-push-'));
  const bin = path.join(root, 'bin');
  fs.mkdirSync(bin);
  const gh = path.join(bin, 'gh');
  t.after(() => fs.rmSync(root, { recursive: true, force: true }));
  return { root, bin, gh };
}

function runHook(env, answer = '') {
  const refs = 'refs/heads/wasm 1111111111111111111111111111111111111111 refs/heads/wasm 2222222222222222222222222222222222222222\n';
  return spawnSync(hook, ['origin', 'https://github.com/jrincayc/ucblogo-code.git'], {
    encoding: 'utf8',
    input: refs,
    env: { ...process.env, PATH: `${env.bin}:${process.env.PATH}`, UCBLOGO_PRE_PUSH_NO_TTY: '1', ...env.mock },
  });
}

test('passing CI allows push', (t) => {
  const env = tempEnvironment(t);
  fs.writeFileSync(env.gh, '#!/usr/bin/env sh\nprintf \'[{"databaseId":1,"status":"completed","conclusion":"success","url":"https://example.invalid/run/1","headSha":"1111111"}]\'\n', { mode: 0o755 });
  const result = runHook(env);
  assert.equal(result.status, 0, result.stderr);
});

test('failing CI cancels push without a tty answer', (t) => {
  const env = tempEnvironment(t);
  fs.writeFileSync(env.gh, '#!/usr/bin/env sh\nprintf \'[{"databaseId":2,"status":"completed","conclusion":"failure","url":"https://example.invalid/run/2","headSha":"1111111"}]\'\n', { mode: 0o755 });
  const result = runHook(env);
  assert.notEqual(result.status, 0);
  assert.match(result.stderr, /latest CI run.*is failing/i);
  assert.match(result.stderr, /Push cancelled/i);
});

test('missing CI lookup warns and allows push', (t) => {
  const env = tempEnvironment(t);
  fs.writeFileSync(env.gh, '#!/usr/bin/env sh\nexit 1\n', { mode: 0o755 });
  const result = runHook(env);
  assert.equal(result.status, 0, result.stderr);
  assert.match(result.stderr, /could not check current CI/i);
});

test('deleted ref does not trigger a CI lookup', (t) => {
  const env = tempEnvironment(t);
  fs.writeFileSync(env.gh, '#!/usr/bin/env sh\nexit 99\n', { mode: 0o755 });
  const refs = 'refs/heads/wasm 0000000000000000000000000000000000000000 refs/heads/wasm 2222222222222222222222222222222222222222\n';
  const result = spawnSync(hook, ['origin', 'https://github.com/jrincayc/ucblogo-code.git'], {
    encoding: 'utf8',
    input: refs,
    env: { ...process.env, PATH: `${env.bin}:${process.env.PATH}` },
  });
  assert.equal(result.status, 0, result.stderr);
});
