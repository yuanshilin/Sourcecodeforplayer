#ifndef VMF_WINDOW_EX_H
#define VMF_WINDOW_EX_H

#ifdef __cplusplus
extern "C" {
#endif 

VMF_API HRESULT  VMF_GetDefaultWritePath(VMF_WCHAR *pPath, VMF_U32 *pLength);
VMF_API HRESULT  VMF_GetModuleFileName(VMF_WCHAR *pPath, VMF_U32 *pLength);

#ifdef __cplusplus
}
#endif 
#endif//VMF_WINDOW_EX_H