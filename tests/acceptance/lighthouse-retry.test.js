const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { it } = require('node:test');
const chai = require('chai');

const source = fs.readFileSync(path.join(__dirname, 'lighthouse.test.js'), 'utf8');
const runtimeError = { code: 'NO_NAVSTART', message: 'Trace has no navigation start' };

function auditResult({ error, performance = 1 } = {}) {
    const lhr = {
        categories: {
            performance: { score: error ? null : performance },
            accessibility: { score: 1 },
            'best-practices': { score: 1 },
            seo: { score: 1 },
        },
        audits: {
            'largest-contentful-paint': error ? {} : { displayValue: '1 s' },
            'cumulative-layout-shift': error ? {} : { numericValue: 0 },
        },
    };
    if (error) lhr.runtimeError = error;
    return { lhr, report: ['<html></html>', JSON.stringify(lhr)] };
}

function loadAudit(outcomes) {
    const cases = [];
    const launches = [];
    const reports = new Map();
    const warnings = [];
    const hooks = {};
    let runs = 0;
    const dependencies = {
        'node:fs/promises': {
            async mkdir() {},
            async writeFile(filename, contents) { reports.set(filename, contents); },
        },
        chai,
        'chrome-launcher': {
            async launch() {
                assert.ok(launches.every((chrome) => chrome.killed));
                const chrome = {
                    port: 9000 + launches.length,
                    killed: false,
                    async kill() { this.killed = true; },
                };
                launches.push(chrome);
                return chrome;
            },
        },
        lighthouse: {
            async default(url, options) {
                assert.equal(url, 'http://localhost/ucblogo.html');
                assert.equal(options.port, launches.at(-1).port);
                const outcome = outcomes[runs++];
                assert.ok(outcome, 'unexpected extra Lighthouse attempt');
                if (outcome instanceof Error) throw outcome;
                return outcome;
            },
        },
        './application-server': {
            createApplicationServer: () => ({
                async start() { return 'http://localhost/ucblogo.html'; },
                async stop() {},
            }),
        },
    };
    vm.runInNewContext(source, {
        require: (name) => dependencies[name] || require(name),
        __dirname,
        process: { env: {} },
        console: { log() {}, warn(message) { warnings.push(message); } },
        describe(name, register) { register.call({ timeout() {} }); },
        before(callback) { hooks.before = callback; },
        after(callback) { hooks.after = callback; },
        it(name, callback) { cases.push(callback); },
    }, { filename: 'lighthouse.test.js' });
    return {
        launches,
        reports,
        warnings,
        async run() {
            await hooks.before();
            try {
                await cases[0]();
            } finally {
                await hooks.after();
            }
        },
    };
}

it('does not retry a successful Lighthouse audit', async () => {
    const audit = loadAudit([auditResult()]);
    await audit.run();
    assert.equal(audit.launches.length, 1);
    assert.ok(audit.launches[0].killed);
    assert.equal(audit.warnings.length, 0);
    assert.equal(audit.reports.size, 2);
});

it('retries Lighthouse runtime errors with fresh Chrome and retains failed reports', async () => {
    const audit = loadAudit([
        auditResult({ error: runtimeError }),
        auditResult({ error: runtimeError }),
        auditResult(),
    ]);
    await audit.run();
    assert.equal(audit.launches.length, 3);
    assert.ok(audit.launches.every((chrome) => chrome.killed));
    assert.equal(audit.warnings.length, 2);
    assert.ok(audit.warnings.every((message) => message.includes('NO_NAVSTART')));
    const profileDirectory = path.resolve(__dirname, '../../reports/lighthouse/mobile');
    for (const attempt of ['attempt-1', 'attempt-2']) {
        const report = audit.reports.get(path.join(profileDirectory, attempt, 'report.json'));
        assert.deepEqual(JSON.parse(report).runtimeError, runtimeError);
    }
    const report = audit.reports.get(path.join(profileDirectory, 'report.json'));
    assert.equal(JSON.parse(report).runtimeError, undefined);
});

it('reports the runtime error after three failed Lighthouse attempts', async () => {
    const failed = auditResult({ error: runtimeError });
    const audit = loadAudit([failed, failed, failed]);
    await assert.rejects(audit.run(), /NO_NAVSTART.*Trace has no navigation start/);
    assert.equal(audit.launches.length, 3);
    assert.ok(audit.launches.every((chrome) => chrome.killed));
    const reportPath = path.resolve(__dirname, '../../reports/lighthouse/mobile/report.json');
    assert.deepEqual(JSON.parse(audit.reports.get(reportPath)).runtimeError, runtimeError);
});

it('does not retry Lighthouse score failures', async () => {
    const audit = loadAudit([auditResult({ performance: 0.1 })]);
    await assert.rejects(audit.run(), /performance score/);
    assert.equal(audit.launches.length, 1);
    assert.ok(audit.launches[0].killed);
});

it('cleans up Chrome and propagates unexpected Lighthouse exceptions', async () => {
    const error = new Error('Chrome disconnected');
    const audit = loadAudit([error]);
    await assert.rejects(audit.run(), (actual) => actual === error);
    assert.equal(audit.launches.length, 1);
    assert.ok(audit.launches[0].killed);
});
