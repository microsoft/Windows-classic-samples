// **************************************************************************

//

// Copyright (c)  Microsoft Corporation, All Rights Reserved
//
// File:  EVPROV.H
//
// Description:
//        Sample event provider - header file defines event provider class
//
// History:
//
// **************************************************************************

#ifndef _EVPROV_H_
#define _EVPROV_H_

// {3CD5248E-14F9-11d1-AE9C-00C04FB68820}
DEFINE_GUID(CLSID_MyEventProvider, 
0x3cd5248e, 0x14f9, 0x11d1, 0xae, 0x9c, 0x0, 0xc0, 0x4f, 0xb6, 0x88, 0x20);

#define EVENTCLASS  L"MyEvent"

#include <NTSECAPI.H>


class CMyEventProvider : public IWbemEventProvider, public IWbemProviderInit, public IWbemEventProviderSecurity
{
    ULONG               m_cRef;
    IWbemServices       *m_pNs;
    IWbemObjectSink     *m_pSink;
    IWbemClassObject    *m_pEventClassDef;
    int                 m_eStatus;
    HANDLE              m_hThread;
            
    static DWORD WINAPI EventThread(LPVOID pArg);
    void InstanceThread();

	//security helper
	HRESULT CheckAccess (SECURITY_DESCRIPTOR *a_SecurityDescriptor ,
					DWORD a_Access , 
					GENERIC_MAPPING *a_Mapping);


public:
    enum { Pending, Running, PendingStop, Stopped };

    CMyEventProvider();
   ~CMyEventProvider();

    //
    // IUnknown members
    //
    STDMETHODIMP         QueryInterface(REFIID, LPVOID *);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // Inherited from IWbemEventProvider
    // =================================

    HRESULT STDMETHODCALLTYPE ProvideEvents( 
            /* [in] */ IWbemObjectSink __RPC_FAR *pSink,
            /* [in] */ long lFlags
            );

    // Inherited from IWbemProviderInit
    // ================================

    HRESULT STDMETHODCALLTYPE Initialize( 
            /* [in] */ LPWSTR pszUser,
            /* [in] */ LONG lFlags,
            /* [in] */ LPWSTR pszNamespace,
            /* [in] */ LPWSTR pszLocale,
            /* [in] */ IWbemServices __RPC_FAR *pNamespace,
            /* [in] */ IWbemContext __RPC_FAR *pCtx,
            /* [in] */ IWbemProviderInitSink __RPC_FAR *pInitSink
            );


	 // Inherited from IWbemEventProviderSecurity
    // ================================

	HRESULT STDMETHODCALLTYPE AccessCheck(
			/* [in] */ WBEM_CWSTR wszQueryLanguage,
			/* [in] */ WBEM_CWSTR wszQuery,
			/* [in] */ long lSidLength,
			/* [in, size_is(lSidLength), unique] */ const BYTE* pSid
			);
};


#endif
