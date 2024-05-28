#ifndef __VMF_STRMIFEX_H__
#define __VMF_STRMIFEX_H__

#ifndef __IVMFSAVEGRAPH_FWD_DEFINED__
#define __IVMFSAVEGRAPH_FWD_DEFINED__
typedef interface IVMFSaveGraph IVMFSaveGraph;
#endif 	/* __IVMFSAVEGRAPH_FWD_DEFINED__ */

#ifndef __IVMFPERSISTEX_FWD_DEFINED__
#define __IVMFPERSISTEX_FWD_DEFINED__
typedef interface IVMFPersistEx IVMFPersistEx;
#endif 	/* __IVMFPERSISTEX_FWD_DEFINED__ */

#ifndef __IVMFGRAPHBUILDEREX_FWD_DEFINED__
#define __IVMFGRAPHBUILDEREX_FWD_DEFINED__
typedef interface IVMFGraphBuilderEx IVMFGraphBuilderEx;
#endif 	/* __IVMFGRAPHBUILDEREX_FWD_DEFINED__ */

#ifndef __IVMFSOURCEFILTER_FWD_DEFINED__
#define __IVMFSOURCEFILTER_FWD_DEFINED__
typedef interface IVMFSourceFilter IVMFSourceFilter;
#endif 	/* __IVMFSOURCEFILTER_FWD_DEFINED__ */

#ifndef __IVMFENUMSOURCE_FWD_DEFINED__
#define __IVMFENUMSOURCE_FWD_DEFINED__
typedef interface IVMFEnumSource IVMFEnumSource;
#endif 	/* __IVMFENUMSOURCE_FWD_DEFINED__ */



#ifndef __IVMFSaveGraph_INTERFACE_DEFINED__
#define __IVMFSaveGraph_INTERFACE_DEFINED__

// {064BCBA6-0EB8-4e6a-AB63-6A55757881F0}
EXTERN_GUID(IID_IVMFSaveGraph, 0x64bcba6, 0xeb8, 0x4e6a, 0xab, 0x63, 0x6a, 0x55, 0x75, 0x78, 0x81, 0xf0);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("064bcba6-0eb8-4e6a-ab63-6a55757881f0")
IVMFSaveGraph : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE SaveGraph(LPCTSTR pName) = 0;
};

#else 	/* C style interface */

typedef struct IVMFSaveGraphVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IFilterGraph * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IFilterGraph * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			IFilterGraph * This);

		HRESULT ( STDMETHODCALLTYPE *SaveGraph )( 
			IFilterGraph * This,
			/* [string][in] */ LPCTSTR pName);

	END_INTERFACE
} IVMFSaveGraphVtbl;

interface IVMFSaveGraph
{
	CONST_VTBL struct IVMFSaveGraphVtbl *lpVtbl;
};



#ifdef COBJMACROS


#define IVMFSaveGraph_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFSaveGraph_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFSaveGraph_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IVMFSaveGraph_SaveGraph(This,pName)	\
	(This)->lpVtbl -> SaveGraph(This,pName)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFSaveGraph_INTERFACE_DEFINED__ */


#ifndef __IVMFPersistEx_INTERFACE_DEFINED__
#define __IVMFPersistEx_INTERFACE_DEFINED__

// {689DA15D-CF5B-42cc-A6C9-1864D3835F9C}
EXTERN_GUID(IID_IVMFPersistEx, 0x689da15d, 0xcf5b, 0x42cc, 0xa6, 0xc9, 0x18, 0x64, 0xd3, 0x83, 0x5f, 0x9c);
#if defined(__cplusplus) && !defined(CINTERFACE)

MIDL_INTERFACE("689da15d-cf5b-42cc-a6c9-1864d3835f9c")
IVMFPersistEx : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetExInfo(LPWSTR *ppExInfo) = 0;
};

#else 	/* C style interface */

typedef struct IVMFPersistExVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IVMFPersistEx * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IVMFPersistEx * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			IVMFPersistEx * This);

		HRESULT ( STDMETHODCALLTYPE *GetExInfo )( 
			IVMFPersistEx * This,
			/* [string][in] */ LPWSTR *ppExInfo);

	END_INTERFACE
} IVMFSaveGraphVtbl;

interface IVMFPersistEx
{
	CONST_VTBL struct IVMFPersistExVtbl *lpVtbl;
};



#ifdef COBJMACROS


#define IVMFPersistEx_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFPersistEx_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFPersistEx_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IVMFPersistEx_GetExInfo(This,ppExInfo)	\
	(This)->lpVtbl -> GetExInfo(This,ppExInfo)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFPersistEx_INTERFACE_DEFINED__ */



#ifndef __IVMFGraphBuilderEx_INTERFACE_DEFINED__
#define __IVMFGraphBuilderEx_INTERFACE_DEFINED__
// {6A084E21-68E6-4d43-AC40-A8299014AF60}
EXTERN_GUID(IID_IVMFGraphBuilderEx, 
			0x6a084e21, 0x68e6, 0x4d43, 0xac, 0x40, 0xa8, 0x29, 0x90, 0x14, 0xaf, 0x60);
#if defined(__cplusplus) && !defined(CINTERFACE)
MIDL_INTERFACE("6A084E21-68E6-4d43-AC40-A8299014AF60")
IVMFGraphBuilderEx : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE RenderFile(VOID* lpFileStruct, GUID *pStructguid) = 0;
	virtual HRESULT STDMETHODCALLTYPE AddSourceFilter( 
		/* [in] */ VOID* lpFileStruct,
		/* [in] */GUID* pStructguid,
		/* [unique][in] */ LPCWSTR lpcwstrFilterName,
		/* [out] */ IBaseFilter **ppFilter) = 0;
};

#else 	/* C style interface */

typedef struct IVMFGraphBuilderExVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IVMFGraphBuilderEx * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IVMFGraphBuilderEx * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			IVMFGraphBuilderEx * This);

		HRESULT ( STDMETHODCALLTYPE *RenderFile )( 
			IVMFGraphBuilderEx * This,
			VOID* lpFileStruct, 
			GUID *pStructguid);

		HRESULT ( STDMETHODCALLTYPE *AddSourceFilter )( 
			IVMFGraphBuilderEx * This,
			VOID* lpFileStruct, 
			GUID *pStructguid,
			LPCWSTR lpcwstrFilterName,
			IBaseFilter **ppFilter);

	END_INTERFACE
} IVMFSaveGraphVtbl;

interface IVMFGraphBuilderEx
{
	CONST_VTBL struct IVMFGraphBuilderExVtbl *lpVtbl;
};



#ifdef COBJMACROS


#define IVMFGraphBuilderEx_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFGraphBuilderEx_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFGraphBuilderEx_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IVMFGraphBuilderEx_RenderFile(This,lpFileStruct,pStructguid)	\
	(This)->lpVtbl -> RenderFile(This,lpFileStruct,pStructguid)

#define IVMFGraphBuilderEx_AddSourceFilter(This,lpFileStruct,pStructguid,lpcwstrFilterName,ppFilter)	\
	(This)->lpVtbl -> AddSourceFilter(This,lpFileStruct,pStructguid,lpcwstrFilterName,ppFilter)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFGraphBuilderEx_INTERFACE_DEFINED__ */



#ifndef __IVMFSourceFilter_INTERFACE_DEFINED__
#define __IVMFSourceFilter_INTERFACE_DEFINED__
/* interface IVMFSourceFilter */
// {1DAECB29-75DF-437a-BA3F-2CFBD0AD91F0}
EXTERN_GUID(IID_IVMFSourceFilter, 
			0x1daecb29, 0x75df, 0x437a, 0xba, 0x3f, 0x2c, 0xfb, 0xd0, 0xad, 0x91, 0xf0);
#if defined(__cplusplus) && !defined(CINTERFACE)

MIDL_INTERFACE("1DAECB29-75DF-437a-BA3F-2CFBD0AD91F0")
IVMFSourceFilter : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE Load( 
		/* [in] */ VOID* lpFileStruct, 
		/* [in] */ GUID* pStructguid,
		/* [unique][in] */ const AM_MEDIA_TYPE *pmt) = 0;

	virtual HRESULT STDMETHODCALLTYPE GetCurFile( 
		/* [in] */ VOID* lpFileStruct, 
		/* [in] */ GUID* pStructguid,
		/* [out] */ AM_MEDIA_TYPE *pmt) = 0;

};

#else 	/* C style interface */

typedef struct IVMFSourceFilterVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IVMFSourceFilter * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IVMFSourceFilter * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			IVMFSourceFilter * This);

		HRESULT ( STDMETHODCALLTYPE *Load )( 
			IVMFSourceFilter * This,
			/* [in] */ VOID* lpFileStruct, 
			/* [in] */ GUID* pStructguid,
			/* [unique][in] */ const AM_MEDIA_TYPE *pmt);

		HRESULT ( STDMETHODCALLTYPE *GetCurFile )( 
			IVMFSourceFilter * This,
			/* [in] */ VOID* lpFileStruct, 
			/* [in] */ GUID* pStructguid,
			/* [out] */ AM_MEDIA_TYPE *pmt);

	END_INTERFACE
} IVMFSourceFilterVtbl;

interface IVMFSourceFilter
{
	CONST_VTBL struct IVMFSourceFilterVtbl *lpVtbl;
};



#ifdef COBJMACROS


#define IVMFSourceFilter_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFSourceFilter_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFSourceFilter_Release(This)	\
	(This)->lpVtbl -> Release(This)


#define IVMFSourceFilter_Load(This,lpFileStruct,pStructguid,pmt)	\
	(This)->lpVtbl -> Load(This,lpFileStruct,pStructguid,pmt)

#define IVMFSourceFilter_GetCurFile(This,lpFileStruct,pStructguid,pmt)	\
	(This)->lpVtbl -> GetCurFile(This,lpFileStruct,pStructguid,pmt)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFSourceFilter_INTERFACE_DEFINED__ */

#ifndef __IVMFEnumSource_INTERFACE_DEFINED__
#define __IVMFEnumSource_INTERFACE_DEFINED__
/* interface IVMFEnumSource */
// {36EF4DA6-987B-478a-AE55-2AC5026A54B3}
EXTERN_GUID(IID_IVMFEnumSource, 
			0x36ef4da6, 0x987b, 0x478a, 0xae, 0x55, 0x2a, 0xc5, 0x2, 0x6a, 0x54, 0xb3);
#if defined(__cplusplus) && !defined(CINTERFACE)

MIDL_INTERFACE("36EF4DA6-987B-478a-AE55-2AC5026A54B3")
IVMFEnumSource : public IUnknown
{
public:
	virtual HRESULT STDMETHODCALLTYPE EnumFileSources( 
		/* [in] */ VOID* lpFileStruct, 
		/* [in] */ GUID* pStructguid,
		IVMFEnumComponent **ppEnum,
		CLSID *pMajorType, 
		CLSID *pSubType, 
		BOOL *pDefault) = 0;
};

#else 	/* C style interface */

typedef struct IVMFEnumSourceVtbl
{
	BEGIN_INTERFACE

		HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
		IVMFEnumSource * This,
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ void **ppvObject);

		ULONG ( STDMETHODCALLTYPE *AddRef )( 
			IVMFEnumSource * This);

		ULONG ( STDMETHODCALLTYPE *Release )( 
			IVMFEnumSource * This);

		HRESULT ( STDMETHODCALLTYPE *EnumFileSources )( 
			IVMFEnumSource * This,
			/* [in] */ VOID* lpFileStruct, 
			/* [in] */ GUID* pStructguid,
			IVMFEnumComponent **ppEnum,
			CLSID *pMajorType, 
			CLSID *pSubType, 
			BOOL *pDefault);

	END_INTERFACE
} IVMFEnumSourceVtbl;

interface IVMFEnumSource
{
	CONST_VTBL struct IVMFEnumSourceVtbl *lpVtbl;
};
#ifdef COBJMACROS
#define IVMFEnumSource_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFEnumSource_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFEnumSource_Release(This)	\
	(This)->lpVtbl -> Release(This)

#define IVMFEnumSource_EnumFileSources(This,lpFileStruct,pStructguid,ppEnum,pMajorType,pSubType,pDefault)	\
	(This)->lpVtbl -> EnumFileSources(This,lpFileStruct,pStructguid,ppEnum,pMajorType,pSubType,pDefault)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFEnumSource_INTERFACE_DEFINED__ */

#endif//__VMF_STRMIFEX_H__