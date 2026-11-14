const assert = require('node:assert/strict');
const { test } = require('node:test');
const { gradeLogoOutput, formatLogoGrade, allowLogoShutdown } = require('./logo-results');
const { gradeLogoProcess, runLogoNode } = require('./run-logo-node');

test('grades actual same-line and multiline Logo statuses and reports OOP as skipped', () => {
    const grade = gradeLogoOutput([
        'skipping oop tests',
        '[ Files ]',
        'Tests.File.OpenWriteCharBufferArgError...    Ok',
        'Tests.Control.Stop... ',
        '    Ok',
        'All tests completed.',
    ].join('\r\n'));
    assert.equal(grade.ok, true);
    assert.deepEqual(grade.counts, { passed: 2, failed: 0, errors: 0, total: 2 });
    assert.deepEqual(grade.skips, ['skipping oop tests']);
    assert.match(formatLogoGrade(grade), /2 passed/);
    assert.match(formatLogoGrade(grade), /SKIP: skipping oop tests/);
});

for (const status of ['Failed', 'Error']) {
    for (const separator of [' ', '\n       ']) {
        test(`rejects intentional ${status} result with ${JSON.stringify(separator)} separator`, () => {
            const grade = gradeLogoOutput(`Tests.Bad...${separator}${status}\nAll tests completed.`);
            assert.equal(grade.ok, false);
            assert.equal(grade.results[0].status, status);
            assert.deepEqual(grade.problems, [`Tests.Bad: ${status}`]);
        });
    }
}

test('expected error diagnostics between a name and Ok are not failed results', () => {
    const grade = gradeLogoOutput([
        'Tests.ExpectedError...    openwritecharbuffer doesn\'t like 123 as input',
        'in Tests.ExpectedError',
        'Error 4: intentional diagnostic mentioning Failed and Error',
        '    Ok',
        'Tests.Other... ',
        'I don\'t know how to intentionalfailure',
        'in Tests.Other',
        'intentionalfailure',
        'Ok',
        'All tests completed.',
    ].join('\n'));
    assert.equal(grade.ok, true);
    assert.equal(grade.counts.passed, 2);
    assert.equal(grade.results[0].diagnostics.length, 3);
});

test('rejects missing completion including a marker quoted in a diagnostic', () => {
    for (const output of ['Tests.Good... Ok', 'Tests.Good... Ok\nExpected All tests completed.']) {
        const grade = gradeLogoOutput(output);
        assert.equal(grade.ok, false);
        assert.equal(grade.completed, false);
        assert.ok(grade.problems.includes('Missing successful completion marker'));
    }
});

test('rejects a failure completion even when every reported result is Ok', () => {
    const grade = gradeLogoOutput('Tests.Good... Ok\nSome tests did not complete.');
    assert.equal(grade.ok, false);
    assert.ok(grade.problems.includes('Some tests did not complete.'));
});

test('rejects empty or result-free completion and never counts a skip as a pass', () => {
    for (const output of ['', 'All tests completed.', 'skipping oop tests\nAll tests completed.']) {
        const grade = gradeLogoOutput(output);
        assert.equal(grade.ok, false);
        assert.equal(grade.counts.total, 0);
        assert.ok(grade.problems.includes('No Logo test results found'));
    }
});

test('rejects truncated records, orphan statuses, duplicate markers and late results', () => {
    for (const output of [
        'Tests.Missing...\nTests.Good... Ok\nAll tests completed.',
        'Tests.Good... Ok\nTests.Missing...\nAll tests completed.',
        'Ok\nTests.Good... Ok\nAll tests completed.',
        'Tests.Good... Ok\nAll tests completed.\nAll tests completed.',
        'Tests.Good... Ok\nAll tests completed.\nTests.Late... Ok',
    ]) {
        assert.equal(gradeLogoOutput(output).ok, false, output);
    }
});

test('rejects runtime crashes in output even after successful completion', () => {
    for (const diagnostic of [
        'RuntimeError: unreachable',
        'Aborted(unhandled exception: 123)',
        'abort(Assertion failed)',
        'Segmentation fault',
        'memory access out of bounds',
    ]) {
        const grade = gradeLogoOutput(`Tests.Good... Ok\nAll tests completed.\n${diagnostic}`);
        assert.equal(grade.ok, false, diagnostic);
    }
});

test('process exit, signal, spawn errors, timeouts and stderr crashes override passing output', () => {
    const passing = { stdout: 'Tests.Good... Ok\nAll tests completed.', stderr: '', code: 0 };
    assert.equal(gradeLogoProcess(passing).ok, true);
    for (const failure of [
        { code: 7 }, { code: null, signal: 'SIGSEGV' },
        { error: new Error('spawn failed') }, { timedOut: true },
        { stderr: 'RuntimeError: Aborted(unhandled exception: 42)' },
    ]) {
        assert.equal(gradeLogoProcess({ ...passing, ...failure }).ok, false);
    }
});

test('Node runner captures output and rejects intentional failed, error and truncated runs', async () => {
    for (const output of [
        'Tests.Bad... Failed\nAll tests completed.',
        'Tests.Bad...\n Error\nAll tests completed.',
        'Tests.Good... Ok',
        'All tests completed.',
    ]) {
        const result = await runLogoNode({ args: ['-e', `process.stdout.write(${JSON.stringify(output)})`] });
        assert.equal(result.stdout, output);
        assert.equal(result.code, 0);
        assert.equal(result.exitCode, 1);
    }
});

test('Node runner propagates real child exit and crashes even with completed output', async () => {
    const print = 'console.log("Tests.Good... Ok\\nAll tests completed.");';
    const failure = await runLogoNode({ args: ['-e', `${print} process.exit(7);`] });
    assert.equal(failure.exitCode, 7);
    const crash = await runLogoNode({ args: ['-e', `${print} throw new Error("genuine crash");`] });
    assert.equal(crash.grade.ok, false);
    assert.equal(crash.exitCode, 1);
    assert.match(crash.stderr, /genuine crash/);
    const signal = await runLogoNode({ args: ['-e', `${print} process.kill(process.pid, "SIGTERM");`] });
    assert.equal(signal.signal, 'SIGTERM');
    assert.equal(signal.exitCode, 1);
    const missing = await runLogoNode({ executable: '/nonexistent/ucblogo-test-runtime' });
    assert.equal(missing.error.code, 'ENOENT');
    assert.equal(missing.exitCode, 1);
    const timeout = await runLogoNode({ args: ['-e', 'setInterval(() => {}, 1000)'], timeoutMs: 100 });
    assert.equal(timeout.timedOut, true);
    assert.equal(timeout.exitCode, 1);
});

test('browser shutdown allowance rejects pre-completion aborts, bad grades and other crashes', () => {
    const shutdown = {
        message: 'RuntimeError: Aborted(unhandled exception: [object Object],). Build with -sASSERTIONS for more info.',
        stack: 'RuntimeError: Aborted(unhandled exception: [object Object],). Build with -sASSERTIONS for more info.\n at Object.doRewind (http://127.0.0.1:8085/ucblogo.js:6454:16)',
    };
    const completed = 'Tests.Good... Ok\nAll tests completed.';
    assert.equal(allowLogoShutdown(shutdown, gradeLogoOutput(completed)), true);
    assert.equal(allowLogoShutdown({
        message: shutdown.message,
        stack: `${shutdown.message}\nabort@http://127.0.0.1:8085/ucblogo.js:1718:11\ndoRewind@http://127.0.0.1:8085/ucblogo.js:6454:16\n`,
    }, gradeLogoOutput(completed)), true);
    for (const output of [
        '', 'Tests.Good... Ok', 'All tests completed.',
        'Tests.Bad... Failed\nAll tests completed.',
        'Tests.Bad...\n Error\nAll tests completed.',
        'Tests.Good... Ok\nSome tests did not complete.',
    ]) {
        assert.equal(allowLogoShutdown(shutdown, gradeLogoOutput(output)), false, output);
    }
    for (const error of [
        { ...shutdown, message: 'RuntimeError: memory access out of bounds' },
        { ...shutdown, message: 'RuntimeError: Aborted(unhandled exception: genuine crash)' },
        { ...shutdown, message: `${shutdown.message} unexpected suffix` },
        { ...shutdown, message: 'TypeError: genuine crash' },
        { ...shutdown, stack: 'at genuineCrash (test.js:1:1)' },
    ]) {
        assert.equal(allowLogoShutdown(error, gradeLogoOutput(completed)), false, error.message);
    }
});