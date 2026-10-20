@echo off
rem msvc-sdk-prerelease.bat - called after vcvars32.bat by devenv/scripts/msvc-build.nix.
rem
rem The MSVC 2002 image DOES ship the Direct3D 8 SDK, but under PlatformSDK\{Include,lib}\prerelease,
rem which vcvars32.bat does not put on INCLUDE/LIB. That is why a d3d8 include/links fails in a normal
rem msvc-build while the files are present on disk - the gap is environment, not a missing library, and
rem nothing here vendors or licenses third-party binaries.
rem
rem Guarded with `if exist`: on an image without these dirs the build behaves exactly as before instead
rem of failing with a new error, and the verdict is printed so the build log shows which case ran.
set PREREL=C:\Program Files\Microsoft Visual Studio .NET 2002\Vc7\PlatformSDK
if exist "%PREREL%\Include\prerelease\d3d8.h" set INCLUDE=%INCLUDE%;%PREREL%\Include\prerelease
if exist "%PREREL%\lib\prerelease\d3d8.lib" set LIB=%LIB%;%PREREL%\lib\prerelease
if defined INCLUDE if exist "%PREREL%\Include\prerelease\d3d8.h" echo [sdk-prerelease] d3d8 SDK on INCLUDE+LIB: %PREREL%
if not exist "%PREREL%\Include\prerelease\d3d8.h" echo [sdk-prerelease] NOT PRESENT in image - d3d8 still unavailable
