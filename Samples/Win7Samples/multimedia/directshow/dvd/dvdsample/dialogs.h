//------------------------------------------------------------------------------
// File: Dialogs.h
//
// Desc: This file contains the header information for the various dialog
//       wrapper classes we use.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: class CAboutDlg
// Desc: This class is a wrapper for the About Dialog
//------------------------------------------------------------------------------

class CAboutDlg  
{
public:
	static BOOL CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CAboutDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CAboutDlg();

private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CSPLangDlg
// Desc: This class is a wrapper for the Subpicture Language Selection Dialog
//------------------------------------------------------------------------------

class CSPLangDlg  
{
public:
	static BOOL CALLBACK SPDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CSPLangDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CSPLangDlg();

private:
	bool GetSPLang(ULONG ulStream, TCHAR * buffer, int iBufLen);
	int MakeSPStreamList(HWND hDlg, int iListID);

    ULONG m_ulSPStream; // the current SP stream
	BOOL m_bSPOn; // the state of the SP stream (on/off) - we use BOOL instead of bool because
                  // this is what GetCurrentSubpicture returns to us
	ULONG m_ulNumLang; // the number of streams available
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CAudioLangDlg
// Desc: This class is a wrapper for the Audio Language Selection Dialog
//------------------------------------------------------------------------------

class CAudioLangDlg  
{
public:
	static BOOL CALLBACK AudioDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CAudioLangDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CAudioLangDlg();

private:
	bool GetAudioLang(ULONG ulStream, TCHAR * buffer, int iBufLen);
	int MakeAudioStreamList(HWND hDlg, int iListID);

    ULONG m_ulAudioStream; // the current Audio stream
	ULONG m_ulNumLang; // the number of streams available
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CAngleDlg
// Desc: This class is a wrapper for the Angle Selection Dialog
//------------------------------------------------------------------------------

class CAngleDlg  
{
public:
	static BOOL CALLBACK AngleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CAngleDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CAngleDlg();

private:
	int MakeAngleList(HWND hDlg, int iListID);

    ULONG m_ulAngle; // the current Angle
	ULONG m_ulNumAngle; // the number of angles available
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CChapterDlg
// Desc: This class is a wrapper for the Chapter Selection Dialog
//------------------------------------------------------------------------------

class CChapterDlg  
{
public:
    ULONG GetChapter() { return m_ulChapter; }
	static BOOL CALLBACK ChapterDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CChapterDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CChapterDlg();

private:
	ULONG m_ulChapter; // the chosen Chapter
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CTitleDlg
// Desc: This class is a wrapper for the Title Selection Dialog
//------------------------------------------------------------------------------

class CTitleDlg  
{
public:
    ULONG GetChapter() { return m_ulChapter; }
    ULONG GetTitle() { return m_ulTitle; }
	static BOOL CALLBACK TitleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CTitleDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CTitleDlg();

private:
	ULONG m_ulChapter; // the chosen Chapter
    ULONG m_ulTitle; // the chosen Title
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CTimeDlg
// Desc: This class is a wrapper for the Time Selection Dialog
//------------------------------------------------------------------------------

class CTimeDlg  
{
public:
    DVD_HMSF_TIMECODE GetTime() { return m_Time; }
    void SetTime(DVD_HMSF_TIMECODE time) { m_Time = time; }
	static BOOL CALLBACK TimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CTimeDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CTimeDlg();

private:
	DVD_HMSF_TIMECODE m_Time; // the chosen time
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


//------------------------------------------------------------------------------
// Name: class CKaraokeDlg
// Desc: This class is a wrapper for the Karaoke Mixing Dialog
//------------------------------------------------------------------------------

class CKaraokeDlg  
{
public:
	static BOOL CALLBACK KaraokeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	bool DoModal();
	CKaraokeDlg(HINSTANCE hInstance, HWND hWnd);
	virtual ~CKaraokeDlg();

private:
	const TCHAR * m_pszChannel2;
	const TCHAR * m_pszChannel3;
	const TCHAR * m_pszChannel4;
	HINSTANCE m_hInstance;
	HWND m_hWnd;
};


