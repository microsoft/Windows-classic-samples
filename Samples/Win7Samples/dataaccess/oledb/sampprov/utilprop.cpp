//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module UTILPROP.CPP | Properties utility object implementation
//
//
// Notes - there are two main methods in this module:
//     - CUtilProps::GetPropertyInfo, a helper function for IDBInfo::GetPropertyInfo
//     - CUtilProps::GetProperties, a helper function for IRowsetInfo::GetProperties
//
// Our property implementation is simplified considerably by the fact that we
// only support reading\getting the properties, we do not support
// writing\setting them. This makes sense because we are a simple provider,
// and our rowset object always creates all the interfaces it exposes. In
// other words, there are really no properties the consumer could set.
//
// The implementation is very simple - we keep a global table of the
// properties we support in s_rgprop. We search this table sequentially.
//
// Note that a full-featured provider would probably need to use a more
// sophisticated implementation. We keep the entire GUID for each property in
// our struct, which would become a waste of space if we had a lot more
// properties. Similarly, with large numbers of properties some sort of
// hashing would be better than the sequential search used here.
//
// Includes ------------------------------------------------------------------

#include "headers.h"
#include "sampver.h"
#define __T(x)      L ## x
#define _TEXT(x)      __T(x)

// Struct containing the properties we know about. The GUID and string fields are
// initialized in the constructor, because C++ makes it awkward to do so at declaration
// time. So, if you change this table, be sure to make parallel changes in CUtilProp::CUtilProp.
PROPSTRUCT s_rgprop[] =
   {
/* 0 */ {DBPROP_IAccessor,					DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IAccessor"},
/* 1 */ {DBPROP_IColumnsInfo,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IColumnsInfo"},
/* 2 */ {DBPROP_IConvertType,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IConvertType"},
/* 3 */ {DBPROP_IRowset,					DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IRowset"},
/* 4 */ {DBPROP_IRowsetChange,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,  VT_BOOL, TRUE,		0,							NULL,			L"IRowsetChange"},
/* 5 */ {DBPROP_IRowsetInfo,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IRowsetInfo"},
/* 6 */ {DBPROP_IRowsetIdentity,			DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IRowsetIdentity"},
/* 7 */ {DBPROP_CANHOLDROWS,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,  VT_BOOL, TRUE,		0,							NULL,			L"Hold Rows"},
/* 8 */ {DBPROP_LITERALIDENTITY,			DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,  VT_BOOL, TRUE,		0,							NULL,			L"Literal Row Identity"},
/* 9 */ {DBPROP_UPDATABILITY,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_I4,	 TRUE,		DBPROPVAL_UP_CHANGE|DBPROPVAL_UP_DELETE|DBPROPVAL_UP_INSERT, NULL,L"Updatability"},
/* 10*/ {DBPROP_IRow,						DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,	VT_BOOL, FALSE,		0,							NULL,			L"IRow"},
/* 11*/ {DBPROP_IRowChange,					DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,	VT_BOOL, FALSE,		0,							NULL,			L"IRowChange"},
/* 12*/ {DBPROP_IGetRow,					DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IGetRow"},
/* 13*/ {DBPROP_IGetSession,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, TRUE,		0,							NULL,			L"IGetSession"},
/* 14*/ {DBPROP_QUICKRESTART,				DBPROPFLAGS_ROWSET | DBPROPFLAGS_READ,						VT_BOOL, FALSE,		0,							NULL,			L"Quick Restart"},
/* 15*/ {DBPROP_SESS_AUTOCOMMITISOLEVELS,	DBPROPFLAGS_SESSION | DBPROPFLAGS_READ,						VT_I4,	 FALSE,		0,							NULL,			L"Autocommit Isolation Levels"},
/* 16*/ {DBPROP_ACTIVESESSIONS,				DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_I4,	 FALSE,		1,							NULL,			L"Active Sessions"},
/* 17*/ {DBPROP_PERSISTENTIDTYPE,			DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_I4,	 FALSE,		DBPROPVAL_PT_GUID_PROPID,	NULL,			L"Persistent ID Type"},
/* 18*/ {DBPROP_PROVIDERFILENAME,			DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_BSTR, FALSE,		0,							L"SAMPPROV.DLL",L"Provider Name"},
/* 19*/ {DBPROP_PROVIDERFRIENDLYNAME,		DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_BSTR, FALSE,		0,							L"Microsoft OLE DB Sample Provider",	L"Provider Friendly Name"},
/* 20*/ {DBPROP_PROVIDEROLEDBVER,			DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_BSTR, FALSE,		0,							L"02.00",		L"OLE DB Version"},
/* 21*/ {DBPROP_PROVIDERVER,				DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_BSTR, FALSE,		0,							_TEXT(VER_PRODUCTVERSION_STR),			L"Provider Version"},
/* 22*/ {DBPROP_ROWSETCONVERSIONSONCOMMAND, DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_BOOL, TRUE,		0,							NULL,			L"Rowset Conversions on Command"},
/* 23*/ {DBPROP_OLEOBJECTS,					DBPROPFLAGS_DATASOURCEINFO | DBPROPFLAGS_READ,				VT_I4,   FALSE,		DBPROPVAL_OO_DIRECTBIND|DBPROPVAL_OO_ROWOBJECT|DBPROPVAL_OO_SINGLETON,	NULL, L"OLE Object Support"},
/* 24*/ {DBPROP_INIT_DATASOURCE,			DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,	VT_BSTR, FALSE,		0,							L"",			L"Data Source"},
#ifdef _WIN64
/* 25*/ {DBPROP_INIT_HWND,					DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,	VT_I8,	 FALSE,		0,							NULL,			L"Window Handle"},
#else
/* 25*/ {DBPROP_INIT_HWND,					DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,	VT_I4,	 FALSE,		0,							NULL,			L"Window Handle"},
#endif
/* 26*/ {DBPROP_INIT_PROMPT,				DBPROPFLAGS_DBINIT | DBPROPFLAGS_READ | DBPROPFLAGS_WRITE,	VT_I2,   FALSE,		4,							NULL,			L"Prompt"}
    };


// Code ----------------------------------------------------------------------

// CUtilProp::CUtilProp ----------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CUtilProp::CUtilProp
    (
    void
	) :
	m_cwchNamePool(0), m_pwchNamePool(NULL)
{
	memcpy(m_rgproperties, s_rgprop, sizeof(s_rgprop));
	memset(m_wszFilePath,0,sizeof(m_wszFilePath));
    return;
}


// CUtilProp::~CUtilProp ---------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CUtilProp:: ~CUtilProp
    (
	void
    )
{
	return;
}



// CUtilProp::GetPropIndex ----------------------------------------------------
//
// @mfunc Returns index of the given property in our global table of properties
//
// @rdesc BOOL
//      @flag TRUE      | found match, copied it to pulIndex out-param
//      @flag FALSE     | no match. In this case, pulIndex has no meaning
//
BOOL CUtilProp::GetPropIndex
    (
		DBPROPID dwPropertyID,  //@parm IN  | PROPID of desired property
	    ULONG *	 pulIndex		//@parm OUT | index of desired property if return was TRUE
    )
{
    //
	// asserts
	//
    assert(pulIndex);

    for (ULONG cNumberOfProperties = 0; 
		 cNumberOfProperties < NUMBER_OF_SUPPORTED_PROPERTIES; 
		 cNumberOfProperties++)
    {
        if( dwPropertyID == m_rgproperties[cNumberOfProperties].dwPropertyID )
        {
            // found a match
            *pulIndex = cNumberOfProperties;
            return TRUE;
         }
    }
    
	// found no matches
    return FALSE;
}



// CUtilProp::LoadDBPROPINFO  ----------------------------------------------------
//
// @mfunc Helper for GetPropertyInfo. Loads field of DBPROPINFO structure.
//
// @rdesc BOOL
//      @flag TRUE          | Method succeeded
//      @flag FALSE         | Method failed (couldn't allocate memory)
//
void CUtilProp::LoadDBPROPINFO
    (
    PROPSTRUCT * pPropStruct,
    DBPROPINFO * pPropInfo
    )
{
    //
	// asserts
	//
    assert(pPropStruct);
    assert(pPropInfo);

    //
	// set the easy fields..
	//
	pPropInfo->pwszDescription	= m_pwchNamePool;
	pPropInfo->dwPropertyID		= pPropStruct->dwPropertyID;
    pPropInfo->dwFlags			= pPropStruct->dwFlags;
    pPropInfo->vtType			= pPropStruct->vtType;

    //
	// init the variant
	//
    VariantInit(&pPropInfo->vValues);

	//
	// fill in the description
	//
	//if( pPropInfo->pwszDescription )
	StringCchCopyExW(pPropInfo->pwszDescription, m_cwchNamePool, pPropStruct->pwstrDescBuffer, &m_pwchNamePool, &m_cwchNamePool, 0);
	m_cwchNamePool --;
	m_pwchNamePool ++; // move over the null terminator

	return;
}


// CUtilProp::LoadDBPROP  ----------------------------------------------------
//
// @mfunc Helper for GetProperties. Loads field of DBPROP structure.
//
// @rdesc HRESULT
//      @flag TRUE          | Method succeeded
//      @flag FALSE         | Method failed (couldn't allocate memory)
//
HRESULT CUtilProp::LoadDBPROP
    (
    PROPSTRUCT*	pPropStruct,
    DBPROP*		pPropSupport
    )
{
    HRESULT hr = S_OK;

	//
	// asserts
	//
    assert(pPropStruct);
    assert(pPropSupport);

    //
	// set the easy fields..
	//
    pPropSupport->dwPropertyID = pPropStruct->dwPropertyID;
    pPropSupport->colid		   = DB_NULLID;
	pPropSupport->dwStatus	   = DBPROPSTATUS_OK;

    //
	// init the variant
	//
    VariantInit(&pPropSupport->vValue);

	//
    // set pPropSupport->vValue based on Variant type
	//
    switch (pPropStruct->vtType)
    {
	    case VT_BOOL:
			V_VT(&pPropSupport->vValue) = VT_BOOL;
			V_BOOL(&pPropSupport->vValue) = 
				  (pPropStruct->boolVal ? VARIANT_TRUE : VARIANT_FALSE);
			break;
        
	    case VT_I2:
		    V_VT(&pPropSupport->vValue) = VT_I2;
			V_I2(&pPropSupport->vValue) = (SHORT)pPropStruct->longVal;
			break;

		case VT_I4:
		    V_VT(&pPropSupport->vValue) = VT_I4;
			V_I4(&pPropSupport->vValue) = (LONG)pPropStruct->longVal;
			break;

#ifdef _WIN64
   		case VT_I8:
		    V_VT(&pPropSupport->vValue) = VT_I8;
			V_I8(&pPropSupport->vValue) = (LONGLONG)pPropStruct->longVal;
			break;
#endif

	    case VT_BSTR:
			V_VT(&pPropSupport->vValue) = VT_BSTR;
			SAFE_SYSALLOC(V_BSTR(&pPropSupport->vValue), pPropStruct->pwstrVal);
			break;
    
		default:
			assert( !"LoadDBPROP unknown variant type!\n\r" );
			hr = E_FAIL;
			break;
    }

CLEANUP:
	
	return hr;
}



//---------------------------------------------------------------------------
// CUtilProp::GetPropertiesArgChk
//
// @mfunc Initialize the buffers and check for E_INVALIDARG cases
//
// NOTE: This routine is used by RowsetInfo and IDBProperties
//
// @rdesc HRESULT indicating the status of the method
//		@flag S_OK | Properties gathered
//		@flag E_INVALIDARG | Invalid parameter values
//
HRESULT CUtilProp::GetPropertiesArgChk
	(
	DWORD				dwBitMask,			//@parm IN  | Mask for object
	const ULONG			cPropertySets,		//@parm IN | Number of property Sets
	const DBPROPIDSET	rgPropertySets[],	//@parm IN | Property Classes and Ids
	ULONG*				pcProperties,		//@parm OUT | Count of structs returned
	DBPROPSET**			prgProperties		//@parm OUT | Array of Properties
	)
{
	//
	// Initialize values
	//
	if( pcProperties )
		*pcProperties = 0;
	
	if( prgProperties )
		*prgProperties = NULL;	

	//
	// Check Arguments
	//
	if( (cPropertySets && !rgPropertySets) || !pcProperties || !prgProperties )
		return E_INVALIDARG;

	//
	// New argument check for > 1 cPropertyIDs and NULL pointer for the array.
	//
	for(ULONG ul=0; ul < cPropertySets; ul++)
	{
		if( rgPropertySets[ul].cPropertyIDs && !rgPropertySets[ul].rgPropertyIDs )
			return E_INVALIDARG;
	
		//
		// Check for propper formation of DBPROPSET_PROPERTIESINERROR
		//
		if( (dwBitMask & PROPSET_DSO || dwBitMask & PROPSET_ROWSET) &&
			(rgPropertySets[ul].guidPropertySet == DBPROPSET_PROPERTIESINERROR) )
		{
			if( (cPropertySets > 1) || 
				rgPropertySets[ul].cPropertyIDs || 
				rgPropertySets[ul].rgPropertyIDs )
				return E_INVALIDARG;
		}
	}
	
	return S_OK;
}


// CUtilProp::IsValidValue  -----------------------------------------
// @mfunc Validate that the variant contains legal values for it's particalur
// type and for the particular PROPID in this propset.
//
// @rdesc HRESULT indicating status
//        
HRESULT CUtilProp::IsValidValue
	(
	DBPROP*	pDBProp
	)
{
	//
	// Switch on the vt type first.
	//
	switch (V_VT(&pDBProp->vValue))
	{
		case VT_BOOL:
			if( V_BOOL(&pDBProp->vValue) != VARIANT_TRUE && 
				V_BOOL(&pDBProp->vValue) != VARIANT_FALSE )
				return S_FALSE;
			break;

		case VT_I2:
			if( V_I2(&pDBProp->vValue) < 0 )
				return S_FALSE;

			//
			// Switch on the property ID for VT_I2.
			//
			switch( pDBProp->dwPropertyID )
			{
				case DBPROP_INIT_PROMPT:

					if( V_I2(&pDBProp->vValue) != DBPROMPT_PROMPT && 
						V_I2(&pDBProp->vValue) != DBPROMPT_COMPLETE &&
						V_I2(&pDBProp->vValue) != DBPROMPT_COMPLETEREQUIRED &&
						V_I2(&pDBProp->vValue) != DBPROMPT_NOPROMPT )
						return S_FALSE;
			}
			
			break;

		case VT_I4:
			if( V_I4(&pDBProp->vValue) < 0 )
				return S_FALSE;

			//
			// Switch on the property ID for VT_I4.
			//
			switch( pDBProp->dwPropertyID )
			{
				case DBPROP_SESS_AUTOCOMMITISOLEVELS:

				if( V_I4(&pDBProp->vValue) != 0 && 
					V_I4(&pDBProp->vValue) != DBPROPVAL_TI_CHAOS &&
					V_I4(&pDBProp->vValue) != DBPROPVAL_TI_READUNCOMMITTED &&
					V_I4(&pDBProp->vValue) != DBPROPVAL_TI_READCOMMITTED &&
					V_I4(&pDBProp->vValue) != DBPROPVAL_TI_REPEATABLEREAD &&
					V_I4(&pDBProp->vValue) != DBPROPVAL_TI_SERIALIZABLE )
					return S_FALSE;
			}
			
			break;

		default:
			break;
	}

	return S_OK;
}


// CUtilProp::GetPropertyInfo  -----------------------------------------
//
// @mfunc	Returns information about rowset and data source properties 
//			supported by the provider
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcPropertyIDSets or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//

STDMETHODIMP CUtilProp::GetPropertyInfo
    (
	BOOL				fDSOInitialized,	//@parm IN  | if Initialized
    ULONG				cPropertyIDSets,	//@parm IN  | # properties
    const DBPROPIDSET	rgPropertyIDSets[],	//@parm IN  | Array of property sets
	ULONG*				pcPropertyInfoSets,	//@parm OUT | # DBPROPSET structures
	DBPROPINFOSET**		prgPropertyInfoSets,//@parm OUT | DBPROPSET structures property 
											//			| information returned
	WCHAR**				ppDescBuffer		//@parm OUT	| Property descriptions
    )
{
    HRESULT			hr				  = S_OK;
	BOOL			fPropsinError	  = FALSE;
	BOOL			fPropsSucceed	  = FALSE;
	BOOL			fIsSpecialGUID	  = FALSE;
	BOOL			fIsNotSpecialGUID = FALSE;
    ULONG			cProps			  = 0;
	ULONG			cCount			  = 0;
    ULONG			ulPropertySets	  = 0;
	ULONG			cBuffer			  = NUMBER_OF_SUPPORTED_PROPERTIES;

    DBPROPINFOSET*	pPropInfoSet = NULL;
    DBPROPINFO*		pPropInfo	 = NULL;
	WCHAR*			pDescBuffer	 = NULL;

	//
	// Initialize values
	//
	if( pcPropertyInfoSets )
		*pcPropertyInfoSets	 = 0;
	
	if( prgPropertyInfoSets )
		*prgPropertyInfoSets = NULL;
	
	if( ppDescBuffer )
		*ppDescBuffer = NULL;

	//
	// Check Arguments
	//
	if( ((cPropertyIDSets > 0) && !rgPropertyIDSets) ||
		!pcPropertyInfoSets || !prgPropertyInfoSets )
        return E_INVALIDARG;

	//
	// New argument check for > 1 cPropertyIDs and NULL pointer for the array.
	//
	for(ULONG ul=0; ul < cPropertyIDSets; ul++)
	{
		if( rgPropertyIDSets[ul].cPropertyIDs && !(rgPropertyIDSets[ul].rgPropertyIDs) )
			return E_INVALIDARG;
		
		if( DBPROPSET_VIEWALL == rgPropertyIDSets[ul].guidPropertySet			||
			DBPROPSET_TRUSTEEALL == rgPropertyIDSets[ul].guidPropertySet		||
			DBPROPSET_TABLEALL == rgPropertyIDSets[ul].guidPropertySet			||
			DBPROPSET_SESSIONALL == rgPropertyIDSets[ul].guidPropertySet		||
			DBPROPSET_ROWSETALL == rgPropertyIDSets[ul].guidPropertySet			||
			DBPROPSET_INDEXALL == rgPropertyIDSets[ul].guidPropertySet			||
			DBPROPSET_DBINITALL == rgPropertyIDSets[ul].guidPropertySet			||
			DBPROPSET_DATASOURCEINFOALL == rgPropertyIDSets[ul].guidPropertySet	||
			DBPROPSET_DATASOURCEALL == rgPropertyIDSets[ul].guidPropertySet		||
			DBPROPSET_CONSTRAINTALL == rgPropertyIDSets[ul].guidPropertySet		||
			DBPROPSET_COLUMNALL == rgPropertyIDSets[ul].guidPropertySet )
			fIsSpecialGUID = TRUE;
		else
			fIsNotSpecialGUID = TRUE;

		if( fIsSpecialGUID && fIsNotSpecialGUID )
			return E_INVALIDARG;

		// figure out the nuber of property descriptions
		if( !rgPropertyIDSets[ul].cPropertyIDs )
			cBuffer += NUMBER_OF_SUPPORTED_PROPERTIES;
		else
			cBuffer += rgPropertyIDSets[ul].cPropertyIDs;
	}

	// save the count of PropertyIDSets
	cProps = cPropertyIDSets;

	// If the consumer does not restrict the property sets
	// by specify an array of property sets and a cPropertySets
	// greater than 0, then we need to make sure we 
	// have some to return
	if( cPropertyIDSets == 0 )
	{
		if( fDSOInitialized )
			cProps = NUMBER_OF_SUPPORTED_PROPERTY_SETS;
		else
			cProps = 1;
	}	

    // use task memory allocater to alloc a DBPROPINFOSET struct
    SAFE_ALLOC(pPropInfoSet, DBPROPINFOSET, cProps);
    memset(pPropInfoSet, 0, cProps * sizeof(DBPROPINFOSET));

	// Alloc memory for ppDescBuffer
	if( ppDescBuffer )
	{
		SAFE_ALLOC(pDescBuffer, WCHAR, 
				   cProps * cBuffer * CCH_GETPROPERTYINFO_DESCRIP_BUFFER_SIZE);
		memset(pDescBuffer, 0, (cProps * cBuffer * sizeof(WCHAR) *
									 CCH_GETPROPERTYINFO_DESCRIP_BUFFER_SIZE));
		*ppDescBuffer = pDescBuffer;
		m_pwchNamePool = pDescBuffer;
		m_cwchNamePool = cProps * cBuffer * CCH_GETPROPERTYINFO_DESCRIP_BUFFER_SIZE;
	}

	// For each supported Property Set
	for(ulPropertySets=0; ulPropertySets < cProps; ulPropertySets++)
	{
		BOOL fGetAllProps = FALSE;

		// If no restrictions return all properties from the three supported property sets
		if( cPropertyIDSets == 0 )
		{
			fGetAllProps = TRUE;

			// only do this once
			if( ulPropertySets == 0 )
			{
				pPropInfoSet[0].guidPropertySet = DBPROPSET_DBINIT;
				pPropInfoSet[0].cPropertyInfos  = NUMBER_OF_SUPPORTED_DBINIT_PROPERTIES;

				if( fDSOInitialized )
				{
					pPropInfoSet[1].guidPropertySet = DBPROPSET_DATASOURCEINFO;
					pPropInfoSet[1].cPropertyInfos  = NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES;
					pPropInfoSet[2].guidPropertySet = DBPROPSET_SESSION;
					pPropInfoSet[2].cPropertyInfos  = NUMBER_OF_SUPPORTED_SESSION_PROPERTIES;
					pPropInfoSet[3].guidPropertySet = DBPROPSET_ROWSET;
					pPropInfoSet[3].cPropertyInfos  = NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES;
				}
			}
		}
		else
		{
			pPropInfoSet[ulPropertySets].guidPropertySet = rgPropertyIDSets[ulPropertySets].guidPropertySet;
			pPropInfoSet[ulPropertySets].cPropertyInfos  = rgPropertyIDSets[ulPropertySets].cPropertyIDs;

			if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DBINITALL) ||
				(rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DBINIT	  &&
				 rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0) )
			{
				fGetAllProps = TRUE;
				pPropInfoSet[ulPropertySets].guidPropertySet = DBPROPSET_DBINIT;
				pPropInfoSet[ulPropertySets].cPropertyInfos  = NUMBER_OF_SUPPORTED_DBINIT_PROPERTIES;
			}
			else if( fDSOInitialized )
			{
				if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFOALL) ||
					((rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFO)	  &&
					 (rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0)) )
				{
					fGetAllProps = TRUE;
					pPropInfoSet[ulPropertySets].guidPropertySet = DBPROPSET_DATASOURCEINFO;
					pPropInfoSet[ulPropertySets].cPropertyInfos  = NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES;
				}
				else if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_SESSIONALL) ||
						 ((rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_SESSION)   &&
						  (rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0)) )
				{
					fGetAllProps = TRUE;
					pPropInfoSet[ulPropertySets].guidPropertySet = DBPROPSET_SESSION;
					pPropInfoSet[ulPropertySets].cPropertyInfos  = NUMBER_OF_SUPPORTED_SESSION_PROPERTIES;
				}
				else if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_ROWSETALL) ||
						 ((rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_ROWSET)   &&
						  (rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0)) )
				{
					fGetAllProps = TRUE;
					pPropInfoSet[ulPropertySets].guidPropertySet = DBPROPSET_ROWSET;
					pPropInfoSet[ulPropertySets].cPropertyInfos  = NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES;
				}
				else if( rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0 )
					fPropsinError = TRUE;
			}
			else if( rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0 )
			{
				// Since we do not support it should return a error with 0 & NULL
				fPropsinError = TRUE;
			}
		}
		
		if( pPropInfoSet[ulPropertySets].cPropertyInfos )
		{
		    // use task memory allocater to alloc array of DBPROPINFO structs
			SAFE_ALLOC(pPropInfo, DBPROPINFO, 
								  pPropInfoSet[ulPropertySets].cPropertyInfos);
			memset(pPropInfo, 0, 
				pPropInfoSet[ulPropertySets].cPropertyInfos * sizeof(DBPROPINFO));

			pPropInfoSet[ulPropertySets].rgPropertyInfos = &pPropInfo[0];
		}

	    // for each prop in our table..
		for (cCount=0; cCount < pPropInfoSet[ulPropertySets].cPropertyInfos; cCount++)
		{
			// init the Variant right up front
			// that way we can VariantClear with no worried (if we need to)
			VariantInit( &pPropInfo[cCount].vValues );

			// set the description pointer
			// pPropInfo[cCount].pwszDescription = pDescBuffer;

			// Check supported property sets
			if( fGetAllProps )
			{
				if( pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_DBINIT )
				{
					// load up their DBPROPINFO from our table
					fPropsSucceed = TRUE;
					LoadDBPROPINFO(&m_rgproperties[START_OF_SUPPORTED_DBINIT_PROPERTIES + cCount], 
								   &pPropInfo[cCount]);
				}
				else if( pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFO )
				{
					// load up their DBPROPINFO from our table
					fPropsSucceed = TRUE;
					LoadDBPROPINFO(&m_rgproperties[START_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES + cCount], 
								   &pPropInfo[cCount] );
				}
				else if( pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_SESSION )
				{
					// load up their DBPROPINFO from our table
					fPropsSucceed = TRUE;
					LoadDBPROPINFO(&m_rgproperties[START_OF_SUPPORTED_SESSION_PROPERTIES + cCount], 
								   &pPropInfo[cCount] );
				}
				else
				{
					// load up their DBPROPINFO from our table
					fPropsSucceed = TRUE;
					LoadDBPROPINFO(&m_rgproperties[START_OF_SUPPORTED_ROWSET_PROPERTIES + cCount], 
								   &pPropInfo[cCount] );
				}
			}
			else
			{
				ULONG ulIndex;

				pPropInfo[cCount].dwPropertyID = rgPropertyIDSets[ulPropertySets].rgPropertyIDs[cCount];
				pPropInfo[cCount].dwFlags	   = DBPROPFLAGS_NOTSUPPORTED;

				if( (GetPropIndex(rgPropertyIDSets[ulPropertySets].rgPropertyIDs[cCount], &ulIndex)) &&
					(pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_DBINIT ||
					 pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFO && fDSOInitialized ||
					 pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_SESSION && fDSOInitialized ||
					 pPropInfoSet[ulPropertySets].guidPropertySet == DBPROPSET_ROWSET && fDSOInitialized) )
				{
					fPropsSucceed = TRUE;
					LoadDBPROPINFO( &m_rgproperties[ulIndex], &pPropInfo[cCount] );
				}
				else
				{
					fPropsinError = TRUE;
					pPropInfo[cCount].pwszDescription = NULL;
				}
			}

			// move the description pointer to the next
			//if( pPropInfo[cCount].pwszDescription )
			//	pDescBuffer += (wcslen(pPropInfo[cCount].pwszDescription) + 1);
		}
		// Set local back to FALSE
		fGetAllProps = FALSE;
	}

	// set count of properties and property information
    *pcPropertyInfoSets	 = cProps;
    *prgPropertyInfoSets = pPropInfoSet;

CLEANUP:

	if( FAILED(hr) ) 
	{
		SAFE_FREE(pPropInfo);
		SAFE_FREE(pPropInfoSet);
		if( ppDescBuffer )
		{
			SAFE_FREE(*ppDescBuffer);
			m_cwchNamePool = 0;
			m_pwchNamePool = NULL;
		}
	}

	if( fPropsSucceed && fPropsinError )
		return DB_S_ERRORSOCCURRED;
	else if( fPropsinError ) 
	{
		if( ppDescBuffer )
		{
			SAFE_FREE(*ppDescBuffer);
			m_cwchNamePool = 0;
			m_pwchNamePool = NULL;
		}
		return DB_E_ERRORSOCCURRED;
	}
	else
		return hr;
}

// CUtilProp::GetProperties ----------------------------------------------------
//
// @mfunc Returns current settings of all properties supported by the DSO/rowset
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcProperties or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CUtilProp::GetProperties
    (
		DWORD				dwBitMask,			//@parm IN  | Mask if Initialized
		ULONG				cPropertyIDSets,	//@parm IN | # of restiction property IDs
		const DBPROPIDSET	rgPropertyIDSets[],	//@parm IN | restriction guids
		ULONG*              pcPropertySets,		//@parm OUT | count of properties returned
		DBPROPSET**			prgPropertySets		//@parm OUT | property information returned
    )
{
    HRESULT			hr            = S_OK;
	BOOL			fPropsinError = FALSE;
	BOOL			fPropsSucceed = FALSE;
    ULONG			cProps		  = 0;
	ULONG			cCount		  = 0;
    ULONG			ulPropertySets= 0;
    DBPROP*			pProp;
    DBPROPSET*		pPropSet;

	// save the count of PropertyIDSets
	cProps = cPropertyIDSets;

	// If the consumer does not restrict the property sets
	// by specify an array of property sets and a cPropertySets
	// greater than 0, then we need to make sure we 
	// have some to return
	if( cPropertyIDSets == 0 )
	{
		// only allow the DBINIT, DATASOURCE and DATASOURCEINFO if Initialized
		if( dwBitMask & PROPSET_INIT )
			cProps = 2;
		else
			cProps = 1;
	}

    // use task memory allocater to alloc a DBPROPSET struct
    SAFE_ALLOC(pPropSet, DBPROPSET, cProps);
    memset(pPropSet, 0, cProps * sizeof(DBPROPSET));

	// For each supported Property Set
	for (ulPropertySets=0; ulPropertySets < cProps; ulPropertySets++)
	{
		BOOL fGetAllProps = FALSE;

		// If no restrictions return all properties from the three supported property sets
		if( cPropertyIDSets == 0 )
		{
				fGetAllProps = TRUE;
				
				// only do this once
				if( ulPropertySets == 0 )
				{
					if( !(dwBitMask & PROPSET_SESSION) )
					{						
						if( !(dwBitMask & PROPSET_ROWSET) )
						{
								pPropSet[0].guidPropertySet = DBPROPSET_DBINIT;
								pPropSet[0].cProperties  = NUMBER_OF_SUPPORTED_DBINIT_PROPERTIES;

							if( dwBitMask & PROPSET_INIT )
							{
								pPropSet[1].guidPropertySet = DBPROPSET_DATASOURCEINFO;
								pPropSet[1].cProperties  = NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES;
							}
						}
						else
						{
							pPropSet[0].guidPropertySet = DBPROPSET_ROWSET;
							pPropSet[0].cProperties  = NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES;
						}
					}
					else
					{
						pPropSet[0].guidPropertySet = DBPROPSET_SESSION;
						pPropSet[0].cProperties  = NUMBER_OF_SUPPORTED_SESSION_PROPERTIES;
					}
				}
		}
		else
		{
			pPropSet[ulPropertySets].guidPropertySet = rgPropertyIDSets[ulPropertySets].guidPropertySet;
			pPropSet[ulPropertySets].cProperties  = rgPropertyIDSets[ulPropertySets].cPropertyIDs;

			if( rgPropertyIDSets[ulPropertySets].cPropertyIDs == 0 )
			{
				fGetAllProps = TRUE;

				if( rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DBINIT &&
					dwBitMask & PROPSET_DSO )
				{
					pPropSet[ulPropertySets].cProperties  = NUMBER_OF_SUPPORTED_DBINIT_PROPERTIES;
				}
				else if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFO) &&
						 ((dwBitMask & PROPSET_DSOINIT) == PROPSET_DSOINIT) )
				{
					pPropSet[ulPropertySets].cProperties  = NUMBER_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES;
				}
				else if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_SESSION) &&
						 (dwBitMask & PROPSET_SESSION))
				{
					pPropSet[ulPropertySets].cProperties  = NUMBER_OF_SUPPORTED_SESSION_PROPERTIES;
				}
				else if( (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_ROWSET) &&
						 (dwBitMask & PROPSET_ROWSET))
				{
					pPropSet[ulPropertySets].cProperties  = NUMBER_OF_SUPPORTED_ROWSET_PROPERTIES;
				}
				else if( rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_PROPERTIESINERROR )
				{
					if( dwBitMask & PROPSET_DSO ) {
						pPropSet[ulPropertySets].guidPropertySet = DBPROPSET_DBINIT;
						fPropsSucceed = TRUE;
					}
					else if( dwBitMask & PROPSET_ROWSET ) {
						pPropSet[ulPropertySets].guidPropertySet = DBPROPSET_ROWSET;
						fPropsSucceed = TRUE;
					}
					else
						fPropsinError = TRUE;
				}
				else
				{
					fGetAllProps = FALSE;
					fPropsinError = TRUE;
				}
			}
		}
		
		if( pPropSet[ulPropertySets].cProperties )
		{
		    // use task memory allocater to alloc array of DBPROP structs
			pProp = (DBPROP*) PROVIDER_ALLOC(sizeof( DBPROP ) *
										 pPropSet[ulPropertySets].cProperties);

			if (!pProp)
			{
				for(ULONG ul=0; ul<ulPropertySets; ul++)
				{
					for(ULONG ul2=0; ul2<pPropSet[ul].cProperties; ul2++)
						VariantClear( &pPropSet[ul].rgProperties[ul2].vValue );

					SAFE_FREE( pPropSet[ul].rgProperties );
				}
				SAFE_FREE( pPropSet );

				return ResultFromScode( E_OUTOFMEMORY );
			}
		
			pPropSet[ulPropertySets].rgProperties = &pProp[0];

			memset( pProp, 0, 
				(pPropSet[ulPropertySets].cProperties * sizeof( DBPROP )));
		}

	    // for each prop in our table..
		for (cCount=0; cCount < pPropSet[ulPropertySets].cProperties; cCount++)
		{
			// init the Variant right up front
			// that way we can VariantClear with no worried (if we need to)
			VariantInit( &pProp[cCount].vValue );

			// Check supported property sets
			if ( pPropSet[ulPropertySets].guidPropertySet == DBPROPSET_DBINIT &&
				 fGetAllProps )
			{
				fPropsSucceed = TRUE;
				// load up their DBPROP from our table
				hr = LoadDBPROP( &m_rgproperties[START_OF_SUPPORTED_DBINIT_PROPERTIES + cCount], 
							&pProp[cCount] );
			}
			else if ( pPropSet[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFO &&
					  fGetAllProps )
			{
				fPropsSucceed = TRUE;
				// load up their DBPROPINFO from our table
				hr = LoadDBPROP( &m_rgproperties[START_OF_SUPPORTED_DATASOURCEINFO_PROPERTIES + cCount], 
							&pProp[cCount] );
			}
			else if ( pPropSet[ulPropertySets].guidPropertySet == DBPROPSET_SESSION &&
					  fGetAllProps )
			{
				fPropsSucceed = TRUE;
				// load up their DBPROPINFO from our table
				hr = LoadDBPROP( &m_rgproperties[START_OF_SUPPORTED_SESSION_PROPERTIES + cCount], 
							&pProp[cCount] );
			}
			else if ( pPropSet[ulPropertySets].guidPropertySet == DBPROPSET_ROWSET &&
					  fGetAllProps )
			{
				fPropsSucceed = TRUE;
				// load up their DBPROPINFO from our table
				hr = LoadDBPROP( &m_rgproperties[START_OF_SUPPORTED_ROWSET_PROPERTIES + cCount], 
							&pProp[cCount] );
			}
			else
			{
				ULONG ulIndex;

				pProp[cCount].dwPropertyID	= rgPropertyIDSets[ulPropertySets].rgPropertyIDs[cCount];
				pProp[cCount].dwStatus		= DBPROPSTATUS_NOTSUPPORTED;

				if(  (GetPropIndex(rgPropertyIDSets[ulPropertySets].rgPropertyIDs[cCount], &ulIndex)) &&
					 (((dwBitMask & PROPSET_DSO) && 
					   (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DBINIT)) ||
					  (((dwBitMask & PROPSET_DSOINIT) == PROPSET_DSOINIT) &&
					   ((rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCE) ||
						(rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_DATASOURCEINFO))) ||
					  ((dwBitMask & PROPSET_SESSION) && 
					   (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_SESSION)) ||
					  ((dwBitMask & PROPSET_ROWSET) && 
					   (rgPropertyIDSets[ulPropertySets].guidPropertySet == DBPROPSET_ROWSET))) )
				{
					fPropsSucceed = TRUE;
					hr = LoadDBPROP( &m_rgproperties[ulIndex], &pProp[cCount] );
				}
				else
					fPropsinError = TRUE;
			}
		}
		// Set local back to FALSE
		fGetAllProps = FALSE;
	}

	// set count of properties and property information
    *pcPropertySets	 = cProps;
    *prgPropertySets = pPropSet;

CLEANUP:

	if( FAILED(hr) ) 
	{
		// clear all variants used so far..
		for (ULONG ulFor=0; ulFor < cCount; ulFor++)
			VariantClear(&pProp[ulFor].vValue);

		// delete the pProp array, return failure
		SAFE_FREE(pProp);
		SAFE_FREE(pPropSet);
	}

	if ( (!fPropsSucceed && cPropertyIDSets) || (!fPropsSucceed && fPropsinError) )
		return ResultFromScode( DB_E_ERRORSOCCURRED );
	else if ( fPropsSucceed && fPropsinError )
		return ResultFromScode( DB_S_ERRORSOCCURRED );
	else
		return ResultFromScode( hr );
}


// CUtilProp::SetPropertiesArgChk -------------------------------------------
//
// @mfunc Initialize the buffers and check for E_INVALIDARG cases
//
// @rdesc HRESULT indicating the status of the method
//		@flag S_OK | Properties gathered
//		@flag E_INVALIDARG | Invalid parameter values
//
HRESULT CUtilProp::SetPropertiesArgChk
	(
	const ULONG		cPropertySets,		//@parm IN | Count of structs returned
	const DBPROPSET	rgPropertySets[]	//@parm IN | Array of Properties Sets
	)
{
	// Argument Checking
	if( !rgPropertySets )
		return ResultFromScode( E_INVALIDARG );

	// New argument check for > 1 cPropertyIDs and NULL pointer for 
	// array of property ids.
	for(ULONG ul=0; ul<cPropertySets; ul++)
	{
		if( rgPropertySets[ul].cProperties && !(rgPropertySets[ul].rgProperties) )
			return ResultFromScode( E_INVALIDARG );
	}
	
	return ResultFromScode( S_OK );
}


// CUtilProp::SetProperties ----------------------------------------------------
//
// @mfunc Set current settings of properties supported by the DSO/rowset
//
// @rdesc HRESULT
//      @flag S_OK          | The method succeeded
//      @flag E_INVALIDARG  | pcProperties or prgPropertyInfo was NULL
//      @flag E_OUTOFMEMORY | Out of memory
//
STDMETHODIMP CUtilProp::SetProperties
    (
		DWORD		dwBitMask,			//@parm IN		Type of PropSet
		ULONG		cPropertyIDSets,	//@parm IN		# of DBPROPSET
		DBPROPSET	rgPropertyIDSets[]	//@parm INOUT	Array of property sets
	)
{
	ULONG cCountProps = 0;
	ULONG cValidProps = 0;
	ULONG ulIndex     = 0;

	// For each supported Property Set
	for (ULONG cPropSets=0; cPropSets < cPropertyIDSets; cPropSets++)
	{
	    // for each prop in the propset
		for (ULONG cCnt=0; cCnt < rgPropertyIDSets[cPropSets].cProperties; cCnt++)
		{
			//Keep track of the number
			cCountProps++;

			// Check Check to see if valid
			switch( dwBitMask )
			{
				case PROPSET_DSO:
					if( rgPropertyIDSets[cPropSets].guidPropertySet == DBPROPSET_DBINIT &&
						GetPropIndex(rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwPropertyID, &ulIndex) )
						break;

					rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSUPPORTED;
					continue;

				case PROPSET_DSOINIT:
					if( (rgPropertyIDSets[cPropSets].guidPropertySet == DBPROPSET_DATASOURCEINFO ||
						 rgPropertyIDSets[cPropSets].guidPropertySet == DBPROPSET_DATASOURCE)  &&
						GetPropIndex(rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwPropertyID, &ulIndex) )
						break;
					
					if( rgPropertyIDSets[cPropSets].guidPropertySet == DBPROPSET_DBINIT )
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSETTABLE;
					else
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSUPPORTED;
					continue;

				case PROPSET_SESSION:
					if( rgPropertyIDSets[cPropSets].guidPropertySet == DBPROPSET_SESSION  &&
						GetPropIndex(rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwPropertyID, &ulIndex) )
						break;

					rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSUPPORTED;
					continue;

				case PROPSET_ROWSET:
					if( rgPropertyIDSets[cPropSets].guidPropertySet == DBPROPSET_ROWSET  &&
						GetPropIndex(rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwPropertyID, &ulIndex) )
						break;

					rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSUPPORTED;
					continue;

				default:
					assert("Unsupported dwBitMask");
			}
			
			// arg checking for the prop
			if( rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions != DBPROPOPTIONS_OPTIONAL &&
				rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions != DBPROPOPTIONS_REQUIRED )
			{
				rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_BADOPTION;
				continue;
			}
			
			if( (V_VT(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) != VT_EMPTY) &&
				(V_VT(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) != m_rgproperties[ulIndex].vtType ||
				 IsValidValue(&rgPropertyIDSets[cPropSets].rgProperties[cCnt]) == S_FALSE) ) 
			{
				rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_BADVALUE;
				continue;
			}
			
			switch( V_VT(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) )
			{
				case VT_BOOL:
					if( m_rgproperties[ulIndex].dwFlags & DBPROPFLAGS_WRITE ) {
						m_rgproperties[ulIndex].boolVal = !!(V_BOOL(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue));
						break;
					}

					if( !!(V_BOOL(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue)) == m_rgproperties[ulIndex].boolVal )
						break;

					if( rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions == DBPROPOPTIONS_OPTIONAL)
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSET;
					else
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSETTABLE;
					continue;

				case VT_I2:
					if( m_rgproperties[ulIndex].dwFlags & DBPROPFLAGS_WRITE ) {
						m_rgproperties[ulIndex].longVal = V_I2(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue);
						break;
					}

					if( V_I2(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) == (SHORT)m_rgproperties[ulIndex].longVal )
						break;

					if( rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions == DBPROPOPTIONS_OPTIONAL)
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSET;
					else
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSETTABLE;
					continue;

				case VT_I4:
					if( m_rgproperties[ulIndex].dwFlags & DBPROPFLAGS_WRITE ) {
						m_rgproperties[ulIndex].longVal = V_I4(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue);
						break;
					}

					if( (V_I4(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) == (LONG)m_rgproperties[ulIndex].longVal) ||
						(rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwPropertyID == DBPROP_UPDATABILITY &&
						 V_I4(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) & m_rgproperties[ulIndex].longVal) )
						break;

					if( rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions == DBPROPOPTIONS_OPTIONAL)
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSET;
					else
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSETTABLE;
					continue;

#ifdef _WIN64
                case VT_I8:
					if( m_rgproperties[ulIndex].dwFlags & DBPROPFLAGS_WRITE ) {
						m_rgproperties[ulIndex].longVal = V_I8(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue);
						break;
					}

					if( (V_I8(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) == (LONG)m_rgproperties[ulIndex].longVal) ||
						(rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwPropertyID == DBPROP_UPDATABILITY &&
						 V_I8(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue) & m_rgproperties[ulIndex].longVal) )
						break;

					if( rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions == DBPROPOPTIONS_OPTIONAL)
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSET;
					else
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSETTABLE;
					continue;
#endif

				case VT_BSTR:
					if( m_rgproperties[ulIndex].dwFlags & DBPROPFLAGS_WRITE ) {
						if( m_rgproperties[ulIndex].dwPropertyID == DBPROP_INIT_DATASOURCE ) {
							StringCchCopyW(m_wszFilePath,MAX_PATH,V_BSTR(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue));
							m_rgproperties[ulIndex].pwstrVal = m_wszFilePath;
						}
						break;
					}

					if( !(wcscmp(V_BSTR(&rgPropertyIDSets[cPropSets].rgProperties[cCnt].vValue), m_rgproperties[ulIndex].pwstrVal)) )
						break;

					if( rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwOptions == DBPROPOPTIONS_OPTIONAL)
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSET;
					else
						rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_NOTSETTABLE;
					continue;

				case VT_EMPTY:
					if( m_rgproperties[ulIndex].dwFlags & DBPROPFLAGS_WRITE )
						memcpy(&m_rgproperties[ulIndex], &s_rgprop[ulIndex], sizeof(PROPSTRUCT));
					break;
			}

			// Initialize dwStatus
			rgPropertyIDSets[cPropSets].rgProperties[cCnt].dwStatus = DBPROPSTATUS_OK;
			cValidProps++;
		}
	}
	
	// Figure out the retcode
	if( cValidProps == cCountProps )
		return ResultFromScode( S_OK );
	else if( cValidProps )
		return ResultFromScode( DB_S_ERRORSOCCURRED );
	else 
		return ResultFromScode( DB_E_ERRORSOCCURRED );
}
