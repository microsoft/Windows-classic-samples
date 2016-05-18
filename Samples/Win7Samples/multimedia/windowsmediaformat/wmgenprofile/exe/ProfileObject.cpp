//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ProfileObject.cpp
//
// Abstract:            The implementation for the profile object helper
//                      classes.  These classes are used as the item data in
//                      the dialog controls, and contain the configuration of
//                      profile objects.
//
//*****************************************************************************

#include "stdafx.h"
#include "GenProfile.h"
#include "ProfileObject.h"
#include "Macros.h"
#include <assert.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/*
** Profile object
*/

//------------------------------------------------------------------------------
// Name: CProfileObject::CProfileObject()
// Desc: Constructor.
//------------------------------------------------------------------------------
CProfileObject::CProfileObject()
{
    m_cRef = 1;
}

//------------------------------------------------------------------------------
// Name: CProfileObject::~CProfileObject()
// Desc: Destructor.
//------------------------------------------------------------------------------
CProfileObject::~CProfileObject()
{
}

//------------------------------------------------------------------------------
// Name: CProfileObject::Type()
// Desc: Returns the object type.
//------------------------------------------------------------------------------
ProfileObjectType CProfileObject::Type()
{
    return OT_Unknown;
}

//------------------------------------------------------------------------------
// Name: CProfileObject::AddRef()
// Desc: Adds a reference to the object.
//------------------------------------------------------------------------------
ULONG CProfileObject::AddRef()
{
    return ++m_cRef;
}

//------------------------------------------------------------------------------
// Name: CProfileObject::Release()
// Desc: Releases a reference and destroys the object 
//       if the reference count is zero.
//------------------------------------------------------------------------------
ULONG CProfileObject::Release()
{
    if ( 0 == --m_cRef )
    {
        delete this;
        return 0;
    }

    return m_cRef;
}


/*
** Stream object
*/

//------------------------------------------------------------------------------
// Name: CStream::CStream()
// Desc: Constructor.
//------------------------------------------------------------------------------
CStream::CStream()
{
    m_strName = "";
    m_stStreamType = ST_Arbitrary;
    m_dwStreamFormatIndex = 0;
    m_dwFormatStringIndex = 0;
    m_dwBufferWindow = 3000;
    m_dwVideoWidth = 320;
    m_dwVideoHeight = 200;
    m_dwSecondsPerKeyframe = 3000;
    m_dwFPS = 30;
    m_dwVideoQuality = 100;
    m_dwStreamBitrate = 100000;
    m_guidArbitraryType = GUID_NULL;
    m_fVideoIsVBR = FALSE;
    m_dwVideoMaxBufferWindow = 0;
    m_dwVideoMaxBitrate = 10000;
    m_nVideoVBRMode = VBR_QUALITYBASED;
    m_fHasSMPTE = FALSE;
    m_wStreamNumber = 0;
    m_dwStreamCodecIndex = 0;
    m_dwVideoVBRQuality = 0;
    m_fIsUncompressed = FALSE;
    m_dwPixelFormatIndex = 0;
    m_dwPixelFormatStringIndex = 0;
    m_dwWaveFormatIndex = 0;
    m_dwWaveFormatStringIndex = 0;
    m_dwLanguageLCID = GetSystemDefaultLCID();
}


//------------------------------------------------------------------------------
// Name: CStream::~CStream()
// Desc: Destructor.
//------------------------------------------------------------------------------
CStream::~CStream()
{
    PrepareForDeletion();
}


//------------------------------------------------------------------------------
// Name: CStream::Type()
// Desc: Returns object type.
//------------------------------------------------------------------------------
ProfileObjectType CStream::Type()
{
    return OT_Stream;
}


//------------------------------------------------------------------------------
// Name: CStream::PrepareForDeletion()
// Desc: Cleans up dependent objects.
//------------------------------------------------------------------------------
void CStream::PrepareForDeletion()
{
    int nIndex;

    for ( nIndex = 0; nIndex < m_dependentObjects.GetSize(); nIndex++ )
    {
        assert( m_dependentObjects[nIndex] );
        m_dependentObjects[nIndex]->NotifyObjectDeleted( this );
        SAFE_RELEASE( m_dependentObjects[nIndex] );
    }
    m_dependentObjects.RemoveAll();
}


//------------------------------------------------------------------------------
// Name: CStream::AddDependent()
// Desc: Adds an object to the list of dependent objects.
//------------------------------------------------------------------------------
HRESULT CStream::AddDependent( CProfileObject* pDependentObject )
{
    if ( !pDependentObject )
    {
        return E_INVALIDARG;
    }

    SAFE_ADDREF( pDependentObject );
    m_dependentObjects.Add( pDependentObject );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStream::RemoveDependent()
// Desc: Removes an object from the list of dependent objects and releases it.
//------------------------------------------------------------------------------
HRESULT CStream::RemoveDependent( CProfileObject* pDependentObject )
{
    BOOL fFound;
    INT nIndex;

    if ( !pDependentObject )
    {
        return E_INVALIDARG;
    }

    fFound = false;
    for( nIndex = 0; nIndex < m_dependentObjects.GetSize(); nIndex++ )
    {
        if ( m_dependentObjects[nIndex] == pDependentObject )
        {
            fFound = true;
            break;
        }
    }

    if ( fFound )
    {
        m_dependentObjects.RemoveAt( nIndex );
        SAFE_RELEASE( pDependentObject );
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStream::DependentCount()
// Desc: Returns the count of dependent objects.
//------------------------------------------------------------------------------
ULONG CStream::DependentCount()
{
    return ( DWORD )m_dependentObjects.GetSize();
}


//------------------------------------------------------------------------------
// Name: CStream::GetDependent()
// Desc: Retrieves a dependent object by its index in the list.
//------------------------------------------------------------------------------
HRESULT CStream::GetDependent( INT nIndex, CProfileObject **ppProfileObject )
{
    if ( !ppProfileObject )
    {
        return E_POINTER;
    }
    if ( nIndex < 0 || nIndex >= m_dependentObjects.GetSize() )
    {
        return E_INVALIDARG;
    }

    *ppProfileObject = m_dependentObjects[ nIndex ];
    SAFE_ADDREF( (*ppProfileObject) );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStream::NotifyObjectDeleted()
// Desc: Removes a deleted object from the list of dependent objects.
//------------------------------------------------------------------------------
HRESULT CStream::NotifyObjectDeleted( CProfileObject* pDeletedObject )
{
    int nIndex;

    if ( !pDeletedObject )
    {
        return E_INVALIDARG;
    }

    for ( nIndex = 0; nIndex < m_dependentObjects.GetSize(); nIndex++ )
    {
        if ( m_dependentObjects[nIndex] == pDeletedObject )
        {
            m_dependentObjects.RemoveAt( nIndex );
            SAFE_RELEASE( pDeletedObject );
            break;
        }
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: Set/Get methods
// Desc: Set and retrieve member values.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Name: CStream::SetName()
//------------------------------------------------------------------------------
HRESULT CStream::SetName( CString strName )
{
    m_strName = strName;

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: CStream::GetName()
//------------------------------------------------------------------------------
CString CStream::GetName()
{
    return m_strName;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamType()
//------------------------------------------------------------------------------
StreamType CStream::GetStreamType( )
{
    return m_stStreamType;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamType()
//------------------------------------------------------------------------------
void CStream::SetStreamType( StreamType stNewType )
{
    assert( ( stNewType == ST_Audio ) || 
            ( stNewType == ST_Video ) || 
            ( stNewType == ST_Script ) || 
            ( stNewType == ST_Image ) || 
            ( stNewType == ST_Web ) || 
            ( stNewType == ST_File ) || 
            ( stNewType == ST_Arbitrary ) );

    m_stStreamType = stNewType;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamFormatIndex()
//------------------------------------------------------------------------------
void CStream::SetStreamFormatIndex( DWORD dwIndex )
{
    m_dwStreamFormatIndex = dwIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamFormatIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetStreamFormatIndex()
{
    return m_dwStreamFormatIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamFormatStringIndex()
//------------------------------------------------------------------------------
void CStream::SetStreamFormatStringIndex( DWORD dwFormatStringIndex )
{
    m_dwFormatStringIndex = dwFormatStringIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamFormatStringIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetStreamFormatStringIndex()
{
    return m_dwFormatStringIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamBufferWindow()
//------------------------------------------------------------------------------
void CStream::SetStreamBufferWindow( DWORD dwBufferWindow )
{
    m_dwBufferWindow = dwBufferWindow;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamBufferWindow()
//------------------------------------------------------------------------------

DWORD CStream::GetStreamBufferWindow()
{
    return m_dwBufferWindow;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoHeight()
//------------------------------------------------------------------------------
void CStream::SetVideoHeight( DWORD dwHeight )
{
    m_dwVideoHeight = dwHeight;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoHeight()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoHeight()
{
    return m_dwVideoHeight;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoWidth()
//------------------------------------------------------------------------------
void CStream::SetVideoWidth( DWORD dwVideoWidth )
{
    m_dwVideoWidth = dwVideoWidth;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoWidth()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoWidth()
{
    return m_dwVideoWidth;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamBitrate()
//------------------------------------------------------------------------------
void CStream::SetStreamBitrate( DWORD dwBitrate )
{
    m_dwStreamBitrate = dwBitrate;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamBitrate()
//------------------------------------------------------------------------------
DWORD CStream::GetStreamBitrate()
{
    return m_dwStreamBitrate;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoFPS()
//------------------------------------------------------------------------------
void CStream::SetVideoFPS( DWORD dwFPS )
{
    m_dwFPS = dwFPS;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoFPS()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoFPS()
{
    return m_dwFPS;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoSecondsPerKeyframe()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoSecondsPerKeyframe()
{
    return m_dwSecondsPerKeyframe;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoSecondsPerKeyframe()
//------------------------------------------------------------------------------
void CStream::SetVideoSecondsPerKeyframe( DWORD dwSecondsPerKeyframe )
{
    m_dwSecondsPerKeyframe = dwSecondsPerKeyframe;
}


//------------------------------------------------------------------------------
// Name: CStream::SetVideoQuality()
//------------------------------------------------------------------------------
void CStream::SetVideoQuality( DWORD dwVideoQuality )
{
    assert( dwVideoQuality <= 100 );

    m_dwVideoQuality = dwVideoQuality;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoQuality()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoQuality()
{
    return m_dwVideoQuality;
}

//------------------------------------------------------------------------------
// Name: CStream::SetArbitraryType()
//------------------------------------------------------------------------------
void CStream::SetArbitraryType( GUID guidType )
{
    m_guidArbitraryType = guidType;
}

//------------------------------------------------------------------------------
// Name: CStream::GetArbitraryType()
//------------------------------------------------------------------------------
GUID CStream::GetArbitraryType()
{
    return m_guidArbitraryType;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoIsVBR()
//------------------------------------------------------------------------------
void CStream::SetVideoIsVBR( BOOL fIsVBR )
{
    m_fVideoIsVBR = fIsVBR;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoIsVBR()
//------------------------------------------------------------------------------
BOOL CStream::GetVideoIsVBR()
{
    return m_fVideoIsVBR;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoVBRMode()
//------------------------------------------------------------------------------
VIDEO_VBR_MODE CStream::GetVideoVBRMode()
{
    return m_nVideoVBRMode;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoVBRMode()
//------------------------------------------------------------------------------
void CStream::SetVideoVBRMode( VIDEO_VBR_MODE nVBRMode )
{
    assert( ( nVBRMode == VBR_OFF ) ||
            ( nVBRMode == VBR_QUALITYBASED ) ||
            ( nVBRMode == VBR_CONSTRAINED ) ||
            ( nVBRMode == VBR_UNCONSTRAINED ) );

    m_nVideoVBRMode = nVBRMode;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoMaxBitrate()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoMaxBitrate()
{
    return m_dwVideoMaxBitrate;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoMaxBitrate()
//------------------------------------------------------------------------------
void CStream::SetVideoMaxBitrate( DWORD dwMaxBitrate )
{
    m_dwVideoMaxBitrate = dwMaxBitrate;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoMaxBufferWindow()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoMaxBufferWindow()
{
    return m_dwVideoMaxBufferWindow;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoMaxBufferWindow()
//------------------------------------------------------------------------------
void CStream::SetVideoMaxBufferWindow( DWORD dwMaxBufferWindow )
{
    m_dwVideoMaxBufferWindow = dwMaxBufferWindow;
}

//------------------------------------------------------------------------------
// Name: CStream::SetIsSMPTE()
//------------------------------------------------------------------------------
void CStream::SetIsSMPTE( BOOL fIsSMPTE )
{
    m_fHasSMPTE = fIsSMPTE;
}

//------------------------------------------------------------------------------
// Name: CStream::GetIsSMPTE()
//------------------------------------------------------------------------------
BOOL CStream::GetIsSMPTE()
{
    return m_fHasSMPTE;
}

//------------------------------------------------------------------------------
// Name: CStream::GetIsUncompressed()
//------------------------------------------------------------------------------
BOOL CStream::GetIsUncompressed()
{
    return m_fIsUncompressed;
}

//------------------------------------------------------------------------------
// Name: CStream::SetIsUncompressed()
//------------------------------------------------------------------------------
void CStream::SetIsUncompressed( BOOL fIsUncompressed )
{
    m_fIsUncompressed = fIsUncompressed;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamNumber()
//------------------------------------------------------------------------------
void CStream::SetStreamNumber( WORD wStreamNumber )
{
    m_wStreamNumber = wStreamNumber;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamNumber()
//------------------------------------------------------------------------------
WORD CStream::GetStreamNumber()
{
    return m_wStreamNumber;
}

//------------------------------------------------------------------------------
// Name: CStream::SetStreamCodecIndex()
//------------------------------------------------------------------------------
void CStream::SetStreamCodecIndex( DWORD dwStreamCodecIndex )
{
    m_dwStreamCodecIndex = dwStreamCodecIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetStreamCodecIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetStreamCodecIndex()
{
    return m_dwStreamCodecIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetVideoVBRQuality()
//------------------------------------------------------------------------------
DWORD CStream::GetVideoVBRQuality()
{
    return m_dwVideoVBRQuality;
}

//------------------------------------------------------------------------------
// Name: CStream::SetVideoVBRQuality()
//------------------------------------------------------------------------------
void CStream::SetVideoVBRQuality( DWORD dwVBRQuality )
{
    assert( dwVBRQuality <= 100 );

    m_dwVideoVBRQuality = dwVBRQuality;
}

//------------------------------------------------------------------------------
// Name: CStream::GetPixelFormatIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetPixelFormatIndex()
{
    return m_dwPixelFormatIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::SetPixelFormatIndex()
//------------------------------------------------------------------------------
void CStream::SetPixelFormatIndex( DWORD dwIndex )
{
    m_dwPixelFormatIndex = dwIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetPixelFormatStringIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetPixelFormatStringIndex()
{
    return m_dwPixelFormatStringIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::SetPixelFormatStringIndex()
//------------------------------------------------------------------------------
void CStream::SetPixelFormatStringIndex( DWORD dwStringIndex )
{
    m_dwPixelFormatStringIndex = dwStringIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetWaveFormatIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetWaveFormatIndex()
{
    return m_dwWaveFormatIndex;
}


//------------------------------------------------------------------------------
// Name: CStream::GetWaveFormatStringIndex()
//------------------------------------------------------------------------------
DWORD CStream::GetWaveFormatStringIndex()
{
    return m_dwWaveFormatStringIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::SetWaveFormatStringIndex()
//------------------------------------------------------------------------------
void CStream::SetWaveFormatStringIndex( DWORD dwStringIndex )
{
    m_dwWaveFormatStringIndex = dwStringIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::SetWaveFormatIndex()
//------------------------------------------------------------------------------
void CStream::SetWaveFormatIndex( DWORD dwIndex )
{
    m_dwWaveFormatIndex = dwIndex;
}

//------------------------------------------------------------------------------
// Name: CStream::GetLanguageLCID()
//------------------------------------------------------------------------------
DWORD CStream::GetLanguageLCID()
{
    return m_dwLanguageLCID;
}

//------------------------------------------------------------------------------
// Name: CStream::SetLanguageLCID()
//------------------------------------------------------------------------------
void CStream::SetLanguageLCID( DWORD dwLCID )
{
    m_dwLanguageLCID = dwLCID;
}


/*
** Mutex object
*/

//------------------------------------------------------------------------------
// Name: CMutex::CMutex()
// Desc: Constructor.
//------------------------------------------------------------------------------
CMutex::CMutex()
{
    m_mtType = MT_Bitrate;
}


//------------------------------------------------------------------------------
// Name: CMutex::~CMutex()
// Desc: Destructor.
//------------------------------------------------------------------------------
CMutex::~CMutex()
{
    PrepareForDeletion();
}


//------------------------------------------------------------------------------
// Name: CMutex::Type()
// Desc: Returns the object type.
//------------------------------------------------------------------------------
ProfileObjectType CMutex::Type()
{
    return OT_Mutex;
}


//------------------------------------------------------------------------------
// Name: CMutex::NotifyObjectDeleted()
// Desc: Removes a deleted object from the m_streamsInMutex list.
//------------------------------------------------------------------------------
HRESULT CMutex::NotifyObjectDeleted( CProfileObject* pDeletedObject )
{
    int nIndex;

    if ( !pDeletedObject )
    {
        return E_INVALIDARG;
    }

    for ( nIndex = 0; nIndex < m_streamsInMutex.GetSize(); nIndex++ )
    {
        if ( m_streamsInMutex[nIndex] == pDeletedObject )
        {
            m_streamsInMutex.RemoveAt( nIndex );
            SAFE_RELEASE( pDeletedObject );
            break;
        }
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CMutex::PrepareForDeletion()
// Desc: Cleans up the m_streamsInMutex list.
//------------------------------------------------------------------------------
void CMutex::PrepareForDeletion()
{
    int nIndex;

    for ( nIndex = 0; nIndex < m_streamsInMutex.GetSize(); nIndex++ )
    {
        assert( m_streamsInMutex[nIndex] );
        m_streamsInMutex[nIndex]->NotifyObjectDeleted( this );
        SAFE_RELEASE( m_streamsInMutex[nIndex] );
    }
    m_streamsInMutex.RemoveAll();
}


//------------------------------------------------------------------------------
// Name: CMutex::AddStream()
// Desc: Adds a stream to the m_streamsInMutex list.
//------------------------------------------------------------------------------
HRESULT CMutex::AddStream( CStream* pNewStream )
{
    if ( !pNewStream )
    {
        return E_INVALIDARG;
    }

    SAFE_ADDREF( pNewStream );
    m_streamsInMutex.Add( (CProfileObject*) pNewStream );
    pNewStream->AddDependent( this );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CMutex::RemoveStream()
// Desc: Removes a stream from the m_streamsInMutex list.
//------------------------------------------------------------------------------
HRESULT CMutex::RemoveStream( CStream* pStream )
{
    BOOL fFound;
    INT nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    fFound = false;
    for( nIndex = 0; nIndex < m_streamsInMutex.GetSize(); nIndex++ )
    {
        if ( m_streamsInMutex[nIndex] == pStream )
        {
            fFound = true;
            break;
        }
    }

    if ( fFound )
    {
        m_streamsInMutex.RemoveAt( nIndex );
        pStream->RemoveDependent( this );
        SAFE_RELEASE( pStream );
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CMutex::InObject()
// Desc: Ascertains whether the specified stream in in the m_streamsInMutex list.
//------------------------------------------------------------------------------
BOOL CMutex::InObject( CStream* pStream )
{
    BOOL fFound;
    INT nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    fFound = false;
    for( nIndex = 0; nIndex < m_streamsInMutex.GetSize(); nIndex++ )
    {
        if ( m_streamsInMutex[nIndex] == pStream )
        {
            fFound = true;
            break;
        }
    }

    return fFound;
}

//------------------------------------------------------------------------------
// Name: CMutex::RemoveAllStreams()
// Desc: Removes all streams from the m_streamsInMutex list.
//------------------------------------------------------------------------------
HRESULT CMutex::RemoveAllStreams()
{
    HRESULT hr = S_OK;

    do
    {
        while ( m_streamsInMutex.GetSize() > 0 )
        {
            hr = RemoveStream( (CStream*) m_streamsInMutex[0] );
            if ( FAILED( hr ) )
            {
                break;
            }
        }
    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CMutex::SetMutexType()
// Desc: Sets the m_mtType member.
//------------------------------------------------------------------------------
void CMutex::SetMutexType( MUTEX_TYPE mtType )
{
    assert( ( mtType == MT_Bitrate ) ||
         ( mtType == MT_Language ) ||
         ( mtType == MT_Presentation ) );

    m_mtType = mtType;
}


//------------------------------------------------------------------------------
// Name: CMutex::GetMutexType()
// Desc: Retrieves the value of the m_mtType member.
//------------------------------------------------------------------------------
MUTEX_TYPE CMutex::GetMutexType()
{
    return m_mtType;
}


//------------------------------------------------------------------------------
// Name: CMutex::StreamCount()
// Desc: Returns the number of streams in the m_streamsInMutex list.
//------------------------------------------------------------------------------
INT CMutex::StreamCount()
{
    return ( DWORD )m_streamsInMutex.GetSize();
}


//------------------------------------------------------------------------------
// Name: CMutex::GetStream()
// Desc: Retrieves a stream from the m_streamsInMutex list by index.
//------------------------------------------------------------------------------
HRESULT CMutex::GetStream( INT nStreamIndex, CStream** ppStream )
{
    if ( !ppStream )
    {
        return E_POINTER;
    }
    if ( ( nStreamIndex < 0) || ( nStreamIndex >= m_streamsInMutex.GetSize() ) )
    {
        return E_INVALIDARG;
    }

    *ppStream = (CStream*) m_streamsInMutex[nStreamIndex];
    SAFE_ADDREF( (*ppStream) );

    return S_OK;
}


/*
** Stream prioritization object
*/

//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::CStreamPrioritizationObject()
// Desc: Constructor.
//------------------------------------------------------------------------------
CStreamPrioritizationObject::CStreamPrioritizationObject()
{
}

//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::~CStreamPrioritizationObject()
// Desc: Destructor.
//------------------------------------------------------------------------------
CStreamPrioritizationObject::~CStreamPrioritizationObject()
{
    PrepareForDeletion();
}

//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::Type()
// Desc: Returns the object type.
//------------------------------------------------------------------------------
ProfileObjectType CStreamPrioritizationObject::Type()
{
    return OT_StreamPrioritization;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::NotifyObjectDeleted()
// Desc: Removes a deleted object from the m_streamPriorities list.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::NotifyObjectDeleted( CProfileObject* pDeletedObject )
{
    int nIndex;

    if ( !pDeletedObject )
    {
        return E_INVALIDARG;
    }

    for ( nIndex = 0; nIndex < m_streamPriorities.GetSize(); nIndex++ )
    {
        if ( m_streamPriorities[nIndex] == pDeletedObject )
        {
            m_streamPriorities.RemoveAt( nIndex );
            SAFE_RELEASE( pDeletedObject );
            break;
        }
    }

    for ( nIndex = 0; nIndex < m_mandatoryStreams.GetSize(); nIndex++ )
    {
        if ( m_mandatoryStreams[nIndex] == pDeletedObject )
        {
            m_mandatoryStreams.RemoveAt( nIndex );
            SAFE_RELEASE( pDeletedObject );
            break;
        }
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::PrepareForDeletion()
// Desc: Cleans up the m_streamPriorities list.
//------------------------------------------------------------------------------
void CStreamPrioritizationObject::PrepareForDeletion()
{
    int nIndex;

    for ( nIndex = 0; nIndex < m_streamPriorities.GetSize(); nIndex++ )
    {
        assert( m_streamPriorities[nIndex] );
        m_streamPriorities[nIndex]->NotifyObjectDeleted( this );
        SAFE_RELEASE( m_streamPriorities[nIndex] );
    }
    m_streamPriorities.RemoveAll();

    for ( nIndex = 0; nIndex < m_mandatoryStreams.GetSize(); nIndex++ )
    {
        assert( m_mandatoryStreams[nIndex] );
        m_mandatoryStreams[nIndex]->NotifyObjectDeleted( this );
        SAFE_RELEASE( m_mandatoryStreams[nIndex] );
    }
    m_mandatoryStreams.RemoveAll();
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::AddStream()
// Desc: Adds a stream to the m_streamPriorities list.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::AddStream( CStream* pNewStream )
{
    if ( !pNewStream )
    {
        return E_INVALIDARG;
    }

    SAFE_ADDREF( pNewStream );
    try
    {
        m_streamPriorities.Add( pNewStream );
        pNewStream->AddDependent( this );
    }
    catch ( ... )
    {
        SAFE_RELEASE( pNewStream );
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::SetStreamMandatory()
// Desc: Adds an object to the m_mandatoryStreams list, or removes one.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::SetStreamMandatory( CStream* pStream, BOOL fIsMandatory )
{
    BOOL fFound;
    INT nFoundIndex;
    int nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    //
    // Search the list for the stream
    //
    fFound = false;
    nFoundIndex = -1;
    for ( nIndex = 0; nIndex < m_mandatoryStreams.GetSize(); nIndex++ )
    {
        if ( m_mandatoryStreams[ nIndex ] == pStream )
        {
            fFound = true;
            nFoundIndex = nIndex;
            break;
        }
    }

    if ( fFound )
    {
        if ( !fIsMandatory )
        {
            SAFE_RELEASE( pStream );
            m_mandatoryStreams.RemoveAt( nFoundIndex );
        }
    }
    else
    {
        if ( fIsMandatory )
        {
            SAFE_ADDREF( pStream );
            m_mandatoryStreams.Add( pStream );
        }
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::ClearMandatoryStreams()
// Desc: Makes all streams non-mandatory.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::ClearMandatoryStreams()
{
    HRESULT hr = S_OK;

    while ( m_mandatoryStreams.GetSize() > 0 )
    {
        hr = SetStreamMandatory( (CStream*) m_mandatoryStreams[0], FALSE );
        if ( FAILED( hr ) )
        {
            break;
        }
    }

    return hr;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::GetStreamMandatory()
// Desc: Ascertains whether a stream is in the m_mandatoryStreams list.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::GetStreamMandatory( CStream* pStream, BOOL *pbIsMandatory )
{
    BOOL fFound;
    int nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }
    if ( !pbIsMandatory )
    {
        return E_POINTER;
    }

    //
    // Search the list for the stream
    //
    fFound = false;
    for ( nIndex = 0; nIndex < m_mandatoryStreams.GetSize(); nIndex++ )
    {
        if ( m_mandatoryStreams[ nIndex ] == pStream )
        {
            fFound = true;
            break;
        }
    }

    *pbIsMandatory = fFound;

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::IncreasePriority()
// Desc: Moves a stream up in the priorities list.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::IncreasePriority( CStream* pStream )
{
    BOOL fFound;
    INT nFoundIndex;
    int nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    //
    // Search the list for the stream
    //
    fFound = false;
    nFoundIndex = -1;
    for ( nIndex = 0; nIndex < m_streamPriorities.GetSize(); nIndex++ )
    {
        if ( m_streamPriorities[ nIndex ] == pStream )
        {
            fFound = true;
            nFoundIndex = nIndex;
            break;
        }
    }

    if ( fFound )
    {
        if ( nFoundIndex > 0 )
        {
            m_streamPriorities.RemoveAt( nFoundIndex );
            m_streamPriorities.InsertAt( nFoundIndex - 1, pStream );
        }

        return S_OK;
    }

    return E_FAIL;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::DecreasePriority()
// Desc: Moves a stream down in the priorities list.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::DecreasePriority( CStream* pStream )
{
    BOOL fFound;
    INT nFoundIndex;
    int nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    //
    // Search the list for the stream
    //
    fFound = false;
    nFoundIndex = -1;
    for ( nIndex = 0; nIndex < m_streamPriorities.GetSize(); nIndex++ )
    {
        if ( m_streamPriorities[ nIndex ] == pStream )
        {
            fFound = true;
            nFoundIndex = nIndex;
            break;
        }
    }

    if ( fFound )
    {
        if ( nFoundIndex < m_streamPriorities.GetSize() )
        {
            m_streamPriorities.RemoveAt( nFoundIndex );
            m_streamPriorities.InsertAt( nFoundIndex + 1, pStream );
        }

        return S_OK;
    }

    return E_FAIL;
}


//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::GetStreamWithPriority()
// Desc: Retrieves a stream by its priority index.
//------------------------------------------------------------------------------
HRESULT CStreamPrioritizationObject::GetStreamWithPriority( INT nPriorityIndex, CStream** ppStream )
{
    if ( !ppStream )
    {
        return E_POINTER;
    }
    if ( ( nPriorityIndex < 0 ) || ( nPriorityIndex > m_streamPriorities.GetSize() ))
    {
        return E_INVALIDARG;
    }

    *ppStream = (CStream*) m_streamPriorities[ nPriorityIndex ];
    SAFE_ADDREF( (*ppStream) );

    return S_OK;
}

//------------------------------------------------------------------------------
// Name: CStreamPrioritizationObject::StreamCount()
// Desc: Returns the number of streams in the m_streamPriorities list.
//------------------------------------------------------------------------------
INT CStreamPrioritizationObject::StreamCount()
{
    return ( DWORD )m_streamPriorities.GetSize();
}


/*
** Bandwidth sharing object
*/

//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::CBandwidthSharingObject()
// Desc: Constructor.
//------------------------------------------------------------------------------
CBandwidthSharingObject::CBandwidthSharingObject()
{
    m_dwSharedBitrate = 100000;
    m_dwBufferWindow = 3000;
    m_guidBandwidthSharingType = CLSID_WMBandwidthSharing_Exclusive;
}

//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::~CBandwidthSharingObject()
// Desc: Destructor.
//------------------------------------------------------------------------------
CBandwidthSharingObject::~CBandwidthSharingObject()
{
    PrepareForDeletion();
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::Type()
// Desc: Returns the object type.
//------------------------------------------------------------------------------
ProfileObjectType CBandwidthSharingObject::Type()
{
    return OT_BandwidthSharing;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::NotifyObjectDeleted()
// Desc: Removes a deleted object from the m_streamsSharingBandwidth list.
//------------------------------------------------------------------------------
HRESULT CBandwidthSharingObject::NotifyObjectDeleted( CProfileObject* pDeletedObject )
{
    int nIndex;

    if ( !pDeletedObject )
    {
        return E_INVALIDARG;
    }

    for ( nIndex = 0; nIndex < m_streamsSharingBandwidth.GetSize(); nIndex++ )
    {
        if ( m_streamsSharingBandwidth[nIndex] == pDeletedObject )
        {
            m_streamsSharingBandwidth.RemoveAt( nIndex );
            SAFE_RELEASE( pDeletedObject );
            break;
        }
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::PrepareForDeletion()
// Desc: Cleans up the m_streamsSharingBandwidth list.
//------------------------------------------------------------------------------
void CBandwidthSharingObject::PrepareForDeletion()
{
    int nIndex;

    for ( nIndex = 0; nIndex < m_streamsSharingBandwidth.GetSize(); nIndex++ )
    {
        assert( m_streamsSharingBandwidth[nIndex] );
        m_streamsSharingBandwidth[nIndex]->NotifyObjectDeleted( this );
        SAFE_RELEASE( m_streamsSharingBandwidth[nIndex] );
    }
    m_streamsSharingBandwidth.RemoveAll();
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::AddStream()
// Desc: Adds a stream to the m_streamsSharingBandwidth list.
//------------------------------------------------------------------------------
HRESULT CBandwidthSharingObject::AddStream( CStream* pNewStream )
{
    if ( !pNewStream )
    {
        return E_INVALIDARG;
    }

    SAFE_ADDREF( pNewStream );
    m_streamsSharingBandwidth.Add( pNewStream );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::RemoveStream()
// Desc: Removes a stream from the m_streamsSharingBandwidth list
//       and releases it.
//------------------------------------------------------------------------------
HRESULT CBandwidthSharingObject::RemoveStream( CStream* pStream )
{
    BOOL fFound;
    INT nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    fFound = false;
    for( nIndex = 0; nIndex < m_streamsSharingBandwidth.GetSize(); nIndex++ )
    {
        if ( m_streamsSharingBandwidth[nIndex] == pStream )
        {
            fFound = true;
            break;
        }
    }

    if ( fFound )
    {
        m_streamsSharingBandwidth.RemoveAt( nIndex );
        SAFE_RELEASE( pStream );
    }

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::InObject()
// Desc: Ascertains whether a stream is in the m_streamsSharingBandwidth list.
//------------------------------------------------------------------------------
BOOL CBandwidthSharingObject::InObject( CStream* pStream )
{
    BOOL fFound;
    INT nIndex;

    if ( !pStream )
    {
        return E_INVALIDARG;
    }

    fFound = false;
    for( nIndex = 0; nIndex < m_streamsSharingBandwidth.GetSize(); nIndex++ )
    {
        if ( m_streamsSharingBandwidth[nIndex] == pStream )
        {
            fFound = true;
            break;
        }
    }

    return fFound;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::RemoveAllStreams()
// Desc: Removes all streams from the m_streamsSharingBandwidth list.
//------------------------------------------------------------------------------
HRESULT CBandwidthSharingObject::RemoveAllStreams()
{
    HRESULT hr = S_OK;

    do
    {
        while ( m_streamsSharingBandwidth.GetSize() > 0 )
        {
            hr = RemoveStream( (CStream*) m_streamsSharingBandwidth[0] );
            if ( FAILED( hr ) )
            {
                break;
            }
        }
    }
    while ( FALSE );

    return hr;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::SetSharedBitrate()
// Desc: Sets the value of the m_dwSharedBitrate member.
//------------------------------------------------------------------------------
void CBandwidthSharingObject::SetSharedBitrate( DWORD dwBitrate )
{
    m_dwSharedBitrate = dwBitrate;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::GetSharedBitrate()
// Desc: Returns the value of the m_dwSharedBitrate member.
//------------------------------------------------------------------------------
DWORD CBandwidthSharingObject::GetSharedBitrate( )
{
    return m_dwSharedBitrate;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::StreamCount()
// Desc: Returns the number of streams in the m_streamsSharingBandwidth list.
//------------------------------------------------------------------------------
INT CBandwidthSharingObject::StreamCount()
{
    return ( DWORD )m_streamsSharingBandwidth.GetSize();
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::GetStream()
// Desc: Retrieves a stream from the m_streamsSharingBandwidth list by index.
//------------------------------------------------------------------------------
HRESULT CBandwidthSharingObject::GetStream( INT nStreamIndex, CStream** ppStream )
{
    if ( !ppStream )
    {
        return E_POINTER;
    }
    if ( ( nStreamIndex < 0) || ( nStreamIndex >= m_streamsSharingBandwidth.GetSize() ) )
    {
        return E_INVALIDARG;
    }

    *ppStream = (CStream*) m_streamsSharingBandwidth[nStreamIndex];
    SAFE_ADDREF( (*ppStream) );

    return S_OK;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::SetBandwidthSharingType()
// Desc: Sets the m_guidBandwidthSharingType member.
//------------------------------------------------------------------------------
void CBandwidthSharingObject::SetBandwidthSharingType( GUID guidType )
{
    m_guidBandwidthSharingType = guidType;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::GetBandwidthSharingType()
// Desc: Returns the value of the m_guidBandwidthSharingType member.
//------------------------------------------------------------------------------
GUID CBandwidthSharingObject::GetBandwidthSharingType()
{
    return m_guidBandwidthSharingType;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::GetBufferWindow()
// Desc: Returns the value of the m_dwBufferWindow member.
//------------------------------------------------------------------------------
DWORD CBandwidthSharingObject::GetBufferWindow()
{
    return m_dwBufferWindow;
}


//------------------------------------------------------------------------------
// Name: CBandwidthSharingObject::SetBufferWindow()
// Desc: Sets the m_dwBufferWindow member.
//------------------------------------------------------------------------------
void CBandwidthSharingObject::SetBufferWindow( DWORD dwBufferWindow )
{
    m_dwBufferWindow = dwBufferWindow;
}

