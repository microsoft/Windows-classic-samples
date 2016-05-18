// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      TpcConpt.h
//
// Description:
//      The header file for InkCollectorEvents, which is designed
//      to be used by C++ applications as a base class for handling
//      Ink Collector events. This base class is meant to be used
//      as a template;  application-specific event handling should
//      be handled by the derived class.  For an example of how to use
//      this class, refer to CMyInkEvents in EventSink.h.
//
//      This file contains both the class definitions and the implementation
//      of the methods.
//
//--------------------------------------------------------------------------

#ifndef TPCCONPT_H
#define TPCCONPT_H

/////////////////////////////////////////////////////////
//
// class InkCollectorEvents
//
// The InkCollectorEvents class handles passing Ink Collector
// events from the Ink Collector to the user of this class.
// It sets up the connection between the InkCollector and
// this class in AdviseInkCollector. In the Invoke method,
// it converts the IDispatch event notification into a
// call to a virtual function which the user of this class
// can override to process a particular event.
//
// Two important points to consider when using this class:
//
// 1.  You must do more than simply override the event's
// virtual function in order to handle the event. For
// all but default events, you will have to call
// IInkCollector::SetEventInterest to guarantee getting
// an event. Stroke, CursorInRange, andCursorOutOfRange are
// the only event interests that are on by default.
//
// 2.  This object marshals itself free threaded so all implemented
// event handlers need to be free threaded as well. Of particular
// importance is using Window's APIs, which may cause a switch to
// another thread; the event handler is not guaranteed
// to be running on the same thread as the window connected
// with the Ink Collector.
//
/////////////////////////////////////////////////////////
class InkCollectorEvents : public _IInkCollectorEvents
{
public:

    //
    // IUnknown Interface
    //
    HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject)
    {
        // Validate the input
        if (NULL == ppvObject)
        {
            return E_POINTER;
        }

        // This object only supports IDispatch/_IInkCollectorEvents
        if ((riid == IID_IUnknown)
            || (riid == IID_IDispatch)
            || (riid == DIID__IInkCollectorEvents))
        {
            *ppvObject = (IDispatch *) this;

            // Note: we do not AddRef here because the lifetime
            //  of this object does not depend on reference counting
            //  but on the duration of the connection set up by
            //  the user of this class.

            return S_OK;
        }
        else if (riid == IID_IMarshal)
        {
            // Assert that the free threaded marshaller has been
            // initialized.  It is necessary to call Init() before
            // invoking this method.
            assert(NULL != m_punkFTM);

            // Use free threaded marshalling.
            return m_punkFTM->QueryInterface(riid, ppvObject);
        }

        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef()
    {

        // Note: we do not AddRef here because the lifetime
        //  of this object does not depend on reference counting
        //  but on the duration of the connection set up by
        //  the user of this class.
        return 1;
    }

    virtual ULONG STDMETHODCALLTYPE Release()
    {

        // Note: we do not do Release here because the lifetime
        //  of this object does not depend on reference counting
        //  but on the duration of the connection set up by
        //  the user of this class.
        return 1;
    }

    //
    // IDispatch Interface
    //
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo)
    {
        // This method is not needed for processing events.
        return E_NOTIMPL;
    }

    STDMETHOD(GetTypeInfo)(
        UINT itinfo,
        LCID lcid,
        ITypeInfo** pptinfo)
    {
        // This method is not needed for processing events.
        return E_NOTIMPL;
    }

    STDMETHOD(GetIDsOfNames)(
        REFIID riid,
        LPOLESTR* rgszNames,
        UINT cNames,
        LCID lcid,
        DISPID* rgdispid)
    {
        // This method is not needed for processing events.
        return E_NOTIMPL;
    }

    // Invoke translates from IDispatch to an event callout
    //  that can be overriden by a subclass of this class.
    STDMETHOD(Invoke)(
        DISPID dispidMember,
        REFIID riid,
        LCID lcid,
        WORD /*wFlags*/,
        DISPPARAMS* pdispparams,
        VARIANT* pvarResult,
        EXCEPINFO* /*pexcepinfo*/,
        UINT* /*puArgErr*/)
    {
        switch(dispidMember)
        {
            case DISPID_ICEStroke:
                Stroke(
                    (IInkCursor*) pdispparams->rgvarg[2].pdispVal,
                    (IInkStrokeDisp*) pdispparams->rgvarg[1].pdispVal,
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_ICECursorDown:
                CursorDown(
                    (IInkCursor*) pdispparams->rgvarg[1].pdispVal,
                    (IInkStrokeDisp*) pdispparams->rgvarg[0].pdispVal);
                break;

            case DISPID_ICENewPackets:
                NewPackets(
                    (IInkCursor*) pdispparams->rgvarg[3].pdispVal,
                    (IInkStrokeDisp*) pdispparams->rgvarg[2].pdispVal,
                    pdispparams->rgvarg[1].lVal,
                    pdispparams->rgvarg[0].pvarVal);
                break;

            case DISPID_IPEDblClick:
                DblClick(
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_IPEMouseMove:
                MouseMove(
                    (InkMouseButton) pdispparams->rgvarg[4].lVal,
                    (InkShiftKeyModifierFlags) pdispparams->rgvarg[3].lVal,
                    pdispparams->rgvarg[2].lVal,
                    pdispparams->rgvarg[1].lVal,
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_IPEMouseDown:
                MouseDown(
                    (InkMouseButton) pdispparams->rgvarg[4].lVal,
                    (InkShiftKeyModifierFlags) pdispparams->rgvarg[3].lVal,
                    pdispparams->rgvarg[2].lVal,
                    pdispparams->rgvarg[1].lVal,
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_IPEMouseUp:
                MouseUp(
                    (InkMouseButton) pdispparams->rgvarg[4].lVal,
                    (InkShiftKeyModifierFlags) pdispparams->rgvarg[3].lVal,
                    pdispparams->rgvarg[2].lVal,
                    pdispparams->rgvarg[1].lVal,
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_IPEMouseWheel:
                MouseWheel(
                    (InkMouseButton) pdispparams->rgvarg[5].lVal,
                    (InkShiftKeyModifierFlags) pdispparams->rgvarg[4].lVal,
                    pdispparams->rgvarg[3].lVal,
                    pdispparams->rgvarg[2].lVal,
                    pdispparams->rgvarg[1].lVal,
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_ICENewInAirPackets:
                NewInAirPackets(
                    (IInkCursor*) pdispparams->rgvarg[2].pdispVal,
                    pdispparams->rgvarg[1].lVal,
                    pdispparams->rgvarg[0].pvarVal);
                break;

            case DISPID_ICECursorButtonDown:
                CursorButtonDown(
                    (IInkCursor*) pdispparams->rgvarg[1].pdispVal,
                    (IInkCursorButton*) pdispparams->rgvarg[0].pdispVal);
                break;

            case DISPID_ICECursorButtonUp:
                CursorButtonUp(
                    (IInkCursor*) pdispparams->rgvarg[1].pdispVal,
                    (IInkCursorButton*) pdispparams->rgvarg[0].pdispVal);
                break;

            case DISPID_ICECursorInRange:
                CursorInRange(
                    (IInkCursor*) pdispparams->rgvarg[2].pdispVal,
                    (VARIANT_BOOL) pdispparams->rgvarg[1].iVal,
                    pdispparams->rgvarg[0]);
                break;

            case DISPID_ICECursorOutOfRange:
                CursorOutOfRange(
                    (IInkCursor*) pdispparams->rgvarg[0].pdispVal);
                break;

            case DISPID_ICESystemGesture:
                SystemGesture(
                    (IInkCursor*) pdispparams->rgvarg[6].pdispVal,
                    (InkSystemGesture) pdispparams->rgvarg[5].lVal,
                    pdispparams->rgvarg[4].lVal,
                    pdispparams->rgvarg[3].lVal,
                    pdispparams->rgvarg[2].lVal,
                    pdispparams->rgvarg[1].bstrVal,
                    pdispparams->rgvarg[0].lVal);
                break;

            case DISPID_ICEGesture:
                Gesture(
                    (IInkCursor*) pdispparams->rgvarg[3].pdispVal,
                    (IInkStrokes*) pdispparams->rgvarg[2].pdispVal,
                    pdispparams->rgvarg[1],
                    (VARIANT_BOOL *)pdispparams->rgvarg[0].pboolVal);
                break;

            case DISPID_ICETabletAdded:
                TabletAdded(
                    (IInkTablet*) pdispparams->rgvarg[0].pdispVal);
                break;

            case DISPID_ICETabletRemoved:
                TabletRemoved(
                    pdispparams->rgvarg[0].lVal);
                break;

            default:
                break;
        }

        return S_OK;
    }

    //
    // Events
    //

    virtual void Stroke(
        IInkCursor* Cursor,
        IInkStrokeDisp* Stroke,
        VARIANT_BOOL *Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void CursorDown(
        IInkCursor* Cursor,
        IInkStrokeDisp* Stroke)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void NewPackets(
        IInkCursor* Cursor,
        IInkStrokeDisp* Stroke,
        long PacketCount,
        VARIANT* PacketData)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void DblClick(
        VARIANT_BOOL *Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void MouseMove(
        InkMouseButton Button,
        InkShiftKeyModifierFlags Shift,
        long pX,
        long pY,
        VARIANT_BOOL *Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void MouseDown(
        InkMouseButton Button,
        InkShiftKeyModifierFlags Shift,
        long pX,
        long pY,
        VARIANT_BOOL *Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void MouseUp(
        InkMouseButton Button,
        InkShiftKeyModifierFlags Shift,
        long pX,
        long pY,
        VARIANT_BOOL *Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void MouseWheel(
        InkMouseButton Button,
        InkShiftKeyModifierFlags Shift,
        long Delta,
        long X,
        long Y,
        VARIANT_BOOL *Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void NewInAirPackets(
        IInkCursor* Cursor,
        long lPacketCount,
        VARIANT* PacketData)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void CursorButtonDown(
        IInkCursor* Cursor,
        IInkCursorButton* Button)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void CursorButtonUp(
        IInkCursor* Cursor,
        IInkCursorButton* Button)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void CursorInRange(
        IInkCursor* Cursor,
        VARIANT_BOOL NewCursor,
        VARIANT ButtonsState)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void CursorOutOfRange(
        IInkCursor* Cursor)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void SystemGesture(
        IInkCursor* Cursor,
        InkSystemGesture Id,
        long X,
        long Y,
        long Modifier,
        BSTR Character,
        long CursorMode)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void Gesture(
        IInkCursor* Cursor,
        IInkStrokes* Strokes,
        VARIANT Gestures,
        VARIANT_BOOL* Cancel)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void TabletAdded(
        IInkTablet* Tablet)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    virtual void TabletRemoved(
        long TabletId)
    {
        // This is a place holder designed to be overridden by
        //  user of this class.
        return;
    }

    //
    // Methods
    //

    // Constructor: initialize memory to null.
    InkCollectorEvents()
    {
        m_pIConnectionPoint = NULL;
        m_punkFTM = NULL;
    }

    // Destructor: free resources
    ~InkCollectorEvents()
    {
        UnadviseInkCollector();

        if (m_punkFTM != NULL)
        {
            m_punkFTM->Release();
        }
    }

    // Init: set up free threaded marshaller.
    // It is necessary to call this method before using
    // this class to handle events.
    HRESULT Init()
    {
        return CoCreateFreeThreadedMarshaler(this, &m_punkFTM);
    }

    // Set up connection between sink and Ink Collector
    HRESULT AdviseInkCollector(
        IInkCollector *pIInkCollector)
    {
        HRESULT hr = S_OK;

        // Check to ensure that the sink is not currently connected
        // with another Ink Collector...
        if (NULL == m_pIConnectionPoint)
        {
            // Get the connection point container
            IConnectionPointContainer *pIConnectionPointContainer;
            hr = pIInkCollector->QueryInterface(
                IID_IConnectionPointContainer,
                (void **) &pIConnectionPointContainer);

            if (FAILED(hr))
            {
                return hr;
            }

            // Find the connection point for Ink Collector events
            hr = pIConnectionPointContainer->FindConnectionPoint(
                __uuidof(_IInkCollectorEvents), &m_pIConnectionPoint);

            if (SUCCEEDED(hr))
            {
                // Hook up sink to connection point
                hr = m_pIConnectionPoint->Advise(this, &m_dwCookie);
            }

            if (FAILED(hr))
            {
                // Clean up after an error.
                if (m_pIConnectionPoint)
                {
                    m_pIConnectionPoint->Release();
                    m_pIConnectionPoint = NULL;
                }
            }

            // We don't need the connection point container any more.
            pIConnectionPointContainer->Release();
        }
        // If the sink is already connected to an Ink Collector, return a
        // failure; only one Ink Collector can be attached at any given time.
        else
        {
            hr = E_FAIL;
        }

        return hr;
    }

    // Remove the connection of the sink to the Ink Collector
    HRESULT UnadviseInkCollector()
    {
        HRESULT hr = S_OK;

        // If there the ink collector is connected to the sink,
        // remove it.  Otherwise, do nothing (there is nothing
        // to unadvise).
        if (m_pIConnectionPoint != NULL)
        {
            hr = m_pIConnectionPoint->Unadvise(m_dwCookie);
            m_pIConnectionPoint->Release();
            m_pIConnectionPoint = NULL;
        }

        return hr;
    }

private:

    //
    //  Data Members
    //

    // Connection point on InkCollector
    IConnectionPoint *m_pIConnectionPoint;

    // Cookie returned from advise
    DWORD m_dwCookie;

    // Free threaded marshaler.
    IUnknown *m_punkFTM;
};

#endif // TPCCONPT_H
