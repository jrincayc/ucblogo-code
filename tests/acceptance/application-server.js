const { spawn } = require('node:child_process');
const net = require('node:net');
const path = require('node:path');

const repositoryRoot = path.resolve(__dirname, '../..');

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

function createApplicationServer() {
    let serverProcess;

    return {
        async start() {
            if (process.env.UCBOLOGO_TEST_URL) return process.env.UCBOLOGO_TEST_URL;

            const port = await findOpenPort();
            const url = `http://127.0.0.1:${port}/ucblogo.html`;
            let launchError;
            serverProcess = spawn(process.env.EMRUN || 'emrun', [
                '--no-browser', '--port', String(port), 'ucblogo.html',
            ], { cwd: repositoryRoot, stdio: 'inherit' });
            serverProcess.once('error', (error) => { launchError = error; });

            const deadline = Date.now() + 30000;
            while (Date.now() < deadline) {
                if (launchError) throw launchError;
                if (serverProcess.exitCode !== null)
                    throw new Error(`emrun exited with code ${serverProcess.exitCode}`);
                try {
                    const response = await fetch(url);
                    if (response.ok) return url;
                } catch (error) {}
                await new Promise((resolve) => setTimeout(resolve, 200));
            }
            throw new Error(`Timed out waiting for emrun at ${url}`);
        },

        async stop() {
            if (!serverProcess || serverProcess.exitCode !== null) return;
            const exited = new Promise((resolve) => serverProcess.once('exit', resolve));
            serverProcess.kill('SIGTERM');
            await Promise.race([exited, new Promise((resolve) => setTimeout(resolve, 3000))]);
        },
    };
}

module.exports = { createApplicationServer };