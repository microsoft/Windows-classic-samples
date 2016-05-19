

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* at Thu Apr 19 15:54:48 2007
 */
/* Compiler settings for .\RegisterServer.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
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

#ifndef __RegisterServer_h_h__
#define __RegisterServer_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IDllRegister_FWD_DEFINED__
#define __IDllRegister_FWD_DEFINED__
typedef interface IDllRegister IDllRegister;
#endif 	/* __IDllRegister_FWD_DEFINED__ */


#ifndef __IExeRegister_FWD_DEFINED__
#define __IExeRegister_FWD_DEFINED__
typedef interface IExeRegister IExeRegister;
#endif 	/* __IExeRegister_FWD_DEFINED__ */


#ifndef __RegistrationClass_FWD_DEFINED__
#define __RegistrationClass_FWD_DEFINED__

#ifdef __cplusplus
typedef class RegistrationClass RegistrationClass;
#else
typedef struct RegistrationClass RegistrationClass;
#endif /* __cplusplus */

#endif 	/* __RegistrationClass_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IDllRegister_INTERFACE_DEFINED__
#define __IDllRegister_INTERFACE_DEFINED__

/* interface IDllRegister */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IDllRegister;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D6117008-CCE3-4614-8ACD-769FA5E8B265")
    IDllRegister : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RegisterDll( 
            /* [in] */ BSTR lpFileName) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UnRegisterDll( 
            /* [in] */ BSTR lpFileName) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IDllRegisterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDllRegister * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDllRegister * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDllRegister * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterDll )( 
            IDllRegister * This,
            /* [in] */ BSTR lpFileName);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *UnRegisterDll )( 
            IDllRegister * This,
            /* [in] */ BSTR lpFileName);
        
        END_INTERFACE
    } IDllRegisterVtbl;

    interface IDllRegister
    {
        CONST_VTBL struct IDllRegisterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDllRegister_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IDllRegister_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IDllRegister_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IDllRegister_RegisterDll(This,lpFileName)	\
    ( (This)->lpVtbl -> RegisterDll(This,lpFileName) ) 

#define IDllRegister_UnRegisterDll(This,lpFileName)	\
    ( (This)->lpVtbl -> UnRegisterDll(This,lpFileName) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDllRegister_INTERFACE_DEFINED__ */


#ifndef __IExeRegister_INTERFACE_DEFINED__
#define __IExeRegister_INTERFACE_DEFINED__

/* interface IExeRegister */
/* [helpstring][uuid][object] */ 


EXTERN_C const IID IID_IExeRegister;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("7585D4FA-07BD-4b4e-9AAB-0749C334558B")
    IExeRegister : public IUnknown
    {
    public:
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE RegisterExe( 
            /* [in] */ BSTR lpFileName,
            HWND hwndWindow) = 0;
        
        virtual /* [helpstring] */ HRESULT STDMETHODCALLTYPE UnregisterExe( 
            /* [in] */ BSTR lpFileName,
            HWND hwndWindow) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IExeRegisterVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IExeRegister * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IExeRegister * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IExeRegister * This);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *RegisterExe )( 
            IExeRegister * This,
            /* [in] */ BSTR lpFileName,
            HWND hwndWindow);
        
        /* [helpstring] */ HRESULT ( STDMETHODCALLTYPE *UnregisterExe )( 
            IExeRegister * This,
            /* [in] */ BSTR lpFileName,
            HWND hwndWindow);
        
        END_INTERFACE
    } IExeRegisterVtbl;

    interface IExeRegister
    {
        CONST_VTBL struct IExeRegisterVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IExeRegister_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IExeRegister_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IExeRegister_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IExeRegister_RegisterExe(This,lpFileName,hwndWindow)	\
    ( (This)->lpVtbl -> RegisterExe(This,lpFileName,hwndWindow) ) 

#define IExeRegister_UnregisterExe(This,lpFileName,hwndWindow)	\
    ( (This)->lpVtbl -> UnregisterExe(This,lpFileName,hwndWindow) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IExeRegister_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  HWND_UserSize(     unsigned long *, unsigned long            , HWND * ); 
unsigned char * __RPC_USER  HWND_UserMarshal(  unsigned long *, unsigned char *, HWND * ); 
unsigned char * __RPC_USER  HWND_UserUnmarshal(unsigned long *, unsigned char *, HWND * ); 
void                      __RPC_USER  HWND_UserFree(     unsigned long *, HWND * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


