/*++

Copyright (c) 1998-1999 Microsoft Corporation

Module Name:

    mspthrd.cpp

Abstract:

    Implementation for MSP thread management classes.

--*/

#include "precomp.h"
#pragma hdrstop

#include <dbt.h>


CMSPThread g_Thread;

extern "C" DWORD WINAPI gfThreadProc(LPVOID p)
{
    return ((CMSPThread *)p)->ThreadProc();
}

HRESULT CMSPThread::Start()
/*++

Routine Description:

    Create the thread if it has not already been created. Otherwise, just
    keep track of how many times the thread start was performed so that
    we only stop the thread when all of these have been paired with a stop.

Arguments:
    
Return Value:

    HRESULT.

--*/
{
    LOG((MSP_TRACE, "CMSPThread::Start - enter"));

    CLock Lock(m_CountLock);

    if ( m_iStartCount == 0 )
    {

        _ASSERTE(m_hCommandEvent == NULL);
        _ASSERTE(m_hThread == NULL);


        TCHAR *ptczEventName = NULL;

#if DBG

        //
        // in debug build, use named events
        //

        TCHAR tszEventName[MAX_PATH];

        _stprintf_s(tszEventName,
            _T("CMSPThread_CommandEvent_pid[0x%lx]CMSPThread[%p]"),
            GetCurrentProcessId(), this);

        LOG((MSP_TRACE, "CMSPThread::Start - creating event[%S]", tszEventName));

        ptczEventName = &tszEventName[0];

#endif


        if ((m_hCommandEvent = ::CreateEvent(
            NULL, 
            FALSE,          // flag for manual-reset event
            FALSE,          // initial state is not set.
            ptczEventName   // No name in release builds, named in debug builds
            )) == NULL)
        {
            LOG((MSP_ERROR, "Can't create the command event"));
            return E_FAIL;
        }

        DWORD dwThreadID;
        m_hThread = ::CreateThread(NULL, 0, gfThreadProc, this, 0, &dwThreadID);

        if (m_hThread == NULL)
        {
            LOG((MSP_ERROR, "Can't create thread.  %ld", GetLastError()));
            return E_FAIL;
        }
    }

    m_iStartCount++;

    LOG((MSP_TRACE, "CMSPThread::Start - exit S_OK"));
    return S_OK;
}

HRESULT CMSPThread::Stop()
/*++

Routine Description:

    Stop the thread.

Arguments:
    
Return Value:

    HRESULT.

--*/
{
    LOG((MSP_TRACE, "CMSPThread::Stop - enter"));

    CLock Lock(m_CountLock);

    //
    // Complain if we get more Stops than Starts.
    //

    if ( m_iStartCount == 0 )
    {
        LOG((MSP_ERROR, "CMSPThread::Stop - thread already stopped - "
            "exit E_FAIL"));
        return E_FAIL;
    }

    //
    // Decrement the start count. Due to the above check we should
    // never go below zero.
    //

    m_iStartCount--;

    _ASSERTE( m_iStartCount >= 0 );

    //
    // If there have now been just as many stops as starts, it's time to stop
    // the thread.
    //

    if ( m_iStartCount == 0 )
    {
        //
        // Our state should be cleaned up from before.
        //

        _ASSERTE(m_hCommandEvent != NULL);
        _ASSERTE(m_hThread != NULL);

        //
        // Allocate a command queue item which we will pass to the thread.
        //

        COMMAND_QUEUE_ITEM * pItem = new COMMAND_QUEUE_ITEM;

        if ( ! pItem )
        {
            LOG((MSP_ERROR, "CMSPThread::Stop - allocate new queue item"));

            return E_OUTOFMEMORY;
        }

        pItem->node.cmd = STOP;

        //
        // Put the command queue item in the command queue.
        //

        m_QueueLock.Lock();
        InsertTailList(&m_CommandQueue, &(pItem->link));
        m_QueueLock.Unlock();

        //
        // Signal thread to process this stop command.
        //

        if (SignalThreadProc() == 0)
        {
            LOG((MSP_ERROR, "CMSPThread::Stop - can't signal the thread - "
                "exit E_FAIL"));

            return E_FAIL;
        }

        //
        // Wait until the thread stops
        //

        if (::WaitForSingleObject(m_hThread, INFINITE) != WAIT_OBJECT_0)
        {
            LOG((MSP_ERROR, "CMSPThread::Stop - timeout while waiting for the "
                "thread to stop"));
        }

        //
        // Clean up our state.
        //

        ::CloseHandle(m_hCommandEvent);
        ::CloseHandle(m_hThread);

        m_hCommandEvent     = NULL;
        m_hThread           = NULL;

    }

    LOG((MSP_TRACE, "CMSPThread::Stop - exit S_OK"));
    return S_OK;
}

HRESULT CMSPThread::Shutdown()
/*++

Routine Description:

    Unconditionally shutdown the thread. MSPs should by default use Stop()
    instead of Shutdwon(), unless they cannot do matched Start() / Stop()
    calls because of some other issue.

Arguments:
    
Return Value:

    HRESULT.

--*/
{
    LOG((MSP_TRACE, "CMSPThread::Shutdown - enter"));

    CLock Lock(m_CountLock);

    //
    // Ignore if we are not started.
    //

    if ( m_iStartCount == 0 )
    {
        LOG((MSP_ERROR, "CMSPThread::Shutdown - thread already stopped - "
            "exit S_OK"));
 
        return S_OK;
    }

    //
    // We are started, so stop now, irrespective of the outstanding start
    // count.
    //

    m_iStartCount = 1;

    HRESULT hr = Stop();
    
    LOG((MSP_(hr), "CMSPThread::Shutodwn - exit 0x%08x", hr));

    return hr;
}

HRESULT CMSPThread::ThreadProc()
/*++

Routine Description:

    the main loop of this thread.

Arguments:
    
Return Value:

    HRESULT.

--*/
{

    LOG((MSP_TRACE, "CMSPThread::ThreadProc - started"));


    BOOL bExitFlag = FALSE;

    m_hDevNotifyVideo = NULL;
    m_hDevNotifyAudio = NULL;
    m_hWndNotif = NULL;


    HRESULT hr = E_FAIL;

    if (FAILED(hr = ::CoInitializeEx(NULL, COINIT_MULTITHREADED)))
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - ConinitialzeEx failed:%x",
            hr));

        return hr;
    }


    //
    // Create a window to receive PNP device notifications. 
    //
    // since this is a base class that is used by more than one msp, we want 
    // to make sure that each msp registers a window class with a unique name
    // 
    // for this reason window class name is derived from threadid.
    //

    DWORD dwThreadID = GetCurrentThreadId();


    //
    // the string needs to be big enough to hold max dword number in hex + 
    // terminating zero. 20 is more than enough.
    //

    TCHAR szWindowClassName[20];

    _stprintf_s(szWindowClassName, _T("%lx"), dwThreadID);


    //
    // configure window class structure for RegisterClass
    //

    WNDCLASS wc;

    ZeroMemory(&wc, sizeof(wc));

    wc.lpfnWndProc = NotifWndProc;
    wc.lpszClassName = szWindowClassName;

    
    //
    // perform the actual registration
    //

    ATOM atomClassRegistration = 0;

    atomClassRegistration = RegisterClass(&wc);

    if (0 == atomClassRegistration)
    {
        LOG((MSP_ERROR, 
            "CMSPThread::ThreadProc - RegisterClass failed, last error %ld", 
            GetLastError()));
        
        hr = E_FAIL;
        goto exit;
    }
    

    //
    // create window that will receive pnp notifications
    //

    m_hWndNotif = CreateWindow(szWindowClassName, _T("MSP PNP Notification Window"), 0,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, NULL, this);

    if (m_hWndNotif == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - can't create notification window"));
        hr = E_FAIL;
        goto exit;
    }


    //
    // success
    //

    LOG((MSP_TRACE, "CMSPThread::ThreadProc - created notification window"));


    //
    // Register to receive PNP device notifications
    //
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = 
        sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = AM_KSCATEGORY_VIDEO;

    if ((m_hDevNotifyVideo = RegisterDeviceNotification( m_hWndNotif, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
        )) == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - can't register for video device notification"));
        hr = E_FAIL;
        goto exit;
    }

    NotificationFilter.dbcc_classguid = AM_KSCATEGORY_AUDIO;

    if ((m_hDevNotifyAudio = RegisterDeviceNotification( m_hWndNotif, 
        &NotificationFilter,
        DEVICE_NOTIFY_WINDOW_HANDLE
        )) == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::ThreadProc - can't register for audio device notification"));
        hr = E_FAIL;
        goto exit;
    }
    
    LOG((MSP_TRACE, "CMSPThread::ThreadProc - registered for PNP device notifications"));

    while (!bExitFlag)
    {
        //
        // Msg:      Grab window messages.
        // Multiple: We only use 1, but Msg and Ex only exist with Multiple.
        // Ex:       Allow flags, so we can pass in MWMO_ALERTABLE.
        //
        
        DWORD dwResult = ::MsgWaitForMultipleObjectsEx(
            1,                // wait for one event
            &m_hCommandEvent, // array of events to wait for
            INFINITE,         // wait forever
            QS_ALLINPUT,      // get all window messages
            MWMO_ALERTABLE    // get APC requests (in case this MSP uses them)
            );

        if ( ( dwResult == WAIT_OBJECT_0 ) || ( dwResult == WAIT_OBJECT_0 + 1 ) )
        {
            LOG((MSP_TRACE, "thread is signaled"));

            m_QueueLock.Lock();
            
            while ( ! IsListEmpty(&m_CommandQueue) )
            {

                LIST_ENTRY * links = RemoveHeadList( &m_CommandQueue );
                
                m_QueueLock.Unlock();
            
                COMMAND_QUEUE_ITEM * pItem =
                    CONTAINING_RECORD(links,
                                      COMMAND_QUEUE_ITEM,
                                      link);

                COMMAND_NODE * pNode = &(pItem->node);

                switch (pNode->cmd)
                {

                case WORK_ITEM:
                
                    LOG((MSP_TRACE, "CMSPThread::ThreadProc - "
                        "got command WORK_ITEM"));

                    pNode->pfn( pNode->pContext );

                    if ( pNode->hEvent != NULL )
                    {
                        if ( SetEvent( pNode->hEvent ) == 0 )
                        {
                            LOG((MSP_ERROR, "CMSPThread::ThreadProc - "
                                "can't signal event for synchronous work "
                                "item"));
                        }
                    }
                    break;

                case STOP:
                    
                    LOG((MSP_TRACE, "CMSPThread::ThreadProc - "
                        "thread is exiting"));

                    bExitFlag = TRUE;
                    break;
                }

                delete pItem;
            
                m_QueueLock.Lock();

            }
            m_QueueLock.Unlock();
            

            //
            // We have processed all commands and unblocked everyone
            // who is waiting for us. Now check for window messages.
            //

            MSG msg;

            // Retrieve the next item in the message queue.

            while ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if ( dwResult == WAIT_IO_COMPLETION )
        {
            // FEATUREFEATURE: The base MSP does not do anything with APC /
            // async I/O. If the derived MSP does something with it, they must
            // implement this to take appropriate action. Question is, how
            // best to expose this to the derived MSP? We could have a method
            // to override, but then how would the derived thread class get
            // instantiated? Instead, we'll have to have a method to set
            // an async I/O completion callback function pointer.
        }
        else
        {
            LOG((MSP_ERROR, "CMSPThread::ThreadProc - "
                "WaitForMultipleObjects failed  %ld", GetLastError()));

            break;
        }
    }

    hr = S_OK;

exit:

    

    //
    //  cleanup: 
    //
    //  Unregister from PNP device notifications
    //  destroy window
    //  unregister window class
    //  couninitialize
    //


    //
    // unregister from video pnp events if needed
    //

    if ( NULL != m_hDevNotifyVideo )
    {
        HRESULT hr2 = UnregisterDeviceNotification(m_hDevNotifyVideo);

        if (FAILED(hr2))
        {

            LOG((MSP_ERROR,
                "CMSPThread::ThreadProc - UnregisterDeviceNotification failed for video events. "
                "hr = %lx", hr2));
        }
    }

    
    //
    // unregister from audio pnp events if needed
    //

    if ( NULL != m_hDevNotifyAudio )
    {

        HRESULT hr2 = UnregisterDeviceNotification(m_hDevNotifyAudio);

        if (FAILED(hr2))
        {

            LOG((MSP_ERROR, 
                "CMSPThread::ThreadProc - UnregisterDeviceNotification failed for audio events. "
                "hr = %lx", hr2));
        }
    }

    
    //
    // destroy window if needed
    //

    if ( NULL != m_hWndNotif )
    {
        
        BOOL bDestroyWindowSuccess = DestroyWindow(m_hWndNotif);

        if ( ! bDestroyWindowSuccess )
        {
            LOG((MSP_ERROR, 
                "CMSPThread::ThreadProc - DestroyWindow failed. LastError = %ld",
                GetLastError()));
        }
    }


    //
    // unregister window class
    //

    if (0 != atomClassRegistration)
    {

        BOOL bUnregisterSuccess = UnregisterClass( (LPCTSTR)atomClassRegistration, ::GetModuleHandle(NULL) );

        if ( ! bUnregisterSuccess )
        {
            LOG((MSP_ERROR, 
                "CMSPThread::ThreadProc - UnregisterClass failed. LastError = %ld", 
                GetLastError()));
        }
    }


    ::CoUninitialize();

    LOG((MSP_(hr), "CMSPThread::ThreadProc - exit. hr = 0x%lx", hr));

    return hr;
}

HRESULT CMSPThread::QueueWorkItem(
    LPTHREAD_START_ROUTINE Function,
    PVOID Context,
    BOOL  fSynchronous
    )
{
    LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - enter"));


    //
    // Create a command block for this.
    //

    COMMAND_QUEUE_ITEM * pItem = new COMMAND_QUEUE_ITEM;

    if ( ! pItem )
    {
        LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
            "can't allocate new queue item - exit E_OUTOFMEMORY"));

        return E_OUTOFMEMORY;
    }


    //
    // Create an event to wait on if this is a synchronous work item.
    // Otherwise the thread proc gets a NULL event handle and it knows not to
    // signal it since it's an asynchronous work item.
    //

    TCHAR *ptczEventName = NULL;

#if DBG

    static LONG lSequenceNumber = 0;


    //
    // in debug build, use named events
    //

    TCHAR tszEventName[MAX_PATH];

    InterlockedIncrement(&lSequenceNumber);


    //
    // identify events by the address of the correspoding queue item, and by 
    // the sequence number
    //

    _stprintf_s(tszEventName,
        _T("CMSPThread_QueueWorkitemEvent_pid[0x%lx]_CMSPThread[%p]_Event[%p]_eventNumber[%lu]"),
        GetCurrentProcessId(), this, pItem, lSequenceNumber);

    LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - creating event[%S]", tszEventName));

    ptczEventName = &tszEventName[0];

#endif


    HANDLE hEvent = NULL;

    if (fSynchronous)
    {
        hEvent = ::CreateEvent(NULL, 
                               FALSE,           // flag for manual-reset event 
                               FALSE,           // initial state is not set.
                               ptczEventName);  // No name in release, named in debug

        if ( hEvent == NULL )
        {
            LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
                "Can't create the Job Done event"));

            delete pItem;
            pItem = NULL;

            return E_FAIL;
        }
    }


    //
    // we already have the q item, now initialize it.
    //

    pItem->node.cmd        = WORK_ITEM;
    pItem->node.pfn        = Function;
    pItem->node.pContext   = Context;
    pItem->node.hEvent     = hEvent;


    //
    // Put the command block on the queue. The queue is protected by a
    // critical section.
    //

    m_QueueLock.Lock();
    InsertTailList(&m_CommandQueue, &(pItem->link));


    //
    // Signal the thread to process the command.
    //

    if (SignalThreadProc() == 0)
    {

        //
        // failed to signal processing thread
        // cleanup and return error
        //

        
        //
        // remove the queue entry we have submitted
        //

        RemoveTailList(&m_CommandQueue);


        //
        // unlock the queue so other threads can use it
        //

        m_QueueLock.Unlock();


        //
        // close handle and delete pItem that we have created -- 
        // no one else is going to do this for us
        //

        if (NULL != hEvent)
        {
            ::CloseHandle(hEvent);
            hEvent = NULL;
        }

        delete pItem;
        pItem = NULL;


        LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
            "can't signal the thread"));

        return E_FAIL;
    }


    //
    // unlock the event queue, so it can be used by processing and other 
    // threads
    //

    m_QueueLock.Unlock();


    //
    // If this is a sychronous work item, wait for it to complete and
    // then close the event handle.
    //
    // FEATUREFEATURE: Rather than creating and deleting an event for each
    // work item, have a cache of events that can be reused.
    //

    if (fSynchronous)
    {
        LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - "
            "blocked waiting for synchronous work item to complete"));
        
        // Wait for the synchronous work item to complete.

        HANDLE hEvents[2];
        DWORD dwEvent;

        hEvents[0] = hEvent;
        hEvents[1] = m_hThread;

        dwEvent = WaitForMultipleObjects( 
            2,
            hEvents,
            FALSE,
            INFINITE);

        switch (dwEvent)
        {
        case WAIT_OBJECT_0 + 0:
            break;

        case WAIT_OBJECT_0 + 1:
            LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
                "thread exited"));

            //
            // if the item is still in the queue, remove it (since the thread 
            // won't)
            //

            m_QueueLock.Lock();
            
            if (IsNodeOnList(&m_CommandQueue, &(pItem->link)))
            {
                RemoveEntryList(&(pItem->link));
                delete pItem;
            }

            m_QueueLock.Unlock();
          

            //
            // time to close event and fail
            //

            ::CloseHandle(hEvent);

            return E_FAIL;        

        default:
            LOG((MSP_ERROR, "CMSPThread::QueueWorkItem - "
                "WaitForSingleObject failed"));
        }

        ::CloseHandle(hEvent);
    }

    LOG((MSP_TRACE, "CMSPThread::QueueWorkItem - exit S_OK"));

    return S_OK;
}

LRESULT CALLBACK CMSPThread::NotifWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{ 
    PNOTIF_LIST pnl;

    if (uMsg == WM_CREATE)
    {
        SetLastError(0);
        if (!SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)(((LPCREATESTRUCT)lParam)->lpCreateParams)))
        {
            if (GetLastError())  // It isn't really an error unless get last error says so
            {
                LOG((MSP_ERROR, "CMSPThread::NotifWndProc - SetWindowLongPtr failed %ld", GetLastError()));
                _ASSERTE(FALSE);
                return -1;
            }
        }
    }
    else
    {
        CMSPThread *me = (CMSPThread*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

        switch (uMsg) 
        { 
            case WM_DEVICECHANGE: 
                switch(wParam)
                {
                case DBT_DEVICEARRIVAL:
                    LOG((MSP_TRACE, "CMSPThread::NotifWndProc - DBT_DEVICEARRIVAL"));

                    me->m_NotifLock.Lock();
                
                    pnl = me->m_NotifList;
                    while (pnl != NULL)
                    {
                        pnl->addr->PnpNotifHandler(TRUE);
                        pnl = pnl->next;
                    }

                    me->m_NotifLock.Unlock();
                    break;

                case DBT_DEVICEREMOVECOMPLETE:
                    LOG((MSP_TRACE, "CMSPThread::NotifWndProc - DBT_DEVICEREMOVECOMPLETE"));

                    me->m_NotifLock.Lock();
                
                    pnl = me->m_NotifList;
                    while (pnl != NULL)
                    {
                        pnl->addr->PnpNotifHandler(FALSE);
                        pnl = pnl->next;
                    }

                    me->m_NotifLock.Unlock();
                    break;
                }

                return 0; 
 
            case WM_DESTROY: 
                return 0; 
 
            default: 
                return DefWindowProc(hwnd, uMsg, wParam, lParam); 
        } 
    }
    return 0; 
} 

HRESULT CMSPThread::RegisterPnpNotification(CMSPAddress *pCMSPAddress)
{
    PNOTIF_LIST pnl;
    HRESULT hr;

    if (!pCMSPAddress)
    {
        LOG((MSP_ERROR, "CMSPThread::RegisterPnpNotification - bad address pointer"));
        return E_POINTER;
    }
    
    m_NotifLock.Lock();

    // Add a new node to the list
    pnl = new NOTIF_LIST;

    if (pnl == NULL)
    {
        LOG((MSP_ERROR, "CMSPThread::RegisterPnpNotification - out of memory"));
        hr = E_OUTOFMEMORY;
    }
    else
    {


        //
        // note that we don't keep addref the address -- it is the 
        // caller's responsibility to ensure we are notified through 
        // UnregisterPnpNotification when the address is going away
        //

        pnl->next = m_NotifList;
        pnl->addr = pCMSPAddress;
        m_NotifList = pnl;
        hr = S_OK;
    }

    m_NotifLock.Unlock();
    return hr;
}

HRESULT CMSPThread::UnregisterPnpNotification(CMSPAddress *pCMSPAddress)
{
    PNOTIF_LIST pnl, pnlLast;
    HRESULT hr = E_FAIL;

    if (!pCMSPAddress)
    {
        LOG((MSP_ERROR, "CMSPThread::UnregisterPnpNotification - bad address pointer"));
        return E_POINTER;
    }
    
    m_NotifLock.Lock();

    pnl = m_NotifList;

    if ((pnl != NULL) && (pnl->addr == pCMSPAddress))
    {
        // It is fist in the list, remove it
        m_NotifList = pnl->next;
        delete pnl;

        hr = S_OK;
    }
    else while (pnl != NULL)
    {
        pnlLast = pnl;
        pnl = pnl->next;

        if ((pnl != NULL) && (pnl->addr == pCMSPAddress))
        {
            // Found it in the list, remove it
            pnlLast->next = pnl->next;
            delete pnl;

            hr = S_OK;

            break;
        }
    }

    if (pnl == NULL)
    {
        LOG(( MSP_WARN, "CMSPThread::UnregisterPnpNotification - address pointer not found in notification list." ));
        hr = E_FAIL;
    }

    m_NotifLock.Unlock();
    return hr;
}

// eof
