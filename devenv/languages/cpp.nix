# ./devenv/languages/cpp.nix

{ config, pkgs, lib, ... }:
let
  unwrapped-clangd = pkgs.runCommand "clangd-unwrapped" {} ''
    mkdir -p $out/bin
    ln -s ${pkgs.clang-tools}/bin/clangd-unwrapped $out/bin/clangd
  '';
in {
  packages = [ (lib.hiPrio unwrapped-clangd) ];

  languages.cplusplus = {
    enable = true;
    lsp = {
      enable = true;
      package = unwrapped-clangd;
    };
  };

  files.".clangd".text = /* yaml */ ''
    CompileFlags:
      Compiler: ${pkgs.pkgsCross.mingw32.stdenv.cc}/bin/i686-w64-mingw32-g++
      BuiltinHeaders: QueryDriver
      Add: [
        --target=i686-w64-mingw32,
        -std=c++98,
        -Wall,
        -Wno-pragma-pack,
        -D_WIN32,
        -DTRACING,
        -DTRACE_TERMINAL,
        -DTRACE_FILE,
        -nostdinc,
        -nostdinc++,
        -isystem,
        ${pkgs.pkgsCross.mingw32.windows.mcfgthreads.dev}/include,
        -isystem,
        ${pkgs.pkgsCross.mingw32.windows.mingw_w64.dev}/include,
        -isystem,
        ${pkgs.pkgsCross.mingw32.stdenv.cc.cc}/include/c++/${pkgs.pkgsCross.mingw32.stdenv.cc.cc.version},
        -isystem,
        ${pkgs.pkgsCross.mingw32.stdenv.cc.cc}/include/c++/${pkgs.pkgsCross.mingw32.stdenv.cc.cc.version}/i686-w64-mingw32,
        -isystem,
        ${pkgs.pkgsCross.mingw32.stdenv.cc.cc}/include/c++/${pkgs.pkgsCross.mingw32.stdenv.cc.cc.version}/backward,
        -I${config.devenv.root}/src,
        -I${config.devenv.root}/modules/openje/src,
        -I${config.devenv.root}/modules/tracing/src,
      ]

    ---
    If:
      PathMatch: [.*\.h, .*\.hpp]
    CompileFlags:
      Add: [-xc++]

    InlayHints:
     Enabled: true
     ParameterNames: true

    Diagnostics:
      ClangTidy:
        Add: ["*"]
'';
}
