const crypto = require('node:crypto');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const assert = require('node:assert/strict');
const { afterEach, beforeEach, describe, it } = require('node:test');

const { stage } = require('../../scripts/stage-web-deploy.js');

const md5 = (content) => crypto.createHash('md5').update(content).digest('hex');

describe('web deploy staging', () => {
    let root;
    let source;
    let target;

    function writeBuild(version = 'p99') {
        const files = {
            'ucblogo.html': '<link rel="preload" href="ucblogo.js"><link rel="icon" href="logologo.ico">',
            'ucblogo.js': 'js',
            'ucblogo.wasm': Buffer.from(`wasm (${version}) wasm`),
        };
        const precache = Object.entries(files)
            .map(([name, content]) => `{url:"/ucblogo-code/${name}",revision:"${md5(content)}"}`)
            .join(',');
        Object.assign(files, {
            'sw.js': `define(["./workbox-aaaaaaaa"],function(e){"use strict";e.precacheAndRoute([${precache}])});`,
            'workbox-aaaaaaaa.js': 'runtime',
            'workbox-aaaaaaaa.js.map': 'runtime map',
            'manifest.json': JSON.stringify({ icons: [{ src: 'ucblogo-192.png' }] }),
            'ucblogo-192.png': 'icon',
            'logologo.ico': 'ico',
            // Present in the source tree but not part of the published app.
            'webserver.js': 'server',
            'pre.js': 'pre',
            'workbox-config.js': 'config',
            'chromeflagsjspi.png': 'png',
        });
        for (const [name, content] of Object.entries(files)) fs.writeFileSync(path.join(source, name), content);
    }

    beforeEach(() => {
        root = fs.mkdtempSync(path.join(os.tmpdir(), 'stage-web-'));
        source = path.join(root, 'source');
        target = path.join(root, 'target');
        fs.mkdirSync(source);
        fs.mkdirSync(target);
        writeBuild();
    });

    afterEach(() => fs.rmSync(root, { recursive: true, force: true }));

    it('copies exactly the runtime files and replaces stale Workbox runtimes', () => {
        fs.writeFileSync(path.join(target, 'workbox-bbbbbbbb.js'), 'stale');
        fs.writeFileSync(path.join(target, 'gpl.html'), 'kept');

        stage(source, target, { expectVersion: 'p99' });

        assert.deepEqual(fs.readdirSync(target).sort(), [
            'gpl.html', 'logologo.ico', 'manifest.json', 'sw.js', 'ucblogo-192.png',
            'ucblogo.html', 'ucblogo.js', 'ucblogo.wasm', 'workbox-aaaaaaaa.js', 'workbox-aaaaaaaa.js.map',
        ]);
    });

    it('fails when a referenced file is missing', () => {
        fs.rmSync(path.join(source, 'ucblogo-192.png'));
        assert.throws(() => stage(source, target), /ucblogo-192\.png/);
    });

    it('fails when sw.js revisions do not match the staged bytes', () => {
        fs.writeFileSync(path.join(source, 'ucblogo.js'), 'changed after sw.js was built');
        assert.throws(() => stage(source, target), /revision for ucblogo\.js/);
    });

    it('fails when the build does not embed the expected version', () => {
        assert.throws(() => stage(source, target, { expectVersion: 'p100' }), /does not embed version \(p100\)/);
    });

    it('refuses to stage a directory onto itself', () => {
        assert.throws(() => stage(source, source), /must differ/);
    });
});
