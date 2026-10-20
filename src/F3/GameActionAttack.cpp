// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <math.h>
#include "F3/GameActionAttack.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/Scripting.hpp"

namespace F3 {
	RandFn GameActionAttack::s_rand = 0;   // NULL -> std rand inside CombatDice
	RandFn GameActionAttack::s_voiceRand = 0;

	const int kReactionAnimMoving[2] = { 33, 34 };        // victim moving: front/back
	const int kReactionAnimIdleFlag[2] = { 37, 38 };      // stationary, +0xAC clear
	const int kReactionAnimIdleFlagSet[2] = { 38, 39 };   // stationary, +0xAC set

	GameActionAttack::GameActionAttack()
	  : actor( 0 ), target( 0 ), calledShotPart( 9 ), reach( 1 ),
	    firstStrikeDueMs( 0 ), nextSubAttackDueMs( 0 ), subAttackPending( false ),
	    volleyContinuing( false ), ammoDeducted( false ), seriesRemaining( 0 ),
	    moveSpeed( 4.0f ), unarmed0x71( true ), queueCount( 0 ), finished( false ) {
		for ( int i = 0; i < kQueueCap; ++i ) {
			queue[i].dmg = 0; queue[i].damageType = 0; queue[i].material = 0;
			queue[i].lastTarget = false;
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
						qh.lastTarget = true;                   // single-target action here
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
				// 4. hit angle (victim-attacker delta -> atan2; shipped multiplies deltas
				// by flt_649C50 and compares two/three threshold floats flt_660820/824/828
				// for the behind-sector flag - constants not yet extracted; deviation:
				// planar atan2 + documented sector test at |angle| > 3*pi/4).
				const float hx = target->pos[0] - actor->pos[0];
				const float hz = target->pos[2] - actor->pos[2];
				const float angle = atan2f( hx, hz );
				const bool fromBehind = ( angle > 2.356f ) || ( angle < -2.356f );
				// 5. reaction anim pick (QueueAnimEvent_0x4673D0 is unported -> id is stored on
				// the victim as the seam; session 7 §3 step 5, victim-moving byte = Entity+0x3A2)
				target->queuedAnimEvent = target->movingFlag
				    ? kReactionAnimMoving[fromBehind ? 1 : 0]
				    : ( qh.lastTarget ? kReactionAnimIdleFlagSet : kReactionAnimIdleFlag )
			          [ fromBehind ? 1 : 0 ];
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
					die.flagsByte0x28 = qh.lastTarget ? 1 : 0;
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
