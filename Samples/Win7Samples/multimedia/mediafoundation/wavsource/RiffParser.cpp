//////////////////////////////////////////////////////////////////////////
//
// RiffParser.cpp : RIFF file parsing.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////


#include "WavSource.h"
#include "RiffParser.h"

// FOURCCs that we need
const FOURCC ckid_WAVE_FILE = FCC('WAVE');    // RIFF type for .wav files
const FOURCC ckid_WAVE_FMT =  FCC('fmt ');    // WAVEFORMATEX chunk
const FOURCC ckid_WAVE_DATA = FCC('data');    // Audio data chunk

CRiffChunk::CRiffChunk()
{
    this->fcc = 0;
    this->cb = 0;
}

CRiffChunk::CRiffChunk(const RIFFCHUNK& c)
{
    this->fcc = c.fcc;
    this->cb = c.cb;
}

//-------------------------------------------------------------------
// CRiffParser constructor
//
// pStream: Stream to read from RIFF file
// id:  FOURCC of the RIFF container. Should be 'RIFF' or 'LIST'.
// cbStartofContainer:  Start of the container, as an offset into the stream.
// hr:  Receives the success or failure of the constructor
//-------------------------------------------------------------------

CRiffParser::CRiffParser(IMFByteStream *pStream, FOURCC id, LONGLONG cbStartOfContainer, HRESULT& hr) :
    m_fccID(id), 
    m_fccType(0),
    m_llContainerOffset(cbStartOfContainer), 
    m_dwContainerSize(0), 
    m_llCurrentChunkOffset(0),
    m_dwBytesRemaining(0)
{
    if (pStream == NULL)
    {
        hr = E_POINTER;
    }
    else
    {
        m_pStream = pStream;
        m_pStream->AddRef();

        hr = ReadRiffHeader();
    }
}

CRiffParser::~CRiffParser()
{
    SafeRelease(&m_pStream);
}


//-------------------------------------------------------------------
// Name: ReadRiffHeader
// Description: 
// Read the container header section. (The 'RIFF' or 'LIST' header.)
//
// This method verifies the header is well-formed and caches the
// container's FOURCC type.
//-------------------------------------------------------------------

HRESULT CRiffParser::ReadRiffHeader()
{
    // Riff chunks must be WORD aligned
    if ((m_llContainerOffset % 2) != 0)
    {
        return E_INVALIDARG;
    }

    // Offset must be positive.
    if (m_llContainerOffset < 0)
    {
        return E_INVALIDARG;
    }

    // Offset + the size of header must not overflow.
    if (MAXLONGLONG - m_llContainerOffset <= sizeof(RIFFLIST))
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    RIFFLIST header = { 0 };
    ULONG cbRead = 0;

    // Seek to the start of the container.
    hr = m_pStream->SetCurrentPosition(m_llContainerOffset);
    
    // Read the header. 
    if (SUCCEEDED(hr))
    {   
        hr = m_pStream->Read((BYTE*)&header, sizeof(header), &cbRead);
    }

    // Make sure we read the number of bytes we expected.
    if (SUCCEEDED(hr))
    {   
        if (cbRead != sizeof(header))
        {
            hr = E_INVALIDARG;
        }
    }

    // Make sure the header ID matches what the caller expected.
    if (SUCCEEDED(hr))
    {   
        if (header.fcc != m_fccID)
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {   
        // The size given in the RIFF header does not include the 8-byte header.
        // However, our m_llContainerOffset is the offset from the start of the
        // header. Therefore our container size = listed size + size of header.

        m_dwContainerSize = header.cb + sizeof(RIFFCHUNK); 
        m_fccType = header.fccListType;

        // Start of the first chunk = start of container + size of container header
        m_llCurrentChunkOffset = m_llContainerOffset + sizeof(RIFFLIST);

        hr = ReadChunkHeader();
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: ReadChunkHeader
// Description: 
// Reads the chunk header. Caller must ensure that the current file 
// pointer is located at the start of the chunk header.
//-------------------------------------------------------------------

HRESULT CRiffParser::ReadChunkHeader()
{
    // Offset + the size of header must not overflow.
    if (MAXLONGLONG - m_llCurrentChunkOffset <= sizeof(RIFFCHUNK))
    {
        return E_INVALIDARG;
    }

    ULONG cbRead;
    HRESULT hr = S_OK;
    
    hr = m_pStream->Read((BYTE*)&m_chunk, sizeof(RIFFCHUNK), &cbRead); 
    
    // Make sure we got the number of bytes we expected.

    if (SUCCEEDED(hr))
    {
        if (cbRead != sizeof(RIFFCHUNK))
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_dwBytesRemaining = m_chunk.DataSize();
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: MoveToNextChunk
// Description: 
// Advance to the start of the next chunk and read the chunk header. 
//-------------------------------------------------------------------

HRESULT CRiffParser::MoveToNextChunk()
{
    // chunk offset is always bigger than container offset,
    // and both are always non-negative.
    assert(m_llCurrentChunkOffset > m_llContainerOffset);
    assert(m_llCurrentChunkOffset >= 0);
    assert(m_llContainerOffset >= 0);

    HRESULT hr = S_OK;
    LONGLONG maxChunkSize = 0;

    // Update current chunk offset to the start of the next chunk
    m_llCurrentChunkOffset = m_llCurrentChunkOffset + ChunkActualSize();

    // Are we at the end?
    if ((m_llCurrentChunkOffset - m_llContainerOffset) >= m_dwContainerSize)
    {
        return E_FAIL;
    }

    // Current chunk offset + size of current chunk
    if (MAXLONGLONG - m_llCurrentChunkOffset <= ChunkActualSize())
    {
        return E_INVALIDARG;
    }
    
    // Seek to the start of the chunk.
    hr = m_pStream->SetCurrentPosition(m_llCurrentChunkOffset);

    // Read the header. 
    if (SUCCEEDED(hr))
    {
        hr = ReadChunkHeader();
    }
 
    // This chunk cannot be any larger than (container size - (chunk offset - container offset) )
    if (SUCCEEDED(hr))
    {
        maxChunkSize = (LONGLONG)m_dwContainerSize - (m_llCurrentChunkOffset - m_llContainerOffset);

        if (maxChunkSize < ChunkActualSize())
        {
            hr = E_INVALIDARG;
        }
    }

    if (SUCCEEDED(hr))
    {
        m_dwBytesRemaining = m_chunk.DataSize();
    }

    return hr;
}

//-------------------------------------------------------------------
// Name: MoveToChunkOffset
// Description: 
// Move the file pointer to a byte offset from the start of the 
// current chunk. 
//-------------------------------------------------------------------

HRESULT CRiffParser::MoveToChunkOffset(DWORD dwOffset)
{
    if (dwOffset > m_chunk.DataSize())
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    
    hr = m_pStream->SetCurrentPosition(m_llCurrentChunkOffset + dwOffset + sizeof(RIFFCHUNK));

    if (SUCCEEDED(hr))
    {
        m_dwBytesRemaining = m_chunk.DataSize() - dwOffset;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: MoveToChunkOffset
// Description: 
// Move the file pointer to the start of the current chunk. 
//-------------------------------------------------------------------

HRESULT CRiffParser::MoveToStartOfChunk()
{
    return MoveToChunkOffset(0);
}


//-------------------------------------------------------------------
// Name: ReadDataFromChunk
// Description: 
// Read data from the current chunk. (Starts at the current file ptr.) 
//-------------------------------------------------------------------

HRESULT CRiffParser::ReadDataFromChunk( BYTE* pData, DWORD dwLengthInBytes )
{
    if (dwLengthInBytes > m_dwBytesRemaining)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;
    ULONG cbRead = 0;

    hr = m_pStream->Read(pData, dwLengthInBytes, &cbRead);

    if (SUCCEEDED(hr))
    {
        m_dwBytesRemaining -= cbRead;
    }

    return hr;
}




/////////////////

// CWavRiffParser is a specialization of the generic RIFF parser object,
// and is designed to parse .wav files.

CWavRiffParser::CWavRiffParser(IMFByteStream *pStream, HRESULT& hr) :
    m_pWaveFormat(NULL), m_cbWaveFormat(0), m_rtDuration(0),
    CRiffParser(pStream, FOURCC_RIFF, 0, hr)
{

}

CWavRiffParser::~CWavRiffParser()
{
    CoTaskMemFree(m_pWaveFormat);
}


//-------------------------------------------------------------------
// Name: Create
// Description: Static creation function.
//-------------------------------------------------------------------

HRESULT CWavRiffParser::Create(IMFByteStream *pStream, CWavRiffParser **ppParser)
{
    if (ppParser == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    // Create a riff parser for the 'RIFF' container
    CWavRiffParser *pParser = new (std::nothrow) CWavRiffParser(pStream, hr);

    if (pParser == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Check the RIFF file type.
    if (pParser->RiffType() != ckid_WAVE_FILE)
    {
        hr = MF_E_INVALID_FILE_FORMAT;
    }

    if (SUCCEEDED(hr))
    {   
        *ppParser = pParser;
    }
    else
    {
        delete pParser;
    }

    return hr;
}


//-------------------------------------------------------------------
// Name: ParseWAVEHeader
// Description: Parsers the RIFF WAVE header.
// 
// Note:
// .wav files should look like this:
//
// RIFF ('WAVE'
//       'fmt ' = WAVEFORMATEX structure
//       'data' = audio data
//       )
//-------------------------------------------------------------------

HRESULT CWavRiffParser::ParseWAVEHeader()
{
    HRESULT hr = S_OK;
    BOOL bFoundData = FALSE;

    // Iterate through the RIFF chunks. Ignore chunks we don't recognize.
    while (SUCCEEDED(hr))
    {
        if (Chunk().FourCC() == ckid_WAVE_FMT)
        {
            // Read the WAVEFORMATEX structure allegedly contained in this chunk.
            // This method does NOT validate the contents of the structure.
            hr = ReadFormatBlock();
        }
        else if (Chunk().FourCC() == ckid_WAVE_DATA)
        {
            // Found the start of the audio data. The format chunk should precede the
            // data chunk. If we did not find the formt chunk yet, that is a failure 
            // case (see below)
            bFoundData = TRUE;
            break;
        }

        if (SUCCEEDED(hr))
        {
            hr = MoveToNextChunk();
        }
    }

    // To be valid, the file must have a format chunk and a data chunk.
    // Fail if either of these conditions is not met.
    if (SUCCEEDED(hr))
    {   
        if (m_pWaveFormat == NULL || !bFoundData)
        {
            hr = MF_E_INVALID_FILE_FORMAT;
        }
    }

    if (SUCCEEDED(hr))
    {   
        m_rtDuration = AudioDurationFromBufferSize(m_pWaveFormat, Chunk().DataSize());
    }
    
    return hr;
}

//-------------------------------------------------------------------
// Name: ReadFormatBlock
// Description: Reads the WAVEFORMATEX structure from the file header.
//-------------------------------------------------------------------

HRESULT CWavRiffParser::ReadFormatBlock()
{
    assert(Chunk().FourCC() == ckid_WAVE_FMT);
    assert(m_pWaveFormat == NULL);

    HRESULT hr = S_OK;

    // Some .wav files do not include the cbSize field of the WAVEFORMATEX 
    // structure. For uncompressed PCM audio, field is always zero. 
    const DWORD cbMinFormatSize = sizeof(WAVEFORMATEX) - sizeof(WORD);

    DWORD cbFormatSize = 0;     // Size of the actual format block in the file.

    // Validate the size
    if (Chunk().DataSize() < cbMinFormatSize)
    {
        return MF_E_INVALID_FILE_FORMAT;
    }

    // Allocate a buffer for the WAVEFORMAT structure.
    cbFormatSize = Chunk().DataSize();
    
    // We store a WAVEFORMATEX structure, so our format block must be at 
    // least sizeof(WAVEFORMATEX) even if the format block in the file
    // is smaller. See note above about cbMinFormatSize.
    m_cbWaveFormat = max(cbFormatSize, sizeof(WAVEFORMATEX));

    m_pWaveFormat = (WAVEFORMATEX*)CoTaskMemAlloc(m_cbWaveFormat);
    if (m_pWaveFormat == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Zero our structure, in case cbFormatSize < m_cbWaveFormat.
    ZeroMemory(m_pWaveFormat, m_cbWaveFormat);

    // Now read cbFormatSize bytes from the file.
    hr = ReadDataFromChunk((BYTE*)m_pWaveFormat, cbFormatSize);

    if (FAILED(hr))
    {
        CoTaskMemFree(m_pWaveFormat);
        m_pWaveFormat = NULL;
        m_cbWaveFormat = 0;
    }
    return hr;
}

