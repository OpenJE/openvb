# ./devenv/processes/docker-rootless.nix

{ config, pkgs, ... }: {
  processes.docker-rootless = {
    exec = /* bash */ ''
      set -euo pipefail

      uid="$(id -u)"
      socket="/run/user/$uid/docker.sock"

      if docker info >/dev/null 2>&1; then
        printf '%s\n' "Docker daemon already running (socket: $socket)."
        tail -f /dev/null
      fi

      printf '%s\n' "Starting dockerd-rootless..."

      nohup ${pkgs.docker}/bin/dockerd-rootless \
        > "${config.git.root}/.devenv/state/dockerd-rootless.log" 2>&1 &
      disown

      for i in $(seq 1 60); do
        if [ -S "$socket" ]; then
          break
        fi
        sleep 0.5
      done

      if ! [ -S "$socket" ]; then
        printf '%s\n' "dockerd-rootless failed to start (socket not found)." >&2
        exit 1
      fi

      tail -f /dev/null
    '';

    cwd = config.git.root;

    ready.exec = ''
      docker info >/dev/null 2>&1 && echo "OK" || exit 1
    '';
  };
}
