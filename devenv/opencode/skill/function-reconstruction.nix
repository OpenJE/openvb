# ./devenv/opencode/skill/function-reconstruction.nix

{ lib, ... }:
let
  name = "function-reconstruction";
  description = "method for reconstructing one function's behavior from ida evidence, child summaries, and repository context without overstating confidence.";
  prompt = lib.strings.removeSuffix "\n" ''
    For a single function or SCC, produce:
    - purpose summary
    - key branches
    - side effects
    - globals touched
    - important callees
    - unresolved dependencies
    - confidence level

    Evidence priority:
    1. decompilation
    2. disassembly
    3. xrefs / callers / callees
    4. stack and type evidence
    5. repo source/docs for comparison only

    Always separate:
    - confirmed findings
    - hypotheses
    - blockers
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
