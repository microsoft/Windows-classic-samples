// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __SECTION_H__
#define __SECTION_H__

/*
 *
 * section.h
 *
 *
 * manage sections of lines. These are contiguous blocks of lines that either
 * all match a contiguous block in another file, or are unmatched.
 *
 * a section can maintain a link to a corresponding section in another
 * file, and can establish links between matching lines in the two sections.
 *
 * a section also knows its compare state (defined in state.h). This says
 * whether it matches another section, or is matched but out of
 * sequence, or is unmatched. These are set during section_makecomposite.
 *
 * sections are held in LISTs. A list of sections can be built by functions
 * here that traverse a LIST of LINEs, or that traverse a list of
 * SECTIONs (to produce a composite list). In both cases, the lists used
 * are managed by the standard list package
 *
 *
 */

/* handle to a section */
typedef struct section FAR * SECTION;


/* make a section, given a first and last line. We return a handle to
 * a section. If the LIST parameter is non-null, we create the section
 * at the end of the list. if list is null, we allocate the memory ourselves.
 * This affects behaviour of section_delete (we only free memory we alloc
 * ourselves).
 *
 * The first and last lines must be on a LIST, with first coming before last.
 */
SECTION section_new(LINE first, LINE last, LIST list);


/* delete a section. free up all associated memory. does NOT delete the
 * associated list of lines.
 *
 *
 * If the section was allocated on a list, it will not be deleted here,
 * only the memory hanging off it will be freed.
 */
void section_delete(SECTION section);


/* match two sections: try to match as many lines as possible between
 * the two sections
 *
 * returns TRUE if any new links between LINEs were made, or FALSE if not.
 */
BOOL section_match(SECTION section1, SECTION section2, BOOL ReSynch);

/* return the handle to the first or last line in the section. If the section
 * is one line long, these will be the same. they should never be NULL
 */
LINE section_getfirstline(SECTION section);
LINE section_getlastline(SECTION section);

/* return the handle to the linked section, if any, or NULL if not linked */
SECTION section_getlink(SECTION section);

/* return a handle to a section that corresponds to this section, but
 * does not match. corresponding sections are found in the same
 * relative position of the file, but are not identical. At least
 * one of section_getlink and section_getcorrespond will return NULL for any
 * given section
 */
SECTION section_getcorrespond(SECTION section);

/* set the compare state for this section */
void section_setstate(SECTION section, int state);


/* return the compare state for this section. This will be 0 unless
 * set by section_getstate, or if the section was built by a call
 * to section_makecomposite.
 */
int section_getstate(SECTION section);


/* return a count of the number of lines in this section */
int section_getlinecount(SECTION section);


/* return the base line number for this section in the left or
 * right files. Base line number is the line number of the
 * first line in this section. Return 0 if the line was not in
 * the left(or right) file.
 *
 * This will only be set for sections created in section_makecomposites.
 *
 * Assumes that lines are numbered incrementally in ascending order.
 */
int section_getleftbasenr(SECTION section);
int section_getrightbasenr(SECTION section);



/*--  section list functions -------------------------------------*/


/* make a list of sections by traversing a list of lines. Contiguous
 * lines that are all linked to contiguous lines are put in the same section.
 * contiguous blocks of lines that are unmatched are put in the same section.
 * sections are kept in order in the list such that the first line of
 * the first section is the first line of the list of lines.
 * left must be TRUE iff the linelist represents a left hand file
 */
LIST section_makelist(LIST linelist, BOOL left);

/* free up a list of sections and all data associated with it */
void section_deletelist(LIST sections);


/* make a composite list of sections by traversing two lists of sections.
 *
 * section are placed in the same order: thus if sec1 is before sec2 in
 * list1, it will be before sec2 in the composite list. Sections that
 * match and are in the same order in both lists, are inserted only once
 * - only one of the two sections will be in the composite list, and the
 * section state will be set to SAME.
 * sections that match but are different places in the two original
 * lists will be inserted twice and the section state will be set to MOVED
 * (MOVED_LEFT and MOVED_RIGHT). Sections that are unmatched will be
 * inserted in order (relative to sections from the same list) with the
 * state set to ONLY_LEFT or ONLY_RIGHT
 */

LIST section_makecomposite(LIST secsleft, LIST secsright);


/* match up sections in the two lists. link sections that are the same,
 * (whose lines are linked), and make correspondence links for sections
 * that are in the same relative position, but not identical.
 * when making correspondence links, we attempt to link lines that
 * match between the two correponding sections. We return TRUE if at any
 * point we increase the number of links - this means that the section
 * lists will have to be rebuilt and rematched. This is *not* done here -
 * it must be done by caller.
 * bDups means allow matching the first occurrence of lines which are not unique.
 */
BOOL section_matchlists(LIST secsleft, LIST secsright, BOOL bDups);


#endif
