// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * utils.cpp
 *
 * some standard file-reading, hashing and checksum routines.
 */

#include "precomp.h"

#include <winnls.h>

#include "gutilsrc.h"



#define IS_BLANK(c) \
    (((c) == ' ') || ((c) == '\t') || ((c) == '\r'))

const WCHAR c_wchMagic = 0xfeff;        // magic marker for Unicode files


/*
 * we need an instance handle. this should be the dll instance
 */
extern HANDLE hLibInst;

/*
 * -- forward declaration of procedures -----------------------------------
 */
INT_PTR dodlg_stringin(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

/*-- readfile: buffered line input ------------------------------*/

/*
 * set of functions to read a line at a time from a file, using
 * a buffer to read a block at a time from the file
 *
 */

/*
 * a FILEBUFFER handle is a pointer to a struct filebuffer
 */
struct filebuffer {
    HANDLE fh;      /* open file handle */
    LPSTR start;    /* offset within buffer of next character */
    LPSTR last;     /* offset within buffer of last valid char read in */

    char buffer[BUFFER_SIZE];

    BOOL fUnicode;  /* TRUE if the file is Unicode */
    WCHAR wzBuffer[MAX_LINE_LENGTH];
    LPWSTR pwzStart;
    LPWSTR pwzLast;
};

typedef enum {
    CT_LEAD = 0,
    CT_TRAIL = 1,
    CT_ANK = 2,
    CT_INVALID = 3,
} DBCSTYPE;

DBCSTYPE
DBCScharType(
            LPTSTR str,
            int index
            )
{
    /*
        TT .. ??? maybe LEAD or TRAIL
        FT .. second == LEAD
        FF .. second == ANK
        TF .. ??? maybe ANK or TRAIL
    */
    //readfile_next uses this on fbuf->buffer which is explicitly NOT null-terminated.
    if ( index >= 0) {   //  EOS is valid parameter.
        LPTSTR pos = str + index;
        DBCSTYPE candidate = (IsDBCSLeadByte( *pos-- ) ? CT_LEAD : CT_ANK);
        BOOL maybeTrail = FALSE;
        for ( ; pos >= str; pos-- ) {
            if ( !IsDBCSLeadByte( *pos ) )
                break;
            maybeTrail ^= 1;
        }
        return maybeTrail ? CT_TRAIL : candidate;
    }
    return CT_INVALID;
}

/*
 * initialise a filebuffer and return a handle to it
 */
FILEBUFFER
APIENTRY
readfile_new(
            HANDLE fh,
            BOOL *pfUnicode
            )
{
    FILEBUFFER fbuf;
    DWORD cbRead;
    WCHAR wchMagic;

    if (pfUnicode) 
    {
        *pfUnicode = FALSE;
    }

    fbuf = (FILEBUFFER) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct filebuffer));
    if (fbuf == NULL) {
        return(NULL);
    }

    if (pfUnicode) 
    {
        /* return file pointer to beginning of file */
        SetFilePointer(fh, 0, NULL, FILE_BEGIN);

        if (!ReadFile(fh, &wchMagic, sizeof(wchMagic), &cbRead, NULL)) {
            HeapFree(GetProcessHeap(), NULL, fbuf);
            return(NULL);
        }

        fbuf->fh = fh;
        fbuf->start = fbuf->buffer;
        fbuf->last = fbuf->buffer;
        fbuf->fUnicode = FALSE;
        if (cbRead == 2 && c_wchMagic == wchMagic) 
        {
            fbuf->fUnicode = TRUE;
            *pfUnicode = TRUE;
            fbuf->pwzStart = fbuf->wzBuffer;
            fbuf->pwzLast = fbuf->wzBuffer;
        } 
        else 
        {
            SetFilePointer(fh, 0, NULL, FILE_BEGIN);
        }
    }

    return(fbuf);
}

/* delims is the set of delimiters used to break lines
 * For program source files the delimiter is \n.
 * Full stop (aka period) i.e. "." is another obvious one.
 * The delimiters are taken as
 * being part of the line they terminate.
 *
 */
static BYTE delims[256];

/* set str to be the set of delims.  str is a \0 delimited string */
void
APIENTRY
readfile_setdelims(
                  LPBYTE str
                  )
{
    /* clear all bytes of delims */
    int i;
    for (i=0; i<256; ++i) {
        delims[i] = 0;
    }

    /* set the bytes in delims which correspond to delimiters */
    for (; *str; ++str) { delims[(int)(*str)] = 1;
    }

} /* readfile_setdelims */


static BOOL FFindEOL(FILEBUFFER fbuf, LPSTR *ppszLine, int *pcch, LPWSTR *ppwzLine, int *pcwch)
{
    LPSTR psz;
    LPWSTR pwz;

    if (fbuf->fUnicode) 
    {
        for (pwz = fbuf->pwzStart; pwz < fbuf->pwzLast; pwz++) 
        {
            if (!*pwz)
                *pwz = '.';

            if (*pwz < 256 && delims[*pwz]) 
            {
                *pcwch = (UINT)(pwz - fbuf->pwzStart) + 1;
                *ppwzLine = fbuf->pwzStart;
                fbuf->pwzStart += *pcwch;
                // notice we fall thru and let the loop below actually return
                break;
            }
        }
    }
    for (psz = fbuf->start; psz < fbuf->last; psz = CharNext(psz)) 
    {
        if (!*psz)
            *psz = '.';

        // use LPBYTE cast to make sure sign extension doesn't index
        // negatively
        if (delims[*(LPBYTE)psz]) 
        {
            *pcch = (UINT)(psz - fbuf->start) + 1;
            *ppszLine = fbuf->start;
            fbuf->start += *pcch;
            return TRUE;
        }
    }
    return FALSE;
}


/*
 * get the next line from a file. returns a pointer to the line
 * in the buffer - so copy it before changing it.
 *
 * the line is *not* null-terminated. *plen is set to the length of the
 * line.
 *
 * A line is terminated by any character in the static var set delims.
 */
__declspec(thread) char szACP[MAX_LINE_LENGTH * sizeof(WCHAR)];
__declspec(thread) WCHAR wzRoundtrip[MAX_LINE_LENGTH];

LPSTR APIENTRY
readfile_next(
             FILEBUFFER fbuf,
             int * plen,
             LPWSTR *ppwz,
             int *pcwch
             )
{
    LPSTR cstart;
    UINT cbFree;
    DWORD cbRead; 

    *ppwz = NULL;
    *pcwch = 0;

    /* look for an end of line in the buffer we have */
    if (FFindEOL(fbuf, &cstart, plen, ppwz, pcwch)) 
    {
        return cstart;
    }

    /* no delimiter in this buffer - this buffer contains a partial line.
     * copy the partial up to the beginning of the buffer, and
     * adjust the pointers to reflect this move
     */
    if (fbuf->fUnicode) 
    {
        if ( ( (fbuf->pwzLast - fbuf->pwzStart) <= MAX_LINE_LENGTH) && ( (fbuf->pwzLast - fbuf->pwzStart) >= 0 ) ) 
        {
            memmove(fbuf->wzBuffer, fbuf->pwzStart, (LPBYTE)fbuf->pwzLast - (LPBYTE)fbuf->pwzStart);
        } 
        else 
        {
            return NULL;
        }
        fbuf->pwzLast = fbuf->wzBuffer + (fbuf->pwzLast - fbuf->pwzStart);
        fbuf->pwzStart = fbuf->wzBuffer;
    }

    if ( ((fbuf->last - fbuf->start) <= BUFFER_SIZE) && ((fbuf->last - fbuf->start) >= 0 ) ) 
    {
        memmove(fbuf->buffer, fbuf->start, (LPBYTE)fbuf->last - (LPBYTE)fbuf->start);
    } 
    else 
    {
        return NULL;
    }

    fbuf->last = fbuf->buffer + (fbuf->last - fbuf->start);
    fbuf->start = fbuf->buffer;

    /* read in to fill the block */
    if (fbuf->fUnicode) 
    {
        // for unicode files, we'll read in the unicode and convert it
        // to ansi.  we are converting to ACP, then converting
        // back to unicode, and comparing the two unicode strings.  for any
        // wchars that are not identical, we replace them with 5-byte hex
        // codes of the format xFFFF.
        UINT cchAnsi;
        UINT cchWide;
        UINT cchRoundtrip;
        LPWSTR pwzOrig;
        LPCWSTR pwzRoundtrip;
        LPSTR pszACP;

        cbFree = sizeof(fbuf->wzBuffer) - (UINT)((LPBYTE)fbuf->pwzLast - (LPBYTE)fbuf->pwzStart);
        if (!ReadFile(fbuf->fh, fbuf->pwzLast, cbFree, &cbRead, NULL)) {
            return NULL;
        }

        // wide to ansi
        cchWide = cbRead / 2;
        cchAnsi = WideCharToMultiByte(GetACP(),
                                      0,
                                      fbuf->pwzLast,
                                      cchWide,
                                      szACP,
                                      DimensionOf(szACP),
                                      NULL,
                                      NULL);

        // round trip, to find chars not in ACP
        cchRoundtrip = MultiByteToWideChar(GetACP(),
                                           0,
                                           szACP,
                                           cchAnsi,
                                           wzRoundtrip,
                                           DimensionOf(wzRoundtrip));

        // find non-ACP chars
        pwzOrig = fbuf->pwzLast;
        pwzRoundtrip = wzRoundtrip;
        pszACP = szACP;
        while (cchWide && cchRoundtrip) 
        {
            if (*pwzOrig == *pwzRoundtrip) 
            {
                // copy the DBCS representation into the buffer
                if (IsDBCSLeadByte(*pszACP))
                    *(fbuf->last++) = *(pszACP++);
                *(fbuf->last++) = *(pszACP++);
            } 
            else 
            {
                // copy a hexized representation into the buffer
                static const char rgHex[] = "0123456789ABCDEF";
                *(fbuf->last++) = 'x';
                *(fbuf->last++) = rgHex[((*pwzOrig) >> 12) & 0xf];
                *(fbuf->last++) = rgHex[((*pwzOrig) >>  8) & 0xf];
                *(fbuf->last++) = rgHex[((*pwzOrig) >>  4) & 0xf];
                *(fbuf->last++) = rgHex[((*pwzOrig) >>  0) & 0xf];
                if (IsDBCSLeadByte(*pszACP))
                    pszACP++;
                pszACP++;
            }

            ++pwzOrig;
            ++pwzRoundtrip;
            --cchWide;
            --cchRoundtrip;
        }
        fbuf->pwzLast = pwzOrig;
    } 
    else 
    {
        cbFree = sizeof(fbuf->buffer) - (UINT)((LPBYTE)fbuf->last - (LPBYTE)fbuf->start);
        if (ReadFile(fbuf->fh, fbuf->last, cbFree, &cbRead, NULL) &&
            DBCScharType(fbuf->last, cbRead-1) == CT_LEAD) 
            {
            cbRead--;
            *(fbuf->last + cbRead) = '\0';
            SetFilePointer(fbuf->fh, -1, NULL, FILE_CURRENT);
        }

        fbuf->last += cbRead;
    }

    /* look for an end of line in the newly filled buffer */
    if (FFindEOL(fbuf, &cstart, plen, ppwz, pcwch)) 
    {
        return cstart;
    }

    /* still no end of line. either the buffer is empty -
     * because of end of file - or the line is longer than
     * the buffer. in either case, return all that we have
     */

    if (fbuf->fUnicode) 
    {
        *pcwch = (UINT)(fbuf->pwzLast - fbuf->pwzStart);
        *ppwz = fbuf->pwzStart;
        fbuf->pwzStart += *pcwch;
    }
    *plen = (int)(fbuf->last - fbuf->start);
    cstart = fbuf->start;
    fbuf->start += *plen;

    if (*plen == 0) {
        return(NULL);
    } else {
        return(cstart);
    }
}


/*
 * delete a FILEBUFFER -  free the buffer. We should NOT close the
 * handle at this point as we did not open it. the opener should close
 * it with a function that corresponds to however he opened it.
 */
void APIENTRY
readfile_delete(
               FILEBUFFER fbuf
               )
{
    HeapFree(GetProcessHeap(), NULL, fbuf);
}


/* --- checksum ----------------------------------------------------  */

/*
 * Produce a checksum for a file:
 * Open a file, checksum it and close it again. err !=0 iff it failed.
 *
 * Overall scheme:
 *         Read in file in blocks of 8K (arbitrary number - probably
 *         beneficial if integral multiple of disk block size).
 *         Generate checksum by the formula
 *         checksum = SUM( rnd(i)*(dword[i]) )
 *         where dword[i] is the i-th dword in the file, the file being
 *         extended by up to three binary zeros if necessary.
 *         rnd(x) is the x-th element of a fixed series of pseudo-random
 *         numbers.
 *
 * You may notice that dwords that are zero do not contribute to the checksum.
 * This worried me at first, but it's OK.  So long as everything else DOES
 * contribute, the checksum still distinguishes between different files
 * of the same length whether they contain zeros or not.
 * An extra zero in the middle of a file will also cause all following non-zero
 * bytes to have different multipliers.  However the algorithm does NOT
 * distinguish between files which only differ in zeros at the end of the file.
 * Multiplying each dword by a pseudo-random function of its position
 * ensures that "anagrams" of each other come to different sums,
 * i.e. the file AAAABBBB will be different from BBBBAAAA.
 * The pseudorandom function chosen is successive powers of 1664525 modulo 2**32
 * 1664525 is a magic number taken from Donald Knuth's "The Art Of Computer Programming"
 *
 * The function appears to be compute bound.  Loop optimisation is appropriate!
 */
CHECKSUM 
APIENTRY
checksum_file(
             LPCSTR fn,
             LONG * err
             )
{
    HANDLE fh=0;
#define BUFFLEN 8192
    BYTE buffer[BUFFLEN];
    unsigned long lCheckSum = 0;         /* grows into the checksum */
    const unsigned long lSeed = 1664525; /* seed for random (Knuth) */
    unsigned long lRand = 1;             /* seed**n */
    unsigned Byte = 0;                   /* buffer[Byte] is next byte to process */
    DWORD Block = 0;                  /* number of bytes in buffer */
    BOOL Ending = FALSE;                 /* TRUE => binary zero padding added */
    int i;                               /* temp loop counter */

    *err = -2;                            /* default is "silly" */

    /* conceivably someone is fiddling with the file...?
       we give 6 goes, with delays of 1,2,3,4 and 5 secs between
    */
    for (i=0; i<=5; ++i) {
        Sleep(1000*i);
        fh = CreateFile(fn, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (fh!=INVALID_HANDLE_VALUE)
            break;

        {
            char msg[300];
            HRESULT hr = StringCchPrintf( msg,300, "SDKDiff: retry open. Error(%d), file(%s)\n"
                                          , GetLastError(), fn);
            if (FAILED(hr)) 
            {
                OutputError(hr, IDS_SAFE_PRINTF);
            }
            OutputDebugString(msg);
        }
    }

    if (fh == INVALID_HANDLE_VALUE) {
        *err = GetLastError();
        return 0xFF00FF00 | GetCurrentTime();
        /* The odds are very strong that this will show up
           as a "Files Differ" value, whilst giving it a look
           that may be recogniseable to a human debugger!
        */
    }

    /* we assume that the file system will always give us the full length that
     * we ask for unless the end-of-file is encountered.
     * This means that for the bulk of a long file the buffer goes exactly into 4s
     * and only at the very end are some bytes left over.
     */

    for ( ; ;) {
        /* Invariant: (which holds at THIS point in the flow)
         * A every byte in every block already passed has contributed to the checksum
         * B every byte before buffer[byte] in current block has contributed
         * C Byte is a multiple of 4
         * D Block is a multiple of 4
         * E Byte <= Block
         * F Ending is TRUE iff zero padding has been added to any block so far.
         * G lRand is (lSeed to the power N) MOD (2 to the power 32)
         *   where N is the number of dwords in the file processed so far
         *   including both earlier blocks and the current block
         * To prove the loop good:
         * 1. Show invariant is initially true
         * 2. Show invariant is preserved by every loop iteration
         * 3. Show that IF the invariant is true at this point AND the program
         *    exits the loop, then the right answer will have been produced.
         * 4. Show the loop terminates.
         */

        if (Byte>=Block) {
            if (Byte>Block) {
                Trace_Error(NULL, "Checksum internal error.  Byte>Block", FALSE);
                *err = -1;
                break;                 /* go home */
            }
            if (!ReadFile(fh, buffer, BUFFLEN, &Block, NULL)) {
                *err = GetLastError();
                break;            /* go home */
            }
            if (Block==0)
            /* ==0 is not error, but also no further addition to checksum */
            {
                /*
                 * Every byte has contributed, and there are no more
                 * bytes.  Checksum complete
                 */
                *err = 0;
                CloseHandle(fh);
                return lCheckSum;        /* success! */
            }

            if (Ending) {
                char msg[300];
                HRESULT hr = StringCchPrintf( msg, 300, "Short read other than last in file %s\n", fn);
                if (FAILED(hr)) 
                {
                    OutputError(hr, IDS_SAFE_PRINTF);
                }
                OutputDebugString(msg);
                break;          /* go home */
            }

            while ( (Block%4) && (Block < BUFFLEN)) 
            {
                buffer[Block++] = 0;
                Ending = TRUE;
            }
            /* ASSERT the block now has a multiple of 4 bytes */
            Byte = 0;
        }
        lRand *= lSeed;
        lCheckSum += lRand* *((DWORD *)(&buffer[Byte]));
        Byte += 4;
    }
    CloseHandle(fh);
    return 0xFF00FF00 | GetCurrentTime();   /* See first "return" in function */
} /* checksum_file */





/* --- internal error popups ----------------------------------------*/

static BOOL sbUnattended = FALSE;

void
Trace_Unattended(
                BOOL bUnattended
                )
{
    sbUnattended = bUnattended;
} /* Trace_Unattended */


/* This function is called to report errors to the user.
 * if the current operation is abortable, this function will be
 * called with fCancel == TRUE and we display a cancel button. otherwise
 * there is just an OK button.
 *
 * We return TRUE if the user pressed OK, or FALSE otherwise (for cancel).
 */
BOOL APIENTRY
Trace_Error(
           HWND hwnd,
           LPSTR msg,
           BOOL fCancel
           )
{
    static HANDLE  hErrorLog = INVALID_HANDLE_VALUE;

    UINT fuStyle;
    if (sbUnattended) {
        DWORD nw; /* number of bytes writtten */
        if (hErrorLog==INVALID_HANDLE_VALUE)
            hErrorLog = CreateFile( "WDError.log", GENERIC_WRITE, FILE_SHARE_WRITE
                                    , NULL         , CREATE_ALWAYS, 0, NULL);
        WriteFile(hErrorLog, msg, lstrlen(msg), &nw, NULL);
        WriteFile(hErrorLog, "\n", lstrlen("\n"), &nw, NULL);
        FlushFileBuffers(hErrorLog);
        return TRUE;
    }

    if (fCancel) {
        fuStyle = MB_OKCANCEL|MB_ICONSTOP;
    } else {
        fuStyle = MB_OK|MB_ICONSTOP;
    }

    if (MessageBox(hwnd, msg, NULL, fuStyle) ==  IDOK) {
        return(TRUE);
    } else {
        return(FALSE);
    }
}

/* ------------ Tracing to a file ------------------------------------*/

static HANDLE  hTraceFile = INVALID_HANDLE_VALUE;

void
APIENTRY
Trace_File(
          LPSTR msg
          )
{
    DWORD nw; /* number of bytes writtten */
    if (hTraceFile==INVALID_HANDLE_VALUE)
        hTraceFile = CreateFile( "sdkdiff.trc"
                                 , GENERIC_WRITE
                                 , FILE_SHARE_WRITE
                                 , NULL
                                 , CREATE_ALWAYS
                                 , 0
                                 , NULL
                               );

    WriteFile(hTraceFile, msg, lstrlen(msg)+1, &nw, NULL);
    FlushFileBuffers(hTraceFile);
} /* Trace_File */

void
APIENTRY
Trace_Close(
           void
           )
{
    if (hTraceFile!=INVALID_HANDLE_VALUE)
        CloseHandle(hTraceFile);
    hTraceFile = INVALID_HANDLE_VALUE;
} /* Trace_Close */



/* ----------- things for strings-------------------------------------*/


/*
 * Compare two pathnames, and if not equal, decide which should come first.
 * Both path names should be lower cased by AnsiLowerBuff before calling.
 *
 * returns 0 if the same, -1 if left is first, and +1 if right is first.
 *
 * The comparison is such that all filenames in a directory come before any
 * file in a subdirectory of that directory.
 *
 * given direct\thisfile v. direct\subdir\thatfile, we take
 * thisfile < thatfile   even though it is second alphabetically.
 * We do this by picking out the shorter path
 * (fewer path elements), and comparing them up till the last element of that
 * path (in the example: compare the 'dir\' in both cases.)
 * If they are the same, then the name with more path elements is
 * in a subdirectory, and should come second.
 *
 * We have had trouble with apparently multiple collating sequences and
 * the position of \ in the sequence.  To eliminate this trouble
 * a. EVERYTHING is mapped to lower case first (actually this is done
 *    before calling this routine).
 * b. All comparison is done by using lstrcmpi with two special cases.
 *    1. Subdirs come after parents as noted above
 *    2. \ must compare low so that fred2\x > fred\x in the same way
 *       that fred2 < fred.  Unfortunately in ANSI '2' < '\\'
 *
 */
int APIENTRY
utils_CompPath(
              LPSTR left,
              LPSTR right
              )
{
    int compval;            // provisional value of comparison

    if (left==NULL) return -1;          // empty is less than anything else
    else if (right==NULL) return 1;           // anything is greater than empty

    for (; ; ) {
        if (*left=='\0' && *right=='\0') return 0;
        if (*left=='\0')  return -1;
        if (*right=='\0')  return 1;

        if (IsDBCSLeadByte(*left) || IsDBCSLeadByte(*right)) {
            if (*right != *left) {
                compval = (*left - *right);
                break;
            }
            ++left;
            ++right;
            if (*right != *left) {
                compval = (*left - *right);
                break;
            }
            ++left;
            ++right;
        } else {
            if (*right==*left) { ++left; ++right; continue; }
            if (*left=='\\') { compval = -1; break; }
            if (*right=='\\') { compval = 1; break; }
            compval = (*left - *right); 
            break;
        }
    }

    /* We have detected a difference.  If the rest of one
       of the strings (including the current character) contains
       some \ characters, but the other one does not, then all
       elements up to the last element of the one with the fewer
       elements are equal and so the other one lies in a subdir
       and so compares greater i.e. x\y\f > x\f
       Otherwise compval tells the truth.
    */

    left = My_mbschr(left, '\\');
    right = My_mbschr(right, '\\');
    if (left && !right) return 1;
    if (right && !left) return -1;

    return compval;

} /* utils_CompPath */


/*
 * generate a hashcode for a null-terminated ascii string.
 *
 * if bIgnoreBlanks is set, then ignore all spaces and tabs in calculating
 * the hashcode.
 *
 * multiply each character by a function of its position and sum these.
 * The function chosen is to multiply the position by successive
 * powers of a large number.
 * The large multiple ensures that anagrams generate different hash
 * codes.
 */
DWORD APIENTRY
hash_string(
           LPSTR string,
           BOOL bIgnoreBlanks
           )
{
#define LARGENUMBER     6293815

    DWORD sum = 0;
    DWORD multiple = LARGENUMBER;
    int index = 1;

    while (*string != '\0') {

        if (bIgnoreBlanks) {
            while (IS_BLANK(*string)) {
                string++;
            }
        }

        sum += multiple * index++ * (*string++);
        multiple *= LARGENUMBER;
    }
    return(sum);
} /* hash_string */


/* return TRUE iff the string is blank.  Blank means the same as
 * the characters which are ignored in hash_string when ignore_blanks is set
 */
BOOL APIENTRY
utils_isblank(
             LPSTR string
             )
{
    while (IS_BLANK(*string)) {
        string++;
    }

    /* having skipped all the blanks, do we see the end delimiter? */
    return(*string == '\0' || *string == '\n');
}



/* --- simple string input -------------------------------------- */

/*
 * static variables for communication between function and dialog
 */
LPSTR dlg_result;
int dlg_size;
LPSTR dlg_prompt, dlg_default, dlg_caption;

/*
 * input of a single text string, using a simple dialog.
 *
 * returns TRUE if ok, or FALSE if error or user canceled. If TRUE,
 * puts the string entered into result (up to resultsize characters).
 *
 * prompt is used as the prompt string, caption as the dialog caption and
 * default as the default input. All of these can be null.
 */

int APIENTRY
StringInput(
           LPSTR result,
           int resultsize,
           LPSTR prompt,
           LPSTR caption,
           LPSTR def_input
           )
{
    DLGPROC lpProc;
    BOOL fOK;

    /* copy args to static variable so that winproc can see them */

    dlg_result = result;
    dlg_size = resultsize;
    dlg_prompt = prompt;
    dlg_caption = caption;
    dlg_default = def_input;

    lpProc = (DLGPROC)MakeProcInstance((WINPROCTYPE)dodlg_stringin, hLibInst);
    fOK = (BOOL) DialogBox((HINSTANCE)hLibInst, "StringInput", GetFocus(), lpProc);
    FreeProcInstance((WINPROCTYPE)lpProc);

    return(fOK);
}

INT_PTR
dodlg_stringin(
              HWND hDlg,
              UINT message,
              WPARAM wParam,
              LPARAM lParam
              )
{
    switch (message) {
    
    case WM_INITDIALOG:
        if (dlg_caption != NULL) {
            SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM) dlg_caption);
        }
        if (dlg_prompt != NULL) {
            SetDlgItemText(hDlg, IDD_GUTILS_LABEL, dlg_prompt);
        }
        if (dlg_default) {
            SetDlgItemText(hDlg, IDD_GUTILS_FILE, dlg_default);
        }
        return(TRUE);

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam)) {
        
        case IDCANCEL:
            EndDialog(hDlg, FALSE);
            return(TRUE);

        case IDOK:
            GetDlgItemText(hDlg, IDD_GUTILS_FILE, dlg_result, dlg_size);
            EndDialog(hDlg, TRUE);
            return(TRUE);
        }
    }
    return(FALSE);
}

#if 0
/***************************************************************************
 * Function: My_mbspbrk
 *
 * Purpose:
 *
 * DBCS version of strpbrk
 *
 */
PUCHAR
My_mbspbrk(
          PUCHAR psz,
          PUCHAR pszSep
          )
{
    PUCHAR pszSepT;
    while (*psz != '\0') {
        pszSepT = pszSep;
        while (*pszSepT != '\0') {
            if (*pszSepT == *psz) {
                return psz;
            }
            pszSepT = CharNext(pszSepT);
        }
        psz = CharNext(psz);
    }
    return NULL;
}

/***************************************************************************
 * Function: My_mbschr
 *
 * Purpose:
 *
 * DBCS version of strchr
 *
 */

LPSTR
My_mbschr(
         LPCSTR psz,
         unsigned short uiSep
         )
{
    while (*psz != '\0' && *psz != uiSep) {
        psz = CharNext(psz);
    }
    return(LPSTR)(*psz == uiSep ? psz : NULL);
}

/***************************************************************************
 * Function: My_mbsncpy
 *
 * Purpose:
 *
 * DBCS version of strncpy
 *
 */

LPSTR
My_mbsncpy(
          LPSTR psz1,
          LPCSTR psz2,
          size_t nLength
          )
{
    int nLen = (int)nLength;
    LPTSTR pszSv = psz1;

    while (0 < nLen) {
        if (*psz2 == '\0') {
            *psz1++ = '\0';
            nLen--;
        } else if (IsDBCSLeadByte(*psz2)) {
            if (nLen == 1) {
                *psz1 = '\0';
            } else {
                *psz1++ = *psz2++;
                *psz1++ = *psz2++;
            }
            nLen -= 2;
        } else {
            *psz1++ = *psz2++;
            nLen--;
        }
    }
    return pszSv;
}

/***************************************************************************
 * Function: LoadRcString
 *
 * Purpose: Loads a resource string from string table and returns a pointer
 *          to the string.
 *
 * Parameters: wID - resource string id
 *
 */

LPTSTR
APIENTRY
LoadRcString(
            UINT wID
            )
{
    static TCHAR szBuf[512];

    LoadString((HANDLE)GetModuleHandle(NULL),wID,szBuf,sizeof(szBuf));
    return szBuf;
}
#endif
