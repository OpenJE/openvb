// Copyright 2024 OpenJE
// R-USERRE-A slice-5b: the command registry TU. Split out of src/F3.cpp verbatim (global
// definition + RegisterCommand body and its citation block moved, character-for-character) so
// the console subsystem - registration, FIFO, drain, dispatch - links as ONE unit and the
// run-all test table can drive Execute() without pulling the whole app TU. No behavior change;
// same symbols, same bodies, same EAs cited. See Commands.cpp for the dispatch half.

#include "F3.hpp"

#include <stdlib.h>
#include <string.h>

#include "JE.hpp"
#include "JE/cls_0x4d8d70.hpp"
#include "JE/cls_0x50db20.hpp"
#include "tracing.hpp"

namespace F3 {

	// 0x70C668 - the command registry. The original builds this during static initialisation
	// (F3::GLOBAL__I_command_registry @ 0x643f60 -> JE::cls_0x4d8d70::ctor_0x59f500, then _atexit of the
	// 0x646b90 thunk -> JE::cls_0x4d8d70::meth_0x59f4d0). It is a real object, never heap-allocated and
	// never NULL, so a namespace-scope instance with the reconstructed constructor/destructor is the
	// faithful model: MSVC runs the ctor before WinMain and registers the dtor with atexit, exactly like
	// the hand-rolled _GLOBAL__I_ walker in the original.
	JE::cls_0x4d8d70 g_commandRegistryRoot;

	// 0x59F5D0
	// Original flow (disasm 0x59f5d0..0x59f6c4):
	//   0x59f5fc-0x59f60c  build a stack tempString (cls_0x50db20: string sub-object at +4 with
	//                      capacity=15 @+0x18, size=0 @+0x14, buffer[0]=0 @+0x04)
	//   0x59f623           cls_0x50db20::meth_0x401bd0(ArgList, strlen(ArgList))  (assign)
	//   0x59f63a           v2 = cls_0x4d8d70::meth_0x545710 lower_bound over the registry (0x70C668)
	//   0x59f649           if (v2 == header) skip the duplicate test entirely (header key never read)
	//   0x59f674           _stricmp(query, nodeKey) >= 0 (lower_bound guarantees nodeKey >= query, so
	//                      this means equality) -> FatalError("Command %s already registered.")
	//   0x59f6b5           *cls_0x4d8d70::meth_0x59f540(&clone) = func_ptr  (insert-or-get returns the
	//                      node's value slot at node+0x28; the clone/insert path is folded into
	//                      InsertOrGet here)
	//   0x59f6bd           if (capacity >= 0x10) free(heap buffer)
	void RegisterCommand( const char* ArgList, void (__cdecl *func_ptr)() ) {
		tracing::instrument( tracing::LOCATION, "ArgList=%s", ArgList );

		// tempString in SSO state, mirroring 0x59f5fc-0x59f60c (object word, buffer[0], size, capacity).
		JE::cls_0x50db20 tempString;
		byte* raw = reinterpret_cast<byte*>( &tempString );
		raw[0] = 0;
		raw[4] = 0;
		*reinterpret_cast<dword*>( raw + 0x14 ) = 0;    // size
		*reinterpret_cast<dword*>( raw + 0x18 ) = 15;   // capacity (inline)

		tempString.meth_0x401bd0( ArgList, static_cast<int>( strlen( ArgList ) ) );

		void* node = g_commandRegistryRoot.LowerBound( &tempString );
		if ( node != reinterpret_cast<void*>( g_commandRegistryRoot.mbr_0x4 ) &&
		     _stricmp( ArgList, JE::cls_0x4d8d70::NodeKeyText( node ) ) == 0 ) {
			JE::FatalError( "Command %s already registered.", ArgList );
			return;
		}

		void** valueSlot = reinterpret_cast<void**>( g_commandRegistryRoot.InsertOrGet( &tempString ) );
		*valueSlot = reinterpret_cast<void*>( func_ptr );

		if ( *reinterpret_cast<dword*>( raw + 0x18 ) >= 0x10 ) {
			free( *reinterpret_cast<void**>( raw + 4 ) );   // 0x59f6c4: j__free(String1[0])
		}
	}

} // namespace F3
