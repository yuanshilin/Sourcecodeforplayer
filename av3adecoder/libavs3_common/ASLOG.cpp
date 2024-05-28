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
 *  Name  : ASLOG.c
 *  Description: unify the way of output log infomation
 *  Maintenance History:     2010/5/22, created by yxie@arcsoft.com.cn
 *-------------------------------------------------------------------------------------------------*/



#include "VMFCom.h"
#include "stdio.h"
#include <stdarg.h>

#include "ASLOG.h"

#ifdef WIN32
#define BIN_COM_MANAFER "VMFCOMManager.dll"
#define EXTERN_GUID_VMF EXTERN_GUID

static HMODULE GetCurModule()
{
    MEMORY_BASIC_INFORMATION m = {0};
    VirtualQuery(GetCurModule, &m, sizeof (MEMORY_BASIC_INFORMATION));
    return (HMODULE) m.AllocationBase;
}

void GetCurrentModuleName(WCHAR *pathw, VMF_U32 size)
{
    GetModuleFileNameW(GetCurModule(), pathw, size);
}

#else
#define BIN_COM_MANAFER "libVMFCOMManager.so"
#define EXTERN_GUID_VMF DEFINE_GUID

void GetCurrentModuleName(WCHAR *pathw, VMF_U32 size)
{
    VMF_StrcpyW(pathw, (VMF_WCHAR *)_ModuleName_);
}
#endif

#ifdef WIN32
#define vsprintf_safe vsprintf_s
#else
#define vsprintf_safe vsnprintf
#endif

VMF_U32 g_logLevel = 5;

class CASLOGInit
{
public:

    CASLOGInit()
    {
        ASLOG_Init();
    }

    ~CASLOGInit()
    {
        ASLOG_Uninit();
    }
};
CASLOGInit g_aslog;

// CLSID_ASLOG COM CLSID is : {28C52B68-23D6-44ce-B923-B6E51FF47DB2}
EXTERN_GUID_VMF(CLSID_ASLOG,
                0x28c52b68, 0x23d6, 0x44ce, 0xb9, 0x23, 0xb6, 0xe5, 0x1f, 0xf4, 0x7d, 0xb2);

// {16389917-8111-474d-B03B-0F34B172599E}
EXTERN_GUID_VMF(IID_IASLOG,
                0x16389917, 0x8111, 0x474d, 0xb0, 0x3b, 0xf, 0x34, 0xb1, 0x72, 0x59, 0x9e);

EXTERN_GUID_VMF(IID_ISetGet,
                0x34389945, 0x8111, 0x144d, 0xb0, 0x12, 0x67, 0x54, 0x36, 0x16, 0x67, 0x93);

#define OPT_SetErrorCode 1
typedef struct
{
	int errorCode;
	char *errorStr;
}ASLOG_ErroInfo;
interface ISetGet : public IUnknown
{
public:
	virtual STDMETHODIMP Set(VMF_U32 optioin, void* p) = 0;
	virtual STDMETHODIMP Get(VMF_U32 optioin, void* p) = 0;
};

interface DECLSPEC_UUID("16389917-8111-474d-b03b-0f34b172599e") IASLOG :
public IUnknown
{
    public:
    virtual STDMETHODIMP ASLOG_Init(WCHAR * module_name, VMF_U32 * logLevel) = 0;
    virtual STDMETHODIMP ASLOG_MsgW(VMF_U32 logLevel, WCHAR * format, va_list args) = 0;
    virtual STDMETHODIMP ASLOG_MsgA(VMF_U32 logLevel, char * format, va_list args) = 0;
    virtual STDMETHODIMP ASLOG_tMsgW(VMF_U32 logLevel, WCHAR * format, va_list args) = 0;
    virtual STDMETHODIMP ASLOG_tMsgA(VMF_U32 logLevel, char * format, va_list args) = 0;
    virtual STDMETHODIMP ASLOG_Flush() = 0;
    DWORD reserved[10];
};

#define OPT_SetSimpleMsg    0x101
#define OPT_GetSimpleMode   0x200
#define OPT_LogHook         0x204
enum LogHookID
{
    Log_Unknown,
    Log_Receive,
    Log_GetBuffer,
    Log_Deliver
};

typedef struct _ASLOG_SimpleMsg
{
    int nModuleID;
    int nMsgID;
    char szMsg[4096];
} ASLOG_SimpleMsg;

IASLOG *g_pIASLOG = NULL;
ISetGet *g_pASSetGet = NULL;

BOOL g_bSimpleMode = FALSE;
static BOOL inited = FALSE;
WCHAR g_module_nameW[MAX_PATH];

IVMFCOMManager *g_pCoMManger = NULL;
VMF_HANDLE g_pComMgrLib = NULL;

HRESULT CreateInstance(GUID clsid, REFIID iid, void **ppF)
{

    HRESULT hr = S_OK;

    VMF_CreateCOMManagerFunc pfnCreateManager = VMF_NULL;

    g_pComMgrLib = VMF_LoadLibrary((VMF_CHAR *)BIN_COM_MANAFER);

    if (!g_pComMgrLib)
    {
        VMF_Printf("COMManager is not exist.\n");
        return E_FAIL;
    }

    pfnCreateManager = (VMF_CreateCOMManagerFunc) VMF_GetProcAddress(g_pComMgrLib, (VMF_CHAR *) "VMF_CreateCOMManager");
    if (!pfnCreateManager)
    {
        hr = E_FAIL;
        goto ERROR_EXIT;
    }


    hr = pfnCreateManager(&g_pCoMManger);
    if (S_OK != hr || !g_pCoMManger)
        goto ERROR_EXIT;

    WCHAR reg_path[255];
    VMF_StrcpyW(reg_path, (VMF_WCHAR *)(L"vmf_registry.reg"));
    hr = g_pCoMManger->SetRegisterFile(reg_path);
    if (hr != S_OK)
        goto ERROR_EXIT;
    hr = g_pCoMManger->CreateComponent(CLSID_ASLOG, NULL, NULL, CLSCTX_INPROC_SERVER, IID_IASLOG, (IUnknown **) ppF);
    if (hr == S_OK)
        return hr;
ERROR_EXIT:
    VMF_Safe_Release(g_pCoMManger);
    VMF_Safe_FreeLibrary(g_pComMgrLib);
    return hr;
}

void ASLOG_Init()
{
    if (inited == TRUE)
        return;

    //VMF_CoInitialize(NULL);

    HRESULT hr = CreateInstance(CLSID_ASLOG, IID_IASLOG, (void **) &g_pIASLOG);
    if (hr != S_OK || g_pIASLOG == NULL)
    {
        g_logLevel = 0; //no need to log anyting
        inited = TRUE;
//        VMF_CoUninitialize();
        return;
    }

    WCHAR pathw[MAX_PATH];


    GetCurrentModuleName(pathw, MAX_PATH);
    WCHAR *wp = pathw + wcslen(pathw) - 1;
    while (*wp != L'\\' && *wp != L'/' && wp > pathw)
        wp--;
    if (*wp == L'\\' || *wp == L'/') wp++;
    VMF_StrcpyW(g_module_nameW, wp);

    g_pIASLOG->ASLOG_Init(g_module_nameW, &g_logLevel);

    if (NULL != g_pASSetGet)
    {
        g_pASSetGet->Release();
        g_pASSetGet = NULL;
    }
    g_pIASLOG->QueryInterface(IID_ISetGet, (void **) &g_pASSetGet);
    
    if (NULL != g_pASSetGet)
        g_bSimpleMode = (g_pASSetGet->Get(OPT_GetSimpleMode, NULL) > 0);

    inited = 1;
}

void ASLOG_Uninit()
{
    if (inited == FALSE)
        return;
    if (g_pIASLOG)
    {
        g_pIASLOG->ASLOG_Flush();

        if (NULL != g_pASSetGet)
        {
            g_pASSetGet->Release();
            g_pASSetGet = NULL;
        }

        g_pIASLOG->Release();
    }

    g_pIASLOG = NULL;
    VMF_Safe_Release(g_pCoMManger);
    VMF_Safe_FreeLibrary(g_pComMgrLib);
    inited = FALSE;
    //VMF_CoUninitialize();
};

void ASLOG_MsgW(unsigned long LogLevel, const WCHAR *format, ...)
{
    if ((g_logLevel >= LogLevel) && g_pIASLOG)
    {
        va_list args;
        va_start(args, format);
        g_pIASLOG->ASLOG_MsgW(LogLevel, (WCHAR*)format, args);
        va_end(args);

    }
};

void ASLOG_MsgA_Simple(int nMsgID, const char *pFormatSimple, unsigned long ulLogLevel, const char *pFormat, ...)
{
    if (g_logLevel < ulLogLevel)
        return;

    if (g_bSimpleMode)
    {
        ASLOG_SimpleMsg msgInfo = {0, 0, "\0"};
        msgInfo.nModuleID = _ModuleID_;
        msgInfo.nMsgID = nMsgID;

        va_list args;
        va_start(args, pFormat);
        vsprintf_safe(msgInfo.szMsg, sizeof(msgInfo.szMsg), pFormatSimple, args);
        va_end(args);

        g_pASSetGet->Set(OPT_SetSimpleMsg, &msgInfo);
    }
    else if (NULL != g_pIASLOG)
    {
        va_list args;
        va_start(args, pFormat);
        g_pIASLOG->ASLOG_MsgA(ulLogLevel, (char*)pFormat, args);
        va_end(args);
    }
}

void ASLOG_MsgA(unsigned long LogLevel, const char *format, ...)
{
    if ((g_logLevel >= LogLevel) && g_pIASLOG)
    {
        va_list args;
        va_start(args, format);
        g_pIASLOG->ASLOG_MsgA(LogLevel, (char*)format, args);
        va_end(args);
    }
};

void ASLOG_tMsgW(unsigned long LogLevel, const WCHAR *format, ...)
{
    if ((g_logLevel >= LogLevel) && g_pIASLOG)
    {
        va_list args;
        va_start(args, format);
        g_pIASLOG->ASLOG_tMsgW(LogLevel, (WCHAR*)format, args);
        va_end(args);
    }
};

void ASLOG_tMsgA(unsigned long LogLevel, const char *format, ...)
{
    if ((g_logLevel >= LogLevel) && g_pIASLOG)
    {
        va_list args;
        va_start(args, format);
        g_pIASLOG->ASLOG_tMsgA(LogLevel, (char*)format, args);
        va_end(args);
    }
};

void ASLOG_ErrorNotify(int errCode, const char *errStr)
{
	if(g_pIASLOG)
	{
		ISetGet *pSet = 0;
		g_pIASLOG->QueryInterface(IID_ISetGet,(void**)&pSet);
		if(pSet)
		{
			ASLOG_ErroInfo errInfo;
			errInfo.errorCode = errCode;
			errInfo.errorStr = (char*)errStr;
			pSet->Set(OPT_SetErrorCode,&errInfo);
			pSet->Release();
		}
	}
}

#define OPT_SetupProcSnap  0x206
#define OPT_UpdateProcSnap  0x207
#define OPT_RemoveModuleMap 0x20a
#define OPT_UpdateProcSnapString  0x20b

typedef struct _SetupProcSnapInfo
{
    void *pModule;
    char *pModuleName;
    char *pVarName;
    void *pVarHandle;
}SetupProcSnapInfo;

typedef struct _UpdateProcSnapInfo
{
    void *pVarHandle;
    long long llVar;
}UpdateProcSnapInfo;

typedef struct _UpdateProcSnapInfoString
{
    void *pVarHandle;
    char* pVar;
}UpdateProcSnapInfoString;

void *ASLOG_SetupProcSnap(void *pModule, char *pMuduleName, char *pVarName)
{
    if (g_pIASLOG)
    {
        ISetGet *pSet = 0;
        g_pIASLOG->QueryInterface(IID_ISetGet, (void**)&pSet);
        if (pSet)
        {
            SetupProcSnapInfo SnapInfo = {};
            SnapInfo.pModule = pModule;
            SnapInfo.pModuleName = pMuduleName;
            SnapInfo.pVarName = pVarName;
            pSet->Set(OPT_SetupProcSnap, &SnapInfo);
            pSet->Release();

            return SnapInfo.pVarHandle;
        }
    }

    return NULL;
}

void ASLOG_UpdateProcSnap(void *pModuleSnap, long long llVar)
{
    if (g_pIASLOG)
    {
        ISetGet *pSet = 0;
        g_pIASLOG->QueryInterface(IID_ISetGet, (void**)&pSet);
        if (pSet)
        {
            UpdateProcSnapInfo updateSnapInfo = {};
            updateSnapInfo.pVarHandle = pModuleSnap;
            updateSnapInfo.llVar = llVar;
            pSet->Set(OPT_UpdateProcSnap, &updateSnapInfo);
            pSet->Release();
        }
    }
}

void ASLOG_UpdateProcSnapString(void *pModuleSnap, char *pVar)
{
    if (g_pIASLOG)
    {
        ISetGet *pSet = 0;
        g_pIASLOG->QueryInterface(IID_ISetGet, (void**)&pSet);
        if (pSet)
        {
            UpdateProcSnapInfoString updateSnapInfo = {};
            updateSnapInfo.pVarHandle = pModuleSnap;
            updateSnapInfo.pVar = pVar;
            pSet->Set(OPT_UpdateProcSnapString, &updateSnapInfo);
            pSet->Release();
        }
    }
}

void ASLOG_UpdateProcSnapBuffer(void *pModuleSnap, void *pVar, int nLen)
{
#define OPT_UpdateProcSnapBuffer  0x20e
	typedef struct _UpdateProcSnapInfoBuffer
	{
		void *pVarHandle;
		void* pVar;
		int nLen;
	}UpdateProcSnapInfoBuffer;

	if (g_pIASLOG)
	{
		ISetGet *pSet = 0;
		g_pIASLOG->QueryInterface(IID_ISetGet, (void**)&pSet);
		if (pSet)
		{
			UpdateProcSnapInfoBuffer updateSnapInfo = {};
			updateSnapInfo.pVarHandle = pModuleSnap;
			updateSnapInfo.pVar = pVar;
			updateSnapInfo.nLen = nLen;
			pSet->Set(OPT_UpdateProcSnapBuffer, &updateSnapInfo);
			pSet->Release();
		}
	}
}

void ASLOG_RemoveProcSnap(void *pModule)
{
    if (g_pIASLOG)
    {
        ISetGet *pSet = 0;
        g_pIASLOG->QueryInterface(IID_ISetGet, (void**)&pSet);
        if (pSet)
        {
            pSet->Set(OPT_RemoveModuleMap, pModule);
            pSet->Release();
        }
    }
}

#ifdef __cplusplus
/*
 * CAutoLogHook
 */
//if any module wants to use CAutoLogHook, it shoud declar macro _CAutoLogHook_, and declare class CAutoLogHook in its' ASLOG.h as below
/*
class CAutoLogHook
{
public:
    CAutoLogHook(void *pCaller, int nID, int nCheckExit = FALSE, int nWarningTime = 1000);
    ~CAutoLogHook();

private:

    struct LogHookInfo
    {
        int nID;
        int nWarningTime;
        int nIsEnter;
        int nCheckExit;
        void *pCaller;
    } m_HookInfo;
};
*/
#ifdef _CAutoLogHook_ 
CAutoLogHook::CAutoLogHook(void *pCaller, int nID, int nCheckExit, int nWarningTime)
{
    if (!g_pIASLOG)
        return;

    ISetGet *pSet = 0;
    g_pIASLOG->QueryInterface(IID_ISetGet, (void**) &pSet);
    if (pSet)
    {
        m_HookInfo.pCaller = pCaller;
        m_HookInfo.nID = nID;
        m_HookInfo.nWarningTime = nWarningTime;
        m_HookInfo.nIsEnter = 1;
        m_HookInfo.nCheckExit = nCheckExit;
        pSet->Set(OPT_LogHook, &m_HookInfo);
        pSet->Release();
    }
}

CAutoLogHook::~CAutoLogHook()
{
    if (!g_pIASLOG)
        return;

    ISetGet *pSet = 0;
    g_pIASLOG->QueryInterface(IID_ISetGet, (void**) &pSet);
    if (pSet)
    {
        m_HookInfo.nIsEnter = 0;
        pSet->Set(OPT_LogHook, &m_HookInfo);
        pSet->Release();
    }
}
#endif
#endif