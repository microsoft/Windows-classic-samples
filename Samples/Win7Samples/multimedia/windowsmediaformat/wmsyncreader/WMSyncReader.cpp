//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            WMSyncReader.cpp
//
// Abstract:            Entry point for the sample program. Parses command line
//						parameters and passes them to the CReader instance.
//
//*****************************************************************************

#include "stdafx.h"
#include "reader.h"

static void Usage();
static HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR *    ptszInput,
                          DWORD *cnsStart, DWORD *cnsEnd,
                          BOOL *fCompressed, BOOL *fAudioPresent,
                          BOOL *fVideoPresent, BOOL *fRangeInFrames);
///////////////////////////////////////////////////////////////
///////
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    TCHAR*    ptszInput         = NULL ;
    HRESULT   hr                = S_OK ;
    DWORD     cnsStart          = 0;
    DWORD     cnsEnd            = 0;
    BOOL      fCompressed       = FALSE;
    BOOL      fAudioPresent     = FALSE;
    BOOL      fVideoPresent     = FALSE;
    BOOL      fRangeInFrames    = FALSE;

    hr = CommandLineParser( argc, argv, &ptszInput, &cnsStart, &cnsEnd, &fCompressed,
                            &fAudioPresent, &fVideoPresent, &fRangeInFrames );
	if ( !fAudioPresent && !fVideoPresent )
	{
		_tprintf(_T("No streams specified to read. Please specify at least one of audio or video streams.\n\n"));
		hr = E_INVALIDARG;
	}

    if ( FAILED( hr ) )
    {
        Usage();
        return( hr );
    }

    hr = CoInitialize( NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "CoInitialize failed: hr = 0x%08x\n" ), hr );
        return( hr );
    }

    CReader reader;

    do 
    {
        hr = reader.SetParams( cnsStart, cnsEnd, fCompressed, fAudioPresent, fVideoPresent, fRangeInFrames );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Incorrect values of input parameters -st or -end\n" ) );
            break ;
        }

        hr = reader.Open( ptszInput );
        if( FAILED( hr ) )
        {
            _tprintf( _T( "Cannot open reader: hr = 0x%08x\n" ), hr );
            break ;
        }
        hr = reader.ReadSamples();
        if( FAILED( hr ) )
        {
            _tprintf( _T( "ReadSamples failed: hr = 0x%08x\n" ), hr );
            break ;
        }

    }while( FALSE );

    if (FAILED( hr ) )
    {
        _tprintf( _T(  "WMSyncReader failed (hr=0x%08x).\n" ), hr );
    }

    hr = reader.Close();
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Close failed: hr = 0x%08x\n" ), hr );       
    }

    CoUninitialize();

	return( 0 );
}

//////////////////////////////////////////////////////////////////////////////
// This function gives the correct usage                                    //
// if the user gives invalid arguments.                                     //
//////////////////////////////////////////////////////////////////////////////
void Usage()
{
   _tprintf( _T( "\nWMSyncReader -i <infile> [ -st <start> -e <end> -c -a -v -f ]\n" ) );
   _tprintf( _T( "\tinfile\t = Input filename (cannot be a network stream)\n" ) );
   _tprintf( _T( "\tstart\t = Start time in (ms), or starting frame number (if -f is specified)\n" ) );
   _tprintf( _T( "\tend\t = End time in (ms), or ending frame number (if -f is specified), 0 means EOF\n" ) );
   _tprintf( _T( "\t-c\t = Read compressed samples\n" ) );
   _tprintf( _T( "\t-a\t = Read audio stream if present\n" ) );
   _tprintf( _T( "\t-v\t = Read video stream if present\n" ) );
   _tprintf( _T( "\t-f\t = Use frame seeking. Reading range is specified in frames numbers rather than time (see -st and -e options). " ) );
   _tprintf( _T( "The file must be frame indexed in order to use -f option.\n\n" ) );
   _tprintf( _T( "\tREMARK = at least one of -a or -v should be set to get any samples\n" ) );

}

///////////////////////////////////////////////////////////////
////////
HRESULT CommandLineParser( int argc, __in_ecount(argc) LPTSTR argv[], __out LPTSTR *    ptszInput,
                          DWORD *cnsStart, DWORD *cnsEnd,
                          BOOL *fCompressed, BOOL *fAudioPresent,
                          BOOL *fVideoPresent, BOOL *fRangeInFrames)
{

    if ( argc < 2 || NULL == argv || NULL == ptszInput
        || NULL == cnsStart || NULL == cnsEnd
        || NULL == fCompressed || NULL == fAudioPresent
        || NULL == fVideoPresent || NULL == fRangeInFrames )
    {
        return( E_INVALIDARG );
    }


    HRESULT hr = S_OK;

    *fCompressed = FALSE;
    *fAudioPresent = FALSE;
    *fVideoPresent = FALSE;
    *fRangeInFrames = FALSE;

    for( int i = 1; i < argc; i ++ )
    {
        if ( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
        {
            i++ ;

            if ( i == argc )
            {
                Usage();
                return( E_INVALIDARG );
            }
            
            *ptszInput = argv[i];
            continue ;

        }

        if ( 0 == _tcsicmp( argv[i], _T( "-st" ) ) )
        {
            i++;

            if ( i == argc )
            {
                *cnsStart = 0;
            }
			else
			{

				int ret = _stscanf_s( argv[i], _T( "%lu" ), cnsStart ) ;
				if (EOF == ret || 0 == ret )
				{
					Usage();
					return( E_INVALIDARG );
				}
			}
            continue;
        }

        if( 0 == _tcsicmp( argv[i] , _T( "-e" ) ) )
        {
            i++ ;

            if(i == argc)
            {
                *cnsEnd = 0;
            }
			else
			{
				int ret = _stscanf_s( argv[i], _T( "%lu" ), cnsEnd ) ;
				if (EOF == ret || 0 == ret )
				{
					Usage();
					return( E_INVALIDARG );
				}
			}

            continue;
        }

        if( 0 == _tcsicmp( argv[i] , _T( "-c" ) ) )
        {
            *fCompressed = TRUE;
            continue;
        }
        if( 0 == _tcsicmp( argv[i] , _T( "-a" ) ) )
        {
            *fAudioPresent = TRUE;
            continue;
        }
        if( 0 == _tcsicmp( argv[i] , _T( "-v" ) ) )
        {
            *fVideoPresent = TRUE;
            continue;
        }
        if( 0 == _tcsicmp( argv[i] , _T( "-f" ) ) )
        {
            *fRangeInFrames = TRUE;
            continue;
        }
    }

    return S_OK;
}
