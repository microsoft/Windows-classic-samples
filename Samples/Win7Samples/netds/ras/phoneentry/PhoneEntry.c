//
//      This is a part of the Microsoft Source Code Samples.
//      Copyright 1993 - 2002 Microsoft Corporation.
//      All rights reserved.
//      This source code is only intended as a supplement to
//      Microsoft Development Tools and/or WinHelp documentation.
//      See these sources for detailed information regarding the
//      Microsoft samples programs.
//
//      APIs used in this sample:
//		    RasEnumDevices()
//		    RasGetCountryInfo()
//		    RasValidateEntryName()
//		    RasSetEntryProperties()
//		    RasEnumEntries()
//		    RasGetEntryProperties()
//

#define STRICT
#include <windows.h>
#include <windowsx.h>
#include <ras.h>
#include <raserror.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "resource.h"
#include <strsafe.h>
#include <winsock.h>
 
#define MAX_BUF_SIZE 256
#define NUMBER_OF_PROPERTY_SHEETS   3

HINSTANCE g_hInstance = NULL;
char g_szEntryName[256] = {0};
LPRASENTRY g_lpRasEntry = NULL;
LPRASDEVINFO g_RasDevInfo = NULL;
BOOL g_fFlag = FALSE;
BOOL g_fValidEntry = FALSE;
DWORD g_dwEntries = 0;
int g_iTemp = 0;

// Macro for counting maximum characters that will fit into a buffer
#define CELEMS(x) ((sizeof(x))/(sizeof(x[0])))

// Property sheet function prototypes
BOOL CALLBACK PropSheet(HWND hWnd);
INT_PTR CALLBACK Page0Proc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Page1Proc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Page2Proc(HWND, UINT, WPARAM, LPARAM);

// Dialog box function prototype
INT_PTR CALLBACK Dlg_Proc1(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK Dlg_Proc2(HWND, UINT, WPARAM, LPARAM);

DWORD
StringToIpAddr(
    LPCSTR pszIpAddr,
    OUT RASIPADDR *pipaddr
    )
{
    DWORD   dwErr;
    CHAR    szIpAddr[17] = { 0 };
    PULONG  pul = (PULONG)pipaddr;


    if('\0' != szIpAddr[0])
    {
        *pul = inet_addr(szIpAddr);
    }
    else
    {
        *pul = 0;
    }

    return 0;
}

//
//  FUNCTION: Dlg_Proc1(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Dialog Procedure for manipulating the TCP/IP settings for a
//			  phonebook entry.
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//			Returns 0.
//
//  COMMENTS:
//		This function handles the dialog box for the TCP/IP settings.
//		The dialog box allows the user to assign an IP address or use a server
//		assigned one. It also allows the user to specify the various name server
//		addresses. Finally, the user can also specify to use IP header compression
//		and to use a default gateway on a remote network.
//
//		The following fields of the RASENTRY structure are modified
//
//		g_lpRasEntry->ipaddr
//		g_lpRasEntry->ipaddrDns
//		g_lpRasEntry->ipaddrDnsAlt
//		g_lpRasEntry->ipaddrWins
//		g_lpRasEntry->ipaddrWinsAlt
//
//		The following flags in the g_lpRasEntry->dwfOptions are modified
//
//		RASEO_SpecificIpAddr
//		RASEO_SpecificNameServers
//		RASEO_IpHeaderCompression
//		RASEO_RemoteDefaultGateway
//

INT_PTR CALLBACK Dlg_Proc1(HWND hWnd, UINT uMsgID, WPARAM wParam, LPARAM lParam)
{
    static HWND hRadio1 = NULL;
    static HWND hRadio2 = NULL;
    static HWND hRadio3 = NULL;
    static HWND hRadio4 = NULL;
    static HWND hEdit1 = NULL;
    static HWND hEdit2 = NULL;
    static HWND hEdit3 = NULL;
    static HWND hEdit4 = NULL;
    static HWND hEdit5 = NULL;
    char buff[256];
    BYTE a, b, c, d;

    // Initialize handles for various controls
    hRadio1 = GetDlgItem(hWnd, IDC_RADIO1);
    hRadio2 = GetDlgItem(hWnd, IDC_RADIO2);
    hRadio3 = GetDlgItem(hWnd, IDC_RADIO3);
    hRadio4 = GetDlgItem(hWnd, IDC_RADIO4);
    hEdit1 = GetDlgItem(hWnd, IDC_EDIT8);
    hEdit2 = GetDlgItem(hWnd, IDC_EDIT2);
    hEdit3 = GetDlgItem(hWnd, IDC_EDIT3);
    hEdit4 = GetDlgItem(hWnd, IDC_EDIT6);
    hEdit5 = GetDlgItem(hWnd, IDC_EDIT7);

    if (!hRadio1 || !hRadio2 || !hRadio3 || !hRadio4 || !hEdit1 || !hEdit2 || !hEdit3 || !hEdit4 || !hEdit5)
    {
        return 0;
    }

    switch (uMsgID)
    {
        case WM_INITDIALOG:
            if (g_fFlag)        // Check if any phonebook entry exists
            {                   // Display parametres of the existing entry
                if (g_lpRasEntry->dwfOptions & RASEO_SpecificIpAddr)
                {
                    CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO2);
                    EnableWindow(hEdit1, TRUE);
                    
                    StringCchPrintf(buff, CELEMS(buff), "%d.%d.%d.%d", g_lpRasEntry->ipaddr.a, g_lpRasEntry->ipaddr.b,
                        g_lpRasEntry->ipaddr.c, g_lpRasEntry->ipaddr.d);
                    SendMessage(hEdit1, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) buff);
                }
                else
                {
                    CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                    EnableWindow(hEdit1, FALSE);
                }

                if (g_lpRasEntry->dwfOptions & RASEO_SpecificNameServers)
                {
                    CheckRadioButton(hWnd, IDC_RADIO3, IDC_RADIO4, IDC_RADIO4);
                    EnableWindow(hEdit2, TRUE);
                    EnableWindow(hEdit3, TRUE);
                    EnableWindow(hEdit4, TRUE);
                    EnableWindow(hEdit5, TRUE);
                    
                    StringCchPrintf(buff, CELEMS(buff), "%d.%d.%d.%d", g_lpRasEntry->ipaddrDns.a, g_lpRasEntry->ipaddrDns.b,
                        g_lpRasEntry->ipaddrDns.c, g_lpRasEntry->ipaddrDns.d);
                    SendMessage(hEdit2, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) buff);
                    
                    StringCchPrintf(buff, CELEMS(buff), "%d.%d.%d.%d", g_lpRasEntry->ipaddrDnsAlt.a, g_lpRasEntry->ipaddrDnsAlt.b,
                        g_lpRasEntry->ipaddrDnsAlt.c, g_lpRasEntry->ipaddrDnsAlt.d);
                    SendMessage(hEdit3, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) buff);
                    
                    StringCchPrintf(buff, CELEMS(buff), "%d.%d.%d.%d", g_lpRasEntry->ipaddrWins.a, g_lpRasEntry->ipaddrWins.b,
                        g_lpRasEntry->ipaddrWins.c, g_lpRasEntry->ipaddrWins.d);
                    SendMessage(hEdit4, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) buff);
                    
                    StringCchPrintf(buff, CELEMS(buff), "%d.%d.%d.%d", g_lpRasEntry->ipaddrWinsAlt.a, g_lpRasEntry->ipaddrWinsAlt.b,
                        g_lpRasEntry->ipaddrWinsAlt.c, g_lpRasEntry->ipaddrWinsAlt.d);
                    SendMessage(hEdit5, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) buff);
                }
                else
                {
                    CheckRadioButton(hWnd, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                    EnableWindow(hEdit2, FALSE);
                    EnableWindow(hEdit3, FALSE);
                    EnableWindow(hEdit4, FALSE);
                    EnableWindow(hEdit5, FALSE);
                }

                if (g_lpRasEntry->dwfOptions & RASEO_IpHeaderCompression)
                {
                    CheckDlgButton(hWnd, IDC_CHECK7, BST_CHECKED);
                }

                if (g_lpRasEntry->dwfOptions & RASEO_RemoteDefaultGateway)
                {
                    CheckDlgButton(hWnd, IDC_CHECK8, BST_CHECKED);
                }
            }
            else
            {                   // No entry exists, display default paramters
                CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO2, IDC_RADIO1);
                CheckRadioButton(hWnd, IDC_RADIO3, IDC_RADIO4, IDC_RADIO3);
                EnableWindow(hEdit1, FALSE);
                EnableWindow(hEdit2, FALSE);
                EnableWindow(hEdit3, FALSE);
                EnableWindow(hEdit4, FALSE);
                EnableWindow(hEdit5, FALSE);
                CheckDlgButton(hWnd, IDC_CHECK7, BST_CHECKED);
                CheckDlgButton(hWnd, IDC_CHECK8, BST_CHECKED);
            }
            break;
        case WM_CLOSE:
            EndDialog(hWnd, 0);
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    if (IsDlgButtonChecked(hWnd, IDC_RADIO2) == BST_CHECKED)
                    {           
                        // Enable the SpecificIpAddr flag and store the
                        // IP address in RASENTRY
                        g_lpRasEntry->dwfOptions |= RASEO_SpecificIpAddr;
                        
                        GetWindowText(GetDlgItem(hWnd, IDC_EDIT8), buff, CELEMS(buff));
                        StringToIpAddr( buff, &g_lpRasEntry->ipaddr );
                    }
                    else        // Disable the SpecificIpAddr flag
                    {
                        g_lpRasEntry->dwfOptions &= ~RASEO_SpecificIpAddr;
                    }

                    if (IsDlgButtonChecked(hWnd, IDC_RADIO4) == BST_CHECKED)
                    {           
                        // Enable the SpecificNameServers flag and store
                        // the DNS, DNS Alt., WINS and WINS Alt. addresses in RASENTRY
                        g_lpRasEntry->dwfOptions |= RASEO_SpecificNameServers;
                        
                        GetWindowText(GetDlgItem(hWnd, IDC_EDIT2), buff, CELEMS(buff));
                        StringToIpAddr( buff, &g_lpRasEntry->ipaddrDns );
                        
                        GetWindowText(GetDlgItem(hWnd, IDC_EDIT3), buff, CELEMS(buff));
                        StringToIpAddr(buff, &g_lpRasEntry->ipaddrDnsAlt);

                        GetWindowText(GetDlgItem(hWnd, IDC_EDIT6), buff, CELEMS(buff));
                        StringToIpAddr(buff, &g_lpRasEntry->ipaddrWins);
                        
                        GetWindowText(GetDlgItem(hWnd, IDC_EDIT7), buff, CELEMS(buff));
                        StringToIpAddr(buff, &g_lpRasEntry->ipaddrWinsAlt);
                    }
                    else        
                    {
                        // Disable the SpecificNameServers
                        g_lpRasEntry->dwfOptions &= ~RASEO_SpecificNameServers;
                    }

                    if (IsDlgButtonChecked(hWnd, IDC_CHECK7) == BST_CHECKED)
                    {
                        // Enable the IpHeaderCompression flag
                        g_lpRasEntry->dwfOptions |= RASEO_IpHeaderCompression;
                    }
                    else        
                    {
                        // Disable the IpHeaderCompression flag
                        g_lpRasEntry->dwfOptions &= ~RASEO_IpHeaderCompression;
                    }

                    if (IsDlgButtonChecked(hWnd, IDC_CHECK8) == BST_CHECKED)
                    {
                        // Enable the RemoteDefaultGateway flag
                        g_lpRasEntry->dwfOptions |= RASEO_RemoteDefaultGateway;
                    }
                    else        
                    {
                        // Disable the RemoteDefaultGateway flag
                        g_lpRasEntry->dwfOptions &= ~RASEO_RemoteDefaultGateway;
                    }

                    EndDialog(hWnd, TRUE);
                    break;
                case IDCANCEL:
                    EndDialog(hWnd, 0);
                    break;
                case IDC_RADIO2:
                    EnableWindow(hEdit1, TRUE);
                    break;
                case IDC_RADIO4:
                    EnableWindow(hEdit2, TRUE);
                    EnableWindow(hEdit3, TRUE);
                    EnableWindow(hEdit4, TRUE);
                    EnableWindow(hEdit5, TRUE);
                    break;
                case IDC_RADIO1:
                    EnableWindow(hEdit1, FALSE);
                    break;
                case IDC_RADIO3:
                    EnableWindow(hEdit2, FALSE);
                    EnableWindow(hEdit3, FALSE);
                    EnableWindow(hEdit4, FALSE);
                    EnableWindow(hEdit5, FALSE);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 0;
}


//
//  FUNCTION: Page0_Proc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Intialized Page 0 of the property sheet.
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//			Returns 0.
//
//  COMMENTS:
//		This function handles the dialog box on the zeroth page (Basic) of the
//		property sheet.
//		The dialog box allows the user to enter the name of the phone book
//		entry. It further allows the user to enter the telephone number
//		and the device that is to be associated with that phone book entry
//
//		It uses the following RAS APIs
//		RasEnumDevices(): To enumerate the various RAS capable devices
//		RasGetCountryInfo(): To obtain the country code and country ID
//		RasValidateEntryName(): To verify the entry name is of the correct form
//		RasSetEntryProperties(): To set the modified values of an existing
//		phone book entry or to set the values of a new entry
//
//		The following fields of the RASENTRY structure are modified
//
//		g_lpRasEntry->dwCountryID
//		g_lpRasEntry->dwCountryCode
//		g_lpRasEntry->szAreaCode
//		g_lpRasEntry->szLocalPhoneNumber
//		g_lpRasEntry->szDeviceType
//		g_lpRasEntry->szDeviceName
//
//		The following flag in the g_lpRasEntry->dwfOptions is modified
//
//		RASEO_UseCountryAndAreaCodes
//

INT_PTR CALLBACK Page0Proc(HWND hWnd, UINT uMsgID, WPARAM wParam, LPARAM lParam)
{
    int i = 0, j = 0, k = 0, iItem =0;
    char szCCode[256] = {0};
    char szCName[256] = {0};
    char szTemp[256] = {0};
    char temp[256] = {0};
    DWORD nRet = 0;
    DWORD cb = 0;
    DWORD cDevices = 0;
    DWORD dwSize = 0;
	WPARAM wp = 0;
    LPRASCTRYINFO lpRasCtryInfo = (LPRASCTRYINFO) temp;
    LONG l = 0;

    static HWND hComboBox = NULL;
    static HWND hComboBox2 = NULL;
    static HWND hEditBox1 = NULL;
    static HWND hEditBox3 = NULL;
    static HWND hEditBox4 = NULL;


    // Initialize the various controls
    hComboBox = GetDlgItem(hWnd, IDC_COMBO3);
    hComboBox2 = GetDlgItem(hWnd, IDC_COMBO1);
    hEditBox1 = GetDlgItem(hWnd, IDC_ENTRYNAME);
    hEditBox3 = GetDlgItem(hWnd, IDC_ACODE);
    hEditBox4 = GetDlgItem(hWnd, IDC_PHONE);

    if (!hComboBox || !hComboBox2 || !hEditBox1 || !hEditBox3 || !hEditBox4)
    {
        return 0;
    }

    lpRasCtryInfo->dwCountryID = 1;
    switch (uMsgID)
    {
        case WM_INITDIALOG:

            cb = sizeof(RASDEVINFO);
            
            // Allocate memory in RASDEVINFO for RasEnumDevices
            g_RasDevInfo = (RASDEVINFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
            if (NULL == g_RasDevInfo)
            {
                StringCchPrintf(szTemp, CELEMS(szTemp), "HeapAlloc failed.");
                return 0;
            }

            g_RasDevInfo->dwSize = sizeof(RASDEVINFO);

            nRet = RasEnumDevices(g_RasDevInfo, &cb, &cDevices);
            switch (nRet)
            {
            case ERROR_BUFFER_TOO_SMALL:
                // If the buffer is too small, free the allocated memory and allocate a bigger buffer.
                if (HeapFree(GetProcessHeap(), 0, (LPVOID)g_RasDevInfo))
                {
                    g_RasDevInfo = (LPRASDEVINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cb);
                    if (NULL == g_RasDevInfo)
                    {
                        printf("HeapAlloc failed.\n");
                        return 0;
                    }

                    g_RasDevInfo->dwSize = sizeof(RASDEVINFO);
                    nRet = RasEnumDevices(g_RasDevInfo, &cb, &cDevices);
                }
                else
                {
                    printf("HeapFree failed.\n");
                    return 0;
                }
                break;

            case ERROR_SUCCESS:
                break;

            default:
                StringCchPrintf(szTemp, CELEMS(szTemp), "RasEnumDevices failed: %d ", nRet);
                MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                return 0;
                break;

            }

            // Check for RAS capable devices using RasEnumDevices
            if (ERROR_SUCCESS != nRet)
            {
                StringCchPrintf(szTemp, CELEMS(szTemp), "RasEnumDevices failed: %d ", nRet);
                MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                return 0;
            }
            else
            {
                g_iTemp = cDevices;
                if (!cDevices)
                {
                    StringCchPrintf(szTemp, CELEMS(szTemp), "No RAS capable devices found.");
                    MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                    return 0;
                }
                else
                {               // Display the devices in the combo box
                    for (i = 0; i < (int) cDevices; i++)
                    {
                        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) g_RasDevInfo[i].szDeviceName);
                    }

                    if (g_fFlag)
                    {
                        wp = SendMessage(hComboBox, CB_FINDSTRINGEXACT, 0, (LPARAM) (LPCTSTR) g_lpRasEntry->szDeviceName);
                        SendMessage(hComboBox, CB_SETCURSEL, wp, 0);
                    }
                    else
                    {
                        SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
                    }
                }
            }

            while (lpRasCtryInfo->dwCountryID)
            {
                // Initialize RASCTRYINFO structure to be used in RasGetCountryInfo().
                lpRasCtryInfo->dwSize = sizeof(RASCTRYINFO);
                dwSize = MAX_BUF_SIZE;     // Minimum size of 256 required.
                
                nRet = RasGetCountryInfo(lpRasCtryInfo, &dwSize);
                if (nRet)
                {
                    StringCchPrintf(szTemp, CELEMS(szTemp), "RasGetCountryInfo failed: %d", nRet);
                    MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                    return FALSE;
                }
                
                StringCchPrintf(szCCode, CELEMS(szCCode), "%s (%d)", (LPSTR) lpRasCtryInfo + lpRasCtryInfo->dwCountryNameOffset, lpRasCtryInfo->dwCountryCode);
                
                // Display the country/region information in the combo box
                SendMessage(hComboBox2, CB_ADDSTRING, 0, (LPARAM) (LPCTSTR) szCCode);
                
                // Store the current phonebook entry's country name and code
                if (g_lpRasEntry->dwCountryID == lpRasCtryInfo->dwCountryID)
                {
                    StringCchCopy(szTemp, CELEMS(szTemp), szCCode);
                }

                lpRasCtryInfo->dwCountryID = lpRasCtryInfo->dwNextCountryID;
            }

            if (g_fFlag == TRUE)   // Flag to indicate phonebook entry exists
            {
                // Display the properties retrieved for the first page
                SendMessage(hEditBox1, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) g_szEntryName);
                if (g_lpRasEntry->dwfOptions & RASEO_UseCountryAndAreaCodes)
                {
                    CheckDlgButton(hWnd, IDC_CHECK6, BST_CHECKED);
                    wp = SendMessage(hComboBox2, CB_FINDSTRINGEXACT, 0, (LPARAM) (LPCTSTR) szTemp);
                    SendMessage(hComboBox2, CB_SETCURSEL, wp, 0);
                    SendMessage(hEditBox3, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) g_lpRasEntry->szAreaCode);
                    EnableWindow(hEditBox3, TRUE);
                    EnableWindow(hComboBox2, TRUE);

                }
                else
                {
                    EnableWindow(hEditBox3, FALSE);
                    EnableWindow(hComboBox2, FALSE);
                }

                SendMessage(hEditBox4, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) g_lpRasEntry->szLocalPhoneNumber);

            }
            else
            {
                // Set Default Values
                EnableWindow(hComboBox2, FALSE);
                EnableWindow(hEditBox3, FALSE);
                g_lpRasEntry->dwfNetProtocols |= RASNP_Ip;
                g_lpRasEntry->dwFramingProtocol = RASFP_Ppp;
            }

            return TRUE;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                // If user selects to use the Country Code and Area Code
                case IDC_ECAC:
                    if (IsDlgButtonChecked(hWnd, IDC_ECAC) == BST_CHECKED)
                    {
                        EnableWindow(hEditBox3, TRUE);
                        EnableWindow(hComboBox2, TRUE);
                        SendMessage(hComboBox2, CB_SETCURSEL, 0, 0);
                    }
                    else
                    {
                        EnableWindow(hEditBox3, FALSE);
                        EnableWindow(hComboBox2, FALSE);
                    }
                    return TRUE;
                default:
                    break;

            }
            break;
        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->code)
            {
                case PSN_APPLY: // OK button has been clicked
                    {
                        HWND hwndControl = NULL;
                        
                        // Get the values in the edit fields of the first page
                        hwndControl = GetDlgItem(hWnd, IDC_ENTRYNAME);
                        if (hwndControl)
                        {
                            GetWindowText(hwndControl, g_szEntryName, CELEMS(g_szEntryName));
                        }

                        hwndControl = GetDlgItem(hWnd, IDC_ACODE);
                        if (hwndControl)
                        {
                            GetWindowText(hwndControl, g_lpRasEntry->szAreaCode, CELEMS(g_lpRasEntry->szAreaCode));
                        }

                        hwndControl = GetDlgItem(hWnd, IDC_PHONE);
                        if (hwndControl)
                        {
                            GetWindowText(hwndControl, g_lpRasEntry->szLocalPhoneNumber, CELEMS(g_lpRasEntry->szLocalPhoneNumber));
                        }

                        // Validate the phonebook entry name using RasValidateEntryName
                        if (nRet = RasValidateEntryName(NULL, (LPSTR)g_szEntryName))
                        {
                            switch (nRet)
                            {
                                case ERROR_ALREADY_EXISTS:
                                    // If entry already exists, warn user that original values
                                    // will be overwritten
                                    g_fValidEntry = TRUE;
                                    StringCchPrintf(szTemp, CELEMS(szTemp), "Entry name %s already exists, will overwrite!", g_szEntryName);
                                    iItem = MessageBox(hWnd, szTemp, "Warning!!!", MB_YESNO | MB_ICONWARNING);
                                    if (iItem == IDNO)
                                        return 0;
                                    break;
                                case ERROR_INVALID_NAME:
                                    g_fValidEntry = FALSE;
                                    StringCchPrintf(szTemp, CELEMS(szTemp), "Entry name %s is invalid", g_szEntryName);
                                    MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                                    return 0;
                                    break;
                            }
                        }
                        else
                        {
                            g_fValidEntry = TRUE;
                        }

                        if (IsDlgButtonChecked(hWnd, IDC_ECAC) == BST_CHECKED)
                            // Enable the UseCountryAndAreaCodes flag
                        {
                            g_lpRasEntry->dwfOptions |= RASEO_UseCountryAndAreaCodes;
                            // Get the country name and code
						    wp = SendMessage(hComboBox2, CB_GETCURSEL, 0, 0);
                            l = SendMessage(hComboBox2, CB_GETLBTEXTLEN, wp, (LPARAM)0);
                            if (CELEMS(szTemp) > l)
                            {
                                SendMessage(hComboBox2, CB_GETLBTEXT, wp, (LPARAM) (LPCSTR) szTemp);
                                StringCchCopy(szCCode, CELEMS(szCCode), szTemp);
                            }
                            else
                            {
                                return FALSE;
                            }

                            k = 0;
                            // Store the country name in a buffer
                            while (szCCode[k] != '(')
                            {
                                k++;
                            }

                            szCCode[k - 1] = '\0';
                            k++;
                            i = 0;

                            // Store the country code in another buffer
                            while (szTemp[k] != ')')
                            {
                                szCName[i] = szTemp[k];
                                i++;
                                k++;
                            }

                            szCName[i] = '\0';
                            
                            // Convert the country code string to an integer
                            j = atoi(szCName);
                            lpRasCtryInfo->dwSize = sizeof(RASCTRYINFO);
                            lpRasCtryInfo->dwCountryID = 1;

                            // Obtain the country ID by using RasGetCountryInfo
                            while (lpRasCtryInfo->dwCountryID)
                            {
                                lpRasCtryInfo->dwSize = sizeof(RASCTRYINFO);
                                dwSize = MAX_BUF_SIZE;
                                if (nRet = RasGetCountryInfo(lpRasCtryInfo, &dwSize))
                                {
                                    StringCchPrintf(szTemp, CELEMS(szTemp), "RasGetCountryInfo failed: %d", nRet);
                                    MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                                    return -1;
                                }
                                if ((lstrcmp(szCCode, (LPSTR) lpRasCtryInfo + lpRasCtryInfo->dwCountryNameOffset)) == 0)
                                {
                                    g_lpRasEntry->dwCountryCode = lpRasCtryInfo->dwCountryCode;
                                    g_lpRasEntry->dwCountryID = lpRasCtryInfo->dwCountryID;
                                    break;
                                }

                                lpRasCtryInfo->dwCountryID = lpRasCtryInfo->dwNextCountryID;
                            }
                        }
                        else
                        {
                            g_lpRasEntry->dwfOptions &= ~RASEO_UseCountryAndAreaCodes;
                        }

                        // Get the device chosen from the combo box
					    wp = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
                        l = SendMessage(hComboBox, CB_GETLBTEXTLEN, wp, (LPARAM) 0);
                        if (CELEMS(g_lpRasEntry->szDeviceName) > l)
                        {
                            SendMessage(hComboBox, CB_GETLBTEXT, wp, (LPARAM) (LPCSTR) g_lpRasEntry->szDeviceName);
                            for (i = 0; i < g_iTemp; i++)
                            {
                                if (!(lstrcmp(g_RasDevInfo[i].szDeviceName, g_lpRasEntry->szDeviceName)))
                                {
                                    StringCchCopy(g_lpRasEntry->szDeviceType, CELEMS(g_lpRasEntry->szDeviceType), g_RasDevInfo[i].szDeviceType);
                                }
                            }

                            // Set the properties of the phonebook entry using
                            // RasSetEntryProperties

                            if (nRet = RasSetEntryProperties(NULL, (LPSTR) g_szEntryName, g_lpRasEntry, g_lpRasEntry->dwSize, NULL, 0))
                            {
                                StringCchPrintf(szTemp, CELEMS(szTemp), "RasSetEntryProperties  failed: %d", nRet);
                                MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                            }
                        }    
                        else
                        {
                            return FALSE;
                        }

                        return TRUE;
                    }
                default:
                    break;
            }
            break;

        default:
            break;
    }

    return FALSE;
}

//
//  FUNCTION: Page1_Proc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Intialized Page 1 of the property sheet.
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//			Returns 0.
//
//  COMMENTS:
//		This function handles the dialog box on the first page (Server) of the
//		property sheet.
//		The dialog box allows the user to enter the dial up server type.
//		There are two options to choose from. It further allows the user to
//		choose the Network Protocols, namely, TCP/IP or IPX/SPX.
//		Finally it allows the user to enable Software Compression and PPP LCP
//		extensions.
//
//		The following fields of the RASENTRY structure are modified
//
//		g_lpRasEntry->dwfNetProtocols
//		g_lpRasEntry->dwFramingProtocol
//
//		The following flags in the g_lpRasEntry->dwfOptions are modified
//
//		RASEO_SwCompression
//		RASEO_DisableLcpExtensions
//

INT_PTR CALLBACK Page1Proc(HWND hWnd, UINT uMsgID, WPARAM wParam, LPARAM lParam)
{
    static HWND hComboBox = NULL;
    static HWND hCheckBox1 = NULL;
    static HWND hCheckBox4 = NULL;
    static HWND hCheckBox5 = NULL;
    static HWND hButton = NULL;
    int i = 0;

    // Initialize the various controls
    hComboBox = GetDlgItem(hWnd, IDC_COMBO2);
    hCheckBox1 = GetDlgItem(hWnd, IDC_CHECK1);
    hCheckBox4 = GetDlgItem(hWnd, IDC_CHECK4);
    hCheckBox5 = GetDlgItem(hWnd, IDC_CHECK5);
    hButton = GetDlgItem(hWnd, IDC_BUTTON1);

    if (!hComboBox || !hCheckBox1 || !hCheckBox4 || !hCheckBox5 || !hButton)
    {
        return FALSE;
    }

    switch (uMsgID)
    {
        case WM_INITDIALOG:
            SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM) "PPP: Windows NT, Windows 95 Plus, Internet");
            SendMessage(hComboBox, CB_ADDSTRING, 1, (LPARAM) "SLIP: Internet");

            if (g_fFlag)           // Phonebook entry exists
            {                   // Display the entry's parameters
                
                switch (g_lpRasEntry->dwFramingProtocol)
                {
                    case RASFP_Ppp:
                        SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
                        break;
                    case RASFP_Slip:
                        SendMessage(hComboBox, CB_SETCURSEL, 1, 0);
                        break;
                    
                    default:
                        break;
                }
                
                if (g_lpRasEntry->dwfOptions & RASEO_SwCompression)
                {
                    CheckDlgButton(hWnd, IDC_CHECK4, BST_CHECKED);
                }
                else
                {
                    CheckDlgButton(hWnd, IDC_CHECK4, BST_UNCHECKED);
                }

                if (g_lpRasEntry->dwfOptions & RASEO_DisableLcpExtensions)
                {
                    CheckDlgButton(hWnd, IDC_CHECK5, BST_UNCHECKED);
                }
                else
                {
                    CheckDlgButton(hWnd, IDC_CHECK5, BST_CHECKED);
                }


                if (g_lpRasEntry->dwfNetProtocols & RASNP_Ip)
                {
                    CheckDlgButton(hWnd, IDC_CHECK1, BST_CHECKED);
                }
                else
                {
                    CheckDlgButton(hWnd, IDC_CHECK1, BST_UNCHECKED);
                }
            }
            else
            {                   
                // Display default parameters
                SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
                CheckDlgButton(hWnd, IDC_CHECK5, BST_CHECKED);
                CheckDlgButton(hWnd, IDC_CHECK1, BST_CHECKED);
            }
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_BUTTON1:   // TCP/IP Settings Button pressed
                    DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG7), hWnd, Dlg_Proc1);
                    return TRUE;

                case IDC_COMBO2:// Framing Protocol settings
                    if (CBN_SELCHANGE == HIWORD(wParam))
                    {
                        i = (int)SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
                        switch (i)
                        {
                            case 0: // PPP

                                CheckDlgButton(hWnd, IDC_CHECK1, BST_CHECKED);
                                CheckDlgButton(hWnd, IDC_CHECK5, BST_CHECKED);
                                EnableWindow(hCheckBox1, TRUE);
                                EnableWindow(hCheckBox4, TRUE);
                                EnableWindow(hCheckBox5, TRUE);
                                EnableWindow(hButton, TRUE);
                                break;
                            case 1: // Slip
                                EnableWindow(hCheckBox1, TRUE);
                                CheckDlgButton(hWnd, IDC_CHECK1, BST_CHECKED);
                                if (IsDlgButtonChecked(hWnd, IDC_CHECK4))
                                    CheckDlgButton(hWnd, IDC_CHECK4, BST_UNCHECKED);
                                if (IsDlgButtonChecked(hWnd, IDC_CHECK5))
                                    CheckDlgButton(hWnd, IDC_CHECK5, BST_UNCHECKED);
                                EnableWindow(hCheckBox4, FALSE);
                                EnableWindow(hCheckBox5, FALSE);
                                EnableWindow(hButton, TRUE);
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            return TRUE;

        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->code)
            {
                case PSN_KILLACTIVE:
                    i = (int)SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
                    switch (i)  // Set framing protocol based on the
                    {           // value returned from the combo box
                        case 0:
                            g_lpRasEntry->dwFramingProtocol = RASFP_Ppp;
                            break;
                        case 1: 
                            g_lpRasEntry->dwFramingProtocol = RASFP_Slip;
                            break;
                        default:
                            g_lpRasEntry->dwFramingProtocol = RASFP_Ppp;
                            break;
                    }

                    if (IsDlgButtonChecked(hWnd, IDC_CHECK4) == BST_CHECKED)
                    {
                        // Enable the SwCompression flag
                        g_lpRasEntry->dwfOptions |= RASEO_SwCompression;
                    }
                    else        
                    {
                        // Disable the SwCompression flag
                        g_lpRasEntry->dwfOptions &= ~RASEO_SwCompression;
                    }

                    if (IsDlgButtonChecked(hWnd, IDC_CHECK5) == BST_UNCHECKED)
                    {
                        // Enable the DisableLcpExtensions flag
                        g_lpRasEntry->dwfOptions |= RASEO_DisableLcpExtensions;
                    }
                    else        
                    {
                        // Disable the DisableLcpExtensions flag
                        g_lpRasEntry->dwfOptions &= ~RASEO_DisableLcpExtensions;
                    }
                    
                    if (IsDlgButtonChecked(hWnd, IDC_CHECK1) == BST_CHECKED)
                    {
                        // Enable Ip Net Protocol
                        g_lpRasEntry->dwfNetProtocols |= RASNP_Ip;
                    }
                    else        
                    {
                        // Disable Ip Net Protocol
                        g_lpRasEntry->dwfNetProtocols &= ~RASNP_Ip;
                    }

                    return TRUE;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return FALSE;
}

//
//  FUNCTION: Page2_Proc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Intialized Page 2 of the property sheet.
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//			Returns 0.
//
//  COMMENTS:
//		This function handles the dialog box on the second page (Script) of the
//		property sheet.
//		The dialog box allows the user to enter the name of script that they
//		want to run after dialing. The user can also enable a Terminal Window
//		to show up after dialing.
//
//		The following field of the RASENTRY structure is modified
//
//		g_lpRasEntry->szScript
//
//		The following flag in the g_lpRasEntry->dwfOptions is modified
//
//		RASEO_TerminalAfterDial
//

INT_PTR CALLBACK Page2Proc(HWND hWnd, UINT uMsgID, WPARAM wParam, LPARAM lParam)
{
    static HWND hEdit1 = NULL;

    // Initialize the various controls
    hEdit1 = GetDlgItem(hWnd, IDC_EDIT1);

    if (!hEdit1)
    {
        return 0;
    }

    switch (uMsgID)
    {
        case WM_INITDIALOG:
            if (g_fFlag)        // Phonebook entry exists
            {                   // Display the parameters of the entry
                if (g_lpRasEntry->dwfOptions & RASEO_TerminalAfterDial)
                {
                    CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO3);
                    EnableWindow(hEdit1, FALSE);
                }
                else
                {
                    if ((strcmp(g_lpRasEntry->szScript, "")) != 0)
                    {
                        CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO2);
                        EnableWindow(hEdit1, TRUE);
                        SendMessage(hEdit1, WM_SETTEXT, 0, (LPARAM) (LPCTSTR) g_lpRasEntry->szScript);
                    }
                    else
                    {
                        CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
                        EnableWindow(hEdit1, FALSE);
                    }
                }
            }
            else
            {                   // Display default parametrs
                CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO1);
                EnableWindow(hEdit1, FALSE);
            }

            break;
        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDC_RADIO1:// Disable the edit box for holding the script name
                case IDC_RADIO3:
                    EnableWindow(hEdit1, FALSE);
                    break;
                case IDC_RADIO2:// Enable the edit box for holding the script name
                    EnableWindow(hEdit1, TRUE);
                    break;
                default:
                    break;
            }
            break;
        case WM_NOTIFY:
            switch (((LPNMHDR) lParam)->code)
            {
                case PSN_KILLACTIVE:
                    if (IsDlgButtonChecked(hWnd, IDC_RADIO3))
                    {
                        // Enable the TerminalAfterDial flag
                        g_lpRasEntry->dwfOptions |= RASEO_TerminalAfterDial;
                    }
                    else        
                    {
                        // Disable the TerminalAfterDial flag 
                        g_lpRasEntry->dwfOptions &= ~RASEO_TerminalAfterDial;
                    }

                    if (IsDlgButtonChecked(hWnd, IDC_RADIO2))
                    {           
                        // Get the script name from the edit box
                        GetWindowText(GetDlgItem(hWnd, IDC_EDIT1), g_lpRasEntry->szScript, 256);
                    }
                    else        
                    {
                        // Corresponds to "None"
                        g_lpRasEntry->szScript[0] = TEXT('\0');
                    }

                    return TRUE;
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
    return 0;
}


//
//  FUNCTION: PropSheet()
//
//  PURPOSE:  Intialized Property Sheet.
//
//  PARAMETERS:
//		none
//
//  RETURN VALUE:
//		Returns 0.
//
//  COMMENTS:
//		This function defines and initializes the property sheet.
//

BOOL CALLBACK PropSheet(HWND hWnd)
{
    // Build property sheet
    PROPSHEETHEADER pshSheet;
    PROPSHEETPAGE pspPage[NUMBER_OF_PROPERTY_SHEETS];
    int iRC = 0;

    ZeroMemory(&pshSheet, sizeof(PROPSHEETHEADER));

    // Sheet
    pshSheet.dwSize = sizeof(PROPSHEETHEADER);
    pshSheet.dwFlags = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
    pshSheet.hwndParent = hWnd;
    pshSheet.hInstance = g_hInstance;
    pshSheet.pszCaption = "EditPhonebookEntries";
    pshSheet.nPages = NUMBER_OF_PROPERTY_SHEETS;
    pshSheet.ppsp = pspPage;

    ZeroMemory(&pspPage, sizeof(PROPSHEETPAGE) * NUMBER_OF_PROPERTY_SHEETS);

    // Page 0
    pspPage[0].dwSize = sizeof(PROPSHEETPAGE);
    pspPage[0].hInstance = g_hInstance;
    pspPage[0].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG2);
    pspPage[0].pfnDlgProc = Page0Proc;

    // Page 1
    pspPage[1].dwSize = sizeof(PROPSHEETPAGE);
    pspPage[1].hInstance = g_hInstance;
    pspPage[1].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG3);
    pspPage[1].pfnDlgProc = Page1Proc;

    // Page 2
    pspPage[2].dwSize = sizeof(PROPSHEETPAGE);
    pspPage[2].hInstance = g_hInstance;
    pspPage[2].pszTemplate = MAKEINTRESOURCE(IDD_DIALOG4);
    pspPage[2].pfnDlgProc = Page2Proc;

    iRC = PropertySheet(&pshSheet);

    return 0;
}


//
//  FUNCTION: WinMain(HWND, UINT, WPARAM, LPARAM)
//
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//		Returns the main dialog box which lists all the available phone
//		book entries.
//

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
    int iRet = 0;
    iRet = (int)DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, Dlg_Proc2);

    if (g_RasDevInfo)
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID)g_RasDevInfo);
        g_RasDevInfo = NULL;
    }

    return iRet;
}

//
//  FUNCTION: Dlg_Proc2(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Display a list of available phone book entries and
//			  allow user to modify it or enter a new entry to the phone book
//
//  PARAMETERS:
//    hwnd     - window handle
//    uMessage - message number
//    wparam   - additional information (dependant on message number)
//    lparam   - additional information (dependant on message number)
//
//  RETURN VALUE:
//			Returns 0.
//
//  COMMENTS:
//		This function displays the dialog box that contains a list of existing
//		phonebook entries. The user can either modify these entries or enter
//		values for a new entry.
//
//		RAS API's used
//
//		RasEnumEntries(): To enumerate the list of the phone book entries
//		RasGetEntryProperties(): To get the properties of an existing entry
//

INT_PTR CALLBACK Dlg_Proc2(HWND hWnd, UINT uMsgID, WPARAM wParam, LPARAM lParam)
{
    int i = 0;
    char szTemp[256] = {0};
    DWORD nRet = 0;
    LPRASENTRYNAME lpRasEntryName = NULL;
    LPRASENTRYNAME lpTempRasEntryName = NULL;

    DWORD cbEntry = sizeof(RASENTRYNAME);
    DWORD cEntries = 0;
    static HWND hComboBox = NULL;
    DWORD dwRasEntryInfoSize = 0;
	WPARAM wp = 0;
    LONG l = 0;

    hComboBox = GetDlgItem(hWnd, IDC_COMBO1);
    if (!hComboBox)
    {
        return FALSE;
    }

    switch (uMsgID)
    {
        case WM_INITDIALOG:
            // Enumerate all the phonebook entries using RasEnumEntries

            lpRasEntryName = (LPRASENTRYNAME)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASENTRYNAME));
            if (NULL == lpRasEntryName)
            {
                return FALSE;
            }

            lpRasEntryName->dwSize = sizeof(RASENTRYNAME);
            nRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &cbEntry, &cEntries);
            if (ERROR_BUFFER_TOO_SMALL == nRet)
            {
                // Need to make sure that we actually free the memory before allocating it again.
                if (HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasEntryName))
                {
                    lpRasEntryName = (LPRASENTRYNAME)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbEntry);
                    if (NULL == lpRasEntryName)
                    {
                        StringCchPrintf(szTemp, CELEMS(szTemp), "Couldn't allocate memory.");
                        MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                        return FALSE;
                    }

                    lpRasEntryName->dwSize = sizeof(RASENTRYNAME);
                    if (nRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &cbEntry, &cEntries))
                    {
                        StringCchPrintf(szTemp, CELEMS(szTemp), "RasEnumEntries  failed: %d", nRet);
                        MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);

                        if (lpRasEntryName)
                        {
                            HeapFree(GetProcessHeap(), 0, (LPVOID)lpRasEntryName);
                            lpRasEntryName = NULL;
                        }
                        
                        return FALSE;
                    }
                }
                else
                {
                    return FALSE;
                }
            }

            lpTempRasEntryName = lpRasEntryName;
            if (cEntries)
            {
                g_dwEntries = cEntries;
                for (i = 0; i < (int) cEntries; i++)
                {
                    SendMessage(hComboBox, CB_ADDSTRING, i,
                        (LPARAM) (LPCTSTR) lpRasEntryName->szEntryName);
                    lpRasEntryName++;
                }

                SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
                GetWindowText(hComboBox, g_szEntryName, CELEMS(g_szEntryName));
            }
            else
            {
                SendMessage(hComboBox, CB_SETCURSEL, 0, 0);
            }

            if (lpTempRasEntryName)
            {
                HeapFree(GetProcessHeap(), 0, (LPVOID)lpTempRasEntryName);
                lpRasEntryName = NULL;
            }
            return TRUE;

        case WM_CLOSE:
            EndDialog(hWnd, 0);
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hWnd, 0);
                    return TRUE;

                case IDNEW:
                    g_fFlag = FALSE;
                    g_lpRasEntry = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(RASENTRY));
                    if (NULL == g_lpRasEntry)
                    {
                        return FALSE;
                    }

                    g_lpRasEntry->dwSize = sizeof(RASENTRY);
                    PropSheet(hWnd);
                    if (g_fValidEntry)
                    {
                        SendMessage(hComboBox, CB_ADDSTRING, 0, (LPARAM) (LPCSTR) g_szEntryName);
						wp = SendMessage(hComboBox, CB_FINDSTRINGEXACT, 0, (LPARAM) (LPCTSTR) g_szEntryName);
                        SendMessage(hComboBox, CB_SETCURSEL, wp, 0);
                        g_dwEntries++;
                    }

                    HeapFree(GetProcessHeap(), 0, (LPVOID)g_lpRasEntry);
                    g_lpRasEntry = NULL;

                    return TRUE;
                case IDCHANGE:
                    if (g_dwEntries)
                    {
                        wp = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
                        l = SendMessage(hComboBox, CB_GETLBTEXTLEN, wp, (LPARAM) 0);
                        if (CELEMS(g_szEntryName) > l)
                        {
                            SendMessage(hComboBox, CB_GETLBTEXT, wp, (LPARAM) (LPCSTR) g_szEntryName);
                        }

                        // Initialize the dwRasEntryInfoSize parameter
                        if ((nRet = RasGetEntryProperties(NULL,
                                    g_szEntryName,
                                    NULL,
                                    &dwRasEntryInfoSize,
                                    NULL,
                                    0)) != ERROR_BUFFER_TOO_SMALL)
                        {
                            StringCchPrintf(szTemp, CELEMS(szTemp), "RasGetEntryProperty failed: %d", nRet);
                            MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                        }

                        if (dwRasEntryInfoSize > 0)
                        {
                            g_lpRasEntry = (LPRASENTRY) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwRasEntryInfoSize);
                            if (NULL == g_lpRasEntry)
                            {
                                return FALSE;
                            }
                            g_lpRasEntry->dwSize = dwRasEntryInfoSize;
                        }
                        else
                        {
                            g_lpRasEntry = NULL;
                        }

                        // Retrieve the properties of the phonebook entry using
                        // RasGetEntryProperties
                        if (nRet = RasGetEntryProperties(NULL,
                                g_szEntryName,
                                g_lpRasEntry,
                                &dwRasEntryInfoSize,
                                NULL,
                                0))
                        {
                            StringCchPrintf(szTemp, CELEMS(szTemp), "RasGetEntryProperty failed: %d", nRet);
                            MessageBox(hWnd, szTemp, NULL, MB_OK | MB_ICONERROR);
                        }
                        else
                        {
                            g_fFlag = TRUE;    // Flag to indicate phonebook entry exists
                            PropSheet(hWnd);
                        }

                        HeapFree(GetProcessHeap(), 0, (LPVOID)g_lpRasEntry);
                        g_lpRasEntry = NULL;

                    }
                    return TRUE;

                case IDC_COMBO1:
					wp = SendMessage(hComboBox, CB_GETCURSEL, 0, 0);
                    l = SendMessage(hComboBox, CB_GETLBTEXTLEN, wp, (LPARAM) 0);
                    if (CELEMS(g_szEntryName) > l)
                    {
                        SendMessage(hComboBox, CB_GETLBTEXT, wp, (LPARAM) (LPCSTR) g_szEntryName);
                    }
                    else
                    {
                        return FALSE;
                    }

                    return TRUE;
            }
            break;
    }

    return FALSE;
}
