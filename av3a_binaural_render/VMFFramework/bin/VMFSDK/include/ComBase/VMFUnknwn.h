#ifndef __VMF_UNKNOWN_H__
#define __VMF_UNKNOWN_H__

#if defined(__cplusplus) && !defined(CINTERFACE)
#if !defined(BEGIN_INTERFACE)
   #define BEGIN_INTERFACE   
   #define END_INTERFACE
#endif
#else
#if !defined(BEGIN_INTERFACE)
#if defined(_MPPC_)
    #define BEGIN_INTERFACE       VOID    *b;
    #define END_INTERFACE
#else
   #define BEGIN_INTERFACE   
   #define END_INTERFACE
#endif
#endif
#endif
/* Forward Declarations */ 
#ifndef __IUnknown_FWD_DEFINED__
#define __IUnknown_FWD_DEFINED__
typedef interface IUnknown IUnknown;
#endif 	/* __IUnknown_FWD_DEFINED__ */


#ifndef __AsyncIUnknown_FWD_DEFINED__
#define __AsyncIUnknown_FWD_DEFINED__
typedef interface AsyncIUnknown AsyncIUnknown;
#endif 	/* __AsyncIUnknown_FWD_DEFINED__ */


#ifndef __IClassFactory_FWD_DEFINED__
#define __IClassFactory_FWD_DEFINED__
typedef interface IClassFactory IClassFactory;
#endif 	/* __IClassFactory_FWD_DEFINED__ */

/* Forward Declarations */ 
#ifndef __IVMFVersion_FWD_DEFINED__
#define __IVMFVersion_FWD_DEFINED__
typedef interface IVMFVersion IVMFVersion;
#endif 	/* __IVMFVersion_FWD_DEFINED__ */

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IUnknown_INTERFACE_DEFINED__
#define __IUnknown_INTERFACE_DEFINED__

/* interface IUnknown */
/* [unique][uuid][object][local] */ 

typedef IUnknown *LPUNKNOWN;

#if (_MSC_VER >= 1100) && defined(__cplusplus) && !defined(CINTERFACE)
	EXTERN_GUID(IID_IUnknown,0x00000000,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
    extern "C++"
    {
        MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
        IUnknown
        {
        public:
            virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
                /* [in] */ REFIID riid,
                /* [iid_is][out] */ void  **ppvObject) = 0;
            
            virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
            
            virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
    	
            template<class Q>
    	HRESULT STDMETHODCALLTYPE QueryInterface(Q** pp)
    	{
    	    return QueryInterface(__uuidof(Q), (void **)pp);
    	} 
        };
    } // extern C++
#else

EXTERN_GUID(IID_IUnknown,0x00000000,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000000-0000-0000-C000-000000000046")
    IUnknown
    {
    public:
        BEGIN_INTERFACE
        virtual HRESULT STDMETHODCALLTYPE QueryInterface( 
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject) = 0;
        
        virtual ULONG STDMETHODCALLTYPE AddRef( void) = 0;
        
        virtual ULONG STDMETHODCALLTYPE Release( void) = 0;
        
        END_INTERFACE
    };
    
#else 	/* C style interface */

    typedef struct IUnknownVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IUnknown * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IUnknown * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IUnknown * This);
        
        END_INTERFACE
    } IUnknownVtbl;

    interface IUnknown
    {
        CONST_VTBL struct IUnknownVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IUnknown_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IUnknown_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IUnknown_Release(This)	\
    (This)->lpVtbl -> Release(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */
#endif 	/* __IUnknown_INTERFACE_DEFINED__ */
#endif /* interface __MIDL_itf_unknwn_0005 */

#ifndef __IVMFVersion_INTERFACE_DEFINED__
#define __IVMFVersion_INTERFACE_DEFINED__
#ifndef _STRUCT_VMF_VERSION_
#define _STRUCT_VMF_VERSION_
	typedef struct
	{
		DWORD dwMajor;
		DWORD dwMinor;
		DWORD dwRelease;
		DWORD dwBuild;
	}VMF_Version;
#endif
EXTERN_GUID(IID_IVMFVersion, 0x31f4691d, 0xec19, 0x47bc, 0xb4, 0xc7, 0x3b, 0xa6, 0x9f, 0x80, 0xda, 0xae);
#if defined(__cplusplus) && !defined(CINTERFACE)
	MIDL_INTERFACE("31F4691D-EC19-47bc-B4C7-3BA69F80DAAE")
	IVMFVersion:public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE GetVersion(VMF_Version *pVersion) = 0;
	};

#else 	/* C style interface */

	typedef struct IVMFVersionVtbl
	{
		BEGIN_INTERFACE

			HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
			IVMFVersion * This,
			/* [in] */ REFIID riid,
			/* [iid_is][out] */ void **ppvObject);

			ULONG ( STDMETHODCALLTYPE *AddRef )( 
				IVMFVersion * This);

			ULONG ( STDMETHODCALLTYPE *Release )( 
				IVMFVersion * This);
			HRESULT (STDMETHODCALLTYPE *GetVersion)(
				IVMFVersion * This,
				VMF_Version *pVersion);

		END_INTERFACE
	} IVMFVersionVtbl;

	interface IVMFVersion
	{
		CONST_VTBL struct IVMFVersionVtbl *lpVtbl;
	};



#ifdef COBJMACROS


#define IVMFVersion_QueryInterface(This,riid,ppvObject)	\
	(This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IVMFVersion_AddRef(This)	\
	(This)->lpVtbl -> AddRef(This)

#define IVMFVersion_Release(This)	\
	(This)->lpVtbl -> Release(This)

#define IVMFVersion_GetVersion(This,pVersion)	\
	(This)->lpVtbl -> GetVersion(This,pVersion)

#endif /* COBJMACROS */
#endif 	/* C style interface */
#endif 	/* __IVMFVersion_INTERFACE_DEFINED__ */ 




#ifndef __IClassFactory_INTERFACE_DEFINED__
#define __IClassFactory_INTERFACE_DEFINED__

/* interface IClassFactory */
/* [unique][uuid][object] */ 

typedef /* [unique] */ IClassFactory *LPCLASSFACTORY;


EXTERN_GUID(IID_IClassFactory,0x00000001,0x0000,0x0000,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46);
#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("00000001-0000-0000-C000-000000000046")
    IClassFactory : public IUnknown
    {
    public:
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE CreateInstance( 
            /* [unique][in] */ IUnknown *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject) = 0;
        
        virtual /* [local] */ HRESULT STDMETHODCALLTYPE LockServer( 
            /* [in] */ BOOL fLock) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IClassFactoryVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IClassFactory * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IClassFactory * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IClassFactory * This);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *CreateInstance )( 
            IClassFactory * This,
            /* [unique][in] */ IUnknown *pUnkOuter,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *LockServer )( 
            IClassFactory * This,
            /* [in] */ BOOL fLock);
        
        END_INTERFACE
    } IClassFactoryVtbl;

    interface IClassFactory
    {
        CONST_VTBL struct IClassFactoryVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IClassFactory_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IClassFactory_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IClassFactory_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IClassFactory_CreateInstance(This,pUnkOuter,riid,ppvObject)	\
    (This)->lpVtbl -> CreateInstance(This,pUnkOuter,riid,ppvObject)

#define IClassFactory_LockServer(This,fLock)	\
    (This)->lpVtbl -> LockServer(This,fLock)

#endif /* COBJMACROS */


#endif 	/* C style interface */
#endif // __IClassFactory_INTERFACE_DEFINED__

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif





