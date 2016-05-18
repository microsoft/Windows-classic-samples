//////////////////////////////////////////////////////////////////////////
// MainWindow.cpp: Main application window.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#include "DshowPlayer.h"
#include "MainWindow.h"


const LONG		ONE_MSEC = 10000;   // The number of 100-ns in 1 msec

const UINT_PTR	IDT_TIMER1 = 1;		// Timer ID
const UINT		TICK_FREQ = 200;	// Timer frequency in msec		

// Forward declarations of functions included in this code module:
void NotifyError(HWND hwnd, TCHAR* sMessage, HRESULT hrStatus);



//-----------------------------------------------------------------------------
// MainWindow constructor.
//-----------------------------------------------------------------------------

MainWindow::MainWindow() : brush(NULL), m_timerID(0), m_pPlayer(NULL)
{
}


//-----------------------------------------------------------------------------
// MainWindow destructor.
//-----------------------------------------------------------------------------

MainWindow::~MainWindow()
{
	if (brush)
	{
		DeleteObject(brush);
	}

	StopTimer();

	SAFE_DELETE(m_pPlayer);
}


//-----------------------------------------------------------------------------
// MainWindow::OnReceiveMessage
// Description: Handles window messages
//-----------------------------------------------------------------------------

LRESULT MainWindow::OnReceiveMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	HRESULT hr;

	switch (message)
	{

	case WM_CREATE:
		hr = OnCreate();
		if (FAILED(hr))
		{
			// Fail and quit.
			NotifyError(m_hwnd, TEXT("Cannot initialize the application."), hr);
			return -1;
		}
		break;
		
	case WM_SIZE:
		OnSize();
		break;

	case WM_PAINT:
		OnPaint();
		break;

	case WM_MOVE:
		OnPaint();
		break;

	case WM_DISPLAYCHANGE:
		m_pPlayer->DisplayModeChanged();
		break;

	case WM_ERASEBKGND:
		return 1;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_TIMER:
		OnTimer();
		break;

	case WM_NOTIFY:
        OnWmNotify((NMHDR*)lParam);
		break;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case IDM_EXIT:
			DestroyWindow(m_hwnd);
			break;

		case ID_FILE_OPENFILE:
			OnFileOpen();
			break;

		case IDC_BUTTON_PLAY:
			OnPlay();
			break;

		case IDC_BUTTON_STOP:
			OnStop();
			break;

		case IDC_BUTTON_PAUSE:
			OnPause();
			break;
	
		case IDC_BUTTON_MUTE:
			OnMute();
			break;
		}
		break;

    // Private filter graph message.
	case WM_GRAPH_EVENT:
		hr = m_pPlayer->HandleGraphEvent(this);
		break;

	default:
		return BaseWindow::OnReceiveMessage(message, wParam, lParam);
	}
	return 0;
}


//-----------------------------------------------------------------------------
// MainWindow::OnCreate
// Description: Called when the window is created.
//-----------------------------------------------------------------------------

HRESULT MainWindow::OnCreate()
{
	HRESULT hr = S_OK;

	// Create the background brush.
	brush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0x80, 0xFF));
	if (brush == NULL)
	{
		hr = __HRESULT_FROM_WIN32(GetLastError());
	}

	// Create the rebar control.
	if (SUCCEEDED(hr))
	{
		hr = rebar.Create(m_hInstance, m_hwnd, IDC_REBAR_CONTROL);
	}

	// Create the toolbar control.
	if (SUCCEEDED(hr))
	{
		hr = toolbar.Create(m_hInstance, m_hwnd, IDC_TOOLBAR, TBSTYLE_FLAT | TBSTYLE_TOOLTIPS);
	}

	// Set the image list for toolbar buttons (normal state).
	if (SUCCEEDED(hr))
	{
		hr = toolbar.SetImageList(
			Toolbar::Normal,			// Image list for normal state
			IDB_TOOLBAR_IMAGES_NORMAL,	// Bitmap resource
			Size(48, 48),				// Size of each button
			5,							// Number of buttons
			RGB(0xFF, 0x00, 0xFF)		// Color mask
			);
	}

	// Set the image list for toolbar buttons (disabled state).
	if (SUCCEEDED(hr))
	{
		hr = toolbar.SetImageList(
			Toolbar::Disabled,			// Image list for normal state
			IDB_TOOLBAR_IMAGES_DISABLED,	// Bitmap resource
			Size(48, 48),				// Size of each button
			5,							// Number of buttons
			RGB(0xFF, 0x00, 0xFF)		// Color mask
			);
	}


	// Add buttons to the toolbar.
	if (SUCCEEDED(hr))
	{
		// Play
		hr = toolbar.AddButton(Toolbar::Button(ID_IMAGE_PLAY, IDC_BUTTON_PLAY));
	}
	
	if (SUCCEEDED(hr))
	{
		// Stop
		hr = toolbar.AddButton(Toolbar::Button(ID_IMAGE_STOP, IDC_BUTTON_STOP));
	}

	if (SUCCEEDED(hr))
	{
		// Pause
		hr = toolbar.AddButton(Toolbar::Button(ID_IMAGE_PAUSE, IDC_BUTTON_PAUSE));
	}

	if (SUCCEEDED(hr))
	{
		// Mute
		hr = toolbar.AddButton(Toolbar::Button(ID_IMAGE_MUTE_OFF, IDC_BUTTON_MUTE));
	}

	// Add the toolbar to the rebar control.
	if (SUCCEEDED(hr))
	{
		hr = rebar.AddBand(toolbar.Window(), 0);
	}

	//// Create the slider for seeking.

	if (SUCCEEDED(hr))
	{
		hr = Slider_Init();	// Initialize the Slider control.
	}

	if (SUCCEEDED(hr))
	{
		hr = seekbar.Create(m_hwnd, Rect(0, 0, 300, 16), IDC_SEEKBAR);
	}

	if (SUCCEEDED(hr))
	{
		hr = seekbar.SetThumbBitmap(IDB_SLIDER_THUMB);

		seekbar.SetBackground(CreateSolidBrush(RGB(239, 239, 231)));
		seekbar.Enable(FALSE);
	}

	if (SUCCEEDED(hr))
	{
		hr = rebar.AddBand(seekbar.Window(), 1);
	}

	//// Create the slider for changing the volume.

	if (SUCCEEDED(hr))
	{
		hr = volumeSlider.Create(m_hwnd, Rect(0, 0, 100, 32), IDC_VOLUME);
	}

	if (SUCCEEDED(hr))
	{
		hr = volumeSlider.SetThumbBitmap(IDB_SLIDER_VOLUME);

		volumeSlider.SetBackground(CreateSolidBrush(RGB(239, 239, 231)));
		volumeSlider.Enable(TRUE);

		// Set the range of the volume slider. In my experience, only the top half of the
		// range is audible.
		volumeSlider.SetRange(MIN_VOLUME / 2, MAX_VOLUME);
		volumeSlider.SetPosition(MAX_VOLUME);
	}

	if (SUCCEEDED(hr))
	{
		hr = rebar.AddBand(volumeSlider.Window(), 2);
	}



	// Create the DirectShow player object.
	if (SUCCEEDED(hr))
	{
		m_pPlayer = new DShowPlayer(m_hwnd);
		if (m_pPlayer == NULL)
		{
			hr = E_OUTOFMEMORY;
		}
	}

	// Set the event notification window.
	if (SUCCEEDED(hr))
	{
		hr = m_pPlayer->SetEventWindow(m_hwnd, WM_GRAPH_EVENT);
	}

	// Set default UI state.
	if (SUCCEEDED(hr))
	{
		UpdateUI();
	}

	return hr;
}



//-----------------------------------------------------------------------------
// MainWindow::OnPaint
// Description: Called when the window should be painted.
//-----------------------------------------------------------------------------

void MainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc;

	hdc = BeginPaint(m_hwnd, &ps);

	if (m_pPlayer->State() != STATE_CLOSED && m_pPlayer->HasVideo())
	{
		// The player has video, so ask the player to repaint. 
		m_pPlayer->Repaint(hdc);
	}
	else
	{
		// The player does not have video. Fill in our client region, not 
		// including the area for the toolbar.

		RECT rcClient;
		RECT rcToolbar;

		GetClientRect(m_hwnd, &rcClient);
		GetClientRect(rebar.Window(), &rcToolbar);

		HRGN hRgn1 = CreateRectRgnIndirect(&rcClient);
		HRGN hRgn2 = CreateRectRgnIndirect(&rcToolbar);

		CombineRgn(hRgn1, hRgn1, hRgn2, RGN_DIFF);

		FillRgn(hdc, hRgn1, brush);

		DeleteObject(hRgn1);
		DeleteObject(hRgn2);
	}

	EndPaint(m_hwnd, &ps);
}


//-----------------------------------------------------------------------------
// MainWindow::OnSize
// Description: Called when the window is resized.
//-----------------------------------------------------------------------------

void MainWindow::OnSize()
{
	// resize the toolbar
	SendMessage(toolbar.Window(), WM_SIZE, 0, 0);

	// resize the rebar 
	SendMessage(rebar.Window(), WM_SIZE, 0, 0);

	RECT rcWindow;
	RECT rcControl;

	// Find the client area of the application.
	GetClientRect(m_hwnd, &rcWindow);

	// Subtract the area of the rebar control.
	GetClientRect(rebar.Window(), &rcControl);
	SubtractRect(&rcWindow, &rcWindow, &rcControl);

	// What's left is the area for the video. Notify the player.
	m_pPlayer->UpdateVideoWindow(&rcWindow);
}


//-----------------------------------------------------------------------------
// MainWindow::OnTimer
// Description: Called when the timer elapses.
//-----------------------------------------------------------------------------

void MainWindow::OnTimer()
{
    // If the player can seek, update the seek bar with the current position.
	if (m_pPlayer->CanSeek())
	{
        REFERENCE_TIME timeNow;

        if (SUCCEEDED(m_pPlayer->GetCurrentPosition(&timeNow)))
        {
			seekbar.SetPosition((LONG)(timeNow / ONE_MSEC));
        }
    }
}


//-----------------------------------------------------------------------------
// MainWindow::OnWmNotify
// Description: Handle WM_NOTIFY messages.
//-----------------------------------------------------------------------------

void MainWindow::OnWmNotify(const NMHDR *pHdr)
{
    switch (pHdr->code)
    {
    case TTN_GETDISPINFO:
        // Display tool tips
        toolbar.ShowToolTip((NMTTDISPINFO*)pHdr);           
        break;

    default:
		switch (pHdr->idFrom)
		{
		case IDC_SEEKBAR:
			OnSeekbarNotify((NMSLIDER_INFO*)pHdr);
			break;

		case IDC_VOLUME:
			OnVolumeSliderNotify((NMSLIDER_INFO*)pHdr);
			break;

        }
        break;
    }
}

//-----------------------------------------------------------------------------
// MainWindow::OnSeekbarNotify
// Description: Handle WM_NOTIFY messages from the seekbar.
//-----------------------------------------------------------------------------

void MainWindow::OnSeekbarNotify(const NMSLIDER_INFO *pInfo)
{
	static PlaybackState state = STATE_CLOSED;

	// Pause when the scroll action begins.
	if (pInfo->hdr.code == SLIDER_NOTIFY_SELECT) 
	{
		state = m_pPlayer->State();
		m_pPlayer->Pause();
	}

	// Update the position continuously.
	m_pPlayer->SetPosition(ONE_MSEC * pInfo->position);

	// Restore the state at the end.
	if (pInfo->hdr.code == SLIDER_NOTIFY_RELEASE)
	{
		if (state == STATE_STOPPED)
		{
			m_pPlayer->Stop();
		}
		else if (state == STATE_RUNNING)
		{
			m_pPlayer->Play();
		}
	}
}


//-----------------------------------------------------------------------------
// MainWindow::OnVolumeSliderNotify
// Description: Handle WM_NOTIFY messages from the volume slider.
//-----------------------------------------------------------------------------

void MainWindow::OnVolumeSliderNotify(const NMSLIDER_INFO *pInfo)
{
	m_pPlayer->SetVolume(pInfo->position);
}


//-----------------------------------------------------------------------------
// MainWindow::OnFileOpen
// Description: Open a new file for playback.
//-----------------------------------------------------------------------------

void MainWindow::OnFileOpen()
{
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));

	WCHAR szFileName[MAX_PATH];
	szFileName[0] = L'\0';

	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = m_hwnd;
	ofn.hInstance = m_hInstance;
	ofn.lpstrFilter = L"All (*.*)/0*.*/0";
	ofn.lpstrFile = szFileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_FILEMUSTEXIST;

	HRESULT hr;

	if (GetOpenFileName(&ofn))
	{
		hr = m_pPlayer->OpenFile(szFileName);

		// Update the state of the UI. 
		UpdateUI();

		// Invalidate the appliction window, in case there is an old video 
		// frame from the previous file and there is no video now. (eg, the
		// new file is audio only, or we failed to open this file.)
		InvalidateRect(m_hwnd, NULL, FALSE);

		// Update the seek bar to match the current state.
		UpdateSeekBar();


		if (SUCCEEDED(hr))
		{
			// If this file has a video stream, we need to notify 
			// the VMR about the size of the destination rectangle.
			// Invoking our OnSize() handler does this.
			OnSize();
		}
		else
		{
			NotifyError(m_hwnd, TEXT("Cannot open this file."), hr);
		}

	}
}

//-----------------------------------------------------------------------------
// MainWindow::OnPlay
// Description: Start playback.
//-----------------------------------------------------------------------------

void MainWindow::OnPlay()
{
	m_pPlayer->Play();

	UpdateUI();
}

//-----------------------------------------------------------------------------
// MainWindow::OnStop
// Description: Stop playback.
//-----------------------------------------------------------------------------

void MainWindow::OnStop()
{
	HRESULT hr = m_pPlayer->Stop();

	// Seek back to the start. 
	if (SUCCEEDED(hr))
	{
		if (m_pPlayer->CanSeek())
		{
			hr = m_pPlayer->SetPosition(0);
		}
	}

	UpdateUI();
}

//-----------------------------------------------------------------------------
// MainWindow::OnPause
// Description: Pause playback.
//-----------------------------------------------------------------------------

void MainWindow::OnPause()
{
	m_pPlayer->Pause();

	UpdateUI();
}


//-----------------------------------------------------------------------------
// MainWindow::OnMute
// Description: Toggle the muted / unmuted state.
//-----------------------------------------------------------------------------

void MainWindow::OnMute()
{
	if (m_pPlayer->IsMuted())
	{
		m_pPlayer->Mute(FALSE);
		toolbar.SetButtonImage(IDC_BUTTON_MUTE, ID_IMAGE_MUTE_OFF);
	}
	else
	{
		m_pPlayer->Mute(TRUE);
		toolbar.SetButtonImage(IDC_BUTTON_MUTE, ID_IMAGE_MUTE_ON);
	}
}



//-----------------------------------------------------------------------------
// MainWindow::OnGraphEvent
// Description: Callback to handle events from the filter graph.
//-----------------------------------------------------------------------------

// ! It is very important that the application does not tear down the graph inside this
// callback.

void MainWindow::OnGraphEvent(long eventCode, LONG_PTR param1, LONG_PTR param2)
{
	switch (eventCode)
	{
	case EC_COMPLETE:
		OnStop();
		break;
	}
}

//-----------------------------------------------------------------------------
// MainWindow::UpdateUI
// Description: Update the UI based on the current playback state.
//-----------------------------------------------------------------------------

void MainWindow::UpdateUI()
{
	BOOL bPlay = FALSE;
	BOOL bPause = FALSE;
	BOOL bStop = FALSE;

	switch (m_pPlayer->State())
	{
	case STATE_RUNNING:
		bPause = TRUE;
		bStop = TRUE;
		break;

	case STATE_PAUSED:
		bPlay = TRUE;
		bStop = TRUE;
		break;

	case STATE_STOPPED:
		bPlay = TRUE;
		break;
	}

	toolbar.Enable(IDC_BUTTON_PLAY, bPlay);
	toolbar.Enable(IDC_BUTTON_PAUSE, bPause);
	toolbar.Enable(IDC_BUTTON_STOP, bStop);

}

//-----------------------------------------------------------------------------
// MainWindow::UpdateSeekBar
// Description: Update the seekbar based on the current playback state.
//-----------------------------------------------------------------------------

void MainWindow::UpdateSeekBar()
{
    // If the player can seek, set the seekbar range and start the time.
    // Otherwise, disable the seekbar.
	if (m_pPlayer->CanSeek())
	{
		seekbar.Enable(TRUE);

		LONGLONG rtDuration = 0;
		m_pPlayer->GetDuration(&rtDuration);

		seekbar.SetRange(0, (LONG)(rtDuration / ONE_MSEC));

		// Start the timer

		m_timerID = SetTimer(m_hwnd, IDT_TIMER1, TICK_FREQ, NULL);
	}
	else
	{
		seekbar.Enable(TRUE);//  FALSE);

		// Stop the old timer, if any.
		StopTimer();
	}
}


//-----------------------------------------------------------------------------
// MainWindow::StopTimer
// Description: Stops the timer.
//-----------------------------------------------------------------------------

void  MainWindow::StopTimer()
{
	if (m_timerID != 0)
	{
		KillTimer(m_hwnd, m_timerID);
		m_timerID = 0;
	}
}


void NotifyError(HWND hwnd, TCHAR* sMessage, HRESULT hrStatus)
{
	TCHAR sTmp[512];

	HRESULT hr = StringCchPrintf(sTmp, 512, TEXT("%s hr = 0x%X"), sMessage, hrStatus);

	if (SUCCEEDED(hr))
	{
		MessageBox(hwnd, sTmp, TEXT("Error"), MB_OK | MB_ICONERROR);
	}
}


