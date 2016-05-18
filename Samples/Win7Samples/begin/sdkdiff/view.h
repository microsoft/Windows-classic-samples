// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __VIEW_H__
#define __VIEW_H__

/*
 *
 * VIEW
 *
 * provide a map between lines in the displayed table, and items in the
 * COMPLIST associated with this view.
 *
 * a view owns a complist: given the request for the text of a particular
 * line number in the displayed table, it will map it to either the
 * name/result of a CompItem in the COMPLIST (one of the files compared),
 * or to a line in the compare output (a line in one of the sections in
 * the composite section list for the selected CompItem).
 *
 * the view thus maintains a mode: OUTLINE mode is when one row is one COMPITEM,
 * and EXPAND mode is when one row is a line in the selected COMPITEM. Within
 * either of these modes, global option flags can select whether to show all
 * rows or only certain rows (eg only COMPITEMs that have a state other than
 * STATE_SAME, or only lines that are in the LEFT file).
 *
 * The view is given the handle to the table window. It will send messages
 * to the table window as appropriate when the view changes for any reason.
 *
 * The view owns the COMPLIST. When the view is deleted, the complist will
 * also be deleted.
 *
 * The table class is using this view. So to get rid of a view, call
 * view_close. This will notify the table window. Only call view_delete
 * when you have received the TQ_CLOSE notification indicating that the
 * table class has finished with this view.
 */




/* view.h includes the term COMPLIST: complist.h uses the term VIEW.
 * Alas MIPS doesn't allow duplicate definitions, even harmless ones,
 * so we need to play games.  Whoever declares it first does
 * the real declares and the second one gets no-ops.
 */
#ifndef INC_VIEW_COMPLIST
#define INC_VIEW_COMPLIST
typedef struct compitem FAR* COMPITEM;          /* handle to a compitem */
typedef struct view FAR * VIEW;                 /* handle to a VIEW     */
typedef struct complist FAR * COMPLIST;         /* handle to a complist */
#endif // INC_VIEW_COMPLIST

/* create a new view. It is told the handle for the associated
 * table window. We don't know yet the COMPLIST.
 */
VIEW view_new(HWND hwndTable);

/* tell the view the handle of its COMPLIST. This is an error if this
 * function has already been called for this view. This will init the
 * view to OUTLINE mode. We return FALSE if an error occured.
 */
BOOL view_setcomplist(VIEW view, COMPLIST cl);

/* get the handle of the COMPLIST for this view */
COMPLIST view_getcomplist(VIEW view);

/*
 * close a view. This causes the table window to be told to close
 * this view. When the table window has finished with the view, it will
 * send a TQ_CLOSE notification to its owner window. On receipt of
 * that, call view_delete
 */
void view_close(VIEW view);

/* delete a view and all data associated with it, including the COMPLIST.
 *
 * DON'T call this function except on receiving a TQ_CLOSE notification
 * from the table window. In other cases, call view_close to notify the
 * table window.
 */
void view_delete(VIEW view);

/*
 * each line has three columns - a line number, a tag and the main text.
 * this function returns the text for the given row in the
 * given view. The pointer is to a text string in the
 * view or in the complist somewhere - it should not be changed, and
 * may be overwritten by the next call to gettext.
 */

LPSTR view_gettext(VIEW view, long row, int col);
LPWSTR view_gettextW(VIEW view, long row, int col);

/*
 * return the line number that this line had in the original left or
 * right list. returns 0 if we are not in expanded mode, or if the
 * line was not in the original list. returns -(linenr) if the
 * line is MOVED and this is the other copy
 */
int view_getlinenr_left(VIEW view, long row);
int view_getlinenr_right(VIEW view, long row);


/* find the max width of the given column, in characters */
int view_getwidth(VIEW view, int col);


/* return the number of visible rows in this view */
long view_getrowcount(VIEW view);


/* get the state for this row. This is one of the STATE_* properties
 * defined in state.h, and is mapped to colour settings by the caller.
 */
int view_getstate(VIEW view, long row);


/* switch to expanded view of the given row. FALSE if row not
 * expandable or no such row. Switch the mapping so that each row
 * maps to one line in the composite section list for the given
 * compitem (the one selected by row in outline mode), and notify
 * the table window to redraw.
 * It is legal (and a no-op) to have rows==-1
 */
BOOL view_expand(VIEW view, long row);

/* return to outline mode. switch the mapping back so that each
 * row maps to one CompItem, and notify the table window so that it
 * is redrawn
 */
void view_outline(VIEW);

/* return a handle to the current CompItem. if the view is currently in
 * expand mode, this will return the handle for the CompItem that is
 * being expanded (regardless of the row parameter). If the mapping is
 * currently outline mode, the handle for the CompItem representing row
 * will be returned, or NULL if that is not valid
 */
COMPITEM view_getitem(VIEW view, long row);

/*
 * return TRUE if the current mapping is expanded mode
 */
BOOL view_isexpanded(VIEW view); 	

/* return a text string describing the current compitem. Only valid
 * if in expand mode. This will be normally the file name
 */
LPSTR view_getcurrenttag(VIEW view);


/* a CompItem has been added to the list. This will cause the
 * table to be notified of the change.
 *
 * This causes a Poll() to take place, and returns TRUE if an abort
 * has been requested. The caller should arrange to abort the current
 * scan operation.
 */
BOOL view_newitem(VIEW view);


/*
 * change view mode. the options affecting view selection have changed -
 * change the mapping if necessary, and redraw if it affects this mode
 *
 * retain current line if possible
 */
void view_changeviewoptions(VIEW view);

/* the compare options have changed - discard all compares and redo
 * as necessary.
 * retain current line if possible
 */
void view_changediffoptions(VIEW view);


/* find the next highlighted line in the given direction: forward if
 * bForward. returns the row number.
 */
long view_findchange(VIEW view, long startrow, BOOL bForward);

/* return the STATE_ value for the indicated row in the view.
 */
int view_getrowstate(VIEW view, long row);

/*
 * return the marked state for a given row. Only compitems can be marked,
 * so it will be FALSE unless it is a compitem on which view_setmark or
 * compitem_setmark have previously set the mark state to TRUE.
 */
BOOL view_getmarkstate(VIEW view, long row);

/*
 * set the mark state for a given row. This is only possible for compitem rows.
 * The mark set can be retrieved by view_getmarkstate or compitem_getmark.
 *
 * We return FALSE if the state could not be set - eg because the
 * row to set is not a compitem row.
 */
BOOL view_setmarkstate(VIEW view, long row, BOOL bMark);

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
void view_expandstart(VIEW);

// are we in the middle of expansion ?
BOOL view_expanding(VIEW);

HWND view_gethwnd(VIEW view);

void view_gototableline(VIEW view, LONG iLine);

BOOL view_findstring(VIEW view, LONG iCol, LPCSTR pszFind, BOOL fSearchDown, BOOL fMatchCase, BOOL fWholeWord);

#endif
