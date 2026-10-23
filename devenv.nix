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
  # anyio-4.14.2 test-suite pin workaround: 7 tests fail on the pinned
  # Python 3.12 + trio, independent of our usage:
  #   - tests/streams/test_tls.py::test_tls_connectable (all 4 backends):
  #     ValueError('server_hostname can only be specified in client mode')
  #   - tests/test_contextmanagers.py::test_exception and
  #     tests/test_itertools.py::test_negative_r[trio]: ExceptionGroup of
  #     unraisable exception warnings
  #   - tests/test_taskgroups.py::test_nested_task_group_filtering[trio]:
  #     trio nursery message text differs from what the test asserts
  # Disable just those tests so the package (and its dependents: mcp,
  # httpcore, httpx, starlette, pytest-subprocess -> imhex-mcp-server)
  # builds again. Remove once the pinned nixpkgs builds anyio 4.14.2 cleanly
  # (check with `nix build nixpkgs#python312Packages.anyio`).
  #
  # The failing tests only check doc formatting, so drop that test path. Remove this
  # overlay once the pinned nixpkgs builds inline-snapshot again (check with
  # `nix build nixpkgs#python312Packages.inline-snapshot`).
  overlays = [
    (
      final: prev: {
        python312Packages = prev.python312Packages.override {
          overrides = self: super: {
            anyio = super.anyio.overridePythonAttrs (old: {
              disabledTests = (old.disabledTests or [ ]) ++ [
                "test_tls_connectable"
                "test_nested_task_group_filtering"
                "test_negative_r"
                "test_exception"
              ];
            });
            inline-snapshot = super.inline-snapshot.overridePythonAttrs (old: {
              disabledTestPaths = (old.disabledTestPaths or [ ]) ++ [ "tests/test_docs.py" ];
            });
          };
        };
      }
    )
  ];
}
