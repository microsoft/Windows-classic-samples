
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
*  PROGRAM:     GETCAPS.C
*
*  PURPOSE:     Handles display of information returned by call to
*               GetDeviceCaps. GetDeviceCaps is called for the
*               currently selected deivce (in the tool bar comobobox),
*               and the results are formatted and displayed in a dialog
*               box.
*
*  FUNTIONS:    GetDeviceCapsDlgProc - handles messages for dialog
*               DisplayDeviceCapsInfo- retrieves device caps info
*               TranslateDeviceCaps  - displays a capability in listbox
*               ComplexDeviceCapsLine- formats a bitfield capability
*
\******************************************************************************/

#include <windows.h>
#include <winspool.h>
#include <string.h>
#include <stdio.h>
#include <winspool.h>
#include "common.h"
#include "getcaps.h"
#include "resource.h"

/******************************************************************************\
*
*  FUNCTION:    GetDeviceCapsDlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    Processes messages for GetDeviceCaps dialog box
*
\******************************************************************************/

LRESULT CALLBACK   GetDeviceCapsDlgProc (HWND   hwnd, UINT msg, WPARAM wParam,
                                         LPARAM lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG:
    {
      BOOL bReturn;
      char buf[BUFSIZE];

      SendDlgItemMessage (hwnd, DID_LISTBOX, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), (LPARAM)0);

	  ghwndDevCaps = hwnd;

      //
      // shove all the caps info in the list box
      //

      SetCursor (LoadCursor (NULL, IDC_WAIT));
      bReturn = DisplayDeviceCapsInfo ();
      SetCursor (LoadCursor (NULL, IDC_ARROW));

      if (!bReturn)
      {
        EndDialog (hwnd, TRUE);
      }

      //
      // set window title to reflect current device
      //

      else
      {
        if ( _snprintf_s (buf, BUFSIZE, BUFSIZE-1, "GetDeviceCaps: %s;%s;%s", gszDeviceName, gszPort, gszDriverName) > 0 )
	        SetWindowText (hwnd, (LPCSTR) buf);
      }

      break;
    }

    case WM_COMMAND:

      switch (LOWORD (wParam))
      {
        case DID_OK:

          EndDialog (hwnd, TRUE);
          return 1;
      }
      break;
  }
  return 0;
}



/******************************************************************************\
*
*  FUNCTION:    DisplayDeviceCapsInfo
*
*  RETURNS:     TRUE if successful,
*               FALSE otherwise
*
*  COMMENTS:    Retrieves all device caps for current deivce & calls
*                 TranslateCaps to insert them in the dialog's listbox.
*
\******************************************************************************/

BOOL DisplayDeviceCapsInfo ()
{
	HDC hdc;
	int i, iValue;
	
	if (!strcmp (gszDeviceName, "Display"))
	{
		if (!(hdc = GetDC (ghwndDevCaps)))
		{
			ErrMsgBox (GetStringRes(IDS_GETDCFAIL), ERR_MOD_NAME);
			return FALSE;
		}
	}
	
	else
	{
		if (!(hdc = CreateDC (gszDriverName, gszDeviceName, gszPort, NULL)))
		{
			char buf[BUFSIZE];
			
			if (_snprintf_s (buf, BUFSIZE, _TRUNCATE, GetStringRes(IDS_FMT_CREDCFAIL),	gszDriverName, gszDeviceName, gszPort) > 0)
				ErrMsgBox (buf, ERR_MOD_NAME);

			return FALSE;
		}
	}
	
	for (i = 0; i < MAX_DEVICE_CAPS; i++)
	{
		iValue = GetDeviceCaps (hdc, gaCaps[i].iValue);
		TranslateDeviceCaps (i, gaCaps[i].iValue, iValue);
	}
	
	if (!strcmp (gszDeviceName, "Display"))
		ReleaseDC(ghwndDevCaps, hdc);
	else
		DeleteDC (hdc);
	
	return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    TranslateDeviceCaps
*
*  INPUTS:      arrayIndex - index into gaCaps[]
*               capIndex   - devcap index (eg. TECHNOLOGY, CURVECAPS)
*               iValue     - value returned by GetDeviceCaps
*
*  COMMENTS:    For simple devcaps (eg. tjose with single numeric return
*               value), appends caps value to string and inserts into
*               listbox. For "complex" caps (those returning multiple
*               bit-values) calls ComplexCapsLine which handles text
*               formattting & insertion.
*
\******************************************************************************/

void TranslateDeviceCaps (int arrayIndex, int capIndex, int iValue)
{
  char buf[BUFSIZE];

  strncpy_s (buf, BUFSIZE, gaCaps[arrayIndex].szValue, _countof(gaCaps[arrayIndex].szValue));

  switch (capIndex)
  {
    case TECHNOLOGY:
    {
      int     i;
      
      for (i = 0; i < MAX_TECHNOLOGY_CAPS; i++)
        if (iValue == (gaTechnologyCaps + i)->iValue)
        {
          strncat_s(buf, BUFSIZE, (gaTechnologyCaps + i)->szValue, _TRUNCATE);
          SendDlgItemMessage (ghwndDevCaps, DID_LISTBOX, LB_INSERTSTRING,
                              (UINT)-1, (LONG_PTR) buf);
          break;
        }
            
      break;
    }

    case CURVECAPS:

      ComplexDeviceCapsLine (buf, gaCurveCaps, MAX_CURVE_CAPS, iValue, BUFSIZE);
      break;

    case LINECAPS:

      ComplexDeviceCapsLine (buf, gaLineCaps, MAX_LINE_CAPS, iValue, BUFSIZE);
      break;

    case POLYGONALCAPS:

      ComplexDeviceCapsLine (buf, gaPolygonCaps, MAX_POLYGON_CAPS, iValue, BUFSIZE);
      break;

    case TEXTCAPS:

      ComplexDeviceCapsLine (buf, gaTextCaps, MAX_TEXT_CAPS, iValue, BUFSIZE);
      break;

    case CLIPCAPS:

      ComplexDeviceCapsLine (buf, gaClipCaps, MAX_CLIP_CAPS, iValue, BUFSIZE);
      break;

    case RASTERCAPS:

      ComplexDeviceCapsLine (buf, gaRasterCaps, MAX_RASTER_CAPS, iValue, BUFSIZE);
      break;

    default:

      sprintf_s(buf, sizeof(buf), gaCaps[arrayIndex].szValue, iValue);

      SendDlgItemMessage (ghwndDevCaps, DID_LISTBOX, LB_INSERTSTRING,
                          (UINT)-1, (LONG_PTR) buf);
      break;
  }
}



/******************************************************************************\
*
*  FUNCTION:     ComplexDeviceCapsLine
*
*  INPUTS:       pbuf        - pointer to buffer containing a cap-type
*                              string
*                pLkUp       - pointer to a CAPSLOOKUP table
*                iMaxEntries - # of enries in table pointed at by pLkUp
*                iValue      - an integer containing 1+ bit-value flags.
*
*  COMMENTS:     This function is used to expand an int containing
*                multiple bit-values into a set of strings which are
*                inserted into the DevCapsDlg listbox. The iValue
*                parameter is checked against each iIndex entry in the
*                CAPSLOOKUP table pointed at by pLkUp, and when matches
*                are found the corresponding (lpszValue) string is
*                inserted.
*
*                The buffer pointed to by pbuf will be destroyed.
*
\******************************************************************************/

void ComplexDeviceCapsLine (char *pbuf, CAPSLOOKUP *pLkUp, int iMaxEntries,
                     int iValue, int iBuffSize)
{
  int  i;
  BOOL bNewLine = FALSE;

  for (i = 0; i < iMaxEntries; i++)

    if (iValue & (pLkUp + i)->iValue)
    {
      if (bNewLine)
      {
        //
        // Keep the first symbolic constant on the same line as the
        //   cap type, eg:  "TECHNOLOGY:     DT_RASDISPLAY".
        //

        strncpy_s (pbuf, iBuffSize, BLANKS, _countof(BLANKS));
        strncat_s (pbuf, iBuffSize, (pLkUp + i)->szValue, _TRUNCATE);
      }
      else
      {
        //
        // Put symbolic constant on new line, eg:
        //                  "                DT_RASPRINTER".
        //

        strncat_s (pbuf, iBuffSize, (pLkUp + i)->szValue, _TRUNCATE);
        bNewLine = TRUE;
      }
      SendDlgItemMessage (ghwndDevCaps, DID_LISTBOX, LB_INSERTSTRING,
                          (UINT)-1, (LONG_PTR) pbuf);
   }
}

