# ./devenv/tasks/check.nix

{ config, ... }: {
  tasks = {
    "check:cppcheck".exec =
      "${config.scripts.cppcheck-openvb.scriptPackage}/bin/cppcheck-openvb";
    "check:cppcheck-openje".exec =
      "${config.scripts.cppcheck-openje.scriptPackage}/bin/cppcheck-openje";
    "check:cppcheck-tracing".exec =
      "${config.scripts.cppcheck-tracing.scriptPackage}/bin/cppcheck-tracing";
    "check:cppcheck-all".exec =
      "${config.scripts.cppcheck-all.scriptPackage}/bin/cppcheck-all";
  };
}
