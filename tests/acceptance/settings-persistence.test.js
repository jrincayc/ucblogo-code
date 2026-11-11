const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');
const { expect } = require('chai');
const { Builder, By, Key, until } = require('selenium-webdriver');
const chrome = require('selenium-webdriver/chrome');
const firefox = require('selenium-webdriver/firefox');
const { createApplicationServer } = require('./application-server');

const repositoryRoot = path.resolve(__dirname, '../..');
const browserNames = (process.env.BROWSERS || 'chrome,firefox')
    .split(',').map((name) => name.trim()).filter(Boolean);
const headless = process.env.HEADLESS !== '0';
const testDelayMs = Math.max(0, Number(process.env.TEST_DELAY_MS) || 0);
const applicationServer = createApplicationServer();
let applicationUrl;

function createDriver(browserName) {
    const builder = new Builder().forBrowser(browserName);
    if (browserName === 'chrome') {
        const options = new chrome.Options().addArguments('--no-sandbox', '--disable-dev-shm-usage');
        if (headless) options.addArguments('--headless=new');
        if (process.env.CHROME_BINARY) options.setChromeBinaryPath(process.env.CHROME_BINARY);
        builder.setChromeOptions(options);
    } else if (browserName === 'firefox') {
        const options = new firefox.Options();
        if (headless) options.addArguments('-headless');
        if (process.env.FIREFOX_BINARY) options.setBinary(process.env.FIREFOX_BINARY);
        builder.setFirefoxOptions(options);
    } else {
        throw new Error(`Unsupported browser: ${browserName}`);
    }
    return builder.build();
}

async function pauseForViewer(driver) {
    if (testDelayMs) await driver.sleep(testDelayMs);
}

async function clickVisible(driver, locator) {
    await driver.findElement(locator).click();
    await pauseForViewer(driver);
}

async function runLogo(driver, command) {
    const input = await driver.findElement(By.id('logoInputText'));
    await input.clear();
    await input.sendKeys(command, Key.ENTER);
    await pauseForViewer(driver);
}

async function waitForLogoReady(driver) {
    await driver.wait(until.elementLocated(By.id('logoInputText')), 30000);
    await driver.wait(async () => driver.executeScript(() => Boolean(
        window.Module && Module.calledRun && Module.settings &&
        document.getElementById('logoInputText') &&
        !document.getElementById('logoInputText').disabled
    )), 30000, 'Logo runtime did not become ready');
}

async function waitForOutput(driver, expected, message) {
    try {
        await driver.wait(async () => (await driver.findElement(By.id('output')))
            .getText().then((text) => text.includes(expected)), 10000, message);
    } catch (error) {
        const state = await driver.executeScript(() => ({
            output: document.getElementById('output').textContent,
            input: document.getElementById('logoInputText').value,
            stdin: Module.FS_runjs_getChar_buffer.slice(0, 120).map((value) =>
                typeof value === 'string' ? value.charCodeAt(0) : value),
        }));
        throw new Error(`${error.message}\nRunjs browser state: ${JSON.stringify(state)}`);
    }
}

function makeTextControl(value, selectionStart, selectionEnd) {
    return {
        value,
        selectionStart,
        selectionEnd,
        focus() { this.focused = true; },
        setSelectionRange(start, end) {
            this.selectionStart = start;
            this.selectionEnd = end;
        },
    };
}

function loadSoftKeyHandler(controls) {
    const source = fs.readFileSync(path.join(repositoryRoot, 'pre.js'), 'utf8');
    const handler = source.match(/Module\.softKeyDown = \(event, key\) => \{[\s\S]*?\n\};/);
    expect(handler, 'soft-key handler source').not.to.equal(null);
    const Module = {};
    const document = {
        getElementById(id) {
            return controls[id];
        },
    };
    vm.runInNewContext(handler[0], { Module, document });
    return Module.softKeyDown;
}

describe('soft-key accessibility', function () {
    it('provides named click buttons in a named group without changing their size', function () {
        const shell = fs.readFileSync(path.join(repositoryRoot, 'shell.html'), 'utf8');
        const applicationScript = shell.match(/<script type='text\/javascript'>\s*([\s\S]*?)\s*<\/script>/);
        const component = shell.match(/class LogoSoftKeys extends HTMLElement \{([\s\S]*?)customElements\.define\("logo-soft-keys", LogoSoftKeys\);/);
        expect(applicationScript, 'application script source').not.to.equal(null);
        expect(() => new vm.Script(applicationScript[1]), 'application script syntax')
            .not.to.throw();
        expect(component, 'soft-key component source').not.to.equal(null);
        const source = component[1];
        const buttons = [...source.matchAll(/<button\b[^>]*aria-label="([^"]+)"[^>]*onclick="Module\.softKeyDown\(event, /g)];

        expect(source).to.include('role="group" aria-label="Logo punctuation keys"');
        expect(buttons.map((button) => button[1])).to.deep.equal([
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
        expect(source).not.to.include('onmousedown=');
        expect(source).not.to.include('<label');
        expect(source).to.include('height: 1.75em;');
        expect(source).to.include('width: 1.75em;');
    });

    it('replaces selected command text and restores the caret', function () {
        const input = makeTextControl('draw turtle', 5, 11);
        const editor = { open: false };
        const handler = loadSoftKeyHandler({
            logoInputText: input,
            editorDialog: editor,
            editorTextArea: makeTextControl('', 0, 0),
        });

        handler({}, '[');

        expect(input.value).to.equal('draw [');
        expect(input.selectionStart).to.equal(6);
        expect(input.selectionEnd).to.equal(6);
        expect(input.focused).to.equal(true);
    });

    it('replaces selected editor text and restores the caret', function () {
        const editorText = makeTextControl('abcXYZdef', 3, 6);
        const handler = loadSoftKeyHandler({
            logoInputText: makeTextControl('', 0, 0),
            editorDialog: { open: true },
            editorTextArea: editorText,
        });

        handler({}, '`');

        expect(editorText.value).to.equal('abc`def');
        expect(editorText.selectionStart).to.equal(4);
        expect(editorText.selectionEnd).to.equal(4);
        expect(editorText.focused).to.equal(true);
    });
});

describe('settings persistence acceptance', function () {
    this.timeout(120000);

    before(async function () { applicationUrl = await applicationServer.start(); });
    after(() => applicationServer.stop());

    for (const browserName of browserNames) {
        describe(browserName, function () {
            this.timeout(90000);
            let driver;

            before(async function () {
                driver = await createDriver(browserName);
                await driver.manage().setTimeouts({ implicit: 0, pageLoad: 30000 });
            });

            after(async function () {
                if (driver) await driver.quit();
            });

            it('labels application controls and manages settings dialog focus', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                const semantics = await driver.executeScript(() => {
                    const labelText = (element) => [...(element.labels || [])]
                        .map((label) => label.textContent.trim()).join(' ');
                    const labelledByText = (element) => {
                        const heading = document.getElementById(
                            element.getAttribute('aria-labelledby'),
                        );
                        return heading && heading.textContent.trim();
                    };
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
                            renderingModes: [...document.querySelectorAll('[name="shapeRenderingMode"]')]
                                .map(labelText),
                        },
                        outputRole: document.getElementById('output').getAttribute('role'),
                        outputName: document.getElementById('output').getAttribute('aria-label'),
                        statusRole: document.getElementById('status').getAttribute('role'),
                        settingsName: labelledByText(document.getElementById('settings')),
                        editorName: labelledByText(document.getElementById('editorDialog')),
                        settingsButton: settingsButton && settingsButton.textContent.trim(),
                        dialogTitles: [...document.querySelectorAll('dialog .dialogTitle')]
                            .map((title) => ({
                                text: title.textContent.trim(),
                                display: getComputedStyle(title).display,
                                parentIsDialog: title.parentElement.tagName === 'DIALOG',
                                margin: getComputedStyle(title).margin,
                                borderBottomWidth: getComputedStyle(title).borderBottomWidth,
                                fontWeight: getComputedStyle(title).fontWeight,
                            })),
                        settingsBodyOverflow: getComputedStyle(
                            document.querySelector('#settings .dialogBody'),
                        ).overflowY,
                        editorBodyOverflow: getComputedStyle(
                            document.querySelector('#editorDialog .dialogBody'),
                        ).overflowY,
                    };
                });
                expect(semantics).to.deep.equal({
                    labels: {
                        command: 'Logo command',
                        history: 'Command history',
                        run: 'Run Logo command',
                        stop: 'Stop Logo command',
                        editor: 'Logo source code',
                        theme: 'Color theme',
                        fontSize: 'Logo text size (5 to 48 px)',
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

                await runLogo(driver, 'setcursor [0 0]');
                const replacementOutput = await driver.executeScript(() => {
                    const output = document.getElementById('output');
                    return {
                        role: output.getAttribute('role'),
                        name: output.getAttribute('aria-label'),
                        tabIndex: output.tabIndex,
                    };
                });
                expect(replacementOutput).to.deep.equal({
                    role: 'log',
                    name: 'Logo output',
                    tabIndex: 0,
                });

                await clickVisible(driver, By.css('#appContainer button[aria-controls="settings"]'));
                const modalFocus = await driver.executeScript(() => ({
                    isModal: document.getElementById('settings').matches(':modal'),
                    focusIsInside: document.getElementById('settings')
                        .contains(document.activeElement),
                }));
                expect(modalFocus).to.deep.equal({ isModal: true, focusIsInside: true });

                await clickVisible(driver, By.css('#settings button'));
                expect(await driver.executeScript(() => document.activeElement.textContent.trim()))
                    .to.equal('Settings');
            });

            it('binds variadic runjs parameters as values', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                const result = await driver.executeScript(
                    () => Module.runjs('arg0 + ":" + arg1', 'bound value', 42),
                );
                expect(result).to.equal('bound value:42');
            });

            it('returns focus to the Logo command input after a soft-key click', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);
                await driver.executeScript(() => {
                    const input = document.getElementById('logoInputText');
                    input.value = 'draw turtle';
                    input.setSelectionRange(5, 11);
                    const button = document.createElement('button');
                    button.id = 'softKeyFocusTestButton';
                    button.type = 'button';
                    button.setAttribute('aria-label', 'Insert left square bracket');
                    button.style.position = 'fixed';
                    button.style.top = '0';
                    button.style.left = '0';
                    button.addEventListener('click', (event) =>
                        Module.softKeyDown(event, '['));
                    document.body.appendChild(button);
                });

                await clickVisible(driver, By.id('softKeyFocusTestButton'));
                const focusState = await driver.executeScript(() => ({
                    activeElementId: document.activeElement.id,
                    inputValue: document.getElementById('logoInputText').value,
                    selectionStart: document.getElementById('logoInputText').selectionStart,
                    selectionEnd: document.getElementById('logoInputText').selectionEnd,
                }));
                expect(focusState).to.deep.equal({
                    activeElementId: 'logoInputText',
                    inputValue: 'draw [',
                    selectionStart: 6,
                    selectionEnd: 6,
                });
            });

            it('binds variadic runjs parameters from Logo', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);
                await runLogo(driver, 'make "writerBeforeRunjs writer');
                await runLogo(driver,
                    `print (runjs "|arg0 + ':' + arg1 + ':' + arg2| "|bound value| 42 0.5)`,
                );
                await waitForOutput(driver, 'bound value:42:0.5',
                    'Logo runjs did not bind its string and numeric rest inputs');
                await runLogo(driver, 'print equal? :writerBeforeRunjs writer');
                await driver.wait(async () => (await driver.findElement(By.id('output')))
                    .getText().then((text) => text.includes('true')),
                10000, 'runjs changed Logo\'s current writer');
            });

            it('binds a large ordered parameter list from Logo', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                const parameterCount = 128;
                const expression = Array.from({ length: parameterCount }, (_, index) =>
                    `${index + 1} * arg${index}`,
                ).join(' + ');
                const parameters = Array.from({ length: parameterCount }, (_, index) => index + 1);
                const expected = parameters.reduce((sum, value) => sum + value * value, 0);
                const command = `print (runjs "|${expression}| ${parameters.join(' ')})`;

                await runLogo(driver, command);
                await waitForOutput(driver, String(expected),
                    `runjs did not preserve all ${parameterCount} bound Logo parameters in order`);
            });

            it('passes sprite names to make and set turtle commands as bound values', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                await runLogo(driver, 'makesprite "runjsSpriteAcceptance');
                await runLogo(driver, 'setturtlesprite "runjsSpriteAcceptance');
                const spriteState = await driver.executeScript(() => ({
                    exists: Boolean(document.getElementById('logoSpriterunjsSpriteAcceptance')),
                    turtleHref: document.getElementById('logoTurtleUse').getAttribute('href'),
                }));
                expect(spriteState).to.deep.equal({
                    exists: true,
                    turtleHref: '#logoSpriterunjsSpriteAcceptance',
                });
            });

            it('passes speech configuration and speak text as bound values', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);
                const speechSupported = await driver.executeScript(() => {
                    if (!window.speechSynthesis || !Module.utterance || !Module.setSpeechVoice)
                        return false;
                    window.__selectedVoiceName = null;
                    Module.setSpeechVoice = (name) => {
                        window.__selectedVoiceName = name;
                        return true;
                    };
                    window.__runjsSpoken = [];
                    speechSynthesis.speak = (utterance) => {
                        window.__runjsSpoken.push({
                            text: utterance.text,
                            lang: utterance.lang,
                            pitch: utterance.pitch,
                            rate: utterance.rate,
                            voice: utterance.voice && utterance.voice.name,
                        });
                    };
                    return true;
                });
                expect(speechSupported, 'browser speech synthesis support').to.equal(true);

                await runLogo(driver, 'setspeechlang "en-US');
                await runLogo(driver, 'setspeechpitch 1.25');
                await runLogo(driver, 'setspeechrate 0.75');
                await runLogo(driver, 'setspeechvoice "BoundVoiceAcceptance');
                await runLogo(driver, 'speak "|bound speech phrase|');

                await driver.wait(async () => driver.executeScript(() =>
                    window.__runjsSpoken.some((call) => call.text === 'bound speech phrase')
                ), 10000, 'Logo speak did not pass its utterance as a bound value');
                const speechState = await driver.executeScript(() => ({
                    lang: Module.utterance.lang,
                    pitch: Module.utterance.pitch,
                    rate: Module.utterance.rate,
                    voice: window.__selectedVoiceName,
                    spoken: window.__runjsSpoken.at(-1),
                }));
                expect(speechState).to.deep.equal({
                    lang: 'en-US',
                    pitch: 1.25,
                    rate: 0.75,
                    voice: 'BoundVoiceAcceptance',
                    spoken: {
                        text: 'bound speech phrase',
                        lang: 'en-US',
                        pitch: 1.25,
                        rate: 0.75,
                        voice: null,
                    },
                });
            });

            it('parses getvoices results through the Logo-to-JS bridge', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);
                const speechSupported = await driver.executeScript(() => {
                    if (!window.speechSynthesis) return false;
                    speechSynthesis.getVoices = () => [
                        { name: 'GetVoicesAcceptance', lang: 'en-US' },
                    ];
                    return true;
                });
                expect(speechSupported, 'browser speech synthesis support').to.equal(true);

                await runLogo(driver, 'print first first getvoices');
                await waitForOutput(driver, 'GetVoicesAcceptance',
                    'getvoices did not return and parse the BLON result');
            });

            it('keeps hostile bound strings as data across a fuzz corpus', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                const corpus = [
                    `';window.__runjsInjected=true;//`,
                    `line one\nRUNJS_END\nwindow.__runjsInjected=true`,
                    `quotes " ' and slash \\`,
                    `nul\0byte`,
                    `tab\treturn\rline`,
                ];
                const expression = `arg0.length + ':' + arg0.includes('RUNJS_END') + ':' + arg0.includes(String.fromCharCode(10))`;
                for (const payload of corpus) {
                    await driver.executeScript('delete window.__runjsInjected');
                    const logoValue = `(word ${Array.from(payload, (character) =>
                        `char ${character.charCodeAt(0)}`,
                    ).join(' ')})`;
                    await runLogo(driver, `print (runjs "|${expression}| ${logoValue})`);
                    const expected = `${payload.length}:${payload.includes('RUNJS_END')}:${payload.includes('\n')}`;
                    await waitForOutput(driver, expected,
                        `runjs changed fuzz input ${JSON.stringify(payload)}`);
                    expect(await driver.executeScript('return window.__runjsInjected'))
                        .to.equal(null);
                }

                await runLogo(driver, 'make "runjsLongWord "');
                await runLogo(driver, 'repeat 9000 [make "runjsLongWord word :runjsLongWord "x]');
                await runLogo(driver, 'print (runjs "|arg0.length| :runjsLongWord)');
                await waitForOutput(driver, '9000', 'runjs corrupted a long bound Logo word');
            });

            it('runs multiline JavaScript from a v-barred Logo word', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                await runLogo(driver, 'load "/tests/runjs-multiline.lg');
                await waitForOutput(driver, '42',
                    'runjs did not preserve newlines inside a v-barred code word');
            });

            it('sets, reports, and steps the Logo text size', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);
                await driver.executeScript('localStorage.clear()');
                await driver.navigate().refresh();
                await waitForLogoReady(driver);

                await runLogo(driver, 'settextsize 24');
                await driver.wait(async () => driver.executeScript(() =>
                    Module.fontSize === 24 &&
                    document.getElementById('logoTextSize').sheet.cssRules[0]
                        .style.fontSize === '24px'
                ), 10000, 'settextsize did not apply the pixel size');
                await runLogo(driver, 'print (word "SIZE textsize)');
                await waitForOutput(driver, 'SIZE24',
                    'textsize did not report the configured pixel size');

                await runLogo(driver, 'increasefont');
                await driver.wait(async () => driver.executeScript(() =>
                    Module.fontSize === 25
                ), 10000, 'increasefont did not increase the text size');
                await runLogo(driver, 'decreasefont');
                await driver.wait(async () => driver.executeScript(() =>
                    Module.fontSize === 24
                ), 10000, 'decreasefont did not decrease the text size');

                const persistedSize = await driver.executeScript(() => ({
                    fontSize: Module.settings.fontSize,
                    storedSize: JSON.parse(localStorage.getItem('ucblogo.settings'))
                        .fontSize,
                    inputSize: document.querySelector('[name="logoTextSize"]').value,
                }));
                expect(persistedSize).to.deep.equal({
                    fontSize: 24,
                    storedSize: 24,
                    inputSize: '24',
                });
            });

            it('keeps SVG label size independent of Logo text size', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                await runLogo(driver, 'settextsize 24');
                await runLogo(driver, 'label "SVG');

                const labelSize = await driver.executeScript(() => {
                    const labels = document.querySelectorAll('#logoDrawingElements text');
                    const label = labels[labels.length - 1];
                    return {
                        rendered: getComputedStyle(label).fontSize,
                        drawingSize: `${Module.graphics.pen_info.fntsz}px`,
                    };
                });
                expect(labelSize.rendered).to.equal(labelSize.drawingSize);
            });

            it('applies dark mode and follows the system preference', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                await runLogo(driver, 'setdarkmode "dark');
                const darkTheme = await driver.executeScript(() => ({
                    setting: Module.settings.darkMode,
                    attribute: document.documentElement.getAttribute('data-theme'),
                    foreground: getComputedStyle(document.body).color,
                    background: getComputedStyle(document.body).backgroundColor,
                    buttonBorderStyle: getComputedStyle(
                        document.getElementById('logoRunButton'),
                    ).borderStyle,
                    buttonBorderColor: getComputedStyle(
                        document.getElementById('logoRunButton'),
                    ).borderColor,
                }));
                expect(darkTheme).to.deep.equal({
                    setting: 'dark',
                    attribute: 'dark',
                    foreground: 'rgb(229, 231, 235)',
                    background: 'rgb(23, 26, 31)',
                    buttonBorderStyle: 'solid',
                    buttonBorderColor: 'rgb(170, 179, 192)',
                });

                await runLogo(driver, 'setdarkmode "system');
                const systemTheme = await driver.executeScript(() => ({
                    setting: Module.settings.darkMode,
                    attribute: document.documentElement.hasAttribute('data-theme'),
                    selected: document.getElementById('darkModeSelect').value,
                    background: getComputedStyle(document.body).backgroundColor,
                    expectedBackground: matchMedia('(prefers-color-scheme: dark)').matches
                        ? 'rgb(23, 26, 31)' : 'rgb(255, 255, 255)',
                }));
                expect(systemTheme).to.deep.equal({
                    setting: 'system',
                    attribute: false,
                    selected: 'system',
                    background: systemTheme.expectedBackground,
                    expectedBackground: systemTheme.expectedBackground,
                });
            });

            it('sets text colors from palette indices and RGB lists', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                const defaultButton = await driver.executeScript(() => {
                    const button = document.getElementById('logoRunButton');
                    const disabledButton = document.getElementById('logoStopButton');
                    return {
                        colorsApplied: document.documentElement.classList
                            .contains('logo-text-colors-set'),
                        appearance: getComputedStyle(button).appearance,
                        disabledAppearance: getComputedStyle(disabledButton).appearance,
                    };
                });
                expect(defaultButton.colorsApplied).to.equal(false);
                expect(defaultButton.appearance).not.to.equal('none');
                expect(defaultButton.disabledAppearance).not.to.equal('none');

                await runLogo(driver, 'settc [15 21 27] [96 84 80]');
                let colors = await driver.executeScript(() => {
                    const output = document.getElementById('output');
                    const container = document.getElementById('outputContainer');
                    const editor = document.getElementById('editorTextArea');
                    const input = document.getElementById('logoInputText');
                    const button = document.getElementById('logoRunButton');
                    const disabledButton = document.getElementById('logoStopButton');
                    const history = document.getElementById('commandHistorySelect');
                    return {
                        foreground: getComputedStyle(output).color,
                        background: getComputedStyle(container).backgroundColor,
                        editorForeground: getComputedStyle(editor).color,
                        editorBackground: getComputedStyle(editor).backgroundColor,
                        inputForeground: getComputedStyle(input).color,
                        inputBackground: getComputedStyle(input).backgroundColor,
                        buttonForeground: getComputedStyle(button).color,
                        buttonBackground: getComputedStyle(button).backgroundColor,
                        editorBorderStyle: getComputedStyle(editor).borderStyle,
                        editorBorderColor: getComputedStyle(editor).borderColor,
                        editorAppearance: getComputedStyle(editor).appearance,
                        editorShadow: getComputedStyle(editor).boxShadow,
                        inputBorderStyle: getComputedStyle(input).borderStyle,
                        inputBorderColor: getComputedStyle(input).borderColor,
                        inputBorderRadius: getComputedStyle(input).borderRadius,
                        inputBorderWidthDevicePixels: Number.parseFloat(
                            getComputedStyle(input).borderWidth,
                        ) * window.devicePixelRatio,
                        inputAppearance: getComputedStyle(input).appearance,
                        inputShadow: getComputedStyle(input).boxShadow,
                        inputBackgroundImage: getComputedStyle(input).backgroundImage,
                        buttonBorderStyle: getComputedStyle(button).borderStyle,
                        buttonBorderColor: getComputedStyle(button).borderColor,
                        buttonAppearance: getComputedStyle(button).appearance,
                        buttonShadow: getComputedStyle(button).boxShadow,
                        disabledBorderStyle: getComputedStyle(disabledButton).borderStyle,
                        disabledBorderWidthDevicePixels: Number.parseFloat(
                            getComputedStyle(disabledButton).borderWidth,
                        ) * window.devicePixelRatio,
                        disabledOpacity: getComputedStyle(disabledButton).opacity,
                        disabledCursor: getComputedStyle(disabledButton).cursor,
                        historyBorderStyle: getComputedStyle(history).borderStyle,
                        historyBorderColor: getComputedStyle(history).borderColor,
                        historyAppearance: getComputedStyle(history).appearance,
                        historyShadow: getComputedStyle(history).boxShadow,
                    };
                });
                const {
                    inputBorderWidthDevicePixels,
                    disabledBorderWidthDevicePixels,
                    ...colorStyles
                } = colors;
                expect(colorStyles).to.deep.equal({
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
                expect(inputBorderWidthDevicePixels).to.be.closeTo(1, 0.01);
                expect(disabledBorderWidthDevicePixels).to.be.closeTo(2, 0.01);

                await runLogo(driver, 'settc [0 35 55] [86 94 98]');
                colors = await driver.executeScript(() => {
                    const output = document.getElementById('output');
                    const container = document.getElementById('outputContainer');
                    const input = document.getElementById('logoInputText');
                    return {
                        foreground: getComputedStyle(output).color,
                        background: getComputedStyle(container).backgroundColor,
                        inputForeground: getComputedStyle(input).color,
                        inputBorderColor: getComputedStyle(input).borderColor,
                    };
                });
                expect(colors).to.deep.equal({
                    foreground: 'rgb(0, 89, 140)',
                    background: 'rgb(219, 240, 250)',
                    inputForeground: 'rgb(0, 89, 140)',
                    inputBorderColor: 'rgb(0, 89, 140)',
                });
            });

            it('keeps settextcolor colors when dark mode changes', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                await runLogo(driver, 'setdarkmode "dark');
                await runLogo(driver, 'settextcolor [15 21 27] [96 84 80]');
                const colorsInDarkMode = await driver.executeScript(() => ({
                    foreground: getComputedStyle(document.getElementById('output')).color,
                    background: getComputedStyle(
                        document.getElementById('outputContainer'),
                    ).backgroundColor,
                    inputBorder: getComputedStyle(
                        document.getElementById('logoInputText'),
                    ).borderColor,
                }));
                expect(colorsInDarkMode).to.deep.equal({
                    foreground: 'rgb(38, 54, 69)',
                    background: 'rgb(245, 214, 204)',
                    inputBorder: 'rgb(38, 54, 69)',
                });

                await runLogo(driver, 'setdarkmode "light');
                const colorsAfterThemeChange = await driver.executeScript(() => ({
                    foreground: getComputedStyle(document.getElementById('output')).color,
                    background: getComputedStyle(
                        document.getElementById('outputContainer'),
                    ).backgroundColor,
                    inputBorder: getComputedStyle(
                        document.getElementById('logoInputText'),
                    ).borderColor,
                    theme: document.documentElement.getAttribute('data-theme'),
                }));
                expect(colorsAfterThemeChange).to.deep.equal({
                    foreground: 'rgb(38, 54, 69)',
                    background: 'rgb(245, 214, 204)',
                    inputBorder: 'rgb(38, 54, 69)',
                    theme: 'light',
                });
            });

            it('persists Logo and dialog settings and restores their effects after reload', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);
                await pauseForViewer(driver);

                await driver.executeScript('localStorage.clear()');
                await driver.navigate().refresh();
                await waitForLogoReady(driver);
                await pauseForViewer(driver);

                const initialFontSize = await driver.executeScript(() => Module.fontSize);
                expect(initialFontSize).to.be.lessThan(48);

                await clickVisible(driver, By.css('button[onclick*="settings"]'));
                await clickVisible(driver, By.css('#darkModeSelect option[value="dark"]'));
                await clickVisible(driver, By.css(
                    'input[name="shapeRenderingMode"][value="optimizespeed"]',
                ));
                await clickVisible(driver, By.id('speakOutputSelect'));
                await clickVisible(driver, By.id('visualBellSelect'));
                await clickVisible(driver, By.css('#settings button'));

                await runLogo(driver, 'increasefont');
                await driver.wait(async () => driver.executeScript(
                    (expected) => Module.fontSize === expected + 1,
                    initialFontSize,
                ), 10000, 'increasefont did not change the font size');

                const settingsBeforeReload = await driver.executeScript(() =>
                    JSON.parse(localStorage.getItem('ucblogo.settings')),
                );
                expect(settingsBeforeReload).to.include({
                    fontSize: initialFontSize + 1,
                    darkMode: 'dark',
                    shapeRenderingMode: 'optimizespeed',
                    speakOutput: true,
                    visualBell: true,
                });

                await driver.navigate().refresh();
                await waitForLogoReady(driver);
                await pauseForViewer(driver);
                await driver.wait(async () => driver.executeScript(() =>
                    document.getElementById('speakOutputSelect').checked &&
                    document.getElementById('visualBellSelect').checked &&
                    document.getElementById('darkModeSelect').value === 'dark' &&
                    document.querySelector(
                        '[name="shapeRenderingMode"]:checked',
                    ).value === 'optimizespeed'
                ), 10000, 'settings dialog did not restore persisted preferences');

                const restoredEffects = await driver.executeScript(() => ({
                    fontSize: document.querySelector('#logoTextSize').sheet
                        .cssRules[0].style.fontSize,
                    shapeRendering: document.getElementById('logoDrawing').style
                        .getPropertyValue('shape-rendering'),
                    speakOutput: Module.graphics.speakOutput,
                    visualBell: Module.visualBell,
                }));
                expect(restoredEffects).to.deep.equal({
                    fontSize: `${initialFontSize + 1}px`,
                    shapeRendering: 'optimizespeed',
                    speakOutput: true,
                    visualBell: true,
                });

                const speechSupported = await driver.executeScript(() => {
                    if (typeof Module.speak !== 'function' || !window.speechSynthesis)
                        return false;
                    window.__logoSpeechCalls = [];
                    const speak = Module.speak;
                    Module.speak = (text) => {
                        window.__logoSpeechCalls.push(text);
                        speak(text);
                    };
                    window._tone = () => {};
                    return true;
                });
                expect(speechSupported, 'browser speech synthesis support').to.equal(true);

                await runLogo(driver, 'print "SETTINGS_SPEECH_ACCEPTANCE');
                await driver.wait(async () => driver.executeScript(() =>
                    window.__logoSpeechCalls.some((text) =>
                        text.includes('SETTINGS_SPEECH_ACCEPTANCE'))
                ), 10000, 'Logo output was not sent to speech synthesis');

                await runLogo(driver, 'type char 7');
                await driver.wait(async () => driver.executeScript(() => {
                    const indicator = document.getElementById('bellIndicator');
                    return !indicator.hidden && indicator.classList.contains('is-visible');
                }), 5000, 'printing the bell character did not show the visual bell');
                expect(await driver.findElement(By.id('bellIndicator'))
                    .getAttribute('textContent')).to.equal('BELL');
            });
        });
    }
});