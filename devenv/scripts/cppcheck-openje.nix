# ./devenv/scripts/cppcheck-openje.nix

{ pkgs, ... }: {
  scripts.cppcheck-openje = {
    description = "Run cppcheck on openje";
    exec = /* bash */ ''
      set -euo pipefail
      exec "${pkgs.cppcheck}/bin/cppcheck" \
        --language=c++ \
        --std=c++03 \
        --enable=warning,style,performance,portability,information \
        --inline-suppr \
        --suppressions-list=openje/.cppcheck-suppress \
        --quiet \
        -I openje/src \
        -I tracing/src \
        openje/src
    '';
  };
}
