const { expect } = require('chai');
const fs = require('node:fs');
const path = require('node:path');
const { Builder, By, Key, input: pointerInput } = require('selenium-webdriver');
const chrome = require('selenium-webdriver/chrome');
const { createApplicationServer } = require('./application-server');

const headless = process.env.HEADLESS !== '0';
const testDelayMs = Math.max(0, Number(process.env.TEST_DELAY_MS) || 0);
const applicationServer = createApplicationServer();
const screenshotDirectory = process.env.AUTOCOMPLETE_SCREENSHOT_DIR ||
    path.join(__dirname, '../../reports/autocomplete');

async function saveScreenshot(driver, name) {
    fs.mkdirSync(screenshotDirectory, { recursive: true });
    const file = path.join(screenshotDirectory, `${name}.png`);
    fs.writeFileSync(file, await driver.takeScreenshot(), 'base64');
    return file;
}

const modes = {
    desktop: { width: 1280, height: 800 },
    phone: { width: 390, height: 844, mobile: true },
};

async function createDriver(mode) {
    const options = new chrome.Options().addArguments('--no-sandbox', '--disable-dev-shm-usage');
    if (headless) options.addArguments('--headless=new');
    if (process.env.CHROME_BINARY) options.setChromeBinaryPath(process.env.CHROME_BINARY);
    const driver = await new Builder().forBrowser('chrome').setChromeOptions(options).build();
    await driver.manage().window().setRect({ width: modes.desktop.width, height: modes.desktop.height });
    if (mode.mobile) await enableResponsiveMode(driver, mode);
    driver.isPhone = Boolean(mode.mobile);
    return driver;
}

async function touchTap(driver, element) {
    const finger = new pointerInput.Pointer('finger', pointerInput.Pointer.Type.TOUCH);
    await driver.actions({ async: true })
        .insert(finger, finger.move({ origin: element }), finger.press(), finger.release())
        .perform();
}

// Same DevTools Protocol calls as DevTools device (responsive) mode.
// Never scale: WebDriver click coordinates ignore the emulation scale and hit the wrong element.
async function enableResponsiveMode(driver, mode) {
    const available = await driver.executeScript(() => window.innerHeight);
    const height = Math.min(mode.height, available);
    await driver.sendDevToolsCommand('Emulation.setDeviceMetricsOverride', {
        width: mode.width, height, deviceScaleFactor: 3, mobile: true,
        screenWidth: mode.width, screenHeight: height,
    });
    await driver.sendDevToolsCommand('Emulation.setTouchEmulationEnabled', { enabled: true, maxTouchPoints: 5 });
}

async function pauseForViewer(driver) {
    if (testDelayMs) await driver.sleep(testDelayMs);
}

function trace(message) {
    if (process.env.TRACE) console.log(`        [${new Date().toISOString().slice(11, 23)}] ${message}`);
}

const stepTimeoutMs = 2000;

async function step(label, action) {
    trace(label);
    let timer;
    const watchdog = new Promise((_, reject) => {
        timer = setTimeout(() => reject(new Error(`step hung for ${stepTimeoutMs} ms: ${label}`)), stepTimeoutMs);
    });
    try {
        return await Promise.race([action(), watchdog]);
    } finally {
        clearTimeout(timer);
    }
}

async function waitForLogoIdle(driver) {
    try {
        await driver.wait(async () => {
            const state = await driver.executeScript(() => ({
                idle: Boolean(window.Module && Module.calledRun && Asyncify.state === 0 &&
                    getValue(Module._LOGO_GIL) === 0),
                blocked: Boolean(window.Module && Module.blockedOnGetc),
            }));
            if (state.blocked) throw new Error('Logo is blocked reading input (continuation > prompt?)');
            return state.idle;
        }, stepTimeoutMs, 'Logo did not become idle');
    } catch (error) {
        const state = await driver.executeScript(() => ({
            output: document.getElementById('output').textContent.slice(-300),
            input: document.getElementById('logoInputText').value,
            blockedOnGetc: Module.blockedOnGetc,
        }));
        throw new Error(`${error.message}\n${JSON.stringify(state)}`);
    }
}

async function runLogo(driver, command) {
    const input = await driver.findElement(By.id('logoInputText'));
    await input.clear();
    await input.sendKeys(command, Key.ENTER);
    await waitForLogoIdle(driver);
    await pauseForViewer(driver);
}

async function typeSlowly(driver, text) {
    const input = await driver.findElement(By.id('logoInputText'));
    for (const character of text) {
        await step(`type ${JSON.stringify(character)}`, () => input.sendKeys(character));
        if (testDelayMs) await driver.sleep(testDelayMs / 4);
    }
    await pauseForViewer(driver);
    return input;
}

async function resetInput(driver) {
    await driver.executeScript(() => {
        const input = document.getElementById('logoInputText');
        input.value = '';
        input.dispatchEvent(new Event('input'));
        input.focus();
    });
}

async function useStubNames(driver, names) {
    await driver.executeScript((stub) => {
        const autocomplete = document.querySelector('logo-autocomplete');
        autocomplete.liveProvider ??= autocomplete.provider;
        autocomplete.provider = (text) =>
            stub.filter((name) => name.startsWith(text.toLowerCase()))
                .map((name) => ({ name, kind: 'primitive' }));
    }, names);
}

async function useLiveNames(driver) {
    await driver.executeScript(() => {
        const autocomplete = document.querySelector('logo-autocomplete');
        autocomplete.provider = autocomplete.liveProvider ?? autocomplete.provider;
    });
}

async function tapOption(driver, name) {
    const option = await step(`tap ${name}: find`, () => driver.executeScript((wanted) => [...document.querySelector('logo-autocomplete')
        .shadowRoot.querySelectorAll('[role="option"]')]
        .find((candidate) => candidate.querySelector('.name').textContent === wanted), name));
    expect(option, `suggestion ${name}`).to.not.equal(null);
    await step(`tap ${name}: ${driver.isPhone ? 'touch' : 'click'}`, () => driver.isPhone
        ? touchTap(driver, option)
        : option.click());
    trace(`tap ${name}: done`);
    await pauseForViewer(driver);
}

async function popupState(driver) {
    return driver.executeScript(() => {
        const input = document.getElementById('logoInputText');
        const root = document.querySelector('logo-autocomplete').shadowRoot;
        const listbox = root.querySelector('[role="listbox"]');
        return {
            open: !listbox.hidden,
            expanded: input.getAttribute('aria-expanded'),
            role: input.getAttribute('role'),
            autocomplete: input.getAttribute('aria-autocomplete'),
            options: [...listbox.querySelectorAll('[role="option"]')].map((option) =>
                option.querySelector('.name').textContent),
            summaries: Object.fromEntries([...listbox.querySelectorAll('[role="option"]')].map((option) =>
                [option.querySelector('.name').textContent, option.querySelector('.summary')?.textContent ?? null])),
            usages: Object.fromEntries([...listbox.querySelectorAll('[role="option"]')].map((option) =>
                [option.querySelector('.name').textContent,
                    [...option.querySelectorAll('.signature')].map((signature) => signature.textContent)])),
            status: root.querySelector('[role="status"]').textContent,
            value: input.value,
            caret: input.selectionStart,
            focused: document.activeElement === input,
            activeElement: document.activeElement?.id || document.activeElement?.tagName,
        };
    });
}

async function optionLayout(driver, name) {
    return driver.executeScript((wanted) => {
        const root = document.querySelector('logo-autocomplete').shadowRoot;
        const listbox = root.querySelector('[role="listbox"]');
        const option = [...listbox.querySelectorAll('[role="option"]')]
            .find((candidate) => candidate.querySelector('.name').textContent === wanted);
        const box = (element) => element.getBoundingClientRect().toJSON();
        return {
            viewportWidth: document.documentElement.clientWidth,
            listbox: box(listbox),
            listboxOverflow: listbox.scrollWidth - listbox.clientWidth,
            option: box(option),
            optionOverflow: option.scrollWidth - option.clientWidth,
            name: box(option.querySelector('.name')),
            usage: box(option.querySelector('.usage')),
            summary: box(option.querySelector('.summary')),
        };
    }, name);
}

describe('Logo word autocompletion', function () {
    this.timeout(testDelayMs ? 120000 : 30000);
    let url;

    before(async function () {
        url = await applicationServer.start();
    });

    after(async function () {
        await applicationServer.stop();
    });

    for (const [modeName, mode] of Object.entries(modes)) describe(`${modeName} (${mode.width}x${mode.height})`, function () {
    let driver;

    before(async function () {
        driver = await createDriver(mode);
        await driver.get(url);
        await waitForLogoIdle(driver);
        await pauseForViewer(driver);
    });

    afterEach(async function () {
        if (this.currentTest.state === 'failed' && driver)
            console.error('popup state at failure:', JSON.stringify(await popupState(driver)));
    });

    after(async function () {
        if (driver) await driver.quit();
    });

    describe('symbol source', function () {
        it('lists primitives, user procedures, variables, and library procedures', async function () {
            await runLogo(driver, 'define "squiggle [[] [fd 1]] make "zebra 3');
            const names = await driver.executeScript(() => {
                const byName = {};
                for (const entry of Module.completion.names()) byName[entry.name] = entry.kind;
                return byName;
            });
            expect(names.forward).to.equal('primitive');
            expect(names.squiggle).to.equal('procedure');
            expect(names.zebra).to.equal('variable');
            expect(names.foreach).to.equal('library');
        });
    });

    describe('popup with stub names', function () {
        beforeEach(async function () {
            await useStubNames(driver, ['forward', 'foreach', 'format', 'back']);
            await resetInput(driver);
        });

        it('is a closed combobox until a matching word is typed', async function () {
            const state = await popupState(driver);
            expect(state.role).to.equal('combobox');
            expect(state.autocomplete).to.equal('list');
            expect(state.expanded).to.equal('false');
            expect(state.open).to.equal(false);
        });

        it('shows matching suggestions while typing', async function () {
            await typeSlowly(driver, 'fo');
            let state = await popupState(driver);
            expect(state.open).to.equal(true);
            expect(state.expanded).to.equal('true');
            expect(state.options).to.have.members(['forward', 'foreach', 'format']);
            expect(state.status).to.match(/3 suggestions/);
            await typeSlowly(driver, 'rw');
            state = await popupState(driver);
            expect(state.options).to.deep.equal(['forward']);
        });

        it('closes when nothing matches', async function () {
            await typeSlowly(driver, 'zz');
            expect((await popupState(driver)).open).to.equal(false);
        });

        it('completes only the word at the caret when a suggestion is tapped', async function () {
            await typeSlowly(driver, 'repeat 4 [fo');
            expect((await popupState(driver)).options).to.include('forward');
            await tapOption(driver, 'forward');
            const state = await popupState(driver);
            expect(state.value).to.equal('repeat 4 [forward ');
            expect(state.caret).to.equal(state.value.length);
            expect(state.focused).to.equal(true);
            expect(state.open).to.equal(false);
        });

        it('shows a one-sentence help summary under each documented word', async function () {
            await useStubNames(driver, ['forward', 'make', 'squiggle']);
            await typeSlowly(driver, 'f');
            let state = await popupState(driver);
            expect(state.summaries.forward).to.equal(
                'Moves the turtle forward, in the direction that it\'s facing, by the specified distance (measured in turtle steps).');
            await resetInput(driver);
            await typeSlowly(driver, 'm');
            state = await popupState(driver);
            expect(state.summaries.make).to.equal(
                'Assigns the value "value" to the variable named "varname", which must be a word.');
            await resetInput(driver);
            await typeSlowly(driver, 's');
            state = await popupState(driver);
            expect(state.summaries.squiggle).to.equal(null);
        });

        it('shows the argument syntax for each documented word', async function () {
            await useStubNames(driver, ['forward', 'foreach', 'squiggle']);
            await typeSlowly(driver, 'f');
            const state = await popupState(driver);
            expect(state.usages.forward).to.deep.equal(['FORWARD dist', 'FD dist']);
            expect(state.usages.foreach).to.deep.equal(
                ['FOREACH data template', '(FOREACH data1 data2 ... template)']);
            await resetInput(driver);
            await typeSlowly(driver, 's');
            expect((await popupState(driver)).usages.squiggle).to.deep.equal([]);
        });

        it('closes after the command is submitted', async function () {
            await typeSlowly(driver, 'fo');
            await driver.findElement(By.id('logoInputText')).sendKeys(Key.ENTER);
            await waitForLogoIdle(driver);
            expect((await popupState(driver)).open).to.equal(false);
        });
    });

    describe('popup with live Logo names', function () {
        beforeEach(async function () {
            await useLiveNames(driver);
            await resetInput(driver);
        });

        it('stays closed while Logo is reading continuation lines', async function () {
            const input = await driver.findElement(By.id('logoInputText'));
            await input.sendKeys('print [', Key.ENTER);
            await driver.wait(async () => driver.executeScript(() => Module.blockedOnGetc === true),
                10000, 'Logo did not block for a continuation line');
            await pauseForViewer(driver);
            await typeSlowly(driver, 'fo');
            expect((await popupState(driver)).open).to.equal(false);
            await input.sendKeys(']', Key.ENTER);
            await waitForLogoIdle(driver);
            await resetInput(driver);
            await typeSlowly(driver, 'forw');
            expect((await popupState(driver)).options).to.include('forward');
        });

        it('stays closed while a Logo program is running', async function () {
            const input = await driver.findElement(By.id('logoInputText'));
            await input.sendKeys('wait 180', Key.ENTER);
            await driver.wait(async () => driver.executeScript(() => getValue(Module._LOGO_GIL, 'i32') !== 0),
                5000, 'Logo did not start running');
            await typeSlowly(driver, 'fo');
            expect((await popupState(driver)).open).to.equal(false);
            await waitForLogoIdle(driver);
            await resetInput(driver);
            await typeSlowly(driver, 'forw');
            expect((await popupState(driver)).options).to.include('forward');
        });

        it('completes a real command from primitives with help', async function () {
            await typeSlowly(driver, 'rep');
            let state = await popupState(driver);
            expect(state.options).to.include('repeat');
            expect(state.usages.repeat).to.deep.equal(['REPEAT num instructionlist']);
            expect(state.summaries.repeat).to.equal('Runs the "instructionlist" repeatedly, "num" times.');
            await tapOption(driver, 'repeat');
            await typeSlowly(driver, '4 [forw');
            await tapOption(driver, 'forward');
            await typeSlowly(driver, '50 righ');
            state = await popupState(driver);
            expect(state.usages.right).to.deep.equal(['RIGHT degrees', 'RT degrees']);
            await tapOption(driver, 'right');
            await typeSlowly(driver, '90]');
            state = await popupState(driver);
            expect(state.value).to.equal('repeat 4 [forward 50 right 90]');
            expect(state.open).to.equal(false);
            await driver.findElement(By.id('logoInputText')).sendKeys(Key.ENTER);
            await waitForLogoIdle(driver);
            const output = await driver.findElement(By.id('output')).getText();
            expect(output).to.include('repeat 4 [forward 50 right 90]');
            expect(output).to.not.match(/I don't know how/);
        });

        it('suggests procedures and variables defined in the session', async function () {
            await runLogo(driver, 'define "zigzag [[] [fd 1]] make "zeppelin 3');
            await resetInput(driver);
            await typeSlowly(driver, 'zi');
            expect((await popupState(driver)).options).to.include('zigzag');
            await resetInput(driver);
            await typeSlowly(driver, 'print :zep');
            expect((await popupState(driver)).options).to.include('zeppelin');
            await tapOption(driver, 'zeppelin');
            expect((await popupState(driver)).value).to.equal('print :zeppelin ');
        });
    });

    describe('layout', function () {
        it('keeps the command input and popup inside the window', async function () {
            const visible = () => driver.executeScript(() => {
                const box = (element) => element.getBoundingClientRect().toJSON();
                const listbox = document.querySelector('logo-autocomplete').shadowRoot
                    .querySelector('[role="listbox"]');
                return {
                    viewportHeight: window.visualViewport?.height ?? window.innerHeight,
                    innerHeight: window.innerHeight,
                    outerHeight: window.outerHeight,
                    screenAvailHeight: screen.availHeight,
                    scrollY: window.scrollY,
                    input: box(document.getElementById('logoInputText')),
                    listbox: listbox.hidden ? null : box(listbox),
                };
            });
            await useStubNames(driver, ['forward', 'foreach']);
            await resetInput(driver);
            let state = await visible();
            console.log(`      viewport: ${JSON.stringify(state)}`);
            console.log(`      screenshot: ${await saveScreenshot(driver, `${modeName}-idle`)}`);
            expect(state.input.top).to.be.at.least(0);
            expect(state.input.bottom).to.be.at.most(state.viewportHeight + 0.5);
            await typeSlowly(driver, 'f');
            state = await visible();
            console.log(`      screenshot: ${await saveScreenshot(driver, `${modeName}-popup`)}`);
            expect(state.listbox).to.not.equal(null);
            expect(state.scrollY).to.equal(0);
            expect(state.input.bottom).to.be.at.most(state.viewportHeight + 0.5);
            expect(state.listbox.top).to.be.at.least(0);
            expect(state.listbox.bottom).to.be.at.most(state.input.top + 0.5);
        });
    });

    describe('option layout', function () {
        beforeEach(async function () {
            await useStubNames(driver, ['forward', 'foreach']);
            await resetInput(driver);
            await typeSlowly(driver, 'f');
        });

        it('fits the popup within the viewport without horizontal scrolling', async function () {
            for (const name of ['forward', 'foreach']) {
                const layout = await optionLayout(driver, name);
                expect(layout.listbox.left).to.be.at.least(0);
                expect(layout.listbox.right).to.be.at.most(layout.viewportWidth + 0.5);
                expect(layout.listboxOverflow).to.be.at.most(0);
                expect(layout.optionOverflow).to.be.at.most(0);
                expect(layout.summary.top).to.be.at.least(layout.name.bottom - 0.5);
            }
        });

        it('keeps touch targets at least 44 CSS pixels tall', async function () {
            const layout = await optionLayout(driver, 'forward');
            expect(layout.option.height).to.be.at.least(44);
        });

        if (mode.mobile) {
            it('wraps long argument syntax below the word in portrait', async function () {
                const layout = await optionLayout(driver, 'foreach');
                expect(layout.usage.top).to.be.at.least(layout.name.bottom - 0.5);
            });
        } else {
            it('shows argument syntax on the same line as the word', async function () {
                for (const name of ['forward', 'foreach']) {
                    const layout = await optionLayout(driver, name);
                    expect(layout.usage.top).to.be.below(layout.name.bottom);
                    expect(layout.usage.left).to.be.at.least(layout.name.right);
                }
            });
        }
    });
    });
});
