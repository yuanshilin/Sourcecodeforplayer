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
**      Name        : VMF_Types.h
**      Purpose     : the definitions for marcos
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_TYPE_H
#define VMF_TYPE_H

#include <VMF_Config.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#ifdef __cplusplus
#define VMF_EXTERN_C    extern "C"
#else
#define VMF_EXTERN_C    extern
#endif

/** The VMF_API and VMF_APIENTRY are platform specific definitions used
 *  to declare OMX function prototypes.  They are modified to meet the
 *  requirements for a particular platform */
#ifdef __SYMBIAN32__   
#   ifdef __VMF_EXPORTS
#       define VMF_API __declspec(dllexport)
#   else
#       ifdef _WIN32
#           define VMF_API __declspec(dllexport) 
#       else
#           define VMF_API __declspec(dllimport)
#       endif
#   endif
#else
#   if defined (WIN32) || defined(WIN64)
#      ifdef __VMF_EXPORTS
#          define VMF_API __declspec(dllexport)
#      else
#          define VMF_API __declspec(dllimport)
#      endif
#   else
#      ifdef __VMF_EXPORTS
#          define VMF_API
#      else
#          define VMF_API extern
#      endif
#   endif
#endif

#ifndef VMF_APIENTRY
#define VMF_APIENTRY 
#endif 

/** VMF_IN is used to identify inputs to an OMX function.  This designation 
    will also be used in the case of a pointer that points to a parameter 
    that is used as an output. */
#ifndef VMF_IN
#define VMF_IN
#endif

/** VMF_OUT is used to identify outputs from an OMX function.  This 
    designation will also be used in the case of a pointer that points 
    to a parameter that is used as an input. */
#ifndef VMF_OUT
#define VMF_OUT
#endif


/** VMF_INOUT is used to identify parameters that may be either inputs or
    outputs from an OMX function at the same time.  This designation will 
    also be used in the case of a pointer that  points to a parameter that 
    is used both as an input and an output. */
#ifndef VMF_INOUT
#define VMF_INOUT
#endif

#if	(!defined(WIN32)) && (!(defined(WIN64)))
#define __stdcall
#endif
#ifndef VMF_CALLBACK
#define VMF_CALLBACK __stdcall
#endif
/** VMF_ALL is used to as a wildcard to select all entities of the same type
 *  when specifying the index, or referring to a object by an index.  (i.e.
 *  use VMF_ALL to indicate all N channels). When used as a port index
 *  for a config or parameter this VMF_ALL denotes that the config or
 *  parameter applies to the entire component not just one port. */
#define VMF_ALL 0xFFFFFFFF

typedef void VMF_VOID;
typedef void* VMF_HANDLE;
typedef int VMF_HRESULT;
typedef double VMF_DOUBLE;
typedef float VMF_FLOAT;
typedef char VMF_CHAR; 
#ifdef NOT_SUPPORT_WCHAR
typedef char VMF_WCHAR;
#else
#include <wchar.h>
typedef wchar_t VMF_WCHAR;
#endif // WIN32

typedef unsigned char VMF_U8;
typedef signed char VMF_S8;
typedef unsigned short VMF_U16;
typedef signed short VMF_S16;
typedef unsigned int VMF_U32;
typedef signed int VMF_S32;
typedef void* VMF_PTR;
#define VMF_STRING VMF_CHAR* 
#define VMF_WSTRING VMF_WCHAR*
typedef unsigned char VMF_BYTE;
typedef int VMF_ERRORTYPE;
#ifdef UNICODE
#define VMF_TCHAR VMF_WCHAR
#define VMF_TSTRING VMF_WSTRING
#else
#define VMF_TCHAR VMF_CHAR
#define VMF_TSTRING VMF_STRING
#endif


#ifdef NOT_SUPPORT_WCHAR
#define LITERAL(x) x
#define LITERALWS  "%s"
#define PRINT_WCHAR "%s"
#define PRINT_LWCHAR() "%s"
#else
#define LITERAL(x) L##x
#define PRINT_WCHAR "%ls"
#define PRINT_LWCHAR() L"%ls"
#endif



/* Users with compilers that cannot accept the "long long" designation should
   define the VMF_SKIP64BIT macro.  It should be noted that this may cause 
   some components to fail to compile if the component was written to require
   64 bit integral types.  However, these components would NOT compile anyway
   since the compiler does not support the way the component was written.
*/
#ifndef VMF_SKIP64BIT
#ifdef __SYMBIAN32__
/** VMF_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned long long VMF_U64;

/** VMF_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed long long VMF_S64;

#elif defined(SUPPORT_WIN32)

/** VMF_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */   
typedef unsigned __int64  VMF_U64;

/** VMF_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed   __int64  VMF_S64;

#else /* WIN32 */

/** VMF_U64 is a 64 bit unsigned quantity that is 64 bit word aligned */
typedef unsigned long long VMF_U64;

/** VMF_S64 is a 64 bit signed quantity that is 64 bit word aligned */
typedef signed long long VMF_S64;

#endif /* WIN32 */
#endif


/** The VMF_BOOL type is intended to be used to represent a true or a false 
    value when passing parameters to and from the OMX core and components.  The
    VMF_BOOL is a 32 bit quantity and is aligned on a 32 bit word boundary.
 */
typedef VMF_S32 VMF_BOOL;
#define VMF_FALSE  0
#define VMF_TRUE   1

#ifndef VMF_NULL
#define VMF_NULL  0
#endif

#ifndef VMF_CONST
#define VMF_CONST const
#endif

#define VMF_FAILED(hr) ((hr) < 0)
#define VMF_SUCCEEDED(hr) ((hr) >= 0)

#ifndef VMF_INFINITE
#define VMF_INFINITE -1
#define VMF_WAIT_TIMEOUT   258L
#endif

#define VMF_MAX_PATH 256
#define VMF_MAX_TEXT 1024

typedef struct VMF_TIME_st
{
	VMF_S32 sec;
	VMF_S32 min;
	VMF_S32 hour;
	VMF_S32 day;
	VMF_S32 mon;
	VMF_S32 year;
	VMF_S32 wday;
	VMF_S32 yday;
	VMF_S32 isdst;
}VMF_TIME;

#define VMF_Safe_Delete(x) {if ((x) != VMF_NULL) delete (x); (x)=VMF_NULL;}
#define VMF_Safe_Array_Delete(x) {if ((x) != VMF_NULL) delete [](x); (x)=VMF_NULL;}
#define VMF_Safe_FreeFile(x) {if ((x) != VMF_NULL) VMF_FreeFile(x); (x) = VMF_NULL;};
#define VMF_Safe_FreeQueue(x) {if ((x) != VMF_NULL) VMF_FreeQueue(x); (x) = VMF_NULL;};

#define VMF_ErrorFalse  1
#define VMF_ErrorNone   0
#define VMF_ErrorPlat   0x80001000
#define VMF_ErrorFailed                (VMF_S32)(VMF_ErrorPlat+1)
#define VMF_ErrorInsufficientResources (VMF_S32)(VMF_ErrorPlat+2)
#define VMF_ErrorUndefined             (VMF_S32)(VMF_ErrorPlat+3)
#define VMF_ErrorBadParameter          (VMF_S32)(VMF_ErrorPlat+4)
#define VMF_ErrorNotImplemented        (VMF_S32)(VMF_ErrorPlat+5)
#define VMF_ErrorTimeout               (VMF_S32)(VMF_ErrorPlat+6) 
#define VMF_ErrorPoint                 (VMF_S32)(VMF_ErrorPlat+7) 
#define VMF_ErrorVersionMismatch       (VMF_S32)(VMF_ErrorPlat+8) 
#define VMF_ErrorNotExist              (VMF_S32)(VMF_ErrorPlat+9) 
#define VMF_ErrorExist                 (VMF_S32)(VMF_ErrorPlat+10) 
#define VMF_ErrorNotPermission         (VMF_S32)(VMF_ErrorPlat+11) 
#define VMF_ErrorNotSupport            (VMF_S32)(VMF_ErrorPlat+12) 
#define VMF_ErrorAllocFailed           (VMF_S32)(VMF_ErrorPlat+13)
#define VMF_ErrorOutOfMemory           (VMF_S32)(VMF_ErrorPlat+14)	
#define VMF_ErrorInvalidOperation      (VMF_S32)(VMF_ErrorPlat+15)	   


#ifdef __cplusplus
}
#endif 

#endif
