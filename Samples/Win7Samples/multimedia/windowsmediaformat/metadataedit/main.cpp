//******************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName: main.cpp
//
// Abstract: Sample to edit and view metadata from a windows media file.
//
//******************************************************************************

#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <wmsdk.h>
#include <strsafe.h>

//------------------------------------------------------------------------------
// Macros
// ------------------------------------------------------------------------------
#ifndef SAFE_RELEASE

#define SAFE_RELEASE( x )       \
    if( NULL != x )             \
    {                           \
        x->Release( );          \
        x = NULL;               \
    }

#endif // SAFE_RELEASE

#ifndef SAFE_DELETE

#define SAFE_DELETE( x )        \
    if( NULL != x )             \
    {                           \
        delete x;               \
        x = NULL;               \
    }

#endif // SAFE_DELETE

#ifndef SAFE_ARRAYDELETE

#define SAFE_ARRAYDELETE( x )   \
    if( NULL != x )             \
    {                           \
        delete [] x;            \
        x = NULL;               \
    }

#endif // SAFE_ARRAYDELETE

//------------------------------------------------------------------------------
// Name: EditorOpenFile()
// Desc: Creates metadata editor, opens file, and gets header info interface.
//------------------------------------------------------------------------------
HRESULT EditorOpenFile( __in LPWSTR pwszInFile, 
                        IWMMetadataEditor ** ppEditor,
                        IWMHeaderInfo ** ppHeaderInfo, 
                        IWMHeaderInfo3 ** ppHeaderInfo3 )
{
    if( ( NULL == pwszInFile ) ||
        ( NULL == ppEditor ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    do
    {
        hr = WMCreateEditor( ppEditor );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not create Metadata Editor ( hr=0x%08x ).\n" ), hr );
            break;
        }

        hr = ( *ppEditor )->Open( pwszInFile );
        if( FAILED ( hr ) )
        {
            _tprintf( _T( "Could not open the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }

        if( NULL != ppHeaderInfo )
        {
            hr = ( *ppEditor )->QueryInterface( IID_IWMHeaderInfo, 
                                                (void **)ppHeaderInfo );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not QI for IWMHeaderInfo ( hr=0x%08x ).\n" ), hr );
                break;
            }
        }

        if( NULL != ppHeaderInfo3 )
        {
            hr = ( *ppEditor )->QueryInterface( IID_IWMHeaderInfo3, 
                                                (void **)ppHeaderInfo3 );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not QI for IWMHeaderInfo3 ( hr=0x%08x ).\n" ), hr );
                break;
            }
        }
    }
    while( FALSE );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: ModifyAttrib()
// Desc: Changes the value of the specified attribute.
//------------------------------------------------------------------------------
HRESULT ModifyAttrib( __in LPWSTR pwszInFile, 
                      WORD wStreamNum, 
                      WORD wAttribIndex, 
                      WORD wAttribType, 
                      __in LPWSTR pwszAttribValue, 
                      WORD wLangIndex )
{
    if( ( NULL == pwszInFile ) ||
        ( NULL == pwszAttribValue ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    IWMMetadataEditor   * pEditor       = NULL;
    IWMHeaderInfo3      * pHeaderInfo3  = NULL;
    BYTE*               pbAttribValue   = NULL;
    DWORD               dwAttribValueLen = 0;
    WMT_ATTR_DATATYPE   AttribDataType  = (WMT_ATTR_DATATYPE) wAttribType;
    DWORD               dwAttribValue   = 0;
    WORD                wAttribValue    = 0;
    QWORD               qwAttribValue   = 0;
    BOOL                fAttribValue    = 0;

    do
    {
        hr = EditorOpenFile( pwszInFile, &pEditor, NULL, &pHeaderInfo3 );
        if(FAILED( hr ) )
        {
            break;
        }

        switch(AttribDataType )
        {
        case WMT_TYPE_DWORD:
            dwAttribValue = _wtoi( pwszAttribValue );                               
            dwAttribValueLen = sizeof( DWORD );
            pbAttribValue = (BYTE *)&dwAttribValue;

            break;

        case WMT_TYPE_WORD:
            wAttribValue = (WORD)_wtoi( pwszAttribValue );                                
            dwAttribValueLen = sizeof(WORD);
            pbAttribValue = (BYTE *)&wAttribValue;

            break;

        case WMT_TYPE_QWORD:
            qwAttribValue = _wtoi64( pwszAttribValue );                             
            dwAttribValueLen = sizeof(QWORD );
            pbAttribValue = (BYTE *)&qwAttribValue;

            break;
                    
        case WMT_TYPE_STRING:
            dwAttribValueLen = ( wcslen( pwszAttribValue ) + 1 ) * sizeof( WCHAR );
            pbAttribValue = (BYTE *)pwszAttribValue;

            break;

        case WMT_TYPE_BOOL:
            fAttribValue = (BOOL)_wtoi( pwszAttribValue );                              
            dwAttribValueLen = sizeof(BOOL);
            pbAttribValue = (BYTE *)&fAttribValue;

            break;

        default:
            _tprintf( _T( "Unsupported data type for SetAttribute\n" ) );
            hr = E_INVALIDARG;
            break;
        }

        hr = pHeaderInfo3->ModifyAttribute( wStreamNum,
                                            wAttribIndex,
                                            AttribDataType, 
                                            wLangIndex,
                                            pbAttribValue,
                                            dwAttribValueLen );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "ModifyAttribute failed for Attribute index = %d ( hr=0x%08x ).\n" ), 
                wAttribIndex, hr );
            break;
        }

        hr = pEditor->Flush();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not flush the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }
    }
    while( FALSE );
    
    SAFE_RELEASE( pHeaderInfo3 );
    SAFE_RELEASE( pEditor );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: AddAttrib()
// Desc: Adds an attribute by using IWMHeaderInfo3::AddAttribute.
//------------------------------------------------------------------------------
HRESULT AddAttrib( __in LPWSTR pwszInFile, 
                   WORD wStreamNum, 
                   __in LPWSTR pwszAttribName, 
                   WORD wAttribType, 
                   __in LPWSTR pwszAttribValue, 
                   WORD wLangIndex )
{
    if( ( NULL == pwszInFile ) ||
        ( NULL == pwszAttribName ) ||
        ( NULL == pwszAttribValue ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    IWMMetadataEditor   * pEditor       = NULL;
    IWMHeaderInfo3      * pHeaderInfo3  = NULL;

    BYTE*               pbAttribValue   = NULL;
    DWORD               dwAttribValueLen = 0;

    WMT_ATTR_DATATYPE   AttribDataType  = ( WMT_ATTR_DATATYPE) wAttribType;

    DWORD               dwAttribValue   = 0;
    WORD                wAttribValue    = 0;
    QWORD               qwAttribValue   = 0;
    BOOL                fAttribValue    = 0;

    do
    {
        hr = EditorOpenFile( pwszInFile, &pEditor, NULL, &pHeaderInfo3 );
        if(FAILED( hr ) )
        {
            break;
        }

        switch( AttribDataType )
        {
        case WMT_TYPE_DWORD:
            dwAttribValue = _wtoi( pwszAttribValue );                               
            dwAttribValueLen = sizeof( DWORD );
            pbAttribValue = (BYTE *)&dwAttribValue;

            break;

        case WMT_TYPE_WORD:
            wAttribValue = (WORD)_wtoi( pwszAttribValue );                                
            dwAttribValueLen = sizeof(WORD);
            pbAttribValue = (BYTE *)&wAttribValue;

            break;

        case WMT_TYPE_QWORD:
            qwAttribValue = _wtoi64( pwszAttribValue );                             
            dwAttribValueLen = sizeof(QWORD );
            pbAttribValue = (BYTE *)&qwAttribValue;

            break;
                    
        case WMT_TYPE_STRING:
            dwAttribValueLen = ( wcslen( pwszAttribValue ) + 1 )* sizeof( WCHAR );
            pbAttribValue = (BYTE *)pwszAttribValue;

            break;

        case WMT_TYPE_BOOL:
            fAttribValue = (BOOL)_wtoi( pwszAttribValue );                              
            dwAttribValueLen = sizeof(BOOL);
            pbAttribValue = (BYTE *)&fAttribValue;

            break;

        default:
            _tprintf( _T( "Unsupported data type for SetAttribute\n" ) );
            hr = E_INVALIDARG;
            break;
        }

        hr = pHeaderInfo3->AddAttribute( wStreamNum,
                                         pwszAttribName,
                                         NULL,
                                         AttribDataType, 
                                         wLangIndex,
                                         pbAttribValue,
                                         dwAttribValueLen );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "AddAttribute failed for Attribute name %ws ( hr=0x%08x ).\n" ), 
                pwszAttribName, hr );
            break;
        }

        hr = pEditor->Flush();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not flush the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }
    }
    while( FALSE );
    
    SAFE_RELEASE( pHeaderInfo3 );
    SAFE_RELEASE( pEditor );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: SetAttrib()
// Desc: Adds an attribute by using IWMHeaderInfo::SetAttribute.
//------------------------------------------------------------------------------
HRESULT SetAttrib( __in LPWSTR pwszInFile, 
                   WORD wStreamNum, 
                   __in LPWSTR pwszAttribName, 
                   WORD wAttribType, 
                   __in LPWSTR pwszAttribValue )
{
    if( ( NULL == pwszInFile ) ||
        ( NULL == pwszAttribName ) ||
        ( NULL == pwszAttribValue ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    IWMMetadataEditor   * pEditor       = NULL;
    IWMHeaderInfo       * pHeaderInfo   = NULL;

    BYTE*               pbAttribValue   = NULL;
    WORD                wAttribValueLen = 0;

    WMT_ATTR_DATATYPE   AttribDataType  = ( WMT_ATTR_DATATYPE ) wAttribType;

    DWORD               dwAttribValue   = 0;
    WORD                wAttribValue    = 0;
    QWORD               qwAttribValue   = 0;
    BOOL                fAttribValue    = 0;

    do
    {
        hr = EditorOpenFile( pwszInFile, &pEditor, &pHeaderInfo, NULL);
        if(FAILED( hr ) )
        {
            break;
        }

        switch( AttribDataType )
        {
        case WMT_TYPE_DWORD:
            dwAttribValue = _wtoi( pwszAttribValue );                               
            wAttribValueLen = sizeof( DWORD );
            pbAttribValue = (BYTE *)&dwAttribValue;

            break;

        case WMT_TYPE_WORD:
            wAttribValue = (WORD)_wtoi( pwszAttribValue );                                
            wAttribValueLen = sizeof(WORD);
            pbAttribValue = (BYTE *)&wAttribValue;

            break;

        case WMT_TYPE_QWORD:
            qwAttribValue = _wtoi64( pwszAttribValue );                             
            wAttribValueLen = sizeof(QWORD );
            pbAttribValue = (BYTE *)&qwAttribValue;

            break;
                    
        case WMT_TYPE_STRING:
            wAttribValueLen = ( wcslen( pwszAttribValue ) + 1 )* sizeof( WCHAR );
            pbAttribValue = (BYTE *)pwszAttribValue;

            break;

        case WMT_TYPE_BOOL:
            fAttribValue = (BOOL)_wtoi( pwszAttribValue );                              
            wAttribValueLen = sizeof(BOOL);
            pbAttribValue = (BYTE *)&fAttribValue;

            break;

        default:
            _tprintf( _T( "Unsupported data type for SetAttribute\n" ) );
            hr = E_INVALIDARG;
            break;
        }

        hr = pHeaderInfo->SetAttribute( wStreamNum,
                                        pwszAttribName,
                                        AttribDataType,                                        
                                        pbAttribValue,
                                        wAttribValueLen );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "SetAttribute failed for Attribute name %ws ( hr=0x%08x ).\n" ), 
                pwszAttribName, hr );
            break;
        }

        hr = pEditor->Flush();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not flush the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }
    }
    while( FALSE );
    
    SAFE_RELEASE( pHeaderInfo );
    SAFE_RELEASE( pEditor );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: PrintListHeader()
// Desc: Add column headings.
//------------------------------------------------------------------------------
void PrintListHeader()
{
    _tprintf( _T( "*\n" ) );
    _tprintf( _T( "* Idx  Name                   Stream Language Type  Value\n" ) );
    _tprintf( _T( "* ---  ----                   ------ -------- ----  -----\n" ) );
}

//------------------------------------------------------------------------------
// Name: PrintAttribute()
// Desc: Display the specified attribute.
//------------------------------------------------------------------------------
HRESULT PrintAttribute( WORD wIndex,
                        WORD wStream,
                        __in LPWSTR wszName,
                        WMT_ATTR_DATATYPE AttribDataType,
                        WORD wLangID,
                        BYTE * pbValue,
                        DWORD dwValueLen )
{
    WCHAR pwszValue[256];
    WCHAR wszNum[256];

    ZeroMemory( pwszValue, sizeof( pwszValue ) );
    ZeroMemory( wszNum, sizeof( wszNum ) );

    //
    // Make the data type string
    //
    WCHAR * pwszType = L"Unknown";
    WCHAR * pTypes[] = 
    { 
        L"DWORD", 
        L"STRING", 
        L"BINARY", 
        L"BOOL", 
        L"QWORD", 
        L"WORD", 
        L"GUID" 
    };

    if( ( sizeof( pTypes ) / sizeof( pTypes[0] ) ) > AttribDataType )
    {
        pwszType = pTypes[AttribDataType];
    }

    //
    // The attribute value.
    //

    switch ( AttribDataType )
    {
    //
    // string
    //
    case WMT_TYPE_STRING:
        if( 0 == dwValueLen )
        {
            StringCbCatW( pwszValue, sizeof( pwszValue ), L"***** NULL *****" );
        }
        else
        {
            if( 0xFEFF == *(WCHAR*)pbValue )
            {
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"UTF-16LE BOM+" );
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"\"" );
                if( 4 <= dwValueLen )
                {
                    StringCbCatW( pwszValue, sizeof( pwszValue ), (WCHAR *)pbValue + 1 );
                }
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"\"" );
            }
            else if( 0xFFFE == *(WCHAR*)pbValue )
            {
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"UTF-16BE BOM+" );
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"\"" );
                if( 4 <= dwValueLen )
                {
                    StringCbCatW( pwszValue, sizeof( pwszValue ), (WCHAR *)pbValue + 1 );
                }
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"\"" );
            }
            else
            {
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"\"" );
                if( 2 <= dwValueLen )
                {
                    StringCbCatW( pwszValue, sizeof( pwszValue ), (WCHAR *)pbValue );
                }
                StringCbCatW( pwszValue, sizeof( pwszValue ), L"\"" );
            }
        }
        break;

    //
    // Binary
    //
    case WMT_TYPE_BINARY:
        StringCbCatW( pwszValue, sizeof( pwszValue ), L"[" );
        _itow_s( dwValueLen, wszNum, 10 );
        StringCbCatW( pwszValue, sizeof( pwszValue ), wszNum );
        StringCbCatW( pwszValue, sizeof( pwszValue ), L" bytes]" );
        break;

    //
    // Boolean
    //
    case WMT_TYPE_BOOL:
        if( *( (BOOL* )pbValue ) == FALSE )
        {
            StringCbCatW( pwszValue, sizeof( pwszValue ), L"False" );
        }
        else
        {
            StringCbCatW( pwszValue, sizeof( pwszValue ), L"True" );
        }
        break;

    //
    // DWORD
    //
    case WMT_TYPE_DWORD:
        StringCbPrintfW( wszNum, sizeof(wszNum), L"%ld, 0x%08lx", 
            ( (DWORD *)pbValue )[0], ( (DWORD *)pbValue )[0] );
        StringCbCatW( pwszValue, sizeof( pwszValue ), wszNum );
        break;

    //
    // QWORD
    //
    case WMT_TYPE_QWORD:
        _ui64tow_s( *( (QWORD* )pbValue ), wszNum, sizeof(wszNum) / sizeof(wszNum[0]), 10 );
        StringCbCatW( pwszValue, sizeof( pwszValue ), wszNum );
        StringCbPrintfW( wszNum, sizeof( wszNum ), L", 0x%08lx%08lx",
            ( (DWORD *)pbValue )[1], ( (DWORD *)pbValue )[0] );
        StringCbCatW( pwszValue, sizeof( pwszValue ), wszNum );
        break;

    //
    // WORD
    //
    case WMT_TYPE_WORD:
        StringCbPrintfW( wszNum, sizeof( wszNum ), L"%d, 0x%04x",
            ( ( WORD* )pbValue )[0], ( ( WORD* )pbValue )[0] );
        StringCbCatW( pwszValue, sizeof( pwszValue ), wszNum );
        break;

    //
    // GUID
    //
    case WMT_TYPE_GUID:
        if( !StringFromGUID2( *(GUID *)pbValue, pwszValue, 256 ) )
        {
            StringCbCatW( pwszValue, sizeof( pwszValue ), L"ERROR" );
        }
        break;

    default:
        StringCbPrintfW( pwszValue, sizeof( pwszValue ), L"Bad data type (%hu): value not displayed",
            AttribDataType );
        break;
    }

    //
    // Dump the string to the screen.
    //
     _tprintf( _T( "* %3u  %-25ls %3hu  %3hu  %7ls  %ls\n" ), 
         wIndex, wszName, wStream, wLangID, pwszType, pwszValue );

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: DeleteAttrib()
// Desc: Delete the specified attribute.
//------------------------------------------------------------------------------
HRESULT DeleteAttrib( __in LPWSTR pwszInFile, 
                      WORD wStreamNum, 
                      WORD wAttribIndex )
{
    HRESULT hr = S_OK;
        
    IWMMetadataEditor   * pEditor       = NULL;
    IWMHeaderInfo3      * pHeaderInfo3  = NULL;

    do
    {
        hr = EditorOpenFile( pwszInFile, &pEditor, NULL, &pHeaderInfo3 );
        if(FAILED( hr ) )
        {
            break;
        }

        hr = pHeaderInfo3->DeleteAttribute( wStreamNum, wAttribIndex );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "DeleteAttribute failed for stream = %d and index = %d ( hr=0x%08x ).\n" ), 
                wStreamNum, wAttribIndex, hr );
            break;
        }

        hr = pEditor->Flush();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not flush the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the file %ws ( hr=0x%08x ).\n" ), 
                pwszInFile, hr );
            break;
        }
    }
    while( FALSE );
    
    SAFE_RELEASE( pHeaderInfo3 );
    SAFE_RELEASE( pEditor );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: ShowAttributes3()
// Desc: Display all attributes for the specified stream,
//       using IWMHeaderInfo3.
//------------------------------------------------------------------------------
HRESULT ShowAttributes3( __in LPWSTR pwszInFile, WORD wStreamNum )
{
    HRESULT             hr              = S_OK;

    IWMMetadataEditor   * pEditor       = NULL;
    IWMHeaderInfo3      * pHeaderInfo3  = NULL;

    WCHAR*             pwszAttribName   = NULL;
    WORD               wAttribNameLen   = 0;
    WMT_ATTR_DATATYPE  wAttribType;
    WORD               wLanguageIndex   = 0;
    BYTE*              pbAttribValue    = NULL;
    DWORD              dwAttribValueLen = 0;

    do
    {
        hr = EditorOpenFile( pwszInFile, &pEditor, NULL, &pHeaderInfo3 );
        if(FAILED( hr ) )
            break;

        WORD wAttributeCount = 0;

        hr = pHeaderInfo3->GetAttributeCountEx( wStreamNum, &wAttributeCount);
        if(FAILED( hr ) )
        {
            _tprintf( _T( "GetAttributeCount failed for stream = %d ( hr=0x%08x ).\n" ), wStreamNum, hr );
            break;
        }

        PrintListHeader();

        for( WORD wAttribIndex = 0; wAttribIndex < wAttributeCount; wAttribIndex++)
        {
            SAFE_ARRAYDELETE( pwszAttribName );
            SAFE_ARRAYDELETE( pbAttribValue );

            hr = pHeaderInfo3->GetAttributeByIndexEx( wStreamNum,
                                                      wAttribIndex,
                                                      pwszAttribName,
                                                      &wAttribNameLen,
                                                      &wAttribType,
                                                      &wLanguageIndex,
                                                      pbAttribValue,
                                                      &dwAttribValueLen );
            if(FAILED( hr ) )
            {
                _tprintf( _T( "GetAttributeByIndexEx failed for index = %d ( hr=0x%08x ).\n" ), wAttribIndex, hr );
                break;
            }

            pwszAttribName = new WCHAR[wAttribNameLen];
            if( NULL == pwszAttribName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            pbAttribValue = new BYTE[dwAttribValueLen];
            if( NULL == pbAttribValue )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pHeaderInfo3->GetAttributeByIndexEx( wStreamNum,
                                                      wAttribIndex,
                                                      pwszAttribName,
                                                      &wAttribNameLen,
                                                      &wAttribType,
                                                      &wLanguageIndex,
                                                      pbAttribValue,
                                                      &dwAttribValueLen );
            if(FAILED( hr ) )
            {
                _tprintf( _T( "GetAttributeByIndexEx failed for index = %d ( hr=0x%08x ).\n" ), 
                    wAttribIndex, hr );
                break;
            }

            hr = PrintAttribute( wAttribIndex,
                                 wStreamNum,
                                 pwszAttribName,
                                 wAttribType,
                                 wLanguageIndex,
                                 pbAttribValue,
                                 dwAttribValueLen );
            if(FAILED( hr ) )
            {
                break;
            }
        }

        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the file %ws ( hr=0x%08x ).\n" ), pwszInFile, hr );
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pHeaderInfo3 );
    SAFE_RELEASE( pEditor );

    SAFE_ARRAYDELETE( pwszAttribName );
    SAFE_ARRAYDELETE( pbAttribValue );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: ShowAttributes()
// Desc: Display all attributes for the specified stream,
//       using IWMHeaderInfo.
//------------------------------------------------------------------------------
HRESULT ShowAttributes( __in LPWSTR pwszInFile, WORD wStreamNum )
{
    HRESULT             hr                  = S_OK;

    IWMMetadataEditor   * pEditor           = NULL;
    IWMHeaderInfo*      pHeaderInfo         = NULL;

    WCHAR               * pwszAttribName    = NULL;
    WORD                wAttribNameLen      = 0;
    WMT_ATTR_DATATYPE   wAttribType;
    BYTE                * pbAttribValue     = NULL;
    WORD                wAttribValueLen     = 0;

    do
    {
        hr = EditorOpenFile( pwszInFile, &pEditor, &pHeaderInfo, NULL );
        if(FAILED( hr ) )
        {
            break;
        }

        WORD wAttributeCount = 0;

        hr = pHeaderInfo->GetAttributeCount( wStreamNum, &wAttributeCount );
        if(FAILED( hr ) )
        {
            _tprintf( _T( "GetAttributeCount failed for stream = %d ( hr=0x%08x ).\n" ), 
                wStreamNum, hr );
            break;
        }

        PrintListHeader();

        for( WORD wAttribIndex = 0; wAttribIndex < wAttributeCount; wAttribIndex++ )
        {
            SAFE_ARRAYDELETE( pwszAttribName );
            SAFE_ARRAYDELETE( pbAttribValue );

            hr = pHeaderInfo->GetAttributeByIndex( wAttribIndex,
                                                   &wStreamNum,
                                                   pwszAttribName,
                                                   &wAttribNameLen,
                                                   &wAttribType,
                                                   pbAttribValue,
                                                   &wAttribValueLen );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "GetAttributeByIndex failed for index = %d ( hr=0x%08x ).\n" ), 
                    wAttribIndex, hr );
                break;
            }

            pwszAttribName = new WCHAR[ wAttribNameLen ];
            if( NULL == pwszAttribName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            pbAttribValue = new BYTE[ wAttribValueLen ];
            if( NULL == pbAttribValue )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = pHeaderInfo->GetAttributeByIndex( wAttribIndex,
                                                   &wStreamNum,
                                                   pwszAttribName,
                                                   &wAttribNameLen,
                                                   &wAttribType,
                                                   pbAttribValue,
                                                   &wAttribValueLen );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "GetAttributeByIndex failed for index = %d ( hr=0x%08x ).\n" ), 
                    wAttribIndex, hr );
                break;
            }

            hr = PrintAttribute( wAttribIndex, 
                                 wStreamNum, 
                                 pwszAttribName, 
                                 wAttribType, 
                                 0, 
                                 pbAttribValue, 
                                 wAttribValueLen );
            if( FAILED( hr ) )
            {
                break;
            }
        }
        
        hr = pEditor->Close();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Could not close the file %ws ( hr=0x%08x ).\n" ), pwszInFile, hr );
            break;
        }
    }
    while( FALSE );
    
    SAFE_RELEASE( pHeaderInfo );
    SAFE_RELEASE( pEditor );

    SAFE_ARRAYDELETE( pwszAttribName );
    SAFE_ARRAYDELETE( pbAttribValue );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: ConvertMBtoWC()
// Desc: Converts a string from multibyte to wide character.
//------------------------------------------------------------------------------
#ifndef UNICODE

HRESULT ConvertMBtoWC( LPCTSTR ptszInString, __out LPWSTR *ppwszOutString )
{
    if( ptszInString == NULL || ppwszOutString == NULL )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr          = S_OK;
    int     nSizeCount  = 0;

    *ppwszOutString     = NULL;

    do
    {
        //
        // Get the memory reqd for this string
        //
        nSizeCount = MultiByteToWideChar( CP_ACP, 0, ptszInString, -1, NULL, 0 );
        if( 0 ==  nSizeCount )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        *ppwszOutString = new WCHAR[ nSizeCount ];
        if( NULL == *ppwszOutString )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }

        if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInString, -1, *ppwszOutString, nSizeCount ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
            break;
        }
    }
    while( FALSE );
    
    if( FAILED( hr ) )
    {
        SAFE_ARRAYDELETE( *ppwszOutString );
        _tprintf( _T( "Internal error ( hr=0x%08x )\n" ), hr );
    }

    return( hr );
}

#endif // UNICODE

//------------------------------------------------------------------------------
// Name: Usage()
// Desc: Displays the command-line usage.
//------------------------------------------------------------------------------
void Usage()
{
    _tprintf( _T( "MetadataEdit\t <filename> show <stream number>\n" ) );
    _tprintf( _T( "\t\t <filename> show3 <stream number>\n" ) );
    _tprintf( _T( "\t\t <filename> delete <stream number> <attrib index>\n" ) );
    _tprintf( _T( "\t\t <filename> set <stream number> <attrib name> <attrib type> <attrib value>\n" ) );
    _tprintf( _T( "\t\t <filename> add <stream number> <attrib name> <attrib type> <attrib value> <attrib language>\n" ) );
    _tprintf( _T( "\t\t <filename> modify <stream number> <attrib index> <attrib type> <attrib value> <attrib language>\n" ) );

    _tprintf( _T( "\n Attrib Type can have one of the following values\n" ) );
    _tprintf( _T( "\t 0 - WMT_TYPE_DWORD\n" ) );
    _tprintf( _T( "\t 1 - WMT_TYPE_STRING\n" ) );
    _tprintf( _T( "\t 3 - WMT_TYPE_BOOL\n" ) );
    _tprintf( _T( "\t 4 - WMT_TYPE_QWORD\n" ) );
    _tprintf( _T( "\t 5 - WMT_TYPE_WORD\n" ) );
}

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: Entry point for the application. Processes command-line arguments.
//------------------------------------------------------------------------------
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    if( argc < 4)
    {
        Usage();
        return( E_INVALIDARG );
    }

    HRESULT hr              = S_OK;

    LPTSTR  ptszInFile      = (LPTSTR)argv[1];

    WORD    wStreamNum      = 0;
    WORD    wAttribIndex    = 0;

    LPTSTR  ptszAttribName  = NULL;
    LPTSTR  ptszAttribValue = NULL;

    WORD    wAttribType     = 0;
    WORD    wLangIndex      = 0;

#ifndef UNICODE

    WCHAR   * pwszAttribName    = NULL;
    WCHAR   * pwszAttribValue   = NULL;
    WCHAR   * pwszInFile        = NULL;

#endif

    do
    {
        // Initialize COM
        hr = CoInitialize( NULL );
        if( FAILED( hr ) )
        {
            break;
        }

#ifndef UNICODE
        hr = ConvertMBtoWC( ptszInFile, &pwszInFile );
        if( FAILED( hr ) )
        {
            break;
        }
#endif

        if( 0 == _tcsicmp( argv[2], _T( "show" ) ) )
        {
            if( 4 != argc )
            {
                Usage();
                hr = E_INVALIDARG;
                break;
            }

            wStreamNum = (WORD)_ttoi( argv[3] );

#ifndef UNICODE
            hr = ShowAttributes( pwszInFile, wStreamNum );
#else
            hr = ShowAttributes( ptszInFile, wStreamNum );
#endif

            if(FAILED( hr ) )
            {
                break;
            }
        }
        else if(0 == _tcsicmp( argv[2], _T( "show3" ) ) )
        {
            if( 4 != argc )
            {
                Usage();
                hr = E_INVALIDARG;
                break;
            }

            wStreamNum = (WORD)_ttoi( argv[3] );

#ifndef UNICODE
            hr = ShowAttributes3( pwszInFile, wStreamNum );
#else
            hr = ShowAttributes3( ptszInFile, wStreamNum );
#endif

            if(FAILED( hr ) )
            {
                break;
            }
        }
        else if(0 == _tcsicmp( argv[2], _T( "delete" ) ) )
        {
            if( 5 != argc )
            {
                Usage();
                hr = E_INVALIDARG;
                break;
            }

            wStreamNum = (WORD)_ttoi( argv[3] );
            wAttribIndex = (WORD)_ttoi( argv[4] );

#ifndef UNICODE
            hr = DeleteAttrib( pwszInFile, wStreamNum, wAttribIndex );
            if(FAILED( hr ) )
            {
                break;
            }

            hr = ShowAttributes3( pwszInFile, wStreamNum );
#else
            hr = DeleteAttrib( ptszInFile, wStreamNum, wAttribIndex );
            if(FAILED( hr ) )
            {
                break;
            }

            hr = ShowAttributes3( ptszInFile, wStreamNum );
#endif

            if(FAILED( hr ) )
            {
                break;
            }
        }
        else if(0 == _tcsicmp( argv[2], _T( "set" ) ) )
        {
            if( 7 != argc )
            {
                Usage();
                hr = E_INVALIDARG;
                break;
            }

            ptszAttribName = (LPTSTR)argv[4];
            ptszAttribValue = (LPTSTR)argv[6];

            wStreamNum = (WORD)_ttoi( argv[3] );
            wAttribType = (WORD)_ttoi( argv[5] ); 

#ifndef UNICODE
            hr = ConvertMBtoWC( ptszAttribName, &pwszAttribName );
            if( FAILED( hr ) )
            {
                break;
            }
            
            hr = ConvertMBtoWC( ptszAttribValue, &pwszAttribValue );
            if( FAILED( hr ) )
            {
                break;
            }
            
            hr = SetAttrib( pwszInFile, 
                            wStreamNum, 
                            pwszAttribName, 
                            wAttribType, 
                            pwszAttribValue );
#else
            hr = SetAttrib( ptszInFile, 
                            wStreamNum, 
                            ptszAttribName, 
                            wAttribType, 
                            ptszAttribValue );
#endif

            if(FAILED( hr ) )
            {
                break;
            }
        }
        else if(0 == _tcsicmp( argv[2], _T( "add" ) ) )
        {
            if( 8 != argc )
            {
                Usage();
                hr = E_INVALIDARG;
                break;
            }

            ptszAttribName = (LPTSTR)argv[4];
            ptszAttribValue = (LPTSTR)argv[6];

            wStreamNum = (WORD)_ttoi( argv[3] );
            wAttribType = (WORD)_ttoi( argv[5] );
            wLangIndex = (WORD)_ttoi( argv[7] );

#ifndef UNICODE

            hr = ConvertMBtoWC( ptszAttribName, &pwszAttribName );
            if( FAILED( hr ) )
            {
                break;
            }
            
            hr = ConvertMBtoWC( ptszAttribValue, &pwszAttribValue );
            if( FAILED( hr ) )
            {
                break;
            }
            
            hr = AddAttrib( pwszInFile, 
                            wStreamNum, 
                            pwszAttribName, 
                            wAttribType, 
                            pwszAttribValue, 
                            wLangIndex );
#else
            hr = AddAttrib( ptszInFile, 
                            wStreamNum, 
                            ptszAttribName, 
                            wAttribType, 
                            ptszAttribValue, 
                            wLangIndex );
#endif

            if(FAILED( hr ) )
            {
                break;
            }
        }
        else if(0 == _tcsicmp( argv[2], _T( "modify" ) ) )
        {
            if( 8 != argc )
            {
                Usage();
                hr = E_INVALIDARG;
                break;
            }

            ptszAttribValue = (LPTSTR)argv[6];

            wStreamNum = (WORD)_ttoi( argv[3] );
            wAttribIndex = (WORD)_ttoi( argv[4] );
            wAttribType = (WORD)_ttoi( argv[5] );         
            wLangIndex = (WORD)_ttoi( argv[7] );

#ifndef UNICODE
            hr = ConvertMBtoWC( ptszAttribValue, &pwszAttribValue );
            if( FAILED( hr ) )
            {
                break;
            }
            
            hr = ModifyAttrib( pwszInFile, 
                               wStreamNum, 
                               wAttribIndex, 
                               wAttribType, 
                               pwszAttribValue, 
                               wLangIndex );
#else
            hr = ModifyAttrib( ptszInFile, 
                               wStreamNum, 
                               wAttribIndex, 
                               wAttribType, 
                               ptszAttribValue, 
                               wLangIndex );
#endif

            if( FAILED( hr ) )
            {
                break;
            }
        }
        else
        {
            Usage();
            hr = E_INVALIDARG;
            break;
        }
    }
    while( FALSE );

#ifndef UNICODE

    SAFE_ARRAYDELETE( pwszInFile );
    SAFE_ARRAYDELETE( pwszAttribName );
    SAFE_ARRAYDELETE( pwszAttribValue );

#endif // UNICODE
    CoUninitialize();

    return( hr );
}
