//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            wmprop.h
//
// Abstract:            Interface of CWMProp class
//
//*****************************************************************************
//
#pragma once
#include <stdio.h>
#include <tchar.h>
#include "wmsdk.h"

class CWMProp : public IWMReaderCallback
{
public:
	CWMProp( HRESULT *phr );
	~CWMProp();
	HRESULT Open( LPCWSTR pwszFileName );
private:
	HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid,
													  void **ppvObject );
	ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE OnSample( /* [in] */ DWORD dwOutputNum,
										/* [in] */ QWORD cnsSampleTime,
										/* [in] */ QWORD cnsSampleDuration,
										/* [in] */ DWORD dwFlags,
										/* [in] */ INSSBuffer __RPC_FAR *pSample,
										/* [in] */ void __RPC_FAR *pvContext);
    HRESULT STDMETHODCALLTYPE OnStatus( /* [in] */ WMT_STATUS Status,
										/* [in] */ HRESULT hr,
										/* [in] */ WMT_ATTR_DATATYPE dwType,
										/* [in] */ BYTE __RPC_FAR *pValue,
										/* [in] */ void __RPC_FAR *pvContext);
	HRESULT OpenFileWithEditor();
	HRESULT OpenFileWithReader();
	HRESULT GetBoolAttribsFromEditor( LPCWSTR pwszName, BOOL *pResult );
	HRESULT GetPropertiesFromProfile();
	HRESULT PrintCodecName( IWMStreamConfig *pConfig );
	

private:
	IWMReader          *m_pReader;
	IWMHeaderInfo      *m_pHeaderInfo;
	IWMMetadataEditor  *m_pIWMEditor;
	IWMProfile         *m_pProfile;
	WCHAR              m_wszFileName[ MAX_PATH ];
	HANDLE             m_hOpenEvent;
	HANDLE             m_hCloseEvent;
	BOOL               m_bIsDRM;
	HRESULT            m_hr;
};

//
// MACROS
//

#define SAFE_RELEASE(p) if ( p ) { p->Release(); p = NULL; }
#define SAFECLOSE_HANDLE(p) if ( p ) { CloseHandle( p ); p = NULL; }
