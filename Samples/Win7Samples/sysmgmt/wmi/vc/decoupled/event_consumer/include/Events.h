/*++

Copyright (C)  Microsoft Corporation

Module Name:

	ProvResv.H

Abstract:


History:

--*/

#ifndef _CProvider_IWbemEventConsumerProvider_H
#define _CProvider_IWbemEventConsumerProvider_H

/******************************************************************************
 *
 *	Name:	CProvider_IWbemEventConsumerProvider
 *
 *	
 *  Description:
 *
 *			Event Consumer implementation of WMI provider interfaces.
 *			Implementation allows WMI to locate IWbemUnboundObjectSink implementation
 *			to forward events to.
 *
 *****************************************************************************/

class CProvider_IWbemEventConsumerProvider :	public IWbemEventConsumerProvider ,
												public IWbemProviderInit , 
												public IWbemShutdown
{
private:

// Private Variables

// Reference count for com object garbage collection.

	LONG m_ReferenceCount ;

// Pointer passed during provider initialization allowing call to WMI.

	IWbemServices *m_CoreService ;

// User/Local/Namespace information passed during initialization.

	BSTR m_Namespace ;
	BSTR m_Locale ;
	BSTR m_User ;

public:

// Constructor/Destructor

	CProvider_IWbemEventConsumerProvider () ;
    ~CProvider_IWbemEventConsumerProvider () ;

public:

	//Non-delegating object IUnknown

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	/* IWbemEventConsumerProvider methods */

	HRESULT STDMETHODCALLTYPE FindConsumer (

		IWbemClassObject *a_LogicalConsumer ,
		IWbemUnboundObjectSink **a_Consumer
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

	/* IWbemShutdown methods */

	HRESULT STDMETHODCALLTYPE Shutdown (

		LONG a_Flags ,
		ULONG a_MaxMilliSeconds ,
		IWbemContext *a_Context
	) ; 
} ;

/******************************************************************************
 *
 *	Name:	CProvider_IWbemUnboundObjectSink
 *
 *	
 *  Description:
 *
 *			Event Consumer implementation of WMI provider interfaces.
 *			WMI uses this implementation to forwards events received from
 *			an event provider. User code for IndicateToConsumer performs
 *			some action based on contents of event received.
 *
 *****************************************************************************/

class CProvider_IWbemUnboundObjectSink :	public IWbemUnboundObjectSink
{
private:

// Private Variables

// Reference count for com object garbage collection.

	LONG m_ReferenceCount ;

public:

// Constructor/Destructor

	CProvider_IWbemUnboundObjectSink () ;
    ~CProvider_IWbemUnboundObjectSink () ;

public:

	//Non-delegating object IUnknown

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

	/* IWbemUnboundObjectSink methods */

	HRESULT STDMETHODCALLTYPE IndicateToConsumer (

		IWbemClassObject *a_LogicalConsumer ,
		long a_ObjectCount ,
		IWbemClassObject **a_Objects
	) ;

} ;

#endif // _CProvider_IWbemEventConsumerProvider_H
