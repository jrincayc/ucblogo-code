const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl, ready, isolateSettings, restoreSettings, runLogo,
    commandInput, idle: logoIdle,
} = require('./testcafe-helpers');

const capabilities = new Set(process.env.TESTCAFE_CAPABILITIES !== undefined
    ? process.env.TESTCAFE_CAPABILITIES.split(',').map((name) => name.trim()).filter(Boolean)
    : JSON.parse(process.env.UCBOLOGO_RUNNER_CHILD_CAPABILITIES || '[]'));
const screenshotRun = Date.now();
const viewerDelay = Number(process.env.TEST_DELAY_MS) || 0;
const interactionDelay = Number(process.env.TEST_INTERACTION_DELAY_MS) || 100;
// Measured worst-case in-page latency (ms) from input event to rendered popup, plus 10%.
const POPUP_LATENCY_BUDGET_MS = Number(process.env.POPUP_LATENCY_BUDGET_MS) || 0;

const autocomplete = Selector('logo-autocomplete');
const listbox = autocomplete.shadowRoot().find('[role="listbox"]');
const clickSuggestionInPage = ClientFunction((name) => {
    const root = document.querySelector('logo-autocomplete').shadowRoot;
    let suggestion;
    for (const node of root.querySelectorAll('[role="option"]')) {
        if (node.querySelector('.name').textContent === name) {
            suggestion = node;
            break;
        }
    }
    if (!suggestion) throw new Error(`Suggestion not found: ${name}`);
    suggestion.click();
});
const clickEditorSuggestionInPage = ClientFunction((name) => {
    const root = document.querySelector('logo-autocomplete[for="editorTextArea"]').shadowRoot;
    let suggestion;
    for (const node of root.querySelectorAll('[role="option"]')) {
        if (node.querySelector('.name').textContent === name) {
            suggestion = node;
            break;
        }
    }
    if (!suggestion) throw new Error(`Editor suggestion not found: ${name}`);
    suggestion.click();
});

const blockedOnInput = ClientFunction(() => Boolean(Module.blockedOnGetc));
const viewportWidth = ClientFunction(() => document.documentElement.clientWidth);
const storedLegacyFontSize = ClientFunction(() => localStorage.getItem('ucblogo.fontSize'));
const storeLegacyFontSize = ClientFunction((value) => {
    if (value === null) localStorage.removeItem('ucblogo.fontSize');
    else localStorage.setItem('ucblogo.fontSize', value);
});

const popupState = ClientFunction(() => {
    const input = document.getElementById('logoInputText');
    const root = document.querySelector('logo-autocomplete').shadowRoot;
    const box = root.querySelector('[role="listbox"]');
    const names = [];
    const summaries = {};
    const usages = {};
    const optionBackgrounds = {};
    for (const node of box.querySelectorAll('[role="option"]')) {
        const word = node.querySelector('.name').textContent;
        names.push(word);
        optionBackgrounds[word] = getComputedStyle(node).backgroundColor;
        summaries[word] = node.querySelector('.summary') ? node.querySelector('.summary').textContent : null;
        usages[word] = Array.from(node.querySelectorAll('.signature')).map((signature) => signature.textContent);
    }
    return {
        open: !box.hidden,
        expanded: input.getAttribute('aria-expanded'),
        activeDescendant: input.getAttribute('aria-activedescendant'),
        role: input.getAttribute('role'),
        autocomplete: input.getAttribute('aria-autocomplete'),
        options: names,
        optionBackgrounds,
        selected: Array.from(box.querySelectorAll('[role="option"]'))
            .filter((node) => node.getAttribute('aria-selected') === 'true')
            .map((node) => node.querySelector('.name').textContent),
        summaries,
        usages,
        status: root.querySelector('[role="status"]').textContent,
        value: input.value,
        caret: input.selectionStart,
        focused: document.activeElement === input,
    };
});

const useStubNames = ClientFunction((names) => {
    const element = document.querySelector('logo-autocomplete');
    element.liveProvider ??= element.provider;
    element.provider = (text) => names.filter((n) => n.startsWith(text.toLowerCase()))
        .map((n) => ({ name: n, kind: 'primitive' }));
});
const useLiveNames = ClientFunction(() => {
    const element = document.querySelector('logo-autocomplete');
    element.provider = element.liveProvider ?? element.provider;
});
const openSettingsDialog = ClientFunction(() => document.getElementById('settings').showModal());
const settingsDialogOpen = ClientFunction(() => document.getElementById('settings').open);
const editorDialogOpen = ClientFunction(() => document.getElementById('editorDialog').open);
const discardEditor = ClientFunction(() => {
    if (document.getElementById('editorDialog').open) Module.editor.discardClicked();
});
const prepareEditorCompletion = ClientFunction(() => {
    const input = document.getElementById('editorTextArea');
    input.value = 'to demo\n  forw 10\nend';
    input.focus();
    input.setSelectionRange(14, 14);
    input.dispatchEvent(new Event('input', { bubbles: true }));
});
const prepareEmptyEditor = ClientFunction(() => {
    const input = document.getElementById('editorTextArea');
    input.value = '';
    input.focus();
});
const editorState = ClientFunction(() => {
    const input = document.getElementById('editorTextArea');
    return {
        dialogOpen: document.getElementById('editorDialog').open,
        value: input.value,
        caret: input.selectionStart,
        focused: document.activeElement === input,
    };
});
const resetInput = ClientFunction(() => {
    const input = document.getElementById('logoInputText');
    input.value = '';
    input.dispatchEvent(new Event('input'));
    input.focus();
});

const measurePopupLatency = ClientFunction((text) => {
    const input = document.getElementById('logoInputText');
    const box = document.querySelector('logo-autocomplete').shadowRoot.querySelector('[role="listbox"]');
    input.focus();
    input.value = text;
    input.setSelectionRange(text.length, text.length);
    const start = performance.now();
    input.dispatchEvent(new Event('input'));
    const elapsed = performance.now() - start;
    return { elapsed, open: !box.hidden, count: box.children.length };
});

const optionLayout = ClientFunction((wanted) => {
    const root = document.querySelector('logo-autocomplete').shadowRoot;
    const box = root.querySelector('[role="listbox"]');
    let node;
    for (const candidate of box.querySelectorAll('[role="option"]')) {
        if (candidate.querySelector('.name').textContent === wanted) {
            node = candidate;
            break;
        }
    }
    if (!node) throw new Error(`Suggestion not found: ${wanted}`);
    const rect = (element) => element.getBoundingClientRect().toJSON();
    const heading = node.querySelector('.heading');
    const usage = node.querySelector('.usage');
    const signatures = usage.querySelectorAll('.signature');
    const measure = document.createElement('canvas').getContext('2d');
    const usageGap = Number.parseFloat(getComputedStyle(usage).columnGap);
    let usageWidth = Math.max(0, signatures.length - 1) * usageGap;
    for (const signature of signatures) {
        measure.font = getComputedStyle(signature).font;
        usageWidth += measure.measureText(signature.textContent).width;
    }
    const inlineViewportWidth = document.documentElement.clientWidth - heading.clientWidth +
        rect(node.querySelector('.name')).width +
        Number.parseFloat(getComputedStyle(heading).columnGap) + usageWidth;
    return {
        viewportWidth: document.documentElement.clientWidth,
        inlineViewportWidth,
        viewportHeight: window.visualViewport?.height ?? window.innerHeight,
        scrollY: window.scrollY,
        input: rect(document.getElementById('logoInputText')),
        listbox: rect(box),
        listboxOverflow: box.scrollWidth - box.clientWidth,
        option: rect(node),
        optionOverflow: node.scrollWidth - node.clientWidth,
        name: rect(node.querySelector('.name')),
        usage: rect(node.querySelector('.usage')),
        summary: rect(node.querySelector('.summary')),
    };
});

async function pause(t) {
    await t.wait(interactionDelay);
}

async function clickSuggestion(t, name) {
    await clickSuggestionInPage(name);
    await pause(t);
}

async function typeSlowly(t, text) {
    await t.typeText(commandInput, text, { caretPos: (await commandInput.value).length, speed: viewerDelay ? 0.3 : 1 });
    await pause(t);
}

async function screenshot(t, name) {
    if (!capabilities.has('screenshots')) return;
    const width = await viewportWidth();
    await t.takeScreenshot({
        path: `autocomplete/${screenshotRun}-${t.browser.alias.replace(/[^a-z0-9-]/gi, '_')}-${width}px-${name}.png`,
        fullPage: false,
    });
}

fixture('Logo word autocompletion')
    .meta({ input: 'TestCafe key simulation and DOM suggestion clicks; not native mobile keyboard or touch' })
    .page(applicationUrl)
    .beforeEach(async (t) => {
        if (capabilities.has('resize')) await t.resizeWindow(1280, 800);
        await ready(t);
        t.ctx.originalLegacyFontSize = await storedLegacyFontSize();
        t.ctx.legacyFontSizeCaptured = true;
        await storeLegacyFontSize(null);
        await isolateSettings(t);
    })
    .afterEach(async (t) => {
        try {
            await useLiveNames();
        } finally {
            if (t.ctx.legacyFontSizeCaptured) await storeLegacyFontSize(t.ctx.originalLegacyFontSize);
            await restoreSettings(t);
        }
    });

test('symbol source lists primitives, user procedures, variables, and library procedures', async (t) => {
    await runLogo(t, 'define "squiggle [[] [fd 1]] make "zebra 3');
    const kinds = await ClientFunction(() => Object.fromEntries(
        Module.completion.names().map((entry) => [entry.name, entry.kind])))();
    await t.expect(kinds.forward).eql('primitive')
        .expect(kinds.squiggle).eql('procedure')
        .expect(kinds.zebra).eql('variable')
        .expect(kinds.foreach).eql('library');
});

test('editor: completes words while Logo is suspended', async (t) => {
    await t.typeText(commandInput, 'edall', { paste: true }).pressKey('enter');
    await t.expect(editorDialogOpen()).ok('Logo editor should open', { timeout: 10000 });
    try {
        await prepareEditorCompletion();
        await clickEditorSuggestionInPage('forward');
        await t.expect(await editorState()).eql({
            dialogOpen: true,
            value: 'to demo\n  forward 10\nend',
            caret: 17,
            focused: true,
        });
    } finally {
        await discardEditor();
        await t.expect(logoIdle()).ok('Logo should return to idle after discarding the editor');
    }
});

test('editor: Tab completion preserves newline and Escape behavior', async (t) => {
    await t.typeText(commandInput, 'edall', { paste: true }).pressKey('enter');
    await t.expect(editorDialogOpen()).ok('Logo editor should open', { timeout: 10000 });
    try {
        const editor = Selector('#editorTextArea');
        await prepareEmptyEditor();
        await t.typeText(editor, 'forw').pressKey('tab');
        await t.expect(editor.value).eql('forward ');
        await t.typeText(editor, 'forw').pressKey('esc');
        await t.expect(editorDialogOpen()).ok()
            .expect(Selector('logo-autocomplete[for="editorTextArea"]').shadowRoot()
                .find('[role="listbox"]').visible).notOk();
        await t.pressKey('enter');
        await t.expect(editor.value).eql('forward forw\n');
        await t.typeText(editor, 'forw').pressKey('ctrl+[');
        await t.expect(editorDialogOpen()).ok()
            .expect(Selector('logo-autocomplete[for="editorTextArea"]').shadowRoot()
                .find('[role="listbox"]').visible).notOk()
            .expect(editor.value).eql('forward forw\nforw')
            .expect(editor.focused).ok();
    } finally {
        await discardEditor();
        await t.expect(logoIdle()).ok('Logo should return to idle after discarding the editor');
    }
});

test('stub: closed combobox until a matching word is typed', async (t) => {
    await useStubNames(['forward', 'foreach', 'format', 'back']);
    await resetInput();
    const state = await popupState();
    await t.expect(state.role).eql('combobox')
        .expect(state.autocomplete).eql('list')
        .expect(state.expanded).eql('false')
        .expect(state.open).notOk();
});

test('stub: shows matching suggestions while typing and closes when nothing matches', async (t) => {
    await useStubNames(['forward', 'foreach', 'format', 'back']);
    await resetInput();
    await typeSlowly(t, 'fo');
    let state = await popupState();
    await t.expect(state.open).ok()
        .expect(state.expanded).eql('true')
        .expect(state.options).eql(['forward', 'foreach', 'format'])
        .expect(state.status).match(/3 suggestions/);
    await typeSlowly(t, 'rw');
    await t.expect((await popupState()).options).eql(['forward']);
    await resetInput();
    await typeSlowly(t, 'zz');
    await t.expect((await popupState()).open).notOk();
});

test('setting: disables suggestions in command input and editor', async (t) => {
    await t.expect(Selector('#autocompleteSelect').checked).ok('autocomplete is enabled by default');
    await useStubNames(['forward']);
    await resetInput();
    await typeSlowly(t, 'fo');
    await t.expect((await popupState()).open).ok();

    await openSettingsDialog();
    await t.click(Selector('#autocompleteSelect'));
    await t.expect(ClientFunction(() => Module.settings.autocomplete)()).eql(false);
    await t.click(Selector('#settings button').withText('Close settings'));
    await resetInput();
    await typeSlowly(t, 'fo');
    const state = await popupState();
    await t.expect(state.open).notOk()
        .expect(state.expanded).eql('false')
        .expect(state.autocomplete).eql('none')
        .expect(state.value).eql('fo');

    await t.selectText(commandInput).pressKey('delete')
        .typeText(commandInput, 'edall', { paste: true }).pressKey('enter');
    await t.expect(editorDialogOpen()).ok('Logo editor should open', { timeout: 10000 });
    try {
        await prepareEditorCompletion();
        await t.expect(Selector('logo-autocomplete[for="editorTextArea"]')
            .shadowRoot().find('[role="listbox"]').visible).notOk();
    } finally {
        await discardEditor();
        await t.expect(logoIdle()).ok('Logo should return to idle after discarding the editor');
    }
});

test('stub: tapping a suggestion completes only the word at the caret', async (t) => {
    await useStubNames(['forward', 'foreach', 'format', 'back']);
    await resetInput();
    await typeSlowly(t, 'repeat 4 [fo');
    await t.expect((await popupState()).options).contains('forward');
    await clickSuggestion(t, 'forward');
    const state = await popupState();
    await t.expect(state.value).eql('repeat 4 [forward ')
        .expect(state.caret).eql(state.value.length)
        .expect(state.focused).ok()
        .expect(state.open).notOk();
});

test('stub: keyboard navigates, accepts, and dismisses suggestions', async (t) => {
    await useStubNames(['forward', 'foreach', 'format']);
    await resetInput();
    await typeSlowly(t, 'fo');
    await t.pressKey('down');
    let state = await popupState();
    await t.expect(state.selected).eql(['forward'])
        .expect(state.activeDescendant).eql('logo-word-option-0')
        .expect(state.value).eql('fo')
        .expect(state.focused).ok()
        .expect(state.optionBackgrounds.forward).notEql(state.optionBackgrounds.foreach);
    await t.pressKey('down');
    state = await popupState();
    await t.expect(state.selected).eql(['foreach']);
    await t.pressKey('up');
    await t.pressKey('enter');
    state = await popupState();
    await t.expect(state.value).eql('forward ')
        .expect(state.open).notOk()
        .expect(state.focused).ok();
    await typeSlowly(t, 'fo');
    await t.pressKey('esc');
    state = await popupState();
    await t.expect(state.open).notOk().expect(state.value).eql('forward fo');
});

test('stub: Tab accepts the first suggestion without leaving the input', async (t) => {
    await useStubNames(['forward', 'foreach', 'format']);
    await resetInput();
    await typeSlowly(t, 'fo');
    await t.pressKey('tab');
    const state = await popupState();
    await t.expect(state.value).eql('forward ')
        .expect(state.open).notOk()
        .expect(state.focused).ok();
});

test('stub: Tab accepts the active suggestion without leaving the input', async (t) => {
    await useStubNames(['forward', 'foreach', 'format']);
    await resetInput();
    await typeSlowly(t, 'fo');
    await t.pressKey('down').pressKey('down').pressKey('tab');
    const state = await popupState();
    await t.expect(state.value).eql('foreach ')
        .expect(state.open).notOk()
        .expect(state.focused).ok();
});

test('stub: Ctrl+[ dismisses suggestions like Escape', async (t) => {
    await useStubNames(['forward', 'foreach', 'format']);
    await resetInput();
    await typeSlowly(t, 'fo');
    await t.pressKey('ctrl+[');
    const state = await popupState();
    await t.expect(state.open).notOk()
        .expect(state.value).eql('fo')
        .expect(state.focused).ok();
});

test('global: Ctrl+[ closes an open modal dialog like Escape', async (t) => {
    await openSettingsDialog();
    await t.expect(settingsDialogOpen()).ok();
    await t.pressKey('ctrl+[');
    await t.expect(settingsDialogOpen()).notOk();
});

test('global: Ctrl+[ with nothing to dismiss leaves the command line alone', async (t) => {
    await useStubNames(['forward']);
    await resetInput();
    await typeSlowly(t, 'print');
    await t.pressKey('ctrl+[');
    const state = await popupState();
    await t.expect(state.value).eql('print').expect(state.focused).ok();
});

test('stub: help summary and argument syntax, none for undocumented words', async (t) => {
    await useStubNames(['forward', 'foreach', 'make', 'squiggle']);
    await resetInput();
    await typeSlowly(t, 'f');
    let state = await popupState();
    await t.expect(state.summaries.forward).eql(
        'Moves the turtle forward, in the direction that it\'s facing, by the specified distance (measured in turtle steps).')
        .expect(state.usages.forward).eql(['FORWARD dist', 'FD dist'])
        .expect(state.usages.foreach).eql(['FOREACH data template', '(FOREACH data1 data2 ... template)']);
    await resetInput();
    await typeSlowly(t, 'm');
    await t.expect((await popupState()).summaries.make).eql(
        'Assigns the value "value" to the variable named "varname", which must be a word.');
    await resetInput();
    await typeSlowly(t, 's');
    state = await popupState();
    await t.expect(state.summaries.squiggle).eql(null).expect(state.usages.squiggle).eql([]);
});

test('stub: closes after the command is submitted', async (t) => {
    await useStubNames(['forward', 'foreach', 'format', 'back']);
    await resetInput();
    await typeSlowly(t, 'fo');
    await t.pressKey('enter').expect(logoIdle()).ok();
    await t.expect((await popupState()).open).notOk();
});

test('live: completes a real command from primitives with help', async (t) => {
    await useLiveNames();
    await resetInput();
    await typeSlowly(t, 'rep');
    let state = await popupState();
    await t.expect(state.options).contains('repeat')
        .expect(state.usages.repeat).eql(['REPEAT num instructionlist'])
        .expect(state.summaries.repeat).eql('Runs the "instructionlist" repeatedly, "num" times.');
    await clickSuggestion(t, 'repeat');
    await typeSlowly(t, '4 [forw');
    await clickSuggestion(t, 'forward');
    await typeSlowly(t, '50 righ');
    await t.expect((await popupState()).usages.right).eql(['RIGHT degrees', 'RT degrees']);
    await clickSuggestion(t, 'right');
    await typeSlowly(t, '90]');
    state = await popupState();
    await t.expect(state.value).eql('repeat 4 [forward 50 right 90]').expect(state.open).notOk();
    await t.pressKey('enter').expect(logoIdle()).ok();
    await t.expect(Selector('#output').innerText).contains('repeat 4 [forward 50 right 90]')
        .expect(Selector('#output').innerText).notContains('I don\'t know how');
});

test('live: suggests procedures and variables defined in the session', async (t) => {
    await useLiveNames();
    await runLogo(t, 'define "zigzag [[] [fd 1]] make "zeppelin 3');
    await resetInput();
    await typeSlowly(t, 'zi');
    await t.expect((await popupState()).options).contains('zigzag');
    await resetInput();
    await typeSlowly(t, 'print :zep');
    await t.expect((await popupState()).options).contains('zeppelin');
    await clickSuggestion(t, 'zeppelin');
    await t.expect((await popupState()).value).eql('print :zeppelin ');
});

test('live: no completion while Logo is blocked reading continuation lines', async (t) => {
    await useLiveNames();
    await resetInput();
    await t.typeText(commandInput, 'print [', { paste: true }).pressKey('enter')
        .expect(blockedOnInput()).ok('Logo should wait for the rest of the list');
    await typeSlowly(t, 'fo');
    await t.expect((await popupState()).open).notOk('popup must stay closed while the C stack is suspended');
    await t.typeText(commandInput, ']', { paste: true }).pressKey('enter')
        .expect(logoIdle()).ok();
    await resetInput();
    await typeSlowly(t, 'forw');
    await t.expect((await popupState()).open).ok('popup works again once Logo is idle')
        .expect((await popupState()).options).contains('forward');
});

test('live: no completion while a Logo program is running', async (t) => {
    await useLiveNames();
    await resetInput();
    await t.typeText(commandInput, 'wait 180', { paste: true }).pressKey('enter')
        .expect(ClientFunction(() => getValue(Module._LOGO_GIL, 'i32') !== 0)())
        .ok('Logo should start running');
    await typeSlowly(t, 'fo');
    await t.expect((await popupState()).open).notOk('popup must stay closed while Logo is running');
    await t.expect(logoIdle()).ok('Logo should return to idle');
    await resetInput();
    await typeSlowly(t, 'forw');
    await t.expect((await popupState()).open).ok('popup works again once Logo is idle')
        .expect((await popupState()).options).contains('forward');
});

test('performance: popup renders within the measured latency budget', async (t) => {
    await useLiveNames();
    const samples = [];
    for (const text of ['f', 'fo', 're', 'p', 's', 'zz']) samples.push(await measurePopupLatency(text));
    const worst = Math.max(...samples.map((sample) => sample.elapsed));
    console.log(`      popup latency ms: ${samples.map((s) => s.elapsed.toFixed(2)).join(', ')} (worst ${worst.toFixed(2)})`);
    if (POPUP_LATENCY_BUDGET_MS) await t.expect(worst).lte(POPUP_LATENCY_BUDGET_MS);
    await resetInput();
});

test('layout: input and popup stay inside the window, fit, and are touch sized', async (t) => {
    await useStubNames(['forward', 'foreach']);
    await resetInput();
    const idleLayout = await ClientFunction(() => ({
        input: document.getElementById('logoInputText').getBoundingClientRect().toJSON(),
        viewportHeight: window.visualViewport?.height ?? window.innerHeight,
    }))();
    await t.expect(idleLayout.input.top).gte(0)
        .expect(idleLayout.input.bottom).lte(idleLayout.viewportHeight + 0.5);
    await screenshot(t, 'idle');
    await typeSlowly(t, 'f');
    await screenshot(t, 'popup');
    for (const name of ['forward', 'foreach']) {
        const layout = await optionLayout(name);
        await t.expect(layout.scrollY).eql(0)
            .expect(layout.input.top).gte(0)
            .expect(layout.input.bottom).lte(layout.viewportHeight + 0.5)
            .expect(layout.listbox.top).gte(0)
            .expect(layout.listbox.bottom).lte(layout.input.top + 0.5)
            .expect(layout.listbox.left).gte(0)
            .expect(layout.listbox.right).lte(layout.viewportWidth + 0.5)
            .expect(layout.listboxOverflow).lte(0)
            .expect(layout.optionOverflow).lte(0)
            .expect(layout.option.height).gte(44)
            .expect(layout.summary.top).gte(layout.name.bottom - 0.5);
        if (layout.viewportWidth >= layout.inlineViewportWidth) {
            await t.expect(layout.usage.top).lt(layout.name.bottom, `${name} syntax on the same line`)
                .expect(layout.usage.left).gte(layout.name.right);
        } else if (name === 'foreach') {
            await t.expect(layout.usage.top).gte(layout.name.bottom - 0.5, 'long syntax wraps below the word');
        }
    }
});
