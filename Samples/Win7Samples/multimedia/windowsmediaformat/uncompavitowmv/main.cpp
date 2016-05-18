//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            main.cpp
//
// Abstract:            Merge samples for audio and video from several AVI files
//                      and either merge these into similar streams or create a 
//                      new stream based on the source stream profile. Profiles 
//                      have to be prx-based. The sample also shows how to create an 
//                      arbitraty stream and do multipass encoding.
//
//*****************************************************************************

#include <tchar.h>
#include <stdio.h>
#include "UncompAVIToWMV.h"

//------------------------------------------------------------------------------
// Name: Usage()
// Desc: Displays command-line usage.
//------------------------------------------------------------------------------
void Usage()
{
    _tprintf( _T( "Usage: UncompAVIToWMV -i <INPUT_FILE> -o <OUTPUT_FILE> [-if]\n" ) ) ;
    _tprintf( _T( "                      [ -ps <SYSTEM_PROFILE_INDEX> | -pf <CUSTOM_PROFILE_FILE> ]\n" ) ) ;
    _tprintf( _T( "                      [-pe] [-sp <FILE>] [-arb] [-m] [-drm] [-smpte] [-d <TIME>]\n" ) ) ;
    _tprintf( _T( "                      [-attrib <NUMBER> <NAME> <TYPE> <VALUE>] [-pause]\n" ) ) ;
    _tprintf( _T( "  -i <INPUT_FILE> = Uncompressed input AVI/WAV file name or if -if specified\n" ) );
    _tprintf( _T( "                    ASCII file containing a list of uncompressed AVI/WAV files\n" ) );
    _tprintf( _T( "  -o <OUTPUT_FILE> = Output file name\n" ) );
    _tprintf( _T( "  -if = load list of AVI/WAV files from file specified by -i <INPUT_FILE>\n" ) );
    _tprintf( _T( "  -ps <SYSTEM_PROFILE_INDEX> = load the system profile by index\n" ) );
    _tprintf( _T( "  -pf <CUSTOM_PROFILE_FILE> = load custom profile by file name\n" ) );
    _tprintf( _T( "  -pe = expand specified profile to include all input streams\n" ) );
    _tprintf( _T( "  -sp <FILE> = saves the current profile to the file\n" ) );
    _tprintf( _T( "  -arb = include arbitrary stream containing sample number\n" ) );
    _tprintf( _T( "  -m = use multipass encoding\n" ) );
    _tprintf( _T( "  -drm = enable digital rights management\n" ) );
    _tprintf( _T( "  -smpte = add SMPTE time code to video stream\n" ) );
    _tprintf( _T( "  -d <TIME> = specify the maximum duration in seconds of the output file\n" ) );
    _tprintf( _T( "  -attrib <NUMBER> <NAME> <TYPE> <VALUE> = add attribute to the specified stream\n" ) );
    _tprintf( _T( "      NUMBER = stream number for attribute: 0 - 63, 0 - all streams\n" ) );
    _tprintf( _T( "      NAME = name of the attribute\n" ) );
    _tprintf( _T( "      TYPE = data type of the attribute: string/qword/word/dword/binary/bool\n" ) );
    _tprintf( _T( "      VALUE = value of the attribute\n" ) );
    _tprintf( _T( "  -pause = wait for keypress when done\n" ) );
    _tprintf( _T( "\n" ) );
    _tprintf( _T( "  REMARK: only AVI files with uncompressed streams are accepted\n\n" ) );

	HRESULT hr = CUncompAVIToWMV::ListSystemProfile();
	if( FAILED( hr ) )
	{
		_tprintf( _T( "Could not list system profile (hr=0x%08x).\n" ), hr );
	}
}

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: Entry point of the application.
//------------------------------------------------------------------------------
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    HRESULT             hr = S_OK;

    TCHAR*              ptszOutFile = NULL;
    TCHAR*              ptszInFile = NULL;
    TCHAR*              ptszOutProfile = NULL;
	TCHAR*				ptszCustomProfile = NULL;
    BOOLEAN             fUseDRM = FALSE;
    CONTENT_DESC        rgCntDesc[20];
    int                 cCntDesc = 0;
    int                 i = 0;
	DWORD				dwSystemProfile = 0xFFFFFFFF;
    IWMProfile*         pIWMProfile = NULL;
    BOOL                fPause = FALSE;
    BOOL                fArbitrary = FALSE;
    BOOL                fInFileListFile = FALSE;
    BOOL                fPreserveProfile = TRUE;
    BOOL                fAddSMPTE = FALSE;
    BOOL                fValidArgument = FALSE;
    int                 nMaxDuration = -1;  // Max duration in seconds of output file 
    CUncompAVIToWMV     converter;

    hr = CoInitialize( NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "CoInitialize failed: (hr=0x%08x)\n" ), hr ) ;
        return( hr );
    }

    for( i = 1; i < argc; i ++ )
    {
        fValidArgument = FALSE;

        if( 0 == _tcsicmp( argv[i], _T( "-attrib" ) ) )
        {
            if( cCntDesc >= 20 )
            {
                _tprintf( _T( "Too many attributes. Max=20\n" ) ) ;
                break;
            }

            i += 4;
            if( i >= argc )
            {
                break;
            }

            rgCntDesc[cCntDesc].wStreamNum = (WORD)_ttoi( argv[i-3] );
            rgCntDesc[cCntDesc].ptszName = argv[i-2];
            rgCntDesc[cCntDesc].ptszType = argv[i-1];
            rgCntDesc[cCntDesc].ptszValue = argv[i];

            if ( rgCntDesc[cCntDesc].wStreamNum > 63 )
            {
                break;
            }

            cCntDesc++;
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
        {
            i++;
            if( i >= argc )
            {
                break;
            }

            ptszInFile = argv[i];
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-if" ) ) )
        {
            fInFileListFile = TRUE;
        }        
        else if( 0 == _tcsicmp( argv[i], _T( "-o" ) ) )
        {
            i++;
            if( i >= argc )
            {
                break;
            }

            ptszOutFile = argv[i];
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-ps" ) ) )
        {
            i++;
            if( i >= argc )
            {
                break;
            }

            if ( !_stscanf_s( argv[i], _T( "%d" ), &dwSystemProfile ) )
                break;
        }
		else if( 0 == _tcsicmp( argv[i], _T( "-pf" ) ) )
		{
			i++;
			if( i >= argc )
			{
				break;
			}

			ptszCustomProfile = argv[i];
		}
        else if( 0 == _tcsicmp( argv[i], _T( "-pe" ) ) )
        {
            fPreserveProfile = FALSE;
        }            
        else if( 0 == _tcsicmp( argv[i], _T( "-sp" ) ) )
        {
            i++;
            if( i >= argc )
            {
                break;
            }

            ptszOutProfile = argv[i];
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-m" ) ) )
        {
            converter.SetPreprocessing( TRUE );
        }        
        else if( 0 == _tcsicmp( argv[i], _T( "-drm" ) ) )
        {
            fUseDRM = TRUE;
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-arb" ) ) )
        {
            fArbitrary = TRUE;
        }            
        else if( 0 == _tcsicmp( argv[i], _T( "-pause" ) ) )
        {
            fPause = TRUE;
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-smpte" ) ) )
        {
            fAddSMPTE = TRUE;
        }
        else if( 0 == _tcsicmp( argv[i], _T( "-d" ) ) )
        {
            i++;
            if( i >= argc )
            {
                break;
            }

            nMaxDuration = _ttoi( argv[i] );
            if ( nMaxDuration <= 0 )
            {
                break;
            }
        }
        else
            break;

        fValidArgument = TRUE;            
    }

    if ( !fValidArgument || i < argc )
    {
        _tprintf( _T( "Invalid argument.\n" ) );
        Usage();
        CoUninitialize();
        return( E_INVALIDARG );
    }

    if( NULL == ptszOutFile || NULL == ptszInFile )
    {
        _tprintf( _T( "An input file and an output file must be specified.\n" ) );
        Usage();
        CoUninitialize();
        return( E_INVALIDARG );
    }

    do
    {
        if( 0xFFFFFFFF != dwSystemProfile )
        {
            if( NULL != ptszCustomProfile )
            {
                //
                // System profile and custom profile cannot be used together.
                //
                _tprintf( _T( "Systemp profile and custom profile can not be used together.\n" ) );
                hr = E_INVALIDARG;
                break;
            }
            else
            {
                //
                // Load system profile
                //
                hr = CUncompAVIToWMV::LoadSystemProfile( dwSystemProfile, 
                                                         &pIWMProfile );
                if( FAILED( hr ) )
                {
                    _tprintf( _T( "Could not load system profile index %d (hr=0x%08x)\n" ), 
                              dwSystemProfile, hr );
                    break;
                }
            }
        }
        else if( NULL != ptszCustomProfile )
        {
            //
            // Load custom profile 
            // 
            hr = CUncompAVIToWMV::LoadCustomProfile( ptszCustomProfile, 
                                                     &pIWMProfile );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not load custom profile %s (hr=0x%08x)\n" ), 
                          ptszCustomProfile, hr );
                break;
            }
        }
        else
        {
		    //
		    // Create an empty profile
		    //
		    hr = CUncompAVIToWMV::CreateEmptyProfile( &pIWMProfile );
		    if( FAILED( hr ) )
		    {
    		    _tprintf( _T( "Could not create empty profile (hr=0x%08x)\n" ), hr );
                break;
		    }

            _tprintf( _T( "-pe option is automatically used when no profile is specified.\n" ) );
            fPreserveProfile = FALSE;
        }

        hr = converter.Initial( ptszInFile, 
                              fInFileListFile, 
                              ptszOutFile, 
                              pIWMProfile, 
                              fArbitrary, 
                              fPreserveProfile,
                              fAddSMPTE,
                              nMaxDuration );
        if( FAILED( hr ) )
        {
            break;
        }

        for( i = 0; i < cCntDesc; i++ )
        {
            hr = converter.SetAttribute( &rgCntDesc[i] ); 
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not set attribute (hr=0x%08x)\n" ), hr );
                break;
            }
        }

        if( FAILED( hr ) )
        {
            break;
        }
    
        if( fUseDRM )
        {
            hr = converter.SetDRM();
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not set DRM (hr=0x%08x)\n" ), hr );
                break;
            }
        }

        if( NULL != ptszOutProfile )
        {
            //
            // Save created profile to file
            //
            hr = converter.SaveProfile( ptszOutProfile, pIWMProfile );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not save current profile to file %s (hr=0x%08x)\n" ), ptszOutProfile, hr );
                break;
            }
        }

        hr = converter.Start( );
        if( FAILED( hr ) )
        {
        }
    }
    while( FALSE );

    SAFE_RELEASE( pIWMProfile );

    if( fPause )
    {
        //
        // Pause until the user presses a key
        //
        _ftprintf( stdout, _T( "\nFinished.  Press enter to continue.\n" ) );
        _gettchar();
    }

    CoUninitialize();
    return( hr );
}

