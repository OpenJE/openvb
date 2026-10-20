// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <math.h>
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
		          CHA = c.special[3], INT = c.special[4], AGI = c.special[5],
		          LCK = c.special[6];
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

	// 0x413310 core (session 21 §3):
	//   raw = Dice(min + ammoMin, max + ammoMax) + flatBonus        [min<=max clamp]
	//   dt  = (calledShotPart 5/6 ? second row : generic row)[damageType] - row select by
	//   PART per the 0x409f90 decompile (doc §3's "mode==5||6" was the loose reading)
	//   if dt>0 { dt -= pen; floor 0 }
	//   dmg = raw - dt/2 - Dice(0, dt/2);  return max(dmg, 0)
	int CalculateDamage( const AttackModeRecord& m, const CombatStats& target,
	                     const AmmoMods& ammo, int calledShotPart, RandFn rnd ) {
		tracing::instrument( tracing::LOCATION, "part=%d dt=%d", calledShotPart,
		                     m.damageType );
		// part 9 guard (session 21 §3) is a game-layer preflight cancel in the original
		// ("No body part was targeted." + action cancel @0x404983 region), and 0x409f90
		// skips it for usable-object targets - so the FORMULA does not zero on 9; the
		// armed action guarantees a concrete part. (Reimpl: GameActionAttack::Start
		// resolves 9 -> 0.)
		int lo = m.damageMin + ammo.damageMinBonus;
		int hi = m.damageMax + ammo.damageMaxBonus;
		if ( lo > hi ) { hi = lo; }              // shipped "clamp min <= max" direction
		int raw = CombatDice( lo, hi, rnd ) + m.flatDamageBonus;

		const int* row = ( calledShotPart == 5 || calledShotPart == 6 )
		                 ? target.dtParts56 : target.dt;
		int dt = row[ m.damageType & 7 ];
		if ( dt > 0 ) {
			dt -= ammo.dtPenetration;
			if ( dt < 0 ) { dt = 0; }
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
