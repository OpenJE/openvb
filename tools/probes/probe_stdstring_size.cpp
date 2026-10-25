#include <stdio.h>
#define _HAS_ITERATOR_DEBUGGING 0
#include <string>
int main() {
	std::string s;
	unsigned char* p = (unsigned char*)&s;
	// SSO layout: pointer (or buffer) at +0, size, capacity somewhere after it.
	printf( "sizeof(std::string) = 0x%X\n", (int)sizeof( std::string ) );
	printf( "cap-looking dwords:" );
	for (int i = 0; i + 3 < (int)sizeof( std::string ); i += 4) {
		int v = *(int*)(p + i);
		printf( " +0x%X=%d", i, v );
	}
	printf( "\n" );
	return 0;
}
