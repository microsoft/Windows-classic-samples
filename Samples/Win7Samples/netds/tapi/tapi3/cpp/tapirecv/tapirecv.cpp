
/*

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    TAPIRecv.cpp

Abstract:

    This sample illustrates the use of Media Streaming Terminal for receiving 
    audio.

    The application listens on addresses that support audio, accepts an 
    incoming call and uses Media Streaming Terminal to get the incoming 
    data, which is written into a wav file.
 
*/


#include "common.h"

#include "Processing.h"


//
// main thread will wait on this event before exiting
//

HANDLE g_hExitEvent = NULL;


//
// the tapi object
//

ITTAPI *g_pTapi = NULL;


//
// the current call
//

ITBasicCallControl *g_pCurrentCall = NULL;


//
// critical section for protecting the global current call
//

CRITICAL_SECTION g_CurrentCallCritSection;

//
// the cookie from registering tapi event notification
//

ULONG g_nTAPINotificationCookie = 0;


///////////////////////////////////////////////////////////////////////////////
// 
// LogMessage
//
//
// logs a message using printf
//
///////////////////////////////////////////////////////////////////////////////

void LogMessage(CHAR *pszFormat, ... )
{
    
    //
    // output buffer -- note: hardcoded limit
    //

    static int const BUFFER_SIZE = 1280;

    char szBuffer[BUFFER_SIZE]; 


    //
    // get current time
    //

    SYSTEMTIME SystemTime;

    GetLocalTime(&SystemTime);

    
    //
    // format thread id and time
    //

    StringCbPrintf( szBuffer, BUFFER_SIZE, "[%lx]:[%02u:%02u:%02u.%03u]::",
             GetCurrentThreadId(),
             SystemTime.wHour,
             SystemTime.wMinute,
             SystemTime.wSecond,
             SystemTime.wMilliseconds);


    size_t iStringLength = 0;

    HRESULT hr = StringCbLength(szBuffer, BUFFER_SIZE, &iStringLength);

    if (FAILED(hr))
    {
        // either this code is wrong, or someone else in the process corrupted 
        // our memory.
        return;
    }


    //
    // get the actual message
    //

    va_list vaArguments;

    va_start(vaArguments, pszFormat);


    size_t iBytesLeft = BUFFER_SIZE - iStringLength;


    //
    // not checking the return code of this function. even if it fails, we will
    // have a null-terminated string that we will be able to log.
    //

    StringCbVPrintf( &(szBuffer[iStringLength]), iBytesLeft, pszFormat, vaArguments);

    va_end(vaArguments);


    //
    // how big is the string now, and how many bytes do we have left?
    //

    hr = StringCbLength(szBuffer, BUFFER_SIZE, &iStringLength);

    if (FAILED(hr))
    {
        // either this code is wrong, or someone else in the process corrupted 
        // our memory.
        return;
    }

    iBytesLeft = BUFFER_SIZE - iStringLength;

    //
    // append a carriage return to the string. ignore the result code, the 
    // result string will be null-terminated no matter what.
    //

    StringCbCat(szBuffer, iBytesLeft, "\n");


    //
    // log the buffer 
    //

    printf(szBuffer);
}


///////////////////////////////////////////////////////////////////////////////
// 
// LogFormat
//
// use LogMessage to log wave format
//
///////////////////////////////////////////////////////////////////////////////

void LogFormat(const WAVEFORMATEX *pWaveFormat)
{
    LogMessage("    Format: ");
    LogMessage("        tag: %u", pWaveFormat->wFormatTag);
    LogMessage("        channels: %u", pWaveFormat->nChannels);
    LogMessage("        samples/sec: %lu", pWaveFormat->nSamplesPerSec);
    LogMessage("        align: %u", pWaveFormat->nBlockAlign);
    LogMessage("        bits/sample: %u", pWaveFormat->wBitsPerSample);

}



///////////////////////////////////////////////////////////////////////////////
//
// AllocateMemory
//
// use win32 heap api to allocate memory on the application's heap
// and zero the allocated memory
//
///////////////////////////////////////////////////////////////////////////////

void *AllocateMemory(SIZE_T nMemorySize)
{
    

    //
    // use HeapAlloc to allocate and clear memory
    //

    return HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, nMemorySize);
}


///////////////////////////////////////////////////////////////////////////////
//
// FreeMemory
//
// use win32 heap api to free memory previously allocated on the application's
// heap
//
///////////////////////////////////////////////////////////////////////////////


void FreeMemory(void *pMemory)
{
    
    //
    // get size of the allocated memory
    //

    SIZE_T nMemorySize = HeapSize(GetProcessHeap(), 0, pMemory);

    if (-1 == nMemorySize)
    {
        LogError("FreeMemory: failed to get size of the memory block %p",
                 pMemory);

        //
        // don't exit -- try freeing anyway
        //

    }
    else
    {
        //
        // fill memory with 0xdd's before freeing, so it is easier to debug 
        // failures caused by using pointer to deallocated memory
        //
        
        if (NULL != pMemory)
        {
            FillMemory(pMemory, nMemorySize, 0xdd);
        }

    }


    //
    // use HeapFree to free memory. use return code to log the result, but
    // do not return it to the caller
    //
    
    BOOL bFreeSuccess = HeapFree(GetProcessHeap(), 0, pMemory);

    if (FALSE == bFreeSuccess)
    {
        LogError("FreeMemory: HeapFree failed");

        //
        // if this assertion fires, it is likely there is a problem with the 
        // memory we are trying to deallocate. Was it allocated using heapalloc
        // and on the same heap? Is this a valid pointer?
        //

        _ASSERTE(FALSE);
    }

}


///////////////////////////////////////////////////////////////////////////////
//
// RegisterCallBack
//
//
// instantiate the callback object and register it with TAPI
//
///////////////////////////////////////////////////////////////////////////////

HRESULT RegisterCallback()
{
    LogMessage("RegisterCallback: started");

    HRESULT hr = S_OK;


    //
    // get connection point container on the tapi object
    // 
    
    IConnectionPointContainer *pCPContainer = NULL;

    hr = g_pTapi->QueryInterface(
                                IID_IConnectionPointContainer,
                                (void **)&pCPContainer
                               );

    if (FAILED(hr))
    {
        LogError("RegisterCallback: "
                 "Failed to get IConnectionPointContainer on TAPI");

        return hr;
    }


    //
    // find connection point for our interface
    //

    IConnectionPoint *pCP = NULL;

    hr = pCPContainer->FindConnectionPoint(IID_ITTAPIEventNotification, &pCP);

    pCPContainer->Release();
    pCPContainer = NULL;

    if (FAILED(hr))
    {
        LogError("RegisterCallback: "
                 "Failed to get a connecion point for ITTAPIEventNotification");

        return hr;
    }


    //
    // create the callback object and register it with TAPI.
    // for simplicity, the callback in this sample is not a 
    // full-fledged COM object. So create is with new.
    //

    ITTAPIEventNotification *pTAPIEventNotification = 
                                                new CTAPIEventNotification;

    if (NULL == pTAPIEventNotification)
    {

        LogError("RegisterCallback: Failed to create a tapi event notification object.");

        pCP->Release();
        pCP = NULL;
        
        return E_OUTOFMEMORY;
    }


    //
    // will use the cookie later to register for events from addresses
    // and to unregister the callback when we no longer needs the events
    //
    
    hr = pCP->Advise(pTAPIEventNotification,
                     &g_nTAPINotificationCookie);

    pCP->Release();
    pCP = NULL;


    //
    // whether Advise failed or succeeded, we no longer need a reference to 
    // the callback
    //

    pTAPIEventNotification->Release();
    pTAPIEventNotification = NULL;

    if (FAILED(hr))
    {
        g_nTAPINotificationCookie = 0;

        LogError("RegisterCallback: Failed to Advise");
    }


    LogMessage("RegisterCallback: completed");

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// UnRegisterCallBack
//
//
// unregister call notification object
//
///////////////////////////////////////////////////////////////////////////////

HRESULT UnRegisterCallBack()
{
    LogMessage("UnRegisterCallBack: started");

    HRESULT hr = S_OK;


    //
    // get connection point container on the tapi object
    // 
    
    IConnectionPointContainer *pCPContainer = NULL;

    hr = g_pTapi->QueryInterface(IID_IConnectionPointContainer,
                                (void **)&pCPContainer);

    if (FAILED(hr))
    {
        LogError("UnRegisterCallBack: "
                 "Failed to get IConnectionPointContainer on TAPI");

        return hr;
    }


    //
    // find connection point for our interface
    //

    IConnectionPoint *pConnectionPoint = NULL;

    hr = pCPContainer->FindConnectionPoint(IID_ITTAPIEventNotification,
                                           &pConnectionPoint);

    pCPContainer->Release();
    pCPContainer = NULL;

    if (FAILED(hr))
    {
        LogError("RegisterCallback: Failed to get a connection point for "
                 "ITTAPIEventNotification");

        return hr;
    }


    // 
    // unregister the callback
    //
    
    hr = pConnectionPoint->Unadvise(g_nTAPINotificationCookie);

    pConnectionPoint->Release();
    pConnectionPoint = NULL;

    g_nTAPINotificationCookie = 0;

    if (FAILED(hr))
    {
        LogError("UnRegisterCallBack: Unadvise failed");
    }
    else
    {
        LogMessage("UnRegisterCallBack: succeeded");
    }


    return hr;
}


///////////////////////////////////////////////////////////////////////////////
// 
// AddressSupportsAudio
//
// return TRUE if the address supports audio
//
///////////////////////////////////////////////////////////////////////////////

BOOL AddressSupportsAudio(IN ITAddress *pAddress)
{

    //
    // get the address's ITMediaSupport so we can check 
    // the media type it supports
    //

    ITMediaSupport *pMediaSupport = NULL;

    HRESULT hr = pAddress->QueryInterface(IID_ITMediaSupport, 
                                          (void**)&pMediaSupport);

    if (FAILED(hr))
    {
        LogError("AddressSupportsAudio: "
                 "Failed to QI address for ITMediaSupport");

        return FALSE;
    }


    //
    // does the address support audio?
    //

    VARIANT_BOOL bAudioSupported = VARIANT_FALSE;

    hr = pMediaSupport->QueryMediaType(TAPIMEDIATYPE_AUDIO, &bAudioSupported);
    
    pMediaSupport->Release();
    pMediaSupport = NULL;

    if (FAILED(hr))
    {
        LogError("AddressSupportsAudio: Failed to QueryMediaType");

        return FALSE;
    }



    if (VARIANT_TRUE == bAudioSupported)
    {
        LogMessage("AddressSupportsAudio: audio supported on this address");

        return TRUE;

    }
    else
    {
        LogMessage("AddressSupportsAudio: "
                   "audio NOT supported on this address");

        return FALSE;

    }
}


///////////////////////////////////////////////////////////////////////////////
// 
// AddressSupportsMST
//
// return TRUE if the address supports media streaming terminal
//
///////////////////////////////////////////////////////////////////////////////

BOOL AddressSupportsMST(IN ITAddress *pAddress)
{

    //
    // get the address's ITTerminalSupport so we can check
    // the terminals it supports
    //

    ITTerminalSupport *pTerminalSupport = NULL;

    HRESULT hr = pAddress->QueryInterface( IID_ITTerminalSupport,
                                           (void **)&pTerminalSupport );

    if (FAILED(hr))
    {
        LogError("AddressSupportsMST: Failed to QI address for ITTerminalSupport");

        return FALSE;
    }


    //
    // get enumeration of dynamic terminal classes this address supports
    //

    IEnumTerminalClass *pTerminalClassEnumerator = NULL;

    hr = pTerminalSupport->EnumerateDynamicTerminalClasses(&pTerminalClassEnumerator);

    pTerminalSupport->Release();
    pTerminalSupport = NULL;

    if (FAILED(hr))
    {

        LogError("AddressSupportsMST: Failed to get dynamic terminal classes");

        return FALSE;

    }


    //
    // walk through terminal class enumeration and see if the 
    // media streaming terminal is in it
    //

    while (TRUE)
    {
        
        GUID guid;
        
        hr = pTerminalClassEnumerator->Next(1, &guid, NULL);

        if (S_OK != hr)
        {
            LogMessage("AddressSupportsMST: no more terminal classes. "
                       "MST not supported on this address");

            break;
        }

        if ( IsEqualGUID(guid, CLSID_MediaStreamTerminal))
        {
            LogMessage("AddressSupportsMST: media streaming terminal supported");

            break;
        }
    }

    
    //
    // no longer need the enumeration
    //

    pTerminalClassEnumerator->Release();
    pTerminalClassEnumerator = NULL;

  
    //
    // if we did not break out of the loop because the enumeration ended,
    // media streaming terminal was not in the list of supported terminals
    //

    if (S_OK == hr)
    {
        return TRUE;
    }
    else 
    {
        return FALSE;
    }

}


///////////////////////////////////////////////////////////////////////////////
//
// ListenOnAddress
// 
// register to receieve notifications of calls on this addres.
// calls on this address will trigger events on the callback object
// we have registered with TAPI earlier.
//
///////////////////////////////////////////////////////////////////////////////

HRESULT ListenOnAddress(ITAddress *pAddress)
{
    
    long ulCallNotificationCookie = 0;


    //
    // we want to be notified of the status of this address
    //
    // the app can keep the cookies to later unregister from events
    // from specific addresses (by calling ITTAPI::UnregisterNotifications)
    // when it no longer wants to receive events for this specific address.
    // this sample does not do this for simplicity.
    //

    HRESULT  hr= g_pTapi->RegisterCallNotifications(pAddress,
                                                   VARIANT_TRUE,
                                                   VARIANT_TRUE,
                                                   TAPIMEDIATYPE_AUDIO,
                                                   g_nTAPINotificationCookie,
                                                   &ulCallNotificationCookie
                                                   );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
//
// StartListening
//
// find all addresses that support audio and start listening on them
//
///////////////////////////////////////////////////////////////////////////////

HRESULT StartListening()
{

    HRESULT hr = S_OK;

    LogMessage("StartListening: started");
    

    //
    // enumerate available addresses
    // 
    
    IEnumAddress *pEnumAddress = NULL;

    hr = g_pTapi->EnumerateAddresses( &pEnumAddress );

    if (FAILED(hr))
    {
        LogError("StartListening: Failed to enumerate addresses");

        return hr;
    }


    //
    // this flag remains false until we succeded starting listening on at 
    // least one address
    //

    BOOL bListenStarted = FALSE;


    //
    // walk through all the addresses and start listening on the ones that
    // support audio
    //
    
    while (TRUE)
    {
        //
        // check the next address
        //

        ITAddress *pAddress = NULL;

        hr = pEnumAddress->Next(1, &pAddress, NULL);

        if (S_OK != hr)
        {

            //
            // no more addresses or error
            //

            break;
        }


        //
        // log the name of the address
        //

        BSTR bstrAddressName;
        hr = pAddress->get_AddressName(&bstrAddressName);

        if (SUCCEEDED(hr))
        {

            LogMessage("StartListening: -> found address [%S]",
                        bstrAddressName);

            SysFreeString(bstrAddressName);
        }
        else
        {
            LogError("StartListening: failed to get address name");
        }


        //
        // if the address supports audio and media streaming terminal, 
        // start listening
        //

        if ( AddressSupportsAudio(pAddress) && AddressSupportsMST(pAddress) )
        {

            //
            // start listening on this address
            // 

            LogMessage("StartListening: Starting listening.");

            hr = ListenOnAddress(pAddress);

            if (SUCCEEDED(hr))
            {

                //
                // we are listening on at least one address
                //

                bListenStarted = TRUE;

                LogMessage("StartListening: "
                           "-> started listening on this address");

            }
            else
            {
                //
                // log an error and continue
                //

                LogError("StartListening: -> failed starting listening on this address, "
                         "hr = 0x%lx", hr);
            }
        }
        else
        {
            LogMessage("StartListening: -> no audio or MST support on this address.");
        }


        pAddress->Release();
        pAddress = NULL;

    }


    pEnumAddress->Release();
    pEnumAddress = NULL;


    //
    // depending on whether we started listening or not, log a message and
    // return the appropriate error code.
    //

    if (bListenStarted)
    {
        LogMessage("StartListening: completed. "
                   "Listening on one or more addresses");

        return S_OK;
    }
    else
    {
        LogMessage("StartListening: completed. Not listening on any address.");

        return E_FAIL;
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// InitializeTAPI
//
// create and initialize the tapi object
//
///////////////////////////////////////////////////////////////////////////////

HRESULT InitializeTAPI()
{
    
    LogMessage("InitializeTAPI: started");

    HRESULT hr = E_FAIL;
        

    //
    // cocreate the TAPI object
    //

    hr = CoCreateInstance(
                          CLSID_TAPI,
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_ITTAPI,
                          (LPVOID *)&g_pTapi
                         );

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: failed to CoCreateInstance TAPI");

        return hr;
    }


    //
    // cannot use tapi until it's initialized
    //

    hr = g_pTapi->Initialize();

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: TAPI failed to initialize");

        g_pTapi->Release();
        g_pTapi = NULL;
        
        return hr;
    }


    //
    // register the callback object that will receive tapi notifications
    //

    hr = RegisterCallback();

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: failed to register callback");

        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;
        
        return hr;
    }


    //
    // we want to be notified of these events:
    //
   
    hr = g_pTapi->put_EventFilter(TE_CALLNOTIFICATION |
                                  TE_CALLSTATE |
                                  TE_CALLMEDIA);

    if (FAILED(hr))
    {
        LogError("InitializeTAPI: Failed to put_EventFilter");

        //
        // unregister callback
        //

        UnRegisterCallBack();

        //
        // shutdown and release TAPI
        //

        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;

        return hr;
    }


    //
    // start listening on the addresses that support audio
    //

    hr = StartListening();

    if (S_OK != hr)
    {
        LogError("InitializeTAPI: Failed to start listening");

        //
        // unregister callback
        //

        UnRegisterCallBack();

        //
        // shutdown and release TAPI
        //

        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;

        return hr;
    }

    LogMessage("InitializeTAPI: succeeded");

    return S_OK;
}



///////////////////////////////////////////////////////////////////////////////
//
// DisconnectAndReleaseCall
// 
// disconnect and release the current call, if we have one
//
///////////////////////////////////////////////////////////////////////////////

void DisconnectAndReleaseCall()
{

    //
    // g_pCurrentCall can be accessed from event-processing thread and
    // needs to be protected
    //

    EnterCriticalSection(&g_CurrentCallCritSection);

    if (NULL != g_pCurrentCall)
    {

        //
        // if the call exists, attempt to disconnect it
        //

        LogMessage("DisconnectAndReleaseCall: disconnecting the call");

        g_pCurrentCall->Disconnect(DC_NORMAL);


        //
        // succeded or failed, release the call
        //

        g_pCurrentCall->Release();
        g_pCurrentCall = NULL;
    }

    
    LeaveCriticalSection(&g_CurrentCallCritSection);
}


///////////////////////////////////////////////////////////////////////////////
//
// ShutdownTapi
//
// - unregister callback object
// - shutdown and release TAPI object
//
///////////////////////////////////////////////////////////////////////////////

void ShutdownTapi()
{

    LogMessage("ShutdownTapi: started");

    
    // 
    // if there is still a call, disconnect it
    //

    HRESULT hr = E_FAIL;


    //
    // by now, the call is released and disconnected.
    // shutdown tapi
    //

    if (NULL != g_pTapi)
    {
        //
        // unregister callback
        //

        UnRegisterCallBack();


        //
        // shutdown and release the TAPI object
        //

        g_pTapi->Shutdown();
        g_pTapi->Release();
        g_pTapi = NULL;

    }

    
    //
    // in the unlikely case another call was received after we released the 
    // call that we had, release the new call.
    //
    
    EnterCriticalSection(&g_CurrentCallCritSection);

    if (NULL != g_pCurrentCall)
    {
        
        g_pCurrentCall->Release();
        g_pCurrentCall = NULL;

    }

    LeaveCriticalSection(&g_CurrentCallCritSection);

    LogMessage("ShutdownTapi: succeeded");
}


///////////////////////////////////////////////////////////////////////////////
//
// CtrlHandler
//
// set shutdown flag when the user requests exit by pressing
// ctrl+break, attempting to log off, close the window, or
// shutdown windows. this will give use a chance to exit gracefully.
//
///////////////////////////////////////////////////////////////////////////////

BOOL CtrlHandler(DWORD nEventType) 
{

    static BOOL bShutdownInProgress = FALSE;

    
    //
    // are we in the middle of shutting down?
    //

    if (TRUE == bShutdownInProgress)
    {
        LogMessage("CtrlHandler: Shutdown already in progress");

        return TRUE;
    }


    //
    // any exit event (close, ctrl+break/C, logoff, shutdown)
    // is a signal for the application to exit.
    //

    switch (nEventType)
    { 
 
        case CTRL_C_EVENT: 
        case CTRL_CLOSE_EVENT: 
        case CTRL_BREAK_EVENT: 
        case CTRL_LOGOFF_EVENT: 
        case CTRL_SHUTDOWN_EVENT:

            LogMessage("CtrlHandler: Initiating shutdown.");
            
            //
            // ignore subsequent shutdown requests...
            //

            bShutdownInProgress = TRUE;
            
            //
            // signal shutdown
            //

            SetEvent(g_hExitEvent);
            break;

        default: 
            break;

    } 

    return TRUE;
}
 

///////////////////////////////////////////////////////////////////////////////
//
// main
// 
///////////////////////////////////////////////////////////////////////////////

int __cdecl main(int argc, char* argv[])
{

    LogMessage("main: started");


    //
    // this event be signalled when it's time to exit
    //

    g_hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (NULL == g_hExitEvent)
    {
        LogError("main: Failed to create exit event");

        return 1;
    }

   

    //
    // we want to handle ctrl+c and ctrl+break events so we can cleanup on exit
    // proceed even in case of failure
    //

    SetConsoleCtrlHandler( (PHANDLER_ROUTINE)CtrlHandler, TRUE);


    //
    // initialize COM
    //

    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if ( FAILED(hr))
    {
        LogError("main: Failed to CoInitialize");

        CloseHandle(g_hExitEvent);
        g_hExitEvent = NULL;

        return 1;
    }


    //
    // start worker thread for tapi message processing
    //

    hr = g_WorkerThread.Initialize();

    if (FAILED(hr))
    {
        LogError("main: Failed to initialize worker thread");
        
        CoUninitialize();

        CloseHandle(g_hExitEvent);
        g_hExitEvent = NULL;

        return 1;
    }


    //
    // initialize critical section used to serialize access to global
    // current call
    //
    // note: InitializeCriticalSection can raise STATUS_NO_MEMORY exception
    //

    InitializeCriticalSection(&g_CurrentCallCritSection);

    //
    // create and initialize tapi object, register the callback object 
    // and start listening
    //
    
    hr = InitializeTAPI();

    if (FAILED(hr))
    {
        
        g_WorkerThread.Shutdown();


        CloseHandle(g_hExitEvent);
        g_hExitEvent = NULL;

        DeleteCriticalSection(&g_CurrentCallCritSection);

        CoUninitialize();
        return 1;
    }

    
    //
    // wait until ctrl+break handler or CS_DISCONNECT handler signal exit event
    //

    LogMessage("main: waiting for exit event");

    DWORD nWaitResult = WaitForSingleObject(g_hExitEvent, INFINITE);
    
    LogMessage("main: exit event signaled");


    //
    // disconnect and release call, if it exists
    //

    DisconnectAndReleaseCall();


    //
    // cleanup tapi
    //

    ShutdownTapi();


    //
    // stop the worker thread
    //

    g_WorkerThread.Shutdown();


    //
    // no longer need the critical section
    //

    DeleteCriticalSection(&g_CurrentCallCritSection);


    //
    // release com
    //
    
    CoUninitialize();


    //
    // no longer need the event
    //

    CloseHandle(g_hExitEvent);
    g_hExitEvent = NULL;


    //
    // exiting... we no longer want to handle ctrl+c and ctrl+break
    //
    
    SetConsoleCtrlHandler( (PHANDLER_ROUTINE) CtrlHandler, FALSE);


    //
    // was the event signaled, or wait failed?
    //

    if (WAIT_OBJECT_0 != nWaitResult)
    {
        LogError("main: Failed to wait for exit event");
        return 1;
    }


    LogMessage("main: exiting");

	return 0;
}

