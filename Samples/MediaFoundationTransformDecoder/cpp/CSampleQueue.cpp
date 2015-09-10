#include "CSampleQueue.h"
#include "CAutoLock.h"
#include "CHWMFT.h"
#include "CHWMFT_DebugLogger.h"

// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

#define SAFEDELETE(x) \
    if((x) != NULL) \
    { \
        delete (x); \
        (x) = NULL; \
    } \

class CSampleQueue::CNode
{
public:
    IMFSample*  pSample;
    CNode*      pNext;

    CNode(void)
    {
        pSample = NULL;
        pNext   = NULL;
    }
    ~CNode(void)
    {
        SAFERELEASE(pSample);
        pNext   = NULL;
    }
};

HRESULT CSampleQueue::Create(
    CSampleQueue**   ppQueue)
{
    HRESULT         hr          = S_OK;
    CSampleQueue*   pNewQueue   = NULL;

    do
    {
        if(ppQueue == NULL)
        {
            hr = E_POINTER;
            break;
        }

        pNewQueue = new CSampleQueue();
        if(pNewQueue == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        (*ppQueue) = pNewQueue;
        (*ppQueue)->AddRef();
    }while(false);

    SAFERELEASE(pNewQueue);

    return hr;
}

ULONG CSampleQueue::AddRef(void)
{
    return InterlockedIncrement(&m_ulRef);
}

HRESULT CSampleQueue::QueryInterface(
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

        if(riid == IID_IUnknown)
        {
            *ppvObject = (IUnknown*)this;
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

ULONG CSampleQueue::Release(void)
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

HRESULT CSampleQueue::AddSample(
    IMFSample*  pSample)
{
    HRESULT hr          = S_OK;
    CNode*  pNewNode    = NULL;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        if(pSample == NULL)
        {
            hr = E_POINTER;
            break;
        }

        pNewNode = new CNode();
        if(pNewNode == NULL)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        pNewNode->pSample           = pSample;
        pNewNode->pSample->AddRef();
        pNewNode->pNext             = NULL;

        {
            CAutoLock lock(&m_csLock);

            if(m_bAddMarker != FALSE)
            {
                hr = pSample->SetUINT64(MYMFT_MFSampleExtension_Marker, m_pulMarkerID);
                if(FAILED(hr))
                {
                    break;
                }

                m_pulMarkerID   = 0;
                m_bAddMarker    = FALSE;
            }

            if(IsQueueEmpty() != FALSE)
            {
                // This is the first sample in the queue
                m_pHead     = pNewNode;
                m_pTail     = pNewNode;

                TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Sample @%p is first sample in Queue @%p",  __FUNCTION__, pSample, this);
            }
            else
            {
                m_pTail->pNext  = pNewNode;
                m_pTail         = pNewNode;

                TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Sample @%p added to back of Queue @%p",  __FUNCTION__, pSample, this);
            }
        }
    }while(false);

    if(FAILED(hr))
    {
        SAFEDELETE(pNewNode);
    }

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit (hr=0x%x)",  __FUNCTION__, hr);

    return hr;
}
 
HRESULT CSampleQueue::GetNextSample(
    IMFSample** ppSample)
{
    HRESULT hr              = S_OK;
    CNode*  pCurrentNode    = NULL;

    do
    {
        if(ppSample == NULL)
        {
            hr = E_POINTER;
            break;
        }

        *ppSample   = NULL;

        {
            CAutoLock lock(&m_csLock);

            if(IsQueueEmpty() != FALSE)
            {
                // The queue is empty
                hr = S_FALSE;
                break;
            }
            
            pCurrentNode = m_pHead;

            (*ppSample) = pCurrentNode->pSample;
            (*ppSample)->AddRef();

            // Now remove the node from the list

            m_pHead = m_pHead->pNext;

            TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Sample @%p removed from queue Queue @%p",  __FUNCTION__, (*ppSample), this);

            if(m_pHead == NULL)
            {
                // This was the last sample in the queue
                m_pTail = NULL;

                TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Queue @%p is not empty",  __FUNCTION__, this);
            }
        }
    }while(false);

    SAFEDELETE(pCurrentNode);

    return hr;
}

HRESULT CSampleQueue::RemoveAllSamples(void)
{
    HRESULT hr          = S_OK;
    CNode*  pCurrNode   = NULL;

    do
    {
        CAutoLock lock(&m_csLock);

        while(IsQueueEmpty() == FALSE)
        {
            pCurrNode = m_pHead;

            m_pHead = m_pHead->pNext;

            delete pCurrNode;
        }

        m_pTail = NULL;
    }while(false);

    return hr;
}

HRESULT CSampleQueue::MarkerNextSample(
    const ULONG_PTR pulID)
{
    HRESULT hr = S_OK;

    do
    {
        CAutoLock lock(&m_csLock);

        m_pulMarkerID   = pulID;
        m_bAddMarker    = TRUE;
    }while(false);

    return hr;
}

BOOL CSampleQueue::IsQueueEmpty(void)
{
    CAutoLock lock(&m_csLock);

    return (m_pHead == NULL) ? TRUE : FALSE;
}

CSampleQueue::CSampleQueue(void)
{
    m_ulRef         = 1;
    m_pHead         = NULL;
    m_pTail         = NULL;
    m_bAddMarker    = FALSE;
    m_pulMarkerID   = 0;

    InitializeCriticalSection(&m_csLock);
}

CSampleQueue::~CSampleQueue(void)
{
    RemoveAllSamples();

    DeleteCriticalSection(&m_csLock);
}