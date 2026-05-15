# ./devenv/opencode/plugin/default.nix

{ ... }: {
  imports = [
    ./f3-address-lookup.nix
    ./f3-class-index.nix
    ./f3-class-info.nix
    ./f3-member-index.nix
    ./f3-method-index.nix
    ./f3-vftable-index.nix
  ];
}
