//------------------------------------------------------------------------------
// File: Dialogs.cpp
//
// Desc: This file contains the implementation for the various dialog wrapper
//       classes used in DvdSample.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//------------------------------------------------------------------------------

#include "resource.h"
#include "DvdCore.h" // so we can directly access the IdvdInfo2 pointers
#include "DvdSample.h"
#include "Dialogs.h"
#include "StringUtil.h"


//------------------------------------------------------------------------------
// Name: struct LANGINFO
// Desc: An easy way of getting the lang code from the DVD_SubpictureATR data
//------------------------------------------------------------------------------
struct LANGINFO 
{
    WORD  wRes1 ;  // don't care, just skip 2 bytes
    WORD  wLang ;  // lang code as a WORD value
    WORD  wRes2 ;  // don't care, another 2 bytes
};


//------------------------------------------------------------------------------
// CAboutDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CAboutDlg::CAboutDlg()
// Desc: This is the CAboutDialog constructor.  It initializes some variables
//       that we'll need later.
//------------------------------------------------------------------------------

CAboutDlg::CAboutDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CAboutDlg::CAboutDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAboutDlg::~CAboutDlg()
// Desc: This is the CAboutDialog destructor.
//------------------------------------------------------------------------------

CAboutDlg::~CAboutDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CAboutDlg::~CAboutDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAboutDlg::DoModal()
// Desc: This method creates the dialog box and handles its return value.
//------------------------------------------------------------------------------
bool CAboutDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CAboutDlg::DoModal()"))) ;

    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_ABOUT), m_hWnd, 
        (DLGPROC) CAboutDlg::AboutDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CAboutDlg::AboutDlgProc()
// Desc: This method is the MessageProc for the CAboutDlg dialog box.  It handles
//       all windows messages sent to the dialog window.
//------------------------------------------------------------------------------

BOOL CALLBACK CAboutDlg::AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CAboutDlg * pThis;

    switch (message)
    {
        case WM_INITDIALOG:
            // get a pointer to the calling object
            pThis = reinterpret_cast<CAboutDlg *>(lParam); 
            return TRUE;

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


//------------------------------------------------------------------------------
// CSPLangDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CSPLangDlg::CSPLangDlg()
// Desc: This is the CSPLangDlg constructor.  It initializes some variables
//       that we'll need later.
//------------------------------------------------------------------------------

CSPLangDlg::CSPLangDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CSPLangDlg::CSPLangDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CSPLangDlg::~CSPLangDlg()
// Desc: This is the CSPLangDlg destructor.
//------------------------------------------------------------------------------

CSPLangDlg::~CSPLangDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CSPLangDlg::~CSPLangDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAboutDlg::DoModa()
// Desc: This method creates the dialog box and handles its return value.
//------------------------------------------------------------------------------

bool CSPLangDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CSPLangDlg::DoModal()"))) ;
    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_SPDLG), m_hWnd, 
        (DLGPROC) CSPLangDlg::SPDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CSPLangDlg::SPDlgProc()
// Desc: This is the Dialog MessageProc for the subpicture language selection
//       dialog.
//------------------------------------------------------------------------------

BOOL CALLBACK CSPLangDlg::SPDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CSPLangDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
            // get a pointer to the calling object
            pThis = reinterpret_cast<CSPLangDlg *>(lParam); 
            if (0 < pThis->MakeSPStreamList(hDlg, IDC_SPLANG))
                return TRUE;
            else
            {
                EndDialog(hDlg, FALSE);
                return FALSE;
            }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    HRESULT hr;

                    // Set the SP state specified by the user
                    pThis->m_bSPOn = IsDlgButtonChecked(hDlg, IDC_SHOWSP) ;
                    hr = g_App.m_pDvdCore->m_pIDvdC2->SetSubpictureState(pThis->m_bSPOn, 0, NULL);
                    ASSERT(SUCCEEDED(hr)) ;

                    // Set the SP stream specific by the user
                    LONG lStream;
                    lStream = (LONG) SendDlgItemMessage(hDlg, IDC_SPLANG, CB_GETCURSEL, 
                        static_cast<WPARAM>(0), static_cast<LPARAM>(0));
                    if (CB_ERR == lStream)
                        DbgLog((LOG_ERROR, 1, 
                        TEXT("WARNING: Couldn't get selected SP stream id (Error %d)"), lStream)) ;
                    else
                    {
                        pThis->m_ulSPStream = lStream;
                        hr = g_App.m_pDvdCore->m_pIDvdC2->SelectSubpictureStream(pThis->m_ulSPStream,
                            0, NULL) ;
                        ASSERT(SUCCEEDED(hr)) ;
                    }

                    EndDialog(hDlg, TRUE);
                    return TRUE;

                } // end of case brackets


                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}


//------------------------------------------------------------------------------
// Name: CSPLangDlg::MakeSPStreamList()
// Desc: This method will populate our dialog box and return the number of SP
//       streams on this disc.  We do some extra work to get around LCID limitations
//       on Win9x.
//------------------------------------------------------------------------------

int CSPLangDlg::MakeSPStreamList(HWND hDlg, int iListID)
{
    DbgLog((LOG_TRACE, 5, TEXT("CSPLangDlg::MakeSPStreamList(0x%lx, %d)"), 
        hDlg, iListID)) ;

    if (Uninitialized == g_App.m_pDvdCore->GetState())
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: DvdCore not initialized yet!"))) ;
        return 0 ;
    }

    // First clear the list box of all SP stream names
    SendDlgItemMessage(hDlg, iListID, CB_RESETCONTENT, static_cast<WPARAM>(0), 
                       static_cast<LPARAM>(0)) ;

    // Find out how many SP streams are there and what's the current lang.
    // This is our chance to find out if someone changed the SP lang through 
    // DVD menu so that we can synch up our SP stream value now.
    HRESULT hr = g_App.m_pDvdCore->m_pIDvdI2->GetCurrentSubpicture(&m_ulNumLang, &m_ulSPStream, 
        &m_bSPOn) ;
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("Not ready to find language information"), TEXT("Warning"), MB_OK) ;
        return 0 ;
    }
    m_bSPOn = !m_bSPOn; // GetCurrentSubpicture returns IsDisabled, not IsOn

    LCID lcid;
    TCHAR szLang[50];

    // Add all of the streams to the dialog box
    for (ULONG ulStream = 0; ulStream < m_ulNumLang; ulStream++)
    {
        if (FAILED(g_App.m_pDvdCore->m_pIDvdI2->GetSubpictureLanguage(ulStream, &lcid)))
        {
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: GetSubpictureLanguage Failed!"))) ;
            return 0 ;
        }

        if (0 == lcid || 0 == GetLocaleInfo(lcid, LOCALE_SLANGUAGE, 
            szLang, sizeof(szLang)/sizeof(szLang[0]))) 
        // 0 is the failure code for GetLocaleInfo
        {
            GetSPLang(ulStream, szLang, sizeof(szLang));
        }

        // Add the language to the listbox
        SendDlgItemMessage(hDlg, iListID, CB_ADDSTRING, static_cast<WPARAM>(0),
            reinterpret_cast<LPARAM>(static_cast<PVOID>(szLang)));
    }

    // set the current stream as the selected item
    if (m_ulNumLang > 0) // if there are any streams
    {
        int iRes = (int) SendDlgItemMessage(hDlg, iListID, CB_SETCURSEL, 
            static_cast<WPARAM>(m_ulSPStream), static_cast<LPARAM>(0)) ;
        if (CB_ERR == iRes)
            DbgLog((LOG_ERROR, 1, 
            TEXT("WARNING: Couldn't set %ld as selected SP stream id (Error %d)"),
            m_ulSPStream, iRes)) ;
    }

    // set the checkbox to refect the current SP state
    CheckDlgButton(hDlg, IDC_SHOWSP, m_bSPOn ? BST_CHECKED : BST_UNCHECKED) ;

    return m_ulNumLang;
}


//------------------------------------------------------------------------------
// Name: CSPLangDlg::GetSPLang()
// Desc: This method gets the LCID language code and looks up the subpicture
//       language based on that.  This is to get around problems with Win95 where
//       not all LCID's are recognized.
//------------------------------------------------------------------------------

bool CSPLangDlg::GetSPLang(ULONG ulStream, TCHAR * buffer, int iBufLen)
{
    DbgLog((LOG_TRACE, 5, TEXT("CSPLangDlg::GetSPLang()"))) ;

    DVD_SubpictureAttributes SPATR ;
    HRESULT hr = g_App.m_pDvdCore->m_pIDvdI2->GetSubpictureAttributes(ulStream, &SPATR);
    ASSERT(SUCCEEDED(hr)) ;
    return g_App.m_pLangLookup->GetLangString(SPATR.Language, buffer, iBufLen) ;
}



//------------------------------------------------------------------------------
// CAudioLangDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CAudioLangDlg::CAudioLangDlg()
// Desc: This is the CAudioLangDlg Constructor.
//------------------------------------------------------------------------------

CAudioLangDlg::CAudioLangDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CAudioLangDlg::CAudioLangDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAudioLangDlg::~CAudioLangDlg()
// Desc: This is the CAudioLangDlg destructor.
//------------------------------------------------------------------------------

CAudioLangDlg::~CAudioLangDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CAudioLangDlg::~CAudioLangDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAudioLangDlg::DoModal()
// Desc: This method creates the dialog box and handles its return value.
//------------------------------------------------------------------------------

bool CAudioLangDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CAudioLangDlg::DoModal()"))) ;
    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_AUDIODLG), m_hWnd, 
        (DLGPROC) CAudioLangDlg::AudioDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CAudioLangDlg::AudioDlgProc()
// Desc: This is the Dialog MessageProc for the audio language selection
//       dialog.
//------------------------------------------------------------------------------

BOOL CALLBACK CAudioLangDlg::AudioDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CAudioLangDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
            pThis = reinterpret_cast<CAudioLangDlg *>(lParam); // get a pointer to the calling object
            if (0 < pThis->MakeAudioStreamList(hDlg, IDC_AUDIOLANG))
                return TRUE;
            else
            {
                EndDialog(hDlg, FALSE);
                return FALSE;
            }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    HRESULT hr;

                    // Set the audio stream specific by the user
                    LONG lStream;
                    lStream = (LONG) SendDlgItemMessage(hDlg, IDC_AUDIOLANG, CB_GETCURSEL, 
                        static_cast<WPARAM>(0), static_cast<LPARAM>(0));
                    if (CB_ERR == lStream)
                        DbgLog((LOG_ERROR, 1, 
                        TEXT("WARNING: Couldn't get selected Audio stream ID (Error %d)"), lStream)) ;
                    else
                    {
                        pThis->m_ulAudioStream = lStream;
                        hr = g_App.m_pDvdCore->m_pIDvdC2->SelectAudioStream(pThis->m_ulAudioStream,
                            0, NULL) ;
                        ASSERT(SUCCEEDED(hr)) ;
                    }

                    EndDialog(hDlg, TRUE);
                    return TRUE;

                } // end of case brackets


                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}


//------------------------------------------------------------------------------
// Name: CAudioLangDlg::MakeAudioStreamList()
// Desc: This method will populate our dialog box and return the number of audio
//       streams on this disc.  We do some extra work to get around LCID limitations
//       on Win9x.
//------------------------------------------------------------------------------

int CAudioLangDlg::MakeAudioStreamList(HWND hDlg, int iListID)
{
    DbgLog((LOG_TRACE, 5, TEXT("CAudioLangDlg::MakeAudioStreamList(0x%lx, %d)"), 
        (ULONG) hDlg, iListID)) ;

    if (Uninitialized == g_App.m_pDvdCore->GetState())
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: DvdCore not initialized yet!"))) ;
        return 0 ;
    }

    // First clear the list box of all audio stream names
    SendDlgItemMessage(hDlg, iListID, CB_RESETCONTENT, static_cast<WPARAM>(0), 
        static_cast<LPARAM>(0)) ;

    // Find out how many audio streams are there and what's the current lang.
    // This is our chance to find out if someone changed the audio lang through 
    // DVD menu so that we can synch up our audio stream value now.
    HRESULT hr = g_App.m_pDvdCore->m_pIDvdI2->GetCurrentAudio(&m_ulNumLang, &m_ulAudioStream) ;
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("Not ready to find language information"), TEXT("Warning"), MB_OK) ;
        return 0 ;
    }

    LCID lcid;
    TCHAR szLang[50];

    // Add all of the streams to the dialog box
    for (ULONG ulStream = 0; ulStream < m_ulNumLang; ulStream++)
    {
        if (FAILED(g_App.m_pDvdCore->m_pIDvdI2->GetAudioLanguage(ulStream, &lcid)))
        {
            DbgLog((LOG_ERROR, 0, TEXT("WARNING: GetAudioLanguage Failed!"))) ;
            return 0 ;
        }

        if (0 == lcid || 0 == GetLocaleInfo(lcid, LOCALE_SLANGUAGE, 
            szLang, sizeof(szLang)/sizeof(szLang[0]))) 
        // 0 is the failure code for GetLocaleInfo
        {
            GetAudioLang(ulStream, szLang, sizeof(szLang));
        }

        // Add the language to the listbox
        SendDlgItemMessage(hDlg, iListID, CB_ADDSTRING, static_cast<WPARAM>(0),
            reinterpret_cast<LPARAM>(static_cast<PVOID>(szLang)));
    }

    // set the current stream as the selected item
    if (m_ulNumLang > 0) // if there are any streams
    {
        int iRes = (int) SendDlgItemMessage(hDlg, iListID, CB_SETCURSEL, 
            static_cast<WPARAM>(m_ulAudioStream), static_cast<LPARAM>(0)) ;
        if (CB_ERR == iRes)
            DbgLog((LOG_ERROR, 1, 
            TEXT("WARNING: Couldn't set %ld as selected audio stream ID (Error %d)"),
            m_ulAudioStream, iRes)) ;
    }

    return m_ulNumLang;
}


//------------------------------------------------------------------------------
// Name: CAudioLangDlg::GetAudioLang()
// Desc: This method gets the LCID language code and looks up the audio
//       language based on that.  This is to get around problems with Win95 where
//       not all LCID's are recognized.
//------------------------------------------------------------------------------

bool CAudioLangDlg::GetAudioLang(ULONG ulStream, TCHAR * buffer, int iBufLen)
{
    DbgLog((LOG_TRACE, 5, TEXT("CAudioLangDlg::GetAudioLang()"))) ;

    DVD_AudioAttributes AudATR ;
    LANGINFO * pSPATR = reinterpret_cast<LANGINFO *>(&AudATR);

    HRESULT hr = g_App.m_pDvdCore->m_pIDvdI2->GetAudioAttributes(ulStream, &AudATR);
    ASSERT(SUCCEEDED(hr)) ;

    return g_App.m_pLangLookup->GetLangString(AudATR.Language, buffer, iBufLen) ;
}


//------------------------------------------------------------------------------
// CAngleDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CAngleDlg::CAngleDlg()
// Desc: This method is the constructor for CAngleDlg
//------------------------------------------------------------------------------

CAngleDlg::CAngleDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CAngleDlg::CAngleDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAngleDlg::CAngleDlg()
// Desc: This method is the destructor for CAngleDlg
//------------------------------------------------------------------------------

CAngleDlg::~CAngleDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CAngleDlg::~CAngleDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CAngleDlg::DoModal()
// Desc: This method creates the dialog and handles its return value.
//------------------------------------------------------------------------------

bool CAngleDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CAngleDlg::DoModal()"))) ;
    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_ANGLEDLG), m_hWnd, 
        (DLGPROC) CAngleDlg::AngleDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CAngleDlg::AngleDlgProc()
// Desc: This is the Dialog MessageProc for the angle selection dialog
//------------------------------------------------------------------------------

BOOL CALLBACK CAngleDlg::AngleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CAngleDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
            // get a pointer to the calling object
            pThis = reinterpret_cast<CAngleDlg *>(lParam); 
            if (0 < pThis->MakeAngleList(hDlg, IDC_ANGLE))
                return TRUE;
            else
            {
                EndDialog(hDlg, FALSE);
                return FALSE;
            }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    HRESULT hr;

                    // Set the Angle specific by the user
                    LONG lAngle;
                    lAngle = (LONG) SendDlgItemMessage(hDlg, IDC_ANGLE, CB_GETCURSEL, 
                        static_cast<WPARAM>(0), static_cast<LPARAM>(0));
                    lAngle += 1; // angles start count at 1 so we have to account for that
                    if (CB_ERR == lAngle)
                        DbgLog((LOG_ERROR, 1, 
                        TEXT("WARNING: Couldn't get selected angle ID (Error %d)"), lAngle)) ;
                    else
                    {
                        pThis->m_ulAngle = lAngle;
                        hr = g_App.m_pDvdCore->m_pIDvdC2->SelectAngle(pThis->m_ulAngle, 0, NULL) ;
                        ASSERT(SUCCEEDED(hr)) ;
                    }

                    EndDialog(hDlg, TRUE);
                    return TRUE;

                } // end of case brackets


                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}


//------------------------------------------------------------------------------
// Name: CAngleDlg::MakeAngleList()
// Desc: This method will populate our dialog box and return the number of angles
//       on this disc.
//------------------------------------------------------------------------------

int CAngleDlg::MakeAngleList(HWND hDlg, int iListID)
{
    DbgLog((LOG_TRACE, 5, TEXT("CAngleDlg::MakeAngleList(0x%lx, %d)"), 
        hDlg, iListID)) ;

    if (Uninitialized == g_App.m_pDvdCore->GetState())
    {
        DbgLog((LOG_ERROR, 0, TEXT("WARNING: DvdCore not initialized yet!"))) ;
        return 0 ;
    }

    // First clear the list box of all angle names
    SendDlgItemMessage(hDlg, iListID, CB_RESETCONTENT, static_cast<WPARAM>(0), 
        static_cast<LPARAM>(0)) ;

    // Find out how many angles are there and what is the current angle.
    // This is our chance to find out if someone changed the angle through 
    // DVD menu so that we can synch up our angle value now.
    HRESULT hr = g_App.m_pDvdCore->m_pIDvdI2->GetCurrentAngle(&m_ulNumAngle, &m_ulAngle) ;
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("Not ready to find angle information"), TEXT("Warning"), MB_OK) ;
        return 0 ;
    }

    TCHAR szAngle[50];

    // Add all of the angles to the dialog box
    for (ULONG ulAngle = 1; ulAngle <= m_ulNumAngle; ulAngle++) // angles start at 1
    {
        hr = StringCchPrintf(szAngle, NUMELMS(szAngle), TEXT("Angle %u\0"), ulAngle);

        // Add the language to the listbox
        SendDlgItemMessage(hDlg, iListID, CB_ADDSTRING, static_cast<WPARAM>(0),
            reinterpret_cast<LPARAM>(static_cast<PVOID>(szAngle)));
    }

    // set the current angle as the selected item
    if (m_ulNumAngle > 0) // if there are any angles
    {
        // angles start at 1 so we have to subtract 1 from the angle number to match the 
        // correct angle
        int iRes = (int) SendDlgItemMessage(hDlg, iListID, CB_SETCURSEL, 
            static_cast<WPARAM>(m_ulAngle - 1), static_cast<LPARAM>(0)) ;  
        if (CB_ERR == iRes)
            DbgLog((LOG_ERROR, 1, 
            TEXT("WARNING: Couldn't set %ld as selected angle ID (Error %d)"),
            m_ulAngle, iRes)) ;
    }

    return m_ulNumAngle;
}



//------------------------------------------------------------------------------
// CChapterDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CChapterDlg::CChapterDlg()
// Desc: This method is the constructor for CChapterDlg
//------------------------------------------------------------------------------

CChapterDlg::CChapterDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CChapterDlg::CChapterDlg()"))) ;

    m_ulChapter = 1; // default value
}


//------------------------------------------------------------------------------
// Name: CChapterDlg::CChapterDlg()
// Desc: This method is the destructor for CChapterDlg
//------------------------------------------------------------------------------

CChapterDlg::~CChapterDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CChapterDlg::~CChapterDlg()"))) ;
}


//------------------------------------------------------------------------------
// Name: CChapterDlg::DoModal()
// Desc: This method creates the dialog and handles its return value.
//------------------------------------------------------------------------------

bool CChapterDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CChapterDlg::DoModal()"))) ;
    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_CHAPTERDLG), m_hWnd, 
        (DLGPROC) CChapterDlg::ChapterDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CChapterDlg::ChapterDlgProc()
// Desc: This is the Dialog MessageProc for the Chapter selection dialog
//------------------------------------------------------------------------------

BOOL CALLBACK CChapterDlg::ChapterDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CChapterDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
        {
            pThis = reinterpret_cast<CChapterDlg *>(lParam); // get a pointer to the calling object
        
            // set default value
            TCHAR buf[10];
            HRESULT hr = StringCchPrintf(buf, NUMELMS(buf), TEXT("%u\0"), pThis->m_ulChapter); 
            SetDlgItemText(hDlg, IDC_PLAYCHAPTER, buf);
        
            //set up spin control
            HWND hEBox = GetDlgItem(hDlg, IDC_PLAYCHAPTER);
            CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
                10, 10, 50, 50, hDlg, ID_SPINCONTROL, pThis->m_hInstance, hEBox, 999, 1, 1);
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    // Set the Chapter specified by the user
                    TCHAR buf[10];
                    GetDlgItemText(hDlg, IDC_PLAYCHAPTER, buf, sizeof(buf)/sizeof(TCHAR));

                    pThis->m_ulChapter = _ttoi(buf);

                    EndDialog(hDlg, TRUE);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}


//------------------------------------------------------------------------------
// CTitleDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CTitleDlg::CTitleDlg()
// Desc: This method is the constructor for CTitleDlg
//------------------------------------------------------------------------------

CTitleDlg::CTitleDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CTitleDlg::CTitleDlg"))) ;

    m_ulTitle = 1; // default value
    m_ulChapter = 1; 
}


//------------------------------------------------------------------------------
// Name: CTitleDlg::CTitleDlg()
// Desc: This method is the destructor for CTitleDlg
//------------------------------------------------------------------------------

CTitleDlg::~CTitleDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CTitleDlg::~CTitleDlg"))) ;
}


//------------------------------------------------------------------------------
// Name: CTitleDlg::DoModal()
// Desc: This method creates the dialog and handles its return value.
//------------------------------------------------------------------------------

bool CTitleDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CTitleDlg::DoModal"))) ;
    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_TITLEDLG), m_hWnd, 
        (DLGPROC) CTitleDlg::TitleDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CTitleDlg::TitleDlgProc()
// Desc: This is the Dialog MessageProc for the Title selection dialog
//------------------------------------------------------------------------------

BOOL CALLBACK CTitleDlg::TitleDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CTitleDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
        {
            pThis = reinterpret_cast<CTitleDlg *>(lParam); // get a pointer to the calling object
        
            // set default values
            TCHAR buf[10];
            HRESULT hr = StringCchPrintf(buf, NUMELMS(buf), TEXT("%u\0"), pThis->m_ulTitle); 
            SetDlgItemText(hDlg, IDC_PLAYCHAPTER, buf);
            hr = StringCchPrintf(buf, NUMELMS(buf), TEXT("%u\0"), pThis->m_ulChapter); 
            SetDlgItemText(hDlg, IDC_PLAYCHAPTER, buf);
        
            //set up spin control
            HWND hEBox = GetDlgItem(hDlg, IDC_PLAYCHAPTER);
            CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
                10, 10, 50, 50, hDlg, ID_SPINCONTROL, pThis->m_hInstance, hEBox, 999, 1, 1);

            hEBox = GetDlgItem(hDlg, IDC_PLAYTITLE);
            CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT,
                10, 10, 50, 50, hDlg, ID_SPINCONTROL, pThis->m_hInstance, hEBox, 99, 1, 1);

            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    // Set the Title specified by the user
                    TCHAR buf[10];
                    GetDlgItemText(hDlg, IDC_PLAYCHAPTER, buf, sizeof(buf)/sizeof(TCHAR));
                    pThis->m_ulChapter = _ttoi(buf);

                    GetDlgItemText(hDlg, IDC_PLAYTITLE, buf, sizeof(buf)/sizeof(TCHAR));
                    pThis->m_ulTitle = _ttoi(buf);

                    EndDialog(hDlg, TRUE);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}



//------------------------------------------------------------------------------
// CTimeDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CTimeDlg::CTimeDlg()
// Desc: This method is the constructor for CTimeDlg
//------------------------------------------------------------------------------

CTimeDlg::CTimeDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CTimeDlg::CTimeDlg"))) ;

    ZeroMemory(&m_Time, sizeof(m_Time));
}


//------------------------------------------------------------------------------
// Name: CTimeDlg::CTimeDlg()
// Desc: This method is the destructor for CTimeDlg
//------------------------------------------------------------------------------

CTimeDlg::~CTimeDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CTimeDlg::~CTimeDlg"))) ;
}


//------------------------------------------------------------------------------
// Name: CTimeDlg::DoModal()
// Desc: This method creates the dialog and handles its return value.
//------------------------------------------------------------------------------

bool CTimeDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CTimeDlg::DoModal"))) ;
    int retVal;

    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_TIMEDLG), m_hWnd, 
        (DLGPROC) CTimeDlg::TimeDlgProc, reinterpret_cast<LPARAM>(this));

    if (TRUE == retVal)
        return true;
    else 
        return false;
}


//------------------------------------------------------------------------------
// Name: CTimeDlg::TimeDlgProc()
// Desc: This is the Dialog MessageProc for the Time selection dialog
//------------------------------------------------------------------------------

BOOL CALLBACK CTimeDlg::TimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CTimeDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
        {
            // get a pointer to the calling object
            pThis = reinterpret_cast<CTimeDlg *>(lParam); 
        
            // set default values
            TCHAR buf[10];
            HRESULT hr = StringCchPrintf(buf, NUMELMS(buf), TEXT("%u\0"), pThis->m_Time.bHours); 
            SetDlgItemText(hDlg, IDC_HOURS, buf);
            hr = StringCchPrintf(buf, NUMELMS(buf), TEXT("%u\0"), pThis->m_Time.bMinutes); 
            SetDlgItemText(hDlg, IDC_MINUTES, buf);
            hr = StringCchPrintf(buf, NUMELMS(buf), TEXT("%u\0"), pThis->m_Time.bSeconds); 
            SetDlgItemText(hDlg, IDC_SECONDS, buf);
        
            //set up spin controls
            HWND hEBox = GetDlgItem(hDlg, IDC_HOURS);
            CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
                10, 10, 50, 50, hDlg, ID_SPINCONTROL, pThis->m_hInstance, hEBox, 99, 0, 
                pThis->m_Time.bHours);
            hEBox = GetDlgItem(hDlg, IDC_MINUTES);
            CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
                10, 10, 50, 50, hDlg, ID_SPINCONTROL, pThis->m_hInstance, hEBox, 60, 0, 
                pThis->m_Time.bMinutes);
            hEBox = GetDlgItem(hDlg, IDC_SECONDS);
            CreateUpDownControl(WS_CHILD | WS_BORDER | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT, 
                10, 10, 50, 50, hDlg, ID_SPINCONTROL, pThis->m_hInstance, hEBox, 60, 0, 
                pThis->m_Time.bSeconds);
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                {
                    // Set the Time specified by the user
                    TCHAR buf[10];
                    GetDlgItemText(hDlg, IDC_HOURS, buf, sizeof(buf)/sizeof(TCHAR));
                    pThis->m_Time.bHours = (BYTE) _ttoi(buf);

                    GetDlgItemText(hDlg, IDC_MINUTES, buf, sizeof(buf)/sizeof(TCHAR));
                    pThis->m_Time.bMinutes = (BYTE) _ttoi(buf);

                    GetDlgItemText(hDlg, IDC_SECONDS, buf, sizeof(buf)/sizeof(TCHAR));
                    pThis->m_Time.bSeconds = (BYTE) _ttoi(buf);

                    EndDialog(hDlg, TRUE);
                    return TRUE;
                }

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }
    return FALSE;
}



//------------------------------------------------------------------------------
// CKaraokeDlg
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Name: CKaraokeDlg::CKaraokeDlg()
// Desc: This method is the constructor for CKaraokeDlg
//------------------------------------------------------------------------------

CKaraokeDlg::CKaraokeDlg(HINSTANCE hInstance, HWND hWnd):
    m_hInstance(hInstance), m_hWnd(hWnd)
{
    DbgLog((LOG_TRACE, 5, TEXT("CKaraokeDlg::CKaraokeDlg"))) ;

    //ZeroMemory(&m_Karaoke, sizeof(m_Karaoke));
}


//------------------------------------------------------------------------------
// Name: CKaraokeDlg::CKaraokeDlg()
// Desc: This method is the destructor for CKaraokeDlg
//------------------------------------------------------------------------------

CKaraokeDlg::~CKaraokeDlg()
{
    DbgLog((LOG_TRACE, 5, TEXT("CKaraokeDlg::~CKaraokeDlg"))) ;
}


//------------------------------------------------------------------------------
// Name: CKaraokeDlg::DoModal()
// Desc: This method establishes that there is karaoke data in the current audio
//       stream.  If there is, it populates the dialog box and displays it.  It also
//       handles the mixing.  In a real application, it might be better to have a function
//       in CDvdCore which does the mixing, but we will violate encapsulation here to
//       make it easier to understand.
//
//       This method mixes a given channel into both speakers.  This can be controlled on 
//       a per-speaker basis if so desired. 
//------------------------------------------------------------------------------

bool CKaraokeDlg::DoModal()
{
    DbgLog((LOG_TRACE, 5, TEXT("CKaraokeDlg::DoModal"))) ;

    // first we should make sure that there is Karaoke Data available
    HRESULT hr;

    DVD_AudioAttributes audioAtr;
    hr = g_App.m_pDvdCore->m_pIDvdI2->GetAudioAttributes(DVD_STREAM_DATA_CURRENT, &audioAtr);
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("GetAudioAttributes Failed"), TEXT("Error!"), MB_OK);
        return false;
    }

    if( DVD_AudioMode_Karaoke != audioAtr.AppMode ) 
    {
        MessageBox(m_hWnd, TEXT("There is no Karaoke Data In This Audio Stream"), 
            TEXT("Error!"), MB_OK);
        return false;
    }

    // if we get here, we are in a karaoke section
    DVD_KaraokeAttributes karaokeAtr;
    hr = g_App.m_pDvdCore->m_pIDvdI2->GetKaraokeAttributes(DVD_STREAM_DATA_CURRENT, &karaokeAtr );
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("GetKaraokeAttributes Failed"), TEXT("Error!"), MB_OK);
        return false;
    }

    // assign names to the various channels.  Channels 0 and 1 are restricted
    m_pszChannel2 = KaraokeAsStr(karaokeAtr.wChannelContents[2]);
    m_pszChannel3 = KaraokeAsStr(karaokeAtr.wChannelContents[3]);
    m_pszChannel4 = KaraokeAsStr(karaokeAtr.wChannelContents[4]);

    int retVal;
    retVal = (int) DialogBoxParam(m_hInstance, MAKEINTRESOURCE(IDD_KARAOKEDLG), m_hWnd, 
        (DLGPROC) CKaraokeDlg::KaraokeDlgProc, reinterpret_cast<LPARAM>(this));
    if (FALSE == retVal)
    {
        return false;
    }

    // else the user clicked OK

    ULONG ulMixFlags = NULL; // Initialize the flags variable to be blank
    INT chkChannel2, chkChannel3, chkChannel4;

    chkChannel2 = (INT) SendDlgItemMessage(m_hWnd, IDC_CHANNEL2, BM_GETCHECK, 0, 0);
    chkChannel3 = (INT) SendDlgItemMessage(m_hWnd, IDC_CHANNEL3, BM_GETCHECK, 0, 0);
    chkChannel4 = (INT) SendDlgItemMessage(m_hWnd, IDC_CHANNEL4, BM_GETCHECK, 0, 0);

    if (BST_CHECKED == chkChannel2)
    {
        ulMixFlags |= DVD_Mix_3to0;
        ulMixFlags |= DVD_Mix_3to1;
    }
    if (BST_CHECKED == chkChannel3)
    {
        ulMixFlags |= DVD_Mix_4to0;
        ulMixFlags |= DVD_Mix_4to1;
    }
    if (BST_CHECKED == chkChannel4)
    {
        ulMixFlags |= DVD_Mix_Lto0;
        ulMixFlags |= DVD_Mix_Lto1;
    }

    hr = g_App.m_pDvdCore->m_pIDvdC2->SelectKaraokeAudioPresentationMode(ulMixFlags);
    if (FAILED(hr))
    {
        MessageBox(m_hWnd, TEXT("SelectKaraokeAudioPresentationMode Failed"), TEXT("Error!"), MB_OK);
        return false;
    }
    else return true;
}


//------------------------------------------------------------------------------
// Name: CKaraokeDlg::KaraokeDlgProc()
// Desc: This is the Dialog MessageProc for the Karaoke selection dialog
//------------------------------------------------------------------------------

BOOL CALLBACK CKaraokeDlg::KaraokeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static CKaraokeDlg * pThis; 

    switch (message)
    {
        case WM_INITDIALOG:
        {
            pThis = reinterpret_cast<CKaraokeDlg *>(lParam); // get a pointer to the calling object

            SetDlgItemText(hDlg, IDC_CHANNEL2, pThis->m_pszChannel2);
            SetDlgItemText(hDlg, IDC_CHANNEL3, pThis->m_pszChannel3);
            SetDlgItemText(hDlg, IDC_CHANNEL4, pThis->m_pszChannel4);
            return TRUE;
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }

    return FALSE;
}


