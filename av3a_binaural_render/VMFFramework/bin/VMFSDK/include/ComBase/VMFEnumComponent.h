#ifndef VMF_ENUMCOMPONENT_H
#define VMF_ENUMCOMPONENT_H
#include <VMF_PlatForm.h>
#include <VMFCOMManager.h>

typedef struct stENUM_COMPONENT 
{
	VMFList		 list;
	EnumComponentInfo info;
}ENUM_COMPONENT;

class CVMFEnumComponent : public CUnknown
						, public IVMFEnumComponent
{
public:
	CVMFEnumComponent(LPUNKNOWN pUnk, ENUM_COMPONENT *pComponentList, VMF_U32 uCount, HRESULT *ret)
				 : CUnknown(TEXT("CVMFEnumComponent"), pUnk, ret)
	{
		m_pCurrent = m_pComponent = pComponentList;
		m_uCount = uCount;
		if(ret)
			*ret = 0;
	};
	~CVMFEnumComponent()
	{
		ENUM_COMPONENT *pHead = m_pComponent;
		ENUM_COMPONENT *pNext = NULL;
		while (pHead)
		{
			pNext = (ENUM_COMPONENT *)VMF_LIST_NEXT(pHead);
			VMF_Safe_Free(pHead->info.pExInfo);
			VMF_DeleteList(VMF_LIST(pHead));
			pHead = pNext;
		}
		m_pComponent = VMF_NULL;
		m_pCurrent = VMF_NULL;
		m_uCount = 0;
	};

	DECLARE_IUNKNOWN;
	STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

	HRESULT Clone(__deref_out IVMFEnumComponent **ppEnum)
	{
		HRESULT hr = E_FAIL;
		CVMFEnumComponent *pEnumCom = NULL;
		if(!ppEnum)
			return E_INVALIDARG;
		pEnumCom = new CVMFEnumComponent(VMF_NULL, m_pComponent, m_uCount, &hr);
		if(VMF_SUCCEEDED(hr) && pEnumCom)
		{
			*ppEnum = (IVMFEnumComponent*)pEnumCom;
		}
		return hr;
	};

	HRESULT Next(__deref_in ULONG cComponents, __deref_out  EnumComponentInfo **ppComponent, __deref_out  ULONG *pcFetched);
	HRESULT Reset();
	HRESULT Skip(__deref_in ULONG celt);
private:
	ULONG m_Ref;
	VMF_Mutex m_Mutex;

	ENUM_COMPONENT* m_pComponent;
	ENUM_COMPONENT* m_pCurrent;
	UINT m_uCount;
};

#endif //VMF_ENUMCOMPONENT_H
