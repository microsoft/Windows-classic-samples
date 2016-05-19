

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0493 */
/* at Thu Apr 27 12:56:38 2006
 */
/* Compiler settings for .\DeviceModelPluginSample.idl:
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

#ifndef __DeviceModelPluginSample_h__
#define __DeviceModelPluginSample_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ISampleDeviceModelPrivateInterface_FWD_DEFINED__
#define __ISampleDeviceModelPrivateInterface_FWD_DEFINED__
typedef interface ISampleDeviceModelPrivateInterface ISampleDeviceModelPrivateInterface;
#endif 	/* __ISampleDeviceModelPrivateInterface_FWD_DEFINED__ */


#ifndef __DeviceModelPluginSample_FWD_DEFINED__
#define __DeviceModelPluginSample_FWD_DEFINED__

#ifdef __cplusplus
typedef class DeviceModelPluginSample DeviceModelPluginSample;
#else
typedef struct DeviceModelPluginSample DeviceModelPluginSample;
#endif /* __cplusplus */

#endif 	/* __DeviceModelPluginSample_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "WcsPlugin.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __ISampleDeviceModelPrivateInterface_INTERFACE_DEFINED__
#define __ISampleDeviceModelPrivateInterface_INTERFACE_DEFINED__

/* interface ISampleDeviceModelPrivateInterface */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_ISampleDeviceModelPrivateInterface;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("94617F91-2C14-4485-9B94-A78579B73523")
    ISampleDeviceModelPrivateInterface : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE SamplePrivateMethod( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ISampleDeviceModelPrivateInterfaceVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            ISampleDeviceModelPrivateInterface * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            ISampleDeviceModelPrivateInterface * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            ISampleDeviceModelPrivateInterface * This);
        
        HRESULT ( STDMETHODCALLTYPE *SamplePrivateMethod )( 
            ISampleDeviceModelPrivateInterface * This);
        
        END_INTERFACE
    } ISampleDeviceModelPrivateInterfaceVtbl;

    interface ISampleDeviceModelPrivateInterface
    {
        CONST_VTBL struct ISampleDeviceModelPrivateInterfaceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ISampleDeviceModelPrivateInterface_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define ISampleDeviceModelPrivateInterface_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define ISampleDeviceModelPrivateInterface_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define ISampleDeviceModelPrivateInterface_SamplePrivateMethod(This)	\
    ( (This)->lpVtbl -> SamplePrivateMethod(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __ISampleDeviceModelPrivateInterface_INTERFACE_DEFINED__ */



#ifndef __DeviceModelPluginSampleLib_LIBRARY_DEFINED__
#define __DeviceModelPluginSampleLib_LIBRARY_DEFINED__

/* library DeviceModelPluginSampleLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_DeviceModelPluginSampleLib;

EXTERN_C const CLSID CLSID_DeviceModelPluginSample;

#ifdef __cplusplus

class DECLSPEC_UUID("48D2ACFF-1716-4262-9163-CC8ADB245D58")
DeviceModelPluginSample;
#endif
#endif /* __DeviceModelPluginSampleLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


