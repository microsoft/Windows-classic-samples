// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __COMPITEM_H__
#define __COMPITEM_H__
/*
 * CompItem
 *
 * a CompItem is a comparison between two files. It keeps track
 * of the two files. It knows how to compare two files, and knows the
 * results of the comparison: the end result is a list of SECTIONs
 * in each of the files, and a composite list of sections.
 *
 * One or other of the files may not exist.
 *
 * a compitem has a state - this indicates whether the two
 * files are the same, or different, or if only one file of that
 * name exists. This state is set on creation of the CompItem
 * and may be queried later.
 *
 */

#ifndef INC_VIEW_COMPLIST
#define INC_VIEW_COMPLIST
typedef struct compitem FAR* COMPITEM;          /* handle to a compitem */
typedef struct view FAR * VIEW;                 /* handle to a VIEW     */
typedef struct complist FAR * COMPLIST;         /* handle to a complist */
#endif // INC_VIEW_COMPLIST

/* build a new compitem from two files. Either (but not both) of the files
 * may be NULL.
 *
 * This operation may cause the files to be read in and compared.
 * In any case, this will be done by the time a call to one of
 * the query*sections functions completes.
 *
 * If the LIST parameter is not NULL, the item will be
 * appended to the list during initialisation - that is, the
 * compitem will do a List_NewLast operation and then initialise the
 * resulting object. If LIST is NULL, the compitem will allocate the
 * structure using some other memory allocation scheme. in either
 * case, the compitem handle will be returned. This also affects
 * behaviour of compitem_delete- we only free the compitem itself if
 * we allocated it ourself and not through List_NewLast.
 */
COMPITEM compitem_new(DIRITEM left, DIRITEM right, LIST list, BOOL fExact);


/* delete a compitem and free all associated data - INCLUDING deleting
 * the two FILEDATAs and all associated list of lines and sections.
 *
 * If the compitem was allocated on a list, it will not be freed, only
 * the memory hanging off it.
 */
void compitem_delete(COMPITEM item);


/* return a handle to a LIST of SECTIONs representing the compared file.
 * this call will cause the list to be created if it hasn't already been.
 *
 * returned handle will be NULL if either of the files is NULL.
 *
 * the list of sections can be traversed using the standard list functions.
 * the list you have a handle to is still owned by the compitem. to delete
 * it, call compitem_delete to delete the whole thing, or
 * compitem_discardsections to throw away all of the results of the compare
 */
LIST compitem_getcomposite(COMPITEM item);


/*
 * discard all compare data - throw away the composite section list and
 * any associated data (including the left and right section lists).
 * retains the two files. This is used either to free up memory when a
 * compitem is no longer being viewed, or to cause a new compare when
 * the global compare options flags (such as ignore_blanks) have changed.
 */
void compitem_discardsections(COMPITEM item);



/* return the handle to the list of sections in the left, right file.
 * These calls will cause the lists to be created if they are not already.
 *
 * the compitem still owns the list. traverse it with the standard list
 * functions, but don't change it or delete it.
 */
LIST compitem_getleftsections(COMPITEM item);
LIST compitem_getrightsections(COMPITEM item);


/* return the handle of the left or right file */
FILEDATA compitem_getleftfile(COMPITEM item);
FILEDATA compitem_getrightfile(COMPITEM item);


/* query the compare state of this compitem */
int compitem_getstate(COMPITEM item);

/* get a pointer to a text string describing the item (normally the
 * file name or filenames if different. The text pointed to should not
 * be changed or freed.
 */
LPSTR compitem_gettext_tag(COMPITEM item);

/* return a pointer to a text string describing the compare result - this
 * will be a text form of the item's state.
 * The text pointed to should not be changed or freed.
 */
LPSTR compitem_gettext_result(COMPITEM item);

/*
 * options for compitem_getfilename, indicating which name is desired
 */
#define CI_LEFT         1       /* name of left file */
#define CI_RIGHT        2       /* name of right file */
#define CI_COMP         3       /* name of composite file */

/*
 * return the name of the file associated with this compitem. The option
 * argument (one of CI_LEFT, CI_RIGHT, CI_COMP) indicates which file
 * is required.
 *
 * The file may be a temporary file, if the file option specifies a remote
 * file, or the composite file.
 *
 * call compitem_freefilename once the file is finished with.
 */
LPSTR compitem_getfilename(VIEW view, COMPITEM item, int option);

/*
 * free memory created by a call to compitem_getfilename. if a temporary
 * file was created, this may cause it to be deleted. The option argument must
 * be the same as passed to the original compitem_getfilename call.
 */
void compitem_freefilename(COMPITEM item, int option, LPSTR filename);


/* save the composite file
 *
 * if savename is not null, write the item out to savename using compopts.
 * otherwise, prompt by dialog for filename and options.
 */
LPSTR compitem_savecomp(VIEW view, COMPITEM ci, LPSTR savename, int listopts);


/*
 * worker function to write the actual composite file
 *
 * if savename is not null, write the list out to savename using compopts.
 * otherwise, prompt by dialog for filename and options.
 */
LPSTR compitem_writefile(VIEW view, COMPITEM ci, LPSTR savename, int compopts);


/*
 * set the mark state of a file. The only use for this is to retrieve it
 * later using compitem_getmark. The state is a bool.
 */
void compitem_setmark(COMPITEM item, BOOL bMark);


/*
 * return the mark state set by compitem_setmark
 */
BOOL compitem_getmark(COMPITEM item);

/* Tell compitem the paths to be used for autocopy.  Copies left to right */
void compitem_SetCopyPaths(LPSTR LeftPath, LPSTR RightPath);

/* Rescan the file, get new checksums etc */
void compitem_rescan(COMPITEM ci);
#endif
