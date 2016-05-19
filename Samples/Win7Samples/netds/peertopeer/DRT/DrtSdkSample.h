// DrtSdkSample.h - Interface for DrtSdkSample

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include <tchar.h>
#include <stdio.h>
#include <drt.h>
#include <vector>

#define KEYSIZE 32
#define VERIFY_OR_ABORT(funcname, hrE) { DisplayError(L#funcname,hr); if (FAILED((hrE))) goto Cleanup; }

//
// Contains the information required for each DRT Registration
// 
typedef struct _REG_CONTEXT
{
    HDRT_REGISTRATION_CONTEXT       hDrtReg;
    DRT_REGISTRATION                regInfo;
} REG_CONTEXT;

//
// Contains the information for a DRT Instance
//
typedef struct _DRT_CONTEXT
{
    HDRT                            hDrt;
    HANDLE                          eventHandle;
    HANDLE                          DrtWaitEvent;
    UINT                            BootstrapProviderType;
    UINT                            SecurityProviderType;
    USHORT                          port;
    DRT_SETTINGS                    settings;
    CERT_CONTEXT                    *pRoot;
    CERT_CONTEXT                    *pLocal;
    std::vector<REG_CONTEXT>        registrations;
} DRT_CONTEXT;

//
// Maps common HRESULTs to descriptive error strings
//
void DisplayError(__in const PWSTR fnname, __in const HRESULT hr);

//
// Callback to handle general DRT Events. 
// These include registration state changes, leafset changes, and status changes.
//
void CALLBACK DrtEventCallback(__in PVOID Param, __in BOOLEAN TimedOut);

//
// Initializes and brings a DRT instance online
//   1) Brings up an ipv6 transport layer
//   2) Attaches a security provider (according to user's choice)
//   3) Attaches a bootstrap provider (according to user's choice)
//   4) Calls DrtOpen to bring the DRT instance online
//
bool InitializeDrt(DRT_CONTEXT *Drt);

//
// Initializes and performs a search through the DRT
//
bool PerformDrtSearch(DRT_CONTEXT* Drt, INT SearchType);

//
// Prints the search path that corresponds to the latest resolve result
//
void PrintSearchPath(HDRT_SEARCH_CONTEXT SearchContext);

//
// Registers a key in the current DRT Instance
//
bool RegisterKey(DRT_CONTEXT* Drt);

//
// Unregisters a previously registered key
//
bool UnRegisterKey(DRT_CONTEXT* Drt);

//
// Deletes and Frees the various objects and providers used by the DRT
//
void CleanupDrt(DRT_CONTEXT *Drt);

//
// Clears any input lingering in the STDIN buffer
//
void FlushCurrentLine();

//
// Presents an interactive menu to the user and returns the user's choice
//   Input: 
//      PCWSTR *choices - An array of strings representing the choices to be present to the users
//
int GetUserChoice(PCWSTR *choices, int numchoices);

//
// Gets a registration key from the user (used for registration and search)
//
bool GetKeyFromUser(PCWSTR pcwszKeyName, BYTE* KeyData);

