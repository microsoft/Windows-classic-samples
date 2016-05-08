//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module TABLE.H
//
//-----------------------------------------------------------------------------
#ifndef _TABLE_H_
#define _TABLE_H_


/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "DataSource.h"
#include "TableCopy.h"



/////////////////////////////////////////////////////////////////
// Defines
//
/////////////////////////////////////////////////////////////////
enum ESQL_STMT
{
	ESQL_SELECT,
	ESQL_INSERT,
	ESQL_CREATE_TABLE,
	ESQL_DROP_TABLE
};


/////////////////////////////////////////////////////////////////
// Structs for Column/Index info
//
/////////////////////////////////////////////////////////////////
struct TYPEINFO
{
	WCHAR			wszTypeName[MAX_NAME_LEN];
	USHORT			wType;
	ULONG			ulColumnSize;
	WCHAR			wszCreateParams[MAX_NAME_LEN];
	VARIANT_BOOL	fIsNullable;
	VARIANT_BOOL	fIsAutoInc;
};


struct INDEXINFO
{
	//INDEXINFO
	WCHAR			wszIndexName[MAX_NAME_LEN];
	VARIANT_BOOL	fUnique;
	VARIANT_BOOL	fClustered;
	DBTYPE			wType;
	DWORD			dwFillFactor;
	DWORD			dwInitialSize;
	DWORD			dwNulls;
	VARIANT_BOOL	fSortBookmarks;
	VARIANT_BOOL	fAutoUpdate;
  	DWORD			dwNullCollation;
  	DWORD			iOrdinal;
	WCHAR			wszColName[MAX_NAME_LEN];
	DWORD			dwCollation;

	//PRIMARYKEY 
	BOOL			fIsPrimaryKey;
};


struct PRIMARYKEY
{
	WCHAR			wszColName[MAX_NAME_LEN];
};


struct TABLEINFO
{
	WCHAR			wszCatalogName[MAX_NAME_LEN];
	WCHAR			wszSchemaName[MAX_NAME_LEN];
	WCHAR			wszTableName[MAX_NAME_LEN];
	WCHAR			wszType[MAX_NAME_LEN];
};

struct COLDESC
{
	//DBCOLUMNINFO
	WCHAR			wszColName[MAX_NAME_LEN];
	DBORDINAL		iOrdinal;
	DBCOLUMNFLAGS	dwFlags;
	DBLENGTH		ulColumnSize;
	DBTYPE			wType;
	BYTE			bPrecision;
	BYTE			bScale;

	//TYPEINFO
	WCHAR			wszTypeName[MAX_NAME_LEN];
	ULONG			ulCreateParams;
	BOOL			fIsNullable;
	BOOL			fIsAutoInc;

	//INDEXINFO
	BOOL			fIsPrimaryKey;
};


struct BINDINGINFO
{
	HACCESSOR	hAccessor;
	ULONG		cBindings;
	DBBINDING*  rgBindings;
};



/////////////////////////////////////////////////////////////////
// CTable 
//
/////////////////////////////////////////////////////////////////
class CTable
{
public:
	//Constructors
	CTable(CWizard* pCWizard);
	virtual ~CTable();
	
	//Members
	virtual BOOL	Connect(HWND hWnd, CDataSource* pCDataSource = NULL);
	virtual BOOL	IsConnected();

	virtual BOOL	GetQuotedID(WCHAR* pwszOutBuff, size_t cwchOutBuff, WCHAR* pwszInBuff);
	virtual HRESULT GetColInfo(DWORD dwInsertOpt);	
	virtual HRESULT GetTypeInfo();
	virtual HRESULT GetLiteralInfo();
	virtual HRESULT GetTypeNameAndParams(ULONG iCol, WCHAR* pwszName, size_t cwchName);

	virtual HRESULT GetColumnDesc(DBCOLUMNDESC** prgColumnDesc);
	virtual HRESULT CreateSQLStmt(ESQL_STMT eSqlStmt, WCHAR* pwszSqlStmt, size_t cwchSqlStmt, BOOL fShowSql = TRUE);

	virtual HRESULT MapTableInfo(CTable* pCSourceTable);
	virtual HRESULT CreateTable();

	virtual HRESULT AdjustBindings(ULONG cBindings, DBBINDING* rgBindings, void* pData);

	virtual HRESULT GetRowset(DWORD dwInsertOpt);
	virtual HRESULT CreateAccessors(ULONG* pcBindingInfo, BINDINGINFO** prgBindingInfo, ULONG* pcRowSize, ULONG ulBlobSize, BOOL* pbOutofLine);

	virtual HRESULT CopyData(CTable* pCSourceTable, DBCOUNTITEM* pcRowsCopied);
	virtual HRESULT CopyIndexes(CTable* pCSourceTable);

	virtual HRESULT GetTypeInfoRowset(IAccessor** ppIAccessor, HACCESSOR* phAccessor, IRowset** ppIRowset);

	//Data
	WCHAR 		m_wszIDQuote[MAX_NAME_LEN];
	WCHAR 		m_wszIDSeperator[MAX_NAME_LEN];

	//TableInfo
	TABLEINFO	m_TableInfo;
	WCHAR		m_wszQualTableName[MAX_NAME_LEN];

	//Index info
	ULONG		m_cIndexes;			// Count of indexes
	INDEXINFO*	m_rgIndexInfo;		// Index information

	//ColumnInfo
	ULONG		m_cColumns;			// Count of columns
	COLDESC* 	m_rgColDesc;		// Column Information
	
	//DataSource
	CDataSource*	m_pCDataSource;
	CWizard*		m_pCWizard;

	//Rowset
	IAccessor*		m_pIAccessor;
	IRowset*		m_pIRowset;
};



///////////////////////////////////////////////////////////////////////////////
// Class CISeqStream
// 
// My implementation of ISeqStream interface
///////////////////////////////////////////////////////////////////////////////
class CISeqStream : public ISequentialStream
{
public:
	//Constructors
	CISeqStream();
	virtual ~CISeqStream();

	virtual BOOL Seek(ULONG iPos);
	virtual BOOL Clear();
	virtual BOOL CompareData(void* pBuffer);
	virtual ULONG Length()  { return m_cBufSize; };

	virtual operator void* const() { return m_pBuffer; };

	STDMETHODIMP_(ULONG)	AddRef(void);
	STDMETHODIMP_(ULONG)	Release(void);
	STDMETHODIMP QueryInterface(REFIID riid, LPVOID *ppv);
	
    STDMETHODIMP Read( 
            /* [out] */ void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbRead);
        
    STDMETHODIMP Write( 
            /* [in] */ const void __RPC_FAR *pv,
            /* [in] */ ULONG cb,
            /* [out] */ ULONG __RPC_FAR *pcbWritten);

	virtual HRESULT Write(ISequentialStream* pISeqStream, ULONG* pcbWritten);

protected:
	//Data

private:

	ULONG		m_cRef;			// reference count

	void*       m_pBuffer;		// buffer
	ULONG       m_cBufSize;     // buffer size
	ULONG       m_iPos;         // current index position in the buffer
};



#endif	//_TABLE_H_
