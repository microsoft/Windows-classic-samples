
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/******************************************************************************\
*
*  PROGRAM:     PRINTER.C
*
*  PURPOSE:     This is a sample application demostrating some of the new
*               printing functionality in Windows NT. This app allows the
*               user to select between various GDI graphics primitives,
*               to choose pen & brush colors, styles, and sizes, and to
*               the print these graphics to a printer. Also, the app
*               provides the user the ability to query information (reso-
*               lution, etc.) about the various printers & drivers by
*               making calls to GetDeviceCaps, etc.
*
*               Functionality for PRINTER is split into six different
*               modules as follows:
*
*                 printer.c - main event loop
*                             main window procedure
*                             about & abort dialog procedures
*                             printing thread
*
*                 paint.c   - handles all painting printers & most painting
*                             to window
*
*                 enumprt.c - manages the display of information returned
*                             from calling EnumPrinters, EnumPrinterDrivers
*
*                 devcapsx.c- manages the display of information returned
*                             from calling DeviceCapabilitiesEx
*
*                 getpdriv.c- manages the display of information returned
*                             from calling GetPrinterDriver
*
*                 getcaps.c - manages the display of information returned
*                             from calling GetDeviceCaps
*
*  FUNCTIONS:   WinMain               - initialization, create window, msg loop
*               MainWndProc           - processes main window msgs
*               AboutDlgProc          - processes About dlg msgs
*               InvalidateClient      - invalidates graphics part of client wnd
*               RefreshPrinterCombobox- updates list of printers
*               PrintThread           - printing done here
*               AbortProc             - msg loop for abort
*               AbortDlgProc          - processes abort dialog messages
*
\******************************************************************************/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // for _mbstrlen
#include <winspool.h>
#include <commdlg.h>
#include "common.h"
#include "resource.h"
#include "printer.h"

// for internationalization
#define My_mbslen _mbstrlen

/******************************************************************************\
*
*  FUNCTION:    WinMain (standard WinMain INPUTS/RETURNS)
*
*  COMMENTS:    Register & create main window, loop for messages
*
\******************************************************************************/

int WINAPI WinMain (HINSTANCE ghInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine,
                    int    nCmdShow)
{
  MSG msg;

  ghInst = ghInstance;

  if (!hPrevInstance)
  {
    WNDCLASS wc;

    wc.style         = MAIN_CLASS_STYLE;
    wc.lpfnWndProc   = (WNDPROC) MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ghInst;
    wc.hIcon         = LoadIcon (ghInst, MAKEINTRESOURCE(MAIN_ICON));
    wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = (LPCSTR) MAIN_MENU_NAME;
    wc.lpszClassName = (LPCSTR) MAIN_CLASS_NAME;

    if (!RegisterClass (&wc))
    {
      MessageBox (NULL, "WinMain(): RegisterClass() failed",
                  GetStringRes2(ERR_MOD_NAME), MB_OK | MB_ICONHAND);

      return FALSE;
    }
  }

  if (!(ghwndMain = CreateWindow ((LPCSTR) MAIN_CLASS_NAME,
                                  (LPCSTR) GetStringRes(MAIN_WND_TITLE),
                                  MAIN_WND_STYLE,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  CW_USEDEFAULT, CW_USEDEFAULT,
                                  NULL, NULL, ghInst, NULL)))
    return FALSE;

  ShowWindow (ghwndMain, nCmdShow);

  while (GetMessage (&msg, NULL, 0, 0))
  {
    TranslateMessage (&msg);
    DispatchMessage  (&msg);
  }

  return (int)msg.wParam;
}



/******************************************************************************\
*
*  FUNCTION:    MainWndProc (standard window procedure INPUTS/RETURNS)
*
*  COMMENTS:    Handles main app window msg processing
*
\******************************************************************************/

LRESULT CALLBACK MainWndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static HMENU hMappingModesSubMenu;
  static HMENU hGraphicsSubMenu;
  static HMENU hPenWidthSubMenu;
  static HMENU hPenStyleSubMenu;
  static HMENU hBrushStyleSubMenu;
  static HWND  hwndCombobox;
  static int   iComboboxWidth;
  static LONG  lTextHeight;

  int i;

  switch (msg)
  {
    case WM_COMMAND:

      switch (LOWORD (wParam))
      {
        case IDM_PRINT:
        case IDM_PRINTDLG:
        {
          DWORD  threadId;
		  HANDLE hThread;

          if (!(hThread = CreateThread (NULL, 0, 
			                 (LPTHREAD_START_ROUTINE) PrintThread,
                             (LPVOID) wParam, 0, &threadId)))
            MessageBox (hwnd,
                        "MainWndProc(): Error creating print thread",
                        GetStringRes2(ERR_MOD_NAME), MB_OK | MB_ICONHAND);
		  else
			  CloseHandle(hThread);
          break;
        }

        case IDM_GETDEVICECAPS:

          DialogBox (ghInst, (LPCTSTR) "List", hwnd,
                     (DLGPROC) GetDeviceCapsDlgProc);
          break;

        case IDM_ENUMPRINTERS:

          DialogBox (ghInst, (LPCTSTR) "List", hwnd,
                     (DLGPROC) EnumPrintersDlgProc);
          break;

        case IDM_GETPRINTERDRIVER:

          if (strcmp (gszDeviceName, "Display"))

            DialogBox (ghInst, (LPCTSTR) "List", hwnd,
                       (DLGPROC) GetPrinterDriverDlgProc);
          else

            MessageBox (hwnd, (LPCTSTR) GetStringRes(IDS_ASKSELPRT),
                        (LPCTSTR) "PRINTER.EXE:", MB_OK);

          break;

        case IDM_ENUMPRINTERDRIVERS:

          DialogBox (ghInst, (LPCTSTR) "List", hwnd,
                     (DLGPROC) EnumPrinterDriversDlgProc);
          break;

        case IDM_REFRESH:

          RefreshPrinterCombobox (hwndCombobox);
          break;

        case IDM_ABOUT:

          DialogBox (ghInst, (LPCTSTR) "About", hwnd, (DLGPROC) AboutDlgProc);
          break;

		case IDM_EXIT:
			DestroyWindow (hwnd);
			break;

        case IDM_HIENGLISH:
        case IDM_HIMETRIC:
        case IDM_LOENGLISH:
        case IDM_LOMETRIC:
        case IDM_TWIPS:
        case IDM_ISOTROPIC:
        case IDM_ANISOTROPIC:
        case IDM_TEXT:

          //
          // Uncheck the last map mode menuitem, check the new map mode
          //   menuitem, set iMappingMode according to menu id, cause a
          //   repaint
          //

          for (i = 0; i < MAX_MAP_MODES; i++)

            if (giMapMode == gaMMLookup[i].iMapMode)
            {
              CheckMenuItem (hMappingModesSubMenu, gaMMLookup[i].wMenuItem,
                             MF_UNCHECKED | MF_BYCOMMAND);
              break;
            }

          CheckMenuItem (hMappingModesSubMenu, LOWORD (wParam),
                         MF_CHECKED | MF_BYCOMMAND);

          for (i = 0; i < MAX_MAP_MODES; i++)

            if (LOWORD (wParam) == gaMMLookup[i].wMenuItem)
            {
              giMapMode = gaMMLookup[i].iMapMode;
              break;
            }

          //
          // invalidate the entire client so toolbar text gets updated
          //

          InvalidateRect (hwnd, NULL, TRUE);
          break;

        case IDM_ARC:
        case IDM_ELLIPSE:
        case IDM_LINETO:
        case IDM_PIE:
        case IDM_PLGBLT:
        case IDM_POLYBEZIER:
        case IDM_POLYGON:
        case IDM_POLYLINE:
        case IDM_POLYPOLYGON:
        case IDM_RECTANGLE:
        case IDM_ROUNDRECT:
        case IDM_STRETCHBLT:
        {
          //
          // Retrieve the DWORD flag value for the particular menuitem,
          //   toggle (un/check) the menuitem, set/clear the flag in
          //   gdwGraphicsOptions, cause a repaint
          //

          DWORD dwGraphic;

          for (i = 0; i < MAX_GRAPHICS; i++)

            if (LOWORD (wParam) == gaGraphicLookup[i].wMenuItem)
            {
              dwGraphic = gaGraphicLookup[i].dwGraphic;
              break;
            }

          if (GetMenuState (hGraphicsSubMenu, LOWORD(wParam), MF_BYCOMMAND)
              & MF_CHECKED)

          {
            gdwGraphicsOptions &= ~dwGraphic;
            CheckMenuItem (hGraphicsSubMenu, LOWORD(wParam),
                            MF_UNCHECKED | MF_BYCOMMAND);
          }
          else
          {
            //
            // Clear/uncheck the ENUMFONTS flag/menuitem
            //

            gdwGraphicsOptions &= ~ENUMFONTS;
            CheckMenuItem (hGraphicsSubMenu, IDM_ENUMFONTS,
                           MF_UNCHECKED | MF_BYCOMMAND);

            gdwGraphicsOptions |= dwGraphic;
            CheckMenuItem (hGraphicsSubMenu, LOWORD(wParam),
                            MF_CHECKED | MF_BYCOMMAND);
          }
          InvalidateClient ();
          break;
        }

        case IDM_ALLGRAPHICS:

          //
          // Clear/uncheck the ENUMFONTS flag/menuitem, set/check all
          //   other graphics flags/menuitems, cause a repaint
          //

          CheckMenuItem (hGraphicsSubMenu, IDM_ENUMFONTS,
                          MF_UNCHECKED | MF_BYCOMMAND);

          for (i = 0; i < MAX_GRAPHICS; i++)

            CheckMenuItem (hGraphicsSubMenu, IDM_ARC + i,
                           MF_CHECKED | MF_BYCOMMAND);

          gdwGraphicsOptions = ALLGRAPHICS | (gdwGraphicsOptions & DRAWAXIS);

          InvalidateClient ();
          break;

        case IDM_NOGRAPHICS:

          //
          // Clear/uncheck all graphics flags/menuitems, cause a repaint
          //

          for (i = 0; i < MAX_GRAPHICS; i++)

            CheckMenuItem (hGraphicsSubMenu, IDM_ARC + i,
                           MF_UNCHECKED | MF_BYCOMMAND);

          gdwGraphicsOptions &= (DRAWAXIS | ENUMFONTS);

          InvalidateClient ();
          break;

        case IDM_ENUMFONTS:

          //
          // Set/clear ENUMFONTS flag, toggle (un/check) menuitem, if
          //   checking IDM_ENUMFONTS then uncheck all other items,
          //   cause a repaint
          //

          if (GetMenuState (hGraphicsSubMenu, IDM_ENUMFONTS, MF_BYCOMMAND)
                & MF_CHECKED)
          {
            gdwGraphicsOptions &= DRAWAXIS;

            CheckMenuItem (hGraphicsSubMenu, IDM_ENUMFONTS,
                           MF_UNCHECKED | MF_BYCOMMAND);
          }

          else
          {
            SendMessage (hwnd, WM_COMMAND, IDM_NOGRAPHICS, 0);

            gdwGraphicsOptions = ENUMFONTS | (gdwGraphicsOptions & DRAWAXIS);

            CheckMenuItem (hGraphicsSubMenu, IDM_ENUMFONTS,
                           MF_CHECKED | MF_BYCOMMAND);
          }
          InvalidateClient ();
          break;

        case IDM_DRAWAXIS:

          //
          // Set/clear DRAWAXIS flag, toggle (un/check) menuitem,
          //   cause a repaint
          //

          if (GetMenuState (hGraphicsSubMenu, IDM_DRAWAXIS, MF_BYCOMMAND)
                & MF_CHECKED)
          {
            gdwGraphicsOptions &= ~DRAWAXIS;

            CheckMenuItem (hGraphicsSubMenu, IDM_DRAWAXIS,
                           MF_UNCHECKED | MF_BYCOMMAND);
          }

          else
          {
            gdwGraphicsOptions |= DRAWAXIS;

            CheckMenuItem (hGraphicsSubMenu, IDM_DRAWAXIS,
                           MF_CHECKED | MF_BYCOMMAND);
          }
          InvalidateClient ();
          break;

        case IDM_SETPENCOLOR:
        case IDM_SETBRUSHCOLOR:
        case IDM_TEXTCOLOR:
        {
          CHOOSECOLOR cc;

          static DWORD adwCustColors[16];

          memset ((void *) &cc, 0, sizeof (CHOOSECOLOR));

          cc.lStructSize  = sizeof (CHOOSECOLOR);
          cc.hwndOwner    = hwnd;
          cc.Flags        = CC_RGBINIT;
          cc.lpCustColors = adwCustColors;

          if (LOWORD (wParam) == IDM_SETPENCOLOR)

            cc.rgbResult = gdwPenColor;

          else if (LOWORD (wParam) == IDM_SETBRUSHCOLOR)

            cc.rgbResult = gdwBrushColor;

          else

            cc.rgbResult = gdwTextColor;

          //
          // bring up choose color common dialog
          //

          ChooseColor (&cc);

          if (LOWORD (wParam) == IDM_SETPENCOLOR)

            gdwPenColor   = cc.rgbResult;

          else if (LOWORD (wParam) == IDM_SETBRUSHCOLOR)

            gdwBrushColor = cc.rgbResult;

          else

            gdwTextColor  = cc.rgbResult;

          InvalidateClient ();
          break;
        }

        case IDM_PENWIDTH_1:
        case IDM_PENWIDTH_2:
        case IDM_PENWIDTH_3:
        case IDM_PENWIDTH_4:
        case IDM_PENWIDTH_5:
        case IDM_PENWIDTH_6:
        case IDM_PENWIDTH_7:
        case IDM_PENWIDTH_8:

          //
          // uncheck old pen width menuitem, check new one, cause a repaint
          //

          for (i = 0; i < MAX_PENWIDTHS; i++)

            if (giPenWidth == gaPenWidths[i].iPenWidth)
            {
              CheckMenuItem (hPenWidthSubMenu, gaPenWidths[i].wMenuItem,
                             MF_UNCHECKED | MF_BYCOMMAND);
              break;
            }

          for (i = 0; i < MAX_PENWIDTHS; i++)

            if (LOWORD(wParam) == gaPenWidths[i].wMenuItem)
            {
              CheckMenuItem (hPenWidthSubMenu, gaPenWidths[i].wMenuItem,
                             MF_CHECKED | MF_BYCOMMAND);

              giPenWidth = gaPenWidths[i].iPenWidth;

              break;
            }

          InvalidateClient ();
          break;

        case IDM_PENCOLOR_SOLID:
        case IDM_PENCOLOR_DASH:
        case IDM_PENCOLOR_DOT:
        case IDM_PENCOLOR_DASHDOT:
        case IDM_PENCOLOR_DASHDOTDOT:
        case IDM_PENCOLOR_NULL:
        case IDM_PENCOLOR_INSIDEFRAME:

          //
          // uncheck old pen style menuitem, check new one, cause a repaint
          //

          for (i = 0; i < MAX_PENSTYLES; i++)

            if (giPenStyle == gaPenStyles[i].iPenStyle)
            {
              CheckMenuItem (hPenStyleSubMenu, gaPenStyles[i].wMenuItem,
                             MF_UNCHECKED | MF_BYCOMMAND);
              break;
            }

          for (i = 0; i < MAX_PENSTYLES; i++)

            if (LOWORD(wParam) == gaPenStyles[i].wMenuItem)
            {
              CheckMenuItem (hPenStyleSubMenu, gaPenStyles[i].wMenuItem,
                             MF_CHECKED | MF_BYCOMMAND);

              giPenStyle = gaPenStyles[i].iPenStyle;

              break;
            }

          InvalidateClient ();
          break;

        case IDM_BRUSHSTYLE_HORIZONTAL:
        case IDM_BRUSHSTYLE_VERTICAL:
        case IDM_BRUSHSTYLE_FDIAGONAL:
        case IDM_BRUSHSTYLE_BDIAGONAL:
        case IDM_BRUSHSTYLE_CROSS:
        case IDM_BRUSHSTYLE_DIAGCROSS:
        
          	for (i = 0; i < MAX_BRUSHSTYLES; i++) {
				
				// Uncheck the old option
				if (giBrushStyle == gaBrushStyles[i].iBrushStyle) {
					CheckMenuItem (hBrushStyleSubMenu, gaBrushStyles[i].wMenuItem,
								   MF_UNCHECKED | MF_BYCOMMAND);
				}

				// Check the new option
				if (LOWORD(wParam) == gaBrushStyles[i].wMenuItem) {

					CheckMenuItem (hBrushStyleSubMenu, gaBrushStyles[i].wMenuItem,
								   MF_CHECKED | MF_BYCOMMAND);
					
					giBrushStyle = gaBrushStyles[i].iBrushStyle;
				}
			}
			
          InvalidateClient ();
          break;

        case ID_COMBOBOX:

          switch (HIWORD(wParam))
          {
            case CBN_SELCHANGE:
            {
              DWORD dwIndex;
              char  buf[BUFSIZE];

              //
              // User clicked on one of the items in the toolbar combobox;
              //   figure out which item, then parse the text apart and
              //   copy it to the gszDriverName, gszDeviceName, and gszPort
              //   variables.
              //

              dwIndex = (DWORD) SendMessage ((HWND) lParam,
                                             CB_GETCURSEL, 0, 0);
              SendMessage ((HWND) lParam, CB_GETLBTEXT, dwIndex,
                           (LONG_PTR) buf);

              if (!strcmp (buf, "Display"))
              {
                strncpy_s (gszDeviceName, _countof(gszDeviceName), "Display", _TRUNCATE);

                gszPort[0]       =
                gszDriverName[0] = '\0';
              }
              else
              {
                LPSTR   lpszSrc;
                LPSTR   lpszDst;

                for (lpszSrc = buf, lpszDst = gszDeviceName;
                    *lpszSrc && *lpszSrc != ';';    ) {
                    if (IsDBCSLeadByte(*lpszSrc)) {
                        *lpszDst++ = *lpszSrc++;
                    }
                    *lpszDst++ = *lpszSrc++;
                }
                *lpszDst = '\0';

                for (lpszSrc++, lpszDst = gszPort;
                    *lpszSrc && *lpszSrc != ';';    ) {
                    if (IsDBCSLeadByte(*lpszSrc)) {
                        *lpszDst++ = *lpszSrc++;
                    }
                    *lpszDst++ = *lpszSrc++;
                }
                *lpszDst = '\0';

                for (lpszSrc++, lpszDst = gszDriverName; *lpszSrc;    ) {
                    if (IsDBCSLeadByte(*lpszSrc)) {
                        *lpszDst++ = *lpszSrc++;
                    }
                    *lpszDst++ = *lpszSrc++;
                }
                *lpszDst = '\0';
              }
              break;
            }
          }
          break;
      }
      break;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      RECT        rect;
      HRGN        hrgn;
      HPEN        hpen, hpenSave;
      HBRUSH      hbr;
      char        buf[BUFSIZE];
      POINT       p;

      BeginPaint (hwnd, &ps);

      //
      // paint 3d toolbar background & client size text
      //

      GetClientRect (hwnd, &rect);
      rect.bottom = 2*glcyMenu;
      FillRect (ps.hdc, &rect, GetStockObject (LTGRAY_BRUSH));
      SelectObject (ps.hdc, GetStockObject (WHITE_PEN));
      MoveToEx (ps.hdc, 0, 2*glcyMenu - 2, NULL);
      LineTo   (ps.hdc, 0, 0);
      LineTo   (ps.hdc, (int) rect.right, 0);
      hpen = CreatePen (PS_SOLID, 1, 0x808080);
      hpenSave = SelectObject (ps.hdc, hpen);
      MoveToEx (ps.hdc, 0, (int) 2*glcyMenu-1, NULL);
      LineTo   (ps.hdc, (int) rect.right - 1, (int) 2*glcyMenu-1);
      LineTo   (ps.hdc, (int) rect.right - 1, 1);
      SelectObject (ps.hdc, hpenSave);
      DeleteObject (hpen);

      GetClientRect (hwnd, &rect);

      //
      // positioning of the string based upon x,y,cx,cy of combobox
      //

      SetBkMode (ps.hdc, TRANSPARENT);

      p.x = rect.right;
      p.y = (rect.bottom - 2*glcyMenu < 0 ? 0 : rect.bottom - 2*glcyMenu);
      SetMapMode (ps.hdc, giMapMode);
      DPtoLP (ps.hdc, &p, 1);

      if ((giMapMode != MM_TEXT) && (giMapMode != MM_ANISOTROPIC))

        //
        // p.y will come out negative because we started with origin in
        //   upper left corner
        //

        p.y = -p.y;

      SetMapMode (ps.hdc, MM_TEXT);
      _snprintf_s (buf, BUFSIZE, _TRUNCATE, "cxClient = %ld (%ld)", rect.right, p.x);
      TextOut (ps.hdc, iComboboxWidth + (int) 3*glcyMenu/2, (int) glcyMenu/8,
               buf, (int) strlen (buf));
      _snprintf_s (buf, BUFSIZE, _TRUNCATE, "cyClient = %ld (%ld)",
				   rect.bottom - 2*glcyMenu < 0 ? 0 : rect.bottom - 2*glcyMenu,
                   p.y);
      TextOut (ps.hdc, iComboboxWidth + (int) 3*glcyMenu/2,
               (int) (glcyMenu/8 + lTextHeight),
               buf, (int) strlen (buf));

      //
      // paint graphics background white
      //

      rect.top    += 2*glcyMenu;
      FillRect (ps.hdc, &rect, GetStockObject (WHITE_BRUSH));


      if (rect.bottom <= 4*glcyMenu)

        //
        // we don't want to overpaint the toolbar, so just skip Paint() call
        //

        goto done_painting;

      //
      // set up a clip region so we don't draw all over our toolbar
      //

      GetClientRect (hwnd, &rect);
      rect.top += 2*glcyMenu;
      hrgn = CreateRectRgnIndirect (&rect);
      SelectClipRgn (ps.hdc, hrgn);
      DeleteObject (hrgn);

      //
      // set up view port, pens/brushes, & map mode, then paint
      //

      rect.top -= 2*glcyMenu;

      if (giMapMode == MM_TEXT || giMapMode == MM_ANISOTROPIC)

        SetViewportOrgEx (ps.hdc, glcyMenu, 3*glcyMenu, NULL);

      else

        SetViewportOrgEx (ps.hdc, glcyMenu, rect.bottom - glcyMenu, NULL);

      rect.bottom -= 4*glcyMenu;
      rect.right  -= 2*glcyMenu;

      hpen = CreatePen (giPenStyle, giPenWidth, gdwPenColor);
      SelectObject (ps.hdc, hpen);
      hbr  = CreateHatchBrush (giBrushStyle, gdwBrushColor);
      SelectObject (ps.hdc, hbr);

      SetTextColor (ps.hdc, gdwTextColor);

      SetMapMode (ps.hdc, giMapMode);
      Paint      (ps.hdc, &rect);

      DeleteObject (hpen);
      DeleteObject (hbr);

done_painting:

      EndPaint (hwnd, &ps);
      break;
    }

    case WM_CREATE:
    {
      HDC         hdc;
      TEXTMETRIC  tm;
      SIZE        size;
      HMENU       hmenu, hPenSubMenu, hBrushSubMenu;

      //
      // initialize the globals
      //

      glcyMenu = (LONG) GetSystemMetrics (SM_CYMENU);

      hmenu                = GetMenu (hwnd);
      hMappingModesSubMenu = GetSubMenu (hmenu, 2);
      hGraphicsSubMenu     = GetSubMenu (hmenu, 3);
      hPenSubMenu          = GetSubMenu (hmenu, 4);
      hPenWidthSubMenu     = GetSubMenu (hPenSubMenu, 1);
      hPenStyleSubMenu     = GetSubMenu (hPenSubMenu, 2);
      hBrushSubMenu        = GetSubMenu (hmenu, 5);
      hBrushStyleSubMenu   = GetSubMenu (hBrushSubMenu, 1);

      GetTextMetrics ((hdc = GetDC (hwnd)), &tm);
      lTextHeight = tm.tmHeight;

      //
      // create combobox to display current printers in. the width
      //   is caluculated by getting the text extent of a typical
      //   entry in the listbox.
      //


      #define ASTRING "long  printer  name;long  port  name;long  driver  name"

      GetTextExtentPoint (hdc, ASTRING, sizeof (ASTRING), &size);

      iComboboxWidth = (int) size.cx;

      ReleaseDC (hwnd, hdc);

      hwndCombobox = CreateWindow ((LPCSTR) "COMBOBOX", (LPCSTR) "",
                                   WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_VSCROLL,
                                   (int) glcyMenu/2,
                                   (int) glcyMenu/2 - 2,  // - 2 = fudge factor
                                   iComboboxWidth,
                                   (int) 6*glcyMenu,
                                   hwnd, NULL, ghInst, NULL);

      SetWindowLongPtr (hwndCombobox, GWLP_ID, ID_COMBOBOX);

      PostMessage (hwnd, WM_COMMAND,
                   (WPARAM) MAKELONG (IDM_REFRESH, 0),
                   (LPARAM) 0);
      PostMessage (hwnd, WM_COMMAND,
                   (WPARAM) MAKELONG (IDM_POLYPOLYGON, 0),
                   (LPARAM) 0);
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
*  FUNCTION:    AboutDlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    Handles "About" dialog messages
*
\******************************************************************************/

LRESULT CALLBACK AboutDlgProc (HWND   hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG:

      return TRUE;

    case WM_COMMAND:

      switch (LOWORD (wParam))
      {
        case IDOK:

          EndDialog (hwnd, TRUE);

          return 1;
      }
      break;
  }
  return 0;
}



/******************************************************************************\
*
*  FUNCTION:    InvalidateClient
*
*  COMMENTS:    Eliminates the flashing of the toolbar when we redraw
*
\******************************************************************************/

void InvalidateClient ()
{
  RECT rect;

  GetClientRect (ghwndMain, &rect);

  rect.top += 2*glcyMenu;

  InvalidateRect (ghwndMain, &rect, TRUE);
}



/******************************************************************************\
*
*  FUNCTION:    RefreshPrinterCombobox
*
*  INPUTS:      hwndCombobox- handle of the toolbar combobox
*
*  COMMENTS:    The idea here is to enumerate all printers & list them in
*               then combobox in the form: "DEVICE_NAME;PORT;DRIVER_NAME".
*               Then later, when a user selects one of these, we just
*               query out the string & parse it apart, sticking the
*               appropriate parts into the DriverName, DeviceName, and
*               Port variables.
*
*               Also, the "Display" option is added to the combobox so
*               that user can get DevCaps info about it.
*
\******************************************************************************/

void RefreshPrinterCombobox (HWND hwndCombobox)
{
  DWORD            dwFlags = PRINTER_ENUM_FAVORITE | PRINTER_ENUM_LOCAL;
  LPPRINTER_INFO_2 pPrinters;
  DWORD            cbPrinters;
  DWORD            cReturned, i;
  char             buf[BUFSIZE];

  SendMessage (hwndCombobox, CB_RESETCONTENT, 0, 0);

  //
  // add the "Display" option to the combobox
  //

  strncpy_s (buf, _countof(buf), "Display", _TRUNCATE);
  SendMessage (hwndCombobox, CB_INSERTSTRING, (UINT)-1, (LONG_PTR) buf);

  //
  // get byte count needed for buffer, alloc buffer, the enum the printers
  //

  EnumPrinters (dwFlags, NULL, 2, NULL, 0, &cbPrinters,
                &cReturned);

  if (!(pPrinters = (LPPRINTER_INFO_2) LocalAlloc (LPTR, cbPrinters + 4)))
  {
    MessageBox (ghwndMain, (LPCTSTR) GetStringRes(IDS_LALLOCFAIL),
                (LPCTSTR)GetStringRes2(ERR_MOD_NAME), MB_OK | MB_ICONEXCLAMATION);
    goto done_refreshing;
  }


  if (!EnumPrinters (dwFlags, NULL, 2, (LPBYTE) pPrinters,
                     cbPrinters, &cbPrinters, &cReturned))
  {
    MessageBox (ghwndMain, (LPCTSTR) GetStringRes(IDS_ENUMPRTFAIL),
                (LPCTSTR) GetStringRes2(ERR_MOD_NAME), MB_OK | MB_ICONEXCLAMATION);
    goto done_refreshing;
  }

  if (cReturned > 0)

    for (i = 0; i < cReturned; i++)
    {
      //
      // for each printer in the PRINTER_INFO_2 array: build a string that
      //   looks like "DEVICE_NAME;PORT;DRIVER_NAME"
      //

      strncpy_s (buf, _countof(buf), (pPrinters + i)->pPrinterName, _TRUNCATE);
      strncat_s (buf, _countof(buf), ";", _TRUNCATE);
      strncat_s (buf, _countof(buf), (pPrinters + i)->pPortName, _TRUNCATE);
      strncat_s (buf, _countof(buf), ";", _TRUNCATE);
      strncat_s (buf, _countof(buf), (pPrinters + i)->pDriverName, _TRUNCATE);

      SendMessage (hwndCombobox, CB_INSERTSTRING, (UINT)-1, (LONG_PTR) buf);
    }

  else

    MessageBox (ghwndMain, GetStringRes(IDS_NOPRTLST), "PRINTER.EXE", MB_OK);

done_refreshing:

  SendMessage (hwndCombobox, CB_SELECTSTRING, (UINT) -1, (LONG_PTR) buf);

  PostMessage (ghwndMain, WM_COMMAND,
               (WPARAM) MAKELONG (ID_COMBOBOX, CBN_SELCHANGE),
               (LPARAM) hwndCombobox);

  LocalFree (LocalHandle (pPrinters));
}



/******************************************************************************\
*
*  FUNCTION:    PrintThread
*
*  INPUTS:      wParam - wParam of a WM_COMMAND message containing menuitem id
*
*  COMMENTS:    This is the code for the print thread created when the user
*               selects the "Print" or "PrintDlg" menuitems. A thread is used
*               here more demostration purposes only, since we really don't
*               have any background processing to do. A real app would want
*               to have alot more error checking here (e.g. check return of
*               StartDoc, StartPage...).
*
\******************************************************************************/

void PrintThread (LPVOID wParam)
{
	DOCINFO di;
	RECT    rect;
	HPEN    hpen;
	HBRUSH  hbr;
	
	switch (LOWORD((WPARAM) wParam)) {
    case IDM_PRINT:
		{
			if (!strcmp (gszDeviceName, "Display"))
			{
				MessageBox (ghwndMain, GetStringRes(IDS_ASKSELPRT),
                    "PRINTER.EXE:", MB_OK);
				return;
			}
			else if (!(ghdc = CreateDC (gszDriverName, gszDeviceName, gszPort, NULL)))
			{
				MessageBox (ghwndMain, "PrintThread(): CreateDC() failed",
                    GetStringRes2(ERR_MOD_NAME), MB_OK);
				return;
			}
			break;
		}
		
    case IDM_PRINTDLG:
		{
			PRINTDLG  pd;
			
			//
			// Initialize a PRINTDLG struct and call PrintDlg to allow user to
			//   specify various printing options...
			//
			
			memset ((void *) &pd, 0, sizeof(PRINTDLG));
			
			pd.lStructSize = sizeof(PRINTDLG);
			pd.hwndOwner   = ghwndMain;
			pd.Flags       = PD_RETURNDC;
			pd.hInstance   = NULL;
			
			PrintDlg(&pd);
			ghdc = pd.hDC;
			
			if (pd.hDevMode)
				
				GlobalFree (pd.hDevMode);
			
			if (pd.hDevNames)
				
				GlobalFree (pd.hDevNames);
			
			if (!ghdc)
			{
				MessageBox (ghwndMain, GetStringRes(IDS_PRTDLGFAIL),
                    GetStringRes2(ERR_MOD_NAME), MB_OK);
				return;
			}
		}
	}
	
	//
	// put up Abort & install the abort procedure
	//
	
	gbAbort = FALSE;
	ghwndAbort = CreateDialog (ghInst, (LPCTSTR) "Abort", ghwndMain,
		(DLGPROC) AbortDlgProc);
	EnableWindow (ghwndMain, FALSE);
	SetAbortProc (ghdc, AbortProc);
	
	//
	// create & select pen/brush
	//
	
	hpen = CreatePen (giPenStyle, giPenWidth, gdwPenColor);
	SelectObject (ghdc, hpen);
	
	hbr  = CreateHatchBrush (giBrushStyle, gdwBrushColor);
	SelectObject (ghdc, hbr);
	
	SetTextColor (ghdc, gdwTextColor);
	
	SetMapMode (ghdc, giMapMode);
	
	SetRect(&rect, 0,0, GetDeviceCaps (ghdc, HORZRES), GetDeviceCaps (ghdc, VERTRES));
	
	di.cbSize      = sizeof(DOCINFO);
	di.lpszDocName = GetStringRes(IDS_PRTTST);
	di.lpszOutput  = NULL;
	di.fwType      = 0; // Windows 95 only; ignored on Windows NT

	StartDoc  (ghdc, &di);
	
	if (gdwGraphicsOptions) {
		
		Paint (ghdc, &rect);
		
	} else {
		LPSTR pBuf = GetStringRes(IDS_BLANKPG); 
		TextOut (ghdc, 5, 5, (LPCTSTR) pBuf, (int) My_mbslen(pBuf));
	}
	
	// If we didn't abort the job
	if (!gbAbort)
	{
		EndPage   (ghdc);  
		EndDoc    (ghdc);
		
		DestroyWindow (ghwndAbort);
	}
	
	// Clean up
	DeleteDC  (ghdc);
	DeleteObject(hpen);
	DeleteObject(hbr);
		
	EnableWindow  (ghwndMain, TRUE);
}
		  		  
/******************************************************************************\
*
*  FUNCTION:    AbortProc
*
*  COMMENTS:    Standard printing abort proc
*
\******************************************************************************/

BOOL CALLBACK AbortProc (HDC hdc, int error)
{
	MSG msg;
	
	while (!gbAbort && PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
		
		if (!IsDialogMessage (ghwndAbort, &msg)) {
			TranslateMessage (&msg);
			DispatchMessage (&msg);
		}
		
	}
	
	return !gbAbort;
}



/******************************************************************************\
*
*  FUNCTION:    AbortDlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    Handles "Abort" dialog messages
*
\******************************************************************************/

LRESULT CALLBACK AbortDlgProc (HWND   hwnd, UINT msg, WPARAM wParam,
                               LPARAM lParam)
{
	switch (msg) {
    case WM_INITDIALOG:
		EnableMenuItem (GetSystemMenu (hwnd, FALSE), SC_CLOSE, MF_GRAYED);
		return TRUE;
		
    case WM_COMMAND:   // There's only one 
		switch (LOWORD (wParam)) {
		case DID_CANCEL:
			MessageBeep(MB_OK);
			gbAbort = TRUE;
			AbortDoc (ghdc);
			DestroyWindow (hwnd);
			break;
		}
		break;
	}
	return 0;
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
  static TCHAR buffer[MAX_PATH];

  buffer[0]=0;
  LoadString (GetModuleHandle (NULL), id, buffer, MAX_PATH);
  return buffer;
}

LPTSTR   GetStringRes2 (int id)
{
  static TCHAR buffer[MAX_PATH];

  buffer[0]=0;
  LoadString (GetModuleHandle (NULL), id, buffer, MAX_PATH);
  return buffer;
}

