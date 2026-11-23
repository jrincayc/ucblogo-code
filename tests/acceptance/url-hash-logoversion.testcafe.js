const { ClientFunction } = require('testcafe');
const { applicationUrl, ready, runLogo, idle, output } = require('./testcafe-helpers');

const reproductionSearch = '?fbclid=IwZXh0bgNhZW0CMTAAcGRvZgVicmlkETF6dmV6S1VIdnZoRGducUEwc3J0YwZhcHBfaWQQMjIyMDM5MTc4ODIwMDg5MgABH9xLKINFMtT3IIBjfCjoX_rIWzEb1HEP5_G0Ufrx14guqFYZa9It0ql6EnSw_aem_2tB-eB7LDJNNugR9HRaqyA';
const reproductionHash = '#logoversion=%3Bwindow.__ucblogoUrlProbe%3Dtrue%3B%2F%2F';
const readUrlState = ClientFunction(() => ({
    href: location.href,
    hash: location.hash,
    search: location.search,
}));
const readHashProbe = ClientFunction(() =>
    window.__ucblogoUrlProbe === undefined ? null : window.__ucblogoUrlProbe);

fixture('URL parameter isolation')
    .page(applicationUrl.split(/[?#]/)[0]);

test('ACC-URL-001 URL search and hash data are not Logo command-line file arguments', async (t) => {
    await ready(t);

    await t.navigateTo(`${applicationUrl.split(/[?#]/)[0]}${reproductionSearch}${reproductionHash}`);
    await t.expect(idle()).ok('Logo must initialize after URL navigation', { timeout: 30000 });

    const urlState = await readUrlState();
    await t.expect(urlState.search).eql(reproductionSearch)
        .expect(urlState.hash).eql(reproductionHash);

    await runLogo(t, 'print logoversion');

    await t.expect(output.textContent).contains('6.2', 'logoversion must remain numeric 6.2');
    await t.expect(output.textContent).notContains("I can't open file",
        'URL parameters must not be interpreted as Logo files');
    await t.expect(output.textContent).notContains('fbclid=',
        'URL search data must not appear in terminal output');
    await t.expect(output.textContent).notContains('__ucblogoUrlProbe',
        'URL hash data must not appear in terminal output');
    await t.expect(readHashProbe()).eql(null,
        'URL hash payload must not create a browser global');

    await runLogo(t, 'print emptyp :command.line');
    await t.expect(output.textContent).contains('true',
        'URL search and hash parameters must not populate command.line');
});

test('ACC-URL-002 early key presses do not abort startup', async (t) => {
    await t.pressKey('a');
    await ready(t);
    await runLogo(t, 'print logoversion');
    await t.expect(output.textContent).contains('6.2');
});
