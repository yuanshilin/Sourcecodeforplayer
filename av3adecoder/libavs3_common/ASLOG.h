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
*  Name  : ASLOG.h
*  Description: unify the way of output log infomation
*  Maintenance History:     2010/5/22, created by yxie@arcsoft.com.cn
*-------------------------------------------------------------------------------------------------*/

#ifndef _ASLOG_H_
#define _ASLOG_H_


// #ifndef __FUNCTION__
// #define __FUNCTION__	"FuncUnknown"
// #endif

#ifndef __FILEW__
#define __STR2WSTR(str)    L##str
#define _STR2WSTR(str)     __STR2WSTR(str)
#define __FILEW__          _STR2WSTR(__FILE__)
#define __FUNCTIONW__      _STR2WSTR(__FUNCTION__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include "VMF_PlatForm.h"
#define LogLevel_FatalError		1
#define LogLevel_Error			2
#define LogLevel_Warning		3
#define LogLevel_KeyInfo		4
#define LogLevel_GeneralInfo	5	

//extern VMF_S32 g_logLevel;

extern WCHAR* _ModuleName_;
extern int _ModuleID_;

void ASLOG_Init();
void ASLOG_Uninit();


#ifdef UNICODE
#define ASLOG_Msg ASLOG_MsgW
#define ASLOG_tMsg ASLOG_tMsgW
#define ASLOG_Function_Enter ASLOG_Function_EnterW
#define ASLOG_Function_Leave ASLOG_Function_LeaveW
#else
#define ASLOG_Msg ASLOG_MsgA
#define ASLOG_tMsg ASLOG_tMsgA
#define ASLOG_Function_Enter ASLOG_Function_EnterA
#define ASLOG_Function_Leave ASLOG_Function_LeaveA
#endif


void ASLOG_MsgA(unsigned long  LogLevel,const char *format,...);
void ASLOG_MsgW(unsigned long  LogLevel,const WCHAR *format,...);
void ASLOG_tMsgA(unsigned long  LogLevel,const char *format,...);
void ASLOG_tMsgW(unsigned long  LogLevel,const WCHAR *format,...);
void ASLOG_ErrorNotify(int errCode, const char *errStr);
void ASLOG_MsgA_Simple(int nMsgID, const char *pFormatSimple, unsigned long ulLogLevel, const char *pFormat, ...);
void *ASLOG_SetupProcSnap(void *pModule, char *pMuduleName, char *pVarName);
void ASLOG_UpdateProcSnap(void *pModuleSnap, long long llVar);
void ASLOG_UpdateProcSnapString(void *pModuleSnap, char *pVar);
void ASLOG_UpdateProcSnapBuffer(void *pModuleSnap, void *pVar, int nLen);
void ASLOG_RemoveProcSnap(void *pModule);

#define ASLOG_Function_EnterA()	\
	if(g_logLevel>=LogLevel_GeneralInfo)\
	{\
		char filename[MAX_PATH];\
		char *p;\
		strcpy(filename,__FILE__);\
		p = filename + strlen(filename) - 1;\
		while(*p != '\\' && *p!= '/' && p>filename ) p--; \
		if(*p=='\\' || *p== L'/') p++;\
		ASLOG_MsgA(LogLevel_GeneralInfo,"enter %s() in file %s, line %d",__FUNCTION__,p,__LINE__);\
	}\
		
#define ASLOG_Function_EnterW()	\
	if(g_logLevel>=LogLevel_GeneralInfo)\
	{\
		char filename[MAX_PATH];\
		char *p;\
		strcpy(filename,__FILE__);\
		p = filename + strlen(filename) - 1;\
		while(*p != '\\' && *p!= '/' && p>filename ) p--; \
		if(*p=='\\' || *p== L'/') p++;\
		ASLOG_MsgW(LogLevel_GeneralInfo,L"enter %s() in file %s, line %d",__FUNCTION__,p,__LINE__);\
	}\


#define ASLOG_Function_LeaveA()	\
	if(g_logLevel>=LogLevel_GeneralInfo)\
	{\
		char filename[MAX_PATH];\
		char *p;\
		VMF_Strcpy(filename,__FILE__);\
		p = filename + strlen(filename) - 1;\
		while(*p != '\\' && *p!= '/' && p>filename ) p--; \
		if(*p=='\\' || *p=='/') p++;\
		ASLOG_MsgA(LogLevel_GeneralInfo,"leave %s() in file %s, line %d",__FUNCTION__,p,__LINE__);\
	}\

#define ASLOG_Function_LeaveW()	\
	if(g_logLevel>=LogLevel_GeneralInfo)\
	{\
		char filename[MAX_PATH];\
		char *p;\
		VMF_Strcpy(filename,__FILE__);\
		p = filename + strlen(filename) - 1;\
		while(*p != '\\' && *p!= '/' && p>filename ) p--; \
		if(*p=='\\' || *p=='/') p++;\
		ASLOG_MsgW(LogLevel_GeneralInfo,L"leave %s() in file %s, line %d",__FUNCTION__,p,__LINE__);\
	}\

#ifdef __cplusplus
}
#endif

#endif