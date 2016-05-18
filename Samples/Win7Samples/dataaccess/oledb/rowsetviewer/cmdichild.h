//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CMDICHILD.H
//
//-----------------------------------------------------------------------------------

#ifndef _CMDICHILD_H_
#define _CMDICHILD_H_


//////////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////////
#include "CDialog.h"
#include "CError.h"
#include "CTrace.h"
#include "CDataLinks.h"
#include "CDataSource.h"
#include "CSession.h"
#include "CCommand.h"
#include "CRowset.h"
#include "CRow.h"
#include "CMultipleResults.h"
#include "CRowPosition.h"
#include "CDataSet.h"
#include "CStream.h"
#include "CBinder.h"

						
/////////////////////////////////////////////////////////////////////
// CQueryBox
//
/////////////////////////////////////////////////////////////////////
class CQueryBox : public CRichEditLite
{
public:
	//constructors
	CQueryBox(CMDIChild* pCMDIChild);
	virtual ~CQueryBox();

	//messages
	virtual BOOL	OnRButtonDown(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL	OnContextMenu(HWND hWnd, REFPOINTS pts);
	virtual BOOL	OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData);

//protected:
	//data
	CMDIChild*		m_pCMDIChild;
};


/////////////////////////////////////////////////////////////////////
// CDataGrid
//
/////////////////////////////////////////////////////////////////////
class CDataGrid : public CListViewLite
{
public:
	//constructors
	CDataGrid(CMDIChild* pCMDIChild);
	virtual ~CDataGrid();

	//messages
	virtual BOOL		OnSize(WPARAM nType, REFPOINTS pts);
	virtual BOOL		OnRButtonDown(WPARAM fwKeys, REFPOINTS pts);
	virtual BOOL		OnContextMenu(HWND hWnd, REFPOINTS pts);
	virtual BOOL		OnKeyDown(WPARAM nVirtKey, LPARAM lKeyData);
	virtual BOOL		OnVScroll(int nScrollCode, int nPos, HWND hWnd);

	//Overloads
	virtual BOOL		OnColumnClick(INT idCtrl, NMLISTVIEW* pNMListView);
	virtual BOOL		OnItemActivate(INT idCtrl, NMLISTVIEW* pNMListView);

	//Helpers
	virtual INDEX		GetSelectedRow(HROW* phRow = NULL, BOOL fValidate = TRUE);
	virtual HRESULT		ScrollGrid(DBROWCOUNT cItems);
	virtual BOOL		ClearAll(WCHAR* pwszEmptyHeader = NULL);

	//Row operations
	virtual HRESULT		AddRefRows(INDEX iIndex);
	virtual HRESULT		ReleaseRows(INDEX iIndex, BOOL fOnlyValidRows);
	virtual HRESULT		ReleaseHeldRows();

	virtual HRESULT		DisplayColumnInfo();

	virtual HRESULT		RefreshData();
	virtual HRESULT		DisplayData(HROW hRow, INDEX iIndex, DBPROPID dwSourceID = DBPROP_IRowset, bool fAlways = FALSE);

	virtual HRESULT		RestartPosition();
	virtual HRESULT		GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, BOOL fRetry = FALSE);	
	virtual HRESULT		DisplayRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM cRowsObtained, HROW* rghRows, BOOL fAdjustFetchPosition = TRUE);
	virtual HRESULT		DisplayFetchPosition(INDEX iIndex, BOOL fLastFetchForward);

	//Interface
	virtual COptionsSheet*	GetOptions();

//protected:
	//data
	CMDIChild*			m_pCMDIChild;
	INDEX				m_lMaxRows;
	INDEX				m_iSelRow;
	INDEX				m_iSelCol;

	//Cursor
	BOOL				m_fLastFetchForward;
	DBROWOFFSET			m_lCurPos;
};


/////////////////////////////////////////////////////////////////////
// CMDIChild
//
/////////////////////////////////////////////////////////////////////
class CMDIChild : public CMDIChildLite
{
public:
	//constructors
	CMDIChild(CMainWindow* pCMainWindow);
	virtual ~CMDIChild();

	virtual BOOL		OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual BOOL		OnDestroy();
	virtual BOOL		OnClose();
	virtual BOOL		OnInitialUpdate();
	virtual BOOL		AutoPosition(BOOL fDefaultPosition = TRUE);

	//Messages
	virtual BOOL		OnSize(WPARAM nType, REFPOINTS pts);
	virtual BOOL		OnSizing(WPARAM nSide, REFPOINTS pts);
	virtual BOOL		OnSetFocus(HWND hWndPrevFocus);
	virtual BOOL		OnMDIActivate(BOOL bActivate, HWND hWndActivate, HWND hWndDeactivate);

	//Overloads
	virtual BOOL		OnCommand(UINT iID, HWND hWndCtrl);
	virtual BOOL		OnNotify(INT idCtrl, NMHDR* pNMHDR);
	virtual BOOL		OnUpdateCommand(HMENU hMenu, UINT nID, DWORD* pdwFlags);

	//members
	virtual BOOL		UpdateControls();
	virtual HRESULT		UpdateWndTitle();
	virtual CBase**		GetObjectAddress(SOURCE eSource = eCUnknown);
	virtual CBase*		GetObject(SOURCE eSource = eCUnknown, BOOL fAlways = FALSE);
	virtual void		SetConfig(WCHAR* pwszConfig, BOOL fCopy = TRUE);

	//Rowset
	virtual HRESULT		HandleRowset(CBase* pCSource, IUnknown* pIUnknown, REFIID riid, DWORD dwFlags, REFGUID guidSource, WCHAR* pwszTableName = NULL, BOOL fSchemaRowset = FALSE);

	//Dataset
	static  INT_PTR WINAPI GetAxisInfoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static  INT_PTR WINAPI GetAxisRowsetProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//ListView
	virtual HRESULT		GetListViewValues(HWND hWndNames, HWND hWndValues, CDataAccess* pCDataAccess, CBindings& rBindings, void* pData);

	//Child Windows
	virtual HRESULT		CreateEnumChild();

	//IRowset
	virtual HRESULT		ChangeSelectedRow(CBase* pCSource, UINT idSource);
	virtual HRESULT		DeleteSelectedRows();
	virtual HRESULT		InsertNewRow();
	virtual HRESULT		UndoChanges();
	virtual HRESULT		UpdateChanges();
	
	//Dialogs
	virtual INT_PTR		DisplayDialog(UINT uID, HWND hWndParent, DLGPROC lpDialogFunc, CBase* pCObject, UINT idSource = 0);
	
	//Info Procs
	static INT_PTR WINAPI GetColInfoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI GetLiteralInfoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI ProviderInfoProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	
	//Transaction Procs
 	static INT_PTR WINAPI StartTransactionProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
   	static INT_PTR WINAPI AbortTransactionProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
   	static INT_PTR WINAPI CommitTransactionProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
   	static INT_PTR WINAPI SetTransactionOptionsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 	static INT_PTR WINAPI JoinTransactionProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 	static INT_PTR WINAPI GetTransactionInfo(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
 	static INT_PTR WINAPI ReleaseTransaction(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual BOOL SetupTransactionCombo(HWND hWnd);

	//Command Procs
	static INT_PTR WINAPI CanConvertProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI SetParameterInfoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI GetParameterInfoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI ExecuteProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI SetCommandTextProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI CommandPersistProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Rowset Procs
	static INT_PTR WINAPI GetNextRowsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI GetBindingsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI RowChangeProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI ColumnChangeProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI OpenRowsetProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI GetColumnsRowsetProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI FindNextRowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI CreateAccessorProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//Chapters
	static INT_PTR WINAPI	GetReferencedRowsetProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual HRESULT			GetChapteredChild(INDEX iSelectedCol, const DBBINDING* pBinding, REFIID riid = IID_IRowset);

	//Row Procs
	static INT_PTR WINAPI RowOpenProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//IDBDataSourceAdmin Procs
	static INT_PTR WINAPI AdminCreateDataSourceProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//IIndexDefinition Procs
	static INT_PTR WINAPI CreateIndexProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI DropIndexProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//ITableDefinition Procs
	static INT_PTR WINAPI AddColumnProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI CreateTableProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI DropColumnProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI DropTableProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//ITableDefinitionWithCosntraints Procs
	static INT_PTR WINAPI DropConstraintProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	//IScopedOperations
	static INT_PTR WINAPI ISCO_DeleteProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR WINAPI ISCO_Proc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	//Interface
	virtual COptionsSheet*	GetOptions();

	//Backpointers
	CMainWindow*			m_pCMainWindow;
	CDataSource*			m_pCDataSource;
	CSession*				m_pCSession;
	CCommand*				m_pCCommand;
	CMultipleResults*		m_pCMultipleResults;
	CDataAccess*			m_pCDataAccess;

	//Controls
	CQueryBox*				m_pCQueryBox;
	CDataGrid*				m_pCDataGrid;

	//Data
	WPARAM					m_lastSizedEdge;
	static	ULONG			m_iChildWindow;
	WCHAR*					m_pwszConfig;

	//Properties
	CPropSets				m_CDefPropSets;

	//Source
	CBase*					m_pCSource;
	ULONG					m_idSource;
};


#endif //_CMDICHILD_H_
