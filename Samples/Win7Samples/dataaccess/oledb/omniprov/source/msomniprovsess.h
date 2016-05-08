// Start of file MSOmniProvSession.h
// File : MSOmniProvSession.h
// All of the Schema rowsets object contain hard-coded schema information
//
// Implementation of:
//		CMSOmniProvSession - Session class,
//      CMSOmniProvSessionTRSchemaRowset - DBSCHEMA_TABLES schema rowset, 
//      CMSOmniProvSessionColSchemaRowset - DBSCHEMA_COLUMNS schema rowset, 
//		CMSOmniProvSessionPTSchemaRowset - DBSCHEMA_PROVIDER_TYPES schema rowset, 
//

#ifndef __CMSOmniProvSession_H_
#define __CMSOmniProvSession_H_

#include "resource.h"       // main symbols
#include "MSOmniProvRS.h"

class CMSOmniProvSessionTRSchemaRowset;
class CMSOmniProvSessionColSchemaRowset;
class CMSOmniProvSessionPTSchemaRowset;


// CMSOmniProvSession
class ATL_NO_VTABLE CMSOmniProvSession : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public IGetDataSourceImpl<CMSOmniProvSession>,
	public IOpenRowsetImpl<CMSOmniProvSession>,
	public ISessionPropertiesImpl<CMSOmniProvSession>,
	public IObjectWithSiteSessionImpl<CMSOmniProvSession>,
	public IDBSchemaRowsetImpl<CMSOmniProvSession>,
	public IDBCreateCommandImpl<CMSOmniProvSession, CMSOmniProvCommand>
{
public:

	CMSOmniProvSession()
	{
	}
	HRESULT FinalConstruct()
	{
		return FInit();
	}

	STDMETHOD(OpenRowset)(IUnknown *pUnk, DBID *pTID, DBID *pInID, REFIID riid,
					   ULONG cSets, DBPROPSET rgSets[], IUnknown **ppRowset)
	{
		CMSOmniProvRowset* pRowset;
		return  CreateRowset(pUnk, pTID, pInID, riid, cSets, rgSets, ppRowset, pRowset);
	}


BEGIN_PROPSET_MAP(CMSOmniProvSession)
	BEGIN_PROPERTY_SET(DBPROPSET_SESSION)
		PROPERTY_INFO_ENTRY(SESS_AUTOCOMMITISOLEVELS)
	END_PROPERTY_SET(DBPROPSET_SESSION)
END_PROPSET_MAP()
BEGIN_COM_MAP(CMSOmniProvSession)
	COM_INTERFACE_ENTRY(IGetDataSource)
	COM_INTERFACE_ENTRY(IOpenRowset)
	COM_INTERFACE_ENTRY(ISessionProperties)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IDBCreateCommand)
	COM_INTERFACE_ENTRY(IDBSchemaRowset)
END_COM_MAP()
BEGIN_SCHEMA_MAP(CMSOmniProvSession)
	SCHEMA_ENTRY(DBSCHEMA_TABLES, CMSOmniProvSessionTRSchemaRowset)
	SCHEMA_ENTRY(DBSCHEMA_COLUMNS, CMSOmniProvSessionColSchemaRowset)
	SCHEMA_ENTRY(DBSCHEMA_PROVIDER_TYPES, CMSOmniProvSessionPTSchemaRowset)  // Base data-types supported
END_SCHEMA_MAP()
};

//      CMSOmniProvSessionTRSchemaRowset - DBSCHEMA_TABLES schema rowset, 
class CMSOmniProvSessionTRSchemaRowset : 
	public CRowsetImpl< CMSOmniProvSessionTRSchemaRowset, CTABLESRow, CMSOmniProvSession,CAtlArray<CTABLESRow> >
{
public:
	HRESULT Execute(DBROWCOUNT* pcRowsAffected, ULONG, const VARIANT*)
	{
		USES_CONVERSION;
		// Currently Hardcoded later on get it from the CStorageClass - Catalog, Schema,Name,Type
		CTABLESRow trData;
		wcscpy_s(trData.m_szSchema, _countof(trData.m_szSchema), OLESTR("OmniProv Schema"));
		wcscpy_s(trData.m_szCatalog, _countof(trData.m_szCatalog), OLESTR("OmniProv Catalog"));
		wcsncpy_s(trData.m_szTable, _countof(trData.m_szTable), OLESTR("OmniTable"), SIZEOF_MEMBER(CTABLESRow, m_szTable));
		wcscpy_s(trData.m_szType, _countof(trData.m_szType), OLESTR("TABLE"));
		if (!m_rgRowData.Add(trData))
			return E_OUTOFMEMORY;
		*pcRowsAffected = 1;
		return S_OK;
	}
	DBSTATUS GetDBStatus(CSimpleRow*, ATLCOLUMNINFO* pInfo)
	{
		if (pInfo->iOrdinal < 4)
			return DBSTATUS_S_OK;
		else
			return DBSTATUS_S_ISNULL;
	}
};

//      CMSOmniProvSessionColSchemaRowset - DBSCHEMA_COLUMNS schema rowset, 
class CMSOmniProvSessionColSchemaRowset : 
	public CRowsetImpl< CMSOmniProvSessionColSchemaRowset, CCOLUMNSRow, CMSOmniProvSession,CAtlArray<CCOLUMNSRow> >
{
	double		m_double;
	HCHAPTER	m_hchapter;

public:
	HRESULT Execute(DBROWCOUNT* pcRowsAffected, ULONG, const VARIANT*)
	{
		USES_CONVERSION;
		// Currently Hardcoded later on get it from the CStorageClass - Catalog, Schema,Name,Type
		CCOLUMNSRow trData[4];   // 4 columns
		long l;
		for(l=0; l<4;l++)
		{
			wcscpy_s(trData[l].m_szTableCatalog, _countof(trData[l].m_szTableCatalog), OLESTR("OmniProv Catalog"));
			wcscpy_s(trData[l].m_szTableSchema, _countof(trData[l].m_szTableSchema), OLESTR("OmniProv Schema"));
			wcscpy_s(trData[l].m_szTableName, _countof(trData[l].m_szTableName), OLESTR("OmniTable"));
			trData[l].m_ulOrdinalPosition = l+1;
			trData[l].m_bIsNullable = VARIANT_FALSE;
			trData[l].m_bColumnHasDefault = VARIANT_TRUE;
			trData[l].m_ulColumnFlags = DBCOLUMNFLAGS_WRITE;  // To write into the columnss
			// Currently all the data is static 
			switch(l)
			{
			case 0:
				trData[l].m_nDataType = DBTYPE_STR;
				trData[l].m_ulCharMaxLength = 10;
				wcscpy_s(trData[l].m_szColumnName, _countof(trData[l].m_szColumnName), OLESTR("CustomerID"));
				break;
			case 1:
				trData[l].m_nDataType = DBTYPE_STR;
				trData[l].m_ulCharMaxLength = 40;
				wcscpy_s(trData[l].m_szColumnName, _countof(trData[l].m_szColumnName), OLESTR("CustomerName"));
				break;
			case 2:
				trData[l].m_nDataType = DBTYPE_STR;
				trData[l].m_ulCharMaxLength = 40;
				wcscpy_s(trData[l].m_szColumnName, _countof(trData[l].m_szColumnName), OLESTR("ItemName "));
				break;
			case 3: // The Long
				trData[l].m_nDataType = DBTYPE_I4;
				trData[l].m_nNumericPrecision = 10;
				wcscpy_s(trData[l].m_szColumnName, _countof(trData[l].m_szColumnName),OLESTR("Sale"));
			}
			m_rgRowData.Add(trData[l]);
		}
		*pcRowsAffected = l+1;
		return S_OK;
	}
	DBSTATUS GetDBStatus(CSimpleRow*, ATLCOLUMNINFO* pInfo)
	{
		switch(pInfo->iOrdinal)
		{
		case 1:
		case 2:
		case 3:
		case 19:
		case 20:
		case 22:
		case 23:
		case 25:
		case 26:
			return DBSTATUS_S_ISNULL;
		default:
			return DBSTATUS_S_OK;
		}
	}
};

//		CMSOmniProvSessionPTSchemaRowset - DBSCHEMA_PROVIDER_TYPES schema rowset, 
class CMSOmniProvSessionPTSchemaRowset : 
	public CRowsetImpl< CMSOmniProvSessionPTSchemaRowset, CPROVIDER_TYPERow, CMSOmniProvSession,CAtlArray<CPROVIDER_TYPERow> >
{
public:
		// Supports
		// char - DBTYPE_STR
	   //  long	- DBTYPE_I4
	HRESULT Execute(DBROWCOUNT* pcRowsAffected, ULONG, const VARIANT*)
	{
		USES_CONVERSION;
		CPROVIDER_TYPERow trData[2];

		// DBTYPE_STR
		wcscpy_s(trData[0].m_szName, _countof(trData[0].m_szName), OLESTR("DBTYPE_STR"));
		trData[0].m_nType = DBTYPE_STR;
		trData[0].m_bIsLong = VARIANT_FALSE;
		trData[0].m_bIsNullable = VARIANT_FALSE;
		trData[0].m_bCaseSensitive = VARIANT_TRUE;
		m_rgRowData.Add(trData[0]);

		// DBTYPE_I4
		wcscpy_s(trData[1].m_szName, _countof(trData[1].m_szName), OLESTR("DBTYPE_I4"));
		trData[1].m_nType = DBTYPE_I4;
		trData[1].m_bIsLong = VARIANT_TRUE;
		trData[1].m_bIsNullable = VARIANT_FALSE;
		m_rgRowData.Add(trData[1]);

		*pcRowsAffected = 2;
		return S_OK;
	}
};
#endif //__CMSOmniProvSession_H_

// End of file MSOmniProvSession.h