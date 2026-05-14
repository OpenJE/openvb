# ./devenv/opencode/skill/ida-callgraph-workflow.nix

{ lib, ... }:
let
  name = "ida-callgraph-workflow";
  description = "bounded ida-backed callgraph traversal workflow for reverse engineering deep call trees with cycle detection, frontier management, and summary propagation.";
  prompt = lib.strings.removeSuffix "\n" ''
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
