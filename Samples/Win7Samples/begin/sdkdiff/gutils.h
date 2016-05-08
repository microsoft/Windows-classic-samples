// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/* standard header for gutils.dll library functions
 * include after windows.h
 */
#ifndef __GUTILS_H__
#define __GUTILS_H__

#define DimensionOf(x) (sizeof(x) / sizeof(x[0]))

/*--------win-16 win-32 porting macros etc ----------------------------*/

/* win32 msg crackers */
#define GET_WM_COMMAND_ID(w, l) (LOWORD(w))
#define GET_WM_COMMAND_CMD(w, l) (HIWORD(w))
#define GET_WM_COMMAND_HWND(w, l) (l)
#define GET_SCROLL_OPCODE(w, l)     (LOWORD(w))
#define GET_SCROLL_POS(w, l)        (HIWORD(w))
#define WINPROCTYPE     WNDPROC

// function for initializing gutils
BOOL WINAPI InitGutils(HANDLE hInstance, DWORD dwReason, LPVOID reserved);



/* ---- file open/save common dialogs ---------------------------*/

/*
 * these functions now rely on to calls to the common dialog libraries.
 *
 * parameters:
 *      prompt - user prompt text (eg for dialog title)
 *      ext    - default extension (eg ".txt")
 *      spec   - default file spec (eg "*.*")
 *      pszFull - full filename will be copied here.
 *      cchMax - size of pszFull buffer.
 *      fn     - last component of file name will be copied here.
 * returns - TRUE if user selected a file that could be opened for
 *           reading (gfile_open) or created and opened for writing (gfile_new)
 *           FALSE if user canceled. if user selects a file that cannot be
 *           opened, a message box is put up and the dialog re-shown.
 */
BOOL APIENTRY gfile_open(HWND hwnd, LPSTR prompt, LPSTR ext, LPSTR spec,
                         LPSTR pszFull, int cchMax, LPSTR fn);
BOOL APIENTRY gfile_new(LPSTR prompt, LPSTR ext, LPSTR spec,
                        LPSTR pszFull, int cchMax, LPSTR fn);



/* --- status line window class ---------------------------------- */
/* The status line is a bar across the top or bottom of the window.
 * It can hold a number of fields which can be either static text
 * or buttons.  The so called "static" text can be changed at any time.
 * The fields can be left or right aligned (default is RIGHT).
 * If the text is marked as VAR then the screen real estate allocated
 * for it will be adjusted whenever the text changes.  VAR fields
 * can be given minimum or maximum sizes (but not both).
 *
 * STATIC text fields can be drawn as raised or lowered rectangles (using
 * shades of grey), or (default) without a border. BUTTON fields will
 * always be drawn as raised rectangles, and will lower when pressed.
 *
 * Button fields will send WM_COMMAND messages when clicked including the
 * field id and the WM_LBUTTONUP notification code. Note that that this
 * is not a full implementation of the button class, and no other messages
 * will be sent. In general, none of the fields of a status bar are
 * implemented as separate windows, so GetDlgItem() and similar calls will not
 * work. Buttons only respond to mouse down events, and there is no handling
 * of the focus or of keyboard events.
 *
 * To use:
 *    call StatusAlloc giving the number of items you are going to add to the
 *    status bar. This returns a handle to use in subsequent calls.
 *
 *    Then call StatusAddItem to define each item in turn.
 *    Buttons are placed in order of definition along the bar starting from
 *    the left (SF_LEFT) and from the right (SF_RIGHT) until the two
 *    sides meet.
 *
 *    Call StatusHeight to find the expected height of this status bar, and
 *    set its position within the parent window, then call StatusCreate to
 *    create the window.
 *
 * Having created the window, send SM_SETTEXT messages to set the new
 * text of a field (static or button), or SM_NEW with a handle (obtained from
 * StatusAlloc) to change the contents of the status line.
 */

/* values for type argument to StatusAddItem */
#define SF_BUTTON       1
#define SF_STATIC       2

/* bits in flags argument to StatusAddItem */
#define SF_RAISE        1       /* paint static as raised 3D rectangle */
#define SF_LOWER        2       /* paint static as lowered 3D rectangle */
#define SF_LEFT         4       /* align field on left of status bar */
#define SF_RIGHT        8       /* align field on right (DEFAULT) */
#define SF_VAR          0x10    /* size of field depends on actual text extent*/
#define SF_SZMAX        0x20    /* (with SF_VAR): width argument is maximum */
#define SF_SZMIN        0x40    /* (with SF_VAR) width arg is minimum size */

/* interfaces */
HWND APIENTRY StatusCreate(HANDLE hInst, HWND hParent, INT_PTR id,
                           LPRECT rcp, HANDLE hmem);

/* return the recommended height in device units of the given status bar */
int APIENTRY StatusHeight(HANDLE hmem);

/* alloc the status bar data structures and return handle*/
HANDLE APIENTRY StatusAlloc(int nitems);

/* set the attributes of a field.
 *
 * hmem obtained from StatusAlloc. itemnr must be less than the nitems
 * passed to StatusAlloc.
 *
 * the width argument is the width of the field in characters (average
 * character width).
 */
BOOL APIENTRY StatusAddItem(HANDLE hmem, int itemnr, int type, int flags,
                            int id, int width, LPSTR text);

/* send these window messages to the class */

#define SM_NEW          (WM_USER+1)     /* wParam handle for new status line */
#define SM_SETTEXT      (WM_USER+2)     /* wparam: item id, lparam new label*/

/* --- bit-map freelist management functions -------------------------------*/

/* init a pre-allocated array of longs to map nblks - set all to free
   you should allocate 1 DWORD in map for every 32 blocks of storage
   you wish to control.
*/
void APIENTRY gbit_init(DWORD FAR * map, long nblks);

/* mark a range of nblks starting at blknr to be busy */
BOOL APIENTRY gbit_alloc(DWORD FAR * map, long blknr, long nblks);

/* mark a range of nblks starting at blknr to be free */
BOOL APIENTRY gbit_free(DWORD FAR * map, long blknr, long nblks);

/* find a free section nblks long, or the biggest found in the map if all
 * are less than nblks long. returns size of region found as return value,
 * and sets blknr to the starting blk of region. Region is *not* marked
 * busy
 */
long APIENTRY gbit_findfree(DWORD FAR* map, long nblks,
                            long mapsize, long FAR * blknr);


/* ----- buffered line input ----------------------------------*/

/*
 * functions for reading a file, one line at a time, with some buffering
 * to make the operation reasonably efficient.
 *
 * call readfile_new to initialise the buffer and give it a handle to
 * an open file. Call readfile_next to get a pointer to the next line.
 * This discards the previous line and gives you a pointer to the line
 * IN THE BUFFER. Make your own copy before calling readfile_next again.
 *
 * call readfile_delete once you have finished with this file. That will close
 * the file and free up any memory.
 */

// MAX_LINE_LENGTH is the max number of physical characters we allow in a line
#define MAX_LINE_LENGTH         (4096)
// BUFFER_SIZE is expressed in bytes, and is large enough to read in
// MAX_LINE_LENGTH wide chars, and also hold MAX_LINE_LENGTH 5-byte hex code
// representations of the chars.
#define BUFFER_SIZE             (MAX_LINE_LENGTH * 5)

/* handle to a file buffer */
typedef struct filebuffer FAR * FILEBUFFER;

/* initialise the buffering for an open file */
FILEBUFFER APIENTRY readfile_new(HANDLE fh, BOOL *pfUnicode);

/* return a pointer to the next line in this file. line must be shorter than
 * buffer size (currently 1024 bytes). Line is not null-terminated: *plen
 * is set to the length of the line including the \n. This call will
 * discard any previous line, so ensure that you have made a copy of one line
 * before you call readfile_next again.
 * MUST CALL readfile_setdelims FIRST!
 */
LPSTR APIENTRY readfile_next(FILEBUFFER fb, int FAR * plen, LPWSTR *ppwz, int *pcwch);

/* set the delimiters to use to break lines.  MUST call this to initialise */
void APIENTRY readfile_setdelims(LPBYTE str);

/*
 * close the file and discard any associated memory and buffers.
 */
void APIENTRY readfile_delete(FILEBUFFER fb);


/* ------ hashing and checksums ------------------------------------------- */

/*
 * generate a 32-bit hash code for a null-terminated string of ascii text.
 *
 * if bIgnoreBlanks is TRUE, we ignore spaces and tabs during the
 * hashcode calculation.
 */

/* hash codes are unsigned longs */

DWORD APIENTRY hash_string(LPSTR string, BOOL bIgnoreBlanks);

/* return TRUE iff the string is blank.  Blank means the same as
 * the characters which are ignored in hash_string when ignore_blanks is set
 */
BOOL APIENTRY utils_isblank(LPSTR string);

/*
 * Compare two pathnames, and if not equal, decide which should come first.
 *
 * returns 0 if the same, -1 if left is first, and +1 if right is first.
 *
 * The comparison is such that all filenames in a directory come before any
 * file in a subdirectory of that directory.
 *
 * To make absolutely certain that you get a canonical sorting, use AnsiLowerBuff
 * to convert BOTH to lower case first.  You may get a funny effect if one one
 * has been converted to lower case and the other not.
 */
int APIENTRY
utils_CompPath(LPSTR left, LPSTR right);
/* given an open file handle open for reading, read the file and
 * generate a 32-bit checksum for the file
 */

/* checksums are unsigned longs */
typedef DWORD CHECKSUM;

/* Open a file, checksum it and close it again. err !=0 iff it failed. */
CHECKSUM APIENTRY checksum_file(LPCSTR fn, LONG FAR * err);


/* --- error message output ----------------------------------------------*/

/*
 * reports error in a dialog, returns TRUE for ok, FALSE for cancel.
 * if fCancel is FALSE, only the OK button is shown, otherwise both ok
 * and cancel. hwnd is the parent window for the dlg. can be null.
 */
BOOL APIENTRY Trace_Error(HWND hwnd, LPSTR msg, BOOL fCancel);

/* Write popups to a file until further notice */
void Trace_Unattended(BOOL bUnattended);

/* --- create/write to trace file ----------------------------------------*/

void APIENTRY Trace_File(LPSTR msg);

/* --- close trace file --------------------------------------------------*/
void APIENTRY Trace_Close(void);

/* --- simple input ------------------------------------------------------*/

/*
 * input of a single text string, using a simple dialog.
 *
 * returns TRUE if ok, or FALSE if error or user canceled. If TRUE,
 * puts the string entered into result (up to resultsize characters).
 *
 *
 * prompt is used as the prompt string, caption as the dialog caption and
 * def_input as the default input. All of these can be null.
 */

int APIENTRY StringInput(LPSTR result, int resultsize, LPSTR prompt,
                         LPSTR caption, LPSTR def_input);



/* --- sockets -----------------------------------------------------------*/

#ifdef SOCKETS

    #include <winsock.h>

BOOL SocketConnect( LPSTR pstrServerName, u_short TCPPort, SOCKET *pSocket );
BOOL SocketListen( u_short TCPPort, SOCKET *pSocket );

#endif

// These are for both SDKDIFF.EXE and GUTILS.DLL.
PUCHAR My_mbspbrk(PUCHAR, PUCHAR);
LPSTR My_mbschr(LPCSTR, unsigned short);
LPSTR My_mbsncpy(LPSTR, LPCSTR, size_t);

// These are for SDKDIFF.EXE.
LPSTR My_mbsrchr(LPCSTR, unsigned short);
int My_mbsncmp(LPCSTR, LPCSTR, size_t);
LPTSTR APIENTRY LoadRcString(UINT);
#endif
