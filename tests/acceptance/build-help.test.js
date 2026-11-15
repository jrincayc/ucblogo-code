const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const assert = require('node:assert/strict');
const { afterEach, beforeEach, describe, it } = require('node:test');

const { buildIndex, helpFileName, parseTexi } = require('../../scripts/build-help.js');

const TEXI = `@chapter Data
@section Constructors
@node WORD
@unnumberedsubsec word
@cindex word

@node SENTENCE
@unnumberedsubsec sentence
@cindex sentence
@cindex se

@chapter Arithmetic
@node SUM
@unnumberedsubsec sum
@cindex sum
`;

describe('help index', () => {
    let dir;

    beforeEach(() => {
        dir = fs.mkdtempSync(path.join(os.tmpdir(), 'build-help-'));
        fs.mkdirSync(path.join(dir, 'helpfiles'));
        fs.writeFileSync(path.join(dir, 'ucblogo.texi'), TEXI);
        const write = (name, usage) => fs.writeFileSync(path.join(dir, 'helpfiles', name), `${usage}\n\nbody text\n`);
        write('word', 'WORD a b');
        write('sentence', 'SENTENCE a b');
        write('se', 'SE a b');
        write('sum', 'SUM a b');
        write('stray', 'STRAY x');
        fs.writeFileSync(path.join(dir, 'helpfiles', 'ALL_NAMES'), 'ignored');
    });

    afterEach(() => fs.rmSync(dir, { recursive: true, force: true }));

    it('groups entries by chapter and section, with chapter-level entries kept', () => {
        const categories = parseTexi(TEXI);
        assert.deepEqual(categories.map((c) => c.title), ['Data: Constructors', 'Arithmetic']);
        assert.deepEqual(categories[0].words.map((w) => w.name), ['word', 'sentence']);
        assert.deepEqual(categories[0].words[1].aliases, ['se']);
    });

    it('places every helpfile exactly once, orphans in Other', () => {
        const { categories, missing } = buildIndex({
            texi: path.join(dir, 'ucblogo.texi'),
            helpDir: path.join(dir, 'helpfiles'),
        });
        const seen = categories.flatMap((c) => c.words.flatMap((w) => [w.name, ...w.aliases]));
        assert.deepEqual(seen.sort(), ['se', 'sentence', 'stray', 'sum', 'word']);
        assert.equal(new Set(seen).size, seen.length);
        assert.equal(categories.at(-1).title, 'Other');
        assert.deepEqual(categories.at(-1).words.map((w) => w.name), ['stray']);
        assert.deepEqual(missing, []);
    });

    it('maps Logo names to helpfile names as makehelp.c does', () => {
        assert.equal(helpFileName('.setfirst'), 'dsetfirst');
        assert.equal(helpFileName('do.while'), 'dodwhile');
        assert.equal(helpFileName('`'), 'RENAME-GRAVE-ACCENT');
        assert.equal(helpFileName('fput'), 'fput');
    });

    it('reports texi words without a helpfile and drops them', () => {
        fs.rmSync(path.join(dir, 'helpfiles', 'sum'));
        const { categories, missing } = buildIndex({
            texi: path.join(dir, 'ucblogo.texi'),
            helpDir: path.join(dir, 'helpfiles'),
        });
        assert.deepEqual(missing, ['sum']);
        assert.ok(!categories.some((c) => c.title === 'Arithmetic'));
    });
});
