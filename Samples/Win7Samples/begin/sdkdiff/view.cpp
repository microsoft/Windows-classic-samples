// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * view.cpp
 *
 *       map rows in window to items in COMPLIST
 *
 *
 * A view owns a COMPLIST, and talks to a table window. The table window
 * shows 3 columns: line nr, tag and text. We also need to supply a state
 * for each row (used to select colour scheme).
 *
 * The COMPLIST can give us a list of its COMPITEMs. Each of these can give
 * us a tag (eg the filenames compared) and the text (usually the compare
 * result), and the state. We make the line number from the
 * COMPITEM's place in the list.
 *
 * If we are asked to switch to 'expand' mode, we ask the selected COMPITEM
 * for its composite section list. We can then get the state (and thus
 * the tag) from each SECTION, and the line nr and text from the LINEs within
 * each section.
 *
 * When moving between expand and outline, and when refreshing the view
 * for some option change, we have to be careful to keep the current row
 * and the selected row in the table what the user would expect (!)
 *
 *
 * WIN32: Functions in this module can be called from the UI thread (to refresh
 * the display) and simultaneously from a worker thread to update the
 * view mapping (view_setcomplist, view_newitem). We use a critical section
 * to manage the synchronisation. We need to protect all access/modification
 * to the view structure elements (particularly bExpand, rows, pLines and
 * pItems), BUT we must not hold the critical section over any calls
 * to SendMessage.
 *
 */

#include "precomp.h"
#include "table.h"

#include "state.h"
#include "sdkdiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "section.h"
#include "compitem.h"
#include "complist.h"
#include "findgoto.h"

#include "view.h"

/*
 * data structures
 */
#ifdef WIN32
    #define huge
#endif

/* in expand mode, we keep an array of one of these per screen line. */
typedef struct viewline {
    LINE line;              /* handle to LINE for this row */
    SECTION section;        /* handle to section containing this line */
    int nr_left;            /* line nr in left file */
    int nr_right;           /* line nr in right file */
} VIEWLINE, * PVIEWLINE;


/*
 * The users VIEW handle is in fact a pointer to this structure
 */
struct view {

    HWND     hwnd;          /* the table window to send notifies to */

    COMPLIST cl;            /* the complist that we own */

    BOOL     bExpand;       /* true if we are in expand mode */
    BOOL     bExpanding;    /* set by view_expandstart, reset by view_expand
                               interrogated in sdkdiff.cpp, causes keystrokes
                               to be ignored.  Protects against mappings being
                               messed up by another thread.
                               (I have doubts about this - Laurie).
                            */
    BOOL     bExpandGuard;  /* Protects against two threads both trying to
                               expand the same item.
                            */

    COMPITEM ciSelect;      /* selected compitem (in expand mode) */

    int      rows;          /* number of rows in this view */

    char     nrtext[12];    /* we use this in view_gettext for the line
                             * number column. overwritten on each call
                             */
    int      maxtag, maxrest;/* column widths in characters for cols 1, 2 */

    /* if we are in outline mode, we map the row number to one entry
     * in this array of COMPITEM handles. this pointer will
     * be NULL in expand mode
     */
    COMPITEM * pItems;

    /* in expand mode we use this array of line and section handles */
    PVIEWLINE pLines;
};

CRITICAL_SECTION CSView;       /* also known to Sdkdiff.cpp WM_EXIT processing */
static BOOL bDoneInit = FALSE;

#define ViewEnter()     EnterCriticalSection(&CSView);
#define ViewLeave()     LeaveCriticalSection(&CSView);

extern long selection;
extern long selection_nrows;

/*------- forward declaration of internal functions ----------------*/

void view_outline_opt(VIEW view, BOOL bRedraw, COMPITEM ci, int* prow);
void view_freemappings(VIEW view);
int view_findrow(VIEW view, int number, BOOL bRight);
BOOL view_expand_item(VIEW view, COMPITEM ci);


/* -----  externally-called functions---------------------------*/
/* view_new
 *
 * create a new view. at this point, we are told the table window handle,
 * and nothing else.
 *
 */
VIEW
view_new(HWND hwndTable)
{
    VIEW view;

    if (!bDoneInit) {
        InitializeCriticalSection(&CSView);
        bDoneInit = TRUE;
    }

    /* alloc the view using HeapAlloc */
    view = (VIEW) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct view));
    if (view == NULL) 
    {
        return NULL;
    }
    /* set the default fields */
    view->hwnd = hwndTable;
    view->cl = NULL;
    view->bExpand = FALSE;
    view->bExpandGuard = FALSE;
    view->ciSelect = NULL;
    view->rows = 0;
    view->pItems = NULL;
    view->pLines = NULL;

    // erase the message in the names field - the central box in the status bar
    SetNames("");
    return(view);
}


/*
 * view_setcomplist.
 *
 * We have to separate view_new and view_setcomplist because we need
 * to give the view handle to the complist and the complist handle to the
 * view. So do a view_new to create a null view; then complist_new() to
 * which you pass a view handle. The complist will then register itself
 * with the view by calling this function. During the build of the complist,
 * it will also update us by calling view_additem, so that we can refresh
 * the display.
 *
 * Here we should initialise an outline view of the complist.
 *
 * We also talk to the status bar using SetNames to set the names of
 * the two items.
 */
BOOL
view_setcomplist(VIEW view, COMPLIST cl)
{
    LPSTR both;

    if (view == NULL) {
        return(FALSE);
    }

    /* there can be only one call to this per VIEW */
    if (view->cl != NULL) {
        return(FALSE);
    }

    ViewEnter();

    view->cl = cl;

    /* set names on status bar to root names of left and right trees */
    both = complist_getdescription(cl);
    ViewLeave();                      // LKGHACK
    SetNames(both);
    ViewEnter();                      // LKGHACK
    complist_freedescription(cl, both);

    ViewLeave();

    view_outline(view);
    return TRUE;
}


/*
 * return a handle to the complist owned by this view
 */
COMPLIST
view_getcomplist(VIEW view)
{
    if (view == NULL) {
        return(NULL);
    }

    return(view->cl);
}


/*
 * close a view. notify the table window that this view should be
 * closed. When the table window has finished with it, it will send
 * a TQ_CLOSE notify that should result in view_delete being called
 * and the memory being freed.
 */
void
view_close(VIEW view)
{
    if (view == NULL) {
        return;
    }

    SendMessage(view->hwnd, TM_NEWID, 0, 0);
}


/*
 * delete a view and all associated data.
 *
 * This function should only be called in response to the table window
 * sending a TQ_CLOSE message. To close the view, call view_close and
 * wait for the TQ_CLOSE before calling this.
 *
 * We delete the associated COMPLIST and all its associated structures.
 */
void
view_delete(VIEW view)
{
    if (view == NULL) {
        return;
    }

    /* we have two arrays that are used for the mapping - an array
     * of compitem handles in outline mode, and an array of
     * VIEWLINE structures in expand mode
     */

    view_freemappings(view);

    complist_delete(view->cl);

    HeapFree(GetProcessHeap(), NULL, view);
}


/*
 * view_outline
 *
 * build an outline mode mapping where one row represents one COMPITEM in
 * the list. check the global option flag outline_include to see which items
 * we should include.
 *
 * If we were in expand mode, then set as the selection the row in outline mode
 * that we were expanding. Also remember to free up the expand mode mapping
 * array
 *
 * once we have built the new mapping, notify the table window to
 * redraw itself.
 */
void
view_outline(VIEW view)
{
    if (view == NULL) {
        return;
    }

    /* all work done by view_outline_opt - this function
     * gives us the option of not updating the display
     */
    view_outline_opt(view, TRUE, NULL, NULL);
}



/*
 * switch to expand mode, expanding the given row into a view
 * of the differences in that file.
 *
 * map the given row nr into a compitem handle, and then
 * call the internal function with that.
 *
 * It is legal (and a no-op) if this function is called with
 * row==-1.
 */
BOOL
view_expand(VIEW view, long row)
{
    COMPITEM ci;
    BOOL bRet;

    if (row<0) return FALSE; 

    ViewEnter();

    if ((view == NULL) || (view->bExpand)) {
        /* no view, or already expanded */
        ViewLeave();
        return(FALSE);
    }

    if (row >= view->rows) {
        /* no such row */
        ViewLeave();
        return FALSE;
    }

    /* remember the compitem we are expanding */
    ci = view->pItems[row];

    bRet = view_expand_item(view, ci);
    return(bRet);
}


/*
 * return the text associated with a given column of a given row.
 * Return a pointer that does not need to be freed after use - ie
 * a pointer into our data somewhere, not a copy
 */
LPSTR
view_gettext(VIEW view, long row, int col)
{
    int line;
    int state;
    LPSTR pstr;
    HRESULT hr;

    pstr = NULL;   /* kill spurious diagnostic */
    if (view == NULL) {
        return(NULL);
    }

    ViewEnter();

    if ((0 > row) || (row >= view->rows)) {
        ViewLeave();
        return(NULL);
    }

    if (view->bExpand) {
        /* we are in expand mode */

        state = section_getstate(view->pLines[row].section);

        switch (col) {
        case 0:
            /* row nr */

            /* line numbers can be from either original file
             * this is a menu-selectable option
             */
            line = 0;
            switch (line_numbers) {
            case IDM_NONRS:
                pstr = NULL;
                break;

            case IDM_LNRS:
                line = view->pLines[row].nr_left;
                if (state == STATE_MOVEDRIGHT
                    || state == STATE_SIMILARRIGHT) {
                    line = -line;
                }
                break;

            case IDM_RNRS:
                line = view->pLines[row].nr_right;
                if (state == STATE_MOVEDLEFT
                    || state == STATE_SIMILARLEFT) {
                    line = -line;
                }
                break;
            }
            if (line == 0) {
                ViewLeave();
                return(NULL);
            }

            if (line < 0) {
                /* lines that are moved appear twice.
                 * show the correct-sequence line nr
                 * for the out-of-seq. copy in brackets.
                 */
                hr = StringCchPrintf(view->nrtext, 12, "(%d)", abs(line));
                if (FAILED(hr)) {
                    OutputError(hr, IDS_SAFE_PRINTF);
                    return(NULL);
                }

            } else {
                hr = StringCchPrintf(view->nrtext, 12, "%d", line);
                if (FAILED(hr)) {
                    OutputError(hr, IDS_SAFE_PRINTF);
                    return(NULL);
                }
            }
            pstr = view->nrtext;
            break;

        case 1:
            /* tag text - represents the state of the line */


            switch (state) {
            case STATE_SAME:
                pstr = "    ";
                break;

            case STATE_LEFTONLY:
            case STATE_SIMILARLEFT:
                pstr = " <! ";
                break;

            case STATE_RIGHTONLY:
            case STATE_SIMILARRIGHT:
                pstr = " !> ";
                break;

            case STATE_MOVEDLEFT:
                pstr = " <- ";
                break;

            case STATE_MOVEDRIGHT:
                pstr = " -> ";
                break;
            }
            break;

        case 2:
            /* main text - line */
            pstr = line_gettext(view->pLines[row].line);
            break;
        }
    } else {
        /* outline mode */
        switch (col) {
        case 0:
            /* row number - just the line number */
            hr = StringCchPrintf(view->nrtext, 12, "%d", row+1);
            if (FAILED(hr)) {
                OutputError(hr, IDS_SAFE_PRINTF);
                return(NULL);
            }
            pstr = view->nrtext;
            break;

        case 1:
            /* tag */
            pstr = compitem_gettext_tag(view->pItems[row]);
            break;

        case 2:
            /* result text */
            pstr = compitem_gettext_result(view->pItems[row]);
            break;
        }
    }
    ViewLeave();
    return(pstr);
}


/*
 * return the text associated with a given column of a given row.
 * Return a pointer that does not need to be freed after use - ie
 * a pointer into our data somewhere, not a copy
 */
LPWSTR
view_gettextW(VIEW view, long row, int col)
{
    int state;
    LPWSTR pwz;

    pwz = NULL;   /* kill spurious diagnostic */
    if (view == NULL) {
        return(NULL);
    }

    ViewEnter();

    if ((0 > row) || (row >= view->rows)) {
        ViewLeave();
        return(NULL);
    }

    if (view->bExpand) {
        /* we are in expand mode */

        state = section_getstate(view->pLines[row].section);

        switch (col) {
        case 2:
            /* main text - line */
            pwz = line_gettextW(view->pLines[row].line);
            break;
        }
    }
    ViewLeave();
    return(pwz);
}

/*
 * return the line number that this row had in the original left
 * file. 0 if not in expand mode. 0 if this row was not in the left file.
 * -(linenr) if this row is a MOVED line, and this is the right file
 * copy
 */
int
view_getlinenr_left(VIEW view, long row)
{
    int state, line;

    if ((0> row) || (view == NULL) || (row >= view->rows) || !view->bExpand) {
        return 0;
    }

    ViewEnter();
    state = section_getstate(view->pLines[row].section);
    line = view->pLines[row].nr_left;
    if (state == STATE_MOVEDRIGHT || state == STATE_SIMILARRIGHT) {
        line = -line;
    }
    ViewLeave();

    return(line);
}

/*
 * return the line number that this row had in the original right
 * file. 0 if not in expand mode. 0 if this row was not in the right file.
 * -(linenr) if this row is a MOVED line, and this is the left file
 * copy
 */
int
view_getlinenr_right(VIEW view, long row)
{
    int state, line;

    if ((0 > row) || (view == NULL) || (row > view->rows) || !view->bExpand) {
        return 0;
    }

    ViewEnter();

    state = section_getstate(view->pLines[row].section);
    line = view->pLines[row].nr_right;
    if (state == STATE_MOVEDLEFT || state == STATE_SIMILARLEFT) {
        line = -line;
    }
    ViewLeave();

    return(line);
}



/* find the maximum width in characters for the given column */
int
view_getwidth(VIEW view, int col)
{
    if (view == NULL) {
        return(0);
    }

    switch (col) {
    case 0:
        /* line nr column - always 5 characters wide */
        return(5);

    case 1:
        /* this is a proportional font field, so add on a margin
         * for error
         */
        return(view->maxtag + (view->maxtag / 20));
    case 2:
        /* this now includes the tab expansion allowance */
        return(view->maxrest);
    default:
        return(0);
    }
}

/* how many rows are there in this view ? */
long
view_getrowcount(VIEW view)
{
    if (view == NULL) {
        return(0);
    }

    return(view->rows);
}

/* return the state for the current row. This is used
 * to select the text colour for the row
 *
 * states for sections are obtained from section_getstate (and apply, and
 * to all lines in that section. States for compitems are obtained
 * from compitem_getstate.
 */
int
view_getstate(VIEW view, long row)
{
    int state;

    if (view == NULL) {
        return(0);
    }

    ViewEnter();
    if ( (row >= view->rows) || (row < 0)) {
        state = 0;
    } else if (view->bExpand) {
        /* its a line state that's needed */
        state = section_getstate(view->pLines[row].section);
    } else {

        /* its a compitem state */
        state = compitem_getstate(view->pItems[row]);
    }
    ViewLeave();
    return(state);
}

/*
 * return the marked state for a given row. Only compitems can be marked,
 * so it will be FALSE unless it is a compitem on which view_setmark or
 * compitem_setmark have previously set the mark state to TRUE.
 */
BOOL
view_getmarkstate(VIEW view, long row)
{
    BOOL bMark = FALSE;

    if (view != NULL) {
        ViewEnter();
        if ( (0 < row) && (row < view->rows) && (!view->bExpand)) {
            bMark = compitem_getmark(view->pItems[row]);
        }
        ViewLeave();
    }
    return(bMark);
}

/*
 * set the mark state for a given row. This is only possible for compitem rows.
 * The mark set can be retrieved by view_getmarkstate or compitem_getmark.
 *
 * We return FALSE if the state could not be set - eg because the
 * row to set is not a compitem row.
 */
BOOL
view_setmarkstate(VIEW view, long row, BOOL bMark)
{
    BOOL bOK = FALSE;

    if (view != NULL) {
        ViewEnter();
        if ( (0 < row) && (0 <= view->rows) && (row < view->rows) && !view->bExpand) {
            compitem_setmark(view->pItems[row], bMark);
            bOK = TRUE;
        }
        ViewLeave();
    }
    return(bOK);
}


/* return a handle to the current compitem. in expand mode,
 * returns the handle to the compitem we are expanding. In outline
 * mode, returns the handle to the compitem for the given row, if valid,
 * or NULL otherwise. row is only used if not in expand mode.
 */
COMPITEM
view_getitem(VIEW view, long row)
{
    COMPITEM ci;

    if (view == NULL) {
        return(NULL);
    }

    ViewEnter();

    if (!view->bExpand) {
        if ((row >= 0) && (row < view->rows)) {
            ci = view->pItems[row];
        } else {
            ci = NULL;
        }
    } else {
        ci = view->ciSelect;
    }

    ViewLeave();
    return(ci);
}

/*
 * return TRUE if the current mapping is expanded mode
 */
BOOL
view_isexpanded(VIEW view)
{
    if (view == NULL) {
        return(FALSE);
    }
    return(view->bExpand);
}


/*
 * return a text string describing the view. This is NULL in outline mode,
 * or the tag text for the current compitem in expanded mode
 */
LPSTR
view_getcurrenttag(VIEW view)
{
    LPSTR str;

    if ((view == NULL) || (!view->bExpand)) {
        return(NULL);
    } else {
        ViewEnter();

        str = compitem_gettext_tag(view->ciSelect);

        ViewLeave();
        return(str);

    }
}


/* notify that CompItems have been added to the complist.
 *
 * rebuild the view (if in outline mode), and refresh the table. Use
 * the table message TM_APPEND if possible (if column widths have not
 * change). If we have to do TM_NEWLAYOUT, then ensure we scroll
 * back to the right row afterwards.
 *
 * This causes a Poll() to take place. We return TRUE if an abort is
 * pending - in this case, the caller should abandon the scan loop.
 *
 * WIN32: enter the critical section for this function since this can be
 * called from the worker thread while the UI thread is using the
 * view that we are about to change.
 *
 * We cannot ever call SendMessage from the
 * worker thread within CSView.  If there is conflict, it will hang.
 *
 */
BOOL
view_newitem(VIEW view)
{
    int maxtag, maxrest;
    int rownr;
    TableSelection Select;
    BOOL bSelect;
    COMPITEM ciTop = NULL;
    BOOL bRedraw = FALSE;
    BOOL bAppend = FALSE;

    // get the top row before remapping in case we need it
    /* find the row at the top of the window */
    rownr = (long) SendMessage(view->hwnd, TM_TOPROW, FALSE, 0);
    // also remember the selection
    bSelect = (BOOL) SendMessage(view->hwnd, TM_GETSELECTION, 0, (LPARAM) &Select);

    // *important*:no critsec over SendMessage
    ViewEnter();

    if ((view != NULL) &&
        !(view->bExpand) &&
        !(view->bExpanding)) {

        /* save some state about the present mapping */
        maxtag = view->maxtag;
        maxrest = view->maxrest;


        // remember the compitem this corresponds to
        if (view->pItems && (rownr >= 0) && (rownr < view->rows)) {
            ciTop = view->pItems[rownr];
        }


        // re-do the outline mapping, but don't tell the table class.
        // ask it to check for the visible row closest to ciTop in case
        // we need to refresh the display
        //
        // since we are holding the critsec, the redraw param
        // *must* be false.
        view_outline_opt(view, FALSE, ciTop, &rownr);

        /* have the column widths changed ? */
        if ((maxtag < view->maxtag) || (maxrest < view->maxrest)) {
            /* yes - need complete redraw */
            bRedraw = TRUE;
        } else {
            bAppend = TRUE;
        }
    }

    ViewLeave();


    if (bRedraw) {

        /* switch to new mapping */
        SendMessage(view->hwnd, TM_NEWLAYOUT, 0, (LPARAM) view);

        // go to the visible row closest to the old top row
        if ((rownr >= 0) && (rownr < view->rows)) {
            SendMessage(view->hwnd, TM_TOPROW, TRUE, rownr);
        }

        // select the old selection too (if the table class allowed
        // us to get it)
        if (bSelect) {
            SendMessage(view->hwnd, TM_SELECT,0, (LPARAM) &Select);
        }

    } else if (bAppend) {
        /* we can just append */

        /*
         * in the WIN32 multiple threads case, the mapping may have
         * changed since we released the critsec. however we are still
         * safe. The table will not allow us to reduce the number of
         * rows, so the worst that can happen is that the table will
         * think there are too many rows, and the table message handler
         * will handle this correctly (return null for the text).
         * The only visible effect is therefore that the scrollbar
         * position is wrong.
         */

        SendMessage(view->hwnd, TM_APPEND, view->rows, (LPARAM) view);
    }

    return(Poll());
}

/*
 * the view mapping options (eg outline_include, expand_mode) have changed -
 * re-do the mapping and then scroll back to the same position in the window
 * if possible.
 */
void
view_changeviewoptions(VIEW view)
{
    long row;
    int state, number;
    BOOL bRight;

    if (view == NULL) {
        return;
    }

    /* find what row we are currently on. Do this BEFORE we enter CSView */
    row = (long) SendMessage(view->hwnd, TM_TOPROW, FALSE, 0);

    ViewEnter();

    if (!view->bExpand) {


        // view_outline_opt allows us to find the first visible row
        // after a given COMPITEM. Do this to look for the old top-row
        // compitem, so that even if it is no longer visible, we can
        // still go to just after it.

        INT newrow = -1;
        if (row < view->rows) {

            COMPITEM ciTop = view->pItems[row];

            view_outline_opt(view, TRUE, ciTop, &newrow);
        } else {
            view_outline_opt(view, TRUE, NULL, NULL);
        }
        ViewLeave();

        // row now has the visible row that corresponds to
        // ciTop or where it would have been
        if ((newrow >=0) && (newrow < view->rows)) {
            SendMessage(view->hwnd, TM_TOPROW, TRUE, newrow);
        }
        return;
    }

    /* expanded mode */


    bRight = FALSE;  /* arbitrarily - avoid strange diagnostic */
    /* save the line number on one side (and remember which side) */
    if (row >= view->rows) {
        number = -1;
    } else {
        state = section_getstate(view->pLines[row].section);
        if ((state == STATE_MOVEDRIGHT) ||
            (state == STATE_RIGHTONLY)) {
            bRight = TRUE;
            number = view->pLines[row].nr_right;
        } else {
            bRight = FALSE;
            number = view->pLines[row].nr_left;
        }
    }

    /* make the new mapping */
    view_expand_item(view, view->ciSelect);

    /* things may happen now due to simultaneous scrolling from
     * two threads.  At least we won't deadlock.
     */
    /* find the nearest row in the new view */
    if (number >= 0) {

        ViewEnter();
        row = view_findrow(view, number, bRight);
        ViewLeave();

        /* scroll this row to top of window */
        if (row >= 0) {

            /* things may happen now due to simultaneous scrolling from
             * two threads.  At least we won't deadlock.
             */
            SendMessage(view->hwnd, TM_TOPROW, TRUE, row);
            return;
        }
    }
}

/* the compare options have changed - re-do the compare completely
 * and make the new mapping. Retain current position in the file.
 */
void
view_changediffoptions(VIEW view)
{
    int state, number;
    long row;
    BOOL bRight = FALSE;
    LIST li;
    COMPITEM ci;

    number = 0;
    if (view == NULL) {
        return;
    }

    /*
     * get current row before entering critsec.
     */
    row = (long) SendMessage(view->hwnd, TM_TOPROW, FALSE, 0);

    ViewEnter();

    /* find the current line number so we can go back to it
     * (only if we are in expanded mode
     */
    if (view->bExpand) {

        state = section_getstate(view->pLines[row].section);
        if ((state == STATE_MOVEDRIGHT) ||
            (state == STATE_SIMILARRIGHT) ||
            (state == STATE_RIGHTONLY)) {
            bRight = TRUE;
            number = view->pLines[row].nr_right;
        } else {
            bRight = FALSE;
            number = view->pLines[row].nr_left;
        }
    }

    /* to force a recompare using the new options, we must
     * tell each compitem to discard its current compare result.
     * we need to traverse the list of compitems calling this
     * for each compare.
     */
    li = complist_getitems(view->cl);

    for (ci = (COMPITEM) List_First(li); ci != NULL; ci = (COMPITEM) List_Next(ci)) {
        compitem_discardsections(ci);
    }

    if (!view->bExpand) {
        ViewLeave();

        // we are in outline mode. Refreshing the outline view
        // will pick up any tag and tag width changes
        view_outline(view);

        // now scroll to the previous position if still there
        if (row < view->rows) {
            SendMessage(view->hwnd, TM_TOPROW, TRUE, row);
        }

        return;
    }

    view_expand_item(view, view->ciSelect);

    /* find the nearest row in the new view */
    ViewEnter();
    row = view_findrow(view, number, bRight);
    ViewLeave();

    /* scroll this row to top of window */
    if (row >= 0) {
        SendMessage(view->hwnd, TM_TOPROW, TRUE, row);
    }
}


/* find the next changed - ie non-same - row in a given direction.
 * for outline mode we find the next STATE_DIFFER. for expand mode, we
 * find the next section
 */
long
view_findchange(VIEW view, long startrow, BOOL bForward)
{
    long i;

    if (view == NULL) {
        return(0);
    }

    if (view->rows <= 0) {
        return(-1);
    }

    ViewEnter();

    if (bForward) {

        if (startrow >= view->rows) {
            ViewLeave();
            return(-1);
        }

        if (!view->bExpand) {

            /* look for next compitem with an expandable state*/
            for (i = startrow; i < view->rows; i++) {
                if (compitem_getstate(view->pItems[i]) == STATE_DIFFER) {
                    ViewLeave();
                    return(i);
                }
            }
            /* none found */
            ViewLeave();
            return(-1);
        } else {
            /*
             * find the next line that matches, then go on to the
             * next line that does not match
             *
             */
            for (i= startrow; i < view->rows; i++) {
                if (section_getstate(view->pLines[i].section)
                    == STATE_SAME) {
                    break;
                }
            }
            for ( ; i < view->rows; i++) {
                if (section_getstate(view->pLines[i].section)
                    != STATE_SAME) {
                    ViewLeave();
                    return(i);
                }
            }

            ViewLeave();

            return(-1);
        }
    } else {
        /* same search backwards */
        if (startrow < 0) {
            ViewLeave();
            return(-1);
        }
        if (view->bExpand) {
            /* search backwards for first row that is not
             * changed (has state SAME). then carry on for
             * the next changed row.
             */
            for (i = startrow; i >= 0; i--) {
                if (section_getstate(view->pLines[i].section)
                    == STATE_SAME) {
                    break;
                }
            }
            for ( ; i >= 0; i--) {
                if (section_getstate(view->pLines[i].section)
                    != STATE_SAME) {
                    ViewLeave();
                    return(i);
                }
            }
            ViewLeave();
            return(-1);
        } else {
            for (i = startrow; i >= 0; i--) {
                if (compitem_getstate(view->pItems[i]) == STATE_DIFFER) {
                    ViewLeave();
                    return(i);
                }
            }
            ViewLeave();
            return(-1);
        }
    }
}


int view_getrowstate(VIEW view, long row)
{
    int state;

    if (view == NULL) {
        return(0);
    }

    if ((view->rows) <= 0 || (row >= view->rows) || (row <= 0) ) {
        return(STATE_SAME);
    }

    ViewEnter();

    state = section_getstate(view->pLines[row].section);

    ViewLeave();

    return state;
}

/*
 * the WIN32 multithread version can try view_expand and view_outline
 * (or view_newitem) at the same time. This is not correctly protected by
 * the critical section, since there are major restrictions about holding
 * critsecs when sending messages from the worker thread.
 *
 * To avoid contention, we call this function to notify that we are
 * starting expanding. view_newitem and ToOutline will return without
 * doing anything if this function has been called and view_expand has not
 * completed.
 */
void
view_expandstart(VIEW view)
{
    view->bExpanding = TRUE;
}

// are we in the middle of expansion ?
BOOL
view_expanding(VIEW view)
{
    return(view->bExpanding);

}



/* ---- internal functions ------------------------------------------ */


/* find the new row number for the line numbered 'number'
 * or the nearest line if possible. if bRight is true, number is
 * a right file number; otherwise it is a left file number.
 *
 * we must be in expand mode
 */
int
view_findrow(
            VIEW view,
            int number,
            BOOL bRight
            )
{
    int i;

    if (!view->bExpand) {
        return(0);
    }

    for (i = 0; i < view->rows; i++) {

        if (bRight) {
            if (view->pLines[i].nr_right == number) {

                /* found the exact number */
                return(i);

            } else if (view->pLines[i].nr_right > number) {

                /* passed our line -stop here */
                return(i);
            }
        } else {
            if (view->pLines[i].nr_left == number) {

                /* found the exact number */
                return(i);

            } else if (view->pLines[i].nr_left > number) {

                /* passed our line -stop here */
                return(i);
            }
        }
    }
    return(-1);
}

/* free memory associated with the expand mode or outline mode mappings
 * called whenever we rebuild the mapping, and on deletion
 */
void
view_freemappings(
                 VIEW view
                 )
{

    if (view->pLines) {

        HeapFree(GetProcessHeap(), NULL, view->pLines);
        view->pLines = NULL;
    } else if (view->pItems) {

        /* previous outline mapping array is still there - free it
         * before we build a new one
         */

        HeapFree(GetProcessHeap(), NULL, view->pItems);
        view->pItems = NULL;
    }
    view->rows = 0;  
}

/* build a view outline to map one row to a COMPITEM handle by traversing
 * the list of COMPITEMs obtained from our complist.
 * optionally tell the table class to redraw (if bRedraw), and if so,
 * scroll the new table to select the row that represents the
 * file we were expanding, if possible
 *
 * *important*: if you are holding the view critsec when you call this, you
 * must pass bRedraw as FALSE or you could deadlock
 *
 * if a COMPITEM ci is passed in, then return in *prow the row number that
 * corresponds to this item in the new view, or if not visible, the first
 * visible row after it (to retain current scroll position)
 */
void
view_outline_opt(
                VIEW view,
                BOOL bRedraw,
                COMPITEM ciFind,
                int * prow
                )
{
    int prev_row = -1;      /* the row nr of the previously-expanded row*/
    int i;                  /* nr of includable items */
    LIST li;
    COMPITEM ci;
    int state;
    TableSelection select;

    /*
     * check that view_setcomplist has already been called. if not,
     * nothing to do
     */
    if (view->cl == NULL) {
        return;
    }

    ViewEnter();

    /* clear the mode flag and free up memory associated with expand mode */
    view->bExpand = FALSE;
    view_freemappings(view);

    /* traverse the list of compitems counting up the number of
     * includable items
     */
    li = complist_getitems(view->cl);

    ci = (COMPITEM) List_First(li);
    for (i = 0; ci != NULL; ci = (COMPITEM) List_Next(ci)) {

        if ((ciFind != NULL) && (prow != NULL)) {
            if (ci == ciFind) {
                // now that we have found the requested item,
                // the next visible row is the one we want,
                // whether it is ci or a later one
                *prow = i;
            }
        }

        state = compitem_getstate(ci);

        if (((outline_include & INCLUDE_SAME) && (state == STATE_SAME)) ||
            ((outline_include & INCLUDE_DIFFER) && (state == STATE_DIFFER)) ||
            ((outline_include & INCLUDE_LEFTONLY) && (state == STATE_FILELEFTONLY)) ||
            ((outline_include & INCLUDE_RIGHTONLY) && (state == STATE_FILERIGHTONLY))) {
            if (!compitem_getmark(ci) || !hide_markedfiles) {
                i++;
            }
        }
    }


    /* allocate an array big enough for all of these */
    { /* DO NOT link in any storage with garbage pointers in it */
        COMPITEM * temp;
        temp = (COMPITEM *)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, i * sizeof(COMPITEM));
        if (temp == NULL)
        {
            return;
        }
        view->pItems = temp;
    }
    view->rows = i;

    /* keep track of the column widths */
    view->maxtag = 0;
    view->maxrest = 0;

    /* loop through again filling the array, and at the same time looking
     * out for the handle of the previously expanded item
     */
    ci = (COMPITEM) List_First(li);
    for (i = 0; ci != NULL; ci = (COMPITEM) List_Next(ci)) {

        state = compitem_getstate(ci);

        if (((outline_include & INCLUDE_SAME) && (state == STATE_SAME)) ||
            ((outline_include & INCLUDE_DIFFER) && (state == STATE_DIFFER)) ||
            ((outline_include & INCLUDE_LEFTONLY) && (state == STATE_FILELEFTONLY)) ||
            ((outline_include & INCLUDE_RIGHTONLY) && (state == STATE_FILERIGHTONLY))) {

            if (!compitem_getmark(ci) || !hide_markedfiles) {

                view->pItems[i] = ci;

                if (ci == view->ciSelect) {
                    prev_row = i;
                }

                /* check the column widths in characters */
                view->maxtag = max(view->maxtag,
                                   lstrlen(compitem_gettext_tag(ci)));
                view->maxrest = max(view->maxrest,
                                    lstrlen(compitem_gettext_result(ci)));


                i++;
            }

        }
    }
    ViewLeave();

    /* inform table of new layout of table - force refresh */
    if (bRedraw) {
        SendMessage(view->hwnd, TM_NEWLAYOUT, 0, (LPARAM) view);

        /* scroll to and highlight the row that represents the file
         * we were previously expanding
         */
        if (prev_row != -1) {
            select.startrow = prev_row;
            select.startcell = 0;
            select.nrows = 1;
            select.ncells = 1;
            SendMessage(view->hwnd, TM_SELECT, 0, (LPARAM) &select);
        }
    }
}


/* expand a view - given the handle to the compitem to expand.
 *
 * called from view_expand, and also to re-do an expanded view
 * after options change in view_changediffoptions and _changeviewoptions
 *
 * we get the composite section list from the compitem,
 * and pick out all the sections that are includable (according
 * to the global option expand_mode: we include all sections, or
 * just those in one side left or right). Once we know the count of rows,
 * allocate the mapping array: in each element of the array we keep
 * a handle to the section for that row (to get the state and hence the
 * tag text), and a handle to the line within that section (for the line text).
 *
 * We no longer insist on only expanding text files that differ - if the
 * compitem can give us a composite section list, we will map it.
 *
 * We need to be able to give a line number for a line, in either of
 * the original files according to which option is in force. Each section
 * can give us its base line number (number of first line in section) in
 * each of the two files or 0 if not present, and we track these here.
 *
 * MUST BE INSIDE CSView BEFORE CALLING HERE.
 */
BOOL
view_expand_item(
                VIEW view,
                COMPITEM ci
                )
{
    LIST li;
    SECTION sh;
    LINE line1, line2;
    int i, base_left, base_right, state;

    // We could be on a second thread trying to expand while it's
    // already going on.  That ain't clever!
    if (view->bExpandGuard) {
        Trace_Error(NULL, "Expansion in progress.  Please wait.", FALSE);
        ViewLeave();
        return FALSE;
    }

    // Ensure that the world knows that we are expanding
    // before we leave the critical section.
    // This is the only way into getcomposite.
    view->bExpandGuard = TRUE;

    // the compitem_getcomposite could take a long time
    // if the file is large and remote. We need to
    // release the critsec during this operation.

    ViewLeave();
    /* get the composite section list */
    li = compitem_getcomposite(ci);
    if (li == NULL) {
        view->bExpanding = FALSE;
        view->bExpandGuard = FALSE;
        return FALSE;
    }

    ViewEnter();

    /* remember the compitem we are expanding */
    view->ciSelect = ci;

    /* switch modes and free the current mapping
     *
     * NOTE: must do this AFTER the compitem_getcomposite,
     * since that can fail: if it fails it could put up a
     * message box, and that could cause a queued paint message
     * to be processed, which would cause us to use these mappings
     * and gpfault if they had been cleared first.
     */
    view->bExpand = TRUE;
    view->bExpanding = FALSE;
    view->bExpandGuard = FALSE;
    view_freemappings(view);


    /* loop through totalling the lines in sections
     * that we should include
     */
    view->rows = 0;
    for ( sh = (SECTION) List_First(li); sh != NULL;
        sh = (SECTION) List_Next(sh)) {

        state = section_getstate(sh);

        if (expand_mode == IDM_RONLY) {
            if ((state == STATE_LEFTONLY) ||
                (state == STATE_SIMILARLEFT) ||
                (state == STATE_MOVEDLEFT)) {
                continue;
            }
        } else if (expand_mode == IDM_LONLY) {
            if ((state == STATE_RIGHTONLY) ||
                (state == STATE_SIMILARRIGHT) ||
                (state == STATE_MOVEDRIGHT)) {
                continue;
            }
        }

        /* include all lines in this section
           if the section meets the include criteria */
        if ( ((state == STATE_SAME)         && (expand_include & INCLUDE_SAME))
             || ((state == STATE_LEFTONLY)     && (expand_include & INCLUDE_LEFTONLY))
             || ((state == STATE_RIGHTONLY)    && (expand_include & INCLUDE_RIGHTONLY))
             || ((state == STATE_MOVEDLEFT)    && (expand_include & INCLUDE_MOVEDLEFT))
             || ((state == STATE_MOVEDRIGHT)   && (expand_include & INCLUDE_MOVEDRIGHT))
             || ((state == STATE_SIMILARLEFT)  && (expand_include & INCLUDE_SIMILARLEFT))
             || ((state == STATE_SIMILARRIGHT) && (expand_include & INCLUDE_SIMILARRIGHT))) {
            view->rows += section_getlinecount(sh);
        }
    }

    /* allocate the memory for the mapping array */
    {    /* DO NOT chain in any storage with garbage pointers in it */
        PVIEWLINE temp;
        temp = (PVIEWLINE) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, view->rows * sizeof(VIEWLINE));
        if (temp == NULL) 
        {
            return FALSE;
        }
        view->pLines = temp;
    }

    /* loop through the sections again filling in the mapping array */
    i = 0;
    view->maxtag = 5;
    view->maxrest = 0;
    for (sh = (SECTION) List_First(li); sh != NULL;
        sh = (SECTION) List_Next(sh)) {

        state = section_getstate(sh);

        if (expand_mode == IDM_RONLY) {
            if ((state == STATE_LEFTONLY) ||
                (state == STATE_SIMILARLEFT) ||
                (state == STATE_MOVEDLEFT)) {
                continue;
            }
        } else if (expand_mode == IDM_LONLY) {
            if ((state == STATE_RIGHTONLY) ||
                (state == STATE_SIMILARRIGHT) ||
                (state == STATE_MOVEDRIGHT)) {
                continue;
            }
        }

        /* include all lines in this section
           if the section meets the include criteria */
        if ( ((state == STATE_SAME)         && (expand_include & INCLUDE_SAME))
             || ((state == STATE_LEFTONLY)     && (expand_include & INCLUDE_LEFTONLY))
             || ((state == STATE_RIGHTONLY)    && (expand_include & INCLUDE_RIGHTONLY))
             || ((state == STATE_MOVEDLEFT)    && (expand_include & INCLUDE_MOVEDLEFT))
             || ((state == STATE_MOVEDRIGHT)   && (expand_include & INCLUDE_MOVEDRIGHT))
             || ((state == STATE_SIMILARLEFT)  && (expand_include & INCLUDE_SIMILARLEFT))
             || ((state == STATE_SIMILARRIGHT) && (expand_include & INCLUDE_SIMILARRIGHT))) {

            /* find the base line number in each file */
            base_left = section_getleftbasenr(sh);
            base_right = section_getrightbasenr(sh);

            /* add each line in section to the view. section_getfirst()
             * returns us to a handle that is in a list. We can
             * call List_Next and will eventually get to the
             * line returned by section_getlast(). Sections always have
             * at least one line
             */
            line1 = section_getfirstline(sh);
            line2 = section_getlastline(sh);

            for (; line1 != NULL; line1 = (LINE) List_Next(line1)) {

                view->pLines[i].line = line1;
                view->pLines[i].section = sh;

                /* calculate the line number for this line by
                 * incrementing the base nr for this section.
                 * Note SIMILAR_RIGHT (or LEFT) lines DO have
                 * left (or right) numbers, but they are dummies.
                 */

                view->pLines[i].nr_left = base_left;
                if (state!=STATE_SIMILARRIGHT && base_left != 0) {
                    base_left++;
                }

                view->pLines[i].nr_right = base_right;
                if (state!=STATE_SIMILARLEFT && base_right != 0) {
                    base_right++;
                }

                /* increment index into view */
                i++;

                /* check the column widths */
                view->maxrest = max(view->maxrest,
                                    (line_gettabbedlength(line1, g_tabwidth)));

                /* end of section ? */
                if (line1 == line2) {
                    break;
                }
            }
        }
    }

    /* We must NOT hold a critical section here as SendMessage may hang */
    ViewLeave();

    /*inform table window of revised mapping */
    SendMessage(view->hwnd, TM_NEWLAYOUT, 0, (LPARAM) view);

    return(TRUE);
}


/*
 *  view_gethwnd
 *
 */
HWND
view_gethwnd(VIEW view)
{
    return(view) ? view->hwnd : NULL;
}


/*
 *  view_gototableline
 *
 */
void
view_gototableline(VIEW view, LONG iLine)
{
    if (view) 
    {
        const LONG cLines = view_getrowcount(view);
        if ( (iLine >= 0) && (iLine < cLines)) 
        {
            TableSelection select;
            memset(&select, 0, sizeof(TableSelection));
            select.startrow = iLine;
            select.nrows    = 1L;
            select.ncells   = 1L;

            SendMessage(view_gethwnd(view), TM_SELECT, 0, (LPARAM) &select);
        }
    }
}


typedef PUCHAR (*STRSUBFUNC)(PUCHAR, PUCHAR, PUCHAR*);
extern PUCHAR My_mbsistr(PUCHAR, PUCHAR, PUCHAR*);
extern PUCHAR My_mbsstr(PUCHAR, PUCHAR, PUCHAR*);

BOOL
view_findstring(VIEW view, LONG iCol, LPCSTR pszFind, BOOL fSearchDown, BOOL fMatchCase, BOOL fWholeWord)
{
    const LONG cRows = view_getrowcount(view);
    BOOL fFound = FALSE;

    if (cRows > 0) 
    {
        STRSUBFUNC pfnSub = (fMatchCase) ? My_mbsstr : My_mbsistr;
        const char *pszRow = NULL;
        const char *pszFound = NULL;
        char *pszEnd = NULL;
        LONG iEnd    = 0;
        LONG iRow    = 0;
        LONG nStep   = 0;
        LONG iWrapAt = 0;
        LONG iWrapTo = 0;

        if (fSearchDown) 
        {
            nStep = 1;
            iRow = selection + selection_nrows - 1;
            iWrapAt = cRows;
            iWrapTo = 0;
        }
        else 
        {
            nStep = -1;
            iRow = selection;
            iWrapAt = -1;
            iWrapTo = cRows - 1;
        }

        iRow += nStep;
        if (iRow < 0 || iRow >= cRows) 
        {
            iRow = iWrapTo;
        }

        iEnd = iRow;

        for (;;) 
        {
            pszRow = view_gettext(view, iRow, iCol);
            if (pszRow) 
            {
                pszEnd = NULL;
                pszFound = (const char*)pfnSub((PUCHAR)pszRow, (PUCHAR)pszFind, (PUCHAR*)&pszEnd);
                if (pszFound) 
                {
                    if (!fWholeWord) 
                    {
                        fFound = TRUE;
                    } 
                    else 
                    {
                        /* check end of string */
                        if (!pszEnd || !*pszEnd || (!IsDBCSLeadByte(*pszEnd) && !isalpha((UCHAR)*pszEnd) && !isdigit((UCHAR)*pszEnd))) 
                        {
                            /* check beginning of string */
                            if (pszFound == pszRow) 
                            {
                                fFound = TRUE;
                            } 
                            else 
                            {
                                const char *pchT = CharPrev(pszRow, pszFound);
                                if (!pchT || !*pchT || (!IsDBCSLeadByte(*pchT) && !isalpha((UCHAR)*pchT) && !isdigit((UCHAR)*pchT))) 
                                {
                                    fFound = TRUE;
                                }
                            }
                        }
                    }

                    if (fFound) 
                    {
                        view_gototableline(view, iRow);
                        break;
                    }
                }
            }

            iRow += nStep;
            if (iRow == iWrapAt) 
            {
                iRow = iWrapTo;
            }

            if (iRow == iEnd) 
            {
                break;
            }
        }
    }

    return fFound;
}
