#!/usr/bin/env node
'use strict';

const { spawnSync } = require('node:child_process');
const { parseMessage } = require('./copilot-accounting.js');

function runGit(repo, args) {
  const result = spawnSync('git', ['-C', repo, ...args], { encoding: 'utf8' });
  if (result.status !== 0) throw new Error((result.stderr || result.stdout || 'git command failed').trim());
  return result.stdout;
}

function commits(repo, range, since, until) {
  const args = ['log', '--format=%H%x00%cI%x00%B%x00%x1e'];
  if (since) args.push(`--since=${since}`);
  if (until) args.push(`--until=${until}`);
  if (range) args.push(range);
  const raw = runGit(repo, args);
  const records = [];
  for (const chunk of raw.split('\x1e')) {
    const normalized = chunk.replace(/^\r?\n/, '').replace(/\r?\n$/, '').replace(/\0$/, '');
    if (!normalized.trim()) continue;
    const fields = normalized.split('\0');
    if (fields.length < 3) continue;
    const [hash, timestamp, ...bodyParts] = fields;
    const body = bodyParts.join('\0').replace(/\r?\n$/, '');
    if (!hash || !timestamp) continue;
    const parsed = parseMessage(body);
    records.push({ hash, timestamp, subject: parsed.body.split('\n')[0] || '(no subject)', record: parsed.record });
  }
  return records;
}

function decimal(value) {
  return value === null ? null : Number(value).toFixed(2);
}

function aggregate(entries) {
  const seen = new Set();
  const valid = [];
  const unavailable = [];
  for (const entry of entries) {
    if (!entry.record) { unavailable.push(entry); continue; }
    if (!seen.has(entry.record.accounting_id)) {
      seen.add(entry.record.accounting_id);
      valid.push(entry);
    }
  }
  const sum = (field) => {
    if (entries.some((entry) => !entry.record || entry.record.totals[field] === null)) return null;
    return [...valid].reduce((total, entry) => total + Number(entry.record.totals[field]), 0).toFixed(2);
  };
  return { valid, unavailable, gross: sum('gross'), discount: sum('discount'), net: sum('net') };
}

function formatSummary(result, mode = 'pr') {
  const display = (value) => value === null ? 'UNAVAILABLE' : `$${decimal(value)}`;
  const totals = `Commits: ${result.valid.length} valid; ${result.unavailable.length} unavailable.\nNet usage: ${display(result.net)}.\n`;
  if (mode === 'issue') return `${totals}Estimated CO2e: ${result.net === null ? 'UNAVAILABLE' : `${(Number(result.net) * 0.20).toFixed(2)} kg`} (approved estimate; not measured).`;
  const rollups = new Map();
  for (const entry of result.valid) {
    for (const row of entry.record.product_sku_totals) {
      const key = `${row.product}\0${row.sku}\0${row.unit}`;
      if (!rollups.has(key)) rollups.set(key, { ...row, gross: 0, discount: 0, net: 0, quantity: 0 });
      const target = rollups.get(key);
      for (const key2 of ['gross', 'discount', 'net', 'quantity']) target[key2] += Number(row[key2] || 0);
    }
  }
  const lines = [totals.trimEnd(), `Gross: ${display(result.gross)}; discount: ${display(result.discount)}; net: ${display(result.net)}.`];
  for (const row of rollups.values()) lines.push(`${row.product} | ${row.sku} | ${row.quantity} ${row.unit} | gross $${row.gross.toFixed(2)} | discount $${row.discount.toFixed(2)} | net $${row.net.toFixed(2)}`);
  lines.push(`Estimated CO2e: ${result.net === null ? 'UNAVAILABLE' : `${(Number(result.net) * 0.20).toFixed(2)} kg`} (0.20 kg/USD approved estimate; not measured).`);
  lines.push('Caveat: account-level totals are temporal attribution, not proof that a commit caused the usage. Unknown values remain unavailable.');
  return lines.join('\n');
}

function parseArgs(argv) {
  const options = { repo: process.cwd(), mode: 'pr' };
  for (let index = 0; index < argv.length; index += 1) {
    const arg = argv[index];
    if (arg === '--repo') options.repo = argv[++index];
    else if (arg === '--range') options.range = argv[++index];
    else if (arg === '--since') options.since = argv[++index];
    else if (arg === '--until') options.until = argv[++index];
    else if (arg === '--issue') options.mode = 'issue';
    else if (arg === '--help') options.help = true;
    else throw new Error(`unknown argument: ${arg}`);
  }
  return options;
}

function main(argv) {
  const options = parseArgs(argv);
  if (options.help) {
    process.stdout.write('Usage: report-agent-usage-v2.js [--repo PATH] [--range A..B] [--since DATE] [--until DATE] [--issue]\n');
    return 0;
  }
  const result = aggregate(commits(options.repo, options.range, options.since, options.until));
  process.stdout.write(`${formatSummary(result, options.mode)}\n`);
  return 0;
}

if (require.main === module) {
  try { process.exitCode = main(process.argv.slice(2)); }
  catch (error) { process.stderr.write(`${error.message}\n`); process.exitCode = 1; }
}

module.exports = { aggregate, commits, formatSummary, parseArgs, runGit };
