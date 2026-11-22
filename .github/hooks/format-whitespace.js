#!/usr/bin/env node
'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort: max).

const fs = require('node:fs');
const path = require('node:path');
const {
    formatPaths,
    changedPaths,
    TARGET_EXTENSIONS,
} = require('../../scripts/format-whitespace.js');

const DEFAULT_ROOT = path.resolve(__dirname, '..', '..');
const ROOT = process.env.UCBLOGO_FORMAT_ROOT
    ? path.resolve(process.env.UCBLOGO_FORMAT_ROOT)
    : DEFAULT_ROOT;

const PATH_KEYS = new Set([
    'filePath', 'file_path', 'path', 'paths', 'filePaths', 'file_paths', 'old_path', 'new_path',
]);
const READ_ONLY_TOOLS = new Set([
    'copilot_getNotebookSummary', 'file_search', 'get_errors', 'grep_search', 'list_dir',
    'read', 'read_file', 'read_notebook_cell_output', 'semantic_search', 'terminal_last_command',
    'terminal_selection', 'vscode_listCodeUsages', 'read_page', 'screenshot_page',
]);
const TOOL_BATCHES = new Set(['multi_tool_use.parallel', 'multi_tool_use.sequential']);

function collectToolNames(value, found = []) {
    if (Array.isArray(value)) {
        value.forEach((item) => collectToolNames(item, found));
    } else if (value && typeof value === 'object') {
        for (const [key, item] of Object.entries(value)) {
            if ((key === 'tool_name' || key === 'recipient_name') && typeof item === 'string') {
                found.push(item.replace(/^functions\./, ''));
            } else {
                collectToolNames(item, found);
            }
        }
    }
    return found;
}

function isReadOnlyRequest(input) {
    const toolNames = collectToolNames(input).filter((name) => !TOOL_BATCHES.has(name));
    return toolNames.length > 0 && toolNames.every((name) => READ_ONLY_TOOLS.has(name));
}

function collectPaths(value, found = []) {
    if (Array.isArray(value)) {
        value.forEach((item) => collectPaths(item, found));
    } else if (value && typeof value === 'object') {
        for (const [key, item] of Object.entries(value)) {
            if (PATH_KEYS.has(key)) {
                if (typeof item === 'string') found.push(item);
                else if (Array.isArray(item)) found.push(...item.filter((entry) => typeof entry === 'string'));
            } else {
                collectPaths(item, found);
            }
        }
    }
    return [...new Set(found)];
}

function isInside(root, file) {
    const relative = path.relative(root, file);
    return relative !== '..' && !relative.startsWith(`..${path.sep}`) && !path.isAbsolute(relative);
}

function eligiblePostToolPath(root, candidate) {
    const absolute = path.resolve(root, candidate);
    if (!isInside(root, absolute) || !fs.existsSync(absolute)) return false;
    let stat;
    try {
        stat = fs.statSync(absolute);
    } catch {
        return false;
    }
    if (!stat.isFile()) return false;
    let real;
    try {
        real = fs.realpathSync.native(absolute);
    } catch {
        return false;
    }
    if (!isInside(root, real) || !TARGET_EXTENSIONS.has(path.extname(real).toLowerCase())) return false;
    const relative = path.relative(root, real).replaceAll(path.sep, '/');
    if (relative === 'docs/accounting/agent-usage-ledger.md' || relative.startsWith('.git/') || relative.startsWith('node_modules/') || relative.startsWith('reports/') || relative.startsWith('screenshots/')) return false;
    return !fs.readFileSync(real).includes(0);
}

let raw = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', (chunk) => { raw += chunk; });
process.stdin.on('end', () => {
    let input = {};
    if (raw.trim()) {
        try {
            input = JSON.parse(raw);
        } catch {
            input = {};
        }
    }

    const mode = process.argv[2] || 'post';
    let paths = [];
    if (mode === 'stop') {
        paths = changedPaths(ROOT);
    } else {
        if (isReadOnlyRequest(input)) return;
        paths = collectPaths(input)
            .map((file) => path.resolve(ROOT, file))
            .filter((file) => eligiblePostToolPath(ROOT, file));
    }

    if (!paths.length) return;

    const results = formatPaths(paths, { root: ROOT });
    const warnings = results
        .filter((result) => result.status === 'skipped' && ['binary-or-invalid-utf8', 'outside-workspace'].includes(result.reason))
        .map((result) => {
            const label = result.reason === 'outside-workspace' ? 'outside workspace' : 'binary or invalid UTF-8';
            const relative = path.relative(ROOT, result.file);
            return `Skipped whitespace formatting for ${relative && !relative.startsWith('..') ? relative : result.file} (${label}).`;
        });

    const errors = results
        .filter((result) => result.status === 'error')
        .map((result) => `Whitespace formatting error for ${result.file}: ${result.reason}`);

    const messages = [...warnings, ...errors];
    if (messages.length) {
        process.stdout.write(JSON.stringify({ systemMessage: messages.join('\n') }));
    }
});

module.exports = { collectToolNames, isReadOnlyRequest, collectPaths, eligiblePostToolPath };
