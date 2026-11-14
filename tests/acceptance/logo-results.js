function gradeLogoOutput(output) {
    const results = [];
    const skips = [];
    const problems = [];
    const diagnostics = [];
    let pending;
    let completed = false;
    let completionCount = 0;
    const lines = String(output).replace(/\r\n?/g, '\n').split('\n');

    function finishPending() {
        if (pending) {
            problems.push(`Missing result for ${pending.name}`);
            pending = undefined;
        }
    }

    for (let index = 0; index < lines.length; index += 1) {
        const line = lines[index].trim();
        if (/\b(?:RuntimeError|Aborted\(|abort\(|Segmentation fault|memory access out of bounds|unreachable executed|Assertion failed|FATAL ERROR)\b/i.test(line)) {
            problems.push(`Runtime crash at line ${index + 1}: ${line}`);
        }
        if (line === 'skipping oop tests') {
            skips.push(line);
            continue;
        }
        if (line === 'All tests completed.' || line === 'Some tests did not complete.') {
            finishPending();
            completionCount += 1;
            completed = line === 'All tests completed.';
            if (!completed) problems.push(line);
            continue;
        }

        const start = /^([^\s]+?)\.\.\.[ \t]*(.*)$/.exec(line);
        let status = line;
        if (start) {
            finishPending();
            if (completionCount) problems.push('Result appears after suite completion');
            pending = { name: start[1], line: index + 1, diagnostics: [] };
            status = start[2].trim();
        }
        if (/^(?:Ok|Failed|Error)$/.test(status)) {
            if (!pending) {
                problems.push(`Result without a test name at line ${index + 1}: ${status}`);
                continue;
            }
            results.push({
                name: pending.name, line: pending.line,
                diagnostics: pending.diagnostics, status,
            });
            if (status !== 'Ok') problems.push(`${pending.name}: ${status}`);
            pending = undefined;
        } else if (pending && status) {
            pending.diagnostics.push(status);
        } else if (line) {
            diagnostics.push(line);
        }
    }
    finishPending();
    if (!completed) problems.push('Missing successful completion marker');
    if (completionCount > 1) problems.push('Multiple suite completion markers');
    if (!results.length) problems.push('No Logo test results found');
    const counts = { passed: 0, failed: 0, errors: 0, total: results.length };
    for (const result of results) {
        if (result.status === 'Ok') counts.passed += 1;
        else if (result.status === 'Failed') counts.failed += 1;
        else counts.errors += 1;
    }
    return { ok: problems.length === 0, completed, counts, results, skips, problems, diagnostics };
}

function formatLogoGrade(grade) {
    const { passed, failed, errors, total } = grade.counts;
    return [
        `Logo: ${passed} passed, ${failed} failed, ${errors} errors (${total} results)`,
        ...grade.skips.map((skip) => `SKIP: ${skip}`),
        ...grade.problems.map((problem) => `FAIL: ${problem}`),
    ].join('\n');
}

function allowLogoShutdown(error, grade) {
    return error.message === 'RuntimeError: Aborted(unhandled exception: [object Object],). Build with -sASSERTIONS for more info.' &&
        /(?:\bat (?:Object\.)?doRewind \([^\n]*\/ucblogo\.js:\d+:\d+\)|(?:^|\n)doRewind@[^\n]*\/ucblogo\.js:\d+:\d+(?:\n|$))/.test(error.stack || '') &&
        grade.ok && grade.completed;
}

module.exports = { gradeLogoOutput, formatLogoGrade, allowLogoShutdown };