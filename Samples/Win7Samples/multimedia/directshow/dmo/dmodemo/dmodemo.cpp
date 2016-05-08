// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

//----------------------------------------------------------------------------
// File: DMODemo.cpp
//
// Desc: The dmodemo sample shows how to stream and playback audio data in a
//       wave file of reasonable size using DirectSound Audio Effect DMOs.
//
//       The basic tasks are as follows:
//       1) get the wave file name and DMO name selected by the user
//       2) call CAppStream::Init() to create the Media Object, get
//          interface pointer of IMediaObject and IMediaObjectInPlace,if
//          there is one. Read the wave file into a buffer and read wav format.
//       3) call CAppStream::Stream() to process the data using the Media Object,
//          and retrieve data from the DMO output stream to a buffer in memory.
//       4) create the DirectSound buffer from the buffer which holds the
//          output data from DMO.
//       5) playback the DirectSound buffer.
//-----------------------------------------------------------------------------

#define STRICT

#include <windows.h>
#include <mmsystem.h>

#include <windowsx.h>
#include <TCHAR.h>
#include <objbase.h>
#include <commdlg.h>
#include <dsound.h>
#include "DSUtil.h"
#include "DXUtil.h"
#include <dmoreg.h>
#include <strsafe.h>

#include "appstream.h"
#include "wave.h"
#include "resource.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

//-----------------------------------------------------------------------------
// Function-prototypes
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam );
HRESULT InitializeAudioEffectDMOList( HWND hDlg, int* iNumDmo );
VOID    OnInitDialog( HWND hDlg );
VOID    OnOpenSoundFile( HWND hDlg );
HRESULT StreamData( HWND hDlg, LPTSTR lpszFileInput, REFCLSID rclsid );
VOID    OnTimer( HWND hDlg );
VOID    EnablePlayUI( HWND hDlg, BOOL bEnable );
VOID    ResetSoundBuffer();

//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define MAX_NUM 100
#define APPNAME TEXT("DMO Demo\0")


// struct holding DMOs registered as DMOCATEGORY_AUDIO_EFFECT
typedef struct tagDMOINFO {
    TCHAR szName[MAX_NUM];
    CLSID clsidDMO;
} DMOINFO;

DMOINFO             g_rgDmoInfo[MAX_PATH]={0};
TCHAR               g_szInputFileName[MAX_PATH]={0};
CSoundManager*      g_pSoundManager = NULL;
CSound*             g_pSound = NULL;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point for the application.  Since we use a simple dialog for
//       user interaction we don't need to pump messages.
//-----------------------------------------------------------------------------
INT APIENTRY WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCmdLine,
                      INT nCmdShow )
{
    // Display the main dialog box.
    DialogBox( hInst, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc );

    return TRUE;
}


//-----------------------------------------------------------------------------
// Name: MainDlgProc()
// Desc: Handles dialog messages
//-----------------------------------------------------------------------------
INT_PTR CALLBACK MainDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
    int iSelectedDMOIndex =0;
    HRESULT hr;

    switch( msg )
    {
        case WM_INITDIALOG:
            OnInitDialog( hDlg );
            break;

        case WM_COMMAND:
            switch( LOWORD(wParam) )
            {
                case IDC_SOUNDFILE:
                    OnOpenSoundFile( hDlg );
                    break;

                case IDCANCEL:
                    EndDialog( hDlg, IDCANCEL );
                    break;

                case IDC_PLAY:
                    iSelectedDMOIndex = ComboBox_GetCurSel( GetDlgItem( hDlg, IDC_DMOCOMBO ) );
                    if( iSelectedDMOIndex < 0 )
                    {
                        MessageBox( hDlg, TEXT("Selecting DMO failed."), TEXT(DEMO_NAME),
                                    MB_OK | MB_ICONERROR );
                        break;
                    }

                    // Very large files may take some time to stream, so update the
                    // window title to indicate progress.
                    SetWindowText( hDlg, TEXT("Reading file...") );
                    hr = StreamData( hDlg, g_szInputFileName, g_rgDmoInfo[iSelectedDMOIndex].clsidDMO );
                    if( FAILED( hr ) )
                    {
                        break;
                    }

                    SetWindowText( hDlg, APPNAME );
                    hr = g_pSound->Play( 0,     // lowest priority
                                         0 );   // no flag is set
                    if( FAILED( hr ) )
                    {
                        MessageBox( hDlg, TEXT("Error playing DirectSound buffer."),
                                    TEXT(DEMO_NAME),
                                    MB_OK | MB_ICONERROR );
                    }

                    EnablePlayUI( hDlg, FALSE );
                    break;

                case IDC_STOP:
                    ResetSoundBuffer();
                    EnablePlayUI( hDlg, TRUE );
                    break;

                default:
                    return FALSE; // Didn't handle message
            }
            break;

        case WM_TIMER:
            OnTimer( hDlg );
            break;

        case WM_DESTROY:
            // Cleanup everything
            SAFE_DELETE( g_pSound );
            SAFE_DELETE( g_pSoundManager );
            KillTimer(hDlg, 1);
            break;

        default:
            return FALSE; // Didn't handle message
    }

    return TRUE; // Handled message
}


//-----------------------------------------------------------------------------
// Name: OnInitDialog()
// Desc: Initializes the dialogs (sets up UI controls, etc.)
//-----------------------------------------------------------------------------
VOID OnInitDialog( HWND hDlg )
{
    int iNumDmo = 0;
    HRESULT hr;

    // Load the icon
    HINSTANCE hInst = (HINSTANCE)(LONG_PTR) GetWindowLongPtr( hDlg, GWLP_HINSTANCE );
    HICON hIcon = LoadIcon( hInst, MAKEINTRESOURCE( IDR_DEMOICON ) );

    // Set the icon for this dialog.
    SendMessage( hDlg, WM_SETICON, ICON_BIG,   (LPARAM) hIcon );  // Set big icon
    SendMessage( hDlg, WM_SETICON, ICON_SMALL, (LPARAM) hIcon );  // Set small icon

    // Enumerate registered DMOs and add them to a global structure
    hr = InitializeAudioEffectDMOList( hDlg, &iNumDmo);
    if( FAILED( hr ) ) {

        MessageBox( hDlg, TEXT("Error enumerating DMOs. "), TEXT(DEMO_NAME),
                    MB_OK | MB_ICONERROR );
        return;
    }

    // Add the DMO names to the combo box and select the first item
    for(int i=0; i < iNumDmo; i++)
        ComboBox_AddString(GetDlgItem(hDlg, IDC_DMOCOMBO), g_rgDmoInfo[i].szName);

    ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_DMOCOMBO), 0);

    // CSoundManager creates a CSound buffer, sets cooperative level to DSSCL_PRIORITY,
    // and sets primary buffer format to stereo 22kHz 16-bit output.
    // (See DirectSound documentation for more information),
    g_pSoundManager = new CSoundManager();

    if( g_pSoundManager == NULL )
    {
        MessageBox( hDlg, TEXT("Creating CSoundManager failed."),
                          TEXT(DEMO_NAME), MB_OK | MB_ICONERROR );
        EndDialog( hDlg, IDCANCEL );
    }

    hr = g_pSoundManager->Initialize( hDlg, DSSCL_PRIORITY );
    if( FAILED( hr ) )
    {
        MessageBox( hDlg, TEXT("Error initializing DirectSound."),
                          TEXT(DEMO_NAME), MB_OK | MB_ICONERROR );
        EndDialog( hDlg, IDCANCEL );
    }

    hr = g_pSoundManager->SetPrimaryBufferFormat( 2, 22050, 16 );
    if( FAILED( hr ) )
    {
        MessageBox( hDlg, TEXT("Error initializing DirectSound."),
                          TEXT(DEMO_NAME), MB_OK | MB_ICONERROR );
        EndDialog( hDlg, IDCANCEL );
    }

    // Set the UI controls
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("No file loaded.") );

    // Create a timer, so we can check for when the soundbuffer is stopped
    SetTimer( hDlg, 0, 250, NULL );
}


//-----------------------------------------------------------------------------
// Name: OnOpenSoundFile()
// Desc: Called when the user requests to open a sound file
//-----------------------------------------------------------------------------
VOID OnOpenSoundFile( HWND hDlg )
{
    static TCHAR strFileName[MAX_PATH] = TEXT("");
    static TCHAR strPath[MAX_PATH] = TEXT("");

    OPENFILENAME ofn;

    // Setup the OPENFILENAME structure
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = hDlg;
    ofn.hInstance = NULL;
    ofn.lpstrFilter = TEXT("Wave Files (*.wav)\0*.wav\0All Files\0*.*\0\0");
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex= 1;
    ofn.lpstrFile = strFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = strPath;
    ofn.lpstrTitle = TEXT("Open Sound File\0");
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = TEXT(".wav\0");
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    // Get the default media path (something like C:\WINDOWS\MEDIA)
    if( '\0' == strPath[0] )
    {
        UINT uResult = GetWindowsDirectory( strPath, MAX_PATH );
        if( uResult == 0 )
        {
            MessageBox( hDlg, TEXT("GetWindowsDirectory() failed."),
                              TEXT(DEMO_NAME), MB_OK | MB_ICONERROR );
            return;
        }

        if( lstrcmp( &strPath[lstrlen(strPath)], TEXT("\\") ) )
        {
            StringCchCat( strPath, ARRAY_SIZE(strPath), TEXT("\\\0") );
        }

        StringCchCat( strPath, ARRAY_SIZE(strPath), TEXT("MEDIA\0") );
    }

    ResetSoundBuffer();

    // Update the UI controls to show the sound as loading a file
    EnableWindow(  GetDlgItem( hDlg, IDC_PLAY ), FALSE);
    EnableWindow(  GetDlgItem( hDlg, IDC_STOP ), FALSE);
    SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Loading file...") );

    // Display the OpenFileName dialog. Then, try to load the specified file
    if( TRUE != GetOpenFileName( &ofn ) )
    {
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Load aborted.") );
        return;
    }

    // Update the UI controls to show the sound as the file is loaded
    SetDlgItemText( hDlg, IDC_FILENAME, strFileName );
    EnablePlayUI( hDlg, TRUE );

    // Remember the path for next time
    StringCchCopy(g_szInputFileName, ARRAY_SIZE(g_szInputFileName), strFileName);
    StringCchCopy( strPath, ARRAY_SIZE(strPath), strFileName );

    TCHAR* strLastSlash = _tcsrchr( strPath, '\\' );
    strLastSlash[0] = '\0';
}


// Comparison callback used by qsort()
int CompareStrings( const void *str1, const void *str2 )
{
   // Compare all of both strings
    return _tcsicmp( (TCHAR *) str1, (TCHAR *) str2);
}

//-----------------------------------------------------------------------------
// Name: OutputDMOs()
// Desc: Called to fill the combo box
//-----------------------------------------------------------------------------
HRESULT InitializeAudioEffectDMOList( HWND hDlg, int* iNumDmo)
{
    HRESULT hrNext;
    IEnumDMO* pDMOEnum = NULL;
    WCHAR* wszDMOName=0;
    DWORD ulNumInfoReturned=0;
    CLSID clsidCurrentDMO;

    // Enumerate all the DMOs registered as DMOCATEGORY_AUDIO_EFFECT.
    HRESULT hr = DMOEnum( DMOCATEGORY_AUDIO_EFFECT,
                          DMO_ENUMF_INCLUDE_KEYED,
                          0,        // Number of input partial media types
                          NULL,
                          0,        // Number of output partial media types
                          NULL,
                          &pDMOEnum );

    if( FAILED( hr ) )
        return hr;

    do
    {
        // Get information about the next DMO in the enumeration
        hrNext = pDMOEnum->Next( 1, &clsidCurrentDMO, &wszDMOName, &ulNumInfoReturned );
        if( FAILED( hrNext ) )
        {
            SAFE_RELEASE( pDMOEnum );
            return hrNext;
        }

        if( S_OK == hrNext )
        {
            // Copy this DMO name and CLSID into a global structure
            StringCchCopy(g_rgDmoInfo[*iNumDmo].szName, MAX_NUM, wszDMOName);
            g_rgDmoInfo[*iNumDmo].szName[MAX_NUM-1] = TEXT('\0'); // NULL-terminate

            g_rgDmoInfo[*iNumDmo].clsidDMO = clsidCurrentDMO;

            CoTaskMemFree( wszDMOName );
            (*iNumDmo)++;
        }

    } while( (S_OK == hrNext) && (*iNumDmo < MAX_NUM) );

    // Now that the global DMO list is completed, sort it alphabetically
    qsort(g_rgDmoInfo, *iNumDmo, sizeof(DMOINFO), CompareStrings);

    // Release the DMO enumerator
    SAFE_RELEASE( pDMOEnum );
    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: StreamData()
// Desc: Called when the user requests to Play
//-----------------------------------------------------------------------------
HRESULT StreamData( HWND hDlg, LPTSTR lpszInputFile, REFGUID rclsid )
{
    HRESULT         hr;
    BYTE            *pbOutData=0;
    ULONG           uDataSize =0 ;
    LPWAVEFORMATEX  pwfx = NULL;        // pointer to waveformatex structure.

    hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
    if( FAILED ( hr ) ) {
        MessageBox( hDlg, TEXT("Could not initialize COM library."),
                          TEXT(DEMO_NAME), MB_OK | MB_ICONERROR );
        return hr;
    }

    CAppStream*  pStream = new CAppStream();
    if ( !pStream )
        return E_OUTOFMEMORY;

    hr = pStream->StreamData( lpszInputFile,
                              rclsid,
                              hDlg,
                              &pbOutData,
                              &uDataSize,
                              &pwfx );
    if ( FAILED( hr ) )
       return hr;

    // Free any previous sound, and make a new one
    SAFE_DELETE( g_pSound );

    // Load the data from memory into a DirectSound buffer
    hr = g_pSoundManager->CreateFromMemory( &g_pSound, pbOutData,uDataSize, pwfx, DSBCAPS_GLOBALFOCUS, GUID_NULL );
    if( FAILED( hr ) )
    {
        // Not a critical failure, so just update the status
        MessageBox( hDlg, TEXT("Could not create sound buffer."),
                          TEXT(DEMO_NAME), MB_OK | MB_ICONERROR );
        SetDlgItemText( hDlg, IDC_FILENAME, TEXT("Could not create sound buffer.") );
        return hr;
    }

    SAFE_DELETE_ARRAY( pbOutData );
    SAFE_DELETE ( pStream );
    SafeGlobalFree( pwfx );

    CoUninitialize();
    return hr;
}


//-----------------------------------------------------------------------------
// Name: OnTimer()
// Desc: When we think the sound is playing this periodically checks to see if
//       the sound has stopped.  If it has then updates the dialog.
//-----------------------------------------------------------------------------
VOID OnTimer( HWND hDlg )
{
    if( IsWindowEnabled( GetDlgItem( hDlg, IDC_STOP ) ) )
    {
        // We think the sound is playing, so see if it has stopped yet.
        if( !g_pSound->IsSoundPlaying() )
        {
            // Update the UI controls to show the sound as stopped
            EnablePlayUI( hDlg, TRUE );
        }
    }
}


//-----------------------------------------------------------------------------
// Name: EnablePlayUI( hDlg,)
// Desc: Enables or disables the Play UI controls
//-----------------------------------------------------------------------------
VOID EnablePlayUI( HWND hDlg, BOOL bEnable )
{
    if( bEnable )
    {
        EnableWindow( GetDlgItem( hDlg, IDC_STOP ),  FALSE);
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY ),  TRUE);
        SetFocus(     GetDlgItem( hDlg, IDC_PLAY ));
    }
    else
    {
        EnableWindow( GetDlgItem( hDlg, IDC_STOP ),  TRUE);
        SetFocus(     GetDlgItem( hDlg, IDC_STOP ));
        EnableWindow( GetDlgItem( hDlg, IDC_PLAY ),  FALSE);
    }
}


//-----------------------------------------------------------------------------
// Name: ResetSoundBuffer()
// Desc: called when user click stop button or open file button
//-----------------------------------------------------------------------------
VOID ResetSoundBuffer()
{
    if( g_pSound )
    {
        g_pSound->Stop();
        g_pSound->Reset();
    }
}


