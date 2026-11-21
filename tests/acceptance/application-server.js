const { spawn } = require('node:child_process');
const net = require('node:net');
const path = require('node:path');

const repositoryRoot = path.resolve(__dirname, '../..');

function createProcessSupervisor({ graceMs = 500 } = {}) {
    const owned = new Set();
    let stopping;
    let closed = false;

    function launch(command, args, options = {}) {
        if (closed) throw new Error('Process supervisor is stopping');
        const child = spawn(command, args, { ...options, detached: process.platform !== 'win32' });
        child.once('error', () => {});
        owned.add(child);
        return child;
    }

    function signalGroup(child, signal) {
        if (!child.pid) return;
        try {
            process.kill(-child.pid, signal);
        } catch (error) {
            if (error.code !== 'ESRCH') throw error;
        }
    }

    async function stop() {
        if (stopping) return stopping;
        closed = true;
        stopping = (async () => {
            if (process.platform === 'win32') {
                await Promise.all([...owned].filter((child) => child.pid).map((child) =>
                    new Promise((resolve, reject) => {
                        const killer = spawn('taskkill', ['/PID', String(child.pid), '/T', '/F'], {
                            stdio: 'ignore', windowsHide: true,
                        });
                        killer.once('error', reject);
                        killer.once('exit', (code) => code === 0 || code === 128
                            ? resolve() : reject(new Error(`taskkill failed with code ${code}`)));
                    })));
            } else {
                for (const child of owned) signalGroup(child, 'SIGTERM');
                if (owned.size) await new Promise((resolve) => setTimeout(resolve, graceMs));
                for (const child of owned) signalGroup(child, 'SIGKILL');
            }
            await Promise.all([...owned].filter((child) => child.pid &&
                child.exitCode === null && child.signalCode === null).map((child) =>
                new Promise((resolve) => child.once('exit', resolve))));
            owned.clear();
        })();
        return stopping;
    }

    return { spawn: launch, stop };
}

function resolveApplicationUrl(env = process.env, warn = console.warn) {
    const preferred = env.TESTCAFE_APP_URL;
    const legacy = env.UCBOLOGO_TEST_URL;
    if (preferred && legacy && new URL(preferred).href !== new URL(legacy).href)
        throw new Error('TESTCAFE_APP_URL conflicts with deprecated UCBOLOGO_TEST_URL');
    if (legacy) warn('UCBOLOGO_TEST_URL is deprecated; use TESTCAFE_APP_URL');
    const supplied = preferred || legacy;
    if (!supplied) return undefined;
    const url = new URL(supplied);
    if (!['http:', 'https:'].includes(url.protocol))
        throw new Error('TESTCAFE_APP_URL must be an HTTP(S) URL');
    return url.href;
}

function findOpenPort() {
    return new Promise((resolve, reject) => {
        const server = net.createServer();
        server.once('error', reject);
        server.listen(0, '127.0.0.1', () => {
            const { port } = server.address();
            server.close((error) => error ? reject(error) : resolve(port));
        });
    });
}

function createApplicationServer(options = {}) {
    const env = options.env || process.env;
    const supervisor = options.supervisor || createProcessSupervisor();
    let serverProcess;

    return {
        async start() {
            const suppliedUrl = resolveApplicationUrl(env);
            const port = suppliedUrl ? undefined : await findOpenPort();
            const url = suppliedUrl || `http://127.0.0.1:${port}/ucblogo.html`;
            let launchError;
            if (!suppliedUrl) {
                serverProcess = supervisor.spawn(env.EMRUN || 'emrun', [
                    '--no-browser', '--port', String(port), 'ucblogo.html',
                ], { cwd: repositoryRoot, stdio: 'inherit', env });
                serverProcess.once('error', (error) => { launchError = error; });
            }

            const deadline = Date.now() + (options.startupTimeoutMs || 30000);
            while (Date.now() < deadline) {
                options.signal?.throwIfAborted();
                if (launchError) throw launchError;
                if (serverProcess && (serverProcess.exitCode !== null || serverProcess.signalCode))
                    throw new Error(`emrun exited with code ${serverProcess.exitCode}`);
                try {
                    const requestTimeout = AbortSignal.timeout(Math.max(1,
                        Math.min(options.fetchTimeoutMs || 2000, deadline - Date.now())));
                    const response = await fetch(url, {
                        signal: options.signal
                            ? AbortSignal.any([options.signal, requestTimeout]) : requestTimeout,
                    });
                    await response.body?.cancel();
                    if (response.ok) return url;
                } catch (error) {}
                await new Promise((resolve) => setTimeout(resolve, Math.max(0, Math.min(200, deadline - Date.now()))));
            }
            throw new Error(`Timed out waiting for application at ${url}`);
        },

        stop: () => supervisor.stop(),
    };
}

module.exports = { createApplicationServer, resolveApplicationUrl, createProcessSupervisor };