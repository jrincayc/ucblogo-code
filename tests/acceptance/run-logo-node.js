#!/usr/bin/env node

const path = require('node:path');
const { spawn } = require('node:child_process');
const {
    gradeLogoOutput,
    formatLogoGrade,
    allowKnownLogoFailures,
} = require('./logo-results');

const repositoryRoot = path.resolve(__dirname, '../..');

function gradeLogoProcess({ stdout, stderr, code, signal, error, timedOut }) {
    const grade = allowKnownLogoFailures(gradeLogoOutput(stdout));
    const stderrGrade = gradeLogoOutput(stderr);
    grade.problems.push(...stderrGrade.problems.filter((problem) => problem.startsWith('Runtime crash')));
    if (error) grade.problems.push(`Logo process error: ${error.message}`);
    if (timedOut) grade.problems.push('Logo process timed out');
    if (signal) grade.problems.push(`Logo process terminated by ${signal}`);
    if (code !== 0) grade.problems.push(`Logo process exit code: ${code}`);
    grade.ok = grade.problems.length === 0;
    return grade;
}

function runLogoNode({
    executable = process.execPath,
    args = [path.join(repositoryRoot, 'ucblogo-node.js'), 'tests/test.lg'],
    timeoutMs = 240000,
} = {}) {
    return new Promise((resolve) => {
        const child = spawn(executable, args, {
            cwd: repositoryRoot,
            stdio: ['ignore', 'pipe', 'pipe'],
        });
        const result = { stdout: '', stderr: '', code: null, signal: null, error: null, timedOut: false };
        child.stdout.setEncoding('utf8');
        child.stderr.setEncoding('utf8');
        child.stdout.on('data', (text) => { result.stdout += text; });
        child.stderr.on('data', (text) => { result.stderr += text; });
        child.once('error', (error) => { result.error = error; });
        const watchdog = setTimeout(() => {
            result.timedOut = true;
            child.kill('SIGKILL');
        }, timeoutMs);
        child.once('close', (code, signal) => {
            clearTimeout(watchdog);
            result.code = code;
            result.signal = signal;
            result.grade = gradeLogoProcess(result);
            result.exitCode = result.grade.ok ? 0 : (code > 0 && code <= 255 ? code : 1);
            resolve(result);
        });
    });
}

if (require.main === module) {
    runLogoNode().then((result) => {
        process.stdout.write(result.stdout);
        process.stderr.write(result.stderr);
        console.log(formatLogoGrade(result.grade));
        process.exitCode = result.exitCode;
    }).catch((error) => {
        console.error(error);
        process.exitCode = 1;
    });
}

module.exports = { gradeLogoProcess, runLogoNode };