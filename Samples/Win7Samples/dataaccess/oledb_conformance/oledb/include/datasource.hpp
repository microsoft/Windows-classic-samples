//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1998-2000 Microsoft Corporation.  
//
// @doc 
//
// @module DataSource Header Module | 	This module contains header information
//					for the CLightDataSource, ClightSession and CLightRowset classes
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//---------------------------------------------------------------------------

#ifndef __CMYDSO_HPP__
#define __CMYDSO_HPP__


#include "CPropSet.hpp"
#include "ProviderInfo.h"

enum CREATIONMETHODSENUM
{
	CREATIONMETHODS_FIRST				= 1,
	CREATIONMETHODS_GETDATASOURCE		= 1,
	CREATIONMETHODS_CREATEDBINSTANCE    = 2,
	CREATIONMETHODS_CREATEDBINSTANCEEX  = 3,
	CREATIONMETHODS_PROMPTINITIALIZE	= 4,
	CREATIONMETHODS_COCREATEDSO         = 5,
	CREATIONMETHODS_ADOCONNECTION		= 6,
	CREATIONMETHODS_LAST				= 5, // ADOCONNECTION returns an Initialized datasource - this value allows for testing all interchangeable datasource creationmethods, often through an iteration.
	CREATIONMETHODS_UNKNOWN				= 0xfb, // this means we don't know which method was used to create the data source object
};


	#define APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)	\
		INIT_ACT(IUnknown)										\
		ACT(IDBCreateSession)									\
		ACT(IDBInitialize)										\
		ACT(IDBProperties)										\
		ACT(IPersist)											\
		ACT(IConnectionPointContainer)							\
		ACT(IDBAsynchStatus)									\
		ACT(IDBDataSourceAdmin)									\
		ACT(IDBInfo)											\
		ACT(IPersistFile)										\
		ACT(ISupportErrorInfo)									\
		FINAL_ACT(IServiceProvider)								
//		ACT(ITestInfo)											\
//		FINAL_ACT(IGetPool)




//class CServiceComp;
class CLightSession;

class CLightDataSource {
	protected:
		IUnknown					*m_pIUnknown;					// [mandatory]
		IDBCreateSession			*m_pIDBCreateSession;			// [mandatory]
		IDBInitialize				*m_pIDBInitialize;				// [mandatory]
		IDBProperties				*m_pIDBProperties;				// [mandatory]
		IPersist					*m_pIPersist;					// [mandatory]
		IConnectionPointContainer	*m_pIConnectionPointContainer;	// [optional]
		IDBAsynchStatus				*m_pIDBAsynchStatus;			// [optional]
		IDBDataSourceAdmin			*m_pIDBDataSourceAdmin;			// [optional]
		IDBInfo						*m_pIDBInfo;					// [optional]
		IPersistFile				*m_pIPersistFile;				// [optional]
		ISupportErrorInfo			*m_pISupportErrorInfo;			// [optional]
		IServiceProvider			*m_pIServiceProvider;			// [optional, for providers who customize the DSL UI]

		LONG						m_lRef;
		// m_fInitialized is used to keep track of DSO's state
		BOOL						m_fInitialized;
		BOOL						m_fIsDirty;	// was initialized before
		// remember what provider was used to create this DSO
		CLSID						*m_pclsidProvider;
		CModifyRegistry				*m_pModifyRegistry;
		CPropSets					m_PropSets;
		CREATIONMETHODSENUM			m_dwCreationMethod;


		// increment/decrement methods to keep track of the number of interfaces
		// kept on the object => used to make sure we have at lease one valid interface
		ULONG						AddRef() {
									  return InterlockedIncrement(&m_lRef);
		}

		ULONG						Release() {
									  return InterlockedDecrement(&m_lRef);
		}

		HRESULT						CacheInterface(
										REFIID		riid		// [in] Interface to be cached
									);

		IUnknown					*pIUnknown();

		// cache registry info about the provider; most interesting for us now is OLEDB_SERVICES
		BOOL						CacheProviderRegistry() {
										if (!m_pclsidProvider)
											return FALSE;

										SAFE_DELETE(m_pModifyRegistry);
										m_pModifyRegistry = new CModifyRegistry(*m_pclsidProvider);
										return TRUE;
		}

		BOOL						IsNativeInterface(REFIID riid) {
										return (IID_IPersist == riid)
											||	(IID_IPersistFile == riid)
											||	(IID_IDBProperties == riid)
											||	(IID_IDBInitialize == riid)
											||	(IID_IUnknown == riid);
		}

	public:
		
		static CSourcesSet			*s_pSourcesSet;

									// create just the wrapper object
									CLightDataSource()
									{
										#define INIT_ACT(Interface)		m_p##Interface = NULL;
										#define	ACT(Interface)			INIT_ACT(Interface)
										#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

												APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

										#undef INIT_ACT
										#undef ACT
										#undef FINAL_ACT

										m_lRef				= 0;
										m_pclsidProvider	= NULL;
										m_pModifyRegistry	= NULL;
										m_dwCreationMethod	= CREATIONMETHODS_UNKNOWN;
										m_fIsDirty			= FALSE;
										m_fInitialized		= FALSE;
									}
									
									~CLightDataSource() {
										ReleaseAll();
									}

		void						ReleaseAll(){
										#define INIT_ACT(Interface)						\
											SAFE_RELEASE(m_p##Interface);
										#define	ACT(Interface)			INIT_ACT(Interface)
										#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

												APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

										#undef INIT_ACT
										#undef ACT
										#undef FINAL_ACT
										m_lRef = 0;
										SAFE_FREE(m_pclsidProvider);
										SAFE_DELETE(m_pModifyRegistry);
										m_fIsDirty			= FALSE;
										m_fInitialized		= FALSE;
										m_PropSets.Free();
		}

		BOOL						SetProviderCLSID(CLSID clsidProvider);
		CREATIONMETHODSENUM			SetCreationMethod(CREATIONMETHODSENUM dwCreationMethod){
										return m_dwCreationMethod = dwCreationMethod;
		}

		HRESULT						ReleaseInterface(REFIID riid);
		HRESULT						QueryInterface(REFIID riid, LPVOID *ppInterface); 

		HRESULT						Attach(
										REFIID				riid,				// [in] Interface to be cached
										IUnknown			*pIUnknown,			// [in] Pointer to that interface
										CREATIONMETHODSENUM	dwCreationMethod	// [in] the method used to create the object
									);

		HRESULT						SetProperties(ULONG cPropSets, DBPROPSET *rgPropSets);
		HRESULT						SetProperties(CPropSets *pPropSets) {
										return SetProperties(*pPropSets, *pPropSets);
		}

		HRESULT						GetPropertyInfo(
										ULONG				cPropertyIDSets,
										const DBPROPIDSET	rgPropertyIDSets[],
										ULONG				*pcPropertyInfoSets,
										DBPROPINFOSET		**prgPropertyInfoSets,
										OLECHAR				**ppDescBuffer
									);

		HRESULT						GetProperties(
										ULONG		cPropertyIDSets,
										DBPROPIDSET	*rgPropertyIDSets, 
										ULONG		*pcPropSets, 
										DBPROPSET	**prgPropSets
									);
		HRESULT						GetProperties(ULONG *pcPropSets, DBPROPSET **prgPropSets) {
										return GetProperties(0, NULL, pcPropSets, prgPropSets);
		}
		HRESULT						GetProperties(CPropSets *pPropSets);
		HRESULT						GetInitProperties(CPropSets *pPropSets);

		HRESULT						Initialize();

		HRESULT						Uninitialize();

		HRESULT						CreateSession(
										IUnknown	*pUnkOuter,
										REFIID		riid,
										IUnknown	**ppDBSession
									);

		HRESULT						CreateSession(
										IUnknown	*pUnkOuter,
										REFIID		riid,
										CLightSession	*pDBSession
									);

		HRESULT						SetCurrentCatalog(WCHAR *pwszCatalogName);
		WCHAR						*GetCurrentCatalog();


		#define INIT_ACT(Interface)						\
			operator Interface* () {					\
				CacheInterface(IID_##Interface);		\
				return m_p##Interface;					\
			}

		#define	ACT(Interface)			INIT_ACT(Interface)

		#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

				APPLY_ON_DSO_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

		#undef INIT_ACT
		#undef ACT
		#undef FINAL_ACT

		CLightDataSource				*operator = (IUnknown *pIUnk);


		// information about the status of the datasource object
		//	CLightDataSource::GetDSOStatus
		//	Returns TRUE if there is any indication that this Datasource is initialized
		//	Returns FALSE if all indications of Initialization are false
		//	Details in what way it seems Initialized using the INIT_INDICATORS structure
		//	Does following tests:
		//		1) GetPropertyInfo for non Initialization values
		//		2) GetProperties for non Initialization values
		//		3) QI for IDBCreateSession
		//		4) IPersistFile::Load returns AlreadyInitialized, if IPersistFile is supported
		BOOL						GetDSOStatus();
		BOOL						IsDSOUninitialized(){
										return FALSE == GetDSOStatus();
		}
		BOOL						IsDSOInitialized(){
										return TRUE == GetDSOStatus();
		}
		// All DSO's retrieved through SC support IGetPool 
		// use this interface to retrieve the status of the DSO (not pooled, create from pool, 
		// drawn from pool, etc)
		BOOL						CheckPropertyValues();
}; //CLightDataSource



// smart class for session object
	#define APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)	\
		INIT_ACT(IUnknown)									\
		ACT(IGetDataSource)									\
		ACT(IOpenRowset)									\
		ACT(ISessionProperties)								\
		ACT(IAlterIndex)									\
		ACT(IAlterTable)									\
		ACT(IBindResource)									\
		ACT(ICreateRow)										\
		ACT(IDBCreateCommand)								\
		ACT(IDBSchemaRowset)								\
		ACT(IIndexDefinition)								\
		ACT(ISupportErrorInfo)								\
		ACT(ITableCreation)									\
		ACT(ITableDefinition)								\
		ACT(ITableDefinitionWithConstraints)				\
		ACT(ITransactionJoin)								\
		ACT(ITransactionLocal)								\
		ACT(ITransaction)									\
		FINAL_ACT(ITransactionObject)

class CLightSession {
	protected:
		// define interfaces member data
		#define INIT_ACT(Interface)		Interface	*m_p##Interface;				
		#define	ACT(Interface)			INIT_ACT(Interface)
		#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

		APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

		#undef INIT_ACT
		#undef ACT
		#undef FINAL_ACT

		LONG						m_lRef;
		CPropSets					m_PropSets;

		// increment/decrement methods to keep track of the number of interfaces
		// kept on the object => used to make sure we have at lease one valid interface
		ULONG						AddRef() {
									  return InterlockedIncrement(&m_lRef);
		}

		ULONG						Release() {
									  return InterlockedDecrement(&m_lRef);
		}

		HRESULT						CacheInterface(
										REFIID		riid		// [in] Interface to be cached
									);

		IUnknown					*pIUnknown();

	public:
		
									// create just the wrapper object
									CLightSession()
									{
										#define INIT_ACT(Interface)		m_p##Interface = NULL;
										#define	ACT(Interface)			INIT_ACT(Interface)
										#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

												APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

										#undef INIT_ACT
										#undef ACT
										#undef FINAL_ACT

										m_lRef				= 0;
									}
									
									~CLightSession() {
										ReleaseAll();
									}

		void						ReleaseAll(){
										#define INIT_ACT(Interface)						\
											SAFE_RELEASE(m_p##Interface);
										#define	ACT(Interface)			INIT_ACT(Interface)
										#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

												APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

										#undef INIT_ACT
										#undef ACT
										#undef FINAL_ACT
										m_lRef = 0;
		}

		HRESULT						ReleaseInterface(REFIID riid);
		HRESULT						QueryInterface(REFIID riid, LPVOID *ppInterface); 

		HRESULT						GetDataSource(
										REFIID		riid,
										IUnknown	**ppDataSource
									);

		HRESULT						GetDataSource(REFIID riid, CLightDataSource &rDataSource);
		HRESULT						GetDataSource(CLightDataSource &rDataSource) {
										return GetDataSource(IID_IDBProperties, rDataSource);
		}

		HRESULT						Attach(
										REFIID		riid,		// [in] Interface to be cached
										IUnknown	*pIUnknown	// [in] Pointer to that interface
									);

		#define INIT_ACT(Interface)						\
			operator Interface* () {					\
				CacheInterface(IID_##Interface);		\
				return m_p##Interface;					\
			}

		#define	ACT(Interface)			INIT_ACT(Interface)

		#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

				APPLY_ON_SESSION_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

		#undef INIT_ACT
		#undef ACT
		#undef FINAL_ACT

		CLightSession					*operator = (IUnknown *pIUnk);

		// check that the session was enlisted if required and possible
		BOOL						CheckTransactionEnlistment(CLightDataSource *pDSO);
}; //CLightSession





// smart class for session object
#define APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)	\
	INIT_ACT(IUnknown)											\
	ACT(IAccessor)												\
	ACT(IColumnsInfo)											\
	ACT(IConvertType)											\
	ACT(IRowset)												\
	ACT(IRowsetInfo)											\
	ACT(IChapteredRowset)										\
	ACT(IColumnsInfo2)											\
	ACT(IColumnsRowset)											\
	ACT(IConnectionPointContainer)								\
	ACT(IDBAsynchStatus)										\
	ACT(IGetRow)												\
	ACT(IRowsetChange)											\
	ACT(IRowsetChapterMember)									\
	ACT(IRowsetCurrentIndex)									\
	ACT(IRowsetFind)											\
	ACT(IRowsetIdentity)										\
	ACT(IRowsetIndex)											\
	ACT(IRowsetLocate)											\
	ACT(IRowsetRefresh)											\
	ACT(IRowsetScroll)											\
	ACT(IRowsetUpdate)											\
	ACT(IRowsetView)											\
	FINAL_ACT(ISupportErrorInfo)					

class CLightRowset {
	protected:
		// define interfaces member data
		#define INIT_ACT(Interface)		Interface	*m_p##Interface;				
		#define	ACT(Interface)			INIT_ACT(Interface)
		#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

		APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

		#undef INIT_ACT
		#undef ACT
		#undef FINAL_ACT

		LONG						m_lRef;

		//accessor, bindings and data for the current row
		HACCESSOR					m_hAccessor;
		DBLENGTH					m_cRowSize;
		DBCOUNTITEM					m_cBindings;
		DBBINDING*					m_rgBindings;
		void*						m_pData;
		BOOL						m_fEmptyBuffer;

		// increment/decrement methods to keep track of the number of interfaces
		// kept on the object => used to make sure we have at lease one valid interface
		ULONG						AddRef() {
									  return InterlockedIncrement(&m_lRef);
		}

		ULONG						Release() {
									  return InterlockedDecrement(&m_lRef);
		}

		HRESULT						CacheInterface(
										REFIID		riid		// [in] Interface to be cached
									);

		IUnknown					*pIUnknown();
		// this is used to release memory for row data and bindings
		BOOL						ReleaseRowData();

		BOOL						FindBinding(DBCOUNTITEM cColumnOrdinal, DBCOUNTITEM *pcBinding);

	public:
		
									// create just the wrapper object
									CLightRowset()
									{
										#define INIT_ACT(Interface)		m_p##Interface = NULL;
										#define	ACT(Interface)			INIT_ACT(Interface)
										#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

												APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

										#undef INIT_ACT
										#undef ACT
										#undef FINAL_ACT

										m_lRef			= 0;
										m_hAccessor		= NULL;
										m_cRowSize		= 0;
										m_cBindings		= 0;
										m_rgBindings	= 0;
										m_pData			= NULL;
										m_fEmptyBuffer	= TRUE;
									}
									
									~CLightRowset() {
										ReleaseAll();
									}

		void						ReleaseAll(){
										#define INIT_ACT(Interface)						\
											SAFE_RELEASE(m_p##Interface);
										#define	ACT(Interface)			INIT_ACT(Interface)
										#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

												APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

										#undef INIT_ACT
										#undef ACT
										#undef FINAL_ACT
										m_lRef = 0;
										ReleaseRowData();
		}

		HRESULT						ReleaseInterface(REFIID riid);
		HRESULT						QueryInterface(REFIID riid, LPVOID *ppInterface); 

		HRESULT						Attach(
										REFIID		riid,		// [in] Interface to be cached
										IUnknown	*pIUnknown	// [in] Pointer to that interface
									);

		#define INIT_ACT(Interface)						\
			operator Interface* () {					\
				CacheInterface(IID_##Interface);		\
				return m_p##Interface;					\
			}
		#define	ACT(Interface)			INIT_ACT(Interface)
		#define FINAL_ACT(Interface)	INIT_ACT(Interface)							

				APPLY_ON_ROWSET_INTERFACES(INIT_ACT, ACT, FINAL_ACT)

		#undef INIT_ACT
		#undef ACT
		#undef FINAL_ACT

		CLightRowset					*operator = (IUnknown *pIUnk);


		// specific methods
		HRESULT							RestartPosition (HCHAPTER hChapter);
		
		// general GetNextRows and several consumer allocated polimorfic variations
		HRESULT							GetNextRows (
											HCHAPTER	hChapter,
											DBROWOFFSET	lRowsOffset,
											DBROWCOUNT	cRows,
											DBCOUNTITEM	*pcRowsObtained,
											HROW		**prghRows
										);
		inline HRESULT					GetNextRows (
											DBROWOFFSET	lRowsOffset,
											DBROWCOUNT	cRows,
											DBCOUNTITEM	*pcRowsObtained,
											HROW		*rghRows
										) {
											ASSERT(rghRows);
											return GetNextRows(DB_NULL_HCHAPTER, lRowsOffset, cRows, pcRowsObtained, &rghRows);
		}
		inline HRESULT					GetNextRows (
											DBROWOFFSET	lRowsOffset,
											DBROWCOUNT	cRows,
											HROW		*rghRows
										) {
											DBCOUNTITEM		cRowsObtained;
											ASSERT(rghRows);
											return GetNextRows(DB_NULL_HCHAPTER, lRowsOffset, cRows, &cRowsObtained, &rghRows);
		}
		inline HRESULT					GetNextRows (
											DBROWCOUNT	cRows,
											HROW		*rghRows
										) {
											DBCOUNTITEM		cRowsObtained;
											ASSERT(rghRows);
											return GetNextRows(DB_NULL_HCHAPTER, 0, cRows, &cRowsObtained, &rghRows);
		}
		inline HRESULT					GetNextRows (HROW *phRow) {
											DBCOUNTITEM		cRowsObtained;
											ASSERT(phRow);
											return GetNextRows(DB_NULL_HCHAPTER, 0, 1, &cRowsObtained, &phRow);
		}

		HRESULT							ReleaseRows(HROW hRow) {
												return ReleaseRows(1, &hRow);
		}
		HRESULT							ReleaseRows(
											DBCOUNTITEM	cRows, 
											HROW		*rghRow, 
											DBREFCOUNT	*rgRefCounts, 
											DBROWSTATUS	*rgRowStatus = NULL
										);
		HRESULT							ReleaseRows(
											DBCOUNTITEM	cRows, 
											HROW		*rghRow, 
											DBREFCOUNT	**prgRefCounts = NULL, 
											DBROWSTATUS	**prgRowStatus = NULL
										);

		HRESULT							GetData(HROW hRow, HACCESSOR hAccessor, void *pData);
		// this stores row data accordig to hRow and current accessor/bindings
		HRESULT							GetData(HROW hRow);


		DBSTATUS *GetColumnStatus(DBORDINAL cColumnOrdinal)
		{
			DBORDINAL	 cBinding;
			DBBINDING	*pBinding = NULL;

			TESTC(FindBinding(cColumnOrdinal, &cBinding));
			pBinding = &m_rgBindings[cBinding];
			if (STATUS_IS_BOUND(*pBinding))
				return (DBSTATUS*)((BYTE*)m_pData + pBinding->obStatus);
			else
		CLEANUP:
				return NULL;
		}

		LPVOID *GetColumnValue(DBORDINAL cColumnOrdinal)
		{
			DBORDINAL	cBinding;
			DBBINDING	*pBinding	= NULL;
			DBSTATUS	*pStatus	= NULL;

			TESTC(FindBinding(cColumnOrdinal, &cBinding));
			pBinding = &m_rgBindings[cBinding];
			
			if (STATUS_IS_BOUND(*pBinding))
				pStatus = (DBSTATUS*)((BYTE*)m_pData + pBinding->obStatus);

			if (VALUE_IS_BOUND(*pBinding) && (!pStatus || (DBSTATUS_S_OK == *pStatus)))
				return (LPVOID*)&VALUE_BINDING(m_rgBindings[cBinding], m_pData);
			else
		CLEANUP:
				return NULL;
		}

		DBBYTEOFFSET *GetColumnLength(DBORDINAL cColumnOrdinal)
		{
			DBORDINAL	cBinding;
			DBBINDING	*pBinding	= NULL;
			DBSTATUS	*pStatus	= NULL;

			TESTC(FindBinding(cColumnOrdinal, &cBinding));
			pBinding = &m_rgBindings[cBinding];
			
			if (STATUS_IS_BOUND(*pBinding))
				pStatus = (DBSTATUS*)((BYTE*)m_pData + pBinding->obStatus);

			if (LENGTH_IS_BOUND(*pBinding) && (!pStatus || (DBSTATUS_S_OK == *pStatus)))
				return (DBBYTEOFFSET*)((BYTE*)m_pData+pBinding->obLength);
			else
		CLEANUP:
				return NULL;
		} 


		HRESULT							GetBindings(
											HACCESSOR			hAccessor,
											DBACCESSORFLAGS		*pdwAccessorFlags,
											DBCOUNTITEM			*pcBindings,
											DBBINDING			**prgBindings
										);

		BOOL							SetDefaultAccessor(HACCESSOR hAccessor);

}; //CLightRowset





/////////////////////////////////////////////////////////////
//Stack Macros
//
// Example usage:
//	DBPROPSET propsetBar = {NULL, 0, GUID_MEMBERS(DBPROPSET_DBINTALL)}; 
//	DBID	  dbidNULL	 = {DBID_MEMBERS(DB_NULLID)};
//
#define GUID_MEMBERS(guid)		 guid.Data1		\
								,guid.Data2		\
								,guid.Data3		\
								,guid.Data4[0]	\
								,guid.Data4[1]	\
								,guid.Data4[2]	\
								,guid.Data4[3]	\
								,guid.Data4[4]	\
								,guid.Data4[5]	\
								,guid.Data4[6]	\
								,guid.Data4[7]

#endif //__CMYDSO_HPP__