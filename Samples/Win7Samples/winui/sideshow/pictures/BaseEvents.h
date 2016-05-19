// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CCallBack
{
public:
    virtual HRESULT Call(
        const CONTENT_ID /*contentId*/,
        ISideShowContent** /*ppIContent*/
        ){return S_FALSE;}
        
    virtual HRESULT Call(
        ISideShowCapabilities* /*pICapabilities*/,
        const DWORD /*dwEventId*/,
        const DWORD /*dwEventSize*/,
        const BYTE* /*pbEventData*/
        ){return S_FALSE;}
    
    virtual HRESULT Call(
        ISideShowCapabilities* /*pIDevice*/
        ){return S_FALSE;}
};


template <class T>
class CContentMissingEvent : public CCallBack
{
private:
    HRESULT (T::*m_fpt)(const CONTENT_ID contentId, ISideShowContent** ppIContent);
    T* m_pInstance;
    
public:
    CContentMissingEvent(
        T* pInstance,
        HRESULT(T::*fpt)(const CONTENT_ID contentId, ISideShowContent** ppIContent)
        )
    {
        m_pInstance = pInstance;
        m_fpt = fpt;
    }
    
    virtual HRESULT Call(
        const CONTENT_ID contentId,
        ISideShowContent** ppIContent
        )
    {
        return (*m_pInstance.*m_fpt)(contentId, ppIContent);
    }
};

template <class T>
class CApplicationEvent : public CCallBack
{
private:
    HRESULT (T::*m_fpt)(ISideShowCapabilities* pICapabilities,
                           const DWORD dwEventId,
                           const DWORD dwEventSize,
                           const BYTE* pbEventData);
    
    T* m_pInstance;
    
public:
    CApplicationEvent(
        T* pInstance,
        HRESULT(T::*fpt)(ISideShowCapabilities* pICapabilities,
                               const DWORD dwEventId,
                               const DWORD dwEventSize,
                               const BYTE* pbEventData)
        )
    {
        m_pInstance = pInstance;
        m_fpt = fpt;
    }
    
    virtual HRESULT Call(
        ISideShowCapabilities* pICapabilities,
        const DWORD dwEventId,
        const DWORD dwEventSize,
        const BYTE* pbEventData
        )
    {
        return (*m_pInstance.*m_fpt)(pICapabilities,
                                 dwEventId,
                                 dwEventSize,
                                 pbEventData);
    }
};

template <class T>
class CDeviceEvent : public CCallBack
{
private:
    HRESULT (T::*m_fpt)(ISideShowCapabilities* pIDevice);
    T* m_pInstance;
    
public:
    CDeviceEvent(
        T* pInstance,
        HRESULT(T::*fpt)(ISideShowCapabilities* pIDevice)
        )
    {
        m_pInstance = pInstance;
        m_fpt = fpt;
    }
    
    virtual HRESULT Call(
        ISideShowCapabilities* pIDevice
        )
    {
        return (*m_pInstance.*m_fpt)(pIDevice);
    }
};


class CBaseEvents : public ISideShowEvents
{
private:
    LONG       m_nRef;
    CCallBack* m_pDeviceAddEvent;
    CCallBack* m_pDeviceRemoveEvent;
    CCallBack* m_pContentMissingEvent;
    CCallBack* m_pApplicationEventEvent;
    
protected:
    //
    // ISideShowEvents methods
    //
    STDMETHOD(ContentMissing)(
        const CONTENT_ID contentId,
        ISideShowContent** ppIContent
        );

    STDMETHOD(ApplicationEvent)(
        ISideShowCapabilities* pICapabilities,
        const DWORD dwEventId,
        const DWORD dwEventSize,
        const BYTE* pbEventData
        );

    STDMETHOD(DeviceAdded)(
        ISideShowCapabilities* pIDevice
        );

    STDMETHOD(DeviceRemoved)(
        ISideShowCapabilities* pIDevice
        );
        
public:
    //
    // IUnknown methods
    //
    STDMETHOD_(ULONG, AddRef)();
    STDMETHOD_(ULONG, Release)();
    
    STDMETHOD(QueryInterface)(
        REFIID riid,
        LPVOID* ppvObject
        );
    
    CBaseEvents();
    virtual ~CBaseEvents();
        
public:
    //
    // Event Registration methods
    //
    void RegisterDeviceAddEvent(CCallBack* pCallBack);
    void RegisterDeviceRemoveEvent(CCallBack* pCallBack);
    void RegisterContentMissingEvent(CCallBack* pCallBack);
    void RegisterApplicationEvent(CCallBack* pCallBack);
};
