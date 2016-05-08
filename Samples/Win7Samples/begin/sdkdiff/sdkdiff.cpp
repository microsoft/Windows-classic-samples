// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*
 * Sdkdiff
 *
 * file and directory comparisons.
 *
 * Compare two directories (including all files and subdirs). Look for names
 * that are present in both (report all that are not). For files that
 * are present in both, produce a line-by-line comparison of the differences
 * between the two files (if any).
 *
 * Overview of Sdkdiff internals - the whole program.
 *
 * Sdkdiff is built from several modules (a "module" has a .h file
 * which describes its interface and a .c file which implements it)
 * Apart from THIS comment which tries to give an overview of the whole
 * scheme of things, each module is as self-contained as possible.
 * This is enforced by the use of opaque data types.  Modules cannot
 * see each others' internal data structures.  Modules are abstract
 * data types.  The term "Module" (from Modula2) and "Class" (from C++)
 * are used synonymously.
 *
 *    Sdkdiff  - main program - parse arguments, put up main window,
 *               handle input, calling other modules as needed
 *               invoke table class to create the main display and
 *               service callbacks from the table class.
 *               Contains global flags for options (e.g. ignore_blanks)
 *    list     - (in gutils) a generalised LIST of anything data type
 *               has full set of operations for insert, delete, join etc.
 *    line     - a LINE is a numbered line of text.  Information is kept to
 *               allow fast comparisons of LINEs.  A LINE can hold a
 *               link to another LINE.  The links are used to connect
 *               lines in one file to matching lines in the other file.
 *    file     - a FILEDATA represents a file as a file name in the form
 *               of a DIRITEM and a LIST of LINEs
 *    scandir  - a DIRITEM represents information about a file.  (for
 *               instance its name, whether it has a known checksum whether
 *               it has a local copy).
 *               a DIRLIST represents a directory, has information on how to
 *               get to it (pipename? password? UNC name etc) and
 *               (within an imbedded DIRECT structure) a LIST of DIRITEMs
 *               representing the files in the directory and a LIST of
 *               DIRECTs representing its subdirectories.
 *    compitem - a COMPITEM is a pair of files together with information
 *               on how they compare in the form of a breakdown of the
 *               files into a LIST of matching or non-matching sections.
 *               Either file can be absent.  This module contains the
 *               file "contrast" algorithm used for the actual comparison
 *               (Algorithm people see ci_compare then talk to Laurie).
 *    tree       (in gutils) A binary tree.  Important because it is what
 *               gives the file comparison its speed as it makes it
 *               an "N log N" algorithm rather than "N squared"
 *    complist - a COMPLIST is the master data structure.  It has a DIRLIST
 *               of the left hand files, a DIRLIST of the right hand files
 *               and a LIST of COMPITEMs. The left and right hand DIRLISTs
 *               are working data used to produce the COMPLIST.  The LIST
 *               is displayed as the outline table.  Any given COMPITEM can
 *               be displayed as an expanded item.
 *    section  - a SECTION is a section of a file (first line, last line)
 *               and information as to what it matches in the other file.
 *    bar.c    - the picture down the left of the screen
 *               has a WNDPROC.  There is no bar.h, neither is there much
 *               of writeup! ???
 *    view     - Although the COMPLIST is the master state, it doesn't do
 *               all the work itself.  The data is actually displayed by
 *               the table class which is highly generalised.  View
 *               owns a COMPLIST (and therefore calls upon the functions
 *               in complist to fill it and interrogate it) and calls
 *               upon (and is called back by) the functions in table to
 *               actually display it.  Read about table in gutils.h
 *    table.c    (in gutils) a highly generalised system for displaying
 *               data in rows and columns.  The interface is in gutils.h
 *               read it if you hope to understand view!
 *    status.c   (in gutils) the status line at the top. See gutils.h
 *
 * The data structures:
 * Each "module" owns storage which is an encapsulated data type, inaccessable
 * from the outside.  Thus COMPLIST holds a list of COMPITEMs, but they are
 * pointers to structures whose definitions are out of scope, thus they are
 * "just opaque pointers".  To access anything in the COMPITEM you have to
 * call functions in COMPITEM.C.  And so on.  The overall scheme of how they
 * link together is below.  Some things are identified by field name, some by
 * type name, some both, some abbreviations.  Many connecting arrows omitted.
 * Look in the C files for details.
 *
 * COMPLIST
 * > left   -----------> DIRLIST    <--------------------
 * > right  -----------> > rootname                       |
 * > LIST of items--     > bFile                          |
 *                  |    > bSum                           |
 *  ----------------     > dot--------> DIRECT     <------+-------------------------
 * |                     > server       > relname         |                         |
 * |                     > hpipe        > DIRLIST head ---                          |
 * |                     > uncname      > DIRECT parent                             |
 * |                     > password     > bScanned                                  |
 * |                                    > LIST of diritems-----> DIRITEM            |
 * |                                    > LIST OF directs     -> > name             |
 * |                                    > enum pos           |   > int size         |
 * |                                    > DIRECT curdir      |   > int checksum     |
 * |                                                         |   > bool sumvalid    |
 * |                                                         |   > DIRECT direct ---
 * |                                                         |   > localname
 *  --->COMPITEM                                             |   > bLocalIsTemp
 *      > left-------------------> FILEDATA                  |
 *      > right------------------> > DIRITEM-----------------
 *      > LIST of CompSecs---      > LIST of lines--> LINE
 *      > LIST of LeftSecs---|                    --> > flags
 *      > LIST of RightSecs--|                   |    > text
 *                           |                   |    > hash
 *                           |                   |    > link
 *                           |                   |    > linenr
 *                            --> SECTION        |
 *                                > first--------|
 *                                > last---------
 *                                > bDiscard
 *                                > SECTION link
 *                                > SECTION correspond
 *                                > int state
 *                                > int leftbase
 *                                > int rightbase
 *
 *
 *************************************************************************
 *
 * Overview of THIS file's business:
 *
 *   we create a table window (gutils.dll) to show the files and the
 *   results of their comparisons. We create a COMPLIST object representing
 *   a list of files and their differences, and a VIEW object to map between
 *   the rows of the table window and the COMPLIST.
 *
 *   This module is responsible for creating and managing the main window,
 *   placing the child windows (table, status window etc) within it, and
 *   handling all menu items. We maintain global option flags set by
 *   menu commands.
 *
 *   Creating a COMPLIST creates a list of unmatched files, and of matching
 *   files that are compared with each other (these are COMPITEMS).
 *   The VIEW provides a mapping between rows on the screen, and items in
 *   the COMPLIST.
 *
 * Something about threads:  (See also thread DOGMA, below)
 *
 *   The win32 version tries to maintain a responsive user interface by
 *   creating worker threads to do long jobs.  This potentially creates
 *   conflicts between the threads as they will both want to update common
 *   variables (for instance the UI thread may be changing the options to
 *   exclude identical files while the worker thread is adding in the
 *   results of new comparisons).  Critical sections are used to manage
 *   the conflicts (as you'd expect).
 *
 *   The Edit options invoke an editor on a separate thread.  This allows
 *   us to repaint our window and thereby allow the user to refer back to
 *   what he saw before invoking the editor.  When he's finished editing,
 *   we would of course like to refresh things and if this is still on the
 *   separate thread it might clash. We avoid this clash by POSTing ourselves
 *   a (WM_COMMAND, IDM_UPDATE) message.
 */

#include "precomp.h"
#include <shellapi.h>
#include <htmlhelp.h>			/* for .CHM file processing */

#include "table.h"
#include <richedit.h>           /* needed for usage dialog */

#include "list.h"               /* needed for compitem.h */
#include "scandir.h"            /* needed for file.h     */
#include "file.h"               /* needed for compitem.h */
#include "compitem.h"           /* needed for view.h     */
#include "complist.h"
#include "view.h"
#include "findgoto.h"

#include "state.h"
#include "sdkdiff.h"
#include "wdiffrc.h"



/*--constants and data types--------------------------------------------*/
CRITICAL_SECTION CSSdkdiff;
/* IF EVER YOU MIGHT ACQUIRE BOTH CSSdkdiff AND CSView, THEN DO SO IN
   THE ORDER:  FIRST GET CSSdkdiff  THEN GET CSView
   else risk deadlock when an idm_exit happens!
*/
#define WDEnter()       EnterCriticalSection(&CSSdkdiff);
#define WDLeave()       LeaveCriticalSection(&CSSdkdiff);

int Version = 2;
int SubVersion = 01;
char pszWorkingDirectoryName[MAX_PATH];

/* When we print the current table, we pass this id as the table id
 * When we are queried for the properties of this table, we know they
 * want the printing properties for the current view. We use this to
 * select different fonts and colours for the printer.
 */
#define TABID_PRINTER   1

BOOL __BERR;

/*
 * structure containing args passed to worker thread in initial
 * case (executing command line instructions) (in WIN16 case,
 * the worker thread function is called synchronously with these args).
 */
typedef struct {
    LPSTR first;
    LPSTR second;
    LPSTR savelist;
    LPSTR savecomp;
    LPSTR notify;
    UINT listopts;
    UINT compopts;
    VIEW view;
    BOOL fDeep;
    BOOL fExit;
    BOOL fOpenedFiles;                  
    BOOL fDescribeFiles;                
    BOOL fInputFile;                    // TRUE means read file list from input file
    BOOL fInputFileSingle;              // TRUE means input file has one filename per line
} THREADARGS, FAR * PTHREADARGS;


/* structure containing all the arguments we'd like to give to do_editfile
   Need a structure because CreateThread only allows for one argument.
*/
typedef struct {
    VIEW view;
    int option;
    long selection;
} EDITARGS, FAR * PEDITARGS;

/*---- string constants --------------------------- */

const CHAR szSdkDiff[]                = "SdkDiff";
static const char szD[]                      = "%d";
static const char szBlanks[]                 = "Blanks";
static const char szAlgorithm2[]             = "Algorithm2";
static const char szPicture[]                = "Picture";
static const char szMonoColours[]            = "MonoColours";
static const char szHideMark[]               = "HideMark";
static const char szSdkDiffViewerClass[]     = "SdkDiffViewerClass";
static const char szSdkDiffMenu[]            = "SdkDiffMenu";
static const char szOutlineMenu[]            = "OutlineFloatMenu";
static const char szExpandMenu[]             = "ExpandFloatMenu";
static const char szSdkDiffAccel[]           = "SdkDiffAccel";
static const char szBarClass[]               = "BarClass";
static const char szLineNumbers[]            = "LineNumbers";
static const char szFileInclude[]            = "FileInclude";
static const char szLineInclude[]            = "LineInclude";
static const char szOutlineSaved[]           = "OutlineSaved";
static const char szOutlineShowCmd[]         = "OutlineShowCmd";
static const char szOutlineMaxX[]            = "OutlineMaxX";
static const char szOutlineMaxY[]            = "OutlineMaxY";
static const char szOutlineNormLeft[]        = "OutlineNormLeft";
static const char szOutlineNormTop[]         = "OutlineNormTop";
static const char szOutlineNormRight[]       = "OutlineNormRight";
static const char szOutlineNormBottom[]      = "OutlineNormBottom";
static const char szEditor[]                 = "Editor";
static const char szFontFaceName[]           = "FontFaceName";
static const char szFontHeight[]             = "FontHeight";
static const char szFontBold[]               = "FontBold";
static const char szFontCharSet[]            = "FontCharSet";
static const char szExpandedSaved[]          = "ExpandedSaved";
static const char szExpandShowCmd[]          = "ExpandShowCmd";
static const char szExpandMaxX[]             = "ExpandMaxX";
static const char szExpandMaxY[]             = "ExpandMaxY";
static const char szExpandNormLeft[]         = "ExpandNormLeft";
static const char szExpandNormTop[]          = "ExpandNormTop";
static const char szExpandNormRight[]        = "ExpandNormRight";
static const char szExpandNormBottom[]       = "ExpandNormBottom";
static const char szColourPrinting[]         = "ColourPrinting";
static const char szTabWidth[]               = "TabWidth";
static const char szShowWhitespace[]         = "ShowWhitespace";
static const char szrgb_outlinehi[]          = "RGBOutlineHi";
static const char szrgb_leftfore[]           = "RGBLeftFore";
static const char szrgb_leftback[]           = "RGBLeftBack";
static const char szrgb_rightfore[]          = "RGBRightFore";
static const char szrgb_rightback[]          = "RGBRightBack";
static const char szrgb_similarleft[]        = "RGBSimilarLeft";
static const char szrgb_similarright[]       = "RGBSimilarRight";
static const char szrgb_similar[]            = "RGBSimilar";
static const char szrgb_mleftfore[]          = "RGBMLeftFore";
static const char szrgb_mleftback[]          = "RGBMLeftBack";
static const char szrgb_mrightfore[]         = "RGBMRightFore";
static const char szrgb_mrightback[]         = "RGBMRightBack";
static const char szrgb_barleft[]            = "RGBBarLeft";
static const char szrgb_barright[]           = "RGBBarRight";
static const char szrgb_barcurrent[]         = "RGBBarCurrent";
static const char szrgb_defaultfore[]        = "RGBDefaultFore";
static const char szrgb_defaultforews[]      = "RGBDefaultForeWS";
static const char szrgb_defaultback[]        = "RGBDefaultBack";

static const char szrgb_fileleftfore[]       = "RGBFileLeftFore";
static const char szrgb_fileleftback[]       = "RGBFileLeftBack";
static const char szrgb_filerightfore[]      = "RGBFileRightFore";
static const char szrgb_filerightback[]      = "RGBFileRightBack";

/*---- colour scheme------------------------------- */

DWORD rgb_outlinehi = RGB(255, 0, 0);   /* hilighted files in outline mode  */

/* expand view */
DWORD rgb_leftfore;          /* foregrnd for left lines */
DWORD rgb_leftback;          /* backgrnd for left lines */
DWORD rgb_rightfore;         /* foregrnd for right lines*/
DWORD rgb_rightback;         /* backgrnd for right lines*/

/* temp hack */
DWORD rgb_similarleft;       /* forground zebra         */
DWORD rgb_similarright;      /* foreground zebra        */
DWORD rgb_similar;           /* unused                  */

/* moved lines */
DWORD rgb_mleftfore;         /* foregrnd for moved-left */
DWORD rgb_mleftback;         /* backgrnd for moved-left */
DWORD rgb_mrightfore;        /* foregrnd for moved-right*/
DWORD rgb_mrightback;        /* backgrnd for moved-right*/

/* bar window */
DWORD rgb_barleft;           /* bar sections in left only  */
DWORD rgb_barright;          /* bar sections in right only */
DWORD rgb_barcurrent;        /* current pos markers in bar */

DWORD rgb_defaultfore;       /* default foreground */
DWORD rgb_defaultforews;     /* default foreground whitespace */
DWORD rgb_defaultback;       /* default background */

DWORD rgb_fileleftfore;       /* outline mode left only file */
DWORD rgb_fileleftback;       /* outline mode left only file */
DWORD rgb_filerightfore;      /* outline mode right only file */
DWORD rgb_filerightback;      /* outline mode right only file */

BOOL gbPerverseCompare = FALSE; // break lines on punctuation (broken & useless)

/* PickUpProfile */
void PickUpProfile( DWORD * pfoo, LPCSTR szfoo)
{
    *pfoo = GetProfileInt(APPNAME, szfoo, *pfoo);
}


void SetColours(void)
{
    /* outline */

    rgb_outlinehi = (DWORD)RGB(255, 0, 0);   /* hilighted files in outline mode  */
    PickUpProfile(&rgb_outlinehi, szrgb_outlinehi);

    rgb_fileleftfore = (DWORD)RGB(0, 0, 0);   /* left only outline mode  */
    PickUpProfile(&rgb_fileleftfore, szrgb_fileleftfore);
    rgb_fileleftback = (DWORD)RGB(255, 255, 255);
    PickUpProfile(&rgb_fileleftback, szrgb_fileleftback);

    rgb_filerightfore = (DWORD)RGB(0, 0, 0);  /* right only outline mode  */
    PickUpProfile(&rgb_filerightfore, szrgb_filerightfore);
    rgb_filerightback = (DWORD)RGB(255, 255, 255);
    PickUpProfile(&rgb_filerightback, szrgb_filerightback);

    /* expand view */
    rgb_leftfore =   (DWORD)RGB(  0,   0,   0);         /* foregrnd for left lines */
    PickUpProfile(&rgb_leftfore, szrgb_leftfore);
    rgb_leftback  =  (DWORD)RGB(255,   0,   0);         /* backgrnd for left lines */
    PickUpProfile(&rgb_leftback, szrgb_leftback);
    rgb_rightfore =  (DWORD)RGB(  0,   0,   0);         /* foregrnd for right lines*/
    PickUpProfile(&rgb_rightfore, szrgb_rightfore);
    rgb_rightback =  (DWORD)RGB(255, 255,   0);         /* backgrnd for right lines*/
    PickUpProfile(&rgb_rightback, szrgb_rightback);

    rgb_similarleft= (DWORD)RGB(  0, 255, 255);         /* foreground zebra        */
    PickUpProfile(&rgb_similarleft, szrgb_similarleft);
    rgb_similarright=(DWORD)RGB(  0, 127, 127);         /* forground zebra         */
    PickUpProfile(&rgb_similarright, szrgb_similarright);
    rgb_similar   =  (DWORD)RGB(  127, 127, 255);       /* same within comp options*/
    PickUpProfile(&rgb_similar, szrgb_similar);

    /* moved lines */
    rgb_mleftfore =  (DWORD)RGB(  0,   0, 128);         /* foregrnd for moved-left */
    PickUpProfile(&rgb_mleftfore, szrgb_mleftfore);
    rgb_mleftback =  (DWORD)RGB(255,   0,   0);         /* backgrnd for moved-left */
    PickUpProfile(&rgb_mleftback, szrgb_mleftback);
    rgb_mrightfore = (DWORD)RGB(  0,   0, 255);         /* foregrnd for moved-right*/
    PickUpProfile(&rgb_mrightfore, szrgb_mrightfore);
    rgb_mrightback = (DWORD)RGB(255, 255,   0);         /* backgrnd for moved-right*/
    PickUpProfile(&rgb_mrightback, szrgb_mrightback);

    /* bar window */
    rgb_barleft =    (DWORD)RGB(255,   0,   0);         /* bar sections in left only  */
    PickUpProfile(&rgb_barleft, szrgb_barleft);
    rgb_barright =   (DWORD)RGB(255, 255,   0);         /* bar sections in right only */
    PickUpProfile(&rgb_barright, szrgb_barright);
    rgb_barcurrent = (DWORD)RGB(  0,   0, 255);         /* current pos markers in bar */
    PickUpProfile(&rgb_barcurrent, szrgb_barcurrent);

    /* defaults */
    rgb_defaultfore = (DWORD)RGB(   0,   0,   0);       /* default foreground colour */
    PickUpProfile(&rgb_defaultfore, szrgb_defaultfore);
    rgb_defaultforews = (DWORD)RGB(192, 192, 192);      /* default foreground whitespace colour */
    PickUpProfile(&rgb_defaultforews, szrgb_defaultforews);
    rgb_defaultback = (DWORD)RGB(255, 255, 255);        /* default background colour */
    PickUpProfile(&rgb_defaultback, szrgb_defaultback);

} /* SetColours */

void SetMonoColours(void)
{
    rgb_outlinehi = GetSysColor(COLOR_WINDOW);   /* hilighted files in outline mode  */

    /* expand view - all changed or moved lines are white on black */
    rgb_leftfore =   GetSysColor(COLOR_WINDOW);         /* foregrnd for left lines */
    rgb_leftback  =  GetSysColor(COLOR_WINDOWTEXT);     /* backgrnd for left lines */
    rgb_rightfore =  GetSysColor(COLOR_WINDOW);         /* foregrnd for right lines*/
    rgb_rightback =  GetSysColor(COLOR_WINDOWTEXT);     /* backgrnd for right lines*/

    rgb_similarleft= GetSysColor(COLOR_WINDOW);         /* foreground zebra        */
    rgb_similarright=GetSysColor(COLOR_WINDOW);         /* foreground zebra        */
    rgb_similar   =  GetSysColor(COLOR_WINDOWTEXT);     /* same within comp options*/

    /* moved lines - black on white */
    rgb_mleftfore =  GetSysColor(COLOR_WINDOWTEXT);     /* foregrnd for moved-left */
    rgb_mleftback =  GetSysColor(COLOR_WINDOW);	        /* backgrnd for moved-left */
    rgb_mrightfore = GetSysColor(COLOR_WINDOWTEXT);     /* foregrnd for moved-right*/
    rgb_mrightback = GetSysColor(COLOR_WINDOW);         /* backgrnd for moved-right*/

    /* bar WINDOWTEXT */
    rgb_barleft =    GetSysColor(COLOR_WINDOWTEXT);     /* bar sections in left only  */
    rgb_barright =   GetSysColor(COLOR_WINDOWTEXT);     /* bar sections in right only */
    rgb_barcurrent = GetSysColor(COLOR_WINDOWTEXT);     /* current pos markers in bar */


    rgb_defaultfore = GetSysColor(COLOR_WINDOWTEXT);    /* default foreground colour */
    rgb_defaultforews = GetSysColor(COLOR_WINDOWTEXT);  /* default foreground whitespace colour */
    rgb_defaultback = GetSysColor(COLOR_WINDOW);        /* default background colour */

} /* SetMonoColours */

/* -------------------------------------------------- */

/* module static data -------------------------------------------------*/


/* current value of window title */
char AppTitle[256];


HWND hwndClient;        /* main window */
HWND hwndRCD;           /* table window */
HWND hwndStatus;        /* status bar across top */
HWND hwndBar;           /* graphic of sections as vertical bars */

HACCEL haccel;

/* the status bar told us it should be this high. Rest of client area
 * goes to the hwndBar and hwndRCD.
 */
int status_height;

#if 0
    #ifndef HINSTANCE
        #define HINSTANCE HANDLE
    #endif
#endif

HINSTANCE hInst;   /* handle to current app instance */
HMENU hMenu;    /* handle to menu for hwndClient */

int nMinMax = SW_SHOWNORMAL;         /* default state of window normal */

/* the message sent to us as a callback by the table window needs to be
 * registered - table_msgcode is the result of the RegisterMessage call
 */
UINT table_msgcode;

/* true if we are currently doing some scan or comparison.
 * WIN32: must get critical section before checking/changing this (call
 * SetBusy.
 */
BOOL fBusy = FALSE;


long     selection      =       -1;     /* selected row in table*/
long selection_nrows    =       0;      /* number of rows in selection */

/* options for DisplayMode field indicating what is currently shown.
 * we use this to know whether or not to show the graphic bar window.
 */
#define MODE_NULL       0       /* nothing displayed */
#define MODE_OUTLINE    1       /* a list of files displayed */
#define MODE_EXPAND     2       /* view is expanded view of one file */

int DisplayMode = MODE_NULL;    /* indicates whether we are in expand mode */

VIEW current_view = NULL;

BOOL fAutoExpand = TRUE;        /* Should we auto expand ? */

/* These two flags are peeked at by lots of other modules */
BOOL bAbort;            /* set to request abort of current operation */
BOOL bTrace;            /* set if tracing is to be enabled */
BOOL bJapan;            /* set if primary language is Japanese */
BOOL bDBCS;             /* set if primary language is Japanese/Korean/Chinese */

char editor_cmdline[256] = "notepad %p";  /* editor cmdline */

char g_szFontFaceName[LF_FACESIZE];
int g_nFontHeight;
BOOL g_fFontBold;
BYTE g_bFontCharSet;
HFONT g_hFont = 0;

/* app-wide global data --------------------------------------------- */


/* current state of menu options */
int line_numbers = IDM_LNRS;
int expand_mode = IDM_BOTHFILES;
int outline_include = INCLUDE_ALL;
int expand_include = INCLUDE_ALL;
BOOL ignore_blanks = TRUE;
BOOL show_whitespace = FALSE;
BOOL Algorithm2 = TRUE;  /* Try duplicates - used in compitem.c */
BOOL picture_mode = TRUE;
BOOL hide_markedfiles = FALSE;
BOOL mono_colours = FALSE;       /* monochrome display */

// tab width - set from TabWidth entry in registry
int g_tabwidth = TABWIDTH_DEFAULT;

BOOL TrackLeftOnly = TRUE;
BOOL TrackRightOnly = TRUE;
BOOL TrackSame = TRUE;
BOOL TrackDifferent = TRUE;
BOOL TrackReadonly = TRUE;

/* function prototypes ---------------------------------------------*/

BOOL InitApplication(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);
void CreateTools(void);
void DeleteTools(void);
INT_PTR APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL SetBusy(void);
void SetNotBusy(void);
void SetSelection(long rownr, long nrows, long dyRowsFromTop);
void SetButtonText(LPSTR cmd);
BOOL ToExpand(HWND hwnd);
void ParseArgs(char * lpCmdLine);
void Trace_Status(LPSTR str);

DWORD WINAPI wd_initial(LPVOID arg);

static HANDLE ghThread = NULL;
/* Some DOGMA about threads:
   When we spin off threads and then while they are still running, try to Exit
   we get race conditions with one thread allocating and the other freeing the
   storage. 
   It might be that given a final structure of A->B->C we have A->B with NULL
   pointers in B when the Exit comes in.  The cleanup thread will clear out
   B and A and THEN the worker thread might try to attach C to B which is no
   longer there.  This means that the worker thread must be Stopped.  To allow
   this to happen quickly, we TerminateThread it.  This will leave the initial
   stack around, but presumably that gets cleaned up on app exit anyway.
   There is only at most one worker thread running, and ghThread is its handle.
*/

static DWORD gdwMainThreadId;     /* threadid of main (user interface) thread
                                     initialised in winmain(), thereafter constant.
                                     See sdkdiff_UI()
                                  */

/* if you are about to put up a dialog box or in fact process input in any way
   on any thread other than the main thread - or if you MIGHT be on a thread other
   than the main thread, then you must call this function with TRUE before doing
   it and with FALSE immediately afterwards.  Otherwise you will get one of a
   number of flavours of not-very-responsiveness
*/
void sdkdiff_UI(BOOL bAttach)
{
    DWORD dwThreadId = GetCurrentThreadId();
    if (dwThreadId==gdwMainThreadId) return;

    if (bAttach) GetDesktopWindow();
    AttachThreadInput(dwThreadId, gdwMainThreadId, bAttach);
} /* sdkdiff_UI */

/*-functions----------------------------------------------------------*/

/* main entry point. register window classes, create windows,
 * parse command line arguments and then perform a message loop
 */


int WINAPI
WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow
        )
{
    MSG msg;
	int len = 0;

	// Get working directory.
	memset(pszWorkingDirectoryName, 0, MAX_PATH);
	len = GetModuleFileName(hInstance, pszWorkingDirectoryName, MAX_PATH);
	if (len != 0) {
		while (pszWorkingDirectoryName[len-1] != '\\' ) {
			len--;
		}
		pszWorkingDirectoryName[len] = 0;
	}	

    InitGutils(GetModuleHandle(NULL), DLL_PROCESS_ATTACH, NULL);

    gdwMainThreadId = GetCurrentThreadId();

    /* create any pens/brushes etc and read in profile defaults */
    CreateTools();

    /* init window class unless other instances running */
    if (!hPrevInstance)
        if (!InitApplication(hInstance))
            return(FALSE);


    /* init this instance - create all the windows */
    if (!InitInstance(hInstance, nCmdShow))
        return(FALSE);

    ParseArgs(lpCmdLine);

    /* message loop */
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(hwndClient, haccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    Trace_Close();     
    return (msg.wParam ? 1 : 0);
}

/* InitApplication
 *
 * - register window class for the main window and the bar window.
 */
BOOL
InitApplication(
                HINSTANCE hInstance
                )
{
    WNDCLASS    wc;
    BOOL resp;

    LCID lcid = GetThreadLocale();

    // set the boolean value for bJapan variable
    bJapan = (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE);
    bDBCS = ((PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_JAPANESE) ||
             (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_KOREAN)   ||
             (PRIMARYLANGID(LANGIDFROMLCID(lcid)) == LANG_CHINESE));

    /* register the bar window class */
    InitBarClass(hInstance);

    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC)MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(hInstance, szSdkDiff);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszClassName = (LPSTR) szSdkDiffViewerClass;
    wc.lpszMenuName = NULL;

    resp = RegisterClass(&wc);

    return(resp);
}

/*
 * create and show the windows
 */
BOOL
InitInstance(
             HINSTANCE hInstance,
             int nCmdShow
             )
{
    RECT rect;
    HANDLE hstatus;
    int bar_width;
    RECT childrc;
    HIGHCONTRAST hc;

    hInst = hInstance;

    /* initialise the list package */
    List_Init();


    hMenu = LoadMenu(hInstance, szSdkDiffMenu);
    haccel = LoadAccelerators(hInstance, szSdkDiffAccel);

    /* create the main window */
    hwndClient = CreateWindow(szSdkDiffViewerClass,
                              szSdkDiff,
                              WS_OVERLAPPEDWINDOW,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              CW_USEDEFAULT,
                              NULL,
                              hMenu,
                              hInstance,
                              NULL
                             );



    if (!hwndClient) {
        return(FALSE);
    }

    /* create 3 child windows, one status, one table and one bar
     * Initially, the bar window is hidden and covered by the table.
     */

    /* create a status bar window as
     * a child of the main window.
     */

    /* build a status struct for two labels and an abort button */
    hstatus = StatusAlloc(3);
    StatusAddItem(hstatus, 0, SF_STATIC, SF_LEFT|SF_VAR|SF_SZMIN, IDL_STATLAB, 14, NULL);
    StatusAddItem(hstatus, 1, SF_BUTTON, SF_RIGHT|SF_RAISE, IDM_ABORT, 8,
                LoadRcString(IDS_EXIT));
    StatusAddItem(hstatus, 2, SF_STATIC, SF_LOWER|SF_LEFT|SF_VAR,
                  IDL_NAMES, 60, NULL);

    /* ask the status bar how high it should be for the controls
     * we have chosen, and save this value for re-sizing.
     */
    status_height = StatusHeight(hstatus);

    /* create a window of this height */
    GetClientRect(hwndClient, &rect);
    childrc = rect;
    childrc.bottom = status_height;
    hwndStatus = StatusCreate(hInst, hwndClient, IDC_STATUS, &childrc,
                              hstatus);

    /* layout constants are stated as percentages of the window width */
    bar_width = (rect.right - rect.left) * BAR_WIN_WIDTH / 100;

    /* create the table class covering all the remaining part of
     * the main window
     */
    hwndRCD = CreateWindow(TableClassName,
                           NULL,
                           WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL,
                           0,
                           status_height,
                           (int)(rect.right - rect.left),
                           (int)(rect.bottom - status_height),
                           hwndClient,
                           (HMENU) IDC_RCDISP1,
                           hInst,
                           NULL);

    /* create a bar window as a child of the main window.
     * this window remains hidden until we switch into MODE_EXPAND
     */
    hwndBar = CreateWindow(szBarClass,
                           NULL,
                           WS_CHILD | WS_VISIBLE,
                           0,
                           status_height,
                           bar_width,
                           (int)(rect.bottom - status_height),
                           hwndClient,
                           (HMENU) IDC_BAR,
                           hInst,
                           NULL);

    /* nMinMax indicates whether we are to be minimised on startup,
     * on command line parameters
     */
    ShowWindow(hwndBar, SW_HIDE);

    if (GetProfileInt(APPNAME, szOutlineSaved, 0)) {
        WINDOWPLACEMENT wp;
        /* restore the previous expanded size and position */
        wp.length = sizeof(wp);
        wp.flags                   = 0;
        wp.showCmd                 = GetProfileInt( APPNAME, szOutlineShowCmd,
                                                    SW_SHOWNORMAL);
        wp.ptMaxPosition.x         = GetProfileInt( APPNAME, szOutlineMaxX,       0);
        wp.ptMaxPosition.y         = GetProfileInt( APPNAME, szOutlineMaxY,       0);
        wp.rcNormalPosition.left   = (int)GetProfileInt( APPNAME, szOutlineNormLeft,  (UINT)(-1));
        wp.rcNormalPosition.top    = (int)GetProfileInt( APPNAME, szOutlineNormTop,   (UINT)(-1));
        wp.rcNormalPosition.right  = (int)GetProfileInt( APPNAME, szOutlineNormRight, (UINT)(-1));
        wp.rcNormalPosition.bottom = (int)GetProfileInt( APPNAME, szOutlineNormBottom,(UINT)(-1));

        if (!SetWindowPlacement(hwndClient,&wp)) {
            ShowWindow(hwndClient, nMinMax);
        }
    } else ShowWindow(hwndClient, nMinMax);


    /* initialise busy flag and status line to show we are idle
     * (ie not comparing or scanning)
     */
    SetNotBusy();

    /* initialise the colour globals */
    hc.cbSize = sizeof(hc);
    SystemParametersInfo(SPI_GETHIGHCONTRAST,0 ,&hc, 0);
    mono_colours = (hc.dwFlags & HCF_HIGHCONTRASTON);
    if (mono_colours) {
        SetMonoColours();
    } else {
        SetColours();
    }
    PostMessage(hwndClient, WM_SYSCOLORCHANGE, 0 , 0);
    UpdateWindow(hwndClient);

    return(TRUE);

} /* InitInstance */



/*
 * complain to command line users about poor syntax
 */

typedef struct
{
    UINT m_ids;
    BOOL m_fInternalOnly;
    BOOL m_fExternalOnly;
    int m_cIndent;
} UsageStringInfo;

static const UsageStringInfo c_rg[] =
{
    { (UINT)-1,             0, 0, 0 },
    { IDS_USAGE_STR00,      0, 0, 0 },
    { IDS_USAGE_STR01,      0, 0, 0 },
    { IDS_USAGE_STR02,      0, 0, 1 },
    { IDS_USAGE_STR03,      0, 0, 1 },
    { IDS_USAGE_STR04,      0, 0, 3 },
    { IDS_USAGE_STR05,      0, 0, 1 },
    { IDS_USAGE_STR06,      0, 1, 1 },
    { IDS_USAGE_STR07,      1, 0, 1 },
    { IDS_USAGE_STR08,      1, 0, 1 },
    { IDS_USAGE_STR08B,     0, 0, 1 },
    { IDS_USAGE_STR09,      1, 0, 1 },
    { IDS_USAGE_STR10,      1, 0, 1 },
    { IDS_USAGE_STR11,      1, 0, 1 },
    { IDS_USAGE_STR12,      1, 0, 1 },
    { IDS_USAGE_STR12B,     1, 0, 1 },
    { IDS_USAGE_STR12C,     1, 0, 1 },
    { IDS_USAGE_STR13,      1, 0, 1 },
    { IDS_USAGE_STR14,      1, 0, 1 },
    { IDS_USAGE_STR15,      0, 0, 1 },
    { IDS_USAGE_STR16,      0, 0, 1 },
    { IDS_USAGE_STR17,      0, 0, 1 },
    { IDS_USAGE_STR18,      0, 0, 1 },
    { IDS_USAGE_STR19,      0, 0, 3 },
    { IDS_USAGE_STR20,      0, 0, 1 },
    { IDS_USAGE_STR21,      1, 0, 1 },
    { IDS_USAGE_STR22,      1, 0, 3 },
    { IDS_USAGE_STR23,      1, 0, 0 },
    { IDS_USAGE_STR24,      1, 0, -1 },
    { IDS_USAGE_STR25,      1, 0, -1 },
    { IDS_USAGE_STR26,      1, 0, -1 },
};

INT_PTR FAR PASCAL UsageDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        {
            HWND hwnd = GetDlgItem(hDlg, IDC_USAGE_TEXT);
            if (hwnd)
            {
                const UsageStringInfo *p;
                int c;
                PARAFORMAT pf;
                CHARFORMAT cf;
                int pos;

                pf.cbSize = sizeof(pf);

                SendMessage(hwnd, EM_SETBKGNDCOLOR, 0, GetSysColor(COLOR_BTNFACE));
                SendMessage(hwnd, EM_SETMARGINS, EC_LEFTMARGIN|EC_RIGHTMARGIN, MAKELONG(4, 4));

                for (p = c_rg, c = NUMELMS(c_rg); c--; p++)
                {
                    LPCSTR psz;
					psz=LoadRcString(p->m_ids);
                    if (p->m_fExternalOnly)
                        continue;
                    if (p->m_fInternalOnly) 
                        continue;

                    pos = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
                    SendMessage(hwnd, EM_REPLACESEL, FALSE, (LPARAM)psz);
                    SendMessage(hwnd, EM_SETSEL, pos, -1);

                    SendMessage(hwnd, EM_GETPARAFORMAT, 0, (LPARAM)&pf);
                    if (p->m_cIndent >= 0)
                    {
                        static const int c_rgIndents[] = { 320*1, 320*5, 320*6 };
                        static const int c_rgOffsets[] = { 320*4, 0, 0 };

                        pf.dwMask |= PFM_STARTINDENT|PFM_OFFSET|PFM_TABSTOPS;
                        pf.dxStartIndent = 0;
                        pf.dxOffset = 0;
                        if (p->m_cIndent)
                        {
                            pf.dxStartIndent = c_rgIndents[p->m_cIndent - 1];
                            pf.dxOffset = c_rgOffsets[p->m_cIndent - 1];
                        }
                        pf.cTabCount = 2;
                        pf.rgxTabs[0] = c_rgIndents[0];
                        pf.rgxTabs[1] = c_rgIndents[1];
                    }
                    else
                    {
                        pf.dwMask |= PFM_STARTINDENT|PFM_OFFSET|PFM_NUMBERING;
                        pf.dxStartIndent = 320;
                        pf.dxOffset = 180;
                        pf.wNumbering = PFN_BULLET;
                    }
                    SendMessage(hwnd, EM_SETPARAFORMAT, 0, (LPARAM)&pf);

                    SendMessage(hwnd, EM_SETSEL, (WPARAM)-1, -1);
                }

                cf.cbSize = sizeof(cf);
                cf.dwMask = CFM_COLOR;
                cf.dwEffects = 0;
                cf.crTextColor = GetSysColor(COLOR_BTNTEXT);
                SendMessage(hwnd, EM_SETSEL, 0, -1);
                SendMessage(hwnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                SendMessage(hwnd, EM_SETSEL, (WPARAM)-1, -1);
                pos = LOWORD(SendMessage(hwnd, EM_GETSEL, 0, 0));
                SendMessage(hwnd, EM_SETSEL, pos - 1, pos);
                SendMessage(hwnd, EM_REPLACESEL, 0, (LPARAM)"");
                SendMessage(hwnd, EM_SETSEL, 0, 0);
            }
        }
        break;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, wParam);
            break;
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

// --------------------------------------------------------------- //
// 
// Function: OurLoadLibrary
//
// Purpose: Load the Kernel32.dll into memory
//          Ensuring we load from the system32 directory
// 
// --------------------------------------------------------------- //
HMODULE OurLoadLibrary(LPSTR lpDll)
{
    HMODULE hLib = NULL;
    UINT uiRet;
    CHAR tcPath[MAX_PATH];
    HRESULT eRet;
    UINT uiSize;

    // Determine the length of our string
    uiSize = lstrlen(lpDll);

    if (0 != uiSize )
    {
        // Retrieve the location of the OS System Directory
        // since we will concatenate lpDll to the end of the system path, reserve the length of lpDll AND the null termination
        uiRet = GetSystemDirectory(tcPath, MAX_PATH - uiSize);

        // if the return value is 0 OR will take up too much space then fail
        if ((0 != uiRet) && (uiRet < (MAX_PATH - uiSize)) )
        {
            // Concatenate the library to the system path
            eRet = StringCchCat(tcPath, MAX_PATH, lpDll);

            // Did the concatenation of the library to the path succeed??
            if (SUCCEEDED(eRet))
            {
                hLib = LoadLibrary(tcPath);
            }
        }
    }

    return hLib;

}
void
sdkdiff_usage(
              LPSTR msg
              )
{
    INT_PTR retval;
    UINT fuStyle =  MB_ICONINFORMATION|MB_OKCANCEL;
	HRESULT hr;


    if (NULL != msg)
    {
        char Usage[4096];

        // since msg may be a pointer returned by LoadRcString, copy it off so
        // we don't stomp on the string when we load IDS_SDKDIFF_USAGE.
        hr = StringCchCopy(Usage, 4096, msg);
		if (FAILED(hr))
        {
			OutputError(hr, IDS_SAFE_COPY);
        }
        msg = Usage;

        retval = MessageBox(hwndClient,
                            msg,
                            LoadRcString(IDS_SDKDIFF_USAGE),
                            fuStyle);
    }
    else
    {
        HINSTANCE h;

        h = OurLoadLibrary("\\riched20.dll");
        if (NULL == h)
        {
            h = OurLoadLibrary("\\riched32.dll");
        }
        if (NULL != h)
        {
            retval = DialogBox(hInst, (LPCSTR)IDD_USAGE, hwndClient, UsageDlgProc);
            FreeLibrary(h);
        }
        else
        {
            sdkdiff_usage(LoadRcString(IDS_ERROR_CANTLOADRICHEDIT));
            retval = IDOK;
        }
    }

    if (retval == IDCANCEL)
        exit(1);
}


/*  Functionally similar to strtok except that " ... " is a token, even if
    it contains spaces and the delimiters are built in (no second parameter)
    GetNextToken(foo) delivers the first token in foo (or NULL if foo is empty)
    and caches foo so that GetNextToken(NULL) then gives the next token.  When there
    are no more tokens left it returns NULL
    It mangles the original by peppering it with NULLs as it chops the tokens
    off.  Each time except the last it inserts a new NULL.
    Obviously not thread safe!
    Command line is limited to 512 chars.

*/
char *
GetNextToken(
             char * Tok
             )
{
    static char * Source;     // The address of the original source string
                              // which gets progressively mangled

    static char RetBuff[512]; // We will build results in here
    static char *Ret;         // We build the results here (in RetBuff)
                              // but moved along each time.

    static char * p;       // the next char to parse in Source
                           // NULL if none left.
	HRESULT hr;

    // Quotes are a nuisance (they are the whole reason why strtok
    // wouldn't work).  If the string starts with quotes then we potentially
    // need to pull together fragments of the string "foo""ba"r => foobar
    // We want to pull these together into storage that we can safely write
    // into and return (can't be stack).  Mangling the original parameter
    // gets very messy so we cache a pointer to the original source that
    // we work through and we build up output tokens in a static
    // and therefore permanently wasted buffer of (arbitrarily) 512 bytes.
    // then we can set Ret to \0 and concatenate bits on as we find them.
    // The rule is that we split at the first space outside quotes.


    // cache the Source if a "first time" call.  Kill the "finished" case.
    if (Tok!=NULL) {
        Source = Tok;
        Ret = RetBuff;
        RetBuff[0] = '\0';
        p = Source;
    } else if (p==NULL) {
        return NULL;          // finished
    } else {
        Ret +=strlen(Ret)+1;  // slide it past last time's stuff
    }

    *Ret = '\0';              // empty string to concatenate onto

    // from here on Tok is used as a temporary.

    // keep taking sections and adding them to the start of Source
    for (; ; ) {

        // for each possibility we grow Ret and move p on.
        if (*p=='\"') {
            ++p;
            Tok = My_mbschr(p, '"');
            if (Tok==NULL) {
                hr = StringCchCat(Ret, 512, p);
				if (FAILED(hr)) {
					OutputError(hr, IDS_SAFE_CAT);
					return(NULL);
				}
                p = NULL;
                return Ret;
            } else {
                *Tok = '\0';    // split the section off, replaceing the "
                hr = StringCchCat(Ret, 512, p); // add it to the result
				if (FAILED(hr)) {
					OutputError(hr, IDS_SAFE_CAT);
					return (NULL);
				}
                p = Tok+1;      // move past the quote
            }
        } else {
            int i = (int)strcspn(p," \"");   // search for space or quote
            if (p[i]=='\0') {
                // It's fallen off the end
                hr = StringCchCat(Ret, 512, p);
				if (FAILED(hr)) {
					OutputError(hr, IDS_SAFE_CAT);
					return (NULL);
				}
                p = NULL;
                return Ret;
            } else if (p[i]==' ') {
                // We've hit a genuine delimiting space
                p[i] = '\0';
                hr = StringCchCat(Ret,512, p);
				if (FAILED(hr)) {
					OutputError(hr, IDS_SAFE_CAT);
					return (NULL);
				}
                p +=i+1;

                // strip trailing spaces (leading spaces for next time)
                while (*p==' ')
                    ++p;
                if (*p=='\0')
                    p = NULL;

                return Ret;
            } else {
                // we've hit a quote
                p[i] = '\0';
                hr = StringCchCat(Ret, 512, p);
				if (FAILED(hr)) {
					OutputError(hr, IDS_SAFE_CAT);
					return (NULL);
				}
                p[i] = '\"';     // put it back so that we can find it again
                p +=i;           // aim at it and iterate
            }
        }

    } // for

} // GetNextToken





/*
 * parse command line arguments
 *
 * The user can give one or two paths. if only one, we assume the second
 * is '.' for the current directory. if one of the two paths is a directory
 * and the other a file, we compare a file of the same name in the two dirs.
 *
 * the command -s filename causes the outline list to be written to a file
 * -s{slrd} filename allows selection of which files are written out;
 * by default, we assume -sld for files left and different.
 * -s{slrd}x causes the program to exit after the list has been written out
 *
 *
 * -T means tree.  Go deep.
 * -D means Directory or Don't go deep.
 * -O means Stay in outline mode.  No auto expand.
 *
 */
void
ParseArgs(
          char * lpCmdLine
          )
{
    PTHREADARGS ta;
 
    BOOL fAllowTwoPaths = TRUE;         // FALSE means -l or -lr was used
    BOOL fReverse = FALSE;              // -lr means reverse
    BOOL fDeepDefault = TRUE;
    char * tok;         /* token from lpCmdLine */

    DWORD threadid;
    UINT idsError = 0;

    /* thread args can't be on the stack since the stack will change
     * before the thread completes execution
     */
    ta = (PTHREADARGS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(THREADARGS));
    if (ta == NULL)
		return;

	ta->first = NULL;
    ta->second = NULL;
    ta->savelist = NULL;
    ta->savecomp = NULL;
    ta->listopts = 0;
    ta->compopts = 0;
    ta->notify = NULL;
    ta->fExit = FALSE;
    ta->fDeep = FALSE;  /* No -T option seen yet */

    tok = GetNextToken(lpCmdLine);

    while ((tok!=NULL) && (lstrlen(tok) > 0)) {

        if (tok[0] == '/' && tok[1] == '/')
        {
            goto LFile;
        }

        /* is this an option ? */
        if ((tok[0] == '-') || (tok[0] == '/')) {
            switch (tok[1]) {
                case 's':
                case 'S':
                    /* read letters for the save option: s,l,r,d */
                    for (tok+=2; *tok != '\0'; ++tok) {
                        switch (*tok) {
                            case 's':
                            case 'S':
                                ta->listopts |= INCLUDE_SAME;
                                break;
                            case 'l':
                            case 'L':
                                ta->listopts |= INCLUDE_LEFTONLY;
                                break;
                            case 'r':
                            case 'R':
                                ta->listopts |= INCLUDE_RIGHTONLY;
                                break;
                            case 'd':
                            case 'D':
                                ta->listopts |= INCLUDE_DIFFER;
                                break;
                            case 'x':
                            case 'X':
                                ta->fExit = TRUE;
                                break;
                            default:
                                idsError = 0;
                                goto LUsage;
                        }
                    }

                    if (ta->listopts == 0) {
                        /* default to left and differ */
                        ta->listopts = (INCLUDE_LEFTONLY) | (INCLUDE_DIFFER);
                    }
                    ta->savelist = GetNextToken(NULL);
                    break;
                case 'f':
                case 'F':
					/* read letters for the save option: s,l,r,d,e,i */
                    for(tok = &tok[2]; *tok != '\0'; ++tok) {
                        switch(*tok) {
                        case 'i':
                        case 'I':
                                ta->compopts |= INCLUDE_SAME;
                                break;
                        case 'l':
                        case 'L':
                                ta->compopts |= INCLUDE_LEFTONLY;
                                break;
                        case 'r':
                        case 'R':
                                ta->compopts |= INCLUDE_RIGHTONLY;
                                break;
                        case 'f':
                        case 'F':
                                ta->compopts |= INCLUDE_MOVEDLEFT;
                                break;
                        case 'g':
                        case 'G':
                                ta->compopts |= INCLUDE_MOVEDRIGHT;
                                break;
                        case 's':
                        case 'S':
                                ta->compopts |= INCLUDE_SIMILARLEFT;
                                break;
                        case 'a':
                        case 'A':
                                ta->compopts |= INCLUDE_SIMILARRIGHT;
                                break;
                        case 'x':
                        case 'X':
                                ta->fExit = TRUE;
                                break;
                        default:
                                idsError = 0;
                                goto LUsage;
                        }
                    }

                    if (ta->compopts == 0) {
                            /* default to showing all diffs (everything but same) */
                            ta->compopts = INCLUDE_LEFTONLY | INCLUDE_RIGHTONLY
                                         | INCLUDE_DIFFER | INCLUDE_MOVEDLEFT
                                         | INCLUDE_MOVEDRIGHT;
                    }
                    ta->savecomp = GetNextToken(NULL);
                    break;
                case 'i':
                case 'I':
                    ta->fInputFile = TRUE;
                    if (tok[2] == '1')
                    {
                        ta->fInputFileSingle = TRUE;
                        tok++;
                    }
                    tok += 2;
                    // allow (but don't require) a space between '-I' and the
                    // 'inputfilename' argument.
                    if (!*tok)
                        tok = GetNextToken(NULL);
                    if (!tok || !*tok)
                    {
                        idsError = IDS_ERROR_IARGS;
                        goto LUsage;
                    }
                    break;
                case 'n':
                case 'N':
                    ta->notify = GetNextToken(NULL);
                    break;
                case 't':
                case 'T':
                    ta->fDeep = TRUE;
                    break;
                case 'd':
                case 'D':
                    ta->fDeep = FALSE;     // This directory only
                    fDeepDefault = FALSE;
                    break;
                case 'o':
                case 'O':
                    fAutoExpand = FALSE;
                    break;
                case 'p':
                case 'P':
                    gbPerverseCompare = TRUE;
                    break;
                case 'x':
                case 'X':
                    {
                        BOOL track;
                        char *c = &tok[2];
                        if (*c == '\0') {
                            TrackLeftOnly = FALSE;
                            TrackRightOnly = FALSE;
                            TrackSame = FALSE;
                            TrackDifferent = TRUE;
                            TrackReadonly = TRUE;
                            break;
                        }
                        track = FALSE;
                        while (*c != '\0') {
                            if (toupper(*c) == 'L') {
                                TrackLeftOnly = track;
                                track = FALSE;
                            } else if (toupper(*c) == 'R') {
                                TrackRightOnly = track;
                                track = FALSE;
                            } else if (toupper(*c) == 'S') {
                                TrackSame = track;
                                track = FALSE;
                            } else if (toupper(*c) == 'D') {
                                TrackDifferent = track;
                                track = FALSE;
                            } else if (toupper(*c) == 'O') {
                                TrackReadonly = track;
                                track = FALSE;
                            } else if (toupper(*c) == 'I') {
                                track = FALSE;
                            } else if (toupper(*c) == '-') {
                                track = TRUE;
                            } else {
                                idsError = 0;
                                goto LUsage;
                            }
                            c++;
                        }
                        break;
                    }

#ifdef DEBUG
                case '!':
                    DebugBreak();
                    break;
#endif

                case '?':
                    {
                        int j = 0;
                        sdkdiff_usage(NULL);
                        for (++tok; tok[1] != '\0'; ++tok)
                            if ('?'==*tok) ++j;

                        if (2==j) {
                            WriteProfileString(APPNAME, "SYSUK", "1");
                        }
                        return;
                    }
                default:
                    idsError = 0;
                    goto LUsage;
            }
        } else {
        LFile:
            if (ta->first == NULL) {
                ta->first = tok;
            } else {
                ta->second = tok;
            }
        }
        tok = GetNextToken(NULL);
    }

    if (ta->fInputFile && ta->first)
    {
        idsError = IDS_ERROR_IARGS;
        goto LUsage;
    }

    /* set the correct depth */
    if (ta->fDeep)
        ;                       /* explicitly set -- leave it alone */
   
    else ta->fDeep = fDeepDefault;  /* global default */

    if (!ta->fInputFile)
    {
        /* any paths to scan ? */
        if (ta->first == NULL)
            return;

        if (ta->second == NULL)
            ta->second = ".";
    }
    else
    {
        ta->fDeep = FALSE;
    }

    SetBusy();

    /* minimise the window if -s flag given */
    if (ta->savelist != NULL || ta->savecomp != NULL) {
        ShowWindow(hwndClient, SW_MINIMIZE);
    }

    /* make an empty view */
    current_view = view_new(hwndRCD);
    DisplayMode = MODE_OUTLINE;

    ta->view = current_view;

    /* attempt to create a worker thread */

    ghThread = CreateThread(NULL, 0, wd_initial, (LPVOID) ta,
                            0, &threadid);
    if (ghThread == NULL)
	{
        wd_initial( (LPVOID) ta);
    }

    return;

LUsage:
    sdkdiff_usage(idsError ? LoadRcString(idsError) : NULL);
} /* ParseArgs */


void
GetFontPref(void)
{
    DeleteObject(g_hFont);
    g_hFont = 0;

    GetProfileString(APPNAME, szFontFaceName, "FixedSys", g_szFontFaceName, sizeof(g_szFontFaceName));
    g_nFontHeight = GetProfileInt(APPNAME, szFontHeight, 12);
    g_fFontBold = GetProfileInt(APPNAME, szFontBold, FALSE);
    g_bFontCharSet = (BYTE)GetProfileInt(APPNAME, szFontCharSet, 0);

    g_hFont = CreateFont(g_nFontHeight, 0, 0, 0,
                         g_fFontBold ? FW_BOLD : FW_DONTCARE,
                         FALSE, FALSE, FALSE,
                         g_bFontCharSet,
                         OUT_DEFAULT_PRECIS,
                         CLIP_LH_ANGLES|CLIP_STROKE_PRECIS,
                         DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
                         g_szFontFaceName);
}


/* create any pens/brushes, and read defaults
 * from the profile file for menu settings etc.
 */
void
CreateTools(void)
{

    /* standard message that table class sends us for
     * notifications and queries.
     */
    table_msgcode = RegisterWindowMessage(TableMessage);

    line_numbers = GetProfileInt(APPNAME, szLineNumbers, line_numbers);
    outline_include = GetProfileInt(APPNAME, szFileInclude, outline_include);
    expand_include = GetProfileInt(APPNAME, szLineInclude, expand_include);
    ignore_blanks = GetProfileInt(APPNAME, szBlanks, ignore_blanks);
    Algorithm2 = GetProfileInt(APPNAME, szAlgorithm2, Algorithm2);
    mono_colours = GetProfileInt(APPNAME, szMonoColours, mono_colours);
    picture_mode = GetProfileInt(APPNAME, szPicture, picture_mode);
    hide_markedfiles = GetProfileInt(APPNAME, szHideMark, hide_markedfiles);

    GetProfileString(APPNAME, szEditor, editor_cmdline, editor_cmdline,
                     sizeof(editor_cmdline));

    g_tabwidth = GetProfileInt(APPNAME, szTabWidth, g_tabwidth);

    InitializeCriticalSection(&CSSdkdiff);

    GetFontPref();
}

/* delete any pens or brushes that were created in CreateTools */
void
DeleteTools(void)
{
    DeleteCriticalSection(&CSSdkdiff);
    DeleteObject(g_hFont);
}


/* check for messages to keep the UI working. Also check whether
 * we have had an abort request (IDM_ABORT), and
 * return TRUE if abort requested, otherwise FALSE
 */
BOOL
Poll(void)
{
    MSG msg;

    /* don't do the message loop in the WIN32 version since we
     * have multiple threads to handle that, and this is being called
     * on a worker thread, not the UI thread.
     *
     * in the WIN32 case, just check for abort requests
     */

    /* message loop */
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (!TranslateAccelerator(hwndClient, haccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return(bAbort);
}

/* position child windows on a resize of the main window */

void
DoResize(
         HWND hWnd
         )
{
    RECT rc;
    int bar_width;

    GetClientRect(hWnd, &rc);
    MoveWindow(hwndStatus, 0, 0, rc.right - rc.left, status_height, TRUE);

    bar_width = (rc.right - rc.left) * BAR_WIN_WIDTH / 100;

    /* bar window is hidden unless in expand mode */
    if ((DisplayMode == MODE_EXPAND) && (picture_mode)) {
        MoveWindow(hwndBar, 0, status_height,
                   bar_width, rc.bottom - status_height, TRUE);
        MoveWindow(hwndRCD, bar_width, status_height,
                   (rc.right - rc.left) - bar_width,
                   rc.bottom - status_height, TRUE);
        ShowWindow(hwndBar, SW_SHOW);
    } else {
        MoveWindow(hwndRCD, 0, status_height, (rc.right - rc.left),
                   rc.bottom - status_height, TRUE);
        ShowWindow(hwndBar, SW_HIDE);
    }

}

INT_PTR
APIENTRY
AboutBox(
         HWND hDlg,
         unsigned message,
         WPARAM wParam,
         LPARAM lParam
         )
{
    char ch[256];
	HRESULT hr;

    switch (message) {

        case WM_INITDIALOG:
            hr = StringCchPrintf(ch, 256, "%d.%02d", Version, SubVersion);
			if (FAILED(hr))
				OutputError(hr, IDS_SAFE_PRINTF);
            SetDlgItemText(hDlg, IDD_VERSION, ch);
            return(TRUE);

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDOK:
                    EndDialog(hDlg, 0);
                    return(TRUE);
            }
            break;
    }
    return(FALSE);
}


/* -- menu commands ---------------------------------------------------*/

/* print the current view */
void
DoPrint(void)
{
    Title head, foot;
    PrintContext context;
    TCHAR szPage[50];
    TCHAR szTitle[20];
	HRESULT hr;

    /* print context contains the header and footer. Use the
     * default margins and printer selection
     */

    /* we set the table id to be TABID_PRINTER. When the table calls
     * back to get text and properties, we use this to indicate
     * that the table refered to is the 'current_view', but in print
     * mode, and thus we will use different colours/fonts.
     */
    context.head = &head;
    context.foot = &foot;
    context.margin = NULL;
    context.pd = NULL;
    context.id = TABID_PRINTER;

    /* header is filenames or just SdkDiff if no names known*/
    if (strlen(AppTitle) > 0) {
        head.ptext = AppTitle;
    } else {
        head.ptext = (LPSTR)szSdkDiff;
    }

    /* header is centred, footer is right-aligned and
     * consists of the page number
     */
    head.props.valid = P_ALIGN;
    head.props.alignment = P_CENTRE;
    hr = StringCchCopy(szPage, 50, LoadRcString(IDS_PAGE));
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_COPY);
    foot.ptext = (LPSTR)szPage;
    foot.props.valid = P_ALIGN;
    foot.props.alignment = P_RIGHT;

    if ( SendMessage(hwndRCD, TM_PRINT, 0, (LPARAM) &context)) {
        Trace_Status(LoadRcString(IDS_SENT_TO_PRINTER));
    } else {
        sdkdiff_UI(TRUE);
        hr = StringCchCopy(szTitle, 20, LoadRcString(IDS_SDKDIFF_ERROR));
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY);
		MessageBox(hwndClient, LoadRcString(IDS_UNABLE_TO_PRINT),
                szTitle, MB_ICONEXCLAMATION);
        sdkdiff_UI(FALSE);
    }
}

/* find the next line in the current view that is
 * not STATE_SAME. Start from the current selection, if valid, or
 * from the top of the window if no selection.
 *
 */
#define FindNextChange()    _FindNextChange(TRUE, TRUE)
#define FindPrevChange()    _FindPrevChange(TRUE, TRUE)

BOOL
_FindNextChange(BOOL fErrorPopup, BOOL fForceAutoCenter)
{
    long row;
    long dyRowsFromTop = -1;
    long top = (int)SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);

    /* start from the selection or top of the window if no selection */
    if (selection >= 0)
    {
        row = selection + 1;
        if (!fForceAutoCenter)
            dyRowsFromTop = selection - top;
    }
    else
    {
        row = top;
    }


    /* find the next 'interesting' line */
    row = view_findchange(current_view, row, TRUE);
    if (row >= 0) {
        SetSelection(row, 1, dyRowsFromTop);
        return(TRUE);
    } else if (fErrorPopup) {
        sdkdiff_UI(TRUE);
        MessageBox(hwndClient, LoadRcString(IDS_NO_MORE_CHANGES), szSdkDiff,
                   MB_ICONINFORMATION|MB_OK);
        sdkdiff_UI(FALSE);

    }
    return(FALSE);
}

/* find the previous line in the current view that is not STATE_SAME
 */
BOOL
_FindPrevChange(BOOL fErrorPopup, BOOL fForceAutoCenter)
{
    long row;
    long dyRowsFromTop = -1;
    long top = (int)SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);

    /* start from the selection or top of window if no selection */
    if (selection >= 0)
    {
        row = selection - 1;
        if (!fForceAutoCenter)
            dyRowsFromTop = selection - top;
    }
    else
    {
        row = top;
    }

    /* find the previous 'interesting' line */
    row = view_findchange(current_view, row, FALSE);
    if (row >= 0) {
        SetSelection(row, 1, dyRowsFromTop);
        return(TRUE);
    } else if (fErrorPopup) {
        sdkdiff_UI(TRUE);
        MessageBox(hwndClient, LoadRcString(IDS_NO_PREV_CHANGES), szSdkDiff,
                   MB_ICONINFORMATION|MB_OK);
        sdkdiff_UI(FALSE);

    }
    return(FALSE);
}

#ifndef WriteProfileInt // Only needed if the profile->registry
// mapping is not in use
BOOL
WriteProfileInt(
                LPSTR AppName,
                LPSTR Key,
                int Int
                )
{       char Str[40];
    HRESULT hr = StringCchPrintf(Str, 40, szD, Int);
	if (FAILED(hr))
		OutputError(hr, IDS_SAFE_PRINTF);
    return WriteProfileString(AppName, Key, Str);

} /* WriteProfileInt */
#endif


/* switch to expand view of the selected line */
BOOL
ToExpand(
         HWND hwnd
         )
{


    if (selection < 0) {
        return(FALSE);
    }

    // nothing to do if already expanded
    if (view_isexpanded(current_view)) {
        return(FALSE);
    }

    /*
     * note that we are starting expansion
     */
    view_expandstart(current_view);


    if (!view_isexpanded(current_view)) {
        /* save the current outline size and position */
        WINDOWPLACEMENT wp;

        wp.length = sizeof(wp);

        if (GetWindowPlacement(hwndClient,&wp)) {
            WriteProfileInt(APPNAME, szOutlineShowCmd, wp.showCmd);
            WriteProfileInt(APPNAME, szOutlineMaxX, wp.ptMaxPosition.x);
            WriteProfileInt(APPNAME, szOutlineMaxY, wp.ptMaxPosition.y);
            WriteProfileInt(APPNAME, szOutlineNormLeft, wp.rcNormalPosition.left);
            WriteProfileInt(APPNAME, szOutlineNormTop, wp.rcNormalPosition.top);
            WriteProfileInt(APPNAME, szOutlineNormRight, wp.rcNormalPosition.right);
            WriteProfileInt(APPNAME, szOutlineNormBottom, wp.rcNormalPosition.bottom);
            WriteProfileInt(APPNAME, szOutlineSaved, 1);
        }

        /* restore the previous expanded size and position, if any */
        if (GetProfileInt(APPNAME, szExpandedSaved, 0)) {
            wp.flags                   = 0;
            wp.showCmd
            = GetProfileInt( APPNAME, szExpandShowCmd
                             , SW_SHOWMAXIMIZED);
            wp.ptMaxPosition.x
            = GetProfileInt( APPNAME, szExpandMaxX, 0);
            wp.ptMaxPosition.y
            = GetProfileInt( APPNAME, szExpandMaxY, 0);
            wp.rcNormalPosition.left
            = GetProfileInt( APPNAME, szExpandNormLeft
                             , wp.rcNormalPosition.left);
            wp.rcNormalPosition.top
            = GetProfileInt( APPNAME, szExpandNormTop
                             , wp.rcNormalPosition.top);
            wp.rcNormalPosition.right
            = GetProfileInt( APPNAME, szExpandNormRight
                             , wp.rcNormalPosition.right);
            wp.rcNormalPosition.bottom
            = GetProfileInt( APPNAME, szExpandNormBottom
                             , wp.rcNormalPosition.bottom);
            SetWindowPlacement(hwndClient,&wp);
        } else ShowWindow(hwndClient, SW_SHOWMAXIMIZED);
    }

    /*change the view mapping to expand mode */
    if (view_expand(current_view, selection)) {

        /* ok - we now have an expanded view - change status
         * to show this
         */

        DisplayMode = MODE_EXPAND;

        /* resize to show the graphic bar picture */
        DoResize(hwndClient);


        /* change button,status text-if we are not still busy*/
        if (!fBusy) {
            TCHAR szBuf[10];
            /* the status field when we are expanded shows the
             * tag field (normally the file name) for the
             * item we are expanding
             */
            SetStatus(view_getcurrenttag(current_view) );
            HRESULT hr = StringCchCopy(szBuf, 10, LoadRcString(IDS_OUTLINE));
			if (FAILED(hr))
				OutputError(hr, IDS_SAFE_COPY);
            SetButtonText(szBuf);
        }

        // Skip to the first change.  (But don't do this if the first
        // line of the file itself has changed; otherwise we would skip
        // over it!)
        if (view_getrowstate(current_view, 0) == STATE_SAME) {
            _FindNextChange(FALSE, TRUE);
        }

        return(TRUE);
    }
    return(FALSE);
} /* ToExpand */

/* switch back to outline view - showing just the list of file names.
 */
void
ToOutline(
          HWND hwnd
          )
{
    // if current_view is NULL, don't access it with view_xxx functions.
    if (!current_view)
        return;

    /*
     * if we are in the middle of expanding, ignore the
     * key stroke - user can try again later
     */
    if (view_expanding(current_view)) {
        return;
    }

    if (view_isexpanded(current_view)) {
        /* save the current expanded size and position */
        WINDOWPLACEMENT wp;

        wp.length = sizeof(wp);
        if (GetWindowPlacement(hwndClient,&wp)) {
            WriteProfileInt(APPNAME, szExpandShowCmd, wp.showCmd);
            WriteProfileInt(APPNAME, szExpandMaxX, wp.ptMaxPosition.x);
            WriteProfileInt(APPNAME, szExpandMaxY, wp.ptMaxPosition.y);
            WriteProfileInt(APPNAME, szExpandNormLeft, wp.rcNormalPosition.left);
            WriteProfileInt(APPNAME, szExpandNormTop, wp.rcNormalPosition.top);
            WriteProfileInt(APPNAME, szExpandNormRight, wp.rcNormalPosition.right);
            WriteProfileInt(APPNAME, szExpandNormBottom, wp.rcNormalPosition.bottom);
            WriteProfileInt(APPNAME, szExpandedSaved, 1);
        }

        /* restore the previous expanded size and position, if any */
        if (GetProfileInt(APPNAME, szOutlineSaved, 0)) {
            wp.flags = 0;
            wp.showCmd
            = GetProfileInt( APPNAME, szOutlineShowCmd
                             , SW_SHOWNORMAL);
            wp.ptMaxPosition.x
            = GetProfileInt( APPNAME, szOutlineMaxX, 0);
            wp.ptMaxPosition.y
            = GetProfileInt( APPNAME, szOutlineMaxY, 0);
            wp.rcNormalPosition.left
            = GetProfileInt( APPNAME, szOutlineNormLeft
                             , wp.rcNormalPosition.left);
            wp.rcNormalPosition.top
            = GetProfileInt( APPNAME, szOutlineNormTop
                             , wp.rcNormalPosition.top);
            wp.rcNormalPosition.right
            = GetProfileInt( APPNAME, szOutlineNormRight
                             , wp.rcNormalPosition.right);
            wp.rcNormalPosition.bottom
            = GetProfileInt( APPNAME, szOutlineNormBottom
                             , wp.rcNormalPosition.bottom);
            SetWindowPlacement(hwndClient,&wp);
        } else {
            ShowWindow(hwndClient, SW_SHOWNORMAL);
        }
    }

    DisplayMode = MODE_OUTLINE;

    /* switch mapping back to outline view */
    view_outline(current_view);

    /* hide bar window and resize to cover */
    DoResize(hwndClient);


    /* change label on button */
    if (!fBusy) {
        TCHAR szBuf[8];
        HRESULT hr = StringCchCopy(szBuf, 8, LoadRcString(IDS_EXPAND));
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY);
        SetButtonText(szBuf);
        SetStatus(NULL);
    }
} /* ToOutline */

/*
 * if the user clicks on a MOVED line in expand mode, we jump to the
 * other line. We return TRUE if this was possible,  or FALSE otherwise.
 * If bMove is not true, then just test to see if it is possible to move
 * and don't actually make the selection change. (I was going to have
 * an IsMoved function but there seemed to be so much in common).
 */
BOOL
ToMoved(
        HWND hwnd,
        BOOL bMove
        )
{
    BOOL bIsLeft;
    int linenr, state;
    long i, total;

    if (DisplayMode != MODE_EXPAND) {
        return(FALSE);
    }
    if (selection < 0) {
        return(FALSE);
    }

    state = view_getstate(current_view, selection);
    if (state == STATE_MOVEDLEFT || state == STATE_SIMILARLEFT) {
        bIsLeft = TRUE;
        /* get the linenr of the other copy */
        linenr = abs(view_getlinenr_right(current_view, selection));
    } else if (state == STATE_MOVEDRIGHT || state == STATE_SIMILARRIGHT) {
        bIsLeft = FALSE;
        /* get the linenr of the other copy */
        linenr = abs(view_getlinenr_left(current_view, selection));
    } else {
        /* not a moved line - so we can't find another copy */
        return(FALSE);
    }

    /* search the view for this line nr */
    total = view_getrowcount(current_view);
    for (i = 0; i < total; i++) {
        if (bIsLeft) {
            if (linenr == view_getlinenr_right(current_view, i)) {
                /* found it */
                if (bMove) {
                    SetSelection(i, 1, -1);
                }
                return(TRUE);
            }
        } else {
            if (linenr == view_getlinenr_left(current_view, i)) {
                if (bMove) {
                    SetSelection(i, 1, -1);
                }
                return(TRUE);
            }
        }
    }
    return(FALSE);
} /* ToMoved */


void
RescanFile(
           HWND hwnd
           )
{
    COMPITEM ci=NULL;
    int iStart;
    int iEnd;
    int i;

    if (selection_nrows > 0 || DisplayMode == MODE_EXPAND) {
        if (DisplayMode == MODE_EXPAND) {
            iStart = 0;
            iEnd = 1;
        } else {
            iStart = selection;
            iEnd = iStart + selection_nrows;
        }
        for (i = iStart; i < iEnd; i++) {
            ci = view_getitem(current_view, i);
            if (ci != NULL) {
                compitem_rescan(ci);
            }
        }
    } else {
        sdkdiff_UI(TRUE);
        MessageBox(hwndClient, LoadRcString(IDS_NOTHING_RESCANNED),
                   szSdkDiff, MB_ICONSTOP|MB_OK);
        sdkdiff_UI(FALSE);

        return;
    }
    PostMessage(hwndClient, WM_COMMAND, IDM_UPDATE, (LPARAM) ci);
} /* RescanFile */


/*
 * launch an editor on the current file (the file we are expanding, or
 * in outline mode the selected row. Option allows selection of the
 * left file, the right file or the composite view of this item.
 * pe points to a packet of parameters that must be freed before returning.
 * The return value is meaningless (just to conform to CreateThread).
 */
LONG WINAPI
do_editfile(
            PEDITARGS pe
            )
{
    VIEW view = pe->view;
    int option = pe->option;
    long L_selection = pe->selection;

    COMPITEM item;
    LPSTR fname;
    char cmdline[MAX_PATH] = {'\0'};
    long selline, currentline;
    char * pOut = cmdline;
    char * pIn = editor_cmdline;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	HRESULT hr;

    item = view_getitem(view, L_selection);
    if (item == NULL) {
        sdkdiff_UI(TRUE);
        MessageBox(hwndClient, LoadRcString(IDS_NOTHING_TO_EDIT),
                   szSdkDiff, MB_ICONSTOP|MB_OK);
        sdkdiff_UI(FALSE);

        return -1;
    }

    fname = compitem_getfilename(view, item, option);

    if ( 0 == fname ) {
        sdkdiff_UI(TRUE);
        MessageBox(hwndClient, LoadRcString(IDS_FILE_DOESNT_EXIST),
                   szSdkDiff, MB_ICONSTOP|MB_OK);
        sdkdiff_UI(FALSE);
        goto error;
    }

    // convert the selected line into a line number within the file
    if (L_selection > 0) {
        selline = L_selection;
    } else {
        // if no current selection, look for the line at top of window
        selline = (long) SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);
    }

    switch ( option ) {
        case CI_LEFT:
            do {
                currentline = view_getlinenr_left( view, selline);

                // if the selected line is not in the left file,
                // backup one line and try again until we hit the top of
                // file or find a line that is within the left file

                if (selline > 0) {
                    selline--;
                }
            } while ((currentline <= 0) && (selline > 0));

            break;

        case CI_RIGHT:
            do {
                currentline = view_getlinenr_right( view, selline);
                if (selline > 0) {
                    selline--;
                }
            } while ((currentline <= 0) && (selline > 0));
            break;

        default:
            currentline = 1;
            break;
    }

    if (currentline <=0) {
        currentline = 1;
    }


    while ( *pIn ) {
        switch ( *pIn ) {
            case '%':
                pIn++;
                switch ( *pIn ) {
                    case 'p':
                        hr = StringCchCopy( pOut, MAX_PATH - (cmdline - pOut), fname);
						if (FAILED(hr)) 
                        {
							OutputError(hr, IDS_SAFE_COPY);
                        }
                        while ( *pOut )
                        {
                            pOut++;
                        }
                        break;

                    case 'l':
                        hr = StringCchPrintf(pOut, MAX_PATH - (cmdline - pOut),  "%d",currentline);
                        if (FAILED(hr))
                        {
                            goto error;
                        }
                        while ( *pOut )
                        {
                            pOut++;
                        }
                        break;

                    default:
                        if (IsDBCSLeadByte(*pIn) && *(pIn+1)) {
                            *pOut++ = *pIn++;
                        }
                        *pOut++ = *pIn;
                        break;
                }
                pIn++;
                break;

            default:
                if (IsDBCSLeadByte(*pIn) && *(pIn+1)) {
                    *pOut++ = *pIn++;
                }
                *pOut++ = *pIn++;
                break;
        }
    }

    *pOut = '\0';

    si.lpTitle = "Edit File";
    si.cb = sizeof(STARTUPINFO);
    si.lpReserved = NULL;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;
    si.lpDesktop = NULL;
    si.dwFlags = STARTF_FORCEONFEEDBACK;  


    if (!CreateProcess(NULL,
                       cmdline,
                       NULL,
                       NULL,
                       FALSE,
                       NORMAL_PRIORITY_CLASS,
                       NULL,
                       NULL,
                       &si,
                       &pi)) {
        sdkdiff_UI(TRUE);
        MessageBox(hwndClient, LoadRcString(IDS_FAILED_TO_LAUNCH_EDT),
                   szSdkDiff, MB_ICONSTOP|MB_OK);
        sdkdiff_UI(FALSE);
        goto error;
    }

    /* wait for completion. */
    WaitForSingleObject(pi.hProcess, INFINITE);

    /* close process and thread handles */
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    /* finished with the filename. deletes it if it was a temp. */
    compitem_freefilename(item, option, fname);

    /*
     * refresh cached view always .  A common trick is to edit the
     * composite file and then save it as a new left or right file.
     * Equally the user can edit the left and save as a new right.
     */

    /* We want to force both files to be re-read, but it's not a terribly
     * good idea to throw the lines away on this thread.  Someone might
     * be reading them on another thread! (e.g. user tries to expand
     * file, seems to be taking a long time, so user decides to edit it
     * to have a look!
     */

    /* We don't discard the lines (well, not on this thread) but we do discard
     * status information.  (Used to work only in expanded mode, relying on
     * status being reset when we go back to outline).
     */


    /* force the compare to be re-done */
    compitem_rescan(item);
    PostMessage(hwndClient, WM_COMMAND, IDM_UPDATE, (LPARAM)item);

    error:
    HeapFree(GetProcessHeap(), NULL, pe);

    return 0;

} /* do_editfile */


/* Launch an editor on a separate thread.  It will actually get a separate
   process, but we want our own thread in this process.  This thread will
   wait until it's finished and then order up a refresh of the UI.
*/
void
do_editthread(
              VIEW view,
              int option
              )
{
    PEDITARGS pe;
    HANDLE thread;
    DWORD threadid;

    pe = (PEDITARGS) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(EDITARGS));
    if (pe == NULL)
		return;
	pe->view = view;
    pe->option = option;
    pe->selection = selection;

    thread = CreateThread( NULL
                           , 0
                           , (LPTHREAD_START_ROUTINE)do_editfile
                           , (LPVOID) pe
                           , 0
                           , &threadid
                         );
    if (thread == NULL)
    {
        do_editfile(pe);
    }
    else CloseHandle(thread);

    // new layout not needed as do_editfile sends IDM_UPDATE

} /* do_editthread */


// we are called when the right mouse button is pressed. Before we are
// called, the row clicked on has been selected. We need to put up
// a context menu.
void
OnRightClick(
            HWND hWnd,
            int x,
            int y)
{
    HMENU L_hMenu, hSubMenu;
    POINT point;
    UINT uEnable;

    if (DisplayMode == MODE_OUTLINE) {
        L_hMenu = LoadMenu(hInst, szOutlineMenu);
    } else if (DisplayMode == MODE_EXPAND) {
        L_hMenu = LoadMenu(hInst, szExpandMenu);
    } else {
        return;
    }


    hSubMenu = GetSubMenu(L_hMenu, 0);

    // -- lots of stuff to disable inappropriate menu items --

    // enable IDM_TOMOVED only if it is a moved line that we can
    // see the other copy of in this view (and only if there is a single
    // selected line)

    if (DisplayMode == MODE_EXPAND) {
        if (ToMoved(hWnd, FALSE) && (1 == selection_nrows)) {
            uEnable = MF_ENABLED;
        } else {
            uEnable = MF_GRAYED;
        }
        EnableMenuItem(L_hMenu, IDM_TOMOVED, MF_BYCOMMAND | uEnable);
    }


    // disable next/prev buttons if no more changes in that direction
    if (view_findchange(current_view, selection+1, TRUE) >=0) {
        uEnable = MF_ENABLED;
    } else {
        uEnable = MF_GRAYED;
    }
    EnableMenuItem(hSubMenu, IDM_FCHANGE, MF_BYCOMMAND | uEnable);

    if (view_findchange(current_view, selection-1, FALSE) >=0) {
        uEnable = MF_ENABLED;
    } else {
        uEnable = MF_GRAYED;
    }
    EnableMenuItem(hSubMenu, IDM_FPCHANGE, MF_BYCOMMAND | uEnable);

    // check for left-only and right-only files and disable the appropriate
    // menu item
    // disable all editxxx and Expand if multiple files selected
    if ((DisplayMode == MODE_OUTLINE) && (selection_nrows > 1)) {
        EnableMenuItem(hSubMenu, IDM_EDITLEFT, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hSubMenu, IDM_EDITRIGHT, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hSubMenu, IDM_EDITCOMP, MF_BYCOMMAND|MF_GRAYED);
        EnableMenuItem(hSubMenu, IDM_EXPAND, MF_BYCOMMAND|MF_GRAYED);
    } else {
        COMPITEM item = view_getitem(current_view, selection);
        UINT uEnableLeft = MF_ENABLED;
        UINT uEnableRight = MF_ENABLED;
        UINT uEnableComp = MF_ENABLED;
        int state;
        FILEDATA fd;

        state = compitem_getstate(item);

        if (state == STATE_FILERIGHTONLY) {
            uEnableLeft = MF_GRAYED;
        } else if (state == STATE_FILELEFTONLY) {
            uEnableRight = MF_GRAYED;
        }

        fd = compitem_getleftfile(item);
        if (fd && file_IsUnicode(fd)) {
            uEnableComp = MF_GRAYED;
        }
        fd = compitem_getrightfile(item);
        if (fd && file_IsUnicode(fd)) {
            uEnableComp = MF_GRAYED;
        }

        EnableMenuItem(hSubMenu, IDM_EDITLEFT, MF_BYCOMMAND | uEnableLeft);
        EnableMenuItem(hSubMenu, IDM_EDITRIGHT, MF_BYCOMMAND | uEnableRight);
        EnableMenuItem(hSubMenu, IDM_EDITCOMP, MF_BYCOMMAND | uEnableComp);

        if (DisplayMode == MODE_OUTLINE) {
            EnableMenuItem(hSubMenu, IDM_EXPAND, MF_BYCOMMAND|MF_ENABLED);
        }
    }


    // convert the window-based co-ord to a screen co-ord
    point.x = x;
    point.y = y;
    ClientToScreen(hwndRCD, &point);

    TrackPopupMenu(
                  hSubMenu,
                  TPM_LEFTALIGN|TPM_RIGHTBUTTON,
                  point.x, point.y,
                  0,
                  hWnd,
                  NULL);

    DestroyMenu(L_hMenu);
}

//
// refresh the display after a rescan of a given line.
// try to maintain existing scroll position and selection.
void
OnUpdate(
         COMPITEM item
         )
{

    // save current scroll position
    long row = (long) SendMessage(hwndRCD, TM_TOPROW, FALSE, 0);

    // ... and current selection
    long lSel = selection;
    long cSel = selection_nrows;

    /* update the display.  Options or files may have changed */
    /* discard lines  (thereby forcing re-read).
     */
    file_discardlines(compitem_getleftfile(item));
    file_discardlines(compitem_getrightfile(item));

    view_changediffoptions(current_view);

    // tell the table view to recalculate its
    // idea of the width of each col etc

    SendMessage(hwndRCD, TM_NEWLAYOUT, 0, (LPARAM) current_view);

    // set old scroll position
    SendMessage(hwndRCD, TM_TOPROW, TRUE, row);

    // set back old selection
    SetSelection(lSel, cSel, lSel - row);


    /* force repaint of bar window */
    InvalidateRect(hwndBar, NULL, TRUE);
}

/* status bar and busy flags --------------------------------------------*/


/* set the Text on the statusbar button to reflect the current state */
void
SetButtonText(
              LPSTR cmd
              )
{
    SendMessage(hwndStatus, SM_SETTEXT, IDM_ABORT, (LPARAM) cmd);
}

/* set the status field (left-hand part) of the status bar. */
void
SetStatus(
          LPSTR cmd
          )
{
    SendMessage(hwndStatus, SM_SETTEXT, IDL_STATLAB, (LPARAM) cmd);
}

/*
 * Trace_Status is called from the ssclient.lib functions to report
 * non-fatal errors - put them on the status line
 */
void
Trace_Status(
             LPSTR str
             )
{
    SetStatus(str);
}


/* set the names field - the central box in the status bar */
void
SetNames(
         LPSTR names
         )
{
    SendMessage(hwndStatus, SM_SETTEXT, IDL_NAMES, (LPARAM) names);
    if (names == NULL) {
        AppTitle[0] = '\0';
    } else {
        My_mbsncpy(AppTitle, names, sizeof(AppTitle));
    }
}

/*
 * if we are not already busy, set the busy flag.
 *
 * WIN32: enter critical section first.
 */
BOOL
SetBusy(void)
{
    HMENU hmenu;


    WDEnter();

    if (fBusy) {
        WDLeave();
        return(FALSE);
    }


    fBusy = TRUE;

    SetStatus(LoadRcString(IDS_COMPARING));
    /* status also on window text, so that you can see even from
     * the icon when the scan has finished
     */
    SetWindowText(hwndClient, LoadRcString(IDS_SCANNING));

    /* disable appropriate parts of menu */
    hmenu = GetMenu(hwndClient);
    EnableMenuItem(hmenu, IDM_FILE,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_DIR,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_PRINT,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_SAVELIST,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_COPYFILES,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);

    /* enable abort only when busy */
    EnableMenuItem(hmenu, IDM_ABORT,MF_ENABLED|MF_BYCOMMAND);
    SetButtonText(LoadRcString(IDS_ABORT));  /* leave DisplayMode unchanged */

    WDLeave();
    return(TRUE);
} /* SetBusy */

void
SetNotBusy(void)
{
    HMENU hmenu;
	HRESULT hr;

    /*
     * this function can be called from the worker thread.
     * Thus we must not cause any SendMessage calls to windows
     * owned by the main thread while holding the CritSec or we
     * could cause deadlock.
     *
     * the critsec is only needed to protect the fBusy flag - so
     * clear the busy flag last, and only get the crit sec as needed.
     */

    /* reset button and status bar (clearing out busy flags) */
    if (current_view == NULL) {
        SetButtonText(LoadRcString(IDS_EXIT));
        SetStatus(NULL);
        DisplayMode = MODE_NULL;
    } else if (view_isexpanded(current_view)) {
        TCHAR szBuf[10];
        hr = StringCchCopy(szBuf, 10, LoadRcString(IDS_OUTLINE));
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY);
        SetButtonText(szBuf);
        SetStatus(view_getcurrenttag(current_view) );
        DisplayMode = MODE_EXPAND;
    } else {
        TCHAR szBuf[8];
        hr = StringCchCopy(szBuf,8, LoadRcString(IDS_EXPAND));
		if (FAILED(hr))
			OutputError(hr, IDS_SAFE_COPY);
        SetButtonText(szBuf);
        SetStatus(NULL);
        DisplayMode = MODE_OUTLINE;
    }

    SetWindowText(hwndClient, szSdkDiff);

    /* re-enable appropriate parts of menu */
    hmenu = GetMenu(hwndClient);
    EnableMenuItem(hmenu, IDM_FILE,MF_ENABLED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_DIR,MF_ENABLED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_PRINT,MF_ENABLED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_SAVELIST,MF_ENABLED|MF_BYCOMMAND);
    EnableMenuItem(hmenu, IDM_COPYFILES,MF_ENABLED|MF_BYCOMMAND);

    /* disable abort now no longer busy */
    EnableMenuItem(hmenu, IDM_ABORT,MF_DISABLED|MF_GRAYED|MF_BYCOMMAND);

    /* clear the busy flag, protected by critical section */
    WDEnter();

    fBusy = FALSE;
    bAbort = FALSE;

    if (ghThread!=NULL) {
        CloseHandle(ghThread);
        ghThread = NULL;
    }
    WDLeave();
} /* SetNotBusy */


BOOL
IsBusy()
{
    BOOL bOK;

    WDEnter();
    bOK = fBusy;
    WDLeave();
    return(bOK);
} /* IsBusy */

void
BusyError(void)
{
    sdkdiff_UI(TRUE);
    MessageBox(hwndClient,
               LoadRcString(IDS_PLEASE_WAIT),
               szSdkDiff, MB_OK|MB_ICONSTOP);
    sdkdiff_UI(FALSE);
} /* BusyError */

/* --- colour scheme --------------------------------------------------- */

/*
 * map the state given into a foreground and a background colour
 * for states that are highlighted. Return P_FCOLOUR if the foreground
 * colour (put in *foreground) is to be used, return P_FCOLOUR|P_BCOLOUR if
 * both *foreground and *background are to be used, or 0 if the default
 * colours are to be used.
 */
UINT
StateToColour(
              int state,
              BOOL bMarked,
              int col,
              DWORD * foreground,
              DWORD * foregroundws,
              DWORD * background
              )
{

    /* we always set both colours - allows all the colours to
       be controlled from the profile.  Important for the
       visually impaired.  So we first set the dafaults.
    */
    *foreground = rgb_defaultfore;
    *foregroundws = rgb_defaultforews;
    *background = rgb_defaultback;

    // marked compitems are highlighted specially - for now, use the
    // colour scheme used for different lines in expand mode
    if (bMarked) {
        *foreground = rgb_rightfore;
        *background = rgb_rightback;
        return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);
    }


    switch (state) {

        case STATE_DIFFER:
            /* files that differ are picked out in a foreground highlight,
             * with the default background
             */
            *foreground = rgb_outlinehi;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_FILELEFTONLY:
            /* zebra lines in both files - right file version */
            *foreground = rgb_fileleftfore;
            *background = rgb_fileleftback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_FILERIGHTONLY:
            /* zebra lines in both files - right file version */
            *foreground = rgb_filerightfore;
            *background = rgb_filerightback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_SIMILAR:
            /* for files that are same within expand compare options
             * e.g. differ only in ignorable blanks  (NYI)
            */
            *foreground = rgb_similar;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_LEFTONLY:
            /* lines only in the left file */
            *foreground = rgb_leftfore;
            *background = rgb_leftback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_RIGHTONLY:
            /* lines only in the right file */
            *foreground = rgb_rightfore;
            *background = rgb_rightback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_MOVEDLEFT:
            /* displaced lines in both files - left file version */
            *foreground = rgb_mleftfore;
            *background = rgb_mleftback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_MOVEDRIGHT:
            /* displaced lines in both files - right file version */
            *foreground = rgb_mrightfore;
            *background = rgb_mrightback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_SIMILARLEFT:
            /* zebra lines in both files - left file version */
            *foreground = rgb_similarleft;
            *background = rgb_leftback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        case STATE_SIMILARRIGHT:
            /* zebra lines in both files - right file version */
            *foreground = rgb_similarright;
            *background = rgb_rightback;
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);

        default:

            /* no highlighting - default colours */
            return(P_FCOLOUR|P_FCOLOURWS|P_BCOLOUR);
    }

}

/* table window communication routines ---------------------------------*/

/* set a given row as the selected row in the table window */
void
SetSelection(
             long rownr,
             long nrows,
             long dyRowsFromTop
             )
{
    TableSelection select;

    select.startrow = rownr;
    select.startcell = 0;
    select.nrows = nrows;
    select.ncells = 1;
    select.dyRowsFromTop = dyRowsFromTop;
    SendMessage(hwndRCD, TM_SELECT, 0, (LPARAM)&select);
}


/* handle table class call back to get nr of rows and columns,
 * and properties for the whole table.
 * the 'table id' is either TABID_PRINTER - meaning we are
 * printing the current_view, or it is the view to
 * use for row/column nr information
 */
long
do_gethdr(
          HWND hwnd,
          lpTableHdr phdr
          )
{
    VIEW view;
    BOOL bIsPrinter = FALSE;

    if (phdr->id == TABID_PRINTER) {
        view = current_view;
        bIsPrinter = TRUE;
    } else {
        view = (VIEW) phdr->id;
    }
    if (view == NULL) {
        return(FALSE);
    }

    phdr->nrows = view_getrowcount(view);

    /*  three columns: line nr, tag and rest of line */

    /*
     * if IDM_NONRS (no line numbers) is selected, suppress the
     * line-nr column entirely to save screen space
     */
    if (line_numbers == IDM_NONRS) {
        phdr->ncols = 2;
        phdr->fixedcols = 0;
    } else {
        phdr->ncols = 3;
        phdr->fixedcols = 1;
    }

    phdr->fixedrows = 0;
    phdr->fixedselectable = FALSE;
    phdr->hseparator = TRUE;
    phdr->vseparator = TRUE;

    phdr->selectmode = TM_ROW | TM_MANY;
    /*
     * find if we are in expand mode - ask for the item we are expanding.
     */
    if (view_isexpanded(view) == TRUE) {

        /* use focus rect as selection mode in expand mode
         * so as not to interfere with background colours.
         */
        phdr->selectmode |= TM_FOCUS;
    } else {
        /* use default solid inversion when possible as it is clearer.*/
        phdr->selectmode |= TM_SOLID;
    }

    /* please send TQ_SCROLL notifications when the table is scrolled */
    phdr->sendscroll = TRUE;
    if (g_hFont) {
        phdr->props.valid = P_FONT;
        phdr->props.hFont = g_hFont;
    }

    return TRUE;
}

/* respond to table callback asking for the size and properties
 * of each column. table id is either TABID_PRINTER (meaning the
 * current_view, for printing) or it is the view to be used.
 */
long
do_getprops(
            HWND hwnd,
            lpColPropsList propslist
            )
{
    int i, cell;
    BOOL bIsPrinter = FALSE;
    VIEW view;
    HFONT hfontSystem = (HFONT)GetStockObject(SYSTEM_FONT);
    HFONT hfontFixed = g_hFont ? g_hFont : (HFONT)GetStockObject(SYSTEM_FIXED_FONT);

    if (propslist->id == TABID_PRINTER) {
        view = current_view;
        bIsPrinter = TRUE;
    } else {
        view = (VIEW) propslist->id;
    }
    if (view == NULL) {
        return(FALSE);
    }

    if (g_hFont)
    {
        HDC hdc;
        TEXTMETRIC tmSystem;
        TEXTMETRIC tmFixed;
        HFONT hfont;

        hdc = GetDC(hwnd);
        if (hdc)
        {
            hfont = (HFONT)SelectObject(hdc, hfontSystem);
            GetTextMetrics(hdc, &tmSystem);
            SelectObject(hdc, hfontFixed);
            GetTextMetrics(hdc, &tmFixed);
            SelectObject(hdc, hfont);
            ReleaseDC(hwnd, hdc);

            if (tmFixed.tmHeight + 1 < tmSystem.tmHeight)
                hfontSystem = hfontFixed;
        }
    }

    /* the table inteface is slightly confused here. we are not
     * guaranteed which columns we are being asked about, so instead
     * of just setting each column cols[0], cols[1] etc, we need
     * to loop through, looking at each column in the table and
     * seeing which it is.
     */
    for (i = 0; i < propslist->ncols; i++) {
        cell = i + propslist->startcol;
        propslist->plist[i].props.valid = 0;

        /* for all column widths, add on 1 for the NULL char. */

        /*
         * skip the line nr column if IDM_NONRS
         */
        if (line_numbers == IDM_NONRS) {
            cell++;
        }

        if (cell == 0) {
            /* properties for line nr column */

            propslist->plist[i].nchars = view_getwidth(view, 0)+1;
            propslist->plist[i].props.valid |= P_ALIGN | P_FONT;
            propslist->plist[i].props.alignment = P_CENTRE;
            propslist->plist[i].props.hFont = hfontSystem;
        } else if (cell == 1) {

            /* properties for tag field */
            propslist->plist[i].nchars = view_getwidth(view, 1)+1;
            propslist->plist[i].props.valid |= P_ALIGN | P_FONT;
            propslist->plist[i].props.alignment = P_LEFT;
            propslist->plist[i].props.hFont = hfontSystem;
        } else {
            /* properties for main text column -
             * use a fixed font unless printing (if
             * printing, best to use the default font, because
             * of resolution differences.
             * add on 8 chars to the width to ensure that
             * the width of lines beginning with tabs
             * works out ok
             */
            propslist->plist[i].nchars = view_getwidth(view, 2)+1;
            propslist->plist[i].props.valid |= P_ALIGN;
            propslist->plist[i].props.alignment = P_LEFT;
            if (!bIsPrinter) {
                propslist->plist[i].props.valid |= P_FONT;
                propslist->plist[i].props.hFont = hfontFixed;
            }
        }
    }
    return (TRUE);
}

/* respond to a table callback asking for the contents of individual cells.
 * table id is either TABID_PRINTER, or it is a pointer to the view
 * to use for data. If going to the printer, don't set the
 * colours (stick to black and white).
 */
long
do_getdata(
           HWND hwnd,
           lpCellDataList cdlist
           )
{
    int start, endcell, col, i;
    int state, markedstate;
    lpCellData cd;
    VIEW view;
    LPSTR textp;
    LPWSTR pwzText;
    BOOL bIsPrinter = FALSE;

    if (cdlist->id == TABID_PRINTER) {
        view = current_view;
        bIsPrinter = TRUE;
    } else {
        view = (VIEW) cdlist->id;
    }

    /* set state data */
    state = view_getstate(view, cdlist->row);
    markedstate = view_getmarkstate(view, cdlist->row);
    start = cdlist->startcell;
    endcell = cdlist->ncells + start;
    if (cdlist->row >= view_getrowcount(view)) {
        return(FALSE);
    }
    for (i = start; i < endcell; i++) {
        cd = &cdlist->plist[i - start];


        /* skip the line number column if IDM_NONRS */
        if (line_numbers == IDM_NONRS) {
            col = i+1;
        } else {
            col = i;
        }

        /* set colour of text to mark out
         * lines that are changed, if not printer - for the
         * printer everything should stay in the default colours
         * or it will be grayed out and look ugly
         */

        if ( !bIsPrinter
          || (GetProfileInt(APPNAME, szColourPrinting, 0) > 0)) {

            /* convert the state of the requested row into a
             * colour scheme. returns P_FCOLOUR and/or
             * P_BCOLOUR if it sets either of the colours
             */
            cd->props.valid |=
            StateToColour(
                         state,
                         markedstate,
                         col,
                         &cd->props.forecolour,
                         &cd->props.forecolourws,
                         &cd->props.backcolour);
        }

        textp = view_gettext(view, cdlist->row, col);
        if (cd->nchars != 0) {
            if (textp == NULL) {
                cd->ptext[0] = '\0';
            } else {
                My_mbsncpy(cd->ptext, textp, cd->nchars -1);
                cd->ptext[cd->nchars - 1] = '\0';
            }
        }

        pwzText = view_gettextW(view, cdlist->row, col);
        if (pwzText && cd->nchars)
        {
            HRESULT hr;
            cd->pwzText = (WCHAR*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cd->nchars * sizeof(*cd->pwzText));
            if (cd->pwzText == NULL)
				return (FALSE);
            hr = StringCchCopyNW(cd->pwzText, cd->nchars, pwzText, cd->nchars -1);
            if (FAILED(hr))
            {
                HeapFree(GetProcessHeap(), NULL, cd->pwzText);
                cd->pwzText = NULL;
                return FALSE;
            }
            cd->pwzText[cd->nchars - 1] = 0;
        }
        else
        {
            HeapFree(GetProcessHeap(), NULL, cd->pwzText);
            cd->pwzText = NULL;
        }

    }
    return(TRUE);
}

/* table window has finished with this view. it can be deleted.
 */
void
SvrClose(void)
{
    view_delete(current_view);
    current_view = NULL;

    /* hide picture - only visible when we are in MODE_EXPAND */
    DisplayMode = MODE_NULL;
    DoResize(hwndClient);

    /* if we already busy when closing this view (ie
     * we are in the process of starting a new scan,
     * then leave the status bar alone, otherwise
     * we should clean up the state of the status bar
     */
    if (!fBusy) {
        SetButtonText(LoadRcString(IDS_EXIT));
        SetNames(NULL);
        SetStatus(NULL);

    }

} /* SvrClose */


/* handle callbacks and notifications from the table class */
long
TableServer(
            HWND hwnd,
            WPARAM cmd,
            LPARAM lParam
            )
{
    lpTableHdr phdr;
    lpColPropsList proplist;
    lpCellDataList cdlist;
    lpTableSelection pselect;

    switch (cmd) {
        case TQ_GETSIZE:
            /* get the nr of rows and cols in this table */
            phdr = (lpTableHdr) lParam;
            return(do_gethdr(hwnd, phdr));

        case TQ_GETCOLPROPS:
            /* get the size and properties of each column */
            proplist = (lpColPropsList) lParam;
            return (do_getprops(hwnd, proplist));

        case TQ_GETDATA:
            /* get the contents of individual cells */
            cdlist = (lpCellDataList) lParam;
            return (do_getdata(hwnd, cdlist));


        case TQ_SELECT:
            /* selection has changed */
        case TQ_ENTER:
            /* user has double-clicked or pressed enter */

            pselect = (lpTableSelection) lParam;

            /* store location for use in later search (IDM_FCHANGE) */

            /*
             * convert selection so that it always runs forward - we
             * do not need to know where the anchor vs endpoint is
             */
            if (pselect->nrows == 0) {
                selection = -1;
                selection_nrows = 0;
            } else {
                if (pselect->nrows < 0) {
                    selection = pselect->startrow + pselect->nrows + 1;
                    selection_nrows = -pselect->nrows;
                } else {
                    selection = (int) pselect->startrow;
                    selection_nrows = pselect->nrows;
                }
                if (cmd == TQ_ENTER) {
                    /* try to expand this row */
                    if (!ToExpand(hwnd)) {
                        /* expand failed - maybe this
                         * is a moved line- show the other
                         * copy
                         */
                        ToMoved(hwnd, TRUE);
                    }

                }
            }
            break;

        case TQ_CLOSE:
            /* close this table - table class no longer needs data*/
            SvrClose();
            break;

        case TQ_SCROLL:
            /* notification that the rows visible in the window
             * have changed -change the current position lines in
             * the graphic bar view (the sections picture)
             */
            if (picture_mode) {
                BarDrawPosition(hwndBar, NULL, TRUE);
            }
            break;

        case TQ_TABS:
            if (lParam != 0) {
                LONG * pTabs = (LONG *) lParam;
                *pTabs = g_tabwidth;
            }
            return TRUE;

        case TQ_SHOWWHITESPACE:
            if (lParam != 0) {
                LONG * pbShowWhitespace = (LONG *) lParam;
                *pbShowWhitespace = (show_whitespace && view_isexpanded(current_view));
            }
            return TRUE;

        default:
            return(FALSE);
    }
    return(TRUE);
}

/* --- thread worker routines (called synchoronously in WIN16)--------------*/

/*
 * called on worker thread (not UI thread) to handle the work
 * requested on the command line. 
 *
 * arg is a pointer to a THREADARGS block allocated using HeapAlloc. This
 * needs to be freed before exiting.
 */
DWORD WINAPI
wd_initial(
           LPVOID arg
           )
{
    PTHREADARGS pta = (PTHREADARGS) arg;
    COMPLIST cl = 0;
    COMPITEM ci = NULL;
    BOOL fOK = TRUE;
    BOOL fPairs = TRUE;

    /* build a complist from these args,
     * and register with the view we have made
     */
 
    {
        cl = complist_args(pta->first, pta->second, pta->view, pta->fDeep);
    }

    /*
     * the app can be closed during execution of this routine if
     * we don't retain BUSY. This means we can be expanding structures
     * while the main thread is freeing them.
     *
     */

    if (cl == NULL || !fOK) {
        view_close(pta->view);
        HeapFree(GetProcessHeap(), NULL, pta);
        SetNotBusy();
        return 0;
    }


    /* if savelist or savecomp was selected, write out the list or comp file */
    if (pta->savelist != NULL || pta->savecomp != NULL) {
        if (pta->savelist != NULL) {
            complist_savelist(cl, pta->savelist, pta->listopts);
        }

        if (pta->savecomp != NULL) {
            /* if list item was selected, use that */
            if (selection >= 0) {
                ci = view_getitem(pta->view, selection);
          }
          else {
              /* default to first visible item, if any */
              if (view_getrowcount(pta->view) > 0) {
                  ci = view_getitem(pta->view, 0);
              }
          }
          compitem_savecomp(pta->view, ci, pta->savecomp, pta->compopts);
        }
        HeapFree(GetProcessHeap(), NULL, pta);
        SetNotBusy();

        /* exit if -x was set */
        if (pta->fExit) {
            exit(0);
        }
    }


    /* if savelist was selected, write out the list and exit if -x was set */
    if (pta->savelist != NULL) {
        complist_savelist(cl, pta->savelist, pta->listopts);
        HeapFree(GetProcessHeap(), NULL, pta);
        SetNotBusy();
        if (pta->fExit) exit(0);
    }

    /* if there was only one file, expand it, unless... */
    if (view_getrowcount(pta->view) == 1) {
        /* The interesting case is where there are a bunch of files
           but only one of them is Different.  In this case we do
           NOT expand it even though it is the only one showing.
        */

        UINT nItems = complist_itemcount(cl);
        /* And even then, don't expand if the option said don't.
        */
        if (nItems==1 && fAutoExpand) {
            SetSelection(0, 1, -1);
            ToExpand(hwndClient);
        }
    }


    HeapFree(GetProcessHeap(), NULL, pta);
    SetNotBusy();
    return(0);
} /* wd_initial */


/*
 * called on worker thread (not UI thread) to handle a Dir request
 * (called synchronously if WIN16).
 */
DWORD WINAPI
wd_dirdialog(LPVOID arg)
{

    VIEW view = (VIEW) arg;

    /* make a COMPLIST using the directory dialog,
     * and notify the view
     */
    if (complist_dirdialog(view) == NULL) {
        view_close(view);
    }

    /* all done! */
    SetNotBusy();
    return(0);
}

/*
 * called on worker thread to do a copy-files operation
 * 
 */
DWORD WINAPI
wd_copy(
        LPVOID arg
        )
{

    VIEW view = (VIEW) arg;

    complist_copyfiles(view_getcomplist(view), NULL, 0);

    SetNotBusy();

    return(0);
}


/*----- winproc for main window ---------------------------------
 *
 */

INT_PTR
APIENTRY
MainWndProc(
            HWND hWnd,
            UINT message,
            WPARAM wParam,
            LPARAM lParam
            )
{
    char str[32];
    long ret;
    DWORD threadid;
	HRESULT hr;

    switch (message) {

        case WM_CREATE:

            /* initialise menu options to default/saved
             * option settings
             */

#define CHECKMENU(id, fChecked)  \
          CheckMenuItem(hMenu, (id), (fChecked) ? MF_CHECKED:MF_UNCHECKED)

            /* outline_include options */
            CHECKMENU(IDM_OUTLINE_INCSAME,   (outline_include & INCLUDE_SAME));
            CHECKMENU(IDM_OUTLINE_INCLEFT,   (outline_include & INCLUDE_LEFTONLY));
            CHECKMENU(IDM_OUTLINE_INCRIGHT,  (outline_include & INCLUDE_RIGHTONLY));
            CHECKMENU(IDM_OUTLINE_INCDIFFER, (outline_include & INCLUDE_DIFFER));

            /* expand_include options */
            CHECKMENU(IDM_EXPAND_INCSAME,         (expand_include & INCLUDE_SAME));
            CHECKMENU(IDM_EXPAND_INCLEFT,         (expand_include & INCLUDE_LEFTONLY));
            CHECKMENU(IDM_EXPAND_INCRIGHT,        (expand_include & INCLUDE_RIGHTONLY));
            CHECKMENU(IDM_EXPAND_INCMOVEDLEFT,    (expand_include & INCLUDE_MOVEDLEFT));
            CHECKMENU(IDM_EXPAND_INCMOVEDRIGHT,   (expand_include & INCLUDE_MOVEDRIGHT));
            CHECKMENU(IDM_EXPAND_INCSIMILARLEFT,  (expand_include & INCLUDE_SIMILARLEFT));
            CHECKMENU(IDM_EXPAND_INCSIMILARRIGHT, (expand_include & INCLUDE_SIMILARRIGHT));

            /* other options */
            CHECKMENU(line_numbers, TRUE);
            CHECKMENU(expand_mode, TRUE);
            CHECKMENU(IDM_IGNBLANKS, ignore_blanks);
            CHECKMENU(IDM_SHOWWHITESPACE, show_whitespace);
            CHECKMENU(IDM_ALG2, Algorithm2);
            CHECKMENU(IDM_MONOCOLS, mono_colours);
            CHECKMENU(IDM_PICTURE, picture_mode);
            CHECKMENU(IDM_HIDEMARK, hide_markedfiles);
#undef CHECKMENU

            /* nothing currently displayed */
            DisplayMode = MODE_NULL;

            break;

        case WM_SYSCOLORCHANGE:
            {
                HIGHCONTRAST hc;

                hc.cbSize = sizeof(hc);
                SystemParametersInfo(SPI_GETHIGHCONTRAST,0 ,&hc, 0);

                mono_colours = (hc.dwFlags & HCF_HIGHCONTRASTON);
                if (mono_colours) {
                    SetMonoColours();
                } else {
                    SetColours();	
                }

                CheckMenuItem(hMenu, IDM_MONOCOLS,
                              mono_colours? MF_CHECKED:MF_UNCHECKED);
                hr = StringCchPrintf(str, 32, szD, mono_colours);
				if (FAILED(hr))
					OutputError(hr, IDS_SAFE_PRINTF);
                WriteProfileString(APPNAME, szMonoColours, str);
                SendMessage(hwndRCD, message, wParam, lParam);
                SendMessage(hwndStatus, message, wParam, lParam);
                SendMessage(hwndBar, message, wParam, lParam);

                /* The diffs are still valid, but force a re-display
                 * of bar window and main table.
                 */
                SendMessage(hwndBar, WM_COMMAND, IDM_MONOCOLS, 0);
                InvalidateRect(hwndBar, NULL, TRUE);
                view_changeviewoptions(current_view);
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam)) {
                case IDM_EXIT:
                    if (ghThread!=NULL) {
                        bAbort = TRUE;
                        SetStatus(LoadRcString(IDS_ABORT_PENDING));
                        break;
                    }
                    if (!view_isexpanded(current_view)) {
                        /* save the current outline size and position */
                        WINDOWPLACEMENT wp;
                        wp.length = sizeof(wp);
                        if (GetWindowPlacement(hwndClient,&wp)) {
                            WriteProfileInt(APPNAME, szOutlineShowCmd, wp.showCmd);
                            WriteProfileInt(APPNAME, szOutlineMaxX, wp.ptMaxPosition.x);
                            WriteProfileInt(APPNAME, szOutlineMaxY, wp.ptMaxPosition.y);
                            WriteProfileInt(APPNAME, szOutlineNormLeft, wp.rcNormalPosition.left);
                            WriteProfileInt(APPNAME, szOutlineNormTop, wp.rcNormalPosition.top);
                            WriteProfileInt(APPNAME, szOutlineNormRight, wp.rcNormalPosition.right);
                            WriteProfileInt(APPNAME, szOutlineNormBottom, wp.rcNormalPosition.bottom);
                            WriteProfileInt(APPNAME, szOutlineSaved, 1);
                        }
                    } else {
                        /* save the current expanded size and position */
                        WINDOWPLACEMENT wp;
                        wp.length = sizeof(wp);
                        if (GetWindowPlacement(hwndClient,&wp)) {
                            WriteProfileInt(APPNAME, szExpandShowCmd, wp.showCmd);
                            WriteProfileInt(APPNAME, szExpandMaxX, wp.ptMaxPosition.x);
                            WriteProfileInt(APPNAME, szExpandMaxY, wp.ptMaxPosition.y);
                            WriteProfileInt(APPNAME, szExpandNormLeft, wp.rcNormalPosition.left);
                            WriteProfileInt(APPNAME, szExpandNormTop, wp.rcNormalPosition.top);
                            WriteProfileInt(APPNAME, szExpandNormRight, wp.rcNormalPosition.right);
                            WriteProfileInt(APPNAME, szExpandNormBottom, wp.rcNormalPosition.bottom);
                            WriteProfileInt(APPNAME, szExpandedSaved, 1);
                        }
                    }
                    DestroyWindow(hWnd);
                    break;

                case IDM_ABORT:
                    /* abort menu item, or status bar button.
                     * the status bar button text gives the appropriate
                     * action depending on our state - abort, outline
                     * or expand. But the command sent is always
                     * IDM_ABORT. Thus we need to check the state
                     * to see what to do. If we are busy, set the abort
                     * flag. If there is nothing to view,
                     * exit, otherwise switch outline<->expand
                     */
                    if (IsBusy()) {
                        bAbort = TRUE;
                        SetStatus(LoadRcString(IDS_ABORT_PENDING));
                    } else if (DisplayMode == MODE_NULL) {
                        DestroyWindow(hWnd);
                    } else if (DisplayMode == MODE_EXPAND) {
                        ToOutline(hWnd);
                    } else {
                        ToExpand(hWnd);
                    }
                    break;

                case IDM_FILE:
                    /* select two files and compare them */
                    if (SetBusy()) {


                        /* close the current view */
                        view_close(current_view);

                        /* make a new empty view */
                        current_view = view_new(hwndRCD);

                        /* make a COMPLIST using the files dialog,
                         * and notify the view
                         */
                        if (complist_filedialog(current_view) == NULL) {
                            view_close(current_view);
                        }

                        /* all done! */
                        SetNotBusy();
                    } else {
                        BusyError();
                    }
                    break;

                case IDM_DIR:

                    /* read two directory names, scan them and
                     * compare all the files and subdirs.
                     */
                    if (SetBusy()) {

                        /* close the current view */
                        view_close(current_view);

                        /* make a new empty view */
                        current_view = view_new(hwndRCD);

                        ghThread = CreateThread(NULL, 0, wd_dirdialog,
                                                (LPVOID) current_view, 0, &threadid);

                        if (ghThread == NULL)
                        {

                            /*
                             * either we are on WIN16, or the
                             * thread call failed. continue
                             * single-threaded.
                             */
                            wd_dirdialog( (LPVOID) current_view);


                        }

                    } else {
                        BusyError();
                    }
                    break;

                case IDM_CLOSE:
                    /* close the output list -
                     * discard all results so far
                     */
                    if (!IsBusy()) {
                        view_close(current_view);
                    }
                    break;

                case IDM_PRINT:
                    /* print the current view -
                     * either the outline list of filenames,
                     * or the currently expanded file.
                     */
                    if (!IsBusy()) {
                        DoPrint();
                    } else {
                        BusyError();
                    }
                    break;

                case IDM_TIME:
                    /* show time it took */
                    {       char msg[50];
                        DWORD tim;
                        if (IsBusy()) {
                            BusyError();
                        } else {
                            tim = complist_querytime();
                            hr = StringCchPrintf(msg, 50, LoadRcString(IDS_SECONDS), tim/1000, tim%1000);
							if (FAILED(hr))
								OutputError(hr, IDS_SAFE_PRINTF);
                            Trace_Status(msg);
                        }
                    }
                    break;

                case IDM_TRACE:
                    /* enable tracing */
                    bTrace = TRUE;
                    Trace_Status(LoadRcString(IDS_TRACING_ENABLED));
                    break;

                case IDM_TRACEOFF:
                    /* enable tracing */
                    bTrace = FALSE;
                    Trace_Status(LoadRcString(IDS_TRACING_DISABLED));
                    break;

                case IDM_SAVELIST:
                    /* allow user to save list of same/different files
                     * to a text file. dialog box to give filename
                     * and select which types of file to include
                     */
                    if (current_view == NULL) {
                        MessageBox(hWnd,
                                   LoadRcString(IDS_CREATE_DIFF_LIST),
                                   szSdkDiff, MB_OK|MB_ICONSTOP);
                        break;
                    }

                    complist_savelist(view_getcomplist(current_view), NULL, outline_include);
                    break;

                case IDM_COPYFILES:
                    /*
                     * copy files that are same/different to a new
                     * root directory. dialog box allows user
                     * to select new root and inclusion options
                     */
                    if (current_view == NULL) {
                        MessageBox(hWnd,
                                   LoadRcString(IDS_CREATE_DIFF_LIST),
                                   szSdkDiff, MB_OK|MB_ICONSTOP);
                        break;
                    }

                    if (SetBusy()) {
                        ghThread = CreateThread(NULL, 0, wd_copy,
                                                (LPVOID) current_view, 0, &threadid);
                        if (ghThread == NULL)
                        {
                            wd_copy( (LPVOID) current_view);
                        }

                    } else {
                        BusyError();
                    }

                    break;

                case IDM_ABOUT:
                    ShellAbout( hWnd,
                                (LPTSTR)szSdkDiff,
                                LoadRcString(IDS_TOOL_DESCRIPTION),
                                LoadIcon(hInst, szSdkDiff)
                              );
                    break;

                case IDM_CONTENTS:
                    /* Help contents */
					TCHAR pszHelp[MAX_PATH];
					memset(pszHelp, 0, MAX_PATH);
					hr = StringCchCopy(pszHelp, MAX_PATH, pszWorkingDirectoryName);
					if (FAILED(hr))
						OutputError(hr, IDS_SAFE_COPY);
					hr = StringCchCat(pszHelp, MAX_PATH, "sdkdiff.chm");
					if (FAILED(hr))
						OutputError(hr, IDS_SAFE_CAT);

					HtmlHelp(hWnd, pszHelp, HH_HELP_FINDER, 0);
                    break;

                    /* launch an editor on the current item - left, right or
                     * composite view
                     */
                case IDM_EDITLEFT:
                    do_editthread(current_view, CI_LEFT);
                    break;

                case IDM_EDITRIGHT:
                    do_editthread(current_view, CI_RIGHT);
                    break;

                case IDM_EDITCOMP:
                    {
                        COMPITEM item = view_getitem(current_view, selection);
                        FILEDATA fdLeft;
                        FILEDATA fdRight;

                        fdLeft = compitem_getleftfile(item);
                        fdRight = compitem_getrightfile(item);
                        if ((fdLeft && file_IsUnicode(fdLeft)) || (fdRight && file_IsUnicode(fdRight))) {
                            MessageBox(hWnd, LoadRcString(IDS_NOCOMPUNICODE),
                                       szSdkDiff, MB_OK);
                        } else {
                            do_editthread(current_view, CI_COMP);
                        }
                    }
                    break;

                    /* allow customisation of the editor command line */
                case IDM_SETEDIT:
                    if (StringInput(editor_cmdline, sizeof(editor_cmdline),
                                    LoadRcString(IDS_EDITOR_COMMAND),
                                    (LPSTR)szSdkDiff, editor_cmdline)) {
                        WriteProfileString(APPNAME, szEditor,
                                           editor_cmdline);
                    }
                    break;

                case IDM_SETTABWIDTH:
                    {
                        char sz[32];
                        int n;

LTabWidth_tryagain:
                        hr = StringCchPrintf(sz, 32, "%d", g_tabwidth);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        if (StringInput(sz, sizeof(sz),
                                        LoadRcString(IDS_TABWIDTH),
                                        (LPSTR)szSdkDiff, sz))
                        {
                            n = atoi(sz);
                            if (n <= 0 || n > 100)
                            {
                                MessageBox(hWnd,
                                           LoadRcString(IDS_BAD_TABWIDTH),
                                           szSdkDiff, MB_OK);
                                goto LTabWidth_tryagain;
                            }
                            WriteProfileInt(APPNAME, szTabWidth, n);
                            g_tabwidth = n;
                            SendMessage(hwndRCD, TM_SETTABWIDTH, 0, n);
                        }
                    }
                    break;

                case IDM_TABWIDTH4:
                case IDM_TABWIDTH8:
                    g_tabwidth = (GET_WM_COMMAND_ID(wParam, lParam) == IDM_TABWIDTH8) ? 8 : 4;
                    SendMessage(hwndRCD, TM_SETTABWIDTH, 0, g_tabwidth);
                    break;

                case IDM_SETFONT:
                    {
                        CHOOSEFONT cf;
                        LOGFONT lf;
                        char szFace[LF_FACESIZE];

                        hr = StringCchCopy(szFace, LF_FACESIZE, g_szFontFaceName);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_COPY);

                        memset(&lf, 0, sizeof(lf));
                        hr = StringCchCopy(lf.lfFaceName, LF_FACESIZE, szFace);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_COPY);
                        lf.lfWeight = g_fFontBold ? FW_BOLD : FW_DONTCARE;
                        lf.lfHeight = g_nFontHeight;
                        lf.lfCharSet = g_bFontCharSet;

                        memset(&cf, 0, sizeof(cf));
                        cf.lStructSize = sizeof(CHOOSEFONT);
                        cf.hwndOwner = hWnd;
                        cf.lpLogFont = &lf;
                        cf.Flags = CF_INITTOLOGFONTSTRUCT|CF_FORCEFONTEXIST|
                                CF_SCREENFONTS|CF_LIMITSIZE;
                        cf.nSizeMin = 8;
                        cf.nSizeMax = 36;
                        cf.lpszStyle = szFace;
                        if (ChooseFont(&cf))
                            {
                            hr = StringCchCopy(g_szFontFaceName, LF_FACESIZE, lf.lfFaceName);
							if (FAILED(hr))
								OutputError(hr, IDS_SAFE_COPY);
                            g_fFontBold = (lf.lfWeight == FW_BOLD);
                            g_nFontHeight = lf.lfHeight;
                            g_bFontCharSet = lf.lfCharSet;

                            WriteProfileString(APPNAME, szFontFaceName, g_szFontFaceName);
                            WriteProfileInt(APPNAME, szFontHeight, g_nFontHeight);
                            WriteProfileInt(APPNAME, szFontBold, g_fFontBold);
                            WriteProfileInt(APPNAME, szFontCharSet, g_bFontCharSet);

                            GetFontPref();

                            view_changeviewoptions(current_view);
                            }
                    }
                    break;



                case IDM_LNRS:
                case IDM_RNRS:
                case IDM_NONRS:

                    /* option selects whether the line nrs displayed
                     * in expand mode are the line nrs in the left
                     * file, the right file or none
                     */

                    CheckMenuItem(GetMenu(hWnd),
                                  line_numbers, MF_UNCHECKED);
                    line_numbers = GET_WM_COMMAND_ID(wParam, lParam);
                    CheckMenuItem(GetMenu(hWnd), line_numbers, MF_CHECKED);
                    hr = StringCchPrintf(str, 32, szD, line_numbers);
					if (FAILED(hr))
						OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szLineNumbers, str);

                    /* change the display to show the line nr style
                     * chosen
                     */

                    view_changeviewoptions(current_view);
                    break;

                /*
                 * options selecting which lines to include in the
                 * expand listing, based on their state
                 */
                case IDM_EXPAND_INCSAME:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_SAME;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCSAME,
                              (expand_include & INCLUDE_SAME) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                case IDM_EXPAND_INCLEFT:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_LEFTONLY;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCLEFT,
                              (expand_include & INCLUDE_LEFTONLY) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                case IDM_EXPAND_INCRIGHT:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_RIGHTONLY;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCRIGHT,
                              (expand_include & INCLUDE_RIGHTONLY) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                case IDM_EXPAND_INCMOVEDLEFT:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_MOVEDLEFT;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCMOVEDLEFT,
                              (expand_include & INCLUDE_MOVEDLEFT) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                case IDM_EXPAND_INCMOVEDRIGHT:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_MOVEDRIGHT;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCMOVEDRIGHT,
                              (expand_include & INCLUDE_MOVEDRIGHT) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                case IDM_EXPAND_INCSIMILARLEFT:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_SIMILARLEFT;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCSIMILARLEFT,
                              (expand_include & INCLUDE_SIMILARLEFT) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                case IDM_EXPAND_INCSIMILARRIGHT:
                        /* toggle flag in expand_include options */
                        expand_include ^= INCLUDE_SIMILARRIGHT;

                        /* check/uncheck as necessary */
                        CheckMenuItem(hMenu, IDM_EXPAND_INCSIMILARRIGHT,
                              (expand_include & INCLUDE_SIMILARRIGHT) ?
                                        MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, expand_include);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szLineInclude, str);
                        view_changeviewoptions(current_view);
                        break;

                    /*
                     * options selecting which files to include in the
                     * outline listing, based on their state
                     */
                case IDM_OUTLINE_INCLEFT:


                    /* toggle flag in outline_include options */
                    outline_include ^= INCLUDE_LEFTONLY;

                    /* check/uncheck as necessary */
                    CheckMenuItem(hMenu, IDM_OUTLINE_INCLEFT,
                                  (outline_include & INCLUDE_LEFTONLY) ?
                                  MF_CHECKED:MF_UNCHECKED);

                    hr = StringCchPrintf(str, 32, szD, outline_include);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szFileInclude, str);
                    view_changeviewoptions(current_view);


                    break;

                case IDM_OUTLINE_INCRIGHT:


                    outline_include ^= INCLUDE_RIGHTONLY;

                    CheckMenuItem(hMenu, IDM_OUTLINE_INCRIGHT,
                                  (outline_include & INCLUDE_RIGHTONLY) ?
                                  MF_CHECKED:MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, outline_include);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szFileInclude, str);
                    view_changeviewoptions(current_view);

                    break;

                case IDM_OUTLINE_INCSAME:


                    outline_include ^= INCLUDE_SAME;

                    CheckMenuItem(hMenu, IDM_OUTLINE_INCSAME,
                                  (outline_include & INCLUDE_SAME) ?
                                  MF_CHECKED:MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, outline_include);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szFileInclude, str);
                    view_changeviewoptions(current_view);


                    break;


                case IDM_OUTLINE_INCDIFFER:



                    outline_include ^= INCLUDE_DIFFER;

                    CheckMenuItem(hMenu, IDM_OUTLINE_INCDIFFER,
                                  (outline_include & INCLUDE_DIFFER) ?
                                  MF_CHECKED:MF_UNCHECKED);

                    hr = StringCchPrintf(str, 32, szD, outline_include);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szFileInclude, str);
                    view_changeviewoptions(current_view);


                    break;

                case IDM_UPDATE:
                    OnUpdate( (COMPITEM) lParam);
                    break;


                case IDM_RESCAN:
                    RescanFile(hWnd);

                    /* do we need to force any repaints? */
                    // - no, as RescanFile sends a IDM_UPDATE
                    break;


                case IDM_LONLY:
                case IDM_RONLY:
                case IDM_BOTHFILES:
                    /* option selects whether the expanded file
                     * show is the combined file, or just one
                     * or other of the input files.
                     *
                     * if we are not in expand mode, this also
                     * causes us to expand the selection
                     */


                    CheckMenuItem(GetMenu(hWnd), expand_mode, MF_UNCHECKED);
                    expand_mode = GET_WM_COMMAND_ID(wParam, lParam);
                    CheckMenuItem(GetMenu(hWnd), expand_mode, MF_CHECKED);

                    /* change the current view to show only the lines
                     * of the selected type.
                     */
                    if (DisplayMode == MODE_OUTLINE) {
                        ToExpand(hWnd);
                    } else {
                        view_changeviewoptions(current_view);
                    }


                    break;


                case IDM_IGNBLANKS:

                    /* if selected, ignore all spaces and tabs on
                     * comparison - expand view only: outline view
                     * will still show that 'text files differ'
                     */

                    ignore_blanks = !ignore_blanks;
                    CheckMenuItem(hMenu, IDM_IGNBLANKS,
                                  ignore_blanks? MF_CHECKED:MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, ignore_blanks);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szBlanks, str);

                    /* invalidate all diffs since we have
                     * changed diff options, and re-do and display the
                     * current diff if we are in expand mode.
                     */
                    view_changediffoptions(current_view);

                    /* force repaint of bar window */
                    InvalidateRect(hwndBar, NULL, TRUE);

                    break;

                case IDM_SHOWWHITESPACE:

                        /* if selected, display all spaces and tabs in
                           the expanded text view */
                        show_whitespace = !show_whitespace;
                        CheckMenuItem(hMenu, IDM_SHOWWHITESPACE,
                                show_whitespace ? MF_CHECKED:MF_UNCHECKED);
                        hr = StringCchPrintf(str, 32, szD, show_whitespace);
						if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                        WriteProfileString(APPNAME, szShowWhitespace, str);

                        // change the current view to show only the lines
                        // of the selected type.
                        if (DisplayMode == MODE_EXPAND) {
                                view_changeviewoptions(current_view);
                        }

                        break;

                case IDM_ALG2:

                    /* if selected, do algorithm2 which does not accept
                     * unsafe matches.
                     */

                    Algorithm2 = !Algorithm2;
                    CheckMenuItem(hMenu, IDM_ALG2,
                                  Algorithm2? MF_CHECKED:MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, Algorithm2);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szAlgorithm2, str);

                    /* invalidate all diffs since we have
                     * changed diff options, and re-do and display the
                     * current diff if we are in expand mode.
                     */
                    view_changediffoptions(current_view);

                    /* force repaint of bar window */
                    InvalidateRect(hwndBar, NULL, TRUE);

                    break;

                case IDM_MONOCOLS:

                    /* Use monochrome colours - toggle */

                    mono_colours = !mono_colours;
                    CheckMenuItem(hMenu, IDM_MONOCOLS,
                                  mono_colours? MF_CHECKED:MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, mono_colours);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szMonoColours, str);
                    if (mono_colours)
                        SetMonoColours();
                    else
                        SetColours();

                    /* The diffs are still valid, but force a re-display
                     * of bar window and main table.
                     */
                    SendMessage(hwndBar, WM_COMMAND, IDM_MONOCOLS, 0);
                    InvalidateRect(hwndBar, NULL, TRUE);
                    view_changeviewoptions(current_view);

                    break;

                case IDM_PICTURE:
                    picture_mode = !picture_mode;
                    CheckMenuItem(hMenu, IDM_PICTURE,
                                  picture_mode? MF_CHECKED:MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, picture_mode);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szPicture, str);
                    DoResize(hWnd);
                    break;

                case IDM_HIDEMARK:
                    // toggle state of marked files hidden or not
                    hide_markedfiles = !hide_markedfiles;
                    CheckMenuItem(hMenu, IDM_HIDEMARK,
                                  hide_markedfiles? MF_CHECKED : MF_UNCHECKED);
                    hr = StringCchPrintf(str, 32, szD, hide_markedfiles);
					if (FAILED(hr))
							OutputError(hr, IDS_SAFE_PRINTF);
                    WriteProfileString(APPNAME, szHideMark, str);

                    // rebuild view with new global option
                    // - note that marks only affect outline views
                    if (!view_isexpanded(current_view)) {
                        view_changeviewoptions(current_view);
                    }
                    break;

                case IDM_FIND:
                  {
                  int nRet = IDCANCEL;
                  DLGPROC lpProc = (DLGPROC) MakeProcInstance((WINPROCTYPE) FindDlgProc, hInst);
                  if (lpProc)
                    {
                    char *pszFind = NULL;
                    sdkdiff_UI(TRUE);
                    nRet = (BOOL) DialogBoxParam(hInst, (LPCTSTR) IDD_FIND, hwndClient, lpProc, 0L);
                    sdkdiff_UI(FALSE);
                    FreeProcInstance(lpProc);
                    }
                  }
                  break;

                case IDM_FINDNEXT:
                  {
                  const LONG iCol = (view_isexpanded(current_view)) ? 2 : 1;
                  FindString(hWnd, iCol, NULL, 1, 0);
                  break;
                  }

                case IDM_FINDPREV:
                  {
                  const LONG iCol = (view_isexpanded(current_view)) ? 2 : 1;
                  FindString(hWnd, iCol, NULL, -1, 0);
                  break;
                  }

                case IDM_GOTOLINE:
                  {
                  if (!view_getrowcount(current_view))
                    {
                    MessageBox(hWnd, LoadRcString(IDS_GOTOLINE_NOLINES), szSdkDiff, MB_OK|MB_ICONSTOP|MB_TASKMODAL);
                    }
                  else
                    {
                    DLGPROC lpProc = (DLGPROC) MakeProcInstance((WINPROCTYPE)GoToLineDlgProc, hInst);
                    if (lpProc)
                      {
                      sdkdiff_UI(TRUE);
                      DialogBoxParam(hInst, (LPCTSTR) IDD_GOTOLINE, hwndClient, lpProc, 0L);
                      sdkdiff_UI(FALSE);
                      FreeProcInstance(lpProc);
                      }
                    }
                  }
                  break;

                case IDM_EDITCOPY:
                    {
                        unsigned long cb = 0;
                        LPCSTR pszText = NULL;
                        long iRow = 0;
                        const BOOL fOutline = !view_isexpanded(current_view);

                        /* first count total data size */
                        for (iRow = 0; iRow < selection_nrows; iRow++) {
                            if (fOutline) {
                                pszText = view_gettext(current_view, selection + iRow, 1);
                                if (!pszText)
                                    break;
                                cb += (lstrlen(pszText) + 3) * sizeof(char);
                            }

                            pszText = view_gettext(current_view, selection + iRow, 2);
                            if (!pszText)
                                break;
                            cb += (lstrlen(pszText)) * sizeof(char);
                        }

                        if (cb && OpenClipboard(NULL)) {
                            /* space for trailing nul char */
                            cb++;

                            /* copy data to clipboard */
                            if (EmptyClipboard()) {
                                HGLOBAL hData = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,  cb);

                                if (hData) {
                                    void *pv = hData;
                                    if (!pv) {
                                        HeapFree(GetProcessHeap(), NULL, hData);
                                    }
                                    else {
                                        LPSTR pszBuf = (LPSTR) pv;
                                        memset(pv, 0, cb);

                                        for (iRow = 0; iRow < selection_nrows; iRow++) {
                                            if (fOutline) {
                                                pszText = view_gettext(current_view, selection + iRow, 1);
                                                if (!pszText)
                                                    break;

                                                hr = StringCchCopy(pszBuf, cb, pszText);
												if (FAILED(hr))
													OutputError(hr, IDS_SAFE_COPY);
                                                pszBuf += lstrlen(pszBuf);
                                                *(pszBuf++) = '\t';
                                                *pszBuf = 0;
                                            }

                                            pszText = view_gettext(current_view, selection + iRow, 2);
                                            if (!pszText)
                                                break;

                                            hr = StringCchCopy(pszBuf, cb, pszText);
											if (FAILED(hr))
												OutputError(hr, IDS_SAFE_COPY);
                                            pszBuf += lstrlen(pszBuf);

                                            if (fOutline) {
                                                *(pszBuf++) = '\r';
                                                *(pszBuf++) = '\n';
                                                *pszBuf = 0;
                                            }
                                        }

                                        SetClipboardData(CF_TEXT, hData);
                                    }
                                }
                            }
                            CloseClipboard();
                        }
                    }
                    break;

                case IDM_MARK:
                    {
                        BOOL bChanged = FALSE;
                        int i;

                        // toggle the mark on the current selection
                        // note that the selection could be multiple rows
                        for (i = 0; i < selection_nrows; i++) {

                            if (view_setmarkstate(current_view, selection + i,
                                                  !view_getmarkstate(current_view, selection + i))) {

                                bChanged = TRUE;
                            }
                        }

                        if (bChanged) {
                            // yes the mark state was changed - need
                            // to rebuild the view.
                            if (!view_isexpanded(current_view)) {
                                view_changeviewoptions(current_view);
                            }
                        }
                        break;
                    }

                case IDM_TOGGLEMARK:
                    // toggle the state of all files: unmark all
                    // marked files and vice versa
                    complist_togglemark(view_getcomplist(current_view));

                    // rebuild view
                    if (!view_isexpanded(current_view)) {
                        view_changeviewoptions(current_view);
                    }
                    break;

                case IDM_MARKPATTERN:
                    // dialog to query the pattern, then set a mark on
                    // all compitems whose title matches that pattern
                    // returns TRUE if anything was changed
                    if (complist_markpattern(view_getcomplist(current_view))) {

                        // rebuild view
                        if (!view_isexpanded(current_view)) {
                            view_changeviewoptions(current_view);
                        }
                    }
                    break;

                case IDM_EXPAND:

                    /* show the expanded view of the
                     * selected file
                     */
                    if (current_view != NULL) {
                        ToExpand(hWnd);
                    }

                    break;

                case IDM_OUTLINE:
                    /* return to the outline view (list of filenames) */
                    ToOutline(hWnd);

                    break;

                case IDM_FCHANGE:
                    /* find the next line in the current view
                     * that is not the same in both files -
                     * in outline view, finds the next filename that
                     * is not identical
                     */
                    FindNextChange();
                    break;

                case IDM_FPCHANGE:
                    /* same as IDM_FCHANGE, but going backwards from
                     * current position
                     */
                    FindPrevChange();
                    break;

                case IDM_FCHANGE_LAURIE:
                    /* same as IDM_FCHANGE, but tries to keep cursor on same
                     * line.
                     */
                    _FindNextChange(TRUE, FALSE);
                    break;

                case IDM_FPCHANGE_LAURIE:
                    /* same as IDM_FPCHANGE, but tries to keep cursor on same
                     * line.
                     */
                    _FindPrevChange(TRUE, FALSE);
                    break;

                    // given a line that has been moved, jump to the
                    // other representation of the same line.
                    // this used to be available just through double-click
                    // but now is also available from a context menu
                case IDM_TOMOVED:
                    ToMoved(hWnd, TRUE);
                    break;
            }
            break;

        case WM_SIZE:
            DoResize(hWnd);
            break;

        case WM_SETFOCUS:
            /* set the focus on the table class so it can process
             * page-up /pagedown keys etc.
             */
            SetFocus(hwndRCD);
            break;

        case WM_KEYDOWN:
            /* although the table window has the focus, he passes
             * back to us any keys he doesn't understand
             * We handle escape here to mean 'return to outline view'
             */
            if (wParam == VK_ESCAPE) {
                ToOutline(hWnd);
            } else if (wParam == VK_APPS) {
                // Handle the context menu keyboard key
                POINT posCursor;
                GetCursorPos(&posCursor);
                ScreenToClient(hwndRCD, &posCursor);
                OnRightClick(hWnd, posCursor.x, posCursor.y);
            }
            break;

        case WM_RBUTTONDOWN:
            /*
             * the table window handles this by performing the
             * selection and then passing the message to us, allowing
             * us to put up a context menu.
             */
            OnRightClick(hWnd, LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_CLOSE:
            SendMessage(hWnd, WM_COMMAND, IDM_EXIT, 0);    
            return TRUE;

            /* don't allow close when busy - process this message in
             * order to ensure this
             */
            break;

        case WM_DESTROY:
			TCHAR pszHelp[MAX_PATH];
			memset(pszHelp, 0, MAX_PATH);
			hr = StringCchCopy(pszHelp, MAX_PATH, pszWorkingDirectoryName);
			if (FAILED(hr))
				OutputError(hr, IDS_SAFE_COPY);
			hr = StringCchCat(pszHelp, MAX_PATH, "sdkdiff.chm");
			if (FAILED(hr))
				OutputError(hr, IDS_SAFE_COPY);
            DeleteTools();
            HtmlHelp(hWnd, pszHelp, HH_CLOSE_ALL, 0);
            PostQuitMessage(0);
            break;

        case TM_CURRENTVIEW:
            /* allow other people such as the bar window to query the
             * current view
             */
            return((INT_PTR) current_view);

#ifdef LATER
#ifdef WM_MOUSEWHEEL
        case WM_MOUSEWHEEL:
            if (LOWORD(wParam) & MK_MBUTTON) {
                if ((short)HIWORD(wParam) < 0) {
                    // The next occurence
                    _FindNextChange(FALSE, TRUE);
                }
                else {
                    _FindPrevChange(FALSE, TRUE);
                }
            }
            break;
#endif
#endif

        case WM_QUERYENDSESSION:
            if (IsBusy())
                return FALSE;
            return TRUE;

        case WM_ENDSESSION:
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;

        default:
            /* handle registered table messages */
            if (message == table_msgcode) {
                ret = TableServer(hWnd, wParam, lParam);
                return(ret);
            }
            return(DefWindowProc(hWnd, message, wParam, lParam));
    }
    return(0);
}

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
            pszSepT = (PUCHAR)CharNext((LPCSTR)pszSepT);
        }
        psz = (PUCHAR)CharNext((LPCSTR)psz);
    }
    return NULL;
}

/***************************************************************************
 * Function: My_mbsstr
 *
 * Purpose:
 *
 * DBCS version of strstr
 *
 */
PUCHAR
My_mbsstr(
         PUCHAR str1,
         PUCHAR str2,
         PUCHAR *pstrEnd
         )
{
  PUCHAR cp = (PUCHAR) str1;
  PUCHAR s1, s2;

  if (!*str2)
    return ((PUCHAR) str1);

  while (*cp)
    {
    s1 = cp;
    s2 = (PUCHAR) str2;

    while (*s1 && *s2)
      {
      if (*s1 - *s2)
        break;

      if (IsDBCSLeadByte(*s1) && (++(*s1) - ++(*s2)))
        break;

      s1++;
      s2++;
      }

    if (!*s2)
      {
      if (pstrEnd)
        *pstrEnd = s1;
      return cp;
      }

    cp = (PUCHAR)CharNext((LPCSTR)cp);
    }

  return NULL;
}

/***************************************************************************
 * Function: My_mbsistr
 *
 * Purpose:
 *
 * DBCS version of case-independent strstr
 *
 */
PUCHAR
My_mbsistr(
          PUCHAR str1,
          PUCHAR str2,
          PUCHAR *pstrEnd
          )
{
  PUCHAR cp = (PUCHAR) str1;
  PUCHAR s1, s2;

  if (!*str2)
    return ((PUCHAR) str1);

  while (*cp)
    {
    s1 = cp;
    s2 = (PUCHAR) str2;

    while (*s1 && *s2)
      {
      if (IsDBCSLeadByte(*s1))
        {
        if ((*s1 - *s2) || (++(*s1) - ++(*s2)))
          break;
        }
      else if (toupper(*s1) - toupper(*s2))
        {
        break;
        }

      s1++;
      s2++;
      }

    if (!*s2)
      {
      if (pstrEnd)
        *pstrEnd = s1;
      return cp;
      }

    cp = (PUCHAR)CharNext((LPCSTR)cp);
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
    return (LPSTR)(*psz == uiSep ? psz : NULL);
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
           size_t nLen
           )
{
    LPSTR pszSv = psz1;
    int Length = (int)nLen;

    while (0 < Length) {
        if (*psz2 == '\0') {
            *psz1++ = '\0';
            Length--;
        } else if (IsDBCSLeadByte(*psz2)) {
            if (Length == 1) {
                *psz1 = '\0';
            } else {
                *psz1++ = *psz2++;
                *psz1++ = *psz2++;
            }
            Length -= 2;
        } else {
            *psz1++ = *psz2++;
            Length--;
        }
    }
    return pszSv;
}

/***************************************************************************
 * Function: My_mbsrchr
 *
 * Purpose:
 *
 * DBCS version of strrchr
 *
 */
LPSTR
My_mbsrchr(
           LPCSTR psz,
           unsigned short uiSep
           )
{
    unsigned const char *pszHead;

    pszHead = (unsigned char *)psz;

    while (*psz != '\0') {
        psz++;
    }
    if (uiSep == '\0') {
        return (LPSTR)psz;
    }

    while ((LPCSTR)psz > (LPCSTR)pszHead) {
        psz = CharPrev((LPCSTR)pszHead, psz);
        if (*psz == uiSep) {
            break;
        }
    }
    return (LPSTR)(*psz == uiSep ? psz : NULL);
}

/***************************************************************************
 * Function: My_mbsncmp
 *
 * Purpose:
 *
 * DBCS version of strncmp
 * If 'nLen' splits a DBC, this function compares the DBC's 2nd byte also.
 *
 */
int
My_mbsncmp(
           LPCSTR psz1,
           LPCSTR psz2,
           size_t nLen
           )
{
    int Length = (int)nLen;

    while (0 < Length) {
        if ('\0' == *psz1 || '\0' == *psz2) {
            return *psz1 - *psz2;
        }
        if (IsDBCSLeadByte(*psz1) || IsDBCSLeadByte(*psz2)) {
            if (*psz1 != *psz2 || *(psz1+1) != *(psz2+1)) {
                return *psz1 - *psz2;
            }
            psz1 += 2;
            psz2 += 2;
            Length -= 2;
        } else {
            if (*psz1 != *psz2) {
                return *psz1 - *psz2;
            }
            psz1++;
            psz2++;
            Length--;
        }
    }
    return 0;
}

/***************************************************************************
 * Function: My_mbsnicmp
 *
 * Purpose:
 *
 * DBCS version of strncmp
 * If 'nLen' splits a DBC, this function compares the DBC's 2nd byte also.
 *
 */
int
My_mbsnicmp(
           PUCHAR psz1,
           PUCHAR psz2,
           size_t nLen
           )
{
    int Length = (int)nLen;

    while (0 < Length) {
        if ('\0' == *psz1 || '\0' == *psz2) {
            return *psz1 - *psz2;
        }
        if (IsDBCSLeadByte(*psz1) || IsDBCSLeadByte(*psz2)) {
            if (*psz1 != *psz2 || *(psz1+1) != *(psz2+1)) {
                return *psz1 - *psz2;
            }
            psz1 += 2;
            psz2 += 2;
            Length -= 2;
        } else {
            if (toupper(*psz1) != toupper(*psz2)) {
                return *psz1 - *psz2;
            }
            psz1++;
            psz2++;
            Length--;
        }
    }
    return 0;
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

    LoadString((HINSTANCE)GetModuleHandle(NULL),wID,szBuf,sizeof(szBuf));
    return szBuf;
}
