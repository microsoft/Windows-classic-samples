
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
*  PROGRAM:     ENUMPRT.C
*
*  PURPOSE:     Handles display of information returned by calls to
*               EnumPrinters, EnumPrinterDrivers. Info formatted and
*               displayed in a dialog box.
*
*
*  FUNTIONS:    EnumPrintersDlgProc      - handles messages for dialog
*               DisplayEnumPrintersInfo  - retrieves printer info
*               SetEnumPrintersDlgFields - formats & displays printer info
*               ComplexEnumPrintersLine  - formats bitfield printer info
*               EnumPrinterDriversDlgProc- handles messages for dialog
*               DisplayPrinterDriversInfo- retrieves, formats, & displays
*                                            printer info
*
\******************************************************************************/

#include <windows.h>
#include <string.h>
#include <drivinit.h>
#include <stdio.h>
#include <winspool.h>
#include <tchar.h>
#include "common.h"
#include "enumprt.h"
#include "resource.h"


/******************************************************************************\
*
*  FUNCTION:    EnumPrintersDlgProc (standard dialog procedure INPUTS/RETURNS)
*
*  COMMENTS:    Processes messages for EnumPrinters dialog box
*
\******************************************************************************/

LRESULT CALLBACK EnumPrintersDlgProc (HWND   hwnd, UINT msg, WPARAM wParam,
                                      LPARAM lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG:
    {
      BOOL bReturn;

      //
      // prompt user for EnumPrinters flags...
      //

      if (DialogBox (GetModuleHandle (NULL), (LPCTSTR) "EnumPrtOpt",
                     NULL, (DLGPROC) EnumPrintersOptionsDlgProc))
      {
        //
        // shove all the enum printer info in the list box
        //

        SetCursor (LoadCursor (NULL, IDC_WAIT));
        bReturn = DisplayEnumPrintersInfo (hwnd);
        SetCursor (LoadCursor (NULL, IDC_ARROW));

        if (!bReturn)

          EndDialog (hwnd, TRUE);

        else

          SetWindowText (hwnd, (LPCTSTR)"EnumPrinters");
      }
      else

          EndDialog (hwnd, TRUE);

      break;
    }

    case WM_COMMAND:

      switch (LOWORD (wParam))
      {
        case DID_OK:

          EndDialog (hwnd, TRUE);
          break;
      }
      break;
  }
  return 0;
}



/******************************************************************************\
*
*  FUNCTION:    EnumPrintersOptionsDlgProc (standard dlg proc INPUTS/RETURNS)
*
*  COMMENTS:    Processes messages for EnumPrtOpt dialog box
*
\******************************************************************************/

LRESULT CALLBACK EnumPrintersOptionsDlgProc (HWND   hwnd,   UINT msg,
                                             WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG:

      gdwEnumFlags = 0;
      gszEnumName[0] = 0;
      break;

    case WM_COMMAND:

      switch (LOWORD (wParam))
      {
        case DID_OK:

          if (gdwEnumFlags)
          {
            if (gdwEnumFlags & PRINTER_ENUM_NAME)
            {
              GetDlgItemText (hwnd, DID_EDITTEXT, (LPTSTR)gszEnumName,
                              BUFSIZE);

#ifdef FORCE_VALID_NAME
              if (!strlen (gszEnumName))
              {
                MessageBox (hwnd,
                            (LPCTSTR) GetStringRes(IDS_ASKDOMSRVNM),
                            (LPCTSTR) "", MB_OK);
                SetFocus (GetDlgItem (hwnd, DID_EDITTEXT));
                break;
              }
#endif

            }

            EndDialog (hwnd, TRUE);
          }

          else

            EndDialog (hwnd, FALSE);

          break;

        case DID_CANCEL:

          EndDialog (hwnd, FALSE);
          break;

        default:

          if (HIWORD(wParam) == BN_CLICKED)
          {
            DWORD dwControlId = (DWORD) LOWORD (wParam);

            if (gdwEnumFlags & dwControlId)
            {
              //
              // remove that flag, if PRINTER_ENUM_NAME disable edittext
              //

              gdwEnumFlags &= ~dwControlId;

              if (dwControlId & PRINTER_ENUM_NAME)
              {
                SetDlgItemText (hwnd, DID_EDITTEXT, (LPCTSTR)"");
                EnableWindow   (GetDlgItem (hwnd, DID_EDITTEXT), FALSE);
              }
            }

            else
            {
              //
              // add that flag, if PRINTER_ENUM_NAME enable edittext
              //

              gdwEnumFlags |= dwControlId;

              if (dwControlId & PRINTER_ENUM_NAME)

                EnableWindow (GetDlgItem (hwnd, DID_EDITTEXT), TRUE);
            }
          }
          break;
      }
  }
  return 0;
}



/******************************************************************************\
*
*  FUNCTION:    DisplayEnumPrintersInfo
*
*  INPUTS:      hwnd - handle of the EnumPrinters dialog box
*
*  RETURNS:     TRUE if successful,
*               FALSE otherwise
*
\******************************************************************************/

BOOL DisplayEnumPrintersInfo (HWND hwnd)
{
    DWORD  dwBytesNeeded;
    DWORD  dwPrtRet1, dwPrtRet2;
    DWORD  dwMaxPrt;
    LPTSTR lpName = gdwEnumFlags & PRINTER_ENUM_NAME ? gszEnumName : NULL;
    
    LPPRINTER_INFO_1 pPrtInfo1 = NULL;
    LPPRINTER_INFO_2 pPrtInfo2 = NULL;
    
    BOOL   bReturn = TRUE;
    
    //
    // get byte count needed for buffer, alloc buffer, the enum the printers
    //
    EnumPrinters (gdwEnumFlags,   // types of printer objects to enumerate          
                  lpName,         // name of printer object                         
                  1,              // specifies type of printer info structure       
                  NULL,           // use NULL to get buffer size                    
                  0,              // size, in bytes, of array                       
                  &dwBytesNeeded, // pointer to variable with no. of bytes          
                                  // copied (or required)                           
                  &dwPrtRet1);    // pointer to variable with no. of printer        
                                  // info. structures copied                        
    //
    // (simple error checking, if these work assume rest will too)
    //
    
    // Allocate enough room for the returned data
    if (!(pPrtInfo1 = (LPPRINTER_INFO_1) LocalAlloc (LPTR, dwBytesNeeded)))
    {
        ErrMsgBox (GetStringRes(IDS_ENUMPRTLALLOCFAIL), GetStringRes2(ERR_MOD_NAME));
        
        return FALSE;  // Bail
    }
    
    if (!EnumPrinters(gdwEnumFlags,      // types of printer objects to enumerate       
                      lpName,            // name of printer object                      
                      1,                 // specifies type of printer info structure    
                      (LPBYTE)pPrtInfo1, // pointer to buffer to receive printer info   
                                         // structures                                  
                      dwBytesNeeded,     // size, in bytes, of array                    
                      &dwBytesNeeded,    // pointer to variable with no. of bytes       
                                         // copied (or required)                        
                      &dwPrtRet1))       // pointer to variable with no. of printer     
                                         // info. structures copied                     
    {
        TCHAR  tcBuffer[256];
        
        // Create and display our error message
        _sntprintf_s (tcBuffer, _countof(tcBuffer), _TRUNCATE,
					  "%s, 1, GetLastError: %d", GetStringRes2(ERR_MOD_NAME), GetLastError());
        ErrMsgBox (GetStringRes(IDS_ENUMPRT1FAIL), tcBuffer);
        
        // Free the buffer we allocated
        LocalFree(pPrtInfo1);

        return FALSE;  // Bail
    }
    
    //
    // If we don't get any printers from the Level == 1 call, there is
    //  no point in continuing... report it, free memory, and return.
    //
    if (dwPrtRet1 == 0) {
        
        MessageBox (ghwndMain,
            (LPCTSTR) "EnumPrinters (Level == 1) returned 0 printers",
            GetStringRes2(ERR_MOD_NAME),
            MB_OK);

        // Free the buffer we allocated
        LocalFree(pPrtInfo1);

        return FALSE;  // Bail
    }
    
    
    //
    // Call EnumPrinters again, this time with Level == 2.
    //
    // get byte count needed for buffer, alloc buffer, the enum the printers
    //
    EnumPrinters (gdwEnumFlags,     // types of printer objects to enumerate       
                  lpName,           // name of printer object                      
                  2,                // specifies type of printer info structure    
                  NULL,             // use NULL to get buffer size                 
                  0,                // size, in bytes, of array                    
                  &dwBytesNeeded,   // pointer to variable with no. of bytes       
                                    // copied (or required)                        
                  &dwPrtRet2);      // pointer to variable with no. of printer     
                                    // info. structures copied                     

    // Allocate enough room for the returned data
    if (!(pPrtInfo2 = (LPPRINTER_INFO_2) LocalAlloc (LPTR, dwBytesNeeded)))	{
        ErrMsgBox (GetStringRes(IDS_ENUMPRTLALLOCFAIL), GetStringRes2(ERR_MOD_NAME));
    } else {
			  
		if (!EnumPrinters (gdwEnumFlags,      // types of printer objects to enumerate    
						   lpName,            // name of printer object                   
						   2,                 // specifies type of printer info structure 
						   (LPBYTE)pPrtInfo2, // pointer to buffer to receive printer info
											  // structures                               
						   dwBytesNeeded,     // size, in bytes, of array                 
						   &dwBytesNeeded,    // pointer to variable with no. of bytes    
											  // copied (or required)                     
						   &dwPrtRet2))       // pointer to variable with no. of printer  
											  // info. structures copied                  
    		dwPrtRet2 = 0;
    }
    
    //
    //  Calling EnumPrinters with Level == 2 frequently returns 0 printers.
    //  If so display only the PRINTER_INFO_1 structures we got before.
    //
    
    if (dwPrtRet2 == 0)
    {
        dwMaxPrt = dwPrtRet1;
        LocalFree (pPrtInfo2);
        pPrtInfo2 = NULL;
    } else {
        dwMaxPrt = dwPrtRet1 > dwPrtRet2 ? dwPrtRet2 : dwPrtRet1;
    }
    
    
    SetEnumPrintersDlgFields (hwnd, dwMaxPrt, pPrtInfo1, pPrtInfo2);
    
    if (pPrtInfo2)
        LocalFree (pPrtInfo2);

	if (pPrtInfo1)
        LocalFree (pPrtInfo1);
    
    return TRUE;
}


/******************************************************************************\
*
*  FUNCTION:    SetEnumPrintersDlgFields
*
*  INPUTS:      hwnd      - handle of the EnumPrinters dialog box
*               dwMaxPrt  - number of elements in the following two arrays
*               pdisplay_prts_info1 - ptr to an array of PRINTER_INFO_1 structs
*               pdisplay_prts_info2 - ptr to an array of PRINTER_INFO_2 structs
*
*  COMMENTS:    This function formats the info returned by EnumPrinters()
*               into readable strings and inserts them in the listbox.
*
\******************************************************************************/

void SetEnumPrintersDlgFields (HWND hwnd, DWORD dwMaxPrt,
                               LPPRINTER_INFO_1 pPrtInfo1,
                               LPPRINTER_INFO_2 pPrtInfo2)
{
	char  buf[256];
	WORD  i;
	DWORD j;
	
	SendDlgItemMessage (hwnd, DID_LISTBOX, LB_RESETCONTENT, 0, 0);
	SendDlgItemMessage (hwnd, DID_LISTBOX, WM_SETFONT, (WPARAM)GetStockObject(ANSI_FIXED_FONT), (LPARAM)0);
	
	for (j = 0; j < dwMaxPrt; j++) {
		//
		// Stick PRINTER_INFO_1  data in listbox
		//
		
		SendDlgItemMessage (hwnd, DID_LISTBOX, LB_INSERTSTRING, (UINT)-1,
			(LONG_PTR) gaEnumPrt[0]);
		
		outstr (gaEnumPrt[1], (pPrtInfo1 + j)->pDescription);
		outstr (gaEnumPrt[2], (pPrtInfo1 + j)->pName);
		outstr (gaEnumPrt[3], (pPrtInfo1 + j)->pComment);
		
		//
		// Stick PRINTER_INFO_2  data in listbox
		//
		
		if (pPrtInfo2 != NULL) {
			
			SendDlgItemMessage (hwnd, DID_LISTBOX, LB_INSERTSTRING, (UINT)-1,
								(LONG_PTR) gaEnumPrt[4]);
			
			outstr (gaEnumPrt[5],  (pPrtInfo2 + j)->pServerName);
			outstr (gaEnumPrt[6],  (pPrtInfo2 + j)->pPrinterName);
			outstr (gaEnumPrt[7],  (pPrtInfo2 + j)->pShareName);
			outstr (gaEnumPrt[8],  (pPrtInfo2 + j)->pPortName);
			outstr (gaEnumPrt[9],  (pPrtInfo2 + j)->pDriverName);
			outstr (gaEnumPrt[10], (pPrtInfo2 + j)->pComment);
			outstr (gaEnumPrt[11], (pPrtInfo2 + j)->pLocation);
			
			if ((pPrtInfo2 + j)->pDevMode)
			{
				DWORD dwFields;
				
				outstr (gaEnumPrt[12], "");
				outstr (gaEnumPrt[13], (pPrtInfo2 + j)->pDevMode->dmDeviceName);
				outnum (gaEnumPrt[14], (pPrtInfo2 + j)->pDevMode->dmSpecVersion);
				outnum (gaEnumPrt[15], (pPrtInfo2 + j)->pDevMode->dmDriverVersion);
				outnum (gaEnumPrt[16], (pPrtInfo2 + j)->pDevMode->dmSize);
				outnum (gaEnumPrt[17], (pPrtInfo2 + j)->pDevMode->dmDriverExtra);
				
				dwFields = (pPrtInfo2 + j)->pDevMode->dmFields;
				ComplexEnumPrintersLine (hwnd, gaEnumPrt[18], gaFields, 
										 MAX_FIELDS,
										 dwFields);
				
				if (dwFields & DM_ORIENTATION) {
					ComplexEnumPrintersLine (hwnd, gaEnumPrt[19], gaOrientation,
											 MAX_ORIENTATION,
											 (DWORD)(pPrtInfo2 + j)->pDevMode->dmOrientation);
				}

				if (dwFields & DM_PAPERSIZE) {
					ComplexEnumPrintersLine (hwnd, gaEnumPrt[20], gaPaperSize,
											 MAX_PAPERSIZE,
											 (DWORD)(pPrtInfo2 + j)->pDevMode->dmPaperSize);
				}

				if (dwFields & DM_PAPERLENGTH)
					outnum (gaEnumPrt[21], (pPrtInfo2 + j)->pDevMode->dmPaperLength);
				
				if (dwFields & DM_PAPERWIDTH)
					outnum (gaEnumPrt[22], (pPrtInfo2 + j)->pDevMode->dmPaperWidth);
				
				if (dwFields & DM_SCALE)
					outnum (gaEnumPrt[23], (pPrtInfo2 + j)->pDevMode->dmScale);
				
				if (dwFields & DM_COPIES)
					outnum (gaEnumPrt[24], (pPrtInfo2 + j)->pDevMode->dmCopies);
				
				if (dwFields & DM_DEFAULTSOURCE) {
					ComplexEnumPrintersLine (hwnd, gaEnumPrt[25], gaDefaultSource,
											 MAX_DEFAULTSOURCE,
											 (DWORD)(pPrtInfo2 + j)->pDevMode->dmDefaultSource);
				}

				if (dwFields & DM_PRINTQUALITY) {
					ComplexEnumPrintersLine (hwnd, gaEnumPrt[26], gaPrintQuality,
											 MAX_PRINTQUALITY,
											 (DWORD)(pPrtInfo2 + j)->pDevMode->dmPrintQuality);
				}

				if (dwFields & DM_COLOR) {
					ComplexEnumPrintersLine (hwnd, gaEnumPrt[27], gaColor,
											 MAX_COLOR,
											 (DWORD)(pPrtInfo2 + j)->pDevMode->dmColor);
				}

				if (dwFields & DM_DUPLEX) {
					ComplexEnumPrintersLine (hwnd, gaEnumPrt[28], gaDuplex,
											 MAX_DUPLEX,
											 (DWORD)(pPrtInfo2 + j)->pDevMode->dmDuplex);
				}
								
				if (dwFields & DM_YRESOLUTION)
					outnum (gaEnumPrt[29], (DWORD)(pPrtInfo2 + j)->pDevMode->dmYResolution);
				
				if (dwFields & DM_TTOPTION)
					outnum (gaEnumPrt[30], (DWORD)(pPrtInfo2 + j)->pDevMode->dmTTOption);
				
				if (dwFields & DM_COLLATE)
					outnum (gaEnumPrt[31], (DWORD)(pPrtInfo2 + j)->pDevMode->dmCollate);
				
				if (dwFields & DM_FORMNAME)
					outstr (gaEnumPrt[32], (pPrtInfo2 + j)->pDevMode->dmFormName);
			}
			else
			{
				outstr (gaEnumPrt[12], NULL);
			}
			
			outstr (gaEnumPrt[33], (pPrtInfo2 + j)->pSepFile);
			outstr (gaEnumPrt[34], (pPrtInfo2 + j)->pPrintProcessor);
			outstr (gaEnumPrt[35], (pPrtInfo2 + j)->pDatatype);
			outstr (gaEnumPrt[36], (pPrtInfo2 + j)->pParameters);
			
			ComplexEnumPrintersLine (hwnd, gaEnumPrt[37], gaAttributes,
									 MAX_ATTRIBUTES,
									 (DWORD)(pPrtInfo2 + j)->Attributes);
			
			for (i = 0; i < MAX_PRIORITIES; i++) {
				if ((pPrtInfo2 + j)->Priority & gaPriorities[i].dwValue) {
					outstr (gaEnumPrt[38], gaPriorities[i].szValue);
					break;
				}
			}
			
			if (i == MAX_PRIORITIES) {
				outnum (gaEnumPrt[39], (pPrtInfo2 + j)->Priority);
			}

			outnum (gaEnumPrt[40], (pPrtInfo2 + j)->DefaultPriority);
			outnum (gaEnumPrt[41], (pPrtInfo2 + j)->StartTime);
			outnum (gaEnumPrt[42], (pPrtInfo2 + j)->UntilTime);
			
			ComplexEnumPrintersLine (hwnd, gaEnumPrt[43], gaStatus, 
									 MAX_STATUS,
									 (DWORD)(pPrtInfo2 + j)->Status);
			
			outnum (gaEnumPrt[44], (pPrtInfo2 + j)->cJobs);
			outnum (gaEnumPrt[45], (pPrtInfo2 + j)->AveragePPM);
        }
    }
}



/******************************************************************************\
*
*  FUNCTION:    ComplexEnumPrintersLine
*
*  INPUTS:      hwnd        - handle of the EnumPrinters dialog box
*               pbuf        - pointer to buffer containing a cap-type
*                              string
*               pLkUp       - pointer to a CAPSLOOKUP table
*               iMaxEntries - # of enries in table pointed at by pLkUp
*               iValue      - an integer containing 1+ bit-value flags.
*
*  COMMENTS:    This function is used to expand an int containing
*               multiple bit-values into a set of strings which are
*               inserted into the DevCapsDlg listbox. The iValue
*               parameter is checked against each iIndex entry in the
*               CAPSLOOKUP table pointed at by pLkUp, and when matches
*               are found the corresponding (lpszValue) string is
*               inserted.
*
*               The buffer pointed to by pbuf will be destroyed.
*
\******************************************************************************/

void ComplexEnumPrintersLine (HWND hwnd, char  *pstr, ENUMPRTLOOKUP *pLkUp,
                              int iMaxEntries, DWORD  dwValue)
{
  char buf [BUFSIZE];
  int  i;
  BOOL bNewLine = FALSE;

  strncpy_s (buf, _countof(buf), pstr, _countof(pstr));

  for (i = 0; i < iMaxEntries; i++)

    if (dwValue & (pLkUp + i)->dwValue)
    {
      if (bNewLine)
      {
        //
        // Keep the first symbolic constant on the same line as the
        //   cap type, eg:  "TECHNOLOGY:     DT_RASDISPLAY".
        //
        strncpy_s (buf, _countof(buf), BLANKS, _countof(BLANKS));
        strncat_s (buf, _countof(buf), (pLkUp + i)->szValue, _TRUNCATE);
      }
      else
      {
        //
        // Put symbolic constant on new line, eg:
        //                  "                DT_RASPRINTER".
        //
		strncat_s (buf, _countof(buf), (pLkUp + i)->szValue, _TRUNCATE);
        bNewLine = TRUE;
      }
      SendDlgItemMessage (hwnd, DID_LISTBOX, LB_INSERTSTRING,
                          (UINT)-1, (LONG_PTR) buf);
   }
}



/******************************************************************************\
*
*  FUNCTION:    EnumPrinterDriversDlgProc (standard dlg proc INPUTS/RETURNS)
*
*  COMMENTS:    Processes messages for EnumPrinterDrivers dialog box
*
\******************************************************************************/

LRESULT CALLBACK EnumPrinterDriversDlgProc (HWND   hwnd,   UINT   msg,
                                            WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
    case WM_INITDIALOG:
    {
      BOOL bReturn;

      SetCursor (LoadCursor (NULL, IDC_WAIT));
      bReturn = DisplayPrinterDriversInfo (hwnd);
      SetCursor (LoadCursor (NULL, IDC_ARROW));

      if (!bReturn)

        EndDialog (hwnd, TRUE);

      else

        SetWindowText (hwnd, (LPCTSTR) "EnumPrinterDrivers");

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
*  FUNCTION:    DisplayPrinterDriversInfo
*
*  INPUTS:      hwnd - handle of the EnumPrinterDrivers dialog box
*
*  RETURNS:     TRUE if successful,
*               FALSE otherwise
*
*  COMMENTS:    Retrieves EnumPrinterDrivers info, formats, & inserts
*               in listbox.
*
\******************************************************************************/

BOOL DisplayPrinterDriversInfo (HWND hwnd)
{
  DWORD         dwBytesNeeded, dwDrvRet, i;
  DRIVER_INFO_1 *pDriverInfo1;
  DRIVER_INFO_2 *pDriverInfo2;
  char          buf[BUFSIZE];
  BOOL          bReturn = TRUE;

  //
  // get byte count needed for buffer, alloc buffer, the enum the drivers
  //

  EnumPrinterDrivers ((LPTSTR) NULL, (LPTSTR) NULL, 1, NULL,
                      0, &dwBytesNeeded, &dwDrvRet);

  //
  // simple error checking, if these work assume rest will too
  //

  if (!(pDriverInfo1 = (DRIVER_INFO_1 *) LocalAlloc (LPTR, dwBytesNeeded)))
  {
    ErrMsgBox (GetStringRes(IDS_LALLOCFAIL), GetStringRes2(ERR_MOD_NAME));
    bReturn = FALSE;
    goto display_prt_drvs_info_done1;
  }

  if (!EnumPrinterDrivers ((LPTSTR) NULL, (LPTSTR) NULL, 1,
                           (LPBYTE) pDriverInfo1, dwBytesNeeded, &dwBytesNeeded,
                           &dwDrvRet))
  {
    ErrMsgBox (GetStringRes(IDS_ENUMPRTDRVRET0), GetStringRes2(ERR_MOD_NAME));
    bReturn = FALSE;
    goto display_prt_drvs_info_done2;
  }

  EnumPrinterDrivers ((LPTSTR) NULL,(LPTSTR) NULL, 2, NULL,
                      0, &dwBytesNeeded, &dwDrvRet);

  pDriverInfo2 = (DRIVER_INFO_2 *) LocalAlloc (LPTR, dwBytesNeeded);

  EnumPrinterDrivers ((LPTSTR) NULL, (LPTSTR) NULL, 2,
                      (LPBYTE) pDriverInfo2, dwBytesNeeded, &dwBytesNeeded,
                      &dwDrvRet);

  if (!dwDrvRet)
  {
    ErrMsgBox (GetStringRes(IDS_ENUMPRTDRVRET0), "");
    bReturn = FALSE;
    goto display_prt_drvs_info_done3;
  }

  //
  // insert formatted info into listbox
  //

  for (i = 0; i < dwDrvRet; i++)
  {
    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[0]) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[1], (pDriverInfo1 + i)->pName) > 0)
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[2]) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[3], (pDriverInfo2 + i)->cVersion) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[4], (pDriverInfo2 + i)->pName) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[5], (pDriverInfo2 + i)->pEnvironment) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[6], (pDriverInfo2 + i)->pDriverPath) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[7], (pDriverInfo2 + i)->pDataFile) > 0 )
		outstr3();

    if ( _snprintf_s (buf, BUFSIZE, _TRUNCATE, gaDriverInfo[8], (pDriverInfo2 + i)->pConfigFile) > 0 )
		outstr3();

  }

display_prt_drvs_info_done3:

  LocalFree (LocalHandle (pDriverInfo2));

display_prt_drvs_info_done2:

  LocalFree (LocalHandle (pDriverInfo1));

display_prt_drvs_info_done1:

  return bReturn;
}

