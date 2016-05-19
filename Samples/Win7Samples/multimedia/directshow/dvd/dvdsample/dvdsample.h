//------------------------------------------------------------------------------
// File: DvdSample.h
//
// Desc: DVD Playback sample app using IDvdGraphBuilder,
//       IDvdInfo2, and IDvdControl2 interfaces.
//
//       This contains headers for all Windows-related "plumbing" for the app
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Class Forward Declarations
//------------------------------------------------------------------------------

class CDvdCore;
class CParentLevels;
class CApp;
extern CApp g_App;
class CSPLangDlg;
class CAudioLangDlg;
class CDVDLanguages; 
class CAngleDlg;
class CKaraokeDlg;


//------------------------------------------------------------------------------
// Name: class CApp
// Desc: This class contains the application interface
//------------------------------------------------------------------------------

class CApp : public IDvdCallback
{
    friend CSPLangDlg;
    friend CAudioLangDlg;
    friend CAngleDlg;
    friend CKaraokeDlg;

public:
	CApp();
	virtual ~CApp();

	void Prohibited( void );
    void Exit( void );
	void SetAppValues(HINSTANCE hInst, PTSTR szAppName, int iAppTitleResId);
    void UpdateStatus(void); // to implement the IDvdCallback interface requirements

	static BOOL CALLBACK SelectParentalLevel(HWND hDlg, UINT message, 
                                             WPARAM wParam, LPARAM lParam);
	bool InitInstance(int nCmdShow);
	RECT GetAppWindow(void);

    inline const TCHAR * GetAppName(void) const { return m_szAppName; };
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;

private:
	LRESULT MenuProc(HWND hWnd, WPARAM wParam, LPARAM lParam);
    LRESULT KeyProc(WPARAM wParam, LPARAM lParam);
	LRESULT ToolTipProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	void DrawStatus(HDC hDC);
	bool OnSelectDisc(void);
	int MakeParentLevelList(HWND hDlg, int iListID);

	DWORD               m_dwProhibitedTime;
	HWND                m_hwndToolBar;
	CDVDLanguages *     m_pLangLookup;
	bool                m_bFullScreenOn;
	bool                m_bCaptionsOn;
	CDvdCore *          m_pDvdCore;
	HWND                m_hWnd;
    TCHAR               m_szAppName[50] ;   // internal name of the app
	TCHAR               m_szAppTitle[50];   // title bar text
	HINSTANCE           m_hInstance;
    CParentLevels *     m_pParentLevels;
};


//------------------------------------------------------------------------------
// Name: class CParentLevels
// Desc: This is a class to wrap the viewing level numbers and names.
//       This makes it easy to add viewing level definitions or change them 
//       to those of another country, for example.
//------------------------------------------------------------------------------
const int LEVELS = 6;
class CParentLevels 
{
public:
    CParentLevels() ;

    int     GetCount(void)   { return m_iCount ; } ;
    int     GetValue(int i)  { return m_aiValues[i] ; } ;
    LPCTSTR GetName(int i)   { return m_alpszNames[i] ; } ;

private:
    LPCTSTR   m_alpszNames[LEVELS] ;
    int       m_aiValues[LEVELS] ;
    int       m_iCount ;
} ;


//------------------------------------------------------------------------------
// Name: class CDVDLanguages
// Desc: A class to wrap a few language names and the corresponding 2-char codes
//       We have only 10 languages from ISO 639 as a sample. It can be extended to
//       include any other language in ISO 639.
//------------------------------------------------------------------------------
class CDVDLanguages 
{
public:
    CDVDLanguages() ;

    bool GetLangString(LCID LangCode, PTSTR pszLang, int iMaxLen) ;

private:
    LCID  m_lcidCodes[10] ;
    LPCTSTR  m_apszLangNames[10] ;
} ;

//------------------------------------------------------------------------------
// Remove Debug logging
//------------------------------------------------------------------------------
#ifndef DbgLog
#define DbgLog(a)
#endif
#ifndef ASSERT
#define ASSERT(x)
#endif

#ifndef DbgInitialise
#define DbgInitialise(x) 
#endif

#ifndef DbgTerminate
#define DbgTerminate()
#endif

#pragma warning ( disable : 4390 )
