# ./devenv/opencode/skill/ida-callgraph-workflow.nix

{ lib, ... }:
let
  name = "ida-callgraph-workflow";
  description = "bounded ida-backed callgraph traversal workflow for reverse engineering deep call trees with cycle detection, frontier management, and summary propagation.";
  prompt = lib.strings.removeSuffix "\n" ''
    This workflow uses opencode-openje as the single source of truth for traversal state. All function registration, edge discovery, progress tracking, and summary submission goes through the re_* tool interface.

    Use this workflow when reverse engineering a function through its callees.

    Core model:
    - Treat analysis as graph traversal, not uncontrolled recursive delegation.
    - Use IDA MCP as the primary evidence source.
    - Reuse prior summaries instead of re-analyzing known nodes.

    Required state per target:
    - target function or SCC
    - visited set
    - active path
    - depth budget
    - fanout budget
    - stable summaries
    - unresolved frontier

    Traversal rules:
    1. Fetch direct evidence for the current function.
    2. If already summarized, reuse the summary.
    3. If the function is already in the active path, stop and mark a cycle boundary.
    4. Descend only into relevant unresolved callees.
    5. Collapse mutually recursive groups into one SCC analysis unit when needed.
    6. Propagate child summaries upward once stable.

    Verification loop:
    - Produce a tentative summary.
    - Verify it against decompilation, disassembly, xrefs, and type evidence.
    - Refine only when new evidence or child summaries justify it.
    - Prefer bounded refinement over endless re-analysis.

    Stopping conditions:
    - stable summary reached
    - cycle boundary detected
    - known utility boundary reached
    - depth/fanout budget exhausted
    - unresolved blocker identified

    Job Lifecycle (opencode-openje integration):

    Before starting traversal:
    1. Call re_function_register(target_ea, status="discovered") for the root.
    2. Call re_job_create(job_type="discover_subgraph", target=target_ea) to claim the work.
    3. Call re_job_next(role) to claim the job atomically.

    During traversal (for each callee):
    4. Call re_function_register(callee_ea, status="discovered") for new functions.
    5. Call re_edge_add(caller, callee, kind, blocking) for discovered edges.
    6. Set callee status to re_function_set_status(callee_ea, "queued") when scheduling.
    7. If callee already has status "worker_done" or "reviewed", reuse summary via re_function_get — do NOT re-analyze.

    For SCC detection:
    8. Create re_job_create(job_type="analyze_scc_cluster", target=scc_root) for cycle groups.

    After analysis:
    9. Submit worker output via re_worker_submit(function_ea, role, model, {purpose, inputs, confidence, evidence}).
    10. Optionally submit for review via re_review_submit.

    Cycle detection:
    - When re_function_get shows a function already in "analyzing" status, mark as cycle boundary.
    - Use re_stale_mark_parents when child summaries change.

    Status flow:
    discovered → queued → analyzing → worker_done → review_pending → reviewed

    Category → Job Type mapping:
    - re-discovery: discover_subgraph, classify_edges, analyze_scc_cluster
    - re-analysis: analyze_function_semantics, analyze_function_types, analyze_function_names
    - re-review: review_function_contract
    - re-synthesis: emit_faithful_cpp, review_cpp_fidelity
    - re-ops: fix_compile_error, apply_ida_patch_plan

    Database Targeting:
    Always use database-targeting skill conventions when opening IDA databases for evidence gathering.
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    name: ${name}
    description: ${description}
  '';
in {
  opencode.skills.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}
