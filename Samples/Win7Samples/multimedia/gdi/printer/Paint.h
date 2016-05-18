
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright 1993 - 2000 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
\******************************************************************************/

/******************************************************************************\
*
*                                PAINT.H
*
\******************************************************************************/

/******************************************************************************\
*                           SYMBOLIC CONSTANTS
\******************************************************************************/

#define BORDER                   5

#define NUM_GRAPHICS_XSLOTS      4
#define NUM_GRAPHICS_YSLOTS      3

#define ERR_MOD_NAME              IDS_ERR_PAINT



/******************************************************************************\
*                                TYPEDEFS
\******************************************************************************/

typedef struct
{
  int left;
  int top;
  int right;
  int bottom;

} RECTI, *PRECTI;

typedef struct tagArFonts
{
  int        nFonts;
  int        cySpace;
  HDC        hdc;
  LOGFONT    *lf;
  TEXTMETRIC *tm;
  int        *Type;

} ARFONTS, *PARFONTS;



/******************************************************************************\
*                             GLOBAL VARIABLES
\******************************************************************************/

HDC      hdcGlobal;          // globals utilized by enum fonts funtions
PARFONTS parFontsGlobal;
int      iFace,jFont;
int      nFaces;

int      giDeltaX;           // vars used by Get*GraphicSlot calls
int      giDeltaY;
int      giColumn;



/******************************************************************************\
*                          EXTERNAL VARIABLES
\******************************************************************************/

extern HANDLE ghInst;
extern HWND   ghwndMain;
extern DWORD  gdwGraphicsOptions;
extern int    giMapMode;
extern LONG   glcyMenu;


/******************************************************************************\
*                            FUNCTION PROTOTYPES
\******************************************************************************/

void GetFirstGraphicSlot (LPRECT, PRECTI);
void GetNextGraphicSlot  (PRECTI);

void         DrawFonts     (HDC, LPRECT);
PARFONTS     BuildFontList (HDC);
void         FreeFontList  (PARFONTS);

int CALLBACK MyEnumCount (LPLOGFONT, LPTEXTMETRIC, DWORD, LPVOID);
int CALLBACK MyEnumCopy  (LPLOGFONT, LPTEXTMETRIC, DWORD, LPVOID);
int CALLBACK MyEnumFaces (LPLOGFONT, LPTEXTMETRIC, DWORD, LPVOID);
