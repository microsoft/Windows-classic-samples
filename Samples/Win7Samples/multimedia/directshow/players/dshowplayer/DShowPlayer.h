//////////////////////////////////////////////////////////////////////////
// DShowPlayer.h: Implements DirectShow playback functionality.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef WINVER				// Allow use of features specific to Windows 95 and Windows NT 4 or later.
#define WINVER 0x0501		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows NT 4 or later.
#define _WIN32_WINNT 0x0600		// Change this to the appropriate value to target Windows 98 and Windows 2000 or later.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 4.0 or later.
#define _WIN32_IE 0x0501	// Change this to the appropriate value to target IE 5.0 or later.
#endif

// Windows Header Files:
#include <windows.h>
#include <strsafe.h>
#include <dshow.h>

// Include the v6 common controls in the manifest
#pragma comment(linker, \
    "\"/manifestdependency:type='Win32' "\
    "name='Microsoft.Windows.Common-Controls' "\
    "version='6.0.0.0' "\
    "processorArchitecture='*' "\
    "publicKeyToken='6595b64144ccf1df' "\
    "language='*'\"")

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if (x) { x->Release(); x = NULL; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { delete x; x = NULL; }
#endif


#include "video.h"

const long MIN_VOLUME = -10000;
const long MAX_VOLUME = 0;

enum PlaybackState
{
	STATE_RUNNING,
	STATE_PAUSED,
	STATE_STOPPED,
	STATE_CLOSED
};

struct GraphEventCallback
{
	virtual void OnGraphEvent(long eventCode, LONG_PTR param1, LONG_PTR param2) = 0;
};


class DShowPlayer
{
public:

	DShowPlayer(HWND hwndVideo);
	~DShowPlayer();

	HRESULT SetEventWindow(HWND hwnd, UINT msg);

	PlaybackState State() const { return m_state; }

	HRESULT OpenFile(const WCHAR* sFileName);
	
	// Streaming
	HRESULT Play();
	HRESULT Pause();
	HRESULT Stop();

	// VMR functionality
	BOOL    HasVideo() const;
	HRESULT UpdateVideoWindow(const LPRECT prc);
	HRESULT Repaint(HDC hdc);
	HRESULT DisplayModeChanged();

	// events
	HRESULT HandleGraphEvent(GraphEventCallback *pCB);

	// seeking
	BOOL	CanSeek() const;
	HRESULT SetPosition(REFERENCE_TIME pos);
	HRESULT GetDuration(LONGLONG *pDuration);
	HRESULT GetCurrentPosition(LONGLONG *pTimeNow);

	// Audio
	HRESULT	Mute(BOOL bMute);
	BOOL	IsMuted() const { return m_bMute; }
	HRESULT	SetVolume(long lVolume);
	long	GetVolume() const { return m_lVolume; }

private:
	HRESULT InitializeGraph();
	void	TearDownGraph();
    HRESULT CreateVideoRenderer();
	HRESULT	RenderStreams(IBaseFilter *pSource);
	HRESULT UpdateVolume();

	PlaybackState	m_state;

	HWND			m_hwndVideo;	// Video clipping window
	HWND			m_hwndEvent;	// Window to receive events
	UINT			m_EventMsg;		// Windows message for graph events

	DWORD			m_seekCaps;		// Caps bits for IMediaSeeking

	// Audio
    BOOL            m_bAudioStream; // Is there an audio stream?
	long			m_lVolume;		// Current volume (unless muted)
	BOOL			m_bMute;		// Volume muted?		

	IGraphBuilder	*m_pGraph;
	IMediaControl	*m_pControl;
	IMediaEventEx	*m_pEvent;
	IMediaSeeking	*m_pSeek;
	IBasicAudio		*m_pAudio;

    BaseVideoRenderer   *m_pVideo;

};

HRESULT RemoveUnconnectedRenderer(IGraphBuilder *pGraph, IBaseFilter *pRenderer, BOOL *pbRemoved);
