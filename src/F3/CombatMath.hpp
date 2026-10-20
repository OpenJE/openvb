// Copyright 2024 OpenJE

#ifndef F3_COMBATMATH_HPP_
#define F3_COMBATMATH_HPP_

#include <stdlib.h>

namespace F3 {
	// Combat math cores ported from discoveries session 21/23 (attack state machine +
	// to-hit formulas complete). Pure functions over POJO records so they port and test
	// independently of the full GameCreature/GIWP/MODE layers.
	//
	// Rand seam: original dice use sub_48CD00(a,b) = min + sub_48CCC0() % (max-min+1)
	// (verified decompile; args auto-swap, equal -> constant). The shipped generator
	// sub_48CCC0 is a global LCG - this port takes an injectable fn for determinism.
	typedef int (*RandFn)();
	int CombatDice( int a, int b, RandFn rnd );   // 0x48CD00

	// F3 skill ids used by combat math (values are this port's keys, NOT shipped enum
	// numbers; the shipped enum order is still open - see C-26 question).
	enum CombatSkill {
		kSkillMelee, kSkillFirearms, kSkillUnarmed, kSkillBarter, kSkillPersuasion,
		kSkillDeception, kSkillMechanics, kSkillMedic, kSkillOutdoorsman, kSkillScience,
		kSkillSneak, kSkillSecurity, kSkillSteal, kSkillCount
	};

	// F3 derived attributes (session 23 §6 table).
	enum DerivedAttr {
		kDerivedActionPoints, kDerivedCarryWeight, kDerivedCombatSequence,
		kDerivedCriticalChance, kDerivedEvasion, kDerivedHealingRate, kDerivedMeleeDamage,
		kDerivedPerceptionRange, kDerivedPerkRate, kDerivedSkillRate, kDerivedThrowingRange,
		kDerivedHitPoints
	};

	// Minimal creature stat block: SPECIAL {STR,PER,END,CHA,INT,AGI,LCK} (shipped
	// GameCreature +0x760..+0x778 per session 23 §6), evasion storage (+0x7FC, STORED
	// value - not computed), two 8-dword DT rows (+0x7B0 generic, +0x7C8 used for attack
	// modes 5/6), and skill rank points.
	struct CombatStats {
	public:
		int skills[kSkillCount];
		int special[7];
		int dt[8];
		int dtParts56[8];   // second row when CALLED-SHOT PART is 5/6 (per 0x409f90
	                    // decompile switch; doc §3 "mode" wording was loose)
		int evasion;
		float pos[3];

		CombatStats() : evasion(0) {
			for ( int i = 0; i < kSkillCount; ++i ) { skills[i] = 0; }
			for ( int i = 0; i < 7; ++i ) { special[i] = 0; }
			for ( int i = 0; i < 8; ++i ) { dt[i] = 0; dtParts56[i] = 0; }
			pos[0] = pos[1] = pos[2] = 0.0f;
		}
	};

	// Attack-mode record (MODE/GWAM record; byte offsets verified session 23 §5).
	struct AttackModeRecord {
	public:
		int   style;          // +0x08 (2 = throwing; 5/6 select the second DT row)
		int   damageType;     // +0x10 (0..7 DT column)
		int   primaryAmmoCost;// +0x1C (nonzero -> ammo to-hit mod applies)
		int   flatDamageBonus;// +0x24
		int   toHitBonus;     // +0x28
		int   attackCount;    // +0x30 (1 = single, no continuation)
		int   damageMin;      // +0x38
		int   damageMax;      // +0x3C
		int   ammoExtra;      // +0x50
	};

	// Ammo prototype modifiers (session 23 §7: GameAmmo +0x3B8/+0x3BC/+0x3C0/+0x3C4).
	struct AmmoMods {
	public:
		int damageMinBonus;
		int damageMaxBonus;
		int dtPenetration;
		int toHitBonus;
		AmmoMods() : damageMinBonus(0), damageMaxBonus(0), dtPenetration(0), toHitBonus(0) {}
	};

	// GameCreature::GetTotalSkill_0x409830 (session 23 §6 table).
	int GetTotalSkill( const CombatStats& c, CombatSkill s );
	// GameCreature::GetDerivedAttribute_0x409a60 (session 23 §6; HIT_POINTS = 6000/AP -
	// shipped oddity, runtime verification pending per their §8.4).
	int GetDerived( const CombatStats& c, DerivedAttr a );

	// GameWeapon::CalculateDamage_0x413310 core (session 21 §3). calledShotPart 9 =
	// "no body part" -> 0 ("No body part was targeted."). Returns >= 0.
	int CalculateDamage( const AttackModeRecord& m, const CombatStats& target,
	                     const AmmoMods& ammo, int calledShotPart, RandFn rnd );

	// GameWeapon::CalculateToHit_0x413560 dispatcher -> ranged 0x413150 / melee 0x412fe0
	// (session 23 §2/§3). effectiveRange: weapon.GetEffectiveRange_0x412f50 (or throwing
	// derived for style==2). Returns clamp(base - armor - penalty, 0, 95).
	int CalculateToHitRanged( const AttackModeRecord& m, const CombatStats& attacker,
	                          const CombatStats& target, const AmmoMods& ammo,
	                          float dist, int effectiveRange, int calledShotPart );
	int CalculateToHitMelee(  const AttackModeRecord& m, const CombatStats& attacker,
	                          const CombatStats& target, const AmmoMods& ammo,
	                          float dist, int effectiveRange, int calledShotPart );

	// Called-shot to-hit difficulty table, dword_0x64F858 (session 23 §4; idx semantics
	// still open, sentinel idx8 dropped).
	extern const int kCalledShotMods[8];

} // namespace F3

#endif // F3_COMBATMATH_HPP_
