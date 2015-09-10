#include "CHWMFT_DecodeTask.h"
#include <Mfapi.h>

#include <initguid.h>
#include "IMYMFT.h"

// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

HRESULT CDecodeTask::Create(
    const DWORD         dwDecodeWorkQueueID,
    IMFSample*          pInputSample,
    IMFAsyncCallback**  ppTask)
{
    HRESULT         hr              = S_OK;
    CDecodeTask*    pNewDecodeTask  = NULL;

    do
    {
        if(pInputSample == NULL)
        {
            hr = E_POINTER;
            break;
        }

        pNewDecodeTask  = new CDecodeTask();
        if(pNewDecodeTask == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pNewDecodeTask->m_dwDecodeWorkQueueID       = dwDecodeWorkQueueID;
        pNewDecodeTask->m_pInputSample              = pInputSample;
        pNewDecodeTask->m_pInputSample->AddRef();

        hr = pNewDecodeTask->QueryInterface(IID_IMFAsyncCallback, (void**)ppTask);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFERELEASE(pNewDecodeTask);

    return hr;
}

HRESULT CDecodeTask::Begin(
    IMYMFT* pMYMFT)
{
    HRESULT             hr              = S_OK;
    IMFAsyncCallback*   pDecodeTask     = NULL;
    IMFAsyncResult*     pResult         = NULL;

    do
    {
        if(pMYMFT == NULL)
        {
            hr = E_POINTER;
            break;
        }

        hr = MFCreateAsyncResult(NULL, this, pMYMFT, &pResult);
        if(FAILED(hr))
        {
            break;
        }

        hr = MFPutWorkItemEx(m_dwDecodeWorkQueueID, pResult);
        if(FAILED(hr))
        {
            break;
        }

    }while(false);

    SAFERELEASE(pDecodeTask);
    SAFERELEASE(pResult);

    return hr;
}

HRESULT CDecodeTask::End(void)
{
    // Nothing to do
    return S_OK;
}

HRESULT CDecodeTask::GetParameters(
    DWORD*  pdwFlags,
    DWORD*  pdwQueue)
{
    HRESULT hr = S_OK;

    do
    {
        if((pdwFlags == NULL) || (pdwQueue == NULL))
        {
            hr = E_POINTER;
            break;
        }

        (*pdwFlags) = 0;
        (*pdwQueue) = m_dwDecodeWorkQueueID;
    }while(false);

    return hr;
}

HRESULT CDecodeTask::Invoke(
    IMFAsyncResult* pAsyncResult)
{
    HRESULT     hr          = S_OK;
    IUnknown*   pStateUnk   = NULL;
    IMYMFT*     pMYMFT      = NULL;

    do
    {
        if(pAsyncResult == NULL)
        {
            hr = E_POINTER;
            break;
        }

        hr = pAsyncResult->GetState(&pStateUnk);
        if(FAILED(hr))
        {
            break;
        }

        hr = pStateUnk->QueryInterface(IID_IMYMFT, (void**)&pMYMFT);
        if(FAILED(hr))
        {
            break;
        }

        hr = pMYMFT->DecodeInputFrame(m_pInputSample);
        // Save the status

        hr = pAsyncResult->SetStatus(hr);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFERELEASE(pStateUnk);
    SAFERELEASE(pMYMFT);

    End();

    return hr;
}

ULONG CDecodeTask::AddRef(void)
{
    return InterlockedIncrement(&m_ulRef);
}

HRESULT CDecodeTask::QueryInterface(
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

        if(riid == IID_IMFAsyncCallback)
        {
            *ppvObject = (IMFAsyncCallback*)this;
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

ULONG CDecodeTask::Release(void)
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

CDecodeTask::CDecodeTask(void)
{
    m_ulRef                 = 1;
    m_pInputSample          = NULL;
    m_dwDecodeWorkQueueID   = 0;
}

CDecodeTask::~CDecodeTask(void)
{
    SAFERELEASE(m_pInputSample);
}