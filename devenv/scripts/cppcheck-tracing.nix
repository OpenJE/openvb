# ./devenv/scripts/cppcheck-tracing.nix

{ pkgs, ... }: {
  scripts.cppcheck-tracing = {
    description = "Run cppcheck on tracing";
    exec = /* bash */ ''
      set -euo pipefail
      exec "${pkgs.cppcheck}/bin/cppcheck" \
        --language=c++ \
        --std=c++03 \
        --enable=warning,style,performance,portability,information \
        --inline-suppr \
        --suppressions-list=tracing/.cppcheck-suppress \
        --quiet \
        -I tracing/src \
        tracing/src
    '';
  };
}
