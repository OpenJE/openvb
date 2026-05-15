# ./devenv/languages/nix.nix

{ pkgs, ... }: {
  languages.nix = {
    enable = true;
    lsp.enable = true;
  };

  packages = with pkgs; [
    nil
  ];
}
