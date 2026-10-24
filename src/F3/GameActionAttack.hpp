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
	//    ** THE "[0..max]" CLAIM ABOVE IS FALSE AS WRITTEN - corrected 2026-10-23 from bytes. **
	//    meth_0x4629C0 is: `mov edx,[esp+4]` (NEW value) ; `mov eax,[ecx+0x20]` (MAX) ;
	//    `cmp edx,eax` ; `mov [ecx+0x24],edx` (STORE FIRST, UNCONDITIONALLY) ; `jle return` ;
	//    `mov [ecx+0x24],eax`. So it computes `cur = min(new, max)` with NO LOWER BOUND, and the
	//    store precedes the test - a hit for more than remaining HP leaves NEGATIVE health in a
	//    shipped GameEntity. The port's former floor (`statCur = cur < 0 ? 0 : cur`) was a
	//    REAL behavior difference, not a rounding detail.
	//    STEP 3 RESOLVED (2026-10-24): the death test was re-derived FIRST (the Apply carries
	//    the `<= 0` state-predicate-shaped conversion, not an HP == 0 compare) and the hp=0-
	//    reaching digest fixture EXISTS (tools/scripts/hp0-death.txt -> artifacts/hp0-death.digest,
	//    exact-0 arc + over-kill leg). With that order complete the floor is RETIRED in
	//    GameActionAttack.cpp's HP update - the write is now min(new, max) only,
	//    shipped-faithful. Deviation retired; the bytes stay recorded here as the evidence.
	//  - hit test: chance >= RandRange(0,100) per RollToHit_0x402260 (session 17 §1) - note the
	//    inclusive 0..100 range (101 outcomes), NOT 1..100 (chat C-33 §1).
	//  - the shipped damage fork is the action's +0x71 "unarmed" bool (0x4040a8/0x4040d4):
	//    unarmed -> GameCreature::CalculateUnarmedDamage_0x409f90, otherwise the weapon twin
	//    0x413310. We carry the flag as `unarmed0x71` and default it TRUE, because the weapon /
	//    inventory registries are unported (so every attack here is unarmed and takes the twin
	//    the demo actually exercises).
	//  - hit-reaction selection (T-012). The shipped test is NOT a behind-arc threshold: the
	//    pre-2026-10-20 rule here (planar atan2 with an arc threshold of three pi over
	//    four) was a substituted guess
	//    and is DELETED - the constants it assumed do not exist (0x660820/824/828 are 360.0f,
	//    180.0f, -180.0f and 0x6CB56C is 0.0f; NO arc-threshold constant of any kind exists at
	//    those sites). Faithful shape = HitAngleWrapFlag (the x87 compare chain) + ReactionAnimFor
	//    (the selector at 0x45C30C..0x45C35C), with these remaining deviations, both of which
	//    are inputs rather than logic:
	//      * we feed the planar position delta; the delta-Euler source fields and the transform
	//        helpers 0x47272B / 0x471357 are unported.
	//      * the shipped wrap byte (Apply's var_B1) is never zeroed - its only stores are
	//        `mov byte,1` at 0x45C2D2/0x45C2DB, and the apparent init at 0x45C1B6 hits var_A5,
	//        a different slot (0x45C22C's `sub esp,0Ch` shifts the window by 0xC). So in the
	//        original an in-band angle reads uninitialised stack, i.e. the previous Apply's
	//        residue. We pass a fresh 0 per hit to stay deterministic; the stickiness is
	//        reproducible by passing the previous byte. Filed, not "fixed".
	//    Consequence worth knowing before reading the tables: the reachable reaction ids are
	//    {33,34,37,38,40,41}. 39 (shipped in the old tables) is unreachable and 41 was missing,
	//    because the stationary selector adds (wrapByte ? 3 : 0) to a base, not 0/1.
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

	// Hit-reaction anim ids. Restated from the selector at 0x45C326..0x45C355 (T-012): the
	// stationary paths do NOT use a two-element table, they add a step of 3 to a base, so the
	// shipped reachable set is {33,34} moving, {37,40} stationary/+0xAC clear, {38,41}
	// stationary/+0xAC set. 39 (in the old table) is UNREACHABLE; 41 was missing entirely.
	extern const int kReactionAnimMoving[2];        // {33,34} = 0x21 + 0/1
	extern const int kReactionAnimIdle[2];          // {37,40} = 0x25 + 0/3, +0xAC clear
	extern const int kReactionAnimIdleSet[2];       // {38,41} = 0x26 + 0/3, +0xAC set

	// The x87 compare chain of GameEffectDamage::Apply (0x45C29E..0x45C2E4). Returns the
	// wrap-hit byte; `unwritten` stands in for what the shipped code reads when NO branch wrote
	// the byte (see HitAngleWrapFlag's body comment - it is an uninitialised stack slot there).
	unsigned char HitAngleWrapFlag( float angle, unsigned char unwritten );

	// Reaction anim id for one queued hit, or 0 when no QueueAnimEvent happens at all (the
	// shipped selector queues NOTHING for a non-first record of a batch: 0x45C30C branches on
	// eff+0xAD straight past the call at 0x45C35A).
	//   moving            @0x45C326: 0x21 + (wrap != 0)      -> { 33, 34 }
	//   stationary, +0xAC=0 @0x45C355: 0x25 + (wrap != 0 ? 3 : 0) -> { 37, 40 }
	//   stationary, +0xAC=1 @0x45C349: 0x26 + (wrap != 0 ? 3 : 0) -> { 38, 41 }
	// The stationary step is 3, NOT 1: the selector is `neg al / sbb eax,eax / and eax,3`, which
	// is the standard integer-mask idiom for (al != 0) ? 3 : 0. So 39 - which the pre-T-012 tables
	// shipped - is NOT REACHABLE from this code, and 41 was missing entirely.
	int ReactionAnimFor( bool firstInBatch, bool targetMoving, bool flurryGe2,
	                     unsigned char wrapByte );

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
		// +0x80 is NOT a pending flag - it is the series vector's `finish` pointer (see the
		// +0x7C/+0x80/+0x84 triplet above). FastArrow's region map (called-shot answer §5)
		// shows the layout doc's "+0x80 next-attack pending flag" row was the same IDA display
		// artifact as "+0x78 series" and "+0x73+3". Modelled stand-in, no shipped offset.
		bool          volleyContinuing;
		bool          ammoDeducted;         // +0x58 per-volley guard
		// POSITION OVERRIDE (packet-91 place-attack surface, T-044/T-047): shipped
		// meth_0x401100 writes the hasPos byte at +0x48 and the vec3 at +0x4C/+0x50/+0x54.
		// With +0x48 set, OnServerUpdate (0x404410) SKIPS victim resolution and the attack
		// aims at these coordinates; the accept guard is attacker && (hasPos || target)
		// (0x46decb). The script-native door NEVER sets this group (T-037) — only the
		// packet-91 model path (driver verb attack-pos) does.
		bool          positionOverride;   // +0x48 (meth_0x401100 flag byte)
		float         overridePos[3];     // +0x4C/+0x50/+0x54
		// Shipped sub-attack series is NOT a scalar: it is an STL vector triplet at
		// +0x7C begin / +0x80 finish / +0x84 storage-end, remaining = (finish-begin)/elem,
		// ctor-initialised by sub_401690(ecx = &this+0x7C). A SECOND parallel vector at
		// +0x88/+0x8C/+0x90 carries per-sub-attack timing (hitDelay / the gap-0x65 machinery);
		// its element layout is deliberately unpinned (FastArrow, T-010c follow-up).
		// Elements are per-sub-attack RECORDS, and the selector writes the computed hit
		// location into record+0x20 (@0x404989) - that is where an uncalled-shot attack gets
		// its target id (CalculateDamage's param_5). NB 10c: the ids are EQUIP-SLOT coverage,
		// not anatomy - {5,6} are the head-family pair, which-of-the-two still inferred.
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
			// effect +0xAC. NOT "is-last-target" (that label was NiceWolf's session-7 inference).
			// FastArrow T-014: the finisher writes it as (count of records whose +0x19 snapshot is
			// set >= 2), and record+0x19 is itself a queue-time copy of the action's +0x71 UNARMED
			// bool (@0x4041A1). So this is "flurry of >= 2 unarmed records", which is why counting
			// records alone is wrong - a 3-hit WEAPON volley leaves it clear.
			bool flurryGe2;
			bool firstInBatch;   // effect +0xAD inverse: non-first records animate NOTHING
			bool unarmedSnapshot;  // record+0x19: queue-time copy of the action's +0x71 (T-014)
		};
		static const int kQueueCap = 16;
		QueuedHit queue[kQueueCap];
		int   queueCount;

		bool  finished;

		GameActionAttack();

		// arm the action (shipped ctor_0x403e40 + meth_0x403fa0(target) equivalents)
		void Start( SpawnedEntity* who, SpawnedEntity* against, unsigned nowMs,
		            const AttackModeRecord* m = 0 );

		// meth_0x401100 equivalent (packet-91 place-attack ONLY — see the +0x48 note above):
		// sets the override flag + coords; call BEFORE Start with a null victim.
		void SetPositionOverride( float x, float y, float z ) {
			positionOverride = true;
			this->overridePos[0] = x; this->overridePos[1] = y; this->overridePos[2] = z;
		}

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
