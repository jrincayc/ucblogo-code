#!/usr/bin/env node
'use strict';

const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const { git } = require('./copilot-accounting.js');

const root = git(['rev-parse', '--show-toplevel']);
const target = path.join(root, '.github', 'git-hooks');
if (!fs.existsSync(path.join(target, 'pre-commit')) || !fs.existsSync(path.join(target, 'commit-msg'))) throw new Error('managed accounting hooks are missing');
for (const name of ['pre-commit', 'commit-msg']) fs.chmodSync(path.join(target, name), 0o755);
const current = spawnSync('git', ['-C', root, 'config', '--get', 'core.hooksPath'], { encoding: 'utf8' });
if (current.status !== 0 && current.status !== 1) throw new Error((current.stderr || 'cannot read core.hooksPath').trim());
const existing = (current.stdout || '').trim();
git(['-C', root, 'config', 'core.hooksPath', '.github/git-hooks']);
process.stdout.write(existing && existing !== '.github/git-hooks'
  ? `Configured .github/git-hooks (replaced local core.hooksPath ${existing}).\n`
  : 'Configured .github/git-hooks.\n');
