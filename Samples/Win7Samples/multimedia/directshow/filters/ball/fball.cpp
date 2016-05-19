//------------------------------------------------------------------------------
// File: FBall.cpp
//
// Desc: DirectShow sample code - implementation of filter behaviors
//       for the bouncing ball source filter.  For more information,
//       refer to Ball.cpp.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include <streams.h>
#include <olectl.h>
#include <initguid.h>
#include "ball.h"
#include "fball.h"

#pragma warning(disable:4710)  // 'function': function not inlined (optimzation)

// Setup data

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudOpPin =
{
    L"Output",              // Pin string name
    FALSE,                  // Is it rendered
    TRUE,                   // Is it an output
    FALSE,                  // Can we have none
    FALSE,                  // Can we have many
    &CLSID_NULL,            // Connects to filter
    NULL,                   // Connects to pin
    1,                      // Number of types
    &sudOpPinTypes };       // Pin details

const AMOVIESETUP_FILTER sudBallax =
{
    &CLSID_BouncingBall,    // Filter CLSID
    L"Bouncing Ball",       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    1,                      // Number pins
    &sudOpPin               // Pin details
};


// COM global table of objects in this dll

CFactoryTemplate g_Templates[] = {
  { L"Bouncing Ball"
  , &CLSID_BouncingBall
  , CBouncingBall::CreateInstance
  , NULL
  , &sudBallax }
};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

//
// DllRegisterServer
//
// Exported entry points for registration and unregistration
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2(TRUE);

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2(FALSE);

} // DllUnregisterServer


//
// DllEntryPoint
//
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);

BOOL APIENTRY DllMain(HANDLE hModule, 
                      DWORD  dwReason, 
                      LPVOID lpReserved)
{
	return DllEntryPoint((HINSTANCE)(hModule), dwReason, lpReserved);
}

//
// CreateInstance
//
// The only allowed way to create Bouncing balls!
//
CUnknown * WINAPI CBouncingBall::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    ASSERT(phr);

    CUnknown *punk = new CBouncingBall(lpunk, phr);
    if(punk == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;
    }
    return punk;

} // CreateInstance


//
// Constructor
//
// Initialise a CBallStream object so that we have a pin.
//
CBouncingBall::CBouncingBall(LPUNKNOWN lpunk, HRESULT *phr) :
    CSource(NAME("Bouncing ball"), lpunk, CLSID_BouncingBall)
{
    ASSERT(phr);
    CAutoLock cAutoLock(&m_cStateLock);

    m_paStreams = (CSourceStream **) new CBallStream*[1];
    if(m_paStreams == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;

        return;
    }

    m_paStreams[0] = new CBallStream(phr, this, L"A Bouncing Ball!");
    if(m_paStreams[0] == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;

        return;
    }

} // (Constructor)


//
// Constructor
//
CBallStream::CBallStream(HRESULT *phr,
                         CBouncingBall *pParent,
                         LPCWSTR pPinName) :
    CSourceStream(NAME("Bouncing Ball"),phr, pParent, pPinName),
    m_iImageWidth(320),
    m_iImageHeight(240),
    m_iDefaultRepeatTime(20)
{
    ASSERT(phr);
    CAutoLock cAutoLock(&m_cSharedState);

    m_Ball = new CBall(m_iImageWidth, m_iImageHeight);
    if(m_Ball == NULL)
    {
        if(phr)
            *phr = E_OUTOFMEMORY;
    }

} // (Constructor)


//
// Destructor
//
CBallStream::~CBallStream()
{
    CAutoLock cAutoLock(&m_cSharedState);
    if(m_Ball)
        delete m_Ball;

} // (Destructor)


//
// FillBuffer
//
// Plots a ball into the supplied video buffer
//
HRESULT CBallStream::FillBuffer(IMediaSample *pms)
{
    CheckPointer(pms,E_POINTER);
    ASSERT(m_Ball);

    BYTE *pData;
    long lDataLen;

    pms->GetPointer(&pData);
    lDataLen = pms->GetSize();

    ZeroMemory(pData, lDataLen);
    {
        CAutoLock cAutoLockShared(&m_cSharedState);

        // If we haven't just cleared the buffer delete the old
        // ball and move the ball on

        m_Ball->MoveBall(m_rtSampleTime - (LONG) m_iRepeatTime);
        m_Ball->PlotBall(pData, m_BallPixel, m_iPixelSize);

        // The current time is the sample's start
        CRefTime rtStart = m_rtSampleTime;

        // Increment to find the finish time
        m_rtSampleTime += (LONG)m_iRepeatTime;

        pms->SetTime((REFERENCE_TIME *) &rtStart,(REFERENCE_TIME *) &m_rtSampleTime);
    }

    pms->SetSyncPoint(TRUE);
    return NOERROR;

} // FillBuffer


//
// Notify
//
// Alter the repeat rate according to quality management messages sent from
// the downstream filter (often the renderer).  Wind it up or down according
// to the flooding level - also skip forward if we are notified of Late-ness
//
STDMETHODIMP CBallStream::Notify(IBaseFilter * pSender, Quality q)
{
    // Adjust the repeat rate.
    if(q.Proportion<=0)
    {
        m_iRepeatTime = 1000;        // We don't go slower than 1 per second
    }
    else
    {
        m_iRepeatTime = m_iRepeatTime*1000 / q.Proportion;
        if(m_iRepeatTime>1000)
        {
            m_iRepeatTime = 1000;    // We don't go slower than 1 per second
        }
        else if(m_iRepeatTime<10)
        {
            m_iRepeatTime = 10;      // We don't go faster than 100/sec
        }
    }

    // skip forwards
    if(q.Late > 0)
        m_rtSampleTime += q.Late;

    return NOERROR;

} // Notify


//
// GetMediaType
//
// I _prefer_ 5 formats - 8, 16 (*2), 24 or 32 bits per pixel and
// I will suggest these with an image size of 320x240. However
// I can accept any image size which gives me some space to bounce.
//
// A bit of fun:
//      8 bit displays get red balls
//      16 bit displays get blue
//      24 bit see green
//      And 32 bit see yellow
//
// Prefered types should be ordered by quality, zero as highest quality
// Therefore iPosition =
// 0    return a 32bit mediatype
// 1    return a 24bit mediatype
// 2    return 16bit RGB565
// 3    return a 16bit mediatype (rgb555)
// 4    return 8 bit palettised format
// (iPosition > 4 is invalid)
//
HRESULT CBallStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    if(iPosition < 0)
    {
        return E_INVALIDARG;
    }

    // Have we run off the end of types?

    if(iPosition > 4)
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if(NULL == pvi)
        return(E_OUTOFMEMORY);

    ZeroMemory(pvi, sizeof(VIDEOINFO));

    switch(iPosition)
    {
        case 0:
        {    
            // Return our highest quality 32bit format

            // since we use RGB888 (the default for 32 bit), there is
            // no reason to use BI_BITFIELDS to specify the RGB
            // masks. Also, not everything supports BI_BITFIELDS

            SetPaletteEntries(Yellow);
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 32;
            break;
        }

        case 1:
        {   // Return our 24bit format

            SetPaletteEntries(Green);
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 24;
            break;
        }

        case 2:
        {       
            // 16 bit per pixel RGB565

            // Place the RGB masks as the first 3 doublewords in the palette area
            for(int i = 0; i < 3; i++)
                pvi->TrueColorInfo.dwBitMasks[i] = bits565[i];

            SetPaletteEntries(Blue);
            pvi->bmiHeader.biCompression = BI_BITFIELDS;
            pvi->bmiHeader.biBitCount    = 16;
            break;
        }

        case 3:
        {   // 16 bits per pixel RGB555

            // Place the RGB masks as the first 3 doublewords in the palette area
            for(int i = 0; i < 3; i++)
                pvi->TrueColorInfo.dwBitMasks[i] = bits555[i];

            SetPaletteEntries(Blue);
            pvi->bmiHeader.biCompression = BI_BITFIELDS;
            pvi->bmiHeader.biBitCount    = 16;
            break;
        }

        case 4:
        {   // 8 bit palettised

            SetPaletteEntries(Red);
            pvi->bmiHeader.biCompression = BI_RGB;
            pvi->bmiHeader.biBitCount    = 8;
            pvi->bmiHeader.biClrUsed        = iPALETTE_COLORS;
            break;
        }
    }

    // (Adjust the parameters common to all formats...)

    // put the optimal palette in place
    for(int i = 0; i < iPALETTE_COLORS; i++)
    {
        pvi->TrueColorInfo.bmiColors[i].rgbRed      = m_Palette[i].peRed;
        pvi->TrueColorInfo.bmiColors[i].rgbBlue     = m_Palette[i].peBlue;
        pvi->TrueColorInfo.bmiColors[i].rgbGreen    = m_Palette[i].peGreen;
        pvi->TrueColorInfo.bmiColors[i].rgbReserved = 0;
    }

    pvi->bmiHeader.biSize       = sizeof(BITMAPINFOHEADER);
    pvi->bmiHeader.biWidth      = m_iImageWidth;
    pvi->bmiHeader.biHeight     = m_iImageHeight;
    pvi->bmiHeader.biPlanes     = 1;
    pvi->bmiHeader.biSizeImage  = GetBitmapSize(&pvi->bmiHeader);
    pvi->bmiHeader.biClrImportant = 0;

    SetRectEmpty(&(pvi->rcSource)); // we want the whole image area rendered.
    SetRectEmpty(&(pvi->rcTarget)); // no particular destination rectangle

    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

    // Work out the GUID for the subtype from the header info.
    const GUID SubTypeGUID = GetBitmapSubtype(&pvi->bmiHeader);
    pmt->SetSubtype(&SubTypeGUID);
    pmt->SetSampleSize(pvi->bmiHeader.biSizeImage);

    return NOERROR;

} // GetMediaType


//
// CheckMediaType
//
// We will accept 8, 16, 24 or 32 bit video formats, in any
// image size that gives room to bounce.
// Returns E_INVALIDARG if the mediatype is not acceptable
//
HRESULT CBallStream::CheckMediaType(const CMediaType *pMediaType)
{
    CheckPointer(pMediaType,E_POINTER);

    if((*(pMediaType->Type()) != MEDIATYPE_Video) ||   // we only output video
       !(pMediaType->IsFixedSize()))                   // in fixed size samples
    {                                                  
        return E_INVALIDARG;
    }

    // Check for the subtypes we support
    const GUID *SubType = pMediaType->Subtype();
    if (SubType == NULL)
        return E_INVALIDARG;

    if((*SubType != MEDIASUBTYPE_RGB8)
        && (*SubType != MEDIASUBTYPE_RGB565)
        && (*SubType != MEDIASUBTYPE_RGB555)
        && (*SubType != MEDIASUBTYPE_RGB24)
        && (*SubType != MEDIASUBTYPE_RGB32))
    {
        return E_INVALIDARG;
    }

    // Get the format area of the media type
    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();

    if(pvi == NULL)
        return E_INVALIDARG;

    // Check the image size. As my default ball is 10 pixels big
    // look for at least a 20x20 image. This is an arbitary size constraint,
    // but it avoids balls that are bigger than the picture...

    if((pvi->bmiHeader.biWidth < 20) || ( abs(pvi->bmiHeader.biHeight) < 20))
    {
        return E_INVALIDARG;
    }

    // Check if the image width & height have changed
    if(pvi->bmiHeader.biWidth != m_Ball->GetImageWidth() || 
       abs(pvi->bmiHeader.biHeight) != m_Ball->GetImageHeight())
    {
        // If the image width/height is changed, fail CheckMediaType() to force
        // the renderer to resize the image.
        return E_INVALIDARG;
    }


    return S_OK;  // This format is acceptable.

} // CheckMediaType


//
// DecideBufferSize
//
// This will always be called after the format has been sucessfully
// negotiated. So we have a look at m_mt to see what size image we agreed.
// Then we can ask for buffers of the correct size to contain them.
//
HRESULT CBallStream::DecideBufferSize(IMemAllocator *pAlloc,
                                      ALLOCATOR_PROPERTIES *pProperties)
{
    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);

    CAutoLock cAutoLock(m_pFilter->pStateLock());
    HRESULT hr = NOERROR;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
    pProperties->cBuffers = 1;
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;

    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if(FAILED(hr))
    {
        return hr;
    }

    // Is this allocator unsuitable

    if(Actual.cbBuffer < pProperties->cbBuffer)
    {
        return E_FAIL;
    }

    // Make sure that we have only 1 buffer (we erase the ball in the
    // old buffer to save having to zero a 200k+ buffer every time
    // we draw a frame)

    ASSERT(Actual.cBuffers == 1);
    return NOERROR;

} // DecideBufferSize


//
// SetMediaType
//
// Called when a media type is agreed between filters
//
HRESULT CBallStream::SetMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    // Pass the call up to my base class

    HRESULT hr = CSourceStream::SetMediaType(pMediaType);

    if(SUCCEEDED(hr))
    {
        VIDEOINFO * pvi = (VIDEOINFO *) m_mt.Format();
        if (pvi == NULL)
            return E_UNEXPECTED;

        switch(pvi->bmiHeader.biBitCount)
        {
            case 8:     // Make a red pixel

                m_BallPixel[0] = 10;    // 0 is palette index of red
                m_iPixelSize   = 1;
                SetPaletteEntries(Red);
                break;

            case 16:    // Make a blue pixel

                m_BallPixel[0] = 0xf8;  // 00000000 00011111 is blue in rgb555 or rgb565
                m_BallPixel[1] = 0x0;   // don't forget the byte ordering within the mask word.
                m_iPixelSize   = 2;
                SetPaletteEntries(Blue);
                break;

            case 24:    // Make a green pixel

                m_BallPixel[0] = 0x0;
                m_BallPixel[1] = 0xff;
                m_BallPixel[2] = 0x0;
                m_iPixelSize   = 3;
                SetPaletteEntries(Green);
                break;

            case 32:    // Make a yellow pixel

                m_BallPixel[0] = 0x0;
                m_BallPixel[1] = 0xff;
                m_BallPixel[2] = 0xff;
                m_BallPixel[3] = 0x00;
                m_iPixelSize   = 4;
                SetPaletteEntries(Yellow);
                break;

            default:
                // We should never agree any other pixel sizes
                ASSERT(FALSE);
                break;
        }

        CBall *pNewBall = new CBall(pvi->bmiHeader.biWidth, abs(pvi->bmiHeader.biHeight));

        if(pNewBall)
        {
            delete m_Ball;
            m_Ball = pNewBall;
        }
        else
            hr = E_OUTOFMEMORY;

        return NOERROR;
    } 

    return hr;

} // SetMediaType


//
// OnThreadCreate
//
// As we go active reset the stream time to zero
//
HRESULT CBallStream::OnThreadCreate()
{
    CAutoLock cAutoLockShared(&m_cSharedState);
    m_rtSampleTime = 0;

    // we need to also reset the repeat time in case the system
    // clock is turned off after m_iRepeatTime gets very big
    m_iRepeatTime = m_iDefaultRepeatTime;

    return NOERROR;

} // OnThreadCreate


//
// SetPaletteEntries
//
// If we set our palette to the current system palette + the colours we want
// the system has the least amount of work to do whilst plotting our images,
// if this stream is rendered to the current display. The first non reserved
// palette slot is at m_Palette[10], so put our first colour there. Also
// guarantees that black is always represented by zero in the frame buffer
//
HRESULT CBallStream::SetPaletteEntries(Colour color)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    HDC hdc = GetDC(NULL);  // hdc for the current display.
    UINT res = GetSystemPaletteEntries(hdc, 0, iPALETTE_COLORS, (LPPALETTEENTRY) &m_Palette);
    ReleaseDC(NULL, hdc);

    if(res == 0)
        return E_FAIL;

    switch(color)
    {
        case Red:
            m_Palette[10].peBlue  = 0;
            m_Palette[10].peGreen = 0;
            m_Palette[10].peRed   = 0xff;
            break;

        case Yellow:
            m_Palette[10].peBlue  = 0;
            m_Palette[10].peGreen = 0xff;
            m_Palette[10].peRed   = 0xff;
            break;

        case Blue:
            m_Palette[10].peBlue  = 0xff;
            m_Palette[10].peGreen = 0;
            m_Palette[10].peRed   = 0;
            break;

        case Green:
            m_Palette[10].peBlue  = 0;
            m_Palette[10].peGreen = 0xff;
            m_Palette[10].peRed   = 0;
            break;
    }

    m_Palette[10].peFlags = 0;
    return NOERROR;

} // SetPaletteEntries


