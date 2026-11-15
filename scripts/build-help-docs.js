'use strict';

// Renders the user manual and webedition.md into sections for the Help dialog.

const GITHUB_BLOB = 'https://github.com/n1ywb-1/ucblogo-code/blob/wasm/';
const MarkdownIt = require('markdown-it');
const markdown = new MarkdownIt({ html: false });
const renderLink = markdown.renderer.rules.link_open
    || ((tokens, index, options, environment, renderer) => renderer.renderToken(tokens, index, options));

markdown.renderer.rules.link_open = (tokens, index, options, environment, renderer) => {
    const token = tokens[index];
    const target = token.attrGet('href');
    if (target.startsWith('#')) token.attrs = token.attrs.filter(([name]) => name !== 'href');
    else {
        token.attrSet('href', new URL(target, GITHUB_BLOB).href);
        token.attrSet('target', '_blank');
        token.attrSet('rel', 'noopener noreferrer');
    }
    return renderLink(tokens, index, options, environment, renderer);
};

markdown.renderer.rules.fence = (tokens, index) =>
    `<pre class="helpDocCode"><code>${escapeHtml(tokens[index].content.trimEnd())}</code></pre>`;

function escapeHtml(text) {
    return text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

function slug(text) {
    return text.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
}

function renderBlocks(lines, headingOffset) {
    const tokens = markdown.parse(lines.join('\n'), {});
    for (const token of tokens) {
        if (token.type === 'heading_open' || token.type === 'heading_close') {
            token.tag = `h${Math.min(6, Number(token.tag.slice(1)) + headingOffset)}`;
        }
    }
    return markdown.renderer.render(tokens, markdown.options, {});
}

function renderMarkdown(source, headingOffset = 2) {
    return renderBlocks(source.split('\n'), headingOffset);
}

// A `#` heading starts a chapter; each `##` heading under it is a section, and text before the first `##` becomes the chapter's own section.
function parseMarkdownSections(source) {
    const sections = [];
    let chapter = null;
    let current = null;
    let fence = false;
    const open = (title, chapterTitle) => {
        current = { title, chapter: chapterTitle, lines: [] };
        sections.push(current);
    };
    for (const line of source.split('\n')) {
        if (/^```/.test(line)) fence = !fence;
        let m;
        if (!fence && (m = /^#\s+(.+?)\s*$/.exec(line))) {
            chapter = m[1];
            open(chapter, chapter);
        } else if (!fence && (m = /^##\s+(.+?)\s*$/.exec(line))) {
            open(m[1], chapter || m[1]);
        } else if (current) {
            current.lines.push(line);
        }
    }
    return sections.filter((s) => s.lines.some((l) => l.trim()));
}

function markdownDoc({ id, title, source }) {
    const sections = parseMarkdownSections(source).map((s) => ({
        id: slug(`${s.chapter}-${s.title}`),
        title: s.title,
        chapter: s.chapter,
        html: renderBlocks(s.lines, 2),
    }));
    return { id, title, sections };
}

const UNDERLINE = /^(=|-){4,}\s*$/;

// Chapters are underlined with `=`, sections with `-`; a chapter without sections is its own section.
function parseManualSections(source) {
    const lines = source.split('\n');
    const sections = [];
    let chapter = null;
    let current = null;
    const open = (title, chapterTitle) => {
        current = { title, chapter: chapterTitle, lines: [] };
        sections.push(current);
    };
    for (let i = 0; i < lines.length; i++) {
        const next = lines[i + 1] || '';
        if (lines[i].trim() && UNDERLINE.test(next) && next.trim().length >= lines[i].trim().length - 2) {
            const title = lines[i].trim();
            if (next.startsWith('=')) {
                chapter = title;
                open(title, title);
            } else {
                open(title, chapter || title);
            }
            i++;
        } else if (current) {
            current.lines.push(lines[i]);
        }
    }
    return sections;
}

// Hard-wrapped description paragraphs become real paragraphs; usage lines and short-line blocks (code) stay preformatted.
function renderManualText(text) {
    const blocks = text.split(/\n[ \t]*\n/).map((block) => block.replace(/^\n+|\s+$/g, '')).filter(Boolean);
    return blocks.map((block) => {
        const lines = block.split('\n');
        const indents = lines.map((line) => line.match(/^ */)[0].length);
        const min = Math.min(...indents);
        const flat = Math.max(...indents) - min <= 1;
        const body = lines.slice(0, lines.length > 1 ? -1 : 1);
        const mean = body.reduce((sum, line) => sum + line.trim().length, 0) / body.length;
        const prose = flat && !lines.some((line) => /\S(?: {4,}|\t+)\S/.test(line))
            && (lines.length > 1 ? mean >= 45 : min >= 4 || mean >= 50);
        if (prose) return `<p>${escapeHtml(lines.map((line) => line.trim()).join(' '))}</p>`;
        return `<pre class="helpDocText">${escapeHtml(lines.map((line) => line.slice(Math.min(min, 8))).join('\n'))}</pre>`;
    }).join('');
}

function manualDoc(source) {
    const body = source.replace(/^Berkeley Logo User Manual\s*\n/, '');
    const intro = parseManualSections(`Introduction\n============\n${body}`);
    const sections = intro.map((s) => ({
        id: slug(`${s.chapter}-${s.title}`),
        title: s.title,
        chapter: s.chapter,
        html: renderManualText(s.lines.join('\n')),
    })).filter((s) => s.html.replace(/<[^>]+>/g, '').trim());
    return { id: 'manual', title: 'User manual', sections };
}

function buildDocs({ manual, webedition }) {
    return {
        docs: [
            manualDoc(manual),
            markdownDoc({ id: 'web', title: 'Web Edition', source: webedition }),
        ],
    };
}

module.exports = { buildDocs, escapeHtml, manualDoc, markdownDoc, renderMarkdown, parseManualSections, parseMarkdownSections };
