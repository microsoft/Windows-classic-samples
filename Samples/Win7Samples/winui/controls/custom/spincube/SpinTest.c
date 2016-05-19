/******************************************************************************\
*
*  PROGRAM:     SPINTEST.C
*
*  PURPOSE:     Demonstrates the use of the SPINCUBE custom control.
*
*  FUNCTIONS:   WinMain        - standard stuff; also loads the
*                                  SPINCUBE.DLL and creates a couple
*                                  of spincube controls.
*               MainWndProc    - generic window procedure.
*               SpintestDlgProc- generic dialog procedure.
*               AboutDlgProc   - processes about dialog messages
*
*                           Microsoft Developer Support
*                  Copyright 1992 - 2000 Microsoft Corporation
*
\******************************************************************************/

#include <windows.h>
#include <stdio.h>
#include "spintest.h"

//
// The exported variables from SPINCUBE.C.
//
//   Although pointers to these vars are actually exported,
//    the compiler will take care of that for us.
//

extern int __declspec(dllimport) giNumSpincubesThisProcess;
extern int __declspec(dllimport) giNumSpincubesAllProcesses;


//
// function prototype for looking up string resources
//

LPTSTR GetStringRes (int);


/******************************************************************************\
*
*  FUNCTION:    WinMain (standard WinMain INPUTS/RETURNS)
*
\******************************************************************************/

int WINAPI WinMain (HINSTANCE hInstance,HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                   int     nCmdShow)
{
  WNDCLASS wc;
  HWND   hwnd;
  MSG    msg;
  RECT   rect;
  WORD   i;

  wc.style         = 0;
  wc.lpfnWndProc   = (WNDPROC) MainWndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInstance;
  wc.hIcon         = LoadIcon (hInstance, TEXT("spintesticon"));
  wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wc.hbrBackground = GetStockObject (WHITE_BRUSH);
  wc.lpszMenuName  = (LPTSTR) TEXT("Menu");
  wc.lpszClassName = (LPTSTR) TEXT("Main");

  if (!RegisterClass (&wc))
  {
    MessageBox (NULL,
                GetStringRes (IDS_REGCLASSFAIL),
                TEXT("SPINTEST"), MB_OK | MB_ICONEXCLAMATION);
    return(FALSE);
  }

  ghInst = hInstance;
  if (!(hwnd = CreateWindow (TEXT("Main"),
                             GetStringRes (IDS_WINDOWTITLE),
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             NULL, NULL, ghInst, NULL)))
    return 0;


  //
  // Create a couple of SpinCube custom controls, we'll size them later in
  //   the WM_SIZE message handler
  //

  for (i = 0; i < 4; i++)

    gahwndSpin[i] = CreateWindow (TEXT("Spincube"), TEXT(""),
                                  WS_VISIBLE | WS_CHILD |
                                  SS_INMOTION | SS_ERASE,
                                  0, 0, 0, 0, hwnd, NULL, NULL, NULL);


  //
  // Delete the SS_ERASE to the 1st & 4th controls so we get the
  //   trailing cubes effect.
  //

  SetWindowLongPtr (gahwndSpin[0], GWL_STYLE,
                 GetWindowLongPtr (gahwndSpin[0], GWL_STYLE) & ~ SS_ERASE);
  SetWindowLongPtr (gahwndSpin[3], GWL_STYLE,
                 GetWindowLongPtr (gahwndSpin[3], GWL_STYLE) & ~ SS_ERASE);


  //
  // Send ourself a WM_SIZE so the controls will get sized appropriately
  //

  GetClientRect (hwnd, &rect);
  SendMessage (hwnd, WM_SIZE, 0,
               MAKELONG((WORD)rect.right,(WORD)rect.bottom));

  ShowWindow (hwnd, nCmdShow);

  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage  (&msg);
  }

  return (int)(msg.wParam);
}



/******************************************************************************\
*
*  FUNCTION:    MainWndProc (standard window procedure INPUTS/RETURNS)
*
\******************************************************************************/

LRESULT CALLBACK MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_COMMAND:

      switch (LOWORD(wParam))
      {
        case IDM_DLGEDITDIALOG:

          DialogBox (ghInst, (LPCTSTR) TEXT("SpintestDlg"), hwnd, (DLGPROC) DlgProc);
          break;

        case IDM_SPINTESTSTATS:

          DialogBox (ghInst, (LPCTSTR) TEXT("Stats"), hwnd, (DLGPROC) DlgProc);
          break;

        case IDM_ABOUT:

          DialogBox (ghInst, (LPCTSTR) TEXT("About"), hwnd, (DLGPROC) DlgProc);
          break;

      }
      break;

    case WM_SIZE:
    {
      //
      // Resize the controls such that each cover half the client area
      //   (plus a little border).
      //

      int width  = (int) LOWORD(lParam);
      int height = (int) HIWORD(lParam);

      SetWindowPos (gahwndSpin[0], NULL,
                    BORDER, BORDER,
                    width/2 - BORDER, height/2 - BORDER,
                    SWP_SHOWWINDOW);
      SetWindowPos (gahwndSpin[1], NULL,
                    width/2 + BORDER, BORDER,
                    width/2 - 2*BORDER, height/2 - BORDER,
                    SWP_SHOWWINDOW);
      SetWindowPos (gahwndSpin[2], NULL,
                    BORDER, height/2 + BORDER,
                    width/2 - BORDER, height/2 - 2*BORDER,
                    SWP_SHOWWINDOW);
      SetWindowPos (gahwndSpin[3], NULL,
                    width/2 + BORDER, height/2 + BORDER,
                    width/2 - 2*BORDER, height/2 - 2*BORDER,
                    SWP_SHOWWINDOW);
      break;
    }

    case WM_DESTROY:

      PostQuitMessage (0);
      break;

    default:

      return (DefWindowProc (hwnd, msg, wParam, lParam));
  }
  return 0;
}



/******************************************************************************\
*
*  FUNCTION:    DlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    Our common dlg proc (why have 3 that do the same thing???)
*
\******************************************************************************/

LRESULT CALLBACK DlgProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_INITDIALOG:

      //
      // If this dlg the "Stats" dlg fill in the appropriate fields.
      //   If not these calls will just fail.
      //
      // If the references to the giNum* vars are commented out &
      //   the program gets rebuilt don't be surprised if no spincubes
      //   appear- since no references to spincube.lib the linker will
      //   infer that it is not needed, & will not cause it to get
      //   loaded. You'll need to make a call to LoadLibrary ("SPINCUBE.DLL")
      //   prior to calling CreateWindow ("SPINCUBE"...).
      //

      SetDlgItemInt (hwnd, 500, giNumSpincubesThisProcess, TRUE);
      SetDlgItemInt (hwnd, 501, giNumSpincubesAllProcesses, TRUE);
      return (TRUE);

    case WM_COMMAND:

      if (LOWORD(wParam) == IDOK)

        EndDialog (hwnd, TRUE);

      return (TRUE);
  }
  return (FALSE);
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
