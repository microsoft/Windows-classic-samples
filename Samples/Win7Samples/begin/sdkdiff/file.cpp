// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * file.cpp
 *
 * an object representing a file and the lines of text it contains.
 *
 * a FILEDATA object is initialised with a DIRITEM handle from which it
 * can get a filename. It knows how to supply a list of LINE handles for the
 * lines of text in the file.
 *
 * The file is read into memory optionally on creation of the FILEDATA object:
 * otherwise, at the first call to file_getlinelist. It can be discarded
 * by calling file_discardlines: in this case, it will be re-read next time
 * file_getlinelist is called.
 *
 * calling file_reset will cause line_reset to be called for all lines
 * in the list. This clears any links and forces a recalc of line checksums.
 *
 * we allocate all memory using HeapAlloc
 *
 */

#include "precomp.h"
#include "sdkdiff.h"
#include "list.h"
#include "line.h"
#include "scandir.h"
#include "file.h"
#include "wdiffrc.h"

extern HANDLE hHeap;

/*
 * we return FILEDATA handles: these are pointers to a
 * filedata struct defined here.
 */
struct filedata {

    DIRITEM diritem;        /* handle to file name information */
    LIST lines;             /* NULL if lines not read in */

    BOOL fUnicode;
};


void file_readlines(FILEDATA fd);

/*-- external functions ---------------------------------------------- */

/*
 * create a new filedata, given the DIRITEM handle which will give us
 * the file name. If bRead is TRUE, read the file into memory.
 */
FILEDATA
file_new(DIRITEM fiName, BOOL bRead)
{
    FILEDATA fd;

    fd = (FILEDATA) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct filedata));
    if (fd == NULL) {
        return(NULL);
    }

    fd->diritem = fiName;
    fd->lines = NULL;

    if (bRead) {
        file_readlines(fd);
    }

    return(fd);
}

/*
 * return a handle to the DIRITEM used to create this FILEDATA
 */
DIRITEM
file_getdiritem(FILEDATA fd)
{
    if (fd == NULL) {
        return(NULL);
    }

    return(fd->diritem);
}


/*
 * delete a filedata and its associated list of lines. note that the diritem
 * is not deleted (this is owned by the DIRLIST, and will be deleted
 * when the DIRLIST is deleted)
 */
void
file_delete(FILEDATA fd)
{
    if (fd == NULL) {
        return;
    }

    /* throw away the line list, if there is one */
    file_discardlines(fd);

    HeapFree(GetProcessHeap(), NULL, fd);
}

/*
 * return a handle to a list of lines in this file. the items in the
 * list are LINE handles.
 *
 * the first call to this function will cause the file to be read into
 * memory if bRead was FALSE on the call to file_new, or if file_discardlines
 * has since been called.
 *
 * the list of lines returned should not be deleted except by calls to
 * file_delete or file_discardlines.
 */
LIST
file_getlinelist(FILEDATA fd)
{
    if (fd == NULL) {
        return NULL;
    }

    if (fd->lines == NULL) {
        file_readlines(fd);
    }
    return(fd->lines);
}


/*
 * discard the list of lines associated with a file. this will cause
 * the file to be re-read next time file_getlinelist is called.
 */
void
file_discardlines(FILEDATA fd)
{
    LINE line;

    if (fd == NULL) {
        return;
    }

    if (fd->lines != NULL) {

        /* clear each line to free any memory associated
         * with them, then discard the entire list
         */
		for( line=(LINE)List_First(fd->lines);  line!=NULL;  line = (LINE)List_Next((LPVOID)line)) {
            line_delete(line);
        }
        List_Destroy(&fd->lines);
    }

    /* this is probably done in List_Destroy, but better do it anyway*/
    fd->lines = NULL;
}


/*
 * force a reset of each line in the list. line_reset discards any
 * links between lines, and any hashcode information. This would be used if
 * the compare options or hashcode options have changed.
 */
void
file_reset(FILEDATA fd)
{
    LINE line;

    if (fd == NULL) {
        return;
    }

    if (fd->lines != NULL) {
		for( line=(LINE)List_First(fd->lines);  line!=NULL;  line =(LINE) List_Next((LPVOID)line)) {
            line_reset(line);
        }
    }
}

/*
 * return a checksum identical to one obtained through dir_getchecksum
 * This will recalculate it for local files.
 */
DWORD
file_checksum(FILEDATA fd)
{
    return(dir_getchecksum(fd->diritem));
}


/*
 * Retrieve the checksum that we have for this file, valid or not.
 * Indicate in bValid whether it is actually valid or not.
 * Do NOT recalculate it or make any new attempt to read the file!
 */
DWORD file_retrievechecksum(FILEDATA fd, BOOL * bValid)
{
    if (dir_validchecksum(fd->diritem)) {
        *bValid = TRUE;
        return dir_getchecksum(fd->diritem);
    } else {
        *bValid = FALSE;
        return 0;
    }
} /* file_retrievechecksum */


/* retrieve the filetime for the file */
FILETIME file_GetTime(FILEDATA fd)
{  return dir_GetFileTime(fd->diritem);
}

/* --- internal functions -------------------------------------------*/

/*
 * read the file into a list of lines.
 *
 * we use the buffered read functions to read a block at a time, and
 * return us a pointer to a line within the block. The line we are
 * pointed to is not null terminated. from this we do a line_new: this
 * will make a copy of the text (since we want to re-use the buffer), and
 * will null-terminate its copy.
 *
 * we also give each line a number, starting at one.
 */
void
file_readlines(FILEDATA fd)
{
    LPSTR textp;
    LPWSTR pwzText;
    int cwch;
    HANDLE fh;
    FILEBUFFER fbuf;
    int linelen;
    int linenr = 1;
    HCURSOR hcurs;

    hcurs = SetCursor(LoadCursor(NULL, IDC_WAIT));

    /* open the file */
    fh = dir_openfile(fd->diritem);

    if (fh == INVALID_HANDLE_VALUE) {
        TRACE_ERROR(LoadRcString(IDS_ERR_OPENING_FILE), FALSE);
        SetCursor(hcurs);
        return;
    }
    /* initialise the file buffering */
    fbuf = readfile_new(fh, &fd->fUnicode);

    if (fbuf)
    {
        /* make an empty list for the files */
        fd->lines = List_Create();

        while ( (textp = readfile_next(fbuf, &linelen, &pwzText, &cwch)) != NULL) {
            if (linelen>0) { /* readfile failure gives linelen==-1 */
                line_new(textp, linelen, pwzText, cwch, linenr++, fd->lines);
            } else {
                line_new("!! <unreadable> !!", 20, NULL, 0, linenr++,fd->lines);
                break;
            }


        }

        /* close filehandle and free buffer */
        readfile_delete(fbuf);
    }

    dir_closefile(fd->diritem, fh);

    SetCursor(hcurs);
}

BOOL
file_IsUnicode(FILEDATA fd)
{
    return fd->fUnicode;
}
