//////////////////////////////////////////////////////////////////////////////
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <windows.h>
#include <prsht.h>
#include "resource.h"

#define MAXPAGES 5

//
//Forward definitinos of respective dialog procs
//
LRESULT CALLBACK IntroDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK IntPage1DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK IntPage3DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK IntPage2DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK EndDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

typedef struct
{
    BOOL fIsBoxChecked; //The state of the first interior page's check box
    BOOL fIsButtonClicked; //The state of the first interior page's group box
    //other shared data added here
} SHAREDWIZDATA;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nCmdShow)
{
    UNREFERENCED_PARAMETER(nCmdShow);
    UNREFERENCED_PARAMETER(hPrevInstance);

    PROPSHEETPAGE   psp;					//defines the property sheet pages
    HPROPSHEETPAGE  rhpsp[MAXPAGES];		//an array to hold the page's HPROPSHEETPAGE handles
    SHAREDWIZDATA   wizdata;				//the shared data structure
    UINT            cPages          = 0;    //number of pages being added
    BOOL            fPagesCreated   = FALSE;//were all of the pages created successfully?

    ZeroMemory(&psp, sizeof(psp));
    ZeroMemory(&wizdata, sizeof(wizdata));
    ZeroMemory(&rhpsp, sizeof(HPROPSHEETPAGE)*MAXPAGES);

    //
    // Create the Wizard pages
    //

    // Opening page

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT | PSP_HIDEHEADER;
    psp.hInstance   = hInstance;
    psp.lParam      = (LPARAM) &wizdata; //The shared data structure
    psp.pfnDlgProc  = (DLGPROC)IntroDlgProc;
    psp.pszTemplate = MAKEINTRESOURCE(IDD_INTRO);

    rhpsp[cPages]	= CreatePropertySheetPage(&psp);

    if (rhpsp[cPages])
    {
        ++cPages;

        // First interior page

        psp.dwFlags             = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        psp.pszHeaderTitle      = MAKEINTRESOURCE(IDS_TITLE1);
        psp.pszHeaderSubTitle	= MAKEINTRESOURCE(IDS_SUBTITLE1);
        psp.pszTemplate         = MAKEINTRESOURCE(IDD_INTERIOR1);
        psp.pfnDlgProc          = (DLGPROC)IntPage1DlgProc;

        rhpsp[cPages]           = CreatePropertySheetPage(&psp);

        if (rhpsp[cPages])
        {
            ++cPages;

            // Second interior page

            psp.dwFlags             = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
            psp.pszHeaderTitle      = MAKEINTRESOURCE(IDS_TITLE2);
            psp.pszHeaderSubTitle   = MAKEINTRESOURCE(IDS_SUBTITLE2);
            psp.pszTemplate         = MAKEINTRESOURCE(IDD_INTERIOR2);
            psp.pfnDlgProc          = (DLGPROC)IntPage2DlgProc;

            rhpsp[cPages]           = CreatePropertySheetPage(&psp);

            if (rhpsp[cPages])
            {
                ++cPages;

                //Third interior page

                psp.dwFlags             = PSP_DEFAULT | PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
                psp.pszHeaderTitle      = MAKEINTRESOURCE(IDS_TITLE3);
                psp.pszHeaderSubTitle   = MAKEINTRESOURCE(IDS_SUBTITLE3);
                psp.pszTemplate         = MAKEINTRESOURCE(IDD_INTERIOR3);
                psp.pfnDlgProc          = (DLGPROC)IntPage3DlgProc;

                rhpsp[cPages]           = CreatePropertySheetPage(&psp);

                if (rhpsp[cPages])
                {
                    ++cPages;

                    // Final page

                    psp.dwFlags     = PSP_DEFAULT | PSP_HIDEHEADER;
                    psp.pszTemplate = MAKEINTRESOURCE(IDD_END);
                    psp.pfnDlgProc  = (DLGPROC)EndDlgProc;

                    rhpsp[cPages]   = CreatePropertySheetPage(&psp);

                    if (rhpsp[cPages])
                    {
                        ++cPages;
                        fPagesCreated = TRUE;
                    }
                }
            }
        }
    }

    if (fPagesCreated)
    {
        // Create the property sheet
        PROPSHEETHEADER psh;  //defines the property sheet
        ZeroMemory(&psh, sizeof(psh));

        psh.dwSize          = sizeof(psh);
        psh.hInstance       = hInstance;
        psh.hwndParent      = NULL;
        psh.phpage          = rhpsp;
        psh.dwFlags         = PSH_WIZARD97 | PSH_WATERMARK | PSH_HEADER;
        psh.pszbmWatermark  = MAKEINTRESOURCE(IDB_WATERMARK);
        psh.pszbmHeader     = MAKEINTRESOURCE(IDB_BANNER);
        psh.nStartPage      = 0;
        psh.nPages          = cPages;

        //Display the wizard
        if (PropertySheet(&psh) == -1)
        {
            // an error occurred, call GetLastError() to retrieve it here.
        }
    }
    else
    {
        // error creating pages -- clean up any we did create
        for (UINT iPage = 0; iPage < cPages; ++iPage)
        {
            DestroyPropertySheetPage(rhpsp[iPage]);
        }
    }

    return 0;
}

//
// Process messages from the Welcome page
//
LRESULT CALLBACK IntroDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    SHAREDWIZDATA *pdata = (SHAREDWIZDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        { 
            // Get the PROPSHEETPAGE lParam value and load it into DWL_USERDATA

            PROPSHEETPAGE *psp = (PROPSHEETPAGE *)lParam;
            pdata = (SHAREDWIZDATA *)(psp->lParam);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (DWORD_PTR)pdata);
            break;
        }

    case WM_NOTIFY:
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
            {
            case PSN_SETACTIVE : //Enable the Next button    
                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_NEXT);
                break;

            case PSN_WIZNEXT:
                //Handle a Next button click here
                break;

            case PSN_RESET:
                //Handle a Cancel button click, if necessary
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }
    return 0;
}

// Process messages from the first interior page
LRESULT CALLBACK IntPage1DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    SHAREDWIZDATA *pdata = (SHAREDWIZDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        { 
            // Get the PROPSHEETPAGE lParam value and load it into DWL_USERDATA

            PROPSHEETPAGE *psp = (PROPSHEETPAGE *)lParam;
            pdata = (SHAREDWIZDATA *)(psp->lParam);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (DWORD_PTR)pdata);
            break;
        }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK1:
            pdata->fIsBoxChecked = !(pdata->fIsBoxChecked);
            break;

        case IDC_RADIO1:
        case IDC_RADIO2:
        case IDC_RADIO3:
            pdata->fIsButtonClicked = TRUE;
            break;

        default:
            break;
        }

        //If any of the radio buttons are clicked, or the
        //checkbox checked, enable the Next button

        if (pdata->fIsBoxChecked || pdata->fIsButtonClicked)
        {
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
        }
        else //otherwise, only enable the Back button
        {
            PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
            {
            case PSN_SETACTIVE: // Enable the appropriate buttons

                // If a radio button has been clicked or the 
                // checkbox checked, enable Back and Next 
                if (pdata->fIsBoxChecked || pdata->fIsButtonClicked) 
                {
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                }
                else 
                {
                    // Otherwise, only enable Back
                    PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK);
                }
                break;

            case PSN_WIZNEXT:

                // If the checkbox is checked, jump to the final page

                if (pdata->fIsBoxChecked)
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_END);
                    return TRUE;
                }
                break;

            case PSN_WIZBACK:
                //Handle a Back button click, if necessary
                break;

            case PSN_RESET:
                //Handle a Cancel button click, if necessary
                break;

            default:
                break;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

//
// Process messages from the second interior page
//
LRESULT CALLBACK IntPage2DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    SHAREDWIZDATA *pdata = (SHAREDWIZDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg) 
    {
    case WM_INITDIALOG:
        { 
            // Get the PROPSHEETPAGE lParam value and load it into DWL_USERDATA

            PROPSHEETPAGE *psp = (PROPSHEETPAGE *)lParam;
            pdata = (SHAREDWIZDATA *)(psp->lParam);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (DWORD_PTR)pdata);
            break;
        }

    case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
            case IDC_UPDATEENABLE:
                {
                    DWORD dwState = 0;
                    if (IsDlgButtonChecked(hwndDlg, IDC_ENABLEBACK))
                    {
                        dwState |= PSWIZB_BACK;
                    }
                    if (IsDlgButtonChecked(hwndDlg, IDC_ENABLEFINISH))
                    {
                        dwState |= PSWIZB_FINISH;
                    }
                    if (IsDlgButtonChecked(hwndDlg, IDC_DISABLEFINISH))
                    {
                        dwState |= PSWIZB_DISABLEDFINISH;
                    }
                    if (IsDlgButtonChecked(hwndDlg, IDC_ENABLENEXT))
                    {
                        dwState |= PSWIZB_NEXT;
                    }
                    SendMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, (WPARAM)0, (LPARAM)dwState);
                }
                break;
            }
            break;
        }	 

    case WM_NOTIFY:
        {
            LPNMHDR lpnm = (LPNMHDR) lParam; 
            switch (lpnm->code)
            {
            case PSN_SETACTIVE : 
                //Enable the correct buttons on for the active page  
                SendMessage(GetParent(hwndDlg), PSM_SETWIZBUTTONS, (WPARAM)0, (LPARAM)PSWIZB_BACK | PSWIZB_NEXT);
                break;

            case PSN_WIZBACK :
                //Handle a Back button click, if necessary
                break;

            case PSN_WIZNEXT :
                //Handle a Next button click, if necessary
                break;

            case PSN_WIZFINISH :
                //Handle a Finish button click, if necessary				
                break;

            case PSN_RESET :
                //Handle a Cancel button click, if necessary				
                break;            
            }            

            break;
        }            
    }

    return 0;	
}


//
// Process messages from the third interior page
//
LRESULT CALLBACK IntPage3DlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    SHAREDWIZDATA *pdata = (SHAREDWIZDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        { 
            // Get the PROPSHEETPAGE lParam value and load it into DWL_USERDATA

            PROPSHEETPAGE *psp = (PROPSHEETPAGE *)lParam;
            pdata = (SHAREDWIZDATA *)(psp->lParam);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (DWORD_PTR)pdata);
            break;
        }

    case WM_NOTIFY:
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
            {
            case PSN_SETACTIVE : //Enable the Next and Back buttons

                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_NEXT);
                break;

            case PSN_WIZNEXT :
                //Handle a Next button click, if necessary
                break;

            case PSN_WIZBACK :
                //Handle a Back button click, if necessary
                break;

            case PSN_RESET :
                //Handle a Cancel button click, if necessary
                break;

            default :
                break;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}

// Process messages from the Completion page
LRESULT CALLBACK EndDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);

    SHAREDWIZDATA *pdata = (SHAREDWIZDATA *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

    switch (uMsg)
    {
    case WM_INITDIALOG:
        { 
            // Get the PROPSHEETPAGE lParam value and load it into DWL_USERDATA

            PROPSHEETPAGE* psp = (PROPSHEETPAGE *)lParam;
            pdata = (SHAREDWIZDATA *)(psp->lParam);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (DWORD_PTR)pdata);
            break;
        }

    case WM_NOTIFY:
        {
            LPNMHDR lpnm = (LPNMHDR) lParam;

            switch (lpnm->code)
            {
            case PSN_SETACTIVE : //Enable the correct buttons on for the active page

                PropSheet_SetWizButtons(GetParent(hwndDlg), PSWIZB_BACK | PSWIZB_FINISH);
                break;

            case PSN_WIZBACK :
                //If the checkbox was checked, jump back
                //to the first interior page, not the third
                if (pdata->fIsBoxChecked)
                {
                    SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, IDD_INTERIOR1);
                    return TRUE;
                }
                break;

            case PSN_WIZFINISH :
                //Handle a Finish button click, if necessary
                break;

            case PSN_RESET :
                //Handle a Cancel button click, if necessary
                break;

            default :
                break;
            }
        }
        break;

    default:
        break;
    }

    return 0;
}   
