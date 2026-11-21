const path = require('node:path');
const fs = require('node:fs');
const { run } = require('../run-testcafe');
const { createProcessSupervisor } = require('../application-server');

const supervisor = createProcessSupervisor({ graceMs: 100 });
const launch = supervisor.spawn;
const roots = new Set();
const tracked = new Set();
supervisor.spawn = (...args) => {
    if (args[0] === 'runner-self-test-emrun') {
        args[0] = process.execPath;
        args[1] = [path.join(__dirname, 'fake-child.cjs')];
        args[2].env.RUNNER_SELF_TEST_MODE = 'hang';
    }
    const child = launch(...args);
    if (child.pid) {
        roots.add(child.pid);
        console.log(`OWNED_PID=${child.pid}`);
    }
    return child;
};
const monitor = process.platform === 'linux' ? setInterval(() => {
    const processes = fs.readdirSync('/proc').filter((name) => /^\d+$/.test(name)).flatMap((name) => {
        try {
            const stat = fs.readFileSync(`/proc/${name}/stat`, 'utf8');
            const fields = stat.slice(stat.lastIndexOf(')') + 2).split(' ');
            return [{ pid: Number(name), parent: Number(fields[1]), group: Number(fields[2]), command: stat }];
        } catch (error) {
            if (error.code === 'ENOENT' || error.code === 'ESRCH') return [];
            throw error;
        }
    });
    let found;
    do {
        found = false;
        for (const entry of processes) {
            if (!tracked.has(entry.pid) && (roots.has(entry.group) || roots.has(entry.parent) || tracked.has(entry.parent))) {
                tracked.add(entry.pid);
                console.log(`TRACKED_PID=${entry.pid}${entry.command.includes('(chrome') ? ' BROWSER_PROCESS' : ''}` +
                    ` GROUP=${entry.group} ${entry.command.slice(0, entry.command.lastIndexOf(')') + 1)}`);
                found = true;
            }
        }
    } while (found);
}, 50) : undefined;
const options = {
    supervisor,
    fixtures: [path.join(__dirname, 'outcomes.js')],
};
if (process.env.RUNNER_SELF_TEST_MODE !== 'real') {
    options.testCafeBin = path.join(__dirname, 'fake-child.cjs');
    if (!['readiness', 'owned-server'].includes(process.env.RUNNER_SELF_TEST_MODE))
        options.server = { start: async () => 'http://127.0.0.1/', stop: async () => {} };
}
run(options).then((code) => { process.exitCode = code; }).catch((error) => {
    console.error(error);
    process.exitCode = 1;
}).finally(() => clearInterval(monitor));