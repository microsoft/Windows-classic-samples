// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __COMPLIST_H__
#define __COMPLIST_H__

/*
 *
 * complist -  a list of CompItems
 *
 * build a list of CompItems from various sources. That is to say, build
 * two corresponding lists of filenames (DIRLISTs) and then build a list
 * of CompItems, with one CompItem for each pair of files whose names
 * match, and one CompItem for each unmatched name.
 *
 * when building the complist, a view handle can be given. If this is non-null,
 * the CompList will register with the view (calling view_setcomplist), and
 * will inform the view of each compitem added, during the build
 * process (so that the user can be kept up to date during a lengthy scan).
 *
 * We can return a handle to this list of CompItems on demand.
 *
 * The CompList owns the DIRLISTs and the list of CompItems. If you delete
 * the CompList, you delete all of these.
 */


/* view.h includes the term COMPLIST: we need to use the term VIEW.
 * Alas MIPS doesn't allow duplicate definitions, even harmless ones,
 * so we need to play games.  Whoever declares it first does
 * the real declares and the second one gets no-ops.
 * We don't care what a VIEW  is. Whatever you give us, we will
 * pass to the view_setcomplist function and view_newitem, and that is all.
 */
#ifndef INC_VIEW_COMPLIST
#define INC_VIEW_COMPLIST
typedef struct compitem FAR* COMPITEM;          /* handle to a compitem */
typedef struct view FAR * VIEW;                 /* handle to a VIEW     */
typedef struct complist FAR * COMPLIST;         /* handle to a complist */
#endif // INC_VIEW_COMPLIST


/*
 * build a complist by putting up two dialogs to allow the user to
 * select two files. This will build a Complist with one CompItem (even
 * if the names don't match).
 */
COMPLIST complist_filedialog(VIEW view);

/*
 * build a complist by putting up a dialog in which the user can specify
 * two directories. The directories will then be scanned and a CompItem
 * added to the list for each pair of names that match, and one for each
 * unmatched name
 */
COMPLIST complist_dirdialog(VIEW view);


/* build a complist from the two pathnames provided */
COMPLIST complist_args(LPSTR path1, LPSTR path2, VIEW view, BOOL fDeep);


/* build/append a complist from the two pathnames provided (either or both is
 * allowed to be null).
 */
void complist_append(COMPLIST *pcl, LPCSTR path1, LPCSTR path2, int *psequence);
BOOL complist_appendfinished(COMPLIST *pcl, LPCSTR pszLeft, LPCSTR pszRight, VIEW view);



/* delete a complist and all associated CompItems and DIRLISTs. Note this
 * does not delete any VIEW - the VIEW owns the COMPLIST and not the other
 * way around.
 */
void complist_delete(COMPLIST cl);

/*
 * get the handle to the list of COMPITEMs. The list continues to be
 * owned by the COMPLIST, so don't delete except by calling complist_delete.
 */
LIST complist_getitems(COMPLIST cl);


/* save the list of files as a series of lines to a file. query the user
 * for the name of the file to write, and the states of lines to be
 * included.
 *
 * if savename is not null, write the list out to savename using listopts.
 * otherwise, prompt by dialog for filename and options.
 */
void complist_savelist(COMPLIST cl, LPSTR savename, UINT listopts);


/*
 * get the description of this complist - a name in the form %s : %s with
 * the dir_getrootdescription() for each side.
 */
LPSTR complist_getdescription(COMPLIST cl);

/* free up memory allocated in a call to complist_getdescription() */
void complist_freedescription(COMPLIST cl, LPSTR path);


/*
 * copy files to a new directory newroot. if newroot is NULL, query the user
 * via a dialog to get the new dir name and options.
 *
 * options are either COPY_FROMLEFT or COPY_FROMRIGHT (indicating which
 * tree is to be the source of the files, plus any or all of
 * INCLUDE_SAME, INCLUDE_DIFFER and INCLUDE_LEFT (INCLUDE_LEFT
 * and INCLUDE_RIGHT are treated the same here since the COPY_FROM* option
 * indicates which side to copy from).
 */

/* option flags */
#define COPY_FROMLEFT       0x100           /* copy files from left tree */
#define COPY_FROMRIGHT      0x200           /* copy files from right tree */
#define COPY_HITREADONLY    0x400           /* overwrite read only files */

void complist_copyfiles(COMPLIST cl, LPSTR newroot, UINT options);


/* return time last operation took in milliseconds */
DWORD complist_querytime(void);


/*
 * complist_togglemark
 *
 * each compitem has a BOOL mark state. This function inverts the value of
 * this state for each compitem in the list.
 */
void complist_togglemark(COMPLIST cl);


/*
 * complist_itemcount
 *
 * return the number of items in the list
 */
UINT
complist_itemcount(COMPLIST cl);


/*
 * query the user for a pattern to match.
 * all compitems with this pattern in their tag string will be
 * marked (the mark state will be set to TRUE);
 *
 * returns TRUE if any states changed
 */
BOOL complist_markpattern(COMPLIST cl);
#endif
