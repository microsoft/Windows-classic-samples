/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Jan 22 16:50:24 2001
 */
/* Compiler settings for ModuleCore.idl:
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

#ifndef __ModuleCore_h__
#define __ModuleCore_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IError_FWD_DEFINED__
#define __IError_FWD_DEFINED__
typedef interface IError IError;
#endif 	/* __IError_FWD_DEFINED__ */


#ifndef __ITestConsole_FWD_DEFINED__
#define __ITestConsole_FWD_DEFINED__
typedef interface ITestConsole ITestConsole;
#endif 	/* __ITestConsole_FWD_DEFINED__ */


#ifndef __IProviderInfo_FWD_DEFINED__
#define __IProviderInfo_FWD_DEFINED__
typedef interface IProviderInfo IProviderInfo;
#endif 	/* __IProviderInfo_FWD_DEFINED__ */


#ifndef __IAliasInfo_FWD_DEFINED__
#define __IAliasInfo_FWD_DEFINED__
typedef interface IAliasInfo IAliasInfo;
#endif 	/* __IAliasInfo_FWD_DEFINED__ */


#ifndef __IError_FWD_DEFINED__
#define __IError_FWD_DEFINED__
typedef interface IError IError;
#endif 	/* __IError_FWD_DEFINED__ */


#ifndef __ITestConsole_FWD_DEFINED__
#define __ITestConsole_FWD_DEFINED__
typedef interface ITestConsole ITestConsole;
#endif 	/* __ITestConsole_FWD_DEFINED__ */


#ifndef __IProviderInfo_FWD_DEFINED__
#define __IProviderInfo_FWD_DEFINED__
typedef interface IProviderInfo IProviderInfo;
#endif 	/* __IProviderInfo_FWD_DEFINED__ */


#ifndef __IAliasInfo_FWD_DEFINED__
#define __IAliasInfo_FWD_DEFINED__
typedef interface IAliasInfo IAliasInfo;
#endif 	/* __IAliasInfo_FWD_DEFINED__ */


#ifndef __ITestCases_FWD_DEFINED__
#define __ITestCases_FWD_DEFINED__
typedef interface ITestCases ITestCases;
#endif 	/* __ITestCases_FWD_DEFINED__ */


#ifndef __ITestModule_FWD_DEFINED__
#define __ITestModule_FWD_DEFINED__
typedef interface ITestModule ITestModule;
#endif 	/* __ITestModule_FWD_DEFINED__ */


#ifndef __ITestCases_FWD_DEFINED__
#define __ITestCases_FWD_DEFINED__
typedef interface ITestCases ITestCases;
#endif 	/* __ITestCases_FWD_DEFINED__ */


#ifndef __ITestModule_FWD_DEFINED__
#define __ITestModule_FWD_DEFINED__
typedef interface ITestModule ITestModule;
#endif 	/* __ITestModule_FWD_DEFINED__ */


/* header files for imported files */
#include "unknwn.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

/* interface __MIDL_itf_ModuleCore_0000 */
/* [local] */ 

typedef 
enum tagVARIATION_STATUS
    {	eVariationStatusFailed	= 0,
	eVariationStatusPassed	= eVariationStatusFailed + 1,
	eVariationStatusNotRun	= eVariationStatusPassed + 1,
	eVariationStatusNonExistent	= eVariationStatusNotRun + 1,
	eVariationStatusUnknown	= eVariationStatusNonExistent + 1,
	eVariationStatusTimedOut	= eVariationStatusUnknown + 1,
	eVariationStatusConformanceWarning	= eVariationStatusTimedOut + 1,
	eVariationStatusException	= eVariationStatusConformanceWarning + 1,
	eVariationStatusAborted	= eVariationStatusException + 1
    }	VARIATION_STATUS;

typedef 
enum tagERRORLEVEL
    {	HR_STRICT	= 0,
	HR_OPTIONAL	= HR_STRICT + 1,
	HR_SUCCEED	= HR_OPTIONAL + 1,
	HR_FAIL	= HR_SUCCEED + 1,
	HR_WARNING	= HR_FAIL + 1
    }	ERRORLEVEL;



extern RPC_IF_HANDLE __MIDL_itf_ModuleCore_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ModuleCore_0000_v0_0_s_ifspec;

#ifndef __IError_INTERFACE_DEFINED__
#define __IError_INTERFACE_DEFINED__

/* interface IError */
/* [helpstring][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_IError;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0C3FEC1-8FBB-11d0-98CE-444553540000")
    IError : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetErrorLevel( 
            /* [retval][out] */ ERRORLEVEL __RPC_FAR *pErrorLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetErrorLevel( 
            /* [in] */ ERRORLEVEL ErrorLevel) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetActualHr( 
            /* [retval][out] */ LONG __RPC_FAR *phrActual) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Validate( 
            /* [in] */ LONG hrActual,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ LONG lLineNo,
            /* [in] */ LONG hrExpected,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Compare( 
            /* [in] */ VARIANT_BOOL fWereEqual,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ LONG lLineNo,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfWereEqual) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogExpectedHr( 
            /* [in] */ LONG hrExpected) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE LogReceivedHr( 
            /* [in] */ LONG hrReceived,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ LONG lLineNo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetModErrors( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetCaseErrors( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetVarErrors( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetModWarnings( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetCaseWarnings( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ResetVarWarnings( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModErrors( 
            /* [retval][out] */ LONG __RPC_FAR *plModErrors) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCaseErrors( 
            /* [retval][out] */ LONG __RPC_FAR *plCaseErrors) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVarErrors( 
            /* [retval][out] */ LONG __RPC_FAR *plVarErrors) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetModWarnings( 
            /* [retval][out] */ LONG __RPC_FAR *plModWarnings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCaseWarnings( 
            /* [retval][out] */ LONG __RPC_FAR *plCaseWarnings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVarWarnings( 
            /* [retval][out] */ LONG __RPC_FAR *plVarWarnings) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Increment( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Transmit( 
            /* [in] */ BSTR bstrTextString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Initialize( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IErrorVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IError __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IError __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorLevel )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ ERRORLEVEL __RPC_FAR *pErrorLevel);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetErrorLevel )( 
            IError __RPC_FAR * This,
            /* [in] */ ERRORLEVEL ErrorLevel);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetActualHr )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *phrActual);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Validate )( 
            IError __RPC_FAR * This,
            /* [in] */ LONG hrActual,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ LONG lLineNo,
            /* [in] */ LONG hrExpected,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Compare )( 
            IError __RPC_FAR * This,
            /* [in] */ VARIANT_BOOL fWereEqual,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ LONG lLineNo,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfWereEqual);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogExpectedHr )( 
            IError __RPC_FAR * This,
            /* [in] */ LONG hrExpected);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *LogReceivedHr )( 
            IError __RPC_FAR * This,
            /* [in] */ LONG hrReceived,
            /* [in] */ BSTR bstrFileName,
            /* [in] */ LONG lLineNo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetModErrors )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetCaseErrors )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetVarErrors )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetModWarnings )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetCaseWarnings )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ResetVarWarnings )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetModErrors )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plModErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCaseErrors )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plCaseErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVarErrors )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plVarErrors);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetModWarnings )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plModWarnings);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCaseWarnings )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plCaseWarnings);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVarWarnings )( 
            IError __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plVarWarnings);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Increment )( 
            IError __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Transmit )( 
            IError __RPC_FAR * This,
            /* [in] */ BSTR bstrTextString);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Initialize )( 
            IError __RPC_FAR * This);
        
        END_INTERFACE
    } IErrorVtbl;

    interface IError
    {
        CONST_VTBL struct IErrorVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IError_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IError_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IError_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IError_GetErrorLevel(This,pErrorLevel)	\
    (This)->lpVtbl -> GetErrorLevel(This,pErrorLevel)

#define IError_SetErrorLevel(This,ErrorLevel)	\
    (This)->lpVtbl -> SetErrorLevel(This,ErrorLevel)

#define IError_GetActualHr(This,phrActual)	\
    (This)->lpVtbl -> GetActualHr(This,phrActual)

#define IError_Validate(This,hrActual,bstrFileName,lLineNo,hrExpected,pfResult)	\
    (This)->lpVtbl -> Validate(This,hrActual,bstrFileName,lLineNo,hrExpected,pfResult)

#define IError_Compare(This,fWereEqual,bstrFileName,lLineNo,pfWereEqual)	\
    (This)->lpVtbl -> Compare(This,fWereEqual,bstrFileName,lLineNo,pfWereEqual)

#define IError_LogExpectedHr(This,hrExpected)	\
    (This)->lpVtbl -> LogExpectedHr(This,hrExpected)

#define IError_LogReceivedHr(This,hrReceived,bstrFileName,lLineNo)	\
    (This)->lpVtbl -> LogReceivedHr(This,hrReceived,bstrFileName,lLineNo)

#define IError_ResetModErrors(This)	\
    (This)->lpVtbl -> ResetModErrors(This)

#define IError_ResetCaseErrors(This)	\
    (This)->lpVtbl -> ResetCaseErrors(This)

#define IError_ResetVarErrors(This)	\
    (This)->lpVtbl -> ResetVarErrors(This)

#define IError_ResetModWarnings(This)	\
    (This)->lpVtbl -> ResetModWarnings(This)

#define IError_ResetCaseWarnings(This)	\
    (This)->lpVtbl -> ResetCaseWarnings(This)

#define IError_ResetVarWarnings(This)	\
    (This)->lpVtbl -> ResetVarWarnings(This)

#define IError_GetModErrors(This,plModErrors)	\
    (This)->lpVtbl -> GetModErrors(This,plModErrors)

#define IError_GetCaseErrors(This,plCaseErrors)	\
    (This)->lpVtbl -> GetCaseErrors(This,plCaseErrors)

#define IError_GetVarErrors(This,plVarErrors)	\
    (This)->lpVtbl -> GetVarErrors(This,plVarErrors)

#define IError_GetModWarnings(This,plModWarnings)	\
    (This)->lpVtbl -> GetModWarnings(This,plModWarnings)

#define IError_GetCaseWarnings(This,plCaseWarnings)	\
    (This)->lpVtbl -> GetCaseWarnings(This,plCaseWarnings)

#define IError_GetVarWarnings(This,plVarWarnings)	\
    (This)->lpVtbl -> GetVarWarnings(This,plVarWarnings)

#define IError_Increment(This)	\
    (This)->lpVtbl -> Increment(This)

#define IError_Transmit(This,bstrTextString)	\
    (This)->lpVtbl -> Transmit(This,bstrTextString)

#define IError_Initialize(This)	\
    (This)->lpVtbl -> Initialize(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IError_GetErrorLevel_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ ERRORLEVEL __RPC_FAR *pErrorLevel);


void __RPC_STUB IError_GetErrorLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_SetErrorLevel_Proxy( 
    IError __RPC_FAR * This,
    /* [in] */ ERRORLEVEL ErrorLevel);


void __RPC_STUB IError_SetErrorLevel_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetActualHr_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *phrActual);


void __RPC_STUB IError_GetActualHr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_Validate_Proxy( 
    IError __RPC_FAR * This,
    /* [in] */ LONG hrActual,
    /* [in] */ BSTR bstrFileName,
    /* [in] */ LONG lLineNo,
    /* [in] */ LONG hrExpected,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult);


void __RPC_STUB IError_Validate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_Compare_Proxy( 
    IError __RPC_FAR * This,
    /* [in] */ VARIANT_BOOL fWereEqual,
    /* [in] */ BSTR bstrFileName,
    /* [in] */ LONG lLineNo,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfWereEqual);


void __RPC_STUB IError_Compare_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_LogExpectedHr_Proxy( 
    IError __RPC_FAR * This,
    /* [in] */ LONG hrExpected);


void __RPC_STUB IError_LogExpectedHr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_LogReceivedHr_Proxy( 
    IError __RPC_FAR * This,
    /* [in] */ LONG hrReceived,
    /* [in] */ BSTR bstrFileName,
    /* [in] */ LONG lLineNo);


void __RPC_STUB IError_LogReceivedHr_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_ResetModErrors_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_ResetModErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_ResetCaseErrors_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_ResetCaseErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_ResetVarErrors_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_ResetVarErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_ResetModWarnings_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_ResetModWarnings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_ResetCaseWarnings_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_ResetCaseWarnings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_ResetVarWarnings_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_ResetVarWarnings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetModErrors_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plModErrors);


void __RPC_STUB IError_GetModErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetCaseErrors_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plCaseErrors);


void __RPC_STUB IError_GetCaseErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetVarErrors_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plVarErrors);


void __RPC_STUB IError_GetVarErrors_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetModWarnings_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plModWarnings);


void __RPC_STUB IError_GetModWarnings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetCaseWarnings_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plCaseWarnings);


void __RPC_STUB IError_GetCaseWarnings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_GetVarWarnings_Proxy( 
    IError __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plVarWarnings);


void __RPC_STUB IError_GetVarWarnings_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_Increment_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_Increment_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_Transmit_Proxy( 
    IError __RPC_FAR * This,
    /* [in] */ BSTR bstrTextString);


void __RPC_STUB IError_Transmit_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IError_Initialize_Proxy( 
    IError __RPC_FAR * This);


void __RPC_STUB IError_Initialize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IError_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_ModuleCore_0009 */
/* [local] */ 

typedef 
enum tagCONSOLEFLAGS
    {	CONSOLE_RAW	= 0,
	CONSOLE_TEXT	= 0x1,
	CONSOLE_XML	= 0x2 | CONSOLE_TEXT
    }	CONSOLEFLAGS;



extern RPC_IF_HANDLE __MIDL_itf_ModuleCore_0009_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ModuleCore_0009_v0_0_s_ifspec;

#ifndef __ITestConsole_INTERFACE_DEFINED__
#define __ITestConsole_INTERFACE_DEFINED__

/* interface ITestConsole */
/* [helpstring][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ITestConsole;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("F2B528DD-15AC-41d4-B582-C0DAA1322CE3")
    ITestConsole : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Log( 
            /* [in] */ BSTR bstrActual,
            /* [in] */ BSTR bstrExpected,
            /* [in] */ BSTR bstrSource,
            /* [in] */ BSTR bstrMessage,
            /* [in] */ BSTR bstrDetails,
            /* [in] */ CONSOLEFLAGS flags,
            /* [in] */ BSTR bstrFilename,
            /* [in] */ LONG iline) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Write( 
            /* [in] */ CONSOLEFLAGS flags,
            /* [in] */ BSTR bstrString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE WriteLine( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITestConsoleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITestConsole __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITestConsole __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITestConsole __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Log )( 
            ITestConsole __RPC_FAR * This,
            /* [in] */ BSTR bstrActual,
            /* [in] */ BSTR bstrExpected,
            /* [in] */ BSTR bstrSource,
            /* [in] */ BSTR bstrMessage,
            /* [in] */ BSTR bstrDetails,
            /* [in] */ CONSOLEFLAGS flags,
            /* [in] */ BSTR bstrFilename,
            /* [in] */ LONG iline);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Write )( 
            ITestConsole __RPC_FAR * This,
            /* [in] */ CONSOLEFLAGS flags,
            /* [in] */ BSTR bstrString);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *WriteLine )( 
            ITestConsole __RPC_FAR * This);
        
        END_INTERFACE
    } ITestConsoleVtbl;

    interface ITestConsole
    {
        CONST_VTBL struct ITestConsoleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITestConsole_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITestConsole_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITestConsole_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITestConsole_Log(This,bstrActual,bstrExpected,bstrSource,bstrMessage,bstrDetails,flags,bstrFilename,iline)	\
    (This)->lpVtbl -> Log(This,bstrActual,bstrExpected,bstrSource,bstrMessage,bstrDetails,flags,bstrFilename,iline)

#define ITestConsole_Write(This,flags,bstrString)	\
    (This)->lpVtbl -> Write(This,flags,bstrString)

#define ITestConsole_WriteLine(This)	\
    (This)->lpVtbl -> WriteLine(This)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITestConsole_Log_Proxy( 
    ITestConsole __RPC_FAR * This,
    /* [in] */ BSTR bstrActual,
    /* [in] */ BSTR bstrExpected,
    /* [in] */ BSTR bstrSource,
    /* [in] */ BSTR bstrMessage,
    /* [in] */ BSTR bstrDetails,
    /* [in] */ CONSOLEFLAGS flags,
    /* [in] */ BSTR bstrFilename,
    /* [in] */ LONG iline);


void __RPC_STUB ITestConsole_Log_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestConsole_Write_Proxy( 
    ITestConsole __RPC_FAR * This,
    /* [in] */ CONSOLEFLAGS flags,
    /* [in] */ BSTR bstrString);


void __RPC_STUB ITestConsole_Write_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestConsole_WriteLine_Proxy( 
    ITestConsole __RPC_FAR * This);


void __RPC_STUB ITestConsole_WriteLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITestConsole_INTERFACE_DEFINED__ */


#ifndef __IProviderInfo_INTERFACE_DEFINED__
#define __IProviderInfo_INTERFACE_DEFINED__

/* interface IProviderInfo */
/* [oleautomation][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IProviderInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0C3FEC2-8FBB-11d0-98CE-444553540000")
    IProviderInfo : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrProviderName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetName( 
            /* [in] */ BSTR bstrProviderName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetFriendlyName( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFriendlyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetFriendlyName( 
            /* [in] */ BSTR bstrFriendlyName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetInitString( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrInitString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetInitString( 
            /* [in] */ BSTR bstrInitString) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetMachineName( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetMachineName( 
            /* [in] */ BSTR bstrMachineName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCLSID( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCLSID( 
            /* [in] */ BSTR bstrCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCLSCTX( 
            /* [retval][out] */ LONG __RPC_FAR *pClsCtx) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetCLSCTX( 
            /* [in] */ LONG ClsCtx) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IProviderInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IProviderInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IProviderInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IProviderInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrProviderName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrProviderName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFriendlyName )( 
            IProviderInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFriendlyName )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInitString )( 
            IProviderInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrInitString);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInitString )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrInitString);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMachineName )( 
            IProviderInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMachineName )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCLSID )( 
            IProviderInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCLSID )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrCLSID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCLSCTX )( 
            IProviderInfo __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pClsCtx);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCLSCTX )( 
            IProviderInfo __RPC_FAR * This,
            /* [in] */ LONG ClsCtx);
        
        END_INTERFACE
    } IProviderInfoVtbl;

    interface IProviderInfo
    {
        CONST_VTBL struct IProviderInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IProviderInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IProviderInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IProviderInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IProviderInfo_GetName(This,pbstrProviderName)	\
    (This)->lpVtbl -> GetName(This,pbstrProviderName)

#define IProviderInfo_SetName(This,bstrProviderName)	\
    (This)->lpVtbl -> SetName(This,bstrProviderName)

#define IProviderInfo_GetFriendlyName(This,pbstrFriendlyName)	\
    (This)->lpVtbl -> GetFriendlyName(This,pbstrFriendlyName)

#define IProviderInfo_SetFriendlyName(This,bstrFriendlyName)	\
    (This)->lpVtbl -> SetFriendlyName(This,bstrFriendlyName)

#define IProviderInfo_GetInitString(This,pbstrInitString)	\
    (This)->lpVtbl -> GetInitString(This,pbstrInitString)

#define IProviderInfo_SetInitString(This,bstrInitString)	\
    (This)->lpVtbl -> SetInitString(This,bstrInitString)

#define IProviderInfo_GetMachineName(This,pbstrMachineName)	\
    (This)->lpVtbl -> GetMachineName(This,pbstrMachineName)

#define IProviderInfo_SetMachineName(This,bstrMachineName)	\
    (This)->lpVtbl -> SetMachineName(This,bstrMachineName)

#define IProviderInfo_GetCLSID(This,pbstrCLSID)	\
    (This)->lpVtbl -> GetCLSID(This,pbstrCLSID)

#define IProviderInfo_SetCLSID(This,bstrCLSID)	\
    (This)->lpVtbl -> SetCLSID(This,bstrCLSID)

#define IProviderInfo_GetCLSCTX(This,pClsCtx)	\
    (This)->lpVtbl -> GetCLSCTX(This,pClsCtx)

#define IProviderInfo_SetCLSCTX(This,ClsCtx)	\
    (This)->lpVtbl -> SetCLSCTX(This,ClsCtx)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IProviderInfo_GetName_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrProviderName);


void __RPC_STUB IProviderInfo_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_SetName_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [in] */ BSTR bstrProviderName);


void __RPC_STUB IProviderInfo_SetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_GetFriendlyName_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrFriendlyName);


void __RPC_STUB IProviderInfo_GetFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_SetFriendlyName_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [in] */ BSTR bstrFriendlyName);


void __RPC_STUB IProviderInfo_SetFriendlyName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_GetInitString_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrInitString);


void __RPC_STUB IProviderInfo_GetInitString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_SetInitString_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [in] */ BSTR bstrInitString);


void __RPC_STUB IProviderInfo_SetInitString_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_GetMachineName_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrMachineName);


void __RPC_STUB IProviderInfo_GetMachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_SetMachineName_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [in] */ BSTR bstrMachineName);


void __RPC_STUB IProviderInfo_SetMachineName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_GetCLSID_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID);


void __RPC_STUB IProviderInfo_GetCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_SetCLSID_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [in] */ BSTR bstrCLSID);


void __RPC_STUB IProviderInfo_SetCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_GetCLSCTX_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *pClsCtx);


void __RPC_STUB IProviderInfo_GetCLSCTX_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IProviderInfo_SetCLSCTX_Proxy( 
    IProviderInfo __RPC_FAR * This,
    /* [in] */ LONG ClsCtx);


void __RPC_STUB IProviderInfo_SetCLSCTX_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IProviderInfo_INTERFACE_DEFINED__ */


#ifndef __IAliasInfo_INTERFACE_DEFINED__
#define __IAliasInfo_INTERFACE_DEFINED__

/* interface IAliasInfo */
/* [oleautomation][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IAliasInfo;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("34ADBB62-E4AD-41e4-AC6B-5ED539CD63D6")
    IAliasInfo : public IProviderInfo
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetCommandLine( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCmdLine) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAliasInfoVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAliasInfo __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAliasInfo __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrProviderName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetName )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrProviderName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetFriendlyName )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetFriendlyName )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrFriendlyName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetInitString )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrInitString);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetInitString )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrInitString);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetMachineName )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetMachineName )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrMachineName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCLSID )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCLSID )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ BSTR bstrCLSID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCLSCTX )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *pClsCtx);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetCLSCTX )( 
            IAliasInfo __RPC_FAR * This,
            /* [in] */ LONG ClsCtx);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCommandLine )( 
            IAliasInfo __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCmdLine);
        
        END_INTERFACE
    } IAliasInfoVtbl;

    interface IAliasInfo
    {
        CONST_VTBL struct IAliasInfoVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAliasInfo_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAliasInfo_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAliasInfo_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAliasInfo_GetName(This,pbstrProviderName)	\
    (This)->lpVtbl -> GetName(This,pbstrProviderName)

#define IAliasInfo_SetName(This,bstrProviderName)	\
    (This)->lpVtbl -> SetName(This,bstrProviderName)

#define IAliasInfo_GetFriendlyName(This,pbstrFriendlyName)	\
    (This)->lpVtbl -> GetFriendlyName(This,pbstrFriendlyName)

#define IAliasInfo_SetFriendlyName(This,bstrFriendlyName)	\
    (This)->lpVtbl -> SetFriendlyName(This,bstrFriendlyName)

#define IAliasInfo_GetInitString(This,pbstrInitString)	\
    (This)->lpVtbl -> GetInitString(This,pbstrInitString)

#define IAliasInfo_SetInitString(This,bstrInitString)	\
    (This)->lpVtbl -> SetInitString(This,bstrInitString)

#define IAliasInfo_GetMachineName(This,pbstrMachineName)	\
    (This)->lpVtbl -> GetMachineName(This,pbstrMachineName)

#define IAliasInfo_SetMachineName(This,bstrMachineName)	\
    (This)->lpVtbl -> SetMachineName(This,bstrMachineName)

#define IAliasInfo_GetCLSID(This,pbstrCLSID)	\
    (This)->lpVtbl -> GetCLSID(This,pbstrCLSID)

#define IAliasInfo_SetCLSID(This,bstrCLSID)	\
    (This)->lpVtbl -> SetCLSID(This,bstrCLSID)

#define IAliasInfo_GetCLSCTX(This,pClsCtx)	\
    (This)->lpVtbl -> GetCLSCTX(This,pClsCtx)

#define IAliasInfo_SetCLSCTX(This,ClsCtx)	\
    (This)->lpVtbl -> SetCLSCTX(This,ClsCtx)


#define IAliasInfo_GetCommandLine(This,pbstrCmdLine)	\
    (This)->lpVtbl -> GetCommandLine(This,pbstrCmdLine)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAliasInfo_GetCommandLine_Proxy( 
    IAliasInfo __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrCmdLine);


void __RPC_STUB IAliasInfo_GetCommandLine_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAliasInfo_INTERFACE_DEFINED__ */



#ifndef __ModuleBase_LIBRARY_DEFINED__
#define __ModuleBase_LIBRARY_DEFINED__

/* library ModuleBase */
/* [version][helpstring][uuid] */ 








EXTERN_C const IID LIBID_ModuleBase;

#ifndef __ITestCases_INTERFACE_DEFINED__
#define __ITestCases_INTERFACE_DEFINED__

/* interface ITestCases */
/* [helpstring][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ITestCases;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0C3FEC3-8FBB-11d0-98CE-444553540000")
    ITestCases : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCaseName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCaseDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SyncProviderInterface( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProviderInterface( 
            /* [retval][out] */ IProviderInfo __RPC_FAR *__RPC_FAR *ppProvInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOwningITestModule( 
            /* [retval][out] */ ITestModule __RPC_FAR *__RPC_FAR *ppTestModule) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [retval][out] */ LONG __RPC_FAR *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariationCount( 
            /* [retval][out] */ LONG __RPC_FAR *plVariationCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE ExecuteVariation( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ VARIATION_STATUS __RPC_FAR *pResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariationID( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ LONG __RPC_FAR *plID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVariationDesc( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITestCasesVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITestCases __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITestCases __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITestCases __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCaseName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCaseDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SyncProviderInterface )( 
            ITestCases __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProviderInterface )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ IProviderInfo __RPC_FAR *__RPC_FAR *ppProvInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOwningITestModule )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ ITestModule __RPC_FAR *__RPC_FAR *ppTestModule);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Terminate )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVariationCount )( 
            ITestCases __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plVariationCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ExecuteVariation )( 
            ITestCases __RPC_FAR * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ VARIATION_STATUS __RPC_FAR *pResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVariationID )( 
            ITestCases __RPC_FAR * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ LONG __RPC_FAR *plID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVariationDesc )( 
            ITestCases __RPC_FAR * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription);
        
        END_INTERFACE
    } ITestCasesVtbl;

    interface ITestCases
    {
        CONST_VTBL struct ITestCasesVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITestCases_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITestCases_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITestCases_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITestCases_GetName(This,pbstrCaseName)	\
    (This)->lpVtbl -> GetName(This,pbstrCaseName)

#define ITestCases_GetDescription(This,pbstrCaseDescription)	\
    (This)->lpVtbl -> GetDescription(This,pbstrCaseDescription)

#define ITestCases_SyncProviderInterface(This)	\
    (This)->lpVtbl -> SyncProviderInterface(This)

#define ITestCases_GetProviderInterface(This,ppProvInfo)	\
    (This)->lpVtbl -> GetProviderInterface(This,ppProvInfo)

#define ITestCases_GetOwningITestModule(This,ppTestModule)	\
    (This)->lpVtbl -> GetOwningITestModule(This,ppTestModule)

#define ITestCases_Init(This,plResult)	\
    (This)->lpVtbl -> Init(This,plResult)

#define ITestCases_Terminate(This,pfResult)	\
    (This)->lpVtbl -> Terminate(This,pfResult)

#define ITestCases_GetVariationCount(This,plVariationCount)	\
    (This)->lpVtbl -> GetVariationCount(This,plVariationCount)

#define ITestCases_ExecuteVariation(This,lIndex,pResult)	\
    (This)->lpVtbl -> ExecuteVariation(This,lIndex,pResult)

#define ITestCases_GetVariationID(This,lIndex,plID)	\
    (This)->lpVtbl -> GetVariationID(This,lIndex,plID)

#define ITestCases_GetVariationDesc(This,lIndex,pbstrDescription)	\
    (This)->lpVtbl -> GetVariationDesc(This,lIndex,pbstrDescription)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITestCases_GetName_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrCaseName);


void __RPC_STUB ITestCases_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_GetDescription_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrCaseDescription);


void __RPC_STUB ITestCases_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_SyncProviderInterface_Proxy( 
    ITestCases __RPC_FAR * This);


void __RPC_STUB ITestCases_SyncProviderInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_GetProviderInterface_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ IProviderInfo __RPC_FAR *__RPC_FAR *ppProvInfo);


void __RPC_STUB ITestCases_GetProviderInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_GetOwningITestModule_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ ITestModule __RPC_FAR *__RPC_FAR *ppTestModule);


void __RPC_STUB ITestCases_GetOwningITestModule_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_Init_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plResult);


void __RPC_STUB ITestCases_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_Terminate_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult);


void __RPC_STUB ITestCases_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_GetVariationCount_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plVariationCount);


void __RPC_STUB ITestCases_GetVariationCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_ExecuteVariation_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ VARIATION_STATUS __RPC_FAR *pResult);


void __RPC_STUB ITestCases_ExecuteVariation_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_GetVariationID_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ LONG __RPC_FAR *plID);


void __RPC_STUB ITestCases_GetVariationID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestCases_GetVariationDesc_Proxy( 
    ITestCases __RPC_FAR * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB ITestCases_GetVariationDesc_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITestCases_INTERFACE_DEFINED__ */


#ifndef __ITestModule_INTERFACE_DEFINED__
#define __ITestModule_INTERFACE_DEFINED__

/* interface ITestModule */
/* [helpstring][oleautomation][uuid][object] */ 


EXTERN_C const IID IID_ITestModule;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A0C3FEC4-8FBB-11d0-98CE-444553540000")
    ITestModule : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetName( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrName) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetDescription( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetOwnerName( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrOwner) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCLSID( 
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetVersion( 
            /* [retval][out] */ LONG __RPC_FAR *plVersion) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetProviderInterface( 
            /* [in] */ IProviderInfo __RPC_FAR *pProvInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetProviderInterface( 
            /* [retval][out] */ IProviderInfo __RPC_FAR *__RPC_FAR *ppProvInfo) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetErrorInterface( 
            /* [in] */ IError __RPC_FAR *pIError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetErrorInterface( 
            /* [retval][out] */ IError __RPC_FAR *__RPC_FAR *ppIError) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Init( 
            /* [retval][out] */ LONG __RPC_FAR *plResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Terminate( 
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCaseCount( 
            /* [retval][out] */ LONG __RPC_FAR *plCaseCount) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetCase( 
            /* [in] */ LONG lIndex,
            /* [retval][out] */ ITestCases __RPC_FAR *__RPC_FAR *ppITestCases) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct ITestModuleVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            ITestModule __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            ITestModule __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            ITestModule __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetName )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrName);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetDescription )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOwnerName )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrOwner);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCLSID )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetVersion )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plVersion);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetProviderInterface )( 
            ITestModule __RPC_FAR * This,
            /* [in] */ IProviderInfo __RPC_FAR *pProvInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetProviderInterface )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ IProviderInfo __RPC_FAR *__RPC_FAR *ppProvInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetErrorInterface )( 
            ITestModule __RPC_FAR * This,
            /* [in] */ IError __RPC_FAR *pIError);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetErrorInterface )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ IError __RPC_FAR *__RPC_FAR *ppIError);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Init )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Terminate )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCaseCount )( 
            ITestModule __RPC_FAR * This,
            /* [retval][out] */ LONG __RPC_FAR *plCaseCount);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetCase )( 
            ITestModule __RPC_FAR * This,
            /* [in] */ LONG lIndex,
            /* [retval][out] */ ITestCases __RPC_FAR *__RPC_FAR *ppITestCases);
        
        END_INTERFACE
    } ITestModuleVtbl;

    interface ITestModule
    {
        CONST_VTBL struct ITestModuleVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define ITestModule_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define ITestModule_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define ITestModule_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define ITestModule_GetName(This,pbstrName)	\
    (This)->lpVtbl -> GetName(This,pbstrName)

#define ITestModule_GetDescription(This,pbstrDescription)	\
    (This)->lpVtbl -> GetDescription(This,pbstrDescription)

#define ITestModule_GetOwnerName(This,pbstrOwner)	\
    (This)->lpVtbl -> GetOwnerName(This,pbstrOwner)

#define ITestModule_GetCLSID(This,pbstrCLSID)	\
    (This)->lpVtbl -> GetCLSID(This,pbstrCLSID)

#define ITestModule_GetVersion(This,plVersion)	\
    (This)->lpVtbl -> GetVersion(This,plVersion)

#define ITestModule_SetProviderInterface(This,pProvInfo)	\
    (This)->lpVtbl -> SetProviderInterface(This,pProvInfo)

#define ITestModule_GetProviderInterface(This,ppProvInfo)	\
    (This)->lpVtbl -> GetProviderInterface(This,ppProvInfo)

#define ITestModule_SetErrorInterface(This,pIError)	\
    (This)->lpVtbl -> SetErrorInterface(This,pIError)

#define ITestModule_GetErrorInterface(This,ppIError)	\
    (This)->lpVtbl -> GetErrorInterface(This,ppIError)

#define ITestModule_Init(This,plResult)	\
    (This)->lpVtbl -> Init(This,plResult)

#define ITestModule_Terminate(This,pfResult)	\
    (This)->lpVtbl -> Terminate(This,pfResult)

#define ITestModule_GetCaseCount(This,plCaseCount)	\
    (This)->lpVtbl -> GetCaseCount(This,plCaseCount)

#define ITestModule_GetCase(This,lIndex,ppITestCases)	\
    (This)->lpVtbl -> GetCase(This,lIndex,ppITestCases)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE ITestModule_GetName_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrName);


void __RPC_STUB ITestModule_GetName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetDescription_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrDescription);


void __RPC_STUB ITestModule_GetDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetOwnerName_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrOwner);


void __RPC_STUB ITestModule_GetOwnerName_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetCLSID_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ BSTR __RPC_FAR *pbstrCLSID);


void __RPC_STUB ITestModule_GetCLSID_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetVersion_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plVersion);


void __RPC_STUB ITestModule_GetVersion_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_SetProviderInterface_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [in] */ IProviderInfo __RPC_FAR *pProvInfo);


void __RPC_STUB ITestModule_SetProviderInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetProviderInterface_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ IProviderInfo __RPC_FAR *__RPC_FAR *ppProvInfo);


void __RPC_STUB ITestModule_GetProviderInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_SetErrorInterface_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [in] */ IError __RPC_FAR *pIError);


void __RPC_STUB ITestModule_SetErrorInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetErrorInterface_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ IError __RPC_FAR *__RPC_FAR *ppIError);


void __RPC_STUB ITestModule_GetErrorInterface_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_Init_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plResult);


void __RPC_STUB ITestModule_Init_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_Terminate_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ VARIANT_BOOL __RPC_FAR *pfResult);


void __RPC_STUB ITestModule_Terminate_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetCaseCount_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [retval][out] */ LONG __RPC_FAR *plCaseCount);


void __RPC_STUB ITestModule_GetCaseCount_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE ITestModule_GetCase_Proxy( 
    ITestModule __RPC_FAR * This,
    /* [in] */ LONG lIndex,
    /* [retval][out] */ ITestCases __RPC_FAR *__RPC_FAR *ppITestCases);


void __RPC_STUB ITestModule_GetCase_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __ITestModule_INTERFACE_DEFINED__ */

#endif /* __ModuleBase_LIBRARY_DEFINED__ */

/* interface __MIDL_itf_ModuleCore_0012 */
/* [local] */ 




extern RPC_IF_HANDLE __MIDL_itf_ModuleCore_0012_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ModuleCore_0012_v0_0_s_ifspec;

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
