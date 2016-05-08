#define MAXROW          12
#define MAXCOL          60
#define FIRSTROW         0
#define FIRSTCOL         0
#define LASTROW         (MAXROW-1)
#define LASTCOL         (MAXCOL-1)
#define TABSTOP          4
#define MAX_LISTCAND     32
#define MAX_CHARS_PER_LINE   10
#define MAX_COMP_STRING_LEN  10
#define DEFAULT_CAND_NUM_PER_PAGE 5
#define X_INDENT         10
#define Y_INDENT         10

#define ID_ICON         1
#define ID_MENU         1
#define IDS_CLASS       1
#define IDS_TITLE       2
#define IDS_CANDUI      3
#define IDS_COMPTITLE   4
#define IDS_CANDTITLE   5
#define IDS_CANDCLASS   6

#define IDC_LIST1                0
#define IDC_LIST32              31

#define IDC_CLEAR               41
#define IDC_ANSIFILL            42
#define IDC_DBCSFILL            43
#define IDC_OPENIME             44
#define IDC_CLOSEIME            45

#define IME_IN_COMPOSITION      1
#define IME_IN_CHOSECAND        2

#define CheckProperty		{				\
    if ( ( gImeUIData.fdwProperty & IME_PROP_SPECIAL_UI ) ||	\
	 !( gImeUIData.fdwProperty & IME_PROP_AT_CARET ) )	\
	goto call_defwinproc;					\
}

typedef struct _IMEUIDATA {
    int        ImeState;   // Current Ime state.
    UINT       uCompLen;   // To save previous composition string length.
    DWORD      fdwProperty;
    HWND       hListCand[ MAX_LISTCAND ];
    HGLOBAL    hListCandMem[ MAX_LISTCAND ];
} IMEUIDATA;

//
// Prototype declaration
//

int WINAPI SteWndProc( HWND, UINT, UINT, LONG );
int WINAPI CandWndProc( HWND hwnd, UINT, WPARAM, LPARAM );

void ResetCaret( HWND );

void ImeUIStartComposition( HWND );
void ImeUIComposition( HWND, WPARAM, LPARAM );
void GetCompositionStr( HWND, LPARAM );
void GetResultStr( HWND );
void ImeUIEndComposition( HWND );
void ImeUIOpenCandidate( HWND, LPARAM );
void ImeUICloseCandidate( HWND, LPARAM );
void ImeUISetOpenStatus( HWND );
void DisplayResultString( HWND, LPSTR );
void DisplayCompString( HWND, LPSTR, LPSTR );
void RestoreImeUI( HWND );
BOOL ImeUINotify( HWND, WPARAM, LPARAM );
void ImeUIChangeCandidate( HWND, LPARAM );
void DisplayCandStrings( HWND, LPCANDIDATELIST );
void CandUIPaint( HWND );
void ImeUIMoveCandWin( HWND );
BOOL MoveCaret( HWND );
void ImeUIClearData( HWND );
void SetIMECompFormPos( HWND );

//
// Global data
//

extern char szSteClass[];
extern char szSteCandUIClass[];
extern char szSteTitle[];
extern char szSteCompTitle[];
extern char szSteCandTitle[];
extern char szCandClass[];

extern UINT   cxMetrics,
              cxOverTypeCaret,
              cyMetrics;

extern int    xPos, yPos;
extern HFONT  hfntFixed;
extern HFONT  hfntOld;
extern BOOL   fInsertMode;
extern int    CaretWidth;
extern int    DBCSFillChar;
extern BYTE   textbuf[MAXROW][MAXCOL];

extern IMEUIDATA gImeUIData;

extern HKL    hCurKL;
