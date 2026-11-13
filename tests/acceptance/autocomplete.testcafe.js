const fs = require('node:fs');
const path = require('node:path');
const { ClientFunction, Selector } = require('testcafe');

const applicationUrl = process.env.TESTCAFE_APP_URL || 'http://127.0.0.1:8085/ucblogo.html';
const screenshotDirectory = path.join(__dirname, '../../reports/autocomplete');
const viewerDelay = Number(process.env.TEST_DELAY_MS) || 0;
const interactionDelay = Number(process.env.TEST_INTERACTION_DELAY_MS) || 100;
// Measured worst-case in-page latency (ms) from input event to rendered popup, plus 10%.
const POPUP_LATENCY_BUDGET_MS = Number(process.env.POPUP_LATENCY_BUDGET_MS) || 0;

const commandInput = Selector('#logoInputText');
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

const logoIdle = ClientFunction(() => Boolean(window.Module && Module.calledRun &&
    Asyncify.state === 0 && getValue(Module._LOGO_GIL) === 0 && !Module.blockedOnGetc));
const blockedOnInput = ClientFunction(() => Boolean(Module.blockedOnGetc));
const isPhone = ClientFunction(() => navigator.maxTouchPoints > 0);

const popupState = ClientFunction(() => {
    const input = document.getElementById('logoInputText');
    const root = document.querySelector('logo-autocomplete').shadowRoot;
    const box = root.querySelector('[role="listbox"]');
    const names = [];
    const summaries = {};
    const usages = {};
    for (const node of box.querySelectorAll('[role="option"]')) {
        const word = node.querySelector('.name').textContent;
        names.push(word);
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
    return {
        viewportWidth: document.documentElement.clientWidth,
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

async function runLogo(t, command) {
    await t.selectText(commandInput).pressKey('delete')
        .typeText(commandInput, command, { paste: true }).pressKey('enter')
        .expect(logoIdle()).ok('Logo must return to idle');
    await pause(t);
}

async function screenshot(t, name) {
    fs.mkdirSync(screenshotDirectory, { recursive: true });
    const mode = (await isPhone()) ? 'phone' : 'desktop';
    const file = path.join(screenshotDirectory, `${mode}-${name}.png`);
    await t.takeScreenshot({ path: path.relative(path.join(__dirname, '../../reports'), file), fullPage: false });
}

fixture('Logo word autocompletion')
    .page(applicationUrl)
    .beforeEach(async (t) => {
        if (!(await isPhone())) await t.resizeWindow(1280, 800);
        await t.expect(Selector('#output').innerText).contains('Welcome to Berkeley Logo', { timeout: 30000 })
            .expect(logoIdle()).ok('Logo must start idle', { timeout: 30000 });
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

test('stub: tapping a suggestion completes only the word at the caret', async (t) => {
    await useStubNames(['forward', 'foreach', 'format', 'back']);
    await resetInput();
    await typeSlowly(t, 'repeat 4 [fo');
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
        .expect(state.activeDescendant).eql('logo-word-option-0');
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
    await clickSuggestion(t, 'zeppelin');
    await t.expect((await popupState()).value).eql('print :zeppelin ');
});

test('live: no completion while Logo is blocked reading continuation lines', async (t) => {
    await useLiveNames();
    await resetInput();
    await t.typeText(commandInput, 'repeat 2 [', { paste: true }).pressKey('enter')
        .expect(blockedOnInput()).ok('Logo should wait for the rest of the list');
    await typeSlowly(t, 'forw');
    await t.expect((await popupState()).open).notOk('popup must stay closed while the C stack is suspended');
    await t.selectText(commandInput).pressKey('delete').typeText(commandInput, ']', { paste: true }).pressKey('enter')
        .expect(logoIdle()).ok();
    await resetInput();
    await typeSlowly(t, 'forw');
    await t.expect((await popupState()).open).ok('popup works again once Logo is idle');
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
    await screenshot(t, 'idle');
    await typeSlowly(t, 'f');
    await screenshot(t, 'popup');
    const phone = await isPhone();
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
        if (!phone) {
            await t.expect(layout.usage.top).lt(layout.name.bottom, `${name} syntax on the same line`)
                .expect(layout.usage.left).gte(layout.name.right);
        }
    }
    if (phone) {
        const layout = await optionLayout('foreach');
        await t.expect(layout.usage.top).gte(layout.name.bottom - 0.5, 'long syntax wraps below the word');
    }
});
