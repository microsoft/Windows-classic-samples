#include "rpc.h"
#include "rpcndr.h"

#ifndef __svcprov_h__
#define __svcprov_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IProvideDBService_FWD_DEFINED__
#define __IProvideDBService_FWD_DEFINED__
typedef interface IProvideDBService IProvideDBService;
#endif 	/* __IProvideDBService_FWD_DEFINED__ */

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IProvideDBService_INTERFACE_DEFINED__
#define __IProvideDBService_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IProvideDBService
 * at Mon Apr 22 16:40:51 1996
 * using MIDL 2.00.72
 ****************************************/
/* [unique][uuid][object][local] */ 

// @msg IID_IProvideDBService | {EFF65380-9C98-11CF-B963-00AA0044773D}
DEFINE_GUID(IID_IProvideDBService, 0xEFF65380L,0x9C98,0x11CF,0xB9,0x63,0x00,0xAA,0x00,0x44,0x77,0x3D);

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IProvideDBService : public IUnknown
    {
    public:
        virtual HRESULT __stdcall ProvideService( 
            /* [in] */ ULONG cProvidedPropertySets,
            /* [size_is][in] */ DBPROPSET __RPC_FAR rgProvidedPropertySets[  ],
            /* [in] */ ULONG cRequestedPropertySets,
            /* [size_is][in] */ DBPROPSET __RPC_FAR rgRequestedPropertySets[  ],
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ IUnknown __RPC_FAR *pDataProvider,
            /* [in] */ REFIID riidRequested,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppServiceProvider) = 0;
        
    };
    
#else 	/* C style interface */
    
    typedef struct IProvideDBServiceVtbl
    {
        
        HRESULT ( __stdcall __RPC_FAR *QueryInterface )( 
            IProvideDBService __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( __stdcall __RPC_FAR *AddRef )( 
            IProvideDBService __RPC_FAR * This);
        
        ULONG ( __stdcall __RPC_FAR *Release )( 
            IProvideDBService __RPC_FAR * This);
        
        HRESULT ( __stdcall __RPC_FAR *ProvideService )( 
            IProvideDBService __RPC_FAR * This,
            /* [in] */ ULONG cProvidedPropertySets,
            /* [size_is][in] */ DBPROPSET __RPC_FAR rgProvidedPropertySets[  ],
            /* [in] */ ULONG cRequestedPropertySets,
            /* [size_is][in] */ DBPROPSET __RPC_FAR rgRequestedPropertySets[  ],
            /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
            /* [in] */ IUnknown __RPC_FAR *pDataProvider,
            /* [in] */ REFIID riidRequested,
            /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppServiceProvider);
        
    } IProvideDBServiceVtbl;
    
    interface IProvideDBService
    {
        CONST_VTBL struct IProvideDBServiceVtbl __RPC_FAR *lpVtbl;
    };
    
    

#ifdef COBJMACROS


#define IProvideDBService_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProvideDBService_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProvideDBService_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProvideDBService_ProvideService(This,cProvidedProperties,rgProvidedProperties,cRequestedProperties,rgRequestedProperties,pUnkOuter,pDataProvider,riidRequested,ppServiceProvider)	\
    (This)->lpVtbl -> ProvideService(This,cProvidedProperties,rgProvidedProperties,cRequestedProperties,rgRequestedProperties,pUnkOuter,pDataProvider,riidRequested,ppServiceProvider)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT __stdcall IProvideDBService_ProvideService_Proxy( 
    IProvideDBService __RPC_FAR * This,
    /* [in] */ ULONG cProvidedPropertySets,
    /* [size_is][in] */ DBPROPSET __RPC_FAR rgProvidedPropertySets[  ],
    /* [in] */ ULONG cRequestedPropertySets,
    /* [size_is][in] */ DBPROPSET __RPC_FAR rgRequestedPropertySets[  ],
    /* [in] */ IUnknown __RPC_FAR *pUnkOuter,
    /* [in] */ IUnknown __RPC_FAR *pDataProvider,
    /* [in] */ REFIID riidRequested,
    /* [iid_is][out] */ IUnknown __RPC_FAR *__RPC_FAR *ppServiceProvider);



void __RPC_STUB IProvideDBService_ProvideService_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProvideDBService_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
