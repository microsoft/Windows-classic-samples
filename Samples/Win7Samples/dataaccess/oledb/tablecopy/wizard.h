//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module WIZARD.H
//
//-----------------------------------------------------------------------------

#ifndef _WIZARD_H_
#define _WIZARD_H_

/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "winmain.h"
#include "Table.h"


/////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////
enum WIZ_STEP
{
	WIZ_STEP1	= 0,
	WIZ_STEP2	= 1,
	WIZ_STEP3	= 2,
	WIZ_STEP4	= 3,
	WIZ_TYPES	= 4,
	END_WIZ		= 5
};

struct TREEINFO
{
	ULONG ulIndex;
	HTREEITEM hParent;
	HTREEITEM hItem;
};


/////////////////////////////////////////////////////////////////
// Forward Declarations
//
/////////////////////////////////////////////////////////////////
class CTableCopy;
class CDataSource;
class CProgress;


/////////////////////////////////////////////////////////////////////
// CDialogBase
//
/////////////////////////////////////////////////////////////////////
class CDialogBase
{
public:
	//constructors
	CDialogBase(HWND hWnd, HINSTANCE hInst);
	virtual ~CDialogBase();

	//members
	virtual INT_PTR Display() = 0;
	virtual ULONG	Destroy();

	//Data
	HWND		m_hWnd;
	HINSTANCE	m_hInst;
};


/////////////////////////////////////////////////////////////////////
// CS1Dialog
//
/////////////////////////////////////////////////////////////////////
class CS1Dialog : public CDialogBase
{
public:
	//constructors
	CS1Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pTableCopy);
	virtual ~CS1Dialog();

	//abstract members
	static BOOL WINAPI DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR Display();

	//members
	virtual BOOL InitControls();
	virtual BOOL RefreshControls();

	virtual BOOL ResetTableList(HWND hWndTable, HWND hWndCol);
	virtual BOOL ChangeTableName(LONG iIndex);

	virtual BOOL ResetColInfo(HWND hWndCol);
	virtual BOOL GetTableColInfo(HWND hWndCol);
	virtual BOOL CreateTableNode(TREEINFO* rgTreeInfo, ULONG ulNameOffset, LONG iParam = 0, LONG iImage = 0, LONG iSelectedImage = 0);

	//Data
	CTableCopy* m_pCTableCopy;
	BOOL		m_fEditing;

	ULONG		m_cTables;
	TABLEINFO*	m_rgTableInfo;
};


/////////////////////////////////////////////////////////////////////
// CS2Dialog
//
/////////////////////////////////////////////////////////////////////
class CS2Dialog : public CDialogBase
{
public:
	//constructors
	CS2Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pTableCopy);
	virtual ~CS2Dialog();

	//abstract members
	static BOOL WINAPI DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR Display();

	//members
	virtual BOOL InitControls();
	virtual BOOL RefreshControls();

	virtual BOOL GetPrimaryKeys();

	virtual BOOL ResetIndexList(HWND hWnd);
	virtual BOOL RecordSelectedIndexes(HWND hWnd);

	//Data
	CTableCopy* m_pCTableCopy;
};



/////////////////////////////////////////////////////////////////////
// CS3Dialog
//
/////////////////////////////////////////////////////////////////////
class CS3Dialog : public CDialogBase
{
public:
	//constructors
	CS3Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pTableCopy);
	virtual ~CS3Dialog();

	//abstract members
	static BOOL WINAPI DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR Display();

	//members
	virtual BOOL InitControls();
	virtual BOOL RefreshControls();

	virtual BOOL Connect(CDataSource* pCDataSource = NULL);
	virtual BOOL EnableTable();

	//Data
	CTableCopy* m_pCTableCopy;
};


/////////////////////////////////////////////////////////////////////
// CS4Dialog
//
/////////////////////////////////////////////////////////////////////
class CS4Dialog : public CDialogBase
{
public:
	//constructors
	CS4Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pTableCopy);
	virtual ~CS4Dialog();

	//abstract members
	static BOOL WINAPI DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR Display();
	
	//members
	virtual BOOL InitControls();
	virtual BOOL RefreshControls();

	//Data
	CTableCopy* m_pCTableCopy;
};


/////////////////////////////////////////////////////////////////////
// CTypesDialog
//
/////////////////////////////////////////////////////////////////////
class CTypesDialog : public CDialogBase
{
public:
	//constructors
	CTypesDialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pTableCopy);
	virtual ~CTypesDialog();

	//abstract members
	static BOOL WINAPI DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR Display();

	//members
	virtual BOOL InitControls();
	virtual BOOL RefreshControls();
	
	virtual BOOL ResetTypeLists();

	//Data
	CTableCopy* m_pCTableCopy;
};


/////////////////////////////////////////////////////////////////////
// CWizard
//
/////////////////////////////////////////////////////////////////////
class CWizard : public CDialogBase
{
public:
	//Constructors
	CWizard(HWND hWnd, HINSTANCE hInst);
	virtual ~CWizard();

	//Members
	virtual INT_PTR Display();
	virtual INT_PTR	DisplayStep(ULONG iStep);
	virtual ULONG	DestroyPrevStep(ULONG iCurStep);

	//Data
	CTableCopy*	m_pCTableCopy;
	CProgress*  m_pCProgress;

	ULONG		m_iPrevStep;
	CDialogBase*	m_rgDialogSteps[END_WIZ];
};



#endif	//_WIZARD_H_
