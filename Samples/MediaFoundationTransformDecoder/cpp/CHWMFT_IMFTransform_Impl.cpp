#include "CHWMFT.h"
#include "CAutoLock.h"
#include <mferror.h>
#include <mfapi.h>
#include "CHWMFT_DebugLogger.h"

// Helper Macros
#define SAFERELEASE(x) \
    if((x) != NULL) \
    { \
        (x)->Release(); \
        (x) = NULL; \
    } \

/****************************
******* IMFTransform ********
****************************/

HRESULT CHWMFT::AddInputStreams(
    DWORD   dwStreams,
    DWORD*  pdwStreamIDs)
{
    /*****************************************
    ** Todo: If your MFT does not have a fixed
    ** number of input streams, you must implement
    ** this function, see:
    ** http://msdn.microsoft.com/en-us/library/ms696211(v=VS.85).aspx
    *****************************************/

    return E_NOTIMPL;
}

HRESULT CHWMFT::DeleteInputStream(
    DWORD   dwStreamID)
{
    /*****************************************
    ** Todo: If your MFT does not have a fixed
    ** number of input streams, you must implement
    ** this function, see:
    ** http://msdn.microsoft.com/en-us/library/ms703159(v=VS.85).aspx
    *****************************************/

    return E_NOTIMPL;
}

HRESULT CHWMFT::GetAttributes(
    IMFAttributes** ppAttributes)
{
    HRESULT hr = S_OK;

    do
    {
        if(ppAttributes == NULL)
        {
            hr = E_POINTER;
            break;
        }

        (*ppAttributes) = m_pAttributes;

        if((*ppAttributes) == NULL)
        {
            hr = E_UNEXPECTED;
            break;
        }

        (*ppAttributes)->AddRef();
    }while(false);

    return hr;
}

HRESULT CHWMFT::GetInputAvailableType(
    DWORD           dwInputStreamID,
    DWORD           dwTypeIndex,
    IMFMediaType**  ppType)
{
    /*****************************************
    ** Todo: This function will return a media
    ** type at a given index. The SDK 
    ** implementation uses a static array of
    ** media types. Your MFT may want to use
    ** a dynamic array and modify the list 
    ** order depending on the MFTs state
    ** See http://msdn.microsoft.com/en-us/library/ms704814(v=VS.85).aspx
    *****************************************/

    HRESULT         hr      = S_OK;
    IMFMediaType*   pMT     = NULL;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(ppType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        /*****************************************
        ** Todo: Modify the accepted input list
        ** g_ppguidInputTypes or use your own
        ** implementation of this function
        *****************************************/
        if(dwTypeIndex >= g_dwNumInputTypes)
        {
            hr = MF_E_NO_MORE_TYPES;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            hr = MFCreateMediaType(&pMT);
            if(FAILED(hr))
            {
                break;
            }

            hr = pMT->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            if(FAILED(hr))
            {
                break;
            }

            hr = pMT->SetGUID(MF_MT_SUBTYPE, *(g_ppguidInputTypes[dwTypeIndex]));
            if(FAILED(hr))
            {
                break;
            }

            (*ppType) = pMT;
            (*ppType)->AddRef();
        }
    }while(false);

    SAFERELEASE(pMT);

    return hr;
}

HRESULT CHWMFT::GetInputCurrentType(
    DWORD           dwInputStreamID,
    IMFMediaType**  ppType)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms705607(v=VS.85).aspx
    *****************************************/

    HRESULT         hr      = S_OK;
    IMFMediaType*   pMT     = NULL;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(ppType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            if(m_pInputMT == NULL)
            {
                hr = MF_E_TRANSFORM_TYPE_NOT_SET;
                break;
            }

            /*******************************************
            ** Return a copy of the media type, not the 
            ** internal one. Returning the internal one 
            ** will allow an external component to modify
            ** the internal media type
            *******************************************/

            hr = MFCreateMediaType(&pMT);
            if(FAILED(hr))
            {
                break;
            }

            hr = DuplicateAttributes(pMT, m_pInputMT);
            if(FAILED(hr))
            {
                break;
            }
        }

        (*ppType) = pMT;
        (*ppType)->AddRef();
    }while(false);

    SAFERELEASE(pMT);

    return hr;
}

HRESULT CHWMFT::GetInputStatus(
    DWORD   dwInputStreamID,
    DWORD*  pdwFlags)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms697478(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pdwFlags == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        *pdwFlags = 0;

        {
            CAutoLock lock(&m_csLock);

            if((m_dwStatus & MYMFT_STATUS_INPUT_ACCEPT_DATA) != 0)
            {
                *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
            }
        }
    }while(false);

    return hr;
}

HRESULT CHWMFT::GetInputStreamAttributes(
    DWORD           dwInputStreamID,
    IMFAttributes** ppAttributes)
{
    /*****************************************
    ** Todo: Becuase this MFT is acting as a decoder
    ** There will not be an upstream HW MFT.
    ** As such, no input stream attributes
    ** are required.
    ** See http://msdn.microsoft.com/en-us/library/ms695366(v=VS.85).aspx,
    ** http://msdn.microsoft.com/en-us/library/dd940330(VS.85).aspx#handshake
    *****************************************/

    return E_NOTIMPL;
}

HRESULT CHWMFT::GetInputStreamInfo(
    DWORD                   dwInputStreamID,
    MFT_INPUT_STREAM_INFO*  pStreamInfo)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms703894(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pStreamInfo == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        pStreamInfo->hnsMaxLatency  = 0; // All input is turned directly into output
        pStreamInfo->dwFlags        = MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER;
        pStreamInfo->cbSize         = 0; // No minimum size is required
        pStreamInfo->cbMaxLookahead = 0; // No lookahead is performed
        pStreamInfo->cbAlignment    = 0; // No memory allignment is required
    }while(false);

    return hr;
}

HRESULT CHWMFT::GetOutputAvailableType(
    DWORD           dwOutputStreamID,
    DWORD           dwTypeIndex,
    IMFMediaType**  ppType)
{
    /*****************************************
    ** Todo: This function will return a media
    ** type at a given index. The SDK 
    ** implementation uses a static array of
    ** media types. Your MFT may want to use
    ** a dynamic array and modify the list 
    ** order depending on the MFTs state
    ** See http://msdn.microsoft.com/en-us/library/ms703812(v=VS.85).aspx
    *****************************************/

    HRESULT         hr      = S_OK;
    IMFMediaType*   pMT     = NULL;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(ppType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwOutputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        /*****************************************
        ** Todo: Modify the accepted output list
        ** g_ppguidOutputTypes or use your own
        ** implementation of this function
        *****************************************/
        if(dwTypeIndex >= g_dwNumOutputTypes)
        {
            hr = MF_E_NO_MORE_TYPES;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            hr = MFCreateMediaType(&pMT);
            if(FAILED(hr))
            {
                break;
            }

            hr = pMT->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            if(FAILED(hr))
            {
                break;
            }

            hr = pMT->SetGUID(MF_MT_SUBTYPE, *(g_ppguidOutputTypes[dwTypeIndex]));
            if(FAILED(hr))
            {
                break;
            }

            /*****************************************
            ** Todo: The following implementation
            ** forces a standard output resolution
            ** and framerate. Your MFT should set these
            ** values properly and update the Media
            ** Type as necessary after decoding the
            ** stream
            *****************************************/
            hr = MFSetAttributeSize(pMT, MF_MT_FRAME_SIZE, MFT_OUTPUT_WIDTH, MFT_OUTPUT_HEIGHT);
            if(FAILED(hr))
            {
                break;
            }

            hr = MFSetAttributeRatio(pMT, MF_MT_FRAME_RATE, MFT_FRAMERATE_NUMERATOR, MFT_FRAMERATE_DENOMINATOR);
            if(FAILED(hr))
            {
                break;
            }

            (*ppType) = pMT;
            (*ppType)->AddRef();
        }
    }while(false);

    SAFERELEASE(pMT);

    return hr;
}

HRESULT CHWMFT::GetOutputCurrentType(
    DWORD           dwOutputStreamID,
    IMFMediaType**  ppType)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms696985(v=VS.85).aspx
    *****************************************/

    HRESULT         hr      = S_OK;
    IMFMediaType*   pMT     = NULL;

    do
    {
        /************************************
        ** Since this MFT is a decoder, it
        ** must not allow this function to be
        ** called until it is unlocked. If
        ** your MFT is an encoder, this function
        ** CAN be called before the MFT is
        ** unlocked
        ************************************/
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(ppType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwOutputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            if(m_pOutputMT == NULL)
            {
                hr = MF_E_TRANSFORM_TYPE_NOT_SET;
                break;
            }

            /*******************************************
            ** Return a copy of the media type, not the 
            ** internal one. Returning the internal one 
            ** will allow an external component to modify
            ** the internal media type
            *******************************************/

            hr = MFCreateMediaType(&pMT);
            if(FAILED(hr))
            {
                break;
            }

            hr = DuplicateAttributes(pMT, m_pOutputMT);
            if(FAILED(hr))
            {
                break;
            }
        }

        (*ppType) = pMT;
        (*ppType)->AddRef();
    }while(false);

    SAFERELEASE(pMT);

    return hr;
}

HRESULT CHWMFT::GetOutputStatus(
    DWORD*  pdwFlags)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms696269(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pdwFlags == NULL)
        {
            hr = E_POINTER;
            break;
        }

        (*pdwFlags) = 0;

        {
            CAutoLock lock(&m_csLock);

            if((m_dwStatus & MYMFT_STATUS_OUTPUT_SAMPLE_READY) != 0)
            {
                *pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
            }
        }

        TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Output Status Flags: 0x%x",  __FUNCTION__, (*pdwFlags));
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit (hr=0x%x)",  __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::GetOutputStreamAttributes(
    DWORD           dwOutputStreamID,
    IMFAttributes** ppAttributes)
{
    /*****************************************
    ** Todo: This MFT does not support a 
    ** hardware handshake, so this function
    ** is not implemented
    ** See http://msdn.microsoft.com/en-us/library/ms703886(v=VS.85).aspx,
    ** http://msdn.microsoft.com/en-us/library/dd940330(VS.85).aspx#handshake
    *****************************************/

    return E_NOTIMPL;
}

HRESULT CHWMFT::GetOutputStreamInfo(
    DWORD                   dwOutputStreamID,
    MFT_OUTPUT_STREAM_INFO* pStreamInfo)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms693880(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pStreamInfo == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwOutputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        pStreamInfo->dwFlags        =   MFT_OUTPUT_STREAM_WHOLE_SAMPLES             | 
                                        MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER  |
                                        MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE         |
                                        MFT_OUTPUT_STREAM_CAN_PROVIDE_SAMPLES;         
        pStreamInfo->cbSize         = (MFT_OUTPUT_WIDTH * MFT_OUTPUT_HEIGHT) * 4; // Since the MFT can output RGB32,
                                                                                  // it may need as many as 4 bytes
                                                                                  // per pixel, so the output buffer
                                                                                  // size must be set accordinly
                                                                                  // Todo: Change this value depending
                                                                                  // On the current output type
        pStreamInfo->cbAlignment    = 0; // No memory allignment is required
    }while(false);

    return hr;
}

HRESULT CHWMFT::GetStreamCount(
    DWORD*  pdwInputStreams,
    DWORD*  pdwOutputStreams)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms697018(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if((pdwInputStreams == NULL) || (pdwOutputStreams == NULL))
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/

        *pdwInputStreams = MFT_MAX_STREAMS;
        *pdwOutputStreams = MFT_MAX_STREAMS;
    }while(false);

    return hr;
}

HRESULT CHWMFT::GetStreamIDs(
    DWORD   dwInputIDArraySize,
    DWORD*  pdwInputIDs,
    DWORD   dwOutputIDArraySize,
    DWORD*  pdwOutputIDs)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms693988(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if((pdwInputIDs == NULL) || (pdwOutputIDs == NULL))
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/

        if((dwInputIDArraySize < MFT_MAX_STREAMS) || (dwOutputIDArraySize < MFT_MAX_STREAMS))
        {
            hr = MF_E_BUFFERTOOSMALL;
            break;
        }

        pdwInputIDs[0]  = 0;
        pdwOutputIDs[0] = 0;
    }while(false);

    return hr;
}

HRESULT CHWMFT::GetStreamLimits(
    DWORD*  pdwInputMinimum,
    DWORD*  pdwInputMaximum,
    DWORD*  pdwOutputMinimum,
    DWORD*  pdwOutputMaximum)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms697040(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if((pdwInputMinimum == NULL) || (pdwInputMaximum == NULL) ||
            (pdwOutputMinimum == NULL) || (pdwOutputMaximum == NULL))
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/

        *pdwInputMinimum    = MFT_MAX_STREAMS;
        *pdwInputMaximum    = MFT_MAX_STREAMS;
        *pdwOutputMinimum   = MFT_MAX_STREAMS;
        *pdwOutputMaximum   = MFT_MAX_STREAMS;
    }while(false);

    return hr;
}

HRESULT CHWMFT::ProcessEvent(
    DWORD           dwInputStreamID,
    IMFMediaEvent*  pEvent)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms695394(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pEvent == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        /****************************************
        ** Todo: this MFT does not handle any 
        ** events. It allows them all to be 
        ** propagated downstream. If your MFT
        ** needs to handle events, implement this
        ** function
        ****************************************/
        hr = E_NOTIMPL;
    }while(false);

    return hr;
}

HRESULT CHWMFT::ProcessInput(
    DWORD       dwInputStreamID,
    IMFSample*  pSample,
    DWORD       dwFlags)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms703131(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): NeedInputCount: %u",  __FUNCTION__, m_dwNeedInputCount);

            if(m_dwNeedInputCount == 0)
            {
                // This call does not correspond to a need input call
                hr = MF_E_NOTACCEPTING;
                break;
            }
            else
            {
                m_dwNeedInputCount--;
            }
        }

        if(pSample == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        // First, put sample into the input Queue

        /***************************************
        ** Since this in an internal function
        ** we know m_pInputSampleQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/
        hr = m_pInputSampleQueue->AddSample(pSample);
        if(FAILED(hr))
        {
            break;
        }

        // Now schedule the work to decode the sample
        hr = ScheduleFrameDecode();
        if(FAILED(hr))
        {
            break;
        }
    }while(false);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit (hr=0x%x)",  __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::ProcessMessage(
    MFT_MESSAGE_TYPE eMessage,
    ULONG_PTR ulParam)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms701863(v=VS.85).aspx
    *****************************************/

    HRESULT hr = S_OK;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if((m_pInputMT == NULL) || (m_pOutputMT == NULL))
        {
            // Can't process messages until media types are set
            hr = MF_E_TRANSFORM_TYPE_NOT_SET;
            break;
        }

        switch(eMessage)
        {
        case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
            {
                hr = OnStartOfStream();
                if(FAILED(hr))
                {
                    break;
                }
            }
            break;
        case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
            {
                hr = OnEndOfStream();
                if(FAILED(hr))
                {
                    break;
                }
            }
            break;
        case MFT_MESSAGE_COMMAND_DRAIN:
            {
                hr = OnDrain((UINT32)ulParam);
                if(FAILED(hr))
                {
                    break;
                }
            }
            break;
        case MFT_MESSAGE_COMMAND_FLUSH:
            {
                hr = OnFlush();
                if(FAILED(hr))
                {
                    break;
                }
            }
            break;
        case MFT_MESSAGE_COMMAND_MARKER:
            {
                hr = OnMarker(ulParam);
                if(FAILED(hr))
                {
                    break;
                }
            }
            break;
        /************************************************
        ** Todo: Add any MFT Messages that are not already
        ** covered
        ************************************************/
        default:
            // Nothing to do, return S_OK
            break;
        };
    }while(false);

    return hr;
}

HRESULT CHWMFT::ProcessOutput(
    DWORD                   dwFlags,
    DWORD                   dwOutputBufferCount,
    MFT_OUTPUT_DATA_BUFFER* pOutputSamples,
    DWORD*                  pdwStatus)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms704014(v=VS.85).aspx
    *****************************************/

    HRESULT     hr      = S_OK;
    IMFSample*  pSample = NULL;

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Enter",  __FUNCTION__);

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): HaveOutputCount: %u",  __FUNCTION__, m_dwHaveOutputCount);

            if(m_dwHaveOutputCount == 0)
            {
                // This call does not correspond to a have output call

                hr = E_UNEXPECTED;
                break;
            }
            else
            {
                m_dwHaveOutputCount--;
            }
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwOutputBufferCount < MFT_MAX_STREAMS)
        {
            hr = E_INVALIDARG;
            break;
        }

        if(IsMFTReady() == FALSE)
        {
            hr = MF_E_TRANSFORM_TYPE_NOT_SET;
            break;
        }

        /***************************************
        ** Since this in an internal function
        ** we know m_pOutputSampleQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/
        hr = m_pOutputSampleQueue->GetNextSample(&pSample);
        if(FAILED(hr))
        {
            break;
        }

        if(pSample == NULL)
        {
            hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
            break;
        }

        /*******************************
        ** Todo: This MFT only has one
        ** input stream, so the output
        ** samples array and stream ID
        ** will only use the first
        ** member
        *******************************/
        pOutputSamples[0].dwStreamID    = 0;
        
        if((pOutputSamples[0].pSample) == NULL)
        {
            // The MFT is providing it's own samples
            (pOutputSamples[0].pSample)   = pSample;
            (pOutputSamples[0].pSample)->AddRef();
        }
        else
        {
            // The pipeline has allocated the samples
            IMFMediaBuffer* pBuffer = NULL;

            do
            {
                hr = pSample->ConvertToContiguousBuffer(&pBuffer);
                if(FAILED(hr))
                {
                    break;
                }

                hr = (pOutputSamples[0].pSample)->AddBuffer(pBuffer);
                if(FAILED(hr))
                {
                    break;
                }
            }while(false);

            SAFERELEASE(pBuffer);

            if(FAILED(hr))
            {
                break;
            }
        }

        /***************************************
        ** Since this in an internal function
        ** we know m_pOutputSampleQueue can never be
        ** NULL due to InitializeTransform()
        ***************************************/
        if(m_pOutputSampleQueue->IsQueueEmpty() != FALSE)
        {
            // We're out of samples in the output queue
            CAutoLock lock(&m_csLock);

            if((m_dwStatus & MYMFT_STATUS_DRAINING) != 0)
            {
                // We're done draining, time to send the event
                IMFMediaEvent*  pDrainCompleteEvent  = NULL;

                do
                {
                    hr = MFCreateMediaEvent(METransformDrainComplete , GUID_NULL, S_OK, NULL, &pDrainCompleteEvent);
                    if(FAILED(hr))
                    {
                        break;
                    }

                    /*******************************
                    ** Todo: This MFT only has one
                    ** input stream, so the drain
                    ** is always on stream zero.
                    ** Update this is your MFT
                    ** has more than one stream
                    *******************************/
                    hr = pDrainCompleteEvent->SetUINT32(MF_EVENT_MFT_INPUT_STREAM_ID, 0);
                    if(FAILED(hr))
                    {
                        break;
                    }

                    /***************************************
                    ** Since this in an internal function
                    ** we know m_pEventQueue can never be
                    ** NULL due to InitializeTransform()
                    ***************************************/
                    hr = m_pEventQueue->QueueEvent(pDrainCompleteEvent);
                    if(FAILED(hr))
                    {
                        break;
                    }
                }while(false);

                SAFERELEASE(pDrainCompleteEvent);

                if(FAILED(hr))
                {
                    break;
                }

                m_dwStatus &= (~MYMFT_STATUS_DRAINING);
            }
        }
    }while(false);

    SAFERELEASE(pSample);

    TraceString(CHMFTTracing::TRACE_INFORMATION, L"%S(): Exit (hr=0x%x)",  __FUNCTION__, hr);

    return hr;
}

HRESULT CHWMFT::SetInputType(
    DWORD           dwInputStreamID,
    IMFMediaType*   pType,
    DWORD           dwFlags)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms700113(v=VS.85).aspx
    *****************************************/

    HRESULT         hr      = S_OK;
    IMFMediaType*   pMT     = NULL;

    do
    {
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwInputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        hr = CheckInputType(pType);
        if(FAILED(hr))
        {
            break;
        }

        /*******************************************
        ** Store a copy of the media type, not the
        ** one passed in by the caller. This way the
        ** caller is unable to modify the internal
        ** media type
        *******************************************/

        hr = MFCreateMediaType(&pMT);
        if(FAILED(hr))
        {
            break;
        }

        hr = DuplicateAttributes(pMT, pType);
        if(FAILED(hr))
        {
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            SAFERELEASE(m_pInputMT);

            m_pInputMT = pMT;
            m_pInputMT->AddRef();
        }

        IsMFTReady();
    }while(false);

    SAFERELEASE(pMT);

    return hr;
}

HRESULT CHWMFT::SetOutputBounds(
    LONGLONG hnsLowerBound,
    LONGLONG hnsUpperBound)
{
    /*****************************************
    ** Todo: This MFT does not support  
    ** sample boundries
    ** See http://msdn.microsoft.com/en-us/library/ms693812(v=VS.85).aspx
    *****************************************/

    return E_NOTIMPL;
}

HRESULT CHWMFT::SetOutputType(
    DWORD           dwOutputStreamID,
    IMFMediaType*   pType,
    DWORD           dwFlags)
{
    /*****************************************
    ** See http://msdn.microsoft.com/en-us/library/ms702016(v=VS.85).aspx
    *****************************************/

    HRESULT         hr      = S_OK;
    IMFMediaType*   pMT     = NULL;

    do
    {
        /************************************
        ** Since this MFT is a decoder, it
        ** must not allow this function to be
        ** called until it is unlocked. If
        ** your MFT is an encoder, this function
        ** CAN be called before the MFT is
        ** unlocked
        ************************************/
        if(IsLocked() != FALSE)
        {
            hr = MF_E_TRANSFORM_ASYNC_LOCKED;
            break;
        }

        if(pType == NULL)
        {
            hr = E_POINTER;
            break;
        }

        /*****************************************
        ** Todo: If your MFT supports more than one
        ** stream, make sure you modify
        ** MFT_MAX_STREAMS and adjust this function
        ** accordingly
        *****************************************/
        if(dwOutputStreamID >= MFT_MAX_STREAMS)
        {
            hr = MF_E_INVALIDSTREAMNUMBER;
            break;
        }

        hr = CheckOutputType(pType);
        if(FAILED(hr))
        {
            break;
        }

        /*******************************************
        ** Store a copy of the media type, not the
        ** one passed in by the caller. This way the
        ** caller is unable to modify the internal
        ** media type
        *******************************************/

        hr = MFCreateMediaType(&pMT);
        if(FAILED(hr))
        {
            break;
        }

        hr = DuplicateAttributes(pMT, pType);
        if(FAILED(hr))
        {
            break;
        }

        {
            CAutoLock lock(&m_csLock);

            SAFERELEASE(m_pOutputMT);

            m_pOutputMT = pMT;
            m_pOutputMT->AddRef();
        }

        IsMFTReady();
    }while(false);

    SAFERELEASE(pMT);

    return hr;
}