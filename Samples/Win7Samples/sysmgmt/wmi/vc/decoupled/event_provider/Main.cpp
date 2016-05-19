/*++

Copyright (C)  Microsoft Corporation

Module Name:

	Main.cpp

Abstract:


History:

--*/

#include <PreComp.h>
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include "Globals.h"

/******************************************************************************
 *
 *	Name:	g_ThreadTerminate
 *
 *	
 *  Description:
 *
 *		Global event used to inform worker thread that it should terminate.
 *
 *****************************************************************************/

HANDLE g_ThreadTerminate = NULL ;

/******************************************************************************
 *
 *	Name:	WindowsMainProc
 *
 *	
 *  Description:
 *
 *			Spin processing windows messages
 *
 *****************************************************************************/

LRESULT CALLBACK WindowsMainProc ( HWND a_hWnd , UINT a_message , WPARAM a_wParam , LPARAM a_lParam )
{
	LRESULT t_rc = 0 ;

	switch ( a_message )
	{
		case WM_DESTROY:
		{
			PostMessage ( a_hWnd , WM_QUIT , 0 , 0 ) ;
		}
		break ;

		default:
		{		
			t_rc = DefWindowProc ( a_hWnd , a_message , a_wParam , a_lParam ) ;
		}
		break ;
	}

	return ( t_rc ) ;
}

/******************************************************************************
 *
 *	Name:	WindowsInit	
 *
 *	
 *  Description:
 *
 *			Initialize a window so that we can close the app down.
 *
 *****************************************************************************/

HWND WindowsInit ( HINSTANCE a_HInstance )
{
	static wchar_t *t_TemplateCode = L"TemplateCode" ;

	WNDCLASS  t_wc ;
 
	t_wc.style            = CS_HREDRAW | CS_VREDRAW ;
	t_wc.lpfnWndProc      = WindowsMainProc ;
	t_wc.cbClsExtra       = 0 ;
	t_wc.cbWndExtra       = 0 ;
	t_wc.hInstance        = a_HInstance ;
	t_wc.hIcon            = LoadIcon(NULL, IDI_HAND) ;
	t_wc.hCursor          = LoadCursor(NULL, IDC_ARROW) ;
	t_wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1) ;
	t_wc.lpszMenuName     = NULL ;
	t_wc.lpszClassName    = t_TemplateCode ;
 
	ATOM t_winClass = RegisterClass ( &t_wc ) ;

	HWND t_HWnd = CreateWindow (

		t_TemplateCode ,              // see RegisterClass() call
		t_TemplateCode ,                      // text for window title bar
		WS_OVERLAPPEDWINDOW | WS_MINIMIZE ,               // window style
		CW_USEDEFAULT ,                     // default horizontal position
		CW_USEDEFAULT ,                     // default vertical position
		CW_USEDEFAULT ,                     // default width
		CW_USEDEFAULT ,                     // default height
		NULL ,                              // overlapped windows have no parent
		NULL ,                              // use the window class menu
		a_HInstance ,
		NULL                                // pointer not needed
	) ;

	ShowWindow ( t_HWnd, SW_SHOW ) ;
	//ShowWindow ( t_HWnd, SW_HIDE ) ;

	return t_HWnd ;
}

/******************************************************************************
 *
 *	Name:	WindowsStop
 *
 *	
 *  Description:
 *
 *			Cleanup window	
 *
 *****************************************************************************/

void WindowsStop ( HWND a_HWnd )
{
	DestroyWindow ( a_HWnd ) ;
}

/******************************************************************************
 *
 *	Name:	WindowsStart
 *
 *	
 *  Description:
 *
 *			Initialize windows dependant resources.
 *
 *****************************************************************************/

HWND WindowsStart ( HINSTANCE a_Handle )
{
	HWND t_HWnd = NULL ;
	if ( ! ( t_HWnd = WindowsInit ( a_Handle ) ) )
	{
    }

	return t_HWnd ;
}

/******************************************************************************
 *
 *	Name:	WindowsDispatch
 *
 *	
 *  Description:
 *
 *			Standard message pump.	
 *
 *****************************************************************************/

void WindowsDispatch ()
{
	BOOL t_GetMessage ;
	MSG t_lpMsg ;

	while (	( t_GetMessage = GetMessage ( & t_lpMsg , NULL , 0 , 0 ) ) == TRUE )
	{
		TranslateMessage ( & t_lpMsg ) ;
		DispatchMessage ( & t_lpMsg ) ;
	}
}

/******************************************************************************
 *
 *	Name:	UninitComServer
 *
 *	
 *  Description:
 *
 *			Uninitialize COM dependant resources,
 *
 *****************************************************************************/

HRESULT UninitComServer ()
{
	CoUninitialize () ;

	return S_OK ;
}

/******************************************************************************
 *
 *	Name:	InitComServer
 *
 *	
 *  Description:
 *
 *			Initialize COM dependant resources.
 *
 *****************************************************************************/

HRESULT InitComServer ( DWORD a_AuthenticationLevel , DWORD a_ImpersonationLevel )
{
	HRESULT t_Result = S_OK ;

    t_Result = CoInitializeEx (

		0, 
		COINIT_MULTITHREADED
	);

#ifndef EOAC_DYNAMIC_CLOAKING
#define EOAC_DYNAMIC_CLOAKING 0x40
#endif

	if ( SUCCEEDED ( t_Result ) ) 
	{

		DWORD dwCapabilities;
		if (a_AuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE) 
			dwCapabilities = EOAC_NONE;
		else
			dwCapabilities = EOAC_SECURE_REFS;

		t_Result = CoInitializeSecurity (

			NULL, 
			-1, 
			NULL, 
			NULL,
			a_AuthenticationLevel,
			a_ImpersonationLevel, 
			NULL, 
			EOAC_DYNAMIC_CLOAKING | dwCapabilities, 
			0
		);

		if ( FAILED ( t_Result ) ) 
		{
			CoUninitialize () ;
			return t_Result ;
		}
	}

	if ( FAILED ( t_Result ) )
	{
		CoUninitialize () ;
	}

	return t_Result  ;
}

/******************************************************************************
 *
 *	Name:		ThreadArg_Struct
 *
 *	
 *  Description:
 *
 *				Structure used to pass objects between threads.
 *	
 *****************************************************************************/

struct ThreadArg_Struct 
{
	IWbemServices *m_Service ;
	IWbemObjectSink *m_Sink ;
	IWbemEventSink *m_EventSink ;
	IWbemDecoupledBasicEventProvider *m_Registrar ;
} ;

/******************************************************************************
 *
 *	Name:	ThreadExecutionFunction
 *
 *	
 *  Description:
 *
 *		Worker thread function used to obtain event class and sit in a loop
 *		generating instances of a particular event class.
 *
 *****************************************************************************/

DWORD ThreadExecutionFunction ( void *a_Context )
{
	HRESULT t_Result = S_OK ;

/*
 *	Initialize com on thread.
 */

	t_Result = CoInitializeEx (

		0, 
		COINIT_MULTITHREADED
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{

/*
 *	Cast thread context to expected type.
 */

		struct ThreadArg_Struct *t_ThreadStruct = ( struct ThreadArg_Struct * ) a_Context ;
		if ( t_ThreadStruct )
		{
			IWbemClassObject *t_Object = NULL ;
			BSTR t_String = SysAllocString ( L"SampleEvent" ) ;
			if ( t_String )
			{
/*
 *	Get the event class from WMI.
 */

				t_Result = t_ThreadStruct->m_Service->GetObject ( 

					t_String , 
					0 , 
					NULL , 
					& t_Object ,
					NULL
				) ;

				if ( SUCCEEDED ( t_Result ) )
				{
/*
 *	Create an event instance.
 */

					IWbemClassObject *t_Instance = NULL ;
					t_Result = t_Object->SpawnInstance ( 0 , & t_Instance ) ;
					if ( SUCCEEDED ( t_Result ) )
					{
						VARIANT t_Variant ;
						VariantInit ( & t_Variant ) ;

						t_Variant.vt = VT_BSTR ;
						t_Variant.bstrVal = SysAllocString ( L"Steve" ) ;
						if (t_Variant.bstrVal)
						{
							t_Result = t_Instance->Put ( 

									L"Name" ,
									0 , 
									& t_Variant ,
									CIM_EMPTY 
								) ;
						}
						else
						{
							t_Result = WBEM_E_OUT_OF_MEMORY;
						}
						

						if ( SUCCEEDED ( t_Result ) )
						{
							VariantClear ( & t_Variant ) ;

/*
 *	Sit in a loop until termination event has been signalled.
 */

							BOOL t_Continue = TRUE ;
							while ( t_Continue )
							{
								DWORD t_Status = WaitForSingleObject ( g_ThreadTerminate , 1000 ) ;
								switch ( t_Status )
								{
									case WAIT_TIMEOUT:
									{
/*
 *	Send the event on its way.
 */
										t_ThreadStruct->m_Sink->Indicate ( 1 , & t_Instance ) ;
									}
									break ;

									case WAIT_OBJECT_0:
									{
/*
 *	Event is signalled so break out of loop.
 */
										t_Continue = FALSE ;
									}
									break ;

									default:
									{
/*
 *	Unknown condition, just break out of loop.
 */
										t_Continue = FALSE ;
									}
									break ;
								}
							}
						}

						t_Instance->Release () ;
					}

					t_Object->Release () ;
				}

				SysFreeString ( t_String ) ;
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}

			t_ThreadStruct->m_Service->Release () ;
			t_ThreadStruct->m_Sink->Release () ;
			t_ThreadStruct->m_Registrar->Release () ;

			delete t_ThreadStruct ;
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}

		CoUninitialize () ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	RestrictedThreadExecutionFunction
 *
 *	
 *  Description:
 *
 *		Worker thread function used to obtain event class and sit in a loop
 *		generating instances of a particular event class.
 *
 *****************************************************************************/

DWORD RestrictedThreadExecutionFunction ( void *a_Context )
{
	HRESULT t_Result = S_OK ;

/*
 *	Initialize com on thread.
 */

	t_Result = CoInitializeEx (

		0, 
		COINIT_MULTITHREADED
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{
/*
 *	Cast thread context to expected type.
 */

		struct ThreadArg_Struct *t_ThreadStruct = ( struct ThreadArg_Struct * ) a_Context ;
		if ( t_ThreadStruct )
		{
			IWbemClassObject *t_Object = NULL ;
			BSTR t_String = SysAllocString ( L"SampleEvent" ) ;
			if ( t_String )
			{
/*
 *	Obtain a sink that will tell us when a user has asked for an event based on the specific
 *	query.
 */

				wchar_t *t_Query = L"Select * from SampleEvent" ;

				IWbemEventSink *t_RestrictedSink = NULL ; 

				t_Result = t_ThreadStruct->m_EventSink->GetRestrictedSink (

					1 ,
					& t_Query ,
					NULL ,
					& t_RestrictedSink
				) ;

				if ( SUCCEEDED ( t_Result ) )
				{
/*
 *	Set event batching parameters to boost performance.
 */
					t_Result = t_RestrictedSink->SetBatchingParameters (

						WBEM_FLAG_MUST_BATCH ,
						10000000 ,
						1000
					);

/*
 *	Get the event class from WMI.
 */

					t_Result = t_ThreadStruct->m_Service->GetObject ( 

						t_String , 
						0 , 
						NULL , 
						& t_Object ,
						NULL
					) ;

					if ( SUCCEEDED ( t_Result ) )
					{
/*
 *	Sit in a loop until termination event has been signalled.
 */

						BOOL t_Continue = TRUE ;
						while ( t_Continue )
						{

							DWORD t_Status = WaitForSingleObject ( g_ThreadTerminate , 10000 ) ;
							switch ( t_Status )
							{
								case WAIT_TIMEOUT:
								{
/*
 *	See if anyone is listening for event
 */
									if ( t_RestrictedSink->IsActive () == S_OK )
									{
/*
 *	Create an event instance.
 */
										IWbemClassObject *t_Instance = NULL ;
										t_Result = t_Object->SpawnInstance ( 0 , & t_Instance ) ;
										if ( SUCCEEDED ( t_Result ) )
										{
											VARIANT t_Variant ;
											VariantInit ( & t_Variant ) ;

											t_Variant.vt = VT_BSTR ;
											t_Variant.bstrVal = SysAllocString ( L"Steve" ) ;

											if (t_Variant.bstrVal)
											{
												t_Result = t_Instance->Put ( 

																	L"Name" ,
																	0 , 
																	& t_Variant ,
																	CIM_EMPTY 
																) ;
												/*
												*	Send the event on its way.
												*/
												t_RestrictedSink->Indicate ( 1 , & t_Instance ) ;

											}
											else
											{
												t_Result = WBEM_E_OUT_OF_MEMORY;
											}
										

											t_Instance->Release () ;
										}
									}
								}
								break ;

								case WAIT_OBJECT_0:
								{
/*
 *	Event is signalled so break out of loop.
 */
									t_Continue = FALSE ;
								}
								break ;

								default:
								{
/*
 *	Unknown condition, just break out of loop.
 */

									t_Continue = FALSE ;
								}
								break ;
							}
						}

						t_Object->Release () ;
					}

					t_RestrictedSink->Release () ;
				}

				SysFreeString ( t_String ) ;
			}
			else
			{
				t_Result = WBEM_E_OUT_OF_MEMORY ;
			}

			t_ThreadStruct->m_Service->Release () ;
			t_ThreadStruct->m_Sink->Release () ;
			t_ThreadStruct->m_Registrar->Release () ;

			delete t_ThreadStruct ;
		}
		else
		{
			t_Result = WBEM_E_INVALID_PARAMETER ;
		}

		CoUninitialize () ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	RegisterDecoupledEvent
 *
 *	
 *  Description:
 *
 *			Register a decoupled event provider object with WMI.
 *			We pass in an implementation of IWbemEventProvider so 
 *			that WMI can inform provider of new client event registrations.
 *
 *****************************************************************************/

HRESULT RegisterDecoupledEvent ( 

	IWbemDecoupledBasicEventProvider *&a_Registrar ,
	HANDLE &a_ThreadHandle
)
{
/*
 *	Cleanup out arguments
 */

	a_ThreadHandle = NULL ;
	a_Registrar = NULL ;

	HRESULT t_Result = S_OK ;

/*
 *	Create an event to inform worker thread to terminate.
 */

	g_ThreadTerminate = CreateEvent ( NULL , FALSE , FALSE , NULL ) ;
	if ( g_ThreadTerminate )
	{
/*
 *	Create the event registrar, so that we can register with WMI.
 */

		t_Result = Provider_Globals :: CreateInstance ( 

			CLSID_WbemDecoupledBasicEventProvider ,
			NULL ,
			CLSCTX_INPROC_SERVER ,
			IID_IWbemDecoupledBasicEventProvider ,
			( void ** ) & a_Registrar 
		) ;

		if ( SUCCEEDED ( t_Result ) )
		{	
			t_Result = a_Registrar->Register ( 

				0 ,
				NULL ,
				NULL ,
				NULL ,
				L"root\\cimv2" ,
				L"DecoupledEventProvider" ,
				NULL
			) ;

			if ( SUCCEEDED ( t_Result ) )
			{
/*
 *	Grab the IWbemServices pointer associated with the provider instance, use it later to 
 *	query for class specific information when generating events.
 */

				IWbemServices *t_Service = NULL ;
				t_Result = a_Registrar->GetService ( 0 , NULL , & t_Service ) ;
				if ( SUCCEEDED ( t_Result ) )
				{

/*
 *	Grab the event sink associated with the event provider instance, use it later to send events to WMI.
 */

					IWbemObjectSink *t_Sink = NULL ;
					t_Result = a_Registrar->GetSink ( 0 , NULL , & t_Sink ) ;
					if ( SUCCEEDED ( t_Result ) )
					{
/*
 *	Pass information taken from WMI and package it so that worker thread can 
 *	proceed.
 */

						struct ThreadArg_Struct *t_ThreadStruct = new ThreadArg_Struct ;
						if ( t_ThreadStruct )
						{
							t_ThreadStruct->m_Service = t_Service ;
							t_ThreadStruct->m_Sink = t_Sink ;
							t_ThreadStruct->m_Registrar = a_Registrar ;

							t_ThreadStruct->m_Service->AddRef () ;
							t_ThreadStruct->m_Sink->AddRef () ;
							t_ThreadStruct->m_Registrar->AddRef () ;

/*
 *	Create worker thread.
 */	
 							
							DWORD t_ThreadId = 0 ;
							a_ThreadHandle = CreateThread (

								NULL ,
								0 , 
								( LPTHREAD_START_ROUTINE ) ThreadExecutionFunction ,
								t_ThreadStruct ,
								0 , 
								& t_ThreadId 
							) ;

							if ( a_ThreadHandle == NULL )
							{
/*
 *	Everything failed to release resources.
 */

								t_ThreadStruct->m_Service->Release () ;
								t_ThreadStruct->m_Sink->Release () ;
								t_ThreadStruct->m_Registrar->Release () ;

								delete t_ThreadStruct ;

								t_Result = WBEM_E_FAILED ;
							}
						}
						else
						{
							t_Result = WBEM_E_OUT_OF_MEMORY ;
						}

						t_Sink->Release () ;
					}

					t_Service->Release () ;
				}
			}
		}

		if ( FAILED ( t_Result ) )
		{
			if ( a_Registrar )
			{
				a_Registrar->Release () ;
				a_Registrar = NULL ;
			}
		}
	}
	else
	{
		t_Result = WBEM_E_OUT_OF_MEMORY ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	RegisterDecoupledRestrictedEvent
 *
 *	
 *  Description:
 *
 *			Register the event provider and create a thread to perform 
 *			generate of events. A restricted sink is an extension to 
 *			a regular event sink, one can specify batching parameters,
 *			detect when atleast one client has registered for a specific query, etc.
 *
 *****************************************************************************/

HRESULT RegisterDecoupledRestrictedEvent ( 

	IWbemDecoupledBasicEventProvider *&a_Registrar ,
	HANDLE &a_ThreadHandle
)
{
/*
 *	Cleanup out arguments
 */

	a_ThreadHandle = NULL ;
	a_Registrar = NULL ;

	HRESULT t_Result = S_OK ;

/*
 *	Create an event to inform worker thread to terminate.
 */

	g_ThreadTerminate = CreateEvent ( NULL , FALSE , FALSE , NULL ) ;
	if ( g_ThreadTerminate )
	{
/*
 *	Create the event registrar, so that we can register with WMI.
 */

		t_Result = Provider_Globals :: CreateInstance ( 

			CLSID_WbemDecoupledBasicEventProvider ,
			NULL ,
			CLSCTX_INPROC_SERVER ,
			IID_IWbemDecoupledBasicEventProvider ,
			( void ** ) & a_Registrar 
		) ;

		if ( SUCCEEDED ( t_Result ) )
		{
/*
 *	Register the provider inteface with WMI, specifying the namespace and provider instance of __Win32provider
 */

			t_Result = a_Registrar->Register ( 

				0 ,
				NULL ,
				NULL ,
				NULL ,
				L"root\\cimv2" ,
				L"DecoupledEventProvider" ,
				NULL
			) ;

			if ( SUCCEEDED ( t_Result ) )
			{
/*
 *	Grab the IWbemServices pointer associated with the provider instance, use it later to 
 *	query for class specific information when generating events.
 */

				IWbemServices *t_Service = NULL ;
				t_Result = a_Registrar->GetService ( 0 , NULL , & t_Service ) ;
				if ( SUCCEEDED ( t_Result ) )
				{
/*
 *	Grab the event sink associated with the event provider instance, use it later to send events to WMI.
 */
					IWbemObjectSink *t_Sink = NULL ;
					t_Result = a_Registrar->GetSink ( 0 , NULL , & t_Sink ) ;
					if ( SUCCEEDED ( t_Result ) )
					{
						IWbemEventSink *t_EventSink = NULL ;
						t_Result = t_Sink->QueryInterface ( IID_IWbemEventSink , ( void **) & t_EventSink ) ;
						if ( SUCCEEDED ( t_Result ) ) 
						{
/*
 *	Pass information taken from WMI and package it so that worker thread can 
 *	proceed.
 */
							struct ThreadArg_Struct *t_ThreadStruct = new ThreadArg_Struct ;
							if ( t_ThreadStruct )
							{
								t_ThreadStruct->m_Service = t_Service ;
								t_ThreadStruct->m_Sink = t_Sink ;
								t_ThreadStruct->m_EventSink = t_EventSink ;
								t_ThreadStruct->m_Registrar = a_Registrar ;
								
								t_ThreadStruct->m_Service->AddRef () ;
								t_ThreadStruct->m_Sink->AddRef () ;
								t_ThreadStruct->m_EventSink->AddRef () ;
								t_ThreadStruct->m_Registrar->AddRef () ;
		
/*
 *	Create worker thread.
 */						
								DWORD t_ThreadId = 0 ;
								a_ThreadHandle = CreateThread (

									NULL ,
									0 , 
									( LPTHREAD_START_ROUTINE ) RestrictedThreadExecutionFunction ,
									t_ThreadStruct ,
									0 , 
									& t_ThreadId 
								) ;

								if ( a_ThreadHandle == NULL )
								{
/*
 *	Everything failed to release resources.
 */

									t_ThreadStruct->m_Service->Release () ;
									t_ThreadStruct->m_Sink->Release () ;
									t_ThreadStruct->m_EventSink->Release () ;
									t_ThreadStruct->m_Registrar->Release () ;

									delete t_ThreadStruct ;

									t_Result = WBEM_E_FAILED ;
								}

								t_EventSink->Release () ;
							}
							else
							{
								t_Result = WBEM_E_OUT_OF_MEMORY ;
							}
						}

						t_Sink->Release () ;
					}

					t_Service->Release () ;
				}
			}
		}

		if ( FAILED ( t_Result ) )
		{
			if ( a_Registrar )
			{
				a_Registrar->Release () ;
				a_Registrar = NULL ;
			}
		}
	}
	else
	{
		t_Result = WBEM_E_OUT_OF_MEMORY ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		Process_DecoupledEvent
 *
 *	
 *  Description:
 *
 *				Register the provider and just sit spinning until the 
 *				application window is destroyed.
 *
 *****************************************************************************/

HRESULT Process_DecoupledEvent ()
{
	HANDLE t_ThreadHandle = NULL ;

	IWbemDecoupledBasicEventProvider *t_EventRegistrar = NULL ;

	HRESULT t_Result = RegisterDecoupledEvent ( t_EventRegistrar , t_ThreadHandle ) ;

	if ( SUCCEEDED ( t_Result ) )
	{
		WindowsDispatch () ;

/*
 *	Wait for provider thread to terminate before completing.
 */

		SetEvent ( g_ThreadTerminate ) ;
		WaitForSingleObject ( t_ThreadHandle , INFINITE ) ;
		CloseHandle ( t_ThreadHandle ) ;
	}

	if ( t_EventRegistrar )
	{
		t_EventRegistrar->UnRegister () ;
		t_EventRegistrar->Release () ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		Process_DecoupledRestrictedEvent
 *
 *	
 *  Description:
 *
 *				Register the provider and just sit spinning until the 
 *				application window is destroyed.
 *
 *****************************************************************************/

HRESULT Process_DecoupledRestrictedEvent ()
{
	HANDLE t_ThreadHandle = NULL ;

	IWbemDecoupledBasicEventProvider *t_EventRegistrar = NULL ;

	HRESULT t_Result = RegisterDecoupledRestrictedEvent ( t_EventRegistrar , t_ThreadHandle ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		WindowsDispatch () ;

/*
 *	Wait for provider thread to terminate before completing.
 */

		SetEvent ( g_ThreadTerminate ) ;
		WaitForSingleObject ( t_ThreadHandle , INFINITE ) ;
		CloseHandle ( t_ThreadHandle ) ;
	}

	if ( t_EventRegistrar )
	{
		t_EventRegistrar->UnRegister () ;
		t_EventRegistrar->Release () ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	Process
 *
 *	
 *  Description:
 *
 *			Main work routine, initialize com components and performs
 *			event provider dispatch loop.
 *
 *****************************************************************************/

HRESULT Process ()
{
	DWORD t_ImpersonationLevel = RPC_C_IMP_LEVEL_IDENTIFY ;
	DWORD t_AuthenticationLevel = RPC_C_AUTHN_LEVEL_PKT_PRIVACY; 

	HRESULT t_Result = InitComServer ( t_AuthenticationLevel , t_ImpersonationLevel ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		try 
		{
#if 0

/*
 *	Create a restricted sink event provider
 */
			t_Result = Process_DecoupledRestrictedEvent () ;

#else

/*
 *	Create a regular sink event provider
 */

			t_Result = Process_DecoupledEvent () ;
#endif
		}
		catch ( ... )
		{
		}

		UninitComServer () ;
	}

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:	Main
 *
 *	
 *  Description:
 *
 *		Main entry point, initialize global resources, windows components and then
 *		perform provider work.
 *
 *****************************************************************************/

int WINAPI WinMain (
  
    HINSTANCE hInstance,		// handle to current instance
    HINSTANCE hPrevInstance,	// handle to previous instance
    LPSTR lpCmdLine,			// pointer to command line
    int nShowCmd 				// show state of window
)
{
	HRESULT t_Result = Provider_Globals :: Global_Startup () ;
	if ( SUCCEEDED ( t_Result ) )
	{
		HWND t_Window = WindowsStart ( hInstance ) ;
		if ( t_Window )
		{
			t_Result = Process () ;

			WindowsStop ( t_Window ) ;
		}

		t_Result = Provider_Globals :: Global_Shutdown () ;
	}

	return 0 ;
}


