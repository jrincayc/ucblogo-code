// Co-authored by GitHub Copilot. Scope: Phase 1 Logo inspector implementation.
// Model: GPT-6 Luna; reasoning setting unavailable (maximum effort requested).
// Task wall-clock through 2026-11-17T18:32:52Z: 21m 02s.
// Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available.
// Co-authored by GitHub Copilot. Scope: Phase 2 Logo inspector implementation.
// Model: GPT-6 Luna (delegated); maximum reasoning effort requested, actual setting unavailable.
// Task wall-clock through 2026-11-17T19:51:29Z: 24m 29s.
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

const OBJECT_FLAGS = Object.freeze({
    PROC_BURIED: 0o1,
    VAL_BURIED: 0o2,
    PLIST_BURIED: 0o4,
    PROC_TRACED: 0o10,
    VAL_TRACED: 0o20,
    PLIST_TRACED: 0o40,
    PROC_STEPPED: 0o100,
    VAL_STEPPED: 0o200,
    PLIST_STEPPED: 0o400,
    PROC_MACRO: 0o1000,
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

    function decodeLogoBytes(pointer, length, maximumBytes = 65536) {
        if (!isWasmAddress(pointer) || !Number.isSafeInteger(length) || length < 0 ||
            pointer + length > heapByteLength())
            fail('memory-error', 'A Logo string has invalid bounds.');
        const renderedLength = Math.min(length, Math.max(0, maximumBytes));
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

    function readStringNode(address, segments, code = 'memory-error', maximumBytes = 65536) {
        const nodeType = nodeTypeAt(address, segments, code);
        if ((nodeType & layout.node.flags.NT_STRING) === 0)
            fail(code, 'A CASEOBJ canonical name is not a string NODE.');
        const unionAddress = address + layout.node.fields.union;
        const pointer = readU32(unionAddress + layout.node.union.nstring.nstringPtr);
        const length = readI32(unionAddress + layout.node.union.nstring.nstringLen);
        const decoded = decodeLogoBytes(pointer, length, maximumBytes);
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

    function readArrayFields(address) {
        const unionAddress = address + layout.node.fields.union;
        const dim = readI32(unionAddress + layout.node.union.narray.dim);
        const origin = readI32(unionAddress + layout.node.union.narray.origin);
        const data = readU32(unionAddress + layout.node.union.narray.data);
        const byteLength = dim * layout.pointerSize;
        if (!Number.isSafeInteger(dim) || dim < 0 || !Number.isSafeInteger(byteLength))
            fail('memory-error', 'An ARRAY has an invalid dimension.');
        if (dim > 0 && data === 0)
            fail('memory-error', 'An ARRAY with elements has no backing data pointer.');
        checkRange(data, byteLength, layout.pointerSize);
        const elements = [];
        for (let index = 0; index < dim; index += 1)
            elements.push(readU32(data + index * layout.pointerSize));
        return { dim, origin, data, elements };
    }

    function readNodeData(address, segments, maximumStringBytes = 65536) {
        if (!isWasmAddress(address) || address === 0)
            fail('bad-address', 'NODE address must be a nonzero wasm32 address.');
        if (address % layout.node.alignment !== 0)
            fail('misaligned', 'NODE address must be 8-byte aligned.');
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
            union = { view: 'narray', ...readArrayFields(address) };
        } else if ((nodeType & layout.node.flags.NT_STRING) !== 0) {
            const string = readStringNode(address, segments, 'memory-error', maximumStringBytes);
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

        const result = {
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
        if (nodeType === layout.node.types.NTFREE) result.freed = true;
        return result;
    }

    function publicNode(address) {
        if (!isWasmAddress(address) || address === 0)
            fail('bad-address', 'NODE address must be a nonzero wasm32 address.');
        if (address % layout.node.alignment !== 0)
            fail('misaligned', 'NODE address must be 8-byte aligned.');

        const layoutCheck = ensureLayout();
        if (!layoutCheck.ok) return errorResult('layout-mismatch', layoutCheck.message);
        return { ok: true, schema: 1, ...readNodeData(address, getSegments()) };
    }

    function readCanonicalName(object, segments) {
        if (nodeTypeAt(object, segments) !== layout.node.types.CONS)
            fail('memory-error', 'A hash entry has no readable object-cell list.');
        const canonical = readU32(
            object + layout.node.fields.union + layout.node.union.ncons.ncar,
        );
        return readStringNode(canonical, segments).text;
    }

    function consCar(address, segments, message = 'A Logo list cell is not a CONS NODE.') {
        if (nodeTypeAt(address, segments) !== layout.node.types.CONS)
            fail('memory-error', message);
        return readU32(address + layout.node.fields.union + layout.node.union.ncons.ncar);
    }

    function consCdr(address, segments, message = 'A Logo list cell is not a CONS NODE.') {
        if (nodeTypeAt(address, segments) !== layout.node.types.CONS)
            fail('memory-error', message);
        return readU32(address + layout.node.fields.union + layout.node.union.ncons.ncdr);
    }

    function readObjectCells(object, segments) {
        const values = [];
        let cell = object;
        for (let index = 0; index < 5; index += 1) {
            values.push(consCar(cell, segments, 'The object-cell list is incomplete.'));
            cell = consCdr(cell, segments, 'The object-cell list is incomplete.');
        }
        if (cell !== 0 && nodeTypeAt(cell, segments) !== layout.node.types.CONS)
            fail('memory-error', 'The object case-list pointer is not a CONS NODE.');
        return {
            object,
            canonical: values[layout.object.cells.canonical],
            procnode: values[layout.object.cells.procnode],
            valnode: values[layout.object.cells.valnode],
            plist: values[layout.object.cells.plist],
            obflags: values[layout.object.cells.obflags],
            caselist: cell,
        };
    }

    function walkObjects(segments, callback) {
        const hashAddress = symbolAddress('hash_table');
        const byteLength = heapByteLength();
        if (hashAddress % 4 !== 0 ||
            hashAddress + layout.hashLength * layout.pointerSize > byteLength)
            fail('memory-error', 'The hash_table is outside the current Wasm heap.');

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
                const record = {
                    ...readObjectCells(object, segments),
                    name: readCanonicalName(object, segments),
                };
                if (callback(record)) return record;
                entry = readU32(
                    entry + layout.node.fields.union + layout.node.union.ncons.ncdr,
                );
            }
        }
        return null;
    }

    function findObject(name, segments) {
        const requestedName = name.toLowerCase();
        return walkObjects(segments, (object) => object.name.toLowerCase() === requestedName);
    }

    function findObjectByAddress(address, segments) {
        return walkObjects(segments, (object) => object.object === address);
    }

    function findObjectValue(name, segments) {
        const object = findObject(name, segments);
        return object === null ? null : object.valnode;
    }

    function traversalLimits(options) {
        if (options === undefined || options === null) options = {};
        if (typeof options !== 'object' || Array.isArray(options))
            fail('bad-options', 'Inspection options must be an object.');

        const limits = { depth: 8, count: 1000, bytes: 65536 };
        const maximums = { depth: 256, count: 100000, bytes: 16 * 1024 * 1024 };
        for (const key of Object.keys(limits)) {
            if (options[key] === undefined) continue;
            if (!Number.isSafeInteger(options[key]) || options[key] < 0 ||
                options[key] > maximums[key])
                fail('bad-options', `${key} must be a nonnegative bounded integer.`);
            limits[key] = options[key];
        }
        limits.roles = options.roles !== false;
        return limits;
    }

    function readListItems(address, segments, maximum = 100000) {
        const items = [];
        const visited = new Set();
        let cell = address;
        while (cell !== 0) {
            const type = nodeTypeAt(cell, segments);
            if (type === layout.node.types.PNIL) break;
            if (type !== layout.node.types.CONS)
                fail('memory-error', 'A Logo list has a non-list tail.');
            if (visited.has(cell)) fail('memory-error', 'A Logo list is cyclic.');
            if (items.length >= maximum)
                fail('limit', 'A Logo list exceeds the inspection limit.');
            visited.add(cell);
            items.push(consCar(cell, segments));
            cell = consCdr(cell, segments);
        }
        return items;
    }

    function printableWord(text, nodeType) {
        if ((nodeType & layout.node.flags.NT_VBAR) !== 0 || text.length === 0 ||
            /[\s\[\]{}|]/u.test(text) || text.charAt(0) === '"' ||
            text.charAt(0) === ':')
            return `|${text.replace(/\|/g, '\\|')}|`;
        return text;
    }

    function decodedValueText(value) {
        if (value.t === 'word') return value.text;
        if (value.t === 'int') return String(value.v);
        if (value.t === 'float')
            return value.v === null ? value.repr : String(value.v);
        if (value.t === 'list') return value.text;
        return null;
    }

    function decodeValue(address, depth, state, segments) {
        if (depth >= state.limits.depth)
            return { t: 'truncated', reason: 'depth' };
        if (address === 0)
            return { t: 'list', items: [], addr: 0, text: '[]' };
        if (!isWasmAddress(address))
            fail('bad-address', 'Logo value address must be a wasm32 address.');

        const location = findNodeLocation(address, segments);
        if (!location) fail('bad-address', 'Logo value address is outside all NODE segments.');
        if (location.misaligned)
            fail('misaligned', 'Logo value address is not aligned to the NODE stride.');
        const nodeType = nodeTypeAt(address, segments);
        if (nodeType === layout.node.types.NTFREE)
            fail('freed-node', 'Logo value points to a freed NODE.');
        if (state.visited.has(address)) return { t: 'ref', addr: address };
        if (state.count >= state.limits.count)
            return { t: 'truncated', reason: 'count' };
        state.visited.add(address);
        state.count += 1;

        const unionAddress = address + layout.node.fields.union;
        if (nodeType === layout.node.types.INTT) {
            return {
                t: 'int',
                v: readI32(unionAddress + layout.node.union.nint.value),
                addr: address,
            };
        }
        if (nodeType === layout.node.types.FLOATT) {
            const value = readF64(unionAddress + layout.node.union.nfloat.value);
            return Number.isFinite(value)
                ? { t: 'float', v: value, addr: address }
                : {
                    t: 'float',
                    v: null,
                    repr: Number.isNaN(value) ? 'nan' : value < 0 ? '-inf' : 'inf',
                    addr: address,
                };
        }
        if ((nodeType & layout.node.flags.NT_STRING) !== 0) {
            const string = readStringNode(address, segments, 'memory-error', state.bytesRemaining);
            state.bytesRemaining -= string.strbytes.length / 2;
            const value = { t: 'word', s: string.text, addr: address };
            if (!string.truncated)
                value.text = printableWord(string.text, nodeType);
            else
                value.truncated = true;
            return value;
        }
        if (nodeType === layout.node.types.CASEOBJ) {
            const stringAddress = readU32(unionAddress + layout.node.union.ncons.ncar);
            const string = readStringNode(
                stringAddress, segments, 'memory-error', state.bytesRemaining,
            );
            state.bytesRemaining -= string.strbytes.length / 2;
            const value = { t: 'word', s: string.text, addr: address };
            if (!string.truncated)
                value.text = printableWord(string.text, string.nodeType);
            else
                value.truncated = true;
            return value;
        }
        if (nodeType === layout.node.types.PUNBOUND)
            return { t: 'unbound', addr: address };
        if (nodeType === layout.node.types.PNIL)
            return { t: 'list', items: [], addr: address, text: '[]' };
        if (nodeType === layout.node.types.CONS) {
            const result = { t: 'list', items: [], addr: address };
            let cell = address;
            let complete = true;
            while (cell !== 0) {
                const cellType = nodeTypeAt(cell, segments);
                if (cellType === layout.node.types.PNIL) break;
                if (cellType !== layout.node.types.CONS)
                    fail('memory-error', 'A Logo list has a non-list tail.');
                if (cell !== address) {
                    if (state.visited.has(cell)) {
                        result.items.push({ t: 'ref', addr: cell });
                        complete = false;
                        break;
                    }
                    if (state.count >= state.limits.count) {
                        result.items.push({ t: 'truncated', reason: 'count' });
                        complete = false;
                        break;
                    }
                    state.visited.add(cell);
                    state.count += 1;
                }

                const item = decodeValue(consCar(cell, segments), depth + 1, state, segments);
                result.items.push(item);
                if (item.t === 'truncated') {
                    complete = false;
                    break;
                }
                const next = consCdr(cell, segments);
                if (next === 0) break;
                const nextType = nodeTypeAt(next, segments);
                if (nextType === layout.node.types.PNIL) break;
                if (nextType !== layout.node.types.CONS)
                    fail('memory-error', 'A Logo list has a non-list tail.');
                if (state.visited.has(next)) {
                    result.items.push({ t: 'ref', addr: next });
                    complete = false;
                    break;
                }
                if (state.count >= state.limits.count) {
                    result.items.push({ t: 'truncated', reason: 'count' });
                    complete = false;
                    break;
                }
                cell = next;
            }
            const itemTexts = result.items.map(decodedValueText);
            if (complete && itemTexts.every((text) => typeof text === 'string'))
                result.text = `[${itemTexts.join(' ')}]`;
            return result;
        }
        if (nodeType === layout.node.types.ARRAY) {
            const array = readArrayFields(address);
            const result = { t: 'array', origin: array.origin, items: [], addr: address };
            for (const element of array.elements) {
                const item = decodeValue(element, depth + 1, state, segments);
                result.items.push(item);
                if (item.t === 'truncated') break;
            }
            return result;
        }
        fail('unsupported-type', `Logo NODE type ${typeName(nodeType) || 'unknown'} is not a value.`);
    }

    function valueFromAddress(address, options, segments) {
        const limits = traversalLimits(options);
        return decodeValue(address, 0, {
            limits,
            visited: new Set(),
            count: 0,
            bytesRemaining: limits.bytes,
        }, segments);
    }

    function readCaseObjectName(address, segments) {
        if (nodeTypeAt(address, segments) !== layout.node.types.CASEOBJ)
            fail('memory-error', 'A procedure formal is not a CASEOBJ NODE.');
        const stringAddress = readU32(address + layout.node.fields.union +
            layout.node.union.ncons.ncar);
        return readStringNode(stringAddress, segments).text;
    }

    function formatNodeText(address, segments, path = new Set(), depth = 0) {
        if (address === 0) return '[]';
        if (depth > 64) return '...';
        const type = nodeTypeAt(address, segments);
        if (type === layout.node.types.NTFREE)
            fail('freed-node', 'Procedure text points to a freed NODE.');
        if (path.has(address)) return '...';
        path.add(address);
        let text;
        if ((type & layout.node.flags.NT_STRING) !== 0) {
            const string = readStringNode(address, segments);
            text = string.text.startsWith(':') &&
                (type & layout.node.flags.NT_VBAR) === 0
                ? string.text : printableWord(string.text, type);
        } else if (type === layout.node.types.INTT) {
            text = String(readI32(address + layout.node.fields.union +
                layout.node.union.nint.value));
        } else if (type === layout.node.types.FLOATT) {
            const value = readF64(address + layout.node.fields.union +
                layout.node.union.nfloat.value);
            text = Number.isFinite(value)
                ? String(value) : Number.isNaN(value) ? 'nan' : value < 0 ? '-inf' : 'inf';
        } else if (type === layout.node.types.CASEOBJ) {
            text = readCaseObjectName(address, segments);
        } else if (type === layout.node.types.QUOTE) {
            const child = readU32(address + layout.node.fields.union +
                layout.node.union.ncons.ncar);
            text = `"${formatNodeText(child, segments, path, depth + 1)}`;
        } else if (type === layout.node.types.COLON) {
            const child = readU32(address + layout.node.fields.union +
                layout.node.union.ncons.ncar);
            text = `:${formatNodeText(child, segments, path, depth + 1)}`;
        } else if (type === layout.node.types.PNIL) {
            text = '[]';
        } else if (type === layout.node.types.CONS) {
            const items = readListItems(address, segments, 10000);
            text = `[${items.map((item) =>
                formatNodeText(item, segments, path, depth + 1)).join(' ')}]`;
        } else {
            text = `<${typeName(type) || 'NODE'}>`;
        }
        path.delete(address);
        return text;
    }

    function formatNodeLine(address, segments) {
        if (address === 0) return '';
        if (nodeTypeAt(address, segments) !== layout.node.types.CONS)
            return formatNodeText(address, segments);
        return readListItems(address, segments, 10000)
            .map((item) => formatNodeText(item, segments)).join(' ');
    }

    function formalInfo(address, segments) {
        const type = nodeTypeAt(address, segments);
        if (type === layout.node.types.INTT)
            return { name: null, source: formatNodeText(address, segments) };
        if (type === layout.node.types.CASEOBJ) {
            const name = readCaseObjectName(address, segments);
            return { name, source: `:${name}` };
        }
        if (type === layout.node.types.CONS) {
            const parts = readListItems(address, segments, 1000);
            if (parts.length === 0) return { name: null, source: '[]' };
            const firstType = nodeTypeAt(parts[0], segments);
            const name = firstType === layout.node.types.CASEOBJ
                ? readCaseObjectName(parts[0], segments)
                : ((firstType & layout.node.flags.NT_STRING) !== 0
                    ? readStringNode(parts[0], segments).text : null);
            const defaults = parts.slice(1).map((part) => formatNodeText(part, segments));
            const sourceParts = [name === null ? '' : `:${name}`, ...defaults].filter(Boolean);
            return { name, source: `[${sourceParts.join(' ')}]` };
        }
        fail('memory-error', 'A procedure formal has an invalid NODE type.');
    }

    function readProcedureLayout(address, segments) {
        if (nodeTypeAt(address, segments) !== layout.node.types.CONS)
            fail('memory-error', 'A user procedure node is not a CONS NODE.');
        const text = consCar(address, segments, 'A user procedure has no text cell.');
        const bodyWordsCell = consCdr(address, segments, 'A user procedure has no body-words cell.');
        const minArgsCell = consCdr(bodyWordsCell, segments, 'A user procedure has no minargs cell.');
        const defaultArgsCell = consCdr(minArgsCell, segments,
            'A user procedure has no default-args cell.');
        const maxArgsCell = consCdr(defaultArgsCell, segments,
            'A user procedure has no maxargs cell.');
        return {
            text,
            formals: consCar(text, segments, 'A user procedure text cell is invalid.'),
            body: consCdr(text, segments, 'A user procedure text cell is invalid.'),
            bodyWords: consCar(bodyWordsCell, segments),
            minArgs: consCar(minArgsCell, segments),
            defaultArgs: consCar(defaultArgsCell, segments),
            maxArgs: consCar(maxArgsCell, segments),
        };
    }

    function readIntegerNode(address, description, segments) {
        if (nodeTypeAt(address, segments) !== layout.node.types.INTT)
            fail('memory-error', `${description} is not an INTT NODE.`);
        return readI32(address + layout.node.fields.union + layout.node.union.nint.value);
    }

    function readProcedureArity(address, type, segments) {
        if (type === layout.node.types.PRIM || type === layout.node.types.MACRO ||
            type === layout.node.types.TAILFORM || type === layout.node.types.INFIX) {
            const unionAddress = address + layout.node.fields.union;
            const fields = layout.node.union.nprim;
            return {
                min: readI16(unionAddress + fields.min),
                default: readI16(unionAddress + fields.dflt),
                max: readI16(unionAddress + fields.max),
            };
        }
        if (type !== layout.node.types.CONS)
            fail('not-procedure', 'The Logo object does not contain a procedure.');
        const procedure = readProcedureLayout(address, segments);
        return {
            min: readIntegerNode(procedure.minArgs, 'Procedure minargs', segments),
            default: readIntegerNode(procedure.defaultArgs, 'Procedure default args', segments),
            max: readIntegerNode(procedure.maxArgs, 'Procedure maxargs', segments),
        };
    }

    function readObjectFlags(address, segments) {
        return readIntegerNode(address, 'Object flags', segments);
    }

    function flagsFor(flagValue, category, procedureType) {
        const masks = category === 'proc'
            ? [OBJECT_FLAGS.PROC_BURIED, OBJECT_FLAGS.PROC_TRACED, OBJECT_FLAGS.PROC_STEPPED]
            : category === 'val'
                ? [OBJECT_FLAGS.VAL_BURIED, OBJECT_FLAGS.VAL_TRACED, OBJECT_FLAGS.VAL_STEPPED]
                : [OBJECT_FLAGS.PLIST_BURIED, OBJECT_FLAGS.PLIST_TRACED,
                    OBJECT_FLAGS.PLIST_STEPPED];
        return {
            buried: (flagValue & masks[0]) !== 0,
            traced: (flagValue & masks[1]) !== 0,
            stepped: (flagValue & masks[2]) !== 0,
            ...(category === 'proc'
                ? { macro: (flagValue & OBJECT_FLAGS.PROC_MACRO) !== 0 ||
                    procedureType === layout.node.types.MACRO }
                : {}),
        };
    }

    function procedureSummary(record, segments) {
        if (record.procnode === 0 ||
            record.procnode === readGlobalPointer('Unbound')) return null;
        const type = nodeTypeAt(record.procnode, segments);
        if (type === layout.node.types.PUNBOUND) return null;
        const flagValue = readObjectFlags(record.obflags, segments);
        const primitive = type === layout.node.types.PRIM || type === layout.node.types.MACRO ||
            type === layout.node.types.TAILFORM || type === layout.node.types.INFIX;
        const flags = flagsFor(flagValue, 'proc', type);
        return {
            name: record.name,
            kind: flags.macro ? 'macro' : primitive ? 'primitive' : 'user',
            arity: readProcedureArity(record.procnode, type, segments),
            flags,
        };
    }

    function procedureDetails(record, summary, segments) {
        if (summary.kind === 'primitive') return summary;
        const procedure = readProcedureLayout(record.procnode, segments);
        const formalAddresses = readListItems(procedure.formals, segments, 1000);
        const formalValues = formalAddresses.map((address) => formalInfo(address, segments));
        const formals = formalValues.map((formal) => formal.name).filter(Boolean);

        let body = [];
        let bodyWordLines = [];
        if (procedure.bodyWords !== 0) {
            bodyWordLines = readListItems(procedure.bodyWords, segments, 10000)
                .map((line) => formatNodeLine(line, segments));
        }
        if (procedure.body !== 0) {
            const bodyAddresses = readListItems(procedure.body, segments, 10000);
            const sourceLines = bodyAddresses.every((line) =>
                nodeTypeAt(line, segments) === layout.node.types.CONS ||
                nodeTypeAt(line, segments) === layout.node.types.PNIL);
            if (sourceLines)
                body = bodyAddresses.map((line) => formatNodeLine(line, segments));
        }
        if (body.length === 0 && bodyWordLines.length > 0) {
            let first = 0;
            let last = bodyWordLines.length;
            if (/^(to|macro)\s/i.test(bodyWordLines[0])) first += 1;
            if (last > first && bodyWordLines[last - 1].trim().toLowerCase() === 'end') last -= 1;
            body = bodyWordLines.slice(first, last);
        }

        const header = `to ${record.name}${formalValues.length
            ? ` ${formalValues.map((formal) => formal.source).join(' ')}` : ''}`;
        const text = bodyWordLines.length > 0
            ? bodyWordLines.join('\n')
            : [header, ...body, 'end'].join('\n');
        return { ...summary, formals, body, text };
    }

    function treeChildInfo(parentType, context, edge, childType) {
        if (parentType === layout.node.types.CASEOBJ) {
            if (edge === 'ncar') return { role: 'strnode' };
            if (edge === 'ncdr')
                return childType === layout.node.types.CONS
                    ? { role: 'object', context: { kind: 'object', index: 0 } }
                    : { role: 'object' };
        }
        if (context && context.kind === 'object') {
            const fields = ['canonical', 'procnode', 'valnode', 'plist', 'obflags'];
            if (edge === 'ncar' && context.index < fields.length) {
                const role = fields[context.index];
                return role === 'procnode' && childType === layout.node.types.CONS
                    ? { role, context: { kind: 'proc', position: 'root' } }
                    : { role };
            }
            if (edge === 'ncdr') {
                if (context.index < 4)
                    return { context: { kind: 'object', index: context.index + 1 } };
                if (context.index === 4) return { role: 'caselist' };
            }
        }
        if (context && context.kind === 'proc') {
            if (context.position === 'root') {
                if (edge === 'ncar')
                    return childType === layout.node.types.CONS
                        ? { role: 'text', context: { kind: 'proc', position: 'text' } }
                        : { role: 'text' };
                if (edge === 'ncdr')
                    return childType === layout.node.types.CONS
                        ? { context: { kind: 'proc', position: 'args', index: 1 } } : {};
            }
            if (context.position === 'text') {
                if (edge === 'ncar') return { role: 'formals' };
                if (edge === 'ncdr') return { role: 'bodylist' };
            }
            if (context.position === 'args') {
                const roles = { 1: 'bodywords', 2: 'minargs', 3: 'dfltargs', 4: 'maxargs' };
                if (edge === 'ncar' && roles[context.index])
                    return { role: roles[context.index] };
                if (edge === 'ncdr' && context.index < 4)
                    return { context: {
                        kind: 'proc', position: 'args', index: context.index + 1,
                    } };
            }
        }
        return {};
    }

    function treeNode(address, depth, state, segments, role, context) {
        if (depth >= state.limits.depth)
            return { truncated: 'depth', ...(role ? { role } : {}) };
        if (!isWasmAddress(address) || address === 0)
            fail('bad-address', 'NODE tree pointer must be a nonzero wasm32 address.');
        const location = findNodeLocation(address, segments);
        if (!location) fail('bad-address', 'NODE tree pointer is outside all NODE segments.');
        if (location.misaligned)
            fail('misaligned', 'NODE tree pointer is not aligned to the NODE stride.');
        const nodeType = nodeTypeAt(address, segments);
        if (state.visited.has(address))
            return { ref: address, ...(role ? { role } : {}) };
        if (state.count >= state.limits.count)
            return { truncated: 'count', ...(role ? { role } : {}) };
        state.visited.add(address);
        state.count += 1;

        const node = readNodeData(address, segments, state.bytesRemaining);
        if (node.union.view === 'nstring')
            state.bytesRemaining -= node.union.strbytes.length / 2;
        if (role) node.role = role;
        if (node.freed) return node;

        if (node.union.view === 'ncons') {
            for (const edge of ['ncar', 'ncdr', 'nobj']) {
                const childAddress = node.union[edge];
                if (childAddress === 0) continue;
                const childType = nodeTypeAt(childAddress, segments);
                const info = state.limits.roles
                    ? treeChildInfo(nodeType, context, edge, childType) : {};
                node.union[edge] = treeNode(
                    childAddress, depth + 1, state, segments, info.role, info.context,
                );
            }
        } else if (node.union.view === 'narray') {
            node.union.elements = node.union.elements.map((element) => {
                if (element === 0) return 0;
                return treeNode(element, depth + 1, state, segments, null, null);
            });
        } else if (node.union.view === 'nobject') {
            for (const edge of ['nvars', 'nprocs', 'nparents']) {
                const childAddress = node.union[edge];
                if (childAddress !== 0)
                    node.union[edge] = treeNode(
                        childAddress, depth + 1, state, segments, null, null,
                    );
            }
        } else if (node.union.view === 'nmethod') {
            for (const edge of ['nparent', 'nprocname']) {
                const childAddress = node.union[edge];
                if (childAddress !== 0)
                    node.union[edge] = treeNode(
                        childAddress, depth + 1, state, segments, null, null,
                    );
            }
        } else if (node.union.view === 'nstring' && node.union.nstring_head !== 0) {
            const head = node.union.nstring_head;
            node.union.nstring_head = {
                addr: head,
                refcnt: node.union.refcnt,
                bytes: node.union.strbytes,
                ...(node.union.truncated ? { truncated: true } : {}),
                role: 'nstring_head',
            };
        }
        return node;
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

    function value(addressOrName, options) {
        return publicCall(() => {
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            let valueAddress;
            if (typeof addressOrName === 'string') {
                if (addressOrName.length === 0)
                    fail('bad-name', 'Logo object name must be a nonempty string.');
                const object = findObject(addressOrName, segments);
                if (object === null)
                    fail('not-found', `No Logo object named ${addressOrName} was found.`);
                valueAddress = object.valnode;
            } else if (typeof addressOrName === 'number') {
                valueAddress = addressOrName;
            } else {
                fail('bad-address', 'Logo value must be selected by a name or NODE address.');
            }
            return {
                ok: true,
                schema: 1,
                value: valueFromAddress(valueAddress, options, segments),
            };
        });
    }

    function variable(name, options) {
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
            return {
                ok: true,
                schema: 1,
                value: valueFromAddress(valueAddress, options, segments),
            };
        });
    }

    function plist(name, options) {
        return publicCall(() => {
            if (typeof name !== 'string' || name.length === 0)
                fail('bad-name', 'Property-list name must be a nonempty string.');
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            const object = findObject(name, segments);
            if (object === null)
                fail('not-found', `No Logo object named ${name} was found.`);

            const limits = traversalLimits(options);
            const state = {
                limits,
                visited: new Set(),
                count: 0,
                bytesRemaining: limits.bytes,
            };
            const entries = [];
            const visitedPairs = new Set();
            const maximumSteps = segments.reduce((total, segment) => total + segment.size, 0);
            let pair = object.plist;
            while (pair !== 0) {
                if (visitedPairs.has(pair))
                    fail('memory-error', 'A property list contains a cycle.');
                if (entries.length >= limits.count) {
                    entries.push({ prop: null, value: { t: 'truncated', reason: 'count' } });
                    break;
                }
                if (visitedPairs.size >= maximumSteps)
                    fail('memory-error', 'A property list exceeds the NODE segment bounds.');
                if (nodeTypeAt(pair, segments) !== layout.node.types.CONS)
                    fail('memory-error', 'A property list entry is not a CONS NODE.');
                visitedPairs.add(pair);
                const propertyAddress = consCar(pair, segments);
                const property = readStringNode(
                    propertyAddress, segments, 'memory-error', state.bytesRemaining,
                );
                state.bytesRemaining -= property.strbytes.length / 2;
                const valueCell = consCdr(pair, segments);
                if (nodeTypeAt(valueCell, segments) !== layout.node.types.CONS)
                    fail('memory-error', 'A property list entry has no value cell.');
                const valueAddress = consCar(valueCell, segments);
                entries.push({
                    prop: property.text,
                    value: decodeValue(valueAddress, 0, state, segments),
                    ...(property.truncated ? { propTruncated: true } : {}),
                });
                pair = consCdr(valueCell, segments);
            }
            return { ok: true, schema: 1, value: entries };
        });
    }

    function procedure(name) {
        return publicCall(() => {
            if (typeof name !== 'string' || name.length === 0)
                fail('bad-name', 'Procedure name must be a nonempty string.');
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            const object = findObject(name, segments);
            if (object === null)
                fail('not-found', `No Logo object named ${name} was found.`);
            const summary = procedureSummary(object, segments);
            if (summary === null)
                fail('not-procedure', `Logo object ${name} has no procedure definition.`);
            return { ok: true, schema: 1, value: procedureDetails(object, summary, segments) };
        });
    }

    function workspace() {
        return publicCall(() => {
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            const unboundAddress = readGlobalPointer('Unbound');
            const procedures = [];
            const variables = [];
            const propertyLists = [];

            walkObjects(segments, (object) => {
                const flagValue = readObjectFlags(object.obflags, segments);
                const procedureInfo = procedureSummary(object, segments);
                if (procedureInfo !== null) procedures.push(procedureInfo);

                if (object.valnode === 0 || (object.valnode !== unboundAddress &&
                    nodeTypeAt(object.valnode, segments) !== layout.node.types.PUNBOUND))
                    variables.push({
                        name: object.name,
                        flags: flagsFor(flagValue, 'val'),
                    });
                if (object.plist !== 0) {
                    if (nodeTypeAt(object.plist, segments) !== layout.node.types.CONS)
                        fail('memory-error', 'An object property list is not a CONS NODE.');
                    propertyLists.push({
                        name: object.name,
                        flags: flagsFor(flagValue, 'plist'),
                    });
                }
                return false;
            });

            const byName = (left, right) => {
                const a = left.name.toLowerCase();
                const b = right.name.toLowerCase();
                return a < b ? -1 : a > b ? 1 : 0;
            };
            procedures.sort(byName);
            variables.sort(byName);
            propertyLists.sort(byName);
            return {
                ok: true,
                schema: 1,
                value: { procedures, variables, propertyLists },
            };
        });
    }

    function tree(root, options) {
        return publicCall(() => {
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            const limits = traversalLimits(options);
            let address;
            let context = null;
            if (typeof root === 'string') {
                if (root.length === 0) fail('bad-name', 'Logo object name must be nonempty.');
                const object = findObject(root, segments);
                if (object === null) fail('not-found', `No Logo object named ${root} was found.`);
                address = object.object;
                if (limits.roles) context = { kind: 'object', index: 0 };
            } else if (typeof root === 'number') {
                address = root;
                if (!isWasmAddress(address) || address === 0)
                    fail('bad-address', 'NODE tree root must be a nonzero wasm32 address.');
                if (address % layout.node.alignment !== 0)
                    fail('misaligned', 'NODE tree root must be 8-byte aligned.');
                requireNodeLocation(address, segments, 'bad-address');
                if (limits.roles) {
                    if (findObjectByAddress(address, segments) !== null) {
                        context = { kind: 'object', index: 0 };
                    } else {
                        const procObject = walkObjects(
                            segments, (object) => object.procnode === address,
                        );
                        if (procObject !== null &&
                            nodeTypeAt(address, segments) === layout.node.types.CONS)
                            context = { kind: 'proc', position: 'root' };
                    }
                }
            } else {
                fail('bad-address', 'NODE tree root must be a name or NODE address.');
            }
            const value = treeNode(address, 0, {
                limits,
                visited: new Set(),
                count: 0,
                bytesRemaining: limits.bytes,
            }, segments, null, context);
            return { ok: true, schema: 1, value };
        });
    }

    function nodes(options) {
        return publicCall(() => {
            if (options === undefined || options === null) options = {};
            if (typeof options !== 'object' || Array.isArray(options))
                fail('bad-options', 'nodes options must be an object.');
            const offset = options.offset === undefined ? 0 : options.offset;
            const requestedLimit = options.limit === undefined ? 100 : options.limit;
            if (!Number.isSafeInteger(offset) || offset < 0)
                fail('bad-options', 'nodes offset must be a nonnegative integer.');
            if (!Number.isSafeInteger(requestedLimit) || requestedLimit < 1)
                fail('bad-options', 'nodes limit must be a positive integer.');
            const limit = Math.min(requestedLimit, 1000);
            const layoutCheck = ensureLayout();
            if (!layoutCheck.ok)
                return errorResult('layout-mismatch', layoutCheck.message);
            const segments = getSegments();
            const total = segments.reduce((count, segment) => count + segment.size, 0);
            if (offset > total) fail('bad-offset', 'nodes offset exceeds the NODE segment count.');

            const result = [];
            let skipped = 0;
            for (const segment of segments) {
                for (let index = 0; index < segment.size; index += 1) {
                    if (skipped < offset) {
                        skipped += 1;
                        continue;
                    }
                    if (result.length >= limit) break;
                    result.push(readNodeData(
                        segment.nodesAddr + index * layout.node.size, segments,
                    ));
                }
                if (result.length >= limit) break;
            }
            const nextOffset = offset + result.length;
            return {
                ok: true,
                schema: 1,
                value: {
                    nodes: result,
                    pagination: {
                        offset,
                        limit,
                        total,
                        nextOffset: nextOffset < total ? nextOffset : null,
                        done: nextOffset >= total,
                    },
                },
            };
        });
    }

    function json(method) {
        let args = Array.prototype.slice.call(arguments, 1);
        let indent = 0;
        if (args.length > 0 && Array.isArray(args[0])) {
            const methodArgs = args[0];
            indent = arguments.length > 2 ? arguments[2] : 0;
            args = methodArgs.slice();
        }
        if (!Number.isInteger(indent) || indent < 0 || indent > 10)
            return JSON.stringify(errorResult('bad-options', 'JSON indentation must be from 0 to 10.'));
        const methodFunction = typeof method === 'string' && method !== 'json' &&
            Object.prototype.hasOwnProperty.call(api, method) ? api[method] : null;
        const result = typeof methodFunction === 'function'
            ? methodFunction.apply(api, args)
            : errorResult('bad-method', 'Unknown Logo inspector method.');
        try {
            return JSON.stringify(result, null, indent);
        } catch (error) {
            return JSON.stringify(errorResult('json-error', 'Inspection result is not JSON-safe.'));
        }
    }

    const api = { status, node, value, variable, plist, procedure, workspace, tree, nodes, json };
    return api;
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