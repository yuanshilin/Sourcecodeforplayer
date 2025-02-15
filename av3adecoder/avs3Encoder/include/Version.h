#pragma once
#include "Instruction.h"

//add version for encoder
#ifdef linux
#ifdef __cplusplus
extern "C" {
#endif

#define VER_MAJOR				1
#define VER_MINOR				0
#define VER_RELEASE				0
#define VER_BUILD				1

#define GENERIC_VERION(Major, Minor, Release, Build)\
void VERSION_AVS3AudioEnc_##Major##_##Minor##_##Release##_##Build(){ }

GENERIC_VERION(1, 0, 0, 1)

#ifdef __cplusplus
}
#endif
#endif


#ifdef ARCH_AARCH64
#ifdef __cplusplus
extern "C" {
#endif

#define VER_MAJOR				1
#define VER_MINOR				0
#define VER_RELEASE				0
#define VER_BUILD				1

#define GENERIC_VERION(Major, Minor, Release, Build)\
void VERSION_AVS3AudioEnc_##Major##_##Minor##_##Release##_##Build(){ }

GENERIC_VERION(1, 0, 0, 1)

#ifdef __cplusplus
}
#endif
#endif