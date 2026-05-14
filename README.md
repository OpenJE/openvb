# OpenVB
### Reverse engineered and open source Fallout: Van Buren

## Disclaimer
- This project is not affiliated with Bethesda Softworks LLC, Interplay Entertainment Corp., Bethesda Game Studios, or Black Isle Studios.
- This project is completely open source and no commercial use is intended.
- All copyrights belong to their respective owners.

## Description
This project is the reverse engineered implementation of the cancelled Fallout 3 project, codenamed Van Buren. The goal is to rewrite the game in C++ as close to the original source code as possible.

Progress is ongoing. The initialization steps of the game are partially reverse-engineered/implemented and compilable/executable, but much more work needs to be done.

Once the game is fully reverse engineered, the plan is to swap out the win32 and DirectX 8 API calls with cross-platform implementations/libraries, and add the missing features and content that was never implemented in the original tech demo. Mod support is a possible goal as well.

For documentation on the structs, classes, and functions of Fallout: Van Buren, see [`docs/f3.md`](docs/f3.md).

This project uses the [OpenJE](https://github.com/OpenJE/openje) Jefferson Engine.

## Project Structure

| Directory | Description |
|-----------|-------------|
| `src/` | F3 game source code (F3.cpp, main.cpp, F3/) |
| `modules/openje/` | OpenJE (Jefferson Engine) git submodule |
| `modules/tracing/` | Tracing library git submodule |
| `modules/opencode-openje/` | OpenCode JE configuration submodule |
| `devenv/` | Devenv configuration (languages, packages, scripts, tasks, services, processes) |
| `docs/` | Reverse-engineering documentation (`f3.md`) |
| `cmake.toml` | CMake configuration (managed by [cmkr](https://github.com/build-cpp/cmkr)) |
| `STYLE.md` | Coding conventions and style guide |
| `flake.nix` | Alternative Nix flake development shells |

## Prerequisites

- **Git** — clone with `--recursive` to fetch all submodules:
  ```bash
  git clone --recursive https://github.com/shanemulcahy88/openvb.git
  ```
- **Nix** package manager (see platform-specific sections below)
- **devenv** (installed via Nix)

NixOS users already have Nix installed, so skip straight to devenv installation.

## Setting Up

### Linux

Multi-user install (recommended):
```bash
sh <(curl --proto '=https' --tlsv1.2 -L https://nixos.org/nix/install) --daemon
```

Single-user install:
```bash
sh <(curl --proto '=https' --tlsv1.2 -L https://nixos.org/nix/install) --no-daemon
```

See [NixOS Download](https://nixos.org/download/#download-nix) for more options.

### macOS

Multi-user install:
```bash
sh <(curl --proto '=https' --tlsv1.2 -L https://nixos.org/nix/install)
```

macOS 15 Sequoia users who see `_nixbld1` errors should see [NixOS/nix#10892](https://github.com/NixOS/nix/issues/10892).

### WSL2 — Option A: Nix inside existing distro

Ensure systemd is enabled in WSL2 (`/etc/wsl.conf` or `wsl --update`), then run the Linux multi-user install command above.

### WSL2 — Option B: NixOS-WSL dedicated distro

1. Download `nixos.wsl` from the [latest release](https://github.com/nix-community/NixOS-WSL/releases/latest).
2. Install:
   ```bash
   wsl --install --from-file nixos.wsl
   ```
   (Requires WSL 2.4.4+. For older WSL: `wsl --import NixOS $env:USERPROFILE\NixOS nixos.wsl --version 2`)
3. Set a password: `passwd`
4. Update channels: `sudo nix-channel --update`

See [NixOS-WSL installation](https://nix-community.github.io/NixOS-WSL/install.html) for full details.

### After Nix is installed on any platform

Install devenv:
```bash
nix profile install --accept-flake-config tarball+https://github.com/cachix/devenv/archive/latest.tar.gz
```

Or install via cachix:
```bash
cachix install devenv
```

Enter the development shell:
```bash
devenv shell
```

See [devenv.sh](https://devenv.sh/getting-started/) for more.

## Building

Enter the devenv shell:
```bash
devenv shell
```

Build with MSVC cross-compilation (requires Docker):
```bash
devenv task run build:msvc
```

Or equivalently:
```bash
msvc-build
```

Other build variants:
```bash
devenv task run build:msvc-debug
msvc-build-debug

devenv task run build:msvc-tracing
msvc-build-tracing
```

The MSVC build uses a Docker container (`ghcr.io/openje/msvc2002`) to replicate the original MSVC 2002 build environment. On NixOS with rootless Docker, run `systemctl --user enable --now docker` first.

Alternative: use `nix develop` to enter the flake.nix development shells (`host` or `mingw32`).

## Available Commands

### Build Scripts

| Command | Description |
|---------|-------------|
| `msvc-build` | Build OpenVB with MSVC release |
| `msvc-build-debug` | Build OpenVB with MSVC debug |
| `msvc-build-tracing` | Build OpenVB with tracing |

### Run Scripts

| Command | Description |
|---------|-------------|
| `f3-reimpl-run` | Run rebuilt F3.exe with Wine |
| `f3-reimpl-run-debug` | Start rebuilt F3.exe Wine GDB stub |
| `f3-reimpl-run-trace` | Run rebuilt traced F3-trace.exe |
| `f3demo-run` | Run original Van Buren demo |
| `f3demo-run-debug` | Start original demo Wine GDB stub |
| `f3-lockstep-debug` | Start original and rebuilt GDB stubs |
| `f3-run` | Run rebuilt F3.exe with Wine (legacy alias) |
| `f3-run-debug` | Run traced F3-trace.exe with Wine (legacy alias) |

### Check Scripts

| Command | Description |
|---------|-------------|
| `cppcheck-openvb` | Run cppcheck on openvb |
| `cppcheck-openje` | Run cppcheck on openje |
| `cppcheck-tracing` | Run cppcheck on tracing |
| `cppcheck-all` | Run cppcheck on all modules |

### Devenv Tasks

| Task | Runs |
|------|------|
| `devenv task run build:msvc` | `msvc-build` |
| `devenv task run build:msvc-debug` | `msvc-build-debug` |
| `devenv task run build:msvc-tracing` | `msvc-build-tracing` |
| `devenv task run run:f3` | `f3-run` |
| `devenv task run run:f3-debug` | `f3-run-debug` |
| `devenv task run run:f3-lockstep-debug` | `f3-lockstep-debug` |
| `devenv task run run:f3-reimpl` | `f3-reimpl-run` |
| `devenv task run run:f3-reimpl-debug` | `f3-reimpl-run-debug` |
| `devenv task run run:f3-reimpl-trace` | `f3-reimpl-run-trace` |
| `devenv task run run:f3demo` | `f3demo-run` |
| `devenv task run run:f3demo-debug` | `f3demo-run-debug` |
| `devenv task run check:cppcheck` | `cppcheck-openvb` |
| `devenv task run check:cppcheck-openje` | `cppcheck-openje` |
| `devenv task run check:cppcheck-tracing` | `cppcheck-tracing` |
| `devenv task run check:cppcheck-all` | `cppcheck-all` |

### Processes

Managed by `devenv processes`:

| Process | Description |
|---------|-------------|
| `ida-orig-headless` | Headless IDA MCP server for original binary |
| `ida-reimpl-headless` | Headless IDA MCP server for reimplementation binary |
| `ida-orig-gdb` | GDB stub for original demo (via Wine) |
| `ida-reimpl-gdb` | GDB stub for rebuilt binary (via Wine) |

## Contributing

If you want to contribute to this project, contact me on **Discord**: **shenmarukai** or by **Email**: **shanemulc@comcast.net**. A lot of work still needs to be done, and any help is appreciated.

## Links

- [OpenJE](https://github.com/OpenJE/openje) — Jefferson Engine
- [STYLE.md](STYLE.md) — Coding conventions
- [docs/f3.md](docs/f3.md) — Struct/class/function documentation
- [devenv.sh](https://devenv.sh/getting-started/) — Devenv getting started guide
- [NixOS Download](https://nixos.org/download/#download-nix) — Nix installer
- [NixOS-WSL](https://nix-community.github.io/NixOS-WSL/install.html) — NixOS-WSL installation
