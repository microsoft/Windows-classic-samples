#include "CHWMFT.h"
#include "CHWMFT_DecodeTask.h"
#include "CAutoLock.h"
#include <mfapi.h>
#include <mferror.h>
#include "CHWMFT_DebugLogger.h"
#include <initguid.h>

// {1F620607-A7FF-4B94-82F4-993F2E17B497}
DEFINE_GUID(MYMFT_MFSampleExtension_Marker, 
0x1f620607, 0xa7ff, 0x4b94, 0x82, 0xf4, 0x99, 0x3f, 0x2e, 0x17, 0xb4, 0x97);

// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

#define MFT_NUM_DEFAULT_ATTRIBUTES  4
#define MFT_HW_URL                  L"MSFT Win8 SDK HW MFT Sample"

// Global Variables
const GUID*     g_ppguidInputTypes[] = 
    {
        &MFVideoFormat_H264,
        &MFVideoFormat_MPEG2,
        &MFVideoFormat_WVC1,
        &MFVideoFormat_MP4V,
    };
const DWORD     g_dwNumInputTypes   = sizeof(g_ppguidInputTypes) / sizeof(g_ppguidInputTypes[0]);

const GUID*     g_ppguidOutputTypes[] =
    {
        &MFVideoFormat_RGB32,
        &MFVideoFormat_NV12,
    };
const DWORD     g_dwNumOutputTypes   = sizeof(g_ppguidOutputTypes) / sizeof(g_ppguidOutputTypes[0]);

// Initializer
HRESULT CHWMFT::CreateInstance(IMFTransform** ppHWMFT)
{
    HRESULT hr          = S_OK;
    CHWMFT* pMyHWMFT    = NULL;

    do
    {
        if(ppHWMFT == NULL)
        {
            hr = E_POINTER;
            break;
        }

        pMyHWMFT = new CHWMFT();
        if(FAILED(hr))
        {
            break;
        }

        hr = pMyHWMFT->InitializeTransform();
        if(FAILED(hr))
        {
            break;
        }

        hr = pMyHWMFT->QueryInterface(IID_IMFTransform, (void**)ppHWMFT);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFERELEASE(pMyHWMFT);

    return hr;
}

/****************************
********** ***********
****************************/

CHWMFT::CHWMFT(void)
{
    /****************************************************
    ** Todo: Initialize All Member variables used by your
    ** MFT
    ****************************************************/

    // Do no insert anything before this call, this is the DLLs object count
    InterlockedIncrement(&m_ulNumObjects);

    if(InterlockedCompareExchange(&m_ulNumObjects, 1, 1) == 1)
    {
        // we're the first object, turn on tracing
        TraceInitialize();
    }

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);
    
    m_ulRef                 = 1;
    m_ulSampleCounter       = 0;
    m_pInputMT              = NULL;
    m_pOutputMT             = NULL;
    m_pAttributes           = NULL;
    m_pEventQueue           = NULL;
    m_dwStatus              = 0;
    m_dwNeedInputCount      = 0;
    m_dwHaveOutputCount     = 0;
    m_dwDecodeWorkQueueID   = 0;
    m_llCurrentSampleTime   = 0;
    m_bShutdown             = FALSE;
    m_bFirstSample          = TRUE;
    m_bDXVA                 = FALSE;
    m_pInputSampleQueue     = NULL;
    m_pOutputSampleQueue    = NULL;
    InitializeCriticalSection(&m_csLock);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit", __FUNCTION__);
}

CHWMFT::~CHWMFT(void)
{
    /****************************************************
    ** Todo: Release All Member variables used by your
    ** MFT
    ****************************************************/

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    SAFERELEASE(m_pInputMT);
    SAFERELEASE(m_pOutputMT);
    SAFERELEASE(m_pAttributes);
    if(m_pEventQueue != NULL)
    {
        m_pEventQueue->Shutdown();
        SAFERELEASE(m_pEventQueue);
    }
    SAFERELEASE(m_pInputSampleQueue);
    SAFERELEASE(m_pOutputSampleQueue);
    DeleteCriticalSection(&m_csLock);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit", __FUNCTION__);

    if(InterlockedCompareExchange(&m_ulNumObjects, 1, 1) == 1)
    {
        // We're the last instance, turn off tracing
        TraceUninitialize();
    }

    InterlockedDecrement(&m_ulNumObjects);
    // Do no insert anything after this call, this is the DLLs object count
}

HRESULT CHWMFT::InitializeTransform(void)
{
    /*************************************
    ** Todo: Use this function to setup
    ** anything that can't be setup in
    ** the constructor
    *************************************/

    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        hr = MFCreateAttributes(&m_pAttributes, MFT_NUM_DEFAULT_ATTRIBUTES);
        if(FAILED(hr))
        {
            break;
        }

        /*********************************
        ** Certain Attributes are required
        ** for HW MFTs
        ** See http://msdn.microsoft.com/en-us/library/dd940330(VS.85).aspx#attributes
        *********************************/
        hr = m_pAttributes->SetUINT32(MF_TRANSFORM_ASYNC, TRUE);
        if(FAILED(hr))
        {
            break;
        }

        /****************************************
        ** !!MSFT_TODO: Report as HW MFT
        ****************************************
        hr = m_pAttributes->SetString(MFT_ENUM_HARDWARE_URL_Attribute, MFT_HW_URL);
        if(FAILED(hr))
        {
            break;
        }

        hr = m_pAttributes->SetString(MFT_ENUM_HARDWARE_VENDOR_ID_Attribut, MFT_HW_VENDOR_ID);
        if(FAILED(hr))
        {
            break;
        }
        */

        hr = m_pAttributes->SetUINT32(MFT_SUPPORT_DYNAMIC_FORMAT_CHANGE, TRUE);
        if(FAILED(hr))
        {
            break;
        }

        /**********************************
        ** Since this is an Async MFT, an
        ** event queue is required
        ** MF Provides a standard implementation
        **********************************/
        hr = MFCreateEventQueue(&m_pEventQueue);
        if(FAILED(hr))
        {
            TraceString(CHMFTTracing::TRACE_ERROR, L"%S(): Failed to create MF Event Queue (hr=0x%x)", __FUNCTION__, hr);
            break;
        }

        hr = CSampleQueue::Create(&m_pInputSampleQueue);
        if(FAILED(hr))
        {
            TraceString(CHMFTTracing::TRACE_ERROR, L"%S(): Failed to create Input Sample Queue (hr=0x%x)", __FUNCTION__, hr);
            break;
        }

        hr = CSampleQueue::Create(&m_pOutputSampleQueue);
        if(FAILED(hr))
        {
            TraceString(CHMFTTracing::TRACE_ERROR, L"%S(): Failed to create Output Sample Queue (hr=0x%x)", __FUNCTION__, hr);
            break;
        }

        /**********************************
        ** Since this is an Async MFT, all
        ** work will be done using standard
        ** MF Work Queues
        **********************************/
        hr = MFAllocateWorkQueueEx(MF_STANDARD_WORKQUEUE, &m_dwDecodeWorkQueueID);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit (hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::CheckInputType(
    IMFMediaType*   pMT)
{
    /*************************************
    ** Todo: Your MFT should verify the
    ** Input media type is acceptable
    ** Modify this function as necessary
    *************************************/

    HRESULT hr      = S_OK;
    GUID    guid    = GUID_NULL;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        /******************************
        ** No need to verify pMT != NULL
        ** as this is a private function
        ******************************/

        hr = pMT->GetGUID(MF_MT_MAJOR_TYPE, &guid);
        if(FAILED(hr))
        {
            break;
        }

        if(guid != MFMediaType_Video)
        {
            hr = MF_E_INVALIDMEDIATYPE;
            break;
        }

        hr = pMT->GetGUID(MF_MT_SUBTYPE, &guid);
        if(FAILED(hr))
        {
            break;
        }

        hr = MF_E_INVALIDMEDIATYPE; // Gets set to S_OK if MT is acceptable

        for(DWORD i = 0; i < g_dwNumInputTypes; i++)
        {
            if(guid == *(g_ppguidInputTypes[i]))
            {
                hr = S_OK;
                break;
            }
        }

        if(FAILED(hr))
        {
            break;
        }

        // The Mediatype is acceptable
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::CheckOutputType(
    IMFMediaType*   pMT)
{
    /*************************************
    ** Todo: Your MFT should verify the
    ** Output media type is acceptable
    ** Modify this function as necessary
    *************************************/

    HRESULT hr              = S_OK;
    GUID    guid            = GUID_NULL;
    UINT32  un32HighWord    = 0;
    UINT32  un32LowWord     = 0;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        /******************************
        ** No need to verify pMT != NULL
        ** as this is a private function
        ******************************/

        hr = pMT->GetGUID(MF_MT_MAJOR_TYPE, &guid);
        if(FAILED(hr))
        {
            break;
        }

        if(guid != MFMediaType_Video)
        {
            hr = MF_E_INVALIDMEDIATYPE;
            break;
        }

        hr = pMT->GetGUID(MF_MT_SUBTYPE, &guid);
        if(FAILED(hr))
        {
            break;
        }

        hr = MF_E_INVALIDMEDIATYPE; // Gets set to S_OK if MT is acceptable

        for(DWORD i = 0; i < g_dwNumOutputTypes; i++)
        {
            if(guid == *(g_ppguidOutputTypes[i]))
            {
                hr = S_OK;
                break;
            }
        }

        if(FAILED(hr))
        {
            break;
        }

        hr = MFGetAttributeSize(pMT, MF_MT_FRAME_SIZE, &un32HighWord, &un32LowWord);
        if(FAILED(hr))
        {
            break;
        }

        if((un32HighWord != MFT_OUTPUT_WIDTH) || (un32LowWord != MFT_OUTPUT_HEIGHT))
        {
            hr = MF_E_INVALIDMEDIATYPE;
            break;
        }

        hr = MFGetAttributeRatio(pMT, MF_MT_FRAME_RATE, &un32HighWord, &un32LowWord);
        if(FAILED(hr))
        {
            break;
        }

        if((un32HighWord != MFT_FRAMERATE_NUMERATOR) || (un32LowWord != MFT_FRAMERATE_DENOMINATOR))
        {
            hr = MF_E_INVALIDMEDIATYPE;
            break;
        }

        // The Mediatype is acceptable
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::ShutdownEventQueue(void)
{
    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        /***************************************
        ** Since this in an internal function
        ** we know m_pEventQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/

        hr = m_pEventQueue->Shutdown();
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::OnStartOfStream(void)
{
    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        {
            CAutoLock lock(&m_csLock);
                
            m_dwStatus |= MYMFT_STATUS_STREAM_STARTED;
        }

        /*******************************
        ** Todo: This MFT only has one
        ** input stream, so RequestSample
        ** is always called with '0'. If
        ** your MFT has more than one
        ** input stream, you will
        ** have to change this logic
        *******************************/
        hr = RequestSample(0);  
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::OnEndOfStream(void)
{
    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        CAutoLock lock(&m_csLock);
                
        m_dwStatus &= (~MYMFT_STATUS_STREAM_STARTED);

        /*****************************************
        ** See http://msdn.microsoft.com/en-us/library/dd317909(VS.85).aspx#processinput
        ** Upon receiving EOS, the outstanding process
        ** input request should be reset to 0
        *****************************************/
        m_dwNeedInputCount = 0;
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::OnDrain(
    const UINT32 un32StreamID)
{
    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        CAutoLock lock(&m_csLock);
                
        m_dwStatus |= (MYMFT_STATUS_DRAINING);

        /*******************************
        ** Todo: This MFT only has one
        ** input stream, so it does not
        ** track the stream being drained. 
        ** If your MFT has more than one
        ** input stream, you will
        ** have to change this logic
        *******************************/
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::OnFlush(void)
{
    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);
    
    do
    {
        CAutoLock lock(&m_csLock);
                
        m_dwStatus &= (~MYMFT_STATUS_STREAM_STARTED);

        hr = FlushSamples();
        if(FAILED(hr))
        {
            break;
        }

        m_llCurrentSampleTime   = 0;    // Reset our sample time to 0 on a flush
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::OnMarker(
    const ULONG_PTR pulID)
{
    HRESULT hr  = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter", __FUNCTION__);

    do
    {
        // No need to lock, our sample queue is thread safe

        /***************************************
        ** Since this in an internal function
        ** we know m_pInputSampleQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/

        hr = m_pInputSampleQueue->MarkerNextSample(pulID);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::RequestSample(
    const UINT32 un32StreamID)
{
    HRESULT         hr      = S_OK;
    IMFMediaEvent*  pEvent  = NULL;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        {
            CAutoLock lock(&m_csLock);

            if((m_dwStatus & MYMFT_STATUS_STREAM_STARTED) == 0)
            {
                // Stream hasn't started
                TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Stream Hasn't Started",  __FUNCTION__);
                hr = MF_E_NOTACCEPTING;
                break;
            }
        }

        hr = MFCreateMediaEvent(METransformNeedInput, GUID_NULL, S_OK, NULL, &pEvent);
        if(FAILED(hr))
        {
            break;
        }

        hr = pEvent->SetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, un32StreamID);
        if(FAILED(hr))
        {
            break;
        }

        /***************************************
        ** Since this in an internal function
        ** we know m_pEventQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/
        
        {
            CAutoLock lock(&m_csLock);

            hr = m_pEventQueue->QueueEvent(pEvent);
            if(FAILED(hr))
            {
                break;
            }

            m_dwNeedInputCount++;

            TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): NeedInputCount: %u",  __FUNCTION__, m_dwNeedInputCount);
        }
    }while(false);

    SAFERELEASE(pEvent);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}


HRESULT CHWMFT::FlushSamples(void)
{
    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        CAutoLock lock(&m_csLock);

        hr = OnEndOfStream();       // Treat this like an end of stream, don't accept new samples until
        if(FAILED(hr))              // a new stream is started
        {
            break;
        }        

        m_dwHaveOutputCount = 0;    // Don't Output samples until new input samples are given

        hr = m_pInputSampleQueue->RemoveAllSamples();
        if(FAILED(hr))
        {
            break;
        }

        hr = m_pOutputSampleQueue->RemoveAllSamples();
        if(FAILED(hr))
        {
            break;
        }

        m_bFirstSample = TRUE; // Be sure to reset our first sample so we know to set discontinuity
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::ScheduleFrameDecode(void)
{
    HRESULT             hr              = S_OK;
    IMFSample*          pInputSample    = NULL;
    CDecodeTask*        pDecodeTask     = NULL;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        // No need to lock, sample queues are thread safe

        /***************************************
        ** Since this in an internal function
        ** we know m_pInputSampleQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/
        hr = m_pInputSampleQueue->GetNextSample(&pInputSample);
        if(FAILED(hr))
        {
            break;
        }

        hr = CDecodeTask::Create(
            m_dwDecodeWorkQueueID,
            pInputSample,
            (IMFAsyncCallback**)&pDecodeTask);
        if(FAILED(hr))
        {
            break;
        }

        hr = pDecodeTask->Begin(this);
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    SAFERELEASE(pInputSample);
    SAFERELEASE(pDecodeTask);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::DecodeInputFrame(
    IMFSample*  pInputSample)
{
    HRESULT         hr                  = S_OK;
    IMFSample*      pOutputSample       = NULL;
    IMFMediaBuffer* pMediaBuffer        = NULL;
    IMFMediaEvent*  pHaveOutputEvent    = NULL;
    DWORD           dwCurrentSample     = 0;
    DWORD           dwDataLen           = 0;
    BYTE*           pbData              = NULL;
    UINT64          pun64MarkerID       = 0;
    LONGLONG        llSampleDuration    = MFT_DEFAULT_SAMPLE_DURATION;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        /****************************************
        ** Todo, here's where the MFT transforms
        ** the input to output samples. In the
        ** SDK sample, the output is simply a 
        ** numbered frame, so no complicated
        ** processing is done here. Your MFT
        ** should reference it's input and output
        ** types to ensure it does the right thing
        ** here and modify this function accordingly
        ** Addionally, your MFT must monitor for
        ** format changes and act accordly
        ** See http://msdn.microsoft.com/en-us/library/ee663587(VS.85).aspx
        ****************************************/

        dwCurrentSample = InterlockedIncrement(&m_ulSampleCounter);

        hr = MFCreateSample(&pOutputSample);
        if(FAILED(hr))
        {
            break;
        }

        if(m_bDXVA != FALSE) // Not thread safe!
        {
            /****************************************
            ** !!MSFT_TODO: handle dxva!
            ****************************************/

            //MessageBox(NULL, L"TODO: Make MFT Handle DXVA!", L"ERROR!", MB_OK);
            hr = E_NOTIMPL;
            break;
        }
        else
        {
            dwDataLen = (MFT_OUTPUT_WIDTH * MFT_OUTPUT_HEIGHT) * 4; // This is the max needed for the
                                                                    // biggest supported output type
            hr = MFCreateMemoryBuffer(dwDataLen, &pMediaBuffer); 
            if(FAILED(hr))                                                                        
            {
                break;
            }

            hr = pMediaBuffer->Lock(&pbData, NULL, NULL);
            if(FAILED(hr))
            {
                break;
            }

            hr = pMediaBuffer->SetCurrentLength(dwDataLen);
            if(FAILED(hr))
            {
                break;
            }

            hr = pOutputSample->AddBuffer(pMediaBuffer);
            if(FAILED(hr))
            {
                break;
            }
        }

        unsigned char pucColors[10] = {
            0,
            25,
            50,
            75,
            100,
            125,
            150,
            175,
            200,
            225
        };

        memset(pbData, pucColors[dwCurrentSample % 10], dwDataLen);     // Fill our buffer with a color correlated to the frame number
                                                                        // This will show our frames changing

        // Now setup the sample
        hr = DuplicateAttributes(pOutputSample, pInputSample);
        if(FAILED(hr))
        {
            break;
        }

        hr = pOutputSample->SetSampleDuration(llSampleDuration);
        if(FAILED(hr))
        {
            break;
        }

        {
            CAutoLock   lock(&m_csLock);
            LONGLONG    llSampleTime    = 0;

            if(FAILED(pInputSample->GetSampleTime(&llSampleTime)))
            {
                llSampleTime            = m_llCurrentSampleTime;
                m_llCurrentSampleTime   += llSampleDuration;
            }

            hr = pOutputSample->SetSampleTime(llSampleTime);
            if(FAILED(hr))
            {
                break;
            }

            if(m_bFirstSample != FALSE)
            {
                TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Sample @%p is discontinuity",  __FUNCTION__, pOutputSample);

                hr = pOutputSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
                if(FAILED(hr))
                {
                    break;
                }

                m_bFirstSample = FALSE;
            }

            TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Output Sample @%p Created: Duration %I64u, Sample Time %I64d", 
                __FUNCTION__, pOutputSample, llSampleDuration, m_llCurrentSampleTime);
        }

        /***************************************
        ** Since this in an internal function
        ** we know m_pOutputSampleQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/
        hr = m_pOutputSampleQueue->AddSample(pOutputSample);
        if(FAILED(hr))
        {
            TraceString(CHMFTTracing::TRACE_ERROR, L"%S(): Failed to add sample to output queue (hr=0x%x)",  __FUNCTION__, hr);
            break;
        }

        // Now that the sample is in the output queue, send out have output event

        hr = MFCreateMediaEvent(METransformHaveOutput, GUID_NULL, S_OK, NULL, &pHaveOutputEvent);
        if(FAILED(hr))
        {
            TraceString(CHMFTTracing::TRACE_ERROR, L"%S(): Failed to create METransformHaveOutput event (hr=0x%x)",  __FUNCTION__, hr);
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            /***************************************
            ** Since this in an internal function
            ** we know m_pEventQueue can never be
            ** NULL due to InitializeTransform()
            ***************************************/

            hr = m_pEventQueue->QueueEvent(pHaveOutputEvent);
            if(FAILED(hr))
            {
                // If this fails, consider decrementing m_dwHaveOutputCount
                TraceString(CHMFTTracing::TRACE_ERROR, L"%S(): Failed to queue METransformHaveOutput event (hr=0x%x)",  __FUNCTION__, hr);
                break;
            }

            m_dwHaveOutputCount++;

            TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): HaveOutputCount: %u",  __FUNCTION__, m_dwHaveOutputCount);

            m_dwStatus |= MYMFT_STATUS_OUTPUT_SAMPLE_READY;
        }

        if(pInputSample->GetUINT64(MYMFT_MFSampleExtension_Marker, &pun64MarkerID) == S_OK)
        {
            // This input sample flagged a marker
            IMFMediaEvent*  pMarkerEvent    = NULL;

            do
            {
                hr = MFCreateMediaEvent(METransformMarker, GUID_NULL, S_OK, NULL, &pMarkerEvent);
                if(FAILED(hr))
                {
                    break;
                }

                hr = pMarkerEvent->SetUINT64(MF_EVENT_MFT_CONTEXT, pun64MarkerID);
                if(FAILED(hr))
                {
                    break;
                }

                /***************************************
                ** Since this in an internal function
                ** we know m_pEventQueue can never be
                ** NULL due to InitializeTransform()
                ***************************************/

                hr = m_pEventQueue->QueueEvent(pMarkerEvent);
                if(FAILED(hr))
                {
                    break;
                }
            }while(false);

            SAFERELEASE(pMarkerEvent);

            if(FAILED(hr))
            {
                break;
            }
        }

        // Done processing this sample, request another

        /*******************************
        ** Todo: This MFT only has one
        ** input stream, so RequestSample
        ** is always called with '0'. If
        ** your MFT has more than one
        ** input stream, you will
        ** have to change this logic
        *******************************/
        hr = RequestSample(0); 
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    if(pMediaBuffer != NULL)
    {
        if(pbData != NULL)
        {
            pMediaBuffer->Unlock();
        }
        pMediaBuffer->Release();
    }
    SAFERELEASE(pOutputSample);
    SAFERELEASE(pHaveOutputEvent);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}

BOOL CHWMFT::IsLocked(void)
{
    /***************************************
    ** Since this in an internal function
    ** we know m_pAttributes can never be
    ** NULL due to InitializeTransform()
    ***************************************/

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    BOOL bUnlocked  = MFGetAttributeUINT32(m_pAttributes,
        MF_TRANSFORM_ASYNC_UNLOCK,
        FALSE
        );

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(%s)", __FUNCTION__, (bUnlocked != FALSE) ? L"False" : L"True");

    return (bUnlocked != FALSE) ? FALSE : TRUE;
}

BOOL CHWMFT::IsMFTReady(void)
{
    /*******************************
    ** The purpose of this function
    ** is to ensure that the MFT is
    ** ready for processing
    *******************************/

    BOOL bReady = FALSE;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        CAutoLock lock(&m_csLock);

        m_dwStatus &= (~MYMFT_STATUS_INPUT_ACCEPT_DATA);

        if(m_pInputMT == NULL)
        {
            // The Input type is not set
            break;
        }

        if(m_pOutputMT == NULL)
        {
            // The output type is not set
            break;
        }

        m_dwStatus |= MYMFT_STATUS_INPUT_ACCEPT_DATA; // The MFT is ready for data

        bReady = TRUE;
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(%s)", __FUNCTION__, bReady ? L"True" : L"False");

    return bReady;
}

HRESULT DuplicateAttributes(
    IMFAttributes*  pDest,
    IMFAttributes*  pSource)
{
    HRESULT     hr      = S_OK;
    GUID        guidKey = GUID_NULL;
    PROPVARIANT pv      = {0};

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        if((pDest == NULL) || (pSource == NULL))
        {
            hr = E_POINTER;
            break;
        }

        PropVariantInit(&pv);

        for(UINT32 un32Index = 0; true; un32Index++)
        {
            PropVariantClear(&pv);
            PropVariantInit(&pv);

            hr = pSource->GetItemByIndex(un32Index, &guidKey, &pv);
            if(FAILED(hr))
            {
                if(hr == E_INVALIDARG)
                {
                    // all items copied
                    hr = S_OK;
                }

                break;
            }

            hr = pDest->SetItem(guidKey, pv);
            if(FAILED(hr))
            {
                break;
            }
        }
    }while(false);

    PropVariantClear(&pv);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit(hr=0x%x)", __FUNCTION__, hr);

    return hr;
}