/*++

Copyright (C)  Microsoft Corporation

Module Name:

	ProvResv.H

Abstract:


History:

--*/

#ifndef _CProvider_IWbemEventProvider_H
#define _CProvider_IWbemEventProvider_H

/******************************************************************************
 *
 *	Name:
 *
 *	
 *  Description:
 *
 *	
 *****************************************************************************/

class CProvider_IWbemEventProvider :	public IWbemEventProvider , 
										public IWbemProviderInit , 
										public IWbemShutdown
{
private:

	LONG m_ReferenceCount ;
	LONG m_InternalReferenceCount ;

	CRITICAL_SECTION m_CriticalSection ;

	IWbemServices *m_CoreService ;
	IWbemClassObject *m_EventObject ;
	IWbemObjectSink *m_EventSink ;

	HANDLE m_ThreadTerminate ;
	HANDLE m_ThreadHandle ;

	BSTR m_Namespace ;
	BSTR m_Locale ;
	BSTR m_User ;

private:

	static DWORD ThreadExecutionFunction ( void *a_Context ) ;

public:

	CProvider_IWbemEventProvider () ;
    ~CProvider_IWbemEventProvider () ;

    STDMETHODIMP_( ULONG ) InternalAddRef () ;
    STDMETHODIMP_( ULONG ) InternalRelease () ;

public:

	//Non-delegating object IUnknown

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	HRESULT STDMETHODCALLTYPE ProvideEvents ( 

		IWbemObjectSink *a_Sink ,
		LONG a_Flags
	) ;

	/* IWbemProviderInit methods */

	HRESULT STDMETHODCALLTYPE Initialize (

		LPWSTR a_User ,
		LONG a_Flags ,
		LPWSTR a_Namespace ,
		LPWSTR a_Locale ,
		IWbemServices *a_Core ,
		IWbemContext *a_Context ,
		IWbemProviderInitSink *a_Sink
	) ;

	HRESULT STDMETHODCALLTYPE Shutdown (

		LONG a_Flags ,
		ULONG a_MaxMilliSeconds ,
		IWbemContext *a_Context
	) ; 
} ;


#endif // _CProvider_IWbemEventProvider_H
