const { ClientFunction, Selector } = require('testcafe');
const { applicationUrl, ready, commandInput, output, diagnostics } = require('./testcafe-helpers');
const { gradeLogoOutput, formatLogoGrade, allowLogoShutdown } = require('./logo-results');

const shutdownErrors = ClientFunction(() => window.__logoSuiteShutdownErrors || []);
const installShutdownCapture = ClientFunction(() => { window.__logoSuiteShutdownErrors = []; });
fixture('Bundled Logo regression suite').page(applicationUrl).disablePageCaching;

test('ACC-LOGO-001 detects suite failures and requires its completion marker', async (t) => {
    await ready(t);
    await installShutdownCapture();
    await t.skipJsErrors({
        fn: (error) => {
            const text = document.getElementById('output')?.textContent || '';
            const grade = gradeLogoOutput(text);
            const allowed = allowLogoShutdown(error, grade);
            window.__logoSuiteShutdownErrors.push({
                message: error.message, stack: error.stack, allowed,
                completed: grade.completed,
            });
            return allowed;
        },
        dependencies: { allowLogoShutdown, gradeLogoOutput },
    });
    await t.typeText(commandInput,
        `ignore runjs "|FS.chdir('/')| load "tests/test.lg`, { paste: true })
        .pressKey('enter');
    await t.expect(output.textContent).contains('All tests completed.', {
        timeout: 240000,
    });
    const results = await output.textContent;
    const grade = gradeLogoOutput(results);
    console.log(formatLogoGrade(grade));
    const state = await diagnostics();
    const errors = await shutdownErrors();
    console.log('Logo shutdown diagnostics:', JSON.stringify({
        status: state.status,
        errors: errors.map((error) => ({
            message: error.message, allowed: error.allowed, completed: error.completed,
        })),
    }));
    if (!grade.ok) console.error('Logo state:', JSON.stringify(state));
    await t.expect(grade.ok).ok(`${formatLogoGrade(grade)}\n${results}`);
    await t.expect(errors.length).gt(0, 'Expected the known final-bye shutdown diagnostic');
    for (const error of errors) {
        await t.expect(error.allowed).ok(`Unexpected JavaScript error: ${JSON.stringify(error)}`);
        await t.expect(error.completed).ok('Shutdown cannot precede Logo suite completion');
        await t.expect(Selector('#javascriptError').textContent).contains(error.message);
    }
});
