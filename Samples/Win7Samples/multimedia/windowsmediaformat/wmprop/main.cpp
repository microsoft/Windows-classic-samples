//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            main.cpp
//
// Abstract:            This sample shows properties on a Windows Media file: 
//						Number of streams, codec, protected, bitrate, stream number,
//						seekable, and stridable.
//
//*****************************************************************************

#include "wmprop.h"


////////////////////////////////////////////////////////////////////////////////
//  Show usage of this program.
////////////////////////////////////////////////////////////////////////////////
void usage( void )
{
	_tprintf( _T( "WMProp <infile>\n" ) );
	_tprintf( _T( "infile = Input Windows Media file for which properties are sought\n" ) );
}

////////////////////////////////////////////////////////////////////////////////
//  Program main entry.
////////////////////////////////////////////////////////////////////////////////
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
	if( argc != 2 )
    {
		usage();
		return( -1 );
	}

	HRESULT hr = S_OK;
	hr = CoInitialize( NULL );
	if ( FAILED( hr ) )
	{
		_tprintf( _T( "CoInitialize failed: (hr=0x%08x)\n" ), hr ) ;
        return( hr );
    }
	
	CWMProp wmProp( &hr );
	if ( FAILED( hr ) )
	{
		_tprintf( _T( "Internal error %lu\n" ), hr );
		return( hr );
	}
		
#ifndef UNICODE		//
					//	Unicode implementation
					//

        WCHAR wszFileName[ MAX_PATH ];
	
		//
        //	Check to ensure a buffer over run does not occur
		//
	
		if ( _tcslen( argv[1] ) < MAX_PATH )
		{
			if( 0 == MultiByteToWideChar( CP_ACP, 0, argv[1], _tcslen( argv[1] ) + 1, wszFileName, MAX_PATH ) )
			{
				_tprintf( _T( "Internal error %lu\n" ), GetLastError() );
				return( E_UNEXPECTED );
			}
			hr = wmProp.Open( wszFileName );
		}
		else
		{
			_tprintf( _T( "File name argument is too long.\n" ) );
			return( E_INVALIDARG );
		}

#else	//
		//	Non-Unicode implementation
		//
	
		if ( _tcslen( argv[1] ) < MAX_PATH )
		{
			hr = wmProp.Open( argv[1] );
		}
		else
		{
			_tprintf( _T( "File name argument is too long.\n" ) );
			return( E_INVALIDARG );
		}


#endif // UNICODE
		
	CoUninitialize();
	return( hr );
}
