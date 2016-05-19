// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

class CBaseContent : public ISideShowContent
{
private:
    LONG    m_nRef;

protected:
    //
    // These methods should be overridden in the subclass to
    // provide (and later free) the content that should be
    // sent to the platform.
    //
    virtual void LoadContent(DWORD* pdwSize, BYTE** ppbData) = 0;
    virtual void FreeContent(BYTE** ppbData) = 0;

    //
    // Use this method to convert from Unicode to UTF-8 strings
    //
    LPSTR AllocTaskUtf8String(LPCWSTR lpszString);

    CONTENT_ID m_contentID;

public:
    CBaseContent();
    virtual ~CBaseContent();

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
