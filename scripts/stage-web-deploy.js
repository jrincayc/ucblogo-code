#!/usr/bin/env node
'use strict';

// Copies exactly the files the Web Edition needs into a deploy directory and verifies them.

const crypto = require('node:crypto');
const fs = require('node:fs');
const path = require('node:path');

// help/docs.json is fetched at runtime (not precached), so the deploy must carry it.
const REQUIRED = ['ucblogo.html', 'ucblogo.js', 'ucblogo.wasm', 'sw.js', 'manifest.json', 'help/docs.json'];
const OPTIONAL = ['ucblogo.js.map', 'ucblogo.wasm.map', 'sw.js.map'];
const WORKBOX_RUNTIME = /^workbox-[0-9a-f]{8}\.js(\.map)?$/;

function fail(message) {
    throw new Error(message);
}

function md5(file) {
    return crypto.createHash('md5').update(fs.readFileSync(file)).digest('hex');
}

// The published tree is flat, so a reference resolves to a bare file name.
function localName(reference) {
    if (/^(?:[a-z][a-z0-9+.-]*:|\/\/|#)/i.test(reference)) return null;
    const name = reference.split(/[?#]/)[0].replace(/^\.\//, '');
    if (name.includes('/')) fail(`Unsupported nested or absolute reference: ${reference}`);
    return name || null;
}

// Precache URLs carry the deployment prefix (for example /ucblogo-code/), so keep only the file name.
function precacheName(url) {
    return path.posix.basename(url.split(/[?#]/)[0]);
}

function parseServiceWorker(source) {
    const runtime = /["']\.\/(workbox-[0-9a-f]{8})["']/.exec(source);
    if (!runtime) fail('sw.js does not import a workbox-xxxxxxxx runtime');
    const entries = [...source.matchAll(/\{url:"([^"]+)",revision:(null|"[0-9a-f]+")\}/g)]
        .map(([, url, revision]) => ({ url, revision: revision === 'null' ? null : revision.slice(1, -1) }));
    if (!entries.length) fail('sw.js has no precache entries');
    return { runtime: `${runtime[1]}.js`, entries };
}

function inventory(dir) {
    const file = (name) => path.join(dir, name);
    const absent = REQUIRED.filter((name) => !fs.existsSync(file(name)));
    if (absent.length) fail(`${dir} is missing ${absent.join(', ')}`);

    const sw = parseServiceWorker(fs.readFileSync(file('sw.js'), 'utf8'));
    const html = fs.readFileSync(file('ucblogo.html'), 'utf8');
    const manifest = JSON.parse(fs.readFileSync(file('manifest.json'), 'utf8'));
    const references = [
        ...[...html.matchAll(/<link\b[^>]*\bhref=["']([^"']+)["']/gi)].map((match) => match[1]),
        ...(manifest.icons || []).map((icon) => icon.src),
    ];

    const names = new Set([...REQUIRED, sw.runtime]);
    for (const reference of references) {
        const name = localName(reference);
        if (name) names.add(name);
    }
    for (const { url } of sw.entries) names.add(precacheName(url));

    const missing = [...names].filter((name) => !fs.existsSync(file(name)));
    if (missing.length) fail(`${dir} is missing referenced files: ${missing.join(', ')}`);

    for (const name of [...OPTIONAL, `${sw.runtime}.map`]) {
        if (fs.existsSync(file(name))) names.add(name);
    }
    return { names: [...names].sort(), sw };
}

function verify(dir, { expectVersion } = {}) {
    const { sw } = inventory(dir);
    for (const { url, revision } of sw.entries) {
        const name = precacheName(url);
        if (revision && md5(path.join(dir, name)) !== revision) {
            fail(`sw.js revision for ${name} does not match the staged file; rebuild sw.js`);
        }
    }
    if (expectVersion) {
        const wasm = fs.readFileSync(path.join(dir, 'ucblogo.wasm'));
        if (!wasm.includes(Buffer.from(`(${expectVersion})`))) {
            fail(`ucblogo.wasm does not embed version (${expectVersion}); rebuild after tagging`);
        }
    }
}

function stage(source, target, options = {}) {
    if (path.resolve(source) === path.resolve(target)) fail('The target must differ from the source');
    const { names } = inventory(source);

    fs.mkdirSync(target, { recursive: true });
    // Browsers refetch the runtime when sw.js changes, so older runtimes are dead weight.
    for (const name of fs.readdirSync(target)) {
        if (WORKBOX_RUNTIME.test(name)) fs.rmSync(path.join(target, name));
    }
    for (const name of names) {
        fs.mkdirSync(path.dirname(path.join(target, name)), { recursive: true });
        fs.copyFileSync(path.join(source, name), path.join(target, name));
    }

    verify(target, options);
    return names;
}

function main(argv) {
    let source = path.resolve(__dirname, '..');
    let target;
    let expectVersion;
    for (let index = 0; index < argv.length; index++) {
        if (argv[index] === '--source') source = path.resolve(argv[++index]);
        else if (argv[index] === '--expect-version') expectVersion = argv[++index];
        else if (!target) target = path.resolve(argv[index]);
        else fail(`Unexpected argument: ${argv[index]}`);
    }
    if (!target) fail('Usage: stage-web-deploy.js <target-dir> [--source <dir>] [--expect-version <tag>]');

    const names = stage(source, target, { expectVersion });
    console.log(`Staged ${names.length} files into ${target}:\n${names.map((name) => `  ${name}`).join('\n')}`);
}

if (require.main === module) {
    try {
        main(process.argv.slice(2));
    } catch (error) {
        console.error(error.message);
        process.exit(1);
    }
}

module.exports = { stage, verify };
