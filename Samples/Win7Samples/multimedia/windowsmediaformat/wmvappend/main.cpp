//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Main.cpp
//
// Abstract:            Entry point to sample, parsing of command line, and 
//						calling into the CAppend class to append files
//
//*****************************************************************************

#include "stdafx.h"
#include "Append.h"

void usage()
{
   _tprintf( _T( "wmvappend -o <outfile> -i1 <firstinfile> -i2 <secondinfile> [-a <attributeindex>]\n" ) ) ;
   _tprintf( _T( "\t outfile\t= Output WMV file name\n" ) ) ;
   _tprintf( _T( "\t firstinfile\t= First Input WMV file name\n" ) ) ;
   _tprintf( _T( "\t secondinfile\t= Second Input WMV file name\n" ) ) ;
   _tprintf( _T( "\t attributeindex\t= (1 or 2)Input file index for applying attributes to outfile\n" ) ) ;
}

#ifndef UNICODE

//------------------------------------------------------------------------------
// Name: ConvertStrToUnicode()
// Desc: Converts a TCHAR string to wide characters.
//------------------------------------------------------------------------------
HRESULT ConvertStrToUnicode( LPCTSTR ptszInString, __out LPWSTR* pwszOutStr)
{
	if( NULL == ptszInString || NULL == pwszOutStr  )
	{
		return( E_INVALIDARG ) ;
	}

    HRESULT hr = S_OK;

	do
	{

		int nSizeCount = 0 ;
		//
		// Get the memory required for this string
		//
		nSizeCount = MultiByteToWideChar( CP_ACP, 0, ptszInString, -1, NULL, 0 ) ;
		if( 0 ==  nSizeCount )
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );
			break ;
		}

		(*pwszOutStr) = new WCHAR[nSizeCount] ;
		if( NULL == (*pwszOutStr) )
		{
			hr = E_OUTOFMEMORY;
			break ;
		}

		if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInString, -1, (*pwszOutStr), nSizeCount ) )
		{
			hr = HRESULT_FROM_WIN32( GetLastError() );			
		}

	}while( FALSE );

	return( hr );
}

#endif // UNICODE

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: Entry point for the application.
//------------------------------------------------------------------------------
int __cdecl _tmain( int argc, __in_ecount(argc) LPTSTR argv[] )
{
    TCHAR*	ptszFirstInFile  = NULL ;
    TCHAR*	ptszSecondInFile = NULL ;
    TCHAR*	ptszOutFile      = NULL ;

    UINT        iAttr         = 1 ;
    HRESULT     hr            = S_OK ;
    BOOL        bEqualProfile = FALSE ;

    for( int i = 1; i < argc; i ++ )
    {
        if( 0 == _tcsicmp( argv[i], _T( "-i1" ) ) )
        {
            i++;
            if( i == argc )
            {
                usage();
                return( E_INVALIDARG );
            }
            ptszFirstInFile = argv[i];
            continue;
        }
        
        if( 0 == _tcsicmp( argv[i], _T( "-i2" ) ) )
        {
            i++;
            if( i == argc )
            {
                usage();
                return( E_INVALIDARG );
            }
            ptszSecondInFile = argv[i];
            continue;
        }

        if ( 0 == _tcsicmp( argv[i], _T( "-o" ) ) )
        {
            i++;
            if ( i == argc )
            {
                usage();
                return( E_INVALIDARG );
            }
            ptszOutFile = argv[i];
            continue;
        }
        
        if ( 0 == _tcsicmp( argv[i], _T( "-a" ) ) )
        {
            i++;
            if ( i == argc )
            {
                usage();
                return( E_INVALIDARG );
            }
            int retval = _stscanf_s( argv[i], _T( "%d" ), &iAttr ) ;
            if( EOF == retval )
            {
                usage();
                return( E_INVALIDARG );
            }
            continue ;
        }

        usage();
        return( E_INVALIDARG );
    }

    if ( NULL == ptszFirstInFile ||  NULL == ptszSecondInFile  || NULL == ptszOutFile  )
	{
		usage();
		return( E_INVALIDARG );
	}

    if( iAttr != 1 && iAttr != 2 )
    {
        _tprintf( _T( "Invalid Attribute Index\n\t Attribute Index can be either 1 or 2\n" ) ) ;
		return( E_INVALIDARG );
    }

	//
	// If attributes are being copied to the destination file from the second input file, swap file
	// names to have them mapped properly 
	// 
	if( iAttr == 2 )
	{
        TCHAR *ptszTemp  = ptszFirstInFile ;
        ptszFirstInFile  = ptszSecondInFile ;
        ptszSecondInFile = ptszTemp ;
	}
    
	if ( 0 == _tcsicmp( ptszFirstInFile, ptszOutFile ) )
	{
		_tprintf( _T( "First file name and destination file name are same. This is not allowed.\n" ) );
		return( E_INVALIDARG );
	}
	else if ( 0 == _tcsicmp( ptszSecondInFile, ptszOutFile ) )
	{
		_tprintf( _T( "Second file name and destination file name are same. This is not allowed.\n" ) );
		return( E_INVALIDARG );
	}
	
    //
    // Start the work
    //    
    CAppend asfAppender ;

    do
    {
        hr = asfAppender.Init() ;
        if( FAILED( hr ) )
        {
            break ;
        }

#ifndef UNICODE

        WCHAR*  pwszFirstInFile  = NULL ;
        WCHAR*  pwszSecondInFile = NULL ;
        WCHAR*  pwszOutFile      = NULL ;
    
        hr = ConvertStrToUnicode( ptszFirstInFile, &pwszFirstInFile ) ;
		if( FAILED( hr ) )
        {
			 _tprintf( _T( "Failed to convert file name to wchar string (hr=0x%08x)\n" ), hr );
            break ;
        }
		
		hr = ConvertStrToUnicode( ptszSecondInFile, &pwszSecondInFile ) ;
		if( FAILED( hr ) )
        {
			 _tprintf( _T( "Failed to convert file name to wchar string (hr=0x%08x)\n" ), hr );
            break ;
        }

		hr = ConvertStrToUnicode( ptszOutFile, &pwszOutFile ) ;
		if( FAILED( hr ) )
        {
			_tprintf( _T( "Failed to convert file name to wchar string (hr=0x%08x)\n" ), hr );
            break ;
        }
    
        hr = asfAppender.CompareProfiles( pwszFirstInFile, pwszSecondInFile, &bEqualProfile ) ;
        if( FAILED( hr ) )
        {
			break ;
        }

        if( !bEqualProfile )
        {
            _tprintf( _T( "Profile Mismatch\nCan not continue... Aborting the process\n" ) );
            hr = E_FAIL ;
            break ;
        }

	    hr = asfAppender.Configure( pwszOutFile ) ;
	    if( FAILED( hr ) )
        {
            break ;
        }

        SAFE_ARRAYDELETE( pwszFirstInFile ) ;
        SAFE_ARRAYDELETE( pwszSecondInFile ) ;
        SAFE_ARRAYDELETE( pwszOutFile );

#else 
    
        hr = asfAppender.CompareProfiles( ptszFirstInFile, ptszSecondInFile, &bEqualProfile ) ;
        if( FAILED( hr ) )
        {
            break ;
        }

        if( !bEqualProfile )
        {
            _tprintf( _T( "Profile Mismatch\nCan not continue... Aborting the process\n" ) );
            hr = E_FAIL ;
            break ;
        }

	    hr = asfAppender.Configure( ptszOutFile ) ;
	    if( FAILED( hr ) )
        {
            break ;
        }

#endif // UNICODE

        hr = asfAppender.StartAppending() ;
        if( FAILED( hr ) )
        {
            break ;
        }    

		hr = asfAppender.Exit() ;
	}
    while( FALSE ) ;

    return( hr );
}
