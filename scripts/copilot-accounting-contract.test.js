'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort unavailable).

const assert = require('node:assert/strict');
const test = require('node:test');
const {
  BEGIN,
  END,
  attach,
  canonicalJson,
  humanSummary,
  parseMessage,
  unavailableRecord,
  validateRecord,
} = require('./copilot-accounting.js');

function baseRecord(overrides = {}) {
  const record = unavailableRecord({
    root: process.cwd(),
    taskId: 'COPILOT-ACCOUNTING-CONTRACT-TEST',
    generatedAt: '2026-11-22T02:35:00Z',
  });
  Object.assign(record, overrides);
  record.accounting_id = 'test-accounting-record-001';
  return validateRecord(record);
}

test('canonical unavailable record is canonical and parseable', () => {
  const record = baseRecord();
  const json = canonicalJson(record);
  assert.equal(canonicalJson(JSON.parse(json)), json);
  assert.match(json, /^\{"schema_version":"1"/);

  const message = attach('subject\n\nbody', record);
  const parsed = parseMessage(message);
  assert.equal(parsed.body, 'subject\n\nbody\n\nCopilot usage: gross UNAVAILABLE; discount UNAVAILABLE; net UNAVAILABLE.\nEstimated CO2e: UNAVAILABLE (method unavailable; not measured).\nAdditional budget: UNAVAILABLE remaining.');
  assert.deepEqual(parsed.record, record);
  assert.equal((message.match(new RegExp(BEGIN, 'g')) || []).length, 1);
  assert.equal((message.match(new RegExp(END, 'g')) || []).length, 1);
});

test('attach preserves an existing valid block on amend or duplicate invocation', () => {
  const record = baseRecord();
  const message = attach('', record);
  assert.equal(attach(message, record), message);
  assert.equal(attach(message, baseRecord({ checkpoint: 'end' })), message);
});

test('duplicate or malformed block layouts are rejected', () => {
  const record = baseRecord();
  const block = `${BEGIN}\n${canonicalJson(record)}\n${END}`;
  assert.throws(() => parseMessage(`${block}\n${block}`), /exactly one accounting block/);
  assert.throws(() => parseMessage(`${BEGIN}\n{}\n`), /end in exactly one accounting block/);
  assert.throws(() => parseMessage(`prefix ${BEGIN} suffix\n${canonicalJson(record)}\n${END}\n`), /exactly one accounting block/);
});

test('noncanonical JSON and unknown fields are rejected', () => {
  const record = baseRecord();
  const compact = canonicalJson(record);
  assert.throws(() => parseMessage(`subject\n\n${BEGIN}\n${compact.replace('{', '{ ')}\n${END}\n`), /invalid accounting JSON|not canonical|fields/);

  const unknown = JSON.parse(compact);
  unknown.prompt = 'forbidden';
  assert.throws(() => validateRecord(unknown), /fields do not match|privacy-unsafe field/);
});

test('money, null reasons, and human summary stay consistent', () => {
  const record = baseRecord();
  record.totals.gross = '1.00';
  record.totals.gross_reason = null;
  record.totals.discount = '0.25';
  record.totals.discount_reason = null;
  record.totals.net = '0.75';
  record.totals.net_reason = null;
  record.billing.availability = 'available';
  record.billing.reason = null;
  record.co2e = {
    kilograms: '0.15',
    kilograms_reason: null,
    classification: 'estimated-not-measured',
    method: '0.20 kg/USD approved rule',
    provenance: 'synthetic test',
  };
  const valid = validateRecord(record);
  assert.match(humanSummary(valid), /gross \$1\.00/);
  assert.match(humanSummary(valid), /discount \$0\.25/);
  assert.match(humanSummary(valid), /net \$0\.75/);

  const bad = JSON.parse(canonicalJson(valid));
  bad.totals.net = '0.7';
  bad.totals.net_reason = null;
  assert.throws(() => validateRecord(bad), /two-decimal money/);

  const badNull = JSON.parse(canonicalJson(valid));
  badNull.totals.gross = null;
  badNull.totals.gross_reason = '';
  assert.throws(() => validateRecord(badNull), /requires a reason/);
});

test('hostile shell and marker text remains literal and safe', () => {
  const record = baseRecord();
  record.relationship.task_id = 'task-$HOME-`id`-$(printf-x)-你好';
  record.relationship.staged_paths = ['file with $11.30 and `backticks`.md'];
  const valid = validateRecord(record);
  const message = attach('literal $(echo unsafe)', valid);
  assert.match(message, /task-\$HOME-`id`-\$\(printf-x\)-你好/);
  assert.match(message, /\$11\.30/);
});
