//------------------------------------------------------------------------------
// File: fil_data.h
//
// Desc: DirectShow sample code - an MFC based C++ filter mapper application.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

/* this ALWAYS GENERATED file contains the definitions for the interfaces */

/* File created by MIDL compiler version 5.01.0164 */
/* at Sat Oct 23 12:28:23 1999
 */
/* Compiler settings for fil_data.idl:
    Os (OptLev=s), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __fil_data_h__
#define __fil_data_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IAMFilterData_FWD_DEFINED__
#define __IAMFilterData_FWD_DEFINED__
typedef interface IAMFilterData IAMFilterData;
#endif 	/* __IAMFilterData_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "strmif.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_fil_data_0000 */
/* [local] */ 




extern RPC_IF_HANDLE __MIDL_itf_fil_data_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_fil_data_0000_v0_0_s_ifspec;

#ifndef __IAMFilterData_INTERFACE_DEFINED__
#define __IAMFilterData_INTERFACE_DEFINED__

/* interface IAMFilterData */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IAMFilterData;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("97f7c4d4-547b-4a5f-8332-536430ad2e4d")
    IAMFilterData : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ParseFilterData( 
            /* [size_is][in] */ BYTE __RPC_FAR *rgbFilterData,
            /* [in] */ ULONG cb,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE CreateFilterData( 
            /* [in] */ REGFILTER2 __RPC_FAR *prf2,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
            /* [out] */ ULONG __RPC_FAR *pcb) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAMFilterDataVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAMFilterData __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAMFilterData __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAMFilterData __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ParseFilterData )( 
            IAMFilterData __RPC_FAR * This,
            /* [size_is][in] */ BYTE __RPC_FAR *rgbFilterData,
            /* [in] */ ULONG cb,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *CreateFilterData )( 
            IAMFilterData __RPC_FAR * This,
            /* [in] */ REGFILTER2 __RPC_FAR *prf2,
            /* [out] */ BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
            /* [out] */ ULONG __RPC_FAR *pcb);
        
        END_INTERFACE
    } IAMFilterDataVtbl;

    interface IAMFilterData
    {
        CONST_VTBL struct IAMFilterDataVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAMFilterData_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAMFilterData_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAMFilterData_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAMFilterData_ParseFilterData(This,rgbFilterData,cb,prgbRegFilter2)	\
    (This)->lpVtbl -> ParseFilterData(This,rgbFilterData,cb,prgbRegFilter2)

#define IAMFilterData_CreateFilterData(This,prf2,prgbFilterData,pcb)	\
    (This)->lpVtbl -> CreateFilterData(This,prf2,prgbFilterData,pcb)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAMFilterData_ParseFilterData_Proxy( 
    IAMFilterData __RPC_FAR * This,
    /* [size_is][in] */ BYTE __RPC_FAR *rgbFilterData,
    /* [in] */ ULONG cb,
    /* [out] */ BYTE __RPC_FAR *__RPC_FAR *prgbRegFilter2);


void __RPC_STUB IAMFilterData_ParseFilterData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAMFilterData_CreateFilterData_Proxy( 
    IAMFilterData __RPC_FAR * This,
    /* [in] */ REGFILTER2 __RPC_FAR *prf2,
    /* [out] */ BYTE __RPC_FAR *__RPC_FAR *prgbFilterData,
    /* [out] */ ULONG __RPC_FAR *pcb);


void __RPC_STUB IAMFilterData_CreateFilterData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAMFilterData_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
