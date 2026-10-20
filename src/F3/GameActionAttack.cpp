// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <math.h>
#include "F3/GameActionAttack.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/Scripting.hpp"

namespace F3 {
	RandFn GameActionAttack::s_rand = 0;   // NULL -> std rand inside CombatDice
	RandFn GameActionAttack::s_voiceRand = 0;

	const int kReactionAnimMoving[2] = { 33, 34 };   // 0x45C331: 0x21 + setne(wrapByte)
	const int kReactionAnimIdle[2]   = { 37, 40 };   // 0x45C355: 0x25 + (wrapByte ? 3 : 0)
	const int kReactionAnimIdleSet[2] = { 38, 41 };  // 0x45C349: 0x26 + (wrapByte ? 3 : 0)

	// Apply's hit-angle chain, instruction for instruction (0x45C29E..0x45C2E4). The constants
	// were read out of the shipped PE, not inferred:
	//   0x6CB56C = 0.0f   | 0x660828 = -180.0f | 0x660824 = 180.0f | 0x660820 = 360.0f
	// The four compares are x87 `fcom/fcomp` + `fnstsw` + `test ah,{5,1}` + `jp/jz/jnz`, i.e.
	// they read C3:C2:C0 as a PAIRWISE-parity/compare result, which is what this function
	// reproduces. Written 1 only on the three out-of-band paths 0x45C2B6->D9, 0x45C2D2, 0x45C2DB.
	//
	// SHIPPED DEFECT (filed, not fixed - see T-012 notes): the byte this returns lives at
	// Apply's var_B1 ([esp+0Fh) and the ONLY stores to that slot are the two `mov byte,1` at
	// 0x45C2D2/0x45C2DB. Nothing ever zeroes it: the `mov [esp+0Fh],bl` at 0x45C1B6 targets
	// var_A5 (frame offset 0x1B), a different slot, because 0x45C22C's `sub esp,0Ch` shifts the
	// window by 0xC between the two. So on every in-band path - and on the two early exits at
	// 0x45C1BA/0x45C1CB, which jump straight to 0x45C2E4 past the whole chain - the shipped
	// code reads an UNINITIALISED stack byte, in practice whatever the previous Apply call left
	// at that address. That is not a "flag stays 0" default; it is stack reuse.
	// `unwritten` is how we model the choice without claiming it as shipped fact: pass 0 for a
	// deterministic port, pass the previous hit's byte to reproduce the stickiness.
	unsigned char HitAngleWrapFlag( float angle, unsigned char unwritten )
	{
		// fcom 0.0 / test ah,5 / jp  @0x45C29E: parity of (C3,C2,C0) selects the sign branch.
		if ( angle >= 0.0f || angle != angle ) {   // >=0 or unordered -> 0x45C2B8
			// fcom 180.0 / test ah,1 / jne @0x45C2B8 -> in-band, byte NOT written
			if ( angle < 180.0f ) { return unwritten; }
			// fcomp 360.0 / test ah,5 / jp @0x45C2C5 -> >=360 in-band, byte NOT written
			if ( angle >= 360.0f ) { return unwritten; }
			return 1;                                // 0x45C2D2: 180 <= angle < 360
		}
		// fcom -180.0 / test ah,1 / jz @0x45C2AB -> 0x45C2D9 writes 1 when angle >= -180
		if ( angle >= -180.0f ) { return 1; }
		return unwritten;                          // angle < -180 falls through to 0x45C2B8
	} // HitAngleWrapFlag

	// The selector block at 0x45C30C..0x45C35C, including its outer gate: a non-first record of
	// a batch had eff+0xAD = 1 (finisher @0x4038D7..0x4038F7), and 0x45C312's branch jumps past
	// the QueueAnimEvent call at 0x45C35A entirely - secondary hits are DELIBERATELY SILENT.
	int ReactionAnimFor( bool firstInBatch, bool targetMoving, bool flurryGe2,
	                     unsigned char wrapByte )
	{
		if ( !firstInBatch ) { return 0; }   // 0x45C312: no QueueAnimEvent at all
		const int band = ( wrapByte != 0 ) ? 1 : 0;
		if ( targetMoving ) { return kReactionAnimMoving[ band ]; }        // 0x45C326
		return flurryGe2 ? kReactionAnimIdleSet[ band ]                    // 0x45C342/0x45C349
		                 : kReactionAnimIdle[ band ];                      // 0x45C34E/0x45C355
	} // ReactionAnimFor

	GameActionAttack::GameActionAttack()
	  : actor( 0 ), target( 0 ), calledShotPart( 9 ), reach( 1 ),
	    firstStrikeDueMs( 0 ), nextSubAttackDueMs( 0 ), subAttackPending( false ),
	    volleyContinuing( false ), ammoDeducted( false ), seriesRemaining( 0 ),
	    moveSpeed( 4.0f ), unarmed0x71( true ), queueCount( 0 ), finished( false ) {
		for ( int i = 0; i < kQueueCap; ++i ) {
			queue[i].dmg = 0; queue[i].damageType = 0; queue[i].material = 0;
			queue[i].flurryGe2 = false;
			queue[i].firstInBatch = true;
			queue[i].unarmedSnapshot = false;
		}
		mode.style = 0; mode.damageType = 4; mode.primaryAmmoCost = 0;
		mode.flatDamageBonus = 0; mode.toHitBonus = 0; mode.attackCount = 1;
		mode.damageMin = 1; mode.damageMax = 3; mode.ammoExtra = 0;
		mode.ammoReqType = 0;   // +0x18 (uninit would be read by the unmodeled bonus branch)
	}

	AttackModeRecord GameActionAttack::MakeUnarmedMode( int dmgMin, int dmgMax,
	                                                    int subAttacks, int toHitBonus ) {
		AttackModeRecord m;
		m.style = 0; m.damageType = 4; m.primaryAmmoCost = 0; m.flatDamageBonus = 0;
		m.toHitBonus = toHitBonus; m.attackCount = subAttacks;
		m.damageMin = dmgMin; m.damageMax = dmgMax; m.ammoExtra = 0;
		m.ammoReqType = 0;            // +0x18: 0 = no ammo-requirement kind (see C-40 branch)
		return m;
	}

	void GameActionAttack::Start( SpawnedEntity* who, SpawnedEntity* against,
	                              unsigned nowMs, const AttackModeRecord* m ) {
		tracing::instrument( tracing::LOCATION, "actor=%p target=%p", who, against );
		actor  = who;
		target = against;
		if ( m ) {
			mode = *m;
		}
		// called-shot preflight (session 21 §2 "unset(9) -> cancel+error"): the shipped
		// resolve stage forces a concrete part before the action arms; we mirror that
		// resolution (9 -> 0 torso-default) instead of replicating a can't-happen cancel.
		if ( calledShotPart == 9 ) {
			calledShotPart = 0;
		}
		if ( !actor || !target ) {
			finished = true;
			return;
		}
		// QueueAnimEvent(7=ToCombat,0.0) lead seam: with no anim layer, first-strike is
		// due immediately (original used the returned duration; dev note in hpp).
		firstStrikeDueMs = nowMs;
		nextSubAttackDueMs = nowMs;
		subAttackPending = false;
		seriesRemaining = mode.attackCount;
		ammoDeducted = false;
	}

	int GameActionAttack::Update( unsigned gameMs ) {
		tracing::instrument( tracing::LOCATION, "ms=%u", gameMs );
		if ( finished ) {
			return 1;
		}
		// target validity (shipped: same-map + alive checks with error strings; entity
		// list is single-map in this slice -> map check trivially true)
		if ( !target || target->entityState == kStateDead
		     || target->entityState == kStateRemoved || target->entityState == kStateDying ) {
			finished = true;
			return 1;
		}

		// distance + auto-move-to-range (deviation: direct step, original issues MoveTo)
		const float dx = target->pos[0] - actor->pos[0];
		const float dy = target->pos[1] - actor->pos[1];
		const float dz = target->pos[2] - actor->pos[2];
		const float dist = sqrtf( dx * dx + dy * dy + dz * dz );
		if ( dist > (float)reach ) {
			const float step = moveSpeed * 0.016f;   // per-frame proxy; pump ms unknown here
			const float inv = step / dist;
			actor->pos[0] += dx * ( inv < 1.0f ? inv : 1.0f );
			actor->pos[1] += dy * ( inv < 1.0f ? inv : 1.0f );
			actor->pos[2] += dz * ( inv < 1.0f ? inv : 1.0f );
			return 0;
		}

		// first strike gate (modeRec[+68] ready timestamp == firstStrikeDue here)
		if ( (int)( gameMs - firstStrikeDueMs ) < 0 ) {
			return 0;
		}

		// sub-attack series: on due, DRAIN the whole pending series (§2 while-loop).
		// DM-beat stand-in: time-based release (see hpp dev notes).
		if ( seriesRemaining > 0 && (int)( gameMs - nextSubAttackDueMs ) >= 0 ) {
			// sub-attack: to-hit then damage, through the SHIPPED fork on the action's +0x71
			// unarmed bool (0x4040a8 / 0x4040d4). Usable-object targets take the +0xA4 DT row and
			// are allowed to keep part 9 (0x409fec `&& !GameUsableObject`).
			while ( seriesRemaining > 0 ) {
				const bool usableTarget =
				    ( target->classIdTag == kClassGameUsableObject );
				const int chance = CalculateToHitMelee( mode, actor->combat,
				                                        target->combat, ammo, dist,
				                                        reach, calledShotPart );
				// RollToHit_0x402260 (session 17 §1): rand = RandRange(0,100) INCLUSIVE (101
				// outcomes, not 1..100), HIT when chance >= rand (chance capped 95 -> roll 100
				// always misses).
				const int roll = CombatDice( 0, 100, s_rand );
				if ( chance >= roll ) {
					const int dmg = unarmed0x71
					    ? CalculateUnarmedDamage( mode, actor->combat, target->combat,
					                              calledShotPart, usableTarget, s_rand )
					    : CalculateDamage( mode, target->combat, ammo,
					                       calledShotPart, s_rand );
					if ( queueCount < kQueueCap ) {   // "Damage for %d queued."
						QueuedHit& qh = queue[ queueCount++ ];
						qh.dmg = dmg;
						qh.damageType = mode.damageType;        // effect +0xA0 stand-in
						qh.material = 0;                        // effect +0xA4 (ammo/proto
						                                       // material - weapon unported)
						// +0xAC / +0xAD are assigned by the FINISHER (meth_0x403770 @0x4038D7..0x4038F7):
						// the first record of the list gets +0xAD = 0 and +0xAC = (marks >= 2); every
						// later record gets +0xAD = 1, which makes Apply skip the reaction block whole.
						qh.firstInBatch = ( queueCount == 1 );
						// record+0x19 is a SNAPSHOT of action+0x71 taken AT queue-insert (@0x4041A1),
						// not a live read: a mid-volley weapon swap changes action+0x71 but cannot
						// change records already queued, so the mark must be copied per record here.
						qh.unarmedSnapshot = unarmed0x71;
						qh.flurryGe2 = false;
					}
				}
				--seriesRemaining;
			}
			// multi-shot continuation (modeRec[+12] != 1 → next volley pending)
			if ( mode.attackCount != 1 && !volleyContinuing ) {
				// sustained fire: re-arm series next Update (chained GameActionAttack in
				// the original; we re-arm in place - same effect for the queue model)
				volleyContinuing = true;
				seriesRemaining = mode.attackCount;
				nextSubAttackDueMs = gameMs + 1000u;   // interval stand-in (modeRec[+68])
			}
		}

		// finisher (meth_0x403770): drain the damage queue into target HP. Deviation:
		// direct HP write; original creates GameEffectDamage per hit (+0x9C payload) and
		// ATTACHES to the target (updater layer unported).
		if ( seriesRemaining == 0 && !volleyContinuing ) {
		// pre-loop census (0x4037D2..0x403804): count records whose queued +0x19 mark
		// is set. +0xAC is TRUE only for >= 2, i.e. an UNARMED flurry - counting records
		// instead would wrongly upgrade a 3-hit WEAPON volley.
		int unarmedMarks = 0;
		for ( int m = 0; m < queueCount; ++m ) { if ( queue[m].unarmedSnapshot ) { ++unarmedMarks; } }
		const bool flurryGe2 = ( unarmedMarks >= 2 );
			// per queued hit, ordered exactly as GameEffectDamage::Apply (0x45C140 §3):
			for ( int i = 0; i < queueCount; ++i ) {
				const QueuedHit& qh = queue[i];
				// 1. DEAD gate (vft+0x50 == state 7): no double damage
				if ( target->entityState == kStateDead || target->entityState == kStateDying ) {
					break;
				}
				// 2. dmg==0 early-out
				if ( qh.dmg == 0 ) {
					continue;
				}
				// 3. HP update @0x45c1a3 -> SetCurrentHealth min-clamp @0x4629C0
				const int oldHp = target->statCur;
				int cur = oldHp - qh.dmg;
				if ( cur > target->statMax ) { cur = target->statMax; }
				target->statCur = cur < 0 ? 0 : cur;
				// 4. hit angle. Shipped shape (Apply 0x45C243..0x45C29C): the negated delta
				// Euler components are prescaled by flt_649C50 = pi/180, run through the two
				// transform helpers 0x47272B / 0x471357, and combined as atan2 via fpatan. The
				// fpatan RESULT is what the compare chain consumes and no instruction scales it,
				// so the value entering the chain is in RADIANS by x87 architecture: the
				// rad-vs-deg question left open in discoveries/2025-10-20-T010a-behind-test-is-
				// wrap-not-threshold.md §3 is CLOSED, and because |atan2| <= pi < 180 the
				// +/-180 and 360 compares are UNREACHABLE for any real input. The degrees
				// reading is therefore recorded as unreachable, not implemented - there is no
				// 180/pi multiply in the binary to justify one.
				// Deviation kept: we feed the planar position delta, because the delta-Euler
				// source fields and both transform helpers are still unported. Shape faithful,
				// inputs not.
				const float hx = target->pos[0] - actor->pos[0];
				const float hz = target->pos[2] - actor->pos[2];
				const float angle = atan2f( hx, hz );
				// The shipped wrap byte is Apply's var_B1. Nothing in Apply ever zeroes it - the
				// only stores are `mov byte,1` at 0x45C2D2/0x45C2DB - so on an in-band angle the
				// original reads uninitialised stack (stack reuse from the previous Apply call).
				// We pass a deterministic fresh 0; see HitAngleWrapFlag for the defect write-up.
				const unsigned char wrapByte = HitAngleWrapFlag( angle, 0 );
				// 5. reaction anim pick (QueueAnimEvent_0x4673D0 unported -> id stored on the
				// victim as the seam; session 7 §3 step 5, victim-moving byte = Entity+0x3A2)
				const int anim = ReactionAnimFor( qh.firstInBatch, target->movingFlag,
				                                flurryGe2, wrapByte );
				if ( anim != 0 ) { target->queuedAnimEvent = anim; }   // 0 = no event queued at all
				// 6. VFX .veg selection - name resolved and parked on the seam, spawn unported
				target->lastHitVeg = SelectHitVeg( qh.damageType, qh.material, qh.dmg );
				// 7. voice roll (audio ids unported; seam kept for determinism)
				(void)CombatDice( 0, 1, s_voiceRand );
				// 8. OnDamage script event (sub_55A220 payload {dmg, attackerId, DT})
				JE::EntityRunScript( target, "OnDamage", qh.dmg,
				                     actor->netId, qh.damageType );
				// 9. server sync 0x46d020 - net broadcast unported (annotated skip)
				// 10. death conversion @0x45c5b9: NEWLY dead -> queue GameActionDie on victim.
				// The shipped code takes the action from the world-server factory and pushes it via
				// Entity::meth_0x550720; we mirror that with a static ring (no heap, same
				// lifetime as the Wait/OperateDoor pools in GameActionMoveTo.cpp). The victim's
				// OWN pump runs it next tick - the action is not executed inline here.
				if ( target->statCur == 0 && target->entityState == kStateActive ) {
					static GameActionDie s_dies[4];
					static int s_di = 0;
					GameActionDie& die = s_dies[ s_di = ( s_di + 1 ) & 3 ];
					die.damageType = qh.damageType;
					die.material = qh.material;
					die.flagsByte0x28 = qh.flurryGe2 ? 1 : 0;
					PushActionOntoEntity( target, 5, &die );   // mode-5 queued-action accept
				}
			}
			queueCount = 0;
			finished = true;
			return 1;
		}
		return 0;
	}
	const char* SelectHitVeg( int damageType, int material, int dmg ) {
		// session 7 §3 step 6 table (+0xA0 switch, +0xA4 sub-cases)
		switch ( damageType ) {
		case 0:
			if ( material == 5 ) { return "FX_HitBloodHighVelocity_01.veg"; }
			return dmg > 0 ? "FX_HitBloodStandard_01.veg" : "FX_HitRicochetMetal_01.veg";
		case 1:
		case 3:
		case 4:
			return material == 5 ? "FX_HitBloodHighVelocity_01.veg"
			                     : "FX_HitBloodStandard_01.veg";
		case 2:
			return "FX_HitElectrical_01.veg";
		case 5:
			if ( material == 1 ) { return "FX_HitFire_01.veg"; }
			if ( material == 2 ) { return "FX_HitBloodStandard_01.veg"; }
			if ( material == 3 ) { return "FX_HitLaser_01.veg"; }
			return "FX_HitBloodStandard_01.veg";
		default:
			return "FX_HitRicochetMetal_01.veg";
		}
	} // SelectHitVeg

	int GameActionDie::Update( SpawnedEntity* victim ) {
		tracing::instrument( tracing::LOCATION, "dt=%d mat=%d", damageType, material );
		if ( !victim ) {
			return 1;
		}
		// GameActionDie::Execute (0x404e90) step 1: JE::ActionDie -> Entity::Kill_0x551ca0 ->
		// ChangeState(6). Legal transitions only (session 8 §1): 4 -> 6 here; the 6 -> 7 edge is
		// OnDeath's own server-gated work, run by the entity pump (session 12 §2), NOT here.
		if ( victim->entityState == kStateActive ) {
			victim->entityState = kStateDying;
			// death anim by the die "category" dword (session 8 §3): 0 -> 46 (45 when the sub
			// class is 4), 1 -> 44, 2/3/5 -> 47 then 48|51|52, default -> 17|18 + 43. Our
			// category source is the damage type carried in +0x1C.
			victim->queuedAnimEvent = ( damageType == 1 ) ? 44 : 46;
		}
		return 1;   // shipped: anim/loot continuation unported (dev note in hpp)
	}
} // namespace F3
