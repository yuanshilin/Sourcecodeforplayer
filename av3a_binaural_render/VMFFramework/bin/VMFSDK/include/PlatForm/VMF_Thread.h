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
**      Name        : VMF_Thread.h
**      Purpose     : the definitions for Thread,Atomic,Event,Semaphore,mutex
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_THREAD_H
#define VMF_THREAD_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>


#ifndef VMF_ATOMIC
#define VMF_ATOMIC
#define VMF_Atomic VMF_HANDLE
VMF_API VMF_Atomic VMF_CreateAtomic();
VMF_API VMF_S32 VMF_AtomicIncrement(VMF_Atomic hAtomic);
VMF_API VMF_S32 VMF_AtomicDecrement(VMF_Atomic hAtomic);
VMF_API VMF_S32 VMF_GetAtomic(VMF_Atomic hAtomic);
VMF_API VMF_S32 VMF_SetAtomic(VMF_Atomic hAtomic, VMF_S32 t);
VMF_API VMF_BOOL VMF_DeleteAtomic(VMF_Atomic hAtomic);
#define VMF_Safe_DeleteAtomic(x) {if ((x)!=VMF_NULL) VMF_DeleteAtomic(x); (x)=VMF_NULL;}

#endif

#ifndef VMF_SEMAPHORE
#define VMF_SEMAPHORE
#define VMF_Semaphore VMF_HANDLE
VMF_API VMF_Semaphore VMF_CreateSemaphore(VMF_VOID* lpSemaphoreAttributes, 
							  VMF_S32 lInitialCount,
							  VMF_S32 lMaximumCount,
							  VMF_STRING lpName);
VMF_API VMF_BOOL VMF_CloseSemaphore(VMF_Semaphore hSem);
VMF_API VMF_S32 VMF_ReleaseSemaphore(VMF_Semaphore hSem, VMF_S32 lReleaseCount, VMF_S32 *lpPreviousCount);
VMF_API VMF_S32 VMF_GetSemaphoreNum(VMF_Semaphore hSem);
VMF_API VMF_BOOL VMF_ResetSemaphore(VMF_Semaphore hSem);
#define VMF_Safe_CloseSemaphore(x) {if ((x)!=VMF_NULL) VMF_CloseSemaphore(x); (x)=VMF_NULL;}
#endif

#ifndef VMF_MUTEX
#define VMF_MUTEX
#define VMF_Mutex VMF_HANDLE
VMF_API VMF_BOOL VMF_InitMutex(VMF_Mutex *pMutex);
VMF_API VMF_BOOL VMF_DeleteMutex(VMF_Mutex *pMutex);
VMF_API VMF_BOOL VMF_EnterMutex(VMF_Mutex *pMutex);
VMF_API VMF_BOOL VMF_LeaveMutex(VMF_Mutex *pMutex);
#endif

#ifndef VMF_THREAD
#define VMF_THREAD
#ifndef _VMF_DEFINE_THREAD_PRIORITY_
#define _VMF_DEFINE_THREAD_PRIORITY_
#define THREAD_BASE_PRIORITY_LOWRT  15  // value that gets a thread to LowRealtime-1
#define THREAD_BASE_PRIORITY_MAX    2   // maximum thread base priority boost
#define THREAD_BASE_PRIORITY_MIN    (-2)  // minimum thread base priority boost
#define THREAD_BASE_PRIORITY_IDLE   (-15) // value that gets a thread to idle
#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)
#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE
#endif

#define VMF_Thread VMF_HANDLE
#define VMF_SysThread VMF_HANDLE
VMF_API VMF_Thread VMF_CreateThread(VMF_VOID *pThreadAttr, VMF_U32 dwStackSize, VMF_VOID *lpStartAddress, 
							VMF_VOID *lpParameter, VMF_U32 dwCreationFlags, VMF_U32 *lpThreadId);
VMF_API VMF_BOOL VMF_CloseThread(VMF_Thread hThread);
VMF_API VMF_BOOL VMF_SetThreadPriority(VMF_Thread hThread, VMF_S32 Priority);
VMF_API VMF_S32 VMF_GetThreadPriority(VMF_Thread hThread);
#define VMF_Safe_CloseThread(x) {if ((x)!=VMF_NULL) VMF_CloseThread(x); (x)=VMF_NULL;}
VMF_API VMF_S32 VMF_GetProcessPid(); 
VMF_API VMF_SysThread VMF_GetCurrentSysThread();
VMF_API VMF_U64 VMF_GetCurrentSysThreadId();
VMF_API VMF_BOOL VMF_SetSysThreadPriority(VMF_SysThread hThread, VMF_S32 Priority);
VMF_API VMF_U32 VMF_GetSysThreadPriority(VMF_SysThread hThread);
VMF_API VMF_BOOL VMF_CancelThread(VMF_Thread hThread);
#endif

#ifndef VMF_EVENT
#define VMF_EVENT
#define VMF_Event VMF_HANDLE
VMF_API VMF_Event VMF_CreateEvent(VMF_VOID *lpEventAttributes, VMF_BOOL bManualReset, VMF_BOOL bInitialState,VMF_STRING lpName);
VMF_API VMF_BOOL VMF_CloseEvent(VMF_Event hEvent);
VMF_API VMF_BOOL VMF_SetEvent(VMF_Event hEvent);
VMF_API VMF_BOOL VMF_ResetEvent(VMF_Event hEvent);
VMF_API VMF_BOOL VMF_PulseEvent(VMF_Event hEvent);
#define VMF_Safe_CloseEvent(x) {if ((x)!=VMF_NULL) VMF_CloseEvent(x); (x)=VMF_NULL;}
#endif

VMF_API VMF_S32 VMF_WaitForSingleObject(VMF_HANDLE hHandle, VMF_U32 dwMilliseconds);
VMF_API VMF_S32 VMF_WaitForMultipleObjects(VMF_U32 nCount,VMF_HANDLE *pHandles, VMF_BOOL bWaitAll, VMF_U32 dwMilliseconds); 
VMF_API VMF_BOOL VMF_CloseHandle(VMF_HANDLE hHandle);
#define VMF_Safe_CloseHandle(x) {if ((x)!=VMF_NULL) VMF_CloseHandle(x); (x)=VMF_NULL;}

VMF_API VMF_VOID VMF_Sleep(VMF_U32 dwMilliseconds);

#ifndef VMF_TIMER
#define VMF_TIMER
#define VMF_Timer VMF_HANDLE

/* flags for fuEvent parameter of timeSetEvent() function */
#define VMF_TIME_ONESHOT    0x0000   /* program timer for single event */
#define VMF_TIME_PERIODIC   0x0001   /* program for continuous periodic event */
#define VMF_TIME_CALLBACK_FUNCTION      0x0000  /* callback is function */
#define VMF_TIME_CALLBACK_EVENT_SET     0x0010  /* callback is event - use SetEvent */
#define VMF_TIME_CALLBACK_EVENT_PULSE   0x0020  /* callback is event - use PulseEvent */
#define VMF_TIME_KILL_SYNCHRONOUS   0x0100
typedef VMF_VOID (VMF_CALLBACK *VMF_LPTIMECALLBACK)(VMF_Timer hTimer, VMF_U32 uMsg, VMF_VOID* dwUser,
							   VMF_VOID* dw1, VMF_VOID* dw2);   
VMF_API VMF_Timer VMF_TimeSetEvent(VMF_U32 uDelay, VMF_U32 uResolution, VMF_LPTIMECALLBACK lpTimeProc,  
					  VMF_U32* dwUser, VMF_U32 fuEvent);

VMF_API VMF_S32 VMF_TimeKillEvent(VMF_Timer uTimerID);
#endif

#ifdef __cplusplus
}
#endif 

#endif






