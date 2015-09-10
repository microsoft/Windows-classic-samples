/*************************************************************************************************
*
* File: FullscreenMagnifierSample.cpp
*
* Description: Implements simple UI to control fullscreen magnification, using the 
* Magnification API.
*
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
* 
*************************************************************************************************/

#include "windows.h"
#include "resource.h"
#include "strsafe.h"
#include "magnification.h"

// Global variables and strings.
const LPWSTR g_pszAppTitle = L"Fullscreen Magnifier Sample";

// Initialize color effect matrices to apply grayscale or restore the colors on the desktop.
MAGCOLOREFFECT g_MagEffectIdentity = {1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
                                      0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
                                      0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
                                      0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
                                      0.0f,  0.0f,  0.0f,  0.0f,  1.0f};

MAGCOLOREFFECT g_MagEffectGrayscale = {0.3f,  0.3f,  0.3f,  0.0f,  0.0f,
                                       0.6f,  0.6f,  0.6f,  0.0f,  0.0f,
                                       0.1f,  0.1f,  0.1f,  0.0f,  0.0f,
                                       0.0f,  0.0f,  0.0f,  1.0f,  0.0f,
                                       0.0f,  0.0f,  0.0f,  0.0f,  1.0f};

// Forward declarations.
INT_PTR CALLBACK SampleDialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);
void InitDlg(_In_ HWND hwndDlg);
void HandleCommand(_In_ HWND hwndDlg, _In_ WORD wCtrlId);
void SetZoom(_In_ HWND hwndDlg, _In_ float fZoomFactor);
void SetColorGrayscaleState(_In_ BOOL fGrayscaleOn);
void SetInputTransform(_In_ HWND hwndDlg, _In_ BOOL fSetInputTransform);
void GetSettings(_In_ HWND hwndDlg);

//
// FUNCTION: WinMain()
//
// PURPOSE: Entry point for the application.
//
int APIENTRY WinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE /*hPrevInstance*/,
                     _In_ LPSTR     /*lpCmdLine*/,
                     _In_ int       /*nCmdShow*/)
{
    // Initialize the magnification functionality for this process.
    if (MagInitialize())
    {
        // Present a dialog box to allow the user to control fullscreen magnification.
        DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG_FULLSCREENMAGNIFICATIONCONTROL), NULL, SampleDialogProc);

        // Any current magnification and color effects will be turned off as a result of calling MagUninitialize().
        MagUninitialize();
    }
    else
    {
        MessageBox(NULL, L"Failed to initialize magnification.", g_pszAppTitle, MB_OK);
    }

    return 0;
}

//
// FUNCTION: SampleDialogProc()
//
// PURPOSE: Dialog proc for the UI used for controlling fullscreen magnification.
//
INT_PTR CALLBACK SampleDialogProc(
  _In_  HWND   hwndDlg,
  _In_  UINT   uMsg,
  _In_  WPARAM wParam,
  _In_  LPARAM /*lParam*/
)
{
    INT_PTR ipRet = 0;

    switch (uMsg)
    {
    case WM_INITDIALOG:

        InitDlg(hwndDlg);

        ipRet = 0;

        break;

    case WM_COMMAND:

        if(HIWORD(wParam) == BN_CLICKED)
        {
            WORD wCtrlId = LOWORD(wParam);

            HandleCommand(hwndDlg, wCtrlId);

            ipRet = 1;
        }

        break;

    case WM_CLOSE:

        EndDialog(hwndDlg, 0);

        break;
    }

    return ipRet;
}

//
// FUNCTION: InitDlg()
//
// PURPOSE: Initialize the sample dialog box's position and controls.
//
void InitDlg(_In_ HWND hwndDlg)
{
    // Position the dialog box in the center of the primary monitor.
    RECT rcDlg;
    GetWindowRect(hwndDlg, &rcDlg);

    int xDlg = (GetSystemMetrics(SM_CXSCREEN) - (rcDlg.right - rcDlg.left)) / 2;
    int yDlg = (GetSystemMetrics(SM_CYSCREEN) - (rcDlg.bottom - rcDlg.top)) / 2;

    SetWindowPos(hwndDlg, NULL, xDlg, yDlg, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

    // Check the "100%" radio box and move focus to it.
    HWND hwndControl = GetDlgItem(hwndDlg, IDC_RADIO_100);
    SendMessage(hwndControl, BM_SETCHECK, BST_CHECKED, 0);
    SetFocus(hwndControl);
}        

//
// FUNCTION: HandleCommand()
//
// PURPOSE: Take action in response to user action at the dialog box's controls.
//
void HandleCommand(_In_ HWND hwndDlg, _In_ WORD wCtrlId)
{
    switch (wCtrlId)
    {
    case IDC_CLOSE:
            
        // Close the sample dialog box.
        EndDialog(hwndDlg, 0);

        break;

    case IDC_RADIO_100: 
    case IDC_RADIO_200:
    case IDC_RADIO_300:
    case IDC_RADIO_400:

        // The user clicked one of the radio button to apply some fullscreen magnification. (We know the control ids are sequential here.)
        SetZoom(hwndDlg, (float)(wCtrlId - IDC_RADIO_100 + 1));

        break;

    case IDC_CHECK_SETGRAYSCALE:
    {
        // The user clicked the checkbox to apply grayscale to the colors on the screen.
        bool fGrayscaleOn = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_SETGRAYSCALE, BM_GETCHECK, 0, 0));

        SetColorGrayscaleState(fGrayscaleOn);

        break;
    }
    case IDC_CHECK_SETINPUTTRANSFORM:
    {
        // The user clicked the checkbox to apply an input transform for touch and pen input.
        bool fInputTransformOn = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_SETINPUTTRANSFORM, BM_GETCHECK, 0, 0));

        SetInputTransform(hwndDlg, fInputTransformOn);

        break;
    }
    case IDC_BUTTON_GETSETTINGS:

        // The user wants to retrieve the current magnification settings.
        GetSettings(hwndDlg);

        break;
    }
}

//
// FUNCTION: SetZoom()
//
// PURPOSE: Apply fullscreen magnification.
//
void SetZoom(_In_ HWND hwndDlg, _In_ float magnificationFactor)
{
    // Attempts to apply a magnification of less than 100% will fail.
    if (magnificationFactor >= 1.0)
    {
        // The offsets supplied to MagSetFullscreenTransform() are relative to the top left corner
        // of the primary monitor, in unmagnified coordinates. To position the top left corner of
        // some window of interest at the top left corner of the magnified view, call GetWindowRect()
        // to get the window rectangle, and pass the rectangle's left and top values as the offsets 
        // supplied to MagSetFullscreenTransform(). 

        // If the top left corner of the window of interest is to be positioned at the top left 
        // corner of the monitor furthest to the left of the primary monitor, then the top left 
        // corner of the virtual desktop would be adjusted by the current magnification, as follows:
        //   int xOffset = rcTargetWindow.left - (int)(rcVirtualDesktop.left / magnificationFactor);
        //   int yOffset = rcTargetWindow.top  - (int)(rcVirtualDesktop.top  / magnificationFactor);

        // For this sample, keep the sample's UI at the center of the magnified view on the primary monitor.
        // In order to do this, it is nececessary to adjust the offsets supplied to MagSetFullscreenTransform()
        // based on the magnification being applied.

        // Note that the calculations in this file which use GetSystemMetrics(SM_C*SCREEN) assume 
        // that the values returned from that function are unaffected by the current DPI setting. 
        // In order to ensure this, the manifest for this app declares the app to be DPI aware.

        int xDlg = (int)((float)GetSystemMetrics(SM_CXSCREEN) * (1.0 - (1.0 / magnificationFactor)) / 2.0);
        int yDlg = (int)((float)GetSystemMetrics(SM_CYSCREEN) * (1.0 - (1.0 / magnificationFactor)) / 2.0);

        BOOL fSuccess = MagSetFullscreenTransform(magnificationFactor, xDlg, yDlg);
        if (fSuccess)
        {
            // If an input transform for pen and touch is currently applied, update the transform
            // to account for the new magnification.
            BOOL fInputTransformEnabled;
            RECT rcInputTransformSource;
            RECT rcInputTransformDest;

            if (MagGetInputTransform(&fInputTransformEnabled, &rcInputTransformSource, &rcInputTransformDest))
            {
                if (fInputTransformEnabled)
                {
                    SetInputTransform(hwndDlg, fInputTransformEnabled);
                }                
            }
        }
    }
}

//
// FUNCTION: SetColorGrayscaleState()
//
// PURPOSE: Either apply grayscale to all colors on the screen, or restore the original colors.
//
void SetColorGrayscaleState(_In_ BOOL fGrayscaleOn)
{
    // Apply the color matrix required to either invert the screen colors or to show the regular colors.
    PMAGCOLOREFFECT pEffect = (fGrayscaleOn ? &g_MagEffectGrayscale : &g_MagEffectIdentity);

    MagSetFullscreenColorEffect(pEffect);
}

//
// FUNCTION: SetInputTransform()
//
// PURPOSE: Apply an input transform to allow touch and pen input to account 
//          for the current fullscreen or lens magnification.
//
void SetInputTransform(_In_ HWND hwndDlg, _In_ BOOL fSetInputTransform)
{
    bool fContinue = true;

    RECT rcSource = {0};
    RECT rcDest   = {0};
    
    // MagSetInputTransform() is used to adjust pen and touch input to account for the current magnification.
    // The "Source" and "Destination" rectangles supplied to MagSetInputTransform() are from the perspective
    // of the currently magnified visuals. The source rectangle is the portion of the screen that is 
    // currently being magnified, and the destination rectangle is the area on the screen which shows the 
    // magnified results.
     
    // If we're setting an input transform, base the transform on the current fullscreen magnification.
    if (fSetInputTransform)
    {
        // Assume here the touch and pen input is going to the primary monitor.
        rcDest.right  = GetSystemMetrics(SM_CXSCREEN);
        rcDest.bottom = GetSystemMetrics(SM_CYSCREEN);

        float magnificationFactor;
        int xOffset;
        int yOffset;

        // Get the currently active magnification.
        if (MagGetFullscreenTransform(&magnificationFactor, &xOffset, &yOffset))
        {
            // Determine the area of the screen being magnified.
            rcSource.left   = xOffset;
            rcSource.top    = yOffset;
            rcSource.right  = rcSource.left + (int)(rcDest.right / magnificationFactor);
            rcSource.bottom = rcSource.top  + (int)(rcDest.bottom / magnificationFactor);
        }        
        else
        {
            // An unexpected error occurred trying to get the current magnification.
            fContinue = false;

            DWORD dwErr = GetLastError();

            WCHAR szError[256];
            StringCchPrintf(szError, ARRAYSIZE(szError), L"Failed to get current magnification. Error was %d", dwErr);
            MessageBox(hwndDlg, szError, g_pszAppTitle, MB_OK);
        }
    }

    if (fContinue)
    {
        // Now set the input transform as required.
        if (!MagSetInputTransform(fSetInputTransform, &rcSource, &rcDest))
        {
            DWORD dwErr = GetLastError();

            // If the last error is E_ACCESSDENIED, then this may mean that the process is not running with
            // UIAccess privileges. UIAccess is required in order for MagSetInputTransform() to success.

            WCHAR szError[256];
            StringCchPrintf(szError, ARRAYSIZE(szError), L"Failed to set input transform. Error was %d", dwErr);
            MessageBox(hwndDlg, szError, g_pszAppTitle, MB_OK);
        }
    }
}

//
// FUNCTION: GetSettings()
//
// PURPOSE: Query all the related settings, and present them to the user.
//
void GetSettings(_In_ HWND hwndDlg)
{
    float  magnificationLevel;
    int    xOffset;
    int    yOffset;
    LPWSTR pszColorStatus = NULL; 
    BOOL   fInputTransformEnabled = FALSE;
    RECT   rcInputTransformSource;
    RECT   rcInputTransformDest;

    // If any unexpected errors occur trying to get the current settings, present no settings data.
    bool fSuccess = true;

    // Get the current magnification level and offset.
    if (!MagGetFullscreenTransform(&magnificationLevel, &xOffset, &yOffset))
    {
        fSuccess = false;
    }

    if (fSuccess)
    {
        // Get the current color effect.
        MAGCOLOREFFECT magEffect;
        if (MagGetFullscreenColorEffect(&magEffect))
        {
            // Present friendly text relating to the color effect.
            if (memcmp(&g_MagEffectIdentity, &magEffect, sizeof(magEffect)) == 0)
            {
                pszColorStatus = L"Identity";
            }
            else if (memcmp(&g_MagEffectGrayscale, &magEffect, sizeof(magEffect)) == 0)
            {
                pszColorStatus = L"Grayscale";
            }
            else
            {
                // This would be an unexpected result from MagGetDesktopColorEffect() 
                // given that the sample only sets the identity or grayscale effects.
                pszColorStatus = L"Unknown";
            }
        }
        else
        {
            fSuccess = false;
        }
    }

    if (fSuccess)
    {
        // Get the current input transform.
        if (!MagGetInputTransform(&fInputTransformEnabled, &rcInputTransformSource, &rcInputTransformDest))
        {
            fSuccess = false;
        }
    }

    // Present the results of all the calls above.
    WCHAR szMessage[256];

    if (fSuccess)
    {
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), 
                        L"The current settings are:\r\n\r\nMagnification level: %f\r\nColor effect: %s\r\nInput transform state: %d", 
                        magnificationLevel, pszColorStatus, fInputTransformEnabled);
    }
    else
    {
        DWORD dwErr = GetLastError();
        StringCchPrintf(szMessage, ARRAYSIZE(szMessage), L"Failed to get magnification setting. Error was %d", dwErr);
    }

    MessageBox(hwndDlg, szMessage, g_pszAppTitle, MB_OK);
}
