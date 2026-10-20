// Copyright 2024 OpenJE

#ifndef F3_GAMEACTIONATTACK_HPP_
#define F3_GAMEACTIONATTACK_HPP_

#include "F3/CombatMath.hpp"

namespace F3 {
	struct SpawnedEntity;

	// F3::GameActionAttack - port of GameActionAttack (0xC8-byte action, shipped
	// OnServerUpdate_0x404320). Field/offset map from discoveries
	// 2025-10-20-attack-state-machine-damage-formula.md §1; flow §2. Damage/to-hit math
	// lives in CombatMath (this file's Update is the scheduling state machine only).
	//
	// DEVIATIONS (each commented at its site, none affects the shipped-math fidelity):
	//  - weapon/inventory/ammo registries unported: actor attacks UNARMED; AmmoMods is a
	//    plain field (default zeros). DeductPrimary/SecondaryAmmo skipped.
	//  - QueueAnimEvent(7=ToCombat) duration seam (mbr_0x5e first-strike delay) and the
	//    "DM" anim beat (mbr_0x65 release) are replaced by explicit ms timers on the same
	//    game clock; fields keep the original semantics (due-times), source differs.
	//  - auto-move-to-range: original issues a GameActionMoveTo(0x48)+AddAction and
	//    returns; our FIFO slot model has no mid-attack handback yet, so the actor steps
	//    toward the target directly at combat.speed (documented, same convergence goal).
	//  - damage queue drain (finisher meth_0x403770) applies HP directly instead of
	//    creating+attaching GameEffectDamage objects (effect/updater layer unported).
	//    HP write follows GameEntity::meth_0x4629C0 semantics: cur in [0..max] at
	//    +0x24/+0x20 == our statCur/statMax; death -> kStateDying then kStateDead.
	//  - hit test: d100 roll <= chance (CombatDice(1,100) through the rand seam).
	struct GameActionAttack {
	public:
		// shipped offsets in comments; flat storage here
		SpawnedEntity*  actor;              // +0x14
		SpawnedEntity*  target;             // +0x34/+0x38
		AttackModeRecord mode;              // +0x30 record (MODE row; unarmed default)
		AmmoMods        ammo;               // weapon+0x3B4.. stand-in (§7 stats block)
		int             calledShotPart;     // +0x73+3 (9 = unset, shipped default)
		int           reach;                // +0x72 melee range (-1 = ranged)
		unsigned      firstStrikeDueMs;     // +0x5e (anim lead)
		unsigned      nextSubAttackDueMs;   // +0x65 dword (DM-beat release time)
		bool          subAttackPending;     // +0x60 b2
		bool          volleyContinuing;     // +0x80 next-attack pending
		bool          ammoDeducted;         // +0x58 per-volley guard
		int           seriesRemaining;      // +0x78 series list remainder (count modelled)
		float         moveSpeed;            // step-toward fallback (deviation above)

		// damage queue (+0xbc): {dmg} per hit; caster/weapon netId keys omitted (single
		// actor/target pair per action object).
		static const int kQueueCap = 16;
		int   queue[kQueueCap];
		int   queueCount;

		bool  finished;

		GameActionAttack();

		// arm the action (shipped ctor_0x403e40 + meth_0x403fa0(target) equivalents)
		void Start( SpawnedEntity* who, SpawnedEntity* against, unsigned nowMs,
		            const AttackModeRecord* m = 0 );

		// OnServerUpdate_0x404320 core; gameMs = world frame ms (same clock as pumps).
		// Returns 1 when the action is finished (pump drops the slot).
		int  Update( unsigned gameMs );

		// unarmed attack-mode record twin (damageType 4 = hand-to-hand -> +MELEE_DAMAGE
		// rolled into max per CalculateUnarmedDamage_0x409f90)
		static AttackModeRecord MakeUnarmedMode( int dmgMin, int dmgMax, int subAttacks,
		                                         int toHitBonus );

		// rand seam for deterministic tests (NULL = std rand)
		static RandFn s_rand;
	};
} // namespace F3

#endif // F3_GAMEACTIONATTACK_HPP_
