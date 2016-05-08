//////////////////////////////////////////////////////////////////////////
//
// ASFManager.cpp : CASFManager class implementation.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#include <new>
#include "ASFManager.h"

// ----- Static Methods -----------------------------------------------
//////////////////////////////////////////////////////////////////////////
//  Name: CreateInstance
//  Description: Instantiates the class statically
//
/////////////////////////////////////////////////////////////////////////

HRESULT CASFManager::CreateInstance(CASFManager **ppASFManager)
{

    // Note: CASFManager constructor sets the ref count to zero.
    // Create method calls AddRef.

    HRESULT hr = S_OK;

    CASFManager *pASFManager = new (std::nothrow) CASFManager(&hr);

    if (!pASFManager)
    {
        return E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        *ppASFManager = pASFManager;
        (*ppASFManager)->AddRef();

        TRACE((L"CASFManager created.\n"));
    }

    LOG_MSG_IF_FAILED(L"CASFManager creation failed.\n", hr);

    SAFE_RELEASE (pASFManager);
    return hr;
}

// ----- Public Methods -----------------------------------------------
//////////////////////////////////////////////////////////////////////////
//  Name: CASFManager
//  Description: Constructor
//
/////////////////////////////////////////////////////////////////////////

CASFManager::CASFManager(HRESULT* hr)
:   m_nRefCount(1),
    m_CurrentStreamID (0),
    m_guidCurrentMediaType (GUID_NULL),
    m_fileinfo(NULL),
    m_pDecoder (NULL),
    m_pContentInfo (NULL),
    m_pIndexer (NULL),
    m_pSplitter (NULL),
    m_pDataBuffer (NULL),
    m_pByteStream(NULL),
    m_cbDataOffset(0),
    m_cbDataLength(0)
{
    //Initialize Media Foundation
    *hr = MFStartup(MF_VERSION);

}

//////////////////////////////////////////////////////////////////////////
//  Name: ~ASFManager
//  Description: Destructor
//
//  -Calls Shutdown
/////////////////////////////////////////////////////////////////////////

CASFManager::~CASFManager()
{

    //Release memory
    Reset();

   // Shutdown the Media Foundation platform
    (void)MFShutdown();

}



/////////////////////////////////////////////////////////////////////
// Name: OpenASFFile
//
// Opens a file and returns a byte stream.
//
// sFileName: Path name of the file
// ppStream:  Receives a pointer to the byte stream.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::OpenASFFile(const WCHAR *sFileName)
{
    HRESULT hr = S_OK;

    IMFByteStream* pStream = NULL;

    // Open a byte stream for the file.
    CHECK_HR(hr = MFCreateFile(
        MF_ACCESSMODE_READ, 
        MF_OPENMODE_FAIL_IF_NOT_EXIST,
        MF_FILEFLAGS_NONE,
        sFileName,
        &pStream
        ));


    TRACE((L"Opened ASF File\n"));

    //Reset the ASF components.
    Reset();

    // Create the Media Foundation ASF objects.
    CHECK_HR(hr = CreateASFContentInfo(pStream, &m_pContentInfo));

    CHECK_HR(hr = CreateASFSplitter(pStream, &m_pSplitter));

    CHECK_HR(hr = CreateASFIndexer(pStream, &m_pIndexer));

done:
    
    LOG_MSG_IF_FAILED(L"CASFManager::OpenASFFile failed.\n", hr);

    SAFE_RELEASE(pStream);

    return hr;
}

 

// ----- Private Methods -----------------------------------------------

/////////////////////////////////////////////////////////////////////
// Name: CreateASFContentInfo
//
// Reads the ASF Header Object from a byte stream and returns a
// pointer to the ASF content information object.
//
// pStream:       Pointer to the byte stream. The byte stream's 
//                current read position must be 0 that indicates the start of the
//                ASF Header Object.
// ppContentInfo: Receives a pointer to the ASF content information
//                object.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::CreateASFContentInfo (IMFByteStream *pContentByteStream,
                                           IMFASFContentInfo **ppContentInfo)
{
    if (!pContentByteStream || !ppContentInfo)
    {
        return E_INVALIDARG;
    }
 
    HRESULT hr = S_OK;
    QWORD cbHeader = 0;

   
    IMFASFContentInfo *pContentInfo = NULL;
    IMFMediaBuffer *pBuffer = NULL;

    // Create the ASF content information object.
    CHECK_HR(hr = MFCreateASFContentInfo(&pContentInfo));
    
    // Read the first 30 bytes to find the total header size.
    CHECK_HR(hr = ReadDataIntoBuffer(
        pContentByteStream, 0, MIN_ASF_HEADER_SIZE, &pBuffer));

    CHECK_HR(hr = pContentInfo->GetHeaderSize(pBuffer, &cbHeader));

    SAFE_RELEASE(pBuffer);
    
    //Read the header into a buffer
    CHECK_HR(hr = ReadDataIntoBuffer(
        pContentByteStream, 0, (DWORD)cbHeader, &pBuffer));

    // Pass the buffer for the header object.
    CHECK_HR(hr = pContentInfo->ParseHeader(pBuffer, 0));


    // Return the pointer to the caller.
    *ppContentInfo = pContentInfo;
    (*ppContentInfo)->AddRef();

    TRACE((L"Created ContentInfo object.\n"));

done:
    
    LOG_MSG_IF_FAILED(L"CASFManager::CreateASFContentInfo failed.\n", hr);

    SAFE_RELEASE(pBuffer);
    SAFE_RELEASE(pContentInfo);
    return hr;
}


/////////////////////////////////////////////////////////////////////
// Name: CreateASFSplitter
//
// Creates the ASF splitter.
//
// pContentByteStream: Pointer to the byte stream that contains the ASF Data Object. 
// ppSplitter:   Receives a pointer to the ASF splitter.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::CreateASFSplitter (IMFByteStream *pContentByteStream,
                                        IMFASFSplitter **ppSplitter)
{
    if (!pContentByteStream || !ppSplitter)
    {
        return E_INVALIDARG;
    }

    if (!m_pContentInfo)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;
    
    IMFASFSplitter *pSplitter = NULL;
    IMFPresentationDescriptor* pPD = NULL;

    UINT64 cbDataOffset = 0, cbDataLength = 0;

    CHECK_HR(hr = MFCreateASFSplitter(&pSplitter));
    
    CHECK_HR(hr = pSplitter->Initialize(m_pContentInfo));

    //Generate the presentation descriptor
    CHECK_HR(hr =  m_pContentInfo->GeneratePresentationDescriptor(&pPD));

    //Get the offset to the start of the Data Object
    CHECK_HR(hr = pPD->GetUINT64(MF_PD_ASF_DATA_START_OFFSET, &cbDataOffset));

    //Get the length of the Data Object
    CHECK_HR(hr = pPD->GetUINT64(MF_PD_ASF_DATA_LENGTH, &cbDataLength));

    m_pByteStream = pContentByteStream;
    m_pByteStream->AddRef();

    m_cbDataOffset = cbDataOffset;
    m_cbDataLength = cbDataLength;

    // Return the pointer to the caller.
    *ppSplitter = pSplitter;
    (*ppSplitter)->AddRef();

    TRACE((L"Created Splitter object.\n"));


done:

    LOG_MSG_IF_FAILED(L"CASFManager::CreateASFSplitter failed.\n", hr);

    SAFE_RELEASE(pSplitter);
    SAFE_RELEASE(pPD);

    return hr;
}
 
/////////////////////////////////////////////////////////////////////
// Name: CreateASFIndexer
//
// Creates the ASF Indexer.
//
// pContentByteStream: Pointer to the content byte stream
//
// ppIndexer:   Receives a pointer to the ASF Indexer.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::CreateASFIndexer (IMFByteStream *pContentByteStream,
                                       IMFASFIndexer **ppIndexer)
{
    if (!pContentByteStream || !ppIndexer)
    {
        return E_INVALIDARG;
    }

    if (!m_pContentInfo)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;

    IMFASFIndexer *pIndexer = NULL;
    IMFByteStream *pIndexerByteStream = NULL;

    QWORD qwLength = 0, qwIndexOffset = 0, qwBytestreamLength = 0;
    DWORD dwByteStreamsNeeded = 0;

    // Create the indexer.
    CHECK_HR(hr = MFCreateASFIndexer(&pIndexer));

    //Initialize the indexer to work with this ASF library
    CHECK_HR(hr =  pIndexer->Initialize(m_pContentInfo));


    //Check if the index exists. 
    //you can only do this after creating the indexer

    //Get byte stream length
    CHECK_HR(hr = pContentByteStream->GetLength(&qwLength));

    //Get index offset
    CHECK_HR(hr = pIndexer->GetIndexPosition( m_pContentInfo, &qwIndexOffset ));
    
    if ( qwIndexOffset >= qwLength)
    {
        //index object does not exist, release the indexer
        goto done;
    }
    else
    {
        // initialize the indexer
        // Create a byte stream that the Indexer will use to read in
        // and parse the indexers.
         CHECK_HR(hr = MFCreateASFIndexerByteStream( pContentByteStream,
                                qwIndexOffset,
                                &pIndexerByteStream ));
    }

    CHECK_HR(hr = pIndexer->GetIndexByteStreamCount( &dwByteStreamsNeeded ));

    if (dwByteStreamsNeeded == 1)
    {
        CHECK_HR(hr = pIndexer->SetIndexByteStreams( &pIndexerByteStream, dwByteStreamsNeeded ));
    }
    
    // Return the pointer to the caller.
    *ppIndexer = pIndexer;
    (*ppIndexer)->AddRef();

    TRACE((L"Created Indexer object.\n"));


done:

    LOG_MSG_IF_FAILED(L"CASFManager::CreateASFIndexer failed.\n", hr);
    SAFE_RELEASE(pIndexer);
    SAFE_RELEASE(pIndexerByteStream);

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: EnumerateStreams
//
// Enumerates the streams in the ASF file.
//
// ppwStreamNumbers: Receives the stream identifiers in an array. 
//                   The caller must release the allocated memory.
//
// ppguidMajorType: Receives the major media type GUIDs in an array. 
//                   The caller must release the allocated memory.
//
// cbTotalStreams:   Receives total number of elements in the array.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::EnumerateStreams (WORD** ppwStreamNumbers, 
                                       GUID** ppguidMajorType, 
                                       DWORD* pcbTotalStreams)
{
    if (!ppwStreamNumbers || !ppguidMajorType || !pcbTotalStreams)
    {
        return E_INVALIDARG;
    }

    if (!m_pContentInfo)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;

    IMFASFStreamConfig* pStream = NULL;
    IMFASFProfile* pProfile = NULL;

    *pcbTotalStreams =0;

    WORD* pwStreamNumbers; 
    GUID* pguidMajorType;   

    CHECK_HR(hr =  m_pContentInfo->GetProfile(&pProfile));

    CHECK_HR(hr = pProfile->GetStreamCount(pcbTotalStreams));

    if (*pcbTotalStreams == 0)
    {
        SAFE_RELEASE(pProfile);

        return E_FAIL;
    }

    //create an array of stream numbers and initialize elements swith 0
    pwStreamNumbers = new WORD[*pcbTotalStreams*sizeof(WORD)];

    if (!pwStreamNumbers)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ZeroMemory (pwStreamNumbers, *pcbTotalStreams*sizeof(WORD));

    //create an array of guids and initialize elements with GUID_NULL.
    pguidMajorType = new GUID[*pcbTotalStreams*sizeof(GUID)];
    
    if (!pguidMajorType)
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    ZeroMemory (pguidMajorType, *pcbTotalStreams*sizeof(GUID));

    //populate the arrays with stream numbers and guids from the profile object
    for (unsigned index = 0; index < *pcbTotalStreams; index++)
    {
        CHECK_HR(hr = pProfile->GetStream(index, &pwStreamNumbers[index], &pStream));

        CHECK_HR(hr = pStream->GetStreamType(&pguidMajorType[index]));

        SAFE_RELEASE(pStream);
    }


    *ppwStreamNumbers = pwStreamNumbers;
    *ppguidMajorType = pguidMajorType;

    TRACE((L"Enumerated streams.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::EnumerateStreams failed.\n", hr);

    SAFE_RELEASE(pProfile);
    SAFE_RELEASE(pStream);

    if (FAILED (hr))
    {
        SAFE_ARRAY_DELETE(pwStreamNumbers);
        SAFE_ARRAY_DELETE(pguidMajorType);
    }

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: SelectStream
//
// Selects the streams in the ASF file.
//
// pwStreamNumber: Specifies the identifier of the stream to be selected
//                 on the splitter. 
//
// pguidCurrentMediaType: Receives the major media type GUID of the 
//                   currently selected stream.
//
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::SelectStream (WORD wStreamNumber,
                                   GUID* pguidCurrentMediaType)
{
    HRESULT hr = S_OK;

    if (wStreamNumber == 0 || !pguidCurrentMediaType)
    {
        return E_INVALIDARG;
    }

    if (! m_pSplitter || ! m_pContentInfo)
    {
        return MF_E_NOT_INITIALIZED;
    }

    //Select the stream you want to parse. This sample allows you to select only one stream at a time
    CHECK_HR(hr  =  m_pSplitter->SelectStreams(&wStreamNumber, 1));

    //Load the appropriate stream decoder
    CHECK_HR(hr = SetupStreamDecoder(wStreamNumber, pguidCurrentMediaType));
    
    m_CurrentStreamID = wStreamNumber;

    m_guidCurrentMediaType = *pguidCurrentMediaType;

    TRACE((L"Stream selected.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::SelectStreams failed.\n", hr);

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: SetupStreamDecoder
//
// Loads the appropriate decoder for stream. The decoder is implemented as
// a Media Foundation Transform (MFT). The class CDecoder provides a wrapper for 
// the MFT. The CASFManager::GenerateSamples feeds compressed samples to 
// the decoder. The MFT decodes the samples and sends them to the CMediaController
// object, which plays 10 seconds of uncompressed audio samples or displays the
// key frame for the video stream
//
// wStreamNumber: Specifies the identifier of the stream. 
//
// pguidCurrentMediaType: Receives the major media type GUID of the 
//                   currently selected stream.
//
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::SetupStreamDecoder (WORD wStreamNumber, 
                                         GUID* pguidCurrentMediaType)
{
    if (! m_pContentInfo)
    {
        return MF_E_NOT_INITIALIZED;
    }

    if (wStreamNumber == 0)
    {
        return E_INVALIDARG;
    }

    IMFASFProfile* pProfile = NULL;
    IMFMediaType* pMediaType = NULL;
    IMFASFStreamConfig *pStream = NULL;

    GUID    guidMajorType = GUID_NULL;
    GUID    guidSubType = GUID_NULL;
    GUID    guidDecoderCategory = GUID_NULL;

    BOOL fIsCompressed = TRUE;

    CLSID *pDecoderCLSIDs = NULL;   // Pointer to an array of CLISDs. 
    UINT32 cDecoderCLSIDs = 0;   // Size of the array.
    
    HRESULT hr = S_OK;

    //Get the profile object that stores stream information
    CHECK_HR(hr =  m_pContentInfo->GetProfile(&pProfile));

    //Get stream configuration object from the profile
    CHECK_HR(hr = pProfile->GetStreamByNumber(wStreamNumber, &pStream));

    //Get the media type
    CHECK_HR(hr = pStream->GetMediaType(&pMediaType));

    //Get the major media type
    CHECK_HR(hr = pMediaType->GetMajorType(&guidMajorType));
        
    //Get the sub media type
    CHECK_HR(hr = pMediaType->GetGUID(MF_MT_SUBTYPE, &guidSubType));
    
    //find out if the media type is compressed
    CHECK_HR(hr = pMediaType->IsCompressedFormat(&fIsCompressed));

    if (fIsCompressed)
    {
        //get decoder category
        if (guidMajorType == MFMediaType_Video)
        {
            guidDecoderCategory = MFT_CATEGORY_VIDEO_DECODER;
        }
        else if (guidMajorType == MFMediaType_Audio)
        {
            guidDecoderCategory = MFT_CATEGORY_AUDIO_DECODER;
        }
        else
        {
            CHECK_HR(hr = MF_E_INVALIDMEDIATYPE);
        }

        // Look for a decoder.
        MFT_REGISTER_TYPE_INFO tinfo;
        tinfo.guidMajorType = guidMajorType;
        tinfo.guidSubtype = guidSubType;

        CHECK_HR(hr = MFTEnum(
            guidDecoderCategory,
            0,                  // Reserved
            &tinfo,             // Input type to match. (Encoded type.)
            NULL,               // Output type to match. (Don't care.)
            NULL,               // Attributes to match. (None.)
            &pDecoderCLSIDs,    // Receives a pointer to an array of CLSIDs.
            &cDecoderCLSIDs     // Receives the size of the array.
            ));

        // MFTEnum can return zero matches.
        if (cDecoderCLSIDs == 0)
        {
            hr = MF_E_TOPO_CODEC_NOT_FOUND;
        }
        else
        {
            //if the CDecoder instance does not exist, create one.
            if (!m_pDecoder)
            {
                CHECK_HR(hr = CDecoder::CreateInstance(&m_pDecoder));
            }
            
            //Load the first MFT in the array for the current media type
            CHECK_HR(hr = m_pDecoder->Initialize(pDecoderCLSIDs[0], pMediaType));
        }
        *pguidCurrentMediaType = guidMajorType;
    }
    else
    {
        // Not compressed. Don't need a decoder. 
         CHECK_HR(hr = MF_E_INVALIDREQUEST);
    }


    TRACE((L"Stream decoder loaded.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::SetupStreamDecoder failed.\n", hr);
    
    SAFE_RELEASE(pProfile);
    SAFE_RELEASE(pMediaType);
    SAFE_RELEASE(pStream);

    CoTaskMemFree(pDecoderCLSIDs);

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: GetSeekPosition
//
// Gets the offset from the start of the ASF Data Object corresponding
// to the specified time that is seeked by the caller.
//
// hnsSeekTime: [In/out]Seek time in hns. This includes the preroll time. The received 
//              value is the actual seek time wth preroll adjustment.
//
// cbDataOffset: Receives the offset in bytes.
//
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::GetSeekPosition (MFTIME* hnsSeekTime, 
                                      QWORD *pcbDataOffset, 
                                      MFTIME* phnsApproxSeekTime)
{
    HRESULT hr = S_OK;


    //if the media type is audio, or doesn't have an indexed data
    //calculate the offset manually
    if (( m_guidCurrentMediaType == MFMediaType_Audio) || (!m_pIndexer))
    {
        CHECK_HR(hr =  GetSeekPositionManually(*hnsSeekTime, pcbDataOffset));
    }

    //if the type is video, get the position with the indexer
    if (( m_guidCurrentMediaType == MFMediaType_Video))
    {
        CHECK_HR(hr =  GetSeekPositionWithIndexer(*hnsSeekTime, pcbDataOffset, phnsApproxSeekTime));        
    }


    TRACE((L"Offset calculated.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::GetSeekPosition failed.\n", hr);

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: GetSeekPositionManually
//
// Gets the offset for audio media types or ones that do not have ASF Index Objects defined.
//Offset is calculated as fraction with respect to time
//
// hnsSeekTime: Presentation time in hns. 
//
// cbDataOffset: Receives the offset in bytes.
//
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::GetSeekPositionManually(MFTIME hnsSeekTime, 
                                    QWORD *cbDataOffset) 
{
    //Get average packet size
    UINT32 averagepacketsize = ( m_fileinfo->cbMaxPacketSize+ m_fileinfo->cbMinPacketSize)/2;

    DWORD dwFlags = 0;

    double fraction = 0;

    HRESULT hr = S_OK;
    
    //Check if the reverse flag is set, if so, offset is calculated from the end of the presentation
    CHECK_HR(hr = this->m_pSplitter->GetFlags(&dwFlags));

    if (dwFlags & MFASF_SPLITTER_REVERSE)
    {
        fraction = ((double) (m_fileinfo->cbPresentationDuration) - (double) (hnsSeekTime))/(double) (m_fileinfo->cbPresentationDuration);
    }
    else
    {
        fraction = (double)(hnsSeekTime)/(double) (m_fileinfo->cbPresentationDuration);
    }
    
    //calculate the number of packets passed
    int seeked_packets = (int)( m_fileinfo->cbPackets * fraction);
    
    //get the offset
    *cbDataOffset = (QWORD)averagepacketsize * seeked_packets;



    if (*cbDataOffset >= 0)
    {
        return S_OK;
    }

    TRACE((L"Offset calculated through fraction.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::GetSeekPositionManually failed.\n", hr);
    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: GetSeekPositionWithIndexer
//
// Gets the offset for video media types that have ASF Index Objects defined.
//
// hnsSeekTime: Presentation time in hns. 
//
// cbDataOffset: Receives the offset in bytes.
//
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::GetSeekPositionWithIndexer ( 
                        MFTIME hnsSeekTime, 
                        QWORD *cbDataOffset, 
                        MFTIME* hnsApproxSeekTime)
{
    if (! m_pIndexer)
    {
        return MF_E_ASF_NOINDEX;
    }

    HRESULT hr = S_OK;

    PROPVARIANT var;
    PropVariantInit(&var);
    
    var.vt = VT_I8;
    var.hVal.QuadPart = hnsSeekTime;

    ASF_INDEX_IDENTIFIER IndexIdentifier;

    BOOL fIsIndexed = FALSE;
    DWORD cbIndexDescriptor = 0;

    DWORD dwFlags = 0;

    //currently only time index is supported, set to GUID_NULL
    IndexIdentifier.guidIndexType = GUID_NULL;
    IndexIdentifier.wStreamNumber =  m_CurrentStreamID;

    //Is the stream indexed? Get the value of cbIndexDescriptor
    hr = m_pIndexer->GetIndexStatus( &IndexIdentifier,
        &fIsIndexed,
        NULL,
        &cbIndexDescriptor );

    if (hr == MF_E_BUFFERTOOSMALL)
    {
        hr = S_OK;
    }

    CHECK_HR(hr);

    //check if the reverse flag needs to be set on the indexer
    CHECK_HR (hr = m_pSplitter->GetFlags(&dwFlags));

    if (dwFlags & MFASF_SPLITTER_REVERSE)
    {
        CHECK_HR (hr = m_pIndexer->SetFlags(MFASF_INDEXER_READ_FOR_REVERSEPLAYBACK));
    }

    //Get the offset from the indexer
    if (fIsIndexed)
    {
        CHECK_HR(hr = m_pIndexer->GetSeekPositionForValue(
            &var, 
            &IndexIdentifier, 
            cbDataOffset, 
            hnsApproxSeekTime, 
            0 ));
    }
    else
    {
        hr = MF_E_ASF_NOINDEX;
    }

    TRACE((L"Offset calculated through the Indexer.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::GetSeekPositionWithIndexer failed.\n", hr);

    //release memory
    PropVariantClear(&var);

    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: GetTestDuration
//
//For audio stream, this method retrieves the duration of the test sample.
//
// hnsSeekTime: Presentation time in hns. 
// hnsTestDuration: Presentation time in hns that represents the end time. 
// dwFlags: Specifies splitter configuration, generate samples in reverse
//          or generate samples for protected content.
/////////////////////////////////////////////////////////////////////

void CASFManager::GetTestDuration(const MFTIME& hnsSeekTime, BOOL bReverse, MFTIME* phnsTestDuration)
{
    MFTIME hnsMaxSeekableTime = m_fileinfo->cbPlayDuration - m_fileinfo->cbPreroll;

    if (bReverse)
    {
        // Reverse playback: The stop time should not go beyond the start of the
        // ASF Data Object (reading backwards in the file).
        if (hnsSeekTime - TEST_AUDIO_DURATION < 0)
        {
            *phnsTestDuration = 0 ;
        }
        else
        {
            *phnsTestDuration = hnsSeekTime - TEST_AUDIO_DURATION;
        }
        
    }
    else
    {
        // Forward playback: The stop time should not exceed the end of the presentation.
        if (hnsSeekTime + TEST_AUDIO_DURATION > hnsMaxSeekableTime)
        {
            *phnsTestDuration = hnsMaxSeekableTime ;
        }
        else
        {
            *phnsTestDuration = hnsSeekTime + TEST_AUDIO_DURATION;
        }
    }
}

/////////////////////////////////////////////////////////////////////
// Name: GenerateSamples
//
//Gets data offset for the seektime and prepares buffer for parsing.
//
// hnsSeekTime: Presentation time in hns. 
// dwFlags: Specifies splitter configuration, generate samples in 
//          reverse or generate samples for protected content.
// pSampleInfo: Pointer to SAMPLE_INFO structure that stores sample 
//          information.
// FuncPtrToDisplaySampleInfo: Callback defined by the caller that 
//          will display the sample information
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::GenerateSamples(
    MFTIME hnsSeekTime, 
    DWORD dwFlags,
    SAMPLE_INFO* pSampleInfo,
    void (*FuncPtrToDisplaySampleInfo)(SAMPLE_INFO*)
    )
{
    if (! m_pSplitter)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;
    QWORD   cbStartOffset = 0;
    DWORD   cbReadLen = 0;
    MFTIME  hnsApproxTime =0;
    MFTIME  hnsTestSampleDuration =0;
    BOOL    bReverse = FALSE;

    // Flush the splitter to remove any samples that were delivered
    // to the ASF splitter during a previous call to this method.
    CHECK_HR(hr = m_pSplitter->Flush());

    //set the reverse flag if applicable
    hr = m_pSplitter->SetFlags(dwFlags);

    if (FAILED (hr))
    {
        dwFlags = 0;
        hr = S_OK;
    }

    bReverse = ((dwFlags & MFASF_SPLITTER_REVERSE) == MFASF_SPLITTER_REVERSE);

    // Get the offset from the start of the ASF Data Object to the desired seek time.
    CHECK_HR(hr =  GetSeekPosition(&hnsSeekTime, &cbStartOffset, &hnsApproxTime));

    // Get the audio playback duration. (The duration is TEST_AUDIO_DURATION or up to
    // the end of the file, whichever is shorter.)
    if (m_guidCurrentMediaType == MFMediaType_Audio)
    {
        GetTestDuration(hnsSeekTime, bReverse, &hnsTestSampleDuration);
    }

    // Notify the MFT we are about to start.
    if (m_pDecoder)
    {
        if ( m_pDecoder->GetDecoderStatus() != STREAMING)
        {
            hr =  m_pDecoder->StartDecoding();
        }

        if (FAILED(hr))
        {
            SAFE_RELEASE(m_pDecoder);
        }
    }
        
    cbReadLen = (DWORD)(m_cbDataLength - cbStartOffset);

    if (bReverse)
    {
        // Reverse playback: Read from the offset back to zero. 
        
        CHECK_HR(hr = GenerateSamplesLoop(
            hnsSeekTime,
            hnsTestSampleDuration,
            bReverse,
            (DWORD)(m_cbDataLength + m_cbDataOffset - cbStartOffset), //DWORD cbDataOffset
            cbReadLen,              //DWORD cbDataLen
            pSampleInfo,
            FuncPtrToDisplaySampleInfo
            ));

    }
    else
    {
        // Forward playback: Read from the offset to the end.
            
        CHECK_HR(hr = GenerateSamplesLoop(
            hnsSeekTime,
            hnsTestSampleDuration,
            bReverse,
            (DWORD)(m_cbDataOffset + cbStartOffset), //DWORD cbDataOffset, 
            cbReadLen,                              //DWORD cbDataLen
            pSampleInfo,
            FuncPtrToDisplaySampleInfo
            ));
    }

    // Note: cbStartOffset is relative to the start of the data object.
    // GenerateSamplesLoop expects the offset relative to the start of the file.

        
done:
    LOG_MSG_IF_FAILED(L"CASFManager::GenerateSamples failed.\n", hr);
    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: GenerateSamplesLoop
//
//Reads 1024 * 4 byte chunks of media data from a byte stream and 
//parses the ASF Data Object starting at the specified offset.
//Collects 5seconds audio samples and sends to the MFT to decode.
//Gets the first key frame for the video stream and sends to the MFT
//
// hnsSeekTime: Presentation time in hns. 
// hnsTestSampleDuration: Presentation time at which the parsing should end.
// bReverse: Specifies if the splitter configured to parse in reverse.
// cbDataOffset: Offset relative to the start of the data object.
// cbDataLen: Length of data to parse
// pSampleInfo: Pointer to SAMPLE_INFO structure that stores sample 
//          information.
// FuncPtrToDisplaySampleInfo: Callback defined by the caller that 
//          will display the sample information
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::GenerateSamplesLoop(
    const MFTIME& hnsSeekTime, 
    const MFTIME& hnsTestSampleDuration,
    BOOL  bReverse,
    DWORD cbDataOffset, 
    DWORD cbDataLen, 
    SAMPLE_INFO* pSampleInfo,
    void (*FuncPtrToDisplaySampleInfo)(SAMPLE_INFO*)
    )
{
    const DWORD READ_SIZE = 1024 * 4;

    HRESULT hr = S_OK;
    DWORD   cbRead = 0;
    DWORD   dwStatusFlags = 0;
    WORD    wStreamNumber =  0;
    BOOL    fComplete = FALSE;

    IMFSample *pSample = NULL;
    IMFMediaBuffer *pBuffer = NULL;

    MFTIME hnsCurrentSampleTime = 0;

    while (!fComplete && (cbDataLen > 0))
    {
        cbRead = min(READ_SIZE, cbDataLen);

        if (bReverse)
        {
            // Reverse playback: Read data chunks going backward from cbDataOffset.
            CHECK_HR(hr = ReadDataIntoBuffer(m_pByteStream, cbDataOffset - cbRead, cbRead, &pBuffer));
            cbDataOffset -= cbRead;
            cbDataLen -= cbRead;
        }
        else
        {
            // Forward playback: Read data chunks going forward from cbDataOffset.
            CHECK_HR(hr = ReadDataIntoBuffer(m_pByteStream, cbDataOffset, cbRead, &pBuffer));
            cbDataOffset += cbRead;
            cbDataLen -= cbRead;
        }

        // Push data on the splitter
        CHECK_HR(hr =  m_pSplitter->ParseData(pBuffer, 0, 0));

        // Start getting samples from the splitter as long as it returns ASF_STATUSFLAGS_INCOMPLETE
        do 
        {
            CHECK_HR(hr = m_pSplitter->GetNextSample(&dwStatusFlags, &wStreamNumber, &pSample));

            if (pSample)
            {
                // Get sample information
                pSampleInfo->wStreamNumber = wStreamNumber;

                //if decoder is initialized, collect test data
                if (m_pDecoder)
                {
                    if (m_guidCurrentMediaType == MFMediaType_Audio)
                    {
                        // Send audio data to the decoder.
                        (void)SendAudioSampleToDecoder(pSample, hnsTestSampleDuration, bReverse, &fComplete, pSampleInfo, FuncPtrToDisplaySampleInfo);
                    }
                    else if (m_guidCurrentMediaType == MFMediaType_Video)
                    {
                        // Send video data to the decoder.
                        (void)SendKeyFrameToDecoder(pSample, hnsSeekTime, bReverse, &fComplete, pSampleInfo, FuncPtrToDisplaySampleInfo);

                    }

                    if (fComplete)
                    {
                        break;
                    }
                }
            }

            SAFE_RELEASE(pSample);

        } while (dwStatusFlags & ASF_STATUSFLAGS_INCOMPLETE);

        SAFE_RELEASE(pBuffer);
    }

done:
    SAFE_RELEASE(pBuffer);
    SAFE_RELEASE(pSample);
    return hr;
}


/////////////////////////////////////////////////////////////////////
// Name: SendAudioSampleToDecoder
//
// For audio, collect test samples and send it to the decoder
//
// pSample:  Uncompressed sample that needs to be decoded
// hnsTestSampleEndTime: Presenation time at which to stop decoding.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::SendAudioSampleToDecoder(
    IMFSample* pSample,
    const MFTIME& hnsTestSampleEndTime,
    BOOL bReverse,
    BOOL *pbComplete,
    SAMPLE_INFO* pSampleInfo,
    void (*FuncPtrToDisplaySampleInfo)(SAMPLE_INFO*))
{
    if (!pSample)
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = S_OK;
    
    MFTIME hnsCurrentSampleTime = 0;
    BOOL   bShouldDecode = FALSE;

    // Get the time stamp on the sample.
    CHECK_HR(hr = pSample->GetSampleTime(&hnsCurrentSampleTime));

    if (bReverse)
    {
        bShouldDecode = (hnsCurrentSampleTime > hnsTestSampleEndTime);
    }
    else
    {
        bShouldDecode = (hnsCurrentSampleTime < hnsTestSampleEndTime);
    }

    if (bShouldDecode)
    {

        // If the decoder is not streaming, start it.
        if ( m_pDecoder->GetDecoderStatus() != STREAMING)
        {
            CHECK_HR (hr =  m_pDecoder->StartDecoding());
        }

        CHECK_HR (hr =  m_pDecoder->ProcessAudio (pSample));
        
        //Get sample information
        (void)GetSampleInfo(pSample, pSampleInfo);

        //Send it to callback to display
        FuncPtrToDisplaySampleInfo(pSampleInfo);

    }
    else
    {
        //all samples have been decoded. Inform the decoder.
        CHECK_HR (hr =  m_pDecoder->StopDecoding());
    }

    *pbComplete = !bShouldDecode;

done:
    return hr;
}


/////////////////////////////////////////////////////////////////////
// Name: SendKeyFrameToDecoder
//
//For Video, get the key frame closest to the time seeked by the caller
//
// pSample:  Uncompressed sample that needs to be decoded
// hnsTestSampleEndTime: Presenation time at which to stop decoding.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::SendKeyFrameToDecoder(
    IMFSample* pSample,
    const MFTIME& hnsSeekTime,
    BOOL bReverse,
    BOOL* fDecodedKeyFrame,
    SAMPLE_INFO* pSampleInfo,
    void (*FuncPtrToDisplaySampleInfo)(SAMPLE_INFO*))
{
    if (!pSample)
    {
        return E_INVALIDARG;
    }
    
    HRESULT hr = S_OK;
    
    MFTIME hnsCurrentSampleTime =0;

    BOOL   fShouldDecode = FALSE;
    UINT32 fIsKeyFrame = 0;

    IMFSample* pKeyFrameSample = NULL;

    // Get the time stamp on the sample
    CHECK_HR (hr = pSample->GetSampleTime (&hnsCurrentSampleTime));

    if ((UINT64)hnsCurrentSampleTime > m_fileinfo->cbPreroll)
    {
        hnsCurrentSampleTime -= m_fileinfo->cbPreroll;
    }

    // Check if the key-frame attribute is set on the sample
    fIsKeyFrame = MFGetAttributeUINT32(pSample, MFSampleExtension_CleanPoint, FALSE);

    if (!fIsKeyFrame)
    {
        return hr;
    }

    // Should we decode this sample?
    if (bReverse)
    {
        // Reverse playback: 
        // Is the sample *prior* to the seek time, and a key frame?
        fShouldDecode = (hnsCurrentSampleTime <= hnsSeekTime) ;
    }
    else
    {
        // Forward playback:
        // Is the sample *after* the seek time, and a key frame?
        fShouldDecode = (hnsCurrentSampleTime >= hnsSeekTime);
    }

    if (fShouldDecode)
    {
        // We found the key frame closest to the seek time.
        // Start the decoder if not already started.
        if ( m_pDecoder->GetDecoderStatus() != STREAMING)
        {
            CHECK_HR (hr =  m_pDecoder->StartDecoding());
        }

        // Set the discontinity attribute.
        CHECK_HR (hr = pSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE)); 

        //Send the sample to the decoder.
        CHECK_HR (hr =  m_pDecoder->ProcessVideo(pSample));

        *fDecodedKeyFrame = TRUE;
        
        //Get sample information
        (void)GetSampleInfo(pSample, pSampleInfo);
        pSampleInfo->fSeekedKeyFrame = *fDecodedKeyFrame;

        //Send it to callback to display
        FuncPtrToDisplaySampleInfo(pSampleInfo);
            
        CHECK_HR (hr =  m_pDecoder->StopDecoding());
    }


done:
    return hr;
}

/////////////////////////////////////////////////////////////////////
// Name: ReadDataIntoBuffer
//
// Reads data from a byte stream and returns a media buffer that
// contains the data.
//
// pStream:  Pointer to the byte stream
// cbOffset: Offset at which to start reading
// cbToRead: Number of bytes to read
// ppBuffer: Receives a pointer to the buffer.
/////////////////////////////////////////////////////////////////////

HRESULT CASFManager::ReadDataIntoBuffer(
    IMFByteStream *pStream,     // Pointer to the byte stream.
    DWORD cbOffset,             // Offset at which to start reading
    DWORD cbToRead,             // Number of bytes to read
    IMFMediaBuffer **ppBuffer   // Receives a pointer to the buffer.
    )
{
    HRESULT hr = S_OK;
    BYTE *pData = NULL;
    DWORD cbRead = 0;   // Actual amount of data read

    IMFMediaBuffer *pBuffer = NULL;

    // Create the media buffer. This function allocates the memory.
    CHECK_HR(hr = MFCreateMemoryBuffer(cbToRead, &pBuffer));

    // Access the buffer.
    CHECK_HR(hr = pBuffer->Lock(&pData, NULL, NULL));

    //Set the offset
    CHECK_HR(hr = pStream->SetCurrentPosition(cbOffset));

    // Read the data from the byte stream.
    CHECK_HR(hr = pStream->Read(pData, cbToRead, &cbRead));

    CHECK_HR(hr = pBuffer->Unlock());
    pData = NULL;

    // Update the size of the valid data.
    CHECK_HR(hr = pBuffer->SetCurrentLength(cbRead));

    // Return the pointer to the caller.
    *ppBuffer = pBuffer;
    (*ppBuffer)->AddRef();

    TRACE((L"Read data from the ASF file into a media buffer.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::ReadDataIntoBuffer failed.\n", hr);
    
    if (pData)
    {
        pBuffer->Unlock();
    }
    SAFE_RELEASE(pBuffer);
    return hr;
}

//////////////////////////////////////////////////////////////////////////
//  Name: SetFilePropertiesObject
//  Description: Retrieves ASF File Object information through attributes on
//  the presentation descriptor that is generated from Content Info
//
/////////////////////////////////////////////////////////////////////////

HRESULT CASFManager::SetFilePropertiesObject(FILE_PROPERTIES_OBJECT* fileinfo)
{
    if (! m_pContentInfo)
    {
        return MF_E_NOT_INITIALIZED;
    }

    HRESULT hr = S_OK;

    IMFPresentationDescriptor *pPD = NULL;

    UINT32 cbBlobSize = 0;

    CHECK_HR( hr =  m_pContentInfo->GeneratePresentationDescriptor(&pPD));

    //get File ID
    hr = pPD->GetGUID(MF_PD_ASF_FILEPROPERTIES_FILE_ID, &fileinfo->guidFileID);

    // get Creation Time
    hr = pPD->GetBlob(MF_PD_ASF_FILEPROPERTIES_CREATION_TIME, (BYTE *)&fileinfo->ftCreationTime, sizeof(FILETIME), &cbBlobSize);

    //get Data Packets Count
    hr = pPD->GetUINT32(MF_PD_ASF_FILEPROPERTIES_PACKETS, &fileinfo->cbPackets);

    //get Play Duration
    hr = pPD->GetUINT64(MF_PD_ASF_FILEPROPERTIES_PLAY_DURATION, &fileinfo->cbPlayDuration);

    //get presentation duration 
    hr = pPD->GetUINT64(MF_PD_DURATION, &fileinfo->cbPresentationDuration);

    //get Send Duration
    hr = pPD->GetUINT64(MF_PD_ASF_FILEPROPERTIES_SEND_DURATION, &fileinfo->cbSendDuration);

    //get preroll
    UINT64 msecspreroll = 0, hnspreroll =0;
    hr = pPD->GetUINT64(MF_PD_ASF_FILEPROPERTIES_PREROLL, &msecspreroll);
    hnspreroll = msecspreroll*10000;
    fileinfo->cbPreroll = hnspreroll;

    //get Flags
    hr = pPD->GetUINT32(MF_PD_ASF_FILEPROPERTIES_FLAGS, &fileinfo->cbFlags);

    //get Maximum Data Packet Size
    hr = pPD->GetUINT32(MF_PD_ASF_FILEPROPERTIES_MAX_PACKET_SIZE, &fileinfo->cbMaxPacketSize);

    //get Minimum Data Packet Size
    hr = pPD->GetUINT32(MF_PD_ASF_FILEPROPERTIES_MIN_PACKET_SIZE, &fileinfo->cbMinPacketSize);
    
    // get Maximum Bit rate
    hr = pPD->GetUINT32(MF_PD_ASF_FILEPROPERTIES_MAX_BITRATE, &fileinfo->cbMaxBitRate);
    

    this->m_fileinfo = fileinfo;

    TRACE((L"Read File Properties Object from the ASF Header Object.\n"));

done:

    LOG_MSG_IF_FAILED(L"CASFManager::SetFilePropertiesObject failed.\n", hr);

    SAFE_RELEASE(pPD);

    return hr;

}

//////////////////////////////////////////////////////////////////////////
//  Name: GetSampleInfo
//  Description: Retrieves sample information from the sample generated by the splitter
//
//  pSample: Pointer to the sample object
//  pSampleInfo: Pointer to the SAMPLE_INFO structure tha stores the sample information.
//
/////////////////////////////////////////////////////////////////////////

HRESULT CASFManager::GetSampleInfo(IMFSample *pSample, SAMPLE_INFO* pSampleInfo)
{
    if (!pSampleInfo || !pSample)
    {
        return E_INVALIDARG;
    }


    HRESULT hr = S_OK;

    //Number of buffers in the sample
    CHECK_HR(hr = pSample->GetBufferCount(&pSampleInfo->cBufferCount));

    //Total buffer length
    CHECK_HR(hr = pSample->GetTotalLength(&pSampleInfo->cbTotalLength));

    //Sample time   
    hr = pSample->GetSampleTime(&pSampleInfo->hnsSampleTime);

    if (hr == MF_E_NO_SAMPLE_TIMESTAMP)
    {
        hr = S_OK;
    }

done:
    return hr;
}

//////////////////////////////////////////////////////////////////////////
//  Name: Reset
//  Description: Releases the existing ASF objects for the current file
//
/////////////////////////////////////////////////////////////////////////

void CASFManager::Reset()
{
    SAFE_RELEASE( m_pContentInfo);
    SAFE_RELEASE( m_pDataBuffer);
    SAFE_RELEASE( m_pIndexer);
    SAFE_RELEASE( m_pSplitter);

    // TEST
    SAFE_RELEASE(m_pByteStream);
    m_cbDataOffset = 0;
    m_cbDataLength = 0;

    if (m_pDecoder)
    {
        m_pDecoder->Reset();
        SAFE_RELEASE (m_pDecoder);
    }

    if( m_fileinfo)
    {
        m_fileinfo = NULL;
    }

    TRACE((L"CASFManager reset.\n"));

}