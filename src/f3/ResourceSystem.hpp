// Copyright 2024 OpenJE

#ifndef F3_RESOURCESYSTEM_HPP_
#define F3_RESOURCESYSTEM_HPP_

#include "f3/global.hpp"
#include "f3/cls_0x50db20.hpp"
#include "f3/cls_0x50dbb0.hpp"
#include "f3/cls_0x50ca00.hpp"
#include "f3/cls_0x50db20.hpp"

namespace f3 {
    // cls_0x49c610
	class ResourceSystem : public cls_0x50db20, public cls_0x50dbb0, public cls_0x50ca00 {
		//: Members
		public:
			dword mbr_0x10;
			dword mbr_0x12c;
			dword mbr_0x130;
			dword mbr_0x134;
			dword mbr_0x138;
			dword mbr_0x13c;
			cls_0x50db20 mbr_0x14;
			dword mbr_0x140;
			dword mbr_0x144;
			dword mbr_0x28;
			dword mbr_0x4;
			dword mbr_0x44;
			dword mbr_0x60;
			dword mbr_0x7c;
			dword mbr_0x8;
			dword mbr_0x98;
			dword mbr_0xb4;
			dword mbr_0xc;
			dword mbr_0xd0;
			dword mbr_0xec;
		//: Methods
		public:
			ResourceSystem(); // ctor_0x49c610
			undefined4 FileStreamSeek( uint * param_1, int param_2, int param_3 ); // meth_0x498800
			void Release( void * param_1 ); // meth_0x498eb0
			undefined4 FileStreamRead( undefined4 param_1, int param_2, int param_3 ); // meth_0x499220
			void meth_0x499900( undefined4 * param_1 ) // meth_0x499900
			void FileDelete( int * param_1 ) // meth_0x49a910
			void meth_0x49aa90( undefined4 * param_1 ); // meth_0x49aa90
			void meth_0x49abd0( undefined4 * param_1 ); // meth_0x49abd0
			void meth_0x49acd0( undefined4 * param_1 ); // meth_0x49acd0
			void OpenFile( int * param_1, undefined4 param_2 ); // meth_0x49ae90
			void meth_0x49b1d0( void * param_1, char param_2 ); // meth_0x49b1d0
			void meth_0x49b240( undefined4 * param_1, undefined4 param_2 ); // meth_0x49b240
			int * meth_0x49b670( int * param_1, undefined4 param_2 ); // meth_0x49b670
			void Demand( undefined4 * param_1, int * param_2, uint param_3 ); // meth_0x49b750
			void FileStreamOpen( char param_1, uint param_2 ); // meth_0x49b9f0
			void meth_0x49bc60(); // meth_0x49bc60
			int * MemStreamOpen( int * param_1, int * param_2 ) // meth_0x49beb0
			void Init( uint param_1, undefined4 * param_2 ); // meth_0x49bf20
			int meth_0x4e8f50(); // meth_0x4e8f50
			void meth_0x4ea010( int * * param_1, int * param_2, int * param_3 ); //meth_0x4ea010
			void meth_0x4ea390( int * param_1, int * param_2, int * param_3 ); // meth_0x4ea390
			void meth_0x4ec880( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x4ec880
			uint meth_0x50ba50( char * param_1, int param_2 ); // meth_0x50ba50
			undefined4 * meth_0x50bb00( char * param_1, int param_2 ); // meth_0x50bb00
			void meth_0x50bdd0( char * param_1, int * param_2 ); // meth_0x50bdd0
			void meth_0x50bf30( undefined4 * param_1, int param_2, int param_3, undefined4 * param_4, int param_5 ); // meth_0x50bf30
			void meth_0x50c4c0( int * * param_1, int * param_2, int * param_3 ); // meth_0x50c4c0
			void meth_0x50c510( int * param_1, int * param_2, int * param_3 ); // meth_0x50c510
			void meth_0x50c870( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x50c870
			void meth_0x50c9a0( int * param_1 ); // meth_0x50c9a0
			void meth_0x50c9e0( int * param_1 ); // meth_0x50c9e0
			void meth_0x516580( int * * param_1, int * param_2, int * param_3 ); // meth_0x516580
			void meth_0x518f10( int * param_1, int * param_2, int * param_3 ); // meth_0x518f10
			void meth_0x519ca0( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x519ca0
			undefined4 * meth_0x571550( undefined4 * param_1, undefined4 * param_2, undefined4 * param_3 ); // meth_0x571550
			void meth_0x572650( int * param_1, undefined4 * param_2, undefined4 * param_3 ); // meth_0x572650
			void meth_0x574820( undefined4 * param_1 ); // meth_0x574820
			void meth_0x579660( int * * param_1, int * param_2, int * param_3 ); // meth_0x579660
			void meth_0x5796e0( int * * param_1, int * param_2, int * param_3 ); // meth_0x5796e0
			void meth_0x579f60( int * param_1, int * param_2, int * param_3 ); // meth_0x579f60
			void meth_0x57a2c0( int * param_1, int * param_2, int * param_3 ); // meth_0x57a2c0
			void meth_0x57a680( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x57a680
			void meth_0x57a790( int * param_1, undefined4 param_2, int * * param_3 ); // meth_0x57a790
	}; // class ResourceSystem
} // namespace f3

#endif // F3_RESOURCESYSTEM_HPP_
