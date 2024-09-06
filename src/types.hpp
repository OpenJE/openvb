// Copyright 2024 OpenJE

#ifndef TYPES_HPP
#define TYPES_HPP

#include "stdafx.h"

// Shorthand Types
typedef unsigned char       uchar;
typedef unsigned short      ushort;
typedef unsigned int        uint;
typedef unsigned long       ulong;
typedef unsigned long long  ulonglong;

// Ghidra Types
typedef uchar       undefined;
typedef ushort      undefined2;
typedef uint        undefined4;
typedef ulonglong   undefined8;
typedef void        code;
typedef long double float10;

// Types
typedef char        int8_t;
typedef short       int16_t;
typedef int         int32_t;
typedef long long   int64_t;
typedef uchar       uint8_t;
typedef ushort      uint16_t;
typedef uint        uint32_t;
typedef ulonglong   uint64_t;

// Ghidra Operations
// Concatination
#define CONCAT11( x, y ) ( (  (uint16_t)( x ) << 8 ) | (  (uint8_t)( y ) ) )
#define CONCAT12( x, y ) ( ( (uint16_t)( x ) << 16 ) | ( (uint16_t)( y ) ) )
#define CONCAT21( x, y ) ( (  (uint16_t)( x ) << 8 ) | (  (uint8_t)( y ) ) )
#define CONCAT22( x, y ) ( ( (uint32_t)( x ) << 16 ) | ( (uint16_t)( y ) ) )
#define CONCAT31( x, y ) ( (  (uint32_t)( x ) << 8 ) | (  (uint8_t)( y ) ) )
#define CONCAT32( x, y ) ( ( (uint32_t)( x ) << 16 ) | ( (uint16_t)( y ) ) )
#define CONCAT33( x, y ) ( ( (uint32_t)( x ) << 24 ) | ( (uint32_t)( y ) ) )
#define CONCAT41( x, y ) ( (  (uint64_t)( x ) << 8 ) | (  (uint8_t)( y ) ) )
#define CONCAT42( x, y ) ( ( (uint64_t)( x ) << 16 ) | ( (uint16_t)( y ) ) )
#define CONCAT43( x, y ) ( ( (uint64_t)( x ) << 24 ) | ( (uint32_t)( y ) ) )
#define CONCAT44( x, y ) ( ( (uint64_t)( x ) << 32 ) | ( (uint32_t)( y ) ) )
#define CONCAT51( x, y ) ( (  (uint64_t)( x ) << 8 ) | (  (uint8_t)( y ) ) )
#define CONCAT52( x, y ) ( ( (uint64_t)( x ) << 16 ) | ( (uint16_t)( y ) ) )
#define CONCAT53( x, y ) ( ( (uint64_t)( x ) << 24 ) | ( (uint32_t)( y ) ) )
#define CONCAT54( x, y ) ( ( (uint64_t)( x ) << 32 ) | ( (uint32_t)( y ) ) )
#define CONCAT55( x, y ) ( ( (uint64_t)( x ) << 40 ) | ( (uint64_t)( y ) ) )
#define CONCAT61( x, y ) ( (  (uint64_t)( x ) << 8 ) | (  (uint8_t)( y ) ) )
#define CONCAT62( x, y ) ( ( (uint64_t)( x ) << 16 ) | ( (uint16_t)( y ) ) )
#define CONCAT63( x, y ) ( ( (uint64_t)( x ) << 24 ) | ( (uint32_t)( y ) ) )
#define CONCAT64( x, y ) ( ( (uint64_t)( x ) << 32 ) | ( (uint32_t)( y ) ) )
#define CONCAT65( x, y ) ( ( (uint64_t)( x ) << 40 ) | ( (uint64_t)( y ) ) )
#define CONCAT66( x, y ) ( ( (uint64_t)( x ) << 48 ) | ( (uint64_t)( y ) ) )
// Data Moving
//#define COPY
//#define LOAD
//#define STORE
// Integer Arithmetic
//#define INT_ADD
//#define INT_SUB
//#define INT_2COMP
//#define INT_MULT
//#define INT_DIV
//#define INT_SDIV
//#define INT_REM
//#define INT_SREM
// Integer Shifts
//#define INT_LEFT
//#define INT_RIGHT
//#define INT_SRIGHT
// Integer Comparison
//#define INT_EQUAL
//#define INT_NOTEQUAL
//#define INT_SLESS
//#define INT_SLESSEQUAL
//#define INT_LESS
//#define INT_LESSEQUAL
// Logical
//#define INT_NEGATE
//#define INT_XOR
//#define INT_AND
//#define INT_OR
//#define POPCOUNT
// Boolean
//#define BOOL_NEGATE
//#define BOOL_XOR
//#define BOOL_AND
//#define BOOL_OR
// Floating Point Arithmetic
//#define FLOAT_ADD
//#define FLOAT_SUB
//#define FLOAT_MULT
//#define FLOAT_DIV
//#define FLOAT_NEG
//#define FLOAT_ABS
//#define FLOAT_SQRT
// Floating Point Comparison
//#define FLOAT_EQUAL
//#define FLOAT_NOTEQUAL
//#define FLOAT_LESS
//#define FLOAT_LESSEQUAL
// Floating Point Conversion
//#define INT2FLOAT
//#define FLOAT2FLOAT
//#define TRUNC( x )
#define CEIL( x ) ceil( x )
#define FLOOR( x ) floor( x )
#define ROUND( x ) ( ( ( x ) >= 0.0f ) ? floorf( ( x ) + 0.5f ) : ceilf( ( x ) - 0.5f ) )
// Branching
//#define BRANCH
//#define CBRANCH
//#define BRANCHIND
//#define CALL
//#define CALLIND
//#define RETURN
// Extension / Truncation
//#define INT_ZEXT
//#define INT_SEXT
//#define PIECE
//#define SUBPIECE
// Overflow Tests
//#define INT_CARRY
//#define INT_SCARRY
//#define INT_SBORROW
//#define FLOAT_NAN
// Managed Code
//#define CPOOLREF
//#define NEW

// OOAnalyzer Types
typedef uchar       byte;
typedef ushort      word ;
typedef uint        dword;
typedef ulonglong   qword;

#endif // TYPES_HPP