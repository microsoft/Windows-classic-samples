//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName:            GenProfileDlg.cpp
//
// Abstract:            The implementation for the MFC dialog.  This file
//                      contains most of the interesting functionality for the
//                      dialog, including the message handlers and the code
//                      that calls into the GenProfile static library.
//
//                      The OnBTNSaveProfile method shows the creation of a
//                      profile using the CreateProfile method and the saving
//                      of a profile to a PRX file.
//
//*****************************************************************************

#include "stdafx.h"
#include "GenProfile.h"
#include "GenProfileDlg.h"
#include "Macros.h"
#include "ControlPositionTable.h"
#include <strsafe.h>
#include <intsafe.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VBR_OFFSET 5000 // The offset used to tell if a particular format is VBR and how many passes it is
#define MAX_FILENAME_LENGTH 50


///////////////////////////////////////////////////////////////////////////////
CGenProfileDlg::CGenProfileDlg( CWnd* pParent )
    : CDialog(CGenProfileDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CGenProfileDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );

    m_dwNextCProfileObjectNumber = 1;
    m_pDisplayedProfileObject = NULL;
    m_pCodecInfo = NULL;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::DoDataExchange( CDataExchange* pDX )
{
    CDialog::DoDataExchange( pDX );
    //{{AFX_DATA_MAP(CGenProfileDlg)
    DDX_Control(pDX, IDC_TXTProfileName, m_txtProfileName);
    DDX_Control(pDX, IDC_CBLanguage, m_cbLanguage);
    DDX_Control(pDX, IDC_CBPixelFormat, m_cbPixelFormat);
    DDX_Control(pDX, IDC_CHKUncompressed, m_chkStreamIsUncompressed);
    DDX_Control(pDX, IDC_TXTBandwidthBufferWindow, m_txtBandwidthBufferWindow);
    DDX_Control(pDX, IDC_TXTStreamVideoVBRQuality, m_txtStreamVideoVBRQuality);
    DDX_Control(pDX, IDC_CHKSMPTE, m_chkSMPTE);
    DDX_Control(pDX, IDC_TXTStreamVideoMaxBufferWindow, m_txtStreamVideoMaxBufferWindow);
    DDX_Control(pDX, IDC_TXTStreamVideoMaxBitrate, m_txtStreamVideoMaxBitrate);
    DDX_Control(pDX, IDC_CHKStreamVideoMaxBufferWindow, m_chkStreamVideoMaxBufferWindow);
    DDX_Control(pDX, IDC_CBStreamVideoVBRMode, m_cbStreamVideoVBRMode);
    DDX_Control(pDX, IDC_CHKStreamVideoVBR, m_chkStreamVideoIsVBR);
    DDX_Control(pDX, IDC_TXTStreamVideoQuality, m_txtStreamVideoQuality);
    DDX_Control(pDX, IDC_TXTStreamVideoSecondsPerKeyframe, m_txtStreamVideoSecondsPerKeyframe);
    DDX_Control(pDX, IDC_TXTStreamVideoFPS, m_txtStreamVideoFPS);
    DDX_Control(pDX, IDC_TXTStreamBitrate, m_txtStreamBitrate);
    DDX_Control(pDX, IDC_TXTStreamVideoWidth, m_txtStreamVideoWidth);
    DDX_Control(pDX, IDC_TXTStreamVideoHeight, m_txtStreamVideoHeight);
    DDX_Control(pDX, IDC_TXTStreamBufferWindow, m_txtStreamBufferWindow);
    DDX_Control(pDX, IDC_CBStreamFormat, m_cbStreamFormat);
    DDX_Control(pDX, IDC_CBStreamType, m_cbStreamType);
    DDX_Control(pDX, IDC_CBStreamCodec, m_cbStreamCodec);
    DDX_Control(pDX, IDC_LSTMandatoryStreams, m_lstMandatoryStreams);
    DDX_Control(pDX, IDC_TXTSharedBitrate, m_txtSharedBitrate);
    DDX_Control(pDX, IDC_LSTPrioritizationStreams, m_lstPrioritizationStreams);
    DDX_Control(pDX, IDC_LSTSharingStreams, m_lstSharingStreams);
    DDX_Control(pDX, IDC_FRAMutexType, m_fraMutexType);
    DDX_Control(pDX, IDC_TXTHELP, m_txtHelp);
    DDX_Control(pDX, IDC_FRAMutexStreams, m_fraMutexStreams);
    DDX_Control(pDX, IDC_RBMutexTypeBitrate, m_rbMutexTypeBitrate);
    DDX_Control(pDX, IDC_RBMutexTypeLanguage, m_rbMutexTypeLanguage);
    DDX_Control(pDX, IDC_RBMutexTypePresentation, m_rbMutexTypePresentation);
    DDX_Control(pDX, IDC_RBBandwidthTypeExclusive, m_rbBandwidthSharingTypeExclusive);
    DDX_Control(pDX, IDC_RBBandwidthTypePartial, m_rbBandwidthSharingTypePartial);
    DDX_Control(pDX, IDC_LSTMutexStreams, m_lstMutexStreams);
    DDX_Control(pDX, IDC_LSTObjects, m_lstProfileObjects);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGenProfileDlg, CDialog)
    //{{AFX_MSG_MAP(CGenProfileDlg)
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTNAddObject, OnBTNAddObject)
    ON_COMMAND(IDM_AddStream, OnMNUAddStream)
    ON_COMMAND(IDM_AddPrioritization, OnMNUAddPrioritization)
    ON_COMMAND(IDM_AddMutex, OnMNUAddMutex)
    ON_COMMAND(IDM_AddBandwidthSharing, OnMNUAddBandwidthSharing)
    ON_LBN_SELCHANGE(IDC_LSTObjects, OnSelchangeLSTObjects)
    ON_BN_CLICKED(IDC_BTNDeleteObject, OnBTNDeleteObject)
    ON_BN_CLICKED(IDC_BTNSaveProfile, OnBTNSaveProfile)
    ON_LBN_SELCHANGE(IDC_LSTMutexStreams, OnSelchangeLSTMutexStreams)
    ON_BN_CLICKED(IDC_RBMutexTypeBitrate, OnRBMutexTypeBitrate)
    ON_BN_CLICKED(IDC_RBMutexTypeLanguage, OnRBMutexTypeLanguage)
    ON_BN_CLICKED(IDC_RBMutexTypePresentation, OnRBMutexTypePresentation)
    ON_LBN_SELCHANGE(IDC_LSTSharingStreams, OnSelchangeLSTSharingStreams)
    ON_BN_CLICKED(IDC_BTNPrioritizationUp, OnBTNPrioritizationUp)
    ON_BN_CLICKED(IDC_BTNPrioritizationDown, OnBTNPrioritizationDown)
    ON_EN_KILLFOCUS(IDC_TXTSharedBitrate, OnKillfocusTXTSharedBitrate)
    ON_LBN_SELCHANGE(IDC_LSTMandatoryStreams, OnSelchangeLSTMandatoryStreams)
    ON_CBN_SELCHANGE(IDC_CBStreamType, OnSelchangeCBStreamType)
    ON_CBN_SELCHANGE(IDC_CBStreamCodec, OnSelchangeCBStreamCodec)
    ON_CBN_SELCHANGE(IDC_CBStreamFormat, OnSelchangeCBStreamFormat)
    ON_EN_KILLFOCUS(IDC_TXTStreamBitrate, OnKillfocusTXTStreamBitrate)
    ON_EN_KILLFOCUS(IDC_TXTStreamBufferWindow, OnKillfocusTXTStreamBufferWindow)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoWidth, OnKillfocusTXTStreamVideoWidth)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoHeight, OnKillfocusTXTStreamVideoHeight)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoFPS, OnKillfocusTXTStreamVideoFPS)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoSecondsPerKeyframe, OnKillfocusTXTStreamVideoSecondsPerKeyframe)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoQuality, OnKillfocusTXTStreamVideoQuality)
    ON_BN_CLICKED(IDC_CHKStreamVideoVBR, OnCHKStreamVideoVBR)
    ON_CBN_SELCHANGE(IDC_CBStreamVideoVBRMode, OnSelchangeCBStreamVideoVBRMode)
    ON_BN_CLICKED(IDC_CHKStreamVideoMaxBufferWindow, OnCHKStreamVideoMaxBufferWindow)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoMaxBufferWindow, OnKillfocusTXTStreamVideoMaxBufferWindow)
    ON_EN_KILLFOCUS(IDC_TXTStreamVideoMaxBitrate, OnKillfocusTXTStreamVideoMaxBitrate)
    ON_BN_CLICKED(IDC_CHKSMPTE, OnChkSMPTE)
    ON_BN_CLICKED(IDC_RBBandwidthTypeExclusive, OnRBBandwidthTypeExclusive)
    ON_BN_CLICKED(IDC_RBBandwidthTypePartial, OnRBBandwidthTypePartial)
  ON_EN_KILLFOCUS(IDC_TXTStreamVideoVBRQuality, OnKillfocusTXTStreamVideoVBRQuality)
  ON_EN_KILLFOCUS(IDC_TXTBandwidthBufferWindow, OnKillfocusTXTBandwidthBufferWindow)
  ON_BN_CLICKED(IDC_CHKUncompressed, OnCHKStreamUncompressed)
  ON_CBN_SELCHANGE(IDC_CBPixelFormat, OnSelchangeCBPixelFormat)
  ON_CBN_SELCHANGE(IDC_CBLanguage, OnSelchangeCBLanguage)
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()


///////////////////////////////////////////////////////////////////////////////
BOOL CGenProfileDlg::DestroyWindow()
{
    //
    // Free up any resources pointed to by the profile object list
    //
    while ( m_lstProfileObjects.GetCount() > 0 )
    {
        DeleteProfileObjectByIndex( 0 );
    }

    SAFE_RELEASE( m_pCodecInfo );

    return CDialog::DestroyWindow();
}


/*
** Start of message handlers
*/

///////////////////////////////////////////////////////////////////////////////
BOOL CGenProfileDlg::OnInitDialog()
{
    HRESULT hr = S_OK;
    INT nNewStringIndex;
    DWORD dwCodecCount;
    CString strStreamType;
    CString strError;

    CDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon( m_hIcon, TRUE );            // Set big icon
    SetIcon( m_hIcon, FALSE );        // Set small icon

    hr = EnsureIWMCodecInfo3( &m_pCodecInfo );
    if ( FAILED( hr ) )
    {
        strError.LoadString( IDS_No_IWMCodecInfo3_error );
        DisplayMessageAndTerminate( strError );
        return FALSE;
    }
    assert( m_pCodecInfo );

    //
    // Setup the stream type combo box
    //
    hr = m_pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Audio, &dwCodecCount );
    if ( SUCCEEDED( hr ) && ( dwCodecCount > 0 ) ) // Only add option if an audio codec exists
    {
        strStreamType.LoadString( IDS_Audio );
        nNewStringIndex = m_cbStreamType.AddString( strStreamType );
        m_cbStreamType.SetItemData( nNewStringIndex, ST_Audio );
        m_bIsAudioCodec = TRUE;
    }
    else
    {
        m_bIsAudioCodec = FALSE;
    }

    hr = m_pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Video, &dwCodecCount );
    if ( SUCCEEDED( hr ) && ( dwCodecCount > 0 ) ) // Only add option if a video codec exists
    {
        strStreamType.LoadString( IDS_Video );
        nNewStringIndex = m_cbStreamType.AddString( strStreamType );
        m_cbStreamType.SetItemData( nNewStringIndex, ST_Video );
        m_bIsVideoCodec = TRUE;
    }
    else
    {
        m_bIsVideoCodec = FALSE;
    }

    strStreamType.LoadString( IDS_Script );
    nNewStringIndex = m_cbStreamType.AddString( strStreamType );
    m_cbStreamType.SetItemData( nNewStringIndex, ST_Script );

    strStreamType.LoadString( IDS_Image );
    nNewStringIndex = m_cbStreamType.AddString( strStreamType );
    m_cbStreamType.SetItemData( nNewStringIndex, ST_Image );

    strStreamType.LoadString( IDS_Web );
    nNewStringIndex = m_cbStreamType.AddString( strStreamType );
    m_cbStreamType.SetItemData( nNewStringIndex, ST_Web );

    strStreamType.LoadString( IDS_File );
    nNewStringIndex = m_cbStreamType.AddString( strStreamType );
    m_cbStreamType.SetItemData( nNewStringIndex, ST_File );

    return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnPaint()
{
    if ( IsIconic() )
    {
        CPaintDC dc( this ); // device context for painting

        SendMessage( WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0 );

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics( SM_CXICON );
        int cyIcon = GetSystemMetrics( SM_CYICON );
        CRect rect;
        GetClientRect( &rect );
        int x = ( rect.Width() - cxIcon + 1 ) / 2;
        int y = ( rect.Height() - cyIcon + 1 ) / 2;

        // Draw the icon
        dc.DrawIcon( x, y, m_hIcon );
    }
    else
    {
        CDialog::OnPaint();
    }
}


///////////////////////////////////////////////////////////////////////////////
HCURSOR CGenProfileDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnBTNAddObject()
{
    CMenu mnuParent;
    CMenu* pmnuContext;
    POINT pntMouseCursor;

    //
    // Load the menu resource
    //
    mnuParent.LoadMenu( MAKEINTRESOURCE( IDR_MNUAddObjectContext ) );

    pmnuContext = mnuParent.GetSubMenu( 0 );

    //
    // Get the current mouse position
    //
    ZeroMemory( &pntMouseCursor, sizeof( pntMouseCursor ) );
    GetCursorPos( &pntMouseCursor );

    //
    // Disable the stream prioritization option if a prioritization object already exists
    //
    if ( StreamPrioritizationObjectExists() )
    {
        pmnuContext->EnableMenuItem( IDM_AddPrioritization, MF_GRAYED );
    }
    else
    {
        pmnuContext->EnableMenuItem( IDM_AddPrioritization, MF_ENABLED );
    }

    //
    // Pop up a context menu to allow the user to select the type of object
    //
    if ( !pmnuContext->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, pntMouseCursor.x, pntMouseCursor.y, this ) )
    {
        return;
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnMNUAddStream()
{
    HRESULT hr = S_OK;
    CStream* pNewStream = NULL;
    INT nNewItemIndex;
    CString strObjectName;
    INT nObjectIndex;
    CProfileObject* pProfileObject;

    do
    {
        //
        // Create stream data for the new stream
        //
        pNewStream = new CStream();
        if ( !pNewStream )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Setup the stream so that it points to a valid, existing codec / format
        //
        if ( m_bIsAudioCodec )
        {
            hr = SetDefaultsForAudioStream( pNewStream ); // Prints its own error
        }
        else if ( m_bIsVideoCodec )
        {
            hr = SetDefaultsForVideoStream( pNewStream ); // Prints its own error
        }
        else
        {
            hr = SetDefaultsForScriptStream( pNewStream );
            if ( FAILED( hr ) )
            {
                DisplayMessage( IDS_Fatal_error );
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Add the new stream to the bottom of all stream prioritization objects
        //
        for ( nObjectIndex = 0; nObjectIndex < m_lstProfileObjects.GetCount(); nObjectIndex++ )
        {
            pProfileObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nObjectIndex );
            assert( pProfileObject );
            if ( (CProfileObject*) -1 == pProfileObject )
            {
                hr = E_UNEXPECTED;
                DisplayMessage( IDS_Fatal_error );
                break;
            }

            if ( OT_StreamPrioritization == pProfileObject->Type() )
            {
                ((CStreamPrioritizationObject*) pProfileObject)->AddStream( pNewStream );
            }
        }
        if ( FAILED( hr ) )
        {
            DisplayMessage( IDS_Fatal_error );
            break;
        }

        //
        // Create a new list entry for the new stream
        //
        strObjectName.Format( _T("Stream %d"), m_dwNextCProfileObjectNumber++ );
        pNewStream->SetName( strObjectName );
        nNewItemIndex = m_lstProfileObjects.AddString( strObjectName );
        if ( ( LB_ERR == nNewItemIndex ) || ( LB_ERRSPACE  == nNewItemIndex ) )
        {
            hr = E_UNEXPECTED;
            DisplayMessage( IDS_Fatal_error );
            break;
        }

        SAFE_ADDREF( pNewStream );
        m_lstProfileObjects.SetItemDataPtr( nNewItemIndex, pNewStream );

        //
        // Redraw the controls, since there might've been a change that affects the current object
        //
        hr = DisplayProfileObject( m_pDisplayedProfileObject );
        if ( FAILED( hr ) )
        {
            DisplayMessage( IDS_Fatal_error );
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_RELEASE( pNewStream );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnMNUAddPrioritization()
{
    HRESULT hr = S_OK;
    CStreamPrioritizationObject *pNewStreamPrioritizationObject = NULL;
    INT nNewItemIndex;
    CString strObjectName;
    INT nProfileObjectIndex;
    CProfileObject* pCurrentObject;


    do
    {
        //
        // Create bandwidth sharing data
        //
        pNewStreamPrioritizationObject = new CStreamPrioritizationObject();
        if ( !pNewStreamPrioritizationObject )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Add all the existing streams, for prioritization
        //
        for ( nProfileObjectIndex = 0; nProfileObjectIndex < m_lstProfileObjects.GetCount(); nProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nProfileObjectIndex );
            assert( pCurrentObject );
            if ( (CProfileObject*) -1 == pCurrentObject )
            {
                hr = E_UNEXPECTED;
                break;
            }

            if ( OT_Stream == pCurrentObject->Type() )
            {
                pNewStreamPrioritizationObject->AddStream( (CStream*) pCurrentObject );
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Create a new list entry for the new stream
        //
        strObjectName.Format( _T("Prioritization %d"), m_dwNextCProfileObjectNumber++ );
        nNewItemIndex = m_lstProfileObjects.AddString( strObjectName );
        if ( ( LB_ERR == nNewItemIndex ) || ( LB_ERRSPACE  == nNewItemIndex ) )
        {
            hr = E_UNEXPECTED;
            break;
        }

        SAFE_ADDREF( pNewStreamPrioritizationObject );
        m_lstProfileObjects.SetItemDataPtr( nNewItemIndex, pNewStreamPrioritizationObject );
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_RELEASE( pNewStreamPrioritizationObject );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnMNUAddMutex()
{
    HRESULT hr = S_OK;
    CMutex* pNewMutex = NULL;
    INT nNewItemIndex;
    CString strObjectName;


    do
    {
        //
        // Create mutex data for the new stream
        //
        pNewMutex = new CMutex();
        if ( !pNewMutex )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Set the defaults for the mutex
        //
        pNewMutex->SetMutexType( MT_Bitrate );

        //
        // Create a new list entry for the new stream
        //
        strObjectName.Format( _T("Mutex %d"), m_dwNextCProfileObjectNumber++ );
        nNewItemIndex = m_lstProfileObjects.AddString( strObjectName );
        if ( ( LB_ERR == nNewItemIndex ) || ( LB_ERRSPACE  == nNewItemIndex ) )
        {
            hr = E_UNEXPECTED;
            break;
        }

        SAFE_ADDREF( pNewMutex );
        m_lstProfileObjects.SetItemDataPtr( nNewItemIndex, pNewMutex );
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_RELEASE( pNewMutex );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnMNUAddBandwidthSharing()
{
    HRESULT hr = S_OK;
    CBandwidthSharingObject* pNewBandwidthSharingObject = NULL;
    INT nNewItemIndex;
    CString strObjectName;


    do
    {
        //
        // Create bandwidth sharing data
        //
        pNewBandwidthSharingObject = new CBandwidthSharingObject();
        if ( !pNewBandwidthSharingObject )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Set defaults for object
        //
        pNewBandwidthSharingObject->SetBandwidthSharingType( CLSID_WMBandwidthSharing_Exclusive );
        pNewBandwidthSharingObject->SetSharedBitrate( 100000 );

        //
        // Create a new list entry for the new stream
        //
        strObjectName.Format( _T("Bandwidth sharing %d"), m_dwNextCProfileObjectNumber++ );
        nNewItemIndex = m_lstProfileObjects.AddString( strObjectName );
        if ( ( LB_ERR == nNewItemIndex ) || ( LB_ERRSPACE  == nNewItemIndex ) )
        {
            hr = E_UNEXPECTED;
            break;
        }

        SAFE_ADDREF( pNewBandwidthSharingObject );
        m_lstProfileObjects.SetItemDataPtr( nNewItemIndex, pNewBandwidthSharingObject );
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_RELEASE( pNewBandwidthSharingObject );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeLSTObjects()
{
    HRESULT hr = S_OK;
    INT nSelectedProfileObjectIndex;
    CProfileObject *pSelectedProfileObject;

    do
    {
        //
        // Get the item that has been selected
        //
        nSelectedProfileObjectIndex = m_lstProfileObjects.GetCurSel();
        if ( LB_ERR == nSelectedProfileObjectIndex )
        {
            pSelectedProfileObject = NULL;
        }
        else
        {
            pSelectedProfileObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nSelectedProfileObjectIndex );
            if ( (CProfileObject*) -1 == pSelectedProfileObject )
            {
                DisplayMessage( IDS_Fatal_error );
                pSelectedProfileObject = NULL;
            }
        }

        hr = DisplayProfileObject( pSelectedProfileObject );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnBTNDeleteObject()
{
    HRESULT hr = S_OK;
    INT nSelectedProfileObjectIndex;

    do
    {
        //
        // Get the selected object
        //
        nSelectedProfileObjectIndex = m_lstProfileObjects.GetCurSel();
        if ( LB_ERR == nSelectedProfileObjectIndex ) // Nothing selected
        {
            break;
        }

        //
        // Delete the profile object
        //
        hr = DeleteProfileObjectByIndex( nSelectedProfileObjectIndex );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Update the display
        //
        hr = DisplayProfileObject( NULL );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnBTNSaveProfile()
{
    HRESULT hr = S_OK;
    IWMProfileManager* pProfileManager = NULL;
    IWMProfile* pProfile = NULL;
    LPWSTR wszProfileData = NULL;
    DWORD dwProfileDataLength;
    CFileDialog cSaveDialog( FALSE, _T(".prx"), NULL, OFN_OVERWRITEPROMPT, _T("Profiles (*.prx)|*.prx||"), this );
    CString strMessage;

    do
    {
        //
        // Get the filename from a common dialog box
        //
        if ( IDOK != cSaveDialog.DoModal() )
        {
            break;
        }

        //
        // Create the profile manager
        //
        hr = WMCreateProfileManager( &pProfileManager );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Create the profile based off the data in the dialog
        //
        hr = CreateProfile( &pProfile );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pProfile );

        //
        // Convert the profile to XML
        //
        dwProfileDataLength = 0;
        hr = pProfileManager->SaveProfile( pProfile, NULL, &dwProfileDataLength );
        if ( FAILED( hr ) )
        {
            break;
        }

        wszProfileData = new WCHAR[ dwProfileDataLength + 1 ];
        if ( !wszProfileData )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = pProfileManager->SaveProfile( pProfile, wszProfileData, &dwProfileDataLength );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Write the profile to a file
        //
        hr = WriteProfileAsPRX( cSaveDialog.GetPathName(), wszProfileData, dwProfileDataLength * sizeof( WCHAR ) );
        if ( FAILED( hr ) )
        {
            break;
        }

        DisplayMessage( IDS_Profile_saved );
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Cant_create_profile_error );
    }

    SAFE_RELEASE( pProfile );
    SAFE_RELEASE( pProfileManager );
    SAFE_ARRAYDELETE( wszProfileData );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeLSTMutexStreams()
{
    CMutex* pSelectedMutex;

    assert( m_pDisplayedProfileObject );
    assert( OT_Mutex == m_pDisplayedProfileObject->Type() );

    do
    {
        pSelectedMutex = (CMutex*) m_pDisplayedProfileObject;

        ValidateMutexStreamsAgainstControl( pSelectedMutex );
    }
    while ( FALSE );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnRBMutexTypeBitrate()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_Mutex == m_pDisplayedProfileObject->Type() );

    ((CMutex*) m_pDisplayedProfileObject)->SetMutexType( MT_Bitrate );

    //
    // Validate that the streams in the mutex are allowed
    //
    ValidateMutexStreamsAgainstControl( (CMutex*) m_pDisplayedProfileObject );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnRBMutexTypeLanguage()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_Mutex == m_pDisplayedProfileObject->Type() );

    ((CMutex*) m_pDisplayedProfileObject)->SetMutexType( MT_Language );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnRBMutexTypePresentation()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_Mutex == m_pDisplayedProfileObject->Type() );

    ((CMutex*) m_pDisplayedProfileObject)->SetMutexType( MT_Presentation );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnCHKStreamUncompressed()
{
    BOOL fIsChecked;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );

    fIsChecked = ( m_chkStreamIsUncompressed.GetCheck() == BST_CHECKED );
    ((CStream*) m_pDisplayedProfileObject)->SetIsUncompressed( fIsChecked );

    //
    // Turn off VBR
    //
    ((CStream*) m_pDisplayedProfileObject)->SetVideoIsVBR( FALSE );

    //
    // Update the display, since changing this will affect the other controls
    //
    ShowStream( (CStream*) m_pDisplayedProfileObject );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeLSTSharingStreams()
{
    HRESULT hr = S_OK;
    INT nSelectedStreamsCount;
    INT* aSelectedStreams = NULL;
    INT nSelectedStreamIndex;
    CStream* pHighlightedStream;
    CBandwidthSharingObject* pSelectedBandwidthSharingObject;

    assert( m_pDisplayedProfileObject );
    assert( OT_BandwidthSharing == m_pDisplayedProfileObject->Type() );

    do
    {
        pSelectedBandwidthSharingObject = (CBandwidthSharingObject*) m_pDisplayedProfileObject;
        pSelectedBandwidthSharingObject->RemoveAllStreams();

        //
        // Get the list of selected items
        //
        nSelectedStreamsCount = m_lstSharingStreams.GetSelCount();
        aSelectedStreams = new INT[ nSelectedStreamsCount ];
        if ( !aSelectedStreams )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if ( LB_ERR == m_lstSharingStreams.GetSelItems( nSelectedStreamsCount, aSelectedStreams ) )
        {
            hr = E_UNEXPECTED;
            break;
        }

        //
        // Select all the streams which should be selected
        //
        for( nSelectedStreamIndex = 0; nSelectedStreamIndex < nSelectedStreamsCount; nSelectedStreamIndex++ )
        {
            pHighlightedStream = (CStream*) m_lstSharingStreams.GetItemDataPtr( aSelectedStreams[ nSelectedStreamIndex ] );
            assert( pHighlightedStream );
            if ( (CStream*) -1 == pHighlightedStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            pSelectedBandwidthSharingObject->AddStream( pHighlightedStream );
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_ARRAYDELETE( aSelectedStreams );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnBTNPrioritizationUp()
{
    HRESULT hr = S_OK;
    CStreamPrioritizationObject* pDisplayedObject;
    CStream* pSelectedStream;
    INT nSelectedIndex;

    assert( m_pDisplayedProfileObject );
    assert( OT_StreamPrioritization == m_pDisplayedProfileObject->Type() );

    do
    {
        //
        // Get the stream selected in the listbox
        //
        pDisplayedObject = (CStreamPrioritizationObject*) m_pDisplayedProfileObject;
        nSelectedIndex = m_lstPrioritizationStreams.GetCurSel();

        if ( 0 != nSelectedIndex )
        {
            pSelectedStream = (CStream*) m_lstPrioritizationStreams.GetItemDataPtr( nSelectedIndex );
            assert( pSelectedStream );
            if ( (CStream*) -1 == pSelectedStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            hr = pDisplayedObject->IncreasePriority( pSelectedStream );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = RefreshStreamPriorityList( pDisplayedObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( LB_ERR == m_lstPrioritizationStreams.SetCurSel( nSelectedIndex - 1 ) )
            {
                break;
            }
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnBTNPrioritizationDown()
{
    HRESULT hr = S_OK;
    CStreamPrioritizationObject* pDisplayedObject;
    CStream* pSelectedStream;
    INT nSelectedIndex;

    assert( m_pDisplayedProfileObject );
    assert( OT_StreamPrioritization == m_pDisplayedProfileObject->Type() );

    do
    {
        //
        // Get the stream selected in the listbox
        //
        pDisplayedObject = (CStreamPrioritizationObject*) m_pDisplayedProfileObject;
        nSelectedIndex = m_lstPrioritizationStreams.GetCurSel();

        if ( nSelectedIndex != m_lstPrioritizationStreams.GetCount() - 1 )
        {
            pSelectedStream = (CStream*) m_lstPrioritizationStreams.GetItemDataPtr( nSelectedIndex );
            assert( pSelectedStream );
            if ( (CStream*) -1 == pSelectedStream )
            {
                break;
            }

            hr = pDisplayedObject->DecreasePriority( pSelectedStream );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = RefreshStreamPriorityList( pDisplayedObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( LB_ERR == m_lstPrioritizationStreams.SetCurSel( nSelectedIndex + 1 ) )
            {
                break;
            }
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTSharedBitrate()
{
    CString strSharedBitrate;
    DWORD dwSharedBitrate;

    assert( m_pDisplayedProfileObject );
    assert( OT_BandwidthSharing == m_pDisplayedProfileObject->Type() );

    m_txtSharedBitrate.GetWindowText( strSharedBitrate );
    dwSharedBitrate = _ttol( strSharedBitrate );

    ((CBandwidthSharingObject*) m_pDisplayedProfileObject)->SetSharedBitrate( dwSharedBitrate );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeLSTMandatoryStreams()
{
    HRESULT hr = S_OK;
    INT nSelectedStreamsCount;
    INT* aSelectedStreams = NULL;
    INT nSelectedStreamIndex;
    CStream* pHighlightedStream;
    CStreamPrioritizationObject* pSelectedObject;

    assert( m_pDisplayedProfileObject );
    assert( OT_StreamPrioritization == m_pDisplayedProfileObject->Type() );

    do
    {
        pSelectedObject = (CStreamPrioritizationObject*) m_pDisplayedProfileObject;
        hr = pSelectedObject->ClearMandatoryStreams();
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Get the list of selected items
        //
        nSelectedStreamsCount = m_lstMandatoryStreams.GetSelCount();
        aSelectedStreams = new INT[ nSelectedStreamsCount ];
        if ( !aSelectedStreams )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if ( LB_ERR == m_lstMandatoryStreams.GetSelItems( nSelectedStreamsCount, aSelectedStreams ) )
        {
            hr = E_UNEXPECTED;
            break;
        }

        //
        // Select all the streams which should be selected
        //
        for( nSelectedStreamIndex = 0; nSelectedStreamIndex < nSelectedStreamsCount; nSelectedStreamIndex++ )
        {
            pHighlightedStream = (CStream*) m_lstMandatoryStreams.GetItemDataPtr( aSelectedStreams[ nSelectedStreamIndex ] );
            assert( pHighlightedStream );
            if ( (CStream*) -1 == pHighlightedStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            pSelectedObject->SetStreamMandatory( pHighlightedStream, TRUE );
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_ARRAYDELETE( aSelectedStreams );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeCBStreamType()
{
    HRESULT hr = S_OK;
    CString strSelectedStreamType;
    INT nSelectedIndex;
    DWORD nSelectedType;
    StreamType stOriginalType;
    CString strMessage;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );

    do
    {
        nSelectedIndex = m_cbStreamType.GetCurSel();
        if ( CB_ERR == nSelectedIndex )
        {
            return;
        }
        nSelectedType = ( DWORD )( m_cbStreamType.GetItemData( nSelectedIndex ) );
        if ( CB_ERR == nSelectedType )
        {
            return;
        }

        //
        // If the stream type changed, then remove it from any mutexes, since it won't be valid anymore
        //
        stOriginalType = ((CStream*) m_pDisplayedProfileObject)->GetStreamType();
        if ( (StreamType) nSelectedType != stOriginalType )
        {
            hr = RemoveStreamFromAllMutexes( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }
            else
            {
                DisplayMessage( IDS_Stream_remove_from_mutexes );
            }
        }

        //
        // Set the defaults for the new stream type
        //
        switch ( nSelectedType )
        {
        case ST_Audio:
            assert( m_bIsAudioCodec );
            hr = SetDefaultsForAudioStream( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = ShowAudioStream( (CStream*) m_pDisplayedProfileObject );
            break;

        case ST_Video:
            assert( m_bIsVideoCodec );
            hr = SetDefaultsForVideoStream( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = ShowVideoStream( (CStream*) m_pDisplayedProfileObject );
            break;

        case ST_Script:
            hr = SetDefaultsForScriptStream( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = ShowScriptStream( (CStream*) m_pDisplayedProfileObject );
            break;

        case ST_Image:
            hr = SetDefaultsForImageStream( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = ShowImageStream( (CStream*) m_pDisplayedProfileObject );
            break;

        case ST_Web:
            hr = SetDefaultsForWebStream( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = ShowWebStream( (CStream*) m_pDisplayedProfileObject );
            break;

        case ST_File:
            hr = SetDefaultsForFileStream( (CStream*) m_pDisplayedProfileObject );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = ShowFileStream( (CStream*) m_pDisplayedProfileObject );
            break;

        default:
            assert( "Unknown stream type selected!" );
            m_cbStreamType.SetCurSel( ST_Script );
            hr = SetDefaultsForScriptStream( (CStream*) m_pDisplayedProfileObject );

            DisplayMessage( IDS_Fatal_error );
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    ShowStreamWindowPlacement( nSelectedType );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeCBStreamCodec()
{
    HRESULT hr = S_OK;
    CString strSelectedFormat;
    INT nSelectedIndex;
    DWORD dwCodecIndex;
    DWORD dwStreamFormatIndex;
    DWORD dwStreamFormatStringIndex;
    CStream* pStream;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Audio ) ||
            ( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Video ) );

    //
    // Get the selected codec from the control
    //
    m_cbStreamCodec.GetWindowText( strSelectedFormat );
    nSelectedIndex = m_cbStreamCodec.GetCurSel();
    assert( CB_ERR != nSelectedIndex );

    pStream = (CStream*) m_pDisplayedProfileObject;

    //
    // Depending on the type of the stream, adjust the stream settings / window configuration
    //
    switch ( pStream->GetStreamType() )
    {
    case ST_Audio:
        pStream->SetStreamCodecIndex( nSelectedIndex );

        dwCodecIndex = ( DWORD )( m_cbStreamCodec.GetItemData( nSelectedIndex ) );
        assert( CB_ERR != dwCodecIndex );

        hr = PopulateAudioFormatCB( dwCodecIndex );
        if ( FAILED( hr ) )
        {
            break;
        }

        dwStreamFormatStringIndex = 0; // Default to the first format in the combo box
        assert( dwStreamFormatStringIndex < (DWORD) m_cbStreamFormat.GetCount() );
        dwStreamFormatIndex = ( DWORD )( m_cbStreamFormat.GetItemData( dwStreamFormatStringIndex ) );

        pStream->SetStreamFormatIndex( dwStreamFormatIndex );
        pStream->SetStreamFormatStringIndex( dwStreamFormatStringIndex );
        m_cbStreamFormat.SetCurSel( dwStreamFormatStringIndex );
        break;

    case ST_Video:
        pStream->SetStreamCodecIndex( nSelectedIndex );

        dwCodecIndex = ( DWORD )( m_cbStreamCodec.GetItemData( nSelectedIndex ) );
        assert( CB_ERR != dwCodecIndex );

        pStream->SetVideoIsVBR( FALSE );
        pStream->SetVideoVBRMode( VBR_QUALITYBASED );
        hr = DisplayVBRControlsForCodec( dwCodecIndex, (CStream*) m_pDisplayedProfileObject );
        if ( FAILED( hr ) )
        {
            break;
        }
        break;

    default:
        assert( !"Invalid stream type from which to be switching codec!" );
        hr = E_UNEXPECTED;
    }

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeCBStreamFormat()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Audio );

    if ( !((CStream*) m_pDisplayedProfileObject)->GetIsUncompressed() )
    {
        INT nSelectedIndex;
        DWORD dwFormatIndex;

        nSelectedIndex = m_cbStreamFormat.GetCurSel();
        assert( CB_ERR != nSelectedIndex );

        dwFormatIndex = ( DWORD )( m_cbStreamFormat.GetItemData( nSelectedIndex ) );
        assert( CB_ERR != dwFormatIndex );

        ((CStream*) m_pDisplayedProfileObject)->SetStreamFormatIndex( dwFormatIndex );

        ((CStream*) m_pDisplayedProfileObject)->SetStreamFormatStringIndex( nSelectedIndex );
    }
    else
    {
        DWORD dwStringIndex = m_cbStreamFormat.GetCurSel();
        DWORD dwIndex = ( DWORD )( m_cbStreamFormat.GetItemData( dwStringIndex ) );

        ((CStream*) m_pDisplayedProfileObject)->SetWaveFormatStringIndex( dwStringIndex );
        ((CStream*) m_pDisplayedProfileObject)->SetWaveFormatIndex( dwIndex );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamBitrate()
{
    CString strBitrate;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() ) ||
            ( ST_Script == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() ) ||
            ( ST_Image == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() ) ||
            ( ST_Web == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() ) ||
            ( ST_File == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() ) ||
            ( ST_Arbitrary == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() ) );

    //
    // Set the bitrate on the stream
    //
    m_txtStreamBitrate.GetWindowText( strBitrate );
    ((CStream*) m_pDisplayedProfileObject)->SetStreamBitrate( _ttol( strBitrate ) );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamBufferWindow()
{
    CString strBufferWindow;
    DWORD dwBufferWindow;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );

    m_txtStreamBufferWindow.GetWindowText( strBufferWindow );
    dwBufferWindow = _ttol( strBufferWindow );

    ((CStream*) m_pDisplayedProfileObject)->SetStreamBufferWindow( dwBufferWindow );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoWidth()
{
    CString strWidth;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() );

    //
    // Set the width on the video stream
    //
    m_txtStreamVideoWidth.GetWindowText( strWidth );
    ((CStream*) m_pDisplayedProfileObject)->SetVideoWidth( _ttol( strWidth ) );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoHeight()
{
    CString strHeight;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() );

    //
    // Set the height on the video stream
    //
    m_txtStreamVideoWidth.GetWindowText( strHeight );
    ((CStream*) m_pDisplayedProfileObject)->SetVideoHeight( _ttol( strHeight ) );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoFPS()
{
    CString strFPS;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() );

    //
    // Set the FPS on the video stream
    //
    m_txtStreamVideoFPS.GetWindowText( strFPS );
    ((CStream*) m_pDisplayedProfileObject)->SetVideoFPS( _ttol( strFPS ) );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoSecondsPerKeyframe()
{
    CString strSecondsPerKeyframe;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() );

    //
    // Set the number of seconds between key frames on the video stream
    //
    m_txtStreamVideoSecondsPerKeyframe.GetWindowText( strSecondsPerKeyframe );
    ((CStream*) m_pDisplayedProfileObject)->SetVideoSecondsPerKeyframe( _ttol( strSecondsPerKeyframe ) );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoQuality()
{
    CString strQuality;
    DWORD dwQuality;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() );

    //
    // Set the quality on the video stream
    //
    m_txtStreamVideoQuality.GetWindowText( strQuality );
    dwQuality = _ttol( strQuality );
    if ( dwQuality > 100 )
    {
        dwQuality = 100;
        strQuality.Format( _T("%ld"), dwQuality );
        m_txtStreamVideoQuality.SetWindowText( strQuality );
    }

    ((CStream*) m_pDisplayedProfileObject)->SetVideoQuality( dwQuality );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnCHKStreamVideoVBR()
{
    HRESULT hr = S_OK;
    BOOL fIsChecked;
    CStream* pStream;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Video );

    do
    {
        pStream = (CStream*) m_pDisplayedProfileObject;

        fIsChecked = ( m_chkStreamVideoIsVBR.GetCheck() == BST_CHECKED );
        if ( !fIsChecked )
        {
            m_cbStreamVideoVBRMode.EnableWindow( FALSE );
            m_txtStreamVideoVBRQuality.EnableWindow( FALSE );
            pStream->SetVideoIsVBR( FALSE );
        }
        else
        {
            m_cbStreamVideoVBRMode.EnableWindow( TRUE );
            m_txtStreamVideoVBRQuality.EnableWindow( TRUE );
            pStream->SetVideoIsVBR( TRUE );
            pStream->SetVideoVBRMode( VBR_QUALITYBASED );
            pStream->SetVideoVBRQuality( 0 );

            hr = SelectItemWithData( &m_cbStreamVideoVBRMode, VBR_QUALITYBASED );
            if ( FAILED( hr ) )
            {
                break;
            }
        }

        hr = DisplayVBRControlsForCodec( pStream->GetStreamCodecIndex(), pStream );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeCBStreamVideoVBRMode()
{
    HRESULT hr = S_OK;
    INT nSelectedIndex;
    DWORD dwCodecIndex;
    DWORD dwSelectedCodec;
    VIDEO_VBR_MODE vbrMode;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Video );
    assert( BST_CHECKED == m_chkStreamVideoIsVBR.GetCheck() );

    do
    {
        nSelectedIndex = m_cbStreamVideoVBRMode.GetCurSel();
        assert( CB_ERR != nSelectedIndex );

        dwSelectedCodec = m_cbStreamCodec.GetCurSel();
        if ( CB_ERR == dwSelectedCodec )
        {
            hr = E_UNEXPECTED;
            break;
        }

        dwCodecIndex = ( DWORD )( m_cbStreamCodec.GetItemData( dwSelectedCodec ) );
        if ( CB_ERR == dwCodecIndex )
        {
            hr = E_UNEXPECTED;
            break;
        }

        vbrMode = (VIDEO_VBR_MODE) m_cbStreamVideoVBRMode.GetItemData( nSelectedIndex );
        assert( CB_ERR != vbrMode );
        ((CStream*)m_pDisplayedProfileObject)->SetVideoVBRMode( vbrMode );
        ((CStream*)m_pDisplayedProfileObject)->SetVideoMaxBufferWindow( 0 );

        hr = DisplayVBRControlsForCodec( dwCodecIndex, (CStream*) m_pDisplayedProfileObject );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnCHKStreamVideoMaxBufferWindow()
{
    BOOL fIsChecked;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Video );

    fIsChecked = ( m_chkStreamVideoMaxBufferWindow.GetCheck() == BST_CHECKED );
    if ( !fIsChecked )
    {
        m_txtStreamVideoMaxBufferWindow.EnableWindow( FALSE );
        ((CStream*) m_pDisplayedProfileObject)->SetVideoMaxBufferWindow( 0 );
    }
    else
    {
        m_txtStreamVideoMaxBufferWindow.EnableWindow( TRUE );
        m_txtStreamVideoMaxBufferWindow.SetWindowText( _T("10000") );

        ((CStream*) m_pDisplayedProfileObject)->SetVideoMaxBufferWindow( 10000 );
    }
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoMaxBufferWindow()
{
    CString strMaxBufferWindow;
    DWORD dwMaxBufferWindow;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Video );
    assert( BST_CHECKED == m_chkStreamVideoIsVBR.GetCheck() );

    m_txtStreamVideoMaxBufferWindow.GetWindowText( strMaxBufferWindow );
    dwMaxBufferWindow = _ttol( strMaxBufferWindow );

    //
    // Make the minimum 1
    //
    if ( 0 == dwMaxBufferWindow )
    {
        dwMaxBufferWindow = 1;
        strMaxBufferWindow.Format( _T("%ld"), dwMaxBufferWindow );
        m_txtStreamVideoMaxBufferWindow.SetWindowText( strMaxBufferWindow );
    }

    ((CStream*) m_pDisplayedProfileObject)->SetVideoMaxBufferWindow( dwMaxBufferWindow );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoMaxBitrate()
{
    CString strMaxBitrate;
    DWORD dwMaxBitrate;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ((CStream*)m_pDisplayedProfileObject)->GetStreamType() == ST_Video );
    assert( BST_CHECKED == m_chkStreamVideoIsVBR.GetCheck() );

    m_txtStreamVideoMaxBitrate.GetWindowText( strMaxBitrate );
    dwMaxBitrate = _ttol( strMaxBitrate );

    ((CStream*) m_pDisplayedProfileObject)->SetVideoMaxBitrate( dwMaxBitrate );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnChkSMPTE()
{
    BOOL fIsChecked;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );


    fIsChecked = ( m_chkSMPTE.GetCheck() == BST_CHECKED );

    ((CStream*) m_pDisplayedProfileObject)->SetIsSMPTE( fIsChecked );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnRBBandwidthTypeExclusive()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_BandwidthSharing == m_pDisplayedProfileObject->Type() );

    ((CBandwidthSharingObject*)m_pDisplayedProfileObject)->SetBandwidthSharingType( CLSID_WMBandwidthSharing_Exclusive );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnRBBandwidthTypePartial()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_BandwidthSharing == m_pDisplayedProfileObject->Type() );

    ((CBandwidthSharingObject*)m_pDisplayedProfileObject)->SetBandwidthSharingType( CLSID_WMBandwidthSharing_Partial );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTStreamVideoVBRQuality()
{
    CString strVBRQuality;
    DWORD dwVBRQuality;

    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );
    assert( ST_Video == ((CStream*) m_pDisplayedProfileObject)->GetStreamType() );
    assert( BST_CHECKED == m_chkStreamVideoIsVBR.GetCheck() );
    assert( VBR_QUALITYBASED == m_cbStreamVideoVBRMode.GetItemData( m_cbStreamVideoVBRMode.GetCurSel() ) );

    //
    // Set the VBR quality on the video stream
    //
    m_txtStreamVideoVBRQuality.GetWindowText( strVBRQuality );
    dwVBRQuality = _ttol( strVBRQuality );
    if ( dwVBRQuality > 100 )
    {
        dwVBRQuality = 100;
        strVBRQuality.Format( _T("%ld"), dwVBRQuality );
        m_txtStreamVideoVBRQuality.SetWindowText( strVBRQuality );
    }

    ((CStream*) m_pDisplayedProfileObject)->SetVideoVBRQuality( dwVBRQuality );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnKillfocusTXTBandwidthBufferWindow()
{
    CString strBufferWindow;
    DWORD dwBufferWindow;

    assert( m_pDisplayedProfileObject );
    assert( OT_BandwidthSharing == m_pDisplayedProfileObject->Type() );

    m_txtBandwidthBufferWindow.GetWindowText( strBufferWindow );
    dwBufferWindow = _ttol( strBufferWindow );

    ((CBandwidthSharingObject*) m_pDisplayedProfileObject)->SetBufferWindow( dwBufferWindow );
}


/*
** Start of helper member functions
*/

///////////////////////////////////////////////////////////////////////////////
BOOL CGenProfileDlg::InBitrateMutex( CStream *pStream )
{
    HRESULT hr;
    ULONG nDependantCount;
    ULONG nDependantIndex;
    CProfileObject* pDependant = NULL;

    assert( pStream );

    //
    // Get the number of dependants for the stream
    //
    nDependantCount = pStream->DependentCount();

    //
    // Loop through all dependants, looking for a bitrate mutex
    //
    for ( nDependantIndex = 0; nDependantIndex < nDependantCount; nDependantIndex++ )
    {
        SAFE_RELEASE( pDependant );
        hr = pStream->GetDependent( nDependantIndex, &pDependant );
        if ( FAILED( hr ) )
        {
            return TRUE; // Default to true, because it's safer
        }
        assert( pDependant );

        if ( !pDependant )
        {
            return TRUE; // Default to true, because it's safer
        }

        if ( ( OT_Mutex == pDependant->Type() ) && ( ((CMutex*) pDependant)->GetMutexType() == MT_Bitrate ) )
        {
            return TRUE;
        }
    }

    SAFE_RELEASE( pDependant );

    return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::DeleteProfileObjectByIndex( INT nCProfileObjectIndex )
{
    HRESULT hr = S_OK;
    CProfileObject* pProfileObjectToDelete;

    do
    {
        //
        // Delete the item's data
        //
        pProfileObjectToDelete = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nCProfileObjectIndex );
        if ( (CProfileObject*) -1 == pProfileObjectToDelete )
        {
            hr = E_UNEXPECTED;
            break;
        }

        assert( pProfileObjectToDelete );
        pProfileObjectToDelete->PrepareForDeletion();
        SAFE_RELEASE( pProfileObjectToDelete );

        //
        // Remove the string from the list
        //
        m_lstProfileObjects.DeleteString( nCProfileObjectIndex );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::DisplayProfileObject( CProfileObject *pProfileObject )
{
    HRESULT hr = S_OK;

    do
    {
        if ( NULL == pProfileObject )
        {
            ShowStreamWindowPlacement( WINSTREAMCONFIG_NONE );
            ShowWindowConfiguration( WINCONFIG_NONE );
        }

        if ( NULL != pProfileObject )
        {
            switch ( pProfileObject->Type() )
            {
            case OT_Stream:
                //
                // Show the help string
                //
                DisplayMessage( IDS_Stream_directions );

                hr = ShowStream( (CStream*) pProfileObject );
                break;

            case OT_Mutex:
                //
                // Show the help string
                //
                DisplayMessage( IDS_Mutex_directions );

                ShowStreamWindowPlacement( WINSTREAMCONFIG_NONE );
                hr = ShowMutex( (CMutex*) pProfileObject );
                break;

            case OT_StreamPrioritization:
                //
                // Show the help string
                //
                DisplayMessage( IDS_Prioritization_directions );

                ShowStreamWindowPlacement( WINSTREAMCONFIG_NONE );
                hr = ShowStreamPrioritizationObject( (CStreamPrioritizationObject*) pProfileObject );
                break;

            case OT_BandwidthSharing:
                //
                // Show the help string
                //
                DisplayMessage( IDS_Bandwidth_directions );

                ShowStreamWindowPlacement( WINSTREAMCONFIG_NONE );
                hr = ShowBandwidthSharingObject( (CBandwidthSharingObject*) pProfileObject );
                break;

            default:
                assert( !"Unknown object type" );
                hr = E_UNEXPECTED;
            }
            if ( FAILED( hr ) )
            {
                break;
            }
        }

        m_pDisplayedProfileObject = pProfileObject;
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    StreamType stStreamType;

    assert( pStream );

    do
    {
        //
        // Configure the dialog based on stream type
        //
        stStreamType = pStream->GetStreamType( );

        switch ( stStreamType )
        {
        case ST_Audio:
            hr = ShowAudioStream( pStream );
            break;

        case ST_Video:
            hr = ShowVideoStream( pStream );
            break;

        case ST_Script:
            hr = ShowScriptStream( pStream );
            break;

        case ST_Image:
            hr = ShowImageStream( pStream );
            break;

        case ST_Web:
            hr = ShowWebStream( pStream );
            break;

        case ST_File:
            hr = ShowFileStream( pStream );
            break;

        default:
            assert( !"Unknown stream type!" );
            hr = E_UNEXPECTED;
            break;
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Show all the stream controls
        //
        ShowWindowConfiguration( WINCONFIG_STREAM );
        ShowStreamWindowPlacement( (DWORD) stStreamType );

        //
        // Set the controls common to all streams
        //
        m_chkSMPTE.SetCheck( pStream->GetIsSMPTE() ? BST_CHECKED : BST_UNCHECKED );

        hr = PopulateLanguageCB();
        if ( FAILED( hr ) )
        {
            break;
        }

        SelectLanguage( pStream->GetLanguageLCID() );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowMutex( CMutex *pMutex )
{
    HRESULT hr = S_OK;
    MUTEX_TYPE mtMutexType;

    assert( pMutex );

    do
    {
        //
        // Add all the stream objects to the mutex list
        //
        hr = PopulateListbox( &m_lstMutexStreams, OT_Stream );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = SelectDependanciesInListMutex( &m_lstMutexStreams, pMutex );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Select the correct radio button for mutex type
        //
        mtMutexType = pMutex->GetMutexType( );
        switch ( mtMutexType )
        {
        case MT_Bitrate:
            m_rbMutexTypeBitrate.SetCheck( TRUE );
            m_rbMutexTypeLanguage.SetCheck( FALSE );
            m_rbMutexTypePresentation.SetCheck( FALSE );
            break;

        case MT_Language:
            m_rbMutexTypeBitrate.SetCheck( FALSE );
            m_rbMutexTypeLanguage.SetCheck( TRUE );
            m_rbMutexTypePresentation.SetCheck( FALSE );
            break;

        case MT_Presentation:
            m_rbMutexTypeBitrate.SetCheck( FALSE );
            m_rbMutexTypeLanguage.SetCheck( FALSE );
            m_rbMutexTypePresentation.SetCheck( TRUE );
            break;

        default:
            assert( !"Unknown mutex type!" );
            hr = E_UNEXPECTED;
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Show all the mutex controls
        //
        ShowWindowConfiguration( WINCONFIG_MUTEX );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::ShowDialogItem( DWORD dwResourceID, int nCmdShow )
{
    HWND hwndSubItem = NULL;

    hwndSubItem = ::GetDlgItem( this->m_hWnd, dwResourceID );
    if ( hwndSubItem )
    {
        ::ShowWindow( hwndSubItem, nCmdShow );
    }
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowBandwidthSharingObject( CBandwidthSharingObject *pBandwidthSharingObject )
{
    HRESULT hr = S_OK;
    CString strSharedBitrate;
    DWORD dwSharedBitrate;
    CString strBufferWindow;
    DWORD dwBufferWindow;

    if ( !pBandwidthSharingObject )
    {
        return E_INVALIDARG;
    }

    do
    {
        //
        // Add all the stream objects to the mutex list
        //
        hr = PopulateListbox( &m_lstSharingStreams, OT_Stream );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = SelectDependanciesInListBandwidth( &m_lstSharingStreams, pBandwidthSharingObject );
        if ( FAILED( hr ) )
        {
            break;
        }

        dwBufferWindow = pBandwidthSharingObject->GetBufferWindow( );
        strBufferWindow.Format( _T("%d"), dwBufferWindow );
        m_txtBandwidthBufferWindow.SetWindowText( strBufferWindow );

        dwSharedBitrate = pBandwidthSharingObject->GetSharedBitrate( );
        strSharedBitrate.Format( _T("%d"), dwSharedBitrate );
        m_txtSharedBitrate.SetWindowText( strSharedBitrate );

        if ( pBandwidthSharingObject->GetBandwidthSharingType() == CLSID_WMBandwidthSharing_Exclusive )
        {
            m_rbBandwidthSharingTypeExclusive.SetCheck( TRUE );
            m_rbBandwidthSharingTypePartial.SetCheck( FALSE );
        }
        else
        {
            m_rbBandwidthSharingTypeExclusive.SetCheck( FALSE );
            m_rbBandwidthSharingTypePartial.SetCheck( TRUE );
        }

        ShowWindowConfiguration( WINCONFIG_BANDWIDTHSHARING );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::ShowWindowConfiguration( DWORD dwConfigurationIndex )
{
    int nControlIndex;
    HWND hwndControl;

    assert( dwConfigurationIndex < NUM_CONFIGS );

    do
    {
        for ( nControlIndex = 0; nControlIndex < NUM_MOVABLE_CONTROLS; nControlIndex++ )
        {
            hwndControl = ::GetDlgItem( this->m_hWnd, WINDOW_POSITION[dwConfigurationIndex][nControlIndex].dwControl );
            if ( hwndControl )
            {
                ::MoveWindow(    hwndControl,
                                WINDOW_POSITION[dwConfigurationIndex][nControlIndex].nX,
                                WINDOW_POSITION[dwConfigurationIndex][nControlIndex].nY,
                                WINDOW_POSITION[dwConfigurationIndex][nControlIndex].nWidth,
                                WINDOW_POSITION[dwConfigurationIndex][nControlIndex].nHeight,
                                TRUE );
                ::ShowWindow( hwndControl, WINDOW_POSITION[dwConfigurationIndex][nControlIndex].fVisible ? SW_SHOW : SW_HIDE );
            }
        }
    }
    while ( FALSE );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::PopulateListbox( CListBox *pListBox, ProfileObjectType potRequestedType )
{
    HRESULT hr = S_OK;
    INT nProfileObjectIndex;
    INT nObjectIndex;
    CProfileObject* pCurrentObject;
    CString strObjectName;

    assert( pListBox );

    do
    {
        pListBox->ResetContent();
        for ( nProfileObjectIndex = 0; nProfileObjectIndex < m_lstProfileObjects.GetCount(); nProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nProfileObjectIndex );
            assert( pCurrentObject );
            if ( (CProfileObject*) -1 == pCurrentObject )
            {
                hr = E_UNEXPECTED;
                break;
            }

            if ( potRequestedType == pCurrentObject->Type() )
            {
                m_lstProfileObjects.GetText( nProfileObjectIndex, strObjectName );
                nObjectIndex = pListBox->AddString( strObjectName );
                pListBox->SetItemDataPtr( nObjectIndex, pCurrentObject );
            }
        }

    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SelectDependanciesInListMutex( CListBox *pListBox, CMutex *pMutex )
{
    INT nDependacyCount;
    INT nDependancyIndex;
    HRESULT hr = S_OK;
    CStream* pCurrentObject;
    CStream* pPotentialMatch;
    INT nMatchingItemIndex;
    INT nProfileObjectIndex;

    assert( pListBox );
    assert( pMutex );

    do
    {
        //
        // Select the list box elements that the profile object depends on
        //
        nDependacyCount = pMutex->StreamCount();
        for ( nDependancyIndex = 0; nDependancyIndex < nDependacyCount; nDependancyIndex++ )
        {
            hr = pMutex->GetStream( nDependancyIndex, &pCurrentObject );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pCurrentObject );

            //
            // Find the stream on the control that matches the stream in the mutex
            //
            nMatchingItemIndex = -1;
            for ( nProfileObjectIndex = 0; nProfileObjectIndex < pListBox->GetCount(); nProfileObjectIndex++ )
            {
                pPotentialMatch = (CStream*) pListBox->GetItemDataPtr( nProfileObjectIndex );
                assert( pPotentialMatch );
                if ( (CStream*) -1 == pPotentialMatch )
                {
                    hr = E_UNEXPECTED;
                    break;
                }

                if ( pPotentialMatch == pCurrentObject )
                {
                    nMatchingItemIndex = nProfileObjectIndex;
                    break;
                }
            }
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( -1 != nMatchingItemIndex )
            {
                if ( LB_ERR == pListBox->SetSel( nMatchingItemIndex, TRUE ) )
                {
                    hr = E_UNEXPECTED;
                    break;
                }
            }
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SelectDependanciesInListBandwidth( CListBox *pListBox, CBandwidthSharingObject* pBandwidthSharingObject )
{
    INT nDependacyCount;
    INT nDependancyIndex;
    HRESULT hr = S_OK;
    CStream* pCurrentObject;
    CStream* pPotentialMatch;
    INT nMatchingItemIndex;
    INT nProfileObjectIndex;

    assert( pListBox );
    assert( pBandwidthSharingObject );

    do
    {
        //
        // Select the list box elements that the profile object depends on
        //
        nDependacyCount = pBandwidthSharingObject->StreamCount();
        for ( nDependancyIndex = 0; nDependancyIndex < nDependacyCount; nDependancyIndex++ )
        {
            hr = pBandwidthSharingObject->GetStream( nDependancyIndex, &pCurrentObject );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pCurrentObject );

            //
            // Find the stream on the control that matches the stream in the mutex
            //
            nMatchingItemIndex = -1;
            for ( nProfileObjectIndex = 0; nProfileObjectIndex < pListBox->GetCount(); nProfileObjectIndex++ )
            {
                pPotentialMatch = (CStream*) pListBox->GetItemDataPtr( nProfileObjectIndex );
                assert( pPotentialMatch );
                if ( (CStream*) -1 == pPotentialMatch )
                {
                    hr = E_UNEXPECTED;
                    break;
                }

                if ( pPotentialMatch == pCurrentObject )
                {
                    nMatchingItemIndex = nProfileObjectIndex;
                    break;
                }
            }
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( -1 != nMatchingItemIndex )
            {
                if ( LB_ERR == pListBox->SetSel( nMatchingItemIndex, TRUE ) )
                {
                    hr = E_UNEXPECTED;
                    break;
                }
            }
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowStreamPrioritizationObject( CStreamPrioritizationObject *pStreamPrioritizationObject )
{
    HRESULT hr = S_OK;
    INT nStreamIndex;
    CStream* pStream;
    BOOL fStreamIsMandatory;

    if ( !pStreamPrioritizationObject )
    {
        return E_INVALIDARG;
    }

    do
    {
        hr = RefreshStreamPriorityList( pStreamPrioritizationObject );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Show mandatory streams, then select the ones that are mandatory
        //
        hr = PopulateListbox( &m_lstMandatoryStreams, OT_Stream );
        if ( FAILED( hr ) )
        {
            break;
        }

        for ( nStreamIndex = 0; nStreamIndex < m_lstMandatoryStreams.GetCount(); nStreamIndex++ )
        {
            pStream = (CStream*) m_lstMandatoryStreams.GetItemDataPtr( nStreamIndex );
            assert( pStream );
            if ( (CStream*) -1 == pStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            hr = pStreamPrioritizationObject->GetStreamMandatory( pStream, &fStreamIsMandatory );
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( fStreamIsMandatory )
            {
                if ( LB_ERR == m_lstMandatoryStreams.SetSel( nStreamIndex, TRUE ) )
                {
                    hr = E_UNEXPECTED;
                    break;
                }
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        ShowWindowConfiguration( WINCONFIG_STREAMPRIORITIZATION );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::RefreshStreamPriorityList( CStreamPrioritizationObject *pPriorityObject )
{
    HRESULT hr = S_OK;
    CStream* pStream = NULL;
    INT nStreamIndex;
    INT nStreamCount;
    CString strStreamName;
    INT nNewItemIndex;

    do
    {
        //
        // Add all the stream objects to the mutex list
        //
        nStreamCount = pPriorityObject->StreamCount();
        m_lstPrioritizationStreams.ResetContent();
        for ( nStreamIndex = 0; nStreamIndex < nStreamCount; nStreamIndex++ )
        {
            SAFE_RELEASE( pStream );
            hr = pPriorityObject->GetStreamWithPriority( nStreamIndex, &pStream );

            if ( FAILED( hr ) )
            {
                break;
            }

            assert( pStream );

            if( NULL == pStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            strStreamName = pStream->GetName();
            nNewItemIndex = m_lstPrioritizationStreams.AddString( strStreamName );
            if ( LB_ERR != nNewItemIndex )
            {
                m_lstPrioritizationStreams.SetItemDataPtr( nNewItemIndex, pStream );
            }
            else
            {
                hr = E_UNEXPECTED;
                break;
            }
        }

    }
    while ( FALSE );

    SAFE_RELEASE( pStream );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
bool CGenProfileDlg::StreamPrioritizationObjectExists()
{
    INT nProfileObjectIndex;
    CProfileObject* pCurrentObject;
    CString strObjectName;

    do
    {
        for ( nProfileObjectIndex = 0; nProfileObjectIndex < m_lstProfileObjects.GetCount(); nProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nProfileObjectIndex );
            assert( pCurrentObject );
            if ( (CProfileObject*) -1 == pCurrentObject )
            {
                DisplayMessage( IDS_Fatal_error );
                break;
            }

            if ( OT_StreamPrioritization == pCurrentObject->Type() )
            {
                return true;
            }
        }
    }
    while ( FALSE );

    return false;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowStreamWindowPlacement( DWORD dwStreamType )
{
    int nControlIndex;
    HWND hwndControl;

    assert( dwStreamType < NUM_STREAMTYPES );

    do
    {
        for ( nControlIndex = 0; nControlIndex < NUM_STREAM_CONTROLS; nControlIndex++ )
        {
            hwndControl = ::GetDlgItem( this->m_hWnd, STREAM_WINDOW_POSITION[dwStreamType][nControlIndex].dwControl );
            if ( hwndControl )
            {
                ::MoveWindow(    hwndControl,
                                STREAM_WINDOW_POSITION[dwStreamType][nControlIndex].nX,
                                STREAM_WINDOW_POSITION[dwStreamType][nControlIndex].nY,
                                STREAM_WINDOW_POSITION[dwStreamType][nControlIndex].nWidth,
                                STREAM_WINDOW_POSITION[dwStreamType][nControlIndex].nHeight,
                                TRUE );
                ::ShowWindow( hwndControl, STREAM_WINDOW_POSITION[dwStreamType][nControlIndex].fVisible ? SW_SHOW : SW_HIDE );
            }
        }
    }
    while ( FALSE );

    return S_OK;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowAudioStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nStreamTypeIndex;
    DWORD dwCodecCount;
    DWORD dwCodecIndex;
    LPWSTR wszCodecName = NULL;
    CString strCodecName;
    DWORD dwCodecNameLength;
    INT nCodecStringIndex;
    DWORD dwStreamFormatIndex;
    CString strBufferWindow;
    CString strStreamType;
    BOOL fIsUncompressedStream;

    assert( pStream );
    assert( ST_Audio == pStream->GetStreamType() );
    assert( m_pCodecInfo );
    assert( m_bIsAudioCodec );

    do
    {
        //
        // Select "audio" in the stream type combo box
        //
        strStreamType.LoadString( IDS_Audio );
        nStreamTypeIndex = m_cbStreamType.FindStringExact( -1, strStreamType );
        assert( CB_ERR != nStreamTypeIndex );
        if ( CB_ERR == nStreamTypeIndex )
        {
            hr = E_UNEXPECTED;
            break;
        }
        m_cbStreamType.SetCurSel( nStreamTypeIndex );

        //
        // If the stream is uncompressed, then show it appropriately
        //
        m_chkStreamIsUncompressed.EnableWindow( TRUE );
        fIsUncompressedStream = pStream->GetIsUncompressed();
        m_chkStreamIsUncompressed.SetCheck( pStream->GetIsUncompressed() ? BST_CHECKED : BST_UNCHECKED );

        if ( !fIsUncompressedStream )
        {
            //
            // Enable controls that are relevant to compressed streams
            //
            m_cbStreamCodec.EnableWindow( TRUE );
            m_txtStreamBitrate.EnableWindow( TRUE );
            m_txtStreamBufferWindow.EnableWindow( TRUE );

            //
            // Get number of audio codecs installed on machine
            //
            hr = m_pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Audio, &dwCodecCount );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( dwCodecCount > 0 );

            //
            // Populate the codec type with the installed audio codecs
            //
            m_cbStreamCodec.ResetContent();
            for ( dwCodecIndex = 0; dwCodecIndex < dwCodecCount; dwCodecIndex++ )
            {
                //
                // Get the name for the codec
                //
                dwCodecNameLength = 0;
                hr = m_pCodecInfo->GetCodecName( WMMEDIATYPE_Audio, dwCodecIndex, NULL, &dwCodecNameLength );
                if ( FAILED( hr ) )
                {
                    break;
                }

                SAFE_ARRAYDELETE( wszCodecName );
                wszCodecName = new WCHAR[ dwCodecNameLength + 1 ];
                if ( !wszCodecName )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = m_pCodecInfo->GetCodecName( WMMEDIATYPE_Audio, dwCodecIndex, wszCodecName, &dwCodecNameLength );
                if ( FAILED( hr ) )
                {
                    break;
                }
                wszCodecName[ dwCodecNameLength ] = L'\0'; // terminate the codec name, just in case

                strCodecName.Format( _T( "%ls" ), wszCodecName );

                nCodecStringIndex = m_cbStreamCodec.AddString( strCodecName );
                if ( CB_ERR == nCodecStringIndex )
                {
                    hr = E_UNEXPECTED;
                    break;
                }
                m_cbStreamCodec.SetItemData( nCodecStringIndex, dwCodecIndex );
            }
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Select the appropriate audio format in the format type combo box
            //
            nCodecStringIndex = pStream->GetStreamCodecIndex();
            assert( (DWORD) nCodecStringIndex < dwCodecCount );
            m_cbStreamCodec.SetCurSel( nCodecStringIndex );

            //
            // Populate the format combo box based on the selected codec
            //
            dwCodecIndex = ( DWORD )( m_cbStreamCodec.GetItemData( nCodecStringIndex ) );
            assert( CB_ERR != dwCodecIndex );
            if ( CB_ERR == dwCodecIndex )
            {
                dwCodecIndex = 0;
                pStream->SetStreamCodecIndex( dwCodecIndex );
            }

            hr = PopulateAudioFormatCB( dwCodecIndex );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Select the format matching the stream
            //
            dwStreamFormatIndex = pStream->GetStreamFormatStringIndex();
            assert( dwStreamFormatIndex < (DWORD) m_cbStreamFormat.GetCount() );
            m_cbStreamFormat.SetCurSel( dwStreamFormatIndex );
        }
        else
        {
            //
            // Disable controls that are irrelevant to uncompressed streams
            //
            m_cbStreamCodec.EnableWindow( FALSE );
            m_txtStreamBitrate.EnableWindow( FALSE );
            m_txtStreamBufferWindow.EnableWindow( FALSE );

            hr = PopulateWaveFormatCB();
            if ( FAILED( hr ) )
            {
                break;
            }

            DWORD dwWaveFormatIndex = pStream->GetWaveFormatStringIndex();
            assert( dwWaveFormatIndex < (DWORD) m_cbStreamFormat.GetCount() );
            m_cbStreamFormat.SetCurSel( dwWaveFormatIndex );
            pStream->SetWaveFormatIndex( ( DWORD )( m_cbStreamFormat.GetItemData( dwWaveFormatIndex ) ) );
        }

        //
        // Make all the controls reflect the values in pStream
        //
        strBufferWindow.Format( _T("%ld"), pStream->GetStreamBufferWindow() );
        m_txtStreamBufferWindow.SetWindowText( strBufferWindow );
    }
    while ( FALSE );

    SAFE_ARRAYDELETE( wszCodecName );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::PopulatePixleFormatCB()
{
    HRESULT hr = S_OK;
    DWORD dwFormatCount;

    if ( m_cbPixelFormat.GetCount() > 0 )
    {
        // Pixel format list has been initialized.
        return hr;
    }

    hr = GetUncompressedPixelFormatCount( &dwFormatCount );
    if ( FAILED( hr ) )
    {
        return hr;
    }

    for ( DWORD i = 0; i < dwFormatCount; i ++ )
    {
        GUID guidFormat;
        DWORD dwFourCC;
        WORD wBitsPerPixel;

        hr = GetUncompressedPixelFormat( i, &guidFormat, &dwFourCC, &wBitsPerPixel );
        if ( FAILED( hr ) )
        {
            break;
        }

        if ( wBitsPerPixel > 8 )
        {
            TCHAR   tszName[16];

            if ( dwFourCC == BI_RGB )
            {
                (void)StringCchPrintf( tszName, ARRAYSIZE(tszName), _T("RGB%d"), (DWORD)wBitsPerPixel );
            }
            else if ( dwFourCC == BI_BITFIELDS )
            {
                assert( FALSE );
            }
            else
            {
                (void)StringCchPrintf( tszName, ARRAYSIZE(tszName), _T("%c%c%c%c"),
                    (char)(dwFourCC & 0xFF),
                    (char)( (dwFourCC>>8) & 0xFF),
                    (char)( (dwFourCC>>16) & 0xFF),
                    (char)( (dwFourCC>>24) & 0xFF) );
            }

            int nIndex = m_cbPixelFormat.AddString( tszName );
            m_cbPixelFormat.SetItemData( nIndex, i );
        }
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::PopulateAudioFormatCB( DWORD dwCodecIndex )
{
    HRESULT hr = S_OK;
    CString strFormatName;

    assert( m_bIsAudioCodec );

    do
    {
        m_cbStreamFormat.ResetContent();

        hr = AddAudioFormatsToCB( FALSE, 0, dwCodecIndex );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = AddAudioFormatsToCB( TRUE, 1, dwCodecIndex );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = AddAudioFormatsToCB( TRUE, 2, dwCodecIndex );
        if ( FAILED( hr ) )
        {
            break;
        }

        assert( m_cbStreamFormat.GetCount() > 0 );
    }
    while ( FALSE );

    return hr;
}

HRESULT CGenProfileDlg::PopulateWaveFormatCB()
{
    HRESULT hr = S_OK;
    DWORD dwFormatCount;

    m_cbStreamFormat.ResetContent();

    hr = GetUncompressedWaveFormatCount( &dwFormatCount );
    if ( FAILED( hr ) )
    {
        return hr;
    }

    for ( DWORD i = 0; i < dwFormatCount; i ++ )
    {
        DWORD dwSamplesPerSecond;
        WORD wNumChannels;
        WORD wBitsPerSample;

        hr = GetUncompressedWaveFormat( i,
                                        &dwSamplesPerSecond,
                                        &wNumChannels,
                                        &wBitsPerSample );
        if ( FAILED( hr ) )
        {
            break;
        }

        CString csFormat;

        if ( wNumChannels == 1 )
        {
            csFormat.Format( _T("%d Hz, %d Bits, Mono"),
                dwSamplesPerSecond, (DWORD)wBitsPerSample );
        }
        else if ( wNumChannels == 2 )
        {
            csFormat.Format( _T("%d Hz, %d Bits, Stereo"),
                dwSamplesPerSecond, (DWORD)wBitsPerSample );
        }
        else
        {
            csFormat.Format( _T("%d Hz, %d Bits, %d Channels"),
                dwSamplesPerSecond, (DWORD)wBitsPerSample, (DWORD)wNumChannels );
        }

        int nIndex = m_cbStreamFormat.AddString( csFormat );
        m_cbStreamFormat.SetItemData( nIndex, i );
    }

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::AddAudioFormatsToCB( BOOL fIsVBR, DWORD dwNumVBRPasses, DWORD dwCodecIndex )
{
    HRESULT hr = S_OK;
    DWORD dwFormatCount;
    DWORD dwFormatIndex;
    CString strFormatName;
    DWORD nFormatStringIndex;
    LPWSTR wszFormatName = NULL;
    DWORD dwFormatNameLength;
    DWORD dwVBROffset;

    assert( m_pCodecInfo );
    assert( m_bIsAudioCodec );

    do
    {
        //
        // Turn on the VBR formats, and get the format count
        //
        dwFormatCount = GetNumberOfFormatsSupported( WMMEDIATYPE_Audio, dwCodecIndex, fIsVBR, dwNumVBRPasses );

        //
        // Loop through all formats using given VBR settings, and add them to the combo box
        //
        for ( dwFormatIndex = 0; dwFormatIndex < dwFormatCount; dwFormatIndex++ )
        {
            //
            // Calculate a dwVBROffset, which is used to keep track if the format is VBR and how many passes
            //
            dwVBROffset = fIsVBR ? VBR_OFFSET * dwNumVBRPasses : 0;

            //
            // Get the name for the given format from the codec
            //
            hr = m_pCodecInfo->GetCodecFormatDesc( WMMEDIATYPE_Audio, dwCodecIndex, dwFormatIndex, NULL, NULL, &dwFormatNameLength );
            if ( FAILED( hr ) )
            {
                break;
            }

            SAFE_ARRAYDELETE( wszFormatName );

            hr = ULongAdd( dwFormatNameLength, 1, &dwFormatNameLength );
            if ( FAILED( hr ) )
            {
                break;
            }
            wszFormatName = new WCHAR[ dwFormatNameLength ];
            if ( !wszFormatName )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = m_pCodecInfo->GetCodecFormatDesc( WMMEDIATYPE_Audio, dwCodecIndex, dwFormatIndex, NULL, wszFormatName, &dwFormatNameLength );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Add the format to the combo box
            //
            strFormatName.Format( _T("%ls"), wszFormatName );

            nFormatStringIndex = m_cbStreamFormat.AddString( strFormatName );
            if ( CB_ERR == nFormatStringIndex )
            {
                continue;
            }
            m_cbStreamFormat.SetItemData( nFormatStringIndex, dwFormatIndex + dwVBROffset );
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_ARRAYDELETE( wszFormatName );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetDefaultsForAudioStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    DWORD dwCodecIndex;
    DWORD dwCodecCount;
    BOOL fFormatFound;
    CString strMessage;

    assert( pStream );
    assert( m_bIsAudioCodec );

    do
    {
        //
        // Set type to audio
        //
        pStream->SetStreamType( ST_Audio );

        hr = m_pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Audio, &dwCodecCount );
        if ( FAILED( hr ) )
        {
            DisplayMessage( IDS_Fatal_error );
            break;
        }
        assert( dwCodecCount > 0 );

        pStream->SetStreamFormatStringIndex( 0 );

        fFormatFound = FALSE;
        for ( dwCodecIndex = 0; dwCodecIndex < dwCodecCount; dwCodecIndex++ )
        {
            //
            // Set the codec
            //
            pStream->SetStreamCodecIndex( dwCodecIndex );

            //
            // Set format to an existing value
            //
            if ( GetNumberOfFormatsSupported( WMMEDIATYPE_Audio, dwCodecIndex, FALSE, 0 ) > 0 )
            {
                pStream->SetStreamFormatIndex( 0 );
                fFormatFound = TRUE;
                break;
            }
            else if ( GetNumberOfFormatsSupported( WMMEDIATYPE_Audio, dwCodecIndex, TRUE, 1 ) > 0 )
            {
                pStream->SetStreamFormatIndex( VBR_OFFSET );
                fFormatFound = TRUE;
                break;
            }
            else if ( GetNumberOfFormatsSupported( WMMEDIATYPE_Audio, dwCodecIndex, TRUE, 2 ) > 0 )
            {
                pStream->SetStreamFormatIndex( 2 * VBR_OFFSET );
                fFormatFound = TRUE;
                break;
            }
        }

        if ( !fFormatFound )
        {
            hr = E_FAIL;
            DisplayMessage( IDS_No_audio_formats_error );
            break;
        }

        //
        // Set other values to defaults
        //
        pStream->SetStreamBufferWindow( 3000 );
        pStream->SetIsSMPTE( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetDefaultsForVideoStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    DWORD dwCodecIndex;
    DWORD dwCodecCount;
    BOOL fFormatFound;
    CString strMessage;

    assert( pStream );
    assert( m_pCodecInfo );
    assert( m_bIsVideoCodec );

    do
    {
        //
        // Set type to video
        //
        pStream->SetStreamType( ST_Video );

        //
        // Get the number of video codecs
        //
        hr = m_pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Video, &dwCodecCount );
        if ( FAILED( hr ) )
        {
            DisplayMessage( IDS_Fatal_error );
            break;
        }
        assert( dwCodecCount > 0 );

        fFormatFound = FALSE;
        for ( dwCodecIndex = 0; dwCodecIndex < dwCodecCount; dwCodecIndex++ )
        {
            //
            // Set the codec
            //
            pStream->SetStreamCodecIndex( dwCodecIndex );

            //
            // Set format to an existing value
            //
            if ( GetNumberOfFormatsSupported( WMMEDIATYPE_Video, dwCodecIndex, FALSE, 0 ) > 0 )
            {
                pStream->SetVideoIsVBR( FALSE );
                pStream->SetVideoVBRMode( VBR_QUALITYBASED );
                fFormatFound = TRUE;
                break;
            }
            else if ( GetNumberOfFormatsSupported( WMMEDIATYPE_Video, dwCodecIndex, TRUE, 1 ) > 0 )
            {
                pStream->SetVideoIsVBR( TRUE );
                pStream->SetVideoVBRMode( VBR_QUALITYBASED );
                fFormatFound = TRUE;
                break;
            }
            else if ( GetNumberOfFormatsSupported( WMMEDIATYPE_Video, dwCodecIndex, TRUE, 2 ) > 0 )
            {
                pStream->SetVideoIsVBR( TRUE );
                pStream->SetVideoVBRMode( VBR_UNCONSTRAINED );
                fFormatFound = TRUE;
                break;
            }
        }

        if ( !fFormatFound )
        {
            hr = E_FAIL;
            DisplayMessage( IDS_No_video_formats_error );
            break;
        }

        //
        // Set other values to defaults
        //
        pStream->SetStreamBitrate( 100000 );
        pStream->SetStreamBufferWindow( 3000 );
        pStream->SetVideoWidth( 320 );
        pStream->SetVideoHeight( 240 );
        pStream->SetVideoFPS( 30 );
        pStream->SetVideoSecondsPerKeyframe( 8 );
        pStream->SetVideoQuality( 100 );
        pStream->SetVideoMaxBitrate( 1000000 );
        pStream->SetVideoMaxBufferWindow( 0 );
        pStream->SetIsSMPTE( FALSE );
        pStream->SetVideoVBRQuality( 0 );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowVideoStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nStreamTypeIndex;
    DWORD dwCodecCount;
    DWORD dwCodecIndex;
    LPWSTR wszCodecName = NULL;
    CString strCodecName;
    DWORD dwCodecNameLength;
    INT nCodecStringIndex;
    CString strBufferWindow;
    CString strVideoValue;
    CString strStreamType;
    BOOL fIsUncompressedStream;

    assert( pStream );
    assert( ST_Video == pStream->GetStreamType() );
    assert( m_pCodecInfo );
    assert( m_bIsVideoCodec );


    do
    {
        //
        // Select "video" in the stream type combo box
        //
        strStreamType.LoadString( IDS_Video );
        nStreamTypeIndex = m_cbStreamType.FindStringExact( -1, strStreamType );
        assert( CB_ERR != nStreamTypeIndex );
        m_cbStreamType.SetCurSel( nStreamTypeIndex );

        //
        // If the stream is uncompressed, then show it appropriately
        //
        m_chkStreamIsUncompressed.EnableWindow( TRUE );

        fIsUncompressedStream = pStream->GetIsUncompressed();
        m_chkStreamIsUncompressed.SetCheck( fIsUncompressedStream ? BST_CHECKED : BST_UNCHECKED );

        if ( !fIsUncompressedStream )
        {
            //
            // Enable controls that are relevant to compressed streams
            //
            m_cbStreamCodec.EnableWindow( TRUE );
            m_txtStreamBitrate.EnableWindow( TRUE );
            m_txtStreamVideoQuality.EnableWindow( TRUE );
            m_txtStreamVideoSecondsPerKeyframe.EnableWindow( TRUE );
            m_txtStreamBufferWindow.EnableWindow( TRUE );

            //
            // Get the number of video codecs installed on machine
            //
            hr = m_pCodecInfo->GetCodecInfoCount( WMMEDIATYPE_Video, &dwCodecCount );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( dwCodecCount > 0 );

            //
            // Populate the codec type with the installed video codecs
            //
            m_cbStreamCodec.ResetContent();
            for ( dwCodecIndex = 0; dwCodecIndex < dwCodecCount; dwCodecIndex++ )
            {
                dwCodecNameLength = 4;
                hr = m_pCodecInfo->GetCodecName( WMMEDIATYPE_Video, dwCodecIndex, NULL, &dwCodecNameLength );
                if ( FAILED( hr ) )
                {
                    break;
                }

                SAFE_ARRAYDELETE( wszCodecName );
                wszCodecName = new WCHAR[ dwCodecNameLength + 1 ];
                if ( !wszCodecName )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = m_pCodecInfo->GetCodecName( WMMEDIATYPE_Video, dwCodecIndex, wszCodecName, &dwCodecNameLength );
                if ( FAILED( hr ) )
                {
                    break;
                }
                wszCodecName[ dwCodecNameLength ] = L'\0'; // terminate the codec name, just in case

                strCodecName.Format( _T( "%ls" ), wszCodecName );

                nCodecStringIndex = m_cbStreamCodec.AddString( strCodecName );
                if ( CB_ERR == nCodecStringIndex )
                {
                    continue;
                }
                m_cbStreamCodec.SetItemData( nCodecStringIndex, dwCodecIndex );
            }
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Set the values of the controls that are relvant to compressd video
            //
            dwCodecIndex = pStream->GetStreamCodecIndex();
            m_cbStreamCodec.SetCurSel( dwCodecIndex );

            hr = DisplayVBRControlsForCodec( dwCodecIndex, pStream );
            if ( FAILED( hr ) )
            {
                break;
            }

            strVideoValue.Format( _T("%ld"), pStream->GetStreamBitrate() );
            m_txtStreamBitrate.SetWindowText( strVideoValue );

            strVideoValue.Format( _T("%ld"), pStream->GetVideoQuality() );
            m_txtStreamVideoQuality.SetWindowText( strVideoValue );
            m_cbPixelFormat.EnableWindow( FALSE );
        }
        else
        {
            //
            // Disable controls that are irrelevant to uncompressed streams
            //
            m_cbStreamCodec.EnableWindow( FALSE );
            m_txtStreamBitrate.EnableWindow( FALSE );
            m_txtStreamVideoQuality.EnableWindow( FALSE );
            m_txtStreamVideoSecondsPerKeyframe.EnableWindow( FALSE );
            m_txtStreamBufferWindow.EnableWindow( FALSE );
            DisableVideoVBRControls();

            hr = PopulatePixleFormatCB();
            if ( FAILED( hr ) )
            {
                break;
            }

            m_cbPixelFormat.EnableWindow( TRUE );
            DWORD dwPixelFormatStringIndex = pStream->GetPixelFormatStringIndex();
            m_cbPixelFormat.SetCurSel( dwPixelFormatStringIndex );
            pStream->SetPixelFormatIndex( ( DWORD )( m_cbPixelFormat.GetItemData( dwPixelFormatStringIndex ) ) );
        }

        //
        // Make all the controls reflect the values in pStream
        //
        strBufferWindow.Format( _T("%ld"), pStream->GetStreamBufferWindow() );
        m_txtStreamBufferWindow.SetWindowText( strBufferWindow );

        strVideoValue.Format( _T("%ld"), pStream->GetVideoWidth() );
        m_txtStreamVideoWidth.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetVideoHeight() );
        m_txtStreamVideoHeight.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetVideoFPS() );
        m_txtStreamVideoFPS.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetVideoSecondsPerKeyframe() );
        m_txtStreamVideoSecondsPerKeyframe.SetWindowText( strVideoValue );

    }
    while ( FALSE );

    SAFE_ARRAYDELETE( wszCodecName );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::DisplayMessage( UINT nStringResourceIndex )
{
    CString strMessage;

    if ( !strMessage.LoadString( nStringResourceIndex ) )
    {
        if ( !strMessage.LoadString( IDS_String_load_error ) )
        {
            MessageBox( _T("Error loading string resources!"), _T("An error occured!") );
        }
    }

    m_txtHelp.SetWindowText( strMessage );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetDefaultsForScriptStream( CStream *pStream )
{
    HRESULT hr = S_OK;

    assert( pStream );

    do
    {
        //
        // Set stream type
        //
        pStream->SetStreamType( ST_Script );

        //
        // Set other values to defaults
        //
        pStream->SetStreamBitrate( 1000 );
        pStream->SetStreamBufferWindow( 3000 );
        pStream->SetIsSMPTE( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowScriptStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nStreamTypeIndex;
    CString strVideoValue;
    CString strStreamType;

    assert( pStream );
    assert( ST_Script == pStream->GetStreamType() );

    do
    {
        //
        // Select "script" in the stream type combo box
        //
        strStreamType.LoadString( IDS_Script );
        nStreamTypeIndex = m_cbStreamType.FindStringExact( -1, strStreamType );
        assert( CB_ERR != nStreamTypeIndex );
        m_cbStreamType.SetCurSel( nStreamTypeIndex );

        //
        // Make all the controls reflect the values in pStream
        //
        strVideoValue.Format( _T("%ld"), pStream->GetStreamBitrate() );
        m_txtStreamBitrate.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetStreamBufferWindow() );
        m_txtStreamBufferWindow.SetWindowText( strVideoValue );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetDefaultsForImageStream( CStream *pStream )
{
    HRESULT hr = S_OK;

    assert( pStream );

    do
    {
        //
        // Set stream type
        //
        pStream->SetStreamType( ST_Image );

        //
        // Set other values to defaults
        //
        pStream->SetStreamBitrate( 1000 );
        pStream->SetStreamBufferWindow( 3000 );
        pStream->SetVideoWidth( 320 );
        pStream->SetVideoHeight( 200 );
        pStream->SetIsSMPTE( FALSE );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowImageStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nStreamTypeIndex;
    CString strVideoValue;
    CString strStreamType;

    assert( pStream );
    assert( ST_Image == pStream->GetStreamType() );

    do
    {
        //
        // Select "Image" in the stream type combo box
        //
        strStreamType.LoadString( IDS_Image );
        nStreamTypeIndex = m_cbStreamType.FindStringExact( -1, strStreamType );
        assert( CB_ERR != nStreamTypeIndex );
        m_cbStreamType.SetCurSel( nStreamTypeIndex );

        //
        // Make all the controls reflect the values in pStream
        //
        strVideoValue.Format( _T("%ld"), pStream->GetStreamBitrate() );
        m_txtStreamBitrate.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetStreamBufferWindow() );
        m_txtStreamBufferWindow.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetVideoWidth() );
        m_txtStreamVideoWidth.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetVideoHeight() );
        m_txtStreamVideoHeight.SetWindowText( strVideoValue );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetDefaultsForWebStream( CStream *pStream )
{
    HRESULT hr = S_OK;

    assert( pStream );

    do
    {
        //
        // Set stream type
        //
        pStream->SetStreamType( ST_Web );

        //
        // Set other values to defaults
        //
        pStream->SetStreamBitrate( 50000 );
        pStream->SetStreamBufferWindow( 3000 );
        pStream->SetIsSMPTE( FALSE );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowWebStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nStreamTypeIndex;
    CString strVideoValue;
    CString strStreamType;

    assert( pStream );
    assert( ST_Web == pStream->GetStreamType() );

    do
    {
        //
        // Select "web" in the stream type combo box
        //
        strStreamType.LoadString( IDS_Web );
        nStreamTypeIndex = m_cbStreamType.FindStringExact( -1, strStreamType );
        assert( CB_ERR != nStreamTypeIndex );
        m_cbStreamType.SetCurSel( nStreamTypeIndex );

        //
        // Make all the controls reflect the values in pStream
        //
        strVideoValue.Format( _T("%ld"), pStream->GetStreamBitrate() );
        m_txtStreamBitrate.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetStreamBufferWindow() );
        m_txtStreamBufferWindow.SetWindowText( strVideoValue );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetDefaultsForFileStream( CStream *pStream )
{
    HRESULT hr = S_OK;

    assert( pStream );

    do
    {
        //
        // Set stream type
        //
        pStream->SetStreamType( ST_File );

        //
        // Set other values to defaults
        //
        pStream->SetStreamBitrate( 50000 );
        pStream->SetStreamBufferWindow( 3000 );
        pStream->SetIsSMPTE( FALSE );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::ShowFileStream( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nStreamTypeIndex;
    CString strVideoValue;
    CString strStreamType;

    assert( pStream );
    assert( ST_File == pStream->GetStreamType() );

    do
    {
        //
        // Select "File" in the stream type combo box
        //
        strStreamType.LoadString( IDS_File );
        nStreamTypeIndex = m_cbStreamType.FindStringExact( -1, strStreamType );
        assert( CB_ERR != nStreamTypeIndex );
        m_cbStreamType.SetCurSel( nStreamTypeIndex );

        //
        // Make all the controls reflect the values in pStream
        //
        strVideoValue.Format( _T("%ld"), pStream->GetStreamBitrate() );
        m_txtStreamBitrate.SetWindowText( strVideoValue );

        strVideoValue.Format( _T("%ld"), pStream->GetStreamBufferWindow() );
        m_txtStreamBufferWindow.SetWindowText( strVideoValue );

        m_chkStreamIsUncompressed.SetCheck( BST_CHECKED );
        m_chkStreamIsUncompressed.EnableWindow( FALSE );
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::CodecSupportsVBRSetting( GUID guidType, DWORD dwCodecIndex, DWORD dwPasses, BOOL* pbIsSupported )
{
    HRESULT hr = S_OK;

    assert( pbIsSupported );
    assert( m_pCodecInfo );

    do
    {
        *pbIsSupported = FALSE; // default to "no"

        //
        // Try setting the requested settings
        //
        hr = SetCodecVBRSettings( m_pCodecInfo, guidType, dwCodecIndex, TRUE, dwPasses );
        if ( FAILED( hr ) )
        {
            hr = S_OK;
            break;
        }

        //
        // If it worked, then the codec should support it
        //
        *pbIsSupported = TRUE;
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::RemoveStreamFromAllMutexes( CStream *pStream )
{
    HRESULT hr = S_OK;
    INT nObjectCount;
    INT nObjectIndex;
    CProfileObject* pCurrentObject;

    assert( pStream );

    do
    {
        nObjectCount = m_lstProfileObjects.GetCount();
        if ( LB_ERR == nObjectCount )
        {
            hr = E_UNEXPECTED;
            break;
        }

        //
        // Loop through all the objects in the object list and remove the stream from each mutex
        //
        for ( nObjectIndex = 0; nObjectIndex < nObjectCount; nObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( nObjectIndex );
            assert( pCurrentObject );
            if ( (CProfileObject*) -1 == pCurrentObject )
            {
                hr = E_UNEXPECTED;
                break;
            }

            if ( OT_Mutex == pCurrentObject->Type() )
            {
                hr = ((CMutex*) pCurrentObject)->RemoveStream( pStream );
                if ( FAILED( hr ) )
                {
                    break;
                }
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::DisplayVBRControlsForCodec( DWORD dwCodecIndex, CStream* pStream )
{
    HRESULT hr = S_OK;
    BOOL fSupportsVBR;
    BOOL fSupports2Pass;
    BOOL fIsVBR;
    INT nVBRModeIndex;
    DWORD dwMaxBitrate;
    DWORD dwMaxBufferWindow;
    CString strValue;
    INT nNewStringIndex;
    CString strVBRType;

    assert( pStream );

    do
    {
        hr = CodecSupportsVBRSetting( WMMEDIATYPE_Video, dwCodecIndex, 1, &fSupportsVBR );
        if ( FAILED( hr ) )
        {
            break;
        }

        if ( fSupportsVBR )
        {
            //
            // Check for 2 pass VBR
            //
            hr = CodecSupportsVBRSetting( WMMEDIATYPE_Video, dwCodecIndex, 2, &fSupports2Pass );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Setup the combo box to display the correct results
            //
            m_cbStreamVideoVBRMode.ResetContent();

            strVBRType.LoadString( IDS_Quality_based_VBR );
            nNewStringIndex = m_cbStreamVideoVBRMode.AddString( strVBRType );
            m_cbStreamVideoVBRMode.SetItemData( nNewStringIndex, VBR_QUALITYBASED );

            if ( fSupports2Pass )
            {
                strVBRType.LoadString( IDS_Constrained_VBR );
                nNewStringIndex = m_cbStreamVideoVBRMode.AddString( strVBRType );
                m_cbStreamVideoVBRMode.SetItemData( nNewStringIndex, VBR_CONSTRAINED );

                strVBRType.LoadString( IDS_Unconstrained_VBR );
                nNewStringIndex = m_cbStreamVideoVBRMode.AddString( strVBRType );
                m_cbStreamVideoVBRMode.SetItemData( nNewStringIndex, VBR_UNCONSTRAINED );
            }

            fIsVBR = pStream->GetVideoIsVBR();
            nVBRModeIndex = pStream->GetVideoVBRMode();

            m_chkStreamVideoIsVBR.EnableWindow( TRUE );
            m_chkStreamVideoIsVBR.SetCheck( fIsVBR ? BST_CHECKED : BST_UNCHECKED );
            m_cbStreamVideoVBRMode.EnableWindow( fIsVBR );
            hr = SelectItemWithData( &m_cbStreamVideoVBRMode, nVBRModeIndex );
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( fIsVBR )
            {
                switch ( nVBRModeIndex )
                {
                case VBR_QUALITYBASED:
                    m_chkStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                    m_txtStreamVideoMaxBitrate.EnableWindow( FALSE );
                    m_txtStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                    m_txtStreamVideoVBRQuality.EnableWindow( TRUE );

                    strValue.Format( _T("%ld"), pStream->GetVideoVBRQuality() );
                    m_txtStreamVideoVBRQuality.SetWindowText( strValue );
                    break;

                case VBR_CONSTRAINED:
                    assert( fSupports2Pass );
                    dwMaxBitrate = pStream->GetVideoMaxBitrate();
                    strValue.Format( _T("%ld"), dwMaxBitrate );
                    m_txtStreamVideoMaxBitrate.SetWindowText( strValue );
                    m_txtStreamVideoMaxBitrate.EnableWindow( TRUE );
                    m_txtStreamVideoVBRQuality.EnableWindow( FALSE );

                    dwMaxBufferWindow = pStream->GetVideoMaxBufferWindow();
                    if ( dwMaxBufferWindow != 0 )
                    {
                        strValue.Format( _T("%ld"), dwMaxBufferWindow );
                        m_txtStreamVideoMaxBufferWindow.SetWindowText( strValue );

                        m_txtStreamVideoMaxBufferWindow.EnableWindow( TRUE );
                        m_chkStreamVideoMaxBufferWindow.SetCheck( BST_CHECKED );
                    }
                    else
                    {
                        m_txtStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                        m_chkStreamVideoMaxBufferWindow.SetCheck( BST_UNCHECKED );
                    }

                    m_chkStreamVideoMaxBufferWindow.EnableWindow( TRUE );
                    break;

                case VBR_UNCONSTRAINED:
                    assert( fSupports2Pass );
                    m_chkStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                    m_txtStreamVideoMaxBitrate.EnableWindow( FALSE );
                    m_txtStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                    m_txtStreamVideoVBRQuality.EnableWindow( FALSE );
                    break;

                default:
                    assert( !"Unknown VBR index!");
                    hr = E_UNEXPECTED;
                    break;
                }
                if ( FAILED( hr ) )
                {
                    break;
                }
            }
            else
            {
                m_chkStreamVideoIsVBR.SetCheck( BST_UNCHECKED );
                m_cbStreamVideoVBRMode.EnableWindow( FALSE );
                m_chkStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                m_txtStreamVideoMaxBitrate.EnableWindow( FALSE );
                m_txtStreamVideoMaxBufferWindow.EnableWindow( FALSE );
                m_txtStreamVideoVBRQuality.EnableWindow( FALSE );
            }
        }
        else
        {
            assert( !pStream->GetVideoIsVBR() );

            DisableVideoVBRControls();
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::CreateProfile( IWMProfile **ppProfile )
{
    HRESULT hr = S_OK;
    IWMProfile* pProfile = NULL;
    IWMProfile3* pProfile3 = NULL;
    IWMProfileManager* pProfileManager = NULL;
    CProfileObject* pCurrentObject;
    WORD wStreamCount;
    WORD wStreamIndex;
    WORD wMutexCount;
    WORD wPrioritizationRecordCount;
    WORD wBandwidthSharingCount;
    DWORD dwProfileObjectIndex;
    DWORD dwProfileObjectCount;
    WM_STREAM_PRIORITY_RECORD* aStreamPriorityRecords = NULL;
    IWMStreamPrioritization* pStreamPrioritization = NULL;
    CString strProfileName;
    LPWSTR wszProfileName = NULL;
    DWORD dwProfileNameLength;

    assert( ppProfile );

    do
    {
        //
        // Give each stream a number, and count the number of each object type
        //
        dwProfileObjectCount = m_lstProfileObjects.GetCount();

        wStreamIndex = 0;
        wMutexCount = 0;
        wPrioritizationRecordCount = 0;
        wBandwidthSharingCount = 0;
        for ( dwProfileObjectIndex = 0; dwProfileObjectIndex < dwProfileObjectCount; dwProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( dwProfileObjectIndex );
            assert( pCurrentObject );

            switch ( pCurrentObject->Type() )
            {
            case OT_Stream:
                wStreamIndex++;
                ((CStream*) pCurrentObject)->SetStreamNumber( wStreamIndex );
                break;

            case OT_Mutex:
                wMutexCount++;
                break;

            case OT_StreamPrioritization:
                wPrioritizationRecordCount++;
                break;

            case OT_BandwidthSharing:
                wBandwidthSharingCount++;
                break;

            default:
                assert( !"Unknown profile object type!" );
                hr = E_UNEXPECTED;
                break;
            }
            if ( FAILED( hr ) )
            {
                break;
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        wStreamCount = wStreamIndex;

        //
        // Create the profile manager and a new profile
        //
        hr = WMCreateProfileManager( &pProfileManager );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pProfileManager );

        hr = pProfileManager->CreateEmptyProfile( WMT_VER_9_0, &pProfile );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pProfile );

        hr = pProfile->QueryInterface( IID_IWMProfile3, (void**) &pProfile3 );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pProfile3 );

        //
        // Add the streams to the profile
        //
        hr = AddStreamsToProfile( pProfile );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Add the mutexes to the profile
        //
        hr = AddMutexesToProfile( pProfile );
        if ( FAILED( hr ) )
        {
            break;
        }

        if ( wPrioritizationRecordCount > 0 )
        {
            assert( 1 == wPrioritizationRecordCount );

            //
            // Create and configure the stream prioritization object
            //
            aStreamPriorityRecords = new WM_STREAM_PRIORITY_RECORD[ wStreamCount ];
            if ( !aStreamPriorityRecords )
            {
                hr = E_OUTOFMEMORY;
                break;
            }

            hr = CreateStreamPrioritizationArray( aStreamPriorityRecords, wStreamCount );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Add the stream prioritization to the profile
            //
            hr = pProfile3->CreateNewStreamPrioritization( &pStreamPrioritization );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pStreamPrioritization );

            hr = pStreamPrioritization->SetPriorityRecords( aStreamPriorityRecords, wPrioritizationRecordCount );
            if ( FAILED( hr ) )
            {
                break;
            }

            hr = pProfile3->SetStreamPrioritization( pStreamPrioritization );
            if ( FAILED( hr ) )
            {
                break;
            }
        }

        //
        // Add bandwidth sharing objects
        //
        hr = AddBandwidthSharingObjectsToProfile( pProfile3 );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the profile name
        //
        m_txtProfileName.GetWindowText( strProfileName );
        dwProfileNameLength = strProfileName.GetLength();
        wszProfileName = new WCHAR[ dwProfileNameLength + 1 ];
        if ( !wszProfileName )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
#ifdef UNICODE
        wcscpy_s(wszProfileName, dwProfileNameLength + 1, strProfileName);
#else // UNICODE
        size_t szCount;
        mbstowcs_s(&szCount, wszProfileName, dwProfileNameLength + 1, strProfileName, dwProfileNameLength); 
#endif // UNICODE

        hr = pProfile->SetName( wszProfileName );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Set the profile description
        //
        hr = SetProfileDescription( pProfile );
        if ( FAILED( hr ) )
        {
            break;
        }


        SAFE_ADDREF( pProfile );
        *ppProfile = pProfile;
    }
    while ( FALSE );

    SAFE_ARRAYDELETE( wszProfileName );
    SAFE_RELEASE( pProfile );
    SAFE_RELEASE( pProfile3 );
    SAFE_RELEASE( pProfileManager );
    SAFE_ARRAYDELETE( aStreamPriorityRecords );
    SAFE_RELEASE( pStreamPrioritization );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetProfileDescription( IWMProfile *pProfile )
{
    HRESULT hr = S_OK;
    DWORD dwProfileObjectCount;
    DWORD dwProfileObjectIndex;
    CString strStreamsDescription;
    CString strProfileDescription;
    CString strNewDescription;
    DWORD dwAudioStreams;
    DWORD dwVideoStreams;
    DWORD dwScriptStreams;
    DWORD dwImageStreams;
    DWORD dwFileStreams;
    DWORD dwWebStreams;
    DWORD dwMutexCount;
    LPWSTR wszProfileDescription = NULL;
    DWORD dwProfileDescriptionLength;
    CProfileObject* pCurrentObject;

    if ( !pProfile )
    {
        return E_INVALIDARG;
    }

    do
    {
        //
        // Count the number of streams and mutexes
        //
        dwAudioStreams = 0;
        dwVideoStreams = 0;
        dwScriptStreams = 0;
        dwImageStreams = 0;
        dwFileStreams = 0;
        dwWebStreams = 0;
        dwMutexCount = 0;
        dwProfileObjectCount = m_lstProfileObjects.GetCount();
        for ( dwProfileObjectIndex = 0; dwProfileObjectIndex < dwProfileObjectCount; dwProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( dwProfileObjectIndex );
            assert( pCurrentObject );

            if ( OT_Stream == pCurrentObject->Type() )
            {
                switch ( ((CStream*) pCurrentObject)->GetStreamType() )
                {
                case ST_Audio:       // Audio stream
                    dwAudioStreams++;
                    break;

                case ST_Video:       // Video stream
                    dwVideoStreams++;
                    break;

                case ST_Script:      // Script stream
                    dwScriptStreams++;
                    break;

                case ST_Image:       // Image stream
                    dwImageStreams++;
                    break;

                case ST_Web:         // Web stream
                    dwWebStreams++;
                    break;

                case ST_File:        // File stream
                    dwFileStreams++;
                    break;

                default:
                    assert( !"Unknown stream type!" );
                    hr = E_UNEXPECTED;
                    break;
                }
                if ( FAILED( hr ) )
                {
                    break;
                }
            }
            else if ( OT_Mutex == pCurrentObject->Type() )
            {
                dwMutexCount++;
            }
        }

        //
        // Create simple description based on the objects in the profile
        //
        strStreamsDescription = _T("Streams: ");
        if ( dwAudioStreams > 0 )
        {
            strNewDescription.Format( _T("%d audio "), dwAudioStreams );
            strStreamsDescription += strNewDescription;
        }
        if ( dwVideoStreams > 0 )
        {
            strNewDescription.Format( _T("%d video "), dwVideoStreams );
            strStreamsDescription += strNewDescription;
        }
        if ( dwScriptStreams > 0 )
        {
            strNewDescription.Format( _T("%d script "), dwScriptStreams );
            strStreamsDescription += strNewDescription;
        }
        if ( dwImageStreams > 0 )
        {
            strNewDescription.Format( _T("%d image "), dwImageStreams );
            strStreamsDescription += strNewDescription;
        }
        if ( dwWebStreams > 0 )
        {
            strNewDescription.Format( _T("%d web "), dwWebStreams );
            strStreamsDescription += strNewDescription;
        }
        if ( dwFileStreams > 0 )
        {
            strNewDescription.Format( _T("%d file "), dwFileStreams );
            strStreamsDescription += strNewDescription;
        }

        if ( dwMutexCount > 0 )
        {
            strProfileDescription.Format( _T("%s Mutexes: %d"), strStreamsDescription, dwMutexCount );
        }
        else
        {
            strProfileDescription.Format( _T("%s"), strStreamsDescription );
        }
        strProfileDescription.TrimRight();

        //
        // Copy the string into a LPWSTR
        //
        dwProfileDescriptionLength = strProfileDescription.GetLength();
        wszProfileDescription = new WCHAR[ dwProfileDescriptionLength + 1 ];
        if ( !wszProfileDescription )
        {
            hr = E_OUTOFMEMORY;
            break;
        }
#ifdef UNICODE
        wcscpy_s( wszProfileDescription, dwProfileDescriptionLength + 1, (LPCTSTR) strProfileDescription );
#else // UNICODE
        size_t szCount;
        mbstowcs_s(&szCount, wszProfileDescription, dwProfileDescriptionLength + 1, strProfileDescription, dwProfileDescriptionLength); 
#endif // UNICODE

        hr = pProfile->SetDescription( wszProfileDescription );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_ARRAYDELETE( wszProfileDescription );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::AddStreamsToProfile( IWMProfile* pProfile )
{
    HRESULT hr = S_OK;
    WORD wStreamIndex;
    CStream* pStream;
    DWORD dwProfileObjectIndex;
    DWORD dwProfileObjectCount;
    CProfileObject* pCurrentObject;
    IWMStreamConfig* pNewStreamConfig = NULL;
    WORD wStreamNumber;
    CString strStreamType;
    CString strStreamName;
    LPWSTR wszStreamName = NULL;
    int nStreamNameLength;

    assert( pProfile );

    do
    {
        dwProfileObjectCount = m_lstProfileObjects.GetCount();
        wStreamIndex = 0;
        for ( dwProfileObjectIndex = 0; dwProfileObjectIndex < dwProfileObjectCount; dwProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( dwProfileObjectIndex );
            assert( pCurrentObject );

            if ( OT_Stream == pCurrentObject->Type() )
            {
                pStream = (CStream*) pCurrentObject;

                SAFE_RELEASE( pNewStreamConfig );
                switch ( pStream->GetStreamType() )
                {
                case ST_Audio:       // Audio stream
                    strStreamType = _T("Audio");
                    if ( !pStream->GetIsUncompressed() )
                    {
                        hr = CreateAudioStream( &pNewStreamConfig,
                                    m_pCodecInfo,
                                    pProfile,
                                    pStream->GetStreamBufferWindow(),
                                    pStream->GetStreamCodecIndex(),
                                    pStream->GetStreamFormatIndex() % VBR_OFFSET,
                                    pStream->GetStreamFormatIndex() / VBR_OFFSET > 0,
                                    pStream->GetStreamFormatIndex() / VBR_OFFSET,
                                    pStream->GetLanguageLCID() );
                    }
                    else
                    {
                        DWORD dwWaveFormatIndex;
                        DWORD dwSamplesPerSec;
                        WORD wNumChannels;
                        WORD wBitsPerSample;

                        dwWaveFormatIndex = pStream->GetWaveFormatIndex();

                        hr = GetUncompressedWaveFormat( dwWaveFormatIndex,
                                                        &dwSamplesPerSec,
                                                        &wNumChannels,
                                                        &wBitsPerSample );
                        if ( SUCCEEDED( hr ) )
                        {
                            hr = CreateUncompressedAudioStream( &pNewStreamConfig,
                                                                pProfile,
                                                                dwSamplesPerSec,
                                                                wNumChannels,
                                                                wBitsPerSample,
                                                                pStream->GetLanguageLCID() );
                        }
                    }
                    break;

                case ST_Video:       // Video stream
                    strStreamType = _T("Video");
                    if ( !pStream->GetIsUncompressed() )
                    {
                        hr = CreateVideoStream( &pNewStreamConfig,
                                    m_pCodecInfo,
                                    pProfile,
                                    pStream->GetStreamCodecIndex(),
                                    pStream->GetStreamBitrate(),
                                    pStream->GetStreamBufferWindow(),
                                    pStream->GetVideoWidth(),
                                    pStream->GetVideoHeight(),
                                    pStream->GetVideoFPS(),
                                    pStream->GetVideoQuality(),
                                    pStream->GetVideoSecondsPerKeyframe(),
                                    pStream->GetVideoIsVBR(),
                                    pStream->GetVideoVBRMode(),
                                    pStream->GetVideoVBRQuality(),
                                    pStream->GetVideoMaxBitrate(),
                                    pStream->GetVideoMaxBufferWindow(),
                                    pStream->GetLanguageLCID() );
                    }
                    else
                    {
                        DWORD dwPixelFormatIndex;
                        GUID guidFormat;
                        DWORD dwFourCC;
                        WORD wBitsPerPixel;

                        dwPixelFormatIndex = pStream->GetPixelFormatIndex();

                        hr = GetUncompressedPixelFormat( dwPixelFormatIndex, &guidFormat, &dwFourCC, &wBitsPerPixel );
                        if ( SUCCEEDED( hr ) )
                        {
                            hr = CreateUncompressedVideoStream( &pNewStreamConfig,
                                        pProfile,
                                        guidFormat,
                                        dwFourCC,
                                        wBitsPerPixel,
                                        NULL,
                                        0,
                                        pStream->GetVideoWidth(),
                                        pStream->GetVideoHeight(),
                                        pStream->GetVideoFPS(),
                                        pStream->GetLanguageLCID() );
                        }
                    }
                    break;

                case ST_Script:      // Script stream
                    strStreamType = _T("Script");
                    hr = CreateScriptStream( &pNewStreamConfig,
                                pProfile,
                                pStream->GetStreamBitrate(),
                                pStream->GetStreamBufferWindow(),
                                pStream->GetLanguageLCID() );
                    break;

                case ST_Image:       // Image stream
                    strStreamType = _T("Image");
                    hr = CreateImageStream( &pNewStreamConfig,
                                pProfile,
                                pStream->GetStreamBitrate(),
                                pStream->GetStreamBufferWindow(),
                                pStream->GetVideoWidth(),
                                pStream->GetVideoHeight(),
                                pStream->GetLanguageLCID() );
                    break;

                case ST_Web:         // Web stream
                    strStreamType = _T("Web");
                    hr = CreateWebStream( &pNewStreamConfig,
                                pProfile,
                                pStream->GetStreamBitrate(),
                                pStream->GetStreamBufferWindow(),
                                pStream->GetLanguageLCID() );
                    break;

                case ST_File:        // File stream
                    strStreamType = _T("File");
                    hr = CreateFileStream( &pNewStreamConfig,
                                pProfile,
                                pStream->GetStreamBitrate(),
                                pStream->GetStreamBufferWindow(),
                                MAX_FILENAME_LENGTH,
                                pStream->GetLanguageLCID() );
                    break;

                default:
                    assert( !"Unknown stream type!" );
                    hr = E_UNEXPECTED;
                    break;
                }
                if ( FAILED( hr ) )
                {
                    break;
                }
                assert( pNewStreamConfig );

                //
                // Set the stream number for the new stream
                //
                wStreamNumber = (WORD) pStream->GetStreamNumber();
                hr = pNewStreamConfig->SetStreamNumber( wStreamNumber );
                if ( FAILED( hr ) )
                {
                    break;
                }

                //
                // Set the stream name
                //
                strStreamName.Format( _T( "%s%d" ), strStreamType, wStreamNumber );
#ifdef UNICODE
                hr = pNewStreamConfig->SetStreamName( (LPWSTR) strStreamName );
#else
                nStreamNameLength = strStreamName.GetLength();
                wszStreamName = new WCHAR[ nStreamNameLength + 1];
                if ( !wszStreamName )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
                size_t szCount;
                mbstowcs_s(&szCount, wszStreamName, nStreamNameLength + 1, strStreamName, nStreamNameLength); 
                hr = pNewStreamConfig->SetStreamName( wszStreamName );
#endif
                if ( FAILED( hr ) )
                {
                    break;
                }

                //
                // Add the stream to the profile
                //
                hr = pProfile->AddStream( pNewStreamConfig );
                if ( FAILED( hr ) )
                {
                    break;
                }

                wStreamIndex++;
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_ARRAYDELETE( wszStreamName );
    SAFE_RELEASE( pNewStreamConfig );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::AddMutexesToProfile( IWMProfile* pProfile )
{
    HRESULT hr = S_OK;
    WORD wMutexIndex;
    CMutex* pMutex;
    DWORD dwProfileObjectIndex;
    DWORD dwProfileObjectCount;
    CProfileObject* pCurrentObject;

    assert( pProfile );

    do
    {
        //
        // Loop through all the objects, creating a mutex for each
        //
        dwProfileObjectCount = m_lstProfileObjects.GetCount();
        wMutexIndex = 0;
        for ( dwProfileObjectIndex = 0; dwProfileObjectIndex < dwProfileObjectCount; dwProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( dwProfileObjectIndex );
            assert( pCurrentObject );

            if ( OT_Mutex == pCurrentObject->Type() )
            {
                pMutex = (CMutex*) pCurrentObject;

                hr = AddMutexToProfile( pProfile, pMutex );
                if ( FAILED( hr ) )
                {
                    break;
                }

                wMutexIndex++;
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::AddMutexToProfile( IWMProfile* pProfile, CMutex *pMutex )
{
    HRESULT hr = S_OK;
    INT nDependacyCount;
    INT nDependancyIndex;
    CStream* pCurrentStream;
    WORD wStreamNumber;
    INT nStreamIndex;
    IWMMutualExclusion* pNewMutex = NULL;
    LPWSTR wszConnectionName = NULL;
    WORD wConnectionNameLength;
    IWMStreamConfig* pStreamConfig = NULL;

    assert( pProfile );
    assert( pMutex );

    do
    {
        //
        // Create a new mutual exclusion object and get the interface used to add streams to it
        //
        hr = pProfile->CreateNewMutualExclusion( &pNewMutex );
        if ( FAILED( hr ) )
        {
            break;
        }
        assert( pNewMutex );

        //
        // Copy the list of streams from the CMutex to the IWMMutualExclusion
        //
        nStreamIndex = 0;
        nDependacyCount = pMutex->StreamCount();
        for ( nDependancyIndex = 0; nDependancyIndex < nDependacyCount; nDependancyIndex++ )
        {
            hr = pMutex->GetStream( nDependancyIndex, &pCurrentStream );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pCurrentStream );

            if ( !pCurrentStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            wStreamNumber = pCurrentStream->GetStreamNumber();

            //
            // Set the connection name for each stream to the same value
            //
            SAFE_RELEASE( pStreamConfig );
            hr = pProfile->GetStreamByNumber( wStreamNumber, &pStreamConfig );
            if ( FAILED( hr ) )
            {
                break;
            }

            if ( 0 == nDependancyIndex )
            {
                //
                // Use the stream name of the first stream in the mutex as
                // the connection name
                //
                SAFE_ARRAYDELETE( wszConnectionName );
                hr = pStreamConfig->GetStreamName( NULL, &wConnectionNameLength );
                if ( FAILED( hr ) )
                {
                    break;
                }

                wszConnectionName = new WCHAR[ wConnectionNameLength ];
                if ( !wszConnectionName )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }

                hr = pStreamConfig->GetStreamName( wszConnectionName, &wConnectionNameLength );
                if ( FAILED( hr ) )
                {
                    break;
                }
            }

            //
            // The connection name only has to match for bitrate mutexes
            //
            if ( MT_Bitrate == pMutex->GetMutexType( ) )
            {
                hr = pStreamConfig->SetConnectionName( wszConnectionName );
                if ( FAILED( hr ) )
                {
                    break;
                }

                hr = pProfile->ReconfigStream( pStreamConfig );
                if ( FAILED( hr ) )
                {
                    break;
                }
            }


            hr = pNewMutex->AddStream( wStreamNumber );
            if ( FAILED( hr ) )
            {
                break;
            }

            nStreamIndex++;
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        assert( nStreamIndex == nDependacyCount );

        //
        // Set the other members of the mutual exclusion
        //
        switch ( pMutex->GetMutexType( ) )
        {
        case MT_Bitrate:
            hr = pNewMutex->SetType( CLSID_WMMUTEX_Bitrate );
            break;

        case MT_Language:
            hr = pNewMutex->SetType( CLSID_WMMUTEX_Language );
            break;

        case MT_Presentation:
            hr = pNewMutex->SetType( CLSID_WMMUTEX_Presentation );
            break;

        default:
            assert( !"Unknown mutex type!" );
            hr = E_UNEXPECTED;
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Add the mutex to the profile
        //
        hr = pProfile->AddMutualExclusion( pNewMutex );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_RELEASE( pNewMutex );
    SAFE_ARRAYDELETE( wszConnectionName );
    SAFE_RELEASE( pStreamConfig );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::CreateStreamPrioritizationArray( WM_STREAM_PRIORITY_RECORD *pPrioritizationInfo, WORD wStreamCount )
{
    HRESULT hr = S_OK;
    CStreamPrioritizationObject* pPrioritizationObject;
    DWORD dwProfileObjectIndex;
    DWORD dwProfileObjectCount;
    CProfileObject* pCurrentObject;

    assert( pPrioritizationInfo );

    do
    {
        //
        // Loop through all the objects to find the stream prioritization object
        //
        dwProfileObjectCount = m_lstProfileObjects.GetCount();
        for ( dwProfileObjectIndex = 0; dwProfileObjectIndex < dwProfileObjectCount; dwProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( dwProfileObjectIndex );
            assert( pCurrentObject );

            if ( OT_StreamPrioritization == pCurrentObject->Type() )
            {
                pPrioritizationObject = (CStreamPrioritizationObject*) pCurrentObject;

                //
                // Set the values
                //
                hr = SetStreamPrioritizationInfo( pPrioritizationInfo, pPrioritizationObject, wStreamCount );
                if ( FAILED( hr ) )
                {
                    break;
                }

                break; // There is only 1 stream prioritization object
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SetStreamPrioritizationInfo( WM_STREAM_PRIORITY_RECORD *pPrioritizationInfo, CStreamPrioritizationObject *pStreamPrioritizationObject, WORD wStreamCount )
{
    HRESULT hr = S_OK;
    INT nStreamsInPrioritizationObject;
    INT nStreamIndex;
    WORD wStreamNumber;
    CStream* pStream = NULL;
    BOOL fStreamIsManatory;

    assert( pPrioritizationInfo );
    assert( pStreamPrioritizationObject );

    do
    {
        //
        // Copy the list of streams from the CStreamPrioritizationObject to the WM_STREAM_PRIORITY_RECORD
        //
        nStreamsInPrioritizationObject = pStreamPrioritizationObject->StreamCount();
        assert( nStreamsInPrioritizationObject == wStreamCount );
        for ( nStreamIndex = 0; nStreamIndex < nStreamsInPrioritizationObject; nStreamIndex++ )
        {
            //
            // Get the stream by priority
            //
            SAFE_RELEASE( pStream );
            hr = pStreamPrioritizationObject->GetStreamWithPriority( nStreamIndex, &pStream );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pStream );

            if ( !pStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            wStreamNumber = pStream->GetStreamNumber();

            //
            // Find out if the stream is mandatory
            //
            hr = pStreamPrioritizationObject->GetStreamMandatory( pStream, &fStreamIsManatory );
            if ( FAILED( hr ) )
            {
                break;
            }

            //
            // Set the values in the WM_STREAM_PRIORITY_RECORD structure
            //
            pPrioritizationInfo[nStreamIndex].wStreamNumber = wStreamNumber;
            pPrioritizationInfo[nStreamIndex].fMandatory = fStreamIsManatory;
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_RELEASE( pStream );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::AddBandwidthSharingObjectsToProfile( IWMProfile3* pProfile3 )
{
    HRESULT hr = S_OK;
    WORD wBandwidthSharingIndex;
    CBandwidthSharingObject* pBandwidthSharingObject;
    DWORD dwProfileObjectIndex;
    DWORD dwProfileObjectCount;
    CProfileObject* pCurrentObject;

    assert( pProfile3 );

    do
    {
        //
        // Loop through all the objects, creating a mutex for each
        //
        dwProfileObjectCount = m_lstProfileObjects.GetCount();
        wBandwidthSharingIndex = 0;
        for ( dwProfileObjectIndex = 0; dwProfileObjectIndex < dwProfileObjectCount; dwProfileObjectIndex++ )
        {
            pCurrentObject = (CProfileObject*) m_lstProfileObjects.GetItemDataPtr( dwProfileObjectIndex );
            assert( pCurrentObject );

            if ( OT_BandwidthSharing == pCurrentObject->Type() )
            {
                pBandwidthSharingObject = (CBandwidthSharingObject*) pCurrentObject;

                hr = AddBandwidthSharingObjectToProfile( pProfile3, pBandwidthSharingObject );
                if ( FAILED( hr ) )
                {
                    break;
                }

                wBandwidthSharingIndex++;
            }
        }
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::AddBandwidthSharingObjectToProfile( IWMProfile3* pProfile3, CBandwidthSharingObject *pBandwidthSharingObject )
{
    HRESULT hr = S_OK;
    INT nDependacyCount;
    INT nDependancyIndex;
    CStream* pCurrentStream;
    WORD wStreamNumber;
    INT nStreamIndex;
    IWMBandwidthSharing* pBSO = NULL;

    assert( pProfile3 );
    assert( pBandwidthSharingObject );

    do
    {
        hr = pProfile3->CreateNewBandwidthSharing( &pBSO );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Don't try to add BSOs with 0 streams - they are not allowed
        //
        nDependacyCount = pBandwidthSharingObject->StreamCount();
        if ( 0 == nDependacyCount )
        {
            break;
        }

        //
        // Copy the list of streams from the CBandwidthSharingObject to the IWMBandwidthSharing
        //
        nStreamIndex = 0;
        for ( nDependancyIndex = 0; nDependancyIndex < nDependacyCount; nDependancyIndex++ )
        {
            hr = pBandwidthSharingObject->GetStream( nDependancyIndex, &pCurrentStream );
            if ( FAILED( hr ) )
            {
                break;
            }
            assert( pCurrentStream );

            if ( !pCurrentStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            wStreamNumber = pCurrentStream->GetStreamNumber();

            hr = pBSO->AddStream( wStreamNumber );
            if ( FAILED( hr ) )
            {
                break;
            }

            nStreamIndex++;
        }
        if ( FAILED( hr ) )
        {
            break;
        }

        assert( nStreamIndex == nDependacyCount );

        //
        // Set the other members of the BSO
        //
        hr = pBSO->SetType( pBandwidthSharingObject->GetBandwidthSharingType() );
        if ( FAILED( hr ) )
        {
            break;
        }

        hr = pBSO->SetBandwidth( pBandwidthSharingObject->GetSharedBitrate(), pBandwidthSharingObject->GetBufferWindow() );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Add the BSO to the profile
        //
        hr = pProfile3->AddBandwidthSharing( pBSO );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    SAFE_RELEASE( pBSO );

    return hr;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::ValidateMutexStreamsAgainstControl( CMutex *pMutex )
{
    HRESULT hr = S_OK;
    INT nSelectedStreamsCount;
    INT nActualStreamCount;
    INT* aSelectedStreams = NULL;
    INT nSelectedStreamIndex;
    CStream* pHighlightedStream;
    StreamType stExpectedStreamType;
    BOOL fStreamIsAllowed;
    CString strMessage;

    assert( m_pDisplayedProfileObject );
    assert( OT_Mutex == m_pDisplayedProfileObject->Type() );
    assert( pMutex );

    do
    {
        pMutex->RemoveAllStreams();

        //
        // Get the list of selected items from control
        //
        nSelectedStreamsCount = m_lstMutexStreams.GetSelCount();
        aSelectedStreams = new INT[ nSelectedStreamsCount ];
        if ( !aSelectedStreams )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        if ( LB_ERR == m_lstMutexStreams.GetSelItems( nSelectedStreamsCount, aSelectedStreams ) )
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // Select all the streams which should be selected
        //
        nActualStreamCount = nSelectedStreamsCount;
        for( nSelectedStreamIndex = 0; nSelectedStreamIndex < nActualStreamCount; nSelectedStreamIndex++ )
        {
            fStreamIsAllowed = TRUE;
            pHighlightedStream = (CStream*) m_lstMutexStreams.GetItemDataPtr( aSelectedStreams[ nSelectedStreamIndex ] );
            assert( pHighlightedStream );
            if ( (CStream*) -1 == pHighlightedStream )
            {
                hr = E_UNEXPECTED;
                break;
            }

            //
            // Use the first stream as the expected types for all the streams.
            // If a stream doesn't match it, it isn't added to the mutex
            //
            if ( 0 == nSelectedStreamIndex )
            {
                stExpectedStreamType = pHighlightedStream->GetStreamType();
            }
            else if ( stExpectedStreamType == pHighlightedStream->GetStreamType() )
            {
            }
            else
            {
                fStreamIsAllowed = FALSE;

                //
                // Print a message saying the stream types didn't match
                //
                DisplayMessage( IDS_Multiple_stream_types_in_mutex_error );

                //
                // Remove the stream from the array of selected streams
                //
                nActualStreamCount--;
                if ( 0 != nActualStreamCount && nActualStreamCount != nSelectedStreamIndex )
                {
                    aSelectedStreams[nSelectedStreamIndex] = aSelectedStreams[nActualStreamCount];
                    nSelectedStreamIndex--; // Loop through this index again
                }
            }

            //
            // Check the stream to see if it's in any other mutexes
            //
            if ( ( pMutex->GetMutexType() == MT_Bitrate ) && ( InBitrateMutex( pHighlightedStream ) ) )
            {
                fStreamIsAllowed = FALSE;

                //
                // Print a message saying the stream types didn't match
                //
                DisplayMessage( IDS_Stream_in_multiple_mutexes_error );

                //
                // Remove the stream from the array of selected streams
                //
                nActualStreamCount--;
                if ( 0 != nActualStreamCount && nActualStreamCount != nSelectedStreamIndex )
                {
                    aSelectedStreams[nSelectedStreamIndex] = aSelectedStreams[nActualStreamCount];
                    nSelectedStreamIndex--; // Loop through this index again
                }
            }

            //
            // Add the stream to the mutex
            //
            if ( fStreamIsAllowed )
            {
                pMutex->AddStream( pHighlightedStream );
            }
        }

        //
        // Re-select the streams in the mutex, since streams may have been removed
        //
        if ( nActualStreamCount != nSelectedStreamsCount )
        {
            if ( LB_ERR == m_lstMutexStreams.SelItemRange( FALSE, 0, m_lstMutexStreams.GetCount() - 1 ) )
            {
                hr = E_OUTOFMEMORY;
                break;
            }


            for( nSelectedStreamIndex = 0; nSelectedStreamIndex < nActualStreamCount; nSelectedStreamIndex++ )
            {
                if ( LB_ERR == m_lstMutexStreams.SetSel( aSelectedStreams[ nSelectedStreamIndex ], TRUE ) )
                {
                    hr = E_OUTOFMEMORY;
                    break;
                }
            }
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        DisplayMessage( IDS_Fatal_error );
    }

    SAFE_ARRAYDELETE( aSelectedStreams );
}


///////////////////////////////////////////////////////////////////////////////
DWORD CGenProfileDlg::GetNumberOfFormatsSupported( GUID guidStreamType,
                                                   DWORD dwCodecIndex,
                                                   BOOL fIsVBR,
                                                   DWORD dwNumPasses )
{
    HRESULT hr;
    DWORD dwCodecFormatCount;

    do
    {
        hr = SetCodecVBRSettings( m_pCodecInfo, guidStreamType, dwCodecIndex, fIsVBR, dwNumPasses );
        if ( FAILED( hr ) )
        {
            break;
        }

        //
        // Get the number of formats the current setting supports
        //
        hr = m_pCodecInfo->GetCodecFormatCount( guidStreamType, dwCodecIndex, &dwCodecFormatCount );
        if ( FAILED( hr ) )
        {
            break;
        }
    }
    while ( FALSE );

    if ( FAILED( hr ) )
    {
        return 0;
    }

    return dwCodecFormatCount;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::DisplayMessageAndTerminate( LPCTSTR tszMessage )
{
    MessageBox( tszMessage, _T("Unrecoverable error occured!") );
    ExitProcess( -1 );
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::SelectItemWithData( CComboBox *pcbComboBox, DWORD dwRequestedItemData )
{
    INT nItemCount;
    INT nItemIndex;
    DWORD_PTR dwItemData;

    assert( pcbComboBox );

    nItemCount = pcbComboBox->GetCount();
    for ( nItemIndex = 0; nItemIndex < nItemCount; nItemIndex++ )
    {
        dwItemData = pcbComboBox->GetItemData( nItemIndex );
        if ( CB_ERR == dwItemData )
        {
            return E_UNEXPECTED;
        }

        if ( dwItemData == dwRequestedItemData )
        {
            pcbComboBox->SetCurSel( nItemIndex );
            return S_OK;
        }
    }

    return E_FAIL;
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::DisableVideoVBRControls()
{
    m_chkStreamVideoIsVBR.EnableWindow( FALSE );
    m_chkStreamVideoIsVBR.SetCheck( BST_UNCHECKED );
    m_cbStreamVideoVBRMode.EnableWindow( FALSE );
    m_chkStreamVideoMaxBufferWindow.EnableWindow( FALSE );
    m_txtStreamVideoMaxBitrate.EnableWindow( FALSE );
    m_txtStreamVideoMaxBufferWindow.EnableWindow( FALSE );
    m_txtStreamVideoVBRQuality.EnableWindow( FALSE );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeCBPixelFormat()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );

    DWORD dwStringIndex = m_cbPixelFormat.GetCurSel();
    DWORD dwIndex = ( DWORD )( m_cbPixelFormat.GetItemData( dwStringIndex ) );

    ((CStream*) m_pDisplayedProfileObject)->SetPixelFormatStringIndex( dwStringIndex );
    ((CStream*) m_pDisplayedProfileObject)->SetPixelFormatIndex( dwIndex );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::OnSelchangeCBLanguage()
{
    assert( m_pDisplayedProfileObject );
    assert( OT_Stream == m_pDisplayedProfileObject->Type() );

    DWORD dwStringIndex = m_cbLanguage.GetCurSel();
    LCID lcid = ( DWORD )( m_cbLanguage.GetItemData( dwStringIndex ) );

    ((CStream*) m_pDisplayedProfileObject)->SetLanguageLCID( lcid );
}


///////////////////////////////////////////////////////////////////////////////
void CGenProfileDlg::SelectLanguage( LCID lcid )
{
    DWORD dwLanguageCount = m_cbLanguage.GetCount();

    for ( DWORD i = 0; i < dwLanguageCount; i ++ )
    {
        if ( m_cbLanguage.GetItemData( i ) == lcid )
        {
            m_cbLanguage.SetCurSel( i );
            break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
HRESULT CGenProfileDlg::PopulateLanguageCB()
{
    HRESULT hr = S_OK;

    if ( m_cbLanguage.GetCount() > 0 )
    {
        // Langauge list has been initialized.
        return hr;
    }

    IMultiLanguage * pMLang = NULL;
    IEnumRfc1766 * pEnumRfc1766 = NULL;

    do
    {
        hr = CoCreateInstance( CLSID_CMultiLanguage,
                               NULL,
                               CLSCTX_ALL,
                               IID_IMultiLanguage,
                               (VOID **) &pMLang );

        if( FAILED( hr ) )
        {
            break;
        }

        hr = pMLang->EnumRfc1766( &pEnumRfc1766 );
        if( FAILED( hr ) )
        {
            break;
        }

        while( SUCCEEDED( hr ) )
        {
            RFC1766INFO rfcinfo = {0};
            ULONG       ulFetched = 0;

            hr = pEnumRfc1766->Next( 1, &rfcinfo, &ulFetched );
            if( FAILED( hr ) || ( S_FALSE == hr ) )
            {
                break;
            }

            if( 0 == ulFetched )
            {
                continue;
            }

            //
            // Check to see if the item dup with the added item, if it is, don't add it.
            //
            BOOL fDup = FALSE;

            for( int i = 0; i < m_cbLanguage.GetCount(); i++ )
            {
                if( ( (LCID)m_cbLanguage.GetItemData(i) ) == rfcinfo.lcid )
                {
                    fDup = TRUE;
                    break;
                }
            }

            if( fDup )
            {
                continue;
            }

            TCHAR szLanguageName[256];


#ifdef UNICODE
            _sntprintf_s( szLanguageName, ARRAY_SIZE(szLanguageName), ARRAY_SIZE(szLanguageName) - 1, _T("%s [%s]"), rfcinfo.wszLocaleName, rfcinfo.wszRfc1766);
#else //UNICODE
            CHAR szLocaleName[128];
            CHAR szRfc1766[128];

            if( 0 == WideCharToMultiByte( CP_ACP, 0, rfcinfo.wszLocaleName, -1, szLocaleName, 128, NULL, NULL )
                || 0 == WideCharToMultiByte( CP_ACP, 0, rfcinfo.wszRfc1766, -1, szRfc1766, 128, NULL, NULL ) )
            {
                hr = HRESULT_FROM_WIN32( GetLastError() );
                break;
            }

            (void)StringCchPrintf( szLanguageName, ARRAYSIZE(szLanguageName), _T("%s [%s]"), szLocaleName, szRfc1766 );

            // prefast fix #140662 - _sntprintf may not null terminate result string
            szLanguageName[ sizeof( szLanguageName )  - 1 ] = '\0';
#endif // UNICODE

            int nIndex = m_cbLanguage.AddString( szLanguageName );
            m_cbLanguage.SetItemData( nIndex, rfcinfo.lcid );
        }

    }
    while( FALSE );

    SAFE_RELEASE( pMLang );
    SAFE_RELEASE( pEnumRfc1766 );

    return( hr );
}
