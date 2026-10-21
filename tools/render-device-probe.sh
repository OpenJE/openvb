#!/bin/sh
# T-031/M-001 support - tools/render-device-probe.sh
#
# Question this answers, mechanically: can a D3D8 device be CREATED at all in this harness? That is
# the precondition for M-001 slice 1's acceptance ("a human sees a magenta frame"), and until now the
# answer came from hand-run one-offs against the human's own X server (:0), which is both unreliable
# and rude.
#
# It runs three cases and reports each, without assuming which will pass:
#   1. no DISPLAY at all      - does wine even start the app?
#   2. Xvfb, device path OFF  - does the app survive a virtual display with no d3d8 attempt?
#   3. Xvfb, device path ON   - does CreateDevice return, or does wined3d fail GL setup?
#
# Usage: tools/render-device-probe.sh [path/to/F3.exe]
# Default exe is build/F3.exe. Set PROBE_ENV_RENDER_VAR / PROBE_ENV_TRACE_VAR to change the opt-in
# names if the render device lands with different variables.
#
# BOUNDARY - read this before quoting the result:
#   * Case 3 needs a build that CONTAINS the render device (F3::RenderDevice + the OPENVB_RENDER
#     opt-in). As of writing that exists only in the M-001 scratch worktree, NOT in the shared tree,
#     because cmake.toml/CMakeLists.txt are the coordinator's lane. If the exe lacks it, case 3
#     degrades to "no trace line" and the verdict must be read as UNKNOWN, not FAIL.
#   * Exit codes: 124 = alive past the timeout (good), 1 = wine exited on its own, 40 = hard fault.
#   * A FAIL here is a statement about THIS wine/graphics stack, not about our code. The tool prints
#     which, because "we can't see it" must never be reported as "we can't build it".
set -u

EXE="${1:-build/F3.exe}"
WINE="${WINE:-$(command -v wine 2>/dev/null || echo /nix/store/b0d6l5vh4fdawmvq5j504a47pcxfsk8i-wine-11.0/bin/wine)}"
DATA="${OPENVB_F3DATA:-C:\\f3data}"
TIMEOUT="${PROBE_TIMEOUT:-10}"
RENDER_VAR="${PROBE_ENV_RENDER_VAR:-OPENVB_RENDER}"
TRACE_VAR="${PROBE_ENV_TRACE_VAR:-OPENVB_RENDER_TRACE}"
TRACE_OUT="/tmp/render-probe-$$.trace"
WINPREFIX="${WINEPREFIX:-}"

if [ ! -f "$EXE" ]; then
	echo "verdict: UNKNOWN - no exe at $EXE (run msvc-build, or pass the worktree exe as \$1)" >&2
	exit 0
fi

run_case() { # name, extra env assignments...
	name="$1"; shift
	rm -f "$TRACE_OUT"
	"$@" timeout "$TIMEOUT" "$WINE" "$EXE" "$DATA" >"${TRACE_OUT}.stderr" 2>&1
	rc=$?
	if [ -f "$TRACE_OUT" ]; then trace="$(tr -d '\r\n' < "$TRACE_OUT")"; else trace=""; fi
	echo "case ${name}: wine exit=${rc} (124=alive, 1=exited, 40=faulted)"
	echo "  trace: ${trace:-<none - the exe has no $RENDER_VAR opt-in, or the path was rejected>}"
	grep -hoiE "Failed to find a suitable pixel format|Failed to get a GL context[^ ]*|XIO: fatal IO error [0-9]+|nodrv_CreateWindow|no card selector[^ ]*" "${TRACE_OUT}.stderr" 2>/dev/null | sort -u | sed 's/^/  diag: /'
}

echo "probe: exe=$EXE wine=$WINE timeout=${TIMEOUT}s prefix=${WINPREFIX:-<default>}"
echo

# 1. no display at all
run_case "1 no-DISPLAY" env -u DISPLAY -u "$RENDER_VAR" -u "$TRACE_VAR"

# 2. Xvfb, device path off. The trace path is a WINDOWS path (the app is a Windows binary and its
# fopen sees wine's namespace): Z:\tmp maps to the host /tmp, which is where this script reads it back.
WIN_TRACE="Z:\\tmp\\render-probe-$$.trace"
run_case "2 xvfb, device OFF" env -u "$RENDER_VAR" "$TRACE_VAR=$WIN_TRACE" xvfb-run -a -s "-screen 0 800x600x24"

# 3. Xvfb, device path on, software GL asked for explicitly
run_case "3 xvfb, device ON" env "$RENDER_VAR=1" LIBGL_ALWAYS_SOFTWARE=1 "$TRACE_VAR=$WIN_TRACE" xvfb-run -a -s "-screen 0 800x600x24"

echo
echo "verdict rules:"
echo "  case 3 alive + trace 'device ready'      -> slice 1 IS verifiable in this harness (screenshot next)"
echo "  case 3 'suitable pixel format'/GL failure -> harness limit: no usable GL context for wined3d here."
echo "                                            NOT a code defect; visual acceptance needs a host with"
echo "                                            a working GL driver."
echo "  case 3 trace <none> and case 2 also none -> the exe lacks the opt-in; result is UNKNOWN."
echo "  case 2 alive but case 3 exits/faults      -> the device attempt itself is what breaks; keep it opt-in."
echo
echo "boundary: three cases, one exe, one wine. It proves what THIS stack can do, nothing about the"
echo "          shipped renderer, and nothing about frames that were never presented."
rm -f "$TRACE_OUT" "${TRACE_OUT}.stderr" /tmp/render-probe-$$.trace 2>/dev/null
exit 0
