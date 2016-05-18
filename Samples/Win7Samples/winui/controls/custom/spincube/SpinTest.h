/******************************************************************************\
*
*                                 SPINTEST.H
*
\******************************************************************************/



/******************************************************************************\
*                             SYMBOLIC CONSTANTS
\******************************************************************************/

#define IDM_DLGEDITDIALOG  101  /* menuitem id's */
#define IDM_SPINTESTSTATS  102
#define IDM_ABOUT          103

#define BORDER             4

#define IDS_REGCLASSFAIL    16
#define IDS_WINDOWTITLE     17

//
// Spincube window styles as defined in spincube.h.
//

#define SS_ERASE                0x0001
#define SS_INMOTION             0x0002



/******************************************************************************\
*                               GLOBAL VARIABLES
\******************************************************************************/

HINSTANCE   ghInst;
HWND        gahwndSpin[4];


/******************************************************************************\
*                              FUNCTION PROTOTYPES
\******************************************************************************/

LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK DlgProc     (HWND, UINT, WPARAM, LPARAM);
