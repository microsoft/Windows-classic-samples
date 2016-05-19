/*++

Copyright (c) Microsoft Corporation. All rights reserved.

--*/

#ifndef __EVENTCALLBACK__
#define __EVENTCALLBACK__

namespace WiaWrap
{

//////////////////////////////////////////////////////////////////////////
//
// CEventCallback
//

/*++

    CEventCallback implements a WIA event callback object that maintains 
    the count of WIA devices on the system. Upon initialization, the object 
    gets the current number of WIA devices on the system and registers itself 
    for the connect and disconnect events, so when new devices are added or
    removed from the system, the object maintains the current device count.

Methods

    CEventCallback
        Initializes the object

    ImageEventCallback
        This method is called when a connect or disconnect event occurs.
        In either case, this method queries the current device count.

    Register
        Sets the initial device count and register for the connect and 
        disconnect events.

    GetNumDevices
        Returns the number of WIA devices on the system.

--*/

class CEventCallback : public IWiaEventCallback
{
public:
    CEventCallback();

    // IUnknown interface

    STDMETHOD(QueryInterface)(REFIID iid, LPVOID *ppvObj);
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();

    // IWiaEventCallback interface

    STDMETHOD(ImageEventCallback)(
        LPCGUID pEventGuid,
        BSTR    bstrEventDescription,
        BSTR    bstrDeviceID,
        BSTR    bstrDeviceDescription,
        DWORD   dwDeviceType,
        BSTR    bstrFullItemName,
        ULONG  *pulEventType,
        ULONG   ulReserved
    );

    // CEventCallback methods

    HRESULT Register();

    ULONG GetNumDevices() const;

private:
    LONG               m_cRef;
    ULONG              m_nNumDevices;
    CComPtr<IUnknown>  m_pConnectEventObject;
    CComPtr<IUnknown>  m_pDisconnectEventObject;
};

}; // namespace WiaWrap

#endif //__EVENTCALLBACK__
