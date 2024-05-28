/*----------------------------------------------------------------------------------------------
*
* This file is ArcSoft's property. It contains ArcSoft's trade secret, proprietary and 		
* confidential information. 
* 
* The information and code contained in this file is only for authorized ArcSoft employemp4 
* to design, create, modify, or review.
* 
* DO NOT DISTRIBUTE, DO NOT DUPLICATE OR TRANSMIT IN ANY FORM WITHOUT PROPER AUTHORIZATION.
* 
* If you are not an INTended recipient of this file, you must not copy, distribute, modify, 
* or take any action in reliance on it. 
* 
* If you have received this file in error, please immediately notify ArcSoft and 
* permanently delete the original and any copy of any file and any printout thereof.
*
*-------------------------------------------------------------------------------------------------*/

/*************************************************************************************************
**      Copyright (c) 2011 by ArcSoft Inc.
**      Name        : VMF_Memory.h
**      Purpose     : the definitions for memory
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_MEMORY_H
#define VMF_MEMORY_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>

VMF_API VMF_VOID* VMF_Malloc(VMF_S32 Size);
VMF_API VMF_VOID* VMF_Calloc(VMF_S32 Num, VMF_S32 Size);
VMF_API VMF_VOID* VMF_Realloc(VMF_VOID* ptr, VMF_S32 NewSize);
VMF_API VMF_VOID VMF_Free(VMF_VOID* ptr);
#define VMF_Safe_Free(p) {if ((p) != VMF_NULL) VMF_Free(p); (p)=VMF_NULL;}

#define VMF_PAGE_NOACCESS          0x01     
#define VMF_PAGE_READONLY          0x02     
#define VMF_PAGE_READWRITE         0x04     
#define VMF_PAGE_WRITECOPY         0x08     
#define VMF_PAGE_EXECUTE           0x10     
#define VMF_PAGE_EXECUTE_READ      0x20     
#define VMF_PAGE_EXECUTE_READWRITE 0x40     
#define VMF_PAGE_EXECUTE_WRITECOPY 0x80     
#define VMF_PAGE_GUARD            0x100     
#define VMF_PAGE_NOCACHE          0x200     
#define VMF_PAGE_WRITECOMBINE     0x400     
#define VMF_MEM_COMMIT           0x1000     
#define VMF_MEM_RESERVE          0x2000     
#define VMF_MEM_DECOMMIT         0x4000     
#define VMF_MEM_RELEASE          0x8000     
#define VMF_MEM_FREE            0x10000     
#define VMF_MEM_PRIVATE         0x20000     
#define VMF_MEM_MAPPED          0x40000     
#define VMF_MEM_RESET           0x80000     
#define VMF_MEM_TOP_DOWN       0x100000     
#define VMF_MEM_WRITE_WATCH    0x200000     
#define VMF_MEM_PHYSICAL       0x400000     
#define VMF_MEM_LARGE_PAGES  0x20000000     
#define VMF_MEM_4MB_PAGES    0x80000000     
#define VMF_SEC_FILE           0x800000     
#define VMF_SEC_IMAGE         0x1000000     
#define VMF_SEC_RESERVE       0x4000000     
#define VMF_SEC_COMMIT        0x8000000     
#define VMF_SEC_NOCACHE      0x10000000     
#define VMF_SEC_LARGE_PAGES  0x80000000       
#define VMF_WRITE_WATCH_FLAG_RESET 0x01  
VMF_API VMF_VOID* VMF_VirtualAlloc(VMF_VOID *lpAddress, VMF_S32 Size, VMF_U32 flAllocationType, VMF_U32 flProtect);
VMF_API VMF_BOOL VMF_VirtualFree(VMF_VOID *lpAddress, VMF_S32 Size, VMF_U32 dwFreeType);

VMF_API VMF_VOID* VMF_AlignedMalloc(VMF_S32 Size, VMF_S32 alignment);
VMF_API VMF_VOID* VMF_AlignedCalloc(VMF_S32 Num, VMF_S32 Size, VMF_S32 alignment); 
VMF_API VMF_VOID VMF_AlignedFree(VMF_VOID* ptr);
#define VMF_Safe_AlignedFree(p) {VMF_AlignedFree(p); (p)=VMF_NULL;}

VMF_API VMF_VOID* VMF_Memcpy(VMF_VOID *pDst, VMF_CONST VMF_VOID *pSrc, VMF_S32 Size);
VMF_API VMF_VOID* VMF_Memmove(VMF_VOID *pDst, VMF_CONST VMF_VOID*pSrc, VMF_S32 Size);
VMF_API VMF_VOID* VMF_Memset(VMF_VOID* ptr, VMF_S32 Value, VMF_S32 Num);
VMF_API VMF_S32 VMF_Memcmp(VMF_CONST VMF_VOID *ptr1, VMF_CONST VMF_VOID *ptr2, VMF_S32 Num);

#ifdef __cplusplus
}
#endif 

#endif





