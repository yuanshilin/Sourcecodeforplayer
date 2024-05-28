#pragma once

#define VER_MAJOR				1
#define VER_MINOR				0
#define VER_RELEASE				0
#define VER_BUILD				1

#ifdef __cplusplus
extern "C" {
#endif 
#define GENERIC_VERION(Major, Minor, Release, Build)\
VMF_API VMF_VOID VERSION_libav3a_binaural_render_##Major##_##Minor##_##Release##_##Build(){ }

GENERIC_VERION(1, 0, 0, 1)
#ifdef __cplusplus
}
#endif 