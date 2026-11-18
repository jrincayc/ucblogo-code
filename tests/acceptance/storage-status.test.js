'use strict';

const { describe, it } = require('node:test');
const assert = require('node:assert/strict');
const vm = require('node:vm');
const fs = require('node:fs');
const path = require('node:path');
const { createStorageStatusController } = require('../../storage-status.js');

class TestEvents {
    listeners = new Map();

    addEventListener(name, listener) {
        this.listeners.set(name, [...(this.listeners.get(name) || []), listener]);
    }

    async emit(name) {
        await Promise.all((this.listeners.get(name) || []).map((listener) => listener()));
    }
}

function makeController({ storageApi, initialRecord = {}, standalone = false, now = 1000 } = {}) {
    const events = new TestEvents();
    const records = { ...initialRecord };
    let persistCalls = 0;
    const controller = createStorageStatusController({
        storageApi: storageApi || {
            persisted: async () => false,
            estimate: async () => ({ usage: 1200, quota: 8000 }),
            persist: async () => { persistCalls++; return false; },
        },
        events,
        getState: () => ({ populated: true, lastSaved: null, lastError: null }),
        getPersistenceRecord: () => records,
        setPersistenceRecord: (record) => Object.assign(records, record),
        isStandalone: () => standalone,
        now: () => now,
    });
    return { controller, events, records, get persistCalls() { return persistCalls; } };
}

describe('storage status controller', () => {
    it('loads without browser globals and tolerates absent or rejecting Storage APIs', async () => {
        const source = fs.readFileSync(path.join(__dirname, '../../storage-status.js'), 'utf8');
        const sandbox = { module: { exports: {} }, globalThis: {} };
        vm.runInNewContext(source, sandbox);
        assert.equal(typeof sandbox.module.exports.createStorageStatusController, 'function');

        const absent = createStorageStatusController({
            getState: () => ({ populated: null }),
            getPersistenceRecord: () => ({}),
            setPersistenceRecord() {},
        });
        assert.deepEqual(await absent.status(), {
            api: false, persisted: null, usage: null, quota: null, populated: null,
        });

        const rejected = makeController({ storageApi: {
            persisted: async () => { throw new Error('unavailable'); },
            estimate: async () => { throw new Error('unavailable'); },
        } });
        const status = await rejected.controller.status();
        assert.equal(status.persisted, null);
        assert.equal(status.usage, null);
        assert.equal(status.quota, null);
    });

    it('reports granted state and zero-valued estimates', async () => {
        const { controller } = makeController({ storageApi: {
            persisted: async () => true,
            estimate: async () => ({ usage: 0, quota: 0 }),
        } });
        const status = await controller.status();
        assert.equal(status.persisted, true);
        assert.equal(status.usage, 0);
        assert.equal(status.quota, 0);
    });

    it('requests only after a successful-save event and limits automatic retries', async () => {
        const result = makeController();
        assert.equal(result.persistCalls, 0);
        await result.events.emit('persisted');
        assert.equal(result.persistCalls, 1);
        assert.deepEqual(result.records, { time: 1000, result: false });
        await result.events.emit('persisted');
        assert.equal(result.persistCalls, 1);
    });

    it('honors cooldown, retries after installation, and lets an explicit request bypass cooldown', async () => {
        const result = makeController({ initialRecord: { result: false, time: 900 } });
        await result.events.emit('persisted');
        assert.equal(result.persistCalls, 0);
        await result.events.emit('appinstalled');
        assert.equal(result.persistCalls, 1);

        const explicit = makeController({ initialRecord: { result: false, time: 900 } });
        assert.equal(await explicit.controller.requestPersistence('button', true), false);
        assert.equal(explicit.persistCalls, 1);
    });

    it('retries on the first standalone save after a denied tab request', async () => {
        const result = makeController({ initialRecord: { result: false, time: 900 } });
        await result.controller.requestPersistence('save');
        assert.equal(result.persistCalls, 0);
        const standalone = makeController({
            initialRecord: { result: false, time: 900 }, standalone: true,
        });
        await standalone.events.emit('persisted');
        assert.equal(standalone.persistCalls, 1);
    });
});