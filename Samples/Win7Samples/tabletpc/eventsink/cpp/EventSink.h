// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      EventSink.h
//
// Description:
//      The header file for the CMyInkEvents and CMyInkCollector used
//      by the EventSink sample. This file contains both the class
//     	definitions and the implementations of the methods.
//
//--------------------------------------------------------------------------
#pragma once

#include "resource.h"

/////////////////////////////////////////////////////////
//
// class CMyInkEvents
//
// The CMyInkEvents class overrides the default implementation
// of the stroke event handler specified in the class
// InkCollectorEvents.
//
// It is important to note that there are two steps in
// receiving an event using this class:
//
// 1. It is necessary to add an override of the event function
//    to this class.
//
// 2. Use IInkCollector::SetEventInterest to ensure that the
//    InkCollector generates the event.  Stroke, CursorInRange,
//    CursorOutOfRange are the only event interests that are on
//    by default.
//
/////////////////////////////////////////////////////////
class CMyInkEvents : public InkCollectorEvents
{
public:

    // Event: Stroke
    virtual void Stroke(
        IInkCursor* Cursor,
        IInkStrokeDisp* Stroke,
        VARIANT_BOOL *Cancel)
    {
        // Demonstrate that we received the event notification.
        MessageBox(m_hWnd, TEXT("Stroke Event"), TEXT("Event Received"), MB_OK);
    }

    CMyInkEvents()
    {
        m_hWnd = NULL;
    }

    HRESULT Init(
        HWND hWnd)
    {
        m_hWnd = hWnd;
        return InkCollectorEvents::Init();
    }

    HWND m_hWnd;
};

/////////////////////////////////////////////////////////
//
// class CMyInkCollector
//
// The CMyInkCollector class handles the connection between
// the window and the Ink Collector. It sets up the connection
// at start up and tears it down at exit.
//
/////////////////////////////////////////////////////////
class CMyInkCollector
{
public:

    // Constructor: just initialize memory
    CMyInkCollector()
    {
        m_pInkCollector = NULL;
    }

    // Destructor: if there is an InkCollector, then release
    //  all resources connected with that InkCollector.
    ~CMyInkCollector()
    {
        if (m_pInkCollector != NULL)
        {
            m_InkEvents.UnadviseInkCollector();
            m_pInkCollector->put_Enabled(VARIANT_FALSE);
            m_pInkCollector->Release();
        }
    }

    // Handle all initializaton
    HRESULT Init(
        HWND hWnd)
    {
        // Initialize event sink. This consists of setting
        //  up the free threaded marshaler.
        HRESULT hr = m_InkEvents.Init(hWnd);

        if (FAILED(hr))
        {
            return hr;
        }

        // Create the ink collector
        hr = CoCreateInstance(CLSID_InkCollector, NULL, CLSCTX_ALL,
            IID_IInkCollector, (void **) &m_pInkCollector);

        if (FAILED(hr))
        {
            return hr;
        }

        // Since this sample only handles the Stroke event, it is
        // not necessary to set the event interest (Stroke, CursorInRange,
        // CursorOutOfRange are the only event interests that are on by
        // default).  In order to handle an event that is not on by default,
        // it is necessary to set the event interest at this point.  The
        // following code illustrates how to listen for the NewPackets
        // event:
        //
        //hr = m_pInkCollector->SetEventInterest(ICEI_NewPackets, VARIANT_TRUE);
        //
        //if (FAILED(hr))
        //{
        //    return hr;
        //}

        // Set up connection between Ink Collector and our event sink
        hr = m_InkEvents.AdviseInkCollector(m_pInkCollector);

        if (FAILED(hr))
        {
            return hr;
        }

        // Attach Ink Collector to window
        hr = m_pInkCollector->put_hWnd((long) hWnd);

        if (FAILED(hr))
        {
            return hr;
        }

        // Allow Ink Collector to receive input.
        return m_pInkCollector->put_Enabled(VARIANT_TRUE);
    }

private:

    IInkCollector *m_pInkCollector;

    CMyInkEvents m_InkEvents;
};
