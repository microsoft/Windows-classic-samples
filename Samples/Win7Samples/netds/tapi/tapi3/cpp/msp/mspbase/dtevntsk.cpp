/*++

Copyright (c) 1998-1999  Microsoft Corporation

Module Name:

    DTEvntSk.cpp 

Abstract:

    This module contains implementation of CPTEventSink.

--*/

#include "precomp.h"
#pragma hdrstop


//
// a helper function that releases the resources allocated inside event info
//

HRESULT FreeEventInfo( MSP_EVENT_INFO * pEvent )
{

    LOG((MSP_TRACE, "FreeEventInfo -  enter"));

    switch ( pEvent->Event )
    {

        case ME_ADDRESS_EVENT:

            if (NULL != pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal)
            {
                (pEvent->MSP_ADDRESS_EVENT_INFO.pTerminal)->Release();
            }

            break;
    
        case ME_CALL_EVENT:

            if (NULL != pEvent->MSP_CALL_EVENT_INFO.pTerminal)
            {
                (pEvent->MSP_CALL_EVENT_INFO.pTerminal)->Release();
            }
    
            if (NULL != pEvent->MSP_CALL_EVENT_INFO.pStream)
            {
                (pEvent->MSP_CALL_EVENT_INFO.pStream)->Release();
            }
    
            break;
    
        case ME_TSP_DATA:

            break;

        case ME_PRIVATE_EVENT:
    
            if ( NULL != pEvent->MSP_PRIVATE_EVENT_INFO.pEvent )
            {
                (pEvent->MSP_PRIVATE_EVENT_INFO.pEvent)->Release();
            }

            break;

        case ME_FILE_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)
            {
                (pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal)->Release();
                pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pParentFileTerminal = NULL;
            }

            if( NULL != pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack )
            {
                (pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack)->Release();
                pEvent->MSP_FILE_TERMINAL_EVENT_INFO.pFileTrack = NULL;
            }

            break;

        case ME_ASR_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)
            {
                (pEvent->MSP_ASR_TERMINAL_EVENT_INFO.pASRTerminal)->Release();
            }

            break;

        case ME_TTS_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)
            {
                (pEvent->MSP_TTS_TERMINAL_EVENT_INFO.pTTSTerminal)->Release();
            }

            break;

        case ME_TONE_TERMINAL_EVENT:

            if( NULL != pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)
            {
                (pEvent->MSP_TONE_TERMINAL_EVENT_INFO.pToneTerminal)->Release();
            }

            break;

        default:

            break;
    }


    LOG((MSP_TRACE, "FreeEventInfo -  finished"));

    return S_OK;
}


CPTEventSink::CPTEventSink() :
    m_pMSPStream(NULL)
{
    LOG((MSP_TRACE, "CPTEventSink::CPTEventSink enter"));
    LOG((MSP_TRACE, "CPTEventSink::CPTEventSink exit"));
}

CPTEventSink::~CPTEventSink()
{
    LOG((MSP_TRACE, "CPTEventSink::~CPTEventSink enter"));
    LOG((MSP_TRACE, "CPTEventSink::~CPTEventSink exit"));
};

// --- ITPluggableTerminalEventSnk ---

/*++
FireEvent

Parameters:

    IN MSPEVENTITEM * pEventItem pointer to the structure that describes the 
    event. all the pointers contained in the structure must be addreffed by 
    the caller, and then released by the caller if FireEvent fails

    FireEvent makes a (shallow) copy of the structure, so the caller can 
    delete the structure when the function returns


Returns:
    S_OK - every thing was OK
    E_FAIL & other - something was wrong

Description:
  This method is called by the dynamic terminals to 
  signal a new event
--*/

STDMETHODIMP CPTEventSink::FireEvent(
    IN const MSP_EVENT_INFO * pEventInfo
    )
{
    LOG((MSP_TRACE, "CPTEventSink::FireEvent enter"));


    //
    // make sure we got a good mspeventitem structure
    //

    if( ! (void*)pEventInfo)
    {
        LOG((MSP_ERROR, "CPTEventSink::FireEvent -"
            "pEventItem is bad, returns E_POINTER"));
        return E_POINTER;
    }


    //
    // Create an MSPEVENTITEM
    //

    MSPEVENTITEM *pEventItem = AllocateEventItem();

    if (NULL == pEventItem)
    {
        LOG((MSP_ERROR, "CPTEventSink::FireEvent -"
            "failed to create MSPEVENTITEM. returning E_OUTOFMEMORY "));

        return E_OUTOFMEMORY;
    }


    //
    // make a shallow copy of the structure
    //

    pEventItem->MSPEventInfo = *pEventInfo;


    Lock();

    HRESULT hr = E_FAIL;

    if (NULL != m_pMSPStream)
    {
    
        //
        // nicely ask stream to process our event
        //

        LOG((MSP_TRACE, "CPTEventSink::FireEvent - passing event [%p] to the stream", pEventItem));


        AsyncEventStruct *pAsyncEvent = new AsyncEventStruct;

        if (NULL == pAsyncEvent)
        {
            LOG((MSP_ERROR, 
                "CPTEventSink::FireEvent - failed to allocate memory for AsyncEventStruct"));

            hr = E_OUTOFMEMORY;
        }
        else
        {

            //
            // stuff the structure with the addref'fed stream on which the 
            // event will be fired and the actual event to fire
            //

            ULONG ulRC =  m_pMSPStream->AddRef();

            if (1 == ulRC)
            {
                //
                // this is a workaround for a timing window: the stream could 
                // be in its desctructor while we are doing the addref. this 
                // condition is very-vary rare, as the timing window is very
                // narrow.
                //
                // the good thing is that stream destructor will not finish 
                // while we are here, because it will try to get event sink's 
                // critical section in its call to SetSinkStream() to set our 
                // stream pointer to NULL.
                // 
                // so if we detect that the refcount after our addref is 1, 
                // that would mean that the stream is in (or is about to start
                // executing its desctructor). in which case we should do 
                // nothing.
                //
                // cleanup and return a failure.
                //

                Unlock();

                LOG((MSP_ERROR, 
                    "CPTEventSink::FireEvent - stream is going away"));

                delete pAsyncEvent;
                pAsyncEvent = NULL;

                FreeEventItem(pEventItem);
                pEventItem = NULL;

                return TAPI_E_INVALIDSTREAM;
            }



            pAsyncEvent->pMSPStream = m_pMSPStream;

            pAsyncEvent->pEventItem = pEventItem;


            //
            // now use thread pool api to schedule the event for future async 
            // processing
            //

            BOOL bQueueSuccess = QueueUserWorkItem(
                CPTEventSink::FireEventCallBack,
                (void *)pAsyncEvent,
                WT_EXECUTEDEFAULT);

            if (!bQueueSuccess)
            {
                
                DWORD dwLastError = GetLastError();

                LOG((MSP_ERROR, 
                    "CPTEventSink::FireEvent - QueueUserWorkItem failed. LastError = %ld", dwLastError));


                //
                // undo the addref we did on the stream object. the event will 
                // be freed later
                //

                m_pMSPStream->Release();


                //
                // the event was not enqueued. delete now.
                //

                delete pAsyncEvent;
                pAsyncEvent = NULL;


                //
                // map the code and bail out
                //

                hr = HRESULT_FROM_WIN32(dwLastError);
            }
            else
            {
                
                //
                // log the event we have submitted, so we can match submission 
                // with processing from the log
                //

                LOG((MSP_TRACE,
                    "CPTEventSink::FireEvent - submitted event [%p]", pAsyncEvent));

                hr = S_OK;

            } // async event structure submitted

        } // async event structure allocated 

    } // msp stream exists
    else
    {
        hr = TAPI_E_INVALIDSTREAM;

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEvent - stream pointer is NULL"));
    }


    Unlock();


    //
    // if we don't have a stream, or if the stream refused to process the 
    // event, cleanup and return an error
    //

    if (FAILED(hr))
    {

        LOG((MSP_ERROR, "CPTEventSink::FireEvent - call to HandleStreamEvent failed. hr = 0x%08x", hr));

        FreeEventItem(pEventItem);

        return hr;
    }


    LOG((MSP_TRACE, "CPTEventSink::FireEvent - exit"));

    return S_OK;
}




/////////////////////////////////////////////////////////////////////////////
//
// CPTEventSink::FireEventCallBack
//
// the callback function that is called by thread pool api to asyncronously to
// process events fired by the terminals. 
// 
// the argument should point to the structure that contains the pointer to the 
// stream on which to fire the event and the pointer to the event to fire.
//
// the dll is guaranteed to not go away, since the structure passed in holds a 
// reference to the stream object on which to process the event
//

// static
DWORD WINAPI CPTEventSink::FireEventCallBack(LPVOID lpParameter)
{
    LOG((MSP_TRACE, "CPTEventSink::FireEventCallBack - enter. Argument [%p]", 
        lpParameter));


    AsyncEventStruct *pEventStruct = (AsyncEventStruct *)lpParameter;

    
    //
    // make sure the structure is valid
    //

    if (!pEventStruct)
    {

        //
        // complain and exit. should not happen, unless there is a problem in 
        // thread pool api or memory corruption
        //

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - Argument does not point to a valid AsyncEventStruct"));

        return FALSE;
    }


    BOOL bBadDataPassedIn = FALSE;

    //
    // the structure contains an addref'fed stream pointer. extract it and 
    // make sure it is still valid
    //

    CMSPStream *pMSPStream = pEventStruct->pMSPStream;

    if (!pMSPStream)
    {

        //
        // should not happen, unless there is a problem in thread pool api or 
        // memory corruption, or someone is over-releasing the stream object
        //

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - stream pointer is bad"));

        pMSPStream = NULL;

        bBadDataPassedIn = TRUE;
    }



    //
    // the structure contains the event that we are tryint to fire.
    // make sure the event we are about to fire is good.
    //

    MSPEVENTITEM *pEventItem = pEventStruct->pEventItem;

    if (!pEventItem)
    {

        //
        // should not happen, unless there is a problem in thread pool api or 
        // memory corruption, or we didn't check success of allocation when we 
        // created the event (which we did!)
        //

        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - event is bad"));

        pEventItem = NULL;

        bBadDataPassedIn = TRUE;
    }


    //
    // bad stream or event structure?
    //

    if (bBadDataPassedIn)
    {

        //
        // release the event if it was good.
        //

        if ( NULL != pEventItem)
        {

            FreeEventItem(pEventItem);
            pEventItem = NULL;
        }


        //
        // release the stream if it was good.
        //

        if (NULL != pMSPStream)
        {
            pMSPStream->Release();
            pMSPStream = NULL;
        }


        //
        // no need to keep the event structure itself, delete it
        //

        delete pEventStruct;
        pEventStruct = NULL;

        return FALSE;
    }

   
    //
    // we have both the stream and the event, fire the event on the stream
    //

    HRESULT hr = pMSPStream->HandleSinkEvent(pEventItem);


    //
    // if HandleSinkEvent succeeded, pEventItem will be released by whoever 
    // will handle the event, otherwise we need to release eventitem here
    //

    if (FAILED(hr))
    {
        LOG((MSP_ERROR, 
            "CPTEventSink::FireEventCallBack - HandleSinkEvent not called or failed. hr = %lx",
            hr));

        //
        // need to free all the resources held by event info
        //

        FreeEventInfo(&(pEventItem->MSPEventInfo));

        FreeEventItem(pEventItem);
        pEventItem = NULL;
    }


    //
    // release the stream pointer that is a part of the structure -- 
    // we don't want any reference leaks.
    //

    //
    // note that the dll may go away at this point (if we are holding the last 
    // reference to the last object from the dll)
    //

    pMSPStream->Release();
    pMSPStream = NULL;


    //
    // at this point we release the stream pointer and either submitted the 
    // event or freed it. we no longer need the event structure.
    //

    delete pEventStruct;
    pEventStruct = NULL;

    LOG((MSP_(hr), "CPTEventSink::FireEventCallBack - exit. hr = %lx", hr));
    
    return SUCCEEDED(hr);
}


/*++

SetSinkStream

Parameters:

    CMSPStream *pStream 
    
      the stream that will be processing our events, or NULL when no stream is 
      available to process our events

Returns:
    S_OK - 

Description:

    this method is called by the stream that is going to process our events

    when the stream is going away and is no longer available to process our 
    messages, it will call SetSinkStream with NULL.

--*/

HRESULT CPTEventSink::SetSinkStream( CMSPStream *pStream )
{
    LOG((MSP_TRACE, "CPTEventSink::SetSinkStream - enter"));


    Lock();


    LOG((MSP_TRACE, 
        "CPTEventSink::SetSinkStream - replacing sink stream [%p] with [%p]", 
        m_pMSPStream, pStream));


    //
    // we don't keep a reference to the stream -- the stream keeps a reference 
    // to us. when the stream goes away, it will let us know.
    //

    m_pMSPStream = pStream;

    Unlock();


    LOG((MSP_TRACE, "CPTEventSink::SetSinkStream - exit"));

    return S_OK;
}
