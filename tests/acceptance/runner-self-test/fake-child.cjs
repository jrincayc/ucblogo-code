const { spawn } = require('node:child_process');

const mode = process.env.RUNNER_SELF_TEST_MODE;
if (mode === 'leaf') {
    process.on('SIGTERM', () => {});
    setInterval(() => {}, 1000);
} else {
    const descendant = spawn(process.execPath, [__filename], {
        env: { ...process.env, RUNNER_SELF_TEST_MODE: 'leaf' }, stdio: 'ignore',
    });
    console.log(`DESCENDANT_PID=${descendant.pid}`);
    console.log(`CHILD_READY=${process.pid}`);
    if (mode?.startsWith('report')) {
        const fs = require('node:fs');
        const Reporter = require('testcafe/lib/reporter');
        const stream = fs.createWriteStream(process.env.RUNNER_SELF_TEST_REPORT);
        const write = stream._write;
        stream._write = function (...args) {
            if (mode === 'report-hang') return;
            if (mode === 'report-error') {
                setImmediate(() => args.at(-1)(new Error('intentional reporter write failure')));
                return;
            }
            setImmediate(() => write.apply(this, args));
        };
        const reporter = Object.create(Reporter.prototype);
        reporter.outStream = stream;
        reporter.disposed = false;
        const payload = JSON.stringify({ passed: 57, data: 'report-data'.repeat(1024 * 1024) });
        stream.end(payload);
        reporter.dispose();
        process.exit(0);
    }
    if (mode === 'exit') process.exit(7);
    if (mode === 'success') process.exit(0);
    process.on('SIGTERM', () => {});
    setInterval(() => {}, 1000);
}