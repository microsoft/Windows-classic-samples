// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __STATE_H__
#define __STATE_H__

/*
 * sdkdiff - windows file and directory comparisons
 *
 * state.h
 *
 * definition of the results of comparisons for files and for lines
 * within files.
 *
 * These need to be globally declared so that the UI code in sdkdiff.cpp can
 * map states to the colour scheme (to correctly highlight changed lines).
 *
 * They apply to files (compitem_getstate() ) and to sections in the
 * composite list (section_getstate). All lines within a section have the
 * same state. The UI code will use the view_getstate() function to find the
 * state for a given line on the screen.
 *
 */

/* applies to both lines or files: they are the same */
#define STATE_SAME		1

/* applies to files.  Same size, date, time */
#define STATE_COMPARABLE	2

/* applies to files.  Different, but only in blanks
 * This state only turns up after the file has been expanded.
 */
#define STATE_SIMILAR		3

/* applies only to files */

/* - files differ (and can be expanded) */
#define STATE_DIFFER		4

/* they are only in the left or right tree */
#define STATE_FILELEFTONLY	5
#define STATE_FILERIGHTONLY	6


/* applies to lines only */

/* the line only exists in one of the lists */
#define STATE_LEFTONLY		7	/* line only in left file */
#define STATE_RIGHTONLY 	8	/* line only in right file */


/* the line is the same in both files, but in
 * different places (thus the line will appear twice in the composite list,
 * once with each of these two states
 */
#define STATE_MOVEDLEFT		9	/* this is the left file version */
#define STATE_MOVEDRIGHT	10	/* this is the right file version*/


#define STATE_SIMILARLEFT	11      /* this is the left file zebra version */
#define STATE_SIMILARRIGHT      12	/* this is the right file zebra version*/

/* In processing the sections to build the composite list, we need to
 * track which sections have been processed.  After this the left and
 * right lists of sections are of no further interest
 */
#define STATE_MARKED		99

#endif
