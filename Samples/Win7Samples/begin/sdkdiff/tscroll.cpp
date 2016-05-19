// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * tscroll.cpp
 *
 * standard table class.
 *
 * scrolling and selection routines
 *
 * see table.h for interface description
 *
 * This implementation currently only supports TM_SINGLE, not TM_MANY
 * modes of selection.
 */

#include "precomp.h"

#include "table.h"
#include "tpriv.h"


VOID
gtab_extendsel(
    HWND hwnd,
    lpTable ptab,
    long startrow,
    long startcell,
    BOOL bNotify
);


/* handle a vscroll message */
void
gtab_msg_vscroll(HWND hwnd, lpTable ptab, int opcode, int pos)
{
    long change;

    switch(opcode) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        change = (pos * ptab->scrollscale) - ptab->toprow;
        break;

    case SB_LINEUP:
        change = -1;
        break;

    case SB_LINEDOWN:
        change = 1;
        break;

    case SB_PAGEUP:
        change = - (ptab->nlines - 3);
        if (change>=0)
            change = -1;    // consider nlines <=3!
        break;

    case SB_PAGEDOWN:
        change = (ptab->nlines - 3);
        if (change<=0)
            change = 1;     // consider nlines <=3!
        break;

    default:
        return;
    }
    gtab_dovscroll(hwnd, ptab, change);
}

/* handle a hscroll message */
void
gtab_msg_hscroll(HWND hwnd, lpTable ptab, int opcode, int pos)
{
    int change;

    switch(opcode) {
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        change = pos - ptab->scroll_dx;
        break;

    case SB_LINEUP:
        change = -(ptab->avewidth);
        break;

    case SB_LINEDOWN:
        change = ptab->avewidth;
        break;

    case SB_PAGEUP:
        change = - (ptab->winwidth * 2 / 3);
        break;

    case SB_PAGEDOWN:
        change = (ptab->winwidth * 2 / 3);
        break;

    default:
        return;
    }
    gtab_dohscroll(hwnd, ptab, change);
}



/*
 * set new vertical scroll pos,
 * adjust linedata array
 * set line win-relative start posns & clip top/bottom posns
 * revise display.
 */
void
gtab_dovscroll(HWND hwnd, lpTable ptab, long change)
{
    int cury, i;
    long ncopy;
    lpCellPos cp;
    LineData ldtemp;
    RECT rc, rcpaint;
    long range;
    long newtop;
    int newpos;
    BOOL fWasVisible = FALSE;

    if (ptab->selvisible)
    {
        fWasVisible = TRUE;
        ptab->selvisible = FALSE;
        gtab_invertsel(hwnd, ptab, NULL);
    }

    range = ptab->hdr.nrows - (ptab->nlines - 1);
    newtop = ptab->toprow + change;
    if (range < 0) {
        range = 0;
    }
    if (newtop > range) {
        change = range - ptab->toprow;
    } else if (newtop < 0) {
        change = -(ptab->toprow);
    }
    ptab->toprow += change;

    newpos = (int) (newtop / ptab->scrollscale);
    SetScrollPos(hwnd, SB_VERT, newpos, TRUE);

    if (ptab->hdr.sendscroll) {
        gtab_sendtq(hwnd, TQ_SCROLL, ptab->toprow);
    }

    /* adjust data ptrs rather than invalidate, to retain the
     * data we know is still valid
     */
    if (abs(change) >= ptab->nlines) {
        gtab_invallines(hwnd, ptab, ptab->hdr.fixedrows,
            ptab->nlines - ptab->hdr.fixedrows);
        InvalidateRect(hwnd, NULL, FALSE);
        change = 0;
    } else if (change < 0) {
        /* copy data down */
        ncopy = (ptab->nlines - ptab->hdr.fixedrows) - abs(change);
        for (i =  ptab->nlines - 1;
                i >= (ptab->hdr.fixedrows + abs(change)); i--) {
            ldtemp = ptab->pdata[i - abs(change)];
            ptab->pdata[i - abs(change)] = ptab->pdata[i];
            ptab->pdata[i] = ldtemp;
        }
        gtab_invallines(hwnd, ptab,
                ptab->hdr.fixedrows, (int) abs(change));
    } else if (change > 0) {
        ncopy = (ptab->nlines - ptab->hdr.fixedrows) - change;
        for (i = ptab->hdr.fixedrows;
                i < (ncopy + ptab->hdr.fixedrows); i++) {
            ldtemp = ptab->pdata[i + change];
            ptab->pdata[i + change] = ptab->pdata[i];
            ptab->pdata[i] = ldtemp;
        }
        gtab_invallines(hwnd, ptab,
            (int) ncopy + ptab->hdr.fixedrows, (int) change);
    }

    /* scroll window */
    GetClientRect(hwnd, &rc);
    rcpaint = rc;
    if (change > 0) {
        rc.top += (int) (change + ptab->hdr.fixedrows) * ptab->rowheight;
        rcpaint.top = (ptab->hdr.fixedrows * ptab->rowheight);
        rcpaint.top += rc.bottom - rc.top;
    } else if (change < 0) {
        rc.top += (ptab->hdr.fixedrows * ptab->rowheight);
        rc.bottom += (int) (change * ptab->rowheight);
        rcpaint.bottom -= rc.bottom - rc.top;
    }

    /* loop through each line setting relative posn and clipping */

    /* set up all rows  - the fixed/moveable difference for
     * rows is made at fetch-time during painting, when we remember
     * which absolute row nr to ask for, for a given screen line
     */
    cury = 0;
    for (i = 0; i < ptab->nlines; i++) {
        cp = &ptab->pdata[i].linepos;
        cp->start = cury;
        cp->clipstart = cury;
        cp->clipend = cury + cp->size;
        cury += cp->size;
    }

    /* now move and repaint the window */
    if (change != 0) {
        if (rc.top < rc.bottom) {
            ScrollWindow(hwnd, 0, (int) -(change * ptab->rowheight),
                &rc, NULL);
        }

        // don't repaint the fixed rows
        rc.top = 0;
        rc.bottom = ptab->hdr.fixedrows * ptab->rowheight;
        ValidateRect(hwnd, &rc);

        /* force repaint now, not just post message for later,
         * since we want to repaint that line before the next
         * scroll down occurs
         */
        ValidateRect(hwnd, &rcpaint);
        RedrawWindow(hwnd, &rcpaint, NULL,
                RDW_NOERASE | RDW_INVALIDATE | RDW_INTERNALPAINT);

    }

    if (fWasVisible)
    {
        gtab_invertsel(hwnd, ptab, NULL);
        ptab->selvisible = TRUE;
    }
}

/*
 * set new horizontal scroll pos,
 * set col win-relative start posns & clip left/right posns
 * revise display.
 */
void
gtab_dohscroll(HWND hwnd, lpTable ptab, long change)
{
    int curx, i;
    int moveable;
    lpCellPos cp;
    int newdx, range;


    /* check that the new scroll pos is still within the valid range */
    range = ptab->rowwidth - ptab->winwidth;
    newdx = ptab->scroll_dx + (int) change;
    if (range < 0) {
        range = 0;
    }
    if (newdx > range) {
        change = range - ptab->scroll_dx;
    } else if (newdx < 0) {
        change = -(ptab->scroll_dx);
    }
    ptab->scroll_dx += (int) change;

    SetScrollPos(hwnd, SB_HORZ, ptab->scroll_dx, TRUE);
    if (ptab->hdr.fixedcols > 0) {
        RECT rc;
        GetClientRect(hwnd, &rc);
        rc.left = ptab->pcellpos[ptab->hdr.fixedcols - 1].clipend;
        InvalidateRect(hwnd, &rc, FALSE);
    } else {
        InvalidateRect(hwnd, NULL, FALSE);
    }

    /* loop through each col setting relative posn and clipping */
    /* clip off 1 pixel left and right (we added 2 on to size for this) */

    /* first set up fixed columns */
    curx = 0;
    for (i = 0; i < ptab->hdr.fixedcols; i++) {
        cp = &ptab->pcellpos[i];
        cp->start = curx + 1;
        cp->clipstart = cp->start;
        cp->clipend = cp->start + cp->size - 2;
        curx += cp->size;
    }

    /* now moveable columns. remember start of moveable cols */
    moveable = curx;
    curx = - ptab->scroll_dx;       /* rel. pos of col */
    for (i = ptab->hdr.fixedcols; i < ptab->hdr.ncols; i++) {
        cp = &ptab->pcellpos[i];
        cp->start = curx + moveable + 1;
        cp->clipstart = max(moveable+1, cp->start);
        cp->clipend = cp->start + cp->size - 2;
        curx += cp->size;
    }
}

/*
 * convert screen line nr to table row nr
 */
long
gtab_linetorow(HWND hwnd, lpTable ptab, int line)
{
    if (line < ptab->hdr.fixedrows) {
        return(line);
    }

    return (line + ptab->toprow);
}

/*
 * convert table row nr to screen line nr or -1 if not on screen
 */
int
gtab_rowtoline(HWND hwnd, lpTable ptab, long row)
{
    if (row < ptab->hdr.fixedrows) {
        return( (int) row);
    }

    row -= ptab->toprow;
    if ((row >= ptab->hdr.fixedrows) && (row < ptab->nlines)) {
        return ( (int) row);
    }
    return(-1);
}


/*
 * check if a given location is within the current selection.
 * Returns true if it is inside the current selection, or false if
 * either there is no selection, or the row, cell passed is outside it.
 */
BOOL
gtab_insideselection(
    lpTable ptab,
    long row,
    long cell)
{
    long startrow, endrow;
    long startcell, endcell;

    if (0 == ptab->select.nrows) {
        // no selection
        return FALSE;
    }

    // selection maintains anchor point as startrow,
    // so the selection can extend forwards or backwards from there.
    // need to convert to forward only for comparison
    startrow = ptab->select.startrow;
    if (ptab->select.nrows < 0) {
        endrow = startrow;
        startrow += ptab->select.nrows + 1;
    } else {
        endrow = startrow + ptab->select.nrows - 1;
    }
    if ((row < startrow) || (row > endrow)) {
        return FALSE;
    }

    // if we are in row-select mode, then that's it - its inside
    if (ptab->hdr.selectmode & TM_ROW) {
        return TRUE;
    }

    // same calculation for cells
    startcell = ptab->select.startcell;
    if (ptab->select.ncells < 0) {
        endcell = startcell;
        startcell += ptab->select.ncells + 1;
    } else {
        endcell = startcell + ptab->select.ncells - 1;
    }
    if ((cell < startcell) || (cell > endcell)) {
        return FALSE;
    }

    return TRUE;
}



/*
 * replace old selection with new. Notify owner if bNotify. Change
 * display to reflect new display.
 */
void
gtab_select(
        HWND hwnd,
        lpTable ptab,
        long row,
        long col,
        long nrows,
        long ncells,
        BOOL bNotify)
{

    /* if in ROW mode, force col and ncells to reflect the entire row. */
    if (ptab->hdr.selectmode & TM_ROW) {
        col = 0;
        ncells = ptab->hdr.ncols;
    }

    /* clear existing sel if valid and visible */
    if ((ptab->select.nrows != 0) && (ptab->selvisible == TRUE)) {

        /* only clear sel if it is different from the new one */
        if ((ptab->select.startrow != row) ||
                (ptab->select.startcell != col) ||
                (ptab->select.nrows != nrows) ||
                (ptab->select.ncells != ncells)) {

            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = FALSE;
        }
    }

    /* set select fields and send TQ_SELECT */
    if (row < ptab->hdr.nrows) {
        ptab->select.startrow = row;
        ptab->select.startcell = col;
        ptab->select.nrows = nrows;
        ptab->select.ncells = ncells;
    } else {
        ptab->select.nrows = 0;
        ptab->select.startrow = 0;
        ptab->select.startcell = 0;
        ptab->select.ncells = 0;
    }

    if (bNotify) {
        gtab_sendtq(hwnd, TQ_SELECT, (LPARAM) &ptab->select);
    }

    /* paint in selection */
    if (nrows != 0) {
        if (!ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = TRUE;
        }
    } else {
        if (ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = FALSE;
        }
        ptab->selvisible = FALSE;
    }
}

/*
 * convert window y co-ord to a line nr
 */
int
gtab_ytoline(HWND hwnd, lpTable ptab, int y)
{
    return(y / ptab->rowheight);
}

/*
 * convert window x co-ord to a cell nr
 */
int
gtab_xtocol(HWND hwnd, lpTable ptab, int x)
{
    int i;
    lpCellPos ppos;

    for (i = 0; i < ptab->hdr.ncols; i++) {
        ppos = &ptab->pcellpos[i];
        if (ppos->clipstart < ppos->clipend) {
            if ( (x >= ppos->clipstart) && (x < ppos->clipend)) {
                return(i);
            }
        }
    }
    return(-1);
}


/*
 * check if x co-ord is 'near' (+- 2 pixels) the right border of given cell
 */
BOOL
gtab_isborder(HWND hwnd, lpTable ptab, long x, long col)
{

    if (abs(ptab->pcellpos[col].clipend - x) < 2) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}


/*
 * set selection and send 'TQ_ENTER' event to owner
 */
void
gtab_enter(HWND hwnd, lpTable ptab, long row, long col, long nrows,
        long ncells)
{
    /* clear existing sel if valid and visible */
    if ((ptab->select.nrows != 0) && (ptab->selvisible == TRUE)) {

        /* only clear sel if it is different from the new one */
        if ((ptab->select.startrow != row) ||
                (ptab->select.startcell != col) ||
                (ptab->select.nrows != nrows) ||
                (ptab->select.ncells != ncells)) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = FALSE;
        }
    }

    /* set select fields and send TQ_ENTER */
    if (row < ptab->hdr.nrows) {
        ptab->select.startrow = row;
        ptab->select.startcell = col;
        ptab->select.nrows = nrows;
        ptab->select.ncells = ncells;
    } else {
        ptab->select.nrows = 0;
        ptab->select.startrow = 0;
        ptab->select.startcell = 0;
        ptab->select.ncells = 0;
    }

    /* paint in selection */
    if (nrows != 0) {
        if (!ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
            ptab->selvisible = TRUE;
        }
        /* do this at end because it could cause a layout-change */
        gtab_sendtq(hwnd, TQ_ENTER, (LPARAM) &ptab->select);
    } else {
        if (ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, NULL);
        }
        ptab->selvisible = FALSE;
    }
}


/*
 * start re-sizing a column
 */
void
gtab_trackcol(HWND hwnd, lpTable ptab, long col, long x)
{

    /* ensure we see the mouse-up */
    SetCapture(hwnd);
    ptab->trackmode = TRACK_COLUMN;
#ifdef WIN32
    ptab->tracknr = col;
    ptab->trackline1 = x;
#else
    // maximum 32767 columns is a reasonable limit!
    ptab->tracknr = (int) (col & 0x7fff);
    ptab->trackline1 = (int) (x & 0x7fff);
#endif

    /* if line at other side of cell is visible, draw that too */
    if (ptab->pcellpos[col].start >= ptab->pcellpos[col].clipstart) {
        ptab->trackline2 = ptab->pcellpos[col].start;
    } else {
        ptab->trackline2 = -1;
    }
    gtab_drawvertline(hwnd, ptab);
}


/*
 * called on right-click events. Select the cell clicked on, and if
 * valid, send on to owner for any context-menu type operation
 */
void
gtab_rightclick(HWND hwnd, lpTable ptab, int x, int y)
{
    long cell, ncells;
    long row;
    HWND hOwner;

    /* find which col, row he selected */
    cell = gtab_xtocol(hwnd, ptab, x);
    if (cell == -1) {
        return;
    }
    row = gtab_linetorow(hwnd, ptab, gtab_ytoline(hwnd, ptab, y));

    /* is he selecting a disabled fixed area ? */
    if ( (row < ptab->hdr.fixedrows) || (cell < ptab->hdr.fixedcols)) {
        if (ptab->hdr.fixedselectable == FALSE) {
            return;
        }
    }

    // ignore if beyond data
    if ((row >= ptab->hdr.nrows) ||
            (cell >= ptab->hdr.ncols)) {
        return;
    }

    /* is this within the already-selected area? */
    if (!gtab_insideselection(ptab, row, cell)) {
        // no selection, or clicked outside the selection - make new selection
        // before sending the right-click

        // if shift is down, extend selection
        if (GetKeyState(VK_SHIFT) & 0x8000) {
            gtab_extendsel(hwnd, ptab, row, cell, TRUE);
        } else {
            /* record and paint new selection */

            if (ptab->hdr.selectmode & TM_ROW) {
                cell = 0;
                ncells = ptab->hdr.ncols;
            } else {
                ncells = 1;
            }
            gtab_select(hwnd, ptab, row, cell, 1, ncells, TRUE);
        }
    }

    // now we have sent the selection, pass the message onto him
    hOwner = (HWND) GetWindowLongPtr(hwnd, WW_OWNER);
    SendMessage(hOwner, WM_RBUTTONDOWN, 0, MAKELONG( (short)x, (short)y));
}


/*
 * called on mouse-down events. decide what to start tracking.
 */
void
gtab_press(HWND hwnd, lpTable ptab, int x, int y)
{
    long cell, ncells;
    long row;

    if (ptab->trackmode != TRACK_NONE) {
        return;
    }

    /* has he grabbed a cell-edge to resize ? */
    cell = gtab_xtocol(hwnd, ptab, x);
    if (cell == -1) {
        return;
    }
    if (gtab_isborder(hwnd, ptab, x, cell)) {
        gtab_trackcol(hwnd, ptab, cell, x);
        return;
    }
    if ( (cell > 0) && gtab_isborder(hwnd, ptab, x, cell-1)) {
        gtab_trackcol(hwnd, ptab, cell, x);
        return;
    }

    /* find which line he selected */
    row = gtab_linetorow(hwnd, ptab, gtab_ytoline(hwnd, ptab, y));

    /* is he selecting a disabled fixed area ? */
    if ( (row < ptab->hdr.fixedrows) || (cell < ptab->hdr.fixedcols)) {
        if (ptab->hdr.fixedselectable == FALSE) {
            return;
        }
    }

    // ignore if beyond data
    if ((row >= ptab->hdr.nrows) ||
            (cell >= ptab->hdr.ncols)) {
        return;
    }


    /* ok, start cell selection */
    ptab->trackmode = TRACK_CELL;
    SetCapture(hwnd);

    /* record and paint new selection */

    if (ptab->hdr.selectmode & TM_ROW) {
        cell = 0;
        ncells = ptab->hdr.ncols;
    } else {
        ncells = 1;
    }

    /*
     * if the shift key is down, then extend the selection to this
     * new anchor point, rather than create a new selection
     */
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        gtab_extendsel(hwnd, ptab, row, cell, FALSE);
    } else {
        gtab_select(hwnd, ptab, row, cell, 1, ncells, FALSE);
    }
    return;
}

/*
 * called on mouse-up. complete any tracking that was happening
 */
void
gtab_release(HWND hwnd, lpTable ptab, int x, int y)
{
    lpCellPos ppos;
    lpProps pprop;
    long row, cell;
    int cx;

    switch(ptab->trackmode) {

    case TRACK_NONE:
        return;

    case TRACK_COLUMN:
        /* erase marker lines */
        gtab_drawvertline(hwnd, ptab);
        ReleaseCapture();
        ptab->trackmode = TRACK_NONE;

        /* adjust cell width */
        ppos = &ptab->pcellpos[ptab->tracknr];
        cx = ptab->trackline1 - ppos->start;
        pprop = &ptab->pcolhdr[ptab->tracknr].props;
        pprop->valid |= P_WIDTH;
        pprop->width = cx;
        gtab_calcwidths(hwnd, ptab);
        gtab_setsize(hwnd, ptab);
        InvalidateRect(hwnd, NULL, FALSE);
        return;

    case TRACK_CELL:
        row = gtab_linetorow(hwnd, ptab, gtab_ytoline(hwnd, ptab, y));
        cell = gtab_xtocol(hwnd, ptab, x);

        ReleaseCapture();
        ptab->trackmode = TRACK_NONE;

        // ignore if before or beyond data
        if ( (row < ptab->hdr.fixedrows) ||
             (cell < ptab->hdr.fixedcols)) {
            if (ptab->hdr.fixedselectable == FALSE) {
                gtab_select(
                    hwnd,
                    ptab,
                    ptab->select.startrow,
                    ptab->select.startcell,
                    ptab->select.nrows,
                    ptab->select.ncells,
                    TRUE);

                return;
            }
        }

        if ((row >= ptab->hdr.nrows) ||
                (cell >= ptab->hdr.ncols)) {
            gtab_select(
                    hwnd,
                    ptab,
                    ptab->select.startrow,
                    ptab->select.startcell,
                    ptab->select.nrows,
                    ptab->select.ncells,
                    TRUE);
            return;
        }

        /*
         * Extend to this new selection end point
         * we used to only do this if shift key pressed, but that
         * is not a good UI.
         */
        gtab_extendsel(hwnd, ptab, row, cell, TRUE);
        return;
    }
}


/* called on mouse-move. if tracking - adjust position, if not,
 * set correct cursor
 */
void
gtab_move(HWND hwnd, lpTable ptab, int x, int y)
{
    BOOL fOK;
    int line;
    long row;
    int col;
    lpCellPos ppos;

    switch(ptab->trackmode) {

    case TRACK_NONE:
        col = gtab_xtocol(hwnd, ptab, x);
        if (col == -1) {
            SetCursor((HCURSOR)hNormCurs);
            return;
        }
        if (gtab_isborder(hwnd, ptab, x, col)) {
            SetCursor((HCURSOR)hVertCurs);
            return;
        }
        if ( (col > 0) && gtab_isborder(hwnd, ptab, x, col-1)) {
            SetCursor((HCURSOR)hVertCurs);
            return;
        }
        SetCursor((HCURSOR)hNormCurs);
        return;

    case TRACK_CELL:
        line = gtab_ytoline(hwnd, ptab, y);

        // we used to only allow drag to extend
        // the selection if the shift key was down.
        // this doesn't seem to work as a UI - you expect
        // to drag and extend.

        /* if extending selection then
         * allow scrolling by dragging off window
         */
        if (line < 0) {
            gtab_dovscroll(hwnd, ptab, -1);
            line = gtab_ytoline(hwnd, ptab, y);
        } else if (line >=  ptab->nlines) {
            gtab_dovscroll(hwnd, ptab, 1);
            line = gtab_ytoline(hwnd, ptab, y);
        }


        row = gtab_linetorow(hwnd, ptab, line);
        col = gtab_xtocol(hwnd, ptab, x);

        // ignore if before or beyond data
        if ( (row < ptab->hdr.fixedrows) || (col < ptab->hdr.fixedcols)) {
            if (ptab->hdr.fixedselectable == FALSE) {
                return;
            }
        }

        if ((row >= ptab->hdr.nrows) ||
            (col >= ptab->hdr.ncols)) {
            return;
        }

        /*
         * extend to this new selection end point
         */
        gtab_extendsel(hwnd, ptab, row, col, FALSE);
        return;

    case TRACK_COLUMN:
        /* check that new x is still visible/valid */
        ppos = &ptab->pcellpos[ptab->tracknr];
        fOK = FALSE;

        if (ptab->tracknr < ptab->hdr.fixedcols)  {
            if ((x > ppos->start) && (x < ptab->winwidth)) {
                fOK = TRUE;
            }
        } else {
            if ((x > ppos->clipstart) && (x < ptab->winwidth)) {
                fOK = TRUE;
            }
        }
        if (fOK == TRUE) {
            gtab_drawvertline(hwnd, ptab);
            ptab->trackline1 = x;
            gtab_drawvertline(hwnd, ptab);
        }
        return;
    }
}

/* dbl-click - send an TQ_ENTER event to the owner (if valid) */
void
gtab_dblclick(HWND hwnd, lpTable ptab, int x, int y)
{
    int cell, line;
    long row;

    line = gtab_ytoline(hwnd, ptab, y);
    cell = gtab_xtocol(hwnd, ptab, x);
    if ( (line < ptab->hdr.fixedrows) || (cell < ptab->hdr.fixedcols) ) {
        if (!ptab->hdr.fixedselectable) {
            return;
        }
    }
    row = gtab_linetorow(hwnd, ptab, line);

    if (ptab->hdr.selectmode & TM_ROW) {
        gtab_enter(hwnd, ptab, row, 0, 1, ptab->hdr.ncols);
    } else {
        gtab_enter(hwnd, ptab, row, cell, 1, 1);
    }
}

/*
 * move selection area to visible part of window. argument bToBottom
 * indicates whether to move the line onto the bottom or the top of the
 * window if not visible - this affects the smoothness of scrolling
 * line-by-line.
 */
void
gtab_showsel(HWND hwnd, lpTable ptab, BOOL bToBottom)
{
    int line;
    long change;

    line = gtab_rowtoline(hwnd, ptab, ptab->select.startrow);

    /* move up if last line or not at all visible */
    if ( (line < 0) || line == (ptab->nlines - 1)) {
        change = ptab->select.startrow - ptab->toprow;
        if (bToBottom) {
            /* change to bottom of window. subtract 2 not 1
             * since nlines includes one line that is only
             * partly visible
             */
            change -= (ptab->nlines - 2);
        }
        change -= ptab->hdr.fixedrows;
        gtab_dovscroll(hwnd, ptab, change);
    }
    /* add support for TM_CELL here! */
}

/*
 * scroll the window so that if possible, the selected row is in the
 * middle 60% of the screen so that context around it is visible.
 */
void
gtab_showsel_middle(HWND hwnd, lpTable ptab, long dyRowsFromTop)
{
    int line = ptab->select.startrow;
    long change = 0;
    int mid_top, mid_end;
    BOOL fScroll = FALSE;

    if (dyRowsFromTop >= 0)
    {
        fScroll = TRUE;
        change = (ptab->select.startrow - dyRowsFromTop) - ptab->toprow;
        change -= ptab->hdr.fixedrows;
    }

    /* is this within the middle 60 % ?  */
    mid_top = ptab->toprow + (ptab->nlines * 20 / 100);
    mid_end = ptab->toprow + (ptab->nlines * 80 / 100);
    if ((line < mid_top + change) || (line > mid_end + change))
    {
        /* no - scroll so that selected line is at
         * the 20% mark
         */
        fScroll = TRUE;
        change = (ptab->select.startrow - mid_top);
        change -= ptab->hdr.fixedrows;
    }

    if (fScroll)
    {
        gtab_dovscroll(hwnd, ptab, change);
    }

    /* again - need code here for TM_CELL mode to ensure that
     * active cell is horizontally scrolled correctly
     */
}


/*
 * extend the selection to set the new anchor point as startrow, startcell.
 *
 * nrows and ncells will then be set to include the end row of the previous
 * selection. nrows, ncells < 0 indicate left and up. -1 and +1 both indicate
 * just one cell or row selected.
 */
VOID
gtab_extendsel(
    HWND hwnd,
    lpTable ptab,
    long startrow,
    long startcell,
    BOOL bNotify
)
{
    long endrow, endcell, nrows, ncells;

    /*
     * if no current selection, then just select the new anchor point
     */
    if (ptab->select.nrows == 0) {
        gtab_select(hwnd, ptab, startrow, startcell, 1,
            (ptab->hdr.selectmode & TM_ROW) ? ptab->hdr.ncols:1,
            bNotify);
        return;
    }

    if (startrow >= ptab->hdr.nrows) {
        startrow = ptab->hdr.nrows -1;
    } else if (startrow < 0) {
        startrow = 0;
    }
    if (startcell >= ptab->hdr.ncols) {
        startcell = ptab->hdr.ncols-1;
    } else if (startcell < 0) {
        startcell = 0;
    }



    /* calculate the row just beyond the selection
     * this is one above for upwards sels, and one below for
     * downard-extending sels. Then adjust down or up one
     * to be the actual (inclusive) last row.
     */
    endrow = ptab->select.startrow + ptab->select.nrows;
    if (ptab->select.nrows < 0) {
        endrow++;
    } else {
        endrow--;
    }

    if (endrow >= ptab->hdr.nrows) {
        endrow = ptab->hdr.nrows-1;
    }
    nrows = endrow - startrow;

    if (nrows >= 0) {
        // convert from exclusive to inclusive
        nrows++;
    } else {
        // convert from exclusive to inclusive
        nrows--;
    }

    /* same calculation for cells */
    endcell = ptab->select.startcell + ptab->select.ncells;
    if (ptab->select.ncells < 0) {
        endcell++;
    } else {
        endcell--;
    }
    ncells = endcell - startcell;
    if (ncells >= 0) {
        ncells++;
    } else {
        ncells--;
    }
    gtab_select(hwnd, ptab, startrow, startcell, nrows, ncells, bNotify);
}



/* move the selection a specified nr of rows or cells
 * if no selection, select first visible unit
 *
 * if bExtend is true and there is a current selection, then extend it rather than
 * replace it. Note that (startrow, startcell) will always be set to the newly
 * selected position - this is the anchor point. nrows or ncells may be negative
 * if the selection extends upwards above the anchor. nrows == -1 is the same
 * as nrows == 1, meaning only the current row is visible. Similarly
 * (in TM_CELL mode), ncells may be negative.
 *
 * Move the selection (ie anchor point) to make it visible. bToBottom
 * indicates whether it should be moved to the bottom or the top
 * of the window.
 */
VOID
gtab_changesel(
    HWND hwnd,
    lpTable ptab,
    long rowincr,
    int cellincr,
    BOOL bToBottom,
    BOOL bExtend
)
{
    long row, col, ncols;

    /* is there a selection ? */
    if (ptab->select.nrows == 0) {

        /* no selection - force a selection
         * at the first visible unit
         */
        if (ptab->hdr.fixedselectable) {
            row = 0;
            col = 0;
        } else {
            row = gtab_linetorow(hwnd, ptab, ptab->hdr.fixedrows);
            /* should really check for first visible cell */
            col = ptab->hdr.fixedcols;
        }
        ncols = 1;
        if (ptab->hdr.selectmode & TM_ROW) {
            col = 0;
            ncols = ptab->hdr.ncols;
        }
        gtab_select(hwnd, ptab, row, col, 1, ncols, TRUE);

    } else {
        /* move the anchor point by rowincr, cellincr */
        row = ptab->select.startrow + rowincr;
        col = ptab->select.startcell + cellincr;


        /*
         * ensure that new anchor point is in a valid position
         */

        while (col >= ptab->hdr.ncols) {
            col -= ptab->hdr.ncols;
            row++;
        }
        while (col < 0) {
            col += ptab->hdr.ncols;
            row--;
        }
        if (row < 0) {
            row = 0;
        }
        if (row >= ptab->hdr.nrows) {
            row = ptab->hdr.nrows-1;
        }
        /* check we haven't moved into non-selectable region */
        if ((row < ptab->hdr.fixedrows) &&
            (!ptab->hdr.fixedselectable)) {
                    row = ptab->hdr.fixedrows;
        }

        if (bExtend) {
            gtab_extendsel(hwnd, ptab, row, col, TRUE);
        } else {
            gtab_select(
                hwnd,
                ptab,
                row,
                col,
                1,
                (ptab->hdr.selectmode & TM_ROW) ? ptab->hdr.ncols : 1,
                TRUE);
        }
    }

    /* ensure selection visible */
    gtab_showsel(hwnd, ptab, bToBottom);
}

/*
 * set the topmost selectable unit in window as the selection
 *
 * if bExtend is TRUE, then extend the selection to include this, rather
 * than replacing the existing selection. Note that (startrow, startcell)
 * is always the anchor point - ie most recently selected end, and the
 * (nrows, ncells) can be + or - to extend the selection downwards or upwards.
 */
void
gtab_selhome(HWND hwnd, lpTable ptab, BOOL bExtend)
{
    long startrow, startcell, ncells;

    if (ptab->hdr.selectmode & TM_ROW) {
        ncells = ptab->hdr.ncols;
    } else {
        ncells = 1;
    }
    startcell = 0;


    if (ptab->hdr.fixedselectable) {
        startrow = gtab_linetorow(hwnd, ptab, 0);
    } else {
        startrow = gtab_linetorow(hwnd, ptab, ptab->hdr.fixedrows);
        if (!(ptab->hdr.selectmode & TM_ROW)) {
            startcell = ptab->hdr.fixedcols;
        }
    }

    if (bExtend) {
        gtab_extendsel(hwnd, ptab, startrow, startcell, TRUE);
    } else {
        gtab_select(hwnd, ptab, startrow, startcell, 1, ncells, TRUE);
    }
}


/* handle key-down events - scroll windows and/or move selection */
int
gtab_key(HWND hwnd, lpTable ptab, int vkey)
{
    long startrow, ncells, startcell;
    BOOL bControl = FALSE;
    BOOL bShift = FALSE;

    if (GetKeyState(VK_CONTROL) & 0x8000) {
        bControl = TRUE;
    }
    if (GetKeyState(VK_SHIFT) & 0x8000) {
        /* ignore shift key here if TM_MANY -multiple selection flag- is
         * not selected
         */
        if (ptab->hdr.selectmode & TM_MANY) {
            bShift = TRUE;
        }
    }

    switch(vkey) {

    case VK_UP:
        if (bControl) {
            /* control-uparrow scrolls window without selection.
             * the selection is de-selected (to avoid surprises
             * moving back to it).
             */
            gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);
            gtab_dovscroll(hwnd, ptab, -1);
        } else {
            /* uparrow moves selection up one line */
            gtab_changesel(hwnd, ptab, -1, 0, FALSE, bShift);
        }
        return(0);

    case VK_DOWN:
        if (bControl) {
            /* control downarrow scrolls window without
             * a selection.
             */
            gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);
            gtab_dovscroll(hwnd, ptab, 1);
        } else {
            /* the normal gtab_changesel behaviour is
             * that if the selected line is not visible now,
             * we scroll it to the top of the window. This is fine
             * in most cases but causes unacceptable jumps when
             * repeatedly scrolling down with the down key.
             *
             * Thus we now have an argument to changesel to say
             * that in this case, if you need to move the line onto
             * the window, move it to the bottom and not the top
             */
            gtab_changesel(hwnd, ptab, 1, 0, TRUE, bShift);
        }
        return(0);

    case VK_LEFT:
        /* if cell-selection mode, move left one cell.
         * otherwise the whole row is selected - scroll
         * the line left a little
         */

        if (ptab->hdr.selectmode & TM_ROW) {
            if (bControl) {
                /* ctrl-left moves to start of line */
                gtab_dohscroll(hwnd, ptab, -(ptab->scroll_dx));
            } else {
                gtab_dohscroll(hwnd, ptab, -(ptab->avewidth));
            }
        } else {
            gtab_changesel(hwnd, ptab, 0, -1, FALSE, bShift);
        }
        return(0);

    case VK_RIGHT:
        /* if cell-selection mode, move right one cell.
         * otherwise the whole row is selected - scroll
         * the line right a little
         */
        if (ptab->hdr.selectmode & TM_ROW) {
            if (bControl) {
                /* control-right moves to right end of line */
                gtab_dohscroll(hwnd, ptab, ptab->rowwidth -
                                ptab->winwidth);
            } else {
                gtab_dohscroll(hwnd, ptab, ptab->avewidth);
            }
        } else {
            gtab_changesel(hwnd, ptab, 0, 1, TRUE, bShift);
        }
        return(0);

    case VK_HOME:
        if (bControl) {
            /* control-home == top of file */
            gtab_dovscroll(hwnd, ptab, -(ptab->toprow));
        }
        /* top of window */
        gtab_selhome(hwnd, ptab, bShift);
        gtab_showsel(hwnd, ptab, FALSE);

        return(0);

    case VK_END:
        if (bControl) {
            /* control-end -> end of file */
            startrow = ptab->hdr.nrows-1;
        } else {
            startrow = gtab_linetorow(hwnd, ptab, ptab->nlines - 1);
            if (startrow >= ptab->hdr.nrows) {
                startrow = ptab->hdr.nrows-1;
            }
        }

        startcell = 0;
        ncells = ptab->hdr.ncols;
        if (!(ptab->hdr.selectmode & TM_ROW)) {
            startcell = ptab->hdr.ncols-1;
            ncells = 1;
        }

        if (bShift) {
            gtab_extendsel(hwnd, ptab, startrow, startcell, TRUE);
        } else {
            gtab_select(hwnd, ptab, startrow, startcell, 1, ncells, TRUE);
        }

        /* we have selected the bottom line. We don't want to
         * move it up into the window, since the intended
         * effect is to select the lowest line. This doesn't
         * apply to the ctrl-end behaviour (move to bottom of
         * buffer.
         */
        if (bControl) {
            /* move the selection to make it visible - but move it
             * to the bottom and not to the top of the window
             */
            gtab_showsel(hwnd, ptab, TRUE);
        }
        return(0);

    case VK_RETURN:
        if (ptab->select.nrows != 0) {
            gtab_showsel(hwnd, ptab, FALSE);
            gtab_enter(hwnd, ptab, ptab->select.startrow,
                    ptab->select.startcell,
                    ptab->select.nrows, ptab->select.ncells);
        }
        return(0);

    case VK_SPACE:
        /* toggle the selection */
        if (ptab->select.nrows == 0) {
                /* no selection - make one */
                gtab_changesel(hwnd, ptab, 0, 0, TRUE, FALSE);
        } else {
                /* there is a selection - deselect it */
                gtab_select(hwnd, ptab, 0, 0, 0, 0, TRUE);
        }
        return(0);

    case VK_PRIOR:          /* page up */

        if (ptab->nlines > 3) {
            gtab_dovscroll(hwnd, ptab, -(ptab->nlines - 3));
        }
        gtab_selhome(hwnd, ptab, bShift);
        return(0);

    case VK_NEXT:           /* page down */

        /* scroll down one page */
        if (ptab->nlines > 3) {
            gtab_dovscroll(hwnd, ptab, (ptab->nlines - 3));
        }

        /* select new bottom line */
        startrow = gtab_linetorow(hwnd, ptab, ptab->nlines - 1);
        if (startrow >= ptab->hdr.nrows) {
            startrow = ptab->hdr.nrows-1;
        }
        startcell = 0;
        ncells = ptab->hdr.ncols;
        if (!(ptab->hdr.selectmode & TM_ROW)) {
            startcell = ptab->hdr.ncols-1;
            ncells = 1;
        }

        /* select bottom line, but don't call showsel
         * since we don't want to adjust it's position - we
         * want it to remain at the bottom of the window
         */
        if (bShift) {
            gtab_extendsel(hwnd, ptab, startrow, startcell, TRUE);
        } else {
            gtab_select(hwnd, ptab, startrow, startcell, 1, ncells, TRUE);
        }
        return(0);

    default:
        return(1);
    }
}

int gtab_mousewheel(HWND hwnd, lpTable ptab, DWORD fwKeys, int zDelta)
{
    static ULONG uScrollLines = 0;

    if (fwKeys & MK_MBUTTON) {
        return 1;
    }

    if (uScrollLines == 0) {
        SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &uScrollLines, FALSE);
        if (uScrollLines == 0) {
            uScrollLines = 3;
        }
    }

    zDelta /= -WHEEL_DELTA;

    if (fwKeys & MK_CONTROL) {
        //
        // Left-Right scroll
        //
        if (ptab->hdr.selectmode & TM_ROW) {
            if (fwKeys & MK_SHIFT) {
                zDelta = (zDelta > 0) ? ptab->rowwidth : -ptab->rowwidth;
            }
            gtab_dohscroll(hwnd, ptab, ptab->avewidth * zDelta);
            return 0;
        }
        return 1;
    }

    if (fwKeys & MK_SHIFT) {
        //
        // Page scroll
        //
        if (ptab->nlines > 3) {
            zDelta *= ptab->nlines - 3;
        }
    }
    else {
        if (uScrollLines) {
            zDelta *= uScrollLines;
            zDelta = min(zDelta, ptab->nlines - 3);
        }
    }

    gtab_dovscroll(hwnd, ptab, zDelta);

    return 0;
}

