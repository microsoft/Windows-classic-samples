//*-----------------------------------------------------------------------
//
//   This is the Test Module for IDBBinderProperties interface, which is a 
//	mandatory interface on BINDER objects.
//
//
//   WARNING:
//          PLEASE USE THE TEST CASE WIZARD TO ADD/DELETE TESTS AND VARIATIONS!
//
//
//   Copyright (C) 1994-2000 Microsoft Corporation
//*-----------------------------------------------------------------------


#include "MODStandard.hpp"
#include "IDBBinderProperties.h"
#include "ExtraLib.h"


//*-----------------------------------------------------------------------
// Module Values
//*-----------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x296f8450, 0x4124, 0x11d2, { 0x88, 0xd1, 0x00, 0x60, 0x08, 0x9f, 0xc4, 0x66} };
DECLARE_MODULE_NAME("IDBBinderProperties");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IDBBinderProperties interface.");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(FALSE)
// }} TCW_MODULE_GLOBALS_END



//*-----------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	TBEGIN
	IBindResource*	pIBR = NULL;

	TESTC(CreateModInfo(pThisTestModule))

	if(!IsUsableInterface(BINDER_INTERFACE, IID_IDBBinderProperties))
	{
		odtLog<<L"SKIP: CONF_STRICT specified and IDBBinderProperties is a Level-1 interface.\n";
		return TEST_SKIPPED;
	}

	pIBR = GetModInfo()->GetRootBinder();
	TESTC_PROVIDER(pIBR != NULL)

CLEANUP:
	TRETURN
}

//*-----------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
//      @flag  TRUE  | Successful initialization
//      @flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	return ReleaseModInfo(pThisTestModule);
}



////////////////////////////////////////////////////////////////////////
//CBinderProp Class
//
////////////////////////////////////////////////////////////////////////
class CBinderProp : public CSessionObject
{
public:

	//Constructor
	CBinderProp(WCHAR* pwszTestCaseName);

	//Destructor
	virtual ~CBinderProp();

protected:

//VARIABLES...

	HRESULT				m_hr;
	ULONG				m_cPropSets;
	DBPROPSET*			m_rgPropSets;
	ULONG				m_cPropSets2;
	DBPROPSET*			m_rgPropSets2;

//INTERFACES...

	IDBBinderProperties*	m_pIDBBinderProperties;

//METHODS...

	//Release all member pointers to interfaces.
	BOOL	ReleaseAll();

	//Wrapper for m_pIDBBinderProperties->GetProperties()
	HRESULT	GetProps(
		ULONG			cPropIDSets,
		DBPROPIDSET*	rgPropIDSets,
		ULONG*			pcPropSets,
		DBPROPSET**		prgPropSets);

	//Wrapper for m_pIDBBinderProperties->GetPropertyInfo()
	HRESULT	GetPropInfos(
		ULONG			cPropIDSets,
		DBPROPIDSET*	rgPropIDSets,
		ULONG*			pcPropInfoSets,
		DBPROPINFOSET**	prgPropInfoSets,
		WCHAR**			ppwszDescBuffer);

	//Wrapper for m_pIDBBinderProperties->SetProperties()
	HRESULT	SetProps(
		ULONG		cPropSets,
		DBPROPSET*	rgPropSets);

	//Wrapper for m_pIDBBinderProperties->Reset()
	HRESULT	Reset();

	//Get an instance of the Root Binder and QI for IDBBinderProperties.
	BOOL	GetRootBinder();

	//Wrapper for the same EXTRALIB function.
	BOOL	SetProperty(
		DBPROPID		PropertyID, 
		GUID			guidPropertySet, 
		void*			pValue = (void*)VARIANT_TRUE, 
		DBTYPE			wType = DBTYPE_BOOL, 
		BOOL			bSetonBothSets = FALSE,
		DBPROPOPTIONS	dwOptions = DBPROPOPTIONS_REQUIRED,
		DBID			colid = DB_NULLID);

	//Verify the properties obtained by calling IDBBinderProperties::
	//GetProperties()
	BOOL	VerifyGetProps(
		HRESULT		hr, 
		ULONG		cPropSets,
		DBPROPSET*	rgPropSets);

	//Verify the property infos obtained by calling IDBBinderProperties::
	//GetPropertyInfo()
	BOOL	VerifyPropInfos(
		ULONG			cPropIDSets,
		DBPROPIDSET*	rgPropIDSets,
		ULONG			cPropInfoSets,
		DBPROPINFOSET*	rgPropInfoSets);

	//Verify the Init properties set by calling IDBBinderProperties::
	//SetProperties()
	BOOL	VerifySetProps(
		ULONG			cPropSets,
		DBPROPSET*		rgPropSets);

	BOOL	VerifyReset(BOOL bDefault = FALSE);

	BOOL	PropsToIDs(
		ULONG			cPropSets,
		DBPROPSET*		rgPropSets,
		ULONG*			pcPropIDSets,
		DBPROPIDSET**	prgPropIDSets);

	BOOL	FreeProps();

};


////////////////////////////////////////////////////////////////////////
//CBinderProp Implementation
//
////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------
//  CBinderProp::CBinderProp
//
CBinderProp::CBinderProp(WCHAR * pwszTestCaseName)	: CSessionObject(pwszTestCaseName) 
{
	m_cPropSets			= 0;
	m_rgPropSets		= NULL;
	m_cPropSets2		= 0;
	m_rgPropSets2		= NULL;

	m_pIDBBinderProperties	= NULL;
}

//----------------------------------------------------------------------
//  CBinderProp::~CBinderProp
//
CBinderProp::~CBinderProp()
{
	ReleaseAll();
}

//----------------------------------------------------------------------
// CBinderProp::ReleaseAll
//
BOOL CBinderProp::ReleaseAll()
{
	FreeProps();

	SAFE_RELEASE(m_pIDBBinderProperties);

	return TRUE;
} //ReleaseAll

//----------------------------------------------------------------------
// CBinderProp::GetProps
//
HRESULT	CBinderProp::GetProps(
	ULONG			cPropIDSets,
	DBPROPIDSET*	rgPropIDSets,
	ULONG*			pcPropSets,
	DBPROPSET**		prgPropSets)
{
	HRESULT		hr = E_FAIL;

	if(!m_pIDBBinderProperties)
		return E_FAIL;

	hr = m_pIDBBinderProperties->GetProperties(cPropIDSets, rgPropIDSets,
		pcPropSets, prgPropSets);

	return hr;
} //GetProps

//----------------------------------------------------------------------
// CBinderProp::VerifyGetProps
//
BOOL CBinderProp::VerifyGetProps(
	HRESULT		hr, 
	ULONG		cPropSets,
	DBPROPSET*	rgPropSets)
{
	TBEGIN
	ULONG		iSet, iProp;
	ULONG		ulPos = 0;
	ULONG		ulSup=0, ulTotal=0;
	DBPROP*		pProp = NULL;

	TESTC(hr==S_OK || hr==DB_S_ERRORSOCCURRED || hr==DB_E_ERRORSOCCURRED)
	TESTC(cPropSets>0 && rgPropSets!=NULL)

	for(iSet=0; iSet<cPropSets; iSet++)
	{
		if(!(rgPropSets[iSet].cProperties) && 
			!(rgPropSets[iSet].rgProperties))
		{
			odtLog<<L"INFO: The PropSet "<<GetPropSetName(rgPropSets[iSet].guidPropertySet)<<L" does not have any properties in it.\n";
			ulTotal++;
			continue;
		}

		TESTC(rgPropSets[iSet].cProperties>0 && 
			rgPropSets[iSet].rgProperties!=NULL)

		for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
		{
			ulTotal++;
			pProp = &(rgPropSets[iSet].rgProperties[iProp]);
			if(pProp->dwStatus == DBPROPSTATUS_OK)
			{
				ulSup++;

				COMPARE( (rgPropSets[iSet].guidPropertySet != DBPROPSET_DBINITALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_DATASOURCEALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_DATASOURCEINFOALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_SESSIONALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_ROWSETALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_COLUMNALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_INDEXALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_TABLEALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_TRUSTEEALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_CONSTRAINTALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_VIEWALL) &&
					(rgPropSets[iSet].guidPropertySet != DBPROPSET_PROPERTIESINERROR), TRUE);
			}
			else
				COMPARE(pProp->dwStatus, DBPROPSTATUS_NOTSET);
		}
	}

	if(hr==S_OK)
		TESTC(ulSup==ulTotal)
	else if(hr==DB_S_ERRORSOCCURRED)
		TESTC(ulSup>0 && ulSup<ulTotal)
	else
		TESTC(ulSup==0 && ulTotal>0) 

CLEANUP:
	TRETURN
} //VerifyGetProps

//----------------------------------------------------------------------
// CBinderProp::GetPropInfos
//
HRESULT	CBinderProp::GetPropInfos(
	ULONG			cPropIDSets,
	DBPROPIDSET*	rgPropIDSets,
	ULONG*			pcPropInfoSets,
	DBPROPINFOSET**	prgPropInfoSets,
	WCHAR**			ppwszDescBuffer)
{
	HRESULT		hr = E_FAIL;

	if(!m_pIDBBinderProperties)
		return E_FAIL;

	hr = m_pIDBBinderProperties->GetPropertyInfo(cPropIDSets, rgPropIDSets,
		pcPropInfoSets, prgPropInfoSets, ppwszDescBuffer);

	return hr;
} //GetPropInfos

//----------------------------------------------------------------------
// CBinderProp::VerifyPropInfos
//
BOOL CBinderProp::VerifyPropInfos(
	ULONG			cPropIDSets,
	DBPROPIDSET*	rgPropIDSets,
	ULONG			cPropInfoSets,
	DBPROPINFOSET*	rgPropInfoSets)
{
	TBEGIN
	ULONG			iSet, iProp;

	TESTC(cPropInfoSets>0 && rgPropInfoSets!=NULL)
	TESTC(cPropInfoSets == cPropIDSets)

	for(iSet=0; iSet<cPropInfoSets; iSet++)
	{
		TESTC(rgPropInfoSets[iSet].guidPropertySet == rgPropIDSets[iSet].guidPropertySet)

		if(rgPropInfoSets[iSet].cPropertyInfos == 0)
		{
			TESTC(!(rgPropIDSets[iSet].cPropertyIDs))
			TESTC(!(rgPropInfoSets[iSet].rgPropertyInfos))
			odtLog<<L"INFO: cPropertyInfos is 0 and rgPropertyInfos is NULL for "<<GetPropSetName(rgPropInfoSets[iSet].guidPropertySet)<<".\n";
		}
		else
		{
			TESTC(rgPropIDSets[iSet].cPropertyIDs == rgPropInfoSets[iSet].cPropertyInfos)
			TESTC(rgPropInfoSets[iSet].rgPropertyInfos != NULL)
			for(iProp=0; iProp<rgPropInfoSets[iSet].cPropertyInfos; iProp++)
			{
				TESTC(rgPropInfoSets[iSet].rgPropertyInfos[iProp].dwPropertyID == rgPropIDSets[iSet].rgPropertyIDs[iProp])
				TESTC(rgPropInfoSets[iSet].rgPropertyInfos[iProp].dwFlags == DBPROPFLAGS_NOTSUPPORTED)
				TESTC(rgPropInfoSets[iSet].rgPropertyInfos[iProp].vtType == VT_EMPTY)
			}
		}
	}

CLEANUP:
	TRETURN
} //VerifyPropInfos

//----------------------------------------------------------------------
// CBinderProp::SetProps
//
HRESULT	CBinderProp::SetProps(
	ULONG		cPropSets,
	DBPROPSET*	rgPropSets)
{
	HRESULT		hr = E_FAIL;

	if(!m_pIDBBinderProperties)
		return E_FAIL;

	hr = m_pIDBBinderProperties->SetProperties(cPropSets, rgPropSets);

	return hr;
} //SetProps

//----------------------------------------------------------------------
// CBinderProp::VerifySetProps
//
BOOL CBinderProp::VerifySetProps(
	ULONG			cPropSets,
	DBPROPSET*		rgPropSets)
{
	TBEGIN
	HRESULT			hr = E_FAIL;
	ULONG			iSet,iProp;
	ULONG			cPropIDSets = 0;
	DBPROPIDSET*	rgPropIDSets = NULL;
	ULONG			cPropSetsGot = 0;
	DBPROPSET*		rgPropSetsGot = NULL;
	DBPROP*			pProp = NULL;

	TESTC(cPropSets>0 && rgPropSets!=NULL)

	for(iSet=0; iSet<cPropSetsGot; iSet++)
		for(iProp=0; iProp<rgPropSetsGot[iSet].cProperties; iProp++)
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);

	TESTC(PropsToIDs(cPropSets, rgPropSets, &cPropIDSets, &rgPropIDSets))

	TEST3C_(hr=GetProps(cPropIDSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot),
		S_OK, DB_S_ERRORSOCCURRED, DB_E_ERRORSOCCURRED)

	CHECKW(hr, S_OK); //Warn if hr is not S_OK.

	if(cPropSets)
		TESTC(VerifyGetProps(hr, cPropSetsGot, rgPropSetsGot))

	for(iSet=0; iSet<cPropSetsGot; iSet++)
	{
		for(iProp=0; iProp<rgPropSetsGot[iSet].cProperties; iProp++)
		{
			pProp = &(rgPropSetsGot[iSet].rgProperties[iProp]);

			if(rgPropSets[iSet].rgProperties[iProp].dwStatus == DBPROPSTATUS_OK)
				TESTC(CompareVariant(&pProp->vValue, &rgPropSets[iSet].rgProperties[iProp].vValue))
		}
	}

CLEANUP:
	FreeProperties(&cPropSetsGot, &rgPropSetsGot);
	FreeProperties(&cPropIDSets, &rgPropIDSets);
	TRETURN
} //VerifySetProps

//----------------------------------------------------------------------
// CBinderProp::Reset
//
HRESULT	CBinderProp::Reset()
{
	if(!m_pIDBBinderProperties)
		return E_FAIL;
	else
		return m_pIDBBinderProperties->Reset();
} //Reset

//----------------------------------------------------------------------
// CBinderProp::VerifyReset
//
BOOL CBinderProp::VerifyReset(BOOL bDefault)
{
	TBEGIN
	HRESULT				hr = E_FAIL;
	ULONG				cPropSets = 0;
	DBPROPSET*			rgPropSets = NULL;

	TESTC_(hr=Reset(), S_OK)

	TESTC_(hr=GetProps(0,NULL, &cPropSets, &rgPropSets), S_OK)

	TESTC(cPropSets==0 && rgPropSets==NULL)

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} //VerifyReset

//----------------------------------------------------------------------
// CBinderProp::GetRootBinder
//
BOOL CBinderProp::GetRootBinder()
{
	TBEGIN
	IBindResource*	pIBR = NULL;

	pIBR = GetModInfo()->GetRootBinder();

	TESTC(VerifyInterface(pIBR, IID_IDBBinderProperties,
		BINDER_INTERFACE,(IUnknown**)&m_pIDBBinderProperties))

	//Reset state of Root Binder.
	TESTC_(Reset(), S_OK)
	
CLEANUP:
	TRETURN;
} //GetRootBinder

//----------------------------------------------------------------------
// CBinderProp::SetProperty
//
BOOL CBinderProp::SetProperty(
		DBPROPID		PropertyID, 
		GUID			guidPropertySet, 
		void*			pValue, 
		DBTYPE			wType, 
		BOOL			bSetonBothSets,
		DBPROPOPTIONS	dwOptions,
		DBID			colid)
{
	COMPARE(::SetProperty(PropertyID, guidPropertySet, &m_cPropSets, 
		&m_rgPropSets, pValue, wType, dwOptions, colid), TRUE);

	if(bSetonBothSets)
		COMPARE(::SetProperty(PropertyID, guidPropertySet, &m_cPropSets2, 
		&m_rgPropSets2, pValue, wType, dwOptions, colid), TRUE);

	return TRUE;
} //SetProperty

//----------------------------------------------------------------------
// CBinderProp::PropsToIDs
//
BOOL CBinderProp::PropsToIDs(
		ULONG			cPropSets,
		DBPROPSET*		rgPropSets,
		ULONG*			pcPropIDSets,
		DBPROPIDSET**	prgPropIDSets)
{
	TBEGIN
	ULONG			iSet,iProp;

	TESTC(cPropSets>0 && rgPropSets!=NULL)
	TESTC(pcPropIDSets != NULL && prgPropIDSets != NULL)

	for(iSet=0; iSet<cPropSets; iSet++)
		for(iProp=0; iProp<rgPropSets[iSet].cProperties; iProp++)
		{
			COMPARE(::SetProperty(rgPropSets[iSet].rgProperties[iProp].dwPropertyID, 
				rgPropSets[iSet].guidPropertySet, pcPropIDSets, 
				prgPropIDSets), TRUE);
		}

CLEANUP:
	TRETURN
} //PropsToIDs

//----------------------------------------------------------------------
// CBinderProp::FreeProps
//
BOOL CBinderProp::FreeProps()
{
	TBEGIN

	if(m_cPropSets && m_rgPropSets)
		COMPARE(FreeProperties(&m_cPropSets, &m_rgPropSets), TRUE);

	if(m_cPropSets2 && m_rgPropSets2)
		COMPARE(FreeProperties(&m_cPropSets2, &m_rgPropSets2), TRUE);

	m_cPropSets = 0;
	m_cPropSets2 = 0;

	TRETURN
} //FreeProperties(&cPropSets, &rgPropSets);s



//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------


// {{ TCW_TEST_CASE_MAP(TCGetAndSetProps)
//*-----------------------------------------------------------------------
// @class Test the GetProperties method
//
class TCGetAndSetProps : public CBinderProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetAndSetProps,CBinderProp);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Get all properties with no properties set.
	int Variation_1();
	// @cmember Set and Get some DBINIT properties.
	int Variation_2();
	// @cmember Set and Get some DATASOURCEINFO properties.
	int Variation_3();
	// @cmember Set and Get some SESSION properties.
	int Variation_4();
	// @cmember Set and Get some ROWSET properties.
	int Variation_5();
	// @cmember Set and Get a combination of properties.
	int Variation_6();
	// @cmember Set and overwrite all props.
	int Variation_7();
	// @cmember Set props, set new props and overwrite old ones.
	int Variation_8();
	// @cmember Set props from Static Array (stack)
	int Variation_9();
	// @cmember Set props with bad status and bad variant types
	int Variation_10();
	// @cmember Set props with all sorts of variants
	int Variation_11();
	// @cmember Set same property with different colids.
	int Variation_12();
	// @cmember Set diffr properties with same colids.
	int Variation_13();
	// @cmember Verify release of props thru ref counts.
	int Variation_14();
	// @cmember Duplicate prop in a set.
	int Variation_15();
	// @cmember Duplicate prop in duplicate sets.
	int Variation_16();
	// @cmember Duplicate props (multiple) in duplicate sets.
	int Variation_17();
	// @cmember GetProperties: E_INVALIDARG cases
	int Variation_18();
	// @cmember SetProperties: E_INVALIDARG cases
	int Variation_19();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGetAndSetProps)
#define THE_CLASS TCGetAndSetProps
BEG_TEST_CASE(TCGetAndSetProps, CBinderProp, L"Test the GetProperties method")
	TEST_VARIATION(1, 		L"Get all properties with no properties set.")
	TEST_VARIATION(2, 		L"Set and Get some DBINIT properties.")
	TEST_VARIATION(3, 		L"Set and Get some DATASOURCEINFO properties.")
	TEST_VARIATION(4, 		L"Set and Get some SESSION properties.")
	TEST_VARIATION(5, 		L"Set and Get some ROWSET properties.")
	TEST_VARIATION(6, 		L"Set and Get a combination of properties.")
	TEST_VARIATION(7, 		L"Set and overwrite all props.")
	TEST_VARIATION(8, 		L"Set props, set new props and overwrite old ones.")
	TEST_VARIATION(9, 		L"Set props from Static Array (stack)")
	TEST_VARIATION(10, 		L"Set props with bad status and bad variant types")
	TEST_VARIATION(11, 		L"Set props with all sorts of variants")
	TEST_VARIATION(12, 		L"Set same property with different colids.")
	TEST_VARIATION(13, 		L"Set diffr properties with same colids.")
	TEST_VARIATION(14, 		L"Verify release of props thru ref counts.")
	TEST_VARIATION(15, 		L"Duplicate prop in a set.")
	TEST_VARIATION(16, 		L"Duplicate prop in duplicate sets.")
	TEST_VARIATION(17, 		L"Duplicate props (multiple) in duplicate sets.")
	TEST_VARIATION(18, 		L"GetProperties: E_INVALIDARG cases")
	TEST_VARIATION(19, 		L"SetProperties: E_INVALIDARG cases")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCGetPropertyInfo)
//*-----------------------------------------------------------------------
// @class Test the GetPropertyInfo method.
//
class TCGetPropertyInfo : public CBinderProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetPropertyInfo,CBinderProp);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember GetPropInfo (0,NULL)
	int Variation_1();
	// @cmember GetPropInfo(DBPROPSET_*ALL)
	int Variation_2();
	// @cmember Get DBINIT, DATASOURCEINFO, SESSION and ROWSET PropInfo
	int Variation_3();
	// @cmember Get a combination of PropInfos
	int Variation_4();
	// @cmember E_INVALIDARG: cPropIDSets=1 but rgPropIDSets=NULL
	int Variation_5();
	// @cmember E_INVALIDARG: cPropID=1 but rgPropIDs=NULL
	int Variation_6();
	// @cmember E_INVALIDARG: pcPropInfoSets=NULL or rgPropInfoSets=NULL
	int Variation_7();
	// @cmember E_INVALIDARG: A special and a normal property set
	int Variation_8();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGetPropertyInfo)
#define THE_CLASS TCGetPropertyInfo
BEG_TEST_CASE(TCGetPropertyInfo, CBinderProp, L"Test the GetPropertyInfo method.")
	TEST_VARIATION(1, 		L"GetPropInfo (0,NULL)")
	TEST_VARIATION(2, 		L"GetPropInfo(DBPROPSET_*ALL)")
	TEST_VARIATION(3, 		L"Get DBINIT, DATASOURCEINFO, SESSION and ROWSET PropInfo")
	TEST_VARIATION(4, 		L"Get a combination of PropInfos")
	TEST_VARIATION(5, 		L"E_INVALIDARG: cPropIDSets=1 but rgPropIDSets=NULL")
	TEST_VARIATION(6, 		L"E_INVALIDARG: cPropID=1 but rgPropIDs=NULL")
	TEST_VARIATION(7, 		L"E_INVALIDARG: pcPropInfoSets=NULL or rgPropInfoSets=NULL")
	TEST_VARIATION(8, 		L"E_INVALIDARG: A special and a normal property set")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCReset)
//*-----------------------------------------------------------------------
// @class Test the Reset method.
//
class TCReset : public CBinderProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCReset,CBinderProp);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - SetProps(0, NULL)
	int Variation_1();
	// @cmember General - Call SetProps in stages.
	int Variation_2();
	// @cmember General - Call SetProp on same props multiple times.
	int Variation_3();
	// @cmember DBINIT - Set CacheAuthInfo, AuthEncryptPassword, Asynch, Mode.
	int Variation_4();
	// @cmember DBINIT - Set MaskPassword, PersistSenAuthInfo, ImpersLevel, Timeout.
	int Variation_5();
	// @cmember DBINIT - Set PersistEncrypted, Prompt, BindFlags, LockOwner.
	int Variation_6();
	// @cmember ROWSET - Set AbortPreserve, AccessOrder, AppendOnly, BlockingStorObj.
	int Variation_7();
	// @cmember ROWSET - Set Bookmarks, BkmSkip, BkmType, CacheDefer.
	int Variation_8();
	// @cmember ROWSET - Set CanFetchBack, CanHoldRows, CanScrollBack,  ChangeInsertedRows.
	int Variation_9();
	// @cmember ROWSET - Set ClientCursor, CmdTimeout, CommitPreserve, Deferred.
	int Variation_10();
	// @cmember ROWSET - Set DelayStorObj, HiddenCols, ImmobileRows, LiteralBkms.
	int Variation_11();
	// @cmember ROWSET - Set MaxRows, MayWriteCol, MemUsage, OrderedBkms.
	int Variation_12();
	// @cmember ROWSET - Set OtherInsert, OtherUpdateDelete, OwnInsert, OwnUpdateDelete.
	int Variation_13();
	// @cmember ROWSET - Set QuickRestart, RemoveDeleted,  RowsetAsynch, RowThreadModel.
	int Variation_14();
	// @cmember ROWSET - Set ServerCursor, ServerDataOnInsert, TransactedObj, UniqueRows.
	int Variation_15();
	// @cmember ROWSET - Set Updatability, IRowsetChange, IColumnsRowset, IRow.
	int Variation_16();
	// @cmember ROWSET - Set IRowsetChange, IRowsetFind, IRowsetIdentity, IRowsetIndex.
	int Variation_17();
	// @cmember ROWSET - Set IRowsetLocate, IRowsetRefresh, IRowsetResynch, IRowsetScroll.
	int Variation_18();
	// @cmember ROWSET - Set IRowsetUpdate, IRowsetView, IViewChapter, IViewFilter, IViewRowset.
	int Variation_19();
	// @cmember ROWSET - Set IViewSort, ISupportErrorInfo, ILockBytes, ISequentialStream.
	int Variation_20();
	// @cmember ROWSET - Set IStorage, IStream, CanHoldRows, IRowsetIdentity.
	int Variation_21();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCReset)
#define THE_CLASS TCReset
BEG_TEST_CASE(TCReset, CBinderProp, L"Test the Reset method.")
	TEST_VARIATION(1, 		L"General - SetProps(0, NULL)")
	TEST_VARIATION(2, 		L"General - Call SetProps in stages.")
	TEST_VARIATION(3, 		L"General - Call SetProp on same props multiple times.")
	TEST_VARIATION(4, 		L"DBINIT - Set CacheAuthInfo, AuthEncryptPassword, Asynch, Mode.")
	TEST_VARIATION(5, 		L"DBINIT - Set MaskPassword, PersistSenAuthInfo, ImpersLevel, Timeout.")
	TEST_VARIATION(6, 		L"DBINIT - Set PersistEncrypted, Prompt, BindFlags, LockOwner.")
	TEST_VARIATION(7, 		L"ROWSET - Set AbortPreserve, AccessOrder, AppendOnly, BlockingStorObj.")
	TEST_VARIATION(8, 		L"ROWSET - Set Bookmarks, BkmSkip, BkmType, CacheDefer.")
	TEST_VARIATION(9, 		L"ROWSET - Set CanFetchBack, CanHoldRows, CanScrollBack,  ChangeInsertedRows.")
	TEST_VARIATION(10, 		L"ROWSET - Set ClientCursor, CmdTimeout, CommitPreserve, Deferred.")
	TEST_VARIATION(11, 		L"ROWSET - Set DelayStorObj, HiddenCols, ImmobileRows, LiteralBkms.")
	TEST_VARIATION(12, 		L"ROWSET - Set MaxRows, MayWriteCol, MemUsage, OrderedBkms.")
	TEST_VARIATION(13, 		L"ROWSET - Set OtherInsert, OtherUpdateDelete, OwnInsert, OwnUpdateDelete.")
	TEST_VARIATION(14, 		L"ROWSET - Set QuickRestart, RemoveDeleted,  RowsetAsynch, RowThreadModel.")
	TEST_VARIATION(15, 		L"ROWSET - Set ServerCursor, ServerDataOnInsert, TransactedObj, UniqueRows.")
	TEST_VARIATION(16, 		L"ROWSET - Set Updatability, IRowsetChange, IColumnsRowset, IRow.")
	TEST_VARIATION(17, 		L"ROWSET - Set IRowsetChange, IRowsetFind, IRowsetIdentity, IRowsetIndex.")
	TEST_VARIATION(18, 		L"ROWSET - Set IRowsetLocate, IRowsetRefresh, IRowsetResynch, IRowsetScroll.")
	TEST_VARIATION(19, 		L"ROWSET - Set IRowsetUpdate, IRowsetView, IViewChapter, IViewFilter, IViewRowset.")
	TEST_VARIATION(20, 		L"ROWSET - Set IViewSort, ISupportErrorInfo, ILockBytes, ISequentialStream.")
	TEST_VARIATION(21, 		L"ROWSET - Set IStorage, IStream, CanHoldRows, IRowsetIdentity.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCSpecialCases)
//*-----------------------------------------------------------------------
// @class Special scenarios
//
class TCSpecialCases : public CBinderProp { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCSpecialCases,CBinderProp);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Set a valid property and set it again with unrecognised VT type.
	int Variation_1();
	// @cmember Set a property, then set it again with another (valid) value.
	int Variation_2();
	// @cmember Set 2 props - one valid and one invalid.
	int Variation_3();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCSpecialCases)
#define THE_CLASS TCSpecialCases
BEG_TEST_CASE(TCSpecialCases, CBinderProp, L"Special scenarios")
	TEST_VARIATION(1, 		L"Set a valid property and set it again with unrecognised VT type.")
	TEST_VARIATION(2, 		L"Set a property, then set it again with another (valid) value.")
	TEST_VARIATION(3, 		L"Set 2 props - one valid and one invalid.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()


// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(4, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCGetAndSetProps)
	TEST_CASE(2, TCGetPropertyInfo)
	TEST_CASE(3, TCReset)
	TEST_CASE(4, TCSpecialCases)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END



// {{ TCW_TC_PROTOTYPE(TCGetAndSetProps)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetAndSetProps - Test the GetProperties method
//| Created:  	8/31/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetAndSetProps::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CBinderProp::Init())
	// }}
	{ 
		return GetRootBinder();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Get all properties with no properties set.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_1()
{ 
	TBEGIN
	ULONG			iSet, iProp;
	DBPROPID		rgPropID[5];
	ULONG			cPropIDSets = 2;
	DBPROPIDSET		rgPropIDSets[2];
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;

	//Make sure no props are set. Then call GetProperties
	//with various parameters.
	TESTC_(Reset(), S_OK)

	//Call GetProperties with (0,NULL) as prop sets.
	TESTC_(m_hr=GetProps(0,NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(!cPropSets && !rgPropSets)

	//Call GetProperties with prop sets like (0,NULL,GUID)

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropIDSets[1].cPropertyIDs = 0;
	rgPropIDSets[1].rgPropertyIDs = NULL;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSET;

	TESTC_(m_hr=GetProps(cPropIDSets,rgPropIDSets, &cPropSets, &rgPropSets), DB_E_ERRORSOCCURRED)
	TESTC(cPropSets==2 && rgPropSets!=NULL)
	TESTC(!rgPropSets[0].cProperties && !rgPropSets[0].rgProperties)
	TESTC(rgPropSets[0].guidPropertySet == DBPROPSET_DBINIT)
	TESTC(!rgPropSets[1].cProperties && !rgPropSets[1].rgProperties)
	TESTC(rgPropSets[1].guidPropertySet == DBPROPSET_ROWSET)

	SAFE_FREE(rgPropSets);

	//Call GetProperties with prop sets containing properties.

	rgPropID[0] = DBPROP_INIT_PROMPT;
	rgPropID[1] = DBPROP_IRowset;
	rgPropID[2] = DBPROP_IRow;
	rgPropID[3] = DBPROP_IRowsetInfo;
	rgPropID[4] = DBPROP_IGetRow;
	rgPropIDSets[0].cPropertyIDs = 2;
	rgPropIDSets[0].rgPropertyIDs = &(rgPropID[0]);
	rgPropIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropIDSets[1].cPropertyIDs = 3;
	rgPropIDSets[1].rgPropertyIDs = &(rgPropID[2]);
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSET;

	TESTC_(m_hr=GetProps(cPropIDSets,rgPropIDSets, &cPropSets, &rgPropSets), DB_E_ERRORSOCCURRED)
	TESTC(cPropSets==2 && rgPropSets!=NULL)

	for(iSet=0; iSet<cPropIDSets; iSet++)
	{
		COMPARE(rgPropSets[iSet].guidPropertySet, rgPropIDSets[iSet].guidPropertySet);

		for(iProp=0; iProp<rgPropIDSets[iSet].cPropertyIDs; iProp++)
		{
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwPropertyID, rgPropID[2*iSet+iProp]);
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_NOTSET);
			COMPARE(V_VT(&rgPropSets[iSet].rgProperties[iProp].vValue), VT_EMPTY);
		}
	}

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set and Get some DBINIT properties.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_2()
{ 
	TBEGIN
	ULONG			iProp;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBPROP*			pProp = NULL;
	DBPROPIDSET		rgPropIDSet[1];
	BSTR			bstrLockOwner = L"IDBBinderProperties Test Module";

	TESTC_(Reset(), S_OK)

	//Set some DBINIT props.
	SetProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);
	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, (void*)DBBINDURLFLAG_SHARE_DENY_WRITE, DBTYPE_I4);
	SetProperty(DBPROP_INIT_LOCKOWNER, DBPROPSET_DBINIT, (void*)bstrLockOwner, DBTYPE_BSTR);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	rgPropIDSet[0].cPropertyIDs = 0;
	rgPropIDSet[0].rgPropertyIDs = NULL;
	rgPropIDSet[0].guidPropertySet = DBPROPSET_DBINIT;

	//Get the DBINIT props passing in GUID to GetProperties.
	//Then verify.

	TESTC_(m_hr=GetProps(1,rgPropIDSet, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets==1 && rgPropSets!=NULL)
	COMPARE(rgPropSets[0].guidPropertySet, DBPROPSET_DBINIT);
	for(iProp=0; iProp<m_rgPropSets[0].cProperties; iProp++)
	{
		pProp = &(m_rgPropSets[0].rgProperties[iProp]);
		COMPARE(rgPropSets[0].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
		COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[0].rgProperties[iProp].vValue), TRUE);
	}

	FreeProperties(&cPropSets, &rgPropSets);;

	//Get the DBINIT props passing in 0,NULL to GetProperties.
	//Then verify.

	TESTC_(m_hr=GetProps(0,NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets==1 && rgPropSets!=NULL)
	COMPARE(rgPropSets[0].guidPropertySet, DBPROPSET_DBINIT);
	for(iProp=0; iProp<m_rgPropSets[0].cProperties; iProp++)
	{
		pProp = &(m_rgPropSets[0].rgProperties[iProp]);
		COMPARE(rgPropSets[0].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
		COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[0].rgProperties[iProp].vValue), TRUE);
	}

CLEANUP:
	FreeProps();
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set and Get some DATASOURCEINFO properties.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_3()
{ 
	TBEGIN
	ULONG			iSet, iProp;
	DBPROPID		rgPropID[5];
	ULONG			cPropIDSets = 2;
	DBPROPIDSET		rgPropIDSets[2];
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;

	TESTC_(Reset(), S_OK)

	//Set 2 VT_BOOL properties.
	SetProperty(DBPROP_BYREFACCESSORS, DBPROPSET_DATASOURCEINFO);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	//Try to get the set props and some more unset ones.

	rgPropID[0] = DBPROP_INIT_PROMPT;
	rgPropID[1] = DBPROP_IRowset;
	rgPropID[2] = DBPROP_BYREFACCESSORS;
	rgPropID[3] = DBPROP_OLEOBJECTS;
	rgPropID[4] = DBPROP_IGetRow;
	rgPropIDSets[0].cPropertyIDs = 2;
	rgPropIDSets[0].rgPropertyIDs = &(rgPropID[0]);
	rgPropIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropIDSets[1].cPropertyIDs = 3;
	rgPropIDSets[1].rgPropertyIDs = &(rgPropID[2]);
	rgPropIDSets[1].guidPropertySet = DBPROPSET_DATASOURCEINFO;

	TESTC_(m_hr=GetProps(cPropIDSets,rgPropIDSets, &cPropSets, &rgPropSets), DB_S_ERRORSOCCURRED)
	TESTC(cPropSets==2 && rgPropSets!=NULL)

	for(iSet=0; iSet<cPropIDSets; iSet++)
	{
		COMPARE(rgPropSets[iSet].guidPropertySet, rgPropIDSets[iSet].guidPropertySet);

		for(iProp=0; iProp<rgPropIDSets[iSet].cPropertyIDs; iProp++)
		{
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwPropertyID, rgPropID[2*iSet+iProp]);
			if(rgPropSets[iSet].rgProperties[iProp].dwPropertyID == DBPROP_BYREFACCESSORS ||
				rgPropSets[iSet].rgProperties[iProp].dwPropertyID == DBPROP_OLEOBJECTS)
			{
				COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
				COMPARE(V_VT(&rgPropSets[iSet].rgProperties[iProp].vValue), VT_BOOL);
			}
			else
			{
				COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_NOTSET);
				COMPARE(V_VT(&rgPropSets[iSet].rgProperties[iProp].vValue), VT_EMPTY);
			}
		}
	}

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Set and Get some SESSION properties.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_4()
{ 
	TBEGIN

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4);
	//Following are provider specific property groups and props.
	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM);
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_ROWSET, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Set and Get some ROWSET properties.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_5()
{ 
	TBEGIN

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_IColumnsRowset, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRow, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET);
	SetProperty(DBPROP_COMMANDTIMEOUT, DBPROPSET_ROWSET, (void*)10, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Set and Get a combination of properties.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_6()
{ 
	TBEGIN
	ULONG			iSet,iProp;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBPROP*			pProp = NULL;
	ULONG			cPropIDSets = 0;
	DBPROPIDSET		rgPropIDSets[9];
	DBID			dbid1, dbid2, dbid3;
	GUID			guid = DBGUID_ROWSET;

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_AUTH_MASK_PASSWORD, DBPROPSET_DBINIT);
	SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, (void*)DB_IMP_LEVEL_IDENTIFY, DBTYPE_I4);
	SetProperty(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, (void*)10, DBTYPE_I4);

	SetProperty(DBPROP_BYREFACCESSORS, DBPROPSET_DATASOURCEINFO);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)DBPROPVAL_OO_DIRECTBIND, DBTYPE_I4);

	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4);

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetIndex, DBPROPSET_ROWSET);

	SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX);
	SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX);

	SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN);
	SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN);

	dbid1.eKind = DBKIND_NAME;
	dbid1.uName.pwszName = NULL;
	dbid2 = DBROWCOL_ROWURL;	//DBKIND_GUID_PROPID
	dbid3.eKind = DBKIND_PGUID_NAME;
	dbid3.uName.pwszName = L"ColID for Property";
	dbid3.uGuid.pguid = &guid;

	//Following are provider specific property groups and props.
	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid1);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_DSO, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid2);
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_ROWSET, (void*)DBPROMPT_COMPLETE, DBTYPE_I2, FALSE, DBPROPOPTIONS_REQUIRED, dbid3);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	//Call GetProperties with prop sets like (0,NULL,guid)

	for(iProp=0; iProp<m_cPropSets; iProp++)
	{
		rgPropIDSets[iProp].cPropertyIDs = 0;
		rgPropIDSets[iProp].rgPropertyIDs = NULL;
		rgPropIDSets[iProp].guidPropertySet = m_rgPropSets[iProp].guidPropertySet;
	}
	cPropIDSets = m_cPropSets;

	TESTC_(m_hr=GetProps(cPropIDSets, rgPropIDSets, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets == m_cPropSets)

	for(iSet=0; iSet<m_cPropSets; iSet++)
	{
		for(iProp=0; iProp<m_rgPropSets[iSet].cProperties; iProp++)
		{
			pProp = &(m_rgPropSets[iSet].rgProperties[iProp]);
			COMPARE(pProp->dwPropertyID, m_rgPropSets[iSet].rgProperties[iProp].dwPropertyID);
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
			COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[iSet].rgProperties[iProp].vValue), TRUE);

			//Compare the colids
			if(pProp->dwPropertyID == DBPROP_UPDATABILITY)
				COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid1), TRUE);
			if(pProp->dwPropertyID == DBPROP_IRowsetLocate)
				COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid2), TRUE);
			if(pProp->dwPropertyID == DBPROP_INIT_PROMPT)
				COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid3), TRUE);
		}
	}

	FreeProperties(&cPropSets, &rgPropSets);

	//Call GetProperties with (0,NULL)

	TESTC_(m_hr=GetProps(0,NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets == m_cPropSets)

	for(iSet=0; iSet<m_cPropSets; iSet++)
	{
		for(iProp=0; iProp<m_rgPropSets[iSet].cProperties; iProp++)
		{
			pProp = &(m_rgPropSets[iSet].rgProperties[iProp]);
			COMPARE(pProp->dwPropertyID, m_rgPropSets[iSet].rgProperties[iProp].dwPropertyID);
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
			COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[iSet].rgProperties[iProp].vValue), TRUE);

			//Compare the colids
			if(pProp->dwPropertyID == DBPROP_UPDATABILITY)
				COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid1), TRUE);
			if(pProp->dwPropertyID == DBPROP_IRowsetLocate)
				COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid2), TRUE);
			if(pProp->dwPropertyID == DBPROP_INIT_PROMPT)
				COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid3), TRUE);
		}
	}

CLEANUP:
	FreeProps();
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Set and overwrite all props.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_7()
{ 
	TBEGIN

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_AUTH_MASK_PASSWORD, DBPROPSET_DBINIT);
	SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, (void*)DB_IMP_LEVEL_IDENTIFY, DBTYPE_I4);
	SetProperty(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, (void*)10, DBTYPE_I4);

	SetProperty(DBPROP_BYREFACCESSORS, DBPROPSET_DATASOURCEINFO);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)DBPROPVAL_OO_DIRECTBIND, DBTYPE_I4);

	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4);

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetIndex, DBPROPSET_ROWSET);

	SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX);
	SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX);

	SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN);
	SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN);

	//Following are provider specific property groups and props.
	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_DSO);
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_ROWSET, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	FreeProps();

	SetProperty(DBPROP_AUTH_MASK_PASSWORD, DBPROPSET_DBINIT, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, (void*)DB_IMP_LEVEL_DELEGATE, DBTYPE_I4);
	SetProperty(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, (void*)20, DBTYPE_I4);

	SetProperty(DBPROP_BYREFACCESSORS, DBPROPSET_DATASOURCEINFO, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)DBPROPVAL_OO_ROWOBJECT, DBTYPE_I4);

	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, (void*)DBPROPVAL_TI_CHAOS, DBTYPE_I4);

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_IRowsetIndex, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);

	SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, (void*)VARIANT_FALSE);

	SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, (void*)VARIANT_FALSE);

	//Following are provider specific property groups and props.
	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_DSO, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_ROWSET, (void*)DBPROMPT_NOPROMPT, DBTYPE_I2);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Set props, set new props and overwrite old ones.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_8()
{ 
	TBEGIN

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, (void*)10, DBTYPE_I4);

	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, (void*)DBPROPVAL_TI_READCOMMITTED, DBTYPE_I4);

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetIndex, DBPROPSET_ROWSET);

	SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX);
	SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX);

	SetProperty(DBPROP_INIT_PROMPT, DBGUID_DSO, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	FreeProps();

	SetProperty(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, (void*)20, DBTYPE_I4);

	SetProperty(DBPROP_SESS_AUTOCOMMITISOLEVELS, DBPROPSET_SESSION, (void*)DBPROPVAL_TI_CHAOS, DBTYPE_I4);

	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_IRowsetIndex, DBPROPSET_ROWSET, (void*)VARIANT_FALSE);

	SetProperty(DBPROP_INDEX_AUTOUPDATE, DBPROPSET_INDEX, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_INDEX_PRIMARYKEY, DBPROPSET_INDEX, (void*)VARIANT_FALSE);

	SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, (void*)VARIANT_FALSE);

	//Following are provider specific property groups and props.
	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_DSO, (void*)VARIANT_FALSE);
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_ROWSET, (void*)DBPROMPT_NOPROMPT, DBTYPE_I2);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Set props from Static Array (stack)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_9()
{ 
	TBEGIN
	ULONG			cPropSets = 0;
	DBPROP			dwProp[4];
	DBPROPSET		rgPropSets[2];
	BSTR			bstrLockOwner = NULL;
	BSTR			bstrLockOwner2 = NULL;

	ULONG		ulRef1;
	CAggregate*	pUnk = NULL;
	pUnk		= new CAggregate();

	bstrLockOwner = SYSSTRING_ALLOC(L"IDBBinderProperties Test Module");
	bstrLockOwner2 = SYSSTRING_ALLOC(L"IDBBinderProperties Test Module - TWO");

	TESTC_(Reset(), S_OK)

	//Set DBPROP for PROMPT.
	memset(&dwProp[0], 0, sizeof(DBPROP)) ;
	dwProp[0].dwPropertyID = DBPROP_INIT_PROMPT;
	dwProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[0].vValue);
	V_VT(&dwProp[0].vValue) = DBTYPE_I2;
	V_I2(&dwProp[0].vValue) = DBPROMPT_NOPROMPT;

	//Set DBPROP for LOCKOWNER.
	memset(&dwProp[1], 0, sizeof(DBPROP)) ;
	dwProp[1].dwPropertyID = DBPROP_INIT_LOCKOWNER;
	dwProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[1].vValue);
	V_VT(&dwProp[1].vValue) = DBTYPE_BSTR;
	V_BSTR(&dwProp[1].vValue) = bstrLockOwner;

	//Set the DBPROPSET.
	cPropSets = 1;
	rgPropSets[0].rgProperties = dwProp;
	rgPropSets[0].cProperties = 2;
	rgPropSets[0].guidPropertySet = DBPROPSET_DBINIT;

	TESTC_(SetProps(cPropSets, rgPropSets), S_OK)

	TESTC(VerifySetProps(cPropSets, rgPropSets))

	//Change values of above 2 props and set 2 props from another prop set.

	V_I2(&dwProp[0].vValue) = DBPROMPT_COMPLETE;
	V_BSTR(&dwProp[1].vValue) = bstrLockOwner2;

	//Set prov specific prop 1
	memset(&dwProp[2], 0, sizeof(DBPROP)) ;
	dwProp[2].dwPropertyID = DBPROP_IRowset;
	dwProp[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[2].vValue);
	V_VT(&dwProp[2].vValue) = DBTYPE_DATE;
	V_DATE(&dwProp[2].vValue) = 40000.99; //past year 2000.

	//Set prov specific prop 2
	memset(&dwProp[3], 0, sizeof(DBPROP)) ;
	dwProp[3].dwPropertyID = DBPROP_IRowsetChange;
	dwProp[3].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[3].vValue);
	V_VT(&dwProp[3].vValue) = DBTYPE_IUNKNOWN;
	V_UNKNOWN(&dwProp[3].vValue) = (IUnknown*)pUnk;
	SAFE_ADDREF(pUnk);

	//Set the DBPROPSET.
	cPropSets = 2;
	rgPropSets[0].rgProperties = dwProp;
	rgPropSets[0].cProperties = 2;
	rgPropSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropSets[1].rgProperties = &(dwProp[2]);
	rgPropSets[1].cProperties = 2;
	rgPropSets[1].guidPropertySet = DBGUID_STREAM; //Provider specific set.

	ulRef1 = pUnk->GetRefCount();

	TESTC_(SetProps(cPropSets, rgPropSets), S_OK)

	COMPARE(pUnk->GetRefCount() > ulRef1, TRUE);

	TESTC(VerifySetProps(cPropSets, rgPropSets))

	TESTC_(Reset(), S_OK)

	COMPARE(pUnk->GetRefCount(), ulRef1);

CLEANUP:
	SYSSTRING_FREE(bstrLockOwner);
	SYSSTRING_FREE(bstrLockOwner2);
	FreeProps();
	SAFE_RELEASE(pUnk);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Set props with bad status and bad variant types
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_10()
{ 
	TBEGIN
	BSTR	bstrIsLong = L"IDBBinderProperties Test - DBPROP_COL_ISLONG";

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_COL_AUTOINCREMENT, DBPROPSET_COLUMN);
	SetProperty(DBPROP_COL_ISLONG, DBPROPSET_COLUMN, (void*)bstrIsLong, DBTYPE_BSTR);

	//Following is provider specific property
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_STREAM, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);

	m_rgPropSets[0].rgProperties[0].dwStatus = DBPROPSTATUS_BADCOLUMN;
	m_rgPropSets[0].rgProperties[1].dwStatus = DBPROPSTATUS_NOTSETTABLE;
	m_rgPropSets[1].rgProperties[0].dwStatus = DBPROPSTATUS_BADVALUE;

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Set props with all sorts of variants
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_11()
{ 
	TBEGIN
	BSTR bstr = L"abcdefg";
	VARIANT vVar;
	VariantInit(&vVar);
	V_VT(&vVar) = VT_I4;
	V_I4(&vVar) = 666;

	ULONG		ulRef1, ulRef2;	
	CAggregate*	pUnk = NULL;
	CDispatch*	pDisp = NULL;
	pUnk		= new CAggregate();
	pDisp		= new CDispatch();

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_ABORTPRESERVE, DBGUID_DSO, (void*)0, DBTYPE_NULL);
	SetProperty(DBPROP_ACTIVESESSIONS, DBGUID_DSO, (void*)1, DBTYPE_R4);
	SetProperty(DBPROP_ASYNCTXNCOMMIT, DBGUID_DSO, (void*)2, DBTYPE_R8);
	SetProperty(DBPROP_AUTH_CACHE_AUTHINFO, DBGUID_DSO, (void*)33, DBTYPE_CY);
	SetProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, DBGUID_DSO, (void*)36504, DBTYPE_DATE);
	SetProperty(DBPROP_AUTH_INTEGRATED, DBGUID_DSO, (void*)(IDispatch*)pDisp, DBTYPE_IDISPATCH);
	SetProperty(DBPROP_AUTH_MASK_PASSWORD, DBGUID_DSO, (void*)(LONG_PTR)DB_E_ERRORSOCCURRED, DBTYPE_ERROR);//cast modified here coz HRESULT is of type LONG
	SetProperty(DBPROP_AUTH_PASSWORD, DBGUID_DSO, (void*)&vVar, DBTYPE_VARIANT);
	SetProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBGUID_DSO, (void*)(IUnknown*)pUnk, DBTYPE_IUNKNOWN);
	SetProperty(DBPROP_IRowsetResynch, DBGUID_SESSION, (void*)6, DBTYPE_UI1);
	SetProperty(DBPROP_ISequentialStream, DBGUID_SESSION, (void*)bstr, DBTYPE_BSTR);

	ulRef1 = pUnk->GetRefCount();
	ulRef2 = pDisp->m_cRef;

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	COMPARE(pUnk->GetRefCount() > ulRef1, TRUE);
	COMPARE(pDisp->m_cRef > ulRef2, TRUE);

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC_(Reset(), S_OK)

	COMPARE(pUnk->GetRefCount(), ulRef1);
	COMPARE(pDisp->m_cRef, ulRef2);

CLEANUP:
	FreeProps();
	SAFE_RELEASE(pUnk);
	SAFE_RELEASE(pDisp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Set same property with different colids.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_12()
{ 
	TBEGIN
	ULONG			iSet,iProp;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBPROP*			pProp = NULL;
	DBID			dbid[7];
	GUID			guid = DBGUID_ROWSET;

	TESTC_(Reset(), S_OK)

	dbid[0].eKind = DBKIND_GUID_NAME;
	dbid[0].uName.pwszName = NULL;
	dbid[0].uGuid.guid = DBGUID_ROWSET;
	dbid[1] = DBROWCOL_ROWURL;	//DBKIND_GUID_PROPID
	dbid[2].eKind = DBKIND_NAME;
	dbid[2].uName.pwszName = L"ColID for Property";
	dbid[3].eKind = DBKIND_PGUID_NAME;
	dbid[3].uName.pwszName = L"ColID for Property";
	dbid[3].uGuid.pguid = &guid;
	dbid[4].eKind = DBKIND_PGUID_PROPID;
	dbid[4].uName.ulPropid = 0;
	dbid[4].uGuid.pguid = &guid;
	dbid[5].eKind = DBKIND_PROPID;
	dbid[5].uName.ulPropid = 0;
	dbid[6].eKind = DBKIND_GUID;
	dbid[6].uGuid.guid = DBGUID_ROWSET;

	//Set same property with different colids.

	for(iProp=0; iProp<2; iProp++)
		SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[iProp]);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	for(iProp=2; iProp<7; iProp++)
		SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[iProp]);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC_(m_hr=GetProps(0,NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets == m_cPropSets)

	for(iSet=0; iSet<m_cPropSets; iSet++)
		for(iProp=0; iProp<m_rgPropSets[iSet].cProperties; iProp++)
		{
			pProp = &(m_rgPropSets[iSet].rgProperties[iProp]);
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
			COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[iSet].rgProperties[iProp].vValue), TRUE);
			COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid[iProp]), TRUE);
		}

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Set diffr properties with same colids.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_13()
{ 
	TBEGIN
	ULONG			ulIndex=0;
	ULONG			iSet,iProp;
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBPROP*			pProp = NULL;
	DBID			dbid[7];
	GUID			guid = DBGUID_ROWSET;

	TESTC_(Reset(), S_OK)

	dbid[0].eKind = DBKIND_GUID_NAME;
	dbid[0].uName.pwszName = L"ColID for Property";
	dbid[0].uGuid.guid = DBGUID_ROWSET;
	dbid[1] = DBROWCOL_ROWURL;	//DBKIND_GUID_PROPID
	dbid[2].eKind = DBKIND_NAME;
	dbid[2].uName.pwszName = L"ColID for Property";
	dbid[3].eKind = DBKIND_PGUID_NAME;
	dbid[3].uName.pwszName = NULL;
	dbid[3].uGuid.pguid = &guid;
	dbid[4].eKind = DBKIND_PGUID_PROPID;
	dbid[4].uName.ulPropid = 0;
	dbid[4].uGuid.pguid = &guid;
	dbid[5].eKind = DBKIND_PROPID;
	dbid[5].uName.ulPropid = 0;
	dbid[6].eKind = DBKIND_GUID;
	dbid[6].uGuid.guid = DBGUID_ROWSET;

	//Set different properties with some same, and some different colids.

	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[0]);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[0]);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[1]);
	SetProperty(DBPROP_IRowsetUpdate, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[2]);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	SetProperty(DBPROP_UPDATABILITY, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[3]);
	SetProperty(DBPROP_IRowsetLocate, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[4]);
	SetProperty(DBPROP_IRowset, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[4]);
	SetProperty(DBPROP_IConvertType, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[5]);
	SetProperty(DBPROP_IAccessor, DBGUID_STREAM, (void*)VARIANT_TRUE, DBTYPE_BOOL, FALSE, DBPROPOPTIONS_REQUIRED, dbid[6]);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC_(m_hr=GetProps(0,NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets == m_cPropSets)

	for(iSet=0; iSet<m_cPropSets; iSet++)
		for(iProp=0; iProp<m_rgPropSets[iSet].cProperties; iProp++)
		{
			switch (iProp)
			{
			case 0:
			case 1:
				ulIndex=0;
				break;
			case 2:
				ulIndex=1;
				break;
			case 3:
				ulIndex=2;
				break;
			case 4:
				ulIndex=3;
				break;
			case 5:
			case 6:
				ulIndex=4;
				break;
			case 7:
				ulIndex=5;
				break;
			case 8:
				ulIndex=6;
				break;
			};

			pProp = &(m_rgPropSets[iSet].rgProperties[iProp]);
			COMPARE(pProp->dwPropertyID, rgPropSets[iSet].rgProperties[iProp].dwPropertyID);
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
			COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[iSet].rgProperties[iProp].vValue), TRUE);
			COMPARE(CompareDBID(rgPropSets[iSet].rgProperties[iProp].colid, dbid[ulIndex]), TRUE);
		}

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

CLEANUP:
	FreeProps();
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Verify release of props thru ref counts.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_14()
{ 
	TBEGIN
	ULONG					iSet,iProp;
	ULONG					ulRef1, ulRef2;	
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	DBPROP*					pProp = NULL;
	CAggregate*				pUnk = NULL;
	CDispatch*				pDisp = NULL;
	IDBBinderProperties*	pIProp = NULL;

	pUnk		= new CAggregate();
	pDisp		= new CDispatch();

	//Increase ref counts by 2.
	SAFE_ADDREF(pUnk);
	SAFE_ADDREF(pUnk);
	SAFE_ADDREF(pDisp);
	SAFE_ADDREF(pDisp);

	TESTC_(CoCreateInstance(CLSID_RootBinder, NULL, 
		GetModInfo()->GetClassContext(), IID_IDBBinderProperties, 
		(void**)&pIProp), S_OK);
	TESTC(pIProp!=NULL)

	TESTC_(pIProp->Reset(), S_OK)

	SetProperty(DBPROP_AUTH_INTEGRATED, DBGUID_DSO, (void*)(IDispatch*)pDisp, DBTYPE_IDISPATCH);
	SetProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBGUID_DSO, (void*)(IUnknown*)pUnk, DBTYPE_IUNKNOWN);

	ulRef1 = pUnk->GetRefCount();
	ulRef2 = pDisp->m_cRef;

	TESTC_(pIProp->SetProperties(m_cPropSets, m_rgPropSets), S_OK)

	COMPARE(pUnk->GetRefCount() > ulRef1, TRUE);
	COMPARE(pDisp->m_cRef > ulRef2, TRUE);

	TESTC_(pIProp->Reset(), S_OK)

	COMPARE(pUnk->GetRefCount(), ulRef1);
	COMPARE(pDisp->m_cRef, ulRef2);

	TESTC_(pIProp->SetProperties(m_cPropSets, m_rgPropSets), S_OK)
	TESTC_(pIProp->SetProperties(m_cPropSets, m_rgPropSets), S_OK)
	TESTC_(pIProp->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets==m_cPropSets)
	for(iSet=0; iSet<m_cPropSets; iSet++)
		for(iProp=0; iProp<m_rgPropSets[iSet].cProperties; iProp++)
		{
			pProp = &(m_rgPropSets[iSet].rgProperties[iProp]);
			COMPARE(pProp->dwPropertyID, rgPropSets[iSet].rgProperties[iProp].dwPropertyID);
			COMPARE(rgPropSets[iSet].rgProperties[iProp].dwStatus, DBPROPSTATUS_OK);
			COMPARE(CompareVariant(&pProp->vValue, &rgPropSets[iSet].rgProperties[iProp].vValue), TRUE);
		}
	FreeProperties(&cPropSets, &rgPropSets);

	COMPARE(pUnk->GetRefCount() > ulRef1, TRUE);
	COMPARE(pDisp->m_cRef > ulRef2, TRUE);

	SAFE_RELEASE(pIProp);

	COMPARE(pUnk->GetRefCount(), ulRef1);
	COMPARE(pDisp->m_cRef, ulRef2);

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProps();
	//Deccrease ref counts by 2.
	SAFE_RELEASE(pUnk);
	SAFE_RELEASE(pUnk);
	SAFE_RELEASE(pDisp);
	SAFE_RELEASE(pDisp);
	//Release objs.
	SAFE_RELEASE(pUnk);
	SAFE_RELEASE(pDisp);
	SAFE_RELEASE(pIProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Duplicate prop in a set.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_15()
{ 
	TBEGIN
	ULONG_PTR	ulVal=0;

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)1, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)2, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)3, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)4, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBBinderProperties, &ulVal))
	COMPARE(ulVal, 4);

	FreeProps();
	TESTC_(Reset(), S_OK)

	//Set it in 2 stages.

	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)1, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)2, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)3, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	FreeProps();
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)4, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)5, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBBinderProperties, &ulVal))
	COMPARE(ulVal, 5);

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Duplicate prop in duplicate sets.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_16()
{ 
	TBEGIN
	ULONG_PTR	ulVal=0;

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)1, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, (void*)2, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DBINIT, (void*)3, DBTYPE_I4);
	SetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DBINIT, (void*)4, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO,
		m_pIDBBinderProperties, &ulVal))
	COMPARE(ulVal, 2);

	TESTC(GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DBINIT,
		m_pIDBBinderProperties, &ulVal))
	COMPARE(ulVal, 4);

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Duplicate props (multiple) in duplicate sets.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_17()
{ 
	TBEGIN
	ULONG			cPropSets = 0;
	DBPROP			dwProp[5];
	DBPROPSET		rgPropSets[2];
	ULONG_PTR		ulVal=0;

	TESTC_(Reset(), S_OK)

	//Set DBPROP for 1st prop, 1st set.
	memset(&dwProp[0], 0, sizeof(DBPROP)) ;
	dwProp[0].dwPropertyID = DBPROP_INIT_PROMPT;
	dwProp[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[0].vValue);
	V_VT(&dwProp[0].vValue) = DBTYPE_I2;
	V_I2(&dwProp[0].vValue) = 1;

	//Set DBPROP for 2nd prop, 1st set.
	memset(&dwProp[1], 0, sizeof(DBPROP)) ;
	dwProp[1].dwPropertyID = DBPROP_INIT_PROMPT;
	dwProp[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[1].vValue);
	V_VT(&dwProp[1].vValue) = DBTYPE_I2;
	V_I2(&dwProp[1].vValue) = 2;

	//Set DBPROP for 1st prop, 2nd set.
	memset(&dwProp[2], 0, sizeof(DBPROP)) ;
	dwProp[2].dwPropertyID = DBPROP_INIT_PROMPT;
	dwProp[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[2].vValue);
	V_VT(&dwProp[2].vValue) = DBTYPE_I2;
	V_I2(&dwProp[2].vValue) = 3;

	//Set DBPROP for 2nd prop, 2nd set.
	memset(&dwProp[3], 0, sizeof(DBPROP)) ;
	dwProp[3].dwPropertyID = DBPROP_INIT_LOCKOWNER;
	dwProp[3].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[3].vValue);
	V_VT(&dwProp[3].vValue) = DBTYPE_I4;
	V_I4(&dwProp[3].vValue) = 8;

	//Set DBPROP for 3rd prop, 2nd set.
	memset(&dwProp[4], 0, sizeof(DBPROP)) ;
	dwProp[4].dwPropertyID = DBPROP_INIT_PROMPT;
	dwProp[4].dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dwProp[4].vValue);
	V_VT(&dwProp[4].vValue) = DBTYPE_I2;
	V_I2(&dwProp[4].vValue) = 4;

	//Set DBPROPSET structs.
	cPropSets = 2;
	rgPropSets[0].rgProperties = dwProp;
	rgPropSets[0].cProperties = 2;
	rgPropSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropSets[1].rgProperties = &(dwProp[2]);
	rgPropSets[1].cProperties = 3;
	rgPropSets[1].guidPropertySet = DBPROPSET_DBINIT; 

	TESTC_(SetProps(cPropSets, rgPropSets), S_OK)

	TESTC(GetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT,
		m_pIDBBinderProperties, &ulVal))
	COMPARE(ulVal, 4);

	TESTC(GetProperty(DBPROP_INIT_LOCKOWNER, DBPROPSET_DBINIT,
		m_pIDBBinderProperties, &ulVal))
	COMPARE(ulVal, 8);

CLEANUP:
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc GetProperties: E_INVALIDARG cases
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_18()
{ 
	TBEGIN
	ULONG			cPropSets = 0;
	DBPROPSET*		rgPropSets = NULL;
	DBPROPID		dbPropID=DBPROP_IRow;
	DBPROPIDSET		rgPropIDSet[1];
	BSTR			bstrLockOwner = L"IDBBinderProperties Test Module";

	TESTC_(Reset(), S_OK)

	SetProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);
	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, (void*)DBBINDURLFLAG_SHARE_DENY_WRITE, DBTYPE_I4);
	SetProperty(DBPROP_INIT_LOCKOWNER, DBPROPSET_DBINIT, (void*)bstrLockOwner, DBTYPE_BSTR);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	rgPropIDSet[0].cPropertyIDs = 0;
	rgPropIDSet[0].rgPropertyIDs = NULL;
	rgPropIDSet[0].guidPropertySet = DBPROPSET_DBINIT;

	TESTC_(m_hr=GetProps(1,NULL, &cPropSets, &rgPropSets), E_INVALIDARG)
	TESTC(!cPropSets && !rgPropSets)

	TESTC_(m_hr=GetProps(1,rgPropIDSet, NULL, &rgPropSets), E_INVALIDARG)
	TESTC(!cPropSets && !rgPropSets)

	TESTC_(m_hr=GetProps(1,rgPropIDSet, &cPropSets, NULL), E_INVALIDARG)
	TESTC(!cPropSets && !rgPropSets)

	TESTC(XCHECK(m_pIDBBinderProperties, IID_IDBBinderProperties, m_hr));

	rgPropIDSet[0].cPropertyIDs = 1;
	rgPropIDSet[0].rgPropertyIDs = NULL;
	rgPropIDSet[0].guidPropertySet = DBPROPSET_DBINIT;

	CHECK(m_hr=GetProps(1,rgPropIDSet, &cPropSets, &rgPropSets), E_INVALIDARG);
	TESTC(!cPropSets && !rgPropSets)

	rgPropIDSet[0].cPropertyIDs = 1;
	rgPropIDSet[0].rgPropertyIDs = NULL;
	rgPropIDSet[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

    CHECK(m_hr=GetProps(1,rgPropIDSet, &cPropSets, &rgPropSets),S_OK );
	TESTC(!cPropSets && !rgPropSets)

	rgPropIDSet[0].cPropertyIDs = 0;
	rgPropIDSet[0].rgPropertyIDs = &dbPropID;
	rgPropIDSet[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	CHECK(m_hr=GetProps(1,rgPropIDSet, &cPropSets, &rgPropSets), S_OK);
	TESTC(!cPropSets && !rgPropSets)

	rgPropIDSet[0].cPropertyIDs = 1;
	rgPropIDSet[0].rgPropertyIDs = &dbPropID;
	rgPropIDSet[0].guidPropertySet = DBPROPSET_PROPERTIESINERROR;

	CHECK(m_hr=GetProps(1,rgPropIDSet, &cPropSets, &rgPropSets), S_OK);
	TESTC(!cPropSets && !rgPropSets)

CLEANUP:
	FreeProps();
	FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc SetProperties: E_INVALIDARG cases
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetAndSetProps::Variation_19()
{ 
	TBEGIN
	ULONG			cPropSets = 0;
	DBPROPSET		rgPropSets[1];
	DBPROPIDSET		rgPropIDSets[1];
	ULONG			cPropSetsGot = 0;
	DBPROPSET*		rgPropSetsGot = NULL;
	DBPROP			dbProp;

	TESTC_(Reset(), S_OK)

	//Set the DBPROPSET. cProperties = 0. So, the property
	//should not get set.
	cPropSets = 1;
	dbProp.dwPropertyID = DBPROP_IRow;
	dbProp.dwOptions = DBPROPOPTIONS_REQUIRED;
	VariantInit(&dbProp.vValue);
	dbProp.colid = DB_NULLID;
	rgPropSets[0].rgProperties = &dbProp;
	rgPropSets[0].cProperties = 1;
	rgPropSets[0].guidPropertySet = DBPROPSET_DBINIT;

	//If cPropSets is 0, then rgPropSets should be ignored.
	//Hence this should be a no-op.
	CHECK(m_hr = SetProps(0, rgPropSets), S_OK);

	//Make the propID set for calling GetProps.
	cPropSets = 1;
	rgPropIDSets[0].rgPropertyIDs = &(dbProp.dwPropertyID);
	rgPropIDSets[0].cPropertyIDs = 1;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_DBINIT;

	//Make sure the prop wasn't set.
	CHECK(m_hr = GetProps(cPropSets, rgPropIDSets, &cPropSetsGot, &rgPropSetsGot), DB_E_ERRORSOCCURRED);
	COMPARE(cPropSetsGot==1 && rgPropSetsGot!=NULL, TRUE);
	COMPARE(rgPropSetsGot[0].rgProperties != NULL, TRUE);
	COMPARE(rgPropSetsGot[0].rgProperties[0].dwPropertyID, rgPropIDSets[0].rgPropertyIDs[0]);
	COMPARE(rgPropSetsGot[0].rgProperties[0].dwStatus, DBPROPSTATUS_NOTSET);

	CHECK(m_hr = SetProps(1, NULL), E_INVALIDARG);

	TESTC(XCHECK(m_pIDBBinderProperties, IID_IDBBinderProperties, m_hr));

	//An element of rgPropSets has cProperties=1, but 
	//rgProperties=NULL. Should get E_INVALIDARG.
	rgPropSets[0].cProperties = 1;
	rgPropSets[0].rgProperties = NULL;
	CHECK(m_hr = SetProps(1, rgPropSets), E_INVALIDARG);

CLEANUP:
	FreeProperties(&cPropSetsGot, &rgPropSetsGot);
	FreeProps();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCGetAndSetProps::Terminate()
{ 
	ReleaseAll();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CBinderProp::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCGetPropertyInfo)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetPropertyInfo - Test the GetPropertyInfo method.
//| Created:  	9/2/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetPropertyInfo::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CBinderProp::Init())
	// }}
	{ 
		return GetRootBinder();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetPropInfo (0,NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_1()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	TESTC_(m_hr=GetPropInfos(0,NULL, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), S_OK)

	TESTC(!cPropInfoSets && !rgPropInfoSets && !pwszDescBuffer)

	TESTC_(m_hr=GetPropInfos(0,NULL, &cPropInfoSets, &rgPropInfoSets, NULL), S_OK)

	TESTC(!cPropInfoSets && !rgPropInfoSets)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc GetPropInfo(DBPROPSET_*ALL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_2()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropIDSets = 11;
	DBPROPIDSET		rgPropIDSets[11];
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_SESSIONALL;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[1].cPropertyIDs = 0;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSETALL;
	rgPropIDSets[1].rgPropertyIDs = NULL;
	rgPropIDSets[2].cPropertyIDs = 0;
	rgPropIDSets[2].guidPropertySet = DBPROPSET_DATASOURCEINFOALL;
	rgPropIDSets[2].rgPropertyIDs = NULL;
	rgPropIDSets[3].cPropertyIDs = 0;
	rgPropIDSets[3].guidPropertySet = DBPROPSET_DBINITALL;
	rgPropIDSets[3].rgPropertyIDs = NULL;
	rgPropIDSets[4].cPropertyIDs = 0;
	rgPropIDSets[4].guidPropertySet = DBPROPSET_COLUMNALL;
	rgPropIDSets[4].rgPropertyIDs = NULL;
	rgPropIDSets[5].cPropertyIDs = 0;
	rgPropIDSets[5].guidPropertySet = DBPROPSET_TABLEALL;
	rgPropIDSets[5].rgPropertyIDs = NULL;
	rgPropIDSets[6].cPropertyIDs = 0;
	rgPropIDSets[6].guidPropertySet = DBPROPSET_DATASOURCEALL;
	rgPropIDSets[6].rgPropertyIDs = NULL;
	rgPropIDSets[7].cPropertyIDs = 0;
	rgPropIDSets[7].guidPropertySet = DBPROPSET_INDEXALL;
	rgPropIDSets[7].rgPropertyIDs = NULL;
	rgPropIDSets[8].cPropertyIDs = 0;
	rgPropIDSets[8].guidPropertySet = DBPROPSET_TRUSTEEALL;
	rgPropIDSets[8].rgPropertyIDs = NULL;
	rgPropIDSets[9].cPropertyIDs = 0;
	rgPropIDSets[9].guidPropertySet = DBPROPSET_CONSTRAINTALL;
	rgPropIDSets[9].rgPropertyIDs = NULL;
	rgPropIDSets[10].cPropertyIDs = 0;
	rgPropIDSets[10].guidPropertySet = DBPROPSET_VIEWALL;
	rgPropIDSets[10].rgPropertyIDs = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), DB_E_ERRORSOCCURRED)

	TESTC(VerifyPropInfos(cPropIDSets, rgPropIDSets, cPropInfoSets, rgPropInfoSets))

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, NULL), DB_E_ERRORSOCCURRED)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Get DBINIT, DATASOURCEINFO, SESSION and ROWSET PropInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_3()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropIDSets = 4;
	DBPROPIDSET		rgPropIDSets[4];
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_SESSION;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[1].cPropertyIDs = 0;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSET;
	rgPropIDSets[1].rgPropertyIDs = NULL;
	rgPropIDSets[2].cPropertyIDs = 0;
	rgPropIDSets[2].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	rgPropIDSets[2].rgPropertyIDs = NULL;
	rgPropIDSets[3].cPropertyIDs = 0;
	rgPropIDSets[3].guidPropertySet = DBPROPSET_DBINIT;
	rgPropIDSets[3].rgPropertyIDs = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), DB_E_ERRORSOCCURRED)

	TESTC(VerifyPropInfos(cPropIDSets, rgPropIDSets, cPropInfoSets, rgPropInfoSets))

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, NULL), DB_E_ERRORSOCCURRED)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Get a combination of PropInfos
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_4()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			ulIndex;
	ULONG			cPropIDSets = 2;
	DBPROPIDSET		rgPropIDSets[2];
	DBPROPID		rgPropIDs[4];
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	rgPropIDs[0] = DBPROP_INIT_PROMPT;
	rgPropIDs[1] = DBPROP_INIT_BINDFLAGS;
	rgPropIDs[2] = DBPROP_CANHOLDROWS;
	rgPropIDs[3] = DBPROP_IRowsetLocate;
	
	rgPropIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSET;
	for(ulIndex=0; ulIndex<2;ulIndex++)
	{
		rgPropIDSets[ulIndex].cPropertyIDs = 2;
		rgPropIDSets[ulIndex].rgPropertyIDs = &(rgPropIDs[2*ulIndex]);
	}

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), DB_E_ERRORSOCCURRED)

	TESTC(VerifyPropInfos(cPropIDSets, rgPropIDSets, cPropInfoSets, rgPropInfoSets))

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, NULL), DB_E_ERRORSOCCURRED)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cPropIDSets=1 but rgPropIDSets=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_5()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropIDSets = 2;
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, NULL, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	TESTC_(m_hr=GetPropInfos(cPropIDSets, NULL, &cPropInfoSets, &rgPropInfoSets, NULL), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	TESTC(XCHECK(m_pIDBBinderProperties, IID_IDBBinderProperties, m_hr));

	tResult = TEST_PASS;

CLEANUP:
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: cPropID=1 but rgPropIDs=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_6()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropIDSets = 2;
	DBPROPIDSET		rgPropIDSets[2];
	DBPROPID		rgPropIDs[4];
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	rgPropIDs[0] = DBPROP_INIT_PROMPT;
	rgPropIDs[1] = DBPROP_INIT_BINDFLAGS;
	
	rgPropIDSets[0].guidPropertySet = DBPROPSET_DBINIT;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSET;

	rgPropIDSets[0].cPropertyIDs = 2;
	rgPropIDSets[0].rgPropertyIDs = &(rgPropIDs[0]);

	rgPropIDSets[1].cPropertyIDs = 1;
	rgPropIDSets[1].rgPropertyIDs = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, NULL), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	rgPropIDSets[1].cPropertyIDs = 0;
	rgPropIDSets[1].rgPropertyIDs = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), DB_E_ERRORSOCCURRED)

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, NULL), DB_E_ERRORSOCCURRED)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: pcPropInfoSets=NULL or rgPropInfoSets=NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_7()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropIDSets = 2;
	DBPROPIDSET		rgPropIDSets[2];
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_SESSION;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[1].cPropertyIDs = 0;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSET;
	rgPropIDSets[1].rgPropertyIDs = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, NULL, &rgPropInfoSets, &pwszDescBuffer), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, NULL, &pwszDescBuffer), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, NULL, &rgPropInfoSets, NULL), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc E_INVALIDARG: A special and a normal property set
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetPropertyInfo::Variation_8()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	ULONG			cPropIDSets = 4;
	DBPROPIDSET		rgPropIDSets[4];
	ULONG			cPropInfoSets = 0;
	DBPROPINFOSET*	rgPropInfoSets = NULL;
	WCHAR*			pwszDescBuffer = NULL;

	rgPropIDSets[0].cPropertyIDs = 0;
	rgPropIDSets[0].guidPropertySet = DBPROPSET_SESSION;
	rgPropIDSets[0].rgPropertyIDs = NULL;
	rgPropIDSets[1].cPropertyIDs = 0;
	rgPropIDSets[1].guidPropertySet = DBPROPSET_ROWSETALL;
	rgPropIDSets[1].rgPropertyIDs = NULL;
	rgPropIDSets[2].cPropertyIDs = 0;
	rgPropIDSets[2].guidPropertySet = DBPROPSET_DATASOURCEINFO;
	rgPropIDSets[2].rgPropertyIDs = NULL;
	rgPropIDSets[3].cPropertyIDs = 0;
	rgPropIDSets[3].guidPropertySet = DBPROPSET_DBINITALL;
	rgPropIDSets[3].rgPropertyIDs = NULL;

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	TESTC_(m_hr=GetPropInfos(cPropIDSets, rgPropIDSets, &cPropInfoSets, &rgPropInfoSets, NULL), E_INVALIDARG)
	TESTC(!cPropInfoSets && !rgPropInfoSets)

	tResult = TEST_PASS;

CLEANUP:
	FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszDescBuffer);
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCGetPropertyInfo::Terminate()
{ 
	ReleaseAll(); 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CBinderProp::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCReset)
//*-----------------------------------------------------------------------
//| Test Case:		TCReset - Test the Reset method.
//| Created:  	9/2/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCReset::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CBinderProp::Init())
	// }}
	{ 
		return GetRootBinder();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - SetProps(0, NULL)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_1()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	TESTC_(SetProps(0, NULL), S_OK)

	TESTC(VerifyReset(TRUE))

	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Call SetProps in stages.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_2()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	BSTR			bstrLockOwner = L"IDBBinderProperties Test Module";

	SetProperty(DBPROP_AUTH_CACHE_AUTHINFO, DBPROPSET_DBINIT);
	SetProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, DBPROPSET_DBINIT);
	SetProperty(DBPROP_REMOVEDELETED, DBPROPSET_ROWSET);
	SetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	//Set props on both propSets - m_rgPropSets & m_rgPropSets2.
	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, (void*)DBBINDURLFLAG_SHARE_DENY_WRITE, DBTYPE_I4, TRUE);
	SetProperty(DBPROP_INIT_LOCKOWNER, DBPROPSET_DBINIT, (void*)bstrLockOwner, DBTYPE_BSTR, TRUE);
	SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, TRUE);

	TESTC_(SetProps(m_cPropSets2, m_rgPropSets2), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset(TRUE))

	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - Call SetProp on same props multiple times.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_3()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	BSTR			bstrLockOwner = L"IDBBinderProperties Test Module";

	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);
	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, (void*)DBBINDURLFLAG_SHARE_DENY_WRITE, DBTYPE_I4);
	SetProperty(DBPROP_INIT_LOCKOWNER, DBPROPSET_DBINIT, (void*)bstrLockOwner, DBTYPE_BSTR);
	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset(TRUE))

	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DBINIT - Set CacheAuthInfo, AuthEncryptPassword, Asynch, Mode.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_4()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_AUTH_CACHE_AUTHINFO, DBPROPSET_DBINIT);
	SetProperty(DBPROP_AUTH_ENCRYPT_PASSWORD, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, (void*)0, DBTYPE_I4);
	SetProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, (void*)DB_MODE_READ, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBINIT - Set MaskPassword, PersistSenAuthInfo, ImpersLevel, Timeout.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_5()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_AUTH_MASK_PASSWORD, DBPROPSET_DBINIT);
	SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, (void*)DB_IMP_LEVEL_IDENTIFY, DBTYPE_I4);
	SetProperty(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, (void*)10, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc DBINIT - Set PersistEncrypted, Prompt, BindFlags, LockOwner.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_6()
{ 
	TESTRESULT		tResult = TEST_FAIL;
	BSTR			bstrLockOwner = L"IDBBinderProperties Test Module";

	SetProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBPROPSET_DBINIT);
	SetProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, (void*)DBPROMPT_COMPLETE, DBTYPE_I2);
	SetProperty(DBPROP_INIT_BINDFLAGS, DBPROPSET_DBINIT, (void*)DBBINDURLFLAG_SHARE_DENY_WRITE, DBTYPE_I4);
	SetProperty(DBPROP_INIT_LOCKOWNER, DBPROPSET_DBINIT, (void*)bstrLockOwner, DBTYPE_BSTR);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set AbortPreserve, AccessOrder, AppendOnly, BlockingStorObj.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_7()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_ABORTPRESERVE, DBPROPSET_ROWSET);
	SetProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_SEQUENTIAL, DBTYPE_I4);
	SetProperty(DBPROP_APPENDONLY, DBPROPSET_ROWSET);
	SetProperty(DBPROP_BLOCKINGSTORAGEOBJECTS, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set Bookmarks, BkmSkip, BkmType, CacheDefer.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_8()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_BOOKMARKSKIPPED, DBPROPSET_ROWSET);
	SetProperty(DBPROP_BOOKMARKTYPE, DBPROPSET_ROWSET, (void*)DBPROPVAL_BMK_NUMERIC, DBTYPE_I4);
	SetProperty(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set CanFetchBack, CanHoldRows, CanScrollBack,  ChangeInsertedRows.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_9()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CANSCROLLBACKWARDS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CHANGEINSERTEDROWS, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set ClientCursor, CmdTimeout, CommitPreserve, Deferred.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_10()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET);
	SetProperty(DBPROP_COMMANDTIMEOUT, DBPROPSET_ROWSET, (void*)10, DBTYPE_I4);
	SetProperty(DBPROP_COMMITPRESERVE, DBPROPSET_ROWSET);
	SetProperty(DBPROP_DEFERRED, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set DelayStorObj, HiddenCols, ImmobileRows, LiteralBkms.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_11()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_DELAYSTORAGEOBJECTS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_HIDDENCOLUMNS, DBPROPSET_ROWSET, (void*)0, DBTYPE_I4);
	SetProperty(DBPROP_IMMOBILEROWS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_LITERALBOOKMARKS, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set MaxRows, MayWriteCol, MemUsage, OrderedBkms.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_12()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_MAXROWS, DBPROPSET_ROWSET, (void*)10, DBTYPE_I4);
	SetProperty(DBPROP_MAYWRITECOLUMN, DBPROPSET_ROWSET);
	SetProperty(DBPROP_MEMORYUSAGE, DBPROPSET_ROWSET, (void*)60, DBTYPE_I4);
	SetProperty(DBPROP_ORDEREDBOOKMARKS, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set OtherInsert, OtherUpdateDelete, OwnInsert, OwnUpdateDelete.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_13()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_OTHERINSERT, DBPROPSET_ROWSET);
	SetProperty(DBPROP_OTHERUPDATEDELETE, DBPROPSET_ROWSET);
	SetProperty(DBPROP_OWNINSERT, DBPROPSET_ROWSET);
	SetProperty(DBPROP_OWNUPDATEDELETE, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set QuickRestart, RemoveDeleted,  RowsetAsynch, RowThreadModel.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_14()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET);
	SetProperty(DBPROP_REMOVEDELETED, DBPROPSET_ROWSET);
	SetProperty(DBPROP_ROWSET_ASYNCH, DBPROPSET_ROWSET, (void*)DBPROPVAL_ASYNCH_INITIALIZE, DBTYPE_I4);
	SetProperty(DBPROP_ROWTHREADMODEL, DBPROPSET_ROWSET, (void*)DBPROPVAL_RT_FREETHREAD, DBTYPE_I4);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set ServerCursor, ServerDataOnInsert, TransactedObj, UniqueRows.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_15()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_SERVERCURSOR, DBPROPSET_ROWSET);
	SetProperty(DBPROP_SERVERDATAONINSERT, DBPROPSET_ROWSET);
	SetProperty(DBPROP_TRANSACTEDOBJECT, DBPROPSET_ROWSET);
	SetProperty(DBPROP_UNIQUEROWS, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set Updatability, IRowsetChange, IColumnsRowset, IRow.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_16()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_UPDATABILITY, DBPROPSET_ROWSET, (void*)7, DBTYPE_I4);
	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IColumnsRowset, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRow, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set IRowsetChange, IRowsetFind, IRowsetIdentity, IRowsetIndex.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_17()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetFind, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetIndex, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set IRowsetLocate, IRowsetRefresh, IRowsetResynch, IRowsetScroll.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_18()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_IRowsetLocate, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetRefresh, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetResynch, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetScroll, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set IRowsetUpdate, IRowsetView, IViewChapter, IViewFilter, IViewRowset.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_19()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetView, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IViewChapter, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IViewFilter, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set IViewSort, ISupportErrorInfo, ILockBytes, ISequentialStream.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_20()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_IViewSort, DBPROPSET_ROWSET);
	SetProperty(DBPROP_ISupportErrorInfo, DBPROPSET_ROWSET);
	SetProperty(DBPROP_ILockBytes, DBPROPSET_ROWSET);
	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc ROWSET - Set IStorage, IStream, CanHoldRows, IRowsetIdentity.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCReset::Variation_21()
{ 
	TESTRESULT		tResult = TEST_FAIL;

	SetProperty(DBPROP_IStorage, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IStream, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_IRowsetIdentity, DBPROPSET_ROWSET);

	TESTC_(SetProps(m_cPropSets, m_rgPropSets), S_OK)

	TESTC(VerifySetProps(m_cPropSets, m_rgPropSets))

	TESTC(VerifyReset())
	
	tResult = TEST_PASS;

CLEANUP:
	FreeProps();
	Reset();
	return tResult;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCReset::Terminate()
{ 
	ReleaseAll();

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CBinderProp::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCSpecialCases)
//*-----------------------------------------------------------------------
//| Test Case:		TCSpecialCases - Special scenarios
//| Created:  	6/11/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCSpecialCases::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CBinderProp::Init())
	// }}
	{ 
		return GetRootBinder();
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Set a valid property and set it again with unrecognised VT type.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_1()
{ 
	TBEGIN
	ULONG					ulRef1=0, ulRef2=0;	
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	DBPROP*					pProp = NULL;
	DBID					colid = DBROWCOL_ROWURL;
	CDispatch*				pDisp = NULL;
	IDBBinderProperties*	pIProp = NULL;

	pDisp		= new CDispatch();

	//Increase ref count by 2.
	SAFE_ADDREF(pDisp);
	SAFE_ADDREF(pDisp);

	//Create a fresh root binder.
	TESTC_(CoCreateInstance(CLSID_RootBinder, NULL, 
		GetModInfo()->GetClassContext(), IID_IDBBinderProperties, 
		(void**)&pIProp), S_OK);
	TESTC(pIProp!=NULL)

	//Construct a valid property.
	SetProperty(DBPROP_AUTH_INTEGRATED, DBGUID_DSO, (void*)(IDispatch*)pDisp, DBTYPE_IDISPATCH, FALSE, DBPROPOPTIONS_REQUIRED, colid);
	//Obtain its initial ref count.
	ulRef1 = pDisp->m_cRef;

	//Set the valid property on root binder.
	TESTC_(pIProp->SetProperties(m_cPropSets, m_rgPropSets), S_OK)

	//Make sure the ref count got increased. Store this value.
	COMPARE(pDisp->m_cRef > ulRef1, TRUE);
	ulRef2 = pDisp->m_cRef;

	//Change the variant type to one not supported by VariantCopy.
	V_VT(&m_rgPropSets[0].rgProperties[0].vValue) = VT_HRESULT;
	m_rgPropSets[0].rgProperties[0].colid = DB_NULLID;

	//Try to set same property, but with invalid VT type.
	//Should fail.
	TESTC_(m_hr=pIProp->SetProperties(m_cPropSets, m_rgPropSets), DB_E_ERRORSOCCURRED);
	COMPARE(m_rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_BADVALUE);

	//Restore to original (valid) VT type.
	V_VT(&m_rgPropSets[0].rgProperties[0].vValue) = VT_DISPATCH;

	//Verify ref count is still same.
	COMPARE(pDisp->m_cRef, ulRef2);

	//Get the property back (the first, valid one)
	TESTC_(m_hr=pIProp->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets && rgPropSets && rgPropSets[0].rgProperties)

	//Make sure the original valid property was not disturbed by 
	//trying to set an invalid property.
	COMPARE(CompareVariant(&rgPropSets[0].rgProperties[0].vValue, &m_rgPropSets[0].rgProperties[0].vValue), TRUE);
	COMPARE(CompareDBID(rgPropSets[0].rgProperties[0].colid, colid), TRUE);
	FreeProperties(&cPropSets, &rgPropSets);

	TESTC_(pIProp->Reset(), S_OK)
	SAFE_RELEASE(pIProp);

	//Verify ref count after the root binder is released.
	COMPARE(pDisp->m_cRef, ulRef1);

CLEANUP:
	//Restore to original (valid) VT type.
	if(m_rgPropSets && m_rgPropSets[0].rgProperties &&
		(V_VT(&m_rgPropSets[0].rgProperties[0].vValue) == VT_HRESULT))
		V_VT(&m_rgPropSets[0].rgProperties[0].vValue) = VT_DISPATCH;
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProps();
	//Deccrease ref count by 2.
	SAFE_RELEASE(pDisp);
	SAFE_RELEASE(pDisp);
	//Release objs.
	SAFE_RELEASE(pDisp);
	SAFE_RELEASE(pIProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Set a property, then set it again with another (valid) value.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_2()
{ 
	TBEGIN
	ULONG					ulRef1=0, ulRef2=0;	
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	DBPROP*					pProp = NULL;
	CDispatch*				pDisp = NULL;
	CDispatch*				pDisp2 = NULL;
	IDBBinderProperties*	pIProp = NULL;

	pDisp		= new CDispatch();
	pDisp2		= new CDispatch();

	//Increase ref count of pDisp by 2.
	SAFE_ADDREF(pDisp);
	SAFE_ADDREF(pDisp);

	//Create a fresh root binder.
	TESTC_(CoCreateInstance(CLSID_RootBinder, NULL, 
		GetModInfo()->GetClassContext(), IID_IDBBinderProperties, 
		(void**)&pIProp), S_OK);
	TESTC(pIProp!=NULL)

	//Construct a valid property.
	SetProperty(DBPROP_AUTH_INTEGRATED, DBGUID_DSO, (void*)(IDispatch*)pDisp, DBTYPE_IDISPATCH);
	//Obtain its initial ref count.
	ulRef1 = pDisp->m_cRef;

	//Set the valid property on root binder.
	TESTC_(pIProp->SetProperties(m_cPropSets, m_rgPropSets), S_OK)

	//Make sure the ref count got increased.
	COMPARE(pDisp->m_cRef > ulRef1, TRUE);

	FreeProps();
	SetProperty(DBPROP_AUTH_INTEGRATED, DBGUID_DSO, (void*)(IDispatch*)pDisp2, DBTYPE_IDISPATCH);
	ulRef2 = pDisp2->m_cRef;

	//Try to set same property, but with another value.
	TESTC_(m_hr=pIProp->SetProperties(m_cPropSets, m_rgPropSets), S_OK);

	//Make sure the ref count got increased.
	COMPARE(pDisp2->m_cRef > ulRef2, TRUE);

	//Get the property back and verify value.
	TESTC_(m_hr=pIProp->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets && rgPropSets && rgPropSets[0].rgProperties)
	COMPARE(CompareVariant(&rgPropSets[0].rgProperties[0].vValue, &m_rgPropSets[0].rgProperties[0].vValue), TRUE);
	FreeProperties(&cPropSets, &rgPropSets);

	TESTC_(pIProp->Reset(), S_OK)
	SAFE_RELEASE(pIProp);

	//Verify ref count after the root binder is released.
	COMPARE(pDisp2->m_cRef, ulRef2);
	//The -1 is becuse we have freed this property already in the
	//FreeProps call just before the 2nd SetProperty call.
	COMPARE(pDisp->m_cRef, ulRef1-1);

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProps();
	//Deccrease ref count by 2.
	SAFE_RELEASE(pDisp);
	SAFE_RELEASE(pDisp);
	//Release objs.
	SAFE_RELEASE(pDisp);
	SAFE_RELEASE(pDisp2);
	SAFE_RELEASE(pIProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Set 2 props - one valid and one invalid.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCSpecialCases::Variation_3()
{ 
	TBEGIN
	ULONG					cPropSets = 0;
	DBPROPSET*				rgPropSets = NULL;
	DBPROP*					pProp = NULL;
	DBID					dbid1, dbid2;
	IDBBinderProperties*	pIProp = NULL;

	dbid1.eKind = DBKIND_NAME;
	dbid1.uName.pwszName = L"TCSpecialCases::Variation_3 dbid 1";
	dbid2.eKind = DBKIND_NAME;
	dbid2.uName.pwszName = L"TCSpecialCases::Variation_3 dbid 2";

	//Create a fresh root binder.
	TESTC_(CoCreateInstance(CLSID_RootBinder, NULL, 
		GetModInfo()->GetClassContext(), IID_IDBBinderProperties, 
		(void**)&pIProp), S_OK);
	TESTC(pIProp!=NULL)

	//Construct a prp set with one valid and one invalid prop.
	SetProperty(DBPROP_AUTH_INTEGRATED, DBGUID_DSO, (void*)4, DBTYPE_I4, FALSE, DBPROPOPTIONS_REQUIRED, dbid1);
	SetProperty(DBPROP_INIT_PROMPT, DBGUID_DSO, (void*)5, DBTYPE_I4, FALSE, DBPROPOPTIONS_REQUIRED, dbid2);

	//Change the variant type to one not supported by VariantCopy.
	V_VT(&m_rgPropSets[0].rgProperties[0].vValue) = VT_HRESULT;

	TESTC_(m_hr=pIProp->SetProperties(m_cPropSets, m_rgPropSets), DB_S_ERRORSOCCURRED);
	COMPARE(m_rgPropSets[0].rgProperties[0].dwStatus, DBPROPSTATUS_BADVALUE);
	COMPARE(m_rgPropSets[0].rgProperties[1].dwStatus, DBPROPSTATUS_OK);

	//Get the property back.
	TESTC_(m_hr=pIProp->GetProperties(0, NULL, &cPropSets, &rgPropSets), S_OK)
	TESTC(cPropSets==1 && rgPropSets && rgPropSets[0].rgProperties)
	COMPARE(rgPropSets[0].cProperties, 1);
	COMPARE(rgPropSets[0].rgProperties[0].dwPropertyID, DBPROP_INIT_PROMPT);
	COMPARE(CompareVariant(&rgPropSets[0].rgProperties[0].vValue, &m_rgPropSets[0].rgProperties[1].vValue), TRUE);

CLEANUP:
	//Restore to original (valid) VT type.
	if(m_rgPropSets && m_rgPropSets[0].rgProperties &&
		(V_VT(&m_rgPropSets[0].rgProperties[0].vValue) == VT_HRESULT))
		V_VT(&m_rgPropSets[0].rgProperties[0].vValue) = VT_I4;
	FreeProperties(&cPropSets, &rgPropSets);
	FreeProps();
	SAFE_RELEASE(pIProp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCSpecialCases::Terminate()
{ 
	ReleaseAll(); 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CBinderProp::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END
