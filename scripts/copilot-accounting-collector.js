#!/usr/bin/env node
'use strict';

// Co-authored by GitHub Copilot (MAI Code 2 Flash, reasoning effort unavailable).

const crypto = require('node:crypto');
const fs = require('node:fs');
const {
  canonicalJson,
  unavailableRecord,
  validateRecord,
} = require('./copilot-accounting.js');

const FORBIDDEN = new Set(['prompt', 'prompts', 'response', 'responses', 'token', 'tokens', 'credential', 'credentials', 'payment', 'card']);
const MONEY = /^(?:0|[1-9][0-9]*)\.[0-9]{2}$/;

function assertPrivacySafe(value, path = 'input') {
  if (Array.isArray(value)) {
    value.forEach((item, index) => assertPrivacySafe(item, `${path}[${index}]`));
    return;
  }
  if (!value || typeof value !== 'object') return;
  for (const [key, item] of Object.entries(value)) {
    if (FORBIDDEN.has(key.toLowerCase())) {
      throw new Error(`privacy-unsafe field: ${path}.${key}`);
    }
    assertPrivacySafe(item, `${path}.${key}`);
  }
}

function normalizeMoney(value, reason, label) {
  if (value === null || value === undefined) {
    if (typeof reason !== 'string' || !reason.trim()) {
      throw new Error(`${label} requires a reason when unavailable`);
    }
    return { value: null, reason };
  }
  let normalized;
  if (typeof value === 'number' && Number.isFinite(value) && value >= 0) {
    normalized = value.toFixed(2);
  } else if (typeof value === 'string' && MONEY.test(value)) {
    normalized = value;
  } else {
    throw new Error(`${label} must be a nonnegative two-decimal value`);
  }
  return { value: normalized, reason: null };
}

function normalizeInteger(value, reason, label) {
  if (value === null || value === undefined) {
    return { value: null, reason: typeof reason === 'string' && reason.trim() ? reason : `${label} unavailable` };
  }
  if (!Number.isInteger(value) || value < 0) throw new Error(`${label} must be a nonnegative integer or null`);
  return { value, reason: null };
}

function normalizeRollup(row, unavailableReason) {
  const gross = normalizeMoney(row.gross, row.grossReason || row.gross_reason || unavailableReason, 'rollup gross');
  const discount = normalizeMoney(row.discount, row.discountReason || row.discount_reason || unavailableReason, 'rollup discount');
  const net = normalizeMoney(row.net, row.netReason || row.net_reason || unavailableReason, 'rollup net');
  return {
    product: String(row.product || 'unknown'),
    sku: String(row.sku || 'unknown'),
    quantity: String(row.quantity ?? '0'),
    unit: String(row.unit || 'unknown'),
    gross: gross.value,
    gross_reason: gross.reason,
    discount: discount.value,
    discount_reason: discount.reason,
    net: net.value,
    net_reason: net.reason,
  };
}

function normalizeSnapshot(input, options = {}) {
  assertPrivacySafe(input);
  if (!input || typeof input !== 'object' || Array.isArray(input)) throw new Error('collector input must be an object');

  const base = unavailableRecord({
    root: options.root,
    taskId: options.taskId || input.taskId || 'unknown-task',
    editorName: options.editorName || input.editorName || 'GitHub Copilot',
    model: options.model || input.model || 'unknown',
    reasoningEffort: options.reasoningEffort || input.reasoningEffort || 'unavailable',
    githubLogin: options.githubLogin || input.githubLogin || 'unknown',
    billingSource: options.billingSource || input.billingSource || 'unavailable',
    generatedAt: options.generatedAt || input.generatedAt,
  });

  const unavailableReason = input.reason || 'billing data unavailable at commit time';
  const totalsInput = input.totals || {};
  const gross = normalizeMoney(totalsInput.gross ?? input.gross, totalsInput.grossReason || totalsInput.gross_reason || input.grossReason || unavailableReason, 'totals gross');
  const discount = normalizeMoney(totalsInput.discount ?? input.discount, totalsInput.discountReason || totalsInput.discount_reason || input.discountReason || unavailableReason, 'totals discount');
  const net = normalizeMoney(totalsInput.net ?? input.net, totalsInput.netReason || totalsInput.net_reason || input.netReason || unavailableReason, 'totals net');

  const billingInput = input.billing || {};
  const hasMeasuredValue = gross.value !== null || discount.value !== null || net.value !== null;
  const availability = billingInput.availability || (hasMeasuredValue ? 'available' : 'unavailable');
  const provenance = input.provenance || (billingInput.source === 'user-supplied' ? 'user-supplied' : 'measured');

  const includedInput = input.includedCredits || input.included_credits || {};
  const baseline = normalizeInteger(includedInput.baseline, includedInput.reason || unavailableReason, 'includedCredits.baseline');
  const used = normalizeInteger(includedInput.used, includedInput.reason || unavailableReason, 'includedCredits.used');
  const remaining = normalizeInteger(includedInput.remaining, includedInput.reason || unavailableReason, 'includedCredits.remaining');
  const resetAt = includedInput.resetAt || includedInput.reset_at || null;

  const budgetInput = input.additionalBudget || input.additional_budget || {};
  const spent = normalizeMoney(budgetInput.spent, budgetInput.spentReason || budgetInput.spent_reason || unavailableReason, 'additionalBudget.spent');
  const limit = normalizeMoney(budgetInput.limit, budgetInput.limitReason || budgetInput.limit_reason || unavailableReason, 'additionalBudget.limit');
  const remainingBudget = normalizeMoney(budgetInput.remaining, budgetInput.remainingReason || budgetInput.remaining_reason || unavailableReason, 'additionalBudget.remaining');

  const productRows = input.productSkuTotals || input.product_sku_totals || [];
  const modelRows = input.modelTotals || input.model_totals || [];
  if (!Array.isArray(productRows) || !Array.isArray(modelRows)) throw new Error('product and model totals must be arrays');

  const co2e = net.value === null
    ? {
        kilograms: null,
        kilograms_reason: unavailableReason,
        classification: 'unavailable',
        method: 'approved net-dollar rule 0.20 kg/USD; input unavailable',
        provenance: 'unavailable',
      }
    : {
        kilograms: (Number(net.value) * 0.20).toFixed(2),
        kilograms_reason: null,
        classification: 'estimated-not-measured',
        method: '0.20 kg/USD approved rule',
        provenance,
      };

  const relationshipInput = input.relationship || {};
  const stagedPaths = relationshipInput.staged_paths || relationshipInput.stagedPaths || base.relationship.staged_paths;
  const issues = relationshipInput.issues || [];

  const record = {
    ...base,
    checkpoint: options.checkpoint || input.checkpoint || base.checkpoint,
    editor: {
      name: input.editor?.name || options.editorName || 'GitHub Copilot',
      model: input.editor?.model || options.model || 'unknown',
      reasoning_effort: input.editor?.reasoning_effort || options.reasoningEffort || 'unavailable',
    },
    account: {
      github_login: input.account?.github_login || options.githubLogin || 'unknown',
      billing_source: input.account?.billing_source || billingInput.source || options.billingSource || 'unavailable',
    },
    generated_at: input.generatedAt || base.generated_at,
    reporting_window: {
      since: input.reportingWindow?.since || input.reporting_window?.since || null,
      until: input.reportingWindow?.until || input.reporting_window?.until || base.generated_at,
      provenance,
      provenance_reason: unavailableReason,
    },
    billing: {
      availability,
      reason: availability === 'available' ? null : (billingInput.reason || unavailableReason),
      source: billingInput.source || provenance,
      as_of: billingInput.asOf || billingInput.as_of || null,
    },
    totals: {
      gross: gross.value,
      gross_reason: gross.reason,
      discount: discount.value,
      discount_reason: discount.reason,
      net: net.value,
      net_reason: net.reason,
    },
    included_credits: {
      baseline: baseline.value,
      used: used.value,
      remaining: remaining.value,
      reset_at: resetAt,
      reason: baseline.reason && used.reason && remaining.reason ? (includedInput.reason || unavailableReason) : null,
    },
    additional_budget: {
      spent: spent.value,
      spent_reason: spent.reason,
      limit: limit.value,
      limit_reason: limit.reason,
      remaining: remainingBudget.value,
      remaining_reason: remainingBudget.reason,
      effective_at: budgetInput.effectiveAt || budgetInput.effective_at || null,
    },
    product_sku_totals: productRows.map((row) => normalizeRollup(row, unavailableReason)),
    model_totals: modelRows.map((row) => normalizeRollup(row, unavailableReason)),
    co2e,
    relationship: {
      task_id: relationshipInput.task_id || relationshipInput.taskId || base.relationship.task_id,
      staged_paths: Array.isArray(stagedPaths) ? stagedPaths.map(String) : [],
      pr: relationshipInput.pr ?? null,
      issues: Array.isArray(issues) ? issues.map(Number).filter(Number.isInteger) : [],
    },
  };

  record.accounting_id = crypto.createHash('sha256').update(canonicalJson({ ...record, accounting_id: 'pending' })).digest('hex');
  return validateRecord(record);
}

function readCollectorFile(file, options = {}) {
  let input;
  try {
    input = JSON.parse(fs.readFileSync(file, 'utf8'));
  } catch (error) {
    throw new Error(`cannot read collector input ${file}: ${error.message}`);
  }
  return normalizeSnapshot(input, options);
}

if (require.main === module) {
  try {
    const file = process.argv[2];
    if (!file) throw new Error('usage: copilot-accounting-collector.js INPUT.json');
    process.stdout.write(`${canonicalJson(readCollectorFile(file))}\n`);
  } catch (error) {
    process.stderr.write(`${error.message}\n`);
    process.exitCode = 1;
  }
}

module.exports = { assertPrivacySafe, normalizeMoney, normalizeSnapshot, readCollectorFile };
