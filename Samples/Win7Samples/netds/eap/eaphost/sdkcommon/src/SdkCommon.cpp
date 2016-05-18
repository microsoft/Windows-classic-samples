// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#include "SdkCommon.h"
#include "memory.h"
#include "strsafe.h"

namespace SDK_METHOD_SAMPLE_COMMON
{

/**
  * Allocate space for the EAPAttributes structure that will contain specified number of EAP attributes.
  *
  * @param  attribCount       The # of Eap attributes to create. 
  *                           
  * @param  ppEapAttributes  (input/output) A pointer to the real data buffer pointer.  
  *                                        The real pointer will be set to point to the allocated memory.
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD
AllocateAttributes(
    IN     DWORD attribCount,
    IN OUT EapAttributes **ppEapAttributes
)
{
	DWORD retCode  = NO_ERROR;
	DWORD bufLen = 0;

	// Sanity Check
	if (! ppEapAttributes)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	if(attribCount > MAX_ATTR_IN_EAP_ATTRIBUTES)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Allocate Memory for the EapAttributes structure.
	//
	retCode = AllocateMemory(sizeof(EapAttributes), (PVOID*)ppEapAttributes);
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// Assign the number of EAP attributes that the EapAttributes structure will contain.
	//
	(*ppEapAttributes)->dwNumberOfAttributes = attribCount;
	
	//
	// Allocate Memory for the EapAttribute pointer in the EapAttributes structure.
	//
	bufLen = sizeof(EapAttribute) * attribCount;
	retCode = AllocateMemory(bufLen, (PVOID*)&((*ppEapAttributes)->pAttribs));
	if (retCode != NO_ERROR)
	{
	    goto Cleanup;
	}

	// AllocateMemory() initializes the buffer to NULL values.

Cleanup:
	if(retCode != NO_ERROR)
		FreeMemory((PVOID*)ppEapAttributes);
	return retCode;
}



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
DWORD
FreeAttributes(
    IN OUT EapAttributes **ppEapAttributes
)
{
	DWORD retCode = NO_ERROR;
	DWORD attribCount = 0;
	EapAttribute *pEapAttrib = NULL;
	DWORD i = 0;

	//Sanity Check
	if (! ppEapAttributes)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	if (! *ppEapAttributes)
	{
		// Return silently. No attribute. Nothing to free.
		goto Cleanup;
	}

	attribCount = (*ppEapAttributes)->dwNumberOfAttributes;

	//
	// Free all the Value pointers in each EAP attribute.
	//
	for(i = 0; i < attribCount; i++)
	{
		pEapAttrib = &(((*ppEapAttributes)->pAttribs)[i]);
		retCode = FreeMemory((PVOID*)&(pEapAttrib->pValue));
		if (retCode != NO_ERROR)
			goto Cleanup;
	}

	//
	// Free the (EapAttribute *) in the EapAttributes structure.
	//
	retCode = FreeMemory((PVOID *)&((*ppEapAttributes)->pAttribs));
	if (retCode != NO_ERROR)
		goto Cleanup;

	//
	// Free the EapAttributes structure.
	//
	retCode = FreeMemory((PVOID*)ppEapAttributes);
	if (retCode != NO_ERROR)
		goto Cleanup;

	// FreeMemory sets the buffer pointer to NULL after freeing the data.

Cleanup:
	return retCode;
}


/**
  * Set an attribute in a pre-allocated list.
  *
  * This function takes the attribute list buffer from AllocateAttributes(),
  * finds the first unset attribute, and sets it to contain the specified data.
  * It allocates memory for the attribute's value member.
  *
  * @param  pEapAttributes  (input/output) A pointer to the real data buffer pointer.  
  * @param  eaType            New attribute's type (from eaptypes.h).
  * @param  dwLength         New attribute's length.
  * @param  pValue             New attribute's value.
  *
  * @return A Win32 error code, indicating success or failure.
  */
DWORD
AddAttribute(
    IN OUT EapAttributes *pEapAttributes,
    IN     EapAttributeType eaType,
    IN     DWORD dwLength,
    IN     PVOID pValue
    )
{
	DWORD retCode = NO_ERROR;
	DWORD i = 0;
	EapAttribute *pEapAttrib = NULL;

	//Sanity Check
	if (! pEapAttributes)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Find the next available (empty) attribute. 
	//
	for (i = 0; i < pEapAttributes->dwNumberOfAttributes; i++)
	{
		pEapAttrib = &((pEapAttributes->pAttribs)[i]);
		if (pEapAttrib->eaType == eatMinimum)
			break;
	}

	//
	// Array bounds check -- make sure we have room for this element.
	//
	if (i == pEapAttributes->dwNumberOfAttributes)
	{
		retCode = ERROR_OUT_OF_STRUCTURES;
		goto Cleanup;
	}

	//
	// Fill up the attribute structure.
	//
	pEapAttrib->eaType  = eaType;
	pEapAttrib->dwLength = dwLength;

	retCode = AllocateMemory(dwLength, (PVOID*)&(pEapAttrib->pValue));
	if (retCode != NO_ERROR)
	{
		goto Cleanup;
	}

	//
	// Copy the actual data length into the buffer.
	//
	CopyMemory(pEapAttrib->pValue, pValue, dwLength);

Cleanup:
	return retCode;
}


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
DWORD
AppendAttributeToList(
    IN OUT EapAttributes      **ppAttributesList,
    IN     EapAttributeType   eaType,
    IN     DWORD                     dwLength,
    IN     PVOID                     value
)
{
	DWORD retCode = NO_ERROR;
	DWORD dwCount = 0;
	DWORD attribCountOld = 0;
	EapAttribute *pEapAttrib = NULL;
	DWORD dwIndex = 0;

	EapAttributes *pNewAttributesList = NULL;

	// Sanity check.
	if (! ppAttributesList)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Determine how many attributes we'll need in the new list.
	//
	if (! *ppAttributesList)
	{
		attribCountOld = 0;
	}
	else
		attribCountOld = (*ppAttributesList)->dwNumberOfAttributes;
	
	dwCount = 1 + attribCountOld;

	//
	// Allocate a new attributes list, with the specified length.
	//
	retCode = AllocateAttributes(dwCount, &pNewAttributesList);
	if (retCode != NO_ERROR)
		goto Cleanup;

	//
	// Copy the old attributes list into the new list.
	//
	if (*ppAttributesList != NULL)
	{
		for( dwIndex = 0; dwIndex < attribCountOld; dwIndex++ )
		{
			pEapAttrib = &((*ppAttributesList)->pAttribs)[dwIndex];
			retCode = AddAttribute(pNewAttributesList,
			                     pEapAttrib->eaType,
			                     pEapAttrib->dwLength,
			                     pEapAttrib->pValue);
			if (retCode != NO_ERROR)
				goto Cleanup;
		}
	}

	//
	// Add the new attribute into the new list.
	//
	retCode = AddAttribute(pNewAttributesList, eaType, dwLength, value);
	if (retCode != NO_ERROR)
		goto Cleanup;

	//
	// Free the old list, and pass back the new list.
	//
	retCode = FreeAttributes(ppAttributesList);
	if (retCode != NO_ERROR)
		goto Cleanup;

	*ppAttributesList = pNewAttributesList;


Cleanup:
	if (retCode != NO_ERROR)
		FreeAttributes(&pNewAttributesList);

	return retCode;
}



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
  * @param  pRootCauseGuid               Root Cause GUID
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
	)
{
	DWORD retCode = NO_ERROR;

	//Sanity Check
	if(!pEapError)
	{
		retCode = ERROR_INVALID_PARAMETER;
		goto Cleanup;
	}

	//
	// Allocate memory for EAP_ERROR.
	//
	retCode = AllocateMemory(sizeof(EAP_ERROR), (PVOID *)pEapError);
	if(retCode != NO_ERROR)
		goto Cleanup;

	ZeroMemory(*pEapError, sizeof(EAP_ERROR));

	//
	// Assign the Win32 Error Code
	//
	(*pEapError)->dwWinError = errCode;

	//
	// Assign the EAP_METHOD_TYPE to indicate which EAP Method send the error.
	//
	(*pEapError)->type.eapType.type = EAPTYPE;
	(*pEapError)->type.eapType.dwVendorId = VENODR_ID;
	(*pEapError)->type.eapType.dwVendorType = VENDOR_TYPE;
	(*pEapError)->type.dwAuthorId = AUTHOR_ID;

	//
	// Assign the reason code
	//
	(*pEapError)->dwReasonCode = dwReasonCode;

	//
	// Assign the RootCause GUID
	//
	if(pRootCauseGuid != NULL)
		memcpy(&((*pEapError)->rootCauseGuid), pRootCauseGuid, sizeof(GUID));

	//
	// Assign the Repair GUID
	//
	if(pRepairGuid != NULL)
		memcpy(&((*pEapError)->repairGuid), pRepairGuid, sizeof(GUID));

	//
	// Assign the HelpLink GUID
	//
	if(pHelpLinkGuid!= NULL)
		memcpy(&((*pEapError)->helpLinkGuid), pHelpLinkGuid, sizeof(GUID));
	
	//
	// Assign the Root Cause String
	//
	retCode = CopyWideString(pRootCauseString, &((*pEapError)->pRootCauseString));
	if(retCode != NO_ERROR)
		goto Cleanup;

	//
	// Assign the Repair String
	//
	retCode = CopyWideString(pRepairString, &((*pEapError)->pRepairString));
	if(retCode != NO_ERROR)
		goto Cleanup;

Cleanup:
	if(retCode != NO_ERROR)
		FreeMemory((PVOID*)pEapError);
	
	return retCode;
}

DWORD CopyWideString(
	IN __in LPWSTR pSrcWString,
	OUT __out LPWSTR *pDestWString)
{
	DWORD retCode = NO_ERROR;
	HRESULT hr = S_OK;
	DWORD dwStringLength = 0;
	LPWSTR pTempString = NULL;

	// Nothing to copy
	if(!pSrcWString)
		goto Cleanup;

	hr = StringCbLengthW(pSrcWString, (size_t)(STRSAFE_MAX_CCH * sizeof(wchar_t)), (size_t *)&dwStringLength);
	if (FAILED(hr))
	{
		retCode = HRESULT_CODE(hr);
		goto Cleanup;
	}
	
	// StringCbLengthW - returns the length of string in bytes (excluding the null character).
	// StringCbCopyW expects the length of string in bytes (including the null character).
	dwStringLength += sizeof(wchar_t);
	retCode = AllocateMemory(dwStringLength, (PVOID *)&pTempString);
	if(retCode != NO_ERROR)
	{
		goto Cleanup;
	}
	
	hr = StringCbCopyW((LPTSTR)pTempString, (size_t)dwStringLength, pSrcWString);
	if (FAILED(hr))
	{
		retCode = HRESULT_CODE(hr);
		goto Cleanup;
	}

	//
	// Set the OUT parameter
	//
	*pDestWString = pTempString;

Cleanup:
	if(retCode != NO_ERROR)
		FreeMemory((PVOID *)&pTempString);
	pTempString = NULL;
	return retCode;
}


/**
  * Convert a 32-bit integer from host format to wire format.
  *
  * This function reorders the bytes in a given 32-bit value, into
  * from host-byte order into network-byte order.
  *
  * The calling function is responsible for allocating memory for the output
  * value.  This can come from an explicit allocation, or from an implicit
  * allocation such as a reference to a local variable.
  *
  *
  * @param  dwHostFormat   [in]  The input value, in host-byte order. 
  * @param  pWireFormat    [out] The output value, in network-byte order. 
  */

VOID
HostToWireFormat32(
    IN     DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(dwHostFormat) >> 24);
    *((PBYTE)(pWireFormat)+1) = (BYTE) ((DWORD)(dwHostFormat) >> 16);
    *((PBYTE)(pWireFormat)+2) = (BYTE) ((DWORD)(dwHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+3) = (BYTE) (dwHostFormat);
}


/**
  * Convert a 16-bit integer from host format to wire format.
  *
  * This function reorders the bytes in a given 16-bit value, into
  * from host-byte order into network-byte order.
  *
  * The calling function is responsible for allocating memory for the output
  * value.  This can come from an explicit allocation, or from an implicit
  * allocation such as a reference to a local variable.
  *
  *
  * @param  wHostFormat   [in]  The input value, in host-byte order. 
  * @param  pWireFormat    [out] The output value, in network-byte order. 
  */

VOID
HostToWireFormat16(
    IN     WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
)
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+1) = (BYTE) (wHostFormat);
}


/**
  * Convert a 16-bit integer from wire format to host format.
  *
  * This function reorders the bytes in a given 16-bit value, into
  * from network-byte order into host-byte order.
  *
  * The calling function is responsible for allocating memory for the output
  * value.  This can come from an explicit allocation, or from an implicit
  * allocation such as a reference to a local variable.
  *
  *
  * @param  wWireFormat   [in]  The input value, in network-byte order. 
  * @param  pHostFormat   [out] The output value, in host-byte order. 
  */

VOID
WireToHostFormat16(
    IN     WORD  wWireFormat,
    IN OUT PBYTE pHostFormat
)
{
    *pHostFormat = ((*((PBYTE)(&wWireFormat)+0) << 8) +
                    (*((PBYTE)(&wWireFormat)+1)));
}


/// Get the current working directory and appends dllName (input param) to it.
DWORD GetFullPath(
	OUT __out LPWSTR &pathName, 
	IN OUT DWORD &pathNameLength, 
	IN __in LPWSTR dllName, 
	IN DWORD sizeofDllNameInBytes
)
{
	DWORD retCode = ERROR_SUCCESS;
	DWORD bufferLength = 0;

	// Get the length that needs to be allocated for getting the current directory.
	bufferLength = GetCurrentDirectory(0, NULL);
	if(!bufferLength)
	{
		retCode = GetLastError();
		goto Cleanup;
	}

	// Buffer Length is the number of Wide Characters including the NULL character.
	// We need to calculate the total number of bytes to be allocated.
	bufferLength = (bufferLength - 1) * sizeof(WCHAR);
	bufferLength += sizeof(WCHAR);  // For the intermediate '\'
	bufferLength += sizeofDllNameInBytes;

	pathNameLength = bufferLength;

	retCode = AllocateMemory(bufferLength, (PVOID *)&pathName);
	if(retCode != ERROR_SUCCESS)
		goto Cleanup;

	// Actually get the directory path.
	bufferLength = GetCurrentDirectory(pathNameLength, pathName);
	if(bufferLength == 0)
		goto Cleanup;

	// Append "\"
	CopyMemory(pathName + bufferLength, L"\\", sizeof(WCHAR));

	// Append DllName.
	CopyMemory(pathName + bufferLength + 1, dllName, sizeofDllNameInBytes);

Cleanup:
	if((retCode != ERROR_SUCCESS) || (bufferLength == 0))
		FreeMemory((PVOID *)&pathName);

	return retCode;
}

/**
  * Report a message to the trace log.
  *
  * This function reports the specified message to a file in
  * \c %SystemRoot%\Tracing\[TraceFileName.log], where \c TraceFileName is the
  * string used in the TraceRegister() call.  This tracing
  * log can help when trying to debug issues on Eap Methods on clients & servers where
  * physical access is not possible, or where debuggers can't be run easily
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
) 
{
	va_list arglist;

	va_start(arglist, Format);

	/// The default level assigned to each trace message.
	DWORD g_DefaultTraceLevel = 0x00010000;

	TraceVprintfExA(g_dwEapTraceId, 
	    g_DefaultTraceLevel | TRACE_USE_MASK | TRACE_USE_MSEC,
	    Format,
	    arglist);

	va_end(arglist);
}

}  // End "namespace SDK_METHOD_SAMPLE_COMMON
