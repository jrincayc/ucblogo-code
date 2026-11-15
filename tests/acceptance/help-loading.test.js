const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { it } = require('node:test');

const source = fs.readFileSync(path.join(__dirname, '../../pre.js'), 'utf8');
const helpSource = source.slice(source.indexOf('Module.help = {'), source.indexOf('// FIXME add options'));

function loader({ loaded = false, fail = false, idleCallback = true } = {}) {
    const callbacks = {};
    const requests = [];
    const Module = {};
    const window = {
        addEventListener(name, callback) { callbacks[name] = callback; },
        requestIdleCallback(callback) { callbacks.idle = callback; },
    };
    if (!idleCallback) delete window.requestIdleCallback;
    vm.runInNewContext(helpSource, {
        Module,
        window,
        document: { readyState: loaded ? 'complete' : 'loading' },
        console: { error() {} },
        setTimeout(callback) { callbacks.idle = callback; },
        fetch: async (url, options) => {
            requests.push({ url, priority: options.priority });
            return { ok: !fail, status: 503, json: async () => ({ docs: [] }) };
        },
    });
    return { Module, callbacks, requests };
}

it('waits for runtime, page load, and idle even if Help is opened early', async () => {
    const { Module, callbacks, requests } = loader();
    const early = Module.help.loadDocs();
    await Promise.resolve();
    assert.equal(requests.length, 0);
    assert.equal(callbacks.load, undefined);
    Module.postRun[0]();
    await Promise.resolve();
    assert.equal(requests.length, 0);
    assert.equal(callbacks.idle, undefined);
    callbacks.load();
    await Promise.resolve();
    assert.equal(requests.length, 0);
    callbacks.idle();
    await early;
    assert.deepEqual(requests, [{ url: 'help/docs.json', priority: 'low' }]);
});

it('shares an eager request and supports browsers without requestIdleCallback', async () => {
    const { Module, callbacks, requests } = loader({ loaded: true, idleCallback: false });
    Module.postRun[0]();
    const first = Module.help.loadDocs();
    assert.equal(first, Module.help.loadDocs());
    callbacks.idle();
    await first;
    assert.equal(Module.help.docs.length, 0);
    assert.equal(requests.length, 1);
});

it('clears a failed request so Retry can issue a fresh one', async () => {
    const { Module, callbacks, requests } = loader({ loaded: true, fail: true });
    Module.postRun[0]();
    callbacks.idle();
    await assert.rejects(Module.help.loadDocs(), /HTTP 503/);
    assert.equal(Module.help.docs, null);
    assert.equal(Module.help.docsPromise, null);
    await assert.rejects(Module.help.loadDocs(), /HTTP 503/);
    assert.equal(requests.length, 2);
});