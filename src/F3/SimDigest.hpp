// Copyright 2024 OpenJE

// T-026 - per-frame simulation digest: one line per frame of the game state that is decided
// independently of whether anything is drawn, so "identical behaviour" becomes a file you diff
// instead of a claim you argue about. Header-only deliberately: a .cpp here would need a
// CMakeLists.txt edit, and the build config is not this lane's to touch (T-02x wave rule).
//
// PROVES: our sim is self-consistent - same input gives the same digest, and a change in a
// simulated input moves the digest and names the field that moved.
// DOES NOT PROVE: that it matches the original. That needs T-028 plus a bootable F3.exe. The
// digest states this in its own header line rather than leaving it in a commit message.

#ifndef F3_SIMDIGEST_HPP_
#define F3_SIMDIGEST_HPP_

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "JE/PathManager.hpp"
#include "F3/CombatManager.hpp"
#include "F3/MapEntitySpawn.hpp"

namespace F3 {
	namespace Sim {
		// Per-entity fields stop after this many actors and the line says so (entsMore=1),
		// because a stable hash over truncated state reads like a stable world.
		const int kMaxEntitiesInLine = 12;

		// Every reported field resolves through ONE of these. A NULL source is never skipped
		// silently - it prints `key=NA:<reason>`. That is the whole difference between a digest
		// that hides a missing seam and one that admits it.
		struct Sources {
			CombatManager*   combat;   // turn queue, world+0x8C in the original
			JE::PathManager* paths;    // A* job list, world+0xF0 (the steerer)
			Sources() : combat( 0 ), paths( 0 ) {}
		};

		// Line 1 is the contract, printed as data so the header and the payload cannot disagree
		// and `head -1` answers "what is in here" without reading the code.
		inline void WriteHeader( FILE* f, const Sources& src ) {
			fprintf( f,
				"# SIM-DIGEST v1 include=frame clockDelta ents <eN=name,pos,yaw,hp,st,ak> "
				"pathJobs turns turnOrder draws\n"
				"# entity-fields=name pos[3] yaw hp=cur/max st=entityState,moving,busy,acted "
				"ak=ACTION CLASS NAMES: our side maps port kind->class in code, the original side maps "
				"[action+0x00] vftptr->class via artifacts/F3.json, UNKNOWN_VT=<ea> when unresolved and "
				"those are counted, never named | cap %d entities, excess -> entsMore=1\n"
				"# COMPARED=frame clockDelta ents eN.name/pos/yaw/hp/st/ak pathJobs turns turnOrder draws | "
				"CONTEXT(after ' ctx:')=tickMs clock (ABSOLUTE) - printed, captured, NOT hashed: uptime and the "
				"shipped first-Update leap make both incomparable across processes, so the digest carries "
				"clockDelta (seconds elapsed since frame 0) as the comparable axis instead; promote a "
				"CONTEXT field in the comparator, never by "
				"re-running the session | caller-supplied=clock tickMs - a headless harness owns simulated time; the app "
				"path passes Host::TickMilliseconds (timeGetTime, machine uptime) and the world "
				"clock Gregorian::TotalSeconds, which STARTS AT 0 and grows by Update(ms)-last "
				"(mbr_0x48 initialises to 0 against an ABSOLUTE tick, so the first pump adds the "
				"whole uptime - see docs/f3/SimDigest.md); cross-context comparisons must pin both\n"
				"# EXCLUDED=reaction-anim-id gfx-texture-slots gui-fragments voice-roll "
				"effect-rolls (presentation-side)\n"
				"# WHY-draws-IS-IN: the reaction id stays out, but one global sequential LCG feeds "
				"both - 8 of the 18 shipped draw sites sit in sub_45C140 (docs/f3/CombatRngSeam.md) "
				"- so damage-draw COUNT shifts the reaction outcome. Counting draws is what keeps "
				"that coupling named instead of implicit; excluding the reaction id alone would be "
				"an unnamed coupling, not a separation.\n"
				"# NA-means=the seam is not ported, NEVER =unchanged\n"
				"# SEED-LIMIT: a parity run pins ONE captured LCG state, so it proves SAME-STATE parity, "
				"NOT same-seed-from-clock parity: identical under a pinned state and divergent in real "
				"time is a possible and meaningful outcome\n"
				"# PROVES=self-consistency of our sim. DOES NOT PROVE=parity with F3.exe (needs "
				"T-028 + a bootable exe).\n",
				kMaxEntitiesInLine );
			// And what the caller actually handed us: a digest that ran with half its sources
			// missing must not look identical to a healthy one.
			fprintf( f, "# SOURCES combat=%s paths=%s\n",
			         src.combat ? "ok" : "NA", src.paths ? "ok" : "NA" );
			fflush( f );
		}

		inline unsigned Fnv1a( const char* s ) {
			unsigned h = 2166136261u;
			for ( ; *s; ++s ) { h = ( h ^ (unsigned char)*s ) * 16777619u; }
			return h;
		}

		// Append with a bound that survives both compilers. VC7 (MSVC 2002) has no snprintf, only
		// _vsnprintf, and _vsnprintf does NOT NUL-terminate when it truncates - so termination is
		// done here unconditionally rather than assumed.
		inline int Append( char* buf, int cap, int used, const char* fmt, ... ) {
			if ( used < 0 || used >= cap - 1 ) { return used < 0 ? 0 : cap - 1; }
			va_list ap;
			va_start( ap, fmt );
			int w;
#ifdef _MSC_VER
			w = _vsnprintf( buf + used, (size_t)( cap - 1 - used ), fmt, ap );
#else
			w = vsnprintf( buf + used, (size_t)( cap - 1 - used ), fmt, ap );
#endif
			va_end( ap );
			int next = used + ( w < 0 ? cap - 1 - used : w );
			if ( next > cap - 1 ) { next = cap - 1; }
			buf[ next ] = 0;
			return next;
		}

		// One frame. `draws`/`drawsSource` are arguments rather than globals because the source
		// genuinely differs by context: the harness injects a counting generator (T-027's RandFn
		// seam), while the app path draws from the global LCG, which has no draw counter yet -
		// adding one means editing CombatMath.cpp, a file another lane is editing right now. The
		// digest therefore prints drawsSrc=injected-counter or NA:global-stream-has-no-counter.
		// The frame-0 clock, so `clockDelta` (seconds ELAPSED in this run) can be the comparable
		// field. The ABSOLUTE clock cannot: the shipped first-Update leap makes it
		// uptime-shaped (docs/f3/WorldTickPump.md), so two runs of one session - let alone the
		// original and ours - never share it. Elapsed time is the axis a parity diff needs.
		inline unsigned& BaseClockRef() {
			static unsigned b = 0xFFFFFFFFu;   // 0xFFFFFFFF = "not captured yet"
			return b;
		}
		inline void ResetBaseClock() { BaseClockRef() = 0xFFFFFFFFu; }
		inline bool BaseClockPending() { return BaseClockRef() == 0xFFFFFFFFu; }
		inline void SetBaseClock( unsigned c ) { BaseClockRef() = c; }

		// Our action kinds are a PORT tag, not a shipped field: the shipped deque at Entity+0x304 holds
		// pointers to action OBJECTS. Comparing those pointers is comparing allocator history; dropping
		// the field leaves the shipped action queue permanently unverified, which is the thing the oracle
		// exists to check. So BOTH sides print a CLASS NAME: ours from this table (a mapping we own and
		// can check), the original's from [action+0x00] vftptr -> class via artifacts/F3.json. Anything
		// unresolved prints UNKNOWN_VT=<ea> and is COUNTED, never named - if the unresolved bucket held
		// every combat action that would be a finding about the DB, not about parity. (T-031 ruling;
		// harness item 23: do not invent a mapping to make a field comparable.)
		inline const char* ActionKindName( int kind ) {
			switch ( kind ) {
			case 0:  return "-";
			case 1:  return "GameActionMoveTo";
			case 2:  return "GameActionWait";
			case 3:  return "GameActionOperateDoor";
			case 4:  return "GameActionAttack";
			case 5:  return "GameActionDie";
			default: return "UNKNOWN_KIND";   // never a silent name
			}
		}

		inline void WriteFrame( FILE* f, const Sources& src, unsigned frame, unsigned tickMs,
		                        unsigned clockSec, int draws, const char* drawsSource ) {
			// Room for 12 entities x 4 class names. Truncation is still reported (truncated=1 is
			// hashed) but it must not be the normal case, and names are longer than digits.
			char line[ 4096 ];
			const int cap = (int)sizeof( line );
			int n = 0;
			line[0] = 0;

			// CONTEXT vs COMPARED - not cosmetics. tickMs is timeGetTime() (machine uptime) and the
			// shipped frame-0 leap makes `clock` uptime-shaped too, so a digest that HASHES tickMs can
			// never be byte-compared across two runs of one session, which is precisely what the parity
			// oracle (T-031) must do. The value is still PRINTED because it is real state and elapsed
			// time has to be attributable. Convention: text before " ctx:" is hashed, text after it is
			// captured-not-compared and can be PROMOTED by changing the comparator, never by re-running
			// a debugger session.
			unsigned& base = BaseClockRef();
			if ( base == 0xFFFFFFFFu ) { base = clockSec; }   // captured on the first frame written
			n = Append( line, cap, n, "frame=%u clockDelta=%u", frame, clockSec - base );

			int count = 0;
			for ( int i = 0; i < 128; ++i ) { if ( g_spawnedEntities[ i ] ) { ++count; } }
			n = Append( line, cap, n, " ents=%d", count );

			int shown = 0;
			for ( int i = 0; i < 128 && shown < kMaxEntitiesInLine; ++i ) {
				const SpawnedEntity* e = g_spawnedEntities[ i ];
				if ( !e ) { continue; }
				n = Append( line, cap, n, " e%d=%s", shown, e->name[0] ? e->name : "-" );
				n = Append( line, cap, n, ",pos=%.2f/%.2f/%.2f",
				             (double)e->pos[0], (double)e->pos[1], (double)e->pos[2] );
				n = Append( line, cap, n, ",yaw=%.2f", (double)e->yaw );
				n = Append( line, cap, n, ",hp=%d/%d", e->statCur, e->statMax );
				n = Append( line, cap, n, ",st=%d,%d,%d,%d", (int)e->entityState,
				             (int)e->movingFlag, (int)e->busy0x314, (int)e->actedLatch0x319 );
				n = Append( line, cap, n, ",ak=%s/%s/%s/%s",
				             ActionKindName( e->actions[0].kind ), ActionKindName( e->actions[1].kind ),
				             ActionKindName( e->actions[2].kind ), ActionKindName( e->actions[3].kind ) );
				++shown;
			}
			if ( count > shown ) { n = Append( line, cap, n, " entsMore=1" ); }

			n = Append( line, cap, n, " pathJobs=" );
			if ( src.paths ) { n = Append( line, cap, n, "%d", src.paths->PendingJobCount() ); }
			else { n = Append( line, cap, n, "NA:no-path-manager-in-this-context" ); }

			n = Append( line, cap, n, " turns=" );
			if ( src.combat ) {
				n = Append( line, cap, n, "%d turnOrder=", src.combat->TurnCount() );
				char order[ 256 ];
				order[0] = 0;
				// Order-sensitive on purpose: WHO ACTS WHEN is the gameplay fact, so the same
				// actors in a different order must produce a different digest.
				src.combat->TurnOrderSignature( order, (int)sizeof( order ) );
				n = Append( line, cap, n, "%s", order[0] ? order : "-" );
			} else {
				n = Append( line, cap, n, "NA:no-combat-manager-arg" );
			}

			// No number when there is no counter: `draws=0` would read as "nothing drew this
			// frame", which is a false statement about state, not a missing seam.
			if ( drawsSource ) { n = Append( line, cap, n, " draws=%d drawsSrc=%s", draws, drawsSource ); }
			else { n = Append( line, cap, n, " draws=NA:no-counter-on-global-stream" ); }

			// A truncated frame is a changed contract, so the suffix is hashed too, not just printed.
			char out[ 4300 ];
			strcpy( out, line );
			if ( n >= cap - 1 ) { strcat( out, " truncated=1" ); }

			// ORDER MATTERS AND IS THE WHOLE POINT: the hash covers everything printed BEFORE
			// " ctx:". Hashing the CONTEXT span instead (which an earlier draft of this file did)
			// makes two runs of the same session differ on every line and quietly defeats the
			// cross-run comparison this format exists to enable - so the CONTEXT fields are
			// appended AFTER the hash is computed.
			fprintf( f, "%s fnv=%08X ctx: tickMs=%u clock=%u\n", out, Fnv1a( out ), tickMs, clockSec );
			fflush( f );
		}
	} // namespace Sim
} // namespace F3

#endif // F3_SIMDIGEST_HPP_
