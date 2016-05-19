#ifndef __MODMACROS_HPP_
#define __MODMACROS_HPP_

/////////////////////////////////////////////////////////////////
// Includes
//
/////////////////////////////////////////////////////////////////
#include "DTMCommon.h"



/////////////////////////////////////////////////////////////////
// Module Defines
//
/////////////////////////////////////////////////////////////////
#define DECLARE_MODULE_NAME(thename)						\
	const wchar_t g_wszModuleName[] =  L##thename;			\
	const wchar_t *gwszModuleName = g_wszModuleName;		\
	extern CThisTestModule* g_pThisTestModule;

#define DECLARE_MODULE_CLSID \
	const GUID CLSID_TESTMODULE 

#define DECLARE_MODULE_OWNER(theowner) \
	const wchar_t g_wszModuleOwner[] = L##theowner 

#define DECLARE_MODULE_DESCRIP(thedescrip) \
	const wchar_t g_wszModuleDescrip[] = L##thedescrip 

#define DECLARE_MODULE_VERSION(theversion) \
	const long g_lModuleVersion = theversion



/////////////////////////////////////////////////////////////////
// Test Case Function Defines
//
/////////////////////////////////////////////////////////////////
#define DECLARE_TEST_CASE_DATA() \
	static const WCHAR		m_wszTestCaseName[];		\
    static const WCHAR		m_wszTestCaseDesc[];		\
	static		 ULONG		m_cVarInfo;					\
	static const VARINFO	m_rgVarInfo[];

#define DECLARE_TEST_CASE_FUNCS(theClass,baseClass)		\
	virtual ~theClass (void) {};						\
    theClass ( wchar_t* pwszTestCaseName) : baseClass(pwszTestCaseName)		\
	{													\
		m_pwszTestCaseName = (WCHAR*)m_wszTestCaseName;	\
		m_pwszTestCaseDesc = (WCHAR*)m_wszTestCaseDesc;	\
		for(m_cVarInfo=0; m_rgVarInfo[m_cVarInfo].pfnVariation; m_cVarInfo++);	\
	};													\
														\
    const WCHAR* GetCaseDesc( void )					\
        {												\
			return m_wszTestCaseDesc;					\
		}												\
	const VARINFO* GetVarInfoArray(void)				\
		{												\
			return m_rgVarInfo;							\
		}												\
	DWORD GetVarCount(void)								\
		{												\
			return m_cVarInfo;							\
		}												\
	const WCHAR* GetCaseName(void)						\
		{												\
			return m_wszTestCaseName;					\
		}



/////////////////////////////////////////////////////////////////
// Test Case Function Defines
//
/////////////////////////////////////////////////////////////////
#define DECLARE_TEST_CASE(theClass) \
	TEST_CASE_##theClass()

    //Allow user to skip over Test Case macros.  This allows
    //  user to have TestWizard maintain cases in a central file,
    //  but for the user to then copy out to separate source files.
    //  This is an interim hack until TestWizard can support 
    //  multiple source files.
    //To use this
    //  1. Split out the test case to different source file
    //  2, In primary source file define NULLTESTCASE prior to 
    //      the MODStandard.hpp reference
    //  3. Do NOT use pre-compiled header for the main source file
#ifdef NULLTESTCASE   
    #define BEG_TEST_CASE(theClass, baseClass, tcName);
    #define TEST_VARIATION(tvNum, tvName);
    #define END_TEST_CASE();
#else
    #define BEG_TEST_CASE(theClass, baseClass, tcName)					\
        const WCHAR		theClass::m_wszTestCaseDesc[] = { tcName };		\
		const WCHAR		theClass::m_wszTestCaseName[] = { L#theClass };	\
			  ULONG		theClass::m_cVarInfo = 0;						\
	    const VARINFO	theClass::m_rgVarInfo [] = {

	#define TEST_VARIATION(tvNum, tvName) \
	    {(PFNVARIATION)&THE_CLASS::Variation_##tvNum, tvName, tvNum},

    #define END_TEST_CASE() \
		{ NULL, NULL, 0 } \
		}; 
#endif



/////////////////////////////////////////////////////////////////
// Test Module Defines
//
/////////////////////////////////////////////////////////////////
#define TEST_MODULE(cCases, tcClass, tcNam)				\
														\
	const short g_nTestCount = cCases;					\
    void * ModuleGetTestCase( LONG iCase, CThisTestModule* pCThisTestModule) \
        {												\
            CTestCases* pCTestCase;						\
            switch( iCase ) {

#define TEST_CASE(iCase, theClass)								\
    case iCase:													\
		pCTestCase = new theClass(NULL);						\
		pCTestCase->SetOwningMod(iCase-1, pCThisTestModule);	\
		return pCTestCase;

#define END_TEST_MODULE()								\
    default:											\
        return NULL;									\
        }												\
    }													\
	GlobalModuleData g_gmd = {	ModuleInit,				\
								ModuleTerminate,		\
								ModuleGetTestCase,		\
								g_nTestCount,			\
								g_wszModuleOwner,		\
								g_wszModuleName,		\
								&CLSID_TESTMODULE,		\
								g_wszModuleDescrip,		\
								g_lModuleVersion };		\
	void SetGlobalModuleData(void)						\
	{													\
		if(g_pThisTestModule)							\
			delete g_pThisTestModule;					\
		g_pThisTestModule = new CThisTestModule(&g_gmd);\
	}



/////////////////////////////////////////////////////////////////
// Test Defines
//
/////////////////////////////////////////////////////////////////
#define TEST_PASS		eVariationStatusPassed
#define TEST_FAIL		eVariationStatusFailed
#define TEST_SKIPPED	eVariationStatusNotRun
#define TEST_CONFORMANCE_WARNING eVariationStatusConformanceWarning

typedef VARIATION_STATUS TESTRESULT;


#endif //__MODMACROS_HPP_
