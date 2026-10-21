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

	unsigned long g_clockMs  = 0;     // the harness owns simulated time (SimDigest header)
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
			g_cm.Tick( stepMs, (unsigned)( g_clockMs / 1000u ) );
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
			if ( ea->actions[0].kind != 0 && ea->actions[1].kind != 0 &&
			     ea->actions[2].kind != 0 && ea->actions[3].kind != 0 ) {
				// deviation D-2: block of four is full and the seam REFUSES (correct for the port,
				// but a driver that ignores it under-tests: fail the run, do not queue a phantom order
				fprintf( stderr, "SIM-DRIVER FAIL line %d: entity %s queue block full (D-2 refusal; drain before more attacks)\n", lineno, a );
				rc = 1; break;
			}
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
		} else if ( strcmp( verb, "wait" ) == 0 ) {
			char who[ 64 ]; int dur = 0;
			if ( sscanf( line, " wait %63s %d", who, &dur ) != 2 ) { rc = 1; break; }
			F3::SpawnedEntity* e = findEnt( who );
			if ( !e || g_waitN >= kMaxActions ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: wait\n", lineno ); rc = 1; break; }
			F3::GameActionWait& w = g_waits[ g_waitN++ ];
			w.durationMs = (unsigned)dur; w.startMs = 0u;   // lazily stamped by the drain
			if ( !F3::EnqueueEntityAction( e, &w, 2 ) ) { fprintf( stderr, "SIM-DRIVER FAIL line %d: wait insert refused\n", lineno ); rc = 1; break; }
		} else if ( strcmp( verb, "move" ) == 0 ) {
			fprintf( stderr, "SIM-DRIVER SKIP line %d: move requires a loaded map + PathManager/grid "
			                 "(SpawnMove mapfile path not wired here) - reported, not faked\n", lineno );
		} else if ( strcmp( verb, "turn" ) == 0 ) {
			char who[ 64 ];
			if ( sscanf( line, " turn %63s", who ) == 1 ) {
				F3::SpawnedEntity* e = findEnt( who );
				if ( e ) { g_cm.AddToTurns( e ); }
			}
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
