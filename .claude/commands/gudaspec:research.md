---
name: GudaSpec Research
description: Transform user requirements into constraint sets through parallel exploration and analysis
category: GudaSpec
tags: [gudaspec, research, constraints, exploration, subagents]
---

<!-- GUDASPEC:RESEARCH:START -->
**Core Philosophy**
- Research produces **constraint sets**, not information dumps. Each constraint narrows the solution space.
- Constraints tell subsequent stages "don't consider this direction," enabling mechanical execution without decisions.
- The output is "約束集合 + 可验证的成功判据" (constraint sets + verifiable success criteria).
- Eliminate ambiguity through structured exploration and user interaction.
- Strictly adhere to **OpenSpec** rules when writing **standardized spec-structured projects**.

**Guardrails**
- **NEVER** divide subagent tasks by roles (e.g., "架构师agent", "安全专家agent"). 
- **ALWAYS** divide by context boundaries (e.g., "user-related code", "authentication logic", "infrastructure config").
- Each subagent context must be self-contained with independent output.
- **MANDATORY**: Use `mcp__auggie-mcp__codebase-retrieval` to minimize search/grep/find operations.
- All subagents must follow the same structured output template for aggregation.
- Do not make architectural decisions—surface constraints that guide decisions.

**Steps**
0. **Generate OpenSpec Proposal**
   - Run `/openspec:proposal`

1. **Initial Codebase Assessment**
   - Combine user requirements with quick codebase scan using `mcp__auggie-mcp__codebase-retrieval`.
   - Determine project scale: single directory vs. multi-directory structure.
   - **Decision point**: If code spans multiple subdirectories or modules → enable parallel Explore subagents.
   - Document finding: "Single agent serial exploration is inefficient; parallel subagents required."

2. **Define Exploration Boundaries (Context-Based Division)**
   - Identify natural context boundaries in the codebase (NOT functional roles).
   - Example divisions:
     * Subagent 1: User domain code (user models, user services, user UI)
     * Subagent 2: Authentication & authorization code (auth middleware, session, tokens)
     * Subagent 3: Configuration & infrastructure (configs, deployments, build scripts)
   - Each boundary should be self-contained: no cross-communication needed between subagents.
   - Define exploration scope and expected output for each subagent.

3. **Prepare Standardized Output Template**
   - Define a unified JSON schema that all Explore subagents must follow:
   ```json
   {
     "module_name": "string - context boundary explored",
     "existing_structures": ["list of key structures/patterns found"],
     "existing_conventions": ["list of conventions/standards in use"],
     "constraints_discovered": ["list of hard constraints that limit solution space"],
     "open_questions": ["list of ambiguities requiring user input"],
     "dependencies": ["list of dependencies on other modules/systems"],
     "risks": ["list of potential risks or blockers"],
     "success_criteria_hints": ["observable behaviors that indicate success"]
   }
   ```
   - Communicate template to all subagents for consistency.

4. **Parallel Subagent Dispatch**
   - For each defined context boundary, spawn an Explore subagent with:
     * **Explicit instruction**: "You MUST use `mcp__auggie-mcp__codebase-retrieval` to reduce search operations."
     * Defined scope and context boundary.
     * Required output template (from Step 3).
     * Clear success criteria: complete analysis of assigned boundary.
   - Monitor subagent execution and collect structured reports.

5. **Aggregate and Synthesize Reports**
   - Collect all subagent JSON outputs.
   - Merge findings into unified constraint sets:
     * **Hard constraints**: Technical limitations, existing patterns that cannot be violated.
     * **Soft constraints**: Conventions, preferences, style guides.
     * **Dependencies**: Cross-module relationships that affect implementation order.
     * **Risks**: Potential blockers that need mitigation.
   - Identify **open questions** from all reports that require user clarification.
   - Synthesize **success criteria** from scenario hints across all contexts.

6. **User Interaction for Ambiguity Resolution**
   - Compile prioritized list of open questions from aggregated reports.
   - Use `AskUserQuestions` tool to present questions systematically:
     * Group related questions together.
     * Provide context for each question.
     * Suggest default answers when applicable.
   - Capture user responses as additional constraints.
   - Update constraint sets with confirmed decisions.

7. **Generate OpenSpec Proposal**
   - Run `/openspec:proposal <requirement-description>` to scaffold formal spec.
   - Transform finalized constraint sets into requirements format.
   - Structure as:
     * **Context**: User need + discovered constraints
     * **Requirements**: Each constraint becomes a requirement with scenario
     * **Success Criteria**: Derived from aggregated hints and user confirmations
   - Ensure proposal includes:
     * All discovered constraints as requirements.
     * Verifiable scenarios for each requirement.
     * Clear dependencies and sequencing.
     * Risk mitigation strategies.

**Reference**
- Review existing constraints: `rg -n "Constraint:|MUST|MUST NOT" openspec/specs`
- Inspect codebase structure: `ls -R` or `mcp__auggie-mcp__codebase-retrieval`
- Check prior research outputs: `ls openspec/changes/*/research-output.md`
- Validate subagent outputs conform to template before aggregation.
- Use `AskUserQuestions` for ANY ambiguity—do not assume or guess.

<!-- GUDASPEC:RESEARCH:END -->
