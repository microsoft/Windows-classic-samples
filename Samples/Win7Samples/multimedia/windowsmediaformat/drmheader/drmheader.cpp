//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DRMHeader.cpp
//
// Abstract:            This sample demonstrates the usage of IWMDRMEditor
//						interface for retrieving various DRM properties.
//
//*****************************************************************************

#include "stdafx.h"
#include "DRMHeaderQuery.h"


//
// DRM properties to be queried
//
const WCHAR *g_DRMProperties[] = {

	g_wszWMDRM_IsDRM                             ,  //= L\"IsDRM\"
	g_wszWMDRM_IsDRMCached                       ,  //= L\"IsDRMCached\"
	g_wszWMDRM_BaseLicenseAcqURL                 ,  //= L\"BaseLAURL\"
	g_wszWMDRM_Rights                            ,  //= L\"Rights\"
	g_wszWMDRM_LicenseID                         ,  //= L\"LID\"

	g_wszWMDRM_ActionAllowed_Playback            ,  //= L\"ActionAllowed.Play\"
	g_wszWMDRM_ActionAllowed_CopyToCD            ,  //= L\"ActionAllowed.Print.redbook\"
	g_wszWMDRM_ActionAllowed_CopyToSDMIDevice    ,  //= L\"ActionAllowed.Transfer.SDMI\"
	g_wszWMDRM_ActionAllowed_CopyToNonSDMIDevice ,  //= L\"ActionAllowed.Transfer.NONSDMI\"
	g_wszWMDRM_ActionAllowed_Backup              ,  //= L\"ActionAllowed.Backup\"

	g_wszWMDRM_DRMHeader                         ,  //= L\"DRMHeader.\"
	g_wszWMDRM_DRMHeader_KeyID                   ,  //= L\"DRMHeader.KID\"
	g_wszWMDRM_DRMHeader_LicenseAcqURL           ,  //= L\"DRMHeader.LAINFO\"
	g_wszWMDRM_DRMHeader_ContentID               ,  //= L\"DRMHeader.CID\"
	g_wszWMDRM_DRMHeader_IndividualizedVersion   ,  //= L\"DRMHeader.SECURITYVERSION\"
	g_wszWMDRM_DRMHeader_ContentDistributor      ,  //= L\"DRMHeader.ContentDistributor\"
	g_wszWMDRM_DRMHeader_SubscriptionContentID   ,  //= L\"DRMHeader.SubscriptionContentID\"
	NULL
};

#ifndef UNICODE

//------------------------------------------------------------------------------
// Name: ConvertTCHARToWCHAR
// Desc: Shows the correct way to convert a TCHAR to a WCHAR
//------------------------------------------------------------------------------
HRESULT ConvertTCHARToWCHAR( TCHAR* ptszInput, WCHAR** pwszOutput )
{
    HRESULT hr = S_OK;
    int nSizeCount = 0;
    
    if ( NULL == ptszInput || NULL == pwszOutput)
    {
        return( E_INVALIDARG );
    }

    //
    // Make wide char string of the file name
    //
    nSizeCount = MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, NULL, 0 ) ;
    if( 0 ==  nSizeCount )
    {
        _tprintf( _T( "Internal error %lu\n" ), GetLastError() );
        return ( E_UNEXPECTED );
    }

    *pwszOutput= new WCHAR[ nSizeCount ];
    if( NULL == *pwszOutput )
    {
        _tprintf( _T( "Internal Error %lu\n" ), GetLastError() ) ;
        SAFE_ARRAYDELETE( *pwszOutput );
        return ( E_UNEXPECTED );
    }

    if( 0 == MultiByteToWideChar( CP_ACP, 0, ptszInput, -1, *pwszOutput, nSizeCount ) )
    {
        _tprintf( _T( "Internal error %lu\n" ), GetLastError() );
        SAFE_ARRAYDELETE( *pwszOutput );
        return ( E_UNEXPECTED );
    }        

    return( hr );
}

#endif

void Usage()
{
	_tprintf( TEXT("Usage:\n\tDRMHeader.exe <FileName> [ PropertyName ]\n\n") );
	_tprintf( TEXT("Queries the requested Property in the file.\n") );
	_tprintf( TEXT("If no property name is specified, all existing properties will be displayed.\n\n") );

}

//------------------------------------------------------------------------------
// Name: PrintProperties()
// Desc: Displays all DRM properties 
//------------------------------------------------------------------------------

void PrintProperties( WCHAR *pwszFileName, WCHAR *pwszPropertyName )
{
	HRESULT hr;
	CDRMHeaderQuery drmHQ;

	hr = drmHQ.Open( pwszFileName );
	if( FAILED( hr ) )
	{
		_tprintf( TEXT("Failed to open file, HR = 0x%X\n"),hr );
		return;
	}

	if( NULL != pwszPropertyName)
	{
		hr = drmHQ.PrintProperty( pwszPropertyName );
		if( FAILED( hr ) )
		{
			_tprintf( TEXT("Failed to query for the given property. HR = 0x%X\n"), hr );
		}
	}
	else
	{
		int i=0;
		while( NULL != g_DRMProperties[i] )
		{
			drmHQ.PrintProperty( g_DRMProperties[i] );
			i++;
		}
	}
}

//------------------------------------------------------------------------------
// Name: _tmain()
// Desc: The entry point for this console application 
//------------------------------------------------------------------------------
int  __cdecl _tmain( int argc, TCHAR* argv[] )
{
    HRESULT         hr = S_OK;

	if( argc < 2 )
	{
		Usage();
		return -1;
	}

    hr = CoInitialize( NULL );
	if( FAILED( hr ) )
	{
		_tprintf( _T( "CoInitialize failed" ) ) ;
		return( 1 );
	}

	WCHAR *pwszFileName		= NULL;
	WCHAR *pwszPropertyName	= NULL;

#ifndef UNICODE
	ConvertTCHARToWCHAR( argv[1], &pwszFileName );
	if( argc>2 )
		ConvertTCHARToWCHAR( argv[2], &pwszPropertyName );
#else
	pwszFileName = argv[1];
	if( argc>2 )
		pwszPropertyName = argv[2];
#endif


	PrintProperties( pwszFileName, pwszPropertyName );
	

#ifndef UNICODE
	SAFE_ARRAYDELETE( pwszFileName );
	SAFE_ARRAYDELETE( pwszPropertyName );
#endif

	CoUninitialize();

	return 0; 
}