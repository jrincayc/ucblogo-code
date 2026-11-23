#!/usr/bin/env node
'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort: max).

const fs = require('node:fs');
const path = require('node:path');
const { spawnSync } = require('node:child_process');
const { TextDecoder } = require('node:util');

const TARGET_EXTENSIONS = new Set(['.md', '.markdown', '.mkd', '.mdown', '.txt', '.text']);
const MARKDOWN_EXTENSIONS = new Set(['.md', '.markdown', '.mkd', '.mdown']);

function isInside(root, candidate) {
    const relative = path.relative(root, candidate);
    return relative !== '..' && !relative.startsWith(`..${path.sep}`) && !path.isAbsolute(relative);
}

function resolveWorkspacePath(root, candidate) {
    const absolute = path.isAbsolute(candidate) ? path.resolve(candidate) : path.resolve(root, candidate);
    if (!isInside(root, absolute)) return null;
    if (!fs.existsSync(absolute)) return null;

    let absoluteStat;
    try {
        absoluteStat = fs.statSync(absolute);
    } catch {
        return null;
    }
    if (!absoluteStat.isFile()) return null;

    let real;
    try {
        real = fs.realpathSync.native(absolute);
    } catch {
        return null;
    }
    if (!isInside(root, real)) return null;

    let realStat;
    try {
        realStat = fs.statSync(real);
    } catch {
        return null;
    }
    if (!realStat.isFile()) return null;
    return real;
}

function decodeUtf8(buffer) {
    if (buffer.includes(0)) return null;
    try {
        return new TextDecoder('utf-8', { fatal: true, ignoreBOM: true }).decode(buffer);
    } catch {
        return null;
    }
}

function normalizeText(text, markdown = false) {
    text = text.replace(/\r\n?/g, '\n');
    let lines = text.split('\n');

    while (lines.length && /^[ \t]*$/.test(lines[lines.length - 1])) lines.pop();

    lines = lines.map((line, index) => {
        const hasFollowingContent = markdown && index < lines.length - 1;
        if (hasFollowingContent && /[^ \t]  $/.test(line)) return line;
        return line.replace(/[ \t]+$/, '');
    });

    if (!lines.length) return '';
    return `${lines.join('\n')}\n`;
}

function formatFile(file, options = {}) {
    const root = path.resolve(options.root || process.cwd());
    const rootReal = fs.existsSync(root) ? fs.realpathSync.native(root) : root;
    const absolute = resolveWorkspacePath(rootReal, file);

    if (!absolute) {
        const candidate = path.resolve(rootReal, file);
        let outside = !isInside(rootReal, candidate);
        if (!outside && fs.existsSync(candidate)) {
            try {
                outside = !isInside(rootReal, fs.realpathSync.native(candidate));
            } catch {
                outside = false;
            }
        }
        return {
            status: 'skipped',
            reason: outside ? 'outside-workspace' : 'missing-or-not-file',
            file: candidate,
        };
    }

    const extension = path.extname(absolute).toLowerCase();
    if (!TARGET_EXTENSIONS.has(extension)) {
        return { status: 'skipped', reason: 'unsupported-extension', file: absolute };
    }

    const buffer = fs.readFileSync(absolute);
    const text = decodeUtf8(buffer);
    if (text === null) {
        return { status: 'skipped', reason: 'binary-or-invalid-utf8', file: absolute };
    }

    const normalized = normalizeText(text, MARKDOWN_EXTENSIONS.has(extension));
    if (normalizeText(normalized, MARKDOWN_EXTENSIONS.has(extension)) !== normalized) {
        throw new Error(`Normalizer was not idempotent for ${absolute}`);
    }

    if (normalized === text) {
        return { status: 'unchanged', file: absolute };
    }

    if (options.check) {
        return { status: 'would-change', file: absolute };
    }

    fs.writeFileSync(absolute, normalized, 'utf8');

    const writtenBuffer = fs.readFileSync(absolute);
    const expectedBuffer = Buffer.from(normalized, 'utf8');
    if (!writtenBuffer.equals(expectedBuffer)) {
        throw new Error(`Post-write byte verification failed for ${absolute}`);
    }

    const written = decodeUtf8(writtenBuffer);
    if (written === null || normalizeText(written, MARKDOWN_EXTENSIONS.has(extension)) !== written) {
        throw new Error(`Post-write verification failed for ${absolute}`);
    }

    return { status: 'formatted', file: absolute };
}

function formatPaths(paths, options = {}) {
    const results = [];
    const seen = new Set();
    for (const candidate of paths) {
        const absolute = path.resolve(options.root || process.cwd(), candidate);
        if (seen.has(absolute)) continue;
        seen.add(absolute);
        try {
            results.push(formatFile(candidate, options));
        } catch (error) {
            results.push({ status: 'error', file: absolute, reason: error.message });
        }
    }
    return results;
}

function splitNulls(buffer) {
    return buffer.toString('utf8').split('\0').filter(Boolean);
}

function changedPaths(root = process.cwd()) {
    const absoluteRoot = path.resolve(root);
    const names = [];

    const head = spawnSync('git', ['-C', absoluteRoot, 'diff', '--name-only', '-z', 'HEAD'], { encoding: null });
    if (head.status === 0) names.push(...splitNulls(head.stdout));

    const staged = spawnSync('git', ['-C', absoluteRoot, 'diff', '--cached', '--name-only', '-z'], { encoding: null });
    if (staged.status === 0) names.push(...splitNulls(staged.stdout));

    const unstaged = spawnSync('git', ['-C', absoluteRoot, 'diff', '--name-only', '-z'], { encoding: null });
    if (unstaged.status === 0) names.push(...splitNulls(unstaged.stdout));

    const untracked = spawnSync('git', ['-C', absoluteRoot, 'ls-files', '--others', '--exclude-standard', '-z'], { encoding: null });
    if (untracked.status === 0) names.push(...splitNulls(untracked.stdout));

    return [...new Set(names)].map((name) => path.resolve(absoluteRoot, name));
}

function displayPath(root, file) {
    const relative = path.relative(root, file);
    return relative && !relative.startsWith('..') ? relative : file;
}

function main(argv) {
    const check = argv.includes('--check');
    const changed = argv.includes('--changed');
    const explicit = argv.filter((argument) => argument !== '--check' && argument !== '--changed');
    const root = fs.existsSync(process.cwd()) ? fs.realpathSync.native(process.cwd()) : process.cwd();

    let paths = explicit;
    if (changed) paths = [...paths, ...changedPaths(root)];
    if (!explicit.length && !changed) {
        console.error('Usage: format-whitespace.js [--check] [--changed] <file> [file...]');
        return 2;
    }

    const results = formatPaths(paths, { root, check });
    const changedResults = results.filter((result) => result.status === 'formatted' || result.status === 'would-change');
    const errors = results.filter((result) => result.status === 'error');
    const skipped = results.filter((result) => result.status === 'skipped' && ['binary-or-invalid-utf8', 'outside-workspace'].includes(result.reason));

    for (const result of skipped) {
        const label = result.reason === 'outside-workspace' ? 'outside workspace' : 'binary or invalid UTF-8';
        console.warn(`Warning: skipped ${displayPath(root, result.file)} (${label}).`);
    }
    for (const result of errors) {
        console.error(`Error: ${displayPath(root, result.file)}: ${result.reason}`);
    }

    if (check) {
        if (changedResults.length) {
            for (const result of changedResults) console.log(`Would change: ${displayPath(root, result.file)}`);
            return errors.length ? 1 : 1;
        }
        console.log('Whitespace check passed.');
        return errors.length ? 1 : 0;
    }

    if (changedResults.length) {
        for (const result of changedResults) console.log(`Formatted: ${displayPath(root, result.file)}`);
    } else {
        console.log('No whitespace changes needed.');
    }
    return errors.length ? 1 : 0;
}

if (require.main === module) process.exitCode = main(process.argv.slice(2));

module.exports = {
    TARGET_EXTENSIONS,
    MARKDOWN_EXTENSIONS,
    decodeUtf8,
    normalizeText,
    formatFile,
    formatPaths,
    changedPaths,
    main,
};
