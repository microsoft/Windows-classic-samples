// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:
//      EventSinks.h
//
// Description:
//      This file contains definitions of the event sink templates,
//      further inherited by the CMathInputControlEventListener class.
//
//      Event source interface used is:
//      _IMathInputControlEvents
//
//--------------------------------------------------------------------------

#pragma once

// IDispEventSimpleImpl requires a constant as a sink id
#define SINK_ID 1

/////////////////////////////////////////////////////////
//
// IMathInputControlEvents
//
// The IMathInputControlEvents class handles passing of Math Input Control
// events from Math Input Control to the user of this class.
// It sets up connection between the IMathInputControl and
// this class in AdviseMathInputControl method. In the Invoke method,
// it converts the IDispatch event notification into a
// call to a virtual function which the user of this class
// can override to process a particular event.
//
/////////////////////////////////////////////////////////

class IMathInputControlEvents: public _IMathInputControlEvents
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

        // This object only supports IDispatch/_IMathInputControlEvents
        if ((riid == IID_IUnknown)
            || (riid == IID_IDispatch)
            || (riid == DIID__IMathInputControlEvents))
        {
            *ppvObject = (IDispatch *) this;

            // Note: we do not AddRef here because the lifetime
            // of this object does not depend on reference counting
            // but on the duration of the connection set up by
            // the user of this class.
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef()
    {
        // Note: we do not AddRef here because the lifetime
        // of this object does not depend on reference counting
        // but on the duration of the connection set up by
        // the user of this class.
        return 1;
    }

    virtual ULONG STDMETHODCALLTYPE Release()
    {
        // Note: we do not do Release here because the lifetime
        // of this object does not depend on reference counting
        // but on the duration of the connection set up by
        // the user of this class.
        return 1;
    }

    //
    // IDispatch Interface
    //
    STDMETHOD(GetTypeInfoCount)(UINT* /*pctinfo*/)
    {
        // This method is not needed for processing events
        return E_NOTIMPL;
    }

    STDMETHOD(GetTypeInfo)(
        UINT /*itinfo*/,
        LCID /*lcid*/,
        ITypeInfo** /*pptinfo*/)
    {
        // This method is not needed for processing events
        return E_NOTIMPL;
    }

    STDMETHOD(GetIDsOfNames)(
        REFIID /*riid*/,
        LPOLESTR* /*rgszNames*/,
        UINT /*cNames*/,
        LCID /*lcid*/,
        DISPID* /*rgdispid*/)
    {
        // This method is not needed for processing events
        return E_NOTIMPL;
    }

    // Invoke translates from IDispatch to an event callout
    // that can be overriden by a subclass of this class
    STDMETHOD(Invoke)(
        DISPID dispidMember,
        REFIID /*riid*/,
        LCID /*lcid*/,
        WORD /*wFlags*/,
        DISPPARAMS* pdispparams,
        VARIANT* /*pvarResult*/,
        EXCEPINFO* /*pexcepinfo*/,
        UINT* /*puArgErr*/)
    {
        switch(dispidMember)
        {
            case DISPID_MICInsert:
                OnMICInsert((BSTR)pdispparams->rgvarg[0].bstrVal);
                break;

            case DISPID_MICClose:
                OnMICClose();
                break;

            case DISPID_MICClear:
                OnMICClear();
                break;

            default:
                break;
        }

        return S_OK;
    }

    //
    // Events
    // Pure virtual functions that will be overriden in the main class
    //
    virtual HRESULT OnMICInsert(BSTR RecoResult) = 0;
    virtual HRESULT OnMICClose(void) = 0;
    virtual HRESULT OnMICClear(void) = 0;

    //
    // Methods
    //

    // Constructor
    IMathInputControlEvents()
    {
        m_pIConnectionPoint = NULL;
    }

    // Destructor
    ~IMathInputControlEvents()
    {
        UnadviseMathInputControl();
    }

    // Set up connection between sink and Math Input Control
    HRESULT AdviseMathInputControl(
        IMathInputControl *pIMathInputControl)
    {
        HRESULT hr = S_OK;

        // Check to ensure that the sink is not currently connected
        // with another Math Input Control...
        if (NULL == m_pIConnectionPoint)
        {
            // Get the connection point container
            IConnectionPointContainer *pIConnectionPointContainer;
            hr = pIMathInputControl->QueryInterface(
                IID_IConnectionPointContainer,
                (void **) &pIConnectionPointContainer);

            if (FAILED(hr))
            {
                return hr;
            }

            // Find the connection point for Math Input Control events
            hr = pIConnectionPointContainer->FindConnectionPoint(
                __uuidof(_IMathInputControlEvents), &m_pIConnectionPoint);

            if (SUCCEEDED(hr))
            {
                // Hook up sink to connection point
                hr = m_pIConnectionPoint->Advise(this, &m_dwCookie);
            }

            if (FAILED(hr))
            {
                // Clean up after an error
                if (m_pIConnectionPoint)
                {
                    m_pIConnectionPoint->Release();
                    m_pIConnectionPoint = NULL;
                }
            }

            // We don't need the connection point container any more
            pIConnectionPointContainer->Release();
        }
        // If the sink is already connected to a Math Input Control, return a
        // failure. Only one Math Input Control can be attached at any given time.
        else
        {
            hr = E_FAIL;
        }

        return hr;
    }

    // Remove the connection of the sink to the Math Input Control
    HRESULT UnadviseMathInputControl()
    {
        HRESULT hr = S_OK;

        // If some Math Input Control is connected to the sink,
        // remove it. Otherwise, do nothing (there is nothing
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

    // Connection point on Math Input Control
    IConnectionPoint *m_pIConnectionPoint;

    // Cookie returned from advise
    DWORD m_dwCookie;
};
