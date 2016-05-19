//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  util.cpp
//
//  Purpose:
//  Implementation of auxiliary functions used by the sample.
//
//----------------------------------------------------------------------------



#include <windows.h>
#include <crtdbg.h>
#include <strsafe.h>

#include "util.h"
#include "main.h"



HRESULT DisplayErrorMessage(LPCWSTR pwszAdditionalMsg, HRESULT hrCode)
{
    HRESULT hr                    = S_OK;
    HRESULT LookupHr              = HRESULT_FROM_WIN32( ERROR_RESOURCE_LANG_NOT_FOUND );
    WCHAR   wszErrorMessage[512]  = { 0 };
    WCHAR  *pwszBITSErrorMsg      = NULL;

    if (FAILED(hrCode))
    {

        //
        // Prepare to get a locale-specific error message.
        // We we are going to try different LCIDs; depending if this is a localized OS or not,
        // we may be able to get localized error messages. Otherwise fall back to English strings. 
        //
        LCID LcidsToTry[] =
        {
            GetThreadLocale(),
            GetUserDefaultLCID(),
            GetSystemDefaultLCID(),
            MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), 0 )
        };

        SIZE_T NumberOfLCIDs = ARRAYSIZE(LcidsToTry);

        //
        // Loop until we run out of LCIDs or until we succeed.
        // Note that if the error is not BITS specific, the function will search
        // winhttp and the system for a description. 
        //
        for (SIZE_T i = 0;
                (( HRESULT_FROM_WIN32( ERROR_RESOURCE_LANG_NOT_FOUND ) == LookupHr || 
                   HRESULT_FROM_WIN32( ERROR_MR_MID_NOT_FOUND ) == LookupHr) 
                && i < NumberOfLCIDs);
                i++)
        {
            LookupHr = g_JobManager->GetErrorDescription(hrCode, LcidsToTry[i], &pwszBITSErrorMsg);
            if (SUCCEEDED(LookupHr))
            {
                break;
            }
        }

        //
        // Build the full message that we are going to display to the user.
        // Note that maybe we didn't get an error description from BITS after all, 
        // or the pwszAdditionalMsg parameter given was NULL. We need to test for those.
        //
        hr = StringCbPrintfW(
            wszErrorMessage, 
            sizeof(wszErrorMessage), 
            L"%s\n\nThe specific error message returned by the system is: [Error 0x%8.8x] %s\nPlease check the BITS SDK or post a question to the BITS newsgroup if you need \nassistance troubleshooting this issue.", 
            (pwszAdditionalMsg? pwszAdditionalMsg : L""),
            hrCode,
            (pwszBITSErrorMsg? pwszBITSErrorMsg : L"")
        );

        if (FAILED(hr))
        {
            goto cleanup;
        }

        MessageBoxW(NULL, wszErrorMessage, L"BITS Upload Sample", MB_OK | MB_ICONERROR | MB_SYSTEMMODAL | MB_SETFOREGROUND );
    }

cleanup:

    return hr;
}


LPCWSTR ConvertGuidToString(REFGUID guid)
{
    static WCHAR wszGuid[MAX_GUID_STRING_LENGTH];

    if (!StringFromGUID2(guid, wszGuid, MAX_GUID_STRING_LENGTH))
    {
        wszGuid[0] = L'\0';
    }

    return reinterpret_cast<WCHAR *>(wszGuid);
}
