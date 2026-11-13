const { expect } = require('chai');
const { Builder, By, Key, until } = require('selenium-webdriver');
const chrome = require('selenium-webdriver/chrome');
const { createApplicationServer } = require('./application-server');

const headless = process.env.HEADLESS !== '0';
const testDelayMs = Math.max(0, Number(process.env.TEST_DELAY_MS) || 0);
const applicationServer = createApplicationServer();

const modes = {
    desktop: { width: 1280, height: 800 },
    phone: { width: 390, height: 844, mobile: true },
};

async function createDriver(mode) {
    const options = new chrome.Options().addArguments('--no-sandbox', '--disable-dev-shm-usage');
    if (headless) options.addArguments('--headless=new');
    if (process.env.CHROME_BINARY) options.setChromeBinaryPath(process.env.CHROME_BINARY);
    if (mode.mobile) {
        options.setMobileEmulation({
            deviceMetrics: { width: mode.width, height: mode.height, pixelRatio: 3, touch: true },
        });
    }
    const driver = await new Builder().forBrowser('chrome').setChromeOptions(options).build();
    if (!mode.mobile) await driver.manage().window().setRect({ width: mode.width, height: mode.height });
    return driver;
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
    this.timeout(180000);
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

        it('shows the argument syntax for each documented word', async function () {
            await useStubNames(driver, ['forward', 'foreach', 'nohelpword']);
            await typeSlowly(driver, 'f');
            const state = await popupState(driver);
            expect(state.usages.forward).to.deep.equal(['FORWARD dist', 'FD dist']);
            expect(state.usages.foreach).to.deep.equal(
                ['FOREACH data template', '(FOREACH data1 data2 ... template)']);
            await resetInput(driver);
            await typeSlowly(driver, 'n');
            expect((await popupState(driver)).usages.nohelpword).to.deep.equal([]);
        });

        it('closes after the command is submitted', async function () {
            await typeSlowly(driver, 'fo');
            await driver.findElement(By.id('logoInputText')).sendKeys(Key.ENTER);
            await waitForLogoIdle(driver);
            expect((await popupState(driver)).open).to.equal(false);
        });
    });

    describe('layout', function () {
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
