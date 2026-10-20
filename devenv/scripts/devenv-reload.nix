# ./devenv/scripts/devenv-reload.nix

{ pkgs, ... }: {
  scripts.devenv-reload = {
    description = "Capture the current devenv environment to a file, verify it, and print the source line (devenv scripts do not hot-reload into tool shells)";
    exec = /* bash */ ''
      set -uo pipefail

      # Why this exists. A harness/tool shell keeps the PATH it was given when the session
      # started, so a script I edit in devenv/scripts/ is NOT visible until the environment is
      # re-captured and sourced. Doing that by hand went wrong twice on 2026-10-20:
      #   * `devenv print-dev-env > env.sh 2>&1` merges devenv's progress chatter
      #     ("Validating lock", "evaluating file ...") into the file, which corrupts the export
      #     lines; sourcing it then silently keeps the OLD PATH, i.e. the OLD installed script.
      #   * the resulting failures look exactly like engine link errors
      #     (kind=undefined-reference), so a green tree reads as 4 red suites.
      # So: keep stderr OUT of the capture, then prove freshness instead of assuming it.
      #
      # usage: devenv-reload [--out FILE] [--check NAME]
      #   --check NAME  additionally assert that NAME resolves inside the captured env
      #                 (default: run-all-tests, cppcheck-openje, msvc-build)

      out="/tmp/openvb-dev-env.sh"
      checks=("run-all-tests" "cppcheck-openje" "msvc-build")
      while [ $# -gt 0 ]; do
        case "$1" in
          --out) out="''${2:-/tmp/openvb-dev-env.sh}"; shift 2 ;;
          --check) checks+=( "$2" ); shift 2 ;;
          -h|--help)
            printf 'usage: devenv-reload [--out FILE] [--check NAME]\n'
            printf '  writes FILE (default /tmp/openvb-dev-env.sh), verifies it, prints the\n'
            printf '  "source FILE" line to run. Never use 2>&1 with print-dev-env.\n'
            exit 0 ;;
          *) printf 'devenv-reload: unknown argument: %s\n' "$1" >&2; exit 2 ;;
        esac
      done

      err="''${out}.err"
      # stderr deliberately NOT merged into the capture.
      if ! devenv print-dev-env > "$out" 2>"$err"; then
        printf 'devenv-reload: print-dev-env failed (see %s)\n' "$err" >&2
        tail -5 "$err" >&2
        exit 1
      fi

      # The capture must actually be shell code: it has to assign PATH and must not start with
      # devenv's progress output.
      if ! grep -q '^export PATH=' "$out" && ! grep -q 'PATH=' "$out"; then
        printf 'devenv-reload: %s has no PATH assignment - not a usable env capture\n' "$out" >&2
        head -3 "$out" >&2
        exit 1
      fi
      if head -1 "$out" | grep -qE 'Validating lock|evaluating file|✓|•'; then
        printf 'devenv-reload: %s starts with devenv progress chatter, refusing to recommend it\n' "$out" >&2
        exit 1
      fi

      # Freshness proof: every checked tool must be present in the captured PATH, and the payload
      # they resolve to must carry the current suite table. Reusing run-all-tests' own guard text
      # keeps this honest without duplicating the table.
      rc=0
      for name in "''${checks[@]}"; do
        if ! grep -qE "/nix/store/[^:\"]*-$name(/bin)?" "$out"; then
          printf 'devenv-reload: %s is NOT in the captured env\n' "$name" >&2
          rc=1
        fi
      done

      printf 'captured %s (%s bytes)\n' "$out" "$(wc -c < "$out" | tr -d ' ')"
      printf 'for each shell you use from now on, run:\n'
      printf '  source %s\n' "$out"
      printf 'then verify: command -v run-all-tests\n'
      [ $rc -eq 0 ] || printf 'devenv-reload: freshness checks FAILED, do not trust gate output yet\n' >&2
      exit $rc
    '';
  };
}
