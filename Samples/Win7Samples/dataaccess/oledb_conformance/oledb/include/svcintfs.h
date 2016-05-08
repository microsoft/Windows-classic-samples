/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0158 */
/* at Thu Feb 04 12:59:47 1999
 */
/* Compiler settings for svcintfs.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


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

#ifndef __svcintfs_h__
#define __svcintfs_h__

#ifdef __cplusplus
extern "C"{
#endif 

#ifndef __IDispenserManagerShutdownGuarantee_FWD_DEFINED__
#define __IDispenserManagerShutdownGuarantee_FWD_DEFINED__
typedef interface IDispenserManagerShutdownGuarantee IDispenserManagerShutdownGuarantee;
#endif 	/* __IDispenserManagerShutdownGuarantee_FWD_DEFINED__ */


/* interface __MIDL_itf_svcintfs_0500 */
/* [local] */ 

//
// IDispenserManagerShutdownGuarantee
// Implemented by Dispenser Manager, called by Dispensers wondering if Dispenser Manager
// guarantees clean shutdown of Dispensers at end of process.
//


extern RPC_IF_HANDLE __MIDL_itf_svcintfs_0500_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_svcintfs_0500_v0_0_s_ifspec;

#ifndef __IDispenserManagerShutdownGuarantee_INTERFACE_DEFINED__
#define __IDispenserManagerShutdownGuarantee_INTERFACE_DEFINED__

/* interface IDispenserManagerShutdownGuarantee */
/* [unique][hidden][local][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDispenserManagerShutdownGuarantee;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("5cb31e11-2b5f-11cf-be10-00aa00a2fa25")
    IDispenserManagerShutdownGuarantee : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE ShutdownGuarantee( 
            /* [out] */ BOOL __RPC_FAR *pfShutdownIsGuaranteed) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDispenserManagerShutdownGuaranteeVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDispenserManagerShutdownGuarantee __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDispenserManagerShutdownGuarantee __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDispenserManagerShutdownGuarantee __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ShutdownGuarantee )( 
            IDispenserManagerShutdownGuarantee __RPC_FAR * This,
            /* [out] */ BOOL __RPC_FAR *pfShutdownIsGuaranteed);
        
        END_INTERFACE
    } IDispenserManagerShutdownGuaranteeVtbl;

    interface IDispenserManagerShutdownGuarantee
    {
        CONST_VTBL struct IDispenserManagerShutdownGuaranteeVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDispenserManagerShutdownGuarantee_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDispenserManagerShutdownGuarantee_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDispenserManagerShutdownGuarantee_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDispenserManagerShutdownGuarantee_ShutdownGuarantee(This,pfShutdownIsGuaranteed)	\
    (This)->lpVtbl -> ShutdownGuarantee(This,pfShutdownIsGuaranteed)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IDispenserManagerShutdownGuarantee_ShutdownGuarantee_Proxy( 
    IDispenserManagerShutdownGuarantee __RPC_FAR * This,
    /* [out] */ BOOL __RPC_FAR *pfShutdownIsGuaranteed);


void __RPC_STUB IDispenserManagerShutdownGuarantee_ShutdownGuarantee_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IDispenserManagerShutdownGuarantee_INTERFACE_DEFINED__ */



#ifdef __cplusplus
}
#endif

#endif
