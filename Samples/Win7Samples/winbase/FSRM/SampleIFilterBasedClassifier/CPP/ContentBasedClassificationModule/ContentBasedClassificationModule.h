

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0553 */
/* at Mon Feb 16 16:40:08 2009
 */
/* Compiler settings for .\ContentBasedClassificationModule.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 7.00.0553 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

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

#ifndef __ContentBasedClassificationModule_h__
#define __ContentBasedClassificationModule_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IContentBasedClassifier_FWD_DEFINED__
#define __IContentBasedClassifier_FWD_DEFINED__
typedef interface IContentBasedClassifier IContentBasedClassifier;
#endif 	/* __IContentBasedClassifier_FWD_DEFINED__ */


#ifndef __ContentBasedClassifier_FWD_DEFINED__
#define __ContentBasedClassifier_FWD_DEFINED__

#ifdef __cplusplus
typedef class ContentBasedClassifier ContentBasedClassifier;
#else
typedef struct ContentBasedClassifier ContentBasedClassifier;
#endif /* __cplusplus */

#endif 	/* __ContentBasedClassifier_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IContentBasedClassifier_INTERFACE_DEFINED__
#define __IContentBasedClassifier_INTERFACE_DEFINED__

/* interface IContentBasedClassifier */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_IContentBasedClassifier;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("74614539-C851-4E87-BCAA-01C30246DC6E")
    IContentBasedClassifier : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IContentBasedClassifierVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IContentBasedClassifier * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IContentBasedClassifier * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IContentBasedClassifier * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IContentBasedClassifier * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IContentBasedClassifier * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IContentBasedClassifier * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IContentBasedClassifier * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IContentBasedClassifierVtbl;

    interface IContentBasedClassifier
    {
        CONST_VTBL struct IContentBasedClassifierVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IContentBasedClassifier_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IContentBasedClassifier_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IContentBasedClassifier_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IContentBasedClassifier_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IContentBasedClassifier_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IContentBasedClassifier_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IContentBasedClassifier_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IContentBasedClassifier_INTERFACE_DEFINED__ */



#ifndef __ContentBasedClassificationModuleLib_LIBRARY_DEFINED__
#define __ContentBasedClassificationModuleLib_LIBRARY_DEFINED__

/* library ContentBasedClassificationModuleLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_ContentBasedClassificationModuleLib;

EXTERN_C const CLSID CLSID_ContentBasedClassifier;

#ifdef __cplusplus

class DECLSPEC_UUID("7853D011-8F9D-4831-BA8A-E5EE99B7CC65")
ContentBasedClassifier;
#endif
#endif /* __ContentBasedClassificationModuleLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


