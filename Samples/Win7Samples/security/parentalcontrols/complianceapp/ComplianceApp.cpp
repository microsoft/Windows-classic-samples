
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    FILE: Windows Parental Controls (WPC) Compliance Application sample

    PURPOSE: Shows simple flow of Compliance API usage for an application
      implementing HTTP-based communication, and not needing any, or at 
      least infrequent, access to the Parental Controls WMI-based settings 
      policy management interfaces.

    FUNCTIONS:

        wmain() - implements overall command line application

    COMMENTS:
        Sample may easily be modified for time, games, or application 
        restrictions awareness.

****************************************************************************/

#include "ComplianceApp.h"

HRESULT EventParseXML(IXMLDOMDocument2* pXMLDom);

DWORD WINAPI EventCallBack(EVT_SUBSCRIBE_NOTIFY_ACTION, PVOID, EVT_HANDLE hEvent);


// Application entry point
int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{

    HRESULT hr = WpcuCOMInit();
    if (FAILED(hr))
    {
        wprintf(L"Error:  Failed to initialize COM, hr is %8x.\n", hr);
    }
    else
    {
        // Obtain Compliance interface
        IWindowsParentalControls* pWPC = NULL;
        hr = CoCreateInstance(__uuidof(WindowsParentalControls), 0, CLSCTX_INPROC_SERVER, 
                    __uuidof(IWindowsParentalControls), (LPVOID *)&pWPC);
        if (FAILED(hr))
        {
            wprintf(L"Info:  Parental Controls interface not detected.\n");
            wprintf(L"Info:   This is an error if on a supported SKU of Windows Vista.\n");

            // Applications needing parental controls functionality on unsupported 
            //  operating system versions will use alternative implementations
        }
        else
        {
            // Obtain WpcUserSettings interface for current user.  Passing in a
            // NULL SID pointer does this.
            IWPCSettings* piWPCSettings = NULL;
            hr = pWPC->GetUserSettings(NULL, &piWPCSettings);
            if (FAILED(hr))
            {
                wprintf(L"Warning:  Unable to obtain the Parental Controls user\n");
                wprintf(L"          settings interface.  This is expected if the\n");
                wprintf(L"          current user is a Protected Administrator or\n");
                wprintf(L"          Built-In Administrator.\n");
            }
            else
            {
                // Have prerequisites for Parental Controls restricted account.
                //  Check if logging is required
                BOOL fLoggingRequired = FALSE;
                hr = piWPCSettings->IsLoggingRequired(&fLoggingRequired);
                if (FAILED(hr))
                {
                    wprintf(L"Error:  IsLoggingRequired() failed, hr is %8x\n", hr);
                }
                else
                {
                    if (fLoggingRequired)
                    {
                        // Set internal state for application to log activity

                        // Optional:  get restrictions state
                        DWORD dwRestrictions;
                        hr = piWPCSettings->GetRestrictions(&dwRestrictions);
                        if (FAILED(hr))
                        {
                            wprintf(L"Error:  GetRestrictions() failed, hr is %8x\n",
                                hr);
                        }
                        else
                        {
                            if (dwRestrictions & WPCFLAG_WEB_FILTERED)
                            {
                                // Take any necessary actions for web filtering of HTTP 
                                //  traffic
                                wprintf(L"Info:  HTTP filtering for user is on\n");
                            }
                            else
                            {
                                wprintf(L"Info:  HTTP filtering for user is off\n");
                            }
                            if (dwRestrictions & WPCFLAG_HOURS_RESTRICTED)
                            {
                                // Take action to monitor logout warning events
                                wprintf(L"Info:  Time restrictions for user are on - \
                                         monitoring for logout events\n");
                                // Set up an Event Tracing for Windows subscription to 
                                //  the winlogon channel
                                HANDLE hSubWinlogon = EvtSubscribe(NULL, 
                                                       NULL, 
                                                       L"Microsoft-Windows-Winlogon/Operational", 
                                                       L"Event/System[EventID=1001] and \
                                                        Event/System/Provider[@Name='Microsoft-Windows-Winlogon'] and \
                                                        Event/System/Provider[@Guid='{dbe9b383-7cf3-4331-91cc-a3cb16a3b538}'] \
                                                        and Event/EventData/Data[@Name='TimeLeft']", 
                                                       NULL, 
                                                       NULL, 
                                                       EventCallBack, 
                                                       EvtSubscribeToFutureEvents);
                                if (!hSubWinlogon)
                                {
                                    hr = HRESULT_FROM_WIN32(GetLastError());
                                    wprintf(L"Error: EvtSubscrible failed, hr is 0x08%x\n", hr);
                                }
                                else
                                {
                                    // Simple spin loop waiting for logout warning callback
                                    wprintf(L"Hit any key to exit\n");
                                    while (!_kbhit())
                                    {
                                        Sleep(10);
                                    }
                                    // Clean up event subscription
                                    EvtClose(hSubWinlogon);
                                }
                            }
                            else
                            {
                                wprintf(L"Info:  Time restrictions for user are off\n");
                            }
                        }
                    }
                }
                piWPCSettings->Release();
            }
            pWPC->Release();
        }
    }
    WpcuCOMCleanup();

    return (SUCCEEDED(hr)) ? 0 : 1;
}

// Runs in calling thread context
DWORD WINAPI EventCallBack(EVT_SUBSCRIBE_NOTIFY_ACTION, PVOID, EVT_HANDLE hEvent)
{
    HRESULT hr = S_OK;
    DWORD cb = 0;
    DWORD dwPropCount = 0;
    
    // Call once to get the required size
    EvtRender(NULL, hEvent, EvtRenderEventXml, cb, NULL, &cb, &dwPropCount);

    // Allocate an array to the required size
    WCHAR* pszBuff = (WCHAR*)(new BYTE[cb]);
    if (!pszBuff)
    {
        hr = E_OUTOFMEMORY;
        wprintf(L"Error:  Out of memory rendering event\n");
    }
    else
    {
        if (!EvtRender(NULL, hEvent, EvtRenderEventXml, cb, pszBuff, &cb, &dwPropCount))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            wprintf(L"Error:  EvtRender failed 0x%08x\n", hr);
        }
        else
        {

            // Initialize COM
            hr = CoInitialize(NULL);
            if (FAILED(hr))
            {
                wprintf(L"Error:  CoInitialize() failed, hr is 0x%08x\n", hr);
            }
            else
            {
                // Create a DOM Document for the ETW event, then load and parse it
                IXMLDOMDocument2* pXMLDocument;

                hr = CoCreateInstance(CLSID_DOMDocument60, 0, CLSCTX_INPROC_SERVER, 
                    IID_IXMLDOMDocument2, (LPVOID *) &pXMLDocument);
                if (FAILED(hr))
                {
                    wprintf(L"Error:  CoCreate of DOMDOcument60 failed, hr is 0x%08x\n", hr);
                }
                else
                {
                    BSTR bstrXML = SysAllocString(pszBuff);
                    if (!bstrXML)
                    {
                        hr = E_OUTOFMEMORY;
                        wprintf(L"Error:  insufficient memory for XML string allocation\n");
                    }
                    else
                    {
                        VARIANT_BOOL varfSuccess;
                        hr = pXMLDocument->loadXML(bstrXML, &varfSuccess);
                        if (FAILED(hr) || varfSuccess != VARIANT_TRUE)
                        {
                            wprintf(L"Error:  loadXML() failed, hr is 0x%08x\n", hr);
                        }
                        else
                        {
                            hr = EventParseXML(pXMLDocument);
                            
                        }
                        SysFreeString(bstrXML);
                    }
                    pXMLDocument->Release();
                }
                CoUninitialize();
            }
        }
        delete [] pszBuff;
    }
   
    return ERROR_SUCCESS;
}

// Runs in callback thread's context
HRESULT EventParseXML(IXMLDOMDocument2* pXMLDom)
{
    HRESULT hr;
    BSTR bstrNameSpace = SysAllocString(L"xmlns:event='http://schemas.microsoft.com/win/2004/08/events/event'");
    BSTR bstrSelectionNamespaces = SysAllocString(L"SelectionNamespaces");
    if (!bstrNameSpace || !bstrSelectionNamespaces)
    {
        hr = E_OUTOFMEMORY;
        wprintf(L"Error:  insufficient memory for namespace string allocations\n");
    }
    else
    {
        VARIANT var;
        var.vt = VT_BSTR;        
        var.bstrVal = bstrNameSpace;
        // Set the namespace
        hr = pXMLDom->setProperty(bstrSelectionNamespaces, var);
        if (SUCCEEDED(hr))
        {
            BSTR bstrTimeLeftXPath = SysAllocString(L"event:Event/event:EventData/event:Data[@Name='TimeLeft']");
            if (!bstrTimeLeftXPath)
            {
                hr = E_OUTOFMEMORY;
                wprintf(L"Error:  insufficient memory for time remaining string allocation\n");
            }
            else
            {
                IXMLDOMNode* piNode = NULL;
                hr = pXMLDom->selectSingleNode(bstrTimeLeftXPath, &piNode);
                if (hr == S_OK)
                {
                    BSTR bstrTimeLeft;
                    hr = piNode->get_text(&bstrTimeLeft);
                    if (hr == S_OK)
                    {
                        wprintf(L"Info:  Warning received - time remaining is %s minutes\n", bstrTimeLeft);
                        SysFreeString(bstrTimeLeft);
                    }
                    else
                    {
                        wprintf(L"Error:  pNode->get_text failed 0x%08x\n", hr);
                    }
                    piNode->Release();
                }
                else
                {
                    wprintf(L"Error:  pNode->selectSingleNode failed 0x%08x\n", hr);
                }
                SysFreeString(bstrTimeLeftXPath);
            }
        }
        SysFreeString(bstrNameSpace);
        SysFreeString(bstrSelectionNamespaces);
    }
    
    return hr;
}