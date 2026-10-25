#!/usr/bin/env python3
"""cmake-toml drift check: does cmake.toml still describe the build that CMakeLists.txt builds?

WHY THIS EXISTS
    CMakeLists.txt is generated from cmake.toml by cmkr ("DO NOT EDIT" in its header), and
    devenv/scripts/msvc-build.nix runs `cmkr gen` on EVERY build - but only when the cached
    cmkr binary is present (build/_cmkr_v*/build/cmkr). That cache has never been populated
    in this checkout, so `cmkr gen` silently skips, CMakeLists.txt is effectively hand-edited,
    and every new translation unit has been registered in the generated file only. The two
    files drifted from 27/27 openje sources (matching) to 77/27 - 50 TUs, every ported class
    among them, exist only in the generated file.

    The drift is invisible at build time and only bites later: bootstrap cmkr once (a fresh
    configure with network, or a restored build/ cache) and the next `cmkr gen` rewrites
    CMakeLists.txt from cmake.toml, deleting those 50 sources from the openje target. The
    build then fails - or worse, "succeeds" against a smaller library.

WHAT IT CHECKS
    For each build target the generated file declares with a `set(<target>_SOURCES ...)`
    block, the quoted paths must equal the target's `sources` array in cmake.toml, as sets
    (order in the toml is free; cmkr sorts). cmake.toml itself is in every list because cmkr
    adds it as a configure dependency - it is compared like any other entry.

USAGE
    python3 tools/check-cmake-toml-drift.py            # exit 0 clean, 1 on drift
    python3 tools/check-cmake-toml-drift.py --verbose  # list the symmetric difference
"""
import re
import sys
import pathlib

ROOT = pathlib.Path( __file__ ).resolve().parent.parent


def generated_sources( cmakelists ):
    """target name -> list of quoted paths in its set(<target>_SOURCES ...) block."""
    out = {}
    for m in re.finditer( r'set\((\w+)_SOURCES\n(.*?)\n\)', cmakelists, re.S ):
        out[ m.group( 1 ) ] = re.findall( r'"([^"]+)"', m.group( 2 ) )
    return out


def toml_sources( toml ):
    """target name -> list of paths in its [target.<name>] sources array."""
    out = {}
    for m in re.finditer( r'\[target\.(\w+)\]\n(.*?)(?=\n\[|\Z)', toml, re.S ):
        sm = re.search( r'sources\s*=\s*\[(.*?)\]', m.group( 2 ), re.S )
        if sm:
            out[ m.group( 1 ) ] = re.findall( r'"([^"]+)"', sm.group( 1 ) )
    return out


def main():
    verbose = '--verbose' in sys.argv
    cmakelists = ( ROOT / 'CMakeLists.txt' ).read_text()
    toml = ( ROOT / 'cmake.toml' ).read_text()

    gen = generated_sources( cmakelists )
    tml = toml_sources( toml )
    if not gen:
        print( 'check-cmake-toml-drift: no set(<target>_SOURCES) blocks found in CMakeLists.txt' )
        return 1

    failures = 0
    for target in sorted( gen ):
        g = gen[ target ]
        t = tml.get( target )
        if t is None:
            print( '[FAIL] %s: CMakeLists.txt builds %d source(s) but cmake.toml has no [target.%s] sources array'
                   % ( target, len( g ), target ) )
            failures += 1
            continue
        only_gen = [ s for s in g if s not in t ]
        only_toml = [ s for s in t if s not in g ]
        if only_gen or only_toml:
            failures += 1
            print( '[FAIL] %s: %d in CMakeLists.txt only, %d in cmake.toml only'
                   % ( target, len( only_gen ), len( only_toml ) ) )
            if verbose:
                for s in only_gen:
                    print( '         built, not in cmake.toml: %s' % s )
                for s in only_toml:
                    print( '         in cmake.toml, not built: %s' % s )
            else:
                print( '         rerun with --verbose for the symmetric difference' )
        else:
            print( '[ok]   %s: %d source(s) agree' % ( target, len( g ) ) )

    if failures:
        print( 'check-cmake-toml-drift: %d target(s) drifted - register new TUs in cmake.toml, '
               'not in the generated CMakeLists.txt' % failures )
        return 1
    print( 'check-cmake-toml-drift: OK' )
    return 0


if __name__ == '__main__':
    sys.exit( main() )
