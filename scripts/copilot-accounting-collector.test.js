'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort unavailable).

const assert = require('node:assert/strict');
const test = require('node:test');
const {
  normalizeMoney,
  normalizeSnapshot,
} = require('./copilot-accounting-collector.js');

const options = {
  root: process.cwd(),
  taskId: 'collector-test',
  editorName: 'GitHub Copilot',
  model: 'MAI Code 2 Flash',
  reasoningEffort: 'unavailable',
  githubLogin: 'n1ywb-1',
  billingSource: 'test synthetic',
  generatedAt: '2026-11-22T03:30:00Z',
};

test('measured snapshot produces typed canonical totals and CO2e', () => {
  const record = normalizeSnapshot({
    checkpoint: 'implementation',
    provenance: 'measured',
    totals: { gross: 12, discount: 1.25, net: 10.75 },
    billing: { availability: 'available', source: 'api', asOf: '2026-11-22T03:29:00Z' },
    includedCredits: { baseline: 20000, used: 15000, remaining: 5000, resetAt: '2026-11-30' },
    additionalBudget: { spent: 11.3, limit: 12, remaining: 0.7, effectiveAt: '2026-11-22' },
    productSkuTotals: [{
      product: 'copilot', sku: 'Copilot AI Credits', quantity: '100', unit: 'AICredits',
      gross: '12.00', grossReason: null, discount: '1.25', discountReason: null,
      net: '10.75', netReason: null,
    }],
    modelTotals: [],
  }, options);

  assert.equal(record.totals.gross, '12.00');
  assert.equal(record.totals.discount, '1.25');
  assert.equal(record.totals.net, '10.75');
  assert.equal(record.billing.availability, 'available');
  assert.equal(record.co2e.kilograms, '2.15');
  assert.equal(record.co2e.classification, 'estimated-not-measured');
});

test('unavailable snapshot keeps null values and explicit reasons', () => {
  const record = normalizeSnapshot({
    reason: 'billing endpoint unavailable',
  }, options);

  assert.equal(record.billing.availability, 'unavailable');
  assert.equal(record.totals.gross, null);
  assert.equal(record.totals.discount, null);
  assert.equal(record.totals.net, null);
  assert.match(record.totals.net_reason, /unavailable/);
  assert.equal(record.co2e.kilograms, null);
  assert.equal(record.co2e.classification, 'unavailable');
});

test('privacy-negative input is rejected', () => {
  assert.throws(() => normalizeSnapshot({
    prompt: 'do not persist me',
  }, options), /privacy-unsafe field/);
});

test('money normalization accepts exact decimal strings and rejects malformed values', () => {
  assert.deepEqual(normalizeMoney('12.00', null, 'test'), { value: '12.00', reason: null });
  assert.deepEqual(normalizeMoney(null, 'not available', 'test'), { value: null, reason: 'not available' });
  assert.throws(() => normalizeMoney('12.0', null, 'test'), /two-decimal/);
});
