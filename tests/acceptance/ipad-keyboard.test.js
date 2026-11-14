const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl, ready, isolateSettings, restoreSettings,
    commandInput, output: logoOutput,
} = require('./testcafe-helpers');

const status = Selector('#status');
const storedLegacyFontSize = ClientFunction(() => localStorage.getItem('ucblogo.fontSize'));
const storeLegacyFontSize = ClientFunction((value) => {
    if (value === null) localStorage.removeItem('ucblogo.fontSize');
    else localStorage.setItem('ucblogo.fontSize', value);
});
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
    if (!viewport) throw new Error('Keyboard simulation requires the Visual Viewport API');
    const heightDescriptor = Object.getOwnPropertyDescriptor(viewport, 'height');
    const before = readLayout();
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
    try {
        document.body.appendChild(overlay);
        Object.defineProperty(viewport, 'height', {
            configurable: true,
            value: baseHeight - keyboardHeight,
        });
        viewport.dispatchEvent(new Event('resize'));
        return { before, after: readLayout(), keyboardHeight };
    } finally {
        if (heightDescriptor) Object.defineProperty(viewport, 'height', heightDescriptor);
        else delete viewport.height;
        overlay.remove();
        viewport.dispatchEvent(new Event('resize'));
    }
}, { dependencies: { readLayout } });

fixture('iPad startup and keyboard viewport simulation')
    .page(applicationUrl)
    .beforeEach(async (t) => {
        await ready(t);
        t.ctx.originalLegacyFontSize = await storedLegacyFontSize();
        t.ctx.legacyFontSizeCaptured = true;
        await storeLegacyFontSize(null);
        await isolateSettings(t);
    })
    .afterEach(async (t) => {
        if (t.ctx.legacyFontSizeCaptured) await storeLegacyFontSize(t.ctx.originalLegacyFontSize);
        await restoreSettings(t);
    });

test('Logo starts and displays its welcome message', async (t) => {
    await ready(t);
    await t.expect(logoOutput.innerText).contains('Welcome to Berkeley Logo',
        'Logo must finish starting and print its welcome message', { timeout: 30000 });
    await t.expect(commandInput.hasAttribute('disabled')).notOk('Logo command input must be enabled');
    await t.expect(status.textContent).notContains('Exception thrown', 'Startup must not report an exception');
});

test('simulated keyboard resizes the app without losing output position', async (t) => {
    await ready(t);
    const initialScrollTop = await prepareScrollableOutput();
    const { before, after, keyboardHeight } = await simulateKeyboard();

    await t.expect(after.visualViewportHeight)
        .eql(before.visualViewportHeight - keyboardHeight,
            'The simulated visual viewport should shrink by the keyboard height');
    await t.expect(Math.abs(after.appHeight - (before.appHeight - keyboardHeight)))
        .lt(2,
            'The app container should shrink by the simulated keyboard height');
    await t.expect(after.documentScrollY)
        .lte(before.documentScrollY + 40,
            'The simulated keyboard should not push the whole document far upward');
    await t.expect(after.outputScrollTop).eql(initialScrollTop,
        'The simulated keyboard should preserve output scroll position');
    const restored = await readLayout();
    await t.expect(restored.visualViewportHeight).eql(before.visualViewportHeight)
        .expect(Math.abs(restored.appHeight - before.appHeight)).lt(2)
        .expect(restored.outputScrollTop).eql(initialScrollTop)
        .expect(Selector('#keyboardSimulation').exists).notOk();
});
