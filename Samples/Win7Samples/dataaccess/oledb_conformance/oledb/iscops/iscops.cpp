//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module IScOps.CPP | Template source file for all test modules.
//

#include "modstandard.hpp"
#include "IScOps.h"
#include "Extralib.h"
#include <math.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x70317e9, 0x5edc, 0x11d2, { 0xb0, 0x2b, 0x0, 0xc0, 0x4f, 0xc2, 0x27, 0x93 } };
DECLARE_MODULE_NAME("IScopedOperations");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IScopedOperations test");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

// macros
#define FILL_PROP_SET(RGPROPSET_EL, NPROP, RGPROP, PROP_GUID)		\
	RGPROPSET_EL.cProperties		= NPROP;						\
	RGPROPSET_EL.rgProperties		= RGPROP;						\
	RGPROPSET_EL.guidPropertySet	= PROP_GUID;					\
	if (NULL != RGPROP)												\
		memset(RGPROP, 0, NPROP*sizeof(DBPROP));

#define FILL_PROP(RGPROP_EL, PROPID, VAR_TYPE, VAR_MACRO, VAR_VALUE, OPTION)		\
	memset(&RGPROP_EL, 0, sizeof(DBPROP));											\
	RGPROP_EL.dwPropertyID			= PROPID;										\
	RGPROP_EL.vValue.vt				= VAR_TYPE;										\
	VAR_MACRO(&RGPROP_EL.vValue)	= VAR_VALUE;									\
	RGPROP_EL.dwOptions				= OPTION;

const CLSID		CLSID_ConfProv		= {0xb2a233c1, 0x5b20, 0x11d0, {0x84, 0x18, 0x0, 0xaa, 0x00, 0x3f, 0xd, 0xd4}};

//global variable
IBindResource	*g_pIBindResource	= NULL;
BOOL			g_fGenURLSuffix		= FALSE;
BOOL			g_fCreatedTable		= FALSE;
BOOL			g_fResetIniFile		= FALSE;
CTable			*g_pConfProvTable2	= NULL;
CTree			*g_pConfProvTree2	= NULL;

// helper function declarations
WINOLEAPI CoCreateInstanceEx(REFCLSID, IUnknown *, DWORD, COSERVERINFO *, ULONG, MULTI_QI *);
HRESULT CreateBinder(WCHAR*, REFIID, IUnknown**);
HRESULT CreateBinder(CLSID clsid, REFIID riid, IUnknown** ppIUnknown);
unsigned WINAPI ThreadProc(LPVOID lpvThreadParam);
//--------------------------------------------------------------------------
//
// Check whether the property sets asked for rowset was preserved
// all the data should be inside, the only thing that is allowed to be modified <nl>
// is the property status
//--------------------------------------------------------------------------
BOOL IsPropSetPreserved
(
	DBPROPSET	*rgInPropertySets,	// the array passed to function call
	DBPROPSET	*rgOutPropertySets,	// the aray returned by function call
	ULONG		cPropertySets			// the size of the arrays
);

// for thread execution
typedef enum _tagMethod{
	Method_CopyAndCheck,
	Method_MoveAndCheck,
} Method;

class TCIScopedOperations;

typedef struct inparam{
	TCIScopedOperations	*pObject;		// the this pointer
	Method				enMethodCalled;	// the method being called
	LPVOID				pParam;		// pointer to method parameter
} CInParam;

// parameters to call TCIScopedOperations::CopyAndCheck
typedef struct _tagCpACParam{
			HRESULT				hRet;					// [out] the value returned by method			
			IScopedOperations	*pIScopedOperations;	// [in] interface to use
			ULONG				cRows;					// [in] number of copy ops	
			WCHAR				**rgpwszSourceURLs;		// [in] source URLs
			WCHAR				**rgpwszDestURLs;		// [in] destination URLs
			DWORD				dwCopyFlags;			// [in] copy flags
			IAuthenticate		*pAuthenticate;			// [in] authenticate interface
			DBSTATUS			*rgdwStatus;			// [out] filled on output
			WCHAR				**rgpwszNewURLs;		// [out] filled on output
			WCHAR				**ppStringsBuffer;		// [out] buffer for new names
			HRESULT				hrExpected;				// [in] the expected result
			BOOL				fValid;					// [in] if the result is to be validated
			HRESULT				*phActualRes;			// [ou] the actual result
} CpACParam;



// truncates the log base 2 of the number
ULONG log2(ULONG x)
{
	ULONG	ulLog = 0;

	for (ulLog = 0; 1 < x; x/=2, ulLog++);
	return ulLog;
} // log2




class CIScOpsTree : public CTree{
	public:
	
	CIScOpsTree
		(
			IUnknown*	pSessionIUnknown,		// [IN] SessionInterface
			WCHAR*		pwszModuleName=NULL,	// [IN] Tree name, optional (Default=NULL)
			ENULL		eNull=USENULLS			// [IN] Should nulls be used (Default=USENULLS)
		) : CTree(pSessionIUnknown, pwszModuleName, eNull) {;} 
	
	virtual ~CIScOpsTree() {;}

	HRESULT				PickALeaf(
		WCHAR			**ppwszURL		//[out] the URL of the chosen row
	);

	HRESULT				PickACollection(
		WCHAR **ppwszURL				//[out] the URL of the chosen row
	);

	HRESULT				PickASingleton(
		WCHAR			**ppwszURL		//[out] the URL of the chosen row
	);

	HRESULT				PickASecondNode(
		WCHAR			*pwszExclURL,	//[in]  exclude URLs in this tree
		NODE_CONSTRAINT	NodeConstr,		//[in]  node constraint
		WCHAR			**ppwszURL		//[out] the URL of the chosen row
	);

	HRESULT				PickASubtree(
		WCHAR			**ppwszURL		//[out] the URL of the chosen row
	);

	HRESULT				PickANode(
		WCHAR			**ppwszURL		//[out] the URL of the chosen row
	);

	HRESULT				PickANode(
		NODE_CONSTRAINT	NodeConstr,		//[in]  node constraint
		WCHAR			**ppwszURL		//[out] the URL of the chosen row
	);

	HRESULT				Pick2Nodes(
		NODE_CONSTRAINT	NodeConstr1,	//[in]  constraint for the first node 
		WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
		NODE_CONSTRAINT	NodeConstr2,	//[in]  constraint for the second node 
		WCHAR			**ppwszURL2,	//[out] the URL of the chosen row
		PAIR_CONSTRAINT	PairConstr		//[in]  pair constraint (none, overlapped, diff)
	);

	HRESULT				Pick2Nodes(
		WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
		WCHAR			**ppwszURL2		//[out] the URL of the chosen row
	);

	HRESULT				Pick2DifLeaves(
		WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
		WCHAR			**ppwszURL2		//[out] the URL of the chosen row
	);

	HRESULT				Pick2DifNodes(
		WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
		WCHAR			**ppwszURL2		//[out] the URL of the chosen row
	);

	HRESULT				Pick2OverlappingNodes(
		WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
		WCHAR			**ppwszURL2		//[out] the URL of the chosen row
	);

	HRESULT				Pick2DifCollections(
		WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
		WCHAR			**ppwszURL2		//[out] the URL of the chosen row
	);

	HRESULT				TrimLeafToSubtree(
		WCHAR			*pwszURL,				//[in/out] the URL of the chosen row
		WCHAR			*pwszAvoidURL = NULL	//[in/out] the URL of the chosen row
	);

	HRESULT				GetInexistentURL(
		WCHAR				**ppwszInexistentRow,	// [out] URL of an inexistent row
		WCHAR				*pwszExclURL = NULL		// [in] path to exclude when chosing the URL
	);

	HRESULT				GetURLWithInexistentFather(
			WCHAR		**ppwszInexistentRow,		// [out] URL of an inexistent row
			WCHAR		*pwszExclURL = NULL			// [in] path to exclude when chosing the URL
	);

	// calls IScopedOperations and verifies it
	virtual HRESULT		DeleteAndCheck(
		IScopedOperations	*pIScopedOperations,	// [in] interface to use
		DBCOUNTITEM			cRows,					// [in] number of copy ops	
		WCHAR				**rgpwszURLs,			// [in] source URLs
		DWORD				dwDeleteFlags,			// [in] copy flags
		DBSTATUS			*rgdwStatus,			// [out] filled on output
		BOOL				fCheckTree		= TRUE
		);

	virtual HRESULT		CopyAndCheck(
		IScopedOperations	*pIScopedOperations,	// [in] interface to use
		DBCOUNTITEM			cRows,					// [in] number of copy ops	
		WCHAR				**rgpwszSourceURLs,		// [in] source URLs
		WCHAR				**rgpwszDestURLs,		// [in] destination URLs
		DWORD				dwCopyFlags,			// [in] copy flags
		IAuthenticate		*pIAuthenticate,		// [in] authentication interface
		DBSTATUS			*rgdwStatus,			// [out] filled on output
		WCHAR				**rgpwszNewURLs,		// [in/out] new URLs
		WCHAR				**ppStringsBufferURLs	// [out] buffer for URL strings
	);

	virtual HRESULT		MoveAndCheck(
		IScopedOperations	*pIScopedOperations,	// [in] interface to use
		DBCOUNTITEM			cRows,					// [in] number of copy ops	
		WCHAR				**rgpwszSourceURLs,		// [in] source URLs
		WCHAR				**rgpwszDestURLs,		// [in] destination URLs
		DWORD				dwMoveFlags,			// [in] copy flags
		IAuthenticate		*pIAuthenticate,		// [in] authentication interface
		DBSTATUS			*rgdwStatus,			// [out] filled on output
		WCHAR				**rgpwszNewURLs,		// [in/out] new URLs
		WCHAR				**ppStringsBufferURLs	// [out] buffer for URL strings
	);

	// wrapper for calling IScopedOperations::OpenRowset and doing  basic general checking
	virtual HRESULT		OpenRowsetAndCheck(
		IScopedOperations	*pIScopedOperations,	// [in] interface to use
		IUnknown			*pUnkOuter,				// [in] controlling IUnknown of the rowset
		DBID				*pTableID,				// [in] URL of the row
		DBID				*pIndexID,				// [in] should be ignored
		REFIID				riid,					// [in] interface to be retrieved
		ULONG				cPropertySets,			// [in] number of elements in prop array
		DBPROPSET			*rgPropertySets,		// [in|out] property array
		IUnknown			**ppRowset				// [out] rowset interface
	);

	// wrapper for calling IScopedOperations::Bind and doing  basic general checking
	virtual HRESULT			BindAndCheck(
		IScopedOperations	*pIScopedOperations,	// [in] interface to use
		IUnknown			*pUnkOuter,				// [in] controlling IUnknown
		LPCOLESTR			pwszURL,				// [in] object to be bound
		DBBINDURLFLAG		dwBindFlags,			// [in] flags to be used for binding
		REFGUID				rguid,					// [in] indicates the type of the object being requested
		REFIID				riid,					// [in] requested interface
		IAuthenticate		*pAuthenticate,			// [in] pointer to IAuthenticate interface to be used
		DBIMPLICITSESSION	*pImplSession,			// [in] implicit session	
		DBBINDURLSTATUS		*pdwBindStatus,			// [out] bind status
		IUnknown			**ppUnk					// [out] interface on the bound object
	);
};


CIScOpsTree		*g_pCTree			= NULL;


BOOL PlayWithSchemaRowset()
{
	TBEGIN
	IRow				*pIRow = NULL;
	IRowSchemaChange	*pIRowSC	= NULL;
	const ULONG			cMaxCols	= 10;
	ULONG				cCols		= 0;
	DBCOLUMNINFO		rgNewColumnInfo[cMaxCols];
	DBCOLUMNACCESS		rgColumns[cMaxCols];
	WCHAR				wszBuffer[2000]=L"";
	ULONG				iOrdinal	= 200;
	DBCOLUMNFLAGS		dwFlags		= 0;
	ULONG				ulColSize	= 0;
	DBTYPE				wType		= DBTYPE_I4;
	BYTE				bPrecision	= 0;
	BYTE				bScale		= 0;
	DBSTATUS			dwStatus	= 0;
	ULONG				ulRowNum	= 15;
	CCol				col;
	HRESULT				hr;
	VARIANT				vValue;
	WCHAR				wszURL[200]	= L"http://rosetest01/davfs/csipos10";

	hr = g_pIBindResource->Bind(NULL, wszURL, 
					DBBINDURLFLAG_READWRITE, DBGUID_ROW, 
					IID_IRow, NULL, NULL, NULL, 
					(IUnknown**)&pIRow);

	VerifyInterface(pIRow, IID_IRowSchemaChange,
		ROW_INTERFACE, (IUnknown**)&pIRowSC);

	memset(rgNewColumnInfo, 0, cMaxCols*sizeof(DBCOLUMNINFO));
	memset(rgColumns, 0, cMaxCols*sizeof(DBCOLUMNACCESS));

	// this fields should be ignored
	rgNewColumnInfo[cCols].pwszName		= wcsDuplicate(wszBuffer);
	rgNewColumnInfo[cCols].iOrdinal		= iOrdinal;
	rgNewColumnInfo[cCols].pTypeInfo	= (ITypeInfo*)0x123456;
	
	rgNewColumnInfo[cCols].dwFlags		= dwFlags;
	rgNewColumnInfo[cCols].ulColumnSize	= ulColSize;
	rgNewColumnInfo[cCols].wType		= wType;
	rgNewColumnInfo[cCols].bPrecision	= bPrecision;
	rgNewColumnInfo[cCols].bScale		= bScale;
	rgNewColumnInfo[cCols].columnid.eKind			= DBKIND_NAME;
	rgNewColumnInfo[cCols].columnid.uName.pwszName	= wcsDuplicate(wszBuffer);

	// this should also be ignored
	rgColumns[cCols].columnid.eKind				= DBKIND_NAME;
	rgColumns[cCols].columnid.uName.pwszName	= (WCHAR*)0x123456;

	rgColumns[cCols].wType		= wType;
	rgColumns[cCols].bPrecision	= bPrecision;
	rgColumns[cCols].bScale		= bScale;
	rgColumns[cCols].dwStatus	= dwStatus;

	//set the value
	VariantInit(&vValue);
	vValue.vt = VT_BSTR;
	V_BSTR(&vValue) = SysAllocString(wszBuffer);
	VariantChangeType(&vValue, &vValue, 0, wType);

	rgColumns[cCols].pData = (void*)&(vValue.bVal);
	cCols++;

	hr = pIRowSC->AddColumns(cCols, rgNewColumnInfo, rgColumns);

	SAFE_RELEASE(pIRowSC);
	SAFE_RELEASE(pIRow);
	TRETURN
} //PlayWithSchemaRowset


//--------------------------------------------------------------------
// @func Reinitialize the Conformance Provider by ignoring the Ini file
//  and constructing the tree - added on 03/30/2001 
//

BOOL ReInitializeConfProv(CThisTestModule* pThisTestModule)
{
	BOOL			fSuccess = FALSE;
	WCHAR*			pwszRootURL = NULL;
	WCHAR*			pwszNewURL = NULL;
	WCHAR*			pwszCmdURL = NULL;
	WCHAR*			pwszRowURL = NULL;
	WCHAR*			pwszRowQuery = NULL;

	CList<WCHAR *,WCHAR *>	NativeTypesList;
	CList<DBTYPE,DBTYPE>	ProviderTypesList;
	CCol					col;
	DBCOLUMNDESC			*rgColumnDesc			= NULL;
	DBORDINAL				cColumnDesc				= 0;
	DBORDINAL				ulIndxCol				= 0;
	BOOL					fTableExist				= FALSE;
	
	CList <CCol, CCol&>		colList;
	POSITION				pos;
	DBORDINAL				cIter=0;
	DBORDINAL				ulParentOrdinal;
	DBORDINAL				rgOrdinals[2];	// ConfProv reserves two columns for its own use


	pwszRootURL = (WCHAR *)PROVIDER_ALLOC((wcslen(L"confprov://dso/session/")+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszRootURL != NULL);
	wcscpy(pwszRootURL, L"confprov://dso/session/");
	GetModInfo()->SetRootURL(pwszRootURL);

	// Create a table.
	if (g_pConfProvTable2)
	{
		g_pConfProvTable2->DropTable();
		SAFE_DELETE(g_pConfProvTable2);
	}
	g_pConfProvTable2 = new CTable(pThisTestModule->m_pIUnknown2);
	
	g_pConfProvTable2->CreateColInfo(NativeTypesList, ProviderTypesList);	
	g_pConfProvTable2->DuplicateColList(colList);

	pos = colList.GetHeadPosition();
	TESTC(NULL != pos)
	
	cColumnDesc = 2;
	for (; pos; )
	{
		POSITION	oldPos = pos;

		col = colList.GetNext(pos);
		if (!col.GetNullable())
			colList.RemoveAt(oldPos);
		else
		{
			col.SetColNum(++cColumnDesc);
			colList.SetAt(oldPos, col);
		}
	}

	TESTC(g_pConfProvTable2->GetColWithAttr(COL_COND_AUTOINC, &ulIndxCol)); 
	// duplicate the first column - use one for index and one for values
	TESTC_(g_pConfProvTable2->GetColInfo(ulIndxCol, col), S_OK);
	col.SetColName(L"RESOURCE_PARSENAME");
	col.SetNullable(FALSE); 
	col.SetColNum(1);
	colList.AddHead(col);

	// Find a candidate for the RESOURCE_PARENTNAME columns
	for(cIter=1; cIter <= g_pConfProvTable2->CountColumnsOnTable(); cIter++)
	{
		TESTC_(g_pConfProvTable2->GetColInfo(cIter, col), S_OK);
			
		if (col.GetIsLong() == FALSE && col.GetIsFixedLength() == FALSE &&
			(col.GetProviderType() == DBTYPE_WSTR ||
			 col.GetProviderType() == DBTYPE_BSTR ||
			 col.GetProviderType() == DBTYPE_STR ))
		break;
	}
	
	// Did we find a candidate?
	TESTC(cIter < g_pConfProvTable2->CountColumnsOnTable());
	ulParentOrdinal = col.GetColNum();

	col.SetColName(L"RESOURCE_PARENTNAME");
	col.SetIsFixedLength(FALSE);
	col.SetColumnSize(200);
	col.SetColNum(2);
	colList.AddHead(col);

	g_pConfProvTable2->SetColList(colList);

	
	g_pConfProvTable2->SetBuildColumnDesc(FALSE);	// do not create ColList again
	
	cColumnDesc = g_pConfProvTable2->CountColumnsOnTable();
	g_pConfProvTable2->BuildColumnDescs(&rgColumnDesc);
	
	// make sure the first column is not autoincrementable 
	FreeProperties(&rgColumnDesc[0].cPropertySets, &rgColumnDesc[0].rgPropertySets);
	SAFE_FREE(rgColumnDesc[0].pwszTypeName);
	
	// make sure the parent column doesn't specify a type name
	SAFE_FREE(rgColumnDesc[ulParentOrdinal-1].pwszTypeName);

	g_pConfProvTable2->SetColumnDesc(rgColumnDesc, cColumnDesc);
//	TESTC_(g_pConfProvTable->CreateTable(0, cColumnDesc), S_OK);
	TESTC_(g_pConfProvTable2->CreateTable(0, 0), S_OK); // avoid creating a rowset on the last col

	// create a unique index on the two special columns
	rgOrdinals[0] = 1;
	rgOrdinals[1] = 2;
	TESTC_(g_pConfProvTable2->CreateIndex(rgOrdinals,2,UNIQUE), S_OK);

	// get the name of the created table
	// and alter the ROOT_URL.
	pwszNewURL = (WCHAR *)PROVIDER_ALLOC((wcslen(pwszRootURL)+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszNewURL != NULL);
	wcscpy(pwszNewURL, pwszRootURL);
	wcscat(pwszNewURL, L"/");
	wcscat(pwszNewURL, g_pConfProvTable2->GetTableName());

	//CreateTree with one node.
	g_pConfProvTree2 = new CTree(pThisTestModule->m_pIUnknown2);
	g_pConfProvTree2->CreateTree(pwszNewURL, 1, 2);

	pwszRootURL = g_pConfProvTree2->GetRootURL();
	TESTC(pwszRootURL && wcslen(pwszRootURL)>1)

	pwszCmdURL = (WCHAR *)PROVIDER_ALLOC((wcslen(L"confprov://dso/session/")+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszCmdURL != NULL);
	wcscpy(pwszCmdURL, L"confprov://dso/session/");
	wcscat(pwszCmdURL, L"select * from ");
	wcscat(pwszCmdURL, g_pConfProvTable2->GetTableName());

	pwszRowURL = (WCHAR *)PROVIDER_ALLOC((wcslen(pwszRootURL)+MAXBUFLEN)*sizeof(WCHAR));
	TESTC(pwszRowURL != NULL);
	wcscpy(pwszRowURL, pwszRootURL);
	wcscat(pwszRowURL, L"/0");

	GetModInfo()->SetRootURL(pwszRowURL);

	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(DATASOURCE_INTERFACE, pwszRootURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(SESSION_INTERFACE, pwszRootURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(ROWSET_INTERFACE, pwszNewURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(ROW_INTERFACE, pwszRowURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(STREAM_INTERFACE, pwszRootURL));
	TESTC(GetModInfo()->GetParseObject()->OverwriteURL(COMMAND_INTERFACE, pwszCmdURL));

	// Override the default Row Scoped Command Query
	pwszRowQuery = (WCHAR *)PROVIDER_ALLOC((wcslen(g_pConfProvTable2->GetTableName())+wcslen(wszSELECT_ALLFROMTBL)+1)*sizeof(WCHAR));
	TESTC(pwszRowQuery != NULL);
	swprintf(pwszRowQuery, wszSELECT_ALLFROMTBL, g_pConfProvTable2->GetTableName());
	
	GetModInfo()->SetRowScopedQuery(pwszRowQuery);

	fSuccess = TRUE;

CLEANUP:
	PROVIDER_FREE(pwszRowQuery);	
	PROVIDER_FREE(pwszNewURL);
	PROVIDER_FREE(pwszCmdURL);
	PROVIDER_FREE(pwszRowURL);

	return fSuccess;	
}

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	TBEGIN
	HRESULT					hr						= E_FAIL;
	DBBINDURLSTATUS			dwBindStatus			= 0;
	IScopedOperations		*pIScopedOperations		= NULL;
	IRow					*pIRow					= NULL;
	VARIANT					vVal;
	WCHAR					*pwszRootBase			= NULL;
	ICreateRow				*pICreateRow			= NULL;
	BOOL					fSupportedScope			= FALSE;
	BOOL					fISCO					= FALSE;
	IDBProperties			*pIDBProperties			= NULL;
	ULONG_PTR				ulOleObj				= 0; 
	BOOL					fConfProv				= FALSE;
	
	if (!ModuleCreateDBSession(pThisTestModule))
	{
		odtLog << L"Fail to initialize\n";
		return FALSE;
	}

	
	
	//Check if provider supports direct binding. If the provider doesn't support 
	//direct binding then we skip all test cases. As per the OLE DB spec if the provider sets
	//DBPROPVAL_OO_DIRECTBIND value of the DBPROP_OLE_OBJECTS, then the consumer
	//can assume that direct binding is supported.
	TESTC_PROVIDER(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		(IUnknown*)pThisTestModule->m_pIUnknown, &ulOleObj) &&
		((ulOleObj & DBPROPVAL_OO_DIRECTBIND) == DBPROPVAL_OO_DIRECTBIND));

	if(CLSID_ConfProv == GetModInfo()->GetProviderCLSID()) 
	{
		fConfProv = TRUE;
	}

	// Added to solve the problem of IscopedOperations not being able to run 
	// with an ini file. Hence we will be using only the Provider string to connect to the
	// data source and then ignore the rest of the sections of the ini file
	// This test doesn't support using the ini file, hence if ini file is used then construct 
	// your own urls instead of using the one's in the ini file. This will work only for
	// ini file used against confprov.

	if(GetModInfo()->GetFileName())
	{	odtLog << L"WARNING: Test does not support using ini file. \n";
		if(fConfProv)
		{
			odtLog << L" Resetting to ignore ini file. \n";
			odtLog << L" This test will construct internally the table and tree based on the ROOT_URL : confprov://dso/session . \n";
			GetModInfo()->ResetIniFile();
			g_fResetIniFile = TRUE;
			ReInitializeConfProv(pThisTestModule);
		}
		else
		{
			odtLog << L"Skipping all test cases.\n";		
			TESTB = TEST_SKIPPED;
			goto CLEANUP;
		}
	}

	if (!GetModInfo()->GetRootURL() && !g_fResetIniFile)
	{
		odtLog << L"Please provide a ROOT_URL keyword in the init string\n";
		TESTB = FALSE;
		goto CLEANUP;
	}

	TESTC(NULL != (g_pIBindResource = GetModInfo()->GetRootBinder()));

	//PlayWithSchemaRowset();

	// create the CTree object
	g_pCTree = new CIScOpsTree(pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a row where to copy the witnes tree
	TESTC(VerifyInterface(g_pIBindResource, IID_ICreateRow,
		BINDER_INTERFACE, (IUnknown**)&pICreateRow));
	TESTC(g_pCTree->MakeSuffix(c_ulTestRow, GetModInfo()->GetRootURL(), &pwszRootBase));
	// try to create a new row
	hr = pICreateRow->CreateRow(NULL, pwszRootBase,
		DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_COLLECTION | DBBINDURLFLAG_OVERWRITE,
		DBGUID_ROW, IID_IRow, NULL, NULL, &dwBindStatus, &g_pwszRootRow,
		(IUnknown**)&pIRow);
	TESTC(S_OK != hr || NULL != pIRow);

	SAFE_RELEASE(pIRow);
	SAFE_FREE(pwszRootBase);

	if (S_OK == hr)
	{
		GetModInfo()->SetRootURL(g_pwszRootRow);
		SAFE_FREE(g_pwszRootRow);

		// create the other 2 children nodes of the root
		TESTC(g_pCTree->MakeSuffix(c_ulRootRow0, GetModInfo()->GetRootURL(), &pwszRootBase));
		TESTC_(hr = pICreateRow->CreateRow(NULL, pwszRootBase,
			DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_COLLECTION,
			DBGUID_ROW, IID_IRow, NULL, NULL, &dwBindStatus, &g_pwszRootRow0,
			(IUnknown**)&pIRow), S_OK);
		SAFE_RELEASE(pIRow);
		SAFE_FREE(pwszRootBase);

		TESTC(g_pCTree->MakeSuffix(c_ulRootRow, GetModInfo()->GetRootURL(), &pwszRootBase));
		TESTC_(hr = pICreateRow->CreateRow(NULL, pwszRootBase,
			DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_COLLECTION | DBBINDURLFLAG_OVERWRITE,
			DBGUID_ROW, IID_IRow, NULL, NULL, &dwBindStatus, &g_pwszRootRow,
			(IUnknown**)&pIRow), S_OK);
		SAFE_RELEASE(pIRow);
		SAFE_FREE(pwszRootBase);
	}
	else
	{
		g_pwszRootRow0 = wcsDuplicate(GetModInfo()->GetRootURL());
	}

	// create the tree
	TESTC_(g_pCTree->CreateTree(g_pwszRootRow0, log2(cMaxRows), 2), S_OK);
	SAFE_FREE(g_pwszRootRow0);
	g_pwszRootRow0 = wcsDuplicate(g_pCTree->GetRootURL());

	// check whether IScopedOperation is supported (bind to the root node)
	TESTC_(hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0,
		DBBINDURLFLAG_READ, //| DBBINDURLFLAG_COLLECTION | DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_OPENIFEXISTS,
		DBGUID_ROW, IID_IRow, NULL, NULL, &dwBindStatus,
		(IUnknown**)&pIRow), S_OK);
	TESTC(NULL != pIRow);

	fISCO = VerifyInterface(pIRow, IID_IScopedOperations,
		ROW_INTERFACE, (IUnknown**)&pIScopedOperations);

	// check DBPROPVAL_OO_SCOPED
	TESTC_(hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0,
		DBBINDURLFLAG_READ, //| DBBINDURLFLAG_COLLECTION | DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_OPENIFEXISTS,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, &dwBindStatus,
		(IUnknown**)&pIDBProperties), S_OK);
	TESTC(NULL != pIDBProperties);

	VariantInit(&vVal);
	fSupportedScope = GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, pIDBProperties, &vVal) 
		&&	VT_I4 == vVal.vt 
		&&	(V_I4(&vVal) & DBPROPVAL_OO_SCOPED);
	VariantClear(&vVal);

	// DBPROPVAL_OO_SCOPED flag is set in DBPROP_OLEOBJECTS if and only if
	// ISCopedOperations is supported
	TESTC(fISCO || !fSupportedScope);
	
	// find out the value of GBPROP_GENERATEURL
	// get the value of the property
	g_fGenURLSuffix = FALSE;
	if (GetProperty(DBPROP_GENERATEURL, DBPROPSET_DATASOURCEINFO, 
					pIDBProperties, &vVal)
		&&	VT_I4 == vVal.vt
		&&	(	DBPROPVAL_GU_NOTSUPPORTED == vVal.lVal
			||	DBPROPVAL_GU_SUFFIX == vVal.lVal))
	g_fGenURLSuffix = DBPROPVAL_GU_SUFFIX == vVal.lVal;

CLEANUP:
	SAFE_FREE(pwszRootBase);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pICreateRow);
	SAFE_RELEASE(pIDBProperties);
	TRETURN
}	
  
//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	WCHAR				*rgpwszURLs[1];
	IScopedOperations	*pIScopedOperations = NULL;
	HRESULT				hr;
	DWORD				dwStatus;

	// delete the witness tree
	// !?! this should only happen for the tree we created, though
	if (g_pIBindResource && g_pwszRootRow)
	{
		// if a ntcop	ew node was created, then the root is one of the new created nodes
		// and it can be bound and deleted
		rgpwszURLs[0] = g_pCTree->GetRootURL();
		CHECK(hr = g_pIBindResource->Bind(NULL, GetModInfo()->GetRootURL(),
			DBBINDURLFLAG_READWRITE,
			DBGUID_ROW, IID_IScopedOperations, NULL, NULL, NULL,
			(IUnknown**)&pIScopedOperations), S_OK);
		COMPARE(NULL != pIScopedOperations, TRUE);
		if (pIScopedOperations)
			CHECK(hr = pIScopedOperations->Delete(1, (const WCHAR**)rgpwszURLs, 0, &dwStatus), S_OK);
	}

	// release the CIScOpsTree
	if (g_pCTree)
	{
		// make sure the witness is preserved
		g_pCTree->DestroyTree();
		delete g_pCTree;
	}

	if (g_pConfProvTable2)
	{
		g_pConfProvTable2->DropTable();
		SAFE_DELETE(g_pConfProvTable2);
	}

	if (g_pConfProvTree2)
	{
		g_pConfProvTree2->DestroyTree();
		SAFE_DELETE(g_pConfProvTree2);
	}

	SAFE_FREE(g_pwszRootRow0);
	SAFE_FREE(g_pwszRootRow);
	SAFE_RELEASE(pIScopedOperations);
	return ModuleReleaseDBSession(pThisTestModule);
}	




//*-----------------------------------------------------------------------
// @class base class for IScopedOperations testing
//
class TCIScopedOperations : public CSessionObject{ 
private:
protected:
		WCHAR				*m_pwszRootURL0;
		IScopedOperations	*m_pIScopedOperations0;
		IScopedOperations	*m_pIScopedOperations;

		BOOL					testIRow(
									IRow	*pIRow, 
									WCHAR	*pwszURL,
									BOOL	fFromRowset
								);
		BOOL					testIGetSession(IGetSession* pIGetSession);
		BOOL					testIColumnsInfo2(IColumnsInfo2* pIColumnsInfo2);
		BOOL					testICreateRow(
									ICreateRow	*pICreateRow, 
									WCHAR		*pwszURL
								);
		BOOL					testRowset(
									IAccessor	*pIAccessor,
									IRowset		*pIRowset
								);

		BOOL					testIGetRow(IGetRow* pIGetRow);

		BOOL					testIGetSourceRow(IGetSourceRow* pIGetSourceRow);
		
		//verifies whether the bindings created are
		// retrievable ok using IAccessor::GetBindings
		BOOL					VerifyBindings(
									IAccessor*		pIAccessor,           //[IN] Pointer to IAccessor. 
									HACCESSOR		hAccessor,            //[IN] handle to accessor
									DBCOUNTITEM		cBindings,            //[IN] Number of binding structs
									DBBINDING*		rgBindings,           //[IN] Binding structures
									DBACCESSORFLAGS dwCreateAccessorFlags //[IN] Value of Accessor flags used to create accessor
								);

		static unsigned WINAPI	CancelProc(void*);


public:
	//constructors
	TCIScopedOperations(WCHAR* pwszTestCaseName = INVALID(WCHAR*)) :
	  CSessionObject(pwszTestCaseName) {}

	virtual ~TCIScopedOperations(){}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// used for threading stuff
	//virtual unsigned MyThreadProc(Method enMethodCalled, LPVOID pParam);

	// make the thread friend of this class
	friend unsigned WINAPI	ThreadProc(LPVOID lpvThreadParam);	

};


// {{ TCW_TEST_CASE_MAP(TCOpenRowset)
//*-----------------------------------------------------------------------
// @class IScopedOperations::OpenRowset test
//
class TCOpenRowset : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOpenRowset,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember cPropertySets != 0, rgPropertySets == NULL => E_INVALIDARG
	int Variation_1();
	// @cmember cPropertySets == 0 and rgPropertySets == NULL
	int Variation_2();
	// @cmember cPRopertySets == 0 and rgPropertySets != NULL
	int Variation_3();
	// @cmember cPRopertySets != 0 and rgPropertySets != NULL
	int Variation_4();
	// @cmember inexistent URL
	int Variation_5();
	// @cmember out of scope URL
	int Variation_6();
	// @cmember URL in scope
	int Variation_7();
	// @cmember NULL ppRowset passed to check properties
	int Variation_8();
	// @cmember not NULL pIndexID => E_INVALIDARG
	int Variation_9();
	// @cmember Check children rowsets of each row in a tree
	int Variation_10();
	// @cmember Aggregation, IID_IUnknown asked
	int Variation_11();
	// @cmember Aggregation, ask another interface than IID_IUnknown
	int Variation_12();
	// @cmember No aggregation
	int Variation_13();
	// @cmember Agg - OpenRowset -> Rowset ->GetReferencedRowset
	int Variation_14();
	// @cmember OpenRowset -> agg IColumnsRowset asking IID_IUnknown
	int Variation_16();
	// @cmember OpenRowset -> agg IColumnsRowset asking non IID_IUnknown
	int Variation_17();
	// @cmember NULL pTableID => children rowset of the current row
	int Variation_19();
	// @cmember NULL pTableID.uName.pwszName => children rowset of the current row
	int Variation_20();
	// @cmember empty table name => children rowset of the current row
	int Variation_21();
	// @cmember Check all rowset interfaces
	int Variation_22();
	// @cmember cProperties != 0 in an element of rgPropertySets while rgProperties == NULL
	int Variation_24();
	// @cmember cProperties == 0 in an element of rgPropertySets while rgProperties != NULL
	int Variation_25();
	// @cmember cProperties == 0 in an element of rgPropertySets and rgProperties == NULL
	int Variation_26();
	// @cmember pTableID.eKind != DBKIND_NAME => E_INVALIDARG
	int Variation_27();
	// @cmember riid == IID_NULL -> E_NOINTERFACE
	int Variation_28();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCOpenRowset)
#define THE_CLASS TCOpenRowset
BEG_TEST_CASE(TCOpenRowset, TCIScopedOperations, L"IScopedOperations::OpenRowset test")
	TEST_VARIATION(1, 		L"cPropertySets != 0, rgPropertySets == NULL => E_INVALIDARG")
	TEST_VARIATION(2, 		L"cPropertySets == 0 and rgPropertySets == NULL")
	TEST_VARIATION(3, 		L"cPRopertySets == 0 and rgPropertySets != NULL")
	TEST_VARIATION(4, 		L"cPRopertySets != 0 and rgPropertySets != NULL")
	TEST_VARIATION(5, 		L"inexistent URL")
	TEST_VARIATION(6, 		L"out of scope URL")
	TEST_VARIATION(7, 		L"URL in scope")
	TEST_VARIATION(8, 		L"NULL ppRowset passed to check properties")
	TEST_VARIATION(9, 		L"not NULL pIndexID => E_INVALIDARG")
	TEST_VARIATION(10, 		L"Check children rowsets of each row in a tree")
	TEST_VARIATION(11, 		L"Aggregation, IID_IUnknown asked")
	TEST_VARIATION(12, 		L"Aggregation, ask another interface than IID_IUnknown")
	TEST_VARIATION(13, 		L"No aggregation")
	TEST_VARIATION(14, 		L"Agg - OpenRowset -> Rowset ->GetReferencedRowset")
	TEST_VARIATION(16, 		L"OpenRowset -> agg IColumnsRowset asking IID_IUnknown")
	TEST_VARIATION(17, 		L"OpenRowset -> agg IColumnsRowset asking non IID_IUnknown")
	TEST_VARIATION(19, 		L"NULL pTableID => children rowset of the current row")
	TEST_VARIATION(20, 		L"NULL pTableID.uName.pwszName => children rowset of the current row")
	TEST_VARIATION(21, 		L"empty table name => children rowset of the current row")
	TEST_VARIATION(22, 		L"Check all rowset interfaces")
	TEST_VARIATION(24, 		L"cProperties != 0 in an element of rgPropertySets while rgProperties == NULL")
	TEST_VARIATION(25, 		L"cProperties == 0 in an element of rgPropertySets while rgProperties != NULL")
	TEST_VARIATION(26, 		L"cProperties == 0 in an element of rgPropertySets and rgProperties == NULL")
	TEST_VARIATION(27, 		L"pTableID.eKind != DBKIND_NAME => E_INVALIDARG")
	TEST_VARIATION(28, 		L"riid == IID_NULL -> E_NOINTERFACE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCopy)
//*-----------------------------------------------------------------------
// @class IScopedOperations::Copy()
//
class TCCopy : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCopy,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember cRows == 0 => the tree is preserved
	int Variation_1();
	// @cmember cRows > 0 and rgpwszSourceURLs is NULL => E_INVALIDARG
	int Variation_2();
	// @cmember cRows>0 and rgpwszDestURLs is NULL => E_INVALIDARG
	int Variation_3();
	// @cmember cRows>0 and rgdwStatus is NULL => E_INVALIDARG
	int Variation_4();
	// @cmember rgpwszNewURLs and ppStringsBuffer are NULL
	int Variation_5();
	// @cmember NULL rgpwszNewURLs and ppStringsBuffer is not NULL
	int Variation_6();
	// @cmember not NULL rgpwszNewURLs and NULL ppwszStringsBuffer
	int Variation_7();
	// @cmember not NULL rgpwszNewURLs and not NULL ppwszStringsBuffer
	int Variation_8();
	// @cmember Copy a subtree to another subtree
	int Variation_9();
	// @cmember Copy a leaf to another one
	int Variation_10();
	// @cmember Copy a subtree instead of a leaf
	int Variation_11();
	// @cmember Copy a leaf instead of a subtree
	int Variation_12();
	// @cmember Copy nerecursive a subtree to a leaf
	int Variation_13();
	// @cmember Copy nerecursive a subtree to a subtree
	int Variation_14();
	// @cmember Inexistent URL passed as source
	int Variation_15();
	// @cmember Invalid destination URL
	int Variation_16();
	// @cmember Source outside current scope
	int Variation_17();
	// @cmember Destination outside current scope
	int Variation_18();
	// @cmember Source in the scope of destination
	int Variation_19();
	// @cmember Destination in the scope of source
	int Variation_20();
	// @cmember atomic ops: one op fails
	int Variation_21();
	// @cmember atomic ops: all ops fail
	int Variation_22();
	// @cmember atomic ops: all ops pass
	int Variation_23();
	// @cmember non atomic, all pass
	int Variation_24();
	// @cmember non atomic, one op fail
	int Variation_25();
	// @cmember non atomic, all ops fail
	int Variation_26();
	// @cmember no replace flag, existing destination
	int Variation_27();
	// @cmember no replace flag, no existing destination
	int Variation_28();
	// @cmember replace, existing destination
	int Variation_29();
	// @cmember replace, no existing destination
	int Variation_30();
	// @cmember generate destination URLs and check them
	int Variation_31();
	// @cmember Inexistent flag => E_INVALIDARG
	int Variation_32();
	// @cmember element of rgpwszSourceURLs is NULL => E_INVALIDARG
	int Variation_33();
	// @cmember element of rgpwszDestURLs is NULL => E_INVALIDARG
	int Variation_34();
	// @cmember Async operation
	int Variation_35();
	// @cmember Try to cancel asynch op befor returning
	int Variation_36();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCopy)
#define THE_CLASS TCCopy
BEG_TEST_CASE(TCCopy, TCIScopedOperations, L"IScopedOperations::Copy()")
	TEST_VARIATION(1, 		L"cRows == 0 => the tree is preserved")
	TEST_VARIATION(2, 		L"cRows > 0 and rgpwszSourceURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(3, 		L"cRows>0 and rgpwszDestURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(4, 		L"cRows>0 and rgdwStatus is NULL => E_INVALIDARG")
	TEST_VARIATION(5, 		L"rgpwszNewURLs and ppStringsBuffer are NULL")
	TEST_VARIATION(6, 		L"NULL rgpwszNewURLs and ppStringsBuffer is not NULL")
	TEST_VARIATION(7, 		L"not NULL rgpwszNewURLs and NULL ppwszStringsBuffer")
	TEST_VARIATION(8, 		L"not NULL rgpwszNewURLs and not NULL ppwszStringsBuffer")
	TEST_VARIATION(9, 		L"Copy a subtree to another subtree")
	TEST_VARIATION(10, 		L"Copy a leaf to another one")
	TEST_VARIATION(11, 		L"Copy a subtree instead of a leaf")
	TEST_VARIATION(12, 		L"Copy a leaf instead of a subtree")
	TEST_VARIATION(13, 		L"Copy nerecursive a subtree to a leaf")
	TEST_VARIATION(14, 		L"Copy nerecursive a subtree to a subtree")
	TEST_VARIATION(15, 		L"Inexistent URL passed as source")
	TEST_VARIATION(16, 		L"Invalid destination URL")
	TEST_VARIATION(17, 		L"Source outside current scope")
	TEST_VARIATION(18, 		L"Destination outside current scope")
	TEST_VARIATION(19, 		L"Source in the scope of destination")
	TEST_VARIATION(20, 		L"Destination in the scope of source")
	TEST_VARIATION(21, 		L"atomic ops: one op fails")
	TEST_VARIATION(22, 		L"atomic ops: all ops fail")
	TEST_VARIATION(23, 		L"atomic ops: all ops pass")
	TEST_VARIATION(24, 		L"non atomic, all pass")
	TEST_VARIATION(25, 		L"non atomic, one op fail")
	TEST_VARIATION(26, 		L"non atomic, all ops fail")
	TEST_VARIATION(27, 		L"no replace flag, existing destination")
	TEST_VARIATION(28, 		L"no replace flag, no existing destination")
	TEST_VARIATION(29, 		L"replace, existing destination")
	TEST_VARIATION(30, 		L"replace, no existing destination")
	TEST_VARIATION(31, 		L"generate destination URLs and check them")
	TEST_VARIATION(32, 		L"Inexistent flag => E_INVALIDARG")
	TEST_VARIATION(33, 		L"element of rgpwszSourceURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(34, 		L"element of rgpwszDestURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(35, 		L"Async operation")
	TEST_VARIATION(36, 		L"Try to cancel asynch op befor returning")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCDelete)
//*-----------------------------------------------------------------------
// @class IScopedOperations::Delete()
//
class TCDelete : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDelete,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember cRows is 0 => no operation
	int Variation_1();
	// @cmember cRows>0, rgpwszURLs is NULL => E_INVALIDARG
	int Variation_2();
	// @cmember cRows>0, rgdwStatus is NULL => E_INVALIDARG
	int Variation_3();
	// @cmember Inexistent URL
	int Variation_4();
	// @cmember URL outside of current scope
	int Variation_5();
	// @cmember Delete a leaf node
	int Variation_6();
	// @cmember Delete a subtree
	int Variation_7();
	// @cmember Non atomic, all ops pass
	int Variation_8();
	// @cmember Non atomic, one op fails, one passed
	int Variation_9();
	// @cmember Non atomic, all ops fail
	int Variation_10();
	// @cmember Atomic, all ops pass
	int Variation_11();
	// @cmember Atomic, on op passes, one fails
	int Variation_12();
	// @cmember Atomic, all ops fail
	int Variation_13();
	// @cmember Delete a row twice
	int Variation_14();
	// @cmember Try to delete a row from a subtree that is already been deleted
	int Variation_15();
	// @cmember Delete an inexistent row
	int Variation_16();
	// @cmember Copy a row to another one and then try to delete the source and the destination
	int Variation_17();
	// @cmember Move a row to another one and then try to delete the source and the destination
	int Variation_18();
	// @cmember Inexistent flag => E_INVALIDARG
	int Variation_19();
	// @cmember Async operation
	int Variation_20();
	// @cmember Try to cancel the asynch operation before the call completes
	int Variation_21();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCDelete)
#define THE_CLASS TCDelete
BEG_TEST_CASE(TCDelete, TCIScopedOperations, L"IScopedOperations::Delete()")
	TEST_VARIATION(1, 		L"cRows is 0 => no operation")
	TEST_VARIATION(2, 		L"cRows>0, rgpwszURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(3, 		L"cRows>0, rgdwStatus is NULL => E_INVALIDARG")
	TEST_VARIATION(4, 		L"Inexistent URL")
	TEST_VARIATION(5, 		L"URL outside of current scope")
	TEST_VARIATION(6, 		L"Delete a leaf node")
	TEST_VARIATION(7, 		L"Delete a subtree")
	TEST_VARIATION(8, 		L"Non atomic, all ops pass")
	TEST_VARIATION(9, 		L"Non atomic, one op fails, one passed")
	TEST_VARIATION(10, 		L"Non atomic, all ops fail")
	TEST_VARIATION(11, 		L"Atomic, all ops pass")
	TEST_VARIATION(12, 		L"Atomic, on op passes, one fails")
	TEST_VARIATION(13, 		L"Atomic, all ops fail")
	TEST_VARIATION(14, 		L"Delete a row twice")
	TEST_VARIATION(15, 		L"Try to delete a row from a subtree that is already been deleted")
	TEST_VARIATION(16, 		L"Delete an inexistent row")
	TEST_VARIATION(17, 		L"Copy a row to another one and then try to delete the source and the destination")
	TEST_VARIATION(18, 		L"Move a row to another one and then try to delete the source and the destination")
	TEST_VARIATION(19, 		L"Inexistent flag => E_INVALIDARG")
	TEST_VARIATION(20, 		L"Async operation")
	TEST_VARIATION(21, 		L"Try to cancel the asynch operation before the call completes")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCMove)
//*-----------------------------------------------------------------------
// @class IScopedOperations::Move()
//
class TCMove : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCMove,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember cRows is 0 => no op
	int Variation_1();
	// @cmember cRows>0 and rgpwszSourceURLs is NULL => E_INVALIDARG
	int Variation_2();
	// @cmember cRows>0 and rgpwszDestURLs is NULL => E_INVALIDARG
	int Variation_3();
	// @cmember cRows>0 and rgdwStatus is NULL => E_INVALIDARG
	int Variation_4();
	// @cmember rgpwszNewURLs is NULL and ppStringsBuffer is NULL
	int Variation_5();
	// @cmember rgpwszNewURLs is NULL and ppStringsBuffer is not NULL
	int Variation_6();
	// @cmember rgpwszNewURLs is not NULL and ppStringsBuffer is NULL
	int Variation_7();
	// @cmember rgpwszNewURLs is not NULL and ppStringsBuffer is not NULL
	int Variation_8();
	// @cmember Inexistent source URL
	int Variation_9();
	// @cmember Source is outside of current scope
	int Variation_10();
	// @cmember The parent of the destination doesn't exist
	int Variation_11();
	// @cmember Destination is outside the current scope
	int Variation_12();
	// @cmember Source is in scope of destination
	int Variation_13();
	// @cmember Destination is in the scope of source
	int Variation_14();
	// @cmember Move a leaf to another existing leaf, with and without replace
	int Variation_15();
	// @cmember Move a leaf to a new row
	int Variation_16();
	// @cmember Move a leaf to a subtree (existent) with and without replace
	int Variation_17();
	// @cmember Move a subtree to a leaf (existent) with and without replacing
	int Variation_18();
	// @cmember Move a subtree to an existent subtree with or without replacing
	int Variation_19();
	// @cmember Move a subtree to a new row
	int Variation_20();
	// @cmember Atomicity, at least one failure
	int Variation_21();
	// @cmember Atomicity, all operations fail
	int Variation_22();
	// @cmember Atomicity, all succeed
	int Variation_23();
	// @cmember Non atomic move operations, at least one succeed, at least one fails => DB_S_ERRORSOCCURRED
	int Variation_24();
	// @cmember Non atomic move operations, all ops fail => DB_E_ERRORSOCCURRED
	int Variation_25();
	// @cmember Non atomic move, all operations succeed
	int Variation_26();
	// @cmember No replace, existing destination => error
	int Variation_27();
	// @cmember Replace, existing destination => ok
	int Variation_28();
	// @cmember No replace, no existing destination => ok
	int Variation_29();
	// @cmember Replace, no existing destination => ok
	int Variation_30();
	// @cmember Generate the destination rowsets in ppStringsBuffer and check them
	int Variation_31();
	// @cmember Inexistent flag => E_INVALIDARG
	int Variation_32();
	// @cmember element of rgpwszSourceURLs is NULL => E_INVALIDARG
	int Variation_33();
	// @cmember element of rgpwszDestURLs is NULL => E_INVALIDARG
	int Variation_34();
	// @cmember Async operation
	int Variation_35();
	// @cmember Try to cancel the asynch operation before the call completes
	int Variation_36();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCMove)
#define THE_CLASS TCMove
BEG_TEST_CASE(TCMove, TCIScopedOperations, L"IScopedOperations::Move()")
	TEST_VARIATION(1, 		L"cRows is 0 => no op")
	TEST_VARIATION(2, 		L"cRows>0 and rgpwszSourceURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(3, 		L"cRows>0 and rgpwszDestURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(4, 		L"cRows>0 and rgdwStatus is NULL => E_INVALIDARG")
	TEST_VARIATION(5, 		L"rgpwszNewURLs is NULL and ppStringsBuffer is NULL")
	TEST_VARIATION(6, 		L"rgpwszNewURLs is NULL and ppStringsBuffer is not NULL")
	TEST_VARIATION(7, 		L"rgpwszNewURLs is not NULL and ppStringsBuffer is NULL")
	TEST_VARIATION(8, 		L"rgpwszNewURLs is not NULL and ppStringsBuffer is not NULL")
	TEST_VARIATION(9, 		L"Inexistent source URL")
	TEST_VARIATION(10, 		L"Source is outside of current scope")
	TEST_VARIATION(11, 		L"The parent of the destination doesn't exist")
	TEST_VARIATION(12, 		L"Destination is outside the current scope")
	TEST_VARIATION(13, 		L"Source is in scope of destination")
	TEST_VARIATION(14, 		L"Destination is in the scope of source")
	TEST_VARIATION(15, 		L"Move a leaf to another existing leaf, with and without replace")
	TEST_VARIATION(16, 		L"Move a leaf to a new row")
	TEST_VARIATION(17, 		L"Move a leaf to a subtree (existent) with and without replace")
	TEST_VARIATION(18, 		L"Move a subtree to a leaf (existent) with and without replacing")
	TEST_VARIATION(19, 		L"Move a subtree to an existent subtree with or without replacing")
	TEST_VARIATION(20, 		L"Move a subtree to a new row")
	TEST_VARIATION(21, 		L"Atomicity, at least one failure")
	TEST_VARIATION(22, 		L"Atomicity, all operations fail")
	TEST_VARIATION(23, 		L"Atomicity, all succeed")
	TEST_VARIATION(24, 		L"Non atomic move operations, at least one succeed, at least one fails => DB_S_ERRORSOCCURRED")
	TEST_VARIATION(25, 		L"Non atomic move operations, all ops fail => DB_E_ERRORSOCCURRED")
	TEST_VARIATION(26, 		L"Non atomic move, all operations succeed")
	TEST_VARIATION(27, 		L"No replace, existing destination => error")
	TEST_VARIATION(28, 		L"Replace, existing destination => ok")
	TEST_VARIATION(29, 		L"No replace, no existing destination => ok")
	TEST_VARIATION(30, 		L"Replace, no existing destination => ok")
	TEST_VARIATION(31, 		L"Generate the destination rowsets in ppStringsBuffer and check them")
	TEST_VARIATION(32, 		L"Inexistent flag => E_INVALIDARG")
	TEST_VARIATION(33, 		L"element of rgpwszSourceURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(34, 		L"element of rgpwszDestURLs is NULL => E_INVALIDARG")
	TEST_VARIATION(35, 		L"Async operation")
	TEST_VARIATION(36, 		L"Try to cancel the asynch operation before the call completes")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindRow)
//*-----------------------------------------------------------------------
// @class IScopedOperations::Bind() test
//
class TCBindRow : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindRow,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Inexistent URL => DB_E_NOTFOUND
	int Variation_1();
	// @cmember URL out of current scope => DB_E_RESOURCEOUTOFSCOPE
	int Variation_2();
	// @cmember Invalid bind flags -> E_INVALIDARG
	int Variation_3();
	// @cmember Get IRow
	int Variation_4();
	// @cmember Test pIScopedOperations as a row interface (QI)
	int Variation_5();
	// @cmember Check that binding to DBGUID_DSO fails
	int Variation_6();
	// @cmember Check that binding to DBGUID_SESSION fails
	int Variation_7();
	// @cmember Invalid rguid
	int Variation_8();
	// @cmember Unsupported interface => E_NOINTERFACE
	int Variation_9();
	// @cmember Not null pImplSession => E_INVALIDARG
	int Variation_10();
	// @cmember Get IColumnsInfo
	int Variation_11();
	// @cmember Get IGetSession
	int Variation_12();
	// @cmember Get IConvertType
	int Variation_13();
	// @cmember Get IColumnsInfo2 (optional)
	int Variation_14();
	// @cmember Get ICreateRow (optional)
	int Variation_15();
	// @cmember Flag - WAITFORINIT
	int Variation_16();
	// @cmember Flag - READWRITE
	int Variation_17();
	// @cmember Flag - SHARE_DENY_READ
	int Variation_18();
	// @cmember Flag - SHARE_DENY_WRITE
	int Variation_19();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_20();
	// @cmember Flag - SHARE_DENY_NONE
	int Variation_21();
	// @cmember Flag - RECURSIVE & SHARE_DENY_WRITE
	int Variation_22();
	// @cmember Flag - OUTPUT
	int Variation_23();
	// @cmember Flag - ASYNCH
	int Variation_24();
	// @cmember Flag - DELATFETCHSTREAM
	int Variation_25();
	// @cmember Flag - DELAYFETCHCOLUMNS
	int Variation_26();
	// @cmember General - Aggregate Row
	int Variation_27();
	// @cmember General - Aggregate implicit session
	int Variation_28();
	// @cmember Flag - RECURSIVE & SHARE_DENY_READ
	int Variation_29();
	// @cmember Flag - RECURSIVE & SHARE_DENY_EXCLUSIVE
	int Variation_30();
	// @cmember Flag - RECURSIVE & SHARE_DENY_NONE => E_INVALIDARG
	int Variation_31();
	// @cmember Flag - RECURSIVE => E_INVALIDARG
	int Variation_32();
	// @cmember Test DBPROPVAL_OO_SCOPED flag of DBPROP_OLEOBJECTS in DBPROPSET_DATASOURCEINFO
	int Variation_33();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindRow)
#define THE_CLASS TCBindRow
BEG_TEST_CASE(TCBindRow, TCIScopedOperations, L"IScopedOperations::Bind() test")
	TEST_VARIATION(1, 		L"Inexistent URL => DB_E_NOTFOUND")
	TEST_VARIATION(2, 		L"URL out of current scope => DB_E_RESOURCEOUTOFSCOPE")
	TEST_VARIATION(3, 		L"Invalid bind flags -> E_INVALIDARG")
	TEST_VARIATION(4, 		L"Get IRow")
	TEST_VARIATION(5, 		L"Test pIScopedOperations as a row interface (QI)")
	TEST_VARIATION(6, 		L"Check that binding to DBGUID_DSO fails")
	TEST_VARIATION(7, 		L"Check that binding to DBGUID_SESSION fails")
	TEST_VARIATION(8, 		L"Invalid rguid")
	TEST_VARIATION(9, 		L"Unsupported interface => E_NOINTERFACE")
	TEST_VARIATION(10, 		L"Not null pImplSession => E_INVALIDARG")
	TEST_VARIATION(11, 		L"Get IColumnsInfo")
	TEST_VARIATION(12, 		L"Get IGetSession")
	TEST_VARIATION(13, 		L"Get IConvertType")
	TEST_VARIATION(14, 		L"Get IColumnsInfo2 (optional)")
	TEST_VARIATION(15, 		L"Get ICreateRow (optional)")
	TEST_VARIATION(16, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(17, 		L"Flag - READWRITE")
	TEST_VARIATION(18, 		L"Flag - SHARE_DENY_READ")
	TEST_VARIATION(19, 		L"Flag - SHARE_DENY_WRITE")
	TEST_VARIATION(20, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(21, 		L"Flag - SHARE_DENY_NONE")
	TEST_VARIATION(22, 		L"Flag - RECURSIVE & SHARE_DENY_WRITE")
	TEST_VARIATION(23, 		L"Flag - OUTPUT")
	TEST_VARIATION(24, 		L"Flag - ASYNCH")
	TEST_VARIATION(25, 		L"Flag - DELATFETCHSTREAM")
	TEST_VARIATION(26, 		L"Flag - DELAYFETCHCOLUMNS")
	TEST_VARIATION(27, 		L"General - Aggregate Row")
	TEST_VARIATION(28, 		L"General - Aggregate implicit session")
	TEST_VARIATION(29, 		L"Flag - RECURSIVE & SHARE_DENY_READ")
	TEST_VARIATION(30, 		L"Flag - RECURSIVE & SHARE_DENY_EXCLUSIVE")
	TEST_VARIATION(31, 		L"Flag - RECURSIVE & SHARE_DENY_NONE => E_INVALIDARG")
	TEST_VARIATION(32, 		L"Flag - RECURSIVE => E_INVALIDARG")
	TEST_VARIATION(33, 		L"Test DBPROPVAL_OO_SCOPED flag of DBPROP_OLEOBJECTS in DBPROPSET_DATASOURCEINFO")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindRowset)
//*-----------------------------------------------------------------------
// @class IScopedOperations::Bind() test
//
class TCBindRowset : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindRowset,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IAccessor
	int Variation_1();
	// @cmember General - Get IColumnsInfo
	int Variation_2();
	// @cmember General - Get IConvertType
	int Variation_3();
	// @cmember General - Get IRowset
	int Variation_4();
	// @cmember General - Get IRowsetInfo
	int Variation_5();
	// @cmember General - Get IRowsetIdentity
	int Variation_6();
	// @cmember General - Bind to URLs of rows of the Rowset
	int Variation_7();
	// @cmember General - Optional interfaces
	int Variation_8();
	// @cmember General - IGetRow (if ROW objects are supported)
	int Variation_9();
	// @cmember General - Aggregate Rowset
	int Variation_10();
	// @cmember General - Aggregate implicit session
	int Variation_11();
	// @cmember Flag - WAITFORINIT
	int Variation_12();
	// @cmember Flag - READWRITE
	int Variation_13();
	// @cmember Flag - SHARE_DENY_READ
	int Variation_14();
	// @cmember Flag - SHARE_DENY_WRITE
	int Variation_15();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_16();
	// @cmember Flag - SHARE_DENY_NONE
	int Variation_17();
	// @cmember Flag - RECURSIVE & SHARE_DENY_WRITE
	int Variation_18();
	// @cmember Flag - ASYNCH
	int Variation_19();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindRowset)
#define THE_CLASS TCBindRowset
BEG_TEST_CASE(TCBindRowset, TCIScopedOperations, L"IScopedOperations::Bind() test")
	TEST_VARIATION(1, 		L"General - Get IAccessor")
	TEST_VARIATION(2, 		L"General - Get IColumnsInfo")
	TEST_VARIATION(3, 		L"General - Get IConvertType")
	TEST_VARIATION(4, 		L"General - Get IRowset")
	TEST_VARIATION(5, 		L"General - Get IRowsetInfo")
	TEST_VARIATION(6, 		L"General - Get IRowsetIdentity")
	TEST_VARIATION(7, 		L"General - Bind to URLs of rows of the Rowset")
	TEST_VARIATION(8, 		L"General - Optional interfaces")
	TEST_VARIATION(9, 		L"General - IGetRow (if ROW objects are supported)")
	TEST_VARIATION(10, 		L"General - Aggregate Rowset")
	TEST_VARIATION(11, 		L"General - Aggregate implicit session")
	TEST_VARIATION(12, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(13, 		L"Flag - READWRITE")
	TEST_VARIATION(14, 		L"Flag - SHARE_DENY_READ")
	TEST_VARIATION(15, 		L"Flag - SHARE_DENY_WRITE")
	TEST_VARIATION(16, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(17, 		L"Flag - SHARE_DENY_NONE")
	TEST_VARIATION(18, 		L"Flag - RECURSIVE & SHARE_DENY_WRITE")
	TEST_VARIATION(19, 		L"Flag - ASYNCH")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCBindStream)
//*-----------------------------------------------------------------------
// @class IScopedOperations::Bind() test
//
class TCBindStream : public TCIScopedOperations { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCBindStream,TCIScopedOperations);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Get IGetSourceRow
	int Variation_1();
	// @cmember General - Get ISequentialStream
	int Variation_2();
	// @cmember General - Aggregate Stream
	int Variation_3();
	// @cmember Flag - WAITFORINIT
	int Variation_4();
	// @cmember Flag - READWRITE
	int Variation_5();
	// @cmember Flag - SHARE_EXCLUSIVE
	int Variation_6();
	// @cmember Flag - RECURSIVE & SHARE_DENY_WRITE
	int Variation_7();
	// @cmember Flag - OUTPUT
	int Variation_8();
	// @cmember Flag - ASYNCH
	int Variation_9();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCBindStream)
#define THE_CLASS TCBindStream
BEG_TEST_CASE(TCBindStream, TCIScopedOperations, L"IScopedOperations::Bind() test")
	TEST_VARIATION(1, 		L"General - Get IGetSourceRow")
	TEST_VARIATION(2, 		L"General - Get ISequentialStream")
	TEST_VARIATION(3, 		L"General - Aggregate Stream")
	TEST_VARIATION(4, 		L"Flag - WAITFORINIT")
	TEST_VARIATION(5, 		L"Flag - READWRITE")
	TEST_VARIATION(6, 		L"Flag - SHARE_EXCLUSIVE")
	TEST_VARIATION(7, 		L"Flag - RECURSIVE & SHARE_DENY_WRITE")
	TEST_VARIATION(8, 		L"Flag - OUTPUT")
	TEST_VARIATION(9, 		L"Flag - ASYNCH")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCOpenRowset)
	TEST_CASE(2, TCCopy)
	TEST_CASE(3, TCDelete)
	TEST_CASE(4, TCMove)
	TEST_CASE(5, TCBindRow)
	TEST_CASE(6, TCBindRowset)
	TEST_CASE(7, TCBindStream)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::GetInexistentURL
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::GetInexistentURL(
	WCHAR **ppwszInexistentRow, 
	WCHAR *pwszExclURL/*=NULL*/
)
{
	HRESULT	hr;
	WCHAR	*pwszLeaf	= NULL;
	WCHAR	wszSuffix[]	= L"/75";

	if (NULL == ppwszInexistentRow)
		return E_INVALIDARG;

	if (NULL == pwszExclURL)
	{
		TESTC_(hr = PickANode(&pwszLeaf), S_OK);
	}
	else
	{
		// exclude the given path
		TESTC_(hr = PickASecondNode(pwszExclURL, NC_NONE, &pwszLeaf), S_OK);
	}
	SAFE_ALLOC(*ppwszInexistentRow, WCHAR, wcslen(pwszLeaf)+wcslen(wszSuffix)+1);
	wcscpy(*ppwszInexistentRow, pwszLeaf);
	wcscat(*ppwszInexistentRow, wszSuffix);

CLEANUP:
	SAFE_FREE(pwszLeaf);
	return hr;
} //CIScOpsTree::GetInexistentURL





/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::GetURLWithInexistentFather
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::GetURLWithInexistentFather(
	WCHAR **ppwszInexistentRow, 
	WCHAR *pwszExclURL/*=NULL*/
)
{
	HRESULT	hr;
	WCHAR	*pwszLeaf	= NULL;
	WCHAR	wszSuffix[]	= L"/75/91";

	if (NULL == ppwszInexistentRow)
		return E_INVALIDARG;

	if (pwszExclURL)
	{
		TESTC_(hr = PickASecondNode(pwszExclURL, NC_NONE, &pwszLeaf), S_OK);
	}
	else
	{
		TESTC_(hr = PickANode(&pwszLeaf), S_OK);
	}
	SAFE_ALLOC(*ppwszInexistentRow, WCHAR, wcslen(pwszLeaf)+wcslen(wszSuffix)+1);
	wcscpy(*ppwszInexistentRow, pwszLeaf);
	wcscat(*ppwszInexistentRow, wszSuffix);

CLEANUP:
	SAFE_FREE(pwszLeaf);
	return hr;
} //CIScOpsTree::GetURLWithInexistentFather




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickASingleton
//
/////////////////////////////////////////////////////////////////////////////
// should return a leaf that is atomic (can't be bound to a rowset)
HRESULT CIScOpsTree::PickASingleton(
	WCHAR			**ppwszURL		//[out] the URL of the chosen row
)
{
	return PickANode(NC_Singleton, ppwszURL);
} //CIScOpsTree::PickASingleton



/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickASubtree
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::PickASubtree(
	WCHAR			**ppwszURL		//[out] the URL of the chosen row
)
{
	return PickANode(NC_SUBTREE, ppwszURL);
} //CIScOpsTree::PickASubtree




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::TrimLeafToSubtree
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::TrimLeafToSubtree(
	WCHAR *pwszURL,					//[in/out] the URL of the chosen row
	WCHAR *pwszAvoidURL/*= NULL*/	//[in/out] the URL of the chosen row
)
// since there aren't CSchema info about the parent of the root row
// do not ever chose the root as a node
{
	DBCOUNTITEM	ulLevel;
	DBCOUNTITEM	ulStartLevel;
	HRESULT	hr;
	WCHAR	*pwChar;

	if (NULL == pwszURL)
		return E_INVALIDARG;

	// how many slashes are in there?
	if (!CHECK(hr = GetRowLevel(m_pStartingSchema->GetRowURL(), &ulStartLevel), S_OK))
		return E_INVALIDARG;

	if (!CHECK(hr = GetRowLevel(pwszURL, &ulLevel), S_OK) || 0 == ulLevel)
		return E_INVALIDARG;

	// trim the URL and the node path
	ulLevel = (2>ulLevel-ulStartLevel)? ulStartLevel: 
		ulStartLevel + rand() % (ulLevel-ulStartLevel); // ulStartLevel..ulLevel = how many levels to trim
	for (; 0 < ulLevel; --ulLevel)
	{
		// get rid of a slash in *ppwszURL
		pwChar	= wcsrchr(pwszURL, L'/');
		*pwChar	= L'\0';
		// make sure the new URL is not the prefix of pwszAvoidURL
		if (pwszAvoidURL && pwszAvoidURL == wcsstr(pwszAvoidURL, pwszURL))
		{
			// restore the previous slash
			*pwChar = L'/';
			break;
		}
	}

	return S_OK;
} //CIScOpsTree::TrimLeafToSubtree




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickALeaf
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::PickALeaf(
	WCHAR **ppwszURL		//[out] the URL of the chosen row
)
{
	return PickANode(NC_LEAF, ppwszURL);
} //CIScOpsTree::PickALeaf




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickACollection
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::PickACollection(
	WCHAR **ppwszURL		//[out] the URL of the chosen row
)
{
	return PickANode(NC_Collection, ppwszURL);
} //CIScOpsTree::PickACollection




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickANode
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::PickANode(
	WCHAR			**ppwszURL		//[out] the URL of the chosen row
)
{
	return PickANode(NC_NONE, ppwszURL);
} //CIScOpsTree::PickANode




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickANode
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::PickANode(
	NODE_CONSTRAINT	NodeConstr,		//[in]  node constraint
	WCHAR			**ppwszURL		//[out] the URL of the chosen row
)
// choses a node from the "starting" subtree
// does ot affect m_pCrtSchema
{
	CSchema		*pCRootSchema = NULL;
	DBCOUNTITEM	ulRow;
	HRESULT		hr = E_FAIL;
	DBCOUNTITEM	cRow;

	if (NULL == ppwszURL)
		return E_INVALIDARG;

	// chose any leaf in the tree
	ulRow = rand() % m_pStartingSchema->GetLeafNo();
	TESTC_(hr = GetRow(ulRow, ppwszURL), S_OK);

	switch (NodeConstr)
	{
		case NC_NONE:
			if (0 == rand() %2)
				TrimLeafToSubtree(*ppwszURL);
			break;

		case NC_LEAF:
			// nothing, we have a leaf and it is indifferent 
			// whether it's a collection or not
			break;

		case NC_SUBTREE:
			TrimLeafToSubtree(*ppwszURL);
			break;

		case NC_Collection:
			// doesn't metter whether it is a subtree or a leaf
			if (!IsCollection(*ppwszURL) || 0 == rand() %2)
				TrimLeafToSubtree(*ppwszURL);
			break;

		case NC_LEAF_Collection:
			// if this is not a Collection, look for another leaf that is Collection
			for (cRow = ulRow; !IsCollection(*ppwszURL) && ulRow-1 != cRow;)
			{
				// move to the next row, see if this row is ok
				SAFE_FREE(*ppwszURL);
				cRow = (cRow+1) % m_pStartingSchema->GetLeafNo();
				TESTC_(hr = GetRow(cRow, ppwszURL), S_OK);
			}
			hr = !IsCollection(*ppwszURL)? E_FAIL: S_OK;
			break;

		case NC_Singleton:
			// if this is not a Singleton, look for another leaf that is Singleton
			for (cRow = ulRow; IsCollection(*ppwszURL) && ulRow-1 != cRow;)
			{
				// move to the next row, see if this row is ok
				SAFE_FREE(*ppwszURL);
				cRow = (cRow+1) % m_pStartingSchema->GetLeafNo();
				TESTC_(hr = GetRow(cRow, ppwszURL), S_OK);
			}
			hr = IsCollection(*ppwszURL)? E_FAIL: S_OK;
			break;


		default:
			hr = E_INVALIDARG;
			break;
	}

CLEANUP:
	if (FAILED(hr))
		SAFE_FREE(*ppwszURL);
	return hr;
} //CIScOpsTree::PickANode




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::PickASecondNode
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::PickASecondNode(
	WCHAR			*pwszExclURL,	//[in]  exclude URLs in this tree
	NODE_CONSTRAINT	NodeConstr,		//[in]  node constraint
	WCHAR			**ppwszURL		//[out] the URL of the chosen row
)
// selects a second node from the declared subtree
// does not affect current m_pCrtSchema
{
	HRESULT		hr = E_FAIL;
	CSchema		*pCSchema		= NULL;
	CSchema		*pCRootSchema	= NULL;
	DBCOUNTITEM		ulExclLeavesNo;
	DBCOUNTITEM		ulLastExclLeaf;
	DBCOUNTITEM		ulTotalLeafNo;
	DBCOUNTITEM		ulTotalAvNo;
	DBCOUNTITEM		ulRow;
	DBCOUNTITEM		cRow;

	if (NULL == ppwszURL || NULL == pwszExclURL)
		return E_INVALIDARG;

	// chose any leaf in the tree
	ulTotalLeafNo	= m_pStartingSchema->GetLeafNo();
	
	pCSchema		= GetSchema(pwszExclURL);
	ulExclLeavesNo	= pCSchema->GetLeafNo();
	ulTotalAvNo		= ulTotalLeafNo-ulExclLeavesNo;
	// find out the number of the last leaf in the subtree
	ulLastExclLeaf	= GetNoOfFirstLeafInSubtree(pCSchema) + ulExclLeavesNo - 1;

	ulRow = (rand() % ulTotalAvNo + ulLastExclLeaf + 1) % ulTotalLeafNo;

	TESTC_(hr = GetRow(ulRow, ppwszURL), S_OK);

	switch (NodeConstr)
	{
		case NC_NONE:
			if (0 == rand() %2)
				TrimLeafToSubtree(*ppwszURL, pwszExclURL);
			break;

		case NC_LEAF:
			// nothing, we have a leaf and it is indifferent 
			// whether it's a collection or not
			break;

		case NC_SUBTREE:
			TrimLeafToSubtree(*ppwszURL, pwszExclURL);
			break;

		case NC_Collection:
			// doesn't metter whether it is a subtree or a leaf
			if (!IsCollection(*ppwszURL) || 0 == rand() %2)
				TrimLeafToSubtree(*ppwszURL, pwszExclURL);
			break;

		case NC_LEAF_Collection:
			// if this is not a Collection, look for another leaf that is Collection
			for (cRow = ulRow; !IsCollection(*ppwszURL) && ulRow-1 != cRow;)
			{
				// move to the next row, see if this row is ok
				SAFE_FREE(*ppwszURL);
				cRow = ((cRow+1) % ulTotalAvNo + ulLastExclLeaf + 1) % ulTotalLeafNo;
				TESTC_(hr = GetRow(cRow, ppwszURL), S_OK);
			}
			hr = !IsCollection(*ppwszURL)? E_FAIL: S_OK;
			break;

		case NC_Singleton:
			// if this is not a Singleton, look for another leaf that is Singleton
			for (cRow = ulRow; IsCollection(*ppwszURL) && ulRow-1 != cRow;)
			{
				// move to the next row, see if this row is ok
				SAFE_FREE(*ppwszURL);
				cRow = ((cRow+1) % ulTotalAvNo + ulLastExclLeaf + 1) % ulTotalLeafNo;
				TESTC_(hr = GetRow(cRow, ppwszURL), S_OK);
			}
			hr = IsCollection(*ppwszURL)? E_FAIL: S_OK;
			break;


		default:
			hr = E_INVALIDARG;
			break;
	}

CLEANUP:
	if (FAILED(hr))
		SAFE_FREE(*ppwszURL);
	return hr;
} //CIScOpsTree::PickASecondLeaf




/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::Pick2Nodes
//
/////////////////////////////////////////////////////////////////////////////
// this is the simplified version
HRESULT CIScOpsTree::Pick2Nodes(
	WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
	WCHAR			**ppwszURL2		//[out] the URL of the chosen row
)
{
	return Pick2Nodes(NC_NONE, ppwszURL1, NC_NONE, ppwszURL2, PCO_NONE);
} //CIScOpsTree::Pick2Nodes



	
/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::Pick2Nodes
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::Pick2Nodes(
	NODE_CONSTRAINT	NodeConstr1,	//[in]  constraint for the first node 
	WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
	NODE_CONSTRAINT	NodeConstr2,	//[in]  constraint for the second node 
	WCHAR			**ppwszURL2,	//[out] the URL of the chosen row
	PAIR_CONSTRAINT	PairConstr		//[in]  pair constraint (none, overlapped, diff)
)
{
	HRESULT		hr = E_FAIL;

	TESTC_(hr = PickANode(NodeConstr1, ppwszURL1), S_OK);
	switch (PairConstr)
	{
		case PCO_NONE:
			// beware of the possibility to chose the same node!
			TESTC_(hr = PickANode(NodeConstr2, ppwszURL2), S_OK);
			break;

		case PCO_OVERLAP:
			// the first node is an ancestor of the second one
			*ppwszURL2 = wcsDuplicate(*ppwszURL1);
			TrimLeafToSubtree(*ppwszURL1);
			break;

		case PCO_DIFF:
			TESTC_(hr = PickASecondNode(*ppwszURL1, NodeConstr2, ppwszURL2), S_OK);
			break;

		default:
			hr = E_INVALIDARG;
			break;
	}

CLEANUP:
	if (FAILED(hr))
	{
		SAFE_FREE(*ppwszURL1);
		SAFE_FREE(*ppwszURL2);
	}
	return hr;
} //CIScOpsTree::Pick2Nodes



/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::Pick2DifLeaves
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::Pick2DifLeaves(
	WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
	WCHAR			**ppwszURL2		//[out] the URL of the chosen row
)
{
	return Pick2Nodes(NC_LEAF, ppwszURL1, NC_LEAF, ppwszURL2, PCO_DIFF);
} //CIScOpsTree::Pick2DifLeaves



/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::Pick2DifCollections
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::Pick2DifCollections(
	WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
	WCHAR			**ppwszURL2		//[out] the URL of the chosen row
)
{
	return Pick2Nodes(NC_Collection, ppwszURL1, NC_Collection, ppwszURL2, PCO_DIFF);
} //CIScOpsTree::Pick2DifCollections



/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::Pick2DifNodes
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::Pick2DifNodes(
	WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
	WCHAR			**ppwszURL2		//[out] the URL of the chosen row
)
{
	return Pick2Nodes(NC_NONE, ppwszURL1, NC_NONE, ppwszURL2, PCO_DIFF);
} //CIScOpsTree::Pick2DifNodes



/////////////////////////////////////////////////////////////////////////////
// CIScOpsTree::Pick2OverlappingNodes
//
/////////////////////////////////////////////////////////////////////////////
HRESULT CIScOpsTree::Pick2OverlappingNodes(
	WCHAR			**ppwszURL1,	//[out] the URL of the chosen row
	WCHAR			**ppwszURL2		//[out] the URL of the chosen row
)
{
	return Pick2Nodes(NC_NONE, ppwszURL1, NC_NONE, ppwszURL2, PCO_OVERLAP);
} //CIScOpsTree::Pick2OverlappingNodes




//--------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::Delete and making general checking
//--------------------------------------------------------------------------
HRESULT CIScOpsTree::DeleteAndCheck(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	DBCOUNTITEM			cRows,					// [in] number of copy ops	
	WCHAR				**rgpwszURLs,			// [in] source URLs
	DWORD				dwDeleteFlags,			// [in] copy flags
	DBSTATUS			*rgdwStatus,			// [out] filled on output
	BOOL				bCheckTree/*=TRUE*/		// [in] whether to check the tree
)
{
	HRESULT				hr;
	DBCOUNTITEM			cOp;
	HRESULT				hres		= E_FAIL;
	IRow				*pIRow		= NULL;
	CSchema				*pSchema	= NULL;
	BOOL				fRelInterf	= (NULL == pIScopedOperations);

	if (NULL == pIScopedOperations)
	{
		TESTC_(hres = m_pIBindResource->Bind(NULL, m_pwszTreeRoot, 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations), S_OK);
	}

	// call the miscfunc wrapper
	hr = IScopedOperations_Delete(pIScopedOperations, cRows, 
		rgpwszURLs, dwDeleteFlags, rgdwStatus);

	// remove all the deleted rows
	if (0 < cRows && (S_OK == hr || DB_S_ERRORSOCCURRED == hr))
		for (cOp = 0; cOp < cRows; cOp++)
		{
			if (S_OK == hr || DBSTATUS_S_OK == rgdwStatus[cOp])
			{
				// adjust the CIScOpsTree (deletion checking was made in miscfunc call
				pSchema = GetSchema(rgpwszURLs[cOp]);
				if (pSchema == m_pRootSchema)
					DestroyTree();
				else
				{
					RemoveSchema(pSchema);
					delete pSchema;
				}
			}
		}

	// otherwise it should be the responsability of the caller to
	// update the tree structure and to check the result

	// check the new tree
	if (bCheckTree)
	{
		ResetPosition();
		TESTC(CheckTree());
	}

	hres = S_OK;

CLEANUP:
	if (fRelInterf)
		SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRow);
	return (S_OK == hres)? hr: E_FAIL;
} //CIScOpsTree::DeleteAndCheck




//------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::Move and making general checking
//------------------------------------------------------------------------
HRESULT CIScOpsTree::CopyAndCheck(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	DBCOUNTITEM			cRows,					// [in] number of copy ops	
	WCHAR				**rgpwszSourceURLs,		// [in] source URLs
	WCHAR				**rgpwszDestURLs,		// [in] destination URLs
	DWORD				dwCopyFlags,			// [in] copy flags
	IAuthenticate		*pIAuthenticate,		// [in] authentication interface
	DBSTATUS			*rgdwStatus,			// [out] filled on output
	WCHAR				**rgpwszNewURLs,		// [in/out] new URLs
	WCHAR				**ppStringsBuffer		// [out] buffer for URL strings
)
{
	HRESULT				hr;
	DBCOUNTITEM			cOp;
	HRESULT				hres			= E_FAIL;
	IRow				*pIRow			= NULL;
	CSchema				*pSourceSchema	= NULL;
	CSchema				*pDestSchema	= NULL;
	CSchema				*pOldDestSchema	= NULL;
	CSchema				*pParentDestSchema = NULL;
	WCHAR				*pwszDestURL	= NULL;
	WCHAR				*pwszParentDestURL	= NULL;
	BOOL				fRelInterf	= (NULL == pIScopedOperations);

	if (NULL == pIScopedOperations)
	{
		TESTC_(hres = m_pIBindResource->Bind(NULL, m_pwszTreeRoot, 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations), S_OK);
	}

	// call the miscfunc wrapper
	hr = IScopedOperations_Copy(pIScopedOperations, cRows, rgpwszSourceURLs, rgpwszDestURLs, 
		dwCopyFlags, pIAuthenticate, rgdwStatus, rgpwszNewURLs, ppStringsBuffer);

	// update and check the tree
	if (	0 < cRows 
		&&	(S_OK == hr || DB_S_ERRORSOCCURRED == hr) 
		&&	(	m_lGenerateURL == DBPROPVAL_GU_NOTSUPPORTED 
			||	NULL != rgpwszNewURLs && NULL != ppStringsBuffer))
	{
		// update the tree configuration
		for (cOp = 0; cOp < cRows; cOp++)
		{
			if (S_OK == hr || DBSTATUS_S_OK == rgdwStatus[cOp])
			{
				// adjust the tree and check operation result
				pSourceSchema	= GetSchema(rgpwszSourceURLs[cOp]);

				// get the destination URL and its parent URL
				if (DBPROPVAL_GU_SUFFIX == m_lGenerateURL)
				{
					ASSERT(rgpwszNewURLs);
					// according to the enclosing if, rgpwszNewURLs is not null
					pwszDestURL = rgpwszNewURLs[cOp];
				}
				else
					pwszDestURL = rgpwszDestURLs[cOp];
				pwszParentDestURL = wcsDuplicate(pwszDestURL);
				*(wcsrchr(pwszParentDestURL, L'/')) = L'\0';
				
				// remove the possible destination row from tree
				pOldDestSchema = RemoveSchema(pwszDestURL);

				//retrieve the schema of the parent of destination
				pParentDestSchema = GetSchema(pwszParentDestURL);

				// add a destination schema
				pDestSchema = new CSchema(pSourceSchema, (CSchema*)NULL, dwCopyFlags & DBCOPY_NON_RECURSIVE);

				// wire it into the tree
				pParentDestSchema->AddChild(pDestSchema);
				
				// update the URL of destination schema
				pDestSchema->SetRowURL(pwszDestURL);
				pDestSchema->UpdateURL(pwszParentDestURL);
		
				// release temp memory
				SAFE_FREE(pwszParentDestURL);
				delete pOldDestSchema;
				pOldDestSchema = NULL;
			}
		}

		// check the tree
		for (cOp = 0; cOp < cRows; cOp++)
		{
			if (!rgpwszSourceURLs[cOp])
				continue;

			if (S_OK != hr && DBSTATUS_S_OK != rgdwStatus[cOp])
				continue;

			// adjust the tree and check operation result
			pSourceSchema	= GetSchema(rgpwszSourceURLs[cOp]);

			// get the destination URL and its parent URL
			if (DBPROPVAL_GU_SUFFIX == m_lGenerateURL)
			{
				ASSERT(rgpwszNewURLs);
				// according to the enclosing if, rgpwszNewURLs is not null
				pwszDestURL = rgpwszNewURLs[cOp];
			}
			else
				pwszDestURL = rgpwszDestURLs[cOp];
			pwszParentDestURL = wcsDuplicate(pwszDestURL);
			*(wcsrchr(pwszParentDestURL, L'/')) = L'\0';
				
			//retrieve the schema of the parent of destination
			pParentDestSchema = GetSchema(pwszParentDestURL);

			// check the destiantion subtree
			hres = SetPosition(pwszParentDestURL);
			if (S_OK == hres)
			{
				TESTC(CheckTree());
			}
			else
				ASSERT(FALSE);
			
			// release temp memory
			SAFE_FREE(pwszParentDestURL);
		}

		// check the new tree
		ResetPosition();
		TESTC(CheckTreeStructure());
	}

	// otherwise it should be the responsability of the caller to
	// update the tree structure and to check the result

	hres = S_OK;

CLEANUP:
	SAFE_FREE(pwszParentDestURL);
	if (fRelInterf)
		SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRow);
	return (S_OK == hres)? hr: E_FAIL;
} //CIScOpsTree::CopyAndCheck


// wrapper for calling IScopedOperations::Move and making general checking
HRESULT CIScOpsTree::MoveAndCheck(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	DBCOUNTITEM			cRows,					// [in] number of copy ops	
	WCHAR				**rgpwszSourceURLs,		// [in] source URLs
	WCHAR				**rgpwszDestURLs,		// [in] destination URLs
	DWORD				dwMoveFlags,			// [in] Move flags
	IAuthenticate		*pIAuthenticate,		// [in] authentication interface
	DBSTATUS			*rgdwStatus,			// [out] filled on output
	WCHAR				**rgpwszNewURLs,		// [in/out] new URLs
	WCHAR				**ppStringsBuffer		// [out] buffer for URL strings
)
{
	HRESULT				hr;
	DBCOUNTITEM			cOp;
	HRESULT				hres				= E_FAIL;
	IRow				*pIRow				= NULL;
	CSchema				*pSourceSchema		= NULL;
	CSchema				*pDestSchema		= NULL;
	CSchema				*pParentDestSchema	= NULL;
	WCHAR				*pwszDestURL	= NULL;
	WCHAR				*pwszParentDestURL	= NULL;
	BOOL				fRelInterf	= (NULL == pIScopedOperations);

	if (NULL == pIScopedOperations)
	{
		TESTC_(hres = m_pIBindResource->Bind(NULL, m_pwszTreeRoot, 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations), S_OK);
	}

	// call the miscfunc wrapper
	hr = IScopedOperations_Move(
		pIScopedOperations, cRows, rgpwszSourceURLs, rgpwszDestURLs, 
		dwMoveFlags, pIAuthenticate, rgdwStatus, rgpwszNewURLs, ppStringsBuffer);

	// remove all the deleted rows
	if (0 < cRows && (S_OK == hr || DB_S_ERRORSOCCURRED == hr))
	{
		for (cOp = 0; cOp < cRows; cOp++)
		{
			if (S_OK == hr || DBSTATUS_S_OK == rgdwStatus[cOp])
			{
				// adjust the tree and check operation result
				pSourceSchema = GetSchema(rgpwszSourceURLs[cOp]);
				RemoveSchema(pSourceSchema);

				// get the destination URL and its parent URL
				if (DBPROPVAL_GU_SUFFIX == m_lGenerateURL)
					pwszDestURL = rgpwszNewURLs[cOp];
				else
					pwszDestURL = rgpwszDestURLs[cOp];
				pwszParentDestURL = wcsDuplicate(pwszDestURL);
				*(wcsrchr(pwszParentDestURL, L'/')) = L'\0';
				
				// remove the possible destination row from tree
				pDestSchema = RemoveSchema(pwszDestURL);
				
				//retrieve the schema of the parent of destination
				pParentDestSchema = GetSchema(pwszParentDestURL);

				// wire it into the tree
				pParentDestSchema->AddChild(pSourceSchema);
				// update the URL of destination schema
				//if (NULL != pDestSchema)
				pSourceSchema->SetRowURL(pwszDestURL);
				pSourceSchema->UpdateURL(pwszParentDestURL);
				
				// release temp memory
				SAFE_FREE(pwszParentDestURL);
				delete pDestSchema;
				pDestSchema = NULL;
			}
		}

		// check the tree
		for (cOp = 0; cOp < cRows; cOp++)
		{
			if (!rgpwszSourceURLs[cOp])
				continue;

			if (S_OK != hr && DBSTATUS_S_OK != rgdwStatus[cOp])
				continue;

			// adjust the tree and check operation result
			pSourceSchema	= GetSchema(rgpwszSourceURLs[cOp]);

			// get the destination URL and its parent URL
			if (DBPROPVAL_GU_SUFFIX == m_lGenerateURL)
				pwszDestURL = rgpwszNewURLs[cOp];
			else
				pwszDestURL = rgpwszDestURLs[cOp];
			pwszParentDestURL = wcsDuplicate(pwszDestURL);
			*(wcsrchr(pwszParentDestURL, L'/')) = L'\0';
				
			//retrieve the schema of the parent of destination
			pParentDestSchema = GetSchema(pwszParentDestURL);

			// check the destiantion subtree
			hres = SetPosition(pwszParentDestURL);
			if (S_OK == hres)
			{
				TESTC(CheckTree());
			}
			else
				ASSERT(FALSE);

			// release temp memory
			SAFE_FREE(pwszParentDestURL);
		}

		// check the new tree
		ResetPosition();
		TESTC(CheckTreeStructure());
	}

	// otherwise it should be the responsability of the caller to
	// update the tree structure and to check the result

	hres = S_OK;

CLEANUP:
	SAFE_FREE(pwszParentDestURL);
	if (fRelInterf)
		SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRow);
	return (S_OK == hres)? hr: E_FAIL;
} //CIScOpsTree::MoveAndCheck




//--------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::OpenRowset and doing  basic general checking
//--------------------------------------------------------------------------
HRESULT CIScOpsTree::OpenRowsetAndCheck(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	IUnknown			*pUnkOuter,				// [in] controlling IUnknown of the rowset
	DBID				*pTableID,				// [in] URL of the row
	DBID				*pIndexID,				// [in] should be ignored
	REFIID				riid,					// [in] interface to be retrieved
	ULONG				cPropertySets,			// [in] number of elements in prop array
	DBPROPSET			*rgPropertySets,		// [in|out] property array
	IUnknown			**ppRowset				// [out] rowset interface
)
{
	HRESULT				hr;
	HRESULT				hres		= E_FAIL;
	BOOL				fRelInterf	= (NULL == pIScopedOperations);
	IRowset				*pIRowset	= NULL;

	if (NULL == pIScopedOperations)
	{
		TESTC_(hres = m_pIBindResource->Bind(NULL, m_pwszTreeRoot, 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations), S_OK);
	}

	// call the miscfunc wrapper
	hr = IScopedOperations_OpenRowset(
		pIScopedOperations, pUnkOuter, pTableID, pIndexID,
		riid, cPropertySets, rgPropertySets, ppRowset);

	if (SUCCEEDED(hr) && pTableID && DBKIND_NAME == pTableID->eKind 
		&& ppRowset && pTableID->uName.pwszName && *(pTableID->uName.pwszName))
	{
		// check the rowset componence and values
		TESTC(VerifyInterface(*ppRowset, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));
		TESTC(CheckRowset(pIRowset, pTableID->uName.pwszName));
	}

	hres = S_OK;

CLEANUP:
	if (fRelInterf)
		SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRowset);
	return (S_OK == hres)? hr: E_FAIL;
} //CIScOpsTree::OpenRowsetAndCheck



//--------------------------------------------------------------------------
//
// wrapper for calling IScopedOperations::Bind and doing  basic general checking
//--------------------------------------------------------------------------
HRESULT CIScOpsTree::BindAndCheck(
	IScopedOperations	*pIScopedOperations,	// [in] interface to use
	IUnknown			*pUnkOuter,				// [in] controlling IUnknown
	LPCOLESTR			pwszURL,				// [in] object to be bound
	DBBINDURLFLAG		dwBindFlags,			// [in] flags to be used for binding
	REFGUID				rguid,					// [in] indicates the type of the object being requested
	REFIID				riid,					// [in] requested interface
	IAuthenticate		*pAuthenticate,			// [in] pointer to IAuthenticate interface to be used
	DBIMPLICITSESSION	*pImplSession,			// [in] implicit session	
	DBBINDURLSTATUS		*pdwBindStatus,			// [out] bind status
	IUnknown			**ppUnk					// [out] interface on the bound object
)
{
	HRESULT				hr;
	HRESULT				hres		= E_FAIL;
	BOOL				fRelInterf	= (NULL == pIScopedOperations);
	IRowset				*pIRowset	= NULL;

	if (NULL == pIScopedOperations)
	{
		TESTC_(hres = m_pIBindResource->Bind(NULL, m_pwszTreeRoot, 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations), S_OK);
	}

	// call the miscfunc wrapper
	hr = IScopedOperations_Bind(
		pIScopedOperations, pUnkOuter, pwszURL, dwBindFlags,
		rguid, riid, pAuthenticate, pImplSession, pdwBindStatus, ppUnk);

	if (SUCCEEDED(hr))
	{
		if (DBGUID_ROW == rguid)
		{
			TESTC_(hr = VerifyRowValues((WCHAR*)pwszURL, *ppUnk, FALSE), S_OK);
		}
		else if (DBGUID_ROWSET == rguid)
		{
			// check the rowset componence and values
			TESTC(VerifyInterface(*ppUnk, IID_IRowset, ROWSET_INTERFACE, (IUnknown**)&pIRowset));
			TESTC(CheckRowset(pIRowset, (WCHAR*)pwszURL));
		}
	}

	hres = S_OK;

CLEANUP:
	if (fRelInterf)
		SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRowset);
	return (S_OK == hres)? hr: E_FAIL;
} //CIScOpsTree::BindAndCheck




// {{ TCW_TC_PROTOTYPE(TCIScopedOperations)
//*-----------------------------------------------------------------------
//| Test Case:	TCIScopedOperations - base class for IScopedOperations testing
//| Created:  	10/8/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIScopedOperations::Init()
{ 
	HRESULT	hr;

	m_pIScopedOperations0	= NULL;
	m_pIScopedOperations	= NULL;
	m_pwszRootURL0			= NULL;
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(!CSessionObject::Init())
	// }}
		return FALSE;

	m_pwszRootURL0 = wcsDuplicate(g_pCTree->GetRootURL());

	// bind the reference root
	TESTC_(hr = g_pIBindResource->Bind(NULL, m_pwszRootURL0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&m_pIScopedOperations0), S_OK);
	
	if (g_pwszRootRow)
		TESTC_(hr = g_pIBindResource->Bind(NULL, g_pwszRootRow, 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&m_pIScopedOperations), S_OK);

//	m_pTable = g_pCTable;

CLEANUP:
	ASSERT(NULL != m_pIScopedOperations0);
	return TRUE;
} 



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCIScopedOperations::Terminate()
{ 

	SAFE_FREE(m_pwszRootURL0);
	SAFE_RELEASE(m_pIScopedOperations0);
	SAFE_RELEASE(m_pIScopedOperations);
	if (g_pCTree)
		COMPARE(g_pCTree->CheckTreeStructure(), TRUE);

// {{ TCW_TERM_BASECLASS_CHECK2
	return CSessionObject::Terminate();
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END




unsigned WINAPI TCIScopedOperations::CancelProc(void *p)
{
	IDBAsynchStatus	*pIDBAsynchStatus = (IDBAsynchStatus*)p;

	pIDBAsynchStatus->Abort(DB_NULL_HCHAPTER, DBASYNCHOP_OPEN);
	_endthreadex(0);
	return TRUE;
} //TCIScopedOperations::CancelProc




//----------------------------------------------------------------------
// TCIScopedOperations::testIColumnsInfo2
//
BOOL TCIScopedOperations::testIColumnsInfo2(IColumnsInfo2* pIColumnsInfo2)
{
	TBEGIN
	DBORDINAL			cColumns		= 0;
	DBID*				rgColumnIDs		= NULL;
	DBCOLUMNINFO*		rgColumnInfo	= NULL;
	OLECHAR*			pStringsBuffer	= NULL;

	if(!pIColumnsInfo2)
		return FALSE;

	TESTC_(pIColumnsInfo2->GetRestrictedColumnInfo(0, NULL,
		0, &cColumns, &rgColumnIDs, &rgColumnInfo, 
		&pStringsBuffer), S_OK)

	TESTC(cColumns>0 && rgColumnIDs && pStringsBuffer && 
		rgColumnInfo)
	TESTC(rgColumnInfo[0].ulColumnSize >0)

CLEANUP:
	SAFE_FREE(rgColumnIDs);
	SAFE_FREE(rgColumnInfo);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} //TCIScopedOperations::testIColumnsInfo2




//TODO: This macro needs to actually test the object returned, not just non-NULL
//A better approch would be to call DefaultObjectTesting on Success and pass in an 
//Object type.  Also you could have a helper that does this for you.  Or you could
//invent a overloaded DefaultObjectTesting which takes an HR and does this call for you.
//Currently this macro is dangerous, as its doesn't verify anything functional on the returned
//object, so removing from Privlib since this the only occurrance of this macro, 
//(thus: reducing any widespread use of it)

// Check returned pointer to an interface, based on hr
#define TESTC_RET_IUNK(hr, pIUnknown)	TESTC(SUCCEEDED(hr)? NULL != pIUnknown: NULL == pIUnknown)

//----------------------------------------------------------------------
// TCIScopedOperations::testIGetSession
//
BOOL TCIScopedOperations::testIGetSession(IGetSession* pIGetSession)
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	IGetDataSource*		pGetDS = NULL;
	IOpenRowset*		pOR = NULL;
	ISessionProperties*	pSP = NULL;
	IDBSchemaRowset*	pSR = NULL;

	if(!pIGetSession)
		return FALSE;

	TEST2C_(hr = pIGetSession->GetSession(IID_IGetDataSource, 
		(IUnknown**)&pGetDS), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC_RET_IUNK(hr, pGetDS)

	TEST2C_(hr = pIGetSession->GetSession(IID_IOpenRowset, 
		(IUnknown**)&pOR), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC_RET_IUNK(hr, pOR)

	TEST2C_(hr = pIGetSession->GetSession(IID_ISessionProperties, 
		(IUnknown**)&pSP), S_OK, DB_E_NOSOURCEOBJECT)
	TESTC_RET_IUNK(hr, pSP)

	TEST3C_(hr = pIGetSession->GetSession(IID_IDBSchemaRowset, 
		(IUnknown**)&pSR), S_OK, DB_E_NOSOURCEOBJECT, E_NOINTERFACE)
	TESTC_RET_IUNK(hr, pSR)

CLEANUP:
	SAFE_RELEASE(pGetDS);
	SAFE_RELEASE(pOR);
	SAFE_RELEASE(pSP);
	SAFE_RELEASE(pSR);
	TRETURN
} //TCIScopedOperations::testIGetSession




//----------------------------------------------------------------------
// TCIScopedOperations::testIGetSourceRow
//
BOOL TCIScopedOperations::testIGetSourceRow(IGetSourceRow* pIGetSourceRow)
{
	TBEGIN
	HRESULT		hr;
	IRow*		pIRow = NULL;

	TESTC(pIGetSourceRow != NULL)

	TESTC_(hr=pIGetSourceRow->GetSourceRow(IID_IRow, NULL), E_INVALIDARG)

	TESTC_(hr=pIGetSourceRow->GetSourceRow(IID_IRowset, (IUnknown**)&pIRow),
		E_NOINTERFACE)
	TESTC(!pIRow)

	TEST2C_(hr=pIGetSourceRow->GetSourceRow(IID_IRow, (IUnknown**)&pIRow),
		S_OK, DB_E_NOSOURCEOBJECT)

	if(hr==S_OK)
		TESTC(pIRow != NULL)
	else
		odtLog<<L"INFO: There is no row object as context for the stream.\n";

CLEANUP:
	SAFE_RELEASE(pIRow);
	TRETURN
} //TCIScopedOperations::testIGetSourceRow




//----------------------------------------------------------------------
// TCIScopedOperations::testIRow
//
BOOL TCIScopedOperations::testIRow(IRow* pIRow, WCHAR *pwszURL, BOOL fFromRowset)
{
	BOOL		bRet = FALSE;
	HRESULT		hr = E_FAIL;
//	CRow		CRow;
	IRowset*	pIRowset = NULL;
	IAccessor*	pIAccessor = NULL;

	if(!pIRow)
		return FALSE;

	TEST2C_(hr=pIRow->GetSourceRowset(IID_IRowset, (IUnknown**)&pIRowset,
		NULL), S_OK, DB_E_NOSOURCEOBJECT)

	if(hr==S_OK)
		TESTC_(hr=pIRow->GetSourceRowset(IID_IAccessor, (IUnknown**)&pIAccessor,
			NULL), S_OK)
	else
		TESTC_(hr=pIRow->GetSourceRowset(IID_IAccessor, (IUnknown**)&pIAccessor,
			NULL), DB_E_NOSOURCEOBJECT)

	if (g_pCTree)
	{
		TESTC_(hr = g_pCTree->VerifyRowValues(pwszURL, pIRow, fFromRowset), S_OK);
	}

	bRet = TRUE;

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	return bRet;
} //TCIScopedOperations::testIRow




//----------------------------------------------------------------------
// TCIScopedOperations::testICreateRow
//
BOOL TCIScopedOperations::testICreateRow(
	ICreateRow	*pICreateRow, 
	WCHAR		*pwszURL
)
{
	TBEGIN
	HRESULT		hr = E_FAIL;
	LPOLESTR	pwszNewURL = NULL;
	IRow*		pIRow = NULL;

	if(!pICreateRow || !pwszURL)
		return FALSE;

	TEST2C_(hr=pICreateRow->CreateRow(NULL, pwszURL, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_OPENIFEXISTS, DBGUID_ROW, 
		IID_IRow, NULL, NULL, NULL, &pwszNewURL, (IUnknown**)&pIRow), 
		S_OK, DB_E_RESOURCEEXISTS)

	if(hr==DB_E_RESOURCEEXISTS)
		odtLog<<L"INFO: The provider does not support OPENIFEXISTS behaviour on ICreateRow.\n";
	else
	{
		TESTC(pwszNewURL != NULL)
		TESTC(testIRow(pIRow, pwszURL, FALSE))
	}

CLEANUP:
	SAFE_RELEASE(pIRow);
	SAFE_FREE(pwszNewURL);
	TRETURN
} //TCIScopedOperations::testICreateRow




//----------------------------------------------------------------------
// TCIScopedOperations::testRowset
//
BOOL TCIScopedOperations::testRowset(
	IAccessor*	pIAccessor,
	IRowset*	pIRowset
)
{
	TBEGIN
	ULONG			ulIndex = 0;
	ULONG			cRows = 0;
	DBBINDSTATUS*	rgStatus = NULL;
	WCHAR*			pStringsBuffer = NULL;
	DBLENGTH		cbRowSize = 0;
	DBCOUNTITEM		cBindings = 0;
	DBBINDING*		rgBindings = NULL;
	HACCESSOR		hAccessor = DB_NULL_HACCESSOR;

	TESTC(pIAccessor!=NULL && pIRowset!=NULL)

	TESTC(DefTestInterface(pIAccessor))
	TESTC(DefTestInterface(pIRowset))

	//Create Accessor with a binding using length, status and value.
	//All columns are bound including BLOB_LONG.
	TESTC_(GetAccessorAndBindings(pIAccessor, DBACCESSOR_ROWDATA,
		&hAccessor, &rgBindings, &cBindings, &cbRowSize,			
  		DBPART_LENGTH | DBPART_STATUS | DBPART_VALUE,
		 ALL_COLS_BOUND, FORWARD, NO_COLS_BY_REF, NULL, 
		 NULL, &pStringsBuffer, DBTYPE_EMPTY, 0, NULL, NULL, 
		 NO_COLS_OWNED_BY_PROV,	DBPARAMIO_NOTPARAM, NO_BLOB_COLS, 
		 &rgStatus),S_OK)

	//Verify status of bindings.
	for(ulIndex=0; ulIndex<cBindings; ulIndex++)
		TESTC(rgStatus[ulIndex] == DBBINDSTATUS_OK)

	TEST2C_(pIRowset->RestartPosition(NULL), S_OK, DB_S_COMMANDREEXECUTED)

	//Call the IAccessor::GetBindings method and verify.
	TESTC(VerifyBindings(pIAccessor, hAccessor, cBindings, rgBindings,
		DBACCESSOR_ROWDATA))

CLEANUP:
	SAFE_FREE(pStringsBuffer);
	SAFE_FREE(rgStatus);
	if(hAccessor)
		pIAccessor->ReleaseAccessor(hAccessor, NULL);
	FreeAccessorBindings(cBindings, rgBindings);
	TRETURN
} //TCIScopedOperations::testRowset




//----------------------------------------------------------------------
// TCIScopedOperations::VerifyBindings
//
BOOL TCIScopedOperations::VerifyBindings(
	IAccessor*		pIAccessor,           //[IN] Pointer to IAccessor. 
	HACCESSOR		hAccessor,            //[IN] handle to accessor
	DBCOUNTITEM		cBindings,            //[IN] Number of binding structs
	DBBINDING*		rgBindings,           //[IN] Binding structures
	DBACCESSORFLAGS dwCreateAccessorFlags //[IN] Value of Accessor flags used to create accessor
)
{
	TBEGIN
	DBCOUNTITEM		cGetBindings = 0;
	ULONG			ulIndex=0;
	DBBINDING*		pGetBind = NULL;
	DBBINDING*		pBind = NULL;
	DBBINDING*		rgGetBindings = NULL;
	DBACCESSORFLAGS	dwGetAccessorFlags = 0;

	TESTC(pIAccessor != NULL)

	//Obtained the bindings.
	TESTC_(pIAccessor->GetBindings(hAccessor, &dwGetAccessorFlags, 
		&cGetBindings, &rgGetBindings),S_OK)

	//Verify the Accessor Flags and number of bindings.
	COMPARE(dwGetAccessorFlags , dwCreateAccessorFlags);
	COMPARE(cGetBindings , cBindings);

	//Verify the binding stuctures.
	if (cGetBindings == 0)
	{
		//This is a null accessor, binding array should be null
		TESTC(rgGetBindings == NULL)
	}
	else
	{
		TESTC(cGetBindings != 0)
		TESTC(rgGetBindings != NULL)

		for(ulIndex=0; ulIndex<cGetBindings; ulIndex++)
		{
			pGetBind = &(rgGetBindings[ulIndex]);
			pBind = &(rgBindings[ulIndex]);

			COMPARE(pGetBind->dwPart , pBind->dwPart);
			COMPARE(pGetBind->iOrdinal , pBind->iOrdinal);
			COMPARE(pGetBind->wType , pBind->wType);
			COMPARE(pGetBind->eParamIO , pBind->eParamIO);				
			COMPARE(pGetBind->pTypeInfo , pBind->pTypeInfo);
							
			//Precision and scale only apply for numeric and decimal 
			//types and then only if VALUE is bound.
			if ((pBind->wType == DBTYPE_NUMERIC || 
				pBind->wType == DBTYPE_DECIMAL ||
				pBind->wType == DBTYPE_DBTIMESTAMP) &&
				(pBind->dwPart & DBPART_VALUE))
			{				
				COMPARE(pGetBind->bPrecision , 
					pBind->bPrecision);
				COMPARE(pGetBind->bScale , pBind->bScale);
			}
			
			//These only apply if value is bound
			if (pBind->dwPart & DBPART_VALUE)
			{
				COMPARE(pGetBind->obValue , pBind->obValue);					
				COMPARE(pGetBind->cbMaxLen , pBind->cbMaxLen);
			}
			
			//These only apply if type is DBTYPE_UNKNOWN				
			if ((pBind->wType == DBTYPE_IUNKNOWN) &&
				(pGetBind->pObject != NULL))
			{
				COMPARE(pGetBind->pObject->dwFlags , 
					pBind->pObject->dwFlags)	;					
				COMPARE(pGetBind->pObject->iid , 
					pBind->pObject->iid);
			}

			//These only apply if length is bound
			if (pBind->dwPart & DBPART_LENGTH)
				COMPARE(pGetBind->obLength , pBind->obLength);
			
			//These only apply if status is bound
			if (pBind->dwPart & DBPART_STATUS)
				COMPARE(pGetBind->obStatus , pBind->obStatus);
		}//For Loop
	}

CLEANUP:
	FreeAccessorBindings(cGetBindings, rgGetBindings);
	TRETURN
} //TCIScopedOperations::VerifyBindings




//----------------------------------------------------------------------
// TCIScopedOperations::testIGetRow
//
BOOL TCIScopedOperations::testIGetRow(IGetRow* pIGetRow)
{
	TBEGIN
	HRESULT		hr;
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW*		rghRows			= NULL;
	LPOLESTR	pwszURL			= NULL;
	IRow*		pIRow			= NULL;
	IRowset*	pIRowset		= NULL;

	TESTC(pIGetRow != NULL)

	TESTC(VerifyInterface(pIGetRow,IID_IRowset,
		ROWSET_INTERFACE,(IUnknown**)&pIRowset))

	TEST2C_(hr=pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),
		S_OK, DB_S_ENDOFROWSET)
	if(hr==S_OK)
	{
		TESTC((cRowsObtained==1)&&(rghRows!=NULL)&&(*rghRows!=NULL))

		//Use the Row Handle to obtain IRow on the Row Object.
		TEST2C_(pIGetRow->GetRowFromHROW(NULL, rghRows[0], IID_IRow, 
			(IUnknown**)&pIRow), S_OK, DB_S_NOROWSPECIFICCOLUMNS)
		TESTC(pIRow != NULL)

		TESTC_(pIGetRow->GetURLFromHROW(rghRows[0], &pwszURL), S_OK)
		TESTC(pwszURL != NULL)
	}

CLEANUP:
	if(pIRowset && rghRows)
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL,NULL,NULL);
	SAFE_FREE(rghRows);
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowset);
	TRETURN
} //TCIScopedOperations::testIGetRow

/*
unsigned TCIScopedOperations::MyThreadProc(Method enMethodCalled, LPVOID pParam)
{
	switch (enMethodCalled)
	{
		case Method_CopyAndCheck:
			{
				CpACParam		*p = (CpACParam*)pParam;
				
				p->hRet = CopyAndCheck(p->pIScopedOperations,
					p->cRows, p->rgpwszSourceURLs, 
					p->rgpwszDestURLs, p->dwCopyFlags,
					p->pAuthenticate, p->rgdwStatus, 
					p->rgpwszNewURLs, p->ppStringsBuffer,
					p->hrExpected, p->fValid, p->phActualRes);
			}
			break;
		case Method_MoveAndCheck:
			{
				CpACParam		*p = (CpACParam*)pParam;
				
				p->hRet = MoveAndCheck(p->pIScopedOperations,
					p->cRows, p->rgpwszSourceURLs, 
					p->rgpwszDestURLs, p->dwCopyFlags,
					p->pAuthenticate, p->rgdwStatus, 
					p->rgpwszNewURLs, p->ppStringsBuffer,
					p->hrExpected, p->fValid, p->phActualRes);
			}
			break;
		default:
			return 1;
	}

	return 0;
} //TCIScopedOperations::MyThreadProc
*/



// {{ TCW_TC_PROTOTYPE(TCOpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCOpenRowset - IScopedOperations::OpenRowset test
//| Created:  	10/8/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIScopedOperations::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cPropertySets != 0, rgPropertySets == NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_1()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;

	// the actual tested operation
	TESTC_(m_hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		1, NULL,
		(IUnknown**)&pIRowset
		), E_INVALIDARG);	
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cPropertySets == 0 and rgPropertySets == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_2()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;

	// the actual tested operation
	TESTC_(m_hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		0, NULL,
		(IUnknown**)&pIRowset
		), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cPRopertySets == 0 and rgPropertySets != NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_3()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	DBPROPSET			rgPropSets[1];
	DBPROP				rgProp[1];

	rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropSets[0].rgProperties		= rgProp;
	rgPropSets[0].cProperties		= 1;
	memset(rgProp, 0, sizeof(DBPROP));
	rgProp[0].dwPropertyID			= DBPROP_IAccessor;
	rgProp[0].vValue.vt				= VT_EMPTY;

	// the actual tested operation
	TESTC_(m_hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		0, rgPropSets,
		(IUnknown**)&pIRowset
		), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cPRopertySets != 0 and rgPropertySets != NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_4()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	DBPROPSET			rgPropSets[1];
	DBPROP				rgProp[1];

	rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropSets[0].rgProperties		= rgProp;
	rgPropSets[0].cProperties		= 1;
	memset(rgProp, 0, sizeof(DBPROP));
	rgProp[0].dwPropertyID			= DBPROP_IAccessor;
	rgProp[0].vValue.vt				= VT_EMPTY;

	// the actual tested operation
	TESTC_(m_hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		1, rgPropSets,
		(IUnknown**)&pIRowset
		), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc inexistent URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_5()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	DBID				TableID;

	TableID.eKind			= DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->GetInexistentURL(&TableID.uName.pwszName), S_OK);

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), DB_E_NOTABLE);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc out of scope URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_6()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	IRowset				*pIRowset			= NULL;
	DBID				TableID;
	WCHAR				*pwszRowURL			= NULL;

	// pick 2 different Collections
	TableID.eKind			= DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->Pick2DifCollections(&pwszRowURL, &TableID.uName.pwszName), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = g_pCTree->OpenRowsetAndCheck(
		pIScopedOperations,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), DB_E_RESOURCEOUTOFSCOPE, S_OK);

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(TableID.uName.pwszName);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc URL in scope
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_7()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	DBID				TableID;

	// pick a row
	TableID.eKind			= DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc NULL ppRowset passed to check properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_8()
{ 
	TBEGIN
	DBID				TableID;
	DBPROPSET			rgPropSets[1];
	DBPROP				rgProp[1];

	// pick a row
	TableID.eKind			= DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	// the actual tested operation
	rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropSets[0].rgProperties		= rgProp;
	rgPropSets[0].cProperties		= 1;
	memset(rgProp, 0, sizeof(DBPROP));
	rgProp[0].dwPropertyID			= DBPROP_IAccessor;
	rgProp[0].vValue.vt				= VT_BOOL;
	V_BOOL(&rgProp[0].vValue)		= VARIANT_TRUE;

	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_IRowset,
		NUMELEM(rgPropSets), rgPropSets,
		NULL), S_OK);

	// try with some bad property, too
	rgPropSets[0].guidPropertySet	= DBPROPSET_TABLE;
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_IRowset,
		NUMELEM(rgPropSets), rgPropSets,
		NULL), DB_E_ERRORSOCCURRED);
	
CLEANUP:
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc not NULL pIndexID => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_9()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	DBID				TableID;
	DBID				IndexID;

	// pick a row
	TableID.eKind			= DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);
	IndexID.eKind			= DBKIND_NAME;
	IndexID.uName.pwszName = L"IndexName";

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, &IndexID, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), E_INVALIDARG);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Check children rowsets of each row in a tree
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_10()
{ 
	TBEGIN
	IRowset		*pIRowset = NULL;
	HRESULT		hr;
	DBID		TableID;
	
	TableID.uName.pwszName = NULL;

	g_pCTree->ResetPosition();
	hr = S_OK;

	// build the table name
	TableID.eKind			= DBKIND_NAME;
	TableID.uName.pwszName = g_pCTree->GetCurrentRowURL();
	for (; S_OK == hr; )
	{
		// the actual tested operation
		TESTC_(hr = g_pCTree->OpenRowsetAndCheck(
			m_pIScopedOperations0,
			NULL, &TableID, NULL, IID_IRowset,
			0, NULL, (IUnknown**)&pIRowset), S_OK);

		SAFE_RELEASE(pIRowset);
		hr = g_pCTree->MoveToNextNode(&TableID.uName.pwszName);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Aggregation, IID_IUnknown asked
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_11()
{ 
	TBEGIN
	CAggregate	Aggregate;
	DBID		TableID;
	HRESULT		hr;

	TableID.eKind = DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	//Try to obtain IID_IUnknown.  
	TESTC_(hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		(IUnknown*)&Aggregate, 
		&TableID, NULL, IID_IUnknown,
		0, NULL, (IUnknown**)&(Aggregate.m_pIUnkInner)), S_OK);
	
	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset));

CLEANUP:
	Aggregate.ReleaseInner();
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Aggregation, ask another interface than IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_12()
{ 
	TBEGIN
	CAggregate	Aggregate;
	IUnknown	*pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error
	DBID		TableID;

	TableID.eKind = DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		(IUnknown*)&Aggregate, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIUnkInner), DB_E_NOAGGREGATION);
	
CLEANUP:
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc No aggregation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_13()
{ 
	TBEGIN
	IRowset		*pIRowset = NULL;
	DBID		TableID;

	TableID.eKind = DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Agg - OpenRowset -> Rowset ->GetReferencedRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_14()
{ 
	TBEGIN
	CAggregate	Aggregate;
	HRESULT		hr;
	IRowsetInfo	*pIRowsetInfo	= NULL;
	IUnknown	*pIUnkOuter		= NULL;
	IUnknown	*pIAggregate	= NULL;
	DBPROPSET	rgPropSet[1];
	DBPROP		rgProp[1];
	DBID		TableID;

	FILL_PROP_SET(rgPropSet[0], 1, rgProp, DBPROPSET_ROWSET);
	FILL_PROP(rgProp[0], DBPROP_BOOKMARKS, VT_BOOL, V_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL);

	TableID.eKind = DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	TEST3C_(hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		(IUnknown*)&Aggregate, 
		&TableID, NULL, IID_IUnknown,
		NUMELEM(rgPropSet), rgPropSet,
		(IUnknown**)&(Aggregate.m_pIUnkInner)), S_OK, DB_S_ERRORSOCCURRED, DB_E_NOAGGREGATION);
	TESTC(DB_S_ERRORSOCCURRED != hr || DBSTATUS_S_OK != rgProp[0].dwStatus);
	
	//Verify Aggregation for this rowset...
	TESTC(Aggregate.VerifyAggregationQI(hr, IID_IRowsetInfo, (IUnknown**)&pIRowsetInfo));

	//Verify we are hooked up...
	//This call we are using the Rowset and asking for IID_IAggregate, 
	//which is the outer object and should succeed!!!  Kind of cool huh!
	hr = pIRowsetInfo->GetReferencedRowset(0, IID_IAggregate, (IUnknown**)&pIAggregate);
	TESTC(hr==S_OK || hr==DB_E_BADORDINAL ||hr==DB_E_NOTAREFERENCECOLUMN);
	if(hr==DB_E_NOTAREFERENCECOLUMN || hr==DB_E_BADORDINAL)
		TESTC(!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, Aggregate.m_pIUnkInner, VARIANT_TRUE));
	// if the result is not S_OK skip variation
	TESTC_PROVIDER(hr==S_OK);	

	//Now make sure the Rowset QI for IUnknown give me the outer
	TESTC_(hr = pIRowsetInfo->GetReferencedRowset(0, IID_IUnknown, (IUnknown**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIUnkOuter));

CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnkOuter);
	Aggregate.ReleaseInner();
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc OpenRowset -> agg IColumnsRowset asking IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_16()
{ 
	TBEGIN
	CAggregate		Aggregate;
	IColumnsRowset	*pIColumnsRowset	= NULL;
	IRowset			*pIRowset			= NULL;
	DBID			TableID;

	TableID.eKind = DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, 
		&TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);

	//Obtain the ColumnsRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIRowset, IID_IColumnsRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));
	
	//Aggregation
	TESTC_(m_hr = pIColumnsRowset->GetColumnsRowset(&Aggregate, 0, NULL, IID_IUnknown, 0, NULL, (IUnknown**)&Aggregate.m_pIUnkInner),S_OK);

	//Verify Aggregation for this session...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(m_hr, IID_IRowset));

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsRowset);
	Aggregate.ReleaseInner();
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc OpenRowset -> agg IColumnsRowset asking non IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_17()
{ 
	TBEGIN
	CAggregate		Aggregate;
	IColumnsRowset	*pIColumnsRowset	= NULL;
	IRowset			*pIRowset			= NULL;
	DBID			TableID;

	TableID.eKind = DBKIND_NAME;
	TESTC_(m_hr = g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);

	//Obtain the ColumnsRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIRowset, IID_IColumnsRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));
	
	//Aggregation
	TESTC_(m_hr = pIColumnsRowset->GetColumnsRowset(&Aggregate, 0, NULL, IID_IOpenRowset, 0, NULL, (IUnknown**)&Aggregate.m_pIUnkInner), DB_E_NOAGGREGATION);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIColumnsRowset);
	Aggregate.ReleaseInner();
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc NULL pTableID => children rowset of the current row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_19()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	WCHAR				*pwszRowURL			= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;

	// pick a random Collection
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszRowURL), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		pIScopedOperations,
		NULL, NULL, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc NULL pTableID.uName.pwszName => children rowset of the current row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_20()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	WCHAR				*pwszRowURL			= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;
	DBID				TableID;

	// pick a random Collection
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszRowURL), S_OK);
	
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = NULL;
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		pIScopedOperations,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);

	TESTC(g_pCTree->CheckRowset(pIRowset, pwszRowURL));
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc empty table name => children rowset of the current row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_21()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	WCHAR				*pwszRowURL			= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;
	DBID				TableID;

	// pick a random Collection
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszRowURL), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = L"";
	TESTC_(m_hr = g_pCTree->OpenRowsetAndCheck(
		pIScopedOperations,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), S_OK);

	TESTC(g_pCTree->CheckRowset(pIRowset, pwszRowURL));

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Check all rowset interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_22()
{ 
	TBEGIN
	ULONG				i, cProp;
	ULONG				cInterfaces				= 0;
	INTERFACEMAP		*rgInterfaces			= NULL;
	IUnknown			*pIUnknown				= NULL;
	IUnknown			*pIU2					= NULL;
	ULONG				ulOrgRefCount			= 0;
	BOOL				bReturn					= FALSE;
	ULONG				cPropertySets			= 0;
	DBPROPSET			*rgPropertySets			= NULL;
	
	ULONG				cOriginalPropertySets	= 0;
	DBPROPSET			*rgOriginalPropertySets	= NULL;
	IScopedOperations	*pIScopedOperations		= NULL;
	HRESULT				hr;
	IGetSession			*pIGetSession			= NULL;
	IGetDataSource		*pIGetDataSource		= NULL;
	IDBProperties		*pIDBProperties			= NULL;
	DBID				TableID;

	//Obtain a list of interfaces for this Object
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cInterfaces, &rgInterfaces));
	
	// set all the properties related to interfaces
	for (i=0; i< cInterfaces; i++)
	{
		SetProperty(rgInterfaces[i].dwPropertyID, DBPROPSET_ROWSET, 
			&cOriginalPropertySets, &rgOriginalPropertySets, 
			DBTYPE_BOOL, VARIANT_TRUE, DBPROPOPTIONS_OPTIONAL);
	}

	TESTC_(g_pIBindResource->Bind(NULL, m_pwszRootURL0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	TESTC(VerifyInterface(pIScopedOperations, IID_IGetSession,
		ROWSET_INTERFACE, (IUnknown**)&pIGetSession));
	TESTC_(pIGetSession->GetSession(IID_IGetDataSource, 
		(IUnknown**)&pIGetDataSource), S_OK);
	TESTC_(pIGetDataSource->GetDataSource(IID_IDBProperties,
		(IUnknown**)&pIDBProperties), S_OK);

	TableID.eKind			= DBKIND_NAME;
	TableID.uName.pwszName	= NULL;

	for (i=0; i< cInterfaces; i++)
	{
		odtLog << "\t" << rgInterfaces[i].pwszName << "\n";

		FreeProperties(&cPropertySets, &rgPropertySets);
		CHECK(0 == cOriginalPropertySets || NULL == rgOriginalPropertySets 
			|| DuplicatePropertySets(cOriginalPropertySets, rgOriginalPropertySets, 
			&cPropertySets, &rgPropertySets), TRUE);

		SAFE_FREE(TableID.uName.pwszName);
		TESTC_(g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

		// the actual tested operation
		hr = g_pCTree->OpenRowsetAndCheck(
			pIScopedOperations,
			NULL, &TableID, NULL, *rgInterfaces[i].pIID,
			cPropertySets, rgPropertySets,
			(IUnknown**)&pIUnknown);

		if (rgInterfaces[i].fMandatory && DB_S_ERRORSOCCURRED != hr)
			CHECK(hr, S_OK);

		if (SettableProperty(rgInterfaces[i].dwPropertyID,
			DBPROPSET_ROWSET, pIDBProperties, ROWSET_INTERFACE))
		{
			COMPARE(DB_S_ERRORSOCCURRED == hr || CHECK(hr, S_OK), TRUE);
		}

		if (!SupportedProperty(rgInterfaces[i].dwPropertyID,
			DBPROPSET_ROWSET, pIDBProperties, ROWSET_INTERFACE))
		{
			CHECK(hr, E_NOINTERFACE);
		}

		if (S_OK != hr && E_NOINTERFACE != hr)
			CHECK(hr, DB_S_ERRORSOCCURRED);

		if (!COMPARE(IsPropSetPreserved(rgOriginalPropertySets, rgPropertySets, cPropertySets), TRUE))
		{
			odtLog << "\tERROR: the rowset property sets are not preserved\n";
		}

		if (SUCCEEDED(hr))
			COMPARE(rgPropertySets[0].rgProperties[i].dwStatus, DBPROPSTATUS_OK);

		for (cProp=0; cProp<cPropertySets; cProp++)
		{
			if (rgInterfaces[cProp].fMandatory)
			{
				COMPARE(rgPropertySets[0].rgProperties[cProp].dwStatus, DBPROPSTATUS_OK);

				COMPARE(VerifyInterface(pIUnknown, *rgInterfaces[cProp].pIID, 
					ROWSET_INTERFACE, &pIU2) || NULL == pIU2, TRUE);
			}
			else if (DBPROPSTATUS_OK == rgPropertySets[0].rgProperties[cProp].dwStatus)
			{
				COMPARE(VerifyInterface(pIUnknown, *rgInterfaces[cProp].pIID, 
					ROWSET_INTERFACE, &pIU2) || NULL == pIU2, TRUE);
			}
			else
			{
				if (DBPROPSTATUS_NOTSETTABLE != rgPropertySets[0].rgProperties[cProp].dwStatus)
					COMPARE(rgPropertySets[0].rgProperties[cProp].dwStatus, DBPROPSTATUS_NOTSUPPORTED);

				pIU2 = INVALID(IUnknown*);
				COMPARE(VerifyInterface(pIUnknown, *rgInterfaces[cProp].pIID, 
					ROWSET_INTERFACE, &pIU2) || NULL == pIU2, TRUE);
			}
			SAFE_RELEASE(pIU2);
		}

		SAFE_RELEASE(pIUnknown);
	}

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIU2);
	FreeProperties(&cPropertySets, &rgPropertySets);
	FreeProperties(&cOriginalPropertySets, &rgOriginalPropertySets);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIGetSession);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBProperties);
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc cProperties != 0 in an element of rgPropertySets while rgProperties == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_24()
{ 
	TBEGIN
	IRowset				*pIRowset = NULL;
	HRESULT				hr;
	DBPROPSET			rgPropSets[1];

	rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropSets[0].rgProperties		= NULL;
	rgPropSets[0].cProperties		= 1;

	// the actual tested operation
	TESTC_(hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		1, rgPropSets,
		(IUnknown**)&pIRowset
		), E_INVALIDARG);	
	TESTC(NULL == pIRowset);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc cProperties == 0 in an element of rgPropertySets while rgProperties != NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_25()
{ 
	TBEGIN
	IRowset				*pIRowset = NULL;
	HRESULT				hr;
	DBPROPSET			rgPropSets[1];
	DBPROP				rgProp[1];

	rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropSets[0].rgProperties		= rgProp;
	rgPropSets[0].cProperties		= 0;
	memset(rgProp, 0, sizeof(DBPROP));
	rgProp[0].dwPropertyID			= DBPROP_IAccessor;
	rgProp[0].vValue.vt				= VT_EMPTY;

	// the actual tested operation
	TESTC_(hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		0, rgPropSets,
		(IUnknown**)&pIRowset
		), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc cProperties == 0 in an element of rgPropertySets and rgProperties == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_26()
{ 
	TBEGIN
	IRowset				*pIRowset = NULL;
	HRESULT				hr;
	DBPROPSET			rgPropSets[1];

	rgPropSets[0].guidPropertySet	= DBPROPSET_ROWSET;
	rgPropSets[0].rgProperties		= NULL;
	rgPropSets[0].cProperties		= 0;

	// the actual tested operation
	TESTC_(hr = m_pIScopedOperations0->OpenRowset(
		NULL, NULL, NULL, IID_IRowset,
		0, rgPropSets,
		(IUnknown**)&pIRowset
		), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc pTableID.eKind != DBKIND_NAME => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_27()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	WCHAR				*pwszRowURL			= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;
	DBID				TableID;

	// pick a random Collection
	TESTC_(g_pCTree->PickACollection(&pwszRowURL), S_OK);

	TESTC_(g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TableID.eKind = DBKIND_GUID;
	TableID.uName.pwszName = L"alfa";
	TESTC_(g_pCTree->OpenRowsetAndCheck(
		pIScopedOperations,
		NULL, &TableID, NULL, IID_IRowset,
		0, NULL, (IUnknown**)&pIRowset), E_INVALIDARG);
	
	TESTC(NULL == pIRowset);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc riid == IID_NULL -> E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_28()
{ 
	TBEGIN
	IRowset				*pIRowset			= NULL;
	DBID				TableID;

	// pick a row
	TableID.eKind			= DBKIND_NAME;
	TESTC_(g_pCTree->PickACollection(&TableID.uName.pwszName), S_OK);

	// the actual tested operation
	TESTC_(g_pCTree->OpenRowsetAndCheck(
		m_pIScopedOperations0,
		NULL, &TableID, NULL, IID_NULL,
		0, NULL, (IUnknown**)&pIRowset), E_NOINTERFACE);
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(TableID.uName.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCOpenRowset::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIScopedOperations::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END






// {{ TCW_TC_PROTOTYPE(TCCopy)
//*-----------------------------------------------------------------------
//| Test Case:		TCCopy - IScopedOperations::Copy()
//| Created:  	10/10/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCopy::Init()
{ 
	TBEGIN

	// {{ TCW_INIT_BASECLASS_CHECK
	TESTC(TCIScopedOperations::Init());
	// }}

CLEANUP:
	TRETURN
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRows == 0 => the tree is preserved
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_1()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszRowURL			= NULL;
	WCHAR				wszString[]			= L"name";
	WCHAR				*rgpwszDest[1]		= {wszString};
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;
	DBSTATUS			rgdwStatus[1];

	TESTC_(g_pCTree->PickANode(&pwszRowURL), S_OK);

	TESTC_(g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(pIScopedOperations->Copy(0, (const WCHAR**)rgpwszDest, 
		(const WCHAR**)rgpwszDest, 0, NULL, 
		rgdwStatus, rgpwszNewURLs, &pwszStringsBuffer), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRows > 0 and rgpwszSourceURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_2()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszRowURL			= NULL;
	WCHAR				wszString[]			= L"name";
	WCHAR				*rgpwszDest[1]		= {wszString};
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;
	DBSTATUS			rgdwStatus[1];

	TESTC_(g_pCTree->PickANode(&pwszRowURL), S_OK);

	TESTC_(g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(pIScopedOperations->Copy(1, NULL, (const WCHAR**)rgpwszDest, 
		0, NULL, rgdwStatus, rgpwszNewURLs, &pwszStringsBuffer), E_INVALIDARG);	

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRows>0 and rgpwszDestURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_3()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszRowURL			= NULL;
	WCHAR				wszString[]			= L"name";
	WCHAR				*rgpwszSource[1]	= {wszString};
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;
	DBSTATUS			rgdwStatus[1];

	TESTC_(m_hr = g_pCTree->PickANode(&pwszRowURL), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Copy(1, (const WCHAR**)rgpwszSource, 
		NULL, 0, NULL, rgdwStatus, rgpwszNewURLs, &pwszStringsBuffer), E_INVALIDARG);	

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRows>0 and rgdwStatus is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_4()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszRowURL			= NULL;
	WCHAR				wszString[]			= L"name";
	WCHAR				*rgpwszSource[1]	= {wszString};
	WCHAR				*rgpwszDest[1]		= {wszString};
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszRowURL), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Copy(1, (const WCHAR**)rgpwszSource, 
		(const WCHAR**)rgpwszDest, 0, NULL, 
		NULL, rgpwszNewURLs, &pwszStringsBuffer), E_INVALIDARG);	

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszRowURL);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc rgpwszNewURLs and ppStringsBuffer are NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_5()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc NULL rgpwszNewURLs and ppStringsBuffer is not NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_6()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pStringsBuffer	= NULL;
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, &pStringsBuffer), E_INVALIDARG);	
	TESTC(NULL == pStringsBuffer);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc not NULL rgpwszNewURLs and NULL ppwszStringsBuffer
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_7()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, rgpwszNewURLs, NULL), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc not NULL rgpwszNewURLs and not NULL ppwszStringsBuffer
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_8()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	WCHAR				*rgpwszNewURLs[2];
	DBSTATUS			rgdwStatus[2];
	WCHAR				*pStringsBuffer		= NULL;
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 4 nodes
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(0, rgpwszSourceURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(1, &rgpwszSourceURLs[1])));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(2, rgpwszDestURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(3, &rgpwszDestURLs[1])));

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, rgpwszNewURLs, &pStringsBuffer), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[1]);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Copy a subtree to another subtree
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_9()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszNewURL			= NULL;
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 different subtrees
	TESTC_(m_hr = pCTree->Pick2Nodes(NC_SUBTREE, rgpwszSourceURLs, 
		NC_SUBTREE, rgpwszDestURLs, PCO_DIFF), S_OK);

	// try to copy to an existing subtree without specifying replace
	odtLog << "without using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// try to copy to an existing subtree specifying replace
	odtLog << "using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	// copy a subtree to a new node
	SAFE_FREE(rgpwszDestURLs[0]);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0],
		NC_SUBTREE, &pwszNewURL), S_OK);
	TESTC(pCTree->MakeSuffix(L"33", pwszNewURL, rgpwszDestURLs));

	odtLog << "create new subtree\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszNewURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Copy a leaf to another one
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_10()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszNewURL = NULL;
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 leaves
	TESTC_(m_hr = pCTree->Pick2Nodes(NC_LEAF, rgpwszSourceURLs, 
		NC_LEAF, rgpwszDestURLs, PCO_DIFF), S_OK);

	// the actual tested operation
	odtLog << "using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	odtLog << "without using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// to a new row
	odtLog << "copy to a new row\n";
	SAFE_FREE(rgpwszDestURLs[0]);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], NC_LEAF, &pwszNewURL), S_OK);
	TESTC(pCTree->MakeSuffix(L"35", pwszNewURL, rgpwszDestURLs));

	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszNewURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Copy a subtree instead of a leaf
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_11()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick a subtree and a leaf
	TESTC_(m_hr = pCTree->Pick2Nodes(NC_SUBTREE, rgpwszSourceURLs, 
		NC_LEAF, rgpwszDestURLs, PCO_DIFF), S_OK);


	// the actual tested operation
	odtLog << "using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	odtLog << "without using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Copy a leaf instead of a subtree
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_12()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_LEAF, rgpwszSourceURLs, 
		NC_SUBTREE, rgpwszDestURLs, PCO_DIFF), S_OK);

	// the actual tested operation
	odtLog << "using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	odtLog << "without using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Copy nerecursive a subtree to a leaf
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_13()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1]={NULL};
	WCHAR				*pwszDestURL		= NULL;
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszNewURL			= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick a subtree and a leaf
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_SUBTREE, rgpwszSourceURLs, 
		NC_LEAF, rgpwszDestURLs, PCO_DIFF), S_OK);

	// the actual tested operation
	odtLog << "using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING|DBCOPY_NON_RECURSIVE, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	odtLog << "without using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_NON_RECURSIVE, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// check the case when the destination doesn't exist yet
	odtLog << "Dest is to be built\n";
	SAFE_FREE(rgpwszDestURLs[0]);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], NC_LEAF, &pwszNewURL), S_OK);
	TESTC(pCTree->MakeSuffix(L"37", pwszNewURL, rgpwszDestURLs));
	
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_NON_RECURSIVE, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszNewURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Copy nerecursive a subtree to a subtree
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_14()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszNewURL			= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 different subtrees
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_SUBTREE, rgpwszSourceURLs, 
		NC_SUBTREE, rgpwszDestURLs, PCO_DIFF), S_OK);

	// the actual tested operation
	odtLog << "using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING|DBCOPY_NON_RECURSIVE, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	odtLog << "without using DBCOPY_REPLACE_EXISTING\n";
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_NON_RECURSIVE, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// creating a brad new subtree
	odtLog << "new subtree\n";
	SAFE_FREE(rgpwszDestURLs[0]);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], NC_LEAF, &pwszNewURL), S_OK);
	TESTC(pCTree->MakeSuffix(L"40", pwszNewURL, rgpwszDestURLs));

	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_NON_RECURSIVE, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszNewURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Inexistent URL passed as source
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_15()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick inexistent node and random node
	TESTC_(m_hr = pCTree->PickANode(rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(rgpwszSourceURLs, rgpwszDestURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Invalid destination URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_16()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick source row and invalid destination
	TESTC_(m_hr = pCTree->PickANode(rgpwszSourceURLs), S_OK);
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(rgpwszDestURLs, rgpwszSourceURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Source outside current scope
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_17()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL			= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick 2 overlapping nodes (for the scope)
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(&pwszRowURL, rgpwszDestURLs), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// pick an out of scope source
	TESTC_(m_hr = pCTree->PickASecondNode(pwszRowURL, NC_NONE, rgpwszSourceURLs), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->CopyAndCheck(pIScopedOperations, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK, DB_E_ERRORSOCCURRED);	
	if (S_OK != m_hr)
	{
		TESTC(DBSTATUS_E_RESOURCEOUTOFSCOPE == rgdwStatus[0]);
	}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	SAFE_RELEASE(pIScopedOperations);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Destination outside current scope
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_18()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL			= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 overlapping nodes: scope and source
	TESTC_(m_hr = pCTree->Pick2DifNodes(&pwszRowURL, rgpwszSourceURLs), S_OK);

	// pick a row to give the scope and source within this scope
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// pick destination (outside of scope)
	TESTC_(m_hr = pCTree->PickASecondNode(pwszRowURL, NC_NONE, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->CopyAndCheck(pIScopedOperations, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED, S_OK);	
	if (S_OK != m_hr)
	{
		TESTC(DBSTATUS_E_RESOURCEOUTOFSCOPE == rgdwStatus[0]);
	}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	SAFE_RELEASE(pIScopedOperations);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Source in the scope of destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_19()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick overlapping rows as source and destination
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(rgpwszDestURLs, rgpwszSourceURLs), S_OK);

	// check that operation is rejected
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	

	TESTC(DBSTATUS_E_CANNOTCOMPLETE == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Destination in the scope of source
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_20()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick destination in the scope of the source
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// check operation fails
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	

	TESTC(DBSTATUS_E_CANNOTCOMPLETE == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc atomic ops: one op fails
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_21()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 different nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	rgpwszSourceURLs[1]	= rgpwszSourceURLs[0];
	
	// generate a bad destination URL
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszDestURLs[1], rgpwszSourceURLs[1]), S_OK);

	TEST2C_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING | DBCOPY_ATOMIC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED, E_INVALIDARG);
	
	if (DB_E_ERRORSOCCURRED == m_hr)
	{
		TESTC(DBSTATUS_S_OK == rgdwStatus[0] 
			||	DBSTATUS_E_UNAVAILABLE == rgdwStatus[0]);
		TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);
	}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	// do not release rgpwszSourceURLs[1], points to the same string as rgpwszSourceURLs[0]
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc atomic ops: all ops fail
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_22()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 different nodes
	TESTC_(m_hr = pCTree->Pick2Nodes(&rgpwszSourceURLs[1], &rgpwszDestURLs[0]), S_OK);
	
	// generate a bad source for the first pair and a bad destination for the second one 
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszSourceURLs[0], rgpwszDestURLs[0]), S_OK);
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(&rgpwszDestURLs[1], rgpwszSourceURLs[1]), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 
		DBCOPY_REPLACE_EXISTING | DBCOPY_ATOMIC, 
		NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED, E_INVALIDARG);
	
	if (DB_E_ERRORSOCCURRED == m_hr)
	{
		TESTC(	DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0] 
			||  DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]); 
		//optim could result in abandoning the second op
	}
	
CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc atomic ops: all ops pass
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_23()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 valid source-destination pairs
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszSourceURLs[1], &rgpwszDestURLs[1]), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 
		DBCOPY_REPLACE_EXISTING | DBCOPY_ATOMIC, 
		NULL, 
		rgdwStatus, NULL, NULL), S_OK, E_INVALIDARG);
	
	if (S_OK == m_hr)
	{
		TESTC(DBSTATUS_S_OK == rgdwStatus[0]);
		TESTC(DBSTATUS_S_OK == rgdwStatus[1]);
	}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc non atomic, all pass
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_24()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 valid source-destination pairs
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(0, rgpwszSourceURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(1, &rgpwszSourceURLs[1])));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(2, rgpwszDestURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(3, &rgpwszDestURLs[1])));

	// the actual tested operation
	TEST2C_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 
		DBCOPY_REPLACE_EXISTING, 
		NULL, 
		rgdwStatus, NULL, NULL), S_OK, E_INVALIDARG);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc non atomic, one op fail
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_25()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	rgpwszSourceURLs[1]	= rgpwszSourceURLs[0];

	// generate a bad destination URL
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(&rgpwszDestURLs[1], rgpwszSourceURLs[1]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 
		DBCOPY_REPLACE_EXISTING, 
		NULL, 
		rgdwStatus, NULL, NULL), DB_S_ERRORSOCCURRED);	
	TESTC(DBSTATUS_S_OK == rgdwStatus[0]);
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	// do not release rgpwszSourceURLs[1], points to the same string as rgpwszSourceURLs[0]
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc non atomic, all ops fail
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_26()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, &rgpwszDestURLs[1]), S_OK);

	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszSourceURLs[1], rgpwszDestURLs[1]), S_OK);
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(&rgpwszDestURLs[0], rgpwszSourceURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 
		DBCOPY_REPLACE_EXISTING, 
		NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc no replace flag, existing destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_27()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc no replace flag, no existing destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_28()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->PickANode(rgpwszSourceURLs), S_OK);
	
	// parent of destination is inexistent (pass the excluded path)
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(rgpwszDestURLs, rgpwszSourceURLs[0]), S_OK);
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

	// destination doesn't exist, but parent of destination exists
	SAFE_FREE(rgpwszDestURLs[0]);
	TESTC_(m_hr = pCTree->GetInexistentURL(rgpwszDestURLs, rgpwszSourceURLs[0]), S_OK);
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), 
		(DBPROPVAL_GU_NOTSUPPORTED == pCTree->GetGenerateURL())? 
		S_OK : DB_E_ERRORSOCCURRED);	
	if (DBPROPVAL_GU_NOTSUPPORTED == pCTree->GetGenerateURL())
	{TESTC(DBSTATUS_S_OK == rgdwStatus[0]);}
	else
	{TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc replace, existing destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_29()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc replace, no existing destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_30()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	TESTC_(m_hr = pCTree->PickANode(rgpwszSourceURLs), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(rgpwszDestURLs, rgpwszSourceURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc generate destination URLs and check them
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_31()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];
	WCHAR				*rgpwszNewURLs[2];
	WCHAR				*pStringsBuffer = NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(0, rgpwszSourceURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(2, &rgpwszSourceURLs[1])));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(1, rgpwszDestURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(3, &rgpwszDestURLs[1])));

	// the actual tested operation
	// checking of the generated URLs is automatically done in the helper function
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 
		DBCOPY_REPLACE_EXISTING, 
		NULL, 
		rgdwStatus, rgpwszNewURLs, &pStringsBuffer), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Inexistent flag => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_32()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	DWORD				dwFlags;
	DWORD				dwOldFlags;
	DWORD				dwTestFlag;
	// build the list of invalid flags mask
	const DWORD			dwInvalidFlags = 0xFFFFFFFF ^ 
									(		DBCOPY_ASYNC
										|	DBCOPY_REPLACE_EXISTING
										|	DBCOPY_ALLOW_EMULATION
										|	DBCOPY_NON_RECURSIVE
										|	DBCOPY_ATOMIC
									);

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	for (dwFlags = dwInvalidFlags; dwFlags;)
	{
		dwOldFlags	= dwFlags;
		dwFlags		= dwFlags & (dwFlags-1);
		dwTestFlag	= dwFlags ^ dwOldFlags;

		TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
			rgpwszDestURLs, dwTestFlag, NULL, 
			rgdwStatus, NULL, NULL), E_INVALIDARG);	
	}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc element of rgpwszSourceURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_33()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], NC_NONE, 
		&rgpwszDestURLs[1]), S_OK);
	rgpwszSourceURLs[1] = NULL;

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	// do not release rgpwszSourceURLs[1]
	// rgpwszSourceURLs[1] and rgpwszSourceURLs[0] points to the same el
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc element of rgpwszDestURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_34()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	rgpwszSourceURLs[1] = rgpwszSourceURLs[0];
	rgpwszDestURLs[1] = NULL;

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Async operation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_35()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	IScopedOperations	*pIScopedOperations = NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// bind to a row
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TEST2C_(m_hr = pIScopedOperations->Copy(1, (const WCHAR**)rgpwszSourceURLs, 
		(const WCHAR**)rgpwszDestURLs, DBCOPY_REPLACE_EXISTING | DBCOPY_ASYNC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ASYNCNOTSUPPORTED, DB_S_ASYNCHRONOUS);
	
CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_RELEASE(pIScopedOperations);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Try to cancel asynch op befor returning
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCopy::Variation_36()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	IScopedOperations	*pIScopedOperations = NULL;
	IDBAsynchStatus		*pIDBAsynchStatus	= NULL;
	HANDLE				hThread;
	unsigned			IDThread;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// bind to a row
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	TESTC_PROVIDER(VerifyInterface(pIScopedOperations, IID_IDBAsynchStatus, ROW_INTERFACE, (IUnknown**)&pIDBAsynchStatus));
	hThread = (HANDLE)_beginthreadex(NULL, 0, CancelProc,
					(void*)pIDBAsynchStatus,
					0, 
					&IDThread);
	TESTC(0 != hThread);

	// the actual tested operation
	TEST3C_(m_hr = pIScopedOperations->Copy(1, (const WCHAR**)rgpwszSourceURLs, 
		(const WCHAR**)rgpwszDestURLs, DBCOPY_REPLACE_EXISTING | DBCOPY_ASYNC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ASYNCNOTSUPPORTED, DB_S_ASYNCHRONOUS, DB_E_CANCELED);

CLEANUP:
	COMPARE(WAIT_OBJECT_0 == WaitForSingleObject(hThread, INFINITE), TRUE);
	if (0 != hThread)
	{
		COMPARE(CloseHandle(hThread), TRUE);
	}

	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIDBAsynchStatus);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCopy::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return TCIScopedOperations::Terminate();
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


/*
// Hack to defile CoInitializeEx
//WINOLEAPI  CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit);
unsigned WINAPI ThreadProc(LPVOID lpvThreadParam)
{
	CInParam	*pThreadParam = (CInParam*)lpvThreadParam;

	CoInitializeEx(NULL, 0);
	TCIScopedOperations	*pObject = pThreadParam->pObject;
	pObject->MyThreadProc(pThreadParam->enMethodCalled, pThreadParam->pParam);
	CoUninitialize();
	return 0;
} //ThreadProc
*/
// {{ TCW_TC_PROTOTYPE(TCDelete)
//*-----------------------------------------------------------------------
//| Test Case:		TCDelete - IScopedOperations::Delete()
//| Created:  	10/20/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDelete::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIScopedOperations::Init())
	// }}
	{ 
		// make sure copying is allowed (g_pwszRootRow is not NULL)
		return NULL != g_pwszRootRow;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRows is 0 => no operation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_1()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszURL			= NULL;
	WCHAR				*rgpwszURL[1];
	DBSTATUS			rgdwStatus[1];

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
	rgpwszURL[0] = pwszURL;

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Delete(0, (const WCHAR**)rgpwszURL, 
		0, rgdwStatus), S_OK);	
	
	// make sure the current node was not deleted (cRows == 0 should be nop)
	SAFE_RELEASE(pIScopedOperations);
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRows>0, rgpwszURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_2()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	DBSTATUS			rgdwStatus[1];

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Delete(1, NULL, 
		0, rgdwStatus), E_INVALIDARG);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRows>0, rgdwStatus is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_3()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszURL			= NULL;
	WCHAR				*rgpwszURL[1];

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
	rgpwszURL[0] = pwszURL;

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Delete(1, (const WCHAR**)rgpwszURL, 
		0, NULL), E_INVALIDARG);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Inexistent URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_4()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*pwszURL			= NULL;
	WCHAR				*rgpwszURL[1];
	DBSTATUS			rgdwStatus[1];

	TESTC_(m_hr = g_pCTree->GetInexistentURL(&pwszURL), S_OK);
	rgpwszURL[0] = pwszURL;

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Delete(1, (const WCHAR**)rgpwszURL, 
		0, rgdwStatus), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc URL outside of current scope
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_5()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations		= NULL;
	IScopedOperations	*pIScopedOperations2	= NULL;
	WCHAR				*rgpwszURL[2]			= {NULL, NULL};
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszURL = NULL;
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(&pwszURL, &rgpwszURL[0]), S_OK);
	
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// try to delete something outside the scope
	TEST2C_(m_hr = pIScopedOperations->Delete(1, (const WCHAR**)rgpwszURL, 
		0, rgdwStatus), DB_E_ERRORSOCCURRED, S_OK);	
	TESTC(S_OK == m_hr || DBSTATUS_E_RESOURCEOUTOFSCOPE == rgdwStatus[0]);

	if (S_OK == m_hr)
	{
		TESTC_(m_hr = g_pIBindResource->Bind(NULL, rgpwszURL[0], 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations2), DB_E_NOTFOUND);
	}
	else
	{
		TESTC_(m_hr = g_pIBindResource->Bind(NULL, rgpwszURL[0], 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations2), S_OK);
	}

	// try to delete something outside the scope and from the same path
	rgpwszURL[1] = wcsDuplicate(pwszURL);
	TESTC_(m_hr = pCTree->TrimLeafToSubtree(rgpwszURL[1]), S_OK);
	TEST2C_(m_hr = pIScopedOperations->Delete(1, (const WCHAR**)&rgpwszURL[1], 
		0, rgdwStatus), DB_E_ERRORSOCCURRED, S_OK);	
	TESTC(S_OK == m_hr || DBSTATUS_E_RESOURCEOUTOFSCOPE == rgdwStatus[0]);

	SAFE_RELEASE(pIScopedOperations2);
	if (S_OK == m_hr)
	{
		TESTC_(m_hr = g_pIBindResource->Bind(NULL, rgpwszURL[1], 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations2), DB_E_NOTFOUND);
	}
	else
	{
		TESTC_(m_hr = g_pIBindResource->Bind(NULL, rgpwszURL[1], 
			DBBINDURLFLAG_READ, DBGUID_ROW, 
			IID_IScopedOperations, NULL, NULL, NULL, 
			(IUnknown**)&pIScopedOperations2), S_OK);
	}

CLEANUP:
	delete pCTree;
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIScopedOperations2);
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Delete a leaf node
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_6()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick up a leaf row
	TESTC_(m_hr = pCTree->PickALeaf(rgpwszURL), S_OK);
	
	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, rgpwszURL, 
		0, rgdwStatus), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Delete a subtree
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_7()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick up a subtree row
	TESTC_(m_hr = pCTree->PickASubtree(rgpwszURL), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, rgpwszURL, 
		0, rgdwStatus), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Non atomic, all ops pass
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_8()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[2] = {NULL, NULL};
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick up a 2 rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszURL[0], &rgpwszURL[1]), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 2, rgpwszURL, 
		0, rgdwStatus), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Non atomic, one op fails, one passed
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_9()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[3] = {NULL, NULL, NULL};
	DBSTATUS			rgdwStatus[3];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// select the delete targets
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszURL[0], &rgpwszURL[2]), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszURL[1]), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 3, rgpwszURL, 
		0, rgdwStatus), DB_S_ERRORSOCCURRED);
	TESTC(DBSTATUS_S_OK == rgdwStatus[0]);
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);
	TESTC(DBSTATUS_S_OK == rgdwStatus[2]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	SAFE_FREE(rgpwszURL[2]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Non atomic, all ops fail
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_10()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// get 2 inexistent URLs
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszURL[0]), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszURL[1]), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 2, rgpwszURL, 
		0, rgdwStatus), DB_E_ERRORSOCCURRED);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Atomic, all ops pass
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_11()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick up a 2 rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszURL[0], &rgpwszURL[1]), S_OK);

	// delete row
	TEST2C_(m_hr = pCTree->DeleteAndCheck(NULL, 2, rgpwszURL, 
		DBDELETE_ATOMIC, rgdwStatus), S_OK, E_INVALIDARG);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Atomic, on op passes, one fails
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_12()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[3];
	DBSTATUS			rgdwStatus[3];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick up a 2 rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszURL[0], &rgpwszURL[2]), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszURL[1]), S_OK);

	// delete row
	TEST2C_(m_hr = pCTree->DeleteAndCheck(NULL, 3, rgpwszURL, 
		DBDELETE_ATOMIC, rgdwStatus), DB_E_ERRORSOCCURRED, E_INVALIDARG);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	SAFE_FREE(rgpwszURL[2]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Atomic, all ops fail
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_13()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick up a 2 inexistent URLs
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszURL[0]), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszURL[1]), S_OK);

	// delete row
	TEST2C_(m_hr = pCTree->DeleteAndCheck(NULL, 2, rgpwszURL, 
		DBDELETE_ATOMIC, rgdwStatus), DB_E_ERRORSOCCURRED, E_INVALIDARG);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Delete a row twice
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_14()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick a row
	TESTC_(m_hr = pCTree->PickANode(rgpwszURL), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, rgpwszURL, 
		0, rgdwStatus), S_OK);	
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, rgpwszURL, 
		0, rgdwStatus), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Try to delete a row from a subtree that is already been deleted
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_15()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[2];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 overlapping nodes (the second one is a descendent of the first)
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(&rgpwszURL[0], &rgpwszURL[1]), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, &rgpwszURL[0], 
		0, rgdwStatus), S_OK);	
	
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, &rgpwszURL[1], 
		0, rgdwStatus), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	SAFE_FREE(rgpwszURL[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Delete an inexistent row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_16()
{ 
	TBEGIN
	WCHAR				*rgpwszURL[1];
	DBSTATUS			rgdwStatus[1];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick an inexistent URL
	TESTC_(m_hr = pCTree->GetInexistentURL(rgpwszURL), S_OK);

	// delete row
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, rgpwszURL, 
		0, rgdwStatus), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURL[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Copy a row to another one and then try to delete the source and the destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_17()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszSourceURLs[0], &rgpwszDestURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->CopyAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	// now delete them
	rgpwszURLs[0] = rgpwszSourceURLs[0];
	rgpwszURLs[1] = rgpwszDestURLs[0];
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 2, rgpwszURLs, 
		0, rgdwStatus), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Move a row to another one and then try to delete the source and the destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_18()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszURLs[2];
	DBSTATUS			rgdwStatus[2];
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(&rgpwszSourceURLs[0], &rgpwszDestURLs[0]), S_OK);

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	// now delete them
	rgpwszURLs[0] = rgpwszSourceURLs[0];
	rgpwszURLs[1] = rgpwszDestURLs[0];
	TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 2, rgpwszURLs, 
		0, rgdwStatus), DB_S_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]); 
	TESTC(DBSTATUS_S_OK == rgdwStatus[1]); 

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Inexistent flag => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_19()
{ 
	TBEGIN
	WCHAR				*rgpwszURLs[1];
	DBSTATUS			rgdwStatus[1];
	DWORD				dwFlags;
	DWORD				dwOldFlags;
	DWORD				dwTestFlag;
	// build the list of invalid flags mask
	const DWORD			dwInvalidFlags = 0xFFFFFFFF ^ 
									(DBDELETE_ASYNC|DBDELETE_ATOMIC);
	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick a row
	TESTC_(m_hr = pCTree->PickANode(rgpwszURLs), S_OK);

	// now delete them
	for (dwFlags = dwInvalidFlags; dwFlags;)
	{
		dwOldFlags	= dwFlags;
		dwFlags		= dwFlags & (dwFlags-1);
		dwTestFlag	= dwFlags ^ dwOldFlags;

		TESTC_(m_hr = pCTree->DeleteAndCheck(NULL, 1, rgpwszURLs, 
			dwTestFlag, rgdwStatus, FALSE), E_INVALIDARG);	
	}

	TESTC(pCTree->CheckTree());

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Async operation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_20()
{ 
	TBEGIN
	WCHAR				*rgpwszURLs[2];
	DBSTATUS			rgdwStatus[2];
	IScopedOperations	*pIScopedOperations = NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszURLs, &rgpwszURLs[1]), S_OK);

	// bind to a row
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TEST2C_(m_hr = pIScopedOperations->Delete(NUMELEM(rgpwszURLs), (const WCHAR**)rgpwszURLs, 
		DBCOPY_REPLACE_EXISTING | DBCOPY_ASYNC, rgdwStatus), DB_E_ASYNCNOTSUPPORTED, DB_S_ASYNCHRONOUS);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszURLs[0]);
	SAFE_FREE(rgpwszURLs[1]);
	SAFE_RELEASE(pIScopedOperations);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Try to cancel the asynch operation before the call completes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCDelete::Variation_21()
{ 
	TBEGIN
	WCHAR				*rgpwszURLs[2];
	DBSTATUS			rgdwStatus[2];
	IScopedOperations	*pIScopedOperations = NULL;
	IDBAsynchStatus		*pIDBAsynchStatus	= NULL;
	HANDLE				hThread;
	unsigned			IDThread;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszURLs, &rgpwszURLs[1]), S_OK);

	// bind to a row
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	TESTC_PROVIDER(VerifyInterface(pIScopedOperations, IID_IDBAsynchStatus, ROW_INTERFACE, (IUnknown**)&pIDBAsynchStatus));
	hThread = (HANDLE)_beginthreadex(NULL, 0, CancelProc,
					(void*)pIDBAsynchStatus,
					0, 
					&IDThread);
	TESTC(0 != hThread);

	// the actual tested operation
	TEST3C_(m_hr = pIScopedOperations->Delete(NUMELEM(rgpwszURLs), (const WCHAR**)rgpwszURLs, 
		DBCOPY_REPLACE_EXISTING | DBCOPY_ASYNC, rgdwStatus), DB_E_ASYNCNOTSUPPORTED, DB_S_ASYNCHRONOUS, DB_E_CANCELED);

CLEANUP:
	COMPARE(WAIT_OBJECT_0 == WaitForSingleObject(hThread, INFINITE), TRUE);
	if (0 != hThread)
	{
		COMPARE(CloseHandle(hThread), TRUE);
	}

	delete pCTree;
	SAFE_FREE(rgpwszURLs[0]);
	SAFE_FREE(rgpwszURLs[1]);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIDBAsynchStatus);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCDelete::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIScopedOperations::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCMove)
//*-----------------------------------------------------------------------
//| Test Case:		TCMove - IScopedOperations::Move()
//| Created:  	10/20/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCMove::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIScopedOperations::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc cRows is 0 => no op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_1()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;
	DBSTATUS			rgdwStatus[1];
	
	TESTC_(m_hr = g_pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pCTree->GetRootURL(), 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Move(0, (const WCHAR**)rgpwszSourceURLs, 
		(const WCHAR**)rgpwszDestURLs, 0, NULL, 
		rgdwStatus, rgpwszNewURLs, &pwszStringsBuffer), S_OK);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc cRows>0 and rgpwszSourceURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_2()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;
	DBSTATUS			rgdwStatus[1];

	TESTC_(m_hr = g_pCTree->PickANode(rgpwszDestURLs), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Move(1, NULL, 
		(const WCHAR**)rgpwszDestURLs, 0, NULL, 
		rgdwStatus, rgpwszNewURLs, &pwszStringsBuffer), E_INVALIDARG);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc cRows>0 and rgpwszDestURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_3()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;
	DBSTATUS			rgdwStatus[1];

	TESTC_(g_pCTree->PickANode(rgpwszSourceURLs), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Move(1, (const WCHAR**)rgpwszSourceURLs, 
		NULL, 0, NULL, 
		rgdwStatus, rgpwszNewURLs, &pwszStringsBuffer), E_INVALIDARG);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc cRows>0 and rgdwStatus is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_4()
{ 
	TBEGIN
	IScopedOperations	*pIScopedOperations	= NULL;
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pwszStringsBuffer	= NULL;

	TESTC_(m_hr = g_pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TESTC_(m_hr = pIScopedOperations->Move(1, (const WCHAR**)rgpwszSourceURLs, 
		(const WCHAR**)rgpwszDestURLs, 0, NULL, 
		NULL, rgpwszNewURLs, &pwszStringsBuffer), E_INVALIDARG);	
	
CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc rgpwszNewURLs is NULL and ppStringsBuffer is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_5()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 diffent nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc rgpwszNewURLs is NULL and ppStringsBuffer is not NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_6()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pStringsBuffer	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, &pStringsBuffer), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(pStringsBuffer);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc rgpwszNewURLs is not NULL and ppStringsBuffer is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_7()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, rgpwszNewURLs, NULL), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc rgpwszNewURLs is not NULL and ppStringsBuffer is not NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_8()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	WCHAR				*rgpwszNewURLs[2];
	DBSTATUS			rgdwStatus[2];
	WCHAR				*pStringsBuffer	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 4 nodes
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(0, rgpwszSourceURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(1, &rgpwszSourceURLs[1])));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(2, rgpwszDestURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(3, &rgpwszDestURLs[1])));

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, rgpwszNewURLs, &pStringsBuffer), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[1]);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Inexistent source URL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_9()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	// pick existent destination and inexistent source
	TESTC_(m_hr = g_pCTree->PickANode(rgpwszDestURLs), S_OK);
	TESTC_(m_hr = g_pCTree->GetInexistentURL(rgpwszSourceURLs, rgpwszDestURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Source is outside of current scope
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_10()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL	= NULL;
	IScopedOperations	*pIScopedOperations = NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick a scope node and the destination
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(&pwszRowURL, rgpwszDestURLs), S_OK);
	// pick source outside the scope
	TESTC_(m_hr = pCTree->PickASecondNode(pwszRowURL, NC_NONE, rgpwszSourceURLs), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->MoveAndCheck(pIScopedOperations, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK, DB_E_ERRORSOCCURRED);
	TESTC(S_OK == m_hr || DBSTATUS_E_RESOURCEOUTOFSCOPE == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc The parent of the destination doesn't exist
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_11()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL	= NULL;

	// pick a source
	TESTC_(m_hr = g_pCTree->PickANode(rgpwszSourceURLs), S_OK);
	TESTC_(m_hr = g_pCTree->GetURLWithInexistentFather(rgpwszDestURLs, rgpwszSourceURLs[0]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[0]);

CLEANUP:
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Destination is outside the current scope
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_12()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL	= NULL;
	IScopedOperations	*pIScopedOperations	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick scope row and source
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(&pwszRowURL, rgpwszSourceURLs), S_OK);
	// pick a destination outside the scope
	TESTC_(m_hr = pCTree->PickASecondNode(pwszRowURL, NC_NONE, rgpwszDestURLs), S_OK);
	
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRowURL, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->MoveAndCheck(pIScopedOperations, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK, DB_E_ERRORSOCCURRED);
	TESTC(S_OK == m_hr || DBSTATUS_E_RESOURCEOUTOFSCOPE == rgdwStatus[0]);

CLEANUP:
	delete pCTree;
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Source is in scope of destination
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_13()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick source and destination
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(rgpwszDestURLs, rgpwszSourceURLs), S_OK);

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	
	TESTC(DBSTATUS_E_CANNOTCOMPLETE == rgdwStatus[0]);
	
CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Destination is in the scope of source
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_14()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick source and destination
	TESTC_(m_hr = pCTree->Pick2OverlappingNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	
	TESTC(DBSTATUS_E_CANNOTCOMPLETE == rgdwStatus[0]);
	
CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Move a leaf to another existing leaf, with and without replace
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_15()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL	= NULL;
	IRow				*pIRow		= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick source and destination
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_LEAF, rgpwszDestURLs, 
		NC_LEAF, &pwszRowURL, PCO_DIFF), S_OK);
	rgpwszSourceURLs[0] = pwszRowURL;

	// try without using replace flag
	TESTC_(m_hr = pCTree->PickASecondNode(pwszRowURL, NC_LEAF, rgpwszSourceURLs), S_OK);
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Move a leaf to a new row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_16()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*pwszDestURL = NULL;
	DBSTATUS			rgdwStatus[1];
	WCHAR				*rgpwszNewURLs[1];
	WCHAR				*pStringsBuffer	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick source (leaf node)
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_LEAF, rgpwszSourceURLs,
		NC_LEAF, &pwszDestURL,
		PCO_DIFF), S_OK);

	TESTC(pCTree->MakeSuffix(L"38", pwszDestURL, rgpwszDestURLs));

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, rgpwszNewURLs, &pStringsBuffer), S_OK);	

	// try without using replace flag
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(pwszDestURL);
	SAFE_FREE(pStringsBuffer);
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_LEAF, rgpwszSourceURLs,
		NC_LEAF, &pwszDestURL,
		PCO_DIFF), S_OK);
	TESTC(pCTree->MakeSuffix(L"42", pwszDestURL, rgpwszDestURLs));

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, rgpwszNewURLs, &pStringsBuffer), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszDestURL);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Move a leaf to a subtree (existent) with and without replace
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_17()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// pick 2 diffent nodes
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_LEAF, rgpwszSourceURLs, 
		NC_SUBTREE, rgpwszDestURLs, PCO_DIFF), S_OK);

	// try without using replace flag
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// try using the replace flag
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	


CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Move a subtree to a leaf (existent) with and without replacing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_18()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_SUBTREE, rgpwszSourceURLs, 
		NC_LEAF, rgpwszDestURLs, PCO_DIFF), S_OK);

	// try without using replace flag
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	


CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Move a subtree to an existent subtree with or without replacing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_19()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_SUBTREE, rgpwszSourceURLs, 
		NC_SUBTREE, rgpwszDestURLs, PCO_DIFF), S_OK);

	// try without using replace flag
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

	// try with replace flag
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Move a subtree to a new row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_20()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*pwszDestURL = NULL;
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_SUBTREE, rgpwszSourceURLs, 
		NC_Collection, &pwszDestURL, PCO_DIFF), S_OK);

	TESTC(pCTree->MakeSuffix(L"33", pwszDestURL, &rgpwszDestURLs[0]));

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

	// try without using replace flag
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszDestURL);
	TESTC_(m_hr = pCTree->Pick2Nodes(
		NC_SUBTREE, rgpwszSourceURLs, 
		NC_Collection, &pwszDestURL, PCO_DIFF), S_OK);

	TESTC(pCTree->MakeSuffix(L"37", pwszDestURL, &rgpwszDestURLs[0]));

	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszDestURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Atomicity, at least one failure
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_21()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], NC_NONE, &rgpwszSourceURLs[1]), S_OK);
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(&rgpwszDestURLs[1], rgpwszSourceURLs[1]), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING | DBMOVE_ATOMIC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED, E_INVALIDARG);

	if (DB_E_ERRORSOCCURRED == m_hr)
		TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Atomicity, all operations fail
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_22()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, &rgpwszDestURLs[1]), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszSourceURLs[1], rgpwszDestURLs[1]), S_OK);
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(&rgpwszDestURLs[0], rgpwszSourceURLs[0]), S_OK);

	// the actual tested operation
	TEST2C_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_ATOMIC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED, E_INVALIDARG);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Atomicity, all succeed
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_23()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(0, rgpwszSourceURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(3, &rgpwszSourceURLs[1])));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(1, rgpwszDestURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(2, &rgpwszDestURLs[1])));

	// the actual tested operation
	TEST2C_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING | DBMOVE_ATOMIC, NULL, 
		rgdwStatus, NULL, NULL), S_OK, E_INVALIDARG);
	if (S_OK == m_hr)
		TESTC(S_OK == rgdwStatus[0] && S_OK == rgdwStatus[1]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Non atomic move operations, at least one succeed, at least one fails => DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_24()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the 2 nodes
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], 
		NC_NONE, &rgpwszSourceURLs[1]), S_OK);
	TESTC_(m_hr = pCTree->GetURLWithInexistentFather(&rgpwszDestURLs[1], rgpwszSourceURLs[1]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), DB_S_ERRORSOCCURRED);	

	TESTC(DBSTATUS_S_OK == rgdwStatus[0]);
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Non atomic move operations, all ops fail => DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_25()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, &rgpwszDestURLs[1]), S_OK);
	// pick the first destination as an existing node (no replace flag is used)
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0],
		NC_NONE, rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->GetInexistentURL(&rgpwszSourceURLs[1], rgpwszDestURLs[1]), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	

	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);
	TESTC(DBSTATUS_E_DOESNOTEXIST == rgdwStatus[1]);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Non atomic move, all operations succeed
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_26()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_PROVIDER(4 <= pCTree->GetRootSchema()->GetLeafNo());
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(0, rgpwszSourceURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(1, &rgpwszSourceURLs[1])));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(2, rgpwszDestURLs)));
	TESTC_PROVIDER(S_OK == (m_hr = pCTree->GetRow(3, &rgpwszDestURLs[1])));

	// the actual tested operation
	TEST2C_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK, DB_S_ERRORSOCCURRED);

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszSourceURLs[1]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc No replace, existing destination => error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_27()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	// pick the rows
	TESTC_(m_hr = g_pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = g_pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ERRORSOCCURRED);	
	TESTC(DBSTATUS_E_RESOURCEEXISTS == rgdwStatus[0]);

CLEANUP:
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Replace, existing destination => ok
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_28()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc No replace, no existing destination => ok
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_29()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, &pwszRowURL), S_OK);
	TESTC(pCTree->MakeSuffix(L"50", pwszRowURL, rgpwszDestURLs));

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, 0, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Replace, no existing destination => ok
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_30()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pwszRowURL	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, &pwszRowURL), S_OK);
	TESTC(pCTree->MakeSuffix(L"34", pwszRowURL, rgpwszDestURLs));

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), S_OK);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pwszRowURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Generate the destination rowsets in ppStringsBuffer and check them
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_31()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	WCHAR				*rgpwszNewURLs[1];
	DBSTATUS			rgdwStatus[1];
	WCHAR				*pStringsBuffer	= NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
		rgpwszDestURLs, DBMOVE_REPLACE_EXISTING, NULL, 
		rgdwStatus, rgpwszNewURLs, &pStringsBuffer), S_OK);	

	//TESTC(NULL != rgpwszNewURLs[0] && 0 == wcscmp(rgpwszDestURLs[0], rgpwszNewURLs[0]));

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(pStringsBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Inexistent flag => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_32()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	DWORD				dwFlags;
	DWORD				dwOldFlags;
	DWORD				dwTestFlag;
	// build the list of invalid flags mask
	const DWORD			dwInvalidFlags = 0xFFFFFFFF ^ 
									(		DBMOVE_ASYNC
										|	DBMOVE_REPLACE_EXISTING
										|	DBMOVE_ALLOW_EMULATION
										|	DBMOVE_ATOMIC
										|	DBMOVE_DONT_UPDATE_LINKS
									);

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);
	
	// pick the rows
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// the actual tested operation
	for (dwFlags = dwInvalidFlags; dwFlags;)
	{
		dwOldFlags	= dwFlags;
		dwFlags		= dwFlags & (dwFlags-1);
		dwTestFlag	= dwFlags ^ dwOldFlags;

		TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 1, rgpwszSourceURLs, 
			rgpwszDestURLs, dwTestFlag, NULL, 
			rgdwStatus, NULL, NULL), E_INVALIDARG);	
	}

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc element of rgpwszSourceURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_33()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	TESTC_(m_hr = pCTree->PickASecondNode(rgpwszSourceURLs[0], NC_NONE, 
		&rgpwszDestURLs[1]), S_OK);
	rgpwszSourceURLs[1] = NULL;

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_FREE(rgpwszDestURLs[1]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc element of rgpwszDestURLs is NULL => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_34()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[2];
	WCHAR				*rgpwszDestURLs[2];
	DBSTATUS			rgdwStatus[2];

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);
	rgpwszSourceURLs[1] = rgpwszSourceURLs[0];
	rgpwszDestURLs[1]	= NULL;

	// the actual tested operation
	TESTC_(m_hr = pCTree->MoveAndCheck(NULL, 2, rgpwszSourceURLs, 
		rgpwszDestURLs, DBCOPY_REPLACE_EXISTING, NULL, 
		rgdwStatus, NULL, NULL), E_INVALIDARG);	

CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Async operation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_35()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	IScopedOperations	*pIScopedOperations = NULL;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// bind to a row
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);
	
	// the actual tested operation
	TEST2C_(m_hr = pIScopedOperations->Move(1, (const WCHAR**)rgpwszSourceURLs, 
		(const WCHAR**)rgpwszDestURLs, DBMOVE_REPLACE_EXISTING | DBMOVE_ASYNC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ASYNCNOTSUPPORTED, DB_S_ASYNCHRONOUS);
	
CLEANUP:
	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_RELEASE(pIScopedOperations);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Try to cancel the asynch operation before the call completes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCMove::Variation_36()
{ 
	TBEGIN
	WCHAR				*rgpwszSourceURLs[1];
	WCHAR				*rgpwszDestURLs[1];
	DBSTATUS			rgdwStatus[1];
	IScopedOperations	*pIScopedOperations = NULL;
	IDBAsynchStatus		*pIDBAsynchStatus	= NULL;
	HANDLE				hThread;
	unsigned			IDThread;

	CIScOpsTree				*pCTree = new CIScOpsTree(m_pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	// create a tree structure of URLs
	TESTC_(m_hr = pCTree->CopyTree(g_pCTree, g_pwszRootRow), S_OK);

	// existing source and destination
	TESTC_(m_hr = pCTree->Pick2DifNodes(rgpwszSourceURLs, rgpwszDestURLs), S_OK);

	// bind to a row
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0, 
		DBBINDURLFLAG_READ, DBGUID_ROW, 
		IID_IScopedOperations, NULL, NULL, NULL, 
		(IUnknown**)&pIScopedOperations), S_OK);

	TESTC_PROVIDER(VerifyInterface(pIScopedOperations, IID_IDBAsynchStatus, ROW_INTERFACE, (IUnknown**)&pIDBAsynchStatus));
	
	hThread = (HANDLE)_beginthreadex(NULL, 0, CancelProc,
					(void*)pIDBAsynchStatus,
					0, 
					&IDThread);
	TESTC(0 != hThread);

	// the actual tested operation
	TEST3C_(m_hr = pIScopedOperations->Move(1, (const WCHAR**)rgpwszSourceURLs, 
		(const WCHAR**)rgpwszDestURLs, DBMOVE_REPLACE_EXISTING | DBMOVE_ASYNC, NULL, 
		rgdwStatus, NULL, NULL), DB_E_ASYNCNOTSUPPORTED, DB_S_ASYNCHRONOUS, DB_E_CANCELED);
	
CLEANUP:
	COMPARE(WAIT_OBJECT_0 == WaitForSingleObject(hThread, INFINITE), TRUE);
	if (0 != hThread)
	{
		COMPARE(CloseHandle(hThread), TRUE);
	}

	delete pCTree;
	SAFE_FREE(rgpwszSourceURLs[0]);
	SAFE_FREE(rgpwszDestURLs[0]);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIDBAsynchStatus);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCMove::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIScopedOperations::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCBindRow)
//*-----------------------------------------------------------------------
//| Test Case:		TCBindRow - IScopedOperations::Bind() test
//| Created:  	10/23/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindRow::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIScopedOperations::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Inexistent URL => DB_E_NOTFOUND
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_1()
{ 
	TBEGIN
	IRow	*pIRow		= NULL;
	WCHAR	*pwszURL	= NULL;

	TESTC_(m_hr = g_pCTree->GetInexistentURL(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_IRow,
		NULL, NULL, NULL,
		(IUnknown**)&pIRow), DB_E_NOTFOUND);

	TESTC(NULL == pIRow);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc URL out of current scope => DB_E_RESOURCEOUTOFSCOPE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_2()
{ 
	TBEGIN
	WCHAR				*pwszRow	= NULL;
	WCHAR				*pwszRowSCO	= NULL;
	IRow				*pIRow		= NULL;
	IScopedOperations	*pIScopedOperations = NULL;

	// pick a couple of overlapped nodes
	TESTC_(m_hr = g_pCTree->Pick2DifNodes(&pwszRowSCO, &pwszRow), S_OK);

	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszRow,
		DBBINDURLFLAG_READ, DBGUID_ROW, IID_IScopedOperations, 
		NULL, NULL, NULL, (IUnknown**)&pIScopedOperations), S_OK);

	TESTC_(m_hr = pIScopedOperations->Bind(
		NULL, pwszRowSCO,
		DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_IRow,
		NULL, NULL, NULL,
		(IUnknown**)&pIRow), DB_E_RESOURCEOUTOFSCOPE);

	TESTC(NULL == pIRow);

CLEANUP:
	SAFE_FREE(pwszRow);
	SAFE_FREE(pwszRowSCO);
	SAFE_RELEASE(pIScopedOperations);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid bind flags -> E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_3()
{ 
	TBEGIN
	IRow	*pIRow		= NULL;
	WCHAR	*pwszURL	= NULL;
	DWORD				dwFlags;
	DWORD				dwOldFlags;
	DWORD				dwTestFlag;

	// build the list of invalid flags mask
	const DWORD			dwInvalidFlags = 0xFFFFFFFF ^ 
									(		DBBINDURLFLAG_READ
										|	DBBINDURLFLAG_WRITE
										|	DBBINDURLFLAG_READWRITE
										|	DBBINDURLFLAG_SHARE_DENY_READ
										|	DBBINDURLFLAG_SHARE_DENY_WRITE
										|	DBBINDURLFLAG_SHARE_EXCLUSIVE
										|	DBBINDURLFLAG_SHARE_DENY_NONE
										|	DBBINDURLFLAG_ASYNCHRONOUS
										|	DBBINDURLFLAG_COLLECTION
										|	DBBINDURLFLAG_DELAYFETCHSTREAM
										|	DBBINDURLFLAG_DELAYFETCHCOLUMNS
										|	DBBINDURLFLAG_RECURSIVE
										|	DBBINDURLFLAG_OUTPUT
										|	DBBINDURLFLAG_WAITFORINIT
										|	DBBINDURLFLAG_OPENIFEXISTS
										|	DBBINDURLFLAG_OVERWRITE
										|	DBBINDURLFLAG_ISSTRUCTUREDDOCUMENT
									);

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	for (dwFlags = dwInvalidFlags; dwFlags;)
	{
		dwOldFlags	= dwFlags;
		dwFlags		= dwFlags & (dwFlags-1);
		dwTestFlag	= dwFlags ^ dwOldFlags;
	
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			dwTestFlag,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, NULL,
			(IUnknown**)&pIRow), E_INVALIDARG);

		TESTC(NULL == pIRow);
	}

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Get IRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_4()
{
	TBEGIN
//	ULONG	ulRow;
	IRow	*pIRow		= NULL;
	WCHAR	*pwszURL	= NULL;

	// prepare table for comparison
	//TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
	//for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
	g_pCTree->ResetPosition();
	m_hr = S_OK;
	pwszURL = g_pCTree->GetCurrentRowURL();
	for (;S_OK == m_hr;)
	{
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, NULL,
			(IUnknown**)&pIRow), S_OK);
		
		TESTC(testIRow(pIRow, pwszURL, FALSE));
		SAFE_RELEASE(pIRow);
		TEST2C_(m_hr = g_pCTree->MoveToNextNode(&pwszURL), S_OK, DB_S_ERRORSOCCURRED);
	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Test pIScopedOperations as a row interface (QI)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_5()
{ 
	TBEGIN
	WCHAR				*pwszURL			= NULL;
	IScopedOperations	*pIScopedOperations = NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
		
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, pwszURL,
		DBBINDURLFLAG_READ, DBGUID_ROW, IID_IScopedOperations, 
		NULL, NULL, NULL, (IUnknown**)&pIScopedOperations), S_OK);

	TESTC(DefaultObjectTesting(pIScopedOperations, ROW_INTERFACE));

CLEANUP:
	SAFE_RELEASE(pIScopedOperations);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Check that binding to DBGUID_DSO fails
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_6()
{ 
	TBEGIN
	WCHAR			*pwszURL	= NULL;
	IDBProperties	*pIDBProperties	= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		DBGUID_DSO, IID_IDBProperties,
		NULL, NULL, NULL,
		(IUnknown**)&pIDBProperties), DB_E_NOTSUPPORTED);

	TESTC(NULL == pIDBProperties);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIDBProperties);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Check that binding to DBGUID_SESSION fails
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_7()
{ 
	TBEGIN
	WCHAR			*pwszURL		= NULL;
	IOpenRowset		*pIOpenRowset	= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		DBGUID_SESSION, IID_IOpenRowset,
		NULL, NULL, NULL,
		(IUnknown**)&pIOpenRowset), DB_E_NOTSUPPORTED);

	TESTC(NULL == pIOpenRowset);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIOpenRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Invalid rguid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_8()
{ 
	TBEGIN
	WCHAR			*pwszURL		= NULL;
	IUnknown		*pIUnknown		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		m_pThisTestModule->m_ProviderClsid, IID_IUnknown,
		NULL, NULL, NULL,
		(IUnknown**)&pIUnknown), DB_E_NOTSUPPORTED);

	TESTC(NULL == pIUnknown);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Unsupported interface => E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_9()
{ 
	TBEGIN
	WCHAR				*pwszURL			= NULL;
	IDBCreateSession	*pIDBCreateSession	= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_IDBCreateSession,
		NULL, NULL, NULL,
		(IUnknown**)&pIDBCreateSession), E_NOINTERFACE);

	TESTC(NULL == pIDBCreateSession);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIDBCreateSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Not null pImplSession => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_10()
{ 
	TBEGIN
	WCHAR				*pwszURL	= NULL;
	IUnknown			*pIUnknown	= NULL;
	DBIMPLICITSESSION	ImplicitSession;
	
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
	memset(&ImplicitSession, 0, sizeof(DBIMPLICITSESSION));

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_IUnknown,
		NULL, &ImplicitSession, NULL,
		(IUnknown**)&pIUnknown), E_INVALIDARG);

	TESTC(NULL == pIUnknown);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_11()
{ 
	TBEGIN
//	ULONG			ulRow;
	IColumnsInfo	*pIColumnsInfo	= NULL;
	WCHAR			*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IColumnsInfo,
			NULL, NULL, NULL,
			(IUnknown**)&pIColumnsInfo), S_OK);
		
		TESTC(DefTestInterface(pIColumnsInfo))
//		SAFE_RELEASE(pIColumnsInfo);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Get IGetSession
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_12()
{ 
	TBEGIN
	IGetSession		*pIGetSession	= NULL;
	WCHAR			*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IGetSession,
			NULL, NULL, NULL,
			(IUnknown**)&pIGetSession), S_OK);
		
	TESTC(testIGetSession(pIGetSession))

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIGetSession);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_13()
{ 
	TBEGIN
	IConvertType	*pIConvertType	= NULL;
	WCHAR			*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IConvertType,
			NULL, NULL, NULL,
			(IUnknown**)&pIConvertType), S_OK);

	TESTC(DefTestInterface(pIConvertType))

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIConvertType);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Get IColumnsInfo2 (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_14()
{ 
	TBEGIN
	IColumnsInfo2	*pIColumnsInfo2	= NULL;
	WCHAR			*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IColumnsInfo2,
			NULL, NULL, NULL,
			(IUnknown**)&pIColumnsInfo2), S_OK, E_NOINTERFACE);

	if (S_OK == m_hr)
	{
		TESTC(DefTestInterface(pIColumnsInfo2))
		TESTC(testIColumnsInfo2(pIColumnsInfo2))
	}


CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIColumnsInfo2);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Get ICreateRow (optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_15()
{ 
	TBEGIN
	ICreateRow		*pICreateRow	= NULL;
	WCHAR			*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_ICreateRow,
			NULL, NULL, NULL,
			(IUnknown**)&pICreateRow), S_OK, E_NOINTERFACE);

	if (S_OK == m_hr)
		TESTC(testICreateRow(pICreateRow, pwszURL))


CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pICreateRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_16()
{ 
	TBEGIN
//	ULONG			ulRow;
	IDBInitialize	*pIDBI			= NULL;
	IRow			*pIRow			= NULL;
	WCHAR			*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST3C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT,
			DBGUID_ROW, IID_IDBInitialize,
			NULL, NULL, NULL,
			(IUnknown**)&pIDBI), S_OK, E_INVALIDARG, E_NOINTERFACE);

		TESTC_PROVIDER(m_hr == S_OK)

		TESTC_(pIDBI->QueryInterface(IID_IRow, (void**)&pIRow), E_NOINTERFACE);
		TESTC(NULL == pIRow);

		TESTC_(pIDBI->Initialize(), S_OK)

		TESTC(DefaultObjectTesting(pIDBI, ROW_INTERFACE))

//		SAFE_RELEASE(pIDBI);
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIDBI);
	SAFE_RELEASE(pIRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_17()
{ 
	TBEGIN
	IRow		*pIRow			= NULL;
	IRowChange	*pIRowChange	= NULL;
	WCHAR		*pwszURL		= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST3C_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READWRITE,
		DBGUID_ROW, IID_IRow,
		NULL, NULL, NULL,
		(IUnknown**)&pIRow), S_OK, E_INVALIDARG, DB_E_READONLY);
		
	TESTC_PROVIDER(S_OK == m_hr);

	TESTC(testIRow(pIRow, pwszURL, FALSE));

	TESTC(VerifyInterface(pIRow, IID_IRowChange, ROW_INTERFACE,
			(IUnknown**)&pIRowChange))

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRowChange);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_READ
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_18()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	DBBINDURLSTATUS		dwStatus;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

//		SAFE_RELEASE(pIRow1);
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_19()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	DBBINDURLSTATUS		dwStatus;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), S_OK);

		SAFE_RELEASE(pIRow1);

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

//		SAFE_RELEASE(pIRow1);
		
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_20()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	DBBINDURLSTATUS		dwStatus;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_EXCLUSIVE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

		SAFE_RELEASE(pIRow1);

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

//		SAFE_RELEASE(pIRow1);
		
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_NONE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_21()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	DBBINDURLSTATUS		dwStatus;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ|DBBINDURLFLAG_SHARE_DENY_NONE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), S_OK);

		SAFE_RELEASE(pIRow1);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), S_OK, DB_E_READONLY);

//		SAFE_RELEASE(pIRow1);
//		
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_22()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	WCHAR				*pwszRowURL		= NULL;
	DBBINDURLSTATUS		dwStatus;
//	ULONG				ulDescRow;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST3C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_WRITE
			|DBBINDURLFLAG_RECURSIVE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED, E_INVALIDARG);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		// check that read is not denied
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), S_OK);

		SAFE_RELEASE(pIRow1);
		
		// check write is denied on the row
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

		SAFE_RELEASE(pIRow1);
		
		// check that the sharing restrictions are propagated to
		// subtree
//		SAFE_FREE(pwszURL);
		TESTC_(m_hr = g_pCTree->SetPosition(pwszURL), S_OK);
		pwszRowURL = pwszURL;
		for (; S_OK == m_hr; )
//		if (ulRow <= cMaxRows/2)
		{
			// pick a row from the subtree
//			ulDescRow = PickRandomRowFromSubtree(ulRow);
//			pwszURL = GetURL(m_pwszRootURL0, ulDescRow);
		
			// check that read is not denied
			TESTC_(m_hr = m_pIScopedOperations0->Bind(
				NULL, pwszURL,
				DBBINDURLFLAG_READ,
				DBGUID_ROW, IID_IRow,
				NULL, NULL, &dwStatus,
				(IUnknown**)&pIRow1), S_OK);
			SAFE_RELEASE(pIRow1);
			
			// check write is denied on the row
			TESTC_(m_hr = m_pIScopedOperations0->Bind(
				NULL, pwszURL,
				DBBINDURLFLAG_WRITE,
				DBGUID_ROW, IID_IRow,
				NULL, NULL, &dwStatus,
				(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);
			SAFE_RELEASE(pIRow1);
			
			TEST2C_(m_hr = g_pCTree->MoveToNextNode(&pwszRowURL), S_OK, DB_S_ERRORSOCCURRED);
		}

		TESTC_(m_hr, DB_S_ERRORSOCCURRED);
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Flag - OUTPUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_23()
{ 
	TBEGIN
//	ULONG	ulRow;
	IRow	*pIRow		= NULL;
	WCHAR	*pwszURL	= NULL;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_OUTPUT,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, NULL,
			(IUnknown**)&pIRow), S_OK, E_INVALIDARG);
		
		TESTC_PROVIDER(m_hr == S_OK);

		TESTC(testIRow(pIRow, pwszURL, FALSE));
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_24()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Flag - DELATFETCHSTREAM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_25()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Flag - DELAYFETCHCOLUMNS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_26()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Row
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_27()
{ 
	TBEGIN
//	ULONG	ulRow;
	WCHAR	*pwszURL	= NULL;
	CAggregate Aggregate(NULL);

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			&Aggregate, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, NULL,
			(IUnknown**)&Aggregate.m_pIUnkInner), S_OK, DB_E_NOAGGREGATION);

		AGGREGATION_SUPPORT(m_hr)

		//Verify Aggregation.
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(m_hr, IID_IRow));

//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_28()
{ 
	TBEGIN
	IRow				*pIRow		= NULL;
	WCHAR				*pwszURL	= NULL;
	CAggregate			Aggregate(NULL);
	DBIMPLICITSESSION	dbImplSess;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = (GUID*)&IID_IUnknown;

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ,
		DBGUID_ROW, IID_IRow,
		NULL, &dbImplSess, NULL,
		(IUnknown**)&pIRow), E_INVALIDARG);	// according to the spec

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_READ
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_29()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	WCHAR				*pwszRowURL		= NULL;
	DBBINDURLSTATUS		dwStatus;
//	ULONG				ulDescRow;
	
	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST3C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_READ
			|DBBINDURLFLAG_RECURSIVE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED, E_INVALIDARG);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		// check that read is not denied
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

		SAFE_RELEASE(pIRow1);
		
		// check write is denied on the row
		TEST2C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), S_OK, DB_E_READONLY);

		SAFE_RELEASE(pIRow1);
		
		// check that the sharing restrictions are propagated to
		// subtree
		TESTC_(m_hr = g_pCTree->SetPosition(pwszURL), S_OK);
		pwszRowURL = pwszURL;
		for (; S_OK == m_hr; )
		{
			// pick a row from the subtree					
			// check that read is not denied
			TESTC_(m_hr = m_pIScopedOperations0->Bind(
					NULL, pwszRowURL,
					DBBINDURLFLAG_READ,
					DBGUID_ROW, IID_IRow,
					NULL, NULL, &dwStatus,
					(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);
			SAFE_RELEASE(pIRow1);
				
			// check write is denied on the row
			TEST2C_(m_hr = m_pIScopedOperations0->Bind(
					NULL, pwszRowURL,
					DBBINDURLFLAG_WRITE,
					DBGUID_ROW, IID_IRow,
					NULL, NULL, &dwStatus,
					(IUnknown**)&pIRow1), S_OK, DB_E_READONLY);
			SAFE_RELEASE(pIRow1);

			TEST2C_(m_hr = g_pCTree->MoveToNextNode(&pwszRowURL), S_OK, DB_S_ERRORSOCCURRED);
		}

		TESTC_(m_hr, DB_S_ERRORSOCCURRED);

//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_30()
{ 
	TBEGIN
//	ULONG				ulRow;
	IRow				*pIRow			= NULL;
	IRow				*pIRow1			= NULL;
	WCHAR				*pwszURL		= NULL;
	WCHAR				*pwszRowURL		= NULL;
	DBBINDURLSTATUS		dwStatus;
//	ULONG				ulDescRow;

	// prepare table for comparison
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	TESTC_(m_hr = PrepareComparison(m_pwszRootURL0, m_pTable), S_OK);
	
//	for (ulRow = 1; ulRow <= cMaxRows; ulRow++)
//	{
//		pwszURL = GetURL(g_pwszRootRow0, ulRow);

		TEST3C_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_EXCLUSIVE
			|DBBINDURLFLAG_RECURSIVE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow), S_OK, DB_S_ERRORSOCCURRED, E_INVALIDARG);

		if (DB_S_ERRORSOCCURRED == m_hr)
			TESTC(	DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus 
				||	DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus);

		TESTC_PROVIDER(m_hr == S_OK)

		// check that read is not denied
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_READ,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

		SAFE_RELEASE(pIRow1);
		
		// check write is denied on the row
		TESTC_(m_hr = m_pIScopedOperations0->Bind(
			NULL, pwszURL,
			DBBINDURLFLAG_WRITE,
			DBGUID_ROW, IID_IRow,
			NULL, NULL, &dwStatus,
			(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);

		SAFE_RELEASE(pIRow1);
		
		// check that the sharing restrictions are propagated to
		// subtree
//		SAFE_FREE(pwszURL);
		TESTC_(m_hr = g_pCTree->SetPosition(pwszURL), S_OK);
		pwszRowURL = pwszURL;
		for (; S_OK == m_hr; )
		{
//		if (ulRow <= cMaxRows/2)
//		{
			// pick a row from the subtree
//			ulDescRow = PickRandomRowFromSubtree(ulRow);
//			pwszURL = GetURL(m_pwszRootURL0, ulDescRow);

			// check that read is not denied
			TESTC_(m_hr = m_pIScopedOperations0->Bind(
				NULL, pwszRowURL,
				DBBINDURLFLAG_READ,
				DBGUID_ROW, IID_IRow,
				NULL, NULL, &dwStatus,
				(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);
			SAFE_RELEASE(pIRow1);
			
			// check write is denied on the row
			TESTC_(m_hr = m_pIScopedOperations0->Bind(
				NULL, pwszRowURL,
				DBBINDURLFLAG_WRITE,
				DBGUID_ROW, IID_IRow,
				NULL, NULL, &dwStatus,
				(IUnknown**)&pIRow1), DB_E_RESOURCELOCKED);
			SAFE_RELEASE(pIRow1);
			TEST2C_(m_hr = g_pCTree->MoveToNextNode(&pwszRowURL), S_OK, DB_S_ERRORSOCCURRED);
		}

		TESTC_(m_hr, DB_S_ERRORSOCCURRED);
//		SAFE_RELEASE(pIRow);
//		SAFE_FREE(pwszURL);
//	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIRow);
	SAFE_RELEASE(pIRow1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_NONE => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_31()
{ 
	TBEGIN
	WCHAR			*pwszURL	= NULL;
	IRow			*pIRow		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	pwszURL = PickRandomRow(g_pwszRootRow0, 2, 31, 0);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ | DBBINDURLFLAG_RECURSIVE 
		|DBBINDURLFLAG_SHARE_DENY_NONE,
		DBGUID_ROW, IID_IRow,
		NULL, NULL, NULL,
		(IUnknown**)&pIRow), E_INVALIDARG);

	TESTC(NULL == pIRow);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE => E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_32()
{ 
	TBEGIN
	WCHAR			*pwszURL	= NULL;
	IRow			*pIRow		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);
//	pwszURL = PickRandomRow(g_pwszRootRow0, 2, 31, 0);

	TESTC_(m_hr = m_pIScopedOperations0->Bind(
		NULL, pwszURL,
		DBBINDURLFLAG_READ | DBBINDURLFLAG_RECURSIVE,
		DBGUID_ROW, IID_IRow,
		NULL, NULL, NULL,
		(IUnknown**)&pIRow), E_INVALIDARG);

	TESTC(NULL == pIRow);

CLEANUP:
	SAFE_FREE(pwszURL);
	SAFE_RELEASE(pIRow);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Test DBPROPVAL_OO_SCOPED flag of DBPROP_OLEOBJECTS in DBPROPSET_DATASOURCEINFO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRow::Variation_33()
{ 
	TBEGIN
	VARIANT					vVal;
	BOOL					fSupportedScope			= FALSE;
	IDBProperties			*pIDBProperties			= NULL;

	// at this point it is known that IScopedOperations is supported

	// check DBPROPVAL_OO_SCOPED
	TESTC_(m_hr = g_pIBindResource->Bind(NULL, g_pwszRootRow0,
		DBBINDURLFLAG_READ, //| DBBINDURLFLAG_COLLECTION | DBBINDURLFLAG_READWRITE | DBBINDURLFLAG_OPENIFEXISTS,
		DBGUID_DSO, IID_IDBProperties, NULL, NULL, NULL,
		(IUnknown**)&pIDBProperties), S_OK);
	TESTC(NULL != pIDBProperties);

	VariantInit(&vVal);
	fSupportedScope = GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, pIDBProperties, &vVal) 
		&&	VT_I4 == vVal.vt 
		&&	(V_I4(&vVal) & DBPROPVAL_OO_SCOPED);
	VariantClear(&vVal);
	TESTC(fSupportedScope);

CLEANUP:
	SAFE_RELEASE(pIDBProperties);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindRow::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIScopedOperations::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



//--------------------------------------------------------------------------
//
// Check whether the property sets asked for rowset was preserved
// all the data should be inside, the only thing that is allowed to be modified <nl>
// is the property status
//--------------------------------------------------------------------------
BOOL IsPropSetPreserved
(
	DBPROPSET	*rgInPropertySets,	// the array passed to function 
	DBPROPSET	*rgOutPropertySets,	// the aray returned by function 
	ULONG		cPropertySets			// the size of the arrays
)
{
	ULONG	ps, pc;
	HRESULT	hr;
	
	if (NULL == rgInPropertySets)
		return rgOutPropertySets == NULL;
	for (ps=0; ps<cPropertySets; ps++)
	{
		// compare the props in that prop set
		if (!CHECK(rgInPropertySets[ps].guidPropertySet == rgOutPropertySets[ps].guidPropertySet, TRUE))
			return FALSE;
		if (!CHECK(rgInPropertySets[ps].cProperties == rgOutPropertySets[ps].cProperties, TRUE))
			return FALSE;
		if (NULL == rgInPropertySets[ps].rgProperties)
		{
			if (!CHECK(NULL == rgOutPropertySets[ps].rgProperties, TRUE))
				return FALSE;
			continue;
		}
		for (pc=0; pc<rgInPropertySets[ps].cProperties; pc++)
		{
			if (!CHECK(rgInPropertySets[ps].rgProperties[pc].dwPropertyID == 
					   rgOutPropertySets[ps].rgProperties[pc].dwPropertyID, TRUE))
				return FALSE;
			if (!CHECK(rgInPropertySets[ps].rgProperties[pc].dwOptions == 
					   rgOutPropertySets[ps].rgProperties[pc].dwOptions, TRUE))
				return FALSE;
			if (!CHECK(hr = CompareDBID(rgInPropertySets[ps].rgProperties[pc].colid, 
					   rgOutPropertySets[ps].rgProperties[pc].colid), TRUE))
				return FALSE;
			if (!CHECK(CompareVariant(&rgInPropertySets[ps].rgProperties[pc].vValue,
					   &rgOutPropertySets[ps].rgProperties[pc].vValue), TRUE))
				return FALSE;
		}
	}
	return TRUE;
} //IsPropSetPreserved

// {{ TCW_TC_PROTOTYPE(TCBindRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCBindRowset - IScopedOperations::Bind() test
//| Created:  	11/19/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindRowset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIScopedOperations::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IAccessor
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_1()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	IAccessor*			pIAccessor		= NULL;
	IRowset*			pIRowset		= NULL;
	WCHAR				*pwszURL		= NULL;
	ULONG				cRows			= 2;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IAccessor, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIAccessor), S_OK);

	TESTC(VerifyInterface(pIAccessor, IID_IRowset, 
		ROWSET_INTERFACE, (IUnknown**)&pIRowset))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get IColumnsInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_2()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	IColumnsInfo		*pIColumnsInfo	= NULL;
	WCHAR				*pwszURL		= NULL;
	ULONG				cRows			= 2;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IColumnsInfo, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIColumnsInfo), S_OK);

	TESTC(DefTestInterface(pIColumnsInfo))

CLEANUP:
	SAFE_RELEASE(pIColumnsInfo);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Get IConvertType
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_3()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	IConvertType		*pIConvertType	= NULL;
	WCHAR				*pwszURL		= NULL;
	ULONG				cRows			= 2;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IConvertType, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIConvertType), S_OK);

	TESTC(DefTestInterface(pIConvertType))

CLEANUP:
	SAFE_RELEASE(pIConvertType);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_4()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	IAccessor*			pIAccessor		= NULL;
	IRowset*			pIRowset		= NULL;
	WCHAR				*pwszURL		= NULL;
	ULONG				cRows			= 2;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIRowset), S_OK);

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowsetInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_5()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	IRowsetInfo			*pIRowsetInfo	= NULL;
	WCHAR				*pwszURL		= NULL;
	ULONG				cRows			= 2;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowsetInfo, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIRowsetInfo), S_OK);

	TESTC(DefTestInterface(pIRowsetInfo))

CLEANUP:
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - Get IRowsetIdentity
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_6()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus		= 0; 
	IRowsetIdentity		*pIRowsetIdentity	= NULL;
	WCHAR				*pwszURL			= NULL;
	ULONG				cRows				= 2;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowsetIdentity, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIRowsetIdentity), S_OK);

CLEANUP:
	SAFE_RELEASE(pIRowsetIdentity);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - Bind to URLs of rows of the Rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_7()
{ 
	TBEGIN
	IRowset		*pIRowset	= NULL;
	IAccessor	*pIAccessor	= NULL;
	WCHAR		*pwszURL	= NULL;

	// prepare table for comparison
	g_pCTree->ResetPosition();
	pwszURL = g_pCTree->GetRootURL();
	m_hr = S_OK;

	for (; S_OK == m_hr; )
	{
		if (g_pCTree->IsCollection(pwszURL))
		{
			TESTC_(m_hr = m_pIScopedOperations0->Bind(
				NULL, pwszURL,
				DBBINDURLFLAG_READ,
				DBGUID_ROWSET, IID_IRowset,
				NULL, NULL, NULL,
				(IUnknown**)&pIRowset), S_OK);
			
			TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
				ROWSET_INTERFACE, (IUnknown**)&pIAccessor))	
			TESTC(testRowset(pIAccessor, pIRowset))

			SAFE_RELEASE(pIAccessor);
			SAFE_RELEASE(pIRowset);
			
			TEST2C_(m_hr = g_pCTree->MoveToNextNode(&pwszURL), S_OK, DB_S_ERRORSOCCURRED);
		}
	}

CLEANUP:
	TOUTPUT_IF_FAILED(pwszURL)
	SAFE_RELEASE(pIAccessor);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - Optional interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_8()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus		= 0; 
	IColumnsRowset		*pIColumnsRowset	= NULL;
	IRowsetChange		*pIRowsetChange		= NULL;
	WCHAR				*pwszURL			= NULL;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IColumnsRowset, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIColumnsRowset), S_OK, E_NOINTERFACE);

	if(!pIColumnsRowset)
		odtLog<<L"INFO: IColumnsRowset is not supported.\n";

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowsetChange, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIRowsetChange), S_OK, E_NOINTERFACE);

	if(!pIRowsetChange)
		odtLog<<L"INFO: IRowsetChange is not supported.\n";

CLEANUP:
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowsetChange);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - IGetRow (if ROW objects are supported)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_9()
{ 
	TBEGIN
	IGetRow				*pIGetRow	= NULL;
	WCHAR				*pwszURL	= NULL;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IGetRow, NULL, NULL, 
		NULL, (IUnknown**)&pIGetRow), S_OK, E_NOINTERFACE);

	if(m_hr == E_NOINTERFACE)
		odtLog<<L"INFO: IGetRow is not supported.\n";
	else
		TESTC(testIGetRow(pIGetRow))

CLEANUP:
	SAFE_RELEASE(pIGetRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Rowset
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_10()
{ 
	TBEGIN
	WCHAR				*pwszURL	= NULL;
	CAggregate			Aggregate(NULL);
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, &Aggregate, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IUnknown, NULL, NULL, 
		NULL, (IUnknown**)&Aggregate.m_pIUnkInner), S_OK, DB_E_NOAGGREGATION);

	AGGREGATION_SUPPORT(m_hr)

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(m_hr, IID_IRowset));

CLEANUP:
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate implicit session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_11()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	WCHAR				*pwszURL	= NULL;

	CAggregate			Aggregate(NULL);
	DBIMPLICITSESSION	dbImplSess;

	dbImplSess.pUnkOuter = &Aggregate;
	dbImplSess.piid = (GUID*)&IID_IUnknown;

	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, IID_IRowset, NULL, &dbImplSess, 
		NULL, (IUnknown**)&pIRowset), E_INVALIDARG);

	TESTC(NULL == pIRowset);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_12()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IDBInitialize		*pIDBI		= NULL;
	WCHAR				*pwszURL	= NULL;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST3C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_WAITFORINIT, DBGUID_ROWSET, 
		IID_IDBInitialize, NULL, NULL, 
		NULL, (IUnknown**)&pIDBI), S_OK, E_NOINTERFACE, E_INVALIDARG);
	TESTC_PROVIDER(m_hr == S_OK)

	TESTC_(pIDBI->QueryInterface(IID_IRowset, (void**)&pIRowset),
		E_NOINTERFACE)
	TESTC(!pIRowset)

	TESTC_(pIDBI->Initialize(), S_OK)

	TESTC(DefaultObjectTesting(pIDBI, ROWSET_INTERFACE))

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIDBI);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_13()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IAccessor			*pIAccessor	= NULL;
	WCHAR				*pwszURL	= NULL;
	
	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST3C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READWRITE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset), S_OK, DB_E_READONLY, E_INVALIDARG);

	TESTC_PROVIDER(S_OK == m_hr);

	TESTC(VerifyInterface(pIRowset, IID_IAccessor, 
		ROWSET_INTERFACE, (IUnknown**)&pIAccessor))			

	TESTC(testRowset(pIAccessor, pIRowset))

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIAccessor);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_READ
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_14()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IRowset				*pIRowset1	= NULL;
	WCHAR				*pwszURL	= NULL;
	DBBINDURLSTATUS		dwStatus;

	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_READ, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		&dwStatus, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	
	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus)

	TESTC_PROVIDER(S_OK == m_hr);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset1), DB_E_RESOURCELOCKED);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_15()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IRowset				*pIRowset1	= NULL;
	WCHAR				*pwszURL	= NULL;
	DBBINDURLSTATUS		dwStatus;

	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_WRITE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		&dwStatus, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	
	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus)

	TESTC_PROVIDER(S_OK == m_hr);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READWRITE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset1), DB_E_READONLY, DB_E_RESOURCELOCKED);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_16()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IRowset				*pIRowset1	= NULL;
	WCHAR				*pwszURL	= NULL;
	DBBINDURLSTATUS		dwStatus;

	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_EXCLUSIVE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		&dwStatus, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	
	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus)

	TESTC_PROVIDER(S_OK == m_hr);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset1), DB_E_RESOURCELOCKED);

	SAFE_RELEASE(pIRowset1);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READWRITE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset1), DB_E_READONLY, DB_E_RESOURCELOCKED);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_DENY_NONE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_17()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IRowset				*pIRowset1	= NULL;
	WCHAR				*pwszURL	= NULL;
	DBBINDURLSTATUS		dwStatus;

	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickACollection(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_NONE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		&dwStatus, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	
	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus)

	TESTC_PROVIDER(S_OK == m_hr);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset1), S_OK);

	SAFE_RELEASE(pIRowset1);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READWRITE, DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		NULL, (IUnknown**)&pIRowset1), DB_E_READONLY, S_OK);

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_18()
{ 
	TBEGIN
	IRowset				*pIRowset	= NULL;
	IRowset				*pIRowset1	= NULL;
	WCHAR				*pwszURL	= NULL;
	DBBINDURLSTATUS		dwStatus;

	// pick a collection URL
	TESTC_(m_hr = g_pCTree->PickASubtree(&pwszURL), S_OK);

	TEST3C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_RECURSIVE 
		| DBBINDURLFLAG_SHARE_DENY_WRITE, 
		DBGUID_ROWSET, 
		IID_IRowset, NULL, NULL, 
		&dwStatus, (IUnknown**)&pIRowset), S_OK, E_INVALIDARG, DB_S_ERRORSOCCURRED);
	
	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwStatus)

	TESTC_PROVIDER(S_OK == m_hr);

	TESTC_(m_hr = g_pCTree->SetPosition(pwszURL), S_OK);
	SAFE_FREE(pwszURL);
	pwszURL = g_pCTree->GetCurrentRowURL();
	for (; S_OK == m_hr; )
	{
		TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
			DBBINDURLFLAG_READWRITE, DBGUID_ROWSET, 
			IID_IRowset, NULL, NULL, 
			NULL, (IUnknown**)&pIRowset1), DB_E_READONLY, DB_E_RESOURCELOCKED);
		SAFE_RELEASE(pIRowset1);
		TEST2C_(m_hr = g_pCTree->MoveToNextNode(&pwszURL), 
			S_OK, DB_S_ERRORSOCCURRED);
	}
	pwszURL = NULL;
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindRowset::Variation_19()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindRowset::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIScopedOperations::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCBindStream)
//*-----------------------------------------------------------------------
//| Test Case:		TCBindStream - IScopedOperations::Bind() test
//| Created:  	11/19/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCBindStream::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIScopedOperations::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Get IGetSourceRow
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_1()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	IGetSourceRow*		pIGetSourceRow	= NULL;
	WCHAR				*pwszURL		= NULL;
		
	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_STREAM, IID_IGetSourceRow, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIGetSourceRow), S_OK, E_NOINTERFACE)

	if(S_OK == m_hr)
		TESTC(testIGetSourceRow(pIGetSourceRow))

CLEANUP:
	SAFE_RELEASE(pIGetSourceRow);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Get ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_2()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	ULONG				cBytes			= 1;
	void*				pBuffer			= NULL;
	ISequentialStream	*pISS			= NULL;
	WCHAR				*pwszURL		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS), S_OK);

	SAFE_ALLOC(pBuffer, BYTE, 1)
	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == 1)

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_RELEASE(pISS);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Aggregate Stream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_3()
{ 
	TBEGIN
	CAggregate			Aggregate(NULL);
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	WCHAR				*pwszURL		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, &Aggregate, pwszURL, 
		DBBINDURLFLAG_READ, DBGUID_STREAM, IID_IUnknown, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&Aggregate.m_pIUnkInner), S_OK, DB_E_NOAGGREGATION);

	AGGREGATION_SUPPORT(m_hr)

	//Verify Aggregation.
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(m_hr, IID_IGetSourceRow));

CLEANUP:
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Flag - WAITFORINIT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_4()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	ISequentialStream	*pISS			= NULL;
	IDBInitialize		*pIDBI			= NULL;
	WCHAR				*pwszURL		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST3C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ|DBBINDURLFLAG_WAITFORINIT, DBGUID_STREAM, 
		IID_IDBInitialize, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pIDBI), S_OK, E_INVALIDARG, E_NOINTERFACE)

	TESTC_PROVIDER(m_hr == S_OK)

	TESTC_(pIDBI->QueryInterface(IID_ISequentialStream, (void**)&pISS),
		E_NOINTERFACE)
	TESTC(!pISS)

	TESTC_(pIDBI->Initialize(), S_OK)

	TESTC(DefaultObjectTesting(pIDBI, STREAM_INTERFACE))

CLEANUP:
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pIDBI);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Flag - READWRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_5()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	ULONG				cBytes			= 1;
	ULONG				cBytesWrote		= 0;
	void				*pBuffer		= NULL;
	ISequentialStream	*pISS			= NULL;
	WCHAR				*pwszURL		= NULL;

	TESTC_(m_hr = g_pCTree->PickANode(&pwszURL), S_OK);

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READWRITE, DBGUID_STREAM, 
		IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS), S_OK, DB_E_READONLY)

	TESTC_PROVIDER(m_hr == S_OK)

	SAFE_ALLOC(pBuffer, BYTE, cBytes)
	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == 1)

	TESTC_(StorageWrite(IID_ISequentialStream, pISS, pBuffer, cBytes, 
		&cBytesWrote), S_OK)
	TESTC(cBytesWrote == cBytes)

CLEANUP:
	SAFE_RELEASE(pISS);
	SAFE_FREE(pwszURL);
	SAFE_FREE(pBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Flag - SHARE_EXCLUSIVE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_6()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	ULONG				cBytes			= 1;
	void				*pBuffer		= NULL;
	ISequentialStream	*pISS			= NULL;
	ISequentialStream	*pISS1			= NULL;
	WCHAR				*pwszURL		= NULL;

	TESTC_(m_hr = g_pCTree->PickASubtree(&pwszURL), S_OK);

	TEST3C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_EXCLUSIVE, 
		DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS), S_OK, E_INVALIDARG, DB_S_ERRORSOCCURRED);

	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwBindStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwBindStatus)
	TESTC_PROVIDER(m_hr == S_OK)

	SAFE_ALLOC(pBuffer, BYTE, 1)
	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == 1)

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, 
		DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS1), DB_E_RESOURCELOCKED);

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_WRITE, 
		DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS1), DB_E_RESOURCELOCKED);

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Flag - RECURSIVE & SHARE_DENY_WRITE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_7()
{ 
	TBEGIN
	DBBINDURLSTATUS		dwBindStatus	= 0; 
	ULONG				cBytes			= 1;
	void				*pBuffer		= NULL;
	ISequentialStream	*pISS			= NULL;
	ISequentialStream	*pISS1			= NULL;
	WCHAR				*pwszURL		= NULL;

	TESTC_(m_hr = g_pCTree->PickASubtree(&pwszURL), S_OK);

	TEST3C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ | DBBINDURLFLAG_SHARE_DENY_WRITE, 
		DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS), S_OK, E_INVALIDARG, DB_S_ERRORSOCCURRED);

	TESTC(DB_S_ERRORSOCCURRED != m_hr 
		|| DBBINDURLSTATUS_S_DENYTYPENOTSUPPORTED == dwBindStatus
		|| DBBINDURLSTATUS_S_DENYNOTSUPPORTED == dwBindStatus)
	TESTC_PROVIDER(m_hr == S_OK)

	SAFE_ALLOC(pBuffer, BYTE, 1)
	TESTC_(GetStorageData(IID_ISequentialStream, pISS, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == 1)

	TESTC_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_READ, 
		DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS1), S_OK);
	SAFE_RELEASE(pISS1);
	SAFE_ALLOC(pBuffer, BYTE, 1)
	TESTC_(GetStorageData(IID_ISequentialStream, pISS1, pBuffer, 
		&cBytes), S_OK)
	TESTC(cBytes == 1)

	TEST2C_(m_hr = g_pCTree->BindAndCheck(m_pIScopedOperations0, NULL, pwszURL, 
		DBBINDURLFLAG_WRITE, 
		DBGUID_STREAM, IID_ISequentialStream, NULL, NULL, 
		&dwBindStatus, (IUnknown**)&pISS1), DB_E_RESOURCELOCKED, DB_E_READONLY);

CLEANUP:
	SAFE_FREE(pBuffer);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS1);
	SAFE_FREE(pwszURL);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Flag - OUTPUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_8()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Flag - ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCBindStream::Variation_9()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCBindStream::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIScopedOperations::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

