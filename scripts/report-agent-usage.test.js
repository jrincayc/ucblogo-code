'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort unavailable).

const assert = require('node:assert/strict');
const test = require('node:test');
const {
  aggregate,
  formatSummary,
  parseArgs,
} = require('./report-agent-usage-v2.js');

function measured(id, gross, discount, net) {
  return {
    hash: id,
    timestamp: '2026-11-22T03:40:00Z',
    subject: id,
    record: {
      accounting_id: id,
      billing: { availability: 'available', provenance: 'measured' },
      totals: { gross, discount, net },
      product_sku_totals: [{
        product: 'copilot', sku: 'Copilot AI Credits', quantity: '100', unit: 'AICredits',
        gross, gross_reason: null, discount, discount_reason: null, net, net_reason: null,
      }],
    },
  };
}

test('aggregate deduplicates accounting IDs', () => {
  const result = aggregate([measured('a', '12.00', '1.25', '10.75'), measured('a', '12.00', '1.25', '10.75')]);
  assert.equal(result.valid.length, 1);
  assert.equal(result.gross, '12.00');
  assert.equal(result.net, '10.75');
});

test('PR and issue summaries preserve unavailable totals', () => {
  const unavailable = { hash: 'missing', record: null };
  const pr = formatSummary(aggregate([unavailable]), 'pr');
  const issue = formatSummary(aggregate([unavailable]), 'issue');
  assert.match(pr, /UNAVAILABLE/);
  assert.match(issue, /UNAVAILABLE/);
  assert.match(pr, /temporal attribution/);
});

test('PR format aggregates measured totals and CO2e', () => {
  const result = aggregate([measured('a', '12.00', '1.25', '10.75')]);
  const pr = formatSummary(result, 'pr');
  assert.match(pr, /Gross: \$12\.00; discount: \$1\.25; net: \$10\.75/);
  assert.match(pr, /2\.15 kg/);
});

test('argument parser recognizes range and issue mode', () => {
  assert.equal(parseArgs(['--range', 'A..B', '--issue']).range, 'A..B');
  assert.equal(parseArgs(['--range', 'A..B', '--issue']).mode, 'issue');
  assert.throws(() => parseArgs(['--unknown']), /unknown argument/);
});
