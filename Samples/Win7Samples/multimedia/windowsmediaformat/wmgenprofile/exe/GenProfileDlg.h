//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            GenProfileDlg.h
//
// Abstract:            The definitions for the dialog's class and supporting
//                      structures
//
//*****************************************************************************

#if !defined(AFX_GENPROFILEEXEDLG_H__BB9CC042_1C3D_409F_AA95_A8C649AC092E__INCLUDED_)
#define AFX_GENPROFILEEXEDLG_H__BB9CC042_1C3D_409F_AA95_A8C649AC092E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GenProfile_lib.h"
#include "ProfileObject.h"

/////////////////////////////////////////////////////////////////////////////
// CGenProfileDlg dialog

class CGenProfileDlg : public CDialog
{
// Construction
public:
    CGenProfileDlg(CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CGenProfileDlg)
	enum { IDD = IDD_GENPROFILEEXE_DIALOG };
	CEdit	m_txtProfileName;
	CComboBox	m_cbLanguage;
	CComboBox	m_cbPixelFormat;
	CButton	m_chkStreamIsUncompressed;
	CEdit	m_txtBandwidthBufferWindow;
	CEdit	m_txtStreamVideoVBRQuality;
    CButton    m_chkSMPTE;
    CEdit    m_txtStreamVideoMaxBufferWindow;
    CEdit    m_txtStreamVideoMaxBitrate;
    CButton    m_chkStreamVideoMaxBufferWindow;
    CComboBox    m_cbStreamVideoVBRMode;
    CButton    m_chkStreamVideoIsVBR;
    CEdit    m_txtStreamVideoQuality;
    CEdit    m_txtStreamVideoSecondsPerKeyframe;
    CEdit    m_txtStreamVideoFPS;
    CEdit    m_txtStreamBitrate;
    CEdit    m_txtStreamVideoWidth;
    CEdit    m_txtStreamVideoHeight;
    CEdit    m_txtStreamBufferWindow;
    CComboBox    m_cbStreamFormat;
    CComboBox    m_cbStreamType;
    CComboBox    m_cbStreamCodec;
    CListBox    m_lstMandatoryStreams;
    CEdit    m_txtSharedBitrate;
    CListBox    m_lstPrioritizationStreams;
    CListBox    m_lstSharingStreams;
    CButton    m_fraMutexType;
    CEdit    m_txtHelp;
    CButton    m_fraMutexStreams;
    CButton m_rbMutexTypeBitrate;
    CButton m_rbMutexTypeLanguage;
    CButton m_rbMutexTypePresentation;
    CButton m_rbBandwidthSharingTypeExclusive;
    CButton m_rbBandwidthSharingTypePartial;
    CListBox    m_lstMutexStreams;
    CListBox    m_lstProfileObjects;
	//}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CGenProfileDlg)
    public:
    virtual BOOL DestroyWindow();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    HICON m_hIcon;

    // Generated message map functions
    //{{AFX_MSG(CGenProfileDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBTNAddObject();
    afx_msg void OnMNUAddStream();
    afx_msg void OnMNUAddPrioritization();
    afx_msg void OnMNUAddMutex();
    afx_msg void OnMNUAddBandwidthSharing();
    afx_msg void OnSelchangeLSTObjects();
    afx_msg void OnBTNDeleteObject();
    afx_msg void OnBTNSaveProfile();
    afx_msg void OnSelchangeLSTMutexStreams();
    afx_msg void OnRBMutexTypeBitrate();
    afx_msg void OnRBMutexTypeLanguage();
    afx_msg void OnRBMutexTypePresentation();
    afx_msg void OnSelchangeLSTSharingStreams();
    afx_msg void OnBTNPrioritizationUp();
    afx_msg void OnBTNPrioritizationDown();
    afx_msg void OnKillfocusTXTSharedBitrate();
    afx_msg void OnSelchangeLSTMandatoryStreams();
    afx_msg void OnSelchangeCBStreamType();
    afx_msg void OnSelchangeCBStreamCodec();
    afx_msg void OnSelchangeCBStreamFormat();
    afx_msg void OnKillfocusTXTStreamBitrate();
    afx_msg void OnKillfocusTXTStreamBufferWindow();
    afx_msg void OnKillfocusTXTStreamVideoWidth();
    afx_msg void OnKillfocusTXTStreamVideoHeight();
    afx_msg void OnKillfocusTXTStreamVideoFPS();
    afx_msg void OnKillfocusTXTStreamVideoSecondsPerKeyframe();
    afx_msg void OnKillfocusTXTStreamVideoQuality();
    afx_msg void OnCHKStreamVideoVBR();
    afx_msg void OnSelchangeCBStreamVideoVBRMode();
    afx_msg void OnCHKStreamVideoMaxBufferWindow();
    afx_msg void OnKillfocusTXTStreamVideoMaxBufferWindow();
    afx_msg void OnKillfocusTXTStreamVideoMaxBitrate();
    afx_msg void OnChkSMPTE();
    afx_msg void OnRBBandwidthTypeExclusive();
    afx_msg void OnRBBandwidthTypePartial();
	afx_msg void OnKillfocusTXTStreamVideoVBRQuality();
	afx_msg void OnKillfocusTXTBandwidthBufferWindow();
	afx_msg void OnCHKStreamUncompressed();
	afx_msg void OnSelchangeCBPixelFormat();
	afx_msg void OnSelchangeCBLanguage();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()

protected:
	HRESULT SetProfileDescription( IWMProfile* pProfile );
	void DisableVideoVBRControls();
	HRESULT SelectItemWithData( CComboBox* pcbComboBox, DWORD dwRequestedItemData );
    void DisplayMessage( UINT nStringResourceIndex );
    void DisplayMessageAndTerminate( LPCTSTR tszMessage );

    DWORD GetNumberOfFormatsSupported( GUID guidStreamType, DWORD dwCodecIndex, BOOL fIsVBR, DWORD dwNumPasses );
    void ValidateMutexStreamsAgainstControl( CMutex* pMutex );
    BOOL InBitrateMutex( CStream* pHighlightedStream );

    HRESULT AddBandwidthSharingObjectToProfile( IWMProfile3* pProfile3, CBandwidthSharingObject* pBandwidthSharingObject );
    HRESULT SetStreamPrioritizationInfo( WM_STREAM_PRIORITY_RECORD* pPrioritizationInfo, CStreamPrioritizationObject* pStreamPrioritizationObject, WORD wStreamCount );
    HRESULT AddMutexToProfile( IWMProfile* pProfile, CMutex *pMutex );

    HRESULT AddBandwidthSharingObjectsToProfile( IWMProfile3* pProfile3 );
    HRESULT CreateStreamPrioritizationArray( WM_STREAM_PRIORITY_RECORD* pPrioritizationInfo, WORD wStreamCount );
    HRESULT AddMutexesToProfile( IWMProfile* pProfile );
    HRESULT AddStreamsToProfile( IWMProfile* pProfile );

    HRESULT CreateProfile( IWMProfile** ppProfile );

    HRESULT DisplayVBRControlsForCodec( DWORD dwCodecIndex, CStream* pStream );
    HRESULT RemoveStreamFromAllMutexes( CStream* pStream );
    HRESULT CodecSupportsVBRSetting( GUID guidType, DWORD dwCodecIndex, DWORD dwPasses, BOOL* pbIsSupported );
    HRESULT ShowFileStream( CStream* pStream );
    HRESULT SetDefaultsForFileStream( CStream* pStream );
    HRESULT ShowWebStream( CStream* pStream );
    HRESULT SetDefaultsForWebStream( CStream* pStream );
    HRESULT ShowImageStream( CStream* pStream );
    HRESULT SetDefaultsForImageStream( CStream* pStream );
    HRESULT ShowScriptStream( CStream* pStream );
    HRESULT SetDefaultsForScriptStream( CStream* pStream );
    HRESULT ShowVideoStream( CStream* pStream );
    HRESULT PopulatePixleFormatCB();
    HRESULT SetDefaultsForVideoStream( CStream* pStream );
    HRESULT SetDefaultsForAudioStream( CStream* pStream );
    HRESULT AddAudioFormatsToCB( BOOL fIsVBR, DWORD dwNumVBRPasses, DWORD dwCodecIndex );
    HRESULT PopulateAudioFormatCB( DWORD dwCodec );
    HRESULT PopulateWaveFormatCB();
    HRESULT ShowAudioStream( CStream* pStream );
    HRESULT ShowStreamWindowPlacement( DWORD dwStreamType );
    HRESULT ShowStream( CStream* pStream );
    HRESULT PopulateLanguageCB();
    bool StreamPrioritizationObjectExists();
    HRESULT RefreshStreamPriorityList( CStreamPrioritizationObject* pPriorityObject );
    HRESULT ShowStreamPrioritizationObject( CStreamPrioritizationObject* pStreamPrioritizationObject );
    HRESULT SelectDependanciesInListMutex( CListBox* pListBox, CMutex* pMutex );
    HRESULT SelectDependanciesInListBandwidth( CListBox* pListBox, CBandwidthSharingObject* pBandwidthSharingObject );
    HRESULT PopulateListbox( CListBox* pListBox, ProfileObjectType potRequestedType );
    void ShowWindowConfiguration( DWORD dwConfigurationIndex );
    HRESULT ShowBandwidthSharingObject( CBandwidthSharingObject* pBandwidthSharingObject );
    void ShowDialogItem( DWORD dwResourceID, int nCmdShow );
    HRESULT ShowMutex( CMutex* pMutex );
    HRESULT DisplayProfileObject( CProfileObject* pCProfileObject );
    HRESULT DeleteProfileObjectByIndex( INT nCProfileObjectIndex );
    void SelectLanguage( LCID lcid );

    DWORD m_dwNextCProfileObjectNumber;
    CProfileObject* m_pDisplayedProfileObject;
    IWMCodecInfo3* m_pCodecInfo;
    BOOL m_bIsVideoCodec;
    BOOL m_bIsAudioCodec;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GENPROFILEEXEDLG_H__BB9CC042_1C3D_409F_AA95_A8C649AC092E__INCLUDED_)
