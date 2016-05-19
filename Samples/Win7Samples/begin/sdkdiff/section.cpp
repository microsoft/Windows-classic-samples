// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * section.cpp
 *
 * manage sections of lines, and lists of sections.
 *
 * a section is a data type that represents a contiguous block of lines
 * of the same state (all unmatched, or all matched to a contiguous block of
 * lines). A section can link up matching lines within the section.
 *
 * section list functions can make and match lists of sections from lists of
 * lines, and create a composite list by combining sections from two lists
 * to create a list that 'best represents' the similarities and differences
 * between the two lists of lines.
 *
 * Assumptions: the lines passed in are on a list (can be traversed with
 *   List_Next() etc. Line numbering using the section_get*basenr()
 *   functions work only if lines are numbered sequentially in ascending order.
 *
 */

#include "precomp.h"

#include "tree.h"

#include "state.h"
#include "sdkdiff.h"
#include "wdiffrc.h"

#include "list.h"
#include "line.h"
#include "section.h"

#ifdef trace
extern BOOL bTrace;  /* in sdkdiff.cpp'.  Read only here */
#endif //trace

/*
 * a section handle (SECTION) is a pointer to one of these structures
 */
struct section {
    LINE first;             /* first line in section */
    LINE last;              /* last line in section */

    BOOL bDiscard;          /* true if not alloc-ed on list */

    SECTION link;           /* we match this section */
    SECTION correspond;     /* we correspond to this section, but
                             * don't match it
                             */

    int state;              /* compare state for section */

    int leftbase;           /* nr in original left list of first line*/
    int rightbase;          /* nr in original right list of first line*/
};

/* --- function prototypes ------------------------------------------*/

TREE section_makectree(SECTION sec);
BOOL section_expandanchor(SECTION sec1, LINE line1, SECTION sec2, LINE line2);



/*-- external functions ------------------------------------------------- */

/*
 * make a new section, given handles to a first and last line.
 *
 * a section must be at least one line long. The lines passed in must be
 * on a list in order.
 *
 * if the list parameter is non-null, we will allocate the section struct
 * on the list. otherwise we will alloc it using HeapAlloc. We remember
 * this in the bDiscard flag for section_delete, so that we only
 * hand back to HeapFree memory that we got.
 */
SECTION
section_new(LINE first, LINE last, LIST list)
{
    SECTION sec;

    /* alloc the sec and remember where we alloc-ed it */
    if (list) 
    {
        sec = (SECTION) List_NewLast(list, sizeof(struct section));
        if (!sec)
            return NULL;
        sec->bDiscard = TRUE;
    } 
    else 
    {
        sec = (SECTION) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct section));
        if (!sec)
            return NULL;
        sec->bDiscard = FALSE;
    }

    sec->first = first;
    sec->last = last;
    sec->link = NULL;
    sec->correspond = NULL;
    sec->state = 0;
    sec->leftbase = 1;
    sec->rightbase = 1;

    return(sec);
}

/*
 * discard a section. free all associated memory (not the line list).
 * free up the section itself if it was not alloc-ed on a list.
 */
void
section_delete(SECTION section)
{
    if (section->bDiscard) {
        HeapFree(GetProcessHeap(), NULL, section);
    }
}




/*
 * match up two sections: match all lines that
 * are unique and identical between the two sections.
 *
 * we use a tree of line handles, keyed by the line hash code. We use a
 * ctree, which keeps a count for multiple identical keys. This allows
 * us to rapidly find lines that are unique within this section.
 * We build two of these trees (one for each line list). For each line
 * that is unique in both trees, we attempt to link the lines.
 *
 * we also attempt to link the first and last line of the section.
 *
 * for each line we successfully link, we spread up and down from
 * this anchor point attempting to link lines.
 *
 * we return true if we linked any lines
 *
 * This routine may be called more than once on the same list of lines.
 * In matching lines we want to find unique, *unmatched* lines: so we only
 * insert lines into the ctree if they are currently unlinked.
 *
 * ReSynch means accept lines which occur AT LEAST once on each side rather than
 * EXACTLY once on each side.  In this case we associate the first occurrence
 * on one side with the first occurrence onthe other.
 *
 * We might ideally keep track of which sections match dubiously and use
 * this to resolve which sections we treat as matched and which as moved
 * when we are forced to make a choice.  Here we are only making links
 * between LINES, not sections, so it makes it a bit tricky.
 */
BOOL
section_match(SECTION sec1, SECTION sec2, BOOL ReSynch)
{
    TREE ctleft, ctright;
    LINE line, line2;
    BOOL bLinked = FALSE;


    if ((sec1 == NULL) || (sec2 == NULL)) {
        return(FALSE);
    }

    if ((sec1->first == NULL) || (sec2->first == NULL)) {
        return(FALSE);
    }
    /* ASSERT if first is non-null, so is last */

    /* attempt to link the first line of each file, and
     * if matched, expand as long as we keep matching
     */
    bLinked |= section_expandanchor(sec1, sec1->first, sec2, sec2->first);


    /* build a tree of lines, indexed by the line hashcode.
     * a ctree will hold only the first value of any given key, but
     * it will keep track of the number of items inserted on this key.
     * thus we can keep count of the number of times this line
     * (or at least this hashcode) appears.
     */
    ctleft = section_makectree(sec1);
    ctright = section_makectree(sec2);

    /* for each unlinked line in one list (doesn't matter which), find if
     * it appears once only in each list. if so, link, and expand
     * the link to link lines before and after the matching line
     * as long as they continue to match.
     *
     * If ReSynch is set then accept lines which occur more than once in
     * each file (this matches the first copy in each file of lines which
     * are the same, but are non unique.  Heuristic: if the lines are short
     * (8 chars or fewer) then don't accept them.  It can waste a lot
     * of time matching spurious {, } or blank lines.
     */
    for (line = sec1->first; line != NULL; line = (LINE)List_Next(line)) {

        if (  (line_getlink(line) == NULL)
              && (  ReSynch
                    ?  (  (ctree_getcount(ctleft, line_gethashcode(line)) >= 1)
                          && (ctree_getcount(ctright, line_gethashcode(line)) >= 1)
                          && (line_gettextlen(line) > 8)
                       )
                    :  (  (ctree_getcount(ctleft, line_gethashcode(line)) == 1)
                          && (ctree_getcount(ctright, line_gethashcode(line)) == 1)
                       )
                 )
           ) {

            /* lines match */
            line2 = * ((LINE FAR *)ctree_find(ctright,
                                              line_gethashcode(line)));
            bLinked |= section_expandanchor(sec1, line, sec2, line2);
        }

        if (line == sec1->last) {
            break;
        }
    }

    /* delete the ctrees */
    ctree_delete(ctleft);
    ctree_delete(ctright);


    /* attempt to link the last lines of each file and
     * expand upwards
     */
    bLinked |= section_expandanchor(sec1, sec1->last, sec2, sec2->last);

    return(bLinked);
} /* section_match */

/* -- accessor functions --------------*/

/*
 * get a handle to the first line in this section
 */
LINE
section_getfirstline(SECTION section)
{
    if (section == NULL) {
        return(NULL);
    }
    return(section->first);
}

/*
 * return a handle to the last line in a section
 */
LINE
section_getlastline(SECTION section)
{
    if (section == NULL) {
        return(NULL);
    }
    return(section->last);
}

/*
 * return a handle to the linked section, if any. A linked section
 * is a section whose lines all match the lines in this section
 */
SECTION
section_getlink(SECTION section)
{
    if (section == NULL) {
        return NULL;
    }
    return(section->link);
}

/*
 * return a handle to the corresponding section (a section which
 * corresponds in position to this one, but whose lines do not match).
 */
SECTION
section_getcorrespond(SECTION section)
{
    if (section == NULL) {
        return(NULL);
    }
    return(section->correspond);
}

/* get the state for this section */
int
section_getstate(SECTION section)
{
    if (section == NULL) {
        TRACE_ERROR("SECTION: null section in getstate call", FALSE);
        return(0);
    }
    return(section->state);
}

/* set the state for this section */
void
section_setstate(SECTION section, int state)
{
    section->state = state;
}

/* return the number of lines in the section. Here we assume that
 * lines in the section are number sequentially in ascending order, and we
 * simply look at the first and last line numbers.
 */
int
section_getlinecount(SECTION section)
{
    if (section->last==NULL && section->first==NULL)
        return 0;   /* the thing below would return 1! */
    if (section->first==NULL || section->last==NULL)
        TRACE_ERROR("SECTION: null first or last (but not both) in getlinecount call", FALSE);

    return(line_getlinenr(section->last) -
           line_getlinenr(section->first)) + 1;
}

/*
 * -- base line numbers --
 *
 * these functions only apply to sections in the composite list. When creating
 * a composite section, we record the line number of the first line in each
 * of the two sections we built it from. Thus we can calculate the
 * line number of any line in the section in either file it appeared in,
 * by adding the index of the line within the section to the base line
 * number.
 */
int
section_getleftbasenr(SECTION section)
{
    return(section->leftbase);
}

void
section_setleftbasenr(SECTION section, int base)
{
    section->leftbase = base;
}

int
section_getrightbasenr(SECTION section)
{
    return(section->rightbase);
}

void
section_setrightbasenr(SECTION section, int base)
{
    section->rightbase = base;
}


/* --- section list functions -------------------------------------*/

/* Theory of handling blank lines:
|                                      
|  If ignore_blanks is FALSE then a blank is just another character.
|  If it is TRUE then we will normally include unmatched blanks in whatever
|  section is surrounding them.  It would be nice if we could arrange to
|  never have a section that is only unmatched blanks, but (at least at
|  the start of the file) it can happen.  Tough.
|
|  Note that there are two DIFFERENT blank handling techniques:
|  In the first phase of the comparison when we are just trying to match up
|  lines, we skip over blank lines both forwards and backwards from an anchor.
|  When we are making real sections for display we only go forwards.
|  This results in a possible anomaly at the top of the whole file where
|  there could be some blanks which do not match and which can only possibly
|  be described as the start of a section.
|  For this reason, we label the sections with their state as early as possible
|  and go by that rather than by the presence or absence of link fields.
|  (It takes some scanning to find a link.  The first line in the section
|  could be a blank).
*/


/* return a LINE which is the last line in an unmatched section
   containing (probably starting with) Line.
   Note that it does not necessarily make progress.

   As noted above, even if blank lines are being ignored, we don't
   mind tagging them onto the end of an already unmatching section.
   This means we carry on until we find the first real link
*/
LINE FindEndOfUnmatched(LINE line)
{
    LINE next;

    for (; ; ) 
    {   next = (LINE)List_Next(line);
        if (next==NULL) return line;
        if (line_getlink(next)!=NULL) return line;
        line = next;
    }
} /* FindEndOfUnmatched */


/* An ignorable line is a blank line with no link and ignore_blanks set

   Given that line is initially not NULL and not ignorable:
   If line is the last line in the list then return NULL
   Else If ignore_blanks is FALSE then return the next line after line
   else return next line which has a link or which is non-blank.
   If there is no such line then return the last line in the list.

   Note that this does always make progress (at the cost of
   sometimes returning NULL).
*/
LINE NextNonIgnorable(LINE line)
{       LINE next;
    if (  line_getlink(line)==NULL
          && ! (ignore_blanks && line_isblank(line))
       )
        TRACE_ERROR("!!Bad call to NextNonIgnorable!!", FALSE);

    next = (LINE)List_Next(line);
    if (next==NULL) return NULL;
    for (; ; ) {
        line = next;
        if (  line_getlink(line)!=NULL) return line;
        if (! ignore_blanks)            return line;
        if (! line_isblank(line))       return line;
        next = (LINE)List_Next(line);
        if (next==NULL) return line;
    }
} /* NextNonIgnorable */


/* Given that line is either linked or an ignorable blank:
   Return a LINE which is the last line in a matched section
   containing (probably starting with) line.
   This could mean returning the line we were given.

   If the lines linked to are not consecutive then the section ends.
   If blanks are being ignored, then any blank line is deemed
   to match (even if it doesn't match).  In this case we need the
   links of the lines before and after the blanks to be consecutive
   in order to carry on.  There could be blank lines on either or both
   ends of the links.
*/
LINE FindEndOfMatched(LINE line)
{
    LINE next;              /* next non-ignored or linked line */
    LINE nextlink;          /* next in other file */

    /* The basic algorithm is to set up next and nextlink to point to
       candidate lines.  Examine them.  If they are good then step
       on to them, else return the line one before.
       There are confusion factors associated with the beginning and
       end of the file.
    */


    if (line==NULL) TRACE_ERROR("FindEndOfMatched called with NULL", FALSE);

    /* As a section (at least at the start of the file) might start
       with an ignored non-linked blank line, first step over any such
    */
    if ( line_getlink(line)==NULL && line_isblank(line) ) {
        next = NextNonIgnorable(line);

        /* There are unfortunately 6 cases to deal with
           * marks where next will be. * against eof means next==NULL
           blank(s) refer to ignorable unlinked blanks.
                  A         B        C        D        E        F
           line-> xxxxx     xxxxx    xxxxx    xxxxx    xxxxx    xxxxx
                 *unlinked  blanks  *linked   blanks  *eof     *blanks
                           *unlinked         *linked            eof

           next could be:

              null - case E => return line
              unlinked ignorable blank - case F => return that blank line
              unlinked other - cases A,B return prev(that unlinked line)
              linked - cases C,D continue from that linked line
        */
        if (next==NULL) return line;
        if (line_getlink(next)==NULL) {
            if (ignore_blanks && line_isblank(next)) {
                return next;
            }
            return(LINE)List_Prev(next);
        }

        line = next;
    }

    /* we have stepped over inital blanks and now do have a link */
    if (line_getlink(line)==NULL)
        TRACE_ERROR("!!FindEndOfMatched -- no link!!", FALSE);

    for ( ; ; ) {

        next = NextNonIgnorable(line);
        /* Same 6 cases - basically same again */
        if (next==NULL) return line;
        if (line_getlink(next)==NULL) {
            if (ignore_blanks && line_isblank(next)) {
                return next;
            }
            return(LINE)List_Prev(next);
        }

        nextlink = NextNonIgnorable(line_getlink(line));

        /* WEAK LOOP INVARIANT
           line is linked.
           next is the next non-ignorable line in this list after line.
           nextlink is the next non-ignorable line after link(line)
                                in the other list (could be NULL etc).
        */
        if (line_getlink(next) != nextlink)
            return(LINE)List_Prev(next);

        line = next;
    }
} /* FindEndOfMatched */


/*
 * make a list of sections by traversing a list of lines. consecutive
 * linked lines that are linked to consecutive lines are put in a single
 * section. blocks of unlinked lines are placed in a section.
 * If ignore_blanks is set then we first try to link them as normal.
 * but if they won't link then we just skip over them and keep them
 * in the same section.
 *
 * left must be set TRUE iff the list of lines is a left hand section.
 * returns a handle to a list of sections
 */
LIST
section_makelist(LIST linelist, BOOL left)
{
    LINE line1, line2;
    LIST sections;
    BOOL matched;
    SECTION sect;

    /* make an empty list of sections */
    sections = List_Create();

    /* for each line in the list */
    for ( line1=(LINE)List_First(linelist);  line1!=NULL;  line1 = (LINE)List_Next((LPVOID)line1)) {

        /* is it linked ? */

        if ( line_getlink(line1) != NULL
             || ( ignore_blanks && line_isblank(line1))
           ) {
            line2 = FindEndOfMatched(line1);
            matched = TRUE;
        } else {
            line2 = FindEndOfUnmatched(line1);
            matched = FALSE;
        }

        /* create the section and add to list */
        sect = section_new(line1, line2, sections);
        sect->state = (matched ? STATE_SAME
                       : left ? STATE_LEFTONLY
                       : STATE_RIGHTONLY
                      );

#ifdef trace
        {       char msg[80];
            StringCchPrintf( msg, 80
                             , "Created section: lines %d..%d of %s %s\r\n"
                             , line_getlinenr(line1)
                             , line_getlinenr(line2)
                             , (left ? "left" : "right")
                             , (matched ? "matching" : "non-matched")
                           );
            if (bTrace) Trace_File(msg);
        }
#endif

        /* advance to end of section (no-op if 1 line section) */
        line1 = line2;
    }

    return(sections);
} /* section_makelist */



/*
 * delete a list of sections
 *
 * sections have no dangling pointers, so all we do is delete the list
 */
void
section_deletelist(LIST sections)
{
    List_Destroy(&sections);
}

/* Return the first line in the range first..last
   which has a link.  Return last if none of them have a link.
   List_Next must lead from first to last eventually.
   It is legit for last to be NULL.
*/
LINE FindFirstWithLink(LINE first, LINE last)
{
    /* The strategy of including blanks on the ENDS of sections rather
       than the start of new sections will mean that this function
       usually strikes gold immediately.  A file with a leading
       blank section is its raison d'etre.
    */
#ifdef trace
    LINE hold = first;
#endif
    while (line_getlink(first)==NULL && first!=last)
        first = (LINE)List_Next(first);

    if (line_getlink(first)==NULL) {
#ifdef trace
        char msg[80];
        StringCchPrintf( msg, 80
                         , "???FindFirstWithLink found an unlinked section: %d..%d\r\n"
                         , line_getlinenr(hold)
                         , line_getlinenr(first)
                       );
        if (bTrace) Trace_File(msg);
#endif
    }
    return first;
} /* FindFirstWithLink */


/*
 * match up two lists of sections. establish links between sections
 * that match, and establish 'correspondence' between sections that
 * are in the same place, but don't match.
 *
 * for each pair of corresponding sections, we also call section_match
 * to try and link up more lines.
 *
 * we return TRUE if we made any more links between lines, or false
 * otherwise.
 *
 */
BOOL
section_matchlists(LIST secsleft, LIST secsright, BOOL bDups)
{
    BOOL bLinked = FALSE;
    SECTION sec1, sec2;

    /* match up linked sections - We know whether a section is
       supposed to link from its state, but we don't know what section
       it links to.  Also we can have sections which are defined to
       be matching but actually contain nothing but ignorable
       blank lines
    */

    /*  for each linked section try to find the section  linked to it. */
    for ( sec1=(SECTION)List_First(secsleft);  sec1!=NULL;  sec1 = (SECTION)List_Next((LPVOID)sec1)) {
        if (sec1->state==STATE_SAME) {
            LINE FirstWithLink = FindFirstWithLink(sec1->first, sec1->last);
#ifdef trace
            {       char msg[80];
                StringCchPrintf( msg, 80
                                 , "matchlists left matching section %d..%d \r\n"
                                 , line_getlinenr(sec1->first)
                                 , line_getlinenr(sec1->last)
                               );
                if (bTrace) Trace_File(msg);
            }
#endif
            for ( sec2=(SECTION)List_First(secsright);  sec2!=NULL;  sec2 = (SECTION)List_Next((LPVOID)sec2)) {
                if ( sec2->state==STATE_SAME
                     && line_getlink(FirstWithLink)
                     == FindFirstWithLink(sec2->first, sec2->last)) {
                    break;
                }
            }
            /* sec2 could be NULL if sec1 is all allowable blanks */
            if (sec2!=NULL) {
                sec1->link = sec2;
                sec2->link = sec1;
#ifdef trace
                {       char msg[80];
                    StringCchPrintf( msg, 80
                                     , "matchlists right matching section is %d..%d \r\n"
                                     , line_getlinenr(sec2->first)
                                     , line_getlinenr(sec2->last)
                                   );
                    if (bTrace) Trace_File(msg);
                }
#endif
            }
        }
    }

    /* go through all unmatched sections. Note that we need to complete
     * the link-up of matching sections before this, since we need
     * all the links in place for this to work.
     */

    for ( sec1=(SECTION)List_First(secsleft);  sec1!=NULL;  sec1 = (SECTION)List_Next((LPVOID)sec1)) {
        SECTION secTemp;

        if (sec1->state == STATE_SAME) {
            /* skip the linked sections */
            continue;
        }

        /* check that the previous and next sections, if
         * they exist, are linked. this should not fail since
         * two consecutive unlinked sections should be made into
         * one section
         */
        secTemp = (SECTION)List_Prev(sec1);
        if (secTemp && secTemp->state!= STATE_SAME) {
            TRACE_ERROR("consecutive unlinked sections", FALSE);
            continue;
        }
        secTemp = (SECTION)List_Next(sec1);
        if (secTemp && secTemp->state!= STATE_SAME) {
            TRACE_ERROR("consecutive unlinked sections.", FALSE);
            continue;
        }

        /* find the section that corresponds to this - that is, the
         * section following the section linked to our previous section.
         * we could be at beginning or end of list.
         */
        if (List_Prev(sec1) != NULL) {
            SECTION secOther;
            secOther = section_getlink((SECTION)List_Prev(sec1));
            if (secOther==NULL)
                continue;

            sec2 = (SECTION)List_Next(secOther);

            /* check this section is not linked */
            if ((sec2 == NULL) || (section_getlink(sec2) != NULL)) {
                continue;
            }

            /* check that the section after these are linked
             * to each other (or both are at end of list).
             */
            if (List_Next(sec1) != NULL) {

                if (section_getlink((SECTION)List_Next(sec1)) !=
                    List_Next(sec2)) {
                    continue;
                }
            } else {
                if (List_Next(sec2) != NULL) {
                    continue;
                }
            }

        } else if (List_Next(sec1) != NULL) {
            SECTION secOther;
            secOther = section_getlink((SECTION)List_Next(sec1));
            if (secOther==NULL)
                continue;

            sec2 = (SECTION)List_Prev(secOther);

            /* check this section is not linked */
            if ((sec2 == NULL) || (section_getlink(sec2) != NULL)) {
                continue;
            }

            /* check that the section before these are linked
             * to each other (or both are at start of list).
             */
            if (List_Prev(sec1) != NULL) {

                if (section_getlink((SECTION)List_Prev(sec1)) !=
                    List_Prev(sec2)) {
                    continue;
                }
            } else {
                if (List_Prev(sec2) != NULL) {
                    continue;
                }
            }
        } else {
            /* there must be at most one section in each
             * file, and they are unmatched. make these correspond.
             * (Though I don't think this would ever do anything except
             * waste time by trying to link that which has already failed)
             * 
             */
            sec2 = (SECTION)List_First(secsright);
        }


        /* make the correspondence links
         * we will want these for  zebra striping
         */
        if ((sec1 != NULL) && (sec2 != NULL)) {
            sec1->correspond = sec2;
            sec2->correspond = sec1;
        }

        /* attempt to link up lines */
        if (section_match(sec1, sec2, bDups)) {
            bLinked = TRUE;
        }
    }

    return(bLinked);
} /* section_matchlists */

/*
 * add a section to the composite list. called from makecomposites
 * to copy a section, add it to the composite list and set the state,
 * leftbase and rightbase.   Note that the state could be STATE_SAME
 * with a NULL section on the left.  May NOT call with STATE_SAME and
 * a NULL right section!
 *
 */
void
section_takesection(LIST compo, SECTION left, SECTION right, int state)
{
    SECTION newsec;
    SECTION sec = NULL;

    /* select which section is being output, and change the state
     * to indicate it has been output
     */
    switch (state) {
    case STATE_SAME:
        /* both the same. we mark both as output, and
         * take the right one.  It is possible that the
         * left one could be NULL (an ignorable blank section)
         */
        if (left!=NULL) left->state = STATE_MARKED;
        right->state = STATE_MARKED;
        sec = right;
        break;

    case STATE_LEFTONLY:
    case STATE_MOVEDLEFT:
        sec = left;
        left->state = STATE_MARKED;
        break;

    case STATE_RIGHTONLY:
    case STATE_MOVEDRIGHT:
        sec = right;
        right->state = STATE_MARKED;
        break;
        /* other cases e.g. SIMILARLEFT should not occur! */
    }


    /* create a new section on the list */
    newsec = section_new(sec->first, sec->last, compo);

    newsec->state = state;


    if (left != NULL) {
        newsec->leftbase = line_getlinenr(left->first);
    } else {
        newsec->leftbase = 0;
    }

    if (right != NULL) {
        newsec->rightbase = line_getlinenr(right->first);
    } else {
        newsec->rightbase = 0;
    }

} /* section_takesection */

/********************************************************************/
/* Zebra striping                                                   */
/********************************************************************/
#ifdef trace
static DWORD ZebraTicks = 0; /* total tick count spent hunting for zebras */
#endif

typedef BYTE bitmap[32];   /* 256 bits.  address the ith bit as m[i/8] & (1<<i%8) */

/* MakeMap
   Turn a string into a bitmap showing its character usage.  ANSI only!
   The bit in the map is set if the corresponding character occurs
   at least once anywhere in the string.
*/
void MakeMap(bitmap bm, LPSTR str)
{   int i;
    for (i=0; i<32; ++i)  bm[i] = 0;   /* clear it (yeah, could use memset) */
    for (i=0; str[i]!='\0'; ++i)    /* for each char */
    {   UINT v = ((UINT)str[i]) & 0x000000ff;   /* 0..256, No negatives! */
        bm[v/8] = BYTE(bm[v/8] | (1<<v%8));
    }
} /* MakeMap */

/* AndMaps
   Set the bit in res whenever the corresponding bit is set
   in both a and b
*/
void AndMaps(bitmap res, bitmap a, bitmap b)
{   int i;
    for (i=0; i<32; ++i)
        res[i] = (char)(a[i] & b[i]);
}

/* OrMaps
   Set the bit in res whenever the corresponding bit is set
   in either a or b
*/
void OrMaps(bitmap res, bitmap a, bitmap b)
{   int i;
    for (i=0; i<32; ++i)
        res[i] = (char)(a[i] | b[i]);
}

/* BitsInMap
   return the number of bits set on in bm
*/
int BitsInMap(bitmap bm)
{   int i;
    int j;
    int cb = 0;                  /* bit count */

    for (i=0; i<32; ++i)         /* for each byte in bm */
    {   for ( j=0; j<8; ++j)     /*    for each bit in byte */
        {   if (bm[i] & (1<<j))    /*    if the bit is set  */
                ++cb;
        }
    }
    return cb;
} /* BitsInMap */

/*------------------------------------------------------------------------------
|  Return TRUE iff left and right match fuzzily.
|  Fuzzy matching is implemented as follows.
|
|  Create a bitmap with one bit for each character in the ANSI set.
|  (This is 32 bytes long).  Clear it, and then OR in bits for each
|  character actually found in string one and then do the same again
|  for string two.
|  AND the two bit maps to get the characters which occur in both lines and
|  OR them to get the total number of different characters in either line.
|  Count the number of bits in each and work off the ratio common/total.
|  If this ratio exceeds a magic number of 3/4 then say they fuzzily match.
|
|  You've probably noticed that this is an application for
|           COUNTING THE BITS IN A BYTE!
 -------------------------------------------------------------------------------*/
BOOL LinesMatch(LINE Left, LINE Right)
{   LPSTR LText = line_gettext(Left);
    LPSTR RText = line_gettext(Right);
    bitmap bmLeft;
    bitmap bmRight;
    bitmap bmBoth;
    bitmap bmEither;
    int nBoth;
    int nEither;

    MakeMap(bmLeft, LText);
    MakeMap(bmRight, RText);
    AndMaps(bmBoth, bmLeft, bmRight);
    OrMaps(bmEither, bmLeft, bmRight);
    nBoth = BitsInMap(bmBoth);
    nEither = BitsInMap(bmEither);

    return(nBoth*4 >= nEither*3);    /* magic ratio is 3/4 */
} /* LinesMatch */


/*
 * add left and right sections to the comnposite list.  These sections
 * do not contain any matching lines, but do correspond, so here is
 * where we can do a fuzzy comparison and decide whether to zebra
 * them or not.
 *
 * If we retain the current structure of the composite list being a
 * list of sections rather than lines, then we might get rather a lot
 * of sections.  Do we care?  We need to build a new section here
 * for every line that we zebra, unpicking the old sections as we do it.
 *
 * At the moment it zebra stripes the lines as long as they keep
 * (fuzzily) matching.  Once there is a mismatch it takes the rest
 * as a block with no attempt to resynch until the next exact match.
 * We could be craftier, but I think we are already getting most of the
 * value from zebra striping.
 */
void TakeTwoSections(LIST compo, SECTION left, SECTION right)
{
    /* Algorithm:
       While there is something on each side,
          if the next lines match
          then take one line from each and make them into sections.
          else take everything that's left
       Finally take everything that's left (occurs if all matched until
                                            one side ran out)
    */

    SECTION NewL;
    SECTION NewR;
    LINE LLine = left->first;      /* first line left to process in left */
    LINE RLine = right->first;     /*   "    "    "   "   "      " right */

#ifdef trace
    DWORD Ticks = GetTickCount();  /* profiling */
#endif

    left->state = STATE_MARKED;
    right->state = STATE_MARKED;

    while (left!=NULL && right !=NULL) {

        if (LinesMatch(LLine, RLine)) {
            NewL = section_new(LLine, LLine, compo);
            NewL->state = STATE_SIMILARLEFT;
            NewL->leftbase = line_getlinenr(LLine);
            NewL->rightbase = line_getlinenr(RLine);

            NewR = section_new(RLine, RLine, compo);
            NewR->state = STATE_SIMILARRIGHT;
            NewR->leftbase = line_getlinenr(LLine);
            NewR->rightbase = line_getlinenr(RLine);

            if (LLine == left->last) left = NULL;  /* done */
            else LLine = (LINE)List_Next(LLine);
            if (RLine == right->last) right = (SECTION)NULL;  /* done */
            else RLine = (LINE)List_Next(RLine);
        } else {
            /* lost synch - take all the rest with no more zebra stuff */
            NewL = section_new(LLine, left->last, compo);
            NewL->state = STATE_LEFTONLY;
            NewL->leftbase = line_getlinenr(LLine);
            NewL->rightbase = 0;
            left = NULL;  /* done */

            NewR = section_new(RLine, right->last, compo);
            NewR->state = STATE_RIGHTONLY;
            NewR->rightbase = line_getlinenr(RLine);
            NewR->leftbase = 0;
            right = NULL;  /* done */
        }
    }

    if (left!=NULL) {
        NewL = section_new(LLine, left->last, compo);
        NewL->state = STATE_LEFTONLY;
        NewL->leftbase = line_getlinenr(LLine);
        NewL->rightbase = 0;
    }
    if (right!=NULL) {
        NewR = section_new(RLine, right->last, compo);
        NewR->state = STATE_RIGHTONLY;
        NewR->rightbase = line_getlinenr(RLine);
        NewR->leftbase = 0;
    }

#ifdef trace
    Ticks = GetTickCount()-Ticks;
    ZebraTicks += Ticks;
    {   char Msg[80];
        StringCchPrintf(Msg, 80, "Zebra time %d, total zebra time %d\n", Ticks, ZebraTicks);
        Trace_File(Msg);
    }
#endif

} /* TakeTwoSections */

/*
 * make a composite list of sections by traversing a list of sections.
 *
 * return a handle to a list of sections.
 *
 * during this, set state, leftbase and rightbase for sections.
 *
 * This function creates a list that corresponds to the 'best' view
 * of the differences between the two lists. We place sections from the
 * two lists into one composite list. sections that match each other are only
 * inserted once (from the right list). sections that match, but in different
 * positions in the two lists are inserted twice, once in each position, with
 * status to indicate this. unmatched sections are inserted in the correct
 * position.
 *
 * - take sections from the left list until the section is linked to one not
 *   already taken.
 * - then take sections from right until we find a section linked to one not
 *   already taken.
 * - if the two sections waiting are linked to each other, take them both
 *   (once- we take the right one and advance past both).
 *
 * - now we have to decide which to take in place and which to declare
 *   'moved'. consider the case where the only change is that the first line
 *   has been moved to the end. We should take the first line (as a move),
 *   then the bulk of the file (SAME) then the last line (as a move). Hence,
 *   in difficult cases, we take the smaller section first, to ensure that
 *   the larger section is taken as SAME.
 *
 *   to indicate which section has been output, we set the state field
 *   to STATE_MARKED once we have taken it.   States in left and right
 *   lists are of no further interest once we have built the composite.
 *
 *   Up to this point we have worked off the STATE of a section.  By now
 *   all the section links are in place, so we can use them too.
 */
LIST
section_makecomposite(LIST secsleft, LIST secsright)
{
    SECTION left, right;
    LIST compo;

    /* make an empty list for the composite */
    compo = List_Create();

    left = (SECTION)List_First(secsleft);
    right = (SECTION)List_First(secsright);

    while ( (left != NULL) || (right != NULL)) {

        if (left == NULL) {
            /* no more in left list - take right section */
            /* is it moved or just unmatched ? */
            if (right->link == NULL) {
                section_takesection(compo, NULL, right, STATE_RIGHTONLY);
                right = (SECTION)List_Next(right);
            } else {
                section_takesection(compo, right->link, right, STATE_MOVEDRIGHT);
                right = (SECTION)List_Next(right);
            }
        } else if (right == NULL) {
            /* right list empty - must be left next */

            /* is it moved or just unmatched ? */
            if (left->link == NULL) {
                section_takesection(compo, left, NULL, STATE_LEFTONLY);
                left = (SECTION)List_Next(left);
            } else {
                section_takesection(compo, left, left->link, STATE_MOVEDLEFT);
                left = (SECTION)List_Next(left);
            }

        } else if (left->state == STATE_LEFTONLY) {
            /* unlinked section on left */
            if (left->correspond==right) {
                TakeTwoSections( compo, left, left->correspond );
                left = (SECTION)List_Next(left);
                right = (SECTION)List_Next(right);
            } else {
                /* see picture below for when this might occur */
                section_takesection(compo, left, NULL, STATE_LEFTONLY);
                left = (SECTION)List_Next(left);
            }

        } else if (left->link==NULL) {
            /* This is an ignorable blank section on the left.
             * We ignore it. (We will take any such from the right)
             */
            left = (SECTION)List_Next(left);

        } else if (left->link->state==STATE_MARKED) {
            /* left is linked to section that is already taken*/
            section_takesection(compo, left, left->link, STATE_MOVEDLEFT);
            left = (SECTION)List_Next(left);

        } else if (right->link == NULL) {
            /* take unlinked section on right
             * Either unmatched or ignorable blanks
             */
            section_takesection(compo, NULL, right, right->state);
            right = (SECTION)List_Next(right);

        } else if (right->link->state==STATE_MARKED) {
            /* right is linked to section that's already taken */
            section_takesection(compo, right->link, right, STATE_MOVEDRIGHT);
            right = (SECTION)List_Next(right);

        } else if (left->link == right) {
            /* sections match */
            section_takesection(compo, left, right, STATE_SAME);
            right = (SECTION)List_Next(right);
            left = (SECTION)List_Next(left);
        } else {
            /* both sections linked to forward sections
             * Original idea was to go by section size and
             * take smallest first as a move so that larger one is
             * unchanged.  Counter example is test file blanks2
             *
             *   Files     Good version    Not so good
             *  A     A       A---A          A---A
             *  B-   -A         ->A RR    LL B->
             *  C | | A         ->A RR    LL C
             *  D  -+-B       B---B       LL D
             *  E   | c       C   c       LL E->
             *  A---  d       D   d          A---A
             *  A     E       E---E          A---A
             *  A     A       A->              ->B
             *  A             A->                c
             *                A---A              d
             *                A                ->E
             *                               A---A
             *                               A
             *
             * A better scheme looks at the distances marked
             * above as RR and LL and take as moved the block that results
             * in the shortest such distance.
             * Let LeftDist = linenr(right->link) - linenr(left)
             *       i.e. 5 in the example above  (the LL lines)
             * Let RightDist = linenr(left->link) - linenr(right)
             *       i.e. 3 in the example above  (the RR lines)
             *
             * if LeftDist > RightDist then treat right as moved
             * and take it first.
             */
            if ( line_getlinenr(right->link->first) - line_getlinenr(left->first)
                 > line_getlinenr(left->link->first)  - line_getlinenr(right->first)
               ) 
               {
                section_takesection(compo, right->link, right, STATE_MOVEDRIGHT);
                right = (SECTION)List_Next(right);
            } else {
                section_takesection(compo, left, left->link, STATE_MOVEDLEFT);
                left = (SECTION)List_Next(left);
            }

        }
    }

    return(compo);
} /* section_makecomposite */

typedef LINE (APIENTRY * MOVEPROC)(LINE);

/* Update PLINE by making it point to the first non-blank
   at-or-after from but not after limit.
   If they are all blank then make it point to limit
   If from is non-blank then leave it alone.
   Return TRUE iff PLINE was updated.
   It is legit for limit to be NULL (meaning end of file).
*/
BOOL AbsorbAnyBlanks(LINE * from, LINE limit, MOVEPROC Move)
{       BOOL progress = FALSE;

    while ( (from!=NULL)
            && (line_isblank(*from))
            && (*from!=limit)
          ) {
        *from = Move(*from);
        progress = TRUE;
    }
    return progress;
} /* AbsorbAnyBlanks */


/* given an anchor point (two lines that we think should match),
 * try to link them, and the lines above and below them for as long
 * as the lines can be linked (are the same, are unlinked).
 *
 * return TRUE if we make any links.
 *
 */
BOOL
section_expandanchor(SECTION sec1, LINE line1, SECTION sec2, LINE line2)
{
    /* when a line is matched we set bChanges.  If we notice some
     * blank lines, but do NOT link any new non-blank lines, we
     * do NOT set bChanges.  (If we did it would cause a closed
     * loop as they would get noticed again next time.  line_link
     * only returns TRUE if it is a NEW link).
     * At this stage we are only interested in making links, not in
     * the size of the section that results (that fun comes later).
     * therefore trailing blanks at the end of a section are not
     * interesting and we don't look for them.
     */
    BOOL bChanges = FALSE;
    LINE left, right;

    /* We handle the section limits by using a sentinel which is one
     * past the end of the section.  (If the section ends at the end
     * of the list then the sentinel is NULL).
     */
    LINE leftend, rightend;
    leftend = (LINE)List_Next(sec1->last);
    rightend = (LINE)List_Next(sec2->last);

#ifdef trace
    {  char msg[120];

        StringCchPrintf( msg, 120, "expanding anchor at %d<->%d in sections %d..%d <-> %d..%d\r\n"
                         , line_getlinenr(line1)
                         , line_getlinenr(line2)
                         , line_getlinenr(sec1->first)
                         , line_getlinenr(sec1->last)
                         , line_getlinenr(sec2->first)
                         , line_getlinenr(sec2->last)
                       );
        if (bTrace) Trace_File(msg);
    }
#endif

    /* null lines shall not match */
    if ((line1 == NULL) || (line2 == NULL)) {
        return(FALSE);
    }

    /* check all lines forward until fail to link (because null,
     * not matching, or already linked).
     * include the passed in anchor point since this has not
     * yet been linked.
     * If blanks are ignorable then skip over any number of whole
     * blank lines.
     */
    left = line1;
    right = line2;
    for (; ; ) {
        if (line_link(left, right) ) {

            bChanges = TRUE;
            left = (LINE)List_Next(left);
            right = (LINE)List_Next(right);
            if (left==leftend || right==rightend) break;
        }
        else if (ignore_blanks) {
            /* even though no match, maybe an ignorable blank? */

            BOOL moved = FALSE;
            moved |= AbsorbAnyBlanks(&left, leftend, (MOVEPROC)List_Next);
            moved |= AbsorbAnyBlanks(&right, rightend, (MOVEPROC)List_Next);
            if (!moved) break; /* it didn't match and we didn't move on */
            if (left==leftend || right==rightend) break;
        }
        else break;
    }

#ifdef trace
    {   char msg[120];
        StringCchPrintf( msg, 120, "marched forwards to (the line before) %d<->%d\r\n"
                         , (left==NULL ? 9999 : line_getlinenr(left))
                         , (right==NULL ? 9999 : line_getlinenr(right))
                       );
        if (bTrace) Trace_File(msg);
    }
#endif

    /* check all matches going backwards from anchor point
       but only if it was a real anchor  (could have been
       end-of-section/end-of-file and non-matching).
    */
    if (line_getlink(line1)==NULL) return bChanges;

    left = (LINE)List_Prev(line1);
    right = (LINE)List_Prev(line2);
    if (left==NULL || right==NULL) return bChanges;

    leftend = (LINE)List_Prev(sec1->first);
    rightend = (LINE)List_Prev(sec2->first);

    for (; ; ) {
        if (line_link(left, right)) {

            bChanges = TRUE;
            left = (LINE)List_Prev(left);
            right = (LINE)List_Prev(right);
            if (left == leftend || right == rightend) break;

        }
        else if (ignore_blanks) {
            /* even though no match, maybe an ignorable blank? */

            BOOL moved = FALSE;
            moved |= AbsorbAnyBlanks(&left, leftend, (MOVEPROC)List_Prev);
            moved |= AbsorbAnyBlanks(&right, rightend, (MOVEPROC)List_Prev);
            if (!moved) break; /* it didn't match and we didn't move on */
            if (left==leftend || right==rightend) break;

        }
        else break;
    }

#ifdef trace
    {   char msg[120];

        StringCchPrintf( msg, 120, "marched backwards to (the line after) %d<->%d\r\n"
                         , (left==NULL ? 0 : line_getlinenr(left))
                         , (right==NULL ? 0 : line_getlinenr(right))
                       );
        if (bTrace) Trace_File(msg);
    }
#endif

    return(bChanges);
}


/*
 * build a ctree from the lines in the section given
 *
 * remember that we are only interested in the lines that are
 * not already linked.
 *
 * the value we store in the tree is the handle of the line. the key
 * is the line hash code
 */
TREE
section_makectree(SECTION sec)
{
    TREE tree;
    LINE line;

    /* make an empty tree */
    tree = ctree_create();

    for (line = sec->first; line != NULL; line = (LINE)List_Next(line)) {
        if (line_getlink(line) == NULL) {
            ctree_update(tree, line_gethashcode(line),
                         &line, sizeof(LINE));
        }
        if (line == sec->last) {
            break;
        }
    }
    return(tree);
}


