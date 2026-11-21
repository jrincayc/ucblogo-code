# Research Findings: UCB Logo MCP Server

**Status:** Complete
**Researched:** 2026-11-19 (UTC)
**Question:** Is a UCB Logo MCP server technically feasible, and what are the main integration options and constraints?
**Scope and assumptions:** Feasibility of a local developer-assistant integration. The target MCP client and desired tools are unspecified, so this covers read-only Logo references, Logo program execution, and persistent interpreter sessions. No implementation, build, or runtime test was requested or performed.

## Key Findings

- **Overall feasibility is good for a read-only reference server and reasonable for subprocess-based program execution.** A persistent, interactive Logo session is the least certain option because it adds state and lifecycle requirements (E1-E5).
- No UCB Logo MCP implementation was found in the repository. The only MCP-named dependency is `testcafe-mcp-server`, listed among development dependencies; it is not evidence of a Logo integration (E1).
- MCP's current specification supports tools over `stdio`, and the official TypeScript SDK supports Node.js with `stdio` and Streamable HTTP transports. This fits an external Node.js server alongside the existing interpreter better than mixing protocol messages with Logo's terminal output (E2-E4).
- The repository can run Logo in a Node/WebAssembly process, but that runtime has host filesystem access and `shell` works in Node. Arbitrary Logo evaluation therefore has substantially greater authority and risk than reading documentation (E3, E5).
- Current MCP tool guidance treats the protocol as stateless. A server that preserves Logo definitions and variables across calls would need explicit state identity and ownership rather than assuming a connection itself is the interpreter session (E6).

## Evidence Ledger

| ID | Finding | Type | Confidence and basis | Source and date | Planning relevance |
| --- | --- | --- | --- | --- | --- |
| E1 | No UCB Logo MCP server source was found. The only MCP-named dependency is `testcafe-mcp-server` in development dependencies. | direct observation | High; workspace search and dependency manifest. | [package.json](../../package.json), workspace search, checked 2026-11-19 | Avoid treating the TestCafe dependency as reusable Logo integration. |
| E2 | MCP defines `stdio` as newline-delimited JSON-RPC over a client-launched subprocess; the current specification also defines Streamable HTTP. | documented fact | High; primary specification. | [MCP Transport Overview, 2026-07-28](https://modelcontextprotocol.io/specification/2026-07-28/basic/transports) | Local subprocess transport is a standard option; HTTP has a separate network-security surface. |
| E3 | `main()` initializes the interpreter and then repeatedly reads Logo commands from stdin; `tests/acceptance/run-logo-node.js` launches the generated Node runtime as a child process. The WebAssembly Node target is built with `ENVIRONMENT=node` and `NODERAWFS=1`. | direct observation | High; source and build rules. | [main.c](../../main.c), [run-logo-node.js](../../tests/acceptance/run-logo-node.js), [Makefile.am](../../Makefile.am), checked 2026-11-19 | Existing subprocess execution is a concrete integration surface; it is not an MCP API. |
| E4 | Because the Logo CLI writes prompts/results to stdout while MCP stdio reserves stdout for protocol messages, directly combining both on one process stream would conflict. A wrapper or process boundary would be needed for CLI-backed tools. | inference | High; follows from E2 and E3. | [main.c](../../main.c); MCP Transports, 2026-07-28 | Impacts runtime wiring and result capture. |
| E5 | Logo file operations call `fopen`; `shell` calls `popen`, and Web Edition documentation says `shell` works in Node. The Node/Wasm build enables `NODERAWFS=1`. | documented fact | High; source/build configuration and project documentation agree. | [files.c](../../files.c), [coms.c](../../coms.c), [webedition.md](../../webedition.md), [Makefile.am](../../Makefile.am), checked 2026-11-19 | A run-code tool inherits meaningful host authority; a subprocess alone is not a sandbox. |
| E6 | The latest MCP tool guidance says MCP has no protocol-level session; cross-call state should be represented explicitly. | documented fact | High; primary specification. | [MCP Tools, 2026-07-28](https://modelcontextprotocol.io/specification/2026-07-28/server/tools) | Persistent Logo workspaces need an explicit state/lifetime model. |
| E7 | The official TypeScript SDK v2 is a current Node.js server option and documents `stdio` and Streamable HTTP support. | documented fact | High; official SDK repository and quickstart. | [TypeScript SDK README](https://github.com/modelcontextprotocol/typescript-sdk/blob/main/README.md) and [Build your first server](https://github.com/modelcontextprotocol/typescript-sdk/blob/main/docs/get-started/first-server.md), checked 2026-11-19; README is maintained on `main`. | Reduces protocol implementation burden; introduces a dependency/version choice. |
| E8 | MCP security guidance calls for input validation, access controls, rate limits, output sanitization, and human confirmation for sensitive calls; local servers run with the launching user's privileges. | documented fact | High; primary specification. | [MCP Tools](https://modelcontextprotocol.io/specification/2026-07-28/server/tools) and [Security Best Practices](https://modelcontextprotocol.io/specification/2026-07-28/basic/security_best_practices), checked 2026-11-19 | Security scope depends primarily on which Logo capabilities are exposed, not on MCP transport alone. |

## Options and Trade-offs

- **Read-only help/docs/reference tools:** high feasibility and comparatively low risk; can expose help entries, manuals, and examples without starting the interpreter. This does not evaluate or validate a submitted program.
- **Program execution through a Logo subprocess:** feasible using the native CLI or generated Node runtime. It can return captured output and exit status, but process startup, time limits, error handling, stdout separation, and host filesystem/shell authority require consideration.
- **Persistent interactive Logo workspace:** potentially supports multi-call exploration with retained definitions and variables. It has higher lifecycle and isolation complexity; current MCP semantics require explicit state identifiers, and this repository's visible entry points are command-line loops rather than a documented MCP-facing session API.
- **Local `stdio` versus Streamable HTTP:** `stdio` matches a local client-launched process and avoids opening a network listener. HTTP can support clients connecting to a shared server but brings authentication, origin, and network-boundary concerns.

## Conflicts and Counterevidence

- The existing `testcafe-mcp-server` dependency may look like prior MCP work, but the repository contains no Logo-facing server implementation or MCP tool definitions (E1).
- Existing Node/WebAssembly execution and a process-launching test helper support feasibility, but they do not establish safe multi-tenant execution or a supported persistent embedding API (E3, E5).
- `stdio` reduces network exposure for a local server; it does not reduce the operating-system privileges of the interpreter process (E5, E8).

## Constraints and Dependencies

- The official TypeScript SDK provides a Node-compatible server layer; use of that SDK would add a direct dependency not currently present as the Logo server library (E7).
- The existing Node/WebAssembly runtime is a generated executable target, built with Emscripten settings for Node and raw filesystem access. The test harness invokes it as a child process (E3, E5).
- If a network-accessible HTTP service is intended, local `stdio` assumptions no longer apply; authentication and origin/network protection become material (E2, E8).

## Unknowns and Unverified Items

- Whether the intended tools are help lookup, example search, program execution/testing, or a persistent interactive workspace.
- Which MCP host(s), protocol revision, and transport must be supported.
- Whether arbitrary Logo execution is in scope, what filesystem/shell access is acceptable, and whether a sandbox is required.
- Whether users need state to persist across tool calls, and how such state would be isolated, expired, and recovered.
- No build, MCP client connection, tool-call prototype, runtime test, or cross-platform check was performed; this is a source-and-specification feasibility assessment only.

## Search Coverage and Limitations

Inspected the repository package manifest, Makefile, native interpreter entry, file and shell primitives, Web Edition Node notes, and Node test runner. Searched for `MCP`, `Model Context Protocol`, `JSON-RPC`, `ucblogo-node`, `runLogo`, `run_evaluator`, `shell`, `popen`, and `NODERAWFS`. Consulted the official MCP 2026-07-28 transport, tools, and security specifications and the official TypeScript SDK README/quickstart. No Git history review or implementation prototype was needed for this bounded feasibility question. Search was focused, not exhaustive; target-client compatibility remains unverified.

## Planning Handoff

The main scope decision is whether the server is a documentation assistant or an execution interface. Read-only lookup has a substantially smaller authority and lifecycle surface. Execution can reuse process-based runtime entry points, but it exposes the interpreter's host capabilities unless separately constrained. A persistent workspace also needs an explicit state model under current MCP semantics. The planning stage should resolve the target client/transport, tool scope, execution permissions, and persistence requirement before choosing among these options.

## Sources

- UCB Logo repository sources cited in the Evidence Ledger; inspected 2026-11-19 UTC.
- Model Context Protocol, *Overview* (transport bindings), *Tools*, and *Security Best Practices*, protocol version 2026-07-28; official specification, versioned 2026-07-28.
- Model Context Protocol, *TypeScript SDK README* and *Build your first server*; official GitHub repository, checked 2026-11-19 UTC. The README is on the actively maintained `main` branch; the quickstart lists Node.js 20 or later.

<!-- Co-authored by GitHub Copilot. Scope: UCB Logo MCP server feasibility research. Model identity and selected reasoning-effort setting are unavailable; maximum effort was requested. Partial measured interval: 12m17s (2026-11-19 03:30:50-03:43:07 UTC, through initial report validation; excludes final citation/accounting edit and verification). Energy (kWh) and CO2: unavailable; no measured data or defensible estimation inputs available. -->