# ./devenv/tasks/run.nix

{ ... }: {
  tasks = {
    "run:f3".exec = "f3-run";
    "run:f3-debug".exec = "f3-run-debug";
    "run:f3-lockstep-debug".exec = "f3-lockstep-debug";
    "run:f3-reimpl".exec = "f3-reimpl-run";
    "run:f3-reimpl-debug".exec = "f3-reimpl-run-debug";
    "run:f3-reimpl-trace".exec = "f3-reimpl-run-trace";
    "run:f3demo".exec = "f3demo-run";
    "run:f3demo-debug".exec = "f3demo-run-debug";
  };
}
