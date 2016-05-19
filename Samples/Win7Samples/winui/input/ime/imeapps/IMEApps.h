/**********************************************************************/
/*                                                                    */
/*      IMEAPPS.H                                                     */
/*                                                                    */
/*      Copyright (c) 1995 - 2000  Microsoft Corporation                */
/*                                                                    */
/**********************************************************************/

#include "wapi.h"

#define ORG_X                     10
#define ORG_Y                     10

#define RESULT_X                  10
#define RESULT_Y                  10
#define RESULTREAD_X              10
#define RESULTREAD_Y              30

#define COMPSTR_X                 10
#define COMPSTR_Y                 50
#define COMPREAD_X                10
#define COMPREAD_Y                60

#define IMESTATUS_CY              20
#define CAND_CX                   130

extern HANDLE hInst;
extern HWND hWndMain;
extern HWND hWndToolBar;
extern HWND hWndStatus;
extern HWND hWndCompStr;
extern HWND hWndCandList;

extern LOGFONT lf;
extern HFONT hFont;

extern int nStatusHeight;
extern BOOL fShowCand;
extern DWORD  fdwProperty;

extern DWORD  dwCompStrLen;
extern DWORD  dwCompAttrLen;
extern DWORD  dwCompClsLen;
extern DWORD  dwCompReadStrLen;
extern DWORD  dwCompReadAttrLen;
extern DWORD  dwCompReadClsLen;
extern DWORD  dwResultStrLen;
extern DWORD  dwResultClsLen;
extern DWORD  dwResultReadStrLen;
extern DWORD  dwResultReadClsLen;

extern MYCHAR szCompStr[512];
extern BYTE   bCompAttr[512];
extern DWORD  dwCompCls[128];
extern MYCHAR szCompReadStr[512];
extern BYTE   bCompReadAttr[512];
extern DWORD  dwCompReadCls[128];
extern MYCHAR szResultStr[512];
extern DWORD  dwResultCls[128];
extern MYCHAR szResultReadStr[512];
extern DWORD  dwResultReadCls[128];
extern MYCHAR szPaintResult[512];
extern MYCHAR szPaintResultRead[512];

extern LPCANDIDATELIST lpCandList;
extern POINT ptImeUIPos;



/* imeapps.c */
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CompStrWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK CandListWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK AboutDlg(HWND, UINT, WPARAM, LPARAM);
LRESULT HandleStartComposition(HWND,WPARAM,LPARAM);
LRESULT HandleEndComposition(HWND,WPARAM,LPARAM);
LRESULT HandleComposition(HWND,WPARAM,LPARAM);
LRESULT HandleChar(HWND,WPARAM,LPARAM);
LRESULT HandleNotify(HWND,UINT,WPARAM,LPARAM);
LRESULT HandlePaint(HWND, WPARAM, LPARAM);
LRESULT HandleCandPaint(HWND, WPARAM, LPARAM);


// setcomp.c
LRESULT HandleChangeAttr(HWND hWnd,BOOL fNext);

// subs.c
void MoveCompCandWindow(HWND hWnd);
BOOL CreateStatus();
void SetStatusItems(HWND hWnd);
void SetOpenStatusParts(BOOL fOpen);
void SetConvModeParts(DWORD dwConvMode);
void ClearConvModeParts();

// toolbar.c
BOOL CreateTBar(HWND);
LRESULT SetTooltipText(HWND hWnd, LPARAM lparam);
void UpdateButton(UINT iID, UINT iFlags);
void UpdateShowOpenStatusButton(BOOL fOpen);
void UpdateShowCandButton();
void UpdateModeButton(DWORD dwConv);

// mode.c
LRESULT HandleModeCommand(HWND hWnd,WPARAM wParam,LPARAM lParam);
LRESULT HandleConvertCommand(HWND, WPARAM, LPARAM);

// imemenu.c
void EndMenuItemIDTable();
void InitMenuItemIDTable();
DWORD FindItemData(UINT wID);
HMENU CreateImeMenu(HWND hWnd, HIMC hIMC, LPMYIMEMENUITEMINFO lpImeParentMenu, BOOL fRight);
