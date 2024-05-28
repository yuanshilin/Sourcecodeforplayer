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
**      Name        : VMF_Config.h
**      Purpose     : the configure about PlatForm
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_CONFIG_H
#define VMF_CONFIG_H

//#define UNICODE 
//#define SUPPORT_WIN32
//#define SUPPORT_WIN64
//#define SUPPORT_METRO
//#define SUPPORT_ANDROID
//#define SUPPORT_ANDROID_30
//#define SUPPORT_MAC

//WIN32 in Window is always defined.
//64 bit compiler in window defines _WIN64
#ifdef _WIN64
#ifndef SUPPORT_WIN64
#define SUPPORT_WIN64
#endif
#endif

#ifdef SUPPORT_METRO
#define SUPPORT_WINRT
#ifndef NOT_SUPPORT_UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif
#define WIN32
#endif

#ifdef SUPPORT_WIN32
#define WIN32
#endif

#ifdef SUPPORT_WIN64
#ifndef _M_X64
#define _M_X64
#endif
#ifndef _WIN64
#define _WIN64
#endif
#ifndef WIN64
#define WIN64
#endif
#endif

#ifdef WIN32
#ifndef NOT_SUPPORT_UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif
#endif

#ifdef _DEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif//_DEBUG

#ifdef SUPPORT_ANDROID
#ifndef linux
#define linux
#endif
#endif//SUPPORT_ANDROID

#ifdef SUPPORT_MAC
#define linux
#define SUPPORT_BIT64
#endif//SUPPORT_MAC

#if defined(_LP64)||defined(WIN64)
#define SUPPORT_BIT64
#endif

#ifdef linux
#ifndef NOT_SUPPORT_UNICODE
#ifndef UNICODE
#define UNICODE
#endif
#endif
#endif

#ifdef UNICODE
#ifndef _UNICODE
#define _UNICODE
#endif
#endif//UNICODE


#if defined(SUPPORT_BIT32)
#undef SUPPORT_BIT64
#endif


//#if (defined(SUPPORT_MAC) || defined(linux)) && defined(UNICODE)
//#error "SUPPORT_MAC and linux conflict with UNICODE!"
//#endif

#if (defined(SUPPORT_WIN32)||defined(SUPPORT_WIN64)) && defined(SUPPORT_MAC)
#error "SUPPORT_MAC conflict with WIN32/WIN64!"
#endif

#endif


