const { ClientFunction, Selector } = require('testcafe');
const { applicationUrl, ready, runLogo } = require('./testcafe-helpers');

const beginObserving = ClientFunction(() => {
    window.storagePersistenceEvents = [];
    window.storagePersistenceTestFile = `storage-test-${Date.now()}-${Math.random().toString(36).slice(2)}.lg`;
    for (const type of ['saved', 'persisted', 'savefailed']) {
        Module.storage.addEventListener(type, (event) => {
            window.storagePersistenceEvents.push({ type, detail: event.detail });
        });
    }
    return window.storagePersistenceTestFile;
});
const readStorageView = ClientFunction(() => Module.storage.status().then((status) => ({
    status,
    events: window.storagePersistenceEvents.map(({ type }) => type),
})));
const hasPersisted = ClientFunction(() =>
    window.storagePersistenceEvents.some((event) => event.type === 'persisted'));
const removeTestFile = ClientFunction(() => {
    const path = `/d/${window.storagePersistenceTestFile || ''}`;
    if (!window.storagePersistenceTestFile) return;
    if (FS.analyzePath(path).exists) FS.unlink(path);
});

fixture('Storage persistence')
    .page(applicationUrl)
    .beforeEach(async (t) => {
        await ready(t);
    })
    .afterEach(async () => {
        await removeTestFile();
    });

test('STORAGE-001 Logo save is observed and Settings reports browser storage', async (t) => {
    const fileName = await beginObserving();
    await runLogo(t, 'define "storagepersistencetest [ [] [print "saved] ]');
    await runLogo(t, `save "${fileName}`);
    await t.expect(hasPersisted()).ok('IDBFS save should complete', { timeout: 30000 });
    await t.click(Selector('#appContainer button[aria-controls="settings"]'));
    await t.expect(Selector('#storageState').innerText).match(
        /protected by the browser|may delete saved files|status is unknown/,
    );
    await t.expect(Selector('#storageUsage').innerText).match(/Approximate/);
    const view = await readStorageView();
    await t.expect(Selector('#settings').exists).ok()
        .expect(Selector('#storageState').getAttribute('role')).eql('status')
        .expect(view.events).contains('saved')
        .expect(view.events).contains('persisted')
        .expect(view.status.populated).eql(true)
        .expect(typeof view.status.api).eql('boolean')
        .expect([null, true, false]).contains(view.status.persisted)
        .expect(view.status.usage === null || typeof view.status.usage === 'number').ok()
        .expect(view.status.quota === null || typeof view.status.quota === 'number').ok()
        .expect(view.status.lastSaved).gt(0)
        .click(Selector('#settings button').withText('Close settings'));
    await runLogo(t, 'erase "storagepersistencetest');
});