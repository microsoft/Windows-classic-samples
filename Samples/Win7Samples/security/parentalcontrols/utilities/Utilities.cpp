
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    FILE: Windows Parental Controls utility function implementation file

    PURPOSE: Common functions for COM, WMI, and generic support

    FUNCTIONS:

        WpcuCOMInit() - standard COM initialization
        WpcuCOMCleanup() - COM uninitialization
        WpcuWmiConnect() - connect to specified WMI namespace, returning services
         pointer
        WpcuSidStringFromUserName() - obtain SID for a given user name
        WpcuSidStringForCurrentUser() - obtain SID for calling process
        WpcuWmiInstancePutString() - put string value to class instance
        WpcuWmiInstancePutStringArray() - put string array to class instance
        WpcuWmiInstancePutDWORD() - put unsigned 32-bit value to class instance
        WpcuWmiInstancePutDWORDArray() - put unsigned 32-bit array to class instance
        WpcuWmiInstancePutBOOL() - put boolean value to class instance
        WpcuWmiStringFromInstance() - get string value from class instance
        WpcuWmiStringArrayFromInstance() - get string array from class instance
        WpcuWmiDWORDFromInstance() - get unsigned 32-bit value from class instance
        WpcuWmiDWORDArrayFromInstance() - get unsigned 32-bit array from class instance
        WpcuWmiBOOLFromInstance() - get boolean value from class instance

    COMMENTS:
        WMI writes consist of Put() calls to the instance, followed by a PutInstance()
        call to commit the changes.  WMI reads use the Get() call.

****************************************************************************/


#include "Utilities.h"


BOOL g_fCoInit = FALSE;


// Do one-time COM initalization per process
HRESULT WpcuCOMInit()
{
    HRESULT hr =  CoInitialize(NULL);
    if (SUCCEEDED(hr))
    {
        g_fCoInit = TRUE;

        // Set general COM security levels
        hr =  CoInitializeSecurity(
            NULL, 
            -1,                          // COM authentication
            NULL,                        // Authentication services
            NULL,                        // Reserved
            RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
            RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
            NULL,                        // Authentication info
            EOAC_NONE,                   // Additional capabilities 
            NULL                         // Reserved
            );
    }

    return hr;
}

// Uninitialize COM
void WpcuCOMCleanup()
{
    if (g_fCoInit)
    {
        CoUninitialize();
    }
}




// Calling function is responsible for freeing ppszSID using LocalAlloc
HRESULT WpcuSidStringFromUserName(PCWSTR pcszUserName, PWSTR* ppszSID)
{
    HRESULT hr = E_INVALIDARG;

    if (pcszUserName && ppszSID)
    {
        DWORD cbSID = 0, cchDomain = 0;
        SID_NAME_USE SidNameUse;

        // Call twice, first with null SID buffer.  Retrieves required buffer
        //  size for domain name
        LookupAccountNameW(NULL, pcszUserName, NULL, &cbSID, NULL, 
            &cchDomain, &SidNameUse);
        if (!cbSID || !cchDomain)
        {
            hr = E_FAIL;
        }
        else
        {
            WCHAR* pszDomain = NULL;
            // Allocate properly sized buffer (with termination character) 
            //  for domain name
            pszDomain = new WCHAR[cchDomain];
            if (!pszDomain)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                // Allocate properly sized buffer (with termination character)
                //  for PSID
                PSID pSID = static_cast<PSID> (new BYTE[cbSID]); //(LocalAlloc (LMEM_FIXED, cbSID));
                if (!pSID)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    // Second call with buffers allocated and sizes set
                    if (!LookupAccountName(NULL, pcszUserName, pSID, &cbSID,
                        pszDomain, &cchDomain, &SidNameUse))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                    else
                    {
                        // Convert PSID to SID string
                        if (!ConvertSidToStringSidW(pSID, ppszSID))
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                        else
                        {
                            hr = S_OK;
                        }
                    }
                    delete[] pSID; // (LocalFree (pSID);
                }
                delete[] pszDomain;
            }
        }
    }

    return (hr);
}

// Calling function is responsible for freeing ppszSID using LocalAlloc
HRESULT WpcuSidStringForCurrentUser(PWSTR* ppszSID)
{
    HRESULT hr = E_INVALIDARG;

    if (ppszSID)
    {
        HANDLE hToken = NULL;
        // Obtain token handle.  Try the thread token first
        if (!OpenThreadToken(GetCurrentThread(), TOKEN_READ, FALSE, &hToken))
        {
            DWORD dwErr = GetLastError();
            if (dwErr == ERROR_NO_TOKEN)
            {
                // Try the process token
                if (!OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &hToken))
                {
                    hr = E_FAIL;
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(dwErr);
            }
        }

        if (SUCCEEDED(hr))
        {
            TOKEN_USER* ptokenUser = NULL;
            DWORD cbToken = 0;
            DWORD cbSid = 0;
            // Call twice.  Call first time with NULL pTokenUser and 0 length to
            // retrive required buffer length.  Returns non-zero (normally failure)
            if (!GetTokenInformation (hToken, TokenUser, ptokenUser, 0, &cbToken))
            {
                ptokenUser = (TOKEN_USER *) LocalAlloc (LMEM_FIXED, cbToken);
                if(!ptokenUser)
                {
                    hr = E_OUTOFMEMORY;
                }
                else
                {
                    // Call second time with required buffer
                    if (!GetTokenInformation (hToken, TokenUser, ptokenUser, cbToken,
                        &cbToken))
                    {
                        hr = HRESULT_FROM_WIN32(GetLastError());
                    }
                    else
                    {
                        // Convert PSID to SID string
                        if (!ConvertSidToStringSidW(ptokenUser->User.Sid, ppszSID))
                        {
                            hr = HRESULT_FROM_WIN32(GetLastError());
                        }
                        else
                        {
                            hr = S_OK;
                        }
                    }
                    LocalFree(ptokenUser);
                }
            }
        }
        if (hToken)
        {
            CloseHandle(hToken);
        }
    }

    return (hr);
}

// Connect to specified namespace in WMI
HRESULT WpcuWmiConnect(PCWSTR pcszWpcNamespace, IWbemServices** ppiWmiServices)
{

    HRESULT hr = E_INVALIDARG;

    if (pcszWpcNamespace && ppiWmiServices)
    {
        // Obtain the initial locator to WMI
        IWbemLocator* pLocator = NULL;
        hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, 
            IID_IWbemLocator, (LPVOID *) &pLocator);
        // Set up to connect to the WPC namespace in WMI on the local machine
        if (SUCCEEDED(hr))
        {
            BSTR bstrWpcNamespace = SysAllocString(pcszWpcNamespace);

            if (bstrWpcNamespace == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                // Connect to the specified namespace with
                // the current user and obtain pointer
                // to make IWbemServices calls.
                hr = pLocator->ConnectServer(
                     bstrWpcNamespace,        // Object path of WMI namespace
                     NULL,                    // User name. NULL = current user
                     NULL,                    // User password. NULL = current
                     NULL,                    // Locale. NULL indicates current
                     NULL,                    // Security flags.
                     0,                       // Authority (e.g. Kerberos)
                     NULL,                    // Context object 
                     ppiWmiServices            // pointer to IWbemServices proxy
                     );

                SysFreeString(bstrWpcNamespace);
            }
            pLocator->Release();
        }
    }

    return(hr);
}


HRESULT WpcuWmiObjectGet(IWbemServices* piWmiServices, PCWSTR pcszObjectPath, 
                         IWbemClassObject** ppiWmiObject)
{
    HRESULT hr = E_INVALIDARG;
    if (piWmiServices && pcszObjectPath && ppiWmiObject)
    {
        BSTR bstrObjectPath = SysAllocString(pcszObjectPath);
        if (bstrObjectPath == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            hr = piWmiServices->GetObject(bstrObjectPath, WBEM_FLAG_RETURN_WBEM_COMPLETE, 
                NULL, ppiWmiObject, NULL);
            SysFreeString(bstrObjectPath);
        }
    }

    return (hr);
}


HRESULT WpcuWmiInstancePutString(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                 PCWSTR pcszValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && pcszValue)
    {
        VARIANT var;
        VariantInit(&var);
        var.bstrVal = SysAllocString(pcszValue);
        if (!(var.bstrVal))
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            var.vt = VT_BSTR;
            hr = piInstance->Put(pcszProperty, 0, &var, 0);
            VariantClear(&var);
        }
    }

    return hr;
}

// dwNumElements specifies the number of elements in the array referenced by
//  ppcszValue
HRESULT WpcuWmiInstancePutStringArray(IWbemClassObject* piInstance, 
                                      PCWSTR pcszProperty, 
                                      DWORD dwNumElements, 
                                      PCWSTR* ppcszValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && ppcszValue)
    {
        VARIANT var;
        VariantInit(&var);
        // Create new SAFEARRAY
        SAFEARRAYBOUND rgsaBound[1];
        rgsaBound[0].cElements = dwNumElements;
        rgsaBound[0].lLbound = 0;
        var.parray = SafeArrayCreate(VT_BSTR, 1, rgsaBound);
        if (var.parray == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Set proper variant type
            var.vt = VT_ARRAY | VT_BSTR;
            // Lock the array and obtain pointer to data
            BSTR* pData;
            hr = SafeArrayAccessData(var.parray, (void HUGEP**)&pData);
            if (SUCCEEDED(hr))
            {
                for (DWORD i = 0; 
                     i < dwNumElements && SUCCEEDED(hr); 
                     i++)
                {
                    // Write array data
                    pData[i] = SysAllocString(ppcszValue[i]);
                    if (pData[i] == NULL)
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }
                // Always unlock the array
                SafeArrayUnaccessData(var.parray);
                if (SUCCEEDED(hr))
                {
                    hr = piInstance->Put(pcszProperty, 0, &var, 0);
                }
            }
            VariantClear(&var);
        }
    }

    return hr;
}

HRESULT WpcuWmiInstancePutDWORD(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                DWORD dwValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty)
    {
        VARIANT var;
        VariantInit(&var);
        // CIM represents a uint32 type within a VT_I4 automation type, which is 
        // referenced as an lVal
        var.lVal = dwValue;
        var.vt = VT_I4;
        hr = piInstance->Put(pcszProperty, 0, &var, 0);
        VariantClear(&var);
    }

    return hr;
}

// dwNumElements specifies the number of elements in the array referenced by
//  pdwValue
HRESULT WpcuWmiInstancePutDWORDArray(IWbemClassObject* piInstance, 
                                     PCWSTR pcszProperty, 
                                     DWORD dwNumElements, 
                                     DWORD* pdwValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && pdwValue)
    {
        VARIANT var;
        VariantInit(&var);
        // Create new SAFEARRAY
        SAFEARRAYBOUND rgsaBound[1];
        rgsaBound[0].cElements = dwNumElements;
        rgsaBound[0].lLbound = 0;
        var.parray = SafeArrayCreate(VT_I4, 1, rgsaBound);
        if (var.parray == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            // Set proper variant type
            var.vt = VT_ARRAY | VT_I4;
            // Lock the array and obtain pointer to data
            DWORD* pData;
            hr = SafeArrayAccessData(var.parray, (void HUGEP**)&pData);
            if (SUCCEEDED(hr))
            {
                for (DWORD i = 0; i < dwNumElements; i++)
                {
                    // Write array data
                    pData[i] = pdwValue[i];
                }
                // Always unlock the array
                SafeArrayUnaccessData(var.parray);
                if (SUCCEEDED(hr))
                {
                    hr = piInstance->Put(pcszProperty, 0, &var, 0);
                }
            }
            VariantClear(&var);
        }
    }

    return hr;
}

HRESULT WpcuWmiInstancePutBOOL(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                BOOL fValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty)
    {
        VARIANT var;
        VariantInit(&var);
        var.boolVal = (fValue) ? VARIANT_TRUE : VARIANT_FALSE;
        var.vt = VT_BOOL;
        hr = piInstance->Put(pcszProperty, 0, &var, 0);
        VariantClear(&var);
    }

    return hr;
}

HRESULT WpcuWmiInstancePutNULLVariant(IWbemClassObject* piInstance, PCWSTR pcszProperty)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty)
    {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_NULL;
        hr = piInstance->Put(pcszProperty, 0, &var, 0);
        VariantClear(&var);
    }

    return hr;
}

// Function returns S_OK and *ppszValue = NULL if retrieved string property is
//  actually a NULL variant
HRESULT WpcuWmiStringFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                  PWSTR* ppszValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && ppszValue)
    {
        *ppszValue = NULL;
        VARIANT var;
        VariantInit(&var);
        hr = piInstance->Get(pcszProperty, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            // Only allow BSTR and NULL states
            if (var.vt != VT_BSTR)
            {  
                if (var.vt != VT_NULL)
                {
                    hr = E_FAIL;
                }
				else
				{
					hr = S_OK;
				}
            }
            else
            {
                size_t cch;
                hr = StringCchLengthW(var.bstrVal, STRSAFE_MAX_CCH, &cch);
                if (SUCCEEDED(hr))
                {
					// Allocate with null terminator
					*ppszValue = new WCHAR[++cch];
					if (!(*ppszValue))
					{
						hr = E_OUTOFMEMORY;
					}
					else
					{
						hr = StringCchCopyW(*ppszValue, cch, var.bstrVal);
					}
                }
            }
        }
        VariantClear(&var);
    }

    return hr;
}

// Function returns S_OK and *pppszValue = NULL if retrieved string array property 
//  is actually a NULL variant
HRESULT WpcuWmiStringArrayFromInstance(IWbemClassObject* piInstance, 
                                       PCWSTR pcszProperty, 
                                       DWORD* pdwNumElements, 
                                       PWSTR** pppszValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && pdwNumElements && pppszValue)
    {
        *pppszValue = NULL;
        *pdwNumElements = 0;

        VARIANT var;
        VariantInit(&var);
        hr = piInstance->Get(pcszProperty, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            // Only allow BSTR array and NULL states
            if (var.vt != (VT_BSTR | VT_ARRAY))
            {
                if (var.vt != VT_NULL)
                {
                    hr = E_FAIL;
                }
				else
				{
					hr = S_OK;
				}
            }
            else
            {
                // Transfer array data from variant
                long lUpper, lLower;
                hr = SafeArrayGetUBound(var.parray, 1, &lUpper);
                if (SUCCEEDED(hr))
                {
                    hr = SafeArrayGetLBound(var.parray, 1, &lLower);
                    if (SUCCEEDED(hr))
                    {
                        DWORD dwNumElements = lUpper - lLower + 1;
                        if (lUpper - lLower + 1 < lUpper - lLower)
                        {
                            // Overflow
                            hr = E_FAIL;
                        }
                        else if (dwNumElements > 0)
                        {
                            BSTR* pData;
                            hr = SafeArrayAccessData(var.parray, 
                                                     (void HUGEP**)(&pData));
                            if (SUCCEEDED(hr))
                            {
                                // Allocate results array
                                *pppszValue = new PWSTR[dwNumElements];
                                if (*pppszValue == NULL)
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                                else
                                {
                                    size_t cch;
                                    for (DWORD i = 0; 
                                         i < dwNumElements && SUCCEEDED(hr); 
                                         i++)
                                    {
                                        hr = StringCchLengthW(pData[lLower + i], 
                                                              STRSAFE_MAX_CCH, 
                                                              &cch);
                                        if (SUCCEEDED(hr))
                                        {
                                            (*pppszValue)[i] = new WCHAR[++cch];
                                            if (!(*pppszValue)[i])
                                            {
                                                hr = E_OUTOFMEMORY;
                                            }
                                            else
                                            {
                                                hr = StringCchCopyW((*pppszValue)[i], 
                                                                    cch, 
                                                                    pData[lLower + i]);
                                                if (SUCCEEDED(hr))
                                                {
                                                    // Successful addition
                                                    (*pdwNumElements)++;
                                                }
                                                else
                                                {
                                                    delete[] (*pppszValue)[i];
                                                    (*pppszValue)[i] = NULL;
                                                }
                                            }
                                        }
                                    }
                                }
                                // Always unlock the array
                                SafeArrayUnaccessData(var.parray);
                            }
                        }
                    }
                }
            }
            VariantClear(&var);
        }
    }

    return hr;
}


HRESULT WpcuWmiDWORDFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                 DWORD* pdwValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && pdwValue)
    {
        VARIANT var;
        VariantInit(&var);
        hr = piInstance->Get(pcszProperty, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            // Only allow DWORD
            if (var.vt != VT_I4)
            {
                hr = E_FAIL;
            }
            else
            {
                // CIM represents a uint32 type within a VT_I4 automation 
                //  type, which is referenced as an lVal
                *pdwValue = var.lVal;
            }
        }
        VariantClear(&var);
    }

    return hr;
}

// Function returns S_OK and *ppdwValue = NULL if retrieved DWORD array property 
// is actually a NULL variant
HRESULT WpcuWmiDWORDArrayFromInstance(IWbemClassObject* piInstance, 
                                      PCWSTR pcszProperty, 
                                      DWORD* pdwNumElements, 
                                      DWORD** ppdwValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && pdwNumElements && ppdwValue)
    {
        *ppdwValue = NULL;
        *pdwNumElements = 0;

        VARIANT var;
        VariantInit(&var);
        hr = piInstance->Get(pcszProperty, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            // Only allow DWORD array and NULL states
            if (var.vt != (VT_I4 | VT_ARRAY))
            {
                if (var.vt != VT_NULL)
                {
                    hr = E_FAIL;
                }
				else
				{
					hr = S_OK;
				}
            }
            else
            {
                // Transfer array data from variant
                long lUpper = 0, lLower = 0;
                hr = SafeArrayGetUBound(var.parray, 1, &lUpper);
                if (SUCCEEDED(hr))
                {
                    hr = SafeArrayGetLBound(var.parray, 1, &lLower);
                    if (SUCCEEDED(hr))
                    {
                        *pdwNumElements = lUpper - lLower + 1;
                        if (lUpper - lLower + 1 < lUpper - lLower)
                        {
                            // Overflow
                            hr = E_FAIL;
                        }
                        else if (*pdwNumElements > 0)
                        {
                            long* pData;
                            hr = SafeArrayAccessData(var.parray, 
                                                     (void HUGEP**)(&pData));
                            if (SUCCEEDED(hr))
                            {
                                *ppdwValue = new DWORD[*pdwNumElements];
                                if (!*ppdwValue)
                                {
                                    hr = E_OUTOFMEMORY;
                                }
                                else
                                {
                                    for (DWORD i = 0; i < *pdwNumElements; i++)
                                    {
                                        (*ppdwValue)[i] = pData[lLower + i];
                                    }
                                }
                                // Always unlock the array
                                SafeArrayUnaccessData(var.parray);
                            }
                        }
                    }
                }
            }
            VariantClear(&var);
        }
    }

    return hr;
}

HRESULT WpcuWmiBOOLFromInstance(IWbemClassObject* piInstance, PCWSTR pcszProperty, 
                                 BOOL* pfValue)
{
    HRESULT hr = E_INVALIDARG;
    if (piInstance && pcszProperty && pfValue)
    {
        VARIANT var;
        VariantInit(&var);
        hr = piInstance->Get(pcszProperty, 0, &var, NULL, NULL);
        if (SUCCEEDED(hr))
        {
            // Only allow BOOL
            if (var.vt != VT_BOOL)
            {
                hr = E_FAIL;
            }
            else
            {
                *pfValue = var.boolVal;
            }
        }
        VariantClear(&var);
    }

    return hr;
}

