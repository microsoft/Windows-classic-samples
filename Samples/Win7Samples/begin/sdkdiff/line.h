// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __LINE_H__
#define __LINE_H__
/*
 * line.h
 *
 * interface definition for the LINE data type.
 *
 * a LINE is a data type representing a line of ansi text along with a
 * line number.
 * a LINE can establish and maintain a link to another line known to be
 * the same.
 *
 * The LINE maintains a copy of the text, a line number associated with it,
 * a handle to another linked line, and a hashcode representing the line.
 *
 * Comparisons between LINEs take note of the global BOOL option flag
 * ignore_blanks. If this is TRUE, all spaces and tabs will be ignored during
 * the process of comparing two LINEs. If this option changes, each line
 * will have to be told by calling line_reset.
 */

/* a handle to a line looks like this. neither you nor your compiler needs
 * to know what the structure itself looks like.
 */
typedef struct fileline FAR * LINE;

/*
 * a LINE handle is a pointer to a struct fileline, defined here
 */
struct fileline
{

    UINT flags;     /* see below */

    LPSTR text;     /* null-terminated copy of line text */
    DWORD hash;     /* hashcode for line */
    LINE link;      /* handle for linked line */
    UINT linenr;    /* line number (any arbitrary value) */
    LPWSTR pwzText; /* null-terminated original unicode text */
};


/*
 * create a new line. space is allocated somewhere in the line object
 * for a null-terminated copy of the text passed in.
 *
 * the line passed in need not be null-terminated (the length of the line
 * is one argument) The copy made will be null-terminated.
 *
 * returns NULL if failed to create the line.
 *
 * The line number can be any value you wish to associate with the line.
 *
 * If the list parameter is non-null, the LINE data will be allocated at
 * the end of the list (a List_NewLast operation will be done). if list is
 * null, the memory will be allocated using HeapAlloc. This also affects
 * the behaviour of line_delete
 *
 * call line_delete to free up memory associated with this line.
 */
LINE line_new(LPSTR text, int linelength, LPWSTR pwzText, int cwchText, UINT linenr, LIST list);



/*
 * discard a line. free up all memory associated with it.
 *
 * if the line was allocated on a list (list argument to line_new was non-null),
 * the associated memory will be freed, but the LINE itself will not be.
 * Otherwise, the line will freed as well.
 */
void line_delete(LINE line);


/*
 * reset: discard existing hashcode and linked-line information, since
 * the ignore_blanks option has changed.
 */
void line_reset(LINE line);

/* test if two lines are alike (they have the same text). Takes note of
 * ignore_blanks in its comparison. Does not take any note of the line numbers
 * associated with each line. Returns TRUE if they are the same.
 */
BOOL line_compare(LINE line1, LINE line2);

/* try to link two lines together. Tests the lines to see if they are
 * alike, and if so, establishes a link between them and returns TRUE.
 * returns FALSE if they are not the same, or if either of them are already
 * linked (a LINE can only have one linked line, and it must be mutual).
 */
BOOL line_link(LINE line1, LINE line2);

/*
 * return a pointer to the text of this line. You get a pointer to the
 * LINE's own copy, so don't free it or extend it. If you modify it,
 * you will need to call line_reset before any line_compares or line_links.
 */
LPSTR line_gettext(LINE line);
LPWSTR line_gettextW(LINE line);

/*
 * return the length of the line in characters, expanding tabs. The tabstops
 * parameter determines the tabstop width to be used.
 *
 * This can be used to calculated
 * display space needed, but note that the line returned by
 * line_gettext() will still have any tabs unexpanded.
 */
int line_gettabbedlength(LINE line, int tabstops);

/*
 * each line has a hashcode associated with it. This is a 32-bit code
 * generated using the hashstring function. It is calculated only once
 * for each line (and thus calls to this function are efficient). To force
 * a recalculation, call line_reset.
 */
DWORD line_gethashcode(LINE line);

/* get the effective text length, ignoring blanks */
int line_gettextlen(LINE line);

/*
 * return the handle for the line that is linked to this line, or NULL if
 * there isn't one. Lines are either not linked at all, or are mutually
 * linked such that
 *              line_getlink( line_getlink(myline)) == myline;
 */
LINE line_getlink(LINE line);

/* return the line number associated with a line. This can be any 32-bit
 * number that was associated with the line when it was created
 */
UINT line_getlinenr(LINE line);

/* return TRUE iff line is blank.  NULL => return FALSE */
BOOL line_isblank(LINE line);
#endif
