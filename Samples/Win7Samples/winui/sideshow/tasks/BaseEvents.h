// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CBaseEvents : public ISideShowEvents
{
private:
    LONG    m_nRef;

public:
    CBaseEvents();
    virtual ~CBaseEvents();

    //
    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(
        REFIID riid,
        LPVOID* ppvObject);            

    STDMETHOD_(ULONG, AddRef)();

    STDMETHOD_(ULONG, Release)();

    //
    // ISideShowEvents methods
    //
    STDMETHOD(ContentMissing)(
        const CONTENT_ID contentId,
        ISideShowContent** ppIContent);

    STDMETHOD(ApplicationEvent)(
        ISideShowCapabilities* pICapabilities,
        const DWORD dwEventId,
        const DWORD dwEventSize,
        const BYTE* pbEventData);

    STDMETHOD(DeviceAdded)(
        ISideShowCapabilities* pIDevice);

    STDMETHOD(DeviceRemoved)(
        ISideShowCapabilities* pIDevice);

};
