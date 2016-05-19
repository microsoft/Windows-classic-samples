/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Thu May 20 18:03:31 1999
 */
/* Compiler settings for DTMCommon.idl:
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

#ifndef __DTMCommon_h__
#define __DTMCommon_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __ILTMSink_FWD_DEFINED__
#define __ILTMSink_FWD_DEFINED__
typedef interface ILTMSink ILTMSink;
#endif 	/* __ILTMSink_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"
#include "ModuleCore.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ILTMSink_INTERFACE_DEFINED__
#define __ILTMSink_INTERFACE_DEFINED__

/* interface ILTMSink */
/* [uuid][object] */ 


EXTERN_C const IID IID_ILTMSink;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("1D9D6DF4-5DCC-11d0-82B1-00C04FC2CABA")
    ILTMSink : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE NotifyText( 
            /* [string][in] */ BSTR __MIDL_0010) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ILTMSinkVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ILTMSink __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ILTMSink __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ILTMSink __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *NotifyText )( 
            ILTMSink __RPC_FAR * This,
            /* [string][in] */ BSTR __MIDL_0010);
        
        END_INTERFACE
    } ILTMSinkVtbl;

    interface ILTMSink
    {
        CONST_VTBL struct ILTMSinkVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ILTMSink_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ILTMSink_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ILTMSink_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ILTMSink_NotifyText(This,__MIDL_0010)	\
    (This)->lpVtbl -> NotifyText(This,__MIDL_0010)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ILTMSink_NotifyText_Proxy( 
    ILTMSink __RPC_FAR * This,
    /* [string][in] */ BSTR __MIDL_0010);


void __RPC_STUB ILTMSink_NotifyText_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ILTMSink_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long __RPC_FAR *, unsigned long            , BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserMarshal(  unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
unsigned char __RPC_FAR * __RPC_USER  BSTR_UserUnmarshal(unsigned long __RPC_FAR *, unsigned char __RPC_FAR *, BSTR __RPC_FAR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long __RPC_FAR *, BSTR __RPC_FAR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
