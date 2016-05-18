//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CDIALOG.H
//
//-----------------------------------------------------------------------------------

#ifndef _CDIALOG_H_
#define _CDIALOG_H_
										   


/////////////////////////////////////////////////////////////////////
// Forwards
//
/////////////////////////////////////////////////////////////////////
class CMainWindow;
class CEnumerator;
class CRowset;


/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "CDialogLite.h"
#include "Error.h"
#include "CEnum.h"
#include "List.h"
#include "CTrace.h"


/////////////////////////////////////////////////////////////////
// Defines 
//
/////////////////////////////////////////////////////////////////
#define MAX_RESTRICTIONS		 7
#define MAX_RECENTCONFIGS		10
#define MAX_RECENTFILES			10

typedef struct _SCHEMAINFO
{
	//Schema
	WCHAR*		pwszSchemaName;
	const GUID* pguidSchema;
	
	//Restrictions
	ULONG		cRestrictions;
	struct
	{
		ULONG   iNode;
		WCHAR*	pwszName;
		DBTYPE	wType;
	} rgResInfo[MAX_RESTRICTIONS];

} SCHEMAINFO;


/////////////////////////////////////////////////////////////////////
// Globals
//
/////////////////////////////////////////////////////////////////////
WCHAR*		GetSchemaName(REFGUID guidSchema);


/////////////////////////////////////////////////////////////////////
// CComboBoxGuid
//
/////////////////////////////////////////////////////////////////////
class CComboBoxGuid : public CComboBoxLite
{
public:
	//constructors
	CComboBoxGuid();
	virtual ~CComboBoxGuid();

	//Overloads
	virtual BOOL		Populate(ULONG cGuidMaps, const WIDEGUIDMAP* rgGuidMaps);

	//Messages

	//Helpers
	virtual INDEX		SetGuid(REFIID riid);
	virtual REFIID		GetGuid();

//protected:
	//data
	ULONG				m_cGuidMaps;
	const WIDEGUIDMAP*	m_rgGuidMaps;
};



/////////////////////////////////////////////////////////////////////
// CComboBoxString
//
/////////////////////////////////////////////////////////////////////
class CComboBoxString : public CComboBoxLite
{
public:
	//constructors
	CComboBoxString();
	virtual ~CComboBoxString();

	//Overloads
	virtual BOOL		OnInitialUpdate();
	virtual WCHAR*		GetSelText();

//protected:
	//data
};


/////////////////////////////////////////////////////////////////////
// CInterfaceDlg
//
/////////////////////////////////////////////////////////////////////
class CInterfaceDlg : public CDialogLite
{
public:
	//constructors
	CInterfaceDlg(UINT nID, WCHAR* pwszTitle, REFIID riid);
	virtual ~CInterfaceDlg();

	//Messages
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL		OnUpdate();

	//Helpers
	virtual BOOL		OnAggregation();
	
	//Interface
	virtual REFIID		GetSelInterface()				{	return m_iid;						}
	virtual CAggregate*	GetAggregate()					{	return m_pCAggregate;				}

	//Helpers
	virtual IUnknown**  ppUnknown()						{   return m_ppIUnknown;				}
	virtual IUnknown*	pUnknown()						{   return m_pIUnknown;					}

	//Defaults
	BOOL				m_fAggregation;
	BOOL				m_fOutput;

protected:
	//Title
	WCHAR*				m_pwszTitle;
	
	//Aggregation
	CAggregate*			m_pCAggregate;
	
	//riid
	IID					m_iid;
	
	//Output (ppIUnknown)
	IUnknown*			m_pIUnknown;
	IUnknown**			m_ppIUnknown;

	//Controls
	CComboBoxGuid		m_CComboInterface;
};



/////////////////////////////////////////////////////////////////////
// CEditValueDlg
//
/////////////////////////////////////////////////////////////////////
class CEditValueDlg : public CDialogLite
{
public:
	//constructors
	CEditValueDlg(UINT nID, WCHAR* pwszTitle);
	virtual ~CEditValueDlg();

	//Messages
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();

	//Interface
	virtual void		SetValue(WCHAR* pwszValue)	{	SAFE_FREE(m_pwszValue); m_pwszValue = wcsDuplicate(pwszValue);	}
	virtual WCHAR*		GetValue()					{	return m_pwszValue;			}
	virtual void		SetReqType(DBTYPE wType)	{	m_wReqType = wType;			}

	//Conversion Routines
	virtual HRESULT		SetVariant(VARIANT* pVariant);
	virtual HRESULT		GetVariant(DBTYPE wType, VARIANT* pVariant);

protected:
	//Title
	WCHAR*				m_pwszTitle;
	
	//String
	WCHAR*				m_pwszValue;
	DBTYPE				m_wReqType;
	
	//Controls
	CRichEditLite		m_editValue;
};



/////////////////////////////////////////////////////////////////////////////
// CRowsetViewerDlg
//
/////////////////////////////////////////////////////////////////////////////
class CRowsetViewerDlg
{
public:
	//Constructors
	CRowsetViewerDlg(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);

	//Helpers
	virtual COptionsSheet*	GetOptions();

//protected:
	//Data
	CMainWindow*			m_pCMainWindow;
	CMDIChild*				m_pCMDIChild;
};


/////////////////////////////////////////////////////////////////////
// CPropertiesDlg
//
/////////////////////////////////////////////////////////////////////
class CPropertiesDlg : public CDialogLite, public CRowsetViewerDlg
{
public:
	//constructors
	CPropertiesDlg(CMainWindow* pCMainWindow);
	virtual ~CPropertiesDlg();

	//members
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();
	virtual BOOL		OnCancel();

	//messages
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL		OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);
	virtual BOOL		OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL		OnSelChange(INT iID, HWND hWndCtrl);

	//overrides
	virtual BOOL		OnPropValueChange();
	virtual BOOL		OnPropSetChange();
	virtual BOOL		OnPropOptionsChange();
	virtual BOOL		OnPropTypeChange();

	virtual void		OnAddDefinedProperties();
	virtual void		OnClearAll();

	//Properties
	HRESULT				SetProperties(HWND hWndParent,		const GUID* pGuidPropertySet, REFIID riidProp, IUnknown* pIUnkProperties, IDBProperties* pIDBPropertyInfo, CPropSets* pCPropSets = NULL, IDBDataSourceAdmin* pIDBDataSourceAdmin = NULL);
	HRESULT				GetProperties(HWND hWndParent,		const GUID* pGuidPropertySet, REFIID riidProp, IUnknown* pIUnkProperties, IDBProperties* pIDBPropertyInfo, CPropSets* pCPropSets = NULL, IDBDataSourceAdmin* pIDBDataSourceAdmin = NULL);
	HRESULT				GetPropertyInfo(HWND hWndParent,	const GUID* pGuidPropertySet, REFIID riidProp, IDBProperties* pIDBPropertyInfo, IDBDataSourceAdmin* pIDBDataSourceAdmin = NULL);
	
	//Helpers
	DBPROPINFO*			GetSelPropInfo(INDEX iSelRow, DBPROPINFOSET** ppPropInfoSet = NULL);
	HRESULT				DisplayProperties(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets);
	HRESULT				DisplayProperty(INDEX iItem, DBPROP* pProp, DBPROPINFO* pPropInfo, REFGUID guidPropertySet, LPARAM lParam = 0);

	//Helpers
	HRESULT				SetProperties(ULONG cPropSets, DBPROPSET* rgPropSets);
	HRESULT				GetProperties(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropSets, DBPROPSET** prgPropSets);
	HRESULT				GetPropertyInfo(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets, WCHAR** ppStringBuffer);

	//Data
	IID					m_iidProp;
	const GUID*			m_pGuidPropertySet;
	
	IUnknown*			m_pIUnkProperties;
	IDBProperties*		m_pIDBPropertyInfo;
	IDBDataSourceAdmin*	m_pIDBDataSourceAdmin;

	CPropSets*			m_pCPropSets;

	//Controls
	CListViewLite		m_listviewProps;
	CComboBoxLite		m_comboPropSet;
	CComboBoxLite		m_comboOptions;
	CComboBoxLite		m_comboType;
	CComboBoxLite		m_comboValue;
	CEditBoxLite		m_editDesc;

	//Saved State
	ULONG				m_cPropInfoSets;
	DBPROPINFOSET*		m_rgPropInfoSets;
	WCHAR*				m_pStringBuffer;
	BOOL				m_bClearAll;
	
	//Method Type...
	enum METHOD_TYPE
	{
		METHOD_SETPROPERTIES,
		METHOD_GETPROPERTIES,
		METHOD_GETPROPERTYINFO
	};

	METHOD_TYPE			m_eMethodType;
};


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProvider
//
/////////////////////////////////////////////////////////////////////////////
class CFullConnectProvider : public CPropPageLite
{
public:
	CFullConnectProvider();

	//Messages
	virtual BOOL	OnInitDialog();
	virtual BOOL	OnApply();
	virtual BOOL	OnSetActive();
	virtual BOOL	OnKillActive();
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL	OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);

	//Overloads
	virtual BOOL	OnSelChange(INT iID, HWND hWndCtrl);
	virtual BOOL	OnDropDown(INT iID, HWND hWndCtrl);

	//Provider
	virtual HRESULT GetProviderName();
	virtual BOOL	RefreshProvider();
	virtual BOOL	RefreshEnum(BOOL fReconnect = FALSE);
	virtual BOOL	UpdateProvider();

	//Provider Page
	WCHAR			m_wszLocation[MAX_NAME_LEN+1];		// DBPROP_INIT_LOCATION
	WCHAR			m_wszDataSource[MAX_NAME_LEN+1];	// DBPROP_INIT_DATASOURCE
	WCHAR			m_wszUserID[MAX_NAME_LEN+1];		// DBPROP_AUTH_USERID
	WCHAR			m_wszPassword[MAX_NAME_LEN+1];		// DBPROP_AUTH_PASSWORD
	LONG			m_dwPromptProp;
	HWND			m_hWndProp;

	//Controls
	CComboBoxLite	m_comboConfig;
	CComboBoxEx		m_comboProvider;
};


/////////////////////////////////////////////////////////////////////////////
// CFullConnectProperties
//
/////////////////////////////////////////////////////////////////////////////
class CFullConnectProperties : public CPropPageLite
{
public:
	CFullConnectProperties();

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnSetActive();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(UINT iID, HWND hWndCtrl);

	//Properties
	virtual BOOL RefreshProperties();
	virtual BOOL UpdateProperties();

	//Properties Page
	WCHAR			m_wszProvString[MAX_NAME_LEN+1];	// DBPROP_INIT_PROVIDERSTRING
	WCHAR			m_wszCatalog[MAX_NAME_LEN+1];		// DBPROP_INIT_CATALOG
	LCID			m_dwlcidProp;
	LONG			m_dwAsynchProp;
	LONG			m_dwTimeoutProp;
	LONG			m_dwModeProp;
};


/////////////////////////////////////////////////////////////////////////////
// CFullConnectSecurity
//
/////////////////////////////////////////////////////////////////////////////
class CFullConnectSecurity : public CPropPageLite
{
public:
	CFullConnectSecurity();

	//Security
	virtual BOOL	RefreshSecurity();
	virtual BOOL	UpdateSecurity();

	//Messages
	virtual BOOL	OnInitDialog();
	virtual BOOL	OnSetActive();
	virtual BOOL	OnKillActive();
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);

	//Security Page
	LONG			m_dwProtectionProp;
	LONG			m_dwImpersonateProp;
	VARIANT_BOOL	m_dwMaskPasswordProp;
	VARIANT_BOOL	m_dwEncryptPasswordProp;
	VARIANT_BOOL	m_dwCacheProp;
	VARIANT_BOOL	m_dwPersistProp;
	VARIANT_BOOL	m_dwPersistEncryptProp;
	WCHAR			m_wszIntegrated[MAX_NAME_LEN+1];	// DBPROP_INIT_INTEGRATED
};


/////////////////////////////////////////////////////////////////////////////
// CFullConnectOptions
//
/////////////////////////////////////////////////////////////////////////////
class CFullConnectOptions : public CPropPageLite
{
public:
	CFullConnectOptions();

	//Options
	virtual BOOL	RefreshOptions();
	virtual BOOL	UpdateOptions();

	//Messages
	virtual BOOL	OnInitDialog();
	virtual BOOL	OnSetActive();
	virtual BOOL	OnKillActive();
	virtual BOOL	OnCommand(UINT iID, HWND hWndCtrl);

	//Options Page
	DWORD			m_dwCLSCTX;
	WCHAR			m_wszRemoteServer[MAX_NAME_LEN+1];
	DWORD			m_dwConnectOpts;
};


//////////////////////////////////////////////////////////////////////////
// CFullConnect
//
//////////////////////////////////////////////////////////////////////////
class CFullConnect : public CPropSheetLite, public CRowsetViewerDlg
{
public:
	//Constructors
	CFullConnect(CMainWindow* pCMainWindow);
	virtual ~CFullConnect();

	//Helpers
	virtual BOOL			LoadDefaults();
	virtual BOOL			ResetDefaults();
	virtual BOOL			SaveDefaults();
	virtual INT_PTR			Display(HWND hWndParent, CEnumerator* pCEnumerator = NULL, ENUMINFO* pEnumInfo = NULL);

	//Recent Config
	virtual BOOL			LoadRecentConfig(ULONG iRecentConfig);
	virtual BOOL			AddRecentConfig(WCHAR* pwszConfigName = NULL);
	virtual BOOL			RemoveRecentConfig(WCHAR* pwszConfigName);
	CList<WCHAR*, WCHAR*>	m_listConfigs;

	//Recent Files
	virtual BOOL			LoadRecentFile(ULONG iRecentFile);
	virtual BOOL			AddRecentFile(WCHAR* pwszFileName);
	virtual BOOL			RemoveRecentFile(WCHAR* pwszFileName);
	CList<WCHAR*, WCHAR*>	m_listFiles;
	
	//Enumerators
	virtual HRESULT			DisplayBrowseEnumInfo(HWND hWnd);
	static INT_PTR WINAPI	BrowseEnumeratorProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Connection
	virtual HRESULT			InitPropCombo(HWND hWndCombo, ULONG cItems, const WIDENAMEMAP* rgNameMap, LPARAM lParam);
	virtual HRESULT			GetAdvProperties();

	virtual HRESULT			CreateProviderInstance(REFIID riid, IUnknown** ppIUnknown, CBase** ppCSource = NULL);
	virtual HRESULT			GetPropSets();
	virtual HRESULT			FullConnect();

//protected:
	//Data
	CFullConnectProvider	m_CProvider;
	CFullConnectProperties	m_CProperties;
	CFullConnectSecurity	m_CSecurity;
	CFullConnectOptions		m_COptions;
	UINT					m_idSource;
	
	//Enum
	CEnumerator*			m_pCEnumerator;
	ENUMINFO				m_EnumInfo;

	//Properties
	CPropSets				m_CPropSets;

	//Saved Configuration
	WCHAR					m_wszConfigName[MAX_NAME_LEN+1];
};



/////////////////////////////////////////////////////////////////////
// CInitStringBox
//
/////////////////////////////////////////////////////////////////////
class CInitStringBox : public CRichEditLite
{
public:
	//constructors

	//overrides
	virtual BOOL OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData)
	{
		switch(nVirtKey)
		{
			case VK_TAB:
			case VK_DOWN:
			{
				MSG msg;
				PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
				SendMessage(m_hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(IDB_BROWSE_PROVIDER, m_hWndParent, 0));
				return TRUE;
			}
		
			default:
				break;
		};

		return FALSE;
	}
};


/////////////////////////////////////////////////////////////////////
// CDSLContextBox
//
/////////////////////////////////////////////////////////////////////
class CDSLContextBox : public CListBoxLite
{
public:
	//constructors

	//overrides
	virtual BOOL OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData)
	{
		switch(nVirtKey)
		{
			case VK_TAB:
			{
				MSG msg;
				PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

				//Tab - ends dialog
				SendMessage(m_hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(IDOK, m_hWndParent, 0));
				return TRUE;
			}

			default:
				break;
		};

		return FALSE;
	}
};



/////////////////////////////////////////////////////////////////////
// CSchemaDlg
//
/////////////////////////////////////////////////////////////////////
class CSchemaDlg : public CWndLite, public CRowsetViewerDlg
{
public:
	//constructors
	CSchemaDlg(CMDIChild* pCMDIChild);
	virtual ~CSchemaDlg();

	//abstract members
	virtual INT_PTR			Display();
	static INT_PTR WINAPI	SchemaRowsetProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI	SchemaTreeProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Schema
	virtual HRESULT		InitSchemas(HWND hWnd);
	virtual HRESULT		GetSelectedSchema(SCHEMAINFO** ppSchemaInfo, ULONG* pdwRestrictions = NULL);
	virtual HRESULT		GetSelectedRestrictions();
	virtual HRESULT		ChangeRestrictions();
	virtual HRESULT		InitSchemaCombo(BOOL fShowSupported);

	//TreeView
	virtual HRESULT		InitTreeView(HWND hWnd);
	virtual HRESULT		DisplayTree();
	virtual HRESULT		RefreshTreeControls();

	//Data
	HWND		m_hWndSchemas;
	HWND		m_hWndTreeWnd;
	HWND		m_hWndTreeView;

	//Saved Dialog Values
	BOOL		m_fEditing;
	BOOL		m_fActive;

	ULONG		m_cSchemas;
	GUID*		m_rgSchemas;
	ULONG*		m_rgRestrictionSupport;

	SCHEMAINFO	m_SchemaInfo;
	INDEX		m_iSchemaIndex;
	INDEX		m_iOldSchemaIndex;
	CRowset*	m_pCTreeRowset;

	ULONG		m_cRestrictions;
	VARIANT		m_rgRestrictions[MAX_RESTRICTIONS];
	
	BOOL		m_fUseRestrictions;
	BOOL		m_fUseProperties;
	BOOL		m_fShowSupported;

	//Properties
	CPropSets	m_CPropSets;

	//Restriction Controls
	CWndLite			m_rgResName[MAX_RESTRICTIONS];
	CComboBoxLite		m_rgResType[MAX_RESTRICTIONS];
	CComboBoxString		m_rgResValue[MAX_RESTRICTIONS];

	//Schema Controls
	CComboBoxLite		m_comboSchema;
};





/////////////////////////////////////////////////////////////////////
// CMultipleResultsDlg
//
/////////////////////////////////////////////////////////////////////
class CMultipleResultsDlg : public CInterfaceDlg, public CRowsetViewerDlg
{
public:
	//constructors
	CMultipleResultsDlg(CMDIChild* pCMDIChild);
	virtual ~CMultipleResultsDlg();

	//Messages
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();

	//Helpers

protected:
	//Controls
	CComboBoxLite		m_CComboReserved;
};


/////////////////////////////////////////////////////////////////////
// CConstraintDlg
//
/////////////////////////////////////////////////////////////////////
class CConstraintDlg : public CDialogLite, public CRowsetViewerDlg
{
public:
	//constructors
	CConstraintDlg(CMainWindow* pCMainWindow);
	virtual ~CConstraintDlg();

	//members
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();

	//messages
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);

	//Methods that set constraint desc and table name
	HRESULT				SetConstraint(HWND hWndParent, DBCONSTRAINTDESC *pConsDesc) {
							return SetConstraintAndTable(hWndParent, pConsDesc, NULL);
	}
	HRESULT				SetConstraintAndTable(		
							HWND				hWndParent,
							DBCONSTRAINTDESC	*pConsDesc,			
							WCHAR				**ppwszTableName
						);
	
	//Helpers

	//Data
	DBCONSTRAINTDESC	*m_pConsDesc;
	WCHAR				**m_ppwszTableName;

	//Controls
	CComboBoxLite		m_comboConstraintType;
	CListBoxLite		m_listDeferrability;
	CListBoxLite		m_listColumnList;
	CListBoxLite		m_listFKColumnList;
	CComboBoxLite		m_comboMatchType;
	CComboBoxLite		m_comboUpdateRule;
	CComboBoxLite		m_comboDeleteRule;

	CEditBoxLite		m_editConstraintName;
	CEditBoxLite		m_editBaseTableName;
	CEditBoxLite		m_editReferencedTableName;
	CEditBoxLite		m_editConstraintText;
	CEditBoxLite		m_editColumnName;
	CEditBoxLite		m_editFKColumnName;
}; //CConstraintDlg


/////////////////////////////////////////////////////////////////////
// CAlterIndexDlg
//
/////////////////////////////////////////////////////////////////////
class CAlterIndexDlg : public CDialogLite, public CRowsetViewerDlg
{
public:
	//constructors
	CAlterIndexDlg(CMDIChild *pCMDIChild);
	virtual ~CAlterIndexDlg();

	//members
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();

	//messages
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);

	//Methods that set constraint desc and table name
	HRESULT				AlterIndex(
							HWND				hWndParent,
							DBID				**ppTableID,
							DBID				**ppIndexID,
							DBID				**ppNewIndexID,
							BOOL				*pfUseIndexProps
						);
	
	//Helpers

	//Data
	DBID				**m_ppTableID;
	DBID				**m_ppIndexID;
	DBID				**m_ppNewIndexID;
	BOOL				*m_pfUseIndexProps;
	//Properties
	CPropSets	m_CPropSets;

	//Controls
	CEditBoxLite		m_editTableName;
	CEditBoxLite		m_editIndexName;
	CEditBoxLite		m_editNewIndexName;
}; //CAlterIndexDlg


/////////////////////////////////////////////////////////////////////
// CAlterTableDlg
//
/////////////////////////////////////////////////////////////////////
class CAlterTableDlg : public CDialogLite, public CRowsetViewerDlg
{
	//Controls
	CEditBoxLite		m_editTableName;
	CEditBoxLite		m_editNewTableName;

public:
	//constructors
	CAlterTableDlg(CMDIChild *pCMDIChild);
	virtual ~CAlterTableDlg();

	//members
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();

	//messages
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);

	//Methods that set constraint desc and table name
	HRESULT				AlterTable(HWND hWndParent);
	
	//Helpers

	//Data
	DBID				*m_pTableID;
	DBID				*m_pNewTableID;
	BOOL				m_fUseTableProps;
	//Properties
	CPropSets			m_CPropSets;
}; //CAlterIndexDlg


/////////////////////////////////////////////////////////////////////
// CAlterColumnDlg
//
/////////////////////////////////////////////////////////////////////
class CAlterColumnDlg : public CDialogLite, public CRowsetViewerDlg
{
	//Controls
	CEditBoxLite		m_editTableName;
	CEditBoxLite		m_editColumnName;

	CEditBoxLite		m_editNewColumnName;
	CEditBoxLite		m_editSize;
	CEditBoxLite		m_editPrecision;
	CEditBoxLite		m_editScale;

	CListBoxLite		m_listColumnDescFlags;
	
	CComboBoxLite		m_comboDBType;
	CComboBoxLite		m_comboTypeName;

public:
	//constructors
	CAlterColumnDlg(CMDIChild *pCMDIChild);
	virtual ~CAlterColumnDlg();

	//members
	virtual BOOL		OnInitDialog();
	virtual BOOL		OnOK();

	//messages
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL		OnCommandNotify(INT wNotifyCode, INT iID, HWND hWndCtrl);
	virtual BOOL		OnSelChange(INT iID, HWND hWndCtrl);

	//Methods that set constraint desc and table name
	HRESULT				AlterColumn(HWND hWndParent);
	
	//Helpers

	//Data
	DBID				*m_pTableID;
	DBID				*m_pColumnID;
	BOOL				m_fUseColProps;
	DBCOLUMNDESC		m_ColDesc;
	DBCOLUMNDESCFLAGS   m_dwColFlags;
	//Properties
	CPropSets			m_CPropSets;
}; //CAlterColumnDlg


/////////////////////////////////////////////////////////////////////////////
// CExecuteParamDlg
//
/////////////////////////////////////////////////////////////////////////////
class CExecuteParamDlg : public CDialogLite, public CRowsetViewerDlg
{
public:
	//Constructors
	CExecuteParamDlg(CMDIChild* pCMDIChild);

	//Messages
	virtual BOOL OnInitDialog();
	virtual BOOL OnOK();
	virtual BOOL OnCancel();

	//Overloads
	virtual BOOL OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL OnContextMenu(HWND hWnd, REFPOINTS pts);

	//Helpers
	virtual BOOL OnAddParameter();
	virtual BOOL OnDelParameter();
	virtual BOOL OnAddSet(INDEX iParamSet, BOOL fUseDefaults = TRUE);
	virtual BOOL OnDelSet(INDEX iParamSet);
	virtual BOOL OnStatusChange(DBSTATUS dbStatus);

	//Helpers
	CListViewLite*		GetActiveParam();
	CListViewLite*		GetActiveValue();

protected:
	//data
	CTabLite								m_tabParamSets;
	CVector<CListViewLite*, ULONG_PTR>		m_vecParams;
	CVector<CListViewLite*, ULONG_PTR>		m_vecValues;
};



#endif	//_CDIALOG_H_
