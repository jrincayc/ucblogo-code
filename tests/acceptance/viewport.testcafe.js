const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl, ready, isolateSettings, restoreSettings,
} = require('./testcafe-helpers');

const reloadPage = ClientFunction(() => window.location.reload());
const viewportSettled = ClientFunction((orientation) => {
    const viewportHeight = window.visualViewport
        ? window.visualViewport.height : window.innerHeight;
    const configuredHeight = Number.parseFloat(getComputedStyle(document.documentElement)
        .getPropertyValue('--app-viewport-height'));
    const appHeight = document.getElementById('appContainer').getBoundingClientRect().height;
    return (window.innerWidth > window.innerHeight) === (orientation === 'landscape') &&
        Math.abs(configuredHeight - viewportHeight) < 1 &&
        Math.abs(appHeight - viewportHeight + 24) < 2;
});
const readViewportSizing = ClientFunction(() => ({
    viewportHeight: window.visualViewport
        ? window.visualViewport.height : window.innerHeight,
    appHeight: document.getElementById('appContainer').getBoundingClientRect().height,
}));
const prepareOutput = ClientFunction((singleLine) => {
    const output = document.getElementById('output');
    const container = document.getElementById('outputContainer');
    const lines = [];
    for (let index = 1; index <= 500; index++) lines.push(`Logo output line ${index}`);
    output.textContent = singleLine ? 'Single line Logo output' : lines.join('\n');
    container.scrollTop = singleLine ? 0 : 120;
    return {
        scrollTop: container.scrollTop,
        maxScrollTop: container.scrollHeight - container.clientHeight,
        lineCount: output.textContent.split('\n').length,
    };
});
const readOutputScrollTop = ClientFunction(() =>
    document.getElementById('outputContainer').scrollTop);
const readScreenMode = ClientFunction(() => Module.graphics.mode);
const textScreen = ClientFunction(() => Module.graphics.text_screen());
const splitScreen = ClientFunction(() => Module.graphics.split_screen());

async function setViewport(t, orientation, width, height) {
    await t.resizeWindow(width, height);
    await t.expect(viewportSettled(orientation)).ok(
        `Viewport sizing did not settle in ${orientation}`, { timeout: 5000 },
    );
}

async function assertScreenModeOutput(t, singleLine) {
    const state = await prepareOutput(singleLine);
    if (singleLine) await t.expect(state.lineCount).eql(1);
    else await t.expect(state.maxScrollTop).gt(state.scrollTop);

    await textScreen();
    await t.expect(readScreenMode()).eql('text_screen')
        .expect(readOutputScrollTop()).eql(state.scrollTop);
    await splitScreen();
    await t.expect(readScreenMode()).eql('split_screen')
        .expect(readOutputScrollTop()).eql(state.scrollTop);
}

fixture('Viewport and output scrolling')
    .page(applicationUrl.split('#')[0])
    .beforeEach(async (t) => {
        await t.setNativeDialogHandler(() => true);
        await ready(t);
        await isolateSettings(t);
        await reloadPage();
        await ready(t);
    })
    .afterEach(async (t) => {
        await restoreSettings(t);
    });

test('aligns the app to a keyboard-offset visual viewport and resets it afterward (simulation)', async (t) => {
    const initialScroll = await prepareOutput(false);
    await t.expect(initialScroll.scrollTop).eql(120);
    const state = await t.eval(() => {
        const viewport = window.visualViewport;
        const app = document.getElementById('appContainer');
        const output = document.getElementById('outputContainer');
        const initialTop = app.getBoundingClientRect().top;
        const descriptors = {};
        for (const name of ['height', 'offsetTop', 'scale']) {
            descriptors[name] = Object.getOwnPropertyDescriptor(viewport, name);
        }
        let keyboardTop;
        let keyboardOutputScroll;
        let scrolledViewportTop;
        let zoomedTop;
        try {
            Object.defineProperty(viewport, 'height', {
                configurable: true, value: window.innerHeight - 320,
            });
            Object.defineProperty(viewport, 'offsetTop', {
                configurable: true, value: 21.5,
            });
            viewport.dispatchEvent(new Event('resize'));
            keyboardTop = app.getBoundingClientRect().top;
            keyboardOutputScroll = output.scrollTop;
            Object.defineProperty(viewport, 'offsetTop', {
                configurable: true, value: 30,
            });
            viewport.dispatchEvent(new Event('scroll'));
            scrolledViewportTop = app.getBoundingClientRect().top;
            Object.defineProperty(viewport, 'scale', {
                configurable: true, value: 2,
            });
            viewport.dispatchEvent(new Event('resize'));
            zoomedTop = app.getBoundingClientRect().top;
        } finally {
            for (const name of ['height', 'offsetTop', 'scale']) {
                if (descriptors[name]) Object.defineProperty(viewport, name, descriptors[name]);
                else delete viewport[name];
            }
            viewport.dispatchEvent(new Event('resize'));
        }
        return {
            initialTop, keyboardTop, keyboardOutputScroll, scrolledViewportTop, zoomedTop,
            restoredTop: app.getBoundingClientRect().top,
            restoredOutputScroll: output.scrollTop,
        };
    });
    await t.expect(state.keyboardTop).within(state.initialTop + 21.4, state.initialTop + 21.6)
        .expect(state.scrolledViewportTop).within(state.initialTop + 29.9, state.initialTop + 30.1)
        .expect(state.zoomedTop).within(state.initialTop - 0.1, state.initialTop + 0.1)
        .expect(state.restoredTop).within(state.initialTop - 0.1, state.initialTop + 0.1)
        .expect(state.keyboardOutputScroll).eql(initialScroll.scrollTop)
        .expect(state.restoredOutputScroll).eql(initialScroll.scrollTop);
});

test('shows viewport diagnostics only when requested and updates on viewport scroll (simulation)', async (t) => {
    await t.expect(Selector('#viewportDiagnostic').count).eql(0);
    await t.navigateTo(`${applicationUrl.split('#')[0]}#viewport-debug`);
    await reloadPage();
    await ready(t);
    const values = await t.eval(() => {
        const viewport = window.visualViewport;
        const descriptor = Object.getOwnPropertyDescriptor(viewport, 'offsetTop');
        try {
            Object.defineProperty(viewport, 'offsetTop', {
                configurable: true, value: 42,
            });
            viewport.dispatchEvent(new Event('scroll'));
            const diagnostic = document.getElementById('viewportDiagnostic');
            return {
                text: diagnostic.textContent,
                pointerEvents: getComputedStyle(diagnostic).pointerEvents,
                transform: diagnostic.style.transform,
            };
        } finally {
            if (descriptor) Object.defineProperty(viewport, 'offsetTop', descriptor);
            else delete viewport.offsetTop;
            viewport.dispatchEvent(new Event('scroll'));
        }
    });
    await t.expect(values.text).contains('visualOffsetTop: 42')
        .expect(values.text).contains('visualPageTop:')
        .expect(values.pointerEvents).eql('none')
        .expect(values.transform).contains('42px');
});

for (const eventName of ['visual viewport resize', 'orientationchange']) {
    test(`resets document scroll on ${eventName} without moving output (simulation)`, async (t) => {
        const state = await t.eval(() => new Promise((resolve, reject) => {
            const output = document.getElementById('output');
            const container = document.getElementById('outputContainer');
            const lines = [];
            for (let index = 1; index <= 500; index++) lines.push(`Logo output line ${index}`);
            output.textContent = lines.join('\n');
            container.scrollTop = 120;
            const initialOutputScroll = container.scrollTop;
            const spacer = document.createElement('div');
            spacer.style.height = '1000px';
            document.body.appendChild(spacer);
            window.scrollTo(0, 200);
            const initialDocumentScroll = window.scrollY;
            let delayedScrollApplied = eventName !== 'orientationchange';
            let delayedScroll;
            if (eventName === 'visual viewport resize') {
                window.visualViewport.dispatchEvent(new Event('resize'));
            } else {
                window.dispatchEvent(new Event('orientationchange'));
                delayedScroll = setTimeout(() => {
                    window.scrollTo(0, 200);
                    delayedScrollApplied = true;
                }, 250);
            }
            const deadline = Date.now() + 5000;
            const poll = () => {
                if (delayedScrollApplied && window.scrollY === 0) {
                    const result = {
                        initialDocumentScroll,
                        finalDocumentScroll: window.scrollY,
                        initialOutputScroll,
                        finalOutputScroll: container.scrollTop,
                    };
                    spacer.remove();
                    resolve(result);
                } else if (Date.now() >= deadline) {
                    clearTimeout(delayedScroll);
                    const finalDocumentScroll = window.scrollY;
                    spacer.remove();
                    window.scrollTo(0, 0);
                    reject(new Error(`Document scroll did not reset: ${finalDocumentScroll}`));
                } else {
                    setTimeout(poll, 25);
                }
            };
            poll();
        }), { dependencies: { eventName } });
        await t.expect(state.initialDocumentScroll).gt(0)
            .expect(state.finalDocumentScroll).eql(0)
            .expect(state.initialOutputScroll).eql(120)
            .expect(state.finalOutputScroll).eql(state.initialOutputScroll);
    });
}

for (const orientation of ['portrait', 'landscape']) {
    const initialViewport = orientation === 'portrait'
        ? { width: 500, height: 900 } : { width: 900, height: 500 };
    const resizedViewport = orientation === 'portrait'
        ? { width: 520, height: 940 } : { width: 940, height: 520 };

    for (const singleLine of [false, true]) {
        const behavior = singleLine ? 'keeps one-line Logo output anchored'
            : 'preserves Logo output scroll position';
        test.meta({ requires: 'resize' })(`${behavior} when resizing in ${orientation}`, async (t) => {
            await setViewport(t, orientation, initialViewport.width, initialViewport.height);
            const state = await prepareOutput(singleLine);
            if (singleLine) await t.expect(state.lineCount).eql(1);
            else await t.expect(state.maxScrollTop).gt(state.scrollTop);

            await setViewport(t, orientation, resizedViewport.width, resizedViewport.height);
            const sizing = await readViewportSizing();
            await t.expect(sizing.appHeight).within(sizing.viewportHeight - 26, sizing.viewportHeight - 22)
                .expect(readOutputScrollTop()).eql(state.scrollTop);
        });

        test.meta({ requires: 'resize' })(`${behavior} when switching screen modes in ${orientation}`, async (t) => {
            await setViewport(t, orientation, initialViewport.width, initialViewport.height);
            await assertScreenModeOutput(t, singleLine);
        });
    }
}

test('preserves Logo output scroll position when switching screen modes at the device viewport', async (t) => {
    await assertScreenModeOutput(t, false);
});

test('keeps one-line Logo output anchored when switching screen modes at the device viewport', async (t) => {
    await assertScreenModeOutput(t, true);
});