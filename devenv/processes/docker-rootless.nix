# ./devenv/processes/docker-rootless.nix

{ config, pkgs, ... }: {
  processes.docker-rootless = {
    exec = /* bash */ ''
      set -euo pipefail

      uid="$(id -u)"
      socket="/run/user/$uid/docker.sock"
      dockerCli="${pkgs.docker}/bin/docker"
      log="${config.git.root}/.devenv/state/dockerd-rootless.log"

      # T-046 (04:36Z dockerd outage, harness items 32-33). The old script nohup+disowned
      # dockerd and ended in a bare `tail -f /dev/null`: when dockerd died, the leader stayed
      # alive and this process kept reporting READY while nothing could build. Two changes:
      #   * every probe pins DOCKER_HOST to THIS uid's socket, so a foreign (rootful or
      #     other-uid) daemon can never answer for ours;
      #   * the script stays wired to the daemon - it watches an adopted daemon, or `wait`s
      #     on the tree it started, and exits nonzero when dockerd goes away so the
      #     supervisor sees the death instead of a masked one. Measured in the outage:
      #     dockerd's exit took rootlesskit and slirp4netns down with it; only the old
      #     script's `tail` survived, which is exactly why the generation looked healthy.
      daemon_up() {
        [ -S "$socket" ] && DOCKER_HOST="unix://$socket" "$dockerCli" info >/dev/null 2>&1
      }

      # One-shot run-path probe (NOT part of the periodic ready.exec): `docker info` can be
      # true while dockerd->containerd->runc is broken, and a build needs the run path. Uses
      # the MSVC toolchain image when it is cached; never pulls - readiness must not depend
      # on registry reachability (that inversion is harness item 33, fixed in msvc-build).
      run_probe() {
        local image="''${MSVC_IMAGE:-ghcr.io/openje/msvc2002:latest}"
        if ! DOCKER_HOST="unix://$socket" "$dockerCli" image inspect "$image" >/dev/null 2>&1; then
          printf 'docker-rootless: run probe skipped (no cached %s)\n' "$image"
          return 0
        fi
        if DOCKER_HOST="unix://$socket" "$dockerCli" run --rm "$image" -lc true >/dev/null 2>&1; then
          printf 'docker-rootless: run probe OK (%s)\n' "$image"
        else
          printf 'docker-rootless: run probe FAILED - daemon answers info but cannot start a container on %s\n' "$socket" >&2
          return 1
        fi
      }

      if daemon_up; then
        printf '%s\n' "Docker daemon already running (socket: $socket)."
        run_probe || exit 1
        while daemon_up; do sleep 2; done
        printf 'docker-rootless: dockerd stopped answering on %s; exiting nonzero for the supervisor.\n' "$socket" >&2
        exit 1
      fi

      printf '%s\n' "Starting dockerd-rootless..."

      nohup ${pkgs.docker}/bin/dockerd-rootless > "$log" 2>&1 &
      daemon_pid=$!

      for i in $(seq 1 60); do
        if daemon_up; then break; fi
        if ! kill -0 "$daemon_pid" 2>/dev/null; then break; fi
        sleep 0.5
      done

      if ! daemon_up; then
        printf '%s\n' "dockerd-rootless failed to start (socket not answering: $socket; see $log)." >&2
        exit 1
      fi

      run_probe || exit 1

      rc=0
      wait "$daemon_pid" || rc=$?
      printf 'docker-rootless: dockerd tree exited (rc=%s); handing off to the supervisor.\n' "$rc" >&2
      [ "$rc" -ne 0 ] || rc=1
      exit "$rc"
    '';

    cwd = config.git.root;

    # Owner-aware readiness. The old probe was a bare `docker info`, which the CLI is happy
    # to answer from ANY daemon reachable from the poller's environment - it reported ready
    # against a dead generation twice during the outage. Each failure prints which case it
    # is (no socket vs socket-without-daemon) so the no-daemon case is detected distinctly.
    ready.exec = /* bash */ ''
      uid="$(id -u)"
      socket="/run/user/$uid/docker.sock"
      dockerCli="${pkgs.docker}/bin/docker"
      if [ ! -S "$socket" ]; then
        printf 'NOT ready: no socket at %s\n' "$socket" >&2
        exit 1
      fi
      if ! DOCKER_HOST="unix://$socket" "$dockerCli" info >/dev/null 2>&1; then
        printf 'NOT ready: socket present but no daemon answers on %s (stale/dead dockerd - foreign daemons do not count). Recover: devenv processes restart docker-rootless\n' "$socket" >&2
        exit 1
      fi
      echo "OK"
    '';
  };
}
