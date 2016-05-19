//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module CCol Header Module | This module contains definition information
// for CCol class for the private library.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 09-01-95	Microsoft	Code review update <nl>
//	[03] 10-01-95	Microsoft	Change to WCHAR * <nl>
//	[04] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CCol Elements|
//
// @subindex CCol
//
//---------------------------------------------------------------------------

#ifndef _CCol_HPP_
#define _CCol_HPP_


///////////////////////////////////////////////////////////////////
// Includes 
//
///////////////////////////////////////////////////////////////////
#include <cguid.h>	//GUID_NULL

///////////////////////////////////////////////////////////////////
// Consts 
//
///////////////////////////////////////////////////////////////////
const DBTYPE g_rgDefaultSubTypes[] = 
{
	DBTYPE_UI1,
	DBTYPE_UI2,
	DBTYPE_UI4,
	DBTYPE_I1,
	DBTYPE_I2,
	DBTYPE_I4,
	DBTYPE_R4,
	DBTYPE_R8,
	DBTYPE_CY,
	DBTYPE_BSTR,
	DBTYPE_NULL,
	DBTYPE_ERROR,
	DBTYPE_BOOL,
	DBTYPE_DATE
};

///////////////////////////////////////////////////////////////////
// Forwards 
//
///////////////////////////////////////////////////////////////////
class CSchema;
BOOL IsFixedLength(DBTYPE wType);
BOOL IsNumericType(DBTYPE wType);
BOOL IsScaleType(DBTYPE wType);


//--------------------------------------------------------------------
// @Class CCol	|	Class definition for Columns 
//
// The class is responsible for column metadata.
//
// Most/all of this information is found in IColmnsInfo::GetColumnsInfo().
// <nl>
// <nl>
//
// Note: The set functions are inline but public because it is a contained class
// in CTable instead of a base class of CTable.
//
//--------------------------------------------------------------------
class CCol 
{
// @access Protected
protected:

	//ColumnInfo
	DBCOLUMNINFO m_ColInfo;
		
	// @cmember Provider data type name. 
	// Found in the schema rowset TYPES. <nl>
	// Default = NULL.
	WCHAR *	m_pwszProviderTypeName;

	// @cmember Literal prefix. <nl>
	// Found in the schema rowset TYPES.<nl>
	// Default = "".
	WCHAR *	m_pwszPrefix;

	// @cmember Literal suffix. <nl>
	// Found in schema rowset TYPES. <nl>
	// Default = "".
	WCHAR *	m_pwszSuffix;

	// @cmember column description <nl>
	// Found in schema rowset COLUMNS and settable through DBPROP_COL_DESCRIPTION. <nl>
	// Default = NULL.
	WCHAR	*m_pwszColDescription;

	// @cmember Special information that the provider needs in order to create this column.
	// Examples would be "max length", "precision, scale", "precision", "length". <nl>
	// Found in schema rowset TYPES. <nl>
	// Default = "".
	WCHAR * m_pwszCreateParams;

	// @cmember Sub data type. <nl>
	// Used mainly for underlying VARIANT type. <nl>
	DBTYPE	m_wSubType;

	// @cmember TRUE if this column is nullable. <nl>
	// Found in schema rowset TYPES. <nl>
	// Default = FALSE. <nl>
	BOOL	m_fNullable;

	// @cmember TRUE if column is unsigned. FALSE if column is signed. 
	// NULL if not applicable. <nl> 
	// Found in schema rowset TYPES.<nl>
	// Default = FALSE.
	BOOL	m_fUnsigned;

	// @cmember Scale of column. 
	// Found in schema rowset TYPES. <nl>
	// Default = 0. 
	SHORT	m_sMinScale;

	// @cmember Scale of column. 
	// Found in schema rowset TYPES. <nl>
	// Default = 0. 
	SHORT	m_sMaxScale;

	// @cmember TRUE if column is autoincremented. 
	// FALSE if column is not autoincremented. 
	// Currently, only one autoinc column can be set to TRUE!<nl>
	// Default = FALSE.
	BOOL	m_fAutoInc;

	// @cmember TRUE if column can be autoincremented. 
	// FALSE if column cannot be autoincremented. 
	// this will not be altered by creating a table <nl>
	// Default = FALSE.
	BOOL	m_fCanAutoInc;

	// @cmember TRUE if column is updateable. <nl>
	// This is set in CTable::WriteAll,
	// The information is found in IColumnsInfo::GetColumnsInfo. 
	// Default = FALSE.
	BOOL	m_fUpdateable;

	// @cmember Indicated column searchability. <nl>
	// Default = 0. <nl>
	// Value should correspond to the OLE DB values listed as:<nl>
	// DB_UNSEARCHABLE - the column cannot be used with a row or scalar
	// operator or in a SQL text WHERE clause <nl>
	// DB_LIKE_ONLY - the column can only be used with the DBOP_like*
	// operators or in a SQL text WHERE caluse using a LIKE predicate <nl>
	// DB_ALL_EXCEPT_LIKE - the column can be used with all row and scalar
	// operators except DBOP_like and in any SQL text WHERE clause not using
	// a like predicate <nl>
	// DB_SEARCHABLE - the column can be used with any row or scalar operator
	// and in a SQL text WHERE caluse
	ULONG	m_ulSearchable;

	// @cmember TRUE if column will be used in where clause. <nl>
	// Default = TRUE.
	BOOL	m_fUseInSQL;

	// @cmember TRUE if the data type is a BLOB that contains very long data.
	// Default = FALSE. <nl>
	BOOL	m_fIsLong;

	// @cmember TRUE if the data type is a fixed length
	// Default = FALSE <nl>
	BOOL	m_fIsFixedLength;

	// @cmember TRUE if column values are unique
	// Default = FALSE <nl>
	BOOL	m_fUnique;

	// @cmember TRUE if the data type is case sensitive
	// Default = FALSE <nl>
	BOOL	m_fCaseSensitive;

	// @cmember Column Type Guid
	GUID	m_gTypeGuid;

	// @cmember Column Default Value
	VARIANT	m_DefaultValue;

	// @cmember Column Has Default Value
	BOOL	m_fHasDefault;

	// @cmember Number of explicitly requested user params
	ULONG	m_cReqParams;

	// @cmember Collation LCID
	ULONG	m_ulLCID;

	// @cmember Number of variant subtypes to be used
	ULONG	m_cVariantSubTypes;

	// @cmember Array of variant subtypes to be used
	DBTYPE * m_prgVariantSubTypes;

	// @cmember Back pointer to schema object
	CSchema * m_pSchema; 

	BOOL m_fIsNewLongType;

// @access Public
public:

	// @cmember Constructor. MAX_PTR
	CCol(	
		IMalloc *		pIMalloc = NULL,				// [IN]  IMalloc pointer to alloc memory with (Default = NULL)
		WCHAR *			pwszColName = NULL,
		DBORDINAL		iOrdinal = 0,			  	// [IN]  Column number (Default = 0)  
		DBTYPE		 	wProviderType = DBTYPE_STR,	// [IN]  Provider datatype (Default = DBTYPE_STR) 
		WCHAR * 		wszProviderTypeName = NULL,	// [IN]  Provider datatype name (Default = NULL) 
		BYTE 			bPrecision = UCHAR_MAX,		// [IN]  Maximum precision allowed for this column (Default = 0xFF) 
		BYTE 			bScale = UCHAR_MAX,			// [IN]  Scale (Default = 0xFF)
		WCHAR * 		wszPrefix=NULL,				// [IN]  Literal Prefix (Default = NULL)
		WCHAR * 		wszSuffix=NULL,				// [IN]  Literal Suffix (Default = NULL)
		BOOL 			fNullable=0,				// [IN]  If type is Nullable (Default = FALSE)
		BOOL 			fUnsigned=-1,				// [IN]  If type is Unsigned (Default = -1)
		BOOL 			fAutoInc=FALSE,				// [IN]  Autoincrementing (Default = FALSE)
		WCHAR * 		wszCreateParams=NULL,		// [IN]  Data type definition (Default = NULL)
		BOOL 			fUpdateable=FALSE,			// [IN]  If type is updateable (Default = FALSE)
		ULONG 			ulSearchable=DB_UNSEARCHABLE,// [IN]  If type is searchable (Default = DB_UNSEARCHABLE)
		BOOL 			fIsLong=FALSE,				// [IN]  Very long data (Default = FALSE)
		GUID			gTypeGuid=GUID_NULL,
		BOOL			fIsFixedLength=FALSE,		// [IN]	Is data fixed length (Default = FALSE)
		BOOL			fCaseSensitive=FALSE,		// [IN]	Is data case sensitive (Default = FALSE)
		BOOL			fUnique=FALSE,				// [IN]	Are deta values unique (Default = FALSE)
		BOOL			fHasDefault=FALSE,			// [IN]	Column has default value (Default = FALSE)
		CSchema *		pSchema=NULL				// [IN] Containing Schema object (Default = NULL)
	);

	CCol(
		CSchema *		pSchema						// [IN] Containing Schema object (Default = NULL)
	);

	void InitCCol(										
		IMalloc *		pIMalloc,				// @parm [IN] IMalloc pointer to alloc memory with (Default = NULL).
												// Null is a valid default because the majority of usage is
												// as a temp variable to hold a member of CTable.m_ColList,
												// which already has a malloc point associated with it.
		WCHAR *			pwszColName,			// @parm [IN] Column Name
		DBORDINAL		iOrdinal,				// @parm [IN] Column Ordinal
		DBTYPE			wType,					// @parm [IN] Datatype (Default = DBTYPE_STR). 
		WCHAR *			pwszProviderTypeName,	// @parm [IN] Provider datatype name (Default = NULL).
		BYTE			bPrecision,				// @parm [IN] Maximum precision allowed for this column (Default = 0xFF).
		BYTE			bScale,					// @parm [IN] Scale (Default = 0xFF).
		WCHAR *			pwszPrefix,				// @parm [IN] Literal Prefix (Default = "").
		WCHAR *			pwszSuffix,				// @parm [IN] Literal Suffix (Default = "").
		BOOL			fNullable,				// @parm [IN] Nullable (Default = FALSE).
		BOOL			fUnsigned/* =-1 */,		// @parm [IN] Unsigned (Default = -1).
		BOOL			fAutoInc,				// @parm [IN] Auto Increment (Default = FALSE).
		WCHAR *			pwszCreateParams,		// @parm [IN] CreateParam info (Default = "").
		BOOL			fUpdateable,			// @parm [IN] Updateable (Default = FALSE).
		ULONG			ulSearchable,			// @parm [IN] Searchable (Default = DB_SEARCHABLE).
		BOOL			fIsLong,				// @parm [IN] Very long data (Default = FALSE).
		GUID			gTypeGuid,
		BOOL			fIsFixedLength,			// @parm [IN] Is column fixed len (Default = FALSE)
		BOOL			fCaseSensitive,			// @parm [IN] Is column case sensitive (Default = FALSE)
		BOOL			fUnique,				// @parm [IN] Does column contain unique values (Default = FALSE)
		BOOL			fHasDefault,			// @parm [IN] Has column a default value (Default = FALSE)
		CSchema *		pSchema					// @parm [IN] Containing Schema object (Default = NULL)
	);

	// @cmember Destructor. Frees string memory.
	virtual ~CCol();

	////////////////////////////////////////
	//
	//	String Functions
	//
	////////////////////////////////////////

	// @cmember Set Column Info
	void SetColInfo(DBCOLUMNINFO* pColInfo);

	// @cmember MakeData
	HRESULT MakeData(
		WCHAR* 		wszData,			// @parm [OUT] Return data
		DBCOUNTITEM	ulRowNum,			// @parm [IN]  Row number which is one based
		EVALUE 		eValue = PRIMARY,	// @parm [IN]  Primary or secondary data 
		ENULL		eNulls = USENULLS,	// @parm [IN]  Allow nulls for diagonal
		DBTYPE*		pwSubType = NULL,	// @parm [IN/OUT]  Column SubType (if Variant)
		DBORDINAL	ulUniqueCol = 1		// @parm [IN]  Unique column, (possible index)
	);

	// Returns the variant sub type corresponding to a particular row seed
	DBTYPE			GetVariantType(DBCOUNTITEM ulRowSeed, CCol& rCol);

	// Returns the variant prefix corresponding to a particular row seed
	WCHAR *			GetVariantPrefix(DBCOUNTITEM ulRowSeed);

	// Returns the variant suffix corresponding to a particular row seed
	WCHAR *			GetVariantSuffix(DBCOUNTITEM ulRowSeed);

	// @cmember Sets column name. 
	// If pwszColName is NULL, m_TableID.uName.pwszName is set to NULL
	void			SetColName(WCHAR * pwszColName);	// [IN] Column name string
	inline WCHAR*	GetColName()						{ return m_ColInfo.pwszName;	}

	// @cmember Sets column ID. 
	void			SetColID(DBID* pColumnID);			// [IN] ColumnID
	inline DBID*	GetColID()							{ return &m_ColInfo.columnid;	}

	// @cmember Sets CreateParms. If m_pwszCreateParams is valid and
	void			SetCreateParams(WCHAR * pwszCreateParams);
	inline WCHAR*	GetCreateParams()							{ return m_pwszCreateParams;	}

	
	// @cmember Sets ProviderTypeName. If m_pwszProviderTypeName is valid and
	void			SetProviderTypeName(WCHAR * pwszProviderTypeName);
	inline WCHAR*	GetProviderTypeName()						{ return m_pwszProviderTypeName;}


	// @cmember Sets Suffix. If m_pwszSuffix is valid and
	void			SetSuffix(WCHAR * pwszSuffix);
	inline WCHAR *	GetSuffix()									{ return m_pwszSuffix;			}

	// @cmember Interface methods for column description
	void			SetColDescription(WCHAR *pwszColDescription);
	WCHAR			*GetColDescription() {
		return m_pwszColDescription;
	}

	// @cmember Sets Prefix.If m_pwszPrefix is valid and
	void SetPrefix(WCHAR * pwszPrefix);
	inline WCHAR*	GetPrefix()									{ return m_pwszPrefix;			}

	// @cmember,mfunc Sets column Ordinal.
	inline void SetColNum(DBORDINAL iOrdinal = 0)					{ m_ColInfo.iOrdinal = iOrdinal;}
	inline DBORDINAL GetColNum()									{ return m_ColInfo.iOrdinal;	}

	// @cmember,mfunc Sets provider type.
	inline void		SetProviderType(DBTYPE wType)				{ m_ColInfo.wType = wType;		}
	inline DBTYPE	GetProviderType()							{ return m_ColInfo.wType;		}

	// @cmember,mfunc Sets SubType, m_wSubType.
	inline void		SetSubType(DBTYPE wSubType)					{ m_wSubType = wSubType;		}
	inline DBTYPE	GetSubType()								{ return m_wSubType;			}

	// @cmember,mfunc Gets Max size
	DBLENGTH		GetMaxSize();

	// @cmember,mfunc Set data source's ColumnSize for this datatype, m_lPrecision.
	inline void		SetColumnSize(DBLENGTH ulColumnSize)		{ /*ASSERT(!IsFixedLength(GetProviderType()));*/ m_ColInfo.ulColumnSize = ulColumnSize;		}
	inline DBLENGTH	GetColumnSize()								{ /*ASSERT(!IsFixedLength(GetProviderType()));*/ return m_ColInfo.ulColumnSize;				}
	DBLENGTH		GetMaxColumnSize(void);

	// @cmember,mfunc Set data source's precision for this datatype, bPrecision.
	inline void		SetPrecision(BYTE bPrecision)				{ /*ASSERT(IsNumericType(GetProviderType()) || GetProviderType()==DBTYPE_DBTIMESTAMP);*/ m_ColInfo.bPrecision = bPrecision;		}
	inline BYTE		GetPrecision()								{ /*ASSERT(IsNumericType(GetProviderType()) || GetProviderType()==DBTYPE_DBTIMESTAMP);*/ return m_ColInfo.bPrecision;			}

	// @cmember,mfunc Set data source's scale for this data type, bScale.
	inline void		SetScale(BYTE bScale)						{ /*ASSERT(IsScaleType(GetProviderType()));*/ m_ColInfo.bScale = bScale;		}
	inline BYTE		GetScale()									{ /*ASSERT(IsScaleType(GetProviderType()));*/ return m_ColInfo.bScale;		}

	// @cmember,mfunc Set data source's scale for this data type, m_sMinScale.
	inline void		SetMinScale(SHORT sScale)					{ /*ASSERT(IsScaleType(GetProviderType()));*/ m_sMinScale=sScale;			}
	inline SHORT	GetMinScale()								{ /*ASSERT(IsScaleType(GetProviderType()));*/ return m_sMinScale;			}

	// @cmember,mfunc Set data source's scale for this data type, m_sMaxScale.
	inline void		SetMaxScale(SHORT sScale)					{ /*ASSERT(IsScaleType(GetProviderType()));*/ m_sMaxScale=sScale;			}
	inline SHORT	GetMaxScale()								{ /*ASSERT(IsScaleType(GetProviderType()));*/ return m_sMaxScale;			}

	// @cmember,mfunc Set data source's ability to null this data type,
	inline void		SetNullable(BOOL fNullable=0)				{ m_fNullable=fNullable;			}
	inline BOOL		GetNullable ()								{ return m_fNullable;				}

	// @cmember,mfunc Set data source's ability to have this data type unsigned,
	inline void		SetUnsigned(BOOL fUnsigned=FALSE)			{ m_fUnsigned=fUnsigned;			}
	inline BOOL		GetUnsigned()								{ return m_fUnsigned;				}

	inline void			SetTypeInfo(ITypeInfo* pTypeInfo)		{ m_ColInfo.pTypeInfo = pTypeInfo;	SAFE_ADDREF(pTypeInfo); }
	inline ITypeInfo*	GetTypeInfo()							{ return m_ColInfo.pTypeInfo;								}

	// @cmember,mfunc Set data source's ability to autoincrement this data type,
	// @@parmopt [IN] Is type autoincrementable (Default = FALSE)
	inline void		SetAutoInc(BOOL fAutoInc=FALSE)				{ m_fAutoInc=fAutoInc;				}
	inline BOOL		GetAutoInc()								{ return m_fAutoInc;				}

	// @cmember,mfunc Set data type's ability to support autoinc columns
	inline void		SetCanAutoInc(BOOL fAutoInc=FALSE)			{ m_fCanAutoInc=fAutoInc;			}
	inline BOOL		CanAutoInc()								{ return m_fCanAutoInc;				}

	// @cmember,mfunc Set column values are unique
	inline BOOL		SetUnique(BOOL fUnique=FALSE)				{ return m_fUnique=fUnique;			}
	inline BOOL		GetUnique()									{ return m_fUnique;					}

	// @cmember,mfunc Set data source's ability to update this data type,
	inline void		SetUpdateable(BOOL fUpdateable=FALSE)		{ m_fUpdateable=fUpdateable;		}
	inline BOOL		GetUpdateable()								{ return m_fUpdateable;				}

	// @cmember,mfunc Set data source's ability to search this data type,
	inline void		SetSearchable(ULONG ulSearchable=0)			{ m_ulSearchable=ulSearchable;		}
	inline ULONG	GetSearchable()								{ return m_ulSearchable;			}

	// @cmember,mfunc Return GetColUsed, m_fUseInSQL.
	inline void		SetUseInSQL(BOOL fColUsed=FALSE)			{ m_fUseInSQL=fColUsed;				} 
	inline BOOL		GetUseInSQL()								{ return m_fUseInSQL;				}

	// @cmember,mfunc Set data source's ability to contain long data,
	inline void		SetIsLong(BOOL fIsLong=FALSE)				{ m_fIsLong=fIsLong;				}
	inline BOOL		GetIsLong()									{ return m_fIsLong;					}

	// @cmember,mfunc SetIsChapter
	inline void		SetIsChapter(BOOL fIsChapter)				{ ENABLE_BIT(m_ColInfo.dwFlags, DBCOLUMNFLAGS_ISCHAPTER, fIsChapter); }

	// @cmember, mfunc Return whether column is fixed length
	inline void		SetIsFixedLength(BOOL fIsFixedLength=FALSE)	{ m_fIsFixedLength=fIsFixedLength;	}
	inline BOOL		GetIsFixedLength()							{ return m_fIsFixedLength;			}

	// @cmember, mfunc Sets whether column has fixed len data or not
	inline void		SetCaseSensitive(BOOL fCaseSensitive=FALSE)	{ m_fCaseSensitive=fCaseSensitive;	}
	inline BOOL		GetCaseSensitive()							{ return m_fCaseSensitive;			}

	// @cmember,mfunc Set Column'sType Guid
	inline void		SetTypeGuid(GUID gTypeGuid)					{ memcpy(&m_gTypeGuid,&gTypeGuid,sizeof(GUID));			}
	inline GUID		GetTypeGuid()								{ return m_gTypeGuid;									}

	// @cmember,mfunc Set Column's DefaultValue
	inline HRESULT	SetDefaultValue(VARIANT DefaultValue)		{ return VariantCopy(&m_DefaultValue, &DefaultValue);	}
	inline VARIANT	GetDefaultValue()							{ return m_DefaultValue;								}

	// @cmember,mfunc Set whether the Column has DefaultValue
	inline BOOL		SetHasDefault(BOOL HasDefault)				{ return m_fHasDefault=HasDefault;	}
	inline BOOL		GetHasDefault()								{ return m_fHasDefault;				}

	// @cmember,mfunc Set whether the user has explicitly requested creation parameters
	inline void		SetReqParams(ULONG cReqParams=MAX_PTR)		{ m_cReqParams=cReqParams;	}
	inline ULONG	GetReqParams()								{ return m_cReqParams;		}

	// @cmember,mfunc Get and Set column LCID
	void			SetLCID(LONG lLCID);
	inline ULONG	GetLCID()									{ return m_ulLCID; }

	// @cmember,mfunc Set when this column is a new LOB column
	void			SetIsNewLongType(BOOL fIsNewLongType)		{m_fIsNewLongType = fIsNewLongType;}
	BOOL			GetIsNewLongType()							{return m_fIsNewLongType;}

	////////////////////////////////////////
	//
	//	Operators
	//
	////////////////////////////////////////

	// @cmember the actual string that would be necessary to create
	// a column of this type. Example: "col1name datatype(x) NULL".
	// Client must use CoTaskMemFree(*ppwszColDef).|
	void CreateColDef(
		WCHAR ** ppwszColDef	// [OUT] Column definition
	);   

	// @cmember Overloaded operator =, necessary for CList functions in CTable.
	// Makes a copy of all strings but refCCol.m_pMalloc has same address as ref'd object.|
	CCol& operator=(
		CCol& refCCol			// Reference to a CCol object
	);

	// @cmember Copy Constructor, necessary for CList functions in CTable.
	// Makes a copy of all strings but refCCol.m_pMalloc has same address as ref'd object.|
	CCol(CCol& refCCol);		// Reference to a CCol object
};

BOOL IsCColNullable(CCol *pCol);
BOOL IsCColNotNullable(CCol *pCol);

#endif	//_CCol_HPP_
