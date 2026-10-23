// Copyright 2024 OpenJE

#ifndef F3_GAMEWEAPON_HPP_
#define F3_GAMEWEAPON_HPP_

#include "F3.hpp"
#include "F3/GameItem.hpp"

namespace F3 {
	struct GameWeapon : public GameItem {
		//: Members
			dword mbr_0x3b0;
			dword mbr_0x3b4;
			dword mbr_0x3b8;
			dword mbr_0x3bc;
			dword mbr_0x3c0;
			dword mbr_0x3c4;
			dword mbr_0x3c8;
			byte mbr_0x3d0;
			dword mbr_0x3e0;
			dword mbr_0x3e4;
			dword mbr_0x3e8;
			dword mbr_0x3ec;
			dword mbr_0x3f0;
			dword mbr_0x3f4;
			dword mbr_0x3f8;
			dword mbr_0x3fc;
			dword mbr_0x4;
			dword mbr_0x400;
			dword mbr_0x404;
			dword mbr_0x408;
			dword mbr_0x40c;
			dword mbr_0x410;
			dword mbr_0x414;
			dword mbr_0x418;
			dword mbr_0x41c;
			dword mbr_0x420;
			dword mbr_0x424;
			dword mbr_0x428;
			dword mbr_0x430;
		//: Methods
			void ctor_0x414510();
			undefined4 virt_meth_0x414680();
			undefined4 virt_meth_0x414690();
			undefined4 virt_meth_0x4146a0();
			undefined4 virt_meth_0x4146b0();
			undefined4 virt_meth_0x4146c0();
			undefined4 virt_meth_0x4146d0();
			undefined4 virt_meth_0x4146e0();
			undefined4 virt_meth_0x4146f0();
			undefined4 virt_meth_0x414700();
			undefined4 virt_meth_0x414710();
			undefined4 virt_meth_0x414720();
			undefined4 virt_meth_0x414730();
			undefined4 virt_meth_0x414740();
			undefined4 virt_meth_0x414750();
			undefined4 virt_meth_0x414760();
			undefined4 virt_meth_0x414770();
			undefined4 virt_meth_0x414780();
			undefined4 virt_meth_0x414790();
			undefined4 virt_meth_0x4147a0();
			undefined4 virt_meth_0x4147b0();
			undefined4 virt_meth_0x4147c0();
			undefined4 virt_meth_0x415660();
			undefined4 virt_meth_0x416a60();

		//: Ported field meanings, each pinned to the instruction that reads it - the excerpt
		//: hex stays the name, the behavior lives in GameWeapon.cpp:
		//   mbr_0x3b4 = ammo caliber            (0x413ada ReloadPrimaryAmmo gate, cell 237 of the ammo)
		//   mbr_0x3b8 = primary modifier/id     (pushed to the server sync at 0x412b6d-0x412b70)
		//   mbr_0x3c4 = secondary id/modifier   (DeductSecondary sync payload, 0x412bfe-era read)
		//   mbr_0x3ec = primary ammo count      (`mov eax,[esi+3ECh]` 0x412b13; `sub [esi+3ECh],edi` 0x412b2f)
		//   mbr_0x3f0 = primary capacity        (cell 252 read at 0x413aae)
		//   mbr_0x3f4 = secondary ammo count    (DeductSecondary mirror)
		//   mbr_0x424 = primary in-magazine flag(cleared at 0x412b37 iff the SUB sets ZF)
		//   mbr_0x428 = secondary in-magazine flag(mirror, 0x412bb7-era)
	};

	// ---- batch C GameWeapon shard: server-sync seam ---------------------------------
	// Shipped's Deduct* tail resolves the weapon's server-side handle through the world-server
	// delta base (`mov ecx,[esi+4]; mov edx,[ecx+4]; mov ecx,[edx+esi+1E0h]` @0x412b6d-0x412b74,
	// then GameServer::meth_0x46d700 @0x46d700 / meth_0x46d7f0 for the pair) and the RTTI probe's
	// result is DISCARDED at the call (0x412b58 pushes it and never uses it). That plumbing belongs
	// to the net/server era; the port injects the observable instead: (id-field, new count).
	// NULL seam = skip the call: PORT-DEFENSIVE, not shipped - shipped always has the server
	// singleton post-Create. Named here so the difference is a decision, not a surprise.
	struct WeaponAmmoSync {
		void ( *pfnPrimary )( unsigned int nModifierId, unsigned int nCount );
		void ( *pfnSecondary )( unsigned int nId, unsigned int nCount );
	};

	// Install/replace the seam (product: the eventual server stub; suite: capture).
	void GameWeaponSetAmmoSync( const WeaponAmmoSync &sync );

	// 0x412b10 - DeductPrimaryAmmo(this, amount). Byte-faithful quirks, kept deliberately:
	// an over-deduct LOGS but still subtracts (0x412b20's jbe skips only the log), so an
	// unsigned wrap is shipped behavior; the in-mag flag at mbr_0x424 is cleared iff the SUB
	// result is exactly zero (ZF from `sub [esi+3ECh],edi` itself, 0x412b2f -> 0x412b35/0x412b37) -
	// which makes "deduct 0 from an empty weapon" clear the flag, a consequence of the encoding,
	// not of intent. The sync fires with the POST-decrement count regardless of path.
	void GameWeaponDeductPrimaryAmmo( GameWeapon &w, unsigned int nAmount );

	// 0x412b90 - DeductSecondaryAmmo(this, amount). The exact mirror: 0x3F4 count, 0x428 flag,
	// 0x3C4 id, meth_0x46d7f0 tail. Ported as its own body, NOT as the primary one with offsets
	// swapped in a loop, because shipped carries two separate functions and a shared-impl port
	// would hide any future divergence between them (mirrors have drifted in this binary before).
	void GameWeaponDeductSecondaryAmmo( GameWeapon &w, unsigned int nAmount );

	// Log-capture seam for tests (ScriptNatives.cpp's s_szLastError precedent): the formatted
	// message is copied AFTER the real JE::LogDebugString call, which remains the behavior.
	const char *GameWeaponGetLastLog();
	void        GameWeaponResetLastLog();

	// ---- shard status (verified against the live IDB before writing; see also the
	// ---- attribution finding on the second ReadAttackModeData row) --------------------
	// Ported here (this slice):      DeductPrimary 0x412b10, DeductSecondary 0x412b90.
	// Verified, awaiting seams:      ReloadPrimary 0x413a90, UnloadPrimary 0x4135c0 (inventory /
	//                                GameAmmo-spawn / critical-section / 3 server methods;
	//                                ReloadSecondary 0x413ca0 + UnloadSecondary 0x413810 NOT yet
	//                                byte-verified as mirrors - verify before claiming symmetry).
	// Verified, portable on existing primitives: ReadData 0x415190 (calls sub_412CD0 = GameItem
	//                                parent - VERIFIED a flat 15-read prefix reader over BYTES
	//                                0x3B0..0x404, [esi+3B0h] @0x412ce1; the decompiler renders
	//                                it char*-relative +944..+1028, which IS byte math - lands
	//                                inside this excerpt's named region; string at [3CCh] via
	//                                sub_49C8D0 = already ported StreamReadString_0x49c8d0;
	//                                ports as file-static) and ReadAttackModeData 0x4148a0
	//                                (GWAM chunk loop; JE::Common::Chunk +
	//                                MemStreamRead_0x498170 already ported).
	// 0x4148a0 VERIFICATION STATE: PAIR ANALYSIS COMPLETE - verdict: ONE RECORD FORMAT, NOT ONE
	// FUNCTION. Table comparison (jump tables read from the exe, base 0x400000): 27 of 28 legs are
	// byte-identical modulo the SEH 4-byte slot shift ([esp+54h] creature vs [esp+50h] weapon;
	// record lea esp+14 vs esp+10), including case 34 -> 0x68. THREE REAL DIVERGENCES:
	//  (a) GATE SOURCE: weapon keys extended reads on CHUNK version [ebx+0Ch] (cmpl 2/3/4/5 at
	//      0x414b7c/414b99/414bbc/414bd4); creature keys on the RECORD's item-version (v3[12] =
	//      byte 0x30, from GameCreature*'s embedded weapon record, base: false complete-object).
	//  (b) CASE-1 ANIM: weapon is CONDITIONAL - `mov al,[esp+76h]` (local_12) `neg/sbb/and 6/add
	//      38h` = 56 or 62 (bytes 8a 44 24 76 f6 d8 1b c0 83 e0 06 83 c0 38 @0x414c81); creature
	//      is CONSTANT 56 (c7 44 24 54 38 @0x40df36). local_12's WRITER IS STILL UNIDENTIFIED
	//      (init to 0 @0x4148e8; no store seen in dumped windows) - until it is found, do NOT
	//      conclude "dead condition"; a zero from a pattern search needs a known-present control
	//      (TrueRaven's two false zeros + the DB's own excerpt failure below prove the class).
	//  (c) WEAPON-ONLY TYPE DERIVATION @0x414c18-0x414c5c: consumed flag (local_3d) -> record
	//      type 2; else switch on [weapon + 0x3B0] (from GameWeapon*, complete object; DB member
	//      mbr_0x3b0) item type: 1-5 -> 1, 6-8 -> 0, 9-10 -> 3. The creature side has NO such
	//      block - its v8 stays at the init value 3.
	// APPEND TARGET RESOLVED (was port-gating): `mov ecx,[local_7c]; add ecx,40Ch; call
	//      sub_40C970` @0x414e07-0x414e11 - the weapon appends to ITS OWN list at WEAPON+0x40C
	//      (from GameWeapon*, complete object - excerpt member mbr_0x40c), record built at
	//      local_78 then destroyed post-append (sub_40A240 @0x414e1a). Creature side appends to
	//      owner+0x6F8 (from GameCreature*, base: false; DB mbr_0x6f8). Different coordinate
	//      systems, different owners - see the annotation law below.
	// LAYOUT LAW (TrueRaven, committed as layout law 20+): +0x6F4 (creature, base:false complete
	// object) and +0x168 (Entity, base:TRUE virtual-base subobject) are TWO COORDINATE SYSTEMS,
	// not one field in two views - 0x6F4-0x168=0x58C is valid arithmetic with NO semantics and
	// must never appear as a "conversion delta". Every offset in this shard carries object AND
	// base flag. The DB's usages list is itself an excerpt: mbr_0x6f0 lists 0x40d2ce but NOT
	// 0x40e441, where my own disasm read `mov eax,[esi+6F0h]` - absence in usages != no usage.
	// PORT DECISION: shared reader CORE ports as a file-static (format: the 9 base reads + 4/1/
	// 4/1/1/1 extension + string at >=4), the THREE divergent legs stay PER-SIDE - a merged
	// "one function" would have to invent which side owns (b) and (c), which is fabrication.
	// Tested twice: weapon triple (gate chunk+0xC, type derivation, append 0x40C) and creature
	// triple (gate byte 0x30, pinned type 3, append 0x6F8). GameCreature::ReadData WIRING stays
	// deliberately unowned (read-family seat's; ledger ruling).
	// ERA-DEFERRED with reason:      ResolveTargets 0x415740 - octree/Map/LOS cone search
	//                                (~1.7KB, mode-type switch 0..0x22, 'Unarmed attack type'
	//                                error rows); sole caller is GameActionAttack::meth_0x402e00
	//                                @0x402e95. LEDGER RULING 2dc7243: option (b) - the row moves
	//                                to FastNova with Step 2; (a)'s stub kept on record there.
	// ATTRIBUTION FINDING (name wrong, IDB): ReadAttackModeData_0x40dba0 is the CREATURE-side
	//                                GWAM reader - sole caller GameCreature::ReadData_0x40e360 at
	//                                0x40e481, ecx=creature (`mov esi` list owner), count field
	//                                [esi+6F0h] (0x40e441), 'GWAM' test @0x40e46f, appends at
	//                                owner+0x6F8 (sub_40C970(this+446 CELLS)); the weapon-side
	//                                twin is 0x4148a0. LEDGER RULING 2dc7243: row reassigned OUT
	//                                of this shard (fifth attribution failure this batch; ledger rule
	//                                now: an IDB method's CLASS is a claim, the sole caller's owner
	//                                is the evidence). Porting it under GameWeapon would have copied
	//                                the IDB's error into product code; it belongs to the GameCreature
	//                                read rows. Filed as ONE pattern with batch B's +0x6F4 neighbor:
	//                                446 cells = 0x6F8 bytes - the same neighborhood, one trap.
} // namespace F3

#endif // F3_GAMEWEAPON_HPP_
