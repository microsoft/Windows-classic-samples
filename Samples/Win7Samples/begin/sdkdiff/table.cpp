// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * standard table class.
 *
 * main interface functions.
 *
 * see table.h for interface description
 */

#include "precomp.h"

#include "table.h"
#include "tpriv.h"

/* global tools etc */
extern HANDLE hLibInst;
HANDLE hVertCurs;
HANDLE hNormCurs;
HPEN hpenDotted;
UINT gtab_msgcode;

/* function prototypes */
LRESULT gtab_wndproc(HWND, UINT, WPARAM, LPARAM);
void gtab_createtools(void);
void gtab_deltable(HWND hwnd, lpTable ptab);
lpTable gtab_buildtable(HWND hwnd, DWORD_PTR id);
void gtab_setsize(HWND hwnd, lpTable ptab);
void gtab_newsize(HWND hwnd, lpTable ptab);
void gtab_calcwidths(HWND hwnd, lpTable ptab);
BOOL gtab_alloclinedata(HWND hwnd, lpTable ptab);
void gtab_invallines(HWND hwnd, lpTable ptab, int start, int count);
void gtab_append(HWND hwnd, lpTable ptab, int rows, DWORD_PTR id);

/*
 * initialise window class - called from DLL main init
 */
void
gtab_init(void)
{
    WNDCLASS wc;

    gtab_createtools();
    gtab_msgcode = RegisterWindowMessage(TableMessage);

    wc.style = CS_GLOBALCLASS | CS_DBLCLKS;
    wc.lpfnWndProc = (WNDPROC)gtab_wndproc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = WLTOTAL;
    wc.hInstance = (HINSTANCE)hLibInst;
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = TableClassName;
    wc.lpszMenuName = NULL;

    RegisterClass(&wc);
}

void
gtab_createtools(void)
{
    hVertCurs = LoadCursor((HINSTANCE)hLibInst, "VertLine");
    hNormCurs = LoadCursor(NULL, IDC_ARROW);

    hpenDotted = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
}

void
gtab_deltools(void)
{
    DeleteObject(hpenDotted);
}


LRESULT
gtab_wndproc(
            HWND hwnd,
            UINT msg,
            WPARAM wParam,
            LPARAM lParam
            )
{
    CREATESTRUCT FAR * csp;
    HWND hOwner;
    lpTable ptab;
    lpTableSelection pselect;
    long oldtop;
    long change;

    switch (msg) {

        case WM_CREATE:
            /* create window. set the wnd extra bytes to
             * contain the owner window and a null table.
             * Owner window is either in lParam or the parent.
             * Then wait for TM_NEWID.
             */
            csp = (CREATESTRUCT FAR *) lParam;
            if (csp->lpCreateParams == NULL) {
                hOwner = GetParent(hwnd);
            } else {
                hOwner = (HWND) csp->lpCreateParams;
            }
            ptab = NULL;

            SetWindowLongPtr(hwnd, WL_TABLE, (LONG_PTR) ptab);
            SetWindowLongPtr(hwnd, WW_OWNER, (LONG_PTR) hOwner);

            SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
            SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
            break;

        case TM_NEWID:
            /* complete change of table.
             * close old table, discard memory and
             * build new table
             */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_sendtq(hwnd, TQ_CLOSE, ptab->hdr.id);
                gtab_deltable(hwnd, ptab);
                SetCursor((HCURSOR)hNormCurs);
                SetWindowLongPtr(hwnd, WL_TABLE, 0);
            }
            if ( (ptab = gtab_buildtable(hwnd, (DWORD_PTR)lParam)) != NULL) {
                SetWindowLongPtr(hwnd, WL_TABLE, (LONG_PTR) ptab);
                gtab_setsize(hwnd, ptab);
            } else {
                SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
                SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case TM_NEWLAYOUT:
            /* change of layout but for same id. no TQ_CLOSE,
             * but otherwise same as TM_NEWID
             */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_deltable(hwnd, ptab);
                SetCursor((HCURSOR)hNormCurs);
                SetWindowLongPtr(hwnd, WL_TABLE, 0);
            }
            if ( (ptab = gtab_buildtable(hwnd, (DWORD_PTR)lParam)) != NULL) {
                SetWindowLongPtr(hwnd, WL_TABLE, (LONG_PTR) ptab);
                gtab_setsize(hwnd, ptab);
            } else {
                SetScrollRange(hwnd, SB_VERT, 0, 0, TRUE);
                SetScrollRange(hwnd, SB_HORZ, 0, 0, TRUE);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case TM_REFRESH:
            /* data in table has changed. nrows may have
             * changed. ncols and col types have not changed
             */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_newsize(hwnd, ptab);
                gtab_sendtq(hwnd, TQ_SHOWWHITESPACE, (LPARAM) &ptab->show_whitespace);
            }
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case TM_SELECT:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                pselect = (lpTableSelection) lParam;

                gtab_select(hwnd, ptab, pselect->startrow,
                            pselect->startcell,
                            pselect->nrows,
                            pselect->ncells,
                            TRUE);
                gtab_showsel_middle(hwnd, ptab, pselect->dyRowsFromTop);
            }
            break;

        case TM_GETSELECTION:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                pselect = (lpTableSelection) lParam;

                *pselect = ptab->select;
            }
            break;

        case TM_PRINT:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                return gtab_print(hwnd, ptab, (lpPrintContext) lParam);
            }
            return FALSE;

        case TM_SETTABWIDTH:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (!ptab)
                return 0;
            ptab->tabchars = (int)lParam;
            InvalidateRect(hwnd, NULL, FALSE);
            break;

        case TM_TOPROW:

            /* return top row. if wParam is TRUE, set lParam
             * as the new toprow
             */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab == NULL) {
                return(0);
            }
            oldtop = ptab->toprow;
            if ((wParam) && (lParam < ptab->hdr.nrows)) {
                change = (long)lParam - ptab->toprow;
                change -= ptab->hdr.fixedrows;
                gtab_dovscroll(hwnd, ptab, change);
            }
            return(oldtop);

        case TM_ENDROW:
            /* return the last visible row in the window */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab == NULL) {
                return(0);
            }
            return(ptab->nlines + ptab->toprow - 1);


        case TM_APPEND:
            /* new rows have been added to the end of the
             * table, but the rest of the table has not
             * been changed. Update without forcing redraw of
             * everything.
             * lParam contains the new total nr of rows
             */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_append(hwnd, ptab, (int) wParam, (DWORD_PTR)lParam);
                return(TRUE);
            }
            break;

        case WM_SIZE:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_setsize(hwnd, ptab);
            }
            break;

        case WM_ERASEBKGND:
            return TRUE;

        case WM_DESTROY:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_sendtq(hwnd, TQ_CLOSE, ptab->hdr.id);
                gtab_deltable(hwnd, ptab);
            }
            break;

        case WM_SYSCOLORCHANGE:
            InvalidateRect(hwnd, NULL, TRUE);
            break;

        case WM_PAINT:
            gtab_paint(hwnd);
            break;

        case WM_HSCROLL:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_msg_hscroll(hwnd, ptab,
                                 GET_SCROLL_OPCODE(wParam, lParam),
                                 GET_SCROLL_POS(wParam, lParam));
            }
            break;

        case WM_VSCROLL:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_msg_vscroll(hwnd, ptab,
                                 GET_SCROLL_OPCODE(wParam, lParam),
                                 GET_SCROLL_POS(wParam, lParam));
            }
            break;

        case WM_MOUSEMOVE:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_move(hwnd, ptab, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            } else {
                SetCursor((HCURSOR)hNormCurs);
            }
            break;

        case WM_LBUTTONDOWN:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_press(hwnd, ptab, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_RBUTTONDOWN:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_rightclick(hwnd, ptab, (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_LBUTTONUP:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_release(hwnd, ptab,
                             (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_LBUTTONDBLCLK:
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                gtab_dblclick(hwnd, ptab,
                              (int)(short)LOWORD(lParam), (int)(short)HIWORD(lParam));
            }
            break;

        case WM_KEYDOWN:
            /* handle key presses for cursor movement about
             * the table, and return/space for selection.
             * Any key we don't handle is passed to the owner window
             * for him to handle.
             * The table window should have the focus
             */
            ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                if (gtab_key(hwnd, ptab, (int)wParam) != 0) {
                    hOwner = (HWND) GetWindowLongPtr(hwnd, WW_OWNER);
                    return(SendMessage(hOwner, WM_KEYDOWN, wParam, lParam));
                } else {
                    return(0);
                }
            }
            break;

#ifdef WM_MOUSEWHEEL
        case WM_MOUSEWHEEL:
            ptab = (lpTable)GetWindowLongPtr(hwnd, WL_TABLE);
            if (ptab != NULL) {
                if (gtab_mousewheel(hwnd,ptab, LOWORD(wParam), (short)HIWORD(wParam))) {
                    hOwner = (HWND)GetWindowLongPtr(hwnd, WW_OWNER);
                    return SendMessage(hOwner, WM_MOUSEWHEEL, wParam, lParam);
                }
            }
            break;
#endif

        default:
            return(DefWindowProc(hwnd, msg, wParam, lParam));
    }
    return(TRUE);
}

/*
 * send a table-query message to the owner window. returns message
 * value.
 */
INT_PTR
gtab_sendtq(
           HWND hwnd,
           UINT cmd,
           LPARAM lParam
           )
{
    HWND hOwner;

    hOwner = (HWND) GetWindowLongPtr(hwnd, WW_OWNER);
    return (SendMessage(hOwner, gtab_msgcode, cmd, lParam));
}

/*
 * free the memory allocated for the array of lines (each containing
 * an array of Cells, each containing an array of chars for the actual
 * data). Called on any occasion that would change the number of visible lines
 */
void
gtab_freelinedata(
                 lpTable ptab
                 )
{
    int i, j, ncols;
    lpCellData cd;


    ncols = ptab->hdr.ncols;

    /* for each line */
    for (i = 0; i < ptab->nlines; i++) {
        /* for each cell */
        for (j = 0; j < ncols; j++) {
            /* free up the actual text space */
            cd = &ptab->pdata[i].pdata[j];
            HeapFree(GetProcessHeap(), NULL, cd->ptext);
            HeapFree(GetProcessHeap(), NULL, cd->pwzText);
        }
        /* dealloc array of CellData */
        HeapFree(GetProcessHeap(), NULL, ptab->pdata[i].pdata);                
    }
    /* de-alloc array of linedatas */
    HeapFree(GetProcessHeap(), NULL, ptab->pdata);
    ptab->pdata = NULL;
}

/* allocate and init array of linedatas (include cell array
 * and text for each cell)
 */
BOOL
gtab_alloclinedata(
                  HWND hwnd,
                  lpTable ptab
                  )
{
    lpLineData pline;
    lpCellData cd;
    int i, j;

    ptab->pdata = (lpLineData) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                        sizeof(LineData) * ptab->nlines);
    if (ptab->pdata == NULL) {
        return(FALSE);
    }
    for (i = 0; i < ptab->nlines; i++) {
        pline = &ptab->pdata[i];
        pline->linepos.size = ptab->rowheight;
        pline->pdata = (lpCellData) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                             sizeof(CellData) * ptab->hdr.ncols);
        if (pline->pdata == NULL) {
            return(FALSE);
        }
        for (j = 0; j < ptab->hdr.ncols; j++) {
            cd = &pline->pdata[j];
            cd->props.valid = 0;
            cd->flags = 0;
            cd->nchars = ptab->pcolhdr[j].nchars;
            if (cd->nchars > 0) {
                cd->ptext = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cd->nchars);
                if (cd->ptext == NULL) {
                    return(FALSE);
                }
                cd->pwzText = 0;
            }
        }
    }
    return(TRUE);
}

/*
 * free up all table data structures. Called for new layout or new data.
 */
void
gtab_deltable(
             HWND hwnd,
             lpTable ptab
             )
{
    int ncols;

    if (ptab == NULL) {
        return;
    }
    ncols = ptab->hdr.ncols;

    if (ptab->pcolhdr != NULL) {
        HeapFree(GetProcessHeap(), NULL, ptab->pcolhdr);
    }
    if (ptab->pcellpos != NULL) {
        HeapFree(GetProcessHeap(), NULL, ptab->pcellpos);
    }
    if (ptab->pdata != NULL) {
        gtab_freelinedata(ptab);
    }
    HeapFree(GetProcessHeap(), NULL, ptab);
}


/*
 * build up a Table struct (excluding data allocation and
 * anything to do with font or window size).
 * return ptr to this or NULL if error
 */
lpTable
gtab_buildtable(
               HWND hwnd,
               DWORD_PTR id
               )
{
    lpTable ptab;
    int ncols, i;
    ColPropsList cplist;

    ptab = (lpTable) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Table));
    if (ptab == NULL) {
        return(NULL);
    }

    // get the tab width. most clients will not support this
    if (gtab_sendtq(hwnd, TQ_TABS, (LPARAM) &ptab->tabchars) == FALSE) {
        ptab->tabchars = TABWIDTH_DEFAULT;
    }

    // get the show whitespace value
    if (gtab_sendtq(hwnd, TQ_SHOWWHITESPACE, (LPARAM) &ptab->show_whitespace) == FALSE) {
        ptab->show_whitespace = FALSE;
    }

    /* get the row/column count from owner window */
    ptab->hdr.id = id;
    ptab->hdr.props.valid = 0;
    ptab->hdr.sendscroll = FALSE;
    if (gtab_sendtq(hwnd, TQ_GETSIZE, (LPARAM) &ptab->hdr) == FALSE) {
        return(NULL);
    }

    ncols = ptab->hdr.ncols;
    ptab->pcolhdr = (lpColProps) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ColProps) * ncols);
    if (ptab->pcolhdr == NULL) {
        /* should prob send TQ_CLOSE at this point */
        return(NULL);
    }

    /* init col properties to default */
    for (i=0; i < ncols; i++) {
        ptab->pcolhdr[i].props.valid = 0;
        ptab->pcolhdr[i].nchars = 0;
    }
    /* get the column props from owner */
    cplist.plist = ptab->pcolhdr;
    cplist.id = id;
    cplist.startcol = 0;
    cplist.ncols = ncols;
    gtab_sendtq(hwnd, TQ_GETCOLPROPS, (LPARAM) &cplist);

    /* init remaining fields */
    ptab->pcellpos = (lpCellPos) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(CellPos) * ncols);
    if (ptab->pcellpos == NULL) {
        return(NULL);
    }

    ptab->scrollscale = 1;
    ptab->scroll_dx = 0;
    ptab->toprow = 0;
    ptab->pdata = NULL;
    ptab->nlines = 0;
    ptab->trackmode = TRACK_NONE;

    /* we have to notify owner of the current selection
     * whenever it is changed
     */
    ptab->select.id = id;
    gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);

    /* calc ave height/width, cell widths and min height.
     * these change only when cell properties / col count changes -
     * ie only on rebuild-header events
     */
    gtab_calcwidths(hwnd, ptab);
    return(ptab);
}

/* set sizes that are based on window size and scroll pos
 * set:
 *      winwidth
 *      nlines
 *      cellpos start, clip start/end
 * alloc linedata and init
 */
void
gtab_setsize(
            HWND hwnd,
            lpTable ptab
            )
{
    RECT rc;
    int nlines;
    long change;
    SCROLLINFO si;

    GetClientRect(hwnd, &rc);
    ptab->winwidth = rc.right - rc.left;
    nlines = (rc.bottom - rc.top) / ptab->rowheight;
    /* nlines is the number of whole lines - add one extra
     * for the partial line at the bottom
     */
    nlines += 1;

    /* alloc space for nlines of data - if nlines has changed */
    if (nlines != ptab->nlines) {
        gtab_freelinedata(ptab);
        ptab->nlines = nlines;
        if (!gtab_alloclinedata(hwnd, ptab)) {
            ptab->nlines = 0;
            return;
        }
    }

    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE|SIF_RANGE;
    si.nMin = 0;

    /* set scroll vertical range */
    si.nMax = ptab->hdr.nrows;
    si.nPage = ptab->nlines;
    if (si.nMax < 0) {
        si.nMax = 0;
        change =  -(ptab->toprow);
    } else if (ptab->toprow > si.nMax) {
        change = si.nMax - ptab->toprow;
    } else {
        change = 0;
    }
    /* the scroll range must be 16-bits for Win3
     * scale until this is true
     */
    ptab->scrollscale = 1;
    while (si.nMax > 32766) {
        ptab->scrollscale *= 16;
        si.nMax /= 16;
        si.nPage /= 16;
    }
    if (!si.nPage)
        si.nPage = 1;

    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    gtab_dovscroll(hwnd, ptab, change);

    /* set horz scroll range */
    si.nMax = ptab->rowwidth;
    si.nPage = ptab->winwidth;
    if (si.nMax < 0) {
        si.nMax = 0;
        change = -(ptab->scroll_dx);
    } else if (ptab->scroll_dx > si.nMax) {
        change = si.nMax - ptab->scroll_dx;
    } else {
        change = 0;
    }
    /* horz scroll range will always be < 16 bits */
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
    gtab_dohscroll(hwnd, ptab, change);
}

/* set column widths/height and totals (based on column props)
 * - no assumption of window size (see gtab_setsize)
 * sets avewidth,rowheight,cellpos.size,rowwidth (total of cellpos.size)
 */
void
gtab_calcwidths(
               HWND hwnd,
               lpTable ptab
               )
{
    int i, cxtotal, cx, ave;
    TEXTMETRIC tm = {0};
    TEXTMETRIC tmcol = {0};
    HDC hdc;
    lpProps hdrprops, cellprops;
    HFONT hfont;

    hfont = NULL;  /* eliminate spurious diagnostic, make code worse */
    hdrprops = &ptab->hdr.props;
    hdc = GetDC(hwnd);
    if (hdc)
    {
        GetTextMetrics(hdc, &tm);
        ptab->rowheight = tm.tmHeight + tm.tmExternalLeading;
        if (hdrprops->valid & P_FONT) {
            hfont = (HFONT)SelectObject(hdc, hdrprops->hFont);
        }
        GetTextMetrics(hdc, &tm);
        if (hdrprops->valid & P_FONT) {
            SelectObject(hdc, hfont);
        }
        ReleaseDC(hwnd, hdc);
    }
    else
    {
        // arbitrary, whatever...
        ptab->rowheight = 14;
        tm.tmHeight = 14;
        tm.tmAveCharWidth = 5;
    }

    /* get width and height of average character */
    ptab->avewidth = tm.tmAveCharWidth;
    if (tm.tmHeight + tm.tmExternalLeading < ptab->rowheight - 2 ||
        tm.tmHeight + tm.tmExternalLeading > ptab->rowheight) {
        // make sure that the default FixedSys (and anything of similar size)
        // doesn't vertically clip the System font used for line numbers,
        // filenames, etc.
        ptab->rowheight = tm.tmHeight;
        if (tm.tmExternalLeading)
            ptab->rowheight += tm.tmExternalLeading;
        else
            ptab->rowheight++;
    }
    if (hdrprops->valid & P_HEIGHT) {
        ptab->rowheight = hdrprops->height;
    }

    /* set pixel width of each cell (and add up for row total)
     * based on ave width * nr chars, unless P_WIDTH set
     */
    cxtotal = 0;
    for (i = 0; i < ptab->hdr.ncols; i++) {
        cellprops = &ptab->pcolhdr[i].props;

        if (cellprops->valid & P_WIDTH) {
            cx = cellprops->width;
        } else if (hdrprops->valid & P_WIDTH) {
            cx = hdrprops->width;
        } else {

            if (cellprops->valid & P_FONT) {
                hdc = GetDC(hwnd);
                if (hdc)
                {
                    hfont = (HFONT)SelectObject(hdc, cellprops->hFont);
                    GetTextMetrics(hdc, &tmcol);
                    SelectObject(hdc, hfont);
                    ReleaseDC(hwnd, hdc);
                    ave = tmcol.tmAveCharWidth;
                }
                else
                    ave = 5;       
            } else {
                ave = ptab->avewidth;
            }
            /* ave width * nchars */
            cx =  ptab->pcolhdr[i].nchars + 1;
            cx *= ave;
        }
        /* add 2 pixels for box lines */
        cx += 2;
        ptab->pcellpos[i].size = cx;
        cxtotal += cx;
    }
    ptab->rowwidth = cxtotal;
}

/* called when row data + possible nrows changes.
 * other changes are ignored
 */
void
gtab_newsize(
            HWND hwnd,
            lpTable ptab
            )
{
    TableHdr hdr;

    /* get new row count */
    hdr = ptab->hdr;
    gtab_sendtq(hwnd, TQ_GETSIZE, (LPARAM) &hdr);
    if (hdr.nrows != ptab->hdr.nrows) {
        ptab->hdr.nrows = hdr.nrows;
        gtab_setsize(hwnd, ptab);
    }

    gtab_invallines(hwnd, ptab, 0, ptab->nlines);

    InvalidateRect(hwnd, NULL, FALSE);
}

void
gtab_invallines(
               HWND hwnd,
               lpTable ptab,
               int start,
               int count
               )
{
    int i, j;

    for (i = start; i < start + count; i++) {
        for (j = 0; j < ptab->hdr.ncols; j++) {
            ptab->pdata[i].pdata[j].flags = 0;
        }
    }
}

/* new rows have been added to the table. adjust the scroll range and
 * position, and redraw the rows if the end of the table is currently
 * visible.
 * rows = the new total row count.
 */
void
gtab_append(
           HWND hwnd,
           lpTable ptab,
           int rows,
           DWORD_PTR id
           )
{
    long oldrows;
    int line, nupdates;
    RECT rc;
    SCROLLINFO si;


    /* change to the new id */
    ptab->hdr.id = id;
    ptab->select.id = id;

    /* update the header, but remember the old nr of rows
     * so we know where to start updating
     */
    oldrows = ptab->hdr.nrows;

    /* check that the new nr of rows is not smaller.
     */
    if (oldrows >= rows) {
        return;
    }

    ptab->hdr.nrows = rows;

    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE|SIF_RANGE;
    si.nMin = 0;

    /* set the vertical scroll range */
    si.nMax = rows;
    si.nPage = ptab->nlines;
    if (si.nMax < 0) {
        si.nMax = 0;
    }

    /* force the scroll range into 16-bits for win 3.1 */
    ptab->scrollscale = 1;
    while (si.nMax > 32766) {
        ptab->scrollscale *= 16;
        si.nMax /= 16;
        si.nPage /= 16;
    }
    if (!si.nPage)
        si.nPage = 1;

    /* now set the scroll bar range and position */
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    if (si.nMax > 0) {
        SetScrollPos(hwnd, SB_VERT,
                     (int) (ptab->toprow / ptab->scrollscale), TRUE);
    }

    /* calculate which screen lines need to be updated - find what
     * screen line the start of the new section is at
     */
    line = gtab_rowtoline(hwnd, ptab, oldrows);
    if (line == -1) {
        /* not visible -> no more to do */
        return;
    }

    /* how many lines to update - rest of screen or nr of
     * new lines if less than rest of screen
     */
    nupdates = min((ptab->nlines - line), (int)(rows - oldrows));

    /* invalidate the screen line buffers to indicate data
     * needs to be refetch from parent window
     */
    gtab_invallines(hwnd, ptab, line, nupdates);

    /* calculate the region of the screen to be repainted -
     * left and right are same as window. top and bottom
     * need to be calculated from screen line height
     */

    GetClientRect(hwnd, &rc);
    rc.top += line * ptab->rowheight;
    rc.bottom = rc.top + (nupdates * ptab->rowheight);

    /* force a repaint of the updated region */
    InvalidateRect(hwnd, &rc, FALSE);
}
