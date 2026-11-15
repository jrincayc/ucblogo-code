#!/usr/bin/env node
'use strict';

// Builds the Web Edition Help category index from docs/ucblogo.texi and helpfiles/.

const fs = require('node:fs');
const path = require('node:path');

const NON_WORD_FILES = /^(HELPCONTENTS|ALL_NAMES|Makefile(\.am|\.in)?)$/;
const OTHER = { id: 'other', title: 'Other' };

function slug(text) {
    return text.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
}

// Entries are `@unnumberedsubsec word` followed by `@cindex alias` lines; the section nests under its chapter.
function parseTexi(source) {
    const categories = [];
    let chapter = null;
    let category = null;
    let entry = null;
    const open = (title, id) => {
        category = { id, title, words: [] };
        categories.push(category);
    };
    for (const line of source.split('\n')) {
        let m;
        if ((m = /^@chapter\s+(.+?)\s*$/.exec(line))) {
            chapter = m[1];
            category = null;
            entry = null;
        } else if ((m = /^@section\s+(.+?)\s*$/.exec(line))) {
            open(`${chapter}: ${m[1]}`, slug(`${chapter} ${m[1]}`));
            entry = null;
        } else if ((m = /^@unnumberedsubsec\s+(\S+)\s*$/.exec(line))) {
            if (!category) open(chapter, slug(chapter));
            entry = { name: m[1].toLowerCase(), aliases: [] };
            category.words.push(entry);
        } else if (entry && (m = /^@cindex\s+(\S+)\s*$/.exec(line))) {
            const alias = m[1].toLowerCase();
            if (alias !== entry.name) entry.aliases.push(alias);
        } else if (/^@(node|chapter|section|unnumberedsubsec)\b/.test(line)) {
            entry = null;
        }
    }
    return categories.filter((c) => c.words.length);
}

// Mirrors makehelp.c: dots become 'd', and the grave accent file is renamed.
function helpFileName(name) {
    return name === '`' ? 'RENAME-GRAVE-ACCENT' : name.replace(/\./g, 'd');
}

function usageLines(helpDir, name) {
    const file = path.join(helpDir, helpFileName(name));
    if (!fs.existsSync(file)) return null;
    const text = fs.readFileSync(file, 'utf8');
    const split = text.search(/\n\s*\n/);
    return text.slice(0, split < 0 ? undefined : split).split('\n')
        .map((line) => line.replace(/\(library procedure\)/i, '').trim())
        .filter(Boolean);
}

function buildIndex({ texi, helpDir }) {
    const categories = parseTexi(fs.readFileSync(texi, 'utf8'));
    const files = fs.readdirSync(helpDir).filter((f) => !NON_WORD_FILES.test(f));
    const placed = new Set();
    const missing = [];
    for (const category of categories) {
        category.words = category.words.filter((word) => {
            const usage = usageLines(helpDir, word.name);
            if (!usage) {
                missing.push(word.name);
                return false;
            }
            word.usage = usage;
            placed.add(helpFileName(word.name));
            word.aliases = word.aliases.filter((alias) => files.includes(helpFileName(alias)));
            word.aliases.forEach((alias) => placed.add(helpFileName(alias)));
            return true;
        });
    }
    const orphans = files.filter((f) => !placed.has(f)).sort();
    const result = categories.filter((c) => c.words.length);
    if (orphans.length) {
        result.push({
            ...OTHER,
            words: orphans.map((name) => ({ name: name.toLowerCase(), aliases: [], usage: usageLines(helpDir, name) || [] })),
        });
    }
    return { categories: result, missing };
}

function main(argv) {
    const root = path.resolve(__dirname, '..');
    const outDir = path.resolve(argv[2] || path.join(root, 'help'));
    const { categories, missing } = buildIndex({
        texi: path.join(root, 'docs', 'ucblogo.texi'),
        helpDir: path.join(root, 'helpfiles'),
    });
    fs.mkdirSync(outDir, { recursive: true });
    fs.writeFileSync(path.join(outDir, 'index.json'), JSON.stringify({ categories }));
    const words = categories.reduce((n, c) => n + c.words.length, 0);
    console.log(`help/index.json: ${categories.length} categories, ${words} words`);
    if (missing.length) console.warn(`No helpfile for: ${missing.join(', ')}`);
}

if (require.main === module) main(process.argv);

module.exports = { buildIndex, helpFileName, parseTexi };
