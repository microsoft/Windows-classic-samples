//-----------------------------------------------------------------------
// This file is part of the Windows SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#pragma once

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <tchar.h>

#include <atlbase.h>
#include <atlcom.h>

#include "WinSATCOMInterfaceI.h"

HRESULT CreateInitiateAssessment( IInitiateWinSATAssessment **pInitiateWinsat )
{
	HRESULT hr;

	hr = CoCreateInstance( __uuidof(CInitiateWinSAT), 
                           NULL, 
                           CLSCTX_INPROC_SERVER, 
                           __uuidof(IInitiateWinSATAssessment), 
                           (void **)pInitiateWinsat);
	return hr;
}

//
// Provide a mechanism to cancel WinSAT using CTRL-C
//
IInitiateWinSATAssessment *pInitiateWinsatForCancel = NULL;
BOOL WINAPI ConsoleControlHandlerRoutine( DWORD dwCtrlType )
{
    UNREFERENCED_PARAMETER(dwCtrlType);    
    if (pInitiateWinsatForCancel != NULL)
    {
        pInitiateWinsatForCancel->CancelAssessment();
    }
    return TRUE;
}

//
// Implementing IWinSATInitiateEvents allows us to receive
// feedback on the progress of WinSAT
//
class WinSATEvents :
  public CComObjectRootEx<CComMultiThreadModel>,
  public IWinSATInitiateEvents
{
public:

    BEGIN_COM_MAP(WinSATEvents)
        COM_INTERFACE_ENTRY(IWinSATInitiateEvents)
    END_COM_MAP()
    DECLARE_NOT_AGGREGATABLE(WinSATEvents)

   STDMETHOD(WinSATComplete)(
       IN HRESULT hresult,
       IN LPCWSTR strDescription
    );

   STDMETHOD(WinSATUpdate)(
       IN UINT uCurrentTick,
       IN UINT uTickTotal,
       IN LPCWSTR strCurrentState
    );

   HRESULT Init();
   HRESULT WaitForCompletion();   

   ~WinSATEvents();

private:
    HANDLE m_hEvent;    
};

HRESULT WinSATEvents::WinSATUpdate(
       IN UINT uCurrentTick,
       IN UINT uTickTotal,
       IN LPCWSTR strCurrentState
    )
{
    wprintf (L"Phase %ws (Step %i/%i):\n", 
             strCurrentState, 
             uCurrentTick, 
             uTickTotal ); 
    return S_OK;
}

STDMETHODIMP
WinSATEvents::WinSATComplete(
       IN HRESULT hresult,
       IN LPCWSTR strDescription
    )
{     
    wprintf (L"Winsat Complete:\n");
    wprintf (L"   Message: %s\n", strDescription);  
    wprintf (L"   hr     : %08.8lx\n", hresult);    
    if (!SetEvent(m_hEvent)) 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT WinSATEvents::Init()
{    
    m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hEvent == INVALID_HANDLE_VALUE)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

HRESULT WinSATEvents::WaitForCompletion()
{   
    if (WaitForSingleObject(m_hEvent, INFINITE) != WAIT_OBJECT_0) 
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return S_OK;
}

WinSATEvents::~WinSATEvents()
{
    if (m_hEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hEvent);
        m_hEvent = INVALID_HANDLE_VALUE;
    }
}

//
// A class to initialize COM.  The destructor guarantees that COM
// is cleaned up on all error paths.
//
class InitializeCOM
{
private:
   HRESULT  hr;
   bool bInitSucceeded;
public:
   InitializeCOM()
   {
       if (FAILED(hr = CoInitialize(NULL)))
       {
            wprintf(L"Failed to initialize COM, error is %08.8lx", hr);
            bInitSucceeded = false;
       }
       else
       {
            bInitSucceeded = true;
       }
   }

   ~InitializeCOM()
   {
       if (bInitSucceeded) 
       {
            CoUninitialize();
       }
   }

   bool Succeeded()
   {
       return bInitSucceeded;
   }

};

//
// Need a COM module because we are implementing a COM object, WinSATEvents 
//
CComModule _Module;

//
// Main function
//
int _tmain(int argc, TCHAR* argv[])
{
    InitializeCOM initializeCOM;
    CComPtr<IInitiateWinSATAssessment> pInitiateWinsat;
	CComObject<WinSATEvents> * winsatEvents;
    CComPtr<IWinSATInitiateEvents> pWinsatEvents;
    LPCWSTR pArgs = NULL;
    HRESULT hr;     

    // Clheck for arguments
    if (argc == 2)
    {
        pArgs = argv[1];        
    } 
    else if (argc != 1) 
    {
        wprintf(L"Usage: InitiateAssessment.exe [arguments]\n");
		wprintf(L"If no arguments are provided, then a formal assessment is run.\n");
		wprintf(L"The first argument, if it exists, is passed to initiate.\n");
		wprintf(L"Use quotes to pass multiple arguments to WinSAT.\n");
        return 0;
    }    

    if (!initializeCOM.Succeeded()) 
    {
        return -1;
    }
    
	if (FAILED(hr = CreateInitiateAssessment(&pInitiateWinsat)))
	{
		wprintf(L"Error creating Winsat Initiate, error is %08.8lx", hr);
		return -1;
	}

    // Setup the control-C handler to cancel the assessment
    pInitiateWinsatForCancel = pInitiateWinsat;
	if (SetConsoleCtrlHandler( ConsoleControlHandlerRoutine, TRUE ) == 0)
	{
        wprintf(L"Can not set control C handler routine");
        return -1;
    }

    // Create an instance of the Winsat Events handler
    // If we failed to initialize the object, then the object is deleted
    // Otherwise, we assign it to a safe pointer so that it is 
    // cleaned up properly on exit.
    if (FAILED(hr = CComObject<WinSATEvents>::CreateInstance(&winsatEvents)))
    {
	    wprintf(L"Failed to Create WinsatEvents, error is %08.8lx", hr);
        return -1;
    }
  
    if (FAILED(hr = winsatEvents->Init())) 
    {
       wprintf (L"Failed to Init WinsatEvents, error is %08.8lx", hr);
       delete winsatEvents;
       return -1;
    }
    pWinsatEvents = winsatEvents;      

    // Use the API to initiate WinSAT 
    // If no parameters were specified, then perform a formal assessment
    // otherwise, initiate with parameters specified
    if (pArgs) 
    {
        hr = pInitiateWinsat->InitiateAssessment( pArgs,                
                                                  pWinsatEvents,
                                                  NULL );
    }  
    else 
    {
        hr = pInitiateWinsat->InitiateFormalAssessment(  pWinsatEvents, 
                                                         NULL );
    }    

    if (FAILED(hr))
    {
       wprintf (L"Failed to InitiateAssessment, error is %08.8lx", hr);       
       return -1;
    } 

    if (FAILED(hr = winsatEvents->WaitForCompletion())) 
    {
        wprintf (L"Failed while waiting completion, error is %08.8lx", hr);
        return -1;
    }      
    return 0;
}
