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
	// (verified decompile; args auto-swap, equal -> constant).
	typedef int (*RandFn)();
	int CombatDice( int a, int b, RandFn rnd );   // 0x48CD00

	// ---- the shipped global draw, sub_48CCC0 (0x48CCC0..0x48CCF6) -----------------
	// It is NOT rand(). Disassembly of the shipped binary:
	//   48ccc0  mov al, byte_707C64 / test / jnz      -> one-time "already seeded" flag
	//   48cccb  call __time32(0) ; call F3::SetStartupTime   (0x48CCA0: state = t, flag = 1)
	//   48ccde  imul eax, 19660Dh                    -> * 1664525
	//   48cce4  add  eax, 3C6EF35Fh                  -> + 1013904223
	//   48cce9  mov  F3__start_time, eax             -> state IS the startup-time dword
	//   48ccee  shr  eax, 8 / and eax, 7FFFh         -> returns (state >> 8) & 0x7FFF
	// So: one global dword, lazily seeded from the wall clock in SECONDS on the first draw,
	// Knuth/MMIX LCG advance, 15-bit output (0..32767). Every dice site in the game shares
	// this single stream, so a draw sequence is a pure function of that one seed.
	// Our port used to fall back to std rand() when no RandFn was injected - the same output
	// range but a different stream, i.e. a silent fidelity deviation. It now falls back to
	// this, so the un-injected app path draws what the original drew.
	int ShippedRand();

	// Test-only: force the LCG state, mirroring what F3::SetStartupTime (0x48CCA0) stores.
	// Registered as a deliberate deviation in docs/DEFERRED-NOTES.md (item D-1, section 2): the shipped
	// code has no way to reseed, and no per-call generator - the injection points below are
	// ours. Passing NULL to a CombatMath entry point uses the global stream above, which is
	// the shipped behaviour; seeding is only ever done by tests.
	void SeedShippedRand( unsigned int seed );

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
	// value - not computed), two 8-dword DT rows (+0x7B0 generic row A, +0x7C8 the second
	// row the target ids {5,6} select), and skill rank points.
	struct CombatStats {
	public:
		int skills[kSkillCount];
		int special[7];
		int dt[8];
		int dtRowB[8];      // the second DT row, selected by target ids {5,6} (raw switch on
	                    // param_5 at 0x413413 / 0x40a06b). NAMED BY WHAT IT IS: the shipped
	                    // model is TWO rows, not six per-part rows (docs/DEFERRED-NOTES.md
	                    // ledger entry 2, harmless-by-data). Formerly dtParts56 - the old name
	                    // asserted anatomy, which 10c disproved: {5,6} are EQUIP-SLOT coverage
	                    // (head-family), not body parts. Which of 5/6 is HEAD and which is EYES
	                    // remains INFERRED (penalty 120 at index 6 favours EYES); ids 0-4 and
	                    // 7-8 stay numeric and the code indexes numerically.
	                    // Evidence, cited by filename per harness item 15:
	                    //   discoveries/2025-10-20-session35-helmet-slot-data-closes-10c.md
	                    //   discoveries/2025-10-20-t010c-conflict-resolved-equip-slot-model.md
	                    //   (pair table: discoveries/2025-10-20-t010c-equip-slot-array-naming.md)
		int dtUsable[8];    // GameUsableObject DT array at +0xA4, indexed by damage type -
	                    // read instead of the creature rows when the target is a usable
	                    // object (0x409f90 @0x40a052: v8[4*param_1[4] + 164])
		int evasion;
		float pos[3];

		CombatStats() : evasion(0) {
			for ( int i = 0; i < kSkillCount; ++i ) { skills[i] = 0; }
			for ( int i = 0; i < 7; ++i ) { special[i] = 0; }
			for ( int i = 0; i < 8; ++i ) { dt[i] = 0; dtRowB[i] = 0; dtUsable[i] = 0; }
			pos[0] = pos[1] = pos[2] = 0.0f;
		}
	};

	// Attack-mode record (MODE/GWAM record; byte offsets verified session 23 §5).
	// NOTE: `style` does NOT pick the DT row - the called-shot TARGET ID does (raw switch on
	// param_5 at 0x413413 / 0x40a06b; chat C-33 + session 29; the ids are equip-slot
	// coverage per 10c, NOT anatomy). Kept here because style==2
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
		int   ammoReqType;    // +0x18 (param_1[6]; 1/3 in the unmodeled faulting branch - the
		                      // limb/punch-vs-kick reading is an UNPROVEN hypothesis, section 12)
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
	//   - the style/extra-object bonus branch (0x40a024-0x40a045) is NOT modeled, permanently:
	//     its `MEMORY[0x24]` addend is a LITERAL null-page read (raw `add esi, ds:0x24`) that
	//     faults on execution - a shipped defect unreachable by demo data. See canonical
	//     fact-sheet section 11 before "fixing" it. Do not assume it is skipped merely because
	//     the weapon pointer is absent: param_5 = action+0x2C can be non-null when unarmed.
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
