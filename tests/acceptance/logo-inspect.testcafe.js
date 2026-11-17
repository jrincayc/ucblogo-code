// Co-authored by GitHub Copilot.
// Task wall-clock through 2026-11-17T17:45:00Z: 1h 56m 14s. Energy (kWh) and CO2: unavailable.
const { ClientFunction } = require('testcafe');
const {
    applicationUrl,
    ready,
    runLogo,
    commandInput,
    idle,
    expectOutput,
} = require('./testcafe-helpers');

const inspectClient = ClientFunction((method, args) =>
    JSON.stringify(Module.logoInspect[method].apply(Module.logoInspect, args)));
const readInjected = ClientFunction(() => window.__i);
const clearInjected = ClientFunction(() => { window.__i = undefined; });
const readHeapLength = ClientFunction(() => window.__logoInspectHeapLength);
const blockedOnGetc = ClientFunction(() => Boolean(Module.blockedOnGetc));

const recordHeapLength = `ignore runjs "|window.__logoInspectHeapLength = HEAPU8.length|`;

async function assertJsonRoundTrip(t, result, label = 'inspection result') {
    await t.expect(JSON.parse(JSON.stringify(result))).eql(result, `${label} must be JSON-safe`);
}

async function inspect(t, method, args = []) {
    const result = JSON.parse(await inspectClient(method, args));
    await assertJsonRoundTrip(t, result, `${method} result`);
    return result;
}

async function expectSuccess(t, result) {
    await t.expect(result.ok).eql(true, JSON.stringify(result.error));
    await t.expect(result.schema).eql(1);
}

function payload(result) {
    return result && result.value && typeof result.value === 'object' ? result.value : result;
}

function valueShape(value) {
    if (value.t === 'word') return { t: value.t, s: value.s };
    if (value.t === 'int' || value.t === 'float') return { t: value.t, v: value.v };
    if (value.t === 'list') return { t: value.t, items: value.items.map(valueShape) };
    if (value.t === 'array') {
        return { t: value.t, origin: value.origin, items: value.items.map(valueShape) };
    }
    if (value.t === 'ref') return { t: value.t };
    if (value.t === 'truncated') return { t: value.t, reason: value.reason };
    return { t: value.t };
}

function findValue(value, predicate, limit = 1000) {
    const pending = [value];
    const seen = new Set();
    let visited = 0;

    while (pending.length > 0) {
        const current = pending.pop();
        if (current === null || typeof current !== 'object' || seen.has(current)) continue;
        seen.add(current);
        if (++visited > limit) throw new Error(`Value walk exceeded ${limit} objects`);
        if (predicate(current)) return current;
        if (current.t === 'ref' || current.t === 'truncated') continue;
        for (const child of Object.values(current)) {
            if (child !== null && typeof child === 'object') pending.push(child);
        }
    }

    return null;
}

function findRole(root, role, limit = 1000) {
    const pending = [root];
    const seen = new Set();
    let visited = 0;

    while (pending.length > 0) {
        const current = pending.pop();
        if (current === null || typeof current !== 'object' || seen.has(current)) continue;
        seen.add(current);
        if (++visited > limit) throw new Error(`Tree walk exceeded ${limit} objects`);
        if (current.role === role) return current;
        if (Object.prototype.hasOwnProperty.call(current, 'ref') ||
            Object.prototype.hasOwnProperty.call(current, 'truncated')) continue;
        for (const child of Object.values(current)) {
            if (child !== null && typeof child === 'object') pending.push(child);
        }
    }

    return null;
}

function isRawNode(value) {
    return value !== null && typeof value === 'object' && Number.isSafeInteger(value.addr) &&
        value.node_type !== null && typeof value.node_type === 'object' &&
        Array.isArray(value.words) && typeof value.bytes === 'string';
}

function collectRawNodes(root, limit = 1000) {
    const pending = [root];
    const seen = new Set();
    const nodes = new Map();
    let visited = 0;

    while (pending.length > 0) {
        const current = pending.pop();
        if (current === null || typeof current !== 'object' || seen.has(current)) continue;
        seen.add(current);
        if (++visited > limit) throw new Error(`Tree walk exceeded ${limit} objects`);
        if (Object.prototype.hasOwnProperty.call(current, 'ref') ||
            Object.prototype.hasOwnProperty.call(current, 'truncated')) continue;
        if (isRawNode(current)) nodes.set(current.addr, current);
        for (const child of Object.values(current)) {
            if (child !== null && typeof child === 'object') pending.push(child);
        }
    }

    return nodes;
}

function rawNodeFingerprint(node) {
    return {
        addr: node.addr,
        segment: node.segment,
        node_type: node.node_type,
        id: node.id,
        my_gen: node.my_gen,
        gen_age: node.gen_age,
        mark_gc: node.mark_gc,
        words: node.words,
        bytes: node.bytes,
        union_view: node.union.view,
    };
}

fixture('Logo inspector').page(applicationUrl).disablePageCaching
    .beforeEach(async (t) => {
        await ready(t);
    });

test('ACC-LOGO-INSPECT-001 reports a valid idle status', async (t) => {
    const result = await inspect(t, 'status');
    const status = payload(result);

    await expectSuccess(t, result);
    await t.expect(status.idle).eql(true);
    await t.expect(status.layout.ok).eql(true);
    await t.expect(status.safe.logoGil === 0 || status.safe.logoGil === false).eql(true);
    await t.expect(status.safe.asyncifyState).eql(0);
    await t.expect(status.safe.insideGc === 0 || status.safe.insideGc === false).eql(true);
});

test('ACC-LOGO-INSPECT-002 reads an integer global variable', async (t) => {
    await runLogo(t, 'make "n 42');
    const result = await inspect(t, 'variable', ['n']);
    const value = payload(result);

    await expectSuccess(t, result);
    await t.expect({ t: value.t, v: value.v }).eql({ t: 'int', v: 42 });
});

test('ACC-LOGO-INSPECT-003 reads a float global variable', async (t) => {
    await runLogo(t, 'make "f 1.5');
    const result = await inspect(t, 'variable', ['f']);
    const value = payload(result);

    await expectSuccess(t, result);
    await t.expect({ t: value.t, v: value.v }).eql({ t: 'float', v: 1.5 });
});

test('ACC-LOGO-INSPECT-004 returns plain and v-barred variable words', async (t) => {
    await runLogo(t, 'make "w "hello  make "v "|a b|');
    const plainResult = await inspect(t, 'variable', ['w']);
    const barredResult = await inspect(t, 'variable', ['v']);
    const plain = payload(plainResult);
    const barred = payload(barredResult);

    await expectSuccess(t, plainResult);
    await expectSuccess(t, barredResult);
    await t.expect({ t: plain.t, s: plain.s }).eql({ t: 'word', s: 'hello' });
    await t.expect({ t: barred.t, s: barred.s }).eql({ t: 'word', s: 'a b' });
});

test('ACC-LOGO-INSPECT-005 returns nested and empty lists', async (t) => {
    await runLogo(t, 'make "l [a [b c] 3]  make "e []');
    const listResult = await inspect(t, 'variable', ['l']);
    const emptyResult = await inspect(t, 'variable', ['e']);
    const list = payload(listResult);
    const empty = payload(emptyResult);

    await expectSuccess(t, listResult);
    await expectSuccess(t, emptyResult);
    await t.expect(valueShape(list)).eql({
        t: 'list',
        items: [
            { t: 'word', s: 'a' },
            { t: 'list', items: [{ t: 'word', s: 'b' }, { t: 'word', s: 'c' }] },
            { t: 'word', s: '3' },
        ],
    });
    await t.expect(list.text).eql('[a [b c] 3]');
    await t.expect(valueShape(empty)).eql({ t: 'list', items: [] });
});

test('ACC-LOGO-INSPECT-006 returns a zero-origin array', async (t) => {
    await runLogo(t, 'make "a {1 2 3}@0');
    const result = await inspect(t, 'variable', ['a']);
    const value = payload(result);

    await expectSuccess(t, result);
    await t.expect(valueShape(value)).eql({
        t: 'array',
        origin: 0,
        items: [{ t: 'word', s: '1' }, { t: 'word', s: '2' }, { t: 'word', s: '3' }],
    });
});

test('ACC-LOGO-INSPECT-007 returns a reference for a cyclic list', async (t) => {
    await runLogo(t, 'make "c [x y]  .setbf :c :c');
    const result = await inspect(t, 'variable', ['c']);
    const value = payload(result);

    await expectSuccess(t, result);
    await t.expect(value.t).eql('list');
    await t.expect(findValue(value, (item) => item.t === 'ref')?.t).eql('ref');
});

test('ACC-LOGO-INSPECT-008 respects the depth limit', async (t) => {
    await runLogo(t, 'make "d [[[[[[[[[[1]]]]]]]]]]');
    const result = await inspect(t, 'value', ['d', { depth: 3 }]);
    let value = payload(result);

    await expectSuccess(t, result);
    for (let level = 0; level < 3; level += 1) {
        await t.expect(value.t).eql('list', `depth ${level} should be a list`);
        value = value.items[0];
    }
    await t.expect(value).eql({ t: 'truncated', reason: 'depth' });
});

test('ACC-LOGO-INSPECT-009 reports user procedure metadata', async (t) => {
    await runLogo(t, 'define "sq [[x] [output :x * :x]]');
    const result = await inspect(t, 'procedure', ['sq']);
    const procedure = payload(result);

    await expectSuccess(t, result);
    await t.expect(procedure.formals).eql(['x']);
    await t.expect(procedure.arity).eql({ min: 1, default: 1, max: 1 });
    await t.expect(procedure.body).eql(['output :x * :x']);
});

test('ACC-LOGO-INSPECT-010 decodes optional procedure arity', async (t) => {
    await runLogo(t, 'define "opt [[a [b 2] [c] 1]]');
    const result = await inspect(t, 'procedure', ['opt']);
    const procedure = payload(result);

    await expectSuccess(t, result);
    await t.expect(procedure.arity).eql({ min: 1, default: 1, max: -1 });
});

test('ACC-LOGO-INSPECT-011 reports SUM as a primitive with arity 0/2/-1', async (t) => {
    const result = await inspect(t, 'procedure', ['sum']);
    const procedure = payload(result);

    await expectSuccess(t, result);
    await t.expect(procedure.kind).eql('primitive');
    await t.expect(procedure.arity).eql({ min: 0, default: 2, max: -1 });
});

test('ACC-LOGO-INSPECT-012 reads a property-list entry', async (t) => {
    await runLogo(t, 'pprop "p "k 7');
    const result = await inspect(t, 'plist', ['p']);
    const entries = payload(result);

    await expectSuccess(t, result);
    await t.expect(entries.map(({ prop, value }) => ({
        prop,
        value: { t: value.t, v: value.v },
    }))).eql([{ prop: 'k', value: { t: 'int', v: 7 } }]);
});

test('ACC-LOGO-INSPECT-013 lists buried procedures and globals in the workspace', async (t) => {
    await runLogo(t, 'make "n 42');
    await runLogo(t, 'pprop "p "k 7');
    await runLogo(t, 'define "sq [[x] [output :x * :x]]');
    await runLogo(t, 'bury "sq');
    const result = await inspect(t, 'workspace');
    const workspace = payload(result);

    await expectSuccess(t, result);
    const sq = workspace.procedures.find((procedure) => procedure.name === 'sq');
    await t.expect(Boolean(sq)).eql(true);
    await t.expect(sq.flags.buried).eql(true);
    await t.expect(workspace.variables.some((variable) => variable.name === 'n')).eql(true);
    await t.expect(workspace.propertyLists.some((plist) => plist.name === 'p')).eql(true);
});

test('ACC-LOGO-INSPECT-014 returns coded errors for invalid inputs without throwing', async (t) => {
    const invalidCalls = [
        ['variable', ['nosuch']],
        ['node', [0]],
        ['node', [1]],
        ['node', [2 ** 32]],
    ];

    for (const [method, args] of invalidCalls) {
        const result = await inspect(t, method, args);
        await t.expect(result.ok).eql(false, `${method} should return ok:false`);
        await t.expect(result.schema).eql(1, `${method} should return schema 1`);
        await t.expect(typeof result.error?.code).eql('string');
        await t.expect(result.error.code.length > 0).eql(true);
    }
});

test('ACC-LOGO-INSPECT-015 inspects state while READWORD is waiting', async (t) => {
    await runLogo(t, 'make "n 42');
    await t.expect(idle()).ok('Logo must be idle before submitting READWORD', { timeout: 30000 });
    await t.typeText(commandInput, 'make "r readword', { paste: true }).pressKey('enter');
    await t.expect(blockedOnGetc()).ok('READWORD should wait for input', { timeout: 30000 });

    const nResult = await inspect(t, 'variable', ['n']);
    const statusResult = await inspect(t, 'status');
    const status = payload(statusResult);
    await expectSuccess(t, nResult);
    await expectSuccess(t, statusResult);
    await t.expect(status.idle).eql(false);

    const inputLine = 'inspect-ready-line';
    await t.typeText(commandInput, inputLine, { paste: true }).pressKey('enter');
    await t.expect(idle()).ok('Logo should return to idle after READWORD', { timeout: 30000 });

    const rResult = await inspect(t, 'variable', ['r']);
    const value = payload(rResult);
    await expectSuccess(t, rResult);
    await t.expect({ t: value.t, s: value.s }).eql({ t: 'word', s: inputLine });
});

test('ACC-LOGO-INSPECT-016 runs from runjs and Logo continues', async (t) => {
    await runLogo(t, 'make "n 42');
    await clearInjected();
    await runLogo(t, `ignore runjs "|window.__i = JSON.stringify(Module.logoInspect.variable('n'))|`);

    const result = JSON.parse(await readInjected());
    await assertJsonRoundTrip(t, result, 'runjs inspection result');
    const value = payload(result);
    await expectSuccess(t, result);
    await t.expect({ t: value.t, v: value.v }).eql({ t: 'int', v: 42 });

    await runLogo(t, 'print "inspector-case16-continues');
    await expectOutput(t, 'inspector-case16-continues');
});

test('ACC-LOGO-INSPECT-017 preserves values after garbage collection', async (t) => {
    const setup = [
        'make "n 42',
        'make "f 1.5',
        'make "w "hello',
        'make "v "|a b|',
        'make "l [a [b c] 3]',
        'make "e []',
        'make "a {1 2 3}@0',
    ];
    for (const command of setup) await runLogo(t, command);
    await runLogo(t, 'gc');

    const inspected = {};
    for (const name of ['n', 'f', 'w', 'v', 'l', 'e', 'a']) {
        inspected[name] = await inspect(t, 'variable', [name]);
        await expectSuccess(t, inspected[name]);
    }

    await t.expect({
        n: valueShape(payload(inspected.n)),
        f: valueShape(payload(inspected.f)),
        w: valueShape(payload(inspected.w)),
        v: valueShape(payload(inspected.v)),
        l: valueShape(payload(inspected.l)),
        e: valueShape(payload(inspected.e)),
        a: valueShape(payload(inspected.a)),
        listText: payload(inspected.l).text,
    }).eql({
        n: { t: 'int', v: 42 },
        f: { t: 'float', v: 1.5 },
        w: { t: 'word', s: 'hello' },
        v: { t: 'word', s: 'a b' },
        l: {
            t: 'list',
            items: [
                { t: 'word', s: 'a' },
                { t: 'list', items: [{ t: 'word', s: 'b' }, { t: 'word', s: 'c' }] },
                { t: 'word', s: '3' },
            ],
        },
        e: { t: 'list', items: [] },
        a: {
            t: 'array',
            origin: 0,
            items: [{ t: 'word', s: '1' }, { t: 'word', s: '2' }, { t: 'word', s: '3' }],
        },
        listText: '[a [b c] 3]',
    });
});

test('ACC-LOGO-INSPECT-018 survives linear-memory growth', async (t) => {
    await runLogo(t, 'make "n 42');
    await runLogo(t, recordHeapLength);
    const initialHeapLength = await readHeapLength();
    await runLogo(t, 'make "growthlist []');

    const chunkSize = 25000;
    const maxListCells = 1000000;
    let allocatedListCells = 0;
    let currentHeapLength = initialHeapLength;
    while (currentHeapLength === initialHeapLength && allocatedListCells < maxListCells) {
        const cellsThisChunk = Math.min(chunkSize, maxListCells - allocatedListCells);
        await runLogo(t, `repeat ${cellsThisChunk} [make "growthlist fput "x :growthlist]`);
        allocatedListCells += cellsThisChunk;
        await runLogo(t, recordHeapLength);
        currentHeapLength = await readHeapLength();
    }

    await t.expect(currentHeapLength).gt(
        initialHeapLength,
        `HEAPU8.length did not grow after ${allocatedListCells} retained list cells`,
    );
    const result = await inspect(t, 'variable', ['n']);
    const value = payload(result);
    await expectSuccess(t, result);
    await t.expect({ t: value.t, v: value.v }).eql({ t: 'int', v: 42 });
});

test('ACC-LOGO-INSPECT-019 decodes a raw integer NODE', async (t) => {
    await runLogo(t, 'make "n 42');
    const valueResult = await inspect(t, 'variable', ['n']);
    const value = payload(valueResult);
    await expectSuccess(t, valueResult);
    await t.expect({ t: value.t, v: value.v }).eql({ t: 'int', v: 42 });

    const nodeResult = await inspect(t, 'node', [value.addr]);
    const node = payload(nodeResult);
    await expectSuccess(t, nodeResult);
    await t.expect({
        value: node.node_type.value,
        octal: node.node_type.octal,
        name: node.node_type.name,
    }).eql({ value: 768, octal: '01400', name: 'INTT' });
    await t.expect(node.union.nint).eql(42);

    const nodeSize = 40;
    await t.expect(node.words.length).eql(nodeSize / 4);
    await t.expect(typeof node.bytes).eql('string');
    await t.expect(node.bytes.length).eql(nodeSize * 2);
    await t.expect(node.words.length).eql(node.bytes.length / 8);
});

test('ACC-LOGO-INSPECT-020 rejects a misaligned NODE address', async (t) => {
    await runLogo(t, 'make "n 42');
    const valueResult = await inspect(t, 'variable', ['n']);
    const value = payload(valueResult);
    await expectSuccess(t, valueResult);

    const result = await inspect(t, 'node', [value.addr + 4]);
    await t.expect(result.ok).eql(false);
    await t.expect(result.error.code).eql('misaligned');
});

test('ACC-LOGO-INSPECT-021 expands object and nested-list NODE roles', async (t) => {
    await runLogo(t, 'make "l [a [b c] 3]');
    const result = await inspect(t, 'tree', ['l']);
    const root = payload(result);
    await expectSuccess(t, result);
    await t.expect(root.node_type.name).eql('CONS');

    const canonical = findRole(root, 'canonical') || root.canonical;
    await t.expect(canonical.node_type.name).eql('STRING');
    await t.expect(canonical.union.text).eql('l');

    const first = findRole(root, 'valnode') || root.valnode;
    const ncar = (node) => node.union.ncar;
    const ncdr = (node) => node.union.ncdr;
    const second = ncdr(first);
    const third = ncdr(second);
    await t.expect(first.node_type.name).eql('CONS');
    await t.expect(second.node_type.name).eql('CONS');
    await t.expect(third.node_type.name).eql('CONS');
    await t.expect(ncdr(third)).eql(0);

    const nestedFirst = ncar(second);
    const nestedSecond = ncdr(nestedFirst);
    await t.expect(nestedFirst.node_type.name).eql('CONS');
    await t.expect(nestedSecond.node_type.name).eql('CONS');
    await t.expect(ncdr(nestedSecond)).eql(0);

    const a = ncar(first);
    const b = ncar(nestedFirst);
    const c = ncar(nestedSecond);
    const three = ncar(third);
    await t.expect(a.union.text).eql('a');
    await t.expect(b.union.text).eql('b');
    await t.expect(c.union.text).eql('c');
    await t.expect(three.node_type.name).eql('STRING');
    await t.expect(three.union.text).eql('3');
});

test('ACC-LOGO-INSPECT-022 terminates cyclic NODE trees with a ref', async (t) => {
    await runLogo(t, 'make "c [x y]  .setbf :c :c');
    const result = await inspect(t, 'tree', ['c']);
    const root = payload(result);
    await expectSuccess(t, result);

    const firstCell = findRole(root, 'valnode') || root.valnode;
    await t.expect(Number.isSafeInteger(firstCell.addr)).eql(true);
    const cycleReference = findValue(root, (value) => value.ref === firstCell.addr);
    await t.expect(Boolean(cycleReference)).eql(true);
});

test('ACC-LOGO-INSPECT-023 includes procedure-node roles', async (t) => {
    await runLogo(t, 'define "sq [[x] [output :x * :x]]');
    const result = await inspect(t, 'tree', ['sq']);
    const root = payload(result);
    await expectSuccess(t, result);

    const procedureNode = findRole(root, 'procnode');
    await t.expect(Boolean(procedureNode)).eql(true);
    await t.expect(Boolean(findRole(procedureNode, 'formals'))).eql(true);
    const minargs = findRole(procedureNode, 'minargs');
    await t.expect(Boolean(minargs)).eql(true);
    await t.expect(minargs.node_type.name).eql('INTT');
    await t.expect(minargs.union.nint).eql(1);
});

test('ACC-LOGO-INSPECT-024 matches tree NODEs to node(addr)', async (t) => {
    await runLogo(t, 'make "l [a [b c] 3]');
    const treeResult = await inspect(t, 'tree', ['l']);
    const tree = payload(treeResult);
    await expectSuccess(t, treeResult);
    const nodes = collectRawNodes(tree);
    await t.expect(nodes.size > 0).eql(true);

    for (const [address, treeNode] of nodes) {
        const nodeResult = await inspect(t, 'node', [address]);
        const node = payload(nodeResult);
        await expectSuccess(t, nodeResult);
        await t.expect(rawNodeFingerprint(treeNode)).eql(
            rawNodeFingerprint(node), `node(${address}) should match its tree NODE`,
        );
    }
});

test('ACC-LOGO-INSPECT-025 decodes escaped spaces from NODE bytes', async (t) => {
    await runLogo(t, 'make "s "a\\ b');
    const result = await inspect(t, 'tree', ['s']);
    const root = payload(result);
    await expectSuccess(t, result);

    const valueNode = findRole(root, 'valnode') || root.valnode;
    const stringNode = valueNode.union.ncar;
    await t.expect(valueNode.node_type.name).eql('CASEOBJ');
    await t.expect(stringNode.node_type.name).eql('BACKSLASH_STRING');
    await t.expect(stringNode.union.text).eql('a b');
    await t.expect(stringNode.union.strbytes).eql('612062');
});