//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#include "FaxNotify.h"
#include "FaxServerNotify.h"
#include "FaxAccountNotify.h"
#include "new"
#include <faxcomex_i.c>

DWORD g_dwThreadId = NULL;
CComModule _Module;

static DWORD TerminateMessageLoop(LPVOID ununsed);

//+---------------------------------------------------------------------------
//
//  function:   GiveUsage
//
//  Synopsis:   prints the usage of the application
//
//  Arguments:  [AppName] - Name of the application whose usage has to be printed
//
//  Returns:    void
//
//----------------------------------------------------------------------------

void GiveUsage(LPTSTR AppName)
{
        _tprintf( TEXT("Usage : %s \n \
					   /s Fax Server Name \n \
					   /o 1 for Account Notification or 0 for Server Notification \n"),AppName);
        _tprintf( TEXT("Usage : %s /? -- help message\n"),AppName);
}

//+---------------------------------------------------------------------------
//
//  function:   IsOSVersionCompatible
//
//  Synopsis:   finds whether the target OS supports this functionality.
//
//  Arguments:  [dwVersion] - Minimum Version of the OS required for the Sample to run.
//
//  Returns:    bool - true if the Sample can run on this OS
//
//----------------------------------------------------------------------------

bool IsOSVersionCompatible(DWORD dwVersion)
{
        OSVERSIONINFOEX osvi;
        BOOL bOsVersionInfoEx;

        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)    ;
        if( !bOsVersionInfoEx  )
        {
                osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
                if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
                        return false;
        }
        bOsVersionInfoEx = (osvi.dwMajorVersion >= dwVersion );
        return (bOsVersionInfoEx == TRUE);
}

//+---------------------------------------------------------------------------
//
//  function:   Initialize
//
//  Synopsis:   Initilizes the class members m_pFaxServer and m_pFaxAccount
//
//  Arguments:  none
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT CFaxNotify::Initialize(LPTSTR lptstrServerName)
{
        HRESULT hr = S_OK;
        m_lpwzServerName = L"";
        IFaxAccountSet* pFaxAccountSet;
        CComBSTR bstrServerName = SysAllocString(lptstrServerName);

        hr = m_pFaxServer.CoCreateInstance(CLSID_FaxServer);       
        if(FAILED(hr))
        {
                //CoCreateInstance failed.
                _tprintf(_T("CoCreateInstance failed. Error 0x%x \n"), hr);
                goto exit;
        }

        //connect to fax server.
        bstrServerName = SysAllocString(m_lpwzServerName);
        hr = m_pFaxServer->Connect(bstrServerName);
        if(FAILED(hr))
        {
                _tprintf(_T("Connect failed. Error 0x%x \n"), hr);
                goto exit;
        }

        FAX_SERVER_APIVERSION_ENUM enumFaxAPIVersion;
        hr = m_pFaxServer->get_APIVersion(&enumFaxAPIVersion);
        if(FAILED(hr))
        {
                //get_APIVersion failed.
                _tprintf(_T("get_APIVersion failed. Error 0x%x \n"), hr);
                goto exit;
        }

        if (enumFaxAPIVersion < fsAPI_VERSION_3) 
        {
                _tprintf(_T("Feature not available on this version of the Fax API"));
                goto exit;
        }        

        //lets also get the account set since that is the basis for all account relates operations
        hr = m_pFaxServer->get_FaxAccountSet(&pFaxAccountSet);
        if(FAILED(hr))
        {
                _tprintf(_T("get_FaxAccountSet failed. Error 0x%x \n"), hr);
                goto exit;
        }
        hr = pFaxAccountSet->GetAccount(NULL, &m_pFaxAccount);
        if (FAILED(hr))
        {
                _tprintf(_T(" GetAccount failed. Error 0x%x \n"), hr);
                goto exit;
        }
        BSTR bstrAccName;
        hr = m_pFaxAccount->get_AccountName(&bstrAccName);
        _tprintf(L"Account Name %s \n", bstrAccName);
exit:
        if(bstrServerName)
                SysFreeString(bstrServerName);
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   Listen
//
//  Synopsis:   Starts listening to Fax Server and User Account Events
//
//  Arguments:  none
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT CFaxNotify::Listen(BOOL bServerNotify)
{
        HRESULT hr = S_OK;
        DWORD dwActCPCookie = 0;
        BOOL fActListening = FALSE;
        CFaxAccountNotify *pActEventsSink = NULL;
        DWORD dwCPCookie = 0;
        BOOL fListening = FALSE;
        CFaxServerNotify *pEventsSink = NULL; 
        CComPtr<IUnknown> pServer =NULL;
        CComPtr<IUnknown> pAccount = NULL;

        if(bServerNotify)
        {
                pEventsSink = new (std::nothrow) CFaxServerNotify;
                if (!pEventsSink)
                {
                        hr = E_OUTOFMEMORY;
                        _tprintf(_T("new CFaxServerNotify failed. Error 0x%x \n"), hr);
                        goto exit;       
                }

                hr = m_pFaxServer->QueryInterface(&pServer);
                if (FAILED(hr))
                {
                        _tprintf(L"QueryInterface(IUnknown) failed. Error 0x%x \n", hr);
                        goto exit;             
                }

                hr = AttachToConnectionPoint(pServer, 
                                (IUnknown*)pEventsSink, 
                                DIID_IFaxServerNotify,
                                &dwCPCookie);
                if (FAILED(hr))
                {
                        _tprintf(L"AttachToConnectionPoint failed. Error 0x%x \n", hr);
                        goto exit;
                }
                else
                {
                        // Register for all events
                        FAX_SERVER_EVENTS_TYPE_ENUM eventTypes = 
                                (FAX_SERVER_EVENTS_TYPE_ENUM)(fsetIN_QUEUE |
                                                              fsetOUT_QUEUE |
                                                              fsetCONFIG |
                                                              fsetACTIVITY |
                                                              fsetQUEUE_STATE |
                                                              fsetIN_ARCHIVE |
                                                              fsetOUT_ARCHIVE |
                                                              fsetFXSSVC_ENDED |
                                                              fsetDEVICE_STATUS |
                                                              fsetINCOMING_CALL);

                        hr = m_pFaxServer->ListenToServerEvents(eventTypes);
                        if (FAILED(hr))
                        {
                                _tprintf(L"ListenToServerEvents failed. Error 0x%x \n", hr);
                                goto exit;
                        }
                        else
                        {
                                _tprintf(L"Successfully registered for fax server events!\n");
                                fListening = TRUE;
                        }                
                }
        }
        else
        {

                pActEventsSink = new (std::nothrow) CFaxAccountNotify;
                if (!pActEventsSink)
                {
                        hr = E_OUTOFMEMORY;
                        _tprintf(_T("new CFaxAccountNotify failed. Error 0x%x \n"), hr);
                        goto exit;
                }

                hr = m_pFaxAccount->QueryInterface(&pAccount);
                if (FAILED(hr))
                {
                        _tprintf(L"QueryInterface(IUnknown) failed. Error 0x%x \n", hr);
                        goto exit;
                }

                hr = AttachToConnectionPoint(pAccount, 
                                (IUnknown*)pActEventsSink, 
                                DIID_IFaxAccountNotify, 
                                &dwActCPCookie);
                if (FAILED(hr))
                {
                        _tprintf(L"AttachToConnectionPoint failed. Error 0x%x \n", hr);
                        goto exit;
                }
                else
                {
                        // Register for all events
                        FAX_ACCOUNT_EVENTS_TYPE_ENUM eventTypes = 
                                (FAX_ACCOUNT_EVENTS_TYPE_ENUM)(faetIN_QUEUE |
                                                               faetOUT_QUEUE |
                                                               faetIN_ARCHIVE |
                                                               faetOUT_ARCHIVE |
                                                               faetFXSSVC_ENDED);

                        hr = m_pFaxAccount->ListenToAccountEvents(eventTypes);
                        if (FAILED(hr))
                        {
                                _tprintf(L"ListenToAccountEvents failed. Error 0x%x \n", hr);  
                                goto exit;
                        }
                        else
                        {
                                _tprintf(L"Successfully registered for fax account events! \n");
                                fActListening = TRUE;
                        }
                }
        }

        if (fActListening || fListening)
        {
                g_dwThreadId = GetCurrentThreadId();
                // Create the thread to terminate the message loop after
                // few minutes
                HANDLE hThread  = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE) TerminateMessageLoop,
                                NULL,
                                0,
                                NULL);
                if (NULL == hThread)
                {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                        _tprintf(L"CreateThread failed. Error 0x%x \n", hr);  
                        goto exit;
                }
                else
                {
                        _tprintf(L"Thread to terminate message loop created successfully \n");
                        _tprintf(L"Test is now going to wait for 2 minutes during which ");
                        _tprintf(L"time it displays any fax events that may occur ... \n");
                        // Start the message loop
                        MSG msg;
                        BOOL bRet;
                        bRet = GetMessage( &msg, NULL, 0, 0 );
                        while(bRet)
                        {
                                if (bRet == -1)
                                {
                                        hr = HRESULT_FROM_WIN32(GetLastError());
                                        _tprintf(L"GetMessage failed. Error 0x%x \n", hr);  
                                        continue;
                                }                               

                                TranslateMessage( &msg );
                                DispatchMessage( &msg );
                        }
                }

                //Listen to no events from Fax Server
                if(fListening)
                {
                        hr = m_pFaxServer->ListenToServerEvents(fsetNONE);
                        if (FAILED(hr))
                        {
                                _tprintf(_T(" ListenToServerEvents(fsetNONE) failed. Error 0x%x \n"), hr);    
                                goto exit;
                        }                     
                        fListening = FALSE;
                }
                if (fActListening)
                {
                        hr = m_pFaxAccount->ListenToAccountEvents(
                                        (FAX_ACCOUNT_EVENTS_TYPE_ENUM)faetNONE);
                        if (FAILED(hr))
                        {
                                _tprintf(_T(" ListenToAccountEvents failed. Error 0x%x \n"), hr);
                                goto exit;
                        }
                        fActListening = FALSE;
                }

                //Unadvise Fax Server Notifications
                if (dwCPCookie)
                {


                        hr = DetachFromConnectionPoint(pServer, 
                                        DIID_IFaxServerNotify, 
                                        dwCPCookie);
                        if (FAILED(hr))
                        {
                                _tprintf(_T(" DetachFromConnectionPoint failed. Error 0x%x \n"), hr);
                                goto exit;
                        }

                }

                //Unadvise Fax Account Notifications
                if (dwActCPCookie)
                {                        
                        hr = DetachFromConnectionPoint(pAccount, 
                                        DIID_IFaxAccountNotify, 
                                        dwActCPCookie);
                        if (FAILED(hr))
                        {
                                _tprintf(_T(" DetachFromConnectionPoint failed. Error 0x%x \n"), hr);
                                goto exit;
                        }                  
                }

        }
exit:
        dwCPCookie = 0;
        dwActCPCookie = 0;
        if(pEventsSink)
                delete pEventsSink;
        pEventsSink = NULL;
        if(pActEventsSink)
                delete pActEventsSink;   
        pActEventsSink = NULL;
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   Terminate
//
//  Synopsis:   Terminates the fax conenction
//
//  Arguments:  none
//
//  Returns:    S_OK if successful
//
//----------------------------------------------------------------------------

HRESULT CFaxNotify::Terminate()
{
        HRESULT hr = S_OK;

        hr = m_pFaxServer->Disconnect();
        if (FAILED(hr))
        {
                _tprintf(_T(" Disconnect failed. Error 0x%x \n"), hr);
        }
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   TerminateMessageLoop
//
//  Synopsis:   This is a thread startup procedure that just waits for
//              2 minutes & posts a WM_QUIT message to thread identified
//              by g_dwThreadId in order to terminate that thread's
//              message loop
//
//  Arguments:  None
//
//  Returns:    0 if successful
//
//----------------------------------------------------------------------------
static DWORD TerminateMessageLoop(LPVOID ununsed)      
{
        // Sleep for 2 minutes & post a WM_QUIT message to
        // the message loop in the thread that executes that's
        // executing the main unit test
        Sleep(2 * 60 * 1000);
        if (!PostThreadMessage(g_dwThreadId, WM_QUIT, 0, 0))
        {
                HRESULT hr = HRESULT_FROM_WIN32(GetLastError());
                _tprintf(_T("PostThreadMessage failed. Error %x \n"), hr);
                return 0;
        }
        return 1;
}

//+---------------------------------------------------------------------------
//
//  function:   GetConnectionPoint
//
//  Synopsis:   Helper function to get the IConnectionPoint interface pointer
//              corresponding to the specified outgoing (source) interface
//
//  Arguments:  pSource : the connectable object (the one which can raise events)
//              riidOutgoingInterface : identifies the connection point on pSource
//                                       to advise to
//              ppIConnPoint : where the output is copied to  
//
//  Returns:    HRESULT : S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT GetConnectionPoint(CComPtr<IUnknown> pSource,
                REFIID riidOutgoingInterface,
                IConnectionPoint **ppIConnPoint)
{
        HRESULT hr = S_OK;
        CComPtr<IConnectionPointContainer> pConnPointContainer;
        CComPtr<IConnectionPoint> pConnPoint;

        hr = pSource->QueryInterface(&pConnPointContainer);
        if (FAILED(hr))
        {
                _tprintf(_T("QueryInterface(IConnectionPointContainer) failed. Error %x \n"), hr);
                goto exit;
        }

        hr = pConnPointContainer->FindConnectionPoint(riidOutgoingInterface,
                        &pConnPoint);
        if (FAILED(hr))
        {
                _tprintf(_T("FindConnectionPoint failed. Error %x \n"), hr);
                goto exit;
        }

        *ppIConnPoint = pConnPoint;
        (*ppIConnPoint)->AddRef();
exit:
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   AttachToConnectionPoint
//
//  Synopsis:   Helper method to attach or advise to a connection point
//  
//
//  Arguments:  pSource : the connectable object (the one which can raise events)
//              pSink   : the sink object that implements outgoing (source) interface
//                        which gets called by pSource
//              riidOutgoingInterface : identifies the connection point on pSource
//                                       to advise to 
//              lpdwCookie : same as the second parameter for Advise method
//
//  Returns:    HRESULT : S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT AttachToConnectionPoint(CComPtr<IUnknown> pSource,
                CComPtr<IUnknown> pSink,
                REFIID riidOutgoingInterface,
                LPDWORD lpdwCookie)
{
        HRESULT hr = S_OK;
        CComPtr<IConnectionPoint> pConnPoint;

        hr = GetConnectionPoint(pSource, riidOutgoingInterface, &pConnPoint);
        if (FAILED(hr))
        {
                _tprintf(_T("GetConnectionPoint failed. Error %x \n"), hr);
                goto exit;
        }
        hr = pConnPoint->Advise(pSink, lpdwCookie);
        if (FAILED(hr))
        {
                _tprintf(_T("Advise failed. Error %x \n"), hr);
                goto exit;
        }
exit:
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   DetachFromConnectionPoint
//
//  Synopsis:   This method is a wrapper for IConnectionPoint.UnAdvise
//  
//
//  Arguments:  pSource : the connectable object (the one which can raise events)
//              riidOutgoingInterface : identifies the connection point on pSource
//                                       to Unadvise from
//              dwCookie : identifies the connection that needs to be broken
//                         This should have been returned from Advise or 
//                         AttachToConnectionPoint method earlier
//
//  Returns:    HRESULT : S_OK if successful
//
//----------------------------------------------------------------------------
HRESULT DetachFromConnectionPoint(CComPtr<IUnknown> pSource,
                REFIID riidOutgoingInterface,
                DWORD dwCookie)
{
        HRESULT hr = S_OK;
        CComPtr<IConnectionPoint> pConnPoint;

        hr = GetConnectionPoint(pSource, riidOutgoingInterface, &pConnPoint);
        if (FAILED(hr))
        {
                _tprintf(_T("GetConnectionPoint failed. Error %x \n"), hr);
                goto exit;
        }
        hr = pConnPoint->Unadvise(dwCookie);
        if (FAILED(hr))
        {
                _tprintf(_T("Unadvise failed. Error %x \n"), hr);
                goto exit;
        }
exit:
        return hr;
}

//+---------------------------------------------------------------------------
//
//  function:   DetachFromConnectionPoint
//
//  Synopsis:   This method prints the status of a job
//
//  Arguments:  pJobStatus : Job status of a job
//
//  Returns:    void
//
//----------------------------------------------------------------------------
void DisplayJobStatus(CComPtr<IFaxJobStatus> pJobStatus)
{
        FAX_JOB_STATUS_ENUM status;
        FAX_JOB_EXTENDED_STATUS_ENUM statusEx;
        FAX_JOB_OPERATIONS_ENUM jobOps;
        FAX_JOB_TYPE_ENUM jobType;
        CComBSTR bstrTemp1, bstrTemp2;
        long lTemp1;
        long lTemp2;
        SYSTEMTIME stTemp1 = {0};
        SYSTEMTIME stTemp2 = {0};
        HRESULT hr = S_OK;
        GET_SIMPLE_PROPERTY(pJobStatus, get_Status, status, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_ExtendedStatusCode, statusEx, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_ExtendedStatus, bstrTemp1, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_CurrentPage, lTemp1, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_Pages, lTemp2, hr, exit);
        _tprintf( L"Status: 0x%x, Extended: %d (%s), CurrentPage/Total Pages: %d/%d",
                        status, statusEx, bstrTemp1, lTemp1, lTemp2);
        bstrTemp1.Empty();
        GET_SIMPLE_PROPERTY(pJobStatus, get_DeviceId, lTemp1, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_CSID, bstrTemp1, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_TSID, bstrTemp2, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_Size, lTemp2, hr, exit);
        _tprintf( L"Device ID: %d, CSID: %s, TSID: %s, Size: %dB", 
                        lTemp1, bstrTemp1, bstrTemp2, lTemp2);
        bstrTemp1.Empty();
        bstrTemp2.Empty();
        GET_SIMPLE_PROPERTY(pJobStatus, get_AvailableOperations, jobOps, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_JobType, jobType, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_Retries, lTemp1, hr, exit);
        GET_UTC_DATE_PROPERTY(pJobStatus,get_ScheduledTime,stTemp1,hr);
        if (FAILED(hr))
        {
                _tprintf( L"get_ScheduledTime failed");
        }

        _tprintf(L"Available Ops: 0x%x, Job Type: %d, Retries: %d, ScheduledTime: %2d:%2d",
                        jobOps, jobType, lTemp1, stTemp1.wHour, stTemp1.wMinute);
        GET_UTC_DATE_PROPERTY(pJobStatus,get_TransmissionStart,stTemp1,hr);
        if (FAILED(hr))
        {
                _tprintf( L"get_TransmissionStart failed");
        }
        GET_UTC_DATE_PROPERTY(pJobStatus,get_TransmissionEnd,stTemp2,hr);

        // Also check for hr != HRESULT_FROM_WIN32(ERROR_INVALID_DATA) because 
        // error_invalid_data means transmission end property is not available which is ok for 
        // cases where the job is still in progress
        if (FAILED(hr) && HRESULT_FROM_WIN32(ERROR_INVALID_DATA) != hr)
        {
                _tprintf( L"get_TransmissionEnd failed");
        }
        GET_SIMPLE_PROPERTY(pJobStatus, get_CallerId, bstrTemp1, hr, exit);
        GET_SIMPLE_PROPERTY(pJobStatus, get_RoutingInformation, bstrTemp2, hr, exit);
        _tprintf(L"Transmission Start->End: %2d:%2d -> %2d:%2d\nCallerID: %s, RoutingInfo: %s",
                        stTemp1.wHour, stTemp1.wMinute, stTemp2.wHour, stTemp2.wMinute,
                        bstrTemp1, bstrTemp2);
        bstrTemp1.Empty();
        bstrTemp2.Empty();

exit:
        return;
}



int  __cdecl _tmain(int argc, _TCHAR* argv[])
{
        HRESULT hr = S_OK;
        bool bRetVal = true;
        LPTSTR lptstrServerName = NULL;
        LPTSTR lptstrOption = NULL;        
        size_t argSize = 0;
        bool bVersion = IsOSVersionCompatible(VISTA);
        CFaxNotify* pNotify = NULL;
        bool bServerNotify = TRUE;

        //Check is OS is Vista
        if(bVersion == false)
        {
                _tprintf(_T("OS Version does not support this feature"));
                bRetVal = false;
                goto Exit1;
        }

        //introducing an artifical scope here so that the COm objects are destroyed before CoInitialize is called
        { 

                int argcount = 0;

#ifdef UNICODE
                argv = CommandLineToArgvW( GetCommandLine(), &argc );
#else
                argv = argvA;
#endif

                if (argc == 1)
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit1;
                }


                // check for commandline switches
                for (argcount=1; argcount<argc; argcount++)
                {                  
                        if(argcount + 1 < argc)
                        {
                                hr = StringCbLength(argv[argcount + 1],1024 * sizeof(TCHAR),&argSize);
                                if(!FAILED(hr))
                                {
                                        if ((argv[argcount][0] == L'/') || (argv[argcount][0] == L'-'))
                                        {
                                                switch (towlower(argv[argcount][1]))
                                                {
                                                        case 's':
                                                                //handling the case " /s fax1 /s fax2 "

                                                                if(lptstrServerName == NULL)
                                                                {
                                                                        lptstrServerName = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));
                                                                        if(lptstrServerName == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrServerName, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrServerName,argSize+1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrServerName: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;
                                                        case 'o':
                                                                //handling the case " /s fax1 /s fax2 "

                                                                if(lptstrOption == NULL)
                                                                {

                                                                        lptstrOption = (TCHAR*) malloc((argSize+1)* sizeof(TCHAR));                        
                                                                        if(lptstrOption == NULL)
                                                                        {
                                                                                _tprintf(_T("lptstrOption: malloc failed. Error %d \n"), GetLastError());
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                        memset(lptstrOption, 0, (argSize+1)* sizeof(TCHAR));
                                                                        hr = StringCchCopyN(lptstrOption,argSize +1, argv[argcount+1],argSize);
                                                                        if(FAILED(hr))
                                                                        {
                                                                                _tprintf(_T("lptstrOption: StringCchCopyN failed. Error 0x%x \n"), hr);
                                                                                bRetVal = false;
                                                                                goto Exit;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                        GiveUsage(argv[0]);
                                                                        bRetVal = false;
                                                                        goto Exit;
                                                                }
                                                                argcount++;
                                                                break;
                                                        case '?':
                                                                GiveUsage(argv[0]);
                                                                bRetVal = false;
                                                                goto Exit;                
                                                        default:
                                                                break;
                                                }//switch
                                        }//if
                                }
                        }
                }//for

                if (lptstrOption == NULL ) 
                {
                        _tprintf( TEXT("Missing args.\n") );
                        GiveUsage(argv[0]);
                        bRetVal = false;
                        goto Exit;
                }
                else
                {
                        if ((_tcscmp(_T("0"), lptstrOption) != 0)  && ((_tcscmp(_T("1"), lptstrOption) != 0)))
                        {
                                _tprintf( TEXT("Missing/Invalid Value.\n") );
                                GiveUsage(argv[0]);
                                bRetVal = false;
                                goto Exit;
                        }
                }


                //initialize COM
                hr = CoInitialize(NULL);
                if(FAILED(hr))
                {
                        //failed to init com
                        _tprintf(_T("Failed to init com. Error 0x%x \n"), hr);
                        bRetVal = false;
                        goto Exit;
                }
                _Module.Init(NULL, NULL , NULL);

                pNotify = new (std::nothrow) CFaxNotify();
                if (!pNotify)
                {
                        hr = E_OUTOFMEMORY;
                        _tprintf(TEXT("Cannot new CFaxNotify \n"));
                        bRetVal = false;
                        goto Exit;
                }

                hr = pNotify->Initialize(lptstrServerName);
                if (FAILED(hr))
                {
                        _tprintf( L"Initialize failed. Error %x \n", hr);
                        bRetVal = false;
                        goto Exit;
                }

                if(_tcscmp(_T("0"), lptstrOption) == 0)                
                {
                        bServerNotify = TRUE;
                }
                else
                {
                        bServerNotify = FALSE;
                }

                pNotify->Listen(bServerNotify);
                if (FAILED(hr))
                {
                        _tprintf( L"Listen failed. Error %x \n", hr);
                        bRetVal = false;
                        goto Exit;
                }
                pNotify->Terminate();
                if (FAILED(hr))
                {
                        _tprintf( L"Terminate failed. Error %x \n", hr);
                        bRetVal = false;
                        goto Exit;
                }
Exit:    
        if(lptstrServerName)
                free(lptstrServerName);
        if(lptstrOption)
                free(lptstrOption);        
        if(pNotify)
                delete pNotify;
        _Module.Term();
        }
        CoUninitialize();

Exit1:
        return bRetVal;
}

