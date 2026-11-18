import { ClientFunction } from 'testcafe';

fixture('Runner self-test').page(process.env.TESTCAFE_APP_URL);

test('intentional assertion failure', async (controller) => {
    await controller.expect(1).eql(2, 'intentional runner self-test failure');
});

test('hung browser action', async () => {
    console.log('HUNG_ACTION_STARTED');
    await ClientFunction(() => new Promise(() => {}))();
});

test('shared capability case', async (controller) => {
    await controller.expect(true).ok();
});

test.meta({ requires: 'resize' })('requires resize', async (controller) => {
    await controller.expect(false).ok('resize case should have been filtered');
});

test.meta({ payload: 'report-data'.repeat(1024 * 1024) })('large JSON report', async (controller) => {
    await controller.expect(true).ok();
});