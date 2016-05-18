#include <windows.h>
#include <imm.h>
#include "FullIME.h"

/************************************************************************
*
* Global data
*
************************************************************************/

char szSteClass[10];                // window class name
char szSteCandUIClass[12];          // Candidate window class name
char szSteTitle[55];                // window frame title
char szSteCompTitle[ 55 ];          // window composition framw title
char szSteCandTitle[ 55 ];          // window candidate frame title
char szCandClass[ 20 ];             // Candidate window class name
UINT  cxMetrics,                    // aver. character width
      cxOverTypeCaret,              // caret width for overtype mode
      cyMetrics;                    // character height
int xPos, yPos;                     // caret position
HFONT hfntFixed;                    // fixed-pitch font
HFONT hfntOld;                      // default font holder
BOOL  fInsertMode;                  // insert/overtype mode flag
int CaretWidth;                     // insert/overtype mode caret width
BYTE textbuf[MAXROW][MAXCOL];       // text buffer
int DBCSFillChar;                   // 'Pattern/DBCS' fill character


IMEUIDATA gImeUIData;               // IMEUI's global data.

HKL hCurKL;                         // the current keyboad layout.
