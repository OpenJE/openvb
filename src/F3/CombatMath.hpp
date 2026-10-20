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
		int dtParts56[8];   // second row when the CALLED-SHOT PART is 5/6 (raw 0x413413/
	                    // 0x40a06b switch on the PART; doc §3's "mode" wording was loose)
		int dtUsable[8];    // GameUsableObject DT array at +0xA4, indexed by damage type -
	                    // read instead of the creature rows when the target is a usable
	                    // object (0x409f90 @0x40a052: v8[4*param_1[4] + 164])
		int evasion;
		float pos[3];

		CombatStats() : evasion(0) {
			for ( int i = 0; i < kSkillCount; ++i ) { skills[i] = 0; }
			for ( int i = 0; i < 7; ++i ) { special[i] = 0; }
			for ( int i = 0; i < 8; ++i ) { dt[i] = 0; dtParts56[i] = 0; dtUsable[i] = 0; }
			pos[0] = pos[1] = pos[2] = 0.0f;
		}
	};

	// Attack-mode record (MODE/GWAM record; byte offsets verified session 23 §5).
	// NOTE: `style` does NOT pick the DT row - the called-shot BODY PART does (raw switch on
	// param_5 at 0x413413 / 0x40a06b; chat C-33 + session 29). Kept here because style==2
	// (throwing) still selects the MELEE skill + 3*STR range in the to-hit twins.
	//
	// FIELD ORDER IS LOAD-BEARING: this is a C++98 aggregate and the suites brace-initialize it
	// POSITIONALLY (`{ 0, 0, 0, 2, 0, 1, 3, 7, 0 }`). Inserting a field in the middle silently
	// reassigns every later value (it cost 10 test failures to learn that) - append new fields
	// at the end instead. The offsets in the comments are documentation, not layout.
	struct AttackModeRecord {
	public:
		int   style;          // +0x08 (2 = throwing)
		int   damageType;     // +0x10 (also the DT index INSIDE the row; 4 = hand-to-hand)
		int   primaryAmmoCost;// +0x1C (param_1[7]: nonzero -> the whole ammo block runs)
		int   flatDamageBonus;// +0x24 (param_1[9])
		int   toHitBonus;     // +0x28
		int   attackCount;    // +0x30 (1 = single, no continuation; param_1[3] in §21)
		int   damageMin;      // +0x38 (param_1[14])
		int   damageMax;      // +0x3C (param_1[15])
		int   ammoExtra;      // +0x50
		int   ammoReqType;    // +0x18 (param_1[6]; gates the unarmed bonus-vs-extra-object branch,
		                      // which is not modeled - see CalculateUnarmedDamage)
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

	// GameWeapon::CalculateDamage_0x413310 core (session 21 §3 + raw decompile re-read).
	// calledShotPart == 9 is an UNCONDITIONAL zero here (@0x41333d, inside the else-branch of
	// the param_1 && param_3 && param_2 validity check) with the shipped log line - the
	// CONDITIONAL form (`param_4 == 9 && !usable`) is the UNARMED twin @0x409f90, a different
	// function. The two sites genuinely differ (session 29 §3, chat C-33 §3).
	int CalculateDamage( const AttackModeRecord& m, const CombatStats& target,
	                     const AmmoMods& ammo, int calledShotPart, RandFn rnd );

	// GameCreature::CalculateUnarmedDamage_0x409f90 - the UNARMED twin, which is what the
	// shipped attack path actually calls for an unarmed actor (fork on the action's +0x71
	// "unarmed" bool at 0x4040a8/0x4040d4). Differences from the weapon twin, all raw-verified:
	//   - no ammo block at all (min/max come straight from the mode record)
	//   - damageType == 4 (hand-to-hand) adds the ATTACKER's MELEE_DAMAGE derived to damageMax
	//   - part 9 zeros only when the target is NOT a GameUsableObject
	//   - usable-object targets read their own DT array (+0xA4) instead of the creature rows
	//   - the style/extra-object bonus branch (0x40a024-0x40a045) is skipped when the extra
	//     object is absent, which is the unarmed case; the shipped `MEMORY[0x24]` addend in that
	//     branch is a decompiler artifact and is NOT modeled (routed to discovery as C-40)
	int CalculateUnarmedDamage( const AttackModeRecord& m, const CombatStats& attacker,
	                            const CombatStats& target, int calledShotPart,
	                            bool targetIsUsableObject, RandFn rnd );

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
