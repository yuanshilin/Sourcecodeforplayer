#ifndef __VMF_COMBASEEX_H__
#define __VMF_COMBASEEX_H__

#ifndef __IVMFLASTERROR_FWD_DEFINED__
#define __IVMFLASTERROR_FWD_DEFINED__
typedef interface IVMFLastError IVMFLastError;
#endif 	/* __IVMFLASTERROR_FWD_DEFINED__ */

#ifndef __IVMFLASTSAMPLE_FWD_DEFINED__
#define __IVMFLASTSAMPLE_FWD_DEFINED__
typedef interface IVMFLastSample IVMFLastSample;
#endif 	/* __IVMFLASTSAMPLE_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IVMFLastError_INTERFACE_DEFINED__
#define __IVMFLastError_INTERFACE_DEFINED__
/* interface IVMFLastError */
// {A11ABBB8-3D5C-4ead-96FE-64166686532D}
EXTERN_GUID(IID_IVMFLastError, 
0xa11abbb8, 0x3d5c, 0x4ead, 0x96, 0xfe, 0x64, 0x16, 0x66, 0x86, 0x53, 0x2d);

#if defined(__cplusplus) && !defined(CINTERFACE)

MIDL_INTERFACE("A11ABBB8-3D5C-4ead-96FE-64166686532D")
IVMFLastError : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetLastError(WCHAR *pError, DWORD *pLength) = 0;
};

#else 	/* C style interface */

typedef struct IVMFLastErrorVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IVMFLastError * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IVMFLastError * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			IVMFLastError * This);

		HRESULT ( STDMETHODCALLTYPE *GetLastError )( 
			IVMFLastError * This,
			WCHAR *pError, 
			DWORD *pLength);

	END_INTERFACE
} IVMFLastErrorVtbl;

interface IVMFLastError
{
	CONST_VTBL struct IIVMFLastErrorVtbl *lpVtbl;
};
#ifdef COBJMACROS
#define IVMFLastError_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFLastError_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFLastError_Release(This)	\
	(This)->lpVtbl -> Release(This)

#define IVMFLastError_GetLastError(pError, pLength)	\
	(This)->lpVtbl ->GetLastError(pError, pLength)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFLastError_INTERFACE_DEFINED__ */


#ifndef __IVMFLastSample_INTERFACE_DEFINED__
#define __IVMFLastSample_INTERFACE_DEFINED__
/* interface IVMFLastSample */
// {9430D396-A2E2-443D-8A06-CD0F88EABAAF}
EXTERN_GUID(IID_IVMFLastSample, 
0x9430d396, 0xa2e2, 0x443d, 0x8a, 0x6, 0xcd, 0xf, 0x88, 0xea, 0xba, 0xaf);

#if defined(__cplusplus) && !defined(CINTERFACE)

MIDL_INTERFACE("9430D396-A2E2-443D-8A06-CD0F88EABAAF")
IVMFLastSample : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetQualProp(IUnknown *pQualProp) = 0;
	virtual HRESULT STDMETHODCALLTYPE NewSegment() = 0;
	virtual HRESULT STDMETHODCALLTYPE SetLastSample(LONGLONG rtStart, LONGLONG rtStop) = 0;
};

#else 	/* C style interface */

typedef struct IVMFLastSampleVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IVMFLastSample * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IVMFLastSample * This);

		ULONG ( IVMFLastSample *Release )( 
			IVMFLastError * This);

		HRESULT (STDMETHODCALLTYPE *SetQualProp)(
			IVMFLastSample * This,
			IUnknown *pQualProp);

		HRESULT (STDMETHODCALLTYPE *NewSegment)(
			IVMFLastSample * This);

		HRESULT ( STDMETHODCALLTYPE *SetLastSample )( 
			IVMFLastSample * This, 
			LONGLONG rtStart,
			LONGLONG rtStop);

	END_INTERFACE
} IVMFLastSampleVtbl;

interface IVMFLastSample
{
	CONST_VTBL struct IVMFLastSampleVtbl *lpVtbl;
};
#ifdef COBJMACROS
#define IVMFLastSample_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFLastSample_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFLastSample_Release(This)	\
	(This)->lpVtbl -> Release(This)

#define IVMFLastSample_SetQualProp(This, p)	\
	(This)->lpVtbl ->SetQualProp(This, p)

#define IVMFLastSample_NewSegment(This)	\
	(This)->lpVtbl ->NewSegment(This)

#define IVMFLastSample_SetLastSample(This, rtStart, rtStop)	\
	(This)->lpVtbl ->SetLastSample(This,  rtStart, rtStop)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFLastError_INTERFACE_DEFINED__ */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif





