// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include <windows.h>
#include <stdlib.h>
#include <rpc.h>
#include <rpcasync.h>
#include <stdio.h>
#include <wlanapi.h>
#include "wcmapi.h"
#include <wchar.h>
#include <string.h>


#define CHOICE_EXIT 5
#define REGKEY_NOT_FOUND 2
#define INVALID_PARAMETER 87

//
// Display the list of interfaces, and associated profile names, and get the user interested interface
// GUID and the profile name
//
HRESULT GetInterfaceAndProfileName(_Out_ GUID *intfGuid, _Out_writes_(profNameLen) LPWSTR profName, _In_ DWORD profNameLen);

//
// Displays meaningful cost values to the user
//
void DisplayCostDescription (_In_ DWORD cost);

//
// Displays cost source to the user
//
void DisplayCostSource (_In_ WCM_CONNECTION_COST_SOURCE costSource);

//
// Displays profile data values to the user
//
void DisplayProfileData (_In_ WCM_DATAPLAN_STATUS* pProfileData);

//
//  Display the Profile Plan Duration 
//
void DisplayProfilePlanDuration (_In_ WCM_TIME_INTERVAL Duration);

//
//  Return TRUE if Profile Plan Duration is available
//
BOOL IsProfilePlanDurationAvailable (_In_ WCM_TIME_INTERVAL Duration);

//
// Checks if the profile data values are default values, or provided by the MNO
//
BOOL IsProfileDataAvailable(_In_ WCM_DATAPLAN_STATUS *pProfileData);

//
// Converts file time to local time, to display to the user
//
void PrintFileTime(_In_ FILETIME time);

//
// Clears any input lingering in the STDIN buffer
//
void FlushCurrentLine();

//
// Displays error description
//
void DisplayError(_In_ DWORD dwError);

//
// Choose indices for Interface Guids and Profile names  displayed
//
void GetInterfaceIdAndProfileIndex(_In_ int numIntfItems, _In_ int numProfNames, _Out_ int *iIntf, _Out_ int *iProfile);
