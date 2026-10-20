// Copyright 2024 OpenJE

#ifndef F3_GAMEPATHMANAGER_HPP_
#define F3_GAMEPATHMANAGER_HPP_

#include "F3.hpp"
#include "JE/PathManager.hpp"

namespace F3 {
	// Game-side path policy (RTTI: .?AVGamePathManager@@; sizeof == 0x10, adds no data over
	// JE::PathManager). Created by F3_GameWorld_CreatePathManager_0x46f9e0 (GameWorld vft[7]).
	// Derived vftable 0x6602e4 (5 slots):
	//   [0] 0x45ba70 Distance             - tail-calls the base 2D distance
	//   [1] 0x45bb10 DistanceDispatch (semantic: PathCostCheck) - base DistanceDispatch + entity
	//                                       scan along the segment
	//                                       with kind rules {1,2,4: map net set blocks; 3: doors}
	//   [2] 0x45bc90 TraceSegmentClear (semantic: CanTravel) - 3D segment sample (steps = dist * 1.666) with the same
	//                                       kind rules, then falls through to base slot [2]
	//   [3] 0x45ba90 EntityGate           - per-type mover clearance thresholds:
	//                                       type<=3 -> size>=0x2B, 4 -> 0x20, 5 -> 0x15, >=6 -> 0xA
	//   [4] 0x45baf0 scalar deleting destructor (added here; base dtor is non-virtual)
	// The steerer (JE_PathSteerer_ProcessJobs_0x563090) consumes these slots as the A* policy:
	// [0] heuristic, [1] edge cost, [2] direct-line shortcut, [3] per-cell entity gate.
	class GamePathManager : public JE::PathManager {
		//: Methods
		public:
			GamePathManager();               // ctor_0x45ba40
			virtual ~GamePathManager();      // dtor_0x45ba60 (completing); deleting dtor 0x45baf0 = vft[4]

			virtual int Distance(int x1, int y1, int x2, int y2);          // 0x45ba70 slot [0]
			// Semantic name: PathCostCheck. Must share the base virtual's name to keep the slot.
			virtual int DistanceDispatch(dword* pointA, dword* pointB, int param_3); // 0x45bb10 slot [1]
			// Semantic name: CanTravel.
			virtual int TraceSegmentClear(int param_1, float param_2, float param_3, float param_4,
				int param_5, float param_6, float param_7);                // 0x45bc90 slot [2]
			virtual bool EntityGate(byte sizeByte, void* entity);          // 0x45ba90 slot [3]
	}; // GamePathManager
} // namespace F3

#endif // F3_GAMEPATHMANAGER_HPP_
