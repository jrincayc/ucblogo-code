const { ClientFunction, Selector } = require('testcafe');
const { applicationUrl, ready, commandInput } = require('./testcafe-helpers');

const helpButton = Selector('button[aria-controls="helpDialog"]').filterVisible().nth(0);
const dialog = Selector('#helpDialog');
const search = Selector('#helpSearch');
const dialogOpen = ClientFunction(() => document.getElementById('helpDialog').open);
const activeId = ClientFunction(() => document.activeElement.id);
const inputValue = ClientFunction(() => document.getElementById('logoInputText').value);
const pressF1 = ClientFunction(() => document.dispatchEvent(
    new KeyboardEvent('keydown', { key: 'F1', bubbles: true, cancelable: true })));

fixture('Help dialog')
    .page(applicationUrl)
    .beforeEach(async (t) => {
        await ready(t);
    });

test('HELP-001 opens with focus in search and closes with Close', async (t) => {
    await t.click(helpButton)
        .expect(dialogOpen()).ok()
        .expect(activeId()).eql('helpSearch')
        .click('#helpClose')
        .expect(dialogOpen()).notOk();
});

test('HELP-002 F1 opens the dialog', async (t) => {
    await pressF1();
    await t.expect(dialogOpen()).ok();
});

test('HELP-003 browses categories to a word entry', async (t) => {
    await t.click(helpButton)
        .click(Selector('#helpCatList h3 button').withText('Constructors'))
        .click(Selector('#helpCatList ul button').withText(/^fput$/))
        .expect(Selector('#helpDetailTitle').innerText).eql('fput')
        .expect(Selector('#helpUsage').innerText).contains('FPUT')
        .expect(Selector('#helpBody').innerText).contains('outputs a list');
});

test('HELP-004 search shows ranked results, none, and clearing', async (t) => {
    await t.click(helpButton)
        .typeText(search, 'fput')
        .expect(Selector('#helpResults li').count).gte(1)
        .expect(Selector('#helpResults li button').nth(0).innerText).contains('fput')
        .expect(Selector('#helpCount').innerText).match(/result/)
        .selectText(search).typeText(search, 'zzzqxx')
        .expect(Selector('#helpCount').innerText).eql('No results')
        .selectText(search).pressKey('delete')
        .expect(Selector('#helpResults').visible).notOk();
});

test('HELP-005 Insert adds the word and a space, keeping the dialog open', async (t) => {
    await t.click(helpButton)
        .typeText(search, 'fput')
        .click(Selector('#helpResults li button').nth(0))
        .click(Selector('.helpInsert[data-close="0"]'))
        .expect(inputValue()).eql('fput ')
        .expect(dialogOpen()).ok()
        .expect(Selector('#helpLive').innerText).contains('Inserted fput');
});

test('HELP-006 Insert and close returns focus to the command input', async (t) => {
    await t.click(helpButton)
        .typeText(search, 'repeat')
        .click(Selector('#helpResults li button').nth(0))
        .click(Selector('.helpInsert[data-close="1"]'))
        .expect(dialogOpen()).notOk()
        .expect(activeId()).eql('logoInputText')
        .expect(inputValue()).contains(' ');
});

const separatorWidth = ClientFunction((selector) => {
    const items = document.querySelectorAll(selector);
    return items.length > 1 ? Number.parseFloat(getComputedStyle(items[1]).borderTopWidth) : null;
});

test('HELP-007 list items are visually separated', async (t) => {
    await t.click(helpButton)
        .expect(separatorWidth('#helpCatList li')).gte(1)
        .click(Selector('#helpCatList h3 button').withText('Constructors'))
        .expect(separatorWidth('#helpCatList ul:not([hidden]) li')).gte(1)
        .typeText(search, 'list')
        .expect(separatorWidth('#helpResults li')).gte(1);
});

test('HELP-008 categories expand one at a time with aria-expanded', async (t) => {
    const constructors = Selector('#helpCatList h3 button').withText('Constructors');
    const selectors = Selector('#helpCatList h3 button').withText('Data Selectors');
    await t.click(helpButton)
        .expect(constructors.getAttribute('aria-expanded')).eql('false')
        .click(constructors)
        .expect(constructors.getAttribute('aria-expanded')).eql('true')
        .expect(Selector('#helpCatList ul:not([hidden]) button').withText(/^fput$/).visible).ok()
        .click(selectors)
        .expect(selectors.getAttribute('aria-expanded')).eql('true')
        .expect(constructors.getAttribute('aria-expanded')).eql('false')
        .expect(Selector('#helpCatList ul:not([hidden])').count).eql(1)
        .click(selectors)
        .expect(Selector('#helpCatList ul:not([hidden])').count).eql(0);
});

test('HELP-009 selecting a search result opens its category and marks the word', async (t) => {
    await t.click(helpButton)
        .typeText(search, 'fput')
        .click(Selector('#helpResults li button').nth(0))
        .expect(Selector('#helpCatList h3 button[aria-expanded="true"]').count).eql(1)
        .expect(Selector('#helpCatList button[aria-current="true"]').innerText).eql('fput');
});

const paneVisibility = ClientFunction(() => {
    const visible = (id) => {
        const node = document.getElementById(id) || document.querySelector(id);
        const box = node.getBoundingClientRect();
        return getComputedStyle(node).display !== 'none' && box.width > 0 && box.height > 0;
    };
    return { list: visible('.helpNav'), detail: visible('helpDetail') };
});

test.meta({ requires: 'resize' })('HELP-010 wide window keeps the word list beside the entry', async (t) => {
    await t.resizeWindow(1280, 800)
        .click(helpButton)
        .click(Selector('#helpCatList h3 button').withText('Constructors'))
        .click(Selector('#helpCatList ul button').withText(/^fput$/))
        .expect(Selector('#helpDetailTitle').innerText).eql('fput')
        .expect(paneVisibility()).eql({ list: true, detail: true })
        .click(Selector('#helpCatList ul button').withText(/^list$/))
        .expect(Selector('#helpDetailTitle').innerText).eql('list')
        .expect(paneVisibility()).eql({ list: true, detail: true });
});

test.meta({ requires: 'resize' })('HELP-011 narrow window shows the entry full width and Back returns to the list', async (t) => {
    await t.resizeWindow(390, 844)
        .click(helpButton)
        .click(Selector('#helpCatList h3 button').withText('Constructors'))
        .click(Selector('#helpCatList ul button').withText(/^fput$/))
        .expect(paneVisibility()).eql({ list: false, detail: true })
        .click('#helpBackWords')
        .expect(paneVisibility()).eql({ list: true, detail: false })
        .expect(ClientFunction(() => document.activeElement.textContent)()).eql('fput');
});

// Result name must sit above its snippet, and a snippet must start at a word boundary of the body.
const resultProblems = ClientFunction(() => {
    const problems = [];
    let checked = 0;
    for (const button of document.querySelectorAll('#helpResults li button')) {
        if (checked++ >= 15) break;
        const name = button.querySelector('span').getBoundingClientRect();
        const small = button.querySelector('small');
        if (small.getBoundingClientRect().top < name.bottom - 1) problems.push(`side by side: ${button.innerText}`);
        const snippet = small.textContent.replace(/^\u2026/, '').replace(/\u2026$/, '');
        const word = Module.help.load().byName.get(button.querySelector('span').textContent.split(',')[0].trim());
        const body = word ? Module.help.body(word).replace(/\s+/g, ' ') : '';
        const at = body.indexOf(snippet.slice(0, 20));
        if (word && at > 0 && body[at - 1] !== ' ') problems.push(`mid-word start: ${snippet.slice(0, 20)}`);
        const stop = body.indexOf(snippet) + snippet.length;
        if (word && stop < body.length && body[stop] !== ' ') problems.push(`mid-word end: ${snippet.slice(-20)}`);
    }
    return problems;
});

test.meta({ requires: 'resize' })('HELP-012 search results stack name over snippet and start snippets at a word', async (t) => {
    await t.resizeWindow(1280, 800)
        .click(helpButton)
        .typeText(search, 'turtle')
        .expect(Selector('#helpResults li').count).gte(5)
        .expect(resultProblems()).eql([]);
});
