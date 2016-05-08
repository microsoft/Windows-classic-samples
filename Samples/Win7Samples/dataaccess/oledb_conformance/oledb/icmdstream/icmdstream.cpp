//*-----------------------------------------------------------------------
//
//   Conformance Test for ICommandStream interface.
//
//   WARNING:
//          PLEASE USE THE TEST CASE WIZARD TO ADD/DELETE TESTS AND VARIATIONS!
//
//
//   Copyright (C) 1994-2000 Microsoft Corporation
//*-----------------------------------------------------------------------


#include "MODStandard.hpp"
#include "ICmdStream.h"
#include "ExtraLib.h"


//*-----------------------------------------------------------------------
// Module Values
//*-----------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x58748a00, 0x8be4, 0x11d3, { 0x89, 0x4b, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("ICommandStream");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("ICommandStream Test Module");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END


///////////////////////////////////////////////////////////
//GLOBALS
///////////////////////////////////////////////////////////
GUID	GUIDNULL = DB_NULLGUID;
WCHAR	wszByteOrder[1] = {0xFEFF};
BOOL	g_fSQLOLEDB = FALSE;


//*-----------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	WCHAR*				wszProviderName = NULL;
	IDBCreateCommand*	pIDBCreateCommand = NULL;
	ICommandStream*		pICmdStream = NULL;

	// Get connection and session objects
	TESTC(ModuleCreateDBSession(pThisTestModule));

	// IDBCreateCommand
	TESTC_PROVIDER(VerifyInterface(pThisTestModule->m_pIUnknown2, IID_IDBCreateCommand, 
		SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand))

	TEST2C_(hr = pIDBCreateCommand->CreateCommand(NULL, 
		IID_ICommandStream, (IUnknown**)&pICmdStream), S_OK, E_NOINTERFACE)

	TESTC_PROVIDER(hr == S_OK)

	// Create a table and store it in pVoid for now
	pThisTestModule->m_pVoid = new CTable(
		(IUnknown *)pThisTestModule->m_pIUnknown2, (LPWSTR)gwszModuleName);

	TESTC(pThisTestModule->m_pVoid != NULL);

	// Start with a table with 10 rows
	TESTC_(((CTable *)pThisTestModule->m_pVoid)->CreateTable(10), S_OK);

	g_fSQLOLEDB=FALSE;
	if (GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, pThisTestModule->m_pIUnknown, &wszProviderName)
		&& wszProviderName)
	{
		if (!wcscmp((LPWSTR)wszProviderName, L"sqloledb.dll"))
			g_fSQLOLEDB=TRUE;
	}

CLEANUP:
	SAFE_FREE(wszProviderName);
	SAFE_RELEASE(pICmdStream);
	SAFE_RELEASE(pIDBCreateCommand);
	TRETURN
}

//*-----------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	// Drop the table created in the ModuleInit
	if( pThisTestModule->m_pVoid )
	{
		// Remove table from database and Delete CTable object
		((CTable *)pThisTestModule->m_pVoid)->DropTable();
		delete ((CTable *)pThisTestModule->m_pVoid);
		pThisTestModule->m_pVoid = NULL;
	}
	
	return ModuleReleaseDBSession(pThisTestModule);
}



///////////////////////////////////////////////////////////////
// Base class for testing ICommandStream interface
//
///////////////////////////////////////////////////////////////
class CCmdStream : public CSessionObject
{
public:

	// @cmember Constructor
	CCmdStream(LPWSTR wszTestCaseName) : CSessionObject (wszTestCaseName)
	{
		m_pwszQ1					= NULL;
		m_pwszQ2					= NULL;
		m_pStorageGet				= NULL;
		m_pStorageSet				= NULL;
		m_pICommandText				= NULL;
		m_pICommandStream			= NULL;	
	};

	// @cmember Destructor
	virtual ~CCmdStream() {};

protected:

//VARIABLES

	//Storage object used for getting back cmd stream
	CStorage*	m_pStorageGet;

	//Storage object used for setting cmd stream
	CStorage*	m_pStorageSet;

	//Queries that will successfully Execute when set on
	//ICommandStream with DBGUID_DEFAULT.
	WCHAR*		m_pwszQ1;
	WCHAR*		m_pwszQ2;

//INTERFACES

	// @cmember ICommandText Interface pointer
	ICommandText *		m_pICommandText;
	// @cmember ICommandText Interface pointer
	ICommandStream *	m_pICommandStream;

//FUNCTIONS

	//@cmember Init
	BOOL	Init();

	//@cmember Terminate
	BOOL	Terminate();

	//Get the Queries.
	BOOL	GetQueries();

	//Wrapper for ICommandStream::GetCommandStream.
	HRESULT	GetCmdStream(IID* piid, GUID* pguid, WCHAR** pwszCmd=NULL);

	//Wrapper for ICommandStream::SetCommandStream.
	HRESULT	SetCmdStream(IID iid, GUID guid, WCHAR* pwszCmd=NULL);

	//Reads the stream obtained by GetCommandStream, or
	//pUnkStream. Reads as unicode.
	HRESULT	Read(WCHAR** ppwszCmd, IUnknown* pUnkStream=NULL);

	//Write to the stream to be set using SetCommandStream.
	HRESULT	Write(WCHAR* pwszCmd);

	//Prepare the command.
	BOOL	PrepCmd();

	//Set a command property
	HRESULT	SetCmdProp(DBPROPSET* prgPropSet, IUnknown* pICmd=NULL);

	//Default testing function for ICommandStream interface.
	BOOL DefTest(ICommandStream* pICS);

	//Verification function for rowsets.
	BOOL	VerifyRowset(EQUERY eQuery, IRowset* pIR);

	//Verification function for m_pwszQ1.
	BOOL	VerifyQuery1(IUnknown* pIStream, REFIID riid);

	//Verification function for m_pwszQ2.
	BOOL	VerifyQuery2(IUnknown* pIStream, REFIID riid);

	BOOL	GetAttrValue(IXMLDOMNode* pNode, WCHAR** ppwszName, WCHAR** ppwszVal);

	BOOL	QIDOMNode(IXMLDOMNode* pIn, IXMLDOMNode** pOut);

	//Set a DBPROP_OUTPUTENCODING.
	HRESULT	SetEncdProp(ENCODING eOE, IUnknown* pICmd=NULL);

};


//--------------------------------------------------------------
// CCmdStream::Init
//
BOOL CCmdStream::Init()
{
	TBEGIN

  	TESTC(COLEDB::Init())

	// Set needed pointers
	SetDataSourceObject(m_pThisTestModule->m_pIUnknown, TRUE);
	SetDBSession((IDBCreateCommand *)m_pThisTestModule->m_pIUnknown2);
	SetTable((CTable *)(m_pThisTestModule->m_pVoid), DELETETABLE_NO);

	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, 
		IID_ICommandStream, (IUnknown**)&m_pICommandStream), S_OK)
	TESTC(VerifyInterface(m_pICommandStream, 
		IID_ICommandText, COMMAND_INTERFACE,(IUnknown **)&m_pICommandText))

	TESTC(GetQueries())

	CHECK(SetEncdProp(UTF16), S_OK);

CLEANUP:
	TRETURN
}

//--------------------------------------------------------------
// CCmdStream::Terminate
//
BOOL CCmdStream::Terminate()
{
	// Release objects
	SAFE_FREE(m_pwszQ1);
	SAFE_FREE(m_pwszQ2);
	SAFE_RELEASE(m_pStorageGet);
	SAFE_RELEASE(m_pStorageSet);
	SAFE_RELEASE(m_pICommandText);
	SAFE_RELEASE(m_pICommandStream);
	ReleaseDBSession();
	ReleaseDataSourceObject();
	
	return(COLEDB::Terminate());
}

//--------------------------------------------------------------
// CCmdStream::GetQueries
//
BOOL CCmdStream::GetQueries()
{
	TBEGIN
	BOOL		fRet = FALSE;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszCol = NULL;
	CCol		rgCol;

  	m_pwszQ1 = wcsDuplicate(GetModInfo()->GetParseObject()->GetQuery(SETCMDSTREAM_QUERY1));
	m_pwszQ2 = wcsDuplicate(GetModInfo()->GetParseObject()->GetQuery(SETCMDSTREAM_QUERY2));

	if(m_pwszQ1)
		goto QUERY2;

	TESTC_(m_pTable->GetColInfo(1, rgCol), S_OK)
	pwszCol = rgCol.GetColName();

	SAFE_ALLOC(pwszSQL, WCHAR, (sizeof(WCHAR) *
		(wcslen(wszSELECT_COLLISTFROMTBL) +	wcslen(pwszCol) +	
		wcslen(m_pTable->GetTableName()) - 4)) + sizeof(WCHAR));

	// Format SQL Statement
	swprintf(pwszSQL, wszSELECT_COLLISTFROMTBL, pwszCol, m_pTable->GetTableName());

	SAFE_ALLOC(m_pwszQ1, WCHAR, wcslen(L"<root xmlns:sql=\"urn:schemas-microsoft-com:xml-sql\"><sql:query> for xml auto</sql:query></root>")+wcslen(pwszSQL)+sizeof(WCHAR));
	wcscpy(m_pwszQ1, L"<root xmlns:sql=\"urn:schemas-microsoft-com:xml-sql\"><sql:query>");
	wcscat(m_pwszQ1, pwszSQL);
	wcscat(m_pwszQ1, L" for xml auto</sql:query></root>");
	//m_pwszQ1 = wcsDuplicate(L"<root xmlns:sql=\"urn:schemas-microsoft-com:xml-sql\"><sql:query>select uid from sysusers for xml auto</sql:query></root>");
	SAFE_FREE(pwszSQL);

	TESTC(m_pwszQ1 != NULL)

QUERY2:

	if(m_pwszQ2)
		goto CLEANUP;

	m_pwszQ2 = wcsDuplicate(L"<root1>Some valid xml. Will be echoed by Execute.</root1>");

	TESTC(m_pwszQ2 != NULL)

CLEANUP:
	SAFE_FREE(pwszSQL);
	TRETURN
}

//--------------------------------------------------------------
// CCmdStream::GetCmdStream
//
HRESULT CCmdStream::GetCmdStream(IID* piid, GUID* pguid, WCHAR** ppwszCmd)
{
	HRESULT	hr=E_FAIL;
	GUID	guidIn;

	if(!m_pICommandStream)
		return E_FAIL;

	if(pguid)
		guidIn = *pguid;

	hr = m_pICommandStream->GetCommandStream(piid, pguid, (IUnknown**)&m_pStorageGet);
	if(m_pStorageGet && g_fSQLOLEDB)
		m_pStorageGet->Seek(0);

	//Common verification
	if(SUCCEEDED(hr))
	{
		COMPARE(m_pStorageGet != NULL, TRUE);
		if(pguid && hr==S_OK)
			COMPARE(*pguid==guidIn, TRUE);

		if(hr == DB_S_DIALECTIGNORED)
		{
			COMPARE(pguid != NULL, TRUE);
			if(pguid)
				COMPARE(*pguid!=guidIn, TRUE);
		}
	}
	else
	{
		COMPARE(m_pStorageGet == NULL, TRUE);
		m_pStorageGet = NULL;
		if(pguid)
			COMPARE(*pguid, GUIDNULL);
		if(piid)
			COMPARE(*piid, IID_NULL);
	}

	//Mask this code with S_OK for convenience.
	if(hr == DB_S_DIALECTIGNORED)
	{
		odtLog<<L"INFO: GetCommandStream returned DB_S_DIALECTIGNORED.\n";
		hr = S_OK;
	}

	if(SUCCEEDED(hr) && ppwszCmd)
	{
		hr = Read(ppwszCmd);
		if(m_pStorageGet && g_fSQLOLEDB)
			m_pStorageGet->Seek(0);
	}

	return hr;
}

//--------------------------------------------------------------
// CCmdStream::SetCmdStream
//
HRESULT CCmdStream::SetCmdStream(IID iid, GUID guid, WCHAR* pwszCmd)
{
	HRESULT	hr=E_FAIL;

	if(!m_pICommandStream || !m_pStorageSet)
		return E_FAIL;

	if(pwszCmd)
	{
		CHECK(Write(pwszCmd), S_OK);
	}

	hr = m_pICommandStream->SetCommandStream(iid, guid, (ISequentialStream*)m_pStorageSet);

	return hr;
}

//--------------------------------------------------------------
// CCmdStream::Read
//
HRESULT CCmdStream::Read(WCHAR** ppwszCmd, IUnknown* pUnkStream)
{
	HRESULT	hr=E_FAIL;
	ULONG	cRead = 0;
	ULONG	ulReadIncr = READ_INCR;
	WCHAR*	pwszCmd = NULL;
	BYTE*	pwszEnd = NULL;
	BOOL	fEnd = FALSE;
	ISequentialStream*	pISS = NULL;

	if(!pUnkStream)
		pUnkStream = (ISequentialStream*)m_pStorageGet;

	if(!m_pICommandStream || !pUnkStream || !ppwszCmd)
		return E_FAIL;

	*ppwszCmd = NULL;

	TESTC(VerifyInterface(pUnkStream, IID_ISequentialStream, 
		STREAM_INTERFACE,(IUnknown **)&pISS))

	SAFE_ALLOC(pwszCmd, WCHAR, ulReadIncr);

	//First read 2 bytes only. If this is the byte-order 
	//mark, remove it.
	hr = pISS->Read(pwszCmd, 2, &cRead);

	//If the stream was empty ...
	if((hr==S_FALSE) && (cRead==0))
	{
		fEnd = TRUE;
		SAFE_FREE(pwszCmd);
	}

	if(cRead==2)
	{
		if(memcmp(wszByteOrder, pwszCmd, 2) == 0)
		{
			hr = pISS->Read(pwszCmd, ulReadIncr, &cRead);
			if(cRead < ulReadIncr)
			{
				fEnd = TRUE;
				pwszEnd = (BYTE*)pwszCmd + cRead;
			}
		}
		else
		{
			hr = pISS->Read((BYTE*)pwszCmd +2, ulReadIncr-2, &cRead);
			if(cRead < ulReadIncr-2)
			{
				fEnd = TRUE;
				pwszEnd = (BYTE*)pwszCmd +2 + cRead;
			}
		}
	}

	while(!fEnd)
	{
		cRead = 0;
		ulReadIncr += READ_INCR;
		SAFE_REALLOC(pwszCmd, WCHAR, ulReadIncr);
		hr = pISS->Read((BYTE*)pwszCmd +ulReadIncr -READ_INCR, READ_INCR, &cRead);
		if(cRead < READ_INCR)
		{
			fEnd = TRUE;
			pwszEnd = (BYTE*)pwszCmd +ulReadIncr -READ_INCR + cRead;
		}
	}

	if(pwszEnd)
		memset((BYTE*)pwszEnd, 0, 2);

	if(fEnd)
	{
		hr = S_OK;
		*ppwszCmd = pwszCmd;
	}

CLEANUP:
	SAFE_RELEASE(pISS);
	return hr;
}

//--------------------------------------------------------------
// CCmdStream::Write
//
HRESULT CCmdStream::Write(WCHAR* pwszCmd)
{
	HRESULT	hr=E_FAIL;
	ULONG	cWrote = 0;
	ULONG	ulLen = 0;

	if(!m_pICommandStream || !m_pStorageSet || !pwszCmd)
		return E_FAIL;

	ulLen = (ULONG)(wcslen(pwszCmd));

	hr = m_pStorageSet->Write(wszByteOrder, 2, NULL);
	hr = m_pStorageSet->Write(pwszCmd, (ulLen)*sizeof(WCHAR), &cWrote);
	m_pStorageSet->Seek(0);

	return hr;
}

//--------------------------------------------------------------
// CCmdStream::PrepCmd
//
BOOL CCmdStream::PrepCmd()
{
	BOOL				fRet = FALSE;
	ICommandPrepare*	pICmdPrep = NULL;

	if(VerifyInterface(m_pICommandStream, 
		IID_ICommandPrepare, COMMAND_INTERFACE,(IUnknown **)
		&pICmdPrep))
	{
		TESTC_(pICmdPrep->Prepare(1), S_OK)
	}

	fRet = TRUE;

CLEANUP:
	SAFE_RELEASE(pICmdPrep);
	return fRet;
}

//--------------------------------------------------------------
// CCmdStream::SetCmdProp
//
HRESULT	CCmdStream::SetCmdProp(DBPROPSET* rgPropSet, IUnknown* pICmd)
{
	HRESULT		hr = E_FAIL;
	ICommandProperties*	pICP = NULL;

	if(pICmd && !VerifyInterface(pICmd, 
		IID_ICommandProperties, COMMAND_INTERFACE,(IUnknown **)
		&pICP))
		return E_FAIL;

	if(!pICP && !VerifyInterface(m_pICommandStream, 
		IID_ICommandProperties, COMMAND_INTERFACE,(IUnknown **)
		&pICP))
		return E_FAIL;

	if(rgPropSet)
		hr = pICP->SetProperties(1, rgPropSet);

	SAFE_RELEASE(pICP);
	return hr;
}

//--------------------------------------------------------------
// CCmdStream::SetEncdProp
//
HRESULT	CCmdStream::SetEncdProp(ENCODING eOE, IUnknown* pICmd)
{
	HRESULT		hr = E_FAIL;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	BSTR		bstr;

	if(eOE == UTF8)
		bstr = L"UTF-8";
	else if(eOE == UTF16)
		bstr = L"UTF-16";
	else
		return E_FAIL;

	SetProperty(DBPROP_OUTPUTENCODING, DBPROPSET_STREAM, 
		&cPropSets, &rgPropSets, (void*)bstr, DBTYPE_BSTR);
	hr = SetCmdProp(rgPropSets, pICmd);
	COMPARE(hr==S_OK || hr==DB_E_ERRORSOCCURRED, TRUE);
	if(FAILED(hr))
		DumpPropertyErrors(cPropSets, rgPropSets);

	FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}

//--------------------------------------------------------------
// CCmdStream::DefTest
//
BOOL CCmdStream::DefTest(ICommandStream* pICS)
{
	TBEGIN

	TESTC(pICS != NULL)
	TESTC(DefaultObjectTesting(pICS, COMMAND_INTERFACE))

CLEANUP:
	TRETURN
}

//--------------------------------------------------------------
// CCmdStream::VerifyRowset
//
BOOL CCmdStream::VerifyRowset(EQUERY eQuery, IRowset* pIR)
{
	TBEGIN
	DBCOUNTITEM		cRows = 0;
	DBCOUNTITEM		ulIndex = 0;
	HROW			hRow = 0;
	void*			pData = NULL;
	CRowset*		pcr = NULL;

	g_pIDBInitialize = m_pIDBInitialize;
	g_pIOpenRowset = m_pIOpenRowset;
	g_pTable = m_pTable;

	pcr = new CRowset();
	TESTC(pcr != NULL)

	//Currently using this only for "select *" query.
	if(eQuery != SELECT_ALLFROMTBL)
		goto CLEANUP;

	cRows = m_pTable->CountRowsOnTable();
	TESTC(SUCCEEDED(pcr->CreateRowset(pIR, SELECT_ALLFROMTBL)))

	for(ulIndex=0; ulIndex<cRows; ulIndex++)
	{
		TESTC_(pcr->GetRow(ulIndex+1, &hRow), S_OK)
		TESTC_(pcr->GetRowData(hRow, &pData), S_OK)
		TESTC(pcr->CompareRowData(hRow, pData))
		SAFE_FREE(pData);
		TESTC_(pcr->ReleaseRows(hRow), S_OK)
	}

CLEANUP:
	g_pIDBInitialize = NULL;
	g_pIOpenRowset = NULL;
	g_pTable = NULL;
	SAFE_DELETE(pcr);
	SAFE_FREE(pData);
	TRETURN
}

//--------------------------------------------------------------
// CCmdStream::VerifyQuery1
//
BOOL CCmdStream::VerifyQuery1(IUnknown* pIStream, REFIID riid)
{
	TBEGIN
	HRESULT				hr=E_FAIL;
	CLSID				clsid;
	DBTYPE				dbType;
	CCol				rgCol;
	DOMNodeType			nodeType;
	ULONG				cRow = 1;
	WCHAR*				wszData = NULL;
	WCHAR*				pwszVal = NULL;
	WCHAR*				pwszColName = NULL;
	IXMLDOMDocument*	pXMLDoc = NULL;
    IXMLDOMNode*		pNode   = NULL;
	IXMLDOMNode*		pRoot   = NULL;
	IXMLDOMNode*		pChild	= NULL;
	IXMLDOMNode*		pNext = NULL;

	VARIANT_BOOL		bSuccess = VARIANT_FALSE;
	VARIANT             streamIn;
	VariantInit(&streamIn);

	TESTC(pIStream != NULL)

	V_VT(&streamIn)        =    VT_UNKNOWN;
    V_UNKNOWN(&streamIn)   =    (ISequentialStream*)pIStream;
	
	pIStream->AddRef();

	TESTC_(m_pTable->GetColInfo(1, rgCol), S_OK)
	dbType = rgCol.GetProviderType();

	//TESTC_(hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, 
	//	IID_IXMLDOMDocument, (void**)&pXMLDoc), S_OK)

	CLSIDFromProgID(L"Msxml2.DOMDocument", &clsid);
	TESTC_(hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, 
      IID_IXMLDOMDocument, (void**)&pXMLDoc), S_OK)

	hr = pXMLDoc->load(streamIn, &bSuccess);
	if(bSuccess != VARIANT_TRUE)
		TESTC_(hr, S_OK)

	TESTC_(hr = pXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&pNode), S_OK)
	
	//Find the first node of type NODE_ELEMENT, i.e. the 
	//root node.
	TESTC_(hr = pNode->get_nodeType(&nodeType), S_OK)
	if(nodeType == NODE_ELEMENT)
		TESTC(QIDOMNode(pNode, &pRoot))
	else
	{
		//Now recurse into any children...
		TESTC(SUCCEEDED(pNode->get_firstChild(&pChild)))
		while(pChild)
		{	
			//Recurse for all siblings...
			TESTC_(hr = pChild->get_nodeType(&nodeType), S_OK)
			if(nodeType == NODE_ELEMENT)
			{
				TESTC(QIDOMNode(pChild, &pRoot))
				break;
			}

			TESTC(SUCCEEDED(pChild->get_nextSibling(&pNext)))
			TESTC_(hr = pNext->get_nodeType(&nodeType), S_OK)
			if(nodeType == NODE_ELEMENT)
			{
				TESTC(QIDOMNode(pNext, &pRoot))
				break;
			}
			
			SAFE_RELEASE(pChild);
			if(pNext)
				TESTC(QIDOMNode(pNext, &pChild))
			SAFE_RELEASE(pNext);
		}
	}

	SAFE_RELEASE(pNode);
	SAFE_RELEASE(pChild);
	SAFE_RELEASE(pNext);

	SAFE_ALLOC(wszData, WCHAR, sizeof(WCHAR)*DATA_SIZE);

	TESTC(SUCCEEDED(pRoot->get_firstChild(&pChild)))
	while(pChild || cRow==1)
	{	
		//Recurse for all siblings...
		TESTC_(hr = pChild->get_nodeType(&nodeType), S_OK)

		if(nodeType == NODE_ELEMENT)
		{
			hr = m_pTable->MakeData(wszData, cRow, 1, PRIMARY, DBTYPE_WSTR);
			GetAttrValue(pChild, &pwszColName, &pwszVal);

			COMPARE(wcscmp(wszData, pwszVal), 0);

			cRow++;
			memset(wszData, 0, sizeof(WCHAR)*DATA_SIZE);
		}
		
		TESTC(SUCCEEDED(pChild->get_nextSibling(&pNext)))

		SAFE_RELEASE(pChild);
		if(pNext)
			TESTC(QIDOMNode(pNext, &pChild))
		SAFE_RELEASE(pNext);
	}

CLEANUP:
	VariantClear(&streamIn);
	SAFE_FREE(wszData);
	SAFE_FREE(pwszVal);
	SAFE_FREE(pwszColName);
	SAFE_RELEASE(pXMLDoc);
    SAFE_RELEASE(pNode);
	SAFE_RELEASE(pChild);
	SAFE_RELEASE(pNext);
	SAFE_RELEASE(pRoot);
	TRETURN;
}

//--------------------------------------------------------------
// CCmdStream::QIDOMNode
//
BOOL CCmdStream::QIDOMNode(IXMLDOMNode* pIn, IXMLDOMNode** ppOut)
{
	TBEGIN

	TESTC(pIn && ppOut)

	TESTC_(pIn->QueryInterface(IID_IXMLDOMNode, (void **)ppOut), S_OK)

CLEANUP:
	TRETURN
}

//--------------------------------------------------------------
// CCmdStream::GetAttrValue
//
BOOL CCmdStream::GetAttrValue(IXMLDOMNode* pNode, WCHAR** ppwszName, WCHAR** ppwszVal)
{
	TBEGIN
	VARIANT					var;
	BSTR					bstrName = NULL;
	IXMLDOMNamedNodeMap*	pAttrs = NULL;
	IXMLDOMNode*			pChild = NULL;;

	VariantInit(&var);

	TESTC(pNode && ppwszName && ppwszVal)

	TESTC(SUCCEEDED(pNode->get_attributes(&pAttrs)))

	TESTC(SUCCEEDED(pAttrs->nextNode(&pChild)))

		//Attribute name
	TESTC(SUCCEEDED(pChild->get_nodeName(&bstrName)))
	*ppwszName = wcsDuplicate(bstrName);

	//Attribute Value
	TESTC(SUCCEEDED(pChild->get_nodeValue(&var)))
	VariantChangeType(&var, &var, 0, DBTYPE_BSTR);

	*ppwszVal = wcsDuplicate(V_BSTR(&var));

CLEANUP:
	VariantClear(&var);
	SYSSTRING_FREE(bstrName);
	SAFE_RELEASE(pAttrs);
	SAFE_RELEASE(pChild);
	TRETURN
}

//--------------------------------------------------------------
// CCmdStream::VerifyQuery2
//
BOOL CCmdStream::VerifyQuery2(IUnknown* pIStream, REFIID riid)
{
	TBEGIN
	HRESULT		hr=E_FAIL;
	WCHAR*		pwszCmd = NULL;

	TESTC(pIStream != NULL)

	TESTC_(Read(&pwszCmd, pIStream), S_OK)

	TESTC(wcsstr(pwszCmd, m_pwszQ2) != NULL)

CLEANUP:
	SAFE_FREE(pwszCmd);
	TRETURN;
}




//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------

// {{ TCW_TEST_CASE_MAP(TCGetCmdStream)
//*-----------------------------------------------------------------------
// @class Test Case for GetCommandStream method
//
class TCGetCmdStream : public CCmdStream { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetCmdStream,CCmdStream);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Verify the command object
	int Variation_1();
	// @cmember DB_E_NOCOMMAND: Get on a fresh cmd object
	int Variation_2();
	// @cmember S_OK: Set/Get ISequentialStream (unprep cmd)
	int Variation_3();
	// @cmember S_OK: Set/Get IStream (unprep cmd)
	int Variation_4();
	// @cmember S_OK: Set/Get ILockBytes (unprep cmd)
	int Variation_5();
	// @cmember S_OK: Set/Get ISequentialStream (prep cmd)
	int Variation_6();
	// @cmember S_OK: Set/Get IStream (prep cmd)
	int Variation_7();
	// @cmember S_OK: Set/Get ILockBytes (prep cmd)
	int Variation_8();
	// @cmember S_OK: Set/Get ISequentialStream (cmd props, optional)
	int Variation_9();
	// @cmember S_OK: Set/Get ISequentialStream (cmd props, required)
	int Variation_10();
	// @cmember S_OK: Set/Get IStream (cmd props)
	int Variation_11();
	// @cmember S_OK: Set/Get ILockBytes (cmd props)
	int Variation_12();
	// @cmember S_OK: Set, Execute (result open), Get
	int Variation_13();
	// @cmember S_OK: Set, Execute (result release), Get
	int Variation_14();
	// @cmember S_OK: Set/Get. Verify cmd stream object lifetime
	int Variation_15();
	// @cmember S_OK: Overwrite command in stream got from GetCmdStream.
	int Variation_16();
	// @cmember S_OK: SetCmdStream. Change underlying cmd. Execute.
	int Variation_17();
	// @cmember S_OK: Set/Get unaffected by Abort transaction.
	int Variation_18();
	// @cmember S_OK: CreateCommand with IID_ICommandStream.
	int Variation_19();
	// @cmember S_OK: GetSpecification with IID_ICommandStream.
	int Variation_20();
	// @cmember S_OK: Test stream got from GetCmdStream (seek, clone, read 1 byte at a time)
	int Variation_21();
	// @cmember S_OK: GetCmdStream. Read partially. GetCmdStream. Read whole.
	int Variation_22();
	// @cmember S_OK: Get ICommandStream on a ROW object's cmd obj.
	int Variation_23();
	// @cmember S_OK: pGuidDialect is NULL
	int Variation_24();
	// @cmember S_OK: piid is NULL
	int Variation_25();
	// @cmember S_OK: piid & pGuidDialect are NULL
	int Variation_26();
	// @cmember S_OK: Set/Get an empty stream
	int Variation_27();
	// @cmember S_OK: Set/Get a very large stream
	int Variation_28();
	// @cmember S_OK: test limitation of using ISequentialStream
	int Variation_29();
	// @cmember S_OK: SetCmdStream, SetCmdStream, GetCmdStream
	int Variation_30();
	// @cmember S_OK: SetCmdStream, SetCmdStream (different cmd), GetCmdStream
	int Variation_31();
	// @cmember S_OK: SetCmdStream, SetCmdStream (different IID), GetCmdStream
	int Variation_32();
	// @cmember S_OK: SetCmdStream, GetCmdStream, GetCmdStream
	int Variation_33();
	// @cmember S_OK: SetCmdText, SetCmdStream, GetCmdText, GetCmdStream
	int Variation_34();
	// @cmember S_OK: SetCmdStream, SetCmdText, GetCmdStream, GetCmdText
	int Variation_35();
	// @cmember S_OK: SetCmdStream, SetCmdText (make call fail), GetCmdStream succeeds.
	int Variation_36();
	// @cmember S_OK: SetCmdText, SetCmdStream (make call fail), GetCmdText succeeds.
	int Variation_37();
	// @cmember DB_S_DIALECTIGNORED: Set and Get with different GUIDS
	int Variation_38();
	// @cmember E_INVALIDARG: ppCommandStream is NULL
	int Variation_39();
	// @cmember DB_E_NOCOMMAND: SetCmdStream, SetCmdStream(NULL), GetCmdStream
	int Variation_40();
	// @cmember DB_E_NOCOMMAND: SetCmdStream, SetCmdText (NULL), GetCmdStream
	int Variation_41();
	// @cmember DB_E_CANTTRANSLATE: SetCmdText, GetCmdStream
	int Variation_42();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGetCmdStream)
#define THE_CLASS TCGetCmdStream
BEG_TEST_CASE(TCGetCmdStream, CCmdStream, L"Test Case for GetCommandStream method")
	TEST_VARIATION(1, 		L"Verify the command object")
	TEST_VARIATION(2, 		L"DB_E_NOCOMMAND: Get on a fresh cmd object")
	TEST_VARIATION(3, 		L"S_OK: Set/Get ISequentialStream (unprep cmd)")
	TEST_VARIATION(4, 		L"S_OK: Set/Get IStream (unprep cmd)")
	TEST_VARIATION(5, 		L"S_OK: Set/Get ILockBytes (unprep cmd)")
	TEST_VARIATION(6, 		L"S_OK: Set/Get ISequentialStream (prep cmd)")
	TEST_VARIATION(7, 		L"S_OK: Set/Get IStream (prep cmd)")
	TEST_VARIATION(8, 		L"S_OK: Set/Get ILockBytes (prep cmd)")
	TEST_VARIATION(9, 		L"S_OK: Set/Get ISequentialStream (cmd props, optional)")
	TEST_VARIATION(10, 		L"S_OK: Set/Get ISequentialStream (cmd props, required)")
	TEST_VARIATION(11, 		L"S_OK: Set/Get IStream (cmd props)")
	TEST_VARIATION(12, 		L"S_OK: Set/Get ILockBytes (cmd props)")
	TEST_VARIATION(13, 		L"S_OK: Set, Execute (result open), Get")
	TEST_VARIATION(14, 		L"S_OK: Set, Execute (result release), Get")
	TEST_VARIATION(15, 		L"S_OK: Set/Get. Verify cmd stream object lifetime")
	TEST_VARIATION(16, 		L"S_OK: Overwrite command in stream got from GetCmdStream.")
	TEST_VARIATION(17, 		L"S_OK: SetCmdStream. Change underlying cmd. Execute.")
	TEST_VARIATION(18, 		L"S_OK: Set/Get unaffected by Abort transaction.")
	TEST_VARIATION(19, 		L"S_OK: CreateCommand with IID_ICommandStream.")
	TEST_VARIATION(20, 		L"S_OK: GetSpecification with IID_ICommandStream.")
	TEST_VARIATION(21, 		L"S_OK: Test stream got from GetCmdStream (seek, clone, read 1 byte at a time)")
	TEST_VARIATION(22, 		L"S_OK: GetCmdStream. Read partially. GetCmdStream. Read whole.")
	TEST_VARIATION(23, 		L"S_OK: Get ICommandStream on a ROW object's cmd obj.")
	TEST_VARIATION(24, 		L"S_OK: pGuidDialect is NULL")
	TEST_VARIATION(25, 		L"S_OK: piid is NULL")
	TEST_VARIATION(26, 		L"S_OK: piid & pGuidDialect are NULL")
	TEST_VARIATION(27, 		L"S_OK: Set/Get an empty stream")
	TEST_VARIATION(28, 		L"S_OK: Set/Get a very large stream")
	TEST_VARIATION(29, 		L"S_OK: test limitation of using ISequentialStream")
	TEST_VARIATION(30, 		L"S_OK: SetCmdStream, SetCmdStream, GetCmdStream")
	TEST_VARIATION(31, 		L"S_OK: SetCmdStream, SetCmdStream (different cmd), GetCmdStream")
	TEST_VARIATION(32, 		L"S_OK: SetCmdStream, SetCmdStream (different IID), GetCmdStream")
	TEST_VARIATION(33, 		L"S_OK: SetCmdStream, GetCmdStream, GetCmdStream")
	TEST_VARIATION(34, 		L"S_OK: SetCmdText, SetCmdStream, GetCmdText, GetCmdStream")
	TEST_VARIATION(35, 		L"S_OK: SetCmdStream, SetCmdText, GetCmdStream, GetCmdText")
	TEST_VARIATION(36, 		L"S_OK: SetCmdStream, SetCmdText (make call fail), GetCmdStream succeeds.")
	TEST_VARIATION(37, 		L"S_OK: SetCmdText, SetCmdStream (make call fail), GetCmdText succeeds.")
	TEST_VARIATION(38, 		L"DB_S_DIALECTIGNORED: Set and Get with different GUIDS")
	TEST_VARIATION(39, 		L"E_INVALIDARG: ppCommandStream is NULL")
	TEST_VARIATION(40, 		L"DB_E_NOCOMMAND: SetCmdStream, SetCmdStream(NULL), GetCmdStream")
	TEST_VARIATION(41, 		L"DB_E_NOCOMMAND: SetCmdStream, SetCmdText (NULL), GetCmdStream")
	TEST_VARIATION(42, 		L"DB_E_CANTTRANSLATE: SetCmdText, GetCmdStream")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCSetCmdStream)
//*-----------------------------------------------------------------------
// @class SetCommandStream Test Case
//
class TCSetCmdStream : public CCmdStream { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSetCmdStream,CCmdStream);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember S_OK: Set cmd props. SetCmdStream. Execute.
	int Variation_1();
	// @cmember S_OK: Execute CmdText, release rowset, SetCmdStream
	int Variation_2();
	// @cmember S_OK: Set/Get/Execute. Verify cmd stream object lifetime
	int Variation_3();
	// @cmember S_OK: Execute some privlib queries and verify.
	int Variation_4();
	// @cmember S_OK: pCommandStream is NULL (cmd state reset)
	int Variation_5();
	// @cmember S_OK: SetCmdStream with stream containing 2 cmds. Ptr on 2nd.
	int Variation_6();
	// @cmember S_OK: SetCmdStream & Execute diffr queries on diffr cmd objs.
	int Variation_7();
	// @cmember S_OK: SetCmdStream, Prepare, GetColInfo.
	int Variation_8();
	// @cmember S_OK: SetCmdStream, Prepare, GetColumnsRowset.
	int Variation_9();
	// @cmember S_OK: Implement stream on a large file. SetCmdStream.
	int Variation_10();
	// @cmember S_OK: Aggregate stream got from Execute.
	int Variation_11();
	// @cmember DB_E_ERRORSINCOMMAND: SetCmdStream with an embedded NULL terminator.
	int Variation_12();
	// @cmember DB_E_ERRORSINCOMMAND: Set unicode stream without BOM
	int Variation_13();
	// @cmember S_OK: Set stream as ansi - no BOM
	int Variation_14();
	// @cmember S_OK: Set DBPROP_DEFERRED.
	int Variation_15();
	// @cmember S_OK: Call SetCmdStream so it fails. Ref cnt unchanged.
	int Variation_16();
	// @cmember S_OK: Release all objects & verify no ref cnts left on stream.
	int Variation_17();
	// @cmember S_OK: SetCmdStream, GetCmdStream, SetCmdStream with stream got. Execute.
	int Variation_18();
	// @cmember S_OK: SetCmdStream, Execute, GetCmdStream
	int Variation_19();
	// @cmember S_OK: SetCmdStream, GetCmdStream, Execute
	int Variation_20();
	// @cmember S_OK: SetCmdStream, Execute, Execute
	int Variation_21();
	// @cmember S_OK: SetCmdText, SetCmdStream, Execute
	int Variation_22();
	// @cmember S_OK: SetCmdStream, GetCmdStream, SetCmdText, Execute. Read got stream.
	int Variation_23();
	// @cmember S_OK: Ask for IUnknown on Execute.
	int Variation_24();
	// @cmember DBPROP_OUTPUTSTREAM - ISequentialStream
	int Variation_25();
	// @cmember DBPROP_OUTPUTSTREAM - ISequentialStream (NULL ppRowset)
	int Variation_26();
	// @cmember DBPROP_OUTPUTSTREAM - IStream
	int Variation_27();
	// @cmember DBPROP_OUTPUTENCODING - UTF-8
	int Variation_28();
	// @cmember DBPROP_OUTPUTENCODING - UTF-16
	int Variation_29();
	// @cmember E_NOINTERFACE: iid passed in does not match object
	int Variation_30();
	// @cmember E_NOINTERFACE: iid is IRowset & IRow.
	int Variation_31();
	// @cmember DB_E_CANTTRANSLATE: SetCmdStream, GetCmdText
	int Variation_32();
	// @cmember DB_E_DIALECTNOTSUPPORTED: Use some bugs guids & DBGUID_NULL
	int Variation_33();
	// @cmember DB_E_DIALECTNOTSUPPORTED (or S_OK): use DBGUID_SQL.
	int Variation_34();
	// @cmember DB_E_OBJECTOPEN: Keeping a result stream open, call Set.
	int Variation_35();
	// @cmember Clear previous error object.
	int Variation_36();
	// @cmember Extended error test - GetCmdStream
	int Variation_37();
	// @cmember Extended error test - SetCmdStream
	int Variation_38();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCSetCmdStream)
#define THE_CLASS TCSetCmdStream
BEG_TEST_CASE(TCSetCmdStream, CCmdStream, L"SetCommandStream Test Case")
	TEST_VARIATION(1, 		L"S_OK: Set cmd props. SetCmdStream. Execute.")
	TEST_VARIATION(2, 		L"S_OK: Execute CmdText, release rowset, SetCmdStream")
	TEST_VARIATION(3, 		L"S_OK: Set/Get/Execute. Verify cmd stream object lifetime")
	TEST_VARIATION(4, 		L"S_OK: Execute some privlib queries and verify.")
	TEST_VARIATION(5, 		L"S_OK: pCommandStream is NULL (cmd state reset)")
	TEST_VARIATION(6, 		L"S_OK: SetCmdStream with stream containing 2 cmds. Ptr on 2nd.")
	TEST_VARIATION(7, 		L"S_OK: SetCmdStream & Execute diffr queries on diffr cmd objs.")
	TEST_VARIATION(8, 		L"S_OK: SetCmdStream, Prepare, GetColInfo.")
	TEST_VARIATION(9, 		L"S_OK: SetCmdStream, Prepare, GetColumnsRowset.")
	TEST_VARIATION(10, 		L"S_OK: Implement stream on a large file. SetCmdStream.")
	TEST_VARIATION(11, 		L"S_OK: Aggregate stream got from Execute.")
	TEST_VARIATION(12, 		L"DB_E_ERRORSINCOMMAND: SetCmdStream with an embedded NULL terminator.")
	TEST_VARIATION(13, 		L"DB_E_ERRORSINCOMMAND: Set unicode stream without BOM")
	TEST_VARIATION(14, 		L"S_OK: Set stream as ansi - no BOM")
	TEST_VARIATION(15, 		L"S_OK: Set DBPROP_DEFERRED.")
	TEST_VARIATION(16, 		L"S_OK: Call SetCmdStream so it fails. Ref cnt unchanged.")
	TEST_VARIATION(17, 		L"S_OK: Release all objects & verify no ref cnts left on stream.")
	TEST_VARIATION(18, 		L"S_OK: SetCmdStream, GetCmdStream, SetCmdStream with stream got. Execute.")
	TEST_VARIATION(19, 		L"S_OK: SetCmdStream, Execute, GetCmdStream")
	TEST_VARIATION(20, 		L"S_OK: SetCmdStream, GetCmdStream, Execute")
	TEST_VARIATION(21, 		L"S_OK: SetCmdStream, Execute, Execute")
	TEST_VARIATION(22, 		L"S_OK: SetCmdText, SetCmdStream, Execute")
	TEST_VARIATION(23, 		L"S_OK: SetCmdStream, GetCmdStream, SetCmdText, Execute. Read got stream.")
	TEST_VARIATION(24, 		L"S_OK: Ask for IUnknown on Execute.")
	TEST_VARIATION(25, 		L"DBPROP_OUTPUTSTREAM - ISequentialStream")
	TEST_VARIATION(26, 		L"DBPROP_OUTPUTSTREAM - ISequentialStream (NULL ppRowset)")
	TEST_VARIATION(27, 		L"DBPROP_OUTPUTSTREAM - IStream")
	TEST_VARIATION(28, 		L"DBPROP_OUTPUTENCODING - UTF-8")
	TEST_VARIATION(29, 		L"DBPROP_OUTPUTENCODING - UTF-16")
	TEST_VARIATION(30, 		L"E_NOINTERFACE: iid passed in does not match object")
	TEST_VARIATION(31, 		L"E_NOINTERFACE: iid is IRowset & IRow.")
	TEST_VARIATION(32, 		L"DB_E_CANTTRANSLATE: SetCmdStream, GetCmdText")
	TEST_VARIATION(33, 		L"DB_E_DIALECTNOTSUPPORTED: Use some bugs guids & DBGUID_NULL")
	TEST_VARIATION(34, 		L"DB_E_DIALECTNOTSUPPORTED (or S_OK): use DBGUID_SQL.")
	TEST_VARIATION(35, 		L"DB_E_OBJECTOPEN: Keeping a result stream open, call Set.")
	TEST_VARIATION(36, 		L"Clear previous error object.")
	TEST_VARIATION(37, 		L"Extended error test - GetCmdStream")
	TEST_VARIATION(38, 		L"Extended error test - SetCmdStream")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// }} END_DECLARE_TEST_CASES()



// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(2, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGetCmdStream)
	TEST_CASE(2, TCSetCmdStream)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END



// {{ TCW_TC_PROTOTYPE(TCGetCmdStream)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetCmdStream - Test Case for GetCommandStream method
//| Created:  	10/26/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetCmdStream::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCmdStream::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Verify the command object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_1()
{ 
	TBEGIN

	TESTC(DefaultObjectTesting(m_pICommandStream, COMMAND_INTERFACE))

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: Get on a fresh cmd object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_2()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid = IID_IAccessor;
	GUID		guid = DBGUID_DEFAULT;

	TESTC_(hr = GetCmdStream(&iid, &guid), DB_E_NOCOMMAND)

CLEANUP:
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ISequentialStream (unprep cmd)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_3()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		TEST2C_(hr = m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL), S_OK, E_OUTOFMEMORY)
		if(hr != S_OK)
		{
			odtLog<<L"INFO: Could not create SQL stmt for "<<GetSQLTokenName((EQUERY)rgSQLTokens[i].lItem)<<".\n";
			continue;
		}
		
		TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ISequentialStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get IStream (unprep cmd)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_4()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		TEST2C_(hr = m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL), S_OK, E_OUTOFMEMORY)
		if(hr != S_OK)
		{
			odtLog<<L"INFO: Could not create SQL stmt for "<<GetSQLTokenName((EQUERY)rgSQLTokens[i].lItem)<<".\n";
			continue;
		}
		
		//IID_IStream may not be supported.
		TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, pwszSQL), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_IStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ILockBytes (unprep cmd)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_5()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		TEST2C_(hr = m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL), S_OK, E_OUTOFMEMORY)
		if(hr != S_OK)
		{
			odtLog<<L"INFO: Could not create SQL stmt for "<<GetSQLTokenName((EQUERY)rgSQLTokens[i].lItem)<<".\n";
			continue;
		}
		
		//IID_IStream may not be supported.
		TEST2C_(hr = SetCmdStream(IID_ILockBytes, DBGUID_DEFAULT, pwszSQL), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ILockBytes);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ISequentialStream (prep cmd)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_6()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

		//Prepare the command.
		TESTC(PrepCmd())

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ISequentialStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get IStream (prep cmd)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_7()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		//IID_IStream may not be supported.
		TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, pwszSQL), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK)

		//Prepare the command.
		TESTC(PrepCmd())

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_IStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ILockBytes (prep cmd)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_8()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	//Loop through all the possible Queries...
	for(ULONG i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		//IID_IStream may not be supported.
		TEST2C_(hr = SetCmdStream(IID_ILockBytes, DBGUID_DEFAULT, pwszSQL), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK)

		//Prepare the command.
		TESTC(PrepCmd())

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ILockBytes);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ISequentialStream (cmd props, optional)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_9()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)(DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE|DBPROPVAL_UP_INSERT), DBTYPE_I4, DBPROPOPTIONS_OPTIONAL);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_S_ERRORSOCCURRED, TRUE);

	ULONG i;
	//Loop through all the possible Queries...
	for(i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ISequentialStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	//unset the 3 properties which were set above.
	for(i=0; i<3; i++)
		VariantClear(&rgPropSets[0].rgProperties[i].vValue);
	SetCmdProp(rgPropSets);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ISequentialStream (cmd props, required)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_10()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_E_ERRORSOCCURRED, TRUE);

	ULONG i;
	//Loop through all the possible Queries...
	for(i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ISequentialStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	//unset the 3 properties which were set above.
	for(i=0; i<3; i++)
		VariantClear(&rgPropSets[0].rgProperties[i].vValue);
	SetCmdProp(rgPropSets);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get IStream (cmd props)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_11()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED, TRUE);

	ULONG i;
	//Loop through all the possible Queries...
	for(i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, pwszSQL), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_IStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	//unset the 3 properties which were set above.
	for(i=0; i<3; i++)
		VariantClear(&rgPropSets[0].rgProperties[i].vValue);
	SetCmdProp(rgPropSets);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get ILockBytes (cmd props)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_12()
{ 
	TBEGIN
	ULONG		i=0;
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	NAMEMAP*	rgSQLTokens = NULL;
	ULONG		cSQLTokens = GetSQLTokenMap(&rgSQLTokens);

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	SetProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED, TRUE);

	//Loop through all the possible Queries...
	for(i=0; i<cSQLTokens; i++)
	{
		ALLOC_SETSTG;

		if(m_pTable->CreateSQLStmt((EQUERY)rgSQLTokens[i].lItem, m_pTable->GetTableName(), &pwszSQL) != S_OK)
			continue;
		
		TEST2C_(hr = SetCmdStream(IID_ILockBytes, DBGUID_DEFAULT, pwszSQL), S_OK, E_NOINTERFACE)
		TESTC_PROVIDER(hr == S_OK)

		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ILockBytes);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		SAFE_FREE(pwszSQL);
		SAFE_FREE(pwszSQLGet);
		RELEASE_GETSTG;
		RELEASE_SETSTG;
	}

CLEANUP:
	//unset the 3 properties which were set above.
	for(i=0; i<3; i++)
		VariantClear(&rgPropSets[0].rgProperties[i].vValue);
	SetCmdProp(rgPropSets);
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set, Execute (result open), Get
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_13()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	ISequentialStream*	pISS = NULL;
	WCHAR* pwszCmd = NULL;

	ALLOC_SETSTG;
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

	TESTC(pwszSQLGet!=NULL)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set, Execute (result release), Get
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_14()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))
	SAFE_RELEASE(pISS);

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

	TESTC(pwszSQLGet!=NULL)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get. Verify cmd stream object lifetime
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_15()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid;
	ULONG		cRef1=0, cRef2=0, cRef3=0;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	cRef1 = m_pStorageSet->GetRefCount();

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	cRef2 = m_pStorageSet->GetRefCount();
	COMPARE(cRef2 > cRef1, TRUE);

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	cRef3 = m_pStorageSet->GetRefCount();
	COMPARE(cRef3 > cRef2, TRUE);

	COMPARE(pwszSQL && pwszSQLGet, TRUE);
	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);

	m_pStorageGet->Release();
	COMPARE(cRef2, m_pStorageGet->GetRefCount());
	m_pStorageGet = NULL;

	m_pStorageSet->Release();
	COMPARE(cRef1, m_pStorageSet->GetRefCount());
	m_pStorageSet = NULL;

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Overwrite command in stream got from GetCmdStream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_16()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	WCHAR*		pwszStream = NULL;
	ULONG		cWrote = 0;
	ULONG		ulLen = 0;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), S_OK)

	//Overwrite command in the m_pStorageGet stream.
	ulLen = (ULONG)wcslen(m_pwszQ2);
	TESTC_(hr = m_pStorageGet->Write(wszByteOrder, 2, NULL), S_OK);
	TESTC_(hr = m_pStorageGet->Write(m_pwszQ2, (ulLen)*sizeof(WCHAR), &cWrote), S_OK);
	m_pStorageGet->Seek(0);

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_FREE(pwszStream);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream. Change underlying cmd. Execute.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_17()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	WCHAR*		pwszSQLGet = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	//Overwrite the stream that was set with another cmd.
	TESTC_(Write(m_pwszQ2), S_OK)

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get unaffected by Abort transaction.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_18()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	ULONG		ulTLevel = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	ITransactionLocal*	pITL = NULL;

	ALLOC_SETSTG;

	TESTC_PROVIDER(VerifyInterface(m_pIOpenRowset, 
		IID_ITransactionLocal, SESSION_INTERFACE,(IUnknown **)&pITL))

	TESTC_(pITL->StartTransaction(ISOLATIONLEVEL_READUNCOMMITTED,0,NULL,&ulTLevel), S_OK)

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(pITL->Abort(NULL, FALSE,NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pITL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK: CreateCommand with IID_ICommandStream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_19()
{ 
	TBEGIN
	IDBCreateCommand*	pIDBCC = NULL;
	ICommandStream*		pICS = NULL;
	ICommandText*		pICT = NULL;

	TESTC(VerifyInterface(m_pIOpenRowset, 
		IID_IDBCreateCommand, SESSION_INTERFACE,(IUnknown **)&pIDBCC))

	TESTC_(pIDBCC->CreateCommand(NULL, IID_ICommandStream, (IUnknown**)&pICS), S_OK)
	TESTC(DefTest(pICS))

	TESTC(VerifyInterface(pICS, IID_ICommandText, 
		COMMAND_INTERFACE,(IUnknown **)&pICT))
	TESTC(DefaultInterfaceTesting(pICT, COMMAND_INTERFACE, IID_ICommandText))

CLEANUP:
	SAFE_RELEASE(pIDBCC);
	SAFE_RELEASE(pICS);
	SAFE_RELEASE(pICT);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK: GetSpecification with IID_ICommandStream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_20()
{ 
	TBEGIN
	IRowsetInfo*		pIRI = NULL;
	ICommandStream*		pICS = NULL;

	TESTC_(m_pTable->ExecuteCommand(SELECT_ALLFROMTBL,
		IID_IRowsetInfo, NULL, NULL, NULL, NULL, 
		EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIRI, 
		(ICommand**)&m_pICommandText), S_OK)
	TESTC(pIRI != NULL)

	TESTC_(pIRI->GetSpecification(IID_ICommandStream, (IUnknown**)&pICS), S_OK)
	TESTC(DefTest(pICS))

CLEANUP:
	SAFE_RELEASE(pIRI);
	SAFE_RELEASE(pICS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Test stream got from GetCmdStream (seek, clone, read 1 byte at a time)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_21()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	ULONG		cRead = 0;
	ULONG		ulReadIncr = 1;
	BYTE*		pwszEnd = NULL;
	BOOL		fEnd = FALSE;
	IStream*	pClone = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_PROVIDER(SetCmdStream(IID_IStream, DBGUID_DEFAULT, pwszSQL) == S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), S_OK)
	COMPARE(iid, IID_IStream);

	SAFE_ALLOC(pwszSQLGet, WCHAR, READ_INCR);

	//----------------------------------------------------- 
	//Read 1 byte at a time.
	//-----------------------------------------------------

	hr = m_pStorageGet->Read(pwszSQLGet, 2, &cRead);
	if(cRead==2)
	{
		if(memcmp(wszByteOrder, pwszSQLGet, 2) == 0)
		{
			hr = m_pStorageGet->Read(pwszSQLGet, 1, &cRead);
			hr = m_pStorageGet->Read((BYTE*)pwszSQLGet+1, 1, &cRead);
		}
	}

	//Read only 1 byte at a time.
	while(!fEnd)
	{
		cRead = 0;
		ulReadIncr += 1;
		if((ulReadIncr/2 > READ_INCR) && !(ulReadIncr%2))
			SAFE_REALLOC(pwszSQLGet, WCHAR, READ_INCR+ulReadIncr/2);
		hr = m_pStorageGet->Read((BYTE*)pwszSQLGet+ulReadIncr, 1, &cRead);
		if(cRead < 1)
		{
			fEnd = TRUE;
			pwszEnd = (BYTE*)pwszSQLGet +ulReadIncr;
		}
	}

	memset((BYTE*)pwszEnd, 0, 2);

	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);
	SAFE_FREE(pwszSQLGet);
	ulReadIncr = 1;
	cRead = 0;

	//----------------------------------------------------- 
	// CLONE stream. Read cloned stream.
	//-----------------------------------------------------

	TEST2C_(hr = m_pStorageGet->Clone(&pClone), S_OK, E_NOTIMPL)

	if(hr != S_OK)
	{
		odtLog<<L"WARNING: Got back an IStream which does not implement the method CLONE.\n";
		goto SEEK;
	}

	TESTC(pClone != NULL)

	TESTC_(Read(&pwszSQLGet, pClone), S_OK)

	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);
	SAFE_FREE(pwszSQLGet);
	ulReadIncr = 1;
	cRead = 0;

	//----------------------------------------------------- 
	// SEEK original stream. 
	//-----------------------------------------------------
SEEK:

	TESTC_(m_pStorageGet->Seek(0), S_OK)

	TESTC_(Read(&pwszSQLGet, (ISequentialStream*)m_pStorageGet), S_OK)

	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pClone);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc S_OK: GetCmdStream. Read partially. GetCmdStream. Read whole.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_22()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	WCHAR*		pwszSQLGet2 = NULL;
	ULONG		cRead = 0;
	ULONG		ulReadIncr = 1;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), S_OK)

	SAFE_ALLOC(pwszSQLGet, WCHAR, 3000);

	//Read first stream partially.
	hr = m_pStorageGet->Read(pwszSQLGet, 4, &cRead);

	//Get another stream and read it fully.
	TEST2C_(hr = m_pICommandStream->GetCommandStream(&iid, &guid, (IUnknown**)&pISS), S_OK, DB_S_DIALECTIGNORED)

	TESTC_(Read(&pwszSQLGet, pISS), S_OK)

	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	SAFE_FREE(pwszSQLGet2);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Get ICommandStream on a ROW object's cmd obj.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_23()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cProps = 0;
	DBPROPSET*			rgProps = NULL;
	IDBCreateCommand*	pIDBCC = NULL;
	ICommandStream*		pICS = NULL;

	SetProperty(DBPROP_IRow, DBPROPSET_ROWSET, &cProps, &rgProps);

	TEST3C_(hr = m_pIOpenRowset->OpenRowset(NULL, 
		&(m_pTable->GetTableID()), NULL, IID_IDBCreateCommand,
		cProps, rgProps, (IUnknown **)&pIDBCC), S_OK, 
		E_NOINTERFACE, DB_E_ERRORSOCCURRED)

	TESTC_PROVIDER(hr == S_OK)
	TESTC_(pIDBCC->CreateCommand(NULL, IID_ICommandStream, (IUnknown**)&pICS), S_OK)
	TESTC(DefTest(pICS))

CLEANUP:
	FreeProperties(&cProps, &rgProps);
	SAFE_RELEASE(pIDBCC);
	SAFE_RELEASE(pICS);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc S_OK: pGuidDialect is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_24()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, NULL, &pwszSQLGet), S_OK)

	COMPARE(pwszSQL && pwszSQLGet, TRUE);
	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc S_OK: piid is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_25()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(NULL, &guid, &pwszSQLGet), S_OK)

	COMPARE(pwszSQL && pwszSQLGet, TRUE);
	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc S_OK: piid & pGuidDialect are NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_26()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(NULL, NULL, &pwszSQLGet), S_OK)

	COMPARE(pwszSQL && pwszSQLGet, TRUE);
	COMPARE(wcscmp(pwszSQL, pwszSQLGet), 0);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get an empty stream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_27()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ULONG		cRead = 0;
	WCHAR*		pwszSQLGet;

	ALLOC_SETSTG;
	
	//Create the stream object, but don't write to it.
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

	COMPARE(pwszSQLGet, NULL);

	//Create the stream object, but don't write to it.
	TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, NULL), S_OK, E_NOINTERFACE)

	if(hr == S_OK)
	{
		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

		COMPARE(pwszSQLGet, NULL);
	}

	//Create the stream object, but don't write to it.
	TEST2C_(hr = SetCmdStream(IID_ILockBytes, DBGUID_DEFAULT, NULL), S_OK, E_NOINTERFACE)

	if(hr == S_OK)
	{
		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

		COMPARE(pwszSQLGet, NULL);
	}

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get a very large stream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_28()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	ULONG		i;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	WCHAR*		pwszSQLLong = NULL;
	ULONG		cWrote = 0;
	ULONG		ulLen = 0;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_COLLISTFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	ulLen = (ULONG)wcslen(pwszSQL);

	hr = m_pStorageSet->Write(wszByteOrder, 2, NULL);

	for(i=0; i<100; i++)
		hr = m_pStorageSet->Write(pwszSQL, (ulLen)*sizeof(WCHAR), &cWrote);

	hr = m_pStorageSet->Write(pwszSQL, (ulLen)*sizeof(WCHAR), &cWrote);
	m_pStorageSet->Seek(0);

	SAFE_ALLOC(pwszSQLLong, WCHAR, (101*ulLen)+1);
	wcscpy(pwszSQLLong, pwszSQL);
	for(i=0; i<100; i++)
		wcscat(pwszSQLLong, pwszSQL);

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT), S_OK)

	TESTC_(hr = GetCmdStream(NULL, &guid, &pwszSQLGet), S_OK)

	TESTC(pwszSQLLong && pwszSQLGet)
	COMPARE(wcscmp(pwszSQLLong, pwszSQLGet), 0);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLLong);
	SAFE_FREE(pwszSQLGet);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc S_OK: test limitation of using ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_29()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ISequentialStream*	pIS1 = NULL;
	ISequentialStream*	pIS2 = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ2), S_OK)

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pIS1), S_OK)
	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pIS1, IID_ISequentialStream))

	TEST2C_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pIS2), S_OK, DB_E_ERRORSINCOMMAND)
	//Verify that query2 was executed.
	if(hr == S_OK)
	{
		TESTC(VerifyQuery2(pIS2, IID_ISequentialStream))
		odtLog<<L"INFO: An ISequentialStream command can be executed multiple times.\n";
	}
	else
		odtLog<<L"INFO: An ISequentialStream command CANNOT be executed multiple times.\n";

CLEANUP:
	SAFE_RELEASE(pIS1);
	SAFE_RELEASE(pIS2);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, SetCmdStream, GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_30()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, SetCmdStream (different cmd), GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_31()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	SAFE_FREE(pwszSQL);
	RELEASE_SETSTG;
	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_COLLISTFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, SetCmdStream (different IID), GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_32()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	HRESULT		hrSet = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQL2 = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_COLLISTFROMTBL, m_pTable->GetTableName(), &pwszSQL2), S_OK)

	RELEASE_SETSTG;
	ALLOC_SETSTG;

	TEST2C_(hrSet = SetCmdStream(IID_ILockBytes, DBGUID_DEFAULT, pwszSQL2), S_OK, E_NOINTERFACE)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

	if(hrSet == S_OK)
	{
		COMPARE(iid, IID_ILockBytes);
		TESTC(pwszSQL2 && pwszSQLGet)
		TESTC(wcscmp(pwszSQL2, pwszSQLGet) == 0)
	}
	else
	{
		COMPARE(iid, IID_ISequentialStream);
		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQL2);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, GetCmdStream, GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_33()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	ULONG		i;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	for(i=0; i<5; i++)
	{
		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_ISequentialStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		RELEASE_GETSTG;
		SAFE_FREE(pwszSQLGet);
	}

	//Set same command, but with different iid.
	TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, NULL), S_OK, E_NOINTERFACE)
	TESTC_PROVIDER(hr == S_OK)

	for(i=0; i<5; i++)
	{
		TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
		COMPARE(iid, IID_IStream);

		TESTC(pwszSQL && pwszSQLGet)
		TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

		RELEASE_GETSTG;
		SAFE_FREE(pwszSQLGet);
	}

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdText, SetCmdStream, GetCmdText, GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_34()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid;
	WCHAR*		pwszCmd = NULL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = m_pICommandText->GetCommandText(&guid, &pwszCmd), DB_E_CANTTRANSLATE)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, SetCmdText, GetCmdStream, GetCmdText
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_35()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_CANTTRANSLATE)

	TEST2C_(hr = m_pICommandText->GetCommandText(&guid, &pwszSQLGet), S_OK, DB_S_DIALECTIGNORED)

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, SetCmdText (make call fail), GetCmdStream succeeds.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_36()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DSO, pwszSQL), DB_E_DIALECTNOTSUPPORTED)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdText, SetCmdStream (make call fail), GetCmdText succeeds.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_37()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DSO, pwszSQL), DB_E_DIALECTNOTSUPPORTED)

	TESTC_(hr = m_pICommandText->GetCommandText(&guid, &pwszSQLGet), S_OK)

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc DB_S_DIALECTIGNORED: Set and Get with different GUIDS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_38()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_SQL;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = m_pICommandStream->GetCommandStream(&iid, &guid, (IUnknown**)&m_pStorageGet), DB_S_DIALECTIGNORED)
	COMPARE(iid, IID_ISequentialStream);

	COMPARE(m_pStorageGet != NULL, TRUE);

	TESTC_(hr = Read(&pwszSQLGet), S_OK)

	TESTC(pwszSQL && pwszSQLGet)
	TESTC(wcscmp(pwszSQL, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: ppCommandStream is NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_39()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid = IID_IRowset;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = m_pICommandStream->GetCommandStream(&iid, &guid, NULL), E_INVALIDARG)

	COMPARE(guid, GUIDNULL);
	COMPARE(iid, IID_NULL);

	iid = IID_IRowset;
	TESTC_(hr = m_pICommandStream->GetCommandStream(&iid, NULL, NULL), E_INVALIDARG)
	COMPARE(iid, IID_NULL);

	guid = DBGUID_DEFAULT;
	TESTC_(hr = m_pICommandStream->GetCommandStream(NULL, &guid, NULL), E_INVALIDARG)
	COMPARE(guid, GUIDNULL);

	TESTC_(hr = m_pICommandStream->GetCommandStream(NULL, NULL, NULL), E_INVALIDARG)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: SetCmdStream, SetCmdStream(NULL), GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_40()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	//Set a command stream.
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	//reset cmd stream.
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_NOCOMMAND)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc DB_E_NOCOMMAND: SetCmdStream, SetCmdText (NULL), GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_41()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	//Set a command stream.
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	//reset cmd stream.
	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_NOCOMMAND)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTTRANSLATE: SetCmdText, GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetCmdStream::Variation_42()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid;
	WCHAR*		pwszCmd = NULL;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_CANTTRANSLATE)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCGetCmdStream::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCmdStream::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCSetCmdStream)
//*-----------------------------------------------------------------------
//| Test Case:		TCSetCmdStream - SetCommandStream Test Case
//| Created:  	11/15/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSetCmdStream::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CCmdStream::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set cmd props. SetCmdStream. Execute.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_1()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	WCHAR*		pwszSQLGet = NULL;
	ICommand*	pICommand = NULL;
	ISequentialStream*	pISS = NULL;

	SetProperty(DBPROP_IRowset, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	//SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	//SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, &cPropSets, &rgPropSets);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_E_ERRORSOCCURRED, TRUE);

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC(VerifyInterface(m_pICommandStream, IID_ICommand, 
		COMMAND_INTERFACE,(IUnknown **)&pICommand))

	TESTC_(pICommand->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(m_pwszQ1 && pwszSQLGet)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Execute CmdText, release rowset, SetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_2()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	WCHAR*		pwszCmd = NULL;
	WCHAR*		pwszSQL = NULL;
	IRowset*	pIR = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, NULL, &lRA, (IUnknown**)&pIR), S_OK)
	TESTC(DefTestInterface(pIR))
	SAFE_RELEASE(pIR);

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pIR);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set/Get/Execute. Verify cmd stream object lifetime
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_3()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBROWCOUNT			lRA = 0;
	IID					iid;
	GUID				guid = DBGUID_DEFAULT;
	ULONG				cRef = 0;
	ULONG				ulLen = 0;
	WCHAR*				pwszSQL = NULL;
	IDBCreateSession*	pIDBCS = NULL;
	IDBCreateCommand*	pIDBCC = NULL;
	ICommand*			pICmd = NULL;
	ICommandStream*		pICS = NULL;
	ISequentialStream*	pISS = NULL;

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IDBCreateSession, 
		(IUnknown**)&pIDBCS, CREATEDSO_SETPROPERTIES|CREATEDSO_INITIALIZE), S_OK)

	TESTC_(pIDBCS->CreateSession(NULL, IID_IDBCreateCommand, 
		(IUnknown**)&pIDBCC), S_OK)

	TESTC_(pIDBCC->CreateCommand(NULL, IID_ICommandStream, (IUnknown**)&pICS), S_OK)
	TESTC(VerifyInterface(pICS, IID_ICommand, 
		COMMAND_INTERFACE,(IUnknown **)&pICmd))

	ALLOC_SETSTG;
	cRef = m_pStorageSet->GetRefCount();

	TESTC_(Write(m_pwszQ1), S_OK)
	TESTC_(hr = pICS->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, (ISequentialStream*)m_pStorageSet), S_OK)

	TEST2C_(hr = pICS->GetCommandStream(&iid, &guid, (IUnknown**)&pISS), S_OK, DB_S_DIALECTIGNORED)
	COMPARE(iid, IID_ISequentialStream);
	SAFE_RELEASE(pISS);

	TESTC_(pICmd->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	SAFE_RELEASE(pIDBCS);
	SAFE_RELEASE(pIDBCC);
	SAFE_RELEASE(pICS);
	SAFE_RELEASE(pICmd);
	SAFE_RELEASE(pISS);

	COMPARE(m_pStorageSet->GetRefCount(), cRef);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pIDBCS);
	SAFE_RELEASE(pIDBCC);
	SAFE_RELEASE(pICS);
	SAFE_RELEASE(pICmd);
	SAFE_RELEASE(pISS);
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Execute some privlib queries and verify.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_4()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	WCHAR*		pwszSQL = NULL;
	IRowset*	pIRowset = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	//Apart from S_OK, can also expect to get -
	//DB_E_ERRORSINCOMMAND, since the type of command may not match with what DBGUID_DEFAULT stand for.
	//E_NOINTERFACE, since we are asking for IID_IRowset and Execute after SetCmdStream may only support returning stream objects.
	TEST3C_(hr = m_pICommandText->Execute(NULL, IID_IRowset, NULL, &lRA, (IUnknown**)&pIRowset), S_OK, DB_E_ERRORSINCOMMAND, E_NOINTERFACE)

	if(hr == S_OK)
		TESTC(VerifyRowset(SELECT_ALLFROMTBL, pIRowset))

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pIRowset);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc S_OK: pCommandStream is NULL (cmd state reset)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_5()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	//Set a command stream.
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	//reset cmd stream.
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_NOCOMMAND)

	RELEASE_GETSTG;
	RELEASE_SETSTG;

	ALLOC_SETSTG;

	//Set a command stream.
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	//reset cmd stream multiple times.
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)
	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_NOCOMMAND)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream with stream containing 2 cmds. Ptr on 2nd.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_6()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	WCHAR*		pwszSQL = NULL;
	ULONG		cWrote = 0;
	ULONG		ulLen = 0;
	ULONG		ulLen2 = 0;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	//Write first command to stream.
	ulLen = (ULONG)wcslen(pwszSQL);
	hr = m_pStorageSet->Write(wszByteOrder, 2, NULL);
	hr = m_pStorageSet->Write(pwszSQL, (ulLen)*sizeof(WCHAR), &cWrote);

	//Write second command to stream.
	ulLen2 = (ULONG)wcslen(m_pwszQ1);
	hr = m_pStorageSet->Write(L";", 2, NULL);
	hr = m_pStorageSet->Write(wszByteOrder, 2, NULL);
	hr = m_pStorageSet->Write(m_pwszQ1, (ulLen2)*sizeof(WCHAR), &cWrote);

	//Seek to beginning of second command.
	m_pStorageSet->Seek(2 + (ulLen)*sizeof(WCHAR) + 2);

	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, (ISequentialStream*)m_pStorageSet), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream & Execute diffr queries on diffr cmd objs.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_7()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ICommand*	pICommand = NULL;
	ICommandStream*		pICS = NULL;
	ISequentialStream*	pISS = NULL;
	ISequentialStream*	pISS2 = NULL;

	ALLOC_SETSTG;

	//Create another cmd object.
	TESTC_(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown**)&pICommand), S_OK)
	TESTC(VerifyInterface(pICommand, IID_ICommandStream, 
		COMMAND_INTERFACE,(IUnknown **)&pICS))

	//Set stream on first command.
	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	RELEASE_SETSTG;
	ALLOC_SETSTG;

	//Set stream on second command.
	TESTC_(hr = Write(m_pwszQ2), S_OK)
	TESTC_(hr = pICS->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, (ISequentialStream*)m_pStorageSet), S_OK)
	
	//Execute command on first.
	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	//Set the encoding on second command also to UTF-16
	CHECK(SetEncdProp(UTF16, (IUnknown*)pICommand), S_OK);

	//Execute command on second.
	TESTC_(pICommand->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS2), S_OK)
	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pISS2, IID_ISequentialStream))

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICS);
	SAFE_RELEASE(pISS);
	SAFE_RELEASE(pISS2);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, Prepare, GetColInfo.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_8()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	DBORDINAL		cCols = 0;
	DBCOLUMNINFO*	rgInfo = NULL;
	WCHAR*			pwszBuf = NULL;
	IColumnsInfo*	pICF = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC(PrepCmd())

	TESTC(VerifyInterface(m_pICommandStream, IID_IColumnsInfo, 
		COMMAND_INTERFACE,(IUnknown **)&pICF))

	CHECK(pICF->GetColumnInfo(&cCols, &rgInfo, &pwszBuf), S_OK);
	COMPARE(!cCols && !rgInfo && !pwszBuf, TRUE);

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(m_pwszQ1 && pwszSQLGet)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_FREE(rgInfo);
	SAFE_FREE(pwszBuf);
	SAFE_RELEASE(pICF);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, Prepare, GetColumnsRowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_9()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	DBCOUNTITEM	cRows = 0;
	HROW*		rghRows = NULL;
	IRowset*	pIR = NULL;
	IColumnsRowset*	pICR = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC(PrepCmd())

	TESTC_PROVIDER(VerifyInterface(m_pICommandStream, IID_IColumnsRowset, 
		COMMAND_INTERFACE,(IUnknown **)&pICR))

	TESTC_(pICR->GetColumnsRowset(NULL, 0, NULL, IID_IRowset, 
		0, NULL, (IUnknown**)&pIR), S_OK)
	TESTC(pIR != NULL)
	TESTC_(pIR->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRows, 
		&rghRows), DB_S_ENDOFROWSET)
	COMPARE(cRows, 0);

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(m_pwszQ1 && pwszSQLGet)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_FREE(rghRows);
	SAFE_RELEASE(pICR);
	SAFE_RELEASE(pIR);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Implement stream on a large file. SetCmdStream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_10()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Aggregate stream got from Execute.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_11()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	IUnknown*			pIUnkInner = NULL;
	ISequentialStream*	pISS = NULL;

	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets);
	CHECK(hr = SetCmdProp(rgPropSets), S_OK);

	CAggregate Aggregate(m_pICommandText);

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(m_pICommandText->Execute(&Aggregate, IID_IUnknown, NULL, &lRA, (IUnknown**)&pIUnkInner), S_OK)
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation.
	TESTC(Aggregate.VerifyAggregationQI(hr, IID_ISequentialStream))

	TESTC(VerifyInterface(pIUnkInner, IID_ISequentialStream, 
		STREAM_INTERFACE,(IUnknown **)&pISS))

	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	if(rgPropSets)
	{
		VariantInit(&rgPropSets[0].rgProperties[0].vValue);
		CHECK(hr = SetCmdProp(rgPropSets), S_OK);
	}
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIUnkInner);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND: SetCmdStream with an embedded NULL terminator.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_12()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	WCHAR*		pwszSQL = NULL;
	ULONG		cWrote = 0;
	ULONG		ulLen = 0;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	//Write first command to stream.
	ulLen = (ULONG)wcslen(pwszSQL);
	hr = m_pStorageSet->Write(wszByteOrder, 2, NULL);
	hr = m_pStorageSet->Write(pwszSQL, (ulLen)*sizeof(WCHAR), &cWrote);

	//Seek to middle of command and put a NULL term char.
	m_pStorageSet->Seek(2 + (ulLen)*sizeof(WCHAR)/2);
	m_pStorageSet->Write(L"", 2, NULL);

	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, (ISequentialStream*)m_pStorageSet), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), DB_E_ERRORSINCOMMAND)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSINCOMMAND: Set unicode stream without BOM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_13()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set stream as ansi - no BOM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_14()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Set DBPROP_DEFERRED.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_15()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Call SetCmdStream so it fails. Ref cnt unchanged.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_16()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	ULONG		cRef=0;
	WCHAR*		pwszSQL = NULL;
	IRowset*	pIR = NULL;

	ALLOC_SETSTG;

	cRef = m_pStorageSet->GetRefCount();

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(m_pTable->ExecuteCommand(SELECT_ALLFROMTBL,
		IID_IRowset, NULL, NULL, NULL, NULL, 
		EXECUTE_IFNOERROR, 0, NULL, NULL, (IUnknown**)&pIR, 
		(ICommand**)&m_pICommandText), S_OK)
	TESTC(pIR != NULL)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), DB_E_OBJECTOPEN)
	COMPARE(m_pStorageSet->GetRefCount(), cRef);
	SAFE_RELEASE(pIR);

	TESTC_(hr = SetCmdStream(IID_IRowset, DBGUID_DEFAULT, pwszSQL), E_NOINTERFACE)
	COMPARE(m_pStorageSet->GetRefCount(), cRef);

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DSO, pwszSQL), DB_E_DIALECTNOTSUPPORTED)
	COMPARE(m_pStorageSet->GetRefCount(), cRef);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pIR);
	RELEASE_SETSTG;
	RELEASE_GETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Release all objects & verify no ref cnts left on stream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_17()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	ULONG				cRef = 0;
	WCHAR*				pwszSQL = NULL;
	IDBCreateSession*	pIDBCS = NULL;
	IDBCreateCommand*	pIDBCC = NULL;
	ICommandStream*		pICS = NULL;

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IDBCreateSession, 
		(IUnknown**)&pIDBCS, CREATEDSO_SETPROPERTIES|CREATEDSO_INITIALIZE), S_OK)

	TESTC_(pIDBCS->CreateSession(NULL, IID_IDBCreateCommand, 
		(IUnknown**)&pIDBCC), S_OK)

	TESTC_(pIDBCC->CreateCommand(NULL, IID_ICommandStream, (IUnknown**)&pICS), S_OK)

	ALLOC_SETSTG;
	cRef = m_pStorageSet->GetRefCount();

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	//Write command to stream.
	TESTC_(Write(pwszSQL), S_OK)

	TESTC_(hr = pICS->SetCommandStream(IID_ISequentialStream, DBGUID_DEFAULT, (ISequentialStream*)m_pStorageSet), S_OK)

	SAFE_RELEASE(pIDBCS);
	SAFE_RELEASE(pIDBCC);
	SAFE_RELEASE(pICS);

	COMPARE(m_pStorageSet->GetRefCount(), cRef);

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pIDBCS);
	SAFE_RELEASE(pIDBCC);
	SAFE_RELEASE(pICS);
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, GetCmdStream, SetCmdStream with stream got. Execute.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_18()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), S_OK)

	TESTC_(hr = m_pICommandStream->SetCommandStream(IID_ISequentialStream, 
		DBGUID_DEFAULT, (ISequentialStream*)m_pStorageGet), S_OK)

	//Execute command on second.
	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, Execute, GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_19()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	ICommand*	pICommand = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC(VerifyInterface(m_pICommandStream, IID_ICommand, 
		COMMAND_INTERFACE,(IUnknown **)&pICommand))

	TESTC_(pICommand->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)
	COMPARE(iid, IID_ISequentialStream);

	TESTC(m_pwszQ1 && pwszSQLGet)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, GetCmdStream, Execute
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_20()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQLGet = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, &pwszSQLGet), S_OK)

	TESTC(m_pwszQ1 && pwszSQLGet)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, Execute, Execute
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_21()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ISequentialStream*	pIS1 = NULL;
	ISequentialStream*	pIS2 = NULL;
	ISequentialStream*	pIS3 = NULL;

	ALLOC_SETSTG;

	TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, m_pwszQ2), S_OK, E_NOINTERFACE)
	TESTC_PROVIDER(hr == S_OK)

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pIS1), S_OK)
	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pIS1, IID_ISequentialStream))

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pIS2), S_OK)
	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pIS2, IID_ISequentialStream))

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pIS3), S_OK)
	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pIS3, IID_ISequentialStream))

CLEANUP:
	SAFE_RELEASE(pIS1);
	SAFE_RELEASE(pIS2);
	SAFE_RELEASE(pIS3);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdText, SetCmdStream, Execute
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_22()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	WCHAR*		pwszSQL = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc S_OK: SetCmdStream, GetCmdStream, SetCmdText, Execute. Read got stream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_23()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	IID			iid;
	GUID		guid = DBGUID_DEFAULT;
	WCHAR*		pwszSQL = NULL;
	WCHAR*		pwszSQLGet = NULL;
	IRowset*	pIR = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), S_OK)

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_IRowset, NULL, &lRA, (IUnknown**)&pIR), S_OK)

	TESTC(VerifyRowset(SELECT_ALLFROMTBL, pIR))

	//Read the got stream.
	TESTC_(hr = Read(&pwszSQLGet), S_OK)
	TESTC(m_pwszQ1 && pwszSQLGet)
	TESTC(wcscmp(m_pwszQ1, pwszSQLGet) == 0)

CLEANUP:
	SAFE_FREE(pwszSQL);
	SAFE_FREE(pwszSQLGet);
	SAFE_RELEASE(pIR);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc S_OK: Ask for IUnknown on Execute.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_24()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	WCHAR*		pwszSQL = NULL;
	IUnknown*	pIUnk = NULL;
	ISequentialStream*	pISS = NULL;

	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET, 
		&cPropSets, &rgPropSets);
	CHECK(hr = SetCmdProp(rgPropSets), S_OK);

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	//Execute asking for IID_IUnknown on the stream.
	TESTC_(m_pICommandText->Execute(NULL, IID_IUnknown, NULL, &lRA, (IUnknown**)&pIUnk), S_OK)

	TESTC(VerifyInterface(pIUnk, IID_ISequentialStream, 
		STREAM_INTERFACE,(IUnknown **)&pISS))

	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

CLEANUP:
	if(rgPropSets)
	{
		VariantInit(&rgPropSets[0].rgProperties[0].vValue);
		CHECK(hr = SetCmdProp(rgPropSets), S_OK);
	}
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszSQL);
	SAFE_RELEASE(pIUnk);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_OUTPUTSTREAM - ISequentialStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_25()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	m_pStorageGet = new CStorage();
	TESTC(m_pStorageGet != NULL)

	SetProperty(DBPROP_OUTPUTSTREAM, DBPROPSET_STREAM, &cPropSets, &rgPropSets, (void*)m_pStorageGet, DBTYPE_IUNKNOWN);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_E_ERRORSOCCURRED, TRUE);
	if(hr!=S_OK)
	{
		odtLog<<L"INFO: DBPROP_OUTPUTSTREAM is not supported.\n";
		goto CLEANUP;
	}

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	//Call Execute with ppRowset=NULL.
	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)

	TESTC(VerifyEqualInterface(pISS, (ISequentialStream*)m_pStorageGet))

	//Verify that query1 was executed.
	TESTC(VerifyQuery1((ISequentialStream*)m_pStorageGet, IID_ISequentialStream))

CLEANUP:
	if(rgPropSets)
	{
		V_VT(&rgPropSets[0].rgProperties[0].vValue) = VT_EMPTY;
		CHECKW(SetCmdProp(rgPropSets), S_OK);
	}
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_OUTPUTSTREAM - ISequentialStream (NULL ppRowset)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_26()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	m_pStorageGet = new CStorage();
	TESTC(m_pStorageGet != NULL)

	SetProperty(DBPROP_OUTPUTSTREAM, DBPROPSET_STREAM, &cPropSets, &rgPropSets, (void*)m_pStorageGet, DBTYPE_IUNKNOWN);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_E_ERRORSOCCURRED, TRUE);
	if(hr!=S_OK)
	{
		odtLog<<L"INFO: DBPROP_OUTPUTSTREAM is not supported.\n";
		goto CLEANUP;
	}

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	//Call Execute with ppRowset=NULL.
	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, NULL), S_OK)

	//Verify that query1 was executed.
	TESTC(VerifyQuery1((ISequentialStream*)m_pStorageGet, IID_ISequentialStream))

CLEANUP:
	if(rgPropSets)
	{
		V_VT(&rgPropSets[0].rgProperties[0].vValue) = VT_EMPTY;
		CHECKW(SetCmdProp(rgPropSets), S_OK);
	}
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_OUTPUTSTREAM - IStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_27()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ULONG		cPropSets = 0;
	DBPROPSET*	rgPropSets = NULL;
	IStream*	pIS = NULL;

	ALLOC_SETSTG;

	m_pStorageGet = new CStorage();
	TESTC(m_pStorageGet != NULL)

	SetProperty(DBPROP_OUTPUTSTREAM, DBPROPSET_STREAM, &cPropSets, &rgPropSets, (void*)m_pStorageGet, DBTYPE_IUNKNOWN);
	hr = SetCmdProp(rgPropSets);
	COMPARE(hr==S_OK || hr==DB_E_ERRORSOCCURRED, TRUE);
	if(hr!=S_OK)
	{
		odtLog<<L"INFO: DBPROP_OUTPUTSTREAM is not supported.\n";
		goto CLEANUP;
	}

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	//Call Execute with valid ppRowset.
	TESTC_(m_pICommandText->Execute(NULL, IID_IStream, NULL, &lRA, (IUnknown**)&pIS), S_OK)
	TESTC(pIS != NULL)

	TESTC(VerifyEqualInterface(pIS, (IStream*)m_pStorageGet))

	//Verify that query1 was executed.
	TESTC(VerifyQuery1((IStream*)m_pStorageGet, IID_IStream))

CLEANUP:
	if(rgPropSets)
	{
		V_VT(&rgPropSets[0].rgProperties[0].vValue) = VT_EMPTY;
		CHECKW(SetCmdProp(rgPropSets), S_OK);
	}
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_OUTPUTENCODING - UTF-8
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_28()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	ULONG		cRead = 0;
	ULONG		ulReadIncr = READ_INCR;
	WCHAR*		pwszCmd = NULL;
	CHAR*		pszCmd = NULL;
	CHAR*		pszEnd = NULL;
	BOOL		fEnd = FALSE;
	DBROWCOUNT	lRA = 0;
	ISequentialStream*	pISS = NULL;

	TESTC_(SetEncdProp(UTF8), S_OK)

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	SAFE_RELEASE(pISS);
	RELEASE_SETSTG;
	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ2), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)

	//Verify that query2 was executed.

	SAFE_ALLOC(pszCmd, CHAR, ulReadIncr);

	hr = pISS->Read(pszCmd, ulReadIncr, &cRead);
	if(cRead < ulReadIncr)
	{
		fEnd = TRUE;
		pszEnd = (CHAR*)pszCmd + cRead;
	}

	while(!fEnd)
	{
		cRead = 0;
		ulReadIncr += READ_INCR;
		SAFE_REALLOC(pszCmd, CHAR, ulReadIncr);
		hr = pISS->Read((BYTE*)pszCmd +ulReadIncr -READ_INCR, READ_INCR, &cRead);
		if(cRead < READ_INCR)
		{
			fEnd = TRUE;
			pszEnd = (CHAR*)pszCmd +ulReadIncr -READ_INCR + cRead;
		}
	}

	memset((BYTE*)pszEnd, 0, 1);

	pwszCmd = ConvertToWCHAR(pszCmd);
	TESTC(pwszCmd != NULL)

	TESTC(wcsstr(pwszCmd, m_pwszQ2) != NULL)

CLEANUP:
	SAFE_FREE(pwszCmd);
	SAFE_FREE(pszCmd);
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_OUTPUTENCODING - UTF-16
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_29()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ISequentialStream*	pISS = NULL;

	TESTC_(SetEncdProp(UTF16), S_OK)

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	SAFE_RELEASE(pISS);
	RELEASE_SETSTG;
	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ2), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pISS, IID_ISequentialStream))

CLEANUP:
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: iid passed in does not match object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_30()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	WCHAR*		pwszSQL = NULL;

	//Make a str4eam object that can only be QIed for ILockBytes.
	ALLOC_SETSTG;
	m_pStorageSet->SetQISeqStream(FALSE);
	m_pStorageSet->SetQIStream(FALSE);
	m_pStorageSet->SetQILockBytes(TRUE);

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), E_NOINTERFACE)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc E_NOINTERFACE: iid is IRowset & IRow.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_31()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_IRow, DBGUID_DEFAULT, pwszSQL), E_NOINTERFACE)
	TESTC_(hr = SetCmdStream(IID_IRowset, DBGUID_DEFAULT, pwszSQL), E_NOINTERFACE)

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc DB_E_CANTTRANSLATE: SetCmdStream, GetCmdText
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_32()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	GUID		guid = DBGUID_DEFAULT;
	GUID		guidNull = DB_NULLGUID;
	WCHAR*		pwszCmd = NULL;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = m_pICommandText->GetCommandText(&guid, &pwszCmd), DB_E_CANTTRANSLATE)
	COMPARE(guid, guidNull);
	COMPARE(pwszCmd, NULL);

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DIALECTNOTSUPPORTED: Use some bugs guids & DBGUID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_33()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	GUID		guidNull = DB_NULLGUID;

	ALLOC_SETSTG;

	CHECK(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DSO, m_pwszQ1), DB_E_DIALECTNOTSUPPORTED);
	CHECK(hr = SetCmdStream(IID_ISequentialStream, DBGUID_ROWSET, m_pwszQ1), DB_E_DIALECTNOTSUPPORTED);
	CHECK(hr = SetCmdStream(IID_ISequentialStream, guidNull, m_pwszQ1), DB_E_DIALECTNOTSUPPORTED);

CLEANUP:
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc DB_E_DIALECTNOTSUPPORTED (or S_OK): use DBGUID_SQL.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_34()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;

	ALLOC_SETSTG;

	TEST2C_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_SQL, m_pwszQ1), S_OK, DB_E_DIALECTNOTSUPPORTED);

	if(hr == S_OK)
		odtLog<<L"INFO: DBGUID_SQL is supported on SetCommandStream.\n";
	else
		odtLog<<L"INFO: DBGUID_SQL is NOT supported on SetCommandStream.\n";

CLEANUP:
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc DB_E_OBJECTOPEN: Keeping a result stream open, call Set.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_35()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	DBROWCOUNT	lRA = 0;
	ISequentialStream*	pISS = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, m_pwszQ1), S_OK)

	TESTC_(m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pISS), S_OK)
	//Verify that query1 was executed.
	TESTC(VerifyQuery1(pISS, IID_ISequentialStream))

	TESTC_(hr = SetCmdStream(IID_ISequentialStream, DBGUID_DEFAULT, NULL), DB_E_OBJECTOPEN)

CLEANUP:
	SAFE_RELEASE(pISS);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Clear previous error object.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_36()
{ 
	TBEGIN
	HRESULT				hr = E_FAIL;
	DBROWCOUNT			lRA = 0;
	ULONG				cRecords = 0;
	IColumnsInfo*		pIC = NULL;
	ISequentialStream*	pIS1 = NULL;
	IErrorInfo*			pIEI = NULL;
	IErrorRecords*		pIER = NULL;

	ALLOC_SETSTG;

	TEST2C_(hr = SetCmdStream(IID_IStream, DBGUID_DEFAULT, m_pwszQ2), S_OK, E_NOINTERFACE)
	TESTC_PROVIDER(hr == S_OK)

	TESTC_(hr = m_pICommandText->Execute(NULL, IID_ISequentialStream, NULL, &lRA, (IUnknown**)&pIS1), S_OK)

	//Cause an error.
	TESTC(VerifyInterface(m_pICommandText, IID_IColumnsInfo, 
		COMMAND_INTERFACE,(IUnknown **)&pIC))
	TESTC_(pIC->GetColumnInfo(NULL, NULL, NULL), E_INVALIDARG)

	//Verify that query2 was executed.
	TESTC(VerifyQuery2(pIS1, IID_ISequentialStream))

	//When stream was read above it returned S_OK. So, the error info
	//should have been cleared.
	//Get the IErrorInfo interface.
	hr = GetErrorInfo(0, &pIEI) ;
	if(hr == S_FALSE)
	{
		odtLog<<L"ErrorInfo is not supported.\n";
		goto CLEANUP;
	}

CLEANUP:
	SAFE_RELEASE(pIC);
	SAFE_RELEASE(pIS1);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Extended error test - GetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_37()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	IID			iid;
	GUID		guid;
	WCHAR*		pwszCmd = NULL;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)
	
	TESTC_(hr = m_pICommandText->SetCommandText(DBGUID_DEFAULT, pwszSQL), S_OK)

	TESTC_(hr = GetCmdStream(&iid, &guid, NULL), DB_E_CANTTRANSLATE)

	TESTC(XCHECK(m_pICommandStream, IID_ICommandStream, hr));

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_GETSTG;
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Extended error test - SetCmdStream
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSetCmdStream::Variation_38()
{ 
	TBEGIN
	HRESULT		hr = E_FAIL;
	WCHAR*		pwszSQL = NULL;

	ALLOC_SETSTG;

	TESTC_(hr = m_pTable->CreateSQLStmt(SELECT_ALLFROMTBL, m_pTable->GetTableName(), &pwszSQL), S_OK)

	TESTC_(hr = SetCmdStream(IID_IRowset, DBGUID_DEFAULT, pwszSQL), E_NOINTERFACE)

	TESTC(XCHECK(m_pICommandStream, IID_ICommandStream, hr));

CLEANUP:
	SAFE_FREE(pwszSQL);
	RELEASE_SETSTG;
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCSetCmdStream::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CCmdStream::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END
