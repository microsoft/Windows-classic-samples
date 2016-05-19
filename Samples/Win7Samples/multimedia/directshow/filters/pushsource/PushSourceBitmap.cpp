//------------------------------------------------------------------------------
// File: PushSourceBitmap.cpp
//
// Desc: DirectShow sample code - In-memory push mode source filter
//       Provides a static bitmap as the video output stream.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>

#include "PushSource.h"
#include "PushGuids.h"

#define BITMAP_NAME TEXT("sample.bmp")

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

// Utility method to query location of installed SDK's media path
const TCHAR* DXUtil_GetDXSDKMediaPath();


/**********************************************
 *
 *  CPushPinBitmap Class
 *  
 *
 **********************************************/

CPushPinBitmap::CPushPinBitmap(HRESULT *phr, CSource *pFilter)
      : CSourceStream(NAME("Push Source Bitmap"), phr, pFilter, L"Out"),
        m_FramesWritten(0),
        m_bZeroMemory(0),
        m_pBmi(0),
        m_cbBitmapInfo(0),
        m_hFile(INVALID_HANDLE_VALUE),
        m_pFile(NULL),
        m_pImage(NULL),
        m_iFrameNumber(0),
        m_rtFrameLength(FPS_5) // Display 5 bitmap frames per second
{
    // The main point of this sample is to demonstrate how to take a DIB
    // in host memory and insert it into a video stream. 
    // To keep this sample as simple as possible, we just read a single 24 bpp bitmap
    // from a file and copy it into every frame that we send downstream.

    // In the filter graph, we connect this filter to the AVI Mux, which creates 
    // the AVI file with the video frames we pass to it. In this case, 
    // the end result is a still image rendered as a video stream.

    // Your filter will hopefully do something more interesting here. 
    // The main point is to set up a buffer containing the DIB pixel bits. 
    // This must be done before you start running.

    TCHAR szCurrentDir[MAX_PATH], szFileCurrent[MAX_PATH], szFileMedia[MAX_PATH];

    // First look for the bitmap in the current directory
    GetCurrentDirectory(MAX_PATH-1, szCurrentDir);
    (void)StringCchPrintf(szFileCurrent, NUMELMS(szFileCurrent), TEXT("%s\\%s\0"), szCurrentDir, BITMAP_NAME);

    m_hFile = CreateFile(szFileCurrent, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                         FILE_ATTRIBUTE_NORMAL, NULL);

    if (m_hFile == INVALID_HANDLE_VALUE)
    {
        // File was not in the application's current directory,
        // so look in the DirectX SDK media path instead.
        (void)StringCchCopy(szFileMedia, NUMELMS(szFileMedia), DXUtil_GetDXSDKMediaPath());
        (void)StringCchCat(szFileMedia, NUMELMS(szFileMedia), BITMAP_NAME);

        m_hFile = CreateFile(szFileMedia, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                             FILE_ATTRIBUTE_NORMAL, NULL);

        if (m_hFile == INVALID_HANDLE_VALUE)
        {
            TCHAR szMsg[MAX_PATH + MAX_PATH + 100];

            (void)StringCchPrintf(szMsg, NUMELMS(szMsg),TEXT("Could not open bitmap source file in the application directory:\r\n\r\n\t[%s]\n\n")
                     TEXT("or in the DirectX SDK Media folder:\r\n\r\n\t[%s]\n\n")
                     TEXT("Please copy this file either to the application's folder\r\n")
                     TEXT("or to the DirectX SDK Media folder, then recreate this filter.\r\n")
                     TEXT("Otherwise, you will not be able to render the output pin.\0"),
                     szFileCurrent, szFileMedia);

            OutputDebugString(szMsg);
            MessageBox(NULL, szMsg, TEXT("PushSource filter error"), MB_ICONERROR | MB_OK);
            *phr = HRESULT_FROM_WIN32(GetLastError());
            return;
        }
    }

    DWORD dwFileSize = GetFileSize(m_hFile, NULL);
    if (dwFileSize == INVALID_FILE_SIZE)
    {
        DbgLog((LOG_TRACE, 1, TEXT("Invalid file size")));
        *phr = HRESULT_FROM_WIN32(GetLastError());
        return;
    }

    m_pFile = new BYTE[dwFileSize];
    if(!m_pFile)
    {
        OutputDebugString(TEXT("Could not allocate m_pImage\n"));
        *phr = E_OUTOFMEMORY;
        return;
    }

    DWORD nBytesRead = 0;
    if(!ReadFile(m_hFile, m_pFile, dwFileSize, &nBytesRead, NULL))
    {
        *phr = HRESULT_FROM_WIN32(GetLastError());
        OutputDebugString(TEXT("ReadFile failed\n"));
        return;
    }

    // WARNING - This code does not verify that the file is a valid bitmap file.
    // In your own filter, you would check this or else generate the bitmaps 
    // yourself in memory.

    int cbFileHeader = sizeof(BITMAPFILEHEADER);

    // Store the size of the BITMAPINFO 
    BITMAPFILEHEADER *pBm = (BITMAPFILEHEADER*)m_pFile;
    m_cbBitmapInfo = pBm->bfOffBits - cbFileHeader;

    // Store a pointer to the BITMAPINFO
    m_pBmi = (BITMAPINFO*)(m_pFile + cbFileHeader);

    // Store a pointer to the starting address of the pixel bits
    m_pImage = m_pFile + cbFileHeader + m_cbBitmapInfo;

    // Close and invalidate the file handle, since we have copied its bitmap data
    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
}


CPushPinBitmap::~CPushPinBitmap()
{   
    DbgLog((LOG_TRACE, 3, TEXT("Frames written %d"),m_iFrameNumber));

    if (m_pFile)
    {
        delete [] m_pFile;
    }

    // The constructor might quit early on error and not close the file...
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
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

HRESULT CPushPinBitmap::GetMediaType(CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pMediaType, E_POINTER);

    // If the bitmap file was not loaded, just fail here.
    if (!m_pImage)
        return E_FAIL;

    // Allocate enough room for the VIDEOINFOHEADER and the color tables
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER*)pMediaType->AllocFormatBuffer(SIZE_PREHEADER + m_cbBitmapInfo);
    if (pvi == 0) 
        return(E_OUTOFMEMORY);

    ZeroMemory(pvi, pMediaType->cbFormat);   
    pvi->AvgTimePerFrame = m_rtFrameLength;

    // Copy the header info
    memcpy(&(pvi->bmiHeader), m_pBmi, m_cbBitmapInfo);

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


HRESULT CPushPinBitmap::DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest)
{
    HRESULT hr;
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    CheckPointer(pAlloc, E_POINTER);
    CheckPointer(pRequest, E_POINTER);

    // If the bitmap file was not loaded, just fail here.
    if (!m_pImage)
        return E_FAIL;

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
HRESULT CPushPinBitmap::FillBuffer(IMediaSample *pSample)
{
    BYTE *pData;
    long cbData;

    CheckPointer(pSample, E_POINTER);

    // If the bitmap file was not loaded, just fail here.
    if (!m_pImage)
        return E_FAIL;

    CAutoLock cAutoLockShared(&m_cSharedState);

    // Access the sample's data buffer
    pSample->GetPointer(&pData);
    cbData = pSample->GetSize();

    // Check that we're still using video
    ASSERT(m_mt.formattype == FORMAT_VideoInfo);

    VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)m_mt.pbFormat;

    // If we want to change the contents of our source buffer (m_pImage)
    // at some interval or based on some condition, this is where to do it.
    // Remember that the new data has the same format that we specified in GetMediaType.
    // For example: 
    // if(m_iFrameNumber > SomeValue)
    //    LoadNewBitsIntoBuffer(m_pImage)

    // Copy the DIB bits over into our filter's output buffer.
    // Since sample size may be larger than the image size, bound the copy size.
    memcpy(pData, m_pImage, min(pVih->bmiHeader.biSizeImage, (DWORD) cbData));

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

    return S_OK;
}


/**********************************************
 *
 *  CPushSourceBitmap Class
 *
 **********************************************/

CPushSourceBitmap::CPushSourceBitmap(IUnknown *pUnk, HRESULT *phr)
           : CSource(NAME("PushSourceBitmap"), pUnk, CLSID_PushSourceBitmap)
{
    // The pin magically adds itself to our pin array.
    m_pPin = new CPushPinBitmap(phr, this);

    if (phr)
    {
        if (m_pPin == NULL)
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }  
}


CPushSourceBitmap::~CPushSourceBitmap()
{
    delete m_pPin;
}


CUnknown * WINAPI CPushSourceBitmap::CreateInstance(IUnknown *pUnk, HRESULT *phr)
{
    CPushSourceBitmap *pNewFilter = new CPushSourceBitmap(pUnk, phr );

    if (phr)
    {
        if (pNewFilter == NULL) 
            *phr = E_OUTOFMEMORY;
        else
            *phr = S_OK;
    }

    return pNewFilter;
}



//-----------------------------------------------------------------------------
// Name: DXUtil_GetDXSDKMediaPath()
// Desc: Returns the DirectX SDK media path
//-----------------------------------------------------------------------------
const TCHAR* DXUtil_GetDXSDKMediaPath()
{
    static TCHAR strNull[2] = {0};
    static TCHAR strPath[MAX_PATH + 10];
    HKEY  hKey=0;
    DWORD type=0, size=MAX_PATH;

    strPath[0] = 0;     // Initialize to NULL
    
    // Open the appropriate registry key
    LONG result = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                TEXT("Software\\Microsoft\\DirectX SDK"),
                                0, KEY_READ, &hKey );
    if( ERROR_SUCCESS != result )
        return strNull;

    result = RegQueryValueEx( hKey, TEXT("DX9D4SDK Samples Path"), NULL,
                              &type, (BYTE*)strPath, &size );

    if( ERROR_SUCCESS != result )
    {
        size = MAX_PATH;    // Reset size field
        result = RegQueryValueEx( hKey, TEXT("DX81SDK Samples Path"), NULL,
                                  &type, (BYTE*)strPath, &size );

        if( ERROR_SUCCESS != result )
        {
            size = MAX_PATH;    // Reset size field
            result = RegQueryValueEx( hKey, TEXT("DX8SDK Samples Path"), NULL,
                                      &type, (BYTE*)strPath, &size );

            if( ERROR_SUCCESS != result )
            {
                RegCloseKey( hKey );
                return strNull;
            }
        }
    }

    RegCloseKey( hKey );
    (void)StringCchCat( strPath, NUMELMS(strPath),TEXT("\\Media\\Misc\\\0") );

    return strPath;
}


