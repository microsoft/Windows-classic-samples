// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// Module:       
//      AdvRecoApp.h
//
// Description:
//      The header file for the CAdvRecoApp class - the application window 
//      class of the AdvReco sample.
//		The methods of the class are defined in the AdvReco.cpp file.
//   
//--------------------------------------------------------------------------

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CAdvRecoApp

class CAdvRecoApp : 
    public CWindowImpl<CAdvRecoApp>,
    public IInkCollectorEventsImpl<CAdvRecoApp>,
    public IInkRecognitionEventsImpl<CAdvRecoApp>
{
public:
    // Constants 

    enum { 
        // submenus indices
        mc_iSubmenuRecognizers = 1, 
        mc_iSubmenuInputScopes = 2, 
        mc_iSubmenuGuides = 3,
        mc_iSubmenuModes = 4,
        // child windows IDs
        mc_iInputWndId = 1, 
        mc_iOutputWndId = 2, 
        mc_iStatusWndId = 3,
        mc_iSSGestLVId = 4,
        mc_iMSGestLVId = 5,
        // recognition guide box data
        mc_iNumRowsCols = 100,
        mc_iGuideColWidth = 100, 
        mc_iGuideRowHeight = 100,
        mc_cxBoxMargin = 4,
        mc_cyBoxMargin = 4,
        // the width of the gesture list views 
        mc_cxGestLVWidth = 160, 
        // the number of the gesture names in the string table
        mc_cNumSSGestures = 36,     // single stroke gestures
        mc_cNumMSGestures =  6,     // multi-stroke gestures
        // pen width
        mc_iPenWidth = 5
    };

    // Automation API interface pointers
    CComPtr<IInkCollector>          m_spIInkCollector;
    CComPtr<IInkRenderer>           m_spIInkRenderer;
    CComPtr<IInkDisp>               m_spIInkDisp;
    CComPtr<IInkStrokes>            m_spIInkStrokes;
    CComPtr<IInkRecognizerContext>  m_spIInkRecoContext;
    CComPtr<IInkRecognizers>        m_spIInkRecognizers;
    CComPtr<IInkRecognizerGuide>    m_spIInkRecoGuide;

    // Child windows
    CInkInputWnd    m_wndInput;
    CRecoOutputWnd  m_wndResults;
    HWND            m_hwndStatusBar;
    HWND            m_hwndSSGestLV;     // single stroke gestures list view
    HWND            m_hwndMSGestLV;     // multiple stroke gestures list view

    // Helper data members
    UINT            m_nCmdRecognizer;
    UINT            m_nCmdInputScope;
    UINT            m_nCmdGuide;
    UINT            m_nCmdMode;
    CComBSTR        m_bstrCurRecoName;
    bool            m_bCoerceInputScope;
    SIZE            m_szGuideBox;
    bool            m_bAllSSGestures;
    bool            m_bAllMSGestures;

    // Static method that creates an object of the class
    static int Run(int nCmdShow);

    // Constructor
    CAdvRecoApp() :
        m_hwndStatusBar(NULL), m_hwndSSGestLV(NULL), m_hwndMSGestLV(NULL),
        m_bCoerceInputScope(false), 
        m_nCmdGuide(0), m_nCmdInputScope(0), m_nCmdRecognizer(0), m_nCmdMode(0),
        m_bAllSSGestures(true), m_bAllMSGestures(true)
    {
        m_szGuideBox.cx = m_szGuideBox.cy = 0;
    }

    // Helper methods
    HMENU   LoadMenu();
    bool    CreateChildWindows();
    void    UpdateLayout();
    void    UpdateMenuRadioItems(UINT iSubMenu, UINT idCheck, UINT idUncheck);
    void    UpdateInputScopeMenu();
    void    UpdateStatusBar();
    bool    UseRecognizer(IInkRecognizer* pIInkRecognizer);
    bool    GetGestureName(InkApplicationGesture idGesture, UINT& idGestureName);
    void    PresetGestures();
    

// Declare the class objects' window class with NULL background.
// There's no need to paint CAdvRecoApp window background because
// the entire client area is covered by the child windows.
DECLARE_WND_CLASS_EX(NULL, 0, -1)
    
// ATL macro's to declare which commands/messages the class is interested in.
BEGIN_MSG_MAP(CAdvRecoApp)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    COMMAND_ID_HANDLER(ID_RECOGNIZER_DEFAULT, OnRecognizer)
    COMMAND_RANGE_HANDLER(ID_RECOGNIZER_FIRST, ID_RECOGNIZER_LAST, OnRecognizer)
    COMMAND_ID_HANDLER(ID_INPUTSCOPE_COERCE, OnInputScopeCoerce)
    COMMAND_RANGE_HANDLER(ID_INPUTSCOPE_FIRST, ID_INPUTSCOPE_LAST, OnInputScope)
    COMMAND_ID_HANDLER(ID_GUIDE_NONE, OnGuide)
    COMMAND_ID_HANDLER(ID_GUIDE_LINES, OnGuide)
    COMMAND_ID_HANDLER(ID_GUIDE_BOXES, OnGuide)
    COMMAND_ID_HANDLER(ID_MODE_INK, OnMode)
    COMMAND_ID_HANDLER(ID_MODE_INK_AND_GESTURES, OnMode)
    COMMAND_ID_HANDLER(ID_MODE_GESTURES, OnMode)
    COMMAND_ID_HANDLER(ID_RECOGNIZE, OnRecognize)
    COMMAND_ID_HANDLER(ID_CLEAR, OnClear)
    COMMAND_ID_HANDLER(ID_EXIT, OnExit)
    NOTIFY_HANDLER(mc_iSSGestLVId, LVN_COLUMNCLICK, OnLVColumnClick)
    NOTIFY_HANDLER(mc_iMSGestLVId, LVN_COLUMNCLICK, OnLVColumnClick)
    NOTIFY_HANDLER(mc_iSSGestLVId, LVN_ITEMCHANGING, OnLVItemChanging)
    NOTIFY_HANDLER(mc_iMSGestLVId, LVN_ITEMCHANGING, OnLVItemChanging)
END_MSG_MAP()

public:

    // Window message handlers
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSetFocus(UINT, WPARAM, LPARAM, BOOL& bHandled);
    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled);
    LRESULT OnLVColumnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnLVItemChanging(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    
    // Command handlers
    LRESULT OnRecognizer(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnInputScopeCoerce(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnInputScope(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnGuide(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnMode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRecognize(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnClear(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    // Ink collector event handler
    HRESULT OnStroke(IInkCursor* pIInkCursor, IInkStrokeDisp* pIInkStroke, 
                     VARIANT_BOOL* pbCancel);
    HRESULT OnGesture(IInkCursor* pIInkCursor, IInkStrokes* pIInkStrokes, 
                      VARIANT vGestures, VARIANT_BOOL* pbCancel);

    // Recognition event handler
    HRESULT OnRecognitionWithAlternates(IInkRecognitionResult* pIInkRecoResult, 
                                        VARIANT vCustomParam,
                                        InkRecognitionStatus RecognitionStatus);
};

