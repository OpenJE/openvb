# ./devenv/languages/cplusplus.nix

{ pkgs, ... }: {
  languages.cplusplus = {
    enable = true;
    lsp = {
      enable = true;
      package = pkgs.clang-tools;
    };
  };
}
