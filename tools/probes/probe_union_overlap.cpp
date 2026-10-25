// Does MSVC 2002 (the shipped toolchain) accept the shape needed to model a SHIPPED OVERLAP - two
// members that the original addresses at different widths over the same bytes (+0x70 dword and
// +0x72/+0x73 bytes)? An anonymous struct inside a union is a Microsoft extension; if it works here,
// the reorderer can express such overlaps instead of refusing the whole class.
#include <stdio.h>
#include <stddef.h>
typedef unsigned char byte;
typedef unsigned int dword;
typedef unsigned short word;

struct S {
	byte mbr_0x6c;
	union {
		dword mbr_0x70;
		struct {
			word pad_0x70;
			byte mbr_0x72;
			byte mbr_0x73;
		};
	};
	byte mbr_0x74;
};


int main() {
	setvbuf( stdout, NULL, _IONBF, 0 );
	S s;
	s.mbr_0x70 = 0xAABBCCDDu;
	printf( "  dword view %08X\n", s.mbr_0x70 );
	printf( "  byte +0x72 view %02X (expect CC on little-endian)\n", s.mbr_0x72 );
	printf( "  byte +0x73 view %02X (expect BB)\n", s.mbr_0x73 );
	printf( "  offsets: 0x6c=%d 0x70=%d 0x72=%d 0x73=%d 0x74=%d sizeof=%d\n",
	        (int)offsetof(S,mbr_0x6c),(int)offsetof(S,mbr_0x70),(int)offsetof(S,mbr_0x72),
	        (int)offsetof(S,mbr_0x73),(int)offsetof(S,mbr_0x74),(int)sizeof(S) );
	int ok = ( s.mbr_0x72 == 0xCC && s.mbr_0x73 == 0xBB && sizeof( S ) == 0x78 );
	printf( ok ? "PROBE OK\n" : "PROBE FAILED\n" );
	return ok ? 0 : 1;
}
