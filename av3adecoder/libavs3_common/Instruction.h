#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

////////////////////
// Platform specials.
//
//#undef _M_IX86 //ssheng 2009-03-25
//
//#ifndef _M_X64 //ssheng 2009-03-25
//#define _M_X64
//#endif

#ifndef _CPU_FLAGS_
#define _CPU_FLAGS_

#if defined __x86_64__
    #define linux
#elif defined __aarch64__
	#define ARCH_AARCH64
#endif

#if defined(linux) && !defined(ARCH_AARCH64) && !defined(ARCH_SW64)
#define _M_X64
#endif

#if defined(_MSC_VER) && _MSC_VER >= 1800 // VS2013
	#define _HAVE_AVX
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
	#define _HAVE_AVX
	#define _HAVE_AVX512
#endif

#ifdef _M_IX86
	#define _MMX
	#define _MME	//sse
	#define _XMM
	#define _XME	//sse2
	#define _XMF
	#define _XMT	//ssse3
	#define _SSE4	
#ifdef _HAVE_AVX
    #define _AVX2
	#define _AVX512
#endif
#elif defined(_M_X64)
	#define _XME
	#define _XMT
	#define _SSE4	
#ifdef _HAVE_AVX
    #define _AVX2
#endif
#ifdef _HAVE_AVX512
	#define _AVX512
#endif
#elif defined(_WIN64)
	#define _XME
	#define _XMT
	#define _SSE4	
#ifdef _HAVE_AVX
	#define _AVX2
#endif
#elif defined(ARCH_AARCH64) && !defined(ARCH_SW64)
	#define _NEON
#else
	#define _CXX
#endif
#endif


////////////////////
// Intrinsics header.
//
#ifdef _MSC_VER
#include <intrin.h>
#endif

#if defined(_MMX) && defined(SUPPORT_MMX)
#include <mmintrin.h>
typedef __m64 MMX, *PMMX;
#endif
//
#if defined(_XMM) && defined(SUPPORT_SSE)
#include <xmmintrin.h>
typedef __m128 XMM, *PXMM;
#endif
//
#if defined(_XME) && defined(SUPPORT_SSE2)
#include <emmintrin.h>
typedef __m128d XMD, *PXMD;
typedef __m128i XME, *PXME;
#endif

#if defined(_XMT) && defined(SUPPORT_SSSE3)
#include <tmmintrin.h>
#endif

#if defined(_SSE4) && defined(SUPPORT_SSE4)
#include <smmintrin.h>
#endif

#if defined(_AVX2) && defined(SUPPORT_AVX2)
#include <immintrin.h>
#endif

#if defined(_AVX512) && defined(SUPPORT_AVX512)
#include <immintrin.h>
#endif

#if defined(_NEON) && defined(SUPPORT_NEON)
#ifndef UINT64_C
#define UINT64_C(c)	c ## ULL
#endif

#ifndef INT64_C
#define INT64_C(c)	c ## LL
#endif
#include <arm_neon.h>
#endif

#ifdef _SRCALIGN
#define _mm_loadu_si128 _mm_load_si128
#endif
#ifdef _DSTALIGN
#define _mm_storeu_si128 _mm_stream_si128
#endif

//v210 
#define READ_PIXELS(src, a, b, c, dstshift) \
{											\
	a =  (src & 0x3FF) >> dstshift;			\
	b = ((src >> 10) & 0x3FF) >> dstshift;	\
	c = ((src >> 20) & 0x3FF) >> dstshift;	\
}

#define WRITE_PIXELS(dst, a, b, c, srcshift)	\
{												\
	dst = ((a << srcshift) & 0x3FF)		|		\
	(((b << srcshift) & 0x3FF) << 10)	|		\
	(((c << srcshift) & 0x3FF) << 20);			\
}