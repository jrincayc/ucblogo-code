const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const { describe, it } = require('node:test');
const { createStorageStatusController } = require('../../storage-status.js');

const repositoryRoot = path.resolve(__dirname, '../..');

describe('startup error reporting', () => {
    it('skips local service workers and handles registration failures', async () => {
        const shell = fs.readFileSync(path.join(repositoryRoot, 'shell.html'), 'utf8');
        const registration = shell.match(/    if \('serviceWorker' in navigator\) \{[\s\S]*?\n    \}/);
        assert.ok(registration, 'service worker registration');
        for (const serviceWorker of [true, false]) {
            for (const hostname of ['localhost', '127.0.0.1', '[::1]', 'example.com']) {
                let load;
                const registrations = [];
                const unregistered = [];
                const warnings = [];
                const context = {
                    Module: { serviceWorker },
                    window: { location: { hostname }, addEventListener: (event, handler) => { load = handler; } },
                    navigator: { serviceWorker: {
                        register: (script) => {
                            registrations.push(script);
                            return Promise.reject(new Error('unavailable'));
                        },
                        getRegistrations: () => Promise.resolve([
                            { unregister: () => unregistered.push(hostname) },
                        ]),
                    } },
                    console: { warn: (...args) => warnings.push(args) },
                };
                vm.runInNewContext(registration[0], context);
                load();
                await Promise.resolve();
                await Promise.resolve();
                const label = `${hostname} serviceWorker=${serviceWorker}`;
                if (!serviceWorker) {
                    assert.deepEqual(registrations, [], label);
                    assert.deepEqual(unregistered, [hostname], label);
                } else if (hostname !== 'example.com') {
                    assert.deepEqual(registrations, [], label);
                    assert.deepEqual(unregistered, [], label);
                } else {
                    assert.deepEqual(registrations, ['sw.js'], label);
                    assert.equal(warnings.length, 1, label);
                }
            }
        }
    });

    it('traces uncaught errors and unhandled rejections without suppressing them', () => {
        const shell = fs.readFileSync(path.join(repositoryRoot, 'shell.html'), 'utf8');
        const handlers = shell.match(/    function showJavaScriptError\([\s\S]*?    window\.addEventListener\('unhandledrejection',[\s\S]*?    \}\);/);
        assert.ok(handlers, 'startup error handlers');
        const traces = [];
        const listeners = {};
        const statuses = [];
        let panel;
        const context = {
            window: { addEventListener: (name, handler) => { listeners[name] = handler; } },
            document: {
                getElementById: () => panel,
                createElement: () => ({ textContent: '', style: {}, setAttribute() {} }),
                body: { appendChild: (element) => { panel = element; } },
            },
            console: { trace: (...args) => traces.push(args), error() {} },
            Module: { setStatus: (status) => statuses.push(status) },
            spinnerElement: { style: {} },
        };
        vm.runInNewContext(handlers[0], context);
        const error = new Error('startup failed');
        error.stack = 'runEmAsmFunction@app.js:1:96161';
        assert.equal(context.window.onerror(error.message, 'app.js', 12, 4, error), undefined);
        assert.equal(traces[0][1], error);
        assert.equal(traces[0][2].source, 'app.js');
        assert.match(panel.textContent, /startup failed/);
        assert.match(panel.textContent, /runEmAsmFunction@app\.js:1:96161/);
        assert.match(panel.textContent, /app\.js:12:4/);
        assert.equal(statuses[0], 'Exception thrown, see JavaScript console');
        const reason = new Error('async startup failed');
        assert.equal(listeners.unhandledrejection({ reason }), undefined);
        assert.equal(traces[1][1], reason);
        assert.match(panel.textContent, /async startup failed/);
        context.window.onerror('Script error.', '', 0, 0, null);
        assert.equal(traces[2][1], 'Script error.');
    });
});

describe('IDBFS persistence signals', () => {
    function initializeStorage({
        queuePersist = true, populateError = null, persistError = null,
        storageApi = {}, persistenceRecord = null,
    } = {}) {
        const source = fs.readFileSync(path.join(repositoryRoot, 'pre.js'), 'utf8');
        const initialization = source.match(/Module\.onRuntimeInitialized = \(\) => \{[\s\S]*?\n\}/);
        assert.ok(initialization, 'runtime initialization source');
        const listeners = new Map();
        const events = [];
        const storage = {
            addEventListener(name, listener) {
                listeners.set(name, [...(listeners.get(name) || []), listener]);
            },
            dispatchEvent(event) {
                events.push(event);
                for (const listener of listeners.get(event.type) || []) listener(event);
            },
        };
        const mount = {};
        const IDBFS = {};
        IDBFS.syncfs = (_mount, populate, callback) => callback(populate ? populateError : persistError);
        if (queuePersist)
            IDBFS.queuePersist = (persistMount) => IDBFS.syncfs(persistMount, false, () => {});
        const FS = {
            mkdir() {},
            mount() {},
            chdir() {},
            syncfs(populate, callback) { IDBFS.syncfs(mount, populate, callback); },
        };
        class CustomEvent {
            constructor(type, options = {}) { this.type = type; this.detail = options.detail; }
        }
        const Module = { storage };
        vm.runInNewContext(initialization[0], {
            Module, document: { getElementById: () => null }, FS, IDBFS, CustomEvent,
            navigator: { storage: storageApi },
            createStorageStatusController,
            localStorage: {
                getItem: () => persistenceRecord ? JSON.stringify(persistenceRecord) : null,
                setItem() {},
            },
            window: { addEventListener() {} },
            matchMedia: () => ({ matches: false, addEventListener() {} }),
            console: { error() {}, log() {} },
        });
        Module.onRuntimeInitialized();
        return { Module, IDBFS, events };
    }

    it('resolves ready after populate and reports population failures', async () => {
        const ready = initializeStorage();
        await ready.Module.storage.ready;
        assert.equal(ready.Module.storage.populated, true);

        const failed = initializeStorage({ populateError: new Error('blocked') });
        await failed.Module.storage.ready;
        assert.equal(failed.Module.storage.populated, false);
        assert.equal(failed.Module.storage.lastError.message, 'blocked');
    });

    it('reports background persist success and failure', () => {
        const success = initializeStorage();
        success.IDBFS.queuePersist({});
        assert.ok(success.events.some((event) => event.type === 'saved'));
        assert.ok(success.events.some((event) => event.type === 'persisted'));

        const failure = initializeStorage({ persistError: new Error('quota') });
        failure.IDBFS.queuePersist({});
        const event = failure.events.find((item) => item.type === 'savefailed');
        assert.equal(event.detail.error.message, 'quota');
        assert.equal(failure.Module.storage.lastError.message, 'quota');
    });

    it('reports unavailable persistence signals when queuePersist is missing', async () => {
        const result = initializeStorage({ queuePersist: false });
        await result.Module.storage.ready;
        assert.ok(result.events.some((event) => event.type === 'signals-unavailable'));
    });

    it('reports approximate estimates and tolerates missing or rejecting APIs', async () => {
        const available = initializeStorage({ storageApi: {
            persisted: async () => false,
            estimate: async () => ({ usage: 2048, quota: 8192 }),
        } });
        const status = await available.Module.storage.status();
        assert.deepEqual(
            { api: status.api, persisted: status.persisted, usage: status.usage, quota: status.quota },
            { api: true, persisted: false, usage: 2048, quota: 8192 },
        );

        const unavailable = initializeStorage({ storageApi: {
            persisted: async () => { throw new Error('unsupported'); },
            estimate: async () => { throw new Error('unsupported'); },
        } });
        const unknown = await unavailable.Module.storage.status();
        assert.equal(unknown.persisted, null);
        assert.equal(unknown.usage, null);
        assert.equal(unknown.quota, null);
    });

    it('requests persistence after a successful save, observes cooldown, and allows an explicit request', async () => {
        let calls = 0;
        const storageApi = {
            persisted: async () => false,
            estimate: async () => ({}),
            persist: async () => { calls++; return false; },
        };
        const saved = initializeStorage({ storageApi });
        assert.equal(calls, 0);
        saved.IDBFS.queuePersist({});
        await new Promise((resolve) => setTimeout(resolve, 0));
        assert.equal(calls, 1);
        saved.IDBFS.queuePersist({});
        await new Promise((resolve) => setTimeout(resolve, 0));
        assert.equal(calls, 1);

        const cooledDown = initializeStorage({
            storageApi,
            persistenceRecord: { result: false, time: Date.now() },
        });
        cooledDown.IDBFS.queuePersist({});
        await new Promise((resolve) => setTimeout(resolve, 0));
        assert.equal(calls, 1);
        await cooledDown.Module.storage.requestPersistence();
        assert.equal(calls, 2);
    });
});

describe('storage durability guidance', () => {
    it('describes the Storage status and the actual session-download coverage', () => {
        const shell = fs.readFileSync(path.join(repositoryRoot, 'shell.html'), 'utf8');
        const runtime = fs.readFileSync(path.join(repositoryRoot, 'pre.js'), 'utf8');
        const docs = fs.readFileSync(path.join(repositoryRoot, 'webedition.md'), 'utf8');
        const download = runtime.match(/Module\.downloadSessionClicked = async \(event\) => \{[\s\S]*?\n\};/);
        assert.ok(download, 'session download handler');
        assert.match(download[0], /runLogo\(`save/);
        assert.match(download[0], /FS\.readFile\('\/tmp\/ucblogo_session\.lgo'\)/);
        assert.doesNotMatch(download[0], /\/d/);
        assert.match(shell, /id="storageState" role="status" aria-live="polite"/);
        assert.match(shell, /id="requestPersistentStorage"/);
        assert.match(docs, /does not export files stored in the virtual/);
        assert.match(docs, /Safari's eviction and/);
        assert.doesNotMatch(docs, /ensure reliable long term data persistence/);
    });
});

function makeTextControl(value, selectionStart, selectionEnd) {
    return {
        value,
        selectionStart,
        selectionEnd,
        focus() { this.focused = true; },
        setSelectionRange(start, end) {
            this.selectionStart = start;
            this.selectionEnd = end;
        },
    };
}

function loadSoftKeyHandler(controls) {
    const source = fs.readFileSync(path.join(repositoryRoot, 'pre.js'), 'utf8');
    const handler = source.match(/Module\.insertAtCaret = [\s\S]*?Module\.softKeyDown = \(event, key\) => \{[\s\S]*?\n\};/);
    assert.ok(handler, 'soft-key handler source');
    const Module = {};
    const document = { getElementById: (id) => controls[id] };
    vm.runInNewContext(handler[0], { Module, document });
    return Module.softKeyDown;
}

describe('soft-key accessibility', () => {
    it('provides named click buttons in a named group without changing their size', () => {
        const shell = fs.readFileSync(path.join(repositoryRoot, 'shell.html'), 'utf8');
        const applicationScript = shell.match(/<script type='text\/javascript'>\s*([\s\S]*?)\s*<\/script>/);
        const component = shell.match(/class LogoSoftKeys extends HTMLElement \{([\s\S]*?)customElements\.define\("logo-soft-keys", LogoSoftKeys\);/);
        assert.ok(applicationScript, 'application script source');
        assert.doesNotThrow(() => new vm.Script(applicationScript[1]), 'application script syntax');
        assert.ok(component, 'soft-key component source');
        const source = component[1];
        const buttons = [...source.matchAll(/<button\b[^>]*aria-label="([^"]+)"[^>]*onclick="Module\.softKeyDown\(event, /g)];

        assert.ok(source.includes('role="group" aria-label="Logo punctuation keys"'));
        assert.deepEqual(buttons.map((button) => button[1]), [
            'Insert double quote',
            'Insert colon',
            'Insert left square bracket',
            'Insert right square bracket',
            'Insert left curly brace',
            'Insert right curly brace',
            'Insert less-than sign',
            'Insert greater-than sign',
            'Insert vertical bar',
            'Insert backtick',
        ]);
        assert.ok(!source.includes('onmousedown='));
        assert.ok(!source.includes('<label'));
        assert.ok(source.includes('height: 1.75em;'));
        assert.ok(source.includes('width: 1.75em;'));
    });

    it('replaces selected command text and restores the caret', () => {
        const input = makeTextControl('draw turtle', 5, 11);
        const handler = loadSoftKeyHandler({
            logoInputText: input,
            editorDialog: { open: false },
            editorTextArea: makeTextControl('', 0, 0),
        });
        handler({}, '[');
        assert.equal(input.value, 'draw [');
        assert.equal(input.selectionStart, 6);
        assert.equal(input.selectionEnd, 6);
        assert.equal(input.focused, true);
    });

    it('replaces selected editor text and restores the caret', () => {
        const editorText = makeTextControl('abcXYZdef', 3, 6);
        const handler = loadSoftKeyHandler({
            logoInputText: makeTextControl('', 0, 0),
            editorDialog: { open: true },
            editorTextArea: editorText,
        });
        handler({}, '`');
        assert.equal(editorText.value, 'abc`def');
        assert.equal(editorText.selectionStart, 4);
        assert.equal(editorText.selectionEnd, 4);
        assert.equal(editorText.focused, true);
    });
});