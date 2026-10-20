# ./devenv/scripts/cppcheck-tracing.nix

{ pkgs, ... }: {
  scripts.cppcheck-tracing = {
    description = "Run cppcheck on modules/tracing (no args = every src file; or pass paths)";
    exec = /* bash */ ''
      set -euo pipefail

      # Same relative-path requirement as cppcheck-openje: the suppression entries are keyed by
      # tracing-relative paths, so the tool has to run from modules/. The old script hard-coded
      # that and died with `cppcheck: error: couldn't open the file:
      # "tracing/.cppcheck-suppress"` when run from the repo root - which is easy to miss because
      # callers that pipe the output measure the pipe's exit status, not cppcheck's.
      if [ ! -d "tracing/src" ]; then
        if [ -d "modules/tracing/src" ]; then
          cd modules
        else
          printf 'cppcheck-tracing: run me from the repo root (or from modules/).\n' >&2
          exit 1
        fi
      fi

      files=()
      for a in "$@"; do
        case "$a" in
          -h|--help)
            printf 'usage: cppcheck-tracing [path ...]\n'
            printf '  no args : every tracing/src/**.cpp (the module is small, so it sweeps)\n'
            printf '  paths    : repo-root-relative or modules/-relative\n'
            exit 0 ;;
        esac
        a="''${a#./}"
        a="''${a#modules/}"
        files+=( "$a" )
      done
      if [ ''${#files[@]} -eq 0 ]; then
        files=( $(cd tracing && git ls-files 'src/*.cpp' | sed 's|^|tracing/|') )
        if [ ''${#files[@]} -eq 0 ]; then
          files=( $(cd tracing && find src -name '*.cpp' | sed 's|^|tracing/|') )
        fi
      fi

      exec "${pkgs.cppcheck}/bin/cppcheck" \
        --language=c++ \
        --std=c++03 \
        --enable=warning,style,performance,portability,information \
        --inline-suppr \
        --suppressions-list=tracing/.cppcheck-suppress \
        --quiet \
        -I tracing/src \
        "''${files[@]}"
    '';
  };
}
