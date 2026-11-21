'use strict';

// Co-authored by GitHub Copilot. Requested model: MAI Code 2 Flash; active model and reasoning-effort metadata: unverified.
const fs = require('node:fs');
const path = require('node:path');
const crypto = require('node:crypto');
const { execFileSync } = require('node:child_process');
const MarkdownIt = require('markdown-it');
const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl,
    ready,
    idle,
    diagnostics,
    output,
    isolateSettings,
    restoreSettings,
} = require('./testcafe-helpers');

const repositoryRoot = path.resolve(__dirname, '../..');
const taskDirectory = '/tmp/copilot/TGS-2026-11-20-T1';
const extractedDirectory = path.join(taskDirectory, 'examples');
const reportDirectory = path.join(repositoryRoot, 'reports/turtle-graphics-examples');
const screenshotsDirectory = path.join(reportDirectory, 'screenshots');
const reportJsonPath = path.join(reportDirectory, 'report.json');
const reportMarkdownPath = path.join(reportDirectory, 'report.md');
const md = new MarkdownIt({ html: false, linkify: false, typographer: false });

const documentPaths = [
    '.github/skills/ucb-logo-turtle-graphics/SKILL.md',
    '.github/skills/ucb-logo-turtle-graphics/references/turtle-primitives.md',
    '.github/skills/ucb-logo-turtle-graphics/references/turtle-patterns.md',
    '.github/skills/ucb-logo-turtle-graphics/references/platform-notes.md',
].map((relativePath) => ({
    relativePath,
    absolutePath: path.join(repositoryRoot, relativePath),
}));

function sha256(value) {
    return crypto.createHash('sha256').update(value).digest('hex');
}

function safeSlug(relativePath) {
    return relativePath
        .replace(/\.md$/i, '')
        .replace(/[^a-z0-9.-]+/gi, '-')
        .replace(/^-+|-+$/g, '')
        .toLowerCase();
}

function gitOutput(args) {
    try {
        return execFileSync('git', args, { cwd: repositoryRoot, encoding: 'utf8' }).trim();
    } catch (error) {
        return null;
    }
}

function fileIdentity(relativePath) {
    const absolutePath = path.join(repositoryRoot, relativePath);
    try {
        const stat = fs.statSync(absolutePath);
        return {
            path: relativePath,
            exists: true,
            size: stat.size,
            sha256: sha256(fs.readFileSync(absolutePath)),
            mtime: stat.mtime.toISOString(),
        };
    } catch (error) {
        return { path: relativePath, exists: false, size: null, sha256: null, mtime: null };
    }
}

function artifactMetadata() {
    return {
        head: gitOutput(['rev-parse', 'HEAD']),
        describe: gitOutput(['describe', '--tags', '--always', '--dirty']),
        dirty: gitOutput(['status', '--porcelain=v1']) || '',
        html: fileIdentity('ucblogo.html'),
        javascript: fileIdentity('ucblogo.js'),
        wasm: fileIdentity('ucblogo.wasm'),
        gitDescriptionInBuild: (() => {
            try {
                const source = fs.readFileSync(path.join(repositoryRoot, 'git.c'), 'utf8');
                const match = source.match(/GIT\s*=\s*"([^"]*)"/);
                return match ? match[1] : null;
            } catch (error) {
                return null;
            }
        })(),
        fresh: false,
        freshness: 'not-rebuilt; existing generated Web artifacts are tested as they are, and no application source changed in this task',
    };
}

function browserPolicy() {
    const envBrowser = process.env.TESTCAFE_BROWSER;
    const envHeadless = process.env.TESTCAFE_HEADLESS;
    const browserValue = envBrowser === undefined ? 'chrome' : envBrowser;
    const headlessValue = envHeadless === undefined ? '1' : envHeadless;
    const allowed = browserValue === 'chrome' && headlessValue === '1' &&
        !/(?:emulation|mobile|touch|remote)/i.test(browserValue);
    return {
        allowed,
        browserValue,
        headlessValue,
        capabilityValue: process.env.UCBOLOGO_RUNNER_CHILD_CAPABILITIES || null,
        enforcedDesktop: 'chrome:headless',
    };
}

function meaningfulLogoSource(source) {
    const lines = source.replace(/\r\n/g, '\n').split('\n');
    const meaningful = [];
    for (const line of lines) {
        const trim = line.replace(/\s*;.*$/, '').trim();
        if (trim) meaningful.push(trim);
    }
    return meaningful;
}

function structuralLogoErrors(source) {
    const errors = [];
    const meaningful = meaningfulLogoSource(source);
    if (!meaningful.length) {
        errors.push('empty or comment-only source');
        return errors;
    }

    let procedureOpens = 0;
    let procedureCloses = 0;
    for (const line of meaningful) {
        const lower = line.toLowerCase();
        if (/^to(?:\s|$)/.test(lower)) procedureOpens += 1;
        if (/^end(?:\s|$)/.test(lower)) procedureCloses += 1;
    }
    if (procedureOpens !== procedureCloses)
        errors.push(`procedure definition imbalance: ${procedureOpens} TO, ${procedureCloses} END`);

    for (const pair of [['[', ']'], ['(', ')']]) {
        let depth = 0;
        for (const line of source.replace(/\r\n/g, '\n').split('\n')) {
            const code = line.replace(/;.*$/, '');
            for (const character of code) {
                if (character === pair[0]) depth += 1;
                else if (character === pair[1]) depth -= 1;
                if (depth < 0) {
                    errors.push(`unmatched closing ${pair[1]}`);
                    return errors;
                }
            }
        }
        if (depth !== 0) errors.push(`unmatched ${pair[0]}/${pair[1]} delimiters`);
    }
    return errors;
}

fs.mkdirSync(reportDirectory, { recursive: true });
fs.mkdirSync(screenshotsDirectory, { recursive: true });
fs.mkdirSync(extractedDirectory, { recursive: true });

const packageRelativeDirectory = '.github/skills/ucb-logo-turtle-graphics';
const packageMissing = !fs.existsSync(path.join(repositoryRoot, packageRelativeDirectory));
const manifests = [];
let parsedFenceCount = 0;
let discoveryProblem = null;

if (packageMissing) {
    discoveryProblem = `Missing skill package: ${packageRelativeDirectory}`;
} else {
    for (const document of documentPaths) {
        let text;
        try {
            text = fs.readFileSync(document.absolutePath, 'utf8');
        } catch (error) {
            continue;
        }
        let tokens;
        try {
            tokens = md.parse(text, {});
        } catch (error) {
            discoveryProblem = `Markdown parser failed in ${document.relativePath}: ${error.message}`;
            continue;
        }

        let fenceOrdinal = 0;
        let logoFenceOrdinal = 0;
        for (const token of tokens) {
            if (token.type !== 'fence') continue;
            fenceOrdinal += 1;
            parsedFenceCount += 1;
            if (token.info !== 'logo') continue;
            logoFenceOrdinal += 1;
            const slug = safeSlug(document.relativePath);
            const sourceName = `${slug}-fence-${logoFenceOrdinal}.lg`;
            const sourcePath = path.join(extractedDirectory, sourceName);
            fs.writeFileSync(sourcePath, token.content, { encoding: 'utf8', flag: 'w' });
            const screenshotCustomPath = `${slug}/fence-${logoFenceOrdinal}.png`;
            const screenshotRelativePath = `reports/turtle-graphics-examples/screenshots/${screenshotCustomPath}`;
            const screenshotAbsolutePath = path.join(screenshotsDirectory, screenshotCustomPath);
            manifests.push({
                id: `${document.relativePath}#logo-fence-${logoFenceOrdinal}`,
                testName: `Turtle graphics example: ${document.relativePath} fence ${logoFenceOrdinal}`,
                documentPath: document.relativePath,
                allFenceOrdinal: fenceOrdinal,
                logoFenceOrdinal,
                sourceText: token.content,
                sourcePath,
                sourceName,
                sourceSha256: sha256(Buffer.from(token.content, 'utf8')),
                sourceMeaningful: meaningfulLogoSource(token.content).length > 0,
                structuralErrors: structuralLogoErrors(token.content),
                screenshotCustomPath,
                screenshotRelativePath,
                screenshotAbsolutePath,
            });
        }
    }

    if (!manifests.length && !discoveryProblem)
        discoveryProblem = 'No runnable `logo` fences exist in the four turtle graphics skill documents.';
}

const initialEntries = manifests.map((manifest) => ({
    id: manifest.id,
    testName: manifest.testName,
    documentPath: manifest.documentPath,
    fenceOrdinal: manifest.logoFenceOrdinal,
    allFenceOrdinal: manifest.allFenceOrdinal,
    sourcePath: manifest.sourcePath,
    sourceRelativePath: path.relative(repositoryRoot, manifest.sourcePath).replace(/\\/g, '/'),
    sourceSha256: manifest.sourceSha256,
    result: 'not-run',
    passed: false,
    browser: null,
    execution: null,
    diagnostics: null,
    screenshot: {
        relativePath: manifest.screenshotRelativePath,
        absolutePath: manifest.screenshotAbsolutePath,
        exists: false,
        size: null,
    },
}));

const reportState = {
    entries: initialEntries,
    browser: null,
    artifact: artifactMetadata(),
    runner: {
        fixture: 'Turtle graphics documentation examples',
        sharedRunner: 'tests/acceptance/run-testcafe.js',
        browserEnvironment: browserPolicy(),
        reportJson: 'reports/turtle-graphics-examples/report.json',
        reportMarkdown: 'reports/turtle-graphics-examples/report.md',
        screenshotDirectory: 'reports/turtle-graphics-examples/screenshots',
        noNodeLogoExecution: true,
        noRawAppFlag: true,
    },
};

function reconcileCounts() {
    const screenshotCount = reportState.entries.filter((entry) =>
        entry.screenshot?.exists && entry.screenshot?.size > 0).length;
    const passed = reportState.entries.filter((entry) => entry.result === 'pass').length;
    const failed = reportState.entries.filter((entry) => entry.result === 'fail').length;
    const notRun = reportState.entries.filter((entry) => entry.result === 'not-run').length;
    return {
        fences: manifests.length,
        parsedFences: parsedFenceCount,
        entries: reportState.entries.length,
        screenshots: screenshotCount,
        passed,
        failed,
        notRun,
    };
}

function writeReports() {
    fs.mkdirSync(reportDirectory, { recursive: true });
    fs.mkdirSync(screenshotsDirectory, { recursive: true });
    const counts = reconcileCounts();
    const report = {
        schemaVersion: 1,
        taskId: 'TGS-2026-11-20-T1',
        generatedAt: new Date().toISOString(),
        discoveryProblem,
        zeroExample: manifests.length === 0,
        artifact: reportState.artifact,
        runner: reportState.runner,
        browser: reportState.browser,
        counts,
        entries: reportState.entries,
    };
    fs.writeFileSync(reportJsonPath, `${JSON.stringify(report, null, 2)}\n`, 'utf8');

    const mdLines = [
        '# Turtle Graphics Documentation Example Report',
        '',
        `- Task: \`TGS-2026-11-20-T1\``,
        `- Generated: \`${report.generatedAt}\``,
        `- Git HEAD: \`${report.artifact.head || 'unknown'}\``,
        `- Artifact freshness: \`${report.artifact.freshness}\``,
        `- Fixture: \`${report.runner.fixture}\``,
        `- Discovery: ${discoveryProblem ? `\`${discoveryProblem}\`` : 'runnable `logo` fences found'}`,
        `- Counts: ${counts.entries} entries, ${counts.fences} fences, ${counts.screenshots} screenshots, ${counts.passed} passed, ${counts.failed} failed, ${counts.notRun} not run`,
        '',
        '## Entries',
        '',
    ];
    if (!reportState.entries.length) {
        mdLines.push('- No parsed runnable `logo` fence entries.', '');
    } else {
        for (const entry of reportState.entries) {
            mdLines.push(`### ${entry.id}`);
            mdLines.push('', `- Result: **${entry.result}**`);
            mdLines.push(`- Source: \`${entry.sourceRelativePath}\``);
            mdLines.push(`- Source SHA-256: \`${entry.sourceSha256}\``);
            const screenshotLink = entry.screenshot.relativePath.startsWith('reports/turtle-graphics-examples/screenshots/')
                ? `screenshots/${entry.screenshot.relativePath.slice('reports/turtle-graphics-examples/screenshots/'.length)}`
                : entry.screenshot.relativePath;
            mdLines.push(`- Screenshot: [${entry.screenshot.relativePath}](${screenshotLink})`);
            if (entry.browser) mdLines.push(`- Browser: \`${entry.browser.prettyUserAgent || entry.browser.alias}\``);
            if (entry.execution) mdLines.push(`- Execution: \`${JSON.stringify(entry.execution)}\``);
            if (entry.diagnostics) mdLines.push(`- Diagnostics: \`${JSON.stringify(entry.diagnostics).replace(/\n/g, ' ')}\``);
            mdLines.push('');
        }
    }
    mdLines.push('<!-- Co-authored by GitHub Copilot. Scope: generated turtle documentation example report. -->', '');
    fs.writeFileSync(reportMarkdownPath, mdLines.join('\n'), 'utf8');
}

writeReports();

const uploadedFileExists = ClientFunction((fileName) => {
    try {
        if (!window.Module?.FS?.analyzePath) return false;
        return Boolean(Module.FS.analyzePath(fileName).exists);
    } catch (error) {
        return false;
    }
});

const drawingHasArtwork = ClientFunction(() => {
    const group = document.getElementById('logoDrawingElements');
    if (!group) return false;
    for (const element of group.children) {
        if (!element.classList.contains('sizer')) return true;
    }
    return false;
});

const uploadEchoPresent = ClientFunction((baselineLength) => {
    const element = document.getElementById('output');
    return Boolean(element && element.textContent.length > baselineLength);
});

function browserIdentity(t) {
    return {
        alias: t.browser.alias,
        name: t.browser.name,
        version: t.browser.version,
        headless: t.browser.headless,
        platform: t.browser.platform,
        os: t.browser.os,
        engine: t.browser.engine,
        userAgent: t.browser.userAgent,
        prettyUserAgent: t.browser.prettyUserAgent,
        nativeAutomation: t.browser.nativeAutomation,
    };
}

function updateTopLevelBrowser(t) {
    reportState.browser = browserIdentity(t);
    writeReports();
}

function runtimeFailureMarkers(state) {
    const text = [state.error || '', state.status || '', state.output || ''].join('\n');
    const patterns = [
        /I don't know how to/i,
        /I don't understand/i,
        /doesn't like/i,
        /didn't output/i,
        /has no value/i,
        /not enough inside/i,
        /too much inside/i,
        /isn['’]t a variable/i,
        /isn['’]t an instruction/i,
        /Couldn't open/i,
        /Couldn't load/i,
        /File not found/i,
        /RuntimeError/i,
        /Aborted\b/i,
        /Segmentation fault/i,
        /memory access out of bounds/i,
        /Assertion failed/i,
        /Uncaught JavaScript error/i,
        /Unhandled promise rejection/i,
        /Exception thrown/i,
        /\bError\s+\d+:/i,
    ];
    return patterns.filter((pattern) => pattern.test(text)).map((pattern) => pattern.toString());
}

function updateEntry(manifest, t, result) {
    const entry = reportState.entries.find((candidate) => candidate.id === manifest.id);
    if (!entry) return;
    entry.testName = manifest.testName;
    entry.result = result.passed ? 'pass' : 'fail';
    entry.passed = result.passed;
    entry.browser = browserIdentity(t);
    entry.execution = result.execution;
    entry.diagnostics = result.diagnostics;
    entry.screenshot = result.screenshot;
    writeReports();
}

function reportConsistencyErrors() {
    const report = JSON.parse(fs.readFileSync(reportJsonPath, 'utf8'));
    const errors = [];
    const counts = report.counts;

    if (report.entries.length !== manifests.length)
        errors.push(`Expected ${manifests.length} report entries, found ${report.entries.length}`);
    if (counts.fences !== manifests.length || counts.entries !== manifests.length)
        errors.push('Report fence and entry counts do not match discovered examples');
    if (counts.screenshots !== manifests.length)
        errors.push('Report screenshot count does not match discovered examples');
    if (counts.passed + counts.failed !== manifests.length || counts.notRun !== 0)
        errors.push('Report results do not account for every discovered example');
    if (!report.artifact?.head || !report.artifact?.html?.sha256 ||
        !report.artifact?.javascript?.sha256 || !report.artifact?.wasm?.sha256)
        errors.push('Report is missing the Web artifact identity');
    if (!report.browser?.name || !report.browser?.version || !report.browser?.headless)
        errors.push('Report is missing headless browser identity');

    for (const manifest of manifests) {
        const entry = report.entries.find((candidate) => candidate.id === manifest.id);
        if (!entry) continue;
        if (!['pass', 'fail'].includes(entry.result))
            errors.push(`${manifest.id} has no terminal result`);
        if (!entry.browser?.name || !entry.browser?.version)
            errors.push(`${manifest.id} is missing browser version details`);
        if (!entry.execution || !entry.diagnostics)
            errors.push(`${manifest.id} is missing execution diagnostics`);
        try {
            const screenshot = fs.statSync(entry.screenshot.absolutePath);
            if (!entry.screenshot.exists || screenshot.size <= 0)
                errors.push(`${manifest.id} is missing a nonempty screenshot`);
        } catch (error) {
            errors.push(`${manifest.id} screenshot is unavailable`);
        }
    }
    return errors;
}

const turtleFixture = fixture('Turtle graphics documentation examples')
    .page(applicationUrl)
    .disablePageCaching;

turtleFixture.before(async (ctx) => {
    ctx.entries = reportState.entries;
});

turtleFixture.after(async (ctx) => {
    for (const manifest of manifests) {
        try {
            fs.rmSync(manifest.sourcePath, { force: true });
        } catch (error) {}
    }
    writeReports();
});

turtleFixture.afterEach(async (t) => {
    if (t.ctx.settingsCaptured)
        await restoreSettings(t);
});

if (!manifests.length) {
    test('No runnable turtle graphics documentation examples', async (t) => {
        updateTopLevelBrowser(t);
        const policy = browserPolicy();
        await t.expect(policy.allowed).ok(
            `This fixture requires desktop headless Chrome; received ${policy.browserValue}, headless=${policy.headlessValue}.`,
        );
        await t.expect(manifests.length).gt(0,
            discoveryProblem || 'No runnable `logo` fences exist in the turtle graphics skill documents.',
        );
    });
} else {
    for (const manifest of manifests) {
        test(manifest.testName, async (t) => {
            const problems = [];
            const execution = {
                exactSourceCheck: false,
                uploadEchoObserved: false,
                idleReached: false,
                drawingObserved: false,
                sourceMeaningful: manifest.sourceMeaningful,
                structuralErrors: manifest.structuralErrors,
            };
            let diagnosticsState = null;
            let browserConsole = null;
            let screenshot = {
                relativePath: manifest.screenshotRelativePath,
                absolutePath: manifest.screenshotAbsolutePath,
                exists: false,
                size: null,
            };

            updateTopLevelBrowser(t);
            const policy = browserPolicy();
            if (!policy.allowed)
                problems.push(`Browser policy rejected ${policy.browserValue}, headless=${policy.headlessValue}`);
            if (!/^chrome/i.test(t.browser.name) && !/^chromium/i.test(t.browser.name))
                problems.push(`Expected Chrome/Chromium, received ${t.browser.name}`);
            if (!t.browser.headless)
                problems.push('Expected headless Chrome');
            if (/emulation|mobile|touch|remote/i.test(t.browser.alias))
                problems.push(`Expected desktop Chrome, alias was ${t.browser.alias}`);

            try {
                await t.navigateTo(applicationUrl);
                await ready(t);
                await isolateSettings(t);

                const actualBytes = fs.readFileSync(manifest.sourcePath);
                const expectedBytes = Buffer.from(manifest.sourceText, 'utf8');
                execution.exactSourceCheck = actualBytes.equals(expectedBytes);
                if (!execution.exactSourceCheck)
                    problems.push('Extracted .lg bytes differ from the Markdown fence token content');

                const before = await diagnostics();
                const baselineLength = before.output?.length || 0;

                if (!manifest.sourceMeaningful)
                    problems.push('Fence is empty or comment-only');
                for (const structuralError of manifest.structuralErrors)
                    problems.push(structuralError);

                await t.setFilesToUpload(Selector('#uploadFileInput'), manifest.sourcePath);
                await t.expect(uploadEchoPresent(baselineLength)).ok(
                    `Upload/load action did not produce observable output for ${manifest.sourceName}`,
                    { timeout: 30000 },
                );
                execution.uploadEchoObserved = true;

                await t.expect(idle()).ok(
                    `Logo did not return to idle after loading ${manifest.sourceName}`,
                    { timeout: 60000 },
                );
                execution.idleReached = true;
                execution.drawingObserved = await drawingHasArtwork();
                if (!execution.drawingObserved)
                    problems.push('Example completed without producing turtle drawing artwork');

                diagnosticsState = await diagnostics();
                try {
                    browserConsole = await t.getBrowserConsoleMessages();
                } catch (error) {
                    browserConsole = { error: String(error) };
                }

                if (typeof diagnosticsState?.error === 'string' && diagnosticsState.error.trim())
                    problems.push(`Visible javascriptError content: ${diagnosticsState.error.trim()}`);

                if (!await uploadedFileExists(manifest.sourceName))
                    problems.push(`Uploaded file was not present in the Emscripten filesystem: ${manifest.sourceName}`);

                const failureMarkers = runtimeFailureMarkers(diagnosticsState);
                if (failureMarkers.length)
                    problems.push(`Logo/runtime failure markers: ${failureMarkers.join(', ')}`);

                const consoleText = JSON.stringify(browserConsole);
                if (/RuntimeError|Segmentation fault|memory access out of bounds|Assertion failed/i.test(consoleText))
                    problems.push('Browser console contains a runtime crash diagnostic');
            } catch (error) {
                problems.push(`Execution failed: ${error.message || String(error)}`);
                try {
                    diagnosticsState = await diagnostics();
                } catch (diagnosticsError) {
                    diagnosticsState = { error: String(diagnosticsError) };
                }
            }

            try {
                await t.takeScreenshot({
                    path: manifest.screenshotCustomPath,
                    fullPage: false,
                });
            } catch (error) {
                problems.push(`Screenshot capture failed: ${error.message || String(error)}`);
            }

            try {
                const stat = fs.statSync(manifest.screenshotAbsolutePath);
                if (!stat.isFile() || stat.size <= 0)
                    problems.push('Missing, unreadable, or zero-byte screenshot');
                else {
                    screenshot.exists = true;
                    screenshot.size = stat.size;
                }
            } catch (error) {
                problems.push('Missing, unreadable, or zero-byte screenshot');
            }

            const passed = problems.length === 0;
            updateEntry(manifest, t, {
                passed,
                execution,
                diagnostics: {
                    state: diagnosticsState,
                    browserConsole,
                    problems,
                    failureMarkers: diagnosticsState ? runtimeFailureMarkers(diagnosticsState) : [],
                },
                screenshot,
            });

            await t.expect(passed).ok(
                `${manifest.id}\n${problems.length ? problems.join('\n') : 'Example passed.'}`,
            );
        });
    }

    test('Turtle graphics example report reconciles fences, results, and screenshots', async (t) => {
        const errors = reportConsistencyErrors();
        await t.expect(errors).eql([], errors.join('\n') || 'Report must account for every runnable Logo fence.');
    });
}

// Co-authored by GitHub Copilot. Requested model: MAI Code 2 Flash; active model and reasoning-effort metadata: unverified.
