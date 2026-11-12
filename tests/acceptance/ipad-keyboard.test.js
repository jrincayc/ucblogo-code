const { ClientFunction, Selector } = require('testcafe');

const applicationUrl = process.env.TESTCAFE_APP_URL ||
    'http://127.0.0.1:8083/ucblogo.html';
const commandInput = Selector('#logoInputText');
const logoOutput = Selector('#output');
const status = Selector('#status');
const startupState = ClientFunction(() => ({
    error: document.getElementById('javascriptError')?.textContent || '',
    status: document.getElementById('status')?.textContent || '',
    output: document.getElementById('output')?.textContent || '',
}));
const readLayout = ClientFunction(() => {
    const app = document.getElementById('appContainer');
    const output = document.getElementById('outputContainer');
    return {
        visualViewportHeight: window.visualViewport?.height || window.innerHeight,
        innerHeight: window.innerHeight,
        visualViewportOffsetTop: window.visualViewport?.offsetTop || 0,
        appTop: app.getBoundingClientRect().top,
        appBottom: app.getBoundingClientRect().bottom,
        appHeight: app.getBoundingClientRect().height,
        documentScrollY: window.scrollY,
        documentHeight: document.documentElement.scrollHeight,
        documentClientHeight: document.documentElement.clientHeight,
        bodyHeight: document.body.getBoundingClientRect().height,
        bodyScrollHeight: document.body.scrollHeight,
        outputScrollTop: output.scrollTop,
    };
});
const prepareScrollableOutput = ClientFunction(() => {
    const output = document.getElementById('output');
    const container = document.getElementById('outputContainer');
    output.textContent = Array.from({ length: 120 }, (_, index) =>
        `Output line ${index + 1}`,
    ).join('\n');
    container.scrollTop = 120;
    return container.scrollTop;
});
const simulateKeyboard = ClientFunction(() => {
    const viewport = window.visualViewport;
    const baseHeight = viewport.height;
    const keyboardHeight = Math.min(320, Math.round(baseHeight * 0.4));
    const overlay = document.createElement('div');
    overlay.id = 'keyboardSimulation';
    overlay.setAttribute('aria-label', 'Simulated iOS keyboard');
    Object.assign(overlay.style, {
        position: 'fixed',
        zIndex: '2147483647',
        left: '0',
        right: '0',
        bottom: '0',
        height: `${keyboardHeight}px`,
        display: 'grid',
        placeItems: 'center',
        background: '#d1d5db',
        color: '#111827',
        font: '16px sans-serif',
        borderTop: '1px solid #6b7280',
    });
    overlay.textContent = 'Simulated iOS keyboard';
    document.body.appendChild(overlay);
    Object.defineProperty(viewport, 'height', {
        configurable: true,
        value: baseHeight - keyboardHeight,
    });
    viewport.dispatchEvent(new Event('resize'));
    return keyboardHeight;
});

fixture('iPad virtual keyboard viewport POC').page(applicationUrl);

test('Logo starts and displays its welcome message', async (t) => {
    try {
        await t.expect(logoOutput.innerText).contains('Welcome to Berkeley Logo',
            'Logo must finish starting and print its welcome message', { timeout: 30000 });
    } catch (error) {
        console.error(`Logo startup state: ${JSON.stringify(await startupState())}`);
        throw error;
    }
    await t.expect(commandInput.hasAttribute('disabled')).notOk('Logo command input must be enabled');
    await t.expect(status.textContent).notContains('Exception thrown', 'Startup must not report an exception');
});

test('simulated keyboard resizes the app without losing output position', async (t) => {
    await t.expect(commandInput.exists).ok('Logo command input should be present');
    await t.expect(ClientFunction(() => Boolean(
        window.Module && Module.calledRun && Module.settings,
    ))()).ok('Logo runtime should be ready');
    await t.expect(logoOutput.innerText).contains('Welcome to Berkeley Logo',
        'Logo must finish starting before the output is replaced', { timeout: 30000 });

    const initialScrollTop = await prepareScrollableOutput();
    const before = await readLayout();
    console.log(`Before keyboard simulation: ${JSON.stringify(before)}`);

    const simulatedKeyboardHeight = await simulateKeyboard();

    const after = await readLayout();
    console.log(`Simulated keyboard height: ${simulatedKeyboardHeight}px`);
    console.log(`After keyboard simulation: ${JSON.stringify(after)}`);

    await t.expect(after.visualViewportHeight)
        .eql(before.visualViewportHeight - simulatedKeyboardHeight,
            'The simulated visual viewport should shrink by the keyboard height');
    await t.expect(Math.abs(after.appHeight - (before.appHeight - simulatedKeyboardHeight)))
        .lt(2,
            'The app container should shrink by the simulated keyboard height');
    await t.expect(after.documentScrollY)
        .lte(before.documentScrollY + 40,
            'The simulated keyboard should not push the whole document far upward');
    await t.expect(after.outputScrollTop).eql(initialScrollTop,
        'The simulated keyboard should preserve output scroll position');
});
