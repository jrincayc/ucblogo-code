#!/usr/bin/env node

const fs = require('node:fs');
const path = require('node:path');
const dns = require('node:dns/promises');
const net = require('node:net');
const { finished } = require('node:stream/promises');
const { createApplicationServer, resolveApplicationUrl, createProcessSupervisor } = require('./application-server');

const repositoryRoot = path.resolve(__dirname, '../..');

function discoverFixtures(directory = __dirname) {
    return fs.readdirSync(directory, { withFileTypes: true })
        .filter((entry) => entry.isFile() &&
            (entry.name.endsWith('.testcafe.js') || entry.name === 'ipad-keyboard.test.js'))
        .map((entry) => path.join(directory, entry.name)).sort();
}

function positiveInteger(value, name, fallback) {
    if (value === undefined && fallback !== undefined) return fallback;
    const number = Number(value);
    if (!Number.isSafeInteger(number) || number <= 0 || number > 2147483647)
        throw new Error(`${name} must be a positive integer no greater than 2147483647`);
    return number;
}

function capabilityFilter(capabilities, filter = () => true, excluded = () => {}) {
    const available = new Set(capabilities);
    return (testName, fixtureName, fixturePath, testMeta = {}, fixtureMeta = {}) => {
        const required = [testMeta.requires, fixtureMeta.requires].filter(Boolean)
            .flatMap((value) => String(value).split(/[\s,]+/).filter(Boolean));
        if (!filter(testName, fixtureName, fixturePath, testMeta, fixtureMeta)) return false;
        const missing = required.filter((name) => !available.has(name));
        if (missing.length) excluded({ testName, fixtureName, missing });
        return missing.length === 0;
    };
}

function configure(env = process.env, argv = process.argv.slice(2), warn = console.warn) {
    const args = argv.filter((argument) => argument !== '--');
    function takeOption(name) {
        const index = args.findIndex((argument) => argument === name || argument.startsWith(`${name}=`));
        if (index < 0) return undefined;
        const equal = args[index].indexOf('=');
        const value = equal < 0 ? args[index + 1] : args[index].slice(equal + 1);
        if (!value || value.startsWith('--')) throw new Error(`${name} requires a value`);
        args.splice(index, equal < 0 ? 2 : 1);
        return value;
    }
    function migrate(preferred, legacy) {
        if (env[legacy] !== undefined) warn(`${legacy} is deprecated; use ${preferred}`);
        return env[preferred] ?? env[legacy];
    }
    const browserOption = takeOption('--browser');
    const migratedBrowser = migrate('TESTCAFE_BROWSER', 'BROWSERS');
    let browser = env.TESTCAFE_BROWSER || browserOption || migratedBrowser || 'chrome';
    const headless = migrate('TESTCAFE_HEADLESS', 'HEADLESS') ?? '1';
    if (!['0', '1'].includes(headless)) throw new Error('TESTCAFE_HEADLESS must be 0 or 1');
    browser = browser.split(',').map((name) => {
        const alias = name.trim();
        if (!alias) throw new Error('TESTCAFE_BROWSER must not contain empty aliases');
        return headless === '1' && /^(chrome|chromium|firefox|edge)$/.test(alias)
            ? `${alias}:headless` : alias;
    }).join(',');
    const remote = browser.split(',').some((alias) => /^remote(?::\d+)?$/.test(alias));
    const mobile = remote || /(?:emulation|mobile=true|touch=true)/i.test(browser);
    const suppliedUrl = resolveApplicationUrl(env, warn);
    if (remote && (!env.TESTCAFE_HOSTNAME || !suppliedUrl))
        throw new Error('Remote browsers require TESTCAFE_HOSTNAME (LAN reachable) and TESTCAFE_APP_URL');
    const ports = env.TESTCAFE_PORTS || (remote ? '38079,38080' : undefined);
    if (ports && (!/^\d+,\d+$/.test(ports) || new Set(ports.split(',').map(Number)).size !== 2 ||
        ports.split(',').some((port) => Number(port) < 1 || Number(port) > 65535)))
        throw new Error('TESTCAFE_PORTS requires two distinct ports in 1..65535');
    if (remote) {
        const hostnameOption = takeOption('--hostname');
        const portsOption = takeOption('--ports');
        if (hostnameOption !== undefined && hostnameOption !== env.TESTCAFE_HOSTNAME)
            throw new Error('--hostname conflicts with required TESTCAFE_HOSTNAME');
        if (portsOption !== undefined && portsOption !== ports)
            throw new Error('--ports conflicts with TESTCAFE_PORTS; configure remote ports in the environment');
    }
    const capabilities = (env.TESTCAFE_CAPABILITIES ?? (mobile ? '' : 'resize,screenshots'))
        .split(',').map((name) => name.trim()).filter(Boolean);
    const nativeIndex = args.indexOf('--native-automation');
    if (nativeIndex >= 0) args.splice(nativeIndex, 1);
    const nativeEnv = env.TESTCAFE_NATIVE_AUTOMATION;
    if (nativeEnv !== undefined && !['0', '1'].includes(nativeEnv))
        throw new Error('TESTCAFE_NATIVE_AUTOMATION must be 0 or 1');
    const nativeAutomation = nativeIndex >= 0 || nativeEnv === '1';
    if (nativeAutomation && (mobile || args.includes('--disable-native-automation')))
        throw new Error('Native automation conflicts with remote/mobile or --disable-native-automation');
    const defaults = [];
    const hasOption = (...names) => args.some((argument) => names.some((name) =>
        argument === name || argument.startsWith(`${name}=`)));
    const addDefault = (name, value, ...aliases) => {
        if (value !== undefined && !hasOption(name, ...aliases)) defaults.push(name, String(value));
    };
    if (!nativeAutomation && !hasOption('--disable-native-automation')) defaults.push('--disable-native-automation');
    addDefault('--test', env.TESTCAFE_TEST, '-t');
    addDefault('--assertion-timeout', env.TESTCAFE_ASSERTION_TIMEOUT || '10000');
    addDefault('--selector-timeout', env.TESTCAFE_SELECTOR_TIMEOUT || '10000');
    addDefault('--browser-init-timeout', positiveInteger(env.TESTCAFE_BROWSER_INIT_TIMEOUT,
        'TESTCAFE_BROWSER_INIT_TIMEOUT', remote ? 300000 : 30000));
    addDefault('--speed', env.TESTCAFE_SPEED);
    addDefault('--hostname', env.TESTCAFE_HOSTNAME);
    addDefault('--ports', ports);
    addDefault('--reporter', 'spec,json:reports/testcafe.json', '-r');
    if (capabilities.includes('screenshots'))
        addDefault('--screenshots', 'takeOnFails=true,pathPattern=failures/${FIXTURE}/${TEST}/${USERAGENT}-${FILE_INDEX}.png');
    const forwarded = [...defaults, ...args];
    const initIndex = forwarded.findLastIndex((argument) =>
        argument === '--browser-init-timeout' || argument.startsWith('--browser-init-timeout='));
    const connectionTimeoutMs = positiveInteger(forwarded[initIndex].includes('=')
        ? forwarded[initIndex].split('=')[1] : forwarded[initIndex + 1], '--browser-init-timeout');
    return {
        browser, remote, mobile, capabilities, suppliedUrl, hostname: env.TESTCAFE_HOSTNAME,
        watchdogMs: positiveInteger(env.TESTCAFE_WATCHDOG_MS, 'TESTCAFE_WATCHDOG_MS', remote ? 900000 : 300000),
        connectionTimeoutMs, args: forwarded,
    };
}

async function validateRemoteHostname(hostname, lookup = dns.lookup) {
    const blocked = new net.BlockList();
    blocked.addSubnet('127.0.0.0', 8, 'ipv4');
    blocked.addSubnet('0.0.0.0', 8, 'ipv4');
    blocked.addSubnet('169.254.0.0', 16, 'ipv4');
    blocked.addSubnet('224.0.0.0', 4, 'ipv4');
    blocked.addAddress('255.255.255.255', 'ipv4');
    blocked.addAddress('::', 'ipv6');
    blocked.addAddress('::1', 'ipv6');
    blocked.addSubnet('fe80::', 10, 'ipv6');
    blocked.addSubnet('ff00::', 8, 'ipv6');
    const addresses = await lookup(hostname, { all: true });
    if (!addresses.length || addresses.some(({ address }) =>
        !net.isIP(address) || blocked.check(address, net.isIP(address) === 6 ? 'ipv6' : 'ipv4')))
        throw new Error('TESTCAFE_HOSTNAME must resolve to a LAN-reachable address, not loopback/unspecified/link-local');
}

async function run(options = {}) {
    const env = { ...(options.env || process.env) };
    const config = configure(env, options.args || process.argv.slice(2), options.warn);
    const supervisor = options.supervisor || createProcessSupervisor();
    const abort = new AbortController();
    let exitCode;
    let cancel;
    const cancelled = new Promise((resolve) => { cancel = resolve; });
    const terminate = (code, message) => {
        if (exitCode !== undefined) return;
        exitCode = code;
        if (message) console.error(message);
        abort.abort(new Error(message || `Runner interrupted (${code})`));
        cancel(code);
    };
    const interrupt = () => terminate(130, 'TestCafe runner received SIGINT');
    const stop = () => terminate(143, 'TestCafe runner received SIGTERM');
    process.on('SIGINT', interrupt);
    process.on('SIGTERM', stop);
    const watchdog = setTimeout(() => terminate(124,
        `TestCafe exceeded the ${config.watchdogMs}ms whole-run watchdog`), config.watchdogMs);
    const server = options.server || createApplicationServer({
        env: { ...env, TESTCAFE_APP_URL: config.suppliedUrl, UCBOLOGO_TEST_URL: undefined },
        supervisor, signal: abort.signal,
    });
    const execution = (async () => {
        if (config.remote) {
            await validateRemoteHostname(config.hostname);
            console.log('Remote session: set TESTCAFE_BROWSER=remote TESTCAFE_HOSTNAME=<LAN address> ' +
                'TESTCAFE_APP_URL=<reachable app URL> TESTCAFE_PORTS=38079,38080. ' +
                'Open the connection URL printed by TestCafe on the device; allow both proxy ports through the firewall. ' +
                'TESTCAFE_BROWSER_INIT_TIMEOUT controls connection waiting; TESTCAFE_WATCHDOG_MS bounds the whole run.');
        }
        abort.signal.throwIfAborted();
        const fixtures = options.fixtures || discoverFixtures();
        if (!fixtures.length) throw new Error('No TestCafe fixtures were found');
        const applicationUrl = await server.start();
        abort.signal.throwIfAborted();
        fs.mkdirSync(path.join(repositoryRoot, 'reports'), { recursive: true });
        const child = supervisor.spawn(process.execPath, [
            '--require', __filename, options.testCafeBin || require.resolve('testcafe/bin/testcafe.js'),
            config.browser, ...fixtures, ...config.args,
        ], {
            cwd: repositoryRoot, stdio: [...(options.stdio || ['inherit', 'inherit', 'inherit']), 'ipc'],
            env: { ...env, TESTCAFE_APP_URL: applicationUrl,
                UCBOLOGO_RUNNER_CONNECTION_TIMEOUT: String(config.connectionTimeoutMs),
                UCBOLOGO_RUNNER_CHILD_CAPABILITIES: JSON.stringify(config.capabilities) },
        });
        return new Promise((resolve, reject) => {
            child.once('error', reject);
            child.on('message', (message) => {
                if (message?.type === 'testcafe-exit' && Number.isInteger(message.code)) resolve(message.code);
            });
            child.once('exit', (code, signal) => resolve(signal ? 1 : code ?? 1));
        });
    })();
    let result;
    try {
        result = await Promise.race([execution, cancelled]);
    } finally {
        try {
            await supervisor.stop();
            await server.stop();
        } finally {
            clearTimeout(watchdog);
            process.removeListener('SIGINT', interrupt);
            process.removeListener('SIGTERM', stop);
        }
    }
    return exitCode ?? result;
}

function preserveReporterStreams(Reporter) {
    const pending = new Set();
    const disposals = new WeakMap();
    const dispose = Reporter.prototype.dispose;
    Reporter.prototype.dispose = function (...args) {
        if (disposals.has(this)) return disposals.get(this);
        const stream = this.outStream;
        const owned = stream && stream !== process.stdout && stream !== process.stderr &&
            !stream.isTTY && typeof stream.end === 'function';
        const flushed = owned ? finished(stream, { readable: false, cleanup: true }) : Promise.resolve();
        flushed.catch(() => {});
        const completion = (async () => {
            const disposed = dispose.apply(this, args);
            if (owned && !stream.writableEnded && !stream.destroyed) stream.end();
            await Promise.all([disposed, flushed]);
        })();
        disposals.set(this, completion);
        pending.add(completion);
        completion.catch(() => {});
        return completion;
    };
    return () => Promise.all(pending);
}

if (process.env.UCBOLOGO_RUNNER_CHILD_CAPABILITIES !== undefined && require.main !== module) {
    const wizardPath = require.resolve('testcafe/lib/cli/remotes-wizard');
    const connectRemotes = require(wizardPath);
    require.cache[wizardPath].exports = async (testCafe, remoteCount, ...args) => {
        if (!remoteCount) return connectRemotes(testCafe, remoteCount, ...args);
        const timeoutMs = Number(process.env.UCBOLOGO_RUNNER_CONNECTION_TIMEOUT);
        let timer;
        try {
            return await Promise.race([
                connectRemotes(testCafe, remoteCount, ...args),
                new Promise((resolve, reject) => {
                    timer = setTimeout(() => reject(new Error(
                        `Remote browser connection exceeded ${timeoutMs}ms browser-init timeout`)), timeoutMs);
                }),
            ]);
        } finally {
            clearTimeout(timer);
        }
    };
    const Runner = require('testcafe/lib/runner');
    const filter = Runner.prototype.filter;
    Runner.prototype.filter = function (predicate) {
        return filter.call(this, capabilityFilter(
            JSON.parse(process.env.UCBOLOGO_RUNNER_CHILD_CAPABILITIES), predicate,
            ({ testName, fixtureName, missing }) => console.log(
                `FILTERED (${missing.join(', ')} unavailable): ${fixtureName}: ${testName}`,
            )));
    };
    if (process.send) {
        const waitForReporters = preserveReporterStreams(require('testcafe/lib/reporter'));
        let exiting = false;
        process.exit = (code = process.exitCode || 0) => {
            if (exiting) return;
            exiting = true;
            setInterval(() => {}, 1000);
            waitForReporters().then(() => {
                process.send({ type: 'testcafe-exit', code });
            }, (error) => {
                console.error('TestCafe reporter flush failed:', error);
                process.send({ type: 'testcafe-exit', code: code || 1 });
            });
        };
    }
}

if (require.main === module) {
    run().then((code) => { process.exitCode = code; }).catch((error) => {
        console.error(error);
        process.exitCode = 1;
    });
}

module.exports = { discoverFixtures, configure, capabilityFilter, validateRemoteHostname, run, preserveReporterStreams };
