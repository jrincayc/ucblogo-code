const assert = require('node:assert/strict');
const { describe, it } = require('node:test');

const {
    buildDocs, manualDoc, markdownDoc, renderMarkdown,
} = require('../../scripts/build-help-docs.js');

describe('help documents', () => {
    it('escapes HTML and renders the supported Markdown subset', () => {
        const html = renderMarkdown('Use `<b>` and **bold** with [site](https://example.com).\n\n- one\n- two\n\n```\nx < y\n```');
        assert.match(html, /<code>&lt;b&gt;<\/code>/);
        assert.match(html, /<strong>bold<\/strong>/);
        assert.match(html, /<a href="https:\/\/example\.com\/" target="_blank" rel="noopener noreferrer">site<\/a>/);
        assert.match(html, /<ul>\s*<li>one<\/li>\s*<li>two<\/li>\s*<\/ul>/);
        assert.match(html, /<pre class="helpDocCode"><code>x &lt; y<\/code><\/pre>/);
    });

    it('never emits raw tags, script URLs, or unescaped attributes from source text', () => {
        const html = renderMarkdown('<script>alert(1)</script> [x](javascript:alert(1)) [y](a"onmouseover="z)');
        assert.ok(!/<script/i.test(html));
        assert.ok(!/href="javascript:/i.test(html));
        assert.ok(!/href="[^"]*"onmouseover/i.test(html));
    });

    it('links repository-relative targets to GitHub and drops in-page anchors', () => {
        const html = renderMarkdown('[plan](docs/plans/x.md) [top](#top)');
        assert.match(html, /href="https:\/\/github\.com\/n1ywb-1\/ucblogo-code\/blob\/wasm\/docs\/plans\/x\.md"/);
        assert.ok(!html.includes('href="#top"'));
        assert.match(html, /top/);
    });

    it('splits Markdown into sections by # and ## headings, keeping chapter intro text', () => {
        const doc = markdownDoc({ id: 'web', title: 'Web', source: '# Intro\nHello\n## Part\nBody\n### Deep\nMore\n# Empty\n## Next\nText' });
        assert.deepEqual(doc.sections.map((s) => [s.chapter, s.title]),
            [['Intro', 'Intro'], ['Intro', 'Part'], ['Empty', 'Next']]);
        assert.match(doc.sections[1].html, /<h5>Deep<\/h5>/);
    });

    it('does not treat # lines inside code fences as headings', () => {
        const doc = markdownDoc({ id: 'web', title: 'Web', source: '# A\n```\n# not heading\n```\ntext' });
        assert.equal(doc.sections.length, 1);
        assert.match(doc.sections[0].html, /# not heading/);
    });

    it('splits manual chapters and sections while retaining the license header', () => {
        const doc = manualDoc('Berkeley Logo User Manual\n * Copyright\n\nPreamble text\n\nSYNTAX\n======\n\nsyntax body\n\nDATA\n====\n\nCONSTRUCTORS\n------------\n\nWORD a b\n');
        assert.deepEqual(doc.sections.map((s) => [s.chapter, s.title]),
            [['Introduction', 'Introduction'], ['SYNTAX', 'SYNTAX'], ['DATA', 'CONSTRUCTORS']]);
        assert.ok(doc.sections[0].html.includes('Copyright'));
        assert.match(doc.sections[2].html, /WORD a b/);
    });

    it('reflows hard-wrapped manual descriptions but keeps usage lines and code preformatted', () => {
        const text = 'FORWARD dist\nFD dist\n\n        moves the turtle forward, in the direction that it\'s facing, by\n        the specified distance (measured in turtle steps).\n\n        to square :n\n        repeat 4 [fd :n rt 90]\n        end\n';
        const doc = manualDoc(`Berkeley Logo User Manual\n\nGRAPHICS\n========\n\n${text}`);
        const html = doc.sections.find((s) => s.title === 'GRAPHICS').html;
        assert.match(html, /<pre class="helpDocText">FORWARD dist\nFD dist<\/pre>/);
        assert.match(html, /<p>moves the turtle forward, in the direction that it&#39;s facing, by the specified distance \(measured in turtle steps\)\.<\/p>|<p>moves the turtle forward, in the direction that it's facing, by the specified distance \(measured in turtle steps\)\.<\/p>/);
        assert.match(html, /<pre class="helpDocText">to square :n\nrepeat 4 \[fd :n rt 90\]\nend<\/pre>/);
    });

    it('builds both documents with unique section ids', () => {
        const { docs } = buildDocs({ manual: 'X\n=\n\nbody text here\n', webedition: '# T\ntext' });
        assert.deepEqual(docs.map((d) => d.id), ['manual', 'web']);
        for (const doc of docs) {
            const ids = doc.sections.map((s) => s.id);
            assert.equal(new Set(ids).size, ids.length);
        }
    });

    it('keeps tab-aligned macro usage lines preformatted', () => {
        const usage = '.MACRO procname :input1 :input2 ...\t\t\t\t(special form)\n.DEFMACRO procname text';
        const doc = manualDoc(`MACROS\n======\n\n${usage}\n`);
        assert.ok(doc.sections[0].html.includes(`<pre class="helpDocText">${usage}</pre>`));
    });
});
