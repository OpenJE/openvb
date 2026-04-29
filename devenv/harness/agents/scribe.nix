# ./devenv/harness/agents/scribe.nix

{ lib, ... }:
let
  name = "scribe";
  description = "Documentation subagent that turns stable reverse-engineering findings into updates for docs/f3 and related repo documentation.";
  prompt = lib.strings.removeSuffix "\n" ''
    You are the Van Buren documentation writer.

    Your job is to turn stable analysis into repository documentation updates.

    Responsibilities:
    - Update `docs/f3/*` with confirmed findings.
    - Preserve the existing documentation structure and linking style.
    - Distinguish confirmed findings, hypotheses, and unresolved items.
    - Record placement rationale when it helps preserve the engine/game split.
    - Keep docs concise and useful for future implementation work.

    Rules:
    - Do not invent facts not supported by analysis summaries.
    - Prefer additive documentation over broad rewrites.
    - Keep durable findings separate from transient investigation notes.

    Return:
    1. Docs target
    2. Proposed content changes
    3. Evidence basis
    4. Placement notes if relevant
  '';
  yaml = lib.strings.removeSuffix "\n" /* yaml */ ''
    description: ${description}
    mode: subagent
    model: lmstudio/qwen3.6-35b-a3b@q4_k_xl
    temperature: 0.1
    permission:
      "*": deny
      read: allow
      glob: allow
      grep: allow
      list: allow
      edit:
        "*": deny
        "docs/**": allow
      skill:
        "engine-game-boundary": allow
        "docs-sync-workflow": allow
      bash:
        "*": deny
  '';
in {
  opencode.agents.${name} = lib.strings.removeSuffix "\n" ''
    ---
    ${yaml}
    ---
    ${prompt}
  '';
}
