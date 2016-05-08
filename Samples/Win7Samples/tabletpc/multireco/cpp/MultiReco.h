// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:       
//      MultiRecoApp.h
//
// Description:
//      The header file for the CMultiRecoApp class - the application window 
//      class of the MultiReco sample.
//		The methods of the class are defined in the MultiReco.cpp file.
//   
//--------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CMultiRecoApp

class CMultiRecoApp : 
    public CWindowImpl<CMultiRecoApp>,
    public IInkCollectorEventsImpl<CMultiRecoApp>,
    public IInkRecognitionEventsImpl<CMultiRecoApp>
{
public:
    // Constants 
    enum { 
        // 'Recognizer' submenu index
        mc_iSubmenuRecognizers = 1, 
        // child windows IDs
        mc_iInputWndId = 1, 
        mc_iOutputWndId = 2, 
        mc_iStatusWndId = 3,
        // recognition guide box data
        mc_iNumRowsCols = 100,
        mc_iGuideColWidth = 80, 
        mc_iGuideRowHeight = 80,
        mc_cxBoxMargin = 4,
        mc_cyBoxMargin = 4,
        // pen width
        mc_iPenWidth = 5
    };

    // Automation API interface pointers
    CComPtr<IInkCollector>          m_spIInkCollector;
    CComPtr<IInkDisp>               m_spIInkDisp;
    CComPtr<IInkStrokes>            m_spIInkStrokes;
    CComPtr<IInkCustomStrokes>      m_spIInkCustomStrokes;
    CComPtr<IInkRecognizerContext>  m_spIInkRecoContext;
    CComPtr<IInkRecognizers>        m_spIInkRecognizers;
    CComPtr<IInkRecognizerGuide>    m_spIInkRecoGuideLined;
    CComPtr<IInkRecognizerGuide>    m_spIInkRecoGuideBoxed;

    // Child windows
    CInkInputWnd    m_wndInput;
    CRecoOutputWnd  m_wndResults;
    HWND            m_hwndStatusBar;

    // Colors to be used with draw ink and output text 
    // - different for each recognizer
    static const COLORREF  mc_crColors[];
    
    // The name of the selected recognizer
    CComBSTR    m_bstrCurRecoName;
    
    // The command id of the menu item corresponding to the selected recognizer
    UINT        m_nCmdRecognizer;

	WCHAR		m_wchFile[MAX_PATH];        // The full path and name of the opened file
	WCHAR		m_wchFileTitle[MAX_PATH];   // The name and extension of the file 

    // The index of the current color in mc_crColors
    WORD        m_iColor;    

    // Static method that creates an object of the class
    static int Run(int nCmdShow);

    // Constructor
    CMultiRecoApp() :
        m_hwndStatusBar(NULL), 
        m_nCmdRecognizer(0),
        m_iColor(0)
    {
        m_wchFile[0] = m_wchFileTitle[0] = 0;
    }

    // Helper methods
    HMENU   LoadMenu();
    bool    CreateChildWindows();
    void    CreateRecoGuides();
    bool    CreateRecoContext(IInkRecognizer* pIInkRecognizer);
    void    StartNewStrokeCollection();
	void	SaveStrokeCollection();
    void    UpdateLayout();
    void    UpdateStatusBar();
    void    UpdateFilename(WCHAR* pwsFile = NULL, WCHAR* pwsFileTitle = NULL);
	bool	DoOpen(bool bNew);
	bool	DoSave(bool bSaveAs, bool bAskUser);
    bool    LoadFile(IInkDisp* pInkDisp);
    

// Declare the class objects' window class with NULL background.
// There's no need to paint CMultiRecoApp window background because
// the entire client area is covered by the child windows.
DECLARE_WND_CLASS_EX(NULL, 0, -1)
    
// ATL macro's to declare which commands/messages the class is interested in.
BEGIN_MSG_MAP(CMultiRecoApp)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_CLOSE, OnClose)
    COMMAND_RANGE_HANDLER(ID_RECOGNIZER_FIRST, ID_RECOGNIZER_LAST, OnNewStrokes)
    COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
    COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileOpen)
    COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
    COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
    COMMAND_ID_HANDLER(ID_FILE_SAVEAS, OnFileSave)
    COMMAND_ID_HANDLER(ID_FILE_EXIT, OnExit)
END_MSG_MAP()

public:

    // Ink collector event handler
    
    HRESULT OnStroke(
            IInkCursor* pIInkCursor, 
            IInkStrokeDisp* pIInkStroke, 
            VARIANT_BOOL* pbCancel
            );

    // Recognition event handler
    
    HRESULT OnRecognition(
            BSTR bstrRecognizedString, 
            VARIANT vCustomParam, 
            InkRecognitionStatus RecognitionStatus
            );

    // Window message handlers

    LRESULT OnCreate(
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& bHandled
            );
    
    LRESULT OnDestroy(
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& bHandled
            );

    LRESULT OnSize(
            UINT uMsg, 
            WPARAM wParam, 
            LPARAM lParam, 
            BOOL& bHandled
            );
    
    LRESULT OnClose(
            UINT /*uMsg*/, 
            WPARAM /*wParam*/, 
            LPARAM /*lParam*/, 
            BOOL& bHandled
            )
    {
        // Ask user to save the last changes
        // Don't close if the user canceled it (DoSave returned false)
        bHandled = !DoSave(/*bSaveAs = */false, /*bAskUser = */ true);
        return 0;
    }
    
    // Command handlers
    
    LRESULT OnFileOpen(
            WORD /*wNotifyCode*/, 
            WORD wID, 
            HWND /*hWndCtl*/, 
            BOOL& /*bHandled*/
            )
    {
        if (true == DoSave(/*bSaveAs = */false, /*bAskUser = */ true))
        {
            DoOpen(/*bNew = */ID_FILE_NEW == wID);
        }
        return 0;
    }
    
    LRESULT OnFileSave(
            WORD /*wNotifyCode*/, 
            WORD wID, 
            HWND /*hWndCtl*/, 
            BOOL& /*bHandled*/
            )
    {
	    DoSave(/*bSaveAs = */ID_FILE_SAVEAS == wID, /*bAskUser = */ false);
        return 0;
    }
    
    LRESULT OnClear(
            WORD /*wNotifyCode*/, 
            WORD /*wID*/, 
            HWND /*hWndCtl*/, 
            BOOL& /*bHandled*/
            );

    LRESULT OnExit(
            WORD /*wNotifyCode*/, 
            WORD /*wID*/, 
            HWND /*hWndCtl*/, 
            BOOL& /*bHandled*/
            )
    {
        ::SendMessage(m_hWnd, WM_CLOSE, 0, 0);
        return 0;
    }

    LRESULT OnNewStrokes(
            WORD wNotifyCode, 
            WORD wID, 
            HWND hWndCtl, 
            BOOL& bHandled
            );
};

