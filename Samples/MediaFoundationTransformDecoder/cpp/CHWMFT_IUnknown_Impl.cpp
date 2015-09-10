#include "CHWMFT.h"
#include "IMYMFT.h"

ULONG CHWMFT::AddRef(void)
{
    return InterlockedIncrement(&m_ulRef);
}

HRESULT CHWMFT::QueryInterface(
    REFIID riid,
    void** ppvObject)
{
    HRESULT hr = S_OK;

    do
    {
        if(ppvObject == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /****************************************************
        ** Todo: add all supported interfaces by your MFT
        ****************************************************/
        if(riid == IID_IMFTransform)
        {
            *ppvObject = (IMFTransform*)this;
        }
        else if(riid == IID_IMFAttributes)
        {
            *ppvObject = (IMFAttributes*)this;
        }
        else if(riid == IID_IMFShutdown)
        {
            *ppvObject = (IMFShutdown*)this;
        }
        else if(riid == IID_IMFMediaEventGenerator)
        {
            *ppvObject = (IMFMediaEventGenerator*)this;
        }
        else if(riid == IID_IMFAsyncCallback)
        {
            *ppvObject = (IMFAsyncCallback*)this;
        }
        else if(riid == IID_IMYMFT)
        {
            *ppvObject = (IMYMFT*)this;
        }
        else if(riid == IID_IUnknown)
        {
            *ppvObject = this;
        }
        else
        {
            *ppvObject = NULL;
            hr = E_NOINTERFACE;
            break;
        }

        AddRef();
    }while(false);

    return hr;
}

ULONG CHWMFT::Release(void)
{
    ULONG   ulRef = 0;
    
    if(m_ulRef > 0)
    {
        ulRef = InterlockedDecrement(&m_ulRef);
    }

    if(ulRef == 0)
    {
        delete this;
    }

    return ulRef;
}