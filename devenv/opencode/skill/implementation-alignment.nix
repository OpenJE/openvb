# ./devenv/opencode/skill/implementation-alignment.nix

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

    ## opencode-openje integration
    Before translating summaries to C++, verify the source function has `reviewed` status via `re_function_get`. Create a `re_job_create` with type `emit_faithful_cpp`. After implementation, submit via `re_worker_submit`. If compilation succeeds, update status via `re_function_set_status`. If compilation fails, create a `fix_compile_error` job.
    When dispatched as `task(category='re-synthesis')`, use `emit_faithful_cpp` job type.
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
