'use strict';

const assert = require('node:assert/strict');
const { spawnSync } = require('node:child_process');
const path = require('node:path');
const test = require('node:test');

const hookPath = path.join(__dirname, 'guard-generated-files.js');

function runHook(input) {
    const result = spawnSync(process.execPath, [hookPath], {
        input: JSON.stringify(input),
        encoding: 'utf8',
    });

    assert.equal(result.status, 0, result.stderr);
    return result.stdout.trim() ? JSON.parse(result.stdout) : null;
}

function call(toolName, toolInput) {
    return { tool_name: toolName, tool_input: toolInput };
}

test('does not request confirmation for read-only generated-file access', () => {
    const readCalls = [
        call('read_file', { filePath: 'ucblogo.js' }),
        call('functions.read_file', { filePath: 'ucblogo.js' }),
        call('list_dir', { path: 'ucblogo.js' }),
        call('copilot_getNotebookSummary', { filePath: 'ucblogo.js' }),
        call('read_notebook_cell_output', { filePath: 'ucblogo.js' }),
    ];

    for (const readCall of readCalls) {
        assert.equal(runHook(readCall), null, readCall.tool_name);
    }
});

test('does not request confirmation for a read-only parallel batch', () => {
    const result = runHook(call('multi_tool_use.parallel', {
        tool_uses: [
            {
                recipient_name: 'functions.read_file',
                parameters: { filePath: 'ucblogo.js' },
            },
            {
                recipient_name: 'functions.list_dir',
                parameters: { path: 'ucblogo.js' },
            },
        ],
    }));

    assert.equal(result, null);
});

test('requests confirmation for an edit to a generated file', () => {
    const result = runHook(call('edit_file', { filePath: 'ucblogo.js' }));

    assert.equal(result.hookSpecificOutput.permissionDecision, 'ask');
    assert.match(result.hookSpecificOutput.permissionDecisionReason, /ucblogo\.js/);
});

test('requests confirmation for a generated-file edit in a mixed parallel batch', () => {
    const result = runHook(call('multi_tool_use.parallel', {
        tool_uses: [
            {
                recipient_name: 'functions.read_file',
                parameters: { filePath: 'ucblogo.js' },
            },
            {
                recipient_name: 'functions.edit_file',
                parameters: { filePath: 'ucblogo.js' },
            },
        ],
    }));

    assert.equal(result.hookSpecificOutput.permissionDecision, 'ask');
    assert.match(result.hookSpecificOutput.permissionDecisionReason, /ucblogo\.js/);
});

test('does not request confirmation for non-generated files', () => {
    const result = runHook(call('edit_file', { filePath: 'shell.html' }));

    assert.equal(result, null);
});
