const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const assert = require('node:assert/strict');

describe('startup error reporting', function () {
    it('skips local service workers and handles registration failures', async function () {
        const shell = fs.readFileSync(path.resolve(__dirname, '../../shell.html'), 'utf8');
        const registration = shell.match(/    if \('serviceWorker' in navigator &&[\s\S]*?\n    \}/);
        assert.ok(registration, 'service worker registration');
        for (const hostname of ['localhost', '127.0.0.1', '[::1]', 'example.com']) {
            let load;
            const registrations = [];
            const warnings = [];
            const context = {
                window: { location: { hostname }, addEventListener: (event, handler) => { load = handler; } },
                navigator: { serviceWorker: { register: (path) => {
                    registrations.push(path);
                    return Promise.reject(new Error('unavailable'));
                } } },
                console: { warn: (...args) => warnings.push(args) },
            };
            vm.runInNewContext(registration[0], context);
            if (hostname !== 'example.com') {
                assert.equal(load, undefined, hostname);
                continue;
            }
            load();
            await Promise.resolve();
            assert.deepEqual(registrations, ['sw.js']);
            assert.equal(warnings.length, 1);
        }
    });

    it('traces uncaught errors and unhandled rejections without suppressing them', function () {
        const shell = fs.readFileSync(path.resolve(__dirname, '../../shell.html'), 'utf8');
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