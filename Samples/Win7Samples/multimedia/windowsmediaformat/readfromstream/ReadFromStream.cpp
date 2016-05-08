//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            ReadFromStream.cpp
//
// Abstract:            Defines the entry point for the console application.
//
//*****************************************************************************

#include "stdafx.h"
#include "Reader.h"

HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR* ptszInput );
void Usage();

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{

    CReader*    pReader     = NULL;
    HRESULT     hr          = S_OK;
    TCHAR*      ptszInput   = NULL;

    //
    // Process the command line options
    //
    hr = CommandLineParser( argc, argv, &ptszInput );
    if( FAILED( hr ) )
    {
        return( hr );
    }

    //
    // Initialize COM
    //
    hr = CoInitialize( NULL );
	if( FAILED( hr ) )
	{
        printf( "CoInitialize() failed (hr=%#X).", hr );
		return( -1 );
	}

    do 
    {
        //
        // Use CReader to read from the input file
        //
        pReader = new CReader;
        if( NULL == pReader )
        {
            printf( "Could not create CReader object.\n" );
            return( -1 );
        }

        hr = pReader->Open( ptszInput );
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pReader->Start();
        if( FAILED( hr ) )
        {
            break;
        }

        hr = pReader->Close();
        if( FAILED( hr ) )
        {
            break;
        }
    }
    while( FALSE );

    SAFE_RELEASE( pReader );

	CoUninitialize();
    
    return( 0 );
}

//------------------------------------------------------------------------------
// Name: Usage()
// Desc: Displays the command-line usage.
//------------------------------------------------------------------------------
void Usage()
{
    printf( "ReadFromStream -i <InFile>\n" );
    printf( "\tInFile:\tThe input Windows Media Format file (WMA/WMV/ASF) name\n\n" );
    return;
}

//------------------------------------------------------------------------------
// Name: CommandLineParser()
// Desc: Validates the arguments and gets the name of the input file.
//------------------------------------------------------------------------------
HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR* ptszInput )
{
    if( ( argc < 1 )        ||
        ( NULL == argv )    ||
        ( NULL == ptszInput ) )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr          = S_OK;
    TCHAR*  ptszMode    = NULL;

    *ptszInput = NULL;    

    for( int i = 1; i < argc; i++ )
    {
        if( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
        {
            i++;

            if( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }

            *ptszInput = argv[i];
            continue;
        }
    }

    if ( NULL == *ptszInput )
    {
        Usage();
        return( E_INVALIDARG );
    }

    return( S_OK );
}

