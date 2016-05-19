//////////////////////////////////////////////////////////////////////////
//
// RiffParser.h : RIFF file parsing.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <mmsystem.h>

#pragma warning(disable: 4201)

#include "aviriff.h" // Holds useful AVI structs


const UINT32 RIFF_CHUNK_SIZE = sizeof(RIFFCHUNK);
const UINT32 RIFF_LIST_SIZE = sizeof(RIFFLIST);



// mmsystem.h defines the following constants:
// FOURCC_LIST
// FOURCC_RIFF


//////////////////////////////////////////////////////////////////////////
//  CRiffChunk
//  Description: Wrapper for RIFFCHUNK. 
//
//  The RIFFCHUNK structure is the chunk header.
//////////////////////////////////////////////////////////////////////////

struct CRiffChunk : public RIFFCHUNK
{
    CRiffChunk();
    CRiffChunk(const RIFFCHUNK& c);

    FOURCC  FourCC() const { return fcc; }
    DWORD   DataSize() const { return cb; }
    BOOL    IsList() const { return fcc == FOURCC_LIST; }
};



//////////////////////////////////////////////////////////////////////////
//  CRiffParser
//  Description: Parses RIFF files.
//
// NOTES:
// This object is initialized to point to a RIFF "container." The 
// container is either a RIFF file or a RIFF list. The parser can walk
// through all of the chunks in the container. 
// 
// If a chunk is a LIST, call EnumerateChunksInList to get a parser for
// that list.
//
//////////////////////////////////////////////////////////////////////////

class CRiffParser
{
public:
    virtual ~CRiffParser();

    HRESULT MoveToNextChunk( void );

    FOURCC  RiffID() const { return m_fccID; }
    FOURCC  RiffType() const { return m_fccType; }

    const CRiffChunk& Chunk() const { return m_chunk; }

    HRESULT MoveToStartOfChunk();
    HRESULT MoveToChunkOffset(DWORD dwOffset);
    HRESULT ReadDataFromChunk(BYTE* pData, DWORD dwLengthInBytes);
    DWORD   BytesRemainingInChunk() const { return m_dwBytesRemaining; }
        

protected:

    CRiffParser(IMFByteStream *pStream, FOURCC id, LONGLONG cbStartOfContainer, HRESULT& hr);

private:

    HRESULT     ReadRiffHeader();
    HRESULT     ReadChunkHeader();

    LONGLONG    ChunkActualSize() const { return sizeof(RIFFCHUNK) + RIFFROUND(m_chunk.cb); }

    IMFByteStream   *m_pStream;

    FOURCC      m_fccID;        // FOURCC of the current container ('RIFF' or 'LIST').
    FOURCC      m_fccType;      // FOURCC of the RIFF file type or LIST type.

    LONGLONG    m_llContainerOffset;    // Start of the container, as an offset into the stream.
    DWORD       m_dwContainerSize;      // Size of the container including the RIFF header.
    LONGLONG    m_llCurrentChunkOffset; // Start of the current RIFF chunk, as an offset into the stream.
        
    CRiffChunk  m_chunk;                // Current RIFF chunk.

    DWORD       m_dwBytesRemaining;     // How many bytes are left in this chunk?
};




//////////////////////////////////////////////////////////////////////////
//  CWavRiffParser
//  Description: Parses the RIFF file structure.
//////////////////////////////////////////////////////////////////////////

class CWavRiffParser : public CRiffParser
{
public:
    static HRESULT      Create(IMFByteStream *pStream, CWavRiffParser **ppParser);

    ~CWavRiffParser();

    HRESULT             ParseWAVEHeader();

    const WAVEFORMATEX* Format() const { return m_pWaveFormat; }
    DWORD               FormatSize() const { return m_cbWaveFormat; }

    MFTIME              FileDuration() const { return m_rtDuration; } 

private:
    
    CWavRiffParser(IMFByteStream *pStream, HRESULT& hr);

    HRESULT             ReadFormatBlock();

    WAVEFORMATEX        *m_pWaveFormat;
    DWORD               m_cbWaveFormat;

    MFTIME              m_rtDuration;               // File duration.

};




