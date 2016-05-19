//------------------------------------------------------------------------------
// File: EZRGB24.cpp
//
// Desc: DirectShow sample code - special effects image filter.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
//
// What this sample illustrates
//
// A special effects filter that does image filtering and colour changes. We
// work on decompressed true colour images and apply a range of effects. The
// effect we do and when it is applied to the video stream can be configured
// through a custom interface we implement and which our property page uses.
// We can also save/load our property settings by supporting IPersistStream
//
//
// Summary
//
// This is a sample special effects filter - it works only with RGB24 formats
// We can be inserted between video decoders and renderers and apply a number
// of different effects to the decompressed video stream. For example we can
// apply red, green or blue filters to the images. We also have posterise and
// greying effects amongst others. The effect can be applied to the stream at
// a given start and end media time, this is defined by using the IIPEffect
// interface and can be viewed by using the property page this filter supports
//
//
// Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. Drag and drop
// an MPEG, AVI or MOV file into the tool and it will be rendered. Then go to
// the video renderer box and disconnect its input pin from the filter it is
// connected to. Go to the Graph menu and select Insert Filters, from the
// dialog select the Special Effects filter and dismiss the dialog. back in
// the tool workspace connect the output of the decoder to the input of the
// sample filter and then connect the output of the sample to the input of
// the video renderer (GraphEdit may put a colour space convertor in between)
// Then press play and then right click on the sample to select a transform
// by default a transform will be applied from 2 to 7 seconds into the movie
//
//
// Files
//
// ezprop.cpp           A property page to control the video effects
// ezprop.h             Class definition for the property page object
// ezprop.rc            Dialog box template for the property page
// ezrgb24.cpp          Main filter code that does the special effects
// ezrgb24.def          What APIs we import and export from this DLL
// ezrgb24.h            Class definition for the special effects filter
// ezuids.h             Header file containing the filter CLSIDs
// iez.h                Defines the special effects custom interface
// makefile             How we build it...
// resource.h           Microsoft Visual C++ generated resource file
//
//
// Base classes used
//
// CTransformFilter     A transform filter with one input and output pin
// CPersistStream       Handles the grunge of supporting IPersistStream
//
//


#include <windows.h>
#include <streams.h>
#include <initguid.h>

#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif

#include "EZuids.h"
#include "iEZ.h"
#include "EZprop.h"
#include "EZrgb24.h"
#include "resource.h"


// Setup information

const AMOVIESETUP_MEDIATYPE sudPinTypes =
{
    &MEDIATYPE_Video,       // Major type
    &MEDIASUBTYPE_NULL      // Minor type
};

const AMOVIESETUP_PIN sudpPins[] =
{
    { L"Input",             // Pins string name
      FALSE,                // Is it rendered
      FALSE,                // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    },
    { L"Output",            // Pins string name
      FALSE,                // Is it rendered
      TRUE,                 // Is it an output
      FALSE,                // Are we allowed none
      FALSE,                // And allowed many
      &CLSID_NULL,          // Connects to filter
      NULL,                 // Connects to pin
      1,                    // Number of types
      &sudPinTypes          // Pin information
    }
};

const AMOVIESETUP_FILTER sudEZrgb24 =
{
    &CLSID_EZrgb24,         // Filter CLSID
    L"Image Effects (EZRGB24)",       // String name
    MERIT_DO_NOT_USE,       // Filter merit
    2,                      // Number of pins
    sudpPins                // Pin information
};


// List of class IDs and creator functions for the class factory. This
// provides the link between the OLE entry point in the DLL and an object
// being created. The class factory will call the static CreateInstance

CFactoryTemplate g_Templates[] = {
    { L"Image Effects"
    , &CLSID_EZrgb24
    , CEZrgb24::CreateInstance
    , NULL
    , &sudEZrgb24 }
  ,
    { L"Special Effects"
    , &CLSID_EZrgb24PropertyPage
    , CEZrgb24Properties::CreateInstance }
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
// Handles sample registry and unregistry
//
STDAPI DllRegisterServer()
{
    return AMovieDllRegisterServer2( TRUE );

} // DllRegisterServer


//
// DllUnregisterServer
//
STDAPI DllUnregisterServer()
{
    return AMovieDllRegisterServer2( FALSE );

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
// Constructor
//
CEZrgb24::CEZrgb24(TCHAR *tszName,
                   LPUNKNOWN punk,
                   HRESULT *phr) :
    CTransformFilter(tszName, punk, CLSID_EZrgb24),
    m_effect(IDC_RED),
    m_lBufferRequest(1),
    CPersistStream(punk, phr)
{
    char sz[60];

    GetProfileStringA("Quartz", "EffectStart", "2.0", sz, 60);
    m_effectStartTime = COARefTime(atof(sz));

    GetProfileStringA("Quartz", "EffectLength", "5.0", sz, 60);
    m_effectTime = COARefTime(atof(sz));

} // (Constructor)


//
// CreateInstance
//
// Provide the way for COM to create a EZrgb24 object
//
CUnknown *CEZrgb24::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    ASSERT(phr);
    
    CEZrgb24 *pNewObject = new CEZrgb24(NAME("Image Effects"), punk, phr);

    if (pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }
    return pNewObject;

} // CreateInstance


//
// NonDelegatingQueryInterface
//
// Reveals IIPEffect and ISpecifyPropertyPages
//
STDMETHODIMP CEZrgb24::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IIPEffect) {
        return GetInterface((IIPEffect *) this, ppv);

    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);

    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface


//
// Transform
//
// Copy the input sample into the output sample - then transform the output
// sample 'in place'. If we have all keyframes, then we shouldn't do a copy
// If we have cinepak or indeo and are decompressing frame N it needs frame
// decompressed frame N-1 available to calculate it, unless we are at a
// keyframe. So with keyframed codecs, you can't get away with applying the
// transform to change the frames in place, because you'll mess up the next
// frames decompression. The runtime MPEG decoder does not have keyframes in
// the same way so it can be done in place. We know if a sample is key frame
// as we transform because the sync point property will be set on the sample
//
HRESULT CEZrgb24::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    CheckPointer(pIn,E_POINTER);   
    CheckPointer(pOut,E_POINTER);   

    // Copy the properties across

    HRESULT hr = Copy(pIn, pOut);
    if (FAILED(hr)) {
        return hr;
    }

    // Check to see if it is time to do the sample

    CRefTime tStart, tStop ;
    hr = pIn->GetTime((REFERENCE_TIME *) &tStart, (REFERENCE_TIME *) &tStop);

    if (tStart >= m_effectStartTime) 
    {
        if (tStop <= (m_effectStartTime + m_effectTime)) 
        {
            return Transform(pOut);
        }
    }

    return NOERROR;

} // Transform


//
// Copy
//
// Make destination an identical copy of source
//
HRESULT CEZrgb24::Copy(IMediaSample *pSource, IMediaSample *pDest) const
{
    CheckPointer(pSource,E_POINTER);   
    CheckPointer(pDest,E_POINTER);   

    // Copy the sample data

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize = pSource->GetActualDataLength();

#ifdef DEBUG
    long lDestSize = pDest->GetSize();
    ASSERT(lDestSize >= lSourceSize);
#endif

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory( (PVOID) pDestBuffer,(PVOID) pSourceBuffer,lSourceSize);

    // Copy the sample times

    REFERENCE_TIME TimeStart, TimeEnd;
    if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
        pDest->SetTime(&TimeStart, &TimeEnd);
    }

    LONGLONG MediaStart, MediaEnd;
    if (pSource->GetMediaTime(&MediaStart,&MediaEnd) == NOERROR) {
        pDest->SetMediaTime(&MediaStart,&MediaEnd);
    }

    // Copy the Sync point property

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK) {
        pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetSyncPoint(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the media type

    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType(pMediaType);

    // Copy the preroll property

    hr = pSource->IsPreroll();
    if (hr == S_OK) {
        pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetPreroll(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the discontinuity property

    hr = pSource->IsDiscontinuity();
    if (hr == S_OK) {
    pDest->SetDiscontinuity(TRUE);
    }
    else if (hr == S_FALSE) {
        pDest->SetDiscontinuity(FALSE);
    }
    else {  // an unexpected error has occured...
        return E_UNEXPECTED;
    }

    // Copy the actual data length

    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);

    return NOERROR;

} // Copy


//
// Transform (in place)
//
// 'In place' apply the image effect to this sample
//
HRESULT CEZrgb24::Transform(IMediaSample *pMediaSample)
{
    BYTE *pData;                // Pointer to the actual image buffer
    long lDataLen;              // Holds length of any given sample
    unsigned int grey,grey2;    // Used when applying greying effects
    int iPixel;                 // Used to loop through the image pixels
    int temp,x,y;               // General loop counters for transforms
    RGBTRIPLE *prgb;            // Holds a pointer to the current pixel

    AM_MEDIA_TYPE* pType = &m_pInput->CurrentMediaType();
    VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pType->pbFormat;
    ASSERT(pvi);

    CheckPointer(pMediaSample,E_POINTER);
    pMediaSample->GetPointer(&pData);
    lDataLen = pMediaSample->GetSize();

    // Get the image properties from the BITMAPINFOHEADER

    int cxImage    = pvi->bmiHeader.biWidth;
    int cyImage    = pvi->bmiHeader.biHeight;
    int numPixels  = cxImage * cyImage;

    // int iPixelSize = pvi->bmiHeader.biBitCount / 8;
    // int cbImage    = cyImage * cxImage * iPixelSize;

    switch (m_effect)
    {
        case IDC_NONE: break;

        // Zero out the green and blue components to leave only the red
        // so acting as a filter - for better visual results, compute a
        // greyscale value for the pixel and make that the red component

        case IDC_RED:
                        
            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels; iPixel++, prgb++) {
                prgb->rgbtGreen = 0;
                prgb->rgbtBlue = 0;
            }
            break;
    
        case IDC_GREEN:

            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels; iPixel++, prgb++) {
                prgb->rgbtRed = 0;
                prgb->rgbtBlue = 0;
            }
            break;

        case IDC_BLUE:
            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels; iPixel++, prgb++) {
                prgb->rgbtRed = 0;
                prgb->rgbtGreen = 0;
            }
            break;

        // Bitwise shift each component to the right by 1
        // this results in the image getting much darker

        case IDC_DARKEN:
                        
            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels; iPixel++, prgb++) {
                prgb->rgbtRed   = (BYTE) (prgb->rgbtRed >> 1);
                prgb->rgbtGreen = (BYTE) (prgb->rgbtGreen >> 1);
                prgb->rgbtBlue  = (BYTE) (prgb->rgbtBlue >> 1);
            }
            break;

        // Toggle each bit - this gives a sort of X-ray effect

        case IDC_XOR:   
            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels; iPixel++, prgb++) {
                prgb->rgbtRed   = (BYTE) (prgb->rgbtRed ^ 0xff);
                prgb->rgbtGreen = (BYTE) (prgb->rgbtGreen ^ 0xff);
                prgb->rgbtBlue  = (BYTE) (prgb->rgbtBlue ^ 0xff);
            }
            break;

        // Zero out the five LSB per each component

        case IDC_POSTERIZE:
            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels; iPixel++, prgb++) {
                prgb->rgbtRed   = (BYTE) (prgb->rgbtRed & 0xe0);
                prgb->rgbtGreen = (BYTE) (prgb->rgbtGreen & 0xe0);
                prgb->rgbtBlue  = (BYTE) (prgb->rgbtBlue & 0xe0);
            }
            break;

        // Take pixel and its neighbor two pixels to the right and average
        // then out - this blurs them and produces a subtle motion effect

        case IDC_BLUR:
            prgb = (RGBTRIPLE*) pData;
            for (y = 0 ; y < pvi->bmiHeader.biHeight; y++) {
                for (x = 2 ; x < pvi->bmiHeader.biWidth; x++,prgb++) {
                    prgb->rgbtRed   = (BYTE) ((prgb->rgbtRed + prgb[2].rgbtRed) >> 1);
                    prgb->rgbtGreen = (BYTE) ((prgb->rgbtGreen + prgb[2].rgbtGreen) >> 1);
                    prgb->rgbtBlue  = (BYTE) ((prgb->rgbtBlue + prgb[2].rgbtBlue) >> 1);
                }
                prgb +=2;
            }
            break;

        // An excellent greyscale calculation is:
        //      grey = (30 * red + 59 * green + 11 * blue) / 100
        // This is a bit too slow so a faster calculation is:
        //      grey = (red + green) / 2

        case IDC_GREY:  
            prgb = (RGBTRIPLE*) pData;
            for (iPixel=0; iPixel < numPixels ; iPixel++, prgb++) {
                grey = (prgb->rgbtRed + prgb->rgbtGreen) >> 1;
                prgb->rgbtRed = prgb->rgbtGreen = prgb->rgbtBlue = (BYTE) grey;
            }
            break;

        // Really sleazy emboss - rather than using a nice 3x3 convulution
        // matrix, we compare the greyscale values of two neighbours. If
        // they are not different, then a mid grey (128, 128, 128) is
        // supplied.  Large differences get father away from the mid grey

        case IDC_EMBOSS:
            prgb = (RGBTRIPLE*) pData;
            for (y = 0 ; y < pvi->bmiHeader.biHeight; y++) 
            {
                grey2 = (prgb->rgbtRed + prgb->rgbtGreen) >> 1;
                prgb->rgbtRed = prgb->rgbtGreen = prgb->rgbtBlue = (BYTE) 128;
                prgb++;

                for (x = 1 ; x < pvi->bmiHeader.biWidth; x++) {
                    grey = (prgb->rgbtRed + prgb->rgbtGreen) >> 1;
                    temp = grey - grey2;
                    if (temp > 127) temp = 127;
                    if (temp < -127) temp = -127;
                    temp += 128;
                    prgb->rgbtRed = prgb->rgbtGreen = prgb->rgbtBlue = (BYTE) temp;
                    grey2 = grey;
                    prgb++;
                }
            }   
            break;
    }

    return NOERROR;

} // Transform (in place)


// Check the input type is OK - return an error otherwise

HRESULT CEZrgb24::CheckInputType(const CMediaType *mtIn)
{
    CheckPointer(mtIn,E_POINTER);

    // check this is a VIDEOINFOHEADER type

    if (*mtIn->FormatType() != FORMAT_VideoInfo) {
        return E_INVALIDARG;
    }

    // Can we transform this type

    if (CanPerformEZrgb24(mtIn)) {
        return NOERROR;
    }
    return E_FAIL;
}


//
// Checktransform
//
// Check a transform can be done between these formats
//
HRESULT CEZrgb24::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    CheckPointer(mtIn,E_POINTER);
    CheckPointer(mtOut,E_POINTER);

    if (CanPerformEZrgb24(mtIn)) 
    {
        if (*mtIn == *mtOut) 
        {
            return NOERROR;
        }
    }

    return E_FAIL;

} // CheckTransform


//
// DecideBufferSize
//
// Tell the output pin's allocator what size buffers we
// require. Can only do this when the input is connected
//
HRESULT CEZrgb24::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    CheckPointer(pAlloc,E_POINTER);
    CheckPointer(pProperties,E_POINTER);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_pInput->CurrentMediaType().GetSampleSize();
    ASSERT(pProperties->cbBuffer);

    // Ask the allocator to reserve us some sample memory, NOTE the function
    // can succeed (that is return NOERROR) but still not have allocated the
    // memory that we requested, so we must check we got whatever we wanted

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT( Actual.cBuffers == 1 );

    if (pProperties->cBuffers > Actual.cBuffers ||
            pProperties->cbBuffer > Actual.cbBuffer) {
                return E_FAIL;
    }
    return NOERROR;

} // DecideBufferSize


//
// GetMediaType
//
// I support one type, namely the type of the input pin
// This type is only available if my input is connected
//
HRESULT CEZrgb24::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    // Is the input pin connected

    if (m_pInput->IsConnected() == FALSE) {
        return E_UNEXPECTED;
    }

    // This should never happen

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

    // Do we have more items to offer

    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    CheckPointer(pMediaType,E_POINTER);
    *pMediaType = m_pInput->CurrentMediaType();

    return NOERROR;

} // GetMediaType


//
// CanPerformEZrgb24
//
// Check if this is a RGB24 true colour format
//
BOOL CEZrgb24::CanPerformEZrgb24(const CMediaType *pMediaType) const
{
    CheckPointer(pMediaType,FALSE);

    if (IsEqualGUID(*pMediaType->Type(), MEDIATYPE_Video)) 
    {
        if (IsEqualGUID(*pMediaType->Subtype(), MEDIASUBTYPE_RGB24)) 
        {
            VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *) pMediaType->Format();
            return (pvi->bmiHeader.biBitCount == 24);
        }
    }

    return FALSE;

} // CanPerformEZrgb24


#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
               if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
               if (FAILED(hr)) return hr;


//
// GetClassID
//
// This is the only method of IPersist
//
STDMETHODIMP CEZrgb24::GetClassID(CLSID *pClsid)
{
    return CBaseFilter::GetClassID(pClsid);

} // GetClassID


//
// ScribbleToStream
//
// Overriden to write our state into a stream
//
HRESULT CEZrgb24::ScribbleToStream(IStream *pStream)
{
    HRESULT hr;

    WRITEOUT(m_effect);
    WRITEOUT(m_effectStartTime);
    WRITEOUT(m_effectTime);

    return NOERROR;

} // ScribbleToStream


//
// ReadFromStream
//
// Likewise overriden to restore our state from a stream
//
HRESULT CEZrgb24::ReadFromStream(IStream *pStream)
{
    HRESULT hr;

    READIN(m_effect);
    READIN(m_effectStartTime);
    READIN(m_effectTime);

    return NOERROR;

} // ReadFromStream


//
// GetPages
//
// Returns the clsid's of the property pages we support
//
STDMETHODIMP CEZrgb24::GetPages(CAUUID *pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_EZrgb24PropertyPage;
    return NOERROR;

} // GetPages


//
// get_IPEffect
//
// Return the current effect selected
//
STDMETHODIMP CEZrgb24::get_IPEffect(int *IPEffect,REFTIME *pStart,REFTIME *pLength)
{
    CAutoLock cAutolock(&m_EZrgb24Lock);
    CheckPointer(IPEffect,E_POINTER);
    CheckPointer(pStart,E_POINTER);
    CheckPointer(pLength,E_POINTER);

    *IPEffect = m_effect;
    *pStart = COARefTime(m_effectStartTime);
    *pLength = COARefTime(m_effectTime);

    return NOERROR;

} // get_IPEffect


//
// put_IPEffect
//
// Set the required video effect
//
STDMETHODIMP CEZrgb24::put_IPEffect(int IPEffect,REFTIME start,REFTIME length)
{
    CAutoLock cAutolock(&m_EZrgb24Lock);

    m_effect = IPEffect;
    m_effectStartTime = COARefTime(start);
    m_effectTime = COARefTime(length);

    SetDirty(TRUE);
    return NOERROR;

} // put_IPEffect


