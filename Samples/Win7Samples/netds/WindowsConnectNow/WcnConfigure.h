// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
// Module Name:
//    WCNConfigure.h
//
// Abstract:
//		Global constants and headers for the sample.
// 
#define _CRT_RAND_S

#ifndef WCNCONFIGURE_H
#define WCNCONFIGURE_H

#include <atlbase.h>
#include <atlcom.h>
#include <windows.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <strsafe.h>

/// STL
#include <vector>
#include <list>

/// WCN API
#include "WcnApi.h"

//Function Discovery
#include <FunctionDiscoveryApi.h>
#include <FunctionDiscovery.h>
#include <FunctionDiscoveryKeys.h>

#include <Iphlpapi.h>
#include <wlanapi.h>


#include "WcnFdHelper.h" 
#include "WcnConnectNotify.h"

const DWORD UUID_LENGTH = 36; // number of chars in a UUID string

const DWORD Pin_Length_8 = 8; //valid max wcn pin length
const DWORD Pin_Length_4 = 4; //valid min wcn pin length

const UINT dwCharsToGenerate = 15;  // 14 chars for the passsphrase  xxxx-xxxx-xxxx 1 for the null terminator
char const *PassphraseCharacterSet = "012345678790abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMONPQRSTUVWXYZ";
const UINT PASSPHRASE_MAX_LENGTH = 63; // max length for wpa2 ascii
const UINT PASSPHRASE_MIN_LENGTH = 8; // min length for wpa2 ascii

const UINT Discovery_Event_Wait_Time_MS = 90000; 

const DWORD WINDOWS7_MAJOR_VERSION = 6;
const DWORD WINDOWS7_MINOR_VERSION = 1;

enum ConfigurationScenario
{
	DeviceConfigPushButton,
	DeviceConfigPin,
	RouterConfig,	
	PCConfigPushButton,
	PCConfigPin,
	None
};

typedef struct _WCN_DEVICE_INFO_PARAMETERS
{
	WCHAR wszDeviceName[65];
	WCHAR wszManufacturerName[129];
	WCHAR wszModelName[129];
	WCHAR wszModelNumber[129];
	WCHAR wszSerialNumber[129];
	UINT uConfigMethods;
}WCN_DEVICE_INFO_PARAMETERS;

typedef struct _CONFIGURATION_PARAMETERS
{
	WCN_PASSWORD_TYPE enumConfigType;
	UUID pDeviceUUID;
	WCHAR* pDevicePin;
	ConfigurationScenario enumConfigScenario;
	WCHAR* pSearchSSID;
	WCHAR* pProfilePassphrase;
	BOOL bFreePassphrase;
	BOOL bTurnOnSoftAP;
	WCHAR* pProfileSSID;
}CONFIGURATION_PARAMETERS;



#endif
