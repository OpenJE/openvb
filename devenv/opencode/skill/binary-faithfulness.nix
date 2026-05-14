# ./devenv/opencode/skill/binary-faithfulness.nix

{ lib, ... }:
let
  name = "binary-faithfulness";
  description = "rules for keeping analysis, documentation, and implementation aligned with observed binary behavior rather than aesthetic rewrites.";
  prompt = lib.strings.removeSuffix "\n" ''
    Prefer binary-faithful behavior over cleanup-oriented rewrites.

    Rules:
    - Do not treat nicer design as evidence.
    - Preserve observed side effects, call order, and behavior when known.
    - Avoid implementing speculative semantics as confirmed behavior.
    - When certainty is low, record the uncertainty instead of flattening it away.

    opencode-openje integration:
    - When analyzing a function, register it via `re_function_register` with status `discovered`. If the analysis reveals a behavioral finding that changes confidence, update via `re_function_set_status` or `re_worker_submit` with evidence. Always track progress via `re_status`.
    - When dispatched as `task(category='re-analysis')`, assume the job type is `analyze_function_semantics` or `analyze_function_types`.
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
