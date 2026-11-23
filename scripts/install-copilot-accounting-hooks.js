#!/usr/bin/env node
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');

function git(root, args) {
  const result = spawnSync('git', ['-C', root, ...args], { encoding: 'utf8' });
  if (result.status !== 0) throw new Error((result.stderr || result.stdout || 'git command failed').trim());
  return result.stdout.trim();
}

const currentDir = process.cwd();
let root;
try {
  root = git(currentDir, ['rev-parse', '--show-toplevel']);
} catch (error) {
  process.stderr.write(`Cannot locate repository: ${error.message}\n`);
  process.exitCode = 1;
}

if (root) {
  const target = path.join(root, '.github', 'git-hooks');
  const prePush = path.join(target, 'pre-push');
  if (!fs.existsSync(prePush)) {
    process.stderr.write('pre-push CI guard is missing\n');
    process.exitCode = 1;
  } else {
    fs.chmodSync(prePush, 0o755);

    for (const name of ['pre-commit', 'commit-msg']) {
      const hook = path.join(target, name);
      if (fs.existsSync(hook)) fs.chmodSync(hook, 0o755);
    }

    const accountingAvailable = ['pre-commit', 'commit-msg']
      .every((name) => fs.existsSync(path.join(target, name)));
    if (!accountingAvailable)
      process.stdout.write('Warning: managed Copilot accounting hooks are unavailable or incomplete.\n');

    const configured = spawnSync('git', ['-C', root, 'config', 'core.hooksPath', '.github/git-hooks'], { encoding: 'utf8' });
    if (configured.status !== 0) {
      process.stderr.write(`Cannot configure core.hooksPath: ${configured.stderr}\n`);
      process.exitCode = 1;
    } else {
      process.stdout.write('Configured .github/git-hooks (including pre-push CI warning).\n');
    }
  }
}
