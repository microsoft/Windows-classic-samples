//------------------------------------------------------------------------------
// File: DvdCore.h
//
// Desc: Declarations for DVD Playback capabilities
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#if !defined(AFX_DVDCORE_H__30902DC7_7AAD_11D3_9973_00C04F9900F6__INCLUDED_)
#define AFX_DVDCORE_H__30902DC7_7AAD_11D3_9973_00C04F9900F6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <tchar.h>
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include <dshow.h>
#include <d3d9.h>
#include <vmr9.h>
#include <IL21Dec.h>
#include <strsafe.h>

// Common files
#include "utils.h"


#pragma warning(disable:4189)  // C4189: local var initialized but not referenced

//------------------------------------------------------------------------------
// Forward declarations
//------------------------------------------------------------------------------

struct IAMLine21Decoder;
struct IGraphBuilder;
struct IVideoWindow;
struct IMediaEventEx;
struct IMediaControl;
struct IDvdControl2;
struct IDvdInfo2;
struct IDvdGraphBuilder;
class IDvdCallback;
class CSPLangDlg;
class CAudioLangDlg;
class CAngleDlg;
class CKaraokeDlg;


//------------------------------------------------------------------------------
// Global data
//------------------------------------------------------------------------------

// DirectShow Graph state
enum GRAPH_STATE 
{
    Uninitialized = 0, 
    Graph_Stopped1, // stopped without reset 
    Graph_Stopped2, // stopped with reset
    Nav_Stopped,
    Playing,
    Graph_Paused,
    Nav_Paused,
    Scanning
} ;

const DWORD WM_DVD_EVENT = WM_USER+100;


//------------------------------------------------------------------------------
// Name: class IDvdCallback
// Desc: This class is just the interface for our callback.  We do this so we can establish
//       a pointer to a function that we can call back to update the screen
//------------------------------------------------------------------------------

class IDvdCallback
{
public:
    virtual void UpdateStatus(void) { } // used to tell the app that time or location changed
    virtual void Prohibited(void) { } // used to notify the app that an operation was prohibited
    virtual void Exit(void) { } // used to notify the app that the playback window was closed
    virtual RECT GetAppWindow(void) { RECT r = {0,0,0,0}; return r; }

protected:
//private:
    IDvdCallback() {  } // so no one can instantiate this class directly.
};


//------------------------------------------------------------------------------
// Name: class CDvdCore
// Desc: This class does nothing . It's just here to show what
//       a class header should look like.
//------------------------------------------------------------------------------

class CDvdCore : public IDvdCallback
{
    // These are granted friend status to simplify the code.  These should probably be
    // forced to use some sort of accessor methods, but that would clutter the code too
    // much for this sample.
    friend CSPLangDlg;
    friend CAudioLangDlg;
    friend CAngleDlg;
    friend CKaraokeDlg;

public:
	bool GetSPAttributes( void );
	bool GetVideoAttributes( void );
	bool GetAudioAttributes( void );
	bool GetDvdText( void );
	bool PlayTime(DVD_HMSF_TIMECODE time);
	bool PlayChapterInTitle(ULONG ulTitle, ULONG ulChapter);
	bool PlayChapter(ULONG ulChap);
	bool FrameStep(void);
	bool SetParentalLevel(ULONG ulLevel);
	bool RestoreBookmark();
	bool SaveBookmark(void);
	bool SetVideoWindowTitle(TCHAR * pszTitle);
	bool EnableCaptions(bool bOn);
	bool TitleMenu(void);
	bool ToggleFullScreen( void );
	bool RootMenu(void);
	bool Pause(void);
	bool FastForward(void);
	bool Rewind(void);
	bool Stop(void);
	bool PrevChapter(void);
	bool NextChapter(void);
	bool Init();
	bool SetDirectory(TCHAR * szDirectory);
    bool ToggleVMR9AndRebuildGraph(void);

	ULONG GetParentalLevel(void);
	HRESULT Play();

	explicit CDvdCore(HINSTANCE hInstance, IDvdCallback * pIDC = NULL);
	virtual ~CDvdCore();

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, 
        LPARAM lParam);

	LRESULT OnKeyEvent(WPARAM wParam, LPARAM lParam);

    inline GRAPH_STATE GetState(void) { return m_eState; }
    inline DVD_HMSF_TIMECODE & GetTime(void) { return m_CurTime; }
    inline ULONG GetTitle(void) { return m_ulCurTitle; }
    inline ULONG GetChapter(void) { return m_ulCurChapter; }

private:
	LRESULT OnClose( void );
	LRESULT OnMouseEvent(UINT uMessage, WPARAM wParam, LPARAM lParam);
	LRESULT OnDvdEvent(UINT uMessage, WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseTimer(WPARAM wParam, LPARAM lParam);
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);

	void UnInitMessageSink();
	void ShowMouseCursor(bool bShow);
	void ReleaseInterfaces();
    void UpdateStatus(void); // this will be am empty implementation to allow us to call
                             // m_pCallback without checking for NULL

	bool DoesFileExist(PTSTR pszFile);
	bool GetDriveLetter(TCHAR * pszDrive, DWORD cchDrive);
	bool ChangeDvdRegion(void);
	bool StopFullScreen(void);
	bool StartFullScreen(void);
	bool SetPlaybackOptions(void);
	bool InitMessageSink(void);
	bool BuildGraph();

	DWORD GetStatusText(AM_DVD_RENDERSTATUS *pStatus, PTSTR pszStatusText, DWORD dwMaxText);
    inline void SetState(GRAPH_STATE state) { m_eState = state; };
	RECT GetPlaybackWindowRect( void );

	ULONG m_ulCurChapter;           // track the current chapter number
	ULONG m_ulCurTitle;             // track the current title number
	DVD_HMSF_TIMECODE m_CurTime;    // track the current playback time
	IDvdCallback * m_pCallback;     // pointer to some class implementing the IDvdCallback interface
	bool m_bMenuOn;                 // we are in a menu
	bool m_bFirstPlay;              // Used to track if this is the first time we're playing or not
	bool m_bFullScreenOn;           // used to track if we are in fullscreen mode or not
	RECT m_RectOrigVideo;           // used to store the original video rectangle during fullscreen playback
    LONG m_lOrigStyle ;             // original video window style bits (before fullscreen)
    LONG m_lOrigStyleEx ;           // original video window extended style bits (.....)
    HHOOK m_hMouseHook ;            // hook handle for mouse messages in fullscreen mode
    DWORD m_dwMouseMoveTime ;       // last time the mouse moved (in milliseconds)
    int   m_iMouseShowCount ;       // ShowCursor() returned mouse state
	HWND m_hWnd;                    // the hWnd for our container window
	bool m_bStillOn;                // used to track if there is a still frame on or not
	HINSTANCE m_hInstance;          // the hInstance of the calling program
    IAMLine21Decoder * m_pIL21Dec;  // IAMLine21Decoder interface
	IGraphBuilder * m_pGraph;       // IGraphBuilder interface
	IVideoWindow * m_pIVW;          // IVideoWindow interface
	IMediaEventEx * m_pIME;         // IMediaEventEx interface
	IMediaControl * m_pIMC;         // IMediaControl interface
	IDvdControl2 * m_pIDvdC2;       // IDvdControl2 interface
	IDvdInfo2 * m_pIDvdI2;          // IDvdInfo2 interface
	IDvdGraphBuilder * m_pIDvdGB;   // IDvdGraphBuilder interface
	DWORD m_dwRenderFlags;          // the flags used to render the graph.  May be used to set different flags.
	TCHAR m_szDiscPath[MAX_PATH];   // may be used to set the initial disc path
    GRAPH_STATE m_eState;           // the state of our current graph
	bool m_bMessageSink;			// did we set up the message sink?
    bool m_bUseVMR9;                // flag to indicate the usage of VMR9 in the DVD graph
};  //  class CDvdCore

//------------------------------------------------------------------------------
// Remove Debug logging
//------------------------------------------------------------------------------
#ifndef DbgLog
#define DbgLog(a)
#endif
#ifndef ASSERT
#define ASSERT(x)
#endif

///////////////////////////////////////////////////////////////////////////
// The following definitions come from the Platform SDK and are required if
// the application is being compiled with the headers from Visual C++ 6.0.
///////////////////////////////////////////////////////////////////////////
#ifndef GetWindowLongPtr
  #define GetWindowLongPtrA   GetWindowLongA
  #define GetWindowLongPtrW   GetWindowLongW
  #ifdef UNICODE
    #define GetWindowLongPtr  GetWindowLongPtrW
  #else
    #define GetWindowLongPtr  GetWindowLongPtrA
  #endif // !UNICODE
#endif // !GetWindowLongPtr

#ifndef SetWindowLongPtr
  #define SetWindowLongPtrA   SetWindowLongA
  #define SetWindowLongPtrW   SetWindowLongW
  #ifdef UNICODE
    #define SetWindowLongPtr  SetWindowLongPtrW
  #else
    #define SetWindowLongPtr  SetWindowLongPtrA
  #endif // !UNICODE
#endif // !SetWindowLongPtr

#ifndef GWLP_USERDATA
  #define GWLP_USERDATA       (-21)
#endif
///////////////////////////////////////////////////////////////////////////
// End Platform SDK definitions
///////////////////////////////////////////////////////////////////////////


#endif // !defined(AFX_DVDCORE_H__30902DC7_7AAD_11D3_9973_00C04F9900F6__INCLUDED_)
