// Co-authored by GitHub Copilot. Scope: Phase 1 Logo inspector implementation.
// Model: GPT-6 Luna; reasoning setting unavailable (maximum effort requested).
// Task wall-clock through 2026-11-17T18:32:52Z: 21m 02s.
// Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available.
'use strict';

const LAYOUT = Object.freeze({
    pointerSize: 4,
    fixnumSize: 4,
    hashLength: 1021,
    memoryViews: {
        HEAPU8: 1,
        HEAP16: 2,
        HEAP32: 4,
        HEAPU32: 4,
        HEAPF64: 8,
    },
    node: {
        size: 40,
        alignment: 8,
        fields: {
            nodeType: 0,
            myGen: 4,
            genAge: 8,
            markGc: 12,
            next: 16,
            oldyoungNext: 20,
            union: 24,
        },
        flags: {
            NT_HTML: 0o200000000,
            NT_METHOD: 0o100000000,
            NT_OBJ: 0o40000000,
            NT_LOCAL: 0o20000000,
            NT_STACK: 0o10000000,
            NT_CONT: 0o4000000,
            NT_INFIX: 0o2000000,
            NT_LINE: 0o1000000,
            NT_TAILFORM: 0o400000,
            NT_MACRO: 0o200000,
            NT_TREE: 0o100000,
            NT_EMPTY: 0o40000,
            NT_AGGR: 0o20000,
            NT_LIST: 0o10000,
            NT_RUNP: 0o4000,
            NT_ARRAY: 0o2000,
            NT_WORD: 0o1000,
            NT_NUMBER: 0o400,
            NT_FLOAT: 0o200,
            NT_PRIM: 0o100,
            NT_VBAR: 0o40,
            NT_STRING: 0o20,
            NT_BACKSL: 0o10,
            NT_PUNCT: 0o4,
            NT_COLON: 0o2,
            NT_CASEOBJ: 0o1,
        },
        types: {
            PUNBOUND: 0,
            PNIL: 0o70000,
            CONS: 0o30000,
            STRING: 0o1020,
            INTT: 0o1400,
            FLOATT: 0o1600,
            PRIM: 0o100,
            MACRO: 0o200100,
            TAILFORM: 0o400100,
            CASEOBJ: 0o1001,
            INFIX: 0o2000100,
            TREE: 0o130000,
            RUN_PARSE: 0o34000,
            QUOTE: 0o1004,
            COLON: 0o1006,
            BACKSLASH_STRING: 0o1030,
            VBAR_STRING: 0o1070,
            HTML_STRING: 0o200001070,
            ARRAY: 0o22000,
            LINE: 0o1030000,
            CONT: 0o4010000,
            STACK: 0o10010000,
            LOCALSAVE: 0o20030000,
            OBJECT: 0o40000000,
            METHOD: 0o100030000,
            NTFREE: -1,
        },
        union: {
            ncons: { ncar: 0, ncdr: 4, nobj: 8 },
            nstring: { nstringPtr: 0, nstringHead: 4, nstringLen: 8 },
            nprim: { fun: 0, priority: 4, min: 6, dflt: 8, max: 10 },
            nint: { value: 0 },
            nfloat: { value: 0 },
            narray: { dim: 0, origin: 4, data: 8 },
            nobject: { vars: 0, procs: 4, parents: 8 },
            nmethod: { parent: 0, procname: 4 },
        },
    },
    segment: { next: 0, size: 4, nodes: 8 },
    stringBlock: { refcnt: 0 },
    registers: {
        r_proc: 0,
        r_argl: 4,
        r_unev: 8,
        r_fun: 12,
        r_ufun: 16,
        r_var: 20,
        r_vsp: 24,
        r_qm_list: 28,
        r_formals: 32,
        r_last_ufun: 36,
        r_this_line: 40,
        r_last_line: 44,
        r_current_unode: 48,
        r_didnt_output_name: 52,
        r_didnt_get_output: 56,
        r_val_status: 60,
        objects: {
            r_usual_parent: 60,
            r_usual_caller: 64,
            r_val_status: 68,
        },
    },
    object: {
        caseobj: { string: 0, object: 4 },
        cells: {
            canonical: 0,
            procnode: 1,
            valnode: 2,
            plist: 3,
            obflags: 4,
            caselist: 5,
        },
    },
});

function createLogoInspector({ memory, symbols, layout } = {}) {
    memory = memory || {};
    symbols = symbols || {};
    layout = layout || LAYOUT;

    let verifiedLayout = null;
    let cachedSpecialChars = null;

    function inspectorError(code, message) {
        const error = new Error(message);
        error.code = code;
        return error;
    }

    function fail(code, message) {
        throw inspectorError(code, message);
    }

    function errorResult(code, message) {
        return {
            ok: false,
            schema: 1,
            error: {
                code,
                message: typeof message === 'string' ? message : 'Inspection failed.',
            },
        };
    }

    function publicCall(callback) {
        try {
            return callback();
        } catch (error) {
            const code = error && typeof error.code === 'string'
                ? error.code : 'memory-error';
            const message = error && typeof error.message === 'string'
                ? error.message : 'Unable to read Logo memory.';
            return errorResult(code, message);
        }
    }

    function sameLayout(actual, expected) {
        if (typeof expected !== 'object' || expected === null)
            return actual === expected;
        if (typeof actual !== 'object' || actual === null)
            return false;
        const expectedKeys = Object.keys(expected);
        const actualKeys = Object.keys(actual);
        return expectedKeys.length === actualKeys.length &&
            expectedKeys.every((key) => sameLayout(actual[key], expected[key]));
    }

    function currentView(name) {
        const view = memory[name];
        const expectedElementSize = layout.memoryViews && layout.memoryViews[name];
        if (!view || typeof view.length !== 'number' ||
            view.BYTES_PER_ELEMENT !== expectedElementSize)
            fail('layout-mismatch', `Current ${name} memory view is unavailable.`);
        return view;
    }

    function heapByteLength() {
        return currentView('HEAPU8').length;
    }

    function checkRange(address, length, alignment) {
        const end = address + length;
        if (!Number.isInteger(address) || !Number.isInteger(length) || address < 0 ||
            length < 0 || (alignment && address % alignment !== 0) ||
            !Number.isSafeInteger(end) || end > heapByteLength())
            fail('memory-error', 'A memory field is outside the current Wasm heap.');
    }

    function readU8(address) {
        checkRange(address, 1, 1);
        return currentView('HEAPU8')[address];
    }

    function readI16(address) {
        checkRange(address, 2, 2);
        return currentView('HEAP16')[address / 2];
    }

    function readI32(address) {
        checkRange(address, 4, 4);
        return currentView('HEAP32')[address / 4];
    }

    function readU32(address) {
        checkRange(address, 4, 4);
        return currentView('HEAPU32')[address / 4];
    }

    function readF64(address) {
        checkRange(address, 8, 8);
        return currentView('HEAPF64')[address / 8];
    }

    function isWasmAddress(address) {
        return typeof address === 'number' && Number.isFinite(address) &&
            Number.isInteger(address) && address >= 0 && address < 0x100000000;
    }

    function symbolAddress(name) {
        let address = symbols[name];
        if (address === undefined) address = symbols[`_${name}`];
        if (!isWasmAddress(address) || address === 0)
            fail('layout-mismatch', `The ${name} data symbol is unavailable.`);
        return address;
    }

    function optionalSymbolAddress(name) {
        let address = symbols[name];
        if (address === undefined) address = symbols[`_${name}`];
        return isWasmAddress(address) && address !== 0 ? address : null;
    }

    function readGlobalIntOrNull(name) {
        const address = optionalSymbolAddress(name);
        if (address === null || address % 4 !== 0 || address + 4 > heapByteLength())
            return null;
        try {
            return readI32(address);
        } catch (error) {
            return null;
        }
    }

    function readGlobalPointer(name) {
        const address = symbolAddress(name);
        return readU32(address);
    }

    function getSegments() {
        const head = readGlobalPointer('segment_list');
        const segments = [];
        const seen = new Set();
        let address = head;
        const maximumSegments = 65536;

        while (address !== 0) {
            if (!isWasmAddress(address) || address % 8 !== 0 || seen.has(address) ||
                segments.length >= maximumSegments)
                fail('memory-error', 'The segment list is invalid or cyclic.');
            seen.add(address);
            checkRange(address, layout.segment.nodes, 4);
            const size = readI32(address + layout.segment.size);
            const nodesAddress = address + layout.segment.nodes;
            const bytes = size * layout.node.size;
            if (!Number.isSafeInteger(size) || size < 2 || !Number.isSafeInteger(bytes) ||
                nodesAddress % layout.node.alignment !== 0 ||
                bytes > heapByteLength() - nodesAddress)
                fail('memory-error', 'A segment size is inconsistent with the NODE stride.');
            segments.push({
                addr: address,
                nodesAddr: nodesAddress,
                size,
                end: nodesAddress + bytes,
            });
            address = readU32(address + layout.segment.next);
        }
        return segments;
    }

    function findNodeLocation(address, segments) {
        if (!isWasmAddress(address) || address === 0 ||
            address % layout.node.alignment !== 0)
            return null;
        for (const segment of segments) {
            if (address < segment.nodesAddr || address >= segment.end) continue;
            const offset = address - segment.nodesAddr;
            if (offset % layout.node.size !== 0) return { misaligned: true };
            const index = offset / layout.node.size;
            if (index >= segment.size) return null;
            return { segment, index };
        }
        return null;
    }

    function requireNodeLocation(address, segments, code) {
        const location = findNodeLocation(address, segments);
        if (!location || location.misaligned)
            fail(code, 'A Logo pointer does not identify an aligned NODE in a segment.');
        return location;
    }

    function nodeTypeAt(address, segments, code = 'memory-error') {
        requireNodeLocation(address, segments, code);
        return readI32(address + layout.node.fields.nodeType);
    }

    function typeName(value) {
        for (const [name, type] of Object.entries(layout.node.types)) {
            if (value === type) return name;
        }
        return null;
    }

    function isKnownType(value) {
        return typeName(value) !== null;
    }

    function nodeFlags(value) {
        return Object.entries(layout.node.flags)
            .filter(([, flag]) => (value & flag) === flag)
            .map(([name]) => name);
    }

    function octalType(value) {
        const digits = (value >>> 0).toString(8);
        return digits === '0' ? '0' : `0${digits}`;
    }

    function decodeUtf8(bytes) {
        if (typeof TextDecoder !== 'function')
            fail('layout-mismatch', 'TextDecoder is unavailable for Logo strings.');
        return new TextDecoder('utf-8').decode(bytes);
    }

    function readSpecialChars() {
        if (cachedSpecialChars !== null) return cachedSpecialChars;
        const address = symbolAddress('special_chars');
        const available = heapByteLength() - address;
        const limit = Math.min(128, available);
        if (limit <= 0) fail('layout-mismatch', 'The special_chars table is outside Wasm memory.');
        const view = currentView('HEAPU8');
        let length = 0;
        while (length < limit && view[address + length] !== 0) length += 1;
        if (length === limit)
            fail('layout-mismatch', 'The special_chars table is not terminated in Wasm memory.');
        cachedSpecialChars = decodeUtf8(view.subarray(address, address + length));
        return cachedSpecialChars;
    }

    function decodeLogoBytes(pointer, length) {
        if (!isWasmAddress(pointer) || !Number.isSafeInteger(length) || length < 0 ||
            pointer + length > heapByteLength())
            fail('memory-error', 'A Logo string has invalid bounds.');
        const renderedLength = Math.min(length, 65536);
        const bytes = currentView('HEAPU8').subarray(pointer, pointer + renderedLength);
        const decoded = decodeUtf8(bytes);
        const specialChars = readSpecialChars();
        let text = '';
        for (const character of decoded) {
            const code = character.codePointAt(0);
            const isUnmappedControl = code >= 7 && code <= 15 && code !== 11;
            if (code >= 3 && code < 3 + specialChars.length && !isUnmappedControl)
                text += specialChars.charAt(code - 3);
            else
                text += character;
        }
        let hex = '';
        for (const byte of bytes) hex += byte.toString(16).padStart(2, '0');
        return { text, strbytes: hex, truncated: renderedLength < length };
    }

    function readStringNode(address, segments, code = 'memory-error') {
        const nodeType = nodeTypeAt(address, segments, code);
        if ((nodeType & layout.node.flags.NT_STRING) === 0)
            fail(code, 'A CASEOBJ canonical name is not a string NODE.');
        const unionAddress = address + layout.node.fields.union;
        const pointer = readU32(unionAddress + layout.node.union.nstring.nstringPtr);
        const length = readI32(unionAddress + layout.node.union.nstring.nstringLen);
        const decoded = decodeLogoBytes(pointer, length);
        return { nodeType, pointer, length, ...decoded };
    }

    function verifyLayout() {
        if (!sameLayout(layout, LAYOUT))
            fail('layout-mismatch', 'The supplied layout table is not the supported wasm32 layout.');

        const byteLength = heapByteLength();
        if (currentView('HEAP16').length * 2 !== byteLength ||
            currentView('HEAP32').length * 4 !== byteLength ||
            currentView('HEAPU32').length * 4 !== byteLength ||
            currentView('HEAPF64').length * 8 > byteLength)
            fail('layout-mismatch', 'The current typed memory views do not match Wasm memory.');

        const segments = getSegments();
        if (segments.length === 0)
            fail('layout-mismatch', 'No NODE segment has been initialized.');
        const firstSegment = segments[0];
        const firstType = nodeTypeAt(firstSegment.nodesAddr, segments, 'layout-mismatch');
        const secondType = nodeTypeAt(
            firstSegment.nodesAddr + layout.node.size, segments, 'layout-mismatch',
        );
        if (!isKnownType(firstType) || !isKnownType(secondType))
            fail('layout-mismatch', 'NODE stride or type offsets do not match the supported layout.');

        const unboundAddress = readGlobalPointer('Unbound');
        const unboundType = nodeTypeAt(unboundAddress, segments, 'layout-mismatch');
        if (unboundType !== layout.node.types.PUNBOUND)
            fail('layout-mismatch', 'Unbound does not have the expected PUNBOUND node type.');

        const hashAddress = symbolAddress('hash_table');
        if (hashAddress % 4 !== 0 ||
            hashAddress + layout.hashLength * layout.pointerSize > byteLength)
            fail('layout-mismatch', 'The hash_table does not fit in Wasm memory.');

        const specialChars = readSpecialChars();
        let sampleName = null;
        for (let bucket = 0; bucket < layout.hashLength; bucket += 1) {
            let entry = readU32(hashAddress + bucket * layout.pointerSize);
            const visitedEntries = new Set();
            while (entry !== 0) {
                if (visitedEntries.has(entry))
                    fail('layout-mismatch', 'A hash bucket contains a cycle.');
                visitedEntries.add(entry);
                if (nodeTypeAt(entry, segments, 'layout-mismatch') !== layout.node.types.CONS)
                    fail('layout-mismatch', 'A nonempty hash bucket is not a CONS chain.');
                const object = readU32(
                    entry + layout.node.fields.union + layout.node.union.ncons.ncar,
                );
                if (nodeTypeAt(object, segments, 'layout-mismatch') !== layout.node.types.CONS)
                    fail('layout-mismatch', 'A hash entry is not an object cell.');
                const canonical = readU32(
                    object + layout.node.fields.union + layout.node.union.ncons.ncar,
                );
                const name = readStringNode(canonical, segments, 'layout-mismatch');
                if (name.text.length > 0) {
                    sampleName = name.text;
                    break;
                }
                entry = readU32(
                    entry + layout.node.fields.union + layout.node.union.ncons.ncdr,
                );
            }
            if (sampleName !== null) break;
        }
        if (specialChars.length === 0)
            fail('layout-mismatch', 'The special_chars table is empty.');
        if (sampleName !== null) return { ok: true, sampleName };
        fail('layout-mismatch', 'The hash_table has no object cell with a readable name.');
    }

    function ensureLayout() {
        if (verifiedLayout) return verifiedLayout;
        try {
            verifiedLayout = verifyLayout();
            return verifiedLayout;
        } catch (error) {
            const message = error && typeof error.message === 'string'
                ? error.message : 'The runtime memory layout could not be verified.';
            return { ok: false, message };
        }
    }

    function publicNode(address) {
        if (!isWasmAddress(address) || address === 0)
            fail('bad-address', 'NODE address must be a nonzero wasm32 address.');
        if (address % LAYOUT.node.alignment !== 0)
            fail('misaligned', 'NODE address must be 8-byte aligned.');

        const layoutCheck = ensureLayout();
        if (!layoutCheck.ok) return errorResult('layout-mismatch', layoutCheck.message);

        const segments = getSegments();
        const location = findNodeLocation(address, segments);
        if (!location) fail('bad-address', 'Address is outside all NODE segments.');
        if (location.misaligned)
            fail('misaligned', 'Address is not aligned to the NODE stride.');

        const fields = layout.node.fields;
        const unionLayout = layout.node.union;
        const nodeType = readI32(address + fields.nodeType);
        const unionAddress = address + fields.union;
        const typeNameValue = typeName(nodeType);
        let union = { view: 'unknown' };

        if (nodeType === layout.node.types.INTT) {
            union = { view: 'nint', nint: readI32(unionAddress + unionLayout.nint.value) };
        } else if (nodeType === layout.node.types.FLOATT) {
            const value = readF64(unionAddress + unionLayout.nfloat.value);
            union = Number.isFinite(value)
                ? { view: 'nfloat', nfloat: value }
                : {
                    view: 'nfloat',
                    nfloat: null,
                    repr: Number.isNaN(value) ? 'nan' : value < 0 ? '-inf' : 'inf',
                };
        } else if (nodeType === layout.node.types.NTFREE) {
            union = { view: 'unknown' };
        } else if (typeNameValue === 'OBJECT') {
            union = {
                view: 'nobject',
                nvars: readU32(unionAddress + unionLayout.nobject.vars),
                nprocs: readU32(unionAddress + unionLayout.nobject.procs),
                nparents: readU32(unionAddress + unionLayout.nobject.parents),
            };
        } else if (typeNameValue === 'METHOD') {
            union = {
                view: 'nmethod',
                nparent: readU32(unionAddress + unionLayout.nmethod.parent),
                nprocname: readU32(unionAddress + unionLayout.nmethod.procname),
            };
        } else if (nodeType === layout.node.types.ARRAY) {
            union = {
                view: 'narray',
                dim: readI32(unionAddress + unionLayout.narray.dim),
                origin: readI32(unionAddress + unionLayout.narray.origin),
                data: readU32(unionAddress + unionLayout.narray.data),
            };
        } else if ((nodeType & layout.node.flags.NT_STRING) !== 0) {
            const string = readStringNode(address, segments);
            const stringHead = readU32(unionAddress + unionLayout.nstring.nstringHead);
            let refcnt = null;
            if (stringHead !== 0) {
                checkRange(stringHead + layout.stringBlock.refcnt, 4, 4);
                refcnt = readU32(stringHead + layout.stringBlock.refcnt);
            }
            union = {
                view: 'nstring',
                nstring_ptr: string.pointer,
                nstring_head: stringHead,
                nstring_len: string.length,
                refcnt,
                text: string.text,
                strbytes: string.strbytes,
            };
            if (string.truncated) union.truncated = true;
        } else if (typeNameValue === 'PRIM' || typeNameValue === 'MACRO' ||
            typeNameValue === 'TAILFORM' || typeNameValue === 'INFIX') {
            union = {
                view: 'nprim',
                fun: readU32(unionAddress + unionLayout.nprim.fun),
                priority: readI16(unionAddress + unionLayout.nprim.priority),
                min: readI16(unionAddress + unionLayout.nprim.min),
                dflt: readI16(unionAddress + unionLayout.nprim.dflt),
                max: readI16(unionAddress + unionLayout.nprim.max),
            };
        } else if (typeNameValue === 'CONS' || typeNameValue === 'PNIL' ||
            typeNameValue === 'CASEOBJ' || typeNameValue === 'TREE' ||
            typeNameValue === 'RUN_PARSE' || typeNameValue === 'LINE' ||
            typeNameValue === 'CONT' || typeNameValue === 'STACK' ||
            typeNameValue === 'LOCALSAVE') {
            union = {
                view: 'ncons',
                ncar: readU32(unionAddress + unionLayout.ncons.ncar),
                ncdr: readU32(unionAddress + unionLayout.ncons.ncdr),
                nobj: readU32(unionAddress + unionLayout.ncons.nobj),
            };
        }

        const byteView = currentView('HEAPU8');
        const wordView = currentView('HEAPU32');
        const nodeBytes = byteView.subarray(address, address + layout.node.size);
        const words = [];
        for (let offset = 0; offset < layout.node.size; offset += 4) {
            words.push(`0x${wordView[(address + offset) / 4].toString(16).padStart(8, '0')}`);
        }
        let bytes = '';
        for (const byte of nodeBytes) bytes += byte.toString(16).padStart(2, '0');

        return {
            ok: true,
            schema: 1,
            addr: address,
            segment: { addr: location.segment.addr, index: location.index },
            node_type: {
                value: nodeType,
                octal: octalType(nodeType),
                name: typeNameValue,
                flags: nodeFlags(nodeType),
            },
            id: null,
            my_gen: readI32(address + fields.myGen),
            gen_age: readI32(address + fields.genAge),
            mark_gc: readI32(address + fields.markGc),
            next: readU32(address + fields.next),
            oldyoung_next: readU32(address + fields.oldyoungNext),
            union,
            words,
            bytes,
        };
    }

    function readCanonicalName(object, segments) {
        if (nodeTypeAt(object, segments) !== layout.node.types.CONS)
            fail('memory-error', 'A hash entry has no readable object-cell list.');
        const canonical = readU32(
            object + layout.node.fields.union + layout.node.union.ncons.ncar,
        );
        return readStringNode(canonical, segments).text;
    }

    function findObjectValue(name, segments) {
        const hashAddress = symbolAddress('hash_table');
        const byteLength = heapByteLength();
        if (hashAddress % 4 !== 0 ||
            hashAddress + layout.hashLength * layout.pointerSize > byteLength)
            fail('memory-error', 'The hash_table is outside the current Wasm heap.');

        const requestedName = name.toLowerCase();
        const visitedEntries = new Set();
        const maximumSteps = segments.reduce((total, segment) => total + segment.size, 0);
        let steps = 0;
        for (let bucket = 0; bucket < layout.hashLength; bucket += 1) {
            let entry = readU32(hashAddress + bucket * layout.pointerSize);
            while (entry !== 0) {
                if (visitedEntries.has(entry) || ++steps > maximumSteps)
                    fail('memory-error', 'The hash_table contains a cycle or invalid chain.');
                visitedEntries.add(entry);
                if (nodeTypeAt(entry, segments) !== layout.node.types.CONS)
                    fail('memory-error', 'A hash bucket entry is not a CONS NODE.');
                const object = readU32(
                    entry + layout.node.fields.union + layout.node.union.ncons.ncar,
                );
                if (nodeTypeAt(object, segments) !== layout.node.types.CONS)
                    fail('memory-error', 'A hash bucket does not contain an object cell.');
                if (readCanonicalName(object, segments).toLowerCase() === requestedName) {
                    let valueCell = object;
                    for (let index = 0; index < layout.object.cells.valnode; index += 1) {
                        if (nodeTypeAt(valueCell, segments) !== layout.node.types.CONS)
                            fail('memory-error', 'The object-cell list ended before its value cell.');
                        valueCell = readU32(
                            valueCell + layout.node.fields.union + layout.node.union.ncons.ncdr,
                        );
                    }
                    if (nodeTypeAt(valueCell, segments) !== layout.node.types.CONS)
                        fail('memory-error', 'The object value cell is not a CONS NODE.');
                    return readU32(
                        valueCell + layout.node.fields.union + layout.node.union.ncons.ncar,
                    );
                }
                entry = readU32(
                    entry + layout.node.fields.union + layout.node.union.ncons.ncdr,
                );
            }
        }
        return null;
    }

    function status() {
        return publicCall(() => {
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);

            const segments = getSegments();
            let nodeCapacity = 0;
            let freeNodeCount = 0;
            for (const segment of segments) {
                nodeCapacity += segment.size;
                for (let index = 0; index < segment.size; index += 1) {
                    const typeAddress = segment.nodesAddr + index * layout.node.size +
                        layout.node.fields.nodeType;
                    if (readI32(typeAddress) === layout.node.types.NTFREE)
                        freeNodeCount += 1;
                }
            }

            const logoGil = readGlobalIntOrNull('LOGO_GIL');
            const insideGc = readGlobalIntOrNull('inside_gc');
            let asyncifyState = null;
            let blockedOnGetc = false;
            try {
                const state = memory.asyncifyState;
                asyncifyState = Number.isFinite(state) ? state : null;
            } catch (error) {
                asyncifyState = null;
            }
            try {
                blockedOnGetc = Boolean(memory.blockedOnGetc);
            } catch (error) {
                blockedOnGetc = false;
            }

            return {
                ok: true,
                schema: 1,
                layout: {
                    ok: true,
                    pointerSize: layout.pointerSize,
                    nodeSize: layout.node.size,
                    nodeAlignment: layout.node.alignment,
                    hashLength: layout.hashLength,
                    sampleName: layoutCheck.sampleName,
                },
                safe: { logoGil, asyncifyState, insideGc },
                idle: logoGil === 0 && asyncifyState === 0 && insideGc === 0 &&
                    !blockedOnGetc,
                heap: {
                    bytes: heapByteLength(),
                    segmentCount: segments.length,
                    nodeCount: nodeCapacity - freeNodeCount,
                    nodeCapacity,
                    freeNodeCount,
                },
            };
        });
    }

    function node(address) {
        return publicCall(() => publicNode(address));
    }

    function variable(name) {
        return publicCall(() => {
            if (typeof name !== 'string' || name.length === 0)
                fail('bad-name', 'Variable name must be a nonempty string.');
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            const valueAddress = findObjectValue(name, segments);
            if (valueAddress === null)
                fail('not-found', `No Logo object named ${name} was found.`);
            if (valueAddress === 0)
                fail('unsupported', 'The variable value is not an integer NODE.');
            const valueLocation = findNodeLocation(valueAddress, segments);
            if (!valueLocation || valueLocation.misaligned)
                fail('memory-error', 'The variable value does not point to a valid NODE.');
            const valueType = readI32(valueAddress + layout.node.fields.nodeType);
            if (valueType !== layout.node.types.INTT)
                fail('unsupported', 'Only integer-valued globals are supported in Phase 1.');
            return {
                ok: true,
                schema: 1,
                value: {
                    t: 'int',
                    v: readI32(valueAddress + layout.node.fields.union +
                        layout.node.union.nint.value),
                    addr: valueAddress,
                },
            };
        });
    }

    return { status, node, variable };
}

if (typeof module !== 'undefined' && module.exports)
    module.exports = { createLogoInspector, LAYOUT };
if (typeof globalThis !== 'undefined') {
    globalThis.createLogoInspector = createLogoInspector;
    globalThis.LAYOUT = LAYOUT;
}

function moduleMemory() {
    return {
        get HEAPU8() { return typeof HEAPU8 === 'undefined' ? null : HEAPU8; },
        get HEAP16() { return typeof HEAP16 === 'undefined' ? null : HEAP16; },
        get HEAP32() { return typeof HEAP32 === 'undefined' ? null : HEAP32; },
        get HEAPU32() { return typeof HEAPU32 === 'undefined' ? null : HEAPU32; },
        get HEAPF64() { return typeof HEAPF64 === 'undefined' ? null : HEAPF64; },
        get asyncifyState() {
            return typeof Asyncify !== 'undefined' && Asyncify &&
                Number.isFinite(Asyncify.state) ? Asyncify.state : null;
        },
        get blockedOnGetc() {
            return typeof Module !== 'undefined' && Module
                ? Module.blockedOnGetc : false;
        },
    };
}

function attachModuleInspector() {
    if (typeof Module === 'undefined' || !Module) return;
    Module.logoInspect = createLogoInspector({
        memory: moduleMemory(),
        symbols: {
            Unbound: Module['_Unbound'],
            LOGO_GIL: Module['_LOGO_GIL'],
            inside_gc: Module['_inside_gc'],
            hash_table: Module['_hash_table'],
            segment_list: Module['_segment_list'],
            the_generation: Module['_the_generation'],
            regs: Module['_regs'],
            var_stack: Module['_var_stack'],
            special_chars: Module['_special_chars'],
        },
        layout: LAYOUT,
    });
}

if (typeof Module !== 'undefined' && Module) {
    const previousOnRuntimeInitialized = Module.onRuntimeInitialized;
    Module.onRuntimeInitialized = function () {
        let result;
        try {
            if (typeof previousOnRuntimeInitialized === 'function')
                result = previousOnRuntimeInitialized.apply(this, arguments);
        } finally {
            attachModuleInspector();
        }
        return result;
    };
}