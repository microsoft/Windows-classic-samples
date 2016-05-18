//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © 2002  Microsoft Corporation.  All Rights Reserved.
//
// Abstract:
//    This code demonstrates how to build a customdial DLL for 
//    Windows 2000  and Windows XP RAS. The code shows how to export and provide a
//    rudimentary implementation for each of the exports required
//    by the DLL.
//
//    You must create an appropriate RAS Entry in the phonebook that inlcudes a path to this
//    custom DLL. See the CustomEntry sample on how to create a phonebook entry with a path to 
//    your custom DLL.
//
//    The DLL provides debug tracing facilities through the use of 
//    OutputDebugString. In this manner one can use programs that intercept
//    this output to view the DLL state while it is running such as DBMon
//    in the Platform SDK, an attached debugger, or other 3rd party 
//    programs.
//

//
// Includes
//

#ifdef WINVER
#undef WINVER
#endif


#define WINVER 0x0500   // needed for Windows 2000 RAS extensions



#ifndef UNICODE
#define UNICODE         // this is a Unicode DLL
#endif

#include <windows.h>    // Windows includes
#include <windowsx.h>   // Windows message crackers
#include <ras.h>        // Ras functions
#include <raserror.h>   // Ras error definitions
#include <rasdlg.h>     // Ras dialog functions
#include <stdio.h>      // Standard I/O functions
#include <strsafe.h>

#include "resource.h"   // resource file for the dialog templates

HANDLE g_hInstance;     // global handle to this DLL

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

// Strucuters for passing data between dialog procs
typedef struct _CUSTOM_ENTRY_DATA_SAMPLE_
{
    RASENTRYDLG tEntryDlg;
    LPRASENTRY  ptEntry;
    TCHAR       szEntryName[RAS_MaxEntryName + 1];
    TCHAR       szPhoneBookPath[MAX_PATH + 1];
} SAMPLE_CUSTOM_ENTRY_DATA, *PSAMPLE_CUSTOM_ENTRY_DATA;


typedef struct _CUSTOM_DIAL_DLG_SAMPLE_
{
    RASDIALDLG      tDialDlg;
    RASDIALPARAMS   tDialParams;
} SAMPLE_CUSTOM_DIAL_DLG, *PSAMPLE_CUSTOM_DIAL_DLG;


//
// OutputTraceString
//
// multiple argument front-end to OutputDebugString
//
void OutputTraceString(LPTSTR lpszFormatString, ...)
{
  const DWORD MAX_DEBUGSTR = 256;      // max string for ouput
  va_list arglist;                     // variable argument list
  TCHAR szOutputString[MAX_DEBUGSTR];  // the string to print - limited

  // create the new string limited to the char count as indicated
  va_start(arglist, lpszFormatString);
  StringCchVPrintf(szOutputString, CELEMS( szOutputString ), lpszFormatString, arglist);
  va_end(arglist);

  // send it out
  OutputDebugString(szOutputString);
}

//
// DllMain 
//
// this is the main entry point into the DLL and is called when
// the DLL is loaded
//
extern "C" BOOL APIENTRY DllMain( HINSTANCE hInstance, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{  
    // save instance handle for register/unregister functionality
    g_hInstance = hInstance;

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hInstance);
        OutputTraceString(L"CustomDial DLLMain (instance: 0x%08x) called - Process Attach\n", hInstance);
        break;
    case DLL_THREAD_ATTACH:
        OutputTraceString(L"CustomDial DLLMain (instance: 0x%08x) called - Thread Attach\n", hInstance);
        break;
    case DLL_THREAD_DETACH:
        OutputTraceString(L"CustomDial DLLMain (instance: 0x%08x) called - Thread Detach\n", hInstance);
        break;
    case DLL_PROCESS_DETACH:
        OutputTraceString(L"CustomDial DLLMain (instance: 0x%08x) called - Process Detach\n", hInstance);
        break;
    }

    return TRUE;
}

//
// RasCustomDial
//
// This export is the custom dial entry when RasDial has been called by
// an application - the call is forwarded to here and it is up to this
// function then to obtain a connection. This implementation simply
// forwards the request back to RasDial. Note that we must add the
// RDEOPT_CustomDial flag to the RasDialExtensions structure so we don't
// recursively get called.
//
extern "C" DWORD WINAPI RasCustomDial (
  HINSTANCE hInstDll,                      // handle to DLL instance
  LPRASDIALEXTENSIONS lpRasDialExtensions, // pointer to function extensions data
  LPCTSTR lpszPhonebook,                   // pointer to full path and file name of phone-book file 
  LPRASDIALPARAMS lpRasDialParams,         // pointer to calling parameters data
  DWORD dwNotifierType,                    // specifies type of RasDial event handler
  LPVOID lpvNotifier,                      // specifies a handler for RasDial events
  LPHRASCONN lphRasConn                    // pointer to variable to receive connection handle
)
{
    DWORD rc = ERROR_SUCCESS;             // return code from RasDial
    RASDIALEXTENSIONS RasDialExtensions;  // used as a copy of the RasDialExtensions structure

    OutputTraceString(L"RasCustomDial called in customdial.dll\n");
  
    if (NULL == lpRasDialParams)
    {
        return ERROR_UNKNOWN;
    }

    // set up the RasDialExtensions structure appropriately
    if (NULL == lpRasDialExtensions)
    {        
        ZeroMemory(&RasDialExtensions, sizeof(RASDIALEXTENSIONS));
        RasDialExtensions.dwSize = sizeof(RASDIALEXTENSIONS);
    } 
    else
    {
        CopyMemory(&RasDialExtensions, lpRasDialExtensions, sizeof(RASDIALEXTENSIONS));
    }


    RasDialExtensions.dwfOptions |= RDEOPT_CustomDial; // added cause we are calling RasDial

    OutputTraceString(L"--- RasCustomDial RasDialParams: E: '%s' UN: '%s' D: '%s'\n", 
        lpRasDialParams->szEntryName,
        lpRasDialParams->szUserName,
        lpRasDialParams->szDomain);

    // Simply call RasDial with the modified RasDialExtensions structure
    // and the original parameters passed in.
    // We are passing  lpvNotifier & lphRasConn directly into RasDial. Since we don't use these variable 
    // locally we skip checking for NULLs.
    rc = RasDial(&RasDialExtensions, lpszPhonebook, lpRasDialParams, dwNotifierType, lpvNotifier, lphRasConn);

    OutputTraceString(L"--- RasCustomDial returning handle 0x%08x\n", *lphRasConn);
    OutputTraceString(L"RasCustomDial exiting customdial.dll with return code: %d\n", rc);

    return rc;
}

//
// RasCustomHangUp
//
// This export is the custom dial entry when RasHangUp has been called by
// an application - the call is forwarded to here and it is up to this
// function then to close a connection. This implementation simply
// forwards the request back to RasHangUp.
//
extern "C" DWORD WINAPI RasCustomHangUp (
  HRASCONN hRasConn    // handle to a RAS connection
)
{
    DWORD rc = SUCCESS;     // return code


    OutputTraceString(L"RasCustomHangUp called in customdial.dll\n");  
    OutputTraceString(L"--- RasCustomHangUp on handle 0x%08x\n", hRasConn);

    // simply call RasHangUp on the handle
    rc = RasHangUp(hRasConn);

    OutputTraceString(L"RasCustomHangUp exiting customdial.dll with return code: %d\n", rc);

    return rc;
}

//
// CustomEntryDlgProc
//
// This function is the dialog procedure for the custom entry dialog and handles
// the basic events that happen within that dialog
//
INT_PTR CALLBACK CustomEntryDlgProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{  
    HWND hEditBox = NULL;                           // handle to an edit box
    HWND hComboBox = NULL;                          // handle to the combo box
    LPRASENTRYDLG lpInfo = NULL;                    // pointer to the RASENTRYDLG structure
    static LPRASENTRY lpRasEntry = NULL;            // pointer to the RASENTRY structure
    static PSAMPLE_CUSTOM_ENTRY_DATA pData = NULL;  // pointer to the SAMPLE_CUSTOM_ENTRY_DATA structure

    switch (uMsg)
    {
    case WM_INITDIALOG:    
        {
            // set up the dialog and the parameters
            pData = (PSAMPLE_CUSTOM_ENTRY_DATA) lParam;
            if (!pData) 
            {
                return FALSE;
            }

            lpInfo = &(pData->tEntryDlg);
            lpRasEntry = pData->ptEntry;                
          
            hComboBox = GetDlgItem(hwndDlg, IDC_LIST_MODEMS);

            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_ENTRYNAME);
            if (hEditBox)
            {
                Edit_LimitText(hEditBox, RAS_MaxEntryName); // Count doesn't include NULL
                Edit_SetText(hEditBox, (LPTSTR)pData->szEntryName);
            }

            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_PHONENO);
            if (hEditBox)
            {
                Edit_LimitText(hEditBox, CELEMS(lpRasEntry->szLocalPhoneNumber) - 1); // Count doesn't include NULL
                Edit_SetText(hEditBox, lpRasEntry->szLocalPhoneNumber);
            }
            
            //
            // enumerate to get the device name to use - we'll just do modems
            //
              
            // first get the size of the buffer required
            LPRASDEVINFO lpRasDevInfo = NULL;   // RASDEVINFO structure pointer
            DWORD dwNumEntries = 0;             // number of entries returned
            DWORD dwSize = sizeof(RASDEVINFO);  // size of buffer
            DWORD rc = 0;                       // return code


            // Allocate buffer with space for at least one structure
            lpRasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
            if (NULL == lpRasDevInfo)
            {
                return FALSE;
            }

            lpRasDevInfo->dwSize = sizeof(RASDEVINFO);

            rc = RasEnumDevices(lpRasDevInfo, &dwSize, &dwNumEntries);
            if (ERROR_BUFFER_TOO_SMALL == rc)
            {
                // If the buffer is too small, free the allocated memory and allocate a bigger buffer.
                if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDevInfo))
                {
                    lpRasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
                    if (NULL == lpRasDevInfo)
                    {
                        OutputTraceString(L"--- Error allocating memory (HeapAlloc) for RASDEVINFO structures for RasEnumDevices(): %d\n", GetLastError());
                        return FALSE;
                    }

                    lpRasDevInfo->dwSize = sizeof(RASDEVINFO);

                    rc = RasEnumDevices(lpRasDevInfo, &dwSize, &dwNumEntries);
                }
                else
                {
                    // Couldn't free the memory
                    return FALSE;
                }
            }

            // Check whether RasEnumDevices succeeded
            if (ERROR_SUCCESS == rc)
            {
                lpRasDevInfo = (LPRASDEVINFO) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
                if (NULL != lpRasDevInfo)
                {
                    lpRasDevInfo->dwSize = sizeof(RASDEVINFO);

                    rc = RasEnumDevices(lpRasDevInfo, &dwSize, &dwNumEntries);
                    if (ERROR_SUCCESS == rc)
                    {
                        for (UINT i = 0; i < dwNumEntries; i++, lpRasDevInfo++)
                        {        
                            if (lstrcmpi(lpRasDevInfo->szDeviceType, RASDT_Modem) == 0)
                            {
                                if (hComboBox)
                                {
                                    // add to the list
                                    ComboBox_AddString(hComboBox, lpRasDevInfo->szDeviceName);         
                                }
                            }          
                        }
                    }

                    HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasDevInfo);
                    lpRasDevInfo = NULL;
                }
                else
                {
                    OutputTraceString(L"--- Error allocating memory (HeapAlloc) for RASDEVINFO structures for RasEnumDevices(): %d\n", GetLastError());
                }
            }
        
            

            if (hComboBox)
            {
                // select the item we have in the entry
                ComboBox_SelectString(hComboBox, -1, lpRasEntry->szDeviceName);
            }

            // move dialog and position according to structure paramters
            // NOTE: we don't take into account multiple monitors or extreme
            // cases here as this is only a quick sample
            
            DWORD xPos = 0;         // x coordinate position for centering
            DWORD yPos = 0;         // y coordinate position for centering

            if (lpInfo->dwFlags & RASDDFLAG_PositionDlg)
            {
                xPos = lpInfo->xDlg;
                yPos = lpInfo->yDlg;
            }
            else
            {
                RECT rectTop;         // parent rectangle used for centering
                RECT rectDlg;         // dialog rectangle used for centering

                // center window within the owner or desktop
                GetWindowRect(lpInfo->hwndOwner != NULL ? lpInfo->hwndOwner : GetDesktopWindow(), &rectTop);
                GetWindowRect(hwndDlg, &rectDlg);
                         
                xPos = ((rectTop.left + rectTop.right) / 2) - ((rectDlg.right - rectDlg.left) / 2);
                yPos = ((rectTop.top + rectTop.bottom) / 2) - ((rectDlg.bottom - rectDlg.top) / 2);
            }

            SetWindowPos(hwndDlg, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);

            return TRUE;
        }
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:          
            {
            TCHAR szEntryName[RAS_MaxEntryName + 1] = {0};  // Entry name
            TCHAR szPhoneBook[MAX_PATH + 1] = {0};          // Phonebook path
            TCHAR szPhoneNumber[RAS_MaxPhoneNumber + 1] = {0};

            // copy back the phonenumber, entry name, & device name
            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_PHONENO);
            if (hEditBox)
            {
                Edit_GetText(hEditBox, lpRasEntry->szLocalPhoneNumber, CELEMS(lpRasEntry->szLocalPhoneNumber));
            }
          
            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_ENTRYNAME);          
            if (hEditBox)
            {
                Edit_GetText(hEditBox, szEntryName, CELEMS(szEntryName));
            }
              
            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_PHONENO);
            if (hEditBox)
            {
                Edit_GetText(hEditBox, szPhoneNumber, CELEMS(szPhoneNumber));
            }

            hComboBox = GetDlgItem(hwndDlg, IDC_LIST_MODEMS);          
            if (hComboBox)
            {
                ComboBox_GetLBText(hComboBox, ComboBox_GetCurSel(hComboBox), lpRasEntry->szDeviceName);
            }
            
            if (!pData) return FALSE;

            // Check entry name for validity
            StringCchCopy(szPhoneBook, CELEMS(szPhoneBook), (LPTSTR)pData->szPhoneBookPath);          
            if (RasValidateEntryName(szPhoneBook, szEntryName) == ERROR_INVALID_NAME)
            {
                MessageBox(hwndDlg, L"The Entry Name is Invalid.", L"Entry name error", MB_OK);
            }
            else          
            {
                StringCchCopy(pData->szEntryName, CELEMS(pData->szEntryName), szEntryName);
                StringCchCopy(lpRasEntry->szLocalPhoneNumber, CELEMS(lpRasEntry->szLocalPhoneNumber), szPhoneNumber);
                
                EndDialog(hwndDlg, TRUE);
            }
            
            return TRUE;
            }
        case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            return TRUE;
        }
        break;      
    }

    return FALSE;
}


//
// RasCustomEntryDlg
//
// this export is the custom dial entry when RasEntryDlg has been called by
// an application - the call is forwarded to here and it is up to this
// function then to display, handle and update the phonebook entry that is
// being modified through its own dialog or other means
//
extern "C" BOOL WINAPI  RasCustomEntryDlg (
  HINSTANCE hInstDll,       // handle to DLL instance
  LPTSTR lpszPhonebook,     // pointer to the full path and 
                            //  file name of the phone-book file
  LPTSTR lpszEntry,         // pointer to the name of the
                            //  phone-book entry to edit,
                            //  copy, or create
  LPRASENTRYDLG lpInfo      // pointer to a structure that
                            //  contains additional parameters
)
{
    DWORD rc = 0;           // return code
    DWORD dwSize = 0;       // size of buffer
    //RASENTRY RasEntry;      // Ras Entry structure
    PSAMPLE_CUSTOM_ENTRY_DATA pEntryData = NULL;    // pointer to allocated strucutre for passing data
    LPRASENTRY pLocalRE = NULL;
    LPRASENTRY pMemberRE = NULL;

    OutputTraceString(L"RasCustomEntryDlg called in customdial.dll\n");

    // Check input parameters
    if (!lpszEntry || !lpInfo)
    {
        return FALSE;
    }

    // create buffer of data to move back and forth between dialog
    dwSize = sizeof(SAMPLE_CUSTOM_ENTRY_DATA);

    pEntryData = (PSAMPLE_CUSTOM_ENTRY_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
    if (pEntryData)
    {
        CopyMemory(&(pEntryData->tEntryDlg), lpInfo, sizeof(RASENTRYDLG));

        dwSize = sizeof(RASENTRY);

        pLocalRE = (LPRASENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
        if (NULL == pLocalRE)
        {
            return FALSE;
        }

        // get the other parameters for the entry (phone and Device Name)
        
        pLocalRE->dwSize = dwSize;
        
        rc = RasGetEntryProperties(lpszPhonebook, lpszEntry, pLocalRE, &dwSize, NULL, NULL);
        if (ERROR_BUFFER_TOO_SMALL == rc)
        {
            if (HeapFree(GetProcessHeap(), 0, (LPVOID)pLocalRE))
            {
                pLocalRE = (LPRASENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);

                if (NULL == pLocalRE)
                {
                    return FALSE;
                }
            }
            else
            {
                return FALSE;
            }

            // get the other parameters for the entry (phone and Device Name)
            
            pLocalRE->dwSize = dwSize;
            rc = RasGetEntryProperties(lpszPhonebook, lpszEntry, pLocalRE, &dwSize, NULL, NULL);
        }


        pMemberRE = (LPRASENTRY)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
        if (NULL == pMemberRE)
        {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pMemberRE);
            return FALSE;
        }

        // Set size and save pointer into structure
        pMemberRE->dwSize = dwSize;
        pEntryData->ptEntry = pMemberRE;
        pMemberRE = NULL;

        if (ERROR_SUCCESS == rc)
        {
            CopyMemory(pEntryData->ptEntry, pLocalRE , pEntryData->ptEntry->dwSize);
        }
        else
        {
            MessageBox(NULL, L"Copy Memory failed", L"Error", MB_OK);
        }    
        
        StringCchCopy(pEntryData->szEntryName, CELEMS(pEntryData->szEntryName), lpszEntry);
        StringCchCopy(pEntryData->szPhoneBookPath, CELEMS(pEntryData->szPhoneBookPath), lpszPhonebook);

        OutputTraceString(L"--- Call Custom Entry Dialog\n");      
        
        INT_PTR ret = DialogBoxParam(hInstDll, MAKEINTRESOURCE(IDD_CUSTOMENTRYDLG), lpInfo->hwndOwner, CustomEntryDlgProc, (LPARAM) pEntryData);

        OutputTraceString(L"--- Return from Custom Entry Dialog: %d\n", rc);      

        if (ret > 0)   // dialog succeeded
        {      
            // copy entry name into lpInfo structure for return
            StringCchCopy(lpInfo->szEntry, CELEMS(lpInfo->szEntry), (LPTSTR) pEntryData->szEntryName);

            // check to see if anything has changed and change the entry
            LPRASENTRY lpRasEntry = pEntryData->ptEntry;

            if ((lstrcmpi(pLocalRE->szLocalPhoneNumber, lpRasEntry->szLocalPhoneNumber) != 0) ||
                (lstrcmpi(pLocalRE->szDeviceName, lpRasEntry->szDeviceName) != 0) ||
                (lstrcmpi(lpInfo->szEntry, lpszEntry) != 0))
            {        
                OutputTraceString(L"--- Calling RasSetEntryProperties to make changes\n");
                if (rc = RasSetEntryProperties(lpszPhonebook, lpInfo->szEntry, lpRasEntry, lpRasEntry->dwSize, NULL, 0) != SUCCESS)
                {
                    OutputTraceString(L"--- RasSetEntryProperties() failed: %d\n", rc);
                    lpInfo->dwError = rc;
                    rc = FALSE;
                }
                else 
                {
                    OutputTraceString(L"--- RasSetEntryProperties() succeeded.\n", rc);
                    rc = TRUE;
                }
            }
            else
            {
                rc = TRUE;      
            }
        }
        else
        {
            lpInfo->dwError = GetLastError();
            OutputTraceString(L"DialogBox Failed (or cancelled) in RasCustomEntryDlg (GetLastError = %d)\n", lpInfo->dwError);
            rc = FALSE;
        }

        if (pLocalRE)
        {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pLocalRE);
            pLocalRE = NULL;
        }

        if (pEntryData->ptEntry)
        {
            HeapFree(GetProcessHeap(), 0, (LPVOID)pEntryData->ptEntry);
            pEntryData->ptEntry = NULL;
        }

        HeapFree(GetProcessHeap(), 0, (LPVOID)pEntryData);
        pEntryData = NULL;

    }
    else
    {
        OutputTraceString(L"--- Memory allocation failed (HeapAlloc) in RasCustomEntryDlg (GetLastError = %d)\n", GetLastError());
    }
 
    OutputTraceString(L"RasCustomEntryDlg exiting customdial.dll with return code: %d\n", rc);

    return rc;
}

//
// CustomDialDlgProc
//
// this function is the dialog procedure for the custom dial dialog and handles
// the basic events that happen within that dialog
//
INT_PTR CALLBACK CustomDialDlgProc(
  HWND hwndDlg,  // handle to dialog box
  UINT uMsg,     // message
  WPARAM wParam, // first message parameter
  LPARAM lParam  // second message parameter
)
{  
    LPRASDIALDLG lpInfo = NULL;                     // pointer to RASDIALDLG structure
    static LPRASDIALPARAMS lpRasDialParams = NULL;  // pointer to RASDIALPARAMS structure
    static PSAMPLE_CUSTOM_DIAL_DLG pDialData = NULL;// pointer to SAMPLE_CUSTOM_DIAL_DLG structure
    HWND hEditBox = NULL;                           // handle to an edit box


    switch (uMsg)
    {
    case WM_INITDIALOG:            
        {  
            pDialData = (PSAMPLE_CUSTOM_DIAL_DLG)lParam;
            if (!pDialData) return FALSE;

            lpInfo = &(pDialData->tDialDlg);
            lpRasDialParams = &(pDialData->tDialParams);

            // set known items
            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_USERNAME);
            if (hEditBox)
            {
                Edit_LimitText(hEditBox, CELEMS(lpRasDialParams->szUserName) - 1); // Count doesn't include NULL
                Edit_SetText(hEditBox, lpRasDialParams->szUserName);
            }

            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_DOMAIN);
            if (hEditBox)
            {
                Edit_LimitText(hEditBox, CELEMS(lpRasDialParams->szDomain) - 1); // Count doesn't include NULL
                Edit_SetText(hEditBox, lpRasDialParams->szDomain);
            }
              
            // Move dialog and position according to structure
            // we don't take into account multiple monitors or extreme
            // cases here as this is only a quick sample
              
            DWORD xPos = 0;         // x coordinate position for centering
            DWORD yPos = 0;         // y coordinate position for centering

            if (lpInfo->dwFlags & RASDDFLAG_PositionDlg)
            {
                xPos = lpInfo->xDlg;
                yPos = lpInfo->yDlg;
            }
            else
            {
                RECT rectTop;         // parent rectangle used for centering
                RECT rectDlg;         // dialog rectangle used for centering

                // center window within the owner or desktop
                GetWindowRect(lpInfo->hwndOwner != NULL ? lpInfo->hwndOwner : GetDesktopWindow(), &rectTop);
                GetWindowRect(hwndDlg, &rectDlg);
                         
                xPos = ((rectTop.left + rectTop.right) / 2) - ((rectDlg.right - rectDlg.left) / 2);
                yPos = ((rectTop.top + rectTop.bottom) / 2) - ((rectDlg.bottom - rectDlg.top) / 2);
            }

            SetWindowPos(hwndDlg, NULL, xPos, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
      
            return TRUE;
        }
    case WM_COMMAND:
      switch (LOWORD(wParam))
        {
        case IDOK:
            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_USERNAME);
            if (hEditBox)
            {
                Edit_GetText(hEditBox, lpRasDialParams->szUserName, CELEMS(lpRasDialParams->szUserName));
            }
            
            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_PASSWORD);
            if (hEditBox)
            {
                Edit_GetText(hEditBox, lpRasDialParams->szPassword, CELEMS(lpRasDialParams->szPassword));
            }

            hEditBox = GetDlgItem(hwndDlg, IDC_EDIT_DOMAIN);
            if (hEditBox)
            {
                Edit_GetText(hEditBox, lpRasDialParams->szDomain, CELEMS(lpRasDialParams->szDomain));
            }

            EndDialog(hwndDlg, TRUE);
            return TRUE;
 
        case IDCANCEL:
            EndDialog(hwndDlg, FALSE);
            return TRUE;
        }
      break;      
    }

    return FALSE;
}

//
// RasCustomDialDlg
//
// This export is the custom dial entry when RasDialDlg has been called by
// an application - the call is forwarded to here and it is up to this
// function then to display, handle and dial the phonebook entry that is
// specified
//
extern "C" BOOL WINAPI  RasCustomDialDlg (
  HINSTANCE hInstDll,      // handle to DLL instance
  DWORD dwFlags,           // reserved
  LPTSTR lpszPhonebook,    // pointer to the full path and 
                           //  file name of the phone-book file
  LPTSTR lpszEntry,        // pointer to the name of the 
                           //  phone-book entry to dial
  LPTSTR lpszPhoneNumber,  // pointer to replacement phone 
                           //  number to dial
  LPRASDIALDLG lpInfo,     // pointer to a structure that 
                           //  contains additional parameters
  PVOID pvInfo
)
{  
    DWORD rc = 0;               // return code
    HRASCONN hRasConn = NULL;
    DWORD dwSize = 0;
    PSAMPLE_CUSTOM_DIAL_DLG pDialData = NULL;
    RASDIALPARAMS RasDialParams;

    OutputTraceString(L"RasCustomDialDlg called in customdial.dll\n");

    if (!lpInfo)
    {
        return FALSE;
    }

    // set up the RasDialParams structure with information passed in
    ZeroMemory(&RasDialParams, sizeof(RASDIALPARAMS));
    RasDialParams.dwSize = sizeof(RASDIALPARAMS);
    StringCchCopy(RasDialParams.szEntryName, CELEMS(RasDialParams.szEntryName), lpszEntry);
    
    if (lpszPhoneNumber != NULL)
    {
        StringCchCopy(RasDialParams.szPhoneNumber, CELEMS(RasDialParams.szPhoneNumber), lpszPhoneNumber);
    }

    // pull information from the Entry for the dialog
    if (lpszEntry != NULL)
    {
        BOOL bPassword;

        if (rc = RasGetEntryDialParams(lpszPhonebook, &RasDialParams, &bPassword) != SUCCESS)
        {
            OutputTraceString(L"--- Error retrieving Entry Dial Parameters: %d\n", rc);
        }
        else
        {
            OutputTraceString(L"--- RasGetEntryDialParams: UN: '%s' D: '%s'\n", 
                              RasDialParams.szUserName,
                              RasDialParams.szDomain);
        }
    }
  
    // display dialog - passing in structure of data that the dialog needs
    // so we can get the username, password and domain information

    // first build the buffer of data to pass
    dwSize = sizeof(SAMPLE_CUSTOM_DIAL_DLG);
    pDialData = (PSAMPLE_CUSTOM_DIAL_DLG) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
    if (pDialData != NULL)
    {
        CopyMemory(&(pDialData->tDialParams), &RasDialParams, sizeof(pDialData->tDialParams));
        CopyMemory(&(pDialData->tDialDlg), lpInfo, sizeof(pDialData->tDialDlg));
      
        INT_PTR ret = DialogBoxParam(hInstDll, MAKEINTRESOURCE(IDD_CUSTOMDIALDLG), lpInfo->hwndOwner, CustomDialDlgProc, (LPARAM) pDialData);
        if (ret > 0)   // dialog succeeded
        { 
            // copy the results back of dialog entry back into stack variable
            CopyMemory(&RasDialParams, &(pDialData->tDialParams), sizeof(RASDIALPARAMS));

            // call the RasCustomDial since we just need it to do the same thing anyway but 
            // one would probably call their own custom dialer here or forward to RasDial using
            // another window for connection status.
            rc = RasCustomDial(hInstDll, NULL, lpszPhonebook, &RasDialParams, 0, 0, &hRasConn);    
   
            if (rc != 0)
            {
                // hang-up on error
                RasCustomHangUp(hRasConn);
                lpInfo->dwError = rc;
                rc = 0;     // return 0 on failure and set error info
            }
            else
            {
                // set the Entry Dial Params since we succeeded the dial
                if (rc = RasSetEntryDialParams(lpszPhonebook, &RasDialParams, TRUE) != SUCCESS)
                {
                    OutputTraceString(L"--- Error setting Entry Dial Parameters: %d\n", rc);
                }

                rc = 1;     // return >0 on success for the entire call
            }
        }
        else
        {
            lpInfo->dwError = GetLastError();
            OutputTraceString(L"--- DialogBox Failed (or cancelled) in RasCustomDialDlg (GetLastError = %d)\n", lpInfo->dwError);      
            rc = 0;
        }

        // Clear out password from memory
        ZeroMemory(&(pDialData->tDialParams.szPassword), sizeof(pDialData->tDialParams.szPassword));

        HeapFree(GetProcessHeap(), 0, (LPVOID)pDialData);
        pDialData = NULL;
    }
    else
    {
        OutputTraceString(L"--- Memory allocation failed (HeapAlloc) in RasCustomDialDlg (GetLastError = %d)\n", GetLastError());
        rc = ERROR_OUTOFMEMORY;
    }
  
    // Clear out structures in order to clear up the password from memory
    ZeroMemory(&RasDialParams, sizeof(RASDIALPARAMS));

    OutputTraceString(L"RasCustomDialDlg exiting customdial.dll with return code: %d\n", rc);

    return rc;
}

//
// RasCustomDeleteEntryNotify
//
// this export is the custom dial entry when RasDeleteEntry has been called by
// an application - the call is forwarded to here and it is up to this
// function then to handle the phonebook entry that is specified
//
extern "C" DWORD WINAPI RasCustomDeleteEntryNotify(
  LPCTSTR lpszPhonebook,
  LPCTSTR lpszEntry,
  DWORD dwFlags
)
{
    DWORD rc = NO_ERROR;     // return code

    OutputTraceString(L"RasCustomDeleteEntryNotify called in customdial.dll\n");

    // just forward the request on - one could do more custom delete stuff here
    rc = RasDeleteEntry(lpszPhonebook, lpszEntry);

    OutputTraceString(L"RasCustomDeleteEntryNotify exiting customdial.dll with return code: %d\n");

    return rc;
}



// EOF: customdial.cpp

