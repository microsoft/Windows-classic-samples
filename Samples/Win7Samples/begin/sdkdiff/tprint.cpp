// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * standard table class.
 *
 * print functions.
 *
 * see table.h for interface description
 */

#include "precomp.h"
#include "gutilsrc.h"

#include "table.h"
#include "tpriv.h"

/* in tpaint.c, calls GetTextExtentPoint */
extern int GetTextExtent(HDC, LPSTR, int);

extern HANDLE hLibInst;

/* function prototypes */
lpTable gtab_printsetup(HWND hwnd, lpTable ptab, lpPrintContext pcontext);
BOOL gtab_prtwidths(HWND hwnd, lpTable ptab,  lpPrintContext
                    pcontext);
void gtab_printjob(HWND hwnd, lpTable ptab, lpPrintContext pcontext);
int APIENTRY AbortProc(HDC hpr, int code);
int APIENTRY AbortDlg(HWND hdlg, UINT msg, UINT wParam, LONG lParam);
BOOL gtab_printpage(HWND hwnd, lpTable ptab, lpPrintContext pcontext, int page);
void gtab_setrects(lpPrintContext pcontext, LPRECT rcinner, LPRECT rcouter);
void gtab_printhead(HWND hwnd, HDC hdc, lpTable ptab, lpTitle head, int page, BOOL fExpandChars);


/*
 * gtab_print
 */
BOOL
gtab_print(HWND hwnd, lpTable ptab, lpPrintContext pcontext)
{
    BOOL fNoContext, fNoMargin, fNoPD;
    BOOL fSuccess = TRUE;
    lpTable ptab_prt;

    fNoContext = FALSE;
    fNoPD = FALSE;
    fNoMargin = FALSE;

    if (pcontext == NULL) {
        fNoContext = TRUE;
        pcontext = (lpPrintContext) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                              sizeof(PrintContext));
        if (pcontext == NULL)
            return(FALSE);
        pcontext->head = pcontext->foot = NULL;
        pcontext->margin = NULL;
        pcontext->pd = NULL;
        pcontext->id = 0;
    }
    if (pcontext->pd == NULL) {
        fNoPD = TRUE;
    }
    if (pcontext->margin == NULL) {
        fNoMargin = TRUE;
    }
    ptab_prt = gtab_printsetup(hwnd, ptab, pcontext);

    if (ptab_prt != NULL) {
        gtab_printjob(hwnd, ptab_prt, pcontext);

        gtab_deltable(hwnd, ptab_prt);
    } else fSuccess = FALSE;
    if (fNoMargin) {
        HeapFree(GetProcessHeap(), NULL, pcontext->margin);
        pcontext->margin = NULL;
    }
    if (fNoPD) {
        if (NULL != pcontext->pd) 
        {
            if (pcontext->pd->hDevMode != NULL) {
                GlobalFree(pcontext->pd->hDevMode);
            }
            if (pcontext->pd->hDevNames != NULL) {
                GlobalFree(pcontext->pd->hDevNames);
            }
        }

        HeapFree(GetProcessHeap(), NULL, pcontext->pd);
        pcontext->pd = NULL;
    }
    if (fNoContext) {
        HeapFree(GetProcessHeap(), NULL, pcontext);
    }
    return fSuccess;
}



/*
 * gtab_printsetup()
 *
 * sets up printercontext - builds lpTable for printer, incl. sizing
 * and initialises pcontext fields that may be null.
 */
lpTable
gtab_printsetup(HWND hwnd, lpTable ptab, lpPrintContext pcontext)
{
    lpTable pprttab;
    PRINTDLG FAR * pd;
    int ncols, i;
    ColPropsList cplist;

    /* set fields for context that user left null */
    if (pcontext->margin == NULL) {
        pcontext->margin = (lpMargin) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Margin));
        if (pcontext->margin == NULL) {
            return(NULL);
        }
        pcontext->margin->left = 10;
        pcontext->margin->right = 10;
        pcontext->margin->top = 15;
        pcontext->margin->bottom = 15;
        pcontext->margin->topinner = 15;
        pcontext->margin->bottominner = 15;
    }

    if (pcontext->pd == NULL) {
        pd = (PRINTDLG FAR *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PRINTDLG));
        if (pd == NULL) {
            return(NULL);
        }
        pcontext->pd = pd;

        pd->lStructSize = sizeof(PRINTDLG);
        pd->hwndOwner = hwnd;
        pd->hDevMode = (HANDLE) NULL;
        pd->hDevNames = (HANDLE) NULL;
        pd->Flags = PD_RETURNDC|PD_RETURNDEFAULT;

        if (PrintDlg(pd) == FALSE) {
            return(NULL);
        }
    }

    /* now create a Table struct by querying the owner */
    pprttab = (lpTable)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Table));

    if (pprttab == NULL) {
        return(NULL);
    }
    pprttab->hdr = ptab->hdr;
    pprttab->tabchars = ptab->tabchars;
    pprttab->show_whitespace = ptab->show_whitespace;

    /* get the row/column count from owner window */
    if (pcontext->id == 0) {
        pprttab->hdr.id = ptab->hdr.id;
    } else {
        pprttab->hdr.id = pcontext->id;
    }
    pprttab->hdr.props.valid = 0;
    pprttab->hdr.sendscroll = FALSE;
    if (gtab_sendtq(hwnd, TQ_GETSIZE, (LPARAM)&pprttab->hdr) == FALSE) {
        return(NULL);
    }

    /* alloc and init the col data structs */
    ncols = pprttab->hdr.ncols;
    pprttab->pcolhdr = (lpColProps) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ColProps) * ncols);
    if (pprttab->pcolhdr == NULL) {
        HeapFree(GetProcessHeap(), NULL, pprttab);
        return(NULL);
    }

    /* init col properties to default */
    for (i=0; i < ncols; i++) {
        pprttab->pcolhdr[i].props.valid = 0;
        pprttab->pcolhdr[i].nchars = 0;
    }
    /* get the column props from owner */
    cplist.plist = pprttab->pcolhdr;
    cplist.id = pprttab->hdr.id;
    cplist.startcol = 0;
    cplist.ncols = ncols;
    gtab_sendtq(hwnd, TQ_GETCOLPROPS, (LPARAM)&cplist);


    pprttab->scrollscale = 1;
    pprttab->pcellpos = (lpCellPos) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
                                              sizeof(CellPos) * ptab->hdr.ncols);
    if (pprttab->pcellpos == NULL) {
        HeapFree(GetProcessHeap(), NULL, pprttab->pcolhdr);
        HeapFree(GetProcessHeap(), NULL, pprttab);
        return(NULL);
    }


    pprttab->pdata = NULL;
    pprttab->nlines = 0;

    if (!gtab_prtwidths(hwnd, pprttab, pcontext)) {
        HeapFree(GetProcessHeap(), NULL, pprttab->pcellpos);
        HeapFree(GetProcessHeap(), NULL, pprttab);
        return(NULL);
    }
    return(pprttab);
}


/* calc the height/width settings and alloc line data */
BOOL
gtab_prtwidths(HWND hwnd, lpTable ptab, lpPrintContext pcontext)
{
    TEXTMETRIC tm;
    int cx, cxtotal, i, curx, cury;
    lpProps hdrprops, cellprops;
    lpCellPos xpos, ypos;
    RECT rcinner, rcouter;

    hdrprops = &ptab->hdr.props;
    GetTextMetrics(pcontext->pd->hDC, &tm);
    ptab->avewidth = tm.tmAveCharWidth;
    ptab->rowheight = tm.tmHeight + tm.tmExternalLeading;
    if (hdrprops->valid & P_HEIGHT) {
        ptab->rowheight = hdrprops->height;
    }

    /* set sizes for headers */
    gtab_setrects(pcontext, &rcinner, &rcouter);

    /* set width/pos for each col. */
    cxtotal = 0;
    curx = rcinner.left;
    for (i = 0; i < ptab->hdr.ncols; i++) {
        cellprops = &ptab->pcolhdr[i].props;
        xpos = &ptab->pcellpos[i];

        if (cellprops->valid & P_WIDTH) {
            cx = cellprops->width;
        } else if (hdrprops->valid & P_WIDTH) {
            cx = hdrprops->width;
        } else {
            cx = ptab->pcolhdr[i].nchars + 1;
            cx *= ptab->avewidth;
        }
        /* add 2 for intercol spacing */
        cx += 2;

        xpos->size = cx;
        xpos->start = curx + 1;
        xpos->clipstart = xpos->start;
        xpos->clipend = xpos->start + xpos->size - 2;
        xpos->clipend = min(xpos->clipend, rcinner.right);

        cxtotal += xpos->size;
        curx += xpos->size;
    }
    ptab->rowwidth = cxtotal;

    if (pcontext->head != NULL) {
        xpos = &pcontext->head->xpos;
        ypos = &pcontext->head->ypos;

        xpos->start = rcouter.left + 1;
        xpos->clipstart = rcouter.left + 1;
        xpos->clipend = rcouter.right - 1;
        xpos->size = rcouter.right - rcouter.left;

        ypos->start = rcouter.top;
        ypos->clipstart = rcouter.top;
        ypos->clipend = rcinner.top;
        ypos->size = ptab->rowheight;
    }

    if (pcontext->foot != NULL) {
        xpos = &pcontext->foot->xpos;
        ypos = &pcontext->foot->ypos;

        xpos->start = rcouter.left + 1;
        xpos->clipstart = rcouter.left + 1;
        xpos->clipend = rcouter.right - 1;
        xpos->size = rcouter.right - rcouter.left;

        ypos->start = rcouter.bottom - ptab->rowheight;
        ypos->clipstart = rcinner.bottom;
        ypos->clipend = rcouter.bottom;
        ypos->size = ptab->rowheight;
    }

    /* set nr of lines per page */
    ptab->nlines = (rcinner.bottom - rcinner.top) / ptab->rowheight;
    if (!gtab_alloclinedata(hwnd, ptab)) {
        return(FALSE);
    }
    /* set line positions */
    cury = rcinner.top;
    for (i = 0; i < ptab->nlines; i++) {
        ypos = &ptab->pdata[i].linepos;
        ypos->start = cury;
        ypos->clipstart = ypos->start;
        ypos->clipend = ypos->start + ypos->size;
        ypos->clipend = min(ypos->clipend, rcinner.bottom);
        cury += ypos->size;
    }
    return(TRUE);
}


/* static information for this module */ 
BOOL g_bAbort;
FARPROC lpAbortProc;
DLGPROC lpAbortDlg;
HWND hAbortWnd;
int npage;
int pages;

void
gtab_printjob(HWND hwnd, lpTable ptab, lpPrintContext pcontext)
{
    int moveables;
    int endpage;
    int startpage = 1;
    HDC hpr;
    int status;
    HANDLE hcurs;
    static char str[256];
    DOCINFO di;
    TCHAR szPage[60];  /* for LoadString */

    hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

    moveables = ptab->nlines - ptab->hdr.fixedrows;
    pages = (int) (ptab->hdr.nrows - ptab->hdr.fixedrows + moveables - 1)
            / moveables;
    endpage = pages;

    if (pcontext->pd->Flags & PD_PAGENUMS) {
        startpage = pcontext->pd->nFromPage;
        endpage = pcontext->pd->nToPage;
    }
    hpr = pcontext->pd->hDC;

    lpAbortDlg = (DLGPROC) MakeProcInstance((WINPROCTYPE) AbortDlg, hLibInst);
    lpAbortProc = (FARPROC) MakeProcInstance((WINPROCTYPE)AbortProc, hLibInst);

    SetAbortProc(hpr, (ABORTPROC) lpAbortProc);

    di.lpszDocName = "Table";
    di.cbSize = lstrlen(di.lpszDocName);
    di.lpszOutput = NULL;
    di.lpszDatatype = NULL;
    di.fwType = 0;

    StartDoc(hpr, &di);

    g_bAbort = FALSE;

    /* add abort modeless dialog later!! */
    hAbortWnd = CreateDialog((HINSTANCE)hLibInst, "GABRTDLG", hwnd, lpAbortDlg);
    if (hAbortWnd != NULL) {
        ShowWindow(hAbortWnd, SW_NORMAL);
        EnableWindow(hwnd, FALSE);
    }
    SetCursor((HCURSOR)hcurs);


    status = 0;  /* kills a "used without init" diagnostic */
    for (npage = startpage; npage<=endpage; npage++) {
        LoadString((HINSTANCE)hLibInst,IDS_PAGE_STR,szPage,sizeof(szPage));
        HRESULT hr = StringCchPrintf(str, 256, szPage,  npage, pages);
        if (FAILED(hr))
            OutputError(hr, IDS_SAFE_PRINTF);
        if (hAbortWnd != NULL)
            SetDlgItemText(hAbortWnd, IDC_LPAGENR, str);
        status = gtab_printpage(hwnd, ptab, pcontext, npage);
        if (status < 0) {
            AbortDoc(hpr);
            break;
        }
    }
    if (status >= 0) {
        EndDoc(hpr);
    }

    if (hAbortWnd != NULL) {
        EnableWindow(hwnd, TRUE);
        DestroyWindow(hAbortWnd);
    }
    FreeProcInstance((WINPROCTYPE) lpAbortDlg);
    FreeProcInstance(lpAbortProc);

    DeleteDC(hpr);
}

int APIENTRY
AbortProc(HDC hpr, int code)
{

    MSG msg;

    if (!hAbortWnd) {
        return(TRUE);
    }
    while (!g_bAbort && PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!IsDialogMessage(hAbortWnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return(!g_bAbort);
}

int APIENTRY
AbortDlg(HWND hdlg, UINT msg, UINT wParam, LONG lParam)
{
    switch (msg) {
    
    case WM_COMMAND:
        g_bAbort = TRUE;
        EndDialog(hdlg, TRUE);
        return TRUE;

    case WM_INITDIALOG:
        return TRUE;
    }
    return(FALSE);
}

/*
 * print a single page. page number is 1-based
 */
BOOL
gtab_printpage(HWND hwnd, lpTable ptab, lpPrintContext pcontext, int page)
{
    HDC hpr;
    int moveables, i;
    int x1, y1, x2, y2;

    hpr = pcontext->pd->hDC;
    StartPage(hpr);

    moveables = ptab->nlines - ptab->hdr.fixedrows;
    ptab->toprow = moveables * (page-1);
    gtab_invallines(hwnd, ptab, ptab->hdr.fixedrows, moveables);

    for (i =0; i < ptab->nlines; i++) {
        gtab_paintline(hwnd, hpr, ptab, i, ptab->show_whitespace, TRUE);
    }
    if ((ptab->hdr.vseparator) && (ptab->hdr.fixedcols > 0)) {
        x1 = ptab->pcellpos[ptab->hdr.fixedcols -1].clipend+1;
        y1 = ptab->pdata[0].linepos.clipstart;
        y2 = ptab->pdata[ptab->nlines-1].linepos.clipend;
        MoveToEx(hpr, x1, y1, NULL);
        LineTo(hpr, x1, y2);
    }
    if ((ptab->hdr.hseparator) && (ptab->hdr.fixedrows > 0)) {
        y1 = ptab->pdata[ptab->hdr.fixedrows-1].linepos.clipend;
        x1 = ptab->pcellpos[0].clipstart;
        x2 = ptab->pcellpos[ptab->hdr.ncols-1].clipend;
        MoveToEx(hpr, x1, y1, NULL);
        LineTo(hpr, x2, y1);
    }

    if (pcontext->head != NULL) {
        gtab_printhead(hwnd, hpr, ptab, pcontext->head, page, FALSE);
    }
    if (pcontext->foot != NULL) {
        gtab_printhead(hwnd, hpr, ptab, pcontext->foot, page, TRUE);
    }

    return(EndPage(hpr));
}


/*
 * calculate the outline positions in pixels for the headers
 * (outer rect) and for the page itself (inner rect). Based on
 * page size and PrintContext margin info (which is in millimetres).
 */
void
gtab_setrects(lpPrintContext pcontext, LPRECT rcinner, LPRECT rcouter)
{
    HDC hpr;
    int hpixels, hmms;
    int vpixels, vmms;
    int h_pixpermm, v_pixpermm;

    hpr = pcontext->pd->hDC;
    hpixels = GetDeviceCaps(hpr, HORZRES);
    vpixels = GetDeviceCaps(hpr, VERTRES);
    vmms = GetDeviceCaps(hpr, VERTSIZE);
    hmms = GetDeviceCaps(hpr, HORZSIZE);

    h_pixpermm = hpixels / hmms;
    v_pixpermm = vpixels / vmms;

    rcouter->top = (pcontext->margin->top * v_pixpermm);
    rcouter->bottom = vpixels - (pcontext->margin->bottom * v_pixpermm);
    rcouter->left = (pcontext->margin->left * h_pixpermm);
    rcouter->right = hpixels - (pcontext->margin->right * h_pixpermm);

    rcinner->left = rcouter->left;
    rcinner->right = rcouter->right;
    rcinner->top = rcouter->top +
                   (pcontext->margin->topinner * v_pixpermm);
    rcinner->bottom = rcouter->bottom -
                      (pcontext->margin->bottominner * v_pixpermm);
}


void
gtab_printhead(HWND hwnd, HDC hdc, lpTable ptab, lpTitle head, int page, BOOL fExpandChars)
{
    RECT rc, rcbox;
    int i, cx, x, y, tab;
    UINT align;
    LPSTR chp, tabp;
    DWORD fcol, bkcol;
    TCHAR str[MAX_PATH * 2];
    TCHAR szbuffer[MAX_PATH];
    HRESULT hr;

    fcol = 0; bkcol = 0;  /* eliminate spurious diagnostic - generate worse code */

    rc.top = head->ypos.clipstart;
    rc.bottom = head->ypos.clipend;
    rc.left = head->xpos.clipstart;
    rc.right = head->xpos.clipend;
    memset(str, 0, MAX_PATH*2);

    /* update page number */
    if (fExpandChars) {
        chp = str;
        for (i = 0; i < lstrlen(head->ptext); i++) {
            memset(szbuffer, 0, MAX_PATH);
            switch (head->ptext[i]) {
            
            case '#':
                hr = StringCchPrintf(szbuffer, MAX_PATH, "%d", page);
                if (FAILED(hr)) {
                    OutputError(hr, IDS_SAFE_PRINTF);
                }
                hr = StringCchCat(str, MAX_PATH*2, szbuffer);
                if (FAILED(hr)) {
                    OutputError(hr, IDS_SAFE_CAT);
                }
                chp += strlen(szbuffer);
                break;

            case '$':
                hr = StringCchPrintf(szbuffer, MAX_PATH, "%d", pages);
                if (FAILED(hr))
                    OutputError(hr, IDS_SAFE_PRINTF);
                hr = StringCchCat(str, MAX_PATH*2, szbuffer);
                if (FAILED(hr)) {
                    OutputError(hr, IDS_SAFE_CAT);
                }
                chp += strlen(szbuffer);
                break;

            default:
                if (IsDBCSLeadByte(head->ptext[i]) &&
                    head->ptext[i+1]) 
                    {
                    *chp = head->ptext[i];
                    chp++;
                    i++;
                }
                *chp++ = head->ptext[i];
                break;
            }
        }
        *chp = '\0';
    } else {
        hr = StringCchCopy(str,(MAX_PATH*2), head->ptext);
        if (FAILED(hr))
            OutputError(hr, IDS_SAFE_COPY);
    }
    chp = str;

    if (head->props.valid & P_ALIGN) {
        align = head->props.alignment;
    } else {
        align = P_LEFT;
    }

    /* set colours if not default */
    if (head->props.valid & P_FCOLOUR) {
        fcol = SetTextColor(hdc, head->props.forecolour);
    }
    if (head->props.valid & P_BCOLOUR) {
        bkcol = SetBkColor(hdc, head->props.backcolour);
    }

    /* calc offset of text within cell for right-align or centering */
    if (align == P_LEFT) {
        cx = ptab->avewidth/2;
    } else {
        if (NULL == chp) {
            cx = 0;
        } else {
            cx = LOWORD(GetTextExtent(hdc, chp, lstrlen(chp)));
        }

        if (align == P_CENTRE) {
            cx = (head->xpos.size - cx) / 2;
        } else {
            cx = head->xpos.size - cx - (ptab->avewidth/2);
        }
    }
    cx += head->xpos.start;

    /* expand tabs on output */
    tab = ptab->avewidth * ptab->tabchars;
    x = 0;
    y = head->ypos.start;

    for ( ; (tabp = My_mbschr(chp, '\t')) != NULL; ) {
        /* perform output upto tab char */
        ExtTextOut(hdc, x+cx, y, ETO_CLIPPED, &rc, chp, (UINT)(tabp-chp), NULL);

        /* advance past the tab */
        x += LOWORD(GetTextExtent(hdc, chp, (INT)(tabp - chp)));
        x = ( (x + tab) / tab) * tab;
        chp = ++tabp;
    }

    /*no more tabs - output rest of string */
    if (NULL != chp) {
        ExtTextOut(hdc, x+cx, y, ETO_CLIPPED, &rc, chp, lstrlen(chp), NULL);
    }

    /* reset colours to original if not default */
    if (head->props.valid & P_FCOLOUR) {
        SetTextColor(hdc, fcol);
    }
    if (head->props.valid & P_BCOLOUR) {
        SetBkColor(hdc, bkcol);
    }

    /* now box cell if marked */
    if (head->props.valid & P_BOX) {
        if (head->props.box != 0) {
            rcbox.top = head->ypos.start;
            rcbox.bottom = rcbox.top + head->ypos.size;
            rcbox.left = head->xpos.start;
            rcbox.right = rcbox.left + head->xpos.size;
            gtab_boxcell(hwnd, hdc, &rcbox, &rc, head->props.box);
        }
    }
}
