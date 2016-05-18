//------------------------------------------------------------------------------
// File: PushSource.H
//
// Desc: DirectShow sample code - In-memory push mode source filter
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


#include <strsafe.h>

// UNITS = 10 ^ 7  
// UNITS / 30 = 30 fps;
// UNITS / 20 = 20 fps, etc
const REFERENCE_TIME FPS_30 = UNITS / 30;
const REFERENCE_TIME FPS_20 = UNITS / 20;
const REFERENCE_TIME FPS_10 = UNITS / 10;
const REFERENCE_TIME FPS_5  = UNITS / 5;
const REFERENCE_TIME FPS_4  = UNITS / 4;
const REFERENCE_TIME FPS_3  = UNITS / 3;
const REFERENCE_TIME FPS_2  = UNITS / 2;
const REFERENCE_TIME FPS_1  = UNITS / 1;

const REFERENCE_TIME rtDefaultFrameLength = FPS_10;

// Filter name strings
#define g_wszPushBitmap     L"PushSource Bitmap Filter"
#define g_wszPushBitmapSet  L"PushSource BitmapSet Filter"
#define g_wszPushDesktop    L"PushSource Desktop Filter"

// Number of bitmap files to load in the CPushPinBitmapSet class
#define NUM_FILES 5


/**********************************************
 *
 *  Class declarations
 *
 **********************************************/

class CPushPinBitmap : public CSourceStream
{
protected:

    int m_FramesWritten;				// To track where we are in the file
    BOOL m_bZeroMemory;                 // Do we need to clear the buffer?
    CRefTime m_rtSampleTime;	        // The time stamp for each sample

    BITMAPINFO *m_pBmi;                 // Pointer to the bitmap header
    DWORD       m_cbBitmapInfo;         // Size of the bitmap header
	
	// File opening variables 
	HANDLE m_hFile;                     // Handle returned from CreateFile
    BYTE * m_pFile;                     // Points to beginning of file buffer
	BYTE * m_pImage;                    // Points to pixel bits                                      

    int m_iFrameNumber;
    const REFERENCE_TIME m_rtFrameLength;

    CCritSec m_cSharedState;            // Protects our internal state
    CImageDisplay m_Display;            // Figures out our media type for us

public:

    CPushPinBitmap(HRESULT *phr, CSource *pFilter);
    ~CPushPinBitmap();

    // Override the version that offers exactly one media type
    HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};


class CPushPinBitmapSet : public CSourceStream
{
protected:

    int m_FramesWritten;				// To track where we are in the file
    BOOL m_bZeroMemory;                 // Do we need to clear the buffer?
    CRefTime m_rtSampleTime;	        // The time stamp for each sample

    BITMAPINFO *m_pBmi[NUM_FILES];      // Pointer to the bitmap headers
    DWORD m_cbBitmapInfo[NUM_FILES];    // Size of the bitmap headers
	
	// File opening variables 
	HANDLE m_hFile[NUM_FILES];          // Handles returned from CreateFile
    BYTE * m_pFile[NUM_FILES];          // Points to beginning of file buffers
	BYTE * m_pImage[NUM_FILES];         // Points to pixel bits                                      
    BOOL m_bFilesLoaded;

    int m_iCurrentBitmap;               // Which bitmap is being displayed
    int m_iFrameNumber;                 // How many frames have been displayed
    const REFERENCE_TIME m_rtFrameLength;   // Duration of one frame

    CCritSec m_cSharedState;            // Protects our internal state
    CImageDisplay m_Display;            // Figures out our media type for us

public:

    CPushPinBitmapSet(HRESULT *phr, CSource *pFilter);
    ~CPushPinBitmapSet();

    // Override the version that offers exactly one media type
    HRESULT GetMediaType(CMediaType *pMediaType);
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};


class CPushPinDesktop : public CSourceStream
{
protected:

    int m_FramesWritten;				// To track where we are in the file
    BOOL m_bZeroMemory;                 // Do we need to clear the buffer?
    CRefTime m_rtSampleTime;	        // The time stamp for each sample

    int m_iFrameNumber;
    const REFERENCE_TIME m_rtFrameLength;

    RECT m_rScreen;                     // Rect containing entire screen coordinates

    int m_iImageHeight;                 // The current image height
    int m_iImageWidth;                  // And current image width
    int m_iRepeatTime;                  // Time in msec between frames
    int m_nCurrentBitDepth;             // Screen bit depth

    CMediaType m_MediaType;
    CCritSec m_cSharedState;            // Protects our internal state
    CImageDisplay m_Display;            // Figures out our media type for us

public:

    CPushPinDesktop(HRESULT *phr, CSource *pFilter);
    ~CPushPinDesktop();

    // Override the version that offers exactly one media type
    HRESULT DecideBufferSize(IMemAllocator *pAlloc, ALLOCATOR_PROPERTIES *pRequest);
    HRESULT FillBuffer(IMediaSample *pSample);
    
    // Set the agreed media type and set up the necessary parameters
    HRESULT SetMediaType(const CMediaType *pMediaType);

    // Support multiple display formats
    HRESULT CheckMediaType(const CMediaType *pMediaType);
    HRESULT GetMediaType(int iPosition, CMediaType *pmt);

    // Quality control
	// Not implemented because we aren't going in real time.
	// If the file-writing filter slows the graph down, we just do nothing, which means
	// wait until we're unblocked. No frames are ever dropped.
    STDMETHODIMP Notify(IBaseFilter *pSelf, Quality q)
    {
        return E_FAIL;
    }

};



class CPushSourceBitmap : public CSource
{

private:
    // Constructor is private because you have to use CreateInstance
    CPushSourceBitmap(IUnknown *pUnk, HRESULT *phr);
    ~CPushSourceBitmap();

    CPushPinBitmap *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

};


class CPushSourceBitmapSet : public CSource
{

private:
    // Constructor is private because you have to use CreateInstance
    CPushSourceBitmapSet(IUnknown *pUnk, HRESULT *phr);
    ~CPushSourceBitmapSet();

    CPushPinBitmapSet *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

};


class CPushSourceDesktop : public CSource
{

private:
    // Constructor is private because you have to use CreateInstance
    CPushSourceDesktop(IUnknown *pUnk, HRESULT *phr);
    ~CPushSourceDesktop();

    CPushPinDesktop *m_pPin;

public:
    static CUnknown * WINAPI CreateInstance(IUnknown *pUnk, HRESULT *phr);  

};


