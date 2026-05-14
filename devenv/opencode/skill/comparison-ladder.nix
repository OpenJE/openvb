# ./devenv/opencode/skill/comparison-ladder.nix

{ lib, ... }:
let
  name = "comparison-ladder";
  description = "escalation workflow for comparing the original Van Buren demo against the rebuilt reimplementation from surface metadata up through runtime checkpoints.";
  prompt = lib.strings.removeSuffix "\n" ''
    Use this comparison order by default:
    1. Binary surface: file size, sections, imports, strings, and obvious metadata.
    2. Structural evidence: functions, xrefs, callees, basic blocks, and control-flow shape.
    3. Type and data-shape evidence: signatures, stack frames, globals, and structures.
    4. Pseudocode and semantic summaries.
    5. Runtime checkpoints and debugger snapshots.

    Rules:
    - Start at the cheapest level that can answer the question.
    - Escalate only when lower-fidelity evidence is insufficient or contradictory.
    - Report the first known divergence point when possible.
    - Return `match`, `partial match`, `mismatch`, or `inconclusive` explicitly.
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
