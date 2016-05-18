// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.



#ifndef __SDKMETHODCOMMON_H__
#define __SDKMETHODCOMMON_H__

#pragma once

#include "stdafx.h"
#include <windows.h>
#include "eaptypes.h"
#include "eapmethodtypes.h"
#include <rtutils.h>

extern DWORD g_dwEapTraceId;

namespace SDK_METHOD_SAMPLE_COMMON
{

/// This EAP provider implements a single EAP protocol, with this Type Id.
const BYTE EAPTYPE = 40;

/// Following are the details of this EAP Provider. (Author, Vendor, Vendor Type and its Version)
const DWORD AUTHOR_ID = 311;
const DWORD VENODR_ID = 0;
const DWORD VENDOR_TYPE = 0;
const DWORD VERSION = 1;

/// Vendor-type: MPPE Send Key.
const BYTE MS_MPPE_SEND_KEY = 16;

/// Vendor-type: MPPE Receive Key.
const BYTE MS_MPPE_RECV_KEY = 17;

/// The byte length of the actual MPPE key = 32 bytes.
const BYTE MPPE_KEY_LENGTH = 32;

/// The Vendor-ID for the software vendor providing this EAP DLL.
const DWORD VENDOR_ID_MICROSOFT = 311;

/// The Vendor Attribute header is 6 bytes long.
const BYTE VENDOR_ATTRIBUTE_HEADER_LENGTH = 6;

/// The Salt field in front of MPPE Key values is 2 bytes long.
const BYTE MPPE_KEY_SALT_LENGTH = 2;

/// MPPE key block size = 16 bytes.  All MPPE keys should be a multiple of
/// this length.
const BYTE MPPE_KEY_BLOCK_LENGTH = 16;

/// Specify "no flags" for WideCharToMultiByte
const DWORD NO_FLAGS = 0;

/// Determine input string length automatically for WideCharToMultiByte
const long AUTOMATIC_STRING_LENGTH = -1;

/// Maximum Eap Packet Size
const DWORD MAX_PACKET_SIZE = 1500;

/// (???)
const DWORD MAX_ATTR_IN_EAP_ATTRIBUTES = 20;

/// Eap Packet Header Length
#define EAP_PACKET_HDR_LEN  ( sizeof( EapPacket ) - 1 )

/// Required for writing registry entries for registering Sdk Eap Methods with EapHost.
const wchar_t eapHostKeyName[] = L"System\\CurrentControlSet\\Services\\EapHost\\Methods";
const wchar_t eapMethodName[] = L"311\\40";
const wchar_t eapHostMethodKeyName[]  = L"System\\CurrentControlSet\\Services\\EapHost\\Methods\\311";
const wchar_t eapMethodId[] = L"40";


//
// Functions related to EapAttributes
//


/**
  * Allocate space for the EAPAttributes structure that will contain specified number of EAP attribute.
  *
  * @param  attribCount       The # of Eap attributes to create. 
  *                           
  * @param  ppEapAttributes  (input/output) A pointer to the real data buffer pointer.  
  *                                        The real pointer will be set to point to the allocated memory.
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD AllocateAttributes(
    IN     DWORD attribCount,
    IN OUT EapAttributes **ppEapAttributes
);

/**
  * Free a EapAttributes structure.
  *
  * This function will walk the chain of attributes, freeing each attribute's
  * Value data pointer, it will free the list's data buffer, and set the former
  * data buffer pointer to point to NULL.
  *
  * @param  ppEapAttributes  (input/output) A pointer to the real data buffer
  *                           pointer.
  *
  * @return A Win32 error code, indicating success or failure.
  *
  */
DWORD FreeAttributes(
    IN OUT EapAttributes **ppEapAttributes
);


/**
  * Set an attribute in a pre-allocated list.
  *
  * This function takes the attribute list buffer from AllocateAttributes(),
  * finds the first unset attribute, and sets it to contain the specified data.
  * It allocates memory for the attribute's value member.
  *
  * @param  pEapAttributes  (input/output) A pointer to the real data buffer pointer.  
  * @param  eaType            New attribute's type (from EapTypes.h).
  * @param  dwLength         New attribute's length.
  * @param  pValue             New attribute's value.
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD AddAttribute(
    IN OUT EapAttributes *pEapAttributes,
    IN     EapAttributeType eaType,
    IN     DWORD dwLength,
    IN     PVOID pValue
    );


/**
  * Append a new attribute onto the end of an existing list (extending list).
  *
  * This function creates a new, larger attribute list, copies attributes
  * from the original list into the new list, and adds the new attribute
  * onto the end of the list.
  *
  *
  * @param  ppAttributesList  (input/output) A pointer to the real attribute
  *                           list data buffer pointer.  The data will be
  *                           copied out of this buffer, the buffer freed, and
  *                           the pointer set to the new attribute list.
  * @param  eaType           New attribute's type (from EapTypes.h).
  * @param  dwLength        New attribute's length
  * @param  value              New attribute's value.
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD AppendAttributeToList(
    IN OUT EapAttributes      **ppAttributesList,
    IN     EapAttributeType   eaType,
    IN     DWORD                     dwLength,
    IN     PVOID                     value
);


/**
  * Report a message to the trace log.
  *
  * This function reports the specified message to a file in
  * \c %SystemRoot%\Tracing\[TraceFileName.log], where \c TraceFileName is the
  * string used in the TraceRegister() call.  This tracing
  * log can help when trying to debug issues on Eap Methods on clients & servers where
  * physical access is not possible, or where debuggers can't be run easily
  *
  *
  * @param  Format     [in]  An output string format specification, which can
  *                          include printf-style format values (ie, "%d").
  *
  * @param  ...        [in]  A number of optional arguments, corresponding to
  *                          the number of format specifiers in the Format
  *                          parameter.
  */

VOID   
EapTrace(
    IN  __in CHAR*   Format, 
    ... 
);

//
// Function related to EapError
//

/**
  * Allocates EAP_ERROR structure and fills it using the input parameters.
  *
  * The memory will be freed by EapPeerFreeErrorMemory() or EapMethodAuthenticatorFreeErrorMemory()
  * depending on who the caller of this API is.
  *
  * @param  pEapError                         EAP_ERROR will be allocated at this pointer and 
  *                                                      populated with above input parameters.
  * @param  errCode                           Win32 error code
  * @param  dwReasonCode                 Reason Code 
  * @param  pRootCauseGuid        	  Root Cause GUID
  * @param  pRepairGuid                      Repair GUID
  * @param  pHelpLinkGuid                   Help Link GUID
  * @param  pRootCauseString              String describing the root cause
  * @param  pRepairString                    String describing the repair mechanism  
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD AllocateandFillEapError(
   	IN OUT EAP_ERROR** pEapError,
	IN DWORD errCode,  
	IN DWORD dwReasonCode,
	IN LPCGUID pRootCauseGuid,
	IN LPCGUID pRepairGuid,
	IN LPCGUID pHelpLinkGuid,
	IN __in LPWSTR pRootCauseString,
	IN __in LPWSTR pRepairString
	);

DWORD CopyWideString(
	IN __in LPWSTR pSrcWString,
	OUT __out LPWSTR *pDestWString);

//
// Functions related to host-network and network-host.
//

/// Convert a 32-bit integer from host format to wire format.
VOID HostToWireFormat32(
	IN     DWORD dwHostFormat,
	IN OUT PBYTE pWireFormat
	);

/// Convert a 16-bit integer from host format to wire format.
VOID HostToWireFormat16(
	IN     WORD  wHostFormat,
	IN OUT PBYTE pWireFormat
	);

/// Convert a 16-bit integer from wire format to host format.
VOID WireToHostFormat16(
	IN     WORD  wWireFormat,
	IN OUT PBYTE pHostFormat
	);

/// Get the current working directory and appends dllName (input param) to it.
DWORD GetFullPath(
		OUT __out LPWSTR &pathName, 
		IN OUT DWORD &pathNameLength, 
		IN __in LPWSTR dllName, 
		IN DWORD sizeofDllNameInBytes);

}  // End "namespace SDK_METHOD_SAMPLE_COMMON
#endif  // __SDKMETHODCOMMON_H__
