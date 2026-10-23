// T-028 driver half — happywolf (01a157ec).
// A scripted event source feeding the SAME seams a real input path reaches in the shipped
// game: entity Action objects enqueued through F3::EnqueueEntityAction (0x550720 funnel,
// discoveries/2025-10-20-input-path-architecture-command-fifo-actions.md + docs/f3/
// EntityActionQueueSeam.md) and drained by F3::TickWorldEntities, with F3::CombatManager
// as the round pump, emitting T-026 digest lines (docs/f3/SimDigest.md grammar).
//
// Door-shape deviations accepted with WildYak's T-037 review (see also notes at the
// attack verb): (1) Start() pre-writes actor before the funnel re-writes it - the port
// offers no scripted entry without that write; (2) target is a raw pointer here, shipped
// stores the +0x38 REF KEY and re-resolves every use (despawn-mid-attack reachable there,
// not here); (3) calledShotPart 9->0 preflight runs inside port Start(). (1)-(3) need a
// port-side StartScripted to fix - driver-only scope cannot. (4) JustAttack is NOT
// expressible: our port models neither +0x6C turn-to-face nor +0x6F approach gate, so
// zeroing them would be setting fields nothing reads - a missing seam, not a missing verb.
// (5) StartDialogue's door calls 0x54c440 BEFORE allocating - it destroys the whole
// Entity+0x304 deque: a scripted dialogue preempts pending orders. No dialogue verb yet;
// when one lands, that side effect is part of the door, not an extra.
//

// D-2 SEVERITY, measured not argued (RedXenon 01a157ec, tools/scripts/d2-stress.txt):
// the shipped out-of-range attack pushes TWO actions onto one owner in ONE OnServerUpdate
// pass (WildYak T-037: 0x40470a + 0x40471a, both `call 0x550720`), and the funnel is
// UNBOUNDED - cls_0x54e070::meth_0x54ea00 allocates a fresh 4-pointer block at 0x54ea54
// when count&3 wraps. So depth 5 is the 3rd-4th approach pass of ONE melee actor. Our port
// models ONE block (actions[4]) and REFUSES the 5th insert. d2-stress.txt drives exactly
// that: 5 attacks on one actor, bare shipped-shape verbs, and the driver FAILS at line 19
// with "queue block full (D-2 refusal)" (exit 1, measured). That refusal IS a dropped
// gameplay order the original would have run. D-2 is therefore a real defect in actions[4],
// not a style note - and this driver is the instrument that measures it, so the claim is
// reproducible by any lane: wine tools/sim-driver.exe tools/scripts/d2-stress.txt -> exit 1.
//
// Build: tools/sim-driver-build.sh  (source list GENERATED from devenv/scripts/
// run-all-tests.nix's PathManager row - this lane may not edit build files, wave rule 1).
// Usage: tools/sim-driver.bin <script.txt>
//
// WHAT THIS IS NOT (printed at start of every run, per the tool-boundary rule):
// this is NOT the 8-slot GameStateLoop world walk. It ticks the two pumps the orders
// actually need (entity action drain + combat turns) and reports every other subsystem
// as an explicit skip. `draws` is never faked: with no counter on the sim stream this
// driver passes drawsSource == NULL, which the digest grammar renders as draws=NA, and
// never as draws=0.

#include "F3/MapEntitySpawn.hpp"
#include "F3/GameActionAttack.hpp"
#include "F3/GameActionMoveTo.hpp"   // live F3::GameActionWait (startMs/durationMs/Expired).
// The shipped-layout record in src/F3/GameActionWait.hpp is F3::GameActionWaitShipped
// (renamed by T-035 after this driver found the collision; both headers now co-TU —
// tools/odr-demo.cpp).
#include "F3/CombatManager.hpp"
#include "F3/CombatMath.hpp"        // F3::SeedShippedRand (test-only LCG-state seam)
#include "F3/SimDigest.hpp"
#include "JE/PathManager.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

	const int kMaxEntities  = 32;
	const int kMaxActions   = 32;   // attacks/wait share this pool; objects must outlive
	                                // the enqueue (the seam stores the POINTER)

	F3::SpawnedEntity   g_ents[ kMaxEntities ];
	F3::GameActionAttack g_atks[ kMaxActions ];
	F3::GameActionWait   g_waits[ kMaxActions ];
	int g_atkN = 0, g_waitN = 0;

	F3::CombatManager  g_cm;
	JE::PathManager*   g_pm = 0;      // no map loaded -> NA:path-manager-in-this-context
	static bool        g_branchPrinted = false;   // T-051: combat-branch trace, once per run

	unsigned long g_clockMs  = 0;     // the harness owns simulated time (SimDigest header)
	// T-011 step 4: CombatManager::Tick's second argument is the ROUND quantum - shipped reads
	// world+0x40, the [World] "Seconds Per Round" config (default 6 at 0x546b5b). It is NOT the
	// elapsed clock, and passing g_clockMs/1000 there (what this driver did) would have made the
	// accumulator grow with wall time, which is the retired C-23 placeholder in a new costume.
	int g_secondsPerRound = 6;
	FILE*         g_digest   = 0;
	unsigned      g_frame    = 0;

	F3::SpawnedEntity* findEnt( const char* name ) {
		for ( int i = 0; i < F3::g_spawnedEntityCount; ++i ) {
			F3::SpawnedEntity* e = F3::g_spawnedEntities[ i ];
			if ( e && strcmp( e->name, name ) == 0 ) { return e; }
		}
		return 0;
	}

	void registerEnt( F3::SpawnedEntity* e ) {
		if ( F3::g_spawnedEntityCount < 128 ) {
			F3::g_spawnedEntities[ F3::g_spawnedEntityCount++ ] = e;
		}
	}

	int skillArg( const char* s ) {   // melee only for now - others reported, not guessed
		if ( strcmp( s, "melee" ) == 0 ) { return F3::kSkillMelee; }
		return -1;
	}

	void runFrames( int frames, int stepMs ) {
		F3::Sim::Sources src;
		src.combat = &g_cm;
		src.paths  = g_pm;
		for ( int i = 0; i < frames; ++i ) {
			g_clockMs += (unsigned long)stepMs;
			F3::TickWorldEntities( (int)g_clockMs );
			g_cm.Tick( stepMs, (unsigned)( g_secondsPerRound > 0 ? g_secondsPerRound : 0 ) );
			if ( !g_branchPrinted ) {
				// T-051: name the branch the manager actually took, read from the LIVE mode -
				// one trace line, not a digest field. Free-run is the only shipped-LIVE branch:
				// +4 (mode) and +14 (rearm) have no nonzero producer anywhere in the image
				// (T-010 10e, 045a7ec), so the round-walk is dead code in shipped config and
				// this driver must never arm it. If the second arm of this ternary ever
				// prints, something armed the dead branch - that is a BUG REPORT, not a feature.
				g_branchPrinted = true;
				fprintf( stderr, "sim-driver: combat branch: %s\n",
				         g_cm.RoundMode() == 0u
				           ? "free-run (mode0x4_==0) - the only shipped-live path (10e); round-walk is dead in shipped config"
				           : "round-walk (mode0x4_!=0) - DEAD in shipped; SOMETHING ARMED IT - T-051 audit expects never" );
			}
			if ( g_digest ) {
				F3::Sim::WriteFrame( g_digest, src, g_frame, (unsigned)g_clockMs,
				                     (unsigned)( g_clockMs / 1000u ), 0, 0 );
			}
			++g_frame;
		}
	}

} // namespace

// The documented headless pattern (every host suite defines this block; src/F3.cpp's version
// is not in any suite's link list). Honest values for a host-authoritative driver: TickMilliseconds
// returns the SIMULATED clock, not uptime; parameter tree/string system are unwired (NULL), which
// the callers already tolerate; IsServerContext is TRUE because this driver is the host.
namespace JE {
	namespace Host {
		char* GetCurrentWorkingDirectory() { static char d[] = "C:\\f3data"; return d; }
		void* GetParameterTree() { return 0; }
		void* GetEngineStringSystem() { return 0; }
		int   TickMilliseconds() { return (int)g_clockMs; }
		bool  IsServerContext() { return true; }
		void  TickWorldEntities( int ms ) { F3::TickWorldEntities( ms ); }   // same routing as src/F3.cpp:790
		JE::PathManager* GetPathJobQueue() { return g_pm; }
	} // namespace Host
} // namespace JE

int main( int argc, char** argv ) {
	fprintf( stderr,
		"sim-driver boundary: pumps = TickWorldEntities + CombatManager::Tick ONLY; not the "
		"8-slot GameStateLoop walk. map/move -> SKIP reported (no ResourceSystem wiring here). "
		"draws=NA (no counter on the sim stream; never reported as 0). clock/tickMs are "
		"simulated by this harness.\n" );
	if ( argc < 2 ) {
		fprintf( stderr, "usage: %s <script.txt>\n", argv[0] );
		return 2;
	}
	FILE* s = fopen( argv[1], "r" );
	if ( !s ) { fprintf( stderr, "SIM-DRIVER FAIL: cannot open script %s\n", argv[1] ); return 2; }

	char line[ 256 ];
	int  lineno = 0;
	int  rc = 0;
	while ( fgets( line, sizeof( line ), s ) ) {
		++lineno;
		char* hash = strchr( line, '#' );
		if ( hash ) { *hash = 0; }
		char verb[ 64 ];
		if ( sscanf( line, "%63s", verb ) != 1 ) { continue; }

		if ( strcmp( verb, "spawn" ) == 0 ) {
			char name[ 64 ]; int slot = -1; float x = 0.0f, y = 0.0f, z = 0.0f; int hp = 45;
			int n = sscanf( line, " spawn %63s %d %f %f %f %d", name, &slot, &x, &y, &z, &hp );
			if ( n < 2 || slot < 0 || slot >= kMaxEntities || name[0] == 0 ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: spawn needs name slot x y z [hp]\n", lineno );
				rc = 1; break;
			}
			F3::SpawnedEntity& e = g_ents[ slot ];
			if ( findEnt( name ) ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: duplicate entity name %s\n", lineno, name );
				rc = 1; break;
			}
			e = F3::SpawnedEntity();
			strncpy( e.name, name, sizeof( e.name ) - 1 );
			e.pos[0] = x; e.pos[1] = y; e.pos[2] = z;
			e.statMax = hp; e.statCur = hp;
			e.entityState = F3::kStateActive;   // ACTIVE pumps (sess-11/12 switch)
			registerEnt( &e );
			// P-REV: the harness's spawn IS the add-to-world edge (the arm it feeds COMPLETES
			// on the next processed tick - 0x5a9070 raw; use `peek` to sample that window).
			// Shipped fires the idle
			// rebase from the net path that follows spawn (Entity::SetInMap_0x551ba0: drain
			// @0x551bbe -> CreateGameAction(1) @0x551bde -> funnel @0x551be4, driven by the
			// Server/Client pumps), and T-056 run H saw the result as GameActionIdle at the
			// first insert on 12/12 entities. The head-of-line consequence is the shipped
			// baseline: orders that do not drain first wait behind the resident (run H's
			// "Attack never fired"); command-boundary packets drain, so `attack`/`attack-pos`/
			// `wait` model that pump and `push` below models the no-drain native shape.
			F3::RebaseOntoIdle( &e, 0 );
		} else if ( strcmp( verb, "melee" ) == 0 ) {
			char who[ 64 ], sk[ 32 ]; int v = 0;
			if ( sscanf( line, " melee %63s %31s %d", who, sk, &v ) != 3 ) { rc = 1; break; }
			int k = skillArg( sk );
			F3::SpawnedEntity* e = findEnt( who );
			if ( k < 0 || !e ) {
				fprintf( stderr, "SIM-DRIVER SKIP line %d: skill '%s' unsupported or entity missing\n",
				         lineno, sk );
				continue;
			}
			e->combat.skills[ k ] = v;
		} else if ( strcmp( verb, "attack" ) == 0 ) {
			// Shipped door (WildYak T-037, docs/f3/ScriptNativeActionDoors.md): Attack() is
			// TWO cells; no mode data crosses the script boundary - the mode row is resolved
			// from the equipped weapon inside OnServerUpdate (0x40b380), which our port has
			// not rebuilt. Bare `attack a t` therefore passes m=NULL: Start() keeps the ctor's
			// own unarmed defaults (GameActionAttack.cpp:74), the closest honest stand-in for
			// an unported resolve. The 6-arg form INVENTS a mode and says so on stderr.
			char a[ 64 ], t[ 64 ]; int dmin = 1, dmax = 3, tohit = 0, bonus = 0;
			int n = sscanf( line, " attack %63s %63s %d %d %d %d", a, t, &dmin, &dmax, &tohit, &bonus );
			F3::SpawnedEntity* ea = findEnt( a );
			F3::SpawnedEntity* et = findEnt( t );
			if ( n < 2 || !ea || !et ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: attack needs two spawned entities\n", lineno );
				rc = 1; break;
			}
			if ( g_atkN >= kMaxActions ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: action pool\n", lineno ); rc = 1; break; }
			// T-071 command-boundary drain: the shipped Server packet pump DRAINS the queue
			// before issuing a replacement order (virt_meth_0x556ea0: case 0x3C drain @0x5576c8
			// -> create @0x5576d8 -> funnel @0x5576f7; case 0x1A drain @0x557233 -> @0x557267;
			// case 0x3B is the stop-packet's own idle rebase). The harness is our command pump,
			// so `attack` takes that shape; the script-native shape (F3::Attack enqueues WITHOUT
			// draining and head-blocks behind the resident - run H's observed truth) is the `push`
			// verb below. Accumulation/D-2 therefore drives through `push`, not repeated attack.
			F3::ClearActionQueue( ea );
			F3::AttackModeRecord m = F3::GameActionAttack::MakeUnarmedMode( dmin, dmax, 1, tohit );
			m.flatDamageBonus = bonus;   // MakeUnarmedMode hard-codes this to 0 (GameActionAttack.cpp:81)
			F3::GameActionAttack& o = g_atks[ g_atkN++ ];
			const F3::AttackModeRecord* mp = 0;
			if ( n >= 6 ) {
				fprintf( stderr, "SIM-DRIVER NOTE line %d: mode row INJECTED by the driver "
				         "(dmin/dmax/tohit/bonus are port-side stands-in for the unported "
				         "weapon resolve, not script data - WildYak T-037)\n", lineno );
				mp = &m;
			}
			// Shipped native also leaves +0x5E/+0x65 timers 0 and +0x78 = 9; Start() nowMs=0
			// matches the first, the 9->0 preflight is port policy we cannot bypass from here
			// (documented deviations 2/3 below).
			o.Start( ea, et, 0u, mp );
			if ( !F3::EnqueueEntityAction( ea, &o, 4 ) ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: insert seam refused (block full, D-2)\n", lineno );
				rc = 1; break;
			}
			// parity with the shipped seam: the owner back-link must overwrite the caller's claim
			if ( o.actor != ea ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: seam did not write owner at +0x14\n", lineno );
				rc = 1; break;
			}
		} else if ( strcmp( verb, "attack-pos" ) == 0 ) {
			// T-047: models the PACKET-91 place-attack door (rec +0x48 hasPos + XYZ at
			// +0x4C/+0x50/+0x54, T-044) — NEVER the script-native Attack() (T-037: natives
			// do not set +0x48). Accept guard attacker && (hasPos || target): no victim needed.
			char a[ 64 ]; float x = 0.0f, y = 0.0f, z = 0.0f;
			if ( sscanf( line, " attack-pos %63s %f %f %f", a, &x, &y, &z ) != 4 ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: attack-pos needs <actor> <x> <y> <z>\n", lineno );
				rc = 1; break;
			}
			F3::SpawnedEntity* ea = findEnt( a );
			if ( !ea ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: attack-pos actor %s not spawned\n", lineno, a );
				rc = 1; break;
			}
			if ( g_atkN >= kMaxActions ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: action pool\n", lineno ); rc = 1; break; }
			F3::ClearActionQueue( ea );   // command-boundary drain (see the `attack` note;
			                              // 0x5576c8 -> 0x5576f7 is literally the move-packet
			                              // shape this harness verb models)
			F3::GameActionAttack& o = g_atks[ g_atkN++ ];
			o.SetPositionOverride( x, y, z );   // meth_0x401100 equivalent (packet handler order)
			o.Start( ea, 0, 0u, 0 );           // attacker-only guard, 0x46decb shape; mode m=0:
			                                   // ctor unarmed defaults (same stand-in as bare attack)
			if ( !F3::EnqueueEntityAction( ea, &o, 4 ) ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: insert seam refused (block full, D-2)\n", lineno );
				rc = 1; break;
			}
			if ( o.actor != ea ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: seam did not write owner at +0x14\n", lineno );
				rc = 1; break;
			}
		} else if ( strcmp( verb, "wait" ) == 0 ) {
			char who[ 64 ]; int dur = 0;
			if ( sscanf( line, " wait %63s %d", who, &dur ) != 2 ) { rc = 1; break; }
			F3::SpawnedEntity* e = findEnt( who );
			if ( !e || g_waitN >= kMaxActions ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: wait\n", lineno ); rc = 1; break; }
			F3::ClearActionQueue( e );   // command-boundary drain (case 0x1A shape - the engine
			                             // wait/anim order replaces the queue; 0x557233 -> 0x557267)
			F3::GameActionWait& w = g_waits[ g_waitN++ ];
			w.durationMs = (unsigned)dur; w.startMs = 0u;   // lazily stamped by the drain
			if ( !F3::EnqueueEntityAction( e, &w, 2 ) ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: wait insert refused\n", lineno ); rc = 1; break; }
		} else if ( strcmp( verb, "push" ) == 0 ) {
			// T-071: the SCRIPT-NATIVE shape - F3::Attack (0x459f5d) enqueues through the
			// funnel WITHOUT draining (new 0xC8 -> ctor 0x403e40 -> meth_0x403fa0 -> funnel,
			// no meth_0x54c440 call anywhere on the path). Behind an idle resident this
			// head-blocks, which is exactly why run H's scripted Attack never fired. Repeated
			// `push` is the honest D-2 accumulator: depth grows to 3 ORDERS + 1 resident and
			// the 4th order hits our first-block wall that the shipped deque would overflow
			// into a second block (0x54ea54 appends; harness item 22; T-070 §4 re-expression).
			char a[ 64 ], t[ 64 ]; int dmin = 1, dmax = 3, tohit = 0, bonus = 0;
			int n = sscanf( line, " push %63s %63s %d %d %d %d", a, t, &dmin, &dmax, &tohit, &bonus );
			F3::SpawnedEntity* ea = findEnt( a );
			F3::SpawnedEntity* et = findEnt( t );
			if ( n < 2 || !ea || !et ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: push needs two spawned entities\n", lineno );
				rc = 1; break;
			}
			if ( g_atkN >= kMaxActions ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: action pool\n", lineno ); rc = 1; break; }
			if ( ea->actions[0].kind != 0 && ea->actions[1].kind != 0 &&
			     ea->actions[2].kind != 0 && ea->actions[3].kind != 0 ) {
				// deviation D-2, now INCLUDING the resident: 3 orders + resident fill the
				// first block; a shipped deque grows past it, our port refuses - fail loudly
				fprintf( stderr, "SIM-DRIVER FAIL line %d: entity %s queue block full (D-2 refusal; 3 orders + unpumped idle arm)\n", lineno, a );
				rc = 1; break;
			}
			F3::AttackModeRecord m = F3::GameActionAttack::MakeUnarmedMode( dmin, dmax, 1, tohit );
			m.flatDamageBonus = bonus;
			F3::GameActionAttack& o = g_atks[ g_atkN++ ];
			const F3::AttackModeRecord* mp = 0;
			if ( n >= 6 ) {
				fprintf( stderr, "SIM-DRIVER NOTE line %d: mode row INJECTED by the driver (same "
				         "stand-in policy as `attack`, WildYak T-037)\n", lineno );
				mp = &m;
			}
			o.Start( ea, et, 0u, mp );
			if ( !F3::EnqueueEntityAction( ea, &o, 4 ) ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: insert seam refused (block full, D-2)\n", lineno );
				rc = 1; break;
			}
			if ( o.actor != ea ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: seam did not write owner at +0x14\n", lineno );
				rc = 1; break;
			}
		} else if ( strcmp( verb, "move" ) == 0 ) {
			fprintf( stderr, "SIM-DRIVER SKIP line %d: move requires a loaded map + PathManager/grid "
			                 "(SpawnMove mapfile path not wired here) - reported, not faked\n", lineno );
		} else if ( strcmp( verb, "turn" ) == 0 ) {
			char who[ 64 ];
			if ( sscanf( line, " turn %63s", who ) == 1 ) {
				F3::SpawnedEntity* e = findEnt( who );
				if ( e ) { g_cm.AddToTurns( e ); }
			}
		} else if ( strcmp( verb, "seed" ) == 0 ) {
			// G-FLAKY (b) root cause: the shipped dice draw sub_48CCC0 lazily seeds its LCG
			// from the WALL CLOCK IN SECONDS on the first draw (0x48cccb call __time32(0);
			// CombatMath.cpp ShippedRand -> time(NULL)). An unseeded scripted boot's to-hit
			// rolls therefore depend on which second the run happens in: same-wallclock runs
			// are byte-identical (why the suite passed) while a second-flip reroll lands or
			// misses randomly across invocations (suite saw victim hp 28 vs no land at all,
			// ~1-in-10). Shipped fidelity is NOT changed: unpinned boots still lazy-seed from
			// time(NULL); this verb is a HARNESS INPUT, same category as `run`'s stepMs - the
			// harness owns every unsimulated clock, and time is one. SeedShippedRand is the
			// documented test-only reseed mirroring SetStartupTime (0x48CCA0: state, flag=1).
			unsigned int s = 0;
			if ( sscanf( line, " seed %u", &s ) != 1 ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: seed needs a state operand\n", lineno );
				rc = 1; break;
			}
			F3::SeedShippedRand( s );
		} else if ( strcmp( verb, "run" ) == 0 ) {
			int frames = 0, stepMs = 16;
			if ( sscanf( line, " run %d %d", &frames, &stepMs ) < 1 || frames <= 0 || stepMs <= 0 ) {
				fprintf( stderr, "SIM-DRIVER FAIL line %d: run needs frames [stepMs]\n", lineno );
				rc = 1; break;
			}
			runFrames( frames, stepMs );
		} else if ( strcmp( verb, "digest" ) == 0 ) {
			char path[ 128 ];
			if ( sscanf( line, " digest %127s", path ) != 1 ) { rc = 1; break; }
			g_digest = fopen( path, "w" );
			if ( !g_digest ) { fprintf( stderr, "SIM-DRIVER FAIL: cannot open digest %s\n", path ); rc = 1; break; }
			F3::Sim::Sources src; src.combat = &g_cm; src.paths = g_pm;
			F3::Sim::WriteHeader( g_digest, src );
		} else if ( strcmp( verb, "peek" ) == 0 ) {
			// P-REV: sample the CURRENT state without ticking - the insert window. Shipped
			// R3/R4b samples live at the funnel bp (between enqueue and the walk), so pairing
			// the transient idle arm (0x5a9070: completes first tick) needs the same sampling
			// instant. The frame number is consumed to keep numbering strictly increasing.
			if ( g_digest ) {
				F3::Sim::Sources src; src.combat = &g_cm; src.paths = g_pm;
				F3::Sim::WriteFrame( g_digest, src, g_frame, (unsigned)g_clockMs,
				                     (unsigned)( g_clockMs / 1000u ), 0, 0 );
				++g_frame;
			}
		} else if ( strcmp( verb, "end" ) == 0 ) {
			if ( g_digest ) {
				fprintf( g_digest, "# END frames=%u every=1 clockMs=%lu\n", g_frame, g_clockMs );
				fclose( g_digest ); g_digest = 0;
			}
		} else {
			fprintf( stderr, "SIM-DRIVER FAIL line %d: unknown verb '%s'\n", lineno, verb );
			rc = 1; break;
		}
	}
	fclose( s );
	if ( g_digest ) { fclose( g_digest ); }
	if ( rc == 0 ) { printf( "SIM-DRIVER OK frames=%u clockMs=%lu\n", g_frame, g_clockMs ); }
	else          { printf( "SIM-DRIVER FAIL (script error, line %d)\n", lineno ); }
	return rc;
}
