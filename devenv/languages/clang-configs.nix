# ./devenv/languages/clang-configs.nix

{ pkgs, ... }: {
  files.".clangd".text = ''
CompileFlags:
  Compiler: ${pkgs.pkgsCross.mingw32.stdenv.cc}/bin/i686-w64-mingw32-g++
  QueryDriver: ${pkgs.pkgsCross.mingw32.stdenv.cc}/bin/i686-w64-mingw32-g++
  BuiltinHeaders: QueryDriver
  Add: [
    -std=c++98,
    -Wall,
    -Wno-pragma-pack,
    -D_WIN32,
    -DTRACING,
    -DTRACE_TERMINAL,
    -DTRACE_FILE,
    -isystem,
    ${pkgs.pkgsCross.mingw32.windows.mcfgthreads.dev}/include,
    -I./src,
    -I./modules/openje/src,
    -I./modules/tracing/src,
  ]

---
If:
  PathMatch: [.*\.h, .*\.hpp]
CompileFlags:
  Add: [-xc++]

InlayHints:
 Enabled: true
 ParameterNames: true

ClangTidy:
  Add: ["*"]
'';
}