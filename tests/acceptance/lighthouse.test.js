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
            const maximumAttempts = 3;
            const reportDirectory = path.join(reportRoot, profile.name);
            let results;

            for (let attempt = 1; attempt <= maximumAttempts; attempt++) {
                const chrome = await chromeLauncher.launch({
                    chromeFlags: ['--headless=new', '--no-sandbox', '--disable-dev-shm-usage'],
                    chromePath: process.env.CHROME_BINARY,
                });

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
                const attemptDirectory = lhr.runtimeError && attempt < maximumAttempts
                    ? path.join(reportDirectory, `attempt-${attempt}`) : reportDirectory;

                await fs.mkdir(attemptDirectory, { recursive: true });
                await Promise.all([
                    fs.writeFile(path.join(attemptDirectory, 'report.html'), report[0]),
                    fs.writeFile(path.join(attemptDirectory, 'report.json'), report[1]),
                ]);

                if (!lhr.runtimeError) break;
                console.warn(`Lighthouse ${profile.name} attempt ${attempt}/${maximumAttempts}: ` +
                    `${lhr.runtimeError.code}: ${lhr.runtimeError.message}`);
            }

            const { lhr } = results;
            console.log(`Lighthouse ${profile.name} reports: ${reportDirectory}`);
            expect(lhr.runtimeError, `Lighthouse ${profile.name} runtime error after ${maximumAttempts} attempts: ` +
                `${lhr.runtimeError?.code}: ${lhr.runtimeError?.message}`).to.be.undefined;
            const scores = Object.fromEntries(
                Object.entries(lhr.categories).map(([name, category]) => [name, category.score]),
            );

            console.log(`Lighthouse ${profile.name} scores: ${JSON.stringify(scores)}`);
            console.log(`Lighthouse ${profile.name} LCP: ${lhr.audits['largest-contentful-paint'].displayValue}`);
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