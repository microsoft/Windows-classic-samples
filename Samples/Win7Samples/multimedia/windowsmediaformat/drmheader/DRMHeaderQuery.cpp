//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DrmHeaderQuery.h
//
// Abstract:            Implementation of CDRMHeaderQuery class.
//
//*****************************************************************************

#include "stdafx.h"
#include "DRMHeaderQuery.h"


//------------------------------------------------------------------------------
// Name: CDRMHeaderQuery::CDRMHeaderQuery(()
// Desc: Constructor.
//------------------------------------------------------------------------------
CDRMHeaderQuery::CDRMHeaderQuery()
{
	m_pEditor		= NULL;
	m_pEditor2		= NULL;
	m_pDRMEditor	= NULL;
	m_pValue		= NULL;
}

//------------------------------------------------------------------------------
// Name: CDRMHeaderQuery::~CDRMHeaderQuery(()
// Desc: Destructor.
//------------------------------------------------------------------------------
CDRMHeaderQuery::~CDRMHeaderQuery()
{
	Close();

	SAFE_RELEASE( m_pDRMEditor );
	SAFE_RELEASE( m_pEditor );
	SAFE_RELEASE( m_pEditor2 );
	SAFE_ARRAYDELETE( m_pValue );
}

//------------------------------------------------------------------------------
// Name: CDRMHeaderQuery::Open()
// Desc: Create the editor and open the file.
//------------------------------------------------------------------------------
HRESULT CDRMHeaderQuery::Open( WCHAR *pwszFileName )
{
	HRESULT hr;
	
	if( NULL == m_pEditor )
	{
		hr = WMCreateEditor( &m_pEditor );
		if( FAILED( hr ) )
		{
			return hr;
		}

		hr = m_pEditor->QueryInterface( IID_IWMMetadataEditor2, ( void ** ) &m_pEditor2 );
		if( FAILED(hr) )
		{
			return hr;
		}

		hr = m_pEditor->QueryInterface( IID_IWMDRMEditor, ( void ** ) &m_pDRMEditor );
		if( FAILED( hr ) )
		{
			return hr;
		}
	}

	//
	//	We want to use OpenEx so that we can specify FILE_SHARE_READ access flag
	//	There is no need to lock the file, because we are only reading an attribute from it.
	//
	//	Otherwise, IWMMetadataEditor::Open() could well be used instead of this
	//	(also skipping the QI step for IWMMetadataEditor2)
	//

	return m_pEditor2->OpenEx( pwszFileName, GENERIC_READ, FILE_SHARE_READ );	

}

//------------------------------------------------------------------------------
// Name: CDRMHeaderQuery::Close()
// Desc: Closes the file.
//------------------------------------------------------------------------------//------------------------------------------------------------------------------
HRESULT CDRMHeaderQuery::Close()
{
	if( NULL != m_pEditor )
	{
		return m_pEditor->Close();
	}
	else
	{
		return NS_E_INVALID_REQUEST;
	}
}

//------------------------------------------------------------------------------
// Name: CDRMHeaderQuery::QueryProperty()
// Desc: Gets the specified DRM property.
//------------------------------------------------------------------------------
HRESULT CDRMHeaderQuery::QueryProperty( const WCHAR *pwszPropertyName )
{
	HRESULT hr;

	if( NULL == m_pDRMEditor )
	{
		return NS_E_INVALID_REQUEST;
	}

	SAFE_ARRAYDELETE( m_pValue );

	hr = m_pDRMEditor->GetDRMProperty( pwszPropertyName,
			  						   &m_Wmt,
									   NULL,
									   &m_wValueLength );
	if( FAILED( hr ) )
	{
		return hr;
	}

	m_pValue = new BYTE[ m_wValueLength ];
	if( NULL == m_pValue )
	{
		return E_OUTOFMEMORY;
	}

	hr = m_pDRMEditor->GetDRMProperty( pwszPropertyName,
			  						   &m_Wmt,
									   m_pValue,
									   &m_wValueLength );
	if( FAILED( hr ) )
	{
		SAFE_ARRAYDELETE( m_pValue );
		return hr;
	}
		

	return hr;
}

//------------------------------------------------------------------------------
// Name: CDRMHeaderQuery::PrintProperty()
// Desc: Display the specified property.
//------------------------------------------------------------------------------
HRESULT CDRMHeaderQuery::PrintProperty( const WCHAR *pwszPropertyName )
{
	HRESULT hr = QueryProperty( pwszPropertyName );
	if( FAILED( hr ) )
	{
		return hr;
	}

	if( WMT_TYPE_STRING == m_Wmt )
	{
		wprintf( L"%s :\t%s\n", pwszPropertyName, (WCHAR *) m_pValue );
		return S_OK;
	}
	else if( WMT_TYPE_BOOL == m_Wmt )
	{
		WCHAR *wBoolNames[2]={ L"TRUE", L"FALSE" };
		
		WCHAR *pwszResult;
		
		if( (BOOL) *m_pValue )
			pwszResult = wBoolNames[0];
		else
			pwszResult = wBoolNames[1];

		wprintf( L"%s :\t%s\n", pwszPropertyName, pwszResult );
		return S_OK;
	}
	else if( WMT_TYPE_WORD == m_Wmt )
	{
		wprintf( L"%s :\t%d\n", pwszPropertyName, (WORD) *m_pValue );
		return S_OK;
	}
	else if( WMT_TYPE_DWORD == m_Wmt )
	{
		wprintf( L"%s :\t%d\n", pwszPropertyName, (DWORD) *m_pValue );
		return S_OK;
	}
	else if( WMT_TYPE_QWORD == m_Wmt )
	{
		wprintf( L"%s :\t%I64u\n", pwszPropertyName, (QWORD) *m_pValue );
		return S_OK;
	}
	else
	{
		wprintf( L"%s :\t(query successful, but cannot format for display)\n", pwszPropertyName );
		return S_OK;
	}
	
	return E_FAIL;
}
