//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module CDIALOG.CPP
//
//-----------------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "Headers.h"
#include "CMainWindow.h"


/////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////
//PROPERTY
enum PROP_COLUMNS
{
	PROP_NAME, 
	PROP_TYPE,
	PROP_VALUE,
	PROP_FLAGS, 
	PROP_DESC,
	PROP_COLID
};

#define NOTSET -2




/////////////////////////////////////////////////////////////////////
// Schema Constraint Mapping and Information
//
/////////////////////////////////////////////////////////////////////
static const SCHEMAINFO g_rgSchemaInfo[] = 
{
	VALUE_PWCHAR(DBSCHEMA_ASSERTIONS),				3, 
			{	
				1,	L"CONSTRAINT_CATALOG",		DBTYPE_BSTR,	
				2,	L"CONSTRAINT_SCHEMA",		DBTYPE_BSTR,	
				3,	L"CONSTRAINT_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_CATALOGS),				1, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,		
			},
	VALUE_PWCHAR(DBSCHEMA_CHARACTER_SETS),			3, 
			{
				1,	L"CHARACTER_SET_CATALOG",	DBTYPE_BSTR,
				2,	L"CHARACTER_SET_SCHEMA",	DBTYPE_BSTR,	
				3,	L"CHARACTER_SET_NAME",		DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_CHECK_CONSTRAINTS),		3, 
			{
				1,	L"CONSTRAINT_CATALOG",		DBTYPE_BSTR,	
				2,	L"CONSTRAINT_SCHEMA",		DBTYPE_BSTR,	
				3,	L"CONSTRAINT_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_COLLATIONS),				3, 
			{
				1,	L"COLLATIONS_CATALOG",		DBTYPE_BSTR,	
				2,	L"COLLATIONS_SCHEMA",		DBTYPE_BSTR,	
				3,	L"COLLATIONS_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_COLUMN_DOMAIN_USAGE),		4, 
			{
				1,	L"DOMAIN_CATALOG",			DBTYPE_BSTR,	
				2,	L"DOMAIN_SCHEMA",			DBTYPE_BSTR,
				3,	L"DOMAIN_NAME",				DBTYPE_BSTR,	
				4,	L"COLUMN_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_COLUMN_PRIVILEGES),		6, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
				4,	L"COLUMN_NAME",				DBTYPE_BSTR,	
				5,	L"GRANTOR",					DBTYPE_BSTR,	
				6,	L"GRANTEE",					DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_COLUMNS),					4, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
				4,	L"COLUMN_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_CONSTRAINT_COLUMN_USAGE),	4,
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
				4,	L"COLUMN_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_CONSTRAINT_TABLE_USAGE),	3,
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
			},		  
	VALUE_PWCHAR(DBSCHEMA_FOREIGN_KEYS),			6, 
			{
				1,	L"PK_TABLE_CATALOG",		DBTYPE_BSTR,	
				2,	L"PK_TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"PK_TABLE_NAME",			DBTYPE_BSTR,	
				4,	L"FK_TABLE_CATALOG",		DBTYPE_BSTR,	
				5,	L"FK_TABLE_SCHEMA",			DBTYPE_BSTR,	
				6,	L"FK_TABLE_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_INDEXES),					5, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"INDEX_NAME",				DBTYPE_BSTR,	
				4,	L"TYPE",					DBTYPE_UI2,	
				5,	L"TABLE_NAME",				DBTYPE_BSTR,	
			},			
	VALUE_PWCHAR(DBSCHEMA_KEY_COLUMN_USAGE),		7, 
			{
				1,	L"CONSTRAINT_CATALOG",		DBTYPE_BSTR,	
				2,	L"CONSTRAINT_SCHEMA",		DBTYPE_BSTR,	
				3,	L"CONSTRAINT_NAME",			DBTYPE_BSTR,	
				4,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				5,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				6,	L"TABLE_NAME",				DBTYPE_BSTR,	
				7,	L"COLUMN_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_PRIMARY_KEYS),			3, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
			},		  
	VALUE_PWCHAR(DBSCHEMA_PROCEDURE_COLUMNS),		4, 
			{
				1,	L"PROCEDURE_CATALOG",		DBTYPE_BSTR,	
				2,	L"PROCEDURE_SCHEMA",		DBTYPE_BSTR,	
				3,	L"PROCEDURE_NAME",			DBTYPE_BSTR,	
				4,	L"COLUMN_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_PROCEDURE_PARAMETERS),	4, 
			{
				1,	L"PROCEDURE_CATALOG",		DBTYPE_BSTR,	
				2,	L"PROCEDURE_SCHEMA",		DBTYPE_BSTR,	
				3,	L"PROCEDURE_NAME",			DBTYPE_BSTR,	
				4,	L"PARAMETER_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_PROCEDURES),				4, 
			{
				1,	L"PROCEDURE_CATALOG",		DBTYPE_BSTR,	
				2,	L"PROCEDURE_SCHEMA",		DBTYPE_BSTR,	
				3,	L"PROCEDURE_NAME",			DBTYPE_BSTR,	
				4,	L"PROCEDURE_TYPE",			DBTYPE_I2,	
			},	
	VALUE_PWCHAR(DBSCHEMA_PROVIDER_TYPES),			2, 
			{
				1,	L"DATA_TYPE",				DBTYPE_UI2,
				2,	L"BEST_MATCH",				DBTYPE_BOOL,	
			},
	VALUE_PWCHAR(DBSCHEMA_REFERENTIAL_CONSTRAINTS),	3, 
			{
				1,	L"CONSTRAINT_CATALOG",		DBTYPE_BSTR,	
				2,	L"CONSTRAINT_SCHEMA",		DBTYPE_BSTR,	
				3,	L"CONSTRAINT_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_SCHEMATA),				3, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"SCHEMA_OWNER",			DBTYPE_BSTR,	
			},	
	VALUE_PWCHAR(DBSCHEMA_SQL_LANGUAGES),			0, 
			{
				1,	L"",						DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_STATISTICS),				3, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_TABLE_CONSTRAINTS),		7, 
			{
				1,	L"CONSTRAINT_CATALOG",		DBTYPE_BSTR,	
				2,	L"CONSTRAINT_SCHEMA",		DBTYPE_BSTR,	
				3,	L"CONSTRAINT_NAME",			DBTYPE_BSTR,	
				4,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				5,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				6,	L"TABLE_NAME",				DBTYPE_BSTR,	
				7,	L"CONSTRAINT_TYPE",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_TABLE_PRIVILEGES),		5,
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
				4,	L"GRANTOR",					DBTYPE_BSTR,	
				5,	L"GRANTEE",					DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_TABLES),					4, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				4,	L"TABLE_NAME",				DBTYPE_BSTR,	
				3,	L"TABLE_TYPE",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_TRANSLATIONS),			3, 
			{
				1,	L"TRANSLATION_CATALOG",		DBTYPE_BSTR,	
				2,	L"TRANSLATION_SCHEMA",		DBTYPE_BSTR,	
				3,	L"TRANSLATION_NAME",		DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_USAGE_PRIVILEGES),		6, 
			{
				1,	L"OBJECT_CATALOG",			DBTYPE_BSTR,	
				2,	L"OBJECT_SCHEMA",			DBTYPE_BSTR,	
				3,	L"OBJECT_NAME",				DBTYPE_BSTR,	
				4,	L"OBJECT_TYPE",				DBTYPE_BSTR,	
				5,	L"GRANTOR",					DBTYPE_BSTR,	
				6,	L"GRANTEE",					DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_VIEW_COLUMN_USAGE),		3, 
			{
				1,	L"VIEW_CATALOG",			DBTYPE_BSTR,	
				2,	L"VIEW_SCHEMA",				DBTYPE_BSTR,	
				3,	L"VIEW_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_VIEW_TABLE_USAGE),		3, 
			{
				1,	L"VIEW_CATALOG",			DBTYPE_BSTR,	
				2,	L"VIEW_SCHEMA",				DBTYPE_BSTR,	
				3,	L"VIEW_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(DBSCHEMA_VIEWS),					3, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
			},

	//2.0
	VALUE_PWCHAR(DBSCHEMA_TABLES_INFO),				4, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				4,	L"TABLE_NAME",				DBTYPE_BSTR,	
				3,	L"TABLE_TYPE",				DBTYPE_BSTR,	
			},


	//2.1
	VALUE_PWCHAR(DBSCHEMA_TRUSTEE),					4, 
			{
				1,	L"TRUSTEE_NAME",			DBTYPE_BSTR,	
				2,	L"TRUSTEE_GUID",			DBTYPE_BSTR,	//Variant doesn't support GUID?
				3,	L"TRUSTEE_PROPID",			DBTYPE_UI4,	
				4,	L"TRUSTEE_TYPE",			DBTYPE_UI4,	
			},

	//2.6
	VALUE_PWCHAR(DBSCHEMA_TABLE_STATISTICS),		7, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
				4,	L"STATISTICS_CATALOG",		DBTYPE_BSTR,	
				5,	L"STATISTICS_SCHEMA",		DBTYPE_BSTR,	
				6,	L"STATISTICS_NAME",			DBTYPE_BSTR,	
				7,	L"STATISTICS_TYPE",			DBTYPE_UI2,	
			},

	VALUE_PWCHAR(DBSCHEMA_CHECK_CONSTRAINTS_BY_TABLE),6, 
			{
				1,	L"TABLE_CATALOG",			DBTYPE_BSTR,	
				2,	L"TABLE_SCHEMA",			DBTYPE_BSTR,	
				3,	L"TABLE_NAME",				DBTYPE_BSTR,	
				4,	L"CONSTRAINT_CATALOG",		DBTYPE_BSTR,	
				5,	L"CONSTRAINT_SCHEMA",		DBTYPE_BSTR,	
				6,	L"CONSTRAINT_NAME",			DBTYPE_BSTR,	
			},

	//OLAP Extensions
	VALUE_PWCHAR(MDSCHEMA_CUBES),					3, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_DIMENSIONS),				5, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
				4,	L"DIMENSION_NAME",			DBTYPE_BSTR,	
				5,	L"DIMENSION_UNIQUE_NAME",	DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_HIERARCHIES),				6, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
				4,	L"DIMENSION_UNIQUE_NAME",	DBTYPE_BSTR,	
				5,	L"HIERARCHY_NAME",			DBTYPE_BSTR,	
				6,	L"HIERARCHY_UNIQUE_NAME",	DBTYPE_BSTR,	
			},
    VALUE_PWCHAR(MDSCHEMA_LEVELS),					7, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
				4,	L"DIMENSION_UNIQUE_NAME",	DBTYPE_BSTR,	
				5,	L"HIERARCHY_UNIQUE_NAME",	DBTYPE_BSTR,	
				6,	L"LEVEL_NAME",				DBTYPE_BSTR,	
				7,	L"LEVEL_UNIQUE_NAME",		DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_MEASURES),				5, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
				4,	L"MEASURE_NAME",			DBTYPE_BSTR,	
				5,	L"MEASURE_UNIQUE_NAME",		DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_PROPERTIES),				9, 
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
				4,	L"DIMENSION_UNIQUE_NAME",	DBTYPE_BSTR,	
				5,	L"HIERARCHY_UNIQUE_NAME",	DBTYPE_BSTR,	
				6,	L"LEVEL_UNIQUE_NAME",		DBTYPE_BSTR,	
				7,	L"MEMBER_UNIQUE_NAME",		DBTYPE_BSTR,	
//				8,	L"PROPERTY_NAME",			DBTYPE_BSTR,	
//				9,	L"PROPERTY_TYPE",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_MEMBERS),					11,
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
				4,	L"DIMENSION_UNIQUE_NAME",	DBTYPE_BSTR,	
				5,	L"HIERARCHY_UNIQUE_NAME",	DBTYPE_BSTR,	
				6,	L"LEVEL_UNIQUE_NAME",		DBTYPE_BSTR,	
				7,	L"LEVEL_NUMBER",			DBTYPE_BSTR,	
//				8,	L"MEMBER_NAME",				DBTYPE_BSTR,	
//				9,	L"MEMBER_UNIQUE_NAME",		DBTYPE_BSTR,	
//				10,	L"MEMBER_TYPE",				DBTYPE_BSTR,	
//				11,	L"MDTREE_OPERATOR",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_FUNCTIONS),				 3,
			{
				1,	L"ORIGIN",					DBTYPE_I4,	
				2,	L"INTERFACE_NAME",			DBTYPE_BSTR,	
				3,	L"FUNCTION_NAME",			DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_ACTIONS),					 3,
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_COMMANDS),				 3,
			{
				//TODO: MDSCHEMA_COMMANDS Restrictions need to be doc'd.
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
			},
	VALUE_PWCHAR(MDSCHEMA_SETS),					 3,
			{
				1,	L"CATALOG_NAME",			DBTYPE_BSTR,	
				2,	L"SCHEMA_NAME",				DBTYPE_BSTR,	
				3,	L"CUBE_NAME",				DBTYPE_BSTR,	
			},

};
			

/////////////////////////////////////////////////////////////////////////////
// CRowsetViewerDlg
//
/////////////////////////////////////////////////////////////////////////////
CRowsetViewerDlg::CRowsetViewerDlg(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild)
{
	ASSERT(pCMainWindow || pCMDIChild);
	m_pCMDIChild	= pCMDIChild;
	m_pCMainWindow	= pCMDIChild ? pCMDIChild->m_pCMainWindow : pCMainWindow;
}


////////////////////////////////////////////////////////////////
// CRowsetViewerDlg::GetOptions
//
/////////////////////////////////////////////////////////////////
COptionsSheet*	CRowsetViewerDlg::GetOptions()
{
	return m_pCMainWindow->GetOptions();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider
//
/////////////////////////////////////////////////////////////////////////////
CFullConnectProvider::CFullConnectProvider()
	: CPropPageLite(IDD_FULLCONNECT_PROVIDER, "Provider")
{
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	CMainWindow* pCMainWindow = pCFullConnect->m_pCMainWindow;

	//Controls...
	m_comboConfig.CreateIndirect(m_hWnd, IDC_CONFIG);
	
	//CombBoxEx (allows images and indentation)
	m_comboProvider.CreateIndirect(m_hWnd, IDC_PROVIDER);
	SendMessage(m_comboProvider.m_hWnd, CBEM_SETIMAGELIST, 0, (LPARAM)ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR));

	//Setup our "dynamic" window handle property...
	//NOTE: We have to do this here, since this needs to not only be a valid window handle,
	//must it also be the full connect dialog handle, so that if the provider displays a prompt
	//dialog its "modal" to this one and not the main window...
	m_hWndProp		= (HWND)NOTSET;
	
	//Auto Save
	CheckDlgButton(IDB_AUTOSAVE,	BST2STATE(!(pCMainWindow->GetOptions()->m_dwCreateOpts & CREATE_NO_AUTOSAVE)));

	//Fill in Enumerator list
	//NOTE: The Enumerator Combo drop down is deferred, since its quite expensive.
	//The drop down is populated once the drop down is selected.  Before population
	//it defaults to the users saved config provider, or MSDASQL if not saved...

	//Obtain the property values for DBPROP_INIT_PROMPT
	ULONG cPropVals = 0;
	const WIDENAMEMAP* rgPropVals = NULL;
	if(SUCCEEDED(GetStaticPropValues(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, &cPropVals, &rgPropVals)))
	{
		//Fill in Prompt Combo
		pCFullConnect->InitPropCombo(GetDlgItem(IDC_PROMPT), cPropVals, rgPropVals, m_dwPromptProp == NOTSET ? 0 : m_dwPromptProp);
	}

	//Load all items from the Registry
	//Limit it to the first 200 registry entries!
	//So we don't spend all day loading this combo box!
	WCHAR wszBuffer[MAX_NAME_LEN];
	for(ULONG i=0; i<200; i++)
	{
		if(S_OK != GetRegEnumKey(HKEY_ROWSETVIEWER, wszCONFIG_KEY, i, wszBuffer, MAX_NAME_LEN))
			break;
		
		m_comboConfig.AddString(wszBuffer);
	}
	
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnApply
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnApply() 
{
	//PropertySheet (BackPointer)
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	CMainWindow* pCMainWindow = pCFullConnect->m_pCMainWindow;

	CWaitCursor waitCursor;
	GetProviderName();
	
	// Make a connection using the supplied values
	if(FAILED(pCFullConnect->FullConnect()))
		return FALSE;

	//Auto Save?
	BOOL bAutoSave = IsDlgButtonChecked(IDB_AUTOSAVE);
	if(bAutoSave)
		pCFullConnect->SaveDefaults();

	//Update Options
	ENABLE_BIT(pCMainWindow->GetOptions()->m_dwCreateOpts, CREATE_NO_AUTOSAVE, !bAutoSave);

	//delegate
	return CPropPageLite::OnApply();
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnSetActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnSetActive() 
{
	RefreshProvider();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnKillActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnKillActive() 
{
	return UpdateProvider();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnCommand(UINT iID, HWND hWndCtrl) 
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	switch(iID)
	{
		case IDB_DEFAULTS:
		{
			pCFullConnect->ResetDefaults();
			return TRUE;
		}

		case IDB_CONFIG_SAVE:
		{
			//Now update registry
			UpdateProvider();
			pCFullConnect->SaveDefaults();
			return TRUE;
		}
						
		case IDB_CONFIG_REMOVE:
		{
			//Obtain the Selected Combo Text
			INDEX iSel = m_comboConfig.GetSelText(pCFullConnect->m_wszConfigName, MAX_NAME_LEN);
			
			//Remove this item from the ComboBox
			m_comboConfig.DeleteString(iSel);
								
			//Formulate the key
			WCHAR wszKeyName[MAX_NAME_LEN];
			StringFormat(wszKeyName, NUMELE(wszKeyName), L"%s\\%s", wszCONFIG_KEY, pCFullConnect->m_wszConfigName); 

			//Now Remove this Item from the registry
			DelRegEntry(HKEY_ROWSETVIEWER, wszKeyName, TRUE/*fSubKeys*/);
			
			//We need to make sure this is not the default Config
			wszKeyName[0] = EOL;
			GetRegEntry(HKEY_ROWSETVIEWER, wszCONFIG_KEY, L"DefaultConfig", wszKeyName, MAX_NAME_LEN);
			if(StringCompare(wszKeyName, pCFullConnect->m_wszConfigName))
				SetRegEntry(HKEY_ROWSETVIEWER, wszCONFIG_KEY, L"DefaultConfig", L"(Default)");

			//And Remove it from our Recent Config List
			pCFullConnect->RemoveRecentConfig(pCFullConnect->m_wszConfigName);
			pCFullConnect->m_wszConfigName[0] = EOL;
			return TRUE;
		}

		case IDB_BROWSE_LOCATION:
		{
			pCFullConnect->m_idSource = IDE_LOCATION;
			
			POINTS pts;
			GetWindowPos(GetDlgItem(IDB_BROWSE_LOCATION), &pts);

			//Display the Context Menu
			DisplayContextMenu( 
						m_hWnd,
						IDM_BROWSE_OPTION, 
						pts,
						m_hWnd
						);
			return TRUE;
		}

		case IDB_BROWSE_DATASOURCE:
		{
			pCFullConnect->m_idSource = IDE_DATASOURCE;
			
			POINTS pts;
			GetWindowPos(GetDlgItem(IDB_BROWSE_DATASOURCE), &pts);

			//Display the Context Menu
			DisplayContextMenu( 
						m_hWnd,
						IDM_BROWSE_OPTION, 
						pts,
						m_hWnd
						);
			return TRUE;
		}
		
		case IDM_BROWSE_FILE:
		{
			WCHAR* pwszTitle = NULL;
			WCHAR* pwszBuffer = NULL;
			
			switch(pCFullConnect->m_idSource)
			{
				case IDE_DATASOURCE:
				{
					pwszTitle = L"DBPROP_INIT_DATASOURCE";
					pwszBuffer = m_wszDataSource;
					break;
				}

				case IDE_LOCATION:
				{
					pwszTitle = L"DBPROP_INIT_LOCATION";
					pwszBuffer = m_wszLocation;
					break;
				}

				default:
					ASSERT(!"Unhandled Source!");
					break;
			}

			//Display Common Dialog to obtain DataSource...
			//This is for providers that take a filename/path for this property
			if(SUCCEEDED(BrowseOpenFileName(GetAppLite()->m_hInstance, m_hWnd, pwszTitle, pwszBuffer, MAX_NAME_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0")))
			{
				CEditBoxLite editSource;
				editSource.CreateIndirect(m_hWnd, pCFullConnect->m_idSource);

				//Just update value
				editSource.ReplaceAll(pwszBuffer);
			}
			return TRUE;
		}

		case IDM_BROWSE_ENUM:
		{
			DisplayDialog(IDD_BROWSE_ENUM, m_hWnd, CFullConnect::BrowseEnumeratorProc, (LPARAM)pCFullConnect);
			return TRUE;
		}
	};

	return FALSE;
}

/////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl)
{
	switch(wNotifyCode)
	{
		case CBN_SELCHANGE:
		{	
			if(OnSelChange(iID, hWndCtrl))
				return TRUE;
			break;
		}

		case CBN_DROPDOWN:
		{	
			if(OnDropDown(iID, hWndCtrl))
				return TRUE;
			break;
		}
	};

	//Otherwise Delegate
	return CWndLite::OnCommandNotify(wNotifyCode, iID, hWndCtrl);
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnDropDown
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnDropDown(INT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		case IDC_PROVIDER:
		{
			CWaitCursor waitCursor;

			//NOTE: The Provider Combo box is deferred
			//since it requires the Root Enumerator which is quite
			//expensive, so only populate the provider combo
			//when user presses the drop down, otherwise it defaults
			//to their saved provider or MSDASQL...
			RefreshEnum();
			return TRUE;
		}
	};

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider::OnSelChange
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::OnSelChange(INT iID, HWND hWndCtrl) 
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	switch(iID)
	{
		case IDC_CONFIG:
		{
			//Obtain the Selected Combo Text
			m_comboConfig.GetSelText(pCFullConnect->m_wszConfigName, MAX_NAME_LEN);
			
			//Now we need to update the FullConnect for this Config
			pCFullConnect->LoadDefaults();
			RefreshProvider();
			return TRUE;
		}

		case IDC_PROVIDER:
		{
			//Need to remove any previously set properties.
			//Specifically any Advanced properties, since this is a new configuration
			pCFullConnect->m_CPropSets.RemoveAll();
			
			//Save the newly selected provider
			GetProviderName();
			return TRUE;
		}
	};

	return FALSE;
}
						

////////////////////////////////////////////////////////////////
// CFullConnectProvider::GetProviderName
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnectProvider::GetProviderName()
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	ENUMINFO* pEnumInfo = &pCFullConnect->m_EnumInfo;

	//Obtain Provider from DropDown
	INDEX iIndex = m_comboProvider.GetCurSel();
	if(iIndex != CB_ERR)
	{
		//Since we have the CBS_SORT turned on.
		//we store the pointer ENUMINFO object in the item data
		ENUMINFO* pEnumInfoData = (ENUMINFO*)m_comboProvider.GetItemParam(iIndex);
		if(pEnumInfoData && (LPARAM)pEnumInfoData!=CB_ERR)
			memcpy(pEnumInfo, pEnumInfoData, sizeof(ENUMINFO));
	}
	else
	{
		//The user must have typed in a Provider Name directly
		//This may not map to a provider in the list, so assume the name is a ProgID/CLSID
		memset(pEnumInfo, 0, sizeof(ENUMINFO));
		m_comboProvider.GetWindowText(pEnumInfo->wszName, MAX_NAME_LEN);
		m_comboProvider.GetWindowText(pEnumInfo->wszParseName, MAX_NAME_LEN);
	}

	//Display Provider Info (Title of Window)
	wSendMessageFmt(::GetParent(m_hWnd), WM_SETTEXT, 0, L"Full Connect - %s", pEnumInfo->wszDescription[0] ? pEnumInfo->wszDescription : L"No Descripition Available");
	return S_OK;
}


////////////////////////////////////////////////////////////////
// CFullConnectProvider::RefreshEnum
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::RefreshEnum(BOOL fReconnect)
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	CMainWindow* pCMainWindow = pCFullConnect->m_pCMainWindow;
	ENUMINFO* pEnumInfo = &pCFullConnect->m_EnumInfo;

	//Remove all previous entries
	m_comboProvider.ResetContent();

	//Initialize the Enumerator to establish a list of 
	//Providers and Enumerators, if we don'thave them already
	if(pCFullConnect->m_pCEnumerator == NULL)
	{
		CEnumerator* pCRootEnum = pCMainWindow->m_pCRootEnumerator;

		//No need to check the HRESULT since CEnumerator is capable of instantiating
		//objects, even without the RootEnum...
		pCRootEnum->CreateEnumInfo(CLSID_OLEDB_ENUMERATOR, fReconnect);

		//Fill out the provider name combo box.
		for(ULONG i=0; i<pCRootEnum->m_cEnumInfo; i++)
		{
			ENUMINFO* pEnumInfo = &pCRootEnum->m_rgEnumInfo[i];
			
			//Determine the Image to use
			UINT iImage = IMAGE_QUESTION;
			if(pEnumInfo->eType == DBSOURCETYPE_DATASOURCE)
				iImage = IMAGE_NORMAL;
			else if(pEnumInfo->eType == DBSOURCETYPE_ENUMERATOR)
				iImage = IMAGE_OBJECTS;
			else if(pEnumInfo->eType == DBSOURCETYPE_BINDER)
				iImage = IMAGE_CHAPTER;
			else if(pEnumInfo->eType == DBSOURCETYPE_DATASOURCE_MDP)
				iImage = IMAGE_CUBE;
			
			//Add the name to the list
			//Since we have the CBS_SORT turned on.
			//we store the pointer ENUMINFO object in the item data
			m_comboProvider.InsertItem(-1/*iItem*/, pEnumInfo->wszName, (LPARAM)pEnumInfo, PARAM_NONE/*iIndent*/, iImage, iImage);
		}
	}

	//Try to find Previous Selection by default
	m_comboProvider.SetSelText(pEnumInfo->wszName, TRUE/*fAddItem*/);
	
	//Obtain Provider
	GetProviderName();
	return TRUE;
}

	

////////////////////////////////////////////////////////////////
// CFullConnectProvider::RefreshProvider
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::RefreshProvider()
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	//Try to find our Saved Provider
	m_comboProvider.SetSelText(pCFullConnect->m_EnumInfo.wszName, TRUE/*fAddItem*/);

	//Obtain Provider from DropDown
	GetProviderName();

	//Restore Saved Values - Properties
	wSendMessage(GetDlgItem(IDE_LOCATION),		WM_SETTEXT, 0, m_wszLocation);
	wSendMessage(GetDlgItem(IDE_DATASOURCE),	WM_SETTEXT, 0, m_wszDataSource);
	wSendMessage(GetDlgItem(IDE_USERID),		WM_SETTEXT, 0, m_wszUserID);
	wSendMessage(GetDlgItem(IDE_PASSWORD),		WM_SETTEXT, 0, m_wszPassword);

	//Set Prompt Combo
	CComboBoxLite comboPrompt(m_hWnd, IDC_PROMPT);
	comboPrompt.SetSelValue(m_dwPromptProp);

	//Try to find our Defaut Configuration
	m_comboConfig.SetSelText(pCFullConnect->m_wszConfigName, TRUE/*fAddItem*/);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnectProvider::UpdateProvider
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectProvider::UpdateProvider()
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	//Obtain Provider Selection
	GetProviderName();

	//Obtain Property Values
	wSendMessage(GetDlgItem(IDE_LOCATION),		WM_GETTEXT, MAX_NAME_LEN, m_wszLocation);
	wSendMessage(GetDlgItem(IDE_DATASOURCE),	WM_GETTEXT, MAX_NAME_LEN, m_wszDataSource);
	wSendMessage(GetDlgItem(IDE_USERID),		WM_GETTEXT, MAX_NAME_LEN, m_wszUserID);
	wSendMessage(GetDlgItem(IDE_PASSWORD),		WM_GETTEXT, MAX_NAME_LEN, m_wszPassword);
	
	//Obtain Prompt Value
	INDEX iSel = (INDEX)SendMessage(GetDlgItem(IDC_PROMPT), CB_GETCURSEL, 0, 0);
	m_dwPromptProp = (DWORD)SendMessage(GetDlgItem(IDC_PROMPT), CB_GETITEMDATA, iSel, 0);

	//Obtain Configuration Name
	m_comboConfig.GetSelText(pCFullConnect->m_wszConfigName, MAX_NAME_LEN);
	return TRUE;
}




/////////////////////////////////////////////////////////////////////////////
// CFullConnectProperties
//
/////////////////////////////////////////////////////////////////////////////
CFullConnectProperties::CFullConnectProperties()
	: CPropPageLite(IDD_FULLCONNECT_PROPERTIES, "Properties")
{
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectProperties::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProperties::OnInitDialog() 
{
	//PropertySheet (BackPointer)
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	CWaitCursor waitCursor;
	
	HWND hWndAsynch		= GetDlgItem(IDC_ASYNCH);
	HWND hWndLocale		= GetDlgItem(IDC_LOCALE);
	HWND hWndMode		= GetDlgItem(IDL_MODE);

	//DBPROP_INIT_ASYNCH
	const static WIDENAMEMAP rgAsynch[] =
	{
		VALUE_WCHAR(DBPROPVAL_ASYNCH_INITIALIZE),
	};

	//Fill in AsynchCombo
	pCFullConnect->InitPropCombo(hWndAsynch, NUMELE(rgAsynch), rgAsynch, m_dwAsynchProp);

	//Fill in LocaleCombo
	pCFullConnect->InitPropCombo(hWndLocale, g_cLCID, g_rgLCID, m_dwlcidProp);

	//DBPROP_INIT_MODE
	const static WIDENAMEMAP rgMode[] =
	{
		VALUE_WCHAR(DB_MODE_READ),
		VALUE_WCHAR(DB_MODE_WRITE),
		VALUE_WCHAR(DB_MODE_READWRITE),
		VALUE_WCHAR(DB_MODE_SHARE_DENY_READ),
		VALUE_WCHAR(DB_MODE_SHARE_DENY_WRITE),
		VALUE_WCHAR(DB_MODE_SHARE_EXCLUSIVE),
		VALUE_WCHAR(DB_MODE_SHARE_DENY_NONE),
	};

	//Fill in Mode ListBox
	SendMessage(hWndMode, LB_RESETCONTENT, 0, 0);
	for(ULONG i=0; i<NUMELE(rgMode); i++)
	{
		INDEX iSel = (INDEX)wSendMessage(hWndMode, LB_ADDSTRING, 0, rgMode[i].pwszName);
		SendMessage(hWndMode,	LB_SETITEMDATA,	iSel, (LPARAM)rgMode[i].lItem);
	}

	//Restore Saved Values
	RefreshProperties();
	
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProperties::OnSetActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProperties::OnSetActive() 
{
	RefreshProperties();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProperties::OnKillActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProperties::OnKillActive() 
{
	return UpdateProperties();
}


////////////////////////////////////////////////////////////////
// CFullConnectProperties::RefreshProperties
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectProperties::RefreshProperties()
{
	HWND hWndMode		= GetDlgItem(IDL_MODE);
	CComboBoxLite		comboAsynch(m_hWnd, IDC_ASYNCH);
	CComboBoxLite		comboLocale(m_hWnd, IDC_LOCALE);

	//Restore Saved Values - Properties
	wSendMessage(GetDlgItem(IDE_PROVSTRING),	WM_SETTEXT, 0, m_wszProvString);
	wSendMessage(GetDlgItem(IDC_CATALOG),		WM_SETTEXT, 0, m_wszCatalog);
	
	//Timeout
	if(m_dwTimeoutProp != NOTSET)
		wSendMessageFmt(GetDlgItem(IDE_TIMEOUT),	WM_SETTEXT, 0, L"%d", m_dwTimeoutProp);

	//Fill in AsynchCombo
	comboAsynch.SetSelValue(m_dwAsynchProp);

	//Fill in LocaleCombo
	comboLocale.SetSelValue(m_dwlcidProp);

	//Fill in Mode ListBox
	INDEX iCount = (INDEX)SendMessage(hWndMode, LB_GETCOUNT, 0, 0);
	for(LONG i=0; i<iCount; i++)
	{
		//Reselect Mode Options (turn it on or off...)
		LPARAM lParam = SendMessage(hWndMode,	LB_GETITEMDATA,	i, 0);
		
		if(m_dwModeProp == NOTSET)
			 SendMessage(hWndMode, LB_SETSEL, 0, i); //Turn off
		else
			 SendMessage(hWndMode, LB_SETSEL, (lParam & m_dwModeProp) == lParam, i);
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnectProperties::UpdateProperties
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectProperties::UpdateProperties()
{
	//Retrieve Values from Properties Sheet
	HWND hWndAsynch = GetDlgItem(IDC_ASYNCH);
	HWND hWndLocale = GetDlgItem(IDC_LOCALE);
	HWND hWndMode	= GetDlgItem(IDL_MODE);

	wSendMessage(GetDlgItem(IDE_PROVSTRING),	WM_GETTEXT, MAX_NAME_LEN, m_wszProvString);
	wSendMessage(GetDlgItem(IDC_CATALOG),		WM_GETTEXT, MAX_NAME_LEN, m_wszCatalog);
	
	//Obtain Asynch Value
	INDEX iSel = (INDEX)SendMessage(hWndAsynch, CB_GETCURSEL, 0, 0);
	m_dwAsynchProp = (DWORD)SendMessage(hWndAsynch, CB_GETITEMDATA, iSel, 0);

	//Obtain Locale Value
	iSel = (INDEX)SendMessage(hWndLocale, CB_GETCURSEL, 0, 0);
	m_dwlcidProp = (DWORD)SendMessage(hWndLocale, CB_GETITEMDATA, iSel, 0);

	//Timeout
	m_dwTimeoutProp = NOTSET;
	if(!GetEditBoxValue(GetDlgItem(IDE_TIMEOUT), &m_dwTimeoutProp, 0/*Min*/, LONG_MAX/*Max*/, TRUE))
	{
		m_dwTimeoutProp = NOTSET;
		return FALSE;
	}
	
	//Obtain all Mode Selected Items...
	INDEX iSelCount = (INDEX)SendMessage(hWndMode, LB_GETSELCOUNT, 0, 0);
	ASSERT(iSelCount < 20);
	LONG rgSelItems[20];
	SendMessage(hWndMode, LB_GETSELITEMS, (WPARAM)20, (LPARAM)rgSelItems);

	m_dwModeProp = iSelCount ? 0 : NOTSET;
	for(LONG i=0; i<iSelCount; i++)
	{
		m_dwModeProp |= SendMessage(hWndMode, LB_GETITEMDATA, rgSelItems[i], 0);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProperties::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectProperties::OnCommand(UINT iID, HWND hWndCtrl) 
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	switch(iID)
	{
		case IDB_MOREPROPERTIES:
		{
			pCFullConnect->GetAdvProperties();
			RefreshProperties();
			return TRUE;
		}

		case IDB_BROWSE_PROVSTRING:
		{
			pCFullConnect->m_idSource = IDE_PROVSTRING;
			
			POINTS pts;
			GetWindowPos(GetDlgItem(IDB_BROWSE_PROVSTRING), &pts);

			//Display the Context Menu
			DisplayContextMenu( 
						m_hWnd,
						IDM_BROWSE_OPTION, 
						pts,
						m_hWnd
						);
			return TRUE;
		}

		case IDM_BROWSE_FILE:
		{
			WCHAR* pwszTitle = NULL;
			WCHAR* pwszBuffer = NULL;
			
			switch(pCFullConnect->m_idSource)
			{
				case IDE_PROVSTRING:
				{
					pwszTitle = L"DBPROP_INIT_PROVIDERSTRING";
					pwszBuffer = m_wszProvString;
					break;
				}

				default:
					ASSERT(!"Unhandled Source!");
					break;
			}

			//Display Common Dialog to obtain DataSource...
			//This is for providers that take a filename/path for this property
			if(SUCCEEDED(BrowseOpenFileName(GetAppLite()->m_hInstance, m_hWnd, pwszTitle, pwszBuffer, MAX_NAME_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0")))
			{
				CEditBoxLite editSource;
				editSource.CreateIndirect(m_hWnd, pCFullConnect->m_idSource);

				//Just update value
				editSource.ReplaceAll(pwszBuffer);
			}
			return TRUE;
		}

		case IDM_BROWSE_ENUM:
		{
			DisplayDialog(IDD_BROWSE_ENUM, m_hWnd, CFullConnect::BrowseEnumeratorProc, (LPARAM)pCFullConnect);
			return TRUE;
		}
	};

	return FALSE;
}



/////////////////////////////////////////////////////////////////////////////
// CFullConnectSecurity
//
/////////////////////////////////////////////////////////////////////////////
CFullConnectSecurity::CFullConnectSecurity()
	: CPropPageLite(IDD_FULLCONNECT_SECURITY, "Security")
{
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectSecurity::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectSecurity::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//PropertySheet (BackPointer)
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	HWND hWndProtection = GetDlgItem(IDC_PROTECTION);
	HWND hWndImpersonation = GetDlgItem(IDC_IMPERSONATE);

	//DBPROP_INIT_PROTECTION_LEVEL
	const static WIDENAMEMAP rgProtection[] =
	{
		VALUE_WCHAR(DB_PROT_LEVEL_NONE),
		VALUE_WCHAR(DB_PROT_LEVEL_CONNECT),
		VALUE_WCHAR(DB_PROT_LEVEL_CALL),
		VALUE_WCHAR(DB_PROT_LEVEL_PKT),
		VALUE_WCHAR(DB_PROT_LEVEL_PKT_INTEGRITY),
		VALUE_WCHAR(DB_PROT_LEVEL_PKT_PRIVACY),
	};

	//Fill in ProtectionCombo
	pCFullConnect->InitPropCombo(hWndProtection, NUMELE(rgProtection), rgProtection, m_dwProtectionProp);

	//DBPROP_INIT_IMPERSONATION_LEVEL
	const static WIDENAMEMAP rgImpersonation[] =
	{
		VALUE_WCHAR(DB_IMP_LEVEL_ANONYMOUS),
		VALUE_WCHAR(DB_IMP_LEVEL_IDENTIFY),
		VALUE_WCHAR(DB_IMP_LEVEL_IMPERSONATE),
		VALUE_WCHAR(DB_IMP_LEVEL_DELEGATE),
	};

	//Fill in ImpersonationCombo
	pCFullConnect->InitPropCombo(hWndImpersonation, NUMELE(rgImpersonation), rgImpersonation, m_dwImpersonateProp);

	//Restore Saved Values
	RefreshSecurity();
	
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectSecurity::OnSetActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectSecurity::OnSetActive() 
{
	RefreshSecurity();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectSecurity::OnKillActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectSecurity::OnKillActive() 
{
	return UpdateSecurity();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectSecurity::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectSecurity::OnCommand(UINT iID, HWND hWndCtrl) 
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();

	switch(iID)
	{
		case IDB_BROWSE_INTEGRATED:
		{
			pCFullConnect->m_idSource = IDE_INTEGRATED;
			
			POINTS pts;
			GetWindowPos(GetDlgItem(IDB_BROWSE_INTEGRATED), &pts);

			//Display the Context Menu
			DisplayContextMenu( 
						m_hWnd,
						IDM_BROWSE_OPTION, 
						pts,
						m_hWnd
						);
			return TRUE;
		}

		case IDM_BROWSE_FILE:
		{
			WCHAR* pwszTitle = NULL;
			WCHAR* pwszBuffer = NULL;
			
			switch(pCFullConnect->m_idSource)
			{
				case IDE_INTEGRATED:
				{
					pwszTitle = L"DBPROP_AUTH_INTEGRATED";
					pwszBuffer = m_wszIntegrated;
					break;
				}

				default:
					ASSERT(!"Unhandled Source!");
					break;
			}

			//Display Common Dialog to obtain DataSource...
			//This is for providers that take a filename/path for this property
			if(SUCCEEDED(BrowseOpenFileName(GetAppLite()->m_hInstance, m_hWnd, pwszTitle, pwszBuffer, MAX_NAME_LEN, NULL, L"DataSource Files (.dsn;.kag;.sav)\0*.dsn;*.kag;*.sav;\0DataBase Files (.mdb;.db;.dbf)\0*.mdb;*.db;*.dbf;\0Program Files (.xls;.clb)\0*.xls;*.clb;\0Text Files (.txt;.csv)\0*.txt;*.csv\0All Files (*.*)\0*.*\0\0")))
			{
				CEditBoxLite editSource;
				editSource.CreateIndirect(m_hWnd, pCFullConnect->m_idSource);

				//Just update value
				editSource.ReplaceAll(pwszBuffer);
			}
			return TRUE;
		}

		case IDM_BROWSE_ENUM:
		{
			DisplayDialog(IDD_BROWSE_ENUM, m_hWnd, CFullConnect::BrowseEnumeratorProc, (LPARAM)pCFullConnect);
			return TRUE;
		}
	};

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CFullConnectSecurity::RefreshSecurity
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectSecurity::RefreshSecurity()
{
	CComboBoxLite			comboProtection(m_hWnd, IDC_PROTECTION);
	CComboBoxLite			comboImpersonation(m_hWnd, IDC_IMPERSONATE);

	//Fill in ProtectionCombo
	comboProtection.SetSelValue(m_dwProtectionProp);

	//Fill in ImpersonationCombo
	comboImpersonation.SetSelValue(m_dwImpersonateProp);

	//Restore Saved Values
	CheckDlgButton(IDB_MASKPASSWORD,		m_dwMaskPasswordProp == VARIANT_TRUE ? BST_CHECKED : m_dwMaskPasswordProp == VARIANT_FALSE ? BST_INDETERMINATE : BST_UNCHECKED);
	CheckDlgButton(IDB_ENCRYPTPASSWORD,		m_dwEncryptPasswordProp == VARIANT_TRUE ? BST_CHECKED : m_dwEncryptPasswordProp == VARIANT_FALSE ? BST_INDETERMINATE : BST_UNCHECKED);
	CheckDlgButton(IDB_CACHEINFO,			m_dwCacheProp == VARIANT_TRUE ? BST_CHECKED : m_dwCacheProp == VARIANT_FALSE ? BST_INDETERMINATE : BST_UNCHECKED);
	CheckDlgButton(IDB_PERSISTINFO,			m_dwPersistProp == VARIANT_TRUE ? BST_CHECKED : m_dwPersistProp == VARIANT_FALSE ? BST_INDETERMINATE : BST_UNCHECKED);
	CheckDlgButton(IDB_ENCRYPTINFO,			m_dwPersistEncryptProp == VARIANT_TRUE ? BST_CHECKED : m_dwPersistEncryptProp == VARIANT_FALSE ? BST_INDETERMINATE : BST_UNCHECKED);
	
	//Restore Save Values - Integrated
	wSendMessage(GetDlgItem(IDE_INTEGRATED),	WM_SETTEXT, 0, m_wszIntegrated);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnectSecurity::UpdateSecurity
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectSecurity::UpdateSecurity()
{
	//Retrieve Values from Security Sheet
	HWND hWndProtection = GetDlgItem(IDC_PROTECTION);
	HWND hWndImpersonate = GetDlgItem(IDC_IMPERSONATE);

	//Obtain Protection Value
	INDEX iSel = (INDEX)SendMessage(hWndProtection, CB_GETCURSEL, 0, 0);
	m_dwProtectionProp = (DWORD)SendMessage(hWndProtection, CB_GETITEMDATA, iSel, 0);

	//Obtain Impersonate Value
	iSel = (INDEX)SendMessage(hWndImpersonate, CB_GETCURSEL, 0, 0);
	m_dwImpersonateProp = (DWORD)SendMessage(hWndImpersonate, CB_GETITEMDATA, iSel, 0);

	//MaskPassword
	m_dwMaskPasswordProp = IsDlgButtonChecked(IDB_MASKPASSWORD) == BST_CHECKED ? VARIANT_TRUE : IsDlgButtonChecked(IDB_MASKPASSWORD) == BST_INDETERMINATE ? VARIANT_FALSE : NOTSET;

	//m_dwEncryptPasswordProp
	m_dwEncryptPasswordProp  = IsDlgButtonChecked(IDB_ENCRYPTPASSWORD) == BST_CHECKED ? VARIANT_TRUE : IsDlgButtonChecked(IDB_ENCRYPTPASSWORD) == BST_INDETERMINATE ? VARIANT_FALSE : NOTSET;

	//m_dwCacheProp
	m_dwCacheProp = IsDlgButtonChecked(IDB_CACHEINFO) == BST_CHECKED ? VARIANT_TRUE : IsDlgButtonChecked(IDB_CACHEINFO) == BST_INDETERMINATE ? VARIANT_FALSE : NOTSET;

	//m_dwPersistProp
	m_dwPersistProp = IsDlgButtonChecked(IDB_PERSISTINFO) == BST_CHECKED ? VARIANT_TRUE : IsDlgButtonChecked(IDB_PERSISTINFO) == BST_INDETERMINATE ? VARIANT_FALSE : NOTSET;

	//m_dwPersistEncryptProp
	m_dwPersistEncryptProp = IsDlgButtonChecked(IDB_ENCRYPTINFO) == BST_CHECKED ? VARIANT_TRUE : IsDlgButtonChecked(IDB_ENCRYPTINFO) == BST_INDETERMINATE ? VARIANT_FALSE : NOTSET;

	wSendMessage(GetDlgItem(IDE_INTEGRATED),	WM_GETTEXT, MAX_NAME_LEN, m_wszIntegrated);
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectOptions
//
/////////////////////////////////////////////////////////////////////////////
CFullConnectOptions::CFullConnectOptions()
	: CPropPageLite(IDD_FULLCONNECT_OPTIONS, "Options")
{
}

/////////////////////////////////////////////////////////////////////////////
// CFullConnectOptions::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectOptions::OnInitDialog() 
{
	CWaitCursor waitCursor;

	//Refresh Options
	RefreshOptions();
	
	//Delegate
	return CPropPageLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectOptions::OnSetActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectOptions::OnSetActive() 
{
	RefreshOptions();
	return TRUE;
}

	
/////////////////////////////////////////////////////////////////////////////
// CFullConnectOptions::OnKillActive
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectOptions::OnKillActive() 
{
	return UpdateOptions();
}


/////////////////////////////////////////////////////////////////////////////
// CFullConnectOptions::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CFullConnectOptions::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		ON_COMMAND(IDB_REMOTE_SERVER,	::EnableWindow(::GetDlgItem(m_hWnd, IDE_REMOTESERVER), ::IsDlgButtonChecked(m_hWnd, IDB_REMOTE_SERVER)))
	};

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CFullConnectOptions::RefreshOptions
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectOptions::RefreshOptions()
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	CMainWindow* pCMainWindow = pCFullConnect->m_pCMainWindow;

	//Set CLSCTX to previous saved values
	CheckDlgButton(IDB_INPROC_SERVER,				BST2STATE(m_dwCLSCTX & CLSCTX_INPROC_SERVER));
	CheckDlgButton(IDB_LOCAL_SERVER,				BST2STATE(m_dwCLSCTX & CLSCTX_LOCAL_SERVER));
	CheckDlgButton(IDB_REMOTE_SERVER,				BST2STATE(m_dwCLSCTX & CLSCTX_REMOTE_SERVER));
	CheckDlgButton(IDB_INPROC_HANDLER,				BST2STATE(m_dwCLSCTX & CLSCTX_INPROC_HANDLER));
	wSendMessage(GetDlgItem(IDE_REMOTESERVER),		WM_SETTEXT, 0, m_wszRemoteServer);
	::EnableWindow(GetDlgItem(IDE_REMOTESERVER),	m_dwCLSCTX & CLSCTX_REMOTE_SERVER);

	//Set Connection Options to previous saved values
	CheckDlgButton(IDB_INIT_INITIALIZE,				BST2STATE(m_dwConnectOpts & CREATE_INITIALIZE));
	CheckDlgButton(IDB_INIT_SETPROPERTIES,			BST2STATE(m_dwConnectOpts & CREATE_SETPROPERTIES));

	//NOTE: Currently these options are repeaded from the Global dialog (since we reuse it)
	//Just allow setting/unsetting global options here for convinence, but these are not 
	//saved per configuration, since all other objects after this point would need to reference this
	//making it difficult unless we did it for just connection QI's and the rest are based upon global
	//settings...
	CheckDlgButton(IDB_QI_MANDATORY,				BST2STATE(pCMainWindow->GetOptions()->m_dwCreateOpts & CREATE_QI_MANDATORY));
	CheckDlgButton(IDB_QI_OPTIONAL,					BST2STATE(pCMainWindow->GetOptions()->m_dwCreateOpts & CREATE_QI_OPTIONAL));
	
	//Set Other Options
	CheckDlgButton(IDB_USESERVICECOMP,				BST2STATE(m_dwConnectOpts & CREATE_USESERVICECOMP));
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnectOptions::UpdateOptions
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnectOptions::UpdateOptions()
{
	CFullConnect* pCFullConnect = (CFullConnect*)GetParent();
	CMainWindow* pCMainWindow = pCFullConnect->m_pCMainWindow;

	//Obtain CLSCTX
	m_dwCLSCTX = 0;
	if(IsDlgButtonChecked(IDB_INPROC_SERVER))
		m_dwCLSCTX |= CLSCTX_INPROC_SERVER;
	if(IsDlgButtonChecked(IDB_LOCAL_SERVER))
		m_dwCLSCTX |= CLSCTX_LOCAL_SERVER;
	if(IsDlgButtonChecked(IDB_REMOTE_SERVER))
		m_dwCLSCTX |= CLSCTX_REMOTE_SERVER;
	if(IsDlgButtonChecked(IDB_INPROC_HANDLER))
		m_dwCLSCTX |= CLSCTX_INPROC_HANDLER;

	//Obtain RemoteServer
	wSendMessage(GetDlgItem(IDE_REMOTESERVER), WM_GETTEXT, MAX_NAME_LEN, m_wszRemoteServer);

	//Obtain Connection Options
	m_dwConnectOpts = 0;
	if(IsDlgButtonChecked(IDB_INIT_INITIALIZE))
		m_dwConnectOpts |= CREATE_INITIALIZE;
	if(IsDlgButtonChecked(IDB_INIT_SETPROPERTIES))
		m_dwConnectOpts |= CREATE_SETPROPERTIES;

	//NOTE: Currently these options are repeaded from the Global dialog (since we reuse it)
	//Just allow setting/unsetting global options here for convinence, but these are not 
	//saved per configuration, since all other objects after this point would need to reference this
	//making it difficult unless we did it for just connection QI's and the rest are based upon global
	//settings...
	ENABLE_BIT(pCMainWindow->GetOptions()->m_dwCreateOpts, CREATE_QI_MANDATORY,	IsDlgButtonChecked(IDB_QI_MANDATORY));
	ENABLE_BIT(pCMainWindow->GetOptions()->m_dwCreateOpts, CREATE_QI_OPTIONAL,	IsDlgButtonChecked(IDB_QI_OPTIONAL));

	//Obtain Other Options
	if(IsDlgButtonChecked(IDB_USESERVICECOMP))
		m_dwConnectOpts |= CREATE_USESERVICECOMP;
	return TRUE;
}




/////////////////////////////////////////////////////////////////////////////
// CFullConnect
//
/////////////////////////////////////////////////////////////////////////////
CFullConnect::CFullConnect(CMainWindow* pCMainWindow)
	: CPropSheetLite("Full Connect"), CRowsetViewerDlg(pCMainWindow)
{
	AddPage(&m_CProvider);
	AddPage(&m_CProperties);
	AddPage(&m_CSecurity);
	AddPage(&m_COptions);

	WCHAR wszBuffer[MAX_NAME_LEN+1];
	ULONG cBytes = 0;

	//Enum
	m_pCEnumerator		= NULL;
	memset(&m_EnumInfo, 0, sizeof(ENUMINFO));
	StringCopy(m_EnumInfo.wszName,		L"MSDASQL", MAX_NAME_LEN);
	StringCopy(m_EnumInfo.wszParseName, L"MSDASQL", MAX_NAME_LEN);

	//Reset all Defaults
	ResetDefaults();

	//Load Configuration Name, so we know which configuration is the default
	GetRegEntry(HKEY_ROWSETVIEWER, wszCONFIG_KEY, L"DefaultConfig",	m_wszConfigName, MAX_NAME_LEN);

	//Load Recent Configurations
	ULONG i=0;
	for(i=0; i<MAX_RECENTCONFIGS; i++)
	{
		cBytes = sizeof(wszBuffer);
		if(S_OK != GetRegEnumValue(HKEY_ROWSETVIEWER, wszRECENTCONFIG_KEY, i, wszBuffer, &cBytes))
			break;
		
		m_listConfigs.AddTail(wcsDuplicate(wszBuffer));
	}

	//Load Recent File
	for(i=0; i<MAX_RECENTFILES; i++)
	{
		cBytes = sizeof(wszBuffer);
		if(S_OK != GetRegEnumValue(HKEY_ROWSETVIEWER, wszRECENTFILE_KEY, i, wszBuffer, &cBytes))
			break;
		
		m_listFiles.AddTail(wcsDuplicate(wszBuffer));
	}
}

	
////////////////////////////////////////////////////////////////
// CFullConnect::~CFullConnect
//
/////////////////////////////////////////////////////////////////
CFullConnect::~CFullConnect()
{
	//Save Recent Configs
	DelRegEntry(HKEY_ROWSETVIEWER, wszRECENTCONFIG_KEY);
	while(!m_listConfigs.IsEmpty())
	{
		WCHAR* pwszConfigName = m_listConfigs.RemoveHead();
		if(FAILED(SetRegEntry(HKEY_ROWSETVIEWER, wszRECENTCONFIG_KEY, pwszConfigName, MAX_NAME_LEN)))
			break;

		SAFE_FREE(pwszConfigName);
	}

	//Save Recent Files
	DelRegEntry(HKEY_ROWSETVIEWER, wszRECENTFILE_KEY);
	while(!m_listFiles.IsEmpty())
	{
		WCHAR* pwszFileName = m_listFiles.RemoveHead();
		if(FAILED(SetRegEntry(HKEY_ROWSETVIEWER, wszRECENTFILE_KEY, pwszFileName, MAX_NAME_LEN)))
			break;

		SAFE_FREE(pwszFileName);
	}
}


////////////////////////////////////////////////////////////////
// CFullConnect::ResetDefaults
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::ResetDefaults()
{
	//Configuration Name
	StringCopy(m_wszConfigName, L"(Default)", MAX_NAME_LEN);

	//Provider Page
	m_CProvider.m_wszLocation[0]		= wEOL;			//DBPROP_INIT_LOCATION
	m_CProvider.m_wszDataSource[0]		= wEOL;			//DBPROP_INIT_DATASOURCE
	m_CProvider.m_wszUserID[0]			= wEOL;			//DBPROP_AUTH_USERID
	m_CProvider.m_wszPassword[0]		= wEOL;			//DBPROP_AUTH_PASSWORD
	m_CProvider.m_dwPromptProp			= DBPROMPT_COMPLETE;
	m_CProvider.m_hWndProp				= (HWND)NOTSET;

	//Properties Page
	m_CProperties.m_wszProvString[0]	= wEOL;			//DBPROP_INIT_PROVIDERSTRING
	m_CProperties.m_wszCatalog[0]		= wEOL;			//DBPROP_INIT_CATALOG
	m_CProperties.m_dwlcidProp			= NOTSET;

	m_CProperties.m_dwAsynchProp		= NOTSET;
	m_CProperties.m_dwTimeoutProp		= NOTSET;
	m_CProperties.m_dwModeProp			= NOTSET;

	//Security Page
	m_CSecurity.m_dwProtectionProp		= NOTSET;
	m_CSecurity.m_dwImpersonateProp		= NOTSET;
	m_CSecurity.m_dwMaskPasswordProp	= NOTSET;
	m_CSecurity.m_dwEncryptPasswordProp	= NOTSET;
	m_CSecurity.m_dwCacheProp			= NOTSET;	
	m_CSecurity.m_dwPersistProp			= NOTSET;		
	m_CSecurity.m_dwPersistEncryptProp	= NOTSET;
	m_CSecurity.m_wszIntegrated[0]		= wEOL;			// DBPROP_INIT_INTEGRATED

	//Options Page
	m_COptions.m_dwCLSCTX				= CLSCTX_INPROC_SERVER;
	m_COptions.m_wszRemoteServer[0]		= wEOL;
	m_COptions.m_dwConnectOpts			= CREATE_INITIALIZE | CREATE_SETPROPERTIES;
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::LoadDefaults
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::LoadDefaults()
{
	//Need to remove any previously set properties.
	//Specifically any Advanced properties, since this is a new configuration
	m_CPropSets.RemoveAll();

	//Configuration Name
	if(m_wszConfigName[0] == wEOL)
		StringCopy(m_wszConfigName, L"(Default)", MAX_NAME_LEN);

	//Formulate the key
	WCHAR wszKeyName[MAX_NAME_LEN];
	StringFormat(wszKeyName, NUMELE(wszKeyName), L"%s\\%s", wszCONFIG_KEY, m_wszConfigName); 

	//Selected Provider
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Provider",				m_EnumInfo.wszName, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ParseName",			m_EnumInfo.wszParseName, MAX_NAME_LEN);

	//Provider Page
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Location",				m_CProvider.m_wszLocation, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"DataSource",			m_CProvider.m_wszDataSource, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"UserID",				m_CProvider.m_wszUserID, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Password",				m_CProvider.m_wszPassword, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"PromptProp",			(ULONG*)&m_CProvider.m_dwPromptProp);
	m_CProvider.m_hWndProp	= (HWND)NOTSET;

	//Properties Page
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ProvString",			m_CProperties.m_wszProvString, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Catalog",				m_CProperties.m_wszCatalog, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"lcidProp",				(ULONG*)&m_CProperties.m_dwlcidProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"AsynchProp",			(ULONG*)&m_CProperties.m_dwAsynchProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"TimeoutProp",			(ULONG*)&m_CProperties.m_dwTimeoutProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ModeProp",				(ULONG*)&m_CProperties.m_dwModeProp);

	//Security Page
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ProtectionProp",		(ULONG*)&m_CSecurity.m_dwProtectionProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ImpersonateProp",		(ULONG*)&m_CSecurity.m_dwImpersonateProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"MaskPasswordProp",		(ULONG*)&m_CSecurity.m_dwMaskPasswordProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"EncryptPasswordProp",	(ULONG*)&m_CSecurity.m_dwEncryptPasswordProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"CacheProp",			(ULONG*)&m_CSecurity.m_dwCacheProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"PersistProp",			(ULONG*)&m_CSecurity.m_dwPersistProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"PersistEncryptProp",	(ULONG*)&m_CSecurity.m_dwPersistEncryptProp);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Integrated",			m_CSecurity.m_wszIntegrated, MAX_NAME_LEN);

	//Options Page
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"CLSCTX",				&m_COptions.m_dwCLSCTX);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"RemoteServer",			m_COptions.m_wszRemoteServer, MAX_NAME_LEN);
	GetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ConnectOptions",		&m_COptions.m_dwConnectOpts);
	return TRUE;
}
	

////////////////////////////////////////////////////////////////
// CFullConnect::SaveDefaults
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::SaveDefaults()
{
	//Obtain the Configuration Name
	m_CProvider.m_comboConfig.GetSelText(m_wszConfigName, MAX_NAME_LEN);
	m_CProvider.m_comboConfig.SetSelText(m_wszConfigName, TRUE/*fAddItem*/);
					
	//Configuration Name
	if(m_wszConfigName[0] == wEOL)
		StringCopy(m_wszConfigName, L"(Default)", MAX_NAME_LEN);

	//Save Configuration Name, so we know which configuration is the default
	SetRegEntry(HKEY_ROWSETVIEWER, wszCONFIG_KEY, L"DefaultConfig", m_wszConfigName);

	//Formulate the key
	WCHAR wszKeyName[MAX_NAME_LEN];
	StringFormat(wszKeyName, NUMELE(wszKeyName), L"%s\\%s", wszCONFIG_KEY, m_wszConfigName); 

	//Selected Provider
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Provider",				m_EnumInfo.wszName);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ParseName",			m_EnumInfo.wszParseName);

	//Provider Page
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Location",				m_CProvider.m_wszLocation);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"DataSource",			m_CProvider.m_wszDataSource);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"UserID",				m_CProvider.m_wszUserID);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Password",				m_CProvider.m_wszPassword);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"PromptProp",			m_CProvider.m_dwPromptProp);
	
	//NOTE: The Window handle doesn't get persisted...
//	m_hWndProp	

	//Properties Page
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ProvString",			m_CProperties.m_wszProvString);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Catalog",				m_CProperties.m_wszCatalog);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"lcidProp",				m_CProperties.m_dwlcidProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"AsynchProp",			m_CProperties.m_dwAsynchProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"TimeoutProp",			m_CProperties.m_dwTimeoutProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ModeProp",				m_CProperties.m_dwModeProp);

	//Security Page
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ProtectionProp",		m_CSecurity.m_dwProtectionProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ImpersonateProp",		m_CSecurity.m_dwImpersonateProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"MaskPasswordProp",		m_CSecurity.m_dwMaskPasswordProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"EncryptPasswordProp",	m_CSecurity.m_dwEncryptPasswordProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"CacheProp",			m_CSecurity.m_dwCacheProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"PersistProp",			m_CSecurity.m_dwPersistProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"PersistEncryptProp",	m_CSecurity.m_dwPersistEncryptProp);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"Integrated",			m_CSecurity.m_wszIntegrated);

	//Options Page
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"CLSCTX",				m_COptions.m_dwCLSCTX);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"RemoteServer",			m_COptions.m_wszRemoteServer);
	SetRegEntry(HKEY_ROWSETVIEWER, wszKeyName, L"ConnectOptions",		m_COptions.m_dwConnectOpts);

	//Update the Saved Configurations (now that successfuly connected)
	AddRecentConfig();
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::LoadRecentConfig
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::LoadRecentConfig(ULONG iRecentConfig)
{
	ASSERT(iRecentConfig < m_listConfigs.GetCount());
	
	//Set the active config
	StringCopy(m_wszConfigName, m_listConfigs.GetAt(m_listConfigs.FindIndex(iRecentConfig)), MAX_NAME_LEN);
	
	//Load the saved properties and Connect
	LoadDefaults();
	FullConnect();
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::AddRecentConfig
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::AddRecentConfig(WCHAR* pwszConfigName)
{
	//Bascially the Alogythym for updating the Recent Configurations
	//is similar to a FIFO stack.  Lastest items become #1 and all other items
	//are moved down.  The only exception is if the item already exists
	//then it is brought up to number one and all other items are reordered...
	if(pwszConfigName == NULL)
		pwszConfigName = m_wszConfigName;

	//Make sure it doesn't already exist in the list
	RemoveRecentConfig(pwszConfigName);

	//Now add it to the list
	m_listConfigs.AddHead(wcsDuplicate(pwszConfigName));
	
	//Make sure the list is less than the Max
	if(m_listConfigs.GetCount() > MAX_RECENTCONFIGS)
	{
		pwszConfigName = m_listConfigs.RemoveTail();
		SAFE_FREE(pwszConfigName);
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::RemoveRecentConfig
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::RemoveRecentConfig(WCHAR* pwszConfigName)
{
	ASSERT(pwszConfigName);
	
	//Make sure it doesn't allready exist in the list
	POSITION pos = m_listConfigs.GetHeadPosition();
	while(pos)
	{
		POSITION posSave = pos;
		WCHAR* pwszName = m_listConfigs.GetNext(pos);
		if(StringCompare(pwszName, pwszConfigName))
		{
			//Remove this item and Add to the head
			pwszName = m_listConfigs.RemoveAt(posSave);
			SAFE_FREE(pwszName);
			return TRUE;
		}
	}

	return TRUE;
}

	
////////////////////////////////////////////////////////////////
// CFullConnect::LoadRecentFile
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::LoadRecentFile(ULONG iRecentFile)
{
	ASSERT(iRecentFile < m_listFiles.GetCount());

	//Obtain the Selected File
	WCHAR* pwszFileName = m_listFiles.GetAt(m_listFiles.FindIndex(iRecentFile));
	
	//Load the saved properties and Connect
	m_pCMainWindow->m_pCServiceComp->ConnectFromFile(pwszFileName);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::AddRecentFile
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::AddRecentFile(WCHAR* pwszFileName)
{
	//Bascially the Alogythym for updating the Recent Files
	//is similar to a FIFO stack.  Lastest items become #1 and all other items
	//are moved down.  The only exception is if the item already exists
	//then it is brought up to number one and all other items are reordered...
	ASSERT(pwszFileName);

	//Make sure it doesn't already exist in the list
	RemoveRecentFile(pwszFileName);

	//Now add it to the list
	m_listFiles.AddHead(wcsDuplicate(pwszFileName));
	
	//Make sure the list is less than the Max
	if(m_listFiles.GetCount() > MAX_RECENTFILES)	
	{
		pwszFileName = m_listFiles.RemoveTail();
		SAFE_FREE(pwszFileName);
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::RemoveRecentFile
//
/////////////////////////////////////////////////////////////////
BOOL CFullConnect::RemoveRecentFile(WCHAR* pwszFileName)
{
	ASSERT(pwszFileName);
	
	//Make sure it doesn't allready exist in the list
	POSITION pos = m_listFiles.GetHeadPosition();
	while(pos)
	{
		POSITION posSave = pos;
		WCHAR* pwszName = m_listFiles.GetNext(pos);
		if(StringCompare(pwszName, pwszFileName))
		{
			//Remove this item and Add to the head
			pwszName = m_listFiles.RemoveAt(posSave);
			SAFE_FREE(pwszName);
			return TRUE;
		}
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::Display
//
/////////////////////////////////////////////////////////////////
INT_PTR CFullConnect::Display(HWND hWndParent, CEnumerator* pCEnumerator, ENUMINFO* pEnumInfo)
{
	//If an Enumerator is passed in, it represents the enumerator to use for instanting the object...
	//Otherwise if NULL - it uses the Root Enumerator Object...
	m_pCEnumerator = pCEnumerator;
	if(pEnumInfo)
	{
		ResetDefaults();
		memcpy(&m_EnumInfo, pEnumInfo, sizeof(ENUMINFO));
	}
	else
	{
		//Load the defaults for this configuration
		LoadDefaults();
	}

	//Display the Property Sheet
    if(DoModal(hWndParent) >=0 )
	{
		//Now display focus to the new MDIChildWindow (if successful...)
		::SetFocus(::GetWindow(m_pCMainWindow->m_hWndMDIClient, GW_CHILD));
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CFullConnect::BrowseEnumeratorProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CFullConnect::BrowseEnumeratorProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{	
			CWaitCursor waitCursor;

			//save off the this pointer
			CFullConnect* pThis = (CFullConnect*)SetThis(hWnd, (void*)lParam);
			CEnumerator* pCRootEnum =  pThis->m_pCMainWindow->m_pCRootEnumerator;
			
			//Controls
			CComboBoxLite comboProvider(hWnd, IDC_PROVIDER);
			CListViewLite listviewEnum(hWnd, IDL_LISTVIEW);

			//We may need to connect to the RootEnumerator, if not done already
			pCRootEnum->CreateEnumInfo(CLSID_OLEDB_ENUMERATOR);
			
			//Setup ListView
			SendMessage(listviewEnum.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);
			listviewEnum.InsertColumn(0, L"SOURCES_NAME");
			
			//Set image list to the ListView
			ListView_SetImageList(listviewEnum.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

			//We just need to fill in a combo box with the same values
			//from the Root Enumerator, but only displaying Enumerators...
			for(ULONG i=0; i<pCRootEnum->m_cEnumInfo; i++)
			{
				if(pCRootEnum->m_rgEnumInfo[i].eType == DBSOURCETYPE_ENUMERATOR)
				{
					//Add the name to the list
					//Since we have the CBS_SORT turned on, the order in the Combo Box does
					//not match our array, so we pass the array index (lParam) as the item data
					comboProvider.AddString(pCRootEnum->m_rgEnumInfo[i].wszName, (LPARAM)&pCRootEnum->m_rgEnumInfo[i]);
				}
			}
		
			//Try to do a smart guess on which Enumerator.
			//Since the spec doesn't have a actual way of prgrammatically 
			//assoiciating providers with their enumerators, we will try and search
			//the enumerator name for the providers name...
			//So if using MSDASQL, it will find MSDASQL Enumerator
			INDEX iIndex = comboProvider.FindString(pThis->m_EnumInfo.wszName, -1);
			comboProvider.SetCurSel(iIndex!=CB_ERR ? iIndex : 0);

			//AutoSize Header
			listviewEnum.SetColumnWidth(0, LVSCW_AUTOSIZE_USEHEADER);

			//Send a selection change to the ComboBox so it updates everything
			SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDC_PROVIDER, hWnd, CBN_SELCHANGE));
			CenterDialog(hWnd);
			return TRUE;
		}

		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			//LBN_SELCHANGE ListBox Selection change
			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				//Selection change in a list box occurred
				case LBN_SELCHANGE:
				{	
					//Get the "this" pointer
					CFullConnect* pThis = (CFullConnect*)GetThis(hWnd);
					CEnumerator* pCRootEnum =  pThis->m_pCMainWindow->m_pCRootEnumerator;

					//Controls
					CComboBoxLite comboProvider(hWnd, IDC_PROVIDER);
					CListViewLite listviewEnum(hWnd, IDL_LISTVIEW);
					
					//The Selection has changed...
					listviewEnum.DeleteAllItems();
					listviewEnum.EnableWindow(FALSE);
					
					//Update the Window Title - with the Description
					INDEX iIndex = comboProvider.GetCurSel();
					ENUMINFO* pEnumInfo = (ENUMINFO*)comboProvider.GetItemParam(iIndex);

					if(pEnumInfo && (LPARAM)pEnumInfo!=CB_ERR)
						wSendMessage(hWnd, WM_SETTEXT, 0, pEnumInfo->wszDescription);
					return 0;
				}
			}
			
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_CONNECT:
				{
					//Get the "this" pointer
					CFullConnect* pThis = (CFullConnect*)GetThis(hWnd);

					//Fill in Enum Dialog
					pThis->DisplayBrowseEnumInfo(hWnd);
					return 0;
				}
								
				case IDOK:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CFullConnect* pThis = (CFullConnect*)GetThis(hWnd);
					WCHAR wszBuffer[MAX_NAME_LEN] = {0};

					//Controls
					CListViewLite listviewEnum(hWnd, IDL_LISTVIEW);
					INDEX iSelRow = listviewEnum.GetNextItem(-1, LVNI_SELECTED);
					
					//Need to obtain Enum selection from the ListView
					listviewEnum.GetItemText(iSelRow, 0, wszBuffer, MAX_NAME_LEN);

					//Now replace the entire edit box with the new selected value
					CEditBoxLite editSource;
					editSource.CreateIndirect(pThis->m_CProvider.m_hWnd, pThis->m_idSource);

					//Just update value
					editSource.ReplaceAll(wszBuffer, TRUE/*bReplaceAll*/);
					EndDialog(hWnd, TRUE);
					return 0;
				}

				case IDCANCEL:
				{
					EndDialog(hWnd, FALSE);
					return 0;
				}
			}
			break;
		}//WM_COMMAND

	
		case WM_NOTIFY:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)lParam;
			
			switch(pDispInfo->hdr.code)
			{
				//Since we have "TwoClickActive" on this will get sent
				//Whenever a row is clicked on twice!
				//This functionality used to be done with NM_DBCLK
				case LVN_ITEMACTIVATE:
				{
					//Send an OK message
					SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, hWnd, 0));
					return 0;
				}
			}
			break;
		}//WM_NOTIFY
	}

	return FALSE;   
}


////////////////////////////////////////////////////////////////
// CFullConnect::DisplayBrowseEnumInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnect::DisplayBrowseEnumInfo(HWND hWnd)
{
	HRESULT hr = S_OK;
	
	ULONG i = 0;
	ENUMINFO* rgEnumInfo = NULL;
	WCHAR wszBuffer[MAX_NAME_LEN];
	CComPtr<IParseDisplayName> spParseDisplayName;
	WCHAR* pwszParseName = NULL;
	CEnumerator* pCEnumerator = new CEnumerator(m_pCMainWindow);
	CBase* pCSource = NULL;

	//Controls
	CComboBoxLite comboProvider(hWnd, IDC_PROVIDER);
	CListViewLite listviewEnum(hWnd, IDL_LISTVIEW);

	//Clear Previous Items
	listviewEnum.DeleteAllItems();
	listviewEnum.EnableWindow(FALSE);

	//Obtain the Specified ParseName
	INDEX iSel = comboProvider.GetCurSel();
	if(iSel == CB_ERR)
	{
		wszBuffer[0] = wEOL;
		comboProvider.GetWindowText(wszBuffer, MAX_NAME_LEN);
		pwszParseName = wszBuffer;
	}
	else
	{
		ENUMINFO* pEnumInfo = (ENUMINFO*)comboProvider.GetItemParam(iSel);
		pwszParseName = pEnumInfo->wszParseName;
	}


	//Connect to the specified Enumerator
	TESTC(hr = m_pCMainWindow->m_pCRootEnumerator->ParseDisplayName(pwszParseName, CLSCTX_ALL, IID_IParseDisplayName, (IUnknown**)&spParseDisplayName, &pCSource));

	//Obtain the Enumerator Rowset
	TESTC(hr = pCEnumerator->CreateObject(pCSource, IID_IParseDisplayName, spParseDisplayName));
	TESTC(hr = pCEnumerator->CreateEnumInfo());
	
	//Now loop through the EnumInfo and Display in the ListView
	//SOURCES_NAME
	for(i=0; i<pCEnumerator->m_cEnumInfo; i++)
		listviewEnum.InsertItem(i, 0, pCEnumerator->m_rgEnumInfo[i].wszName);
	listviewEnum.EnableWindow(TRUE);

CLEANUP:
	SAFE_FREE(rgEnumInfo);
	SAFE_RELEASE(pCEnumerator);
	return hr;
}


////////////////////////////////////////////////////////////////
// CFullConnect::InitPropCombo
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnect::InitPropCombo(HWND hWndCombo, ULONG cItems, const WIDENAMEMAP* rgNameMap, LPARAM lParam)
{
	//Remove any existing Data
	SendMessage(hWndCombo,	CB_RESETCONTENT, 0, 0);
	INDEX iDefSel = 0;
	
	//First Item of every Property Combo is "No Set"
	INDEX iSel = (INDEX)SendMessage(hWndCombo, CB_ADDSTRING, 0, (LPARAM)"Not Set");
	SendMessage(hWndCombo,	CB_SETITEMDATA,	iSel, (LPARAM)NOTSET);
	
	//Fill in all Combo values...
	for(ULONG i=0; i<cItems; i++)
	{
		iSel = (INDEX)wSendMessage(hWndCombo, CB_ADDSTRING, 0, rgNameMap[i].pwszName);
		SendMessage(hWndCombo, CB_SETITEMDATA,	iSel, (LPARAM)rgNameMap[i].lItem);

		if(rgNameMap[i].lItem == lParam)
			iDefSel = iSel;
	}
	
	//Set the specified Default
	SendMessage(hWndCombo,	CB_SETCURSEL, iDefSel, 0);
	return S_OK;
}


////////////////////////////////////////////////////////////////
// CFullConnect::GetAdvProperties
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnect::GetAdvProperties()
{
	HRESULT hr = S_OK;
	CComPtr<IDBProperties> spDBProperties;
	ULONG iPropSet = 0;
	CPropertiesDlg sCPropertiesDlg(m_pCMainWindow);

	//Obtain instance of Provider/Enum (if we haven't done so already)
	TESTC(hr = m_CProvider.GetProviderName());
	TESTC(hr = CreateProviderInstance(IID_IDBProperties, (IUnknown**)&spDBProperties));

	//GetProperties (incase we haven't already)
	TESTC(hr = GetPropSets());
	
	//Bring up the SetProperties Dialog
	TESTC(hr = sCPropertiesDlg.SetProperties(m_CProperties.m_hWnd, &DBPROPSET_DBINITALL, IID_IDBProperties, spDBProperties, spDBProperties, &m_CPropSets));

	//Now read just our saved values...
	//Add update Dialogs, we only need to update values our dialogs know
	//about, all other properties are just saved in the set and added later...
	for(iPropSet=0; iPropSet<m_CPropSets.GetCount(); iPropSet++)
	{
		const DBPROPSET* pPropSet = &m_CPropSets[iPropSet];
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			if(pPropSet->guidPropertySet == DBPROPSET_DBINIT)
			{
				switch(pProp->dwPropertyID)
				{
					//Provider Page
					case DBPROP_INIT_LOCATION:
						VariantToString(&pProp->vValue, m_CProvider.m_wszLocation, MAX_NAME_LEN, CONV_VARBOOL);
						break;

					case DBPROP_INIT_DATASOURCE:
						VariantToString(&pProp->vValue, m_CProvider.m_wszDataSource, MAX_NAME_LEN, CONV_VARBOOL);
						break;

					case DBPROP_AUTH_USERID:
						VariantToString(&pProp->vValue, m_CProvider.m_wszUserID, MAX_NAME_LEN, CONV_VARBOOL);
						break;

					case DBPROP_AUTH_PASSWORD:
						VariantToString(&pProp->vValue, m_CProvider.m_wszPassword, MAX_NAME_LEN, CONV_VARBOOL);
						break;

					case DBPROP_INIT_PROMPT:
						m_CProvider.m_dwPromptProp = V_I2(&pProp->vValue);
						break;

					case DBPROP_INIT_HWND:
						//Note: This works for 32bit as well as 64bit values (since HWND varies).
						//Reason, the variant is a union, so all values are placed into the same 
						//starting location, so we simply take the adress and cast to the type.  
						m_CProvider.m_hWndProp		= *(HWND*)&V_I4(&pProp->vValue);
						break;
					
					//Propeerties Page
					case DBPROP_INIT_PROVIDERSTRING:
						VariantToString(&pProp->vValue, m_CProperties.m_wszProvString, MAX_NAME_LEN, CONV_VARBOOL);
						break;

					case DBPROP_INIT_CATALOG:
						VariantToString(&pProp->vValue, m_CProperties.m_wszCatalog, MAX_NAME_LEN, CONV_VARBOOL);
						break;

					case DBPROP_INIT_LCID:
						m_CProperties.m_dwlcidProp = V_I4(&pProp->vValue);
						break;
					
					case DBPROP_INIT_ASYNCH:
						m_CProperties.m_dwAsynchProp = V_I4(&pProp->vValue);
						break;
					
					case DBPROP_INIT_TIMEOUT:
						m_CProperties.m_dwTimeoutProp = V_I4(&pProp->vValue);
						break;
			
					case DBPROP_INIT_MODE:
						m_CProperties.m_dwModeProp = V_I4(&pProp->vValue);
						break;
					
					//Security Page
					case DBPROP_INIT_PROTECTION_LEVEL:
						m_CSecurity.m_dwProtectionProp = V_I4(&pProp->vValue);
						break;

					case DBPROP_INIT_IMPERSONATION_LEVEL:
						m_CSecurity.m_dwImpersonateProp = V_I4(&pProp->vValue);
						break;

					case DBPROP_AUTH_MASK_PASSWORD:
						m_CSecurity.m_dwMaskPasswordProp = V_BOOL(&pProp->vValue);
						break;

					case DBPROP_AUTH_ENCRYPT_PASSWORD:
						m_CSecurity.m_dwEncryptPasswordProp = V_BOOL(&pProp->vValue);
						break;

					case DBPROP_AUTH_CACHE_AUTHINFO:
						m_CSecurity.m_dwCacheProp = V_BOOL(&pProp->vValue);
						break;

					case DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO:
						m_CSecurity.m_dwPersistProp = V_BOOL(&pProp->vValue);
						break;

					case DBPROP_AUTH_PERSIST_ENCRYPTED:
						m_CSecurity.m_dwPersistEncryptProp = V_BOOL(&pProp->vValue);
						break;

					case DBPROP_AUTH_INTEGRATED:
						VariantToString(&pProp->vValue, m_CSecurity.m_wszIntegrated, MAX_NAME_LEN, CONV_VARBOOL);
						break;
				}
			}
		}
	};

CLEANUP:
	return hr;
}



////////////////////////////////////////////////////////////////
// CFullConnect::CreateProviderInstance
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnect::CreateProviderInstance(REFIID riid, IUnknown** ppIUnknown, CBase** ppCSource)
{
	//Which Enumerator, Root Enum or passed in enumerator...
	CEnumerator* pCEnumerator				= m_pCEnumerator ? m_pCEnumerator : m_pCMainWindow->m_pCRootEnumerator;
	HRESULT hr = S_OK;

	//Delegate out the Enumerator to instansiate this object
	TESTC(hr = pCEnumerator->ParseDisplayName(m_EnumInfo.wszParseName, m_COptions.m_dwCLSCTX, riid, ppIUnknown, ppCSource, m_COptions.m_dwConnectOpts, m_COptions.m_wszRemoteServer));

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CFullConnect::GetPropSets
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnect::GetPropSets()
{
	HRESULT hr = S_OK;

	//Free Previous Properties
	//This method gets called whenever connect gets called.
	//So the user may or may not have called 'More', which would have already 
	//Obtained these properties.  Also the user may have updated the dialogs
	//afterwards, so bascially it boils down to the fact we need to "reget" all
	//Properties that are in the dialog, and save all the prev properties (More)
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	m_CPropSets.Detach(&cPropSets, &rgPropSets);

	//Setup Properties from Provider Page	
	if(m_CProvider.m_wszLocation[0])
		m_CPropSets.SetProperty(DBPROP_INIT_LOCATION,		DBPROPSET_DBINIT, DBTYPE_BSTR, m_CProvider.m_wszLocation);
	if(m_CProvider.m_wszDataSource[0])
		m_CPropSets.SetProperty(DBPROP_INIT_DATASOURCE,		DBPROPSET_DBINIT, DBTYPE_BSTR, m_CProvider.m_wszDataSource);
	if(m_CProvider.m_wszUserID[0])
		m_CPropSets.SetProperty(DBPROP_AUTH_USERID,			DBPROPSET_DBINIT, DBTYPE_BSTR, m_CProvider.m_wszUserID);
	if(m_CProvider.m_wszPassword[0])
		m_CPropSets.SetProperty(DBPROP_AUTH_PASSWORD,		DBPROPSET_DBINIT, DBTYPE_BSTR, m_CProvider.m_wszPassword);
	if(m_CProvider.m_dwPromptProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_PROMPT,			DBPROPSET_DBINIT, DBTYPE_I2,	(void*)(LONG_PTR)m_CProvider.m_dwPromptProp);
	
	//By default we will set HWND if the user has requested a prompt value (except noprompt), or 
	//the user has explicitly set the hwnd themselves in the Advanced Properties...
	if(m_CProvider.m_hWndProp != (HWND)NOTSET || (m_CProvider.m_dwPromptProp != NOTSET && m_CProvider.m_dwPromptProp != DBPROMPT_NOPROMPT))
	{
		//Setup our "dynamic" window handle property...
		//NOTE: We have to do this here, since this needs to not only be a valid window handle,
		//must it also be the full connect dialog handle, so that if the provider displays a prompt
		//dialog its "modal" to this one and not the main window...

		//Note: On 64bit HWND is I8 instead of I4
		DBTYPE wType = (sizeof(HWND)==8) ? DBTYPE_I8 : DBTYPE_I4;
		HWND hWnd	= (m_CProvider.m_hWndProp != (HWND)NOTSET) ? m_CProvider.m_hWndProp : m_CProvider.m_hWnd;
		m_CPropSets.SetProperty(DBPROP_INIT_HWND,			DBPROPSET_DBINIT, wType, (void*)hWnd);
	}

	//Setup Properties from Properties Page
	if(m_CProperties.m_wszProvString[0])
		m_CPropSets.SetProperty(DBPROP_INIT_PROVIDERSTRING,	DBPROPSET_DBINIT, DBTYPE_BSTR, m_CProperties.m_wszProvString);
	if(m_CProperties.m_wszCatalog[0])
		m_CPropSets.SetProperty(DBPROP_INIT_CATALOG,		DBPROPSET_DBINIT, DBTYPE_BSTR, m_CProperties.m_wszCatalog);
	if(m_CProperties.m_dwTimeoutProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_TIMEOUT,		DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(LONG_PTR)m_CProperties.m_dwTimeoutProp);
	if(m_CProperties.m_dwAsynchProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_ASYNCH,			DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(LONG_PTR)m_CProperties.m_dwAsynchProp);
	if(m_CProperties.m_dwlcidProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_LCID,			DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(LONG_PTR)m_CProperties.m_dwlcidProp);
	if(m_CProperties.m_dwModeProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_MODE,			DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(LONG_PTR)m_CProperties.m_dwModeProp);

	//Setup Properties from Security Page
	if(m_CSecurity.m_dwProtectionProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_PROTECTION_LEVEL,DBPROPSET_DBINIT, DBTYPE_I4,		(void*)(LONG_PTR)m_CSecurity.m_dwProtectionProp);
	if(m_CSecurity.m_dwImpersonateProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_INIT_IMPERSONATION_LEVEL,DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(LONG_PTR)m_CSecurity.m_dwImpersonateProp);
	if(m_CSecurity.m_dwMaskPasswordProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_AUTH_MASK_PASSWORD,	DBPROPSET_DBINIT, DBTYPE_BOOL,		(void*)(LONG_PTR)m_CSecurity.m_dwMaskPasswordProp);
	if(m_CSecurity.m_dwEncryptPasswordProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_AUTH_ENCRYPT_PASSWORD,DBPROPSET_DBINIT, DBTYPE_BOOL,	(void*)(LONG_PTR)m_CSecurity.m_dwEncryptPasswordProp);
	if(m_CSecurity.m_dwCacheProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_AUTH_CACHE_AUTHINFO,	DBPROPSET_DBINIT, DBTYPE_BOOL,		(void*)(LONG_PTR)m_CSecurity.m_dwCacheProp);
	if(m_CSecurity.m_dwPersistProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO,	DBPROPSET_DBINIT, DBTYPE_BOOL,	(void*)(LONG_PTR)m_CSecurity.m_dwPersistProp);
	if(m_CSecurity.m_dwPersistEncryptProp != NOTSET)
		m_CPropSets.SetProperty(DBPROP_AUTH_PERSIST_ENCRYPTED,	DBPROPSET_DBINIT, DBTYPE_BOOL,	(void*)(LONG_PTR)m_CSecurity.m_dwPersistEncryptProp);
	if(m_CSecurity.m_wszIntegrated[0])
		m_CPropSets.SetProperty(DBPROP_AUTH_INTEGRATED,		DBPROPSET_DBINIT, DBTYPE_BSTR,		m_CSecurity.m_wszIntegrated);

	//Loop over all the Advanced Properties set,
	//And adjust any of the existing property options or type.
	//Always use the "latest" value specified in the dialogs, but the Advncaed
	//is the only way to set the type or options (REQUIRED/OPTIONAL)...
	
	//If there are properties that exist out side of the ones avilable in the 
	//dialogs, jusat set it, it may be an provider specific property,
	//or a newly added one to the group...
	for(ULONG iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			if(pPropSet->guidPropertySet != DBPROPSET_DBINIT)
			{
				//Otherwise this is not a settable property through the 
				//Dialogs, so just add the property, since it may be a 
				//provider specific property or newly added to DBINIT set...
				m_CPropSets.SetProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, DBTYPE_VARIANT, &pProp->vValue, pProp->dwOptions);
				break;
			}
			else
			{
				//DBPROPSET_DBINIT - Probably alread included from the dialogs
				switch(pProp->dwPropertyID)
				{
					//Provider Page
					case DBPROP_INIT_LOCATION:
					case DBPROP_INIT_DATASOURCE:
					case DBPROP_AUTH_USERID:
					case DBPROP_AUTH_PASSWORD:
					case DBPROP_INIT_PROMPT:
					case DBPROP_INIT_HWND:
					
					//Propeerties Page
					case DBPROP_INIT_PROVIDERSTRING:
					case DBPROP_INIT_CATALOG:
					case DBPROP_INIT_LCID:
					case DBPROP_INIT_ASYNCH:
					case DBPROP_INIT_TIMEOUT:
					case DBPROP_INIT_MODE:
					
					//Security Page
					case DBPROP_INIT_PROTECTION_LEVEL:
					case DBPROP_INIT_IMPERSONATION_LEVEL:
					case DBPROP_AUTH_MASK_PASSWORD:
					case DBPROP_AUTH_ENCRYPT_PASSWORD:
					case DBPROP_AUTH_CACHE_AUTHINFO:
					case DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO:
					case DBPROP_AUTH_PERSIST_ENCRYPTED:
					case DBPROP_AUTH_INTEGRATED:
					{	
						//All off these properties are available through the Dialogs
						//But not all the options and types are.  So if this property
						//exists in the current set, then adjust any options and types...
						//Since these are only exposed in the Advnaced page...
						
						//Find this property in the current set...
						DBPROP* pPropFound = m_CPropSets.FindProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, pProp->colid);
						if(pPropFound)
						{
							//Override any options - from advanced
							pPropFound->dwOptions = pProp->dwOptions;
							//Override any wType - from advanced
							pPropFound->vValue.vt = pProp->vValue.vt;
							//Override any colid - from advanced
							//memcpy(pProp->colid, pProp->colid, sizeof(DBID));
						}
						else
						{
							//It may not have been found in the current set
							//since the dialog may have been Empty, ("").  Its hard
							//to determine what empty means in a edit box, does it mean
							//Empty String, NULL, not set, or VT_EMPTY.
							//If this property exists in the Advanced set this is where
							//we will obtain this information...

							//The Advanced Page will always override the edit boxes...
							if(pProp->vValue.vt == VT_EMPTY || (pProp->vValue.vt == VT_BSTR && pProp->vValue.bstrVal && !pProp->vValue.bstrVal[0]))
								m_CPropSets.SetProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, DBTYPE_VARIANT, &pProp->vValue, pProp->dwOptions);
						}
						break;
					}
					
					default:
					{
						//Otherwise this is not a settable property through the 
						//Dialogs, so just add the property, since it may be a 
						//provider specific property or newly added to DBINIT set...
						m_CPropSets.SetProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, DBTYPE_VARIANT, &pProp->vValue, pProp->dwOptions);
						break;
					}
				}
			}
		}
	};

	FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}

/////////////////////////////////////////////////////////////////
// CFullConnect::FullConnect
//
/////////////////////////////////////////////////////////////////
HRESULT CFullConnect::FullConnect()
{
	CWaitCursor waitCursor;
	HRESULT hr = S_OK;
	CBase* pCSource = NULL;
	CBase* pCObject = NULL;
	CComPtr<IUnknown> spUnknown;

	//Obtain instance of Provider/Enum (if haven't done so already)...
	TESTC(hr = CreateProviderInstance(IID_IUnknown, &spUnknown, &pCSource));

	//Setup PropSets from all saved options
	TESTC(hr = GetPropSets());

	//Connect using the specified properties
	//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
	pCObject = m_pCMainWindow->HandleObjectType(pCSource, spUnknown, IID_IUnknown, eCDataSource, m_CPropSets.GetCount(), m_CPropSets.GetPropSets(), CREATE_NEWWINDOW | CREATE_DETERMINE_TYPE | m_COptions.m_dwConnectOpts | CREATE_NODISPLAY);
	if(!pCObject)
		TESTC(hr = E_FAIL);

	//Update the Object...
	pCObject->m_dwCLSCTX = m_COptions.m_dwCLSCTX;
	if(pCObject->m_pCMDIChild)
		pCObject->m_pCMDIChild->SetConfig(m_wszConfigName);

	//Now Display the object...
	pCObject->DisplayObject();


CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::CPropertiesDlg
//
/////////////////////////////////////////////////////////////////
CPropertiesDlg::CPropertiesDlg(CMainWindow* pCMainWindow)
	: CDialogLite(IDD_SETPROPERTIES), CRowsetViewerDlg(pCMainWindow)
{
	m_pIUnkProperties		= NULL;
	m_pIDBPropertyInfo		= NULL;
	m_pIDBDataSourceAdmin	= NULL;

	m_pCPropSets			= NULL;

	//Save State
	m_cPropInfoSets			= 0;
	m_rgPropInfoSets		= NULL;
	m_pStringBuffer			= NULL;
	m_bClearAll				= TRUE;
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::~CPropertiesDlg
//
/////////////////////////////////////////////////////////////////
CPropertiesDlg::~CPropertiesDlg()
{
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::SetProperties
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::SetProperties(HWND hWndParent, const GUID* pGuidPropertySet, REFIID riidProp, IUnknown* pIUnkProperties, IDBProperties* pIDBPropertyInfo, CPropSets* pCPropSets, IDBDataSourceAdmin* pIDBDataSourceAdmin)
{
//	NOTE: We can have valid causes where there is not properties interface passed in.
//	When OpenRowset is called there is no method to obtain current property values, other than the 
//	passed in pcPropSets, which also may be empty is no properties are set...
//	ASSERT(pIUnkProperties);
	
	//Setup variable for StaticProcs to use...
	m_iidProp				= riidProp;
	m_pGuidPropertySet		= pGuidPropertySet;
	
	m_pIUnkProperties		= pIUnkProperties;
	m_pIDBPropertyInfo		= pIDBPropertyInfo;
	m_pIDBDataSourceAdmin	= pIDBDataSourceAdmin;

	m_pCPropSets			= pCPropSets;
	m_eMethodType			= METHOD_SETPROPERTIES;

	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::GetProperties
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::GetProperties(HWND hWndParent, const GUID* pGuidPropertySet, REFIID riidProp, IUnknown* pIUnkProperties, IDBProperties* pIDBPropertyInfo, CPropSets* pCPropSets, IDBDataSourceAdmin* pIDBDataSourceAdmin)
{
	ASSERT(pIUnkProperties);

	//Setup variable for StaticProcs to use...
	m_iidProp				= riidProp;
	m_pGuidPropertySet		= pGuidPropertySet;
	
	m_pIUnkProperties		= pIUnkProperties;
	m_pIDBPropertyInfo		= pIDBPropertyInfo;
	m_pIDBDataSourceAdmin	= pIDBDataSourceAdmin;

	m_pCPropSets			= pCPropSets;
	m_eMethodType			= METHOD_GETPROPERTIES;

	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::GetPropertyInfo
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::GetPropertyInfo(HWND hWndParent, const GUID* pGuidPropertySet, REFIID riidProp, IDBProperties* pIDBPropertyInfo, IDBDataSourceAdmin* pIDBDataSourceAdmin)
{
	ASSERT(pIDBPropertyInfo || pIDBDataSourceAdmin);

	//Setup variable for StaticProcs to use...
	m_iidProp				= riidProp;
	m_pGuidPropertySet		= pGuidPropertySet;
	
	m_pIUnkProperties		= NULL;
	m_pIDBPropertyInfo		= pIDBPropertyInfo;
	m_pIDBDataSourceAdmin	= pIDBDataSourceAdmin;

	m_pCPropSets			= NULL;
	m_eMethodType			= METHOD_GETPROPERTYINFO;

	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Setup Controls
	m_listviewProps.CreateIndirect(m_hWnd, IDL_PROPERTY);
	m_comboPropSet.CreateIndirect(m_hWnd, IDC_PROPSET);

	//Header (change controls)
	m_comboOptions.CreateIndirect(m_hWnd, IDC_PROPEDIT_OPTIONS);
	m_comboType.CreateIndirect(m_hWnd, IDC_PROPEDIT_TYPE);
	m_comboValue.CreateIndirect(m_hWnd, IDC_PROPEDIT_VALUE);
	m_editDesc.CreateIndirect(m_hWnd, IDE_PROPEDIT_NAME);

	//Use Extended ListView Controls...
	SendMessage(m_listviewProps.m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES);

	//ColumnHeaders
	m_listviewProps.InsertColumn(PROP_NAME,		L"Property");
	m_listviewProps.InsertColumn(PROP_TYPE,		L"Type");
	m_listviewProps.InsertColumn(PROP_VALUE,	L"Value");
	m_listviewProps.InsertColumn(PROP_FLAGS,	m_eMethodType == METHOD_GETPROPERTYINFO ? L"Flags" : L"Options");
	m_listviewProps.InsertColumn(PROP_DESC,		L"Description");
	if(m_eMethodType != METHOD_GETPROPERTYINFO)
	{
		m_listviewProps.InsertColumn(PROP_COLID,	L"ColID");
	}
	
	//Set image list to the ListView
	ListView_SetImageList(m_listviewProps.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);
	ListView_SetImageList(m_listviewProps.m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_STATE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_STATE);

	//Default PropertySet
	IID iidProp = m_iidProp;

	//Now adjust display and find required PropSet
	if(iidProp == IID_IDBProperties)
	{
		if(m_eMethodType == METHOD_SETPROPERTIES)
			SetWindowText(L"IDBProperties::SetProperties");
		else if(m_eMethodType == METHOD_GETPROPERTIES)
			SetWindowText(L"IDBProperties::GetProperties");
		else 
			SetWindowText(L"IDBProperties::GetPropertyInfo");
	}
	else if(iidProp == IID_IDBDataSourceAdmin)
	{
		if(m_eMethodType == METHOD_SETPROPERTIES)
			SetWindowText(L"IDBDataSourceAdmin::ModifyDataSource");
		else
			SetWindowText(L"IDBDataSourceAdmin::GetCreationProperties");
	}
	else if(iidProp == IID_ISessionProperties)
	{
		if(m_eMethodType == METHOD_SETPROPERTIES)
			SetWindowText(L"ISessionProperties::SetProperties");
		else
			SetWindowText(L"ISessionProperties::GetProperties");
	}
	else if(iidProp == IID_ICommandProperties)
	{
		if(m_eMethodType == METHOD_SETPROPERTIES)
			SetWindowText(L"ICommandProperties::SetProperties");
		else
			SetWindowText(L"ICommandProperties::GetProperties");
	}
	else if(iidProp == IID_IRowsetInfo)
	{
		if(m_eMethodType == METHOD_SETPROPERTIES)
			SetWindowText(L"Set Properties for rgPropSet params");
		else
			SetWindowText(L"IRowsetInfo::GetProperties");
	}
	else
	{
		ASSERT(!"Unhandled Property Type!");
	}
	

	//Fill in the PROPSET Combo
	for(ULONG i=0; i<g_cPropSetMaps; i++)
	{
		INDEX iSel = m_comboPropSet.AddString(g_rgPropSetMaps[i].pwszName, i);
		if(m_eMethodType == METHOD_SETPROPERTIES)
			if(m_pGuidPropertySet && *m_pGuidPropertySet == *(g_rgPropSetMaps[i].pGuid))
				m_comboPropSet.SetCurSel(iSel);
	}

	//Add DBPROPSET All (0, NULL)...
	INDEX iSel = m_comboPropSet.AddString(L"All Properties", CB_ERR);

	//Default Selection
	iSel = m_comboPropSet.GetCurSel();
	m_comboPropSet.SetCurSel(iSel==CB_ERR || (ULONG)iSel>g_cPropSetMaps ? g_cPropSetMaps : iSel);
	
	//Fill in Types Combo
	m_comboType.Populate(g_cVariantTypes, g_rgVariantTypes);

	//Fill in Options Combo
	m_comboOptions.AddString(L"REQUIRED");	// == 0
	m_comboOptions.AddString(L"OPTIONAL");	// == 1

	//Include OLE DB Defined Properties
	CheckDlgButton(IDB_ADDDEFINEDPROPS,		FALSE/*Default*/);
	m_bClearAll = TRUE;

	//Display properties for just this set...
	const ULONG cPropertyIDSets = 1;
	DBPROPIDSET rgPropertyIDSets[cPropertyIDSets];
	if(m_pGuidPropertySet)
	{
		rgPropertyIDSets[0].cPropertyIDs = 0;
		rgPropertyIDSets[0].rgPropertyIDs = NULL;
		rgPropertyIDSets[0].guidPropertySet = *m_pGuidPropertySet;
	}

	//Do we limit the initial property set?
	BOOL fInitialPropSet = m_eMethodType == METHOD_SETPROPERTIES && m_pGuidPropertySet;

	//Display the Properties (again - this  time all of them)
	TESTC(DisplayProperties(fInitialPropSet ? cPropertyIDSets : 0, rgPropertyIDSets));
	
CLEANUP:
	//AutoSize Columns
	m_listviewProps.SetColumnWidth(PROP_NAME,	LVSCW_AUTOSIZE_USEHEADER);
	m_listviewProps.SetColumnWidth(PROP_TYPE,	LVSCW_AUTOSIZE_USEHEADER);
	m_listviewProps.SetColumnWidth(PROP_VALUE,	LVSCW_AUTOSIZE_USEHEADER);
	m_listviewProps.SetColumnWidth(PROP_FLAGS,	LVSCW_AUTOSIZE_USEHEADER);
	m_listviewProps.SetColumnWidth(PROP_DESC,	LVSCW_AUTOSIZE_USEHEADER);
	if(m_eMethodType != METHOD_GETPROPERTYINFO)
	{
		m_listviewProps.SetColumnWidth(PROP_COLID,	LVSCW_AUTOSIZE_USEHEADER);
	}

	//Disable Combos
	m_editDesc.EnableWindow(FALSE);
	m_comboOptions.EnableWindow(FALSE);
	m_comboType.EnableWindow(FALSE);
	m_comboValue.EnableWindow(FALSE);

	//Delegate
	return CDialogLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnOK
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnOK()
{
	CWaitCursor waitCursor;
	HRESULT hr = S_OK;

	//Send a selection change message to the Value Combo
	if(m_eMethodType == METHOD_SETPROPERTIES)
		OnPropValueChange();
	CPropSets sCPropSets; 
	CPropIDSets sCPropIDSets;

	//Now that the user has selected all the properties
	//they wish to have set, we need to loop through all the checked
	//properties and add them to our list...
	INDEX cItems = m_listviewProps.GetItemCount();
	for(LONG i=0; i<cItems; i++)
	{
		//Only interested in checked items
		BOOL bChecked = m_listviewProps.GetItemState(i, LVIS_STATEIMAGEMASK) & INDEXTOSTATEIMAGEMASK(STATE_CHECKED);
		if(!bChecked)
			continue;

		//Only interested in actual items, (not headers or spaces)
		DBPROPINFOSET* pPropInfoSet = NULL;
		DBPROPINFO* pPropInfo = GetSelPropInfo(i, &pPropInfoSet);
		if(!pPropInfo || !pPropInfoSet)
			continue;

		if(m_eMethodType == METHOD_SETPROPERTIES)
		{
			//Now we have a real checked item so add it to our propset
			sCPropSets.SetProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, DBTYPE_VARIANT, &pPropInfo->vValues, pPropInfo->dwFlags & DBPROPFLAGS_REQUIRED ? DBPROPOPTIONS_REQUIRED : DBPROPOPTIONS_OPTIONAL);
		}
		else
		{
			//Add the PropertyID
			sCPropIDSets.SetProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet);
		}
	}

	if(m_eMethodType == METHOD_SETPROPERTIES)
	{
		//Either just save the properties back in the propset provided
		if(m_pCPropSets)
		{
			m_pCPropSets->Attach(sCPropSets);
		}
		//Or actually set the properties...
		else
		{
			//Now actually set the Properties
			TESTC(hr = SetProperties(sCPropSets.GetCount(), sCPropSets.GetPropSets()));
		}
	}
	else
	{
		//Don't end the Dialog, (if the user has selected properties)
		//just recall GetProperties with those selected properties.  A good way to test
		//a provider on a sub set of properties, rather than always calling with (0, NULL)
		if(sCPropIDSets.GetCount())
		{
			TESTC(hr = DisplayProperties(sCPropIDSets.GetCount(), sCPropIDSets.GetPropSets()));
			return FALSE;
		}
	}

CLEANUP:
	//Don't end the  Dialog on Error...
	if(FAILED(hr))
		return FALSE;

	//Free Saved State...
	FreeProperties(&m_cPropInfoSets, &m_rgPropInfoSets);
	SAFE_FREE(m_pStringBuffer);

	//Delegate
	return CDialogLite::OnOK();
}



/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnCancel
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnCancel()
{
	//Cleanup any memory allocated
	FreeProperties(&m_cPropInfoSets, &m_rgPropInfoSets);
	SAFE_FREE(m_pStringBuffer);

	//Delegate
	return CDialogLite::OnCancel();
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::DisplayProperty
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::DisplayProperty(INDEX iItem, DBPROP* pProp, DBPROPINFO* pPropInfo, REFGUID guidPropertySet, LPARAM lParam)
{
	//Display this property in the listview
	static WCHAR wszBuffer[MAX_NAME_LEN+1] = {0};
	ASSERT(pPropInfo);

	//Determine which icon to display
	INT iImage = IMAGE_QUESTION;	//Default - unknown flags

	//READ|WRITE
	if(BIT_SET(pPropInfo->dwFlags, DBPROPFLAGS_READ|DBPROPFLAGS_WRITE))
	{
		//Read-Write	- (very common)
		iImage = IMAGE_NORMAL;
	}
	//READ
	else if(BIT_SET(pPropInfo->dwFlags, DBPROPFLAGS_READ))
	{
		//Read-Only		- (very common)
		iImage = IMAGE_LOCK;
	}
	//WRITE
	else if(BIT_SET(pPropInfo->dwFlags, DBPROPFLAGS_WRITE))
	{
		//Write-Only	- (not common, able to set but not retrieve values)
		iImage = IMAGE_DELETE;
	}
	
	//PROP_NAME (Indent from PropertySet)
	//Also store the wType as the lParam
	WCHAR* pwszName = GetPropertyName(pPropInfo->dwPropertyID, guidPropertySet);
	if(pwszName)
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"     %s", pwszName);
	else
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"     0x%x", pPropInfo->dwPropertyID);
	m_listviewProps.InsertItem(iItem, PROP_NAME, wszBuffer, lParam, iImage);
			
	//PROP_TYPE
	m_listviewProps.InsertItem(iItem, PROP_TYPE, GetVariantTypeName(pPropInfo->vtType));

	//PROP_VALUE
	VARIANT* pVariant = pProp ? &pProp->vValue : &pPropInfo->vValues;
	if(pVariant->vt == VT_EMPTY)
	{
		//Provide a default value?
		if(m_eMethodType == METHOD_SETPROPERTIES)
		{
			V_VT(pVariant) = pPropInfo->vtType;
			switch(V_VT(pVariant))
			{
				case VT_EMPTY:
				case VT_NULL:
					break;

				case VT_BOOL:
					V_BOOL(pVariant) = VARIANT_TRUE;
					break;

				case VT_I4:
					V_I4(pVariant) = 0;
					break;

				case VT_I2:
					V_I2(pVariant) = 0;
					break;

				case VT_BSTR:
					V_BSTR(pVariant) = NULL;
					break;

				default:
					V_VT(pVariant) = VT_EMPTY;
					break;
			};
		}
	}

	//PROP_VALUE
	VariantToString(pVariant, wszBuffer, MAX_NAME_LEN, CONV_VARBOOL);
	m_listviewProps.InsertItem(iItem, PROP_VALUE, wszBuffer);


	//PROP_FLAGS
	if(m_eMethodType == METHOD_GETPROPERTYINFO)
	{
		//PROP_FALGS
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pPropInfo->dwFlags);
		m_listviewProps.InsertItem(iItem, PROP_FLAGS, wszBuffer);
	}
	else
	{
		if(pProp)
		{
			if(pProp->dwOptions == DBPROPOPTIONS_OPTIONAL)
				m_listviewProps.InsertItem(iItem, PROP_FLAGS, L"OPTIONAL");
			else if(pProp->dwOptions == DBPROPOPTIONS_REQUIRED)
				m_listviewProps.InsertItem(iItem, PROP_FLAGS, L"REQUIRED");
			else
			{
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"0x%08x", pProp->dwOptions);
				m_listviewProps.InsertItem(iItem, PROP_FLAGS, wszBuffer);
			}
		}
		else
		{
			m_listviewProps.InsertItem(iItem, PROP_FLAGS, pPropInfo->dwFlags & DBPROPFLAGS_REQUIRED ? L"REQUIRED" : L"OPTIONAL");
		}
	}

	//PROP_DESC
	m_listviewProps.InsertItem(iItem, PROP_DESC, pPropInfo->pwszDescription);
			
	if(pProp && m_eMethodType != METHOD_GETPROPERTYINFO)
	{
		//PROP_COLID
		DBIDToString(pProp ? &pProp->colid : NULL, wszBuffer, MAX_NAME_LEN);
		m_listviewProps.InsertItem(iItem, PROP_COLID, wszBuffer);
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::DisplayProperties
//
/////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::DisplayProperties(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets)
{
	HRESULT hr = S_OK;
	WCHAR wszBuffer[MAX_NAME_LEN+1];
	ULONG iPropSet,iProp,iItems = 0;
	
	//Now remove any previous items
	m_listviewProps.DeleteAllItems();

	//Free Previous Properties...
	FreeProperties(&m_cPropInfoSets, &m_rgPropInfoSets);
	SAFE_FREE(m_pStringBuffer);

	//GetPropertyInfo
	//Both GetProperties and SetProperites needs this info as well, not just GetPropertyInfo.
	//Since they need to know the description and readwrite flags...
	hr = GetPropertyInfo(cPropertyIDSets, rgPropertyIDSets, &m_cPropInfoSets, &m_rgPropInfoSets, &m_pStringBuffer);

	//Do any specific post processing before displaying
	switch(m_eMethodType)
	{
		case METHOD_GETPROPERTYINFO:
		{
			//We just display the info returned from GetPropertyInfo...
			break;
		}

		case METHOD_SETPROPERTIES:
		{
			ULONG cPropSets = 0;
			DBPROPSET* rgPropSets = NULL;

			//For the SetProperties dialog, we want to display all the properties
			//obtained from GetPropertyInfo and provide the current values of the properties
			//obtained from GetProperties.  This allows the user access to set all available properties
			//and have the current value so they know what it is before setting it...
			hr = GetProperties(0, NULL, &cPropSets, &rgPropSets);

			//Now combine both sets...
			if(SUCCEEDED(hr) && cPropSets && rgPropSets)
				TESTC(hr = CombineProperties(&m_cPropInfoSets, &m_rgPropInfoSets, cPropSets, rgPropSets, TRUE/*bFreeAddedPropSet*/));
			break;
		}

		case METHOD_GETPROPERTIES:
		{
			//Save off the PropertyInfo
			ULONG cPropInfoSets				= m_cPropInfoSets;
			DBPROPINFOSET* rgPropInfoSets	= m_rgPropInfoSets;
			m_cPropInfoSets = 0;
			m_rgPropInfoSets = NULL;

			ULONG cPropSets = 0;
			DBPROPSET* rgPropSets = NULL;

			//GetProperties
			hr = GetProperties(cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets);

			//Now combine both sets...
			if(SUCCEEDED(hr) && cPropSets && rgPropSets)
			{
				//NOTE: We don't want to just combine the properties returned from GetProperties
				//to those returned from GetPropertyInfo, since we should only show those properties
				//returned from GetProperties not a combination of both.  But we do want the 
				//flags and descriptions found in the Info so handle that seperatly...
				
				//Dump our cPropSet into PropInfo sets, so we can have one routine to 
				//dump all properties illregardless of source, and can add the propinfo flags, etc...
				TESTC(hr = CombineProperties(&m_cPropInfoSets, &m_rgPropInfoSets, cPropSets, rgPropSets, TRUE/*bFreeAddedPropSet*/));

				//Add the Prop Flags and Descriptions from GetPropertyInfo
				for(iPropSet=0; iPropSet<m_cPropInfoSets; iPropSet++)
				{
					DBPROPINFOSET* pPropInfoSet = &m_rgPropInfoSets[iPropSet];
					for(iProp=0; iProp<pPropInfoSet->cPropertyInfos; iProp++)
					{
						DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[iProp];
						DBPROPINFO* pFoundProp= FindProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, cPropInfoSets, rgPropInfoSets);
						if(pFoundProp)
						{
							pPropInfo->pwszDescription	= pFoundProp->pwszDescription;
							pPropInfo->dwFlags			|=pFoundProp->dwFlags;
						}
					}
				}
			}

			//Free the previous property info
			FreeProperties(&cPropInfoSets, &rgPropInfoSets);
			break;
		}
	};

	//Add any defined properties (if requested)
	if(IsDlgButtonChecked(IDB_ADDDEFINEDPROPS))
	{
		if(cPropertyIDSets==0 || (rgPropertyIDSets && rgPropertyIDSets[0].cPropertyIDs==0))
		{
			//NOTE: We redo this first, by obtaining the static ones first, and then adding the 
			//properties from above.  This way all defined properties are nicely ordered,
			//as opposed to what some providers return for an ordering...
			ULONG cPropInfoSets				= m_cPropInfoSets;
			DBPROPINFOSET* rgPropInfoSets	= m_rgPropInfoSets;
			m_cPropInfoSets = 0;
			m_rgPropInfoSets = NULL;
			
			//First obtain all the OLE DB Defined Properties for this set.
			//NOTE: We ask for FALSE - return allocated arrays instead of static arrays...
			TESTC(hr = GetAllocedPropInfo(cPropertyIDSets, rgPropertyIDSets, &m_cPropInfoSets, &m_rgPropInfoSets));
	
			//Combine any properties returned from above
			TESTC(hr = CombineProperties(&m_cPropInfoSets, &m_rgPropInfoSets, cPropInfoSets, rgPropInfoSets, TRUE/*bFreeAddedPropSet*/));
		}
	}

	//Now Display all the properties
	iItems = 0;
	for(iPropSet=0; iPropSet<m_cPropInfoSets; iPropSet++)
	{
		DBPROPINFOSET* pPropInfoSet = &m_rgPropInfoSets[iPropSet];
		
		//We need to display this PropertySet
		WCHAR* pwszName = GetPropSetName(pPropInfoSet->guidPropertySet);
		if(pwszName == NULL)
		{
			StringFromGUID2(pPropInfoSet->guidPropertySet, wszBuffer, MAX_NAME_LEN);
			pwszName = wszBuffer;
		}
		m_listviewProps.InsertItem(iItems, PROP_NAME, pwszName, PARAM_NONE, IMAGE_NORMAL);
		m_listviewProps.SetItemState(iItems, 0,  INDEXTOSTATEIMAGEMASK(STATE_NORMAL),  LVIS_STATEIMAGEMASK);
		iItems++;

		//Now display all properties in the set
		for(iProp=0; iProp<pPropInfoSet->cPropertyInfos; iProp++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[iProp];

			//Try to find this Property in the PropSet
			DBPROP* pFoundProp = m_pCPropSets ? m_pCPropSets->FindProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, DB_NULLID) : NULL;

			//To make our lives easier we will store the [iPropSet,iProp] index into the
			//PropInfoSet or every property into the lParam element of the list view
			//This way whatever message were one we have the neccessary propinfo...
			LPARAM lParam = MAKELONG(iPropSet,iProp);

			//Supply Default Type and Values if there is currently none
			if(pFoundProp)
			{
				pPropInfo->vtType = V_VT(&pFoundProp->vValue);
				XTEST(VariantCopyFast(&pPropInfo->vValues, &pFoundProp->vValue));
			}

			//Delegate to Display this property
			TESTC(hr = DisplayProperty(iItems, pFoundProp, pPropInfo, pPropInfoSet->guidPropertySet, lParam));

			//Set Item State to Checked/Unchecked
			m_listviewProps.SetItemState(iItems, 0,  INDEXTOSTATEIMAGEMASK((pFoundProp && m_pCPropSets) ? STATE_CHECKED : STATE_UNCHECKED),  LVIS_STATEIMAGEMASK);
			iItems++;
		}

		//Now display a space between sets...
		m_listviewProps.InsertItem(iItems, 0, L"", PARAM_NONE, IMAGE_NORMAL);
		m_listviewProps.SetItemState(iItems, 0,  INDEXTOSTATEIMAGEMASK(STATE_NORMAL),  LVIS_STATEIMAGEMASK);
		iItems++;
	}

CLEANUP:
	return hr;
}
			

/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::SetProperties
//
/////////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets)
{
	HRESULT hr = S_OK;
	
	if(m_pIUnkProperties)
	{
		IID iidProp = m_iidProp;
		
		//Now actually set the Properties
		if(iidProp == IID_IDBProperties)
		{
			XTEST_(hr = ((IDBProperties*)m_pIUnkProperties)->SetProperties(cPropSets, rgPropSets),S_OK);
			TRACE_METHOD(hr, L"IDBProperties::SetProperties(%d, 0x%p)", cPropSets, rgPropSets);
		}
		else if(iidProp == IID_IDBDataSourceAdmin)
		{
			XTEST_(hr = ((IDBDataSourceAdmin*)m_pIUnkProperties)->ModifyDataSource(cPropSets, rgPropSets),S_OK);
			TRACE_METHOD(hr, L"IDBDataSourceAdmin::ModifyDataSource(%d, 0x%p)", cPropSets, rgPropSets);
		}
		else if(iidProp == IID_ISessionProperties)
		{
			XTEST_(hr = ((ISessionProperties*)m_pIUnkProperties)->SetProperties(cPropSets, rgPropSets),S_OK);
			TRACE_METHOD(hr, L"ISessionProperties::SetProperties(%d, 0x%p)", cPropSets, rgPropSets);
		}
		else if(iidProp == IID_ICommandProperties)
		{
			XTEST_(hr = ((ICommandProperties*)m_pIUnkProperties)->SetProperties(cPropSets, rgPropSets),S_OK);
			TRACE_METHOD(hr, L"ICommandProperties::SetProperties(%d, 0x%p)", cPropSets, rgPropSets);
		}
		else
		{
			ASSERT(!"Unhandled Property Source");
		}

		//Display Any Property Errors...
		hr = DisplayPropErrors(hr, cPropSets, rgPropSets);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::GetProperties
//
/////////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::GetProperties(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropSets, DBPROPSET** prgPropSets)
{
	HRESULT hr = S_OK;
	ASSERT(pcPropSets);
	ASSERT(prgPropSets);
	
	if(m_pIUnkProperties)
	{
		if(m_iidProp == IID_IDBProperties)
		{
			//IDBProperties::GetProperties
			XTEST(hr = ((IDBProperties*)m_pIUnkProperties)->GetProperties(cPropertyIDSets, rgPropertyIDSets, pcPropSets, prgPropSets));
			TRACE_METHOD(hr, L"IDBProperties::GetProperties(%d, 0x%p, &%d, &0x%p)", cPropertyIDSets, rgPropertyIDSets, *pcPropSets, *prgPropSets);
		}
		else if(m_iidProp == IID_ISessionProperties)
		{
			//ISessionProperties::GetProperties
			XTEST(hr = ((ISessionProperties*)m_pIUnkProperties)->GetProperties(cPropertyIDSets, rgPropertyIDSets, pcPropSets, prgPropSets));
			TRACE_METHOD(hr, L"ISessionProperties::GetProperties(%d, 0x%p, &%d, &0x%p)", cPropertyIDSets, rgPropertyIDSets, *pcPropSets, *prgPropSets);
		}
		else if(m_iidProp == IID_ICommandProperties)
		{
			//ICommandProperties::GetProperties
			XTEST(hr = ((ICommandProperties*)m_pIUnkProperties)->GetProperties(cPropertyIDSets, rgPropertyIDSets, pcPropSets, prgPropSets));
			TRACE_METHOD(hr, L"ICommandProperties::GetProperties(%d, 0x%p, &%d, &0x%p)", cPropertyIDSets, rgPropertyIDSets, *pcPropSets, *prgPropSets);
		}
		else if(m_iidProp == IID_IRowsetInfo)
		{
			//IRowsetInfo::GetProperties
			XTEST(hr = ((IRowsetInfo*)m_pIUnkProperties)->GetProperties(cPropertyIDSets, rgPropertyIDSets, pcPropSets, prgPropSets));
			TRACE_METHOD(hr, L"IRowsetInfo::GetProperties(%d, 0x%p, &%d, &0x%p)", cPropertyIDSets, rgPropertyIDSets, *pcPropSets, *prgPropSets);
		}

		//Display Any Property Errors...
		hr = DisplayPropErrors(hr, *pcPropSets, *prgPropSets);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::GetPropertyInfo
//
/////////////////////////////////////////////////////////////////////
HRESULT CPropertiesDlg::GetPropertyInfo(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, WCHAR** ppStringBuffer)
{
	HRESULT hr = S_OK;
	ASSERT(pcPropInfoSets);
	ASSERT(prgPropInfoSets);
	ASSERT(ppStringBuffer);
	
	if(m_pIDBPropertyInfo || m_pIDBDataSourceAdmin)
	{
		//Normally GetPropertyInfo supplies all the Property Information,
		//But OLE DB does have a couple of other interfaces that supply prop info
		if(m_pIDBPropertyInfo)
		{
			//IDBProperties::GetPropertyInfo
			//NOTE: GetPropertyInfo may fail since the provider may not support this PropertySet.
			//Just continue since we allow the user to set the OLE DB Defined Properties
			XTEST(hr = m_pIDBPropertyInfo->GetPropertyInfo(cPropertyIDSets, rgPropertyIDSets, pcPropInfoSets, prgPropInfoSets, ppStringBuffer));
			TRACE_METHOD(hr, L"IDBProperties::GetPropertyInfo(%d, 0x%p, &%d, &0x%p, &0x%p)", cPropertyIDSets, rgPropertyIDSets, *pcPropInfoSets, *prgPropInfoSets, *ppStringBuffer);
		}
		else
		{
			//IDBDataSourceAdmin::GetCreationProperties
			//NOTE: GetCreationProperties may fail since the provider may not support this PropertySet.
			//Just continue since we allow the user to set the OLE DB Defined Properties
			XTEST(hr = m_pIDBDataSourceAdmin->GetCreationProperties(cPropertyIDSets, rgPropertyIDSets, pcPropInfoSets, prgPropInfoSets, ppStringBuffer));
			TRACE_METHOD(hr, L"IDBDataSourceAdmin::GetCreationProperties(%d, 0x%p, &%d, &0x%p, &0x%p)", cPropertyIDSets, rgPropertyIDSets, *pcPropInfoSets, *prgPropInfoSets, *ppStringBuffer);
		}

		//Display Any Property Errors...
		hr = DisplayPropErrors(hr, *pcPropInfoSets, *prgPropInfoSets);
	}

	return hr;
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::GetSelPropInfo
//
/////////////////////////////////////////////////////////////////////
DBPROPINFO* CPropertiesDlg::GetSelPropInfo(INDEX iSelRow, DBPROPINFOSET** ppPropInfoSet)
{
	if(iSelRow == LVM_ERR)
		return NULL;
	
	//Obtain PropInfo element
	//This value is stored as the lParam of the ListView row
	LPARAM lParam = m_listviewProps.GetItemParam(iSelRow);
	if(lParam == LVM_ERR || lParam == PARAM_NONE)
		return NULL;
	
	ULONG iPropInfoSet = LOWORD(lParam);
	ULONG iPropInfo = HIWORD(lParam);
	
	//Verify results...
	if(iPropInfoSet >= m_cPropInfoSets || m_rgPropInfoSets==NULL || iPropInfo >= m_rgPropInfoSets[iPropInfoSet].cPropertyInfos || m_rgPropInfoSets[iPropInfoSet].rgPropertyInfos==NULL)
		return NULL;
	
	//Does the user want the Set as well...
	if(ppPropInfoSet)
		*ppPropInfoSet = &m_rgPropInfoSets[iPropInfoSet];
		
	//Otherwise we have a valid propset and property...
	return &m_rgPropInfoSets[iPropInfoSet].rgPropertyInfos[iPropInfo];
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnPropOptionsChange
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnPropOptionsChange()
{
	//Obtain the selected propery
	INDEX iSelRow = m_comboOptions.GetItemParam(0);
	DBPROPINFO* pPropInfo = GetSelPropInfo(iSelRow);
	if(pPropInfo)
	{
		//Get the new selection
		//TODO64: DBPROPFLAGS is not extended for 64bit?
		ENABLE_BIT(pPropInfo->dwFlags, DBPROPFLAGS_REQUIRED, m_comboOptions.GetCurSel()==DBPROPOPTIONS_REQUIRED);
		
		//Display the new selection
		m_listviewProps.SetItemText(iSelRow, PROP_FLAGS, pPropInfo->dwFlags & DBPROPFLAGS_REQUIRED ? L"REQUIRED" : L"OPTIONAL");

		//Automatically check this property, since the user 
		//changed the Options they probably want to set it...
		m_listviewProps.SetItemState(iSelRow, 0,  INDEXTOSTATEIMAGEMASK(STATE_CHECKED),  LVIS_STATEIMAGEMASK);
	}

	return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnPropTypeChange
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnPropTypeChange()
{
	//Obtain the selected propery
	INDEX iSelRow = m_comboOptions.GetItemParam(0);
	DBPROPINFO* pPropInfo = GetSelPropInfo(iSelRow);
	if(pPropInfo)
	{

		//Get new selection
		INDEX iSel = m_comboType.GetCurSel();
		pPropInfo->vtType = (DBTYPE)m_comboType.GetItemParam(iSel);
		m_listviewProps.SetItemText(iSelRow, PROP_TYPE, GetVariantTypeName(pPropInfo->vtType));
		
		switch(pPropInfo->vtType)
		{
			case VT_BOOL:
				m_comboValue.SetSelText(L"VARIANT_FALSE", TRUE/*fAddItem*/);
				m_comboValue.SetSelText(L"VARIANT_TRUE", TRUE/*fAddItem*/);
				break;

			case VT_EMPTY:
			case VT_NULL:
				m_comboValue.ResetContent();
				m_comboValue.SetSelText(L"", TRUE/*fAddItem*/);
				break;
		};
		
		//Send a selection change message to the Value Combo
		OnPropValueChange();

		//Automatically check this property, since the user 
		//changed the Type they probably want to set it...
		m_listviewProps.SetItemState(iSelRow, 0,  INDEXTOSTATEIMAGEMASK(STATE_CHECKED),  LVIS_STATEIMAGEMASK);
	}

	return TRUE;
}

	
/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnPropValueChange
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnPropValueChange()
{
	//Obtain the selected propery
	INDEX iSelRow = m_comboOptions.GetItemParam(0);
	DBPROPINFO* pPropInfo = GetSelPropInfo(iSelRow);
	if(pPropInfo)
	{
		WCHAR wszBuffer[MAX_NAME_LEN+1] = {0};
		WCHAR wszPrevValue[MAX_NAME_LEN+1] = {0};
		
		//Get new selection
		m_comboValue.GetSelText(wszBuffer, MAX_NAME_LEN);
		
		//Verify Value
		XTEST(VariantClearFast(&pPropInfo->vValues));
		if(FAILED(StringToVariant(wszBuffer, (DBTYPE)pPropInfo->vtType, &pPropInfo->vValues)))
		{
			wMessageBox(GetFocus(), MB_TASKMODAL | MB_ICONERROR | MB_OK | MB_DEFBUTTON1, 
				wsz_ERROR, L"Unable to create Variant for type=%s, value=\"%s\"", GetVariantTypeName(pPropInfo->vtType), wszBuffer);
			m_comboValue.SetFocus();
 			return TRUE; //Don't Allow the Change
		}

		//NOTE: We convert again here, in case the user entered
		//some "alias" name for the value.  IE: True and we actually allow this
		//but display in the view as VARIANT_TRUE...
		VariantToString(&pPropInfo->vValues, wszBuffer, MAX_NAME_LEN, CONV_VARBOOL);

		//Automatically check this property, since the user 
		//changed the Value they probably want to set it...
		//NOTE: We only do it if the value has changed...
		m_listviewProps.GetItemText(iSelRow, PROP_VALUE, wszPrevValue, MAX_NAME_LEN);
		if(!StringCompare(wszBuffer, wszPrevValue))
		{
			m_listviewProps.SetItemState(iSelRow, 0,  INDEXTOSTATEIMAGEMASK(STATE_CHECKED),  LVIS_STATEIMAGEMASK);
			m_listviewProps.SetItemText(iSelRow, PROP_VALUE, wszBuffer);
		}
	}

	return TRUE;
}
	
/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl)
{
	switch(wNotifyCode)
	{
		case CBN_SELCHANGE:
		{	
			if(OnSelChange(iID, hWndCtrl))
				return TRUE;
			break;
		}
	};

	//Otherwise Delegate
	return CDialogLite::OnCommandNotify(wNotifyCode, iID, hWndCtrl);
}


/////////////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnSelChange
//
/////////////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnSelChange(INT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		case IDC_PROPEDIT_VALUE:
			return OnPropValueChange();
		
		case IDC_PROPEDIT_TYPE:
			return OnPropTypeChange();
		
		case IDC_PROPEDIT_OPTIONS:
			return OnPropOptionsChange();
		
		case IDC_PROPSET:
			return OnPropSetChange();
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnClearAll
//
/////////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnClearAll()
{
	CWaitCursor waitCursor;

	//Send a selection change message to the Value Combo
	OnPropValueChange();

	//Now that the user has selected to clear all properties
	//We need to loop through all the checked
	//properties and just uncheck them...
	INDEX cItems = m_listviewProps.GetItemCount();
	for(LONG i=0; i<cItems; i++)
	{
		//Get the Image and Param
		//Only interested in actual items, (not headers or spaces)
		LPARAM lParam = m_listviewProps.GetItemParam(i);
		if(lParam == PARAM_NONE)
			continue;

		//Set the state
		m_listviewProps.SetItemState(i, 0,  INDEXTOSTATEIMAGEMASK(m_bClearAll ? STATE_UNCHECKED : STATE_CHECKED),  LVIS_STATEIMAGEMASK);
	}

	//Clear All is a toggle...
	if(m_bClearAll)
	{
		m_bClearAll = FALSE;
		::SetWindowText(GetDlgItem(IDB_CLEARALL), "SelectAll");
	}
	else
	{
		m_bClearAll = TRUE;
		::SetWindowText(GetDlgItem(IDB_CLEARALL), "ClearAll");
	}
}
				

				
/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnCommand(UINT iID, HWND hWndCtrl)
{
	switch(iID)
	{
		ON_COMMAND(IDB_ADDDEFINEDPROPS,			OnAddDefinedProperties())
		ON_COMMAND(IDB_CLEARALL,				OnClearAll())
	};

	return FALSE;
}
	
/////////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	NM_LISTVIEW* pListView = (NM_LISTVIEW*)pNMHDR;
			
	//ListView
	switch(pListView->hdr.code)
	{
		case LVN_ITEMCHANGED:
		{
			//Check boxes
			if(pListView->uNewState & LVIS_STATEIMAGEMASK)
			{
				//Get the Current Param
				LPARAM lParam = m_listviewProps.GetItemParam(pListView->iItem);

				//All lParam are PARAM_NONE for headers and non properties...
				//We don't want to change icons for those...
				if(lParam == PARAM_NONE && 
					(pListView->uNewState & INDEXTOSTATEIMAGEMASK(STATE_CHECKED) ||	pListView->uNewState & INDEXTOSTATEIMAGEMASK(STATE_UNCHECKED)))
				{
					m_listviewProps.SetItemState(pListView->iItem, pListView->iSubItem,  INDEXTOSTATEIMAGEMASK(STATE_NORMAL),  LVIS_STATEIMAGEMASK);
					return TRUE;
				}
			}
			return FALSE;
		}	

		case LVN_ITEMACTIVATE:
		{
			//We only allow changes for SetProperties
			if(m_eMethodType != METHOD_SETPROPERTIES)
				return FALSE;
			
			//Obtain PropInfo element
			INDEX iSelRow = pListView->iItem;
			DBPROPINFO* pPropInfo = GetSelPropInfo(iSelRow);
			if(pPropInfo)
			{
				//TODO: Item and sub item appear to be correct.
				//Should use inplace editing in the listview, rather than seperate headers...
				
				//Which column was selected?
				switch(pListView->iSubItem)
				{
					case PROP_NAME:
					case PROP_DESC:
						//These items are not settable...
						break;

					case PROP_TYPE:
					{
/*						//Obtain the rect of the selected item.
						RECT rect = { LVIR_BOUNDS, pListView->iSubItem, 0, 0};
						SendMessage(m_listviewProps.m_hWnd, LVM_GETSUBITEMRECT, pListView->iItem, (LPARAM)&rect);
						
						//Bring up a ComboBox
						//NOTE: The "Height" of the combo indicates the "dropdown" height of the combo...
						static CComboBoxLite comboTypes;
						comboTypes.Create(m_listviewProps.m_hWnd, L"COMBOBOX", NULL, IDC_TYPE,
							CBS_DROPDOWNLIST | CBS_AUTOHSCROLL |
								WS_TABSTOP | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL, 
							0,
							rect.left, rect.top, rect.right-rect.left, (rect.bottom-rect.top)*15);
						comboTypes.SetFont(m_pCMainWindow->m_pCMDIObjects->m_pCObjTree->GetFont());

						//Fill in the Type Combo
						comboTypes.Populate(g_cVariantTypes, g_rgVariantTypes);
						comboTypes.OnCreate(NULL);
*/						break;
					}

					case PROP_VALUE:
						break;
					case PROP_FLAGS: 
						break;
						
					case PROP_COLID:
						//TODO:
						break;

					default:
				//		ASSERT(L"Unhandled Column?");
						break;
				};
			}
			
			return TRUE;
		}

		case LVN_ITEMCHANGING:
		{
			//We Selected a "new" item
			if(pListView->uNewState & LVNI_FOCUSED &&
				pListView->uNewState & LVNI_SELECTED)
			{								
				//Send a selection change message to the Value Combo
				if(m_eMethodType == METHOD_SETPROPERTIES)
					OnPropValueChange();

				//Clear Previous info
				m_editDesc.SetWindowText(L"");
				m_comboValue.ResetContent();
						
				//Obtain PropInfo element
				INDEX iSelRow = pListView->iItem;
				DBPROPINFO* pPropInfo = GetSelPropInfo(iSelRow);
				if(pPropInfo)
				{
					//Insert Name (Description)
					WCHAR wszBuffer[MAX_NAME_LEN+1] = {0};
					m_listviewProps.GetItemText(iSelRow, PROP_DESC, wszBuffer, MAX_NAME_LEN);
					m_editDesc.SetWindowText(wszBuffer);
					
					//Set Current Type Selection
					m_comboType.SetSelValue(pPropInfo->vtType);

					//InsertValue
					if(pPropInfo->vtType == DBTYPE_BOOL)
					{
						m_comboValue.AddString(L"VARIANT_TRUE");
						m_comboValue.AddString(L"VARIANT_FALSE");
					}
					
					wszBuffer[0] = EOL;
					m_listviewProps.GetItemText(iSelRow, PROP_VALUE, wszBuffer, MAX_NAME_LEN);
					INDEX iIndex = m_comboValue.SetSelText(wszBuffer, TRUE/*fAddItem*/);

					//Now select the item
					m_comboValue.SetCurSel(iIndex);

					//InsertOptions
//					m_comboOptions.SetItemParam(0, iSelRow);
					m_comboOptions.SetSelText(pPropInfo->dwFlags & DBPROPFLAGS_REQUIRED ? L"REQUIRED" : L"OPTIONAL");
				}

				//Keep track of which row the data belongs to...
				m_comboOptions.SetItemParam(0, iSelRow);

				//Enable Combos
				m_editDesc.EnableWindow(pPropInfo && m_eMethodType == METHOD_SETPROPERTIES);
				m_comboType.EnableWindow(pPropInfo && m_eMethodType == METHOD_SETPROPERTIES);
				m_comboValue.EnableWindow(pPropInfo && m_eMethodType == METHOD_SETPROPERTIES);
				m_comboOptions.EnableWindow(pPropInfo && m_eMethodType == METHOD_SETPROPERTIES);
				return FALSE; //Allow the change
			}
		}
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnAddDefinedProperties
//
/////////////////////////////////////////////////////////////////
void CPropertiesDlg::OnAddDefinedProperties()
{
	OnPropSetChange();
}

	
////////////////////////////////////////////////////////////////
// CPropertiesDlg::OnPropSetChange
//
/////////////////////////////////////////////////////////////////
BOOL CPropertiesDlg::OnPropSetChange()
{
	CWaitCursor waitCursor;

	//Get new selection
	INDEX iIndex = m_comboPropSet.GetItemParam(m_comboPropSet.GetCurSel());
	
	GUID guidPropertySet = IID_NULL;
	if(iIndex != CB_ERR && iIndex<(INDEX)g_cPropSetMaps)
		guidPropertySet = *(g_rgPropSetMaps[iIndex].pGuid); 
	
	//Display properties for just this set...
	const ULONG cPropertyIDSets = 1;
	DBPROPIDSET rgPropertyIDSets[cPropertyIDSets];
	rgPropertyIDSets[0].cPropertyIDs = 0;
	rgPropertyIDSets[0].rgPropertyIDs = NULL;
	rgPropertyIDSets[0].guidPropertySet = guidPropertySet;

	//Display the SetProperties (again - this  time all of them)
	DisplayProperties(iIndex==CB_ERR ? 0 : cPropertyIDSets, rgPropertyIDSets);
		
	//Disable Combos
	m_editDesc.EnableWindow(FALSE);
	m_comboOptions.EnableWindow(FALSE);
	m_comboType.EnableWindow(FALSE);
	m_comboValue.EnableWindow(FALSE);
	return TRUE;
}

						



static const ULONG RES_NAME[MAX_RESTRICTIONS]	= { IDT_RESTRICTION1, IDT_RESTRICTION2, IDT_RESTRICTION3, IDT_RESTRICTION4, IDT_RESTRICTION5, IDT_RESTRICTION6, IDT_RESTRICTION7, /*IDT_RESTRICTION8, IDT_RESTRICTION9, IDT_RESTRICTION10, IDT_RESTRICTION11, IDT_RESTRICTION12*/ };
static const ULONG RES_TYPE[MAX_RESTRICTIONS]	= { IDC_RESTRICTION1, IDC_RESTRICTION2, IDC_RESTRICTION3, IDC_RESTRICTION4, IDC_RESTRICTION5, IDC_RESTRICTION6, IDC_RESTRICTION7, /*IDC_RESTRICTION8, IDC_RESTRICTION9, IDC_RESTRICTION10, IDC_RESTRICTION11, IDC_RESTRICTION12*/ };
static const ULONG RES_VALUE[MAX_RESTRICTIONS]	= { IDE_RESTRICTION1, IDE_RESTRICTION2, IDE_RESTRICTION3, IDE_RESTRICTION4, IDE_RESTRICTION5, IDE_RESTRICTION6, IDE_RESTRICTION7, /*IDE_RESTRICTION8, IDE_RESTRICTION9, IDE_RESTRICTION10, IDE_RESTRICTION11, IDE_RESTRICTION12*/ };
//////////////////////////////////////////////////////////////////////////////
// GetSchemaName
//
//////////////////////////////////////////////////////////////////////////////
WCHAR* GetSchemaName(REFGUID guidSchema)
{
	for(ULONG i=0; i<NUMELE(g_rgSchemaInfo); i++)
	{
		if(guidSchema == *(g_rgSchemaInfo[i].pguidSchema))
			return g_rgSchemaInfo[i].pwszSchemaName;
	}

	return NULL;
}



////////////////////////////////////////////////////////////////
// CSchemaDlg::CSchemaDlg
//
/////////////////////////////////////////////////////////////////
CSchemaDlg::CSchemaDlg(CMDIChild* pCMDIChild)	
	: CRowsetViewerDlg(NULL, pCMDIChild)
{
	//Data
	m_hWndSchemas = NULL;
	m_hWndTreeWnd = NULL;
	m_hWndTreeView = NULL;

	//Saved Dialog Values
	m_fEditing = FALSE;
	m_fActive = TRUE;

	m_cSchemas = 0;
	m_rgSchemas = NULL;
	m_rgRestrictionSupport = NULL;
	
	//Rowset - This is for the SchemaTree sheet, it uses its own rowset
	//so it doesn't mess up the users rowset, just to display a tree...
	m_pCTreeRowset		= NULL;

	m_iSchemaIndex		= LVM_ERR;
	m_iOldSchemaIndex	= LVM_ERR;
	
	m_cRestrictions		= 0;
	m_fUseRestrictions	= TRUE;
	m_fUseProperties	= TRUE;	//Default
	m_fShowSupported	= TRUE;

	//Only set these "Default" properties, if requested by the user
	if(GetOptions()->m_dwRowsetOpts & ROWSET_SETDEFAULTPROPS)
	{
		//DBPROP_CANHOLDROWS is required by the OLE DB Spec - Level-0 Conformance
		//Since it is also legal to set a ReadOnly property, just blindy set it...
		m_CPropSets.SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBTYPE_BOOL, (void*)VARIANT_TRUE, DBPROPOPTIONS_REQUIRED);
	}
}


////////////////////////////////////////////////////////////////
// CSchemaDlg::~CSchemaDlg
//
/////////////////////////////////////////////////////////////////
CSchemaDlg::~CSchemaDlg()
{
	m_hWnd = NULL;
	SAFE_RELEASE(m_pCTreeRowset);

	SAFE_FREE(m_rgSchemas);
	SAFE_FREE(m_rgRestrictionSupport);
	
	//Free Restrictions
	FreeData(DBTYPE_VARIANT, m_cRestrictions, m_rgRestrictions);
}



////////////////////////////////////////////////////////////////
// CSchemaDlg::Display
//
/////////////////////////////////////////////////////////////////
INT_PTR CSchemaDlg::Display()
{
	//Now Display the dialog
	PROPSHEETPAGE psp[2];
	PROPSHEETHEADER psh;

	//Header
	psh.dwSize			= sizeof(PROPSHEETHEADER);
	psh.dwFlags			= PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.hwndParent		= m_pCMDIChild->m_hWnd;
	psh.hInstance		= GetAppLite()->m_hInstance;
	psh.pszIcon			= NULL;
	psh.pszCaption		= "IDBSchemaRowset";
	psh.nPages			= NUMELE(psp);
	psh.nStartPage		= 0;
	psh.ppsp			= (LPCPROPSHEETPAGE) &psp;

	//Provider
	psp[0].dwSize		= sizeof(PROPSHEETPAGE);
	psp[0].dwFlags		= PSP_USETITLE;
	psp[0].hInstance	= GetAppLite()->m_hInstance;
	psp[0].pszTemplate	= MAKEINTRESOURCE(IDD_SCHEMAROWSET);
	psp[0].pszIcon		= NULL;
	psp[0].pfnDlgProc	= SchemaRowsetProc;
	psp[0].pszTitle		= "Schemas";
	psp[0].lParam		= (LPARAM)this;

	//Properties
	psp[1].dwSize		= sizeof(PROPSHEETPAGE);
	psp[1].dwFlags		= PSP_USETITLE;
	psp[1].hInstance	= GetAppLite()->m_hInstance;
	psp[1].pszTemplate	= MAKEINTRESOURCE(IDD_SCHEMATREEVIEW);
	psp[1].pszIcon		= NULL;
	psp[1].pfnDlgProc	= SchemaTreeProc;
	psp[1].pszTitle		= "TreeView";
	psp[1].lParam		= (LPARAM)this;
	
	//Display the Property Sheets
	return PropertySheet(&psh);
}


////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::InitSchemas
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::InitSchemas(HWND hWnd)
{
	HRESULT hr = S_OK;
	m_hWndSchemas = hWnd;
	INDEX i,iSel = 0;

	//Default Check UseRestrictions and UseProperties
	::CheckDlgButton(hWnd, IDB_USEPROPERTIES,		BST2STATE(m_fUseProperties));
	::CheckDlgButton(hWnd, IDB_USERESTRICTIONS,		BST2STATE(m_fUseRestrictions));
	::CheckDlgButton(hWnd, IDB_SHOWSUPPORTED,		BST2STATE(m_fShowSupported));

	//Obtain Supported Schemas
	CSession* pCSession = (CSession*)m_pCMDIChild->GetObject(eCSession);

	//Free any previously obtain schemainfo
	SAFE_FREE(m_rgSchemas);
	SAFE_FREE(m_rgRestrictionSupport);

	//Obtain Supported Schemas
	ASSERT(pCSession && pCSession->m_pIDBSchemaRowset);
	XTEST(hr = pCSession->m_pIDBSchemaRowset->GetSchemas(&m_cSchemas, &m_rgSchemas, &m_rgRestrictionSupport));
	TESTC(TRACE_METHOD(hr, L"IDBSchemaRowset::GetSchemas(&%d, &0x%p, &0x%p)", m_cSchemas, m_rgSchemas, m_rgRestrictionSupport));

	//Fill in Schema Combo
	m_comboSchema.CreateIndirect(m_hWndSchemas, IDC_SCHEMAS);
	InitSchemaCombo(m_fShowSupported);
	
	//Fill in Controls
	for(i=0; i<MAX_RESTRICTIONS; i++)
	{
		//Name
		m_rgResName[i].CreateIndirect(m_hWndSchemas, RES_NAME[i]);
		
		//Type
		m_rgResType[i].CreateIndirect(m_hWndSchemas, RES_TYPE[i]);
		m_rgResType[i].Populate(g_cVariantTypes, g_rgVariantTypes);

		//Value
		m_rgResValue[i].CreateIndirect(m_hWndSchemas, RES_VALUE[i]);
	}

	//Try to find DBSCHEMA_TABLES by default (or our saved index)
	if(m_iSchemaIndex == LVM_ERR)
		m_iSchemaIndex = m_comboSchema.FindStringExact(L"DBSCHEMA_TABLES");
	m_iSchemaIndex = m_comboSchema.SetCurSel(m_iSchemaIndex==CB_ERR ? 0 : m_iSchemaIndex);

CLEANUP:
	return hr;
}
	

////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::InitSchemaCombo
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::InitSchemaCombo(BOOL fShowSupported)
{
	WCHAR wszBuffer[MAX_NAME_LEN+1];
	const SCHEMAINFO* pSchemaInfo = NULL;

	//Delete all the current items...
	m_comboSchema.ResetContent();

	//Display the complete list of Schemas (if requested)
	if(!fShowSupported)
	{
		for(ULONG i=0; i<NUMELE(g_rgSchemaInfo); i++)
		{
			pSchemaInfo = &g_rgSchemaInfo[i];
		
			//Now Add it to the ComboBox dialog...
			m_comboSchema.AddString(pSchemaInfo->pwszSchemaName, (LPARAM)pSchemaInfo);
		}
	}

	//Loop over returned Schemas
	for(ULONG i=0; i<m_cSchemas; i++)
	{
		//Find this Schema in our List so we know the "text" name of it...
		pSchemaInfo = NULL;
		for(ULONG j=0; j<NUMELE(g_rgSchemaInfo); j++)
		{
			if(m_rgSchemas[i] == *g_rgSchemaInfo[j].pguidSchema)
			{
				pSchemaInfo = &g_rgSchemaInfo[j];
				break;
			}
		}

		//There may be a provider specific SchemaGuid, 
		//so just display the Guid value and not a name
		if(pSchemaInfo == NULL)
			StringFromGUID2(m_rgSchemas[i], wszBuffer, MAX_NAME_LEN);
		
		//Now Add it to the ComboBox dialog...
		//Only add if were only showing supported, or its not in the known list...
		if(fShowSupported || pSchemaInfo==NULL)
			m_comboSchema.AddString(pSchemaInfo ? pSchemaInfo->pwszSchemaName : wszBuffer, pSchemaInfo ? (LPARAM)pSchemaInfo : (LPARAM)i);
	}
	
	return S_OK;
}


////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::GetSelectedSchema
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::GetSelectedSchema(SCHEMAINFO** ppSchemaInfo, ULONG* pdwRestrictions)
{
	ASSERT(ppSchemaInfo);
	SCHEMAINFO* pSchemaInfo = NULL;

	if(pdwRestrictions)
		*pdwRestrictions = 0;

	//User has selected a different Schema
	//We need to update our restriction list...

	//Get Selected Schema
	m_iSchemaIndex = m_comboSchema.GetCurSel();
	if(m_iSchemaIndex != CB_ERR)
		pSchemaInfo = (SCHEMAINFO*)m_comboSchema.GetItemParam(m_iSchemaIndex);
	
	//Provider Specific Schemas have the Item Data as the index rather than a pointer
	if(pSchemaInfo>=0 && pSchemaInfo<(SCHEMAINFO*)(ULONG_PTR)m_cSchemas)
	{
		m_iSchemaIndex = (INDEX)pSchemaInfo;
		pSchemaInfo = NULL;
	}
	
	if(pSchemaInfo)
	{
		//Need to find the corresponding restrictions...
		if(pdwRestrictions)
		{
			m_iSchemaIndex = CB_ERR;
			for(ULONG i=0; i<m_cSchemas; i++)
			{
				if(*pSchemaInfo->pguidSchema == m_rgSchemas[i])
				{
					m_iSchemaIndex = i;
					*pdwRestrictions = m_rgRestrictionSupport[m_iSchemaIndex];
					break;
				}
			}
		}
	}
	//This is a little more difficult, since there must be a provider 
	//specific schema, so we need to build up the info...
	else
	{
		//Clear our old SchemaInfo temp storage
		memset(&m_SchemaInfo, 0, sizeof(SCHEMAINFO));

		if(m_iSchemaIndex>=0 && (ULONG)m_iSchemaIndex<m_cSchemas)
		{
			//Figure our how many restrictions are supported
			//This is required since E_INVALIDARG will be produced if
			//cRestrictions > than the number of max restricitons defined for this Guid
			for(ULONG i=0; i<MAX_RESTRICTIONS; i++)
			{
				if(m_rgRestrictionSupport[m_iSchemaIndex] & (0x00000001 << i))
					m_SchemaInfo.cRestrictions = i+1;
				m_SchemaInfo.rgResInfo[i].iNode =	i+1;
				m_SchemaInfo.rgResInfo[i].wType	=	DBTYPE_BSTR;	//Default
			}

			//dwRestrictions
			if(pdwRestrictions)
				*pdwRestrictions = m_rgRestrictionSupport[m_iSchemaIndex];

			//pGuid
			m_SchemaInfo.pguidSchema = &m_rgSchemas[m_iSchemaIndex];
		}

		//Return a pointer to our class temp SchemaInfo
		pSchemaInfo = &m_SchemaInfo;
	}

	*ppSchemaInfo = pSchemaInfo;
	return S_OK;
}



////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::ChangeRestrictions
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::ChangeRestrictions()
{
	ULONG i=0; 
	HRESULT hr = S_OK;
	SCHEMAINFO* pSchemaInfo = NULL;
	DWORD dwRestrictions = 0;

	//Get the Selected Schema
	TESTC(hr = GetSelectedSchema(&pSchemaInfo, &dwRestrictions));

	//Display RESTRICTION Info
	for(i=0; i<MAX_RESTRICTIONS; i++)
	{
		//Display Restriction Name
		if(i < pSchemaInfo->cRestrictions)
		{
			m_rgResName[i].SetWindowText(pSchemaInfo->rgResInfo[i].pwszName ? pSchemaInfo->rgResInfo[i].pwszName : L"Unknown");
			m_rgResType[i].SetSelValue(pSchemaInfo->rgResInfo[i].wType);
		}
		else
		{
			m_rgResName[i].SetWindowText(L"");
			m_rgResType[i].SetSelValue(DBTYPE_BSTR);
		}
		
		//Default Value
		m_rgResName[i].EnableWindow(dwRestrictions & (0x00000001 << i));
		m_rgResValue[i].SetWindowText(L"");
	}

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::GetSelectedRestrictions
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::GetSelectedRestrictions()
{
	HRESULT hr = S_OK;
	SCHEMAINFO* pSchemaInfo = NULL;
	WCHAR* pwszValue = NULL;

	//Obtain the Currently selected Schema
	TESTC(hr = GetSelectedSchema(&pSchemaInfo));

	//Properties
	m_fUseProperties	= ::IsDlgButtonChecked(m_hWndSchemas, IDB_USEPROPERTIES);
	m_fShowSupported	= ::IsDlgButtonChecked(m_hWndSchemas, IDB_SHOWSUPPORTED);

	//Free Previous Restrictions
	FreeData(DBTYPE_VARIANT, m_cRestrictions, m_rgRestrictions);
	m_cRestrictions = 0;
	
	//Obtain Selected Restrictions
	m_fUseRestrictions = ::IsDlgButtonChecked(m_hWndSchemas, IDB_USERESTRICTIONS);
	if(m_fUseRestrictions)
	{
		m_cRestrictions = 0;

		ULONG i;
		//Initialize Restrictions
		for(i=0; i<MAX_RESTRICTIONS; i++)
			VariantInitFast(&m_rgRestrictions[i]);

		//Get the Restriction info
		for(i=0; i<MAX_RESTRICTIONS; i++)
		{
			DBTYPE wType = (DBTYPE)m_rgResType[i].GetItemParam(m_rgResType[i].GetCurSel());
				
			//If Not VT_EMPTY we need to setup the restriction Variant
			if(wType != VT_EMPTY)
			{
				//If user specified, or a "special" selection in the drop down
				//NOTE: Since all the types are defaulted to the actual type of the restriction
				//to make it easier for consumers to not have to remember the correct type.
				//We have to come up with a different way for the user to indicate to use this
				//resctriction.  Currently if the edit box is left blank, we assume the user
				//is not interesed in the restriction and its not bound.  If the user truely
				//wants NULL or Empty String, those options are available in the combo drop down...

				pwszValue = m_rgResValue[i].GetSelText();
				if(pwszValue || m_rgResValue[i].GetCurSel()!=CB_ERR)
				{
					XTESTC(hr = StringToVariant(pwszValue, wType, &m_rgRestrictions[i])); 

					//Record we are using this restrictions
					m_cRestrictions = i+1;
					SAFE_FREE(pwszValue);
				}
			}
		}
	}
			
CLEANUP:
	SAFE_FREE(pwszValue);
	return hr;
}

	
////////////////////////////////////////////////////////////////
// CSchemaDlg::SchemaRowsetProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CSchemaDlg::SchemaRowsetProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE* ps = NULL;
	static BOOL fAggregation	= FALSE;		//Default
	static BOOL fOutput			= TRUE;			//Default
	
	switch(message)
	{
		case WM_INITDIALOG:
		{
			//Save the "this" pointer
			CWaitCursor waitCursor;
			ps = (PROPSHEETPAGE*)lParam;
			CSchemaDlg* pThis = (CSchemaDlg*)SetThis(hWnd, (void*)ps->lParam);

			//Aggregation
			::CheckDlgButton(hWnd, IDB_AGGREGATION,		BST2STATE(fAggregation));

			//Output (ppIUnknown)
			::CheckDlgButton(hWnd, IDB_OUTPUT,			BST2STATE(fOutput));

			//Schemas
			pThis->InitSchemas(hWnd);
			pThis->ChangeRestrictions();
			return TRUE;
		}

		case WM_COMMAND:
		{
			//CBN_SELCHANGE ListBox Selection change
			switch(GET_WM_COMMAND_CMD(wParam, lParam))
			{
				//Selection change in a list box occurred
				case CBN_SELCHANGE:
				{	
					//See which combo box has changed
					switch(GET_WM_COMMAND_ID(wParam, lParam))
					{
						case IDC_SCHEMAS:
						{
							//Get the "this" pointer
							CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);

							//Get Selected Schema
							pThis->ChangeRestrictions();
							return 0;
						}
					}
					break;
				}
				
				//Selection change in a list box occurred
				case CBN_DROPDOWN:
				{	
					//See which combo box has changed
					switch(GET_WM_COMMAND_ID(wParam, lParam))
					{
						case IDC_SCHEMAS:
						{
							//Get the "this" pointer
							CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
							
							//Update button state
							BOOL fShowSupported = ::IsDlgButtonChecked(hWnd, IDB_SHOWSUPPORTED);

							//Get Selected Schema
							//Only need to do this if (ShowSupported) has changed...
							if(fShowSupported != pThis->m_fShowSupported)
							{
								pThis->m_fShowSupported = fShowSupported;
								pThis->InitSchemaCombo(fShowSupported);
							}
							return 0;
						}
					}
					break;
				}

				return FALSE;
			}

			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDB_SETPROPERTIES:
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					CMDIChild* pCMDIChild = pThis->m_pCMDIChild;
					CMainWindow* pCMainWindow = pCMDIChild->m_pCMainWindow;

					CSession* pCSession			= (CSession*)pCMDIChild->GetObject(eCSession);
					CDataSource* pCDataSource	= SOURCE_GETPARENT(pCSession, CDataSource);

					//SetProperties
					CPropertiesDlg sCPropertiesDlg(pCMainWindow);
					sCPropertiesDlg.SetProperties(hWnd, &DBPROPSET_ROWSETALL, IID_IRowsetInfo, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &pThis->m_CPropSets);
					return 0;
				}
			}
			break;

		}//WM_COMMAND

		case WM_NOTIFY:
		{	
			switch (((NMHDR*)lParam)->code) 
    		{
				case PSN_SETACTIVE:
				{	
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					pThis->m_fActive = TRUE;
					return 0;
				}

				case PSN_KILLACTIVE://Switch
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					if(FAILED(pThis->GetSelectedRestrictions()))
					{
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}
					return 0;
				}

				case PSN_APPLY:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					CMDIChild* pCMDIChild = pThis->m_pCMDIChild;

					CSession* pCSession			= (CSession*)pCMDIChild->GetObject(eCSession);
					CAggregate* pCAggregate		= NULL;
					CComPtr<IUnknown> spUnknown;
					HRESULT hr = S_OK;

					if(pThis->m_fEditing)
					{
						//There is a bug in the TreeView control for editing here is KB article
						//Article ID: Q130691 BUG: ESC/ENTER Keys Don't Work When Editing Labels in TreeView
						//So one way to work around this is to just have a flag (m_fEditing)
						//to indicate we were in editing mode.
						SendMessage(pThis->m_hWndTreeView, TVM_ENDEDITLABELNOW, (WPARAM) (wParam==IDCANCEL ? TRUE : FALSE), (LPARAM)0);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}

					if(pThis->m_fActive)
					{
						//Setup Properties
						ULONG cPropSets = 0;
						DBPROPSET* rgPropSets = NULL;
						
						if(pThis->m_fUseProperties)
						{
							cPropSets = pThis->m_CPropSets.GetCount();
							rgPropSets = pThis->m_CPropSets.GetPropSets();
						}

						//Obtain the Aggregation argument
						fAggregation = ::IsDlgButtonChecked(hWnd, IDB_AGGREGATION);
						if(fAggregation)
							pCAggregate = new CAggregate();
						
						//Obtain the Output (ppIUnknown) argument
						fOutput = ::IsDlgButtonChecked(hWnd, IDB_OUTPUT);

						//Get the selected Schema...
						SCHEMAINFO* pSchemaInfo = NULL;
						TESTC(hr = pThis->GetSelectedSchema(&pSchemaInfo));
						
						//Create the Schema Rowset
						TESTC(hr = pCSession->GetSchemaRowset(pCAggregate, *pSchemaInfo->pguidSchema, pThis->m_cRestrictions, pThis->m_rgRestrictions, IID_IUnknown, cPropSets, rgPropSets, fOutput ? &spUnknown : NULL));
						
						//Process the Rowset
						TESTC(hr = pCMDIChild->HandleRowset(pCSession, spUnknown, IID_IUnknown, CREATE_NEWWINDOW_IFEXISTS, *pSchemaInfo->pguidSchema, NULL, TRUE/*bSchemaRowset*/));
					}

				CLEANUP:
					SAFE_RELEASE(pCAggregate);
					if(FAILED(hr))
					{
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}
					return 0;
				}

				case PSN_RESET:		//CANCEL
					return 0;

			}
    	}//WM_NOTIFY

	}//switch(message);

	return FALSE;
}


////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::InitTreeView
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::InitTreeView(HWND hWnd)
{
	HRESULT hr = S_OK;
	m_hWndTreeWnd = hWnd;

	//Saved Defaults
	m_fEditing = FALSE;
	m_iOldSchemaIndex = LVM_ERR;
	m_hWndTreeView = ::GetDlgItem(hWnd, IDC_TREEVIEW);
	
	//Set image list to the Table Window 
	HIMAGELIST hTableImageList = ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR);
	TreeView_SetImageList(m_hWndTreeView, hTableImageList, TVSIL_NORMAL);
	return hr;
}


////////////////////////////////////////////////////////////////
// CSchemaDlg::SchemaTreeProc
//
/////////////////////////////////////////////////////////////////
INT_PTR WINAPI CSchemaDlg::SchemaTreeProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static PROPSHEETPAGE* ps = NULL;

	switch(message)
	{
		case WM_INITDIALOG:
		{
			CWaitCursor waitCursor;

			//Save the "this" pointer
			ps = (PROPSHEETPAGE*)lParam;
			CSchemaDlg* pThis = (CSchemaDlg*)SetThis(hWnd, (void*)ps->lParam);
			
			//Initialize the TreeView Page
			pThis->InitTreeView(hWnd);
			return TRUE;
		}


		case WM_COMMAND:
		{
			//Filter out any Control Notification codes
			if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
			{
				return UNHANDLED_MSG;
			}

			//Regular command messages
			switch(GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDM_OPENROWSET:
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);

					//Send a IDM_COPYTEXT to copy the text to the edit box
					SendMessage(hWnd, WM_COMMAND, GET_WM_COMMAND_MPS(IDM_COPYTEXT, hWnd, 0));

					//Delete to our OpenRowset dialog box...
					pThis->m_pCMDIChild->m_pCSource = pThis->m_pCMDIChild->GetObject(eCSession);
					pThis->m_pCMDIChild->m_idSource = IDM_OPENROWSET;
					DisplayDialog(IDD_OPENROWSET, hWnd, pThis->m_pCMDIChild->OpenRowsetProc, (LPARAM)pThis->m_pCMDIChild);
					return 0;
				}		  

				case IDM_COPYTEXT:
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					WCHAR wszBuffer[MAX_NAME_LEN+1];
					
					//Get the Selected Item
					HTREEITEM hItem = (HTREEITEM)SendMessage(pThis->m_hWndTreeView, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, (LPARAM)TVI_ROOT);
					
					if(TV_GetItemText(pThis->m_hWndTreeView, hItem, wszBuffer, MAX_NAME_LEN))
					{
						//Now just need to place this name in the EditBox
						//Inserted after the current "caret"
						pThis->m_pCMDIChild->m_pCQueryBox->ReplaceAll(wszBuffer, FALSE/*bReplaceAll*/, TRUE/*fHighlight*/);
					}
					return 0;
				}		  
			
				case IDB_DISPLAYTREE:
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);

					//Make sure we do a regeneration of the tree
					pThis->DisplayTree();
					return 0;
				}
			}
		
			return FALSE;
		}

		// Update status bar to reflect menu selection
		case WM_MENUSELECT:
		{	
			//Get the "this" pointer
			CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
			pThis->m_pCMDIChild->m_pCMainWindow->OnMenuSelect(LOWORD(wParam));
			return 0;
		}

		case WM_CONTEXTMENU:
		{	
			CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
		
			//Only display the Context menu, if there is a selected item
			HTREEITEM hItem = (HTREEITEM)SendMessage(pThis->m_hWndTreeView, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, (LPARAM)TVI_ROOT);

			if(hItem)
			{
				DisplayContextMenu( 
								hWnd,
								IDM_TREESCHEMA, 
								MAKEPOINTS(lParam),
								hWnd
								);
			}
			return FALSE;
		}

		case WM_NOTIFY:
		{	
			switch (((NMHDR*)lParam)->code) 
    		{
				case TVN_BEGINLABELEDIT:
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);

					//Idicate we have started to edit
					pThis->m_fEditing = TRUE;
					return FALSE; //Allow the edited change
				}

				case TVN_ENDLABELEDIT:
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					pThis->m_fEditing = FALSE;
					return TRUE; //Allow the edited change
				}

				case TVN_SELCHANGED:
				{
					//There is a problem with the SELCHANGED notification
					//It can be sent when either a item is selected or
					//DELETED, since when an item deleted the selection moves
					//to a different selection.
					NM_TREEVIEW* pTreeView = (NM_TREEVIEW*)lParam;
					if(pTreeView->itemNew.state & TVIS_SELECTED && pTreeView->action)
					{
						//Get the "this" pointer
						CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
						pThis->RefreshTreeControls();
					}
					return 0;
				}

				case TTN_GETDISPINFO:
				{	
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					return 0;
				}
				
				case PSN_SETACTIVE:
				{	
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
			
					//Need to display the specified SchemaRowset
					pThis->RefreshTreeControls();
					pThis->m_fActive = FALSE;
					return 0;
				}

				case PSN_KILLACTIVE://Switch
					return 0;

				case PSN_APPLY:
				{
					CWaitCursor waitCursor;

					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					if(pThis->m_fEditing)
					{
						//There is a bug in the TreeView control for editing here is KB article
						//Article ID: Q130691 BUG: ESC/ENTER Keys Don't Work When Editing Labels in TreeView
						//So one way to work around this is to just have a flag (m_fEditing)
						//to indicate we were in editing mode.
						SendMessage(pThis->m_hWndTreeView, TVM_ENDEDITLABELNOW, (WPARAM) (wParam==IDCANCEL ? TRUE : FALSE), (LPARAM)0);
						SetWindowLongPtr(hWnd, DWLP_MSGRESULT, PSNRET_INVALID_NOCHANGEPAGE);
						return TRUE;
					}
					
					SAFE_RELEASE(pThis->m_pCTreeRowset);
					return 0;
				}

				case PSN_RESET:		//CANCEL
				{
					//Get the "this" pointer
					CSchemaDlg* pThis = (CSchemaDlg*)GetThis(hWnd);
					SAFE_RELEASE(pThis->m_pCTreeRowset);
					return 0;
				}

			}
    	}//WM_NOTIFY

	}//switch(message);

	return FALSE;
}


////////////////////////////////////////////////////////////////
// HRESULT CSchemaDlg::RefreshTreeControls
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::RefreshTreeControls()
{
	HWND hWndColName = ::GetDlgItem(m_hWndTreeWnd, IDT_COLUMNNAME);

	if(m_iSchemaIndex != m_iOldSchemaIndex)
	{
		::EnableWindow(m_hWndTreeView, FALSE);
		::EnableWindow(hWndColName, FALSE);
		::EnableWindow(::GetDlgItem(m_hWndTreeWnd, IDM_COPYTEXT), FALSE);
		::EnableWindow(::GetDlgItem(m_hWndTreeWnd, IDM_OPENROWSET), FALSE);
	}
	else
	{
		::EnableWindow(m_hWndTreeView, TRUE);
		::EnableWindow(hWndColName, TRUE);

		//Get the Selected TreeItem
		HTREEITEM hItem = (HTREEITEM)SendMessage(m_hWndTreeView, TVM_GETNEXTITEM, (WPARAM)TVGN_CARET, (LPARAM)TVI_ROOT);

		//Update the ColName header...
		ASSERT(m_pCTreeRowset);
		
		DBORDINAL iOrdinal = TV_GetItemParam(m_hWndTreeView, hItem);
		if(hItem && iOrdinal != LVM_ERR && iOrdinal<m_pCTreeRowset->m_ColumnInfo.GetCount())
		{
			wSendMessage(hWndColName, WM_SETTEXT, 0, GetColName(m_pCTreeRowset->m_ColumnInfo.GetOrdinal(iOrdinal)));
			::EnableWindow(::GetDlgItem(m_hWndTreeWnd, IDM_COPYTEXT), TRUE);
			::EnableWindow(::GetDlgItem(m_hWndTreeWnd, IDM_OPENROWSET), TRUE);
		}
		else
		{
			SendMessage(hWndColName, WM_SETTEXT, 0, (LPARAM)"");
			::EnableWindow(::GetDlgItem(m_hWndTreeWnd, IDM_COPYTEXT), FALSE);
			::EnableWindow(::GetDlgItem(m_hWndTreeWnd, IDM_OPENROWSET), FALSE);
		}
	}

	return S_OK;
}


////////////////////////////////////////////////////////////////
// CSchemaDlg::DisplayTree
//
/////////////////////////////////////////////////////////////////
HRESULT CSchemaDlg::DisplayTree()
{
	CWaitCursor waitCursor;
	HRESULT hr = E_FAIL;
	WCHAR wszBuffer[MAX_COL_SIZE];

	DBCOUNTITEM iCol,cRowsObtained = 0;
	HROW  rghRows[MAX_BLOCK_SIZE];
	HROW*  phRow = rghRows;
	
	DBORDINAL cColOrds = 0;
	DBORDINAL* rgColOrds = NULL;

	DBORDINAL iOrdinal, iColumn;
	HTREEITEM hFoundItem, hParent;

	//Setup Properties
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	SCHEMAINFO* pSchemaInfo = NULL;
	CComPtr<IRowset> spRowset;
	CSession* pCSession = (CSession*)m_pCMDIChild->GetObject(eCSession);
	
	if(m_fUseProperties)
	{
		cPropSets = m_CPropSets.GetCount();
		rgPropSets = m_CPropSets.GetPropSets();
	}

	//Get the Selected Schema
	TESTC(hr = GetSelectedSchema(&pSchemaInfo));

	//Create the SchemaRowset
	TESTC(hr = pCSession->GetSchemaRowset(NULL, *pSchemaInfo->pguidSchema, m_cRestrictions, m_rgRestrictions, IID_IRowset, cPropSets, rgPropSets, (IUnknown**)&spRowset));
	
	//(either from Command, IOpenRowset, or Schema's
	if(m_pCTreeRowset == NULL)
		m_pCTreeRowset = new CRowset(NULL, m_pCMDIChild);
	TESTC(hr = m_pCTreeRowset->CreateObject(pCSession, IID_IRowset, spRowset));

	//Record that the source is a SchemaRowset...
	if(pSchemaInfo->pguidSchema)
	{
		m_pCTreeRowset->m_guidSource	= *pSchemaInfo->pguidSchema;
		m_pCTreeRowset->m_bSchemaRowset	= TRUE;
	}

	//Clear TreeView object
	SendMessage(m_hWndTreeView, TVM_DELETEITEM, 0, (LPARAM)TVI_ROOT);

	//Create ColumnInfo
	TESTC(hr = m_pCTreeRowset->GetColInfo());

	///Create Accessors and Setup bindings, (don't bind bookmark)
	TESTC(hr = m_pCTreeRowset->CreateAccessors(BIND_ALLCOLSEXPECTBOOKMARK));

	//All columns that are restrictions are done first, this creates the tree
	//nodes.  Then all remaining columns are just elements under the last node.
	//We need to setup this array of ordinals to use to indicate the ordering...
	SAFE_ALLOC(rgColOrds, DBORDINAL, m_pCTreeRowset->m_ColumnInfo.GetCount());

	//Add all restriction columns first
	cColOrds = pSchemaInfo->cRestrictions;
	for(iCol=0; iCol<cColOrds; iCol++)
	{
		ASSERT(pSchemaInfo->rgResInfo[iCol].iNode < m_pCTreeRowset->m_ColumnInfo.GetCount());
		rgColOrds[iCol] = pSchemaInfo->rgResInfo[iCol].iNode;
	}
	
	//Add all remaining columns, (that are not already in the list...)
	iColumn = cColOrds;
	for(iCol=0; iCol<m_pCTreeRowset->m_Bindings.GetCount(); iCol++)
	{
		BOOL bFound = FALSE;
		iOrdinal = m_pCTreeRowset->m_Bindings[iCol].iOrdinal;
		for(DBCOUNTITEM j=0; j<cColOrds && !bFound; j++)
		{
			if(iOrdinal == rgColOrds[j])
				bFound = TRUE;
		}

		//Add it to the list, if it doesn't already exist...
		if(!bFound)
		{
			rgColOrds[iColumn] = iOrdinal;	
			iColumn++;
		}
	}

	//Grab all the rows
	while(TRUE)
	{
		//Obtain the rows in blocks...
		//NOTE: We are using a "static" array passed in for GetNextRows.  This is a major performance
		//gain that is allowed by the spec, so the provider doesn't have to allocate the array
		//or guess how large, and since we are reusing it it in a loop it has signicant gains...
		TESTC(hr = m_pCTreeRowset->GetNextRows(0, MAX_BLOCK_SIZE, &cRowsObtained, &phRow));
		
		//NOTE: We cant just use (HROW**)&rghRows, since the compiler always "assumes" &array = array!
		//Which is not what we want, you actually have to use a temp for the currect address.  This
		//assert is to make sure the provider doesn't allocate an array when the user is 
		//passing in a user allocated array for performance...
		ASSERT(phRow == rghRows);

		//ENDOFROWSET
		if(cRowsObtained==0)
			break;
	
		//Loop through all rows obtained...
		for(ULONG iRow=0; iRow<cRowsObtained; iRow++)
		{
			//Get the Data
			TESTC(hr = m_pCTreeRowset->GetData(rghRows[iRow]));

			//Loop through all the restrictions first
			//We will create nodes based upon the sort order of the restrictions.
			//Any column that is not in the sort oder will just be an item under a node
			hParent = NULL;
			for(ULONG i=0; i<cColOrds; i++)
			{
				//Get the Data for this Column
				const DBBINDING* pBinding = m_pCTreeRowset->m_Bindings.GetOrdinal(rgColOrds[i]);
				const DBCOLUMNINFO* pColInfo = m_pCTreeRowset->m_ColumnInfo.GetOrdinal(rgColOrds[i]);
				m_pCTreeRowset->GetColumnData(pBinding, m_pCTreeRowset->m_pData, NULL, NULL, NULL, wszBuffer, MAX_COL_SIZE, GetOptions()->m_dwConvFlags | CONV_TYPENAME, pColInfo->wType);
				
				//Insert it into the TreeView
				if(wszBuffer[0])
				{
					//Make sure it doesn't already exist...
					hFoundItem = TV_FindItem(m_hWndTreeView, hParent, wszBuffer);
					if(hFoundItem)
					{
						hParent = hFoundItem;
					}
					else
					{
						//Insert the Item (using node Icon - 0)
						hParent = TV_InsertItem(m_hWndTreeView, hParent, TVI_LAST, wszBuffer, rgColOrds[i], IMAGE_FORM, IMAGE_FORM);
					}
				}
			}

			//Now go through the regular non-node items and just 
			//add the the latest parent
			for(iCol=cColOrds; iCol<m_pCTreeRowset->m_Bindings.GetCount(); iCol++)
			{
				const DBBINDING* pBinding = m_pCTreeRowset->m_Bindings.GetOrdinal(rgColOrds[iCol]);
				const DBCOLUMNINFO* pColInfo = m_pCTreeRowset->m_ColumnInfo.GetOrdinal(rgColOrds[iCol]);

				//Get the Data for this Column
				m_pCTreeRowset->GetColumnData(pBinding, m_pCTreeRowset->m_pData, NULL, NULL, NULL, wszBuffer, MAX_COL_SIZE, GetOptions()->m_dwConvFlags | CONV_TYPENAME, pColInfo->wType);

				//Insert it into the TreeView
				if(wszBuffer[0])
				{
					//Display the Item (using item icon - 1)
					TV_InsertItem(m_hWndTreeView, hParent, TVI_LAST, wszBuffer, rgColOrds[iCol], IMAGE_NORMAL, IMAGE_NORMAL);
				}
			}

			//Free the out-of-line data
			m_pCTreeRowset->m_Bindings.FreeData(m_pCTreeRowset->m_pData);
		}

		//Release the rows obtained
		TESTC(hr = m_pCTreeRowset->ReleaseRows(cRowsObtained, rghRows));
	}

	//We now have displayed this tree
	m_iOldSchemaIndex = m_iSchemaIndex;

CLEANUP:
	SAFE_FREE(rgColOrds);
	RefreshTreeControls();
	return hr;
}




/////////////////////////////////////////////////////////////////////////////
// CComboBoxGuid
//
/////////////////////////////////////////////////////////////////////////////
CComboBoxGuid::CComboBoxGuid()
{
	m_cGuidMaps		= 0;
	m_rgGuidMaps	= NULL;
}


/////////////////////////////////////////////////////////////////////////////
// ~CComboBoxGuid
//
/////////////////////////////////////////////////////////////////////////////
CComboBoxGuid::~CComboBoxGuid()
{
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxGuid::Populate
//
/////////////////////////////////////////////////////////////////////////////
BOOL CComboBoxGuid::Populate(ULONG cGuidMaps, const WIDEGUIDMAP* rgGuidMaps)
{
	m_cGuidMaps		= cGuidMaps;
	m_rgGuidMaps	= rgGuidMaps;
	
	//Remove any existing Data
	ResetContent();

	//Fill in the Combo Box...
	for(ULONG i=0; i<cGuidMaps; i++)
	{
		AddString(rgGuidMaps[i].pwszName, (ULONG_PTR)&rgGuidMaps[i]);
	}

	//Restore the Default Selection...
	RestoreSelection();
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxGuid::SetGuid
//
/////////////////////////////////////////////////////////////////////////////
INDEX CComboBoxGuid::SetGuid(REFIID riid)
{
	//Find the specified guid...
	for(ULONG i=0; i<m_cGuidMaps; i++)
	{
		WIDEGUIDMAP* pGuidMap = (WIDEGUIDMAP*)GetItemParam(i);
		
		//Is this a match
		if(pGuidMap && *pGuidMap->pGuid == riid)
		{
			return SetCurSel(i);
		}
	}

	return CB_ERR;
}


/////////////////////////////////////////////////////////////////////////////
// REFIID CComboBoxGuid::GetGuid
//
/////////////////////////////////////////////////////////////////////////////
REFIID CComboBoxGuid::GetGuid()
{
	INDEX iSel = GetCurSel();
	WIDEGUIDMAP* pGuidMap = (WIDEGUIDMAP*)GetItemParam(iSel);
	
	if(iSel != CB_ERR && pGuidMap)
	{
		SaveSelection();
		return *pGuidMap->pGuid;
	}

	return GUID_NULL;
}





/////////////////////////////////////////////////////////////////////////////
// CComboBoxString
//
/////////////////////////////////////////////////////////////////////////////
CComboBoxString::CComboBoxString()
{
}


/////////////////////////////////////////////////////////////////////////////
// ~CComboBoxString
//
/////////////////////////////////////////////////////////////////////////////
CComboBoxString::~CComboBoxString()
{
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxString::OnInitialUpdate
//
/////////////////////////////////////////////////////////////////////////////
BOOL CComboBoxString::OnInitialUpdate()
{
	//Populate the <NULL> option
	AddString(L"<NULL>",			VT_NULL	/*NULL pointer*/);
	AddString(L"<Empty String>",	VT_EMPTY/*Empty String*/);
	
	//Delegate
	return CComboBoxLite::OnInitialUpdate();
}


/////////////////////////////////////////////////////////////////////////////
// CComboBoxString::GetSelText
//
/////////////////////////////////////////////////////////////////////////////
WCHAR* CComboBoxString::GetSelText()
{
	INDEX iSel = GetCurSel();
	if(iSel == CB_ERR)
	{
		//Delegate
		return CComboBoxLite::GetWindowText();
	}
	
	DBTYPE wType = (DBTYPE)GetItemParam(iSel);
	switch(wType)
	{
		case VT_NULL:
			//Null pointer
			return NULL;

		case VT_EMPTY:
			//Empty string
			return wcsDuplicate(L"");
	
		default:
			ASSERT(!"Unhandled Drop Down!");
			break;
	};

	return NULL;
}



/////////////////////////////////////////////////////////////////////////////
// CInterfaceDlg
//
/////////////////////////////////////////////////////////////////////////////
CInterfaceDlg::CInterfaceDlg(UINT nID, WCHAR* pwszTitle, REFIID riid)
	: CDialogLite(nID)
{
	//Title
	m_pwszTitle		= pwszTitle;
	
	//Aggregation
	m_fAggregation	= FALSE;	//Default
	m_pCAggregate	= NULL;
	
	//riid
	m_iid			= riid;

	//Output
	m_fOutput		= TRUE;		//Default
	m_pIUnknown		= NULL;
	m_ppIUnknown	= NULL;
}


/////////////////////////////////////////////////////////////////////////////
// ~CInterfaceDlg
//
/////////////////////////////////////////////////////////////////////////////
CInterfaceDlg::~CInterfaceDlg()
{
	//Aggregation
	SAFE_RELEASE(m_pCAggregate);

	//Output
	SAFE_RELEASE(m_pIUnknown);
}


/////////////////////////////////////////////////////////////////////////////
// CInterfaceDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CInterfaceDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Update Title and Window
	if(m_pwszTitle)
		wSendMessage(m_hWnd, WM_SETTEXT, 0, m_pwszTitle);

	//Setup the Aggregation Button
	SAFE_RELEASE(m_pCAggregate);
	CheckDlgButton(IDB_AGGREGATION, BST2STATE(m_fAggregation));		//Default
	
	//Setup the Interface Combo
	m_CComboInterface.CreateIndirect(m_hWnd, IDC_INTERFACE);
	m_CComboInterface.Populate(g_cInterfaceMaps, g_rgInterfaceMaps);
	m_CComboInterface.SetGuid(m_iid);

	//Setup the ppIUnknown Button
	SAFE_RELEASE(m_pIUnknown);
	CheckDlgButton(IDB_OUTPUT, BST2STATE(m_fOutput));			//Default
	
	CenterDialog(m_hWnd);
	return CDialogLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CInterfaceDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CInterfaceDlg::OnOK()
{
	CWaitCursor waitCursor;

	//Update all our items...
	OnUpdate();

	//Delegate
	return CDialogLite::OnOK();
}


/////////////////////////////////////////////////////////////////////////////
// CInterfaceDlg::OnUpdate
//
/////////////////////////////////////////////////////////////////////////////
BOOL CInterfaceDlg::OnUpdate()
{
	//Obtain the Aggregation argument
	if(IsDlgButtonChecked(IDB_AGGREGATION))
		m_pCAggregate = new CAggregate();

	//Obtain the riid argument
	m_iid = m_CComboInterface.GetGuid();

	//Obtain the Output (ppIUnknown) argument
	if(IsDlgButtonChecked(IDB_OUTPUT))
		m_ppIUnknown = &m_pIUnknown;

	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CInterfaceDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CInterfaceDlg::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		//File Menu
		ON_COMMAND(IDB_AGGREGATION,		OnAggregation())
	}

	return CDialogLite::OnCommand(iID, hWndCtrl);
}

/////////////////////////////////////////////////////////////////////////////
// CInterfaceDlg::OnAggregation
//
/////////////////////////////////////////////////////////////////////////////
BOOL CInterfaceDlg::OnAggregation() 
{
	//Aggregation Combo Selection has changed...
	//If we are now using Aggregation, automatically change the requested
	//riid to IID_IUnknown, since its an error otherwise...
	if(IsDlgButtonChecked(IDB_AGGREGATION))
	{
		m_CComboInterface.SetGuid(IID_IUnknown);
	}

	return TRUE;
}




/////////////////////////////////////////////////////////////////////////////
// CEditValueDlg
//
/////////////////////////////////////////////////////////////////////////////
CEditValueDlg::CEditValueDlg(UINT nID, WCHAR* pwszTitle)
	: CDialogLite(nID)
{
	ASSERT(pwszTitle);

	//Title
	m_pwszTitle		= pwszTitle;

	//String
	m_pwszValue		= NULL;
	m_wReqType		= DBTYPE_WSTR;
}


/////////////////////////////////////////////////////////////////////////////
// ~CEditValueDlg
//
/////////////////////////////////////////////////////////////////////////////
CEditValueDlg::~CEditValueDlg()
{
	//Value
	SAFE_FREE(m_pwszValue);
}


/////////////////////////////////////////////////////////////////////////////
// CEditValueDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CEditValueDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Update Title and Window
	SetWindowText(m_pwszTitle);

	//Setup the Value
	m_editValue.CreateIndirect(m_hWnd, IDE_VALUE);
	if(m_pwszValue)
		m_editValue.SetWindowText(m_pwszValue);
	
	//Delegate
	CenterDialog(m_hWnd);
	return CDialogLite::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CEditValueDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CEditValueDlg::OnOK()
{
	CWaitCursor waitCursor;

	if(m_wReqType)
	{
		//Obtain the value argument
		SAFE_FREE(m_pwszValue);
		m_pwszValue = m_editValue.GetWindowText();

		//Need to validate (and obtain) the value, so we know it can be converted to the requested type.
		//If not then do not continue (return FALSE), so the user gets another chance
		//to correct the value...
		if(m_wReqType != DBTYPE_WSTR)
		{
			CVariant cVariant;
			if(FAILED(StringToVariant(m_pwszValue, m_wReqType, &cVariant)))
			{
				wMessageBox(m_editValue.m_hWnd, MB_TASKMODAL | MB_ICONERROR | MB_OK,  wsz_ERROR, 
					L"Unable to convert specified string \"%s\", to %s", m_pwszValue, GetDBTypeName(m_wReqType));
				m_editValue.SetFocus();
				return FALSE;
			}
		}
	}

	//Delegate
	return CDialogLite::OnOK();
}


/////////////////////////////////////////////////////////////////////////////
// CEditValueDlg::SetVariant
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEditValueDlg::SetVariant(VARIANT* pVariant)
{
	HRESULT hr = S_OK;
	ASSERT(pVariant);
	CVariant cVariant;
	
	//Make sure we can convert to a string value
	hr = cVariant.ChangeType(DBTYPE_BSTR, pVariant);
	if(SUCCEEDED(hr))
	{
		ASSERT(cVariant.vt == VT_BSTR);
		SetValue(V_BSTR(&cVariant));
	}
	return hr;
}


/////////////////////////////////////////////////////////////////////////////
// CEditValueDlg::GetVariant
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CEditValueDlg::GetVariant(DBTYPE wType, VARIANT* pVariant)
{
	ASSERT(pVariant);
	ASSERT(m_wReqType);

	return StringToVariant(m_pwszValue, wType, pVariant);
}



	


/////////////////////////////////////////////////////////////////////////////
// CMultipleResultsDlg
//
/////////////////////////////////////////////////////////////////////////////
CMultipleResultsDlg::CMultipleResultsDlg(CMDIChild* pCMDIChild)
	: CInterfaceDlg(IDD_MULTIPLERESULTS, NULL, IID_IRowset), CRowsetViewerDlg(NULL, pCMDIChild)
{
	//Data
}


/////////////////////////////////////////////////////////////////////////////
// ~CMultipleResultsDlg
//
/////////////////////////////////////////////////////////////////////////////
CMultipleResultsDlg::~CMultipleResultsDlg()
{
}


/////////////////////////////////////////////////////////////////////////////
// CMultipleResultsDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMultipleResultsDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Populate the lResultFlag Flag
	m_CComboReserved.CreateIndirect(m_hWnd, IDC_RESERVED);
	m_CComboReserved.Populate(g_cResultFlagMaps, g_rgResultFlagMaps);
	m_CComboReserved.SetSelValue(DBRESULTFLAG_DEFAULT);

	//Delegate
	return CInterfaceDlg::OnInitDialog();
}


/////////////////////////////////////////////////////////////////////////////
// CMultipleResultsDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CMultipleResultsDlg::OnOK()
{
	CWaitCursor waitCursor;
	HRESULT hr = S_OK;

	//Obtain the MultipleResults object
	CMultipleResults* pCMultipleResults = SOURCE_GETOBJECT(m_pCMDIChild->m_pCSource, CMultipleResults);
	if(pCMultipleResults)
	{
		DBROWCOUNT	cRowsAffected = 0;

		//Obtain inherited items...
		CInterfaceDlg::OnUpdate();

		//lResultFlag Flag
		//Used to determine which object to obtain when riid is ambigous
		DB_LRESERVE	lResultFlag = m_CComboReserved.GetSelValue();
		if(lResultFlag == CB_ERR)
			lResultFlag = DBRESULTFLAG_DEFAULT;

		//IMultipleResults::GetResult...
		//NOTE: Can pontentially return other object types: (ie: CREATE_DETERMINE_TYPE)
		if(SUCCEEDED(hr = pCMultipleResults->GetResult(GetAggregate(), lResultFlag, GetSelInterface(), &cRowsAffected, ppUnknown())))
			m_pCMainWindow->HandleObjectType(pCMultipleResults, pUnknown(), GetSelInterface(), eCRowset, 0, NULL, CREATE_NEWWINDOW_IFEXISTS | CREATE_DETERMINE_TYPE);
	}

//CLEANUP:
	if(SUCCEEDED(hr))
		CDialogLite::OnOK();
	return TRUE;
}



////////////////////////////////////////////////////////////////
// CConstraintDlg::CConstraintDlg
//
/////////////////////////////////////////////////////////////////
CConstraintDlg::CConstraintDlg(CMainWindow* pCMainWindow)
	: CDialogLite(IDD_ADDCONSTRAINTDESC), CRowsetViewerDlg(pCMainWindow)
{
	m_pConsDesc		= NULL;
	m_ppwszTableName	= NULL;
} //CConstraintDlg::CConstraintDlg


////////////////////////////////////////////////////////////////
// CConstraintDlg::~CConstraintDlg
//
/////////////////////////////////////////////////////////////////
CConstraintDlg::~CConstraintDlg()
{
} //CConstraintDlg::~CConstraintDlg


////////////////////////////////////////////////////////////////
// CConstraintDlg::SetConstraintAndTable
//
/////////////////////////////////////////////////////////////////
HRESULT CConstraintDlg::SetConstraintAndTable(							
	HWND				hWndParent,
	DBCONSTRAINTDESC	*pConsDesc,			
	WCHAR				**ppwszTableName
)
{
	m_pConsDesc			= pConsDesc;
	m_ppwszTableName	= ppwszTableName;

	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;	
} //CConstraintDlg::SetConstraintAndTable


/////////////////////////////////////////////////////////////////////
// CConstraintDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////
BOOL CConstraintDlg::OnInitDialog()
{
	CWaitCursor waitCursor;
	INDEX		iSel;
	ULONG		index;

	//Setup Controls
	m_comboConstraintType.CreateIndirect(m_hWnd, IDC_CONSTYPE);
	m_listDeferrability.CreateIndirect(m_hWnd, IDC_DEFERRABILITY);
	m_listColumnList.CreateIndirect(m_hWnd, IDC_COLLIST);
	m_listFKColumnList.CreateIndirect(m_hWnd, IDC_FKCOLLIST);
	m_comboMatchType.CreateIndirect(m_hWnd, IDC_MATCHTYPE);
	m_comboUpdateRule.CreateIndirect(m_hWnd, IDC_UPDATERULE);
	m_comboDeleteRule.CreateIndirect(m_hWnd, IDC_DELETERULE);

	m_editConstraintName.CreateIndirect(m_hWnd, IDE_CONSNAME);
	m_editBaseTableName.CreateIndirect(m_hWnd, IDE_TABLEID);
	m_editReferencedTableName.CreateIndirect(m_hWnd, IDE_REFERENCEDTABLE);
	m_editConstraintText.CreateIndirect(m_hWnd, IDE_CONSTEXT);
	m_editColumnName.CreateIndirect(m_hWnd, IDE_COLNAME);
	m_editFKColumnName.CreateIndirect(m_hWnd, IDE_FKCOLNAME);

	//Now adjust display
	if(NULL != m_ppwszTableName)
	{
		SetWindowText(L"ITableDefinitionWithConstraints::AddConstraint");
		m_editBaseTableName.EnableWindow(TRUE);
	}
	else
	{
		SetWindowText(L"ITableDefinitionWithConstraints::CreateTableWithConstraints");
		m_editBaseTableName.EnableWindow(FALSE);
	}
	
	//Fill in the constraint type list
	for (index = 0; index < g_cConsTypeMaps; index++)
	{
		iSel = m_comboConstraintType.AddString(g_rgConsTypeMaps[index].pwszName);
		m_comboConstraintType.SetItemParam(iSel, g_rgConsTypeMaps[index].lItem);
	}
	m_comboConstraintType.SetCurSel(1);

	//Fill in the deferrability list
	for (index = 0; index < g_cDeferrabilityMaps; index++)
	{
		iSel = m_listDeferrability.AddString(g_rgDeferrabilityMaps[index].pwszName);
		m_listDeferrability.SetItemParam(iSel, g_rgDeferrabilityMaps[index].lItem);
	}

	//Fill in the match type list
	for (index = 0; index < g_cMatchTypeMaps; index++)
	{
		iSel = m_comboMatchType.AddString(g_rgMatchTypeMaps[index].pwszName);
		m_comboMatchType.SetItemParam(iSel, g_rgMatchTypeMaps[index].lItem);
	}

	//Fill in the update and delete list
	for (index = 0; index < g_cUpDelRuleMaps; index++)
	{
		// update list
		iSel = m_comboUpdateRule.AddString(g_rgUpDelRuleMaps[index].pwszName);
		m_comboUpdateRule.SetItemParam(iSel, g_rgUpDelRuleMaps[index].lItem);
		
		// delete list
		iSel = m_comboDeleteRule.AddString(g_rgUpDelRuleMaps[index].pwszName);
		m_comboDeleteRule.SetItemParam(iSel, g_rgUpDelRuleMaps[index].lItem);
	}

	//Delegate
	return CDialogLite::OnInitDialog();
} // CConstraintDlg::OnInitDialog




/////////////////////////////////////////////////////////////////////////////
// CConstraintDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CConstraintDlg::OnCommand(UINT iID, HWND hWndCtrl) 
{
	WCHAR	*pwszColName = NULL;

	switch(iID)
	{
		case IDB_ADDCOL:
		{
			// add column to the list of columns
			pwszColName = m_editColumnName.GetWindowText();
			if (pwszColName)
				m_listColumnList.AddString(pwszColName);
			SAFE_FREE(pwszColName);
			return TRUE;
		}

		case IDB_ADDFKCOL:
		{
			// add column to the list of columns
			pwszColName = m_editFKColumnName.GetWindowText();
			if (pwszColName)
				m_listFKColumnList.AddString(pwszColName);
			SAFE_FREE(pwszColName);
			return TRUE;
		}

		case IDB_DELCOL:
		{
			m_listColumnList.DeleteString(m_listColumnList.GetCurSel());
			return TRUE;
		}
		
		case IDB_DELFKCOL:
		{
			m_listFKColumnList.DeleteString(m_listFKColumnList.GetCurSel());
			return TRUE;
		}
	};

	return FALSE;
} //CConstraintDlg::OnCommand


/////////////////////////////////////////////////////////////////////////////
// CConstraintDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CConstraintDlg::OnOK()
{
	CWaitCursor waitCursor;
	HWND		hWndDeferrability = GetDlgItem(IDC_DEFERRABILITY);
	const ULONG	cMaxSel = 2;
	INDEX		cSel;
	INT			rgSel[cMaxSel];
	DBORDINAL	index;
	HRESULT		hr;
	WCHAR		*pwszText	= NULL;

	// fill in table name
	if (m_ppwszTableName)
	{
		*m_ppwszTableName = this->m_editBaseTableName.GetWindowText();
	}

	// fill in pConsDesc structure
	if (m_pConsDesc)
	{
		WCHAR	*pwszConsName = m_editConstraintName.GetWindowText();

		// get constraint name
		if (pwszConsName)
		{
			SAFE_ALLOC(m_pConsDesc->pConstraintID, DBID, 1);
			m_pConsDesc->pConstraintID->eKind			= DBKIND_NAME;
			m_pConsDesc->pConstraintID->uName.pwszName	= pwszConsName ;
		}

		// get the type
		m_pConsDesc->ConstraintType = (DBCONSTRAINTTYPE)m_comboConstraintType.GetItemParam(m_comboConstraintType.GetCurSel());
		
		// get deferrability
		m_pConsDesc->Deferrability = 0;
		cSel = SendMessage(hWndDeferrability, LB_GETSELITEMS, (WPARAM)cMaxSel, (LPARAM)rgSel);

		if (1 == cSel)
			m_pConsDesc->Deferrability |= m_listDeferrability.GetItemParam(rgSel[0]);
		else if (2 == cSel)
			m_pConsDesc->Deferrability = DBDEFERRABILITY_DEFERRED | DBDEFERRABILITY_DEFERRABLE;
		
		// get text of constraint
		m_pConsDesc->pwszConstraintText = m_editConstraintText.GetWindowText();

		// get list of column names
		m_pConsDesc->cColumns = m_listColumnList.GetCount();
		if (m_pConsDesc->cColumns)
		{
			SAFE_ALLOC(m_pConsDesc->rgColumnList, DBID, m_pConsDesc->cColumns);
			for (index = 0; index < m_pConsDesc->cColumns; index++)
			{
				m_pConsDesc->rgColumnList[index].eKind = DBKIND_NAME;
				SAFE_ALLOC(m_pConsDesc->rgColumnList[index].uName.pwszName, WCHAR, 
					m_listColumnList.GetTextLen(index)+1);
				m_listColumnList.GetText(index, m_pConsDesc->rgColumnList[index].uName.pwszName);
			}
		}

		// get referenced table name
		pwszText = m_editReferencedTableName.GetWindowText();
		if (pwszText)
		{
			SAFE_ALLOC(m_pConsDesc->pReferencedTableID, DBID, 1);
			m_pConsDesc->pReferencedTableID->eKind			= DBKIND_NAME;
			m_pConsDesc->pReferencedTableID->uName.pwszName	= pwszText;
		}

		// get list of foreign key column names
		m_pConsDesc->cForeignKeyColumns = m_listFKColumnList.GetCount();
		if (m_pConsDesc->cForeignKeyColumns)
		{
			SAFE_ALLOC(m_pConsDesc->rgForeignKeyColumnList, DBID, m_pConsDesc->cForeignKeyColumns);
			for (index = 0; index < m_pConsDesc->cForeignKeyColumns; index++)
			{
				m_pConsDesc->rgForeignKeyColumnList[index].eKind = DBKIND_NAME;
				SAFE_ALLOC(m_pConsDesc->rgForeignKeyColumnList[index].uName.pwszName, WCHAR, 
					m_listFKColumnList.GetTextLen(index)+1);
				m_listFKColumnList.GetText(index, m_pConsDesc->rgForeignKeyColumnList[index].uName.pwszName);
			}
		}

		// get match type
		m_pConsDesc->MatchType = (DBMATCHTYPE)m_comboMatchType.GetItemParam(m_comboMatchType.GetCurSel());

		// get update rule
		m_pConsDesc->UpdateRule = (DBUPDELRULE)m_comboUpdateRule.GetItemParam(m_comboUpdateRule.GetCurSel());
		
		// get delete rule 
		m_pConsDesc->DeleteRule = (DBUPDELRULE)m_comboDeleteRule.GetItemParam(m_comboDeleteRule.GetCurSel());
	}

CLEANUP:
	//Delegate
	return CDialogLite::OnOK();
} //CConstraintDlg::OnOK




////////////////////////////////////////////////////////////////
// CAlterIndexDlg::CAlterIndexDlg
//
/////////////////////////////////////////////////////////////////
CAlterIndexDlg::CAlterIndexDlg(CMDIChild *pCMDIChild)
	: CDialogLite(IDD_ALTERINDEX), CRowsetViewerDlg(NULL, pCMDIChild)
{
	m_ppTableID			= NULL;
	m_ppIndexID			= NULL;
	m_ppNewIndexID		= NULL;
	m_pfUseIndexProps	= NULL;
} //CAlterIndexDlg::CAlterIndexDlg


////////////////////////////////////////////////////////////////
// CAlterIndexDlg::~CAlterIndexDlg
//
/////////////////////////////////////////////////////////////////
CAlterIndexDlg::~CAlterIndexDlg()
{
} //CAlterIndexDlg::~CAlterIndexDlg


////////////////////////////////////////////////////////////////
// CAlterIndexDlg::AlterIndex
//
/////////////////////////////////////////////////////////////////
HRESULT CAlterIndexDlg::AlterIndex(
	HWND				hWndParent,
	DBID				**ppTableID,
	DBID				**ppIndexID,
	DBID				**ppNewIndexID,
	BOOL				*pfUseIndexProps
)
{
	ASSERT(ppTableID);
	ASSERT(ppIndexID);
	ASSERT(ppNewIndexID);
	ASSERT(pfUseIndexProps);

	m_ppTableID			= ppTableID;
	m_ppIndexID			= ppIndexID;
	m_ppNewIndexID		= ppNewIndexID;
	m_pfUseIndexProps	= pfUseIndexProps;

	*m_ppTableID = NULL;
	*m_ppIndexID = NULL;
	*m_ppNewIndexID = NULL;

	*pfUseIndexProps = FALSE;

	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;	
} //CAlterIndexDlg::AlterIndex


/////////////////////////////////////////////////////////////////////
// CAlterIndexDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////
BOOL CAlterIndexDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Setup Controls
	m_editTableName.CreateIndirect(m_hWnd, IDE_TABLEID);
	m_editIndexName.CreateIndirect(m_hWnd, IDE_INDEXID);
	m_editNewIndexName.CreateIndirect(m_hWnd, IDE_NewIndexID);

	//Auto Save buttons for change IndexID and use index properties
	CheckDlgButton(IDB_USEPROPERTIES, BST_UNCHECKED);
	CheckDlgButton(IDC_ChangeIndexID, (*m_pfUseIndexProps)? BST_CHECKED: BST_UNCHECKED);

	m_editNewIndexName.EnableWindow(FALSE);

	//Delegate
	return CDialogLite::OnInitDialog();
} // CAlterIndexDlg::OnInitDialog




/////////////////////////////////////////////////////////////////////////////
// CAlterIndexDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterIndexDlg::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch (iID)
	{
		case IDB_INDEXPROPERTIES:
		{
			// get the new index properties
			//Get the "this" pointer
//			CAlterIndexDlg	*pThis			= (CAlterIndexDlg*)GetThis(hWndCtrl);
			CMDIChild		*pCMDIChild		= this->m_pCMDIChild;
			CMainWindow		*pCMainWindow	= pCMDIChild->m_pCMainWindow;

			CSession		*pCSession		= (CSession*)pCMDIChild->GetObject(eCSession);
			CDataSource		*pCDataSource	= SOURCE_GETPARENT(pCSession, CDataSource);
			CPropertiesDlg	sCPropertiesDlg(pCMainWindow);

			//SetProperties
			sCPropertiesDlg.SetProperties(hWndCtrl, &DBPROPSET_INDEXALL, IID_IDBProperties, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &this->m_CPropSets);
			return TRUE;
		}
		case IDC_ChangeIndexID:
		{
			m_editNewIndexName.EnableWindow(IsDlgButtonChecked(IDC_ChangeIndexID));
			return TRUE;
		}
	}

	return FALSE;
} //CAlterIndexDlg::OnCommand


/////////////////////////////////////////////////////////////////////////////
// CAlterIndexDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterIndexDlg::OnOK()
{
	CWaitCursor waitCursor;
	WCHAR		*pwszTableName = NULL;
	WCHAR		*pwszIndexName = NULL;
	WCHAR		*pwszNewIndexName = NULL;
	HRESULT		hr;

	pwszTableName = m_editTableName.GetWindowText();
	if (pwszTableName)
	{
		SAFE_ALLOC(*m_ppTableID, DBID, 1);
		(*m_ppTableID)->eKind			= DBKIND_NAME;
		(*m_ppTableID)->uName.pwszName	= pwszTableName;
	}

	pwszIndexName = m_editIndexName.GetWindowText();
	if (pwszIndexName)
	{
		SAFE_ALLOC(*m_ppIndexID, DBID, 1);
		(*m_ppIndexID)->eKind			= DBKIND_NAME;
		(*m_ppIndexID)->uName.pwszName	= pwszIndexName;
	}

	pwszNewIndexName = m_editNewIndexName.GetWindowText();
	if (pwszNewIndexName)
	{
		SAFE_ALLOC(*m_ppNewIndexID, DBID, 1);
		(*m_ppNewIndexID)->eKind			= DBKIND_NAME;
		(*m_ppNewIndexID)->uName.pwszName	= pwszNewIndexName;
	}

	*m_pfUseIndexProps = IsDlgButtonChecked(IDB_USEPROPERTIES);

CLEANUP:
	//Delegate
	return CDialogLite::OnOK();
} //CAlterIndexDlg::OnOK


////////////////////////////////////////////////////////////////
// CAlterTableDlg::CAlterTableDlg
//
/////////////////////////////////////////////////////////////////
CAlterTableDlg::CAlterTableDlg(CMDIChild *pCMDIChild)
	: CDialogLite(IDD_ALTERTABLE), CRowsetViewerDlg(NULL, pCMDIChild)
{
	m_pTableID			= NULL;
	m_pNewTableID		= NULL;
	m_fUseTableProps	= FALSE;
} //CAlterTableDlg::CAlterTableDlg



////////////////////////////////////////////////////////////////
// CAlterTableDlg::CAlterTableDlg
//
/////////////////////////////////////////////////////////////////
HRESULT CAlterTableDlg::AlterTable(HWND hWndParent)
{
	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;	
} //CAlterTableDlg::AlterTable


////////////////////////////////////////////////////////////////
// CAlterTableDlg::~CAlterTableDlg
//
/////////////////////////////////////////////////////////////////
CAlterTableDlg::~CAlterTableDlg()
{
	DBIDFree(m_pTableID);
	SAFE_FREE(m_pTableID);
	DBIDFree(m_pNewTableID);
	SAFE_FREE(m_pNewTableID);
} //CAlterTableDlg::~CAlterTableDlg


/////////////////////////////////////////////////////////////////////
// CAlterTableDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////
BOOL CAlterTableDlg::OnInitDialog()
{
	CWaitCursor waitCursor;

	//Setup Controls
	m_editTableName.CreateIndirect(m_hWnd, IDE_TABLEID);
	m_editNewTableName.CreateIndirect(m_hWnd, IDE_NewTableID);

	//Auto Save buttons for change IndexID and use index properties
	CheckDlgButton(IDB_USEPROPERTIES, BST_UNCHECKED);

	//Delegate
	return CDialogLite::OnInitDialog();
} // CAlterTableDlg::OnInitDialog




/////////////////////////////////////////////////////////////////////////////
// CAlterTableDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterTableDlg::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch (iID)
	{
		case IDB_TABLEPROPERTIES:
		{
			// get the new index properties
			//Get the "this" pointer
			CMDIChild		*pCMDIChild		= m_pCMDIChild;
			CMainWindow		*pCMainWindow	= pCMDIChild->m_pCMainWindow;

			CSession		*pCSession		= (CSession*)pCMDIChild->GetObject(eCSession);
			CDataSource		*pCDataSource	= SOURCE_GETPARENT(pCSession, CDataSource);
			CPropertiesDlg	sCPropertiesDlg(pCMainWindow);

			//SetProperties
			sCPropertiesDlg.SetProperties(hWndCtrl, &DBPROPSET_TABLEALL, IID_IDBProperties, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &m_CPropSets);
			return TRUE;
		}
	}

	return FALSE;
} //CAlterTableDlg::OnCommand


/////////////////////////////////////////////////////////////////////////////
// CAlterTableDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterTableDlg::OnOK()
{
	CWaitCursor waitCursor;
	WCHAR		*pwszTableName = NULL;
	WCHAR		*pwszNewTableName = NULL;
	HRESULT		hr;

	pwszTableName = m_editTableName.GetWindowText();
	if (pwszTableName)
	{
		SAFE_ALLOC(m_pTableID, DBID, 1);
		m_pTableID->eKind			= DBKIND_NAME;
		m_pTableID->uName.pwszName	= pwszTableName;
	}

	pwszNewTableName = m_editNewTableName.GetWindowText();
	if (pwszNewTableName)
	{
		SAFE_ALLOC(m_pNewTableID, DBID, 1);
		m_pNewTableID->eKind			= DBKIND_NAME;
		m_pNewTableID->uName.pwszName	= pwszNewTableName;
	}

	m_fUseTableProps = IsDlgButtonChecked(IDB_USEPROPERTIES);

CLEANUP:
	//Delegate
	return CDialogLite::OnOK();
} //CAlterTableDlg::OnOK




////////////////////////////////////////////////////////////////
// CAlterColumnDlg::CAlterColumnDlg
//
/////////////////////////////////////////////////////////////////
CAlterColumnDlg::CAlterColumnDlg(CMDIChild *pCMDIChild)
	: CDialogLite(IDD_ALTERCOLUMN), CRowsetViewerDlg(NULL, pCMDIChild)
{
	m_pTableID			= NULL;
	m_pColumnID			= NULL;
	m_fUseColProps		= FALSE;
	m_dwColFlags		= 0;
	memset(&m_ColDesc, 0, sizeof(DBCOLUMNDESC));
} //CAlterColumnDlg::CAlterColumnDlg



////////////////////////////////////////////////////////////////
// CAlterColumnDlg::AlterColumn
//
/////////////////////////////////////////////////////////////////
HRESULT CAlterColumnDlg::AlterColumn(HWND hWndParent)
{
	//Now Display the dialog
	if(DoModal(hWndParent) == IDOK)
		return S_OK;

	return DB_E_CANCELED;	
} //CAlterColumnDlg::AlterColumn


////////////////////////////////////////////////////////////////
// CAlterColumnDlg::~CAlterColumnDlg
//
/////////////////////////////////////////////////////////////////
CAlterColumnDlg::~CAlterColumnDlg()
{
	DBIDFree(m_pTableID);
	SAFE_FREE(m_pTableID);
	DBIDFree(m_pColumnID);
	SAFE_FREE(m_pColumnID);
	
	DBIDFree(&m_ColDesc.dbcid);
	SAFE_FREE(m_ColDesc.pwszTypeName);
} //CAlterColumnDlg::~CAlterColumnDlg


/////////////////////////////////////////////////////////////////////
// CAlterColumnDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////////
BOOL CAlterColumnDlg::OnInitDialog()
{
	CWaitCursor waitCursor;
	INDEX		iSel;
	DBORDINAL	index;
	CMDIChild	*pCMDIChild		= m_pCMDIChild;
	WCHAR		wszBuffer[MAX_NAME_LEN+1];

	CSession		*pCSession		= (CSession*)pCMDIChild->GetObject(eCSession);

	// make sure we have provider type info in the session
	if (0 == pCSession->m_cProvTypes)
		pCSession->GetProviderTypes();

	//Setup Controls
	m_editTableName.CreateIndirect(m_hWnd, IDE_TABLEID);
	m_editColumnName.CreateIndirect(m_hWnd, IDE_COLUMN);
	m_editNewColumnName.CreateIndirect(m_hWnd, IDE_COLUMNID);

	m_editSize.CreateIndirect(m_hWnd, IDE_SIZE);
	m_editPrecision.CreateIndirect(m_hWnd, IDE_PRECISION);
	m_editScale.CreateIndirect(m_hWnd, IDE_SCALE);

	m_listColumnDescFlags.CreateIndirect(m_hWnd, IDL_CDFLAGS);

	m_comboDBType.CreateIndirect(m_hWnd, IDC_TYPE);
	m_comboTypeName.CreateIndirect(m_hWnd, IDC_TYPENAME);

	//Auto Save buttons for change IndexID and use index properties
	CheckDlgButton(IDB_USEPROPERTIES, BST_UNCHECKED);

	//Fill in the column desc flag list
	for (index = 0; index < g_cCDFlagsMaps; index++)
	{
		iSel = m_listColumnDescFlags.AddString(g_rgCDFlagsMaps[index].pwszName);
		m_listColumnDescFlags.SetItemParam(iSel, g_rgCDFlagsMaps[index].lItem);
	}

	//Fill in the type name list
	for (index = 0; index < pCSession->m_cProvTypes; index++)
	{
		iSel = m_comboTypeName.AddString(pCSession->m_rgProvTypes[index].wszTypeName);
		m_comboTypeName.SetItemParam(iSel, (LPARAM)(&pCSession->m_rgProvTypes[index]));
	}
	m_comboTypeName.SetCurSel(0);

	//Fill in the DBTYPE list
	for (index = 0; index < g_cDBTypes; index++)
	{
		iSel = m_comboDBType.AddString(g_rgDBTypes[index].pwszName);
		m_comboDBType.SetItemParam(iSel, g_rgDBTypes[index].lItem);
	}

	// select the corresponding DBTYPE
	wSendMessage(m_comboDBType.GetWnd(), CB_SELECTSTRING, -1, GetDBTypeName(pCSession->m_rgProvTypes[0].wType));
				
	StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", pCSession->m_rgProvTypes[0].ulColumnSize);
	m_editSize.SetWindowText(wszBuffer); 
	
	if (IsNumericType(pCSession->m_rgProvTypes[0].wType))
	{
		StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", pCSession->m_rgProvTypes[0].ulColumnSize);
		m_editPrecision.SetWindowText(wszBuffer); 
	}
	else
	{
		m_editPrecision.SetWindowText(L"0"); 
	}
	
	m_editScale.SetWindowText(L"0");

	//Delegate
	return CDialogLite::OnInitDialog();
} // CAlterColumnDlg::OnInitDialog




/////////////////////////////////////////////////////////////////////////////
// CAlterColumnDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterColumnDlg::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch (iID)
	{
		case IDB_SETPROPERTIES:
		{
			// get the new index properties
			//Get the "this" pointer
			CSession		*pCSession		= (CSession*)m_pCMDIChild->GetObject(eCSession);
			CDataSource		*pCDataSource	= SOURCE_GETPARENT(pCSession, CDataSource);
			CPropertiesDlg	sCPropertiesDlg(m_pCMainWindow);

			//SetProperties
			sCPropertiesDlg.SetProperties(hWndCtrl, &DBPROPSET_COLUMNALL, IID_IDBProperties, NULL, pCDataSource ? pCDataSource->m_pIDBProperties : NULL, &m_CPropSets);
			return TRUE;
		}
	}

	return FALSE;
} //CAlterColumnDlg::OnCommand


/////////////////////////////////////////////////////////////////////
// CAlterColumnDlg::OnCommandNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CAlterColumnDlg::OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl)
{
	switch(wNotifyCode)
	{
		case CBN_SELCHANGE:
		{	
			if(OnSelChange(iID, hWndCtrl))
				return TRUE;
			break;
		}
	};

	//Otherwise Delegate
	return CDialogLite::OnCommandNotify(wNotifyCode, iID, hWndCtrl);
} // CAlterColumnDlg::OnCommandNotify


/////////////////////////////////////////////////////////////////////////////
// CAlterColumnDlg::OnSelChange
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterColumnDlg::OnSelChange(INT iID, HWND hWndCtrl) 
{
	static WCHAR			wszBuffer[MAX_NAME_LEN+1];

	switch(iID)
	{
		case IDC_TYPENAME:
		{
			CMDIChild	*pCMDIChild		= m_pCMDIChild;
			CSession	*pCSession		= (CSession*)pCMDIChild->GetObject(eCSession);

			PROVTYPEINFO	*pTypeInfo	= NULL;
			pTypeInfo = (PROVTYPEINFO*)m_comboTypeName.GetItemParam(m_comboTypeName.GetCurSel());
			
			if (pTypeInfo)
			{
				// select the corresponding DBTYPE
				wSendMessage(m_comboDBType.GetWnd(), CB_SELECTSTRING, -1, GetDBTypeName(pTypeInfo->wType));
				
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", pTypeInfo->ulColumnSize);
				m_editSize.SetWindowText(wszBuffer); 
				
				if (IsNumericType(pTypeInfo->wType))
				{
					StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", pTypeInfo->ulColumnSize);
					m_editPrecision.SetWindowText(wszBuffer); 
				}
				else
				{
					m_editPrecision.SetWindowText(L"0"); 
				}
			}														

			return TRUE;
		}
		
		case IDC_TYPE:
		{
			CMDIChild	*pCMDIChild		= m_pCMDIChild;
			CSession	*pCSession		= (CSession*)pCMDIChild->GetObject(eCSession);
			DBTYPE		wType;
			DBLENGTH ulMaxSize = 0;
			BYTE  bPrecision, bScale;

			wType = (DBTYPE)m_comboDBType.GetItemParam(m_comboDBType.GetCurSel());

										 
			//Get Default Size,Prec,Scale for this type...
			GetDBTypeMaxSize(wType, &ulMaxSize, &bPrecision, &bScale);

			//Set Size
			wSendMessageFmt(m_editSize.GetWnd(), WM_SETTEXT, 0, L"%lu", ulMaxSize!=0 ? ulMaxSize : 255);

			//Set Precision
			wSendMessageFmt(m_editPrecision.GetWnd(), WM_SETTEXT, 0, L"%d", bPrecision);
			
			//Set Scale
			wSendMessageFmt(m_editScale.GetWnd(), WM_SETTEXT, 0, L"%d", bScale);

			// no type name
			m_comboTypeName.SetWindowText(L"");				
			m_comboTypeName.SetCurSel(-1);

			return TRUE;
		}
	};

	return FALSE;
} //CAlterColumnDlg::OnSelChange
 


/////////////////////////////////////////////////////////////////////////////
// CAlterColumnDlg::OnOK
//
/////////////////////////////////////////////////////////////////////////////
BOOL CAlterColumnDlg::OnOK()
{
	CWaitCursor waitCursor;
	WCHAR		*pwszTableName	= NULL;
	WCHAR		*pwszColumnName	= NULL;
	HRESULT		hr;
	const ULONG	cMaxSel = 2;
	INDEX		cSel;
	INT			rgSel[cMaxSel];
	HWND		hWndCDFlags		= GetDlgItem(IDL_CDFLAGS);
	LONG		lValue;
	DBORDINAL	index;
	
	CMDIChild	*pCMDIChild		= m_pCMDIChild;
	CSession	*pCSession		= (CSession*)pCMDIChild->GetObject(eCSession);

	PROVTYPEINFO	*pTypeInfo	= NULL;

	pwszTableName = m_editTableName.GetWindowText();
	if (pwszTableName)
	{
		SAFE_ALLOC(m_pTableID, DBID, 1);
		m_pTableID->eKind			= DBKIND_NAME;
		m_pTableID->uName.pwszName	= pwszTableName;
	}

	pwszColumnName = m_editColumnName.GetWindowText();
	if (pwszColumnName)
	{
		SAFE_ALLOC(m_pColumnID, DBID, 1);
		m_pColumnID->eKind			= DBKIND_NAME;
		m_pColumnID->uName.pwszName	= pwszColumnName;
	}

	// fill in the flags
	cSel = SendMessage(hWndCDFlags, LB_GETSELITEMS, (WPARAM)cMaxSel, (LPARAM)rgSel);

	m_dwColFlags = 0;
	for (index = 0; index < (ULONG)cSel; index++)
	{
		m_dwColFlags |= this->m_listColumnDescFlags.GetItemParam(rgSel[index]);
	}

	// fill in the DBCOLUMNDESC structure
	memset(&m_ColDesc, 0, sizeof(DBCOLUMNDESC));

	// set column name
	pwszColumnName = m_editNewColumnName.GetWindowText();
	if (pwszColumnName)
	{
		m_ColDesc.dbcid.eKind			= DBKIND_NAME;
		m_ColDesc.dbcid.uName.pwszName	= pwszColumnName;
	}

	// get type name 
	if (LB_ERR != m_comboTypeName.GetCurSel())
	{
		pTypeInfo = (PROVTYPEINFO*)m_comboTypeName.GetItemParam(m_comboTypeName.GetCurSel());
		m_ColDesc.pwszTypeName = wcsDuplicate(pTypeInfo->wszTypeName);
	}
	else
	{
		// it might be another value
		m_ColDesc.pwszTypeName = m_comboTypeName.GetWindowText();
	}

	// get wType
	m_ColDesc.wType = (DBTYPE)m_comboDBType.GetItemParam(m_comboDBType.GetCurSel());

	// get column size
	lValue = 0;
	GetEditBoxValue(m_editSize.GetWnd(), &lValue, 0/*Min*/, LONG_MAX/*Max*/, TRUE);
	m_ColDesc.ulColumnSize = lValue;

	// get precision
	GetEditBoxValue(m_editPrecision.GetWnd(), &lValue, 0/*Min*/, 255/*Max*/, TRUE);
	m_ColDesc.bPrecision = (BYTE)lValue;

	// get size
	GetEditBoxValue(m_editScale.GetWnd(), &lValue, 0/*Min*/, 255/*Max*/, TRUE);
	m_ColDesc.bScale = (BYTE)lValue;

	// get properties
	m_fUseColProps = IsDlgButtonChecked(IDB_USEPROPERTIES);
	if (m_fUseColProps)
	{
		m_ColDesc.cPropertySets = m_CPropSets.GetCount();
		m_ColDesc.rgPropertySets= m_CPropSets.GetPropSets();
	}

CLEANUP:
	//Delegate
	return CDialogLite::OnOK();
} //CAlterColumnDlg::OnOK



////////////////////////////////////////////////////////////////
// CExecuteParamDlg
//
/////////////////////////////////////////////////////////////////
CExecuteParamDlg::CExecuteParamDlg(CMDIChild* pCMDIChild)
	: CDialogLite(IDD_PARAMEXECUTE), CRowsetViewerDlg(NULL, pCMDIChild)
{
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnInitDialog
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnInitDialog()
{
	CWaitCursor waitCursor;
	WCHAR		wszBuffer[MAX_COL_SIZE];

	//Command
	CCommand* pCCommand = SOURCE_GETOBJECT(m_pCMDIChild->m_pCSource, CCommand);
	CParameters& rParameters	= pCCommand->m_Parameters;
	CBindings&	rBindings		= rParameters.GetBindings();
	DB_UPARAMS cParamSets		= max(rParameters.GetParams().cParamSets, 1);
	DWORD dwConvFlags			= GetOptions()->m_dwConvFlags;

	//Tab Control
	m_tabParamSets.CreateIndirect(m_hWnd, IDP_TABS);

	//Add all the Tabs
	ASSERT(m_vecParams.GetCount() == 0);
	ASSERT(m_vecValues.GetCount() == 0);
	for(ULONG iParamSet=0; iParamSet<cParamSets; iParamSet++)
	{
		//Add the tab
		OnAddSet(iParamSet, rBindings.GetCount()==0/*fUseDefaults*/);
		CListViewLite* plistParams = m_vecParams[iParamSet];
		CListViewLite* plistValues = m_vecValues[iParamSet];

		//Now add the previously setup data (if any)
		DBLENGTH	dbLength = 0;
		DBSTATUS	dbStatus = DBSTATUS_S_OK;

		//Display the data for this paramset...
		for(ULONG i=0; i<rBindings.GetCount(); i++)
		{
			const DBBINDING* pBinding = &rBindings[i];

			//Value
			//Get the Length, Status, Data for this Column
			pCCommand->GetColumnData(pBinding, rParameters.GetData(iParamSet), &dbStatus, &dbLength, NULL, wszBuffer, MAX_COL_SIZE, dwConvFlags, pBinding->wType);

			//Insert the Data into the list (make an "easy" edit)
			plistValues->InsertItem(i, 0, wszBuffer, (LPARAM)pBinding->wType, 0);

			//Insert Parameter Name
			StringFormat(wszBuffer, NUMELE(wszBuffer), L"Parameter %lu", i+1);
			plistParams->InsertItem(i, 0, wszBuffer);

			//Insert the Length into the List
			if(LENGTH_IS_BOUND(*pBinding))
				StringFormat(wszBuffer, NUMELE(wszBuffer), L"%lu", dbLength);
			else
				StringCopy(wszBuffer, L"Not Bound", MAX_COL_SIZE);
			plistParams->InsertItem(i, 1, wszBuffer);

			//Insert the Status into the List
			plistParams->InsertItem(i, 2, STATUS_IS_BOUND(*pBinding) ? GetStatusName(dbStatus) : L"Not Bound");

			//Set Item State to Checked/Unchecked
			plistParams->SetItemState(i, 0,  INDEXTOSTATEIMAGEMASK(STATE_CHECKED),  LVIS_STATEIMAGEMASK);
		}

		//AutoSize column (now that the data is inserted)
		plistParams->SetColumnWidth(0,	LVSCW_AUTOSIZE_USEHEADER);
		plistParams->SetColumnWidth(1,	LVSCW_AUTOSIZE_USEHEADER);
		plistParams->SetColumnWidth(2,	LVSCW_AUTOSIZE_USEHEADER);
		plistValues->SetColumnWidth(0,	LVSCW_AUTOSIZE_USEHEADER);
	}
	
	//Now activate the default Parameter Set
	m_tabParamSets.SetCurSel(0, TRUE/*fSendNotification*/);
	CenterDialog(m_hWnd);
	return CDialogLite::OnInitDialog();
}

				
////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnOK
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnOK()
{
	CWaitCursor waitCursor;

	//Get the "this" pointer
	CCommand* pCCommand = SOURCE_GETOBJECT(m_pCMDIChild->m_pCSource, CCommand);
	CParameters& rParameters = pCCommand->m_Parameters;
	HRESULT hr = S_OK;

	//Setup ParamInfo
	DBPARAMINFO* rgParamInfo = NULL;
	DB_UPARAMS cParamsProv = 0;
	DBPARAMINFO* rgParamInfoProv = NULL;
	WCHAR* pwszNamesBuffer = NULL;
	WCHAR* pwszCommandText = NULL;

	ULONG iParam,iParamSet = 0;
	DB_UPARAMS cParams = 0;
	DB_UPARAMS cParamSets = m_tabParamSets.GetItemCount();

	//SetCommandText
	BOOL fSetCommandText = ::IsDlgButtonChecked(m_hWndParent, IDB_SETCOMMANDTEXT);
	BOOL fSetCommandStream = ::IsDlgButtonChecked(m_hWndParent, IDB_SETCOMMANDSTREAM);

	//Obtain the number of parameters (Items) in the list
	//NOTE: Since all the sets have the same number of parameters, and we are
	//guareenteed at least one set, we only need to check the first set...
	cParams = m_vecParams[0]->GetItemCount();
	cParams = cParams == LVM_ERR ? 0 : cParams;

	//Default ParameterInfo
	//NOTE: Some providers cannot derive parameter information
	//or the user may have only setup param info for some parameters, so by default
	//create parameter info for all params bound as INPUT and WSTR (or binding type).
	SAFE_ALLOC(rgParamInfo, DBPARAMINFO, cParams);
	for(iParam=0; iParam<cParams; iParam++)
	{
		//Setup the ParamInfo
		rgParamInfo[iParam].iOrdinal	= iParam+1;
		rgParamInfo[iParam].dwFlags		= DBPARAMFLAGS_ISINPUT;
		rgParamInfo[iParam].pwszName	= NULL;
		rgParamInfo[iParam].ulParamSize	= MAX_COL_SIZE;
		rgParamInfo[iParam].pTypeInfo	= NULL;
		rgParamInfo[iParam].wType		= GetOptions()->GetBindingType(DBTYPE_WSTR);
		rgParamInfo[iParam].bPrecision	= 0;
		rgParamInfo[iParam].bScale		= 0;
	}

	//Did the user indicate they wanted to set the command text...
	if(fSetCommandText || fSetCommandStream)
	{
		//SetCommandText
		pwszCommandText = m_pCMDIChild->m_pCQueryBox->GetSelectedText();
		if(fSetCommandText)
			hr = pCCommand->SetCommandText(pwszCommandText);
		else
			hr = pCCommand->SetCommandStream(pwszCommandText);
						
		//Prepare - we have to prepare for GetParameterInfo
		hr = pCCommand->Prepare(0);
	}
	
	//Try to obtain more info about the params (native type for bindings)
	if(SUCCEEDED(hr = pCCommand->GetParameterInfo(&cParamsProv, &rgParamInfoProv, &pwszNamesBuffer)))
	{
		//Loop over the info returned from the provider
		for(ULONG iParam=0; iParam<cParamsProv; iParam++)
		{
			DBPARAMINFO* pParamInfo = &rgParamInfoProv[iParam];

			//Update our default ParamInfo...
			if(pParamInfo->iOrdinal && pParamInfo->iOrdinal <= cParams)
			{
				//NOTE: Some providers seem not to be able to determine
				//if the parameter is input or output, and do not return any flags.
				//This will fail accessor creation, unless some flag is set, so turn
				//on INPUT if none of the directional flags are on...
				if(!BIT_SET(pParamInfo->dwFlags, DBPARAMFLAGS_ISINPUT) && !BIT_SET(pParamInfo->dwFlags, DBPARAMFLAGS_ISOUTPUT))
					pParamInfo->dwFlags	|= DBPARAMFLAGS_ISINPUT;

				ASSERT(rgParamInfo[pParamInfo->iOrdinal-1].iOrdinal == pParamInfo->iOrdinal);
				memcpy(&rgParamInfo[pParamInfo->iOrdinal-1], pParamInfo, sizeof(DBPARAMINFO));
			}
		}
	}					

	//Create Parameter Accessor
	TESTC(hr = pCCommand->CreateParamAccessor(cParams, rgParamInfo, cParamSets));
	
	//Setup Data Buffer
	//Loop through all the parameter sets
	for(iParamSet=0; iParamSet<cParamSets; iParamSet++)
		TESTC(hr = m_pCMDIChild->GetListViewValues(m_vecParams[iParamSet]->m_hWnd, m_vecValues[iParamSet]->m_hWnd, pCCommand, rParameters.GetBindings(), rParameters.GetData(iParamSet)));

CLEANUP:
	SAFE_FREE(rgParamInfo);
	SAFE_FREE(rgParamInfoProv);
	SAFE_FREE(pwszNamesBuffer);
	SAFE_FREE(pwszCommandText);

	//Don't exit if we can't successfully create a parameter from the data entered...
	if(FAILED(hr))
		return FALSE;

	//Remove all the sets...
	//NOTE: Since were actually removing the tab, the indexes compact, so 
	//we only need to keep removing the first tab...
	for(iParamSet=0; iParamSet<cParamSets; iParamSet++)
		OnDelSet(0);
	return CDialogLite::OnOK();
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnCancel
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnCancel()
{
	//Remove all the sets...
	//NOTE: Since were actually removing the tab, the indexes compact, so 
	//we only need to keep removing the first tab...
	INDEX cParamSets = m_tabParamSets.GetItemCount();
	for(INDEX i=0; i<cParamSets; i++)
		OnDelSet(0);

	//Delegate
	return CDialogLite::OnCancel();
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnAddParameter
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnAddParameter()
{
	WCHAR wszBuffer[MAX_NAME_LEN];

	//Loop over all paramsets...
	for(INDEX iParamSet=0; iParamSet<m_tabParamSets.GetItemCount(); iParamSet++)
	{
		//Make our lives easier
		CListViewLite* plistParams = m_vecParams[iParamSet];
		CListViewLite* plistValues = m_vecValues[iParamSet];
				
		//Get the Total Number of ListView Items
		INDEX cItems = plistParams->GetItemCount();
		if(cItems != LVM_ERR)
		{
			//Insert a new Parameter
			StringFormat(wszBuffer, NUMELE(wszBuffer), L"Parameter %lu", cItems+1);
			plistParams->InsertItem(cItems, 0,	wszBuffer);
			plistParams->InsertItem(cItems, 1,	L"0");
			plistParams->InsertItem(cItems, 2,	L"DBSTATUS_S_OK");
			plistValues->InsertItem(cItems, 0,	L"Enter Parameter Value...");

			//Set Item State to Checked/Unchecked
			plistParams->SetItemState(cItems, 0,  INDEXTOSTATEIMAGEMASK(STATE_CHECKED),  LVIS_STATEIMAGEMASK);
		}
	}

	return TRUE;
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnDelParameter
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnDelParameter()
{
	//Loop over all paramsets...
	for(INDEX iParamSet=0; iParamSet<m_tabParamSets.GetItemCount(); iParamSet++)
	{
		//Make our lives easier
		CListViewLite* plistParams = m_vecParams[iParamSet];
		CListViewLite* plistValues = m_vecValues[iParamSet];

		//Get the Total Number of ListView Items
		INDEX cItems = plistParams->GetItemCount();
		if(cItems != LVM_ERR)
		{
			//Delete the Last Item
			plistParams->DeleteItem(cItems-1);
			plistValues->DeleteItem(cItems-1);
		}
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnAddSet
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnAddSet(INDEX iParamSet, BOOL fUseDefaults)
{
	//Add the new Tab
	WCHAR wszBuffer[MAX_NAME_LEN];
	StringFormat(wszBuffer, NUMELE(wszBuffer), L"ParamSet %lu", iParamSet+1);
	m_tabParamSets.InsertItem(iParamSet, wszBuffer);

	//Obtain the bounding rectangle
	//NOTE: We have to do this AFTER we insert the tab so that it includes 
	//the vertical size of the tab, otherwise we will end up using the space
	//needed by the later inserted tab...
	RECT rect;
	GetClientRect(m_tabParamSets.m_hWnd, &rect);
	m_tabParamSets.AdjustRect(&rect);
	int	 cxInside		= rect.right - rect.left;
	int	 cyInside		= rect.bottom - rect.top;
	
	//Create the Params ListView (dynamically...)
	CListViewLite* plistParams = new CListViewLite;
	plistParams->Create(m_tabParamSets.m_hWnd, WC_LISTVIEWW, NULL, IDL_PARAMNAMES, 
								WS_TABSTOP | WS_CHILD | /*WS_VISIBLE |*/ WS_BORDER | WS_VSCROLL | /*LVS_SINGLESEL |*/ LVS_AUTOARRANGE | LVS_REPORT | LVS_EDITLABELS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE,
								rect.left, rect.top, cxInside/2, cyInside);
	m_vecParams.AddElement(plistParams);
	
	//Create the Values ListView (dynamically...)
	CListViewLite* plistValues = new CListViewLite;
	plistValues->Create(m_tabParamSets.m_hWnd, WC_LISTVIEWW, NULL, IDL_PARAMVALUES, 
								WS_TABSTOP | WS_CHILD | /*WS_VISIBLE |*/ WS_BORDER | WS_VSCROLL | /*LVS_SINGLESEL |*/ LVS_AUTOARRANGE | LVS_REPORT | LVS_EDITLABELS | LVS_SHOWSELALWAYS, WS_EX_CLIENTEDGE,
								rect.left + cxInside/2, rect.top, cxInside - cxInside/2, cyInside);
	m_vecValues.AddElement(plistValues);
	
	//Use Extended ListView Styles!
	SendMessage(plistParams->m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_CHECKBOXES);
	SendMessage(plistValues->m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE);

	//Set image list to the Window 
	ListView_SetImageList(plistParams->m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_STATE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_STATE);
	ListView_SetImageList(plistValues->m_hWnd, ImageList_LoadImage(GetAppLite()->m_hInstance, MAKEINTRESOURCE(IDB_IMAGE), 16, 16, CLR_DEFAULT , IMAGE_BITMAP, LR_DEFAULTCOLOR), LVSIL_SMALL);

	//We need to the ListView
	//Headers/Columns contain ColInfo information 
	//Rows are per columns
	plistParams->InsertColumn(0,	L"Parameter");
	plistParams->InsertColumn(1,	L"Length");
	plistParams->InsertColumn(2,	L"Status");
	plistValues->InsertColumn(0,	L"Value");
	
	//This new set needs to have the same number of parameters as the first set...
	if(fUseDefaults)
	{
		INDEX cParams = max(m_vecParams[0]->GetItemCount(), 1);
		for(INDEX iParam=0; iParam<cParams; iParam++)
		{
			//Parameters
			StringFormat(wszBuffer, NUMELE(wszBuffer), L"Parameter %lu", iParam+1);
			plistParams->InsertItem(iParam, 0,	wszBuffer);
			plistParams->InsertItem(iParam, 1,	L"0");
			plistParams->InsertItem(iParam, 2,	L"DBSTATUS_S_OK");
			plistValues->InsertItem(iParam, 0,	L"Enter Parameter Value...");
		}
	}

	//AutoSize column
	plistParams->SetColumnWidth(0,	LVSCW_AUTOSIZE_USEHEADER);
	plistParams->SetColumnWidth(1,	LVSCW_AUTOSIZE_USEHEADER);
	plistParams->SetColumnWidth(2,	LVSCW_AUTOSIZE_USEHEADER);
	plistValues->SetColumnWidth(0,	LVSCW_AUTOSIZE_USEHEADER);

	//Now select this new tab
	m_tabParamSets.SetCurSel(iParamSet, TRUE/*fSendNotification*/);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnDelSet
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnDelSet(INDEX iParamSet)
{
	INDEX cTabs = m_tabParamSets.GetItemCount();
	INDEX iNextTab = iParamSet<(cTabs-1) ? iParamSet+1 : max(0, iParamSet-1);

	//Display the next tab (before deleting the previous selection)
	//ie: SelChanging needs to inform its going away...
	m_tabParamSets.SetCurSel(iNextTab, TRUE/*fSendNotification*/);

	//Delete the Tab
	m_tabParamSets.DeleteItem(iParamSet);

	//Now delete the dynamic listviews
	CListViewLite* plistParams = m_vecParams[iParamSet];
	plistParams->OnDestroy();
	SAFE_DELETE(plistParams);

	//Now delete the dynamic listviews
	CListViewLite* plistValues = m_vecValues[iParamSet];
	plistValues->OnDestroy();
	SAFE_DELETE(plistValues);

	//Now remove the listview from the vectors
	m_vecParams.RemoveAt(iParamSet);
	m_vecValues.RemoveAt(iParamSet);

	//Refresh the tabs...
	m_tabParamSets.SetCurSel(m_tabParamSets.GetCurSel(), TRUE/*fSendNotification*/);
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnStatusChange
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnStatusChange(DBSTATUS dbStatus)
{
	CListViewLite* plistParams = GetActiveParam();
	if(plistParams)
	{
		//Insert the Status into the List
		INDEX iSelRow = plistParams->GetNextItem(-1, LVNI_SELECTED);
		if(iSelRow == LVM_ERR)
			return FALSE;

		plistParams->InsertItem(iSelRow, 2, GetStatusName(dbStatus));
	}
	return TRUE;
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::GetActiveParam
//
/////////////////////////////////////////////////////////////////
CListViewLite*	CExecuteParamDlg::GetActiveParam()
{
	//Obtain the active tab
	INDEX iParamSet = m_tabParamSets.GetCurSel();
	if(iParamSet == LVM_ERR)
		return NULL;

	//Return the active listview
	return m_vecParams[iParamSet];
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::GetActiveValue
//
/////////////////////////////////////////////////////////////////
CListViewLite*	CExecuteParamDlg::GetActiveValue()
{
	//Obtain the active tab
	INDEX iParamSet = m_tabParamSets.GetCurSel();
	if(iParamSet == LVM_ERR)
		return NULL;

	//Return the active listview
	return m_vecValues[iParamSet];
}


////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnContextMenu
//
/////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnContextMenu(HWND hWnd, REFPOINTS pts)
{
	//Obtain the active ListView
	CListViewLite* plistParams = GetActiveParam();
	if(plistParams)
	{
		//Must have selected a row to change status
		INDEX iSel = plistParams->GetNextItem(-1, LVNI_SELECTED);
		if(iSel == LVM_ERR)
			return FALSE;

		//Make sure the Param listview is being selected.
		//NOTE: the hWnd of the control is the tab window, so we have to use
		//the hittest to correctly determine which child...
		//NOTE: ContextMenu is in Screen Coordinates
		if(plistParams->HitTest(pts, NULL, FALSE/*fClientCoords*/) != LVM_ERR)
		{
			//Display Menu
			DisplayContextMenu(
								m_hWnd,
								IDM_CHANGESTATUS, 
								pts,
								m_hWnd
								);
		}
	}
	return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnCommand
//
/////////////////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnCommand(UINT iID, HWND hWndCtrl) 
{
	switch(iID)
	{
		ON_COMMAND(IDB_ADD,					OnAddParameter())
		ON_COMMAND(IDB_DELETE,				OnDelParameter())
		
		//WThe user always adds at the end..
		ON_COMMAND(IDB_ADDSET,				OnAddSet(m_tabParamSets.GetItemCount()))
		
		//NOTE: We don't allow the user to delete the first (default) set...
		ON_COMMAND(IDB_DELETESET,			m_tabParamSets.GetItemCount()>1 ? OnDelSet(m_tabParamSets.GetItemCount()-1) : FALSE)

		ON_COMMAND(IDM_DBSTATUS_S_OK,		OnStatusChange(DBSTATUS_S_OK))
		ON_COMMAND(IDM_DBSTATUS_S_ISNULL,	OnStatusChange(DBSTATUS_S_ISNULL))
		ON_COMMAND(IDM_DBSTATUS_S_DEFAULT,	OnStatusChange(DBSTATUS_S_DEFAULT))
		ON_COMMAND(IDM_DBSTATUS_S_IGNORE,	OnStatusChange(DBSTATUS_S_IGNORE))
	};

	return FALSE;
}


/////////////////////////////////////////////////////////////////////
// CExecuteParamDlg::OnNotify
//
/////////////////////////////////////////////////////////////////////
BOOL CExecuteParamDlg::OnNotify(INT idCtrl, NMHDR* pNMHDR)
{
	switch(pNMHDR->code)
	{
		case TCN_SELCHANGE:
		{
			if(pNMHDR->idFrom == IDP_TABS)
			{
				//Obtain the active listview
				CListViewLite* plistParams = GetActiveParam();
				CListViewLite* plistValues = GetActiveValue();
				
				//Show the new active listviews
				if(plistParams && plistValues)
				{
					plistParams->ShowWindow(SW_SHOW);
					plistParams->UpdateWindow();
					plistValues->ShowWindow(SW_SHOW);
					plistValues->UpdateWindow();
				}
				return TRUE;
			}

			return FALSE;
		}

		case TCN_SELCHANGING:
		{
			if(pNMHDR->idFrom == IDP_TABS)
			{
				//Obtain the active listview
				CListViewLite* plistParams = GetActiveParam();
				CListViewLite* plistValues = GetActiveValue();

				//Hide the old listviews
				if(plistParams && plistValues)
				{
					plistParams->ShowWindow(SW_HIDE);
					plistValues->ShowWindow(SW_HIDE);
				}
				return TRUE;
			}

			return FALSE;
		}

		//Since we have "TwoClickActive" on this will get sent
		//Whenever a row is clicked on twice!
		//This functionality used to be done with NM_DBCLK
		case LVN_ITEMACTIVATE:
		{
			//Obtain the active listview
			CListViewLite* plistValues = GetActiveValue();
			if(plistValues)
			{
				//Obtain the SelectedRow
				INDEX iSel = plistValues->GetNextItem(-1, LVNI_SELECTED);
				if(iSel == LVM_ERR)
					return FALSE;

				//Now edit this label
				plistValues->EditLabel(iSel);
			}
			return TRUE;
		}

		case LVN_BEGINLABELEDIT:
			return FALSE;//allow the user to change the value of the item.
			
		case LVN_ENDLABELEDIT:
		{
			LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

			//Obtain the active listview
			CListViewLite* plistValues = GetActiveValue();
			if(plistValues)
			{
				//Now update the ListView with the new value
				if(pDispInfo->item.pszText)
				{

					WCHAR wszBuffer[MAX_NAME_LEN]={0};
					ConvertToWCHAR(pDispInfo->item.pszText, wszBuffer, MAX_NAME_LEN);
					plistValues->SetItemText(pDispInfo->item.iItem, 0, wszBuffer);
				}
			}
			return TRUE; //Allow the edited change
		}

		case LVN_ITEMCHANGED:
		{
			NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

			if(pNMListView->uNewState & LVNI_FOCUSED &&
				pNMListView->uNewState & LVNI_SELECTED)
			{
				//Obtain the active listview
				CListViewLite* plistParams = GetActiveParam();
				CListViewLite* plistValues = GetActiveValue();
				if(plistParams && plistValues)
				{
					if(idCtrl == IDL_PARAMVALUES)
					{
						SyncSibling(plistParams->m_hWnd, plistValues->m_hWnd);
	           			return TRUE;
					}

					if(idCtrl == IDL_PARAMNAMES)
					{
						SyncSibling(plistValues->m_hWnd, plistParams->m_hWnd);
						return TRUE;
					}
				}				
				return FALSE;
			}
		}
	}

	return FALSE;
}


