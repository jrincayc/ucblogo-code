const { ClientFunction } = require('testcafe');
const {
    ready,
    runLogo,
    expectOutput,
    isolateSettings,
    restoreSettings,
} = require('./testcafe-helpers');

const installMocks = ClientFunction(() => {
    const state = { originals: [], spoken: [], selectedVoiceName: null };
    window.__bridgeTest = state;
    const replace = (target, key, value) => {
        state.originals.push({ target, key, descriptor: Object.getOwnPropertyDescriptor(target, key) });
        Object.defineProperty(target, key, { configurable: true, writable: true, value });
    };
    state.speechSupported = Boolean(window.speechSynthesis && Module.utterance &&
        typeof Module.setSpeechVoice === 'function');
    if (window.speechSynthesis) {
        replace(window.speechSynthesis, 'getVoices', () => []);
        replace(window.speechSynthesis, 'speak', (utterance) => {
            state.spoken.push({
                text: utterance.text,
                lang: utterance.lang,
                pitch: utterance.pitch,
                rate: utterance.rate,
                voice: utterance.voice && utterance.voice.name,
            });
        });
        replace(window.speechSynthesis, 'cancel', () => {});
        replace(window.speechSynthesis, 'pause', () => {});
        replace(window.speechSynthesis, 'resume', () => {});
    }
    if (Module.utterance) {
        state.utterance = Module.utterance;
        state.utteranceValues = {
            text: Module.utterance.text,
            lang: Module.utterance.lang,
            pitch: Module.utterance.pitch,
            rate: Module.utterance.rate,
            voice: Module.utterance.voice,
        };
        Module.utterance.voice = null;
    }
    replace(Module, 'setSpeechVoice', (name) => {
        state.selectedVoiceName = name;
        return true;
    });
    const MockAudioContext = function () {
        this.currentTime = 0;
        this.destination = {};
        this.createOscillator = () => ({
            type: 'sine',
            frequency: { value: 0 },
            connect: () => {},
            start: () => {},
            stop: () => {},
            disconnect: () => {},
        });
        this.createGain = () => ({
            gain: { setValueAtTime: () => {}, linearRampToValueAtTime: () => {} },
            connect: () => {},
            disconnect: () => {},
        });
        this.close = () => Promise.resolve();
        this.resume = () => Promise.resolve();
    };
    replace(window, 'AudioContext', MockAudioContext);
    replace(window, 'webkitAudioContext', MockAudioContext);
    replace(window, '__runjsInjected', undefined);
});

const restoreMocks = ClientFunction(() => {
    const state = window.__bridgeTest;
    if (!state) return;
    if (state.utterance) {
        for (const key of Object.keys(state.utteranceValues))
            state.utterance[key] = state.utteranceValues[key];
    }
    for (let index = state.originals.length - 1; index >= 0; index--) {
        const original = state.originals[index];
        if (original.descriptor)
            Object.defineProperty(original.target, original.key, original.descriptor);
        else delete original.target[original.key];
    }
    delete window.__bridgeTest;
});

const directRunjs = ClientFunction(() =>
    Module.runjs('arg0 + ":" + arg1', 'bound value', 42));
const readSprite = ClientFunction(() => ({
    exists: Boolean(document.getElementById('logoSpriterunjsSpriteAcceptance')),
    turtleHref: document.getElementById('logoTurtleUse').getAttribute('href'),
}));
const speechSupported = ClientFunction(() => window.__bridgeTest.speechSupported);
const hasBoundSpeech = ClientFunction(() => {
    for (const call of window.__bridgeTest.spoken) {
        if (call.text === 'bound speech phrase') return true;
    }
    return false;
});
const readSpeech = ClientFunction(() => {
    const state = window.__bridgeTest;
    return {
        lang: Module.utterance.lang,
        pitch: Module.utterance.pitch,
        rate: Module.utterance.rate,
        voice: state.selectedVoiceName,
        spoken: state.spoken[state.spoken.length - 1],
    };
});
const mockVoices = ClientFunction(() => {
    if (!window.speechSynthesis) return false;
    window.speechSynthesis.getVoices = () => [
        { name: 'GetVoicesAcceptance', lang: 'en-US' },
    ];
    return true;
});
const clearInjection = ClientFunction(() => { delete window.__runjsInjected; });
const readInjection = ClientFunction(() =>
    window.__runjsInjected === undefined ? null : window.__runjsInjected);

fixture('Logo JavaScript bridge')
    .page(process.env.TESTCAFE_APP_URL || 'http://127.0.0.1:8085/ucblogo.html')
    .beforeEach(async (t) => {
        await ready(t);
        await isolateSettings(t);
        await installMocks();
    })
    .afterEach(async (t) => {
        try {
            await restoreMocks();
        } finally {
            await restoreSettings(t);
        }
    });

test('ACC-BRIDGE-001 binds variadic runjs parameters as values', async (t) => {
    await t.expect(directRunjs()).eql('bound value:42');
});

test('ACC-BRIDGE-002 binds variadic runjs parameters from Logo and preserves the writer', async (t) => {
    await runLogo(t, 'make "writerBeforeRunjs writer');
    await runLogo(t,
        `print (runjs "|arg0 + ':' + arg1 + ':' + arg2| "|bound value| 42 0.5)`);
    await expectOutput(t, 'bound value:42:0.5');
    await runLogo(t, 'print equal? :writerBeforeRunjs writer');
    await expectOutput(t, 'true');
});

test('ACC-BRIDGE-003 binds a large ordered parameter list from Logo', async (t) => {
    const parameterCount = 128;
    const expression = Array.from({ length: parameterCount }, (_, index) =>
        `${index + 1} * arg${index}`).join(' + ');
    const parameters = Array.from({ length: parameterCount }, (_, index) => index + 1);
    const expected = parameters.reduce((sum, value) => sum + value * value, 0);
    await runLogo(t, `print (runjs "|${expression}| ${parameters.join(' ')})`);
    await expectOutput(t, String(expected));
});

test('ACC-BRIDGE-004 passes sprite names to make and set turtle commands as bound values', async (t) => {
    await runLogo(t, 'makesprite "runjsSpriteAcceptance');
    await runLogo(t, 'setturtlesprite "runjsSpriteAcceptance');
    await t.expect(readSprite()).eql({
        exists: true,
        turtleHref: '#logoSpriterunjsSpriteAcceptance',
    });
});

test('ACC-BRIDGE-005 passes speech configuration and speak text as bound values', async (t) => {
    await t.expect(speechSupported()).eql(true, 'browser speech synthesis support');
    await runLogo(t, 'setspeechlang "en-US');
    await runLogo(t, 'setspeechpitch 1.25');
    await runLogo(t, 'setspeechrate 0.75');
    await runLogo(t, 'setspeechvoice "BoundVoiceAcceptance');
    await runLogo(t, 'speak "|bound speech phrase|');
    await t.expect(hasBoundSpeech()).ok('Logo speak did not pass its utterance as a bound value', {
        timeout: 10000,
    });
    await t.expect(readSpeech()).eql({
        lang: 'en-US',
        pitch: 1.25,
        rate: 0.75,
        voice: 'BoundVoiceAcceptance',
        spoken: {
            text: 'bound speech phrase',
            lang: 'en-US',
            pitch: 1.25,
            rate: 0.75,
            voice: null,
        },
    });
});

test('ACC-BRIDGE-006 parses getvoices results through the Logo-to-JS bridge', async (t) => {
    await t.expect(mockVoices()).eql(true, 'browser speech synthesis support');
    await runLogo(t, 'print first first getvoices');
    await expectOutput(t, 'GetVoicesAcceptance');
});

test('ACC-BRIDGE-007 keeps hostile bound strings as data across a fuzz corpus and 9000 characters', async (t) => {
    const corpus = [
        `';window.__runjsInjected=true;//`,
        `line one\nRUNJS_END\nwindow.__runjsInjected=true`,
        `quotes " ' and slash \\`,
        `nul\0byte`,
        `tab\treturn\rline`,
    ];
    const expression = `arg0.length + ':' + arg0.includes('RUNJS_END') + ':' + arg0.includes(String.fromCharCode(10))`;
    for (const payload of corpus) {
        await clearInjection();
        const logoValue = `(word ${Array.from(payload, (character) =>
            `char ${character.charCodeAt(0)}`).join(' ')})`;
        await runLogo(t, `print (runjs "|${expression}| ${logoValue})`);
        const expected = `${payload.length}:${payload.includes('RUNJS_END')}:${payload.includes('\n')}`;
        await expectOutput(t, expected);
        await t.expect(readInjection()).eql(null);
    }
    await runLogo(t, 'make "runjsLongWord "');
    await runLogo(t, 'repeat 9000 [make "runjsLongWord word :runjsLongWord "x]');
    await runLogo(t, 'print (runjs "|arg0.length| :runjsLongWord)');
    await expectOutput(t, '9000');
});

test('ACC-BRIDGE-008 runs multiline JavaScript from a v-barred Logo word', async (t) => {
    await runLogo(t, 'load "/tests/runjs-multiline.lg');
    await expectOutput(t, '42');
});