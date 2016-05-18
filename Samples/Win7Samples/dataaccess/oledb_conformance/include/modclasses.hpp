//-----------------------------------------------------------------------------
// Microsoft Local Test Manager (LTM)
// Copyright (C) 1997 - 1999 By Microsoft Corporation.
//	  
// @doc
//												  
// @module MODCLASSES.HPP
//
//-----------------------------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////////
#ifndef __MODCLASSES_HPP_
#define __MODCLASSES_HPP_

////////////////////////////////////////////////////////////////////
// Include
//
////////////////////////////////////////////////////////////////////
#include "ModuleCore.h"
#include "MODError.hpp"

#pragma warning( disable : 4275 )

////////////////////////////////////////////////////////////////////
// Forwards
//
////////////////////////////////////////////////////////////////////
class CThisTestModule;
class CTestCases;


////////////////////////////////////////////////////////////////////
// Defines
//
////////////////////////////////////////////////////////////////////
typedef BOOL (*pfnModGlobal)(CThisTestModule *);
typedef void *(*pfnModGetCase)(long, CThisTestModule *);
typedef int  (IUnknown::*PFNVARIATION)(void);

typedef struct tagVARINFO
{
	PFNVARIATION	pfnVariation;
	const WCHAR*	wszDescription;
	DWORD			id;						// Variation ID. Valid values start at 1.
} VARINFO;


struct GlobalModuleData
{
	pfnModGlobal	m_pfnModuleInit;
	pfnModGlobal	m_pfnModuleTerminate;
	pfnModGetCase	m_pfnModuleGetCase;
	WORD			m_wTestCount;
	const WCHAR *	m_wszModuleOwner;
	const WCHAR *	m_wszModuleName;
	const GUID *	m_pguidModuleCLSID;
	const WCHAR *	m_wszDescription;
	DWORD			m_dwVersion;
};




////////////////////////////////////////////////////////////////////
// CTestModuleClassFactory
//
////////////////////////////////////////////////////////////////////
class CTestModuleClassFactory : public IClassFactory //@base public | IClassFactory
{
	protected:				//@Access protected:
		// @cmember Reference Counter
		DWORD				m_cRef;
		GlobalModuleData *	m_pGlobData;

 	public:				  	//@Access public:
		// @cmember Class Constructor
		CTestModuleClassFactory(GlobalModuleData *);
		// @cmember Class Destructor
		virtual ~CTestModuleClassFactory();

        // @cmember Retrieve pointer to specified interface
        STDMETHODIMP 			QueryInterface(REFIID, LPVOID*);
        // @cmember Increment reference count
        STDMETHODIMP_(DWORD) 	AddRef(void);
        // @cmember Decrement reference count
        STDMETHODIMP_(DWORD) 	Release(void);

        // @cmember Create specified object
        STDMETHODIMP         	CreateInstance(LPUNKNOWN, REFIID, LPVOID*);
        // @cmember Toggle lifetime of object
        STDMETHODIMP         	LockServer(BOOL);
};


////////////////////////////////////////////////////////////////////
// CThisTestModule
//
////////////////////////////////////////////////////////////////////
class CThisTestModule : public ITestModule
{
	protected:
		DWORD						m_cRef;
	
	public:
		GlobalModuleData			m_gmd;
		
		SHORT						m_nTestCount;
		BSTR						m_pwszProviderName;
		BSTR						m_pwszProviderFName;		// Provider Friendly Name
		BSTR						m_pwszInitString;
		BSTR						m_pwszMachineName;

		CLSID						m_ProviderClsid;
		CLSCTX						m_clsctxProvider;

		CError *					m_pError;
		IProviderInfo*				m_pIProviderInfo; 
		
		IUnknown *					m_pIUnknown;
		IUnknown *					m_pIUnknown2;

		void *						m_pVoid;
		void *						m_pVoid2;

		CThisTestModule(GlobalModuleData *);
		virtual ~CThisTestModule();

		STDMETHODIMP QueryInterface(REFIID, void **);
		STDMETHODIMP_(DWORD) AddRef(void);
		STDMETHODIMP_(DWORD) Release(void);

		STDMETHODIMP GetName				(BSTR *);
		STDMETHODIMP GetDescription			(BSTR *);
		STDMETHODIMP GetOwnerName			(BSTR *);
		STDMETHODIMP GetCLSID				(BSTR *);
		STDMETHODIMP GetVersion				(LONG *);

		STDMETHODIMP SetProviderInterface	(IProviderInfo *);
		STDMETHODIMP SetErrorInterface		(IError *);

		STDMETHODIMP GetProviderInterface	(IProviderInfo **);
		STDMETHODIMP GetErrorInterface		(IError **);

		STDMETHODIMP Init					(LONG *);
		STDMETHODIMP Terminate				(VARIANT_BOOL *);
		STDMETHODIMP GetCaseCount			(LONG *);
		STDMETHODIMP GetCase				(LONG, ITestCases **);
}; 


////////////////////////////////////////////////////////////////////
// CTestCases
//
////////////////////////////////////////////////////////////////////
class CTestCases : public ITestCases
{
	protected:
		DWORD				m_cRef;

	public:
		DWORD				m_dwTestCaseNumber;
		BSTR				m_pwszCLSID;
		BSTR				m_pwszTestCaseName;
		BSTR				m_pwszTestCaseDesc;	

		CThisTestModule *	m_pThisTestModule;
		CError*				m_pError;

		BSTR	 			m_pwszProviderName;
		BSTR	 			m_pwszProviderFName;
        BSTR	 			m_pwszInitString;
		CLSID				m_ProviderClsid;
		BSTR				m_pwszMachineName;
		CLSCTX				m_clsctxProvider;

		DWORD				m_pIStats;
		DWORD				m_pTmdSpy;

		CTestCases(const WCHAR* pwszTestCaseName);
        virtual ~CTestCases(void);

	private:
		void DeleteProviderInfo(void);

	public:
		STDMETHODIMP QueryInterface(REFIID, void **);
		STDMETHODIMP_(DWORD) AddRef(void);
		STDMETHODIMP_(DWORD) Release(void);

		STDMETHODIMP GetName			(BSTR *);
		STDMETHODIMP GetDescription		(BSTR *);

		STDMETHODIMP SyncProviderInterface	();
		STDMETHODIMP GetProviderInterface(IProviderInfo **);

		STDMETHODIMP GetOwningITestModule(ITestModule **);

		STDMETHODIMP Init				(LONG *);
		STDMETHODIMP Terminate			(VARIANT_BOOL *);
		STDMETHODIMP GetVariationCount	(LONG *);

		STDMETHODIMP ExecuteVariation	(LONG, VARIATION_STATUS *);
		STDMETHODIMP GetVariationDesc	(LONG, BSTR *);
		STDMETHODIMP GetVariationID		(LONG, LONG *);

		HRESULT SetOwningMod(LONG iVariation, CThisTestModule* pCThisTestModule);

		virtual BOOL			Init        (void)		{ return TRUE; }
		virtual BOOL			Terminate   (void)		{ return TRUE; }

		virtual DWORD			GetVarCount(void)		{ assert(FALSE); return 0;		}
		virtual const VARINFO*	GetVarInfoArray(void)	{ assert(FALSE); return NULL;	} 
		virtual const WCHAR*	GetCaseDesc(void)		{ assert(FALSE); return NULL;	}
		virtual const WCHAR*	GetCaseName(void)		{ assert(FALSE); return NULL;	} 

};

#endif // __MODCLASSES_HPP_
