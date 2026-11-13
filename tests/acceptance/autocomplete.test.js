const { expect } = require('chai');
const { Builder, By, Key, until } = require('selenium-webdriver');
const chrome = require('selenium-webdriver/chrome');
const { createApplicationServer } = require('./application-server');

const headless = process.env.HEADLESS !== '0';
const testDelayMs = Math.max(0, Number(process.env.TEST_DELAY_MS) || 0);
const applicationServer = createApplicationServer();

function createDriver() {
    const options = new chrome.Options().addArguments('--no-sandbox', '--disable-dev-shm-usage');
    if (headless) options.addArguments('--headless=new');
    if (process.env.CHROME_BINARY) options.setChromeBinaryPath(process.env.CHROME_BINARY);
    return new Builder().forBrowser('chrome').setChromeOptions(options).build();
}

async function pauseForViewer(driver) {
    if (testDelayMs) await driver.sleep(testDelayMs);
}

async function waitForLogoIdle(driver) {
    await driver.wait(async () => driver.executeScript(() => Boolean(
        window.Module && Module.calledRun && Asyncify.state === 0 &&
        getValue(Module._LOGO_GIL) === 0
    )), 30000, 'Logo did not become idle');
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
        await input.sendKeys(character);
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
        document.querySelector('logo-autocomplete').provider = (text) =>
            stub.filter((name) => name.startsWith(text.toLowerCase()))
                .map((name) => ({ name, kind: 'primitive' }));
    }, names);
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
            status: root.querySelector('[role="status"]').textContent,
            value: input.value,
            caret: input.selectionStart,
            focused: document.activeElement === input,
        };
    });
}

describe('Logo word autocompletion', function () {
    this.timeout(120000);
    let driver;
    let url;

    before(async function () {
        url = await applicationServer.start();
        driver = createDriver();
        await driver.get(url);
        await waitForLogoIdle(driver);
        await pauseForViewer(driver);
    });

    after(async function () {
        if (driver) await driver.quit();
        await applicationServer.stop();
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
            const option = await driver.executeScript(() => [...document.querySelector('logo-autocomplete')
                .shadowRoot.querySelectorAll('[role="option"]')].find((o) => o.querySelector('.name').textContent === 'forward'));
            await option.click();
            await pauseForViewer(driver);
            const state = await popupState(driver);
            expect(state.value).to.equal('repeat 4 [forward ');
            expect(state.caret).to.equal(state.value.length);
            expect(state.focused).to.equal(true);
            expect(state.open).to.equal(false);
        });

        it('shows a one-sentence help summary under each documented word', async function () {
            await useStubNames(driver, ['forward', 'make', 'nohelpword']);
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
            await typeSlowly(driver, 'n');
            state = await popupState(driver);
            expect(state.summaries.nohelpword).to.equal(null);
        });

        it('closes after the command is submitted', async function () {
            await typeSlowly(driver, 'fo');
            await driver.findElement(By.id('logoInputText')).sendKeys(Key.ENTER);
            await waitForLogoIdle(driver);
            expect((await popupState(driver)).open).to.equal(false);
        });
    });
});
