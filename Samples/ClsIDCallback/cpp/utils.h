#pragma once

#include "stdafx.h"
#include "NotifyInterfaceImp.h"

const CLSID CLSID_CNotifyInterfaceImp = {0xC48FF713,0xB257,0x4242,{0xB1,0xFF,0xA8,0x6B,0x61,0xDF,0x3B,0x3E}};

const WCHAR JOBTRANSFERRED_EVENTNAME[] = L"EVENT_CALLBACK_JOBTRANSFERRED";
const WCHAR JOBERROR_EVENTNAME[] = L"EVENT_CALLBACK_JOBERROR";

HRESULT UnRegisterServer();
void FormatObjectPath();
HRESULT RegisterServer();

typedef struct
{
    LPWSTR RemoteFile;
    LPWSTR LocalFile;
} DOWNLOAD_FILE;

template <class T> 
void RegisterClassObject(REFCLSID rclsid, LPDWORD lpdwCookieReceiver, T* pT_in = NULL)
{
	T* pT = NULL;
	HRESULT	hr = S_OK;

	if (pT_in)
	{
		pT = pT_in;
		pT -> AddRef();
	}
	else
	{
		pT = new T();
	}

	if (pT)
	{
		IUnknown* pIUnknown = NULL;

		pT -> QueryInterface(IID_IUnknown, (void**)&pIUnknown);

		if (pIUnknown)
		{
			hr = ::CoRegisterClassObject
				(
				rclsid,
				pIUnknown,
				CLSCTX_LOCAL_SERVER,
				REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED,
				lpdwCookieReceiver
				);

			pIUnknown -> Release();
			pIUnknown = NULL;
		}

		pT -> Release();
		pT = NULL;
	}
};
