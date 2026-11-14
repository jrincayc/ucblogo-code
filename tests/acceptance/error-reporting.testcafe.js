const { ClientFunction, Selector } = require('testcafe');

const applicationUrl = process.env.TESTCAFE_APP_URL || 'http://127.0.0.1:8085/ucblogo.html';
const errorPanel = Selector('#javascriptError');
const status = Selector('#status');
const triggerUncaughtError = ClientFunction(() => {
    const error = new Error('testcafe uncaught browser error');
    window.onerror(error.message, 'testcafe-error.js', 23, 7, error);
});
const triggerUnhandledRejection = ClientFunction(() => {
    const rejection = new Event('unhandledrejection', { cancelable: true });
    Object.defineProperty(rejection, 'reason', {
        value: 'testcafe unhandled browser rejection',
    });
    window.dispatchEvent(rejection);
    return rejection.defaultPrevented;
});

fixture('JavaScript error reporting').page(applicationUrl);

test('uncaught browser errors appear in the visible diagnostic panel', async (t) => {
    await t.expect(Selector('#output').innerText).contains('Welcome to Berkeley Logo', {
        timeout: 30000,
    });
    await triggerUncaughtError();
    await t.expect(errorPanel.innerText).contains('Uncaught JavaScript error')
        .expect(errorPanel.innerText).contains('testcafe uncaught browser error')
        .expect(errorPanel.innerText).contains('testcafe-error.js:23:7')
        .expect(status.innerText).contains('Exception thrown');
});

test('unhandled rejections appear in the diagnostic panel without being suppressed', async (t) => {
    await t.skipJsErrors({ message: 'testcafe unhandled browser rejection' });
    const prevented = await triggerUnhandledRejection();
    await t.expect(prevented).notOk('the rejection must not be suppressed');
    await t.expect(errorPanel.innerText).contains('Unhandled promise rejection')
        .expect(errorPanel.innerText).contains('testcafe unhandled browser rejection');
});
