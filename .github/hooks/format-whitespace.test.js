'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort: max).

const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const test = require('node:test');

const hookPath = path.join(__dirname, 'format-whitespace.js');

function tempGitRoot(t) {
    const root = fs.mkdtempSync(path.join(os.tmpdir(), 'ucblogo-format-hook-'));
    const git = spawnSync('git', ['-C', root, 'init', '--quiet'], { encoding: 'utf8' });
    assert.equal(git.status, 0, git.stderr);
    t.after(() => fs.rmSync(root, { recursive: true, force: true }));
    return root;
}

function runHook(root, mode, input) {
    const result = spawnSync(process.execPath, [hookPath, mode], {
        cwd: root,
        env: { ...process.env, UCBLOGO_FORMAT_ROOT: root },
        input: JSON.stringify(input || {}),
        encoding: 'utf8',
    });
    assert.equal(result.status, 0, result.stderr);
    return result.stdout.trim() ? JSON.parse(result.stdout) : null;
}

test('PostToolUse formats a Markdown edit and succeeds without output', (t) => {
    const root = tempGitRoot(t);
    const file = path.join(root, 'post.md');
    fs.writeFileSync(file, 'Post edit  \r\nBody\t\r\n\r\n', 'utf8');

    const output = runHook(root, 'post', {
        tool_name: 'edit_file',
        tool_input: { filePath: file },
    });

    assert.equal(output, null);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Post edit  \nBody\n');
});

test('PostToolUse ignores read-only tools and unsupported files', (t) => {
    const root = tempGitRoot(t);
    const file = path.join(root, 'read.md');
    fs.writeFileSync(file, 'Do not change  \n', 'utf8');

    assert.equal(runHook(root, 'post', {
        tool_name: 'read_file',
        tool_input: { filePath: file },
    }), null);

    const source = path.join(root, 'source.js');
    fs.writeFileSync(source, 'const value = 1;   \n', 'utf8');
    assert.equal(runHook(root, 'post', {
        tool_name: 'edit_file',
        tool_input: { filePath: source },
    }), null);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Do not change  \n');
    assert.equal(fs.readFileSync(source, 'utf8'), 'const value = 1;   \n');
});

test('PostToolUse prefilters and skips a binary Markdown file without warning', (t) => {
    const root = tempGitRoot(t);
    const file = path.join(root, 'binary.md');
    const binary = Buffer.from([0x41, 0x00, 0x42]);
    fs.writeFileSync(file, binary, 'binary');

    const output = runHook(root, 'post', {
        tool_name: 'create_file',
        tool_input: { filePath: file },
    });

    assert.equal(output, null);
    assert.deepEqual(fs.readFileSync(file), binary);
});

test('Stop hook formats changed and untracked Markdown files', (t) => {
    const root = tempGitRoot(t);
    const file = path.join(root, 'stop.md');
    fs.writeFileSync(file, 'Stop edit   \r\n\r\n', 'utf8');

    assert.equal(runHook(root, 'stop', {}), null);
    assert.equal(fs.readFileSync(file, 'utf8'), 'Stop edit\n');
});
