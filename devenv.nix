# ./devenv.nix

{ ... }: {
  imports = [
    ./devenv
  ];

  # nixpkgs pin workaround: python3.12-inline-snapshot 0.34.2 runs tests/test_docs.py
  # in its install-check. That test re-formats the docs code snippets with black and
  # compares them against the checked-in snapshots, which black >= 26 wraps differently
  # (dict literals + docstrings). tests/test_docs.py::{testing,categories,code_generation}
  # fail, so the package never builds and it takes down rich-toolkit -> fastapi-cli -> fastapi -> mcp ->
  # imhex-mcp-server -> devenv-profile -> devenv-shell.
  #
  # The failing tests only check doc formatting, so drop that test path. Remove this
  # overlay once the pinned nixpkgs builds inline-snapshot again (check with
  # `nix build nixpkgs#python312Packages.inline-snapshot`).
  overlays = [
    (
      final: prev: {
        python312Packages = prev.python312Packages.override {
          overrides = self: super: {
            inline-snapshot = super.inline-snapshot.overridePythonAttrs (old: {
              disabledTestPaths = (old.disabledTestPaths or [ ]) ++ [ "tests/test_docs.py" ];
            });
          };
        };
      }
    )
  ];
}
