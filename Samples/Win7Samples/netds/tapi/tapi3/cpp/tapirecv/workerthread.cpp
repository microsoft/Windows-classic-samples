/*

Copyright (c) 1999 - 2000  Microsoft Corporation

Module Name:

    WorkerThread.cpp

Abstract:

    Implementation of CWorkerThread class
 
*/


#include "common.h"
#include "WorkerThread.h"



///////////////////////////////////////////////////////////////////////////////
//
// CWorkerThread::Initialize
//
// create the thread
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CWorkerThread::Initialize()
{

    //
    // if the thread is already initialized 
    //

    if ( (NULL != m_hThreadHandle) && (0 != m_nThreadID) )
    {

        LogError("CWorkerThread::Initialize() thread is already running");

        return E_UNEXPECTED;
    }


    //
    // start the thread
    //
    
    m_hThreadHandle = CreateThread(NULL,        // security
                                   0,           // stack
                                   ThreadFunction,    // thread function
                                   0,           // thread parameter
                                   0,           // flags
                                   &m_nThreadID);

    //
    // thread creation failed. log a message and return an error
    // 

    if (NULL == m_hThreadHandle)
    {
        LogError("CWorkerThread::Initialize() "
                 "failed to start the worker thread");

        return E_FAIL;
    }


    //
    // thread created. log thread id.
    //
    
    LogMessage("CWorkerThread::Initialize() succeeded. thread id [0x%lx]",
                m_nThreadID);

    return S_OK;

}


///////////////////////////////////////////////////////////////////////////////
//
// CWorkerThread::Shutdown
//
// exit the thread and close thread handle
//
///////////////////////////////////////////////////////////////////////////////

HRESULT CWorkerThread::Shutdown()
{

     LogError("CWorkerThread::Shutdown() entered");

    //
    // if the thread is running, post quit message and wait for thread to exit
    //

    if (NULL != m_hThreadHandle)
    {
        
        //
        // let the thread know it's time to go home
        //

        PostMessage(WM_QUIT, NULL, NULL);
    

        //
        // wait for the thread to disappear
        //

        DWORD rc = WaitForSingleObject(m_hThreadHandle, INFINITE);
        
        //
        // close thread handle
        //

        CloseHandle(m_hThreadHandle);
        m_hThreadHandle = NULL;

        m_nThreadID = 0;


        if (rc != WAIT_OBJECT_0)
        {
            LogError("CWorkerThread::Shutdown() "
                     "Failed waiting for thread to shutdown");

            return E_FAIL;
        }
        else
        {

            LogMessage("CWorkerThread::Shutdown() finished");

            return S_OK;

        }

    }
    else
    {
        //
        // thread was not running
        //

        LogError("CWorkerThread::Shutdown() thread is not running");

        return E_FAIL;
    }

}


///////////////////////////////////////////////////////////////////////////////
//
// CWorkerThread::~CWorkerThread
//
// log a message and fire assert in the thread is still running
//
///////////////////////////////////////////////////////////////////////////////

CWorkerThread::~CWorkerThread()
{
    LogError("CWorkerThread::~CWorkerThread() entered");

    
    //
    // the thread should not be running at this point -- the app should have 
    // called Shutdown()
    // 
    
    if (NULL != m_hThreadHandle)
    {
        LogError("CWorkerThread::~CWorkerThread thread [%ld] is still running",
                  m_nThreadID);
    }

    _ASSERTE(NULL == m_hThreadHandle);
    

    LogError("CWorkerThread::~CWorkerThread() finished");
}


///////////////////////////////////////////////////////////////////////////////
//
// CWorkerThread::PostMessage
//
// posts a message to the thread
//
///////////////////////////////////////////////////////////////////////////////

BOOL CWorkerThread::PostMessage( UINT Msg,       // message to post
                                 WPARAM wParam,  // first message parameter
                                 LPARAM lParam   // second message parameter
                                 )
{
    
    BOOL bReturn = FALSE;


    if (NULL != m_hThreadHandle)
    {

        //
        // the thread is running, post the message
        //

        bReturn = PostThreadMessage(m_nThreadID, Msg, wParam, lParam);

    }


    return bReturn;
}


///////////////////////////////////////////////////////////////////////////////
//
// CWorkerThread::ThreadFunction
//
//
// thread function. receives messages posted to the thread. Forwards TAPI
// messages to TAPI event handler. Exits on WM_QUIT.
//
///////////////////////////////////////////////////////////////////////////////

unsigned long _stdcall CWorkerThread::ThreadFunction(void *)
{

    LogMessage("CWorkerThread::ThreadFunction() starting");


    CoInitializeEx(NULL, COINIT_MULTITHREADED);

    //
    // force creation of message queue
    // 

    MSG msg;


    //
    // keep retrieving messages until we fail or get WM_QUIT
    //
    
    while (GetMessage(&msg, NULL, 0, 0))
    {

        switch (msg.message)
        {
        
        case WM_PRIVATETAPIEVENT:
            {

                //
                // get event from the message
                //

                TAPI_EVENT TapiEvent = (TAPI_EVENT)msg.wParam;
                IDispatch *pEvent = (IDispatch*)msg.lParam;


                //
                // pass event to the event handler
                //
                
                OnTapiEvent(TapiEvent, pEvent);

                pEvent = NULL;


                //
                // do not check the error code from event handler - 
                // there is not much we can do here anyway.
                //
                
                //
                // Note: pEvent is released by OnTapiEvent, so we should not
                // use it after OnTapiEvent returns
                //


            }

            break;

        default:

            //
            // we are not interested in any other messages
            //

            break;
        }


    } // while (GetMessage())


    CoUninitialize();

    LogMessage("CWorkerThread::ThreadFunction() exiting");

    return 0;

} // CWorkerThread::ThreadFunction
