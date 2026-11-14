const { ClientFunction, Selector } = require('testcafe');

const applicationUrl = process.env.TESTCAFE_APP_URL || 'http://127.0.0.1:8085/ucblogo.html';
const commandInput = Selector('#logoInputText');
const output = Selector('#output');
const idle = ClientFunction(() => Boolean(window.Module && Module.calledRun &&
    typeof Asyncify !== 'undefined' && Asyncify.state === 0 &&
    getValue(Module._LOGO_GIL) === 0 && !Module.blockedOnGetc &&
    document.getElementById('logoInputText') &&
    !document.getElementById('logoInputText').disabled));
const diagnostics = ClientFunction(() => ({
    url: location.href,
    status: document.getElementById('status')?.textContent,
    error: document.getElementById('javascriptError')?.textContent,
    input: document.getElementById('logoInputText')?.value,
    output: document.getElementById('output')?.textContent.slice(-12000),
    blockedOnGetc: window.Module?.blockedOnGetc,
    stdinLength: window.Module?.FS_runjs_getChar_buffer?.length,
}));
const reload = ClientFunction(() => location.reload());
const storedSettings = ClientFunction(() => localStorage.getItem('ucblogo.settings'));
const storedLegacyFont = ClientFunction(() => localStorage.getItem('ucblogo.fontSize'));
const storeLegacyFont = ClientFunction((value) => {
    if (value === null) localStorage.removeItem('ucblogo.fontSize');
    else localStorage.setItem('ucblogo.fontSize', value);
});
const storeSettings = ClientFunction((value) => {
    if (value === null) localStorage.removeItem('ucblogo.settings');
    else localStorage.setItem('ucblogo.settings', value);
});

async function ready(t) {
    await t.setNativeDialogHandler((type) => type === 'beforeunload');
    await t.expect(output.textContent).contains('Welcome to Berkeley Logo', { timeout: 30000 });
    await t.expect(idle()).ok('Logo must be initialized and waiting for input', { timeout: 30000 });
}

async function runLogo(t, command) {
    await t.expect(idle()).ok('Logo must be idle before submitting input', { timeout: 30000 });
    await t.selectText(commandInput).pressKey('delete')
        .typeText(commandInput, command, { paste: true }).pressKey('enter');
    await t.expect(idle()).ok(`Logo must finish: ${command}`, { timeout: 30000 });
    const delay = Number(process.env.TEST_DELAY_MS) || 0;
    if (delay > 0) await t.wait(delay);
}

async function expectOutput(t, text) {
    try {
        await t.expect(output.textContent).contains(text, { timeout: 10000 });
    } catch (error) {
        console.error('Logo state:', JSON.stringify(await diagnostics()));
        throw error;
    }
}

async function isolateSettings(t) {
    await ready(t);
    t.ctx.originalSettings = await storedSettings();
    t.ctx.originalLegacyFont = await storedLegacyFont();
    t.ctx.settingsCaptured = true;
    await storeSettings(null);
    await storeLegacyFont(null);
    await reload();
    await ready(t);
}

async function restoreSettings(t) {
    try {
        if (t.ctx.settingsCaptured) {
            await storeSettings(t.ctx.originalSettings);
            await storeLegacyFont(t.ctx.originalLegacyFont);
            await reload();
            await ready(t);
            await storeSettings(t.ctx.originalSettings);
            await storeLegacyFont(t.ctx.originalLegacyFont);
        }
    } finally {
        if (t.testRun.errs.length) {
            console.error('Logo state:', JSON.stringify(await diagnostics()));
        }
    }
}

module.exports = {
    applicationUrl, ready, runLogo, expectOutput, isolateSettings, restoreSettings,
    diagnostics, commandInput, output, idle,
};