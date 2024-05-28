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
**      Name        : VMF_Time.h
**      Purpose     : the definitions for time
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_TIME_H
#define VMF_TIME_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>

VMF_API VMF_BOOL VMF_GetCurTime( VMF_TIME *timer);
VMF_API VMF_DOUBLE VMF_DiffTime(VMF_TIME time2, VMF_TIME time1);
VMF_API VMF_U64 VMF_GetTime();//in microseconds 

typedef struct
{ 
	VMF_U32 wPeriodMin; 
	VMF_U32 wPeriodMax; 
} VMF_TIMECAPS;

#define VMF_TIMERR_NOERROR 0
#define VMF_TIMERR_NOCANDO -1
#define VMF_TIMERR_INVALPARAM -2

VMF_API VMF_U32 VMF_TimeGetDevCaps(VMF_TIMECAPS *pptc,  VMF_U32 cbtc);
VMF_API VMF_U32 VMF_TimeEndPeriod(VMF_U32 uPeriod);
VMF_API VMF_U32 VMF_TimeBeginPeriod(VMF_U32 uPeriod);

#ifdef __cplusplus
}
#endif 

#endif





