# ./devenv/opencode/skill/docs-sync-workflow.nix

{ lib, ... }:
let
  name = "docs-sync-workflow";
  description = "workflow for promoting stable reverse-engineering findings into docs/f3 while keeping confirmed findings separate from hypotheses and open questions.";
  prompt = lib.strings.removeSuffix "\n" ''
    When updating docs:
    - Preserve existing docs/f3 structure and links.
    - Prefer additive updates over broad rewrites.
    - Record confirmed findings clearly.
    - Label hypotheses and unresolved items explicitly.
    - Keep documentation useful for future implementation work.

    ## opencode-openje integration
    - When promoting stable findings to docs, first verify the function is `reviewed` in the ledger via `re_function_get` or `re_tree`. Only document functions with `reviewed` or `worker_done` status. Reference summary versions from `re_status`.
    - This skill supports all RE categories when documentation is the output artifact.
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
