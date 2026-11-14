const assert = require('node:assert/strict');
const fs = require('node:fs');
const http = require('node:http');
const net = require('node:net');
const os = require('node:os');
const path = require('node:path');
const { spawn } = require('node:child_process');
const { once } = require('node:events');
const { Writable } = require('node:stream');
const { test } = require('node:test');
const { configure, discoverFixtures, capabilityFilter, validateRemoteHostname, run, preserveReporterStreams } = require('./run-testcafe');
const { createApplicationServer, resolveApplicationUrl } = require('./application-server');

const driver = path.join(__dirname, 'runner-self-test/driver.cjs');

function cleanEnv(overrides = {}) {
    const env = { ...process.env };
    for (const name of Object.keys(env)) {
        if (/^(TESTCAFE_|UCBOLOGO_|RUNNER_SELF_TEST_|BROWSERS$|HEADLESS$)/.test(name)) delete env[name];
    }
    return { ...env, ...overrides };
}

function launch(overrides, args = []) {
    const child = spawn(process.execPath, [driver, ...args], {
        env: cleanEnv(overrides), stdio: ['ignore', 'pipe', 'pipe'],
    });
    let output = '';
    child.stdout.on('data', (data) => { output += data; });
    child.stderr.on('data', (data) => { output += data; });
    const done = once(child, 'close').then(([code, signal]) => ({ code, signal, output }));
    return { child, done, output: () => output };
}

async function waitForOutput(session, pattern, timeoutMs = 15000) {
    const deadline = Date.now() + timeoutMs;
    while (!pattern.test(session.output())) {
        if (session.child.exitCode !== null || session.child.signalCode)
            assert.fail(`Child exited before ${pattern}: ${session.output()}`);
        if (Date.now() >= deadline) {
            session.child.kill('SIGTERM');
            assert.fail(`Timed out waiting for ${pattern}: ${session.output()}`);
        }
        await new Promise((resolve) => setTimeout(resolve, 25));
    }
}

function isRunning(pid) {
    try {
        process.kill(pid, 0);
        if (process.platform === 'linux') {
            const stat = fs.readFileSync(`/proc/${pid}/stat`, 'utf8');
            if (stat.slice(stat.lastIndexOf(')') + 2).startsWith('Z')) return false;
        }
        return true;
    } catch (error) {
        if (error.code === 'ESRCH' || error.code === 'ENOENT') return false;
        throw error;
    }
}

async function assertClean(output) {
    const pids = [...new Set([...output.matchAll(/(?:OWNED|DESCENDANT|TRACKED)_PID=(\d+)/g)]
        .map((match) => Number(match[1])))];
    assert.ok(pids.length, output);
    const deadline = Date.now() + 1500;
    while (pids.some(isRunning) && Date.now() < deadline)
        await new Promise((resolve) => setTimeout(resolve, 25));
    for (const pid of pids) assert.equal(isRunning(pid), false, `Owned process ${pid} survived: ${output}`);
    console.log(`Cleanup verified for PIDs ${pids.join(', ')}`);
}

async function serve(context, handler = (request, response) => response.end('<!doctype html><title>Runner self-test</title>')) {
    const server = http.createServer(handler);
    await new Promise((resolve) => server.listen(0, '127.0.0.1', resolve));
    context.after(() => {
        server.closeAllConnections();
        return new Promise((resolve) => server.close(resolve));
    });
    return `http://127.0.0.1:${server.address().port}/`;
}

for (const ended of [false, true]) {
    test(`reporter flush waits for large JSON with stream already ended=${ended}`, async () => {
        let release;
        let output = '';
        const stream = new Writable({
            write(chunk, encoding, callback) {
                release = () => { output += chunk.toString(); callback(); };
            },
        });
        class FakeReporter {
            dispose() { return Promise.resolve(); }
        }
        const waitForReporters = preserveReporterStreams(FakeReporter);
        const reporter = new FakeReporter();
        reporter.outStream = stream;
        const payload = JSON.stringify({ passed: 57, data: 'report-data'.repeat(1024 * 1024) });
        stream.write(payload);
        if (ended) stream.end();
        const disposed = reporter.dispose();
        assert.equal(reporter.dispose(), disposed);
        let complete = false;
        const flushed = waitForReporters().then(() => { complete = true; });
        await Promise.resolve();
        assert.equal(complete, false);
        assert.equal(stream.writableEnded, true);
        release();
        await Promise.all([disposed, flushed]);
        assert.equal(JSON.parse(output).data.length, 11 * 1024 * 1024);
        assert.equal(JSON.parse(output).passed, 57);
    });
}

test('defaults use headless Chrome, proxy automation, reports, and desktop capabilities', () => {
    const config = configure({}, []);
    assert.equal(config.browser, 'chrome:headless');
    assert.ok(config.args.includes('--disable-native-automation'));
    assert.ok(!config.args.includes('--headed'));
    assert.ok(config.args.includes('spec,json:reports/testcafe.json'));
    assert.deepEqual(config.capabilities, ['resize', 'screenshots']);
    assert.equal(config.watchdogMs, 300000);
    assert.equal(configure({ TESTCAFE_HEADLESS: '0' }, []).browser, 'chrome');
    assert.ok(!configure({}, ['--native-automation']).args.includes('--disable-native-automation'));
    assert.ok(!configure({ TESTCAFE_NATIVE_AUTOMATION: '1' }, []).args.includes('--disable-native-automation'));
});

for (const mode of ['report', 'report-error', 'report-hang']) {
    test(`fake child reporter flush ${mode} preserves completion and cleanup`, { timeout: 20000 }, async (context) => {
        const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'runner-report-'));
        context.after(() => fs.rmSync(directory, { recursive: true, force: true }));
        const reportPath = path.join(directory, 'report.json');
        const session = launch({ RUNNER_SELF_TEST_MODE: mode, RUNNER_SELF_TEST_REPORT: reportPath,
            TESTCAFE_WATCHDOG_MS: mode === 'report-hang' ? '3000' : '10000' });
        const result = await session.done;
        assert.equal(result.code, mode === 'report' ? 0 : mode === 'report-error' ? 1 : 124, result.output);
        if (mode === 'report') {
            const report = JSON.parse(fs.readFileSync(reportPath, 'utf8'));
            assert.equal(report.passed, 57);
            assert.equal(report.data.length, 11 * 1024 * 1024);
        }
        if (mode === 'report-error') assert.match(result.output, /TestCafe reporter flush failed:.*intentional reporter write failure/);
        if (mode === 'report-hang') assert.match(result.output, /whole-run watchdog/);
        await assertClean(result.output);
    });
}

test('legacy environment migrates with warnings, preferred names win', () => {
    const warnings = [];
    const config = configure({ BROWSERS: 'chrome,firefox', HEADLESS: '0' }, [], (message) => warnings.push(message));
    assert.equal(config.browser, 'chrome,firefox');
    assert.equal(warnings.length, 2);
    assert.equal(configure({ BROWSERS: 'firefox', TESTCAFE_BROWSER: 'chrome',
        HEADLESS: '0', TESTCAFE_HEADLESS: '1' }, [], () => {}).browser, 'chrome:headless');
});

test('URL aliases validate conflicts and protocols', () => {
    const warnings = [];
    assert.equal(resolveApplicationUrl({ UCBOLOGO_TEST_URL: 'http://localhost:9000' },
        (message) => warnings.push(message)), 'http://localhost:9000/');
    assert.equal(warnings.length, 1);
    assert.throws(() => resolveApplicationUrl({ TESTCAFE_APP_URL: 'http://localhost:1',
        UCBOLOGO_TEST_URL: 'http://localhost:2' }), /conflicts/);
    assert.throws(() => resolveApplicationUrl({ TESTCAFE_APP_URL: 'file:///tmp/app' }), /HTTP/);
});

test('forwarded filters keep exact values and lose only the separator', () => {
    const options = ['--', '--test', 'exact case', '--fixture', 'Exact fixture', '--test-grep', '^ACC-'];
    const config = configure({ TESTCAFE_TEST: 'ignored' }, options);
    assert.deepEqual(config.args.slice(-6), options.slice(1));
    assert.ok(!config.args.includes('--'));
    assert.equal(config.args.filter((argument) => argument === '--test').length, 1);
    assert.ok(configure({}, ['-r', 'json:custom.json']).args.includes('json:custom.json'));
    assert.ok(!configure({}, ['-r', 'json:custom.json']).args.includes('spec,json:reports/testcafe.json'));
});

test('invalid timeout values fail before startup instead of disabling timer bounds', () => {
    for (const value of ['0', '-1', 'NaN', '2147483648']) {
        assert.throws(() => configure({ TESTCAFE_WATCHDOG_MS: value }, []), /positive integer/);
        assert.throws(() => configure({ TESTCAFE_BROWSER_INIT_TIMEOUT: value }, []), /positive integer/);
        assert.throws(() => configure({}, ['--browser-init-timeout', value]), /positive integer/);
    }
    assert.throws(() => configure({}, ['--browser-init-timeout']), /positive integer/);
});

test('discovery includes suffix and legacy iPad, excludes self-test fixtures', () => {
    const fixtures = discoverFixtures();
    assert.ok(fixtures.some((file) => file.endsWith('ipad-keyboard.test.js')));
    assert.ok(fixtures.some((file) => file.endsWith('autocomplete.testcafe.js')));
    assert.deepEqual(fixtures, [...fixtures].sort());
    assert.ok(fixtures.every((file) => !file.includes('runner-self-test') && !file.endsWith('runner.test.js')));
});

test('remote prerequisites, ports, longer limits and explicit capability override', async () => {
    assert.throws(() => configure({ TESTCAFE_BROWSER: 'remote' }, []), /require/);
    const env = { TESTCAFE_BROWSER: 'remote', TESTCAFE_HOSTNAME: '192.168.1.10',
        TESTCAFE_APP_URL: 'http://192.168.1.10:8080/' };
    const config = configure(env, []);
    assert.ok(config.args.includes('38079,38080'));
    assert.ok(config.args.includes('300000'));
    assert.equal(config.watchdogMs, 900000);
    assert.equal(config.connectionTimeoutMs, 300000);
    assert.equal(configure(env, ['--browser-init-timeout', '700']).connectionTimeoutMs, 700);
    assert.deepEqual(config.capabilities, []);
    assert.deepEqual(configure({ ...env, TESTCAFE_CAPABILITIES: 'resize,screenshots' }, []).capabilities,
        ['resize', 'screenshots']);
    assert.throws(() => configure({ ...env, TESTCAFE_PORTS: '80,80' }, []), /distinct ports/);
    assert.throws(() => configure(env, ['--hostname', '127.0.0.1']), /conflicts/);
    assert.throws(() => configure(env, ['--ports', '1,2']), /conflicts/);
    for (const address of ['127.0.0.1', '::1', '0.0.0.0', '::ffff:127.0.0.1', '::ffff:7f00:1', 'ff02::1'])
        await assert.rejects(validateRemoteHostname('host', async () => [{ address }]), /LAN-reachable/);
    await validateRemoteHostname('host', async () => [{ address: '192.168.1.10' }]);
    assert.deepEqual(configure({ TESTCAFE_BROWSER: 'chrome:emulation:device=iPhone X' }, []).capabilities, []);
});

test('capability filter preserves shared cases and combines test/fixture requirements and name filters', () => {
    const remote = capabilityFilter([], (name) => name === 'selected');
    assert.equal(remote('selected', '', '', {}), true);
    assert.equal(remote('other', '', '', {}), false);
    assert.equal(remote('selected', '', '', { requires: 'resize' }), false);
    assert.equal(remote('selected', '', '', {}, { requires: 'resize' }), false);
    assert.equal(capabilityFilter(['resize', 'screenshots'])('', '', '', { requires: 'resize,screenshots' }), true);
});

test('supplied URL readiness succeeds or fails within its bound without owning the server', async (context) => {
    const url = await serve(context);
    const ready = createApplicationServer({ env: { TESTCAFE_APP_URL: url } });
    assert.equal(await ready.start(), url);
    await ready.stop();
    assert.equal((await fetch(url)).status, 200);
    const hangingUrl = await serve(context, () => {});
    const unavailable = createApplicationServer({ env: { TESTCAFE_APP_URL: hangingUrl },
        startupTimeoutMs: 180, fetchTimeoutMs: 50 });
    const started = Date.now();
    await assert.rejects(unavailable.start(), /Timed out waiting/);
    await unavailable.stop();
    assert.ok(Date.now() - started < 1500);
});

test('whole-run watchdog starts before server readiness', async () => {
    let stopped = false;
    const started = Date.now();
    assert.equal(await run({ env: { TESTCAFE_WATCHDOG_MS: '100' }, args: [],
        server: { start: () => new Promise(() => {}), stop: async () => { stopped = true; } } }), 124);
    assert.equal(stopped, true);
    assert.ok(Date.now() - started < 1500);
});

test('whole-run watchdog stops an owned server tree before readiness', { timeout: 10000 }, async () => {
    const session = launch({ RUNNER_SELF_TEST_MODE: 'owned-server', EMRUN: 'runner-self-test-emrun',
        TESTCAFE_WATCHDOG_MS: '1000' });
    const result = await session.done;
    assert.equal(result.code, 124, result.output);
    assert.match(result.output, /CHILD_READY=/);
    await assertClean(result.output);
});

for (const mode of ['exit', 'success', 'hang']) {
    test(`fake child ${mode}: preserves status and cleans owned process group`, { timeout: 20000 }, async () => {
        const session = launch({ RUNNER_SELF_TEST_MODE: mode, TESTCAFE_WATCHDOG_MS: '5000' });
        const result = await session.done;
        assert.equal(result.code, mode === 'exit' ? 7 : mode === 'success' ? 0 : 124, result.output);
        assert.match(result.output, /DESCENDANT_PID=/);
        await assertClean(result.output);
    });
}

for (const [signal, code] of [['SIGINT', 130], ['SIGTERM', 143]]) {
    test(`${signal} cleans only task-owned trees`, { timeout: 20000, skip: process.platform === 'win32' }, async (context) => {
        const unrelated = spawn(process.execPath, ['-e', 'setInterval(() => {}, 1000)'], { stdio: 'ignore' });
        context.after(() => unrelated.kill());
        const session = launch({ RUNNER_SELF_TEST_MODE: 'hang', TESTCAFE_WATCHDOG_MS: '10000' });
        await waitForOutput(session, /CHILD_READY=/);
        session.child.kill(signal);
        const result = await session.done;
        assert.equal(result.code, code, result.output);
        await assertClean(result.output);
        assert.equal(isRunning(unrelated.pid), true, 'An unrelated process was killed');
    });
}

test('real TestCafe invalid browser startup returns nonzero and cleans up', { timeout: 30000 }, async (context) => {
    const url = await serve(context);
    const session = launch({ RUNNER_SELF_TEST_MODE: 'real', TESTCAFE_APP_URL: url,
        TESTCAFE_BROWSER: 'intentionally-invalid-runner-browser', TESTCAFE_WATCHDOG_MS: '20000' },
    ['--test', 'shared capability case']);
    const result = await session.done;
    assert.equal(result.code, 1, result.output);
    assert.match(result.output, /Cannot find|not found|not installed|not a browser/i);
    await assertClean(result.output);
});

test('real remote connection timeout is separate from the whole-run watchdog', { timeout: 30000 }, async (context) => {
    const address = Object.values(os.networkInterfaces()).flat().find((entry) =>
        entry.family === 'IPv4' && !entry.internal)?.address;
    if (!address) return context.skip('No LAN interface available for a remote connection check');
    const reservations = [net.createServer(), net.createServer()];
    await Promise.all(reservations.map((server) => new Promise((resolve) => server.listen(0, address, resolve))));
    const ports = reservations.map((server) => server.address().port).join(',');
    await Promise.all(reservations.map((server) => new Promise((resolve) => server.close(resolve))));
    const url = await serve(context);
    const session = launch({ RUNNER_SELF_TEST_MODE: 'real', TESTCAFE_APP_URL: url,
        TESTCAFE_BROWSER: 'remote', TESTCAFE_HOSTNAME: address, TESTCAFE_PORTS: ports,
        TESTCAFE_BROWSER_INIT_TIMEOUT: '600', TESTCAFE_WATCHDOG_MS: '20000' },
    ['--test', 'shared capability case']);
    const result = await session.done;
    assert.equal(result.code, 1, result.output);
    assert.match(result.output, /Remote browser connection exceeded 600ms/);
    assert.match(result.output, /TESTCAFE_HOSTNAME=<LAN address>/);
    assert.doesNotMatch(result.output, /whole-run watchdog/);
    await assertClean(result.output);
});

test('real Chrome runner reporter flush completes large JSON before parent cleanup', {
    timeout: 45000, skip: process.env.RUNNER_BROWSER_TESTS !== '1',
}, async (context) => {
    const directory = fs.mkdtempSync(path.join(os.tmpdir(), 'runner-browser-report-'));
    context.after(() => fs.rmSync(directory, { recursive: true, force: true }));
    const reportPath = path.join(directory, 'report.json');
    const url = await serve(context);
    const session = launch({ RUNNER_SELF_TEST_MODE: 'real', TESTCAFE_APP_URL: url,
        TESTCAFE_WATCHDOG_MS: '30000' },
    ['--test', 'large JSON report', '--reporter', `spec,json:${reportPath}`]);
    const result = await session.done;
    assert.equal(result.code, 0, result.output);
    const report = JSON.parse(fs.readFileSync(reportPath, 'utf8'));
    assert.equal(report.passed, 1);
    assert.equal(report.total, 1);
    assert.equal(report.fixtures[0].tests[0].meta.payload.length, 11 * 1024 * 1024);
    await assertClean(result.output);
});

for (const outcome of ['assertion', 'hang', 'capabilities']) {
    test(`real Chrome runner ${outcome}`, { timeout: 45000, skip: process.env.RUNNER_BROWSER_TESTS !== '1' }, async (context) => {
        const url = await serve(context);
        const selected = outcome === 'assertion' ? 'intentional assertion failure' : 'hung browser action';
        const args = outcome === 'capabilities' ? ['--test-grep', 'shared capability case|requires resize'] : ['--test', selected];
        const session = launch({ RUNNER_SELF_TEST_MODE: 'real', TESTCAFE_APP_URL: url,
            TESTCAFE_CAPABILITIES: '', TESTCAFE_WATCHDOG_MS: outcome === 'hang' ? '15000' : '30000' }, args);
        const result = await session.done;
        assert.equal(result.code, outcome === 'assertion' ? 1 : outcome === 'hang' ? 124 : 0, result.output);
        if (outcome === 'assertion') assert.match(result.output, /intentional runner self-test failure/);
        if (outcome === 'hang') {
            assert.match(result.output, /HUNG_ACTION_STARTED/);
            if (process.platform === 'linux') assert.match(result.output, /BROWSER_PROCESS/);
        }
        if (outcome === 'capabilities') assert.doesNotMatch(result.output, /resize case should have been filtered/);
        await assertClean(result.output);
        console.log(`Real Chrome ${outcome}: exit ${result.code}`);
    });
}