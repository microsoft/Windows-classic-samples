/*++

Copyright (c) 1999 - 2000 Microsoft Corporation

Module Name:

    sampstrm.h

Abstract:

    Declaration of the sample MSP stream class.

--*/

#ifndef __SAMPSTRM_H_
#define __SAMPSTRM_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000


/////////////////////////////////////////////////////////////////////////////
// CSampleMSPStream
/////////////////////////////////////////////////////////////////////////////
class CSampleMSPStream : public CMSPStream
    //  if you want to allow your object to be used in scripting apps
    //  it has to expose IObjectSafety interface. if your object is safe for 
    //  scripting on all of its interfaces, you can use the implementation
    //  of IObjectSafety from MSPUtils.h
    //, public CMSPObjectSafetyImpl

{
public:
// DECLARE_POLY_AGGREGATABLE(CSampleMSP)

// To add extra interfaces to this class, use the following:
// BEGIN_COM_MAP(CSampleMSPStream)
//     COM_INTERFACE_ENTRY( YOUR_INTERFACE_HERE )
//     // if you want to allow your object to be used in scripting apps
//     // it has to expose IObjectSafety interface
//     // COM_INTERFACE_ENTRY( IObjectSafety )
//     COM_INTERFACE_ENTRY_CHAIN(CMSPStream)
// END_COM_MAP()

public:

    //
    // Construction and destruction.
    //

    CSampleMSPStream();
    virtual ~CSampleMSPStream();
    virtual void FinalRelease();

    //
    // Required base class overrides.
    // 

    STDMETHOD (get_Name) (
        OUT     BSTR *                  ppName
        );

    //
    // We override these methods to implement our terminal handling.
    // This consists of only allowing one terminal on the stream at a time
    // and adding our filters and the terminal to the graph at the right
    // times.
    //

    STDMETHOD (SelectTerminal) (
        IN      ITTerminal *            pTerminal
        );

    STDMETHOD (UnselectTerminal) (
        IN     ITTerminal *             pTerminal
        );

    STDMETHOD (StartStream) ();

    STDMETHOD (PauseStream) ();

    STDMETHOD (StopStream) ();

    //
    // Overrides for event handling.
    //

    virtual HRESULT ProcessGraphEvent(
        IN  long lEventCode,
        IN  long lParam1,
        IN  long lParam2
        );

    //
    // Public methods specific to our implementation.
    //

    virtual HRESULT FireEvent(IN MSP_CALL_EVENT        type,
                              IN HRESULT               hrError,
                              IN MSP_CALL_EVENT_CAUSE  cause);

    // A real MSP would have some arguments to this method,
    // which would then be used to configure transport filters. For example,
    // an MSP whose transports are wavein and waveout filters might have
    // WaveInIDs and WaveOutIDs passed as arguments to this method.
    virtual HRESULT ConfigureTransport(void);

protected:
    //
    // Protected data members.
    // A real MSP stream will have more... such as pointers to filters.
    //

    BOOL          m_fTransportConfigured;
    BOOL          m_fTerminalConnected;
    FILTER_STATE  m_DesiredGraphState;
    FILTER_STATE  m_ActualGraphState;

private:
    //
    // Private helper methods.
    //

    HRESULT ConnectTerminal(ITTerminal * pTerminal);
    HRESULT ConnectToTerminalPin(IPin * pTerminalPin);
};

#endif //__SAMPSTRM_H_
