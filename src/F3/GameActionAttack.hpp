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
	//  - hit test: chance >= RandRange(0,100) per RollToHit_0x402260 (session 17 §1) - note the
	//    inclusive 0..100 range (101 outcomes), NOT 1..100 (chat C-33 §1).
	//  - the shipped damage fork is the action's +0x71 "unarmed" bool (0x4040a8/0x4040d4):
	//    unarmed -> GameCreature::CalculateUnarmedDamage_0x409f90, otherwise the weapon twin
	//    0x413310. We carry the flag as `unarmed0x71` and default it TRUE, because the weapon /
	//    inventory registries are unported (so every attack here is unarmed and takes the twin
	//    the demo actually exercises).
	// GameActionDie (session 7 §3 step 10): created by the damage Apply when the victim
	// newly dies, queued onto the VICTIM's action pipeline (mode-5 "accepts queued
	// actions"). Fields per the shipped init: +0x1C damageType, +0x20 material, +0x28 byte.
	// DEVIATION: shipped death pipeline (anim/loot/fade) unported; this action performs
	// the lifecycle flip ACTIVE -> DYING, and the DYING -> DEAD edge runs in the entity pump
	// (OnDeath_0x5509b0, server-gated - session 12 §2), NOT here.
	// NB: src/F3/GameActionDie.hpp is the DB-generated stub for the same shipped class (its
	// mbr_0x1c / mbr_0x20 / mbr_0x28 are exactly these three fields). It is included nowhere;
	// if it ever is, delete the stub rather than renaming this port.
	struct GameActionDie {
	public:
		int damageType;   // +0x1C (from effect +0xA0)
		int material;     // +0x20 (effect +0xA4)
		unsigned char flagsByte0x28;  // +0x28 (effect +0xAC-derived)
		GameActionDie() : damageType(0), material(0), flagsByte0x28(0) {}
		int Update( struct SpawnedEntity* victim );   // 1 = done
	};

	// Hit-reaction anim ids (session 7 §3 step 5): moving -> 33 front / 34 back;
	// stationary -> (+0xAC set ? 38/39 : 37/38), front/back by the behind-sector flag.
	extern const int kReactionAnimMoving[2];    // {33,34}
	extern const int kReactionAnimIdleFlag[2];  // {37,38}
	extern const int kReactionAnimIdleFlagSet[2]; // {38,39}

	// .veg hit-effect selection (session 7 §3 step 6 table): damageType (+0xA0 0..5) with
	// material (+0xA4) sub-cases. Pure name resolver - VFX spawn layer consumes later.
	const char* SelectHitVeg( int damageType, int material, int dmg );

	struct GameActionAttack {
	public:
		// shipped offsets in comments; flat storage here
		SpawnedEntity*  actor;              // +0x14
		SpawnedEntity*  target;             // +0x34/+0x38
		AttackModeRecord mode;              // +0x30 record (MODE row; unarmed default)
		AmmoMods        ammo;               // weapon+0x3B4.. stand-in (§7 stats block)
		int             calledShotPart;     // +0x78 (9 = unset, shipped default). Was written
		                              // +0x73+3 here, which is an IDA int-index artifact: the
		                              // raw instruction at 0x4025db is 83 7E 78 09 =
		                              // `cmp dword [esi+0x78], 9` (T-010c, FastArrow; the
		                              // enclosing gate reads mode+0x41 at 0x4025d4).
		int           reach;                // +0x72 melee range (-1 = ranged)
		unsigned      firstStrikeDueMs;     // +0x5e (anim lead)
		unsigned      nextSubAttackDueMs;   // +0x65 dword (DM-beat release time)
		bool          subAttackPending;     // +0x60 b2
		bool          volleyContinuing;     // +0x80 next-attack pending
		bool          ammoDeducted;         // +0x58 per-volley guard
		// Shipped sub-attack series is NOT a scalar: it is an STL vector triplet at
		// +0x7C begin / +0x80 finish / +0x84 storage-end, remaining = (finish-begin)/elem,
		// ctor-initialised by sub_401690(ecx = &this+0x7C). A SECOND parallel vector at
		// +0x88/+0x8C/+0x90 carries per-sub-attack timing (hitDelay / the gap-0x65 machinery);
		// its element layout is deliberately unpinned (FastArrow, T-010c follow-up).
		// Elements are per-sub-attack RECORDS, and the selector writes the computed hit
		// location into record+0x20 (@0x404989) - that is where an uncalled-shot attack gets
		// its body part, feeding CalculateDamage's param_5.
		// DEVIATION: modelled as a plain countdown. It is a stand-in for the vector's
		// remaining count and has NO shipped offset - the earlier "+0x78" label here was
		// IDA display skew (its raw form is a vector cursor, not `mov [esi+0x78]`), and
		// +0x78 belongs solely to calledShotPart above.
		int           seriesRemaining;
		float         moveSpeed;            // step-toward fallback (deviation above)
		bool          unarmed0x71;          // +0x71 unarmed/weapon fork (see header notes)

		// damage queue (+0xbc): full queued-hit records (effect payload stand-ins:
		// dmg/DT/material/+0xAC marker per session 7 §3 drain-config idx1..3).
		struct QueuedHit {
			int dmg;
			int damageType;   // effect +0xA0
			int material;     // effect +0xA4
			bool lastTarget;  // effect +0xAC ("last target of action")
		};
		static const int kQueueCap = 16;
		QueuedHit queue[kQueueCap];
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
		// voice-roll coin flip seam (session 7 §3 step 7) - anim/sound ids pending audio
		// layer; exposed for determinism in tests.
		static RandFn s_voiceRand;
	};
} // namespace F3

#endif // F3_GAMEACTIONATTACK_HPP_
