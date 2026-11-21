'use strict';

function createStorageStatusController({
    storageApi,
    events,
    getState,
    getPersistenceRecord,
    setPersistenceRecord,
    onRequestComplete = () => {},
    isStandalone = () => false,
    now = Date.now,
    cooldownMs = 30 * 24 * 60 * 60 * 1000,
}) {
    let requestAttempted = false;
    let installedRetried = false;
    let standaloneRetried = false;

    async function status() {
        const result = {
            api: Boolean(storageApi),
            persisted: null,
            usage: null,
            quota: null,
            ...getState(),
        };
        if (!storageApi) return result;

        if (typeof storageApi.persisted === 'function') {
            try {
                result.persisted = await storageApi.persisted();
            } catch (error) {
                result.persisted = null;
            }
        }
        if (typeof storageApi.estimate === 'function') {
            try {
                const estimate = await storageApi.estimate();
                result.usage = Number.isFinite(estimate?.usage) ? estimate.usage : null;
                result.quota = Number.isFinite(estimate?.quota) ? estimate.quota : null;
            } catch (error) {
                result.usage = null;
                result.quota = null;
            }
        }
        return result;
    }

    async function requestPersistence(reason, explicit = false) {
        if (!storageApi || typeof storageApi.persist !== 'function') return null;
        const standalone = isStandalone();
        if (!explicit) {
            if (reason === 'appinstalled') {
                if (installedRetried) return null;
                installedRetried = true;
            } else if (standalone) {
                if (standaloneRetried) return null;
                standaloneRetried = true;
            } else if (requestAttempted) {
                return null;
            }
        }

        const current = await status();
        if (current.persisted !== false) return current.persisted;
        const record = getPersistenceRecord();
        const deniedRecently = record.result === false &&
            now() - record.time < cooldownMs;
        if (!explicit && deniedRecently && reason !== 'appinstalled' && !standalone)
            return false;

        requestAttempted = true;
        try {
            const granted = await storageApi.persist();
            setPersistenceRecord({ time: now(), result: granted });
            return granted;
        } catch (error) {
            setPersistenceRecord({ time: now(), result: false });
            return false;
        }
    }

    if (events && typeof events.addEventListener === 'function') {
        events.addEventListener('persisted', () => {
            return requestPersistence('save').then(onRequestComplete, onRequestComplete);
        });
        events.addEventListener('appinstalled', () => {
            return requestPersistence('appinstalled').then(onRequestComplete, onRequestComplete);
        });
    }

    return { status, requestPersistence };
}

if (typeof module !== 'undefined' && module.exports)
    module.exports = { createStorageStatusController };
if (typeof globalThis !== 'undefined')
    globalThis.createStorageStatusController = createStorageStatusController;