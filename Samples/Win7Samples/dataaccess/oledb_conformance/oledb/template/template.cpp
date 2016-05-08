//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc 
//
// @module TEMPLATE.CPP | Template source file for all test modules.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "template.h"

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x17dff980, 0x64a3, 0x11ce, { 0xb1, 0x24, 0x00, 0xaa, 0x00, 0x57, 0x59, 0x9e }};
DECLARE_MODULE_NAME("Enter your module name here");
DECLARE_MODULE_OWNER("Enter your email name here");
DECLARE_MODULE_DESCRIP("Enter module description here");
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
	return TRUE;
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
	return TRUE;
}	


// {{ TCW_TEST_CASE_MAP(DefaultTestCase)
//*-----------------------------------------------------------------------
// @class This is a default test case. You can change it's name, delete it, or add more test cases through the Test Wizard tool.
//
class DefaultTestCase : public CTestCases{ 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(DefaultTestCase,CTestCases);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember DefaultVariation - You can rename this variation, delete it, or add new variations through Test Wizard tool.
	int Variation_1();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(DefaultTestCase)
#define THE_CLASS DefaultTestCase
BEG_TEST_CASE(DefaultTestCase, CTestCases, L"This is a default test case. You can change it's name, delete it, or add more test cases through the Test Wizard tool.")
	TEST_VARIATION(1, 		L"DefaultVariation - You can rename this variation, delete it, or add new variations through Test Wizard tool.")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(1, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, DefaultTestCase)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(DefaultTestCase)
//*-----------------------------------------------------------------------
//| Test Case:	DefaultTestCase - This is a default test case. You can change it's name, delete it, or add more test cases through the Test Wizard tool.
//| Created:  	9/27/1999
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL DefaultTestCase::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTestCases::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc DefaultVariation - You can rename this variation, delete it, or add new variations through Test Wizard tool.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int DefaultTestCase::Variation_1()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL DefaultTestCase::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTestCases::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END





