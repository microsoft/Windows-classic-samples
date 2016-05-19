//--------------------------------------------------------------------
// Microsoft OLE/DB Testing
//
// Copyright 1995-2000 Microsoft Corporation. 
// All Rights Reserved.
//
// @doc This module 
//
// @module irowdel.CPP 
//
#include "modstandard.hpp"	// Standard headers to be precompiled in modcore.cpp			
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "irowdel.h"		// Testcase's header 

#define MAXROWS	50		// Number of Rows in Table

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xa5223690, 0x0e43, 0x11cf, { 0x98, 0x96, 0x00, 0xaa, 0x00, 0x37, 0xda, 0x9b }};
DECLARE_MODULE_NAME("IRowsetDelete");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for IRowsetChange::Delete Interfaces");
DECLARE_MODULE_VERSION(814566995);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END
      

#ifdef _WIN64
//this would be more descriptive as DBROWCOUNT but since
//privlib want a ULONG for it's GetProperty, eventhough
//properties are unsigned, ULONG to VARIANT will be handled instead
#define V_DBCOUNTITEM(X)		V_UNION(X, ullVal)
#else
#define V_DBCOUNTITEM(X)		V_UNION(X, ulVal)
#endif	// _WIN64


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Globals
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
ULONG g_ulRowCount;

//--------------------------------------------------------------------
// @func Module level initialization routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleInit(CThisTestModule * pThisTestModule)
{
	if (ModuleCreateDBSession(pThisTestModule))
	{
		return TRUE;
	}
	return FALSE;
}	
  

//--------------------------------------------------------------------
// @func Module level termination routine
//
// @rdesc Success or Failure
// 		@flag  TRUE  | Successful initialization
//		@flag  FALSE | Initialization problems
//
BOOL ModuleTerminate(CThisTestModule * pThisTestModule)
{
	//Free the interface we got in ModuleCreateDBSession(
	return ModuleReleaseDBSession(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class TCIROWDEL Base Class for IRowsetChange:DeleteRows Testcases
class TCIROWDEL : public CRowsetObject
{
	public:
		// @cmember Constructor
		TCIROWDEL(LPWSTR wszTestCaseName) : CRowsetObject(wszTestCaseName)
		{
			m_pIRowset				= NULL;
			m_pIRowsetChange		= NULL;
			m_pIRowsetInfo			= NULL;
			m_pTableDel				= NULL;
		};

		// @cmember Destructor
		virtual ~TCIROWDEL(){};

		// @cmember QI for IRowsetChange with Properties set
		BOOL QIWithProperties
		(
			ULONG			cPropertySets,	// @parm [IN] # Prop's
			DBPROPSET		*rgPropertySets // @parm [IN] Array of prop's
		);

	protected:
		// @cmember IRowset Interface
		IRowset				*m_pIRowset;
		// @cmember IRowsetChange Interface
		IRowsetChange		*m_pIRowsetChange;
		// @cmember IRowsetInfo Interface
		IRowsetInfo			*m_pIRowsetInfo;
		// @cmember CTable Object
		CTable				*m_pTableDel;
		// @cmember Array of Property Sets
		DBPROPSET			m_rgPropertySets[1];
		// @cmember	Array of Properties
		DBPROP				m_rgProperties[4];
};


// @class TCIERROR Base Class for IRowsetChange::ErrorArray Testcases
class TCIERROR : public CRowsetObject
{
	public:
		// @cmember Constructor
		TCIERROR(LPWSTR wszTestCaseName) : CRowsetObject(wszTestCaseName)
		{
			m_pIRowsetUpdate		= NULL;
			m_pIRowsetInfo			= NULL;
			m_pIRowset				= NULL;
			m_pIRowsetChange		= NULL;
			m_pTableDel				= NULL;
			m_pTableDel2			= NULL;
			m_cRowsObtained			= 0;
			m_rghRows				= 0;
			m_ulMaxPendingRows		= 0;
			m_ulUpdFlags			= DBPROPVAL_UP_DELETE;
		};

		// @cmember Destructor
		virtual ~TCIERROR(){};

		// @cmember Returns a IRowsetChange Pointer with Properties
		HRESULT PrepIRowsetChange
		(
			ULONG		cPropertySets,	// @parm [IN] # Prop's
			DBPROPSET	*rgPropertySets,// @parm [IN] Array of prop's
			CTable		*pTable,		// @parm [IN] Table Name
			BOOL		Update = FALSE	// @parm [IN] Get an Update object
		);

		// @cmember Returns a IRowsetChange Pointer with Properties
		HRESULT CleanupIRowsetChange();

	protected:
		// @cmember IRowset Interface
		IRowset *			m_pIRowset;
		// @cmember IRowsetInfo Interface
		IRowsetInfo			*m_pIRowsetInfo;
		// @cmember IRowsetChange Interface
		IRowsetChange *		m_pIRowsetChange;
		// @cmember IRowsetUpdate Interface
		IRowsetUpdate *	m_pIRowsetUpdate;
		// @cmember CTable Object
		CTable *			m_pTableDel;
		// @cmember CTable Object
		CTable *			m_pTableDel2;
		// @cmember Number of rows returned
		DBCOUNTITEM			m_cRowsObtained;
		// @cmember Pointer to the hRows
		HROW *				m_rghRows;
		// @cmember max pendingrows
		ULONG_PTR			m_ulMaxPendingRows;
		// @cmember Array of Property Sets
		DBPROPSET			m_rgPropertySets[1];
		// @cmember	Array of Properties
		DBPROP				m_rgProperties[4];
		// @cmember The Updatability Flags for DBPROP_UPDATABILITY
		ULONG_PTR			m_ulUpdFlags;
};

// {{ TCW_TEST_CASE_MAP(TCIROWDEL_Properties)
//--------------------------------------------------------------------
// @class IRowsetChange::DeleteRows
//
class TCIROWDEL_Properties : public TCIROWDEL { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIROWDEL_Properties,TCIROWDEL);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember No Properties Set - E_NOINTERFACE
	int Variation_1();
	// @cmember IRowset Properties Set - DB_E_ERRORSOCCURRED
	int Variation_2();
	// @cmember IRowsetLocate Properties Set - S_OK or DB_E_ERRORSOCCURRED
	int Variation_3();
	// @cmember IRowsetChange Properties Set - S_OK or DB_E_ERRORSOCCURRED
	int Variation_4();
	// @cmember IRowsetUpdate Properties Set - S_OK or DB_E_ERRORSOCCURRED
	int Variation_5();
	// @cmember IRowsetChange & CanHoldRows Properties Set - S_OK
	int Variation_6();
	// @cmember IRowsetChange & OtherInsert Properties Set - S_OK
	int Variation_7();
	// @cmember IRowsetChange & OtherUpdateDelete Properties Set - S_OK
	int Variation_8();
	// @cmember IRowsetChange & OwnUpdateDelete Properties Set - S_OK
	int Variation_9();
	// @cmember IRowsetChange & RemoveDeleted Properties Set - S_OK
	int Variation_10();
	// @cmember IRowsetChange & ServerCursor Properties Set - S_OK
	int Variation_11();
	// @cmember IRowsetChange & OtherInsert & RemoveDeleted Properties Set - S_OK
	int Variation_12();
	// @cmember IRowsetChange & IRowsetUpdate Properties Set - S_OK
	int Variation_13();
	// @cmember IRowsetChange & IRowsetUpdate & CanHoldRows Properties Set - S_OK
	int Variation_14();
	// @cmember IRowsetChange & IRowsetUpdate & OtherInsert Properties Set - S_OK
	int Variation_15();
	// @cmember IRowsetChange & IRowsetUpdate & OtherUpdateDelete Properties Set - S_OK
	int Variation_16();
	// @cmember IRowsetChange & IRowsetUpdate & OwnUpdateDelete Properties Set - S_OK
	int Variation_17();
	// @cmember IRowsetChange & IRowsetUpdate & RemoveDeleted Properties Set - S_OK
	int Variation_18();
	// @cmember IRowsetChange & IRowsetUpdate & ServerCursor Properties Set - S_OK
	int Variation_19();
	// @cmember IRowsetChange & IRowsetUpdate & OtherInsert & RemoveDeleted Properties Set - S_OK
	int Variation_20();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIROWDEL_Properties)
#define THE_CLASS TCIROWDEL_Properties
BEG_TEST_CASE(TCIROWDEL_Properties, TCIROWDEL, L"IRowsetChange::DeleteRows")
	TEST_VARIATION(1, 		L"No Properties Set - E_NOINTERFACE")
	TEST_VARIATION(2, 		L"IRowset Properties Set - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(3, 		L"IRowsetLocate Properties Set - S_OK or DB_E_ERRORSOCCURRED")
	TEST_VARIATION(4, 		L"IRowsetChange Properties Set - S_OK or DB_E_ERRORSOCCURRED")
	TEST_VARIATION(5, 		L"IRowsetUpdate Properties Set - S_OK or DB_E_ERRORSOCCURRED")
	TEST_VARIATION(6, 		L"IRowsetChange & CanHoldRows Properties Set - S_OK")
	TEST_VARIATION(7, 		L"IRowsetChange & OtherInsert Properties Set - S_OK")
	TEST_VARIATION(8, 		L"IRowsetChange & OtherUpdateDelete Properties Set - S_OK")
	TEST_VARIATION(9, 		L"IRowsetChange & OwnUpdateDelete Properties Set - S_OK")
	TEST_VARIATION(10, 		L"IRowsetChange & RemoveDeleted Properties Set - S_OK")
	TEST_VARIATION(11, 		L"IRowsetChange & ServerCursor Properties Set - S_OK")
	TEST_VARIATION(12, 		L"IRowsetChange & OtherInsert & RemoveDeleted Properties Set - S_OK")
	TEST_VARIATION(13, 		L"IRowsetChange & IRowsetUpdate Properties Set - S_OK")
	TEST_VARIATION(14, 		L"IRowsetChange & IRowsetUpdate & CanHoldRows Properties Set - S_OK")
	TEST_VARIATION(15, 		L"IRowsetChange & IRowsetUpdate & OtherInsert Properties Set - S_OK")
	TEST_VARIATION(16, 		L"IRowsetChange & IRowsetUpdate & OtherUpdateDelete Properties Set - S_OK")
	TEST_VARIATION(17, 		L"IRowsetChange & IRowsetUpdate & OwnUpdateDelete Properties Set - S_OK")
	TEST_VARIATION(18, 		L"IRowsetChange & IRowsetUpdate & RemoveDeleted Properties Set - S_OK")
	TEST_VARIATION(19, 		L"IRowsetChange & IRowsetUpdate & ServerCursor Properties Set - S_OK")
	TEST_VARIATION(20, 		L"IRowsetChange & IRowsetUpdate & OtherInsert & RemoveDeleted Properties Set - S_OK")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCIERROR_ErrorArray)
//--------------------------------------------------------------------
// @class IRowsetChange::ErrorArray
//
class TCIERROR_ErrorArray : public TCIERROR { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCIERROR_ErrorArray,TCIERROR);
	// }} TCW_DECLARE_FUNCS_END
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Hard Delete 1 Row with NULL prgErrors - E_INVALIDARG
	int Variation_1();
	// @cmember Hard Delete 1 Row with NULL rghRows - E_INVALIDARG
	int Variation_2();
	// @cmember Hard Delete 0 Row with NULL rghRows - S_OK
	int Variation_3();
	// @cmember Hard Delete 1 row with Ignoring Errors - S_OK
	int Variation_4();
	// @cmember Hard Delete 1 Row with NULL pcErrors - S_OK
	int Variation_5();
	// @cmember Hard Delete 1 Row valid pcErrors and prgErrors - S_OK
	int Variation_6();
	// @cmember Hard Delete 1 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
	int Variation_7();
	// @cmember Hard Delete 50 Row with NULL prgErrors- E_INVALIDARG
	int Variation_8();
	// @cmember Hard Delete 5 Row with Ignoring Errors - DB_S_ERRORSOCCURRED
	int Variation_9();
	// @cmember Hard Delete 7 Row with NULL pcErrors- DB_S_ERRORSOCCURRED
	int Variation_10();
	// @cmember Hard Delete 3 Row valid pcErrors and prgErrors - S_OK
	int Variation_11();
	// @cmember Hard Delete 6 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
	int Variation_12();
	// @cmember Soft Delete 1 Row with NULL prgErrors - E_INVALIDARG
	int Variation_13();
	// @cmember Soft Delete 1 Row with NULL rghRows - E_INVALIDARG
	int Variation_14();
	// @cmember Soft Delete 0 Row with NULL rghRows - S_OK
	int Variation_15();
	// @cmember Soft Delete 1 row with Ignoring Errors - S_OK
	int Variation_16();
	// @cmember Soft Delete 1 Row with NULL pcErrors - S_OK
	int Variation_17();
	// @cmember Soft Delete 1 Row valid pcErrors and prgErrors - S_OK
	int Variation_18();
	// @cmember Soft Delete 1 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
	int Variation_19();
	// @cmember Soft Delete 50 Row with NULL prgErrors- E_INVALIDARG
	int Variation_20();
	// @cmember Soft Delete 5 Row with Ignoring Errors - DB_S_ERRORSOCCURRED
	int Variation_21();
	// @cmember Soft Delete 7 Row with NULL pcErrors- DB_S_ERRORSOCCURRED
	int Variation_22();
	// @cmember Soft Delete 3 Row valid pcErrors and prgErrors - S_OK
	int Variation_23();
	// @cmember Soft Delete 6 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
	int Variation_24();
	// @cmember array of bad bad row handles, DBROWSTATUS_E_INVALID'em
	int Variation_25();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCIERROR_ErrorArray)
#define THE_CLASS TCIERROR_ErrorArray
BEG_TEST_CASE(TCIERROR_ErrorArray, TCIERROR, L"IRowsetChange::ErrorArray")
	TEST_VARIATION(1, 		L"Hard Delete 1 Row with NULL prgErrors - E_INVALIDARG")
	TEST_VARIATION(2, 		L"Hard Delete 1 Row with NULL rghRows - E_INVALIDARG")
	TEST_VARIATION(3, 		L"Hard Delete 0 Row with NULL rghRows - S_OK")
	TEST_VARIATION(4, 		L"Hard Delete 1 row with Ignoring Errors - S_OK")
	TEST_VARIATION(5, 		L"Hard Delete 1 Row with NULL pcErrors - S_OK")
	TEST_VARIATION(6, 		L"Hard Delete 1 Row valid pcErrors and prgErrors - S_OK")
	TEST_VARIATION(7, 		L"Hard Delete 1 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(8, 		L"Hard Delete 50 Row with NULL prgErrors- E_INVALIDARG")
	TEST_VARIATION(9, 		L"Hard Delete 5 Row with Ignoring Errors - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(10, 		L"Hard Delete 7 Row with NULL pcErrors- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(11, 		L"Hard Delete 3 Row valid pcErrors and prgErrors - S_OK")
	TEST_VARIATION(12, 		L"Hard Delete 6 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(13, 		L"Soft Delete 1 Row with NULL prgErrors - E_INVALIDARG")
	TEST_VARIATION(14, 		L"Soft Delete 1 Row with NULL rghRows - E_INVALIDARG")
	TEST_VARIATION(15, 		L"Soft Delete 0 Row with NULL rghRows - S_OK")
	TEST_VARIATION(16, 		L"Soft Delete 1 row with Ignoring Errors - S_OK")
	TEST_VARIATION(17, 		L"Soft Delete 1 Row with NULL pcErrors - S_OK")
	TEST_VARIATION(18, 		L"Soft Delete 1 Row valid pcErrors and prgErrors - S_OK")
	TEST_VARIATION(19, 		L"Soft Delete 1 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(20, 		L"Soft Delete 50 Row with NULL prgErrors- E_INVALIDARG")
	TEST_VARIATION(21, 		L"Soft Delete 5 Row with Ignoring Errors - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(22, 		L"Soft Delete 7 Row with NULL pcErrors- DB_S_ERRORSOCCURRED")
	TEST_VARIATION(23, 		L"Soft Delete 3 Row valid pcErrors and prgErrors - S_OK")
	TEST_VARIATION(24, 		L"Soft Delete 6 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED")
	TEST_VARIATION(25, 		L"array of bad bad row handles, DBROWSTATUS_E_INVALID'em")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END
// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(2, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCIROWDEL_Properties)
	TEST_CASE(2, TCIERROR_ErrorArray)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCIROWDEL_Properties)
//*-----------------------------------------------------------------------
//| Test Case:		TCIROWDEL_Properties - IRowsetChange::DeleteRows
//|	Created:		09/26/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIROWDEL_Properties::Init()
{
	ULONG ulUpdValue = 0;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIROWDEL::Init())
	// }}
	{
		// Check to see if IRowsetChange is supported
		if( !SupportedProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown))
		{
			return TEST_SKIPPED;
		}
		// Create a table with MAXROWS rows of all supported DataTypes
		m_pTableDel=new CTable(m_pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName);

		if(FAILED(m_pTableDel->CreateTable(MAXROWS, 1, NULL, PRIMARY)))
		{
			return FALSE;
		}
		// Call IOpenRowset to return a Rowset
		if(SUCCEEDED(m_pTableDel->CreateRowset(	USE_OPENROWSET, 
												IID_IRowsetInfo, 
												0, 
												NULL, 
												(IUnknown**)&m_pIRowsetInfo, 
												NULL, 
												NULL, 
												NULL)))
		{
			return TRUE;
		}
	}
		
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc No Properties Set - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_1()
{
	BOOL fPassFail = FALSE;

	// Needed for DBPROP_IRowset
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties	= 0;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	// Call IOpenRowset to return a Rowset
	if(!CHECK(m_hr=m_pTableDel->CreateRowset(USE_OPENROWSET,IID_IRowset,1,
				m_rgPropertySets,(IUnknown**)&m_pIRowset,NULL,NULL,NULL), S_OK))
		return TEST_FAIL;

	// Check the QI for IRowsetChange
	if(!VerifyInterface(m_pIRowset,IID_IRowsetChange,
							ROWSET_INTERFACE,(IUnknown**)&m_pIRowsetChange))
	{
		if(COMPARE(m_pIRowsetChange, NULL))
			fPassFail = TRUE;
	}
	else
	{
		// Check to see if IRowsetChange is VARIANT_TRUE
		COMPARE(GetProperty(DBPROP_IRowsetChange,DBPROPSET_ROWSET,
											m_pIRowsetInfo,VARIANT_TRUE), TRUE);

		if(COMPARE(!m_pIRowsetChange, NULL))
			fPassFail = TRUE;
	}

	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetChange);

	if(fPassFail)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc IRowset Properties Set - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_2()
{
	BOOL fPassFail = FALSE;

	// Needed for DBPROP_IRowset
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties	= 1;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties->dwPropertyID = DBPROP_IRowset;
	m_rgProperties->dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties->colid = DB_NULLID;
	m_rgProperties->vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties->vValue) = VARIANT_FALSE;

	// Call IOpenRowset to return a Rowset
	m_hr=m_pTableDel->CreateRowset(USE_OPENROWSET,IID_IRowset,1,m_rgPropertySets,
											(IUnknown**)&m_pIRowset,NULL,NULL,NULL);

	// Check the Property Status 
	COMPARE(m_rgProperties->dwStatus, DBPROPSTATUS_NOTSETTABLE);

	// Property is Supported but ReadOnly
	if( (CHECK(m_hr, DB_E_ERRORSOCCURRED)) && (COMPARE(m_pIRowset, NULL)) )
		fPassFail = TRUE;

	SAFE_RELEASE(m_pIRowset);

	if(fPassFail)
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc IRowsetLocate Properties Set - S_OK or DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_3()
{
	// Needed for DBPROP_IRowsetLocate
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 1;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties->dwPropertyID = DBPROP_IRowsetLocate;
	m_rgProperties->dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties->colid = DB_NULLID;
	m_rgProperties->vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties->vValue) = VARIANT_TRUE;

	// Set the DBPROP_IRowsetLocate property to TRUE
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange Properties Set - S_OK or DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_4()
{
	// Needed for DBPROP_IRowsetUpdate
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 1;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties->dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties->dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties->colid = DB_NULLID;
	m_rgProperties->vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties->vValue) = VARIANT_TRUE;

	// Set the DBPROP_IRowsetUpdate property to TRUE
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc IRowsetUpdate Properties Set - S_OK or DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_5()
{
	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 1;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties->dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties->dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties->colid = DB_NULLID;
	m_rgProperties->vValue.vt = VT_BOOL;
	V_BOOL(&m_rgProperties->vValue) = VARIANT_TRUE;

	// Set the DBPROP_IRowsetChange property to TRUE
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & CanHoldRows Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_6()
{
	// Needed for DBPROP_IRowsetChange & CanHoldRows
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_CANHOLDROWS;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & CanHoldRows Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;

}
// }}

// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & OtherInsert Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_7()
{
	// Needed for DBPROP_IRowsetChange & OtherInsert
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_OTHERINSERT;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & OtherInsert Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & OtherUpdateDelete Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_8()
{
	// Needed for DBPROP_IRowsetChange & OtherUpdateDelete
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_OTHERUPDATEDELETE;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & OtherUpdateDelete Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & OwnUpdateDelete Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_9()
{
	// Needed for DBPROP_IRowsetChange & OwnUpdateDelete
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_OWNUPDATEDELETE;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & OwnUpdateDelete Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & RemoveDeleted Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_10()
{
	// Needed for DBPROP_IRowsetChange & RemoveDeleted
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_REMOVEDELETED;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & RemoveDeleted Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & ServerCursor Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_11()
{
	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_SERVERCURSOR;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & ServerCursor Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & OtherInsert & RemoveDeleted Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_12()
{
	// Needed for DBPROP_IRowsetChange & OtherInsert & RemoveDeleted
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_OTHERINSERT;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_REMOVEDELETED;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;
	
	// IID_IRowsetChange & OtherInsert & RemoveDeleted Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_13()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IID_IRowsetUpdate Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & CanHoldRows Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_14()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & CanHoldRows
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_CANHOLDROWS;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IRowsetUpdate & CanHoldRows Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & OtherInsert Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_15()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & OtherInsert
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_OTHERINSERT;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IRowsetUpdate & OtherInsert Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & OtherUpdateDelete Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_16()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & OtherUpdateDelete
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_OTHERUPDATEDELETE;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IRowsetUpdate & OtherUpdateDelete Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & OwnUpdateDelete Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_17()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & OwnUpdateDelete
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_OWNUPDATEDELETE;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IRowsetUpdate & OwnUpdateDelete Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & RemoveDeleted Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_18()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & RemoveDeleted
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_REMOVEDELETED;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IRowsetUpdate & RemoveDeleted Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & ServerCursor Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_19()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & ServerCursor
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_SERVERCURSOR;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & IRowsetUpdate & ServerCursor Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}

// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc IRowsetChange & IRowsetUpdate & OtherInsert & RemoveDeleted Properties Set - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIROWDEL_Properties::Variation_20()
{
	// Needed for DBPROP_IRowsetChange & IRowsetUpdate & OtherInsert & RemoveDeleted
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 4;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[1].vValue)) = VARIANT_TRUE;

	m_rgProperties[2].dwPropertyID = DBPROP_OTHERINSERT;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	m_rgProperties[3].dwPropertyID = DBPROP_REMOVEDELETED;
	m_rgProperties[3].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[3].colid = DB_NULLID;
	m_rgProperties[3].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[3].vValue)) = VARIANT_TRUE;

	// IID_IRowsetChange & OtherInsert & RemoveDeleted Properties set
	if(QIWithProperties(1, m_rgPropertySets))
		return TEST_PASS;
	else
		return TEST_FAIL;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIROWDEL_Properties::Terminate()
{
	ULONG	cCnt	= 0;
	
	if(TCIROWDEL::Terminate())
	{
		SAFE_RELEASE(m_pIRowsetInfo);

		// Drop the AutoMakeTable
		if(m_pTableDel)
		{
			//if an ini file is being used then delete and repopulate
			if(GetModInfo()->GetFileName())
			{
				//delete all rows in the table.
				if(m_pTableDel->DeleteRows(ALLROWS) == S_OK)
				{
					// RePopulate table in case an .ini file is being used.
					for(cCnt=1;cCnt<=MAXROWS;cCnt++)
					{
						if(m_pTableDel->Insert(cCnt, PRIMARY) != S_OK)
						{
							return FALSE;
						}
					}
				}
			}
			CHECK(m_pTableDel->DropTable(),S_OK);
			delete m_pTableDel;
			m_pTableDel = NULL;
		}
		return TRUE;
	}
	return FALSE;
}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc Setup IRowset Pointer with specified Properties.
// IRowset in m_pIRowset on a valid rowset object. 

BOOL TCIROWDEL::QIWithProperties(ULONG cPropertySet, DBPROPSET *rgPropertySets)
{
	BOOL		fPassFail	= FALSE;
	ULONG		ulFailed	= 0;

	// Call IOpenRowset to return a Rowset
	HRESULT hr=m_pTableDel->CreateRowset(USE_OPENROWSET, IID_IRowset, cPropertySet, 
						rgPropertySets, (IUnknown**)&m_pIRowset, NULL, NULL, NULL);

	// Loop thru all of the properties
	for(ULONG ulIndex=0; ulIndex < cPropertySet; ulIndex++)
	{
		for(ULONG ulIndex1=0; ulIndex1 < rgPropertySets[ulIndex].cProperties; ulIndex1++)
		{
			//if multiple props were set they might be conflicting on some providers
			if (DB_E_ERRORSOCCURRED		==	hr														&&
				DBPROPSTATUS_CONFLICTING==	rgPropertySets[ulIndex].rgProperties[ulIndex].dwStatus	&&
				1						<	rgPropertySets[ulIndex].cProperties)
			{
				ulFailed++;
				continue;
			}
			// Check to see if the property is supported
			if(!SupportedProperty(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
											DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown))
			{
				COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
				ulFailed++;
			}
			else if( (!SettableProperty(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
												DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown)) &&
					 (!GetProperty(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
								DBPROPSET_ROWSET, (m_pIRowset ? m_pIRowset : (IRowset*)m_pIRowsetInfo), 
								V_BOOL(&(rgPropertySets[ulIndex].rgProperties[ulIndex1].vValue)))) )
			{
				COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_NOTSETTABLE);
				ulFailed++;
			}
			else
				COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_OK);
		}
	}

	// Check the HResult returned
	(!ulFailed) ? CHECK(hr, S_OK) : CHECK(hr, DB_E_ERRORSOCCURRED);

	// Check the Rowset pointer on Failure
	if((FAILED(hr)) && (COMPARE(m_pIRowset, NULL)))
		fPassFail = TRUE;

	// Get an IRowsetChange pointer
	if(m_pIRowset)
	{
		if( (VerifyInterface(m_pIRowset,IID_IRowsetChange,ROWSET_INTERFACE,
						(IUnknown**)&m_pIRowsetChange)) && (m_pIRowsetChange))
			fPassFail = TRUE;
		else if( (!VerifyInterface(m_pIRowset,IID_IRowsetChange,ROWSET_INTERFACE,
						(IUnknown**)&m_pIRowsetChange)) && (!m_pIRowsetChange))
			fPassFail = TRUE;
	}

	// Release IRowset and IRowsetChange
	SAFE_RELEASE(m_pIRowset);
	SAFE_RELEASE(m_pIRowsetChange);

	if(fPassFail)
		return TEST_PASS;
	else
		return TEST_FAIL;
}


// {{ TCW_TC_PROTOTYPE(TCIERROR_ErrorArray)
//*-----------------------------------------------------------------------
//| Test Case:		TCIERROR_ErrorArray - IRowsetChange::ErrorArray
//|	Created:		11/08/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIERROR_ErrorArray::Init()
{
	ULONG_PTR ulUpdValue = 0;

	// {{ TCW_INIT_BASECLASS_CHECK
	if(TCIERROR::Init())
	// }}
	{
		// Check to see if IRowsetChange is supported
		if( !SupportedProperty(DBPROP_IRowsetChange,
								DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown))
			return TEST_SKIPPED;
		
		// Create a table with MAXROWS rows of all supported DataTypes
		m_pTableDel = new CTable(m_pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName);
		m_pTableDel2= new CTable(m_pThisTestModule->m_pIUnknown2, (WCHAR *)gwszModuleName);

		if(FAILED(m_pTableDel->CreateTable(MAXROWS, 1, NULL, PRIMARY)))
			return FALSE;

		// Make sure the Rowset has MAXROWS records
		if( (!IsProviderReadOnly(m_pThisTestModule->m_pIUnknown2)) && 
			(SupportedProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, 
							m_pThisTestModule->m_pIUnknown, ROWSET_INTERFACE)) )
		{
			// Count the rows in the Table
			for(; m_pTableDel->GetRowsOnCTable() < MAXROWS;)
			{
				HRESULT hr=m_pTableDel->Insert(0, PRIMARY, TRUE, NULL, FALSE, 1);
				if(FAILED(hr))
					return FALSE;
			}
		}

		if(FAILED(m_pTableDel2->CreateTable(MAXROWS, 1, NULL, PRIMARY)))
			return FALSE;

		// Make sure the Rowset has MAXROWS records
		if( (!IsProviderReadOnly(m_pThisTestModule->m_pIUnknown2)) && 
			(SupportedProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET, 
							m_pThisTestModule->m_pIUnknown, ROWSET_INTERFACE)) )
		{
			// Count the rows in the Table
			for(; m_pTableDel2->GetRowsOnCTable() < MAXROWS;)
			{
				HRESULT hr=m_pTableDel2->Insert(0, PRIMARY, TRUE, NULL, FALSE, 1);
				if(FAILED(hr))
					return FALSE;
			}
		}

		// Call IOpenRowset to return a Rowset
		if(FAILED(m_pTableDel->CreateRowset(USE_OPENROWSET, IID_IRowsetChange, 0, NULL, 
										(IUnknown**)&m_pIRowsetChange, NULL, NULL, NULL)))
			return FALSE;

		// Get the MaxPendingRows allowed
		GetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, m_pIRowsetChange, &m_ulMaxPendingRows);
		
		// Figure out the DBPROP_UPDATABILITY value if ReadOnly
		if( !(SettableProperty(DBPROP_UPDATABILITY,DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown)) &&
			(GetProperty(DBPROP_UPDATABILITY,DBPROPSET_ROWSET,m_pIRowsetChange,&ulUpdValue) && 
			(ulUpdValue & m_ulUpdFlags)) )
			m_ulUpdFlags = ulUpdValue;

		SAFE_RELEASE(m_pIRowsetChange);
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 1 Row with NULL prgErrors - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_1()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;
	
	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
	{
		goto CLEANUP;
	}
	// IRowsetChange with a valid cRows and NULL rghRows (E_INVALIDARG)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,NULL,&rgRowStatus), E_INVALIDARG))
		fPassFail = TEST_PASS;

	// Compare the Status values
	COMPARE(rgRowStatus, NULL);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);
	
	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 1 Row with NULL rghRows - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_2()
{
	BOOL fPassFail = TEST_SKIPPED;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with cRows of 100 and NULL rghRows (E_INVALIDARG)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,100,NULL,NULL), E_INVALIDARG))
		fPassFail = TEST_PASS;

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);
	
	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 0 Row with NULL rghRows - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_3()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;
	HROW		hRow		= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a 0 cRows and NULL rghRows (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,0,&hRow,&rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 1 row with Ignoring Errors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_4()
{
	BOOL fPassFail = TEST_SKIPPED;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a NULL rgStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,m_rghRows,NULL), S_OK))
		fPassFail = TEST_PASS;

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);
	
	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 1 Row with NULL pcErrors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_5()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a valid rgRowStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[1],&rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 1 Row valid pcErrors and prgErrors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_6()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a valid pcErrors and valid rgRowStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[2],&rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 1 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_7()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// Delete the 3rd Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[2],NULL);

	// IRowsetChange with a valid pcError and valid rgRowStatus (DB_E_ERRORSOCCURRED)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[2],&rgRowStatus), DB_E_ERRORSOCCURRED))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_E_DELETED);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 50 Row with NULL prgErrors- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_8()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange))
		goto CLEANUP;

	// IRowsetChange with a valid pcErrors and NULL rgRowStatus (E_INVALIDARG)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,50,NULL,&rgRowStatus), E_INVALIDARG))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, NULL);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 5 Row with Ignoring Errors - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_9()
{
	BOOL		fPassFail = TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[5];

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 5))
		goto CLEANUP;

	// Delete 1-3 Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER, 3, m_rghRows, NULL);

	// IRowsetChange with 3 deleted and 2 valid rows (DB_S_ERRORSOCCURRED)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,5,m_rghRows,rgRowStatus), DB_S_ERRORSOCCURRED))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus[0], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[1], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[2], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[3], DBROWSTATUS_S_OK);
	COMPARE(rgRowStatus[4], DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 7 Row with NULL pcErrors- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_10()
{
	BOOL		fPassFail = TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[7];

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 7))
		goto CLEANUP;

	// Delete 1-4 Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER, 4, m_rghRows, NULL);

	// IRowsetChange with 7 deleted and 3 valid rows (DB_S_ERRORSOCCURRED)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,7,m_rghRows,rgRowStatus), DB_S_ERRORSOCCURRED))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus[0], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[1], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[2], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[3], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[4], DBROWSTATUS_S_OK);
	COMPARE(rgRowStatus[5], DBROWSTATUS_S_OK);
	COMPARE(rgRowStatus[6], DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 3 Row valid pcErrors and prgErrors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_11()
{
	BOOL		fPassFail = TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[3];

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 3))
		goto CLEANUP;

	// IRowsetChange with a valid pcError and valid rgRowStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,3,&m_rghRows[3],rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus[0], DBROWSTATUS_S_OK);
	COMPARE(rgRowStatus[1], DBROWSTATUS_S_OK);
	COMPARE(rgRowStatus[2], DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Hard Delete 6 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_12()
{
	BOOL		fPassFail = TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[6];

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 6))
		goto CLEANUP;

	// Delete 1-6 Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER, 6, m_rghRows, NULL);

	// IRowsetChange with a valid pcError and valid prgRowStatus (DB_E_ERRORSOCCURRED)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,6,m_rghRows,rgRowStatus), DB_E_ERRORSOCCURRED))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus[0], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[1], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[2], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[3], DBROWSTATUS_E_DELETED);
	COMPARE(rgRowStatus[4], DBROWSTATUS_E_DELETED);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 1 Row with NULL prgErrors - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_13()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;
	
	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a valid cRows and NULL rghRows (E_INVALIDARG)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,NULL,&rgRowStatus), E_INVALIDARG))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, NULL);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 1 Row with NULL rghRows - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_14()
{
	BOOL fPassFail = TEST_SKIPPED;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with cRows of 100 and NULL rghRows (E_INVALIDARG)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(1,100,NULL,NULL), E_INVALIDARG))
		fPassFail = TEST_PASS;

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 0 Row with NULL rghRows - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_15()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a 0 cRows and NULL rghRows (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,0,NULL,&rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 1 row with Ignoring Errors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_16()
{
	BOOL fPassFail = TEST_SKIPPED;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a NULL rgStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,m_rghRows,NULL), S_OK))
		fPassFail = TEST_PASS;

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 1 Row with NULL pcErrors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_17()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a valid rgRowStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[1],&rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 1 Row valid pcErrors and prgErrors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_18()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// IRowsetChange with a valid pcErrors and valid rgRowStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[2],&rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 1 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_19()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// Delete the 3rd Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[2],NULL);

	// IRowsetChange with a valid pcError and valid rgRowStatus (DB_E_ERRORSOCCURRED)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,1,&m_rghRows[2],&rgRowStatus), DB_E_ERRORSOCCURRED))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, DBROWSTATUS_E_DELETED);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 50 Row with NULL prgErrors- E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_20()
{
	BOOL		fPassFail	= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus	= NULL;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 3;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange))
		goto CLEANUP;

	// IRowsetChange with a valid pcErrors and NULL rgRowStatus (E_INVALIDARG)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,50,NULL,&rgRowStatus), E_INVALIDARG))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus, NULL);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 5 Row with Ignoring Errors - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_21()
{
	BOOL		fPassFail			= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[5];
	DBCOUNTITEM	ulMaxPendingRows	= m_ulMaxPendingRows;
	DBCOUNTITEM	i					= 0;
	DBCOUNTITEM	x					= 0;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 4;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	m_rgProperties[3].dwPropertyID = DBPROP_MAXPENDINGROWS;
	m_rgProperties[3].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgProperties[3].colid = DB_NULLID;
	m_rgProperties[3].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[3].vValue) = ulMaxPendingRows;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 5))
		goto CLEANUP;

	// Set the MaxPendingRows to the allowed limit
	if( (!ulMaxPendingRows) || (ulMaxPendingRows > 5) )
		ulMaxPendingRows = 5;

	// Delete MaxPendingRows  
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,ulMaxPendingRows,m_rghRows,NULL);

	// IRowsetChange with MaxPendingRows deleted and the remaining  valid rows (DB_S_ERRORSOCCURRED)
	m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,5,m_rghRows,rgRowStatus);
	
	(ulMaxPendingRows <= 5) ? CHECK(m_hr,DB_E_ERRORSOCCURRED) : CHECK(m_hr,DB_S_ERRORSOCCURRED);
	
	for (i=0; i<ulMaxPendingRows; i++)
		COMPARE(rgRowStatus[i], DBROWSTATUS_E_DELETED);

	for (x=ulMaxPendingRows; x<5; x++)
		COMPARE(rgRowStatus [x], DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED);

	fPassFail = TEST_PASS;

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 7 Row with NULL pcErrors- DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_22()
{
	BOOL		fPassFail = TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[7];
	LONG		lMaxPendingRows = (LONG)m_ulMaxPendingRows;
	ULONG		i	=0;
	ULONG		x	=0;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 4;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	m_rgProperties[3].dwPropertyID = DBPROP_MAXPENDINGROWS;
	m_rgProperties[3].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgProperties[3].colid = DB_NULLID;
	m_rgProperties[3].vValue.vt = VT_I4;
	m_rgProperties[3].vValue.lVal = lMaxPendingRows;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 7))
		goto CLEANUP;

	// Set the MaxPendingRows to the allowed limit
	if( (!lMaxPendingRows) || (lMaxPendingRows > 7) )
		lMaxPendingRows = 7;

	// Delete 1-4 Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,lMaxPendingRows,m_rghRows,NULL);

	// IRowsetChange with 7 deleted and 3 valid rows (DB_S_ERRORSOCCURRED)
	m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,7,m_rghRows,rgRowStatus);

	(lMaxPendingRows <= 7) ? CHECK(m_hr,DB_E_ERRORSOCCURRED) : CHECK(m_hr,DB_S_ERRORSOCCURRED);

	for (i=0; i<(ULONG)lMaxPendingRows; i++)
		COMPARE(rgRowStatus[i], DBROWSTATUS_E_DELETED);

	for (x=lMaxPendingRows; x<7; x++)
		COMPARE(rgRowStatus[x], DBROWSTATUS_E_MAXPENDCHANGESEXCEEDED);

	fPassFail = TEST_PASS;

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 3 Row valid pcErrors and prgErrors - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_23()
{
	BOOL		fPassFail	 = TEST_SKIPPED;
	DBROWSTATUS	*rgRowStatus = NULL;
	LONG		lMaxPendingRows = (LONG)m_ulMaxPendingRows;
	ULONG		i	=0;

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 4;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	m_rgProperties[3].dwPropertyID = DBPROP_MAXPENDINGROWS;
	m_rgProperties[3].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgProperties[3].colid = DB_NULLID;
	m_rgProperties[3].vValue.vt = VT_I4;
	m_rgProperties[3].vValue.lVal = lMaxPendingRows;

	rgRowStatus=(DBROWSTATUS *)PROVIDER_ALLOC(sizeof(DBROWSTATUS)*3);

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 3))
		goto CLEANUP;

	// Set the MaxPendingRows to the allowed limit
	if( (!lMaxPendingRows) || (lMaxPendingRows > 3) )
		lMaxPendingRows = 3;

	// IRowsetChange with a valid pcError and valid rgRowStatus (S_OK)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,lMaxPendingRows,&m_rghRows[lMaxPendingRows],rgRowStatus), S_OK))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	for(i=0; i<(ULONG)lMaxPendingRows; i++)
		COMPARE(rgRowStatus[i], DBROWSTATUS_S_OK);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);
	PROVIDER_FREE(rgRowStatus);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Soft Delete 6 Row valid pcErrors and prgErrors - DB_S_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_24()
{
	BOOL		fPassFail	 = TEST_SKIPPED;
	DBROWSTATUS	*rgRowStatus = NULL;
	LONG		lMaxPendingRows = (LONG)m_ulMaxPendingRows;
	ULONG		i;
	
	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 4;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	m_rgProperties[2].dwPropertyID = DBPROP_IRowsetUpdate;
	m_rgProperties[2].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[2].colid = DB_NULLID;
	m_rgProperties[2].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[2].vValue)) = VARIANT_TRUE;

	m_rgProperties[3].dwPropertyID = DBPROP_MAXPENDINGROWS;
	m_rgProperties[3].dwOptions = DBPROPOPTIONS_OPTIONAL;
	m_rgProperties[3].colid = DB_NULLID;
	m_rgProperties[3].vValue.vt = VT_I4;
	m_rgProperties[3].vValue.lVal = lMaxPendingRows;

	rgRowStatus=(DBROWSTATUS *)PROVIDER_ALLOC(sizeof(DBROWSTATUS)*6);
	
	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel2,TRUE);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (m_cRowsObtained < 6))
		goto CLEANUP;

	// Set the MaxPendingRows to the allowed limit
	if( (!lMaxPendingRows) || (lMaxPendingRows > 6) )
		lMaxPendingRows = 6;

	// Delete 1-m_lMaxPendingRows Row
	m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,lMaxPendingRows,m_rghRows,NULL);

	// IRowsetChange with a valid pcError and valid prgRowStatus (DB_E_ERRORSOCCURRED)
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,lMaxPendingRows,m_rghRows,rgRowStatus), DB_E_ERRORSOCCURRED))
		fPassFail = TEST_PASS;

	// Clean-up all Allocated Memory and Interfaces
	for(i=0; i<(ULONG)lMaxPendingRows; i++)
		COMPARE(rgRowStatus[i], DBROWSTATUS_E_DELETED);

CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);
	PROVIDER_FREE(rgRowStatus);

	return fPassFail;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc array of bad bad row handles, DBROWSTATUS_E_INVALID'em
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCIERROR_ErrorArray::Variation_25()
{
	BOOL		fPassFail		= TEST_SKIPPED;
	DBROWSTATUS	rgRowStatus[5];
	HROW		hRow[5]			= {ULONG_MAX,ULONG_MAX,ULONG_MAX,ULONG_MAX,ULONG_MAX};

	// Needed for DBPROP_IRowsetChange
	m_rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	m_rgPropertySets[0].cProperties = 2;
	m_rgPropertySets[0].rgProperties = m_rgProperties;

	m_rgProperties[0].dwPropertyID = DBPROP_IRowsetChange;
	m_rgProperties[0].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[0].colid = DB_NULLID;
	m_rgProperties[0].vValue.vt = VT_BOOL;
	V_BOOL(&(m_rgProperties[0].vValue)) = VARIANT_TRUE;

	m_rgProperties[1].dwPropertyID = DBPROP_UPDATABILITY;
	m_rgProperties[1].dwOptions = DBPROPOPTIONS_REQUIRED;
	m_rgProperties[1].colid = DB_NULLID;
	m_rgProperties[1].vValue.vt = VT_I4;
	V_DBCOUNTITEM(&m_rgProperties[1].vValue) = m_ulUpdFlags;

	// Get a IRowsetChange Pointer with Properties Set
	m_hr=PrepIRowsetChange(1,m_rgPropertySets,m_pTableDel);

	// See if the Properties where supported
	if((m_hr == DB_E_ERRORSOCCURRED) && (!m_pIRowsetChange) && (!m_pIRowset))
	{
		goto CLEANUP;
	}
	//if no interface then change is not supported at this level
	if((m_hr == E_NOINTERFACE) && (!m_pIRowsetChange))
	{
		goto CLEANUP;
	}
	fPassFail	 = TEST_FAIL;

	// Check the pointers
	if((!m_pIRowset) || (!m_pIRowsetChange) || (!m_cRowsObtained))
		goto CLEANUP;

	// Release Row Handles
	if(m_pIRowset && m_rghRows)
	{
		CHECK(m_hr=m_pIRowset->ReleaseRows(m_cRowsObtained,m_rghRows,NULL,NULL,NULL),S_OK);
		m_rghRows	= NULL;
	}
	// IRowsetChange with a invalid rghRows, all valid row handles were released
	if(CHECK(m_hr=m_pIRowsetChange->DeleteRows(DB_NULL_HCHAPTER,5,&hRow[0],&rgRowStatus[0]), DB_E_ERRORSOCCURRED))
	{
		fPassFail = TEST_PASS;
	}
	// Clean-up all Allocated Memory and Interfaces
	COMPARE(rgRowStatus[0], DBROWSTATUS_E_INVALID);
	COMPARE(rgRowStatus[1], DBROWSTATUS_E_INVALID);
	COMPARE(rgRowStatus[2], DBROWSTATUS_E_INVALID);
	COMPARE(rgRowStatus[3], DBROWSTATUS_E_INVALID);
	COMPARE(rgRowStatus[4], DBROWSTATUS_E_INVALID);
CLEANUP:
	// Clean-up all Allocated Memory and Interfaces
	CHECK(CleanupIRowsetChange(), S_OK);

	return fPassFail;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCIERROR_ErrorArray::Terminate()
{
	ULONG	cCnt	= 0;
	
	if(TCIERROR::Terminate())
	{
		SAFE_RELEASE(m_pIRowsetInfo);

		// Drop the AutoMakeTable
		if(m_pTableDel) 
		{
			//if an ini file is being used then delete and repopulate
			if(GetModInfo()->GetFileName())
			{
				//delete all rows in the table.
				if(m_pTableDel->DeleteRows(ALLROWS) == S_OK)
				{
					// RePopulate table in case an .ini file is being used.
					for(cCnt=1;cCnt<=MAXROWS;cCnt++)
					{
						if(m_pTableDel->Insert(cCnt, PRIMARY) != S_OK)
						{
							return FALSE;
						}
					}
				}
			}
			CHECK(m_pTableDel->DropTable(),S_OK);
			delete m_pTableDel;
			m_pTableDel = NULL;
		}

		if(m_pTableDel2) 
		{
			//if an ini file is being used then delete and repopulate
			if(GetModInfo()->GetFileName())
			{
				//delete all rows in the table.
				if(m_pTableDel2->DeleteRows(ALLROWS) == S_OK)
				{
					// RePopulate table in case an .ini file is being used.
					for(cCnt=1;cCnt<=MAXROWS;cCnt++)
					{
						if(m_pTableDel2->Insert(cCnt, PRIMARY) != S_OK)
						{
							return FALSE;
						}
					}
				}
			}
			CHECK(m_pTableDel2->DropTable(),S_OK);
			delete m_pTableDel2;
			m_pTableDel2 = NULL;
		}

		return TRUE;
	}

	return FALSE;
}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc Setup IRowsetChange Pointer with specified Properties.
// IRowsetChange in m_pIRowsetChange on a valid rowset object. 

HRESULT	TCIERROR::PrepIRowsetChange(ULONG cPropertySets, DBPROPSET *rgPropertySets, CTable *pTable, BOOL Update)
{	
	ULONG		ulFailed	= 0;
	ULONG		ulPassed	= 0;
	ULONG_PTR	ulValue		= 0;


	// Call IOpenRowset to return a Rowset
	HRESULT hr=pTable->CreateRowset(USE_OPENROWSET, IID_IRowset, cPropertySets, 
						rgPropertySets, (IUnknown**)&m_pIRowset, NULL, NULL, NULL);

	// Loop thru all of the properties
	for(ULONG ulIndex=0; ulIndex < cPropertySets; ulIndex++)
	{
		for(ULONG ulIndex1=0; ulIndex1 < rgPropertySets[ulIndex].cProperties; ulIndex1++)
		{
			// Check to see if the property is supported
			if(!SupportedProperty(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
											DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown))
			{
				COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_NOTSUPPORTED);
				if(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwOptions	== DBPROPOPTIONS_REQUIRED)
					ulFailed++;
			}
			else if( (!SettableProperty(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
												DBPROPSET_ROWSET,m_pThisTestModule->m_pIUnknown)) )
			{
				// Call IOpenRowset to return a Rowset
				if(FAILED(m_pTableDel->CreateRowset(USE_OPENROWSET, IID_IRowsetInfo, 0, NULL, 
												(IUnknown**)&m_pIRowsetInfo, NULL, NULL, NULL)))
					goto CLEANUP;

				// Check the BOOL Properties
				if( (rgPropertySets[ulIndex].rgProperties[ulIndex1].vValue.vt == VT_BOOL) && 
					(!GetProperty(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
								DBPROPSET_ROWSET, m_pIRowsetInfo, 
								V_BOOL(&(rgPropertySets[ulIndex].rgProperties[ulIndex1].vValue)))) )
				{
					COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_NOTSETTABLE);
					if(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwOptions	== DBPROPOPTIONS_REQUIRED)
						ulFailed++;
				}
				else if( (GetProperty(	rgPropertySets[ulIndex].rgProperties[ulIndex1].dwPropertyID,
										DBPROPSET_ROWSET, 
										m_pIRowsetInfo, 
										&ulValue)) && 
						 (ulValue != V_UI4(&(rgPropertySets[ulIndex].rgProperties[ulIndex1].vValue))))
				{
					COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_NOTSETTABLE);
					if(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwOptions	== DBPROPOPTIONS_REQUIRED)
						ulFailed++;
				}
				else
				{
					COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_OK);
					ulPassed++;
				}
				
				SAFE_RELEASE(m_pIRowsetInfo);
			}
			else
			{
				COMPARE(rgPropertySets[ulIndex].rgProperties[ulIndex1].dwStatus, DBPROPSTATUS_OK);
				ulPassed++;
			}
		}
	}

	// Check the HResult returned
	(!ulFailed) ? CHECK(hr, S_OK) : ((ulFailed) ? CHECK(hr, DB_E_ERRORSOCCURRED)
												: CHECK(hr, DB_S_ERRORSOCCURRED));
	// Check the Rowset pointer on Failure
	if(FAILED(hr)) {
		COMPARE(m_pIRowset, NULL);
		goto CLEANUP;
	}

	// Get a set of HROWS
	if(FAILED(hr=m_pIRowset->GetNextRows(NULL,0,MAXROWS,
												&m_cRowsObtained, &m_rghRows)))
		goto CLEANUP;

	// QI for IRowsetChange
	if(!VerifyInterface(m_pIRowset,IID_IRowsetChange,ROWSET_INTERFACE,
											(IUnknown**)&m_pIRowsetChange)) {
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

	// QI for IRowsetUpdate
	if((Update) && (!VerifyInterface(m_pIRowsetChange,IID_IRowsetUpdate,
							ROWSET_INTERFACE,(IUnknown**)&m_pIRowsetUpdate))) {
		hr = E_NOINTERFACE;
		goto CLEANUP;
	}

CLEANUP:
	
	SAFE_RELEASE(m_pIRowsetInfo);
	return hr;
}

//--------------------------------------------------------------------
// @mfunc Cleanup IRowsetChange Pointer with specified Properties.
// IRowsetChange in m_pIRowsetChange on a valid rowset object. 

HRESULT	TCIERROR::CleanupIRowsetChange()
{	
	//initialization
	m_hr	=	S_OK;

	// IRowsetUpdate::Update Pending Delete's
	if(m_pIRowsetUpdate)
		CHECK(m_hr=m_pIRowsetUpdate->Update(NULL,0,NULL,NULL,NULL,NULL), S_OK);

	// Release Row Handles
	if(m_pIRowset && m_rghRows)
		CHECK(m_hr=m_pIRowset->ReleaseRows(m_cRowsObtained,m_rghRows,
												NULL,NULL,NULL),S_OK);
	// Release pIRowsetUpdate
	SAFE_RELEASE(m_pIRowsetUpdate);
	SAFE_RELEASE(m_pIRowsetChange);
	SAFE_RELEASE(m_pIRowset);

	// Zero out cRows and free the memory
	m_cRowsObtained = 0;
	PROVIDER_FREE(m_rghRows);

	return m_hr;
}

