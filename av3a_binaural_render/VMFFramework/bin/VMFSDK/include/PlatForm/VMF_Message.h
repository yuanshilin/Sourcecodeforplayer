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
**      Name        : VMF_Message.h
**      Purpose     : the definitions for Message
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_MESSAGE_H
#define VMF_MESSAGE_H

#include <VMF_Config.h>
#include <VMF_Thread.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define VMF_MSGHandle VMF_HANDLE//VMF_PMSGHandle is VMFMSGHandle

typedef struct
{
	VMF_S32  x;
	VMF_S32  y;
} VMF_POINT;

typedef struct {
	VMF_MSGHandle hHandle;
	VMF_U32 message;
	VMF_VOID *wParam;
	VMF_VOID *lParam;
	VMF_U32 time;
	VMF_POINT pt;
} VMF_MSG, *VMF_PMSG;

typedef VMF_BOOL (VMF_CALLBACK *VMF_PeekMessageFunc)(VMF_MSG *pMsg, VMF_MSGHandle hHandle, VMF_U32 wMsgFilterMin, 
						 VMF_U32 wMsgFilterMax, VMF_U32 wRemoveMsg);
typedef VMF_VOID* (VMF_CALLBACK *VMF_DispatchMessageFunc)(const VMF_MSG *lpmsg);
typedef VMF_VOID* (VMF_CALLBACK *VMF_SendMessageFunc)(VMF_MSGHandle hHandle, VMF_U32 Msg, VMF_VOID* wParam, VMF_VOID* lParam);
typedef VMF_BOOL (VMF_CALLBACK *VMF_PostMessageFunc)(VMF_MSGHandle hHandle, VMF_U32 Msg, VMF_VOID* wParam, VMF_VOID* lParam);

typedef struct VMF_MSGCall_st{
	VMF_VOID *pCall;
	VMF_PeekMessageFunc pPeekMessage;
	VMF_DispatchMessageFunc pDispatchMessage;
	VMF_SendMessageFunc pSendMessage;
	VMF_PostMessageFunc pPostMessage;
}VMF_MSGCall, *VMF_PMSGHandle;

/*
* PeekMessage() Options
*/
#define VMF_PM_NOREMOVE         0x0000
#define VMF_PM_REMOVE           0x0001
#define VMF_PM_NOYIELD          0x0002
#define VMF_QS_KEY              0x0001
#define VMF_QS_MOUSEMOVE        0x0002
#define VMF_QS_MOUSEBUTTON      0x0004
#define VMF_QS_POSTMESSAGE      0x0008
#define VMF_QS_TIMER            0x0010
#define VMF_QS_PAINT            0x0020
#define VMF_QS_SENDMESSAGE      0x0040
#define VMF_QS_HOTKEY           0x0080
#define VMF_QS_ALLPOSTMESSAGE   0x0100
#define VMF_QS_RAWINPUT         0x0400

VMF_API VMF_BOOL VMF_PeekMessage(VMF_MSG *pMsg, VMF_MSGHandle hHandle, VMF_U32 wMsgFilterMin, 
						 VMF_U32 wMsgFilterMax, VMF_U32 wRemoveMsg);
VMF_API VMF_BOOL VMF_PostSysThreadMessage(VMF_U32 idThread, VMF_U32 Msg, VMF_VOID* wParam, VMF_VOID* lParam);
VMF_API VMF_BOOL VMF_PostThreadMessage(VMF_Thread hThread, VMF_U32 Msg, VMF_VOID* wParam, VMF_VOID* lParam);
VMF_API VMF_VOID* VMF_DispatchMessage(const VMF_MSG *lpmsg);
VMF_API VMF_VOID* VMF_SendMessage(VMF_MSGHandle hHandle, VMF_U32 Msg, VMF_VOID* wParam, VMF_VOID* lParam);
VMF_API VMF_BOOL VMF_PostMessage(VMF_MSGHandle hHandle, VMF_U32 Msg, VMF_VOID* wParam, VMF_VOID* lParam);
VMF_API VMF_U32 VMF_GetQueueStatus(VMF_U32 flags);
VMF_API VMF_U32 VMF_RegisterWindowMessage(VMF_TCHAR *lpString);
VMF_API VMF_U32 VMF_MsgWaitForMultipleObjects(VMF_U32 nCount, VMF_CONST VMF_HANDLE* pHandles,
									   VMF_BOOL bWaitAll,VMF_U32 dwMilliseconds,VMF_U32 dwWakeMask);
#ifdef __cplusplus
}
#endif 

#endif
