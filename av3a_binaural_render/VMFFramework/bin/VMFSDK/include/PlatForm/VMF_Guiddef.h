#ifndef __VMF_GUIDDEF_H__
#define __VMF_GUIDDEF_H__

#ifndef __VMF_DEFINE_OLEGUID__
#define __VMF_DEFINE_OLEGUID__
#define DEFINE_OLEGUID(name, l, w1, w2) EXTERN_GUID((name), (l), (w1), (w2), 0xC0,0,0,0,0,0,0,0x46)
#endif

#ifndef __LPGUID_DEFINED__
#define __LPGUID_DEFINED__
typedef GUID *LPGUID;
#endif

#ifndef __LPCGUID_DEFINED__
#define __LPCGUID_DEFINED__
typedef VMF_CONST GUID *LPCGUID;
#endif

#ifndef __IID_DEFINED__
#define __IID_DEFINED__
typedef GUID IID;
typedef IID *LPIID;
#define IID_NULL            GUID_NULL
#define IsEqualIID(riid1, riid2) IsEqualGUID((riid1), (riid2))
typedef GUID CLSID;
typedef CLSID *LPCLSID;
#define CLSID_NULL          GUID_NULL
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID((rclsid1), (rclsid2))
typedef GUID FMTID;
typedef FMTID *LPFMTID;
#define FMTID_NULL          GUID_NULL
#define IsEqualFMTID(rfmtid1, rfmtid2) IsEqualGUID((rfmtid1), (rfmtid2))

#ifdef __midl_proxy
#define __MIDL_CONST
#else
#define __MIDL_CONST VMF_CONST
#endif

#ifndef _REFGUID_DEFINED
#define _REFGUID_DEFINED
#ifdef __cplusplus
#define REFGUID VMF_CONST GUID &
#else
#define REFGUID VMF_CONST GUID * __MIDL_CONST
#endif
#endif

#ifndef _REFIID_DEFINED
#define _REFIID_DEFINED
#ifdef __cplusplus
#define REFIID VMF_CONST IID &
#else
#define REFIID VMF_CONST IID * __MIDL_CONST
#endif
#endif

#ifndef _REFCLSID_DEFINED
#define _REFCLSID_DEFINED
#ifdef __cplusplus
#define REFCLSID VMF_CONST IID &
#else
#define REFCLSID VMF_CONST IID * __MIDL_CONST
#endif
#endif

#ifndef _REFFMTID_DEFINED
#define _REFFMTID_DEFINED
#ifdef __cplusplus
#define REFFMTID VMF_CONST IID &
#else
#define REFFMTID VMF_CONST IID * __MIDL_CONST
#endif
#endif

#endif // !__IID_DEFINED__

#if !defined (_SYS_GUID_OPERATORS_)
#define _SYS_GUID_OPERATORS_
#define IsEqualGUID(rguid1, rguid2) VMF_IsEqualGUID((rguid1), (rguid2))

// Same type, different name

#define IsEqualIID(riid1, riid2) IsEqualGUID((riid1), (riid2))
#define IsEqualCLSID(rclsid1, rclsid2) IsEqualGUID((rclsid1), (rclsid2))


#if !defined _SYS_GUID_OPERATOR_EQ_ && !defined _NO_SYS_GUID_OPERATOR_EQ_
#define _SYS_GUID_OPERATOR_EQ_
// A couple of C++ helpers

#ifdef __cplusplus
__inline int operator==(REFGUID guidOne, REFGUID guidOther)
{
    return IsEqualGUID(guidOne,guidOther);
}

__inline int operator!=(REFGUID guidOne, REFGUID guidOther)
{
    return !(guidOne == guidOther);
}
#endif
#endif  // _SYS_GUID_OPERATOR_EQ_
#endif  // _SYS_GUID_OPERATORS_
#endif  // _GUIDDEF_H_
