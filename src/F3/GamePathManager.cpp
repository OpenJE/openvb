// Copyright 2024 OpenJE

#include "tracing.hpp"
#include <string.h>
#include "F3/GamePathManager.hpp"
#include "F3/MapEntitySpawn.hpp"
#include "JE/SpatialGrid.hpp"
#include <string.h>

namespace F3 {

	// 0x45ba40
	// Original: JE::PathManager::ctor_0x560b80(this), then vftptr = &GamePathManager_vftable_0x6602e4.
	// Both are implicit in C++ (base member init + compiler-emitted vftptr store).
	GamePathManager::GamePathManager() {
		tracing::instrument( tracing::LOCATION, "" );
	} // GamePathManager

	// 0x45ba60 (completing destructor, 0xb bytes: runs base dtor + restores vftptr).
	// The scalar deleting destructor at 0x45baf0 (dtor + conditional operator delete) is
	// compiler-generated here as vft[4]; the original frees via j__free (not operator delete),
	// which the lockstep comparison should keep in mind.
	GamePathManager::~GamePathManager() {
		tracing::instrument( tracing::LOCATION, "" );
	} // ~GamePathManager

	// 0x45ba70 slot [0]: pure tail-call to JE_PathManager_Distance_0x5604a0.
	int GamePathManager::Distance(int x1, int y1, int x2, int y2) {
		tracing::instrument( tracing::LOCATION, "" );

		return JE::PathManager::Distance(x1, y1, x2, y2);
	} // Distance

	// 0x45bb10 slot [1]: DistanceDispatch override (semantic name: PathCostCheck).
	// Reconstructed structure:
	//   1. base JE_PathManager_DistanceDispatch_0x5604d0 over (pointA, pointB) -> vft[0] distance.
	//   2. if the node context (param_3) carries a grid at +80 (v4[20]): build a result container
	//      (0x14-byte RB-tree nodes via JE_RbTree_NodeAlloc14_0x56e2b0) and enumerate entities in
	//      the rectangle with JE_SpatialGrid_QueryEntities_0x405f80(this grid, query from
	//      map+364+12, mover cell from param_3+16/20 and pointA x/y).
	//   3. per entity, switch on entity kind at +0x21C:
	//        1, 2, 4 -> blocked iff map flag +820 && JE_EntitySet_Contains_0x54aeb0(map, entity)
	//        3       -> GameDoor: blocked iff !F3_GameDoor_IsOpen_0x40e5a0(door, entity)
	//      blocked entities apply a penalty write into the candidate node (param_3 + 32765 dwords).
	// UNRESOLVED: grid query + penalty store depend on JE::Map / grid reconstruction; until then
	// only the base distance pass is reproduced (behavior-equivalent to the original's early
	// exit when the grid pointer is NULL).
	int GamePathManager::DistanceDispatch(dword* pointA, dword* pointB, int param_3) {
		tracing::instrument( tracing::LOCATION, "" );

		int result = JE::PathManager::DistanceDispatch(pointA, pointB, param_3);

		// 0x45bb3f: v6 = *(dword*)(param_3 + 80); entity scan gated on v6 != NULL.
		dword grid = *reinterpret_cast<dword*>(reinterpret_cast<byte*>(param_3) + 80);
		if ( grid != 0 ) {
			// UNRESOLVED: spatial query + blocking rules (0x45bb52..0x45bc5d).
		}
		return result;
	} // DistanceDispatch

	// 0x45bc90 slot [2]: TraceSegmentClear override (semantic name: CanTravel).
	// Reconstructed structure (see docs/f3/GamePathManager.md):
	//   deltas: dA = param_7 - param_4, dB = param_6 - param_3, dC = (float)param_5 - param_2;
	//   dist  = IntSqrt(dC*dC + dA*dA); steps = dist * 1.666f (5/3 pace factor);
	//   if steps > 1: sample p = (param_2, param_3, param_4) + delta * (t/steps), resolve the
	//   active grid via the world singleton (F3::GetWorldServerInstance_0x401150, vft[+84] with
	//   mover+492), world->cell with JE_WorldToOctreeCell_0x560680, enumerate entities around
	//   each sample (JE_SpatialGrid_QueryEntities_0x405f80 via grid source mover+80) and apply
	//   the same kind rules as PathCostCheck (kinds 1/2/4 net-set block, kind 3 door-open flag at
	//   door[188]); any block -> return 0.
	//   Otherwise fall through to the base TraceSegmentClear (vft[2] -> vft[3] gates).
	// UNRESOLVED: the sampling loop shares the JE::Map/grid dependencies; until reconstructed the
	// fall-through alone is emitted, which yields the original's behavior for steps <= 1 (clear)
	// and the base's conservative blocked answer otherwise.
	int GamePathManager::TraceSegmentClear(int param_1, float param_2, float param_3, float param_4,
		int param_5, float param_6, float param_7) {
		tracing::instrument( tracing::LOCATION, "" );

		// goal-x arrives as the FLOAT BIT PATTERN in an int slot (0x45BCB0 register juggling)
		float gx;
		memcpy( &gx, &param_5, 4 );

		const float dx = gx - param_2;
		const float dz = param_7 - param_4;
		const int dist = JE::IntSqrt( (int)( dx * dx + dz * dz ) );
		const float steps = (float)dist * 1.666f;   // 5/3 pace factor
		if ( steps <= 1.0f ) {
			return 1;   // 0x45BE0B: trivially clear
		}

		JE::Map* map = GetActiveMap();
		if ( !map || !map->Grid() ) {
			// no grid context => base conservative answer (original: unmapped editor mode)
			return JE::PathManager::TraceSegmentClear(param_1, param_2, param_3, param_4,
			                                          param_5, param_6, param_7);
		}

		// Sample t = 1..steps-1 (ENDPOINTS DELIBERATELY EXCLUDED - the mover's and target's
		// own cells never block; matches 0x45BC4E..0x45BD60 where occupant rules apply to
		// intermediate samples). Kind rules from the original (creatures/usable/container
		// block when net-set membership; closed doors block) currently collapse to
		// ANY-intermediate-occupant blocks - door state modeling pending (deviation noted).
		void* hits[8];
		for ( int t = 1; t < (int)steps; ++t ) {
			const float f = (float)t / steps;
			const float px = param_2 + dx * f;
			const float pz = param_4 + dz * f;
			float p[3] = { px, 0.0f, pz };
			int cr[2];
			JE::WorldToOctreeCell_0x560680( cr, p, map );
			JE::SpatialGrid::Run run = { cr[0], 1 };   // absolute column run (footprint model)
			if ( map->Grid()->QueryEntities( &run, 1, cr[0], cr[1], hits, 8 ) > 0 ) {
				return 0;   // occupant at an intermediate cell
			}
		}
		return 1;
	} // TraceSegmentClear

	// 0x45ba90 slot [3]: EntityGate (per-mover-type clearance).
	// Original: GameEntity* mover = __RTDynamicCast(entity, Entity -> GameEntity);
	//           type = mover's dword at byte offset 0x18 (the decompiler renders the void* cast as
	//           `+6`, i.e. dword index 6 = 0x18 - CONFIRMED: F3::GameEntity::ctor_0x4678a0 initializes
	//           mbr_0x18 = 4, F3::GameCreature::ctor_0x40d010 sets it to 0, F3::GameDoor::ctor_0x40e7f0
	//           writes 3 to the equivalent slot; van-buren-docs entity chunk ftypes are 1000-based so
	//           this is a runtime class enum, not the .grp ftype).
	//   type <= 3 -> sizeByte >= 0x2B (43)
	//   type == 4 -> sizeByte >= 0x20 (32)
	//   type == 5 -> sizeByte >= 0x15 (21)
	//   type >= 6 -> sizeByte >= 0xA  (10)
	// The RTTI check would null a non-GameEntity and fault on the load; the unchecked read is
	// faithful to that.
	// Shipped gate: slot[3] of the 0x45BA90 mover-gate vft table (file note above); body mirrors
	// its per-mover-type clearance reads exactly (class enum at entity+0x18, RTTI-checked load).
	bool GamePathManager::EntityGate(byte sizeByte, void* entity) {
		tracing::instrument( tracing::LOCATION, "" );

		dword type = *reinterpret_cast<dword*>(reinterpret_cast<byte*>(entity) + 0x18);
		if ( type <= 3 && sizeByte >= 0x2B ) {
			return true;
		}
		if ( type == 4 ) {
			return sizeByte >= 0x20;
		}
		if ( type == 5 ) {
			return sizeByte >= 0x15;
		}
		return type >= 6 && sizeByte >= 0xA;
	} // EntityGate

} // namespace F3
