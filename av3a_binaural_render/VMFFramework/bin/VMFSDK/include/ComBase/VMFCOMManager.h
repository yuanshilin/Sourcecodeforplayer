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
*-------------------------------------------------------------------------------------------------*/

/*************************************************************************************************
**      Copyright (c) 2011 by ArcSoft Inc.
**      Name        : VMF_COMManager.h
**      Purpose     : the definitions for IVMFCOMManager
**      Additional  :
**------------------------------------------------------------------------------------------------
**      Maintenance History:
**************************************************************************************************/
#ifndef VMF_COMMANAGER_H
#define VMF_COMMANAGER_H

#include <VMF_PlatForm.h>
#include <VMFComBase.h>


typedef struct _stEnumComponentInfo
{
	CLSID clsid;
	WCHAR* pExInfo;			//caller should Call VMF_Free() to free it.
	CLSID clsidCategory;
	DWORD dwMerit;
}EnumComponentInfo;

VMF_VOID DeleteEnumComponentInfo(EnumComponentInfo *pCompInfo);

class IVMFEnumComponent : public IUnknown
{
public:
	virtual HRESULT Clone(IVMFEnumComponent **ppEnum) = 0;
	virtual HRESULT Next(ULONG cComponents, EnumComponentInfo **ppCompInfo, ULONG *pcFetched) = 0;
	virtual HRESULT Reset() = 0;
	virtual HRESULT Skip(ULONG celt) = 0;
};

EXTERN_GUID(IID_VMF_COM_MANAGER, 0x597740f9, 0xaf06, 0x4812, 0xa9, 0x52, 0x9c, 0x9b, 0x77, 0xea, 0x9b, 0x9f);
class IVMFCOMManager:public IUnknown
{
public:
	virtual HRESULT SetRegisterFile(WCHAR* pFileName) = 0;
	//pSize: Size, in TCHAR values
	virtual HRESULT GetRegisterFile(WCHAR* pFileName, UINT *pSize) = 0;
	virtual HRESULT CreateComponent(CLSID clsid, WCHAR* pExInfo, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID guid, IUnknown **ppInterface) = 0;

	//pSize: Size, in TCHAR values
	virtual HRESULT GetComponentName(CLSID clsid, WCHAR* pExInfo, WCHAR* pName, UINT *pSize) = 0;
	virtual HRESULT GetComponentPath(CLSID clsid, WCHAR* pExInfo, WCHAR* pPath, UINT *pSize) = 0;

	//dwFlags : [in] dwFlags=1, enum source filter.
	//dwMerit : [in] Minimum merit value. The enumeration excludes filters with a lesser merit value. 
	//				 If dwMerit is higher than MERIT_DO_NOT_USE, it also excludes filters whose category has a merit less than or equal to MERIT_DO_NOT_USE.
	virtual HRESULT EnumMatchingFilters(DWORD dwFlags, BOOL bExactMatch, DWORD dwMerit, BOOL bInputNeeded, DWORD  cInputTypes, const CLSID *pInputTypes,
										BOOL bOutputNeeded, DWORD  cOutputTypes,const CLSID *pOutputTypes, IVMFEnumComponent **ppEnum) = 0;
	
	virtual HRESULT EnumFileSources(WCHAR *pFileName, IVMFEnumComponent **ppEnum, CLSID *pMajorType, CLSID *pSubType, BOOL *pDefault=NULL) = 0;

	//enum components of a category
	virtual HRESULT EnumComponents(CLSID guidCategory, DWORD dwFlags, IVMFEnumComponent **ppEnum) = 0;

	virtual VOID Delete() = 0;
};

EXTERN_GUID(IID_VMF_REGISTER, 0x205250ee, 0x36c4, 0x406e, 0x9f, 0xd9, 0x92, 0xfa, 0x4f, 0xca, 0xcd, 0xd1);
class IVMFRegister: public IUnknown
{
public:
	virtual HRESULT SetRegisterFile(WCHAR* pFileName) = 0;
	//pSize: Size, in TCHAR values
	virtual HRESULT GetRegisterFile(WCHAR* pFileName, UINT *pSize) = 0;
	virtual HRESULT Register(WCHAR* FilePath) = 0;
	virtual HRESULT UnRegister(WCHAR* FilePath) = 0;

	virtual VOID Delete() = 0;
};


// {EB603039-32DB-492E-8E6E-3A5BAC645268}
EXTERN_GUID(IID_VMF_COM_READER, 0xeb603039, 0x32db, 0x492e, 0x8e, 0x6e, 0x3a, 0x5b, 0xac, 0x64, 0x52, 0x68);
class IVMFCOMReader: public IUnknown
{
public:
	virtual HRESULT SetRegisterFile(WCHAR* pFileName) = 0;
	//pSize: Size, in TCHAR values
	virtual HRESULT GetRegisterFile(WCHAR* pFileName, UINT *pSize) = 0;
	virtual HRESULT EnumAllFilters(IVMFEnumComponent **ppEnum) = 0;
	virtual HRESULT GetFilterInfo(CLSID clsid, WCHAR* pExInfo, WCHAR *pComInfo, UINT *pSize) = 0;
};

#ifdef __cplusplus
extern "C" {
#endif
	STDAPI VMF_CreateCOMManager(IVMFCOMManager **ppCOMManager);
	STDAPI VMF_CreateRegister(IVMFRegister **ppRegister);
	STDAPI VMF_CreateCOMReader(IVMFCOMReader **ppCOMReader);

	typedef HRESULT (STDAPICALLTYPE *VMF_CreateCOMManagerFunc)(IVMFCOMManager **ppCOMManager);
	typedef HRESULT (STDAPICALLTYPE *VMF_CreateRegisterFunc)(IVMFRegister **ppCOMManager);
	typedef HRESULT (STDAPICALLTYPE *VMF_CreateCOMReaderFunc)(IVMFCOMReader **ppCOMReader);

#ifdef __cplusplus
}
#endif
#endif
