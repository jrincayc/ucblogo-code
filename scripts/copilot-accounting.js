#!/usr/bin/env node
'use strict';

const crypto = require('node:crypto');
const fs = require('node:fs');
const { spawnSync } = require('node:child_process');

const BEGIN = 'COPILOT-ACCOUNTING-BEGIN v1';
const END = 'COPILOT-ACCOUNTING-END';
const MONEY = /^(?:0|[1-9][0-9]*)\.[0-9]{2}$/;
const UTC = /^\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}Z$/;
const TOP_KEYS = ['schema_version', 'accounting_id', 'checkpoint', 'editor', 'account', 'generated_at', 'parent_revision', 'reporting_window', 'billing', 'totals', 'included_credits', 'additional_budget', 'product_sku_totals', 'model_totals', 'co2e', 'relationship'];
const CHECKPOINTS = new Set(['start', 'pre-commit', 'andon', 'significant-event', 'end', 'implementation']);
const AVAILABILITY = new Set(['available', 'partial', 'unavailable', 'blocked']);
const FORBIDDEN = new Set(['prompt', 'prompts', 'response', 'responses', 'token', 'tokens', 'credential', 'credentials', 'payment', 'card']);

function fail(message) {
  throw new Error(message);
}

function own(value, key) {
  return Object.prototype.hasOwnProperty.call(value, key);
}

function exactKeys(value, keys, label) {
  if (!value || typeof value !== 'object' || Array.isArray(value)) fail(`${label} must be an object`);
  const actual = Object.keys(value).sort();
  const expected = [...keys].sort();
  if (JSON.stringify(actual) !== JSON.stringify(expected)) fail(`${label} fields do not match the v1 schema`);
}

function money(value, label) {
  if (typeof value !== 'string' || !MONEY.test(value)) fail(`${label} must be a two-decimal money string`);
}

function nullableMoney(value, reason, label) {
  if (value === null) {
    if (typeof reason !== 'string' || !reason.trim()) fail(`${label} requires a reason when null`);
    return;
  }
  money(value, label);
  if (reason !== null) fail(`${label} reason must be null when value is available`);
}

function safeText(value, label, max = 512) {
  if (typeof value !== 'string' || value.length === 0 || value.length > max) fail(`${label} must be nonempty bounded UTF-8 text`);
  if (/[\x00-\x1f\x7f]/.test(value)) fail(`${label} must not contain control characters`);
  if (Buffer.from(value, 'utf8').toString('utf8') !== value) fail(`${label} must be valid UTF-8 text`);
}

function validateRollup(rows, label) {
  if (!Array.isArray(rows) || rows.length > 1000) fail(`${label} must be an array`);
  for (const row of rows) {
    exactKeys(row, ['product', 'sku', 'quantity', 'unit', 'gross', 'gross_reason', 'discount', 'discount_reason', 'net', 'net_reason'], `${label} row`);
    safeText(row.product, `${label}.product`);
    safeText(row.sku, `${label}.sku`);
    safeText(row.quantity, `${label}.quantity`, 128);
    safeText(row.unit, `${label}.unit`, 64);
    nullableMoney(row.gross, row.gross_reason, `${label}.gross`);
    nullableMoney(row.discount, row.discount_reason, `${label}.discount`);
    nullableMoney(row.net, row.net_reason, `${label}.net`);
  }
}

function canonicalize(value) {
  exactKeys(value, TOP_KEYS, 'record');
  exactKeys(value.editor, ['name', 'model', 'reasoning_effort'], 'editor');
  exactKeys(value.account, ['github_login', 'billing_source'], 'account');
  exactKeys(value.reporting_window, ['since', 'until', 'provenance', 'provenance_reason'], 'reporting_window');
  exactKeys(value.billing, ['availability', 'reason', 'source', 'as_of'], 'billing');
  exactKeys(value.totals, ['gross', 'gross_reason', 'discount', 'discount_reason', 'net', 'net_reason'], 'totals');
  exactKeys(value.included_credits, ['baseline', 'used', 'remaining', 'reset_at', 'reason'], 'included_credits');
  exactKeys(value.additional_budget, ['spent', 'spent_reason', 'limit', 'limit_reason', 'remaining', 'remaining_reason', 'effective_at'], 'additional_budget');
  exactKeys(value.co2e, ['kilograms', 'kilograms_reason', 'classification', 'method', 'provenance'], 'co2e');
  exactKeys(value.relationship, ['task_id', 'staged_paths', 'pr', 'issues'], 'relationship');
  const rollupKeys = ['product', 'sku', 'quantity', 'unit', 'gross', 'gross_reason', 'discount', 'discount_reason', 'net', 'net_reason'];
  const orderedRollup = (rows) => rows.map((row) => {
    exactKeys(row, rollupKeys, 'rollup row');
    const ordered = {};
    for (const key of rollupKeys) ordered[key] = row[key];
    return ordered;
  });
  const ordered = {};
  for (const key of TOP_KEYS) {
    if (key === 'editor') ordered.editor = { name: value.editor.name, model: value.editor.model, reasoning_effort: value.editor.reasoning_effort };
    else if (key === 'account') ordered.account = { github_login: value.account.github_login, billing_source: value.account.billing_source };
    else if (key === 'reporting_window') ordered.reporting_window = { since: value.reporting_window.since, until: value.reporting_window.until, provenance: value.reporting_window.provenance, provenance_reason: value.reporting_window.provenance_reason };
    else if (key === 'billing') ordered.billing = { availability: value.billing.availability, reason: value.billing.reason, source: value.billing.source, as_of: value.billing.as_of };
    else if (key === 'totals') ordered.totals = { gross: value.totals.gross, gross_reason: value.totals.gross_reason, discount: value.totals.discount, discount_reason: value.totals.discount_reason, net: value.totals.net, net_reason: value.totals.net_reason };
    else if (key === 'included_credits') ordered.included_credits = { baseline: value.included_credits.baseline, used: value.included_credits.used, remaining: value.included_credits.remaining, reset_at: value.included_credits.reset_at, reason: value.included_credits.reason };
    else if (key === 'additional_budget') ordered.additional_budget = { spent: value.additional_budget.spent, spent_reason: value.additional_budget.spent_reason, limit: value.additional_budget.limit, limit_reason: value.additional_budget.limit_reason, remaining: value.additional_budget.remaining, remaining_reason: value.additional_budget.remaining_reason, effective_at: value.additional_budget.effective_at };
    else if (key === 'product_sku_totals') ordered.product_sku_totals = orderedRollup(value.product_sku_totals);
    else if (key === 'model_totals') ordered.model_totals = orderedRollup(value.model_totals);
    else if (key === 'co2e') ordered.co2e = { kilograms: value.co2e.kilograms, kilograms_reason: value.co2e.kilograms_reason, classification: value.co2e.classification, method: value.co2e.method, provenance: value.co2e.provenance };
    else if (key === 'relationship') ordered.relationship = { task_id: value.relationship.task_id, staged_paths: [...value.relationship.staged_paths], pr: value.relationship.pr, issues: [...value.relationship.issues] };
    else ordered[key] = value[key];
  }
  return ordered;
}

function validateRecord(record) {
  const value = canonicalize(record);
  if (value.schema_version !== '1') fail('schema_version must be 1');
  safeText(value.accounting_id, 'accounting_id', 128);
  if (!CHECKPOINTS.has(value.checkpoint)) fail('checkpoint is not supported');
  for (const [key, label] of [['name', 'editor.name'], ['model', 'editor.model'], ['reasoning_effort', 'editor.reasoning_effort'], ['github_login', 'account.github_login'], ['billing_source', 'account.billing_source']]) {
    if (value.editor[key] !== undefined) safeText(value.editor[key], label, 256);
    else if (value.account[key] !== undefined) safeText(value.account[key], label, 256);
  }
  if (!UTC.test(value.generated_at) || Number.isNaN(Date.parse(value.generated_at))) fail('generated_at must be an exact UTC timestamp');
  safeText(value.parent_revision, 'parent_revision', 64);
  exactKeys(value.reporting_window, ['since', 'until', 'provenance', 'provenance_reason'], 'reporting_window');
  for (const key of ['since', 'until']) if (value.reporting_window[key] !== null && (!UTC.test(value.reporting_window[key]) || Number.isNaN(Date.parse(value.reporting_window[key])))) fail(`reporting_window.${key} is invalid`);
  safeText(value.reporting_window.provenance, 'reporting_window.provenance');
  if (value.reporting_window.provenance_reason !== null) safeText(value.reporting_window.provenance_reason, 'reporting_window.provenance_reason');
  exactKeys(value.billing, ['availability', 'reason', 'source', 'as_of'], 'billing');
  if (!AVAILABILITY.has(value.billing.availability)) fail('billing availability is not supported');
  if (value.billing.reason !== null) safeText(value.billing.reason, 'billing.reason');
  safeText(value.billing.source, 'billing.source');
  if (value.billing.as_of !== null && (!UTC.test(value.billing.as_of) || Number.isNaN(Date.parse(value.billing.as_of)))) fail('billing.as_of is invalid');
  exactKeys(value.totals, ['gross', 'gross_reason', 'discount', 'discount_reason', 'net', 'net_reason'], 'totals');
  nullableMoney(value.totals.gross, value.totals.gross_reason, 'totals.gross');
  nullableMoney(value.totals.discount, value.totals.discount_reason, 'totals.discount');
  nullableMoney(value.totals.net, value.totals.net_reason, 'totals.net');
  exactKeys(value.included_credits, ['baseline', 'used', 'remaining', 'reset_at', 'reason'], 'included_credits');
  for (const key of ['baseline', 'used', 'remaining']) if (value.included_credits[key] !== null && (!Number.isInteger(value.included_credits[key]) || value.included_credits[key] < 0)) fail(`included_credits.${key} must be null or a nonnegative integer`);
  if (value.included_credits.reset_at !== null && !/^\d{4}-\d{2}-\d{2}$/.test(value.included_credits.reset_at)) fail('included_credits.reset_at is invalid');
  if (value.included_credits.reason !== null) safeText(value.included_credits.reason, 'included_credits.reason');
  exactKeys(value.additional_budget, ['spent', 'spent_reason', 'limit', 'limit_reason', 'remaining', 'remaining_reason', 'effective_at'], 'additional_budget');
  nullableMoney(value.additional_budget.spent, value.additional_budget.spent_reason, 'additional_budget.spent');
  nullableMoney(value.additional_budget.limit, value.additional_budget.limit_reason, 'additional_budget.limit');
  nullableMoney(value.additional_budget.remaining, value.additional_budget.remaining_reason, 'additional_budget.remaining');
  if (value.additional_budget.effective_at !== null && !/^\d{4}-\d{2}-\d{2}$/.test(value.additional_budget.effective_at)) fail('additional_budget.effective_at is invalid');
  validateRollup(value.product_sku_totals, 'product_sku_totals');
  validateRollup(value.model_totals, 'model_totals');
  exactKeys(value.co2e, ['kilograms', 'kilograms_reason', 'classification', 'method', 'provenance'], 'co2e');
  if (value.co2e.kilograms !== null) {
    if (typeof value.co2e.kilograms !== 'string' || !MONEY.test(value.co2e.kilograms)) fail('co2e.kilograms must be a two-decimal decimal string or null');
  } else if (typeof value.co2e.kilograms_reason !== 'string' || !value.co2e.kilograms_reason.trim()) fail('co2e.kilograms requires a reason when null');
  if (value.co2e.kilograms_reason !== null) safeText(value.co2e.kilograms_reason, 'co2e.kilograms_reason');
  if (value.co2e.classification !== 'estimated-not-measured' && value.co2e.classification !== 'unavailable') fail('co2e.classification is invalid');
  safeText(value.co2e.method, 'co2e.method');
  safeText(value.co2e.provenance, 'co2e.provenance');
  exactKeys(value.relationship, ['task_id', 'staged_paths', 'pr', 'issues'], 'relationship');
  safeText(value.relationship.task_id, 'relationship.task_id', 256);
  if (!Array.isArray(value.relationship.staged_paths) || value.relationship.staged_paths.length > 10000) fail('relationship.staged_paths must be an array');
  for (const item of value.relationship.staged_paths) safeText(item, 'relationship.staged_paths item', 1024);
  if (value.relationship.pr !== null && (!Number.isInteger(value.relationship.pr) || value.relationship.pr < 1)) fail('relationship.pr must be null or a positive integer');
  if (!Array.isArray(value.relationship.issues) || value.relationship.issues.some((item) => !Number.isInteger(item) || item < 1)) fail('relationship.issues must be an integer array');

  assertNoForbiddenKeys(value);
  return value;
}

function assertNoForbiddenKeys(value, path = 'record') {
  if (Array.isArray(value)) return value.forEach((item, index) => assertNoForbiddenKeys(item, `${path}[${index}]`));
  if (!value || typeof value !== 'object') return;
  for (const [key, item] of Object.entries(value)) {
    if (FORBIDDEN.has(key.toLowerCase())) fail(`privacy-unsafe field: ${path}.${key}`);
    assertNoForbiddenKeys(item, `${path}.${key}`);
  }
}

function canonicalJson(record) {
  return JSON.stringify(validateRecord(record));
}

function parseMessage(message) {
  if (typeof message !== 'string') fail('commit message must be text');
  const lines = message.split(/\r?\n/);
  const begin = lines.reduce((count, line, index) => line === BEGIN ? [...count, index] : count, []);
  const end = lines.reduce((count, line, index) => line === END ? [...count, index] : count, []);
  if (!begin.length && !end.length) return { body: message.replace(/\s+$/, ''), record: null };
  if (begin.length !== 1 || end.length !== 1 || begin[0] >= end[0] || end[0] !== lines.length - 1 && !(end[0] === lines.length - 2 && lines[lines.length - 1] === '')) fail('commit message must end in exactly one accounting block');
  const json = lines.slice(begin[0] + 1, end[0]).join('\n');
  let parsed;
  try { parsed = JSON.parse(json); } catch (error) { fail(`invalid accounting JSON: ${error.message}`); }
  if (canonicalJson(parsed) !== json) fail('accounting JSON is not canonical');
  const body = lines.slice(0, begin[0]).join('\n').replace(/\s+$/, '');
  return { body, record: parsed };
}

function humanSummary(record) {
  const totals = record.totals;
  const display = (value) => value === null ? 'UNAVAILABLE' : `$${value}`;
  const lines = [`Copilot usage: gross ${display(totals.gross)}; discount ${display(totals.discount)}; net ${display(totals.net)}.`];
  lines.push(record.co2e.kilograms === null
    ? 'Estimated CO2e: UNAVAILABLE (method unavailable; not measured).'
    : `Estimated CO2e: ${record.co2e.kilograms} kg (${record.co2e.method}; estimated/not measured).`);
  lines.push(`Additional budget: ${record.additional_budget.remaining === null ? 'UNAVAILABLE remaining' : `$${record.additional_budget.remaining} remaining`}.`);
  return lines.join('\n');
}

function attach(message, record) {
  const parsed = parseMessage(message);
  if (parsed.record) {
    return message.endsWith('\n') ? message : `${message}\n`;
  }
  const body = parsed.body ? `${parsed.body}\n\n` : '';
  return `${body}${humanSummary(record)}\n\n${BEGIN}\n${canonicalJson(record)}\n${END}\n`;
}

function git(args, options = {}) {
  const result = spawnSync('git', args, { encoding: 'utf8', ...options });
  if (result.status !== 0) fail((result.stderr || result.stdout || 'git command failed').toString().trim());
  return Buffer.isBuffer(result.stdout) ? result.stdout : result.stdout.trim();
}

function repositoryRoot(cwd = process.cwd()) {
  return git(['-C', cwd, 'rev-parse', '--show-toplevel']);
}

function unavailableRecord(options = {}) {
  const root = options.root || repositoryRoot();
  const head = spawnSync('git', ['-C', root, 'rev-parse', '--verify', 'HEAD'], { encoding: 'utf8' });
  const parent = head.status === 0 ? head.stdout.trim() : '0000000000000000000000000000000000000000';
  const generatedAt = (options.generatedAt || new Date().toISOString()).replace(/\.\d{3}Z$/, 'Z');
  const stagedBuffer = git(['-C', root, 'diff', '--cached', '--name-only', '-z'], { encoding: null });
  const paths = Buffer.from(stagedBuffer || '').toString('utf8').split('\0').filter(Boolean);
  const reason = options.reason || 'billing data unavailable at commit time';
  const base = {
    schema_version: '1',
    accounting_id: '',
    checkpoint: options.checkpoint || 'pre-commit',
    editor: { name: options.editorName || 'unknown', model: options.model || 'unknown', reasoning_effort: options.reasoningEffort || 'unverified' },
    account: { github_login: options.githubLogin || 'unknown', billing_source: options.billingSource || 'unavailable' },
    generated_at: generatedAt,
    parent_revision: parent,
    reporting_window: { since: null, until: generatedAt, provenance: 'commit-time snapshot', provenance_reason: reason },
    billing: { availability: 'unavailable', reason, source: 'unavailable', as_of: null },
    totals: { gross: null, gross_reason: reason, discount: null, discount_reason: reason, net: null, net_reason: reason },
    included_credits: { baseline: options.includedBaseline ?? null, used: options.includedUsed ?? null, remaining: options.includedRemaining ?? null, reset_at: options.resetAt || null, reason },
    additional_budget: { spent: options.additionalSpent ?? null, spent_reason: options.additionalSpent === undefined ? reason : null, limit: options.additionalLimit ?? null, limit_reason: options.additionalLimit === undefined ? reason : null, remaining: options.additionalRemaining ?? null, remaining_reason: options.additionalRemaining === undefined ? reason : null, effective_at: null },
    product_sku_totals: [],
    model_totals: [],
    co2e: { kilograms: null, kilograms_reason: reason, classification: 'unavailable', method: 'approved net-dollar rule 0.20 kg/USD; input unavailable', provenance: 'unavailable' },
    relationship: { task_id: options.taskId || 'unknown-task', staged_paths: paths, pr: null, issues: [] },
  };
  base.accounting_id = crypto.createHash('sha256').update(canonicalJson({ ...base, accounting_id: 'pending' })).digest('hex');
  return validateRecord(base);
}

function readRecord(file) {
  let record;
  try { record = JSON.parse(fs.readFileSync(file, 'utf8')); } catch (error) { fail(`cannot read accounting candidate ${file}: ${error.message}`); }
  return validateRecord(record);
}

module.exports = { BEGIN, END, attach, canonicalJson, git, humanSummary, parseMessage, readRecord, repositoryRoot, unavailableRecord, validateRecord };

if (require.main === module) {
  try {
    const file = process.argv[2];
    const record = file ? readRecord(file) : unavailableRecord();
    process.stdout.write(`${canonicalJson(record)}\n`);
  } catch (error) {
    process.stderr.write(`${error.message}\n`);
    process.exitCode = 1;
  }
}
