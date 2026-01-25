---
name: GudaSpec: Implementation
description: Execute approved OpenSpec changes via multi-model collaboration with Codex/Gemini.
category: GudaSpec
tags: [openspec, implementation, multi-model, codex, gemini]
---
<!-- GUDASPEC:START -->
**Guardrails**
- Never apply external model prototypes directly—all Codex/Gemini outputs serve as reference only and must be rewritten into readable, maintainable, production-grade code.
- Keep changes tightly scoped to the requested outcome; enforce side-effect review before applying any modification.
- Minimize documentation—avoid unnecessary comments; prefer self-explanatory code.
- Refer to `openspec/AGENTS.md` (located inside the `openspec/` directory—run `ls openspec` or `openspec update` if you don't see it) for additional OpenSpec conventions or clarifications.

**Steps**
Track these steps as TODOs and complete them one by one.

1. Run `openspec view` to inspect current project status and review `Active Changes`; ask the user to confirm which proposal ID they want to implement and wait for explicit confirmation before proceeding.
2. Run `/openspec:apply <proposal_id>`.
3. Work through tasks sequentially as defined in `tasks.md`; but Don't complete all tasks in `tasks.md` at once. Just identify **the smallest verifiable phase** that can be quickly accomplished this time, and strictly control the context window for a single implementation.
4. Analyze each task in `tasks.md` and route to the appropriate model kernel:
   - **Route A: Gemini Kernel** — for frontend/UI/styling tasks (CSS, React, Vue, HTML, component design).
   - **Route B: Codex Kernel** — for backend/logic/algorithm tasks (API, data processing, business logic, debugging).
   - **Mandatory constraint**: When communicating with Codex/Gemini, the prompt **must explicitly require** returning a `Unified Diff Patch` only; external models are strictly forbidden from making any real file modifications.
5. Upon receiving the diff patch from Codex/Gemini, **never apply it directly**; rewrite the prototype by removing redundancy, ensuring clear naming and simple structure, aligning with project style, and eliminating unnecessary comments.
6. Before applying any change, perform a mandatory side-effect review: verify the change does not exceed `tasks.md` scope, does not affect unrelated modules, and does not introduce new dependencies or break existing interfaces; make targeted corrections if issues are found.
7. After completing the task, run **multi-model review** to verify.
8. After corrections are completed, mark the task item as `- [x]` in `tasks.md`.
9. Once all tasks are confirmed complete, run `/openspec:archive`.
<!-- GUDASPEC:END -->
