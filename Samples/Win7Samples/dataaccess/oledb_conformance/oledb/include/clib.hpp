// clib.hpp
#include "Extralib.h"

typedef struct {
	GUID	guidSchema;
	ULONG	cRestrictions;
	ULONG	*rgRestrictions;
} SSchemaEntry;

// this class contains info related to various schema constraints
// this should be updated based on the spec
// there is no programmatical meaning to get this info, aside 
// using this class
class CSchemaInfo{
public:

	// number of schemas that have info
	ULONG	m_cSchemas;

	SSchemaEntry	*m_rgSchemaEntry;

	CSchemaInfo();
	~CSchemaInfo() {SAFE_FREE(m_rgSchemaEntry);}

	LONG			GetSchemaIndex(GUID guidSchema);
	SSchemaEntry	*operator[](GUID guidSchema);
}; //CSchemaInfo



//==================================================================
//
//			Classes related to schema rowsets
//
//==================================================================


// this class should be used to access data in
// schema rowsets as conveninent as possible and
// still allow some basic testing
class CSchemaRowset
{
private:
	IDBSchemaRowset	*m_pIDBSR;
	
	// @cmember returns the index of the given schema in the array or -1 if it doesn't exist
	LONG				GetSchemaIndex(GUID guidSchema);
	CSchemaInfo			m_SchemaInfo;

public:
	ULONG			m_cSchemas;
	// for speed, this could be turned into a hash table
	GUID			*m_rgSchemas;
	ULONG			*m_rgRestrictionSupport;

						CSchemaRowset(
							IUnknown	*pIUnknown,	// pointer to the session object
							BOOL		fQI = TRUE	// whether the interface should 
													// be QI-ed for, or it is already
													// an IDBSchemaRowset interface
						);

						~CSchemaRowset();

						operator IDBSchemaRowset*() {
								return m_pIDBSR;}

	void				FreeCache() {
							SAFE_FREE(m_rgSchemas);
							SAFE_FREE(m_rgRestrictionSupport);
							m_cSchemas = 0;
						};

	// @cmember This method calls IDBSchemaRowset::GetSchemas and
	// caches the results
	HRESULT				GetSchemas();

	// @cmember Tells whether a certain schema is supported or not
	HRESULT				IsSchemaSupported(GUID guidSchema);

	// @cmember Restriction support for a certain schema
	HRESULT				GetRestrictionSupport(GUID guidSchema, ULONG *pRestSupp);

	// @ cmember Calls GetRowset on the IDBSchemaRowset interface and
	// does some basic checking
	HRESULT				GetRowset(
						   IUnknown *      pUnkOuter,
						   REFGUID         rguidSchema,
						   ULONG           cRestrictions,
						   const VARIANT   rgRestrictions[],
						   REFIID          riid,
						   ULONG           cPropertySets,
						   DBPROPSET       rgPropertySets[],
						   IUnknown **     ppRowset							
						   );

}; //CSchemaRowset





class CComfRowset : public CRowset{
public:
	CComfRowset(WCHAR* pwszTestCaseName = INVALID(WCHAR*)): CRowset(pwszTestCaseName) {;}
    virtual ~CComfRowset() {CRowset::~CRowset();}

	DBSTATUS *GetStatus(DBCOUNTITEM cBinding)
	{
		DBBINDING	*pBinding = &m_rgBinding[cBinding];
		
		if (STATUS_IS_BOUND(*pBinding))
			return (DBSTATUS*)((BYTE*)m_pData + pBinding->obStatus);
		else
			return NULL;
	}

	LPVOID *GetValue(DBCOUNTITEM cBinding)
	{
		DBBINDING	*pBinding = &m_rgBinding[cBinding];
		DBSTATUS	*pStatus = GetStatus(cBinding);

		if (VALUE_IS_BOUND(*pBinding) && DBSTATUS_S_OK == *pStatus)
			return (LPVOID*)&VALUE_BINDING(m_rgBinding[cBinding], m_pData);
		else
			return NULL;
	}

	DBBYTEOFFSET *GetLength(DBCOUNTITEM cBinding)
	{
		DBBINDING	*pBinding = &m_rgBinding[cBinding];
		DBSTATUS	*pStatus = GetStatus(cBinding);

		if (LENGTH_IS_BOUND(*pBinding) && DBSTATUS_S_OK == *pStatus)
			return (DBBYTEOFFSET*)((BYTE*)m_pData+pBinding->obLength);
		else
			return NULL;
	} 
}; //CComfRowset




// this class gets the names of the catalogs on a server
class CCatalogs
{
protected:
	DBORDINAL	m_cCatalogs;
	WCHAR		**m_rgCatalogs;

public:
	IUnknown		*m_pSessionIUnknown;
	IDBProperties	*m_pIDBProperties;

	CCatalogs(IUnknown *pSessionIUnknown);
	~CCatalogs();

	WCHAR		*operator[](DBORDINAL index);
	DBORDINAL	cCatalogs() {
					return m_cCatalogs;
	}
	WCHAR		**rgCatalogs(){
					return m_rgCatalogs;
	}

	BOOL		GetCurrentCatalog(WCHAR **ppwszCatalogName);
	HRESULT		SetCurrentCatalog(WCHAR *pwszCatalogName);
	HRESULT		ChangeCurrentCatalog(WCHAR **ppwszCatalogName = NULL);

	HRESULT		DoesCatalogExist(WCHAR *pwszCatalogName) {
					// checks whether a given catalog exists in the data source
					if (!pwszCatalogName)
						return E_INVALIDARG;

					if (!m_rgCatalogs)
					{
						ASSERT(0 == m_cCatalogs);
						return E_FAIL;
					}

					for (ULONG index=0; index < m_cCatalogs; index++)
					{
						if (0 == wcscmp(pwszCatalogName, m_rgCatalogs[index]))
							return S_OK;
					}

					return E_FAIL;
	}
}; // CCatalogs



class CAccessibleCatalogs: public CCatalogs
{
	public:
		CAccessibleCatalogs(IUnknown *pSessionIUnknown);
		~CAccessibleCatalogs() {}
}; //CAccessibleCatalogs



// this class grabs data related to all the constraints of a table
class CConstraints
{
private:
	DBCOUNTITEM			m_cConstraints;
	DBCONSTRAINTDESC	*m_rgConstraints;
	// cache for criteria
	WCHAR				*m_pwszTableCatalog;
	WCHAR				*m_pwszTableSchema;
	WCHAR				*m_pwszTableName;
	WCHAR				*m_pwszConsCatalog;
	WCHAR				*m_pwszConsSchema;
	WCHAR				*m_pwszConsName;
	IUnknown			*m_pSessionIUnknown;
	IUnknown			*m_pDSOIUnknown;

	// store all the constraint names you have already given away
	// assumptionis made that after they are no longer in use, they
	// are returned to the available name space
	DBORDINAL			m_cMaxConsNames;
	DBORDINAL			m_cConsNames;
	WCHAR				**m_rgConsNames;

	LONG				GetConsNameIndex(WCHAR *pwszConsName);

	HRESULT				ParseQualTableName(WCHAR *pwszQualTableName, 
							WCHAR **pwszTableCatalog,
							WCHAR **pwszTableSchema,
							WCHAR **pwszTableName);

public:

	CConstraints(IUnknown *pSessionIUnknown);
	
	~CConstraints(); 

	HRESULT				GetConstraints(
		WCHAR	*pwszTableCatalog,	// [in] table catalog restriction
		WCHAR	*pwszTableSchema,	// [in] table schema restriction
		WCHAR	*pwszTableName,		// [in] table name restriction (better not be NULL!)
		WCHAR	*pwszConsCatalog,	// [in] constraint catalog restriction
		WCHAR	*pwszConsSchema,	// [in] constraint schema restriction
		WCHAR	*pwszConsName		// [in] constraint name restriction
	);

	DBCOUNTITEM			GetConstrNo(){
								return m_cConstraints;}

	DBCONSTRAINTDESC	*GetConstraints(){
								return m_rgConstraints;}

	DBCONSTRAINTDESC	*operator[](DBID *pConstraintID);

	HRESULT				CheckIncludedConstraints(DBORDINAL cConsDesc, DBCONSTRAINTDESC *rgConsDesc);

	// @cmember This method checks whether the all the previously existent constraints still exist
	// and whether the constraints in the rgConsDesc array were added
	HRESULT				CheckAddConstraints(DBORDINAL cConsDesc, DBCONSTRAINTDESC *rgConsDesc);

	// @cmember Check whether a certain constraint was dropped from the table
	HRESULT				CheckDropConstraint(DBCONSTRAINTDESC *pConsDesc);

	// checked whether the constraint array is the same as last time
	HRESULT				AreConstraintsPreserved();

	// @cmember Check whether a given constraint exists
	HRESULT				DoesConstraintExist(WCHAR *pwszConsName, BOOL &fExist);

	// @cmember Build a name for a constraint
	HRESULT				MakeConstraintName(WCHAR **ppwszConsName);

	// @cmember Return  name that is already been used for constraints
	HRESULT				ReturnConstraintName(WCHAR *pwszConsName);

	// @cmember Book constraint name
	HRESULT				BookConstraintName(WCHAR *pwszConsName);
}; //CConstraints






// this class grabs data related to all the check constraints of a table
class CCheckConsRowset
{
private:
	WCHAR				*m_pwszConsCatalog;
	WCHAR				*m_pwszConsSchema;
	WCHAR				*m_pwszConsName;

	IUnknown			*m_pSessionIUnknown;
	IUnknown			*m_pDSOIUnknown;

	LONG				m_lCrtPos;

	CTable				*m_pTable;
	CComfRowset			m_Rowset;

public:

	static BOOL			s_fMetadataInitialized;
	static BOOL			s_fSchemaSupported;
	static BOOL			s_fConsCatalogR;
	static BOOL			s_fConsSchemaR;
	static BOOL			s_fConsNameR;

	static BOOL			IsSchemaSupported() {
							ASSERT(s_fMetadataInitialized);
							return s_fSchemaSupported;
	}

	CCheckConsRowset(IUnknown *pSessionIUnknown);
	
	~CCheckConsRowset(); 

	HRESULT				GetCheckConstraints(
		WCHAR	*pwszConsCatalog,	// [in] constraint catalog restriction
		WCHAR	*pwszConsSchema,	// [in] constraint schema restriction
		WCHAR	*pwszConsName		// [in] constraint name restriction
	);

	DBCOUNTITEM			GetConstrNo();

	WCHAR				*operator[](DBORDINAL index);
}; //CCheckConsRowset






// this class grabs data related to the PK constraint of a table
class CPKRowset
{
private:
	WCHAR				*m_pwszTableCatalog;
	WCHAR				*m_pwszTableSchema;
	WCHAR				*m_pwszTableName;

	WCHAR				*m_pwszPKName;

	IUnknown			*m_pSessionIUnknown;
	IUnknown			*m_pDSOIUnknown;

	LONG				m_lCrtPos;

	CTable				*m_pTable;
	CComfRowset			m_Rowset;

public:

	static BOOL			s_fMetadataInitialized;
	static BOOL			s_fSchemaSupported;
	static BOOL			s_fTableCatalogR;
	static BOOL			s_fTableSchemaR;
	static BOOL			s_fTableNameR;

	static BOOL			IsSchemaSupported() {
							ASSERT(s_fMetadataInitialized);
							return s_fSchemaSupported;
	}

	CPKRowset(IUnknown *pSessionIUnknown);
	
	~CPKRowset(); 

	HRESULT				GetPrimaryKeys(
		WCHAR	*pwszTableCatalog,	// [in] constraint catalog restriction
		WCHAR	*pwszTableSchema,	// [in] constraint schema restriction
		WCHAR	*pwszTableName		// [in] constraint name restriction
	);

	DBCOUNTITEM			GetConstrNo();

	DBID				*operator[](DBORDINAL index);

	WCHAR				*GetPKName();
}; //CPKRowset






// this class grabs data related to all the FK constraints of a table 
class CFKRowset
{
private:
	WCHAR				*m_pwszTableCatalog;
	WCHAR				*m_pwszTableSchema;
	WCHAR				*m_pwszTableName;

	WCHAR				*m_pwszFKName;

	WCHAR				*m_pwszReferencedTableCatalog;
	WCHAR				*m_pwszReferencedTableSchema;
	WCHAR				*m_pwszReferencedTableName;

	IUnknown			*m_pSessionIUnknown;
	IUnknown			*m_pDSOIUnknown;

	LONG				m_lCrtPos;

	CTable				*m_pTable;
	CComfRowset			m_Rowset;

	DBID				*m_pCrtReferencedColID;
	DBID				*m_pCrtFKColID;

	WCHAR				*m_pwszUpdateRule;
	WCHAR				*m_pwszDeleteRule;

	static DBUPDELRULE	StringToUpDelRule(WCHAR *pwszRule){
							ASSERT(pwszRule);
							if (0 == wcscmp(L"CASCADE", pwszRule))
								return DBUPDELRULE_CASCADE;
							else if (0 == wcscmp(L"SET NULL", pwszRule))
								return DBUPDELRULE_SETNULL;
							else if (0 == wcscmp(L"SET DEFAULT", pwszRule))
								return DBUPDELRULE_SETDEFAULT;
							else ASSERT(0 == wcscmp(L"NO ACTION", pwszRule));
							return DBUPDELRULE_NOACTION;
	}

public:

	static BOOL			s_fMetadataInitialized;
	static BOOL			s_fSchemaSupported;
	static BOOL			s_fTableCatalogR;
	static BOOL			s_fTableSchemaR;
	static BOOL			s_fTableNameR;

	static BOOL			IsSchemaSupported() {
							ASSERT(s_fMetadataInitialized);
							return s_fSchemaSupported;
	}

	CFKRowset(IUnknown *pSessionIUnknown);
	
	~CFKRowset(); 

	HRESULT				GetForeignKeys(
							WCHAR	*pwszTableCatalog,	// [in] constraint catalog restriction
							WCHAR	*pwszTableSchema,	// [in] constraint schema restriction
							WCHAR	*pwszTableName,		// [in] constraint name restriction
							WCHAR	*pwszFKName			// [in] foreign key constraint
						);

	DBCOUNTITEM			GetConstrNo();

	BOOL				MoveToRow(DBORDINAL index);

	WCHAR				*GetReferencedTableName();
	WCHAR				*GetFKName();
	DBID				*GetCrtReferencedColID() {
							return m_pCrtReferencedColID;
	}
	DBID				*GetCrtFKColID() {
							return m_pCrtFKColID;
	}
	DBUPDELRULE			GetUpdateRule() {
							return StringToUpDelRule(m_pwszUpdateRule);
	}
	DBUPDELRULE			GetDeleteRule() {
							return StringToUpDelRule(m_pwszDeleteRule);						
	}
}; //CFKRowset




// this class grabs data related key column usage
class CKeyColumnUsageRowset
{
private:
	WCHAR				*m_pwszTableCatalog;
	WCHAR				*m_pwszTableSchema;
	WCHAR				*m_pwszTableName;

	WCHAR				*m_pwszConsCatalog;
	WCHAR				*m_pwszConsSchema;
	WCHAR				*m_pwszConsName;

	IUnknown			*m_pSessionIUnknown;
	IUnknown			*m_pDSOIUnknown;

	LONG				m_lCrtPos;

	CTable				*m_pTable;
	CComfRowset			m_Rowset;

	DBID				*m_pCrtColID;

public:

	static BOOL			s_fMetadataInitialized;
	static BOOL			s_fSchemaSupported;

	static BOOL			s_fTableCatalogR;
	static BOOL			s_fTableSchemaR;
	static BOOL			s_fTableNameR;

	static BOOL			s_fConsCatalogR;
	static BOOL			s_fConsSchemaR;
	static BOOL			s_fConsNameR;

	static BOOL			IsSchemaSupported() {
							ASSERT(s_fMetadataInitialized);
							return s_fSchemaSupported;
	}

	CKeyColumnUsageRowset(IUnknown *pSessionIUnknown);
	
	~CKeyColumnUsageRowset(); 

	HRESULT				GetKeyColumnUsage(
							WCHAR	*pwszTableCatalog,	// [in] constraint catalog restriction
							WCHAR	*pwszTableSchema,	// [in] constraint schema restriction
							WCHAR	*pwszTableName,		// [in] constraint name restriction
							WCHAR	*pwszConsCatalog,	// [in] constraint catalog restriction
							WCHAR	*pwszConsSchema,	// [in] constraint schema restriction
							WCHAR	*pwszConsName		// [in] constraint name restriction
						);

	DBCOUNTITEM			GetConstrNo();

	BOOL				MoveToRow(DBORDINAL index);

	WCHAR				*GetConstraintName() {
								return m_pwszConsName;
	}
	DBID				*GetCrtColID() {
							return m_pCrtColID;
	}
}; //CKeyColumnUsageRowset






//===============================================================================
//
// helper class for DBCONSTRAINTDESC
//
//===============================================================================
class CConsDesc
{
protected:
	DBCONSTRAINTDESC	*m_pConsDesc;
	BOOL				m_fFreeMem;

public:
	static IUnknown		*s_pDSOIUnknown;
	static IUnknown		*s_pSessionIUnknown;
	static CConstraints	*s_pConstraints;
	static BOOL			s_fInsideTransaction;

						CConsDesc(
							DBCONSTRAINTTYPE ConsType, 
							DBDEFERRABILITY Def = 0);

						CConsDesc(DBCONSTRAINTDESC *pConsDesc);

						~CConsDesc();

	virtual	BOOL		IsEqual(CConsDesc *pConsDesc) {
							return	pConsDesc &&
									pConsDesc->GetConstraintType() == GetConstraintType()
								&&	pConsDesc->GetDeferrability() == GetDeferrability()
								&&	CompareDBID(*(pConsDesc->GetConstraintID()), *GetConstraintID());
						}

						operator DBCONSTRAINTDESC*(){
							return m_pConsDesc;}

						operator DBID*(){
							return m_pConsDesc->pConstraintID;}

	// interface methods for constraint DBID
	void				SetConstraintName(WCHAR *pwszConstraintName);
	DBID				*GetConstraintID() {
							return m_pConsDesc->pConstraintID;}
	DBID				*SetConstraintID(DBID *pConstraintID);

	// interface methods for constraint type
	DBCONSTRAINTTYPE	SetConstraintType(DBCONSTRAINTTYPE ConstraintType){
							return m_pConsDesc->ConstraintType = ConstraintType;}
	DBCONSTRAINTTYPE	GetConstraintType(){
							return m_pConsDesc->ConstraintType;}

	// interface methods for constraint deferrability
	DBDEFERRABILITY		SetDeferrability(DBDEFERRABILITY Deferrability){
							return m_pConsDesc->Deferrability = Deferrability;}
	DBDEFERRABILITY		GetDeferrability(){
							return m_pConsDesc->Deferrability;}

	DBCONSTRAINTDESC	*GetConsDescOwnership(){
							if (m_fFreeMem)
							{
								m_fFreeMem = FALSE;
								return m_pConsDesc;
							}
							else
								return NULL;
						}

	virtual HRESULT		CopyTo(DBCONSTRAINTDESC *pCD); 

	// @cmember Build a name for the constraint
	HRESULT				MakeConstraintName();

	virtual HRESULT		GetInfoFromSchemaRowset(
		WCHAR			*pwszTableCatalog,
		WCHAR			*pwszTableSchema,
		WCHAR			*pwszTableName,
		WCHAR			*pwszConsCatalog,
		WCHAR			*pwszConsSchema,
		WCHAR			*pwszConsName
		) { return S_OK;}
}; //CConsDesc




class CKeyConstraint: public CConsDesc
{
protected:
	DBORDINAL			m_cMaxColumns;

	DBID				*AddColumnID(
							DBORDINAL		*pcMaxColumns,		// [in/out] maximum number of columns
							DBORDINAL		*pcColumns,			// [in/out] number of columns
							DBID			**prgColumnList,	// [in/out] array of columns
							DBID			*pColumnID			// [in] column ID to be added
						);

	HRESULT				ReleaseColumnIDs(
							DBORDINAL		*pcMaxColumns,		// [in] maximum number of columns
							DBORDINAL		*pcColumns,			// [in/out] number of columns
							DBID			**prgColumnList		// [in] array of columns
						);

	LONG				GetColumnIndex(
							DBORDINAL		ulColumns,
							DBID			*rgColumnList, 
							DBID			*pColumnID
						);

public:
						CKeyConstraint(
							DBCONSTRAINTTYPE ConsType, 
							DBDEFERRABILITY Def = 0);

						CKeyConstraint(DBCONSTRAINTDESC *pConsDesc) : CConsDesc(pConsDesc){
							ASSERT(NULL != pConsDesc);
							m_cMaxColumns = m_pConsDesc->cColumns;
						}

						~CKeyConstraint() {;}

	DBID				*rgColumnList() {
							return m_pConsDesc->rgColumnList;}
	DBORDINAL			cColumns() {
							return m_pConsDesc->cColumns;}

	// add a column to rgColumnList
	DBID				*AddColumn(DBID *pColumnID) {
							return AddColumnID(	&m_cMaxColumns, 
												&m_pConsDesc->cColumns, 
												&m_pConsDesc->rgColumnList, 
												pColumnID);
						}

	HRESULT				ReleaseColumnList() {
							return ReleaseColumnIDs(&m_cMaxColumns, 
												&m_pConsDesc->cColumns, 
												&m_pConsDesc->rgColumnList);
	}

	HRESULT				SetConstraint(
							CTable				*pTable, 
							DBORDINAL			cCol,
							DBORDINAL			*rgCol,
							DBDEFERRABILITY		Deferrability		= 0
						);

	HRESULT				SetConstraint(
							CTable				*pTable, 
							DBORDINAL			indexCol,
							DBDEFERRABILITY		Deferrability		= 0
						) 
	{
							return SetConstraint(pTable, 1, &indexCol, Deferrability);
	}

	HRESULT				CopyTo(DBCONSTRAINTDESC *pCD); 

	LONG				GetColumnIndex(DBID *pColumnID) {
							return GetColumnIndex(
									m_pConsDesc->cColumns,
									m_pConsDesc->rgColumnList, 
									pColumnID);
	}

	BOOL				IsEqual(CConsDesc*);
}; // CKeyConstraint




class CUniqueCons: public CKeyConstraint
{
public:
						CUniqueCons(DBDEFERRABILITY Def = 0) :
							CKeyConstraint(DBCONSTRAINTTYPE_UNIQUE, Def) {;}

						CUniqueCons(DBCONSTRAINTDESC *pConsDesc) : CKeyConstraint(pConsDesc){
							m_pConsDesc->ConstraintType = DBCONSTRAINTTYPE_UNIQUE;	
						}

						~CUniqueCons() {;}

	HRESULT				GetInfoFromSchemaRowset(
							WCHAR			*pwszTableCatalog,
							WCHAR			*pwszTableSchema,
							WCHAR			*pwszTableName,
							WCHAR			*pwszConsCatalog,
							WCHAR			*pwszConsSchema,
							WCHAR			*pwszConsName
						);
}; //CUniqueCons



class CPrimaryKeyCons: public CKeyConstraint
{
public:
						CPrimaryKeyCons(
							DBDEFERRABILITY Def = 0) :
							CKeyConstraint(DBCONSTRAINTTYPE_PRIMARYKEY, Def) {;}

						CPrimaryKeyCons(DBCONSTRAINTDESC *pConsDesc) : CKeyConstraint(pConsDesc){
							m_pConsDesc->ConstraintType = DBCONSTRAINTTYPE_PRIMARYKEY;	
						}

						~CPrimaryKeyCons() {;}

	HRESULT				GetInfoFromSchemaRowset(
							WCHAR			*pwszTableCatalog,
							WCHAR			*pwszTableSchema,
							WCHAR			*pwszTableName,
							WCHAR			*pwszConsCatalog,
							WCHAR			*pwszConsSchema,
							WCHAR			*pwszConsName
						);
}; //CPrimaryKeyCons



class CForeignKeyCons: public CKeyConstraint
{
protected:
	DBORDINAL	m_cMaxForeignKeyColumns;

public:
						CForeignKeyCons(
							DBID			*pReferencedID	= NULL,
							DBMATCHTYPE		MatchType		= DBMATCHTYPE_NONE, 
							DBUPDELRULE		UpdateRule		= DBUPDELRULE_NOACTION,
							DBUPDELRULE		DeleteRule		= DBUPDELRULE_NOACTION,
							DBDEFERRABILITY	Def				= 0);

						CForeignKeyCons(DBCONSTRAINTDESC *pConsDesc) : CKeyConstraint(pConsDesc){
							m_pConsDesc->ConstraintType = DBCONSTRAINTTYPE_FOREIGNKEY;	
							m_cMaxForeignKeyColumns = m_pConsDesc->cForeignKeyColumns;
						}

						~CForeignKeyCons() {;}

	DBID				*rgForeignKeyColumnList() {
							return m_pConsDesc->rgForeignKeyColumnList;}
	DBORDINAL			cForeignKeyColumns() {
							return m_pConsDesc->cForeignKeyColumns;}

	// interface methods for handling the referenced table
	DBID				*SetReferencedTableID(DBID *pDBID);
	DBID				*GetReferencedTableID() {
							return m_pConsDesc->pReferencedTableID;
						}

	// interface methods for update/delete rules and match type
	DBUPDELRULE			SetUpdateRule(DBUPDELRULE UpdateRule) {
							return m_pConsDesc->UpdateRule = UpdateRule; }
	DBUPDELRULE			GetUpdateRule() {
							return m_pConsDesc->UpdateRule;}
	DBUPDELRULE			SetDeleteRule(DBUPDELRULE DeleteRule) {
							return m_pConsDesc->DeleteRule = DeleteRule; }
	DBUPDELRULE			GetDeleteRule() {
							return m_pConsDesc->DeleteRule;}
	DBMATCHTYPE			SetMatchType(DBMATCHTYPE MatchType) {
							return m_pConsDesc->MatchType = MatchType;}
	DBMATCHTYPE			GetMatchType() {
							return m_pConsDesc->MatchType;}

	// add a foreign key column to rgForeignKeyColumnList
	DBID				*AddForeignKeyColumn(DBID *pColumnID) {
							return AddColumnID(	&m_cMaxForeignKeyColumns, 
												&m_pConsDesc->cForeignKeyColumns, 
												&m_pConsDesc->rgForeignKeyColumnList, 
												pColumnID);
						}

	HRESULT				ReleaseForeignKeyColumnList() {
							return ReleaseColumnIDs(&m_cMaxForeignKeyColumns, 
												&m_pConsDesc->cForeignKeyColumns, 
												&m_pConsDesc->rgForeignKeyColumnList);
	}

	HRESULT				SetConstraint(
		CTable				*pBaseTable, 
		DBORDINAL			cBaseCol,
		DBORDINAL			*rgBaseCol,
		CTable				*pReferencedTable, 
		DBORDINAL			cReferencedCol,
		DBORDINAL			*rgReferencedCol,
		DBMATCHTYPE			MatchType			= DBMATCHTYPE_FULL,
		DBUPDELRULE			UpdateRule			= DBUPDELRULE_NOACTION,
		DBUPDELRULE			DeleteRule			= DBUPDELRULE_NOACTION,
		DBDEFERRABILITY		Deferrability		= 0
	);

	HRESULT				CopyTo(DBCONSTRAINTDESC *pCD); 

	HRESULT				GetInfoFromSchemaRowset(
		WCHAR			*pwszTableCatalog,
		WCHAR			*pwszTableSchema,
		WCHAR			*pwszTableName,
		WCHAR			*pwszConsCatalog,
		WCHAR			*pwszConsSchema,
		WCHAR			*pwszConsName
	);

	LONG				GetFKColumnIndex(DBID *pColumnID) {
							return GetColumnIndex(
									m_pConsDesc->cForeignKeyColumns,
									m_pConsDesc->rgForeignKeyColumnList, 
									pColumnID);
	}

	BOOL				IsEqual(CConsDesc*);
}; //CForeignKeyCons



class CCheckCons: public CConsDesc
{
public:
						CCheckCons(
							WCHAR			*pwszConstraintText = NULL,
							DBMATCHTYPE		MatchType = DBMATCHTYPE_NONE, 
							DBDEFERRABILITY	Def = 0) :
							CConsDesc(DBCONSTRAINTTYPE_CHECK, Def) 
						{
							  m_pConsDesc->MatchType = MatchType;
							  m_pConsDesc->pwszConstraintText = wcsDuplicate(pwszConstraintText);
						}

						CCheckCons(DBCONSTRAINTDESC *pConsDesc) : CConsDesc(pConsDesc){
							m_pConsDesc->ConstraintType = DBCONSTRAINTTYPE_CHECK;	
						}

						~CCheckCons() {;}

	void				SetConstraintText(WCHAR *pwszConstraintText){
							  SAFE_FREE(m_pConsDesc->pwszConstraintText);
							  m_pConsDesc->pwszConstraintText = wcsDuplicate(pwszConstraintText);
	}

	WCHAR				*GetConstraintText(){
							  return m_pConsDesc->pwszConstraintText;
	  }

	HRESULT				CopyTo(DBCONSTRAINTDESC *pCD);
	
	HRESULT				GetInfoFromSchemaRowset(
		WCHAR			*pwszTableCatalog,
		WCHAR			*pwszTableSchema,
		WCHAR			*pwszTableName,
		WCHAR			*pwszConsCatalog,
		WCHAR			*pwszConsSchema,
		WCHAR			*pwszConsName
	);

	HRESULT				SetIsNotNULLCheckConstraint(
		CTable			*pTable,
		DBORDINAL		ulCol
	);

	HRESULT				SetIsNotNULLCheckConstraint(WCHAR*);

	HRESULT				SetLTCheckConstraint(
		DBCOLUMNDESC	*pColDesc,
		DBORDINAL		ulSeed
	);

	HRESULT				SetLTCheckConstraint(
		CTable			*pTable,
		DBORDINAL		ulCol,
		DBORDINAL		ulSeed
	);
}; //CCheckCons



class CConsDescArray
{
protected:
	DBORDINAL			m_cMaxConsDesc;
	DBORDINAL			m_cConsDesc;
	DBCONSTRAINTDESC	*m_rgConsDesc;

public:
						CConsDescArray() {
							m_cMaxConsDesc	= 0;
							m_cConsDesc		= 0;
							m_rgConsDesc	= NULL;
						}

						~CConsDescArray() {
							FreeConstraintDesc(&m_cConsDesc, &m_rgConsDesc);
						}

						operator ULONG() {
							ASSERT(m_cConsDesc < ULONG_MAX);
							return (ULONG)m_cConsDesc;
						}

						operator DBCONSTRAINTDESC*() {
							return m_rgConsDesc;
						}

	DBCONSTRAINTDESC	*AddConsDesc(CConsDesc &cons);
}; //CConsDescArray

class CConstraintRestrictions
{
public:
	WCHAR				*m_pwszTableCatalog;
	WCHAR				*m_pwszTableSchema;
	WCHAR				*m_pwszTableName;
	WCHAR				*m_pwszConsCatalog;
	WCHAR				*m_pwszConsSchema;
	WCHAR				*m_pwszConsName;
	CConsDesc			*m_pConsDesc;
	
	CConstraintRestrictions(
		CConsDesc *pConsDesc,
		WCHAR	*pwszTableCatalog,	// [in] table catalog restriction
		WCHAR	*pwszTableSchema,	// [in] table schema restriction
		WCHAR	*pwszTableName,		// [in] table name restriction (better not be NULL!)
		WCHAR	*pwszConsCatalog,	// [in] constraint catalog restriction
		WCHAR	*pwszConsSchema,	// [in] constraint schema restriction
		WCHAR	*pwszConsName		// [in] constraint name restriction
	)
	{
		m_pConsDesc = pConsDesc;
		m_pwszTableCatalog = wcsDuplicate(pwszTableCatalog);
		m_pwszTableSchema = wcsDuplicate(pwszTableSchema);
		m_pwszTableName = wcsDuplicate(pwszTableName);
		m_pwszConsCatalog = wcsDuplicate(pwszConsCatalog);
		m_pwszConsSchema = wcsDuplicate(pwszConsSchema);
		m_pwszConsName = wcsDuplicate(pwszConsName);
	}
	~CConstraintRestrictions()
	{
		PROVIDER_FREE(m_pwszTableCatalog);
		PROVIDER_FREE(m_pwszTableSchema);
		PROVIDER_FREE(m_pwszTableName);
		PROVIDER_FREE(m_pwszConsCatalog);
		PROVIDER_FREE(m_pwszConsSchema);
		PROVIDER_FREE(m_pwszConsName);
	}
}; //CConstraintRestrictions