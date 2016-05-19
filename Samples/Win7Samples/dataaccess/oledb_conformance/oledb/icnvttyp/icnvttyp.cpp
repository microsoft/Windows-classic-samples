//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module ICNVTTYP.CPP | IConvertType source file for all test modules.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "ICnvtTyp.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xc1d3c4a0, 0xf1ad, 0x11cf, { 0x83, 0x24, 0x00, 0xa0, 0xc9, 0x0d, 0x80, 0x4c }};
DECLARE_MODULE_NAME("IConvertType");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test module for IConvertType Interface.");
DECLARE_MODULE_VERSION(795921705);
// TCW_WizardVersion(2)
// TCW_Automation(True)
// }} TCW_MODULE_GLOBALS_END

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
		return TRUE;

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
	// Free the interface we got in ModuleCreateDBSession()
	return ModuleReleaseDBSession(pThisTestModule);
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class TCIDBCRSES Base Class for IDBCreateSession:CreateSession Testcases
class TCIConvertType : public CRowsetObject
{
	public:
		// @cmember Constructor
		TCIConvertType(LPWSTR wstrTestCaseName) : CRowsetObject(wstrTestCaseName)
		{
			m_pIRow					= NULL;
			m_pIConvertType			= NULL;
			m_pICommandWithParams	= NULL;
			m_fRowsetCnvtOnCmd		= FALSE;
		};

		// @cmember Destructor
		virtual ~TCIConvertType(){};

	protected:
		// @cmember Pointer to IConvertType interface.
		IConvertType *	m_pIConvertType;
		// @cmember Pointer to IRow interface.
		IRow *			m_pIRow;
		// @cmember Pointer to ICommandWithParameters.
		ICommandWithParameters * m_pICommandWithParams;

		// @cmember Rowset conversions are supported on the command.
		BOOL			m_fRowsetCnvtOnCmd;
		// @cmember HResult.
		HRESULT			m_Exphr;
};


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCCommandCanConvert)
//--------------------------------------------------------------------
// @class Gives information on the availability of type conversions on a command or on a rowset.
//
class TCCommandCanConvert : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCommandCanConvert,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid case.
	int Variation_1();
	// @cmember Invalid. Desired conversion is not valid.
	int Variation_2();
	// @cmember wfromDBType or wtoDBType did not refer to valid OLEDB types.
	int Variation_3();
	// @cmember dwConvert flags were invalid.
	int Variation_4();
	// @cmember DB_E_BADCONVERTFLAG condition 2.
	int Variation_5();
	// @cmember Valid Command Rowset case.
	int Variation_6();
	// @cmember Valid DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_7();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
	int Variation_8();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
	int Variation_9();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
	int Variation_10();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
	int Variation_11();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
	int Variation_12();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
	int Variation_13();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
	int Variation_14();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
	int Variation_15();
	// @cmember Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_16();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_17();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
	int Variation_18();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
	int Variation_19();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
	int Variation_20();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
	int Variation_21();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
	int Variation_22();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
	int Variation_23();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
	int Variation_24();
	// @cmember Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_25();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_26();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
	int Variation_27();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_28();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
	int Variation_29();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
	int Variation_30();
	// @cmember Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
	int Variation_31();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCCommandCanConvert)
#define THE_CLASS TCCommandCanConvert
BEG_TEST_CASE(TCCommandCanConvert, TCIConvertType, L"Gives information on the availability of type conversions on a command or on a rowset.")
	TEST_VARIATION(1, 		L"Valid case.")
	TEST_VARIATION(2, 		L"Invalid. Desired conversion is not valid.")
	TEST_VARIATION(3, 		L"wfromDBType or wtoDBType did not refer to valid OLEDB types.")
	TEST_VARIATION(4, 		L"dwConvert flags were invalid.")
	TEST_VARIATION(5, 		L"DB_E_BADCONVERTFLAG condition 2.")
	TEST_VARIATION(6, 		L"Valid Command Rowset case.")
	TEST_VARIATION(7, 		L"Valid DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(8, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.")
	TEST_VARIATION(9, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.")
	TEST_VARIATION(10, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.")
	TEST_VARIATION(11, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.")
	TEST_VARIATION(12, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.")
	TEST_VARIATION(13, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.")
	TEST_VARIATION(14, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.")
	TEST_VARIATION(15, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.")
	TEST_VARIATION(16, 		L"Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(17, 		L"Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(18, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.")
	TEST_VARIATION(19, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.")
	TEST_VARIATION(20, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.")
	TEST_VARIATION(21, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.")
	TEST_VARIATION(22, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.")
	TEST_VARIATION(23, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.")
	TEST_VARIATION(24, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.")
	TEST_VARIATION(25, 		L"Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(26, 		L"Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(27, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.")
	TEST_VARIATION(28, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(29, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.")
	TEST_VARIATION(30, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.")
	TEST_VARIATION(31, 		L"Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExecuteRowsetCanConvert)
//--------------------------------------------------------------------
// @class Gives information on the availability of type conversions on a command or on a rowset.
//
class TCExecuteRowsetCanConvert : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExecuteRowsetCanConvert,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid case.
	int Variation_1();
	// @cmember Invalid.  Desired conversion is not valid.
	int Variation_2();
	// @cmember wfromDBType or wtoDBType did not refer to valid OLEDB types.
	int Variation_3();
	// @cmember dwConvert flags were invalid.
	int Variation_4();
	// @cmember DB_E_BADCONVERTFLAG condition 3.
	int Variation_5();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
	int Variation_6();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
	int Variation_7();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
	int Variation_8();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
	int Variation_9();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
	int Variation_10();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
	int Variation_11();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
	int Variation_12();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
	int Variation_13();
	// @cmember Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_14();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_15();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
	int Variation_16();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_17();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
	int Variation_18();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
	int Variation_19();
	// @cmember Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
	int Variation_20();
	// @cmember Valid IGetRow::GetRowFromHRow off a valid rowset.
	int Variation_21();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCExecuteRowsetCanConvert)
#define THE_CLASS TCExecuteRowsetCanConvert
BEG_TEST_CASE(TCExecuteRowsetCanConvert, TCIConvertType, L"Gives information on the availability of type conversions on a command or on a rowset.")
	TEST_VARIATION(1, 		L"Valid case.")
	TEST_VARIATION(2, 		L"Invalid.  Desired conversion is not valid.")
	TEST_VARIATION(3, 		L"wfromDBType or wtoDBType did not refer to valid OLEDB types.")
	TEST_VARIATION(4, 		L"dwConvert flags were invalid.")
	TEST_VARIATION(5, 		L"DB_E_BADCONVERTFLAG condition 3.")
	TEST_VARIATION(6, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.")
	TEST_VARIATION(7, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.")
	TEST_VARIATION(8, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.")
	TEST_VARIATION(9, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.")
	TEST_VARIATION(10, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.")
	TEST_VARIATION(11, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.")
	TEST_VARIATION(12, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.")
	TEST_VARIATION(13, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.")
	TEST_VARIATION(14, 		L"Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(15, 		L"Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(16, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.")
	TEST_VARIATION(17, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(18, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.")
	TEST_VARIATION(19, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.")
	TEST_VARIATION(20, 		L"Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.")
	TEST_VARIATION(21, 		L"Valid IGetRow::GetRowFromHRow off a valid rowset.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCOpenRowsetCanConvert)
//--------------------------------------------------------------------
// @class Gives information on the availability of type conversions on a command or on a rowset.
//
class TCOpenRowsetCanConvert : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOpenRowsetCanConvert,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid case.
	int Variation_1();
	// @cmember Invalid.  Desired conversion is not valid.
	int Variation_2();
	// @cmember wfromDBType or wtoDBType did not refer to valid OLEDB types.
	int Variation_3();
	// @cmember dwConvert flags were invalid.
	int Variation_4();
	// @cmember DB_E_BADCONVERTFLAG condition 3.
	int Variation_5();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
	int Variation_6();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
	int Variation_7();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
	int Variation_8();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
	int Variation_9();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
	int Variation_10();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
	int Variation_11();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
	int Variation_12();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
	int Variation_13();
	// @cmember Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_14();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_15();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
	int Variation_16();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_17();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
	int Variation_18();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
	int Variation_19();
	// @cmember Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
	int Variation_20();
	// @cmember Valid IGetRow::GetRowFromHRow off a valid rowset.
	int Variation_21();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCOpenRowsetCanConvert)
#define THE_CLASS TCOpenRowsetCanConvert
BEG_TEST_CASE(TCOpenRowsetCanConvert, TCIConvertType, L"Gives information on the availability of type conversions on a command or on a rowset.")
	TEST_VARIATION(1, 		L"Valid case.")
	TEST_VARIATION(2, 		L"Invalid.  Desired conversion is not valid.")
	TEST_VARIATION(3, 		L"wfromDBType or wtoDBType did not refer to valid OLEDB types.")
	TEST_VARIATION(4, 		L"dwConvert flags were invalid.")
	TEST_VARIATION(5, 		L"DB_E_BADCONVERTFLAG condition 3.")
	TEST_VARIATION(6, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.")
	TEST_VARIATION(7, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.")
	TEST_VARIATION(8, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.")
	TEST_VARIATION(9, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.")
	TEST_VARIATION(10, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.")
	TEST_VARIATION(11, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.")
	TEST_VARIATION(12, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.")
	TEST_VARIATION(13, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.")
	TEST_VARIATION(14, 		L"Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(15, 		L"Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(16, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.")
	TEST_VARIATION(17, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(18, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.")
	TEST_VARIATION(19, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.")
	TEST_VARIATION(20, 		L"Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.")
	TEST_VARIATION(21, 		L"Valid IGetRow::GetRowFromHRow off a valid rowset.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExecuteIRowCanConvert)
//--------------------------------------------------------------------
// @class Gives information on the availability of type conversions on a Row object.
//
class TCExecuteIRowCanConvert : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExecuteIRowCanConvert,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid case.
	int Variation_1();
	// @cmember Invalid.  Desired conversion is not valid.
	int Variation_2();
	// @cmember wfromDBType or wtoDBType did not refer to valid OLEDB types.
	int Variation_3();
	// @cmember dwConvert flags were invalid.
	int Variation_4();
	// @cmember DB_E_BADCONVERTFLAG condition 3.
	int Variation_5();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
	int Variation_6();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
	int Variation_7();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
	int Variation_8();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
	int Variation_9();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
	int Variation_10();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
	int Variation_11();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
	int Variation_12();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
	int Variation_13();
	// @cmember Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_14();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_15();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
	int Variation_16();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_17();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
	int Variation_18();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
	int Variation_19();
	// @cmember Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
	int Variation_20();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCExecuteIRowCanConvert)
#define THE_CLASS TCExecuteIRowCanConvert
BEG_TEST_CASE(TCExecuteIRowCanConvert, TCIConvertType, L"Gives information on the availability of type conversions on a Row object.")
	TEST_VARIATION(1, 		L"Valid case.")
	TEST_VARIATION(2, 		L"Invalid.  Desired conversion is not valid.")
	TEST_VARIATION(3, 		L"wfromDBType or wtoDBType did not refer to valid OLEDB types.")
	TEST_VARIATION(4, 		L"dwConvert flags were invalid.")
	TEST_VARIATION(5, 		L"DB_E_BADCONVERTFLAG condition 3.")
	TEST_VARIATION(6, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.")
	TEST_VARIATION(7, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.")
	TEST_VARIATION(8, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.")
	TEST_VARIATION(9, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.")
	TEST_VARIATION(10, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.")
	TEST_VARIATION(11, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.")
	TEST_VARIATION(12, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.")
	TEST_VARIATION(13, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.")
	TEST_VARIATION(14, 		L"Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(15, 		L"Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(16, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.")
	TEST_VARIATION(17, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(18, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.")
	TEST_VARIATION(19, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.")
	TEST_VARIATION(20, 		L"Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCOpenIRowCanConvert)
//--------------------------------------------------------------------
// @class Gives information on the availability of type conversions on a IRow object.
//
class TCOpenIRowCanConvert : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOpenIRowCanConvert,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid case.
	int Variation_1();
	// @cmember Invalid.  Desired conversion is not valid.
	int Variation_2();
	// @cmember wfromDBType or wtoDBType did not refer to valid OLEDB types.
	int Variation_3();
	// @cmember dwConvert flags were invalid.
	int Variation_4();
	// @cmember DB_E_BADCONVERTFLAG condition 3.
	int Variation_5();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
	int Variation_6();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
	int Variation_7();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
	int Variation_8();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
	int Variation_9();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
	int Variation_10();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
	int Variation_11();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
	int Variation_12();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
	int Variation_13();
	// @cmember Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
	int Variation_14();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_15();
	// @cmember Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
	int Variation_16();
	// @cmember Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
	int Variation_17();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
	int Variation_18();
	// @cmember Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
	int Variation_19();
	// @cmember Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
	int Variation_20();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCOpenIRowCanConvert)
#define THE_CLASS TCOpenIRowCanConvert
BEG_TEST_CASE(TCOpenIRowCanConvert, TCIConvertType, L"Gives information on the availability of type conversions on a IRow object.")
	TEST_VARIATION(1, 		L"Valid case.")
	TEST_VARIATION(2, 		L"Invalid.  Desired conversion is not valid.")
	TEST_VARIATION(3, 		L"wfromDBType or wtoDBType did not refer to valid OLEDB types.")
	TEST_VARIATION(4, 		L"dwConvert flags were invalid.")
	TEST_VARIATION(5, 		L"DB_E_BADCONVERTFLAG condition 3.")
	TEST_VARIATION(6, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.")
	TEST_VARIATION(7, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.")
	TEST_VARIATION(8, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.")
	TEST_VARIATION(9, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.")
	TEST_VARIATION(10, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.")
	TEST_VARIATION(11, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.")
	TEST_VARIATION(12, 		L"Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.")
	TEST_VARIATION(13, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.")
	TEST_VARIATION(14, 		L"Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.")
	TEST_VARIATION(15, 		L"Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(16, 		L"Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.")
	TEST_VARIATION(17, 		L"Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.")
	TEST_VARIATION(18, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.")
	TEST_VARIATION(19, 		L"Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.")
	TEST_VARIATION(20, 		L"Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCDataConvert)
//--------------------------------------------------------------------
// @class Test case to test API level testing for DATA convert interface.
//
class TCDataConvert : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	IDataConvert *	m_pIDataConvert;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCDataConvert,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Source or Destination DBTYPE is invalid. returns DB_E_BADBINDINFO.
	int Variation_1();
	// @cmember pSrc is a NULL pointer, returns E_FAIL
	int Variation_2();
	// @cmember Requested conversion resulted in an overflow DB_E_OVERFLOW
	int Variation_3();
	// @cmember DB_E_ERRORSOCCURRED cases.
	int Variation_4();
	// @cmember Unsupported conversion DB_E_UNSUPPORTEDCONVERSION.
	int Variation_5();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCDataConvert)
#define THE_CLASS TCDataConvert
BEG_TEST_CASE(TCDataConvert, TCIConvertType, L"Test case to test API level testing for DATA convert interface.")
	TEST_VARIATION(1, 		L"Source or Destination DBTYPE is invalid. returns DB_E_BADBINDINFO.")
	TEST_VARIATION(2, 		L"pSrc is a NULL pointer, returns E_FAIL")
	TEST_VARIATION(3, 		L"Requested conversion resulted in an overflow DB_E_OVERFLOW")
	TEST_VARIATION(4, 		L"DB_E_ERRORSOCCURRED cases.")
	TEST_VARIATION(5, 		L"Unsupported conversion DB_E_UNSUPPORTEDCONVERSION.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCCZombie)
//--------------------------------------------------------------------
// @class Induce zombie states on the Command
//
class TCCZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	// @cmember TestTxn
	int TestTxn(ETXN eTxn, BOOL fRetaining);

	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE
	int Variation_2();
	// @cmember S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE
	int Variation_3();
	// @cmember S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCCZombie)
#define THE_CLASS TCCZombie
BEG_TEST_CASE(TCCZombie, CTransaction, L"Induce zombie states on the Command")
	TEST_VARIATION(1, 		L"S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE")
	TEST_VARIATION(3, 		L"S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE")
	TEST_VARIATION(4, 		L"S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCRZombie)
//--------------------------------------------------------------------
// @class Induce zombie states on the Rowset
//
class TCRZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCRZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	// @cmember TestTxn
	int TestTxn(ETXN eTxn, BOOL fRetaining);

	// {{ TCW_TESTVARS()
	// @cmember S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE
	int Variation_1();
	// @cmember S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE
	int Variation_2();
	// @cmember S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE
	int Variation_3();
	// @cmember S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCRZombie)
#define THE_CLASS TCRZombie
BEG_TEST_CASE(TCRZombie, CTransaction, L"Induce zombie states on the Rowset")
	TEST_VARIATION(1, 		L"S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE")
	TEST_VARIATION(2, 		L"S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE")
	TEST_VARIATION(3, 		L"S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE")
	TEST_VARIATION(4, 		L"S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public TCIConvertType { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,TCIConvertType);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IConvertType calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IConvertType calls with previous error object existing
	int Variation_2();
	// @cmember Invalid IConvertType calls with no previous error object existing
	int Variation_3();
	// }} TCW_TESTVARS_END
};

// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, TCIConvertType, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IConvertType calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IConvertType calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid IConvertType calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(7, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCCommandCanConvert)
	TEST_CASE(2, TCExecuteRowsetCanConvert)
	TEST_CASE(3, TCOpenRowsetCanConvert)
	TEST_CASE(4, TCExecuteIRowCanConvert)
	TEST_CASE(5, TCOpenIRowCanConvert)
	TEST_CASE(6, TCDataConvert)
	TEST_CASE(7, TCCZombie)
	TEST_CASE(8, TCRZombie)
	TEST_CASE(9, TCExtendedErrors)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCCommandCanConvert)
//*-----------------------------------------------------------------------
//| Test Case:		TCCommandCanConvert - Gives information on the availability of type conversions on a command or on a rowset.
//|	Created:		08/08/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommandCanConvert::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Set the current Session Pointer
		SetDBSession(m_pThisTestModule->m_pIUnknown2);

		// Check to see if Commands are supported
		if( !m_pIDBCreateCommand ) {
			odtLog << L"Commands not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}

		// Check to see if the conversion is supported on the command
		if( GetProperty(DBPROP_ROWSETCONVERSIONSONCOMMAND, 
						DBPROPSET_DATASOURCEINFO, m_pThisTestModule->m_pIUnknown) )
			m_fRowsetCnvtOnCmd = TRUE;

		// Get a Command Object.
		TESTC_(CreateCommandObject(), S_OK);

		// Get an ICommandWithParameters pointer
		VerifyInterface(m_pICommand, IID_ICommandWithParameters,
							 COMMAND_INTERFACE, (IUnknown **)&m_pICommandWithParams);

		// Verify and Create the Interface pointer for IConvertType.
		TESTC(VerifyInterface(m_pICommand, IID_IConvertType,
							COMMAND_INTERFACE,(IUnknown **)&m_pIConvertType));

		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_1()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, DBCONVERTFLAGS_PARAMETER), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid. Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_2()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_FALSE;
	
	// If RowsetConversions are supported
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
						DBTYPE_DBTIME, DBCONVERTFLAGS_COLUMN), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc wfromDBType or wtoDBType did not refer to valid OLEDB types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_3()
{ 
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_FALSE;
	
	// If RowsetConversions are supported
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;

	// Call Column with invalid types
	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), m_Exphr);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), m_Exphr);

	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						DBTYPE_I8, DBCONVERTFLAGS_COLUMN), m_Exphr);

	// Initialize the returncode
	m_Exphr = S_FALSE;
	
	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;

	// Call Parameter with invalid types
	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						USHRT_MAX, DBCONVERTFLAGS_PARAMETER), m_Exphr);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						USHRT_MAX, DBCONVERTFLAGS_PARAMETER), m_Exphr);

	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						DBTYPE_I8, DBCONVERTFLAGS_PARAMETER), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwConvert flags were invalid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_4()
{
	TBEGIN;

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, USHRT_MAX), DB_E_BADCONVERTFLAG);

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
								DBTYPE_I8, 16), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCONVERTFLAG condition 2.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_5()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If RowsetConversions are supported
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Valid Command Rowset case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_6()
{
	TBEGIN;

	IConvertType *	pIConvertType = NULL;
	
	// Create the rowset object.
	TESTC_(CreateRowsetObject(SELECT_ALLFROMTBL), S_OK);

	// Get an IRowset Interface and an IConvertType Interface
	TESTC(VerifyInterface(m_pIAccessor, IID_IConvertType,
						 ROWSET_INTERFACE, (IUnknown **)&pIConvertType));

	// Check for invalid dwConvert Flag
	TESTC_(pIConvertType->CanConvert(DBTYPE_WSTR, 
							DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);
	
CLEANUP:

	// Cleanup the Rowset Objects created
	ReleaseRowsetObject();
	SAFE_RELEASE(pIConvertType);

	// Clean up the Table
	if( m_pTable )
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_7()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISFIXEDLENGTH),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_8()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_FALSE;
	
	// If Parameters are supported
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_DBTIME, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISFIXEDLENGTH), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_9()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_10()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
				DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_11()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_12()
{
	TBEGIN;

	// If Parameters are supported
	if( !m_pICommandWithParams )
	{
		TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);
	}
	else
	{
		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);
	}

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_13()
{
	TBEGIN;

	// If Parameters are supported
	if( m_pICommandWithParams )
	{
		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
				DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);

		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
				DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);

		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
				DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);
	}
	else
	{
		TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
				DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

		TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
				DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

		TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
				DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);
	}

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_14()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_BYREF,
			DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_ARRAY,
			DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_VECTOR,
			DBTYPE_WSTR,DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_15()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_16()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Parameters are supported
	if( !m_pICommandWithParams )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_17()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_18()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_19()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
				DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_20()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_21()
{
	TBEGIN;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
	{
		TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);
	}
	else
	{
		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);
	}

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_22()
{
	TBEGIN;

	// If Rowset Conversions are supported on the command
	if( m_fRowsetCnvtOnCmd )
	{
		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
				DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);

		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
				DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);

		TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
				DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), S_OK, S_FALSE);
	}
	else
	{
		TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
				DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

		TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
				DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

		TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
				DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);
	}

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_23()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_BYREF,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_ARRAY,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_VECTOR,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_24()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_25()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH),m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_26()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_27()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	// DBCONVERTFLAGS_ISLONG with the Default of COLUMN
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR, 
									DBCONVERTFLAGS_ISLONG), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_28()
{
	TBEGIN;

	// Initialize the returncode
	m_Exphr = S_OK;

	// If Rowset Conversions are supported on the command
	if( !m_fRowsetCnvtOnCmd )
		m_Exphr = DB_E_BADCONVERTFLAG;
	
	// DBCONVERTFLAGS_ISFIXEDLENGTH with the Default of COLUMN
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
								DBCONVERTFLAGS_ISFIXEDLENGTH), m_Exphr);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_29()
{
	TBEGIN;

	// BSTR is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	// BSTR is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_30()
{
	TBEGIN;

	// IUNKNOWN is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
			DBCONVERTFLAGS_PARAMETER | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	// IUNKNOWN is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCommandCanConvert::Variation_31()
{
	TBEGIN;

	// DBTYPE_EMPTY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_EMPTY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_NULL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NULL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I2, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R8, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_CY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_CY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DATE is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DATE, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BSTR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IDISPATCH is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IDISPATCH, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ERROR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ERROR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BOOL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BOOL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VARIANT is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARIANT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IUNKNOWN is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI1, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DECIMAL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DECIMAL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ARRAY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ARRAY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYREF is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BYREF, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I1, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI2, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI4, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_GUID is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_GUID, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VECTOR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VECTOR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_FILETIME is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_FILETIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_RESERVED is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_RESERVED, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYTES is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_STR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_WSTR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_NUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_UDT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_UDT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBDATE is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBDATE, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIME is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIMESTAMP is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIMESTAMP, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_HCHAPTER is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_HCHAPTER, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_PROPVARIANT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_PROPVARIANT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_VARNUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// VT_INT is a valid Variant type
	TESTC_(m_pIConvertType->CanConvert(VT_INT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// VT_RECORD is a valid Variant type new in VC 6.
	TESTC_(m_pIConvertType->CanConvert(36, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// 15 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(15, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 32 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(32, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 37 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(37, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// -1 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(-1, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCommandCanConvert::Terminate()
{
	// Release all the objects created
	SAFE_RELEASE(m_pIConvertType);
	SAFE_RELEASE(m_pICommandWithParams);

	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExecuteRowsetCanConvert)
//*-----------------------------------------------------------------------
//| Test Case:		TCExecuteRowsetCanConvert - Gives information on the availability of type conversions on a command or on a rowset.
//|	Created:		08/08/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExecuteRowsetCanConvert::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Set the current Session Pointer
		SetDBSession(m_pThisTestModule->m_pIUnknown2);

		// Check to see if Commands are supported
		if(!m_pIDBCreateCommand) {
			odtLog << L"Commands not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}

		// Create the rowset object.
		TESTC_(CreateRowsetObject(SELECT_ALLFROMTBL),S_OK);

		// Verify and Create the Interface pointer for IConvertType.
		TESTC(VerifyInterface(m_pIAccessor, IID_IConvertType,
						ROWSET_INTERFACE,(IUnknown **)&m_pIConvertType));

		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_1()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_2()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
						DBTYPE_DBTIME, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc wfromDBType or wtoDBType did not refer to valid OLEDB types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_3()
{ 
	TBEGIN;

	// Call with invalid types
	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						DBTYPE_I8, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwConvert flags were invalid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_4()
{
	TBEGIN;

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, USHRT_MAX), DB_E_BADCONVERTFLAG);

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, 16), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCONVERTFLAG condition 3.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_5()
{
	TBEGIN;

	// Check to see if we get an Error
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
					DBTYPE_NUMERIC, DBCONVERTFLAGS_PARAMETER), DB_E_BADCONVERTFLAG);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_6()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_DBTIME, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISFIXEDLENGTH), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_7()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_8()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_9()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_10()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_11()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_12()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_BYREF,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_ARRAY,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_VECTOR,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_13()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_14()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_15()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_16()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISLONG with the Default of COLUMN
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR, 
											DBCONVERTFLAGS_ISLONG), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_17()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISFIXEDLENGTH with the Default of COLUMN
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
								DBCONVERTFLAGS_ISFIXEDLENGTH), S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_18()
{
	TBEGIN;

	// BSTR is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_19()
{
	TBEGIN;

	// IUNKNOWN is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_20()
{
	TBEGIN;

	// DBTYPE_EMPTY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_EMPTY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_NULL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NULL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I2, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R8, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_CY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_CY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DATE is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DATE, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BSTR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IDISPATCH is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IDISPATCH, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ERROR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ERROR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BOOL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BOOL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VARIANT is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARIANT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IUNKNOWN is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI1, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DECIMAL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DECIMAL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ARRAY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ARRAY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYREF is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BYREF, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I1, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI2, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI4, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_GUID is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_GUID, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VECTOR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VECTOR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_FILETIME is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_FILETIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_RESERVED is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_RESERVED, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYTES is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_STR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_WSTR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_NUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_UDT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_UDT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBDATE is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBDATE, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIME is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIMESTAMP is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIMESTAMP, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_HCHAPTER is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_HCHAPTER, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_PROPVARIANT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_PROPVARIANT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_VARNUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// VT_INT is a valid Variant type
	TESTC_(m_pIConvertType->CanConvert(VT_INT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// VT_RECORD is a valid Variant type new in VC 6.
	TESTC_(m_pIConvertType->CanConvert(36, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// 15 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(15, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 32 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(32, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 37 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(37, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// -1 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(-1, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Valid IGetRow::GetRowFromHRow off a valid rowset.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteRowsetCanConvert::Variation_21()
{
	TBEGIN;

	DBCOUNTITEM	ulIndex			= 0;
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW*		rghRows			= NULL;
	BOOL		fSuccess		= FALSE;

	IGetRow	*		pIGetRow		= NULL;
	IRowset *		pIRowset		= NULL;
	IConvertType *	pIConvertType	= NULL;

	// Verify and Create the Interface pointer for IRowset.
	if( !VerifyInterface(m_pIAccessor, IID_IGetRow,
							ROW_INTERFACE,(IUnknown **)&pIGetRow))
	{
		odtLog << L"Row objects not supported by Provider." << ENDL;
		return TEST_SKIPPED;
	}

	// Verify and Create the Interface pointer for IRowset.
	TESTC(VerifyInterface(m_pIAccessor, IID_IRowset,
						ROWSET_INTERFACE,(IUnknown **)&pIRowset));

	// Get each row in the rowset
	for(ulIndex=1; ulIndex<=m_pTable->GetRowsOnCTable(); ulIndex++)	
	{
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//The following tests the GetRowFromHROW method.
		TESTC_(pIGetRow->GetRowFromHROW(NULL, rghRows[0], IID_IConvertType, (IUnknown**)&pIConvertType), S_OK);
		TESTC_(pIConvertType->CanConvert(DBTYPE_WSTR,DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);
		TESTC_(pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);

		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_RELEASE(pIConvertType);
	}

	// Make sure it did all of the columns
	TESTC(ulIndex > m_pTable->GetRowsOnCTable());

CLEANUP:
	
	// Cleanup
	pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
	PROVIDER_FREE(rghRows);
	
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGetRow);
	SAFE_RELEASE(pIConvertType);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExecuteRowsetCanConvert::Terminate()
{
	// Release all the objects created
	SAFE_RELEASE(m_pIConvertType);

	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Clean up the Table
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCOpenRowsetCanConvert)
//*-----------------------------------------------------------------------
//| Test Case:		TCOpenRowsetCanConvert - Gives information on the availability of type conversions on a command or on a rowset.
//|	Created:		08/08/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowsetCanConvert::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Create the rowset object.
		TESTC_(CreateRowsetObject(USE_OPENROWSET),S_OK);

		// Verify and Create the Interface pointer for IConvertType.
		TESTC(VerifyInterface(m_pIAccessor, IID_IConvertType,
						ROWSET_INTERFACE,(IUnknown **)&m_pIConvertType));

		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_1()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_2()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
						DBTYPE_DBTIME, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc wfromDBType or wtoDBType did not refer to valid OLEDB types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_3()
{ 
	TBEGIN;

	// Call with invalid types
	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						DBTYPE_I8, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwConvert flags were invalid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_4()
{
	TBEGIN;

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, USHRT_MAX), DB_E_BADCONVERTFLAG);

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, 16), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCONVERTFLAG condition 3.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_5()
{
	TBEGIN;

	// Check to see if we get an Error
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
					DBTYPE_NUMERIC, DBCONVERTFLAGS_PARAMETER), DB_E_BADCONVERTFLAG);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_6()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_DBTIME, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISFIXEDLENGTH), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_7()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_8()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_9()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_10()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_11()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_12()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_BYREF,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_ARRAY,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_VECTOR,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_13()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_14()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_15()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_16()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISLONG with the Default of COLUMN
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR, 
											DBCONVERTFLAGS_ISLONG), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_17()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISFIXEDLENGTH with the Default of COLUMN
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
								DBCONVERTFLAGS_ISFIXEDLENGTH), S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_18()
{
	TBEGIN;

	// BSTR is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_19()
{
	TBEGIN;

	// IUNKNOWN is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_20()
{
	TBEGIN;

	// DBTYPE_EMPTY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_EMPTY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_NULL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NULL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I2, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R8, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_CY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_CY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DATE is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DATE, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BSTR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IDISPATCH is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IDISPATCH, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ERROR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ERROR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BOOL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BOOL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VARIANT is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARIANT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IUNKNOWN is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI1, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DECIMAL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DECIMAL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ARRAY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ARRAY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYREF is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BYREF, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I1, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI2, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI4, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_GUID is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_GUID, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VECTOR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VECTOR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_FILETIME is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_FILETIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_RESERVED is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_RESERVED, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYTES is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_STR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_WSTR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_NUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_UDT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_UDT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBDATE is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBDATE, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIME is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIMESTAMP is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIMESTAMP, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_HCHAPTER is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_HCHAPTER, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_PROPVARIANT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_PROPVARIANT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_VARNUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// VT_INT is a valid Variant type
	TESTC_(m_pIConvertType->CanConvert(VT_INT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// VT_RECORD is a valid Variant type new in VC 6.
	TESTC_(m_pIConvertType->CanConvert(36, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// 15 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(15, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 32 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(32, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 37 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(37, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// -1 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(-1, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Valid IGetRow::GetRowFromHRow off a valid rowset.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowsetCanConvert::Variation_21()
{
	TBEGIN;

	DBCOUNTITEM	ulIndex			= 0;
	DBCOUNTITEM	cRowsObtained	= 0;
	HROW*		rghRows			= NULL;
	BOOL		fSuccess		= FALSE;

	IGetRow	*		pIGetRow		= NULL;
	IRowset *		pIRowset		= NULL;
	IConvertType *	pIConvertType	= NULL;

	// Verify and Create the Interface pointer for IRowset.
	if( !VerifyInterface(m_pIAccessor, IID_IGetRow,
							ROW_INTERFACE,(IUnknown **)&pIGetRow))
	{
		odtLog << L"Row objects not supported by Provider." << ENDL;
		return TEST_SKIPPED;
	}

	// Verify and Create the Interface pointer for IRowset.
	TESTC(VerifyInterface(m_pIAccessor, IID_IRowset,
						ROWSET_INTERFACE,(IUnknown **)&pIRowset));

	// Get each row in the rowset
	for(ulIndex=1; ulIndex<=m_pTable->GetRowsOnCTable(); ulIndex++)	
	{
		TESTC_(pIRowset->GetNextRows(NULL, 0, 1, &cRowsObtained, &rghRows),S_OK);
		
		//The following tests the GetRowFromHROW method.
		TESTC_(pIGetRow->GetRowFromHROW(NULL, rghRows[0], IID_IConvertType, (IUnknown**)&pIConvertType), S_OK);
		TESTC_(pIConvertType->CanConvert(DBTYPE_WSTR,DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);
		TESTC_(pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL),S_OK);

		cRowsObtained = 0;
		PROVIDER_FREE(rghRows);
		SAFE_RELEASE(pIConvertType);
	}

	// Make sure it did all of the columns
	TESTC(ulIndex > m_pTable->GetRowsOnCTable());

CLEANUP:
	
	// Cleanup
	pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
	PROVIDER_FREE(rghRows);
	
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIGetRow);
	SAFE_RELEASE(pIConvertType);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowsetCanConvert::Terminate()
{
	// Release the interface.
	SAFE_RELEASE(m_pIConvertType);
	
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Clean up the Table
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExecuteIRowCanConvert)
//*-----------------------------------------------------------------------
//| Test Case:		TCExecuteIRowCanConvert - Gives information on the availability of type conversions on a IRow object.
//|	Created:		10/08/98
//|	Updated:		10/08/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExecuteIRowCanConvert::Init()
{
	ULONG_PTR	ulOleObj = 0;
	HRESULT		hr		 = E_FAIL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Set the current Session Pointer
		SetDBSession(m_pThisTestModule->m_pIUnknown2);

		// Check to see if Commands are supported
		if( !m_pIDBCreateCommand ) {
			odtLog << L"Commands not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}

		// Check to see if Row objects are supported
		GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, 
									m_pThisTestModule->m_pIUnknown, &ulOleObj);

		// Create a table
		m_pTable = new CTable(m_pIOpenRowset, m_pwszTestCaseName);
		TESTC(m_pTable != NULL);
		
		QTESTC_(m_pTable->CreateTable(NUM_ROWS),S_OK);

		// Create the rowset object.
		if( FAILED(hr=m_pTable->CreateRowset(SELECT_ALLFROMTBL, IID_IRow, 0, NULL, (IUnknown**)&m_pIRow)) )
		{
			// Check to see if the provider supports IRow
			if( hr == E_NOINTERFACE && 
				((ulOleObj & DBPROPVAL_OO_SINGLETON) != DBPROPVAL_OO_SINGLETON) ) {
				odtLog << L"Row objects not supported by Provider." << ENDL;
				return TEST_SKIPPED;
			}

			goto CLEANUP;
		}
		else
		{
			// Check the DBPROP_OLEOBJECT
			if( !(ulOleObj & DBPROPVAL_OO_SINGLETON) )
				TWARNING("DBPROP_OLEOBJECT should return DBPROPVAL_OO_SINGLETON.");
		}

		// Verify and Create the Interface pointer for IConvertType.
		TESTC(VerifyInterface(m_pIRow, IID_IConvertType,
						ROW_INTERFACE,(IUnknown **)&m_pIConvertType));

		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_1()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_2()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
						DBTYPE_DBTIME, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc wfromDBType or wtoDBType did not refer to valid OLEDB types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_3()
{ 
	TBEGIN;

	// Call with invalid types
	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						DBTYPE_I8, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwConvert flags were invalid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_4()
{
	TBEGIN;

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, USHRT_MAX), DB_E_BADCONVERTFLAG);

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, 16), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCONVERTFLAG condition 3.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_5()
{
	TBEGIN;

	// Check to see if we get an Error
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
					DBTYPE_NUMERIC, DBCONVERTFLAGS_PARAMETER), DB_E_BADCONVERTFLAG);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_6()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_DBTIME, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISFIXEDLENGTH), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_7()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_8()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_9()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_10()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_11()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_12()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_BYREF,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_ARRAY,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_VECTOR,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_13()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_14()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_15()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_16()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISLONG with the Default of COLUMN
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR, 
											DBCONVERTFLAGS_ISLONG), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_17()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISFIXEDLENGTH with the Default of COLUMN
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
								DBCONVERTFLAGS_ISFIXEDLENGTH), S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_18()
{
	TBEGIN;

	// BSTR is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_19()
{
	TBEGIN;

	// IUNKNOWN is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExecuteIRowCanConvert::Variation_20()
{
	TBEGIN;

	// DBTYPE_EMPTY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_EMPTY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_NULL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NULL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I2, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R8, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_CY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_CY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DATE is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DATE, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BSTR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IDISPATCH is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IDISPATCH, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ERROR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ERROR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BOOL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BOOL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VARIANT is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARIANT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IUNKNOWN is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI1, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DECIMAL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DECIMAL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ARRAY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ARRAY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYREF is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BYREF, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I1, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI2, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI4, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_GUID is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_GUID, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VECTOR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VECTOR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_FILETIME is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_FILETIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_RESERVED is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_RESERVED, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYTES is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_STR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_WSTR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_NUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_UDT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_UDT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBDATE is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBDATE, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIME is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIMESTAMP is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIMESTAMP, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_HCHAPTER is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_HCHAPTER, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_PROPVARIANT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_PROPVARIANT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_VARNUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// VT_INT is a valid Variant type
	TESTC_(m_pIConvertType->CanConvert(VT_INT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// VT_RECORD is a valid Variant type new in VC 6.
	TESTC_(m_pIConvertType->CanConvert(36, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// 15 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(15, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 32 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(32, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 37 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(37, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// -1 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(-1, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExecuteIRowCanConvert::Terminate()
{
	// Release all the objects created
	SAFE_RELEASE(m_pIConvertType);
	SAFE_RELEASE(m_pIRow);

	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Clean up the Table
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCOpenIRowCanConvert)
//*-----------------------------------------------------------------------
//| Test Case:		TCOpenIRowCanConvert - Gives information on the availability of type conversions on a IRow object.
//|	Created:		10/08/98
//|	Updated:		10/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenIRowCanConvert::Init()
{
	ULONG_PTR	ulOleObj = 0;
	HRESULT		hr		 = E_FAIL;

	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Set the current Session Pointer
		SetDBSession(m_pThisTestModule->m_pIUnknown2);

		// Check to see if Row objects are supported
		GetProperty(DBPROP_OLEOBJECTS, DBPROPSET_DATASOURCEINFO, 
									m_pThisTestModule->m_pIUnknown, &ulOleObj);

		// Create a table
		m_pTable = new CTable(m_pIOpenRowset, m_pwszTestCaseName);
		TESTC(m_pTable != NULL);
		
		QTESTC_(m_pTable->CreateTable(NUM_ROWS),S_OK);

		// Create the rowset object.
		if( FAILED(hr=m_pTable->CreateRowset(USE_OPENROWSET, IID_IRow, 0, NULL, (IUnknown**)&m_pIRow)) )
		{
			// Check to see if the provider supports IRow
			if( hr == E_NOINTERFACE && 
				((ulOleObj & DBPROPVAL_OO_SINGLETON) != DBPROPVAL_OO_SINGLETON) ) {
				odtLog << L"Row objects not supported by Provider." << ENDL;
				return TEST_SKIPPED;
			}

			goto CLEANUP;
		}
		else
		{
			// Check the DBPROP_OLEOBJECT
			if( !(ulOleObj & DBPROPVAL_OO_SINGLETON) )
				TWARNING("DBPROP_OLEOBJECT should return DBPROPVAL_OO_SINGLETON.");
		}

		// Verify and Create the Interface pointer for IConvertType.
		TESTC(VerifyInterface(m_pIRow, IID_IConvertType,
						ROW_INTERFACE,(IUnknown **)&m_pIConvertType));

		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_1()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_2()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
						DBTYPE_DBTIME, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc wfromDBType or wtoDBType did not refer to valid OLEDB types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_3()
{ 
	TBEGIN;

	// Call with invalid types
	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);

	TESTC_(m_pIConvertType->CanConvert(USHRT_MAX, 
						DBTYPE_I8, DBCONVERTFLAGS_COLUMN), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc dwConvert flags were invalid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_4()
{
	TBEGIN;

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, USHRT_MAX), DB_E_BADCONVERTFLAG);

	// Check for invalid dwConvert Flag
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I8, 
						DBTYPE_I8, 16), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DB_E_BADCONVERTFLAG condition 3.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_5()
{
	TBEGIN;

	// Check to see if we get an Error
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, 
					DBTYPE_NUMERIC, DBCONVERTFLAGS_PARAMETER), DB_E_BADCONVERTFLAG);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH.  Desired conversion is not valid.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_6()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_DBTIME, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISFIXEDLENGTH), S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_WSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_7()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_STR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_8()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BYTES case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_9()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
					DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_VARNUMERIC case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_10()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
						DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_11()
{
	TBEGIN;

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_BYREF,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_ARRAY,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_WSTR | DBTYPE_VECTOR,
			DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with BYREF, ARRAY, and VECTOR on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_12()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_BYREF,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_ARRAY,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_I4 | DBTYPE_VECTOR,
			DBTYPE_WSTR,DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_13()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_14()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG and DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_15()
{
	TBEGIN;

	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG | DBCONVERTFLAGS_ISFIXEDLENGTH), DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISLONG on a Variable Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_16()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISLONG with the Default of COLUMN
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR, 
											DBCONVERTFLAGS_ISLONG), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Invalid DBCONVERTFLAGS_ISFIXEDLENGTH on a Fixed Type.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_17()
{
	TBEGIN;

	// DBCONVERTFLAGS_ISFIXEDLENGTH with the Default of COLUMN
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR, 
								DBCONVERTFLAGS_ISFIXEDLENGTH), S_OK, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_BSTR case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_18()
{
	TBEGIN;

	// BSTR is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_ISLONG with DBTYPE_IUNKNOWN case.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_19()
{
	TBEGIN;

	// IUNKNOWN is a FIXED LENGTH DBType
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
			DBCONVERTFLAGS_COLUMN | DBCONVERTFLAGS_ISLONG),DB_E_BADCONVERTFLAG);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Valid DBCONVERTFLAGS_FROMVARIANT with valid variant types.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenIRowCanConvert::Variation_20()
{
	TBEGIN;

	// DBTYPE_EMPTY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_EMPTY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_NULL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_NULL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I2, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R4, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_R8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_R8, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_CY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_CY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DATE is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DATE, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BSTR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BSTR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IDISPATCH is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IDISPATCH, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ERROR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ERROR, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BOOL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BOOL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VARIANT is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VARIANT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_IUNKNOWN is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_IUNKNOWN, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI1, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_DECIMAL is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_DECIMAL, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_ARRAY is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_ARRAY, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYREF is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_BYREF, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I1 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I1, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI2 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI2, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI4 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI4, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_I8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_I8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_UI8 is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_UI8, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_GUID is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_GUID, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_VECTOR is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_VECTOR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_FILETIME is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_FILETIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_RESERVED is a valid Variant type
	TEST2C_(m_pIConvertType->CanConvert(DBTYPE_RESERVED, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), S_OK, S_FALSE);

	// DBTYPE_BYTES is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_BYTES, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_STR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_STR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_WSTR is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_WSTR, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_NUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_NUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_UDT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_UDT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBDATE is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBDATE, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIME is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIME, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_DBTIMESTAMP is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_DBTIMESTAMP, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_HCHAPTER is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_HCHAPTER, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_PROPVARIANT is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_PROPVARIANT, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// DBTYPE_VARNUMERIC is a invalid Variant type
	TESTC_(m_pIConvertType->CanConvert(DBTYPE_VARNUMERIC, DBTYPE_WSTR,
									DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE);

	// VT_INT is a valid Variant type
	TESTC_(m_pIConvertType->CanConvert(VT_INT, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// VT_RECORD is a valid Variant type new in VC 6.
	TESTC_(m_pIConvertType->CanConvert(36, DBTYPE_WSTR,
										DBCONVERTFLAGS_FROMVARIANT), S_FALSE);

	// 15 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(15, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 32 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(32, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// 37 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(37, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

	// -1 is a invalid Variant type
	TEST2C_(m_pIConvertType->CanConvert(-1, DBTYPE_WSTR, 
							DBCONVERTFLAGS_FROMVARIANT), DB_E_BADTYPE, S_FALSE);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenIRowCanConvert::Terminate()
{
	// Release the interface.
	SAFE_RELEASE(m_pIConvertType);
	SAFE_RELEASE(m_pIRow);
	
	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Clean up the Table
	if (m_pTable)
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCDataConvert)
//*-----------------------------------------------------------------------
//| Test Case:		TCDataConvert - Test case to test API level testing for DATA convert interface.
//|	Created:		08/15/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDataConvert::Init()
{
	CLSID clsid;

	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Create DataConvert object.
		TESTC_(CLSIDFromString((WCHAR *)CLASSID_IDataConvert, &clsid), S_OK);

		TESTC_(CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, 
							IID_IDataConvert, (void **)&m_pIDataConvert), S_OK);
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Source or Destination DBTYPE is invalid. returns DB_E_BADBINDINFO.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataConvert::Variation_1()
{
	TBEGIN;

	// Test Invalid arguments for DataConvert Function.
	TESTC_(m_pIDataConvert->DataConvert(USHRT_MAX,
						USHRT_MAX, 0, NULL, NULL, NULL, 
						USHRT_MAX, ULONG_MAX, NULL, UCHAR_MAX,
						UCHAR_MAX, 0), DB_E_BADBINDINFO);

	TESTC_(m_pIDataConvert->DataConvert(DBTYPE_BOOL, 
						USHRT_MAX, 0, NULL, NULL, NULL, 
						USHRT_MAX, ULONG_MAX, NULL, UCHAR_MAX, 
						UCHAR_MAX, 0), DB_E_BADBINDINFO);
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc pSrc is a NULL pointer, returns E_FAIL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataConvert::Variation_2()
{
	TBEGIN;

	ULONG src_data = ULONG_MAX;

	// Test Invalid arguments for DataConvert Function.
	TESTC_(m_pIDataConvert->DataConvert(DBTYPE_I4,
						DBTYPE_I2, ULONG_MAX, NULL, (void *)&src_data,
						NULL, USHRT_MAX, ULONG_MAX, NULL, 
						UCHAR_MAX, UCHAR_MAX, 0), S_OK);

	TESTC_(m_pIDataConvert->DataConvert(DBTYPE_I4, 
						DBTYPE_I2, ULONG_MAX, NULL, NULL,
						NULL, USHRT_MAX, ULONG_MAX, NULL,
						UCHAR_MAX, UCHAR_MAX, 0), E_FAIL);
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Requested conversion resulted in an overflow DB_E_OVERFLOW
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataConvert::Variation_3()
{
	TBEGIN;

	LONG src_data = LONG_MAX;
	LONG dst_data =0;
	DBSTATUS dbsStatus;

	// Test Invalid arguments for DataConvert Function.
	TESTC_(m_pIDataConvert->DataConvert(DBTYPE_I4,
						DBTYPE_I2, 4, NULL, (void *)&src_data,
						(void *)&dst_data, 2, DBSTATUS_S_OK, 
						&dbsStatus, 0, 0, 0), DB_E_DATAOVERFLOW);
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DB_E_ERRORSOCCURRED cases.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataConvert::Variation_4()
{
	TBEGIN;

	WCHAR src_data[22] = L"ABCDEFGHIJ";
	WCHAR dst_data[14];
	DBSTATUS dbsStatus;
	
	// Test Invalid arguments for DataConvert Function.
	TESTC_(m_pIDataConvert->DataConvert(DBTYPE_WSTR, 
						DBTYPE_WSTR, 20, NULL, (void *)src_data,
						(void *)dst_data, 10, DBSTATUS_S_OK, &dbsStatus,
						0, 0, DBDATACONVERT_SETDATABEHAVIOR), DB_E_ERRORSOCCURRED);
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Unsupported conversion DB_E_UNSUPPORTEDCONVERSION.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCDataConvert::Variation_5()
{
	TBEGIN;

	LONG src_data = LONG_MAX;

	// Test Invalid arguments for DataConvert Function.
	TESTC_(m_pIDataConvert->DataConvert(DBTYPE_I4,
						DBTYPE_BYREF|DBTYPE_I2, 4, NULL, (void *)&src_data,
						NULL, USHRT_MAX, ULONG_MAX, NULL, 
						UCHAR_MAX, UCHAR_MAX, 0), DB_E_UNSUPPORTEDCONVERSION);
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCDataConvert::Terminate()
{
	// Release the DataConvert pointer
	SAFE_RELEASE(m_pIDataConvert);

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCCZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCCZombie - Induce zombie states on the Command
//|	Created:		08/15/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCZombie::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		// Check to see if Commands are supported
		if(!m_pIDBCreateCommand) {
			odtLog << L"Commands not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}

		// Register Interface with Zombie
		if(RegisterInterface(COMMAND_INTERFACE, IID_IConvertType, 0, NULL))
			return TRUE;
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCZombie::Variation_1()
{
	// S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCZombie::Variation_2()
{
	// S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCZombie::Variation_3()
{
	// S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCZombie::Variation_4()
{
	// S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCZombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCRZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCRZombie - Induce zombie states on the Rowset
//|	Created:		08/15/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRZombie::Init()
{
	// Check to see if Transactions are usable
	if(!IsUsableInterface(SESSION_INTERFACE, IID_ITransactionLocal))
		return TEST_SKIPPED;

	// Initialize to a invalid pointer
	m_pITransactionLocal = INVALID(ITransactionLocal*);
	
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		// Register Interface with Zombie
		if(RegisterInterface(ROWSET_INTERFACE, IID_IConvertType, 0, NULL))
			return TRUE;
	}

	// Check to see if ITransaction is supported
    if(!m_pITransactionLocal)
		return TEST_SKIPPED;

    // Clear the bad pointer value
	if(m_pITransactionLocal == INVALID(ITransactionLocal*))
		m_pITransactionLocal = NULL;

	return FALSE;
}

// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRZombie::Variation_1()
{
	// S_OK - Abort IConvertType::CanConvert with fRetaining=TRUE
	return TestTxn(ETXN_ABORT, TRUE);
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRZombie::Variation_2()
{
	// S_OK - Abort IConvertType::CanConvert with fRetaining=FALSE
	return TestTxn(ETXN_ABORT, FALSE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRZombie::Variation_3()
{
	// S_OK - Commit IConvertType::CanConvert with fRetaining=TRUE
	return TestTxn(ETXN_COMMIT, TRUE);
}
// }}

// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRZombie::Variation_4()
{
	// S_OK - Commit IConvertType::CanConvert with fRetaining=FALSE
	return TestTxn(ETXN_COMMIT, FALSE);
}
// }}

// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCRZombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:		07/11/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCIConvertType::Init() )
	// }}
	{
		// Create the rowset object.
		TESTC_(CreateRowsetObject(USE_OPENROWSET), S_OK);

		// Verify and Create the Interface pointer for IConvertType.
		TESTC(VerifyInterface(m_pIAccessor, IID_IConvertType,
						ROWSET_INTERFACE,(IUnknown **)&m_pIConvertType));
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IConvertType calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	TBEGIN;
	HRESULT hr = E_FAIL;
	
	// Cause an Error
	m_pExtError->CauseError();

	// Check the conversion
	TEST2C_(hr=m_pIConvertType->CanConvert(DBTYPE_WSTR, 
							DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN),S_OK, S_FALSE);

	// Do extended check following IConvertType
	TESTC(XCHECK(m_pIConvertType, IID_IConvertType, hr));

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IConvertType calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	TBEGIN;
	HRESULT hr = E_FAIL;

	// Cause an Error
	m_pExtError->CauseError();
  
	// Call with invalid types
	TESTC_(hr=m_pIConvertType->CanConvert(USHRT_MAX, 
								USHRT_MAX, DBCONVERTFLAGS_COLUMN), S_FALSE);
  	
	// Do extended check following IConvertType
	TESTC(XCHECK(m_pIConvertType, IID_IConvertType, hr));

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IConvertType calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	TBEGIN;
	HRESULT hr = E_FAIL;

	// IConvertType with a bad ConvertFlag
	TESTC_(hr=m_pIConvertType->CanConvert(DBTYPE_I8, 
								DBTYPE_I8, USHRT_MAX), DB_E_BADCONVERTFLAG);

	// Do extended check following IConvertType
	TESTC(XCHECK(m_pIConvertType, IID_IConvertType, hr));

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Terminate()
{
	// Release the interface.
	SAFE_RELEASE(m_pIConvertType);

	ReleaseRowsetObject();
	ReleaseCommandObject();
	ReleaseDBSession();
	ReleaseDataSourceObject();

	// Clean up the Table
	if( m_pTable )
	{
		m_pTable->DropTable();
		delete m_pTable;
		m_pTable = NULL;
	}

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCIConvertType::Terminate());
}	// }}
// }}
// }}


//--------------------------------------------------------------------
// @mfunc Test Zombie cases
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCCZombie::TestTxn(ETXN eTxn, BOOL fRetaining)
{
	int						 fPassFail			 = TEST_FAIL;	// ReturnValue
	HRESULT					 ExpectedHr			 = E_UNEXPECTED;// Expected HRESULT
	DBCOUNTITEM				cRowsObtained		 = 0;			// Number of rows returned, should be 1
	HROW *					 rghRows			 = NULL;		// Array of Row Handles
	IConvertType *			 pIConvertType		 = NULL;		// IConvertType Pointer
	ICommandWithParameters * pICommandWithParams = NULL;		// ICommandWithParameter Pointer

	// Retrieve an Interface pointer to IDBCreateCommand within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIConvertType));

	// Obtain the ABORT or COMMIT PRESERVE flag and adjust ExpectedHr 
	if( ((eTxn == ETXN_COMMIT) && (m_fCommitPreserve)) ||
	    ((eTxn == ETXN_ABORT) && (m_fAbortPreserve)) )
		ExpectedHr = S_OK;

	// Commit or Abort the transaction, with retention as specified
	switch (eTxn)
	{
		case ETXN_COMMIT:
			TESTC(GetCommit(fRetaining));
			break;

		case ETXN_ABORT:
			TESTC(GetAbort(fRetaining));
			break;

		default:
			goto CLEANUP;
	}

	// Test zombie
	TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
		
	// Verify and Create the Interface pointer for IConvertType.
	if( VerifyInterface(m_pICommand, IID_ICommandWithParameters,
						COMMAND_INTERFACE,(IUnknown **)&pICommandWithParams) )
		ExpectedHr = S_OK;
	else
		ExpectedHr = DB_E_BADCONVERTFLAG;

	// Call for WSTR to WSTR
	TESTC_(pIConvertType->CanConvert(DBTYPE_WSTR, 
									DBTYPE_WSTR, DBCONVERTFLAGS_PARAMETER), ExpectedHr);

	fPassFail = TEST_PASS;

CLEANUP:
	
	// Release the row handle on the 1st rowset
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	PROVIDER_FREE(rghRows);

	// Release Interfaces
	SAFE_RELEASE(pIConvertType);
	SAFE_RELEASE(pICommandWithParams);

	// Cleanup Transactions
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);

	return fPassFail;
}

//--------------------------------------------------------------------
// @mfunc Test Zombie cases for the Rowset tests
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCRZombie::TestTxn(ETXN eTxn, BOOL fRetaining)
{
	int					fPassFail			= TEST_FAIL;	// ReturnValue
	HRESULT				ExpectedHr			= E_UNEXPECTED;	// Expected HRESULT
	DBCOUNTITEM			cRowsObtained		= 0;			// Number of rows returned, should be 1
	HROW *				rghRows				= NULL;			// Array of Row Handles
	IConvertType*		pIConvertType		= NULL;			// IConvertType Pointer

	// Retrieve an Interface pointer to IDBCreateCommand within a Transaction
	TESTC(StartTransaction(SELECT_ALLFROMTBL, (IUnknown**)&pIConvertType));

	// Obtain the ABORT or COMMIT PRESERVE flag and adjust ExpectedHr 
	if( ((eTxn == ETXN_COMMIT) && (m_fCommitPreserve)) ||
	    ((eTxn == ETXN_ABORT) && (m_fAbortPreserve)) )
		ExpectedHr = S_OK;

	// Commit or Abort the transaction, with retention as specified
	switch (eTxn)
	{
		case ETXN_COMMIT:
			TESTC(GetCommit(fRetaining));
			break;

		case ETXN_ABORT:
			TESTC(GetAbort(fRetaining));
			break;

		default:
			goto CLEANUP;
	}

	// Test zombie
	TESTC_(m_pIRowset->GetNextRows(0,0,1,&cRowsObtained,&rghRows), ExpectedHr);
				
	// Call for WSTR to WSTR
	TESTC_(pIConvertType->CanConvert(DBTYPE_WSTR, 
									DBTYPE_WSTR, DBCONVERTFLAGS_COLUMN), S_OK);
	fPassFail = TEST_PASS;

CLEANUP:
	
	// Release the row handle on the 1st rowset
	CHECK(m_pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL), S_OK);
	PROVIDER_FREE(rghRows);

	// Release the pIConvertType
	SAFE_RELEASE(pIConvertType);

	// Cleanup Transactions
	CleanUpTransaction(fRetaining ? S_OK : XACT_E_NOTRANSACTION);

	return fPassFail;
}
