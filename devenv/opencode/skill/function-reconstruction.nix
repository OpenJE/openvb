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

    opencode-openje integration:
    When reconstructing a function, create a `re_job_create` with type `analyze_function_semantics`. Register the target via `re_function_register`. Check `re_tree` for existing summaries before starting. Submit findings via `re_worker_submit` with purpose (summary), inputs (parameters), side effects, and confidence. Submit for review via `re_review_submit` when the contract is stable.
    When dispatched as `task(category='re-analysis')`, use `analyze_function_semantics` job type.
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
