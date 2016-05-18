//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module STEP1.CPP
//
//-----------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////////
#include "wizard.h"
#include "common.h"
#include "table.h"

/////////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////////

//Enum Column Header for the ListView controls
enum COL_HEADERS
{
    //IDL_COLUMNS
    COL_COLNAME			= 0,	//ColInfo.pwszName
    COL_COLTYPE			= 1,	//ColInfo.wType
    COL_COLORDINAL		= 2,	//ColInfo.iOrdinal
    COL_COLSIZE			= 3,	//ColInfo.ulColumnSize
    COL_COLPREC			= 4,	//ColInfo.bPrecision
    COL_COLSCALE		= 5,	//ColInfo.bScale
    COL_COLISFIXED		= 6,	//ColInfo.dwFlags ISFIXEDLENGTH
    COL_COLISLONG		= 7,	//ColInfo.dwFlags ISLONG
    COL_COLISNULLABLE	= 8,	//ColInfo.dwFlags ISNULLABLE
    COL_COLWRITE		= 9,	//ColInfo.dwFlags WRITE
    COL_COLISROWID		=10,	//ColInfo.dwFlags ISROWID
    COL_COLISROWVER		=11,	//ColInfo.dwFlags ISROWVER
};

enum EICON
{
    ICON_CATALOG		= 0,
    ICON_SCHEMA			= 1,
    ICON_TYPE			= 2,
    ICON_TABLE			= 3,
    ICON_SYSTABLE		= 4,
    ICON_VIEW			= 5,
    ICON_SYNONYM		= 6,

    ICON_COLUMN			= 0,
    ICON_READONLY		= 1,
    ICON_LONG			= 2,
};


/////////////////////////////////////////////////////////////////////
// CS1Dialog::CS1Dialog
//
/////////////////////////////////////////////////////////////////////
CS1Dialog::CS1Dialog(HWND hWnd, HINSTANCE hInst, CTableCopy* pCTableCopy)
    : CDialogBase(hWnd, hInst)
{
    ASSERT(pCTableCopy);
    m_pCTableCopy = pCTableCopy;
    m_fEditing = FALSE;

    m_cTables = 0;
    m_rgTableInfo = NULL;
}


/////////////////////////////////////////////////////////////////////
// CS1Dialog::~CS1Dialog
//
/////////////////////////////////////////////////////////////////////
CS1Dialog::~CS1Dialog()
{
    SAFE_FREE(m_rgTableInfo);
}


/////////////////////////////////////////////////////////////////////////////
// ULONG CS1Dialog::Display
//
/////////////////////////////////////////////////////////////////////////////
INT_PTR CS1Dialog::Display()
{
    //Create a Modal dialog box
    return DialogBoxParam(m_hInst, MAKEINTRESOURCE(IDD_FROM_INFO), NULL, (DLGPROC)DlgProc, (LPARAM)this);
}


/////////////////////////////////////////////////////////////////////
// CS1Dialog::DlgProc
//
/////////////////////////////////////////////////////////////////////
BOOL WINAPI CS1Dialog::DlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static WCHAR	wszBuffer[MAX_NAME_LEN+1];

    switch(msg) 
    {
        case WM_INITDIALOG:
        {
            Busy();
            CS1Dialog* pThis = (CS1Dialog*)lParam;
            SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
            
            //On INIT we know we have a valid hWnd to store
            CenterDialog(hWnd);
            pThis->m_hWnd = hWnd;
            CTable* pCFromTable = pThis->m_pCTableCopy->m_pCFromTable;

            //Init all controls to the default values
            pThis->InitControls();

            // If there is a source to look at, Display the table list
            if(pCFromTable->IsConnected()) 
                pThis->ResetTableList(GetDlgItem(hWnd, IDL_TABLES), GetDlgItem(hWnd, IDL_COLUMNS));

            pThis->RefreshControls();
            pThis->m_pCTableCopy->m_pCWizard->DestroyPrevStep(WIZ_STEP1);
            return HANDLED_MSG;
        }//case WM_INITDIALOG

        case WM_COMMAND:
        {
            //Obtain the "this" pointer
            CS1Dialog	*pThis = (CS1Dialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

            CTable		*pCFromTable		= pThis->m_pCTableCopy->m_pCFromTable;		
            CTable		*pCToTable			= pThis->m_pCTableCopy->m_pCToTable;		
            CDataSource	*pCToDataSource		= pCToTable->m_pCDataSource;		
            CDataSource	*pCFromDataSource	= pCFromTable->m_pCDataSource;		
            
            //Filter out any Control Notification codes
            if(GET_WM_COMMAND_CMD(wParam, lParam) > 1)
            {
                return UNHANDLED_MSG;
            }

            //LBN_SELCHANGE ListBox Selection change
            if(GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE
                && IDC_PROVIDER_NAME == GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                Busy();
/*
                //Get new selection
                LONG iSel = 0;
                if((iSel = SendMessage(GetDlgItem(pThis->m_hWnd, IDC_PROVIDER_NAME), CB_GETCURSEL, 0, 0L)) != CB_ERR)
                {
                    //Since we have the CBS_SORT turned on, the order in the Combo Box does
                    //not match our array, so we pass the array index (lParam) as the item data
                    LONG lParam = SendMessage(GetDlgItem(pThis->m_hWnd, IDC_PROVIDER_NAME), CB_GETITEMDATA, iSel, 0L);
                    if((lParam < (LONG)pCFromDataSource->m_cProviderInfo) && (wcscmp(pCFromDataSource->m_rgProviderInfo[lParam].wszName, pCFromDataSource->m_pwszProviderName)!=0))
                    {
                        //Clear Table/Column List Views
                        SendMessage(GetDlgItem(hWnd, IDL_TABLES), TVM_DELETEITEM, (WPARAM)0, (LPARAM)TVI_ROOT);
                        SendMessage(GetDlgItem(hWnd, IDL_COLUMNS), LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0);

                        //Clear Table info
                        memset(&pCFromTable->m_TableInfo, 0, sizeof(TABLEINFO));
                        pCFromTable->m_wszQualTableName[0] = EOL;

                        //Disconnect from the DataSource and Update controls
                        pCFromTable->m_pCDataSource->Disconnect();
                        pThis->RefreshControls();
                    }
                }
                else
                {
                    //The user must have typed in a Provider Name directly
                    //This may not map to a provider in the list, so assume the name is a ProgID/CLSID
                    LONG lParam = wSendMessage(GetDlgItem(pThis->m_hWnd, IDC_PROVIDER_NAME), CB_GETLBTEXT, 0, wszBuffer);
                }
                return HANDLED_MSG;
*/
            }
            if(pThis->m_fEditing)
            {
                //There is a bug in the TreeView control for editing here is KB article
                //Article ID: Q130691 BUG: ESC/ENTER Keys Don't Work When Editing Labels in TreeView
                //So one way to work around this is to just have a flag (m_fEditing)
                //to indicate we were in editing mode.
                SendDlgItemMessage(hWnd, IDL_TABLES, TVM_ENDEDITLABELNOW, (WPARAM) (wParam==IDCANCEL ? TRUE : FALSE), (LPARAM)0);
                return HANDLED_MSG;
            }

            // Now check for regular command ids
            switch(GET_WM_COMMAND_ID(wParam, lParam)) 
            {
                case IDB_FROM_CONNECT:
                {
                    //on connect get whatever is now listed in the drop down
                    LRESULT iSel = 0;
                    if((iSel = SendMessage(GetDlgItem(pThis->m_hWnd, IDC_PROVIDER_NAME), CB_GETCURSEL, 0, 0L)) != CB_ERR)
                    {
                        //Since we have the CBS_SORT turned on, the order in the Combo Box does
                        //not match our array, so we pass the array index (lParam) as the item data
                        LRESULT lParam = SendMessage(GetDlgItem(pThis->m_hWnd, IDC_PROVIDER_NAME), CB_GETITEMDATA, iSel, 0L);
                        pCFromTable->m_pCDataSource->m_pwszProviderName = pCFromDataSource->m_rgProviderInfo[lParam].wszName;
                        if((lParam < (LONG)pCFromDataSource->m_cProviderInfo) && (wcscmp(pCFromDataSource->m_rgProviderInfo[lParam].wszName, pCFromDataSource->m_pwszProviderName)!=0))
                        {
                            //Clear Table/Column List Views
                            SendMessage(GetDlgItem(hWnd, IDL_TABLES), TVM_DELETEITEM, (WPARAM)0, (LPARAM)TVI_ROOT);
                            SendMessage(GetDlgItem(hWnd, IDL_COLUMNS), LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0);

                            //Clear Table info
                            memset(&pCFromTable->m_TableInfo, 0, sizeof(TABLEINFO));
                            pCFromTable->m_wszQualTableName[0] = EOL;

                            //Disconnect from the DataSource and Update controls
                            pCFromTable->m_pCDataSource->Disconnect();
                        }
                    }
                    else
                    {
                        //The user must have typed in a Provider Name directly
                        //This may not map to a provider in the list, so assume the name is a ProgID/CLSID
                        wSendMessage(GetDlgItem(pThis->m_hWnd, IDC_PROVIDER_NAME), WM_GETTEXT, MAX_NAME_LEN, wszBuffer);
                        pCFromTable->m_pCDataSource->m_pwszProviderName = wszBuffer;
                    }
    
                    //Try to connect to the DataSource
                    Busy();
                    if(pCFromTable->Connect(hWnd))
                    {
                        Busy();
                        //Clear Table info
                        memset(&pCFromTable->m_TableInfo, 0, sizeof(TABLEINFO));
                        pCFromTable->m_wszQualTableName[0] = EOL;

                        //ResetTableList
                        pThis->ResetTableList(GetDlgItem(hWnd, IDL_TABLES), GetDlgItem(hWnd, IDL_COLUMNS));
                    }
                    
                    pThis->RefreshControls();
                    return HANDLED_MSG;
                }//case IDB_FROM_CONNECT

                case IDOK:
                    Busy();
                    pThis->GetTableColInfo(GetDlgItem(hWnd, IDL_COLUMNS));
                    pThis->m_pCTableCopy->m_pCWizard->DisplayStep(WIZ_STEP2);
                    return HANDLED_MSG;
                
                case IDCANCEL:
                    Busy();
                    EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam));
                    return HANDLED_MSG;
            }//switch GET_WM_COMMAND_ID

            return UNHANDLED_MSG;
        }//case WM_COMMAND

        
        // Now look for WM_NOTIFY messages
        case WM_NOTIFY:
        {
            if(wParam == IDL_COLUMNS)
            {
                //Obtain the "this" pointer
                CS1Dialog* pThis = (CS1Dialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
                NM_LISTVIEW* pListView = (NM_LISTVIEW*)lParam;
            
                switch(pListView->hdr.code)
                {
                    case LVN_ITEMCHANGED:
                    {
                        //Refresh Controls, ("Next" button)
                        pThis->RefreshControls();
                        return UNHANDLED_MSG;
                    }
                }
                return UNHANDLED_MSG;
            }

            if(wParam == IDL_TABLES)
            {
                //Obtain the "this" pointer
                CS1Dialog* pThis = (CS1Dialog*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);
                CTable* pCFromTable = pThis->m_pCTableCopy->m_pCFromTable;
                NM_TREEVIEW* pTreeView = (NM_TREEVIEW*)lParam;
            
                switch(pTreeView->hdr.code)
                {								 
                    case TVN_BEGINLABELEDIT:
                    {
                        //Idicate we have started to edit
                        pThis->m_fEditing = TRUE;
                        return FALSE; //Allow the edited change
                    }

                    case TVN_ENDLABELEDIT:
                    {
                        Busy();
                        pThis->m_fEditing = FALSE;
                        TV_DISPINFO* pDispInfo = (TV_DISPINFO*)lParam;

                        //If Schemas are available - don't allow the change
                        if(pCFromTable->m_pCDataSource->m_pIDBSchemaRowset)
                            return FALSE;

                        //Just need to obtain the new tablename...
                        if(pDispInfo->item.pszText)
                        {
                            //Now update the window TableName myself
                            TV_ITEM tvItem = { TVIF_TEXT | TVIF_STATE, pDispInfo->item.hItem, TVIS_SELECTED, TVIS_SELECTED, pDispInfo->item.pszText, 0, 0, 0, 0, 0};
                            SendDlgItemMessage(hWnd, IDL_TABLES, TVM_SETITEM, (WPARAM)0, (LPARAM)&tvItem);

                            //Change the TableName (if different)
                            memset(&pThis->m_rgTableInfo[0], 0, sizeof(TABLEINFO));
                            ConvertToWCHAR(pDispInfo->item.pszText, pThis->m_rgTableInfo[0].wszTableName, MAX_NAME_LEN);
                            pThis->ChangeTableName(0); 
                        }
                                                
                        //Refresh Controls ("Next" button);
                        pThis->RefreshControls();
                        return TRUE; //Allow the edited change
                    }
                        
                    case TVN_SELCHANGED:
                    {
                        //There is a problem with the SELCHANGED notification
                        //It can be sent when either a item is selected or
                        //DELETED, since when an item deleted the selection moves
                        //to a different selection.
                        if((pTreeView->itemNew.state == TVIS_SELECTED && pTreeView->action)
                            || (pTreeView->itemNew.state & TVIS_SELECTED && pTreeView->itemNew.state != TVIS_SELECTED)) 
                        {
                            Busy();
                            //We assume it sends us the Param of the item
                            ASSERT(pTreeView->itemNew.mask & TVIF_PARAM);
                
                            //Change the TableName (if different)
                            pThis->ChangeTableName((LONG)pTreeView->itemNew.lParam); 
                        }

                        //Refresh Controls ("Next" button);
                        pThis->RefreshControls();
                        return UNHANDLED_MSG;
                    }
                }
                return UNHANDLED_MSG;
            }//IDL_TABLES
            return UNHANDLED_MSG;
        }//WM_NOTIFY
    }//msg

    return UNHANDLED_MSG;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::InitControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::InitControls()
{
    HWND		hWndTable		= GetDlgItem(m_hWnd, IDL_TABLES);
    HWND		hWndCol			= GetDlgItem(m_hWnd, IDL_COLUMNS);
    HWND		hWndProv		= GetDlgItem(m_hWnd, IDC_PROVIDER_NAME);
    CDataSource	*pCDataSource	= m_pCTableCopy->m_pCFromTable->m_pCDataSource;
    HRESULT		hr;	

    //Create the Table ImageList
    HIMAGELIST hTableImageList = ImageList_Create(16, 16, ILC_MASK, 1, 0 );

    //IDI_CATALOG - normal catalog icon
    HICON hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_CATALOG));
    ImageList_AddIcon(hTableImageList, hIcon);
    //IDI_SCHEMA - normal schema icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SCHEMA));
    ImageList_AddIcon(hTableImageList, hIcon);
    //IDI_TYPE - normal type icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_TYPE));
    ImageList_AddIcon(hTableImageList, hIcon);
    //IDI_TABLE - normal table icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_TABLE));
    ImageList_AddIcon(hTableImageList, hIcon);
    //IDI_SYSTABLE - normal system table icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SYSTABLE));
    ImageList_AddIcon(hTableImageList, hIcon);
    //IDI_VIEW - normal view icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_VIEW));
    ImageList_AddIcon(hTableImageList, hIcon);
    //IDI_SYNONYM - normal synonym icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_SYNONYM));
    ImageList_AddIcon(hTableImageList, hIcon);

    //Set image list to the Table Window 
    TreeView_SetImageList(hWndTable, hTableImageList, TVSIL_NORMAL);
    
    //Create the Col ImageList
    HIMAGELIST hColImageList = ImageList_Create(16, 16, ILC_MASK, 1, 0 );

    //IDI_COLUMN - normal column icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COLUMN));
    ImageList_AddIcon(hColImageList, hIcon);
    //IDI_COLUMNREAD - read-only column icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COLUMNREAD));
    ImageList_AddIcon(hColImageList, hIcon);
    //IDI_COLUMNLONG - long column icon
    hIcon = LoadIcon(m_hInst, MAKEINTRESOURCE(IDI_COLUMNLONG));
    ImageList_AddIcon(hColImageList, hIcon);

    //Set image list to the Table Window 
    ListView_SetImageList(hWndCol, hColImageList, LVSIL_SMALL);

    //ListView COLUMNS
    LV_InsertColumn(hWndCol,	COL_COLNAME,		"ColName");
    LV_InsertColumn(hWndCol,	COL_COLTYPE,		"Type");
    LV_InsertColumn(hWndCol,	COL_COLSIZE,		"Size");
    LV_InsertColumn(hWndCol,	COL_COLORDINAL,		"Ordinal");
    LV_InsertColumn(hWndCol,	COL_COLPREC,		"Precision");
    LV_InsertColumn(hWndCol,	COL_COLSCALE,		"Scale");
    LV_InsertColumn(hWndCol,	COL_COLISFIXED,		"ISFIXED");
    LV_InsertColumn(hWndCol,	COL_COLISLONG,		"ISLONG");
    LV_InsertColumn(hWndCol,	COL_COLISNULLABLE,	"ISNULLABLE");
    LV_InsertColumn(hWndCol,	COL_COLWRITE,		"WRITE");
    LV_InsertColumn(hWndCol,	COL_COLISROWID,		"ISROWID");
    LV_InsertColumn(hWndCol,	COL_COLISROWVER,	"ISROWVER");

    ULONG i;
    //AutoSize all columns
    for( i=0; i<=COL_COLISROWVER; i++)
        SendMessage(hWndCol, LVM_SETCOLUMNWIDTH, (WPARAM)i,		(LPARAM)LVSCW_AUTOSIZE_USEHEADER);

    //Use Extended ListView Styles!
    SendMessage(hWndCol, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_TWOCLICKACTIVATE | LVS_EX_SUBITEMIMAGES);

    //Initialize the Provider List (if not done so already)
    if(pCDataSource->m_rgProviderInfo == NULL)
    {
        hr =	pCDataSource->GetProviders();
    }
    
    WCHAR wszBuffer[MAX_NAME_LEN*2];

    //Fill out the provider name combo box.
    for(i=0; i<pCDataSource->m_cProviderInfo; i++)
    {
        //Add the name to the list
        //Since we have the CBS_SORT turned on, the order in the Combo Box does
        //not match our array, so we pass the array index (lParam) as the item data
        StringCchPrintfW(wszBuffer, sizeof(wszBuffer)/sizeof(WCHAR), wsz_PROVIDER_INFO_, pCDataSource->m_rgProviderInfo[i].wszName);
        LRESULT iIndex = wSendMessage(hWndProv, CB_ADDSTRING, (WPARAM)0, wszBuffer);
        SendMessage(hWndProv, CB_SETITEMDATA, (WPARAM)iIndex, (LPARAM)i);
    }

    //By default, it selects MSDASQL
    if(pCDataSource->m_pwszProviderName == NULL)
    {
        pCDataSource->m_pwszProviderName = L"MSDASQL";
    }

    //Try and select the previous selected Provider
    if(CB_ERR == wSendMessage(hWndProv, CB_SELECTSTRING, 0, pCDataSource->m_pwszProviderName))
    {
        //If not found, just select the first one
        SendMessage(hWndProv, CB_SETCURSEL, 0, 0);
    }

    //Indicate were not in the middle of an TreeView editing command
    m_fEditing = FALSE;
    
    // Enable Connect button only if there are providers installed.
    EnableWindow(GetDlgItem(m_hWnd, IDB_FROM_CONNECT), SendMessage(hWndProv, CB_GETCURSEL, 0, 0L) != CB_ERR);
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::RefreshControls
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::RefreshControls()
{
    CTable* pCFromTable = m_pCTableCopy->m_pCFromTable;
    CDataSource* pCDataSource = pCFromTable->m_pCDataSource;

    // Must have a connection to edit other controls
    BOOL fConnected = pCFromTable->IsConnected();
    LRESULT cSelColumns = SendDlgItemMessage(m_hWnd, IDL_COLUMNS, LVM_GETSELECTEDCOUNT, 0, 0);

    //Enable dialog items, only if connected
    EnableWindow(GetDlgItem(m_hWnd, IDL_TABLES),		fConnected);
    EnableWindow(GetDlgItem(m_hWnd, IDL_COLUMNS),		fConnected);
    EnableWindow(GetDlgItem(m_hWnd, IDT_FROMTABLEHELP),	fConnected);
     
    //Enable OK/Next if there is a table and at least 1 column selected
    EnableWindow(GetDlgItem(m_hWnd, IDOK),	fConnected && cSelColumns);

    //Store the selected ProviderName and ProviderDesc
    LRESULT iSel = 0;
    if((iSel = SendMessage(GetDlgItem(m_hWnd, IDC_PROVIDER_NAME), CB_GETCURSEL, 0, 0L)) != CB_ERR)
    {
        //Since we have the CBS_SORT turned on, the order in the Combo Box does
        //not match our array, so we pass the array index (lParam) as the item data
        LRESULT lParam = SendMessage(GetDlgItem(m_hWnd, IDC_PROVIDER_NAME), CB_GETITEMDATA, iSel, 0L);
        ASSERT(lParam < (LONG)pCDataSource->m_cProviderInfo);
        pCDataSource->m_pwszProviderName = pCDataSource->m_rgProviderInfo[lParam].wszName;
        pCDataSource->m_pwszProviderParseName = pCDataSource->m_rgProviderInfo[lParam].wszParseName;
    }

    // Show user the connection string
    if(fConnected)
    {
        //CONNECT_STRING
        wSetDlgItemText(m_hWnd, IDT_CONNECT, wsz_CONNECT_STRING_, 
            pCDataSource->m_pwszProviderName,
            pCDataSource->m_pwszDataSource, 
            pCDataSource->m_pwszDBMS,
            pCDataSource->m_pwszDBMSVer,
            pCDataSource->m_pwszProviderFileName,
            pCDataSource->m_pwszProviderVer);
            
        //TABLEHELPMSG
        //Display the Qualified TableName
        if(pCFromTable->m_TableInfo.wszTableName[0])
            wSetDlgItemText(m_hWnd, IDT_FROMTABLEHELP, wsz_FROMQUALTABLE_, pCFromTable->m_wszQualTableName, cSelColumns);
        else 
            wSetDlgItemText(m_hWnd, IDT_FROMTABLEHELP, wsz_FROMTABLEHELP_, pCDataSource->m_pwszTableTerm);
    }
    else
    {
        //NO CONNECT_STRING
        wSetDlgItemText(m_hWnd, IDT_CONNECT, wsz_NOT_CONNECTED);
    }
        
    return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::GetTableColInfo
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::GetTableColInfo(HWND hWndCol)
{
    CTable* pCFromTable = m_pCTableCopy->m_pCFromTable;

    // Get the count of selected items
    LRESULT cSelColumns = SendMessage(hWndCol, LVM_GETSELECTEDCOUNT, (WPARAM)0, (LPARAM)0);
    LRESULT	iIndex = -1;

    //Loop over all the selected columns in the list
    pCFromTable->m_cColumns = (ULONG)cSelColumns;
    for(LRESULT i=0; i<cSelColumns; i++) 
    {
        iIndex = SendMessage(hWndCol, LVM_GETNEXTITEM, (WPARAM)iIndex, (LPARAM)LVNI_SELECTED);
        
        //"compact" the m_rgColDesc array to only the selected items
        if(iIndex != LVM_ERR)
            memmove(&pCFromTable->m_rgColDesc[i], &pCFromTable->m_rgColDesc[iIndex], sizeof(COLDESC));
    }
    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::CreateTableNode
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::CreateTableNode(TREEINFO* rgTreeInfo, ULONG ulNameOffset, LONG iParam, LONG iImage, LONG iSelectedImage)
{
    //ulNameOffset is the offset into TABELEINFO indicating
    //Catalog / Schema / Type name were interested in comparing
    ASSERT(ulNameOffset == offsetof(TABLEINFO, wszCatalogName)
        || ulNameOffset == offsetof(TABLEINFO, wszSchemaName)
        || ulNameOffset == offsetof(TABLEINFO, wszType));
    
    //Create the specified node in the tree.
    //Basically this is a fairly complex function that builds the nodes to the 
    //TreeView control, listing Catalog/Schema/TableType/TableName heieracrcy.

    //This is an extremly simple algortym that loops over the specified column 
    //(Catalog/Schema/Type) and adds only unique names as a node to the TreeView.
    //It then updates "rgTreeInfo" hParents and hItems accordingly.

    CHAR szBuffer[MAX_NAME_LEN];

    ULONG i;
    //Loop over all Tables
    ULONG cFoundInfo = 0;
    for(i=0; i<m_cTables; i++)
    {
        BOOL bFound = FALSE;
        WCHAR* pwszName = (WCHAR*)((BYTE*)&m_rgTableInfo[i] + ulNameOffset);

        //Try to find Type value in the FoundList
        for(ULONG j=0; j<cFoundInfo; j++)
        {
            ULONG ulFoundIndex = rgTreeInfo[j].ulIndex;
            WCHAR* pwszFoundName = (WCHAR*)((BYTE*)&m_rgTableInfo[ulFoundIndex] + ulNameOffset);
            
            if(wcscmp(pwszName, pwszFoundName)==0
                && rgTreeInfo[i].hParent == rgTreeInfo[ulFoundIndex].hParent)
            {
                bFound = TRUE;
                rgTreeInfo[i].hItem = rgTreeInfo[ulFoundIndex].hItem;
                break;
            }
        }	
        
        if(!bFound && pwszName[0])
        {
            //Add it to the list
            rgTreeInfo[cFoundInfo].ulIndex = i;
            cFoundInfo++;

            //Add it to the TreeView
            ConvertToMBCS(pwszName, szBuffer, MAX_NAME_LEN);
            rgTreeInfo[i].hItem = TV_InsertItem(GetDlgItem(m_hWnd, IDL_TABLES), rgTreeInfo[i].hParent, TVI_SORT, szBuffer, iParam, iImage, iSelectedImage);
        }
    }

    //Update Parents
    for(i=0; i<m_cTables; i++)
        rgTreeInfo[i].hParent = rgTreeInfo[i].hItem;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::ResetTableList
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::ResetTableList(HWND hWndTable, HWND hWndCol)
{
    HRESULT hr;
    CHAR			szBuffer[MAX_NAME_LEN];

    IRowset* pIRowset = NULL;
    IAccessor* pIAccessor = NULL;
    
    //get the data
    HROW*		rghRows = NULL;
    DBCOUNTITEM	i,cRowsObtained = 0;
    BOOL		bFound = FALSE;

    HACCESSOR hAccessor = DB_NULL_HACCESSOR;
    TREEINFO* rgTreeInfo = NULL;

    //Use the passed in Session interface
    CTable*	pCFromTable = m_pCTableCopy->m_pCFromTable;
    CDataSource* pCFromDataSource = pCFromTable->m_pCDataSource;
    IOpenRowset* pIOpenRowset = pCFromTable->m_pCDataSource->m_pIOpenRowset;
    IDBSchemaRowset* pIDBSchemaRowset = pCFromTable->m_pCDataSource->m_pIDBSchemaRowset;

    //Delete all previous items
    SendMessage(hWndTable, TVM_DELETEITEM, (WPARAM)0, (LPARAM)TVI_ROOT);
    SendMessage(hWndCol, LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0);

    // Bind the user and table name for the list
    const static ULONG cBindings = 4;
    const static DBBINDING rgBindings[cBindings] = 
    {
        //TABLE_CATALOG
        1,	 			
        offsetof(TABLEINFO, wszCatalogName),	// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        MAX_NAME_LEN,						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_WSTR, 						// data type indicator
        0,									// precision
        0, 									// scale

        //TABLE_SCHEMA
        2,	 			
        offsetof(TABLEINFO, wszSchemaName),	// offset of value in consumers buffer
        0,									// offset of length
        0,									// offset of status
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value is bound only										
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// 
        MAX_NAME_LEN,						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_WSTR, 						// data type indicator
        0,									// precision
        0, 									// scale

        //TABLE_NAME
        3,	 								// ordinal
        offsetof(TABLEINFO, wszTableName),		// offset of value in consumers buffer
        0,									// offset of length in consumers buffer
        0,									// offset of status in consumers buffer
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value only
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// input param
        MAX_NAME_LEN,						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_WSTR, 						// data type indicator
        0,									// precision
        0, 									// scale

        //TABLE_TYPE
        4,	 								// ordinal
        offsetof(TABLEINFO, wszType),	// offset of value in consumers buffer
        0,									// offset of length in consumers buffer
        0,									// offset of status in consumers buffer
        NULL,								// reserved
        NULL,								// for ole object
        NULL,								// reserved
        DBPART_VALUE,						// specifies Value only
        DBMEMOWNER_CLIENTOWNED,				// memory is client owned
        DBPARAMIO_NOTPARAM,					// input param
        MAX_NAME_LEN,						// size in bytes of the value part in the consumers buffer
        0, 									// reserved
        DBTYPE_WSTR, 						// data type indicator
        0,									// precision
        0, 									// scale
    };

    //Reset TableInfo
    m_cTables = 0;
    SAFE_FREE(m_rgTableInfo);

    //Provider doesn't have to support IDBSchemaRowset
    if(pIDBSchemaRowset)
    {
        //GetRowset
        //DBSCHEMA_TABLES is required a SCHEMA (if IDBSchemaRowset is supported)
        XTESTC(hr = pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_TABLES, 0, NULL, IID_IRowset, 0, NULL,(IUnknown **)&pIRowset));

        //Create the Accessor
        XTESTC(hr = pIRowset->QueryInterface(IID_IAccessor, (void **)&pIAccessor));
        XTESTC(hr = pIAccessor->CreateAccessor(DBACCESSOR_ROWDATA, cBindings, rgBindings, 0, &hAccessor, NULL));

        //Grab all the rows
        while(TRUE)
        {
            XTESTC(hr = pIRowset->GetNextRows(NULL, 0, MAX_BLOCK_SIZE, &cRowsObtained, &rghRows));
            
            //ENDOFROWSET
            if(cRowsObtained==0)
                break;
        
            //Realloc Table struct for Table
            SAFE_REALLOC(m_rgTableInfo, TABLEINFO, m_cTables + cRowsObtained);
            memset(&m_rgTableInfo[m_cTables], 0, cRowsObtained*sizeof(TABLEINFO));

            //Loop over the rows retrived
            for(i=0; i<cRowsObtained; i++) 
            {
                //Get the Data
                XTESTC(hr = pIRowset->GetData(rghRows[i], hAccessor, &m_rgTableInfo[m_cTables]));
                m_cTables++;
            }

            //Release the rows obtained
            XTESTC(hr = pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL));
            SAFE_FREE(rghRows);
        }
    }

    //Provider doesn't have to support IDBSchemaRowset
    if(pIDBSchemaRowset == NULL)
    {
        m_cTables = 1;
        SAFE_ALLOC(m_rgTableInfo, TABLEINFO, 1);
        memset(m_rgTableInfo, 0, sizeof(TABLEINFO));

        if(pCFromTable->m_TableInfo.wszTableName[0])
        {
            //Just Display the TableName in the EditBox
            StringCchCopyW(m_rgTableInfo[0].wszTableName, 
                           sizeof(m_rgTableInfo[0].wszTableName)/sizeof(WCHAR),
                           pCFromTable->m_TableInfo.wszTableName);
        }
        else
        {
            //Just Display "Enter - <TableTerm>"
            StringCchPrintfW(m_rgTableInfo[0].wszTableName, 
                             sizeof(m_rgTableInfo[0].wszTableName)/sizeof(WCHAR),
                             L"Enter - %s",
                             pCFromDataSource->m_pwszTableTerm);
        }
    }
    

    //Create rgTreeInfo array
    SAFE_ALLOC(rgTreeInfo, TREEINFO, m_cTables);
    memset(rgTreeInfo, 0, m_cTables*sizeof(TREEINFO));

    //Create Tree Nodes for the TreeView control
    CreateTableNode(rgTreeInfo, offsetof(TABLEINFO, wszCatalogName), -1, ICON_CATALOG, ICON_CATALOG); 
    CreateTableNode(rgTreeInfo, offsetof(TABLEINFO, wszSchemaName),	 -1, ICON_SCHEMA, ICON_SCHEMA); 
    CreateTableNode(rgTreeInfo, offsetof(TABLEINFO, wszType),		 -1, ICON_TYPE, ICON_TYPE); 

    //Now display all the tables
    for(i=0; i<m_cTables; i++)
    {
        //Add it to the TreeView
        ConvertToMBCS(m_rgTableInfo[i].wszTableName, szBuffer, MAX_NAME_LEN);

        if(wcscmp(m_rgTableInfo[i].wszType, L"TABLE")==0)
            rgTreeInfo[i].hItem = TV_InsertItem(hWndTable, rgTreeInfo[i].hParent, TVI_SORT, szBuffer, (LONG)i, ICON_TABLE, ICON_TABLE);
        else if(wcscmp(m_rgTableInfo[i].wszType, L"VIEW")==0)
            rgTreeInfo[i].hItem = TV_InsertItem(hWndTable, rgTreeInfo[i].hParent, TVI_SORT, szBuffer, (LONG)i, ICON_VIEW, ICON_VIEW);
        else if(wcscmp(m_rgTableInfo[i].wszType, L"SYSTEM TABLE")==0)
            rgTreeInfo[i].hItem = TV_InsertItem(hWndTable, rgTreeInfo[i].hParent, TVI_SORT, szBuffer, (LONG)i, ICON_SYSTABLE, ICON_SYSTABLE);
        else
            rgTreeInfo[i].hItem = TV_InsertItem(hWndTable, rgTreeInfo[i].hParent, TVI_SORT, szBuffer, (LONG)i, ICON_SYNONYM, ICON_SYNONYM);
    }

    // If there was a previous selection, select it again on Back
    bFound = FALSE;
    if(pCFromTable->m_TableInfo.wszTableName[0])
    {
        //Find the previously selected TableName
        //With TreeView controls there is no "Find" method as in ListView controls!
        //So we have to simulate our own find, by seraching for the TableName
        for(i=0; i<m_cTables; i++)
        {
            if(memcmp(&m_rgTableInfo[i], &pCFromTable->m_TableInfo, sizeof(TABLEINFO))==0)
            {
                bFound = TRUE;
                //Select Table in the TreeView control
                SendMessage(hWndTable, TVM_SELECTITEM, (WPARAM)TVGN_CARET, (LPARAM)rgTreeInfo[i].hItem);
                ResetColInfo(GetDlgItem(m_hWnd, IDL_COLUMNS));
                break;
            }
        }
    }

    if(!bFound)
    {
        //Reset TableInfo
        memset(&pCFromTable->m_TableInfo, 0, sizeof(TABLEINFO));
    
        //Otheriwse if there was no previous selection, default to auto expand
        //the "TABLE" type, or the first type in the tree
        bFound = FALSE;
        for(i=0; i<m_cTables; i++)
        {
            if(wcscmp(m_rgTableInfo[i].wszType, L"TABLE")==0)
            {
                bFound = TRUE;
                SendMessage(hWndTable, TVM_ENSUREVISIBLE, (WPARAM)0, (LPARAM)rgTreeInfo[i].hItem);
                break;
            }
        }

        if(!bFound && m_cTables)
            SendMessage(hWndTable, TVM_ENSUREVISIBLE, (WPARAM)0, (LPARAM)rgTreeInfo[0].hItem);
    }


CLEANUP:
    if(hAccessor && pIAccessor)
        XTEST(pIAccessor->ReleaseAccessor(hAccessor,NULL));

    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIAccessor);

    SAFE_FREE(rgTreeInfo);
    SAFE_FREE(rghRows);
    return hr==S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::ResetColInfo
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::ResetColInfo(HWND hWndCol)
{
    HRESULT hr;
    CHAR    szBuffer[MAX_NAME_LEN];
    ULONG   i;
    LONG    lFoundColumn;

    CTable*	pCFromTable = m_pCTableCopy->m_pCFromTable;
    IOpenRowset* pIOpenRowset = pCFromTable->m_pCDataSource->m_pIOpenRowset;

    //Save the currently selected columns
    ULONG cSelColumns = pCFromTable->m_cColumns;
    COLDESC* rgSelColDesc = pCFromTable->m_rgColDesc;
    
    //Reset current Window Column ListView
    SendMessage(hWndCol, LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0L);
    
    // Get a list of columns based on the selected table
    pCFromTable->m_cColumns = 0;
    pCFromTable->m_rgColDesc = NULL;
    QTESTC(hr = pCFromTable->GetColInfo(IDR_PARAM_SETS));

    //Loop through all columns and update window
    for(i=0; i<pCFromTable->m_cColumns; i++)
    {	
        COLDESC* pColDesc = &pCFromTable->m_rgColDesc[i];

        //COLNAME (item)
        ConvertToMBCS(pColDesc->wszColName, szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndCol, i, COL_COLNAME, szBuffer, 0, pColDesc->dwFlags & (DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN) ? ((pColDesc->dwFlags & DBCOLUMNFLAGS_ISLONG) ? ICON_LONG : ICON_COLUMN) : ICON_READONLY);

        //COLTYPE (subitem)
        ConvertToMBCS(GetDBTypeName(pColDesc->wType), szBuffer, MAX_NAME_LEN);
        LV_InsertItem(hWndCol, i, COL_COLTYPE, szBuffer);

        //Ordinal (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pColDesc->iOrdinal);
        LV_InsertItem(hWndCol, i, COL_COLORDINAL, szBuffer);

        //ColumnSize (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pColDesc->ulColumnSize);
        LV_InsertItem(hWndCol, i, COL_COLSIZE, szBuffer);

        //Precision (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pColDesc->bPrecision);
        LV_InsertItem(hWndCol, i, COL_COLPREC, szBuffer);

        //Scale (SubItem)
        StringCchPrintfA(szBuffer, sizeof(szBuffer), "%d", pColDesc->bScale);
        LV_InsertItem(hWndCol, i, COL_COLSCALE, szBuffer);

        //FLAGS (SubItem)
        LV_InsertItem(hWndCol, i, COL_COLISFIXED, pColDesc->dwFlags & DBCOLUMNFLAGS_ISFIXEDLENGTH ? "TRUE" : "FALSE");
        LV_InsertItem(hWndCol, i, COL_COLISLONG, pColDesc->dwFlags & DBCOLUMNFLAGS_ISLONG ? "TRUE" : "FALSE");
        LV_InsertItem(hWndCol, i, COL_COLISNULLABLE, pColDesc->dwFlags & DBCOLUMNFLAGS_ISNULLABLE ? "TRUE" : "FALSE");
        LV_InsertItem(hWndCol, i, COL_COLWRITE, pColDesc->dwFlags & (DBCOLUMNFLAGS_WRITE | DBCOLUMNFLAGS_WRITEUNKNOWN) ? "TRUE" : "FALSE");
        LV_InsertItem(hWndCol, i, COL_COLISROWID, pColDesc->dwFlags & DBCOLUMNFLAGS_ISROWID ? "TRUE" : "FALSE");
        LV_InsertItem(hWndCol, i, COL_COLISROWVER, pColDesc->dwFlags & DBCOLUMNFLAGS_ISROWVER ? "TRUE" : "FALSE");

    }
    
    // If there is an existing columns list (only on Back or error), then
    // the user already has a list so use it.
    lFoundColumn = -1;
    for(i=0; i<cSelColumns; i++) 
    {
        //Find the Column Name in the Window List
        ConvertToMBCS(rgSelColDesc[i].wszColName, szBuffer, MAX_NAME_LEN);
        lFoundColumn = LV_FindItem(hWndCol, szBuffer, lFoundColumn);
            
        //Select the Column Name if found in the list, and bring into view
        if(lFoundColumn != LVM_ERR)
        {
            LV_SetItemState(hWndCol, lFoundColumn, COL_COLNAME, LVIS_SELECTED, LVIS_SELECTED);

            //Ensure that the first item is Visible
            if(i==0)
                SendMessage(hWndCol, LVM_ENSUREVISIBLE, (WPARAM)lFoundColumn, (LPARAM)FALSE);
        }	
    }
    
    //Otherwise, just select all as default
    if(cSelColumns == 0)
    {
        for(i=0; i<pCFromTable->m_cColumns; i++)
            LV_SetItemState(hWndCol, i, COL_COLNAME, LVIS_SELECTED, LVIS_SELECTED);
    }

CLEANUP:
    SAFE_FREE(rgSelColDesc);
    return hr==S_OK;
}


/////////////////////////////////////////////////////////////////////////////
// BOOL CS1Dialog::ChangeTableName
//
/////////////////////////////////////////////////////////////////////////////
BOOL CS1Dialog::ChangeTableName(LONG iIndex)
{
    CTable* pCFromTable = m_pCTableCopy->m_pCFromTable;
    CTable* pCToTable = m_pCTableCopy->m_pCToTable;
    
    //Index must fall with the m_rgTableInfo array range
    //Otherwise we have selected a "tree-folder" and need to free the column list
    if(iIndex < 0 || iIndex >= (LONG)m_cTables)
    {
        //Resest TableName
        memset(&pCFromTable->m_TableInfo, 0, sizeof(TABLEINFO));

        //Reset current Window Column ListView
        SendMessage(GetDlgItem(m_hWnd, IDL_COLUMNS), LVM_DELETEALLITEMS, (WPARAM)0, (LPARAM)0L);
        return FALSE;
    }

    //Only change the TableName if not equal to "Enter - <TableTerm>"
    if(wcsstr(m_rgTableInfo[iIndex].wszTableName, L"Enter - "))
        return FALSE;

    //TableInfo
    memcpy(&pCFromTable->m_TableInfo, &m_rgTableInfo[iIndex], sizeof(TABLEINFO));
    
    //QualifiedTableName syntax
    // #1.  TableName
    // #2.  Owner.TableName (always a ".")
    // #3.  Catalog[CatalogSeperator]TableName
    // #4.  Catalog[CatalogSeperator]Owner.TableName
    if(pCFromTable->m_TableInfo.wszSchemaName[0])
    {
        StringCchPrintfW(pCFromTable->m_wszQualTableName, 
                         sizeof(pCFromTable->m_wszQualTableName)/sizeof(WCHAR),
                         L"%s.%s", 
                         pCFromTable->m_TableInfo.wszSchemaName, 
                         pCFromTable->m_TableInfo.wszTableName);
    }
    else
    {
        StringCchCopyW(pCFromTable->m_wszQualTableName, 
                       sizeof(pCFromTable->m_wszQualTableName)/sizeof(WCHAR),
                       pCFromTable->m_TableInfo.wszTableName);
    }

    //Free the current columns list, since the new 
    //table will have diffent columns
    pCFromTable->m_cColumns = 0;
    SAFE_FREE(pCFromTable->m_rgColDesc);
    
    //Reset the column list, since we have a new table
    ResetColInfo(GetDlgItem(m_hWnd, IDL_COLUMNS));

    //Reset the index list, since we have a new table
    pCFromTable->m_cIndexes = 0;
    SAFE_FREE(pCFromTable->m_rgIndexInfo);

    //Reset the "To" table, since we have a new table
    memset(&pCToTable->m_TableInfo, 0, sizeof(TABLEINFO));
    return TRUE;
}
