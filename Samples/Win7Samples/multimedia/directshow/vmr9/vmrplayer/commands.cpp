//------------------------------------------------------------------------------
// File: commands.cpp
//
// Desc: DirectShow sample code
//       - Processes commands from the user.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------------------------

#include "project.h"


// Global data
extern CMovie *pMovie;

// Prototypes
extern void RepositionMovie(HWND hwnd);

typedef LPBITMAPINFOHEADER PDIB;

// Constants
#define BFT_BITMAP 0x4d42   /* 'BM' */

// Macros
#define DibNumColors(lpbi)      ((lpbi)->biClrUsed == 0 && (lpbi)->biBitCount <= 8 \
                                    ? (int)(1 << (int)(lpbi)->biBitCount)          \
                                    : (int)(lpbi)->biClrUsed)

#define DibSize(lpbi)           ((lpbi)->biSize + (lpbi)->biSizeImage + (int)(lpbi)->biClrUsed * sizeof(RGBQUAD))

#define DibPaletteSize(lpbi)    (DibNumColors(lpbi) * sizeof(RGBQUAD))


/******************************Public*Routine******************************\
* VcdPlyerCaptureImage
*
\**************************************************************************/
BOOL VcdPlyerCaptureImage(LPCTSTR szFile)
{
    HRESULT hr;

    if(pMovie)
    {
        BYTE* lpCurrImage = NULL;

        // Read the current video frame into a byte buffer.  The information
        // will be returned in a packed Windows DIB and will be allocated
        // by the VMR.
        if(SUCCEEDED(hr = pMovie->GetCurrentImage(&lpCurrImage)))
        {
            BITMAPFILEHEADER    hdr;
            DWORD               dwSize, dwWritten;
            LPBITMAPINFOHEADER  pdib = (LPBITMAPINFOHEADER) lpCurrImage;

            // Create a new file to store the bitmap data
            HANDLE hFile = CreateFile(szFile, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

            if (hFile == INVALID_HANDLE_VALUE)
                return FALSE;

            // Initialize the bitmap header
            dwSize = DibSize(pdib);
            hdr.bfType          = BFT_BITMAP;
            hdr.bfSize          = dwSize + sizeof(BITMAPFILEHEADER);
            hdr.bfReserved1     = 0;
            hdr.bfReserved2     = 0;
            hdr.bfOffBits       = (DWORD)sizeof(BITMAPFILEHEADER) + pdib->biSize +
                DibPaletteSize(pdib);

            // Write the bitmap header and bitmap bits to the file
            WriteFile(hFile, (LPCVOID) &hdr, sizeof(BITMAPFILEHEADER), &dwWritten, 0);
            WriteFile(hFile, (LPCVOID) pdib, dwSize, &dwWritten, 0);

            // Close the file
            CloseHandle(hFile);

            // The app must free the image data returned from GetCurrentImage()
            CoTaskMemFree(lpCurrImage);

            // Give user feedback that the write has completed
            TCHAR szText[128], szDir[MAX_PATH];
            GetCurrentDirectory(MAX_PATH, szDir);

            // Strip off the trailing slash, if it exists
            int nLength = (int) wcslen(szDir);
            if (szDir[nLength-1] == TEXT('\\'))
                szDir[nLength-1] = TEXT('\0');

            (void)StringCchPrintf(szText, NUMELMS(szText), TEXT("Captured current image to %s\\%s.\0"), szDir, szFile);
            MessageBox(hwndApp, szText, TEXT("Captured bitmap"), MB_OK);
            return TRUE;
        }
        else
        {
            MessageBox(hwndApp, TEXT("Failed to capture image!"), TEXT("VMRPlayer9"), MB_OK);
            return FALSE;
        }

    }

    return FALSE;
}


/******************************Public*Routine******************************\
* VcdPlyerDisplayCapturedImage
*
\**************************************************************************/
BOOL
VcdPlyerDisplayCapturedImage(
    LPCTSTR szFile
    )
{
    // Open the bitmap with the system-default application
    ShellExecute(hwndApp, TEXT("open\0"), szFile, NULL, NULL, SW_SHOWNORMAL);

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerOpenCmd
*
\**************************************************************************/
BOOL
VcdPlayerOpenCmd(
    int strmID
    )
{
    static BOOL fFirstTime = TRUE;
    BOOL fRet;
    TCHAR achFileName[MAX_PATH];
    TCHAR achFilter[MAX_PATH];
    LPTSTR lp;

    if(fFirstTime)
    {
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hwndApp;
        ofn.Flags = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST |
                    OFN_SHAREAWARE | OFN_PATHMUSTEXIST;
    }

    (void)StringCchCopy(achFilter, NUMELMS(achFilter), IdStr(STR_FILE_FILTER));
    ofn.lpstrFilter = achFilter;

    /*
    ** Convert the resource string into to something suitable for
    ** GetOpenFileName ie.  replace '#' characters with '\0' characters.
    */
    for(lp = achFilter; *lp; lp++)
    {
        if(*lp == TEXT('#'))
        {
            *lp = TEXT('\0');
        }
    }

    ofn.lpstrFile = achFileName;
    ofn.nMaxFile = sizeof(achFileName) / sizeof(TCHAR);
    ZeroMemory(achFileName, sizeof(achFileName));

    fRet = GetOpenFileName(&ofn);
    if(fRet)
    {
        if(strmID == 0)
        {
            fFirstTime = FALSE;
            ProcessOpen(achFileName);
        }
        else
        {
            if(pMovie)
            {
                pMovie->RenderSecondFile(achFileName);
            }
        }

        InitStreamParams(strmID);
    }

    return fRet;
}


/******************************Public*Routine******************************\
* VcdPlayerCloseCmd
*
\**************************************************************************/
BOOL
VcdPlayerCloseCmd(
    void
    )
{
    if(pMovie)
    {
        LONG cx, cy;

        g_State = VCD_NO_CD;
        pMovie->GetMoviePosition(&lMovieOrgX, &lMovieOrgY, &cx, &cy);
        pMovie->StopMovie();
        pMovie->CloseMovie();

        SetDurationLength((REFTIME)0);
        SetCurrentPosition((REFTIME)0);

        delete pMovie;
        pMovie = NULL;
    }

    g_bSecondFileLoaded = FALSE;
    InvalidateRect(hwndApp, NULL, FALSE);
    UpdateWindow(hwndApp);

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerPlayCmd
*
\**************************************************************************/
BOOL
VcdPlayerPlayCmd(
    void
    )
{
    BOOL fStopped = (g_State & VCD_STOPPED);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if((fStopped || fPaused))
    {
        if(pMovie)
        {
            pMovie->PlayMovie();
        }

        g_State &= ~(fStopped ? VCD_STOPPED : VCD_PAUSED);
        g_State |= VCD_PLAYING;
    }

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerPlayCmd
*
\**************************************************************************/
BOOL
VcdPlayerStopCmd(
    void
    )
{
    BOOL fPlaying = (g_State & VCD_PLAYING);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if((fPlaying || fPaused))
    {
        if(pMovie)
        {
            pMovie->StopMovie();
            SetCurrentPosition(pMovie->GetCurrentPosition());
        }

        g_State &= ~(fPlaying ? VCD_PLAYING : VCD_PAUSED);
        g_State |= VCD_STOPPED;
    }

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerStepCmd
*
\**************************************************************************/
BOOL
VcdPlayerStepCmd(
    void
    )
{
    if(pMovie)
    {
        // Ensure that the video is paused to update toolbar buttons
        if(g_State & VCD_PLAYING)
            VcdPlayerPauseCmd();

        if(pMovie->FrameStepMovie())
        {
            g_State |= VCD_STEPPING;
            return TRUE;
        }
    }
    return FALSE;
}


/******************************Public*Routine******************************\
* VcdPlayerPauseCmd
*
\**************************************************************************/
BOOL
VcdPlayerPauseCmd(
    void
    )
{
    BOOL fPlaying = (g_State & VCD_PLAYING);
    BOOL fPaused  = (g_State & VCD_PAUSED);

    if(fPlaying)
    {
        if(pMovie)
        {
            pMovie->PauseMovie();
            SetCurrentPosition(pMovie->GetCurrentPosition());
        }

        g_State &= ~VCD_PLAYING;
        g_State |= VCD_PAUSED;
    }
    else if(fPaused)
    {
        if(pMovie)
        {
            pMovie->PlayMovie();
        }

        g_State &= ~VCD_PAUSED;
        g_State |= VCD_PLAYING;
    }

    return TRUE;
}


/******************************Public*Routine******************************\
* VcdPlayerSeekCmd
*
\**************************************************************************/
void
VcdPlayerSeekCmd(
    REFTIME rtSeekBy
    )
{
    REFTIME rt;
    REFTIME rtDur;

    rtDur = pMovie->GetDuration();
    rt = pMovie->GetCurrentPosition() + rtSeekBy;

    rt = max(0, min(rt, rtDur));

    pMovie->SeekToPosition(rt,TRUE);
    SetCurrentPosition(pMovie->GetCurrentPosition());
}


/******************************Public*Routine******************************\
* ProcessOpen
*
\**************************************************************************/
void
ProcessOpen(
    TCHAR *achFileName,
    BOOL bPlay
    )
{
    /*
    ** If we currently have a video loaded we need to discard it here.
    */
    if(g_State & VCD_LOADED)
    {
        VcdPlayerCloseCmd();
    }

    StringCchCopy(g_achFileName, MAX_PATH, achFileName);

    pMovie = new CMovie(hwndApp);

    if(pMovie)
    {
        HRESULT hr = pMovie->OpenMovie(g_achFileName);
        if(SUCCEEDED(hr))
        {
            TCHAR achTmp[MAX_PATH];

            nRecentFiles = SetRecentFiles(achFileName, nRecentFiles, 3);

            (void)StringCchPrintf(achTmp, NUMELMS(achTmp), IdStr(STR_APP_TITLE_LOADED), g_achFileName);
            g_State = (VCD_LOADED | VCD_STOPPED);

            // SetDurationLength(pMovie->GetDuration());
            g_TimeFormat = VcdPlayerChangeTimeFormat(g_TimeFormat);

            RepositionMovie(hwndApp);
            pMovie->SetBorderClr(RGB(0x00, 0x80, 0x80));

            //  If play
            if(bPlay)
            {
                pMovie->PlayMovie();
            }
        }
        else
        {
            TCHAR Buffer[MAX_ERROR_TEXT_LEN];

            if(AMGetErrorText(hr, Buffer, MAX_ERROR_TEXT_LEN))
            {
                MessageBox(hwndApp, Buffer, IdStr(STR_APP_TITLE), MB_OK);
            }
            else
            {
                MessageBox(hwndApp,
                    TEXT("Failed to open the movie.  Either the file was ")
                    TEXT("not found or the wave device is in use."),
                    IdStr(STR_APP_TITLE), MB_OK);
            }

            pMovie->CloseMovie();
            delete pMovie;
            pMovie = NULL;
        }
    }

    InitStreamParams(0);

    InvalidateRect(hwndApp, NULL, FALSE);
    UpdateWindow(hwndApp);
}


/******************************Public*Routine******************************\
* VcdPlayerChangeTimeFormat
*
* Tries to change the time format to id.  Returns the time format that
* actually got set.  This may differ from id if the graph does not support
* the requested time format.
*
\**************************************************************************/
int
VcdPlayerChangeTimeFormat(
    int id
    )
{
    // Menu items are disabled while we are playing
    BOOL    bRet = FALSE;
    int     idActual = id;

    assert(pMovie);

    if (pMovie)
    {
        assert(pMovie->StatusMovie() != MOVIE_NOTOPENED);

        // Change the time format with the filtergraph
        switch(id)
        {
            case IDM_FRAME:
                bRet = pMovie->SetTimeFormat(TIME_FORMAT_FRAME);
                break;

            case IDM_FIELD:
                bRet = pMovie->SetTimeFormat(TIME_FORMAT_FIELD);
                break;

            case IDM_SAMPLE:
                bRet = pMovie->SetTimeFormat(TIME_FORMAT_SAMPLE);
                break;

            case IDM_BYTES:
                bRet = pMovie->SetTimeFormat(TIME_FORMAT_BYTE);
                break;
        }

        if(!bRet)
        {
            // IDM_TIME and all other cases,  everyone should support IDM_TIME
            bRet = pMovie->SetTimeFormat(TIME_FORMAT_MEDIA_TIME);
            assert(bRet);
            idActual = IDM_TIME;
        }

        // Pause the movie to get a current position

        SetDurationLength(pMovie->GetDuration());
        SetCurrentPosition(pMovie->GetCurrentPosition());
    }

    return idActual;
}


/******************************Public*Routine******************************\
* VcdPlayerRewindCmd
*
\**************************************************************************/
BOOL
VcdPlayerRewindCmd(
    void
    )
{
    if(pMovie)
    {
        pMovie->SeekToPosition((REFTIME)0,FALSE);
    }

    return TRUE;
}


