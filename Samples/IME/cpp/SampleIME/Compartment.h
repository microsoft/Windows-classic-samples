// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class CCompartment
{
public:
    CCompartment(_In_ IUnknown *punk, TfClientId tfClientId, _In_ REFGUID guidCompartment);
    ~CCompartment();

    HRESULT _GetCompartment(_Outptr_ ITfCompartment **ppCompartment);
    HRESULT _GetCompartmentBOOL(_Out_ BOOL &flag);
    HRESULT _SetCompartmentBOOL(_In_ BOOL flag);
    HRESULT _GetCompartmentDWORD(_Out_ DWORD &dw);
    HRESULT _SetCompartmentDWORD(_In_ DWORD dw);
    HRESULT _ClearCompartment();

    VOID _GetGUID(GUID *pguid)
    {
        *pguid = _guidCompartment;
    }

private:
    GUID _guidCompartment;
    IUnknown* _punk;
    TfClientId _tfClientId;
};

typedef HRESULT (*CESCALLBACK)(void *pv, REFGUID guidCompartment);

class CCompartmentEventSink : public ITfCompartmentEventSink
{
public:
    CCompartmentEventSink(_In_ CESCALLBACK pfnCallback, _In_ void *pv);
    ~CCompartmentEventSink();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, _Outptr_ void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfCompartmentEventSink
    STDMETHODIMP OnChange(_In_ REFGUID guid);

    // function
    HRESULT _Advise(_In_ IUnknown *punk, _In_ REFGUID guidCompartment);
    HRESULT _Unadvise();

private:
    ITfCompartment *_pCompartment;
    DWORD _dwCookie;
    CESCALLBACK _pfnCallback;
    void *_pv;

    LONG _refCount;
};
