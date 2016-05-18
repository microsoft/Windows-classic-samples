//*****************************************************************************
//
// Microsoft Windows Media
// Copyright (C) Microsoft Corporation. All rights reserved.
//
// FileName: ContextSamplePropPageImpl.cpp
//
// Abstract:
//
//*****************************************************************************

#include "stdafx.h"
#include "ContextSamplePropPageImpl.h"
#include "ContextSamplePropPage.h"
#include <atlbase.h>


/////////////////////////////////////////////////////////////////////////////
// CContextSamplePropPage

CContextSamplePropPage::CContextSamplePropPage( void )
{
    m_dwTitleID = IDS_TITLEContextSamplePropPage;
    m_dwHelpFileID = IDS_HELPFILEContextSamplePropPage;
    m_dwDocStringID = IDS_DOCSTRINGContextSamplePropPage;

    m_wmsOrigContextTypes = WMS_CONTEXT_PLUGIN_NO_CONTEXT;
    m_wmsContextTypes = WMS_CONTEXT_PLUGIN_NO_CONTEXT;
    m_fInitializing = TRUE;
}


/////////////////////////////////////////////////////////////////////////////
STDMETHODIMP 
CContextSamplePropPage::Apply( void )
{
    HRESULT hr = S_OK;
    long lMaxSize = 0;

    if ( ! m_bDirty )
    {
        return( S_OK );
    }

    hr = RetrieveDialogInformation();

    if( SUCCEEDED( hr ) )
    {
        //
        // Set all the properties that have changed.
        //
        if( !m_pPluginAdmin )
        {
            MessageBox( IDS_SERVER_CONNECT_ERROR );
            return( E_FAIL );
        }
        else
        {
            // Check the old values and only set if they have changed
            if( ( NULL != m_bstrOrigOutputPath.m_str )
                && ( NULL != m_bstrOutputPath.m_str )
                && _wcsicmp( m_bstrOrigOutputPath, m_bstrOutputPath ) )
            {
                m_pPluginAdmin->put_OutputPath( m_bstrOutputPath );
            }

            if( m_wmsOrigContextTypes != m_wmsContextTypes )
            {
                m_pPluginAdmin->put_ContextTypes( m_wmsContextTypes );
            }
        }
    }

    if( SUCCEEDED( hr ) )
    {
        SetDirty( FALSE );
    }

    return( hr );
} //end of Apply.


/////////////////////////////////////////////////////////////////////////////
void 
CContextSamplePropPage::MessageBox( UINT id )
{
    CComBSTR bstrMessage;
    CComBSTR bstrTitle( "Windows Media Services" );

    if( bstrMessage.LoadString( id ) )
    {
        ::MessageBox( m_hWnd, bstrMessage, bstrTitle, MB_OK );
    }
}


/////////////////////////////////////////////////////////////////////////////
HRESULT 
CContextSamplePropPage::Connect()
{
    // Interface pointers are given in the following order:
    // m_ppUnk[0] = Plugin Admin Interface
    // m_ppUnk[1] = Server Interface (IWMSServer)
    // m_ppUnk[2] = Pubpoint Interface (IWMSPublishingPoint)

    // IF m_nObjects == 3 then m_pUnk[2] will have the pubpoint interface

    if( 0 == m_nObjects )
    {
        return( E_UNEXPECTED );
    }

    m_pPluginAdmin = m_ppUnk[0];

    if( ! m_pPluginAdmin )
    {
        return( E_FAIL );
    }

    return( S_OK );
}


/////////////////////////////////////////////////////////////////////////////
LRESULT 
CContextSamplePropPage::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
    ATLTRACE( "== CContextSamplePropPage::OnInitDialog\n" );

    EnableThemeDialogTexture( m_hWnd, ETDT_ENABLETAB );

    HRESULT hr = S_OK;
    m_fInitializing = TRUE;

    hr = Connect();
    if ( FAILED( hr ) )
    {
        MessageBox( IDS_SERVER_CONNECT_ERROR );
        Deactivate();
        return( m_fInitializing = FALSE );
    }

    hr = GetPluginSetting();
    if( SUCCEEDED( hr ) )
    {
        hr = PopulateControls();
    }

    return( m_fInitializing = FALSE );
} //end of OnInitDialog.


/////////////////////////////////////////////////////////////////////////////
HRESULT CContextSamplePropPage::RetrieveDialogInformation()
{
    WCHAR wstrOutputPath[MAX_PATH];
    HRESULT hr = S_OK;

    //
    // Grab the new information for the Output Path property.
    //
    if( GetDlgItemText( IDC_EDIT_OUTPUT_PATH, wstrOutputPath, MAX_PATH) )
    {
        m_bstrOutputPath = wstrOutputPath;
        if( NULL == m_bstrOutputPath.m_str )
        {
            hr = E_FAIL;
        }
    }
    else
    {
        // Failed to get text from control
        hr = E_FAIL;
    }

    // reset to zero and then set bits accordingly
    m_wmsContextTypes = WMS_CONTEXT_PLUGIN_NO_CONTEXT;

    if( BST_CHECKED == ::SendMessage( GetDlgItem( IDC_CHECK_USER_CONTEXT ), BM_GETCHECK, 0, 0 ) )
    {
        m_wmsContextTypes = (WMS_CONTEXT_PLUGIN_CONTEXT_TYPE) ( m_wmsContextTypes | WMS_CONTEXT_PLUGIN_USER_CONTEXT );
    }
    if( BST_CHECKED == ::SendMessage( GetDlgItem( IDC_CHECK_PRESENTATION_CONTEXT ), BM_GETCHECK, 0, 0 ) )
    {
        m_wmsContextTypes = (WMS_CONTEXT_PLUGIN_CONTEXT_TYPE) ( m_wmsContextTypes | WMS_CONTEXT_PLUGIN_PRESENTATION_CONTEXT );
    }
    if( BST_CHECKED == ::SendMessage( GetDlgItem( IDC_CHECK_COMMAND_REQUEST_CONTEXT ), BM_GETCHECK, 0, 0 ) )
    {
        m_wmsContextTypes = (WMS_CONTEXT_PLUGIN_CONTEXT_TYPE) ( m_wmsContextTypes | WMS_CONTEXT_PLUGIN_COMMAND_REQUEST_CONTEXT );
    }
    if( BST_CHECKED == ::SendMessage( GetDlgItem( IDC_CHECK_COMMAND_RESPONSE_CONTEXT ), BM_GETCHECK, 0, 0 ) )
    {
        m_wmsContextTypes = (WMS_CONTEXT_PLUGIN_CONTEXT_TYPE) ( m_wmsContextTypes | WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT );
    }

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
HRESULT 
CContextSamplePropPage::GetPluginSetting()
{
    HRESULT hr = S_OK;

    ATLTRACE( "CContextSamplePropPage::GetPluginSetting\n" );

    //
    // Get plugin's properties.
    //
    if( !m_pPluginAdmin )
    {
        MessageBox( IDS_PLUGIN_PROPS_ERROR );
        m_fInitializing = FALSE;
        return( E_FAIL );
    }
    hr = m_pPluginAdmin->get_OutputPath( &m_bstrOrigOutputPath );
    if( FAILED( hr ) )
    {
        MessageBox( IDS_PLUGIN_PROPS_ERROR );
        Deactivate();
        m_fInitializing = FALSE;
        return( hr );
    }
    m_bstrOutputPath = m_bstrOrigOutputPath;

    hr = m_pPluginAdmin->get_ContextTypes( &m_wmsOrigContextTypes );
    if( FAILED( hr ) )
    {
        MessageBox( IDS_PLUGIN_PROPS_ERROR );
        Deactivate();
        m_fInitializing = FALSE;
        return( hr );
    }
    m_wmsContextTypes = m_wmsOrigContextTypes;

    return( hr );
} //end of GetPluginSetting.


/////////////////////////////////////////////////////////////////////////////
HRESULT 
CContextSamplePropPage::PopulateControls()
{
    HRESULT hr = S_OK;
    bool fEnableMaxSize = false;
    
    ATLTRACE( "CContextSamplePropPage::PopulateControls\n" );

    //
    // Display the plugin's properties.
    //
    ::EnableWindow( GetDlgItem( IDC_EDIT_OUTPUT_PATH ), TRUE );
    SetDlgItemText( IDC_EDIT_OUTPUT_PATH, ( wchar_t * ) m_bstrOutputPath );
    ::SendMessage( GetDlgItem( IDC_EDIT_OUTPUT_PATH ), EM_SETLIMITTEXT, MAX_PATH - 1, 0 );

    if( WMS_CONTEXT_PLUGIN_USER_CONTEXT & m_wmsContextTypes )
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_USER_CONTEXT ), BM_SETCHECK, BST_CHECKED, 0 );
    }
    else
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_USER_CONTEXT ), BM_SETCHECK, BST_UNCHECKED, 0 );
    }

    if( WMS_CONTEXT_PLUGIN_PRESENTATION_CONTEXT & m_wmsContextTypes )
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_PRESENTATION_CONTEXT ), BM_SETCHECK, BST_CHECKED, 0 );
    }
    else
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_PRESENTATION_CONTEXT ), BM_SETCHECK, BST_UNCHECKED, 0 );
    }

    if( WMS_CONTEXT_PLUGIN_COMMAND_REQUEST_CONTEXT & m_wmsContextTypes )
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_COMMAND_REQUEST_CONTEXT ), BM_SETCHECK, BST_CHECKED, 0 );
    }
    else
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_COMMAND_REQUEST_CONTEXT ), BM_SETCHECK, BST_UNCHECKED, 0 );
    }

    if( WMS_CONTEXT_PLUGIN_COMMAND_RESPONSE_CONTEXT & m_wmsContextTypes )
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_COMMAND_RESPONSE_CONTEXT ), BM_SETCHECK, BST_CHECKED, 0 );
    }
    else
    {
        ::SendMessage( GetDlgItem( IDC_CHECK_COMMAND_RESPONSE_CONTEXT ), BM_SETCHECK, BST_UNCHECKED, 0 );
    }

    return( hr );
} //end of PopulateControls.
