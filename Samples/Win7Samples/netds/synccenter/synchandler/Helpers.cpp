//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Module Name:
//      Helpers.cpp
//
//  Abstract:
//      Implementation of helpers used to format resource strings.
//
//////////////////////////////////////////////////////////////////////////////

#include "Pch.h"

//----------------------------------------------------------------------------
//
//  Description:
//      Finds a localized resource and returns a pointer to the constant string.
//      This is adapted from a blog article published by Raymond Chen entitled
//      "The format of string resources."
//      http://blogs.msdn.com/oldnewthing/archive/2004/01/30/65013.aspx
//
//  Parameters:
//      hInstance       - Handle to the module that contains the resource.
//      uID             - ID of the format string  resource to be loaded.
//      nLangID         - Specifies the language of the resource.
//      ppcstrResource  - Pointer that recieves the resource string.
//      pcch            - Length of the resource string.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      S_FALSE         - The requested string resource was empty.
//      Other HRESULTs  - Error returned by LoadResource or FindResourceEx.
//
//----------------------------------------------------------------------------
HRESULT FindStringResourceEx(
    __in                        HINSTANCE         hInstance,
    __in                        UINT              uID,
    __in                        UINT              nLangID,
    __deref_out_ecount(*pcch)   const WCHAR     **ppcstrResource,
    __out                       size_t           *pcch)
{
    *ppcstrResource = NULL;
    *pcch = 0;

    // Convert the string ID into a bundle number
    // Strings listed in .rc files are grouped together in
    // bundles of 16, starting with bundle number 1.
    HRSRC hrsrc = FindResourceEx(
                        hInstance,
                        RT_STRING,
                        MAKEINTRESOURCE(uID / 16 + 1),
                        (WORD) nLangID);
    HRESULT hr = (hrsrc != NULL) ? S_OK : ResultFromKnownLastError();
    if (SUCCEEDED(hr))
    {
        // Load the resource bundle into memory.
        HGLOBAL hglob = LoadResource(hInstance, hrsrc);
        hr = (hglob != NULL) ? S_OK : ResultFromKnownLastError();
        if (SUCCEEDED(hr))
        {
            // Strings in the resource table are not null terminated.
            // Treat a failure to lock the resource as an empty string, since
            // LockResource() isn't documented to call SetLastError().
            const WCHAR *pcstr = NULL;
            pcstr = reinterpret_cast<const WCHAR *>(LockResource(hglob));
            hr = (pcstr != NULL) ? S_OK : S_FALSE;
            if (hr == S_OK)
            {
                // We have the string bundle we're looking for,
                // let's traverse it and find the specific
                // string entry we want.
                for (UINT iResource = 0; iResource < (uID & 15); iResource++)
                {
                    // The length of the string is stored as the
                    // first word in the entry.
                    pcstr += 1 + (UINT) *pcstr;
                }

                hr = S_FALSE;

                // Get the length of our resource string
                *pcch = (UINT) *pcstr;

                // If we have a zero length string, pcstr + 1
                // will point to the next string resource.
                // We should only reassign *ppcstrResource if
                // our length is greater than zero.
                if (*pcch > 0)
                {
                    hr = S_OK;
                    *ppcstrResource = pcstr + 1;
                }

                UnlockResource(pcstr);
            } // if: resource locked successfully

            FreeResource(hglob);
        } // if: resource loaded successfully
    }  // if: string resource found

    return hr;

} //*** FindStringResourceEx

//----------------------------------------------------------------------------
//
//  Description:
//      Finds a resource for the current language (for the current thread)
//      and returns a pointer to the contant string.
//
//  Parameters:
//      hInstance       - Handle to the module that contains the resource.
//      uID             - ID of the format string resource to be loaded.
//      ppcstrResource  - Pointer that recieves the resource string.
//      pcch            - Length of the resource string.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
HRESULT FindStringResource(
    __in                        HINSTANCE       hInstance,
    __in                        UINT            uID,
    __deref_out_ecount(*pcch)   const WCHAR   **ppcstrResource,
    __out                       size_t         *pcch)
{
    return FindStringResourceEx(hInstance, uID, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), ppcstrResource, pcch);

} //*** FindStringResource

//----------------------------------------------------------------------------
//
//  Description:
//      Finds a resource string and returns an allocated copy of the string.
//      The returned string should be freed using LocalFree().
//
//  Parameters:
//      hInstance       - Handle to the module that contains the resource.
//      uID             - ID of the string to be loaded.
//      ppwsz           - Pointer to the allocated sstring.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
HRESULT AllocStringFromResource(
    __in            HINSTANCE     hInstance,
    __in            UINT          uID,
    __deref_out     PWSTR        *ppwsz)
{
    *ppwsz = NULL;

    // Retrieve a pointer to the string resource we are looking for.
    // Resource strings are stored with their length as the first word
    // followed by the actual string value.
    const WCHAR *pcstrResource;
    size_t cch;
    HRESULT hr = FindStringResource(hInstance, uID, &pcstrResource, &cch);
    if (SUCCEEDED(hr))
    {
        // Get the length of the resource string.
        // Note that resource strings are not null terminated.
        PWSTR pwsz = (PWSTR) LocalAlloc(LMEM_ZEROINIT, (cch + 1) * sizeof(WCHAR));
        hr = (pwsz != NULL) ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            // Copy the resource value out.
            // If this resource points to an empty string (length zero),
            // cch will be zero, and we will fill the resulting string
            // with a single null terminator.
            hr = StringCchCopyNExW(pwsz, cch + 1, pcstrResource, cch, NULL, NULL, STRSAFE_IGNORE_NULLS);
            if (SUCCEEDED(hr))
            {
                *ppwsz = pwsz;
            }
        }
    } // if: found string resource

    return hr;

} //*** AllocStringFromResource

//----------------------------------------------------------------------------
//
//  Description:
//      Format a resource string with the given optional parameters.
//
//  Parameters:
//      hInstance
//          Handle to the module that contains the resource.
//
//      uID
//          ID of the format string resource to be loaded.
//
//      ppwszOut
//          Pointer to the string formatted from the string
//              resource and the parameters.  Caller must
//              deallocate using LocalFree().
//
//      ...
//          Optional parameters to format into the string.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
HRESULT FormatString(
    __in            HINSTANCE     hInstance,
    __in            UINT          uID,
    __deref_out     PWSTR        *ppwszOut,
    ...)
{
    *ppwszOut = NULL;

    va_list vaParamList;
    va_start(vaParamList, ppwszOut);
    HRESULT hr = FormatStringVA(hInstance, uID, ppwszOut, vaParamList);
    va_end(vaParamList);

    return hr;

} //*** FormatString

//----------------------------------------------------------------------------
//
//  Description:
//      Format a given string with the given optional parameters.
//
//  Parameters:
//      pwszFormat
//          Pointer to a string containing the format string
//
//      ppwszOut
//          Pointer which will receive the formatted string.  Caller must
//          deallocate with LocalFree().
//
//      ...
//          Optional parameters to format into the string.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
HRESULT FormatString(__in PCWSTR pwszFormat, __deref_out PWSTR *ppwszOut, ...)
{
    *ppwszOut = NULL;

    va_list vaParamList;
    va_start(vaParamList, ppwszOut);
    HRESULT hr = FormatStringVA(pwszFormat, ppwszOut, vaParamList);
    va_end(vaParamList);

    return hr;

}//*** FormatString

//----------------------------------------------------------------------------

HRESULT FormatStringVA(__in PCWSTR pwszFormat, __deref_out PWSTR *ppwszOut, __in va_list vaParamList)
{
    HRESULT hr = S_OK;
    *ppwszOut = NULL;

    if (FormatMessageW(
            (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING),
            pwszFormat,
            0,
            0,
            (PWSTR) ppwszOut,
            0,
            &vaParamList) == 0)
    {
        hr = ResultFromKnownLastError();
    }

    return hr;

}//*** FormatStringVA

//----------------------------------------------------------------------------
//
//  Description:
//      Format a resource string with the given va list.
//
//  Parameters:
//      hInstance
//          Handle to the module that contains the resource.
//
//      uID
//          ID of the format string resource to be loaded.
//
//      ppwszOut
//          Pointer to the formated resource string.  Caller
//              must deallocate using LocalFree().
//
//      vaParamList
//          Variable argument list.
//
//  Return Values:
//      S_OK            - Operation completed successfully.
//      E_OUTOFMEMORY   - Error allocating the object.
//      Other HRESULTs.
//
//----------------------------------------------------------------------------
HRESULT FormatStringVA(
    __in            HINSTANCE      hInstance,
    __in            UINT           uID,
    __deref_out     PWSTR         *ppwszOut,
    __in            va_list        vaParamList)
{
    *ppwszOut = NULL;

    // Retrieve the localized format string from the DLL.
    PWSTR pwszFormat;
    HRESULT hr = AllocStringFromResource(hInstance, uID, &pwszFormat);
    if (SUCCEEDED(hr))
    {
        if (FormatMessageW(
                (FORMAT_MESSAGE_ALLOCATE_BUFFER
                | FORMAT_MESSAGE_FROM_STRING),
                pwszFormat,
                0,
                0,
                (PWSTR) ppwszOut,
                0,
                &vaParamList) == 0)
        {
            hr = ResultFromKnownLastError();
        }

        LocalFree(pwszFormat);
    }

    return hr;

} //*** FormatStringVA

//----------------------------------------------------------------------------
//
//  Description:
//      Compare two Sync Center IDs, e.g. two sync handler IDs or two sync
//      item IDs.  It is expected that both IDs are for the same type of
//      object (e.g. sync handler or sync item).  This routine should be used
//      in all places the must compare for equality so that the correct
//      comparison routine is used everywhere.  Incorrect comparison routines
//      include those that factor code page or locale into the comparison.
//
//  Parameters:
//      pszFirstID      - First sync handler or sync item ID to compare.
//      pszSecondID     - Second sync handler or sync item ID to compare.
//
//  Return Values:
//      -1      - First ID sorts lexically before second ID.
//      0       - Both IDs are equivalent.
//      1       - First ID sorts lexically after second ID.
//
//----------------------------------------------------------------------------
int CompareSyncMgrID(__in PCWSTR pszFirstID, __in PCWSTR pszSecondID)
{
    int nCompareResult = CompareStringOrdinal(pszFirstID, -1 /*cchCount1*/, pszSecondID, -1 /*cchCount2*/, TRUE /*bIgnoreCase*/);

    int nResult;
    switch (nCompareResult)
    {
        case CSTR_EQUAL:
            nResult = 0;
            break;
        case CSTR_GREATER_THAN:
            nResult = 1;
            break;
        case CSTR_LESS_THAN:
            nResult = -1;
            break;
        default:
            // This should never happen.  If it does, consider them different.
            nResult = -1;
            break;
    }

    return nResult;

} //*** CompareSyncMgrID

