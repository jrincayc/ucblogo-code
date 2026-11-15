const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');
const { describe, it } = require('node:test');

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