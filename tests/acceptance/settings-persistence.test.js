const { spawn } = require('node:child_process');
const net = require('node:net');
const path = require('node:path');
const { expect } = require('chai');
const { Builder, By, Key, until } = require('selenium-webdriver');
const chrome = require('selenium-webdriver/chrome');
const firefox = require('selenium-webdriver/firefox');

const repositoryRoot = path.resolve(__dirname, '../..');
const browserNames = (process.env.BROWSERS || 'chrome,firefox')
    .split(',').map((name) => name.trim()).filter(Boolean);
const headless = process.env.HEADLESS !== '0';
const testDelayMs = Math.max(0, Number(process.env.TEST_DELAY_MS) || 0);
let emrunProcess;
let applicationUrl;

function findOpenPort() {
    return new Promise((resolve, reject) => {
        const server = net.createServer();
        server.once('error', reject);
        server.listen(0, '127.0.0.1', () => {
            const { port } = server.address();
            server.close((error) => error ? reject(error) : resolve(port));
        });
    });
}

async function startApplicationServer() {
    if (process.env.UCBOLOGO_TEST_URL) {
        applicationUrl = process.env.UCBOLOGO_TEST_URL;
        return;
    }

    const port = await findOpenPort();
    applicationUrl = `http://127.0.0.1:${port}/ucblogo.html`;
    let launchError;
    emrunProcess = spawn(process.env.EMRUN || 'emrun', [
        '--no-browser', '--port', String(port), 'ucblogo.html',
    ], { cwd: repositoryRoot, stdio: 'inherit' });
    emrunProcess.once('error', (error) => { launchError = error; });

    const deadline = Date.now() + 30000;
    while (Date.now() < deadline) {
        if (launchError) throw launchError;
        if (emrunProcess.exitCode !== null)
            throw new Error(`emrun exited with code ${emrunProcess.exitCode}`);
        try {
            const response = await fetch(applicationUrl);
            if (response.ok) return;
        } catch (error) {}
        await new Promise((resolve) => setTimeout(resolve, 200));
    }
    throw new Error(`Timed out waiting for emrun at ${applicationUrl}`);
}

async function stopApplicationServer() {
    if (!emrunProcess || emrunProcess.exitCode !== null) return;
    const exited = new Promise((resolve) => emrunProcess.once('exit', resolve));
    emrunProcess.kill('SIGTERM');
    await Promise.race([exited, new Promise((resolve) => setTimeout(resolve, 3000))]);
}

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

describe('settings persistence acceptance', function () {
    this.timeout(120000);

    before(startApplicationServer);
    after(stopApplicationServer);

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

            it('binds variadic runjs parameters as values', async function () {
                await driver.get(applicationUrl);
                await waitForLogoReady(driver);

                const result = await driver.executeScript(
                    () => Module.runjs('arg0 + ":" + arg1', 'bound value', 42),
                );
                expect(result).to.equal('bound value:42');
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