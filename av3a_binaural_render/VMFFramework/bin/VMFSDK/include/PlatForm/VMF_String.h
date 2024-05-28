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
**      Name        : VMF_String.h
**      Purpose     : the definitions for strings
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef  VMF_STRING_H
#define  VMF_STRING_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif 

#include <VMF_Types.h>

VMF_API VMF_STRING VMF_Strcpy(VMF_STRING destination, VMF_STRING source);
VMF_API VMF_STRING VMF_Strncpy(VMF_STRING destination, VMF_CONST VMF_STRING source, VMF_S32 num);
VMF_API VMF_STRING VMF_Strcat(VMF_STRING destination, VMF_CONST VMF_STRING source);
VMF_API VMF_STRING VMF_SafeStrcat(VMF_STRING destination, VMF_S32 size, VMF_CONST VMF_STRING source);
VMF_API VMF_STRING VMF_Strncat(VMF_STRING destination, VMF_STRING source, VMF_S32 num);
VMF_API VMF_S32 VMF_Strcmp(VMF_CONST VMF_STRING str1, VMF_CONST VMF_STRING str2);
VMF_API VMF_S32 VMF_Strncmp(VMF_CONST VMF_STRING str1, VMF_CONST VMF_STRING str2, VMF_S32 num);
VMF_API VMF_S32 VMF_Strlen(VMF_CONST VMF_STRING str);
VMF_API VMF_S32 VMF_Sscanf(VMF_CONST VMF_STRING str, const VMF_CHAR *format, VMF_VOID **ppOut, VMF_U32 nCount);
VMF_API VMF_S32 VMF_Sprintf(VMF_CONST VMF_STRING str,VMF_CONST VMF_CHAR *format, ...);
VMF_API VMF_S32 VMF_SafeSprintf(VMF_CONST VMF_STRING str,VMF_S32 size,VMF_CONST VMF_CHAR *format, ...);
VMF_API VMF_S32 VMF_Printf(VMF_CONST VMF_CHAR *format, ...);
VMF_API VMF_STRING VMF_Strstr(VMF_STRING str1, VMF_STRING str2);

VMF_API VMF_WSTRING VMF_StrcpyW(VMF_WSTRING destination, VMF_WSTRING source);
VMF_API VMF_WSTRING VMF_StrncpyW(VMF_WSTRING destination, VMF_CONST VMF_WSTRING source, VMF_S32 num);
VMF_API VMF_WSTRING VMF_StrcatW(VMF_WSTRING destination, VMF_CONST VMF_WSTRING source);
VMF_API VMF_WSTRING VMF_SafeStrcatW(VMF_WSTRING destination, VMF_S32 size, VMF_CONST VMF_WSTRING source);
VMF_API VMF_WSTRING VMF_StrncatW(VMF_WSTRING destination, VMF_WSTRING source, VMF_S32 num);
VMF_API VMF_S32 VMF_StrcmpW(VMF_CONST VMF_WSTRING str1, VMF_CONST VMF_WSTRING str2);
VMF_API VMF_S32 VMF_StrncmpW(VMF_CONST VMF_WSTRING str1, VMF_CONST VMF_WSTRING str2, VMF_S32 num);
VMF_API VMF_S32 VMF_StrlenW(VMF_CONST VMF_WSTRING str);
VMF_API VMF_S32 VMF_SscanfW(VMF_CONST VMF_WSTRING str, const VMF_WCHAR *format, VMF_VOID **ppOut, VMF_U32 nCount);
VMF_API VMF_S32 VMF_SprintfW(VMF_CONST VMF_WSTRING str,VMF_CONST VMF_WCHAR *format, ...);
VMF_API VMF_S32 VMF_SafeSprintfW(VMF_CONST VMF_WSTRING str,VMF_S32 size,VMF_CONST VMF_WCHAR *format, ...);
VMF_API VMF_S32 VMF_PrintfW(VMF_CONST VMF_WCHAR *format, ...);
VMF_API VMF_WSTRING VMF_StrstrW(VMF_WSTRING str1, VMF_WSTRING str2);

#ifdef UNICODE
#define VMF_StrcpyT   VMF_StrcpyW
#define VMF_StrncpyT  VMF_StrncpyW
#define VMF_StrcatT   VMF_StrcatW
#define VMF_SafeStrcatT VMF_SafeStrcatW
#define VMF_StrncatT   VMF_StrncatW
#define VMF_StrcmpT    VMF_StrcmpW
#define VMF_StrncmpT   VMF_StrncmpW
#define VMF_StrlenT    VMF_StrlenW
#define VMF_SscanfT    VMF_SscanfW
#define VMF_SprintfT   VMF_SprintfW
#define VMF_SafeSprintfT VMF_SafeSprintfW
#define VMF_PrintfT    VMF_PrintfW
#define VMF_StrstrT    VMF_StrstrW
#else
#define VMF_StrcpyT   VMF_Strcpy
#define VMF_StrncpyT  VMF_Strncpy
#define VMF_StrcatT   VMF_Strcat
#define VMF_SafeStrcatT VMF_SafeStrcat
#define VMF_StrncatT   VMF_Strncat
#define VMF_StrcmpT    VMF_Strcmp
#define VMF_StrncmpT   VMF_Strncmp
#define VMF_StrlenT    VMF_Strlen
#define VMF_SscanfT    VMF_Sscanf
#define VMF_SprintfT   VMF_Sprintf
#define VMF_SafeSprintfT VMF_SafeSprintf
#define VMF_PrintfT    VMF_Printf
#define VMF_StrstrT    VMF_Strstr
#endif
//
//  Code Page Default Values.
//
#define VMF_CP_ACP                    0           // default to ANSI code page
#define VMF_CP_OEMCP                  1           // default to OEM  code page
#define VMF_CP_MACCP                  2           // default to MAC  code page
#define VMF_CP_THREAD_ACP             3           // current thread's ANSI code page
#define VMF_CP_SYMBOL                 42          // SYMBOL translations
#define VMF_CP_UTF7                   65000       // UTF-7 translation
#define VMF_CP_UTF8                   65001       // UTF-8 translation
#define VMF_MB_PRECOMPOSED            0x00000001  // use precomposed chars
#define VMF_MB_COMPOSITE              0x00000002  // use composite chars
#define VMF_MB_USEGLYPHCHARS          0x00000004  // use glyph chars, not ctrl chars
#define VMF_MB_ERR_INVALID_CHARS      0x00000008  // error for invalid chars
#define VMF_WC_COMPOSITECHECK         0x00000200  // convert composite to precomposed
#define VMF_WC_DISCARDNS              0x00000010  // discard non-spacing chars
#define VMF_WC_SEPCHARS               0x00000020  // generate separate chars
#define VMF_WC_DEFAULTCHAR            0x00000040  // replace w/ default char
VMF_API VMF_S32 VMF_MultiByteToWideChar(VMF_U32 CodePage, VMF_U32 dwFlags, VMF_STRING lpMultiByteStr, VMF_S32 cbMultiByte, VMF_WSTRING lpWideCharStr,  
						VMF_S32 cchWideChar);
VMF_API VMF_S32 VMF_WideCharToMultiByte(VMF_U32 CodePage, VMF_U32 dwFlags, VMF_WSTRING lpWideCharStr, VMF_S32 cchWideChar, 
						VMF_STRING lpMultiByteStr, VMF_S32 cbMultiByte, VMF_STRING lpDefaultChar, VMF_BOOL *lpUsedDefaultChar);

VMF_API VMF_S32 VMF_WStrToInt(VMF_WSTRING str);
VMF_API VMF_S32 VMF_StrToInt(VMF_STRING str);
#ifdef __cplusplus
}
#endif 

#endif





