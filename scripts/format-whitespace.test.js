'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort: max).

const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const test = require('node:test');

const {
    normalizeText,
    formatFile,
    formatPaths,
} = require('./format-whitespace.js');

const scriptPath = path.resolve(__dirname, 'format-whitespace.js');

function tempRoot(t) {
    const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ucblogo-format-'));
    t.after(() => fs.rmSync(root, { recursive: true, force: true }));
    return root;
}

function tempGitRoot(t) {
    const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ucblogo-format-cli-'));
    const git = spawnSync('git', ['-C', root, 'init', '--quiet'], { encoding: 'utf8' });
    assert.equal(git.status, 0, git.stderr);
    t.after(() => fs.rmSync(root, { recursive: true, force: true }));
    return root;
}

function runCli(root, args) {
    return spawnSync(process.execPath, [scriptPath, ...args], {
        cwd: root,
        encoding: 'utf8',
    });
}

test('CLI rejects a bare invocation', (t) => {
    const root = tempGitRoot(t);
    const result = runCli(root, []);

    assert.equal(result.status, 2);
    assert.match(result.stderr, /^Usage: format-whitespace\.js/);
});

test('CLI --changed succeeds when no paths are changed', (t) => {
    const root = tempGitRoot(t);

    const check = runCli(root, ['--check', '--changed']);
    assert.equal(check.status, 0, check.stderr);
    assert.equal(check.stdout.trim(), 'Whitespace check passed.');

    const format = runCli(root, ['--changed']);
    assert.equal(format.status, 0, format.stderr);
    assert.equal(format.stdout.trim(), 'No whitespace changes needed.');
});

test('CLI check and format modes handle changed Markdown files', (t) => {
    const root = tempGitRoot(t);
    const file = path.join(root, 'changed.md');
    fs.writeFileSync(file, 'Needs newline', 'utf8');

    const check = runCli(root, ['--check', '--changed']);
    assert.equal(check.status, 1, check.stderr);
    assert.match(check.stdout, /Would change: changed\.md/);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Needs newline');

    const format = runCli(root, ['--changed']);
    assert.equal(format.status, 0, format.stderr);
    assert.match(format.stdout, /Formatted: changed\.md/);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Needs newline\n');
});

test('CLI explicit-file paths preserve check and format behavior', (t) => {
    const root = tempGitRoot(t);
    const file = path.join(root, 'explicit.md');
    fs.writeFileSync(file, 'Needs newline', 'utf8');

    const check = runCli(root, ['--check', file]);
    assert.equal(check.status, 1, check.stderr);
    assert.match(check.stdout, /Would change:/);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Needs newline');

    const format = runCli(root, [file]);
    assert.equal(format.status, 0, format.stderr);
    assert.match(format.stdout, /Formatted:/);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Needs newline\n');
});

test('normalizes CRLF, trailing whitespace, terminal blanks, and final newline', () => {
    const input = 'First\t  \r\nSecond   \r\nThird\r\n\r\n\r\n';
    assert.equal(normalizeText(input, true), 'First\nSecond\nThird\n');
});

test('preserves exactly two spaces only for Markdown hard breaks', () => {
    assert.equal(normalizeText('First  \nSecond\n', true), 'First  \nSecond\n');
    assert.equal(normalizeText('First   \nSecond\n', true), 'First\nSecond\n');
    assert.equal(normalizeText('Final  ', true), 'Final\n');
    assert.equal(normalizeText('First  \nSecond\n', false), 'First\nSecond\n');
});

test('preserves embedded blank lines but removes trailing blank lines', () => {
    assert.equal(normalizeText('A\n\nB\n\n  \n', true), 'A\n\nB\n');
});

test('an empty or whitespace-only file becomes empty', () => {
    assert.equal(normalizeText('', true), '');
    assert.equal(normalizeText(' \t\r\n  \r\n', true), '');
});

test('formats a file and is byte-for-byte idempotent', (t) => {
    const root = tempRoot(t);
    const file = path.join(root, 'doc.md');
    fs.writeFileSync(file, 'Heading  \r\nBody\t\r\n\r\n', 'utf8');

    const first = formatFile(file, { root });
    assert.equal(first.status, 'formatted');
    assert.equal(fs.readFileSync(file, 'utf8'), 'Heading  \nBody\n');

    const second = formatFile(file, { root });
    assert.equal(second.status, 'unchanged');
    assert.equal(fs.readFileSync(file, 'utf8'), 'Heading  \nBody\n');
});

test('check mode reports a change without rewriting', (t) => {
    const root = tempRoot(t);
    const file = path.join(root, 'check.txt');
    fs.writeFileSync(file, 'Needs newline', 'utf8');

    const results = formatPaths([file], { root, check: true });
    assert.equal(results[0].status, 'would-change');
    assert.equal(fs.readFileSync(file, 'utf8'), 'Needs newline');
});

test('unsupported and binary files are skipped', (t) => {
    const root = tempRoot(t);
    const source = path.join(root, 'source.js');
    const binary = path.join(root, 'binary.md');
    fs.writeFileSync(source, 'const x = "  ";\n', 'utf8');
    fs.writeFileSync(binary, Buffer.from([0x41, 0x00, 0x42, 0x0a]), 'binary');

    const results = formatPaths([source, binary], { root });
    assert.equal(results[0].status, 'skipped');
    assert.equal(results[0].reason, 'unsupported-extension');
    assert.equal(results[1].status, 'skipped');
    assert.equal(results[1].reason, 'binary-or-invalid-utf8');
});

test('outside-workspace and symlink escapes are skipped', (t) => {
    const root = tempRoot(t);
    const outsideRoot = tempRoot(t);
    const outside = path.join(outsideRoot, 'outside.md');
    fs.writeFileSync(outside, 'Outside  \n', 'utf8');

    const direct = formatFile(outside, { root });
    assert.equal(direct.status, 'skipped');
    assert.equal(direct.reason, 'outside-workspace');

    const escape = path.join(root, 'escape.md');
    fs.symlinkSync(outside, escape);
    const linked = formatFile(escape, { root });
    assert.equal(linked.status, 'skipped');
    assert.equal(linked.reason, 'outside-workspace');
    assert.equal(fs.readFileSync(outside, 'utf8'), 'Outside  \n');
});

test('duplicate and invalid UTF-8 paths are handled deterministically', (t) => {
    const root = tempRoot(t);
    const invalid = path.join(root, 'invalid.txt');
    fs.writeFileSync(invalid, Buffer.from([0xff, 0xfe, 0x0a]));

    const results = formatPaths([invalid, invalid], { root });
    assert.equal(results.length, 1);
    assert.equal(results[0].status, 'skipped');
    assert.equal(results[0].reason, 'binary-or-invalid-utf8');
});
