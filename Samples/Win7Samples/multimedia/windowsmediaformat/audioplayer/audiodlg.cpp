//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            AudioDlg.cpp
//
// Abstract:            Implementation of AudioPlayer's UI.
//
//*****************************************************************************

#include "stdafx.h"
#include <Commdlg.h>
#include <commctrl.h>
#include <stdio.h>
#include "AudioPlay.h"
#include "AudioDlg.h"

//
// Global variables
//
AUDIOSTATUS g_Status;
BOOL		g_IsSeeking;
CAudioPlay	*g_pAudioplay;
HINSTANCE	g_hInst;
HWND		g_hwndDialog;
TCHAR		g_ptszFileName[ MAX_PATH ];
int		    g_iDlgHeight;

//------------------------------------------------------------------------------
// Name: DlgProc()
// Desc: Dialog box procedure.
//------------------------------------------------------------------------------
INT_PTR CALLBACK DlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	HRESULT	hr		= S_OK;
	HICON	hIcon	= NULL;
	RECT	rect;

	switch( uMsg )
	{
	case WM_INITDIALOG:
		
		g_hwndDialog = hwndDlg;

		//
		// Load the application icon
		//
		hIcon = LoadIcon( g_hInst, MAKEINTRESOURCE( IDI_WMAICON ) );
		if( hIcon )
		{
			SendMessage( hwndDlg, WM_SETICON, ICON_SMALL, ( LPARAM )hIcon );
			SendMessage( hwndDlg, WM_SETICON, ICON_BIG, ( LPARAM )hIcon );
		}

		GetWindowRect( hwndDlg, &rect );

		//
		// Store the Window height in a global variable for future reference
		//
		g_iDlgHeight = rect.bottom - rect.top;

        //
        // Ready to open and play a file
        //
		SetCurrentStatus( READY );
		
        //
        // Create and initialize the audio player
        //
        g_pAudioplay = new CAudioPlay;
        if( NULL == g_pAudioplay )
        {
			//
			// Creation has failed. Close the application.
			//
			SendMessage( hwndDlg, WM_CLOSE, 0,0 );
            return TRUE;
        }

		hr = g_pAudioplay->Init();
		if( FAILED(hr) )
		{
			//
			// Init has failed. Close the application.
			//
			SendMessage( hwndDlg, WM_CLOSE, 0,0 );
		}

		return TRUE;

	case WM_COMMAND:

		switch( LOWORD( wParam ) ) 
		{
		case IDC_FILENAME:

			if( EN_CHANGE == HIWORD( wParam ) )
			{
				//
                // Filename has been changed
				// Use this notification for enabling or disabling the Play button
				//
				TCHAR tszFileName[ MAX_PATH ];

				GetDlgItemText( hwndDlg, IDC_FILENAME, tszFileName, MAX_PATH );

                //
                // If filename is not empty, enable the Play button
                //
				if( _tcslen( tszFileName) > 0 )
				{
					EnableWindow( GetDlgItem( hwndDlg, IDC_PLAY ), TRUE );

					SetCurrentStatus( CLOSED );
				}
				else
				{
					EnableWindow( GetDlgItem( hwndDlg, IDC_PLAY ), FALSE );
				}
				
				SetCurrentStatus( READY );
				
				return TRUE;
			}
			
			return FALSE;

		case IDC_OPEN:

            //
            // Show the OpenFile dialog
            //
			if( ShowOpenFileDialog() )
			{
				//
				// Display the file name
				//
				SetDlgItemText( hwndDlg, IDC_FILENAME, g_ptszFileName );

				SetFocus( GetDlgItem( hwndDlg, IDC_PLAY ) );
			}

			return TRUE;
		
		case IDC_STOP:
			
			SetCurrentStatus( STOPPING );

            //
            // Stop the audio player
            //
            if( NULL != g_pAudioplay )
            {
			    hr = g_pAudioplay->Stop();
			    if( FAILED( hr ) )
			    {
				    SetCurrentStatus( g_Status );

				    TCHAR tszErrMesg[128];
                    (void)StringCchPrintf( tszErrMesg, ARRAYSIZE(tszErrMesg), _T( "Unable to Stop (hr=%#X)" ), hr );
				    MessageBox( hwndDlg, tszErrMesg, ERROR_DIALOG_TITLE, MB_OK );
			    }
            }

			return TRUE;
			
		case IDC_PAUSE:
			
            //
            // Pause the audio player
            //
            if( NULL != g_pAudioplay )
            {
			    hr = g_pAudioplay->Pause();
			    if( FAILED( hr ) )
			    {
				    TCHAR tszErrMesg[128];
                    (void)StringCchPrintf( tszErrMesg, ARRAYSIZE(tszErrMesg), _T("Unable to Pause (hr=%#X)"), hr );
				    MessageBox( hwndDlg, tszErrMesg, ERROR_DIALOG_TITLE, MB_OK );
			    }
			    else
			    {
				    SetCurrentStatus( PAUSE );
			    }
            }

			break;
		
		case IDC_PLAY:
			
			return( OnPlay() );

		case IDCANCEL:
			//
			// Close the player before exiting application
			//
            if( NULL != g_pAudioplay )
            {
			    g_pAudioplay->Exit();
			    g_pAudioplay->Release();
            }

			EndDialog( hwndDlg, wParam );

			return TRUE;
		}
		break;

	case WM_HSCROLL:

        if( NULL == g_pAudioplay )
        {
            break;
        }

		//
		// Seek only when the file is seekable
		//
		if( ( LOWORD( wParam ) == TB_THUMBTRACK || 
			  LOWORD( wParam ) == TB_BOTTOM ||
			  LOWORD( wParam ) == TB_PAGEDOWN ||
			  LOWORD( wParam ) == TB_PAGEUP ||
			  LOWORD( wParam ) == TB_TOP ) &&
            g_pAudioplay->IsSeekable() )
		{
			//
			// Set g_IsSeeking, to be referenced when thumb tracking is over
			//
			g_IsSeeking = TRUE;
		}
		else if( LOWORD( wParam ) == TB_ENDTRACK && g_pAudioplay->IsSeekable() && g_IsSeeking )
		{
			DWORD_PTR dwPos = SendDlgItemMessage( hwndDlg, IDC_SLIDER, TBM_GETPOS, 0, 0 );

			// 
			// Start the file from the new position
			//
			hr = g_pAudioplay->Start( ( QWORD )dwPos * 10000 );
			if( FAILED ( hr ) )
			{
				g_IsSeeking = FALSE;
			}
		}

		break;
	}

    return FALSE;
}

//------------------------------------------------------------------------------
// Name: SetCurrentStatus()
// Desc: Update the controls.
//------------------------------------------------------------------------------
void SetCurrentStatus( AUDIOSTATUS currentStatus )
{
	RECT rect;

	switch( currentStatus )
	{	
	case READY:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Ready" ) );
		
		GetWindowRect( g_hwndDialog, &rect );
		MoveWindow( g_hwndDialog,
					rect.left,
					rect.top,
					rect.right - rect.left,
					( UINT )( SMALLDLGSIZE *  g_iDlgHeight ),
					TRUE );

		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_OPEN ), TRUE );

		SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, EM_SETREADONLY, FALSE, 0 );
		return;

	case OPENING:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Opening..." ) );

		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), TRUE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PLAY ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_OPEN ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_SLIDER ), FALSE );

		SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, EM_SETREADONLY, TRUE, 0 );
		SetFocus( GetDlgItem( g_hwndDialog, IDC_STOP ) );
		break;

	case PLAY:
		//
		// Reset the global variable which might have been set while seeking
		// or stop operation
		//
		g_IsSeeking = FALSE;

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Playing..." ) );

		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), TRUE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PLAY ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_OPEN ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_SLIDER ), g_pAudioplay->IsSeekable() );
        {
            DWORD_PTR max = SendDlgItemMessage( g_hwndDialog, IDC_SLIDER, TBM_GETRANGEMAX, 0,0 );
        }

		SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, EM_SETREADONLY, ( WPARAM )TRUE, 0 );

		GetWindowRect( g_hwndDialog, &rect );
		MoveWindow( g_hwndDialog,
					rect.left,
					rect.top,
					rect.right - rect.left,
					g_iDlgHeight,
					TRUE );

		if( !g_pAudioplay->IsBroadcast() )
		{
			EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), TRUE );
		    SetFocus( GetDlgItem( g_hwndDialog, IDC_PAUSE ) );
		}
        else
        {
			EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), FALSE );
		    SetFocus( GetDlgItem( g_hwndDialog, IDC_STOP ) );
        }

		break;

	case PAUSE:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Paused" ) );
		
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PLAY ), TRUE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_SLIDER ), FALSE );

		SetFocus( GetDlgItem(g_hwndDialog, IDC_PLAY ) );

		break;

	case CLOSED:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "" ) );

		EnableWindow( GetDlgItem( g_hwndDialog, IDC_OPEN ), TRUE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_SLIDER ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PLAY ), TRUE );

		SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, EM_SETREADONLY, ( WPARAM )FALSE, 0 );
		SendDlgItemMessage( g_hwndDialog, IDC_SLIDER , TBM_SETPOS, TRUE, 0 );
		SendDlgItemMessageW( g_hwndDialog, IDC_DURATION, WM_SETTEXT, 0, ( WPARAM )L"" );

		GetWindowRect( g_hwndDialog, &rect );
		MoveWindow( g_hwndDialog,
					rect.left,
					rect.top,
					rect.right - rect.left,
					( UINT )( SMALLDLGSIZE * g_iDlgHeight),
					TRUE );
		break;

	case STOP:
		
		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Stopped" ) );
		
		SendDlgItemMessage( g_hwndDialog, IDC_SLIDER , TBM_SETPOS, TRUE, 0 ); 

        EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PLAY ), TRUE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_OPEN ), TRUE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_SLIDER ), FALSE );

		SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, EM_SETREADONLY, ( WPARAM ) FALSE, 0 );

		SetFocus( GetDlgItem( g_hwndDialog, IDC_PLAY ) );

		g_IsSeeking = FALSE;
        SetTime( 0, g_pAudioplay->GetFileDuration() );

		break;

	case BUFFERING:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Buffering..." ) );
		return;

	case STOPPING:
		//
		// Since we are going to position the trackbar at the beginning,
		// set the global variable
		//
		g_IsSeeking = TRUE;
		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Trying to Stop...Please Wait" ) );
			
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_PAUSE ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_OPEN ), FALSE );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_SLIDER ), FALSE );

		SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, EM_SETREADONLY, ( WPARAM )TRUE, 0 );
		SetFocus( GetDlgItem( g_hwndDialog, IDC_STATUS ) );

		return;

	case ACQUIRINGLICENSE:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Acquiring License..." ) );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), TRUE );

		{
			LPTSTR ptszFile = _tcsrchr( g_ptszFileName, _T( '\\' ) );
            if( NULL == ptszFile )
            {
			    SetWindowText( g_hwndDialog, g_ptszFileName );
            }
            else
            {
			    SetWindowText( g_hwndDialog, ptszFile + 1 );
            }
		}

		SetFocus( GetDlgItem( g_hwndDialog, IDC_STOP ) );
		break;

	case INDIVIDUALIZING:

		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Individualizing..." ) );
		EnableWindow( GetDlgItem( g_hwndDialog, IDC_STOP ), TRUE );

		{
			LPTSTR ptszFile = _tcsrchr( g_ptszFileName, _T( '\\' ) );
            if( NULL == ptszFile )
            {
			    SetWindowText( g_hwndDialog, g_ptszFileName );
            }
            else
            {
			    SetWindowText( g_hwndDialog, ptszFile + 1 );
            }
		}

		SetFocus( GetDlgItem( g_hwndDialog, IDC_STOP ) );
		break;

	case LICENSEACQUIRED:
		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "License acquired" ) );
		break;

	case INDIVIDUALIZED:
		SetDlgItemText( g_hwndDialog, IDC_STATUS, _T( "Individualization complete" ) );
		break;
	
	default:
		return;
	}

	g_Status = currentStatus;
}

//------------------------------------------------------------------------------
// Name: SetTime()
// Desc: Update the slider and duration label with new time value.
//------------------------------------------------------------------------------
void SetTime( QWORD cnsTimeElapsed, QWORD cnsFileDuration )
{
	//
	// Do not set new time if seeking is going on
	//
	if( g_IsSeeking )
	{
		return;
	}

    DWORD dwSeconds = 0;
    TCHAR tszTime[20];
	TCHAR tszTemp[10];
	UINT  nHours = 0;
    UINT  nMins = 0;
	
	ZeroMemory( (void *)tszTime, sizeof( tszTime ) );

    dwSeconds = ( DWORD )( cnsTimeElapsed / 10000000 );
	nHours = dwSeconds / 60 / 60;
	dwSeconds %= 3600;
	nMins = dwSeconds / 60;
	dwSeconds %= 60;

    //
    // Format the string
    //
	if( 0 != nHours )
	{
		(void)StringCchPrintf( tszTemp, ARRAYSIZE(tszTemp), _T( "%d:" ), nHours );
		(void)StringCchCat( tszTime, ARRAYSIZE(tszTime), tszTemp );
	}
	
	(void)StringCchPrintf( tszTemp, ARRAYSIZE(tszTemp), _T( "%02d:%02d / " ), nMins, dwSeconds );
	(void)StringCchCat( tszTime, ARRAYSIZE(tszTime), tszTemp );
	
	nHours = 0;
    nMins = 0;

    dwSeconds = ( DWORD )( cnsFileDuration / 10000000 );
	nHours = dwSeconds / 60 / 60;
	dwSeconds %= 3600;
	nMins = dwSeconds / 60;
	dwSeconds %= 60;

    if( 0 != nHours )
	{
		(void)StringCchPrintf( tszTemp, ARRAYSIZE(tszTemp), _T( "%d:" ), nHours );
		(void)StringCchCat( tszTime, ARRAYSIZE(tszTime), tszTemp );
	}
	
	(void)StringCchPrintf( tszTemp, ARRAYSIZE(tszTemp), _T( "%02d:%02d" ), nMins, dwSeconds );
	(void)StringCchCat( tszTime, ARRAYSIZE(tszTime), tszTemp );

    SendDlgItemMessage( g_hwndDialog, IDC_SLIDER , TBM_SETPOS, TRUE, ( LONG )( cnsTimeElapsed / 10000 ) );
	SendDlgItemMessage( g_hwndDialog, IDC_DURATION, WM_SETTEXT, 0, ( WPARAM )tszTime );

    return;
}


//------------------------------------------------------------------------------
// Name: SetItemText()
// Desc: Set the text of the specified control.
//------------------------------------------------------------------------------
DWORD_PTR SetItemText( UINT nControlID, LPCWSTR pwszText )
{
#ifdef UNICODE

    return SendDlgItemMessage( g_hwndDialog, nControlID, WM_SETTEXT, 0, ( WPARAM )pwszText );

#else

    //
    // Convert the wide-character string to a multi-byte string before sending it to the control
    //
    size_t cchLen = wcslen( pwszText );
	LPTSTR pszText = new TCHAR[ cchLen + 1 ];
	if( pszText == NULL )
	{
		return 0;
	}

	if( 0 == WideCharToMultiByte( CP_ACP, 0, pwszText, -1, pszText, cchLen + 1, NULL, NULL ) )
	{
		return 0;
	}

    return SendDlgItemMessage( g_hwndDialog, nControlID, WM_SETTEXT, 0, ( WPARAM )pszText );
#endif
}

//------------------------------------------------------------------------------
// Name: OnPlay()
// Desc: Take actions when the Play button is clicked.
//------------------------------------------------------------------------------
BOOL OnPlay()
{
	HRESULT hr = S_OK;

    if( NULL == g_pAudioplay )
    {
        return( E_UNEXPECTED );
    }

	//
	// Check previous status of the player
	//
	switch( g_Status )
	{
	case PAUSE:
		//
		// Player was PAUSEed, now resume it
		//
		hr = g_pAudioplay->Resume();
		if( FAILED( hr ) )
		{
			TCHAR tszErrMesg[ 128 ];
            (void)StringCchPrintf( tszErrMesg, ARRAYSIZE(tszErrMesg), _T( "Unable to resume (hr=%#X)" ), hr );
			MessageBox( g_hwndDialog, tszErrMesg, ERROR_DIALOG_TITLE, MB_OK );
		}
		else
		{
			SetCurrentStatus( PLAY );
		}
		
		break;
	
	case STOP:
        //
        // Player was STOPped, now start it
        //
        hr = g_pAudioplay->Start();
        if( FAILED( hr ) )
        {
            TCHAR tszErrMesg[ 128 ];
            (void)StringCchPrintf( tszErrMesg, ARRAYSIZE(tszErrMesg), _T("Unable to start (hr=%#X)"), hr );
            MessageBox( g_hwndDialog, tszErrMesg, ERROR_DIALOG_TITLE, MB_OK );
        }
        else
        {
            SetCurrentStatus( OPENING );
        }

        break;

    case CLOSED:
        //
        // The play is being called for the current file for the first time.
        // Start playing the file
        //
        SetCurrentStatus( OPENING );
        
        //
        // Get the file name
        //
        GetDlgItemText( g_hwndDialog, IDC_FILENAME, g_ptszFileName, MAX_PATH );

        //
        // Remove leading spaces from the file name        
        //
        TCHAR *ptszTemp = g_ptszFileName;
        while( *ptszTemp == _T(' ') )
        {
            ptszTemp++;
        }

        if( g_ptszFileName != ptszTemp )
        {
            memmove( g_ptszFileName, ptszTemp, sizeof( TCHAR ) * ( _tcslen( ptszTemp ) + 1 ) );
            SendDlgItemMessage( g_hwndDialog, IDC_FILENAME, WM_SETTEXT, 0, ( WPARAM )g_ptszFileName );
        }

        //
        // Open the file. We may need to convert the filename string from multibytes to wide characters
        //
#ifndef UNICODE
		{
			WCHAR pwszFileName[ MAX_PATH ];

			if( 0 == MultiByteToWideChar( CP_ACP, 0, g_ptszFileName, -1, pwszFileName, MAX_PATH ) )
			{
                //
                // Convertion failed
                //
				SetCurrentStatus( CLOSED );
				SetCurrentStatus( READY );
				break;
			}

			hr = g_pAudioplay->Open( pwszFileName );
		}
#else
		hr = g_pAudioplay->Open( g_ptszFileName );
#endif // UNICODE

		if( FAILED( hr ) )
		{
			SetCurrentStatus( CLOSED );
			SetCurrentStatus( READY );
		}
		else
		{
            //
            // Start to play from the beginning
            //
            hr = g_pAudioplay->Start();
            if( FAILED( hr ) )
            {
                TCHAR tszErrMesg[ 128 ];
                (void)StringCchPrintf( tszErrMesg, ARRAYSIZE(tszErrMesg), _T("Unable to start (hr=%#X)"), hr );
                MessageBox( g_hwndDialog, tszErrMesg, ERROR_DIALOG_TITLE, MB_OK );
            }
            else
            {
                //
                // Set the max range of the slider to be the value of the file's duration in milliseconds
                //
			    SendDlgItemMessage( g_hwndDialog, IDC_SLIDER, TBM_SETRANGEMAX, TRUE,
								    ( DWORD )( g_pAudioplay->GetFileDuration() / 10000 ) );
    			
                //
                // Update the window title with the file's name
                //
			    LPTSTR ptszFile = _tcsrchr( g_ptszFileName, _T( '\\' ) );
                if( NULL != ptszFile )
                {
			        SetWindowText( g_hwndDialog, ptszFile + 1 );
                }
                else
                {
			        SetWindowText( g_hwndDialog, g_ptszFileName );
                }
            }
		}

		break;
	}
	
	return TRUE;
}

//------------------------------------------------------------------------------
// Name: ShowOpenFileDialog()
// Desc: Show the OpenFile dialog box.
//------------------------------------------------------------------------------
BOOL ShowOpenFileDialog()
{
	OPENFILENAME    opfn;

	//
	// Initialize the filename string and an OPENFILENAME structure
	//
	ZeroMemory( g_ptszFileName, sizeof( g_ptszFileName ) );
	ZeroMemory( &opfn, sizeof( OPENFILENAME ) );

	opfn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	opfn.lStructSize = sizeof( opfn );
	opfn.hwndOwner = g_hwndDialog;
	opfn.hInstance = g_hInst;
	opfn.lpstrFilter = _T( "Media Files (*.asf, *.wma, *.wmv)\0*.asf;*.wma;*.wmv\0All files(*.*)\0*.*\0\0\0" );
	opfn.lpstrCustomFilter = NULL;
	opfn.nFilterIndex = 1;
	opfn.lpstrFile = g_ptszFileName;
	opfn.nMaxFile = MAX_PATH;
	opfn.lpstrFileTitle = NULL;
	opfn.lpstrInitialDir = NULL;
	opfn.lpstrTitle = NULL;
	opfn.lpstrDefExt = _T( "*.ASF" );
	
	//
	// Display the open file dialog
	//
	return( GetOpenFileName( &opfn ) );
}

