
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/****************************************************************************

    PROGRAM: Select.c

    PURPOSE: Contains library routines for selecting a region

    FUNCTIONS:

        StartSelection(HWND, POINT, LPRECT, int) - begin selection area
        UpdateSelection(HWND, POINT, LPRECT, int) - update selection area
        EndSelection(POINT, LPRECT) - end selection area
        ClearSelection(HWND, LPRECT, int) - clear selection area

*******************************************************************************/

#include "windows.h"
#include "select.h"

/****************************************************************************
   FUNCTION: DllMain(HANDLE, DWORD, LPVOID)

   PURPOSE:  DllMain is called by Windows when
             the DLL is initialized, Thread Attached, and other times.
             Refer to SDK documentation, as to the different ways this
             may be called.
             
             The DllMain function should perform additional initialization
             tasks required by the DLL.  In this example, no initialization
             tasks are required.  DllMain should return a value of 1 if
             the initialization is successful.
           
*******************************************************************************/
BOOL APIENTRY DllMain(HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
    return 1;
        UNREFERENCED_PARAMETER(hInst);
        UNREFERENCED_PARAMETER(ul_reason_being_called);
        UNREFERENCED_PARAMETER(lpReserved);
}


/****************************************************************************

    FUNCTION: StartSelection(HWND, POINT, LPRECT, int)

    PURPOSE: Begin selection of region

****************************************************************************/

INT APIENTRY StartSelection(
    HWND hWnd,
    MPOINT ptCurrent,
    LPRECT lpSelectRect,
    INT fFlags)
{
    if (lpSelectRect->left != lpSelectRect->right ||
            lpSelectRect->top != lpSelectRect->bottom)
        ClearSelection(hWnd, lpSelectRect, fFlags);

    lpSelectRect->right = ptCurrent.x;
    lpSelectRect->bottom = ptCurrent.y;

    /* If you are extending the box, then invert the current rectangle */

    if ((fFlags & SL_SPECIAL) == SL_EXTEND)
        ClearSelection(hWnd, lpSelectRect, fFlags);

    /* Otherwise, set origin to current location */

    else {
        lpSelectRect->left = ptCurrent.x;
        lpSelectRect->top = ptCurrent.y;
    }
    SetCapture(hWnd);
    return 1;
}

/****************************************************************************

    FUNCTION: UpdateSelection(HWND, POINT, LPRECT, int) - update selection area

    PURPOSE: Update selection

****************************************************************************/

INT APIENTRY UpdateSelection(
    HWND hWnd,
    MPOINT ptCurrent,
    LPRECT lpSelectRect,
    INT fFlags)
{
    HDC hDC;
    SHORT OldROP;

    hDC = GetDC(hWnd);

    switch (fFlags & SL_TYPE) {

        case SL_BOX:
            OldROP = (SHORT)SetROP2(hDC, R2_NOTXORPEN);
            MoveToEx(hDC, lpSelectRect->left, lpSelectRect->top, NULL);
            LineTo(hDC, lpSelectRect->right, lpSelectRect->top);
            LineTo(hDC, lpSelectRect->right, lpSelectRect->bottom);
            LineTo(hDC, lpSelectRect->left, lpSelectRect->bottom);
            LineTo(hDC, lpSelectRect->left, lpSelectRect->top);
            LineTo(hDC, ptCurrent.x, lpSelectRect->top);
            LineTo(hDC, ptCurrent.x, ptCurrent.y);
            LineTo(hDC, lpSelectRect->left, ptCurrent.y);
            LineTo(hDC, lpSelectRect->left, lpSelectRect->top);
            SetROP2(hDC, OldROP);
            break;
    
        case SL_BLOCK:
            PatBlt(hDC,
                lpSelectRect->left,
                lpSelectRect->bottom,
                lpSelectRect->right - lpSelectRect->left,
                ptCurrent.y - lpSelectRect->bottom,
                DSTINVERT);
            PatBlt(hDC,
                lpSelectRect->right,
                lpSelectRect->top,
                ptCurrent.x - lpSelectRect->right,
                ptCurrent.y - lpSelectRect->top,
                DSTINVERT);
            break;
    }
    lpSelectRect->right = ptCurrent.x;
    lpSelectRect->bottom = ptCurrent.y;
    ReleaseDC(hWnd, hDC);
    return 1;
}

/****************************************************************************

    FUNCTION: EndSelection(POINT, LPRECT)

    PURPOSE: End selection of region, release capture of mouse movement

****************************************************************************/

INT APIENTRY EndSelection(
    MPOINT ptCurrent,
    LPRECT lpSelectRect)
{
    lpSelectRect->right = ptCurrent.x;
    lpSelectRect->bottom = ptCurrent.y;
    ReleaseCapture();
    return 1;
}

/****************************************************************************

    FUNCTION: ClearSelection(HWND, LPRECT, int) - clear selection area

    PURPOSE: Clear the current selection

****************************************************************************/

INT APIENTRY ClearSelection(
    HWND hWnd,
    LPRECT lpSelectRect,
    INT fFlags)
{
    HDC hDC;
    INT2DWORD OldROP;

    hDC = GetDC(hWnd);
    switch (fFlags & SL_TYPE) {

        case SL_BOX:
            OldROP = SetROP2(hDC, R2_NOTXORPEN);
            MoveToEx(hDC, lpSelectRect->left, lpSelectRect->top, NULL);
            LineTo(hDC, lpSelectRect->right, lpSelectRect->top);
            LineTo(hDC, lpSelectRect->right, lpSelectRect->bottom);
            LineTo(hDC, lpSelectRect->left, lpSelectRect->bottom);
            LineTo(hDC, lpSelectRect->left, lpSelectRect->top);
            SetROP2(hDC, OldROP);
            break;

        case SL_BLOCK:
            PatBlt(hDC,
                lpSelectRect->left,
                lpSelectRect->top,
                lpSelectRect->right - lpSelectRect->left,
                lpSelectRect->bottom - lpSelectRect->top,
                DSTINVERT);
            break;
    }
    ReleaseDC(hWnd, hDC);
    return 1;
}
