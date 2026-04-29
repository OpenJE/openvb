# ./devenv/harness/skills/implementation-alignment.nix

{ lib, ... }:
let
  name = "implementation-alignment";
  description = "workflow for translating stable reverse-engineering summaries into minimal c++ changes aligned with repo conventions and existing architecture.";
  prompt = lib.strings.removeSuffix "\n" ''
    Before implementing:
    - confirm the behavior summary is stable enough
    - identify the minimal code location that should change
    - map binary behavior to existing source structures
    - preserve repo style and architecture
    - verify whether unresolved analysis should block coding
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
