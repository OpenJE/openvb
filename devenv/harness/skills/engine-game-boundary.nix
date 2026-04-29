# ./devenv/harness/skills/engine-game-boundary.nix

{ lib, ... }:
let
  name = "engine-game-boundary";
  description = "workflow for deciding whether recovered behavior belongs in openje, src/F3, or a boundary adapter, using existing IDA structure as strong but overridable prior evidence.";
  prompt = lib.strings.removeSuffix "\n" ''
    Use this skill whenever you need to decide where recovered behavior should live in the repository.

    Repository split:
    - `openje/` is the engine/runtime/shared systems side.
    - `src/F3/` is the Van Buren game logic side.
    - Some behavior belongs at a boundary layer where game code drives engine primitives.

    Evidence priority for placement:
    1. Existing curated IDA state: class ownership, methods, members, types, comments, and prior naming.
    2. Fresh IDA evidence: decompilation, disassembly, xrefs, callees, callers, and type usage.
    3. Repository structure and docs relationships.

    Trust model:
    - Treat existing IDA names/types/comments/class ownership as strong prior evidence.
    - Do not treat them as unquestionable truth.
    - Prefer refinement over replacement unless new evidence clearly contradicts the existing interpretation.

    Placement classes:
    - `engine`: reusable runtime/framework/shared systems
    - `game`: Van Buren-specific gameplay and content semantics
    - `boundary`: adapters/glue between engine and game layers
    - `unknown`: not stable enough to place safely

    Conservative policy:
    - If placement is not strongly supported, prefer `boundary` or `unknown`.
    - Do not auto-implement across the engine/game split while placement remains uncertain.

    Return:
    1. Placement guess
    2. Placement confidence
    3. Evidence and prior-IDA-state rationale
    4. Candidate repo target
    5. Remaining uncertainty
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
