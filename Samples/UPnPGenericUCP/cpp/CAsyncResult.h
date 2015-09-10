// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//
// CAsyncResult.h : class definition
//

class CUPnPAsyncResult : public IUPnPAsyncResult 
{ 
public:

CUPnPAsyncResult()
{
    m_lRefCount = 0;
    m_hReady = 0;
}

~CUPnPAsyncResult()
{
    ::CloseHandle(m_hReady);
}

public: 
//Initialization Method
STDMETHODIMP Init()
{
    HRESULT hr = S_OK;
    m_hReady = ::CreateEvent(NULL,   // default security attributes
                             FALSE,  // manual reset event object
                             FALSE,  // initial state is unsignaled
                             NULL);  // unnamed object
    if (!m_hReady)
    {
        hr = E_FAIL;
    }

    return hr;
}



// IUnknown methods 
STDMETHODIMP QueryInterface(_In_ REFIID iid, _Out_ LPVOID* ppvObject )
{
    HRESULT hr = S_OK;

    if(NULL == ppvObject)
    {
        hr = E_POINTER;
    }
    else
    {
        *ppvObject = NULL;
    }

    if(SUCCEEDED(hr))
    {
        if(IsEqualIID(iid, IID_IUnknown) || IsEqualIID(iid, IID_IUPnPAsyncResult))
        {
            *ppvObject = static_cast<IUPnPAsyncResult*>(this);
            AddRef();
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
};

STDMETHODIMP_(ULONG) AddRef()
{
    return ::InterlockedIncrement(&m_lRefCount);
};

STDMETHODIMP_(ULONG) Release()
{
    LONG lRefCount = ::InterlockedDecrement(&m_lRefCount);
    if(0 == lRefCount)
    {
        delete this;
    }
    return lRefCount;
};

// ISynchronize methods
//+---------------------------------------------------------------------------
//
//  Member:	AsyncOperationComplete
//
//  Purpose:    Called when an Async operation has been completed
//
//  Arguments:
//                  ullRequestID   [in] ID of the completed request
//
//  Returns:    None
//
//  Notes:	
//                  
//
STDMETHODIMP AsyncOperationComplete(_In_ ULONG64 ullRequestID)
{
    UNREFERENCED_PARAMETER(ullRequestID);

    HRESULT hr = S_OK;

    if(!SetEvent(m_hReady))
    {
        // If SetEvent fails, then the completion event fails to trigger
        hr = E_HANDLE;
    }
    return hr;
};

public: 

    HANDLE GetHandle(){ return m_hReady;} 

private:
    LONG        m_lRefCount;
    HANDLE      m_hReady;

};

