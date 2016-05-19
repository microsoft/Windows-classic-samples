//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Drm.cpp
//
// Abstract:            Implementation of class CDRM which handles DRM license
//                      acquisition for protected content.
//
//*****************************************************************************

//
#include "stdafx.h"
#include "nserror.h"
#include "AudioPlay.h"
#include "AudioDlg.h"
#include <stdio.h>
#include <shellapi.h>

#ifdef SUPPORT_DRM
#include "DRM.h"
#pragma warning( disable:4192 )
#import "shdocvw.dll"

class __declspec(uuid("0002DF01-0000-0000-C000-000000000046")) InternetExplorer;
struct __declspec(uuid("D30C1661-CDAF-11D0-8A3E-00C04FC9E26E")) IWebBrowser2;

//
// This function is used automate the internet browser for DRMv7 non-silent license acquisition
//
HRESULT OpenURLWithData( LPWSTR wszURL, BYTE* pbPostData )
{
    static const LPSTR POST_DATA_PREFIX = "nonsilent=1&challenge=";
    static const LPWSTR POST_HEADER_DATA = L"Content-Type: application/x-www-form-urlencoded\r\n";

    HRESULT hr = S_OK;
    SHDocVw::IWebBrowser2* pWebBrowser = NULL;
    BSTR bstrURL = NULL;
    BSTR bstrHeader = NULL;
    VARIANT vtPostData = {0};
    VARIANT vtEmpty;
    VARIANT vtHeader;
    int nPostDataSize;
    SAFEARRAY* saPostData = NULL;
    void* pvData;
    int nPostDataPrefixSize;
    
    if ( !wszURL )
        return E_INVALIDARG;
    
    do
    {

        //
        // Create the web browser instance
        //
        hr = ::CoCreateInstance( __uuidof(InternetExplorer), NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
                               __uuidof(IWebBrowser2), reinterpret_cast<LPVOID *>(&pWebBrowser) );
        if( FAILED(hr) )
		{
			//Can't create instance of IE!
			return hr;
		}
        
        //
        // Convert URL to BSTR
        //
        bstrURL = SysAllocString( wszURL );
        if ( !bstrURL )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Create a variant for the header information
        //
        bstrHeader = SysAllocString( POST_HEADER_DATA );
        if ( !bstrHeader )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        VariantInit( &vtEmpty );
        VariantInit( &vtPostData );
        VariantInit( &vtHeader );

        V_VT( &vtHeader ) = VT_BSTR;
        V_BSTR( &vtHeader ) = bstrHeader;
        
        if ( pbPostData )
        {
            //
            // Convert post data to a variant
            //
            nPostDataSize = strlen( (char*) pbPostData );
            nPostDataPrefixSize = strlen( POST_DATA_PREFIX );
            saPostData = SafeArrayCreateVector( VT_UI1, 0, nPostDataPrefixSize + nPostDataSize + 1 );
            if( NULL == saPostData )
            {
                hr = E_OUTOFMEMORY;
                break;
            }
    
            hr = SafeArrayAccessData( saPostData, &pvData );
			if( FAILED(hr) )
			{
				break;
			}
    
            //
            // Copy the prefix, then the data
            //
            CopyMemory( pvData, POST_DATA_PREFIX, nPostDataPrefixSize );
            CopyMemory( ((LPBYTE)pvData) + nPostDataPrefixSize, pbPostData, nPostDataSize + 1 );
    
            hr = SafeArrayUnaccessData( saPostData );
			if( FAILED(hr) )
			{
				break;
			}
    
            V_VT(&vtPostData) = VT_ARRAY | VT_UI1;
            V_ARRAY(&vtPostData) = saPostData;
        }

        try
        {
            //
            // Show the web browser
            //
            pWebBrowser->put_Visible ( (VARIANT_BOOL) TRUE ); //  Ignore the hr - it's not really important either way
    
            //
            // Navigate to the requested URL
            //
            hr = pWebBrowser->Navigate( bstrURL, &vtEmpty, &vtEmpty, &vtPostData, &vtHeader );
            if( FAILED(hr) )
			{
				break;
			}
        }
        catch ( ... )
        {
			 // An exception was thrown calling the IE control!
            hr = E_FAIL;
        }

    }
    while ( FALSE );
    
    SAFE_RELEASE( pWebBrowser );
	SAFE_SYSFREESTRING( bstrURL );
	SAFE_SYSFREESTRING( bstrHeader );
    if( saPostData )
    {
        SafeArrayDestroy( saPostData );
        saPostData = NULL;
    }
    
    return hr;
}


//------------------------------------------------------------------------------
// Name: CDRM::CDRM()
// Desc: Constructor.
//------------------------------------------------------------------------------
CDRM::CDRM()
{
	m_pParent		    = NULL;
	m_pDRMReader	    = NULL;
	m_pwszURL		    = NULL;
	m_statusDRM		    = NONE;
    m_pWMGetLicenseData = NULL;
    m_dwLastDRMVersion  = 0;
	m_bDoNotConnectToUntrustedURL = FALSE;	// if this is set to TRUE in the way, we will purposedly fail License Acquisition.
}

//------------------------------------------------------------------------------
// Name: CDRM::~CDRM()
// Desc: Destructor.
//------------------------------------------------------------------------------
CDRM::~CDRM()
{
	Exit();
}

//------------------------------------------------------------------------------
// Name: CDRM::Init()
// Desc: Initializes some member variables.
//------------------------------------------------------------------------------
HRESULT CDRM::Init( CAudioPlay* pParent, IWMReader* pReader, LPCWSTR pwszURL )
{
    if( ( NULL == pParent ) ||
        ( NULL == pReader ) ||
        ( NULL == pwszURL ) )
    {
        return( E_INVALIDARG );
    }

	m_bDoNotConnectToUntrustedURL = FALSE;

	m_pParent = pParent;
    m_pParent->AddRef();

    m_pwszURL = ( LPWSTR )pwszURL;

    //
    // Query for IWMDRMReader interface
    //
	HRESULT hr = pReader->QueryInterface( IID_IWMDRMReader, ( void** )&m_pDRMReader );
	if( FAILED( hr ) )
	{
        //Did you remember to link to your stublib and unlink to wmvcore.lib?
		TCHAR tszErrMsg[256];		
        sprintf_s( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Could not QI for IWMDRMReader (hr=%#X)" ), hr );
		MessageBox( g_hwndDialog, tszErrMsg, ERROR_DIALOG_TITLE, MB_OK );
	}

	return( hr );
}


//------------------------------------------------------------------------------
// Name: CDRM::Exit()
// Desc: Cleanup.
//------------------------------------------------------------------------------
HRESULT CDRM::Exit()
{
    SAFE_ARRAYDELETE( m_pWMGetLicenseData );
	SAFE_RELEASE( m_pDRMReader );
    SAFE_RELEASE( m_pParent );
	m_pwszURL		= NULL;
	m_statusDRM		= NONE;

	return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CDRM::Cancel()
// Desc: Cancel DRM operations.
//------------------------------------------------------------------------------
HRESULT CDRM::Cancel()
{
    //
    // If m_pDRMReader is NULL, either Init() has not been called or Exit() has
    // been called, and it indicates no DRM operation is being executed.
    //
	if( NULL == m_pDRMReader )
	{
		return( S_FALSE );
	}

	//
	// Cancel current operation
	//
	HRESULT hr = S_OK;

	switch( m_statusDRM )
	{
	case NONSILENT:
        //
        // We cannot cancel this operation once the browser is brought up.
        //
		break;

	case SILENT:
		hr = m_pDRMReader->CancelLicenseAcquisition();
		if( SUCCEEDED( hr ) )
		{
			m_pParent->SetAsyncEvent( NS_S_DRM_ACQUIRE_CANCELLED );
		}
		break;

	case INDIVIDUALIZE:
		hr = m_pDRMReader->CancelIndividualization();
		if( SUCCEEDED( hr ) )
		{
			m_pParent->SetAsyncEvent( NS_E_DRM_INDIVIDUALIZATION_INCOMPLETE );
		}
		break;

	default:
		break;
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CDRM::OnDRMStatus()
// Desc: Handles DRM notifications in OnStatus.
//------------------------------------------------------------------------------
HRESULT CDRM::OnDRMStatus( /* [in] */ WMT_STATUS Status,
						   /* [in] */ HRESULT hr,
						   /* [in] */ WMT_ATTR_DATATYPE dwType,
						   /* [in] */ BYTE __RPC_FAR *pValue,
						   /* [in] */ void __RPC_FAR *pvContext )
{
	if( NULL == m_pDRMReader )
	{
		return( S_OK );
	}

	HRESULT hRes = S_OK;
    
	DWORD cbWMGetLicenseDataSize;
	DWORD dwContinue;

	switch( Status )
	{
	case WMT_NO_RIGHTS:

        //
        // This is a DRM v1 file that requires non-silent license acquisition. In this sample,
        // a browser window will be brought up and directed to the web site 
        // from which a license can be acquired after filling in required info.
        // The user must click Play in order to play the file after
        // the license is acquired.
        // With v1 files, pValue is a WCHAR string containing the license acquisition URL.

        m_dwLastDRMVersion = 1;
		m_statusDRM = NONSILENT;
		SetCurrentStatus( ACQUIRINGLICENSE );

		hRes = GetNonSilentLicense( pValue );
		if( FAILED( hRes ) )
		{
			m_pParent->SetAsyncEvent( hRes );
		}
        else
        {
		    m_pParent->SetAsyncEvent( NS_E_DRM_NO_RIGHTS );
        }
		
		break;

	case WMT_NO_RIGHTS_EX:
        //
        // This event is fired only for DRM v7 files
        // pValue is a WM_GET_LICENSE_DATA structure
        //

		if( m_bDoNotConnectToUntrustedURL == TRUE )
		{
			//
			// WRT the warning message displayed by WMT_LICENSEURL_SIGNATURE_STATE message
			// the user has previously chosen not to connect to license acquisition URL (silent or non silent)
			//
			SetCurrentStatus( CLOSED );
			SetCurrentStatus( READY );
			return E_FAIL;
		}

        m_dwLastDRMVersion = 7;
		m_statusDRM = SILENT;
		SetCurrentStatus( ACQUIRINGLICENSE );

        //
        // Copy the WM_GET_LICENSE_DATA structure
        //
        cbWMGetLicenseDataSize = 0;
        SAFE_ARRAYDELETE( m_pWMGetLicenseData );

        //
		// First, retrieve the size of the data to be copied
		//
        hRes = CopyWMGetLicenseData( (WM_GET_LICENSE_DATA*) pValue, NULL, &cbWMGetLicenseDataSize );
        if ( FAILED( hRes ) )
        {
            break;
        }
                
        //
		// Allocate the memory for the structure
		//
        m_pWMGetLicenseData = (WM_GET_LICENSE_DATA*) new BYTE[ cbWMGetLicenseDataSize ];
        if ( !m_pWMGetLicenseData )
        {
            hRes = E_OUTOFMEMORY;
            break;
        }
        //
		// Store the license data in case silent acquisition fails and
		// we need to try it non-silently using AcquireLastV7LicenseNonSilently
		//
        hRes = CopyWMGetLicenseData( (WM_GET_LICENSE_DATA*) pValue, m_pWMGetLicenseData, &cbWMGetLicenseDataSize );
        if ( FAILED( hRes ) )
        {
            break;
        }

        //
		// Acquire the license silently
		//
		hRes = m_pDRMReader->AcquireLicense( 1 );
		if( FAILED( hRes ) )
		{
			m_pParent->SetAsyncEvent( hRes );
		}
		
		break;

	case WMT_ACQUIRE_LICENSE:

        //
        // This event is fired only for DRM v7 files after
        // an application has called AcquireLicense (for silent acquisition)
        // or MonitorLicenseAcquisition (for non-silent acquisition)
        // pValue is a WM_GET_LICENSE_DATA structure
        //

		hRes = HandleAcquireLicense( pValue );
		if( FAILED( hRes ) )
		{
			m_pParent->SetAsyncEvent( hRes );
		}
				
		break;

	case WMT_NEEDS_INDIVIDUALIZATION:
		
		m_statusDRM = INDIVIDUALIZE;
		SetCurrentStatus( INDIVIDUALIZING );

        //
		// Individualize the client.
		//
		hRes = m_pDRMReader->Individualize( 0 );
		if( FAILED( hRes ) )
		{
			m_pParent->SetAsyncEvent( hRes );
		}
		break;
	
	case WMT_INDIVIDUALIZE:
		
         //
		// Individualize the client.
		// IMPORTANT!!! This operation will cause the user's system
        // to be modified. In your application you should
		// always let the user choose whether or not to individualize.
		// See the SDK documentation or the Microsoft Web site for more information.
		//
		hRes = HandleIndividualize( pValue );
		if( FAILED( hRes ) )
		{
			m_pParent->SetAsyncEvent( hRes );
		}

		break;

	case WMT_LICENSEURL_SIGNATURE_STATE:
	
			switch((int)*pValue)
			{
				case WMT_DRMLA_UNTRUSTED:
      				dwContinue = MessageBox( g_hwndDialog, _T( "A license is required to play this content, but the authenticity of the license acquisition URL can not be verified .\n\nTo obtain the license, a web connection will have to be made to the content provider's site with this untrusted URL. You might be required to register or pay a fee for the license -- it varies by provider.\n\nCAUTION: Web pages can contain elements that could be harmful to your computer. It is important to be certain that the content is from a trustworthy source before continuing.\n\nDo you want to continue acquiring the license?" ),
            	            					ERROR_DIALOG_TITLE, MB_YESNO );
					break;
				case WMT_DRMLA_TRUSTED:
					//Proceed silently
					break;

				case WMT_DRMLA_TAMPERED:
						dwContinue = MessageBox( g_hwndDialog, _T( "The license acquisition URL in this file has been tampered with. You are strongly advised not to navigate to this Web site. Navigate to this Web site anyway?" ),
                          						ERROR_DIALOG_TITLE, MB_YESNO );
					break;
				default:
					dwContinue = MessageBox( g_hwndDialog, _T( "The license acquisition URL in this file is not signed and its validity cannot be guaranteed. Get a license anyway?" ),
                 						ERROR_DIALOG_TITLE, MB_YESNO );
					break;
			}

			if(dwContinue == IDYES)
				m_bDoNotConnectToUntrustedURL = FALSE; //correct? or just fail silently?
			else
				m_bDoNotConnectToUntrustedURL = TRUE;
		//}
		break;

	default:
		break;
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CDRM::HandleAcquireLicense()
// Desc: Handles WMT_ACQUIRE_LICENSE notifications in OnDRMStatus.
//		 This function is called in two cases: 
//       (1) when silent acquisition succeeds using m_pDRMReader->AcquireLicense(1)
//       (2) when non-silent v7 acquisition succeeds if you call m_pDRMReader->MonitorLicenseAcquisition
//       Note: v1 licenses have no notification mechanism like this
//
//------------------------------------------------------------------------------
HRESULT CDRM::HandleAcquireLicense( BYTE *pValue )
{
	WM_GET_LICENSE_DATA* pLicenseData = ( WM_GET_LICENSE_DATA* )pValue;

	if( NS_S_DRM_LICENSE_ACQUIRED != pLicenseData->hr )
	{
		return ( pLicenseData->hr );
	}
	
	m_statusDRM = NONE;

	//
	// License Acquisition Complete            
	// Reopen the reader
	//
	SetCurrentStatus( LICENSEACQUIRED );

	return( m_pParent->ReopenReader( this ) );
}

//------------------------------------------------------------------------------
// Name: CDRM::HandleIndividualize()
// Desc: Handles WMT_INDIVIDUALIZE notifications in OnDRMStatus.
//       
// Individualize the client.
// IMPORTANT!!! This operation will cause the user's system
// to be modified. In your application you should
// always let the user choose whether or not to individualize.
// See the SDK documentation or the Microsoft Web site for more information.
//
//------------------------------------------------------------------------------
HRESULT CDRM::HandleIndividualize( BYTE *pValue )
{
	WM_INDIVIDUALIZE_STATUS* pStatus = ( WM_INDIVIDUALIZE_STATUS* )pValue;

    // The WMT_INDIVIDUALIZE event will be sent to your application
    // repeatedly while the new component is downloading.
    // You can add code here to use the information in pStatus to
    // implement a progress bar or some other user interface that 
    // informs the user of the status of the download operation.
    // 
	if( INDI_SUCCEED != pStatus->enIndiStatus )
	{
		return( pStatus->hr );
	}
	
	m_statusDRM = NONE;

    //
	// Individualization Complete            
    // Reopen the reader
	//
	SetCurrentStatus( INDIVIDUALIZED );
	
	return( m_pParent->ReopenReader( this ) );
}

//------------------------------------------------------------------------------
// Name: CDRM::GetNonSilentLicense()
// Desc: Handles WMT_NO_RIGHTS notifications in OnDRMStatus.
//       This function does non-silent acquisition for v1 only.
//       v7 non-silent licenses are acquired using AcquireLastV7LicenseNonSilently
//------------------------------------------------------------------------------
HRESULT CDRM::GetNonSilentLicense( BYTE *pValue )
{
    if( NULL == pValue )
    {
        return( E_INVALIDARG );
    }

	HRESULT hr = S_OK;
	LPTSTR  ptszEscapedURL = NULL;
	LPTSTR  ptszURL = NULL;
	TCHAR   tszErrMsg[256];
    DWORD   cchURL;

	ZeroMemory( ( void* )tszErrMsg, sizeof( tszErrMsg ) );

	do
	{
        //
        // m_pwszURL is the file we are trying to open.
        // It may be in the form of a local path. If it is, we
        // need to make it a URL by prepending "file://". We also
        // need to create escape sequences for certain characters if they
        // are present in the string.
		hr = MakeEscapedURL( m_pwszURL, &ptszEscapedURL );
		if( FAILED( hr ) )
		{
			break;
		}

        cchURL = _tcslen( ptszEscapedURL ) + wcslen( ( LPWSTR )pValue ) + 30;
		ptszURL = new TCHAR[ cchURL ];
		if( NULL == ptszURL )
		{
			_tcscpy_s( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Insufficient memory" ) );
			hr = HRESULT_FROM_WIN32( GetLastError() );
			break;
		}

        //
        // Construct the complete HTTP string required for v1 license requests
        //
		_stprintf_s( ptszURL, cchURL, _T( "%ws&filename=%s&embedded=false" ), ( LPWSTR )pValue, ptszEscapedURL );
		
		//
		// Launch this URL which will acquire the licence when the user fills all
		// the details asked at the site.
		//
        hr = LaunchURL( ptszURL );
        if( FAILED( hr ) )
        {
            _stprintf_s( tszErrMsg, ARRAYSIZE(tszErrMsg), _T( "Unable to launch the URL (Err=%#X)" ), hr );
			break;
        }
	}
	while( FALSE );

    delete[] ptszEscapedURL;
	delete[] ptszURL;

	if( FAILED( hr ) && ( _tcslen( tszErrMsg ) > 0 ) )
	{
		MessageBox( g_hwndDialog, tszErrMsg, ERROR_DIALOG_TITLE, MB_OK );
	}

	return( hr );
}

//------------------------------------------------------------------------------
// Name: CDRM::MakeEscapedURL()
// Desc: Put escape sequences in the file URL.
//------------------------------------------------------------------------------
HRESULT CDRM::MakeEscapedURL( LPCWSTR pwszInURL, LPTSTR *ppszOutURL )
{
    if( ( NULL == pwszInURL ) ||
        ( NULL == ppszOutURL ) )
    {
        return( E_INVALIDARG );
    }

    BOOL    bNeedFilePrefix = FALSE;
    DWORD   cEscapees = 0;
    DWORD   cchOutURL = 0;
    DWORD   cchToCopy = 0;
    LPCWSTR pwszTemp = NULL;
    LPTSTR  pchNext = NULL;
    LPCWSTR  pchToEscape = NULL;
    WCHAR   chEscape;

	//
    // Check if we need to pre-pend "file://" to the output URL
    //
    bNeedFilePrefix = ( NULL == wcsstr( pwszInURL, L"://" ) );

    //
    // Count how many characters need to be escaped
    //
    pwszTemp = pwszInURL;

    while( TRUE )
    {
        pchToEscape = wcspbrk( pwszTemp, L" #$%&\\+,;=@[]^{}" );
        if( NULL == pchToEscape )
        {
            break;
        }

        cEscapees++;
        pwszTemp = pchToEscape + 1;
    }

    //
    // Allocate space for out URL
    //
    cchOutURL = wcslen( pwszInURL ) + ( 2 * cEscapees ) + 1;

    if( bNeedFilePrefix )
    {
        cchOutURL += _tcslen( _T( "file://" ) );
    }

    *ppszOutURL = new TCHAR[ cchOutURL ];
    if( NULL == *ppszOutURL )
    {
		MessageBox( g_hwndDialog, _T( "Insufficient memory" ), ERROR_DIALOG_TITLE, MB_OK );
        return( HRESULT_FROM_WIN32( GetLastError() ) );
    }

    //
    // Fill in the output URL
    //
    pwszTemp = pwszInURL;
    pchNext = *ppszOutURL;

    if( bNeedFilePrefix )
    {
        _tcscpy_s( *ppszOutURL, cchOutURL, _T( "file://" ) );
        pchNext += _tcslen( _T( "file://" ) );
    }

	pchToEscape = NULL;

    while( TRUE )
    {
        pchToEscape = wcspbrk( pwszTemp, L" #$%&\\+,;=@[]^{}" );
        if( NULL == pchToEscape )
        {
            //
            // Copy the rest of the input string and get out
            //
            _stprintf_s( pchNext, cchOutURL, _T( "%ws" ), pwszTemp );
            break;
        }

        //
        // Copy all characters since the previous escapee
        //
        cchToCopy = pchToEscape - pwszTemp;
		chEscape = *pchToEscape;

        if( cchToCopy > 0 )
        {
            _stprintf_s( pchNext, cchOutURL, _T("%ws"), pwszTemp );
            pchNext += cchToCopy;
        }

        //
        // Expand this character into an escape code and move on
        //
        pchNext += _stprintf_s( pchNext, cchOutURL, _T( "%%%02x" ), chEscape );

        pwszTemp = pchToEscape + 1;
    }

    return( S_OK );
}

//------------------------------------------------------------------------------
// Name: CDRM::LaunchURL()
// Desc: Opens a URL in the browser.
//       ptszURL contains the license acquisition URL from the content header, 
//       our correctly formed file name, and the "&embedded=false" string
//------------------------------------------------------------------------------
HRESULT CDRM::LaunchURL( LPCTSTR ptszURL )
{
    DWORD               cchLaunchCommand = 0;
    HRESULT             hr = S_OK;
    LPTSTR              ptszParam = NULL;
    LPTSTR              ptszLaunchCommand = NULL;
    PROCESS_INFORMATION ProcInfo;
    STARTUPINFO         StartUp;
    TCHAR               tszShellOpenCommand[ MAX_PATH * 2 ];
    TCHAR               tszApplicationName[ MAX_PATH * 2 ];
    
    do
    {
        //
        // Find the appropriate command with which to launch URL
        //
        hr = GetShellOpenCommand( tszShellOpenCommand,
                                  sizeof( tszShellOpenCommand ),
                                  tszApplicationName,
                                  sizeof( tszApplicationName) );
        if( FAILED( hr ) )
        {
            break;
        }
    
        //
        // Build the appropriate command line by possibly substituting our URL parameter
        // First search the open command for "%1" or "%*"
        //
        ptszParam = _tcsstr( tszShellOpenCommand, _T( "\"%1\"" ) );
        if( NULL == ptszParam )
        {
            ptszParam = _tcsstr( tszShellOpenCommand, _T( "\"%*\"" ) );
        }
    
        if( NULL != ptszParam )
        {
            //
            // Substitute "%1" or "%*" with the URL
            //
            cchLaunchCommand = _tcslen( tszShellOpenCommand ) + _tcslen( ptszURL ) - 3; // +1 for NULL, -4 for "%1" or "%*"
            ptszLaunchCommand = new TCHAR[ cchLaunchCommand ];

            *ptszParam = _T( '\0' );
            _stprintf_s( ptszLaunchCommand, cchLaunchCommand, _T( "%s%s%s" ), tszShellOpenCommand, ptszURL, ptszParam + 4 );
        }
        else
        {
            //
            // No "%1" or "%*", so simply append the URL to the open command
            //
            cchLaunchCommand = _tcslen( tszShellOpenCommand ) + _tcslen( ptszURL ) + 1 + 1; // 1 for space, 1 for NULL
            ptszLaunchCommand = new CHAR[ cchLaunchCommand ];
            
            _stprintf_s( ptszLaunchCommand, cchLaunchCommand, _T( "%s %s" ), tszShellOpenCommand, ptszURL );
        }
    
        //
        // Use CreateProcess to launch the browser.
        //
        ZeroMemory( ( LPVOID )&ProcInfo, sizeof( PROCESS_INFORMATION ) );
        ZeroMemory( ( LPVOID )&StartUp, sizeof( STARTUPINFO ) );
    
        StartUp.cb = sizeof(STARTUPINFO);
    
        if( !CreateProcess( tszApplicationName,
                            ptszLaunchCommand,
                            NULL,
                            NULL,
                            FALSE,
                            0,
                            NULL,
                            NULL,
                            &StartUp,
                            &ProcInfo ) )
        {
            hr = HRESULT_FROM_WIN32( GetLastError() );
        }
        else
        {
            //
            // CreateProcess succeeded and we do not need the handles to the thread
            // or the process, so close them now.
            //
            if( NULL != ProcInfo.hThread )
            {
                CloseHandle( ProcInfo.hThread );
            }
    
    		if( NULL != ProcInfo.hProcess )
    		{
    			CloseHandle( ProcInfo.hProcess );
    		}
        }
    }
    while( FALSE );
    
    SAFE_ARRAYDELETE( ptszLaunchCommand );

    return( hr );
}

//------------------------------------------------------------------------------
// Name: CDRM::GetShellOpenCommand()
// Desc: Find the command for the shell's open verb associated with HTTP protocol.
//------------------------------------------------------------------------------
HRESULT CDRM::GetShellOpenCommand( LPTSTR ptszShellOpenCommand,
                                   DWORD cbShellOpenCommand,
                                   LPTSTR ptszApplicationName,
                                   DWORD cbApplicationName)
{
    HKEY hKey = NULL;
    LONG lResult;

    //
    // Find the command for the shell's open verb associated with HTTP protocol
    //
    lResult = RegOpenKeyEx( HKEY_CLASSES_ROOT,
                            _T( "http\\shell\\open\\command" ),
                            0,
                            KEY_READ,
                            &hKey );
    if( ERROR_SUCCESS == lResult )
    {
        lResult = RegQueryValueEx( hKey,
                                   NULL,
                                   0,
                                   NULL,
                                   ( BYTE* )ptszShellOpenCommand,
                                   &cbShellOpenCommand );
    }

    if( NULL != hKey )
    {
        RegCloseKey( hKey );
    }

    if( ERROR_SUCCESS != lResult )
    {
        return( HRESULT_FROM_WIN32( lResult ) );
    }

    //
    // Find the application name out of the open command, stripping quotes if necessary
    //
    TCHAR *pchFirst = ptszShellOpenCommand;
    TCHAR *pchNext = NULL;

    //
    // Strip out any leading space
    //
    while( _T( ' ' ) == *pchFirst )
    {
        pchFirst++;
    }

    //
    // Strip out quotes
    //
    if( _T( '"' ) == *pchFirst )
    {
        pchFirst++;
        pchNext = _tcschr( pchFirst, _T( '"' ) );
    }
    else
    {
        pchNext = _tcschr( pchFirst + 1, _T( ' ' ) );
    }

    if( NULL == pchNext )
    {
        pchNext = ptszShellOpenCommand + _tcslen( ptszShellOpenCommand );
    }

    _tcsncpy_s( ptszApplicationName, cbApplicationName, pchFirst, pchNext - pchFirst );
    ptszApplicationName[ pchNext - pchFirst ] = _T( '\0' );

    return( S_OK );
}


//------------------------------------------------------------------------------
// Name: CDRM::CopyWMGetLicenseData()
// Desc: Stores the WM_GET_LICENSE_DATA structure.for possible later use during
//       non-silent acquisition in AcquireLastV7LicenseNonSilently function
//------------------------------------------------------------------------------
HRESULT CDRM::CopyWMGetLicenseData( WM_GET_LICENSE_DATA* pOriginalData, WM_GET_LICENSE_DATA* pDestData, DWORD* pcbDestSize )
{
    HRESULT hr = S_OK;
    DWORD pcbRequiredSize;
    INT nURLLength;
    INT nLocalFilenameLength;
    
    if ( !pOriginalData )
    {
        return E_INVALIDARG;
    }
    
    if ( !pcbDestSize )
    {
        return E_POINTER;
    }
    
    //
    // Do a little verification of the original data
    //
    if ( ( NULL == pOriginalData->wszURL ) ||
         ( NULL == pOriginalData->wszLocalFilename ) ||
         ( NULL == pOriginalData->pbPostData && pOriginalData->dwPostDataSize != 0 ) )
    {
        return E_INVALIDARG;
    }
    
    //
    // Calculate the number of bytes required for the data
    //
    nURLLength = wcslen( pOriginalData->wszURL );
    nLocalFilenameLength = wcslen( pOriginalData->wszLocalFilename );
    pcbRequiredSize = sizeof( WM_GET_LICENSE_DATA ) +
                      ( nURLLength + 1 ) * sizeof( WCHAR ) +
                      ( nLocalFilenameLength + 1 ) * sizeof( WCHAR ) + 
                      pOriginalData->dwPostDataSize;

    //
    // Copy the structure, if there's enough space for it
    //
    if ( pDestData )
    {
        if ( *pcbDestSize >= pcbRequiredSize )
        {
            memcpy( pDestData, pOriginalData, sizeof( WM_GET_LICENSE_DATA ) );
        
            pDestData->wszURL = (LPWSTR) ( ((BYTE*) pDestData) + sizeof( WM_GET_LICENSE_DATA ) );
            pDestData->wszLocalFilename = pDestData->wszURL + nURLLength + 1;
            pDestData->pbPostData = (BYTE*) ( pDestData->wszLocalFilename + nLocalFilenameLength + 1 );
        
            wcscpy_s( pDestData->wszURL, nURLLength, pOriginalData->wszURL );
            wcscpy_s( pDestData->wszLocalFilename, nLocalFilenameLength, pOriginalData->wszLocalFilename );
            memcpy( pDestData->pbPostData, pOriginalData->pbPostData, pOriginalData->dwPostDataSize );
        }
        else
        {
            hr = E_INVALIDARG; // Buffer was too small
        }
    }
    
    //
    // Update the size required / copied
    //
    *pcbDestSize = pcbRequiredSize;
    
    return hr;
}


//------------------------------------------------------------------------------
// Name: CDRM::AcquireLastV7LicenseNonSilently()
// Desc: Tries to acquire the license non-silently.after silent acquisition failed
//------------------------------------------------------------------------------
HRESULT CDRM::AcquireLastV7LicenseNonSilently()
{
    HRESULT hr;
    LPTSTR tszLocalURL = NULL;
    
    //
    // Check to make sure we have info for a v7 license acquisition
    //
    if ( !m_pWMGetLicenseData )
    {
        return E_UNEXPECTED;
    }

    m_statusDRM = NONSILENT;

    do
    {

		hr = m_pDRMReader->MonitorLicenseAcquisition();
		if( FAILED(hr) )
			return hr;
        
        //
        // Launch the file in m_pWMGetLicenseData->wszLocalFilename, which will
        // redirect to the proper page for non-silent license acquisition
        //    
        hr = OpenURLWithData( m_pWMGetLicenseData->wszURL, m_pWMGetLicenseData->pbPostData );
    }
    while ( FALSE );
    
    SAFE_ARRAYDELETE( tszLocalURL );
    
    return hr;
}


//------------------------------------------------------------------------------
// Name: CDRM::GetLastDRMVersion()
// Desc: Retrieves the value of m_dwLastDRMVersion.
//------------------------------------------------------------------------------
DWORD CDRM::GetLastDRMVersion()
{
    return m_dwLastDRMVersion;
}


#endif // SUPPORT_DRM
