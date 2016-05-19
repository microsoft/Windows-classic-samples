//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc 
//
// @module ROWSET.H | CRowset base object and contained interface
// definitions
//
//
#ifndef _ROWSET_H_
#define _ROWSET_H_


#include "fileio.h"
#include "bitarray.h"
#include "extbuff.h"
#include "hashtbl.h"
#include "command.h"
#include "baseobj.h"

#include "dbsess.h"

// Forward declarations ------------------------------------------------------

class CImpIRowset;
class CImpIRowsetChange;
class CImpIColumnsInfo;
class CImpIAccessor;
class CImpIRowsetInfo;
class CImpIRowsetIdentity;
class CImpIConvertType;
class CImpIGetRow;

typedef CImpIRowset*		PIMPIROWSET;
typedef CImpIRowsetChange*	PIMPIROWSETCHANGE;
typedef CImpIColumnsInfo*	PIMPICOLUMNSINFO;
typedef CImpIAccessor *		PIMPIACCESSOR;
typedef CImpIRowsetIdentity*PIMPIROWSETIDENTITY;
typedef CImpIRowsetInfo*	PIMPIROWSETINFO;
typedef CImpIConvertType*	PIMPICONVERTTYPE;
typedef CImpIGetRow*		PIMPIGETROW;

// General Helper Function
HRESULT GetInternalTypeFromCSVType(SWORD swDataType, BOOL fIsSigned, DWORD* pdwdbType);


// Classes -------------------------------------------------------------------

//----------------------------------------------------------------------------
// @class CRowset | Rowset object. Containing class for all interfaces on the Rowset 
// Object
//
class CRowset : public CBaseObj					//@base public | CBaseObj
{
	//	Contained interfaces are friends
	friend class CImpIColumnsInfo;
	friend class CImpIRowset;
	friend class CImpIRowsetChange;
	friend class CImpIAccessor;
	friend class CImpIRowsetIdentity;
	friend class CImpIRowsetInfo;
	friend class CImpIConvertType;
	friend class CImpIGetRow;

	// Row object interfaces are friends
	// since row object can have implicit rowset context
	friend class CRow;
	friend class CImpIRow;
	friend class CImpIGetSession;
	friend class CImpIRowChange;

	private: //@access private
		//@cmember Creates Helper Classes 
		HRESULT CreateHelperFunctions(void);
		//@cmember Returns the Buffer Pointer for the specified row
		ROWBUFF* GetRowBuff(DBCOUNTITEM iRow, BOOL fDataLocation = FALSE);
		//@cmember Establishes the data area bindings
		HRESULT Rebind(BYTE* pBase);
		//@cmember Establishes the data area bindings
		BOOL SupportIRowsetChange();

	protected: //@access protected
		//@cmember File Manipulation Class
		CFileIO*						m_pFileio;
		//@cmember Count of Columns in Result Set
		DBORDINAL						m_cCols;			
		//@cmember array of accessor ptrs
		LPEXTBUFFER     				m_pextbufferAccessor;
		//@cmember internal buffer structure
		PLSTSLOT        				m_pIBuffer;         
		//@cmember bit array to mark active rows
		LPBITARRAY						m_prowbitsIBuffer;	
		//@cmember size of row data in the buffer
		DBLENGTH           				m_cbRowSize;        
		//@cmember size of row in the buffer
		ULONG           				m_cbTotalRowSize;        
		//@cmember points to the first buffered row 
		BYTE*							m_rgbRowData;      
		//@cmember index of the first available rowbuffer
		ULONG							m_irowMin;          
		//@cmember current # of rows in the buffer
		DBCOUNTITEM        				m_cRows;
		//@cmember position in the resultset
		DBCOUNTITEM        				m_irowFilePos;
		//@cmember Start of the rowset
		DBCOUNTITEM						m_irowLastFilePos;
		//@cmember status word for the entire cursor
		UDWORD          				m_dwStatus;         
		//@cmember remember last binding location
		BYTE*							m_pLastBindBase;
		//@cmember RefCount of all outstanding row handles
		DBREFCOUNT         				m_ulRowRefCount;	
        //@cmember Object that created this rowset
		CBaseObj*						m_pParentObj;
		//@member Utility object to manage properties
		PCUTILPROP						m_pUtilProp;
		//@cmember File Path Name
		WCHAR							m_wszFilePath[MAX_PATH];
		//@cmember Data Source Path Name
		WCHAR							m_wszDataSourcePath[MAX_PATH];
		                       
		// Interface and OLE Variables

		//@cmember Reference count
		DBREFCOUNT						m_cRef;												
		//@cmember Contained IColumnsInfo
		PIMPICOLUMNSINFO				m_pIColumnsInfo;				
		//@cmember Contained IRowset
		PIMPIROWSET						m_pIRowset;
		//@cmember Contained IRowsetChange
		PIMPIROWSETCHANGE				m_pIRowsetChange;				
		//@cmember Contained IAccessor
		PIMPIACCESSOR					m_pIAccessor;
		//@cmember Contained IRowsetIdentity
		PIMPIROWSETIDENTITY				m_pIRowsetIdentity;
		//@cmember Contained IRowsetInfo
		PIMPIROWSETINFO					m_pIRowsetInfo;
		//@cmember Contained IConvertType
		PIMPICONVERTTYPE				m_pIConvertType;
		//@cmember Contained IGetRow
		PIMPIGETROW						m_pIGetRow;

	public: //@access public
		//@cmember Constructor
		 CRowset(LPUNKNOWN);
		//@cmember Destructor
		~CRowset(void);

		//@cmember Intitialization Routine
		BOOL FInit(CFileIO*, CBaseObj*, WCHAR* pwszFileName, WCHAR* pwszDataSource);			
		//@cmember Return the CUtilProp object
		inline PCUTILPROP GetCUtilProp() { return m_pUtilProp; };
		//@cmember Return m_pFileio
		inline CFileIO* GetFileObj() { return m_pFileio; };

		//	Object's base IUnknown
		//@cmember Request an Interface
		STDMETHODIMP				QueryInterface(REFIID, LPVOID *);
		//@cmember Increments the Reference count
		STDMETHODIMP_(DBREFCOUNT)	AddRef(void);
		//@cmember Decrements the Reference count
		STDMETHODIMP_(DBREFCOUNT)	Release(void);

		//Back pointer to a parent cmd object. 
		PCCOMMAND				m_pCreator;  
};

typedef CRowset *PCROWSET;


//----------------------------------------------------------------------------
// @class CImpIRowset | Contained IRowset class
//
class CImpIRowset : public IRowset		//@base public | IRowset
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRowset)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRowset, CImpIRowset);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IRowset members
		//@cmember GetData Method
		STDMETHODIMP	GetData(HROW, HACCESSOR, void*);
		//@cmember GetNextRows Method
		STDMETHODIMP	GetNextRows(HCHAPTER, DBROWOFFSET, DBROWCOUNT, DBCOUNTITEM*, HROW**);
        //cmember ReleaseRows method
		STDMETHODIMP	ReleaseRows(DBCOUNTITEM, const HROW rghRows[], DBROWOPTIONS rgRowOptions[], DBREFCOUNT rgRefCounts[], DBROWSTATUS rgRowStatus[]);
        //@cmember RestartPosition method
		STDMETHODIMP	RestartPosition(HCHAPTER);
        //@cmember AddRefRows method
        STDMETHODIMP	AddRefRows(DBCOUNTITEM, const HROW rghRows[], DBREFCOUNT rgRefCounts[], DBROWSTATUS rgRowStatus[]);
};


//----------------------------------------------------------------------------
// @class CImpIRowsetChange | Contained IRowsetChange class
//
class CImpIRowsetChange : public IRowsetChange	//@base public | IRowsetChange
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRowset)

	public: //@access public
		
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRowset, CImpIRowsetChange);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IRowsetChange members
		//@cmember SetData Method
	    STDMETHODIMP	SetData(HROW, HACCESSOR, void*);
	    STDMETHODIMP	DeleteRows(HCHAPTER, DBCOUNTITEM, const HROW rghRows[], DBROWSTATUS rgRowStatus[]);
		STDMETHODIMP    InsertRow(HCHAPTER hChapter,HACCESSOR hAccessor, void* pData, HROW* phRow);
};


//----------------------------------------------------------------------------
// @class CImpIColumnsInfo | Contained IColumnsInfo class
//
class CImpIColumnsInfo : public IColumnsInfo 		//@base public | IColumnsInfo
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CBaseObj)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CBaseObj, CImpIColumnsInfo);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IColumnsInfo members
		//@cmember GetColumnInfo method
	    STDMETHODIMP	GetColumnInfo(DBORDINAL*, DBCOLUMNINFO**, WCHAR**);
		//@cmember MapColumnIDs
		STDMETHODIMP	MapColumnIDs(DBORDINAL, const DBID rgColumnIDs[], DBORDINAL rgColumns[]);
};



//----------------------------------------------------------------------------
// @class CImpIRowsetInfo | Contained IRowsetInfo class
//
class CImpIRowsetInfo : public IRowsetInfo 		//@base public | IRowsetInfo
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRowset)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRowset, CImpIRowsetInfo);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//@cmember GetReferencedRowset
		STDMETHODIMP			GetReferencedRowset
			(
				DBORDINAL	iOrdinal, 
				REFIID		rrid,
				IUnknown**	ppReferencedRowset
			);

		//@cmember GetProperties
		STDMETHODIMP			GetProperties
		    (
			    const ULONG			cPropertySets,
			    const DBPROPIDSET	rgPropertySets[],
			    ULONG*              pcProperties,
			    DBPROPSET**			prgProperties
		    );

		//@cmember GetSpecification Method
		STDMETHODIMP			GetSpecification(REFIID, IUnknown**);
};



//----------------------------------------------------------------------------
// @class CImpIRowsetIdentity | Contained IRowsetIdentity class
//
class CImpIRowsetIdentity : public IRowsetIdentity 		//@base public | IRowsetIdentity
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRowset)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRowset, CImpIRowsetIdentity);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//@cmember IsSameRow
		STDMETHODIMP	IsSameRow
			(
				HROW hThisRow, 
				HROW hThatRow
			);
};



//----------------------------------------------------------------------------
// @class CImpIConvertType | Contained IConvertType class
//
class CImpIConvertType : public IConvertType 		//@base public | IConvertType
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CBaseObj)

	public: //@access public
 		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CBaseObj, CImpIConvertType);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//@cmember CanConvert
		STDMETHODIMP CImpIConvertType::CanConvert
			(
				DBTYPE			wFromType,		//@parm IN | src type
				DBTYPE			wToType,		//@parm IN | dst type
				DBCONVERTFLAGS	dwConvertFlags	//@parm IN | conversion flags
			);
};



//----------------------------------------------------------------------------
// @class CImpIGetRow | Contained IGetRow class
//
class CImpIGetRow : public IGetRow		//@base public | IGetRow
{
	private: //@access private
		DEFINE_DEFAULT_IUNKNOWN_MEMBER_DATA(CRowset)

	public: //@access public
		DEFINE_DEFAULT_IUNKNOWN_CTOR_DTOR(CRowset, CImpIGetRow);
		DEFINE_DEFAULT_IUNKNOWN_ADDREF_RELEASE

		//	IGetRow members
		//@cmember GetRowFromHROW Method
		STDMETHODIMP GetRowFromHROW
			(
				IUnknown *	pIUnkOuter,
				HROW		hRow,
				REFIID		riid,
				IUnknown **	ppUnk
			);

		//@cmember GetURLFromHROW Method
		STDMETHODIMP GetURLFromHROW
			(
				HROW		hRow,
				LPOLESTR *	ppwszURL
			);
};


#endif

