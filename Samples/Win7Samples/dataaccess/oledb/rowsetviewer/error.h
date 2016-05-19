//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module ERROR.H
//
//-----------------------------------------------------------------------------------

#ifndef _ERROR_H_
#define _ERROR_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
typedef struct _NAMEMAP
{
	CHAR*		pszName;	// Name
	LONG		lItem;		// Item
} NAMEMAP;

typedef struct _WIDENAMEMAP
{
	WCHAR*		pwszName;	// Name
	LONG		lItem;		// Item
} WIDENAMEMAP;

typedef struct _WIDEGUIDMAP
{
	WCHAR*			pwszName;	// Name
	const GUID*		pGuid;		// Guid
} WIDEGUIDMAP;

typedef struct _WIDEDBIDMAP
{
	WCHAR*			pwszName;	// Name
	const DBID*		pDBID;		// DBID
} WIDEDBIDMAP;


////////////////////////////////////////////////////////////////////////////
// Extened Error Info
//
////////////////////////////////////////////////////////////////////////////
enum ERRORPOST
{
	EP_ERRORINFO_ALWAYS		= 0x00000002,
	EP_ERRORINFO_FAILURE	= 0x00000004,
	EP_ERRORINFO_NEVER		= 0x00000008,

	EP_HRESULT_ALWAYS		= 0x00000010,
	EP_HRESULT_NOERRORINFO	= 0x00000020,
	EP_HRESULT_NEVER		= 0x00000040,
	EP_HRESULT_FAILURE		= 0x00000080,

	EP_REFCOUNT_FAILURE		= 0x00000100,
	EP_IMALLOC_SPY			= 0x00001000,
	EP_IMALLOC_ALLOCS		= 0x00002000
};

extern DWORD g_dwErrorPost;
extern DWORD g_dwBreakID;
BOOL SetErrorPosting(ERRORPOST eErrorPost, BOOL fSet = TRUE);
BOOL GetErrorPosting(ERRORPOST eErrorPost);


WCHAR*		GetMapName(REFGUID guid, ULONG cGuidMap, const WIDEGUIDMAP* rgGuidMap);
WCHAR*		GetMapName(const DBID* pDBID, ULONG cDBIDMap, const WIDEDBIDMAP* rgDBIDMap);
CHAR*		GetMapName(LONG lItem, ULONG cNameMap, const NAMEMAP* rgNameMap);
WCHAR*		GetMapName(LONG lItem, ULONG cNameMap, const WIDENAMEMAP* rgNameMap, DWORD dwConvFlags = CONV_NONE);
LONG		GetMapName(CHAR* psz, ULONG cNameMap, const NAMEMAP* rgNameMap);
LONG		GetMapName(WCHAR* pwsz, ULONG cNameMap, const WIDENAMEMAP* rgNameMap);
WCHAR*		GetBitMapName(DWORD dwItem, ULONG cNameMap, const WIDENAMEMAP* rgNameMap, DWORD dwBitStart = 0);

CHAR*		GetErrorName(HRESULT hr);
WCHAR*		GetStatusName(DBSTATUS dbStatus, DWORD dwConvFlags = CONV_DECIMAL);
DBSTATUS	GetStatusValue(WCHAR* pszName);
WCHAR*		GetInterfaceName(REFIID riid);
WCHAR*		GetObjectTypeName(REFGUID rguidObjectType);
WCHAR*		GetDialectName(REFGUID rguid);
WCHAR*		GetRowColName(const DBID* pColumnid);

//Generic Errors
HRESULT		DisplayHRESULT(HRESULT hrActual, WCHAR* pwszFile = L"Unknown", ULONG ulLine = 0, INT* piResult = 0);
HRESULT		DisplayWinError(DWORD dwError, WCHAR* pwszFile, LONG lLine, WCHAR* pwszText = L"Windows");

//IErrorInfo
HRESULT		DisplayAllErrors(HRESULT hrActual, WCHAR* pwszFile = L"Unknown", ULONG ulLine = 0, INT* piResult = 0);
HRESULT		DisplayAllErrors(HRESULT hrActual, HRESULT hrExpected, WCHAR* pwszFile = L"Unknown", ULONG ulLine = 0, INT* piResult = 0);
HRESULT		DisplayErrorInfo(HRESULT hrActual, WCHAR* pwszFile = L"Unknown", ULONG ulLine = 0, INT* piResult = 0);

//ISQLErrorInfo
HRESULT		GetSqlErrorInfo(ULONG iRecord, IErrorRecords* pIErrorRecords, BSTR* pBstr, LONG* plNativeError = NULL);

//Binding Errors
HRESULT		DisplayBindingErrors(HRESULT hrReturned, DBCOUNTITEM cBindings, const DBBINDING* rgBindings, void* pData);
HRESULT		DisplayColumnErrors(HRESULT hrReturned, DBORDINAL cColumns, DBID* rgColumns, DBSTATUS* rgStatus);
HRESULT		DisplayColAccessErrors(HRESULT hrReturned, DBORDINAL cColAccess, DBCOLUMNACCESS* rgColAccess);
HRESULT		DisplayRowErrors(HRESULT hrReturned, DBROWCOUNT cRows, HROW* rghRows, DBROWSTATUS* rgRowStatus);
HRESULT		DisplayAccessorErrors(HRESULT hrReturned, DBCOUNTITEM cBindings, const DBBINDING* rgBindings, DBBINDSTATUS* rgStatus);

//Properties
WCHAR*		GetPropSetName(REFGUID guidPropertySet);
WCHAR*		GetPropertyName(DBPROPID dwPropertyID, REFGUID guidPropertySet);
WCHAR*		GetPropStatusName(DBPROPSTATUS dbPropStatus);
HRESULT		GetStaticPropValues(DBPROPID dwPropertyID, REFGUID guidPropertySet, ULONG* pcPropValMap, const WIDENAMEMAP** prgPropValMap);
HRESULT		GetStaticPropInfo(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcStaticInfoSets, const DBPROPINFOSET** prgStaticInfoSets);
HRESULT		GetAllocedPropInfo(ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropInfoSets, DBPROPINFOSET** prgPropInfoSets);
BOOL		IsSpecialPropSet(REFGUID guidSpecialPropertySet, GUID* pGuidPropertySet);

//Property Errors
HRESULT		DisplayPropErrors(HRESULT hrReturned, REFIID riid, IUnknown* pIUnknown);
HRESULT		DisplayPropErrors(HRESULT hrReturned, ULONG cPropSets, DBPROPSET* rgPropSets);
HRESULT		DisplayPropErrors(HRESULT hrReturned, ULONG cPropInfoSets, DBPROPINFOSET* rgPropInfoSets);

//RefCount Errors
HRESULT		DisplayRefCountErrors(WCHAR* pwszName, ULONG ulActRefCount, ULONG ulExpRefCount = 0);

//Notifications
WCHAR*		GetPhaseName(DBEVENTPHASE ePhase);
WCHAR*		GetReasonName(DBREASON eReason);
WCHAR*		GetAsynchReason(ULONG ulOperation);
WCHAR*		GetAsynchPhase(ULONG ulAsynchPhase);


/////////////////////////////////////////////////////////////////////////////
// 	Name MAPS
//
/////////////////////////////////////////////////////////////////////////////
extern const ULONG			g_cInterfaceMaps;
extern const WIDEGUIDMAP	g_rgInterfaceMaps[];

extern const ULONG			g_cDBTypes;
extern const WIDENAMEMAP	g_rgDBTypes[];

extern const ULONG			g_cPropSetMaps;
extern const WIDEGUIDMAP	g_rgPropSetMaps[];

extern const ULONG			g_cVariantTypes;
extern const WIDENAMEMAP	g_rgVariantTypes[];
			 
extern const WIDENAMEMAP	g_rgIsoLevels[];
extern const ULONG			g_cIsoLevels;

extern const WIDENAMEMAP	g_rgXACTTC[];
extern const ULONG			g_cXACTTC;

extern const ULONG			g_cColFlagsMaps;
extern const WIDENAMEMAP	g_rgColFlagsMaps[];

extern const ULONG			g_cObjectTypeMaps;
extern const WIDEGUIDMAP	g_rgObjectTypeMaps[];

extern const ULONG			g_cDialectMaps;
extern const WIDEGUIDMAP	g_rgDialectMaps[];

extern const ULONG			g_cPromptOptions;
extern const WIDENAMEMAP	g_rgPromptOptions[];

extern const ULONG			g_cSourceType;
extern const WIDENAMEMAP	g_rgSourceType[];

extern const ULONG			g_cBindURLMaps;
extern const WIDENAMEMAP	g_rgBindURLMaps[];

extern const ULONG			g_cRowColMaps;
extern const WIDEDBIDMAP	g_rgRowColMaps[];

extern const ULONG			g_cLCID;
extern const WIDENAMEMAP	g_rgLCID[];

extern const ULONG			g_cConsTypeMaps;
extern const WIDENAMEMAP	g_rgConsTypeMaps[];

extern const ULONG			g_cDeferrabilityMaps;
extern const WIDENAMEMAP	g_rgDeferrabilityMaps[];

extern const ULONG			g_cMatchTypeMaps;
extern const WIDENAMEMAP	g_rgMatchTypeMaps[];

extern const ULONG			g_cUpDelRuleMaps;
extern const WIDENAMEMAP	g_rgUpDelRuleMaps[];

extern const ULONG			g_cCDFlagsMaps;
extern const WIDENAMEMAP	g_rgCDFlagsMaps[];

extern const ULONG			g_cResultFlagMaps;
extern const WIDENAMEMAP	g_rgResultFlagMaps[];

#endif	//_ERROR_H_
