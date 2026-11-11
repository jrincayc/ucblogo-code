const fs = require('node:fs/promises');
const path = require('node:path');
const { expect } = require('chai');
const chromeLauncher = require('chrome-launcher');
const lighthouse = require('lighthouse').default;
const { createApplicationServer } = require('./application-server');

const applicationServer = createApplicationServer();
const reportDirectory = path.resolve(
    process.env.LIGHTHOUSE_REPORT_DIR || path.join(__dirname, '../../reports/lighthouse'),
);
let applicationUrl;

describe('Lighthouse acceptance', function () {
    this.timeout(180000);

    before(async function () {
        applicationUrl = await applicationServer.start();
    });

    after(async function () {
        await applicationServer.stop();
    });

    it('meets baseline quality scores and writes HTML and JSON reports', async function () {
        const chrome = await chromeLauncher.launch({
            chromeFlags: ['--headless=new', '--no-sandbox', '--disable-dev-shm-usage'],
            chromePath: process.env.CHROME_BINARY,
        });
        let results;

        try {
            results = await lighthouse(applicationUrl, {
                logLevel: 'error',
                output: ['html', 'json'],
                port: chrome.port,
            });
        } finally {
            await chrome.kill();
        }

        const { lhr, report } = results;
        const scores = Object.fromEntries(
            Object.entries(lhr.categories).map(([name, category]) => [name, category.score]),
        );

        await fs.mkdir(reportDirectory, { recursive: true });
        await Promise.all([
            fs.writeFile(path.join(reportDirectory, 'report.html'), report[0]),
            fs.writeFile(path.join(reportDirectory, 'report.json'), report[1]),
        ]);

        console.log(`Lighthouse scores: ${JSON.stringify(scores)}`);
        console.log(`Lighthouse reports: ${reportDirectory}`);
        expect(scores.performance, 'performance score').to.be.at.least(0.5);
        expect(scores.accessibility, 'accessibility score').to.be.at.least(0.8);
        expect(scores['best-practices'], 'best practices score').to.be.at.least(0.8);
        expect(scores.seo, 'SEO score').to.be.at.least(0.8);
    });
});