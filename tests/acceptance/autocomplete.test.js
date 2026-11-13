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
});
