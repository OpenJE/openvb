# ./devenv/scripts/cppcheck-openvb.nix

{ pkgs, ... }: {
  scripts.cppcheck-openvb = {
    description = "Run cppcheck on openvb";
    exec = /* bash */ ''
      set -euo pipefail
      exec "${pkgs.cppcheck}/bin/cppcheck" \
        --language=c++ \
        --std=c++03 \
        --enable=warning,style,performance,portability,information \
        --inline-suppr \
        --suppressions-list=.cppcheck-suppress \
        --quiet \
        src
    '';
  };
}
