# ./devenv/opencode/skill/type-recovery.nix

{ lib, ... }:
let
  name = "type-recovery";
  description = "conservative workflow for recovering function signatures, stack variables, structs, enums, and field meanings from ida evidence.";
  prompt = lib.strings.removeSuffix "\n" ''
    Use this skill when type uncertainty blocks behavioral understanding.

    Rules:
    - Prefer minimal, evidence-backed type changes.
    - Use stack frame, type, struct, and xref evidence.
    - Distinguish safe assertions from tentative guesses.
    - Record any unresolved ambiguity instead of forcing a type.

    opencode-openje integration:
    - When recovering types, create a `re_job_create` with type `analyze_function_types`. Register the target function via `re_function_register` with status `analyzing`. Submit type findings via `re_worker_submit` with inputs (recovered parameters) and purpose (type inference summary). Use `re_review_submit` for type verdicts.
    - When dispatched as `task(category='re-analysis')`, use `analyze_function_types` job type.
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
