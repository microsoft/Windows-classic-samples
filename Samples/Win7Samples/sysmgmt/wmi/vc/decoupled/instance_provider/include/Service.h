/*++

Copyright (C)  Microsoft Corporation

Module Name:

	ProvResv.H

Abstract:


History:

--*/

#ifndef _CProvider_IWbemServices_H
#define _CProvider_IWbemServices_H

#include <wbemcli.h>
#include <wbemprov.h>

/******************************************************************************
 *
 *	Name:	Object_Property
 *
 *	
 *  Description:
 *
 *			Structure to encode static object data.
 *		
 *****************************************************************************/

struct Object_Property
{
	ULONG m_State ;
	CIMTYPE m_Type ;
	wchar_t *m_Name ;
	wchar_t *m_Value ;
} ;



/******************************************************************************
 *
 *	Name:	CProvider_IWbemServices
 *
 *	
 *  Description:
 *
 *			Instance provider implementation of WMI provider interfaces.
 *			Implementation allows WMI to query for instances, execute methods, etc.
 *
 *****************************************************************************/

class CProvider_IWbemServices : public IWbemServices , 
								public IWbemPropertyProvider ,	
								public IWbemProviderInit , 
								public IWbemShutdown
{
private:

// Private Variables

// Reference count for com object garbage collection.

	LONG m_ReferenceCount ;         //Object reference count

	CRITICAL_SECTION m_CriticalSection ;

// Pointer passed during provider initialization allowing call to WMI.

	IWbemServices *m_CoreService ;

// User/Local/Namespace information passed during initialization.

	BSTR m_Namespace ;
	BSTR m_Locale ;
	BSTR m_User ;

// Useful information

	BSTR m_ComputerName ;
	BSTR m_OperatingSystemVersion ;

// Cached object representing process class.

	IWbemClassObject *m_Win32_ProcessEx_Object ;
	IWbemClassObject *m_ContactInfo_Object ;


private:

	HRESULT CreateInstanceEnumAsync_Process (

		IWbemClassObject *a_ClassObject ,
 		long a_Flags , 
		IWbemContext *a_Context,
		IWbemObjectSink *a_Sink
	) ;

	HRESULT CreateInstanceEnumAsync_Contacts (

	IWbemClassObject *a_ClassObject ,
	long a_Flags , 
	IWbemContext __RPC_FAR *a_Context,
	IWbemObjectSink FAR *a_Sink
);

public:

// Constructor/Destructor

	CProvider_IWbemServices () ;
    ~CProvider_IWbemServices () ;

public:

	//Non-delegating object IUnknown

    STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;

    /* IWbemServices methods */

    HRESULT STDMETHODCALLTYPE OpenNamespace ( 

        const BSTR a_Namespace ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemServices **a_Service ,
        IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE QueryObjectSink ( 

        long a_Flags ,
        IWbemObjectSink **a_Sink
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE GetObject ( 

		const BSTR a_ObjectPath ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemClassObject **ppObject ,
        IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE PutClass ( 

        IWbemClassObject *a_Object ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE DeleteClass ( 

        const BSTR a_Class ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE CreateClassEnum ( 

        const BSTR a_Superclass ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IEnumWbemClassObject **a_Enum
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE PutInstance (

		IWbemClassObject *a_Instance ,
		long a_Flags , 
		IWbemContext *a_Context ,
		IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE DeleteInstance ( 

		const BSTR a_ObjectPath ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE CreateInstanceEnum (

		const BSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IEnumWbemClassObject **a_Enum
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE ExecQuery ( 

		const BSTR a_QueryLanguage,
		const BSTR a_Query,
		long a_Flags ,
		IWbemContext *a_Context ,
		IEnumWbemClassObject **a_Enum
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}


    HRESULT STDMETHODCALLTYPE ExecNotificationQuery (

		const BSTR a_QueryLanguage ,
		const BSTR a_Query ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IEnumWbemClassObject **a_Enum
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

	HRESULT STDMETHODCALLTYPE ExecNotificationQueryAsync ( 

        const BSTR a_QueryLanguage ,
        const BSTR a_Query ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE ExecMethod (

        const BSTR a_ObjectPath ,
        const BSTR a_MethodName ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemClassObject *a_InParams ,
        IWbemClassObject **a_OutParams ,
        IWbemCallResult **a_CallResult
	)
	{
		return WBEM_E_NOT_AVAILABLE ;
	}

    HRESULT STDMETHODCALLTYPE CancelAsyncCall ( 

        IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE GetObjectAsync (
        
		const BSTR a_ObjectPath ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE PutClassAsync ( 

        IWbemClassObject *a_Object ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE DeleteClassAsync ( 

        const BSTR a_Class ,
        long a_Flags ,
        IWbemContext *a_Context ,
        IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE CreateClassEnumAsync ( 

		const BSTR a_Superclass ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE PutInstanceAsync (

		IWbemClassObject *a_Instance ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink 
	) ;

    HRESULT STDMETHODCALLTYPE DeleteInstanceAsync ( 

		const BSTR a_ObjectPath,
		long a_Flags,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE CreateInstanceEnumAsync (

		const BSTR a_Class ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE ExecQueryAsync (

		const BSTR a_QueryLanguage ,
		const BSTR a_Query ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemObjectSink *a_Sink
	) ;

    HRESULT STDMETHODCALLTYPE ExecMethodAsync ( 

		const BSTR a_ObjectPath ,
		const BSTR a_MethodName ,
		long a_Flags ,
		IWbemContext *a_Context ,
		IWbemClassObject *a_InParams ,
		IWbemObjectSink *a_Sink
	) ;

	/* IWbemPropertyProvider methods */

    HRESULT STDMETHODCALLTYPE GetProperty (

        long a_Flags ,
        const BSTR a_Locale ,
        const BSTR a_ClassMapping ,
        const BSTR a_InstanceMapping ,
        const BSTR a_PropertyMapping ,
        VARIANT *a_Value
	) ;
        
    HRESULT STDMETHODCALLTYPE PutProperty (

        long a_Flags ,
        const BSTR a_Locale ,
        const BSTR a_ClassMapping ,
        const BSTR a_InstanceMapping ,
        const BSTR a_PropertyMapping ,
        const VARIANT *a_Value
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


#endif // _CProvider_IWbemServices_H
