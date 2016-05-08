//------------------------------------------------------------------------------
// File: PushSourceBitmapSet.cpp
//
// Desc: DirectShow sample code - In-memory push mode source filter
//       Provides a rotating set of bitmaps as the video output stream.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>

#include "PushSource.h"
#include "PushGuids.h"


const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

// Utility method to query location of installed SDK's media path
const TCHAR* DXUtil_GetDXSDKMediaPath();


/**********************************************
 *
 *  CPushPinBitmapSet Class
 *  
 *
 **********************************************/

CPushPinBitmapSet::CPushPinBitmapSet(HRESULT *phr, CSource *pFilter)
      : CSourceStream(NAME("Push Source BitmapSet"), phr, pFilter, L"Out"),
        m_FramesWritten(0),
        m_bZeroMemory(0),
        m_iFrameNumber(0),
        m_rtFrameLength(FPS_2), // Display 2 bitmap frames per second
        m_iCurrentBitmap(0),
        m_bFilesLoaded(FALSE)
{
    int nFilesLoaded=0;

    // Initialize member data arrays
    ZeroMemory(&m_cbBitmapInfo, NUM_FILES * sizeof(DWORD));
    ZeroMemory(&m_pBmi,   NUM_FILES * sizeof(BITMAPINFO *));
    ZeroMemory(&m_hFile,  NUM_FILES * sizeof(HANDLE));
    ZeroMemory(&m_pFile,  NUM_FILES * sizeof(BYTE *));
    ZeroMemory(&m_pImage, NUM_FILES * sizeof(BYTE *));

    // The main point of this sample is to demonstrate how to take a DIB
    // in host memory and insert it into a video stream. 
    // We read a set of bitmaps from files and copy one bitmap
    // into every frame that we send downstream.

    // In the filter graph, we connect this filter to the AVI Mux, which creates 
    // the AVI file with the video frames we pass to it. In this case, 
    // the end result is a rotating set of images rendered as a video stream.    

    // Read the current directory and SDK media directory into local strings
    TCHAR szCurrentDir[MAX_PATH], szMediaDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH-1, szCurrentDir);
    StringCchCopy(szMediaDir, MAX_PATH, DXUtil_GetDXSDKMediaPath());

    for (int i=0; i < NUM_FILES; i++)
    {
        TCHAR szFileCurrent[MAX_PATH], szFileMedia[MAX_PATH];

        // Assume that the bitmap in the application's directory
        (void)StringCchPrintf(szFileCurrent, NUMELMS(szFileCurrent),TEXT("%s\\BitmapSet%d.bmp\0"), szCurrentDir, i);

        m_hFile[i] = CreateFile(szFileCurrent, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL, NULL);

        if (m_hFile[i] == INVALID_HANDLE_VALUE)
        {
            // File was not in the application's current directory,
            // so look in the DirectX SDK media path instead.  The path contained
            // in szMediaDir will already have a trailing backslash '\'.
            (void)StringCchPrintf(szFileMedia, NUMELMS(szFileMedia),TEXT("%sBitmapSet%d.bmp\0"), szMediaDir, i);

            m_hFile[i] = CreateFile(szFileMedia, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                                    FILE_ATTRIBUTE_NORMAL, NULL);

            if (m_hFile[i] == INVALID_HANDLE_VALUE)
            {
                TCHAR szMsg[2*MAX_PATH + 100];

                (void)StringCchPrintf(szMsg, NUMELMS(szMsg), TEXT("Could not open bitmap source file (#%d of %d) in the application directory:\r\n\r\n\t[%s]\n\n")
                         TEXT("or in the DirectX SDK Media folder:\r\n\r\n\t[%s]\n\n")
                         TEXT("Please copy this file either to the application's folder\r\n")
                         TEXT("or to the DirectX SDK Media folder, then recreate this filter.\r\n")
                         TEXT("Otherwise, you will not be able to render the output pin.\0"),
                         i+1, NUM_FILES, szFileCurrent, szFileMedia);

                OutputDebugString(szMsg);
                MessageBox(NULL, szMsg, TEXT("PushSource filter error"), MB_ICONERROR | MB_OK);
                *phr = HRESULT_FROM_WIN32(GetLastError());
                return;
            }
        }

        DWORD dwFileSize = GetFileSize(m_hFile[i], NULL);
        if (dwFileSize == INVALID_FILE_SIZE)
        {
            DbgLog((LOG_TRACE, 1, TEXT("Invalid file size")));
            *phr = HRESULT_FROM_WIN32(GetLastError());
            return;
        }

        m_pFile[i] = new BYTE[dwFileSize];
        if(!m_pFile[i])
        {
            OutputDebugString(TEXT("Could not allocate m_pImage\n"));
            *phr = E_OUTOFMEMORY;
            return;
        }

        DWORD nBytesRead = 0;
        if(!ReadFile(m_hFile[i], m_pFile[i], dwFileSize, &nBytesRead, NULL))
        {
            *phr = HRESULT_FROM_WIN32(GetLastError());
            OutputDebugString(TEXT("ReadFile failed\n"));
            return;
        }

        // WARNING - This code does not verify the file is a valid bitmap file.
        // In your own filter, you would check this or else generate the bitmaps 
        // yourself in memory.

        int cbFileHeader = sizeof(BITMAPFILEHEADER);

        // Store the size of the BITMAPINFO 
        BITMAPFILEHEADER *pBm = (BITMAPFILEHEADER*)m_pFile[i];
        m_cbBitmapInfo[i] = pBm->bfOffBits - cbFileHeader;

        // Store a pointer to the BITMAPINFO
        m_pBmi[i] = (BITMAPINFO*)(m_pFile[i] + cbFileHeader);

        // Store a pointer to the starting address of the pixel bits
        m_pImage[i] = m_pFile[i] + cbFileHeader + m_cbBitmapInfo[i];

        // Close and invalidate the file handle, since we have copied its bitmap data
        CloseHandle(m_hFile[i]);
        m_hFile[i] = INVALID_HANDLE_VALUE;

        // Count this is a successful file load.  If not all files load
        // properly, then the filter will not operate correctly.
        nFilesLoaded++;
    }

    // Make sure that ALL files were properly loaded
    if (nFilesLoaded != NUM_FILES)
        *phr = E_FAIL;
    else
        m_bFilesLoaded = TRUE;
}


CPushPinBitmapSet::~CPushPinBitmapSet()
{   
    DbgLog((LOG_TRACE, 3, TEXT("Frames written %d"),m_iFrameNumber));

    for (int i=0; i < NUM_FILES; i++)
    {
        if (m_pFile[i])
        {
            delete [] m_pFile[i];
            m_pFile[i] = 0;
        }

        // The constructor might quit early on error and not close the file...
        if (m_hFile[i] != INVALID_HANDLE_VALUE)
        {
            CloseHandle(m_hFile[i]);
        }
    }
}


// GetMediaType: This method tells the downstream pin what types we support.

// Here is how CSourceStream deals with media types:
//
// If you support exactly one type, override GetMediaType(CMediaType*). It will then be
// called when (a) our filter proposes a media type, (b) the other filter proposes a
// type and we have to check that type.
//
// If you support > 1 type, override GetMediaType(int,CMediaType*) AND CheckMediaType.
//
// In this case we support only one type, which we obtain from the bitmap file.

HRESULT CPushPinBitmapSet::GetMediaType(CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);

    // If the bitmap files were not loaded, just fail here.
    if (!m_bFilesLoaded)
        return E_FAIL;

    // Allocate enough room for the VIDEOINFOHEADER and the color tables
    VIDEOINFOHEADER *pvi = 
        (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer(SIZE_PREHEADER + 
                                                        m_cbBitmapInfo[m_iCurrentBitmap]);
    if (pvi == 0) 
        return(E_OUTOFMEMORY);

    // Initialize the video info header
    ZeroMemory(pvi, pMediaType->cbFormat);   
    pvi->AvgTimePerFrame = m_rtFrameLength;

    // Copy the header info from the current bitmap
    memcpy(&(pvi->bmiHeader), m_pBmi[m_iCurrentBitmap], m_cbBitmapInfo[m_iCurrentBitmap]);

    // Set image size for use in FillBuffer
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);

    // Clear source and target rectangles
    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pMediaType->SetType(&MEDIATYPE_Video);
    pMediaType->SetFormatType(&FORMAT_VideoInfo);
    pMediaType->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pMediaType->SetSubtype(&SubTypeGUID);
    pMediaType->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return S_OK;
}


HRESULT CPushPinBitmapSet::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    HRESULT hr;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // If the bitmap files were not loaded, just fail here.
    if (!m_bFilesLoaded)
        return E_FAIL;

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*) m_mt.Format();
    
    // Ensure a minimum number of buffers
    if (pRequest->cBuffers == 0)
    {
        pRequest->cBuffers = 2;
    }
    pRequest->cbBuffer = pvi->bmiHeader.biSizeImage;

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pRequest, &Actual);
    if (FAILED(hr)) 
    {
        return hr;
    }

    // Is this allocator unsuitable?
    if (Actual.cbBuffer < pRequest->cbBuffer) 
    {
        return E_FAIL;
    }

    return S_OK;
}


// This is where we insert the DIB bits into the video stream.
// FillBuffer is called once for every sample in the stream.
HRESULT CPushPinBitmapSet::FillBuffer(IMediaSample *pSample)
{
    BYTE *pData;
    long cbData;

    // If the bitmap files were not loaded, just fail here.
    if (!m_bFilesLoaded)
        return E_FAIL;

    CheckPointer(pSample, E_POINTER);
    CAutoLock cAutoLockShared(&m_cSharedState);

    // Access the sample's data buffer
    pSample->GetPointer(&pData);
    cbData = pSample->GetSize();

    // Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

    // Copy the DIB bits over into our filter's output buffer.
    // Since sample size may be larger than the image size, bound the copy size.
    // Remember that the new data has the same format that we specified in GetMediaType.
    memcpy(pData, m_pImage[m_iCurrentBitmap], min(pVih->bmiHeader.biSizeImage, (DWORD) cbData));

    // Set the timestamps that will govern playback frame rate.
    // If this file is getting written out as an AVI,
    // then you'll also need to configure the AVI Mux filter to 
    // set the Average Time Per Frame for the AVI Header.
    // The current time is the sample's start
    REFERENCE_TIME rtStart = m_iFrameNumber * m_rtFrameLength;
    REFERENCE_TIME rtStop  = rtStart + m_rtFrameLength;

    pSample->SetTime(&rtStart, &rtStop);
    m_iFrameNumber++;

    // Set TRUE on every sample for uncompressed frames
    pSample->SetSyncPoint(TRUE);

    // Increment the current buffer so that the next FillBuffer() call 
    // will use the bits from the next bitmap in the set.
    m_iCurrentBitmap++;
    m_iCurrentBitmap %= NUM_FILES;

    return S_OK;
}


/**********************************************
 *
 *  CPushSourceBitmapSet Class
 *
 **********************************************/

CPushSourceBitmapSet::CPushSourceBitmapSet(IUnknown *pUnk, HRESULT *phr)
           : CSource(NAME("PushSourceBitmapSet"), pUnk, CLSID_PushSourceBitmapSet)
{
    // The pin magically adds itself to our pin array.
    m_pPin = new CPushPinBitmapSet(phr, this);

    if (phr)
    {
        if (m_pPin == NULL)
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }  
}


CPushSourceBitmapSet::~CPushSourceBitmapSet()
{
    delete m_pPin;
}


CUnknown * WINAPI CPushSourceBitmapSet::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    CPushSourceBitmapSet *pNewFilter = new CPushSourceBitmapSet(pUnk, phr );

    if (phr)
    {
        if (pNewFilter == NULL) 
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }

    return pNewFilter;
}


