//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Error Header file Module | This module contains definition information
// for OLE DB strings
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//
// @head3 OLE DB strings Elements|
//
//---------------------------------------------------------------------------

#ifndef _STRINGS_H_
#define _STRINGS_H_


/////////////////////////////////////////////////////////////////////////////
// CString
//
/////////////////////////////////////////////////////////////////////////////
class CString
{
public:

//Constructors
	CString();
	CString(LPCSTR lpsz);
	virtual ~CString();

	//methods
	operator LPCTSTR() const;           // as a C string
	const CString& operator=(LPCSTR lpsz);
	BOOL operator==(LPCSTR lpsz);
	BOOL operator==(CString& rCString);

	// concatentation operator
	const CString& operator + (LPCSTR lpsz);

//Implementation
protected:
	CHAR* m_pszString;
};




/////////////////////////////////////////////////////////////////////////////
// CWString
//
/////////////////////////////////////////////////////////////////////////////
class CWString
{
public:

//Constructors
	CWString();
	CWString(LPCWSTR lpwsz);
	CWString(CWString &String);
	virtual ~CWString();

	//methods
	operator LPCWSTR() const;           // as a C string
	const CWString& operator=(LPCWSTR lpwsz);
	const CWString& operator=(CWString wszString);
	BOOL operator==(LPCWSTR lpsz);
	BOOL operator==(CWString& rCWString);

	// concatentation operator
	const CWString& operator + (LPCWSTR lpwsz);

//Implementation
protected:
	WCHAR* m_pwszString;
};




///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define VALUE_WCHAR(value) value, L#value
#define VALUE_CHAR(value) value, #value

typedef struct _WIDENAMEMAP
{
	LONG		lItem;		// Item
	WCHAR*		pwszName;	// Name
} WIDENAMEMAP;


typedef struct _NAMEMAP
{
	LONG		lItem;		// Item
	CHAR*		pszName;	// Name
} NAMEMAP;


typedef struct _WIDEGUIDMAP
{
	const GUID*		pGuid;		// Guid
	WCHAR*			pwszName;	// Name
} WIDEGUIDMAP;


typedef struct _GUIDMAP
{
	const GUID*		pGuid;		// Guid
	CHAR*			pszName;	// Name
} GUIDMAP;


typedef struct _WIDEDBIDMAP
{
	const DBID*		pDBID;		// DBID
	WCHAR*			pqszName;	// Name
} WIDEDBIDMAP;


typedef struct _INITPROPINFOMAP
{
	DBPROPID	dwPropertyID;
	VARTYPE		vt;
	WCHAR*		pwszName;
	WCHAR*		pwszDesc;
} INITPROPINFOMAP;
	

extern const ULONG g_cInitPropInfoMap;
extern const INITPROPINFOMAP g_rgInitPropInfoMap[];

extern const ULONG g_cObjTypeMap;
extern const WIDEGUIDMAP g_rgObjTypeMap[];

extern const ULONG g_cRowColMap;
extern const WIDEDBIDMAP g_rgRowColMap[];


////////////////////////////////////////////////////////////////////////////
// Extened Error Info
//
////////////////////////////////////////////////////////////////////////////
WCHAR*	GetMapName(REFGUID guid, ULONG cGuidMap, const WIDEGUIDMAP* rgGuidMap);
WCHAR*	GetMapName(LONG lItem, ULONG cNameMap, const WIDENAMEMAP* rgNameMap);
LONG		GetMapName(WCHAR* pwsz, ULONG cNameMap, const WIDENAMEMAP* rgNameMap);

CHAR*	GetMapName(REFGUID guid, ULONG cGuidMap, const GUIDMAP* rgGuidMap);
CHAR*	GetMapName(LONG lItem, ULONG cNameMap, const NAMEMAP* rgNameMap);
LONG		GetMapName(CHAR* psz, ULONG cNameMap, const NAMEMAP* rgNameMap);

WCHAR*	GetErrorName(HRESULT hr);
WCHAR*	GetPropSetName(REFGUID guidPropertySet);
WCHAR*	GetPropertyName(DBPROPID dwPropertyID, REFGUID guidPropertySet);
WCHAR*	GetStaticPropDesc(DBPROPID dwPropertyID, REFGUID guidPropertySet = DBPROPSET_DBINIT);

WCHAR*	GetStatusName(DBSTATUS dwStatus);
WCHAR*	GetPropStatusName(DBSTATUS dwStatus);
WCHAR*	GetRowStatusName(DBROWSTATUS dwRowStatus);
WCHAR*	GetBindStatusName(DBBINDSTATUS dwBindStatus);
WCHAR*	GetInterfaceName(REFIID riid);
DBSTATUS GetStatusValue(WCHAR* pwszName);

WCHAR*	GetDBTypeName(DBTYPE wType);
DBTYPE	GetDBType(WCHAR* pwszName);

EQUERY	GetSQLTokenValue(WCHAR* pwszName);
WCHAR*	GetSQLTokenName(EQUERY eQuery);
ULONG	GetSQLTokenMap(NAMEMAP** prgNameMap);

WCHAR* GetObjectTypeName(REFGUID rguid);
WCHAR* GetBindURLStatusName(DBBINDURLSTATUS dwBindStatus);

#endif	//_STRINGS_H_
