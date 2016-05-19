//////////////////////////////////////////////////////////////////////////
//
// Winmain.cpp : Defines the entry point for the application.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////
//
//
//This sample demostrates the use of the following ASF objects provided by
//Media Foundation, for reading an ASF media file:
//- ASF Splitter 
//- ASF ContentInfo
//- ASF Indexer
//
//All of these objects are controlled by the ASF Manager, implemented in the
//CASFManager class. 
//
//The user opens an ASF media file (*.wma or *.wmv) by using a standard Open
//dialog box. The global file attributes is shown on the Information pane.
//The ASF Manager enumerates the streams contained in the media file  
//and displays only the first audio stream and the video stream to the user. 
//
//The user can: then 
//- Select a stream for parsing. By default, the first stream is selected by the ASF Manager.
//- Specify the offset from where to start parsing by seeking to a point in the presentation.
//- Check the Reverse check box to generate samples in reverse.
//
//The ASF Manager generates samples that passed through a decoder for decompression. 
//As the samples are generated, sample attributes are shown on the Information pane.
//
//To test the samples, the user can use to Media Controller, implemented in the
//CMediaController class. For an audio stream, the Media Controller plays samples for
//5 seconds. For a video stream, it shows the first key frame as a bitmap.
// 
//////////////////////////////////////////////////////////////////////////

//local project files

#include "MF_ASFParser.h"
#include "resource.h"
#include "ASFManager.h"

// Global Variables:

HINSTANCE   g_hInst = NULL;         //Global application instance.
HWND        g_hWnd = NULL;          //Global handle to the main Window.

CASFManager*        g_pASFManager = NULL;       //Pointer to the ASF Manager object.
CMediaController*   g_pMediaController = NULL;  //Pointer to the Media Controller object.

GUID g_guidMediaType = GUID_NULL;   //Major media type GUID of the currently selected stream.

FILE_PROPERTIES_OBJECT  g_fileinfo; //FILE_PROPERTIES_OBJECT variable that stores global ASF file attributes.

MFTIME  g_seektime =0;          //Seek time in 100 nano-seconds
BOOL    g_fIsReverse = FALSE;   //Reverse flag.


//////////////////////////////////////////////////////////////////////////
//  Name: FormatTimeString
//  Description: Converts MFTIME format to mm:ss format.
//
//  time: MFTIME type 100 nano second unit.
//  szTimeString: NULL-terminated string containing the time string.
///////////////////////////////////////////////////////////////////////////

void FormatTimeString(MFTIME time, WCHAR *szTimeString, DWORD cbSize, HRESULT hr)
{
    //Convert nanoseconds to seconds
    MFTIME TimeInSeconds = 0, Hours = 0, Minutes = 0, Seconds = 0;

    TimeInSeconds = (MFTIME)time/10000000;

    if (TimeInSeconds > 60)
    {
        Minutes = TimeInSeconds/60;

        if( Minutes > 60 )
        {
            Hours = Minutes/60;
            Minutes = Minutes % 60;
        }
        Seconds = (TimeInSeconds % 60);
    }
    else
    {
        Seconds = TimeInSeconds;
    }

    hr = StringCchPrintf (szTimeString, cbSize, L"%02d:%02d:%02d", (int) Hours, (int) Minutes, (int)Seconds);    
}

//////////////////////////////////////////////////////////////////////////
//  Name: NotifyError
//  Description: Show a message box with an error message.
//
//  sErrorMessage: NULL-terminated string containing the error message.
//  hrErr: HRESULT from the error.
/////////////////////////////////////////////////////////////////////////

void NotifyError(const WCHAR *szErrorMessage, HRESULT hrErr, HWND hWnd)
{
    WCHAR szMessage[MAX_STRING_SIZE];

    HRESULT hr = StringCchPrintf (szMessage, MAX_STRING_SIZE, L"%s (HRESULT = 0x%X)", szErrorMessage, hrErr);

    if (SUCCEEDED(hr))
    {
        MessageBox(hWnd, szMessage, NULL, MB_OK | MB_ICONERROR);
    }

}

//////////////////////////////////////////////////////////////////////////
//  Name: DisplaySampleInfo
//  Description: Displays sample attributes on the Information pane.
//  Parameter: Handle to the main Window.

/////////////////////////////////////////////////////////////////////////
void DisplaySampleInfo(SAMPLE_INFO *sampleinfo)
{
    HRESULT hr = S_OK;

    WCHAR szMessage [MAX_STRING_SIZE], szTemp [MAX_STRING_SIZE];
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"");

    SendMessage(GetDlgItem(g_hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    if (sampleinfo->fSeekedKeyFrame == TRUE)
    {
        StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Seeked Key Frame");
        SendMessage(GetDlgItem(g_hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);
    }

    //Stream number
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Stream number: %d", sampleinfo->wStreamNumber);
    SendMessage(GetDlgItem(g_hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Total buffers in the sample
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Buffer count: %d", sampleinfo->cBufferCount);
    SendMessage(GetDlgItem(g_hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Total sample size in bytes
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Total length: %d", sampleinfo->cbTotalLength);
    SendMessage(GetDlgItem(g_hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Current sample time
    FormatTimeString(sampleinfo->hnsSampleTime, szMessage, MAX_STRING_SIZE, hr);
    StringCchPrintf(szTemp, MAX_STRING_SIZE, L"Sample time: %s (%d hns)", szMessage, sampleinfo->hnsSampleTime);
    SendMessage(GetDlgItem(g_hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szTemp);

    ZeroMemory((void*)sampleinfo, sizeof(SAMPLE_INFO));
}

//////////////////////////////////////////////////////////////////////////
//  Name: DisplayFilePropertiesObject
//  Description: Displays File Properties Object Header about the currently open ASF file.
//  
//  Parameter: Handle to the main Window.
/////////////////////////////////////////////////////////////////////////

void DisplayFilePropertiesObject(const FILE_PROPERTIES_OBJECT& fileinfo, HWND hWnd)
{
    SYSTEMTIME systemtime;

    HRESULT hr = S_OK;

    WCHAR szMessage [MAX_STRING_SIZE], szTemp [MAX_STRING_SIZE];
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"");

    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_RESETCONTENT, 0, 0);

    SetDlgItemText(hWnd, IDC_STATIC_INFO, L"Global File Attributes");
    
    //File ID
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"File ID: %08lX %04X %04X", fileinfo.guidFileID);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Creation Time
    FileTimeToSystemTime(&fileinfo.ftCreationTime, &systemtime);
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Creation Time: %d-%d-%d %02d:%02d:%02d", systemtime.wYear,  systemtime.wMonth, systemtime.wDay, systemtime.wHour, systemtime.wMinute, systemtime.wSecond);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Data Packet Count
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Packets: %d ", fileinfo.cbPackets);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Play Duration
    FormatTimeString(fileinfo.cbPlayDuration, szMessage, MAX_STRING_SIZE, hr);
    StringCchPrintf(szTemp, MAX_STRING_SIZE, L"Play duration: %s (%I64d hns)", szMessage, fileinfo.cbPlayDuration);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szTemp);

    //Presenation Duration
    FormatTimeString(fileinfo.cbPresentationDuration, szMessage, MAX_STRING_SIZE, hr);
    StringCchPrintf(szTemp, MAX_STRING_SIZE, L"Presentation duration: %s (%I64d hns)", szMessage, fileinfo.cbPresentationDuration);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szTemp);

    //Send Duration
    FormatTimeString(fileinfo.cbSendDuration, szMessage, MAX_STRING_SIZE, hr);
    StringCchPrintf(szTemp, MAX_STRING_SIZE, L"Send duration: %s (%I64d hns)", szMessage, fileinfo.cbSendDuration);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szTemp);

    //Preroll
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Preroll: %I64d hns", fileinfo.cbPreroll);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Flags
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Flags: %d", fileinfo.cbFlags);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Minimum Data Packet Size
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Minimum packet size: %d bytes", fileinfo.cbMinPacketSize);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Maximum Data Packet Size
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Maximum packet size: %d bytes", fileinfo.cbMaxPacketSize);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    //Maximum Bit rate
    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"Maximum bit rate: %d bits/sec", fileinfo.cbMaxBitRate);
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);

    StringCchPrintf(szMessage, MAX_STRING_SIZE, L"");
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_ADDSTRING, 0, (LPARAM)szMessage);
}

//////////////////////////////////////////////////////////////////////////
//  Name: ResetUI
//  Description: Reset controls to original state because there is not ASF file
//               that is currently open. 
//  Parameter: Handle to the main Window.
/////////////////////////////////////////////////////////////////////////

void ResetUI(HWND hWnd)
{
    //Hide Show File Properties button
    ShowWindow(GetDlgItem(hWnd, IDC_FILE_PROP), SW_HIDE); 
        
    //Hide Information pane
    ShowWindow(GetDlgItem(hWnd, IDC_INFO), SW_HIDE);
    
    //Hide Splitter configuration controls
    ShowWindow(GetDlgItem(hWnd, IDC_STATIC_PARSER_CONFIG), SW_HIDE); 
    ShowWindow(GetDlgItem(hWnd, IDC_STATIC_STREAM), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_STATIC_STREAM2), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_STATIC_STREAM1), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_STREAM1), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_STREAM2), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_REVERSE), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_STATIC_SEEK), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_SEEK), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_TIME), SW_HIDE);
    SendMessage(GetDlgItem(hWnd, IDC_STREAM1), BM_SETCHECK , BST_UNCHECKED, 0);
    SendMessage(GetDlgItem(hWnd, IDC_STREAM2), BM_SETCHECK , BST_UNCHECKED, 0);
    
    //Hide the Generate Samples button
    ShowWindow(GetDlgItem(hWnd, IDC_PARSE), SW_HIDE);

    //Hide the test controls
    ShowWindow(GetDlgItem(hWnd, IDC_PLAY_AUDIO), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_SHOW_BITMAP), SW_HIDE);
}

//////////////////////////////////////////////////////////////////////////
//  Name: StreamSelect
//  Description: Selects audio or video stream for parsing.
//  Parameter: 
//      wStreamNumber: Stream number of the stream to select. 
//      hWnd: Handle to the main Window.
//
/////////////////////////////////////////////////////////////////////////

void StreamSelect(WORD wStreamNumber, HWND hWnd)
{
    HRESULT hr = S_OK;

    //New stream selection so hide the test controls for audio and video.
    ShowWindow(GetDlgItem(hWnd, IDC_PLAY_AUDIO), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_SHOW_BITMAP), SW_HIDE);

    //Select the specified stream for parsing
    hr = g_pASFManager->SelectStream(wStreamNumber, &g_guidMediaType);

    //If the media type is invalid, the stream will be parsed but generated samples cannot be tested
    if (hr == MF_E_INVALIDMEDIATYPE)
    {
        ShowWindow(GetDlgItem(hWnd, IDC_PARSE), SW_SHOW);
        NotifyError(L"Media type is not supported by the stream decoder. \nNo test samples will be available", hr, hWnd);
        return;
    }

    //Stream selected, show the GenerateSamples button
    if (hr == S_OK)
    {
        ShowWindow(GetDlgItem(hWnd, IDC_PARSE), SW_SHOW);
        return;
    }
    else
    {
        NotifyError(L"Could not select stream", hr, hWnd);
    }
    
    return;

}

//////////////////////////////////////////////////////////////////////////
//  Name: GetAvailableStreams
//  Description: Enumerates available streams in the ASF file and selects the
//               first stream.
//  Parameter: Handle to the main Window.
/////////////////////////////////////////////////////////////////////////

void GetAvailableStreams(HWND hWnd)
{
    HRESULT hr = S_OK;  

    WORD *pwStreamNumbers = NULL;
    GUID *pGUIDs = NULL;

    DWORD cbTotalStreams = 0;

    WCHAR szLabel[MAX_STRING_SIZE];
    StringCchPrintf(szLabel, MAX_STRING_SIZE, L"");

    //Get the stream numbers and the major media type GUIDs for the streams
    hr = g_pASFManager->EnumerateStreams(&pwStreamNumbers, &pGUIDs, &cbTotalStreams);
    
    if(SUCCEEDED(hr) && pwStreamNumbers)
    {

        for ( DWORD index = 0; index < cbTotalStreams; index++)
        {
            if(pGUIDs[index] == MFMediaType_Audio)
            {
                //For audio stream make UI adjustments:
                
                //Show the stream number
                StringCchPrintf(szLabel, MAX_STRING_SIZE, L"%d", pwStreamNumbers[index]);
                SetDlgItemText(hWnd, IDC_STATIC_STREAM1, szLabel);
                ShowWindow(GetDlgItem(hWnd, IDC_STATIC_STREAM1), SW_SHOW);
                
                //Designate the radio button 1 as Audio
                SetDlgItemText(hWnd, IDC_STREAM1, L"- Audio");
                ShowWindow(GetDlgItem(hWnd, IDC_STREAM1), SW_SHOW);
            }

            if(pGUIDs[index] == MFMediaType_Video)
            {
                //For video stream make UI adjustments:
                
                //Show the stream number
                StringCchPrintf(szLabel, MAX_STRING_SIZE, L"%d", pwStreamNumbers[index]);
                SetDlgItemText(hWnd, IDC_STATIC_STREAM2, szLabel);
                SetDlgItemText(hWnd, IDC_STREAM2, L"- Video");
                
                //Designate the radio button 1 as Video
                ShowWindow(GetDlgItem(hWnd, IDC_STATIC_STREAM2), SW_SHOW);
                ShowWindow(GetDlgItem(hWnd, IDC_STREAM2), SW_SHOW);
            }
            
        }
        
        //By default select the first stream and based on the GUID show the corresponding
        //stream radio button as checked.
        StreamSelect(pwStreamNumbers[0], hWnd);

        if(pGUIDs[0] == MFMediaType_Audio)
        {
            SendMessage(GetDlgItem(hWnd, IDC_STREAM1), BM_SETCHECK , BST_CHECKED, 0);
        }
        if(pGUIDs[0] == MFMediaType_Video)
        {
            SendMessage(GetDlgItem(hWnd, IDC_STREAM2), BM_SETCHECK , BST_CHECKED, 0);
        }

        //Show the related UI controls
        ShowWindow(GetDlgItem(hWnd, IDC_STATIC_STREAM), SW_SHOW);
        ShowWindow(GetDlgItem(hWnd, IDC_STATIC_PARSER_CONFIG), SW_SHOW);

    }
    else
    {
        NotifyError(L"No streams found.", hr, hWnd);
    }

    SAFE_ARRAY_DELETE( pwStreamNumbers);
    SAFE_ARRAY_DELETE( pGUIDs);
}



//////////////////////////////////////////////////////////////////////////
//  Name: SetSeekBar
//  Description: Setup seek bar range. 
//
/////////////////////////////////////////////////////////////////////////

void SetSeekBar(HWND hWnd)
{
    WCHAR szMessage [MAX_STRING_SIZE];

    HRESULT hr = S_OK;

    //Setup the track bar for seeking
    SendMessage(GetDlgItem(hWnd, IDC_SEEK), TBM_SETRANGEMIN, TRUE, 0);
    SendMessage(GetDlgItem(hWnd, IDC_SEEK), TBM_SETRANGEMAX, TRUE, (LPARAM)g_fileinfo.cbPresentationDuration/10000000);
    ShowWindow(GetDlgItem(hWnd, IDC_SEEK), SW_SHOW);
    
    //Show play duration
    FormatTimeString(g_fileinfo.cbPresentationDuration, szMessage, MAX_STRING_SIZE, hr);
    SetDlgItemText(hWnd, IDC_TIME, szMessage);
    ShowWindow(GetDlgItem(hWnd, IDC_TIME), SW_SHOW);
}

////////////////////////////////////////////////////////////////////////////////////////
//  Name: OnOpen
//  Description: Opens the standard open file dialog and allows the user to a wma or wmv file.
//      
//  Parameter: Handle to the main Window.
/////////////////////////////////////////////////////////////////////////////////////////

void OnOpen(HWND hWnd)
{
    HRESULT hr = S_OK;
    DWORD extendedError = 0;
    const size_t bufferSize = 2000;

    ZeroMemory((void*)&g_fileinfo, sizeof(FILE_PROPERTIES_OBJECT));

    //Reset the UI and show the controls later depending on the file opened and stream selection.
    ResetUI(hWnd);
    
    if (g_pASFManager)
    {
        // Show the File Open dialog.
        WCHAR path[bufferSize];
        path[0] = L'\0';

        OPENFILENAME ofn;
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFilter = L"Windows Media \0*.wma;*.wmv\0ASF\0*.asf\0All files\0*.*\0";
        ofn.lpstrFile = path;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_EXPLORER | OFN_ENABLESIZING | OFN_FILEMUSTEXIST;

        if (GetOpenFileName(&ofn))
        {
            CHECK_HR (hr = g_pASFManager->OpenASFFile(path)); 

            CHECK_HR (hr = g_pASFManager->SetFilePropertiesObject(&g_fileinfo));

            //Get File Properties Header Object that stores the global file attributes
            DisplayFilePropertiesObject(g_fileinfo, hWnd);
            
            //Show the Show File Properties button that will display the global file attributes
            ShowWindow(GetDlgItem(hWnd, IDC_FILE_PROP), SW_SHOW);
            
            //Show the Information pane
            ShowWindow(GetDlgItem(hWnd, IDC_INFO), SW_SHOW);

            //Enumerate the streams in the file
            GetAvailableStreams(hWnd);

            //Setup seek bar that the user can use to set the offset
            SetSeekBar(hWnd);

            //If the file is reversible, show the Reverse check box 
            if (g_fileinfo.cbMaxPacketSize == g_fileinfo.cbMinPacketSize)
            {
                ShowWindow(GetDlgItem(hWnd, IDC_REVERSE), SW_SHOW);
            }

            return;
        }
    }

done:

    if (FAILED(hr))
    {
        NotifyError(L"Cannot open the ASF file.", hr, hWnd);
    }

    return;

}


//////////////////////////////////////////////////////////////////////////
//  Name: OnParse
//  Description: Generates uncompressed samples for the selected stream.
//  Parameter: Handle to the main Window.
/////////////////////////////////////////////////////////////////////////

void OnParse(HWND hWnd)
{
    assert ( g_pASFManager!=NULL);

    HRESULT hr = S_OK;

    DWORD dwFlags = 0;

    SAMPLE_INFO sampleinfo;
    ZeroMemory(&sampleinfo, sizeof(SAMPLE_INFO));

    g_hWnd = hWnd;

    //Reset UI
    //Hide the test controls
    ShowWindow(GetDlgItem(hWnd, IDC_PLAY_AUDIO), SW_HIDE);
    ShowWindow(GetDlgItem(hWnd, IDC_SHOW_BITMAP), SW_HIDE);
    SetDlgItemText(hWnd, IDC_STATIC_INFO, L"Sample Attributes");
    SendMessage(GetDlgItem(hWnd, IDC_INFO), LB_RESETCONTENT, 0, 0);

    //Get the Media Controller that will control test data 
    if (!g_pMediaController)
    {
        hr = g_pASFManager->GetMediaController(&g_pMediaController);
    }
    
    //Clear exisiting test content from the Media Controller
    if (SUCCEEDED (hr))
    {
        hr = g_pMediaController->Reset();
    }
    if (FAILED (hr))
    {
        NotifyError(L"Media controller could not be found.", hr, hWnd);
    }

    //Check if the user requested reverse parsing
    if (g_fIsReverse)
    {
        dwFlags |= MFASF_SPLITTER_REVERSE;
    }

    //Get the data offset based on seektime
    hr = g_pASFManager->GenerateSamples(g_seektime, dwFlags, &sampleinfo, &DisplaySampleInfo);


    if (SUCCEEDED(hr))
    {
        //If the Media Controller collected any test content
        if (g_pMediaController && g_pMediaController->HasTestMedia())
        {
            //For the audio stream, show the Test Audio button
            if (g_guidMediaType == MFMediaType_Audio)
            {
                ShowWindow(GetDlgItem(hWnd, IDC_PLAY_AUDIO), SW_SHOW);
            }

            //For the video stream, show the Show Bitmap button
            if (g_guidMediaType == MFMediaType_Video)
            {           
                ShowWindow(GetDlgItem(hWnd, IDC_SHOW_BITMAP), SW_SHOW);
            }
        }
    }
    else
    {
        NotifyError(L"Samples could not be generated.", hr, hWnd);
    }

}

//////////////////////////////////////////////////////////////////////////
//  Name: OnPlayTestAudio
//  Description: Play the test audio clip.
//  Parameter: Handle to the main Window.
/////////////////////////////////////////////////////////////////////////

void OnPlayTestAudio(HWND hWnd)
{
    HRESULT hr = S_OK;

    //Check if the Media Controller has test content
    if (g_pMediaController && g_pMediaController->HasTestMedia())
    {
        hr = g_pMediaController->PlayAudio();
    }
    else
    {
        NotifyError(L"Test clip not found.", E_FAIL, hWnd);
    }

    if (FAILED (hr))
    {
        NotifyError(L"Could not play test audio clip.", hr, hWnd);
    }
}

//////////////////////////////////////////////////////////////////////////
//  Name: ConvertLabelToStreamNumber
//  Description: Converts the control caption to a valid stream number and then 
//               selects the stream.
//
//  lpControlID: Control ID
///////////////////////////////////////////////////////////////////////////

void ConvertLabelToStreamNumber( HWND hWnd, int ControlID)
{
    WCHAR szLabel [3];

    GetDlgItemText(hWnd, ControlID, (LPWSTR)szLabel, sizeof(szLabel));

    WCHAR *szStop = (szLabel+1);

    int l = wcstol(szLabel, &szStop, 10);
    
    WORD wStreamNumber = (WORD)l;

    StreamSelect(wStreamNumber, hWnd);
}

//////////////////////////////////////////////////////////////////////////
//  Name: SetReverse
//  Description: Setup the reverse flag and the seektime when reverse is enabled. 
//
/////////////////////////////////////////////////////////////////////////

void SetReverse(HWND hWnd)
{
    //Check if the Reverse check box is enabled.
    if (SendMessage(GetDlgItem(hWnd, IDC_REVERSE), BM_GETCHECK , 0, 0)==BST_CHECKED)
    {
        LRESULT nMax = SendMessage(GetDlgItem(hWnd, IDC_SEEK), TBM_GETRANGEMAX , 0, 0);
        SendMessage(GetDlgItem(hWnd, IDC_SEEK), TBM_SETPOS, TRUE, nMax);

        //Setup seek time
        g_seektime = nMax*10000000;

        g_fIsReverse = TRUE; 
    }
    else
    {
        LRESULT nMin = SendMessage(GetDlgItem(hWnd, IDC_SEEK), TBM_GETRANGEMIN , 0, 0);
        SendMessage(GetDlgItem(hWnd, IDC_SEEK), TBM_SETPOS, TRUE, nMin);

        g_seektime = nMin*1;

        g_fIsReverse = FALSE; 
    }
}



//////////////////////////////////////////////////////////////////////////
//  Name: UIBitmap
//  Description: Dialogbox callback for the window that shows the bitmap
//               for the key frame. 
/////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK UIBitmap(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    switch (message)
    {
    case WM_INITDIALOG :
        {
            RECT rcClient = { 0, 0, 0, 0 };

            g_pMediaController->GetBitmapDimensions((UINT32*)&rcClient.right, (UINT32*)&rcClient.bottom);

            AdjustWindowRectEx(&rcClient, GetWindowStyle(hDlg), FALSE, GetWindowExStyle(hDlg));

            SetWindowPos(
                hDlg, 
                HWND_TOP, 
                0, 0, 
                // New window size:
                (rcClient.right - rcClient.left), (rcClient.bottom - rcClient.top),
                SWP_NOMOVE
                );

        }
        return TRUE;

    case WM_PAINT:
        hr = g_pMediaController->DrawKeyFrame(hDlg);
        if (FAILED(hr))
        {
            NotifyError(L"Could not display bitmap.", hr, hDlg);
            EndDialog(hDlg, 0);
        }
        break;

    case WM_ERASEBKGND:
        return TRUE;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;

    default:
        return FALSE; // Dialog manager will perform the default action.
    }
    return (INT_PTR)TRUE;
}

//////////////////////////////////////////////////////////////////////////
//  Name: OnShowBitmap
//  Description: Creates a Window and shows the key frame in another window.
//
/////////////////////////////////////////////////////////////////////////

void OnShowBitmap(void)
{
    DialogBox(g_hInst, MAKEINTRESOURCE(IDD_BITMAP), g_hWnd, UIBitmap); 
}


//////////////////////////////////////////////////////////////////////////
//  Name: UIMain
//  Description: Dialogbox callback for the main Window.
/////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK UIMain(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    HRESULT hr = S_OK;

    int seekposition =0;

    switch (message)
    {
    case WM_INITDIALOG:
        ResetUI(hDlg);
        return TRUE;

    case WM_COMMAND:
        switch  ( wParam )
        {
        case IDC_OPENFILE:
            OnOpen(hDlg);
            break;

        case IDC_FILE_PROP:
            DisplayFilePropertiesObject(g_fileinfo, hDlg);
            break;

        case IDC_PARSE:
            OnParse(hDlg);
            break;

        case IDC_SHOW_BITMAP:
            OnShowBitmap();
            break;

        case IDC_PLAY_AUDIO:
            OnPlayTestAudio(hDlg);
            break;

        default:
            break;
        }

        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch (LOWORD(wParam)) 
            { 
            case IDC_STREAM1:
                ConvertLabelToStreamNumber(hDlg, IDC_STATIC_STREAM1);
                break;

            case IDC_STREAM2:
                ConvertLabelToStreamNumber(hDlg, IDC_STATIC_STREAM2);
                break;

            case IDC_REVERSE:
                SetReverse(hDlg);   
                break;

            }  // switch (inner)
        } // if
        // wm_command
        break;

    case WM_HSCROLL:
        switch ( LOWORD(wParam) )
        {
        case TB_ENDTRACK:
            DWORD dwPos;
            dwPos = (DWORD)SendMessage(GetDlgItem(hDlg, IDC_SEEK), TBM_GETPOS, 0, 0); 
            //Get current slider position and convert value into seconds.
            g_seektime = dwPos*10000000;

            

            break;
        default: 
            break; 
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, 0);
        break;

    default:
        return FALSE; // Dialog manager will perform the default action.
    }
    return (INT_PTR)TRUE;
}


//////////////////////////////////////////////////////////////////////////
//  Name: WinMain
//  Description: Application entry point.
/////////////////////////////////////////////////////////////////////////

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE /* hPrevInstance */,
                       LPTSTR    /* lpCmdLine */,
                       int       nCmdShow)

{

    TRACE_INIT();

    g_hInst = hInstance;

    HRESULT hr = S_OK;
    
    //Initialize CASFManager object as a global instance
    CHECK_HR( hr = CASFManager::CreateInstance(&g_pASFManager));

    DialogBox( hInstance, (LPCTSTR)IDD_MAIN, NULL, UIMain );

    TRACE((L"Released resources. Closing application...\n"));


done:
    if (FAILED(hr))
    {
        NotifyError(L"ASF manager could not be created", hr, g_hWnd);
    }

    SAFE_RELEASE (g_pASFManager); //This also releases the Media Controller.
    
    TRACE_CLOSE();

    return 0;
}
