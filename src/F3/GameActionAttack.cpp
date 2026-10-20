// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <math.h>
#include "F3/GameActionAttack.hpp"
#include "F3/MapEntitySpawn.hpp"

namespace F3 {
	RandFn GameActionAttack::s_rand = 0;   // NULL -> std rand inside CombatDice

	GameActionAttack::GameActionAttack()
	  : actor( 0 ), target( 0 ), calledShotPart( 9 ), reach( 1 ),
	    firstStrikeDueMs( 0 ), nextSubAttackDueMs( 0 ), subAttackPending( false ),
	    volleyContinuing( false ), ammoDeducted( false ), seriesRemaining( 0 ),
	    moveSpeed( 4.0f ), queueCount( 0 ), finished( false ) {
		for ( int i = 0; i < kQueueCap; ++i ) { queue[i] = 0; }
		mode.style = 0; mode.damageType = 4; mode.primaryAmmoCost = 0;
		mode.flatDamageBonus = 0; mode.toHitBonus = 0; mode.attackCount = 1;
		mode.damageMin = 1; mode.damageMax = 3; mode.ammoExtra = 0;
	}

	AttackModeRecord GameActionAttack::MakeUnarmedMode( int dmgMin, int dmgMax,
	                                                    int subAttacks, int toHitBonus ) {
		AttackModeRecord m;
		m.style = 0; m.damageType = 4; m.primaryAmmoCost = 0; m.flatDamageBonus = 0;
		m.toHitBonus = toHitBonus; m.attackCount = subAttacks;
		m.damageMin = dmgMin; m.damageMax = dmgMax; m.ammoExtra = 0;
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
			// to-hit + damage rolls per sub-attack
			while ( seriesRemaining > 0 ) {
				const int chance = CalculateToHitMelee( mode, actor->combat,
				                                        target->combat, ammo, dist,
				                                        reach, calledShotPart );
				const int roll = CombatDice( 1, 100, s_rand );
				if ( roll <= chance ) {
					const int dmg = CalculateDamage( mode, target->combat, ammo,
					                                 calledShotPart, s_rand );
					if ( queueCount < kQueueCap ) {   // "Damage for %d queued."
						queue[ queueCount++ ] = dmg;
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
			for ( int i = 0; i < queueCount; ++i ) {
				int cur = target->statCur - queue[i];
				if ( cur > target->statMax ) { cur = target->statMax; }   // 0x4629C0 clamp
				target->statCur = cur < 0 ? 0 : cur;
				if ( target->statCur == 0 && target->entityState == kStateActive ) {
					target->entityState = kStateDying;   // OnDeath flip; server-only 6->7
					target->entityState = kStateDead;    // (this slice is the server)
				}
			}
			queueCount = 0;
			finished = true;
			return 1;
		}
		return 0;
	}
} // namespace F3
