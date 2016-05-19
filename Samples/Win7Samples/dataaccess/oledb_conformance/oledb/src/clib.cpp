// clib.cpp

#include "modstandard.hpp"
#include "privlib.h"
#include "oledberr.h"
#include "clib.hpp"

CSchemaInfo	SchemaInfo;

extern const wchar_t *gwszModuleName;


BOOL CCheckConsRowset::s_fMetadataInitialized				= FALSE;
BOOL CCheckConsRowset::s_fSchemaSupported					= FALSE;
BOOL CCheckConsRowset::s_fConsCatalogR						= FALSE;
BOOL CCheckConsRowset::s_fConsSchemaR						= FALSE;
BOOL CCheckConsRowset::s_fConsNameR							= FALSE;

BOOL CPKRowset::s_fMetadataInitialized						= FALSE;
BOOL CPKRowset::s_fSchemaSupported							= FALSE;
BOOL CPKRowset::s_fTableCatalogR							= FALSE;
BOOL CPKRowset::s_fTableSchemaR								= FALSE;
BOOL CPKRowset::s_fTableNameR								= FALSE;

BOOL CFKRowset::s_fMetadataInitialized						= FALSE;
BOOL CFKRowset::s_fSchemaSupported							= FALSE;
BOOL CFKRowset::s_fTableCatalogR							= FALSE;
BOOL CFKRowset::s_fTableSchemaR								= FALSE;
BOOL CFKRowset::s_fTableNameR								= FALSE;

BOOL CKeyColumnUsageRowset::s_fMetadataInitialized			= FALSE;
BOOL CKeyColumnUsageRowset::s_fSchemaSupported				= FALSE;
BOOL CKeyColumnUsageRowset::s_fTableCatalogR				= FALSE;
BOOL CKeyColumnUsageRowset::s_fTableSchemaR					= FALSE;
BOOL CKeyColumnUsageRowset::s_fTableNameR					= FALSE;
BOOL CKeyColumnUsageRowset::s_fConsCatalogR					= FALSE;
BOOL CKeyColumnUsageRowset::s_fConsSchemaR					= FALSE;
BOOL CKeyColumnUsageRowset::s_fConsNameR					= FALSE;


//helper functions
BOOL CheckRestriction(
	BOOL		fSupportedRestriction,
	WCHAR		*pwszRestriction,
	WCHAR		*pwszReturnValue
)
{
	//returns TRUE if current value conforms to the restriction
	BOOL	fRes = FALSE;

	if (fSupportedRestriction)
	{
		// they should be the same
		TESTC(		(NULL == pwszRestriction)
				||		(NULL != pwszReturnValue)
					&&	(0 == wcscmp(pwszRestriction, pwszReturnValue)));
		fRes = TRUE;
	}
	else
		fRes = !pwszRestriction || pwszRestriction && (0 == wcscmp(pwszRestriction, pwszReturnValue));

CLEANUP:
	return fRes;
} //CheckRestriction




//------------------------------------------------------------------------
//			CLASS CSchemaInfo
//
//------------------------------------------------------------------------
CSchemaInfo::CSchemaInfo()
{
#define SET_RESTR(SchemaID, cRestr)			\
	m_rgSchemaEntry[m_cSchemas].guidSchema = SchemaID;				\
	m_rgSchemaEntry[m_cSchemas].cRestrictions = cRestr;				\
	m_rgSchemaEntry[m_cSchemas].rgRestrictions = rg##SchemaID;		\
	m_cSchemas++;

	ULONG	index = 0;
	ULONG	rgDBSCHEMA_ASSERTIONS[]					= {1, 2, 3};
	ULONG	rgDBSCHEMA_CATALOGS[]					= {1};
	ULONG	rgDBSCHEMA_CHECK_CONSTRAINTS[]			= {1, 2, 3};
	ULONG	rgDBSCHEMA_CONSTRAINT_COLUMN_USAGE[]	= {1, 2, 3, 4, 7, 8, 9};
	ULONG	rgDBSCHEMA_CONSTRAINT_TABLE_USAGE[]		= {1, 2, 3, 4, 5, 6};
	ULONG	rgDBSCHEMA_FOREIGN_KEYS[]				= {1, 2, 3, 7, 8, 9};
	ULONG	rgDBSCHEMA_KEY_COLUMN_USAGE[]			= {1, 2, 3, 4, 5, 6, 7};
	ULONG	rgDBSCHEMA_PRIMARY_KEYS[]				= {1, 2, 3};
	ULONG	rgDBSCHEMA_REFERENTIAL_CONSTRAINTS[]	= {1, 2, 3};
	ULONG	rgDBSCHEMA_TABLE_CONSTRAINTS[]			= {1, 2, 3, 4, 5, 6, 7};
	
	// allocate the arrays
	m_cSchemas = 31;	// maximum value

	SAFE_ALLOC(m_rgSchemaEntry, SSchemaEntry, m_cSchemas);

	memset(m_rgSchemaEntry, 0, m_cSchemas*sizeof(SSchemaEntry));

	m_cSchemas = 0;

	SET_RESTR(DBSCHEMA_ASSERTIONS, 3);
	SET_RESTR(DBSCHEMA_CATALOGS, 1);

	SET_RESTR(DBSCHEMA_CHECK_CONSTRAINTS, 3);
	SET_RESTR(DBSCHEMA_CONSTRAINT_COLUMN_USAGE, 7);
	SET_RESTR(DBSCHEMA_CONSTRAINT_TABLE_USAGE, 6);
	SET_RESTR(DBSCHEMA_FOREIGN_KEYS, 6);
	SET_RESTR(DBSCHEMA_KEY_COLUMN_USAGE, 7);
	SET_RESTR(DBSCHEMA_PRIMARY_KEYS, 3);
	SET_RESTR(DBSCHEMA_REFERENTIAL_CONSTRAINTS, 3);
	SET_RESTR(DBSCHEMA_TABLE_CONSTRAINTS, 7);

CLEANUP:
	return;
} //CSchemaInfo::CSchemaInfo




LONG CSchemaInfo::GetSchemaIndex(GUID guidSchema)
{
	ULONG index;

	for (index=0; index < m_cSchemas; index++)
	{
		if (m_rgSchemaEntry[index].guidSchema == guidSchema)
			return (LONG)index;
	}

	return -1;
} //CSchemaInfo::GetSchemaIndex




SSchemaEntry *CSchemaInfo::operator[](GUID guidSchema)
{
	LONG	index = GetSchemaIndex(guidSchema);

	return (-1 == index)? NULL: &m_rgSchemaEntry[index];
} //CSchemaInfo::GetSchemaIndex






//------------------------------------------------------------------------
//			CLASS CSchemaRowset
//
//------------------------------------------------------------------------
CSchemaRowset::CSchemaRowset(
	IUnknown	*pIUnknown, 
	BOOL		fQI	/*=TRUE*/
)
{
	m_pIDBSR = NULL;
	
	ASSERT(pIUnknown);
	if (fQI)
	{
		BOOL result = VerifyInterface(pIUnknown, IID_IDBSchemaRowset,
			SESSION_INTERFACE, (IUnknown**)&m_pIDBSR);
		ASSERT(result);
	}
	else
	{	
		// use the interface without QI
		// the caller is responsible to pass
		// apropriate interface in this case
		pIUnknown->AddRef();
		m_pIDBSR = (IDBSchemaRowset*)pIUnknown;
	}

	m_cSchemas				= 0;
	m_rgSchemas			= NULL;
	m_rgRestrictionSupport	= NULL;
} //CSchemaRowset::CSchemaRowset


CSchemaRowset::~CSchemaRowset()
{
	SAFE_RELEASE(m_pIDBSR);
	FreeCache();
} //CSchemaRowset::~CSchemaRowset




//-----------------------------------------------------------
//
//	CSchemaRowset::GetSchemaIndex
//
// This method returns the index of the schema in the schema array
//-----------------------------------------------------------
LONG CSchemaRowset::GetSchemaIndex(GUID guidSchema)
{
	ULONG index;

	for (index=0; index < m_cSchemas; index++)
	{
		if (m_rgSchemas[index] == guidSchema)
			return (LONG)index;
	}

	return -1;
} //CSchemaRowset::GetSchemaIndex




//-----------------------------------------------------------
//
//	CSchemaRowset::IsSchemaSupported
//
// This method tells whether a certain schema is supported or not
//-----------------------------------------------------------
HRESULT CSchemaRowset::IsSchemaSupported(GUID guidSchema)
{
	HRESULT	hr = S_OK;
	LONG	index;

	if (0 == m_cSchemas)
		TESTC_(hr = GetSchemas(), S_OK);

	index = GetSchemaIndex(guidSchema);
	hr = (-1 != index)? S_OK: E_FAIL;

CLEANUP:
	return hr;
} //CSchemaRowset::IsSchemaSupported




//-----------------------------------------------------------
//
//	CSchemaRowset::GetRestrictionSupport
//
// Restriction support for a certain schema
//-----------------------------------------------------------
HRESULT CSchemaRowset::GetRestrictionSupport(GUID guidSchema, ULONG *pRestSupp)
{
	HRESULT	hr = E_INVALIDARG;
	LONG	index;

	TESTC(NULL != pRestSupp);
	*pRestSupp = 0;

	if (0 == m_cSchemas)
		TESTC_(GetSchemas(), S_OK);

	index = GetSchemaIndex(guidSchema);
	hr = (-1 != index)? S_OK: DB_E_NOTFOUND;
	if (-1 < index)
		*pRestSupp = m_rgRestrictionSupport[index];

CLEANUP:
	return hr;
} //CSchemaRowset::GetRestrictionSupport

//-----------------------------------------------------------
//
//	CSchemaRowset::GetSchemas
//
// This method calls IDBSchemaRowset::GetSchemas and
// caches the results
//-----------------------------------------------------------
HRESULT CSchemaRowset::GetSchemas()
{
	HRESULT		hr = E_FAIL;
	ULONG		cSchemas;
	GUID		*rgSchemas;
	ULONG		*rgRestrictionSupport;
	ULONG		index1;
	LONG		index2;
	
	if (!m_pIDBSR)
		return E_FAIL;

	hr = m_pIDBSR->GetSchemas(&cSchemas, &rgSchemas, &rgRestrictionSupport);

	// check the validity of the returned value
	TEST3C_(hr, S_OK, E_FAIL, E_OUTOFMEMORY);

	// NOTE: if tester wants to check the conditions when E_INVALIDARG are returned, 
	// he should use the cast operator and execute the call to GetSchemas direct

	if (S_OK == hr)
	{
		// check the old cache if it exist
		if (0 < m_cSchemas)
		{
			COMPARE(cSchemas == m_cSchemas, TRUE);
			COMPARE(2 < cSchemas, TRUE);	// at least 3 schemas should be supported:TABLES, COLUMNS, and PROVIDER_TYPES schema rowsets

			// check the tables
			for (index1 = 0; index1 < cSchemas; index1++)
			{
				// find the schema in the other array - optimize for the case when they are identical 
				index2 = index1;
				if (rgSchemas[index1] != m_rgSchemas[index1])
				{
					index2 = GetSchemaIndex(rgSchemas[index1]);
				}

				if (0 > index2)
				{
					hr = E_FAIL;
					// signal there is was an error
					CHECK(hr, S_OK);
				}
				else
				{
					// check restriction support, too
					COMPARE(rgRestrictionSupport[index1] == m_rgRestrictionSupport[index2], TRUE);
				}
			}
			
			// release values, since we have the old cache
			SAFE_FREE(rgSchemas);
			SAFE_FREE(rgRestrictionSupport);
		}
		else
		{
			// save values to the cache
			m_cSchemas				= cSchemas;
			m_rgSchemas				= rgSchemas;
			m_rgRestrictionSupport	= rgRestrictionSupport;
		}
	}
	else
	{
		// this is for E_FAIL and E_OUTOFMEMORY
		TESTC(0		== cSchemas);
		TESTC(NULL	== rgSchemas);
		TESTC(NULL	== rgRestrictionSupport);
	}

CLEANUP:
	return hr;
} //CSchemaRowset::GetSchemas




// @ cmember Calls GetRowset on the IDBSchemaRowset interface and
// does some basic checking
HRESULT CSchemaRowset::GetRowset(
   IUnknown *      pUnkOuter,
   REFGUID         rguidSchema,
   ULONG           cRestrictions,
   const VARIANT   rgRestrictions[],
   REFIID          riid,
   ULONG           cPropertySets,
   DBPROPSET       rgPropertySets[],
   IUnknown **     ppRowset							
)
{
	HRESULT		hr;

	// call the method
	hr = m_pIDBSR->GetRowset(pUnkOuter, rguidSchema, cRestrictions, rgRestrictions,
            riid, cPropertySets, rgPropertySets, ppRowset);

	// check the parameters and try to guess the possible returned value
	switch (hr)
	{
		case DB_S_ASYNCHRONOUS:
			break;

		case DB_S_ERRORSOCCURRED:
			// check that some property was not set and that some other property was set

		case S_OK:
			// on success, check that the columns correspondings to restrictions do
			// actually contain the restricted values
			break;

		case DB_E_ERRORSOCCURRED:
			// make sure all of the properties failed
			break;

		case E_INVALIDARG:
			// Check the possible reasons for returning E_INVALIDARG:
			//		rguidSchema was invalid.
			//		rguidSchema specified a schema rowset that was not supported by 
			// the provider.
			//		cRestrictions was greater than the number of restriction columns 
			// for the schema rowset specified in rguidSchema.
			//		In one or more restriction values specified in rgRestrictions, 
			// the vt element of the VARIANT was the incorrect type.
			//		cRestrictions was greater than zero, and rgRestrictions was a null 
			// pointer.
			//		In an element of rgRestrictions, the vt element of the VARIANT was 
			// not VT_EMPTY and the provider did not support the corresponding restriction.
			//		ppRowset was a null pointer.
			//		In an element of rgPropertySets, cProperties was not 0 and 
			// rgProperties was a null pointer.
			//		cPropertySets was greater than zero, and rgPropertySets was a null pointer.
			break;

		case DB_E_NOTSUPPORTED:
			// some of the asked restrictions are not supported
			break;

		case E_NOINTERFACE:
		case E_OUTOFMEMORY:
		case E_FAIL:
		case DB_E_ABORTLIMITREACHED:
		case DB_E_NOTFOUND:
		case DB_E_OBJECTOPEN:
		case DB_SEC_E_PERMISSIONDENIED:
			break;

		case DB_E_NOAGGREGATION:
			TESTC(pUnkOuter && riid != IID_IUnknown);
			break;

		default:
			TESTC(FALSE);
	}


CLEANUP:
	return hr;
} //CSchemaRowset::GetRowset




//------------------------------------------------------------------------
//			CLASS CCatalogs
//
//------------------------------------------------------------------------
CCatalogs::CCatalogs(IUnknown *pSessionIUnknown)
{
	IRowset			*pIRowset	= NULL;
	CTable			*pTable		= NULL;

	IGetDataSource	*pIGetDataSource = NULL;

	m_pSessionIUnknown	= NULL;
	m_pIDBProperties		= NULL;

	// set data source interface
	TESTC(NULL != pSessionIUnknown);
	m_pSessionIUnknown = pSessionIUnknown;
	m_pSessionIUnknown->AddRef();

	// get data source interface
	TESTC(VerifyInterface(m_pSessionIUnknown, IID_IGetDataSource, SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));

	if (CHECK(pIGetDataSource->GetDataSource(IID_IDBProperties, (IUnknown**)&m_pIDBProperties), S_OK))
	{
		CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
		CComfRowset		Rowset;
		DBORDINAL		index;
		HROW			hRow;
		const DBORDINAL	ulCatalogName = 0;
		WCHAR			*pwszCatalogName;

		pTable = new CTable(m_pSessionIUnknown, L"NoModuleName");
		m_cCatalogs		= 0;
		m_rgCatalogs	= NULL;

		// get the rowset interface
		if (S_OK != SchemaRowset.IsSchemaSupported(DBSCHEMA_CATALOGS))
		{
			odtLog << "Provider does not support DBSCHEMA_CATALOGS\n";
			goto CLEANUP;
		}

		TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_CATALOGS, 0, NULL, IID_IRowset,
			0, NULL, (IUnknown**)&pIRowset), S_OK);

		Rowset.DropRowset();
		Rowset.SetTable(pTable, DELETETABLE_NO);

		TESTC_PROVIDER(S_OK == Rowset.CreateRowset(pIRowset, USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK));

		//Try to find the specified row with this table...
		m_cCatalogs = Rowset.GetTotalRows();
		SAFE_ALLOC(m_rgCatalogs, WCHAR*, m_cCatalogs);
		memset(m_rgCatalogs, 0, (size_t)m_cCatalogs*sizeof(WCHAR*));

		for (index = 0; index < m_cCatalogs; index++)
		{
			// get the next catalog
			TESTC_(Rowset.GetNextRows(&hRow), S_OK);
			TESTC_(Rowset.GetRowData(hRow, &Rowset.m_pData),S_OK);

			// get the name of the constraint
			pwszCatalogName = (WCHAR*)Rowset.GetValue(ulCatalogName);
			TESTC(NULL != pwszCatalogName);
			m_rgCatalogs[index] = wcsDuplicate(pwszCatalogName);
			Rowset.ReleaseRows(hRow);	
		}
		TESTC_(Rowset.GetNextRows(&hRow), DB_S_ENDOFROWSET);
	}

CLEANUP:
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGetDataSource);
	delete pTable;
} // CCatalogs::CCatalogs




CCatalogs::~CCatalogs()
{
	SAFE_RELEASE(m_pSessionIUnknown);
	SAFE_RELEASE(m_pIDBProperties);
	for (DBORDINAL index = 0; index < m_cCatalogs; index++)
	{
		SAFE_FREE(m_rgCatalogs[index]);
	}
	SAFE_FREE(m_rgCatalogs);
	m_rgCatalogs	= NULL;
	m_cCatalogs		= 0;
} //CCatalogs::~CCatalogs



WCHAR *CCatalogs::operator[](DBORDINAL index)
{
	if (!m_rgCatalogs || m_cCatalogs <= index)
		return NULL;
	else
		return m_rgCatalogs[index];
} //CCatalogs::operator[]




//---------------------------------------------------------------
//
//		CCatalogs::GetCurrentCatalog
//
// Gets the current catalog;
//---------------------------------------------------------------
BOOL CCatalogs::GetCurrentCatalog(WCHAR **ppwszCatalogName)
{
	return GetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, m_pIDBProperties,
		ppwszCatalogName);
} //CCatalogs::GetCurrentCatalog




//---------------------------------------------------------------
//
//		CCatalogs::SetCurrentCatalog
//
// Sets the current catalog
//---------------------------------------------------------------
HRESULT CCatalogs::SetCurrentCatalog(WCHAR *pwszCatalogName)
{
	HRESULT			hr					= E_FAIL;
	ULONG			cPropSets			= 0;
	DBPROPSET		*rgPropSets			= NULL;

	SAFE_ALLOC(rgPropSets, DBPROPSET, 1);
	rgPropSets[0].cProperties = 0;
	rgPropSets[0].rgProperties = NULL;
	rgPropSets[0].guidPropertySet = DBPROPSET_DATASOURCE;

	TESTC_(::SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, VT_BSTR, (LPVOID)pwszCatalogName), S_OK);
	hr = m_pIDBProperties->SetProperties(cPropSets, rgPropSets);

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	return hr;
} // CCatalogs::SetCurrentCatalog




//---------------------------------------------------------------
//
//		CCatalogs::ChangeCurrentCatalog
//
// Changes the current catalog
//---------------------------------------------------------------
HRESULT CCatalogs::ChangeCurrentCatalog(WCHAR **ppwszCatalogName/*=NULL*/)
{
	HRESULT			hr					= E_FAIL;
	ULONG			cPropSets			= 0;
	DBPROPSET		*rgPropSets			= NULL;
	WCHAR			*pwszCurrentCatalog	= NULL;
	BOOL			fSettableCurrentCatalog = SettableProperty(DBPROP_CURRENTCATALOG, 
													DBPROPSET_DATASOURCE, m_pIDBProperties);
	DBORDINAL		indxCat = 0;

	if (!fSettableCurrentCatalog)
		goto CLEANUP;

	// use the array of catalogs to pick one catalog
	if (1 >= m_cCatalogs)
		goto CLEANUP;

	TESTC(GetCurrentCatalog(&pwszCurrentCatalog));
	SAFE_ALLOC(rgPropSets, DBPROPSET, 1);
	rgPropSets[0].cProperties = 0;
	rgPropSets[0].rgProperties = NULL;
	rgPropSets[0].guidPropertySet = DBPROPSET_DATASOURCE;

	indxCat = rand() % m_cCatalogs;
	if (0 == wcscmp(pwszCurrentCatalog, m_rgCatalogs[indxCat]))
		indxCat = (indxCat + 1) % m_cCatalogs;

	TESTC_(::SetProperty(DBPROP_CURRENTCATALOG, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, VT_BSTR, (LPVOID)m_rgCatalogs[indxCat]), S_OK);
	hr = m_pIDBProperties->SetProperties(cPropSets, rgPropSets);

	TESTC_(hr, S_OK);

	// return the new catalog name if asked
	if (ppwszCatalogName)
	{
		SAFE_ALLOC(*ppwszCatalogName, WCHAR, 1+wcslen(m_rgCatalogs[indxCat]));
		wcscpy(*ppwszCatalogName, m_rgCatalogs[indxCat]);
	}

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszCurrentCatalog);
	return hr;
} //CCatalogs::ChangeCurrentCatalog




//------------------------------------------------------------------------
//			CLASS CCatalogs
//
//------------------------------------------------------------------------
CAccessibleCatalogs::CAccessibleCatalogs(IUnknown *pSessionIUnknown) :
	CCatalogs(pSessionIUnknown)
{
	// remove all the catalogs that are not accessible for the current user
	// user will be able to switch to any of the remaining catalogs
	HRESULT		hr;
	DBORDINAL	index;
	WCHAR		*pwszCrtCatalog;

	TESTC(GetCurrentCatalog(&pwszCrtCatalog));

	for (index = 0; index < m_cCatalogs;)
	{
		// check whether the user can switch the current dialog to this one
		hr = SetCurrentCatalog(m_rgCatalogs[index]);
		if (pwszCrtCatalog == m_rgCatalogs[index])
		{
			CHECK(hr, S_OK);
			index++;
		}
		else if (S_OK != hr)
		{
			// release the element and copy the last one here
			// do not increase counter
			CHECK(hr, DB_E_ERRORSOCCURRED);
			SAFE_FREE(m_rgCatalogs[index]);
			m_rgCatalogs[index] = m_rgCatalogs[--m_cCatalogs];
		}
		else
		{
			// change back the current catalog
			CHECK(SetCurrentCatalog(pwszCrtCatalog), S_OK);
			index++;
		}
	}

CLEANUP:
	if (pwszCrtCatalog)
		CHECK(SetCurrentCatalog(pwszCrtCatalog), S_OK);
	return;
} //CAccessibleCatalogs::CAccessibleCatalogs





//------------------------------------------------------------------------
//			CLASS CConstraints
//
//------------------------------------------------------------------------


//------------------------------------------------------------------------
//			CConstraints:CConstraints
//	Constructor
//
//------------------------------------------------------------------------
CConstraints::CConstraints(IUnknown *pSessionIUnknown)
{ 
	IGetDataSource	*pIGetDataSource = NULL;

	m_cConstraints		= 0; 
	m_rgConstraints		= NULL;	
	m_pwszTableCatalog	= NULL; 
	m_pwszTableSchema	= NULL;
	m_pwszTableName		= NULL; 
	m_pwszConsCatalog	= NULL;
	m_pwszConsSchema	= NULL; 
	m_pwszConsName		= NULL;
	m_pSessionIUnknown	= NULL;
	m_pDSOIUnknown		= NULL;
	m_cMaxConsNames		= 10;
	m_cConsNames		= 0;
	m_rgConsNames		= NULL;
	
	SAFE_ALLOC(m_rgConsNames, WCHAR*, m_cMaxConsNames);
	memset(m_rgConsNames, 0, (size_t)m_cMaxConsNames * sizeof(WCHAR*));

	TESTC(NULL != pSessionIUnknown);
	pSessionIUnknown->AddRef();
	m_pSessionIUnknown = pSessionIUnknown;

	TESTC(VerifyInterface(m_pSessionIUnknown, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown, &m_pDSOIUnknown), S_OK);

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	return;
} //CConstraints::CConstraints




CConstraints::~CConstraints() 
{
	DBORDINAL	index;

	for (index = 0; index < m_cConsNames; index++)
	{
		SAFE_FREE(m_rgConsNames[index]);
	}
	SAFE_FREE(m_rgConsNames);

	SAFE_RELEASE(m_pSessionIUnknown);
	SAFE_RELEASE(m_pDSOIUnknown);
	FreeConstraintDesc(&m_cConstraints, &m_rgConstraints);
	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);
	SAFE_FREE(m_pwszConsCatalog);
	SAFE_FREE(m_pwszConsSchema);
	SAFE_FREE(m_pwszConsName);
} // CConstraints::~CConstraints


//------------------------------------------------------------------------
//			Cconstraint::ParseQualTableName()
//
//------------------------------------------------------------------------
HRESULT	CConstraints::ParseQualTableName(WCHAR *pwszQualTableName, 
							WCHAR **pwszTableCatalog,
							WCHAR **pwszTableSchema,
							WCHAR **pwszTableName) {

	// Qualified Table names can be in any of these forms
	//		Catalog.Schema.TableName
	//		Schema.TableName
	//		TableName
			
	//odtLog << pwszQualTableName << "\n";
	// Step 1: find number of '.' in Qualified Table Name
	WCHAR	*p				= pwszQualTableName,
			*q				= NULL;
	bool	bInsideQuote	= false;
	DWORD	dwCountDots		= 0;
	WCHAR	*pwszList[3]	= { NULL, NULL, NULL };

	while(*p) {
		if (*p == L'"') 
			bInsideQuote = !bInsideQuote;
		else if (*p == L'.' && !bInsideQuote)
			dwCountDots++;
		p++;
	}
	if (dwCountDots > 2)
		return E_FAIL;

	// Now get each parameter
	bInsideQuote		= false;
	p					= pwszQualTableName;
	q					= p;
	WCHAR				ch, *r;

	for(DWORD i=0;i<=dwCountDots;i++) {
		q = p;
		if (*q == L'"') {
			q++;
			bInsideQuote = true;
		}

		p=q;
		while( !(*p==L'.' && bInsideQuote==false) && *p) {
			if (*p == L'"')
				bInsideQuote = !bInsideQuote;
			p++;
		}

		r = p;
		if (*p == L'.') p--;
		if (*(p-1) == L'"') p--;

		if (*p !=L'.' && *p!='"') p++;
		ch = *p;
		*p = 0;
		pwszList[i] = wcsDuplicate(q);
		*p = ch;

		if (*p == L'"') p++;
		if (*p == L'.') p++;
	}

	SAFE_FREE(*pwszTableCatalog);
	SAFE_FREE(*pwszTableSchema);
	SAFE_FREE(*pwszTableName);

	switch(dwCountDots) {
	case 0:
		*pwszTableCatalog = NULL;
		*pwszTableSchema = NULL;
		*pwszTableName = pwszList[0];
		//odtLog << "Parse: Table Name = " << *pwszTableName << "\n";
		break;

	case 1:
		*pwszTableCatalog = NULL;
		*pwszTableSchema = pwszList[0];
		*pwszTableName = pwszList[1];
		//odtLog << "Parse: Schema Name = " << *pwszTableSchema << ", Table Name = " << *pwszTableName << "\n";
		break;

	case 2:
		*pwszTableCatalog = pwszList[0];
		*pwszTableSchema = pwszList[1];
		*pwszTableName = pwszList[2];
		//odtLog << "Parse: Catalog = " << *pwszTableCatalog << ", Schema Name = " << *pwszTableSchema << ", Table Name = " << *pwszTableName << "\n";
		break;
	}

	return S_OK;
}


//------------------------------------------------------------------------
//			CConstraints:GetConstraints
//	Get the array on constraints according to the specified criteria
//
//------------------------------------------------------------------------
HRESULT CConstraints::GetConstraints(
	WCHAR	*pwszTableCatalogR,	// [in] table catalog restriction
	WCHAR	*pwszTableSchemaR,	// [in] table schema restriction
	WCHAR	*pwszTableNameR,		// [in] table name restriction (better not be NULL!)
	WCHAR	*pwszConsCatalogR,	// [in] constraint catalog restriction
	WCHAR	*pwszConsSchemaR,	// [in] constraint schema restriction
	WCHAR	*pwszConsNameR		// [in] constraint name restriction
)
{
	HRESULT			hr = E_FAIL;
	CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
	CComfRowset		Rowset;
	IRowset			*pIRowset = NULL;
	const ULONG		cMaxRestr = 7;
	VARIANT			rgvRestr[cMaxRestr];
	ULONG			ulIndex, ulRow;
	CTable			*pTable = new CTable(m_pSessionIUnknown, L"NoModuleName");
	DBCOUNTITEM		cRows = 0;

	ULONG			ulConsCatalog		= 0;	// column ordinal
	WCHAR			*pwszConsCatalog	= NULL;
	ULONG			ulConsSchema		= 1;	// column ordinal
	WCHAR			*pwszConsSchema		= NULL;
	ULONG			ulConsName			= 2;	// column ordinal
	WCHAR			*pwszConsName		= NULL;

	ULONG			ulTableCatalog		= 3;	// column ordinal
	WCHAR			*pwszTableCatalog	= NULL;
	ULONG			ulTableSchema		= 4;	// column ordinal
	WCHAR			*pwszTableSchema	= NULL;
	ULONG			ulTableName			= 5;	// column ordinal
	WCHAR			*pwszTableName		= NULL;

	ULONG			ulConstrType		= 6;	// column ordinal
	WCHAR			*pwszConstrType		= NULL;
	ULONG			ulIsDeferrable		= 7;	// column ordinal
	VARIANT_BOOL	*pbIsDeferrable	= NULL;
	ULONG			ulInitiallyDeferred	= 8;	// column ordinal
	VARIANT_BOOL	*pbInitiallyDeferred= NULL;
	DBDEFERRABILITY	Deferrability		= 0;
	HROW			hRow;

	// these var tell whether various restrictions are supported
	BOOL			fTableCatalogR		= FALSE;
	BOOL			fTableSchemaR		= FALSE;
	BOOL			fTableNameR			= FALSE;
	BOOL			fConsCatalogR		= FALSE;
	BOOL			fConsSchemaR		= FALSE;
	BOOL			fConsNameR			= FALSE;

	CConstraintRestrictions **prgConsRestrictions = NULL;
	ULONG					cConsRestrictions = 0;

	ULONG		ulRestrictionSupport;
	CConsDesc		*pCons				= NULL;

	BOOL			fDeletepCons = TRUE;

	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantInit(&rgvRestr[ulIndex]);
	}

	TESTC(NULL != m_pSessionIUnknown);

	FreeConstraintDesc(&m_cConstraints, &m_rgConstraints);
	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);
	SAFE_FREE(m_pwszConsCatalog);
	SAFE_FREE(m_pwszConsSchema);
	SAFE_FREE(m_pwszConsName);
	m_pwszTableCatalog	= wcsDuplicate(pwszTableCatalogR);
	m_pwszTableSchema	= wcsDuplicate(pwszTableSchemaR);
	m_pwszTableName		= wcsDuplicate(pwszTableNameR);
	m_pwszConsCatalog	= wcsDuplicate(pwszConsCatalogR);
	m_pwszConsSchema	= wcsDuplicate(pwszConsSchemaR);
	m_pwszConsName		= wcsDuplicate(pwszConsNameR);

	// There may be situations where Fully qualified table name may contain the catalog, schema
	// and table name. In this situation, m_pwszTableCatalog and m_pwszTableSchema might be null.
	// So, call ParseQualTableName() to retrieve each of these components in their proper pointers.
	if (!m_pwszTableCatalog && !m_pwszTableSchema && m_pwszTableName)
		ParseQualTableName(m_pwszTableName,
			&m_pwszTableCatalog, &m_pwszTableSchema, &m_pwszTableName);

	// Similarly, do it for constraint name
	if (!m_pwszConsCatalog && !m_pwszConsSchema && m_pwszConsName)
		ParseQualTableName(m_pwszConsName,
		&m_pwszConsCatalog, &m_pwszConsSchema, &m_pwszConsName);

	TESTC_PROVIDER(S_OK == SchemaRowset.IsSchemaSupported(DBSCHEMA_TABLE_CONSTRAINTS));
	TESTC_(SchemaRowset.GetRestrictionSupport(DBSCHEMA_TABLE_CONSTRAINTS, &ulRestrictionSupport), S_OK);

	// set the restrictions
	fTableCatalogR	= ulRestrictionSupport & 8;
	fTableSchemaR	= ulRestrictionSupport & 16;
	fTableNameR		= ulRestrictionSupport & 32;
	
	fConsCatalogR	= ulRestrictionSupport & 1;
	fConsSchemaR	= ulRestrictionSupport & 2;
	fConsNameR		= ulRestrictionSupport & 4;

	if (m_pwszTableCatalog && fTableCatalogR)
	{
		rgvRestr[3].vt = VT_BSTR;
		V_BSTR(&rgvRestr[3]) = SysAllocString(m_pwszTableCatalog);
	}
	if (m_pwszTableSchema && fTableSchemaR)
	{
		rgvRestr[4].vt = VT_BSTR;
		V_BSTR(&rgvRestr[4]) = SysAllocString(m_pwszTableSchema);
	}
	if (m_pwszTableName && fTableNameR)
	{
		rgvRestr[5].vt = VT_BSTR;
		V_BSTR(&rgvRestr[5]) = SysAllocString(m_pwszTableName);
	}
	if (m_pwszConsCatalog && fConsCatalogR)
	{
		rgvRestr[0].vt = VT_BSTR;
		V_BSTR(&rgvRestr[0]) = SysAllocString(m_pwszConsCatalog);
	}
	if (m_pwszConsSchema && fConsSchemaR)
	{
		rgvRestr[1].vt = VT_BSTR;
		V_BSTR(&rgvRestr[1]) = SysAllocString(m_pwszConsSchema);
	}
	if (m_pwszConsName && fConsNameR)
	{
		rgvRestr[2].vt = VT_BSTR;
		V_BSTR(&rgvRestr[2]) = SysAllocString(m_pwszConsName);
	}


	// get TABLE_CONSTRAINTS Rowset
	TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_TABLE_CONSTRAINTS, cMaxRestr, rgvRestr, 
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	Rowset.DropRowset();
	Rowset.SetTable(pTable, DELETETABLE_NO);

	TESTC_PROVIDER(S_OK == Rowset.CreateRowset(pIRowset, USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK));

	m_cConstraints = Rowset.GetTotalRows();
	cRows = m_cConstraints;

	SAFE_ALLOC(m_rgConstraints, DBCONSTRAINTDESC, m_cConstraints);
	memset(m_rgConstraints, 0, (size_t)m_cConstraints*sizeof(DBCONSTRAINTDESC));
	
	SAFE_ALLOC(prgConsRestrictions, CConstraintRestrictions*, m_cConstraints);
	memset(prgConsRestrictions, 0, (size_t)m_cConstraints*sizeof(CConstraintRestrictions*));

	//for (ulIndex = 0; ulIndex < m_cConstraints; ulIndex++)
	for (ulRow = 0,ulIndex = 0; ulRow < cRows && ulIndex < m_cConstraints; ulRow++)
	{
		TESTC_(Rowset.GetNextRows(&hRow), S_OK);
		TESTC_(hr = Rowset.GetRowData(hRow, &Rowset.m_pData),S_OK);

		// get the type of the constraint
		pwszConstrType = (WCHAR*)Rowset.GetValue(ulConstrType);
		TESTC(NULL != pwszConstrType);

		// get table info
		pwszTableCatalog	= (WCHAR*)Rowset.GetValue(ulTableCatalog);
		pwszTableSchema		= (WCHAR*)Rowset.GetValue(ulTableSchema);
		pwszTableName		= (WCHAR*)Rowset.GetValue(ulTableName);
		COMPARE(NULL != pwszTableName, TRUE);

		// get constraint info
		pwszConsCatalog		= (WCHAR*)Rowset.GetValue(ulConsCatalog);
		pwszConsSchema		= (WCHAR*)Rowset.GetValue(ulConsSchema);
		pwszConsName		= (WCHAR*)Rowset.GetValue(ulConsName);

		TESTC(ulIndex<m_cConstraints);
		if (0 == wcscmp(pwszConstrType, L"UNIQUE"))
			pCons = new CUniqueCons(&m_rgConstraints[ulIndex++]); 
		else if (0 == wcscmp(pwszConstrType, L"FOREIGN KEY"))
			pCons = new CForeignKeyCons(&m_rgConstraints[ulIndex++]); 
		else if (0 == wcscmp(pwszConstrType, L"PRIMARY KEY"))
			pCons = new CPrimaryKeyCons(&m_rgConstraints[ulIndex++]);
		else if (0 == wcscmp(pwszConstrType, L"CHECK"))
			pCons = new CCheckCons(&m_rgConstraints[ulIndex++]);
		else
			TESTC(FALSE);

		TESTC(pCons!=NULL);
		fDeletepCons = TRUE;

		if (COMPARE(NULL != pwszConsName, TRUE))
			// fill-in constraint data
			pCons->SetConstraintName(pwszConsName);

		// this should be checked in an inferior layer, actually
		if (	!CheckRestriction(fTableCatalogR,	pwszTableCatalogR,	pwszTableCatalog)
			||	!CheckRestriction(fTableSchemaR,	pwszTableSchemaR,	pwszTableSchema)
			||	!CheckRestriction(fTableNameR,		pwszTableNameR,		pwszTableName)
			||	!CheckRestriction(fConsCatalogR,	pwszConsCatalogR,	pwszConsCatalog)
			||	!CheckRestriction(fConsSchemaR,		pwszConsSchemaR,	pwszConsSchema)
			||	!CheckRestriction(fConsNameR,		pwszConsNameR,		pwszConsName)
			)
			goto LOOP;

		// get deferrabilitty and initial deref
		pbIsDeferrable = (VARIANT_BOOL*)Rowset.GetValue(ulIsDeferrable);
		pbInitiallyDeferred = (VARIANT_BOOL*)Rowset.GetValue(ulInitiallyDeferred);

		if (NULL != pbIsDeferrable)
			Deferrability = (VARIANT_TRUE == *pbIsDeferrable)? DBDEFERRABILITY_DEFERRED: 0;
			
		if (NULL != pbInitiallyDeferred)
			Deferrability |= (VARIANT_TRUE == *pbInitiallyDeferred)? DBDEFERRABILITY_DEFERRABLE: 0;

		pCons->SetDeferrability(Deferrability);

		// If we're inside transaction we can not have more than one active result set except when MARS is on =>
		// cache pCons object and constraint restrictions so we can call GetInfoFromSchemaRowset 
		// after releasing current result set (DBSCHEMA_TABLE_CONSTRAINTS rowset)
		if (pwszConsName && !CConsDesc::s_fInsideTransaction)
		{	
			pCons->GetInfoFromSchemaRowset(
				pwszTableCatalog, pwszTableSchema, pwszTableName,
				pwszConsCatalog, pwszConsSchema, pwszConsName
			);
		}
		else if (pwszConsName && CConsDesc::s_fInsideTransaction)
		{
			prgConsRestrictions[cConsRestrictions++] = new CConstraintRestrictions(pCons,
				pwszTableCatalog, pwszTableSchema, pwszTableName,
				pwszConsCatalog, pwszConsSchema, pwszConsName
			);
			// set the flag to not delete pCons object as we cached it and will use later
			fDeletepCons = FALSE;
		}


LOOP:
		Rowset.ReleaseRows(hRow);	
		if(fDeletepCons)
			SAFE_DELETE(pCons);
	}

	if(cRows==m_cConstraints)
	{
		TESTC_(Rowset.GetNextRows(&hRow), DB_S_ENDOFROWSET);
	}

	if(CConsDesc::s_fInsideTransaction && prgConsRestrictions && cConsRestrictions>0)
	{
		//we can only have one active result set while in transaction - release one before opening another
		SAFE_RELEASE(pIRowset);
		for (ulIndex=0; ulIndex < cConsRestrictions; ulIndex++)
		{
			prgConsRestrictions[ulIndex]->m_pConsDesc->GetInfoFromSchemaRowset(
				prgConsRestrictions[ulIndex]->m_pwszTableCatalog, prgConsRestrictions[ulIndex]->m_pwszTableSchema, prgConsRestrictions[ulIndex]->m_pwszTableName,
				prgConsRestrictions[ulIndex]->m_pwszConsCatalog, prgConsRestrictions[ulIndex]->m_pwszConsSchema, prgConsRestrictions[ulIndex]->m_pwszConsName
			);
		}

	}

	hr = S_OK;

CLEANUP:
	if(fDeletepCons)
		SAFE_DELETE(pCons);

	SAFE_RELEASE(pIRowset);
	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantClear(&rgvRestr[ulIndex]);
	}

	if(prgConsRestrictions)
	{
		for (ulIndex=0; ulIndex < cConsRestrictions; ulIndex++)
		{
			SAFE_DELETE(prgConsRestrictions[ulIndex]->m_pConsDesc);
			SAFE_DELETE(prgConsRestrictions[ulIndex]);
		}
		SAFE_FREE(prgConsRestrictions);
	}

	delete pTable;
	if (!m_rgConstraints)
		m_cConstraints = 0;
	return hr;
} //CConstraints::GetConstraints



//------------------------------------------------------------------------
//			CConstraints:CConstraints::CheckIncludedConstraints
//	Check specified constraints are included in the current list of constraints
//
//------------------------------------------------------------------------
HRESULT CConstraints::CheckIncludedConstraints(DBORDINAL cConsDesc, DBCONSTRAINTDESC *rgConsDesc)
{
	HRESULT				hr = E_FAIL;
	DBORDINAL			indexCons;
	DBCONSTRAINTDESC	*pConsDesc;
	CConsDesc			*pCons	= NULL;
	CConsDesc			*pCons2	= NULL;

	if (cConsDesc > m_cConstraints)
		goto CLEANUP;

	for (indexCons = 0; indexCons < cConsDesc; indexCons++)
	{
		if (NULL == rgConsDesc[indexCons].pConstraintID)
			continue;

		pConsDesc = (*this)[rgConsDesc[indexCons].pConstraintID];
		// check that this constraint exists in the new constraint list
		if (NULL == pConsDesc)
			goto CLEANUP;

		// if name is the same, check the rest
		switch (pConsDesc->ConstraintType)
		{
			case DBCONSTRAINTTYPE_UNIQUE:
				pCons = new CUniqueCons(pConsDesc);
				pCons2 = new CUniqueCons(&rgConsDesc[indexCons]);
				if (!pCons->IsEqual(pCons2))
					goto CLEANUP;
				break;

			case DBCONSTRAINTTYPE_PRIMARYKEY:
				pCons = new CPrimaryKeyCons(pConsDesc);
				pCons2 = new CPrimaryKeyCons(&rgConsDesc[indexCons]);
				if (!pCons->IsEqual(pCons2))
					goto CLEANUP;
				break;

			case DBCONSTRAINTTYPE_FOREIGNKEY:
				pCons = new CForeignKeyCons(pConsDesc);
				pCons2 = new CForeignKeyCons(&rgConsDesc[indexCons]);
				if (!pCons->IsEqual(pCons2))
					goto CLEANUP;
				break;

			case DBCONSTRAINTTYPE_CHECK:
				pCons = new CCheckCons(pConsDesc);
				pCons2 = new CCheckCons(&rgConsDesc[indexCons]);
				if (!pCons->IsEqual(pCons2))
					goto CLEANUP;
				break;
		}

		SAFE_DELETE(pCons);
		SAFE_DELETE(pCons2);
	}

	hr = S_OK;

CLEANUP:
	SAFE_DELETE(pCons);
	SAFE_DELETE(pCons2);
	return hr;
} //CConstraints::CheckIncludedConstraints




//------------------------------------------------------------------------
//			CConstraints:CheckAddConstraints
//	Check constraint preservation and addition of the new ones
//
//------------------------------------------------------------------------
HRESULT CConstraints::CheckAddConstraints(DBORDINAL cConsDesc, DBCONSTRAINTDESC *rgConsDesc)
{
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			cConstraints = m_cConstraints;
	DBCONSTRAINTDESC	*rgConstraints = m_rgConstraints;
	WCHAR				*pwszTableCatalog = wcsDuplicate(m_pwszTableCatalog);
	WCHAR				*pwszTableSchema = wcsDuplicate(m_pwszTableSchema);
	WCHAR				*pwszTableName = wcsDuplicate(m_pwszTableName);
	WCHAR				*pwszConsCatalog = wcsDuplicate(m_pwszConsCatalog);
	WCHAR				*pwszConsSchema = wcsDuplicate(m_pwszConsSchema);
	WCHAR				*pwszConsName = wcsDuplicate(m_pwszConsName);

	m_cConstraints	= 0;
	m_rgConstraints	= NULL;
	
	// get constraints with the same criteria as before
	TESTC(NULL == m_pwszConsCatalog && NULL == m_pwszConsSchema
		&& NULL == m_pwszConsName);

	TESTC_(GetConstraints(pwszTableCatalog, pwszTableSchema, pwszTableName,
			pwszConsCatalog, pwszConsSchema, pwszConsName), S_OK);

	// compare the 2 lists
	TESTC(cConstraints + cConsDesc == m_cConstraints);
	
	// check that all the constraints
	TESTC_(CheckIncludedConstraints(cConstraints, rgConstraints), S_OK);
	TESTC_(CheckIncludedConstraints(cConsDesc, rgConsDesc), S_OK);

	hr = S_OK;

CLEANUP:
	FreeConstraintDesc(&cConstraints, &rgConstraints);
	SAFE_FREE(pwszTableCatalog);
	SAFE_FREE(pwszTableSchema);
	SAFE_FREE(pwszTableName);
	SAFE_FREE(pwszConsCatalog);
	SAFE_FREE(pwszConsSchema);
	SAFE_FREE(pwszConsName);
	return hr;
} //CConstraints::CheckAddConstraints




//------------------------------------------------------------------------
//			CConstraints:CheckDropConstraint
//	Check constraint preservation and addition of the new ones
//
//------------------------------------------------------------------------
HRESULT CConstraints::CheckDropConstraint(DBCONSTRAINTDESC *pConsDesc)
{
	HRESULT				hr = E_FAIL;
	CConstraints		Cons(this->m_pSessionIUnknown);

	CHECK(CheckIncludedConstraints(1, pConsDesc), S_OK);

	TESTC_(Cons.GetConstraints(m_pwszTableCatalog, m_pwszTableSchema, m_pwszTableName,
		m_pwszConsCatalog, m_pwszConsSchema, m_pwszConsName), S_OK);
	TESTC_(Cons.CheckIncludedConstraints(1, pConsDesc), E_FAIL);

	hr = S_OK;

CLEANUP:
	return hr;
} // CConstraints::CheckDropConstraint




//------------------------------------------------------------------------
//			CConstraints::AreConstraintsPreserved
//	Check constraint preservation 
//
//------------------------------------------------------------------------
HRESULT CConstraints::AreConstraintsPreserved()
{
	HRESULT				hr = E_FAIL;
	DBCOUNTITEM			cConstraints = m_cConstraints;
	DBCONSTRAINTDESC	*rgConstraints = m_rgConstraints;
	WCHAR				*pwszTableCatalog = wcsDuplicate(m_pwszTableCatalog);
	WCHAR				*pwszTableSchema = wcsDuplicate(m_pwszTableSchema);
	WCHAR				*pwszTableName = wcsDuplicate(m_pwszTableName);
	WCHAR				*pwszConsCatalog = wcsDuplicate(m_pwszConsCatalog);
	WCHAR				*pwszConsSchema = wcsDuplicate(m_pwszConsSchema);
	WCHAR				*pwszConsName = wcsDuplicate(m_pwszConsName);

	m_cConstraints = 0;
	m_rgConstraints = NULL;

	// get constraints with the same criteria as before
	TESTC(NULL == m_pwszConsCatalog && NULL == m_pwszConsSchema
		&& NULL == m_pwszConsName);

	TESTC_(GetConstraints(pwszTableCatalog, pwszTableSchema, pwszTableName,
			pwszConsCatalog, pwszConsSchema, pwszConsName), S_OK);

	// compare the 2 lists
	TESTC(cConstraints == m_cConstraints);
	
	// check that all the constraints are preserved
	TESTC_(CheckIncludedConstraints(cConstraints, rgConstraints), S_OK);

	hr = S_OK;

CLEANUP:
	FreeConstraintDesc(&cConstraints, &rgConstraints);
	SAFE_FREE(pwszTableCatalog);
	SAFE_FREE(pwszTableSchema);
	SAFE_FREE(pwszTableName);
	SAFE_FREE(pwszConsCatalog);
	SAFE_FREE(pwszConsSchema);
	SAFE_FREE(pwszConsName);
	return hr;
} // CConstraints::AreConstraintsPreserved




DBCONSTRAINTDESC *CConstraints::operator[](DBID *pConstraintID)
{
	DBORDINAL	index;

	for (index=0; index < m_cConstraints; index++)
	{
		if (CompareDBID(*pConstraintID, *m_rgConstraints[index].pConstraintID, m_pDSOIUnknown))
			return &m_rgConstraints[index];
	}

	return NULL;
} //CConstraints::operator[]



// Check whether a given constraint exists or not
HRESULT CConstraints::DoesConstraintExist(WCHAR *pwszConsName, BOOL &fExist)
{
	HRESULT			hr			= E_FAIL;
	IRowset			*pIRowset	= NULL;
	const ULONG		cMaxRestr	= 7;
	VARIANT			rgvRestr[cMaxRestr];
	HROW			hRow;
	HROW			*phRow		= &hRow;;
	DBORDINAL		cRowsObtained;
	CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
	DBORDINAL		ulIndex;

	fExist = FALSE;

	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantInit(&rgvRestr[ulIndex]);
	}

	TESTC(NULL != pwszConsName);

	// set the restrictions
	if (pwszConsName)
	{
		rgvRestr[2].vt = VT_BSTR;
		V_BSTR(&rgvRestr[2]) = SysAllocString(pwszConsName);
	}

	if (S_OK != SchemaRowset.IsSchemaSupported(DBSCHEMA_TABLE_CONSTRAINTS))
	{
		fExist	= NULL;
		hr		= S_OK;
		goto CLEANUP;
	}

	// get TABLE_CONSTRAINTS Rowset
	TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_TABLE_CONSTRAINTS, cMaxRestr, rgvRestr, 
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	TEST2C_(hr = pIRowset->GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow), 
		S_OK, DB_S_ENDOFROWSET);

	fExist = DB_S_ENDOFROWSET != hr;

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantClear(&rgvRestr[ulIndex]);
	}
	return hr;
} // CConstraints::DoesConstraintExist




// Build a name for a constraint
HRESULT CConstraints::MakeConstraintName(WCHAR **ppwszConsName)
{
	HRESULT			hr			= E_FAIL;		// Result code
	IDBInfo			*pIDBInfo	= NULL;			// IDBInfo interface pointer
	const ULONG		cLiterals	= 1;			
	DBLITERAL		rgLiterals[cLiterals] = {DBLITERAL_TABLE_NAME};
	DBLITERALINFO	*rgLiteralInfo	= NULL;
	DBLITERAL		cLiteralInfo	= 0;
	WCHAR			*pwszLiteralInfoBuffer = NULL;
	DBORDINAL		lConstraintLen;
	const DBORDINAL	ulMaxAttempts	= 200;
	DBORDINAL		i;
	WCHAR			*pwszConsName	= NULL;
	BOOL			fFound;

	TESTC(NULL != ppwszConsName);
	*ppwszConsName = NULL;

	//Obtain IDBInfo interface
	if(!VerifyInterface(m_pDSOIUnknown, IID_IDBInfo, DATASOURCE_INTERFACE, (IUnknown**)&pIDBInfo))
		return E_NOINTERFACE;

	TEST2C_(hr = pIDBInfo->GetLiteralInfo(cLiterals, rgLiterals, 
		&cLiteralInfo, &rgLiteralInfo, &pwszLiteralInfoBuffer), S_OK, DB_E_ERRORSOCCURRED);

	if (S_OK != hr)
		goto CLEANUP;

	if (!rgLiteralInfo[0].fSupported || rgLiteralInfo[0].cchMaxLen < 8)
		lConstraintLen = 8;
	else
		lConstraintLen = rgLiteralInfo[0].cchMaxLen;

	// use the info
	// While constraint is already in data source, 
	// Build new constraint name and check it
	// NOTE: We use a for loop so we don't loop infinitly if
	// a unique constraint cannot be generated...
	for(i=0; i<ulMaxAttempts; i++)
	{
		pwszConsName = MakeObjectName((LPWSTR)gwszModuleName, (size_t)lConstraintLen);

		// If table is found try again, FALSE means not found
		TESTC_(hr = DoesConstraintExist(pwszConsName, fFound), S_OK);
		if(!fFound && 0 > GetConsNameIndex(pwszConsName))
		{
			// Assign name and keep track of it
			*ppwszConsName = pwszConsName;
			
			BookConstraintName(pwszConsName);

			goto CLEANUP;
		}

		//Cleanup here, incase we have to loop again for another tablename...
		SAFE_FREE(pwszConsName);
	}

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pIDBInfo);
	SAFE_FREE(pwszLiteralInfoBuffer);
	SAFE_FREE(rgLiteralInfo);
	return hr;
} //CConstraints::MakeConstraintName




// Book constraint name
HRESULT CConstraints::BookConstraintName(WCHAR *pwszConsName)
{
	HRESULT	hr = E_FAIL;

	if (!pwszConsName)
		return S_OK;

	// save the alloc-ed name in the array
	if (m_cMaxConsNames <= m_cConsNames)
	{
		DBORDINAL		index;
		const DBORDINAL	delta = 10;

		SAFE_REALLOC(m_rgConsNames, WCHAR*, m_cMaxConsNames+delta);
		m_cMaxConsNames += delta;
		for (index = m_cMaxConsNames-delta; index < m_cMaxConsNames; index++)
		{
			m_rgConsNames[index] = NULL;
		}
	}

	SAFE_ALLOC(m_rgConsNames[m_cConsNames], WCHAR, 1 + wcslen(pwszConsName));
	wcscpy(m_rgConsNames[m_cConsNames++], pwszConsName);
	
	hr = S_OK;

CLEANUP:
	return hr;
} //CConstraints::BookConstraintName




LONG CConstraints::GetConsNameIndex(WCHAR *pwszConsName)
{
	LONG	index;

	if (NULL == pwszConsName)
		return -1L;

	ASSERT(LONG_MAX > m_cConsNames);
	for (index = 0; index < (LONG)m_cConsNames; index++)
	{
		if (0 == wcscmp(pwszConsName, m_rgConsNames[index]))
			return index;
	}
	
	return -1L;
} // CConstraints::GetConsNameIndex




HRESULT CConstraints::ReturnConstraintName(WCHAR *pwszConsName)
{
	HRESULT		hr = E_FAIL;
	LONG		index = GetConsNameIndex(pwszConsName);

	if (0 <= index)
	{
		// release current position and copy the end one over it
		SAFE_FREE(m_rgConsNames[index]);
		m_rgConsNames[index] = m_rgConsNames[--m_cConsNames];
		m_rgConsNames[m_cConsNames] = NULL;
	}

	hr = S_OK;

	return hr;
} // CConstraints::ReturnConstraintName






	
//-----------------------------------------------------------
//
//	Class CConsDesc
//
//-----------------------------------------------------------
IUnknown		*CConsDesc::s_pSessionIUnknown	= NULL;
IUnknown		*CConsDesc::s_pDSOIUnknown		= NULL;
CConstraints	*CConsDesc::s_pConstraints		= NULL;
BOOL			CConsDesc::s_fInsideTransaction = FALSE;

CConsDesc::CConsDesc(
	DBCONSTRAINTTYPE	ConsType, 
	DBDEFERRABILITY		Def/*=0*/
)
{
	TESTC(NULL != s_pConstraints);

	m_fFreeMem = TRUE;
	// alloc memory for constraint description
	SAFE_ALLOC(m_pConsDesc, DBCONSTRAINTDESC, 1);
	memset(m_pConsDesc, 0, sizeof(DBCONSTRAINTDESC));
	
	SAFE_ALLOC(m_pConsDesc->pConstraintID, DBID, 1);

	m_pConsDesc->pConstraintID->eKind = DBKIND_NAME;

	// create a default name
	TESTC_(s_pConstraints->MakeConstraintName(&m_pConsDesc->pConstraintID->uName.pwszName), S_OK);
	
	m_pConsDesc->ConstraintType = ConsType;
	m_pConsDesc->Deferrability = Def;

CLEANUP:
	return;
} //CConsDesc::CConsDesc




CConsDesc::CConsDesc(
	DBCONSTRAINTDESC	*pConsDesc
)
{
	TESTC(NULL != pConsDesc);
	m_fFreeMem	= FALSE;
	m_pConsDesc = pConsDesc;

CLEANUP:
	return;
} //CConsDesc::CConsDesc




CConsDesc::~CConsDesc()
{
	DBORDINAL	cConsDesc=1;

	if (	m_pConsDesc && m_pConsDesc->pConstraintID 
		&&	DBKIND_NAME == m_pConsDesc->pConstraintID->eKind
		&&	s_pConstraints)
		s_pConstraints->ReturnConstraintName(m_pConsDesc->pConstraintID->uName.pwszName);

	if (m_fFreeMem)
		FreeConstraintDesc(&cConsDesc, &m_pConsDesc);
} //CConsDesc::~CConsDesc




void CConsDesc::SetConstraintName(WCHAR *pwszConstraintName)
{
	if (!m_pConsDesc->pConstraintID)
	{
		SAFE_ALLOC(m_pConsDesc->pConstraintID, DBID, 1);
	}
	else
	{
		if (DBKIND_NAME == m_pConsDesc->pConstraintID->eKind &&	s_pConstraints)
			s_pConstraints->ReturnConstraintName(m_pConsDesc->pConstraintID->uName.pwszName);
		ReleaseDBID(m_pConsDesc->pConstraintID, FALSE);
	}

	m_pConsDesc->pConstraintID->eKind = DBKIND_NAME;
	m_pConsDesc->pConstraintID->uName.pwszName = wcsDuplicate(pwszConstraintName);
	s_pConstraints->BookConstraintName(pwszConstraintName);

CLEANUP:
	return;
} // CConsDesc::SetConstraintName




DBID *CConsDesc::SetConstraintID(DBID *pConstraintID)
{
	if (	m_pConsDesc && m_pConsDesc->pConstraintID 
		&&	DBKIND_NAME == m_pConsDesc->pConstraintID->eKind &&	s_pConstraints)
		s_pConstraints->ReturnConstraintName(m_pConsDesc->pConstraintID->uName.pwszName);

	ReleaseDBID(m_pConsDesc->pConstraintID, NULL == pConstraintID);

	if (pConstraintID)
	{
		if (!m_pConsDesc->pConstraintID)
		{
			SAFE_ALLOC(m_pConsDesc->pConstraintID, DBID, 1);
		}

		DuplicateDBID(*pConstraintID, m_pConsDesc->pConstraintID);

		if ((DBKIND_NAME == pConstraintID->eKind) && pConstraintID->uName.pwszName)
			// make clear you use this name			
			s_pConstraints->BookConstraintName(pConstraintID->uName.pwszName);
	}

CLEANUP:
	return m_pConsDesc->pConstraintID;
} //CConsDesc::SetConstraintID



HRESULT CConsDesc::CopyTo(DBCONSTRAINTDESC *pCD)
{
	HRESULT		hr			= E_FAIL;
	DBORDINAL	cConsDesc	= 1;

	if (pCD)
	{
		CConsDesc			cons(pCD);
	
		// release all the unused content
		FreeConstraintDesc(&cConsDesc, &pCD, FALSE);

		// copy  from the other structure, just the relevant fields
		cons.SetConstraintID(GetConstraintID());
		cons.SetConstraintType(GetConstraintType());
		cons.SetDeferrability(GetDeferrability());
	
		hr = S_OK;
	}

	return hr;
} //CConsDesc::CopyTo




// Build a name for a constraint
HRESULT CConsDesc::MakeConstraintName()
{
	HRESULT			hr = E_FAIL;
	WCHAR			*pwszName = NULL;

	TESTC(NULL != s_pConstraints);
	TESTC_(s_pConstraints->MakeConstraintName(&pwszName), S_OK);
	SetConstraintName(pwszName);
	// return the name to the pool because both MakeConstraintName and
	// SetCosntraintName book the name
	s_pConstraints->ReturnConstraintName(pwszName);
	
	hr = S_OK;

CLEANUP:
	SAFE_FREE(pwszName);
	return hr;
} //CConsDesc::MakeConstraintName




CKeyConstraint::CKeyConstraint(
	DBCONSTRAINTTYPE	ConsType, 
	DBDEFERRABILITY		Def/*=0*/
) : CConsDesc(ConsType, Def)
{
	TESTC(	DBCONSTRAINTTYPE_UNIQUE == ConsType 
		||	DBCONSTRAINTTYPE_PRIMARYKEY == ConsType
		||	DBCONSTRAINTTYPE_FOREIGNKEY == ConsType);

	// allocate initial space for rgColumnList
	m_cMaxColumns = 10;

	SAFE_ALLOC(m_pConsDesc->rgColumnList, DBID, m_cMaxColumns);
	memset(m_pConsDesc->rgColumnList, 0, (size_t)m_cMaxColumns * sizeof(DBID));

CLEANUP:
	return;
} // CKeyConstraint::CKeyConstraint




DBID *CKeyConstraint::AddColumnID(
	DBORDINAL		*pcMaxColumns,	// [in/out] maximum number of columns
	DBORDINAL		*pcColumns,		// [in/out] number of columns
	DBID			**prgColumnList,	// [in/out] array of columns
	DBID			*pColumnID		// [in] column ID to be added
)
{
	DBORDINAL	cColumns;
	DBORDINAL	cMaxColumns;
	DBID		*rgColumnList;

	TESTC(NULL != pcMaxColumns);
	TESTC(NULL != pcColumns);
	TESTC(NULL != prgColumnList);
	TESTC(NULL != pColumnID);

	cColumns		= *pcColumns;
	cMaxColumns		= *pcMaxColumns;
	rgColumnList	= *prgColumnList;

	if (cMaxColumns <= cColumns)
	{
		const DBORDINAL	cDelta	= 10;
		DBORDINAL		lower	= cMaxColumns;

		// resize the column list
		cMaxColumns += cDelta;
		SAFE_REALLOC(rgColumnList, DBID, cMaxColumns);
		// fill the extra with 0
		memset(rgColumnList + lower, 0, cDelta * sizeof(DBID));

		*prgColumnList	= rgColumnList;
		*pcMaxColumns	= cMaxColumns;
	}

	// add the columnID
	DuplicateDBID(*pColumnID, &rgColumnList[cColumns++]);
	(*pcColumns)++;

CLEANUP:
	return rgColumnList;
} // CKeyConstraint::AddColumnID




HRESULT CKeyConstraint::ReleaseColumnIDs(
	DBORDINAL		*pcMaxColumns,		// [in/out] maximum number of columns
	DBORDINAL		*pcColumns,			// [in/out] number of columns
	DBID			**prgColumnList		// [in/out] array of columns
)
{
	DBORDINAL	index;
	HRESULT		hr = E_FAIL;

	TESTC(NULL != pcMaxColumns);
	TESTC(NULL != pcColumns);
	TESTC(NULL != prgColumnList);

	for (index = 0; index < *pcColumns; index++)
	{
		ReleaseDBID(*prgColumnList + index, FALSE);
	}

	SAFE_FREE(*prgColumnList);

	*pcColumns		= 0;
	*pcMaxColumns	= 0;

	hr = S_OK;

CLEANUP:
	return hr;
} // CKeyConstraint::ReleaseColumnIDs




HRESULT CKeyConstraint::SetConstraint(
	CTable				*pTable, 
	DBORDINAL			cCol,
	DBORDINAL			*rgCol,
	DBDEFERRABILITY		Deferrability		/*= 0*/
)
{
	HRESULT		hr = E_FAIL;
	CCol		col;
	DBORDINAL	index;

	TESTC(NULL != pTable);
	TESTC(0 == cCol || NULL != rgCol);
	
	TESTC_(pTable->GetColInfo(rgCol[0], col), S_OK);
	{
		SetDeferrability(Deferrability);

		CHECK(ReleaseColumnList(), S_OK);

		for (index=0; index < cCol; index++)
		{
			TESTC_(pTable->GetColInfo(rgCol[index], col), S_OK);

			AddColumn(col.GetColID());
		}	
	}
	
	hr = S_OK;

CLEANUP:
	return hr;
} // CKeyConstraint::SetConstraint




HRESULT CKeyConstraint::CopyTo(DBCONSTRAINTDESC *pCD)
{
	HRESULT	hr = E_FAIL;

	if (S_OK == CConsDesc::CopyTo(pCD))
	{
		DBORDINAL	index;
		CKeyConstraint		cons(pCD);

		// and now, fix the list of columns
		if (rgColumnList())
		{
			for (index = 0; index < cColumns(); index++)
			{
				cons.AddColumn(rgColumnList() + index);
			}
		}
		else
		{
			((DBCONSTRAINTDESC*)cons)->cColumns = cColumns();
			((DBCONSTRAINTDESC*)cons)->rgColumnList = NULL;
		}

		hr = S_OK;
	}

	return hr;
} // CKeyConstraint::CopyTo



LONG CKeyConstraint::GetColumnIndex(
	DBORDINAL		ulColumns,	
	DBID			*rgColumnList, 
	DBID			*pColumnID
)
{
	LONG		index;

	if (!rgColumnList || !pColumnID)
		return -1;

	ASSERT(LONG_MAX > ulColumns);
	for (index = 0; index < (LONG)ulColumns; index++)
	{
		if (CompareDBID(*pColumnID, rgColumnList[index], s_pDSOIUnknown))
			return index;
	}

	return -1;
} //CKeyConstraint::GetColumnIndex



BOOL CKeyConstraint::IsEqual(CConsDesc *pConsDesc)
{
	CKeyConstraint		*pCons	= NULL;
	DBORDINAL			index;

	if (!CConsDesc::IsEqual(pConsDesc))
		return FALSE;

	switch (GetConstraintType())
	{
		case DBCONSTRAINTTYPE_UNIQUE:
			if (!CKeyColumnUsageRowset::IsSchemaSupported())
				return TRUE;
			break;

		case DBCONSTRAINTTYPE_PRIMARYKEY:
			if (!CPKRowset::IsSchemaSupported())
				return TRUE;
			break;

		case DBCONSTRAINTTYPE_FOREIGNKEY:
			if (!CFKRowset::IsSchemaSupported())
				return TRUE;
			break;

		default:
			ASSERT(FALSE);
	}
	
	pCons = (CKeyConstraint*)pConsDesc;

	// check the number of columns
	if (!COMPARE(0 < pCons->cColumns(), TRUE))
		return FALSE;
	if (!COMPARE(0 < cColumns(), TRUE))
		return FALSE;

	if (!COMPARE(pCons->cColumns() == cColumns(), TRUE))
		return FALSE;

	// check column list
	for (index = 0; index < cColumns(); index++)
	{
		if (!COMPARE(-1 < GetColumnIndex(pCons->rgColumnList() + index), TRUE))
			return FALSE;
	}

	return TRUE;
} //CKeyConstraint::IsEqual



HRESULT CPrimaryKeyCons::GetInfoFromSchemaRowset(
	WCHAR			*pwszTableCatalog,
	WCHAR			*pwszTableSchema,
	WCHAR			*pwszTableName,
	WCHAR			*pwszConsCatalog,
	WCHAR			*pwszConsSchema,
	WCHAR			*pwszConsName
)
{
	DBORDINAL	index;
	HRESULT		hr			= E_FAIL;
	DBID		*pColumnID	= NULL;
	WCHAR		*pwszPKName	= NULL;
	CPKRowset	PKRowset(s_pSessionIUnknown);

	if (!CPKRowset::IsSchemaSupported())
		return S_OK;

	TESTC_(PKRowset.GetPrimaryKeys(pwszTableCatalog, pwszTableSchema, pwszTableName), S_OK);

	for (index = 0; ;index++)
	{
		pColumnID = PKRowset[index];

		if (!pColumnID)
			break;

		AddColumn(pColumnID);
		ReleaseDBID(pColumnID, TRUE);
		pColumnID = NULL;
	}

	TESTC(0 < index);
	pwszPKName = PKRowset.GetPKName();
	TESTC(	DBKIND_NAME == m_pConsDesc->pConstraintID->eKind
		&&	m_pConsDesc->pConstraintID->uName.pwszName
		&&	L'\0' != m_pConsDesc->pConstraintID->uName.pwszName[0]
		&&	pwszPKName
		&&	0 == wcscmp(pwszPKName, m_pConsDesc->pConstraintID->uName.pwszName));
	hr = S_OK;

CLEANUP:
	return hr;
} //CPrimaryKeyCons::GetInfoFromSchemaRowset



CForeignKeyCons::CForeignKeyCons(
	DBID			*pReferencedID	/*= NULL*/,
	DBMATCHTYPE		MatchType		/*= DBMATCHTYPE_NONE*/, 
	DBUPDELRULE		UpdateRule		/*= DBUPDELRULE_NOACTION*/,
	DBUPDELRULE		DeleteRule		/*= DBUPDELRULE_NOACTION*/,
	DBDEFERRABILITY	Def				/*= 0*/
) :  CKeyConstraint(DBCONSTRAINTTYPE_FOREIGNKEY, Def) 
{
	m_pConsDesc->UpdateRule	= UpdateRule;
	m_pConsDesc->DeleteRule	= DeleteRule;
	m_pConsDesc->MatchType	= MatchType;
	SetReferencedTableID(pReferencedID);
	
	// allocate initial space for rgForeignKeyColumnList
	m_cMaxForeignKeyColumns = 10;

	SAFE_ALLOC(m_pConsDesc->rgForeignKeyColumnList, DBID, m_cMaxForeignKeyColumns);
	memset(m_pConsDesc->rgForeignKeyColumnList, 0, 
		(size_t)m_cMaxForeignKeyColumns * sizeof(DBID));

CLEANUP:
	return;
} //CForeignKeyCons::CForeignKeyCons




DBID *CForeignKeyCons::SetReferencedTableID(DBID *pDBID)
{
	ReleaseDBID(m_pConsDesc->pReferencedTableID, NULL == pDBID);

	if (pDBID)
	{
		if (!m_pConsDesc->pReferencedTableID)
		{
			SAFE_ALLOC(m_pConsDesc->pReferencedTableID, DBID, 1);
		}

		DuplicateDBID(*pDBID, m_pConsDesc->pReferencedTableID);
	}
	else
		m_pConsDesc->pReferencedTableID = NULL;

CLEANUP:
	return m_pConsDesc->pReferencedTableID;
} //CForeignKeyCons::SetReferenceTableID




HRESULT CForeignKeyCons::SetConstraint(
	CTable				*pBaseTable, 
	DBORDINAL			cBaseCol,
	DBORDINAL			*rgBaseCol,
	CTable				*pReferencedTable, 
	DBORDINAL			cReferencedCol,
	DBORDINAL			*rgReferencedCol,
	DBMATCHTYPE			MatchType			/*= DBMATCHTYPE_FULL*/,
	DBUPDELRULE			UpdateRule			/*= DBUPDELRULE_NOACTION*/,
	DBUPDELRULE			DeleteRule			/*= DBUPDELRULE_NOACTION*/,
	DBDEFERRABILITY		Deferrability		/*= 0*/
)
{
	HRESULT				hr = E_FAIL;
	CCol				col;
	DBORDINAL			index;

	TESTC(NULL != m_pConsDesc);
	TESTC(NULL != pBaseTable);
	TESTC(NULL != pReferencedTable);
	TESTC(0 == cBaseCol || NULL != rgBaseCol);
	TESTC(0 == cReferencedCol || NULL != rgReferencedCol);

	SetReferencedTableID(&pReferencedTable->GetTableID());

	SetUpdateRule(UpdateRule);
	SetDeleteRule(DeleteRule);
	SetMatchType(MatchType);
	SetDeferrability(Deferrability);

	// set column lists
	CHECK(ReleaseForeignKeyColumnList(), S_OK);
	for (index=0; index < cBaseCol; index++)
	{
		TESTC_(pBaseTable->GetColInfo(rgBaseCol[index], col), S_OK);
		AddForeignKeyColumn(col.GetColID());
	}

	// set list of foreign columns
	CHECK(ReleaseColumnList(), S_OK);
	for (index=0; index < cReferencedCol; index++)
	{
		TESTC_(pReferencedTable->GetColInfo(rgReferencedCol[index], col), S_OK);
		AddColumn(col.GetColID());
	}	

	hr = S_OK;

CLEANUP:
	return hr;
} // CForeignKeyCons::SetConstraint




HRESULT CForeignKeyCons::CopyTo(DBCONSTRAINTDESC *pCD)
{
	HRESULT	hr = E_FAIL;

	if (S_OK == CKeyConstraint::CopyTo(pCD))
	{
		DBORDINAL			index;
		CForeignKeyCons		cons(pCD);

		cons.SetReferencedTableID(GetReferencedTableID());
		cons.SetMatchType(GetMatchType());
		cons.SetUpdateRule(GetUpdateRule());
		cons.SetDeleteRule(GetDeleteRule());

		// and now, fix the list of columns
		if (rgForeignKeyColumnList())
		{
			for (index = 0; index < cColumns(); index++)
			{
				cons.AddForeignKeyColumn(rgForeignKeyColumnList() + index);
			}
		}
		else
		{
			((DBCONSTRAINTDESC*)cons)->rgForeignKeyColumnList = NULL;
			((DBCONSTRAINTDESC*)cons)->cForeignKeyColumns = cForeignKeyColumns();
		}

		hr = S_OK;
	}

	return hr;
} // CForeignKeyCons::CopyTo



HRESULT CForeignKeyCons::GetInfoFromSchemaRowset(
	WCHAR			*pwszTableCatalog,
	WCHAR			*pwszTableSchema,
	WCHAR			*pwszTableName,
	WCHAR			*pwszConsCatalog,
	WCHAR			*pwszConsSchema,
	WCHAR			*pwszConsName
)
{
	DBORDINAL	index;
	DBID		RefTableID;
	HRESULT		hr			= E_FAIL;
	CFKRowset	FKRowset(s_pSessionIUnknown);

	if (!CFKRowset::IsSchemaSupported())
		return S_OK;

	TESTC_(FKRowset.GetForeignKeys(pwszTableCatalog, pwszTableSchema, pwszTableName, pwszConsName), S_OK);

	for (index = 0; FKRowset.MoveToRow(index); index++)
	{
		AddColumn(FKRowset.GetCrtReferencedColID());
		AddForeignKeyColumn(FKRowset.GetCrtFKColID());
	}

	TESTC(0 < index);

	RefTableID.eKind = DBKIND_NAME;
	RefTableID.uName.pwszName = wcsDuplicate(FKRowset.GetReferencedTableName());
	SetReferencedTableID(&RefTableID);

	SAFE_FREE(RefTableID.uName.pwszName);

	SetUpdateRule(FKRowset.GetUpdateRule());
	SetDeleteRule(FKRowset.GetDeleteRule());

	hr = S_OK;

CLEANUP:
	return hr;
} //CForeignKeyCons::GetInfoFromSchemaRowset



BOOL CForeignKeyCons::IsEqual(CConsDesc *pConsDesc)
{
	CForeignKeyCons		*pCons	= NULL;
	DBORDINAL			index;

	if (!CKeyConstraint::IsEqual(pConsDesc))
		return FALSE;

	// if getting info from the schema rowset is not possible, skip further comparison
	if (!CFKRowset::IsSchemaSupported())
		return TRUE;

	pCons = (CForeignKeyCons*)pConsDesc;

	// check the number of columns 
	if (!COMPARE(0 < pCons->cForeignKeyColumns(), TRUE))
		return FALSE;
	if (!COMPARE(0 < cForeignKeyColumns(), TRUE))
		return FALSE;

	if (!COMPARE(pCons->cForeignKeyColumns() == cForeignKeyColumns(), TRUE))
		return FALSE;

	// check delete and update rules
	if (!COMPARE(pCons->GetDeleteRule() == GetDeleteRule(), TRUE))
		return FALSE;

	if (!COMPARE(pCons->GetUpdateRule() == GetUpdateRule(), TRUE))
		return FALSE;

	// check match type
	// Note: include this as soon as FOREIGN_KEYS stores info about match type
//	if (!COMPARE(pCons->GetMatchType() == GetMatchType(), TRUE))
//		return FALSE;

	// check referenced table id
	if (!COMPARE(NULL != pCons->GetReferencedTableID(), TRUE))
		return FALSE;
	if (!COMPARE(NULL != GetReferencedTableID(), TRUE))
		return FALSE;

	IUnknown	*pIUnknown = NULL;

	if (!COMPARE(CompareDBID(*(pCons->GetReferencedTableID()), *(GetReferencedTableID()), s_pDSOIUnknown), TRUE))
		return FALSE;

	// check the column that make up the foreign key
	for (index = 0; index < cForeignKeyColumns(); index++)
	{
		if (!COMPARE(-1 < GetFKColumnIndex(pCons->rgForeignKeyColumnList() + index), TRUE))
			return FALSE;
	}

	return TRUE;
} //CForeignKeyCons::IsEqual



HRESULT CCheckCons::CopyTo(DBCONSTRAINTDESC *pCD)
{
	HRESULT	hr = E_FAIL;

	if (S_OK == CConsDesc::CopyTo(pCD))
	{
		CCheckCons		cons(pCD);

		cons.SetConstraintText(GetConstraintText());

		hr = S_OK;
	}

	return hr;
} // CCheckCons::CopyTo




HRESULT CCheckCons::GetInfoFromSchemaRowset(
	WCHAR			*pwszTableCatalog,
	WCHAR			*pwszTableSchema,
	WCHAR			*pwszTableName,
	WCHAR			*pwszConsCatalog,
	WCHAR			*pwszConsSchema,
	WCHAR			*pwszConsName
)
{
	HRESULT				hr					= E_FAIL;
	WCHAR				*pwszCheckClause	= NULL;
	CCheckConsRowset	CheckConsRowset(s_pSessionIUnknown);

	if (!CCheckConsRowset::IsSchemaSupported())
		return S_OK;

	TESTC_(CheckConsRowset.GetCheckConstraints(pwszConsCatalog, pwszConsSchema, pwszConsName), S_OK);

	pwszCheckClause = CheckConsRowset[0];
	if (pwszCheckClause)
		SetConstraintText(pwszCheckClause);

	hr = S_OK;

CLEANUP:
	return hr;
} //CCheckCons::GetInfoFromSchemaRowset



HRESULT CCheckCons::SetIsNotNULLCheckConstraint(
	CTable			*pTable,
	DBORDINAL		ulCol
)
{
	CCol		col;
	HRESULT		hr = E_FAIL;

	TESTC_(pTable->GetColInfo(ulCol , col), S_OK);
		
	hr = SetIsNotNULLCheckConstraint(col.GetColName());

CLEANUP:
	return hr;
} //CCheckCons::SetIsNotNULLCheckConstraint



HRESULT CCheckCons::SetIsNotNULLCheckConstraint(
	WCHAR			*pwszColName
)
{
	HRESULT		hr					= E_FAIL;
	WCHAR		*pwszConstraintText	= NULL;
	WCHAR		*wszClause			= L" IS NOT NULL";
	CTable		Table(s_pSessionIUnknown, (LPWSTR)gwszModuleName);

	// create the text of the constraint
	SAFE_ALLOC(pwszConstraintText, WCHAR, 
		1 + wcslen(pwszColName) + wcslen(wszClause));
	
	wcscpy(pwszConstraintText, pwszColName);
	wcscat(pwszConstraintText, wszClause);
	
	SetConstraintText(pwszConstraintText);

	hr = S_OK;

CLEANUP:
	SAFE_FREE(pwszConstraintText);
	return hr;
} //CCheckCons::SetIsNotNULLCheckConstraint



HRESULT CCheckCons::SetLTCheckConstraint(
	DBCOLUMNDESC	*pColDesc,
	DBORDINAL		ulSeed
)
{
	HRESULT		hr = E_FAIL;
	CTable		Table(s_pSessionIUnknown, (LPWSTR)gwszModuleName);

	TESTC(Table.ColumnDesc2ColList(pColDesc, 1));

	hr = SetLTCheckConstraint(&Table, 1, ulSeed);

CLEANUP:
	return hr;
} //CCheckCons::SetLTCheckConstraint



HRESULT CCheckCons::SetLTCheckConstraint(
	CTable			*pTable,
	DBORDINAL		ulCol,
	DBORDINAL		ulSeed
)
{
	HRESULT		hr = E_FAIL;
	CTable		Table(s_pSessionIUnknown, (LPWSTR)gwszModuleName);
	WCHAR		*pwszConstraintText	= NULL;
	WCHAR		*pwszColValue		= NULL;
	CCol		col;

	// create the text of the constraint
	TESTC_(pTable->GetColInfo(ulCol , col), S_OK);	
	if (S_OK != pTable->GetLiteralAndValue(col, &pwszColValue, ulSeed, col.GetColNum(), PRIMARY))
	{
		hr = E_FAIL;
		goto CLEANUP;
	}

	SAFE_ALLOC(pwszConstraintText, WCHAR, 1 + wcslen(col.GetColName()) + wcslen(pwszColValue) + 3);
	
	wcscpy(pwszConstraintText, col.GetColName());
	wcscat(pwszConstraintText, L" < ");
	wcscat(pwszConstraintText, pwszColValue);
	
	SetConstraintText(pwszConstraintText);

	hr = S_OK;

CLEANUP:
	SAFE_FREE(pwszConstraintText);
	SAFE_FREE(pwszColValue);
	return hr;
} //CCheckCons::SetLTCheckConstraint



HRESULT CUniqueCons::GetInfoFromSchemaRowset(
	WCHAR			*pwszTableCatalog,
	WCHAR			*pwszTableSchema,
	WCHAR			*pwszTableName,
	WCHAR			*pwszConsCatalog,
	WCHAR			*pwszConsSchema,
	WCHAR			*pwszConsName
)
{
	DBORDINAL				index;
	HRESULT					hr			= E_FAIL;
	DBID					*pColumnID	= NULL;
	CKeyColumnUsageRowset	KeyColumnUsageRowset(s_pSessionIUnknown);

	if (!CKeyColumnUsageRowset::IsSchemaSupported())
		return S_OK;

	TESTC_(KeyColumnUsageRowset.GetKeyColumnUsage(
		pwszTableCatalog, pwszTableSchema, pwszTableName,
		pwszConsCatalog, pwszConsSchema, pwszConsName), S_OK);

	for (index = 0; KeyColumnUsageRowset.MoveToRow(index); index++)
	{
		AddColumn(KeyColumnUsageRowset.GetCrtColID());
	}

	TESTC(0 < index);

	hr = S_OK;

CLEANUP:
	return hr;
} //CUniqueCons::GetInfoFromSchemaRowset



DBCONSTRAINTDESC *CConsDescArray::AddConsDesc(CConsDesc &cons)
{
	// adds a constraint desc to te array
	// it copies cons, including the constraint name
	CConsDesc	*pcons = NULL;

	if (m_cMaxConsDesc <= m_cConsDesc)
	{
		const DBORDINAL	cDelta	= 5;
		DBORDINAL		lower	= m_cMaxConsDesc;

		// resize the array of constraint descriptions
		m_cMaxConsDesc += cDelta;
		SAFE_REALLOC(m_rgConsDesc, DBCONSTRAINTDESC, m_cMaxConsDesc);
		// fill the extra with 0
		memset(m_rgConsDesc + lower, 0, cDelta * sizeof(DBCONSTRAINTDESC));
	}

	CHECK(cons.CopyTo(m_rgConsDesc + m_cConsDesc), S_OK);
	m_cConsDesc++;

CLEANUP:
	delete pcons;
	return m_rgConsDesc;
} // CConsDescArray::AddConsDesc




CCheckConsRowset::CCheckConsRowset(IUnknown *pSessionIUnknown)
{
	IGetDataSource	*pIGetDataSource = NULL;
	CSchemaRowset	SchemaRowset(pSessionIUnknown);
	ULONG			ulRestrictionSupport;
	HRESULT			hr;

	m_pwszConsCatalog	= NULL;
	m_pwszConsSchema	= NULL; 
	m_pwszConsName		= NULL;
	m_pSessionIUnknown	= NULL;
	m_pDSOIUnknown		= NULL;
	m_lCrtPos			= -1;

	m_pTable			= NULL;

	TESTC(NULL != pSessionIUnknown);
	pSessionIUnknown->AddRef();
	m_pSessionIUnknown = pSessionIUnknown;

	if (!s_fMetadataInitialized)
	{
		s_fMetadataInitialized = TRUE;

		hr = SchemaRowset.IsSchemaSupported(DBSCHEMA_CHECK_CONSTRAINTS);
		s_fSchemaSupported = S_OK == hr;
		if (!s_fSchemaSupported)
		{
			odtLog << "DBSCHEMA_CHECK_CONSTRAINTS is not supported!\n";
			goto CLEANUP;
		}

		TESTC_(SchemaRowset.GetRestrictionSupport(DBSCHEMA_CHECK_CONSTRAINTS, &ulRestrictionSupport), S_OK);

		// set the restrictions
		s_fConsCatalogR	= ulRestrictionSupport & 1;
		s_fConsSchemaR	= ulRestrictionSupport & 2;
		s_fConsNameR	= ulRestrictionSupport & 4;
	}
	
	TESTC(VerifyInterface(m_pSessionIUnknown, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown, &m_pDSOIUnknown), S_OK);
	
	m_pTable = new CTable(m_pSessionIUnknown, L"NoModuleName");

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	return;
} //CCheckConsRowset::CCheckConsRowset
	


CCheckConsRowset::~CCheckConsRowset()
{
	delete m_pTable;
	SAFE_FREE(m_pwszConsCatalog);
	SAFE_FREE(m_pwszConsSchema);
	SAFE_FREE(m_pwszConsName);
	SAFE_RELEASE(m_pSessionIUnknown);
	SAFE_RELEASE(m_pDSOIUnknown);
} //CCheckConsRowset::~CCheckConsRowset



HRESULT CCheckConsRowset::GetCheckConstraints(
	WCHAR	*pwszConsCatalogR,	// [in] constraint catalog restriction
	WCHAR	*pwszConsSchemaR,	// [in] constraint schema restriction
	WCHAR	*pwszConsNameR		// [in] constraint name restriction
)
{
	HRESULT			hr = E_FAIL;
	CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
	IRowset			*pIRowset = NULL;
	const ULONG		cMaxRestr = 3;
	VARIANT			rgvRestr[cMaxRestr];

	DBORDINAL		ulIndex;

	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantInit(&rgvRestr[ulIndex]);
	}

	TESTC(NULL != m_pSessionIUnknown);

	SAFE_FREE(m_pwszConsCatalog);
	SAFE_FREE(m_pwszConsSchema);
	SAFE_FREE(m_pwszConsName);

	m_pwszConsCatalog	= wcsDuplicate(pwszConsCatalogR);
	m_pwszConsSchema	= wcsDuplicate(pwszConsSchemaR);
	m_pwszConsName		= wcsDuplicate(pwszConsNameR);

	if (m_pwszConsCatalog && s_fConsCatalogR)
	{
		rgvRestr[0].vt = VT_BSTR;
		V_BSTR(&rgvRestr[0]) = SysAllocString(m_pwszConsCatalog);
	}
	if (m_pwszConsSchema && s_fConsSchemaR)
	{
		rgvRestr[1].vt = VT_BSTR;
		V_BSTR(&rgvRestr[1]) = SysAllocString(m_pwszConsSchema);
	}
	if (m_pwszConsName && s_fConsNameR)
	{
		rgvRestr[2].vt = VT_BSTR;
		V_BSTR(&rgvRestr[2]) = SysAllocString(m_pwszConsName);
	}

	// get CHECK_CONSTRAINTS Rowset
	TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_CHECK_CONSTRAINTS, cMaxRestr, rgvRestr, 
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	m_Rowset.DropRowset();
	m_Rowset.SetTable(m_pTable, DELETETABLE_NO);

	TESTC_PROVIDER(S_OK == m_Rowset.CreateRowset(pIRowset, USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK));
	m_lCrtPos = -1;

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantClear(&rgvRestr[ulIndex]);
	}
	return hr;
} //CCheckConsRowset::GetCheckConstraints



DBCOUNTITEM CCheckConsRowset::GetConstrNo()
{
	TBEGIN
	HROW			hRow;
	DBCOUNTITEM		cRows				= 0;
	HRESULT			hr					= E_FAIL;

	ULONG			ulConsCatalog		= 0;	// column ordinal
	WCHAR			*pwszConsCatalog	= NULL;
	ULONG			ulConsSchema		= 1;	// column ordinal
	WCHAR			*pwszConsSchema		= NULL;
	ULONG			ulConsName			= 2;	// column ordinal
	WCHAR			*pwszConsName		= NULL;

	TESTC_(hr = m_Rowset.RestartPosition(),S_OK);

	while(S_OK == hr)
	{
		//Get the next row
		TEST2C_(hr = m_Rowset.GetNextRows(&hRow), S_OK, DB_S_ENDOFROWSET);

		if(DB_S_ENDOFROWSET == hr)  
		{
			m_Rowset.ReleaseRows(hRow);
			break;
		}

		TESTC_(hr = m_Rowset.GetRowData(hRow, &m_Rowset.m_pData),S_OK);
	
		// get constraint info
		pwszConsCatalog		= (WCHAR*)m_Rowset.GetValue(ulConsCatalog);
		pwszConsSchema		= (WCHAR*)m_Rowset.GetValue(ulConsSchema);
		pwszConsName		= (WCHAR*)m_Rowset.GetValue(ulConsName);

		if (	CheckRestriction(s_fConsCatalogR,	m_pwszConsCatalog,	pwszConsCatalog)
			||	CheckRestriction(s_fConsSchemaR,	m_pwszConsSchema,	pwszConsSchema)
			||	CheckRestriction(s_fConsNameR,		m_pwszConsName,	pwszConsName)
			)
			cRows++;

		m_Rowset.ReleaseRows(hRow);
	}

	//Restore Position to the begining
	TESTC_(hr = m_Rowset.RestartPosition(),S_OK);

CLEANUP:
	return cRows;
} //CCheckConsRowset::GetConstrNo



WCHAR *CCheckConsRowset::operator[](DBORDINAL index)
{
	ULONG			ulConsCatalog		= 0;	// column ordinal
	WCHAR			*pwszConsCatalog	= NULL;
	ULONG			ulConsSchema		= 1;	// column ordinal
	WCHAR			*pwszConsSchema		= NULL;
	ULONG			ulConsName			= 2;	// column ordinal
	WCHAR			*pwszConsName		= NULL;

	ULONG			ulCheckClause		= 3;	// column ordinal
	WCHAR			*pwszCheckClause	= NULL;

	HROW			hRow;
	HRESULT			hr;

	LONG			lIndex				= -1;


	if ((LONG)index < m_lCrtPos)
	{
		TESTC_(hr = m_Rowset.RestartPosition(),S_OK);
		m_lCrtPos = -1;
	}

	lIndex = m_lCrtPos;

	if (- 1 < m_lCrtPos)
		pwszCheckClause		= (WCHAR*)m_Rowset.GetValue(ulCheckClause);

	for (; lIndex < (LONG)index; )
	{
		pwszCheckClause = NULL;

		TEST2C_(hr = m_Rowset.GetNextRows(&hRow), S_OK, DB_S_ENDOFROWSET);

		if (DB_S_ENDOFROWSET == hr)
		{
			m_Rowset.ReleaseRows(hRow);	
			goto CLEANUP;
		}

		TESTC_(hr = m_Rowset.GetRowData(hRow, &m_Rowset.m_pData),S_OK);

		// get constraint info
		pwszConsCatalog		= (WCHAR*)m_Rowset.GetValue(ulConsCatalog);
		pwszConsSchema		= (WCHAR*)m_Rowset.GetValue(ulConsSchema);
		pwszConsName		= (WCHAR*)m_Rowset.GetValue(ulConsName);

		TESTC(NULL != pwszConsName);

		pwszCheckClause		= (WCHAR*)m_Rowset.GetValue(ulCheckClause);

		// this should be checked in an inferior layer, actually
		if (	!CheckRestriction(s_fConsCatalogR,	m_pwszConsCatalog,	pwszConsCatalog)
			||	!CheckRestriction(s_fConsSchemaR,		m_pwszConsSchema,	pwszConsSchema)
			||	!CheckRestriction(s_fConsNameR,		m_pwszConsName,		pwszConsName)
			)
			goto LOOP;

		lIndex++;

LOOP:
		m_Rowset.ReleaseRows(hRow);	
	}

	m_lCrtPos = (LONG)lIndex;

CLEANUP:
	return pwszCheckClause;
} //CCheckConsRowset::operator[]



CPKRowset::CPKRowset(IUnknown *pSessionIUnknown)
{
	IGetDataSource	*pIGetDataSource = NULL;
	CSchemaRowset	SchemaRowset(pSessionIUnknown);
	ULONG			ulRestrictionSupport;
	HRESULT			hr;

	m_pwszPKName		= NULL;
	m_pwszTableCatalog	= NULL;
	m_pwszTableSchema	= NULL; 
	m_pwszTableName		= NULL;
	m_pSessionIUnknown	= NULL;
	m_pDSOIUnknown		= NULL;
	m_lCrtPos			= -1;

	m_pTable			= NULL;

	TESTC(NULL != pSessionIUnknown);
	pSessionIUnknown->AddRef();
	m_pSessionIUnknown = pSessionIUnknown;

	if (!s_fMetadataInitialized)
	{
		s_fMetadataInitialized = TRUE;

		hr = SchemaRowset.IsSchemaSupported(DBSCHEMA_PRIMARY_KEYS);
		s_fSchemaSupported = S_OK == hr;
		if (!s_fSchemaSupported)
		{
			odtLog << "DBSCHEMA_PRIMARY_KEYS is not supported!\n";
			goto CLEANUP;
		}

		TESTC_(SchemaRowset.GetRestrictionSupport(DBSCHEMA_PRIMARY_KEYS, &ulRestrictionSupport), S_OK);

		// set the restrictions
		s_fTableCatalogR	= ulRestrictionSupport & 1;
		s_fTableSchemaR		= ulRestrictionSupport & 2;
		s_fTableNameR		= ulRestrictionSupport & 4;
	}
	
	TESTC(VerifyInterface(m_pSessionIUnknown, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown, &m_pDSOIUnknown), S_OK);
	
	m_pTable = new CTable(m_pSessionIUnknown, L"NoModuleName");

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	return;
} //CPKRowset::CPKRowset
	


CPKRowset::~CPKRowset()
{
	delete m_pTable;
	SAFE_FREE(m_pwszPKName);
	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);
	SAFE_RELEASE(m_pSessionIUnknown);
	SAFE_RELEASE(m_pDSOIUnknown);
} //CPKRowset::~CPKRowset



HRESULT CPKRowset::GetPrimaryKeys(
	WCHAR	*pwszTableCatalogR,	// [in] constraint catalog restriction
	WCHAR	*pwszTableSchemaR,	// [in] constraint schema restriction
	WCHAR	*pwszTableNameR		// [in] constraint name restriction
)
{
	HRESULT			hr = E_FAIL;
	CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
	IRowset			*pIRowset = NULL;
	const ULONG		cMaxRestr = 3;
	VARIANT			rgvRestr[cMaxRestr];

	DBORDINAL		ulIndex;

	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantInit(&rgvRestr[ulIndex]);
	}

	TESTC(NULL != m_pSessionIUnknown);

	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);

	m_pwszTableCatalog	= wcsDuplicate(pwszTableCatalogR);
	m_pwszTableSchema	= wcsDuplicate(pwszTableSchemaR);
	m_pwszTableName		= wcsDuplicate(pwszTableNameR);

	if (m_pwszTableCatalog && s_fTableCatalogR)
	{
		rgvRestr[0].vt = VT_BSTR;
		V_BSTR(&rgvRestr[0]) = SysAllocString(m_pwszTableCatalog);
	}
	if (m_pwszTableSchema && s_fTableSchemaR)
	{
		rgvRestr[1].vt = VT_BSTR;
		V_BSTR(&rgvRestr[1]) = SysAllocString(m_pwszTableSchema);
	}
	if (m_pwszTableName && s_fTableNameR)
	{
		rgvRestr[2].vt = VT_BSTR;
		V_BSTR(&rgvRestr[2]) = SysAllocString(m_pwszTableName);
	}

	// get PRIMARY_KEYS Rowset
	TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_PRIMARY_KEYS, cMaxRestr, rgvRestr, 
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	m_Rowset.DropRowset();
	m_Rowset.SetTable(m_pTable, DELETETABLE_NO);

	TESTC_PROVIDER(S_OK == m_Rowset.CreateRowset(pIRowset, USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK));
	m_lCrtPos = -1;

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantClear(&rgvRestr[ulIndex]);
	}
	return hr;
} //CPKRowset::GetPrimaryKeys



DBCOUNTITEM CPKRowset::GetConstrNo()
{
	TBEGIN
	HROW			hRow;
	DBCOUNTITEM		cRows				= 0;
	HRESULT			hr					= E_FAIL;

	ULONG			ulTableCatalog		= 0;	// column ordinal
	WCHAR			*pwszTableCatalog	= NULL;
	ULONG			ulTableSchema		= 1;	// column ordinal
	WCHAR			*pwszTableSchema	= NULL;
	ULONG			ulTableName			= 2;	// column ordinal
	WCHAR			*pwszTableName		= NULL;

	TESTC_(hr = m_Rowset.RestartPosition(),S_OK);

	while(S_OK == hr)
	{
		//Get the next row
		TEST2C_(hr = m_Rowset.GetNextRows(&hRow), S_OK, DB_S_ENDOFROWSET);

		if(DB_S_ENDOFROWSET == hr)  
		{
			m_Rowset.ReleaseRows(hRow);
			break;
		}

		TESTC_(hr = m_Rowset.GetRowData(hRow, &m_Rowset.m_pData),S_OK);
	
		// get constraint info
		pwszTableCatalog	= (WCHAR*)m_Rowset.GetValue(ulTableCatalog);
		pwszTableSchema		= (WCHAR*)m_Rowset.GetValue(ulTableSchema);
		pwszTableName		= (WCHAR*)m_Rowset.GetValue(ulTableName);

		if (	CheckRestriction(s_fTableCatalogR,	m_pwszTableCatalog,	pwszTableCatalog)
			||	CheckRestriction(s_fTableSchemaR,	m_pwszTableSchema,	pwszTableSchema)
			||	CheckRestriction(s_fTableNameR,		m_pwszTableName,	pwszTableName)
			)
			cRows++;

		m_Rowset.ReleaseRows(hRow);
	}

	//Restore Position to the begining
	TESTC_(hr = m_Rowset.RestartPosition(),S_OK);

CLEANUP:
	return cRows;
} //CPKRowset::GetConstrNo



DBID *CPKRowset::operator[](DBORDINAL index)
{
	ULONG			ulTableCatalog		= 0;	// column ordinal
	WCHAR			*pwszTableCatalog	= NULL;
	ULONG			ulTableSchema		= 1;	// column ordinal
	WCHAR			*pwszTableSchema	= NULL;
	ULONG			ulTableName			= 2;	// column ordinal
	WCHAR			*pwszTableName		= NULL;

	HROW			hRow;
	HRESULT			hr;

	LONG			lIndex				= -1;

	DBID			*pColumnID			= NULL;

	ULONG			ulColumnName		= 3;	// column ordinal
	WCHAR			*pwszColumnName		= NULL;
	ULONG			ulColumnGUID		= 4;	// column ordinal
	GUID			*pguidColumnGUID	= NULL;
	ULONG			ulColumnPROPID		= 5;	// column ordinal
	ULONG			*pulColumnPROPID	= NULL;

	ULONG			ulPKName			= 7;
	WCHAR			*pwszPKName			= NULL;

	if ((LONG)index < m_lCrtPos)
	{
		TESTC_(hr = m_Rowset.RestartPosition(),S_OK);
		m_lCrtPos = -1;
	}

	lIndex = m_lCrtPos;

	for (; lIndex < (LONG)index; )
	{
		TEST2C_(hr = m_Rowset.GetNextRows(&hRow), S_OK, DB_S_ENDOFROWSET);

		if (DB_S_ENDOFROWSET == hr)
		{
			m_Rowset.ReleaseRows(hRow);	
			goto CLEANUP;
		}

		TESTC_(hr = m_Rowset.GetRowData(hRow, &m_Rowset.m_pData),S_OK);

		// get constraint info
		pwszTableCatalog	= (WCHAR*)m_Rowset.GetValue(ulTableCatalog);
		pwszTableSchema		= (WCHAR*)m_Rowset.GetValue(ulTableSchema);
		pwszTableName		= (WCHAR*)m_Rowset.GetValue(ulTableName);

		pwszPKName			= (WCHAR*)m_Rowset.GetValue(ulPKName);
		TESTC(NULL != pwszPKName);
		if (!m_pwszPKName)
			m_pwszPKName = wcsDuplicate(pwszPKName);
		else
			TESTC(0 == wcscmp(m_pwszPKName, pwszPKName));

		TESTC(NULL != pwszPKName);


		// this should be checked in an inferior layer, actually
		if (	!CheckRestriction(s_fTableCatalogR,	m_pwszTableCatalog,	pwszTableCatalog)
			||	!CheckRestriction(s_fTableSchemaR,	m_pwszTableSchema,	pwszTableSchema)
			||	!CheckRestriction(s_fTableNameR,	m_pwszTableName,	pwszTableName)
			)
			goto LOOP;

		lIndex++;

LOOP:
		m_Rowset.ReleaseRows(hRow);	
	}

	if (lIndex == (LONG)index)
	{
		SAFE_ALLOC(pColumnID, DBID, 1);

		pwszColumnName	= (WCHAR*)m_Rowset.GetValue(ulColumnName);
		pguidColumnGUID	= (GUID*)m_Rowset.GetValue(ulColumnGUID);
		pulColumnPROPID	= (ULONG*)m_Rowset.GetValue(ulColumnPROPID);

		pColumnID->eKind = DBKIND_NAME;
		pColumnID->uName.pwszName = wcsDuplicate(pwszColumnName);
		TESTC(NULL == pguidColumnGUID);
		TESTC(NULL == pulColumnPROPID);
	}

	m_lCrtPos = (LONG)lIndex;

CLEANUP:
	return pColumnID;
} //CPKRowset::operator[]



WCHAR *CPKRowset::GetPKName()
{
	DBID	*pColID = NULL;

	if (NULL == m_pwszPKName)
	{
		pColID = (*this)[0];
		TESTC(NULL != pColID);
		ReleaseDBID(pColID, TRUE);
	}

CLEANUP:
	return m_pwszPKName;
} //CPKRowset::GetPKName




//=============================================
CFKRowset::CFKRowset(IUnknown *pSessionIUnknown)
{
	IGetDataSource	*pIGetDataSource = NULL;
	CSchemaRowset	SchemaRowset(pSessionIUnknown);
	ULONG			ulRestrictionSupport;
	HRESULT			hr;

	m_pwszTableCatalog	= NULL;
	m_pwszTableSchema	= NULL; 
	m_pwszTableName		= NULL;
	
	m_pwszFKName		= NULL;

	m_pwszUpdateRule	= NULL;
	m_pwszDeleteRule	= NULL;

	m_pwszReferencedTableCatalog	= NULL;
	m_pwszReferencedTableSchema		= NULL;
	m_pwszReferencedTableName		= NULL;

	m_pSessionIUnknown	= NULL;
	m_pDSOIUnknown		= NULL;
	m_lCrtPos			= -1;

	m_pCrtReferencedColID	= NULL;
	m_pCrtFKColID			= NULL;

	m_pTable				= NULL;

	TESTC(NULL != pSessionIUnknown);
	pSessionIUnknown->AddRef();
	m_pSessionIUnknown = pSessionIUnknown;

	if (!s_fMetadataInitialized)
	{
		s_fMetadataInitialized = TRUE;

		hr = SchemaRowset.IsSchemaSupported(DBSCHEMA_FOREIGN_KEYS);
		s_fSchemaSupported = S_OK == hr;
		if (!s_fSchemaSupported)
		{
			odtLog << "DBSCHEMA_FOREIGN_KEYS is not supported!\n";
			goto CLEANUP;
		}
		
		TESTC_(SchemaRowset.GetRestrictionSupport(DBSCHEMA_FOREIGN_KEYS, &ulRestrictionSupport), S_OK);

		// set the restrictions
		s_fTableCatalogR	= ulRestrictionSupport & 8;
		s_fTableSchemaR		= ulRestrictionSupport & 16;
		s_fTableNameR		= ulRestrictionSupport & 32;
	}
	
	TESTC(VerifyInterface(m_pSessionIUnknown, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown, &m_pDSOIUnknown), S_OK);
	
	m_pTable = new CTable(m_pSessionIUnknown, L"NoModuleName");

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	return;
} //CFKRowset::CFKRowset
	


CFKRowset::~CFKRowset()
{
	delete m_pTable;

	SAFE_FREE(m_pwszUpdateRule);
	SAFE_FREE(m_pwszDeleteRule);

	SAFE_FREE(m_pwszFKName);

	SAFE_FREE(m_pwszReferencedTableCatalog);
	SAFE_FREE(m_pwszReferencedTableSchema);
	SAFE_FREE(m_pwszReferencedTableName);

	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);

	ReleaseDBID(m_pCrtReferencedColID);
	ReleaseDBID(m_pCrtFKColID);
	m_pCrtReferencedColID	= NULL;
	m_pCrtFKColID			= NULL;

	SAFE_RELEASE(m_pSessionIUnknown);
	SAFE_RELEASE(m_pDSOIUnknown);
} //CFKRowset::~CFKRowset



HRESULT CFKRowset::GetForeignKeys(
	WCHAR	*pwszTableCatalogR,	// [in] constraint catalog restriction
	WCHAR	*pwszTableSchemaR,	// [in] constraint schema restriction
	WCHAR	*pwszTableNameR,	// [in] constraint name restriction
	WCHAR	*pwszFKNameR		// [in] foreign key constraint
)
{
	HRESULT			hr = E_FAIL;
	CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
	IRowset			*pIRowset = NULL;
	const ULONG		cMaxRestr = 6;
	VARIANT			rgvRestr[cMaxRestr];

	DBORDINAL		ulIndex;

	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantInit(&rgvRestr[ulIndex]);
	}

	TESTC(NULL != m_pSessionIUnknown);

	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);

	SAFE_FREE(m_pwszFKName);

	m_pwszTableCatalog	= wcsDuplicate(pwszTableCatalogR);
	m_pwszTableSchema	= wcsDuplicate(pwszTableSchemaR);
	m_pwszTableName		= wcsDuplicate(pwszTableNameR);

	m_pwszFKName		= wcsDuplicate(pwszFKNameR);

	if (m_pwszTableCatalog && s_fTableCatalogR)
	{
		rgvRestr[3].vt = VT_BSTR;
		V_BSTR(&rgvRestr[3]) = SysAllocString(m_pwszTableCatalog);
	}
	if (m_pwszTableSchema && s_fTableSchemaR)
	{
		rgvRestr[4].vt = VT_BSTR;
		V_BSTR(&rgvRestr[4]) = SysAllocString(m_pwszTableSchema);
	}
	if (m_pwszTableName && s_fTableNameR)
	{
		rgvRestr[5].vt = VT_BSTR;
		V_BSTR(&rgvRestr[5]) = SysAllocString(m_pwszTableName);
	}

	// get FOREIGN_KEYS Rowset
	TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_FOREIGN_KEYS, cMaxRestr, rgvRestr, 
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	m_Rowset.DropRowset();
	m_Rowset.SetTable(m_pTable, DELETETABLE_NO);

	TESTC_PROVIDER(S_OK == m_Rowset.CreateRowset(pIRowset, USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK));
	m_lCrtPos = -1;

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantClear(&rgvRestr[ulIndex]);
	}
	return hr;
} //CFKRowset::GetForeignKeys



DBCOUNTITEM CFKRowset::GetConstrNo()
{
	TBEGIN
	DBORDINAL	cRows = 0;

	cRows = (ULONG)(m_lCrtPos + 1);

	for (; ; cRows++)
	{
		if (!MoveToRow(cRows))
			break;
	}
	
	return cRows;
} //CPKRowset::GetConstrNo



BOOL CFKRowset::MoveToRow(DBORDINAL index)
{
	ULONG			ulRefTableCatalog			= 0;	// column ordinal
	WCHAR			*pwszRefTableCatalog		= NULL;
	ULONG			ulRefTableSchema			= 1;	// column ordinal
	WCHAR			*pwszRefTableSchema			= NULL;
	ULONG			ulRefTableName				= 2;	// column ordinal
	WCHAR			*pwszRefTableName			= NULL;

	ULONG			ulTableCatalog				= 6;	// column ordinal
	WCHAR			*pwszTableCatalog			= NULL;
	ULONG			ulTableSchema				= 7;	// column ordinal
	WCHAR			*pwszTableSchema			= NULL;
	ULONG			ulTableName					= 8;	// column ordinal
	WCHAR			*pwszTableName				= NULL;

	HROW			hRow;
	HRESULT			hr;

	LONG			lIndex						= -1;

	ULONG			ulFKColumnName				= 9;	// column ordinal
	WCHAR			*pwszFKColumnName			= NULL;
	ULONG			ulFKColumnGUID				= 10;	// column ordinal
	GUID			*pguidFKColumnGUID			= NULL;
	ULONG			ulFKColumnPROPID			= 11;	// column ordinal
	ULONG			*pulFKColumnPROPID			= NULL;

	ULONG			ulReferencedColumnName		= 3;	// column ordinal
	WCHAR			*pwszReferencedColumnName	= NULL;
	ULONG			ulReferencedColumnGUID		= 4;	// column ordinal
	GUID			*pguidReferencedColumnGUID	= NULL;
	ULONG			ulReferencedColumnPROPID	= 5;	// column ordinal
	ULONG			*pulReferencedColumnPROPID	= NULL;

	ULONG			ulUpdateRule				= 13;
	WCHAR			*pwszUpdateRule				= NULL;
	ULONG			ulDeleteRule				= 14;
	WCHAR			*pwszDeleteRule				= NULL;

	ULONG			ulFKName					= 16;
	WCHAR			*pwszFKName					= NULL;

	if ((LONG)index < m_lCrtPos)
	{
		TESTC_(hr = m_Rowset.RestartPosition(),S_OK);
		m_lCrtPos = -1;
	}

	if ((LONG)index == m_lCrtPos)
		return TRUE;

	ReleaseDBID(m_pCrtReferencedColID, TRUE);
	ReleaseDBID(m_pCrtFKColID, TRUE);
	m_pCrtReferencedColID	= NULL;
	m_pCrtFKColID			= NULL;

	lIndex = m_lCrtPos;

	for (; lIndex < (LONG)index; )
	{
		TEST2C_(hr = m_Rowset.GetNextRows(&hRow), S_OK, DB_S_ENDOFROWSET);

		if (DB_S_ENDOFROWSET == hr)
		{
			m_Rowset.ReleaseRows(hRow);	
			return FALSE;
		}

		TESTC_(hr = m_Rowset.GetRowData(hRow, &m_Rowset.m_pData),S_OK);

		// get constraint info
		pwszTableCatalog	= (WCHAR*)m_Rowset.GetValue(ulTableCatalog);
		pwszTableSchema		= (WCHAR*)m_Rowset.GetValue(ulTableSchema);
		pwszTableName		= (WCHAR*)m_Rowset.GetValue(ulTableName);

		pwszRefTableCatalog	= (WCHAR*)m_Rowset.GetValue(ulRefTableCatalog);
		pwszRefTableSchema	= (WCHAR*)m_Rowset.GetValue(ulRefTableSchema);
		pwszRefTableName	= (WCHAR*)m_Rowset.GetValue(ulRefTableName);

		if (!m_pwszReferencedTableCatalog)
		{
			TESTC((-1 == lIndex) || !pwszRefTableCatalog);
			m_pwszReferencedTableCatalog = wcsDuplicate(pwszRefTableCatalog);
		}
		//else
		//	TESTC(pwszRefTableCatalog && (0 == wcscmp(m_pwszReferencedTableCatalog, pwszRefTableCatalog)));

		if (!m_pwszReferencedTableSchema)
		{
			TESTC((-1 == lIndex) || !pwszRefTableSchema);
			m_pwszReferencedTableSchema = wcsDuplicate(pwszRefTableSchema);
		}
		//else
		//	TESTC(pwszRefTableSchema && (0 == wcscmp(m_pwszReferencedTableSchema, pwszRefTableSchema)));

		if (!m_pwszReferencedTableName)
		{
			TESTC((-1 == lIndex) || !pwszRefTableName);
			m_pwszReferencedTableName = wcsDuplicate(pwszRefTableName);
		}
		else
		{
			//TESTC(pwszRefTableName && (0 == wcscmp(m_pwszReferencedTableName, pwszRefTableName)));
		}

		pwszFKName			= (WCHAR*)m_Rowset.GetValue(ulFKName);
		TESTC(NULL != pwszFKName);
		if (!m_pwszFKName)
			m_pwszFKName = wcsDuplicate(pwszFKName);
		else if (0 != wcscmp(m_pwszFKName, pwszFKName))
			goto LOOP;

		pwszUpdateRule		= (WCHAR*)m_Rowset.GetValue(ulUpdateRule);
		TESTC(NULL != pwszUpdateRule);
		if (!m_pwszUpdateRule)
			m_pwszUpdateRule = wcsDuplicate(pwszUpdateRule);
		else if (0 != wcscmp(m_pwszUpdateRule, pwszUpdateRule))
			goto LOOP;

		pwszDeleteRule		= (WCHAR*)m_Rowset.GetValue(ulDeleteRule);
		TESTC(NULL != pwszDeleteRule);
		if (!m_pwszDeleteRule)
			m_pwszDeleteRule = wcsDuplicate(pwszDeleteRule);
		else if (0 != wcscmp(m_pwszDeleteRule, pwszDeleteRule))
			goto LOOP;

		// this should be checked in an inferior layer, actually
		if (	!CheckRestriction(s_fTableCatalogR,	m_pwszTableCatalog,	pwszTableCatalog)
			||	!CheckRestriction(s_fTableSchemaR,	m_pwszTableSchema,	pwszTableSchema)
			||	!CheckRestriction(s_fTableNameR,	m_pwszTableName,	pwszTableName)
			)
			goto LOOP;

		lIndex++;

LOOP:
		m_Rowset.ReleaseRows(hRow);	
	}

	if (lIndex == (LONG)index)
	{
		SAFE_ALLOC(m_pCrtReferencedColID, DBID, 1);

		pwszReferencedColumnName	= (WCHAR*)m_Rowset.GetValue(ulReferencedColumnName);
		pguidReferencedColumnGUID	= (GUID*)m_Rowset.GetValue(ulReferencedColumnGUID);
		pulReferencedColumnPROPID	= (ULONG*)m_Rowset.GetValue(ulReferencedColumnPROPID);

		m_pCrtReferencedColID->eKind = DBKIND_NAME;
		m_pCrtReferencedColID->uName.pwszName = wcsDuplicate(pwszReferencedColumnName);
		TESTC(NULL == pguidReferencedColumnGUID);
		TESTC(NULL == pulReferencedColumnPROPID);

		SAFE_ALLOC(m_pCrtFKColID, DBID, 1);

		pwszFKColumnName	= (WCHAR*)m_Rowset.GetValue(ulFKColumnName);
		pguidFKColumnGUID	= (GUID*)m_Rowset.GetValue(ulFKColumnGUID);
		pulFKColumnPROPID	= (ULONG*)m_Rowset.GetValue(ulFKColumnPROPID);

		m_pCrtFKColID->eKind = DBKIND_NAME;
		m_pCrtFKColID->uName.pwszName = wcsDuplicate(pwszFKColumnName);
		TESTC(NULL == pguidFKColumnGUID);
		TESTC(NULL == pulFKColumnPROPID);
	}

	m_lCrtPos = (LONG)lIndex;

CLEANUP:
	return TRUE;
} //CFKRowset::MoveToRow



WCHAR *CFKRowset::GetFKName()
{
	if (!m_pwszFKName)
		MoveToRow(0);

	return m_pwszFKName;
} //CFKRowset::GetFKName



WCHAR *CFKRowset::GetReferencedTableName()
{
	if (!m_pwszReferencedTableName)
		MoveToRow(0);

	return m_pwszReferencedTableName;
} //CFKRowset::GetReferencedTableName




//=============================================
CKeyColumnUsageRowset::CKeyColumnUsageRowset(IUnknown *pSessionIUnknown)
{
	IGetDataSource	*pIGetDataSource = NULL;
	CSchemaRowset	SchemaRowset(pSessionIUnknown);
	ULONG			ulRestrictionSupport;
	HRESULT			hr;

	m_pwszTableCatalog	= NULL;
	m_pwszTableSchema	= NULL; 
	m_pwszTableName		= NULL;
	
	m_pwszConsCatalog	= NULL;
	m_pwszConsSchema	= NULL; 
	m_pwszConsName		= NULL;
	
	m_pSessionIUnknown	= NULL;
	m_pDSOIUnknown		= NULL;
	m_lCrtPos			= -1;

	m_pTable			= NULL;

	m_pCrtColID			= NULL;

	TESTC(NULL != pSessionIUnknown);
	pSessionIUnknown->AddRef();
	m_pSessionIUnknown = pSessionIUnknown;

	if (!s_fMetadataInitialized)
	{
		s_fMetadataInitialized = TRUE;
		
		hr = SchemaRowset.IsSchemaSupported(DBSCHEMA_KEY_COLUMN_USAGE);
		s_fSchemaSupported = S_OK == hr;
		if (!s_fSchemaSupported)
		{
			odtLog << "DBSCHEMA_KEY_COLUMN_USAGE is not supported!\n";
			goto CLEANUP;
		}

		TESTC_(SchemaRowset.GetRestrictionSupport(DBSCHEMA_KEY_COLUMN_USAGE , &ulRestrictionSupport), S_OK);

		// set the restrictions
		s_fConsCatalogR		= ulRestrictionSupport & 1;
		s_fConsSchemaR		= ulRestrictionSupport & 2;
		s_fConsNameR		= ulRestrictionSupport & 4;

		s_fTableCatalogR	= ulRestrictionSupport & 8;
		s_fTableSchemaR		= ulRestrictionSupport & 16;
		s_fTableNameR		= ulRestrictionSupport & 32;
	}
	
	TESTC(VerifyInterface(m_pSessionIUnknown, IID_IGetDataSource, 
		SESSION_INTERFACE, (IUnknown**)&pIGetDataSource));

	TESTC_(pIGetDataSource->GetDataSource(IID_IUnknown, &m_pDSOIUnknown), S_OK);
	
	m_pTable = new CTable(m_pSessionIUnknown, L"NoModuleName");

CLEANUP:
	SAFE_RELEASE(pIGetDataSource);
	return;
} //CKeyColumnUsageRowset::CKeyColumnUsageRowset
	


CKeyColumnUsageRowset::~CKeyColumnUsageRowset()
{
	SAFE_DELETE(m_pTable);

	SAFE_FREE(m_pwszConsCatalog);
	SAFE_FREE(m_pwszConsSchema);
	SAFE_FREE(m_pwszConsName);

	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);

	ReleaseDBID(m_pCrtColID);
	m_pCrtColID = NULL;

	SAFE_RELEASE(m_pSessionIUnknown);
	SAFE_RELEASE(m_pDSOIUnknown);
} //CKeyColumnUsageRowset::~CKeyColumnUsageRowset



HRESULT CKeyColumnUsageRowset::GetKeyColumnUsage(
	WCHAR	*pwszTableCatalogR,	// [in] constraint catalog restriction
	WCHAR	*pwszTableSchemaR,	// [in] constraint schema restriction
	WCHAR	*pwszTableNameR,	// [in] constraint name restriction
	WCHAR	*pwszConsCatalogR,	// [in] constraint catalog restriction
	WCHAR	*pwszConsSchemaR,	// [in] constraint schema restriction
	WCHAR	*pwszConsNameR		// [in] constraint name restriction
)
{
	HRESULT			hr = E_FAIL;
	CSchemaRowset	SchemaRowset(m_pSessionIUnknown);
	IRowset			*pIRowset = NULL;
	const ULONG		cMaxRestr = 7;
	VARIANT			rgvRestr[cMaxRestr];

	DBORDINAL		ulIndex;

	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantInit(&rgvRestr[ulIndex]);
	}

	TESTC(NULL != m_pSessionIUnknown);

	SAFE_FREE(m_pwszConsCatalog);
	SAFE_FREE(m_pwszConsSchema);
	SAFE_FREE(m_pwszConsName);

	SAFE_FREE(m_pwszTableCatalog);
	SAFE_FREE(m_pwszTableSchema);
	SAFE_FREE(m_pwszTableName);

	m_pwszConsCatalog	= wcsDuplicate(pwszConsCatalogR);
	m_pwszConsSchema	= wcsDuplicate(pwszConsSchemaR);
	m_pwszConsName		= wcsDuplicate(pwszConsNameR);

	m_pwszTableCatalog	= wcsDuplicate(pwszTableCatalogR);
	m_pwszTableSchema	= wcsDuplicate(pwszTableSchemaR);
	m_pwszTableName		= wcsDuplicate(pwszTableNameR);

	// constraint restrictions
	if (m_pwszConsCatalog && s_fConsCatalogR)
	{
		rgvRestr[0].vt = VT_BSTR;
		V_BSTR(&rgvRestr[0]) = SysAllocString(m_pwszConsCatalog);
	}
	if (m_pwszConsSchema && s_fConsSchemaR)
	{
		rgvRestr[1].vt = VT_BSTR;
		V_BSTR(&rgvRestr[1]) = SysAllocString(m_pwszConsSchema);
	}
	if (m_pwszConsName && s_fConsNameR)
	{
		rgvRestr[2].vt = VT_BSTR;
		V_BSTR(&rgvRestr[2]) = SysAllocString(m_pwszConsName);
	}

	// table restrictions
	if (m_pwszTableCatalog && s_fTableCatalogR)
	{
		rgvRestr[3].vt = VT_BSTR;
		V_BSTR(&rgvRestr[3]) = SysAllocString(m_pwszTableCatalog);
	}
	if (m_pwszTableSchema && s_fTableSchemaR)
	{
		rgvRestr[4].vt = VT_BSTR;
		V_BSTR(&rgvRestr[4]) = SysAllocString(m_pwszTableSchema);
	}
	if (m_pwszTableName && s_fTableNameR)
	{
		rgvRestr[5].vt = VT_BSTR;
		V_BSTR(&rgvRestr[5]) = SysAllocString(m_pwszTableName);
	}

	// get KEY_COLUMN_USAGE  Rowset
	TESTC_(SchemaRowset.GetRowset(NULL, DBSCHEMA_KEY_COLUMN_USAGE , cMaxRestr, rgvRestr, 
		IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);

	m_Rowset.DropRowset();
	m_Rowset.SetTable(m_pTable, DELETETABLE_NO);

	TESTC_PROVIDER(S_OK == m_Rowset.CreateRowset(pIRowset, USE_SUPPORTED_SELECT_ALLFROMTBL, IID_IRowset, NULL, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_EXCEPTBOOKMARK));
	m_lCrtPos = -1;

	hr = S_OK;

CLEANUP:
	SAFE_RELEASE(pIRowset);
	for (ulIndex=0; ulIndex< cMaxRestr; ulIndex++)
	{
		VariantClear(&rgvRestr[ulIndex]);
	}
	return hr;
} //CKeyColumnUsageRowset::GetKeyColumnUsage



DBCOUNTITEM CKeyColumnUsageRowset::GetConstrNo()
{
	TBEGIN
	DBORDINAL	cRows = 0;

	cRows = (ULONG)(m_lCrtPos + 1);

	for (; ; cRows++)
	{
		if (!MoveToRow(cRows))
			break;
	}
	
	return cRows;
} //CKeyColumnUsageRowset::GetConstrNo



BOOL CKeyColumnUsageRowset::MoveToRow(DBORDINAL index)
{
	ULONG			ulConsCatalog		= 0;	// column ordinal
	WCHAR			*pwszConsCatalog	= NULL;
	ULONG			ulConsSchema		= 1;	// column ordinal
	WCHAR			*pwszConsSchema		= NULL;
	ULONG			ulConsName			= 2;	// column ordinal
	WCHAR			*pwszConsName		= NULL;

	ULONG			ulTableCatalog		= 3;	// column ordinal
	WCHAR			*pwszTableCatalog	= NULL;
	ULONG			ulTableSchema		= 4;	// column ordinal
	WCHAR			*pwszTableSchema	= NULL;
	ULONG			ulTableName			= 5;	// column ordinal
	WCHAR			*pwszTableName		= NULL;

	ULONG			ulColumnName		= 6;	// column ordinal
	WCHAR			*pwszColumnName		= NULL;
	ULONG			ulColumnGUID		= 7;	// column ordinal
	GUID			*pguidColumnGUID	= NULL;
	ULONG			ulColumnPROPID		= 8;	// column ordinal
	ULONG			*pulColumnPROPID	= NULL;

	HROW			hRow;
	HRESULT			hr;

	LONG			lIndex				= -1;

	if ((LONG)index < m_lCrtPos)
	{
		TESTC_(hr = m_Rowset.RestartPosition(),S_OK);
		m_lCrtPos = -1;
	}

	if ((LONG)index == m_lCrtPos)
		return TRUE;

	ReleaseDBID(m_pCrtColID, TRUE);
	m_pCrtColID = NULL;

	lIndex = m_lCrtPos;

	for (; lIndex < (LONG)index; )
	{
		TEST2C_(hr = m_Rowset.GetNextRows(&hRow), S_OK, DB_S_ENDOFROWSET);

		if (DB_S_ENDOFROWSET == hr)
		{
			m_Rowset.ReleaseRows(hRow);	
			return FALSE;
		}

		TESTC_(hr = m_Rowset.GetRowData(hRow, &m_Rowset.m_pData),S_OK);

		// get constraint info
		pwszConsCatalog		= (WCHAR*)m_Rowset.GetValue(ulConsCatalog);
		pwszConsSchema		= (WCHAR*)m_Rowset.GetValue(ulConsSchema);
		pwszConsName		= (WCHAR*)m_Rowset.GetValue(ulConsName);

		pwszTableCatalog	= (WCHAR*)m_Rowset.GetValue(ulTableCatalog);
		pwszTableSchema		= (WCHAR*)m_Rowset.GetValue(ulTableSchema);
		pwszTableName		= (WCHAR*)m_Rowset.GetValue(ulTableName);

		// this should be checked in an inferior layer, actually
		if (	!CheckRestriction(s_fConsCatalogR,	m_pwszConsCatalog,	pwszConsCatalog)
			||	!CheckRestriction(s_fConsSchemaR,	m_pwszConsSchema,	pwszConsSchema)
			||	!CheckRestriction(s_fConsNameR,		m_pwszConsName,		pwszConsName)
			||	!CheckRestriction(s_fTableCatalogR,	m_pwszTableCatalog,	pwszTableCatalog)
			||	!CheckRestriction(s_fTableSchemaR,	m_pwszTableSchema,	pwszTableSchema)
			||	!CheckRestriction(s_fTableNameR,	m_pwszTableName,	pwszTableName)
			)
			goto LOOP;

		lIndex++;

LOOP:
		m_Rowset.ReleaseRows(hRow);	
	}

	if (lIndex == (LONG)index)
	{
		SAFE_ALLOC(m_pCrtColID, DBID, 1);

		pwszColumnName	= (WCHAR*)m_Rowset.GetValue(ulColumnName);
		pguidColumnGUID	= (GUID*)m_Rowset.GetValue(ulColumnGUID);
		pulColumnPROPID	= (ULONG*)m_Rowset.GetValue(ulColumnPROPID);

		m_pCrtColID->eKind = DBKIND_NAME;
		m_pCrtColID->uName.pwszName = wcsDuplicate(pwszColumnName);
		TESTC(NULL == pguidColumnGUID);
		TESTC(NULL == pulColumnPROPID);
	}

	m_lCrtPos = (LONG)lIndex;

CLEANUP:
	return TRUE;
} //CKeyColumnUsageRowset::MoveToRow




