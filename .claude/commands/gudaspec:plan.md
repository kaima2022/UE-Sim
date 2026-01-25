---
name: GudaSpec: Plan
description: Refine proposals into zero-decision executable task flows via multi-model analysis.
category: GudaSpec
tags: [gudaspec, plan, multi-model, pbt]
allowed-tools: Bash(openspec:*), mcp__codex__codex, mcp__gemini__gemini
argument-hint: [proposal_id]
---
<!-- GUDASPEC:START -->
**Guardrails**
- Strictly adhere to **OpenSpec** rules when writing **standardized spec-structured projects**.
- The goal of this phase is to eliminate ALL decision points from the task flow—implementation should be pure mechanical execution.
- Do not proceed to implementation until every ambiguity is resolved and every constraint is explicitly documented.
- Multi-model collaboration is mandatory: use both `mcp__codex__codex` and `mcp__gemini__gemini` to surface blind spots and conflicting assumptions.
- Every requirement must have Property-Based Testing (PBT) properties defined—focus on invariants, not just example-based tests.
- If constraints cannot be fully specified, escalate back to the user or return to the research phase rather than making assumptions.
- Refer to `openspec/AGENTS.md` for additional conventions; run `openspec update` if the file is missing.

**Steps**
1. Run `openspec view` to display all **Active Changes**, then confirm with the user which proposal ID (`<proposal_id>`) they wish to refine into a zero-decision plan.

2. Run `/openspec:proposal <proposal_id>`.

3. **Multi-Model Implementation Analysis**: Invoke both MCP tools to provide diverse implementation perspectives:
   ```
   mcp__codex__codex: "Analyze proposal <proposal_id>: Provide implementation approach, identify technical risks, and suggest alternative architectures. Focus on edge cases and failure modes."
   
   mcp__gemini__gemini: "Analyze proposal <proposal_id>: Evaluate from maintainability, scalability, and integration perspectives. Highlight potential conflicts with existing systems."
   ```
   Synthesize responses and present consolidated options to the user for constraint selection.

4. **Multi-Model Uncertainty Elimination Audit**: Invoke both MCP tools to detect remaining ambiguities:
   ```
   mcp__codex__codex: "Review proposal <proposal_id> for decision points that remain unspecified. List each as: [AMBIGUITY] <description> → [REQUIRED CONSTRAINT] <what must be decided>."
   
   mcp__gemini__gemini: "Identify implicit assumptions in proposal <proposal_id>. For each assumption, specify: [ASSUMPTION] <description> → [EXPLICIT CONSTRAINT NEEDED] <concrete specification>."
   ```
   
   **Anti-Pattern Detection** (flag and reject):
   - Information collection without decision boundaries (e.g., "JWT vs OAuth2 vs session—all viable")
   - Technical comparisons without selection criteria
   - Deferred decisions marked as "to be determined during implementation"
   
   **Target Pattern** (required for approval):
   - Explicit technology choices with parameters (e.g., "JWT with accessToken TTL=15min, refreshToken TTL=7days")
   - Concrete algorithm selections with configurations (e.g., "bcrypt with cost factor=12")
   - Precise behavioral rules (e.g., "Lock account for 30min after 5 failed login attempts")
   
   Iterate with user until ALL ambiguities are resolved into explicit constraints.

5. **Multi-Model PBT Property Extraction**: Invoke both MCP tools to derive testable invariants:
   ```
   mcp__codex__codex: "Extract Property-Based Testing properties from proposal <proposal_id>. For each requirement, identify: [INVARIANT] <mathematical property that must always hold> → [FALSIFICATION STRATEGY] <how to generate test cases that attempt to break it>."
   
   mcp__gemini__gemini: "Analyze proposal <proposal_id> for system properties. Define: [PROPERTY] <name> | [DEFINITION] <formal description> | [BOUNDARY CONDITIONS] <edge cases to test> | [COUNTEREXAMPLE GENERATION] <approach>."
   ```
   
   **PBT Property Categories to Extract**:
   - **Commutativity/Associativity**: Order-independent operations
   - **Idempotency**: Repeated operations yield same result
   - **Round-trip**: Encode→Decode returns original
   - **Invariant Preservation**: State constraints maintained across operations
   - **Monotonicity**: Ordering guarantees (e.g., timestamps always increase)
   - **Bounds**: Value ranges, size limits, rate constraints

**Reference**
- Use `openspec show <id> --json --deltas-only` to inspect proposal structure when validation fails.
- Use `openspec list --specs` to check for conflicts with existing specifications.
- Search existing patterns with `rg -n "INVARIANT:|PROPERTY:|Constraint:" openspec/` before defining new ones.
- If MCP tools are unavailable, run `/mcp` to check connection status and authenticate if needed.
- For complex proposals, consider running steps 2-4 iteratively on sub-components rather than the entire proposal at once.
- Use `AskUserQuestions` for ANY ambiguity—do not assume or guess.

**Exit Criteria**
A proposal is ready to exit the Plan phase only when:
- [ ] All multi-model analyses completed and synthesized
- [ ] Zero ambiguities remain (verified by step 3 audit)
- [ ] All PBT properties documented with falsification strategies
- [ ] `openspec validate <id> --strict` returns zero issues
- [ ] User has explicitly approved all constraint decisions
<!-- GUDASPEC:END -->
