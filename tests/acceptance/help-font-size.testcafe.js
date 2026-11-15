const { ClientFunction, Selector } = require('testcafe');
const {
    applicationUrl, ready, isolateSettings, restoreSettings,
} = require('./testcafe-helpers');

const helpButton = Selector('button[aria-controls="helpDialog"]').filterVisible().nth(0);

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
const openEntry = async (t) => {
    await t.click(helpButton)
        .click(Selector('#helpCatList h3 button').withText('Constructors'))
        .click(Selector('#helpCatList ul button').withText(/^fput$/))
        .expect(Selector('#helpBody').innerText).contains('outputs a list');
};

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
        await openEntry(t);
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
