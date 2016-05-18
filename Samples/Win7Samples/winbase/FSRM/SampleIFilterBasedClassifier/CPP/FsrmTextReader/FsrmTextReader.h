

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0553 */
/* at Mon Feb 16 12:34:41 2009
 */
/* Compiler settings for .\FsrmTextReader.idl:
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

#ifndef __FsrmTextReader_h__
#define __FsrmTextReader_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ITextTokenizer_FWD_DEFINED__
#define __ITextTokenizer_FWD_DEFINED__
typedef interface ITextTokenizer ITextTokenizer;
#endif 	/* __ITextTokenizer_FWD_DEFINED__ */


#ifndef __TextTokenizer_FWD_DEFINED__
#define __TextTokenizer_FWD_DEFINED__

#ifdef __cplusplus
typedef class TextTokenizer TextTokenizer;
#else
typedef struct TextTokenizer TextTokenizer;
#endif /* __cplusplus */

#endif 	/* __TextTokenizer_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ITextTokenizer_INTERFACE_DEFINED__
#define __ITextTokenizer_INTERFACE_DEFINED__

/* interface ITextTokenizer */
/* [unique][helpstring][nonextensible][dual][uuid][object] */ 


EXTERN_C const IID IID_ITextTokenizer;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5573E9A1-0D7F-4322-9B11-9CFA47788629")
    ITextTokenizer : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InitializeWithPropertyBag( 
            /* [in] */ IUnknown *propertyBag) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE DoesContainWordsFromList( 
            /* [in] */ SAFEARRAY * pWordList,
            /* [retval][out] */ VARIANT_BOOL *pBooleanResult) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE Cleanup( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITextTokenizerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ITextTokenizer * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ITextTokenizer * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ITextTokenizer * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            ITextTokenizer * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            ITextTokenizer * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            ITextTokenizer * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            ITextTokenizer * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InitializeWithPropertyBag )( 
            ITextTokenizer * This,
            /* [in] */ IUnknown *propertyBag);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *DoesContainWordsFromList )( 
            ITextTokenizer * This,
            /* [in] */ SAFEARRAY * pWordList,
            /* [retval][out] */ VARIANT_BOOL *pBooleanResult);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *Cleanup )( 
            ITextTokenizer * This);
        
        END_INTERFACE
    } ITextTokenizerVtbl;

    interface ITextTokenizer
    {
        CONST_VTBL struct ITextTokenizerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITextTokenizer_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ITextTokenizer_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ITextTokenizer_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ITextTokenizer_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define ITextTokenizer_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define ITextTokenizer_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define ITextTokenizer_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 


#define ITextTokenizer_InitializeWithPropertyBag(This,propertyBag)	\
    ( (This)->lpVtbl -> InitializeWithPropertyBag(This,propertyBag) ) 

#define ITextTokenizer_DoesContainWordsFromList(This,pWordList,pBooleanResult)	\
    ( (This)->lpVtbl -> DoesContainWordsFromList(This,pWordList,pBooleanResult) ) 

#define ITextTokenizer_Cleanup(This)	\
    ( (This)->lpVtbl -> Cleanup(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ITextTokenizer_INTERFACE_DEFINED__ */



#ifndef __FsrmTextReaderLib_LIBRARY_DEFINED__
#define __FsrmTextReaderLib_LIBRARY_DEFINED__

/* library FsrmTextReaderLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_FsrmTextReaderLib;

EXTERN_C const CLSID CLSID_TextTokenizer;

#ifdef __cplusplus

class DECLSPEC_UUID("32AD5F48-C746-44A3-93FC-FEBF03079110")
TextTokenizer;
#endif
#endif /* __FsrmTextReaderLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


