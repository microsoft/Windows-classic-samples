// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//------------------------------------------------------------------------------
// File: AppStream.cpp
//
// Desc: DirectShow sample code - implementation of CAppStream class.
//------------------------------------------------------------------------------

#include <windows.h>
#include <mmsystem.h>

#include <malloc.h>
#include <mediaobj.h>

#include "uuids.h"
#include "dmo.h"
#include "wave.h"
#include "appstream.h"
#include "dxutil.h"     //to use free memory macro

//
// CAppStream - reads data from a WAV file, transforms it using a DMO and then
//              copies the results into an output buffer.
//
//-----------------------------------------------------------------------------
// Name: CAppStream::CAppStream()
// Desc: Constructor
//-----------------------------------------------------------------------------
CAppStream::CAppStream():
    m_pObject(NULL),
    m_pObjectInPlace(NULL),
    m_pbInData(NULL),
    m_pwfx(NULL),
    m_uDataSize(0)
{
    ZeroMemory(&m_mt, sizeof(m_mt));
}

//-----------------------------------------------------------------------------
// Name: CAppStream::~CAppStream()
// Desc: Destructor
//-----------------------------------------------------------------------------
CAppStream::~CAppStream()
{
    SAFE_RELEASE( m_pObject);
    SAFE_RELEASE( m_pObjectInPlace);
    SafeGlobalFree(m_pbInData);
}

//-----------------------------------------------------------------------------
// Name: CAppStream::StreamData()
// Desc: Load data from input file, create media object and set input&output type
//-----------------------------------------------------------------------------

HRESULT CAppStream::StreamData( LPTSTR lpszInputFile,
                                REFGUID rclsid,
                                HWND hDlg,
                                BYTE **ppbOutData,
                                ULONG *pbDataSize,
                                LPWAVEFORMATEX  *ppwfx )
{
    HRESULT hr;
    hr = Init( lpszInputFile, rclsid, hDlg );
    if ( FAILED( hr ) ) {
       return hr;
    }

    hr = Stream( ppbOutData, pbDataSize, ppwfx );
    if ( FAILED( hr )) {
        MessageBox( hDlg, TEXT("Streaming data failed."), TEXT(DEMO_NAME),
                                    MB_OK | MB_ICONERROR );
        return hr;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CAppStream::Init()
// Desc: Load data from input file, create media object and set input&output type
//-----------------------------------------------------------------------------

HRESULT CAppStream::Init(LPTSTR lpszInputFile, REFGUID rclsid, HWND hDlg)
{
    // create DMO
    HRESULT hr = CoCreateInstance(rclsid,
                         NULL,
                         CLSCTX_INPROC,
                         IID_IMediaObject,
                         (void **) &m_pObject);
    if ( FAILED( hr ) ){
        MessageBox( hDlg, TEXT("Can't create this DMO."), TEXT(DEMO_NAME),MB_OK|MB_ICONERROR );
        return hr;
    }

    hr = m_pObject->QueryInterface(IID_IMediaObjectInPlace, (void**)&m_pObjectInPlace);

    // read wave file
    if( WaveLoadFile( lpszInputFile, (UINT*) &m_uDataSize, &m_pwfx, &m_pbInData ) != 0 ){
        MessageBox( hDlg, TEXT("Can't load input file."), TEXT(DEMO_NAME),MB_OK|MB_ICONERROR );
        return E_FAIL;
    }

    if( m_pwfx->wFormatTag != WAVE_FORMAT_PCM ) {
        MessageBox( hDlg, TEXT("Can't process compressed data."), TEXT(DEMO_NAME),MB_OK|MB_ICONERROR );
        return E_FAIL;
    }

    m_mt.majortype  = MEDIATYPE_Audio;
    m_mt.subtype    = MEDIASUBTYPE_PCM;
    m_mt.formattype = FORMAT_WaveFormatEx;
    m_mt.cbFormat   = sizeof(WAVEFORMATEX);
    m_mt.pbFormat   = (BYTE*) (m_pwfx);
    m_mt.pUnk = NULL;       // CopyMediaType will crash if we don't intialize this

    hr = m_pObject->SetInputType( 0,    //Input Stream index
                                  &m_mt,
                                  0 );  // No flags specified
    if ( FAILED( hr ) ){
        MessageBox( hDlg, TEXT("Can't set input type."), TEXT(DEMO_NAME),MB_OK | MB_ICONERROR );
        return hr;
    }

    hr = m_pObject->SetOutputType( 0,       // Output Stream Index
                                   &m_mt,
                                   0);  // No flags specified
    if ( FAILED( hr ) ){
       MessageBox( hDlg, TEXT("Can't set output type."), TEXT(DEMO_NAME),MB_OK | MB_ICONERROR );
       return hr;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CAppStream::Stream()
// Desc: called to get the output from DMO
//-----------------------------------------------------------------------------

HRESULT CAppStream::Stream( BYTE **ppbOutData, ULONG *pbDataSize, LPWAVEFORMATEX *ppwfx )
{
    HRESULT         hr = S_OK;
    BYTE            *pOut;

    *pbDataSize     = m_uDataSize;
    *ppwfx          = m_pwfx;

    if ( m_pObjectInPlace ){

        pOut = new BYTE [m_uDataSize];

        if( pOut == 0 ){
            return E_OUTOFMEMORY;
        }
        CopyMemory(pOut, m_pbInData, m_uDataSize);

        // pass the number of samples to Process()
        hr = m_pObjectInPlace->Process( m_uDataSize,
                                        pOut,
                                        0,
                                        DMO_INPLACE_NORMAL);
        if( FAILED( hr ) ){
            return hr;
        }
        *ppbOutData = pOut;
        SAFE_RELEASE( m_pObjectInPlace );
    }
    else
    {
        CMediaBuffer            *pInputBuffer;
        const REFERENCE_TIME    rtStart     = 0;
        const REFERENCE_TIME    rtStop      = 0;
        BYTE*                   pBuffer;
        DWORD                   dwLength;

        // create and fill CMediaBuffer
        hr = CreateBuffer(m_uDataSize, &pInputBuffer);
        if( FAILED( hr ) ){
            return hr;
        }

        hr = pInputBuffer->GetBufferAndLength( &pBuffer, &dwLength );
        if( FAILED( hr ) ){
            return hr;
        }
        CopyMemory(pBuffer, m_pbInData, m_uDataSize);

        hr = pInputBuffer->SetLength( m_uDataSize );
        if( FAILED( hr ) ){
            return hr;
        }

        // call processInput
        hr = m_pObject->ProcessInput( 0,
                                pInputBuffer,
                                DMO_INPUT_DATA_BUFFERF_SYNCPOINT,
                                rtStart,
                                rtStop - rtStart);
        if( FAILED( hr ) ){
            return hr;
        }

        //release input buffer
        SAFE_RELEASE( pInputBuffer );

        // retrieve the output data from DMO and put into pOut
        if(S_FALSE == hr){
            return E_FAIL;
        } else {
            pOut = NULL;
            hr = ProcessOutputs( &pOut );
            if( FAILED( hr ) ){
                delete [] pOut;
                return hr;
            }
        }

        *ppbOutData = pOut;
        SAFE_RELEASE( m_pObject );
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: CAppStream::ProcessOutputs()
// Desc: retrieve the output data from DMO
//-----------------------------------------------------------------------------

HRESULT CAppStream::ProcessOutputs( BYTE **ppbOutData )
{
    HRESULT hr = S_OK;
    DWORD   dwStatus=0;
    ULONG   ulSize=0;
    BYTE    *pOut=0;

    CMediaBuffer            *pOutputBuffer;
    DMO_OUTPUT_DATA_BUFFER  dataBufferStruct;

    hr = CreateBuffer( m_uDataSize,&pOutputBuffer );
    if ( FAILED( hr ) ) {
        return hr;
    }

    dataBufferStruct.pBuffer      = pOutputBuffer;
    dataBufferStruct.dwStatus     = 0;  // No flag is set
    dataBufferStruct.rtTimestamp  = 0;  // not used in ProcessOutput()
    dataBufferStruct.rtTimelength = 0;  // not used in ProcessOutput()

    *ppbOutData = new BYTE[m_uDataSize];
    if( *ppbOutData == 0 ){
       return E_OUTOFMEMORY;
    }

    //process until no more data
    if (SUCCEEDED(hr)) do {
        hr = m_pObject->ProcessOutput(  DMO_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER,
                                        1, //output buffer count
                                        &dataBufferStruct,
                                        &dwStatus );
        if ( FAILED( hr ) ) {
            return hr;
        }

        if( SUCCEEDED(hr) && (hr != S_FALSE) ) {
            hr = dataBufferStruct.pBuffer->GetBufferAndLength(&pOut, &ulSize);
            if ( FAILED( hr ) ) {
                return hr;
            }

            CopyMemory(*ppbOutData, pOut, m_uDataSize);

            hr = dataBufferStruct.pBuffer->SetLength( 0 );
            if( FAILED( hr ) ) {
                break;
            }
        }

    } while ( dataBufferStruct.dwStatus & DMO_OUTPUT_DATA_BUFFERF_INCOMPLETE );

    // free output buffer allocated:
    SAFE_RELEASE( pOutputBuffer );

    // Send Discontinuity on output stream
    hr = m_pObject->Discontinuity( 0 );
    if ( FAILED( hr ) ) {
        return hr;
    }

    return hr;
}


//-----------------------------------------------------------------------------
// Name: CreateBuffer()
// Desc: create a CMediaBuffer
//-----------------------------------------------------------------------------

HRESULT CreateBuffer(DWORD cbMaxLength, CMediaBuffer **ppBuffer)
{
    CMediaBuffer *pBuffer = new CMediaBuffer( cbMaxLength );

    if ( pBuffer == NULL || FAILED( pBuffer->Init() ) ) {
        delete pBuffer;
        *ppBuffer = NULL;

        return E_OUTOFMEMORY;
    }

    *ppBuffer = pBuffer;
    (*ppBuffer)->AddRef();

    return S_OK;
}
