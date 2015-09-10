// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved


#include "SdkCommon.h"
#include <ObjBase.h>
#include "DebugHelper.h"
#include <Strsafe.h>
#include <NapUtil.h>
#include <new>
#include "Sddl.h"


namespace SDK_SAMPLE_COMMON
{

//
// Declarations of private methods (not accessible by external code).
//
    // Create a basic SoH Constructor object.
    HRESULT CreateSoHConstructor(
        _Outref_ INapSoHConstructor* &pISohConstructor);

    // Create a basic SoH Processor object.
    HRESULT CreateSoHProcessor(
        _Outref_ INapSoHProcessor* &pISohProcessor);


//
// Implementation of public methods.
//

// Helper Function for populating already allocated CountedString structures
// This makes filling the NapRegistrationInfo struct cleaner in the sample code
HRESULT FillCountedString(
    _In_ const WCHAR* src, 
    _Inout_ CountedString* dest)
{
    HRESULT hr = S_OK;
    CountedString *tCS = NULL;

    if ( (NULL == src) )
    {
        DebugPrintfW(L" --- SdkCommon - FillCountedString(): bad src pointer");
        hr = E_POINTER;
        goto Cleanup;
    }

    if ( (NULL == dest) )
    {
        DebugPrintfW(L" --- SdkCommon - FillCountedString(): bad dest pointer");
        hr = E_POINTER;
        goto Cleanup;
    }

    if ( (NULL != dest->string) ||
            (0 != dest->length) )
    {
        DebugPrintfW(L" --- SdkCommon - FillCountedString(): dest already has values set/allocated");
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    hr = AllocCountedString( &tCS, src );

    if ( FAILED(hr) )
    {
        // Failed to allocate memory
        DebugPrintfW(L" --- SdkCommon - FillCountedString: Failed to allocate buffer memory (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    *dest = *tCS;

    // free the struct, but not the embedded string buffer, as we are passing out a reference to it
    CoTaskMemFree(tCS);

Cleanup:
    return hr;
}


// Helper Function for depopulating CountedString structures
// leaves the structure intact, but frees the buffer underneath
// for use with FillCountedString above
HRESULT EmptyCountedString(
    _Inout_ CountedString * cs)
{
    HRESULT hr = S_OK;

    if ( NULL == cs)
    {
        DebugPrintfW(L" --- SdkCommon - EmptyCountedString(): bad pointer");
        hr = E_POINTER;
        goto Cleanup;
    }

    CoTaskMemFree(cs->string);
    cs->string = NULL;
    cs->length = 0;

Cleanup:
    return hr;

}


// Create an SoH Constructor object.
HRESULT CreateOutputSoHConstructor(
    _Outref_ INapSoHConstructor* &pISohConstructor,
    _In_ SystemHealthEntityId  systemHealthId,
    _In_ BOOL sohType)
{
    HRESULT hr = S_OK;

    //
    // Create an empty SoH Constructor for this client.
    //

	pISohConstructor = NULL;
	hr = CreateSoHConstructor(pISohConstructor);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateOutputSoHConstructor(): failed on call to CreateSoHConstructor (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    // Initialize the SoH Constructor, as specified by the caller.
    hr = pISohConstructor->Initialize(systemHealthId, sohType);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateOutputSoHConstructor(): failed on call to Initialize (error = 0x%08x)" ,hr);
        ReleaseObject(pISohConstructor);
    }

 Cleanup:
    return hr;
}


// Create an SoH Processor object.
HRESULT CreateInputSoHProcessor(
    _Outref_ INapSoHProcessor* &pISohProcessor,
    _Out_ SystemHealthEntityId &systemHealthId,
    _In_ BOOL sohType,
    _In_ SoH *pInputSoh)
{
    HRESULT hr = S_OK;

    //
    // Create the SoH processor that will be used for this input SoH.
    //

	pISohProcessor = NULL;
    hr = CreateSoHProcessor(pISohProcessor);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateInputSoHProcessor(): failed on call to CreateSoHProcessor (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    // Initialize the SoH Processor, as specified by the caller.
    hr = pISohProcessor->Initialize(pInputSoh, sohType, &systemHealthId);
    if ( FAILED(hr) )
    {
        DebugPrintfW(L" --- SdkCommon - CreateInputSoHProcessor(): failed on call to Initialize (error = 0x%08x)" ,hr);
        ReleaseObject(pISohProcessor);
    }

 Cleanup:
    return hr;
}


// Release a reference (pointer) to an IUnknown object.
void ReleaseObject(
    _Pre_opt_valid_ _Post_ptr_invalid_ IUnknown *pIUnknown)
{
    if (pIUnknown)
    {
        pIUnknown->Release();
    }
}


// Free a WCHAR string buffer. Upon exit, the input variable will be set to
// NULL.
void FreeMemory(
    _Pre_opt_valid_ _Post_ptr_invalid_ WCHAR* &pAllocatedMemory)
{	
	if (pAllocatedMemory)
	{
		CoTaskMemFree(pAllocatedMemory);
		pAllocatedMemory = NULL;
	}
}


// Free a CountedString struct, including the buffer pointed to by the
// internal string member. Upon exit, the input variable will be set to NULL.
void FreeMemory(
    _Pre_opt_valid_ _Post_ptr_invalid_ CountedString* &pAllocatedMemory)
{
	if (pAllocatedMemory)
	{
		FreeMemory(pAllocatedMemory->string);
		CoTaskMemFree(pAllocatedMemory);
		pAllocatedMemory = NULL;
	}
}


// Allocate a buffer to contain a WCHAR string, which is "stringSizeInBytes" wchars long.
HRESULT AllocateMemory(
    _Outref_result_bytebuffer_(stringSizeInBytes) WCHAR* &pString, 
    _In_ size_t stringSizeInBytes)
{
	HRESULT hr = S_OK;

	pString = static_cast<WCHAR*>(CoTaskMemAlloc(stringSizeInBytes));
	if (!(pString))
	{
		// Failed to allocate memory
		hr = E_OUTOFMEMORY;
		goto Cleanup;
	}

	ZeroMemory(pString, stringSizeInBytes);

Cleanup:
	return hr;
}


// Allocate a buffer to contain a CountedString struct, whose overall string member's
// buffer is "stringSizeInBytes" wchars long.
HRESULT AllocateMemory(
    _Outref_ CountedString* &pString, 
    _In_ size_t stringSizeInBytes)
{
	HRESULT hr = S_OK;

	pString = static_cast<CountedString*>(CoTaskMemAlloc(sizeof(CountedString)));
	if (!pString)
	{
		// Failed to allocate memory
		hr = E_OUTOFMEMORY;
		goto Cleanup;
	}

	ZeroMemory(pString, sizeof(CountedString));

    WCHAR* pBuffer;
	hr = AllocateMemory(pBuffer, stringSizeInBytes);
	if (FAILED(hr))
	{
		// delete the allocated memory
		FreeMemory(pString);
		goto Cleanup;
	}

	ZeroMemory(pBuffer, stringSizeInBytes);
    pString->string = pBuffer;

Cleanup:
	return hr;
}

//
// Implementation of private methods.
//

// Create a basic SoH Constructor object.
HRESULT CreateSoHConstructor(
    _Outref_ INapSoHConstructor* &pISohConstructor)
{
    HRESULT hr = S_OK;

    //
    // Construct a basic SoH Constructor object.
    //

	pISohConstructor = NULL;
	hr = CoCreateInstance(CLSID_NapSoHConstructor,
                           0,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(INapSoHConstructor),
                           reinterpret_cast<void**>(&pISohConstructor) );
    if (FAILED(hr))
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHConstructor(): failed on call to CoCreateInstance (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    if (! pISohConstructor)
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHConstructor(): CoCreate success, but pointer is bad.");
        hr = E_POINTER;
        goto Cleanup;
    }

 Cleanup:
    return hr;
}


// Create a basic SoH Processor object.
HRESULT CreateSoHProcessor(
    _Outref_ INapSoHProcessor* &pISohProcessor)
{
    HRESULT hr = S_OK;

    //
    // Construct a basic SoH object (QuarSoH interface).
    //

	pISohProcessor = NULL;
	hr = CoCreateInstance(CLSID_NapSoHProcessor,
                           0,
                           CLSCTX_INPROC_SERVER,
                           __uuidof(INapSoHProcessor),
                           reinterpret_cast<void**>(&pISohProcessor) );

    if (FAILED(hr))
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHProcessor(): failed on call to CoCreateInstance (error = 0x%08x)" ,hr);
        goto Cleanup;
    }

    if (! pISohProcessor)
    {
        DebugPrintfW(L" --- SdkCommon - CreateSoHProcessor(): CoCreate success, but pointer is bad.");
        hr = E_POINTER;
        goto Cleanup;
    }

 Cleanup:
    return hr;
}


// setting security on COM to allow communication to/from the
// NAPAgent service, which runs as NetworkService
HRESULT InitializeSecurity()
{
    DWORD errorCode = 0;
    HRESULT hr = S_OK;
    HANDLE hToken = NULL;
    PTOKEN_USER pTokenUser = NULL;
    PTOKEN_PRIMARY_GROUP pTokenGroup = NULL;
    PACL pDacl = NULL;

    // Initialize an absolute SECURITY_DESCRIPTOR structure.
    SECURITY_DESCRIPTOR sd;
    if (InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION) == FALSE)
    {
        errorCode = GetLastError();
        wprintf(L"InitializeSecurityDescriptor failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    // Open the access token associated with the calling process.
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE)
    {
        errorCode = GetLastError();
        wprintf(L"OpenProcessToken failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }
  
    // Retrieve the token information in a TOKEN_USER structure.
    DWORD bufferSize = 0;
    if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &bufferSize))
    {
        errorCode = GetLastError();
        if (errorCode != ERROR_INSUFFICIENT_BUFFER)
        {
            wprintf(L"GetTokenInformation failed for TokenUser. GetLastError returned: %lu\n", errorCode);
            hr = HRESULT_FROM_WIN32(errorCode);
            goto Cleanup;
        }
    }

    pTokenUser = reinterpret_cast<PTOKEN_USER>(new (std::nothrow) BYTE[bufferSize]);
    if (pTokenUser == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    memset(pTokenUser, 0, bufferSize);
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, bufferSize, &bufferSize))
    {
        errorCode = GetLastError();
        wprintf(L"GetTokenInformation failed for TokenUser. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    // Retrieve the token information in a TOKEN_PRIMARY_GROUP structure.
    bufferSize = 0;
    if (!GetTokenInformation(hToken, TokenPrimaryGroup, NULL, 0, &bufferSize))
    {
        errorCode = GetLastError();
        if (errorCode != ERROR_INSUFFICIENT_BUFFER)
        {
            wprintf(L"GetTokenInformation failed for TokenPrimaryGroup. GetLastError returned: %lu\n", errorCode);
            hr = HRESULT_FROM_WIN32(errorCode);
            goto Cleanup;
        }
    }

    pTokenGroup = reinterpret_cast<PTOKEN_PRIMARY_GROUP>(new (std::nothrow) BYTE[bufferSize]);
    if (pTokenGroup == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    memset(pTokenGroup, 0, bufferSize);
    if (!GetTokenInformation(hToken, TokenPrimaryGroup, pTokenGroup, bufferSize, &bufferSize))
    {
        errorCode = GetLastError();
        wprintf(L"GetTokenInformation failed for TokenPrimaryGroup. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    // Set user and group from token information
    if (!SetSecurityDescriptorOwner(&sd, pTokenUser->User.Sid, FALSE))
    {
        errorCode = GetLastError();
        wprintf(L"SetSecurityDescriptorOwner failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    if (!SetSecurityDescriptorGroup(&sd, pTokenGroup->PrimaryGroup, FALSE))
    {
        errorCode = GetLastError();
        wprintf(L"SetSecurityDescriptorGroup failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    // Create the well-known network service SID
    BYTE sid[SECURITY_MAX_SID_SIZE];
    DWORD sidSize = SECURITY_MAX_SID_SIZE;
    if (!CreateWellKnownSid(WinNetworkServiceSid, NULL, sid, &sidSize))
    {
        errorCode = GetLastError();
        wprintf(L"SetSecurityDescriptorGroup failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    // Calculate the amount of memory that must be allocated for the DACL.
    DWORD cbDacl = 
        sizeof(ACL) +                               // Size of memory required for the ACL structure
        sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD); // Size of ACE structure that the ACL is to contain minus the SidStart member (DWORD) of the ACE
    cbDacl += GetLengthSid(sid);                    // Length of the SID that ACE is to contain

    // Create and initialize an ACL.
    pDacl = reinterpret_cast<PACL>(new (std::nothrow) BYTE[cbDacl]);
    if (pDacl == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }
    memset(pDacl, 0, cbDacl);

    if (InitializeAcl(pDacl, cbDacl, ACL_REVISION) == FALSE)
    {
        errorCode = GetLastError();
        wprintf(L"InitializeAcl failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    if (AddAccessAllowedAce(pDacl,                     // Pointer to the ACL
                            ACL_REVISION2,             // Required constant
                            COM_RIGHTS_EXECUTE,        // Access mask
                            sid                        // Pointer to the network service SID
                            ) == FALSE)
    {
        errorCode = GetLastError();
        wprintf(L"AddAccessAllowedAce failed for the network service group. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    // Insert the DACL into the absolute SECURITY_DESCRIPTOR structure.
    if (SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE) == FALSE)
    {
        errorCode = GetLastError();
        wprintf(L"SetSecurityDescriptorDacl failed. GetLastError returned: %lu\n", errorCode);
        hr = HRESULT_FROM_WIN32(errorCode);
        goto Cleanup;
    }

    hr = CoInitializeSecurity( &sd,
                               -1,
                               NULL,
                               NULL,
                               RPC_C_AUTHN_LEVEL_PKT, 
                               RPC_C_IMP_LEVEL_IMPERSONATE,
                               NULL,
                               EOAC_NONE,
                               NULL );
    if (FAILED(hr))
    {
        wprintf(L"CoInitializeSecurity failed (error = %x)\n", hr);
    }

Cleanup:
    if (hToken != NULL)
    {
        CloseHandle(hToken);
    }
    if (pTokenUser != NULL)
    {
        delete [] pTokenUser;
    }
    if (pTokenGroup != NULL)
    {
        delete [] pTokenGroup;
    }
    if (pDacl != NULL)
    {
        delete [] pDacl;
    }
    return hr;
}


HRESULT CreateKeyPath(
    _In_reads_(cchDest) LPWSTR keyPath,
    _In_ size_t cchDest,
    _In_z_ const LPCWSTR pKey,
    _In_opt_z_ const LPCWSTR pSubKey1,
    _In_opt_z_ const LPCWSTR pSubKey2)
{
    HRESULT hr = StringCchCopyW(keyPath, cchDest, pKey);
    if (FAILED(hr))
    {
        goto Cleanup;
    }
    if (pSubKey1 != NULL)
    {
        hr = StringCchCatW(keyPath, cchDest, pSubKey1);
        if (FAILED(hr))
        {
            DebugPrintfW(L" --- SdkCommon - CreateKeyPath(): StringCchCatW failed with pSubKey1 (error = 0x%08x)" ,hr);
            goto Cleanup;
        }
    }
    if (pSubKey2 != NULL)
    {
        hr = StringCchCatW(keyPath, cchDest, pSubKey2);
        if (FAILED(hr))
        {
            DebugPrintfW(L" --- SdkCommon - CreateKeyPath(): StringCchCatW failed with pSubKey2 (error = 0x%08x)" ,hr);
            goto Cleanup;
        }
    }

Cleanup:
    return hr;
}


HRESULT SdkSetRegistryValue(
    _In_z_ const LPCWSTR pKey,
    _In_opt_z_ const LPCWSTR pValueName,
    _In_ DWORD type,
    _In_reads_bytes_(cbData) const void* pData,
    _In_ DWORD cbData)
{
    HRESULT hr = S_OK;

    HKEY hkey;
    hr = RegCreateKeyEx(HKEY_LOCAL_MACHINE, pKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hkey, NULL);
    if (hr != ERROR_SUCCESS)
    {
        DebugPrintfW(L" --- SdkCommon - SdkSetRegistryValue(): RegCreateKeyEx failed (error = 0x%08x)" ,hr);
        return hr;
    }

    hr = RegSetValueEx(hkey, pValueName, 0, type, static_cast<CONST BYTE*>(pData), cbData);
    if (hr != ERROR_SUCCESS)
    {
        DebugPrintfW(L" --- SdkCommon - SdkSetRegistryValue(): RegSetValueEx failed (error = 0x%08x)" ,hr);
        goto Cleanup;
    }
    
Cleanup:
    RegCloseKey(hkey);
    return hr;
}



HRESULT SdkSetRegistryStringValue(
    _In_z_ const LPCWSTR pKey,
    _In_opt_z_ const LPCWSTR pValueName,
    _In_z_ const LPCWSTR pData)
{
    DWORD cbData = (pData == NULL) ? 0 : static_cast<DWORD>((wcslen(pData)+1) * sizeof(WCHAR));
    HRESULT hr = SdkSetRegistryValue(
        pKey, 
        pValueName, 
        REG_SZ, 
        static_cast<const void*>(pData), 
        cbData);
    if (hr != ERROR_SUCCESS)
    {
        DebugPrintfW(L" --- SdkCommon - SdkSetRegistryStringValue(): SdkSetRegistryValue failed (error = 0x%08x)" ,hr);
    }

    return hr;
}



HRESULT DeleteRegistryTree(
    _In_z_ LPCWSTR baseKey,
    _In_z_ LPCWSTR keyName)
{
    HKEY hkey;
    DWORD hr = RegOpenKeyEx(HKEY_LOCAL_MACHINE, baseKey, NULL, KEY_ALL_ACCESS, &hkey);
    if(hr == ERROR_SUCCESS)
    {
        hr = RegDeleteTree(hkey, keyName);
        if (hr != ERROR_SUCCESS)
        {
            DebugPrintfW(L" --- SdkCommon - DeleteRegistryTree(): RegDeleteTree failed (error = 0x%08x)" ,hr);
        }

        RegCloseKey(hkey);
    }
    else
    {
        DebugPrintfW(L" --- SdkCommon - DeleteRegistryTree(): RegOpenKeyEx failed (error = 0x%08x)" ,hr);
    }

    return hr;
}

}  // End "namespace SDK_SAMPLE_COMMON".
