// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
#pragma once

template<class T, class InterfaceT> class SampleUnknownImpl : public T
{
public:
    SampleUnknownImpl()
    {
        m_RefCount = 0;
    }
    virtual HRESULT STDMETHODCALLTYPE QueryInterface (
        REFIID riid,
        void __RPC_FAR *__RPC_FAR *ppvObject )
    {
        if ( riid == __uuidof ( IUnknown ) )
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }
        else if ( riid == __uuidof ( InterfaceT ) )
        {
            *ppvObject = this;
            AddRef();
            return S_OK;
        }

        return E_NOINTERFACE;
    }

    virtual ULONG STDMETHODCALLTYPE AddRef ( void )
    {
        return InterlockedIncrement ( &m_RefCount );
    }

    virtual ULONG STDMETHODCALLTYPE Release ( void )
    {
        LONG newCount = InterlockedDecrement ( &m_RefCount );
        if ( 0 == newCount )
        {
            delete this;
        }
        return newCount;
    }
private:
    volatile LONG m_RefCount;
};


