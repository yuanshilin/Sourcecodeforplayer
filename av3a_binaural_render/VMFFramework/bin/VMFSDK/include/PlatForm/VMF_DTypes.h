#include <VMF_PlatForm.h>
#if (defined(WIN32) || defined(WIN64))
#include <Windows.h>


#if defined (SUPPORT_METRO) || defined(SUPPORT_WINRT)
#define CreateEvent(lpEventAttributes, bManualReset, bInitialState,lpName)\
	CreateEventExW((lpEventAttributes), (lpName), ((bInitialState)?CREATE_EVENT_INITIAL_SET:0x00)|((bManualReset)?CREATE_EVENT_MANUAL_RESET:0x00), EVENT_ALL_ACCESS)
#define InitializeCriticalSection(x) InitializeCriticalSectionEx((x), 0, 0)
#define CreateSemaphore(lpSemaphoreAttributes, lInitialCount,lMaximumCount, lpName)\
	CreateSemaphoreEx((lpSemaphoreAttributes), (lInitialCount),(lMaximumCount), (lpName), 0, SEMAPHORE_ALL_ACCESS)
#define WaitForSingleObject(hHandle, dwMilliseconds) WaitForSingleObjectEx((hHandle),(dwMilliseconds), FALSE)
#define WaitForMultipleObjects(nCount,pHandles,bWaitAll, dwMilliseconds)\
	WaitForMultipleObjectsEx((nCount),(pHandles),(bWaitAll),(dwMilliseconds), FALSE)

#define GetThreadPriority VMF_GetThreadPriority
#define SetThreadPriority VMF_SetThreadPriority
#define GetCurrentSysThreadId VMF_GetCurrentSysThreadId
#define GetSysThreadPriority VMF_GetSysThreadPriority
#define SetSysThreadPriority VMF_SetSysThreadPriority
#define GetSysCurrentThread VMF_GetCurrentSysThread
#define CreateThread VMF_CreateThread
#define LPTHREAD_START_ROUTINE VMF_VOID*

#define GetTickCount() GetTickCount64()
#define Sleep(x) VMF_Sleep(x)
#define MulDiv   VMF_MulDiv
#define lstrcmpW(src1, src2)                   VMF_StrcmpW((src1), (src2))
#define lstrcmpA(src1, src2)                   VMF_Strcmp((src1), (src2))
#define lstrlenW(src)                          VMF_StrlenW(src)
#define lstrlenA(src)                          VMF_Strlen(src)

#ifndef _VMF_DEFINE_MSG_
#define _VMF_DEFINE_MSG_
#define MSG VMF_MSG
#define PeekMessage VMF_PeekMessage
#define PostSysThreadMessage VMF_PostSysThreadMessage
#define PostThreadMessage VMF_PostSysThreadMessage
#define DispatchMessage VMF_DispatchMessage
#define SendMessage VMF_SendMessage
#define GetQueueStatus VMF_GetQueueStatus
#define RegisterWindowMessage VMF_RegisterWindowMessage
#define MsgWaitForMultipleObjects(nCount, pHandles, bWaitAll,dwMilliseconds,dwWakeMask)\
	 WaitForMultipleObjects((nCount),(pHandles),(bWaitAll), (dwMilliseconds))
#endif

#ifndef _VMF_DEFINE_TIMER_
#define _VMF_DEFINE_TIMER_
#define timeSetEvent VMF_TimeSetEvent
#define timeKillEvent VMF_TimeKillEvent
#define TIME_ONESHOT VMF_TIME_ONESHOT 
#define TIME_PERIODIC VMF_TIME_PERIODIC 
#define TIME_CALLBACK_FUNCTION VMF_TIME_CALLBACK_FUNCTION 
#define TIME_CALLBACK_EVENT_SET VMF_TIME_CALLBACK_EVENT_SET  
#define TIME_CALLBACK_EVENT_PULSE VMF_TIME_CALLBACK_EVENT_PULSE 
#define TIME_KILL_SYNCHRONOUS VMF_TIME_KILL_SYNCHRONOUS 
#endif


#ifndef _VMF_DEFINE_WAVE_FORMAT_PCM_
#define _VMF_DEFINE_WAVE_FORMAT_PCM_
typedef struct waveformat_tag {
	WORD    wFormatTag;        /* format type */
	WORD    nChannels;         /* number of channels (i.e. mono, stereo, etc.) */
	DWORD   nSamplesPerSec;    /* sample rate */
	DWORD   nAvgBytesPerSec;   /* for buffer estimation */
	WORD    nBlockAlign;       /* block size of data */
} WAVEFORMAT, *PWAVEFORMAT, NEAR *NPWAVEFORMAT, FAR *LPWAVEFORMAT;
/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM     1
/* specific waveform format structure for PCM data */
typedef struct pcmwaveformat_tag {
	WAVEFORMAT  wf;
	WORD        wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, NEAR *NPPCMWAVEFORMAT, FAR *LPPCMWAVEFORMAT;
#endif /* WAVE_FORMAT_PCM */

inline static DWORD timeGetTime()
{
	return ((DWORD)(VMF_GetTime()/1000));
}

#ifndef _VMF_DEFINE_FOURCC_
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1)<<8) | ((DWORD)(BYTE)(ch2)<<16) | ((DWORD)(BYTE)(ch3)<<24))
#define mmioFOURCC(ch0, ch1, ch2, ch3) MAKEFOURCC(ch0, ch1, ch2, ch3)
#define FCC(ch4) ((((DWORD)(ch4)&0xFF)<<24)|(((DWORD)(ch4)&0xFF00)<<8)|(((DWORD)(ch4)&0xFF0000)>>8)|(((DWORD)(ch4)&0xFF000000)>>24))
#endif

#else
#define GetCurrentSysThreadId GetCurrentThreadId
#define GetSysThreadPriority GetThreadPriority
#define SetSysThreadPriority SetThreadPriority
#define GetSysCurrentThread  GetCurrentThread
#endif

#define __VMF_TYPES_H__
#define _VMF_WIN_OBJBASE_
#define __VMF_DEFINE_OLEGUID__

#ifndef _VMF_DEFINE_DEREF_
#define _VMF_DEFINE_DEREF_
#ifndef __range
#define __range(x,y)
#endif
#ifndef __field_ecount_opt
#define __field_ecount_opt(x)
#endif
#ifndef __allocator
#define __allocator
#endif
#endif

#define _T(quote)   TEXT(quote)

#define StringCchCopyW(dst, size, src)         VMF_StrncpyW((dst), (src), (size))
#define StringCchCopyA(dst, size, src)         VMF_Strncpy((dst), (src), (size))
#ifdef UNICODE
#define StringCchCopy StringCchCopyW
#else
#define StringCchCopy StringCchCopyA
#endif

#ifndef _STRSAFE_H_INCLUDED_
#ifndef _VMF_DEFINE_STRINGCCHPRINTF_
#define _VMF_DEFINE_STRINGCCHPRINTF_
#ifdef UNICODE
#define StringCchPrintf VMF_SafeSprintfW
#define StringCchCat VMF_SafeStrcatW
#else
#define StringCchPrintf VMF_SafeSprintf
#define StringCchCat VMF_SafeStrcat
#endif
#endif
#endif
#elif defined(SUPPORT_MAC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _VMF_DEFINE_SIZE_TT_
#define _DEFINE_MEMSET_
#define _VMF_DEFINE_NULL_
#elif defined(linux)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _VMF_DEFINE_SIZE_TT_
#define _DEFINE_MEMSET_
#endif//WIN32

#ifndef __VMF_TYPES_H__
#define __VMF_TYPES_H__

#ifndef _VMF_EXTERN_C_
#define _VMF_EXTERN_C_
#define EXTERN_C VMF_EXTERN_C
#endif
#ifndef _VMF_DEFINE_far_
#define _VMF_DEFINE_far_
#define far
#endif

#ifndef _VMF_DEFINE_near_
#define _VMF_DEFINE_near_
#define near
#endif

#ifndef _VMF_DEFINE_FAR_
#define _VMF_DEFINE_FAR_
#ifndef FAR
#define FAR                 far
#endif
#endif
#ifndef _VMF_DEFINE_NEAR_
#define _VMF_DEFINE_NEAR_
#define NEAR                near
#endif

#ifndef _VMF_DEFINE_CONST_
#define _VMF_DEFINE_CONST_
#define CONST               VMF_CONST
#endif

#ifndef _VMF_DEFINE_PVOID_
#define _VMF_DEFINE_PVOID_
#define VOID void        
typedef VMF_VOID            *PVOID;
#endif

#ifndef _VMF_DEFINE_DWORD_
#define _VMF_DEFINE_DWORD_
typedef VMF_U32             DWORD;
#endif

#ifndef _VMF_DEFINE_BOOL_
#define _VMF_DEFINE_BOOL_
#ifdef SUPPORT_MAC
typedef VMF_S8             BOOL; // conflict in xcode
#else
typedef VMF_S32             BOOL;
#endif
#endif

#ifndef _VMF_DEFINE_BOOLEAN_
#define _VMF_DEFINE_BOOLEAN_
#ifdef SUPPORT_MAC
typedef VMF_S8             BOOLEAN; 
#else
typedef VMF_S32             BOOLEAN;
#endif
#endif

#ifndef _VMF_DEFINE_BYTE_
#define _VMF_DEFINE_BYTE_
#ifndef BYTE
typedef VMF_U8              BYTE;
#endif
#endif

#ifndef _VMF_DEFINE_UCHAR_
#define _VMF_DEFINE_UCHAR_
typedef VMF_U8              UCHAR;
#endif

#ifndef _VMF_DEFINE_USHORT_
#define _VMF_DEFINE_USHORT_
typedef VMF_U16              USHORT;
#endif

#ifndef _VMF_DEFINE_WORD_
#define _VMF_DEFINE_WORD_
typedef VMF_U16             WORD;
#endif

#ifndef _VMF_DEFINE_SHORT_
#define _VMF_DEFINE_SHORT_
typedef VMF_S16             SHORT;
#endif

#ifndef _VMF_DEFINE_FLOAT_
#define _VMF_DEFINE_FLOAT_
typedef VMF_FLOAT           FLOAT;
#endif

#ifndef _VMF_DEFINE_PFLOAT_
#define _VMF_DEFINE_PFLOAT_
typedef FLOAT               *PFLOAT;
#endif

#ifndef _VMF_DEFINE_DOUBLE_
#define _VMF_DEFINE_DOUBLE_
typedef VMF_DOUBLE           DOUBLE;
#endif

#ifndef _VMF_DEFINE_PDOUBLE_
#define _VMF_DEFINE_PDOUBLE_
typedef DOUBLE               *PDOUBLE;
#endif

#ifndef _VMF_DEFINE_PBOOL_
#define _VMF_DEFINE_PBOOL_
typedef BOOL            *PBOOL;
#endif


#ifndef _VMF_DEFINE_LPBOOL_
#define _VMF_DEFINE_LPBOOL_
typedef BOOL             *LPBOOL;
#endif

#ifndef _VMF_DEFINE_PBYTE_
#define _VMF_DEFINE_PBYTE_
typedef BYTE            *PBYTE;
#endif

#ifndef _VMF_DEFINE_LPBYTE_
#define _VMF_DEFINE_LPBYTE_
typedef BYTE             *LPBYTE;
#endif

#ifndef _VMF_DEFINE_CHAR_
#define _VMF_DEFINE_CHAR_
typedef char             CHAR;
#endif

#ifndef _VMF_DEFINE_PINT_
#define _VMF_DEFINE_PINT_
typedef VMF_S32         *PINT;
#endif

#ifndef _VMF_DEFINE_LPINT_
#define _VMF_DEFINE_LPINT_
typedef VMF_S32          *LPINT;
#endif

#ifndef _VMF_DEFINE_PWORD_
#define _VMF_DEFINE_PWORD_
typedef WORD            *PWORD;
#endif

#ifndef _VMF_DEFINE_LPWORD_
#define _VMF_DEFINE_LPWORD_
typedef WORD far            *LPWORD;
#endif

#ifndef _VMF_DEFINE_LPLONG_
#define _VMF_DEFINE_LPLONG_
typedef VMF_S32 far         *LPLONG;
#endif

#ifndef _VMF_DEFINE_LONG_
#define _VMF_DEFINE_LONG_
typedef VMF_S32          LONG;
#endif

#ifndef _VMF_DEFINE_ULONG_
#define _VMF_DEFINE_ULONG_
typedef VMF_U32          ULONG;
typedef VMF_U32          *PULONG;
#endif

#ifndef _VMF_DEFINE_PDWORD_
#define _VMF_D FINE_PDWORD_
typedef DWORD near          *PDWORD;
#endif

#ifndef _VMF_DEFINE_LPDWORD_
#define _VMF_DEFINE_LPDWORD_
typedef DWORD far           *LPDWORD;
#endif

#ifndef _VMF_DEFINE_LPVOID_
#define _VMF_DEFINE_LPVOID_
typedef VMF_VOID far        *LPVOID;
#endif

#ifndef _VMF_DEFINE_LPCVOID_
#define _VMF_DEFINE_LPCVOID_
typedef CONST VMF_VOID far  *LPCVOID;
#endif

#ifndef _VMF_DEFINE_INT_
#define _VMF_DEFINE_INT_
typedef VMF_S32        INT;
#endif

#ifndef _VMF_DEFINE_UINT_
#define _VMF_DEFINE_UINT_
typedef VMF_U32        UINT;
#endif

#ifndef _VMF_DEFINE_INT32
#define _VMF_DEFINE_INT32
typedef VMF_S32		   INT32;
#endif

#ifndef _VMF_DEFINE_UINT32
#define _VMF_DEFINE_UINT32
#ifndef UINT32
typedef VMF_U32		   UINT32;
#endif
#endif

#ifndef _VMF_DEFINE_UINT8
#define _VMF_DEFINE_UINT8
typedef VMF_U8 			UINT8;
#endif

#ifndef _VMF_DEFINE_PUINT_
#define _VMF_DEFINE_PUINT_
typedef VMF_U32        *PUINT;
#endif

#ifndef _VMF_DEFINE_GUID_
#define _VMF_DEFINE_GUID_
typedef VMF_GUID       GUID;
#endif

#ifndef _VMF_DEFINE_HRESULT_
#define _VMF_DEFINE_HRESULT_
typedef VMF_S32 HRESULT;
#endif

#ifndef _VMF_DEFINE_MMRESULT_
#define _VMF_DEFINE_MMRESULT_
typedef VMF_U32 MMRESULT;
#endif

#ifndef _VMF_DEFINE_ULONG_PTR_
#define _VMF_DEFINE_ULONG_PTR_
#ifdef SUPPORT_BIT64
typedef VMF_U64 ULONG_PTR,DWORD_PTR,*PULONG_PTR;
#else
typedef VMF_U32 ULONG_PTR,DWORD_PTR,*PULONG_PTR;
#endif
#endif

#ifndef _VMF_DEFINE_LONG_PTR_
#define _VMF_DEFINE_LONG_PTR_
#ifdef SUPPORT_BIT64
typedef VMF_S64 LONG_PTR, *PLONG_PTR;
#else
 typedef VMF_S32 LONG_PTR, *PLONG_PTR;
 #endif
#endif


#ifndef _VMF_DEFINE_WCHAR_
#define _VMF_DEFINE_WCHAR_
typedef VMF_WCHAR WCHAR;
#endif

#ifndef _VMF_DEFINE_TCHAR_
#define _VMF_DEFINE_TCHAR_
#ifdef UNICODE
#define TCHAR WCHAR
typedef  WCHAR* PTCHAR;
#else
#define TCHAR CHAR
typedef  CHAR* PTCHAR;
#endif
typedef TCHAR *PCTSTR;
typedef VMF_CONST TCHAR *LPCTSTR;
typedef TCHAR *PTSTR, *LPTSTR;
#endif

#ifndef _VMF_DEFINE_LPWSTR_
#define _VMF_DEFINE_LPWSTR_
typedef WCHAR *LPWSTR, *BSTR, *PWSTR;
#endif

#ifndef _VMF_DEFINE_LPCWSTR_
#define _VMF_DEFINE_LPCWSTR_
typedef VMF_CONST WCHAR *LPCWSTR,*PCWSTR;
#endif

#ifndef _VMF_DEFINE_LONGLONG_
#define _VMF_DEFINE_LONGLONG_
typedef VMF_S64 LONGLONG;
typedef VMF_U64 ULONGLONG;
#endif

#ifndef _VMF_DEFINE_HANDLE_
#define _VMF_DEFINE_HANDLE_
typedef VMF_VOID *HANDLE;
#endif

#ifndef _VMF_DEFINE_COLORREF_
#define _VMF_DEFINE_COLORREF_
typedef DWORD   COLORREF;
typedef DWORD   *LPCOLORREF;
#endif

#ifndef _VMF_DEFINE_RECT_
#define _VMF_DEFINE_RECT_
typedef struct tagRECT
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
} RECT, *PRECT, NEAR *NPRECT, FAR *LPRECT;
typedef const RECT FAR* LPCRECT;
typedef struct _RECTL       /* rcl */
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
} RECTL, *PRECTL, *LPRECTL;
typedef const RECTL FAR* LPCRECTL;
#endif

#ifndef _VMF_DEFINE_POINT_
#define _VMF_DEFINE_POINT_
typedef struct tagPOINT
{
	LONG  x;
	LONG  y;
} POINT, *PPOINT, NEAR *NPPOINT, FAR *LPPOINT;

typedef struct _POINTL      /* ptl  */
{
	LONG  x;
	LONG  y;
} POINTL, *PPOINTL;
#endif

#ifndef _VMF_DEFINE_SIZE_
#define _VMF_DEFINE_SIZE_
typedef struct tagSIZE
{
	LONG        cx;
	LONG        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE               *PSIZEL, *LPSIZEL;
#endif

#ifndef _VMF_DEFINE_POINTS_
#define _VMF_DEFINE_POINTS_
typedef struct tagPOINTS
{
#ifndef _MAC
	SHORT   x;
	SHORT   y;
#else
	SHORT   y;
	SHORT   x;
#endif
} POINTS, *PPOINTS, *LPPOINTS;
#endif

#ifndef _VMF_DEFINE_PALETTEENTRY_
#define _VMF_DEFINE_PALETTEENTRY_
typedef struct tagPALETTEENTRY {
	BYTE        peRed;
	BYTE        peGreen;
	BYTE        peBlue;
	BYTE        peFlags;
} PALETTEENTRY, *PPALETTEENTRY, FAR *LPPALETTEENTRY;
#endif // !_PALETTEENTRY_DEFINED

#ifndef _VMF_DEFINE_LOGPALETTE_
#define _VMF_DEFINE_LOGPALETTE_
/* Logical Palette */
typedef struct tagLOGPALETTE {
	WORD        palVersion;
	WORD        palNumEntries;
	PALETTEENTRY        palPalEntry[1];
} LOGPALETTE, *PLOGPALETTE, NEAR *NPLOGPALETTE, FAR *LPLOGPALETTE;
#endif // !_LOGPALETTE_DEFINED


#ifndef _VMF_DEFINE_OLECHAR_
#define _VMF_DEFINE_OLECHAR_
typedef WCHAR OLECHAR;
typedef OLECHAR *LPOLESTR;
typedef VMF_CONST OLECHAR *LPCOLESTR;
#endif

#ifndef _VMF_DEFINE_DWORDLONG_
#define _VMF_DEFINE_DWORDLONG_
typedef VMF_U64 DWORDLONG;
typedef DWORDLONG *PDWORDLONG;
#endif
#ifndef _VMF_DEFINE_LCID_
#define _VMF_DEFINE_LCID_
typedef DWORD LCID;         
typedef PDWORD PLCID;       
typedef WORD   LANGID; 
#endif

#ifndef _VMF_DEFINE_LPSTR_
#define _VMF_DEFINE_LPSTR_
typedef CHAR *NPSTR, *LPSTR, *PSTR;
typedef VMF_CONST CHAR *LPCSTR, *PCSTR;
#endif

#ifndef _VMF_DEFINE_SIZE_T_
#define _VMF_DEFINE_SIZE_T_
typedef ULONG_PTR SIZE_T, *PSIZE_T;
typedef LONG_PTR SSIZE_T, *PSSIZE_T;
#endif

#ifndef _VMF_DEFINE_Int32x32To64_
#define _VMF_DEFINE_Int32x32To64_
#define Int32x32To64( a, b ) (LONGLONG)((LONGLONG)(LONG)(a) * (LONG)(b))
#define UInt32x32To64( a, b ) (ULONGLONG)((ULONGLONG)(DWORD)(a) * (DWORD)(b))
#endif

#ifndef _VMF_DEFINE_HKEY_
#define _VMF_DEFINE_HKEY_
typedef VMF_VOID *HKEY;
#endif

#ifndef _VMF_DEFINE_HINSTANCE_
#define _VMF_DEFINE_HINSTANCE_
typedef VMF_VOID *HINSTANCE;
#endif

#ifndef _VMF_DEFINE_CALLBACK_
#define _VMF_DEFINE_CALLBACK_
#ifndef STDMETHODCALLTYPE
#ifdef _68K_
#define STDMETHODCALLTYPE       __cdecl
#else
#define STDMETHODCALLTYPE       __stdcall
#endif
#endif
#define STDMETHODVCALLTYPE      __cdecl

#define STDAPICALLTYPE          __stdcall
#define STDAPIVCALLTYPE         __cdecl

#ifdef _MAC
#define CALLBACK    PASCAL
#define WINAPI      CDECL
#define WINAPIV     CDECL
#define APIENTRY    WINAPI
#define APIPRIVATE  CDECL
#ifdef _68K_
#define PASCAL      __pascal
#else
#define PASCAL
#endif
#elif (_MSC_VER >= 800) || defined(_STDCALL_SUPPORTED)
#define CALLBACK    __stdcall
#define WINAPI      __stdcall
#define WINAPIV     __cdecl
#define APIENTRY    WINAPI
#define APIPRIVATE  __stdcall
#define PASCAL      __stdcall
#else
#define CALLBACK
#define WINAPI
#define WINAPIV
#define APIENTRY    WINAPI
#define APIPRIVATE
#define PASCAL      pascal
#endif
#define STDAPI                  EXTERN_C HRESULT STDAPICALLTYPE
#define STDAPI_(type)           EXTERN_C type STDAPICALLTYPE

#define STDMETHODIMP            HRESULT STDMETHODCALLTYPE
#define STDMETHODIMP_(type)     type STDMETHODCALLTYPE

// The 'V' versions allow Variable Argument lists.

#define STDAPIV                 EXTERN_C HRESULT STDAPIVCALLTYPE
#define STDAPIV_(type)          EXTERN_C type STDAPIVCALLTYPE

#define STDMETHODIMPV           HRESULT STDMETHODVCALLTYPE
#define STDMETHODIMPV_(type)    type STDMETHODVCALLTYPE

#if (defined(WIN32) || defined(WIN64))
#define DllExport __declspec(dllexport) 
#else
#define DllExport
#endif

#endif//_VMF_DEFINE_CALLBACK_

#ifndef _VMF_DEFINE_DEREF_
#define _VMF_DEFINE_DEREF_
#define __format_string
#define __inout_opt
#define __deref_out
#define __deref_in
#define __deref_inout_opt
#define __in_opt
#define __in
#define __out
#define __inout
#define __out_opt
#define __in_bcount(x)
#define __in_ecount(x)
#define __out_ecount(x)
#define __deref_out_opt
#define __out_ecount_part(x,y)
#define __out_bcount_part(x,y)
#define __range(x,y)
#define __field_ecount_opt(x)
#define __in_bcount_opt(x)
#define __out_bcount(x)
#define __bcount_opt(x)
#define __allocator
#define __reserved
#define IN
#define __control_entrypoint(x)
#define __RPC__inout
#define __RPC__in
#define __RPC__out
#define __RPC__in_opt
#define __RPC__in_ecount_full(x)
#define __RPC__out_ecount_full(x)
#define __RPC__inout_ecount_full(x)
#define UNALIGNED
#endif

#ifndef _VMF_DEFINE_CRITICAL_SECTION_
#define _VMF_DEFINE_CRITICAL_SECTION_
typedef VMF_Mutex CRITICAL_SECTION;
#define InitializeCriticalSection(x) VMF_InitMutex(x)
#define DeleteCriticalSection(x) VMF_DeleteMutex(x)
#define EnterCriticalSection(x) VMF_EnterMutex(x)
#define LeaveCriticalSection(x) VMF_LeaveMutex(x)
#endif

#ifndef _VMF_DEFINE_FALSE_
#define _VMF_DEFINE_FALSE_
#ifndef FALSE
#define FALSE    VMF_FALSE
#endif
#ifndef TRUE
#define TRUE     VMF_TRUE
#endif
#endif

#ifndef _VMF_DEFINE_NULL_
#define _VMF_DEFINE_NULL_
#ifndef NULL
#define NULL      0
#endif
#endif

#ifndef INFINITE
#ifndef INFINITE
#define INFINITE VMF_INFINITE 
#endif
#endif

#ifndef _VMF_DEFINE_WAIT_OBJECT_0_
#define _VMF_DEFINE_WAIT_OBJECT_0_
#ifndef WAIT_OBJECT_0
#define WAIT_OBJECT_0       0
#endif
#endif

#ifndef _VMF_DEFINE_HWND_
#define _VMF_DEFINE_HWND_
typedef VMF_VOID *HWND;
#endif

#ifndef _VMF_DEFINE_BITMAPINFOHEADER_
#define _VMF_DEFINE_BITMAPINFOHEADER_
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L
#define BI_JPEG       4L
#define BI_PNG        5L
typedef struct tagBITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} BITMAPINFOHEADER, FAR *LPBITMAPINFOHEADER, *PBITMAPINFOHEADER;
typedef struct tagRGBQUAD {
	BYTE    rgbBlue;
	BYTE    rgbGreen;
	BYTE    rgbRed;
	BYTE    rgbReserved;
} RGBQUAD;
typedef RGBQUAD FAR* LPRGBQUAD;
typedef struct tagBITMAPINFO {
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[1];
} BITMAPINFO, FAR *LPBITMAPINFO, *PBITMAPINFO;
#endif

#ifndef _VMF_DEFINE_TEXT_
#define _VMF_DEFINE_TEXT_
#ifdef UNICODE
#ifdef SUPPORT_MAC
#define __TEXT(quote) quote
#else
#define __TEXT(quote) L##quote
#endif
#else
#define __TEXT(quote) quote
#endif
#define TEXT(quote) __TEXT(quote)   // r_winnt
#define _T(quote)   __TEXT(quote)
#endif

#ifndef _VMF_DEFINE_DEFINE_GUID_
#define _VMF_DEFINE_DEFINE_GUID_
#define DEFINE_GUID VMF_DEFINE_GUID
#define EXTERN_GUID VMF_EXTERN_GUID
#endif

#ifndef _VMF_DEFINE_VARIANT_
#define _VMF_DEFINE_VARIANT_
typedef VOID VARIANT;
#endif

#ifndef _VMF_DEFINE_WPARAM_
#define _VMF_DEFINE_WPARAM_
typedef LONG_PTR WPARAM;
typedef LONG_PTR LPARAM;
#endif

#define UNREFERENCED_PARAMETER(P)

#define StringCchCopyW(dst, size, src)         VMF_StrncpyW((dst), (src), (size))
#define StringCchCopyA(dst, size, src)         VMF_Strncpy((dst), (src), (size))
#ifdef UNICODE
#define StringCchCopy StringCchCopyW
#else
#define StringCchCopy StringCchCopyA
#endif
#define lstrcmpW(src1, src2)                   VMF_StrcmpW(src1, src2)
#define lstrlenW(src)                          VMF_StrlenW(src)

#ifndef _VMF_DEFINE_ZEROMEMORY_
#define _VMF_DEFINE_ZEROMEMORY_
#define ZeroMemory(dst, size)         VMF_Memset((dst), 0, (size))
#endif

#ifndef min
#define min(x, y)         ((x)>(y)?(y):(x))
#endif
#ifndef max
#define max(x, y)         ((x)>(y)?(x):(y))
#endif
#ifndef abs
#define abs(x)            ((x)>=0?(x):(-(x)))
#endif

#ifndef _VMF_DEFINE_COPYMEMORY_
#define _VMF_DEFINE_COPYMEMORY_
#define CopyMemory(dst, src, size)         VMF_Memcpy((dst), (src), (size))
#endif

#ifndef _VMF_DEFINE_MOVEMEMORY_
#define _VMF_DEFINE_MOVEMEMORY_
#define MoveMemory(dst, src, size)         VMF_Memmove((dst), (src), (size))
#endif

#ifndef _VMF_DEFINE_FIELD_OFFSET_
#define _VMF_DEFINE_FIELD_OFFSET_
#define FIELD_OFFSET(type, field)    ((LONG)(LONG_PTR)&(((type *)0)->field))
#endif

#ifndef _VMF_DEFINE_THREAD_
#define _VMF_DEFINE_THREAD_
#define CloseHandle(x)    VMF_CloseHandle(x)
#define CreateEvent(lpEventAttributes, bManualReset, bInitialState,lpName)\
		VMF_CreateEvent((lpEventAttributes), (bManualReset), (bInitialState),(lpName))
#define SetEvent(x) VMF_SetEvent(x)
#define ResetEvent(x) VMF_ResetEvent(x)
#define PulseEvent(x)  VMF_PulseEvent((VMF_Event)(x))
#define CreateSemaphore(lpSemaphoreAttributes,lInitialCount,lMaximumCount,lpName)\
		VMF_CreateSemaphore((lpSemaphoreAttributes),(lInitialCount),(lMaximumCount),(lpName))
#define ReleaseSemaphore(hSem, lReleaseCount, lpPreviousCount)\
		VMF_ReleaseSemaphore((hSem), (lReleaseCount), (lpPreviousCount))
#define WaitForSingleObject(hHandle, dwMilliseconds)\
		VMF_WaitForSingleObject((hHandle), (dwMilliseconds))
#define WaitForMultipleObjects(nCount, hHandles, bWaitAll, dwMilliseconds)\
	VMF_WaitForMultipleObjects((nCount), (hHandles), (bWaitAll), (dwMilliseconds))
#define GetThreadPriority VMF_GetThreadPriority
#define SetThreadPriority VMF_SetThreadPriority
#define GetCurrentSysThreadId VMF_GetCurrentSysThreadId
#define GetSysThreadPriority VMF_GetSysThreadPriority
#define SetSysThreadPriority VMF_SetSysThreadPriority
#define GetSysCurrentThread VMF_GetCurrentSysThread
#define CreateThread VMF_CreateThread
#define LPTHREAD_START_ROUTINE VMF_VOID*
#define Sleep(x) VMF_Sleep(x)
#endif

#ifndef _VMF_DEFINE_MAXVALUE_
#define _VMF_DEFINE_MAXVALUE_
#ifndef SUPPORT_WIN32
#define I64 
#define i64 
#endif
#define MINCHAR     0x80        
#define MAXCHAR     0x7f        
#define MINSHORT    0x8000      
#define MAXSHORT    0x7fff      
#define MINLONG     0x80000000  
#define MAXLONG     0x7fffffff  
#define MAXBYTE     0xff        
#define MAXWORD     0xffff      
#define MAXDWORD    0xffffffff 
#ifndef SHRT_MIN
#define SHRT_MIN    (-32768)        /* minimum (signed) short value */
#endif
#ifndef SHRT_MAX
#define SHRT_MAX      32767         /* maximum (signed) short value */
#endif
#ifndef USHRT_MAX
#define USHRT_MAX     0xffff        /* maximum unsigned short value */
#endif
#ifndef INT_MIN
#define INT_MIN     (-2147483647 - 1) /* minimum (signed) int value */
#endif
#ifndef INT_MAX
#define INT_MAX       2147483647    /* maximum (signed) int value */
#endif
#ifndef UINT_MAX
#define UINT_MAX      0xffffffff    /* maximum unsigned int value */
#endif
#ifndef LONG_MIN
#define LONG_MIN    (-2147483647L - 1) /* minimum (signed) long value */
#endif
#ifndef LONG_MAX
#define LONG_MAX      2147483647L   /* maximum (signed) long value */
#endif
#ifndef ULONG_MAX
#define ULONG_MAX     0xffffffffUL  /* maximum unsigned long value */
#endif
#ifdef WIN32
#ifndef LLONG_MAX
#define LLONG_MAX     9223372036854775807i64       /* maximum signed long long int value */
#endif
#ifndef LLONG_MIN
#define LLONG_MIN   (-9223372036854775807i64 - 1)  /* minimum signed long long int value */
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX    0xffffffffffffffffui64       /* maximum unsigned long long int value */
#endif
/* minimum signed 64 bit value */
#define _I64_MIN    (-9223372036854775807i64 - 1)
/* maximum signed 64 bit value */
#define _I64_MAX      9223372036854775807i64
/* maximum unsigned 64 bit value */
#define _UI64_MAX     0xffffffffffffffffui64
#else
#ifndef LLONG_MAX
#define LLONG_MAX     9223372036854775807LL     /* maximum signed long long int value */
#endif
#ifndef LLONG_MIN
#define LLONG_MIN   (-9223372036854775807LL - 1)  /* minimum signed long long int value */
#endif
#ifndef ULLONG_MAX
#define ULLONG_MAX    0xffffffffffffffffLL      /* maximum unsigned long long int value */
#endif
/* minimum signed 64 bit value */
#define _I64_MIN    (-9223372036854775807LL - 1)
/* maximum signed 64 bit value */
#define _I64_MAX      9223372036854775807LL
/* maximum unsigned 64 bit value */
#define _UI64_MAX     0xffffffffffffffffLL
#endif
#endif

// #ifndef _VMF_DEFINE_SIZE_TT_
// #define _VMF_DEFINE_SIZE_TT_
// #ifndef WIN32
// #define size_t VMF_S32
// #endif
// #endif

#ifndef _VMF_DEFINE_HACCEL_
#define _VMF_DEFINE_HACCEL_
typedef VMF_VOID *HACCEL;
#endif

#ifndef _VMF_DEFINE_LARGE_INTEGER_
#define _VMF_DEFINE_LARGE_INTEGER_
typedef union _LARGE_INTEGER {
	struct {
		DWORD LowPart;
		LONG HighPart;
	};
	struct {
		DWORD LowPart;
		LONG HighPart;
	} u;
	LONGLONG QuadPart;
} LARGE_INTEGER;
typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER {
	struct {
		DWORD LowPart;
		DWORD HighPart;
	};
	struct {
		DWORD LowPart;
		DWORD HighPart;
	} u;
	ULONGLONG QuadPart;
} ULARGE_INTEGER;
typedef ULARGE_INTEGER *PULARGE_INTEGER;
#endif

#ifndef _VMF_DEFINE_INVALID_HANDLE_VALUE_
#define _VMF_DEFINE_INVALID_HANDLE_VALUE_
#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)
#endif

#ifndef _VMF_DEFINE_WAVE_FORMAT_PCM_
#define _VMF_DEFINE_WAVE_FORMAT_PCM_
typedef struct waveformat_tag {
	WORD    wFormatTag;        /* format type */
	WORD    nChannels;         /* number of channels (i.e. mono, stereo, etc.) */
	DWORD   nSamplesPerSec;    /* sample rate */
	DWORD   nAvgBytesPerSec;   /* for buffer estimation */
	WORD    nBlockAlign;       /* block size of data */
} WAVEFORMAT, *PWAVEFORMAT, NEAR *NPWAVEFORMAT, FAR *LPWAVEFORMAT;
/* flags for wFormatTag field of WAVEFORMAT */
#define WAVE_FORMAT_PCM     1
/* specific waveform format structure for PCM data */
typedef struct pcmwaveformat_tag {
	WAVEFORMAT  wf;
	WORD        wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, NEAR *NPPCMWAVEFORMAT, FAR *LPPCMWAVEFORMAT;
#endif /* WAVE_FORMAT_PCM */

#ifndef _VMF_DEFINE_TIMEGETTIME_
#define _VMF_DEFINE_TIMEGETTIME_
inline DWORD static timeGetTime()
{
	return ((DWORD)(VMF_GetTime()/1000));
}
#endif

#ifndef _VMF_DEFINE_STRINGCCHPRINTF_
#define _VMF_DEFINE_STRINGCCHPRINTF_
#ifdef UNICODE
#define StringCchPrintf VMF_SafeSprintfW
#define StringCchCat VMF_SafeStrcatW
#else
#define StringCchPrintf VMF_SafeSprintf
#define StringCchCat VMF_SafeStrcat
#endif
#endif

#ifndef _VMF_DEFINE_STRINGCONVERT_
#define _VMF_DEFINE_STRINGCONVERT_
#define MultiByteToWideChar VMF_MultiByteToWideChar
#define WideCharToMultiByte VMF_WideCharToMultiByte
#define CP_ACP VMF_CP_ACP
#define CP_OEMCP VMF_CP_OEMCP                  
#define CP_MACCP VMF_CP_MACCP                 
#define CP_THREAD_ACP VMF_CP_THREAD_ACP            
#define CP_SYMBOL VMF_CP_SYMBOL               
#define CP_UTF7 VMF_CP_UTF7                  
#define CP_UTF8 VMF_CP_UTF8                  
#define MB_PRECOMPOSED VMF_MB_PRECOMPOSED            
#define MB_COMPOSITE VMF_MB_COMPOSITE              
#define MB_USEGLYPHCHARS VMF_MB_USEGLYPHCHARS          
#define MB_ERR_INVALID_CHARS VMF_MB_ERR_INVALID_CHARS     
#define WC_COMPOSITECHECK VMF_WC_COMPOSITECHECK        
#define WC_DISCARDNS VMF_WC_DISCARDNS              
#define WC_SEPCHARS VMF_WC_SEPCHARS               
#define WC_DEFAULTCHAR VMF_WC_DEFAULTCHAR            
#endif

#ifndef _VMF_DEFINE_MSG_
#define _VMF_DEFINE_MSG_
#define MSG VMF_MSG
#define PeekMessage VMF_PeekMessage
#define PostSysThreadMessage VMF_PostSysThreadMessage
#define PostThreadMessage VMF_PostSysThreadMessage
#define DispatchMessage VMF_DispatchMessage
#define SendMessage VMF_SendMessage
#define GetQueueStatus VMF_GetQueueStatus
#define RegisterWindowMessage VMF_RegisterWindowMessage
#define MsgWaitForMultipleObjects VMF_MsgWaitForMultipleObjects
#define LOWORD(l)           ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l)           ((WORD)((DWORD_PTR)(l) >> 16))
#define LOBYTE(w)           ((BYTE)((DWORD_PTR)(w) & 0xff))
#define HIBYTE(w)           ((BYTE)((DWORD_PTR)(w) >> 8))
#define MAKEWORD(a,b)       ((WORD)(((BYTE)((DWORD_PTR)(a)&0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b)&0xff)))<<8))
#define MAKELONG(a,b)       ((LONG)(((WORD)((DWORD_PTR)(a)&0xffff)) | ((LONG)((WORD)((DWORD_PTR)(b)&0xffff)))<<16))
#endif

#ifndef _VMF_DEFINE_TIMER_
#define _VMF_DEFINE_TIMER_
#define timeSetEvent VMF_TimeSetEvent
#define timeKillEvent VMF_TimeKillEvent
#define TIME_ONESHOT VMF_TIME_ONESHOT 
#define TIME_PERIODIC VMF_TIME_PERIODIC 
#define TIME_CALLBACK_FUNCTION VMF_TIME_CALLBACK_FUNCTION 
#define TIME_CALLBACK_EVENT_SET VMF_TIME_CALLBACK_EVENT_SET  
#define TIME_CALLBACK_EVENT_PULSE VMF_TIME_CALLBACK_EVENT_PULSE 
#define TIME_KILL_SYNCHRONOUS VMF_TIME_KILL_SYNCHRONOUS 
#endif

#ifndef _VMF_DEFINE_COTASKMEMALLOC_
#define _VMF_DEFINE_COTASKMEMALLOC_
#define CoTaskMemAlloc VMF_CoTaskMemAlloc
#define CoTaskMemRealloc VMF_CoTaskMemRealloc
#define CoTaskMemFree VMF_CoTaskMemFree
#define GetTickCount  VMF_GetTickCount
#define MulDiv        VMF_MulDiv
#define GetSystemInfo VMF_GetSystemInfo
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#ifndef _VMF_DEFINE_LPTIMECALLBACK_
#define _VMF_DEFINE_LPTIMECALLBACK_
#define LPTIMECALLBACK VMF_LPTIMECALLBACK
#endif

#ifndef _VMF_DEFINE_FOURCC_
#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
	((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1)<<8) | ((DWORD)(BYTE)(ch2)<<16) | ((DWORD)(BYTE)(ch3)<<24))
#define mmioFOURCC(ch0, ch1, ch2, ch3) MAKEFOURCC(ch0, ch1, ch2, ch3)
#define FCC(ch4) ((((DWORD)(ch4)&0xFF)<<24)|(((DWORD)(ch4)&0xFF00)<<8)|(((DWORD)(ch4)&0xFF0000)>>8)|(((DWORD)(ch4)&0xFF000000)>>24))
#endif

#ifndef _DEFINE_STRUCT_
#define _DEFINE_STRUCT_
#define __STRUCT__ struct
#ifdef __cplusplus
#ifndef interface
#define interface class
#endif
#else
#define interface __STRUCT__
#endif
#ifdef __cplusplus
#define MIDL_INTERFACE(x)   class 
#else
#define MIDL_INTERFACE(x)   struct 
#endif
#endif

#ifndef _DEFINE_WIN_LIBRARY_
#define _DEFINE_WIN_LIBRARY_
#define HMODULE VMF_Lib
#define LoadLibraryA VMF_LoadLibrary
#define GetProcAddress VMF_GetProcAddress
#define FreeLibrary    VMF_FreeLibrary
#endif

// #ifndef _DEFINE_MEMSET_
// #define _DEFINE_MEMSET_
// #define memset VMF_Memset
// #define memcpy VMF_Memcpy
// #define free VMF_Free
// #define malloc VMF_Malloc
//#endif

#endif
