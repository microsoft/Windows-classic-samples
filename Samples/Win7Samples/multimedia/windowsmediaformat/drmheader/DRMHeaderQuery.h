//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            DrmHeaderQuery.h
//
// Abstract:            Declaration of CDRMHeaderQuery class, which is used to 
//						handle the IWMDRMEditor interface of the WM Format SDK
//
//*****************************************************************************

#ifndef _CDRMHeaderQuery_H
#define _CDRMHeaderQuery_H

#include "stdafx.h"

class CDRMHeaderQuery  
{
public:
	CDRMHeaderQuery();
	virtual ~CDRMHeaderQuery();

	HRESULT Open( WCHAR *pwszFileName );
	HRESULT Close();

	//	QueryProperty() will interrogate the property through the DRM editor
	//	and put the value in m_pValue, the length in m_wValueLength
	HRESULT QueryProperty( const WCHAR *pwszPropertyName );

	HRESULT PrintProperty( const WCHAR *pwszPropertyName );

protected:

// SDK interface pointers
	IWMMetadataEditor	*m_pEditor;
	IWMMetadataEditor2	*m_pEditor2;
	IWMDRMEditor		*m_pDRMEditor;

// variables to hold the query results after QueryProperty is called
	WMT_ATTR_DATATYPE	m_Wmt;
	BYTE				*m_pValue;
	WORD				m_wValueLength;
	
};

#endif // !defined(AFX_DRMHEADERQUERY_H__B64CDEFD_5299_47A6_9C92_957845F9271B__INCLUDED_)
