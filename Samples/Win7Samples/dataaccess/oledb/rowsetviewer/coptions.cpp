//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module COPTIONS.CPP
//
//-----------------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "COptions.h"


/////////////////////////////////////////////////////////////////////////////
// COptionsSheet
//
/////////////////////////////////////////////////////////////////////////////
COptionsSheet::COptionsSheet()
	: CPropSheetLite("Options")
{
	AddPage(&m_CDataSource);
	AddPage(&m_CCommand);
	AddPage(&m_CAccessor);
	AddPage(&m_CRowset);
	AddPage(&m_CNotify);
	AddPage(&m_CError);

	//DataSource
	m_dwCLSCTX				= CLSCTX_INPROC_SERVER;
	m_wszRemoteServer[0]	= EOL;
	m_dwCreateOpts			= CREATE_INITIALIZE | CREATE_SETPROPERTIES;

	//Command
	m_dwCommandOpts			= COMMAND_ROWSAFFECTED | COMMAND_RELEASE_OPENOBJECTS;

	//Accessor
	m_wBindingType			= DBTYPE_WSTR;
	m_dwMaxLength			= MAX_COL_SIZE;
	m_dwAccessorOpts		= ACCESSOR_BIND_VALUE | ACCESSOR_BIND_LENGTH | ACCESSOR_BIND_STATUS;

	//Rowset
	m_dwConvFlags			= CONV_ALPHABOOL;
	m_dwRowsetOpts			= ROWSET_SETDEFAULTPROPS;
	m_cRowsToFetch			= 0;

	//Notify
	m_dwNotifyOpts			= NOTIFY_OLEDB | NOTIFY_IDBASYNCHNOTIFY | NOTIFY_IROWSETNOTIFY | NOTIFY_IROWPOSITIONCHANGE | NOTIFY_IPOOLNOTIFY;

	//Errors

	//Load Options from the Registry
	LoadOptions();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsSheet::LoadOptions
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsSheet::LoadOptions()
{
	//DataSource
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"CLSCTX",				&m_dwCLSCTX);
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"RemoteServer",			m_wszRemoteServer, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"ConnectOpts",			&m_dwCreateOpts);

	//NOTE: Since these bits we added later, the default is 0.  Instead of having negative logic
	//all over the place (NO_QI_OPTIONAL), we just store and obtain the bits in reverse
	ENABLE_BIT(m_dwCreateOpts, CREATE_QI_MANDATORY, !(m_dwCreateOpts & CREATE_QI_MANDATORY));
	ENABLE_BIT(m_dwCreateOpts, CREATE_QI_OPTIONAL,	!(m_dwCreateOpts & CREATE_QI_OPTIONAL));

	//Command
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"CommandOpts",			&m_dwCommandOpts);

	//Accessor
	DWORD dwType = m_wBindingType;	
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"BindingType",			&dwType);
	m_wBindingType = (DBTYPE)dwType;
																		
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"MaxLength",			&m_dwMaxLength);
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"AccessorOpts",			&m_dwAccessorOpts);

	//Rowset																
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"ConvFlags",			&m_dwConvFlags);
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"RowsetOpts",			&m_dwRowsetOpts);
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"RowsToFetch",			&m_cRowsToFetch);
								   
	//Notifications
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"NotifyOpts",			&m_dwNotifyOpts);

	//Errors
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"ErrorPostingFlags",	&g_dwErrorPost);
	GetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"BreakID",				&g_dwBreakID);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// COptionsSheet::SaveOptions
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsSheet::SaveOptions()
{
	//DataSource
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"CLSCTX",				m_dwCLSCTX);
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"RemoteServer",			m_wszRemoteServer);

	//NOTE: Since these bits we added later, the default is 0.  Instead of having negative logic
	//all over the place (NO_QI_OPTIONAL), we just store and obtain the bits in reverse
	DWORD dwCreateOpts = m_dwCreateOpts;
	ENABLE_BIT(dwCreateOpts, CREATE_QI_MANDATORY,	!(dwCreateOpts & CREATE_QI_MANDATORY));
	ENABLE_BIT(dwCreateOpts, CREATE_QI_OPTIONAL,	!(dwCreateOpts & CREATE_QI_OPTIONAL));
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"ConnectOpts",			dwCreateOpts);

	//Command
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"CommandOpts",			m_dwCommandOpts);

	//Accessor
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"BindingType",			m_wBindingType);
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"MaxLength",			m_dwMaxLength);
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"AccessorOpts",			m_dwAccessorOpts);

	//Rowset															
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"ConvFlags",			m_dwConvFlags);
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"RowsetOpts",			m_dwRowsetOpts);
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"RowsToFetch",			m_cRowsToFetch);

	//Notifications
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"NotifyOpts",			m_dwNotifyOpts);

	//Errors
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"ErrorPostingFlags",	g_dwErrorPost);
	SetRegEntry(HKEY_ROWSETVIEWER, wszOPTIONS_KEY, L"BreakID",				g_dwBreakID);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// COptionsSheet::GetBindingType
//
/////////////////////////////////////////////////////////////////////////////
DBTYPE COptionsSheet::GetBindingType(DBTYPE wNativeType)
{
	//Native Binding
	if((m_wBindingType & ~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR)) == DBTYPE_UDT)
		return wNativeType | (m_wBindingType - DBTYPE_UDT);

	//Otherwise use the type selected
	return m_wBindingType;
}

	
/////////////////////////////////////////////////////////////////////////////
// COptionsDataSource
//
/////////////////////////////////////////////////////////////////////////////
COptionsDataSource::COptionsDataSource()
	: CPropPageLite(IDD_FULLCONNECT_OPTIONS, "DataSource")
{
}


/////////////////////////////////////////////////////////////////////////////
// COptionsDataSource::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsDataSource::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Set CLSCTX to previous saved values
	CheckDlgButton(IDB_INPROC_SERVER,		BST2STATE(pCOptionsSheet->m_dwCLSCTX & CLSCTX_INPROC_SERVER));
	CheckDlgButton(IDB_LOCAL_SERVER,		BST2STATE(pCOptionsSheet->m_dwCLSCTX & CLSCTX_LOCAL_SERVER));
	CheckDlgButton(IDB_REMOTE_SERVER,		BST2STATE(pCOptionsSheet->m_dwCLSCTX & CLSCTX_REMOTE_SERVER));
	CheckDlgButton(IDB_INPROC_HANDLER,		BST2STATE(pCOptionsSheet->m_dwCLSCTX & CLSCTX_INPROC_HANDLER));

	//Currently there is no way to pass RemoteServer in GetDataSource methods...
	wSendMessage(GetDlgItem(IDE_REMOTESERVER), WM_SETTEXT, 0, pCOptionsSheet->m_wszRemoteServer);
	::EnableWindow(GetDlgItem(IDE_REMOTESERVER), pCOptionsSheet->m_dwCLSCTX & CLSCTX_REMOTE_SERVER);

	//Set Connection Options to previous saved values
	CheckDlgButton(IDB_INIT_INITIALIZE,		BST2STATE(pCOptionsSheet->m_dwCreateOpts & CREATE_INITIALIZE));
	CheckDlgButton(IDB_INIT_SETPROPERTIES,	BST2STATE(pCOptionsSheet->m_dwCreateOpts & CREATE_SETPROPERTIES));

	//AutoQI
	CheckDlgButton(IDB_QI_MANDATORY,		BST2STATE(pCOptionsSheet->m_dwCreateOpts & CREATE_QI_MANDATORY));
	CheckDlgButton(IDB_QI_OPTIONAL,			BST2STATE(pCOptionsSheet->m_dwCreateOpts & CREATE_QI_OPTIONAL));

	//Set Other Options
	CheckDlgButton(IDB_USESERVICECOMP,		BST2STATE(pCOptionsSheet->m_dwCreateOpts & CREATE_USESERVICECOMP));
	
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsDataSource::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsDataSource::OnApply() 
{
	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();
	
	//Obtain CLSCTX
	pCOptionsSheet->m_dwCLSCTX = 0;
	if(IsDlgButtonChecked(IDB_INPROC_SERVER))
		pCOptionsSheet->m_dwCLSCTX |= CLSCTX_INPROC_SERVER;
	if(IsDlgButtonChecked(IDB_LOCAL_SERVER))
		pCOptionsSheet->m_dwCLSCTX |= CLSCTX_LOCAL_SERVER;
	if(IsDlgButtonChecked(IDB_REMOTE_SERVER))
		pCOptionsSheet->m_dwCLSCTX |= CLSCTX_REMOTE_SERVER;
	if(IsDlgButtonChecked(IDB_INPROC_HANDLER))
		pCOptionsSheet->m_dwCLSCTX |= CLSCTX_INPROC_HANDLER;

	//Obtain RemoteServer
	wSendMessage(GetDlgItem(IDE_REMOTESERVER), WM_GETTEXT, MAX_NAME_LEN, pCOptionsSheet->m_wszRemoteServer);

	//Obtain Connection Options
	pCOptionsSheet->m_dwCreateOpts = 0;
	if(IsDlgButtonChecked(IDB_INIT_INITIALIZE))
		pCOptionsSheet->m_dwCreateOpts |= CREATE_INITIALIZE;
	if(IsDlgButtonChecked(IDB_INIT_SETPROPERTIES))
		pCOptionsSheet->m_dwCreateOpts |= CREATE_SETPROPERTIES;

	//AutoQI
	ENABLE_BIT(pCOptionsSheet->m_dwCreateOpts,	CREATE_QI_MANDATORY,	IsDlgButtonChecked(IDB_QI_MANDATORY));
	ENABLE_BIT(pCOptionsSheet->m_dwCreateOpts,	CREATE_QI_OPTIONAL,		IsDlgButtonChecked(IDB_QI_OPTIONAL));

	//Obtain Other Options
	if(IsDlgButtonChecked(IDB_USESERVICECOMP))
		pCOptionsSheet->m_dwCreateOpts |= CREATE_USESERVICECOMP;

	//delegate
	return CPropPageLite::OnApply();
}



/////////////////////////////////////////////////////////////////////////////
// COptionsDataSource::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsDataSource::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		ON_COMMAND(IDB_REMOTE_SERVER,	::EnableWindow(GetDlgItem(IDE_REMOTESERVER), IsDlgButtonChecked(IDB_REMOTE_SERVER)))
	};

	return FALSE;
}

					
/////////////////////////////////////////////////////////////////////////////
// COptionsCommand
//
/////////////////////////////////////////////////////////////////////////////
COptionsCommand::COptionsCommand()
	: CPropPageLite(IDD_OPTIONS_COMMAND, "Command")
{
}


/////////////////////////////////////////////////////////////////////////////
// COptionsCommand::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsCommand::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Display RowAffected
	CheckDlgButton(IDB_DISPLAY_ROWSAFFECTED,	BST2STATE(pCOptionsSheet->m_dwCommandOpts & COMMAND_ROWSAFFECTED));

	//Display ReleaseRowset
	CheckDlgButton(IDB_RELEASE_OPENOBJECTS,		BST2STATE(pCOptionsSheet->m_dwCommandOpts & COMMAND_RELEASE_OPENOBJECTS));

	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsCommand::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsCommand::OnApply() 
{
	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Display RowsAffected
	ENABLE_BIT(pCOptionsSheet->m_dwCommandOpts, COMMAND_ROWSAFFECTED, IsDlgButtonChecked(IDB_DISPLAY_ROWSAFFECTED));

	//Display ReleaseRowset
	ENABLE_BIT(pCOptionsSheet->m_dwCommandOpts, COMMAND_RELEASE_OPENOBJECTS, IsDlgButtonChecked(IDB_RELEASE_OPENOBJECTS));

	//delegate
	return CPropPageLite::OnApply();
}



/////////////////////////////////////////////////////////////////////////////
// COptionsAccessor
//
/////////////////////////////////////////////////////////////////////////////
COptionsAccessor::COptionsAccessor()
	: CPropPageLite(IDD_OPTIONS_ACCESSOR, "Accessor")
{
}


/////////////////////////////////////////////////////////////////////////////
// COptionsAccessor::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsAccessor::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Populate Binding Type Combo
	m_comboBindingTypes.CreateIndirect(m_hWnd, IDC_BINDINGTYPE);

	//Add the Native Binding Type
	m_comboBindingTypes.AddString(L"Native Type", DBTYPE_UDT);

	//Add all the others
	for(ULONG i=0; i<g_cDBTypes; i++)
		m_comboBindingTypes.AddString(g_rgDBTypes[i].pwszName, g_rgDBTypes[i].lItem);

	//Select the Default Type...
	DBTYPE wBindingType	= pCOptionsSheet->GetBindingType();
	DBTYPE wBaseType = wBindingType & ~(DBTYPE_BYREF | DBTYPE_ARRAY | DBTYPE_VECTOR | DBTYPE_RESERVED);
	m_comboBindingTypes.SetSelValue(wBaseType);

	//MODIFIERS (BYREF, ARRAY, VECTOR)
	CheckDlgButton(IDB_TO_BYREF,	BST2STATE(wBindingType & DBTYPE_BYREF));
	CheckDlgButton(IDB_TO_ARRAY,	BST2STATE(wBindingType & DBTYPE_ARRAY));
	CheckDlgButton(IDB_TO_VECTOR,	BST2STATE(wBindingType & DBTYPE_VECTOR));

	//DBPART
	CheckDlgButton(IDB_BIND_VALUE,	BST2STATE(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BIND_VALUE));
	CheckDlgButton(IDB_BIND_LENGTH,	BST2STATE(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BIND_LENGTH));
	CheckDlgButton(IDB_BIND_STATUS,	BST2STATE(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BIND_STATUS));

	//DBMEMOWNER
	CheckDlgButton(IDB_OWNED_PROVIDER,	BST2STATE(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_OWNED_PROVIDER));

	//MaxLength
	wSendMessageFmt(GetDlgItem(IDE_MAXLENGTH), WM_SETTEXT, 0, L"%lu", pCOptionsSheet->m_dwMaxLength);

	//BLOB Binding Type
	if(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BLOB_ISEQSTREAM)
		CheckDlgButton(IDB_BLOB_ISEQSTREAM, BST_CHECKED);
	else if(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BLOB_ILOCKBYTES)
		CheckDlgButton(IDB_BLOB_ILOCKBYTES, BST_CHECKED);
	else if(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BLOB_ISTORAGE)
		CheckDlgButton(IDB_BLOB_ISTORAGE, BST_CHECKED);
	else if(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BLOB_ISTREAM)
		CheckDlgButton(IDB_BLOB_ISTREAM, BST_CHECKED);
	else 
		CheckDlgButton(IDB_BLOB_INLINE, BST_CHECKED);

	//Bookmark Column
	CheckDlgButton(IDB_BIND_BOOKMARK, BST2STATE(pCOptionsSheet->m_dwAccessorOpts & ACCESSOR_BIND_BOOKMARK));

	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsAccessor::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsAccessor::OnApply() 
{
	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//BindingType
	DBTYPE wBindingType = (DBTYPE)m_comboBindingTypes.GetItemParam(m_comboBindingTypes.GetCurSel());
	if(wBindingType == CB_ERR)
		wBindingType = DBTYPE_WSTR;

	//MODIFIERS (BYREF, ARRAY, VECTOR)
	if(IsDlgButtonChecked(IDB_TO_BYREF))
		wBindingType |= DBTYPE_BYREF;
	if(IsDlgButtonChecked(IDB_TO_ARRAY))
		wBindingType |= DBTYPE_ARRAY;
	if(IsDlgButtonChecked(IDB_TO_VECTOR))
		wBindingType |= DBTYPE_VECTOR;
	pCOptionsSheet->SetBindingType(wBindingType);

	//DBPART
	pCOptionsSheet->m_dwAccessorOpts &= ~(ACCESSOR_BIND_VALUE | ACCESSOR_BIND_LENGTH | ACCESSOR_BIND_STATUS);
	if(IsDlgButtonChecked(IDB_BIND_VALUE))
		pCOptionsSheet->m_dwAccessorOpts |= ACCESSOR_BIND_VALUE;
	if(IsDlgButtonChecked(IDB_BIND_LENGTH))
		pCOptionsSheet->m_dwAccessorOpts |= ACCESSOR_BIND_LENGTH;
	if(IsDlgButtonChecked(IDB_BIND_STATUS))
		pCOptionsSheet->m_dwAccessorOpts |= ACCESSOR_BIND_STATUS;

	//DBMEMOWNER
	ENABLE_BIT(pCOptionsSheet->m_dwAccessorOpts, ACCESSOR_OWNED_PROVIDER,	IsDlgButtonChecked(IDB_OWNED_PROVIDER));

	//MaxLength
	if(!GetEditBoxValue(GetDlgItem(IDE_MAXLENGTH), (LONG*)&pCOptionsSheet->m_dwMaxLength, 0/*Min*/))
	{
		pCOptionsSheet->m_dwMaxLength = MAX_COL_SIZE;
		return FALSE;
	}

	//BLOB Binding Type
	ENABLE_BIT(pCOptionsSheet->m_dwAccessorOpts, ACCESSOR_BLOB_ISEQSTREAM,	IsDlgButtonChecked(IDB_BLOB_ISEQSTREAM));
	ENABLE_BIT(pCOptionsSheet->m_dwAccessorOpts, ACCESSOR_BLOB_ILOCKBYTES,	IsDlgButtonChecked(IDB_BLOB_ILOCKBYTES));
	ENABLE_BIT(pCOptionsSheet->m_dwAccessorOpts, ACCESSOR_BLOB_ISTORAGE,	IsDlgButtonChecked(IDB_BLOB_ISTORAGE));
	ENABLE_BIT(pCOptionsSheet->m_dwAccessorOpts, ACCESSOR_BLOB_ISTREAM,		IsDlgButtonChecked(IDB_BLOB_ISTREAM));

	//Bookmark Column
	ENABLE_BIT(pCOptionsSheet->m_dwAccessorOpts, ACCESSOR_BIND_BOOKMARK,	IsDlgButtonChecked(IDB_BIND_BOOKMARK));

	//delegate
	return CPropPageLite::OnApply();
}

/////////////////////////////////////////////////////////////////////////////
// COptionsRowset
//
/////////////////////////////////////////////////////////////////////////////
COptionsRowset::COptionsRowset()
	: CPropPageLite(IDD_OPTIONS_ROWSET, "Rowset")
{
}


/////////////////////////////////////////////////////////////////////////////
// COptionsRowset::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsRowset::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Hexidecimal
	CheckDlgButton(IDB_DISPLAY_HEXADECIMAL,		BST2STATE(pCOptionsSheet->m_dwConvFlags & CONV_HEX));
	//Use MSDADC Only
	CheckDlgButton(IDB_MSDADC_ONLY,				BST2STATE(pCOptionsSheet->m_dwConvFlags & CONV_MSDADC_ONLY));

	//DefaultProps
	CheckDlgButton(IDB_SETDEFAULTPROPS,			BST2STATE(pCOptionsSheet->m_dwRowsetOpts & ROWSET_SETDEFAULTPROPS));

	//RowsToFetch
	CheckDlgButton(IDB_ROWSTOFETCH,				BST2STATE(pCOptionsSheet->m_dwRowsetOpts & ROWSET_ROWSTOFETCH));
	CheckDlgButton(IDB_DEFAULTFETCH,			BST2STATE(!(pCOptionsSheet->m_dwRowsetOpts & ROWSET_ROWSTOFETCH)));
	wSendMessageFmt(GetDlgItem(IDE_ROWSTOFETCH), WM_SETTEXT, 0, L"%lu", pCOptionsSheet->m_cRowsToFetch);
	::EnableWindow(GetDlgItem(IDE_ROWSTOFETCH), pCOptionsSheet->m_dwRowsetOpts & ROWSET_ROWSTOFETCH);

	//FetchData
	CheckDlgButton(IDB_FETCHDATA,				BST2STATE(!(pCOptionsSheet->m_dwRowsetOpts & ROWSET_NODATA)));

	//Hidden Columns
	CheckDlgButton(IDB_HIDDENCOLUMNS,			BST2STATE(pCOptionsSheet->m_dwRowsetOpts & ROWSET_HIDDENCOLUMNS));

	//Always ReleaseRows
	CheckDlgButton(IDB_ALWAYSRELEASEROWS,		BST2STATE(pCOptionsSheet->m_dwRowsetOpts & ROWSET_ALWAYSRELEASEROWS));

	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsRowset::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsRowset::OnApply() 
{
	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Hexidecimal
	ENABLE_BIT(pCOptionsSheet->m_dwConvFlags,	CONV_HEX,				IsDlgButtonChecked(IDB_DISPLAY_HEXADECIMAL));
	//Use MSDADC Only
	ENABLE_BIT(pCOptionsSheet->m_dwConvFlags,	CONV_MSDADC_ONLY,		IsDlgButtonChecked(IDB_MSDADC_ONLY));

	//DefaultProps
	ENABLE_BIT(pCOptionsSheet->m_dwRowsetOpts,	ROWSET_SETDEFAULTPROPS,	IsDlgButtonChecked(IDB_SETDEFAULTPROPS));

	//RowsToFetch
	ENABLE_BIT(pCOptionsSheet->m_dwRowsetOpts,	ROWSET_ROWSTOFETCH,		IsDlgButtonChecked(IDB_ROWSTOFETCH));
	if(pCOptionsSheet->m_dwRowsetOpts & ROWSET_ROWSTOFETCH)
	{
		if(!GetEditBoxValue(GetDlgItem(IDE_ROWSTOFETCH), (LONG*)&pCOptionsSheet->m_cRowsToFetch, 0/*Min*/))
		{
			pCOptionsSheet->m_cRowsToFetch = 0;
			return FALSE;
		}
	}

	//FetchData
	ENABLE_BIT(pCOptionsSheet->m_dwRowsetOpts,	ROWSET_NODATA,			!IsDlgButtonChecked(IDB_FETCHDATA));

	//HiddenColumns
	ENABLE_BIT(pCOptionsSheet->m_dwRowsetOpts,	ROWSET_HIDDENCOLUMNS,	IsDlgButtonChecked(IDB_HIDDENCOLUMNS));

	//Always ReleaseRows
	ENABLE_BIT(pCOptionsSheet->m_dwRowsetOpts,	ROWSET_ALWAYSRELEASEROWS,	IsDlgButtonChecked(IDB_ALWAYSRELEASEROWS));

	//delegate
	return CPropPageLite::OnApply();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsDataRowset::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsRowset::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		ON_COMMAND(IDB_ROWSTOFETCH,		::EnableWindow(GetDlgItem(IDE_ROWSTOFETCH), IsDlgButtonChecked(IDB_ROWSTOFETCH)))
		ON_COMMAND(IDB_DEFAULTFETCH,	::EnableWindow(GetDlgItem(IDE_ROWSTOFETCH), IsDlgButtonChecked(IDB_ROWSTOFETCH)))
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// COptionsNotify
//
/////////////////////////////////////////////////////////////////////////////
COptionsNotify::COptionsNotify()
	: CPropPageLite(IDD_OPTIONS_NOTIFICATIONS, "Notify")
{
}


/////////////////////////////////////////////////////////////////////////////
// COptionsNotify::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsNotify::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Display
	CheckDlgButton(IDB_DISPLAY_OLEDB,				BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_OLEDB));
	CheckDlgButton(IDB_DISPLAY_PROMPTVETO,			BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_PROMPTVETO));

	//IUnknown
	CheckDlgButton(IDB_DISPLAY_ADDREF,				BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_ADDREF));
	CheckDlgButton(IDB_DISPLAY_RELEASE,				BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_RELEASE));
	CheckDlgButton(IDB_DISPLAY_QI,					BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_QI));

	//Trace Provider
	CheckDlgButton(IDB_USE_TRACEPROVIDER,			BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_TRACEPROVIDER));

	//Notifications
	CheckDlgButton(IDB_DISPLAY_IDBASYNCHNOTIFY,		BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_IDBASYNCHNOTIFY));
	CheckDlgButton(IDB_DISPLAY_IROWSETNOTIFY,		BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_IROWSETNOTIFY));
	CheckDlgButton(IDB_DISPLAY_IROWPOSITIONCHANGE,	BST2STATE(pCOptionsSheet->m_dwNotifyOpts & NOTIFY_IROWPOSITIONCHANGE));
		
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsNotify::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsNotify::OnApply() 
{
	//PropertySheet (BackPointer)
	COptionsSheet* pCOptionsSheet = (COptionsSheet*)GetParent();

	//Display
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_OLEDB,				IsDlgButtonChecked(IDB_DISPLAY_OLEDB));
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_PROMPTVETO,			IsDlgButtonChecked(IDB_DISPLAY_PROMPTVETO));

	//IUnknown
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_ADDREF,				IsDlgButtonChecked(IDB_DISPLAY_ADDREF));
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_RELEASE,				IsDlgButtonChecked(IDB_DISPLAY_RELEASE));
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_QI,					IsDlgButtonChecked(IDB_DISPLAY_QI));

	//Trace Provider
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_TRACEPROVIDER,		IsDlgButtonChecked(IDB_USE_TRACEPROVIDER));

	//Notiifcations
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_IDBASYNCHNOTIFY,		IsDlgButtonChecked(IDB_DISPLAY_IDBASYNCHNOTIFY));
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_IROWSETNOTIFY,		IsDlgButtonChecked(IDB_DISPLAY_IROWSETNOTIFY));
	ENABLE_BIT(pCOptionsSheet->m_dwNotifyOpts, NOTIFY_IROWPOSITIONCHANGE,	IsDlgButtonChecked(IDB_DISPLAY_IROWPOSITIONCHANGE));

	//delegate
	return CPropPageLite::OnApply();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsError
//
/////////////////////////////////////////////////////////////////////////////
COptionsError::COptionsError()
	: CPropPageLite(IDD_OPTIONS_ERRORS, "Errors")
{
}


/////////////////////////////////////////////////////////////////////////////
// COptionsError::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsError::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//ErrorInfo
	if(GetErrorPosting(EP_ERRORINFO_ALWAYS))
		CheckDlgButton(IDB_ERRORINFO_ALWAYS,		BST_CHECKED);
	if(GetErrorPosting(EP_ERRORINFO_FAILURE))
		CheckDlgButton(IDB_ERRORINFO_FAILURE,		BST_CHECKED);
	if(GetErrorPosting(EP_ERRORINFO_NEVER))
		CheckDlgButton(IDB_ERRORINFO_NEVER,			BST_CHECKED);

	//HRESULTs
	if(GetErrorPosting(EP_HRESULT_ALWAYS))
		CheckDlgButton(IDB_HRESULT_ALWAYS,			BST_CHECKED);
	if(GetErrorPosting(EP_HRESULT_NOERRORINFO))
		CheckDlgButton(IDB_HRESULT_NOERRORINFO,		BST_CHECKED);
	if(GetErrorPosting(EP_HRESULT_FAILURE))
		CheckDlgButton(IDB_HRESULT_FAILURE,			BST_CHECKED);
	if(GetErrorPosting(EP_HRESULT_NEVER))
		CheckDlgButton(IDB_HRESULT_NEVER,			BST_CHECKED);

	//IMallocSpy
	CheckDlgButton(IDB_IMALLOC_SPY,					BST2STATE(GetErrorPosting(EP_IMALLOC_SPY)));
	CheckDlgButton(IDB_IMALLOC_ALLOCS,				BST2STATE(GetErrorPosting(EP_IMALLOC_ALLOCS)));
	::EnableWindow(GetDlgItem(IDB_IMALLOC_ALLOCS),	GetErrorPosting(EP_IMALLOC_SPY));
	::EnableWindow(GetDlgItem(IDB_BREAKID),			GetErrorPosting(EP_IMALLOC_SPY));

	//BreakID
	CheckDlgButton(IDB_BREAKID,						BST2STATE(g_dwBreakID));
	wSendMessageFmt(GetDlgItem(IDE_BREAKID), WM_SETTEXT, 0, L"%d", g_dwBreakID);
	::EnableWindow(GetDlgItem(IDE_BREAKID), GetErrorPosting(EP_IMALLOC_SPY) && g_dwBreakID);

	//RefCounts
	CheckDlgButton(IDB_REFCOUNT_FAILURE,			BST2STATE(GetErrorPosting(EP_REFCOUNT_FAILURE)));
	
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// COptionsError::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsError::OnApply() 
{
	//ErrorInfo
	SetErrorPosting(EP_ERRORINFO_ALWAYS,	IsDlgButtonChecked(IDB_ERRORINFO_ALWAYS));
	SetErrorPosting(EP_ERRORINFO_FAILURE,	IsDlgButtonChecked(IDB_ERRORINFO_FAILURE));
	SetErrorPosting(EP_ERRORINFO_NEVER,		IsDlgButtonChecked(IDB_ERRORINFO_NEVER));

	//HRESULTs
	SetErrorPosting(EP_HRESULT_ALWAYS,		IsDlgButtonChecked(IDB_HRESULT_ALWAYS));
	SetErrorPosting(EP_HRESULT_NOERRORINFO,	IsDlgButtonChecked(IDB_HRESULT_NOERRORINFO));
	SetErrorPosting(EP_HRESULT_FAILURE,		IsDlgButtonChecked(IDB_HRESULT_FAILURE));
	SetErrorPosting(EP_HRESULT_NEVER,		IsDlgButtonChecked(IDB_HRESULT_NEVER));

	//IMallocSpy
	SetErrorPosting(EP_IMALLOC_SPY,			IsDlgButtonChecked(IDB_IMALLOC_SPY));
	SetErrorPosting(EP_IMALLOC_ALLOCS,		IsDlgButtonChecked(IDB_IMALLOC_ALLOCS));

	//Do we need to turn on spying. (if not done already)
	if(GetErrorPosting(EP_IMALLOC_SPY))
	{
		CWinApp* pCWinApp = (CWinApp*)GetAppLite();
		if(pCWinApp && !pCWinApp->m_pCMallocSpy)
		{
			pCWinApp->m_pCMallocSpy = new CMallocSpy;
			if(pCWinApp->m_pCMallocSpy)
				pCWinApp->m_pCMallocSpy->Register(); 
		}
	}

	//BreakID
	g_dwBreakID = 0;
	if(IsDlgButtonChecked(IDB_BREAKID))
	{
		//Obtain the BreakID value from the EditBox
		if(!GetEditBoxValue(GetDlgItem(IDE_BREAKID), (LONG*)&g_dwBreakID, 0/*Min*/))
			return FALSE;
	}

	//RefCounts
	SetErrorPosting(EP_REFCOUNT_FAILURE,	IsDlgButtonChecked(IDB_REFCOUNT_FAILURE));

	//delegate
	return CPropPageLite::OnApply();
}



/////////////////////////////////////////////////////////////////////////////
// COptionsError::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL COptionsError::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		case IDB_IMALLOC_SPY:
		{
			BOOL bChecked = IsDlgButtonChecked(IDB_IMALLOC_SPY);
			::EnableWindow(GetDlgItem(IDB_IMALLOC_ALLOCS), bChecked);
			::EnableWindow(GetDlgItem(IDB_BREAKID),			bChecked);
			::EnableWindow(GetDlgItem(IDE_BREAKID),			bChecked);
			return TRUE;
		}

		ON_COMMAND(IDB_BREAKID,			::EnableWindow(GetDlgItem(IDE_BREAKID),			IsDlgButtonChecked(IDB_BREAKID)))
	};

	return FALSE;
}
