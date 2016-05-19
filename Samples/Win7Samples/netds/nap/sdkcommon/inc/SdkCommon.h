// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#pragma once
#ifndef __SDKCOMMON_H__
#define __SDKCOMMON_H__

#include <windows.h>
#include <NapProtocol.h>

namespace SDK_SAMPLE_COMMON
{

// The System Health ID used by both the SDK SHA (client) & SDK SHV (server).
static const UINT32 QuarSampleSystemHealthId = 0x000137F0;

// Vendor-specific data sent by the client (SHA) to the server (SHV).
// The physical TLV's Value contains the Vendor ID, then this data.

static const BYTE   SDK_CLIENT_VENDOR_DATA_HEALTHY[]    = { 0xAA, 0xAA };
static const BYTE   SDK_CLIENT_VENDOR_DATA_UNHEALTHY[]    = { 0xBA, 0xAD };
static const size_t SDK_CLIENT_VENDOR_DATA_SIZE = sizeof(SDK_CLIENT_VENDOR_DATA_HEALTHY);

// Maximum allowed threads for the sample SHV
#define SDK_SHV_MAX_THREADS     128

// The byte size of MAX_PATH in WCHARs, with 1 extra WCHAR for a trailing NULL.
static const size_t MAX_PATH_WCHAR_SIZE = (sizeof(WCHAR) *(MAX_PATH+1));



// Both INapSoHConstructor::Initialize() and INapSoHProcessor::Initialize()
// have a boolean input parameter that indicates whether the SoH being
// manipulated is an SoH Request (TRUE) or an SoH Response (FALSE).  The
// following variables hide the actual boolean values behind named constants
// that are less ambiguous, to prevent confusion from other possible
// interpretations of the literal boolean values.

static const BOOL SOH_REQUEST  = TRUE;
static const BOOL SOH_RESPONSE = FALSE;


//
// SDK Note:
// Define any supplemental error codes specific to the System Health
// components under development.  See the SDK header winerror.h for
// information regarding the contents of HRESULTs, to determine the best
// way to construct new vendor-specific error values.
//


//
// Facility 0x27: The Windows Network Access Protection (NAP) system.
//
// Error: QUAR_E_NOTPATCHED
//
// Meaning: The client must install patches before it will be considered
// healthy.

#define QUAR_E_NOTPATCHED             _HRESULT_TYPEDEF_(0x80270050L)

//
// Error: QUAR_E_COMPLIANT
//
// Meaning: The client is considered healthy.

#define QUAR_E_COMPLIANT             _HRESULT_TYPEDEF_(0x00000000L)



// Helper Function for populating already allocated CountedString structures
// This makes filling the NapRegistrationInfo struct cleaner in the sample code
HRESULT FillCountedString (const WCHAR* src, CountedString * dest) throw ();

// Helper Function for depopulating CountedString structures
// leaves the structure intact, but frees the buffer underneath
// for use with FillCountedString above
HRESULT EmptyCountedString( CountedString * cs ) throw ();


// Create an SoH Constructor object.
HRESULT CreateOutputSoHConstructor(INapSoHConstructor* &pSohResponse,
                                   SystemHealthEntityId  systemHealthId,
                                   BOOL                  sohType) throw();


// Create an SoH Processor object.

HRESULT CreateInputSoHProcessor(INapSoHProcessor*   &pSohResponse,
                                SystemHealthEntityId &systemHealthId,
                                BOOL                  sohType,
                                SoH                  *pInputSoh) throw();


// Release a COM reference (pointer) to an IUnknown object.

void ReleaseObject(IUnknown *pIUnknown) throw();


// Free a WCHAR string buffer. Upon exit, the input variable will be set to
// NULL.

void FreeMemory(WCHAR* &pAllocatedMemory) throw();


// Free a CountedString struct, including the buffer pointed to by the
// internal string member. Upon exit, the input variable will be set to NULL.

void FreeMemory(CountedString* &pAllocatedMemory) throw();


// Allocate a buffer to contain a WCHAR string, which is "stringSizeInBytes" wchars long.

HRESULT AllocateMemory(OUT WCHAR* &pString, IN size_t stringSizeInBytes) throw();


// Allocate a buffer to contain a CountedString struct, whose overall string member's
// buffer is "stringSizeInBytes" wchars long.

HRESULT AllocateMemory(OUT CountedString* &pString, IN size_t stringSizeInBytes) throw();

}  // End "namespace SDK_SAMPLE_COMMON".

// The CLSID for the Config COM object
static const GUID CLSID_SDK_SHV_UI = { /* 230b2a03-bbb3-4d50-839b-74f095e2b53e */
    0x230b2a03,
    0xbbb3,
    0x4d50,
    {0x83, 0x9b, 0x74, 0xf0, 0x95, 0xe2, 0xb5, 0x3e}
  };


#endif  // __SDKCOMMON_H__
