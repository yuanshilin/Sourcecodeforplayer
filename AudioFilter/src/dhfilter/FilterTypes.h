
#ifndef FILTER_TYPES_H
#define FILTER_TYPES_H

#include <stdint.h>

#ifdef _LINUX_ANDROID_

typedef void				MVoid;
typedef signed char			MInt8;
typedef unsigned char		MUInt8;
typedef char*               MPChar;
typedef const char*         MPCChar;
typedef signed short		MInt16;
typedef unsigned short		MUInt16;
typedef int32_t             MInt32;
typedef uint32_t            MUInt32;
typedef int64_t             MInt64;
typedef uint64_t            MUInt64;

typedef float               MFloat;
typedef double              MDouble;
#else
typedef void				MVoid;
typedef signed char			MInt8;
typedef unsigned char		MUInt8;
typedef char*               MPChar;
typedef const char*         MPCChar;
typedef signed short		MInt16;
typedef unsigned short		MUInt16;
typedef signed int          MInt32;
typedef unsigned int        MUInt32;
typedef signed long long    MInt64;
typedef unsigned long long  MUInt64;

typedef float               MFloat;
typedef double              MDouble;
#endif

#endif
