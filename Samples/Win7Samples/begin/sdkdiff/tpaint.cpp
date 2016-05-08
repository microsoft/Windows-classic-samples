// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * standard table class.
 *
 * paint functions.
 *
 * see table.h for interface description
 */

#include "precomp.h"

#include "table.h"
#include "tpriv.h"

int
GetTextExtent(HDC hdc, LPSTR text, INT len)
{
    SIZE sz;

    GetTextExtentPoint(hdc, text, len, &sz);
    return(sz.cx);
}

void gtab_updatecontig(HWND hwnd, lpTable ptab, int line, int cell1, int count);

/* change all cr/lf chars in input text to nul chars */
void gtab_delcr(LPSTR ptext)
{
    LPSTR chp;

    if (ptext == NULL) {
        return;
    }
    for (chp = ptext; (chp = My_mbschr(chp, '\r')) != NULL; ) {
        *chp = '\0';
    }
    for (chp = ptext; (chp = My_mbschr(chp, '\n')) != NULL; ) {
        *chp = '\0';
    }
}
void gtab_delcrW(LPWSTR pwzText)
{
    LPWSTR pwch;

    if (pwzText)
        for (pwch = pwzText; *pwch; pwch++)
            if (*pwch == '\r' || *pwch == '\n')
                *pwch = 0;
}

/* ensure that all visible cells in the given line have valid
 * text and property contents. loop through the cells, picking out
 * contiguous blocks of visible, invalid cells and call
 * gtab_updatecontig to update these from the owner window.
 */
void
gtab_updateline(HWND hwnd, lpTable ptab, int line)
{
    lpCellPos ppos;
    int cell1, cellcount;
    lpLineData pline;
    lpCellData cd;
    int i;

    if (line < 0 || line >= ptab->nlines)
        return;

    pline = &ptab->pdata[line];
    cell1 = 0;
    cellcount = 0;
    for (i = 0; i < ptab->hdr.ncols; i++) {
        ppos = &ptab->pcellpos[i];
        cd = &pline->pdata[i];
        if (ppos->clipstart < ppos->clipend) {
            if ((cd->flags & CELL_VALID) == 0) {
                /* add a cell to the list to be updated*/
                if (cellcount++ == 0) {
                    cell1 = i;
                }
            } else {
                /* this cell already valid - so end of
                 * a contig block. if the contig
                 * block just ended contained cells to update,
                 * do it now
                 */
                if (cellcount > 0) {
                    gtab_updatecontig(hwnd, ptab,
                                      line, cell1, cellcount);
                }
                cellcount = 0;
            }
        }
        /* cell not visible - end of a contig block. If it was a
         * non-empty contig block, then update it now.
         */
        if (cellcount > 0) {
            gtab_updatecontig(hwnd, ptab, line, cell1, cellcount);
            cellcount = 0;  
        }
    }
    if (cellcount > 0) {
        gtab_updatecontig(hwnd, ptab, line, cell1, cellcount);
        cellcount = 0;
    }
}

/*
 * update a contiguous block of invalid cells by calling the owner window
 */
void
gtab_updatecontig(HWND hwnd, lpTable ptab, int line, int cell1, int count)
{
    lpLineData pline;
    lpCellData cd;
    CellDataList list;
    lpProps colprops;
    int i;
    COLORREF rgb;

    pline = &ptab->pdata[line];
    cd = &pline->pdata[cell1];

    list.id = ptab->hdr.id;
    list.row = gtab_linetorow(hwnd, ptab, line);
    list.startcell = cell1;
    list.ncells = count;
    list.plist = cd;

    /* clear out prop flags */
    rgb = GetSysColor(COLOR_WINDOW);
    for (i = 0; i < count; i++) {
        cd[i].props.valid = P_BCOLOUR;
        cd[i].props.backcolour = rgb;
        if (cd[i].nchars > 0) {
            cd[i].ptext[0] = '\0';
            if (cd[i].pwzText) {
                cd[i].pwzText[0] = '\0';
            }
        }
    }

    if (list.row < ptab->hdr.nrows) {
        gtab_sendtq(hwnd, TQ_GETDATA, (LPARAM) &list);
    }

    /* for each cell, mark valid and set properties */
    for (i = 0; i < count; i++) {
        cd[i].flags |= CELL_VALID;
        gtab_delcr(cd[i].ptext);
        gtab_delcrW(cd[i].pwzText);
        /* fetch properties from hdr and colhdr */
        colprops = &ptab->pcolhdr[i + cell1].props;
        if (!(cd[i].props.valid & P_FCOLOUR)) {
            if (colprops->valid & P_FCOLOUR) {
                cd[i].props.valid |= P_FCOLOUR;
                cd[i].props.forecolour = colprops->forecolour;
            } else if (ptab->hdr.props.valid & P_FCOLOUR) {
                cd[i].props.valid |= P_FCOLOUR;
                cd[i].props.forecolour =
                ptab->hdr.props.forecolour;
            }
        }

        if (!(cd[i].props.valid & P_FCOLOURWS)) {
            if (colprops->valid & P_FCOLOURWS) {
                cd[i].props.valid |= P_FCOLOURWS;
                cd[i].props.forecolourws = colprops->forecolourws;
            } else if (ptab->hdr.props.valid & P_FCOLOURWS) {
                cd[i].props.valid |= P_FCOLOURWS;
                cd[i].props.forecolourws =
                ptab->hdr.props.forecolourws;
            }
        }

        if (!(cd[i].props.valid & P_BCOLOUR)) {
            if (colprops->valid & P_BCOLOUR) {
                cd[i].props.valid |= P_BCOLOUR;
                cd[i].props.backcolour = colprops->backcolour;
            } else if (ptab->hdr.props.valid & P_BCOLOUR) {
                cd[i].props.valid |= P_BCOLOUR;
                cd[i].props.backcolour =
                ptab->hdr.props.backcolour;
            }
        }

        if (!(cd[i].props.valid & P_FONT)) {
            if (colprops->valid & P_FONT) {
                cd[i].props.valid |= P_FONT;
                cd[i].props.hFont = colprops->hFont;
            } else if (ptab->hdr.props.valid & P_FONT) {
                cd[i].props.valid |= P_FONT;
                cd[i].props.hFont = ptab->hdr.props.hFont;
            }
        }

        if (!(cd[i].props.valid & P_ALIGN)) {
            if (colprops->valid & P_ALIGN) {
                cd[i].props.valid |= P_ALIGN;
                cd[i].props.alignment = colprops->alignment;
            } else if (ptab->hdr.props.valid & P_ALIGN) {
                cd[i].props.valid |= P_ALIGN;
                cd[i].props.alignment =
                ptab->hdr.props.alignment;
            }
        }

        if (!(cd[i].props.valid & P_BOX)) {
            if (colprops->valid & P_BOX) {
                cd[i].props.valid |= P_BOX;
                cd[i].props.box = colprops->box;
            } else if (ptab->hdr.props.valid & P_BOX) {
                cd[i].props.valid |= P_BOX;
                cd[i].props.box = ptab->hdr.props.box;
            }
        }
        /* you can't set width/height per cell - this
         * is ignored at cell level.
         */
    }

}

void
gtab_boxcell(HWND hwnd, HDC hdc, LPRECT rcp, LPRECT pclip, UINT boxmode)
{
    if (boxmode & P_BOXTOP) {
        MoveToEx(hdc, max(rcp->left, pclip->left),
                 max(rcp->top, pclip->top), NULL);
        LineTo(hdc, min(rcp->right, pclip->right),
               max(rcp->top, pclip->top));
    }
    if (boxmode & P_BOXBOTTOM) {
        MoveToEx(hdc, max(rcp->left, pclip->left),
                 min(rcp->bottom, pclip->bottom), NULL);
        LineTo(hdc, min(rcp->right, pclip->right),
               min(rcp->bottom, pclip->bottom));
    }
    if (boxmode & P_BOXLEFT) {
        MoveToEx(hdc, max(rcp->left, pclip->left),
                 max(rcp->top, pclip->top), NULL);
        MoveToEx(hdc, max(rcp->left, pclip->left),
                 min(rcp->bottom, pclip->bottom), NULL);
    }
    if (boxmode & P_BOXRIGHT) {
        MoveToEx(hdc, min(rcp->right, pclip->right),
                 max(rcp->top, pclip->top), NULL);
        LineTo(hdc, min(rcp->right, pclip->right),
               min(rcp->bottom, pclip->bottom));
    }
}

void
gtab_paintcell(HWND hwnd, HDC hdc, lpTable ptab, int line, int cell, BOOL show_whitespace, BOOL fPrinting)
{
    lpLineData pline;
    lpCellData cd;
    lpCellPos ppos;
    RECT rc, rcbox;
    int cx, x, y;
    UINT align;
    LPSTR chp, tabp=NULL;
    LPWSTR pwch, pwchBreak=NULL;
    DWORD fcol, fcolOld=0, fcolws=0;
    DWORD bkcol, bkcolOld=0;
    HFONT hfont, hfontOld=NULL;
    HBRUSH hbr;
    char szCharSet[] = "\t ";
    WCHAR wzCharSet[] = L"\t ";
    char szSpaceReplace[] = { (unsigned char)183,(unsigned char) 0};
    char szTabReplace[] = { (unsigned char)187, (unsigned char) 0};
    int cxSpaceReplace;
    int cxTabReplace;
    SIZE size ={0,0};
    TEXTMETRIC tm;
    int yOfs;
    DWORD etoFlags = ETO_CLIPPED;

    fcol = 0; bkcol = 0;
    hfont = 0;          
    /* init pointers to cell text and properties */
    pline = &ptab->pdata[line];
    cd = &pline->pdata[cell];
    ppos = &ptab->pcellpos[cell];

    /* draw gutter */
    rc.top = pline->linepos.clipstart;
    rc.bottom = pline->linepos.clipend;
    rc.left = (cell > 0) ? ptab->pcellpos[cell - 1].clipend : 0;
    rc.right = ppos->clipstart;
    if (cell > ptab->hdr.fixedcols && ptab->hdr.fixedcols < ptab->hdr.ncols) {
        rc.left = max(rc.left, ptab->pcellpos[ptab->hdr.fixedcols].clipstart);
    }
    if (ptab->hdr.fixedcols > 0 && cell == ptab->hdr.fixedcols) {
        rc.right--;
    }
    if (!fPrinting && rc.right > rc.left) {
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
    }

    /* clip all output to this rectangle */
    rc.top = pline->linepos.clipstart;
    rc.bottom = pline->linepos.clipend;
    rc.left = ppos->clipstart;
    rc.right = ppos->clipend;

    /* check cell properties and colours */
    if (cd->props.valid & P_ALIGN) {
        align = cd->props.alignment;
    } else {
        align = P_LEFT;
    }

    if (!fPrinting && cd->props.valid & P_FONT) {
        hfontOld = (HFONT)SelectObject(hdc, cd->props.hFont);
    }

    // get y offset to center text vertically within cell
    GetTextMetrics(hdc, &tm);
    yOfs = (rc.bottom - rc.top - tm.tmHeight) / 2;

    /* set replacement chars and char widths */
    cxSpaceReplace = GetTextExtent(hdc, szSpaceReplace, 1);
    cxTabReplace = cxSpaceReplace * ptab->tabchars;

    /* set colours if not default */
    if (!fPrinting) {
        if (cd->props.valid & P_FCOLOUR) {
            fcol = cd->props.forecolour;
            fcolOld = SetTextColor(hdc, fcol);
        }
        if (cd->props.valid & P_FCOLOURWS) {
            fcolws = cd->props.forecolourws;
        } 
        else {
            fcolws = fcol;
        }
        if (cd->props.valid & P_BCOLOUR) {
            /* there is a non-default background colour.
            * create a brush and fill the entire cell with it
             */
            hbr = CreateSolidBrush(cd->props.backcolour);
            if (hbr) 
            {
                FillRect(hdc, &rc, hbr);
                DeleteObject(hbr);
            }

            /* also set colour as background colour for the text */
            bkcolOld = SetBkColor(hdc, cd->props.backcolour);
        }
    }

    /* calc offset of text within cell for right-align or centering */
    if (align == P_LEFT) {
        cx = ptab->avewidth/2;
    } else {
        cx = 0;

        if (cd->pwzText) {
            GetTextExtentPoint32W(hdc, cd->pwzText, (int)wcslen(cd->pwzText), &size);
        } else if (cd->ptext) {
            GetTextExtentPoint32A(hdc, cd->ptext, (int)lstrlen(cd->ptext), &size);
        }
        cx = size.cx;

        if (align == P_CENTRE) {
            cx = (ppos->size - cx) / 2;
        } else {
            cx = ppos->size - cx - (ptab->avewidth/2);
        }
    }
    cx += ppos->start;

    /* expand tabs on output and show whitespace on output */
    x = 0;
    y = pline->linepos.start + yOfs;

    /* set search string for strpbrk fn;
       don't search for space chars unless we're showing whitespace */
    if (!show_whitespace) 
    {
        szCharSet[1] = '\0';
        wzCharSet[1] = '\0';
    }

    // determine the string to display (ansi/unicode).  if we have a string
    // and it's not empty, then loop and display it.
    chp = cd->ptext;
    pwch = cd->pwzText;
    if (pwch ? *pwch : (chp && *chp)) 
    {
        while (TRUE) 
        {
            if (pwch) 
            {
                pwchBreak = wcspbrk(pwch, wzCharSet);
                if (!pwchBreak)
                    pwchBreak = pwch + wcslen(pwch);
            } 
            else 
            {
                tabp = (LPSTR)My_mbspbrk((PUCHAR)chp, (PUCHAR)szCharSet);
                if (!tabp)
                    tabp = chp + lstrlen(chp);
            }

            /* perform output up to tab/space char */
            if (pwch)
                ExtTextOutW(hdc, x+cx, y, etoFlags, &rc, pwch, (UINT)(pwchBreak-pwch), NULL);
            else
                ExtTextOutA(hdc, x+cx, y, etoFlags, &rc, chp, (UINT)(tabp-chp), NULL);

            /* advance past the tab */
            if (pwch) 
            {
                GetTextExtentPoint32W(hdc, pwch, (UINT)(pwchBreak - pwch), &size);
                pwch = pwchBreak;
            } 
            else 
            {
                GetTextExtentPoint32A(hdc, chp, (UINT)(tabp - chp), &size);
                chp = tabp;
            }
            x += size.cx;

            // bail if we hit null terminator
            if (pwch ? !*pwch : !*chp)
                break;

            /* handle tab chars */
            while (pwch ? (*pwch == '\t') : (*chp == '\t')) 
            {
                /* print replacement char */
                if (show_whitespace) 
                {
                    if (!fPrinting)
                        SetTextColor(hdc, fcolws);

                    ExtTextOut(hdc, x + cx, y, etoFlags, &rc, szTabReplace, 1, NULL);

                    if (!fPrinting)
                        SetTextColor(hdc, fcol);
                }

                /* advance past the tab */
                if (cxTabReplace > 0)
                    x += cxTabReplace - (x % cxTabReplace);
                if (pwch)
                    pwch = ++pwchBreak;
                else
                    chp = ++tabp;
            }

            /* handle space chars */
            if (show_whitespace) 
            {
                while (pwch ? (*pwch == ' ') : (*chp == ' ')) 
                {
                    /* replace space char with visible char */
                    if (!fPrinting)
                        SetTextColor(hdc, fcolws);

                    ExtTextOut(hdc, x + cx, y, etoFlags, &rc, szSpaceReplace, 1, NULL);

                    if (!fPrinting)
                        SetTextColor(hdc, fcol);

                    x += cxSpaceReplace;
                    if (pwch)
                        pwch = ++pwchBreak;
                    else
                        chp = ++tabp;
                }
            }
        }
    }

    /* reset colours to original if not default */
    if (!fPrinting) {
        if (cd->props.valid & P_FCOLOUR) {
            SetTextColor(hdc, fcolOld);
        }
        if (cd->props.valid & P_BCOLOUR) {
            SetBkColor(hdc, bkcolOld);
        }
        if ( (cd->props.valid & P_FONT) && ( NULL != hfontOld)) {
            SelectObject(hdc, hfontOld);
        }
    }

    /* now box cell if marked */
    if (!fPrinting) {
        if ((cd->props.valid & P_BOX)) {
            if (cd->props.box != 0) {
                rcbox.top = y;
                rcbox.bottom = rcbox.top + pline->linepos.size;
                rcbox.left = ppos->start;
                rcbox.right = ppos->start + ppos->size;
                gtab_boxcell(hwnd, hdc, &rcbox, &rc, cd->props.box);
            }
        }
    }
}

/* fetch and paint the specified line */
void
gtab_paintline(HWND hwnd, HDC hdc, lpTable ptab, int line, BOOL show_whitespace, BOOL fPrinting)
{
    lpCellPos ppos = NULL;
    int i;
    RECT rc;

    if (line < 0 || line >= ptab->nlines)
        return;

    if (!fPrinting)
        GetClientRect(hwnd, &rc);

    gtab_updateline(hwnd, ptab, line);

    for (i = 0; i < ptab->hdr.ncols; i++) {
        ppos = &ptab->pcellpos[i];
        /* show whitespace iff the flag is set
           and we're painting the main text column */
        if (ppos->clipstart < ppos->clipend) {
            gtab_paintcell(hwnd, hdc, ptab, line, i,
                           (show_whitespace && (i == 2)), fPrinting);
        }
    }

    if (!fPrinting && ppos) {
        rc.top = ptab->pdata[line].linepos.clipstart;
        rc.bottom = ptab->pdata[line].linepos.clipend;
        rc.left = ppos->clipend;
        FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
    }
}

void
gtab_paint(HWND hwnd)
{
    PAINTSTRUCT ps;
    int y, y2, i;
    HDC hDC = BeginPaint(hwnd, &ps);
    lpTable ptab = (lpTable) GetWindowLongPtr(hwnd, WL_TABLE);

    if (!ptab) {
        FillRect(hDC, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
    } else {
        /* separator lines between fixed rows/columns
         * (ie headers) and the rest - if enabled
         */
        /* paint here first for good impression,
         * and again after to clean up!!
         */
        if (ptab->hdr.vseparator) {
            gtab_vsep(hwnd, ptab, hDC);
        }
        if (ptab->hdr.hseparator) {
            gtab_hsep(hwnd, ptab, hDC);
        }

        /* paint only the rows that need painting */
        for (i = 0; i < ptab->nlines; i++) {
            y = ptab->pdata[i].linepos.start;
            y2 = y + ptab->pdata[i].linepos.size;
            if ( (y <= ps.rcPaint.bottom) &&
                 (y2 >= ps.rcPaint.top)) {
                gtab_paintline(hwnd, hDC, ptab, i, ptab->show_whitespace, FALSE);
            }
        }
        if (ptab->hdr.vseparator) {
            gtab_vsep(hwnd, ptab, hDC);
        }
        if (ptab->hdr.hseparator) {
            gtab_hsep(hwnd, ptab, hDC);
        }
        if (ptab->selvisible) {
            gtab_invertsel(hwnd, ptab, hDC);
        }
    }

    EndPaint(hwnd, &ps);
}


void
gtab_vsep(HWND hwnd, lpTable ptab, HDC hdc)
{
    int x;
    RECT rc;

    if (ptab->hdr.fixedcols < 1) {
        return;
    }
    x = ptab->pcellpos[ptab->hdr.fixedcols - 1].clipend+1;
    GetClientRect(hwnd, &rc);
    MoveToEx(hdc, x, rc.top, NULL);
    LineTo(hdc, x, rc.bottom);
}

void
gtab_hsep(HWND hwnd, lpTable ptab, HDC hdc)
{
    int y;
    RECT rc;

    if (ptab->hdr.fixedrows < 1) {
        return;
    }
    y = ptab->rowheight * ptab->hdr.fixedrows;
    GetClientRect(hwnd, &rc);
    MoveToEx(hdc, rc.left, y-1, NULL);
    LineTo(hdc, rc.right, y-1);
}

/* draw in (inverting) the dotted selection lines for tracking a col width
 */
void
gtab_drawvertline(HWND hwnd, lpTable ptab)
{
    RECT rc;
    HDC hdc;
    HPEN hpen;

    hdc = GetDC(hwnd);
    if (hdc) 
    {
        SetROP2(hdc, R2_XORPEN);
        hpen = (HPEN)SelectObject(hdc, hpenDotted);
        GetClientRect(hwnd, &rc);

        MoveToEx(hdc, ptab->trackline1, rc.top, NULL);
        LineTo(hdc, ptab->trackline1, rc.bottom);
        if (ptab->trackline2 != -1) {
            MoveToEx(hdc, ptab->trackline2, rc.top, NULL);
            LineTo(hdc, ptab->trackline2, rc.bottom);
        }

        SelectObject(hdc, hpen);
        ReleaseDC(hwnd, hdc);
    }
}


/*
 * mark the selected line, if visible, in the style chosen by the
 * client app. This can be TM_SOLID, meaning an inversion of
 * the whole selected area or TM_FOCUS, meaning, inversion of the first
 * cell, and then a dotted focus rectangle for the rest.
 *
 * this function inverts either style, and so will turn the selection
 * both on and off.
 */
void
gtab_invertsel(HWND hwnd, lpTable ptab, HDC hdc_in)
{
    HDC hdc;
    int firstline, lastline;
    long startrow, lastrow, toprow, bottomrow;
    RECT rc;
    int lastcell;



    /* get the selection start and end rows ordered vertically */
    if (ptab->select.nrows == 0) {
        return;
    } else if (ptab->select.nrows < 0) {
        startrow = ptab->select.startrow + ptab->select.nrows + 1;
        lastrow = ptab->select.startrow;
    } else {
        startrow = ptab->select.startrow;
        lastrow = ptab->select.startrow + ptab->select.nrows -1;
    }

    /* is selected area (or part of it) visible on screen ?  */
    firstline = gtab_rowtoline(hwnd, ptab, startrow);
    lastline = gtab_rowtoline(hwnd, ptab, lastrow);


    if (firstline < 0) {
        toprow = gtab_linetorow(hwnd, ptab,
                                ptab->hdr.fixedselectable ? 0: ptab->hdr.fixedrows);
        if ((toprow >= startrow)  &&
            (toprow <= lastrow)) {
            firstline = gtab_rowtoline(hwnd, ptab, toprow);
        } else {
            return;
        }
    } else {
        toprow = 0;
    }


    if (lastline < 0) {
        bottomrow = gtab_linetorow(hwnd, ptab, ptab->nlines-1);
        if ((bottomrow <= lastrow) &&
            (bottomrow >=startrow)) {
            lastline = gtab_rowtoline(hwnd, ptab, bottomrow);
        } else {
            return;
        }
    }


    rc.top = ptab->pdata[firstline].linepos.clipstart;
    rc.bottom = ptab->pdata[lastline].linepos.clipend;



    /* selection mode includes a flag TM_FOCUS indicating we should
     * use a focus rect instead of the traditional inversion for
     * selections in this table. This interferes with multiple backgrnd
     * colours less.  However we still do inversion for fixedcols.
     */

    lastcell = (int)(ptab->select.startcell + ptab->select.ncells - 1);


    /*
     * invert the whole area for TM_SOLID or just the first
     * cell for TM_FOCUS
     */
    rc.left = ptab->pcellpos[ptab->select.startcell].clipstart;
    if (ptab->hdr.selectmode & TM_FOCUS) {
        rc.right = ptab->pcellpos[ptab->select.startcell].clipend;
    } else {
        rc.right = ptab->pcellpos[lastcell].clipend;
    }

    if (hdc_in == NULL) {
        hdc = GetDC(hwnd);
        if (!hdc) 
        {
            return;
        }
    } else {
        hdc = hdc_in;
    }

    InvertRect(hdc, &rc);

    /*
     * draw focus rectangle around remaining cells on this line, if there
     * are any
     */
    if (ptab->hdr.selectmode & TM_FOCUS) {
        if (toprow > startrow) {
            rc.top--;
        }
        if (ptab->select.ncells > 1) {
            rc.left = ptab->pcellpos[ptab->select.startcell+1].clipstart;
            rc.right = ptab->pcellpos[lastcell].clipend;
            DrawFocusRect(hdc, &rc);
        }
    }

    if (hdc_in == NULL) {
        ReleaseDC(hwnd, hdc);
    }
}
