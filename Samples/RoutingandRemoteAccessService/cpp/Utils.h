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
#include "wcmapi.h"
#include <wchar.h>
#include <string.h>
#include <strsafe.h>
#include <mprerror.h>
#include <MprApi.h>

#define MPRAPI_SAMPLE_MALLOC(_size)       HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (_size))
#define MPRAPI_SAMPLE_FREE(_ptr)          if((_ptr)) { HeapFree(GetProcessHeap(),0, (_ptr)); (_ptr) = NULL; }

#define RAS_MaxEkuOID         512

// MPR admin API function pointers
//
typedef DWORD (APIENTRY * PMPRADMININTERFACECREATE)(HANDLE, DWORD, LPBYTE, HANDLE*);
extern PMPRADMININTERFACECREATE g_pMprAdminInterfaceCreate;

typedef DWORD (APIENTRY * PMPRADMININTERFACEGETHANDLE)(HANDLE, LPWSTR, HANDLE*, BOOL);
extern PMPRADMININTERFACEGETHANDLE g_pMprAdminInterfaceGetHandle;

typedef DWORD (APIENTRY * PMPRADMININTERFACEGETINFO)(HANDLE, HANDLE, DWORD, LPBYTE*);
extern PMPRADMININTERFACEGETINFO g_pMprAdminInterfaceGetInfo;

typedef DWORD (APIENTRY * PMPRADMININTERFACESETCUSTOMINFOEX)(HANDLE, HANDLE, MPR_IF_CUSTOMINFOEX*);
extern PMPRADMININTERFACESETCUSTOMINFOEX g_pMprAdminInterfaceSetCustomInfoEx;

typedef DWORD (APIENTRY * PMPRADMININTERFACEGETCUSTOMINFOEX)(HANDLE, HANDLE, MPR_IF_CUSTOMINFOEX*);
extern PMPRADMININTERFACEGETCUSTOMINFOEX g_pMprAdminInterfaceGetCustomInfoEx;

typedef DWORD (APIENTRY * PMPRADMINBUFFERFREE)(LPVOID);
extern PMPRADMINBUFFERFREE  g_pMprAdminBufferFree;

typedef DWORD (APIENTRY * PMPRADMINSERVERDISCONNECT)(HANDLE);
extern PMPRADMINSERVERDISCONNECT g_pMprAdminServerDisconnect;

typedef DWORD (APIENTRY * PMPRADMINSERVERCONNECT)(LPWSTR,HANDLE*);
extern PMPRADMINSERVERCONNECT g_pMprAdminServerConnect;

typedef DWORD (APIENTRY * PMPRCONFIGBUFFERFREE)(LPVOID);
extern PMPRCONFIGBUFFERFREE g_pMprConfigBufferFree;

typedef DWORD (APIENTRY * PMPRADMINSERVERGETINFOEX)(MPR_SERVER_HANDLE, MPR_SERVER_EX*);
extern PMPRADMINSERVERGETINFOEX g_pMprAdminServerGetInfoEx;

typedef DWORD (APIENTRY * PMPRADMINSERVERSETINFOEX)(MPR_SERVER_HANDLE, MPR_SERVER_SET_CONFIG_EX*);
extern PMPRADMINSERVERSETINFOEX g_pMprAdminServerSetInfoEx;

typedef DWORD (APIENTRY * PMPRADMINCONNECTIONENUM)(MPR_SERVER_HANDLE, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD, LPDWORD);
extern PMPRADMINCONNECTIONENUM g_pMprAdminConnectionEnum;

// MPR config API function pointers
//
typedef DWORD (APIENTRY * PMPRCONFIGSERVERCONNECT)(LPWSTR,HANDLE*);
extern PMPRCONFIGSERVERCONNECT g_pMprConfigServerConnect;

typedef VOID (APIENTRY * PMPRCONFIGSERVERDISCONNECT)(HANDLE);
extern PMPRCONFIGSERVERDISCONNECT g_pMprConfigServerDisconnect;

typedef DWORD (APIENTRY * PMPRCONFIGINTERFACECREATE)(HANDLE, DWORD, LPBYTE, HANDLE*);
extern PMPRCONFIGINTERFACECREATE g_pMprConfigInterfaceCreate;

typedef DWORD (APIENTRY * PMPRCONFIGINTERFACEGETHANDLE)(HANDLE, LPWSTR, HANDLE*);
extern PMPRCONFIGINTERFACEGETHANDLE g_pMprConfigInterfaceGetHandle;

typedef DWORD (APIENTRY * PMPRCONFIGINTERFACESETCUSTOMINFOEX)(HANDLE,HANDLE, MPR_IF_CUSTOMINFOEX*);
extern PMPRCONFIGINTERFACESETCUSTOMINFOEX g_pMprConfigInterfaceSetCustomInfoEx;

typedef DWORD (APIENTRY * PMPRCONFIGINTERFACEGETCUSTOMINFOEX)(HANDLE,HANDLE, MPR_IF_CUSTOMINFOEX*);
extern PMPRCONFIGINTERFACEGETCUSTOMINFOEX g_pMprConfigInterfaceGetCustomInfoEx;

typedef DWORD (APIENTRY * PMPRCONFIGSERVERSETINFOEX)(MPR_SERVER_HANDLE, MPR_SERVER_SET_CONFIG_EX*);
extern PMPRCONFIGSERVERSETINFOEX g_pMprConfigServerSetInfoEx;

typedef DWORD (APIENTRY * PMPRCONFIGSERVERGETINFOEX)(MPR_SERVER_HANDLE, MPR_SERVER_EX*);
extern PMPRCONFIGSERVERGETINFOEX g_pMprConfigServerGetInfoEx;

//
// Loads the MprAPI.dll and imports the required APIs
//
DWORD LoadMprApiLibrary();

//
// Free the MprAPI.dll if already loaded
//
VOID ReleaseMprApiLibrary();

//
// Connects to the RRAS server for configuration and administration 
//
DWORD RemoteAccessServerConenct(
    _In_opt_ LPWSTR serverName, 
    _Out_ MPR_SERVER_HANDLE* serverHandleAdmin,
    _Out_ HANDLE* serverHandleConfig
    );
//
// Disconnects the connection with remote access server made in RemoteAccessServerConenct.
//
VOID RemoteAccessServerDisconenct(
    _In_ MPR_SERVER_HANDLE serverHandleAdmin,
    _In_ HANDLE serverHandleConfig
    );

// 
// Retrieves the firsts certificate from the system certificate store and returns it's certificate name  and Hash blob.
//
DWORD GetCertificateNameAndHashBlob(
    _Out_ CERT_NAME_BLOB* certName,
    _Out_ CERT_NAME_BLOB* certHash
    );

//
// Prints the specified integrity method in string format 
//
VOID PrintIntegrityMethod(
    _In_ DWORD integrityMethod
    );

//
// Prints the specified encryption method in string format 
//
VOID PrintEncryptionMethod(
    _In_ DWORD encryptionMethod
    );

//
// Prints the specified cipher transform constant in string format 
//
VOID PrintCipherTransformConstant(
    _In_ DWORD cipherTransformConstant
    );

//
// Prints the specified auth transform constant in string format 
//
VOID PrintAuthTransformConstant(
    _In_ DWORD authTransformConstant
    );
//
// Prints the specified PFS group in string format 
//
VOID PrintPfsGroup(
    _In_ DWORD pfsGroup
    );

//
// Prints the specified DH group in string format 
//
VOID PrintDHGroup(
    _In_ DWORD dhGroup
    );

//
// Prints various fields of the specified ROUTER_CUSTOM_IKEv2_POLICY0 structure 
// in string format
//
VOID PrintCustomIkev2Policy(
    _In_ LPWSTR prefix, 
    _In_ ROUTER_CUSTOM_IKEv2_POLICY0* customIkev2Policy
    );

//
// Prints the specified interface type in string format  
//
VOID PrintInterfaceType(
    _In_ ROUTER_INTERFACE_TYPE ifType
    );

//
// Prints the specified connection state in string format  
//
VOID PrintConnectionState(
    _In_ ROUTER_CONNECTION_STATE connState
    );

//
// Prints the specified encryption type in string format  
//
VOID PrintEncryptionType(
    _In_ DWORD encryptionType
    );

//
// Prints the specified entry type in string format  
//
VOID PrintEntryType(
    _In_ DWORD entryType
    );

//
// Prints the specified VPN strategy in string format  
//
VOID PrintVpnStrategy(
    _In_ DWORD vpnStrategy
    );

//
// Converts file time to local time, to display to the user
//
VOID PrintFileTime(
    _In_ FILETIME* time
    );

//
// Clears any input lingering in the STDIN buffer
//
VOID FlushCurrentLine();

//
// Displays error description
//
VOID DisplayError(
    _In_ DWORD error
    );


//********************************************************************************************
//              EKU related methods               
//********************************************************************************************

// Defining a structure for storing EKU information
    typedef struct _EKU_Info
    {
        const WCHAR       *Name;
        bool              IsOid;
    } EKU_Info;

// 
// Gets some basic certificate EKUs and returns it's EKUs  and number of EKU.
//
DWORD GetCertificateEkus(
    _Out_ PMPR_CERT_EKU*    certEKUs,
    _Out_ DWORD*            TotalEKUs
    );

//
//This method frees the memory allocated for EKUs
//
VOID FreeCertificateEKU(
    _In_ PMPR_CERT_EKU      certificateEKUs,
    _In_ DWORD              dwTotalEkus
    );


