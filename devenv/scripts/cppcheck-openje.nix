# ./devenv/scripts/cppcheck-openje.nix

{ pkgs, ... }: {
  scripts.cppcheck-openje = {
    description = "Run cppcheck on openje (no args = all src/JE/**.cpp changed vs HEAD; or pass paths relative to modules/)";
    exec = /* bash */ ''
      set -euo pipefail

      # cppcheck must run from modules/ with openje-relative paths: the suppression list entries
      # are keyed by those paths (e.g. dangerousTypeCast:openje/src/JE/Map.cpp), so running from
      # the repo root silently disables them. The old script pointed at an openje/ symlink that
      # no longer exists (modules/openje is a real submodule now).
      if [ ! -d "openje/src/JE" ]; then
        if [ -d "modules/openje/src/JE" ]; then
          cd modules
        else
          printf 'cppcheck-openje: run me from the repo root (or from modules/).\n' >&2
          exit 1
        fi
      fi

      # Normalise arguments to modules/-relative paths so both "modules/openje/src/JE/X.cpp"
      # (what you'd paste from the repo root) and "openje/src/JE/X.cpp" work. Do NOT use
      # "''${@/#modules//}": bash reads that as pattern="modules", replacement="/".
      files=()
      for a in "$@"; do
        a="''${a#./}"
        a="''${a#modules/}"
        files+=( "$a" )
      done
      if [ ''${#files[@]} -eq 0 ]; then
        # Default: existing .cpp files under src/JE that differ from HEAD, incl. untracked new ones.
        # `|| true` matters: with set -euo pipefail a grep that matches nothing (clean tree) would
        # abort the whole script instead of reaching the "nothing changed" branch below.
        changed="$(
          cd openje
          {
            git diff --name-only HEAD -- src/JE
            git ls-files --others --exclude-standard -- src/JE
          } 2>/dev/null | grep -E '\.cpp$' | sort -u | while read -r rel; do
            [ -f "$rel" ] && printf 'openje/%s\n' "$rel"
          done || true
        )"
        if [ -z "$changed" ]; then
          printf 'cppcheck-openje: nothing changed vs HEAD under openje/src/JE (pass paths to force).\n'
          exit 0
        fi
        files=( $changed )
        printf 'cppcheck-openje: %d file(s) changed vs HEAD\n' ''${#files[@]}
      fi

      # --error-exitcode is deliberately NOT set: matches cppcheck-openvb (report, don't gate).
      exec "${pkgs.cppcheck}/bin/cppcheck" \
        --language=c++ \
        --std=c++03 \
        --enable=warning,style \
        --inline-suppr \
        --suppressions-list=openje/.cppcheck-suppress \
        --quiet \
        -I openje/src \
        -I tracing/src \
        "''${files[@]}"
    '';
  };
}
