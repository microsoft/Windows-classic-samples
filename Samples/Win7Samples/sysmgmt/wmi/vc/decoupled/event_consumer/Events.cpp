/*++

Copyright (C)  Microsoft Corporation

Module Name:

	XXXX

Abstract:


History:

--*/

#include "precomp.h"

#include <wbemcli.h>
#include <wbemprov.h>
#include "Globals.h"
#include "Events.h"

/******************************************************************************
 *
 *	Name:	CProvider_IWbemEventConsumerProvider
 *
 *	
 *  Description:
 *
 *		Constructor for object. Initialize variables to NULL. Increment global object count
 *	
 *****************************************************************************/

CProvider_IWbemEventConsumerProvider:: CProvider_IWbemEventConsumerProvider () : 

	m_ReferenceCount ( 0 ) , 
	m_User ( NULL ) ,
	m_Locale ( NULL ) ,
	m_Namespace ( NULL ) ,
	m_CoreService ( NULL )
{
	InterlockedIncrement ( & Provider_Globals :: s_ObjectsInProgress ) ;
}

/******************************************************************************
 *
 *	Name:	~CProvider_IWbemEventConsumerProvider
 *
 *	
 *  Description:
 *
 *		Constructor for object. Uninitialize variables . Decrement global object count
 *	
 *****************************************************************************/

CProvider_IWbemEventConsumerProvider:: ~CProvider_IWbemEventConsumerProvider ()
{
	if ( m_User ) 
	{
		SysFreeString ( m_User ) ;
	}

	if ( m_Locale ) 
	{
		SysFreeString ( m_Locale ) ;
	}

	if ( m_Namespace ) 
	{
		SysFreeString ( m_Namespace ) ;
	}

	if ( m_CoreService ) 
	{
		m_CoreService->Release () ;
	}

	InterlockedDecrement ( & Provider_Globals :: s_ObjectsInProgress ) ;
}

/******************************************************************************
 *
 *	Name:		AddRef
 *
 *	
 *  Description:
 *
 *		Perform Locked increment. Keep com object Alive.
 *
 *****************************************************************************/

STDMETHODIMP_(ULONG) CProvider_IWbemEventConsumerProvider:: AddRef ( void )
{
	return InterlockedIncrement ( & m_ReferenceCount ) ;
}

/******************************************************************************
 *
 *	Name:
 *
 *		Release
 *
 *  Description:
 *
 *		Perform Locked decrement. Attempt to destroy com object.
 *	
 *****************************************************************************/

STDMETHODIMP_(ULONG) CProvider_IWbemEventConsumerProvider:: Release ( void )
{
	LONG t_Reference ;
	if ( ( t_Reference = InterlockedDecrement ( & m_ReferenceCount ) ) == 0 )
	{
/*
 *	No more outstanding references, delete the object.
 */

		delete this ;
	}

	return t_Reference ;
}

/******************************************************************************
 *
 *	Name:	QueryInterface
 *
 *	
 *  Description:
 *
 *			Determine interface availabilty for com object.
 *
 *****************************************************************************/

STDMETHODIMP CProvider_IWbemEventConsumerProvider:: QueryInterface (

	REFIID a_Riid , 
	LPVOID FAR *a_Void 
) 
{
/*
 *	Just clean up out parameter first.
 */

	*a_Void = NULL ;

	if ( a_Riid == IID_IUnknown )
	{
		*a_Void = ( LPVOID ) this ;
	}
	else if ( a_Riid == IID_IWbemEventConsumerProvider )
	{
/*
 *	Make sure we support event consumer interface IWbemEventConsumerProvider
 */

		*a_Void = ( LPVOID ) ( IWbemEventConsumerProvider * ) this ;		
	}	
	else if ( a_Riid == IID_IWbemProviderInit )
	{
/*
 *	We support optional interface IWbemProviderInit
 */

		*a_Void = ( LPVOID ) ( IWbemProviderInit * ) this ;		
	}	
	else if ( a_Riid == IID_IWbemShutdown )
	{
/*
 *	We support optional interface IWbemShutdown
 */

		*a_Void = ( LPVOID ) ( IWbemShutdown * ) this ;		
	}	

/*
 *	Retain a reference on outgoing parameter.
 */

	if ( *a_Void )
	{
		( ( LPUNKNOWN ) *a_Void )->AddRef () ;

		return ResultFromScode ( S_OK ) ;
	}
	else
	{
		return ResultFromScode ( E_NOINTERFACE ) ;
	}
}

/******************************************************************************
 *
 *	Name:	FindConsumer
 *
 *	
 *  Description:
 *
 *			Wmi uses this method to obtain a specific endpoint for 
 *			an event consumer registration defined by a_LogicalConsumer.
 *			Provider returns an implementation of IWbemUnboundObjectSink so that
 *			Wmi can deliver the event.	
 *
 *****************************************************************************/

HRESULT CProvider_IWbemEventConsumerProvider:: FindConsumer (

	IWbemClassObject *a_LogicalConsumer ,
	IWbemUnboundObjectSink **a_Consumer
)
{
	HRESULT t_Result = S_OK ;

	if ( a_Consumer )
	{
/*
 *	Cleanup the outgoing parameter
 */

		*a_Consumer = NULL ;

		CProvider_IWbemUnboundObjectSink *t_Sink = new CProvider_IWbemUnboundObjectSink ;
		if ( t_Sink )
		{
/*
 *	AddRef for outgoing parameter ;
 */

			t_Sink->AddRef () ;

			*a_Consumer = t_Sink ;
		}
		else
		{
			t_Result = WBEM_E_OUT_OF_MEMORY ;
		}
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		Initialize
 *
 *	
 *  Description:
 *
 *				Wmi calls this optional interface to inform the provider of
 *				configuration information associated within the client, locale
 *				and namespace. Provider retains this information for future use.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemEventConsumerProvider:: Initialize (

	LPWSTR a_User,
	LONG a_Flags,
	LPWSTR a_Namespace,
	LPWSTR a_Locale,
	IWbemServices *a_CoreService,         // For anybody
	IWbemContext *a_Context,
	IWbemProviderInitSink *a_Sink     // For init signals
)
{
	HRESULT t_Result = S_OK ;

	if ( a_CoreService ) 
	{
/*
 *	Interface we can call back to retrieve WMI related information, class definitions, etc.
 */
		m_CoreService = a_CoreService ;
		m_CoreService->AddRef () ;
	}
	else
	{
		t_Result = WBEM_E_INVALID_PARAMETER ;
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_User ) 
		{
			m_User = SysAllocString ( a_User ) ;
			if ( m_User == NULL )
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_Locale ) 
		{
			m_Locale = SysAllocString ( a_Locale ) ;
			if ( m_Locale == NULL )
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
	}

	if ( SUCCEEDED ( t_Result ) )
	{
		if ( a_Namespace ) 
		{
			m_Namespace = SysAllocString ( a_Namespace ) ;
			if ( m_Namespace == NULL )
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}
		}
	}
	
/*
 *	Indicate to WMI status of call.
 */

	a_Sink->SetStatus ( t_Result , 0 ) ;

/*
 *	returning failing means that WMI need not check status specified in SetStatus call.
 */

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		Shutdown
 *
 *	
 *  Description:
 *
 *				Optional interface and method that informs provider of object implementation
 *				being released by WMI. Shutdown is not guaranteed to be called on system
 *				shutdown.
 *
 *****************************************************************************/

HRESULT CProvider_IWbemEventConsumerProvider:: Shutdown (

	LONG a_Flags ,
	ULONG a_MaxMilliSeconds ,
	IWbemContext *a_Context
)
{
	HRESULT t_Result = S_OK ;

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		CProvider_IWbemUnboundObjectSink
 *
 *	
 *  Description:
 *
 *				Constructor, initializes variables. Increment global object count
 *
 *****************************************************************************/

CProvider_IWbemUnboundObjectSink:: CProvider_IWbemUnboundObjectSink () : 

	m_ReferenceCount ( 0 )
{
	InterlockedIncrement ( & Provider_Globals :: s_ObjectsInProgress ) ;
}

/******************************************************************************
 *
 *	Name:		~CProvider_IWbemUnboundObjectSink
 *
 *	
 *  Description:
 *
 *				Destructor, uninitializes variables. Decrement global object count
 *
 *****************************************************************************/

CProvider_IWbemUnboundObjectSink:: ~CProvider_IWbemUnboundObjectSink()
{
	InterlockedDecrement ( & Provider_Globals :: s_ObjectsInProgress ) ;
}

/******************************************************************************
 *
 *	Name:		AddRef
 *
 *	
 *  Description:
 *
 *		Perform Locked increment. Keep com object Alive.
 *
 *****************************************************************************/

STDMETHODIMP_(ULONG) CProvider_IWbemUnboundObjectSink:: AddRef ( void )
{
	return InterlockedIncrement ( & m_ReferenceCount ) ;
}

/******************************************************************************
 *
 *	Name:
 *
 *	
 *  Description:
 *
 *	
 *****************************************************************************/

STDMETHODIMP_(ULONG) CProvider_IWbemUnboundObjectSink:: Release ( void )
{
	LONG t_Reference ;
	if ( ( t_Reference = InterlockedDecrement ( & m_ReferenceCount ) ) == 0 )
	{
/*
 *	No more outstanding references, delete the object.
 */

		delete this ;
	}

	return t_Reference ;
}

/******************************************************************************
 *
 *	Name:	QueryInterface
 *
 *	
 *  Description:
 *
 *			Determine interface availabilty for com object.
 *
 *****************************************************************************/

STDMETHODIMP CProvider_IWbemUnboundObjectSink:: QueryInterface (

	REFIID a_Riid , 
	LPVOID FAR *a_Void 
) 
{
/*
 *	Just clean up out parameter first.
 */

	*a_Void = NULL ;

	if ( a_Riid == IID_IUnknown )
	{
		*a_Void = ( LPVOID ) this ;
	}
	else if ( a_Riid == IID_IWbemUnboundObjectSink )
	{
/*
 *	Make sure we support event consumer interface IWbemUnboundObjectSink
 */

		*a_Void = ( LPVOID ) ( IWbemUnboundObjectSink * ) this ;		
	}	

/*
 *	Retain a reference on outgoing parameter.
 */

	if ( *a_Void )
	{
		( ( LPUNKNOWN ) *a_Void )->AddRef () ;

		return ResultFromScode ( S_OK ) ;
	}
	else
	{
		return ResultFromScode ( E_NOINTERFACE ) ;
	}
}

/******************************************************************************
 *
 *	Name:	IndicateToConsumer
 *
 *	
 *  Description:
 *
 *			Interface method called by WMI to indicate an event has been generated.
 *			User code should be added here to perform some action, we perform a beep.			
 *
 *****************************************************************************/

HRESULT CProvider_IWbemUnboundObjectSink:: IndicateToConsumer (

	IWbemClassObject *a_LogicalConsumer ,
	long a_ObjectCount ,
	IWbemClassObject **a_Objects
)
{
	HRESULT t_Result = S_OK ;

	MessageBeep ( MB_ICONEXCLAMATION ) ;
 
	return t_Result ;
}

