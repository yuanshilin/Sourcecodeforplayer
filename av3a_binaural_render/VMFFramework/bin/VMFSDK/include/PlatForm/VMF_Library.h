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
**      Name        : VMF_Library.h
**      Purpose     : the definitions for Library
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_LIBRARY_H
#define VMF_LIBRARY_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>

#ifndef VMF_LIBRAGRY
#define VMF_LIBRAGRY
#define VMF_Lib  VMF_HANDLE
VMF_API VMF_Lib VMF_LoadLibrary(VMF_STRING pLibName);
VMF_API VMF_VOID* VMF_GetProcAddress(VMF_Lib hLib, VMF_STRING pProcName);
VMF_API VMF_BOOL VMF_FreeLibrary(VMF_Lib hLib);
#define VMF_Safe_FreeLibrary(x) {if ((x) != VMF_NULL) VMF_FreeLibrary(x); (x) = VMF_NULL;}
#endif

#ifdef __cplusplus
}
#endif 

#endif






