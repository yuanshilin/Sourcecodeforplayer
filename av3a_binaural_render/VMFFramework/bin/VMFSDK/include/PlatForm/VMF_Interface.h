/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and       
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employees 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an intended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/

/*************************************************************************************************
**      Copyright (c) 2011 by ArcSoft Inc.
**      Name        : VMF_Interface.h
**      Purpose     : the definitions for IVMFInterface
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_I_INTERFACE_H
#define VMF_I_INTERFACE_H

#include <VMF_Config.h>
#include <VMF_Types.h>

typedef struct VMF_GUID_st
{
	VMF_U32 Data1;
	VMF_U16 Data2;
	VMF_U16 Data3;
	VMF_U8  Data4[8];
}VMF_GUID;


#ifndef _STRUCT_VMF_VERSION_
#define _STRUCT_VMF_VERSION_
typedef struct
{
	VMF_U32 dwMajor;
	VMF_U32 dwMinor;
	VMF_U32 dwRelease;
	VMF_U32 dwBuild;
}VMF_Version;
#endif

#if defined(WIN32)||defined(WIN64)
#define VMF_DEFINE_GUID(name, w0, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
	 VMF_EXTERN_C VMF_CONST VMF_GUID (name)
#define VMF_EXTERN_GUID(name, w0, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
       VMF_EXTERN_C VMF_CONST VMF_GUID __declspec(selectany) (name) \
				= { w0, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#else
#define VMF_DEFINE_GUID(name, w0, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
       EXTERN_C VMF_CONST VMF_GUID (name) \
                = { w0, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define VMF_EXTERN_GUID(name, w0, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
       EXTERN_C VMF_CONST VMF_GUID (name)
#endif

#define COPY_GUID(dst, src)  VMF_Memcpy(&(dst), &(src), sizeof(VMF_GUID))

#define VMF_Safe_Release(x) {if ((x) != VMF_NULL) (x)->Release(); (x) = VMF_NULL;}

#ifdef __cplusplus
extern "C" {
#endif 
VMF_API VMF_BOOL VMF_IsEqualGUID(VMF_GUID guid1, VMF_GUID guid2);
VMF_API VMF_BOOL VMF_GetVersion(VMF_Version *pVersion);
#ifdef __cplusplus
}
#endif 
#endif

