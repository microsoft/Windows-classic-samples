
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
*  MODULE:      PAINT.C
*
*  PURPOSE:     Given an HDC and a pointer to a bounding rectangle
*               draw all graphics/fonts based on the flags set in
*               the dwGraphicsOptions global variable.
*
*  FUNTIONS:    Paint              - main painting routine
*               GetFirstGraphicSlot- computes bounding rect of 1st
*                                      graphic
*               GetNextGraphicSlot - computes bounding rect of next
*                                      graphic
*               DrawFonts          - draws enumerated fonts
*               BuildFontList      - builds a list of fonts of fonts
*                                      supported by a given DC
*               MyEnumFaces        - enumerates the font facenames
*                                      supported by a given DC
*               MyEnumCopy         - copies LOGFONT & TEXTMETRIC info
*                                      to a global variable
*               MyEnumCount        - counts total number of fonts
*                                      supported by a given DC
*               FreeFontList       - frees a (BuildFontList-) font list
*
*  COMMENTS:    Most of the font-enumeration code "lifted" from NTF.EXE
*               sample. For more complete documentation have a look a
*               that.
*
\******************************************************************************/

#include <windows.h>
#include <string.h>
#include "common.h"
#include "paint.h"
#include "resource.h"


/******************************************************************************\
*
*  FUNCTION:    Paint
*
*  INPUTS:      hdc    - device context to paint
*               lpRect - bounding rectangle of device to paint
*
*  RETURNS:     TRUE if painting went ok, or
*               FALSE if error while painting
*
\******************************************************************************/

BOOL Paint (HDC hdc, LPRECT lpRect)
{
	RECT  rect;
	HPEN  hpenSave;
	RECTI ri;
	
	if (gdwGraphicsOptions & DRAWAXIS)
	{
		//
		// draw axis
		//
		
		rect = *lpRect;
		
		hpenSave = SelectObject (hdc, GetStockObject (BLACK_PEN));
		
		//
		// if giMapMode == MM_TEXT, MM_ANISOTROPIC then (0,0) is upper left corner
		//
		
		if (giMapMode == MM_TEXT || giMapMode == MM_ANISOTROPIC)
		{
			MoveToEx (hdc, rect.left - glcyMenu/2, rect.bottom, NULL);
			LineTo   (hdc, rect.left,  rect.bottom + glcyMenu/2);
			LineTo   (hdc, rect.left + glcyMenu/2,  rect.bottom);
			
			MoveToEx (hdc, rect.left,  rect.bottom + glcyMenu/2, NULL);
			LineTo   (hdc, rect.left,  rect.top);
			LineTo   (hdc, rect.right + glcyMenu/2, rect.top);
			
			MoveToEx (hdc, rect.right, rect.top - glcyMenu/2, NULL);
			LineTo   (hdc, rect.right + glcyMenu/2,  rect.top);
			LineTo   (hdc, rect.right,  rect.top + glcyMenu/2);
		}
		
		//
		// else (0,0) is lower left corner
		//
		
		else
		{
			MoveToEx (hdc, rect.left - glcyMenu/2, rect.bottom, NULL);
			LineTo   (hdc, rect.left,  rect.bottom + glcyMenu/2);
			LineTo   (hdc, rect.left + glcyMenu/2,  rect.bottom);
			
			MoveToEx (hdc, rect.left,  rect.bottom + glcyMenu/2, NULL);
			LineTo   (hdc, rect.left,  rect.top);
			LineTo   (hdc, rect.right + glcyMenu/2, rect.top);
			
			MoveToEx (hdc, rect.right, rect.top + glcyMenu/2, NULL);
			LineTo   (hdc, rect.right + glcyMenu/2, rect.top);
			LineTo   (hdc, rect.right, rect.top - glcyMenu/2);
		}
		
		SelectObject (hdc, hpenSave);
	}
	
	//
	// look at bits in gdwGraphicsOptions & determine which graphics to draw
	//
	
	if (gdwGraphicsOptions & ENUMFONTS)
	{
		DrawFonts (hdc, lpRect);
		return TRUE;
	}
	
	giDeltaX = (int) ((lpRect->right - BORDER) / NUM_GRAPHICS_XSLOTS);
	giDeltaY = (int) ((lpRect->bottom - lpRect->top - BORDER)
		/ NUM_GRAPHICS_YSLOTS);
	
	GetFirstGraphicSlot (lpRect, &ri);
	
	if (gdwGraphicsOptions & ARC)
	{
		Arc (hdc, ri.left, ri.top, ri.right, ri.bottom,
			ri.left, ri.top, ri.right-10, ri.bottom-10);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & ELLIPSE)
	{
		Ellipse (hdc, ri.left, ri.top, ri.right, ri.bottom);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & LINETO)
	{
		int i;
		
		for (i = PS_SOLID; i <= PS_DASHDOTDOT; i++)
		{
			MoveToEx (hdc, ri.left, ri.top + (i+1)*giDeltaY/7, NULL);
			LineTo   (hdc, ri.right, ri.top + (i+1)*giDeltaY/7);
		}
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & PIE)
	{
		Pie (hdc, ri.left, ri.top, ri.right, ri.bottom,
			ri.left, ri.top, ri.right-10, ri.bottom-10);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & PLG_BLT)
	{
		HBITMAP hbm;
		BITMAP  bm;
		HDC     hdcMem;
		POINT   ap[3];
		
		hbm = LoadBitmap (ghInst, "printer");
		hdcMem = CreateCompatibleDC (hdc);
		SelectObject (hdcMem, hbm);
		
		GetObject (hbm, sizeof(BITMAP), (LPSTR)&bm);
		
		//
		// special case here because PlgBlt requires coordinates
		//   of upper left, upper right, & lower left
		//
		
		ap[0].x = (LONG) (ri.left + (ri.right - ri.left)/4);
		ap[0].y = (LONG) (ri.top + (ri.bottom - ri.top)/4);
		ap[1].x = (LONG) ri.right;
		ap[1].y = (LONG) ri.top;
		ap[2].x = (LONG) ri.left;
		ap[2].y = (LONG) ri.bottom;
		
		PlgBlt (hdc, ap, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, NULL, 0, 0);
		
		DeleteDC(hdcMem);
		DeleteObject(hbm);
		GetNextGraphicSlot (&ri);
		
	}
	
	if (gdwGraphicsOptions & POLYBEZIER)
	{
		POINT ap[4];
		
		ap[0].x  = (LONG) ri.left;    ap[0].y = (LONG) ri.top;
		ap[1].x  = (LONG) ri.left;    ap[1].y = (LONG) ri.bottom;
		ap[2].x  = (LONG) ri.right;   ap[2].y = (LONG) ri.top;
		ap[3].x  = (LONG) ri.right;   ap[3].y = (LONG) ri.bottom;
		
		PolyBezier (hdc, ap, 4);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & POLYGON)
	{
		POINT ap[5];
		
		ap[0].x  = (LONG) ri.left;    ap[0].y = (LONG) ri.top;
		ap[1].x  = (LONG) ri.right;   ap[1].y = (LONG) ri.bottom;
		ap[2].x  = (LONG) ri.right;   ap[2].y = (LONG) ri.top;
		ap[3].x  = (LONG) ri.left;    ap[3].y = (LONG) ri.bottom;
		ap[4].x  = (LONG) ri.left;    ap[4].y = (LONG) ri.top;
		
		Polygon (hdc, ap, 4);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & POLYLINE)
	{
		POINT ap[4];
		
		ap[0].x  = (LONG) ri.left;    ap[0].y = (LONG) ri.top;
		ap[1].x  = (LONG) ri.left;    ap[1].y = (LONG) ri.bottom;
		ap[2].x  = (LONG) ri.right;   ap[2].y = (LONG) ri.top;
		ap[3].x  = (LONG) ri.right;   ap[3].y = (LONG) ri.bottom;
		
		Polyline (hdc, ap, 4);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & POLYPOLYGON)
	{
		POINT ap[8];
		int ai[2] = { 4, 4 };
		
		ap[0].x = (LONG)  ri.left;
		ap[0].y = (LONG)  ri.top;
		ap[1].x = (LONG) (ri.left + giDeltaX/4);
		ap[1].y = (LONG)  ri.top;
		ap[2].x = (LONG) (ri.left + giDeltaX/4);
		ap[2].y = (LONG) (ri.top  + giDeltaY/4);
		ap[3].x = (LONG)  ri.left;
		ap[3].y = (LONG) (ri.top  + giDeltaY/4);
		ap[4].x = (LONG) (ri.right  - 2*giDeltaX/3);
		ap[4].y = (LONG) (ri.bottom - 2*giDeltaY/3);
		ap[5].x = (LONG)  ri.right;
		ap[5].y = (LONG) (ri.bottom - 2*giDeltaY/3);
		ap[6].x = (LONG)  ri.right;
		ap[6].y = (LONG)  ri.bottom;
		ap[7].x = (LONG) (ri.right  - 2*giDeltaX/3);
		ap[7].y = (LONG)  ri.bottom;
		
		PolyPolygon (hdc, ap, ai, 2);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & RECTANGLE)
	{
		Rectangle (hdc, ri.left, ri.top, ri.right, ri.bottom);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & ROUNDRECT)
	{
		RoundRect (hdc, ri.left, ri.top, ri.right, ri.bottom, 15, 25);
		GetNextGraphicSlot (&ri);
	}
	
	if (gdwGraphicsOptions & STRETCH_BLT)
	{
		HBITMAP hbm;
		BITMAP  bm;
		HDC     hdcMem;
		
		hbm = LoadBitmap (ghInst, "printer");
		hdcMem = CreateCompatibleDC (hdc);
		SelectObject (hdcMem, hbm);
		
		GetObject (hbm, sizeof(BITMAP), (LPSTR)&bm);
		
		StretchBlt (hdc, ri.left, ri.top, ri.right-ri.left,
			ri.bottom - ri.top, hdcMem, 0, 0,
			bm.bmWidth, bm.bmHeight, SRCCOPY);
		
		DeleteDC(hdcMem);
		DeleteObject(hbm);
		GetNextGraphicSlot (&ri);
	}
	
	return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    GetFirstGraphicSlot
*
*  INPUTS:      pri - pointer to a RECTI
*
\******************************************************************************/

void GetFirstGraphicSlot (LPRECT lpRect, PRECTI pri)
{
  pri->left   = BORDER;
  pri->top    = lpRect->top + BORDER;
  pri->right  = giDeltaX;
  pri->bottom = giDeltaY;

  giColumn = 1;
}



/******************************************************************************\
*
*  FUNCTION:    GetNextGraphicSlot
*
*  INPUTS:      pri - pointer to a RECTI
*
\******************************************************************************/

void GetNextGraphicSlot (PRECTI pri)
{
  if (++giColumn <= NUM_GRAPHICS_XSLOTS)
  {
    pri->left  += giDeltaX;
    pri->right += giDeltaX;
  }
  else
  {
    giColumn = 1;

    pri->left   =  BORDER;
    pri->top    += giDeltaY;
    pri->right  =  giDeltaX;
    pri->bottom += giDeltaY;
  }
}



/******************************************************************************\
*
*  FUNCTION:    DrawFonts
*
*  INPUTS:      hdc   - device context to enumerate from & draw on
*               pRect - pointer to bounding rect to draw fonts in
*
*  LOCAL VARS:  i, j       - loop variables
*               xText      - starting x position to draw text
*               yText      - starting y position to draw text
*               iMaxStrLen - length in pels of string to draw
*
\******************************************************************************/

void DrawFonts (HDC hdc, LPRECT lpRect)
{
  int      i, j, xText, yText, iMaxStrLen = 0;
  PARFONTS paf;

  paf = BuildFontList (hdc);

  xText = yText = 2;

  //
  // set the appropriate text align mode depending on
  //   whether we're drawing from top down or bottom up
  //

  if ((giMapMode == MM_TEXT) || (giMapMode == MM_ANISOTROPIC))

    SetTextAlign (hdc, TA_TOP);

  else

    SetTextAlign (hdc, TA_BOTTOM);

  for (i = 0; i < nFaces; i++)
  {
    for (j = 0; j < (paf + i)->nFonts; j++)
    {
      HFONT      hFont, hSaveFont;
      SIZE       size;
      POINT      LogPtExtent;

      LogPtExtent.x = lpRect->right;
      LogPtExtent.y = lpRect->bottom;

      hFont = CreateFontIndirect ((paf + i)->lf + j);

      hSaveFont = SelectObject (hdc, hFont);

      TextOut (hdc, xText, yText, ((paf + i)->lf + j)->lfFaceName,
               (int) (strlen(((paf + i)->lf + j)->lfFaceName)));

      GetTextExtentPoint (hdc, ((paf + i)->lf + j)->lfFaceName,
                          (int) (strlen(((paf+i)->lf+j)->lfFaceName)),
                          &size);
      size.cx += 2;
      iMaxStrLen = iMaxStrLen > (int)size.cx ? iMaxStrLen: (int) size.cx;

      if (!(i == (nFaces - 1) && j == ((paf + i)->nFonts - 1)))
      {
        TEXTMETRIC *pNextTM;

        pNextTM = j < ((paf+i)->nFonts-1) ?
                          (paf+i)->tm+j+1 : (paf+i+1)->tm;

        //
        // add in the height of the face name we just drew
        //

        yText += (int) ((paf + i)->tm + j)->tmHeight;

        //
        // if the next facename will be drawn outside the bounding rect then
        //   start at first line of next column
        //

        if (yText + (int) pNextTM->tmHeight > (int) LogPtExtent.y)
        {
          yText = 2;
          xText += iMaxStrLen + 2;
          iMaxStrLen = 0;
        }
      }

      SelectObject (hdc, hSaveFont);
      DeleteObject (hFont);

      if (xText > (int) LogPtExtent.x)
      {
        if (GetDeviceCaps (hdc, TECHNOLOGY) & DT_RASDISPLAY)
        {
          //
          // If we're drawing to the screen & have run out of
          //   room then tell user how many fonts there are left
          //   (that we haven't displayed)
          //

          int   k;
          int   iFontsLeft = (paf + i)->nFonts - j - 1;
          char  buf[40];
          SIZE  size;


          for (k = i + 1; k < nFaces; k++)

            iFontsLeft += (paf + k)->nFonts;

          sprintf_s (buf, _countof(buf), GetStringRes(IDS_FMT_MOREDSPFNTNL),
                     iFontsLeft);

          GetTextExtentPoint (hdc, buf, (int) strlen(buf), &size);

          if ((xText = lpRect->right - size.cx) < glcyMenu + 1)

            xText = glcyMenu/2 + 1;

          TextOut (hdc, xText, lpRect->bottom, buf, (int) strlen(buf));

          goto done_enumfonts;
        }

        else
        {
          //
          // Else we're drawing to a printer & have filled up
          //   the first page. If there's any fonts left to draw
          //   then start a new page.
          //

          if (!(i == nFaces - 1 && j == (paf + i)->nFonts - 1))
          {
            EndPage   (hdc);
            xText = yText = 2;
            StartPage (hdc);
          }
        }
      }
    }
  }

done_enumfonts:

  FreeFontList (paf);
}




/*  In the callback functions for the enumerations, there is a limited
 *  ability to pass in parameters.  For that reason, declare the following
 *  global variables to be used by any of the call back functions.
 *
 *        HDC      hdcGlobal;
 *        PARFONTS parFontsGlobal;
 *        int      iFace,jFont;
 *        int      nFaces;
 *
 *
 * General call structure:
 *
 *        BuildFontList
 *            EnumFonts
 *                MyEnumCount
 *            LocalAlloc
 *            EnumFonts
 *                MyEnumFaces
 *                    EnumFonts
 *                        MyEnumCount
 *                    LocalAlloc
 *                    LocalAlloc
 *                    LocalAlloc
 *                    EnumFonts
 *                        MyEnumCopy
 */



/******************************************************************************\
*
*  FUNCTION:    BuildFontList
*
*  GLOBAL VARS: (see above)
*
\******************************************************************************/

PARFONTS BuildFontList (HDC hdcIn)
{
  nFaces = 0;

  hdcGlobal = hdcIn;

  //
  // count the total number of face names.
  //

  EnumFonts (hdcGlobal, NULL, (FONTENUMPROC)MyEnumCount, (LPARAM)&nFaces);

  //
  // allocate the pointer to the array of PArFont structures.
  //

  parFontsGlobal = (PARFONTS) LocalAlloc (LPTR, sizeof(ARFONTS) * (nFaces+1));

  //
  // step through all fonts again.  For each one fill a LOGFONT and
  //   a TEXTMETRIC stucture.
  //

  iFace = 0;
  EnumFonts (hdcGlobal, NULL, (FONTENUMPROC)MyEnumFaces, 0);

  return parFontsGlobal;
}



/******************************************************************************\
*
*  FUNCTION:    MyEnumFaces
*
*  GLOBAL VARS: (see above)
*
\******************************************************************************/

int CALLBACK MyEnumFaces (LPLOGFONT lpLogFont, LPTEXTMETRIC lpTEXTMETRICs,
                          DWORD fFontType, LPVOID  lpData)
{
  int nFonts;

  nFonts = 0;
  EnumFonts (hdcGlobal, lpLogFont->lfFaceName, (FONTENUMPROC)MyEnumCount,
             (LPARAM)&nFonts);

  parFontsGlobal[iFace].lf   = (LOGFONT *)    LocalAlloc (LPTR,
                                                          sizeof(LOGFONT) *
                                                            nFonts);
  parFontsGlobal[iFace].tm   = (TEXTMETRIC *) LocalAlloc (LPTR,
                                                          sizeof(TEXTMETRIC) *
                                                            nFonts);
  parFontsGlobal[iFace].Type = (int *)        LocalAlloc (LPTR,
                                                          sizeof(int) *
                                                            nFonts);

  if ((parFontsGlobal[iFace].lf   == NULL) ||
      (parFontsGlobal[iFace].tm   == NULL) ||
      (parFontsGlobal[iFace].Type == NULL))
  {
    ErrMsgBox (GetStringRes(IDS_LALLOCFAIL), ERR_MOD_NAME);
    return FALSE;
  }

  parFontsGlobal[iFace].nFonts = nFonts;

  jFont = 0;
  EnumFonts (hdcGlobal, lpLogFont->lfFaceName, (FONTENUMPROC)MyEnumCopy, 0);

  iFace++;

  return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    MyEnumCopy
*
*  GLOBAL VARS: (see above)
*
\******************************************************************************/

int CALLBACK MyEnumCopy (LPLOGFONT lpLogFont, LPTEXTMETRIC lpTEXTMETRICs,
                         DWORD fFontType, LPVOID  lpData)
{
  LOGFONT    *lplf;
  TEXTMETRIC *lptm;
  int        *pType;

  lplf  = parFontsGlobal[iFace].lf;
  lptm  = parFontsGlobal[iFace].tm;
  pType = parFontsGlobal[iFace].Type;

  lplf[jFont]  = *lpLogFont;
  lptm[jFont]  = *lpTEXTMETRICs;
  pType[jFont] = fFontType;

  jFont++;
  return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    MyEnumCount
*
*  GLOBAL VARS: (see above)
*
\******************************************************************************/

int CALLBACK MyEnumCount (LPLOGFONT lpLogFont, LPTEXTMETRIC lpTEXTMETRICs,
                          DWORD fFontType, LPVOID lpData)
{
  (*(LPINT)lpData)++;
  return TRUE;
}



/******************************************************************************\
*
*  FUNCTION:    FreeFontList
*
*  INPUTS:      paf - pointer to ARFONTS struct to free
*
\******************************************************************************/

void FreeFontList (PARFONTS paf)
{
  int i;

  for (i = 0; i < nFaces; i++)
  {
    LocalFree (LocalHandle ((LPSTR) ((paf + i)->lf  )));
    LocalFree (LocalHandle ((LPSTR) ((paf + i)->tm  )));
    LocalFree (LocalHandle ((LPSTR) ((paf + i)->Type)));
  }
  LocalFree (LocalHandle ((LPSTR) paf));
}

