// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#ifndef __TRACEAPP__
#define __TRACEAPP__

#include "resource.h"

#include "tedobj.h"
#include "tedplayer.h"
#include "MFErrorHandler.h"
#include "..\tedutil\tedutilinc.h"
#include "tedutil.h"

class CTedAppVideoWindowHandler 
    : public ITedVideoWindowHandler
{
public:
    CTedAppVideoWindowHandler(HWND hWndParent);
    ~CTedAppVideoWindowHandler();
    
    HRESULT STDMETHODCALLTYPE GetVideoWindow(LONG_PTR* phWnd);
    HRESULT STDMETHODCALLTYPE ReleaseVideoWindow(LONG_PTR hWnd);

    HRESULT STDMETHODCALLTYPE ShowWindows(int nCmdShow);
    
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    
private:
    HWND m_hWndParent;
    LONG m_cRef;
    CAtlArray<CTedVideoWindow*> m_arrWindows;
    const static DWORD m_dwDefaultWindowWidth = 320;
    const static DWORD m_dwDefaultWindowHeight = 240;
    const static DWORD m_dwCascadeMargin = 20;
};

class CTedAppMediaEventHandler 
    : public CTedMediaEventHandler
{
public:
    CTedAppMediaEventHandler(CTedApp* pApp);

    void NotifyEventError(HRESULT hr);
    void HandleMediaEvent(IMFMediaEvent* pEvent);
    
private:
    CTedApp* m_pApp;
};

class CTedAppTopoEventHandler
    : public ITedTopoEventHandler
{
public:
    CTedAppTopoEventHandler(CTedApp* pApp);
    ~CTedAppTopoEventHandler();
    
    HRESULT STDMETHODCALLTYPE NotifyAddedNode(int nNodeID);
    HRESULT STDMETHODCALLTYPE NotifyRemovedNode(int nNodeID);
    HRESULT STDMETHODCALLTYPE NotifyConnection(int nUpNodeID, int nDownNodeID);
    HRESULT STDMETHODCALLTYPE NotifyDisconnection(int nUpNodeID, int nDownNodeID);
    
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface);
    ULONG STDMETHODCALLTYPE AddRef();
    ULONG STDMETHODCALLTYPE Release();
    
private:
    CTedApp* m_pApp;
    LONG m_cRef;
};

class CTedChooserDialog 
    : public CDialogImpl<CTedChooserDialog>
{
public:
    CTedChooserDialog(const CAtlString& strTitle);

    enum { IDD = IDD_CHOOSER };

    void AddPossibleChoice(CAtlStringW strChoice);
    CAtlStringW GetChoice() { return m_strChoice; }
protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    BEGIN_MSG_MAP( CTedChooserDialog )
       MESSAGE_HANDLER( WM_INITDIALOG, OnInitDialog )

       COMMAND_HANDLER(IDOK, 0, OnOK)
       COMMAND_HANDLER(IDCANCEL, 0, OnCancel)
    END_MSG_MAP()

private:
    CAtlString m_strTitle;
    CAtlArray<CAtlStringW> m_arrChoices;
    CAtlStringW m_strChoice;
    DWORD m_nChosenIndex;

    HWND m_hChooserCombo;
};

class CTedAboutDialog
    : public CDialogImpl<CTedAboutDialog>
{
public:
    enum { IDD = IDD_ABOUTBOX };
    
protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) { EndDialog(IDOK); return 0; }
    
    BEGIN_MSG_MAP( CTedAboutDialog )
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);

        COMMAND_HANDLER(IDOK, 0, OnOK)
    END_MSG_MAP()
};

class CTedInputURLDialog
    : public CDialogImpl<CTedInputURLDialog>
{
public:
    enum { IDD = IDD_INPUTURL };

    CAtlString GetURL();

protected:
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    BEGIN_MSG_MAP( CTedInputURLDialog )
        COMMAND_HANDLER(IDOK, 0, OnOK)
    END_MSG_MAP()

private:
    CAtlString m_strURL;
};

class CTedCaptureSourceDialog
    : public CDialogImpl<CTedCaptureSourceDialog>
{
public:
    enum { IDD = IDD_INPUTCAPSOURCE };

    CTedCaptureSourceDialog( bool bVideo );
    ~CTedCaptureSourceDialog();
    IMFActivate* GetSourceActivate();

protected:
    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        EndDialog(IDCANCEL);
        return 0;
    }

    BEGIN_MSG_MAP( CTedCaptureSourceDialog )
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_HANDLER(IDOK, 0, OnOK)
        COMMAND_HANDLER(IDCANCEL, 0, OnCancel)
    END_MSG_MAP()

private:
    HRESULT EnumCaptureSources( bool bVideo );

private:
    IMFActivate** m_ppSourceActivates;
    DWORD         m_dwActivates;

    DWORD         m_dwSelectedIndex;
};

///////////////////////////////////////////////////////////////////////////////
// 
class CTedApp 
    : public CWindowImpl<CTedApp>
{
public:
    CTedApp();
    ~CTedApp();

    HRESULT Init(LPCWSTR lpCmdLine);

    void HandleMMError(const CAtlStringW& message, HRESULT errResult);
    void HandleSeekerScroll(WORD wPos);
    void HandleRateScroll(WORD wPos);
    void NotifySplitterMoved();
    void NotifyTopoChange();

protected:    
    void HandleTopologySet(HRESULT hrTopologySet);
        
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSessionPlay(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTopologySet(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTopologyReady(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSessionEnded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSplitterMoved(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnUntrustedComponent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnProtectedContent(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnIndividualization(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnMediaCapabilitiesChanged(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
    LRESULT OnSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnLoad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnNewTopology(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddSink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddSAR(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddEVR(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddTransform(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddTee(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddCustomMFT(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddCustomSink(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddVideoCaptureSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnAddAudioCaptureSource(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnLoadTopology(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnActionPlay(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnActionStop(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnActionPause(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnSpy(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnCustomTopoloader(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRemote(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnHelpHelp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnHelpAbout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRenderFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRenderURL(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnRenderTranscode(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    
    BEGIN_MSG_MAP(CTedApp)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_MF_SESSIONPLAY, OnSessionPlay)
        MESSAGE_HANDLER(WM_MF_TOPOLOGYSET, OnTopologySet)
        MESSAGE_HANDLER(WM_MF_TOPOLOGYREADY, OnTopologyReady)
        MESSAGE_HANDLER(WM_MF_SESSIONENDED, OnSessionEnded)
        MESSAGE_HANDLER(WM_SPLITTERSIZE, OnSplitterMoved)
        MESSAGE_HANDLER(WM_TIMER, OnTimer)
        MESSAGE_HANDLER(WM_MF_HANDLE_UNTRUSTED_COMPONENT, OnUntrustedComponent)
        MESSAGE_HANDLER(WM_MF_HANDLE_PROTECTED_CONTENT, OnProtectedContent)
        MESSAGE_HANDLER(WM_MF_HANDLE_INDIVIDUALIZATION, OnIndividualization)
        MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
        MESSAGE_HANDLER(WM_MF_CAPABILITIES_CHANGED, OnMediaCapabilitiesChanged)

        COMMAND_HANDLER(ID_FILE_SAVE, 0, OnSave)
        COMMAND_HANDLER(ID_FILE_SAVE, 1, OnSave)
        COMMAND_HANDLER(ID_FILE_LOAD, 0, OnLoad)
        COMMAND_HANDLER(ID_FILE_LOAD, 1, OnLoad)
        COMMAND_HANDLER(ID_EDIT_DELETE, 0, OnDelete)
        COMMAND_HANDLER(ID_EDIT_DELETE, 1, OnDelete)
        COMMAND_HANDLER(ID_TOPOLOGY_NEWTOPOLOGY, 0, OnNewTopology)
        COMMAND_HANDLER(ID_TOPOLOGY_NEWTOPOLOGY, 1, OnNewTopology)
        COMMAND_HANDLER(ID_ADD_SOURCE, 0, OnAddSource)
        COMMAND_HANDLER(ID_ADD_SOURCE, 1, OnAddSource)
        COMMAND_HANDLER(ID_ADD_SINK, 0, OnAddSink)
        COMMAND_HANDLER(ID_ADD_SAR, 0, OnAddSAR)
        COMMAND_HANDLER(ID_ADD_SAR, 1, OnAddSAR)
        COMMAND_HANDLER(ID_ADD_EVR, 0, OnAddEVR)
        COMMAND_HANDLER(ID_ADD_EVR, 1, OnAddEVR)
        COMMAND_HANDLER(ID_ADD_TRANSFORM, 0, OnAddTransform)
        COMMAND_HANDLER(ID_ADD_TRANSFORM, 1, OnAddTransform)
        COMMAND_HANDLER(ID_ADD_TEE, 0, OnAddTee)
        COMMAND_HANDLER(ID_TOPOLOGY_ADDCUSTOMMFT, 0, OnAddCustomMFT)
        COMMAND_HANDLER(ID_TOPOLOGY_ADDCUSTOMSINK, 0, OnAddCustomSink)
        COMMAND_HANDLER(ID_TOPOLOGY_ADD_VIDEO_CAPTURE_SOURCE, 0, OnAddVideoCaptureSource)
        COMMAND_HANDLER(ID_TOPOLOGY_ADD_VIDEO_CAPTURE_SOURCE, 1, OnAddVideoCaptureSource)
        COMMAND_HANDLER(ID_TOPOLOGY_ADD_AUDIO_CAPTURE_SOURCE, 0, OnAddAudioCaptureSource)
        COMMAND_HANDLER(ID_TOPOLOGY_ADD_AUDIO_CAPTURE_SOURCE, 1, OnAddAudioCaptureSource)
        COMMAND_HANDLER(ID_LOAD_TOPOLOGY, 0, OnLoadTopology)
        COMMAND_HANDLER(ID_LOAD_TOPOLOGY, 1, OnLoadTopology)
        COMMAND_HANDLER(ID_PLAY_PLAY, 0, OnActionPlay)
        COMMAND_HANDLER(ID_PLAY_STOP, 0, OnActionStop)
        COMMAND_HANDLER(ID_PLAY_PAUSE, 0, OnActionPause)
        COMMAND_HANDLER(ID_TOOLS_SPY, 0, OnSpy)
        COMMAND_HANDLER(ID_TOOLS_CUSTOMTOPOLOADER, 0, OnCustomTopoloader)
        COMMAND_HANDLER(IDM_EXIT, 0, OnExit)
        COMMAND_HANDLER(ID_HELP_HELP, 0, OnHelpHelp)
        COMMAND_HANDLER(ID_HELP_HELP, 1, OnHelpHelp)
        COMMAND_HANDLER(ID_HELP_ABOUT, 0, OnHelpAbout)
        COMMAND_HANDLER(ID_FILE_RENDER, 0, OnRenderFile)
        COMMAND_HANDLER(ID_FILE_RENDERURL, 0, OnRenderURL)
        COMMAND_HANDLER(ID_FILE_TRANSCODE, 0, OnRenderTranscode)
    END_MSG_MAP()

public:
    CTedPlayer* PPlayer() { return m_pPlayer; }
   
protected:
    void EnableInput(UINT item, BOOL enabled);
    void RebuildDockWithOneView();
    void DisablePlayback();
    HRESULT HasBuggedPins(IMFTopology* pTopology, bool* fBuggedPins);
    void ResetInterface();
    HRESULT ResolveTopologyFromEditor();
    HRESULT SetTopologyOnPlayer(IMFTopology* pTopo, BOOL fIsProtected, BOOL fIsTranscode);
    HRESULT Play();
    void LoadFile(LPCWSTR szFile);

private:
    ITedTopoView* m_pTopoView;
    CPropertyController* m_pPropertyController;
   
    CTedAppVideoWindowHandler* m_pVideoWindowHandler;
    CTedAppTopoEventHandler* m_pTopoEventHandler;
    CTedAppMediaEventHandler* m_pMediaEventHandler;
    CTedContentProtectionManager* m_pCPM;
    
    CTedPlayer * m_pPlayer;
    CDock* m_pDock;
    CSplitterBar* m_pSplitter;
    CSplitterBar* m_pPropSplitter;
    CTedMainToolbar* m_pMainToolbar;

    HMENU m_hMenu;

    CMFErrorHandler m_MFErrorHandler;
    
    bool m_fStopTrackingUntilSessionStarted;

    IMFTopology* m_pPendingTopo;
    TOPOID m_PendingTopoID;
    bool m_fMergeRequired;
    bool m_fResolved;
    bool m_fPendingPlay;
    bool m_fCanSeek;

    HWND m_hEditWnd;
    CWindow m_EditWindow;
    
    static const DWORD m_dwMaxAcceptedFileNameLength = 1024;
    static const double m_dblInitialSplitterPos;
    static const DWORD m_dwSplitterWidth = 4;
    static unsigned int MAIN_TOOLBAR_ID;
    static const int m_nSeekerRange;
    static const UINT_PTR ms_nTimerID;
    static const DWORD ms_dwTimerLen;
};

void HandleSeekerScrollFunc(WORD wPos);
void HandleRateScrollFunc(WORD wPos);

#endif

