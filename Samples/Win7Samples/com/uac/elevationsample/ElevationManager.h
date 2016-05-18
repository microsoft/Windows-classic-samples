// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include <Windows.h>
#include <objbase.h>
#include <tchar.h>
#include <sddl.h>
#include <strsafe.h>

#define MonikerName L"Elevation:Administrator!new:%s"	//Moniker to use
#define CLSIDSize 45									//size required for CLSID
#define MonikerSize	30									//size required for moniker name

class ElevationManager 
{
private:
	HRESULT CInitHR;
public:
	ElevationManager(__out HRESULT *);
	~ElevationManager(void);

	HRESULT RegisterServer(const TCHAR *, HWND);
	HRESULT UnRegisterServer(const TCHAR *, HWND);
	LRESULT SetButtonShield(HWND);
private:
	 HRESULT CoCreateInstanceAsAdmin(HWND, REFCLSID, REFIID, __out void **);
	 BOOL GetAccessPermissionsForLUAServer(SECURITY_DESCRIPTOR **);
	 BOOL SetAccessPermissions(HKEY, PSECURITY_DESCRIPTOR);
	 HRESULT RegisterAsExe(const TCHAR *, HWND, BOOL);
	 HRESULT RegisterAsDll(const TCHAR *, HWND, BOOL);
	 BOOL IsFileExe(const TCHAR *);
};
