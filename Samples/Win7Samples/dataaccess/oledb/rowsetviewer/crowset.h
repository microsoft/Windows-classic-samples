//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module CROWSET.H
//
//-----------------------------------------------------------------------------------

#ifndef _CROWSET_H_
#define _CROWSET_H_


//////////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////////
enum BINDCOLS
{
	BIND_ALLCOLS				= 0x00000001,
	BIND_ALLCOLSEXPECTBOOKMARK	= 0x00000002,
	BIND_UPDATEABLECOLS			= 0x00000004
};


//////////////////////////////////////////////////////////////////////////////
// Forwards
//
//////////////////////////////////////////////////////////////////////////////
class CMainWindow;
class CMDIChild;


/////////////////////////////////////////////////////////////////
// CBindings class
//
/////////////////////////////////////////////////////////////////
class CBindings	: public CVector<DBBINDING, DBCOUNTITEM>
{
public:
	//Constructor
	CBindings()	{}
	~CBindings() {}

	//Overload
	void		RemoveAll()
	{
		//Overloaded - inorder to first free out-of-line data
		::FreeBindings(&m_cElements, &m_rgElements);

		//Now - Delegate
		CVector<DBBINDING, DBCOUNTITEM>::RemoveAll();
	}

	HRESULT		FreeData(void* pv, BOOL fSetData = FALSE)
	{
		//Delegate
		return ::FreeBindingData(m_cElements, m_rgElements, pv, fSetData);
	}

	const DBBINDING*	GetOrdinal(DBORDINAL iOrdinal)
	{
		//Due to the bookmark column being present or absent causes the Binding
		//to not be lined exactly (1:1) to the Ordinal position...

		//no-ops
		if(!GetCount() || !GetElements() || (iOrdinal==0 && GetElement(0).iOrdinal!=0))
			return NULL;
		
		//Bookmark
		if(GetElement(0).iOrdinal == 0)
		{
			//[0,1,2,3,4,5...]
			ASSERT(GetElement(iOrdinal).iOrdinal == iOrdinal);
			return &GetElement(iOrdinal);
		}
		else
		{
			//[1,2,3,4,5...]
			ASSERT(GetElement(iOrdinal-1).iOrdinal == iOrdinal);
			return &GetElement(iOrdinal-1);
		}
		
		return NULL;
	}
	
protected:
	//Data
};


/////////////////////////////////////////////////////////////////
// CColumnInfo class
//
/////////////////////////////////////////////////////////////////
class CColumnInfo	: public CVector<DBCOLUMNINFO, DBORDINAL>
{
public:
	//Constructor
	CColumnInfo()	
	{
		m_cHiddenColumns	= 0;
	}
	~CColumnInfo() 
	{
	}

	//Overload
	void		RemoveAll()
	{
		//Overloaded - inorder to first free out-of-line data
		m_cstrStringBuffer.Empty();
		m_cHiddenColumns	= 0;

		//Now - Delegate
		CVector<DBCOLUMNINFO, DBORDINAL>::RemoveAll();
	}

	//Overload
	void		Attach(DBORDINAL cColumnInfo, DBCOLUMNINFO* rgColumnInfo, WCHAR* pwszStringBuffer, DBORDINAL cHiddenColumns = 0)
	{
		//First - Delegate
		//NOTE: We have to do this first, since this method call RemoveAll, which will empty
		//anything we do in this class, since its virtual...
		CVector<DBCOLUMNINFO, DBORDINAL>::Attach(cColumnInfo, rgColumnInfo);

		//Overloaded - inorder to first obtain other items
		m_cstrStringBuffer.Attach(pwszStringBuffer);
		m_cHiddenColumns	= cHiddenColumns;

	}

	BOOL		IsHidden(DBORDINAL iOrdinal)
	{
		//Hidden Column
		if(m_cHiddenColumns)
		{
			//cColInfo already includes cHiddenColumns, this...
			ASSERT(m_cHiddenColumns <= m_cElements);

			//Is this a hidden column
			//If this ordinal is >= to the first hidden column then its hidden...
			if(iOrdinal >= m_rgElements[m_cElements - m_cHiddenColumns].iOrdinal)
				return TRUE;
		}

		return FALSE;

	}
	
	const DBCOLUMNINFO*	GetOrdinal(DBORDINAL iOrdinal)
	{
		//Due to the bookmark column being present or absent causes the Binding
		//to not be lined exactly (1:1) to the Ordinal position...

		//no-ops
		if(!GetCount() || !GetElements() || (iOrdinal==0 && GetElement(0).iOrdinal!=0))
			return NULL;
		
		//Bookmark
		if(GetElement(0).iOrdinal == 0)
		{
			//[0,1,2,3,4,5...]
			ASSERT(GetElement(iOrdinal).iOrdinal == iOrdinal);
			return &GetElement(iOrdinal);
		}
		else
		{
			//[1,2,3,4,5...]
			ASSERT(GetElement(iOrdinal-1).iOrdinal == iOrdinal);
			return &GetElement(iOrdinal-1);
		}
		
		return NULL;
	}
	
protected:
	//Data
	CComWSTR	m_cstrStringBuffer;
	DBORDINAL	m_cHiddenColumns;	//DBPROP_HIDDENCOLUMNS
};


////////////////////////////////////////////////////////////////
// CDataAccess
//
/////////////////////////////////////////////////////////////////
class CDataAccess : public CAsynchBase
{
public:
	//Constructors
	CDataAccess(SOURCE eObjectType, CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CDataAccess();
	
	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);

	//Members
	virtual HRESULT			GetColInfo();
	virtual HRESULT			GetColInfo(DBORDINAL* pcColumns, DBCOLUMNINFO** prgColumnInfo, WCHAR** ppStringBuffer, DBORDINAL* pcHiddenColumns);
	virtual INT				GetColumnImage(const DBCOLUMNINFO* pColInfo, DBSTATUS dbStatus = DBSTATUS_S_OK);

	virtual HRESULT			ValidateAccessor(HACCESSOR hAccessor, ULONG ulRefCount = 1);
	virtual HRESULT			SetColumnData(const DBBINDING* pBinding, void* pData, DBSTATUS   dbStatus, DBLENGTH   dbLength, WCHAR* pwszValue, DWORD dwFlags, DBTYPE wBackendType);
	virtual HRESULT			GetColumnData(const DBBINDING* pBinding, void* pData, DBSTATUS* pdbStatus, DBLENGTH* pdbLength, DBTYPE* pwSubType, WCHAR* pwszValue, ULONG ulMaxSize, DWORD dwFlags, DBTYPE wBackendType);

	virtual HRESULT			CreateAccessors(BINDCOLS eBindCols);
	virtual HRESULT			CreateAccessor(DBACCESSORFLAGS dwAccessorFlags, DBCOUNTITEM cBindings, const DBBINDING* rgBindings, DBLENGTH cRowSize, HACCESSOR* phAccessor);
	virtual HRESULT			SetupBindings(BINDCOLS eBindCols, DBCOUNTITEM* cBindings, DBBINDING** prgBindings, DBLENGTH* pcRowSize = NULL);

	virtual HRESULT			AddRefAccessor(HACCESSOR hAccessor);
	virtual HRESULT			ReleaseAccessor(HACCESSOR* phAccessor, BOOL fReleaseAlways = FALSE);
	virtual HRESULT			GetColumnsRowset(CAggregate* pCAggregate, bool fOptColumns, REFIID riid, ULONG cPropSets, DBPROPSET* rgPropSets, IUnknown** ppIUnknown);

	//Rowset
	//[MANADATORY]
	IAccessor*					m_pIAccessor;
	IColumnsInfo*				m_pIColumnsInfo;
	IConvertType*				m_pIConvertType;

	//[OPTIONAL]
	IColumnsRowset*				m_pIColumnsRowset;

	//ColInfo
	CColumnInfo					m_ColumnInfo;

	//Accessor
	CBindings					m_Bindings;
	HACCESSOR					m_hAccessor;
	DBLENGTH					m_cbRowSize;
	void*						m_pData;
	
	//Bookmark Accessor
	HACCESSOR					m_hBmkAccessor;
	BOOL						m_bSchemaRowset;
};


////////////////////////////////////////////////////////////////
// CRowset 
//
/////////////////////////////////////////////////////////////////
class CRowset : public CDataAccess
{
public:
	//Constructors
	CRowset(CMainWindow* pCMainWindow, CMDIChild* pCMDIChild = NULL);
	virtual ~CRowset();
	
	//IUnknown Helpers
	virtual HRESULT			AutoQI(DWORD dwCreateOpts);
	virtual HRESULT			AutoRelease();
	virtual IUnknown**		GetInterfaceAddress(REFIID riid);
 	
	//Pure Virtual
	virtual WCHAR*			GetObjectName()			{ return m_hChapter ? L"Chapter" : L"Rowset";		} 
	virtual UINT			GetObjectMenu()			{ return IDM_ROWSETMENU;							}
	virtual LONG			GetObjectImage()		{ return m_hChapter ? IMAGE_CHAPTER : IMAGE_TABLE;	}
	virtual REFIID			GetDefaultInterface()	{ return IID_IRowset;								}

	virtual WCHAR*			GetObjectDesc();
	virtual HRESULT			DisplayObject();

	//Members
	virtual HRESULT			ValidateRow(HROW hRow, ULONG ulRefCount = 1);

	virtual HRESULT			GetData(HROW hRow, HACCESSOR hAccessor = NULL, void* pData = NULL, DBPROPID dwSourceID = DBPROP_IRowset);
	virtual HRESULT			GetBookmark(HROW hRow, DBBKMARK* pcbBookmark, BYTE** ppBookmark);
	virtual HRESULT			GetChapter(HROW hRow, DBORDINAL iOrdinal, HCHAPTER* phChapter);

	virtual HRESULT			RestartPosition();
	virtual HRESULT			GetNextRows(DBROWOFFSET lOffset, DBROWCOUNT cRows, DBCOUNTITEM* pcRowsObtained, HROW** prghRows);
	virtual HRESULT			AddRefRows(DBROWCOUNT cRows, HROW* rghRows, ULONG* rgRefCounts = NULL);
	virtual HRESULT			ReleaseRows(DBROWCOUNT cRows, HROW* rghRows, ULONG* rgRefCounts = NULL);
	virtual HRESULT			GetRowFromHROW(CAggregate* pCAggregate, HROW hRow, REFIID riid, IUnknown** ppIUnknown);


	//Rowset
	//[MANADATORY]
	IRowset*					m_pIRowset;
	IRowsetInfo*				m_pIRowsetInfo;

	//[OPTIONAL]
	IRowsetChange*				m_pIRowsetChange;
	IRowsetIdentity*			m_pIRowsetIdentity;
	IRowsetLocate*				m_pIRowsetLocate;
	IRowsetFind*				m_pIRowsetFind;
	IRowsetView*				m_pIRowsetView;
	IChapteredRowset*			m_pIChapteredRowset;
	IRowsetResynch*				m_pIRowsetResynch;
	IRowsetRefresh*				m_pIRowsetRefresh;
	IRowsetIndex*				m_pIRowsetIndex;
	IRowsetScroll*				m_pIRowsetScroll;
	IRowsetUpdate*				m_pIRowsetUpdate;
	IRowsetBookmark*			m_pIRowsetBookmark;
	IGetRow*					m_pIGetRow;

	//Notifications
	DWORD						m_dwCookieRowsetNotify;

	//Chapters
	HCHAPTER					m_hChapter;

	//Properties
	BOOL						m_fRemoveDeleted;
};





#endif //_CROWSET_H_
