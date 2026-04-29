# ./devenv/tasks/build.nix

{ ... }: {
  tasks = {
    "build:msvc".exec = "msvc-build";
    "build:msvc-debug".exec = "msvc-build-debug";
    "build:msvc-tracing".exec = "msvc-build-tracing";
  };
}
