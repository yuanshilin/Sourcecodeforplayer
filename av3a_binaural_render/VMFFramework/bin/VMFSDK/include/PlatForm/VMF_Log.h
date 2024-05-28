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
**      Name        : VMF_Log.h
**      Purpose     : the definitions for logs
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_LOG_H
#define VMF_LOG_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>

VMF_API VMF_S32 VMF_Log(VMF_CONST VMF_STRING format, ...);
VMF_API VMF_S32 VMF_LogW(VMF_CONST VMF_WSTRING format, ...);
#ifdef UNICODE
#define VMF_LogT VMF_LogW
#else
#define VMF_LogT VMF_Log
#endif
VMF_API VMF_S32 VMF_LogIF(VMF_U32 Flag,VMF_U32 LogFlag, VMF_CONST VMF_STRING format, ...);
VMF_API VMF_S32 VMF_LevelLog(VMF_U32 LevelFlag, VMF_U32 Flag, VMF_CONST VMF_STRING format, ...);

#if defined(WIN32)||defined(WIN64)
#define __func__ __FUNCTION__
#endif

#ifdef __cplusplus
}
#endif 

#endif





