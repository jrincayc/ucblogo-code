const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl, ready, isolateSettings, restoreSettings,
} = require('./testcafe-helpers');

const setTextSize = ClientFunction((size) => Module.setFontSize(size));
// Computed px size of each representative Help element, plus a reference element outside the dialog.
const helpFontSizes = ClientFunction(() => {
    const size = (selector) => {
        const node = document.querySelector(selector);
        return node ? Number.parseFloat(getComputedStyle(node).fontSize) : null;
    };
    return {
        reference: size('#logoRunButton'),
        title: size('#helpTitle'),
        search: size('#helpSearch'),
        close: size('#helpClose'),
        category: size('#helpCatList h3 button'),
        word: size('#helpCatList ul button'),
        count: size('#helpCount'),
        usage: size('#helpUsage'),
        body: size('#helpBody'),
    };
});
// Opening via helpUI keeps this test independent of toolbar layout at large sizes; button clicks are covered in help-dialog.testcafe.js.
const openHelp = ClientFunction(() => helpUI.open());
const helpOpen = ClientFunction(() => document.getElementById('helpDialog').open);
// Word buttons are built when a category is expanded.
const expandFirstCategory = ClientFunction(() => document.querySelector('#helpCatList h3 button').click());
const helpWords = Selector('#helpCatList ul button');

fixture('Help dialog follows the Logo text size')
    .page(applicationUrl)
    .beforeEach(async (t) => {
        await ready(t);
        await isolateSettings(t);
    })
    .afterEach(async (t) => {
        await restoreSettings(t);
    });

// The page sets one px size on every element; the dialog must match it, except for explicit em scaling.
for (const size of [16, 24, 40]) {
    test(`HELP-FONT-${size} dialog text uses the ${size}px Logo text size`, async (t) => {
        await setTextSize(size);
        await openHelp();
        await t.expect(helpOpen()).ok('Help dialog opens');
        await expandFirstCategory();
        await t.expect(helpWords.count).gt(0, 'Help word list is built');
        const sizes = await helpFontSizes();
        await t.expect(sizes.reference).eql(size);
        for (const part of ['title', 'search', 'close', 'category', 'word', 'usage', 'body']) {
            await t.expect(Math.round(sizes[part] / sizes.reference * 100) / 100)
                .gte(part === 'title' ? 1 : 1, `${part} is ${sizes[part]}px at Logo text size ${size}px`)
                .expect(sizes[part]).lte(size * 1.1, `${part} is ${sizes[part]}px at ${size}px`);
        }
        await t.expect(sizes.count).within(size * 0.8, size);
    });
}
