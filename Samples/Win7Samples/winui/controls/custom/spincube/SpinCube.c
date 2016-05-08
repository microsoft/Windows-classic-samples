/******************************************************************************\
*
*  MODULE:      SPINCUBE.C
*
*
*  PURPOSE:     To provide a generic Windows NT dynamic link library
*               sample demonstrating the use of DLL entry points, exported
*               variables, using C runtime in a DLL, etc...
*
*               This module also provides a functional example of how
*               to create a custom control library which may be used by
*               applications (i.e. SPINTEST.EXE) and the Dialog Editor.
*
*
*  FUNCTIONS:   DllMain()      - Registers spincube class when a
*                                      process loads this DLL.
*               CustomControlInfo()  - Called by DLGEDIT to initialize
*                                      a CCINFO structure(s).
*               SpincubeStyle()      - Brings up dialog box which allows
*                                      user to modify control style.
*               SpincubeSizeToText() - Called by DLGEDIT if user requests
*                                      that control be sized to fit text.
*               SpincubeWndProc()    - Window procedure for spincube
*                                      control.
*               SpincubeDlgProc()    - Procedure for control style dialog.
*
*
*  COMMMENTS:   The dialog editor interface has changed since Win 3.0.
*               Recommend browsing the NT CUSTCNTL.H file to get an
*               idea of the new interface.
*
*
*                           Microsoft Developer Support
*                         Copyright Microsoft Corporation
*
\******************************************************************************/

#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include "spincube.h"


//
// function prototype for Paint() in PAINT.C
//

void Paint (HWND);

//
// function prototype for looking up string resources
//

LPTSTR GetStringRes (int);


//
// Declared below are the module's 2 exported variables.
//
//   giNumSpincubesThisProcess is an instance variable that contains
//   the number of (existing) Spincube controls created by the
//   current process.
//
//   giNumSpincubesAllProcesses is a shared (between processes) variable
//   which contains the total number of (existing) Spincube controls
//   created by all processes in the system.
//
//

int giNumSpincubesThisProcess = 0;

#pragma data_seg(".MYSEG")

  int giNumSpincubesAllProcesses = 0;

#pragma data_seg()


//
// Some global vars for this module
//

HANDLE    ghMod;   // DLL's module handle
LPCCSTYLE gpccs;   // global pointer to a CCSTYLE structure

CCSTYLEFLAG aSpincubeStyleFlags[] = { { SS_ERASE,    0, TEXT("SS_ERASE")    },
                                      { SS_INMOTION, 0, TEXT("SS_INMOTION") } };



/******************************************************************************\
*
*  FUNCTION:    DllMain
*
*  INPUTS:      hDLL       - DLL module handle
*               dwReason   - reason being called (e.g. process attaching)
*               lpReserved - reserved
*
*  RETURNS:     TRUE if initialization passed, or
*               FALSE if initialization failed.
*
*  COMMENTS:    On DLL_PROCESS_ATTACH registers the SPINCUBECLASS
*
*               DLL initialization serialization is guaranteed within a
*               process (if multiple threads then DLL entry points are
*               serialized), but is not guaranteed across processes.
*
*               When synchronization objects are created, it is necesaary
*               to check the return code of GetLastError even if the create
*               call succeeded. If the object existed, ERROR_ALREADY_EXISTED
*               will be returned.
*
*               If your DLL uses any C runtime functions then you should
*               always call _CRT_INIT so that the C runtime can initialize
*               itself appropriately. Failure to do this may result in
*               indeterminate behavior. When the DLL entry point is called
*               for DLL_PROCESS_ATTACH & DLL_THREAD_ATTACH circumstances,
*               _CRT_INIT should be called before any other initilization
*               is performed. When the DLL entry point is called for
*               DLL_PROCESS_DETACH & DLL_THREAD_DETACH circumstances,
*               _CRT_INIT should be called after all cleanup has been
*               performed, i.e. right before the function returns.
*
\******************************************************************************/

BOOL WINAPI DllMain (HINSTANCE hDLL, DWORD dwReason, LPVOID lpReserved)
{
  ghMod = hDLL;
  switch (dwReason)
  {
    case DLL_PROCESS_ATTACH:
    {
      WNDCLASS wc;

      wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_OWNDC |
                         CS_GLOBALCLASS ;
      wc.lpfnWndProc   = (WNDPROC) SpincubeWndProc;
      wc.cbClsExtra    = 0;
      wc.cbWndExtra    = SPINCUBE_EXTRA;
      wc.hInstance     = hDLL;
      wc.hIcon         = NULL;
      wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
      wc.hbrBackground = NULL;
      wc.lpszMenuName  = (LPTSTR) NULL;
      wc.lpszClassName = (LPTSTR) SPINCUBECLASS;

      if (!RegisterClass (&wc))
      {
        MessageBox (NULL,
                    GetStringRes (IDS_REGCLASSFAIL),
                    (LPCTSTR) TEXT("SPINCUBE.DLL"),
                    MB_OK | MB_ICONEXCLAMATION);

        return FALSE;
      }

      break;
    }


    case DLL_PROCESS_DETACH:
    {

      if (!UnregisterClass ((LPTSTR) SPINCUBECLASS, hDLL ))
      {
        MessageBox (NULL,
                    GetStringRes (IDS_UNREGFAIL),
                    (LPCTSTR) TEXT("SPINCUBE.DLL"),
                    MB_OK | MB_ICONEXCLAMATION);

        return FALSE;
      }

      break;
    }


    default:

      break;
  }
  return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    CustomControlInfo
*
*  INPUTS:      acci - pointer to an array od CCINFOA structures
*
*  RETURNS:     Number of controls supported by this DLL
*
*  COMMENTS:    See CUSTCNTL.H for more info
*
\******************************************************************************/

UINT CALLBACK CustomControlInfo (LPCCINFO acci)
{
  //
  // Dlgedit is querying the number of controls this DLL supports, so return 1.
  //   Then we'll get called again with a valid "acci"
  //

  if (!acci)

    return 1;


  //
  // Fill in the constant calues.
  //

  acci[0].flOptions         = 0;
  acci[0].cxDefault         = 40;      // default width  (dialog units)
  acci[0].cyDefault         = 40;      // default height (dialog units)
  acci[0].flStyleDefault    = WS_CHILD |
                              WS_VISIBLE |
                              SS_INMOTION;
  acci[0].flExtStyleDefault = 0;
  acci[0].flCtrlTypeMask    = 0;
  acci[0].cStyleFlags       = NUM_SPINCUBE_STYLES;
  acci[0].aStyleFlags       = aSpincubeStyleFlags;
  acci[0].lpfnStyle         = SpincubeStyle;
  acci[0].lpfnSizeToText    = SpincubeSizeToText;
  acci[0].dwReserved1       = 0;
  acci[0].dwReserved2       = 0;


  //
  // Copy the strings
  //
  // NOTE: MAKE SURE THE STRINGS COPIED DO NOT EXCEED THE LENGTH OF
  //       THE BUFFERS IN THE CCINFO STRUCTURE!
  //

  _tcsncpy_s (acci[0].szClass, _countof(acci[0].szClass), SPINCUBECLASS, _TRUNCATE);
  _tcsncpy_s (acci[0].szDesc, _countof(acci[0].szDesc),  SPINCUBEDESCRIPTION, _TRUNCATE);
  _tcsncpy_s (acci[0].szTextDefault, _countof(acci[0].szTextDefault), SPINCUBEDEFAULTTEXT, _TRUNCATE);


  //
  // Return the number of controls that the DLL supports
  //

  return 1;
}



/******************************************************************************\
*
*  FUNCTION:    SpincubeStyle
*
*  INPUTS:      hWndParent - handle of parent window (dialog editor)
*               pccs       - pointer to a CCSTYLE structure
*
*  RETURNS:     TRUE  if success,
*               FALSE if error occured
*
\******************************************************************************/

BOOL CALLBACK SpincubeStyle (HWND hWndParent, LPCCSTYLE pccs)
{
  gpccs = pccs;

  if (DialogBox (ghMod, TEXT("SpincubeStyle"), hWndParent,
                       (DLGPROC)SpincubeDlgProc) == -1)
  {
    MessageBox (hWndParent,
                GetStringRes (IDS_DLGBOXFAIL),
                (LPCTSTR) TEXT("Spincube.dll"),
                MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);

    return FALSE;
  }

  return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    SpincubeSizeToText
*
*  INPUTS:      flStyle    - control style
*               flExtStyle - control extended style
*               hFont      - handle of font used to draw text
*               pszText    - control text
*
*  RETURNS:     Width (in pixels) control must be to accomodate text, or
*               -1 if an error occurs.
*
*  COMMENTS:    Just no-op here (since we never actually display text in
*               the control it doesn't need to be resized).
*
\******************************************************************************/

INT CALLBACK SpincubeSizeToText (DWORD flStyle, DWORD flExtStyle,
                                 HFONT hFont,   LPTSTR pszText)
{
  return -1;
}



/******************************************************************************\
*
*  FUNCTION:    SpincubeWndProc (standard window procedure INPUTS/RETURNS)
*
*  COMMENTS:    This is the window procedure for our custom control. At
*               creation we alloc a SPINCUBEINFO struct, initialize it,
*               and associate it with this particular control. We also
*               start a timer which will invalidate the window every so
*               often; this causes a repaint, and the cube gets drawn in
*               a new position. Left button clicks will turn toggle the
*               erase option, causing a "trail" of cubes to be left when
*               off. Right button clicks will toggle the motion state of
*               the control (and turn the timer on/off).
*
\******************************************************************************/

LRESULT CALLBACK SpincubeWndProc (HWND hwnd, UINT msg, WPARAM wParam,
                                  LPARAM lParam)
{
  switch (msg)
  {
    case WM_CREATE:
    {
      //
      // Alloc & init a SPINCUBEINFO struct for this particular control
      //

      HDC            hdc;
      LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
      PSPINCUBEINFO  pSCI = (PSPINCUBEINFO) LocalAlloc (LPTR,
                                                        sizeof(SPINCUBEINFO));
      if (!pSCI)
      {
        MessageBox (NULL,
                    GetStringRes (IDS_ALLOCFAIL),
                    (LPCTSTR) TEXT("SPINCUBE.DLL"),
                    MB_OK | MB_ICONEXCLAMATION);
        return -1;
      }


      //
      // Alloc the compatible DC for this control.
      //

      hdc = GetDC (hwnd);

      if ((pSCI->hdcCompat = CreateCompatibleDC (hdc)) == NULL)
      {
        MessageBox (NULL,
                    GetStringRes (IDS_CREATEDCFAIL),
                    (LPCTSTR) TEXT("SPINCUBE.DLL"),
                    MB_OK | MB_ICONEXCLAMATION);
        return -1;
      }

      ReleaseDC (hwnd, hdc);


      //
      // Initialize this instance structure
      //

      pSCI->fCurrentXRotation =
      pSCI->fCurrentYRotation =
      pSCI->fCurrentZRotation = (float) 0.0;

      pSCI->fCurrentXRotationInc =
      pSCI->fCurrentYRotationInc =
      pSCI->fCurrentZRotationInc = (float) 0.2617; // a random # (15 degrees)

      pSCI->iCurrentXTranslation =
      pSCI->iCurrentYTranslation =
      pSCI->iCurrentZTranslation = 0;

      //
      // All these calculations so the cubes start out with random movements.
      //

      if ((pSCI->iCurrentXTranslationInc = (rand() % 10) + 2) > 7)

        pSCI->iCurrentXTranslationInc = -pSCI->iCurrentXTranslationInc;

      if ((pSCI->iCurrentYTranslationInc = (rand() % 10) + 2) <= 7)

        pSCI->iCurrentYTranslationInc = -pSCI->iCurrentYTranslationInc;

      if ((pSCI->iCurrentZTranslationInc = (rand() % 10) + 2) > 7)

        pSCI->iCurrentZTranslationInc = -pSCI->iCurrentZTranslationInc;

      pSCI->rcCubeBoundary.left   =
      pSCI->rcCubeBoundary.top    = 0;
      pSCI->rcCubeBoundary.right  = lpcs->cx;
      pSCI->rcCubeBoundary.bottom = lpcs->cy;

      pSCI->iOptions  = SPINCUBE_REPAINT_BKGND;
      pSCI->hbmCompat = NULL;

      SetWindowLongPtr (hwnd, GWL_SPINCUBEDATA, (LONG_PTR) pSCI);

      SetTimer (hwnd, SPIN_EVENT, SPIN_INTERVAL, NULL);

      //
      // Increment the count vars
      //

      giNumSpincubesThisProcess++;
      giNumSpincubesAllProcesses++;

      break;
    }

    case WM_PAINT:

      Paint (hwnd);
      break;

    case WM_TIMER:

      switch (wParam)
      {
        case SPIN_EVENT:
        {
          PSPINCUBEINFO pSCI = (PSPINCUBEINFO) GetWindowLongPtr (hwnd,
                                                              GWL_SPINCUBEDATA);

          InvalidateRect (hwnd, &pSCI->rcCubeBoundary, FALSE);

          break;
        }
      }

      break;

    case WM_LBUTTONDBLCLK:
    {
      //
      // Toggle the erase state of the control
      //

      if (DO_ERASE(hwnd))

        SetWindowLongPtr (hwnd, GWL_STYLE,
                       GetWindowLongPtr (hwnd, GWL_STYLE) & ~SS_ERASE);


      else
      {
        //
        // Repaint the entire control to get rid of the (cube trails) mess
        //

        PSPINCUBEINFO pSCI = (PSPINCUBEINFO) GetWindowLongPtr (hwnd,
                                                            GWL_SPINCUBEDATA);

        SetWindowLongPtr (hwnd, GWL_STYLE,
                       GetWindowLongPtr (hwnd, GWL_STYLE) | SS_ERASE);
        pSCI->iOptions |= SPINCUBE_REPAINT_BKGND;
        InvalidateRect (hwnd, NULL, FALSE);
        SendMessage (hwnd, WM_PAINT, 0, 0);
      }
      break;
    }

    case WM_RBUTTONDBLCLK:
    {
      //
      // Toggle the motion state of the control
      //

      if (IN_MOTION(hwnd))
      {
        KillTimer (hwnd, SPIN_EVENT);
        SetWindowLongPtr (hwnd, GWL_STYLE,
                       GetWindowLongPtr (hwnd, GWL_STYLE) & ~SS_INMOTION);
      }
      else
      {
        SetTimer (hwnd, SPIN_EVENT, SPIN_INTERVAL, NULL);
        SetWindowLongPtr (hwnd, GWL_STYLE,
                       GetWindowLongPtr (hwnd, GWL_STYLE) | SS_INMOTION);
      }

      break;
    }

    case WM_SIZE:

      if (wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
      {
        PSPINCUBEINFO pSCI = (PSPINCUBEINFO) GetWindowLongPtr (hwnd,
                                                            GWL_SPINCUBEDATA);
        //
        // Get a new bitmap which is the new size of our window
        //

        HDC hdc = GetDC (hwnd);
        HBITMAP hbmTemp = CreateCompatibleBitmap (hdc,
                                                  (int) LOWORD (lParam),
                                                  (int) HIWORD (lParam));
        if (!hbmTemp)
        {
          //
          // Scream, yell, & committ an untimely demise...
          //

          MessageBox (NULL,
                      GetStringRes (IDS_CREATEBITMAPFAIL),
                      (LPCTSTR) TEXT("SPINCUBE.DLL"),
                      MB_OK | MB_ICONEXCLAMATION);
          DestroyWindow (hwnd);
        }

        pSCI->hbmSave = SelectObject (pSCI->hdcCompat, hbmTemp);
        if (pSCI->hbmCompat)
        	DeleteObject (pSCI->hbmCompat);
        ReleaseDC    (hwnd, hdc);
        pSCI->hbmCompat = hbmTemp;


        //
        // Reset the translation so the cube doesn't go spinning off into
        //   space somewhere- we'd never see it again!
        //

        pSCI->iCurrentXTranslation =
        pSCI->iCurrentYTranslation =
        pSCI->iCurrentZTranslation = 0;

        //
        // All these calculations so the cube starts out with random movements,
        //

        if ((pSCI->iCurrentXTranslationInc = (rand() % 10) + 2) > 7)

          pSCI->iCurrentXTranslationInc = -pSCI->iCurrentXTranslationInc;

        if ((pSCI->iCurrentYTranslationInc = (rand() % 10) + 2) <= 7)

          pSCI->iCurrentYTranslationInc = -pSCI->iCurrentYTranslationInc;

        if ((pSCI->iCurrentZTranslationInc = (rand() % 10) + 2) > 7)

          pSCI->iCurrentZTranslationInc = -pSCI->iCurrentZTranslationInc;

        pSCI->rcCubeBoundary.left   =
        pSCI->rcCubeBoundary.top    = 0;
        pSCI->rcCubeBoundary.right  = (int) LOWORD (lParam);
        pSCI->rcCubeBoundary.bottom = (int) HIWORD (lParam);

        pSCI->iOptions |= SPINCUBE_REPAINT_BKGND;

        InvalidateRect (hwnd, NULL, FALSE);
      }

      break;

    case WM_DESTROY:
    {
      PSPINCUBEINFO pSCI = (PSPINCUBEINFO) GetWindowLongPtr (hwnd,
                                                          GWL_SPINCUBEDATA);
      //
      // Clean up all the resources used for this control
      //

      if (IN_MOTION(hwnd))

        KillTimer (hwnd, SPIN_EVENT);

      SelectObject (pSCI->hdcCompat, pSCI->hbmSave);
      DeleteObject (pSCI->hbmCompat);
      DeleteDC     (pSCI->hdcCompat);

      LocalFree (LocalHandle ((LPVOID) pSCI));


      //
      // Decrement the global count vars
      //

      giNumSpincubesThisProcess--;
      giNumSpincubesAllProcesses--;

      break;
    }

    default:

      return (DefWindowProc(hwnd, msg, wParam, lParam));
  }

  return ((LONG) TRUE);
}



/******************************************************************************\
*
*  FUNCTION:    SpincubeDlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    This dialog comes up in response to a user requesting to
*               modify the control style. This sample allows for changing
*               the control's text, and this is done by modifying the
*               CCSTYLE structure pointed at by "gpccs" (a pointer
*               that was passed to us by dlgedit).
*
\******************************************************************************/

LRESULT CALLBACK SpincubeDlgProc (HWND hDlg, UINT msg, WPARAM wParam,
                                  LPARAM lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG :
    {
      if (gpccs->flStyle & SS_ERASE)

        CheckDlgButton (hDlg, DID_ERASE, 1);

      if (gpccs->flStyle & SS_INMOTION)

        CheckDlgButton (hDlg, DID_INMOTION, 1);

      break;
    }

    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        case DID_ERASE:

          if (IsDlgButtonChecked (hDlg, DID_ERASE))

            gpccs->flStyle |= SS_ERASE;

          else

            gpccs->flStyle &= ~SS_ERASE;

          break;

        case DID_INMOTION:

          if (IsDlgButtonChecked (hDlg, DID_INMOTION))

            gpccs->flStyle |= SS_INMOTION;

          else

            gpccs->flStyle &= ~SS_INMOTION;

          break;

        case DID_OK:

          EndDialog  (hDlg, 1);
          break;
      }
      break;
  }
  return FALSE;
}





/******************************************************************************\
*
*  FUNCTION:    GetStringRes (int id INPUT ONLY)
*
*  COMMENTS:    Load the resource string with the ID given, and return a
*               pointer to it.  Notice that the buffer is common memory so
*               the string must be used before this call is made a second time.
*
\******************************************************************************/

LPTSTR   GetStringRes (int id)
{
  static TCHAR szBuffer[MAX_PATH];

  szBuffer[0]=0;
  LoadString (GetModuleHandle (NULL), id, szBuffer, MAX_PATH);
  return szBuffer;
}
