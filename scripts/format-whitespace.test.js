'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort: max).

const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const test = require('node:test');

const {
    normalizeText,
    formatFile,
    formatPaths,
} = require('./format-whitespace.js');

function tempRoot(t) {
    const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ucblogo-format-'));
    t.after(() => fs.rmSync(root, { recursive: true, force: true }));
    return root;
}

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
