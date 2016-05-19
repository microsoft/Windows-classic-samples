// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

/****************************************************************************
*   SimpleDictation.h
*       Definition of the CSimpleDict class
*****************************************************************************/

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"
#define MAX_EDIT_TEXT   1000
#define GID_DICTATION   0           // Dictation grammar has grammar ID 0

#define WM_RECOEVENT    WM_APP      // Window message used for recognition events

class CSimpleDict
{
public:
    CSimpleDict( HINSTANCE hInstance ) :
            m_hInstance( hInstance ),
            m_bGotReco( FALSE ),
            m_bInSound( FALSE )
    {}

    static LRESULT CALLBACK CSimpleDict::SimpleDictDlgProc( HWND, UINT, WPARAM, LPARAM );
                                    // Message handler
    bool InitDialog( HWND hDlg );   // Starts up SR
    void RecoEvent();               // Called whenever dialog process is notified of recognition

    HINSTANCE                   m_hInstance;
    HWND                        m_hDlg;
    BOOL                        m_bInSound;
    BOOL                        m_bGotReco;
    LCID                        m_lcid;
    CComPtr<ISpRecoContext>     m_cpRecoCtxt;
    CComPtr<ISpRecoGrammar>     m_cpDictationGrammar;
};

