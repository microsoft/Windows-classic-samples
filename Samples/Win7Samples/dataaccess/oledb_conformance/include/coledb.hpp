//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Connection Class Header Module | Declaration of base class for OLE DB Test Modules.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 COLEDB Elements|
//
// @subindex COLEDB|
// @subindex CDataSourceObject|
// @subindex CSessionObject|
// @subindex CCommandObject|
// @subindex CRowsetObject|             
//
//---------------------------------------------------------------------------

#ifndef __COLEDB_HPP_
#define __COLEDB_HPP_

#include "CTable.hpp"
#include "modstandard.hpp"
#include "cexterr.hpp"		//For extended error support


////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////
#define DEFINE_IUNKNOWN_MEMBER_DATA(Class, Interface)					\
		ULONG			m_cRef;											\
		CBase*			m_pCBase;										\
		IUnknown*		m_pUnkOuter;									


#define DEFINE_IUNKNOWN_CTOR_DTOR(Class, Interface)						\
	Class( CBase* pCBase, IUnknown* pUnkOuter)							\
	{																	\
		m_cRef		= 0;												\
		m_pCBase	= pCBase;											\
		m_pUnkOuter = pUnkOuter ? pUnkOuter : m_pCBase;					\
	}																	\
	virtual ~Class()													\
	{																	\
	}																	

#define DEFINE_DELEGATING_QI_ADDREF_RELEASE								\
	STDMETHODIMP_(ULONG)	AddRef(void)								\
		{																\
			return m_pUnkOuter->AddRef();								\
		}																\
	STDMETHODIMP_(ULONG)	Release(void)								\
		{																\
			return m_pUnkOuter->Release();								\
		}																\
	STDMETHODIMP			QueryInterface(REFIID riid, LPVOID *ppv)	\
		{																\
			return m_pUnkOuter->QueryInterface(riid, ppv);				\
		}


#define DEFINE_QI_ADDREF_RELEASE										\
	STDMETHODIMP_(ULONG)	AddRef(void)								\
		{																\
			InterlockedIncrement((LONG*)&m_cRef);						\
			return m_cRef;												\
		}																\
	STDMETHODIMP_(ULONG)	Release(void)								\
		{																\
			if(InterlockedDecrement((LONG*)&m_cRef))					\
				return m_cRef;											\
																		\
			delete this;												\
			return 0;													\
		}																\
	STDMETHODIMP			QueryInterface(REFIID riid, LPVOID *ppv);




////////////////////////////////////////////////////////
// CBase
//
////////////////////////////////////////////////////////
class CBase : public IUnknown
{
	protected: //@access protected
		DEFINE_IUNKNOWN_MEMBER_DATA(CBase, IUnknown);

	public: //@access public
		CBase(CBase* pCBase, LPUNKNOWN pUnkOuter);
		virtual ~CBase();

		//IUnknown
		DEFINE_QI_ADDREF_RELEASE;

		//Helpers
		IUnknown*	GetBaseObj();
		HRESULT		GetBaseInterface(REFIID riid, IUnknown** ppIUnknown);
}; 


//--------------------------------------------------------------------
// @class COLEDB | Base Class for connection.
//
// This class is the base class for the OLE DB object classes. Its main function
// is to grab a pIMalloc pointer and contain the VerifyInterface function.
//
// @base public | CTestCases
//
//--------------------------------------------------------------------
class COLEDB : public CTestCases
{
	// @access public
public:
	// @cmember Constructor. Passes test case name to CTestCases. <nl>
	COLEDB(WCHAR * pwszTestCaseName  = INVALID(WCHAR*));

	// @cmember Destructor frees m_pIMalloc. <nl>
	virtual ~COLEDB();	
	
	// @cmember Init function. <nl>
	virtual BOOL Init();

	// @cmember Terminate function. <nl> 
	virtual BOOL Terminate();

	// @cmember	HRESULT for OLE and OLE DB calls. <nl>
	HRESULT    	m_hr;
	
	// @cmember	Task Memory Allocation Interface. <nl>
	IMalloc*	m_pIMalloc;	

	// @cmember Extended error object. <nl>
	CExtError*	m_pExtError;

	// @cmember Flag to tell the CompareString func to print the String. <nl>
	BOOL		m_fLocalize;

protected:
	BOOL		m_fInsideTestCase;
};

//--------------------------------------------------------------------
// @class CDataSourceObject | Base Class for getting a DSO.
//
// This class will grab a valid interface on the Data Source Object and make a connection to 
// the data source.	Note that the connection is currently hardcoded but
// this will change.
//
// @base public | COLEDB
//
//--------------------------------------------------------------------
class CDataSourceObject : public COLEDB
{
	// @access	public
	public:

	// @cmember	Interface for Data Source Object, gotten by GetDataSourceObject. <nl>
	IDBInitialize *	m_pIDBInitialize;	

	// @cmember We must be initialized if we have a valid IDBCreateCommand ptr. <nl>
	BOOL m_fInitialized;

	// @cmember	Constructor. <nl>
	CDataSourceObject(
		WCHAR * pwszTestCaseName // [IN] Test Case Name to pass to table creation object
	);
 
	// @cmember	Destructor. <nl>
	virtual ~CDataSourceObject(void);	

	// @cmember Instantiates a DSO and puts an ITransactionDispenser interface <nl>
	//in m_pIDBInitialize. <nl>
	HRESULT CreateDataSourceObject(void);

	// @cmember Initialize DataSource. <nl>
	HRESULT InitializeDSO(EREINITIALIZE eReinitialize	=	REINITIALIZE_NO, // [IN] Reinitialize the data source (Default = NO)
			ULONG cPropSets = 0, DBPROPSET* rgPropSets = NULL);

	// @cmember Unitialize DataSource. <nl>
	HRESULT UninitializeDSO(void);
	
	// @cmember Sets the IDBInitialize interface for a Data Source Object, which <nl>
	//will be used instead of getting a DataSource Object using CreateDataSourceObject. <nl>
	HRESULT SetDataSourceObject(
		IUnknown*	pDataSource,				// [IN] Use client's DataSource interface
		BOOL		fAlreadyInitialized = FALSE	// [IN] Whether the DSO has been initialized.
	);

	// @cmember Releases object and associated interfaces. <nl>
	void ReleaseDataSourceObject(void);	
	
	// @cmember Returns the m_pIDBInitialize pointer. 
	//	No AddRef.<nl>
	IDBInitialize* pIDBInitialize() { ASSERT(m_pIDBInitialize!=NULL); return m_pIDBInitialize;};

	// @cmember Returns DSO pointer. <nl>
	HRESULT GetDataSourceObject(
		REFIID		riid,		// [IN] IID of DSO pointer
		IUnknown**	ppIUnknown	// [OUT] DSO pointer
	);
};

//--------------------------------------------------------------------
// @class CSessionObject | Session Object class. <nl>
//
// Creates or Sets the Session object. Also encapsulates the CTable object. 
// And has the flag to indicate if the Command object has been found.
//
// @base public | CDataSourceObject
//
//--------------------------------------------------------------------
class CSessionObject : public CDataSourceObject
{
	// @access public
	public:

	// @cmember	Interface for DB Session, gotten by GetDBSession. <nl>
	// This is a mandatory interface if commands are not supported. <nl>
	IOpenRowset * m_pIOpenRowset;

	// @cmember	Interface for DB Session, gotten by GetDBSession. <nl>
	// This is a mandatory interface if commands are supported. <nl>
	IDBCreateCommand * 	m_pIDBCreateCommand;

	// @cmember Array of DBPROPSET structures. <nl>
	DBPROPSET *	m_rgPropSets;				

	// @cmember Count of properties. <nl>
	ULONG		m_cPropSets;				 

	// @cmember Rowset from IColumnsRowset, client must check <nl>
	// m_pIOpenRowset or m_pIDBCreateCommand to figure out <nl>
	// if it is a rowset or a simple rowset. <nl>
	IUnknown * m_pColRowset;

	//@cmember Table object for class. <nl>
	CTable * m_pTable;
	
	// @cmember Some of the QUERY enums require 2 tables. If two tables <nl>
	// are required. CRowsetObject creates the second table <nl>
	// with NUMROWS number of rows and places the second table in m_pTable2. <nl>
	CTable * m_pTable2;

	// @cmember Should table from SetTable be destroyed in <nl>
	// CSessionObject's destructor. <nl>
	EDELETETABLE m_fDeleteTable;

	// @cmember Should table from SetTable be destroyed in <nl>
	// CSessionObject's destructor. <nl>
	EDELETETABLE m_fDeleteTable2;

	// @cmember	Constructor. <nl>
	CSessionObject(
		WCHAR * pwszTestCaseName	// parm [IN] Test Case Name
	);

	// @cmember Destructor. <nl>
	virtual ~CSessionObject(void);	

	// @cmember Creates a DBSession (thru Initialization if necessary) <nl>
	// and puts an IDBCreateCommand interface in m_pIDBCreateCommand. <nl>
	HRESULT CreateDBSession(
		EROWSETGENERATED eRowsetGenerated = EITHER_GENERATED	//[IN] Whether IOpenRowset or IDBCreateCommand
																//should be used to eventually retrieve a rowset
																//Default is that IDBCreateCommand is tried first, 
																//if not supported, IOpenRowset is used.
		);

	// @cmember Sets the IDBCreateCommand interface for a DB Session, which <nl>
	// will be used instead of getting a DB Session using CreateDBSession. <nl>
	void SetDBSession(
		IUnknown * pIDBCreateCommand	 //[IN] IDBCreateCommand pointer
	);

	// @cmember Releases session and associated interfaces. <nl>
	void 		ReleaseDBSession(void);			

	// @cmember Sets current session's table to pTable. <nl>
	void SetTable(
		CTable * pTable,				// [IN] Client's table object
		EDELETETABLE eDeleteTable		// [IN] Should table be deleted
	);

	// @cmember Sets current session's second table to pTable. <nl>
	void SetTable2(
		CTable * pTable,				// [IN] Client's table object
		EDELETETABLE eDeleteTable		// [IN] Should table be deleted
	);

	// @cmember Returns the m_pIOpenRowset pointer. 
	//	No AddRef.<nl>
	IOpenRowset* pIOpenRowset() { ASSERT(m_pIOpenRowset!=NULL); return m_pIOpenRowset;};

	// @cmember Get Session Pointer. <nl>
	HRESULT GetSessionObject(
		REFIID		riid,			// [IN] IID of Session Pointer
		IUnknown**	ppIUnknown		// [OUT] Session Pointer
	);	

	// @cmember Flag telling if Commands are supported
	inline BOOL GetCommandSupport() { return m_pIDBCreateCommand!=NULL;	}

	// @cmember Get a column specific property on a property set
	BOOL GetColSpecProp(GUID, DBPROPID*);

	// @cmember Get a non column specific property on a given property set
	BOOL GetNonColSpecProp(GUID, DBPROPID*);
};

//--------------------------------------------------------------------
// @class CCommandObject | Command Object class. <nl> 
//
// Creates or sets the command object, if it can be found. If it cannot
// be found, it marks that on the Session object.
//
// @base public | CSessionObject
//
//--------------------------------------------------------------------
class CCommandObject : public CSessionObject 
{
	// @access public
	public:

	// @cmember Interface for Command Object, gotten by GetCommandObject. <nl>
	ICommand * 			m_pICommand;

	// @cmember Interface for Rowset Object, gotten by GetRowsetObject. <nl>
	IAccessor *			m_pIAccessor; 	
	
	// @cmember Constructor. <nl>
	CCommandObject(
		WCHAR * pwszTestCaseName	// [IN] Test case name
	);

	// @cmember Destructor. <nl>
	virtual ~CCommandObject(void);	

	// @cmember Instantiates a command object and puts an ICommand interface <nl>
	// in m_pICommand. <nl>
	HRESULT CreateCommandObject(IUnknown* pIUnkOuter = NULL);

	// @cmember Sets the ICommand interface for a Command Object, which <nl>
	// will be used instead of getting a Command Object using CreateCommandObject. <nl>
	void SetCommandObject(
		ICommand * pICommand	// [IN] Client's current command object addref'd
	);

	// @cmember Releases object and associated interfaces. <nl>
	void ReleaseCommandObject(
		ULONG	ulRefCount = 0		// [IN] Expected ref count after the release		
	);

	// @cmember Get Command Pointer. <nl>
	HRESULT GetCommandObject(
		REFIID			riid,			// [IN] IID of Command Object
		IUnknown**		ppIUnknown		// [OUT] Command Pointer
	);		
};

//--------------------------------------------------------------------
// @class CRowsetObject | Rowset object class
//
// Creates or sets the rowset object. Also the only place a table is created. 
//
// @base public | CCommandObject
//
//--------------------------------------------------------------------
class CRowsetObject : public CCommandObject
{
	// @access public
	public:
	
	// the number of restrictions
	// used for schema rowsets
	ULONG		m_cRestrictions;

	// array od restrisctions
	VARIANT		*m_rgRestrictions;

	// @cmember Array of col ordinals for table. <nl>
	DB_LORDINAL *	m_rgTableColOrds;			

	// @cmember Count of columns in rowset. <nl>
	DBORDINAL		m_cRowsetCols;				

	// @cmember Constructor - takes test case name. <nl>
	CRowsetObject(
		WCHAR * pwszTestCaseName	 // [IN] Testcase name
	);

	// @cmember Destructor. <nl>
	virtual ~CRowsetObject(void);
	
	// @cmember Create rowset object. <nl>
	HRESULT	CreateRowsetObject(
		EQUERY		eQuery = USE_SUPPORTED_SELECT_ALLFROMTBL,//[IN] Query with which to create rowset,
															//default is do "select * from <tbl>, using
															//a command if supported, else using IOpenRowset
		REFIID		riid = IID_IAccessor,					//[IN] Type of riid to ask for 
		EEXECUTE	eExecute = EXECUTE_ALWAYS,				//@parm [IN] whether to always execute the command
															//can be either EXECUTE_ALWAYS or 
															//EXECTUE_IFNOERROR	
		IUnknown**  ppIRowset = NULL
	);
	
	// @cmember Sets the IAccessor interface for a Rowset Object, which <nl>
	// will be used instead of getting a Rowset Object using CreateRowsetObject. <nl>
	virtual	BOOL SetRowsetObject(
		IAccessor * pIAccessor		//[IN] IAccessor for rowset
	);
		
	// @cmember Sets the desired rowset properties to be used in CreateRowsetObject(). <nl>
	HRESULT	SetRowsetProperties(
		DBPROPSET 	rgProperties[],	//[IN] Array of DBPROPSET structures.
		ULONG		cProperties		//[IN] Count of property sets.
	);

	// @cmember Releases object and associated interfaces. <nl>
	void ReleaseRowsetObject(
		DBREFCOUNT		ulRefCount = 0	//[IN] Expected ref count after object is released
	);

	// @cmember CreateColumnRowset <nl>
	// Generates an IColumnsRowset based on rowset currently held in this class. <nl>
	// Returns all optional columns in rowset. <nl>
	HRESULT CreateColumnRowset(void);

	// @mfunc Releases the restrictions array
	void ReleaseRestrictions();

	// @cmember Set the restrictions to be used for a schema rowset
	BOOL	SetRestrictions(ULONG cRestrictions, VARIANT *rgRestrictions);
};



//////////////////////////////////////////////////////////////////////
// @class CRowObject | Row object class
//
// Creates or sets the row object.
//
//////////////////////////////////////////////////////////////////////
class CRowObject
{
public:
	// Constructors
	CRowObject();
	virtual ~CRowObject(void);
	
	// Members
	HRESULT			CreateRowObject(IUnknown* pUnkRowset, HROW hRow);
	virtual HRESULT	SetRowObject(IUnknown* pUnkRow);
	virtual void	ReleaseRowObject();
	virtual void	ReleaseColAccess();

	//CreateColAccess
	HRESULT CreateColAccess
			(
				DBORDINAL*		pcColAccess, 
				DBCOLUMNACCESS** prgColAccess, 
				void**			ppData, 
				DBLENGTH*		pcbRowSize		= NULL, 
				DWORD			dwColsToBind	= ALL_COLS_BOUND, 
				BLOBTYPE		dwBlobType		= NO_BLOB_COLS,
				ECOLUMNORDER	eBindingOrder	= FORWARD, 
				ECOLS_BY_REF	eColsByRef		= NO_COLS_BY_REF,
				DBTYPE			dwModifier		= DBTYPE_EMPTY, 
				DBORDINAL		cColsToBind		= 0, 
				DBORDINAL*		rgColsToBind	= NULL, 
				DBPART			dwPart			= DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS
			);

	//CreateColAccessUsingFilter
	HRESULT CreateColAccessUsingFilter
			(
				DBORDINAL		*pcColAccess, 
				DBCOLUMNACCESS	**prgColAccess, 
				void			**ppData, 
				CSchema			*pSchema,
				DBLENGTH		*pcbRowSize	= NULL
			);

	//GetColumnInfo
	HRESULT GetColumnInfo
		(
			DBORDINAL*			pcColumnInfo,			// @parm [OUT] Count of Columns
			DBCOLUMNINFO**		prgColumnInfo,			// @parm [OUT] Array of ColumnInfo
			WCHAR**				ppStringBuffer
		);

	//GetExtraColumnInfo
	HRESULT GetExtraColumnInfo
		(
			DBORDINAL*			pcColumnInfo,			// @parm [OUT] Count of Columns
			DBCOLUMNINFO**		prgColumnInfo,			// @parm [OUT] Array of ColumnInfo
			WCHAR**				ppStringBuffer,
			DBORDINAL**			prgColOrdinals = NULL
		);

	//IsExtraColumn
	BOOL	IsExtraColumn(DBID* pColumnID);

	//BindingsToColAccess
	HRESULT BindingsToColAccess(DBCOUNTITEM	cBindings, DBBINDING* rgBindings, void*	pData,
				DBORDINAL*	pcColumns, DBCOLUMNACCESS**	prgColAccess);

	//ColAccessToBindings
	HRESULT ColAccessToBindings(DBORDINAL	cColumns, DBCOLUMNACCESS* rgColAccess,
				DBCOUNTITEM* pcBindings, DBBINDING** prgBindings,	void** ppData);

	//GetSourceRowset
	HRESULT	GetSourceRowset(REFIID riid, IUnknown** ppIRowset, HROW* phRow);
	HRESULT	Open
		(
			IUnknown*			pIUnkOuter,
			const DBID*			pColumnID,
			REFGUID				rGuidType		= GUID_NULL,
			REFIID				riid			= IID_IUnknown,
			IUnknown**			ppIUnknown		= NULL
		);	

	HRESULT	VerifyOpen
		(
			DBCOUNTITEM			iRow,
			CSchema*			pSchema,
			IUnknown*			pIUnkOuter,
			const DBID*			pColumnID,
			REFGUID				rGuidType		= GUID_NULL,
			REFIID				riid			= IID_IUnknown,
			IUnknown**			ppIUnknown		= NULL
		);	

	//GetData
	HRESULT GetColumns(DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess);
	HRESULT GetColumns(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData);

	//VerifyGetColumns
	BOOL	VerifyGetColumns
		(
			DBCOUNTITEM			iRow,
			CSchema*			pCSchema,
			DWORD				dwColsToBind	= ALL_COLS_BOUND,			
			BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
			ECOLUMNORDER		eBindingOrder	= FORWARD,		
			ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
			DBTYPE				dwModifier		= DBTYPE_EMPTY,
			DBORDINAL			cColsToBind		= 0,
			DBORDINAL*			rgColsToBind    = NULL,
			DBPART				dwPart			= DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS
		);

	
	//SetData
	HRESULT SetColumns(DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess);
	HRESULT SetColumns(DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData);
	HRESULT FillColAccess(CSchema* pSchema, DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess, DBCOUNTITEM iRow, EVALUE eValue = PRIMARY, DWORD	dwColsToBind = UPDATEABLE_COLS_BOUND);

	//VerifySetColumns
	BOOL	VerifySetColumns
		(
			DBCOUNTITEM			iRow,
			CTable*				pCTable,
			DWORD				dwColsToBind	= UPDATEABLE_NONINDEX_COLS_BOUND,
			BLOBTYPE			dwBlobType		= NO_BLOB_COLS,
			ECOLUMNORDER		eBindingOrder	= FORWARD,		
			ECOLS_BY_REF		eColsByRef		= NO_COLS_BY_REF,				
			DBTYPE				dwModifier		= DBTYPE_EMPTY,
			DBORDINAL			cColsToBind		= 0,
			DBORDINAL*			rgColsToBind    = NULL,
			DBPART				dwPart			= DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS
		);

	//UpdateStatusLength
	HRESULT UpdateStatusLength(DBORDINAL cColumns, DBCOLUMNACCESS* rgColAccess, 
				DBCOUNTITEM cBindings, DBBINDING* rgBindings, void* pData);

	//CompareColAccess
	BOOL	CompareColAccess(DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess, 
				DBCOUNTITEM iRow,	CSchema* pSchema, EVALUE eValue = PRIMARY);
	
	//CompareColBuffer
	BOOL	CompareColBuffer(DBORDINAL cGetColAccess, DBCOLUMNACCESS* rgGetColAccess, 
				DBORDINAL cSetColAccess, DBCOLUMNACCESS* rgSetColAccess, BOOL fSetData = TRUE);

	//GetResRowsetData
	BOOL	GetResRowsetData(DBID colid, void**ppData);

	//IGetSession::GetSession wrapper
	HRESULT	GetSession(REFIID riid, IUnknown** ppSession);

	//ExecuteCommand - executes a row scoped command
	HRESULT ExecuteCommand(EQUERY eQuery, REFIID riid, ULONG cPropSets,
				DBPROPSET* rgPropSets, IUnknown **ppIRowset);

	//Interfaces

	IRow*	pIRow()	{ ASSERT(m_pIRow);  return m_pIRow; }

	IGetSession*		pIGetSession()		{ ASSERT(m_pIGetSession);	return m_pIGetSession;	}
	IConvertType*		pIConvertType()		{ ASSERT(m_pIConvertType);  return m_pIConvertType; }
	IColumnsInfo*		pIColumnsInfo()		{ ASSERT(m_pIColumnsInfo);  return m_pIColumnsInfo; }
	IDBCreateCommand*	pIDBCreateCommand()	{ return m_pIDBCreateCommand; }


//protected:
	DBORDINAL			m_cColAccess;
	DBCOLUMNACCESS*		m_rgColAccess;
	void*				m_pData;

protected:
	// Data
	IRow*				m_pIRow;
	IGetSession*		m_pIGetSession;
	IConvertType*		m_pIConvertType;
	IColumnsInfo*		m_pIColumnsInfo;

	//optional
	IDBCreateCommand*	m_pIDBCreateCommand;
};



#endif // __COLEDB_HPP_
