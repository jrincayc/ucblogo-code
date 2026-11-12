const fs = require('node:fs/promises');
const path = require('node:path');
const { expect } = require('chai');
const chromeLauncher = require('chrome-launcher');
const lighthouseModule = require('lighthouse');
const lighthouse = lighthouseModule.default;
const { createApplicationServer } = require('./application-server');

const applicationServer = createApplicationServer();
const reportRoot = path.resolve(
    process.env.LIGHTHOUSE_REPORT_DIR || path.join(__dirname, '../../reports/lighthouse'),
);
const profiles = [
    { name: 'mobile', minimumPerformance: 0.567 },
    { name: 'desktop', config: lighthouseModule.desktopConfig, minimumPerformance: 0.882 },
];
let applicationUrl;

describe('Lighthouse acceptance', function () {
    this.timeout(180000);

    before(async function () {
        applicationUrl = await applicationServer.start();
    });

    after(async function () {
        await applicationServer.stop();
    });

    for (const profile of profiles) {
        it(`${profile.name} meets baseline scores and writes HTML and JSON reports`, async function () {
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
                }, profile.config);
            } finally {
                await chrome.kill();
            }

            const { lhr, report } = results;
            const scores = Object.fromEntries(
                Object.entries(lhr.categories).map(([name, category]) => [name, category.score]),
            );
            const reportDirectory = path.join(reportRoot, profile.name);

            await fs.mkdir(reportDirectory, { recursive: true });
            await Promise.all([
                fs.writeFile(path.join(reportDirectory, 'report.html'), report[0]),
                fs.writeFile(path.join(reportDirectory, 'report.json'), report[1]),
            ]);

            console.log(`Lighthouse ${profile.name} scores: ${JSON.stringify(scores)}`);
            console.log(`Lighthouse ${profile.name} LCP: ${lhr.audits['largest-contentful-paint'].displayValue}`);
            console.log(`Lighthouse ${profile.name} reports: ${reportDirectory}`);
            expect(lhr.audits['cumulative-layout-shift'].numericValue,
                'cumulative layout shift').to.be.below(0.1);
            expect(scores.performance, 'performance score')
                .to.be.at.least(profile.minimumPerformance);
            expect(scores.accessibility, 'accessibility score').to.be.at.least(0.8);
            expect(scores['best-practices'], 'best practices score').to.be.at.least(0.8);
            expect(scores.seo, 'SEO score').to.be.at.least(0.8);
        });
    }
});