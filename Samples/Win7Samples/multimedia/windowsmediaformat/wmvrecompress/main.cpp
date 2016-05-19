//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            main.cpp
//
// Abstract:            Parse the parameters and call CWMVRecompress::Recompress
//
//*****************************************************************************

#include <tchar.h>
#include <stdio.h>
#include "WMVRecompress.h"

///////////////////////////////////////////////////////////////////////////////
// Convert a TCHAR string to WCHAR string. 
// Caller must release the memory of pwszOutput by calling delete[] pwszOutput.
///////////////////////////////////////////////////////////////////////////////
HRESULT ConvertTCharToWChar( LPCTSTR ptszInput, __out LPWSTR * pwszOutput )
{
    int cchOutput = 0;
    
    if( NULL == ptszInput || NULL == pwszOutput )
    {
        return( E_INVALIDARG );
    }

    //
    // Get output buffer size
    //
#ifdef UNICODE
    cchOutput = wcslen( ptszInput ) + 1;
#else //UNICODE
    cchOutput = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 );
    if( 0 == cchOutput )
    {
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }
#endif // UNICODE

    *pwszOutput = new WCHAR[ cchOutput ];
    if( NULL == *pwszOutput)
    {
        return( E_OUTOFMEMORY );
    }

#ifdef UNICODE
    wcsncpy_s( *pwszOutput, cchOutput, ptszInput, cchOutput - 1 );
#else //UNICODE
    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *pwszOutput, cchOutput ) )
    {
        SAFE_ARRAYDELETE( *pwszOutput );
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }        
#endif // UNICODE

    return( S_OK );
}

////////////////////////////////////////////////////////////////////////////////
// This function gives the correct usage, 
// if the user gives invalid arguments.      
////////////////////////////////////////////////////////////////////////////////
void Usage()
{
    _tprintf( _T( "Usage: WMVRecompress -i <INPUT_FILE> -o <OUTPUT_FILE> \n" ) );
    _tprintf( _T( "                     { -ps <SYSTEM_PROFILE_INDEX> | -pf <CUSTOM_PROFILE_FILE> }\n" ) );
    _tprintf( _T( "                     [-m] [-c] [-s]\n" ) );
    _tprintf( _T( "  -i <INPUT_FILE> = input ASF/WMV/WMA file name\n" ) );
    _tprintf( _T( "  -o <OUTPUT_FILE> = output file name\n" ) );
    _tprintf( _T( "  -ps <SYSTEM_PROFILE_INDEX> = load the system profile by index\n" ) );
    _tprintf( _T( "  -pf <CUSTOM_PROFILE_FILE> = load custom profile by file name\n" ) );
    _tprintf( _T( "  -m = use multi-pass encoding if supported by the codec\n" ) );
    _tprintf( _T( "  -c = enable multi-channel output if the audio stream supports (for WinXP or newer only)\n" ) );
    _tprintf( _T( "  -s = enable smart recompression if the source has one or more audio streams\n" ) );
    _tprintf( _T( "\n" ) );

    //
    // List system profiles
    //
    HRESULT hr = CWMVRecompress::ListSystemProfile();
    if( FAILED( hr ) )
    {
        _tprintf( _T( "Could not list system profile (hr=0x%08x).\n" ), hr );
    }
}

////////////////////////////////////////////////////////////////////////////////
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    HRESULT             hr = S_OK;
    WCHAR               * pwszOutFile = NULL;
    WCHAR               * pwszInFile = NULL;
    WCHAR               * pwszCustomProfile = NULL;
    DWORD               dwSystemProfile = 0xFFFFFFFF;
    BOOL                fMultiPass = FALSE;
    BOOL                fMultiChannel = FALSE;
    BOOL                fSmartRecompression = FALSE;
    BOOL                fValidArgument = FALSE;
    IWMProfile          * pIWMProfile = NULL;
    CWMVRecompress      * pWMVRecompress = NULL;

    //
    // Initial COM library
    //
    hr = CoInitialize( NULL );
    if( FAILED( hr ) )
    {
        _tprintf( _T( "CoInitialize failed: (hr=0x%08x)\n" ), hr );
        return( hr );
    }

    do
    {
        //
        // Parse the command line
        //
        int i;
        for( i = 1; i < argc; i ++ )
        {
            fValidArgument = FALSE;

            if( 0 == _tcsicmp( argv[i], _T( "-i" ) ) )
            {
                i++;
                if( i >= argc )
                {
                    break;
                }

                hr = ConvertTCharToWChar( argv[i], &pwszInFile );
                if( FAILED( hr ) )
                {
                    break;
                }
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-o" ) ) )
            {
                i++;
                if( i >= argc )
                {
                    break;
                }

                hr = ConvertTCharToWChar( argv[i], &pwszOutFile );
                if( FAILED( hr ) )
                {
                    break;
                }
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-ps" ) ) )
            {
                i++;
                if( i >= argc )
                {
                    break;
                }

                if( !_stscanf_s( argv[i], _T( "%d" ), &dwSystemProfile ) )
                {
                    break;
                }
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-pf" ) ) )
            {
                i++;
                if( i >= argc )
                {
                    break;
                }

                hr = ConvertTCharToWChar( argv[i], &pwszCustomProfile );
                if( FAILED( hr ) )
                {
                    break;
                }
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-m" ) ) )
            {
                fMultiPass = TRUE;
            }        
            else if( 0 == _tcsicmp( argv[i], _T( "-c" ) ) )
            {
                fMultiChannel = TRUE;
            }
            else if( 0 == _tcsicmp( argv[i], _T( "-s" ) ) )
            {
                fSmartRecompression = TRUE;
            }
            else
            {
                break;
            }

            fValidArgument = TRUE;            
        }

        if( !fValidArgument || i < argc )
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "Invalid argument.\n" ) );
            Usage();
            break;
        }

        //
        // The input file and the output file must not be empty, or equal.
        //
        if( NULL == pwszInFile || 
            NULL == pwszOutFile || 
            0 == _wcsicmp(pwszInFile, pwszOutFile) )
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "Input file and output file must be specified.\n" ) );
            Usage();
            break;
        }

        //
        // MultiChannel output is only supported on Windows XP. We need to check
        // the current OS version to ascertain whether multichannel output is enabled.
        //
        if( fMultiChannel )
        {
            OSVERSIONINFO vi;

            memset( &vi, 0, sizeof( vi ) );
            vi.dwOSVersionInfoSize = sizeof( vi );

            if( !GetVersionEx( &vi ) )
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
                _tprintf( _T( "Could not get the system version.\n" ) );
                break;
            }

            if( vi.dwMajorVersion < 5
                || vi.dwMajorVersion == 5 && vi.dwMinorVersion < 1 )
            {
                hr = E_INVALIDARG;
                _tprintf( _T( "Multi-channel is only supported on Windows XP or newer version.\n" ) );
                Usage();
                break;
            }
        }

        if( 0xFFFFFFFF != dwSystemProfile )
        {
            if( NULL != pwszCustomProfile )
            {
                //
                // System profile and custom profile cannot be used together.
                //
                hr = E_INVALIDARG;
                _tprintf( _T( "System profile and custom profile can not be used together.\n" ) );
                break;
            }
            else
            {
                //
                // Load system profile
                //
                hr = CWMVRecompress::LoadSystemProfile( dwSystemProfile, &pIWMProfile );
                if( FAILED( hr ) )
                {
                    _tprintf( _T( "Could not load system profile (hr=0x%08x)\n" ), hr );
                    break;
                }
            }
        }
        else if( NULL != pwszCustomProfile )
        {
            //
            // Load custom profile 
            // 
            hr = CWMVRecompress::LoadCustomProfile( pwszCustomProfile, &pIWMProfile );
            if( FAILED( hr ) )
            {
                _tprintf( _T( "Could not load custom profile %s (hr=0x%08x)\n" ), pwszCustomProfile, hr );
                break;
            }
        }
        else
        {
            hr = E_INVALIDARG;
            _tprintf( _T( "Systemp profile or custom profile must be specified.\n" ) );
            break;
        }

        pWMVRecompress = new CWMVRecompress();
        if( NULL == pWMVRecompress )
        {
            hr = E_OUTOFMEMORY;
            _tprintf( _T( "Could not allocate memory for CWMVRecompress: (hr=0x%08x)\n" ), hr );
            break;
        }

        //
        // Recompress the WMV file using the specified profile.
        //
        hr = pWMVRecompress->Recompress( pwszInFile, 
                                         pwszOutFile, 
                                         pIWMProfile, 
                                         fMultiPass, 
                                         fMultiChannel,
                                         fSmartRecompression );
        if( FAILED( hr ) )
        {
            break;
        }

    }
    while ( FALSE );

    //
    // Release all resources
    //
    SAFE_ARRAYDELETE( pwszCustomProfile );
    SAFE_ARRAYDELETE( pwszOutFile );
    SAFE_ARRAYDELETE( pwszInFile );
    SAFE_RELEASE( pWMVRecompress );
    SAFE_RELEASE( pIWMProfile );

    CoUninitialize();

    return( hr );
}
