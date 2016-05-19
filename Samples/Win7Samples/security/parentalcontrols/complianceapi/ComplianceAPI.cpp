
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    FILE: Windows Parental Controls (WPC) Compliance API sample

    PURPOSE: Demonstrates usage of COM Compliance API used by
              applications not needing to manage settings via
              WMI.

    FUNCTIONS:

        wmain() - implements overall command line application
        WpcsCmplApiUserIsLoggingRequired() - updates out param flag 
          specifying whether activity logging policy is on for a user
        WpcsCmplApiUserSettingsChangeTime() - sets out param time of 
          last change in policy settings for a user
        WpcsCmplApiUserGetRestrictions() - sets bitfields in out 
          param DWORD of active restriction silos for a user
        WpcsCmplApiGamesIsBlocked() - sets out param DWORD reason 
          code for any blocking of game specified by AppID GUID for 
          a user
        WpcsCmplApiWebGetSettings() - sets bitfields in out param 
          DWORD of active web restriction policies for a user
        WpcsCmplApiWebRequestUrlOverride() - specifying a URL and 
          optionally a set of associated subURLs, this method fires
          an administrator override request dialog
        CmdLineParse() - helper handling command line input


    COMMENTS:
        None of the Compliance API functions change state of policy
        settings for a user

****************************************************************************/


#include "ComplianceAPI.h"


HRESULT WpcsCmplApiUserIsLoggingRequired(IWindowsParentalControls* pWPC, PCWSTR pcszSID,
                                         BOOL* pfLoggingRequired);

HRESULT WpcsCmplApiUserSettingsChangeTime(IWindowsParentalControls* pWPC, PCWSTR pcszSID,
                                          SYSTEMTIME* pLastTime);

HRESULT WpcsCmplApiUserGetRestrictions(IWindowsParentalControls* pWPC, PCWSTR pcszSID,
                                       DWORD* pdwRestrictions);

HRESULT WpcsCmplApiGamesIsBlocked(IWindowsParentalControls* pWPC, PCWSTR pcszSID, 
                                  GUID guidAppID, DWORD* pdwBlockedReasons);

HRESULT WpcsCmplApiWebGetSettings(IWindowsParentalControls* pWPC, PCWSTR pcszSID,
                                  DWORD* pdwWebSettings);

HRESULT WpcsCmplApiWebRequestUrlOverride(IWindowsParentalControls* pWPC, 
                                         HWND hAppWindow, PCWSTR pcszSID, PCWSTR pcszUrl, 
                                         DWORD dwNumSubUrl, PCWSTR* ppcszSubUrl, 
                                         BOOL* pfChanged);


HRESULT CmdLineParse(int argc, WCHAR* argv[], PARSERESULT* pParseResult);

void Usage (PCWSTR pcszProgramName);

// call as Usage(argv[0])
void Usage (PCWSTR pcszProgramName)
{
    wprintf(L"Usage:  %s [<username>] <function> [additional args]\n", pcszProgramName);
    wprintf(L" Note:  If user name is not specified, report is for current user\n\n");
    wprintf(L" Where functions, and arguments are as follows:\n\n");
    wprintf(L"  user \t\t\t\tShow settings for user\n\n");
    wprintf(L"isgameblocked  <app ID GUID> \tShow if and why blocked for \n \
             \t\t\tspecified game\n\n");
    wprintf(L"requrloverride <url> [<subURL1> <subURL2> ..]\n");
    wprintf(L"\t\t\t\tRequest URL override, \n\t\t\t\tshowing changed status response\n\n");
}


// Application entry point
int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{
    // Declare command line parsing result structure
    PARSERESULT stParseResult;

    // Parse operational mode and arguments from command line.
    //  Function is responsible for printing its own errors.
    HRESULT hr = CmdLineParse(argc, argv, &stParseResult);
    if (hr == E_INVALIDARG)
    {
        // Printf usage and bypass further initialization
        Usage(argv[0]);
    }
    else if (SUCCEEDED(hr))
    {
        hr = WpcuCOMInit();
        if (FAILED(hr))
        {
            wprintf(L"Error:  Failed to initialize COM, hr is %8x.\n", hr);
        }
        else
        {
            // Obtain Compliance Interface
            IWindowsParentalControls* piWPC;
            hr = CoCreateInstance(__uuidof(WindowsParentalControls), 0, CLSCTX_INPROC_SERVER, 
                                  __uuidof(IWindowsParentalControls), (LPVOID *)&piWPC);
            if (FAILED(hr))
            {
                wprintf(L"Info:  Parental Controls interface not detected.\n");
                wprintf(L"Info:   This is an error if on a supported SKU.\n");
                hr = E_FAIL;
            }
            else
            {
                // Perform mode-specific operations
                switch (stParseResult.eOperation)
                {
                    case OPERATION_USER:
                    {
                        BOOL fLoggingRequired;
                        SYSTEMTIME LastTime;
                        DWORD dwUserRestrictions;
                        DWORD dwWebSettings;
                        hr = WpcsCmplApiUserIsLoggingRequired(piWPC, stParseResult.pszSID, 
                             &fLoggingRequired);
                        if (FAILED(hr))
                        {
                            wprintf(L"Error:  WpcsCmplApiUserIsLoggingRequired() failed, hr is %8x.\n", hr);
                        }
                        else
                        {
                            // Print results
                            hr = WpcsCmplApiUserSettingsChangeTime(piWPC, stParseResult.pszSID, 
                                &LastTime);
                            if (FAILED(hr))
                            {
                                wprintf(L"Error:  WpcsCmplApiUserSettingsChangeTime() failed, hr is %8x.\n", hr);
                            }
                            else
                            {
                                hr = WpcsCmplApiUserGetRestrictions(piWPC, stParseResult.pszSID, 
                                    &dwUserRestrictions);
                                if (FAILED(hr))
                                {
                                    wprintf(L"Error:  WpcsCmplApiUserGetRestrictions() failed, hr is %8x.\n", hr);
                                }
                                else
                                {
                                    hr = WpcsCmplApiWebGetSettings(piWPC, stParseResult.pszSID, &dwWebSettings);
                                    if (FAILED(hr))
                                    {
                                        wprintf(L"Error:  WpcsmplApiWebGetSettings() failed, hr is %8x.\n", hr);
                                    }
                                    else
                                    {
                                        wprintf(L"Info:  User IsLoggingRequired is %s\n", 
                                                (fLoggingRequired == TRUE) ? L"TRUE" : L"FALSE");

                                        // Display SYSTEMTIME info
                                        wprintf(L"Info:  Last settings change time is %4d/%02d/%02d %02d:%02d:%02d.%03d  UTC\n",
                                                 LastTime.wYear, LastTime.wMonth, LastTime.wDay, 
                                                 LastTime.wHour, LastTime.wMinute, LastTime.wSecond, 
                                                 LastTime.wMilliseconds);

                                        // Print web settings
                                        wprintf(L"Info:  User web settings:");
                                        if (dwWebSettings == WPCFLAG_WEB_SETTING_NOTBLOCKED)
                                        {
                                            wprintf(L"  No blocking\n");
                                        }
                                        else
                                        {
                                            // Leave room for future expansion
                                            wprintf(L"\t Download blocking is %s\n", 
                                                (dwWebSettings & WPCFLAG_WEB_SETTING_DOWNLOADSBLOCKED) ?
                                                L"on" : L"off");
                                        }
                                        
                                        // Print user restrictions fields
                                        wprintf(L"Info:  User restrictions:");
                                        if (dwUserRestrictions == WPCFLAG_NO_RESTRICTION)
                                        {
                                            wprintf(L"\t None\n");
                                        }
                                        else
                                        {
                                            wprintf(L"\n\t Logging required is %s\n", 
                                                (dwUserRestrictions & WPCFLAG_LOGGING_REQUIRED) ?
                                                L"on" : L"off");
                                            wprintf(L"\t Web restrictions are %s\n", 
                                                (dwUserRestrictions & WPCFLAG_WEB_FILTERED) ?
                                                L"on" : L"off");
                                            wprintf(L"\t Hours restrictions are %s\n", 
                                                (dwUserRestrictions & WPCFLAG_HOURS_RESTRICTED) ?
                                                L"on" : L"off");
                                            wprintf(L"\t Games restrictions are %s\n", 
                                                (dwUserRestrictions & WPCFLAG_HOURS_RESTRICTED) ?
                                                L"on" : L"off");
                                            wprintf(L"\t Application restrictions are %s\n", 
                                                (dwUserRestrictions & WPCFLAG_APPS_RESTRICTED) ?
                                                L"on" : L"off");

                                        }
                                    }
                                }
                            }
                        }
                    }
                    break;


                case OPERATION_ISGAMEBLOCKED:
                    {
                        DWORD dwBlockedReasons = 0;
                        hr = WpcsCmplApiGamesIsBlocked(piWPC, stParseResult.pszSID, 
                                                       stParseResult.guidAppID, &dwBlockedReasons);
                        if (FAILED(hr))
                        {
                            wprintf(L"Error:  WpcsCmplApiGamesIsBlocked() failed, hr is %8x.\n", hr);
                        }
                        else
                        {
                            // Print games IsBlocked reasons
                            wprintf(L"Info:  Games IsBlocked reason:\n");
                            if (dwBlockedReasons == WPCFLAG_ISBLOCKED_NOTBLOCKED)
                            {
                                wprintf(L"\tNot blocked\n");
                            }
                            else if (dwBlockedReasons == WPCFLAG_ISBLOCKED_INTERNALERROR)
                            {
                                wprintf(L"tInternal error\n");
                            }
                            else if (dwBlockedReasons & WPCFLAG_ISBLOCKED_EXPLICITBLOCK)
                            {
                                wprintf(L"\tExplicit block\n");
                            }
                            else if (dwBlockedReasons & WPCFLAG_ISBLOCKED_RATINGBLOCKED)
                            {
                                wprintf(L"\tRating block\n");
                            } 
                            else if (dwBlockedReasons & WPCFLAG_ISBLOCKED_DESCRIPTORBLOCKED)
                            {
                                wprintf(L"\tRating descriptor block\n");
                            }
                            else
                            {
                                wprintf(L"\tUnknown reason\n");
                            }
                        }
                    }
                    break;

                case OPERATION_REQUESTURLOVERRIDE:
                    {
                        BOOL fChanged = FALSE;
                        // Pass in handle of application main window.  For purposes of this console-based
                        //  sample, NULL is used
                        hr = WpcsCmplApiWebRequestUrlOverride(piWPC, 
                                            NULL,
                                            stParseResult.pszSID, 
                                            stParseResult.stRequestUrlOverride.pszUrl, 
                                            stParseResult.stRequestUrlOverride.dwNumSubUrl, 
                                            stParseResult.stRequestUrlOverride.ppcszSubUrl, 
                                            &fChanged);
                        if (FAILED(hr))
                        {
                            wprintf(L"Error:  WpcsCmplApiWebRequestUrlOverride() failed, hr is %8x.\n", hr);
                        }
                        else
                        {
                            wprintf(L"Info:  User web request URL override changed is %s.\n", 
                                fChanged ? L"TRUE" : L"FALSE");
                        }
                    }
                    break;
                }
                piWPC->Release();
            }    
            WpcuCOMCleanup();
        }
    }


    if (stParseResult.pszSID)
    {
        LocalFree(stParseResult.pszSID);
    }

    return (SUCCEEDED(hr)) ? 0 : 1;
}


HRESULT WpcsCmplApiUserIsLoggingRequired(IWindowsParentalControls* piWPC, PCWSTR pcszSID,
                                         BOOL* pfLoggingRequired)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation.  Allow NULL pcszSID to signal method to
    //  use current user SID
    if (piWPC && pfLoggingRequired)
    {
        // Obtain WpcUserSettings interface
        IWPCSettings* piWPCSettings = NULL;
        hr = piWPC->GetUserSettings(pcszSID, &piWPCSettings);
        if (SUCCEEDED(hr))
        {
            // Call method
            hr = piWPCSettings->IsLoggingRequired(pfLoggingRequired);
            piWPCSettings->Release();
        }
    }
    
    return (hr);
}

HRESULT WpcsCmplApiUserSettingsChangeTime(IWindowsParentalControls* piWPC, PCWSTR pcszSID,
                                          SYSTEMTIME* pLastTime)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation.  Allow NULL pcszSID to signal method to
    //  use current user SID
    if (piWPC && pLastTime)
    {
        // Obtain WpcUserSettings interface
        IWPCSettings* piWPCSettings = NULL;
        hr = piWPC->GetUserSettings(pcszSID, &piWPCSettings);
        if (SUCCEEDED(hr))
        {
            // Call method
            hr = piWPCSettings->GetLastSettingsChangeTime(pLastTime);
            piWPCSettings->Release();
        }
    }
    
    return (hr);
}

HRESULT WpcsCmplApiUserGetRestrictions(IWindowsParentalControls* piWPC, PCWSTR pcszSID,
                                       DWORD* pdwRestrictions)
{
    HRESULT hr = E_INVALIDARG;
    // Do basic parameter validation.  Allow NULL pcszSID to signal method to
    //  use current user SID
    if (piWPC && pdwRestrictions)
    {
        // Obtain WpcUserSettings interface
        IWPCSettings* piWPCSettings = NULL;
        hr = piWPC->GetUserSettings(pcszSID, &piWPCSettings);
        if (SUCCEEDED(hr))
        {
            // Call method
            hr = piWPCSettings->GetRestrictions(pdwRestrictions);
            piWPCSettings->Release();
        }
    }
    
    return (hr);
}

HRESULT WpcsCmplApiGamesIsBlocked(IWindowsParentalControls* piWPC, PCWSTR pcszSID,
                                  GUID guidAppID, DWORD* pdwBlockedReasons)
{
    HRESULT hr = E_INVALIDARG;
    // Do basic parameter validation.  Allow NULL pcszSID to signal method to
    //  use current user SID
    if (piWPC && pdwBlockedReasons)
    {
        // Obtain WpcGamesSettings interface
        IWPCGamesSettings* piWPCGamesSettings = NULL;
        hr = piWPC->GetGamesSettings(pcszSID, &piWPCGamesSettings);
        if (SUCCEEDED(hr))
        {
            // Call method
            hr = piWPCGamesSettings->IsBlocked(guidAppID, pdwBlockedReasons);
            piWPCGamesSettings->Release();
        }
    }
    
    return (hr);
}

HRESULT WpcsCmplApiWebGetSettings(IWindowsParentalControls* piWPC, PCWSTR pcszSID,
                                  DWORD* pdwWebSettings)
{
    HRESULT hr = E_INVALIDARG;
    // Do basic parameter validation.  Allow NULL pcszSID to signal method to
    //  use current user SID
    if (piWPC && pdwWebSettings)
    {
        // Obtain WpcWebSettings interface
        IWPCWebSettings* piWPCWebSettings = NULL;
        hr = piWPC->GetWebSettings(pcszSID, &piWPCWebSettings);
        if (SUCCEEDED(hr))
        {
            // Call method
            hr = piWPCWebSettings->GetSettings(pdwWebSettings);
            piWPCWebSettings->Release();
        }
    }
    
    return (hr);
}

HRESULT WpcsCmplApiWebRequestUrlOverride(IWindowsParentalControls* piWPC,
                                         HWND hAppWindow, PCWSTR pcszSID, PCWSTR pcszUrl, 
                                         DWORD dwNumSubUrl, PCWSTR* ppcszSubUrl, 
                                         BOOL* pfChanged)
{
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation.  Allow NULL pcszSID to signal method to
    //  use current user SID
    if (piWPC && pcszUrl && (!dwNumSubUrl || ppcszSubUrl) && pfChanged)
    {
        // Obtain WpcWebSettings interface
        IWPCWebSettings* piWPCWebSettings = NULL;
        hr = piWPC->GetWebSettings(pcszSID, &piWPCWebSettings);
        if (SUCCEEDED(hr))
        {
            // Call method
            hr = piWPCWebSettings->RequestURLOverride(hAppWindow, pcszUrl, dwNumSubUrl, 
                                                      ppcszSubUrl, pfChanged);
            piWPCWebSettings->Release();
        }
    }

    return (hr);
}

//
// Helper functions
//


// Parse the command line
HRESULT CmdLineParse(int argc, WCHAR* argv[], PARSERESULT* pParseResult)
{
    // Default is invalid until args are validated
    HRESULT hr = E_INVALIDARG;

    // Do basic parameter validation
    if (pParseResult && argc >= ARGS_MIN)
    {
        ZeroMemory(pParseResult, sizeof(PARSERESULT));
        BOOL fOperation = FALSE;
        PCWSTR pcszUserName = NULL;
        int i;
        
        // Determine operational mode and check prerequisites
        for (i = 1; !fOperation && i < 3 && i < argc; ++i)
        {
            if (_wcsicmp(argv[i], L"user") == 0)
            {
                pParseResult->eOperation = OPERATION_USER;
                fOperation = TRUE;
            }
            else if (_wcsicmp(argv[i], L"requrloverride") == 0)
            {
                pParseResult->eOperation = OPERATION_REQUESTURLOVERRIDE;
                fOperation = TRUE;
            }
            else if (_wcsicmp(argv[i], L"isgameblocked") == 0)
            {
                pParseResult->eOperation = OPERATION_ISGAMEBLOCKED;
                fOperation = TRUE;
            }
            if (fOperation && (i == 2))
            {
                //there is a username
                pcszUserName = argv[1];
            }
        }

        if (fOperation)
        {
            hr = S_OK;
            switch (pParseResult->eOperation)
            {
                case OPERATION_ISGAMEBLOCKED:
                    if (i == argc)
                    {
                        // No app ID
                        hr = E_INVALIDARG;
                    }
                    else
                    {
                        hr = CLSIDFromString(argv[i], &(pParseResult->guidAppID));
                    }
                    break;

                case OPERATION_REQUESTURLOVERRIDE:
                    if (i == argc)
                    {
                        // No URL
                        hr = E_INVALIDARG;
                    }
                    else
                    {
                    pParseResult->stRequestUrlOverride.dwNumSubUrl = argc - i -1;
                    pParseResult->stRequestUrlOverride.pszUrl = argv[i];
                    if (pParseResult->stRequestUrlOverride.dwNumSubUrl > 0)
                    {
                        pParseResult->stRequestUrlOverride.ppcszSubUrl = (PCWSTR*)(&(argv[i+1]));
                    }
                    }
                    break;
            }
        }

        if (SUCCEEDED(hr) && pcszUserName)
        {
            // Function allocates local memory for pszSID buffer
            hr = WpcuSidStringFromUserName(pcszUserName, &pParseResult->pszSID);
        }
    }
    return (hr);
}

