//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            main.cpp
//      
// Abstract:            Entry point for WMVNetWrite sample. This sample 
//                      demonstrates how to stream a Windows Media file over 
//                      the network to other clients by sending ("Netwriting")its 
//                      compressed samples.
//                      
//
//*****************************************************************************

#include <TCHAR.H>
#include <stdio.h>
#include "wmsdk.h"
#include "NetWrite.h"


//----------------------------------------------------------------------------
// Name: ConvertTCharToWChar
// Desc: Helper function to convert a TCHAR string to a WCHAR string.
//
// ptszInput: Contains the input string.
// ppwszOutput: Receives the output string. 
//
// The caller must delete the returned WCHAR string.
//----------------------------------------------------------------------------

HRESULT ConvertTCharToWChar( LPCTSTR ptszInput, __out LPWSTR * ppwszOutput )
{
    int cchOutput = 0;
    
    if( NULL == ptszInput || NULL == ppwszOutput )
    {
        return( E_INVALIDARG );
    }

    //
    // Get the size needed for the output buffer.
    //
#ifdef UNICODE
    cchOutput = wcslen( ptszInput ) + 1;
#else //UNICODE
    cchOutput = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 );
    if( 0 == cchOutput )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
#endif// UNICODE

    // Allocate the buffer.
    *ppwszOutput = new WCHAR[ cchOutput ];
    if( NULL == *ppwszOutput)
    {
        return( E_OUTOFMEMORY );
    }

    // Convert the input string. 

#ifdef UNICODE
    wcsncpy_s( *ppwszOutput, cchOutput, ptszInput, cchOutput - 1 );
#else //UNICODE
    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *ppwszOutput, cchOutput ) )
    {
        if(*ppwszOutput )
        {
            delete *ppwszOutput;
            *ppwszOutput = NULL;
        }

        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }        
#endif// UNICODE

    return( S_OK );
}


//----------------------------------------------------------------------------
// Name: Usage
// Desc: Displays the valid command-line arguments for this application.
//----------------------------------------------------------------------------

void Usage()
{
   _tprintf( _T( "wmvnetwrite -i <infile> [-p <portnum>] [-c <maxclient>] [-s <server URL>]\n" ) );
   _tprintf( _T( "\tinfile\t= Input WMV file name\n" ) );
   _tprintf( _T( "\tportnum\t= Port Number (for incoming connections)\n" ) );
   _tprintf( _T( "\tmaxclient = Maximum Clients allowed to connect\n" ) );
   _tprintf( _T( "\tserver URL = URL of Server for Push Distribution\n" ) );
}


//----------------------------------------------------------------------------
// Name: _tmain
// Desc: Entry point for the application.
//----------------------------------------------------------------------------

int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    DWORD   dwPortNum       = 8080;     // Default port number.
    TCHAR*  ptszFileName    = NULL;     // File to broadcast.
    TCHAR*  ptszServerURL   = NULL;     // URL on the server, for push distribution
    int     nMaxClient  = 10;           // Maximum number of clients that can connect.
    HRESULT hr = S_OK;

    // Loop through the command-line arguments. On failure, display the correct
    // usage and exit.
    for( int i = 1; i < argc; i ++ )
    {
        if( 0 == _tcsicmp( argv[i], _T( "-p" ) ) )
        {
            i++;
            
            if( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }
            
            int retval = _stscanf_s( argv[i], _T( "%d" ), &dwPortNum );
            if( retval == 0 )
            {
                Usage();
                return( E_INVALIDARG );
            }
            continue;
            
        }
        
        if( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
        {
            i++;
            
            if( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }
            
            ptszFileName = argv[i];
            continue;
        }
        
        if( 0 == _tcsicmp( argv[i] , _T( "-c" ) ) )
        {
            i++;
            
            if(i == argc)
            {
                Usage();
                return ( E_INVALIDARG );
            }
            
            int retval = _stscanf_s( argv[i], _T( "%d" ), &nMaxClient );
            if( 0 == retval )
            {
                Usage();
                return( E_INVALIDARG );
            }
            continue;
        }

        if( 0 == _tcsicmp( argv[i], _T( "-s" ) ) )
        {
            i++;
            
            if( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }
            
            ptszServerURL = argv[i];
            continue;
        }
    }
    
    if( NULL == ptszFileName )
    {
        Usage();
        return( E_INVALIDARG );
    }

    CNetWrite netWriter;  // Helper object that broadcasts the file.
     
    WCHAR *pwszFile = NULL;
    WCHAR *pwszServerURL = NULL;

    // Declare a dummy 'do' loop. On failure, we can break from the loop.
    do
    {
        // Convert the file name to a wide-character string.
        hr = ConvertTCharToWChar( ptszFileName, &pwszFile );
        if( FAILED( hr ) )
            break;

        
        // The server URL is optional, so it might be NULL.
        if( ptszServerURL != NULL )
        {
            // Convert the server URL to wide-character string.
            hr = ConvertTCharToWChar( ptszServerURL, &pwszServerURL);
            if( FAILED( hr ) )
                break;
        }

        // Initialize our helper object.
        hr = netWriter.Init();
        if( FAILED( hr ) )
        {
            break;
        }

        // Configure the helper object with the port number, file name, maximum
        // number of clients, and server URL.
        hr = netWriter.Configure(dwPortNum, pwszFile, nMaxClient, pwszServerURL);
        if( FAILED( hr ) )
        {
            break;
        }

        // Write all of the samples to the network.
        hr = netWriter.WritetoNet();
        if(FAILED(hr))
        {
            break;
        }

    }
    while(FALSE);  // Go through the dummy loop one time only.
    
    // Free memory. 
    SAFE_ARRAYDELETE( pwszFile );
    SAFE_ARRAYDELETE( pwszServerURL );
    
    return hr;
}
