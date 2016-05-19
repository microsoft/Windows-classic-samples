// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 *
 * bar.cpp
 *
 * supports bar window graphically showing two lists of
 * sections.
 *
 * showing coloured vertical bars for the sections  of text,
 * with linking lines for the sections that are the same.
 *
 * get the sections by sending TM_CURRENTVIEW to hwndClient
 */

/*---includes-----------------------------------------------------------*/

#include "precomp.h"
#include "table.h"

#include "state.h"
#include "wdiffrc.h"
#include "sdkdiff.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"
#include "view.h"

/*--- forward declaration of functions--------------- */

INT_PTR APIENTRY BarWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void BarPaint(HWND hwnd);
void DrawSection(HDC hdc, int cx, int cy, int lines, SECTION sec, int sidecode);
void DrawLink(HDC hdc, int cx, int cy, int lines, SECTION sec);
void BarClick(HWND hwnd, int x, int y);

/* --- globals and constants -------------------------*/

HPEN hpenSame, hpenLeft, hpenRight;
HBRUSH hbrSame, hbrLeft, hbrRight;
HBRUSH hbrSideBar;

/*-- externally called functions---------------------------------------*/

/* InitBarClass
 *
 * - create bar window class
 */
BOOL
InitBarClass(HINSTANCE hInstance)
{
    WNDCLASS    wc;
    BOOL resp;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = (WNDPROC)BarWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = (LPSTR) "BarClass";
    wc.lpszMenuName = NULL;

    resp = RegisterClass(&wc);

    return(resp);
}



/* winproc supporting bar window painting etc
 *
 */

INT_PTR
APIENTRY
BarWndProc(
          HWND hWnd,
          UINT message,
          WPARAM wParam,
          LPARAM lParam
          )
{
    switch (message) {
    case WM_CREATE:

        hpenSame = CreatePen(PS_SOLID, 1, RGB(0,0,0));
        hbrSame = CreateSolidBrush(RGB(255,255,255));

        hpenLeft = CreatePen(PS_SOLID, 1, rgb_barleft);
        hbrLeft = CreateSolidBrush(rgb_barleft);

        hpenRight = CreatePen(PS_SOLID, 1, rgb_barright);
        hbrRight = CreateSolidBrush(rgb_barright);

        hbrSideBar = CreateSolidBrush(rgb_barcurrent);
        break;

    case WM_DESTROY:
        DeleteObject(hpenSame);
        DeleteObject(hpenLeft);
        DeleteObject(hpenRight);
        DeleteObject(hbrSame);
        DeleteObject(hbrLeft);
        DeleteObject(hbrRight);
        DeleteObject(hbrSideBar);
        break;

    case WM_PAINT:
        BarPaint(hWnd);
        break;

    case WM_LBUTTONDOWN:
        BarClick(hWnd, LOWORD(lParam), HIWORD(lParam));
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        case IDM_MONOCOLS:
            DeleteObject(hpenLeft);
            DeleteObject(hpenRight);
            DeleteObject(hbrLeft);
            DeleteObject(hbrRight);
            DeleteObject(hbrSideBar);

            hpenLeft = CreatePen(PS_SOLID, 1, rgb_barleft);
            hbrLeft = CreateSolidBrush(rgb_barleft);
            hpenRight = CreatePen(PS_SOLID, 1, rgb_barright);
            hbrRight = CreateSolidBrush(rgb_barright);
            hbrSideBar = CreateSolidBrush(rgb_barcurrent);

            break;
        default: /* no action */
            break;
        }
        break;

    default:
        return(DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0;
}

/* draw the current position as side-bars down the bar window,
 * showing which lines from each file are currently in view. HDC can be
 * NULL (we get one ourselves if so). If bErase is true, we clear
 * the previous side-bars first.
 *
 * this is called from BarPaint when we paint the whole window, and
 * from TableServer() whenever it receives a TQ_SCROLL notification that
 * the table window has been scrolled.
 */
void
BarDrawPosition(HWND hwndBar, HDC hdcIn, BOOL bErase)
{
    HDC hdc;
    int total_lines, cy, cx;
    RECT rc, rcLeft, rcRight;
    VIEW view;
    COMPITEM item;
    LIST listleft, listright;
    long toprow, endrow, i;
    int left_first, left_last, right_first, right_last, linenr;

    /* get a hdc if we weren't given one */
    if (hdcIn == NULL) {
        hdc = GetDC(hwndBar);
        if (!hdc)
            return;
    } else {
        hdc = hdcIn;
    }

    /* set horz position of bars */
    GetClientRect(hwndBar, &rc);
    cx = (int)(rc.right - rc.left);
    cy = (int)(rc.bottom - rc.top);

    /* layout constants are defined as percentages of window width */
    rcLeft.left = cx * L_POS_START / 100;
    rcRight.left = cx * R_POS_START / 100;
    rcLeft.right = rcLeft.left +  (cx * L_POS_WIDTH / 100);
    rcRight.right = rcRight.left +  (cx * R_POS_WIDTH / 100);

    /* erase the whole marker section if requested */
    if (bErase) {
        rcLeft.top = rc.top;
        rcLeft.bottom = rc.bottom;
        rcRight.top = rc.top;
        rcRight.bottom = rc.bottom;

        FillRect(hdc, &rcLeft, (HBRUSH)GetStockObject(WHITE_BRUSH));

        FillRect(hdc, &rcRight, (HBRUSH)GetStockObject(WHITE_BRUSH));
    }


    /*
     * calculate the vertical scaling - depends on the
     * total number of lines shown
     */

    /* get the handles to the two lists of sections */
    view = (VIEW) SendMessage(hwndClient, TM_CURRENTVIEW, 0, 0);
    /* make sure we are in expand mode */
    if (view_isexpanded(view) == FALSE) {
        /* get rid of the dc if we made it ourselves */
        if (hdcIn == NULL) {
            ReleaseDC(hwndBar, hdc);
        }
        return;
    }

    item = view_getitem(view, 0);

    listleft = compitem_getleftsections(item);
    listright = compitem_getrightsections(item);

    /* if there is only one list of sections, draw nothing. The
     * picture for a single file is not very exciting.
     */

    if ((listleft == NULL) || (listright == NULL)) {
        /* get rid of the dc if we made it ourselves */
        if (hdcIn == NULL) {
            ReleaseDC(hwndBar, hdc);
        }
        return;
    }

    /* take the longest of the two files and use this
     * for vertical scaling. the scale is such that the longest file
     * *just fits*.
     */
    total_lines = line_getlinenr(section_getlastline((SECTION)List_Last(listleft)));
    total_lines = max(total_lines,
                      (int) line_getlinenr(section_getlastline((SECTION)List_Last(listright))));

    /* get the current top row and nr of rows visible */
    toprow = (LONG)SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);
    endrow = (LONG)SendMessage(hwndRCD, TM_ENDROW, FALSE, 0);
    endrow = min(endrow, view_getrowcount(view)-1);

    /*
     * find the first and last line nrs from each file currently visible.
     *
     */
    left_first = left_last = right_first = right_last = 0;

    for (i = toprow; i <= endrow; i++) {
        linenr = view_getlinenr_left(view, i);

        if (linenr > 0) {

            if (left_first == 0) {
                left_first = linenr;
            }
            left_first = min(left_first, linenr);
            left_last = max(left_last, linenr);
        }

        linenr = view_getlinenr_right(view, i);
        if (linenr > 0) {
            if (right_first == 0) {
                right_first = linenr;
            }
            right_first = min(right_first, linenr);
            right_last = max(right_last, linenr);
        }
    }

    /* draw the two markers as thick bars -> elongated rectangles */
    rcLeft.top = MulDiv(left_first-1, cy, total_lines);
    rcLeft.bottom = MulDiv(left_last, cy, total_lines);
    FillRect(hdc, &rcLeft, hbrSideBar);

    rcRight.top = MulDiv(right_first-1, cy, total_lines);
    rcRight.bottom = MulDiv(right_last, cy, total_lines);
    FillRect(hdc, &rcRight, hbrSideBar);

    /* get rid of the dc if we made it ourselves */
    if (hdcIn == NULL) {
        ReleaseDC(hwndBar, hdc);
    }
}


/*--- internal functions -------------------------------------------*/

/* paint the bar window */
void
BarPaint(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC hdc;
    VIEW view;
    COMPITEM item;
    LIST listleft, listright;
    SECTION sec;
    int total_lines, cx, cy;
    RECT rc;

    hdc = BeginPaint(hwnd, &ps);

    /* draw a separator line at the very edge of the window */
    GetClientRect(hwnd, &rc);
    MoveToEx(hdc, (int)(rc.right-1), rc.top, NULL);
    LineTo(hdc, (int)(rc.right-1), rc.bottom);


    /* first gather information about what is to be displayed */

    /* find the total lines (for horz. scaling) */

    /* get the handles to the two lists of sections */
    view = (VIEW) SendMessage(hwndClient, TM_CURRENTVIEW, 0, 0);

    /* make sure we are in expand mode */
    if (view_isexpanded(view) == FALSE) {
        return;
    }

    item = view_getitem(view, 0);

    listleft = compitem_getleftsections(item);
    listright = compitem_getrightsections(item);

    /*
     * don't bother if there is only one list - not very interesting
     */
    if ((listleft == NULL) || (listright == NULL)) {
        EndPaint(hwnd, &ps);
        return;
    }

    /* take the longest of the two files and use this
     * for vertical scaling. the scale is such that the longest file
     * *just fits*.
     */
    total_lines = (int) line_getlinenr(section_getlastline((SECTION)List_Last(listleft)));
    total_lines = max(total_lines,
                      (int) line_getlinenr(section_getlastline((SECTION)List_Last(listright))));

    /* horizontal spacing:
     *
     * there are two columns, for the left and right files, and a gap
     * between them criss-crossed by lines marking the links.
     *
     * Each of the columns then has three sections, for the
     * position marker, the different sections
     * and the linked sections. The width and positions of these items
     * are defined (in sdkdiff.h) as percentages of the window width.
     */

    cx = (int)(rc.right - rc.left);
    cy = (int)(rc.bottom - rc.top);

    /* draw all the left sections and links */
    for ( sec=(SECTION)List_First(listleft);  sec!=NULL;  sec = (SECTION)List_Next((LPVOID)sec)) {
        DrawSection(hdc, cx, cy, total_lines, sec, STATE_LEFTONLY);

        if (section_getlink(sec) != NULL) {
            DrawLink(hdc, cx, cy, total_lines, sec);
        }
    }

    /* draw all the right sections */
    for ( sec=(SECTION)List_First(listright);  sec!=NULL;  sec = (SECTION)List_Next((LPVOID)sec)) {
        DrawSection(hdc, cx, cy, total_lines, sec, STATE_RIGHTONLY);
    }

    /* now draw current position markers */
    BarDrawPosition(hwnd, hdc, FALSE);

    EndPaint(hwnd, &ps);
}


/* paint a single section */
void
DrawSection(HDC hdc, int cx, int cy, int lines, SECTION sec, int sidecode)
{
    int x1, y1, x2, y2;
    HPEN hpenOld;
    HBRUSH hbrOld;

    /* calculate the vertical position from the scaling. the scaling
     * is such that the longest file just fits
     */
    y1 = MulDiv(line_getlinenr(section_getfirstline(sec))- 1, cy, lines);
    y2 = MulDiv(line_getlinenr(section_getlastline(sec)), cy, lines);


    /* left or right  - set bar position and width*/
    if (sidecode == STATE_LEFTONLY) {
        if (section_getlink(sec) != NULL) {
            x1 = L_MATCH_START;
            x2 = L_MATCH_WIDTH;
        } else {
            x1 = L_UNMATCH_START;
            x2 = L_UNMATCH_WIDTH;
        }
    } else {
        if (section_getlink(sec) != NULL) {
            x1 = R_MATCH_START;
            x2 = R_MATCH_WIDTH;
        } else {
            x1 = R_UNMATCH_START;
            x2 = R_UNMATCH_WIDTH;
        }
    }
    /* bar position defines are in percentages of the win width (cx) */
    x1 = cx * x1 / 100;
    x2 = (cx * x2 / 100) + x1;


    /* select pens and brushes */
    if (section_getlink(sec) != NULL) {
        hpenOld = (HPEN)SelectObject(hdc, hpenSame);
        hbrOld = (HBRUSH)SelectObject(hdc, hbrSame);
    } else if (sidecode == STATE_LEFTONLY) {
        hpenOld = (HPEN)SelectObject(hdc, hpenLeft);
        hbrOld = (HBRUSH)SelectObject(hdc, hbrLeft);
    } else {
        hpenOld = (HPEN)SelectObject(hdc, hpenRight);
        hbrOld = (HBRUSH)SelectObject(hdc, hbrRight);
    }

    /* draw the section as a coloured elongated rectangle */
    Rectangle(hdc, x1, y1, x2, y2);

    /* de-select the pen and brush in favour of the default */
    SelectObject(hdc, hpenOld);
    SelectObject(hdc, hbrOld);
}

/* draw a line linking two sections. Indicates a section from each
 * file that match each other. psec points to the section in the
 * left file.
 */
void
DrawLink(HDC hdc, int cx, int cy, int lines, SECTION sec)
{
    int x1, y1, x2, y2;
    int ybase, yrange;
    SECTION other;

    other = section_getlink(sec);

    /* position the link line halfway down the section
     * - allow for the case where
     * the section is one line (ie halve the co-ords, not the line nr)
     */
    ybase = MulDiv(line_getlinenr(section_getfirstline(sec)) - 1, cy, lines);
    yrange = MulDiv(line_getlinenr(section_getlastline(sec)), cy, lines);
    y1 = ((yrange - ybase) / 2) + ybase;

    ybase = MulDiv(line_getlinenr(section_getfirstline(other)) - 1, cy, lines);
    yrange = MulDiv(line_getlinenr(section_getlastline(other)), cy, lines);
    y2 = ((yrange - ybase) / 2) + ybase;

    /* horizontal layout constants are defined as percentages of the
     * window width
     */
    x1 = cx * (L_MATCH_START + L_MATCH_WIDTH) / 100;
    x2 = cx * R_UNMATCH_START / 100;

    MoveToEx(hdc, x1, y1, NULL);
    LineTo(hdc, x2, y2);
}


/* the user has clicked on the bar window. Translate the clicked position into
 * a line in one of the files if possible, and scroll the table window to
 * show that line.
 */
void
BarClick(HWND hwnd, int x, int y)
{
    RECT rc;
    int xleft, xright;
    int linenr, i, that;
    BOOL bIsLeft;
    int tot_left, tot_right, total_lines;
    LIST listleft, listright;
    VIEW view;
    COMPITEM item;
    TableSelection select;

    /* find size of the window to get horz scaling, and see
     * where click was
     */
    GetClientRect(hwnd, &rc);

    /* was it near either of the bars ? */

    /* horz positioning is in percentages of window width */
    xleft = max(L_UNMATCH_START + L_UNMATCH_WIDTH,
                L_MATCH_START + L_MATCH_WIDTH);
    xright = min(R_UNMATCH_START, R_MATCH_START);
    xleft = xleft * (rc.right - rc.left) / 100;
    xright = xright * (rc.right - rc.left) / 100;

    if (x < xleft) {
        bIsLeft = TRUE;
    } else if (x > xright) {
        bIsLeft = FALSE;
    } else {
        /* click was between the two bars - ignore it */
        return;
    }

    /* calculate the vertical scaling (based on total lines displayed)
     * so that we can convert the y position into a line nr
     */

    /* get the handles to the two lists of sections */
    view = (VIEW) SendMessage(hwndClient, TM_CURRENTVIEW, 0, 0);

    /* make sure we are in expand mode */
    if (view_isexpanded(view) == FALSE) {
        return;
    }

    item = view_getitem(view, 0);

    listleft = compitem_getleftsections(item);
    listright = compitem_getrightsections(item);

    /* ignore the click if only one list of sections, since in
     * this case there is nothing drawn for him to click on.
     */
    if ((listleft == NULL) || (listright == NULL)) {
        return;
    }

    /* take the longest of the two files and use this
     * for vertical scaling. the scale is such that the longest file
     * *just fits*.
     */
    tot_left = line_getlinenr(section_getlastline((SECTION)List_Last(listleft)));
    tot_right = line_getlinenr(section_getlastline((SECTION)List_Last(listright)));

    total_lines = max(tot_left, tot_right);


    /* convert vertical position into a line nr. The vertical scaling
     * can be calculated from knowing that the longest list of
     * lines just fits in the window.
     * Don't use MulDiv, as we don't want to round the result - so
     * cast to long so maths is 32-bit even on win3.1
     */
    linenr = (int) (((long) total_lines * y) / (rc.bottom - rc.top)) + 1;

    /* check that the line is valid */
    if (bIsLeft) {
        if (linenr > tot_left) {
            return;
        }
    } else {
        if (linenr > tot_right) {
            return;
        }
    }

    /* search the current view, looking for a row with this
     * line nr on the correct side
     */
    for (i = 0; i < view_getrowcount(view); i++) {
        if (bIsLeft) {
            that = view_getlinenr_left(view,i);
        } else {
            that = view_getlinenr_right(view,i);
        }

        if (linenr == that) {
            /* found the matching line- select it in the
             * table window
             */
            select.startrow = i;
            select.startcell = 0;
            select.nrows = 1;
            select.ncells = 1;
            SendMessage(hwndRCD, TM_SELECT, 0, (LPARAM)&select);
            return;
        }
    }

    sdkdiff_UI(TRUE);
    MessageBox(hwndClient, LoadRcString(IDS_LINE_NOT_VISIBLE),
               "SdkDiff", MB_ICONSTOP|MB_OK);
    sdkdiff_UI(FALSE);
}
