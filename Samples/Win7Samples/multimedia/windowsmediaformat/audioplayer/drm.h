//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            Drm.h
//
// Abstract:            Definition of class CDRM which handles DRM version 1 and 
//                      version 7 license acquisition for protected content.
//
//*****************************************************************************

//
#if !defined(AFX_DRM_H__45D1728D_0E30_436B_BA83_9DAEF4D21BD7__INCLUDED_)
#define AFX_DRM_H__45D1728D_0E30_436B_BA83_9DAEF4D21BD7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Defines the current DRM operation
typedef enum DRMSTATUS
{
	NONE = 0,
	SILENT, // Silent license acquisition (DRM Version 7 only)
	NONSILENT, // Non-silent license acquisition
	INDIVIDUALIZE // Downloading individualized DRM component (DRM version 7 only)
} DRMSTATUS;

///////////////////////////////////////////////////////////////////////////////
class CDRM  
{
public:
	CDRM();
	~CDRM();

	HRESULT OnDRMStatus( /* [in] */ WMT_STATUS Status,
						 /* [in] */ HRESULT hr,
						 /* [in] */ WMT_ATTR_DATATYPE dwType,
						 /* [in] */ BYTE __RPC_FAR *pValue,
						 /* [in] */ void __RPC_FAR *pvContext );
	HRESULT Init( CAudioPlay* pParent, IWMReader* pReader, LPCWSTR pwszURL );
	HRESULT Exit();
	HRESULT Cancel();
	
    HRESULT AcquireLastV7LicenseNonSilently();
    DWORD GetLastDRMVersion();

private:

	HRESULT GetNonSilentLicense( BYTE* pValue );
	HRESULT HandleAcquireLicense( BYTE *pValue );
	HRESULT HandleIndividualize( BYTE *pValue );
	HRESULT MakeEscapedURL( LPCWSTR pwszInURL, LPTSTR *ppszOutURL );
    HRESULT LaunchURL( LPCTSTR ptszURL );
    HRESULT GetShellOpenCommand( LPTSTR ptszShellOpenCommand,
                                 DWORD cbShellOpenCommand,
                                 LPTSTR ptszApplicationName,
                                 DWORD cbApplicationName);
	static HRESULT CopyWMGetLicenseData( WM_GET_LICENSE_DATA* pOriginalData, WM_GET_LICENSE_DATA* pDestData, DWORD* pcbDestSize );

private:
	CAudioPlay*		m_pParent;
	IWMDRMReader*	m_pDRMReader;
	LPWSTR			m_pwszURL;
	DRMSTATUS		m_statusDRM;
    WM_GET_LICENSE_DATA* m_pWMGetLicenseData;
    DWORD           m_dwLastDRMVersion;

	BOOL			m_bDoNotConnectToUntrustedURL;
};

#endif // !defined(AFX_DRM_H__45D1728D_0E30_436B_BA83_9DAEF4D21BD7__INCLUDED_)
