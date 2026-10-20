// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <math.h>
#include "JE.hpp"             // JE::LogDebugString - the shipped debug strings are part of the port
#include "F3/CombatMath.hpp"

namespace F3 {
	const int kCalledShotMods[8] = { 10, 40, 40, 40, 40, 60, 120, 60 };  // dword_64F858

	// 0x48CD00: v3=min, v2=max; equal -> param_1; return min + rand % (max-min+1).
	int CombatDice( int a, int b, RandFn rnd ) {
		if ( a == b ) {
			return a;
		}
		int lo = a, hi = b;
		if ( b < a ) { lo = b; hi = a; }
		return lo + ( rnd ? rnd() : (int)rand() ) % ( hi - lo + 1 );
	}

	// GameCreature::GetTotalSkill_0x409830 - base rank points + SPECIAL combos
	// (session 23 §6 dump; special[] = {STR,PER,END,CHA,INT,AGI,LCK}).
	int GetTotalSkill( const CombatStats& c, CombatSkill s ) {
		tracing::instrument( tracing::LOCATION, "skill=%d", (int)s );
		const int base = c.skills[s];
		const int STR = c.special[0], PER = c.special[1], END = c.special[2],
		          CHA = c.special[3], INT = c.special[4], AGI = c.special[5];
		// No shipped skill formula reads LUCK (0x409830's 13 cases don't touch +0x778) - it is
		// read only by GetDerived's CRITICAL_CHANCE, so it is deliberately not bound here.
		switch ( s ) {
		case kSkillMelee:
		case kSkillFirearms:
			return base + 50;
		case kSkillUnarmed:
			return END + 2 * STR + base;
		case kSkillBarter:
		case kSkillPersuasion:
			return 3 * CHA + base;
		case kSkillDeception:
			return INT + 2 * CHA + base;
		case kSkillMechanics:
		case kSkillMedic:
			return PER + 2 * INT + base;
		case kSkillOutdoorsman:
			return END + 2 * PER + base;
		case kSkillScience:
			return 3 * INT + base;
		case kSkillSneak:
		case kSkillSecurity:
			return AGI + 2 * PER + base;
		case kSkillSteal:
			return PER + 2 * AGI + base;
		default:
			return base;   // shipped: debug "UNKNOWN SKILL: %d" + rank passthrough
		}
	}

	// GameCreature::GetDerivedAttribute_0x409a60 (session 23 §6). HIT_POINTS = 6000/AP
	// as shipped (their §8.4: smells wrong-on-purpose; runtime verify pending - ported
	// faithfully, guard AP==0 -> 0 to avoid the shipped divide-by-zero).
	int GetDerived( const CombatStats& c, DerivedAttr a ) {
		const int STR = c.special[0], PER = c.special[1], END = c.special[2],
		          INT = c.special[4], AGI = c.special[5], LCK = c.special[6];
		const int ap = AGI + 20;
		switch ( a ) {
		case kDerivedActionPoints:      return ap;
		case kDerivedCarryWeight:       return 25 * ( STR + 1 );
		case kDerivedCombatSequence:    return 2 * PER;
		case kDerivedCriticalChance:    return LCK;
		case kDerivedEvasion:           return c.evasion;   // stored, not computed
		case kDerivedHealingRate:       return END;
		case kDerivedMeleeDamage:       return STR;
		case kDerivedPerceptionRange:   return 3 * PER + 40;
		case kDerivedPerkRate:          return 3;
		case kDerivedSkillRate:         return 2 * INT + 5;
		case kDerivedThrowingRange:     return 3 * STR;
		case kDerivedHitPoints:         return ap ? 6000 / ap : 0;
		default:                        return 0;   // RAD/FLASH/BIO: 0 this build
		}
	}

	// 0x413310 core, ported against a fresh raw re-read (chat C-33 Â§3 + session 29 Â§3 corrected
	// my earlier reading):
	//   if (part == 9) -> LogDebugString("...No body part was targeted.") + return 0   UNCONDITIONAL
	//   ammo block runs ONLY when mode+0x1C (primaryAmmoCost) != 0; max takes the ammo max bonus
	//     and min takes its bonus only while it stays BELOW that bonused max (0x4133eb) - the min
	//     is NOT clamped upward when the bonus would overshoot.
	//   raw  = Dice(min, max) + flatBonus(mode+0x24)
	//   row  = part switch (0,1,2,3,4,7 -> +0x7B0 | 5,6 -> +0x7C8 | default -> log + +0x7B0 and
	//          STILL computes, it does not return 0), indexed by mode+0x10 (damage type) INSIDE it
	//   if (dt > 0) { dt -= ammoPenetration; floor 0 }   (penetration is 0 without an ammo block)
	//   dmg  = raw - dt/2 - Dice(0, dt/2); return max(dmg, 0)
	int CalculateDamage( const AttackModeRecord& m, const CombatStats& target,
	                     const AmmoMods& ammo, int calledShotPart, RandFn rnd ) {
		tracing::instrument( tracing::LOCATION, "part=%d dt=%d", calledShotPart,
		                     m.damageType );
		// 0x41333d sits in the else-branch of the param_1 && param_3 && param_2 validity check
		// (our POJOs cannot be null, so that guard is structurally satisfied here). The part-9
		// rejection is UNCONDITIONAL in this twin; the conditional form (`part == 9 && !usable`)
		// belongs to the unarmed twin @0x409f90 - the two sites genuinely differ.
		if ( calledShotPart == 9 ) {
			JE::LogDebugString( NULL, "GameWeapon::CalculateDamage - No body part was targeted.\n" );
			return 0;
		}
		int lo = m.damageMin;
		int hi = m.damageMax;
		int pen = 0;
		if ( m.primaryAmmoCost != 0 ) {            // mode+0x1C; the shipped ammo lookup + its
			hi += ammo.damageMaxBonus;          // "Unable to find ammo" FatalError are unported
			if ( lo + ammo.damageMinBonus < hi ) {
				lo += ammo.damageMinBonus;      // 0x4133eb direction
			}
			pen = ammo.dtPenetration;            // ammo+0x3C0
		}
		int raw = CombatDice( lo, hi, rnd ) + m.flatDamageBonus;

		const int* row;
		switch ( calledShotPart ) {
		case 0: case 1: case 2: case 3: case 4: case 7:
			row = target.dt; break;
		case 5: case 6:
			row = target.dtParts56; break;
		default:
			JE::LogDebugString( NULL,
			    "GameWeapon::CalculateDamage() - Unknown CALLEDTARGET encountered.\n" );
			row = target.dt; break;
		}
		int dt = row[ m.damageType & 7 ];
		if ( dt > 0 ) {
			dt -= pen;
			if ( dt < 0 ) { dt = 0; }
		}
		int dmg = raw - dt / 2 - CombatDice( 0, dt / 2, rnd );
		return dmg > 0 ? dmg : 0;
	}

	// 0x409f90 GameCreature::CalculateUnarmedDamage - the twin the shipped attack path actually
	// calls for an unarmed actor (fork on the action's +0x71 bool at 0x4040a8/0x4040d4). Raw
	// differences from the weapon twin: no ammo block at all, MELEE_DAMAGE rides on damageMax
	// when the mode's damage type is 4 (hand-to-hand), part 9 zeros only for NON-usable targets,
	// and a GameUsableObject target contributes its own +0xA4 DT array instead of the creature
	// rows. DEVIATION: the 0x40a024-0x40a045 extra-object bonus branch is not modeled - its
	// addend decompiles as `MEMORY[0x24]` (base register lost) and it is provably inert when the
	// extra object is absent, which is the unarmed case. Routed to discovery as C-40.
	int CalculateUnarmedDamage( const AttackModeRecord& m, const CombatStats& attacker,
	                            const CombatStats& target, int calledShotPart,
	                            bool targetIsUsableObject, RandFn rnd ) {
		tracing::instrument( tracing::LOCATION, "part=%d dt=%d usable=%d", calledShotPart,
		                     m.damageType, (int)targetIsUsableObject );
		if ( calledShotPart == 9 && !targetIsUsableObject ) {
			return 0;                            // 0x409fec - silent, no log line at this site
		}
		int hi = m.damageMax;
		if ( m.damageType == 4 ) {
			hi += GetDerived( attacker, kDerivedMeleeDamage );   // 0x409ffd-0x40a00a
		}
		int raw = m.flatDamageBonus + CombatDice( m.damageMin, hi, rnd );

		int dt;
		if ( targetIsUsableObject ) {
			dt = target.dtUsable[ m.damageType & 7 ];            // 0x40a052: v8[4*type + 164]
		} else {
			const int* row;
			switch ( calledShotPart ) {
			case 0: case 1: case 2: case 3: case 4: case 7:
				row = target.dt; break;
			case 5: case 6:
				row = target.dtParts56; break;
			default:
				JE::LogDebugString( NULL,
				    "GameCreature::CalculateUnarmedDamage() - Unknown CALLEDTARGET encountered.\n" );
				row = target.dt; break;
			}
			dt = row[ m.damageType & 7 ];
		}
		int dmg = raw - dt / 2 - CombatDice( 0, dt / 2, rnd );
		return dmg > 0 ? dmg : 0;
	}

	static inline int ClampChance( int v ) {
		if ( v < 0 ) { return 0; }
		if ( v > 95 ) { return 95; }             // session 23: cap 95 (100-5 always-miss)
		return v;
	}

	// 0x413150 (session 23 §2). penalty = (200 / ((range-10)/5)) * max(0, int(0.2*d - 2)).
	int CalculateToHitRanged( const AttackModeRecord& m, const CombatStats& attacker,
	                          const CombatStats& target, const AmmoMods& ammo,
	                          float dist, int effectiveRange, int calledShotPart ) {
		tracing::instrument( tracing::LOCATION, "range=%d dist=%g", effectiveRange, dist );
		const CombatSkill sk = ( m.style == 2 ) ? kSkillMelee : kSkillFirearms;
		int base = m.toHitBonus + GetTotalSkill( attacker, sk );
		if ( m.primaryAmmoCost != 0 ) {
			base += ammo.toHitBonus;             // ammo accuracy mod (only when uses ammo)
		}
		int range = ( m.style == 2 )
		      ? GetDerived( attacker, kDerivedThrowingRange )
		      : effectiveRange;
		int penalty = 0;
		const int denom = ( range - 10 ) / 5;    // shipped int-division (range>10 assumed)
		if ( denom != 0 ) {
			int steps = (int)( 0.2f * dist - 2.0f );
			if ( steps < 0 ) { steps = 0; }
			penalty = ( 200 / denom ) * steps;
		}
		int armor = GetDerived( target, kDerivedEvasion );
		if ( calledShotPart != 9 && calledShotPart >= 0 && calledShotPart < 8 ) {
			armor += kCalledShotMods[ calledShotPart ];
		}
		return ClampChance( base - armor - penalty );
	}

	// 0x412fe0 (session 23 §3): MELEE skill always; out of reach -> 0; no distance
	// penalty; armor identical.
	int CalculateToHitMelee( const AttackModeRecord& m, const CombatStats& attacker,
	                         const CombatStats& target, const AmmoMods& ammo,
	                         float dist, int effectiveRange, int calledShotPart ) {
		int base = m.toHitBonus + GetTotalSkill( attacker, kSkillMelee );
		if ( m.primaryAmmoCost != 0 ) {
			base += ammo.toHitBonus;
		}
		if ( dist > (float)effectiveRange ) {
			return 0;                            // out of reach
		}
		int armor = GetDerived( target, kDerivedEvasion );
		if ( calledShotPart != 9 && calledShotPart >= 0 && calledShotPart < 8 ) {
			armor += kCalledShotMods[ calledShotPart ];
		}
		return ClampChance( base - armor );
	}
} // namespace F3
