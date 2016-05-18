// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CHelloWorldContent : public ISideShowContent
{
private:
    LONG    m_nRef;

public:
    CHelloWorldContent();
    virtual ~CHelloWorldContent();

    //
    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(
            REFIID riid,
            LPVOID* ppvObject);            

    STDMETHOD_(ULONG, AddRef)();

    STDMETHOD_(ULONG, Release)();

    //
    // ISideShowContent methods
    //
    STDMETHOD(GetContent)(
            ISideShowCapabilities *pICapabilities,
            DWORD *pdwSize,
            BYTE **ppbData);
        
    STDMETHOD(get_ContentId)(
            PCONTENT_ID pcontentId);
        
    STDMETHOD(get_DifferentiateContent)(
            BOOL *pfDifferentiateContent);
};
