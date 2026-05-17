# ./devenv/packages/local.nix

{ pkgs, ... }:
let
  py311 = pkgs.python311Packages;

  _imhex-mcp-src = pkgs.fetchFromGitHub {
    owner = "jmpnop";
    repo = "imhexMCP";
    rev = "c9ceb7f791e5e9233c555a1fc3770b4403d08dcf";
    hash = "sha256-gDcUYTQrYBCU/XI+Y/dJIc3nFNFInMMB0HQuMNblTOg=";
  };
in rec {
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
}
