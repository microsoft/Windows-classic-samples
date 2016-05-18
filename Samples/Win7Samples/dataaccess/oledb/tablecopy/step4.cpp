//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module STEP4.CPP
//
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "wizard.h"
#include "common.h"
#include "tablecopy.h"
#include "table.h"


/////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////
//Enum Type Header for the ListView controls
enum COL_HEADERS
{
    //IDL_FROM_TYPES
    //ID_TO_TYPES
    COL_COLNAME			= 0,	//ColInfo.pwszName
    COL_COLTYPENAME		= 1,	//TypeInfo.pwszTypeName
    COL_COLTYPE			= 2,	//ColInfo.wType
    COL_COLORDINAL		= 3,	//ColInfo.iOrdinal
    COL_COLSIZE			= 4,	//ColInfo.ulColumnSize
    COL_COLPREC			= 5,	//ColInfo.bPrecision
    COL_COLSCALE		= 6,	//ColInfo.bScale
    COL_COLISFIXED		= 7,	//ColInfo.dwFlags ISFIXEDLENGTH
    COL_COLISLONG		= 8,	//ColInfo.dwFlags ISLONG
    COL_COLISNULLABLE	= 9,	//ColInfo.dwFlags ISNULLABLE
    COL_COLWRITE		=10,	//ColInfo.dwFlags WRITE
    COL_COLISROWID		=11,	//ColInfo.dwFlags ISROWID
    COL_COLISROWVER		=12,	//ColInfo.dwFlags ISROWVER
};

enum EICON
{
    ICON_COLUMN			= 0,
    ICON_READONLY		= 1,
    ICON_LONG			= 2,
};

/////////////////////////////////////////////////////////////////////
// CS4Dialog::CS4Dialog
//
/////////////////////////////////////////////////////////////////////
CS4Dialog::CS4Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pCTableCopy)
    : CDialogBase(hWnd, hInst)
{
    ASSERT(pCTableCopy);
    m_pCTableCopy = pCTableCopy;
}


/////////////////////////////////////////////////////////////////////
// CS4Dialog::~CS4Dialog
//
/////////////////////////////////////////////////////////////////////
CS4Dialog::~CS4Dialog()
{
}


/////////////////////////////////////////////////////////////////////////////
// ULONG CS4Dialog::Display
//
/////////////////////////////////////////////////////////////////////////////
INT_PTR CS4Dialog::Display()
{							  
    return DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_OPTIONS), NULL, (DLGPROC)DlgProc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////
// CS4Dialog::DlgProc
//
/////////////////////////////////////////////////////////////////////
BOOL WINAPI CS4Dialog::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) 
    {
        case WM_INITDIALOG:
        {
            Busy();
            //Store the "this" pointer, since this is a static method
            CS4Dialog* pThis = (CS4Dialog*)lParam;
            SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            //On INIT we know we have a valid hWnd to store
            pThis->m_hWnd = hWnd;
            
            pThis->InitControls();
            CenterDialog(hWnd);
            pThis->m_pCTableCopy->m_pCWizard->DestroyPrevStep(WIZ_STEP4);
            return HANDLED_MSG;
        }

        case WM_COMMAND:
        {
            //Obtain the "this" pointer
            CS4Dialog* pThis = (CS4Dialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

            // All buttons are handled the same way
            switch(GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDX_COPY_TABLE:
                case IDX_COPY_INDEXES:
                case IDX_SHOW_SQL:

                case IDR_ALL_ROWS:
                case IDR_ROW_COUNT:
                case IDR_PARAM_SETS:
                case IDR_INSERTROW_IMMEDIATE:
                case IDR_INSERTROW_BUFFERED:
                case IDR_BLOB_SIZE:
                case IDR_ISEQ_STREAM:
                    Busy();
                    pThis->RefreshControls();
                    return HANDLED_MSG;


                case IDOK:
                    Busy();
                    if(!pThis->RefreshControls())
                        return HANDLED_MSG;

                    pThis->m_pCTableCopy->m_pCWizard->DisplayStep(WIZ_TYPES);
                    return HANDLED_MSG;

                case IDB_PREV:
                    Busy();
                    if(!pThis->RefreshControls())
                        return HANDLED_MSG;

                    pThis->m_pCTableCopy->m_pCWizard->DisplayStep(WIZ_STEP3);
                    return HANDLED_MSG;
                
                case IDCANCEL:
                    Busy();
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    return HANDLED_MSG;
            }
        }
    }

    return UNHANDLED_MSG;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS4Dialog::InitControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS4Dialog::InitControls()
{
    CDataSource* pCToDataSource = m_pCTableCopy->m_pCToTable->m_pCDataSource;
    
    //IDX_COPY_TABLE (default-checked)
    EnableWindow(GetDlgItem(m_hWnd, IDX_COPY_TABLE), pCToDataSource->m_pICommandText || pCToDataSource->m_pITableDefinition);
    CheckDlgButton(m_hWnd, IDX_COPY_TABLE, m_pCTableCopy->m_fCopyTables);
    wSetDlgItemText(m_hWnd, IDX_COPY_TABLE, pCToDataSource->m_pwszTableTerm);
    
    //IDX_COPY_INDEXES (default-checked)
    EnableWindow(GetDlgItem(m_hWnd, IDX_COPY_INDEXES), pCToDataSource->m_pICommandText || pCToDataSource->m_pIIndexDefinition);
    CheckDlgButton(m_hWnd, IDX_COPY_INDEXES, m_pCTableCopy->m_fCopyIndexes);

    //IDX_COPY_PRIMARYKEYS (default-checked)
    EnableWindow(GetDlgItem(m_hWnd, IDX_COPY_PRIMARYKEYS), pCToDataSource->m_fPrimaryKeysSupported);
    CheckDlgButton(m_hWnd, IDX_COPY_PRIMARYKEYS, m_pCTableCopy->m_fCopyPrimaryKeys);
    
    //IDX_SHOW_SQL (checked)
    EnableWindow(GetDlgItem(m_hWnd, IDX_SHOW_SQL), pCToDataSource->m_pICommandText != NULL);
    CheckDlgButton(m_hWnd, IDX_SHOW_SQL, m_pCTableCopy->m_fShowQuery);

    //IDR_ALL_ROWS (default)
    //IDR_MAX_ROWS
    CheckRadioButton(m_hWnd, IDR_ALL_ROWS, IDR_ROW_COUNT, m_pCTableCopy->m_dwRowOpt);
    wSetDlgItemText(m_hWnd, IDE_ROW_COUNT, L"%lu", m_pCTableCopy->m_ulMaxRows);

    //IDR_PARAM_SETS (default)
    //If multiple param sets are not supported by this driver, select only 1 set
    if(!pCToDataSource->m_fMultipleParamSets)
        m_pCTableCopy->m_ulParamSets = 1;
    if (pCToDataSource->m_pICommandText == NULL)
    {
        m_pCTableCopy->m_dwInsertOpt = IDR_INSERTROW_IMMEDIATE;
        EnableWindow(GetDlgItem(m_hWnd, IDR_PARAM_SETS), FALSE);
    }
    CheckRadioButton(m_hWnd, IDR_PARAM_SETS, IDR_INSERTROW_BUFFERED, m_pCTableCopy->m_dwInsertOpt);
    wSetDlgItemText(m_hWnd, IDE_PARAM_SETS, L"%lu", m_pCTableCopy->m_ulParamSets);

    //Enable IDR_INSERTROW_IMMEDIATE radio button if supported by the provider
    EnableWindow(GetDlgItem(m_hWnd, IDR_INSERTROW_IMMEDIATE), pCToDataSource->m_fIRowsetChange);

    //Enable IDR_INSERTROW_BUFFERED radio button if supported by the provider
    EnableWindow(GetDlgItem(m_hWnd, IDR_INSERTROW_BUFFERED), pCToDataSource->m_fIRowsetUpdate);

    //IDR_BLOB_SIZE	(default)
    //IDR_ISEQ_STREAM 
    EnableWindow(GetDlgItem(m_hWnd, IDR_ISEQ_STREAM), pCToDataSource->m_dwStorageObjects & DBPROPVAL_SS_ISEQUENTIALSTREAM);
    CheckRadioButton(m_hWnd, IDR_BLOB_SIZE, IDR_ISEQ_STREAM, m_pCTableCopy->m_dwBlobOpt);
    wSetDlgItemText(m_hWnd, IDE_BLOB_SIZE, L"%lu", m_pCTableCopy->m_ulBlobSize);

    //Limit the TextLength of the Edit Controls to 10 chars
    SendDlgItemMessage(m_hWnd, IDE_ROW_COUNT, EM_LIMITTEXT, (WPARAM)10, 0L);
    SendDlgItemMessage(m_hWnd, IDE_PARAM_SETS, EM_LIMITTEXT, (WPARAM)10, 0L);
    SendDlgItemMessage(m_hWnd, IDE_BLOB_SIZE, EM_LIMITTEXT, (WPARAM)10, 0L);
    
    //Only allow "NEXT" button if Some form of INSERT is supported
    EnableWindow(GetDlgItem(m_hWnd, IDOK), pCToDataSource->m_pICommandText || pCToDataSource->m_pITableDefinition);
        
    // Refresh the controls
    RefreshControls();
    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// BOOL CS4Dialog::RefreshControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS4Dialog::RefreshControls()
{
    // IDE_ROW_COUNT Get the Copy options first so we can check the row count if given.
    if(IsDlgButtonChecked(m_hWnd, IDR_ROW_COUNT)) 
    {
        m_pCTableCopy->m_dwRowOpt = IDR_ROW_COUNT;
        EnableWindow(GetDlgItem(m_hWnd, IDE_ROW_COUNT), TRUE);
    }
    else
    {
        m_pCTableCopy->m_dwRowOpt = IDR_ALL_ROWS;
        EnableWindow(GetDlgItem(m_hWnd, IDE_ROW_COUNT), FALSE);
    }
        
    //Verify IDE_ROW_COUNT has legal value
    if(!GetEditBoxValue(GetDlgItem(m_hWnd, IDE_ROW_COUNT), 1, LONG_MAX, &m_pCTableCopy->m_ulMaxRows))
        return FALSE;

    //IDE_PARAM_SETS Get the Insert ParamSets options
    if(IsDlgButtonChecked(m_hWnd, IDR_PARAM_SETS))
    {
        m_pCTableCopy->m_dwInsertOpt = IDR_PARAM_SETS;
        EnableWindow(GetDlgItem(m_hWnd, IDE_PARAM_SETS), m_pCTableCopy->m_pCToTable->m_pCDataSource->m_fMultipleParamSets);
    }
    else if(IsDlgButtonChecked(m_hWnd, IDR_INSERTROW_IMMEDIATE))
    {
        //IDR_INSERTROW_IMMEDIATE
        m_pCTableCopy->m_dwInsertOpt = IDR_INSERTROW_IMMEDIATE;
        EnableWindow(GetDlgItem(m_hWnd, IDE_PARAM_SETS), FALSE);
    }
    else
    {
        //IDR_INSERTROW_IMMEDIATE
        m_pCTableCopy->m_dwInsertOpt = IDR_INSERTROW_BUFFERED;
        EnableWindow(GetDlgItem(m_hWnd, IDE_PARAM_SETS), FALSE);
    }

    //Verify IDE_PARAM_SETS has legal value
    if(!GetEditBoxValue(GetDlgItem(m_hWnd, IDE_PARAM_SETS), 1, LONG_MAX, &m_pCTableCopy->m_ulParamSets))
        return FALSE;

    //IDE_BLOB_SIZE Get the Insert ParamSets options
    if(IsDlgButtonChecked(m_hWnd, IDR_BLOB_SIZE))
    {
        m_pCTableCopy->m_dwBlobOpt = IDR_BLOB_SIZE;
        EnableWindow(GetDlgItem(m_hWnd, IDE_BLOB_SIZE), TRUE);
    }
    else
    {
        m_pCTableCopy->m_dwBlobOpt = IDR_ISEQ_STREAM;
        EnableWindow(GetDlgItem(m_hWnd, IDE_BLOB_SIZE), FALSE);
    }
    
    //Verify IDE_BLOB_SIZE has legal value
    if(!GetEditBoxValue(GetDlgItem(m_hWnd, IDE_BLOB_SIZE), 1, MAX_COL_SIZE, &m_pCTableCopy->m_ulBlobSize))
        return FALSE;

    // Get Create options
    m_pCTableCopy->m_fCopyTables = IsDlgButtonChecked(m_hWnd, IDX_COPY_TABLE);
    m_pCTableCopy->m_fCopyIndexes = IsDlgButtonChecked(m_hWnd, IDX_COPY_INDEXES);
    m_pCTableCopy->m_fCopyPrimaryKeys = IsDlgButtonChecked(m_hWnd, IDX_COPY_PRIMARYKEYS);
    
    // Set other Options
    m_pCTableCopy->m_fShowQuery = IsDlgButtonChecked(m_hWnd, IDX_SHOW_SQL);
    return TRUE;
}


/////////////////////////////////////////////////////////////////////
// CTypesDialog::CTypesDialog
//
/////////////////////////////////////////////////////////////////////
CTypesDialog::CTypesDialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pCTableCopy)
    : CDialogBase(hWnd, hInst)
{
    ASSERT(pCTableCopy);
    m_pCTableCopy = pCTableCopy;
}


/////////////////////////////////////////////////////////////////////
// CTypesDialog::~CTypesDialog
//
/////////////////////////////////////////////////////////////////////
CTypesDialog::~CTypesDialog()
{
}


/////////////////////////////////////////////////////////////////////////////
// ULONG CTypesDialog::Display
//
/////////////////////////////////////////////////////////////////////////////
INT_PTR CTypesDialog::Display()
{
    return DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_DATA_TYPES), NULL, (DLGPROC)DlgProc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////
// CTypesDialog::DlgProc
//
/////////////////////////////////////////////////////////////////////
BOOL WINAPI CTypesDialog::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg) 
    {
        case WM_INITDIALOG:
        {
            Busy();
            //Store the "this" pointer, since this is a static method
            CTypesDialog* pThis = (CTypesDialog*)lParam;
            SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            //On INIT we know we have a valid hWnd to store
            CenterDialog(hWnd);
            pThis->m_hWnd = hWnd;
            
            //Tell TableCopy to Map the Types from the Source->Target
            //If MapTypes fails, start over, (ie: same as hitting OK)
            if(FAILED(pThis->m_pCTableCopy->MapTypes()))
            {
                EndDialog(hWnd, 0);
                return UNHANDLED_MSG;
            }

            pThis->InitControls();
            pThis->RefreshControls();

            // Now Display the lists of Mappings
            pThis->ResetTypeLists();
            pThis->m_pCTableCopy->m_pCWizard->DestroyPrevStep(WIZ_TYPES);
            return HANDLED_MSG;
        }

        // Now look for WM_NOTIFY messages
        case WM_NOTIFY:
        {
            if(wParam == IDL_FROM_TYPES || wParam == IDL_TO_TYPES)
            {
                //Obtain the "this" pointer
                CTypesDialog* pThis = (CTypesDialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
                NM_LISTVIEW* pListView = (NM_LISTVIEW*)lParam;
            
                switch(pListView->hdr.code)
                {
                    case LVN_ITEMCHANGED:
                    {
                        if(pListView->uNewState & LVNI_FOCUSED &&
                            pListView->uNewState & LVNI_SELECTED)
                        {
                            if(wParam == IDL_FROM_TYPES)
                            {
                                Busy();
                                SyncSibling(GetDlgItem(hWnd,IDL_TO_TYPES), GetDlgItem(hWnd,IDL_FROM_TYPES));
                                return HANDLED_MSG;
                            }

                            if(wParam == IDL_TO_TYPES)
                            {
                                Busy();
                                SyncSibling(GetDlgItem(hWnd,IDL_FROM_TYPES), GetDlgItem(hWnd,IDL_TO_TYPES));
                                return HANDLED_MSG;
                            }
                            return UNHANDLED_MSG; //No return Value
                        }
                        return UNHANDLED_MSG;
                    }//case LVN_ITEMCHANGED
                }
                return UNHANDLED_MSG;
            }//IDL_TABLES
            return UNHANDLED_MSG;
        }//WM_NOTIFY

        case WM_COMMAND:
        {
            // All buttons are handled the same way

            //Obtain the "this" pointer
            CTypesDialog* pThis = (CTypesDialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
            
            switch(GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDOK:
                    //Now copy the Tables
                    pThis->m_pCTableCopy->CopyTables();

                    Busy();
                    pThis->m_pCTableCopy->m_pCWizard->DisplayStep(WIZ_STEP1);
                    return HANDLED_MSG;
                    
                case IDB_PREV:
                    Busy();
                    pThis->m_pCTableCopy->m_pCWizard->DisplayStep(WIZ_STEP4);
                    return HANDLED_MSG;

                case IDCANCEL:
                    Busy();
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    return HANDLED_MSG;    
            }
        }
    }

    return UNHANDLED_MSG;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CTypesDialog::InitControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTypesDialog::InitControls()
{
    HWND hWndFrom = GetDlgItem(m_hWnd, IDL_FROM_TYPES);
    HWND hWndTo = GetDlgItem(m_hWnd, IDL_TO_TYPES);

    CDataSource* pCFromDataSource = m_pCTableCopy->m_pCFromTable->m_pCDataSource;
    CDataSource* pCToDataSource = m_pCTableCopy->m_pCToTable->m_pCDataSource;
    
    //CONNECT_STRING
    wSetDlgItemText(m_hWnd, IDT_FROM_CONNECT, wsz_TYPES_STRING_,
        pCFromDataSource->m_pwszProviderName,
        pCFromDataSource->m_pwszDataSource, 
        pCFromDataSource->m_pwszTableTerm,
        m_pCTableCopy->m_pCFromTable->m_wszQualTableName);

    //CONNECT_STRING
    wSetDlgItemText(m_hWnd, IDT_TO_CONNECT, wsz_TYPES_STRING_,
        pCToDataSource->m_pwszProviderName,
        pCToDataSource->m_pwszDataSource, 
        pCToDataSource->m_pwszTableTerm,
        m_pCTableCopy->m_pCToTable->m_wszQualTableName);

    //Create the Col ImageList
    HIMAGELIST hColImageList = ImageList_Create(16, 16, ILC_MASK, 1, 0 );

    //IDI_COLUMN - normal column icon
    HICON hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COLUMN));
    ImageList_AddIcon(hColImageList, hIcon);
    //IDI_COLUMNREAD - read-only column icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COLUMNREAD));
    ImageList_AddIcon(hColImageList, hIcon);
    //IDI_COLUMNLONG - long column icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COLUMNLONG));
    ImageList_AddIcon(hColImageList, hIcon);

    //Set image list to the Table Window 
    ListView_SetImageList(hWndFrom, hColImageList, LVSIL_SMALL);
    ListView_SetImageList(hWndTo, hColImageList, LVSIL_SMALL);

    //Use Extended ListView Styles!
    SendMessage(hWndFrom, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);
    SendMessage(hWndTo, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);

    //Source ListView COLUMNS
    LV_InsertColumn(hWndFrom,	COL_COLNAME,		"ColName");
    LV_InsertColumn(hWndFrom,	COL_COLTYPENAME,	"TypeName");
    LV_InsertColumn(hWndFrom,	COL_COLTYPE,		"Type");
    LV_InsertColumn(hWndFrom,	COL_COLSIZE,		"Size");
    LV_InsertColumn(hWndFrom,	COL_COLORDINAL,		"Ordinal");
    LV_InsertColumn(hWndFrom,	COL_COLPREC,		"Precision");
    LV_InsertColumn(hWndFrom,	COL_COLSCALE,		"Scale");
    LV_InsertColumn(hWndFrom,	COL_COLISFIXED,		"ISFIXED");
    LV_InsertColumn(hWndFrom,	COL_COLISLONG,		"ISLONG");
    LV_InsertColumn(hWndFrom,	COL_COLISNULLABLE,	"ISNULLABLE");
    LV_InsertColumn(hWndFrom,	COL_COLWRITE,		"WRITE");
    LV_InsertColumn(hWndFrom,	COL_COLISROWID,		"ISROWID");
    LV_InsertColumn(hWndFrom,	COL_COLISROWVER,	"ISROWVER");

    //Target ListView COLUMNS
    LV_InsertColumn(hWndTo,		COL_COLNAME,		"ColName");
    LV_InsertColumn(hWndTo,		COL_COLTYPENAME,	"TypeName");
    LV_InsertColumn(hWndTo,		COL_COLTYPE,		"Type");
    LV_InsertColumn(hWndTo,		COL_COLSIZE,		"Size");
    LV_InsertColumn(hWndTo,		COL_COLORDINAL,		"Ordinal");
    LV_InsertColumn(hWndTo,		COL_COLPREC,		"Precision");
    LV_InsertColumn(hWndTo,		COL_COLSCALE,		"Scale");
    LV_InsertColumn(hWndTo,		COL_COLISFIXED,		"ISFIXED");
    LV_InsertColumn(hWndTo,		COL_COLISLONG,		"ISLONG");
    LV_InsertColumn(hWndTo,		COL_COLISNULLABLE,	"ISNULLABLE");
    LV_InsertColumn(hWndTo,		COL_COLWRITE,		"WRITE");
    LV_InsertColumn(hWndTo,		COL_COLISROWID,		"ISROWID");
    LV_InsertColumn(hWndTo,		COL_COLISROWVER,	"ISROWVER");

    //AutoSize all columns
    for(ULONG i=0; i<=COL_COLISROWVER; i++)
    {
        SendMessage(hWndFrom, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
        SendMessage(hWndTo, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);
    }
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CTypesDialog::RefreshControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTypesDialog::RefreshControls()
{
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CTypesDialog::ResetTypeLists
//
/////////////////////////////////////////////////////////////////////////////
BOOL CTypesDialog::ResetTypeLists()
{
    HWND		hWndFrom;				// From data types
    HWND		hWndTo;					// To data types
    
    WCHAR		wszBuffer[MAX_NAME_LEN];
    CHAR		szBuffer[MAX_NAME_LEN];	

    CTable* pCFromTable = m_pCTableCopy->m_pCFromTable;
    CTable* pCToTable = m_pCTableCopy->m_pCToTable;

    // Get window handles to speed up the population
    hWndFrom = GetDlgItem(m_hWnd, IDL_FROM_TYPES);
    hWndTo	 = GetDlgItem(m_hWnd, IDL_TO_TYPES);
        
    // Now populate each list
    for(ULONG i=0;  i<pCFromTable->m_cColumns; i++) 
    {
        COLDESC* pCFromColDesc = &pCFromTable->m_rgColDesc[i];
        COLDESC* pCToColDesc = &pCToTable->m_rgColDesc[i];
        
        //COLNAME (item)
        ConvertToMBCS(pCFromColDesc->wszColName, szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndFrom, i, COL_COLNAME, szBuffer, 0, pCFromColDesc->dwFlags & (DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN) ? ((pCFromColDesc->dwFlags & DBCOLUMNFLAGS_ISLONG) ? ICON_LONG : ICON_COLUMN) : ICON_READONLY);
        ConvertToMBCS(pCToColDesc->wszColName, szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndTo, i, COL_COLNAME, szBuffer, 0, pCToColDesc->dwFlags & (DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN) ? ((pCToColDesc->dwFlags & DBCOLUMNFLAGS_ISLONG) ? ICON_LONG : ICON_COLUMN) : ICON_READONLY);

        //COLTYPENAME (subitem)
        pCFromTable->GetTypeNameAndParams(i, wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR));
        ConvertToMBCS(wszBuffer, szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndFrom, i, COL_COLTYPENAME, szBuffer);
        pCToTable->GetTypeNameAndParams(i, wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR));
        ConvertToMBCS(wszBuffer, szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndTo, i, COL_COLTYPENAME, szBuffer);

        //COLTYPE (subitem)
        ConvertToMBCS(GetDBTypeName(pCFromColDesc->wType), szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndFrom, i, COL_COLTYPE, szBuffer);
        ConvertToMBCS(GetDBTypeName(pCToColDesc->wType), szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndTo, i, COL_COLTYPE, szBuffer);

        //Ordinal (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCFromColDesc->iOrdinal);
        LV_InsertItem(hWndFrom, i, COL_COLORDINAL, szBuffer);
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCToColDesc->iOrdinal);
        LV_InsertItem(hWndTo, i, COL_COLORDINAL, szBuffer);

        //ColumnSize (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCFromColDesc->ulColumnSize);
        LV_InsertItem(hWndFrom, i, COL_COLSIZE, szBuffer);
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCToColDesc->ulColumnSize);
        LV_InsertItem(hWndTo, i, COL_COLSIZE, szBuffer);

        //Precision (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCFromColDesc->bPrecision);
        LV_InsertItem(hWndFrom, i, COL_COLPREC, szBuffer);
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCToColDesc->bPrecision);
        LV_InsertItem(hWndTo, i, COL_COLPREC, szBuffer);

        //Scale (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCFromColDesc->bScale);
        LV_InsertItem(hWndFrom, i, COL_COLSCALE, szBuffer);
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pCToColDesc->bScale);
        LV_InsertItem(hWndTo, i, COL_COLSCALE, szBuffer);

        //ISFIXED (SubItem)
        LV_InsertItem(hWndFrom, i, COL_COLISFIXED, pCFromColDesc->dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH ? "TRUE" : "FALSE");
        LV_InsertItem(hWndTo, i, COL_COLISFIXED, pCToColDesc->dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH ? "TRUE" : "FALSE");

        //ISLONG (SubItem)
        LV_InsertItem(hWndFrom, i, COL_COLISLONG, pCFromColDesc->dwFlags & DBCOLUMNFLAGS_ISLONG ? "TRUE" : "FALSE");
        LV_InsertItem(hWndTo, i, COL_COLISLONG, pCToColDesc->dwFlags & DBCOLUMNFLAGS_ISLONG ? "TRUE" : "FALSE");
        
        //ISNULLABLE (SubItem)
        LV_InsertItem(hWndFrom, i, COL_COLISNULLABLE, pCFromColDesc->dwFlags & DBCOLUMNFLAGS_ISNULLABLE ? "TRUE" : "FALSE");
        LV_InsertItem(hWndTo, i, COL_COLISNULLABLE, pCToColDesc->dwFlags & DBCOLUMNFLAGS_ISNULLABLE ? "TRUE" : "FALSE");

        //WRITE (SubItem)
        LV_InsertItem(hWndFrom, i, COL_COLWRITE, pCFromColDesc->dwFlags & (DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN) ? "TRUE" : "FALSE");
        LV_InsertItem(hWndTo, i, COL_COLWRITE, pCToColDesc->dwFlags & (DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN) ? "TRUE" : "FALSE");

        //ISROWID (SubItem)
        LV_InsertItem(hWndFrom, i, COL_COLISROWID, pCFromColDesc->dwFlags & DBCOLUMNFLAGS_ISROWID ? "TRUE" : "FALSE");
        LV_InsertItem(hWndTo, i, COL_COLISROWID, pCToColDesc->dwFlags & DBCOLUMNFLAGS_ISROWID ? "TRUE" : "FALSE");

        //ISROWVER (SubItem)
        LV_InsertItem(hWndFrom, i, COL_COLISROWVER, pCFromColDesc->dwFlags & DBCOLUMNFLAGS_ISROWVER ? "TRUE" : "FALSE");
        LV_InsertItem(hWndTo, i, COL_COLISROWVER, pCToColDesc->dwFlags & DBCOLUMNFLAGS_ISROWVER ? "TRUE" : "FALSE");
    }

    return TRUE;
}


