//------------------------------------------------------------------------------
// File: SampleCGB.cpp
//
// Desc: DirectShow sample code - Sample capture graph builder class
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "stdafx.h"


#ifndef __STREAMS__
#define __STREAMS__
#endif

#include "ks.h"
#include "ksproxy.h"

#include  "SampleCGB.h"


//
//  ISampleCaptureGraphBuilder is trying to connect the video pin to the MPEG2 demux.
//  The possible configurations are:
//
//
//
// 1. capture filter- > audio / video encoder & multiplexer -> MPEG2 demux
//
// ----------------------         ---------------------------------------    ---------------
// |                   v|   --->  |                                     |    |             |
// | Capture Filter     |         | Audio / video encoder & Multiplexer |  ->| MPEG2 demux |
// |                   a|   --->  |                                     |    |             |
// ----------------------         ---------------------------------------    ---------------    
//
//
//
//
//
//                        -> audio encoder -> 
//  2. capture filter                           multiplexer -> MPEG2 demux
//                        -> video encoder ->   
//
//  -----------------------     -----------------    ----------------     ---------------   
//  |                    a|  -> | audio encoder | -> |              |     |             |
//  |                     |     -----------------    |              |     |             |
//  | capture filter      |                          |  multiplexer |  -> | MPEG2 demux |
//  |                     |     -----------------    |              |     |             |
//  |                    v|  -> | video encoder | -> |              |     |             |
//  -----------------------     -----------------    ----------------     ---------------
//
//
//
//
//
//                      a
//  3.  capture filter      - audio & video encoder -> multiplexer -> MPEG2 demux
//                      v
//
//  -----------------------     -----------------    ----------------     ---------------   
//  |                    a|  -> | audio encoder | -> |              |     |             |
//  |                     |     |               |    |              |     |             |
//  | capture filter      |     |               |    |  multiplexer |  -> | MPEG2 demux |
//  |                     |     |               |    |              |     |             |
//  |                    v|  -> | video encoder | -> |              |     |             |
//  -----------------------     -----------------    ----------------     ---------------
//
//
//
//
//  4. capture filter -> MPEG2 demux
//
// ------------------------------       ----------------
// |                   MPEG2 PS | --->  |              |
// | Capture Filter             |       | MPEG2 demux  |
// |                            |       |              |
// ------------------------------       ----------------    
//
//


//
//  How the algorithm works: 
//      1. the video pin doesn't stream MPEG2    
//          1. tries to connect the pin to an encoder   
//          2. tries to connect the encoder directly to the MPEG2 demux
//          3. if not possible, tries to find a multiplexor that can be connected to the 
//             encoder and MPEG2 demux
//          4. connect audio pin to the MPEG2 demux using the same algorithm as in 
//             video pin case
//           
//      2. if pin streams MPEG2 PS, then connect it to the MPEG2 demux
//      3. program the MPEG2 demux
//      4. render the video and the audio pin from the MPEG2 demux      
//      


static 
BYTE
Mpeg2ProgramVideo [] = {
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcSource.left              = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcSource.top               = 0x00000000
    0xD0, 0x02, 0x00, 0x00,                         //  .hdr.rcSource.right             = 0x000002d0
    0xE0, 0x01, 0x00, 0x00,                         //  .hdr.rcSource.bottom            = 0x000001e0
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.left              = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.top               = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.right             = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.rcTarget.bottom            = 0x00000000
    0x00, 0x09, 0x3D, 0x00,                         //  .hdr.dwBitRate                  = 0x003d0900
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwBitErrorRate             = 0x00000000
    0x63, 0x17, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, //  .hdr.AvgTimePerFrame            = 0x0000000000051763
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwInterlaceFlags           = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwCopyProtectFlags         = 0x00000000
    0x04, 0x00, 0x00, 0x00,                         //  .hdr.dwPictAspectRatioX         = 0x00000004
    0x03, 0x00, 0x00, 0x00,                         //  .hdr.dwPictAspectRatioY         = 0x00000003
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwReserved1                = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.dwReserved2                = 0x00000000
    0x28, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biSize           = 0x00000028
    0xD0, 0x02, 0x00, 0x00,                         //  .hdr.bmiHeader.biWidth          = 0x000002d0
    0xE0, 0x01, 0x00, 0x00,                         //  .hdr.bmiHeader.biHeight         = 0x00000000
    0x00, 0x00,                                     //  .hdr.bmiHeader.biPlanes         = 0x0000
    0x00, 0x00,                                     //  .hdr.bmiHeader.biBitCount       = 0x0000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biCompression    = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biSizeImage      = 0x00000000
    0xD0, 0x07, 0x00, 0x00,                         //  .hdr.bmiHeader.biXPelsPerMeter  = 0x000007d0
    0x27, 0xCF, 0x00, 0x00,                         //  .hdr.bmiHeader.biYPelsPerMeter  = 0x0000cf27
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biClrUsed        = 0x00000000
    0x00, 0x00, 0x00, 0x00,                         //  .hdr.bmiHeader.biClrImportant   = 0x00000000
    0x98, 0xF4, 0x06, 0x00,                         //  .dwStartTimeCode                = 0x0006f498
    0x56, 0x00, 0x00, 0x00,                         //  .cbSequenceHeader               = 0x00000056
    0x02, 0x00, 0x00, 0x00,                         //  .dwProfile                      = 0x00000002
    0x02, 0x00, 0x00, 0x00,                         //  .dwLevel                        = 0x00000002
    0x00, 0x00, 0x00, 0x00,                         //  .Flags                          = 0x00000000
                                                    //  .dwSequenceHeader [1]
    0x00, 0x00, 0x01, 0xB3, 0x2D, 0x01, 0xE0, 0x24,
    0x09, 0xC4, 0x23, 0x81, 0x10, 0x11, 0x11, 0x12, 
    0x12, 0x12, 0x13, 0x13, 0x13, 0x13, 0x14, 0x14, 
    0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x15, 0x15, 
    0x15, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 
    0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 
    0x18, 0x18, 0x18, 0x19, 0x18, 0x18, 0x18, 0x19, 
    0x1A, 0x1A, 0x1A, 0x1A, 0x19, 0x1B, 0x1B, 0x1B, 
    0x1B, 0x1B, 0x1C, 0x1C, 0x1C, 0x1C, 0x1E, 0x1E, 
    0x1E, 0x1F, 0x1F, 0x21, 0x00, 0x00, 0x01, 0xB5, 
    0x14, 0x82, 0x00, 0x01, 0x00, 0x00
} ;


static
BYTE
MPEG1AudioFormat [] = {
    0x50, 0x00, 0x02, 0x00, 0x80, 0xBB, 0x00, 0x00,
    0x00, 0x7D, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00,
    0x16, 0x00, 0x02, 0x00, 0x00, 0xE8, 0x03, 0x00,
    0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x1C, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
} ;



HRESULT 
ISampleCaptureGraphBuilder::AllocCapFile( LPCOLESTR lpwstr, DWORDLONG dwlSize )
{
    return graphBuilder2_->AllocCapFile( lpwstr, dwlSize );
}


HRESULT 
ISampleCaptureGraphBuilder::ControlStream( const GUID *pCategory,
                                          const GUID *pType,
                                          IBaseFilter *pFilter,
                                          REFERENCE_TIME *pstart,
                                          REFERENCE_TIME *pstop,
                                          WORD wStartCookie,
                                          WORD wStopCookie )
{
    return graphBuilder2_->ControlStream( pCategory, pType, pFilter, 
                                          pstart, pstop, wStartCookie, wStopCookie );
}


HRESULT 
ISampleCaptureGraphBuilder::CopyCaptureFile(  LPOLESTR lpwstrOld,
                                              LPOLESTR lpwstrNew,
                                              int fAllowEscAbort,
                                              IAMCopyCaptureFileProgress *pCallback)
{
    return graphBuilder2_->CopyCaptureFile( lpwstrOld, lpwstrNew, 
                                            fAllowEscAbort, pCallback );
}

HRESULT ISampleCaptureGraphBuilder::FindInterface(const GUID *pCategory,
                                                  const GUID *pType,
                                                  IBaseFilter *pf,
                                                  REFIID riid,
                                                  void **ppint
                                                  )
{
    return graphBuilder2_->FindInterface( pCategory, pType, pf, riid, ppint );
}

HRESULT 
ISampleCaptureGraphBuilder::FindPin( IUnknown *pSource,
                                      PIN_DIRECTION pindir,
                                      const GUID *pCategory,
                                      const GUID *pType,
                                      BOOL fUnconnected,
                                      int num,
                                      IPin **ppPin)
{
    return graphBuilder2_->FindPin( pSource, pindir, pCategory, pType, 
                                    fUnconnected, num, ppPin ); 
}


HRESULT ISampleCaptureGraphBuilder::GetFiltergraph( IGraphBuilder **ppfg )
{
    return graphBuilder2_->GetFiltergraph( ppfg );
}

HRESULT 
ISampleCaptureGraphBuilder::RenderStream( const GUID *pCategory,
                                          const GUID *pType,
                                          IUnknown *pSource,
                                          IBaseFilter *pIntermediate,
                                          IBaseFilter *pSink)
{
    if( !pType ||  !::IsEqualGUID( MEDIATYPE_Stream, *pType ) )
    {
        return graphBuilder2_->RenderStream( pCategory, pType, pSource, 
                                             pIntermediate, pSink );
    }

    
    HRESULT hr;
    if( !graph_ )
    {
        hr = GetFiltergraph( &graph_ );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    //
    //  try to build MPEG2 graph
    //
    SmartPtr< IBaseFilter > captureFilter;

    hr = pSource->QueryInterface( & captureFilter );
    if( FAILED( hr ) )
    {
        return E_INVALIDARG;
    }

    hr = BuildMPEG2Segment( captureFilter);
    if( pSink || FAILED( hr ) )
    {
        return hr;
    }

    hr = ConfigureMPEG2Demux( pMPEG2Demux_ );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = RenderStream(NULL, &MEDIATYPE_Video, pMPEG2Demux_, NULL, NULL );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = RenderStream(NULL, &MEDIATYPE_Audio, pMPEG2Demux_, NULL, NULL );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return S_OK;
}


HRESULT 
ISampleCaptureGraphBuilder::SetFiltergraph( IGraphBuilder *pfg )
{
    return graphBuilder2_->SetFiltergraph( pfg );
}


HRESULT 
ISampleCaptureGraphBuilder::SetOutputFileName(  const GUID *pType,
                                                LPCOLESTR lpwstrFile,
                                                IBaseFilter **ppf,
                                                IFileSinkFilter **pSink )
{
    if( ! pType || ! lpwstrFile || !ppf || !pSink )
    {
        return E_INVALIDARG;
    }

    if( !::IsEqualGUID( *pType, MEDIASUBTYPE_Mpeg2 ) )
    {
        return graphBuilder2_->SetOutputFileName(pType, lpwstrFile, ppf, pSink );
    }

    HRESULT hr;
    if( !graph_ )
    {
        hr = GetFiltergraph( &graph_ );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    //
    //  Configure the dump filter
    //
    SmartPtr< IFileSinkFilter > pDump;
    hr = CoCreateInstance(CLSID_Dump, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pDump);
    if( FAILED( hr ) )
    {
        return hr;
    }
    
    hr = pDump->SetFileName( lpwstrFile, NULL );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pDump.QueryInterface( &pMPEG2Demux_ );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = graph_->AddFilter( pMPEG2Demux_, L"Dump" );
    if( FAILED( hr ) )
    {
        pMPEG2Demux_ = NULL;
        return hr;
    }

    
    *pSink = pDump;
    return S_OK;
}



//
//  A device can stream directly MPEG2 stream,
//  or it can be linked with a video codec and multiplexer
//
//

//
//  Loop through every media type supported by this pin
//  to see if there is one which can be considered MPEG2
//
BOOL ISampleCaptureGraphBuilder::IsMPEG2Pin( IPin *pPin )
{
    if( !pPin )
    {
        return FALSE;   // NULL pointer
    }
    
    SmartPtr<IEnumMediaTypes> pMediaTypes;
    HRESULT hr = pPin->EnumMediaTypes( &pMediaTypes );
    if( FAILED( hr ) )
    {
        return FALSE;
    }

    hr = pMediaTypes->Reset();
    if( FAILED( hr ) )
    {
        return FALSE;
    }

    ULONG           fetched;
    AM_MEDIA_TYPE   *mediaType;
    while( S_OK == pMediaTypes->Next( 1, &mediaType, &fetched ) )
    {
        if( 
            (
                ::IsEqualGUID( mediaType->majortype, MEDIATYPE_Video ) ||
                ::IsEqualGUID( mediaType->majortype, MEDIATYPE_Stream ) 
            )
            &&
            (
                ::IsEqualGUID( mediaType->subtype, MEDIASUBTYPE_MPEG2_VIDEO ) ||
                ::IsEqualGUID( mediaType->subtype,  MEDIASUBTYPE_MPEG2_PROGRAM )
            )
        )
        {
            DeleteMediaType( mediaType );
            return TRUE;
        }
        DeleteMediaType( mediaType );
    }

    return FALSE;
}

BOOL ISampleCaptureGraphBuilder::IsVideoPin( IPin *pPin )
{
    return HasMediaType( pPin, MEDIATYPE_Video );
}



HRESULT ISampleCaptureGraphBuilder::GetEncodersByCategory( IEnumMoniker **ppEncoders )
{
    SmartPtr<ICreateDevEnum> pDeviceEnum;
        
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDeviceEnum);
    if( FAILED( hr ) )
    {
        return hr;
    }

    return pDeviceEnum->CreateClassEnumerator( KSCATEGORY_ENCODER, ppEncoders, 0 );
}



HRESULT 
ISampleCaptureGraphBuilder::GetEncodersByEnumerating(
                                        IPin *pPin,   
                                        const REGPINMEDIUM& pinMedium,
                                        IEnumMoniker **ppEncoders )
{
    SmartPtr<IFilterMapper2> pFilterMapper2;

    HRESULT hr = CoCreateInstance(CLSID_FilterMapper2, NULL, CLSCTX_INPROC_SERVER, IID_IFilterMapper2, (void**)&pFilterMapper2);
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pFilterMapper2->EnumMatchingFilters( 
                                ppEncoders, 
                                NULL, 
                                FALSE, 
                                0,          //any merit
                                TRUE, 
                                0, 
                                NULL, 
                                &pinMedium, 
                                NULL, 
                                FALSE, 
                                TRUE, 
                                0, 
                                NULL, 
                                NULL, 
                                NULL );


    return hr;
}


//
//  looks for an MPEG2 pin
//
HRESULT 
ISampleCaptureGraphBuilder::FindMPEG2Pin( 
                        IBaseFilter *pFilter, 
                        IPin **ppPin )
{
    if( !pFilter )
    {
        return E_POINTER;
    }


    SmartPtr<IEnumPins> pEnumPins;
    HRESULT hr = pFilter->EnumPins( &pEnumPins );
    if( FAILED( hr ) )
    {
        return hr;
    }
    
    SmartPtr<IPin>   pTempPin;
    ULONG           fetched;
    PIN_DIRECTION   dir;

    hr = pEnumPins->Reset( );
    while( pTempPin.Release(), S_OK == pEnumPins->Next( 1, &pTempPin, &fetched ) )
    {
        hr = pTempPin->QueryDirection( &dir );
        if( FAILED( hr ) || PINDIR_INPUT == dir )
        {
            continue;
        }
        if( IsMPEG2Pin( pTempPin ) )
        {
            (*ppPin) = pTempPin.Detach();
            return S_OK;
        }
    }
    return E_FAIL;
}

//
//  search the encoder that has this special medium
//  video == TRUE -- look for a video pin
//  video == FALSE -- look for a audio pin  
//
HRESULT ISampleCaptureGraphBuilder::FindPin( 
            IBaseFilter *pFilter, 
            const REGPINMEDIUM& regPinMedium, 
            PIN_DIRECTION direction, 
            BOOL video,             
            IPin **ppPin)
{
    if( !pFilter )
    {
        return E_POINTER;
    }

    SmartPtr<IEnumPins> pEnumPins;
    HRESULT hr = pFilter->EnumPins( &pEnumPins );
    if( FAILED( hr ) )
    {
        return hr;
    }
    
    SmartPtr<IPin>   pTempPin;
    ULONG           fetched;
    REGPINMEDIUM    regPinMediumTemp;
    PIN_DIRECTION   dir;

    hr = pEnumPins->Reset( );
    while( pTempPin.Release(), S_OK == pEnumPins->Next( 1, &pTempPin, &fetched ) )
    {
        ASSERT( pTempPin );

        hr = pTempPin->QueryDirection( &dir );
        if( FAILED( hr ) || dir != direction )
        {
            continue;
        }

        hr = GetMedium( pTempPin, regPinMediumTemp );
        if( FAILED( hr ) )
        {
            continue;
        }

        if( !IsVideoPin( pTempPin ) )
        {
            continue;
        }

        if( ::IsEqualGUID( regPinMediumTemp.clsMedium, regPinMedium.clsMedium  ) &&
            regPinMediumTemp.dw1 == regPinMedium.dw1 )
        {
            (*ppPin) = pTempPin.Detach();
            return S_OK;
        }
    }

    return E_FAIL;
}

//
//  Get a special medium from this pin.
//  If there is not one, return GUID_NULL.
//  Returns the first one it finds special
//

HRESULT 
ISampleCaptureGraphBuilder::GetMedium( 
                        IPin *pPin, 
                        REGPINMEDIUM& regPinMedium )
{
    if( !pPin )
    {
        return E_POINTER;
    }

    SmartPtr<IKsPin> pKsPin;
    HRESULT hr = pPin->QueryInterface(IID_IKsPin, (void**)&pKsPin );
    if( FAILED( hr ) )
    {
        return hr;
    }

    PKSMULTIPLE_ITEM pmi;
    hr = pKsPin->KsQueryMediums( &pmi );
    if( FAILED( hr ) )
    {
        return hr;
    }
    
    REGPINMEDIUM *pMedium = (REGPINMEDIUM *)(pmi + 1);
    for( ULONG i  = 0; i < pmi->Count; i++ )
    {
        if( !::IsEqualGUID( pMedium->clsMedium, GUID_NULL ) &&
            !::IsEqualGUID( pMedium->clsMedium, KSMEDIUMSETID_Standard )
        )
        {
            regPinMedium.clsMedium = pMedium->clsMedium;
            regPinMedium.dw1 = pMedium->dw1;
            regPinMedium.dw2 = pMedium->dw2;
            CoTaskMemFree( pmi );
            return S_OK;
        }
    }

    regPinMedium.clsMedium = GUID_NULL;
    regPinMedium.dw1 = 0;
    regPinMedium.dw2 = 0;
    CoTaskMemFree( pmi );
    return S_OK;
}



//
//  Adds the MPEG2 demux and renders 
//  the audio and video pin until the end (until the renderers)
//

HRESULT ISampleCaptureGraphBuilder::AddMPEG2Demux( )
{
    if( pMPEG2Demux_ )
    {
        //
        //  Instead of a MPEG2 demux there is a 
        //  dump filter in which the file will be dumped
        //
        return S_OK;
    }

    HRESULT hr = CoCreateInstance(CLSID_MPEG2Demultiplexer, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&pMPEG2Demux_);
	if( FAILED( hr ) )
    {
        return hr;
    }
    return graph_->AddFilter( static_cast<IBaseFilter *>( pMPEG2Demux_ ), L"MPEG2 Demux" );
}



HRESULT 
ISampleCaptureGraphBuilder::FindVideoPin( 
                                IBaseFilter *pFilter, 
                                IPin **ppPin  )
{
    if( !pFilter )
    {
        return E_POINTER;
    }

    SmartPtr<IEnumPins> pEnumPins;
    HRESULT hr = pFilter->EnumPins( &pEnumPins );
    if( FAILED( hr ) )
    {
        return hr;
    }

    SmartPtr<IPin>   pTempPin;
    ULONG           fetched;
    hr = pEnumPins->Reset( );

    while( pTempPin.Release(), S_OK == pEnumPins->Next( 1, &pTempPin, &fetched ) )
    {
        if( IsVideoPin( pTempPin ) )
        {
            (*ppPin) = pTempPin.Detach();
            return S_OK;
        }
    }
    return E_FAIL;
}

BOOL ISampleCaptureGraphBuilder::IsAudioPin( IPin *pPin )
{
    if( !pPin )
    {
        return FALSE;   // NULL pointer
    }

    return HasMediaType( pPin, MEDIATYPE_Audio) ;
}


BOOL ISampleCaptureGraphBuilder::HasMediaType(IPin *pPin,  REFGUID majorType )
{
    if( !pPin )
    {
        return FALSE;
    }

    SmartPtr<IEnumMediaTypes> pMediaTypes;
    HRESULT hr = pPin->EnumMediaTypes( &pMediaTypes );
    if( FAILED( hr ) )
    {
        return FALSE;
    }

    hr = pMediaTypes->Reset();
    if( FAILED( hr ) )
    {
        return FALSE;
    }

    ULONG           fetched;
    AM_MEDIA_TYPE   *mediaType;

    while( S_OK == pMediaTypes->Next( 1, &mediaType, &fetched ) )
    {
        if( ::IsEqualGUID( mediaType->majortype, majorType ) )
        {
            DeleteMediaType( mediaType );
            return TRUE;
        }
        DeleteMediaType( mediaType );
    }

    return FALSE;

}


HRESULT 
ISampleCaptureGraphBuilder::FindAudioPin( 
                                IBaseFilter *pFilter, 
                                IPin **ppPin  )
{
    if( !pFilter )
    {
        return E_POINTER;
    }

    SmartPtr<IEnumPins> pEnumPins;
    HRESULT hr = pFilter->EnumPins( &pEnumPins );
    if( FAILED( hr ) )
    {
        return hr;
    }

    SmartPtr<IPin>   pTempPin;
    ULONG           fetched;
    hr = pEnumPins->Reset( );

    while( pTempPin.Release(), S_OK == pEnumPins->Next( 1, &pTempPin, &fetched ) )
    {
        if( IsAudioPin( pTempPin ) )
        {
            (*ppPin) = pTempPin.Detach();
            return S_OK;
        }
    }
    return E_FAIL;
}



HRESULT ISampleCaptureGraphBuilder::FindEncoder( 
                IEnumMoniker *pEncoders, 
                REGPINMEDIUM pinMedium, 
                IBaseFilter **ppEncoder  )
{
    if( ! pEncoders )
    {
        return E_INVALIDARG;
    }

    if( IsEqualGUID( pinMedium.clsMedium, GUID_NULL ) ||
        IsEqualGUID( pinMedium.clsMedium, KSMEDIUMSETID_Standard ) )
    {
        return E_INVALIDARG;
    }

    HRESULT                 hr;
    SmartPtr<IBaseFilter>    pFilter;
    SmartPtr<IMoniker>       pMoniker;
    ULONG                   fetched;
    SmartPtr<IPin>           pPin;

    while( pFilter.Release(), pMoniker.Release(), 
           S_OK == pEncoders->Next( 1, &pMoniker, &fetched ) )
    {
        hr = pMoniker->BindToObject(
            0, 0, IID_IBaseFilter, reinterpret_cast<void **>( &pFilter ) );
        if( FAILED( hr ) )
        {
            continue;
        }

        hr = FindPin( pFilter, pinMedium, PINDIR_INPUT, TRUE, &pPin );
        if( SUCCEEDED( hr ) )
        {
            *ppEncoder = pFilter.Detach();
            return hr;
        }
    }

    return E_FAIL;
}



HRESULT 
ISampleCaptureGraphBuilder::RenderToMPEG2Demux( 
                            IPin *pPin, 
                            IEnumMoniker *pEncoders)
{
    if( !pPin || !pEncoders )
    {
        return E_INVALIDARG;
    }

    REGPINMEDIUM pinMedium ;
    pinMedium.clsMedium = GUID_NULL;
    pinMedium.dw1 = 0;
    pinMedium.dw2 = 0;

    SmartPtr<IBaseFilter> pFilter;
    SmartPtr<IMoniker>    pMoniker;
    ULONG                fetched;
    HRESULT              hr;

    while( pFilter.Release(), pMoniker.Release(), S_OK == pEncoders->Next( 1, &pMoniker, &fetched ) )
    {
        hr = pMoniker->BindToObject(
            0, 0, IID_IBaseFilter, reinterpret_cast<void **>( &pFilter ) );
        if( FAILED( hr ) )
        {
            continue;
        }

        hr = graph_->AddFilter( pFilter, L"Encoder" );
        if( FAILED( hr ) )
        {
            continue;
        }

        hr = ConnectPin( pPin, pFilter );
        if( FAILED( hr ) )
        {
            graph_->RemoveFilter( pFilter );
            continue;
        }

        hr = ConnectEncoderToMPEG2Demux( pFilter, pinMedium );
        if( SUCCEEDED( hr ) )
        {
            pEncoder_ = pFilter;
            return S_OK;
        }
        graph_->RemoveFilter( pFilter );
    }

    return E_FAIL;
}


//
//  Loop through every encoder available on system.
//  Look first after the one that has a special medium if
//  there is a special one on pPin.
//  Otherwise, try to render using ICaptureGrapBuilder2
//  and the encoder that matches this will be the chosen one/
//  If the encoder is found, then this will be rendered to the 
//  MPEG2 demux.
//

HRESULT 
    ISampleCaptureGraphBuilder::RenderToMPEG2Demux( 
        IPin *pPin, 
        const REGPINMEDIUM& pinMedium, 
        IEnumMoniker *pEncoders 
)
{
    //
    //  The pin has a special medium, 
    //  there shold be an encoder with the same 
    //  medium
    //
    SmartPtr< IBaseFilter > pEncoder; 
    HRESULT hr = FindEncoder( pEncoders, pinMedium, &pEncoder );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = graph_->AddFilter( pEncoder, L"Encoder" );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = ConnectPin( pPin, pEncoder );
    if( FAILED(  hr ) )
    {
        hr = graph_->RemoveFilter( pEncoder );
        return hr;
    }

    //
    //  the video pin was rendered to the same 
    //  ( hardware? ) encoder with the same mediu
    //
    hr = ConnectEncoderToMPEG2Demux( pEncoder, pinMedium );
    if( FAILED( hr ) )
    {
        hr = graph_->RemoveFilter( pEncoder );
        return hr;
    }

    pEncoder_ = pEncoder;
    return S_OK;
}



HRESULT 
ISampleCaptureGraphBuilder::RenderToMPEG2Demux( IPin *pPin )
{
    if( !pPin )
    {
        return E_INVALIDARG;
    }

    REGPINMEDIUM pinMedium;
    HRESULT hr = GetMedium( pPin, pinMedium );
    if( FAILED( hr ) )
    {
        return hr;
    }


    SmartPtr< IEnumMoniker > pEncoders;
    if( ::IsEqualGUID( pinMedium.clsMedium, GUID_NULL ) )
    {
        //
        //  Search throgh the codec category 
        //  
        hr = GetEncodersByCategory( &pEncoders );
        if( FAILED( hr ) )
        {
                return hr;
        }

        hr = RenderToMPEG2Demux( pPin, pEncoders );
        if( SUCCEEDED( hr ) )
        {
            return S_OK;
        }
    }
    else
    {
        //
        //  search through encoders category; identify
        //  the encoder using the medium
        //
        hr = GetEncodersByCategory( &pEncoders );
        if( FAILED( hr ) )
        {
            return hr;
        }

        hr = RenderToMPEG2Demux( pPin, pinMedium, pEncoders  );
        if( SUCCEEDED( hr ) )
        {
            return S_OK;
        }
        
        pEncoders = NULL;
        hr = GetEncodersByEnumerating( pPin, pinMedium, &pEncoders );
        if( FAILED( hr ) )
        {
            return hr;
        }

        hr = RenderToMPEG2Demux( pPin, pinMedium, pEncoders );
        if( FAILED( hr ) )
        {
            return hr;
        }
    }
    return S_OK;
}


//
//  pEncoder - the encoder to be connected using a multiplexer
//
//
HRESULT 
ISampleCaptureGraphBuilder::ConnectEncoderToMPEG2Demux( 
                IBaseFilter *pEncoder, 
                const REGPINMEDIUM& pinMedium )
{

    REGPINMEDIUM regPinMedium ;
    regPinMedium.clsMedium = GUID_NULL;
    regPinMedium.dw1 = 0;
    regPinMedium.dw2 = 0;
    //
    //  try a direct connection between 
    //  codec and MPEG2Demux
    //
    HRESULT hr = ConnectFilters( 
                        pEncoder, 
                        pMPEG2Demux_ );
    if( SUCCEEDED( hr ) )
    {
        return S_OK;
    }

    //
    //  no luck
    //  maybe I need a multiplexer 
    //
    SmartPtr< IEnumMoniker > pMultiplexers;
    hr = GetMultiplexersByCategory( &pMultiplexers );
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = ConnectMultiplexerToMPEG2Demux( pEncoder, pMultiplexers );
    if( SUCCEEDED( hr ) )
    {
        return S_OK;
    }

    if( FALSE == ::IsEqualGUID( pinMedium.clsMedium, GUID_NULL ) )
    {
        //
        //  get the multiplexers using IFilterMapper2
        //  assuming that the encoder and the multiplexer have the same medium
        //
        pMultiplexers = NULL;
        hr = GetMultiplexersByFilterMapper( &pMultiplexers, pinMedium );
        if( FAILED( hr ) )
        {
            return hr;
        }
        hr = ConnectMultiplexerToMPEG2Demux( pEncoder, pMultiplexers );
        if( SUCCEEDED( hr ) )
        {
            return S_OK;
        }
    }
    return E_FAIL;
}


HRESULT 
ISampleCaptureGraphBuilder::GetMultiplexersByCategory( IEnumMoniker **ppMultiplexers )
{
    SmartPtr<ICreateDevEnum> pDeviceEnum;
        
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&pDeviceEnum);
    if( FAILED( hr ) )
    {
        return hr;
    }

    return pDeviceEnum->CreateClassEnumerator( KSCATEGORY_MULTIPLEXER, ppMultiplexers, 0 );
}

HRESULT 
ISampleCaptureGraphBuilder::GetMultiplexersByFilterMapper( 
                                    IEnumMoniker **ppMultiplexers, 
                                    const REGPINMEDIUM& pinMedium  )
{
    return E_NOTIMPL;
}


HRESULT 
ISampleCaptureGraphBuilder::ConnectMultiplexerToMPEG2Demux( 
                                        IBaseFilter *pEncoder, 
                                        IEnumMoniker *pMultiplexers )
{
    if( !pEncoder || !pMultiplexers )
    {
        return E_INVALIDARG;
    }

    SmartPtr<IBaseFilter> pFilter;
    SmartPtr<IMoniker>   pMoniker;
    ULONG               fetched;
    HRESULT             hr;

    while( pFilter.Release(), pMoniker.Release(), 
           S_OK == pMultiplexers->Next( 1, &pMoniker, &fetched ) )
    {
        hr = pMoniker->BindToObject(
            0, 0, IID_IBaseFilter, reinterpret_cast<void **>( &pFilter ) );
        if( FAILED( hr ) )
        {
            continue;
        }

        hr = graph_->AddFilter( pFilter, NULL );
        if( FAILED( hr ) )
        {
            continue;
        }

        //
        //  connect the encoder to the multiplexer
        //
        hr = ConnectFilters( pEncoder, pFilter );
        if( FAILED( hr ) )
        {
            graph_->RemoveFilter( pFilter );
            continue;
        }

        //
        //  connect the multiplexer to the encoder
        //
        hr = ConnectFilters( pFilter, pMPEG2Demux_ );
        if( SUCCEEDED( hr ) )
        {
            pMultiplexer_ = pFilter;
            return S_OK;
        }

    }

    return E_FAIL;
}


HRESULT 
ISampleCaptureGraphBuilder::BuildMPEG2Segment(IBaseFilter *pFilter)
{

    if( ! pFilter )
    {
        return E_FAIL;
    }

    
    HRESULT hr = AddMPEG2Demux( );
    if( FAILED( hr ) )
    {
        return hr;
    }

    //
    //  Search a MPEG2 pin on the 
    //  filter
    //
    SmartPtr<IPin> pPin;
    hr = FindMPEG2Pin( pFilter, &pPin );
    if( SUCCEEDED( hr ) )
    {
        hr = ConnectPin( pPin, pMPEG2Demux_ );
        if( FAILED( hr ) )
        {
            graph_->RemoveFilter( pMPEG2Demux_ );
            return E_FAIL;
        }
        return S_OK;
    }

    //
    //  no pins that streams directly MPEG2 stream
    //
    hr = FindVideoPin( pFilter, &pPin );
    if( FAILED( hr ) )
    {
        graph_->RemoveFilter( pMPEG2Demux_ );
        return hr; // no video pin
    }

    hr = RenderToMPEG2Demux( pPin );
    if( FAILED( hr ) )
    {
        graph_->RemoveFilter( pMPEG2Demux_ );
        return hr;
    }

    SmartPtr<IPin> pAudioPin;
    hr = FindAudioPin( pFilter, &pAudioPin );
    if( FAILED( hr ) )
    {
        //
        //  don't bother with audio
        //
        return S_OK;
    }

    //
    //  try to connect the audio pin directly to encoder
    //  if this is not possible, then try to find an encoder
    //  and connect it to the multiplexer
    //
    ASSERT( pEncoder_ );
    hr = ConnectPin( pAudioPin, pEncoder_ );
    if( FAILED( hr ) )
    {
        hr = ConnectAudioPinToMultiplexer( pAudioPin, pMultiplexer_ );
    }

    return S_OK;
}


HRESULT 
ISampleCaptureGraphBuilder::ConnectAudioPinToMultiplexer( 
                                            IPin *pPin, 
                                            IBaseFilter *pMultiplexer)
{
    if( !pPin || !pMultiplexer )
    {
        return E_INVALIDARG;
    }

    REGPINMEDIUM pinMedium;
    HRESULT hr = GetMedium( pPin, pinMedium );
    if( FAILED( hr ) )
    {
        return hr;
    }

    SmartPtr<IBaseFilter> pEncoder;
    SmartPtr<IEnumMoniker> pEncoders;

    if( FALSE == ::IsEqualGUID( pinMedium.clsMedium, GUID_NULL ) )
    {
        //
        //  search through encoders category; identify
        //  the encoder using the medium
        //
        hr = GetEncodersByCategory( &pEncoders );
        if( FAILED( hr ) )
        {
            return hr;
        }

        hr = FindEncoder( pEncoders, pinMedium, &pEncoder );
        if( SUCCEEDED( hr ) )
        {
            hr = graph_->AddFilter( pEncoder, L"Audio Encoder" );
            if( SUCCEEDED( hr ) &&
                SUCCEEDED( ConnectPin( pPin, pEncoder ) ) &&
                SUCCEEDED( ConnectFilters( pEncoder, pMultiplexer ) )
            )
            {
                return S_OK;
            }
        }
        
        
        pEncoders = NULL;
        hr = GetEncodersByEnumerating( pPin, pinMedium, &pEncoders );
        if( FAILED( hr ) )
        {
            return hr;
        }

        hr = FindEncoder( pEncoders, pinMedium, &pEncoder );
        if( SUCCEEDED( hr ) )
        {
            hr = graph_->AddFilter( pEncoder, L"Audio Encoder" );
            if( SUCCEEDED( hr ) &&
                SUCCEEDED( ConnectPin( pPin, pEncoder ) ) &&
                SUCCEEDED( ConnectFilters( pEncoder, pMultiplexer ) )
            )
            {
                return S_OK;
            }
        }
        return E_FAIL;
    }


    //
    //  Search throgh the codec category 
    //  
    hr = GetEncodersByCategory( &pEncoders );
    if( FAILED( hr ) )
    {
        return hr;
    }


    SmartPtr<IBaseFilter>    pFilter;
    SmartPtr<IMoniker>       pMoniker;
    ULONG                   fetched;
    while( pFilter.Release(), pMoniker.Release(), S_OK == pEncoders->Next( 1, &pMoniker, &fetched ) )
    {
        hr = pMoniker->BindToObject(
            0, 0, IID_IBaseFilter, reinterpret_cast<void **>( &pFilter ) );
        if( FAILED( hr ) )
        {
            continue;
        }

        hr = graph_->AddFilter( pFilter, L"Audio Encoder" );
        if( FAILED( hr ) )
        {
            continue;
        }

        hr = ConnectPin( pPin, pFilter );
        if( FAILED( hr ) )
        {
            graph_->RemoveFilter( pFilter );
            continue;
        }

        hr = ConnectFilters( pFilter, pMultiplexer );
        if( SUCCEEDED( hr ) )
        {
            return S_OK;
        }
        graph_->RemoveFilter( pFilter );
    }

    return E_FAIL;

}


HRESULT 
ISampleCaptureGraphBuilder::CreateVideoPin(
        IMpeg2Demultiplexer *pIMpeg2Demux )
{
    if( !pIMpeg2Demux )
    {
        return E_INVALIDARG;
    }

    AM_MEDIA_TYPE amTypeVideo;
    amTypeVideo.majortype = MEDIATYPE_Video;
    amTypeVideo.subtype = MEDIASUBTYPE_MPEG2_VIDEO;
    amTypeVideo.bFixedSizeSamples = TRUE;
    amTypeVideo.bTemporalCompression = 0;
    amTypeVideo.formattype = FORMAT_MPEG2Video;
    amTypeVideo.pUnk = NULL;
    amTypeVideo.cbFormat = sizeof( Mpeg2ProgramVideo );
    amTypeVideo.pbFormat = Mpeg2ProgramVideo;

    //
    // Create video pin
    //

    SmartPtr<IPin> pVideoOutPin;
    HRESULT hr = pIMpeg2Demux->CreateOutputPin( &amTypeVideo, L"MpegVideo", &pVideoOutPin );
    if( FAILED( hr ) )
    {
        return hr;
    }


    SmartPtr<IMPEG2StreamIdMap> pIVideoPIDMap;
    hr = pVideoOutPin->QueryInterface( &pIVideoPIDMap );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pIVideoPIDMap->MapStreamId(VidPID_, MPEG2_PROGRAM_ELEMENTARY_STREAM , 0, 0);
    if( FAILED( hr ) )
    {
        return hr;
    }


#ifdef USE_VMR
    //
    //  Get the VMR interface and add it to the graph
    //
    SmartPtr<IBaseFilter> pVMR;
    hr = pVMR.CoCreateInstance( CLSID_VideoMixingRenderer );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = graph_->AddFilter( pVMR, L"VMR" );
    if( FAILED( hr ) )
    {
        return hr;
    }

    //
    //before rendering the VMR, make the number of streams 1
    //
    SmartPtr<IVMRFilterConfig> pConfig;
    hr = pVMR.QueryInterface( &pConfig );
    if( FAILED( hr ) )
    {
        return hr;
    }
    hr = pConfig->SetNumberOfStreams( 1 );
    if( FAILED( hr ) )
    {
        return hr;
    }


    //
    //  Get the input pin from the VMR
    //
    SmartPtr<IPin> pInputPin;
    hr = graphBuilder2_->FindPin(
            static_cast<IBaseFilter *>( pVMR ),  
            PINDIR_INPUT, 
            NULL, 
            NULL, 
            TRUE, 
            0, 
            &pInputPin
        );
    if( FAILED( hr ) )
    {

        hr = pIMpeg2Demux->DeleteOutputPin(L"MpegVideo");
        graph_->RemoveFilter( pVMR );
        return hr;
    }

    return graph_->Connect( pVideoOutPin, pInputPin );
#endif

    return hr;
}



HRESULT 
ISampleCaptureGraphBuilder::CreateAudioPin(
            IMpeg2Demultiplexer *pIMpeg2Demux 
    )
{
    if( !pIMpeg2Demux )
    {
        return E_INVALIDARG;
    }

    //
    // for audio: could be Mpeg1, Mpeg2, AC3: if Mpeg1 failed (connect failed) try Mpeg2.if failed tried AC3
    // Audio struct of AC3 can be copied from dev code.
    //
    AM_MEDIA_TYPE amTypeAudio;
    amTypeAudio.majortype   = MEDIATYPE_Audio;
    amTypeAudio.subtype     = MEDIASUBTYPE_MPEG2_AUDIO;
    amTypeAudio.bFixedSizeSamples = TRUE;
    amTypeAudio.bTemporalCompression = 0;
    amTypeAudio.formattype  = FORMAT_WaveFormatEx;
    amTypeAudio.pUnk        = NULL;
    amTypeAudio.cbFormat    = sizeof( MPEG1AudioFormat );
    amTypeAudio.pbFormat    = MPEG1AudioFormat;


    SmartPtr<IPin> pAudioOutPin;
    HRESULT hr = pIMpeg2Demux->CreateOutputPin(&amTypeAudio, L"MpegAudio", &pAudioOutPin);
    if( FAILED( hr ) )
    {
        return hr;
    }
    
    SmartPtr<IMPEG2StreamIdMap> pIAudioPIDMap;
    hr = pAudioOutPin->QueryInterface( &pIAudioPIDMap );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = pIAudioPIDMap->MapStreamId(AudPID_, MPEG2_PROGRAM_ELEMENTARY_STREAM, 0, 0);
    if( FAILED( hr ) )
    {
        return hr;
    }
    

    return hr;
}



HRESULT ISampleCaptureGraphBuilder::ConfigureMPEG2Demux( IBaseFilter *pFilter)
{
    
    if( ! pFilter )
    {
        return E_INVALIDARG;
    }

    //
    // Create video pin and render it
    //
    SmartPtr<IMpeg2Demultiplexer> pIMpeg2Demux;
    HRESULT hr = pFilter->QueryInterface(IID_IMpeg2Demultiplexer, (void**)&pIMpeg2Demux );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = CreateVideoPin( pIMpeg2Demux );
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr = CreateAudioPin( pIMpeg2Demux );
    if( FAILED( hr ) )
    {
        return hr;
    }

    return S_OK;
}


HRESULT 
ISampleCaptureGraphBuilder::ConnectFilters(IBaseFilter *pUpFilter, IBaseFilter *pDownFilter)
{
    if( !pUpFilter || !pDownFilter )
    {
        return E_INVALIDARG;
    }

    // All the need pin & pin enumerator pointers
    SmartPtr<IEnumPins>  pEnumUpFilterPins , 
                        pEnumDownFilterPins;

    SmartPtr<IPin>   pUpFilterPin , 
                    pDownFilterPin;

    HRESULT hr = S_OK;

    // Get the pin enumerators for both the filtera
    hr = pUpFilter->EnumPins(&pEnumUpFilterPins); 
    if( FAILED( hr ) )
    {
        return hr;
    }

    hr= pDownFilter->EnumPins(&pEnumDownFilterPins); 
    if( FAILED( hr ) )
    {
        return hr;
    }


    // Loop on every pin on the Upstream Filter
    BOOL bConnected = FALSE;
    PIN_DIRECTION pinDir;
    ULONG nFetched = 0;
    while(pUpFilterPin.Release( ), S_OK == pEnumUpFilterPins->Next(1, &pUpFilterPin, &nFetched) )
    {
        // Make sure that we have the output pin of the upstream filter
        hr = pUpFilterPin->QueryDirection( &pinDir );
        if( FAILED( hr ) || PINDIR_INPUT == pinDir )
        {
            continue;
        }

        //
        // I have an output pin; loop on every pin on the Downstream Filter
        //
        while(pDownFilterPin.Release( ), S_OK == pEnumDownFilterPins->Next(1, &pDownFilterPin, &nFetched) )
        {
            hr = pDownFilterPin->QueryDirection( &pinDir );
            if( FAILED( hr ) || PINDIR_OUTPUT == pinDir )
            {
                continue;
            }

            // Try to connect them and exit if u can else loop more until you can
            if(SUCCEEDED(graph_->ConnectDirect(pUpFilterPin, pDownFilterPin, NULL)))
            {
                bConnected = TRUE;
                break;
            }
        }

        hr = pEnumDownFilterPins->Reset();
        if( FAILED( hr ) )
        {
            return hr;
        }
    }

    if( !bConnected )
    {
        return E_FAIL;
    }

    return S_OK;
}


HRESULT 
ISampleCaptureGraphBuilder::ConnectPin( 
                    IPin *pPin, 
                    IBaseFilter *pDownFilter )
{
    if( !pPin || !pDownFilter )
    {
        return E_INVALIDARG;
    }

    PIN_DIRECTION   pinDirection;
    HRESULT hr = pPin->QueryDirection( &pinDirection );
    if( FAILED( hr ) || PINDIR_INPUT == pinDirection )
    {
        return E_FAIL;
    }

    //
    //  Add the filter to the graph
    //
    BOOL bConnected = FALSE;
    ULONG nFetched = 0;
    SmartPtr<IPin> pDownFilterPin;

    //
    //  Loop through every input pin from downstream filter
    //  and try to connect the pin
    //  
    SmartPtr< IEnumPins > pEnumDownFilterPins;
    hr= pDownFilter->EnumPins( &pEnumDownFilterPins ); 
    if( FAILED( hr ) )
    {
        return hr;
    }

    while(pDownFilterPin.Release( ), S_OK == pEnumDownFilterPins->Next(1, &pDownFilterPin, &nFetched) )
    {
        hr = pDownFilterPin->QueryDirection( &pinDirection );
        if( FAILED( hr ) )
        {
            continue;
        }
        if( PINDIR_OUTPUT == pinDirection )
        {
            continue;
        }

        hr = graph_->ConnectDirect(pPin, pDownFilterPin, NULL);
        if(SUCCEEDED(hr))
        {
            bConnected = TRUE;
            break;
        }
    }

    if( !bConnected )
    {
        graph_->RemoveFilter( pDownFilter );
        return E_FAIL;
    }

    return S_OK;
}
