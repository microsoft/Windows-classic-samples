//////////////////////////////////////////////////////////////////////////
//
// Decoder.cpp : CDecoder class implementation.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#include "Decoder.h"

///////////////////////////////////////////////////////////////////////
//  Name: CreateInstance
//  Description:  Static class method to create the CDecoder object.
//  
//  ppDecoder: Receives an AddRef's pointer to the CDecoder object. 
//            The caller must release the pointer.
/////////////////////////////////////////////////////////////////////////

HRESULT CDecoder::CreateInstance(CDecoder **ppDecoder)
{  
    CDecoder *pDecoder = new CDecoder();

    if (!pDecoder)
    {
        return E_OUTOFMEMORY;
    }

    *ppDecoder = pDecoder;
    (*ppDecoder)->AddRef();
            
    TRACE((L"CDecoder created.\n"));

    SAFE_RELEASE (pDecoder);

    return S_OK;
}

// ----- Public Methods -----------------------------------------------
//////////////////////////////////////////////////////////////////////////
//  Name: CDecoder
//  Description: Constructor
//
/////////////////////////////////////////////////////////////////////////

CDecoder::CDecoder()
: m_nRefCount (1),
m_pMFT (NULL),
m_dwInputID (0),
m_dwOutputID (0),
m_DecoderState (0),
m_pMediaController (NULL)
{

};

// ----- Public Methods -----------------------------------------------
//////////////////////////////////////////////////////////////////////////
//  Name: CDecoder
//  Description: Destructor
//
/////////////////////////////////////////////////////////////////////////

CDecoder::~CDecoder()
{
    (void)UnLoad();
}

/////////////////////////////////////////////////////////////////////
// Name: Initialize
//
// Initializes the MFT with decoder object specified by the CLSID.
//
// pclsid: Path name of the file
// pMediaType:  Pointer to the media type of the stream that the
//              the MFT will decode.
/////////////////////////////////////////////////////////////////////

HRESULT CDecoder::Initialize(CLSID clsid, 
                             IMFMediaType *pMediaType)
{

    if (!pMediaType || clsid == GUID_NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    //Unload the existing MFT.
    if (m_pMFT)
    {
        CHECK_HR (hr = UnLoad());
    }

    //Create the MFT decoder
    CHECK_HR (hr = CoCreateInstance(
        clsid, 
        NULL,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMFTransform),
        (void**)&m_pMFT));


    //Create the media controller that will work with uncompressed data that the decoder generates
    if (!m_pMediaController)
    {
        CHECK_HR (hr = CMediaController::CreateInstance(&m_pMediaController)); 
    }

    CHECK_HR (hr =  ConfigureDecoder( pMediaType));

    TRACE((L"MFT initialized.\n"));

done:
    if (FAILED(hr))
    {
        LOG_MSG_IF_FAILED(L"MFT creation failed\n", hr);

        hr = UnLoad();
    }
   
    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: UnLoad
//
// Unloads the MFT.
//
/////////////////////////////////////////////////////////////////////

HRESULT CDecoder::UnLoad()
{
    HRESULT hr = S_OK;
    
    if (m_pMFT)
    {
        if (m_pMediaController)
        {
            CHECK_HR (hr = m_pMediaController->Reset());
        }
        SAFE_RELEASE(m_pMFT);
    }
    
    TRACE((L"MFT unloaded.\n"));

done:
    
    LOG_MSG_IF_FAILED(L"MFT could not be unloaded.\n", hr);

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: ConfigureDecoder
//
// Configures the MFT with the currently loaded decoder.
//
// pMediaType:  Pointer to the media type of the stream that will the
//              input type of the decoder.
/////////////////////////////////////////////////////////////////////

HRESULT CDecoder::ConfigureDecoder(IMFMediaType *pMediaType)
{
    if (!pMediaType)
    {
        return E_INVALIDARG;
    }

    if (! m_pMFT)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK, hrRes = S_OK;

    GUID guidMajorType = GUID_NULL, guidSubType = GUID_NULL;

    IMFMediaType* pOutputType = NULL;


    //Because this is a decoder transform, the number of input=output=1
    //Get the input and output stream ids. This is different from the stream numbers

    hr = m_pMFT->GetStreamIDs( 1, &m_dwInputID, 1, &m_dwOutputID );

    //Set the input type to the one that is received

    if (SUCCEEDED(hr) || hr == E_NOTIMPL)
    {
        CHECK_HR (hr = m_pMFT->SetInputType( m_dwInputID, pMediaType, 0 ));
    }

    if (SUCCEEDED(hr))
    {
        //Loop through the available output type until we find:
        //For audio media type: PCM audio
        //For video media type: uncompressed RGB32
        for ( DWORD dwTypeIndex = 0; (hrRes != MF_E_NO_MORE_TYPES) ; dwTypeIndex++ )
        {
            hrRes =  m_pMFT->GetOutputAvailableType(
                                                m_dwOutputID,
                                                dwTypeIndex,
                                                &pOutputType);

            if (pOutputType && SUCCEEDED(hrRes))
            {
                CHECK_HR (hr = pOutputType->GetMajorType( &guidMajorType ));

                CHECK_HR (hr = pOutputType->GetGUID( MF_MT_SUBTYPE, &guidSubType ));

                if ((guidMajorType == MFMediaType_Audio) && (guidSubType == MFAudioFormat_PCM))
                {
                    CHECK_HR (hr =  m_pMFT->SetOutputType(m_dwOutputID, pOutputType, 0));

                    CHECK_HR (hr =  m_pMediaController->OpenAudioDevice(pOutputType));

                    break;
                }
            
                else if((guidMajorType == MFMediaType_Video) && (guidSubType == MFVideoFormat_RGB32))
                {
                    CHECK_HR (hr =  m_pMFT->SetOutputType(m_dwOutputID, pOutputType, 0));
                    break;
                }

                SAFE_RELEASE(pOutputType);

            }
            else
            {
                //Output type not found

                hr = E_FAIL;
                break;
            }
        }

    }


done:
    
    LOG_MSG_IF_FAILED(L"MFT could not be configured.\n", hr);

    SAFE_RELEASE(pOutputType);

    return hr;

}

/////////////////////////////////////////////////////////////////////
// Name: ProcessAudio
//
// Passes the input sample through the decoder and sends the output samples
// to the CMediaController class. This class adds the buffers of the 
// output sample to the audio test sample that it maintains. When ready, the 
// caller can play the test sample through methods on the CMediaController
//class.
//
// pSample: Pointer to a compressed sample that needs to be decoded
/////////////////////////////////////////////////////////////////////

HRESULT CDecoder::ProcessAudio(IMFSample *pSample)
{
    if (!pSample)
    {
        return E_INVALIDARG;
    }

    if (! m_pMFT || ! m_pMediaController)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK, hrRes = S_OK;

    DWORD dwStatus = 0;

    IMFMediaBuffer* pBufferOut = NULL;
    IMFSample* pSampleOut = NULL;
    
    //get the size of the output buffer processed by the decoder.
    //Again, there is only one output so the output stream id is 0.
    MFT_OUTPUT_STREAM_INFO mftStreamInfo;
    ZeroMemory(&mftStreamInfo, sizeof(MFT_OUTPUT_STREAM_INFO));

    CHECK_HR (hr =  m_pMFT->GetOutputStreamInfo(m_dwOutputID, &mftStreamInfo));

    MFT_OUTPUT_DATA_BUFFER mftOutputData;
    ZeroMemory(&mftOutputData, sizeof(mftOutputData));

    CHECK_HR (hr =  m_pMFT->ProcessInput(m_dwInputID, pSample, 0));


    //Request output samples from the decoder
    do
    {
        //create a buffer for the output sample
        CHECK_HR (hr = MFCreateMemoryBuffer(mftStreamInfo.cbSize, &pBufferOut));

        //Create the output sample
        CHECK_HR (hr = MFCreateSample(&pSampleOut));

        //Add the output buffer 
        CHECK_HR (hr = pSampleOut->AddBuffer(pBufferOut));

        //Set the output sample
        mftOutputData.pSample = pSampleOut;

        //Set the output id
        mftOutputData.dwStreamID = m_dwOutputID;

        //Generate the output sample
        hrRes =  m_pMFT->ProcessOutput(0, 1, &mftOutputData, &dwStatus);

        //Send it to the media controller so that it can collect the test sample
        CHECK_HR (hr =  m_pMediaController->AddToAudioTestSample(mftOutputData.pSample));


        SAFE_RELEASE(pBufferOut);
        SAFE_RELEASE(pSampleOut);
        

    }while(hrRes != MF_E_TRANSFORM_NEED_MORE_INPUT);

done:
    SAFE_RELEASE(pBufferOut);
    SAFE_RELEASE(pSampleOut);
    
    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: ProcessVideo
//
// Passes the input sample through the decoder and sends the output sample data
// to the CMediaController class. This class creates a bitmap for the sample.
// When ready, the caller can display the bitmap through methods on 
// the CMediaController class.
//
// pSample: Pointer to a compressed sample that needs to be decoded
/////////////////////////////////////////////////////////////////////

HRESULT CDecoder::ProcessVideo(IMFSample *pSample)
{
    if (!pSample)
    {
        return E_INVALIDARG;
    }

    if (! m_pMFT || ! m_pMediaController)
    {
        return MF_E_NOT_INITIALIZED;
    }
    
    HRESULT hr = S_OK, hrRes = S_OK;

    DWORD dwStatus = 0;
    
    DWORD cbTotalLength = 0, cbCurrentLength = 0;

    BYTE *pData = NULL;

    IMFMediaBuffer* pBufferOut = NULL;
    IMFSample* pSampleOut = NULL;
    IMFSample* pBitmapSample = NULL;
    IMFMediaType* pMediaType = NULL;

    //Create a buffer for the transform output
    MFT_OUTPUT_STREAM_INFO mftStreamInfo;
    ZeroMemory(&mftStreamInfo, sizeof(MFT_OUTPUT_STREAM_INFO));

    //get the size of the output buffer processed by the decoder.
    //Again, there is only one output so the output stream id is 0.
    CHECK_HR (hr =  m_pMFT->GetOutputStreamInfo(0, &mftStreamInfo));
    
    //Request samples from the decoder
    MFT_OUTPUT_DATA_BUFFER mftOutputData;
    ZeroMemory(&mftOutputData, sizeof(mftOutputData));

    //Create the bitmap sample that the media controller will use to create the bitmap
    CHECK_HR (hr = MFCreateSample(&pBitmapSample));

    //Send input to the decoder. There is only one input stream so the ID is 0.
    CHECK_HR (hr =  m_pMFT->ProcessInput(m_dwInputID, pSample, 0));

    //Request output samples from the decoder
    do
    {
        //create a buffer for the output sample
        CHECK_HR (hr = MFCreateMemoryBuffer(mftStreamInfo.cbSize, &pBufferOut));

        //Create the output sample
        CHECK_HR (hr = MFCreateSample(&pSampleOut));

        //Add the output buffer 
        CHECK_HR (hr = pSampleOut->AddBuffer(pBufferOut));

        //Set the output sample
        mftOutputData.pSample = pSampleOut;

        mftOutputData.dwStreamID = m_dwOutputID;

        //Generate the output sample
        hrRes =  m_pMFT->ProcessOutput(0, 1, &mftOutputData, &dwStatus);

        //Add the buffer to the bitmap sample
        CHECK_HR (hr = pBitmapSample->AddBuffer(pBufferOut));

        SAFE_RELEASE(pBufferOut);
        SAFE_RELEASE(pSampleOut);
        

    }while(hrRes != MF_E_TRANSFORM_NEED_MORE_INPUT);

    //Get all bitmap data in one buffer
    CHECK_HR (hr = pBitmapSample->ConvertToContiguousBuffer(&pBufferOut));

    CHECK_HR (hr =  m_pMFT->GetOutputCurrentType(m_dwOutputID, &pMediaType));  
    
    //Get a pointer to the memory
    CHECK_HR (hr = pBufferOut->Lock(&pData, &cbTotalLength, &cbCurrentLength)); 

    //Send it to the media controller to create the bitmap
    CHECK_HR (hr = m_pMediaController->CreateBitmapForKeyFrame(pData, pMediaType)); 
    
    CHECK_HR (hr = pBufferOut->Unlock());

    pData = NULL;

done:

    if (pData && FAILED(hr))
    {
        pBufferOut->Unlock();
    }

    SAFE_RELEASE(pBufferOut);
    SAFE_RELEASE(pSampleOut);
    SAFE_RELEASE(pMediaType);


    return hr;
}

HRESULT CDecoder::StartDecoding(void)
{
    if(! m_pMFT)
    {
        return MF_E_NOT_INITIALIZED;
    }
    
    HRESULT hr =  m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
    
    if (SUCCEEDED(hr))
    {
         m_DecoderState = STREAMING;
    }
    return hr;

}

HRESULT CDecoder::StopDecoding(void)
{
    if(! m_pMFT)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr =  m_pMFT->ProcessMessage(MFT_MESSAGE_NOTIFY_END_STREAMING, 0);
    
    if (SUCCEEDED(hr))
    {
         m_DecoderState = NOT_STREAMING;
    }
    return hr;

}
