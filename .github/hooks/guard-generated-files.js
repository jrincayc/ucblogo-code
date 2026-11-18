#!/usr/bin/env node
'use strict';

// Asks for confirmation before agent edit tools change generated Web Edition outputs in the repo root.

const path = require('node:path');

const ROOT = path.resolve(__dirname, '..', '..');
const GENERATED = /^(ucblogo\.(html|js|wasm)|sw\.js|workbox-[0-9a-f]{8}\.js)$/;
const PATH_KEYS = new Set(['filePath', 'file_path', 'path']);

function collectPaths(value, found = []) {
    if (Array.isArray(value)) {
        value.forEach((item) => collectPaths(item, found));
    } else if (value && typeof value === 'object') {
        for (const [key, item] of Object.entries(value)) {
            if (PATH_KEYS.has(key) && typeof item === 'string') found.push(item);
            else collectPaths(item, found);
        }
    }
    return found;
}

let raw = '';
process.stdin.setEncoding('utf8');
process.stdin.on('data', (chunk) => { raw += chunk; });
process.stdin.on('end', () => {
    let input;
    try {
        input = JSON.parse(raw);
    } catch {
        return;
    }
    const hit = collectPaths(input.tool_input)
        .map((file) => path.resolve(ROOT, file))
        .find((file) => path.dirname(file) === ROOT && GENERATED.test(path.basename(file)));
    if (!hit) return;
    process.stdout.write(JSON.stringify({
        hookSpecificOutput: {
            hookEventName: 'PreToolUse',
            permissionDecision: 'ask',
            permissionDecisionReason: `${path.basename(hit)} is a generated build output. Edit its source (shell.html, pre.js, liblogo.js, workbox-config.js, Makefile.am) and rebuild instead.`,
        },
    }));
});
