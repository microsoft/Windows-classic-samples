//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IOpenRW.cpp | This module tests the OLE DB IOpenRowset interface 
//

//////////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////////
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "MODStandard.hpp"		// Standard headers			
#include "IOpenRW.h"			// IOpenRowset header
#include "ExtraLib.h"


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x055b31b0, 0xca41, 0x11cf, { 0xbc, 0x5d, 0x00, 0xa0, 0xc9, 0x0d, 0x80, 0x7a }};
DECLARE_MODULE_NAME("IOpenRowset");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IOpenRowset interface test");
DECLARE_MODULE_VERSION(837801675);
// TCW_WizardVersion(2)
// TCW_Automation(False)
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
    return CommonModuleInit(pThisTestModule, IID_IOpenRowset);
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
	return CommonModuleTerminate(pThisTestModule);
}	


// {{ TCW_TEST_CASE_MAP(TCOpenRowset)
//--------------------------------------------------------------------
// @class IOpenRowset::OpenRowset test
//
class TCOpenRowset : public COpenRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCOpenRowset,COpenRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember General - QI - Mandatory and Optional
	int Variation_1();
	// @cmember General - QI invalid
	int Variation_2();
	// @cmember Empty
	int Variation_3();
	// @cmember DBPROP_ACTIVESESSIONS
	int Variation_4();
	// @cmember DBPROP_MULTIPLECONNECTIONS
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Empty
	int Variation_7();
	// @cmember Empty
	int Variation_8();
	// @cmember Boundary - Prop[element of struct NULL]
	int Variation_9();
	// @cmember Boundary - Prop[0, NULL, NULL]
	int Variation_10();
	// @cmember Boundary - Prop[0, valid, NULL]
	int Variation_11();
	// @cmember Boundary - Prop[0, NULL, valid]
	int Variation_12();
	// @cmember Boundary - Prop[0, valid, valid]
	int Variation_13();
	// @cmember Boundary - Prop[N, NULL, NULL]
	int Variation_14();
	// @cmember Boundary - Prop[N, valid, NULL]
	int Variation_15();
	// @cmember Boundary - Prop[N, NULL, valid]
	int Variation_16();
	// @cmember Boundary - Prop[N, valid, valid]
	int Variation_17();
	// @cmember Boundary - Prop[N, valid, valid]
	int Variation_18();
	// @cmember Boundary - IID_NULL - valid ppRowset
	int Variation_19();
	// @cmember Boundary - IID_NULL - NULL ppRowset
	int Variation_20();
	// @cmember Boundary - IID_NULL - valid properties
	int Variation_21();
	// @cmember Boundary - IID_NULL - invalid properties
	int Variation_22();
	// @cmember Boundary - All Defaults - VT_EMPTY
	int Variation_23();
	// @cmember Error - DB_E_NOTABLE
	int Variation_24();
	// @cmember Error - DB_E_NOINDEX
	int Variation_25();
	// @cmember Error - DB_E_ERRORSOCCURRED - CONFLICTING
	int Variation_26();
	// @cmember Error - DB_E_ERRORSOCCURRED - NOTSUPPORTED
	int Variation_27();
	// @cmember Error - DB_E_ERRORSOCCURRED - NOTSUPPORTED
	int Variation_28();
	// @cmember Error - DB_E_ERRORSOCCURRED - BADPROPERTYOPTION
	int Variation_29();
	// @cmember Error - DB_E_ERRORSOCCURRED - BADPROPERTYVALUE
	int Variation_30();
	// @cmember Error - DB_E_ERRORSOCCURRED - NOTSETABLE
	int Variation_31();
	// @cmember Error - DB_E_ERRORSOCCURRED - Bad Col info?
	int Variation_32();
	// @cmember Error - DB_E_ERRORSOCCURRED - All prop in error
	int Variation_33();
	// @cmember Error - E_NOINTERFACE - Invalid riid
	int Variation_34();
	// @cmember Error - E_NOINTERFACE - riid==IID_ILockBytes
	int Variation_35();
	// @cmember Error - riid== IID_NULL - E_NOINTERFACE
	int Variation_36();
	// @cmember Error - DB_SEC_E_PERMISSIONDENIED
	int Variation_37();
	// @cmember Parameters - pTableID - Empty Table
	int Variation_38();
	// @cmember Parameters - pTableID - 1 Row Table
	int Variation_39();
	// @cmember Parameters - DBPROP_OPENROWSETSUPPORT
	int Variation_40();
	// @cmember Parameters - pIndexID - Integrated Index Rowset
	int Variation_41();
	// @cmember Parameters - riid - All rowset IIDs
	int Variation_42();
	// @cmember Parameters - riid - All Rowset interface Properties - singularly
	int Variation_43();
	// @cmember Parameters - riid - All Rowset interface Properties - combinations
	int Variation_44();
	// @cmember Parameters - ppRowset - NULL && some error properties
	int Variation_45();
	// @cmember Parameters - ppRowset - NULL && no error properties
	int Variation_46();
	// @cmember Sequence - OpenRowset Twice (same table
	int Variation_47();
	// @cmember Sequence - OpenRowset Twice (diff table
	int Variation_48();
	// @cmember Sequence - OpenRowset Twice while altering table
	int Variation_49();
	// @cmember Sequence - Properties in error are still set?
	int Variation_50();
	// @cmember Sequence - Change backend table, refetch rows, verify
	int Variation_51();
	// @cmember Sequence - IColumnsInfo without prepare
	int Variation_52();
	// @cmember Sequence - IRowsetInfo::GetSpecifications with session object
	int Variation_53();
	// @cmember Related - 2 rowsets open with OpenRowset
	int Variation_54();
	// @cmember Related - 1 Rowset with Execute [select all], 1 with OpenRowset
	int Variation_55();
	// @cmember Properties - Verify implied properties
	int Variation_56();
	// @cmember Properties - Buffered Mode
	int Variation_57();
	// @cmember Properties - Static - Cursor
	int Variation_58();
	// @cmember Properties - Forward Only -  Cursor
	int Variation_59();
	// @cmember Properties - KeySet - Cursor
	int Variation_60();
	// @cmember Properties - Dynamic - Cursor
	int Variation_61();
	// @cmember Boundary - Table[NULL, NULL]
	int Variation_62();
	// @cmember Boundary - Table[NULL, valid]
	int Variation_63();
	// @cmember Boundary - Table[valid, NULL]
	int Variation_64();
	// @cmember Boundary - Table[valid, valid]
	int Variation_65();
	// @cmember Boundary - Table[valid, invalid]
	int Variation_66();
	// @cmember Boundary - Table[invalid, valid]
	int Variation_67();
	// @cmember Boundary - Table[NULL, invalid]
	int Variation_68();
	// @cmember Boundary - Long - Invalid TableName
	int Variation_69();
	// @cmember Boundary - Empty TableName
	int Variation_70();
	// @cmember Boundary - pwszName==NULL TableName
	int Variation_71();
	// @cmember Boundary - TableName starting with SQL command
	int Variation_72();
	// @cmember Boundary - TableName same as SQL command
	int Variation_73();
	// @cmember Boundary - TableName - Fully Qualified
	int Variation_74();
	// @cmember Boundary - TableName - Quoted
	int Variation_75();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCOpenRowset)
#define THE_CLASS TCOpenRowset
BEG_TEST_CASE(TCOpenRowset, COpenRowset, L"IOpenRowset::OpenRowset test")
	TEST_VARIATION(1, 		L"General - QI - Mandatory and Optional")
	TEST_VARIATION(2, 		L"General - QI invalid")
	TEST_VARIATION(3, 		L"Empty")
	TEST_VARIATION(4, 		L"DBPROP_ACTIVESESSIONS")
	TEST_VARIATION(5, 		L"DBPROP_MULTIPLECONNECTIONS")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Empty")
	TEST_VARIATION(8, 		L"Empty")
	TEST_VARIATION(9, 		L"Boundary - Prop[element of struct NULL]")
	TEST_VARIATION(10, 		L"Boundary - Prop[0, NULL, NULL]")
	TEST_VARIATION(11, 		L"Boundary - Prop[0, valid, NULL]")
	TEST_VARIATION(12, 		L"Boundary - Prop[0, NULL, valid]")
	TEST_VARIATION(13, 		L"Boundary - Prop[0, valid, valid]")
	TEST_VARIATION(14, 		L"Boundary - Prop[N, NULL, NULL]")
	TEST_VARIATION(15, 		L"Boundary - Prop[N, valid, NULL]")
	TEST_VARIATION(16, 		L"Boundary - Prop[N, NULL, valid]")
	TEST_VARIATION(17, 		L"Boundary - Prop[N, valid, valid]")
	TEST_VARIATION(18, 		L"Boundary - Prop[N, valid, valid]")
	TEST_VARIATION(19, 		L"Boundary - IID_NULL - valid ppRowset")
	TEST_VARIATION(20, 		L"Boundary - IID_NULL - NULL ppRowset")
	TEST_VARIATION(21, 		L"Boundary - IID_NULL - valid properties")
	TEST_VARIATION(22, 		L"Boundary - IID_NULL - invalid properties")
	TEST_VARIATION(23, 		L"Boundary - All Defaults - VT_EMPTY")
	TEST_VARIATION(24, 		L"Error - DB_E_NOTABLE")
	TEST_VARIATION(25, 		L"Error - DB_E_NOINDEX")
	TEST_VARIATION(26, 		L"Error - DB_E_ERRORSOCCURRED - CONFLICTING")
	TEST_VARIATION(27, 		L"Error - DB_E_ERRORSOCCURRED - NOTSUPPORTED")
	TEST_VARIATION(28, 		L"Error - DB_E_ERRORSOCCURRED - NOTSUPPORTED")
	TEST_VARIATION(29, 		L"Error - DB_E_ERRORSOCCURRED - BADPROPERTYOPTION")
	TEST_VARIATION(30, 		L"Error - DB_E_ERRORSOCCURRED - BADPROPERTYVALUE")
	TEST_VARIATION(31, 		L"Error - DB_E_ERRORSOCCURRED - NOTSETABLE")
	TEST_VARIATION(32, 		L"Error - DB_E_ERRORSOCCURRED - Bad Col info?")
	TEST_VARIATION(33, 		L"Error - DB_E_ERRORSOCCURRED - All prop in error")
	TEST_VARIATION(34, 		L"Error - E_NOINTERFACE - Invalid riid")
	TEST_VARIATION(35, 		L"Error - E_NOINTERFACE - riid==IID_ILockBytes")
	TEST_VARIATION(36, 		L"Error - riid== IID_NULL - E_NOINTERFACE")
	TEST_VARIATION(37, 		L"Error - DB_SEC_E_PERMISSIONDENIED")
	TEST_VARIATION(38, 		L"Parameters - pTableID - Empty Table")
	TEST_VARIATION(39, 		L"Parameters - pTableID - 1 Row Table")
	TEST_VARIATION(40, 		L"Parameters - DBPROP_OPENROWSETSUPPORT")
	TEST_VARIATION(41, 		L"Parameters - pIndexID - Integrated Index Rowset")
	TEST_VARIATION(42, 		L"Parameters - riid - All rowset IIDs")
	TEST_VARIATION(43, 		L"Parameters - riid - All Rowset interface Properties - singularly")
	TEST_VARIATION(44, 		L"Parameters - riid - All Rowset interface Properties - combinations")
	TEST_VARIATION(45, 		L"Parameters - ppRowset - NULL && some error properties")
	TEST_VARIATION(46, 		L"Parameters - ppRowset - NULL && no error properties")
	TEST_VARIATION(47, 		L"Sequence - OpenRowset Twice (same table")
	TEST_VARIATION(48, 		L"Sequence - OpenRowset Twice (diff table")
	TEST_VARIATION(49, 		L"Sequence - OpenRowset Twice while altering table")
	TEST_VARIATION(50, 		L"Sequence - Properties in error are still set?")
	TEST_VARIATION(51, 		L"Sequence - Change backend table, refetch rows, verify")
	TEST_VARIATION(52, 		L"Sequence - IColumnsInfo without prepare")
	TEST_VARIATION(53, 		L"Sequence - IRowsetInfo::GetSpecifications with session object")
	TEST_VARIATION(54, 		L"Related - 2 rowsets open with OpenRowset")
	TEST_VARIATION(55, 		L"Related - 1 Rowset with Execute [select all], 1 with OpenRowset")
	TEST_VARIATION(56, 		L"Properties - Verify implied properties")
	TEST_VARIATION(57, 		L"Properties - Buffered Mode")
	TEST_VARIATION(58, 		L"Properties - Static - Cursor")
	TEST_VARIATION(59, 		L"Properties - Forward Only -  Cursor")
	TEST_VARIATION(60, 		L"Properties - KeySet - Cursor")
	TEST_VARIATION(61, 		L"Properties - Dynamic - Cursor")
	TEST_VARIATION(62, 		L"Boundary - Table[NULL, NULL]")
	TEST_VARIATION(63, 		L"Boundary - Table[NULL, valid]")
	TEST_VARIATION(64, 		L"Boundary - Table[valid, NULL]")
	TEST_VARIATION(65, 		L"Boundary - Table[valid, valid]")
	TEST_VARIATION(66, 		L"Boundary - Table[valid, invalid]")
	TEST_VARIATION(67, 		L"Boundary - Table[invalid, valid]")
	TEST_VARIATION(68, 		L"Boundary - Table[NULL, invalid]")
	TEST_VARIATION(69, 		L"Boundary - Long - Invalid TableName")
	TEST_VARIATION(70, 		L"Boundary - Empty TableName")
	TEST_VARIATION(71, 		L"Boundary - pwszName==NULL TableName")
	TEST_VARIATION(72, 		L"Boundary - TableName starting with SQL command")
	TEST_VARIATION(73, 		L"Boundary - TableName same as SQL command")
	TEST_VARIATION(74, 		L"Boundary - TableName - Fully Qualified")
	TEST_VARIATION(75, 		L"Boundary - TableName - Quoted")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCExtendedErrors)
//--------------------------------------------------------------------
// @class Extended Errors
//
class TCExtendedErrors : public COpenRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCExtendedErrors,COpenRowset);
	// }} TCW_DECLARE_FUNCS_END
 

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Valid IOpenRowset calls with previous error object existing.
	int Variation_1();
	// @cmember Invalid IOpenRowset calls with previous error object existing
	int Variation_2();
	// @cmember Invalid IOpenRowset calls with no previous error object existing
	int Variation_3();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCExtendedErrors)
#define THE_CLASS TCExtendedErrors
BEG_TEST_CASE(TCExtendedErrors, COpenRowset, L"Extended Errors")
	TEST_VARIATION(1, 		L"Valid IOpenRowset calls with previous error object existing.")
	TEST_VARIATION(2, 		L"Invalid IOpenRowset calls with previous error object existing")
	TEST_VARIATION(3, 		L"Invalid IOpenRowset calls with no previous error object existing")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

  
// {{ TCW_TEST_CASE_MAP(TCZombie)
//--------------------------------------------------------------------
// @class Test the Zombie states of IOpenRowset
//
class TCZombie : public CTransaction { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCZombie,CTransaction);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Zombie - ABORT with fRetaining TRUE
	int Variation_1();
	// @cmember Zombie - ABORT with fRetaining FALSE
	int Variation_2();
	// @cmember Zombie - COMMIT with fRetaining TRUE
	int Variation_3();
	// @cmember Zombie - COMMIT with fRetaining FALSE
	int Variation_4();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCZombie)
#define THE_CLASS TCZombie
BEG_TEST_CASE(TCZombie, CTransaction, L"Test the Zombie states of IOpenRowset")
	TEST_VARIATION(1, 		L"Zombie - ABORT with fRetaining TRUE")
	TEST_VARIATION(2, 		L"Zombie - ABORT with fRetaining FALSE")
	TEST_VARIATION(3, 		L"Zombie - COMMIT with fRetaining TRUE")
	TEST_VARIATION(4, 		L"Zombie - COMMIT with fRetaining FALSE")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCAggregation)
//--------------------------------------------------------------------
// @class Test all Aggregation Senarios
//
class TCAggregation : public COpenRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCAggregation,COpenRowset);
	// }} TCW_DECLARE_FUNCS_END
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Aggregation - OpenRowset - non-IUnknown
	int Variation_1();
	// @cmember Aggregation - OpenRowset - IUnknown
	int Variation_2();
	// @cmember Aggregation - OpenRowset -> Rowset -> GetReferencedRowset
	int Variation_3();
	// @cmember Aggregation - CreateSession - non-IUnknown
	int Variation_4();
	// @cmember Aggregation - CreateSession - IUnknown
	int Variation_5();
	// @cmember Aggregation - OpenRowset -> Rowset -> GetSpecification
	int Variation_6();
	// @cmember Aggregation - CreateCommand - non-IUnknown
	int Variation_7();
	// @cmember Aggregation - CreateCommand - IUnknown
	int Variation_8();
	// @cmember Aggregation - CreateSession -> Command -> GetDBSession
	int Variation_9();
	// @cmember Aggregation - CreateCommand -> Rowset -> GetSpecification
	int Variation_10();
	// @cmember Aggregation - SchemaRowset - non-IUnknown
	int Variation_11();
	// @cmember Aggregation - SchemaRowset - IUnknown
	int Variation_12();
	// @cmember Aggregation - CreateSession -> SchemaRowset -> GetSpecification
	int Variation_13();
	// @cmember Aggregation - Execute - non-IUnknown
	int Variation_14();
	// @cmember Aggregation - MultipleResults - IUnknown
	int Variation_15();
	// @cmember Aggregation - CreateCommand -> MultipleResults -> GetSpecification
	int Variation_16();
	// @cmember Aggregation - IColumnsRowset - non-IUnknown
	int Variation_17();
	// @cmember Aggregation - IColumnsRowset - IUnknown
	int Variation_18();
	// @cmember Aggregation - Rowset -> IColumnsRowset -> GetSpecification
	int Variation_19();
	// @cmember Aggregation - ISourcesRowset - non-IUnknown
	int Variation_20();
	// @cmember Aggregation - ISourcesRowset - IUnknown
	int Variation_21();
	// @cmember Aggregation - Enumerator -> SourcesRowset -> GetSpecification
	int Variation_22();
	// }} TCW_TESTVARS_END
};
// {{ TCW_TESTCASE(TCAggregation)
#define THE_CLASS TCAggregation
BEG_TEST_CASE(TCAggregation, COpenRowset, L"Test all Aggregation Senarios")
	TEST_VARIATION(1, 		L"Aggregation - OpenRowset - non-IUnknown")
	TEST_VARIATION(2, 		L"Aggregation - OpenRowset - IUnknown")
	TEST_VARIATION(3, 		L"Aggregation - OpenRowset -> Rowset -> GetReferencedRowset")
	TEST_VARIATION(4, 		L"Aggregation - CreateSession - non-IUnknown")
	TEST_VARIATION(5, 		L"Aggregation - CreateSession - IUnknown")
	TEST_VARIATION(6, 		L"Aggregation - OpenRowset -> Rowset -> GetSpecification")
	TEST_VARIATION(7, 		L"Aggregation - CreateCommand - non-IUnknown")
	TEST_VARIATION(8, 		L"Aggregation - CreateCommand - IUnknown")
	TEST_VARIATION(9, 		L"Aggregation - CreateSession -> Command -> GetDBSession")
	TEST_VARIATION(10, 		L"Aggregation - CreateCommand -> Rowset -> GetSpecification")
	TEST_VARIATION(11, 		L"Aggregation - SchemaRowset - non-IUnknown")
	TEST_VARIATION(12, 		L"Aggregation - SchemaRowset - IUnknown")
	TEST_VARIATION(13, 		L"Aggregation - CreateSession -> SchemaRowset -> GetSpecification")
	TEST_VARIATION(14, 		L"Aggregation - Execute - non-IUnknown")
	TEST_VARIATION(15, 		L"Aggregation - MultipleResults - IUnknown")
	TEST_VARIATION(16, 		L"Aggregation - CreateCommand -> MultipleResults -> GetSpecification")
	TEST_VARIATION(17, 		L"Aggregation - IColumnsRowset - non-IUnknown")
	TEST_VARIATION(18, 		L"Aggregation - IColumnsRowset - IUnknown")
	TEST_VARIATION(19, 		L"Aggregation - Rowset -> IColumnsRowset -> GetSpecification")
	TEST_VARIATION(20, 		L"Aggregation - ISourcesRowset - non-IUnknown")
	TEST_VARIATION(21, 		L"Aggregation - ISourcesRowset - IUnknown")
	TEST_VARIATION(22, 		L"Aggregation - Enumerator -> SourcesRowset -> GetSpecification")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCProperties)
//*-----------------------------------------------------------------------
// @class Test all senarios dealing with OPTIONAL
//
class TCProperties : public COpenRowset { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCProperties,COpenRowset);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember OPTIONAL - DBPROPSTATUS_OK [Group]
	int Variation_1();
	// @cmember OPTIONAL - DBPROPSTATUS_OK [Single]
	int Variation_2();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTSET [Group]
	int Variation_3();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTSET [Single]
	int Variation_4();
	// @cmember OPTIONAL - DBPROPSTATUS_BADCOLUMN [Group]
	int Variation_5();
	// @cmember OPTIONAL - DBPROPSTATUS_BADCOLUMN [Single]
	int Variation_6();
	// @cmember OPTIONAL - DBPROPSTATUS_BADOPTION [Group]
	int Variation_7();
	// @cmember OPTIONAL - DBPROPSTATUS_BADOPTION [Single]
	int Variation_8();
	// @cmember OPTIONAL - DBPROPSTATUS_BADVALUE [Group]
	int Variation_9();
	// @cmember OPTIONAL - DBPROPSTATUS_BADVALUE [Single]
	int Variation_10();
	// @cmember OPTIONAL - DBPROPSTATUS_CONFLICTING [Group]
	int Variation_11();
	// @cmember OPTIONAL - DBPROPSTATUS_CONFLICTING [Single]
	int Variation_12();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTALLSETTABLE [Group]
	int Variation_13();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTALLSETTABLE [Single]
	int Variation_14();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTSETTABLE [Group]
	int Variation_15();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTSETTABLE [Single]
	int Variation_16();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTSUPPORTED [Group]
	int Variation_17();
	// @cmember OPTIONAL - DBPROPSTATUS_NOTSUPPORTED [Single]
	int Variation_18();
	// @cmember Empty
	int Variation_19();
	// @cmember Multiple Sets - Static Arrays
	int Variation_20();
	// @cmember Multiple Sets - Static Arrays - With Errors
	int Variation_21();
	// @cmember Empty
	int Variation_22();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCProperties)
#define THE_CLASS TCProperties
BEG_TEST_CASE(TCProperties, COpenRowset, L"Test all senarios dealing with OPTIONAL")
	TEST_VARIATION(1, 		L"OPTIONAL - DBPROPSTATUS_OK [Group]")
	TEST_VARIATION(2, 		L"OPTIONAL - DBPROPSTATUS_OK [Single]")
	TEST_VARIATION(3, 		L"OPTIONAL - DBPROPSTATUS_NOTSET [Group]")
	TEST_VARIATION(4, 		L"OPTIONAL - DBPROPSTATUS_NOTSET [Single]")
	TEST_VARIATION(5, 		L"OPTIONAL - DBPROPSTATUS_BADCOLUMN [Group]")
	TEST_VARIATION(6, 		L"OPTIONAL - DBPROPSTATUS_BADCOLUMN [Single]")
	TEST_VARIATION(7, 		L"OPTIONAL - DBPROPSTATUS_BADOPTION [Group]")
	TEST_VARIATION(8, 		L"OPTIONAL - DBPROPSTATUS_BADOPTION [Single]")
	TEST_VARIATION(9, 		L"OPTIONAL - DBPROPSTATUS_BADVALUE [Group]")
	TEST_VARIATION(10, 		L"OPTIONAL - DBPROPSTATUS_BADVALUE [Single]")
	TEST_VARIATION(11, 		L"OPTIONAL - DBPROPSTATUS_CONFLICTING [Group]")
	TEST_VARIATION(12, 		L"OPTIONAL - DBPROPSTATUS_CONFLICTING [Single]")
	TEST_VARIATION(13, 		L"OPTIONAL - DBPROPSTATUS_NOTALLSETTABLE [Group]")
	TEST_VARIATION(14, 		L"OPTIONAL - DBPROPSTATUS_NOTALLSETTABLE [Single]")
	TEST_VARIATION(15, 		L"OPTIONAL - DBPROPSTATUS_NOTSETTABLE [Group]")
	TEST_VARIATION(16, 		L"OPTIONAL - DBPROPSTATUS_NOTSETTABLE [Single]")
	TEST_VARIATION(17, 		L"OPTIONAL - DBPROPSTATUS_NOTSUPPORTED [Group]")
	TEST_VARIATION(18, 		L"OPTIONAL - DBPROPSTATUS_NOTSUPPORTED [Single]")
	TEST_VARIATION(19, 		L"Empty")
	TEST_VARIATION(20, 		L"Multiple Sets - Static Arrays")
	TEST_VARIATION(21, 		L"Multiple Sets - Static Arrays - With Errors")
	TEST_VARIATION(22, 		L"Empty")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(5, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCOpenRowset)
	TEST_CASE(2, TCExtendedErrors)
	TEST_CASE(3, TCZombie)
	TEST_CASE(4, TCAggregation)
	TEST_CASE(5, TCProperties)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END


// {{ TCW_TC_PROTOTYPE(TCOpenRowset)
//*-----------------------------------------------------------------------
//| Test Case:		TCOpenRowset - IOpenRowset::OpenRowset test
//|	Created:			04/30/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COpenRowset::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - QI - Mandatory and Optional
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_1()
{
	TBEGIN

	//[Mandatory] interface
    TESTC(DefaultObjectTesting(pIOpenRowset(), SESSION_INTERFACE));
    
CLEANUP:
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - QI invalid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_2()
{
	TBEGIN

	//invalid session interface
	TEST_(QI(pIOpenRowset(),IID_IConnectionPoint),E_NOINTERFACE)
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_3()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_ACTIVESESSIONS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_4()
{
	TBEGIN

	IGetDataSource* pIGetDataSource = NULL;
	ULONG_PTR	ulMaxSessions = 0;

	TESTC_PROVIDER(GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession, &ulMaxSessions));

	//Provider only allows 1 active Session
    //Obtain the session pointer
	if( ulMaxSessions != 1 )
	{
	    TESTC_(g_pIDBCreateSession->CreateSession(NULL,IID_IGetDataSource,(IUnknown**)&pIGetDataSource),S_OK)
		TESTC_(QI(pIGetDataSource,IID_IOpenRowset),S_OK)
	}
	else
	{
	    TESTC_(g_pIDBCreateSession->CreateSession(NULL,IID_IGetDataSource,(IUnknown**)&pIGetDataSource),DB_E_OBJECTCREATIONLIMITREACHED)
	}

CLEANUP:
    SAFE_RELEASE(pIGetDataSource);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc DBPROP_MULTIPLECONNECTIONS
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_5()
{
	TBEGIN
	const ULONG cDSOs		= 2;	//# of DSOs
	const ULONG cSessions	= 3;	//# of Sessions per DSO
	const ULONG cRowsets	= 4;	//# of Rowsets per session
	ULONG iObject			= 0;
	ULONG_PTR ulActiveSessions	= 0;
	HRESULT hr = S_OK;

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//We are not only storing the total rowsets (cDSOs*cSession*cRowsets) but also
	//are storing the DSOs and Sessions...
	IUnknown*		rgObjects[cDSOs + (cDSOs * cSessions) + (cDSOs * cSessions * cRowsets)];
	IUnknown**		ppIterator = rgObjects;
	memset(rgObjects, 0, sizeof(rgObjects));

    //DBPROP_MULTIPLECONNECTIONS.  All providers must (by default) allow the user to create
	//multiple sessions, commands, rowsets, etc.  This may spawn new connections or hstmt's
	//to the server.  The user can turn this off with MULTIPLECONNECTIONS=FALSE, if the
	//provider supports this property.
	
	//Most of the other senarios in the rest of the Conformance Tests already do a good job of this
	//but for a sanity check lets do a simple "tree" verification, (of 3, 4, 5)
	//NOTE: We are interested in OpenRowset, as Commands will be tested in the Command Test...
	//
	// Let: D=DSO, S=Session, R=Rowset.
	//								|
	//								|
	//							+---+---+
	//							D1	D2	D3	(3 DSOs total)
	//							|
	//							|
	//						+---+---+---+	(4 Sessions per DSO)
	//						S1	S2	S3	S4
	//						|	...	...	
	//						|
	//					+---+---+--=+---+	(5 Rowsets per Session)
	//					R1	R2	R3	R4	R5

	//NOTE: we are storing the pointers in an single array for simplicity, 
	//altough its really tree based.  (ie: the reason for the incrementor++)
	
	//Multiple DataSources 
	//(not dependent upon the property but interesting none the less)
	for(ULONG iDSO=0; iDSO<cDSOs; iDSO++)
	{
		IUnknown** ppDSO = ppIterator++;
		BOOL bMultipleConnections = TRUE;

		//Create a new DSO
		TESTC_(CreateNewDSO(NULL, IID_IDBProperties, ppDSO, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
		
		//Wither its supported or not the default behavior has to allow for multiple objects open...
		if(SupportedProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, *ppDSO))
		{
			//If supported make sure the default is VARIANT_TRUE
			TESTC(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, *ppDSO, VARIANT_TRUE));

			//For one of the DSOs (the first one) we will turn off MultipleConnections
			//to make sure first that it works, and second that it doesn't affect the other DSO
			if(iDSO == 0)
			{
				//Make sure we set the value to its default value - VARIANT_TRUE
				::SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, DBTYPE_BOOL, VARIANT_TRUE);
				TESTC_(hr = ((IDBProperties*)*ppDSO)->SetProperties(cPropSets, rgPropSets),S_OK);
				TESTC(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, *ppDSO, VARIANT_TRUE));
				::FreeProperties(&cPropSets, &rgPropSets);
				
				//Make sure we set the value to its default value - VT_EMPTY
				::SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, DBTYPE_EMPTY, (void*)NULL);
				TESTC_(hr = ((IDBProperties*)*ppDSO)->SetProperties(cPropSets, rgPropSets),S_OK);
				TESTC(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, *ppDSO, VARIANT_TRUE));
				::FreeProperties(&cPropSets, &rgPropSets);

				if(SettableProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, *ppDSO))
				{
					//Set it to FALSE
					::SetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, &cPropSets, &rgPropSets, DBTYPE_BOOL, (ULONG_PTR) VARIANT_FALSE);
					TESTC_(hr = ((IDBProperties*)*ppDSO)->SetProperties(cPropSets, rgPropSets),S_OK);
					TESTC(GetProperty(DBPROP_MULTIPLECONNECTIONS, DBPROPSET_DATASOURCE, *ppDSO, VARIANT_FALSE));
					bMultipleConnections = FALSE;
				}
			}
		}
		
		//DBPROP_ACTIVESESSIONS	- some (limited) providers have session limits
		GetProperty(DBPROP_ACTIVESESSIONS, DBPROPSET_DATASOURCEINFO, *ppDSO, &ulActiveSessions);
	
		//Multiple Sessions
		for(ULONG iSession=0; iSession<cSessions; iSession++)
		{
			IUnknown** ppSession = ppIterator++;
			TEST3C_(hr = CreateNewSession(*ppDSO, IID_IOpenRowset, (IUnknown**)ppSession),S_OK,DB_E_OBJECTCREATIONLIMITREACHED,DB_E_OBJECTOPEN);

			if(hr == S_OK)
			{
				if(bMultipleConnections == FALSE && iSession > 0)
					TWARNING("Provider able to open additional sessions (" << iSession+1 << ") even when MULTIPLECONNECTIONS=FALSE?");
				
				//Multiple Rowsets
				for(ULONG iRowset=0; iRowset<cRowsets; iRowset++)
				{
					CRowset RowsetA;
					
					//IOpenRowset::OpenRowset
					IUnknown** ppRowset = ppIterator++;
					TEST2C_(hr = ((IOpenRowset*)*ppSession)->OpenRowset(NULL, &m_pTable->GetTableID(), NULL, IID_IUnknown, 0, NULL, ppRowset),S_OK,DB_E_OBJECTOPEN);
					
					if(hr == S_OK)
					{
						TESTC(DefaultObjectTesting(*ppRowset, ROWSET_INTERFACE));
					
						//Verify this rowset
						//NOTE: Can't really verify rows of data since OpenRowset doesn't 
						//have to return the same order as the SELECT_ORDERBYNUMERIC
						//Since this variation is only interested in "OpenRowset" we will just make
						//sure the rows and columns are the same. (functional rowset)
						RowsetA.CreateRowset(*ppRowset);
						TESTC(RowsetA.GetTotalRows() == m_pTable->GetRowsOnCTable());
						
						if(bMultipleConnections == FALSE && (iSession > 0 || iRowset > 0))
							TWARNING("Provider able to open additional rowsets (" << iRowset+1 << ") for Session (" << iSession+1 << ") even when MULTIPLECONNECTIONS=FALSE?");
					}
					else
					{
						TESTC(*ppRowset == NULL);
						TESTC(bMultipleConnections == FALSE && (iSession > 0 || iRowset > 0));
					}
				}
			}
			else
			{
				if(hr == DB_E_OBJECTCREATIONLIMITREACHED)
				{
					//Some providers have a limit of the number of sessions.
					//Take this into account and verify
					TESTC(ulActiveSessions!=0 && ulActiveSessions<((iDSO*cSessions)+iSession+1));
				}
				else
				{
					//We only turned off MultipleConnections for the first DSO
					TESTC(bMultipleConnections == FALSE && (iSession > 0));
				}
			}
		}
	}

	//All objects were created, and must be a seperate object according to the spec
	for(iObject=0; iObject<NUMELEM(rgObjects); iObject++)
		for(ULONG iNext=iObject+1; iNext<NUMELEM(rgObjects); iNext++)
		{
			//We might have hit a session limit (reason for some being NULL)
			if(rgObjects[iObject] && rgObjects[iNext])
			{
				//Verify all objects are unique
				TESTC(!VerifyEqualInterface(rgObjects[iObject], rgObjects[iNext]));
			}
		}


CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);

	//Now release everything!
	for(iObject=0; iObject<NUMELEM(rgObjects); iObject++)
		SAFE_RELEASE(rgObjects[iObject]);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_6()
{
	return TEST_SKIPPED;
} 
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_7()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_8()
{
	return TEST_SKIPPED;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[element of struct NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_9()
{
	TBEGIN

    IRowset* pIRowset = INVALID(IRowset*);

	const ULONG cPropSets = 1;
	DBPROPSET rgPropSets[cPropSets];

	//an element in rgPropSets, cProperties was not 0 and rgProperties == NULL
	rgPropSets[0].cProperties     = 10;
	rgPropSets[0].rgProperties    = NULL;
	rgPropSets[0].guidPropertySet = DBPROPSET_ROWSET;

    //NULL element in PropSet struct - E_INVALIDARG
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,cPropSets,rgPropSets,(IUnknown**)&pIRowset),E_INVALIDARG)
    TESTC(pIRowset==NULL)

CLEANUP:
    if(pIRowset != INVALID(IRowset*))
		SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[0, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_10()
{                                                                
	TBEGIN

    IRowset* pIRowset = NULL;

    //Properties (0,NULL) - S_OK
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)
    TESTC(ValidInterface(IID_IRowset, pIRowset))

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[0, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_11()
{
	TBEGIN

    IRowset* pIRowset = NULL;
    SetProperty(DBPROP_CACHEDEFERRED);
    
    //Properties (0,valid) - S_OK
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,0,m_rgPropSets,(IUnknown**)&pIRowset),S_OK)
    TESTC(ValidInterface(IID_IRowset, pIRowset))

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[0, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_12()
{
	TBEGIN

    IRowset* pIRowset = NULL;
        
    //Properties (0,NULL) - S_OK
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)
    TESTC(ValidInterface(IID_IRowset, pIRowset))
    
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[0, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_13()
{
	TBEGIN

    IRowset* pIRowset = NULL;

    //Set 3 properties
    SetProperty(DBPROP_BOOKMARKS);
    SetProperty(DBPROP_CANHOLDROWS);
    SetProperty(DBPROP_OWNUPDATEDELETE);
    
    //Properties (0,valid) - S_OK
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,0,m_rgPropSets,(IUnknown**)&pIRowset),S_OK)
    TESTC(ValidInterface(IID_IRowset, pIRowset))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[N, NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_14()
{
	TBEGIN
    IRowset* pIRowset = INVALID(IRowset*);
        
    //Properties (N,NULL) - E_INVALIDARG
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,5,NULL,(IUnknown**)&pIRowset),E_INVALIDARG)
    TESTC(pIRowset==NULL)
     
CLEANUP:
    if(pIRowset!=INVALID(IRowset*)) SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[N, valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_15()
{
	TBEGIN
    IRowset* pIRowset = NULL;
    
    //DBPROP_CANHOLDROWS is required by Level-0 conformance spec
    //Make sure it ignores ColID for non-column properties
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED, DBCOLUMN_MAYSORT);

    //Properties (N,valid) - S_OK
    TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset),S_OK);

	//Make sure all the required rowset properites are supported and VARIANT_TRUE
	TESTC(GetProperty(DBPROP_IAccessor,		DBPROPSET_ROWSET, pIRowset));
	TESTC(GetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET, pIRowset));
	TESTC(GetProperty(DBPROP_IConvertType,	DBPROPSET_ROWSET, pIRowset));
	TESTC(GetProperty(DBPROP_IRowset,		DBPROPSET_ROWSET, pIRowset));
	TESTC(GetProperty(DBPROP_IRowsetInfo,	DBPROPSET_ROWSET, pIRowset));
	TESTC(GetProperty(DBPROP_CANHOLDROWS,	DBPROPSET_ROWSET, pIRowset));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[N, NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_16()
{
	TBEGIN
    IRowset* pIRowset = INVALID(IRowset*);
        
    //Properties (N,NULL) - E_INVALIDARG
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,3000,NULL,(IUnknown**)&pIRowset),E_INVALIDARG)
    TESTC(pIRowset==NULL)

CLEANUP:
    if(pIRowset!=INVALID(IRowset*)) SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[N, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_17()
{
	TBEGIN
    IRowset*	 pIRowset = NULL;

    //Set 3 properties
    SetProperty(DBPROP_IAccessor,		DBPROPSET_ROWSET);	//SET TO DEFAULT
    SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET);	//SET TO DEFAULT
    SetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET);	//SET TO DEFAULT
    SetProperty(DBPROP_IConvertType,	DBPROPSET_ROWSET);	//SET TO DEFAULT
    SetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET);	//SET TO DEFAULT
    SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET);	//SET TO DEFAULT
    
    //Properties (N,valid), set ReadOnly to it current value - S_OK
    TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset),S_OK);
 
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Prop[N, valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_18()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set 3 properties
    SetProperty(DBPROP_IAccessor,		DBPROPSET_ROWSET, (void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET, (void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET, (void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET, (void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IConvertType,	DBPROPSET_ROWSET, (void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IRowset,			DBPROPSET_ROWSET, (void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    
    //Properties (N,valid), with property errors - DB_E_ERRORSOCCURRED
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,m_cPropSets,m_rgPropSets,(IUnknown**)&pIRowset), DB_E_ERRORSOCCURRED)
	TESTC(pIRowset==NULL)
	
	//Verify property error array
	TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_NOTSETTABLE));
 
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Boundary - IID_NULL - valid ppRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_19()
{
	TBEGIN
    IUnknown* pIUnknown = NULL;

    //Set 3 properties
    SetSettableProperty(DBPROP_CANHOLDROWS);       //OK
    SetSettableProperty(DBPROP_OWNUPDATEDELETE);   //OK
    
    //IID_NULL - valid properties
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_NULL,m_cPropSets,m_rgPropSets,&pIUnknown),E_NOINTERFACE)
    TESTC(pIUnknown==NULL)
	
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIUnknown);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Boundary - IID_NULL - NULL ppRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_20()
{
	TBEGIN
    //Set 3 properties
    //CANHOLDROWS is required for Level 0 conformance
	SetProperty(DBPROP_CANHOLDROWS);       //OK
    
    //IID_IRowset - NULL ppRowset - S_OK - valid properties
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,m_cPropSets,m_rgPropSets,NULL),S_OK)

	//IID_NULL - NULL ppRowset - E_NOINTERFACE (IID_NULL)
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_NULL,m_cPropSets,m_rgPropSets,NULL),E_NOINTERFACE)
	
CLEANUP:
	FreeProperties();
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Boundary - IID_NULL - valid properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_21()
{
	TBEGIN
    IUnknown* pIUnknown = NULL;

    //Set 3 properties
    SetSettableProperty(DBPROP_CANHOLDROWS);       //OK
    SetSettableProperty(DBPROP_OWNUPDATEDELETE);   //OK
    
    //IID_NULL - valid properties
    TESTC_(CreateOpenRowset(IID_NULL, &pIUnknown),E_NOINTERFACE);
    TESTC(pIUnknown==NULL)
	
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIUnknown);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Boundary - IID_NULL - invalid properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_22()
{
	TBEGIN
    IUnknown* pIUnknown = NULL;
	HRESULT hr = S_OK;

    //Set 3 properties
    SetSettableProperty(DBPROP_CANHOLDROWS);       //OK
    SetSettableProperty(DBPROP_OWNUPDATEDELETE);   //OK
    SetProperty(DBPROP_IRowset);					//SET TO DEFAULT
    SetProperty(DBPROP_GROUPBY);					//NOTSUPPORTED (wrong PROPSET)
    
    //IID_NULL - valid properties
    //Since there are 2 errors, (IID_NULL and Property Errors)
	//Its provider specific which one gets evaulated first.
    TEST2C_(CreateOpenRowset(IID_NULL, &pIUnknown), E_NOINTERFACE, DB_E_ERRORSOCCURRED);
	
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIUnknown);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//--------------------------------------------------------------------
// @mfunc Boundary - All Defaults - VT_EMPTY
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_23()
{
	TBEGIN
    IRowsetInfo*	 pIRowsetInfo = NULL;
	ULONG iPropSet, iProp = 0;

    //Set properties (all to defaults)
    SetProperty(DBPROP_IAccessor,		DBPROPSET_ROWSET,	NULL, DBTYPE_EMPTY);	//SET TO DEFAULT
    SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET,	NULL, DBTYPE_EMPTY);	//SET TO DEFAULT
    SetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET,	NULL, DBTYPE_EMPTY);	//SET TO DEFAULT
    SetProperty(DBPROP_IConvertType,	DBPROPSET_ROWSET,	NULL, DBTYPE_EMPTY);	//SET TO DEFAULT
    SetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET,	NULL, DBTYPE_EMPTY);	//SET TO DEFAULT
    SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET,	NULL, DBTYPE_EMPTY);	//SET TO DEFAULT
    
    //Properties (N,valid), set ReadOnly to it current value - S_OK
	//All properties can be set to VT_EMPTY (even readonly) as this indicates
	//to return the property to its default value...
    TESTC_(CreateOpenRowset(IID_IRowsetInfo, (IUnknown**)&pIRowsetInfo),S_OK);

	//Now that we have a rowset, obtain all the properties on the current rowset
	FreeProperties();
	TESTC_(pIRowsetInfo->GetProperties(0, NULL, &m_cPropSets, &m_rgPropSets),S_OK);

	//Now set all those properties as VT_EMPTY
	for(iPropSet=0; iPropSet<m_cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &m_rgPropSets[iPropSet];
		for(iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			VariantClear(&pProp->vValue);
			V_VT(&pProp->vValue) = VT_EMPTY; 
		}
	}

	//Now set all as VT_EMPTY
	SAFE_RELEASE(pIRowsetInfo);
    TESTC_(CreateOpenRowset(IID_IRowsetInfo, (IUnknown**)&pIRowsetInfo),S_OK);

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIRowsetInfo);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_NOTABLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_24()
{
	TBEGIN
    IRowset* pIRowset = NULL;
    
	//NULLID for a TableID - DB_E_NOTABLE
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&NULLDBID,NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),DB_E_NOTABLE)
    TESTC(pIRowset==NULL)
     
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_NOINDEX
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_25()
{
	TBEGIN
    IRowset* pIRowset = NULL;

	//NULLID for a IndexID - DB_E_NOINDEX
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),&NULLDBID,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),DB_E_NOINDEX)
    TESTC(pIRowset==NULL)
     
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN    
}
// }}


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - CONFLICTING
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_26()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
	SetSettableProperty(DBPROP_CANHOLDROWS);
	SetSettableProperty(DBPROP_IRowsetUpdate);
	SetSettableProperty(DBPROP_IRowsetChange);

	//This variation requires these props supported by the driver
	//if not were done, since we can't get conflicting properties
	TESTC_PROVIDER(SetSettableProperty(DBPROP_OWNUPDATEDELETE));  //Conflicts with QBU
	TESTC_PROVIDER(SetSettableProperty(KAGPROP_QUERYBASEDUPDATES, DBPROPSET_PROVIDERROWSET));
    
	//Property errors - DB_E_ERRORSOCCURRED
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
       
    //Verify error array							  
	if(SettableProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET))
		TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	if(SettableProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET))
		TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IRowsetUpdate,DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	if(SettableProperty(DBPROP_IRowsetChange, DBPROPSET_ROWSET))
		TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IRowsetChange,DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNUPDATEDELETE,DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
	TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, KAGPROP_QUERYBASEDUPDATES,DBPROPSET_PROVIDERROWSET, DBPROPSTATUS_OK));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - NOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_27()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
    SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSETALL);	//NOTSUPPORTED (Invalid PropSet)
    SetProperty(DBPROP_IRowset,		DBPROPSET_ROWSETALL);	//NOTSUPPORTED (Invalid PropSet)
    SetProperty(DBPROP_IAccessor,	DBPROPSET_ROWSET);		//OK
    
    //Property errors - DB_E_ERRORSOCCURRED
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
       
    //Verify error array
    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,	DBPROPSET_ROWSETALL,	DBPROPSTATUS_NOTSUPPORTED))
    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IRowset,		DBPROPSET_ROWSETALL,	DBPROPSTATUS_NOTSUPPORTED))
    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IAccessor,		DBPROPSET_ROWSET,		DBPROPSTATUS_OK))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - NOTSUPPORTED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_28()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
	SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_DATASOURCEINFO,(void*)VARIANT_TRUE,DBTYPE_BOOL);	//NOTSUPPORTED
    
    //Property errors - DB_E_ERRORSOCCURRED
	TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
	TESTC(pIRowset == NULL);
		
	//Verify error array
	if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, DBPROPSTATUS_OK))

	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_DATASOURCEINFO, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - BADPROPERTYOPTION
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_29()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);			//OK
    SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_ROWSET,(void*)VARIANT_TRUE,DBTYPE_BOOL,ULONG_MAX);    //BADOPTION
    
    //Property errors - DB_E_ERRORSOCCURRED
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
       
    //Verify error array
	if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, DBPROPSTATUS_OK))

	//DBPROP_CANHOLDROWS is a required property (Conformance)
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBPROPSTATUS_BADOPTION))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - BADPROPERTYVALUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_30()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set 3 properties
    SetSettableProperty(DBPROP_QUICKRESTART);       //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);  //OK
    SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)10, DBTYPE_I4);    // BADVALUE

    //Property errors - DB_E_ERRORSOCCURRED
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
       
    //Verify error array
	if(SettableProperty(DBPROP_QUICKRESTART, DBPROPSET_ROWSET))
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_QUICKRESTART, DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, DBPROPSTATUS_OK))

	//DBPROP_CANHOLDROWS is a required property (Conformance)
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBPROPSTATUS_BADVALUE))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);			   
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - NOTSETABLE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_31()
{
	TBEGIN
    IRowset* pIRowset = NULL;
      
    //Set 3 properties
    SetSettableProperty(DBPROP_CANHOLDROWS);        //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);  //OK
    SetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, (void*)10, DBTYPE_I4);    //NOTSETTABLE
    
    //Property errors - DB_E_ERRORSOCCURRED
	TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
       
    //Verify error array
	if(SettableProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET))
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS,DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	
	if(SupportedProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET))
	{
		if(SettableProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET))
		{
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MAXPENDINGROWS,DBPROPSET_ROWSET, DBPROPSTATUS_OK))
		}
		else
		{
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MAXPENDINGROWS,DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE))
		}
	}
	else
	{
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MAXPENDINGROWS,DBPROPSET_ROWSET, DBPROPSTATUS_NOTSUPPORTED))
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - Bad Col info?
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_32()
{
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set 3 properties                                                   
    SetSettableProperty(DBPROP_CANHOLDROWS);							//OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);						//OK

	//Rowset Properties that look at ColumnIDs
    SetProperty(DBPROP_DEFERRED,		DBPROPSET_ROWSET,(void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_REQUIRED, DBCOLUMN_MAYSORT);    //BADCOLUMN
    SetProperty(DBPROP_CACHEDEFERRED,	DBPROPSET_ROWSET,(void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_REQUIRED, DBCOLUMN_MAYSORT);    //BADCOLUMN
    
    //Property errors - DB_E_ERRORSOCCURRED
    //S_OK or DB_E_ERRORSOCCURRED - depending upon DBPROPFLAGS_COLUMNOK
	TEST2C_(hr = CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset), S_OK, DB_E_ERRORSOCCURRED)
       
    //Verify error array
	if(SettableProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET))
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS,DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	
	//DBPROP_DEFERRED
	if(hr==S_OK)
	{
		TESTC(!(GetPropInfoFlags(DBPROP_DEFERRED, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK));
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_DEFERRED, DBPROPSET_ROWSET, DBPROPSTATUS_OK));
		TESTC(!(GetPropInfoFlags(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK));
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	}
	else
	{
		BOOL fDeferredColumnOk = GetPropInfoFlags(DBPROP_DEFERRED, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK;
		BOOL fCacheDefColumnOk = GetPropInfoFlags(DBPROP_CACHEDEFERRED, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK;
		
		//Verify 
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_DEFERRED,DBPROPSET_ROWSET, fDeferredColumnOk ? DBPROPSTATUS_BADCOLUMN : DBPROPSTATUS_OK));
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CACHEDEFERRED,DBPROPSET_ROWSET, fCacheDefColumnOk ? DBPROPSTATUS_BADCOLUMN : DBPROPSTATUS_OK));
	}	

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_E_ERRORSOCCURRED - All prop in error
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_33()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    SetProperty(DBPROP_IAccessor);								// DBPROPSTATUS_OK);
    SetProperty(DBPROP_QUICKRESTART,DBPROPSET_ROWSET,(void*)VARIANT_TRUE,DBTYPE_BOOL,ULONG_MAX);    // DBPROPSTATUS_BADOPTION);
    SetProperty(DBPROP_IRowsetInfo);    						// DBPROPSTATUS_OK);
    SetProperty(DBPROP_BOOKMARKS);								// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_CACHEDEFERRED);							// DBPROPSTATUS_OK); 
    SetProperty(DBPROP_CANFETCHBACKWARDS);						// DBPROPSTATUS_OK);     				
    SetProperty(DBPROP_CANHOLDROWS);							// DBPROPSTATUS_OK);     				
    SetProperty(DBPROP_CANSCROLLBACKWARDS);						// DBPROPSTATUS_OK);     				
    SetProperty(DBPROP_COMMITPRESERVE);							// DBPROPSTATUS_OK);
    SetProperty(DBPROP_DEFERRED);								// DBPROPSTATUS_OK);     				
    SetProperty(DBPROP_MAXOPENROWS, DBPROPSET_ROWSET, (void*)1, DBTYPE_I4);	
    SetProperty(DBPROP_MAXPENDINGROWS, DBPROPSET_ROWSET, (void*)1, DBTYPE_I4);
    SetProperty(DBPROP_MAYWRITECOLUMN);							// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_MEMORYUSAGE, DBPROPSET_ROWSET, (void*)-1, DBTYPE_I4);    // DBPROPSTATUS_BADVALUE);
    SetProperty(DBPROP_COLUMNRESTRICT);							// DBPROPSTATUS_OK);
    SetProperty(DBPROP_ROWRESTRICT);							// DBPROPSTATUS_OK);
    SetProperty(DBPROP_ORDEREDBOOKMARKS);						// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_LITERALBOOKMARKS);						// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_OTHERINSERT);							// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_OTHERUPDATEDELETE);						// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_OWNINSERT);								// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_OWNUPDATEDELETE);						// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_REENTRANTEVENTS);						// DBPROPSTATUS_OK); 
    SetProperty(DBPROP_REMOVEDELETED);							// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_SERVERCURSOR);							// DBPROPSTATUS_OK);     
    SetProperty(DBPROP_LITERALIDENTITY);						// DBPROPSTATUS_OK);     

    //Call OpenRowset
    //Property errors - DB_E_ERRORSOCCURRED
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)
       
    //Verify error array
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IAccessor,				DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_QUICKRESTART,			DBPROPSET_ROWSET, DBPROPSTATUS_BADOPTION));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IRowsetInfo,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));	
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_BOOKMARKS,				DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CACHEDEFERRED,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));			
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));				
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANSCROLLBACKWARDS,	DBPROPSET_ROWSET, DBPROPSTATUS_OK));				
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_COMMITPRESERVE,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_DEFERRED,				DBPROPSET_ROWSET, DBPROPSTATUS_OK));				
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MAXOPENROWS,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MAXPENDINGROWS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MAYWRITECOLUMN,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));		    
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_MEMORYUSAGE,			DBPROPSET_ROWSET, DBPROPSTATUS_BADVALUE));			
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_COLUMNRESTRICT,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));      
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ROWRESTRICT,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));		
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ORDEREDBOOKMARKS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_LITERALBOOKMARKS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERINSERT,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));				
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERUPDATEDELETE,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));			    
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNINSERT,				DBPROPSET_ROWSET, DBPROPSTATUS_OK));			    
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNUPDATEDELETE,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));			    
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_REENTRANTEVENTS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));		
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_REMOVEDELETED,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));		
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_SERVERCURSOR,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));			    
    TCOMPARE_(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_LITERALIDENTITY,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Error - E_NOINTERFACE - Invalid riid
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_34()
{
	TBEGIN
	IRowset* pIRowset = NULL;

    //Invalid REFIID
    TESTC_(CreateOpenRowset(IID_IConnectionPoint, (IUnknown**)&pIRowset),E_NOINTERFACE)

CLEANUP:
	SAFE_RELEASE(pIRowset);
	FreeProperties();
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Error - E_NOINTERFACE - riid==IID_ILockBytes
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_35()
{
	TBEGIN
	IRowset* pIRowset = NULL;
    
    //IID_ILockBytes - E_NOINTERFACE
    TESTC_(CreateOpenRowset(IID_ILockBytes,(IUnknown**)&pIRowset),E_NOINTERFACE)

CLEANUP:
	SAFE_RELEASE(pIRowset);
	FreeProperties();
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Error - riid== IID_NULL - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_36()
{
	TBEGIN
	IRowset* pIRowset = NULL;

    //IID_NULL - E_NOINTERFACE
    TESTC_(CreateOpenRowset(IID_NULL,(IUnknown**)&pIRowset),E_NOINTERFACE)
	TESTC(pIRowset==NULL)

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Error - DB_SEC_E_PERMISSIONDENIED
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_37()
{
	TBEGIN
	// TO DO:  Add your own code here
	//I don't know how to open a table with a column I don't have permission to?
	return TEST_PASS;
}
// }}


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pTableID - Empty Table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_38()
{
	TBEGIN
    IRowset* pIRowset = NULL;
    CRowset Rowset;
  
    //Set properties           
    SetSettableProperty(DBPROP_CANHOLDROWS);		//OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
    
    //Valid Table, with no rows	- S_OK
    TESTC_(CreateOpenRowset(g_pEmptyTable,IID_IRowset,(IUnknown**)&pIRowset),S_OK)

    //Verify no rows in the returned rowset, (empty table)
    TESTC_PROVIDER(Rowset.CreateRowset(pIRowset)==S_OK);
    TESTC(Rowset.GetTotalRows() == g_pEmptyTable->GetRowsOnCTable())

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pTableID - 1 Row Table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_39()
{
	TBEGIN
    IRowset* pIRowset = NULL;
    CRowset Rowset;
    
    //Set properties           
    SetSettableProperty(DBPROP_CANHOLDROWS);       //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
    
    //Valid Table, with 1 row	- S_OK
	TESTC_(CreateOpenRowset(g_p1RowTable,IID_IRowset,(IUnknown**)&pIRowset),S_OK)

    //Verify 1 row in the returned rowset, (1 row table)
    TESTC_PROVIDER(Rowset.CreateRowset(pIRowset)==S_OK);
    TESTC(Rowset.GetTotalRows() == g_p1RowTable->GetRowsOnCTable())

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Parameters - DBPROP_OPENROWSETSUPPORT
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_40()
{
	TBEGIN
	ULONG_PTR ulValue = 0;

    //DBPROP_OPENROWSETSUPPORT
    if(GetProperty(DBPROP_OPENROWSETSUPPORT, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession, &ulValue))
	{
		//NOTE: DBPROPVAL_ORS_TABLE=0, so this check makes sure that not only one of the 
		//valid defined bits but also inclusive of TABLE=0...

		//Make sure it has valid values (not outside the range)
		TESTC(!(ulValue & ~(DBPROPVAL_ORS_TABLE | 
							DBPROPVAL_ORS_INDEX | 
							DBPROPVAL_ORS_INTEGRATEDINDEX | 
							DBPROPVAL_ORS_STOREDPROC | 
							DBPROPVAL_ORS_HISTOGRAM
							))		);
	}
	else
	{
		//IOpenRowset is a required Level - 0 interface, and support for DBPROP_OPENROWSETSUPPORT
		//really should be supported, so generic consumers know at least Tables can be opened,
		//as well as knowing wither or not StoredProcs, Index, Integrated indexes are supported...
		TWARNING("DBPROP_OPENROWSETSUPPORT is not supported?");
	}
    
CLEANUP:
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pIndexID - Integrated Index Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_41()
{
	TBEGIN

//CLEANUP:
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - All rowset IIDs
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_42()
{
	TBEGIN
	HRESULT hr = S_OK;
    IUnknown* pIUnknown = NULL;

	//Obtain the Rowset IIDs
	ULONG i, cRowsetIIDs = 0;
	INTERFACEMAP* rgRowsetIIDs = NULL;
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cRowsetIIDs, &rgRowsetIIDs));

    //Loop through all rowset IIDs...
	for(i=0; i<cRowsetIIDs; i++)
	{
		//Asking for IID_I* is requesting a rowset that supports this interface
		//This is implicilty like requesting DBPROP_I* ahead of time...
		TEST2C_(hr = CreateOpenRowset(*rgRowsetIIDs[i].pIID, &pIUnknown), S_OK, E_NOINTERFACE);
	
		//Success, very this interface...
		if(hr == S_OK)
		{
			CRowset RowsetA;

			if(!ValidInterface(*rgRowsetIIDs[i].pIID, pIUnknown))
				TERROR(L"Interface Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");

			//Make sure the rowset is valid and contains the correct data...
			//NOTE: Can't really verify rows of data since OpenRowset doesn't 
			//have to return the same order as the SELECT_ORDERBYNUMERIC
			//Since this variation is only interested in "OpenRowset" we will just make
			//sure the rows and columns are the same. (functional rowset)
			TESTC_(RowsetA.CreateRowset(pIUnknown),S_OK);
			TESTC(RowsetA.GetTotalRows() == m_pTable->GetRowsOnCTable());
		}
		else
		{
			//Some providers still don't met the spec and fail IID_IRowsetUpdate,
			//since the property DBPROP_IRowsetUpdate was not set.  This is wrong, and 
			//against the spec, which states that asking for IID_I* is just like implicilty
			//asking for the property.  Test it here, to make sure the provider truely doesn't
			//support this interface, so we don't skip them incorrectly

			//Make sure this is allowed to not be required
			TCOMPARE_(!rgRowsetIIDs[i].fMandatory);

			//Set properties
			SetProperty(rgRowsetIIDs[i].dwPropertyID);
    		hr = CreateOpenRowset(IID_IUnknown, &pIUnknown);
			if(hr != DB_E_ERRORSOCCURRED || !VerifyProperties(hr, m_cPropSets, m_rgPropSets))
				TERROR(L"Property Incorrect for " << GetPropertyName(rgRowsetIIDs[i].dwPropertyID, DBPROPSET_ROWSET) << "\n");
		}
		
		FreeProperties();
	    SAFE_RELEASE(pIUnknown);
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIUnknown);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - All Rowset interface Properties - singularly
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_43()
{
	TBEGIN
	HRESULT hr = S_OK;
    IUnknown* pIUnknown = NULL;
    IUnknown* pIUnkProp = NULL;
     
	//Obtain the Rowset IIDs
	ULONG i, cRowsetIIDs = 0;
	INTERFACEMAP* rgRowsetIIDs = NULL;
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cRowsetIIDs, &rgRowsetIIDs));

    //Loop through all rowset IIDs...
	for(i=0; i<cRowsetIIDs; i++)
	{
		//Set DBPROP_I* is requesting a rowset that supports this interface
		//This is implicilty like requesting IID_I* on the method call...
		SetProperty(rgRowsetIIDs[i].dwPropertyID);
		TEST2C_(hr = CreateOpenRowset(IID_IUnknown, &pIUnknown), S_OK, DB_E_ERRORSOCCURRED);
		FreeProperties();
	
		//Success, very this interface...
		if(hr == S_OK)
		{
			CRowset RowsetA;

			if(QI(pIUnknown, *rgRowsetIIDs[i].pIID, (void**)&pIUnkProp)!=S_OK)
				TERROR(L"Interface Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");

			//Release the IID_IUnknown interface, so were only left with the
			//interface we set the property for (make sure the rowset is useable
			//even after releasing requested interface, crashes quite nicely on 
			//some providers who don't properly refcount sub interfaces)
			SAFE_RELEASE(pIUnknown);
			
			if(!ValidInterface(*rgRowsetIIDs[i].pIID, pIUnkProp))
				TERROR(L"Interface Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");

			//Make sure the rowset is valid and contains th correct data...
			//NOTE: Can't really verify rows of data since OpenRowset doesn't 
			//have to return the same order as the SELECT_ORDERBYNUMERIC
			//Since this variation is only interested in "OpenRowset" we will just make
			//sure the rows and columns are the same. (functional rowset)
			TESTC_(RowsetA.CreateRowset(pIUnkProp),S_OK);
			TESTC(RowsetA.GetTotalRows() == m_pTable->GetRowsOnCTable());
		}
		else
		{
			//Some providers still don't met the spec and fail DBPROP_I*,
			//since the property DBPROP_I* was set and the interface IID_I* was not asked
			//for.  This is wrong, and against the spec, which states that asking for 
			//DBPROP_I* is just like implicilty asking for IID_I*.  Test it here, 
			//to make sure the provider truely doesn't support this interface, 
			//so we don't skip them incorrectly

			//Make sure this is allowed to not be required
			TCOMPARE_(!rgRowsetIIDs[i].fMandatory);

			//Make sure asking for the IID also fails...
    		if(CreateOpenRowset(*rgRowsetIIDs[i].pIID, &pIUnknown)!=E_NOINTERFACE)
				TERROR(L"Property Incorrect for " << GetPropertyName(rgRowsetIIDs[i].dwPropertyID, DBPROPSET_ROWSET) << "\n");
		}
		
	    SAFE_RELEASE(pIUnknown);
	    SAFE_RELEASE(pIUnkProp);
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIUnknown);
    SAFE_RELEASE(pIUnkProp);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - All Rowset interface Properties - combinations
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_44()
{
	TBEGIN
    IUnknown* pIUnknown = NULL;
    IUnknown* pIUnkProp = NULL;
	HRESULT hrOpenRowset = S_OK;
	HRESULT hr = S_OK;
     
	//Obtain the Rowset IIDs
	ULONG iMap, cRowsetMaps = 0;
	INTERFACEMAP* rgRowsetMaps = NULL;
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cRowsetMaps, &rgRowsetMaps));

	//There are a slew of Rowset properties.  Most of the interesting properties are tested
	//throughout the Conformance Tests in conjuction with a few other properties, in the related
	//areas of the properties

	//Most real-world consumers will either "require" the property functionality to even begin
	//their application and will fail if the rowset is not created with that functionality,
	//or more likey will set a slew of "optional" properties and have code that will work with
	//or without the extra optional behavior.  We already have senarios that cover the later
	//later on in this test.  But we don't have enough converage of "required" property combinations.

	//Since required properties will fail to generate a rowset if ANY of the properties fail
	//this is a little trickier to implement.  In light of this we also want to try as many
	//combinations as we can, not just all properties, but many interesting combinations...
	
    //Loop through all rowset IIDs...
	for(iMap=0; iMap<cRowsetMaps; iMap++)
	{
		//Set this property
		if(rgRowsetMaps[iMap].dwPropertyID)
			if(!FindProperty(rgRowsetMaps[iMap].dwPropertyID, DBPROPSET_ROWSET, m_cPropSets, m_rgPropSets))
				SetProperty(rgRowsetMaps[iMap].dwPropertyID, DBPROPSET_ROWSET);

		//Loop through all rowset IIDs...
		for(ULONG iNext=0; iNext<cRowsetMaps; iNext++)
		{
			//Set this property
			if(rgRowsetMaps[iNext].dwPropertyID)
				if(!FindProperty(rgRowsetMaps[iNext].dwPropertyID, DBPROPSET_ROWSET, m_cPropSets, m_rgPropSets))
					SetProperty(rgRowsetMaps[iNext].dwPropertyID, DBPROPSET_ROWSET);
	
			//Try to open the rowset
			//NOTE: We don't check the property if it supported or settable first, since
			//that would mean we would never "try" that senario just becuase the provider
			//says its not there, which in fact it may be, so we will do the attempt first
			//and then verify the result matches correctly with what the provider supports.
		
			//All these properties are set as required, so either every one succeeds, or 
			//it fails if one required property cannot be met.
			TEST2C_(hrOpenRowset = CreateOpenRowset(IID_IUnknown, &pIUnknown), S_OK, DB_E_ERRORSOCCURRED);

			//Make sure that every property that was set, is available on the returned rowset
			for(ULONG iSet=0; iSet<cRowsetMaps; iSet++)
			{
				INTERFACEMAP* pSetMap = &rgRowsetMaps[iSet];
				DBPROP* pProp = NULL;
				FindProperty(pSetMap->dwPropertyID, DBPROPSET_ROWSET, m_cPropSets, m_rgPropSets, &pProp);
				
				if(pProp)
				{
					if(SUCCEEDED(hrOpenRowset))
					{
						//OpenRowset succeeded...
						//See if this interface is available...
						TESTC_(QI(pIUnknown, *pSetMap->pIID, (void**)&pIUnkProp),S_OK);

						//Make sure its a valid interface
						//NOTE: I've tried to actually verify the rowset and data above (after the
						//CreateOpenRowset call) but its too expensive, since their are hundreads
						//of combinations.  I could limit it down to only verifying the rowset
						//data on the outer loop, but actually I already do that in other variations
						//for all properties and all interfaces...
						TESTC(DefaultObjectTesting(pIUnkProp, ROWSET_INTERFACE));
						TESTC(pProp->dwStatus == DBPROPSTATUS_OK);
					}
					else
					{
						//Was this property the reason for the failure
						if(pProp->dwStatus != DBPROPSTATUS_OK)
						{
							//A mandatory property cannot fail
							TESTC(!pSetMap->fMandatory);
							TESTC(pProp->dwOptions == DBPROPOPTIONS_REQUIRED);
							
							switch(pProp->dwStatus)
							{
								case DBPROPSTATUS_NOTSUPPORTED:
									//Make not be supported
									if(SupportedProperty(pProp->dwPropertyID, DBPROPSET_ROWSET))
										TERROR("Property status incorrect for " << GetPropertyName(pSetMap->dwPropertyID, DBPROPSET_ROWSET) << " property is supported, but status indicates it is not: " << GetPropStatusName(pProp->dwStatus));
									break;

								case DBPROPSTATUS_NOTSETTABLE:
									//Must be supported, but readonly
									TESTC(SupportedProperty(pProp->dwPropertyID, DBPROPSET_ROWSET))
									if(SettableProperty(pProp->dwPropertyID, DBPROPSET_ROWSET))
										TERROR("Property status incorrect for " << GetPropertyName(pSetMap->dwPropertyID, DBPROPSET_ROWSET) << " property is settable, but status indicates it is not: " << GetPropStatusName(pProp->dwStatus));
									break;
								
								case DBPROPSTATUS_CONFLICTING:
									//This is an interesting status, and not something
									//that we can simply programmically verify
									TWARNING("Property " << GetPropertyName(pSetMap->dwPropertyID, DBPROPSET_ROWSET) << " was CONFLICTING with some other property, please verify");
									break;

								default:
									//Not a appropiate status for this senario
									TERROR("Property status for " << GetPropertyName(pSetMap->dwPropertyID, DBPROPSET_ROWSET) << " was not a valid status for this senario: " << GetPropStatusName(pProp->dwStatus));
									break;
							};

							//Now Remove this failing property so it doesn't prevent future succeess
							pProp->dwPropertyID	= DBPROP_IRowset;

						}
					}
				}

				SAFE_RELEASE(pIUnkProp);
			}

			SAFE_RELEASE(pIUnknown);
		}

		FreeProperties();
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIUnknown);
    SAFE_RELEASE(pIUnkProp);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppRowset - NULL && some error properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_45()
{
	TBEGIN
	//Set 3 properties
	SetProperty(DBPROP_CANHOLDROWS);       //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
    
	//Find out if IStorage is really supported
    BOOL fIStorageSettable = SettableProperty(DBPROP_IStorage, DBPROPSET_ROWSET);
	if(!fIStorageSettable)
		SetProperty(DBPROP_IStorage);			        
	
    //NULL Rowset and property errors - DB_E_ERRORSOCCURRED
    TESTC_(CreateOpenRowset(IID_IRowset, NULL), fIStorageSettable ? S_OK : DB_E_ERRORSOCCURRED)
       
    //Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,      DBPROPSET_ROWSET, DBPROPSTATUS_OK))
    if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS,DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	
	if(!fIStorageSettable)
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IStorage,       DBPROPSET_ROWSET, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	FreeProperties();
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppRowset - NULL && no error properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_46()
{
	TBEGIN
    //Set 3 properties
    SetProperty(DBPROP_CANHOLDROWS);        //OK

    //NULL Rowset and no property errors - S_OK
    TESTC_(CreateOpenRowset(IID_IRowset,NULL),S_OK)

    //NULL Rowset and warnings - DB_S_ERRORSOCCURRED
	//Wrong Property Set - as OPTIONAL
	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_DATASOURCEINFO, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);  
    TESTC_(CreateOpenRowset(IID_IRowset,NULL),DB_S_ERRORSOCCURRED)
		
CLEANUP:
	FreeProperties();
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Sequence - OpenRowset Twice (same table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_47()
{
	TBEGIN
    IRowset* pIRowset  = NULL;
    IRowset* pIRowset2 = NULL;
 
    //Set properties
    SetSettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET);
	SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
    
	//Use CursorEngine ( if requested )
	if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
		SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET);
	
    //Call OpenRowset #1
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)
	FreeProperties();

    //Set 3 properties
    SetSettableProperty(DBPROP_OWNUPDATEDELETE, DBPROPSET_ROWSET);
    SetSettableProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET);
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET);
    
    //Call OpenRowset #2
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset2),S_OK)
    
    //Verify different rowset pointers
    TESTC(pIRowset != pIRowset2)


CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIRowset2);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Sequence - OpenRowset Twice (diff table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_48()
{
	TBEGIN
    IRowset* pIRowset  = NULL;
    IRowset* pIRowset2 = NULL;

    //Set properties
    SetSettableProperty(DBPROP_BOOKMARKS);          //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);  //OK
    
    //Call OpenRowset #1, using Table#1
    TESTC_(CreateOpenRowset(m_pTable,IID_IRowset,(IUnknown**)&pIRowset),S_OK)
	FreeProperties();

    //Set properties
    SetSettableProperty(DBPROP_OWNUPDATEDELETE);    //OK
    SetSettableProperty(DBPROP_BOOKMARKS);	        //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);  //OK
    
    //Call OpenRowset #2, using table #2
    TESTC_(CreateOpenRowset(g_p1RowTable,IID_IRowset,(IUnknown**)&pIRowset2),S_OK)
    
    //Verify different rowset pointers
    TESTC(pIRowset != pIRowset2)


CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIRowset2);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Sequence - OpenRowset Twice while altering table
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_49()
{
	TBEGIN
    HRESULT	 ExpectedHr	   = S_OK;
	ULONG	 ulRowCnt  = 1;
    HROW	 hRow	   = NULL;

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession))
	{
		ExpectedHr	 = DB_E_ERRORSOCCURRED;
		ulRowCnt = 0;
	}

	CRowsetChange RowsetChange;
    CRowsetChange RowsetChange2;
	IRowsetChange* pIRowsetChange  = NULL;
    IRowsetChange* pIRowsetChange2 = NULL;

	//Should not attempt to use this interface, if using strict and not required
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange));
	
    //Set properties
    SetProperty(DBPROP_IRowsetChange);

    //Call OpenRowset #1
	//Must support IRowsetChange otherwise we are done
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowsetChange,(IUnknown**)&pIRowsetChange)==S_OK);
    
	//Create a rowset
	TESTC_PROVIDER(RowsetChange.CreateRowset(pIRowsetChange)==S_OK);
	TESTC_(RowsetChange.GetRow(FIRST_ROW,&hRow),S_OK)
    TESTC_(RowsetChange.DeleteRow(hRow),ExpectedHr)
	RowsetChange.pTable()->SubtractRow(ulRowCnt);
    TESTC_(RowsetChange.ReleaseRows(hRow),S_OK)

    //Verify this rowset equals the backend table, immediate mode
	if(RowsetChange.GetProperty(DBPROP_REMOVEDELETED))
		COMPC(RowsetChange.GetTotalRows(), RowsetChange.pTable()->GetRowsOnCTable())
	else
		COMPC(RowsetChange.GetTotalRows()-ulRowCnt , RowsetChange.pTable()->GetRowsOnCTable())
	
	FreeProperties();

    //Set properties
    SetProperty(DBPROP_IRowsetChange);
    
    //Call OpenRowset #2
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowsetChange2)==S_OK);
    TESTC_PROVIDER(RowsetChange2.CreateRowset(pIRowsetChange2)==S_OK);

    //Verify different rowset pointers
    //Verify this rowset does equal the backend table, no changes yet
    TESTC(pIRowsetChange != pIRowsetChange2)
	COMPC(RowsetChange2.GetTotalRows(), RowsetChange2.pTable()->GetRowsOnCTable())

	//Verify rowsets are equal, the delete was in immediate mode
	if(RowsetChange.GetProperty(DBPROP_REMOVEDELETED))
		TESTC(RowsetChange2.CompareRowset(pIRowsetChange))
	else
		COMPC(RowsetChange.GetTotalRows()-ulRowCnt , RowsetChange2.GetTotalRows())

CLEANUP:
	FreeProperties();
    RowsetChange.ReleaseRows(hRow);
    SAFE_RELEASE(pIRowsetChange);
    SAFE_RELEASE(pIRowsetChange2);
	TableInsert(ONE_ROW);	//Adjust the table
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Properties in error are still set?
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_50()
{
	TBEGIN
    IRowset* pIRowset   = NULL;
    IRowset* pIRowset2  = NULL;

    //Set 3 properties
    SetSettableProperty(DBPROP_IRowsetUpdate);			//OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);		//OK
    SetProperty(DBPROP_GROUPBY);						//NOTSUPPORTED (wrong PROPSET)
    
    //Call OpenRowset #1, with properties
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED)

    //Verify error array
	if(SettableProperty(DBPROP_IRowsetUpdate, DBPROPSET_ROWSET))
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IRowsetUpdate, DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	if(SettableProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET))
	    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, DBPROPSTATUS_OK))
    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_GROUPBY, DBPROPSET_ROWSET, DBPROPSTATUS_NOTSUPPORTED))
    FreeProperties();
	    
	//Call OpenRowset #2, with no properties, should not be set from last time?
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset2),S_OK);
   	TESTC(pIRowset2!=NULL);
	
	//Just because IID_IRowsetUpdate was set last time, shouldn't be able to query for it now
    TESTC_(QI(pIRowset2,IID_IRowsetUpdate),E_NOINTERFACE);

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIRowset2);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Sequence - Change backend table, refetch rows, verify
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_51()
{
	TBEGIN
    IRowset* pIRowset  = NULL;

    //Set properties
    SetSettableProperty(DBPROP_BOOKMARKS);          //OK
    SetSettableProperty(DBPROP_CANFETCHBACKWARDS);  //OK
    
    //Call OpenRowset #1
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)

    //TODO
    //Alter the backend table and verify

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Sequence - IColumnsInfo without prepare
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_52()
{
	TBEGIN
    IRowset* pIRowset  = NULL;
    IColumnsInfo* pIColumnsInfo = NULL;

    DBORDINAL cColumns = 0;
    DBCOLUMNINFO* rgInfo = NULL;
    WCHAR* rgStringsBuffer = NULL;

    //Call OpenRowset
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)
    TESTC(pIRowset!=NULL);
		
    //Obtain IRowsetInfo interface [Mandatory]
    TESTC_(QI(pIRowset,IID_IColumnsInfo,(void**)&pIColumnsInfo),S_OK)

    //Call IColumnsInfo, make sure not DB_E_NOTPREPARED
    TESTC_(pIColumnsInfo->GetColumnInfo(&cColumns,&rgInfo,&rgStringsBuffer),S_OK)
    TESTC(cColumns!=0 && rgInfo!=NULL && rgStringsBuffer!=NULL)

    //Just as a verify the correct number of columns though IColumnsInfo
	//Accually the Privlib does not deal with a bookmark column,
	if(rgInfo[0].iOrdinal==0)
		TESTC(cColumns-1 == m_pTable->CountColumnsOnTable())
	else
		TESTC(cColumns == m_pTable->CountColumnsOnTable())

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIColumnsInfo);

    PROVIDER_FREE(rgInfo);
    PROVIDER_FREE(rgStringsBuffer);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Sequence - IRowsetInfo::GetSpecifications with session object
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_53()
{
	TBEGIN
    HRESULT hr = S_OK;

	IRowset* pIRowset  = NULL;
    IRowsetInfo* pIRowsetInfo = NULL;
    IOpenRowset* pIOpenRowset = NULL;

    //Call OpenRowset
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)
    TESTC(pIRowset!=NULL);
    
    //Obtain IRowsetInfo interface [Mandatory]
    TESTC_(QI(pIRowset,IID_IRowsetInfo,(void**)&pIRowsetInfo),S_OK)
    
    //Call IRowsetInfo::GetSpecification
	hr = pIRowsetInfo->GetSpecification(IID_IOpenRowset,(IUnknown **)&pIOpenRowset);

	//The OLEDB Spec allows GetSpecification to return S_FALSE or S_OK
	if(hr==S_OK)
	{
		//Succeeded
		TESTC(pIOpenRowset != NULL);
	}
	else if(hr==S_FALSE)
	{
		//S_FALSE means an "inferior" provder and we should let them know it
		TESTC(pIOpenRowset==NULL);
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}
	else
	{
		//GetSpecification failed!
		TESTC(pIOpenRowset==NULL);
		TESTC_(hr, S_OK);
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    SAFE_RELEASE(pIRowsetInfo);
    SAFE_RELEASE(pIOpenRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Related - 2 rowsets open with OpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_54()
{
	TBEGIN
    HRESULT	 ExpectedHr				= S_OK;
	ULONG	 ulRowCnt			= 1;
    IRowset* pIRowset			= NULL;
	HROW	 rghRow[THREE_ROWS] = {NULL,NULL,NULL};

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO,g_pIDBCreateSession))
	{
		ExpectedHr	 = DB_E_ERRORSOCCURRED;
		ulRowCnt = 0;
	}

    CRowsetChange RowsetChange;

	//Should be doing this variation is not a required interface and Strict
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange));

    //Generate 1 rowset by ICommand/Execute
    //If can't set this property, were done
	RowsetChange.SetProperty(DBPROP_CANHOLDROWS);
	RowsetChange.SetProperty(DBPROP_IRowsetChange);
	TESTC_PROVIDER(RowsetChange.CreateRowset(SELECT_ALLFROMTBL)==S_OK);

    //Generate 1 rowset by OpenRowset
	SetSettableProperty(DBPROP_CANHOLDROWS);
	SetSettableProperty(DBPROP_IRowsetChange);
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)
        
    //Verify both rowsets are equal...
    TESTC(RowsetChange.CompareRowset(pIRowset))

	//Make sure all operations work
    TESTC_(RowsetChange.GetRow(FIRST_ROW,TWO_ROWS,rghRow),S_OK)
    TESTC_(RowsetChange.DeleteRow(rghRow[ROW_ONE]),ExpectedHr)
	RowsetChange.pTable()->SubtractRow(ulRowCnt);
    TESTC_(RowsetChange.ModifyRow(rghRow[ROW_TWO]),S_OK)
    TESTC_(RowsetChange.InsertRow(&rghRow[ROW_THREE]),S_OK)
	RowsetChange.pTable()->AddRow();

CLEANUP:
    FreeProperties();
    RowsetChange.ReleaseRows(THREE_ROWS,rghRow);
	SAFE_RELEASE(pIRowset);
	TableInsert(ONE_ROW);	//Adjust the table
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Related - 1 Rowset with Execute [select all], 1 with OpenRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_55()
{
	TBEGIN
    IRowset* pIRowset  = NULL;
    CRowset ExecuteRowset;

	//Generate 1 rowset by ICommand/Execute
    TESTC_PROVIDER(ExecuteRowset.CreateRowset(SELECT_ALLFROMTBL)==S_OK);

    //Generate 1 rowset by OpenRowset, even though command object is not SELECT*
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)
        
    //Verify both rowsets are equal...
    TESTC(ExecuteRowset.CompareRowset(pIRowset))

CLEANUP:
    FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Properties - Verify implied properties
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_56()
{
	TBEGIN
    IRowset* pIRowset = NULL;
    CRowset Rowset;

	//Should not attempt to use this interface, if using strict and not required
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetUpdate));

    //Verify implied properties
	//Not all providers will support IID_IRowsetUpdate
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowsetUpdate,(IUnknown**)&pIRowset)==S_OK);
	
	//Create a rowset
	TESTC_(Rowset.CreateRowset(pIRowset),S_OK);

    //Verify IID_IRowsetChange is also implied?
    TESTC(Rowset.GetProperty(DBPROP_IRowsetChange));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Properties - Buffered Mode
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_57()
{
	TBEGIN
    HRESULT	 ExpectedHr	  = S_OK;
	ULONG	 ulRowCnt = 1;
    IRowset* pIRowset = NULL;
    HROW	 hRow	  = DB_NULL_HROW;

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession))
	{
		ExpectedHr	 = DB_E_ERRORSOCCURRED;
		ulRowCnt = 0;
	}

    CRowsetUpdate RowsetUpdate;

	//Should be doing this variation is not a required interface and Strict
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange));
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetUpdate));

    //Set 1 property
    //If IRowsetUpdate is not supported, were done
	SetProperty(DBPROP_IRowsetUpdate);
    SetProperty(DBPROP_IRowsetChange);
    SetProperty(DBPROP_CANHOLDROWS);

    //Call OpenRowset
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset)==S_OK);
    TESTC_PROVIDER(RowsetUpdate.CreateRowset(pIRowset)==S_OK);
    
    //Verify in buffered mode        
    TESTC_(RowsetUpdate.GetRow(FIRST_ROW,&hRow),S_OK)
    TESTC_(RowsetUpdate.DeleteRow(hRow),S_OK)
    TESTC_(RowsetUpdate.GetPendingRows(ONE_ROW),S_OK)

	//Verify rowset has 1 less row than the backend table, 
	//Rowset has 1 row deleted, but the table shouldn't see it 
	//since Update has not been called 
	if(RowsetUpdate.GetProperty(DBPROP_REMOVEDELETED))
		COMPC(RowsetUpdate.GetTotalRows()+ulRowCnt, RowsetUpdate.pTable()->GetRowsOnCTable())
	else
		COMPC(RowsetUpdate.GetTotalRows(), RowsetUpdate.pTable()->GetRowsOnCTable())

	//Update
    TESTC_(RowsetUpdate.UpdateAll(),ExpectedHr)
	RowsetUpdate.pTable()->SubtractRow(ulRowCnt);

	//Verify rowset does equal backend table, update occurred, a row was deleted
	//As long as REMOVEDELETED is on, otherwise the row still exists in the rowset
	//its just marked as deleted
	if(RowsetUpdate.GetProperty(DBPROP_REMOVEDELETED))
		COMPC(RowsetUpdate.GetTotalRows(), RowsetUpdate.pTable()->GetRowsOnCTable())
	else
		COMPC(RowsetUpdate.GetTotalRows()-ulRowCnt, RowsetUpdate.pTable()->GetRowsOnCTable())

CLEANUP:
	FreeProperties();
    RowsetUpdate.ReleaseRows(hRow);
    SAFE_RELEASE(pIRowset);
	TableInsert(ONE_ROW);	//Adjust the table
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Properties - Static - Cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_58()
{
	TBEGIN
    IRowset* pIRowset  = NULL;
    HROW hRow = DB_NULL_HROW;
    CRowset Rowset;

    //Call OpenRowset #1, (static-cursor)
	SetProperty(DBPROP_CANFETCHBACKWARDS);
	
    //Create a open rowset
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset)==S_OK);
    TESTC_(Rowset.CreateRowset(pIRowset),S_OK);
    
	//Get the last row
	TESTC_(Rowset.GetRow(Rowset.GetTotalRows(),&hRow),S_OK)
    TESTC_(Rowset.ReleaseRows(hRow),S_OK)

    //Verify rowset matches backend
	COMPC(Rowset.GetTotalRows(), Rowset.pTable()->GetRowsOnCTable());

CLEANUP:
	FreeProperties();
    Rowset.ReleaseRows(hRow);
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Properties - Forward Only -  Cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_59()
{
	TBEGIN
    IRowset* pIRowset  = NULL;
    HROW hRow = DB_NULL_HROW;
    CRowset Rowset;

    //Call OpenRowset #1, (Forward-Only-cursor)
    TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK)
    TESTC_(Rowset.CreateRowset(pIRowset),S_OK);
    
	//Get the last row
	TESTC_(Rowset.GetRow(Rowset.GetTotalRows(),&hRow),S_OK)
    TESTC_(Rowset.ReleaseRows(hRow),S_OK)

    //Verify rowset matches backend, immediate mode
	COMPC(Rowset.GetTotalRows(), Rowset.pTable()->GetRowsOnCTable());

CLEANUP:
	FreeProperties();
    Rowset.ReleaseRows(hRow);
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Properties - KeySet - Cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_60()
{
	TBEGIN
    HRESULT		ExpectedHr			= S_OK;
	ULONG		ulRowCnt		= 1;
	DBROWSTATUS	rgExpStatus[1]	= {DBROWSTATUS_S_OK};

    HROW hRow = DB_NULL_HROW;
	DBROWSTATUS rgRowStatus[1];

	//Check to see if the DSO is ReadOnly
	if(GetProperty(DBPROP_DATASOURCEREADONLY, DBPROPSET_DATASOURCEINFO,g_pIDBCreateSession))
	{
		ExpectedHr = DB_E_ERRORSOCCURRED;
		rgExpStatus[ROW_ONE] = DBROWSTATUS_E_PERMISSIONDENIED;
		ulRowCnt = 0;
	}

    CRowsetChange RowsetChange;
    IRowsetChange* pIRowsetChange  = NULL;

	//Should not attempt to use this interface, if using strict and not required
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange));

    //Call OpenRowset #1, (KeySet-cursor)
	SetProperty(DBPROP_IRowsetChange);

    //Create a open rowset
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowsetChange,(IUnknown**)&pIRowsetChange)==S_OK)
    TESTC_PROVIDER(RowsetChange.CreateRowset(pIRowsetChange)==S_OK);
    
	//Get the first row
	TESTC_(RowsetChange.GetRow(FIRST_ROW,&hRow),S_OK)
    //Delete that row
	TESTC_(RowsetChange.DeleteRow(ONE_ROW, &hRow, rgRowStatus),ExpectedHr)
	TESTC(rgRowStatus[ROW_ONE]==rgExpStatus[ROW_ONE]);
	RowsetChange.pTable()->SubtractRow(ulRowCnt);

	//Release the row
	TESTC_(RowsetChange.ReleaseRows(hRow),S_OK)

    //Should not equal backend, if remove deleted
	if(RowsetChange.GetProperty(DBPROP_REMOVEDELETED))
		COMPC(RowsetChange.GetTotalRows(), RowsetChange.pTable()->GetRowsOnCTable())
	else
		COMPC(RowsetChange.GetTotalRows()-ulRowCnt, RowsetChange.pTable()->GetRowsOnCTable())

CLEANUP:
	FreeProperties();
    RowsetChange.ReleaseRows(hRow);
    SAFE_RELEASE(pIRowsetChange);
	TableInsert(ONE_ROW);	//Adjust the table
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Properties - Dynamic - Cursor
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_61()
{
	TBEGIN
    HROW hRow = DB_NULL_HROW;
    CRowsetChange RowsetChange;
    IRowsetChange* pIRowsetChange  = NULL;

	//Should not attempt to use this interface, if using strict and not required
	TESTC_PROVIDER(IsUsableInterface(ROWSET_INTERFACE, IID_IRowsetChange));

    //Call OpenRowset #1, (dynamic-cursor)
	SetProperty(DBPROP_IRowsetChange);
	SetProperty(DBPROP_OTHERINSERT);
	
    //Create a open rowset
    TESTC_PROVIDER(CreateOpenRowset(IID_IRowsetChange,(IUnknown**)&pIRowsetChange)==S_OK)
    TESTC_PROVIDER(RowsetChange.CreateRowset(pIRowsetChange)==S_OK);
    
	//Get that row
	TESTC_(RowsetChange.GetRow(RowsetChange.GetTotalRows(),&hRow),S_OK)
    //Delete that row
	TESTC_(RowsetChange.DeleteRow(hRow),S_OK)
	RowsetChange.pTable()->SubtractRow();
    TESTC_(RowsetChange.ReleaseRows(hRow),S_OK)

    //Verify rowset matches backend, dynamic cursor, immediate mode
	COMPC(RowsetChange.GetTotalRows(), RowsetChange.pTable()->GetRowsOnCTable())

CLEANUP:
	FreeProperties();
    RowsetChange.ReleaseRows(hRow);
    SAFE_RELEASE(pIRowsetChange);
	TableInsert(ONE_ROW);	//Adjust the table
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[NULL, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_62()
{
	TBEGIN
    IRowset* pIRowset = INVALID(IRowset*);
    
    TESTC_(pIOpenRowset()->OpenRowset(NULL,NULL,NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),E_INVALIDARG)
    TESTC(pIRowset==NULL)
     
CLEANUP:
    if(pIRowset!=INVALID(IRowset*)) SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[NULL, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_63()
{
	TBEGIN
    IRowset* pIRowset = NULL;
    
    //Should require a TableID since the IndexID is actually wrong      
	//Pass an Invalid INDEXID, even if indexes are supported this should fail...
	TESTC_(pIOpenRowset()->OpenRowset(NULL, NULL, &m_pTable->GetTableID(), IID_IRowset,0,NULL,(IUnknown**)&pIRowset), DB_E_NOINDEX);
	TESTC(pIRowset==NULL)
     
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[valid, NULL]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_64()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)
    TESTC(ValidInterface(IID_IRowset,pIRowset))
     
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[valid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_65()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    //DB_E_NOINDEX - the index specified is not valid on the Table
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),&VALID_INDEXID,IID_IRowset,0,NULL,(IUnknown**)&pIRowset), DB_E_NOINDEX);
    TESTC(pIRowset==NULL);
     
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[valid, invalid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_66()
{
	TBEGIN
    IRowset* pIRowset = NULL;
       
    TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),&INVALID_INDEXID,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),DB_E_NOINDEX)
    TESTC(pIRowset==NULL) 

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[invalid, valid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_67()
{
	TBEGIN
    IRowset* pIRowset = NULL;

	//Since both are passed in we can actual get either NOINDEX or NOTABLE
	//DB_E_NOINDEX - for a provider that does not support opening indexes with IOpenRowset
	//DB_E_NOTABLE - for a provider that does support opening indexes, and tries to validate the tablename first
    TEST2C_(pIOpenRowset()->OpenRowset(NULL, &INVALID_TABLEID, &VALID_INDEXID, IID_IRowset,0,NULL,(IUnknown**)&pIRowset), DB_E_NOINDEX, DB_E_NOTABLE);
	TESTC(pIRowset==NULL) 

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Table[NULL, invalid]
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_68()
{
	TBEGIN
    IRowset* pIRowset = NULL;

	TESTC_(pIOpenRowset()->OpenRowset(NULL, NULL, &INVALID_INDEXID, IID_IRowset,0,NULL,(IUnknown**)&pIRowset), DB_E_NOINDEX)
    TESTC(pIRowset==NULL) 

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Long - Invalid TableName
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_69()
{
	TBEGIN
    IRowset* pIRowset = NULL;
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );

	//Make a large unicode string (MAX_FNAME)
	WCHAR wszBuffer[_MAX_FNAME+1];
	LocaleInfo.MakeUnicodeIntlString(wszBuffer, _MAX_FNAME);

	//Make it larger than MAX_PATH for those providers dealing with files...
	WCHAR wszLargeBuffer[MAX_QUERY_LEN+1];
	LocaleInfo.MakeUnicodeIntlString(wszLargeBuffer, MAX_QUERY_LEN);
	
    DBID TableID;
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = wszBuffer;

	//Verify that garbage for a TableID doesn't work
	TESTC_(pIOpenRowset()->OpenRowset(NULL, &INVALID_TABLEID, NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),DB_E_NOTABLE)
    TESTC(pIRowset==NULL) 

	//Verify that a TableID <= MAX_FNAME fails gracfully
	TESTC_(pIOpenRowset()->OpenRowset(NULL,&TableID,NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),DB_E_NOTABLE)
    TESTC(pIRowset==NULL) 
		
	//TODO - this crashes the oracle server and blocks automation, waiting for fix...
	TESTC_PROVIDER(FALSE);

	//Verify that a TableID > MAX_FNAME (a non-existent table) also doesn't work
	TableID.uName.pwszName = wszLargeBuffer;
	TESTC_(pIOpenRowset()->OpenRowset(NULL,&TableID,NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),DB_E_NOTABLE)
    TESTC(pIRowset==NULL) 

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Boundary - Empty TableName
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_70()
{
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    DBID TableID;
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = L"";
	
	//Verify that Empty String for a table name returns DB_E_NOTABLE
	//Some Providers might actually allow Empty/NULL tablename, (ProviderWriter),
	//Since the spec allows this, we either verify S_OK or DB_E_NOTABLE
	TEST2C_(hr = pIOpenRowset()->OpenRowset(NULL,&TableID,NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset), S_OK, DB_E_NOTABLE);
    
	if(hr==S_OK)
	{
		TESTC(pIRowset!=NULL);
	}
	else
	{
		TESTC(pIRowset==NULL);
	}
	
CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc Boundary - pwszName==NULL TableName
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_71()
{
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    DBID TableID;
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = NULL;
	
    //Set properties
	SetProperty(DBPROP_IRowset,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);	
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);

	//Verify that Empty String for a table name returns DB_E_NOTABLE
	//Some Providers might actually allow Empty/NULL tablename, (ProviderWriter),
	//Since the spec allows this, we either verify S_OK or DB_E_NOTABLE
	TEST2C_(hr = pIOpenRowset()->OpenRowset(NULL,&TableID,NULL,IID_IRowset,m_cPropSets,m_rgPropSets,(IUnknown**)&pIRowset), S_OK, DB_E_NOTABLE);
    
	if(hr==S_OK)
	{
		TESTC(pIRowset!=NULL);
	}
	else
	{
		TESTC(pIRowset==NULL);
	}
	
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc Boundary - TableName starting with SQL command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_72()
{
	TBEGIN
    IRowset* pIRowset = NULL;

    DBID TableID;
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = L"updateable_TCOpenRowset_Variation_72"; 
	
    //Set properties
	SetProperty(DBPROP_IRowset,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);	
	SetProperty(DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);

	//Verify returns DB_E_NOTABLE
	TESTC_(pIOpenRowset()->OpenRowset(NULL,&TableID,NULL,IID_IRowset, m_cPropSets, m_rgPropSets ,(IUnknown**)&pIRowset),DB_E_NOTABLE)
    TESTC(pIRowset==NULL) 

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc Boundary - TableName same as SQL command
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_73()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    DBID TableID;
	TableID.eKind = DBKIND_NAME;
	TableID.uName.pwszName = L"select"; 
	
    //Set properties
	SetProperty(DBPROP_IRowset,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);	
	SetProperty(DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL);

	//Verify returns DB_E_NOTABLE
	TESTC_(pIOpenRowset()->OpenRowset(NULL,&TableID,NULL,IID_IRowset,m_cPropSets,m_rgPropSets,(IUnknown**)&pIRowset),DB_E_NOTABLE)
    TESTC(pIRowset==NULL) 

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc Boundary - TableName - Fully Qualified
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCOpenRowset::Variation_74()
{
	TBEGIN
    IRowset* pIRowset = NULL;
	DBID TableID = m_pTable->GetTableID();
	HROW hRow = NULL;
	DBBINDING* pBinding = NULL;
	BOOL bFound = FALSE;
	HRESULT hr = S_OK;

	WCHAR* pwszQualTableName = NULL;
	WCHAR* pwszCatalogName = NULL;
	WCHAR* pwszSchemaName = NULL;
	WCHAR* pwszTableName = NULL;


	//Obtain Schema TABLES Rowset
	//We don't want to put any restrictions, since its not required to support restrictions...
	//NOTE: SchemaRowsets are not required to be supported, so we allow E_NOINTERFACE.  But we don't 
	//allow E_INVALIDARG since if you support schema rowsets, TABLES, COLUMNS, and PROVIDER TYPES are
	//required.
	CRowset Rowset;
	Rowset.SetProperty(DBPROP_CANHOLDROWS);
	TEST2C_(hr = Rowset.CreateRowset(SELECT_DBSCHEMA_TABLE, IID_IRowset, m_pTable, DBACCESSOR_ROWDATA, DBPART_ALL, ALL_COLS_BOUND),S_OK,E_NOINTERFACE);
	TESTC_PROVIDER(hr==S_OK);

	//Try to find the specified row with this table...
	while(hr = Rowset.GetNextRows(&hRow)==S_OK)
	{
		//GetData for this row
		TESTC_(hr = Rowset.GetRowData(hRow, &Rowset.m_pData),S_OK);

		//TABLE_NAME column
		DBBINDING* pCatalogBinding	= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[1] : &Rowset.m_rgBinding[0];
		DBBINDING* pSchemaBinding	= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[2] : &Rowset.m_rgBinding[1];
		DBBINDING* pTableBinding	= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[3] : &Rowset.m_rgBinding[2];
		DBBINDING* pTypeBinding		= Rowset.m_rgBinding[0].iOrdinal == 0 ? &Rowset.m_rgBinding[4] : &Rowset.m_rgBinding[3];

		//See if this is our table...
		//See if this contains a tablename...
		if(STATUS_BINDING(*pTableBinding, Rowset.m_pData)==DBSTATUS_S_OK)
		{
			//TableName
			pwszTableName = (WCHAR*)&VALUE_BINDING(*pTableBinding, Rowset.m_pData);

			TableID = m_pTable->GetTableID();
			if(wcscmp(TableID.uName.pwszName, pwszTableName)==0)
			{
				//Catalog Name
				if(STATUS_BINDING(*pCatalogBinding, Rowset.m_pData)==DBSTATUS_S_OK)
					pwszCatalogName = (WCHAR*)&VALUE_BINDING(*pCatalogBinding, Rowset.m_pData);
				//Schema Name
				if(STATUS_BINDING(*pSchemaBinding, Rowset.m_pData)==DBSTATUS_S_OK)
					pwszSchemaName = (WCHAR*)&VALUE_BINDING(*pSchemaBinding, Rowset.m_pData);
				
				//Construct a fully Qualified TableName...
				TESTC_(hr = m_pTable->GetQualifiedName(pwszCatalogName, pwszSchemaName, pwszTableName, &pwszQualTableName),S_OK);
				TableID.uName.pwszName = pwszQualTableName;

				//IOpenRowset
				SetSupportedProperty(DBPROP_ACCESSORDER, DBPROPSET_ROWSET, (void*)DBPROPVAL_AO_RANDOM, DBTYPE_I4);
				TESTC_(hr = pIOpenRowset()->OpenRowset(NULL, &TableID, NULL, IID_IRowset, m_cPropSets, m_rgPropSets, (IUnknown**)&pIRowset),S_OK);

				//Do some default testing
				TESTC(DefaultObjectTesting(pIRowset, ROWSET_INTERFACE))
			}
		}

		//Release this row...	
		Rowset.ReleaseRows(hRow);
	}

    //We should have found our table in the SchemaTables rowset
	TESTC(pwszQualTableName != NULL)

CLEANUP:
    Rowset.ReleaseRows(hRow);
	SAFE_RELEASE(pIRowset);
	SAFE_FREE(pwszQualTableName);
	FreeProperties();
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Boundary - TableName - Quoted
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCOpenRowset::Variation_75()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	WCHAR* pwszQuotedName = NULL;
	DBID TableID = m_pTable->GetTableID();

	//Fully Quoted (LiteralInfo) tablename
	TESTC_(m_pTable->GetQuotedName(TableID.uName.pwszName, &pwszQuotedName),S_OK);
	TableID.uName.pwszName = pwszQuotedName;

	//IOpenRowset
    TESTC_(pIOpenRowset()->OpenRowset(NULL, &TableID, NULL, IID_IRowset, 0, NULL, (IUnknown**)&pIRowset), S_OK);
    TESTC(ValidInterface(IID_IRowset, pIRowset))
     
CLEANUP:
    SAFE_RELEASE(pIRowset);
	SAFE_FREE(pwszQuotedName);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCOpenRowset::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(COpenRowset::Terminate());
}	// }}
// }}
// }}

 
// {{ TCW_TC_PROTOTYPE(TCExtendedErrors)
//*-----------------------------------------------------------------------
//| Test Case:		TCExtendedErrors - Extended Errors
//|	Created:			07/05/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCExtendedErrors::Init()
{	
	return COpenRowset::Init();
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Valid IOpenRowset calls with previous error object existing.
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_1()
{
	TBEGIN
	IRowset* pIRowset = NULL;

	//For the method of the interface, first create an error object on
	//the current thread, then try get S_OK from the IOpenRowset method.
	//We then check extended errors to verify nothing is set since an 
	//error object shouldn't exist following a successful call.

	TESTC(m_pExtError->CauseError())
	TESTC_(m_hr=(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset)),S_OK)
	//Do extended check following 
	TESTC(m_pExtError->ValidateExtended(m_hr, pIOpenRowset(), IID_IOpenRowset, LONGSTRING(__FILE__), __LINE__))

	TESTC(ValidInterface(IID_IRowset, pIRowset))
		
	
CLEANUP:
	SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Invalid IOpenRowset calls with previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_2()
{
	TBEGIN
	IRowset* pIRowset = INVALID(IRowset*);
        
    //For the method of the interface, first create an error object on
	//the current thread, then try get a failure from the IOpenRowset method.
	//We then check extended errors to verify the right extended error behavior.
	
	TESTC(m_pExtError->CauseError())

	TESTC_(m_hr=(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,5,NULL,(IUnknown**)&pIRowset)),E_INVALIDARG)
 	//XCHECK(m_pIOpenRowset, IID_IOpenRowset, m_hr)	
	TESTC(m_pExtError->ValidateExtended(m_hr, pIOpenRowset(), IID_IOpenRowset, LONGSTRING(__FILE__), __LINE__))
    TESTC(pIRowset==NULL)

CLEANUP:
    if(pIRowset!=INVALID(IRowset*)) SAFE_RELEASE(pIRowset);
    TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Invalid IOpenRowset calls with no previous error object existing
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCExtendedErrors::Variation_3()
{
	TBEGIN
	IRowset* pIRowset = NULL;
	
	//For the method of the interface, with no error object on
	//the current thread, try get a failure from the IOpenRowset method.
	//We then check extended errors to verify the right extended error behavior.
  
    //Set 3 properties
    SetProperty(DBPROP_IAccessor,	DBPROPSET_ROWSET,(void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IRowsetInfo,	DBPROPSET_ROWSET,(void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    SetProperty(DBPROP_IColumnsInfo,DBPROPSET_ROWSET,(void*)VARIANT_FALSE); //NOTSETTABLE - Only to default value
    
    TESTC_(m_hr=(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,m_cPropSets,m_rgPropSets,(IUnknown**)&pIRowset)),DB_E_ERRORSOCCURRED)

	TESTC(m_pExtError->ValidateExtended(m_hr, pIOpenRowset(), IID_IOpenRowset, LONGSTRING(__FILE__), __LINE__))
    TESTC(pIRowset==NULL)

	//Verify property error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IAccessor,   DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE))
    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IRowsetInfo, DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE))
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IColumnsInfo,DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE))
 
CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);	
    TRETURN
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
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(COpenRowset::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCZombie)
//*-----------------------------------------------------------------------
//| Test Case:		TCZombie - Test the Zombie states of IOpenRowset
//|	Created:			07/19/96
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CTransaction::Init())
	// }}
	{
		//register interface to be tested                                         
   		if(RegisterInterface(SESSION_INTERFACE, IID_IOpenRowset))
			return TRUE;
	}

	//Not all providers have to support transactions
	//If a required interface, an error would ahve been posted by VerifyInterface
	TEST_PROVIDER(m_pITransactionLocal != NULL);
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_1()
{
	TBEGIN
	IOpenRowset* pIOpenRowset = NULL;
	IRowset* pIRowset = NULL;

	//Start the Transaction
	//And obtain the IOpenRowset interface
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIOpenRowset))
	TESTC(pIOpenRowset!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetAbort(TRUE))
	
	//Verify we still can use IOpenRowset after an ABORT			
	TESTC_(pIOpenRowset->OpenRowset(NULL,&m_pCTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)

CLEANUP:
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Zombie - ABORT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_2()
{
	TBEGIN
	IOpenRowset* pIOpenRowset = NULL;
	IRowset* pIRowset = NULL;

	//Start the Transaction
	//This also generates the Rowset from IOpenRowset
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIOpenRowset))
	TESTC(pIOpenRowset!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetAbort(FALSE))
	
	//Verify we still can use IOpenRowset after an ABORT			
	TESTC_(pIOpenRowset->OpenRowset(NULL,&m_pCTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)

CLEANUP:
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining TRUE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_3()
{
	TBEGIN
	IOpenRowset* pIOpenRowset = NULL;
	IRowset* pIRowset = NULL;

	//Start the Transaction
	//This also generates the Rowset from IOpenRowset
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIOpenRowset))
	TESTC(pIOpenRowset!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Abort the Transaction with fRetaining==TRUE
	TESTC(GetCommit(TRUE))
	
	//Verify we still can use IOpenRowset after a COMMIT			
	TESTC_(pIOpenRowset->OpenRowset(NULL,&m_pCTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)

CLEANUP:
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(S_OK);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Zombie - COMMIT with fRetaining FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCZombie::Variation_4()
{
	TBEGIN
	IOpenRowset* pIOpenRowset = NULL;
	IRowset* pIRowset = NULL;

	//Start the Transaction
	//This also generates the Rowset from IOpenRowset
	TESTC(StartTransaction(USE_SUPPORTED_SELECT_ALLFROMTBL,(IUnknown**)&pIOpenRowset))
	TESTC(pIOpenRowset!=NULL)

	//Verify we have a valid rowset pointer
	TESTC(m_pIRowset!=NULL)

	//Abort the Transaction with fRetaining==FALSE
	TESTC(GetCommit(FALSE))
	
	//Verify we still can use IOpenRowset after a COMMIT			
	TESTC_(pIOpenRowset->OpenRowset(NULL,&m_pCTable->GetTableID(),NULL,IID_IRowset,0,NULL,(IUnknown**)&pIRowset),S_OK)

CLEANUP:
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIRowset);
	CleanUpTransaction(XACT_E_NOTRANSACTION); //No longer in a transaction
	TRETURN
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCZombie::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(CTransaction::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCAggregation)
//*-----------------------------------------------------------------------
//| Test Case:		TCAggregation - Test all Aggregation Senarios
//|	Created:			10/30/97
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAggregation::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COpenRowset::Init())
	// }}
	{
		return TRUE;
	}
	return FALSE;
}




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - OpenRowset - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_1()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIUnkInner, &Aggregate), DB_E_NOAGGREGATION);
	
CLEANUP:
	FreeProperties();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - OpenRowset - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_2()
{
	TBEGIN
	IUnknown* pIUnkInner = NULL;
	CAggregate Aggregate(pIOpenRowset());

	//Aggregation
	HRESULT hr = CreateOpenRowset(IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset));

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - OpenRowset -> Rowset -> GetReferencedRowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_3()
{
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IRowsetInfo* pIRowsetInfo = NULL;
	IUnknown* pIAggregate   = NULL;
	IUnknown* pIUnkOuter	= NULL;
	IUnknown* pIUnkInner	= NULL;
	HRESULT hr = S_OK;

	//Aggregation
	SetSettableProperty(DBPROP_BOOKMARKS);
	hr = CreateOpenRowset(IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowsetInfo, (IUnknown**)&pIRowsetInfo));

	//Verify we are hooked up...
	//This call we are using the Rowset and asking for IID_IAggregate, 
	//which is the outer object and should succeed!!!  Kind of cool huh!
	TEST3C_(hr = pIRowsetInfo->GetReferencedRowset(0, IID_IAggregate, (IUnknown**)&pIAggregate), S_OK, DB_E_BADORDINAL, DB_E_NOTAREFERENCECOLUMN);
	if(hr==DB_E_NOTAREFERENCECOLUMN || hr==DB_E_BADORDINAL)
		TESTC(!GetProperty(DBPROP_BOOKMARKS, DBPROPSET_ROWSET, pIUnkInner, VARIANT_TRUE));
	TESTC_PROVIDER(hr==S_OK);

	//Now make sure the Rowset QI for IUnknown give me the outer
	TESTC_(hr = pIRowsetInfo->GetReferencedRowset(0, IID_IUnknown, (IUnknown**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIUnkOuter));

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnkOuter);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateSession - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_4()
{ 
	TBEGIN
    CAggregate Aggregate(g_pIDBCreateSession);
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Aggregation
	TESTC_(CreateNewSession(NULL, IID_IOpenRowset, &pIUnkInner, &Aggregate),DB_E_NOAGGREGATION);
	TESTC(pIUnkInner == NULL);
	
	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateSession - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_5()
{
    CAggregate Aggregate(g_pIDBCreateSession);
	IUnknown* pIUnkInner = NULL;

	//Aggregation
	HRESULT hr = CreateNewSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation for this session...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IOpenRowset));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - OpenRowset -> Rowset -> GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_6()
{
	TBEGIN
    CAggregate Aggregate(g_pIDBCreateSession);
    IRowsetInfo* pIRowsetInfo = NULL;
    IOpenRowset* pIOpenRowset = NULL;
	IUnknown* pIAggregate  = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;

	//Aggregation
	HRESULT hr = CreateNewSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation for this session...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IOpenRowset, (IUnknown**)&pIOpenRowset));
	
	//Use CursorEngine ( if requested )
	if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
		SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET);

	//Now Create the Rowset
	//NOTE:  We don't call CreateOpenRowset (our helper) since this is a new IOpenRowset
	//interface, not from our main init.  Since this session is aggregated...
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIOpenRowset->OpenRowset(NULL, &m_pTable->GetTableID(), NULL, IID_IRowsetInfo, m_cPropSets, m_rgPropSets, (IUnknown**)&pIRowsetInfo),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetSpecification
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pIOpenRowset));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}


CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateCommand - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_7()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IDBCreateCommand* pIDBCreateCommand = NULL;
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_PROVIDER(QI(pIOpenRowset(), IID_IDBCreateCommand, (void**)&pIDBCreateCommand)==S_OK);
	TESTC_(pIDBCreateCommand->CreateCommand(&Aggregate, IID_ICommand, &pIUnkInner),DB_E_NOAGGREGATION);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	SAFE_RELEASE(pIDBCreateCommand);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateCommand - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_8()
{
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IDBCreateCommand* pIDBCreateCommand = NULL;
	IUnknown* pIUnkInner = NULL;
	HRESULT hr = S_OK;

	//Aggregation
	TESTC_PROVIDER(QI(pIOpenRowset(), IID_IDBCreateCommand, (void**)&pIDBCreateCommand)==S_OK);
	hr = pIDBCreateCommand->CreateCommand(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation for this command...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ICommand));

CLEANUP:
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateSession -> Command -> GetDBSession
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCAggregation::Variation_9()
{
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IDBCreateCommand* pIDBCreateCommand = NULL;
    ICommand* pICommand = NULL;
	IUnknown* pIAggregate  = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;

	//Aggregation
	HRESULT hr = CreateNewSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation for this command...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IOpenRowset));

	//Provider may not support Commands...
	TESTC_PROVIDER(QI(pIUnkInner, IID_IDBCreateCommand, (void**)&pIDBCreateCommand)==S_OK);

	//Now Create the Command
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommand, (IUnknown**)&pICommand),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetDBSession
	TEST2C_(hr = pICommand->GetDBSession(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);
	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pIDBCreateCommand));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"ICommand::GetDBSession unable to retrieve Parent object!");
	}

CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
}
// }}




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateCommand -> Rowset -> GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_10()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
	IUnknown* pIAggregate = NULL;
    IDBCreateCommand* pIDBCreateCommand = NULL;
    ICommand* pICommand = NULL;
    IRowsetInfo* pIRowsetInfo = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	HRESULT hr = S_OK;

	//Aggregation
	TESTC_PROVIDER(VerifyInterface(pIOpenRowset(), IID_IDBCreateCommand, SESSION_INTERFACE, (IUnknown**)&pIDBCreateCommand));
	hr = pIDBCreateCommand->CreateCommand(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this command...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ICommand, (IUnknown**)&pICommand));

	//Build the SQL Statment and Set the Command Text
	TESTC_(hr = g_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &pICommand), S_OK);

	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pICommand->Execute(NULL, IID_IRowsetInfo, NULL, NULL, (IUnknown**)&pIRowsetInfo),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetSpecification
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pICommand));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}

CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIDBCreateCommand);
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - SchemaRowset - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_11()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error
	
	//Obtain the SchemaRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIOpenRowset(), IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset));

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_(pIDBSchemaRowset->GetRowset(&Aggregate, DBSCHEMA_TABLES, 0, NULL, IID_IRowset, 0, NULL, &pIUnkInner),DB_E_NOAGGREGATION);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	SAFE_RELEASE(pIDBSchemaRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - SchemaRowset - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_12()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	IUnknown* pIUnkInner = NULL;
    CAggregate Aggregate(pIOpenRowset());

	//Obtain the SchemaRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIOpenRowset(), IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset));

	//Use CursorEngine ( if requested )
	if(GetModInfo()->UseServiceComponents() & SERVICECOMP_CURSORENGINE)
		SetProperty(DBPROP_CLIENTCURSOR, DBPROPSET_ROWSET);

	//Aggregation
	hr = pIDBSchemaRowset->GetRowset(&Aggregate, DBSCHEMA_TABLES, 0, NULL, IID_IUnknown, m_cPropSets, m_rgPropSets, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset));

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateSession -> SchemaRowset -> GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_13()
{ 
	TBEGIN
	HRESULT hr = S_OK;
	IGetDataSource*	pIGetDataSource = NULL;
	IDBSchemaRowset* pIDBSchemaRowset = NULL;
	IUnknown* pIAggregate = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;

    CAggregate Aggregate(pIOpenRowset());

	//Create a new DSO for those providers that only support 1 session
	//Since we need to create a session thats aggregated with our object...
	hr = CreateNewSession(NULL, IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Verify Aggregation for this session...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IGetDataSource, (IUnknown**)&pIGetDataSource));
	
	//Obtain the SchemaRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIGetDataSource, IID_IDBSchemaRowset, SESSION_INTERFACE, (IUnknown**)&pIDBSchemaRowset));

	//IDBSchemaRowset::GetRowset (note: Session is Aggregated, not rowset)
	ulRefCountBefore = Aggregate.GetRefCount();
	hr = pIDBSchemaRowset->GetRowset(NULL, DBSCHEMA_TABLES, 0, NULL, IID_IRowsetInfo, 0, NULL, (IUnknown**)&pIRowsetInfo);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetSpecification
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pIGetDataSource));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}


CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIDBSchemaRowset);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Execute - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_14()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    ICommand* pICommand = NULL;
    ICommandText* pICommandText = NULL;
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//NOTE:  ICommandText inherits from ICommand, so we really should only
	//have to test ICommandText::Execute but you never know, some providers
	//may have specical logic or seperate interfaces for some reason...

	//Create Command [OPTIONAL] interface
	TESTC_PROVIDER(CreateNewCommand(pIOpenRowset(), IID_ICommand, (IUnknown**)&pICommand)==S_OK);

	//Build the SQL Statment and Set the Command Text
	TESTC_(g_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &pICommand), S_OK);
	TESTC_(QI(pICommand, IID_ICommandText, (void**)&pICommandText),S_OK);

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TCHECK(pICommand->Execute(&Aggregate, IID_IRowset, NULL, NULL, &pIUnkInner),DB_E_NOAGGREGATION);
	TESTC(pIUnkInner == NULL);

	TCHECK(pICommandText->Execute(&Aggregate, IID_IRowset, NULL, NULL, &pIUnkInner),DB_E_NOAGGREGATION);
	TESTC(pIUnkInner == NULL);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICommandText);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - MultipleResults - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_15()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    ICommand* pICommand = NULL;
    ICommandText* pICommandText = NULL;
	ULONG_PTR ulMultipleResults = 0;
	IUnknown* pIUnkInner = NULL;
	HRESULT hr = S_OK;

	//NOTE:  ICommandText inherits from ICommand, so we really should only
	//have to test ICommandText::Execute but you never know, some providers
	//may have specical logic or seperate interfaces for some reason...

	//Create Command [OPTIONAL] interface
	TESTC_PROVIDER(CreateNewCommand(pIOpenRowset(), IID_ICommand, (IUnknown**)&pICommand)==S_OK);
	TESTC_PROVIDER(GetProperty(DBPROP_MULTIPLERESULTS, DBPROPSET_DATASOURCEINFO, g_pIDBCreateSession, &ulMultipleResults) && ulMultipleResults & DBPROPVAL_MR_SUPPORTED);

	//Build the SQL Statment and Set the Command Text
	TESTC_(hr = g_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &pICommand), S_OK);
	TESTC_(QI(pICommand, IID_ICommandText, (void**)&pICommandText),S_OK);

	//Aggregation
	TESTC_(SetRowsetProperty(pICommandText, DBPROPSET_ROWSET, DBPROP_IMultipleResults),S_OK);
	hr = pICommand->Execute(&Aggregate, IID_IUnknown, NULL, NULL, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IMultipleResults));
	SAFE_RELEASE(pICommand);
	Aggregate.ReleaseInner();
	SAFE_RELEASE(pIUnkInner);

	//Aggregation
	hr = pICommandText->Execute(&Aggregate, IID_IUnknown, NULL, NULL, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this rowset...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IMultipleResults));

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pICommandText);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - CreateCommand -> MultipleResults -> GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_16()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    ICommand* pICommand = NULL;
	IUnknown* pIAggregate = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;
	IMultipleResults* pIMultipleResults = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	HRESULT hr = S_OK;

	//Create Command (note: The Rowset isn't Aggregated, the Command is)
	TESTC_PROVIDER(CreateNewCommand(pIOpenRowset(), IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate)==S_OK);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggeregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ICommand, (IUnknown**)&pICommand));

	//Build the SQL Statment and Set the Command Text
	TESTC_(g_pTable->ExecuteCommand(SELECT_ALLFROMTBL, IID_IRowset, NULL, NULL, NULL, NULL, EXECUTE_NEVER, 0, NULL, NULL, NULL, &pICommand), S_OK);

	//ICommand::Execute
	TEST2C_(hr = pICommand->Execute(NULL, IID_IMultipleResults, NULL, NULL, (IUnknown**)&pIMultipleResults),S_OK,E_NOINTERFACE);
	TESTC_PROVIDER(hr==S_OK);

	//IMultipleResults::GetResult
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(pIMultipleResults->GetResult(NULL, 0, IID_IRowsetInfo, NULL, (IUnknown**)&pIRowsetInfo),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetSpecification
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pICommand));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}

CLEANUP:
	SAFE_RELEASE(pICommand);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIMultipleResults);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - IColumnsRowset - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_17()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IRowset* pIRowset = NULL;
    IColumnsRowset* pIColumnsRowset = NULL;
	IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

	//Now Create the Rowset
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset),S_OK);

	//Obtain the ColumnsRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIRowset, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));

	//Try to obtain anything but IID_IUnknown.  
	//This should fail, this is a requirement for COM Aggregation...
	TESTC_(pIColumnsRowset->GetColumnsRowset(&Aggregate, 0, NULL, IID_IRowset, 0, NULL, &pIUnkInner),DB_E_NOAGGREGATION);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowset);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - IColumnsRowset - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_18()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IRowset* pIRowset = NULL;
    IColumnsRowset* pIColumnsRowset = NULL;
	IUnknown* pIUnkInner = NULL;
	HRESULT hr = S_OK;

	//Now Create the Rowset
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset),S_OK);

	//Obtain the ColumnsRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIRowset, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));

	//Aggregation
	hr = pIColumnsRowset->GetColumnsRowset(&Aggregate, 0, NULL, IID_IUnknown, 0, NULL, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation for this session...
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset));

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Rowset -> IColumnsRowset -> GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_19()
{ 
	TBEGIN
    CAggregate Aggregate(pIOpenRowset());
    IRowset* pIRowset = NULL;
    IColumnsRowset* pIColumnsRowset = NULL;
	IUnknown* pIAggregate = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	HRESULT hr = S_OK;

	//Now Create the Rowset (note: Rowset is Aggregated, not ColumnsRowset)
	hr = CreateOpenRowset(IID_IUnknown, (IUnknown**)&pIUnkInner, &Aggregate);
	Aggregate.SetUnkInner(pIUnkInner);

	//Verify Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IRowset, (IUnknown**)&pIRowset));

	//Obtain the ColumnsRowset interface [OPTIONAL] interface
	TESTC_PROVIDER(VerifyInterface(pIRowset, IID_IColumnsRowset, ROWSET_INTERFACE, (IUnknown**)&pIColumnsRowset));

	//IColumnsRowset::GetRowset
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIColumnsRowset->GetColumnsRowset(NULL, 0, NULL, IID_IRowsetInfo, 0, NULL, (IUnknown**)&pIRowsetInfo),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//GetSpecification
	TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

	if(hr==S_OK)
	{
		TESTC(VerifyEqualInterface(pIAggregate, pIRowset));

		//Verify the child correctly addref'd the parent outer.
		//The is an absolute requirement that the child keep the parent outer alive.
		//If it doesn't addref the outer, the outer can be released externally since
		//its not being used anymore due to the fact the outer controls the refcount
		//of the inner.  Many providers incorrectly addref the inner, which does nothing
		//but guareentee the inner survives, but the inner will delegate to the outer
		//and crash since it no longer exists...
		TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
	}
	else
	{
		TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
	}

CLEANUP:
	FreeProperties();
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIColumnsRowset);
	SAFE_RELEASE(pIRowset);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - ISourcesRowset - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_20()
{ 
	TBEGIN
	ULONG i,cEnumInfo = 0;
	ENUMINFO* rgEnumInfo = NULL;
    ISourcesRowset* pISourcesRowset = NULL;

	//Now try to verify all the other Enumerators registered...
	TESTC_(GetEnumInfo(NULL, &cEnumInfo, &rgEnumInfo),S_OK);
	
	//Loop over all enumerators...
	for(i=0; i<cEnumInfo; i++)
	{
		CAggregate Aggregate(pIOpenRowset());
		IUnknown* pIUnkInner = INVALID(IUnknown*); //Make sure pointer is NULLed on error

		//Only deal with Enumerators...
		if(rgEnumInfo[i].wType != DBSOURCETYPE_ENUMERATOR)
			continue;

		//Obtain the Enumerator
		TESTC_(CreateFromDisplayName(NULL, rgEnumInfo[i].wszParseName, IID_ISourcesRowset, (IUnknown**)&pISourcesRowset),S_OK);

		//Try to obtain anything but IID_IUnknown.  
		TESTC_(pISourcesRowset->GetSourcesRowset(&Aggregate, IID_IRowset, 0, NULL, &pIUnkInner),DB_E_NOAGGREGATION);

		//Inner object cannot RefCount the outer object - COM rule for CircularRef
		COMPARE(Aggregate.GetRefCount(), 1);
		TESTC(pIUnkInner == NULL);

		SAFE_RELEASE(pISourcesRowset);
	}

CLEANUP:
	SAFE_RELEASE(pISourcesRowset);
	SAFE_FREE(rgEnumInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - ISourcesRowset - IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_21()
{ 
	TBEGIN
	ULONG i,cEnumInfo = 0;
	ENUMINFO* rgEnumInfo = NULL;
    ISourcesRowset* pISourcesRowset = NULL;
	IUnknown* pIUnkInner = NULL;
	HRESULT hr = S_OK;

	//Now try to verify all the other Enumerators registered...
	TESTC_(GetEnumInfo(NULL, &cEnumInfo, &rgEnumInfo),S_OK);
	
	//Loop over all enumerators...
	for(i=0; i<cEnumInfo; i++)
	{
		CAggregate Aggregate(pIOpenRowset());

		//Only deal with Enumerators...
		if(rgEnumInfo[i].wType != DBSOURCETYPE_ENUMERATOR)
			continue;

		//Obtain the Enumerator
		TESTC_(CreateFromDisplayName(NULL, rgEnumInfo[i].wszParseName, IID_ISourcesRowset, (IUnknown**)&pISourcesRowset),S_OK);

		//Aggregation
		hr = pISourcesRowset->GetSourcesRowset(&Aggregate, IID_IUnknown, 0, NULL, (IUnknown**)&pIUnkInner);
		Aggregate.SetUnkInner(pIUnkInner);

		//Verify Aggregation for this...
		//NOTE: We want to continue to test the other providers...
		if(!Aggregate.VerifyAggregationQI(hr, IID_IRowset))
			TOUTPUT("Unable to aggregate provider " << rgEnumInfo[i].wszName);
		SAFE_RELEASE(pISourcesRowset);
		SAFE_RELEASE(pIUnkInner);
	}

CLEANUP:
	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIUnkInner);
	SAFE_FREE(rgEnumInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Enumerator -> SourcesRowset -> GetSpecification
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCAggregation::Variation_22()
{ 
	TBEGIN
	ULONG i,cEnumInfo = 0;
	ENUMINFO* rgEnumInfo = NULL;
    ISourcesRowset* pISourcesRowset = NULL;
	IRowsetInfo* pIRowsetInfo = NULL;
	IUnknown* pIAggregate = NULL;
	IUnknown* pIUnkInner = NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	HRESULT hr = S_OK;

	//Now try to verify all the other Enumerators registered...
	TESTC_(GetEnumInfo(NULL, &cEnumInfo, &rgEnumInfo),S_OK);
	
	//Loop over all enumerators...
	for(i=0; i<cEnumInfo; i++)
	{
		CAggregate Aggregate(pIOpenRowset());
		CLSID clsid;

		//Only deal with Enumerators...
		if(rgEnumInfo[i].wType != DBSOURCETYPE_ENUMERATOR)
			continue;

		//Obtain the Enumerator  (note: Enumerator is Aggregated, not rowset)
		//NOTE:  The ParseDisplayName interface has no way to do aggregation!
		//Thus I'm reallying upon the Root Enumerator wszParseName to be the CLSID
		//which it is ans will never change, but just a note as to why I'm not
		//calling IParseDisplayName::ParseDisplayName on the Enum wszParseName...
		TESTC_(CLSIDFromString(rgEnumInfo[i].wszParseName, &clsid),S_OK);
		hr = CoCreateInstance(clsid, &Aggregate, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnkInner);
		Aggregate.SetUnkInner(pIUnkInner);

		//Verify Aggregation for this...
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_ISourcesRowset, (IUnknown**)&pISourcesRowset));

		//ISourcesRowset::GetRowset
		ulRefCountBefore = Aggregate.GetRefCount();
		TESTC_(hr = pISourcesRowset->GetSourcesRowset(NULL, IID_IRowsetInfo, 0, NULL, (IUnknown**)&pIRowsetInfo),S_OK);
		ulRefCountAfter = Aggregate.GetRefCount();

		//GetSpecification
		TEST2C_(hr = pIRowsetInfo->GetSpecification(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK,S_FALSE);

		if(hr==S_OK)
		{
			TESTC(VerifyEqualInterface(pIAggregate, pISourcesRowset));

			//Verify the child correctly addref'd the parent outer.
			//The is an absolute requirement that the child keep the parent outer alive.
			//If it doesn't addref the outer, the outer can be released externally since
			//its not being used anymore due to the fact the outer controls the refcount
			//of the inner.  Many providers incorrectly addref the inner, which does nothing
			//but guareentee the inner survives, but the inner will delegate to the outer
			//and crash since it no longer exists...
			TCOMPARE_(ulRefCountAfter > ulRefCountBefore);
		}
		else
		{
			TWARNING(L"IRowsetInfo::GetSpecification unable to retrieve Parent object!");
		}

		SAFE_RELEASE(pISourcesRowset);
		SAFE_RELEASE(pIRowsetInfo);
		SAFE_RELEASE(pIAggregate);
	}

CLEANUP:
	SAFE_RELEASE(pISourcesRowset);
	SAFE_RELEASE(pIRowsetInfo);
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIUnkInner);
	SAFE_FREE(rgEnumInfo);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCAggregation::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(COpenRowset::Terminate());
}	// }}
// }}
// }}

// {{ TCW_TC_PROTOTYPE(TCProperties)
//*-----------------------------------------------------------------------
//| Test Case:		TCProperties - Test all senarios dealing with OPTIONAL
//| Created:  	12/15/97
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCProperties::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(COpenRowset::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_OK [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_1()
{ 
	TBEGIN
    CRowset RowsetA;
	IRowset* pIRowset = NULL;
	HROW hRow1, hRow2;

    //Set properties
	//All of these properties are required and must be able to be to TRUE
	//CANHOLDROWS is a required Level-0 property
	SetProperty(DBPROP_IAccessor,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IColumnsInfo,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IConvertType,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IRowset,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_IRowsetInfo,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK
	//Should always be S_OK, since we are dealig with OPTIONAL meaning
	//the only time it will return NOTSET is if the property could not be set
	//According to the Level 0 spec, CANHOLDROWS must be able to be set ALWAYS
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset),S_OK);
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));

	//Verify DBPROP_Rowset Set
	TESTC_(QI(pIRowset, IID_IRowset),S_OK);

	//Verify DBPROP_CANHOLDROWS
	TESTC_(RowsetA.CreateRowset(pIRowset),S_OK);
	TESTC_(RowsetA.GetNextRows(&hRow1),S_OK);
	TESTC_(RowsetA.GetNextRows(&hRow2),S_OK);

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_OK [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_2()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
	SetProperty(DBPROP_IRowset, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);	
    
    //OPTIONAL - S_OK
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK)
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));

	//Verify DBPROP_Rowset Set
	TESTC_(QI(pIRowset, IID_IRowset),S_OK);

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTSET [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_3()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

	//Obtain the Rowset IIDs
	ULONG i, cRowsetIIDs = 0;
	INTERFACEMAP* rgRowsetIIDs = NULL;
	TESTC(GetInterfaceArray(ROWSET_INTERFACE, &cRowsetIIDs, &rgRowsetIIDs));

    //These properties must all be set, even when optional
	for(i=0; i<cRowsetIIDs; i++)
		SetProperty(rgRowsetIIDs[i].dwPropertyID,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL properties - S_OK/DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),S_OK,DB_S_ERRORSOCCURRED);
		
	if(hr==S_OK)
	{
		//All properties could be set...
		//Verify entire propset array is DBPROPSTATUS_OK
		TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK))

		//Since all were able to be set, make sure there are there...
		for(ULONG i=0; i<cRowsetIIDs; i++)
			if(QI(pIRowset, *rgRowsetIIDs[i].pIID)!=S_OK)
				TERROR(L"Property Status Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");
	}
	else
	{
		//At least one of the properties is either NOTSUPPORTED, NOTSETTABLE, or NOTSET
		TESTC(!VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));

		//Loop through all the properties
		for(ULONG i=0; i<cRowsetIIDs; i++)
		{
			DBPROPSTATUS dwPropStatus;
			TCOMPARE_(GetPropStatus(m_cPropSets, m_rgPropSets, rgRowsetIIDs[i].dwPropertyID,	DBPROPSET_ROWSET, &dwPropStatus));
		
			//Verify optional property status
			//This function will allow NOTSET for optional even though we pass in OK...
			if(!VerifyPropStatus(m_cPropSets, m_rgPropSets, rgRowsetIIDs[i].dwPropertyID,	DBPROPSET_ROWSET, DBPROPSTATUS_OK))
				TERROR(L"Property Status Incorrect for " << GetPropertyName(rgRowsetIIDs[i].dwPropertyID, DBPROPSET_ROWSET) << "\n");
		
			//Now whatever ones were DBPROPSTATUS_OK need to actually have the interface
			if(QI(pIRowset, *rgRowsetIIDs[i].pIID) != ((dwPropStatus == DBPROPSTATUS_OK) ? S_OK : E_NOINTERFACE))
				TERROR(L"Property Status Incorrect for " << GetInterfaceName(*rgRowsetIIDs[i].pIID) << "\n");
		}
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTSET [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_4()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set properties as OPTIONAL
	SetProperty(DBPROP_IColumnsRowset,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_OPTIONAL);

    //OPTIONAL properties - S_OK/DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);

	if(hr==S_OK)
	{
		//All properties could be set...
		//Verify entire propset array is DBPROPSTATUS_OK
		TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK))

		//Since all were able to be set, make sure there are there...
		TESTC_(QI(pIRowset, IID_IColumnsRowset),S_OK);
	}
	else
	{
		//At least one of the properties is either NOTSUPPORTED, NOTSETTABLE, or NOTSET
		DBPROPSTATUS dwPropStatus1;
		TESTC(GetPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IColumnsRowset,	DBPROPSET_ROWSET, &dwPropStatus1));

		//Make sure there not all DBPROPSTATUS_OK
		TESTC(!VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));

		//Verify optional property status
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IColumnsRowset,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));

		//Now whatever ones were DBPROPSTATUS_OK need to actually have the interface
		TESTC_(QI(pIRowset, IID_IColumnsRowset),			dwPropStatus1 == DBPROPSTATUS_OK ? S_OK : E_NOINTERFACE);
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_BADCOLUMN [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_5()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set properties
	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL, DBCOLUMN_MAYSORT);
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
    
    //OPTIONAL - DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);

	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));

	//DBPROPSTATUS_BADCOLUMN
	if(hr==S_OK)
	{
		TESTC(!(GetPropInfoFlags(DBPROP_ISequentialStream, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK));
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream, DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	}
	else
	{
		if(GetPropInfoFlags(DBPROP_ISequentialStream, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK)
		{
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream, DBPROPSET_ROWSET, DBPROPSTATUS_BADCOLUMN));
		}
		else
		{
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream, DBPROPSET_ROWSET, DBPROPSTATUS_NOTALLSETTABLE));
		}
	}	

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_BADCOLUMN [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_6()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set properties
	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL, DBCOLUMN_MAYSORT);
    
    //OPTIONAL - DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);

	//DBPROPSTATUS_BADCOLUMN
	if(hr==S_OK)
	{
		TESTC(!(GetPropInfoFlags(DBPROP_ISequentialStream, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK));
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream, DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	}
	else
	{
		if(GetPropInfoFlags(DBPROP_ISequentialStream, DBPROPSET_ROWSET) & DBPROPFLAGS_COLUMNOK)
		{
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream, DBPROPSET_ROWSET, DBPROPSTATUS_BADCOLUMN));
		}
		else
		{
			TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream, DBPROPSET_ROWSET, DBPROPSTATUS_NOTALLSETTABLE));
		}
	}	

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_BADOPTION [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_7()
{ 
	TBEGIN
	IRowset* pIRowset = NULL;

    //Set properties
	//Make sure their not do dwOptions & OPTIONAL
 	SetProperty(DBPROP_CANFETCHBACKWARDS,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL | 0x00000002);	
	SetProperty(DBPROP_IAccessor,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
    
    //OPTIONAL - DB_E_ERRORSOCCURRED
	//Since invalid option (not OPTIONAL) this is treated as an error
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(pIRowset == NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS,	DBPROPSET_ROWSET, DBPROPSTATUS_BADOPTION));
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IAccessor,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_BADOPTION [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_8()
{ 
	TBEGIN
	IRowset* pIRowset = NULL;

    //Set properties
	//Make sure their not do dwOptions & OPTIONAL
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL | 0x00000002);	
    
    //OPTIONAL - DB_E_ERRORSOCCURRED
	//Since invalid option (not OPTIONAL) this is treated as an error
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), DB_E_ERRORSOCCURRED);
	TESTC(pIRowset == NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBPROPSTATUS_BADOPTION));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_BADVALUE [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_9()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
	//Make sure there looking for VARIANT_TRUE not TRUE
	SetProperty(DBPROP_CANFETCHBACKWARDS,	DBPROPSET_ROWSET, (void*)0x00000001, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), DB_S_ERRORSOCCURRED)
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, DBPROPSTATUS_BADVALUE));
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_BADVALUE [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_10()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
	//Make sure there looking for VARIANT_TRUE not TRUE
	SetProperty(DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, (void*)0x00000001, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), DB_S_ERRORSOCCURRED)
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_ROWSET, DBPROPSTATUS_BADVALUE));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_CONFLICTING [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_11()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set properties
	SetProperty(DBPROP_OTHERINSERT,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_OTHERUPDATEDELETE,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_OWNINSERT,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_OWNUPDATEDELETE,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK/DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);
		
	if(hr==S_OK)
	{
		TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));
	}
	else
	{	
		//DBPROP_OTHERINSERT
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
		//DBPROP_OTHERUPDATEDELETE
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
		//DBPROP_OWNINSERT
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
		//DBPROP_OWNUPDATEDELETE
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_CONFLICTING [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_12()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set properties
	SetProperty(DBPROP_OTHERINSERT,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_OTHERUPDATEDELETE,	DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_OWNINSERT,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_OWNUPDATEDELETE,		DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK/DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);
		
	if(hr==S_OK)
	{
		TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));
	}
	else
	{	
		//DBPROP_OTHERINSERT
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
		//DBPROP_OTHERUPDATEDELETE
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OTHERUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
		//DBPROP_OWNINSERT
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNINSERT,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
		//DBPROP_OWNUPDATEDELETE
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_OK) || 
			VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_OWNUPDATEDELETE,	DBPROPSET_ROWSET, DBPROPSTATUS_CONFLICTING));
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTALLSETTABLE [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_13()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK; 

    //Set properties
	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK/DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);
		
	if(hr==S_OK)
	{
		TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));
	}
	else
	{
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream,	DBPROPSET_ROWSET, DBPROPSTATUS_OK));
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS,	DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTALLSETTABLE [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_14()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;
	HRESULT hr = S_OK;

    //Set properties
	SetProperty(DBPROP_ISequentialStream, DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - S_OK/DB_S_ERRORSOCCURRED
	TEST2C_(hr = CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), S_OK, DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);
		
	if(hr==S_OK)
	{
		TESTC(VerifyPropSetStatus(m_cPropSets, m_rgPropSets, DBPROPSTATUS_OK));
	}
	else
	{
		TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_ISequentialStream,			DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	}

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTSETTABLE [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_15()
{ 
	TBEGIN
	IRowset* pIRowset = NULL;

    //Set properties
	SetProperty(DBPROP_IAccessor,			DBPROPSET_ROWSET, (void*)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS,			DBPROPSET_ROWSET, (void*)VARIANT_TRUE, DBTYPE_BOOL, DBPROPOPTIONS_REQUIRED);
    
    //OPTIONAL - DB_S_ERRORSOCCURRED - NOTSETTABLE
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IAccessor,			DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE));
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTSETTABLE [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_16()
{ 
	TBEGIN
	IRowset* pIRowset = NULL;

    //Set properties
	SetProperty(DBPROP_IColumnsInfo, DBPROPSET_ROWSET, (void*)VARIANT_FALSE, DBTYPE_BOOL, DBPROPOPTIONS_OPTIONAL);
    
    //OPTIONAL - DB_S_ERRORSOCCURRED - NOTSETTABLE
	TESTC_(CreateOpenRowset(IID_IRowset, (IUnknown**)&pIRowset), DB_S_ERRORSOCCURRED);
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_IColumnsInfo,			DBPROPSET_ROWSET, DBPROPSTATUS_NOTSETTABLE));

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTSUPPORTED [Group]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_17()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
    SetProperty(DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET,(void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_OPTIONAL);
	SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_DATASOURCEINFO,(void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_OPTIONAL);	//NOTSUPPORTED
    
    //Property errors - OPTIONAL - DB_S_ERRORSOCCURRED
	TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_S_ERRORSOCCURRED)
	TESTC(pIRowset != NULL);
		
	//Verify error array
    TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANFETCHBACKWARDS, DBPROPSET_ROWSET, DBPROPSTATUS_OK))
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_DATASOURCEINFO, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc OPTIONAL - DBPROPSTATUS_NOTSUPPORTED [Single]
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_18()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    //Set properties
	SetProperty(DBPROP_CANHOLDROWS,DBPROPSET_DATASOURCEINFO,(void*)VARIANT_TRUE,DBTYPE_BOOL,DBPROPOPTIONS_OPTIONAL);	//NOTSUPPORTED
    
    //Property errors - OPTIONAL - DB_S_ERRORSOCCURRED
	TESTC_(CreateOpenRowset(IID_IRowset,(IUnknown**)&pIRowset),DB_S_ERRORSOCCURRED)
	TESTC(pIRowset != NULL);
		
	//Verify error array
	TESTC(VerifyPropStatus(m_cPropSets, m_rgPropSets, DBPROP_CANHOLDROWS, DBPROPSET_DATASOURCEINFO, DBPROPSTATUS_NOTSUPPORTED))

CLEANUP:
	FreeProperties();
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_19()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Multiple Sets - Static Arrays
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_20()
{ 
	TBEGIN
    IUnknown* pIUnknown = NULL;

    //We want to create Muliple Property Sets using the Same Set
	//We need to create the sets manually, since our helper SetProperty
	//will always allocate within the found propset, and we want multiple similar sets 
	//The other useful thing with doing it manualy, is that we can create the 
	//propset "statically" so this will make sure the provider is not releasing
	//our reallocating our static array...
	
	//Create selected Properties
	DBPROPID rgPropertyIDs[] = 
	{
		//PropSet 0 (0 properties)
		//PropSet 1	(1 property)
		DBPROP_CANHOLDROWS,
		//PropSet 2	(2 properties)
		DBPROP_IAccessor,
		DBPROP_IColumnsInfo,
		//PropSet 3	(3 properties
		DBPROP_IConvertType,
		DBPROP_IRowset,
		DBPROP_IRowsetInfo,
		//PropSet 4	(4 properties
		DBPROP_IConvertType,
		DBPROP_IRowset,
		DBPROP_IRowsetInfo,
		DBPROP_IRowset,
	};
	const ULONG cProperties = NUMELEM(rgPropertyIDs);
	ULONG iPropertyID = 0;

	//create the inner structs...
	DBPROP rgProperties[cProperties];
	memset(rgProperties, 0, sizeof(DBPROP)*cProperties);
	
	//Create the outer structs...
	const ULONG cPropSets = 5;
	DBPROPSET rgPropSets[cPropSets];
	memset(rgPropSets, 0, sizeof(DBPROPSET)*cPropSets);

	//Loop Over PropSets
	for(ULONG iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		//rgProperties is just a large array, we will index into a different
		//locations for each propset.
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];

		pPropSet->cProperties = iPropSet;
		pPropSet->rgProperties = &rgProperties[iPropertyID];
		pPropSet->guidPropertySet = DBPROPSET_ROWSET;
		
		//Loop Over Properties...
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			pProp->dwPropertyID		= rgPropertyIDs[iPropertyID++];

			//Garbage the Status, so we know their touching every propset on return
			pProp->dwStatus			= INVALID(DBPROPSTATUS); 
			pProp->dwOptions		= iProp%2 ? DBPROPOPTIONS_REQUIRED : DBPROPOPTIONS_OPTIONAL;
			
			//Variant
			V_VT(&pProp->vValue)	= VT_BOOL;
			V_BOOL(&pProp->vValue)	= VARIANT_TRUE;
		}
	}
	
    //S_OK - No properties in error
	TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IUnknown,cPropSets,rgPropSets,&pIUnknown),S_OK);
	TESTC(pIUnknown != NULL);
		
	//Verify error array
	TCOMPARE_(VerifyPropSetStatus(cPropSets, rgPropSets, DBPROPSTATUS_OK));

CLEANUP:
    SAFE_RELEASE(pIUnknown);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Multiple Sets - Static Arrays - With Errors
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_21()
{ 
	TBEGIN
    IRowset* pIRowset = NULL;

    //We want to create Muliple Property Sets using the Same Set
	//We need to create the sets manually, since our helper SetProperty
	//will always allocate within the found propset, and we want multiple similar sets 
	//The other useful thing with doing it manualy, is that we can create the 
	//propset "statically" so this will make sure the provider is not releasing
	//our reallocating our static array...
	
	//Create selected Properties
	DBPROPID rgPropertyIDs[] = 
	{
		//PropSet 0 (0 properties)
		//PropSet 1	(1 property)
		DBPROP_CANHOLDROWS,
		//PropSet 2	(2 properties)
		DBPROP_IAccessor,
		DBPROP_IRowsetChange,
		//PropSet 3	(3 properties
		DBPROP_IConvertType,
		DBPROP_IRowset,
		DBPROP_IRowsetInfo,
		//PropSet 4	(4 properties
		DBPROP_IConvertType,
		DBPROP_IRowset,
		DBPROP_IRowsetChange,
		DBPROP_IRowset,
	};
	const ULONG cProperties = NUMELEM(rgPropertyIDs);
	ULONG iPropertyID = 0;

	//create the inner structs...
	DBPROP rgProperties[cProperties];
	memset(rgProperties, 0, sizeof(DBPROP)*cProperties);
	
	//Create the outer structs...
	const ULONG cPropSets = 5;
	DBPROPSET rgPropSets[cPropSets];
	memset(rgPropSets, 0, sizeof(DBPROPSET)*cPropSets);

	//Loop Over PropSets
	for(ULONG iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		//rgProperties is just a large array, we will index into a different
		//locations for each propset.
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];

		pPropSet->cProperties = iPropSet;
		pPropSet->rgProperties = &rgProperties[iPropertyID];
		pPropSet->guidPropertySet = iPropSet < cPropSets-1 ? DBPROPSET_ROWSET : DBPROPSET_DBINIT;
		
		//Loop Over Properties...
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			pProp->dwPropertyID		= rgPropertyIDs[iPropertyID++];

			//Garbage the Status, so we know their touching every propset on return
			pProp->dwStatus			= INVALID(DBPROPSTATUS); 
			pProp->dwOptions		= iProp%2 ? DBPROPOPTIONS_REQUIRED : DBPROPOPTIONS_OPTIONAL;
			
			//Variant
			V_VT(&pProp->vValue)	= VT_BOOL;
			V_BOOL(&pProp->vValue)	= VARIANT_TRUE;
		}
	}
	
    //DB_E_ERRORSOCCURRED - 3 Proeprties were in the wrong propset
	TESTC_(pIOpenRowset()->OpenRowset(NULL,&m_pTable->GetTableID(),NULL,IID_IRowset,cPropSets,rgPropSets,(IUnknown**)&pIRowset),DB_E_ERRORSOCCURRED);
	TESTC(pIRowset == NULL);
		
	//Verify error array
	TCOMPARE_(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_CANHOLDROWS,	DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	TCOMPARE_(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_IAccessor,		DBPROPSET_ROWSET, DBPROPSTATUS_OK));
	TCOMPARE_(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_IRowsetChange,	DBPROPSET_ROWSET, DBPROPSTATUS_OK));

	//These were in the wrong property group (DBINIT)
	TCOMPARE_(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_IConvertType,	DBPROPSET_DBINIT, DBPROPSTATUS_NOTSUPPORTED));
	TCOMPARE_(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_IRowset,		DBPROPSET_DBINIT, DBPROPSTATUS_NOTSUPPORTED));
	TCOMPARE_(VerifyPropStatus(cPropSets, rgPropSets, DBPROP_IRowsetChange,	DBPROPSET_DBINIT, DBPROPSTATUS_NOTSUPPORTED));

CLEANUP:
    SAFE_RELEASE(pIRowset);
    TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCProperties::Variation_22()
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
BOOL TCProperties::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(COpenRowset::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

