const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl, ready, runLogo, expectOutput, isolateSettings, restoreSettings,
} = require('./testcafe-helpers');

const resetPage = ClientFunction(() => window.location.reload());
const setPersistedControls = ClientFunction(() => {
    const theme = document.getElementById('darkModeSelect');
    theme.value = 'dark';
    theme.dispatchEvent(new Event('change', { bubbles: true }));
    const rendering = document.querySelector(
        '[name="shapeRenderingMode"][value="optimizespeed"]',
    );
    rendering.checked = true;
    rendering.dispatchEvent(new Event('change', { bubbles: true }));
    for (const id of ['speakOutputSelect', 'visualBellSelect']) {
        const toggle = document.getElementById(id);
        toggle.checked = true;
        toggle.dispatchEvent(new Event('change', { bubbles: true }));
    }
});
const readTextSize = ClientFunction(() => ({
    fontSize: Module.fontSize,
    cssSize: document.getElementById('logoTextSize').sheet.cssRules[0].style.fontSize,
    setting: Module.settings.fontSize,
    storedSize: JSON.parse(localStorage.getItem('ucblogo.settings')).fontSize,
    inputSize: document.querySelector('[name="logoTextSize"]').value,
}));
const readLabelFontSize = ClientFunction(() => {
    const labels = document.querySelectorAll('#logoDrawingElements text');
    const label = labels[labels.length - 1];
    return {
        rendered: getComputedStyle(label).fontSize,
        drawingSize: `${Module.graphics.pen_info.fntsz}px`,
    };
});
const readTheme = ClientFunction(() => ({
    setting: Module.settings.darkMode,
    attribute: document.documentElement.getAttribute('data-theme'),
    foreground: getComputedStyle(document.body).color,
    background: getComputedStyle(document.body).backgroundColor,
    buttonBorderStyle: getComputedStyle(document.getElementById('logoRunButton')).borderStyle,
    buttonBorderColor: getComputedStyle(document.getElementById('logoRunButton')).borderColor,
}));
const readTextColors = ClientFunction(() => ({
    foreground: getComputedStyle(document.getElementById('output')).color,
    background: getComputedStyle(document.getElementById('outputContainer')).backgroundColor,
    inputForeground: getComputedStyle(document.getElementById('logoInputText')).color,
    inputBorder: getComputedStyle(document.getElementById('logoInputText')).borderColor,
}));
const readTextControlStyles = ClientFunction(() => {
    const output = getComputedStyle(document.getElementById('output'));
    const container = getComputedStyle(document.getElementById('outputContainer'));
    const editor = getComputedStyle(document.getElementById('editorTextArea'));
    const input = getComputedStyle(document.getElementById('logoInputText'));
    const button = getComputedStyle(document.getElementById('logoRunButton'));
    const disabledButton = getComputedStyle(document.getElementById('logoStopButton'));
    const history = getComputedStyle(document.getElementById('commandHistorySelect'));
    return {
        foreground: output.color,
        background: container.backgroundColor,
        editorForeground: editor.color,
        editorBackground: editor.backgroundColor,
        inputForeground: input.color,
        inputBackground: input.backgroundColor,
        buttonForeground: button.color,
        buttonBackground: button.backgroundColor,
        editorBorderStyle: editor.borderStyle,
        editorBorderColor: editor.borderColor,
        editorAppearance: editor.appearance,
        editorShadow: editor.boxShadow,
        inputBorderStyle: input.borderStyle,
        inputBorderColor: input.borderColor,
        inputBorderRadius: input.borderRadius,
        inputBorderWidthDevicePixels: Number.parseFloat(input.borderWidth) * window.devicePixelRatio,
        inputAppearance: input.appearance,
        inputShadow: input.boxShadow,
        inputBackgroundImage: input.backgroundImage,
        buttonBorderStyle: button.borderStyle,
        buttonBorderColor: button.borderColor,
        buttonAppearance: button.appearance,
        buttonShadow: button.boxShadow,
        disabledBorderStyle: disabledButton.borderStyle,
        disabledBorderWidthDevicePixels: Number.parseFloat(disabledButton.borderWidth) *
            window.devicePixelRatio,
        disabledOpacity: disabledButton.opacity,
        disabledCursor: disabledButton.cursor,
        historyBorderStyle: history.borderStyle,
        historyBorderColor: history.borderColor,
        historyAppearance: history.appearance,
        historyShadow: history.boxShadow,
    };
});
const inspectAccessibility = ClientFunction(() => {
    const labelText = (element) => {
        const labels = [];
        for (const label of element.labels || []) labels.push(label.textContent.trim());
        return labels.join(' ');
    };
    const labelledByText = (element) => {
        const heading = document.getElementById(element.getAttribute('aria-labelledby'));
        return heading && heading.textContent.trim();
    };
    const renderingModes = [];
    for (const input of document.querySelectorAll('[name="shapeRenderingMode"]'))
        renderingModes.push(labelText(input));
    const dialogTitles = [];
    for (const title of document.querySelectorAll('dialog .dialogTitle')) {
        const style = getComputedStyle(title);
        dialogTitles.push({
            text: title.textContent.trim(),
            display: style.display,
            parentIsDialog: title.parentElement.tagName === 'DIALOG',
            margin: style.margin,
            borderBottomWidth: style.borderBottomWidth,
            fontWeight: style.fontWeight,
        });
    }
    const settingsButton = document.querySelector(
        '#appContainer button[aria-controls="settings"]',
    );
    return {
        labels: {
            command: labelText(document.getElementById('logoInputText')),
            history: document.getElementById('commandHistorySelect').getAttribute('aria-label'),
            run: document.getElementById('logoRunButton').getAttribute('aria-label'),
            stop: document.getElementById('logoStopButton').getAttribute('aria-label'),
            editor: labelText(document.getElementById('editorTextArea')),
            theme: labelText(document.getElementById('darkModeSelect')),
            fontSize: labelText(document.getElementById('logoTextSizeInput')),
            speak: labelText(document.getElementById('speakOutputSelect')),
            visualBell: labelText(document.getElementById('visualBellSelect')),
            renderingModes,
        },
        outputRole: document.getElementById('output').getAttribute('role'),
        outputName: document.getElementById('output').getAttribute('aria-label'),
        statusRole: document.getElementById('status').getAttribute('role'),
        settingsName: labelledByText(document.getElementById('settings')),
        editorName: labelledByText(document.getElementById('editorDialog')),
        settingsButton: settingsButton && settingsButton.textContent.trim(),
        dialogTitles,
        settingsBodyOverflow: getComputedStyle(
            document.querySelector('#settings .dialogBody'),
        ).overflowY,
        editorBodyOverflow: getComputedStyle(
            document.querySelector('#editorDialog .dialogBody'),
        ).overflowY,
    };
});
const inspectSoftKeys = ClientFunction(() => {
    const touchDevice = navigator.maxTouchPoints > 0 ||
        matchMedia('(any-hover: none)').matches;
    const hosts = document.querySelectorAll('logo-soft-keys');
    const initialGroups = document.querySelectorAll('logo-soft-keys [role="group"]').length;
    let group = document.querySelector('logo-soft-keys [role="group"]');
    let temporaryHost = null;
    try {
        if (!group) {
            const descriptor = Object.getOwnPropertyDescriptor(navigator, 'maxTouchPoints');
            try {
                Object.defineProperty(navigator, 'maxTouchPoints', {
                    configurable: true, value: 1,
                });
                const SoftKeys = customElements.get('logo-soft-keys');
                temporaryHost = new SoftKeys();
            } finally {
                if (descriptor) Object.defineProperty(navigator, 'maxTouchPoints', descriptor);
                else delete navigator.maxTouchPoints;
            }
            document.body.appendChild(temporaryHost);
            group = temporaryHost.querySelector('[role="group"]');
        }
        const buttons = [];
        for (const button of group.querySelectorAll('button')) {
            const style = getComputedStyle(button);
            buttons.push({
                name: button.getAttribute('aria-label'),
                type: button.type,
                clickHandler: (button.getAttribute('onclick') || '')
                    .startsWith('Module.softKeyDown(event, '),
                mouseDownHandler: button.hasAttribute('onmousedown'),
                widthEm: Number.parseFloat(style.width) / Number.parseFloat(style.fontSize),
                heightEm: Number.parseFloat(style.height) / Number.parseFloat(style.fontSize),
            });
        }
        return {
            touchDevice,
            hostCount: hosts.length,
            initialGroups,
            groupName: group.getAttribute('aria-label'),
            labelCount: group.querySelectorAll('label').length,
            buttons,
        };
    } finally {
        if (temporaryHost) temporaryHost.remove();
    }
});
const prepareSoftKeySelection = ClientFunction((editorMode) => {
    const editor = document.getElementById('editorDialog');
    if (editorMode) editor.showModal();
    const input = document.getElementById(editorMode ? 'editorTextArea' : 'logoInputText');
    input.value = editorMode ? 'abcXYZdef' : 'draw turtle';
    input.focus();
    input.setSelectionRange(editorMode ? 3 : 5, editorMode ? 6 : 11);
    const name = editorMode ? 'Insert backtick' : 'Insert left square bracket';
    const container = editorMode ? editor : document.getElementById('appContainer');
    for (const button of container.querySelectorAll('logo-soft-keys button')) {
        if (button.getAttribute('aria-label') === name &&
            button.getBoundingClientRect().width > 0 &&
            getComputedStyle(button).visibility === 'visible') return true;
    }
    const button = document.createElement('button');
    button.id = 'softKeyFocusTestButton';
    button.type = 'button';
    button.setAttribute('aria-label', name);
    button.setAttribute('onclick', editorMode
        ? 'Module.softKeyDown(event, String.fromCharCode(96))'
        : "Module.softKeyDown(event, '[')");
    button.style.position = 'fixed';
    button.style.top = '0';
    button.style.left = '0';
    (editorMode ? editor : document.body).appendChild(button);
    return false;
});
const clickRenderedSoftKey = ClientFunction((name, editorMode) => {
    const container = document.getElementById(editorMode ? 'editorDialog' : 'appContainer');
    for (const button of container.querySelectorAll('logo-soft-keys button')) {
        if (button.getAttribute('aria-label') === name &&
            button.getBoundingClientRect().width > 0 &&
            getComputedStyle(button).visibility === 'visible') {
            button.click();
            return;
        }
    }
    throw new Error(`No rendered soft key: ${name}`);
});
const readTextFocus = ClientFunction((id) => {
    const input = document.getElementById(id);
    return {
        activeElementId: document.activeElement.id,
        value: input.value,
        selectionStart: input.selectionStart,
        selectionEnd: input.selectionEnd,
    };
});
const removeSoftKeyTestControls = ClientFunction(() => {
    const button = document.getElementById('softKeyFocusTestButton');
    if (button) button.remove();
    const editor = document.getElementById('editorDialog');
    if (editor.open) editor.close();
});
const readOutputSemantics = ClientFunction(() => {
    const outputElement = document.getElementById('output');
    return {
        role: outputElement.getAttribute('role'),
        name: outputElement.getAttribute('aria-label'),
        tabIndex: outputElement.tabIndex,
    };
});
const readStoredSettings = ClientFunction(() =>
    JSON.parse(localStorage.getItem('ucblogo.settings')),
);
const readPersistedState = ClientFunction(() => ({
    settings: Object.assign({}, Module.settings),
    fontSize: Module.fontSize,
    cssSize: document.getElementById('logoTextSize').sheet.cssRules[0].style.fontSize,
    shapeRendering: document.getElementById('logoDrawing').style
        .getPropertyValue('shape-rendering'),
    speakOutput: Module.graphics.speakOutput,
    visualBell: Module.visualBell,
    dialog: {
        darkMode: document.getElementById('darkModeSelect').value,
        fontSize: document.getElementById('logoTextSizeInput').value,
        speakOutput: document.getElementById('speakOutputSelect').checked,
        visualBell: document.getElementById('visualBellSelect').checked,
        shapeRenderingMode: document.querySelector('[name="shapeRenderingMode"]:checked').value,
    },
}));
const installOutputProbes = ClientFunction(() => {
    if (typeof Module.speak !== 'function' || !window.speechSynthesis) return false;
    const probe = {
        calls: [],
        speak: Module.speak,
        synthesisSpeak: speechSynthesis.speak,
        tone: window._tone,
    };
    window.__settingsOutputProbe = probe;
    Module.speak = (text) => {
        probe.calls.push(text);
        probe.speak(text);
    };
    speechSynthesis.speak = () => {};
    window._tone = () => {};
    return true;
});
const restoreOutputProbes = ClientFunction(() => {
    const probe = window.__settingsOutputProbe;
    if (!probe) return;
    Module.speak = probe.speak;
    speechSynthesis.speak = probe.synthesisSpeak;
    window._tone = probe.tone;
    delete window.__settingsOutputProbe;
});
const readSpokenOutput = ClientFunction(() => window.__settingsOutputProbe.calls.join(''));
const readBell = ClientFunction(() => {
    const indicator = document.getElementById('bellIndicator');
    return {
        hidden: indicator.hidden,
        visible: indicator.classList.contains('is-visible'),
        text: indicator.textContent,
    };
});

fixture('Settings persistence')
    .page(applicationUrl)
    .beforeEach(async (t) => {
        await ready(t);
        await isolateSettings(t);
    })
    .afterEach(async (t) => {
        await restoreSettings(t);
    });

test('ACC-SET-001 sets, reports, and steps the Logo text size', async (t) => {
    await runLogo(t, 'settextsize 24');
    await t.expect(readTextSize()).eql({
        fontSize: 24,
        cssSize: '24px',
        setting: 24,
        storedSize: 24,
        inputSize: '24',
    });
    await runLogo(t, 'print (word "SIZE textsize)');
    await expectOutput(t, 'SIZE24');
    await runLogo(t, 'increasefont');
    await t.expect(ClientFunction(() => Module.fontSize === 25)()).ok();
    await runLogo(t, 'decreasefont');
    await t.expect(readTextSize()).eql({
        fontSize: 24,
        cssSize: '24px',
        setting: 24,
        storedSize: 24,
        inputSize: '24',
    });
});

test('ACC-SET-001b clamps the Logo text size to a minimum of 8px, also from stored settings', async (t) => {
    await runLogo(t, 'settextsize 3');
    await t.expect(readTextSize()).eql({
        fontSize: 8,
        cssSize: '8px',
        setting: 8,
        storedSize: 8,
        inputSize: '8',
    });
    await ClientFunction(() => localStorage.setItem('ucblogo.settings', JSON.stringify({ fontSize: 5 })))();
    await resetPage();
    await ready(t);
    await t.expect(ClientFunction(() => Module.fontSize)()).eql(8);
});

test('ACC-SET-001c typing a size in the text size box does not apply partial values', async (t) => {
    await t.click(Selector('#appContainer button[aria-controls="settings"]'));
    const input = Selector('#logoTextSizeInput');
    await t.selectText(input).pressKey('delete');
    await t.expect(input.value).eql('');
    await t.expect(ClientFunction(() => Module.fontSize)()).notEql(8, 'empty box must not apply');
    await t.typeText(input, '24', { replace: true });
    await t.expect(readTextSize()).eql({
        fontSize: 24,
        cssSize: '24px',
        setting: 24,
        storedSize: 24,
        inputSize: '24',
    });
    await t.typeText(input, '3', { replace: true }).pressKey('tab');
    await t.expect(readTextSize()).eql({
        fontSize: 8,
        cssSize: '8px',
        setting: 8,
        storedSize: 8,
        inputSize: '8',
    });
});

test('ACC-SET-002 keeps SVG label size independent of Logo text size', async (t) => {
    await runLogo(t, 'settextsize 24');
    await runLogo(t, 'label "SVG');
    const size = await readLabelFontSize();
    await t.expect(size.rendered).eql(size.drawingSize);
});

test('ACC-SET-003 applies dark mode and follows the system preference', async (t) => {
    await runLogo(t, 'setdarkmode "dark');
    await t.expect(readTheme()).eql({
        setting: 'dark',
        attribute: 'dark',
        foreground: 'rgb(229, 231, 235)',
        background: 'rgb(23, 26, 31)',
        buttonBorderStyle: 'solid',
        buttonBorderColor: 'rgb(170, 179, 192)',
    });
    await runLogo(t, 'setdarkmode "system');
    const systemTheme = await ClientFunction(() => ({
        setting: Module.settings.darkMode,
        attribute: document.documentElement.hasAttribute('data-theme'),
        selected: document.getElementById('darkModeSelect').value,
        background: getComputedStyle(document.body).backgroundColor,
        expectedBackground: matchMedia('(prefers-color-scheme: dark)').matches
            ? 'rgb(23, 26, 31)' : 'rgb(255, 255, 255)',
    }))();
    await t.expect(systemTheme).eql({
        setting: 'system',
        attribute: false,
        selected: 'system',
        background: systemTheme.expectedBackground,
        expectedBackground: systemTheme.expectedBackground,
    });
});

test('ACC-SET-004 applies text colors and full flat-control styling', async (t) => {
    const defaultButton = await ClientFunction(() => ({
        colorsApplied: document.documentElement.classList.contains('logo-text-colors-set'),
        appearance: getComputedStyle(document.getElementById('logoRunButton')).appearance,
        disabledAppearance: getComputedStyle(document.getElementById('logoStopButton')).appearance,
    }))();
    await t.expect(defaultButton.colorsApplied).eql(false)
        .expect(defaultButton.appearance).notEql('none')
        .expect(defaultButton.disabledAppearance).notEql('none');

    await runLogo(t, 'settc [15 21 27] [96 84 80]');
    const colors = await readTextControlStyles();
    const inputBorderWidthDevicePixels = colors.inputBorderWidthDevicePixels;
    const disabledBorderWidthDevicePixels = colors.disabledBorderWidthDevicePixels;
    delete colors.inputBorderWidthDevicePixels;
    delete colors.disabledBorderWidthDevicePixels;
    await t.expect(colors).eql({
        foreground: 'rgb(38, 54, 69)',
        background: 'rgb(245, 214, 204)',
        editorForeground: 'rgb(38, 54, 69)',
        editorBackground: 'rgb(245, 214, 204)',
        inputForeground: 'rgb(38, 54, 69)',
        inputBackground: 'rgb(245, 214, 204)',
        buttonForeground: 'rgb(38, 54, 69)',
        buttonBackground: 'rgb(245, 214, 204)',
        editorBorderStyle: 'solid',
        editorBorderColor: 'rgb(38, 54, 69)',
        editorAppearance: 'none',
        editorShadow: 'none',
        inputBorderStyle: 'solid',
        inputBorderColor: 'rgb(38, 54, 69)',
        inputBorderRadius: '0px',
        inputAppearance: 'none',
        inputShadow: 'none',
        inputBackgroundImage: 'none',
        buttonBorderStyle: 'solid',
        buttonBorderColor: 'rgb(38, 54, 69)',
        buttonAppearance: 'none',
        buttonShadow: 'none',
        disabledBorderStyle: 'dashed',
        disabledOpacity: '0.65',
        disabledCursor: 'not-allowed',
        historyBorderStyle: 'solid',
        historyBorderColor: 'rgb(38, 54, 69)',
        historyAppearance: 'auto',
        historyShadow: 'none',
    });
    await t.expect(inputBorderWidthDevicePixels).within(0.99, 1.01)
        .expect(disabledBorderWidthDevicePixels).within(1.99, 2.01);

    await runLogo(t, 'settc [0 35 55] [86 94 98]');
    await t.expect(readTextColors()).eql({
        foreground: 'rgb(0, 89, 140)',
        background: 'rgb(219, 240, 250)',
        inputForeground: 'rgb(0, 89, 140)',
        inputBorder: 'rgb(0, 89, 140)',
    });
});

test('ACC-SET-005 preserves text colors across theme changes', async (t) => {
    await runLogo(t, 'setdarkmode "dark');
    await runLogo(t, 'settextcolor [15 21 27] [96 84 80]');
    const expectedColors = {
        foreground: 'rgb(38, 54, 69)',
        background: 'rgb(245, 214, 204)',
        inputForeground: 'rgb(38, 54, 69)',
        inputBorder: 'rgb(38, 54, 69)',
    };
    await t.expect(readTextColors()).eql(expectedColors);
    await runLogo(t, 'setdarkmode "light');
    await t.expect(readTextColors()).eql(expectedColors)
        .expect(Selector('html').getAttribute('data-theme')).eql('light');
});

test('ACC-SET-006 persists Logo and dialog settings across reload', async (t) => {
    const initialFontSize = await ClientFunction(() => Module.fontSize)();
    await t.expect(initialFontSize).lt(48);
    await t.click(Selector('#appContainer button[aria-controls="settings"]'));
    await setPersistedControls();
    await t.click(Selector('#settings button'));
    await runLogo(t, 'increasefont');
    await t.expect(ClientFunction(() => Module.fontSize)()).eql(initialFontSize + 1);
    const expectedSettings = {
        fontSize: initialFontSize + 1,
        darkMode: 'dark',
        shapeRenderingMode: 'optimizespeed',
        speakOutput: true,
        visualBell: true,
    };
    await t.expect(readStoredSettings()).eql(expectedSettings);
    await resetPage();
    await ready(t);
    await t.expect(readPersistedState()).eql({
        settings: expectedSettings,
        fontSize: initialFontSize + 1,
        cssSize: `${initialFontSize + 1}px`,
        shapeRendering: 'optimizespeed',
        speakOutput: true,
        visualBell: true,
        dialog: {
            darkMode: 'dark',
            fontSize: String(initialFontSize + 1),
            speakOutput: true,
            visualBell: true,
            shapeRenderingMode: 'optimizespeed',
        },
    });
    await t.expect(readStoredSettings()).eql(expectedSettings);

    try {
        await t.expect(installOutputProbes()).ok('Browser speech synthesis support');
        await runLogo(t, 'print "SETTINGS_SPEECH_ACCEPTANCE');
        await expectOutput(t, 'SETTINGS_SPEECH_ACCEPTANCE');
        await t.expect(readSpokenOutput()).contains('SETTINGS_SPEECH_ACCEPTANCE');
        await runLogo(t, 'type char 7');
        await t.expect(readBell()).eql({ hidden: false, visible: true, text: 'BELL' });
    } finally {
        await restoreOutputProbes();
    }
});

test('ACC-UI-001 labels controls and manages settings dialog focus', async (t) => {
    await t.expect(inspectAccessibility()).eql({
        labels: {
            command: 'Logo command',
            history: 'Command history',
            run: 'Run Logo command',
            stop: 'Stop Logo command',
            editor: 'Logo source code',
            theme: 'Color theme',
            fontSize: 'Logo text size (8 to 48 px)',
            speak: 'Speak Logo output',
            visualBell: 'Show visual bell indicator',
            renderingModes: [
                'Geometric precision (anti-aliasing)',
                'Crisp edges (no anti-aliasing)',
                'Optimize speed (no anti-aliasing)',
            ],
        },
        outputRole: 'log',
        outputName: 'Logo output',
        statusRole: 'status',
        settingsName: 'Settings',
        editorName: 'Logo editor',
        settingsButton: 'Settings',
        dialogTitles: [
            'Device orientation permission',
            'Logo information',
            'Logo files',
            'Settings',
            'Logo editor',
            'Logo help',
        ].map((text) => ({
            text,
            display: 'block',
            parentIsDialog: true,
            margin: '0px',
            borderBottomWidth: '2px',
            fontWeight: '700',
        })),
        settingsBodyOverflow: 'auto',
        editorBodyOverflow: 'hidden',
    });
    await runLogo(t, 'setcursor [0 0]');
    await t.expect(readOutputSemantics()).eql({ role: 'log', name: 'Logo output', tabIndex: 0 });
    await t.click(Selector('#appContainer button[aria-controls="settings"]'));
    await t.expect(Selector('#settings').visible).ok();
    const modalState = await ClientFunction(() => ({
        modal: document.getElementById('settings').matches(':modal'),
        focusInside: document.getElementById('settings')
            .contains(document.activeElement),
    }))();
    await t.expect(modalState).eql({ modal: true, focusInside: true });
    await t.click(Selector('#settings button'));
    await t.expect(ClientFunction(() => document.activeElement.textContent.trim())())
        .eql('Settings');
});

test('ACC-UI-002 clicking a soft key replaces selected text and restores focus', async (t) => {
    try {
        const rendered = await prepareSoftKeySelection(false);
        if (rendered) await clickRenderedSoftKey('Insert left square bracket', false);
        else await t.click(Selector('#softKeyFocusTestButton'));
        await t.expect(readTextFocus('logoInputText')).eql({
            activeElementId: 'logoInputText',
            value: 'draw [',
            selectionStart: 6,
            selectionEnd: 6,
        });
    } finally {
        await removeSoftKeyTestControls();
    }
});

test('ACC-UI-003 names light-DOM soft keys and preserves their size', async (t) => {
    const state = await inspectSoftKeys();
    await t.expect(state.hostCount).gt(0)
        .expect(state.initialGroups).eql(state.touchDevice ? state.hostCount : 0)
        .expect(state.groupName).eql('Logo punctuation keys')
        .expect(state.labelCount).eql(0)
        .expect(state.buttons.map((button) => button.name)).eql([
            'Insert double quote',
            'Insert colon',
            'Insert left square bracket',
            'Insert right square bracket',
            'Insert left curly brace',
            'Insert right curly brace',
            'Insert less-than sign',
            'Insert greater-than sign',
            'Insert vertical bar',
            'Insert backtick',
        ]);
    for (const button of state.buttons) {
        await t.expect(button.type).eql('button')
            .expect(button.clickHandler).eql(true)
            .expect(button.mouseDownHandler).eql(false)
            .expect(button.widthEm).within(1.74, 1.76)
            .expect(button.heightEm).within(1.74, 1.76);
    }
});

test('ACC-UI-004 clicking a soft key replaces selected editor text and restores focus', async (t) => {
    try {
        const rendered = await prepareSoftKeySelection(true);
        if (rendered) await clickRenderedSoftKey('Insert backtick', true);
        else await t.click(Selector('#softKeyFocusTestButton'));
        await t.expect(readTextFocus('editorTextArea')).eql({
            activeElementId: 'editorTextArea',
            value: 'abc`def',
            selectionStart: 4,
            selectionEnd: 4,
        });
    } finally {
        await removeSoftKeyTestControls();
    }
});
