// T-035 acceptance artifact (redxenon/happywolf 01a157ec).
// One TU including BOTH F3 wait headers. Before the rename this MUST fail to compile
// (two different `F3::GameActionWait` structs collide — redefinition); after renaming the
// documentation-grade header's struct to F3::GameActionWaitShipped it MUST compile.
// That flip is the whole point: the two records are different animals — one is the shipped
// layout (JE-vb-derived, mbr_0x14, ctor/dtor/virt slots), the other is the working drain
// stand-in ({startMs,durationMs,Expired}). No field offsets move in either; name+comments only.
#include "F3/GameActionWait.hpp"
#include "F3/GameActionMoveTo.hpp"

int main() { return 0; }
