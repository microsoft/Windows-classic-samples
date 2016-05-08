/*++

Copyright (C)  Microsoft Corporation

Module Name:

	Main.cpp

Abstract:


History:

--*/

#include "precomp.h"
#include <objbase.h>
#include <stdio.h>
#include <tchar.h>
#include <wbemcli.h>
#include <wbemprov.h>
#include "Globals.h"
#include "Events.h"

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
 *	Name:	RegisterDecoupledEventConsumer
 *
 *	
 *  Description:
 *
 *			Register a decoupled event consumer object with WMI.
 *			We pass in an implementation of IWbemEventConsumerProvider so 
 *			that WMI can forward events.	
 *
 *****************************************************************************/

HRESULT RegisterDecoupledEventConsumer (

	IWbemDecoupledRegistrar *&a_Registrar 
)
{
/*
 *	Cleanup out arguments
 */

	a_Registrar = NULL ;

/*
 *	Create the event registrar, so that we can register with WMI.
 */

	HRESULT t_Result = Provider_Globals :: CreateInstance ( 

		CLSID_WbemDecoupledRegistrar ,
		NULL ,
		CLSCTX_INPROC_SERVER ,
		IID_IWbemDecoupledRegistrar ,
		( void ** ) & a_Registrar 
	) ;

	if ( SUCCEEDED ( t_Result ) )
	{
/*
 *	Create our implementation instance of the event consumer
 */

		CProvider_IWbemEventConsumerProvider *t_Consumer = new CProvider_IWbemEventConsumerProvider ;
		if ( t_Consumer )
		{
			t_Consumer->AddRef () ;

			IUnknown *t_Unknown = NULL ;
			t_Result = t_Consumer->QueryInterface ( IID_IUnknown , ( void ** ) & t_Unknown ) ;
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
					L"DecoupledEventConsumer" ,
					t_Unknown
				) ;

				t_Unknown->Release () ;
			}

			t_Consumer->Release () ;
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

	return t_Result ;
}

/******************************************************************************
 *
 *	Name:		Process_DecoupledEventConsumer
 *
 *	
 *  Description:
 *
 *				Register the provider and just sit spinning until the 
 *				application window is destroyed.
 *
 *****************************************************************************/

HRESULT Process_DecoupledEventConsumer ()
{
	IWbemDecoupledRegistrar *t_EventConsumerRegistrar = NULL ;

	HRESULT t_Result = RegisterDecoupledEventConsumer ( t_EventConsumerRegistrar ) ;
	if ( SUCCEEDED ( t_Result ) )
	{
		WindowsDispatch () ;
	}

	if ( t_EventConsumerRegistrar )
	{
		t_EventConsumerRegistrar->UnRegister () ;
		t_EventConsumerRegistrar->Release () ;
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
 *			event consumer dispatch loop.
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
			t_Result = Process_DecoupledEventConsumer () ;
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


