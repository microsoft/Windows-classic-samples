//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            SDKSamplePlaylistParser.cpp
//
// Abstract:
//
//*****************************************************************************

#include "stdafx.h"
#include "SDKSamplePlaylistPlugin.h"
#include "SDKSamplePlaylistParser.h"


/////////////////////////////////////////////////////////////////////////////
//
// [CSDKSamplePlaylistParserPlugin]
//
/////////////////////////////////////////////////////////////////////////////
CSDKSamplePlaylistParserPlugin::CSDKSamplePlaylistParserPlugin()
{
} // CSDKSamplePlaylistParserPlugin.



/////////////////////////////////////////////////////////////////////////////
//
// [~CSDKSamplePlaylistParserPlugin]
//
/////////////////////////////////////////////////////////////////////////////
CSDKSamplePlaylistParserPlugin::~CSDKSamplePlaylistParserPlugin()
{
} // ~CSDKSamplePlaylistParserPlugin.




/////////////////////////////////////////////////////////////////////////////
//
// [InitializePlugin]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParserPlugin::InitializePlugin( 
                                IWMSContext *pServerContext,
                                IWMSNamedValues *pNamedValues,
                                IWMSClassObject *pClassFactory
                                )
{
    //
    // We don't really use any of these parameters later on.
    // We are merely illustrating how you would hold a reference to it
    // using smart pointers if you did need to use it later on.
    //

    m_spServerContext = pServerContext;
    m_spNamedValues = pNamedValues;
    m_spClassFactory = pClassFactory;

    return( S_OK );
} // InitializePlugin.



/////////////////////////////////////////////////////////////////////////////
//
// [GetCustomAdminInterface]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParserPlugin::GetCustomAdminInterface( 
                    IDispatch **ppValue
                    )
{
    if ( NULL == ppValue )
    {
        return( E_INVALIDARG );
    }

    *ppValue = NULL;

    //
    // Since we have no configuration paramters to edit,
    // we expose no administration interface.
    //
    return( S_OK );
}



/////////////////////////////////////////////////////////////////////////////
//
// [OnHeartbeat]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParserPlugin::OnHeartbeat()
{
    return( S_OK );
} // OnHeartbeat.



/////////////////////////////////////////////////////////////////////////////
//
// [EnablePlugin]
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CSDKSamplePlaylistParserPlugin::EnablePlugin( long *pdwFlags, long *pdwHeartbeatPeriod )
{
    if ( ( NULL == pdwFlags ) || ( NULL == pdwHeartbeatPeriod ) )
    {
        return ( E_POINTER );
    }

    *pdwFlags = 0;
    // We don't need to get OnHeartbeat() called.
    *pdwHeartbeatPeriod = 0;
    
    return ( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
//
// [DisablePlugin]
//
/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CSDKSamplePlaylistParserPlugin::DisablePlugin()
{
    return ( S_OK );
}




/////////////////////////////////////////////////////////////////////////////
//
// [Shutdown]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParserPlugin::ShutdownPlugin()
{
    m_spNamedValues.Release();
    m_spServerContext.Release(); 
    m_spClassFactory.Release();
    return( S_OK );
} // ShutdownPlugin.




/////////////////////////////////////////////////////////////////////////////
//
// [CreatePlaylistParser]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParserPlugin::CreatePlaylistParser(
                            IWMSCommandContext *pCommandContext,
                            IWMSContext *pUserContext,
                            IWMSContext *pPresentationContext,
                            DWORD dwFlags,
                            IWMSClassObject *pFactory,
                            IWMSBufferAllocator *pBufferAllocator,
                            IWMSPlaylistParserPluginCallback *pCallback,
                            QWORD qwContext
                            )
{
    HRESULT hr = S_OK;
    
    CComPtr<CComSDKSamplePlaylistParser> spParser;

    if ( NULL == pCallback )
    {
        return( E_INVALIDARG );
    }

    spParser = new CComSDKSamplePlaylistParser;
    if ( spParser == NULL )
    {
        return( E_OUTOFMEMORY );
    }

    hr = spParser->Initialize(
                    pCommandContext,
                    pUserContext,
                    pPresentationContext,
                    m_spServerContext,
                    m_spNamedValues,
                    dwFlags,
                    pFactory
                    );
    if ( FAILED( hr ) )
    {
        // When we return a failed HRESULT, we should not make the callback
        return( hr );
    }

    pCallback->OnCreatePlaylistParser( hr, spParser, qwContext );

    // We called the callback, so we MUST return S_OK here.
    return( S_OK );
} // CreatePlaylistParser.






/////////////////////////////////////////////////////////////////////////////
//
//                             PLAYLIST PARSER
//
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//
// [CSDKSamplePlaylistParser]
//
/////////////////////////////////////////////////////////////////////////////
CSDKSamplePlaylistParser::CSDKSamplePlaylistParser()
{
} // CSDKSamplePlaylistParser.



/////////////////////////////////////////////////////////////////////////////
//
// [~CSDKSamplePlaylistParser]
//
/////////////////////////////////////////////////////////////////////////////
CSDKSamplePlaylistParser::~CSDKSamplePlaylistParser()
{
} // ~CSDKSamplePlaylistParser.



/////////////////////////////////////////////////////////////////////////////
//
// [Initialize]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT
CSDKSamplePlaylistParser::Initialize(
    IWMSCommandContext *pCommandContext,
    IWMSContext *pUserContext,
    IWMSContext *pPresentationContext,
    IWMSContext *pServerContext,
    IWMSNamedValues *pNamedValues,
    DWORD dwFlags,
    IWMSClassObject *pClassFactory
    )
{
    //
    // We don't really use any of these parameters later on.
    // We are merely illustrating how you would hold a reference to it
    // using smart pointers if you did need to use it later on.
    //

    m_spCommandContext = pCommandContext;
    m_spUserContext = pUserContext;
    m_spPresentationContext = pPresentationContext;
    m_spServerContext = pServerContext;
    m_spNamedValues = pNamedValues;
    m_spClassFactory = pClassFactory;

    return( S_OK );
} // Initialize.






/////////////////////////////////////////////////////////////////////////////
//
// [Shutdown]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT
CSDKSamplePlaylistParser::Shutdown()
{
    m_spCommandContext.Release();
    m_spUserContext.Release();
    m_spPresentationContext.Release();
    m_spServerContext.Release();
    m_spNamedValues.Release();
    m_spClassFactory.Release();
    
    return( S_OK );
} // Shutdown.






/////////////////////////////////////////////////////////////////////////////
//
// [ReadPlaylist]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParser::ReadPlaylist( 
                                INSSBuffer *pIBuffer,
                                IXMLDOMDocument *pPlayList,
                                IWMSPlaylistParserCallback *pCallback,
                                QWORD qwContext
                                )
{
    HRESULT hr = S_OK;
    BYTE *pbBuffer = NULL;
    BYTE *pbEndBuffer = NULL;
    BYTE *pbUrl = NULL;
    BYTE *pbLineEnd = NULL;
    BYTE *pbEndUrl = NULL;

    DWORD dwLength = 0;
    BYTE bPrevious = '\0';
    BSTR bstrUrl = NULL;
    
    CComPtr<IXMLDOMNode> spWsxNode;
    CComPtr<IXMLDOMElement> spPlayListMediaEntry;
    CComPtr<IXMLDOMElement> spPlayListTopEntry;
    CComPtr<IXMLDOMNode> spOldPlayListEntry;

    if ( ( NULL == pIBuffer ) 
        || ( NULL == pPlayList )
        || ( NULL == pCallback ) )
    {
        return( E_INVALIDARG );
    }

    //
    // Create the NODE_PROCESSING_INSTRUCTION
    // (i.e. "<?wsx version='1.0'?>
    //

    BSTR bstrWsx = SysAllocString( L"wsx" );
    BSTR bstrEmpty = SysAllocString( L"" );
    BSTR bstrVersion = SysAllocString( L"version='1.0'" );
    BSTR bstrSmil = SysAllocString( L"smil" );
    BSTR bstrTagName = SysAllocString( L"media" );
    BSTR bstrAttrName = SysAllocString( L"src" );

    if ( ( NULL == bstrWsx )
        || ( NULL == bstrEmpty )
        || ( NULL == bstrVersion )
        || ( NULL == bstrSmil )
        || ( NULL == bstrTagName )
        || ( NULL == bstrAttrName ) )
    {
        hr = E_OUTOFMEMORY;
        goto done;
    }

    VARIANT varNodeType;
    VariantInit( &varNodeType );
    V_VT( &varNodeType ) = VT_I4;
    V_I4( &varNodeType ) = NODE_PROCESSING_INSTRUCTION;
    
    hr = pPlayList->createNode(
                        varNodeType,
                        bstrWsx,
                        bstrEmpty,
                        &spWsxNode
                        );
    
    VariantClear( &varNodeType );

    if ( FAILED(hr) )
    {
        goto done;
    }

    hr = pPlayList->appendChild( spWsxNode, &spOldPlayListEntry );
    if ( FAILED(hr) )
    {
        goto done;
    }

    hr = spWsxNode->put_text( bstrVersion );
    if ( FAILED(hr) )
    {
        goto done;
    }

    //
    // Create the top level SMIL tag
    //

    hr = pPlayList->createElement( bstrSmil, &spPlayListTopEntry );
    if ( FAILED(hr) )
    {
        goto done;
    }

    spOldPlayListEntry = NULL;
    hr = pPlayList->appendChild( spPlayListTopEntry, &spOldPlayListEntry );
    if ( FAILED(hr) )
    {
        goto done;
    }

    //
    // Get a pointer to the playlist in preperation for parsing
    //

    hr = pIBuffer->GetBufferAndLength( &pbBuffer, &dwLength );
    if ( FAILED( hr ) )
    {
        goto done;
    }

    //
    // Walk through the playlist and extract each line
    //

    pbEndBuffer = pbBuffer + dwLength;
    pbUrl = pbBuffer;

    while ( pbUrl < pbEndBuffer )
    {
        //
        // Seek the beginning of the next filled line.
        // Skip any white spaces or blank lines in the middle.
        //
        while( ( pbUrl < pbEndBuffer )
                && isspace( *pbUrl ) )
        {
            ++pbUrl;
        }

        if( pbUrl >= pbEndBuffer )
        {
            break;  // end of playlist
        }

        //
        // Seek the end of the current line
        //
        pbLineEnd = pbUrl + 1;
        while( ( pbLineEnd < pbEndBuffer )
                && ( '\n' != *pbLineEnd )
                && ( '\r' != *pbLineEnd ) )
        {
            ++pbLineEnd;
        }

        //
        // If the line begins with '#', then the entire line is a comment.
        // Skip it and continue in the next line.
        //
        if( '#' == *pbUrl )
        {
            pbUrl = pbLineEnd;
            continue;
        }

        //
        // Find the end of the URL.
        // Trailing spaces are not to be considered part of the URL.
        // A comma may be used at the end of the line as an optional separator,
        // it's not to be considered part of the URL.
        //

        pbEndUrl = pbLineEnd;

        while( ( pbUrl < pbEndUrl )
            && isspace( pbEndUrl[-1] ) )
        {
            --pbEndUrl;
        }

        if( ( pbUrl < pbEndUrl )
            && ( ',' == pbEndUrl[-1] ) )
        {
            --pbEndUrl;
        }

        while( ( pbUrl < pbEndUrl )
            && isspace( pbEndUrl[-1] ) )
        {
            --pbEndUrl;
        }

        //
        // If the line contains simply a comma,
        // discard it and continue in the next line
        //
        if( pbUrl == pbEndUrl )
        {
            pbUrl = pbLineEnd;
            continue;
        }

        //
        // Copy the URL to a BSTR
        //

        int cchNeeded = MultiByteToWideChar(
                                CP_ACP,
                                0,
                                (LPCSTR)pbUrl,
                                (int)( pbEndUrl - pbUrl ),
                                NULL,
                                0
                                );
        if( 0 >= cchNeeded )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            goto done;
        }

        bstrUrl = SysAllocStringLen( NULL, cchNeeded );
        if( NULL == bstrUrl )
        {
            hr = E_OUTOFMEMORY;
            goto done;
        }

        int cchConverted = MultiByteToWideChar(
                                    CP_ACP,
                                    0,
                                    (LPCSTR)pbUrl,
                                    (int)( pbEndUrl - pbUrl ),
                                    bstrUrl,
                                    cchNeeded
                                    );
        if( cchConverted != cchNeeded )
        {
            hr = HRESULT_FROM_WIN32( ERROR_NO_UNICODE_TRANSLATION );
            goto done;
        }

        //
        // Create the corresponding MEDIA tag in the playlist
        //

        hr = pPlayList->createElement(
                            bstrTagName,
                            &spPlayListMediaEntry
                            );
        if ( FAILED( hr ) )
        {
            goto done;
        }

        //
        // Set the src attribute on the MEDIA tag to the URL
        //

        VARIANT varAttribute;
        VariantInit( &varAttribute );
        V_VT( &varAttribute ) = VT_BSTR;
        V_BSTR( &varAttribute ) = bstrUrl;
        
        hr = spPlayListMediaEntry->setAttribute( bstrAttrName, varAttribute );

        V_BSTR( &varAttribute ) = NULL;
        V_VT( &varAttribute ) = VT_EMPTY;
        VariantClear( &varAttribute );

        if ( FAILED( hr ) )
        {
            goto done;
        }

        //
        // Add the MEDIA tag to the playlist
        //

        spOldPlayListEntry = NULL;
        hr = spPlayListTopEntry->appendChild( spPlayListMediaEntry, &spOldPlayListEntry );            
        if ( FAILED( hr ) )
        {
            goto done;
        }

        spPlayListMediaEntry.Release();

        //
        // Continue in the next line
        //

        pbUrl = pbLineEnd;
    }

    //
    // Notify the callback the playlist was successfully parsed
    //

    pCallback->OnReadPlaylist( S_OK, qwContext );

done:
    
    if ( NULL != bstrWsx )
    {
        SysFreeString( bstrWsx );
        bstrWsx = NULL;
    }

    if ( NULL != bstrEmpty )
    {
        SysFreeString( bstrEmpty );
        bstrEmpty = NULL;
    }

    if ( NULL != bstrVersion )
    {
        SysFreeString( bstrVersion );
        bstrVersion = NULL;
    }

    if ( NULL != bstrSmil )
    {
        SysFreeString( bstrSmil );
        bstrSmil = NULL;
    }

    if ( NULL != bstrTagName )
    {
        SysFreeString( bstrTagName );
        bstrTagName = NULL;
    }

    if ( NULL != bstrAttrName )
    {
        SysFreeString( bstrAttrName );
        bstrAttrName = NULL;
    }

    if ( NULL != bstrUrl )
    {
        SysFreeString( bstrUrl );
        bstrUrl = NULL;
    }

    return( hr );
} // ReadPlaylist.



/////////////////////////////////////////////////////////////////////////////
//
// [WritePlaylist]
//
/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParser::WritePlaylist( 
                        IXMLDOMDocument *pPlayList,
                        IWMSPlaylistParserCallback *pCallback,
                        QWORD qwContext
                        )
{
    return( E_NOTIMPL );
} // WritePlaylist.



/////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE 
CSDKSamplePlaylistParser::ReadPlaylistFromDirectory( 
                                IWMSDirectory __RPC_FAR *pDirectory,
                                LPWSTR pszwFilePattern,
                                IXMLDOMDocument __RPC_FAR *pPlaylist,
                                IWMSPlaylistParserCallback __RPC_FAR *pCallback,
                                QWORD qwContext
                                )
{
    return( E_NOTIMPL );
} // ReadPlaylistFromDirectory.

