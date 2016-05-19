//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Connection Class Header Module | Declaration of base class for OLE DB Test Modules.
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 10-05-95	Microsoft	Created <nl>
//	[02] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 CModInfo Elements|
//
// @subindex CModInfo|
//
//---------------------------------------------------------------------------

#ifndef __CMODINFO_HPP_
#define __CMODINFO_HPP_


///////////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////////
#include "CTable.hpp"
#include "CParseStrm.hpp"
#include "modstandard.hpp"



///////////////////////////////////////////////////////////////////
// Forwards
//
///////////////////////////////////////////////////////////////////
class CPoolManager;


///////////////////////////////////////////////////////////////////
// Enums
//
///////////////////////////////////////////////////////////////////
enum INSERT_OPTION
{
	INSERT_COMMAND			= 0,		
	INSERT_WITHPARAMS		= 1,	
	INSERT_ROWSETCHANGE		= 2,
	DEFAULT_INSERT			= 3,
};

enum TABLE_OPTION
{
	TABLE_DROPALWAYS			= 0x00000001,
};

enum SERVICECOMP_OPTION
{
	SERVICECOMP_INVOKE			= 0x00000001,
	SERVICECOMP_CURSORENGINE	= 0x00000002,
};



enum DEBUGMODE_OPTION
{
	DEBUGMODE_OFF				= 0x00000000,
	DEBUGMODE_DIALOGS			= 0x00000001,
	DEBUGMODE_MEMORY			= 0x00000002,
	DEBUGMODE_FULL				= 0x00000004,	// Use when normally limited for automation.
	DEBUGMODE_POOLING			= 0x00000010,
};

enum WARNINGLEVEL
{
	WARNINGLEVEL_NONE,	//No Warnings
	WARNINGLEVEL_1,		//Level 1
	WARNINGLEVEL_2,		//Level 2
	WARNINGLEVEL_3,		//Level 3
	WARNINGLEVEL_ALL,	//Level 4
	WARNINGLEVEL_ERROR,	//Treat As Errors
};


enum CREATEDSO_OPTIONS
{
	CREATEDSO_NONE						= 0x00000001,
	CREATEDSO_SETPROPERTIES_NOPASSWORD	= 0x00000002,
	CREATEDSO_SETPROPERTIES				= 0x00000004,
	CREATEDSO_INITIALIZE				= 0x00000008,
};


//--------------------------------------------------------------------
// @class CModInfo | Base Class for connection.
//
// This class is a Provider Info class, containing all info with regards to the 
// provider
//
//--------------------------------------------------------------------
class CModInfo
{
public:
	//constructors
	CModInfo();
	virtual ~CModInfo();	

	//methods
	virtual BOOL  Init(CThisTestModule* pCThisTestModule);

	virtual BOOL  ParseAll();
	virtual BOOL  ParseInitFileName();
	virtual BOOL  ParseInitFile();
	virtual BOOL  ParseInitString();

	virtual BOOL  CreateRootBinder();

	virtual BOOL  GetInitProps(ULONG*	pcPropSets, DBPROPSET**	prgPropSets);
	virtual BOOL  GetInitStringValue(WCHAR* pwszKeyword, WCHAR** ppwszValue);
	static BOOL  GetStringKeywordValue(WCHAR* pwszString, WCHAR* pwszKeyword, WCHAR** ppwszValue);
	virtual BOOL  AddToInitString(CHAR* pszString);
	virtual BOOL  GetFriendlyNameValue(DBPROPID dwPropertyID, WCHAR* pwszName, LONG* plValue);
	static WCHAR* FindStringValue(WCHAR* pwszString, WCHAR* pwszKeyword);

	virtual BOOL	SetTableName(WCHAR* pwszTableName);
	virtual HRESULT	DropTable();
	virtual void	ResetIniFile();
	
	virtual BOOL  SetDefaultQuery(WCHAR* pwszDefaultQuery);
	virtual BOOL  SetRowScopedQuery(WCHAR* pwszRowScopedQuery);
	virtual BOOL  SetInitString(WCHAR* pwszString);
	virtual BOOL  SetRootURL(WCHAR* pwszRootURL);

	virtual HRESULT CreateProvider(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD dwOptions = 0);
	virtual HRESULT CreateProvider(CLSID clsid, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD dwOptions = 0);
	virtual HRESULT InitializeDataSource(IUnknown* pDataSource, DWORD dwOptions = CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE);

	//interface
	inline virtual BOOL	  UseITableDefinition(BOOL flag);
	inline virtual BOOL   IsUsingITableDefinition()			{ return m_fTableCreation;		  }
	inline virtual BOOL   UseIRowsetIndex(BOOL flag);
	inline virtual BOOL   IsUsingIRowsetIndex()				{ return m_fRowsetIndex;		  }

	inline virtual BOOL   IsStrictLeveling()				{ return m_fStrictConformance;	  }
	inline virtual ULONG  GetProviderLevel()				{ return m_dwProviderLevel;		  }
	inline virtual ULONG  SetInsert(ULONG dwInsert)			{ return m_dwInsert = dwInsert;	  }
	inline virtual ULONG  GetInsert()						{ return m_dwInsert;			  }
	inline virtual DWORD  PlugInServiceComponents(DWORD dwVal){ return m_dwServiceComponents = dwVal;}
	inline virtual DWORD  UseServiceComponents()			{ return m_dwServiceComponents;	  }
	inline virtual ULONG  GetWarningLevel()					{ return m_dwWarningLevel;		  }
	inline virtual DWORD  GetDebugMode()					{ return m_dwDebugMode;			  }

	inline virtual WCHAR* GetTableName()					{ return m_pwszTableName;		  }
	inline virtual DWORD  GetTableOpts()					{ return m_dwTableOpts;			  }

	inline virtual WCHAR* GetInitString() 					{ return m_pwszInitString;		  }
	inline virtual WCHAR* GetFileName()						{ return m_pwszFileName;		  }
	inline virtual WCHAR* GetDefaultQuery()					{ return m_pwszDefaultQuery;	  }
	inline virtual WCHAR* GetRowScopedQuery()				{ return m_pwszRowScopedQuery;	  }
	inline virtual WCHAR* GetEnumerator() 					{ return m_pwszEnumerator;		  }

	inline virtual void	  SetProviderCLSID(REFCLSID clsid)	{ m_clsidProvider = clsid;		  }
	inline virtual REFCLSID  GetProviderCLSID() 			{ return m_clsidProvider;		  }
	inline virtual CLSCTX SetClassContext(CLSCTX ClsCtx)	{ return m_ClassContext = ClsCtx; }
	inline virtual CLSCTX GetClassContext() 				{ return m_ClassContext;		  }
	inline virtual WCHAR* SetRemoteMachine(WCHAR* pwszRM)	{ return m_pwszRemoteMachine = pwszRM;}
	inline virtual WCHAR* GetRemoteMachine() 				{ return m_pwszRemoteMachine;		}
	inline virtual EQUERY GetRowsetQuery()					{ return m_eRowsetQuery;			}

	inline virtual CThisTestModule* GetThisTestModule()		{ return m_pCThisTestModule;		}
	inline virtual CError*			GetErrorObject()		{ return m_pCError;					}
	inline virtual CParseInitFile*	GetParseObject()		{ return m_pCParseInitFile;			}
	inline virtual CLocaleInfo*		GetLocaleInfo()			{ return m_pCLocaleInfo;			}
	inline virtual CPoolManager*	GetPoolManager()		{ return m_pCPoolManager;			}

	inline virtual BOOL   SetCompReadOnlyCols(BOOL flag)	{ return m_fCompareReadOnlyCols = flag; }		
	inline virtual BOOL   GetCompReadOnlyCols()				{ return m_fCompareReadOnlyCols; }

	inline virtual WCHAR*			GetRootBinderProgID()	{ return m_pwszRootBinderProgID;}
	inline virtual WCHAR*			GetRootURL()			{ return m_pwszRootURL;}
	inline virtual IBindResource*	GetRootBinder()			{ return m_pIBindResource;}

	inline virtual BOOL   SetUseIntlIdentifier(BOOL flag)	{ return m_fUseIntlIdentifier = flag; }		
	inline virtual BOOL   GetUseIntlIdentifier()			{ return m_fUseIntlIdentifier; }
	inline virtual BOOL   IsWin9x()							{ return m_fIsWin9x; }

	inline virtual const WCHAR* GetBackend()			{ return m_pwszBackend; }
	inline virtual BOOL InitBackendInfo(IDBInitialize *pIDBInitialize = NULL);
	inline virtual WCHAR* GetProviderVer()				{ return m_pwszProviderVer; }

protected:
	BOOL	m_fStrictConformance;
	BOOL	m_fTableCreation;
	BOOL	m_fRowsetIndex;
	DWORD	m_dwDebugMode;
	DWORD	m_dwProviderLevel;
	DWORD	m_dwInsert; 
	DWORD   m_dwServiceComponents;
	DWORD   m_dwWarningLevel;
	BOOL	m_fCompareReadOnlyCols;		
	BOOL	m_fUseIntlIdentifier;

	WCHAR*	m_pwszTableName;
	DWORD	m_dwTableOpts;
	
	WCHAR*	m_pwszFileName;
	WCHAR*	m_pwszDefaultQuery;
	WCHAR*	m_pwszRowScopedQuery;
	WCHAR*	m_pwszEnumerator;

	CLSID	m_clsidProvider;
	CLSCTX	m_ClassContext;
	WCHAR*	m_pwszRemoteMachine;

	WCHAR*	m_pwszBackend;
	LONG	m_MajorBackendVer;
	WCHAR*	m_pwszBackendVer;
	WCHAR*	m_pwszProviderVer;

	//Initialization
	BOOL				m_fParsedProps;
	WCHAR*				m_pwszInitString;
	ULONG				m_cInitPropSets;
	DBPROPSET*			m_rgInitPropSets;
	BOOL				m_fIsWin9x;

	//Rowset Creation
	EQUERY				m_eRowsetQuery;

	CError*				m_pCError;
	CThisTestModule*	m_pCThisTestModule;
	CParseInitFile*		m_pCParseInitFile;

	//Root Binder
	WCHAR*				m_pwszRootBinderProgID;
	WCHAR*				m_pwszRootURL;
	IBindResource*		m_pIBindResource;

	//International Data
	CLocaleInfo*		m_pCLocaleInfo;
	
	//Pooling
	CPoolManager*		m_pCPoolManager;       //Global Pool Manager

private:
};


///////////////////////////////////////////////////////////////////
// Pooling Hooks
//
///////////////////////////////////////////////////////////////////
HRESULT		DisplayPooling(IUnknown* pDataSource, BOOL fEnumPools = FALSE);
DWORD		GetPoolState(IUnknown* pDataSource);
BOOL		DrawnFromPool(IUnknown* pDataSource);
BOOL		CanBePooled(IUnknown* pDataSource);
BOOL		CreatedFromPool(IUnknown* pDataSource);


////////////////////////////////////////////////////////////////////////
// CPool
//
////////////////////////////////////////////////////////////////////////
class CPool
{
public:
	//constructors
	CPool(IUnknown* pObject = NULL);
	virtual ~CPool();

	//helpers
	virtual HRESULT		Create(IUnknown* pObject);
	virtual WCHAR*		GetPoolID();
	virtual DWORD		GetPoolTimeout();
	virtual DWORD		GetRetryTimeout();
	virtual DWORD		GetExpBackOff();
	
	//Interface
	virtual	BOOL		GetHooks()				{ return m_fHooks;		}
	virtual	void		SetHooks(BOOL fHooks)	{ m_fHooks = fHooks;	}

protected:
	//Data
	ULONG				ulInUse;
	ULONG				ulIdle;
	WCHAR*				m_pwszInitString;

	//Can't make this conditional logic, since it will change the interface...
	IUnknown*			m_pUnkPoolInfo;
	BOOL				m_fHooks;
};



////////////////////////////////////////////////////////////////////////
// CPoolManager
//
////////////////////////////////////////////////////////////////////////
class CPoolManager
{
public:
	//constructors
	CPoolManager();
	virtual ~CPoolManager();

	//helpers
	virtual HRESULT		Create();
	virtual HRESULT		DisplayPools();
	virtual HRESULT		ReleaseObject(IUnknown** ppObject);

	virtual HRESULT		EnumPools(ULONG* pcPools, IUnknown** prgpPoolInfo[]);
	virtual HRESULT		FindPool(WCHAR* pwszPoolID, IUnknown** ppPoolInfo);
	virtual HRESULT		ReleasePools(ULONG cPools, IUnknown** rgpPoolInfo);

protected:
	//Data
	//Can't make this conditional logic, since it will change the interface...
	IUnknown*			m_pUnkMngrInfo;
};




#endif // __CMODINFO_HPP_
