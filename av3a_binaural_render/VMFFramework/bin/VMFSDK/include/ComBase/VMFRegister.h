#ifndef __VMF_REGISTER_H__
#define __VMF_REGISTER_H__
#include <VMF_PlatForm.h>
#include <VMFCom.h>

EXTERN_GUID(VMF_CLSID_CATEGORY_MANAGER,	0x9a3a0fff, 0x3059, 0x43bb, 0x94, 0x38, 0x72, 0x12, 0x9d, 0x2c, 0x3b, 0x6c);
EXTERN_GUID(VMF_CLSID_CATEGORY_COMPONENT, 0x2fdd3692, 0xd1ea, 0x402a, 0x88, 0xfd, 0x3b, 0xe0, 0xdf, 0xb0, 0xe1, 0x44);
// {204BA3B1-2C3B-438B-9D78-C91F7088681A}
EXTERN_GUID(VMF_CLSID_CATEGORY_PROTOCOL, 0x204ba3b1, 0x2c3b, 0x438b, 0x9d, 0x78, 0xc9, 0x1f, 0x70, 0x88, 0x68, 0x1a);
// {E281A12C-2046-4A12-903A-766889C5A9B6}
EXTERN_GUID(VMF_CLSID_CATEGORY_FILEEXTMAP, 0xe281a12c, 0x2046, 0x4a12, 0x90, 0x3a, 0x76, 0x68, 0x89, 0xc5, 0xa9, 0xb6);
// {5BAE1E82-3EE3-4FA9-92EA-FB97BA905EC1}
EXTERN_GUID(VMF_CLSID_CATEGORY_MEDIATYPEMAP, 0x5bae1e82, 0x3ee3, 0x4fa9, 0x92, 0xea, 0xfb, 0x97, 0xba, 0x90, 0x5e, 0xc1);

typedef struct _REGISTERFILTER
{
	const CLSID * clsID;
	const WCHAR * strName;
	ULONG dwMerit;
	UINT nPins;
	const REGFILTERPINS* lpPin;
}REGISTERFILTER;

typedef struct _REGISTERINFO
{
	const WCHAR* pName;
	const CLSID* pClsID;
	VOID* pfnNew;
	VOID* plpfnInit;
	const REGISTERFILTER* pSetupFilter;
}VMF_REGISTERINFO;


typedef VMF_HRESULT (WINAPI *DllGetRegisterInfoFunc)(CLSID **ppclsidCategory, VOID **ppRegisterInfo, UINT *pRegisterNum);

typedef enum
{
	VMF_REGINFOTYPE_DEFAULT = 0,
	VMF_REGINFOTYPE_FILEEXT = 1,	//VMF_FILEEXTENSION
	VMF_REGINFOTYPE_PROTOCOL = 2,	//VMF_REG_PROTOCOL
	VMF_REGINFOTYPE_CHECKBYTES = 3,	//VMF_REG_CHECKBYTES
	VMF_REGINFOTYPE_FILTERTYPE = 4,	//VMF_REG_FILTERTYPE
	VMF_REGINFOTYPE_DATA = 5	    //VMF_REG_DATA
}VMF_REGINFOTYPE;

typedef struct _REGISTERINFO_BASE
{
	VMF_REGINFOTYPE eType;
	UINT uSize;
}VMF_REGISTERINFO_BASE;

typedef struct _VMF_FILEEXTENSION
{
	VMF_REGISTERINFO_BASE base;
	CHAR			*pFileExt;		//file extension
	CLSID			clsid;			//file source clsid
	CHAR			*pExInfo;		//extension information if any
	CLSID			MajorType;
	CLSID			SubType;
}VMF_REG_FILEEXTENSION;

typedef struct _VMF_PROTOCOL
{
	VMF_REGISTERINFO_BASE base;
	CHAR			*pProtocol;		//protocol
	CLSID			clsid;			//file source clsid
	CHAR			*pExInfo;		//extension information if any,
	UINT			uExtNum;
	CHAR			**ppFileExt;	//file extension
}VMF_REG_PROTOCOL;

typedef struct _VMF_REG_CHECKBYTES
{
	VMF_REGISTERINFO_BASE	base;
	CLSID MajorType;	//file major media type
	CLSID SubType;		//file sub media type
	CLSID clsid;		//file source clsid
	CHAR *pExInfo;		//extension information if any
	UINT cCheckBytes;	//the count of check bytes
	CHAR **ppCheckBytes;	//file check bytes if any
	UINT *pCheckBytesSize;	
}VMF_REG_CHECKBYTES;

typedef struct _VMF_REG_FILTERTYPE
{
	VMF_REGISTERINFO_BASE	base;
	CHAR *pFilterType;
}VMF_REG_FILTERTYPE;

typedef struct _VMF_REG_DATA
{
	VMF_REGISTERINFO_BASE	base;
	CHAR *pData;
}VMF_REG_DATA;
typedef VMF_HRESULT (WINAPI *DllGetRegisterInfoExFunc)(VMF_REGISTERINFO_BASE **pppRegisterInfo, UINT *pRegisterNum);
#endif