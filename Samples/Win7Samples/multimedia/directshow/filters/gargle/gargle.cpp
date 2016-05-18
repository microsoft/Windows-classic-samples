//------------------------------------------------------------------------------
// File: Gargle.cpp
//
// Desc: DirectShow sample code - "gargle" filter, a transform filter
//       that turns humans in to daleks!!!.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------


//
// Gargle Filter:  What This Sample Illustrates
//
// An in-place transform filter.
// Saving and restoring properties in a saved graph.
// Handling decompressed sound.
// Use of debug macros ASSERT, DbgBreak, DbgLog, DbgBreakPoint
//
//
//     Summary
//
// A simple, in-place transform, audio effect which modifies the data
// in the samples that pass through it.  The effect is an amplitude
// modulation with a synthesised secondary wave function.
// The secondary wave can be a triangular or square wave.  A properties
// sheet allows the shape and frequency of the secondary wave to be chosen.
//
// At low modulation frequencies it sounds like a tremolo, at higher
// modulation frequencies it sounds like a distortion, adding extra
// frequencies above and below the original unmodulated sound.
//
//
//     Demonstration instructions
//
// Start GraphEdit, which is available in the SDK DXUtils folder. Drag and drop
// onto the tool any WAV, MPEG, AVI or MOV file which has a sound track.
// A graph will be built to render the file.  From the Graph menu, select
// "Insert filters" and insert Gargle.  (If it is not on the list then you
// failed to register it properly.  If it fails to load then you either
// didn't build it properly or the registration does not correctly point to
// the path where gargle.ax is now found.)
// In the graph displayed, find the audio renderer.  Disconnect it (click on
// the incoming arrow and then press the Delete key).
// Create a connection between the same two filters via gargle.  (Drag from
// previous arrow tail pin to gargle input pin.  Drag from Gargle output pin to
// audio renderer input pin).
// Right click on the Gargle box to bring up the property page.  Click on
// the GraphEdit Play button and experiment with different properties.
//
//
//     Implementation
//
// This filter has one input pin, one output pin and
// does its transform in-place (i.e. without copying the data)
// on the push thread (i.e. it is called with a buffer, which it
// transforms and gives to the next filter downstream.  It is
// then blocked until that filter returns.  It then returns
// to its own caller.)
//
// The filter modulates sound by multiplying the value of each sample
// by the value of triangular or square waveform.  Depending on
// the frequency of the modulation it will sound like recurent fading,
// like a tremolo or like a sort of distortion.
//
// It has a properties page which allows control of two properties
// (the frequency and shape of the modulating waveform).  It exports a
// private interface (IGargle) which the properties page uses to get or
// set the frequency and/or shape.
//
// As far as possible the properties page code has been separated and
// is implemented in the files GargProp.* whereas the basic filter
// code is in this file and its header gargle.h.
//
// The word "sample" is used in two senses.  It means either a sound sample
// which is 8 or 16 bits of data representing the instantaneous sound pressure
// or else it means an DirectShow sample which is the unit of data that is
// passed between filters, i.e. a buffer full of sound samples.
//
//
//    Known problems ("features NOT illustrated by this sample"):
//
// 1. The properties sheet does NOT give real-time control.
//    Moving the knob immediately changes the modulation being applied
//    to the samples being processed by the filter, but there can then
//    be a long latency before those data are rendered.  (To avoid audio
//    break-up the audio renderer typically keeps a sizeable queue of
//    samples ready to be played).
//
// 2. Because it operates by alterating the data in the samples
//    it cannot work on a read-only stream.  As defined here
//    it refuses such a connection (e.g. it cannot connect directly
//    to the output of the "infinite tee" filter).
//    In such a case the intelligent graph-building code will often
//    make the connection by inserting an "ACM wrapper" filter.
//    This works but is rather inefficient.
//
//
//      Files
//
// gargle.cpp    This file - main implementation
// gargprop.h    Class definition of properties class (used in gargle.cpp)
// gargprop.cpp  Implementation of the properties sheet
// gargprop.rc   Defines the property page dialog
// resource.h    constants shared between gargprop.rc and gargprop.cpp
// igargle.h     Interface between gargle and gargprop
// garguids.h    The public class ids (only referred to in gargle.cpp)
// gargle.def    Imports and exports
//
//
//     Base classes used (refer to docs for diagram of what they inherit):
//
// CTransInPlaceFilter
// CPersistStream
// CBasePropertyPage

//=============================================================================
//=============================================================================

#include <streams.h>

// Eliminate two expected level 4 warnings from the Microsoft compiler.
// The class does not have an assignment or copy operator, and so cannot
// be passed by value.  This is normal.  This file compiles clean at the
// highest (most picky) warning level (-W4).
#pragma warning(disable: 4511 4512)


#include <initguid.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif
#include "garguids.h"             // Our own uuids
#include "igargle.h"              // IGargle (properties)
#include "gargprop.h"             // CGargleProperties


//------------------------------------------------------------------------
// CGargle - the gargle filter class
//------------------------------------------------------------------------

class CGargle
    // Inherited classes
    : public CTransInPlaceFilter       // Main DirectShow interfaces

    , public ISpecifyPropertyPages     // Needed for properties only

    , public IGargle                   // Needed for properties only.
                                       // Without this the PURE virtual
                                       // functions in IGargle will not
                                       // be implemented.  (If they ever
                                       // got called the entire app would
                                       // silently ExitProcess!!).

    , public CPersistStream            // Implements IPersistStream
                                       // to alow saving of properties
                                       // in a saved graph.
{

public:

    static CUnknown * WINAPI CreateInstance(LPUNKNOWN punk, HRESULT *phr);

    DECLARE_IUNKNOWN;

    //
    // --- CTransInPlaceFilter Overrides --
    //

    HRESULT CheckInputType(const CMediaType *mtIn);

    // Basic COM - used here to reveal our property interface.
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid, void ** ppv);

    // CPersistStream overrides
    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();
    STDMETHODIMP GetClassID(CLSID *pClsid);

    // --- ISpecifyPropertyPages ---

    // return our property pages
    STDMETHODIMP GetPages(CAUUID * pPages);

    // IGargle - private interface to put/set properties
    STDMETHODIMP get_GargleRate(int *GargleRate);
    STDMETHODIMP put_GargleRate(int GargleRate);
    STDMETHODIMP put_DefaultGargleRate(void);
    STDMETHODIMP put_GargleShape(int iGargleShape);
    STDMETHODIMP get_GargleShape(int *GargleShape);

private:

    // Constructor
    CGargle(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr);

    // Overrides the PURE virtual Transform of CTransInPlaceFilter base class
    // This is where the "real work" is done.
    HRESULT Transform(IMediaSample *pSample);

    // This is where the real work is really done (called from Transform)
    void MessItAbout(PBYTE pb, int cb);

    // Overrides a CTransformInPlace function.  Called as part of connecting.
    virtual HRESULT SetMediaType(PIN_DIRECTION direction, const CMediaType *pmt);

    // If there are multiple instances of this filter active, it's
    // useful for debug messages etc. to know which one this is.
    // This variable has no other purpose.
    static int m_nInstanceCount;                   // total instances
    int m_nThisInstance;

    const int           m_DefaultGargleRate;   // The default rate (Hz)
    int                 m_GargleRate;          // The current rate (Hz)
    int                 m_Shape;               // 0==triangle, 1==square
    int                 m_SamplesPerSec;       // Current sample format
    int                 m_BytesPerSample;      // Current sample format
    int                 m_Channels;            // Current sample format
    int                 m_Phase;               // See MessItAbout in gargle.cpp
    CCritSec            m_GargleLock;          // To serialise access.

}; // class CGargle


//------------------------------------------------------------------------
// Implementation
//------------------------------------------------------------------------


// Put out the name of a function and instance on the debugger.
// Invoke this at the start of functions to allow a trace.
#define DbgFunc(a) DbgLog(( LOG_TRACE                        \
                          , 2                                \
                          , TEXT("CGargle(Instance %d)::%s") \
                          , m_nThisInstance                  \
                          , TEXT(a)                          \
                         ));


// Self-registration data structures

const AMOVIESETUP_MEDIATYPE
sudPinTypes =   { &MEDIATYPE_Audio        // clsMajorType
                , &MEDIASUBTYPE_NULL };   // clsMinorType

const AMOVIESETUP_PIN
psudPins[] = { { L"Input"            // strName
               , FALSE               // bRendered
               , FALSE               // bOutput
               , FALSE               // bZero
               , FALSE               // bMany
               , &CLSID_NULL         // clsConnectsToFilter
               , L"Output"           // strConnectsToPin
               , 1                   // nTypes
               , &sudPinTypes        // lpTypes
               }
             , { L"Output"           // strName
               , FALSE               // bRendered
               , TRUE                // bOutput
               , FALSE               // bZero
               , FALSE               // bMany
               , &CLSID_NULL         // clsConnectsToFilter
               , L"Input"            // strConnectsToPin
               , 1                   // nTypes
               , &sudPinTypes        // lpTypes
               }
             };

const AMOVIESETUP_FILTER
sudGargle = { &CLSID_Gargle                   // class id
            , L"Gargle"                       // strName
            , MERIT_DO_NOT_USE                // dwMerit
            , 2                               // nPins
            , psudPins                        // lpPin
            };

// Needed for the CreateInstance mechanism
CFactoryTemplate g_Templates[2]= { { L"Gargle"
                                   , &CLSID_Gargle
                                   , CGargle::CreateInstance
                                   , NULL
                                   , &sudGargle
                                   }
                                 , { L"Gargle Property Page"
                                   , &CLSID_GargProp
                                   , CGargleProperties::CreateInstance
                                   }
                                 };

int g_cTemplates = sizeof(g_Templates)/sizeof(g_Templates[0]);

// initialise the static instance count.
int CGargle::m_nInstanceCount = 0;



//
// CGargle::Constructor
//
// Construct a CGargle object.
//
CGargle::CGargle(TCHAR *tszName, LPUNKNOWN punk, HRESULT *phr)
    : CTransInPlaceFilter (tszName, punk, CLSID_Gargle, phr)
    , CPersistStream(punk, phr)
    , m_DefaultGargleRate (DefaultGargleRate)
    , m_GargleRate        (DefaultGargleRate)
    , m_SamplesPerSec     (0)
    , m_BytesPerSample    (0)
    , m_Channels          (0)
    , m_Phase             (0)
    , m_Shape             (0)
{
    m_nThisInstance = ++m_nInstanceCount; // Useful for debug, no other purpose

    DbgFunc("CGargle");

} // (CGargle constructor)


//
// CreateInstance
//
// Override CClassFactory method.
// Provide the way for COM to create a CGargle object.
//
CUnknown * WINAPI CGargle::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    ASSERT(phr);
    
    CGargle *pNewObject = new CGargle(NAME("Gargle Filter"), punk, phr);
    if (pNewObject == NULL) {
        if (phr)
            *phr = E_OUTOFMEMORY;
    }

    return pNewObject;

} // CreateInstance



//
// NonDelegatingQueryInterface
//
// Override CUnknown method.
// Reveal our persistent stream, property pages and IGargle interfaces.
// Anyone can call our private interface so long as they know the private UUID.
//
STDMETHODIMP CGargle::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IGargle) {
        return GetInterface((IGargle *) this, ppv);

    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);

    } else if (riid == IID_IPersistStream) {
        return GetInterface((IPersistStream *) this, ppv);

    } else {
        // Pass the buck
        return CTransInPlaceFilter::NonDelegatingQueryInterface(riid, ppv);
    }

} // NonDelegatingQueryInterface


// GetClassID
//
// Override CBaseMediaFilter method for interface IPersist
// Part of the persistent file support.  We must supply our class id
// which can be saved in a graph file and used on loading a graph with
// a gargle in it to instantiate this filter via CoCreateInstance.
//
STDMETHODIMP CGargle::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid,E_POINTER);
    
    *pClsid = CLSID_Gargle;
    return NOERROR;

} // GetClassID


//
// SizeMax
//
// Override CPersistStream method.
// State the maximum number of bytes we would ever write in a file
// to save our properties.
//
int CGargle::SizeMax()
{
    // When an int is expanded as characters it takes at most 12 characters
    // including a trailing delimiter.
    // Wide chars doubles this and we want two ints.
    //
    return 48;

}  // SizeMax


//
// WriteToStream
//
// Override CPersistStream method.
// Write our properties to the stream.
//
HRESULT CGargle::WriteToStream(IStream *pStream)
{
    HRESULT hr;

    hr = WriteInt(pStream, m_GargleRate);
    if (FAILED(hr)) 
        return hr;

    hr = WriteInt(pStream, m_Shape);
    if (FAILED(hr)) 
        return hr;

    return NOERROR;

} // WriteToStream


//
// ReadFromStream
//
// Override CPersistStream method.
// Read our properties from the stream.
//
HRESULT CGargle::ReadFromStream(IStream *pStream)
{
    HRESULT hr;

    m_GargleRate = ReadInt(pStream, hr);
    if (FAILED(hr)) 
        return hr;

    m_Shape = ReadInt(pStream, hr);
    if (FAILED(hr)) 
        return hr;

    return NOERROR;

} // ReadFromStream


//
// MessItAbout
//
// Mess the sound about by modulating it with a waveform.
// We know the frequency of the modulation (from the slider setting
// which we were told through our internal interface, IGargle, and
// which we stored in m_GargleRate).  At the end of the call we
// record what part of the waveform we finished at in m_Phase and
// we resume at that point next time.
// Uses and updates m_Phase
// Uses m_SamplesPerSec, m_Channels, m_GargleRate, m_Shape
//
void CGargle::MessItAbout(PBYTE pb, int cb)
{
    CAutoLock foo(&m_GargleLock);

    // We know how many samples per sec and how
    // many channels so we can calculate the modulation period in samples.
    //
    int Period = (m_SamplesPerSec * m_Channels) / m_GargleRate;

    while (cb > 0) 
    {
       --cb;

       // If m_Shape is 0 (triangle) then we multiply by a triangular waveform
       // that runs 0..Period/2..0..Period/2..0... else by a square one that
       // is either 0 or Period/2 (same maximum as the triangle) or zero.
       //
       {
           // m_Phase is the number of samples from the start of the period.
           // We keep this running from one call to the next,
           // but if the period changes so as to make this more
           // than Period then we reset to 0 with a bang.  This may cause
           // an audible click or pop (but, hey! it's only a sample!)
           //
           ++m_Phase;

           if (m_Phase>Period) 
               m_Phase = 0;

           int M = m_Phase;      // m is what we modulate with

           if (m_Shape ==0 ) {   // Triangle
               if (M>Period/2) M = Period-M;  // handle downslope
           } else {             // Square wave
               if (M<=Period/2) M = Period/2; else M = 0;
           }

           if (m_BytesPerSample==1) 
           {
               // 8 bit sound uses 0..255 representing -128..127
               // Any overflow, even by 1, would sound very bad.
               // so we clip paranoically after modulating.
               // I think it should never clip by more than 1
               //
               int i = *pb-128;               // sound sample, zero based

               i = (i*M*2)/Period;            // modulate
               if (i>127)  i = 127;           // clip
               if (i<-128) i = -128;

               *pb = (unsigned char)(i+128);  // reset zero offset to 128

           } 
           else if (m_BytesPerSample==2) 
           {
               // 16 bit sound uses 16 bits properly (0 means 0)
               // We still clip paranoically
               //
               short int *psi = (short int *)pb;
               int i = *psi;                  // in a register, we might hope

               i = (i*M*2)/Period;            // modulate
               if (i>32767)  i = 32767;        // clip
               if (i<-32768) i = -32768;

               *psi = (short)i;
               ++pb;  // nudge it on another 8 bits here to get a 16 bit step
               --cb;  // and nudge the count too.

           } else {

               DbgBreak("Too many bytes per sample");
               // just leave it alone!

           }

       }
       ++pb;   // move on 8 bits to next sound sample
    }

} // MessItAbout


//
// Transform
//
// Override CTransInPlaceFilter method.
// Convert the input sample into the output sample.
//
HRESULT CGargle::Transform(IMediaSample *pSample)
{
    CheckPointer(pSample,E_POINTER);   
    DbgFunc("Transform");

    // Get the details of the data (address, length)
    //
    BYTE *pSampleBuffer;
    int iSize = pSample->GetActualDataLength();
    pSample->GetPointer(&pSampleBuffer);

    // Actually transform the data
    //
    MessItAbout(pSampleBuffer, iSize );

    return NOERROR;

} // Transform


//
// CheckInputType
//
// Override CTransformFilter method.
// Part of the Connect process.
// Ensure that we do not get connected to formats that we can't handle.
// We only work for wave audio, 8 or 16 bit, uncompressed.
//
HRESULT CGargle::CheckInputType(const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);

    DisplayType(TEXT("CheckInputType"), pmt);

    WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmt->pbFormat;

    // Reject non-Audio types.
    //
    if (pmt->majortype != MEDIATYPE_Audio) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    // Reject invalid format blocks
    //
    if (pmt->formattype != FORMAT_WaveFormatEx)
        return VFW_E_TYPE_NOT_ACCEPTED;

    // Reject compressed audio
    //
    if (pwfx->wFormatTag != WAVE_FORMAT_PCM) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    // Accept only 8 or 16 bit
    //
    if (pwfx->wBitsPerSample!=8 && pwfx->wBitsPerSample!=16) {
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    return NOERROR;

} // CheckInputType


//
// SetMediaType
//
// Override CTransformFilter method.
// Called when a connection attempt has succeeded. If the output pin
// is being connected and the input pin's media type does not agree then we
// reconnect the input (thus allowing its media type to change,) and vice versa.
//
HRESULT CGargle::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    CheckPointer(pmt,E_POINTER);
    DbgFunc("SetMediaType");

    // Record what we need for doing the actual transform

    WAVEFORMATEX *pwfx = (WAVEFORMATEX *) pmt->Format();
    m_Channels =  pwfx->nChannels;
    m_SamplesPerSec = pwfx->nSamplesPerSec;

    // Ignored: pwfx->nAvgBytesPerSec;
    // Ignored: pwfx->nBlockAlign;
    m_BytesPerSample = pwfx->wBitsPerSample/8;

    // Call the base class to do its thing
    CTransInPlaceFilter::SetMediaType(direction, pmt);

    // Reconnect where necessary.
    if( m_pInput->IsConnected() && m_pOutput->IsConnected() )
    {
        FILTER_INFO fInfo;

        QueryFilterInfo( &fInfo );

        if (direction == PINDIR_OUTPUT && *pmt != m_pInput->CurrentMediaType() )
            fInfo.pGraph->Reconnect( m_pInput );

        QueryFilterInfoReleaseGraph( fInfo );

        ASSERT(!(direction == PINDIR_INPUT && *pmt != m_pOutput->CurrentMediaType()));
    }

    return NOERROR;

} // SetMediaType


// ==============Implementation of the private IGargle interface ==========
// ==================== needed to support the property page ===============


//
// get_GargleRate
//
// Set *GargleRate to our current rate (Hz)
//
STDMETHODIMP CGargle::get_GargleRate(int *pGargleRate)
{
    CheckPointer(pGargleRate,E_POINTER);
    CAutoLock foo(&m_GargleLock);

    *pGargleRate = m_GargleRate;

    DbgLog((LOG_TRACE, 1, TEXT("get_GargleRate: %d"), *pGargleRate));

    return NOERROR;

} // get_GargleRate




//
// put_GargleRate
//
// Set the current rate from GargleRate.
//
STDMETHODIMP CGargle::put_GargleRate(int GargleRate)
{
    CAutoLock foo(&m_GargleLock);

    m_GargleRate = GargleRate;
    CPersistStream::SetDirty(TRUE);             // Need to scribble

    DbgLog((LOG_TRACE, 1, TEXT("put_GargleRate: %x"), m_GargleRate));

    return NOERROR;

} // put_GargleRate


//
// put_DefaultGargleRate
//
// Set the current gargle rate to the default
//
STDMETHODIMP CGargle::put_DefaultGargleRate(void)
{
    CAutoLock foo(&m_GargleLock);

    DbgLog((LOG_TRACE, 1, TEXT("put_DefaultGargleRate")));

    m_GargleRate = m_DefaultGargleRate;
    CPersistStream::SetDirty(TRUE);                     // Need to scribble

    return NOERROR;

} // put_DefaultGargleRate


//
// put_GargleShape
//
// Alter the waveform between triangle and square
//
STDMETHODIMP CGargle::put_GargleShape(int iGargleShape)
{
    if (iGargleShape<0 || iGargleShape>1)
        return E_INVALIDARG;

    m_Shape = iGargleShape;
    CPersistStream::SetDirty(TRUE);                     // Need to scribble

    return NOERROR;
} // put_GargleShape


//
// get_GargleShape
//
// Return 0 if the current shape is triangle, 1 if it's square
//
STDMETHODIMP CGargle::get_GargleShape(int *pGargleShape)
{
    CheckPointer(pGargleShape,E_POINTER);

    *pGargleShape = m_Shape;
    return NOERROR;

} // get_GargleShape


// ==============Implementation of the IPropertypages Interface ===========

//
// GetPages
//
STDMETHODIMP CGargle::GetPages(CAUUID * pPages)
{
    CheckPointer(pPages,E_POINTER);

    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) {
        return E_OUTOFMEMORY;
    }

    *(pPages->pElems) = CLSID_GargProp;

    return NOERROR;

} // GetPages


////////////////////////////////////////////////////////////////////////
//
// Exported entry points for registration and unregistration 
// (in this case they only call through to default implementations).
//
////////////////////////////////////////////////////////////////////////

STDAPI DllRegisterServer()
{
  return AMovieDllRegisterServer2( TRUE );
}


STDAPI DllUnregisterServer()
{
  return AMovieDllRegisterServer2( FALSE );
}

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

#pragma warning(disable: 4514) // "unreferenced inline function has been removed"



