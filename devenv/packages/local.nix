# ./devenv/packages/local.nix

{ pkgs, ... }:

let
  py311 = pkgs.python311Packages;
  py312 = pkgs.python312Packages;

  _imhex-mcp-src = pkgs.fetchFromGitHub {
    owner = "dfdgsdfg";
    repo = "imhexMCP";
    rev = "45b25b65205a3f3c1348c910f083cbcd5819fed9";
    sha256 = "0wwy57x8dcli9yxwzjc68zxwcqf2rwgrbnjr53vanmkjhh7xsbxh";
  };
in
rec {
  imhex-mcp-src = _imhex-mcp-src;

  f3demo = pkgs.callPackage ./f3demo.nix { };

  hexpat-language-server = pkgs.rustPlatform.buildRustPackage {
    pname = "hexpat-language-server";
    version = "unstable-2025-04-16";

    src = pkgs.fetchFromGitHub {
      owner = "Calcoph";
      repo = "hexpat-lsp";
      rev = "main";
      hash = "sha256-itrpBruWiTPT5HkQwPG7ma87HcYvSM65JGe8CrDHTi8=";
    };

    cargoHash = "sha256-77prNRR8j365CGFyApBD/hDe28sor5D0EKQKsg8p6/U=";

    meta = with pkgs.lib; {
      description = "Language server for ImHex Pattern Language";
      homepage = "https://github.com/Calcoph/hexpat-lsp";
      license = licenses.mit;
      platforms = platforms.linux;
      mainProgram = "hexpat-language-server";
    };
  };

  math-mcp = pkgs.buildNpmPackage {
    pname = "math-mcp";
    version = "0.1.1";

    src = pkgs.fetchFromGitHub {
      owner = "EthanHenrickson";
      repo = "math-mcp";
      rev = "6cca48319cfceede5c75e350dda6f0e9994a7b13";
      hash = "sha256-VuEzFWm3oke10q0kx4Le8FtHqmujgFJ8QuLqOEYjyP4=";
    };

    npmBuildScript = "build:stdio";
    npmDepsHash = "sha256-o2qqrxD25dZ74G7mcVLhVjcTQAtP0mKFO9pWAhmJCP4=";

    nativeBuildInputs = [ pkgs.makeWrapper ];

    postInstall = ''
      makeWrapper ${pkgs.nodejs}/bin/node $out/bin/math-mcp \
        --add-flags $out/lib/node_modules/math-mcp/build/index.js
    '';

    meta = with pkgs.lib; {
      description = "MCP server for math operations";
      homepage = "https://github.com/EthanHenrickson/math-mcp";
      license = licenses.mit;
      platforms = platforms.linux;
      mainProgram = "math-mcp";
    };
  };

  idapro = py311.buildPythonPackage {
    pname = "idapro";
    version = "0.0.7";

    src = pkgs.fetchPypi {
      pname = "idapro";
      version = "0.0.7";
      sha256 = "13wq8j8mby3ff7lhx50m2f9m9c11kgrif7wbli741n74355ihbhb";
    };

    format = "pyproject";
    nativeBuildInputs = [ py311.setuptools ];

    meta = with pkgs.lib; {
      description = "Python helpers shared with IDA Pro MCP";
      homepage = "https://pypi.org/project/idapro/";
      license = licenses.mit;
      platforms = platforms.linux;
    };
  };

  ida-pro-mcp = py311.buildPythonApplication {
    pname = "ida-pro-mcp";
    version = "2.0.0";

    src = pkgs.fetchFromGitHub {
      owner = "mrexodia";
      repo = "ida-pro-mcp";
      rev = "c2472d1c1e676f0198070e0e27f708973c7a4254";
      sha256 = "0qbb11iq8hy72q1n8cz7i3pkkvr3dwab8v9dqrbrmck93xzrdrnx";
    };

    format = "pyproject";
    nativeBuildInputs = [ py311.setuptools ];

    propagatedBuildInputs = [
      idapro
      py311.tomli-w
    ];

    meta = with pkgs.lib; {
      description = "AI-powered reverse engineering assistant for IDA Pro via MCP";
      homepage = "https://github.com/mrexodia/ida-pro-mcp";
      license = licenses.mit;
      platforms = platforms.linux;
    };
  };

  imhex-mcp-server = py312.buildPythonApplication {
    pname = "imhex-mcp-server";
    version = "0.1.0";

    src = _imhex-mcp-src;
    sourceRoot = "source/mcp-server";

    format = "pyproject";

    nativeBuildInputs = [
      py312.setuptools
      pkgs.python312
    ];

    postPatch = ''
      # Make sibling ../lib available to the installed server.
      cp -r ../lib ./imhex_mcp_lib

      # Fix logger bug: server.py uses logger.warning() before logger is defined.
      substituteInPlace server.py \
        --replace-fail \
          '    logger.warning("Enhanced client not available - performance optimizations disabled")' \
          '    logging.warning("Enhanced client not available - performance optimizations disabled")'

      # Ensure the copied library package is importable.
      substituteInPlace server.py \
        --replace-fail \
          'import logging' \
          'import logging
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parent / "imhex_mcp_lib"))'

      # Remove redundant imhex_client.connect() in main().
      # send_command() handles its own connect lifecycle.
      substituteInPlace server.py \
        --replace-fail \
          '        imhex_client.connect()' \
          '        # send_command() handles its own connect lifecycle'

      # Fix Python code that accidentally uses JSON boolean/null syntax.
      # OpenCode tools/list was failing with: name 'false' is not defined.
      ${pkgs.python312}/bin/python - <<'PY'
from pathlib import Path
import re

path = Path("server.py")
text = path.read_text()

text = re.sub(r'(?<![A-Za-z0-9_])false(?![A-Za-z0-9_])', 'False', text)
text = re.sub(r'(?<![A-Za-z0-9_])true(?![A-Za-z0-9_])', 'True', text)
text = re.sub(r'(?<![A-Za-z0-9_])null(?![A-Za-z0-9_])', 'None', text)

path.write_text(text)
PY

      # Add sync entry point for console_scripts because main() is async.
      cat >> server.py <<'PYEOF'

def entry_point():
    """Sync entry point for console_scripts."""
    asyncio.run(main())
PYEOF

      # Update entry point to use sync wrapper.
      substituteInPlace pyproject.toml \
        --replace-fail '"server:main"' '"server:entry_point"'
    '';

    propagatedBuildInputs = with py312; [
      mcp
      pydantic
      zstandard
      pyyaml
      prometheus-client
    ];

    meta = with pkgs.lib; {
      description = "MCP server for ImHex hex editor - AI-powered binary analysis";
      homepage = "https://github.com/dfdgsdfg/imhexMCP";
      license = licenses.gpl2;
      platforms = platforms.linux;
      mainProgram = "imhex-mcp-server";
    };
  };
}
