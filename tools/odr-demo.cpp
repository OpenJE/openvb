// T-035 acceptance artifact (redxenon/happywolf 01a157ec).
// One TU including BOTH F3 wait headers. Before the rename this MUST fail to compile
// (two different `F3::GameActionWait` structs collide — redefinition); after renaming the
// documentation-grade header's struct to F3::GameActionWaitShipped it MUST compile.
// That flip is the whole point: the two records are different animals — one is the shipped
// layout (JE-vb-derived, mbr_0x14, ctor/dtor/virt slots), the other is the working drain
// stand-in ({startMs,durationMs,Expired}). No field offsets move in either; name+comments only.
#include "F3/GameActionWait.hpp"
#include "F3/GameActionMoveTo.hpp"   // pump-side F3::GameActionWait
#include "F3/GameActionDie.hpp"
#include "F3/GameActionAttack.hpp"    // pump-side F3::GameActionDie

// Both shipped-layout headers and both pump-model headers in ONE translation unit.
// RED  (renames undone): error: redefinition of 'struct F3::GameActionWait'
//                       error: redefinition of 'struct F3::GameActionDie'
// GREEN (tree as-is)  : compiles clean.
// Reference the two documentation structs so the compiler actually checks them, not just the
// preprocessor: a TU that never names them would pass even if the rename were half-done.
int main() {
	// Name every struct so the compiler actually checks both declarations exist; sizeof makes the
	// layouts real participants in the TU rather than dead includes.
	F3::GameActionWaitShipped* w = 0; (void)w;
	F3::GameActionWait*        pw = 0; (void)pw;
	F3::GameActionDieShipped*  d = 0; (void)d;
	F3::GameActionDie*         pd = 0; (void)pd;
	return (int)(sizeof(F3::GameActionWaitShipped) + sizeof(F3::GameActionWait)
	           + sizeof(F3::GameActionDieShipped)  + sizeof(F3::GameActionDie)) > 0 ? 0 : 1;
}
