#ifndef __WIA_SCANPROFILE_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//------------------------------------------------------------
//Please read the ReadME.txt which explains the purpose of the
//sample.
//-------------------------------------------------------------
#define __WIA_SCANPROFILE_SAMPLE

#define PROFILE_NAME TEXT("New_Profile")

#include "scanprofiles.h"
// scanprofiles.h is a generated file generated from scanprofiles.idl by running midl.
#include "utils.h"

HRESULT CreateScanProfilesWithoutUI();

HRESULT CreateScanProfileUI(HWND hwndParent);

HRESULT CreateScanProfiles(BSTR bstrDeviceID , BSTR bstrDeviceName);

HRESULT ReadDevIDandDevName( IWiaPropertyStorage *pWiaPropertyStorage , BSTR* pbstrDevID , BSTR* pbstrDevName);

HRESULT EnumWiaDevicesandCreateScanProfiles( IWiaDevMgr2 *pWiaDevMgr2 );

HRESULT ProfileDisplayDeviceID(IScanProfile* pScanProfile);

HRESULT ProfileSetAndDisplayName(IScanProfile* pScanProfile);

HRESULT GetNumProfiles(IScanProfileMgr* pScanProfileMgr, BSTR bstrDevID, ULONG* lNumProfiles);

HRESULT DisplayAllProfilesForDevice(IScanProfileMgr* pScanProfileMgr,BSTR bstrDevID, ULONG lNumProfiles);    



#endif


