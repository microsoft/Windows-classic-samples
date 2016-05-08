//////////////////////////////////////////////////////////////////////////
//
// Parse.cpp
// MPEG-1 parsing code.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "MPEG1Source.h"
#include "Parse.h"

// HAS_FLAG: Test if 'b' contains a specified bit flag
#define HAS_FLAG(b, flag) (((b) & (flag)) == (flag))


// MAKE_WORD: Convert two bytes into a WORD
inline WORD MAKE_WORD(BYTE b1, BYTE b2)
{
    return ((b1 << 8) | b2);
}

// MAKE_DWORD_HOSTORDER:
// Convert the first 4 bytes of an array into a DWORD in host order (ie, no byte swapping).
inline DWORD MAKE_DWORD_HOSTORDER(const BYTE *pData)
{
    return ((DWORD*)pData)[0];
}

// MAKE_DWORD:
// Convert the first 4 bytes of an array into a DWORD in MPEG-1 stream byte order.
inline DWORD MAKE_DWORD(const BYTE *pData)
{
    return htonl( MAKE_DWORD_HOSTORDER(pData) );
}


//-------------------------------------------------------------------
// AdvanceBufferPointer
// Advances a byte array pointer. 
// 
// pData: The array pointer. 
// cbBufferSize: The array size. On output, the size remaining.
// cbAdvance: Number of bytes to advance the pointer.
//
// This function is just a helper for keeping a valid pointer as we
// walk through a buffer. 
//-------------------------------------------------------------------

inline HRESULT AdvanceBufferPointer(const BYTE* &pData, DWORD &cbBufferSize, DWORD cbAdvance)
{
    assert(cbBufferSize >= cbAdvance);
    if (cbBufferSize < cbAdvance)
    {
        return E_FAIL;
    }
    cbBufferSize -= cbAdvance;
    pData += cbAdvance;
    return S_OK;
}

// ValidateBufferSize:
// Fails if cbBufferSize < cbMinRequiredSize.
inline HRESULT ValidateBufferSize(DWORD cbBufferSize, DWORD cbMinRequiredSize)
{
    return (cbBufferSize >= cbMinRequiredSize? S_OK : E_FAIL);
}

// Forward declarations.
HRESULT ParseStreamData(const BYTE *pData, MPEG1StreamHeader& header);
HRESULT ParseStreamId(BYTE id, StreamType *pType, BYTE *pStreamNum);
HRESULT ParsePTS(const BYTE *pData, LONGLONG *pPTS);

HRESULT GetFrameRate(BYTE frameRateCode, MFRatio *pRatio);
HRESULT GetPixelAspectRatio(BYTE pixelAspectCode, MFRatio *pRatio);

HRESULT GetAudioBitRate(MPEG1AudioLayer layer, BYTE index, DWORD *pdwBitRate);
HRESULT GetSamplingFrequency(BYTE code, DWORD *pdwSamplesPerSec);


//-------------------------------------------------------------------
// Buffer class
//-------------------------------------------------------------------


Buffer::Buffer() : m_begin(0), m_end(0)
{
}


//-------------------------------------------------------------------
// Initalize
// Sets the initial buffer size.
//-------------------------------------------------------------------

HRESULT Buffer::Initalize(DWORD cbSize)
{
    HRESULT hr = SetSize(cbSize);
    if (SUCCEEDED(hr))
    {
        ZeroMemory(Ptr(), cbSize);
    }
    return hr;
}


//-------------------------------------------------------------------
// DataPtr
// Returns a pointer to the start of the buffer.
//-------------------------------------------------------------------

BYTE* Buffer::DataPtr()
{
    return Ptr() + m_begin;
}


//-------------------------------------------------------------------
// DataSize
// Returns the size of the buffer.
//
// Note: The "size" is determined by the start and end pointers.
// The memory allocated for the buffer can be larger.
//-------------------------------------------------------------------

DWORD Buffer::DataSize() const
{
    assert(m_end >= m_begin);

    return m_end - m_begin;
}


//-------------------------------------------------------------------
// Reserve
// Reserves additional bytes of memory for the buffer.
//
// This method does *not* increase the value returned by DataSize().
//
// After this method returns, the value of DataPtr() might change,
// so do not cache the old value.
//-------------------------------------------------------------------

HRESULT Buffer::Reserve(DWORD cb)
{
    if (cb > MAXDWORD - DataSize())
    {
        return E_INVALIDARG; // Overflow
    }

    HRESULT hr = S_OK;

    // If this would push the end position past the end of the array, 
    // then we need to copy up the data to start of the array. We might 
    // also need to realloc the array.

    if (cb > GetCount() - m_end)
    {
        // New end position would be past the end of the array.
        // Check if we need to grow the array.

        if (cb > CurrentFreeSize())
        {
            // Array needs to grow
            CHECK_HR(hr = SetSize(DataSize() + cb));
        }
        
        MoveMemory(Ptr(), DataPtr(), DataSize());

        // Reset begin and end. 
        m_end = DataSize(); // Update m_end first before resetting m_begin!
        m_begin = 0;
    }

    assert(CurrentFreeSize() >= cb);

done:
    return hr;
}


//-------------------------------------------------------------------
// MoveStart
// Moves the start of the buffer by cb bytes.
//
// Call this method after consuming data from the buffer.
//-------------------------------------------------------------------

HRESULT Buffer::MoveStart(DWORD cb)
{
    // Cannot advance pass the end of the buffer.
    if (cb > DataSize())
    {
        return E_INVALIDARG;
    }

    m_begin += cb;
    return S_OK;
}


//-------------------------------------------------------------------
// MoveEnd
// Moves end position of the buffer.
//-------------------------------------------------------------------

HRESULT Buffer::MoveEnd(DWORD cb)
{
    HRESULT hr = S_OK;

    hr = Reserve(cb);

    if (SUCCEEDED(hr))
    {
        m_end += cb;
    }
    return hr;
}


//-------------------------------------------------------------------
// CurrentFreeSize (private)
//
// Returns the size of the array minus the size of the data.
//-------------------------------------------------------------------

DWORD Buffer::CurrentFreeSize() const
{
    assert(GetCount() >= DataSize());
    return GetCount() - DataSize();
}


//-------------------------------------------------------------------
// Parser class
//-------------------------------------------------------------------


Parser::Parser() : m_SCR(0), m_muxRate(0), m_pHeader(NULL), m_bHasPacketHeader(FALSE), m_bEOS(FALSE)
{
    ZeroMemory(&m_curPacketHeader, sizeof(m_curPacketHeader));
}

Parser::~Parser()
{
    CoTaskMemFree(m_pHeader);
}

//-------------------------------------------------------------------
// GetSystemHeader class
//
// Returns a copy of the system header. 
// Do not call this method unless HasSystemHeader() returns TRUE.
// 
// The caller must free the returned structure by calling 
// CoTaskMemFree.
//-------------------------------------------------------------------

HRESULT Parser::GetSystemHeader(MPEG1SystemHeader **ppHeader)
{
    if (ppHeader == NULL)
    {
        return E_POINTER;
    }

    if (!HasSystemHeader())
    {
        return E_FAIL;
    }

    assert(m_pHeader->cbSize > 0);

    MPEG1SystemHeader *pHeader = (MPEG1SystemHeader*)CoTaskMemAlloc(m_pHeader->cbSize);
    if (pHeader == NULL)
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(pHeader, m_pHeader, m_pHeader->cbSize);
    
    *ppHeader = pHeader;
    return S_OK;
}


//-------------------------------------------------------------------
// ParseBytes
// Parses as much data as possible from the pData buffer, and returns
// the amount of data parsed in pAte (*pAte <= cbLen).
// 
// Return values:
//      S_OK: The method consumed some data (*pAte > 0).
//      S_FALSE: The method did not consume any data (*pAte == 0). 
//      [or an error code]
//
// If the method returns S_FALSE, the caller must allocate a larger
// buffer and pass in more data.
//-------------------------------------------------------------------

HRESULT Parser::ParseBytes(const BYTE *pData, DWORD cbLen, DWORD *pAte)
{
    *pAte = 0;

    if (cbLen < 4)
    {
        return S_FALSE;
    }

    HRESULT hr = S_OK;
    DWORD cbLengthToStartCode = 0;  // How much we skip to reach the next start code.
    DWORD cbParsed = 0;             // How much we parse after the start code.

    m_bHasPacketHeader = FALSE;

    hr = FindNextStartCode(pData, cbLen, &cbLengthToStartCode);

    if (hr == S_OK)
    {
        cbLen -= cbLengthToStartCode;
        pData += cbLengthToStartCode;

        switch (MAKE_DWORD(pData))
        {
        case MPEG1_PACK_START_CODE:
            // Start of pack.
            hr = ParsePackHeader(pData, cbLen, &cbParsed);
            break;

        case MPEG1_SYSTEM_HEADER_CODE:
            // Start of system header.
            hr = ParseSystemHeader(pData, cbLen, &cbParsed);
            break;

        case MPEG1_STOP_CODE:
            // Stop code, end of stream.
            cbParsed = sizeof(DWORD);
            hr = OnEndOfStream();
            break;

        default:
            // Start of packet.
            hr = ParsePacketHeader(pData, cbLen, &cbParsed);
            break;
            
        }
    }

    if (hr == S_OK)
    {
        *pAte = cbLengthToStartCode + cbParsed;
    }
    return hr;
};


//-------------------------------------------------------------------
// FindNextStartCode
// Looks for the next start code in the buffer.
//
// pData: Pointer to the buffer.
// cbLen: Size of the buffer.
// pAte: Receives the number of bytes *before* the start code.
//
// If no start code is found, the method returns S_FALSE.
//-------------------------------------------------------------------

HRESULT Parser::FindNextStartCode(const BYTE *pData, DWORD cbLen, DWORD *pAte)
{
    HRESULT hr = S_FALSE;

    DWORD cbLeft = cbLen;

    while (cbLeft > 4)
    {
        if ( (MAKE_DWORD_HOSTORDER(pData) & 0x00FFFFFF) == 0x00010000 )
        {
            hr = S_OK;
            break;
        }

        cbLeft -= 4;
        pData += 4;
    }
    *pAte = (cbLen - cbLeft);
    return hr;
}


//-------------------------------------------------------------------
// ParsePackHeader
// Parses the start of an MPEG-1 pack.
//-------------------------------------------------------------------

HRESULT Parser::ParsePackHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte)
{
    assert( MAKE_DWORD(pData) == MPEG1_PACK_START_CODE );

    if (cbLen < MPEG1_PACK_HEADER_SIZE)
    {
        return S_FALSE; // Not enough data yet.
    }

    // Check marker bits
    if ( ((pData[4] & 0xF1) != 0x21) ||
         ((pData[6] & 0x01) != 0x01) ||
         ((pData[8] & 0x01) != 0x01) ||
         ((pData[9] & 0x80) != 0x80) ||
         ((pData[11] & 0x01) != 0x01) )
    {
        return E_FAIL;
    }


    // Calculate the SCR.
    LONGLONG scr = ( (pData[8] & 0xFE) >> 1) | 
                   ( (pData[7]) << 7) | 
                   ( (pData[6] & 0xFE) << 14) | 
                   ( (pData[5]) << 22) | 
                   ( (pData[4] & 0x0E) << 29);

    DWORD muxRate = ( (pData[11] & 0xFE) >> 1) |
                    ( (pData[10]) << 7) |
                    ( (pData[9] & 0x7F) << 15);


    m_SCR = scr;
    m_muxRate = muxRate;

    *pAte = MPEG1_PACK_HEADER_SIZE;

    return S_OK;
}


//-------------------------------------------------------------------
// ParseSystemHeader.
// Parses the MPEG-1 system header.
// 
// NOTES: 
// The system header optionally appears after the pack header.
// The first pack must contain a system header. 
// Subsequent packs may contain a system header.
//-------------------------------------------------------------------

HRESULT Parser::ParseSystemHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte)
{
    assert( MAKE_DWORD(pData) == MPEG1_SYSTEM_HEADER_CODE );

    if (cbLen < MPEG1_SYSTEM_HEADER_MIN_SIZE)
    {
        return S_FALSE; // Not enough data yet.
    }

    // Find the total header length. 
    DWORD cbHeaderLen = MPEG1_SYSTEM_HEADER_PREFIX +  MAKE_WORD(pData[4], pData[5]);
    
    if (cbHeaderLen < MPEG1_SYSTEM_HEADER_MIN_SIZE - MPEG1_SYSTEM_HEADER_PREFIX)
    {
        return E_FAIL;  // Invalid value.
    }

    if (cbLen < cbHeaderLen)
    {
        return S_FALSE; // Not enough data yet.
    }

    // We have enough data to parse the header. 

    HRESULT hr = S_OK;

    // Did we already see a system header?
    if (!HasSystemHeader())
    {
        // This is the first time we've seen the header. Parse it.

        // Calculate the number of stream info's in the header.
        DWORD cStreamInfo = (cbHeaderLen - MPEG1_SYSTEM_HEADER_MIN_SIZE) / MPEG1_SYSTEM_HEADER_STREAM;

        // Calculate the structure size.
        DWORD cbSize = sizeof(MPEG1SystemHeader);
        if (cStreamInfo > 1)
        {
            cbSize += sizeof(MPEG1StreamHeader) * (cStreamInfo - 1);
        }

        // Allocate room for the header.
        m_pHeader = (MPEG1SystemHeader*)CoTaskMemAlloc(cbSize);
        if (m_pHeader == NULL)
        {
            CHECK_HR(hr = E_OUTOFMEMORY);
        }

        m_pHeader->cbSize = cbSize;

        // Check marker bits
        if ( ((pData[6] & 0x80) != 0x80) ||
             ((pData[8] & 0x01) != 0x01) ||
             ((pData[10] & 0x20) != 0x20) ||
             (pData[11] != 0xFF) )
        {
            CHECK_HR(hr = E_FAIL);  // Invalid bits.
        }

        m_pHeader->rateBound = ((pData[6] & 0x7F) << 16) | (pData[7] << 8) | (pData[8] >> 1);
        m_pHeader->cAudioBound = pData[9] >> 2;
        m_pHeader->bFixed = HAS_FLAG(pData[9], 0x02);
        m_pHeader->bCSPS = HAS_FLAG(pData[9], 0x01);
        m_pHeader->bAudioLock = HAS_FLAG(pData[10], 0x80);
        m_pHeader->bVideoLock = HAS_FLAG(pData[10], 0x40);
        m_pHeader->cVideoBound = pData[10] & 0x1F;
        m_pHeader->cStreams = cStreamInfo;

        // Parse the stream information.
        const BYTE *pStreamInfo = pData + MPEG1_SYSTEM_HEADER_MIN_SIZE;

        for (DWORD i = 0; i < cStreamInfo; i++)
        {
            CHECK_HR(hr = ParseStreamData(pStreamInfo, m_pHeader->streams[i]));

            pStreamInfo += MPEG1_SYSTEM_HEADER_STREAM;
        }
    }

    *pAte = cbHeaderLen;

done:

    if (FAILED(hr))
    {
        CoTaskMemFree(m_pHeader);
        m_pHeader = NULL;
    }

    return hr;
}

//-------------------------------------------------------------------
// ParsePacketHeader
//
// Parses the packet header. 
//
// If the method returns S_OK, then HasPacket() returns TRUE and the 
// caller can start parsing the packet.
//-------------------------------------------------------------------

HRESULT Parser::ParsePacketHeader(const BYTE *pData, DWORD cbLen, DWORD *pAte)
{
    if (!HasSystemHeader())
    {
        return E_FAIL; // We should not get a packet before the first system header.
    }

    if (cbLen < MPEG1_PACKET_HEADER_MIN_SIZE)
    {
        return S_FALSE; // Not enough data yet.
    }

    // Before we parse anything else in the packet header, look for the header length.
    DWORD cbPacketLen = MAKE_WORD(pData[4], pData[5]) + MPEG1_PACKET_HEADER_MIN_SIZE;

    // We want enough data for the maximum packet header OR the total packet size, whichever is less.
    if (cbLen < cbPacketLen && cbLen < MPEG1_PACKET_HEADER_MAX_SIZE)
    {
        return S_FALSE; // Not enough data yet.
    }

    // Make sure the start code is 0x000001xx 
    if ((MAKE_DWORD(pData) & 0xFFFFFF00) != MPEG1_START_CODE_PREFIX)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;
    BYTE id = 0;
    StreamType type = StreamType_Unknown;
    BYTE num = 0;
    BOOL bHasPTS = FALSE;

    ZeroMemory(&m_curPacketHeader, sizeof(m_curPacketHeader));

    // Find the stream ID.
    id = pData[3];
    CHECK_HR(hr = ParseStreamId(id, &type, &num));

    DWORD cbLeft = cbPacketLen - MPEG1_PACKET_HEADER_MIN_SIZE;
    pData = pData + MPEG1_PACKET_HEADER_MIN_SIZE;
    DWORD cbPadding = 0;
    LONGLONG pts = 0;

    // Go past the stuffing bytes.
    while ((cbLeft > 0) && (*pData == 0xFF))
    {
        AdvanceBufferPointer(pData, cbLeft, 1);
        ++cbPadding;
    }

    // Check for invalid number of stuffing bytes.
    if (cbPadding > MPEG1_PACKET_HEADER_MAX_STUFFING_BYTE)
    {
        CHECK_HR(hr = E_FAIL);
    }

    // The next bits are:
    // (optional) STD buffer size (2 bytes)
    // union
    // {
    //      PTS (5 bytes)
    //      PTS + DTS (10 bytes)
    //      '0000 1111' (1 bytes)
    // }

    CHECK_HR(hr = ValidateBufferSize(cbLeft, 1));

    if ((*pData & 0xC0) == 0x40)
    {
        // Skip STD buffer size.
        CHECK_HR(hr = AdvanceBufferPointer(pData, cbLeft, 2));
    }
    
    CHECK_HR(ValidateBufferSize(cbLeft, 1));

    if ((*pData & 0xF1) == 0x21)
    {
        // PTS
        CHECK_HR(ValidateBufferSize(cbLeft, 5));

        ParsePTS(pData, &pts);
        bHasPTS = TRUE;

        CHECK_HR(hr = AdvanceBufferPointer(pData, cbLeft, 5));
    }
    else if ((*pData & 0xF1) == 0x31)
    {
        // PTS + DTS
        CHECK_HR(ValidateBufferSize(cbLeft, 10));

        // Parse PTS but skip DTS.
        ParsePTS(pData, &pts);
        bHasPTS = TRUE;

        CHECK_HR(hr = AdvanceBufferPointer(pData, cbLeft, 10));

    }
    else if ((*pData) == 0x0F)
    {
        CHECK_HR(hr = AdvanceBufferPointer(pData, cbLeft, 1));
    }
    else
    {
        CHECK_HR(hr = E_FAIL); // Unexpected bit field
    }

    m_curPacketHeader.stream_id = id;
    m_curPacketHeader.type = type;
    m_curPacketHeader.number = num;
    m_curPacketHeader.cbPacketSize = cbPacketLen;
    m_curPacketHeader.cbPayload = cbLeft;
    m_curPacketHeader.bHasPTS = bHasPTS;
    m_curPacketHeader.PTS = pts;

    // Client can read the packet now.
    m_bHasPacketHeader = TRUE;

    *pAte = cbPacketLen - cbLeft;

done:
    return hr;
}

//-------------------------------------------------------------------
// OnEndOfStream
// Called when the parser reaches the MPEG-1 stop code.
//
// Note: Obviously the parser is not guaranteed to see a stop code 
// before the client reaches the end of the source data. The client 
// must be prepared to handle that case.
//-------------------------------------------------------------------

HRESULT Parser::OnEndOfStream()
{
    m_bEOS = TRUE;
    ClearPacket();

    return S_OK;
}


//-------------------------------------------------------------------
// Static functions 
//-------------------------------------------------------------------


//-------------------------------------------------------------------
// ParsePTS
// Parse the 33-bit Presentation Time Stamp (PTS)
//-------------------------------------------------------------------

HRESULT ParsePTS(const BYTE *pData, LONGLONG *pPTS)
{
    BYTE byte1 = pData[0];
    WORD word1 = MAKE_WORD(pData[1], pData[2]);
    WORD word2 = MAKE_WORD(pData[3], pData[4]);

    // Check marker bits.
    // The first byte can be '0010xxx1' or '0x11xxxx1'
    if (((byte1 & 0xE1) != 0x21) ||
        ((word1 & 0x01) != 0x01) ||
        ((word2 & 0x01) != 0x01) )
    {
        return E_FAIL;
    }

    LARGE_INTEGER li;

    // The PTS is 33 bits, so bit 32 goes in the high-order DWORD
    li.HighPart = (byte1 & 0x08) >> 3;

    li.LowPart = (static_cast<DWORD>(byte1 & 0x06) << 29) | 
              (static_cast<DWORD>(word1 & 0xFFFE) << 14) | 
              (static_cast<DWORD>(word2) >> 1);

    *pPTS = li.QuadPart;
    return S_OK;
}


//-------------------------------------------------------------------
// ParseStreamData
// Parses the stream information (for one stream) in the system 
// header. 
//-------------------------------------------------------------------

HRESULT ParseStreamData(const BYTE *pStreamInfo, MPEG1StreamHeader& header)
{
    // Check marker bits.
    if ( (pStreamInfo[1] & 0xC0) != 0xC0 )
    {
        return E_FAIL; // Invalid bits
    }

    HRESULT hr = S_OK;
    BYTE id = 0;
    BYTE num = 0;
    DWORD bound = 0;
    StreamType type = StreamType_Unknown;

    // The id is a stream code plus (for some types) a stream number, bitwise-OR'd.

    id = pStreamInfo[0];

    hr = ParseStreamId(id, &type, &num);
    if (FAILED(hr))
    {
        return hr;
    }

    // Calculate STD bound.
    bound = pStreamInfo[2] | ((pStreamInfo[1] & 0x1F) << 8);

    if (pStreamInfo[1] & 0x20)
    {
        bound *= 1024;
    }
    else
    {
        bound *= 128;
    }

    header.stream_id = id;
    header.type = type;
    header.number = num;
    header.sizeBound = bound;

    return S_OK;
}



//-------------------------------------------------------------------
// ParseStreamId
// Parses an MPEG-1 stream ID.
//
// Note: 
// The id is a stream code, plus (for some types) a stream number, 
// bitwise-OR'd. This function returns the type and the stream number. 
//
// See ISO/EIC 11172-1, sec 2.4.4.2 
//-------------------------------------------------------------------

HRESULT ParseStreamId(BYTE id, StreamType *pType, BYTE *pStreamNum)
{
    StreamType type = StreamType_Unknown;
    BYTE num = 0;

    switch (id)
    {
    case MPEG1_STREAMTYPE_ALL_AUDIO:
        type = StreamType_AllAudio;
        break;

    case MPEG1_STREAMTYPE_ALL_VIDEO:
        type = StreamType_AllVideo;
        break;

    case MPEG1_STREAMTYPE_RESERVED:
        type = StreamType_Reserved;
        break;

    case MPEG1_STREAMTYPE_PRIVATE1:
        type = StreamType_Private1;
        break;

    case MPEG1_STREAMTYPE_PADDING:
        type = StreamType_Padding;
        break;

    case MPEG1_STREAMTYPE_PRIVATE2:
        type = StreamType_Private2;
        break;

    default:
        if ((id & 0xE0) == MPEG1_STREAMTYPE_AUDIO_MASK)
        {
            type = StreamType_Audio;
            num = id & 0x1F;
        }
        else if ((id & 0xF0) == MPEG1_STREAMTYPE_VIDEO_MASK)
        {
            type = StreamType_Video;
            num = id & 0x0F;
        }
        else if ((id & 0xF0) == MPEG1_STREAMTYPE_DATA_MASK)
        {
            type = StreamType_Data;
            num = id & 0x0F;
        }
        else
        {
            TRACE((L"ParseStreamId: Unknown stream ID: %d\n", id));
            return E_FAIL; // Unknown stream ID code.
        }
    }

    *pType = type;
    *pStreamNum = num;
    return S_OK;
}


//-------------------------------------------------------------------
// ReadVideoSequenceHeader
// Parses a video sequence header.
//
// Call Parser::HasPacket() to ensure that pData points to the start 
// of a payload, and call Parser::PacketHeader() to verify it is a 
// video packet. 
//-------------------------------------------------------------------

HRESULT ReadVideoSequenceHeader(
    const BYTE *pData, 
    DWORD cbData, 
    MPEG1VideoSeqHeader& seqHeader, 
    DWORD *pAte
    )
{
    DWORD cbPadding = 0;

    *pAte = 0;

    // Skip to the sequence header code.
    while ( ((DWORD*)pData)[0] == 0 )
    {
        pData += 4;
        cbPadding += 4;

        if (cbData < 4)
        {
            *pAte = cbPadding;
            return S_FALSE;
        }
    }

    cbData -= cbPadding;

    // Validate the sequence header code.
    if (MAKE_DWORD(pData) != MPEG1_SEQUENCE_HEADER_CODE)
    {
        return E_FAIL;
    }

    // Check for the minimum size buffer.
    if (cbData < MPEG1_VIDEO_SEQ_HEADER_MIN_SIZE)
    {
        return S_FALSE;
    }

    // Calculate the actual required size.
    DWORD cbRequired = MPEG1_VIDEO_SEQ_HEADER_MIN_SIZE;

    // Look for quantization matrices.
    BOOL bNonIntra = FALSE;
    if ( HAS_FLAG(pData[11], 0x02) )
    {
        // Intra quantization matrix is TRUE.
        cbRequired += 64;

        // Non-intra flag follows the intra matrix, but first check
        // if we have enough data before trying to get the non-intra flag.
        if (cbData >= cbRequired)
        {
            bNonIntra = HAS_FLAG( pData[11 + 64], 0x01 );
        }
    }
    // Intra is FALSE, look for non-intra flag
    else if ( HAS_FLAG(pData[11], 0x01) )
    {
        cbRequired += 64;
    }

    if (cbData < cbRequired)
    {
        return S_FALSE;
    }

    ZeroMemory(&seqHeader, sizeof(seqHeader));

    // Check the marker bit.
    if ( !HAS_FLAG(pData[10], 0x20) )
    {
        return E_FAIL;
    }

    BYTE parCode = pData[7] >> 4;
    BYTE frameRateCode = pData[7] & 0x0F;

    if (FAILED(GetPixelAspectRatio(parCode, &seqHeader.pixelAspectRatio)))
    {
        return E_FAIL;
    }

    if (FAILED(GetFrameRate(frameRateCode, &seqHeader.frameRate)))
    {
        return E_FAIL;
    }

    seqHeader.width = (pData[4] << 4) | (pData[5] >> 4) ;
    seqHeader.height = ((pData[5] & 0x0F) << 8) | (pData[6]);
    seqHeader.bitRate = (pData[8] << 10) | (pData[9] << 2) | (pData[10] >> 6);

    if (seqHeader.bitRate == 0)
    {
        return E_FAIL;  // Not allowed.
    }
    else if (seqHeader.bitRate == 0x3FFFF)
    {
        seqHeader.bitRate = 0; // Variable bit-rate.
    }
    else
    {
        seqHeader.bitRate = seqHeader.bitRate * 400; // Units of 400 bps
    }

    seqHeader.cbVBV_Buffer = ( ((pData[10] & 0x1F) << 5) | (pData[11] >> 3) ) * 2048;
    seqHeader.bConstrained = HAS_FLAG(pData[11], 0x04);

    seqHeader.cbHeader = cbRequired;
    CopyMemory(seqHeader.header, pData, cbRequired);

    *pAte = cbRequired + cbPadding;

    return S_OK;
}



//-------------------------------------------------------------------
// GetFrameRate
// Returns the frame rate from the picture_rate field of the sequence 
// header.
//
// See ISO/IEC 11172-2, 2.4.3.2 "Sequence Header"
//-------------------------------------------------------------------

HRESULT GetFrameRate(BYTE frameRateCode, MFRatio *pRatio)
{
    MFRatio frameRates[] = 
    {
        { 0, 0 },           // invalid
        { 24000, 1001 },    // 23.976 fps
        { 24, 1 },
        { 25, 1 },
        { 30000, 1001 },    // 29.97 fps
        { 50, 1 },
        { 60000, 1001 },    // 59.94 fps
        { 60, 1 }
    };

    if (frameRateCode < 1 || frameRateCode >= ARRAYSIZE(frameRates))
    {
        return MF_E_INVALIDTYPE;
    }

    pRatio->Numerator = frameRates[frameRateCode].Numerator;
    pRatio->Denominator = frameRates[frameRateCode].Denominator;

    return S_OK;
}

//-------------------------------------------------------------------
// GetPixelAspectRatio
// Returns the pixel aspect ratio (PAR) from the pel_aspect_ratio 
// field of the sequence header.
//
// See ISO/IEC 11172-2, 2.4.3.2 "Sequence Header"
//-------------------------------------------------------------------

HRESULT GetPixelAspectRatio(BYTE pixelAspectCode, MFRatio *pRatio)
{
    DWORD height[] = { 0, 10000, 6735, 7031, 7615, 8055, 8437, 8935, 9157, 
        9815, 10255, 10695, 10950, 11575, 12015 };

    const DWORD width = 10000;

    if (pixelAspectCode < 1 || pixelAspectCode >= ARRAYSIZE(height))
    {
        return MF_E_INVALIDTYPE;
    }

    pRatio->Numerator = height[pixelAspectCode];
    pRatio->Denominator = width;

    return S_OK;

}


//-------------------------------------------------------------------
// ReadAudioFrameHeader
// Parses an audio frame header.
//
// Call Parser::HasPacket() to ensure that pData points to the start 
// of a payload, and call Parser::PacketHeader() to verify it is an 
// audio packet. 
//-------------------------------------------------------------------

HRESULT ReadAudioFrameHeader(
    const BYTE *pData, 
    DWORD cbData, 
    MPEG1AudioFrameHeader& audioHeader, 
    DWORD *pAte
    )
{
    HRESULT hr = S_OK;
    MPEG1AudioFrameHeader header;
    ZeroMemory(&header, sizeof(header)); 

    BYTE bitRateIndex = 0;
    BYTE samplingIndex = 0;

    *pAte = 0;

    if (cbData < MPEG1_AUDIO_FRAME_HEADER_SIZE)
    {
        return S_FALSE;
    }


    if (pData[0] != 0xFF)
    {
        return E_FAIL;
    }

    if (!HAS_FLAG(pData[1], 0xF8))
    {
        return E_FAIL;
    }

    // Layer bits
    switch (pData[1] & 0x06)
    {
    case 0x00:
        return E_FAIL;

    case 0x06:
        header.layer = MPEG1_Audio_Layer1;
        break;

    case 0x04:
        header.layer = MPEG1_Audio_Layer2;
        break;

    case 0x02:
        header.layer = MPEG1_Audio_Layer3;
        break;

    default:
        return E_UNEXPECTED; // Cannot actually happen, given our bitmask above.
    }

    bitRateIndex = (pData[2] & 0xF0) >> 4;
    samplingIndex = (pData[2] & 0x0C) >> 2;

    // Bit rate.
    // Note: Accoring to ISO/IEC 11172-3, some combinations of bitrate and 
    // mode are not valid. However, this is up to the decoder to validate.
    CHECK_HR(hr = GetAudioBitRate(header.layer, bitRateIndex, &header.dwBitRate));

    // Sampling frequency.
    CHECK_HR(hr = GetSamplingFrequency(samplingIndex, &header.dwSamplesPerSec));

    header.mode = static_cast<MPEG1AudioMode>((pData[3] & 0xC0) >> 6);
    header.modeExtension = (pData[3] & 0x30) >> 4;
    header.emphasis = (pData[3] & 0x03);

    // Parse the various bit flags.
    if (HAS_FLAG(pData[1], 0x01))
    {
        header.wFlags |= MPEG1_AUDIO_PROTECTION_BIT;
    }
    if (HAS_FLAG(pData[2], 0x01))
    {
        header.wFlags |= MPEG1_AUDIO_PRIVATE_BIT;
    }
    if (HAS_FLAG(pData[3], 0x08))
    {
        header.wFlags |= MPEG1_AUDIO_COPYRIGHT_BIT;
    }
    if (HAS_FLAG(pData[3], 0x04))
    {
        header.wFlags |= MPEG1_AUDIO_ORIGINAL_BIT;
    }

    if (header.mode == MPEG1_Audio_SingleChannel)
    {
        header.nChannels = 1;
    }
    else
    {
        header.nChannels = 2;
    }

    header.nBlockAlign = 1; 

    CopyMemory(&audioHeader, &header, sizeof(audioHeader));

done:
    return  hr;

};


//-------------------------------------------------------------------
// GetAudioBitRate
// Returns the audio bit rate in KBits per second, from the 
// bitrate_index field of the audio frame header.
//
// See ISO/IEC 11172-3, 2.4.2.3, "Header"
//-------------------------------------------------------------------

HRESULT GetAudioBitRate(MPEG1AudioLayer layer, BYTE index, DWORD *pdwBitRate)
{
    const DWORD MAX_BITRATE_INDEX = 14;

    // Table of bit rates. 
    const DWORD bitrate[3][ (MAX_BITRATE_INDEX+1) ] = 
    {
        { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448 },    // Layer I
        { 0, 32, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 384 },       // Layer II
        { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 }         // Layer III
    };

    if (layer < MPEG1_Audio_Layer1 || layer > MPEG1_Audio_Layer3)
    {
        return E_INVALIDARG;
    }
    if (index > MAX_BITRATE_INDEX)
    {
        return E_INVALIDARG;
    }

    *pdwBitRate = bitrate[layer][index];

    return S_OK;
}

//-------------------------------------------------------------------
// GetSamplingFrequency
// Returns the sampling frequency in samples per second, from the
// sampling_frequency field of the audio frame header.
//
// See ISO/IEC 11172-3, 2.4.2.3, "Header"
//-------------------------------------------------------------------

inline HRESULT GetSamplingFrequency(BYTE code, DWORD *pdwSamplesPerSec)
{
    switch (code)
    {
    case 0:
        *pdwSamplesPerSec = 44100;
        break;
    case 1:
        *pdwSamplesPerSec = 48000;
        break;
    case 2:
        *pdwSamplesPerSec = 32000;
        break;
    default:
        return E_FAIL;
    }
    return S_OK;
}
        
