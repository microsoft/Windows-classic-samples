// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * scandir.cpp
 *
 * build lists of filenames given a pathname.
 *
 * dir_buildlist takes a pathname and returns a handle. Subsequent
 * calls to dir_firstitem and dir_nextitem return handles to
 * items within the list, from which you can get the name of the
 * file (relative to the original pathname, or complete), and a checksum
 * and filesize.
 *
 * The list can be either built entirely during the build call, or
 * built one directory at a time as required by dir_nextitem calls. This
 * option affects only relative performance, and is taken as a
 * recommendation only (ie some of the time we will ignore the flag).
 *
 * the list is ordered alphabetically (case-insensitive using lstrcmpi).
 * within any one directory, we list filenames before going on
 * to subdirectory contents.
 *
 * All memory is allocated using HeapAlloc
 */

#include "precomp.h"

#include "list.h"
#include "scandir.h"

#include "sdkdiff.h"
#include "wdiffrc.h"

#ifdef trace
extern BOOL bTrace;  /* in sdkdiff.cpp.  Read only here */
#endif //trace

/*
 * The caller gets handles to two things: a DIRLIST, representing the
 * entire list of filenames, and a DIRITEM: one item within the list.
 *
 * from the DIRITEM he can get the filename relative to tree root
 * passed to dir_build*) - and also he can get to the next
 * DIRITEM (and back to the DIRLIST).
 *
 * We permit lazy building of the tree (usually so the caller can keep
 * the user-interface uptodate as we go along). In this case,
 * we need to store information about how far we have scanned and
 * what is next to do. We need to scan an entire directory at a time and then
 * sort it so we can return files in the correct order.
 *
 *
 * We scan an entire directory and store it in a DIRECT struct. This contains
 * a list of DIRITEMs for the files in the current directory, and a list of
 * DIRECTs for the subdirectories (possible un-scanned).
 *
 * dir_nextitem will use the list functions to get the next DIRITEM on the list.
 * When the end of the list is reached, it will use the backpointer back to the
 * DIRECT struct to find the next directory to scan.
 *
 */

/*
 * hold name and information about a given file (one ITEM in a DIRectory)
 * caller's DIRITEM handle is a pointer to one of these structures
 */
struct diritem {
    LPSTR name;             /* ptr to filename (final element only) */
    long size;              /* filesize */
    DWORD checksum;         /* checksum of file */
    DWORD attr;             /* file attributes */
    FILETIME ft_lastwrite;  /* last write time, set whenever size is set */
    BOOL sumvalid;          /* TRUE if checksum calculated */
    BOOL fileerror;         // true if some file error occurred
    struct direct * direct; /* containing directory */
    LPSTR localname;        /* name of temp copy of file */
    BOOL bLocalIsTemp;      /* true if localname is tempfile. not
                             * defined if localname is NULL
                             */
    int sequence;           /* sequence number, for dir_compsequencenumber */
};


/* DIRECT: hold state about directory and current position in list of filenames.
 */
typedef struct direct {
    LPSTR relname;          /* name of dir relative to DIRLIST root */
    DIRLIST head;           /* back ptr (to get fullname and server) */
    struct direct * parent; /* parent directory (NULL if above tree root)*/

    BOOL bScanned;          /* TRUE if scanned */
    LIST diritems;          /* list of DIRITEMs for files in cur. dir */
    LIST directs;           /* list of DIRECTs for child dirs */

    int pos;                /* where are we? begin, files, dirs */
    struct direct * curdir; /* subdir being scanned (ptr to list element)*/
} * DIRECT;

/* values for direct.pos */
#define DL_FILES        1       /* reading files from the diritems */
#define DL_DIRS         2       /* in the dirs: List_Next on curdir */


/*
 * the DIRLIST handle returned from a build function is in fact
 * a pointer to one of these. Although this is not built from a LIST object,
 * it behaves like a list to the caller.
 */
struct dirlist {

    char rootname[MAX_PATH];        /* name of root of tree */
    BOOL bFile;             /* TRUE if root of tree is file, not dir */
    BOOL bSum;              /* TRUE if checksums required */
    DIRECT dot;             /* dir  for '.' - for tree root dir */

    LPSTR pPattern;         /* wildcard pattern or NULL */
    LPSTR pDescription;     /* description */

    DIRLIST pOtherDirList;
};

extern BOOL bAbort;             /* from sdkdiff.cpp (read only here). */

/* file times are completely different under DOS and NT.
   On NT they are FILETIMEs with a 2 DWORD structure.
   Under DOS they are single long.  We emulate the NT
   thing under DOS by providing CompareFileTime and a
   definition of FILETIME
*/

/*-- forward declaration of internal functions ---------------------------*/

BOOL iswildpath(LPCSTR pszPath);
LPSTR dir_finalelem(LPSTR path);
void dir_cleardirect(DIRECT dir);
void dir_adddirect(DIRECT dir, LPSTR path);
BOOL dir_addfile(DIRECT dir, LPSTR path, DWORD size, FILETIME ft, DWORD attr, int *psequence);
void dir_scan(DIRECT dir, BOOL bRecurse);
BOOL dir_isvalidfile(LPSTR path);
BOOL dir_fileinit(DIRITEM pfile, DIRECT dir, LPSTR path, long size, FILETIME ft, DWORD attr, int *psequence);
void dir_dirinit(DIRECT dir, DIRLIST head, DIRECT parent, LPSTR name);
long dir_getpathsizeetc(LPSTR path, FILETIME FAR*ft, DWORD FAR*attr);
DIRITEM dir_findnextfile(DIRLIST dl, DIRECT curdir);

/* --- external functions ------------------------------------------------*/


/* ----- list initialisation/cleanup --------------------------------------*/


/*
 * build a list of filenames
 *
 * optionally build the list on demand, in which case we scan the
 * entire directory but don't recurse into subdirs until needed
 *
 * if bSum is true, checksum each file as we build the list. checksums can
 * be obtained from the DIRITEM (dir_getchecksum(DIRITEM)). If bSum is FALSE,
 * checksums will be calculated on request (the first call to dir_getchecksum
 * for a given DIRITEM).
 */

DIRLIST
dir_buildlist(
              LPSTR path,
              BOOL bSum,
              BOOL bOnDemand
              )
{
    DIRLIST dlOut = 0;
    DIRLIST dl = 0;
    BOOL bFile;
    char tmppath[MAX_PATH] = {0};
    LPSTR pstr;
    LPSTR pPat = NULL;
    LPSTR pTag = NULL;
	HRESULT hr;

    /*
     * copy the path so we can modify it
     */
    hr = StringCchCatN(tmppath, MAX_PATH, path, sizeof(tmppath)-1);
    if (FAILED(hr))
    {
        goto LError;
    }

    /* look for wildcards and separate out pattern if there */
    if (My_mbschr(tmppath, '*') || My_mbschr(tmppath, '?'))
    {
        pstr = dir_finalelem(tmppath);
        pPat = (char*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlen(pstr) +1);
        if (pPat == NULL)
			goto LError;
		hr = StringCchCopy(pPat, (lstrlen(pstr)+1), pstr);
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_COPY);
			goto LError;
		}
        *pstr = '\0';
    }

    /* we may have reduced the path to nothing - replace with . if so */
    if (lstrlen(tmppath) == 0)
    {
        hr = StringCchCopy(tmppath, MAX_PATH, ".");
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_COPY);
			goto LError;
		}
    }
    else
    {
        /*
         * remove the trailing slash if unnecessary (\, c:\ need it)
         */
        pstr = &tmppath[lstrlen(tmppath) -1];
        if ((*pstr == '\\') && (pstr > tmppath) && (pstr[-1] != ':')
            && !IsDBCSLeadByte((BYTE)*(pstr-1)))
        {
            *pstr = '\0';
        }
    }


    /* check if the path is valid */
    if ((pTag && !iswildpath(tmppath)) || (tmppath[0] == '/' && tmppath[1] == '/'))
    {
        bFile = TRUE;
    }
    else if (dir_isvaliddir(tmppath))
    {
        bFile = FALSE;
    }
    else if (dir_isvalidfile(tmppath))
    {
        bFile = TRUE;
    }
    else
    {
        /* not valid */
        goto LError;
    }


    /* alloc and init the DIRLIST head */

    dl = (DIRLIST) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dirlist));
    if (dl == NULL)
    {
		goto LError;
    }
	dl->pOtherDirList = NULL;

    /* convert the pathname to an absolute path */
    if (NULL == _fullpath(dl->rootname, tmppath, sizeof(dl->rootname)))
    {
        goto LError;
    }

    dl->bSum = bSum;
    dl->bSum = FALSE;  // to speed things up. even if we do want checksums,
                       // let's get them on demand, not right now.
    dl->bFile = bFile;

    if (pPat)
    {
        dl->pPattern = pPat;
        pPat = 0;
    }


    /* make a '.' directory for the tree root directory -
     * all files and subdirs will be listed from here
     */
    {    /* Do NOT chain on anything with garbage pointers in it */
        DIRECT temp;
        temp = (DIRECT) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct direct));
        if (temp == NULL)
			return NULL;
		dl->dot = temp;
    }

    dir_dirinit(dl->dot, dl, NULL, ".");

    /* were we given a file or a directory ? */
    if (bFile)
    {
        /* its a file. create a single file entry
         * and set the state accordingly
         */
        long fsize;
        FILETIME ft;
        DWORD attr;
        fsize = dir_getpathsizeetc(tmppath, &ft, &attr);

        dl->dot->bScanned = TRUE;

		if (!dir_addfile(dl->dot, dir_finalelem(tmppath), fsize, ft, attr, 0))
            goto LError;
    }
    else
    {
        /* scan the root directory and return. if we are asked
         * to scan the whole thing, this will cause a recursive
         * scan all the way down the tree
         */
        dir_scan(dl->dot, (!bOnDemand) );
    }

    dlOut = dl;
    dl = 0;

LError:
    if (pTag)
        HeapFree(GetProcessHeap(), NULL,  pTag);
    if (pPat)
        HeapFree(GetProcessHeap(), NULL, pPat);
    dir_delete(dl);
    return dlOut;
} /* dir_buildlist */

/*
 * build/append a list of filenames
 *
 * if bSum is true, checksum each file as we build the list. checksums can
 * be obtained from the DIRITEM (dir_getchecksum(DIRITEM)). If bSum is FALSE,
 * checksums will be calculated on request (the first call to dir_getchecksum
 * for a given DIRITEM).
 */

BOOL
dir_appendlist(
               DIRLIST *pdl,
               LPCSTR path,
               BOOL bSum,
               int *psequence
               )
{
    DIRLIST dl;
    BOOL bFile=FALSE;
    char tmppath[MAX_PATH];
    LPSTR pstr;
    LPSTR pTag = NULL;
    BOOL fSuccess = FALSE;
	HRESULT hr;

    if (path)
    {
        // copy the path so we can modify it
        hr = StringCchCopy(tmppath, MAX_PATH, path);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY); 

		// remove the trailing slash if unnecessary (\, c:\ need it)
        pstr = &tmppath[lstrlen(tmppath) -1];
        if ((*pstr == '\\') && (pstr > tmppath) && (pstr[-1] != ':')
            && !IsDBCSLeadByte((BYTE)*(pstr-1)))
        {
            *pstr = '\0';
        }


        /* check if the path is valid */
        if ((pTag && !iswildpath(tmppath))|| (tmppath[0] == '/' && tmppath[1] == '/'))
        {
            // assume valid
            bFile = TRUE;
        }
        else if (dir_isvaliddir(tmppath))
        {
            bFile = FALSE;
        }
        else if (dir_isvalidfile(tmppath))
        {
            bFile = TRUE;
        }
        else
        {
            /* not valid */
            goto LError;
        }
    }

    if (!*pdl)
    {
        DIRECT temp;

        /* alloc and init the DIRLIST head */
        *pdl = (DIRLIST) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct dirlist));
        if (*pdl == NULL)
			goto LError;
		(*pdl)->pOtherDirList = NULL;

        (*pdl)->bSum = bSum;
        (*pdl)->bSum = FALSE;   // to speed things up. even if we do want
                                // checksums, let's get them on demand, not
                                // right now.
        (*pdl)->bFile = FALSE;

        /* make a null directory for the tree root directory -
         * all files and subdirs will be listed from here
         */
        /* Do NOT chain on anything with garbage pointers in it */
        temp = (DIRECT) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct direct));
        if (temp == NULL)
			goto LError;
		(*pdl)->dot = temp;

        dir_dirinit((*pdl)->dot, (*pdl), NULL, "");
        (*pdl)->dot->relname[0] = 0;
        (*pdl)->dot->bScanned = TRUE;
    }
    dl = *pdl;

    if (path)
    {
        /* were we given a file or a directory ? */
        if (bFile)
        {
            /* its a file. create a single file entry
             * and set the state accordingly
             */
            long fsize;
            FILETIME ft;
            DWORD attr;

            fsize = dir_getpathsizeetc(tmppath, &ft, &attr);

			if (!dir_addfile(dl->dot, tmppath, fsize, ft, attr, psequence))
                goto LError;
        }
    }

    fSuccess = TRUE;

LError:
    if (pTag)
        HeapFree(GetProcessHeap(), NULL, pTag);
    return fSuccess;
} /* dir_appendlist */

void
dir_setotherdirlist(
                    DIRLIST dl,
                    DIRLIST otherdl
                    )
{
    dl->pOtherDirList = otherdl;
}

/* free up the DIRLIST and all associated memory */
void
dir_delete(
           DIRLIST dl
           )
{
    if (dl == NULL) {
        return;
    }

    dir_cleardirect(dl->dot);
    HeapFree(GetProcessHeap(), NULL, dl->dot);

    if (dl->pPattern) {
        HeapFree(GetProcessHeap(), NULL, dl->pPattern);
    }

    HeapFree(GetProcessHeap(), NULL,  dl);
}

/* ----- DIRLIST functions ------------------------------------------------*/

/* was the original build request a file or a directory ? */
BOOL
dir_isfile(
           DIRLIST dl
           )
{
    if (dl == NULL) {
        return(FALSE);
    }

    return(dl->bFile);
}


/* return the first file in the list, or NULL if no files found.
 * returns a DIRITEM. This can be used to get filename, size and chcksum.
 * if there are no files in the root, we recurse down until we find a file
 */
DIRITEM
dir_firstitem(
              DIRLIST dl
              )
{
    if (dl == NULL) {
        return(NULL);
    }

    /*
     * reset the state to indicate that no files have been read yet
     */
    dl->dot->pos = DL_FILES;
    dl->dot->curdir = NULL;

    /* now get the next filename */
    return(dir_findnextfile(dl, dl->dot));
} /* dir_firstitem */


/*
 * get the next filename after the one given.
 *
 * The List_Next function can give us the next element on the list of files.
 * If this is null, we need to go back to the DIRECT and find the
 * next list of files to traverse (in the next subdir).
 *
 * after scanning all the subdirs, return to the parent to scan further
 * dirs that are peers of this, if there are any. If we have reached the end of
 * the tree (no more dirs in dl->dot to scan), return NULL.
 *
 * Don't recurse to lower levels unless fDeep is TRUE
 */
DIRITEM
dir_nextitem(
             DIRLIST dl,
             DIRITEM cur,
             BOOL fDeep
             )
{
    DIRITEM next;

    if ((dl == NULL) || (cur == NULL)) {
        TRACE_ERROR("DIR: null arguments to dir_nextitem", FALSE);
        return(NULL);
    }

    if (bAbort) return NULL;  /* user requested abort */

    /* local list */

    if ( (next = (DIRITEM)List_Next(cur)) != NULL) {
        /* there was another file on this list */
        return(next);
    }
    if (!fDeep) return NULL;

    /* get the head of the next list of filenames from the directory */
    cur->direct->pos = DL_DIRS;
    cur->direct->curdir = NULL;
    return(dir_findnextfile(dl, cur->direct));
} /* dir_nextitem */

DIRITEM
dir_findnextfile(
                 DIRLIST dl,
                 DIRECT curdir
                 )
{
    DIRITEM curfile;

    if (bAbort) return NULL;  /* user requested abort */

    if ((dl == NULL) || (curdir == NULL)) {
        return(NULL);
    }

    /* scan the subdir if necessary */
    if (!curdir->bScanned) {
        dir_scan(curdir, FALSE);
    }

    /* have we already read the files in this directory ? */
    if (curdir->pos == DL_FILES) {
        /* no - return head of file list */
        curfile = (DIRITEM) List_First(curdir->diritems);
        if (curfile != NULL) {
            return(curfile);
        }

        /* no more files - try the subdirs */
        curdir->pos = DL_DIRS;
    }

    /* try the next subdir on the list, if any */
    /* is this the first or the next */
    if (curdir->curdir == NULL) {
        curdir->curdir = (DIRECT) List_First(curdir->directs);
    } else {
        curdir->curdir = (DIRECT) List_Next(curdir->curdir);
    }
    /* did we find a subdir ? */
    if (curdir->curdir == NULL) {

        /* no more dirs - go back to parent if there is one */
        if (curdir->parent == NULL) {
            /* no parent - we have exhausted the tree */
            return(NULL);
        }

        /* reset parent state to indicate this is the current
         * directory - so that next gets the next after this.
         * this ensures that multiple callers of dir_nextitem()
         * to the same tree work.
         */
        curdir->parent->pos = DL_DIRS;
        curdir->parent->curdir = curdir;

        return(dir_findnextfile(dl, curdir->parent));
    }

    /* there is a next directory - set it to the
     * beginning and get the first file from it
     */
    curdir->curdir->pos = DL_FILES;
    curdir->curdir->curdir = NULL;
    return(dir_findnextfile(dl, curdir->curdir));

} /* dir_findnextfile */


/*
 * get a description of this DIRLIST - this is essentially the
 * rootname with any wildcard specifier at the end. 
 *
 * NOTE that this is not a valid path to the tree root - for that you
 * need dir_getrootpath().
 */
LPSTR
dir_getrootdescription(
                       DIRLIST dl
                       )
{
    LPSTR pname;
	HRESULT hr;

    // allow enough space for \\servername! + MAX_PATH
    pname = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH + 15);
    if (pname == NULL) {
        return(NULL);
    }

    if (dl->pDescription) {
        hr = StringCchCopy(pname, (MAX_PATH+15), dl->pDescription);
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_COPY);
			return (NULL);
		}
    } else {
        hr = StringCchCopy(pname, (MAX_PATH+15), dl->rootname);
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_COPY);
			return (NULL);
		}

        if (dl->pPattern) {
            hr = StringCchCat(pname,(MAX_PATH+15), "\\");
			if (FAILED(hr)) {
				OutputError(hr, IDS_SAFE_CAT);
				return (NULL);
			}
            hr = StringCchCat(pname,(MAX_PATH+15),dl->pPattern);
			if (FAILED(hr)) {
				OutputError(hr, IDS_SAFE_CAT);
				return (NULL);
			}
        }
    }

    return(pname);
}

/*
 * free up a string returned from dir_getrootdescription
 */
VOID
dir_freerootdescription(
                        DIRLIST dl,
                        LPSTR string
                        )
{
    HeapFree(GetProcessHeap(), NULL, string);
}


/*
 * dir_getrootpath
 *
 * return the path to the DIRLIST root. This will be a valid path, not
 * including the checksum server name or pPattern etc
 */
LPSTR
dir_getrootpath(
                DIRLIST dl
                )
{
    return(dl->rootname);
}



/*
 * free up a path created by dir_getrootpath
 */
void
dir_freerootpath(
                 DIRLIST dl,
                 LPSTR path
                 )
{
    return;
}


/*
 * set custom description for dirlist
 */
void
dir_setdescription(DIRLIST dl, LPCSTR psz)
{
    dl->pDescription = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlen(psz) + 1);
	if (dl->pDescription) {
        HRESULT hr = StringCchCopy(dl->pDescription, (lstrlen(psz)+1), psz);
		if (FAILED(hr)) 
			OutputError(hr, IDS_SAFE_COPY);
	}
}




/*
 * returns TRUE if the DIRLIST parameter has a wildcard specified
 */
BOOL
dir_iswildcard(
               DIRLIST dl
               )
{
    return (dl->pPattern != NULL);
}




/* --- DIRITEM functions ----------------------------------------------- */


/*
 * Return a handle to the DIRLIST given a handle to the DIRITEM within it.
 *
 */
DIRLIST
dir_getlist(
            DIRITEM item
            )
{
    if (item == NULL) {
        return(NULL);
    } else {
        return(item->direct->head);
    }
}


/*
 * return the name of the current file relative to tree root
 * This allocates storage.  Call dir_freerelname to release it.
 */
LPSTR
dir_getrelname(
               DIRITEM cur
               )
{
    LPSTR name;
	HRESULT hr;

    /* check this is a valid item */
    if (cur == NULL) {
        return(NULL);
    }

    name = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH);
	if (name == NULL)
		return NULL;
        hr = StringCchCopy(name, MAX_PATH, cur->direct->relname);
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_COPY);
			return(NULL);
		}
    hr = StringCchCat(name, MAX_PATH, cur->name);
	if (FAILED(hr)) {
		OutputError(hr, IDS_SAFE_CAT);
		return(NULL);
	}

    return(name);
} /* dir_getrelname */


/* free up a relname that we allocated. This interface allows us
 * some flexibility in how we store relative and complete names
 *
 */
void
dir_freerelname(
                DIRITEM cur,
                LPSTR name
                )
{
    if ((cur != NULL)) {
        if (name != NULL) {
            HeapFree(GetProcessHeap(), NULL, name);
        }
    }
} /* dir_freerelname */


/*
 * get an open-able name for the file. This is the complete pathname
 * of the item (DIRLIST rootpath + DIRITEM relname)
 * 
 */
LPSTR
dir_getopenname(
                DIRITEM item
                )
{
    LPSTR fname;
    DIRLIST phead;
	HRESULT hr;

    if (item == NULL) {
        return(NULL);
    }

    phead = item->direct->head;

    if (item->localname != NULL) {
        return(item->localname);
    }

    if (phead->bFile) {
        return(phead->rootname);
    }

    // build up the file name from rootname+relname
    // start with the root portion
    fname = (char*) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH);
    if (!fname)
        return NULL;
    hr = StringCchCopy(fname, MAX_PATH, phead->rootname);
	if (FAILED(hr)) {
		OutputError(hr, IDS_SAFE_COPY);
		return(NULL);
	}

    /*
     * it's a simple local name - add both relname and name to make
     * the complete filename
     */
    /* avoid the . or .\ at the end of the relname */
    if (*CharPrev(fname, fname+lstrlen(fname)) == '\\') {
        hr = StringCchCat(fname, MAX_PATH,&item->direct->relname[2]);
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_COPY);
			return(NULL);
		}
    } else {
        hr = StringCchCat(fname, MAX_PATH,&item->direct->relname[1]);
		if (FAILED(hr)) {
			OutputError(hr, IDS_SAFE_CAT);
			return(NULL);
		}
    }
    hr = StringCchCat(fname, MAX_PATH, item->name);
	if (FAILED(hr)) {
		OutputError(hr, IDS_SAFE_CAT);
		return(NULL);
	}

    return(fname);
}



/*
 * free up memory created by a call to dir_getopenname(). This *may*
 * cause the file to be deleted if it was a temporary copy.
 */
void
dir_freeopenname(
                 DIRITEM item,
                 LPSTR openname
                 )
{
    if ((item == NULL) || (openname == NULL)) {
        return;
    }

    if (item->localname != NULL) {
        /* freed in dir_cleardirect */
        return;
    }
    if (item->direct->head->bFile) {
        /* we used the rootname */
        return;
    }

    HeapFree(GetProcessHeap(), NULL, openname);

} /* dir_freeopenname */


/*
 * return an open file handle to the file. if it is local,
 * just open the file. 
 */
HANDLE
dir_openfile(
             DIRITEM item
             )
{
    LPSTR fname;
    HANDLE fh;


    fname = dir_getopenname(item);
    if (fname == NULL) {
        return INVALID_HANDLE_VALUE;
    }

    fh = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE,
                    0, OPEN_EXISTING, 0, 0);

    dir_freeopenname(item, fname);

    return(fh);
} /* dir_openfile */




/*
 * close a file opened with dir_openfile.
 */
void
dir_closefile(
              DIRITEM item,
              HANDLE fh
              )
{
    CloseHandle(fh);

} /* dir_closefile */



/* Recreate all the checksums and status for di as though
   it had never been looked at before
*/
void
dir_rescanfile(
               DIRITEM di
               )
{
    LPSTR fullname;

    if (di==NULL) return;

    /* start with it invalid, erroneous and zero */
    di->sumvalid = FALSE;
    di->fileerror = TRUE;
    di->checksum = 0;

    fullname = dir_getopenname(di);
    di->size = dir_getpathsizeetc(fullname, &(di->ft_lastwrite), &(di->attr));
    di->checksum = dir_getchecksum(di);

    dir_freeopenname(di, fullname);

    di->sumvalid = !(di->fileerror);

} /* dir_rescanfile */


/* return a TRUE iff item has a valid checksum */
BOOL
dir_validchecksum(
                  DIRITEM item
                  )
{
    return (item!=NULL) && (item->sumvalid);
}


BOOL
dir_fileerror(
              DIRITEM item
              )
{
    return (item == NULL) || (item->fileerror);
}


/* return the current file checksum. Open the file and
 * calculate the checksum if it has not already been done.
 */
DWORD
dir_getchecksum(
                DIRITEM cur
                )
{
    LPSTR fullname;

    /* check this is a valid item */
    if (cur == NULL) {
        return(0);
    }

    if (!cur->sumvalid) {
        /*
         * need to calculate checksum
         */
            LONG err;

            fullname = dir_getopenname(cur);
            cur->checksum = checksum_file(fullname, &err);
            if (err==0) {
                cur->sumvalid = TRUE;
                cur->fileerror = FALSE;
            } else {
                cur->fileerror = TRUE;
                return 0;
            }

            dir_freeopenname(cur, fullname);
    }

    return(cur->checksum);
} /* dir_getchecksum */



/* return the file size (set during scanning) - returns 0 if invalid */
long
dir_getfilesize(
                DIRITEM cur
                )
{
    /* check this is a valid item */
    if (cur == NULL) {
        return(0);
    }


    return(cur->size);
} /* dir_getfilesize */

/* return the file attributes (set during scanning) - returns 0 if invalid */
DWORD
dir_getattr(
            DIRITEM cur
            )
{
    /* check this is a valid item */
    if (cur == NULL) {
        return(0);
    }


    return(cur->attr);
} /* dir_getattr */

/* return the file time (last write time) (set during scanning), (0,0) if invalid */
FILETIME
dir_GetFileTime(
                DIRITEM cur
                )
{
    /* return time of (0,0) if this is an invalid item */
    if (cur == NULL) {
        FILETIME ft;
        ft.dwLowDateTime = 0;
        ft.dwHighDateTime = 0;
        return ft;
    }

    return(cur->ft_lastwrite);

} /* dir_GetFileTime */

/*
 * extract the portions of a name that match wildcards - for now,
 * we only support wildcards at start and end.
 * if pTag is non-null, then the source will have a tag matching it that
 * can also be ignored.
 */
void
dir_extractwildportions(
                       LPSTR pDest,
                       LPSTR pSource,
                       LPSTR pPattern,
                       LPSTR pTag
                       )
{
    int size;

    /*
     * for now, just support the easy cases where there is a * at beginning or
     * end
     */

    if (pPattern[0] == '*') {
        size = lstrlen(pSource) - (lstrlen(pPattern) -1);

    } else if (pPattern[lstrlen(pPattern) -1] == '*') {
        pSource += lstrlen(pPattern) -1;
        size = lstrlen(pSource);
    } else {
        size = lstrlen(pSource);
    }

    if (pTag != NULL) {
        size -= lstrlen(pTag);
    }

    My_mbsncpy(pDest, pSource, size);
    pDest[size] = '\0';
}

/*
 * compares two DIRITEM paths that are both based on wildcards. if the
 * directories match, then the filenames are compared after removing
 * the fixed portion of the name - thus comparing only the
 * wildcard portion.
 */
int
dir_compwildcard(
                DIRLIST dleft,
                DIRLIST dright,
                LPSTR lname,
                LPSTR rname
                )
{
    LPSTR pfinal1, pfinal2;
    char final1[MAX_PATH], final2[MAX_PATH];
    int res;

    /*
     * relnames always have at least one backslash
     */
    pfinal1 = My_mbsrchr(lname, '\\');
    pfinal2 = My_mbsrchr(rname, '\\');

    My_mbsncpy(final1, lname, (size_t)(pfinal1 - lname));
    final1[pfinal1 - lname] = '\0';
    My_mbsncpy(final2, rname, (size_t)(pfinal2 - rname));
    final2[pfinal2 - rname] = '\0';


    /*
     * compare all but the final component - if not the same, then
     * all done.
     */
    res = utils_CompPath(final1,final2);
    if (res != 0) {
        return(res);
    }

    // extract just the wildcard-matching portions of the final elements
    dir_extractwildportions(final1, &pfinal1[1], dleft->pPattern, 0);
    dir_extractwildportions(final2, &pfinal2[1], dright->pPattern, 0);

    return(utils_CompPath(final1, final2));
}

/*
 * compares two DIRLIST items, based on a sequence number rather than filenames.
 */
BOOL dir_compsequencenumber(DIRITEM dleft, DIRITEM dright, int *pcmpvalue)
{
    if (!dleft->sequence && !dright->sequence)
        return FALSE;

    if (!dleft->sequence)
        *pcmpvalue = -1;
    else if (!dright->sequence)
        *pcmpvalue = 1;
    else
        *pcmpvalue = dleft->sequence - dright->sequence;

    return TRUE;
}






/* --- file copying ---------------------------------------------------*/

static int nLocalCopies;        /* cleared in startcopy, ++d in copy
                                ** inspected in endcopy
                                */

/* start a bulk copy */
BOOL
dir_startcopy(
              DIRLIST dl
              )
{
    nLocalCopies = 0;
    return(TRUE);
} /* dir_startcopy */

int
dir_endcopy(
            DIRLIST dl
            )
{
	return(nLocalCopies);
} /* dir_endcopy */

/* Build the real path from item and newroot into newpath.
 * Create directories as needed so that it is valid.
 * If mkdir fails, return FALSE, but return the full path that we were
 * trying to make anyway.
 */
BOOL
dir_MakeValidPath(
                  LPSTR newpath,
                  DIRITEM item,
                  LPSTR newroot
                  )
{
    LPSTR relname;
    LPSTR pstart, pdest, pel;
    BOOL bOK = TRUE;
	HRESULT hr;

    /*
     * name of file relative to the tree root
     */
    relname = dir_getrelname(item);

    /*
     * build the new pathname by concatenating the new root and
     * the old relative name. add one path element at a time and
     * ensure that the directory exists, creating it if necessary.
     */
    hr = StringCchCopy(newpath, MAX_PATH, newroot);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);

    /* add separating slash if not already there */
    if (*CharPrev(newpath, newpath+lstrlen(newpath)) != '\\') {
        hr = StringCchCat(newpath, MAX_PATH, "\\");
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_CAT);
    }

    pstart = relname;
    while ( (pel = My_mbschr(pstart, '\\')) != NULL) {

        /*
         * ignore .
         */
        if (My_mbsncmp(pstart, ".\\", 2) != 0) {

            pdest = &newpath[lstrlen(newpath)];

            // copy all but the backslash
            // on NT you can create a dir 'fred\'
            // on dos you have to pass 'fred' to _mkdir()
            My_mbsncpy(pdest, pstart, (size_t)(pel - pstart));
            pdest[pel - pstart] = '\0';

            /* create subdir if necessary */
            if (!dir_isvaliddir(newpath)) {
                if (_mkdir(newpath) != 0) {
                    /* note error, but keep going */
                    bOK = FALSE;
                }
            }

            // now insert the backslash
            hr = StringCchCat(pdest, (MAX_PATH - lstrlen(newpath)),"\\");
			if (FAILED(hr)) 
				OutputError(hr, IDS_SAFE_CAT);
        }

        /* found another element ending in slash. incr past the \\ */
        pel++;

        pstart = pel;
    }

    /*
     * there are no more slashes, so pstart points at the final
     * element
     */
    hr = StringCchCat(newpath, (MAX_PATH - lstrlen(newpath)), pstart);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_CAT);
    dir_freerelname(item, relname);
    return bOK;
}



/*
 * create a copy of the file, in the new root directory.
 * returns TRUE for success and FALSE for failure.
 */
BOOL
dir_copy(
         DIRITEM item,
         LPSTR newroot,
         BOOL HitReadOnly,
         BOOL CopyNoAttributes
         )
{
    /*
     * newpath must be static for Win 3.1 so that it is in the
     * data segment (near) and not on the stack (far).
     */
    static char newpath[MAX_PATH];
    BOOL bOK;

    char msg[MAX_PATH+40];
    BY_HANDLE_FILE_INFORMATION bhfi;
    HANDLE hfile;
    DWORD fa;
	HRESULT hr;

    /*
     * check that the newroot directory itself exists
     */
    if ((item == NULL) || !dir_isvaliddir(newroot)) {
        return(FALSE);
    }

    if (!dir_MakeValidPath(newpath, item, newroot)) return FALSE;
     
    /* local copy of file */
    LPSTR pOpenName;

    pOpenName = dir_getopenname(item);

    /* if the target file already exists and is readonly,
     * warn the user, and delete if ok (remembering to clear
     * the read-only flag
     */
    bOK = TRUE;
    fa = GetFileAttributes(newpath);
    if ( (fa != -1) &&  (fa & FILE_ATTRIBUTE_READONLY)) {
        hr = StringCchPrintf(msg, (MAX_PATH+40), LoadRcString(IDS_IS_READONLY),
                 (LPSTR) newpath);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_PRINTF);

        sdkdiff_UI(TRUE);
        if ((HitReadOnly)
		 || (MessageBox(hwndClient, msg, LoadRcString(IDS_COPY_FILES),
                           MB_OKCANCEL|MB_ICONSTOP) == IDOK)) {
            sdkdiff_UI(FALSE);
            SetFileAttributes(newpath, fa & ~FILE_ATTRIBUTE_READONLY);
            DeleteFile(newpath);   
        } else {
            sdkdiff_UI(FALSE);
            bOK = FALSE; /* don't overwrite */
             // abort the copy... go and release resources
        }
     }

    if (bOK) {
        bOK = CopyFile(pOpenName, newpath, FALSE);
    }
    // The attributes are copied by CopyFile
    if (bOK) {

        /* having copied the file, now copy the times */
        hfile = CreateFile(pOpenName, GENERIC_READ, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        GetFileTime(hfile, &bhfi.ftCreationTime,
                    &bhfi.ftLastAccessTime, &bhfi.ftLastWriteTime);
        CloseHandle(hfile);

        hfile = CreateFile(newpath, GENERIC_WRITE, 0, NULL,
                           OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        SetFileTime(hfile, &bhfi.ftCreationTime,
                    &bhfi.ftLastAccessTime,
                    &bhfi.ftLastWriteTime);
        CloseHandle(hfile);

        if (CopyNoAttributes) {
            // Prepare to kill the attributes...
            SetFileAttributes(newpath, FILE_ATTRIBUTE_NORMAL);
        } 
    }
    if (bOK)
        ++nLocalCopies;

    dir_freeopenname(item, pOpenName);
  

    return(bOK);
} /* dir_copy */



/*--- internal functions ---------------------------------------- */

/* fill out a new DIRECT for a subdirectory (pre-allocated).
 * init files and dirs lists to empty (List_Create). set the relname
 * of the directory by pre-pending the parent relname if there
 * is a parent, and appending a trailing slash (if there isn't one).
 */
void
dir_dirinit(
            DIRECT dir,
            DIRLIST head,
            DIRECT parent,
            LPSTR name
            )
{
    int size;
	HRESULT hr;

    dir->head = head;
    dir->parent = parent;

    /* add on one for the null and one for the trailing slash */
    size = lstrlen(name) + 2;
    if (parent != NULL) {
        size += lstrlen(parent->relname);
    }

    /* build the relname from the parent and the current name
     * with a terminating slash
     */
    dir->relname = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, size);
    if (dir->relname == NULL)
		return;
	if (parent != NULL) {
        hr = StringCchCopy(dir->relname, size,parent->relname);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY);
    } else {
        dir->relname[0] = '\0';
    }

    hr = StringCchCat(dir->relname, size, name);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_CAT);

    if (*CharPrev(dir->relname, dir->relname+lstrlen(dir->relname)) != '\\')
    {
        hr = StringCchCat(dir->relname, size, "\\");
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_CAT);
    }

    /* force name to lowercase */
    AnsiLowerBuff(dir->relname, lstrlen(dir->relname));

    dir->diritems = List_Create();
    dir->directs = List_Create();
    dir->bScanned = FALSE;
    dir->pos = DL_FILES;

} /* dir_dirinit */

/* initialise the contents of an (allocated) DIRITEM struct. checksum
 * the file if dir->head->bSum is true
 *
 */
BOOL
dir_fileinit(
             DIRITEM pfile,
             DIRECT dir,
             LPSTR path,
             long size,
             FILETIME ft,
             DWORD attr,
             int *psequence
             )
{
    BOOL bFileOk = TRUE;
	HRESULT hr;

    pfile->name = (char*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, lstrlen(path) + 1);
    if (pfile->name == NULL)
		return NULL;

	hr = StringCchCopy(pfile->name, (lstrlen(path)+1), path);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);

	/*force name to lower case */
    AnsiLowerBuff(pfile->name, lstrlen(path));

    pfile->direct = dir;
    pfile->size = size;
    pfile->ft_lastwrite = ft;
    pfile->attr = attr;

    pfile->sequence = psequence ? *psequence : 0;

    pfile->localname = NULL;

    if (dir->head->bSum) {
        LONG err;
        LPSTR openname;

        openname = dir_getopenname(pfile);
        pfile->checksum = checksum_file(openname, &err);

        if (err!=0) {
            pfile->sumvalid = FALSE;
        } else {
            pfile->sumvalid = TRUE;
        }
        dir_freeopenname(pfile, openname);

    } else {
        pfile->sumvalid = FALSE;
    }

    return bFileOk;
} /* dir_fileinit */



/* is this a valid file or not */
BOOL
dir_isvalidfile(
                LPSTR path
                )
{
    DWORD dwAttrib;

    dwAttrib = GetFileAttributes(path);
    if (dwAttrib == -1) {
        return(FALSE);
    }
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        return(FALSE);
    }
    return(TRUE);
} /* dir_isvalidfile */


/* is this a valid directory ? */
BOOL
dir_isvaliddir(
               LPCSTR path
               )
{
    DWORD dwAttrib;

    dwAttrib = GetFileAttributes(path);
    if (dwAttrib == -1) {
        return(FALSE);
    }
    if (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) {
        return(TRUE);
    }
    return(FALSE);
} /* dir_isvaliddir */



/*
 * scan the directory given. add all files to the list
 * in alphabetic order, and add all directories in alphabetic
 * order to the list of child DIRITEMs. If bRecurse is true, go on to
 * recursive call dir_scan for each of the child DIRITEMs
 */
void
dir_scan(
         DIRECT dir,
         BOOL bRecurse
         )
{
    PSTR path=NULL, completepath = NULL;
    int size;
    DIRECT child;
    BOOL bMore;
    long filesize;
    FILETIME ft;
    BOOL bIsDir;
    LPSTR name;
    HANDLE hFind;
    WIN32_FIND_DATA finddata;
	HRESULT hr;

    /* make the complete search string including *.* */
    size = lstrlen(dir->head->rootname);
    size += lstrlen(dir->relname);

    /* add on one null and \*.* */
    // in fact, we need space for pPattern instead of *.* but add an
    // extra few in case pPattern is less than *.*
    if (dir->head->pPattern != NULL) {
        size += lstrlen(dir->head->pPattern);
    }
    size += 5;

    path = (PSTR)HeapAlloc(GetProcessHeap(), NULL, size);
	if (path == NULL)
		goto LSkip;
    completepath = (PSTR)HeapAlloc(GetProcessHeap(), NULL, size);
	if (completepath == NULL)
		goto LSkip;

    if (!path || !completepath)
        goto LSkip;

    /*
     * fill out path with all but the *.*
     */
    hr = StringCchCopy(path, size, dir->head->rootname);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);

    /* omit the . at the beginning of the relname, and the
     * .\ if there is a trailing \ on the rootname
     */
    if (*CharPrev(path, path+lstrlen(path)) == '\\') {
        hr = StringCchCat(path, size, &dir->relname[2]);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_CAT);
    } else {
        hr = StringCchCat(path, size, &dir->relname[1]);
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_CAT);
    }


    /*
     * do this scan twice, once for subdirectories
     * (using *.* as the final element)
     * and the other for files (using the pattern or *.* if none)
     */

    hr = StringCchCopy(completepath, size, path);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);
    hr = StringCchCat(completepath, size, "*.*");
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_CAT);


    /*
     * scan for all subdirectories
     */

    hFind = FindFirstFile(completepath, &finddata);
    bMore = (hFind != INVALID_HANDLE_VALUE);

    while (bMore) {

        bIsDir = (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        name = (LPSTR) &finddata.cFileName;
        filesize = finddata.nFileSizeLow;     
        if (bIsDir) {
            if ( (lstrcmp(name, ".") != 0) &&
                 (lstrcmp(name, "..") != 0))  {

                if (dir->head->pOtherDirList == NULL) {
                    dir_adddirect(dir, name);
                } else {
                    char otherName[MAX_PATH+1];
                    hr = StringCchCopy(otherName, (MAX_PATH+1), dir_getrootpath(dir->head->pOtherDirList));
					if (FAILED(hr))
						OutputError(hr, IDS_SAFE_COPY);
                    if (otherName[strlen(otherName)-1] == '\\') {
                        hr = StringCchCat(otherName, (MAX_PATH+1), &dir->relname[2]);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_CAT);
                    } else {
                        hr = StringCchCat(otherName, (MAX_PATH+1), &dir->relname[1]);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_CAT);
                    }
                    hr = StringCchCat(otherName, (MAX_PATH+1), name);
					if (FAILED(hr))
						OutputError(hr, IDS_SAFE_CAT);

                    if (dir_isvaliddir(otherName)) {
                        dir_adddirect(dir, name);
                    }
                }
            }
        }
        if (bAbort) break;  /* User requested abort */

        bMore = FindNextFile(hFind, &finddata);
    }

    FindClose(hFind);

    /*
     * now do it a second time looking for files
     */
    hr = StringCchCopy(completepath, size, path);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);
    hr = StringCchCat(completepath, size, 
            dir->head->pPattern == NULL ? "*.*" : dir->head->pPattern);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_CAT);

    /* read all file entries in the directory */
    hFind = FindFirstFile(completepath, &finddata);
    bMore = (hFind != INVALID_HANDLE_VALUE);

    while (bMore) {
        if (bAbort) break;  /* user requested abort */

        bIsDir = (finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
        name = (LPSTR) &finddata.cFileName;
        filesize = finddata.nFileSizeLow;
        ft = finddata.ftLastWriteTime;
        if (!bIsDir) {
			dir_addfile(dir, name, filesize, ft, finddata.dwFileAttributes, 0);
        }
        bMore = FindNextFile(hFind, &finddata);
    }

    FindClose(hFind);

LSkip:
    HeapFree(GetProcessHeap(), NULL, path);
    HeapFree(GetProcessHeap(), NULL, completepath);

    dir->bScanned = TRUE;
    dir->pos = DL_FILES;

    if (bRecurse) {
		for( child=(DIRECT)List_First(dir->directs);  child!=NULL;  child = (DIRECT)List_Next((LPVOID)child)) {
            if (bAbort) break;  /* user requested abort */
            dir_scan(child, TRUE);
        }
    }

} /* dir_scan */


/*
 * add the file 'path' to the list of files in dir, in order.
 *
 * checksum the file if dir->head->bSum  is true
 *
 */
BOOL
dir_addfile(
            DIRECT dir,
            LPSTR path,
            DWORD size,
            FILETIME ft,
            DWORD attr,
            int *psequence
            )
{
    DIRITEM pfile;

    AnsiLowerBuff(path, lstrlen(path));  

    // when psequence is passed, do not sort the list
    if (!psequence)
    {
		for( pfile=(DIRITEM)List_Last(dir->diritems);  pfile!=NULL;  pfile = (DIRITEM)List_Prev((LPVOID)pfile)) {
            if (utils_CompPath(pfile->name, path) <= 0) {
                break;     /* goes after this one */
            }
        }
        /* goes after pfile, NULL => goes at start */
        pfile = (DIRITEM)List_NewAfter(dir->diritems, pfile, sizeof(struct diritem));
    }
    else
    {
        /* append to end -- psequence implies that we are being called in
         * sequence order and do not need to do any further sorting.
         */
        pfile = (DIRITEM)List_NewBefore(dir->diritems, NULL, sizeof(struct diritem));
    }

	if (!dir_fileinit(pfile, dir, path, size, ft, attr, psequence))
    {
        List_Delete(pfile);
        return FALSE;
    }

    return TRUE;
} /* dir_addfile */


/* add a new directory in alphabetic order on
 * the list dir->directs
 *
 */
void
dir_adddirect(
              DIRECT dir,
              LPSTR path
              )
{
    DIRECT child;
    LPSTR finalel;
    char achTempName[MAX_PATH];

    AnsiLowerBuff(path, lstrlen(path));
    for( child=(DIRECT)List_First(dir->directs);  child!=NULL;  child = (DIRECT)List_Next((LPVOID)child)) {
        int cmpval;

        /* we need to compare the child name with the new name.
         * the child name is a relname with a trailing
         * slash - so compare only the name up to but
         * not including the final slash.
         */
        finalel = dir_finalelem(child->relname);

        /*
         * we cannot use strnicmp since this uses a different
         * collating sequence to lstrcmpi. So copy the portion
         * we are interested in to a null-term. buffer.
         */
        My_mbsncpy(achTempName, finalel, lstrlen(finalel)-1);
        achTempName[lstrlen(finalel)-1] = '\0';

        cmpval = utils_CompPath(achTempName, path);

#ifdef trace
        {   
			HRESULT hr;
			char msg[600];
            hr = StringCchPrintf( msg, 600, "dir_adddirect: %s %s %s\n"
                      , achTempName
                      , ( cmpval<0 ? "<"
                          : (cmpval==0 ? "=" : ">")
                        )
                      , path
                    );
			if (FAILED(hr))
				OutputError(hr, IDS_SAFE_PRINTF);
            if (bTrace) Trace_File(msg);
        }
#endif
        if (cmpval > 0) {

            /* goes before this one */
            child = (DIRECT)List_NewBefore(dir->directs, child, sizeof(struct direct));
            dir_dirinit(child, dir->head, dir, path);
            return;
        }
    }
    /* goes at end */
    child = (DIRECT)List_NewLast(dir->directs, sizeof(struct direct));
    dir_dirinit(child, dir->head, dir, path);
} /* dir_adddirect */


/* free all memory associated with a DIRECT (including freeing
 * child lists). Don't de-alloc the direct itself (allocated on a list)
 */
void
dir_cleardirect(
                DIRECT dir
                )
{
    DIRITEM pfile;
    DIRECT child;

    /* clear contents of files list */
	for( pfile=(DIRITEM)List_First(dir->diritems);  pfile!=NULL;  pfile = (DIRITEM)List_Next((LPVOID)pfile)) {
        HeapFree(GetProcessHeap(), NULL, pfile->name);

        if (pfile->localname) {
            if (pfile->bLocalIsTemp) {
                /*
                 * the copy will have copied the attributes,
                 * including read-only. We should unset this bit
                 * so we can delete the temp file.
                 */
                SetFileAttributes(pfile->localname,
                                  GetFileAttributes(pfile->localname)
                                  & ~FILE_ATTRIBUTE_READONLY);
                DeleteFile(pfile->localname);
            }

            HeapFree(GetProcessHeap(), NULL, pfile->localname);
            pfile->localname = NULL; 
        }
    }
    List_Destroy(&dir->diritems);

    /* clear contents of dirs list (recursively) */
	for( child=(DIRECT)List_First(dir->directs);  child!=NULL;  child = (DIRECT)List_Next((LPVOID)child)) {
        dir_cleardirect(child);
    }
    List_Destroy(&dir->directs);

    HeapFree(GetProcessHeap(), NULL, dir->relname);

} /* dir_cleardirect */



/*
 * return a pointer to the final element in a path. note that
 * we may be passed relnames with a trailing final slash - ignore this
 * and return the element before that final slash.
 */
LPSTR
dir_finalelem(
              LPSTR path
              )
{
    LPSTR chp;
    int size;

    /* is the final character a slash ? */
    size = lstrlen(path) - 1;
    if (*(chp = CharPrev(path, path+lstrlen(path))) == '\\') {
            /* find the slash before this */
            while (chp > path) {
                    if (*(chp = CharPrev(path, chp)) == '\\') {
                            /* skip the slash itself */
                            chp++;
                            break;
                    }
            }
            return(chp);
    }
    /* look for final slash */
    chp = My_mbsrchr(path, '\\');
    if (chp != NULL) {
        return(chp+1);
    }

    /* no slash - is there a drive letter ? */
    chp = My_mbsrchr(path, ':');
    if (chp != NULL) {
        return(chp+1);
    }

    /* this is a final-element anyway */
    return(path);

} /* dir_finalelem */



/* find the size of a file given a pathname to it */
long
dir_getpathsizeetc(
                   LPSTR path,
                   FILETIME *pft,
                   DWORD *pattr
                   )
{
    HANDLE fh;
    long size;

    fh = CreateFile(path, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (fh == INVALID_HANDLE_VALUE)
    {
        HANDLE hFind;
        WIN32_FIND_DATA finddata;

        hFind = FindFirstFile(path, &finddata);
        if (hFind == INVALID_HANDLE_VALUE)
        {
            return 0;                      
        }
        else
        {
            FindClose(hFind);
            if (pft)
                *pft = finddata.ftLastWriteTime;
            if (pattr != NULL)
                *pattr = finddata.dwFileAttributes;
            return finddata.nFileSizeLow;
        }
    }

    size = GetFileSize(fh, NULL);
    if (pft)
        GetFileTime(fh, NULL, NULL, pft);
    if (pattr != NULL)
        *pattr = GetFileAttributes(path);
    CloseHandle(fh);
    return(size);
} /* dir_getpathsize */
 
/* ---- helpers ----------------------------------------------------------- */

BOOL iswildpath(LPCSTR pszPath)
{
    if (strchr(pszPath, '*') || strchr(pszPath, '?'))
        return TRUE;

    if (!(pszPath[0] && pszPath[0] == '/' && pszPath[1] && pszPath[1] == '/'))
    {
        DWORD dw;

        dw = GetFileAttributes(pszPath);
        if (dw != (DWORD)-1 && (dw & FILE_ATTRIBUTE_DIRECTORY))
            return TRUE;
    }

    return FALSE;
}
