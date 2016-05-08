//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module ROW.H | ROW base object and contained interface
// definitions
//
//

#ifndef _ROW_H_
#define _ROW_H_


#include "baseobj.h"

// #defines ------------------------------------------------------------------

// number of extra columns supported on a row object
#define EXTRA_COLUMNS			1
// The default stream is the first extra column
#define DEFAULT_STREAM			0
#define DEFAULT_STREAM_ORDINAL	1

// Forward declarations ------------------------------------------------------

class CImpIRow;
class CImpIColumnsInfo;
class CImpIConvertType;
class CImpIGetSession;
class CImpIRowChange;

typedef CImpIRow*			PIMPIROW;
typedef CImpIColumnsInfo*	PIMPICOLUMNSINFO;
typedef CImpIConvertType*	PIMPCONVERTTYPE;
typedef	CImpIGetSession*	PIMPIGETSESSION;
typedef CImpIRowChange*		PIMPIROWCHANGE;


// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CRow | Row object. Containing class for all interfaces on the Row
// Object
//
class CRow : public CBaseObj					//@base public | CBaseObj
{
	//	Contained interfaces are friends
	friend class CImpIRow;
	friend class CImpIColumnsInfo;
	friend class CImpIConvertType;
	friend class CImpIGetSession;
	friend class CImpIRowChange;

	// Stream object interfaces are friends
	// since stream objects always have implicit rowset context
	friend class CStream;
	
	protected: //@access protected
		//@cmember parent Object (either Rowset or Session)
		CBaseObj *						m_pParentObj;
		//@cmember associated Row Handle
		HROW							m_hRow;
		//@cmember File Manipulation Class
		CFileIO *						m_pFileio;
		//@cmember Row buffer size
		DBLENGTH						m_cbRowSize;
		//@cmember ptr to the row buffer
		BYTE *							m_pRowBuff;
		//@cmember count of Extra columns
		DBORDINAL						m_cExtraCols;
		//@cmember array of Extra dbcolumninfo structs
		DBCOLUMNINFO					m_rgExtracolinfo[EXTRA_COLUMNS];

		// Interface and OLE Variables

		//@cmember Reference count
		DBREFCOUNT						m_cRef;												
		//@cmember Contained IRow
		PIMPIROW						m_pIRow;
		//@cmember Contained IColumnsInfo
		PIMPICOLUMNSINFO				m_pIColumnsInfo;				
		//@cmember Contained IConvertType
		PIMPICONVERTTYPE				m_pIConvertType;
		//@cmember Contained IGetSession
		PIMPIGETSESSION					m_pIGetSession;
		//@cmember Contained IRowChange
		PIMPIROWCHANGE					m_pIRowChange;

	public: //@access public
		//@cmember Constructor
		CRow(LPUNKNOWN);
		//@cmember Destructor
		virtual ~CRow(void);
	
		//@cmember Init row object that is a part of a rowset
		BOOL			FInit(CRowset*);
		//@cmember Init row object that is a result of a direct bind
		BOOL			FInit(CDBSession*, CFileIO*, DBCOUNTITEM);
		//@cmember Set the row object row handle value
		STDMETHODIMP	SetRowHandle(HROW);
		//@cmember Return m_pFileio
		CFileIO *		GetFileObj();
		//@cmember Return underlying ROWBUFF
		ROWBUFF *		GetRowBuff();
		//@cmember Return true if columnID is valid
		BOOL 			GetColumnOrdinal(CFileIO*, DBID*, DBORDINAL*);
		//@cmember Return m_cExtraCols
		inline DBORDINAL		GetExtraColCount() { return m_cExtraCols; };
		//@cmember Return 
		inline DBCOLUMNINFO *	GetExtraColumnInfo() { return m_rgExtracolinfo; };
		
		//	Object's base IUnknown
		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);		
};


//----------------------------------------------------------------------------
// @class CImpIRow | Contained IRow class
//
class CImpIRow : public IRow		//@base public | IRow
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRow)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRow, CImpIRow);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IRow members
		//@cmember GetColumns Method
		STDMETHODIMP GetColumns(DBORDINAL cColumns, DBCOLUMNACCESS rgColumns[]);

		//@cmember GetSourceRowset Method
		STDMETHODIMP GetSourceRowset(REFIID riid, IUnknown ** ppRowset, HROW * phRow);

		//@cmember Open Method
		STDMETHODIMP Open
					(
						IUnknown *	pIUnkOuter,
						DBID *		pColumnID,
						REFGUID		rguidColumnType,
						DWORD		dwBindFlags, 
						REFIID		riid,
						IUnknown ** ppUnk
					);
};


//----------------------------------------------------------------------------
// @class CImpIGetSession | Contained IGetSession class
//
class CImpIGetSession : public IGetSession 		//@base public | IGetSession
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRow)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRow, CImpIGetSession);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IGetSession members
		//@cmember GetSessionMethod
        STDMETHODIMP	GetSession(REFIID riid, IUnknown ** ppSession);
};


//----------------------------------------------------------------------------
// @class CImpIRowChange | Contained IRowChange class
//
class CImpIRowChange : public IRowChange		//@base public | IRowChange
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRow)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRow, CImpIRowChange);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		// IRowChange members
		//@cmember SetColumns method
		STDMETHODIMP SetColumns(DBORDINAL cColumns, DBCOLUMNACCESS rgColumns[]);
};


#endif //_ROW_H_

