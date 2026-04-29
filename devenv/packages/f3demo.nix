# ./devenv/packages/f3demo.nix

{ stdenvNoCC, fetchurl, unrar }:

stdenvNoCC.mkDerivation {
  pname = "f3demo";
  version = "1.0";

  src = fetchurl {
    url = "https://archive.org/download/f3demo/F3_Demo.rar";
    sha256 = "0c69gxjkrmpy570q0j7ndv9x1q368vx6kqara5km0lh0grn2cf05";
  };

  nativeBuildInputs = [ unrar ];

  unpackPhase = ''
    runHook preUnpack
    unrar x "$src"
    runHook postUnpack
  '';

  installPhase = ''
    runHook preInstall
    mkdir -p "$out/share/f3demo"
    cp -r . "$out/share/f3demo/"
    runHook postInstall
  '';
}
