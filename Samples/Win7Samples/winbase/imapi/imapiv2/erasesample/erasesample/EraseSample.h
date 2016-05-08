

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Mon Oct 23 11:04:56 2006
 */
/* Compiler settings for .\EraseSample.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __EraseSample_h__
#define __EraseSample_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IEraseSample_FWD_DEFINED__
#define __IEraseSample_FWD_DEFINED__
typedef interface IEraseSample IEraseSample;
#endif 	/* __IEraseSample_FWD_DEFINED__ */


#ifndef __DEraseSampleEvents_FWD_DEFINED__
#define __DEraseSampleEvents_FWD_DEFINED__
typedef interface DEraseSampleEvents DEraseSampleEvents;
#endif 	/* __DEraseSampleEvents_FWD_DEFINED__ */


#ifndef __MsftEraseSample_FWD_DEFINED__
#define __MsftEraseSample_FWD_DEFINED__

#ifdef __cplusplus
typedef class MsftEraseSample MsftEraseSample;
#else
typedef struct MsftEraseSample MsftEraseSample;
#endif /* __cplusplus */

#endif 	/* __MsftEraseSample_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "imapi2.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_EraseSample_0000_0000 */
/* [local] */ 





extern RPC_IF_HANDLE __MIDL_itf_EraseSample_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EraseSample_0000_0000_v0_0_s_ifspec;

#ifndef __IEraseSample_INTERFACE_DEFINED__
#define __IEraseSample_INTERFACE_DEFINED__

/* interface IEraseSample */
/* [helpstring][unique][dual][nonextensible][uuid][object] */ 


EXTERN_C const IID IID_IEraseSample;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("2A97995D-523A-40cc-AE40-D54EA214BAE5")
    IEraseSample : public IDiscFormat2
    {
    public:
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_Recorder( 
            /* [in] */ IDiscRecorder2 *value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_Recorder( 
            /* [retval][ref][out] */ IDiscRecorder2 **value) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_FullErase( 
            /* [in] */ VARIANT_BOOL value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_FullErase( 
            /* [retval][ref][out] */ VARIANT_BOOL *value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_CurrentPhysicalMediaType( 
            /* [retval][ref][out] */ IMAPI_MEDIA_PHYSICAL_TYPE *value) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_ClientName( 
            /* [in] */ BSTR value) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ClientName( 
            /* [retval][ref][out] */ BSTR *value) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE EraseMedia( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IEraseSampleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IEraseSample * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IEraseSample * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IEraseSample * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IEraseSample * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IEraseSample * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IEraseSample * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IEraseSample * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsRecorderSupported )( 
            IEraseSample * This,
            /* [in] */ IDiscRecorder2 *recorder,
            /* [retval][ref][out] */ VARIANT_BOOL *value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *IsCurrentMediaSupported )( 
            IEraseSample * This,
            /* [in] */ IDiscRecorder2 *recorder,
            /* [retval][ref][out] */ VARIANT_BOOL *value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaPhysicallyBlank )( 
            IEraseSample * This,
            /* [retval][ref][out] */ VARIANT_BOOL *value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_MediaHeuristicallyBlank )( 
            IEraseSample * This,
            /* [retval][ref][out] */ VARIANT_BOOL *value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_SupportedMediaTypes )( 
            IEraseSample * This,
            /* [retval][ref][out] */ SAFEARRAY * *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_Recorder )( 
            IEraseSample * This,
            /* [in] */ IDiscRecorder2 *value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_Recorder )( 
            IEraseSample * This,
            /* [retval][ref][out] */ IDiscRecorder2 **value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_FullErase )( 
            IEraseSample * This,
            /* [in] */ VARIANT_BOOL value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_FullErase )( 
            IEraseSample * This,
            /* [retval][ref][out] */ VARIANT_BOOL *value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_CurrentPhysicalMediaType )( 
            IEraseSample * This,
            /* [retval][ref][out] */ IMAPI_MEDIA_PHYSICAL_TYPE *value);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE *put_ClientName )( 
            IEraseSample * This,
            /* [in] */ BSTR value);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE *get_ClientName )( 
            IEraseSample * This,
            /* [retval][ref][out] */ BSTR *value);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *EraseMedia )( 
            IEraseSample * This);
        
        END_INTERFACE
    } IEraseSampleVtbl;

    interface IEraseSample
    {
        CONST_VTBL struct IEraseSampleVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IEraseSample_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IEraseSample_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IEraseSample_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IEraseSample_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IEraseSample_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IEraseSample_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IEraseSample_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define IEraseSample_IsRecorderSupported(This,recorder,value)	\
    ( (This)->lpVtbl -> IsRecorderSupported(This,recorder,value) ) 

#define IEraseSample_IsCurrentMediaSupported(This,recorder,value)	\
    ( (This)->lpVtbl -> IsCurrentMediaSupported(This,recorder,value) ) 

#define IEraseSample_get_MediaPhysicallyBlank(This,value)	\
    ( (This)->lpVtbl -> get_MediaPhysicallyBlank(This,value) ) 

#define IEraseSample_get_MediaHeuristicallyBlank(This,value)	\
    ( (This)->lpVtbl -> get_MediaHeuristicallyBlank(This,value) ) 

#define IEraseSample_get_SupportedMediaTypes(This,value)	\
    ( (This)->lpVtbl -> get_SupportedMediaTypes(This,value) ) 


#define IEraseSample_put_Recorder(This,value)	\
    ( (This)->lpVtbl -> put_Recorder(This,value) ) 

#define IEraseSample_get_Recorder(This,value)	\
    ( (This)->lpVtbl -> get_Recorder(This,value) ) 

#define IEraseSample_put_FullErase(This,value)	\
    ( (This)->lpVtbl -> put_FullErase(This,value) ) 

#define IEraseSample_get_FullErase(This,value)	\
    ( (This)->lpVtbl -> get_FullErase(This,value) ) 

#define IEraseSample_get_CurrentPhysicalMediaType(This,value)	\
    ( (This)->lpVtbl -> get_CurrentPhysicalMediaType(This,value) ) 

#define IEraseSample_put_ClientName(This,value)	\
    ( (This)->lpVtbl -> put_ClientName(This,value) ) 

#define IEraseSample_get_ClientName(This,value)	\
    ( (This)->lpVtbl -> get_ClientName(This,value) ) 

#define IEraseSample_EraseMedia(This)	\
    ( (This)->lpVtbl -> EraseMedia(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IEraseSample_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_EraseSample_0000_0001 */
/* [local] */ 

#define DISPID_IERASESAMPLEEVENTS_UPDATE 0x200


extern RPC_IF_HANDLE __MIDL_itf_EraseSample_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_EraseSample_0000_0001_v0_0_s_ifspec;

#ifndef __DEraseSampleEvents_INTERFACE_DEFINED__
#define __DEraseSampleEvents_INTERFACE_DEFINED__

/* interface DEraseSampleEvents */
/* [helpstring][unique][oleautomation][nonextensible][uuid][object] */ 


EXTERN_C const IID IID_DEraseSampleEvents;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E2DDFE2A-3BD5-4896-87E2-B6B2F58298E0")
    DEraseSampleEvents : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Update( 
            /* [in] */ IDispatch *object,
            /* [in] */ LONG elapsedSeconds,
            /* [in] */ LONG estimatedTotalSeconds) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct DEraseSampleEventsVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            DEraseSampleEvents * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            DEraseSampleEvents * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            DEraseSampleEvents * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            DEraseSampleEvents * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            DEraseSampleEvents * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            DEraseSampleEvents * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            DEraseSampleEvents * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Update )( 
            DEraseSampleEvents * This,
            /* [in] */ IDispatch *object,
            /* [in] */ LONG elapsedSeconds,
            /* [in] */ LONG estimatedTotalSeconds);
        
        END_INTERFACE
    } DEraseSampleEventsVtbl;

    interface DEraseSampleEvents
    {
        CONST_VTBL struct DEraseSampleEventsVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define DEraseSampleEvents_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define DEraseSampleEvents_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define DEraseSampleEvents_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define DEraseSampleEvents_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define DEraseSampleEvents_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define DEraseSampleEvents_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define DEraseSampleEvents_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define DEraseSampleEvents_Update(This,object,elapsedSeconds,estimatedTotalSeconds)	\
    ( (This)->lpVtbl -> Update(This,object,elapsedSeconds,estimatedTotalSeconds) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __DEraseSampleEvents_INTERFACE_DEFINED__ */



#ifndef __EraseSampleLib_LIBRARY_DEFINED__
#define __EraseSampleLib_LIBRARY_DEFINED__

/* library EraseSampleLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_EraseSampleLib;

EXTERN_C const CLSID CLSID_MsftEraseSample;

#ifdef __cplusplus

class DECLSPEC_UUID("786e85d1-9f56-47ed-9cbb-9be1e9b2b074")
MsftEraseSample;
#endif
#endif /* __EraseSampleLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


