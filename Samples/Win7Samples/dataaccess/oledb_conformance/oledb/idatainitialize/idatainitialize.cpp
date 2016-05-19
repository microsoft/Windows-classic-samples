//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc  
//
// @module IDataInitialize.cpp | 
// 		This module tests the Service Component IDataInitialize interface 
//

// disable warning: C4312 Conversion to bigger-size warning. 
// For example, "type cast": conversion from "int" to "int*_ptr64" of greater size. 
#pragma warning(disable: 4312)

//////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include <sqloledb.h>

#include "MODStandard.hpp"
#include "IDataInitialize.h"		// IDataInitialize test header
#include "ExtraLib.h"

#include "MSDASC.H"					// Service Components and DataLink




//////////////////////////////////////////////////////////////////////
// Defines
//
//////////////////////////////////////////////////////////////////////

const DWORD CLSCTX_INVALID	= 0x10000000;
const DWORD CREATE_INVALID	= 0x10000000;
const DWORD DBPROMPTOPTIONS_INVALID	= 0x10000000;

enum PASSWORD_OPTIONS
{
	PASSWORD_EXCLUDED = 0,
	PASSWORD_INCLUDED = 1,
};

#define MAX_ITERATIONS	10

const static GUID IID_IDPOInternal	= { 0x986F3C20, 0x9879, 0x11d1, { 0xBA, 0x17, 0x00, 0xAA, 0x00, 0x0D, 0x96, 0x65 }};




///////////////////////////////////////////////////////////////////
// FileNames
//
///////////////////////////////////////////////////////////////////
#define FILENAME_INVALID  		L"\\/*?\"<>|"	 
#define FILENAME_NOTFOUND		L"abcdefg.hij"
#define FILENAME_INVALIDPATH	L"*"
#define FILENAME_PATH			L"\\"
#define FILENAME_VALID			L"dslsaved.udl"
#define GENERATE_NAME			(INVALID(WCHAR*))


//*-----------------------------------------------------------------------
// Module Values
//*-----------------------------------------------------------------------
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0x1febd102, 0x8cf8, 0x11d1, { 0x95, 0x2b, 0x00, 0xc0, 0x4f, 0xb6, 0x6a, 0x50} };
DECLARE_MODULE_NAME("IDataInitialize");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("IDataInitialize interface test");
DECLARE_MODULE_VERSION(1);
// TCW_WizardVersion(2)
// TCW_Automation(False)
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
	BOOL bReturn = FALSE;
	IDataInitialize* pIDataInitialize = NULL;
	IDBPromptInitialize* pIDBPromptInitialize = NULL;

	//ModuleInit
	if(CommonModuleInit(pThisTestModule))
	{
		//Make sure the random seed is random...
		//For our "random" variations
		srand((UINT)time(NULL));

		//For this test, there must be service components installed, 
		//And the service components, must support these interfaces
		TESTC_(CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER, IID_IDataInitialize, (void**)&pIDataInitialize),S_OK); 

		//Create an Instance of IDBPromptInitialize
		TESTC_(CoCreateInstance(CLSID_DataLinks, NULL, CLSCTX_INPROC_SERVER, IID_IDBPromptInitialize, (void**)&pIDBPromptInitialize),S_OK); 
		bReturn = TRUE;
	}

CLEANUP:
	SAFE_RELEASE(pIDataInitialize);
	SAFE_RELEASE(pIDBPromptInitialize);
	return bReturn;
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
	return CommonModuleTerminate(pThisTestModule);
}


////////////////////////////////////////////////////////////////
// VerifyMultiQI
//
////////////////////////////////////////////////////////////////
BOOL VerifyMultiQI(HRESULT hr, ULONG cMultiQI, MULTI_QI* rgMultiQI)
{
	TBEGIN

	//The following code is always needed whenever calling a method that
	//returns an interface pointer and an HRESULT.  This is fairly trival code
	//but keeps all this common checking in one location
	
	//Verify Results	
	if(hr == CO_S_NOTALLINTERFACES)
	{
		//At least one, (but not all) of the interfaces requested were successfully retrieved. 
		ULONG cSucceeded = 0;
		for(ULONG i=0; i<cMultiQI; i++)
		{
			TESTC(rgMultiQI != NULL);
			if(SUCCEEDED(rgMultiQI[i].hr))
			{
				TESTC(rgMultiQI[i].pItf != NULL);
				cSucceeded++;
			}
			else
			{
				TESTC(rgMultiQI[i].pItf == NULL);
			}
		}
		//Make sure at least one succeeded, but not all succeeded...
		TESTC(cSucceeded >= 1 && cSucceeded < cMultiQI);
	}
	else if(SUCCEEDED(hr))
	{
		//All interfaces requested were successfully retrieved. 
		for(ULONG i=0; i<cMultiQI; i++)
		{
			TESTC(rgMultiQI != NULL);
			TESTC(rgMultiQI[i].pItf != NULL);
			TESTC_(rgMultiQI[i].hr, S_OK);
		}
	}
	else
	{
		//No interfaces requested were successfully retrieved. 
		for(ULONG i=0; i<cMultiQI; i++)
		{
			TESTC(rgMultiQI != NULL);
			TESTC(rgMultiQI[i].pItf == NULL);
			TESTC(FAILED(rgMultiQI[i].hr));
		}
	}

CLEANUP:
	TRETURN
}	


////////////////////////////////////////////////////////////////
// FreeMultiQI
//
////////////////////////////////////////////////////////////////
BOOL FreeMultiQI(ULONG cMultiQI, MULTI_QI* rgMultiQI)
{
	for(ULONG i=0; i<cMultiQI; i++)
		SAFE_RELEASE(rgMultiQI[i].pItf);

	return TRUE;
}


////////////////////////////////////////////////////////////////
// RemoveFile
//
////////////////////////////////////////////////////////////////
BOOL RemoveFile(LPCWSTR pwszFileName)
{
	TBEGIN
	
	//Convert to MBCS - dynamically allocate so we have no limit on size...
	CHAR* pszFileName = ConvertToMBCS(pwszFileName, AreFileApisANSI() ? CP_ACP : CP_OEMCP);
	
	//DeleteFile
	QTESTC(DeleteFileA(pszFileName));

CLEANUP:
	SAFE_FREE(pszFileName)
	TRETURN
}


////////////////////////////////////////////////////////////////
// CreateFile
//
////////////////////////////////////////////////////////////////
HANDLE CreateFile(WCHAR* pwszFileName, WCHAR** ppwszFullPath, DWORD dwCreateOpts = CREATE_ALWAYS, DWORD dwAccess = GENERIC_READ | GENERIC_WRITE, DWORD dwShareMode = FILE_SHARE_READ)
{
	TBEGIN

	CHAR szBuffer[MAX_QUERY_LEN+100];
	szBuffer[0] = '\0';
	int ulStrLen = 0;

	if(ppwszFullPath)
	{
		//Obtain the System Directory
		GetSystemDirectory(szBuffer, MAX_QUERY_LEN+100);

		//Convert to MBCS - dynamically allocate so we have no limit on size...
		strcat(szBuffer, "\\"); 
		ulStrLen = (int)strlen(szBuffer);
	}
	
	ConvertToMBCS(pwszFileName, szBuffer + ulStrLen, MAX_QUERY_LEN-ulStrLen, AreFileApisANSI() ? CP_ACP : CP_OEMCP); 
	if(ppwszFullPath)
		*ppwszFullPath = ConvertToWCHAR(szBuffer);
	
	//CreateFile
	HANDLE hFile = CreateFileA(szBuffer, dwAccess, dwShareMode, NULL, dwCreateOpts, 0, NULL);
	return hFile;
}


////////////////////////////////////////////////////////////////
// MakeUDLFileName
//
////////////////////////////////////////////////////////////////
WCHAR* MakeUDLFileName()
{
	//There are plenty of different approches of creating unique filenames:
	//Do it ourselves, _wtempnam, timestamps, guids, etc...
	//But we are interesting in what consumers might actualy do to create UDL names
	//and we should make sure our own APIs to do this work, so we will use GetTempFileName
	//and friends, although it means a conversion...
/*	CHAR szDir[_MAX_DIR] = {0};
	CHAR szPath[_MAX_PATH] = {0};

	//NOTE: We also want some "variation", so we will use a path sometime, and othertimes
	//will use the current path.
	static ULONG ulSeed = 0;
	ulSeed++;

	//Obtain the temp path (c:\temp - usally)
	//If this fails for some reason just use the current path...
	if(!GetTempPathA(NUMELEM(szDir), szDir))
		szDir[0] = '.';
	
	//GetFileName
	if(!GetTempFileNameA(
		ulSeed%2 ? szDir : ".",							// pointer to directory name for temporary 
		ulSeed%4 ? "Prefix" : "IDataInitialize",		// pointer to filename prefix, NOTE: Win95 fails if this is NULL!
		0,												// number used to create temporary filename
		szPath											// Path
		))
		strcpy(szPath, "dslsaved");						// default

	//GetTempFileName actually creates a file, sigh.  It doesn't if you pass in
	//the "number", but then it doesn't use the timestamp and its no longer unique...
	//So delete the file before preceeding...
	DeleteFileA(szPath);

	//We also want to very the extension used.  By default GetTempFileName
	//appends .tmp to the name.  But we want to test:
	//0 - other extensions (ie: .tmp), 1 - UDL extension (ie: .udl), 3 - no extensions
	if(ulSeed %3)
	{
		CHAR szDrive[_MAX_DRIVE] = {0};
		CHAR szFileName[_MAX_FNAME] = {0};

		//Split the filename into its parts to change the extension
		_splitpath(szPath, szDrive, szDir, szFileName, NULL);
		strcpy(szPath, szDrive);
		strcat(szPath, szDir);
		strcat(szPath, szFileName);
		if(ulSeed %3 == 1)
			strcat(szPath, ".udl");
	}

	//Return the allocated string to the user...
	return ConvertToWCHAR(szPath);
*/
	GUID guid;
	WCHAR* pwszFileName = NULL;

	//NOTE: The above would work really nice, if Win95 actually returned unique filenames!
	//Since it doesn't and causes numerous problems since they may already be in the cache
	//the counts and numbers are all off.  
		
	//The simplest approach is to just use a guid as the filename...
	if(SUCCEEDED(CoCreateGuid(&guid)))
		StringFromCLSID(guid, &pwszFileName);

	return pwszFileName;
}


////////////////////////////////////////////////////////////////
// MakeInitString
//
////////////////////////////////////////////////////////////////
WCHAR* MakeInitString()
{
	//TODO:
	//For now we will just obtain the unqiue string as the filename
	return MakeUDLFileName();
}


//////////////////////////////////////////////////////////////////////////
// GetMachineName
//
//////////////////////////////////////////////////////////////////////////
WCHAR* GetMachineName()
{
	CHAR szComputerName[MAX_COMPUTERNAME_LENGTH + 1];
	WCHAR* pwszComputerName = NULL;
	DWORD dwLen = sizeof(szComputerName);

	if(GetComputerNameA(szComputerName, &dwLen))
		ConvertToWSTR(szComputerName, DBTYPE_STR, &pwszComputerName);

	return pwszComputerName;
}


//////////////////////////////////////////////////////////////////////////
// DisplayProperties
//
//////////////////////////////////////////////////////////////////////////
HRESULT DisplayProperties(IUnknown* pDataSource)
{
	TBEGIN
	HRESULT hr = S_OK;
	
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	IDBProperties* pIDBProperties = NULL;

	//no-op
	if(!pDataSource)
		return S_OK;

	//Obtain the all properties...
	TESTC_(QI(pDataSource, IID_IDBProperties, (void**)&pIDBProperties),S_OK)
	TESTC_(pIDBProperties->GetProperties(0, NULL, &cPropSets, &rgPropSets),S_OK);

	//Now display all the properties
	TESTC(VerifyProperties(S_OK, cPropSets, rgPropSets, FALSE/*fOpenRowset*/, TRUE/*fAlwaysTrace*/));

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBProperties);
	return hr;
}


////////////////////////////////////////////////////////////////
// CPerfTimer
//
////////////////////////////////////////////////////////////////
class CPerfTimer
{
public:
	//constructors
	CPerfTimer() 
	{
		m_lStart.QuadPart	= 0;
		m_lStop.QuadPart	= 0;
	}
	
	virtual ~CPerfTimer() 
	{
	}

	//interface
	inline	BOOL		Start()			{	return QueryPerformanceCounter(&m_lStart);		}
	inline	BOOL		Stop()			{	return QueryPerformanceCounter(&m_lStop);		}
	inline	BOOL		Restart()		{	m_lStop.QuadPart = 0; return Start();			}

	inline	LONGLONG	GetFreq()		{   static LONGLONG sFreq = QueryFrequency();	return sFreq;								}
	inline	LONGLONG	GetDelta()		{   return m_lStop.QuadPart > m_lStart.QuadPart ? m_lStop.QuadPart - m_lStart.QuadPart : 0;	}
	inline	double		GetSeconds()	{   return (double)GetDelta() / (double)GetFreq();											}

	//Helpers
	static  LONGLONG	QueryFrequency()
	{	
		LARGE_INTEGER lFreq = {0}; 
		QueryPerformanceFrequency(&lFreq);
		return lFreq.QuadPart;
	}

protected:
	LARGE_INTEGER	m_lStart;
	LARGE_INTEGER	m_lStop;
};







////////////////////////////////////////////////////////////////
// CUDLInfo
//
////////////////////////////////////////////////////////////////
class CUDLInfo
{
public:
	//constructors
	CUDLInfo(LPCWSTR pwszFileName, LPCWSTR pwszInitString, double dSeconds)
	{
		SetFileName(pwszFileName);
		SetInitString(pwszInitString);
		SetSeconds(dSeconds);
		SetAccessID(0);
		m_fCached			= TRUE;
	}

	virtual ~CUDLInfo() 
	{
	}

	//interface
	LPCWSTR		GetFileName()		{ return m_cstrFileName;	}
	LPCWSTR		GetInitString()		{ return m_cstrInitString;	}
	double		GetSeconds()		{ return m_dSeconds;		}
	DWORD		GetAccessID()		{ return m_dwAccessID;		}
 
	void		SetFileName(LPCWSTR pwszFileName)			{ m_cstrFileName	= pwszFileName;		}
	void		SetInitString(LPCWSTR pwszInitString)		{ m_cstrInitString	= pwszInitString;	}
	void		SetSeconds(double dSeconds)					{ m_dSeconds		= dSeconds;			}
	void		SetAccessID(DWORD dwAccessID)				{ m_dwAccessID		= dwAccessID;		}
	void		SetCached(BOOL fCached)						{ m_fCached			= fCached;			}

protected:
	CWString	m_cstrFileName;
	CWString	m_cstrInitString;
	double		m_dSeconds;
	DWORD		m_dwAccessID;
	BOOL		m_fCached;
};


	
////////////////////////////////////////////////////////////////
// CUDLCache
//
//	Tha main puspose of this cache to to verify that the
//	developerment cache (DSL - UDL File Cache) is functioning correctly.
//
//	Some thought and design actually went into this cache.
//	NOTE: This is not designed toward a development cache, but
//	geared toward proving the simplest, maintainable, and bug free
//	verification cache.  If we have bugs or limitations in our approach
//	we will seariously miss or be unable to test issues with the real system.
//
//	On of the main issues of dev's Cache is that its limited in size,
//	so it has to be compacted based upon LRU.  If we choose a hash table then
//	we have the same issues dev has (since that's there Data Structure choice).
//	If we choose a map, then we have similar issues.  But if we choose a simple
//	stack (linked list) where are new and touched items are on the head, we
//	have a much simiplier good of compacting and different model to verify dev.
// 
////////////////////////////////////////////////////////////////
#define REGKEY_DATAACCESS		L"SOFTWARE\\Microsoft\\DataAccess"

#define REGKEY_POOLING			REGKEY_DATAACCESS L"\\Session Pooling"
#define REGKEY_POOLING_RETRY	L"Retry Wait"
#define DEFAULT_POOLING_RETRY	(64)
#define REGKEY_POOLING_BACKOFF	L"ExpBackOff"
#define DEFAULT_POOLING_BACKOFF	(2)

#define REGKEY_UDL				REGKEY_DATAACCESS L"\\Udl Pooling"
#define REGKEY_UDL_CACHESIZE	L"Cache Size"
#define DEFAULT_UDL_CACHE_SIZE	(60)

class CUDLCache
{
public:
	//constructors
	CUDLCache() 
	{
		m_ulMaxSize			= DEFAULT_UDL_CACHE_SIZE;
		m_ulSavedOrgSize	= -1;
		
		//Obtain the registry, and update if doesn't exist or is 0
		//NOTE: Since this class is static this happens very before anything else (dll load time)
		//so this is always done before the MSDASC object is created, which is when its looked at.
		m_ulSavedOrgSize	= ModifyRegUDLCaching(m_ulMaxSize/*default*/, FALSE/*fOverride*/);
		if(m_ulSavedOrgSize)
			m_ulMaxSize = m_ulSavedOrgSize;
		
		m_dwAccessID	= 0;
	}

	virtual ~CUDLCache()
	{
		RemoveAll();

		//Restore the UDL Cacheing. (if it was modified)
		//NOTE: Since this class is static this happens at (dll load time)
		//Which is waht we want so the value remains changed for successive runs until shutdown...
		if(m_ulSavedOrgSize != -1)
			ModifyRegUDLCaching(m_ulSavedOrgSize, TRUE/*fOverride*/);
	}

	//Array-like methods
	CUDLInfo*	operator[](INDEX iIndex)
	{
		POSITION pos = m_listCUDLInfo.FindIndex(iIndex);
		if(pos)
			return m_listCUDLInfo.GetAt(pos);

		return NULL;
	}
	
	void		RemoveAll()
	{
		//Since we simplified and reduced copies of the objects, by using pointers
		//we need to actually free the objects, when this static objects goes out of scope...
		POSITION pos = m_listCUDLInfo.GetHeadPosition();
		while(pos)
		{
			CUDLInfo* pCUDLInfo = m_listCUDLInfo.GetNext(pos);
			SAFE_DELETE(pCUDLInfo);
		}

		//Empty the list...
		m_listCUDLInfo.RemoveAll();
	}

	//interface
	CUDLInfo*	RemoveUDLInfo(LPCWSTR pwszFileName)
	{
		//Try and find the requested UDL
		POSITION pos = FindUDLInfo(pwszFileName);
		if(pos)
		{
			CUDLInfo* pCUDLInfo = m_listCUDLInfo.GetAt(pos);
			m_listCUDLInfo.RemoveAt(pos);
			return pCUDLInfo;
		}
		
		return NULL;
	}

	ULONG		GetCurrentSize()
	{
		return (ULONG) m_listCUDLInfo.GetCount();
	}

	ULONG		GetMaxCacheSize()
	{
		return m_ulMaxSize;
	}

	DWORD		GetAccessID()
	{
		return m_dwAccessID;
	}

	DWORD		SetAccessID(DWORD dwAccessID)
	{
		return m_dwAccessID = dwAccessID;
	}

	//interface
	POSITION	FindUDLInfo(LPCWSTR pwszFileName)
	{
		POSITION pos = m_listCUDLInfo.GetHeadPosition();
		POSITION posSave = NULL;
		while(pos)
		{
			posSave = pos;	//::GetNext increments the position
			CUDLInfo* pCUDLInfo = m_listCUDLInfo.GetNext(pos);
			
			if(pwszFileName)
			{
				if(wcscmp(pwszFileName, pCUDLInfo->GetFileName())==0)
					return posSave;
			}
			else
			{		
				//Maybe they are both NULL
				if(!pCUDLInfo->GetFileName())
					return posSave;
					
			}
		}

		return NULL;
	}

	CUDLInfo*	ChooseRandomUDL()
	{
		//Choose a random UDL
		//NOTE: This should never be 0 - but we don't want "divide-by-0" exception 
		//as thats not handled so just return NULL
		if(GetCurrentSize())
		{
			ULONG iRand = rand() % GetCurrentSize();
			POSITION pos = m_listCUDLInfo.FindIndex(iRand);
			if(pos)
				return m_listCUDLInfo.GetAt(pos);
		}

		return NULL;
	}

	//Helpers
	CUDLInfo*	GetUDLInfo(LPCWSTR pwszFileName)
	{
		//See if this UDL is in the list
		POSITION pos = FindUDLInfo(pwszFileName);
		if(pos)
		{
			//Make sure we don't grow larger than max size...
			ASSERT(GetCurrentSize() <= GetMaxCacheSize());

			//If the UDL already exists, to improve performance we remove
			//the current occurance and add it to the head (below)...
			CUDLInfo* pCUDLInfo = m_listCUDLInfo.GetAt(pos);
			return pCUDLInfo;
		}

		return NULL;
	}

	void		SetUDLInfo(CUDLInfo* pCUDLInfo, ULONG ulDevCacheSize)
	{
		ASSERT(pCUDLInfo);
		POSITION pos = FindUDLInfo(pCUDLInfo->GetFileName());

		if(pos)
		{
			//If the UDL already exists, update it...
			CUDLInfo* pCUDLPrev = m_listCUDLInfo.GetAt(pos);
			m_listCUDLInfo.SetAt(pos, pCUDLInfo);
			
			//Delete the old one...
			SAFE_DELETE(pCUDLPrev);
		}
		else
		{
			//If the UDL doesn't already exist in the cache, then we need to 
			//potentially compact the cache size, before adding it...

			//If we are about to overflow the cache, compact...
			if(GetCurrentSize() >= m_ulMaxSize)
			{
				DWORD dwMin		= m_listCUDLInfo.GetHead()->GetAccessID();
				DWORD dwMax		= m_listCUDLInfo.GetHead()->GetAccessID();

				//Find the min and max...
				POSITION pos = m_listCUDLInfo.GetHeadPosition();
				while(pos)
				{
					CUDLInfo* pCUDLExpired = m_listCUDLInfo.GetNext(pos);
					dwMin = min(dwMin, pCUDLExpired->GetAccessID());
					dwMax = max(dwMax, pCUDLExpired->GetAccessID());
				}
				
				//Developement has choosen (performance) not to cut a "fixed" number of the
				//last items, but to try and cut a large number if there have been few accesses
				//and a small number if they are all relatively uniform, something like:
				///  (Max (Timestamp) - Min (Timestamp)) * 20%
				ASSERT(dwMax >= dwMin);
				DWORD dwExpired = dwMin + DWORD(0.2 * (dwMax - dwMin));
				
				//Calculate expired items
				pos = m_listCUDLInfo.GetHeadPosition();
				while(pos)
				{
					POSITION posSave = pos;	//::GetNext moves the position
					CUDLInfo* pCUDLExpired = m_listCUDLInfo.GetNext(pos);
					if(pCUDLExpired->GetAccessID() <= dwExpired)
					{
						m_listCUDLInfo.RemoveAt(posSave);
						SAFE_DELETE(pCUDLExpired);
					}
				}
			}

			//Now store the UDL in the Cache
			m_listCUDLInfo.AddHead(pCUDLInfo);
		}
	
		//Make sure we don't grow larger than max size...
		ASSERT(GetCurrentSize() <= GetMaxCacheSize());
	}

	// ModifyRegUDLCaching
	DWORD ModifyRegUDLCaching(DWORD dwNewCacheSize, BOOL fOverride)
	{
		HKEY hKey = NULL;
		DWORD dwCacheSize = 0;
		DWORD cbValue = sizeof(dwCacheSize);
		HRESULT hr = S_OK;

		//Obtain the reg Key, (will create it if it doesn't exist)
		if(S_OK == CreateRegKey(HKEY_LOCAL_MACHINE, REGKEY_UDL, &hKey))
		{
			//Obtain the Current Value for UDL Caching (so we can restore it)
			hr = GetRegEntry(hKey, NULL, REGKEY_UDL_CACHESIZE, &dwCacheSize);
			
			//Now set the new value...
			//NOTE: Only change the setting if it doesn't exits or it 0, so we can play with the 
			//setting without having to re-compile the tests, or have another indicator...
			if(fOverride || hr!=S_OK || dwCacheSize == 0)
				SetRegEntry(hKey, NULL, REGKEY_UDL_CACHESIZE, dwNewCacheSize);
		}

	//CLEANUP:
		if(hKey)
			RegCloseKey(hKey);
		return dwCacheSize;
	}

		
protected:
	ULONG		m_ulMaxSize;
	ULONG 		m_ulSavedOrgSize;
	DWORD		m_dwAccessID;
	
	CList<CUDLInfo*, CUDLInfo*> m_listCUDLInfo;
};


//UDL File Caching
//NOTE: This map is static so we have a lookup table for all testcases and all runs
//since caching is one until the Service Component DLL is unloaded...
//NOTE: We have to use CString since this is the lookup and has a operator== defined.
//If we used just WCHAR* then the only way you could lookup would be if the pointers matched.
static CUDLCache	s_CUDLCache;


	
////////////////////////////////////////////////////////////////
// CDBInit
//
////////////////////////////////////////////////////////////////
class CDBInit : public CDataSource
{
public:
	//constructors
	CDBInit(WCHAR* pwszTestCaseName = INVALID(WCHAR*));
	virtual ~CDBInit();

	//methods
	virtual BOOL	Init();
	virtual BOOL	Terminate();

	//Class Factory
	virtual HRESULT GetClassObject(REFIID riid, IUnknown** ppFactory = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	virtual HRESULT GetClassObject(CLSID clsidProv, REFIID riid, IUnknown** ppFactory = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	virtual HRESULT CreateInstance(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppFactory = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	virtual HRESULT CreateInstance(CLSID clsidProv, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppFactory = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);

	//Helpers
	virtual HRESULT CreateDataSource(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource = NULL, DWORD dwOptions = 0);
	virtual HRESULT CreateDataSource(CLSID clsidProv, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD dwOptions = 0);
	virtual HRESULT CreateDiffProvider(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource = NULL, DWORD dwOptions = 0, CLSID* pCLSID = NULL);

	//CreateDBInstance
	virtual HRESULT CreateDBInstance(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	virtual HRESULT CreateDBInstance(CLSID clsidProv, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	static ULONG WINAPI Thread_CreateDBInstance(LPVOID pv);

	//CreateDBInstanceEx
	virtual HRESULT CreateDBInstanceEx(IUnknown* pIUnkOuter, ULONG cMultiQI, MULTI_QI* rgMultiQI, COSERVERINFO* pServerInfo = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	virtual HRESULT CreateDBInstanceEx(CLSID clsidProv, IUnknown* pIUnkOuter, ULONG cMultiQI, MULTI_QI* rgMultiQI, COSERVERINFO* pServerInfo = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER);
	static ULONG WINAPI Thread_CreateDBInstanceEx(LPVOID pv);

	//GetDataSource
	virtual HRESULT GetDataSource(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD dwCreateOpts, WCHAR* pwszFmt, ...);
	virtual HRESULT GetDataSource(IUnknown* pIUnkOuter, WCHAR* pwszInitString, REFIID riid = IID_IUnknown, IUnknown** ppDataSource = NULL, DWORD clsctx = CLSCTX_INPROC_SERVER, DWORD dwCreateOpts = CREATEDSO_NONE, BOOL fInsideThreads = FALSE);
	static ULONG WINAPI Thread_GetDataSource(LPVOID pv);
	virtual BOOL	VerifyGetDataSource(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType);
	virtual BOOL	VerifyGetDataSource(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue, WCHAR* pwszFmt, ...);
	virtual BOOL	VerifyGetDataSource(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue, REFIID riid, IUnknown** ppDataSource, WCHAR* pwszInitString);
	
	//GetInitializationString
	virtual HRESULT GetInitString(IUnknown* pDataSource, BOOL fIncludePassword, WCHAR** ppwszInitString, BOOL fInsideThreads = FALSE);
	static ULONG WINAPI Thread_GetInitString(LPVOID pv);
	virtual BOOL	VerifyInitString(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue);
	virtual BOOL	VerifyInitString(IUnknown* pDataSource, WCHAR* pwszInitString, BOOL fReturnedString = TRUE, BOOL fInsideThreads = FALSE);

	//UDL File Caching
	virtual BOOL	FullDevUDLCache();
	virtual BOOL	SetDevUDLCacheSize(ULONG ulCacheSize);
	virtual ULONG	GetDevUDLCacheSize();
	virtual BOOL	VerifyDevUDLCache(HRESULT hrExpected, LPCWSTR pwszFileName, CUDLInfo* pCUDLInfo);
	virtual DWORD	GetDevGlobalTickCount();

	//LoadStringFromStorage
	virtual HRESULT LoadStringInternal(LPCWSTR pwszFileName, WCHAR** ppwszInitString);
	virtual HRESULT LoadString(LPCWSTR pwszFileName, WCHAR** ppwszInitString = NULL);
	virtual BOOL	VerifyLoadString(LPCWSTR pwszFileName, LPCWSTR pwszStringExpected);
	virtual BOOL	VerifyPersistedInitString(LPCWSTR pwszInitString, LPCWSTR pwszInitStringReturned);
	static ULONG WINAPI Thread_LoadString(LPVOID pv);
	static ULONG WINAPI	Thread_RandomLoadString(LPVOID pv);

	//WriteStringToStorage
	virtual HRESULT WriteString(LPCWSTR pwszFileName, LPCWSTR pwszInitString = NULL, DWORD dwCreation = CREATE_ALWAYS);
	virtual BOOL	VerifyWriteString(LPCWSTR pwszFileName, LPCWSTR pwszInitString, DWORD dwCreation = CREATE_ALWAYS);
	static ULONG WINAPI Thread_WriteString(LPVOID pv);

	//PromptDataSource
	virtual HRESULT PromptDataSource(IUnknown* pIUnkOuter, HWND hWnd = GetDesktopWindow(), DBPROMPTOPTIONS dwPromptOpts = DBPROMPTOPTIONS_WIZARDSHEET, ULONG cFilters = 0, DBSOURCETYPE* rgFilters = NULL, WCHAR* pwszFilter = NULL, REFIID riid = IID_IDBInitialize, IUnknown** ppDataSource = NULL, BOOL fInsideThreads = FALSE);
	static ULONG WINAPI Thread_PromptDataSource(LPVOID pv);

	virtual HRESULT PromptFileName(HWND hWnd = GetDesktopWindow(), DBPROMPTOPTIONS dwPromptOpts = DBPROMPTOPTIONS_NONE, WCHAR* pwszInitDir = NULL, WCHAR* pwszInitFile = NULL, WCHAR** ppwszSelectedFile = NULL);
	static ULONG WINAPI Thread_PromptFileName(LPVOID pv);

	virtual HRESULT	AppendToInitString(WCHAR** ppwszInitString, DBPROPID dwPropertyID, GUID guidPropertySet, VARIANT* pVariant);
	virtual WCHAR*  CreatePropString(ULONG cPropSets, DBPROPSET* rgPropSets, BOOL fIncludeProvider = TRUE);
	virtual WCHAR*	CreateInitPropString(BOOL fIncludeProvider = TRUE);
	
	virtual BOOL	VerifyCompleteSequence(DWORD dwOptions, BOOL fPasswordIncluded = PASSWORD_INCLUDED);

	//Pooling
	virtual HRESULT	CreatePool(WCHAR* pwszInitString);
	static ULONG WINAPI Thread_CreatePool(LPVOID pv);

	//Properties
	virtual HRESULT SetInitProps(IUnknown* pDataSource);
	virtual HRESULT SetProperties(IUnknown* pDataSource, ULONG cPropSets, DBPROPSET* rgPropSets);
	virtual HRESULT GetProperties(IUnknown* pDataSource, ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropSets, DBPROPSET** prgPropSets);
	virtual BOOL	CompareProperties(IUnknown* pDataSource, ULONG cPropSets, DBPROPSET* rgPropSets, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions = EXACT_MATCH);
	virtual BOOL	CompareProperties(IUnknown* pDataSource, IUnknown* pDataSource2, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions = EXACT_MATCH);


	virtual BOOL	IsUninitialized(IUnknown* pDataSource);
	virtual BOOL	IsInitialized(IUnknown* pDataSource, BOOL fQuick = TRUE/*FALSE*/);
	virtual BOOL	VerifyInitialize(IUnknown* pDataSource, BOOL fRegPropsSet = TRUE);

	virtual BOOL	VerifyPassword(IUnknown* pDataSource, BOOL fIncludePassword, WCHAR* pwszInitString, BOOL fReturnedString = TRUE, BOOL fAllowUnitialize = FALSE);
	virtual BOOL	GetSomePropValue(VARIANT* pVariant, DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, BOOL fValid = TRUE);
	virtual BOOL	IsDefaultValue(IUnknown* pDataSource, DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue);
	virtual BOOL	AreDefaultValues(IUnknown* pIUnknown, ULONG cPropSets, DBPROPSET* rgPropSets);
	virtual BOOL	IsDPO(IUnknown* pDataSource);
	virtual BOOL	IsDCM(IUnknown* pDataSource);

	//Additional Testing
	virtual HRESULT	SetHiddenInitProperties(IUnknown* pDataSource);

	//interfaces
	virtual IDataInitialize*		const pDataInit();
	virtual IDBPromptInitialize*	const pDBPromptInit();
	virtual IDBInitialize*			const pDBInitSC();

	virtual WCHAR*					const GetProgID();

protected:
	IDataInitialize*		m_pIDataInitialize;
	IDBPromptInitialize*	m_pIDBPromptInitialize;
	IDBInitialize*			m_pIDBInitializeSC;

	WCHAR*					m_pwszProgID;
	WCHAR*					m_pwszProperties;
	BOOL					m_fReqInitProps;
	BOOL					m_fTestingHooks;


private:
};


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CDBInit
//
////////////////////////////////////////////////////////////////////////////
CDBInit::CDBInit(WCHAR * wstrTestCaseName)	: CDataSource(wstrTestCaseName) 
{
	//Interfaces
	m_pIDataInitialize		= NULL;
	m_pIDBPromptInitialize	= NULL;
	m_pIDBInitializeSC		= NULL;
	m_fTestingHooks			= TRUE;


	m_pwszProgID = NULL;
	m_pwszProperties = NULL;
	m_fReqInitProps = FALSE;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::~CDBInit
//
////////////////////////////////////////////////////////////////////////////
CDBInit::~CDBInit()
{
	//Interfaces
	SAFE_RELEASE(m_pIDataInitialize);
	SAFE_RELEASE(m_pIDBPromptInitialize);
	SAFE_RELEASE(m_pIDBInitializeSC);


	SAFE_FREE(m_pwszProgID);
	SAFE_FREE(m_pwszProperties);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::Init
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::Init()
{
	TBEGIN
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
		
	//Create an Instance of IDataInitialize
	TESTC_(CreateInstance(CLSID_MSDAINITIALIZE, NULL, IID_IDataInitialize, (IUnknown**)&m_pIDataInitialize),S_OK); 

	
	//Create an Instance of IDBPromptInitialize
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBPromptInitialize, (IUnknown**)&m_pIDBPromptInitialize),S_OK); 

	//Create a DataSource with SC
	TESTC_(m_pIDataInitialize->CreateDBInstance(PROVIDER_CLSID, NULL, CLSCTX_INPROC_SERVER, NULL, IID_IDBInitialize, (IUnknown**)&m_pIDBInitializeSC),S_OK);
	
	//Init CDataSource (inhertied class)
	TESTC(CDataSource::Init());

	//Initialize the DataSource...
	TESTC_(CDataSource::Initialize(),S_OK);

	//Just so we have a file existing, save the curret InitString
	TESTC_(WriteString(GENERATE_NAME),S_OK);

	//Obtain Provider ProgID (for init string parsing)
	TESTC_(ProgIDFromCLSID(PROVIDER_CLSID, &m_pwszProgID),S_OK);

	//Obtain Complete String for All Properties
	TESTC(GetInitProps(&cPropSets, &rgPropSets));
	m_pwszProperties = CreatePropString(cPropSets, rgPropSets);

	//Inidicates if this provider requires Initialization properties
	//This is used when trying to connect using no properties (Error conditions)
	m_fReqInitProps = cPropSets > 0;

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::pIDataInit
//
////////////////////////////////////////////////////////////////////////////
IDataInitialize*  const CDBInit::pDataInit()
{
	ASSERT(m_pIDataInitialize);
	return m_pIDataInitialize;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::pIDBPromptInit
//
////////////////////////////////////////////////////////////////////////////
IDBPromptInitialize*  const CDBInit::pDBPromptInit()
{
	ASSERT(m_pIDBPromptInitialize);
	return m_pIDBPromptInitialize;
}



////////////////////////////////////////////////////////////////////////////
//  CDBInit::pIDBInitSC
//
////////////////////////////////////////////////////////////////////////////
IDBInitialize*  const CDBInit::pDBInitSC()
{
	ASSERT(m_pIDBInitializeSC);
	return m_pIDBInitializeSC;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetProgID
//
////////////////////////////////////////////////////////////////////////////
WCHAR*  const CDBInit::GetProgID()
{
	ASSERT(m_pwszProgID);
	return m_pwszProgID;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetClassObject
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::GetClassObject(REFIID riid, IUnknown** ppFactory, DWORD clsctx)
{
	//Delegate
	return GetClassObject(CLSID_MSDAINITIALIZE, riid, ppFactory, clsctx);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetClassObject
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::GetClassObject(CLSID clsidProv, REFIID riid, IUnknown** ppFactory, DWORD clsctx)
{
	TBEGIN
	HRESULT hr = S_OK;
	IUnknown* pFactory = NULL;

	//CoGetClassObject
	hr = CoGetClassObject(clsidProv, clsctx, NULL, riid, (void**)&pFactory);
	TESTC(VerifyOutputInterface(hr, riid, &pFactory));

CLEANUP:
	if(ppFactory)
		*ppFactory = pFactory;
	else
		SAFE_RELEASE(pFactory);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateInstance
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateInstance(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD clsctx)
{
	//Delegate
	return CreateInstance(CLSID_MSDAINITIALIZE, pIUnkOuter, riid, ppIUnknown, clsctx);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateInstance
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateInstance(CLSID clsidProv, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppIUnknown, DWORD clsctx)
{
	TBEGIN
	HRESULT hr = S_OK;
	IUnknown* pIUnknown = NULL;

	//CoCreateInstance
	hr = CoCreateInstance(clsidProv, pIUnkOuter, clsctx, riid, (void**)&pIUnknown);
	TESTC(VerifyOutputInterface(hr, riid, &pIUnknown));

	if(!ppIUnknown && SUCCEEDED(hr))
	{
		//If the user doesn't want this interface back, we can do additional for pooling
		TESTC_(ReleaseDataSource(&pIUnknown),S_OK);
	}

CLEANUP:
	if(ppIUnknown)
		*ppIUnknown = pIUnknown;
	else
		SAFE_RELEASE(pIUnknown);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDBInstance
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDBInstance(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD clsctx)
{
	//Delegtion
	return CreateDBInstance(PROVIDER_CLSID, pIUnkOuter, riid, ppDataSource, clsctx);
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDBInstance
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDBInstance(CLSID clsidProv, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD clsctx)
{
	TBEGIN
	HRESULT hr = S_OK;
	IUnknown* pDataSource = NULL;

	//CreateDBInstance
	hr = pDataInit()->CreateDBInstance(clsidProv, pIUnkOuter, clsctx, NULL, riid, &pDataSource);
	TESTC(VerifyOutputInterface(hr, riid, &pDataSource));
	
CLEANUP:
	if(ppDataSource)
		*ppDataSource = pDataSource;
	else
		SAFE_RELEASE_DSO(pDataSource);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDBInstanceEx
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDBInstanceEx(IUnknown* pIUnkOuter, ULONG cMultiQI, MULTI_QI* rgMultiQI, COSERVERINFO* pServerInfo, DWORD clsctx)
{
	//Delegtion
	return CreateDBInstanceEx(PROVIDER_CLSID, pIUnkOuter, cMultiQI, rgMultiQI, pServerInfo, clsctx);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDBInstanceEx
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDBInstanceEx(CLSID clsidProv, IUnknown* pIUnkOuter, ULONG cMultiQI, MULTI_QI* rgMultiQI, COSERVERINFO* pServerInfo, DWORD clsctx)
{
	TBEGIN
	HRESULT hr = S_OK;
	ULONG i=0;

	//CreateDBInstanceEx
	hr = pDataInit()->CreateDBInstanceEx(clsidProv, pIUnkOuter, clsctx, NULL, pServerInfo, cMultiQI, rgMultiQI);
	
	//Verify Results
	//The only exception is that we may be testing E_INVALIDARG
	//combinations in which the underlying array doesn't have to be filled out...
	if(hr != E_INVALIDARG)
		TESTC(VerifyMultiQI(hr, cMultiQI, rgMultiQI));
	
CLEANUP:
	return hr;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_CreateDBInstance
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_CreateDBInstance(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis		= (CDBInit*)THREAD_FUNC;
	IID* pIID			= (IID*)THREAD_ARG1;
	HRESULT hrExpected	= (HRESULT)(LONG_PTR)THREAD_ARG2;
	ASSERT(pThis && pIID);

	//Local Variables
	ThreadSwitch(); //Let the other thread(s) catch up

	//CreateDBInstance (helper)
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		//Since we have no use for the returned DataSource, just pass in NULL
		//and let the function do additional testing on the datasource returned...
		TESTC_(pThis->CreateDBInstance(NULL, *pIID, NULL), hrExpected);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_CreateDBInstanceEx
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_CreateDBInstanceEx(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis		= (CDBInit*)THREAD_FUNC;
	ULONG cIIDs			= (ULONG)THREAD_ARG1;
	IID* rgIIDs			= (IID*)THREAD_ARG2;
	HRESULT hrExpected	= (HRESULT)THREAD_ARG3;
	ASSERT(pThis);
	ULONG i=0;

	ThreadSwitch(); //Let the other thread(s) catch up

	//Setup the MultiQI structures
	ULONG cMultiQI = cIIDs;
	MULTI_QI rgMultiQI[MAX_THREADS];
	for(i=0; i<cIIDs; i++)
	{
		rgMultiQI[i].pIID	= &rgIIDs[i];
		rgMultiQI[i].pItf	= NULL;
		rgMultiQI[i].hr		= S_OK;
	}

	//CreateDBInstanceEx (helper)
	for(i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(pThis->CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), hrExpected);

		//Loop through the returned DataSources.
		//And do additional testing...
		for(ULONG iDSO=0; iDSO<cMultiQI; iDSO++)
		{
			if(SUCCEEDED(rgMultiQI[iDSO].hr))
				TESTC_(ReleaseDataSource(&rgMultiQI[iDSO].pItf),S_OK);
		}
		
		//Free Returned interfaces
		FreeMultiQI(cMultiQI, rgMultiQI);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	FreeMultiQI(cMultiQI, rgMultiQI);
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::GetDataSource(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD dwCreateOpts, WCHAR* pwszFmt, ...)
{
	HRESULT hr = S_OK;

	//Process all varible input args...
	va_list		marker;
	WCHAR		wszInitString[MAX_QUERY_LEN+100];
	wszInitString[0] = L'\0';

	if(pwszFmt)
	{
		// Use format and arguments as input
		//This version will not overwrite the stack, since it only copies
		//upto the max size of the array
		va_start(marker, pwszFmt);
		_vsnwprintf(wszInitString, MAX_QUERY_LEN+100, pwszFmt, marker);
		va_end(marker);
	
		//Make sure there is a NULL Terminator, vsnwprintf will not copy
		//the terminator if length==MAX_QUERY_LEN
		wszInitString[MAX_QUERY_LEN+100-1] = L'\0';
	}
	else
	{
		//Use the Default InitString
		wcscpy(wszInitString, m_pwszProperties);
	}

	//Delegate
	//Use the Default Init String
	QTESTC_(hr = GetDataSource(pIUnkOuter, wszInitString, riid, ppDataSource, CLSCTX_INPROC_SERVER, dwCreateOpts),S_OK);

CLEANUP:
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::GetDataSource(IUnknown* pIUnkOuter, WCHAR* pwszInitString, REFIID riid, IUnknown** ppDataSource, DWORD clsctx, DWORD dwCreateOpts, BOOL fInsideThreads)
{
	TBEGIN
	HRESULT hr = S_OK;
	IUnknown* pDataSource = ppDataSource ? *ppDataSource : NULL;

	//Input DataSource
	IUnknown* pIUnkInput = pDataSource;				//RefCount = 1
	SAFE_ADDREF(pIUnkInput);						//RefCount = 2
	ULONG ulOrgRefCount = GetRefCount(pIUnkInput);	

	//GetDataSource
	hr = pDataInit()->GetDataSource(pIUnkOuter, clsctx, pwszInitString, riid, &pDataSource);
	TRACE(L"  GetDataSource(\"%s\") - 0x%08x\n", pwszInitString, hr);

	//Success 
	if(SUCCEEDED(hr))
	{
		//*ppDataSource!=NULL on input	
		if(pIUnkInput)
		{
			TESTC(pDataSource != NULL);

			//DSL is supposed to QI the previous pointer,
			//And call release on the input, so the net result is no change...
			//If were in multiple threads the refcount may be incemented by other threads
			//for itmes such as GetProperties (IDBProperties) or other reasons...
			if(!fInsideThreads)
				TESTC(ulOrgRefCount == GetRefCount(pDataSource));

			//The output pointer may not be the same as the input, if they were
			//not the same REFIID, since you can ask for a different one on output.
			//We should make sure that the IUnknowns of the objects are the same...
			TESTC(VerifyEqualInterface(pIUnkInput, pDataSource));
			
			//Verify the InitString
			//Not a "exact" comparision (FALSE), since we called GetDataSource
			//From the string, which could return additional properties
			TESTC(VerifyInitString(pDataSource, pwszInitString, FALSE, fInsideThreads));
		}
		else
		{
			TESTC(pDataSource != NULL);
			TESTC(pIUnkInput == NULL);

			//Verify the InitString
			//Not a "exact" coparision (FALSE), since we called GetDataSource
			//From the string, which could return additional properties
			TESTC(VerifyInitString(pDataSource, pwszInitString, FALSE, fInsideThreads));
		}

		//Create Options
		if(dwCreateOpts)
		{
			QTESTC_(hr = InitializeDataSource(pDataSource, dwCreateOpts),S_OK);
		}

	}
	else
	{
		TESTC(pDataSource == pIUnkInput);
		if(!fInsideThreads)
			TESTC(ulOrgRefCount == GetRefCount(pIUnkInput));
	}

CLEANUP:
	SAFE_RELEASE(pIUnkInput);
	if(ppDataSource)
		*ppDataSource = pDataSource;
	else
		SAFE_RELEASE_DSO(pDataSource);
	return hr;
}
	

////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_GetDataSource
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_GetDataSource(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis = (CDBInit*)THREAD_FUNC;
	WCHAR* pwszInitString = (WCHAR*)THREAD_ARG1;
	IUnknown* pDataSource = (IUnknown*)THREAD_ARG2;
	HRESULT hrExpected = (HRESULT)THREAD_ARG3;
	ASSERT(pThis && pwszInitString);

	ThreadSwitch(); //Let the other thread(s) catch up
	
	//Make a local copy so were not destroying copies for other threads.
	IDBInitialize* pIDBInitialize = (IDBInitialize*)pDataSource;

	//GetDataSource (helper)
	//TRUE - indicate we are calling this inside a thread - so refcount is not so strict...
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(pThis->GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER, CREATEDSO_NONE/*dwCreateOpts*/, TRUE/*fInsideThreads*/), hrExpected);
		if(pDataSource == NULL)
		{
			SAFE_RELEASE_DSO(pIDBInitialize);
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreatePool
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreatePool(WCHAR* pwszInitString)
{
	TBEGIN
	HRESULT hr = S_OK;
	IDBInitialize* pIDBInitialize = NULL;

	//Ensure that we have a pool for this type of datasource...
	//The simplest way to do this is to just create a release (1+ number of CPUs) DataSources
	//so the next DataSource will be drawn from the pool...
	for(ULONG i=0; i<9; i++)
	{
		//Obtain the DataSource
		TESTC_(hr = GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
		
		//Initialize so it can be pooled...
		TESTC(VerifyInitialize(pIDBInitialize));

		//Add this DataSource to our internal pool tracker
		TESTC_(ReleaseDataSource((IUnknown**)&pIDBInitialize),S_OK);
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	return hr;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_CreatePool
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_CreatePool(void* pv)
{
	//Thread Stack Variables
	CDBInit* pThis			= (CDBInit*)THREAD_FUNC;
	BOOL fFreeThreaded		= (BOOL)THREAD_ARG1;
	WCHAR* pwszInitString	= (WCHAR*)THREAD_ARG2;
	HRESULT hrExpected		= (HRESULT)THREAD_ARG3;
	ASSERT(pThis && pwszInitString);

	//Threading Model
	THREAD_BEGIN_(fFreeThreaded);
	ThreadSwitch(); //Let the other thread(s) catch up
	
	//Make a local copy so were not destroying copies for other threads.
	IDBInitialize* pIDBInitialize = NULL;

	//GetDataSource (helper)
	//TRUE - indicate we are calling this inside a thread - so refcount is not so strict...
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(pThis->GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER, CREATEDSO_NONE/*dwCreateOpts*/, TRUE/*fInsideThreads*/), hrExpected);
//		TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), hrExpected);
		SAFE_RELEASE_DSO(pIDBInitialize);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetInitString
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::GetInitString(IUnknown* pDataSource, BOOL fIncludePassword, WCHAR** ppwszInitString, BOOL fInsideThreads)
{
	ASSERT(ppwszInitString);
	TBEGIN
	HRESULT hr = S_OK;
	ULONG ulOrgRefCount = GetRefCount(pDataSource);	

	//GetInitializationString
	hr = pDataInit()->GetInitializationString(pDataSource, (boolean)fIncludePassword, ppwszInitString);
	if(SUCCEEDED(hr))
	{
		TESTC(*ppwszInitString != NULL);

		// TESTC(wcslen(*ppwszInitString) >= 0);
		// Fix for new compile tool
		TESTC((wcslen(*ppwszInitString) > 0 ||
			wcslen(*ppwszInitString)==0));  //"Verifies" valid string

		TRACE(L"  GetInitString(\"%s\") = 0x%08x\n", *ppwszInitString, hr);

		//Verify the Password is included or excluded...
		TESTC(VerifyPassword(pDataSource, fIncludePassword, *ppwszInitString));
	}
	else
	{
		TESTC(*ppwszInitString == NULL);
	}

	//Just make sure there were no additional refcounts taken on pDataSource
	if(!fInsideThreads)
		TESTC(ulOrgRefCount == GetRefCount(pDataSource));
	
CLEANUP:
	return hr;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_GetInitString
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_GetInitString(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis = (CDBInit*)THREAD_FUNC;
	IUnknown* pDataSource	= (IUnknown*)THREAD_ARG1;
	BOOL fIncludePassword	= (BOOL)THREAD_ARG2;
	WCHAR** ppwszInitString = (WCHAR**)THREAD_ARG3;
	HRESULT hrExpected		= (HRESULT)THREAD_ARG4;
	ASSERT(pThis && pDataSource && ppwszInitString);

	ThreadSwitch(); //Let the other thread(s) catch up

	//GetInitString (helper)
	//TRUE - indicate we are calling this inside a thread - so refcount is not so strict...
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(pThis->GetInitString(pDataSource, fIncludePassword, ppwszInitString, TRUE), hrExpected);

		//Release all but the last iteration, which gets returned to the user...
		if(i<MAX_ITERATIONS-1)
			SAFE_FREE(*ppwszInitString);
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::FullDevUDLCache
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::FullDevUDLCache()
{
	TBEGIN
	ULONG i,cCurSize	= s_CUDLCache.GetCurrentSize();
	ULONG cMaxSize		= s_CUDLCache.GetMaxCacheSize();

	//Make sure the cache is full (if not already)
	for(i=cCurSize; cCurSize<cMaxSize; cCurSize++)
	{
		TESTC(VerifyWriteString(GENERATE_NAME, GENERATE_NAME));
	}

CLEANUP:
	TRETURN
}


	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::SetDevUDLCacheSize
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::SetDevUDLCacheSize(ULONG ulCacheSize)
{
	TBEGIN
	HRESULT hr = E_NOTIMPL;


	if(hr==E_NOTIMPL)
	{
		if(m_fTestingHooks)
		{
			//NOTE: Testing hooks are not available in Retail
			TWARNING(L"Unable to Set UDL Cache Size, hooks not available...");
			m_fTestingHooks = FALSE;
		}
		
		QTESTC(FALSE);
	}

CLEANUP:
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetDevUDLCacheSize
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::GetDevUDLCacheSize()
{
	TBEGIN
	DWORD dwCacheSize = s_CUDLCache.GetCurrentSize();
	HRESULT hr = E_NOTIMPL;


	if(hr==E_NOTIMPL)
	{
		//NOTE: Testing hooks are not available in Retail drops.
		if(m_fTestingHooks)
		{
			TWARNING(L"Unable to Obtain True UDL Cache Size, hooks not available...");
			m_fTestingHooks = FALSE;
		}
	}

	return dwCacheSize;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetDevGlobalTickCount
//
////////////////////////////////////////////////////////////////////////////
DWORD CDBInit::GetDevGlobalTickCount()
{
	TBEGIN
	DWORD dwAccessID = s_CUDLCache.GetAccessID();
	//NOTE: This functionality is not available in Retail drops.


	return dwAccessID;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyDevUDLCache
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyDevUDLCache(HRESULT hrExpected, LPCWSTR pwszFileName, CUDLInfo* pCUDLInfo)
{
	TBEGIN
	WCHAR* pwszInitString = NULL;
	DWORD dwTickCount = 0;
	HRESULT hr = E_NOTIMPL;


	if(hr==E_NOTIMPL)
	{
		//NOTE: Testing hooks are not available in Retail drops.
		if(m_fTestingHooks)
		{
			TWARNING(L"Unable to verify UDL File caching, hooks not available...");
			m_fTestingHooks = FALSE;
		}
	}

	SAFE_FREE(pwszInitString);
	TRETURN
}

		
////////////////////////////////////////////////////////////////////////////
//  CDBInit::LoadStringInternal
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::LoadStringInternal(LPCWSTR pwszFileName, WCHAR** ppwszInitString)
{
	HRESULT hr = S_OK;
	CUDLInfo* pCUDLInfo = NULL;

	//Service Components current implementation increments the latest
	//dwAccessID (dwTickCount) ALWAYS (even on failure), except in the cases that
	//one of the string passed in is NULL.  
	//TODO: Once this bug is fixed, move the "increment" logic to a if(SUCCEEDED(hr)) clause

	if(pwszFileName && ppwszInitString)
	{
		s_CUDLCache.SetAccessID(s_CUDLCache.GetAccessID() + 1);
	}

	//LoadStringFromStorage
	hr = pDataInit()->LoadStringFromStorage(pwszFileName, ppwszInitString);
	if(SUCCEEDED(hr))
	{
	}

	pCUDLInfo = s_CUDLCache.GetUDLInfo(pwszFileName);
	if(pCUDLInfo)
	{
		pCUDLInfo->SetAccessID(s_CUDLCache.GetAccessID());
	}

	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::LoadString
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::LoadString(LPCWSTR pwszFileName, WCHAR** ppwszInitString)
{
	TBEGIN
	HRESULT hr = E_FAIL;
	WCHAR* pwszInitString = NULL;
	CPerfTimer perfTimer;

	//Before calling LoadStringFromStorage we need to do some verification
	//on cached UDLs (before it gets added to the cache by Load)

	//Lookup in our test cache to see if the UDL should be Cached
	//NOTE: Don't modify the list if we are within threads...
	CUDLInfo* pCUDLInfo = s_CUDLCache.GetUDLInfo(pwszFileName);
	if(pCUDLInfo)
	{
		//Use private test hook, to make sure that DSL also
		//has this FileName UDL in the cache...
		TESTC(VerifyDevUDLCache(S_OK/*cached*/, pwszFileName, pCUDLInfo));
	}
	else
	{
		//Use private test hook, to make sure that DSL also
		//doesn't have this FileName UDL in the cache...
		TESTC(VerifyDevUDLCache(S_FALSE/*not cached*/, pwszFileName, pCUDLInfo));
	}
	
	//Now actually call LoadStringFromStorage (and time it)
	perfTimer.Start();
	hr = LoadStringInternal(pwszFileName, &pwszInitString);
	perfTimer.Stop();
	
	//Verify results
	if(SUCCEEDED(hr))
	{
		TESTC(pwszInitString != NULL);
	
		//If this string doesn't already exits in our map, then add it.
		//So we can keep track of the cached UDL files, and their values...
		if(!pCUDLInfo)
		{
			pCUDLInfo = new CUDLInfo(pwszFileName, pwszInitString, perfTimer.GetSeconds());
			s_CUDLCache.SetUDLInfo(pCUDLInfo, GetDevUDLCacheSize());
			SAFE_FREE(pwszInitString);

			//Now to make sure we test this UDL file can be obtained from the cache.
			//Call LoadStringFromStorage again, and verify its pulled from the cache...
			//NOTE: Make sure it was the same hresult as the first time...
			perfTimer.Start();
			TESTC_(LoadStringInternal(pwszFileName, &pwszInitString), hr);
			perfTimer.Stop();

			TESTC(pwszInitString != NULL);
		}

		//Make sure the InitStrings are the same...
		TESTC(VerifyPersistedInitString(pwszInitString, pCUDLInfo->GetInitString()));

		//Use private test hook, to make sure that DSL also
		//has this FileName UDL in the cache...
		TESTC(VerifyDevUDLCache(S_OK/*cached*/, pwszFileName, pCUDLInfo));
		

	}
	else
	{
		TESTC(pwszInitString == NULL);

		//Use private test hook, to make sure that DSL also
		//doesn't have this FileName UDL in the cache...
		TESTC(VerifyDevUDLCache(S_FALSE/*not cached*/, pwszFileName, pCUDLInfo));

		//Make sure this file is not in the cache...
		TESTC(!s_CUDLCache.GetUDLInfo(pwszFileName));

		//Just as a sanity check, make sure the name specified doesn't end up in the cache
		//on failure.  The easiest way to do this is to always call LoadStringFromStorage again
		//NOTE: Make sure it was the same hresult as the first time...
		TESTC_(LoadStringInternal(pwszFileName, &pwszInitString), hr);
		TESTC(pwszInitString == NULL);
	}

CLEANUP:
	if(ppwszInitString)
		*ppwszInitString = pwszInitString;
	else
		SAFE_FREE(pwszInitString);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyLoadString
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyLoadString(LPCWSTR pwszFileName, LPCWSTR pwszStringExpected)
{
	TBEGIN
	HRESULT hr = S_OK;
	WCHAR* pwszInitStringReturned = NULL;
	WCHAR* pwszEnd = NULL;
	CHAR*  pszFileName = NULL;
	CHAR   szAliasName[_MAX_PATH];

	//LoadString
	TESTC_(LoadString(pwszFileName, &pwszInitStringReturned),S_OK);

	//Verify the Strings Match
	TESTC(VerifyPersistedInitString(pwszStringExpected, pwszInitStringReturned));

	//We need to also make sure that we can load the Short File Name version
	//of the file as well, so this gets tested for all our combinations of the filenames...
	//NOTE: Not all names have an equivilent alias name, UNC, etc...
	pszFileName = ConvertToMBCS(pwszFileName, AreFileApisANSI() ? CP_ACP : CP_OEMCP);
	if(pszFileName && GetShortPathName(pszFileName, szAliasName, _MAX_PATH))
	{
		//LoadString
		SAFE_FREE(pwszInitStringReturned);
		TESTC_(LoadString(pwszFileName, &pwszInitStringReturned),S_OK);

		//Verify the Strings Match
		TESTC(VerifyPersistedInitString(pwszStringExpected, pwszInitStringReturned));
	}
	
CLEANUP:
	SAFE_FREE(pwszInitStringReturned);
	SAFE_FREE(pszFileName);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_LoadString
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_LoadString(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis		= (CDBInit*)THREAD_FUNC;
	CUDLInfo* pCUDLInfo	= (CUDLInfo*)THREAD_ARG1;
	HRESULT	hrExpected	= (HRESULT)THREAD_ARG2;
	ASSERT(pThis);	

	//Locals
	HRESULT hr = S_OK;
	WCHAR* pwszInitString = NULL;
	
	//NOTE: pCUDLInfo can be null if wanting to use a unqiuq file name
	if(!pCUDLInfo)
		pCUDLInfo = s_CUDLCache.ChooseRandomUDL();

	ThreadSwitch(); //Let the other thread(s) catch up

	//LoadString (helper)
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		if(hrExpected != S_OK)
		{
			//Some variations we have Write and Load called from numerous threads, so 
			//there is now good way to verify what should be cached vs. non-cached story since
			//numerous things are hitting at one time.  We could simply solve this by blocking
			//in our helper, but then we just serialized all access and won't find any crash in
			//the provider.  Better to to least run Load and Write simulataniously and
			//worry about caching in more "orgainized" variations, like threads that do all loads
			//or all write, or other mixed single threaded senarios...
			TEST2C_(hr = pThis->LoadStringInternal(pCUDLInfo->GetFileName(), &pwszInitString),S_OK, hrExpected);
		}
		else
		{
			TESTC_(hr = pThis->LoadString(pCUDLInfo->GetFileName(), &pwszInitString), S_OK);
		}

		if(SUCCEEDED(hr))
		{
			TESTC(pThis->VerifyPersistedInitString(pCUDLInfo->GetInitString(), pwszInitString));
			SAFE_FREE(pwszInitString);
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_FREE(pwszInitString);
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_RandomLoadString
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_RandomLoadString(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis	= (CDBInit*)THREAD_FUNC;
	ASSERT(pThis);

	ULONG i = 0;
	ULONG* rgIndexes = NULL;
	ULONG iRand = 0;
	ULONG cUnique = 0;
	ULONG	cMaxSize	= s_CUDLCache.GetCurrentSize();

	//Other variations do somewhat "ordered" pegging, which is great, but not 
	//a good simulation of what might be done on the middle tier.  To simulate this
	//we will create a random UDL chooser, and will continue the process until all
	//UDLs files have been chossen randomly.  
	
	//NOTE: This actualy doesn't take that long, a system puedso generator seems
	//to only have arround < 10% duplication before all numbers are tried, so it will
	//take around N (max cache size) * 10% = N*(N*.1) = which is around 1/10 of the 
	//time it would take a normal N^2 algroythm (which we see all over the place...)

	//Initialize an array to store indicators or tried numbers...
	SAFE_ALLOC(rgIndexes, ULONG, cMaxSize);
	memset(rgIndexes, 0, cMaxSize * sizeof(ULONG));

	ThreadSwitch(); //Let the other thread(s) catch up

	//Load until we have loaded all UDLs files in the cache...
	while(cUnique < cMaxSize)
	{
		//Retrieve randomly (until all items have been retrieved)
		iRand = rand() % cMaxSize;
		// ASSERT(iRand >=0 && iRand < cMaxSize); - always false
		
		//Store that we tried this one..
		if(!rgIndexes[iRand])
			cUnique++;
		rgIndexes[iRand]++;
		
		//Load a random UDL file from the cache...
		CUDLInfo* pCUDLInfo = s_CUDLCache[iRand];
		TESTC(pThis->VerifyLoadString(pCUDLInfo->GetFileName(), pCUDLInfo->GetInitString()));
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	SAFE_FREE(rgIndexes);
	TRETURN
}

		
////////////////////////////////////////////////////////////////////////////
//  CDBInit::WriteString
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::WriteString(LPCWSTR pwszFileName, LPCWSTR pwszInitString, DWORD dwCreation)
{
	TBEGIN
	HRESULT hr = S_OK;
	WCHAR* pwszGeneratedName = NULL;
	WCHAR* pwszGeneratedInitString = NULL;
	CUDLInfo* pCUDLInfo = NULL;

	//Obtain the cache size before calling write
	ULONG cCacheSize = GetDevUDLCacheSize();	

	//Default FileName, generate the name...
	if(pwszFileName == GENERATE_NAME)
		pwszFileName = pwszGeneratedName = MakeUDLFileName();

	//Default InitString
	if(pwszInitString == NULL)
	{
		TESTC_(hr = GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszGeneratedInitString),S_OK);
		pwszInitString = pwszGeneratedInitString;
	}

	//Generated InitString
	if(pwszInitString == GENERATE_NAME)
		pwszInitString = pwszGeneratedInitString = MakeInitString();

	//Was this UDL file already in the cache
	pCUDLInfo = s_CUDLCache.GetUDLInfo(pwszFileName);
	if(pCUDLInfo)
		TESTC(VerifyDevUDLCache(S_OK/*cached*/, pwszFileName, pCUDLInfo));
	
	//Now call WriteStringToStorage
	hr = pDataInit()->WriteStringToStorage(pwszFileName, pwszInitString, dwCreation);

	if(SUCCEEDED(hr))
	{
		//If found in the cache, make sure dev has removed it from the cache...
		//ie: the dev cache size should be 1 smaller...
		if(pCUDLInfo)
		{
			cCacheSize--;	
			s_CUDLCache.RemoveUDLInfo(pwszFileName);
			SAFE_DELETE(pCUDLInfo);	
		}

		//Make sure that the DSL has removed this item from thier cache...
		TESTC(VerifyDevUDLCache(S_FALSE/*not cached*/, pwszFileName, pCUDLInfo));
		TESTC(!m_fTestingHooks || cCacheSize == GetDevUDLCacheSize());	
	}

CLEANUP:
	SAFE_FREE(pwszGeneratedName);
	SAFE_FREE(pwszGeneratedInitString);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyWriteString
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyWriteString(LPCWSTR pwszFileName, LPCWSTR pwszInitString, DWORD dwCreation)
{
	TBEGIN
	WCHAR* pwszGeneratedName = NULL;
	WCHAR* pwszGeneratedInitString = NULL;

	//Default FileName, generate the name...
	if(pwszFileName == GENERATE_NAME)
		pwszFileName = pwszGeneratedName = MakeUDLFileName();

	//Default InitString
	if(pwszInitString == NULL)
	{
		TESTC_(GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszGeneratedInitString),S_OK);
		pwszInitString = pwszGeneratedInitString;
	}

	//Generated InitString
	if(pwszInitString == GENERATE_NAME)
		pwszInitString = pwszGeneratedInitString = MakeInitString();

	//Make sure this file doesn't already exist...
	RemoveFile(pwszFileName);

	//Create this new File with the specified InitString
	TESTC_(WriteString(pwszFileName, pwszInitString, dwCreation),S_OK);

	//Load and Verify
	TESTC(VerifyLoadString(pwszFileName, pwszInitString));

CLEANUP:
	RemoveFile(pwszFileName);
	SAFE_FREE(pwszGeneratedName);
	SAFE_FREE(pwszGeneratedInitString);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_WriteString
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_WriteString(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis		= (CDBInit*)THREAD_FUNC;
	CUDLInfo* pCUDLInfo	= (CUDLInfo*)THREAD_ARG1;
	HRESULT hrExpected	= (HRESULT)THREAD_ARG2;
	ASSERT(pThis);
	HRESULT hr = S_OK;

	//NOTE: pCUDLInfo can be null if wanting to use a unqiuq file name
	if(!pCUDLInfo)
		pCUDLInfo = s_CUDLCache.ChooseRandomUDL();

	ThreadSwitch(); //Let the other thread(s) catch up

	//WriteString (helper)
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TEST2C_(hr = pThis->pDataInit()->WriteStringToStorage(pCUDLInfo->GetFileName(), pCUDLInfo->GetInitString(), CREATE_ALWAYS),S_OK, hrExpected);

		//Make sure that the DSL has removed this item from thier cache...
		if (SUCCEEDED(hr))
			TESTC(pThis->VerifyDevUDLCache(S_FALSE/*not cached*/, pCUDLInfo->GetFileName(), pCUDLInfo));
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::PromptDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::PromptDataSource(IUnknown* pIUnkOuter, HWND hWnd, DBPROMPTOPTIONS dwPromptOpts, ULONG cFilters, DBSOURCETYPE* rgFilters, WCHAR* pwszFilter, REFIID riid, IUnknown** ppDataSource, BOOL fInsideThreads)
{
	TBEGIN
	IUnknown* pDataSource = ppDataSource ? *ppDataSource : NULL;
	HRESULT hr = S_OK;

	//Input DataSource
	IUnknown* pIUnkInput = pDataSource;				//RefCount = 1
	SAFE_ADDREF(pIUnkInput);						//RefCount = 2
	ULONG ulOrgRefCount = GetRefCount(pIUnkInput);	

	//IDBPromptInitialize::PromptDataSource
	hr = pDBPromptInit()->PromptDataSource(pIUnkOuter, hWnd, dwPromptOpts, cFilters, rgFilters, pwszFilter, riid, &pDataSource);

	//Verify Results
	if(SUCCEEDED(hr))
	{
		if(pIUnkInput)
		{
			TESTC(pDataSource != NULL);

			//The output pointer may not be the same as the input, if they were
			//not the same REFIID, since you can ask for a different one on output.
			//We should make sure that the IUnknowns of the objects are the same...
			TESTC(VerifyEqualInterface(pIUnkInput, pDataSource));

			//DSL is supposed to QI the previous pointer,
			//And call release on the input, so the net result is no change...
			if(!fInsideThreads)
				TESTC(GetRefCount(pDataSource) == ulOrgRefCount);
		}
		else
		{
			TESTC(pDataSource != NULL);
			TESTC(pIUnkInput == NULL);
		}
	}
	else
	{
		TESTC(pDataSource == pIUnkInput);
		//DSL is supposed to QI the previous pointer,
		//And call release on the input, so the net result is no change...
		if(!fInsideThreads)
			TESTC(GetRefCount(pDataSource) == ulOrgRefCount);
	}

CLEANUP:
	SAFE_RELEASE(pIUnkInput);
	if(ppDataSource)
		*ppDataSource = pDataSource;
	else
		SAFE_RELEASE_DSO(pDataSource);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_PromptDataSource
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_PromptDataSource(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis = (CDBInit*)THREAD_FUNC;
	DBPROMPTOPTIONS dwPromptOptions = (DBPROMPTOPTIONS)THREAD_ARG1;
	IID* pIID = (IID*)THREAD_ARG2;
	IUnknown** ppDataSource = (IUnknown**)THREAD_ARG3;
	HRESULT hrExpected = (HRESULT)THREAD_ARG4;
	ASSERT(pThis && pIID && ppDataSource);

	ThreadSwitch(); //Let the other thread(s) catch up

	//PromptDataSource (helper)
	//TRUE = fInsideThreads
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(pThis->PromptDataSource(NULL, NULL, dwPromptOptions, 0, NULL, NULL, *pIID, ppDataSource, TRUE), hrExpected);

		//Release all but the last iteration, which gets returned to the user...
		if(i<MAX_ITERATIONS-1)
		{
			TESTC_(ReleaseDataSource(ppDataSource),S_OK);
		}
	}
	
	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}



////////////////////////////////////////////////////////////////////////////
//  CDBInit::PromptFileName
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::PromptFileName(HWND hWnd, DBPROMPTOPTIONS dwPromptOpts, WCHAR* pwszInitDir, WCHAR* pwszInitFile, WCHAR** ppwszSelectedFile)
{
	TBEGIN
	WCHAR* pwszSelectedFile = NULL;
	HRESULT hr = S_OK;

	//IDBPromptInitialize::PromptFileName
	QTESTC_(hr = pDBPromptInit()->PromptFileName(hWnd, dwPromptOpts, pwszInitDir, pwszInitFile, &pwszSelectedFile),S_OK);

	
CLEANUP:
	if(ppwszSelectedFile)
		*ppwszSelectedFile = pwszSelectedFile;
	else
		SAFE_FREE(pwszSelectedFile);
	return hr;
}



////////////////////////////////////////////////////////////////////////////
//  CDBInit::Thread_PromptFileName
//
////////////////////////////////////////////////////////////////////////////
ULONG CDBInit::Thread_PromptFileName(void* pv)
{
	THREAD_BEGIN

	//Thread Stack Variables
	CDBInit* pThis = (CDBInit*)THREAD_FUNC;
	DBPROMPTOPTIONS dwPromptOptions = (DBPROMPTOPTIONS)THREAD_ARG1;
	WCHAR** ppwszSelectedFile = (WCHAR**)THREAD_ARG2;
	HRESULT hrExpected = (HRESULT)THREAD_ARG3;
	ASSERT(pThis && ppwszSelectedFile);

	ThreadSwitch(); //Let the other thread(s) catch up

	//PromptFileName (helper)
	for(ULONG i=0; i<MAX_ITERATIONS; i++)
	{
		TESTC_(pThis->PromptFileName(NULL, dwPromptOptions, NULL, NULL, ppwszSelectedFile),hrExpected);

		//Release all but the last iteration, which gets returned to the user...
		if(i<MAX_ITERATIONS-1)
			SAFE_FREE(*ppwszSelectedFile);
	}

	ThreadSwitch(); //Let the other thread(s) catch up

CLEANUP:
	THREAD_RETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDiffProvider
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDiffProvider(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD dwOptions, CLSID* pCLSID)
{
	TBEGIN
	HRESULT hr = S_OK;
	CLSID clsidProv = CLSID_MSDASQL;

	ULONG i,cEnumInfo = 0;
	ENUMINFO* rgEnumInfo = NULL;
	
	//We will assume MSDASQL is a different provider.
	//If we happen to be running against MSDASQL, then we will need to pick another 
	//provider from the Enumerator, if one exists...
	if(PROVIDER_CLSID == clsidProv)
	{
		//Obtain Root Enumerator info...
		TESTC_(GetEnumInfo(NULL, &cEnumInfo, &rgEnumInfo),S_OK);
		
		//Loop through all providers
		for(i=0; i<cEnumInfo; i++)
		{
		
			//Ignore other Enumerators...
			if(rgEnumInfo[i].wType != DBSOURCETYPE_DATASOURCE)
				continue;
			
			//Obtain the CLSID
			TESTC_(CLSIDFromString(rgEnumInfo[i].wszParseName, &clsidProv),S_OK);
			if(clsidProv != PROVIDER_CLSID)
				break;
		}
	}

	//Unable to find a different provider
	if(PROVIDER_CLSID == clsidProv)
		return E_FAIL;

	hr = CreateDataSource(clsidProv, pIUnkOuter, riid, ppDataSource, dwOptions);
	TESTC(VerifyOutputInterface(hr, riid, ppDataSource));
	
CLEANUP:
	if(pCLSID)
		*pCLSID = clsidProv;
	SAFE_FREE(rgEnumInfo);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDataSource(IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD dwOptions)
{
	//Delegation
	return CreateDataSource(PROVIDER_CLSID, pIUnkOuter, riid, ppDataSource, dwOptions);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateDataSource
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::CreateDataSource(CLSID clsidProv, IUnknown* pIUnkOuter, REFIID riid, IUnknown** ppDataSource, DWORD dwOptions)
{
	TBEGIN
	HRESULT hr = S_OK;
	IUnknown* pDataSource = NULL;

	//Create a new DataSource
	QTESTC_(hr = CreateDBInstance(clsidProv, pIUnkOuter, riid, &pDataSource),S_OK);
	
	//SetProperties (if requested)
	QTESTC_(hr = InitializeDataSource(pDataSource, dwOptions),S_OK);

CLEANUP:
	if(SUCCEEDED(hr) && ppDataSource)
		*ppDataSource = pDataSource;
	else
		SAFE_RELEASE_DSO(pDataSource);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyPersistedInitString
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyPersistedInitString(LPCWSTR pwszInitString, LPCWSTR pwszInitStringReturned)
{
	TBEGIN
	
	//Verify results
	TESTC(pwszInitString && pwszInitStringReturned);

	//Make sure the strings are equal...
	//Spec doesn't allow Leading and Trailing Whitespace to be lost...
	if(wcscmp(pwszInitString, pwszInitStringReturned)!=0)
	{
		TERROR(L"Load/Write don't match!  Write (" << wcslen(pwszInitString) << ") \"" << pwszInitString 
			<< "\" Load (" << wcslen(pwszInitStringReturned) << ") \"" << pwszInitStringReturned << "\"");
	}

CLEANUP:
	TRETURN
}
	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyInitString
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyInitString(IUnknown* pDataSource, WCHAR* pwszInitString, BOOL fReturnedString, BOOL fInsideThreads)
{
	TBEGIN
	ULONG i,cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszKeyword = NULL;
	WCHAR* pwszValue = NULL;
	
	//No we need to make sure that all properties previsouly set
	//Appear in the InitString.  We will assume GetProperties works correctly
	//and has been well tested in the Properties test...

	//Obtain all the Properites
	TESTC_(GetProperties(pDataSource, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//Loop through specified Properties and verify exist in the String...
	for(i=0; i<cPropSets; i++)
	{
		DBPROPSET* pPropSet = &rgPropSets[i];
		for(ULONG j=0; j<pPropSet->cProperties; j++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[j];
			
			//Get the Property Keyword...
			DBPROPINFO* pPropInfo = GetPropInfo(pProp->dwPropertyID, pPropSet->guidPropertySet, pDataSource);
			if(!pPropInfo || !pPropInfo->dwFlags)
			{
				odtLog << "Missing Property Information for PropertyID = " << pProp->dwPropertyID << "\n";
				continue;
			}
			
			//Verify Description
			if(!pPropInfo->pwszDescription)
			{
				//Not much testing in the InitString we can do without a description! A unqiue 
				//description is required by the OLE DB Spec for all properties even provider specific ones
				TERROR("Missing Property Description for PropertyID = " << pProp->dwPropertyID);
				continue;
			}
			
			//There are 2 senarios you would call this method.
			//1 - fReturnedString = TRUE, where you have previously called GetInitString
			//					 and want to verify all properties set on the DataSource
			//					 are indeed returned in the string
			//2 - fReturnedString = FALSE, where you have previously called GetDataSource
			//					 passing it a String, and want to make sure the
			//					 returned DataSource does indeed contain the property
			//					 But it could also contain other properties besides 
			//					 what wass passed in the InitString
						
			//Now Obtain the Value from the InitString 
			BOOL bFound = GetModInfo()->GetStringKeywordValue(pwszInitString, pPropInfo->pwszDescription, &pwszValue);
			if(bFound || (fReturnedString && V_VT(&pProp->vValue)!=VT_EMPTY))
			{
				//Some properties are spec'd to never be returned in the InitString
				//For those we need to verify thery are not returned, and all others
				//make sure they do exist...
				if(pPropSet->guidPropertySet == DBPROPSET_DBINIT && 
					(	
						pProp->dwPropertyID == DBPROP_INIT_ASYNCH || 
						pProp->dwPropertyID == DBPROP_INIT_PROMPT || 
						pProp->dwPropertyID == DBPROP_INIT_HWND	||
						pProp->dwPropertyID == DBPROP_INIT_OLEDBSERVICES ||
						pProp->dwPropertyID == DBPROP_INIT_TIMEOUT ||
						pProp->dwPropertyID == DBPROP_INIT_GENERALTIMEOUT ||
						pProp->dwPropertyID == DBPROP_INIT_LCID
					))
				{
					if(bFound && fReturnedString)
						TERROR("Property \"" << pPropInfo->pwszDescription << "\" - should not be returned in the InitString!");
				}
				else if(pProp->dwPropertyID == DBPROP_AUTH_PASSWORD && pPropSet->guidPropertySet == DBPROPSET_DBINIT)
				{
					//Special Password Handling...
					//Password, there are a number of different requirements...
					VerifyPassword(pDataSource, TRUE, pwszInitString, fReturnedString);
				}
				else if(!(pPropInfo->dwFlags & DBPROPFLAGS_WRITE))
				{
					//ReadOnly properties should not be returned in the InitString
					if(bFound && fReturnedString)
						TERROR("ReadOnly Property \"" << pPropInfo->pwszDescription << "\" - should not be returned in the InitString!");
				}
				else
				{
					//Verify Property Value is equal to InitString Value
					//This is case-sensitive since all values should be preserved 
					//exactly as the user has entered them...
					if(!bFound)
					{
						if(!IsDefaultValue(pDataSource, pProp->dwPropertyID, pPropSet->guidPropertySet, VT_VARIANT, &pProp->vValue))
							TERROR("Unable to find property \"" << pPropInfo->pwszDescription << "\" in the InitString and this is NOT the default value...");
						
						//According to the spec all non-DPO objects have
						//have properties in the InitString, including defaults...
						if(!IsDPO(pDataSource))
							TERROR("Unable to find property \"" << pPropInfo->pwszDescription << "\" in the InitString and this is a non-DPO object...");
					}
					else if(!pwszValue && !fReturnedString)
					{
						//Property was Specified in the InitString as keyword=;
						//Property returned in GetProperties should be the default
						if(!IsDefaultValue(pDataSource, pProp->dwPropertyID, pPropSet->guidPropertySet, VT_VARIANT, &pProp->vValue) && !(pPropInfo->vtType==VT_BSTR && V_BSTR(&pProp->vValue) && (V_BSTR(&pProp->vValue))[0]==L'\0'))
							TERROR("Property returned in the InitString \"" << pPropInfo->pwszDescription << "\" is NOT the default value...");
					}
					else
					{
						LONG lValue = 0;

						//Otherwise Compare the string name to the property value...
						switch(V_VT(&pProp->vValue))
						{
							case DBTYPE_I4:
								if(!GetModInfo()->GetFriendlyNameValue(pProp->dwPropertyID, pwszValue, &lValue) || lValue != V_I4(&pProp->vValue))
									TERROR("Property Values for \"" << pPropInfo->pwszDescription << "\" don't match!  InitString: " << lValue << " GetProperties: " << V_I4(&pProp->vValue));
								break;

							case DBTYPE_I2:
								if(!GetModInfo()->GetFriendlyNameValue(pProp->dwPropertyID, pwszValue, &lValue) || lValue != V_I2(&pProp->vValue))
									TERROR("Property Values for \"" << pPropInfo->pwszDescription << "\" don't match!  InitString: " << lValue << " GetProperties: " << V_I2(&pProp->vValue));
								break;

							case DBTYPE_BOOL:
								if(!GetModInfo()->GetFriendlyNameValue(pProp->dwPropertyID, pwszValue, &lValue) || lValue != V_BOOL(&pProp->vValue))
									TERROR("Property Values for \"" << pPropInfo->pwszDescription << "\" don't match!  InitString: " << lValue << " GetProperties: " << V_BOOL(&pProp->vValue));
								break;

							case DBTYPE_BSTR:
							{	
								WCHAR* pwszPropValue = V_BSTR(&pProp->vValue);
								if(pwszPropValue == NULL || wcscmp(pwszPropValue, pwszValue)!=0)
								{
									//NOTE:  Our test (privlib) parser is fairly
									//good and has been adjusted quite well to deal very similar to the
									//GetDataSource parser.  But there are issues with embedding quotes
									//that it doesn't handle that well, without spending the time for a full blown
									//tokenizing parser.  It will not deal with embedding quotes too well since 
									//since it will tag on the first (";) and not know how to find the
									//correct ending one, in these cases, we might need manually verifiecation...
									if(pwszPropValue && wcsstr(pwszPropValue, L"\";"))
									{
										if(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS)
										{
											TWARNING("Our test parser is not a full blown parser and cannot handle all issues with embedding quotes.");
											TWARNING("Please verify this is correct - InitString: \"" << pwszInitString << "\" GetProperties: \"" << pwszPropValue << "\"");
										}
									}
									else
									{
										//Special Case:
										if(pProp->dwPropertyID == DBPROP_INIT_PROVIDERSTRING && pPropSet->guidPropertySet == DBPROPSET_DBINIT
											&& IsInitialized(pDataSource))
										{
											//Its hard to verify properties after Initialization with ProviderString.
											//Since the provider may also return more information in this property
											//than was passed in.  User may pass in DSN=dsn; UID=uid and on return
											//contain those and many more provider specific values in this string...
										//	TWARNING("Unable to verify provider string after initialization - InitString: \"" << pwszValue << "\" GetProperties: \"" << pwszPropValue << "\"");
										}
										else
										{
											TERROR("Property Values for \"" << pPropInfo->pwszDescription << "\" don't match!  InitString: \"" << pwszValue << "\" GetProperties: \"" << pwszPropValue << "\"");
										}
									}
								}
								break;
							}

							case DBTYPE_EMPTY:
								//String should only return non-default values
								TERROR("Property \"" << pPropInfo->pwszDescription << "\" returned in the InitString as Empty =;?");
								break;

							default:
							{	
								//If the property is not one of the above values, then it is not an
								//OLE DB Defined property, so it doesn't have a "textual" friendly
								//name, and should compare exactly the to the type.
								VARIANT vVariant;
								VariantInit(&vVariant);

								//Convert the string value to a numeric (so we don't have issues
								//with leading/trailing digits.
								if(FAILED(StringToVariant(pwszValue, V_VT(&pProp->vValue), &vVariant)))
									TERROR("Unable to convert Property value to a Variant for \"" << pPropInfo->pwszDescription << "\" Value=" << pwszValue);

								//Now compare literally...
								//Verify the Same Value
								if(!CompareVariant(&pProp->vValue, &vVariant))
								{
									WCHAR wszBuffer[MAX_QUERY_LEN];
									VariantToString(&pProp->vValue, wszBuffer, MAX_QUERY_LEN, FALSE);

									//Display Differences...
									TERROR(L"Property values for \"" << pPropInfo->pwszDescription << "\" don't match:  Expected: \"" << wszBuffer << "\" Actual: \"" << pwszValue << "\"");
								}

								VariantClear(&vVariant);
								break;
							}
						};
					}
				}
			}

			//Cleanup for next property...
			SAFE_FREE(pPropInfo->pwszDescription);
			FreePropInfos(1, pPropInfo);
			SAFE_FREE(pwszValue);
		}
	}

	//Additional Testing
	//Since almost all methods that Create an InitString call this method to
	//Verify its correct, we might as well obtain additional testing, by verifing
	//That this particular string can be Saved and Loaded...
	if(pwszInitString && !fInsideThreads)
		TESTC(VerifyWriteString(GENERATE_NAME, pwszInitString));

CLEANUP:
	SAFE_FREE(pwszKeyword);
	SAFE_FREE(pwszValue);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyInitString
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyInitString(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue)
{
	TBEGIN
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszInitString = NULL;
	IDBProperties* pIDBProperties = NULL;
	IUnknown* pDataSource2 = NULL;
	HRESULT hr = S_OK;

	//Create a new DSO
	TESTC_(CreateDBInstance(NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties),S_OK);

	//DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO = TRUE
	::SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);
	hr = pIDBProperties->SetProperties(cPropSets, rgPropSets);
	::FreeProperties(&cPropSets, &rgPropSets);

	//SetProperties
	//Its very possible this provider does not support this property...
	QTESTC(::SetProperty(dwPropertyID, guidPropertySet, &cPropSets, &rgPropSets, (void*)pValue, wType));
	hr = pIDBProperties->SetProperties(cPropSets, rgPropSets);
	TEST2C_(hr, S_OK, DB_E_ERRORSOCCURRED);

	//Make sure this is a unsupported property...
	//Or its a badvalue for this property...
	if(FAILED(hr))
	{
		DBPROP* pProp = NULL;
		FindProperty(dwPropertyID, guidPropertySet, cPropSets, rgPropSets, &pProp);
		if(SettableProperty(dwPropertyID, guidPropertySet, pIDBProperties))
		{
			TESTC(pProp->dwStatus == DBPROPSTATUS_BADVALUE);
		}
		else
		{
			TESTC(pProp->dwStatus == DBPROPSTATUS_NOTSUPPORTED);
		}
	}

	//GetInitString from the passed in DataSource
	QTESTC_(GetInitString(pIDBProperties, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//No we need to make sure that all properties previsouly set
	//Appear in the InitString.  We will assume GetProperties works correctly
	//and has been well tested in the Properties test...
	QTESTC(VerifyInitString(pIDBProperties, pwszInitString));

	//Since the DataSource is used for this function only and not returned to the user
	//we might as well do additional testing, as try to get this object into the pool
	//for greater testing of the pooling...
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pDataSource2),S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBProperties);
	SAFE_RELEASE_DSO(pDataSource2);
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszInitString);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyCompleteSequence
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyCompleteSequence(DWORD dwOptions, BOOL fIncludePassword)
{
	TBEGIN
	IDBProperties* pIDBProperties = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	BOOL bPersistInfo = FALSE;

	//Adjust if no properties have been set...
	if(dwOptions & CREATEDSO_NONE)
		fIncludePassword = FALSE;

	//Create a new DataSource
	QTESTC_(CreateDBInstance(PROVIDER_CLSID, NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties),S_OK);
	
	// Set properties (if requested)
	if (dwOptions & CREATEDSO_SETPROPERTIES)
		QTESTC_(InitializeDataSource(pIDBProperties, CREATEDSO_SETPROPERTIES), S_OK);

	//DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO = TRUE
	::SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);
	SetProperties(pIDBProperties, cPropSets, rgPropSets);
	bPersistInfo = GetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, pIDBProperties);

	//SetProperties (if requested)
	//QTESTC_(InitializeDataSource(pIDBProperties, dwOptions),S_OK);

	// Initialize provider (if requested)
	QTESTC_(InitializeDataSource(pIDBProperties, dwOptions & ~CREATEDSO_SETPROPERTIES), S_OK);


	//GetInitString
	TESTC_(GetInitString(pIDBProperties, fIncludePassword, &pwszInitString),S_OK);

	//Verify Load/Write String
	TESTC(VerifyWriteString(GENERATE_NAME, pwszInitString));

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);

	//Initialize - with properties set...
	TESTC(VerifyInitialize(pIDBInitialize, dwOptions & CREATEDSO_NONE ? FALSE : (fIncludePassword && bPersistInfo))); 

	//Since the DataSource is used for this function only and not returned to the user
	//we might as well do additional testing, as try to get this object into the pool
	TESTC_(ReleaseDataSource((IUnknown**)&pIDBInitialize), S_OK);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE_DSO(pIDBProperties);
	SAFE_RELEASE_(pIDBInitialize);  //Verify 0 RefCount...
	SAFE_FREE(pwszInitString);
	TRETURN
}

		
////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetSomePropValue
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::GetSomePropValue(VARIANT* pVariant, DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, BOOL fValid)
{
	ASSERT(pVariant);
	BOOL bFound = FALSE;
	VariantInit(pVariant);
	V_VT(pVariant) = wType;
	
	//Variable Loop Counter...
	static ULONG iTimesCalled = 0;
	iTimesCalled++;

	//DBPROPSET_DBINIT
	if(guidPropertySet == DBPROPSET_DBINIT)
	{
		switch(dwPropertyID)
		{
			case DBPROP_INIT_ASYNCH:		
				V_I4(pVariant) = fValid ? DBPROPVAL_ASYNCH_INITIALIZE : 0;
				bFound = TRUE;
				break;

			case DBPROP_INIT_PROMPT:
				V_I4(pVariant) = fValid ? dwPropertyID % 4 + 1 : 0;
				bFound = TRUE;
				break;
		};
	}
	else if(guidPropertySet == DBPROPSET_SESSION)
	{
		switch(dwPropertyID)
		{
			case DBPROP_SESS_AUTOCOMMITISOLEVELS:
				//DBPROPVAL_TI_CHAOS							 0x00000010L
				//DBPROPVAL_TI_READUNCOMMITTED					 0x00000100L
				//DBPROPVAL_TI_BROWSE							 0x00000100L
				//DBPROPVAL_TI_CURSORSTABILITY					 0x00001000L
				//DBPROPVAL_TI_READCOMMITTED					 0x00001000L
				//DBPROPVAL_TI_REPEATABLEREAD					 0x00010000L
				//DBPROPVAL_TI_SERIALIZABLE						 0x00100000L
				//DBPROPVAL_TI_ISOLATED							 0x00100000L
				V_I4(pVariant) = fValid ? (DBPROPVAL_TI_CHAOS << iTimesCalled % 5) : 0;
				bFound = TRUE;
				break;
		};
	}
	
	//Not Found cases...
	if(!bFound)
	{
		//All we can really do is to apply a "default" value within the range
		//of the indicated type...
		switch(wType)
		{
			case VT_BOOL:
				V_BOOL(pVariant) = (dwPropertyID % 2) ? VARIANT_TRUE : VARIANT_FALSE;
				break;
			
			case VT_BSTR:
				V_BSTR(pVariant) = SysAllocString(L"SomeRealyCoolPropertyValue");
				break;

			case VT_I2:
				V_I2(pVariant) = (INT)(fValid ? dwPropertyID % 3 + 1 : dwPropertyID % INT_MAX);
				break;
									
			case VT_I4:
				V_I4(pVariant) = fValid ? dwPropertyID % 3 + 1 : dwPropertyID;
				break;

			default:
				//Worse case we can always set VT_EMPTY
				//Which means restore the property to its default value...
				V_VT(pVariant) = VT_EMPTY;
				break;
		};
	}

	return bFound;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyGetDataSource
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyGetDataSource(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType)
{
	TBEGIN
	WCHAR* pwszPropDesc = NULL;
	WCHAR* pwszProvider = CreateString(m_pwszProperties);
	
	//Try the static array in the Privlib, this way we can get Descriptions even
	//For properties not supported by the provider, (in case were testing this).
	//All other descriptions, will have to be obtained from the provider...
	if(GetStaticPropDesc(dwPropertyID, guidPropertySet))
	{
		pwszPropDesc = wcsDuplicate(GetStaticPropDesc(dwPropertyID, guidPropertySet));
	}
	else
	{
		if(!(pwszPropDesc = GetPropDesc(dwPropertyID, guidPropertySet, pIDBInit())) )
		{
			TERROR("Specified PropertyID \"" << dwPropertyID << "\" is not returned in GetPropertyInfo!");
			return FALSE;
		}
	}
	
	BOOL fIsSettable = SettableProperty(dwPropertyID, guidPropertySet, pDBInitSC());


	//Individual Testing.
	//Senarios thate are unique depending upon the type...
	switch(wType)
	{
		case DBTYPE_BOOL:
			//True
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_TRUE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"True"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)VARIANT_TRUE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"TRUE"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_TRUE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"-1"));
									  
			//False
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"False"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"FALSE"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0"));
			
			//Specified more than once
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s =%s; %s; %s =%s", pwszPropDesc, L"true", pwszProvider, pwszPropDesc, L"false"));

			//Keyword CaseInsensitive
			ConvertString(pwszPropDesc, TRUE); //Convert to Upper
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"False"));
			ConvertString(pwszPropDesc, FALSE); //Convert to Lower																
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"False"));

			//Invalid Senarios
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s = \"\"\"\"", pwszProvider, pwszPropDesc),				!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"String"),			!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"VARIANT_FALSE"),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"VARIANT_TRUE"),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%u", pwszProvider, pwszPropDesc, ULONG_MAX),			!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =%d", pwszProvider, pwszPropDesc, LONG_MIN),			!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)VARIANT_FALSE,	L"%s; %s =1.1", pwszProvider, pwszPropDesc),					!fIsSettable);
			break;

		case DBTYPE_BSTR:
			//Strings - Boundary
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"",								L"%s; %s = \"\"", pwszProvider, pwszPropDesc));
			
			//Strings
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"My Integrated Server",			L"%s; %s =%s", pwszProvider, pwszPropDesc, L"My Integrated Server"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"...",								L"%s; %s =%s", pwszProvider, pwszPropDesc, L"..."));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"Data Source = word",				L"%s; %s =%s", pwszProvider, pwszPropDesc, L"Data Source = word"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L" ",								L"%s; %s =%s", pwszProvider, pwszPropDesc, L"\" \""));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L" ",								L"%s; %s =%s", pwszProvider, pwszPropDesc, L"\' \'"));

			//Numerics - Should just treat as strings
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"-11",								L"%s; %s =%s", pwszProvider, pwszPropDesc, L"-11"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, L"\\\\server\\dir\\file.ext",		L"%s; %s =%s", pwszProvider, pwszPropDesc, L"\\\\server\\dir\\file.ext"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"c:\\long dir\\longfilename.ext",	L"%s; %s =%s", pwszProvider, pwszPropDesc, L"c:\\long dir\\longfilename.ext"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"http:/iisserver/default.htm",		L"%s; %s =%s", pwszProvider, pwszPropDesc, L"http:/iisserver/default.htm"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, L"4294967295",						L"%s; %s =%u", pwszProvider, pwszPropDesc, ULONG_MAX));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, L"-2147483648",						L"%s; %s =%d", pwszProvider, pwszPropDesc, LONG_MIN));

			//Specified more than once
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, L"My Integrated Server",			L"%s =%s; %s; %s =%s", pwszPropDesc, L"Data Source = word", pwszProvider, pwszPropDesc, L"My Integrated Server"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, L"Data Source = word",				L"%s =%s; %s; %s =%s", pwszPropDesc, L"My Integrated Server", pwszProvider, pwszPropDesc, L"Data Source = word"));

			//Keyword CaseInsensitive
			ConvertString(pwszPropDesc, TRUE); //Convert to Upper
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"My Integrated Server",			L"%s; %s =%s", pwszProvider, pwszPropDesc, L"My Integrated Server"));
			ConvertString(pwszPropDesc, FALSE); //Convert to Lower
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	L"My Integrated Server",			L"%s; %s =%s", pwszProvider, pwszPropDesc, L"My Integrated Server"));
			break;

		case DBTYPE_I4:
			if (guidPropertySet == DBPROPSET_DBINIT && dwPropertyID == DBPROP_INIT_ASYNCH)
			{
				//Decimal
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"1"));
				TESTC(!VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));

				//Hex/Octal/Binary
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)0x1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x1"));
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)01,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"01"));
				TESTC(!VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)0x2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x2"));

				//Keyword CaseInsensitive
				ConvertString(pwszPropDesc, TRUE); //Convert to Upper
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"1"));
				ConvertString(pwszPropDesc, FALSE); //Convert to Lower														
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"1"));

				//Empty
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, DBTYPE_EMPTY, (void*)NULL,					L"%s; %s =;", pwszProvider, pwszPropDesc));

				//Invalid Senarios
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =3.4", pwszProvider, pwszPropDesc),				!fIsSettable);
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =\"\"", pwszProvider, pwszPropDesc),				!fIsSettable);
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"String"),		!fIsSettable);
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%u", pwszProvider, pwszPropDesc, ULONG_MAX),		!fIsSettable);
			}
			else
			{

				//Decimal
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"1"));
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));

				//Hex/Octal/Binary
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)0x1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x1"));
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)01,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"01"));
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)0x2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x2"));

				//Specified more than once
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s =%s; %s; %s =%s", pwszPropDesc, L"1", pwszProvider, pwszPropDesc, L"2"));

				//Keyword CaseInsensitive
				ConvertString(pwszPropDesc, TRUE); //Convert to Upper
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));
				ConvertString(pwszPropDesc, FALSE); //Convert to Lower														
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));

				//Empty
				TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, DBTYPE_EMPTY, (void*)NULL,					L"%s; %s =;", pwszProvider, pwszPropDesc));

				//Invalid Senarios
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =3.4", pwszProvider, pwszPropDesc),				!fIsSettable);
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =\"\"", pwszProvider, pwszPropDesc),				!fIsSettable);
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"String"),		!fIsSettable);
				TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%u", pwszProvider, pwszPropDesc, ULONG_MAX),		!fIsSettable);

			}
			break;

		case DBTYPE_I2:
			//Decimal (DBPROMPT)
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"1"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)3,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"3"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)4,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"4"));
									  
			//Hex/Octal/Binary
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x1"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)01,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"01"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x2"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)3,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x3"));

			//Specified more than once
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s =%s; %s; %s =%s", pwszPropDesc, L"1", pwszProvider, pwszPropDesc, L"2"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)3,							L"%s =%s; %s; %s =%s", pwszPropDesc, L"4", pwszProvider, pwszPropDesc, L"3"));

			//Keyword CaseInsensitive
			ConvertString(pwszPropDesc, TRUE); //Convert to Upper
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)3,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"3"));
			ConvertString(pwszPropDesc, FALSE); //Convert to Lower														
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));

			//Invalid Senarios
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"String"),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%u", pwszProvider, pwszPropDesc, ULONG_MAX),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%d", pwszProvider, pwszPropDesc, LONG_MIN),	!fIsSettable);

			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =1.9", pwszProvider, pwszPropDesc),			!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%d", pwszProvider, pwszPropDesc, 32767+1),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%d", pwszProvider, pwszPropDesc, -32768-1),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%d", pwszProvider, pwszPropDesc, 12345678),	!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%d", pwszProvider, pwszPropDesc, -12345678),	!fIsSettable);
			break;

		case DBTYPE_I8:
			//Decimal
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"1"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));

			//Hex/Octal/Binary
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)0x1,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x1"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)01,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"01"));
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType,	(void*)0x2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"0x2"));

			//Specified more than once
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s =%s; %s; %s =%s", pwszPropDesc, L"1", pwszProvider, pwszPropDesc, L"2"));

			//Keyword CaseInsensitive
			ConvertString(pwszPropDesc, TRUE); //Convert to Upper
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));
			ConvertString(pwszPropDesc, FALSE); //Convert to Lower														
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)2,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"2"));

			//Empty
			TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, DBTYPE_EMPTY, (void*)NULL,					L"%s; %s =;", pwszProvider, pwszPropDesc));

			//Invalid Senarios
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =3.4", pwszProvider, pwszPropDesc),				!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =\"\"", pwszProvider, pwszPropDesc),				!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%s", pwszProvider, pwszPropDesc, L"String"),		!fIsSettable);
			TESTC_(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, (void*)0,							L"%s; %s =%u", pwszProvider, pwszPropDesc, ULONG_MAX),		!fIsSettable);

		default:
			//TODO
			TOUTPUT(L"Need to add additional testing for this property type = " << GetDBTypeName(wType));
			break;
	};

	//Common Testing.
	//Senarios that are similar for all types...

	//VT_EMPTY
	TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, DBTYPE_EMPTY, NULL,	L"%s; %s =;", pwszProvider, pwszPropDesc));
	TESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, DBTYPE_EMPTY,	NULL,	L"%s; %s =", pwszProvider, pwszPropDesc));

CLEANUP:
	SAFE_FREE(pwszPropDesc);
	SAFE_FREE(pwszProvider);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyGetDataSource
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyGetDataSource(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue, WCHAR* pwszFmt, ...)
{
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;

	//Process all varible input args...
	va_list		marker;
	WCHAR		wszInitString[MAX_QUERY_LEN+100] = {0};

	// Use format and arguments as input
	//This version will not overwrite the stack, since it only copies
	//upto the max size of the array
	va_start(marker, pwszFmt);
	_vsnwprintf(wszInitString, MAX_QUERY_LEN+100, pwszFmt, marker);
	va_end(marker);

	//Make sure there is a NULL Terminator, vsnwprintf will not copy
	//the terminator if length==MAX_QUERY_LEN
	wszInitString[MAX_QUERY_LEN+100-1] = L'\0';

	//Delegate - Call without an existing DSO
	QTESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, pValue, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, wszInitString));
	SAFE_RELEASE_DSO(pIDBInitialize);
	
	//Delegate - Now call with an existing DSO
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
	QTESTC(VerifyGetDataSource(dwPropertyID, guidPropertySet, wType, pValue, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, wszInitString));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyGetDataSource
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyGetDataSource(DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue, REFIID riid, IUnknown** ppDataSource, WCHAR* pwszInitString)
{
	TBEGIN
	WCHAR*		pwszAddInitString = NULL;
	WCHAR*		pwszProviderString = NULL;
	IUnknown*	pDataSource = ppDataSource ? *ppDataSource : NULL;
	HRESULT hr = S_OK;

	//Test InitString
	hr = GetDataSource(NULL, pwszInitString, riid, &pDataSource);
	
	if(hr==S_OK)
	{
		//Verify the Returned DataSource has the correct property set
		QTESTC(VerifyDataSource(pDataSource, dwPropertyID, guidPropertySet, wType, pValue));
		QTESTC(VerifyPassword(pDataSource, TRUE, pwszInitString, FALSE));

		//Might as well do some bonus testing of an Init String with 
		//this particular property
		QTESTC(VerifyInitString(dwPropertyID, guidPropertySet, wType, pValue));

		//Now that we have all Properties set...
		//Not a "exact" comparision (FALSE), since we called GetDataSource
		//From the string, which could return additional properties
		TESTC(VerifyInitString(pDataSource, pwszInitString, FALSE));

		//Would be nice to test properties after Initialization as well
		//Create a String that will hopefully get use connected, this is done
		//by using the InitProps and taking on this additional property.
		//The reason we tack it on is so that it overrides any init ones...
		pwszAddInitString = CreateInitPropString(FALSE);

		//PROVIDERSTRING - is difficult since SC will combine all normal ExtendedProperties into
		//this value plus any additional tokens it doesn't recognize.  The problem in verifying
		//this is that if we are explicitly testing PROVIDERSTRING we expect it to be a particular
		//value, but if the init properties also contains a ProviderString it increases our expected
		//value and makes it difficult to determine the resulting ProviderString.  So if we are
		//explicitily testing PROVIDERSTRING and the InitProperties also use the provider string
		//don't add the Init Property string, we just may not be able to connect in this rare case...
		if(guidPropertySet == DBPROPSET_DBINIT && dwPropertyID == DBPROP_INIT_PROVIDERSTRING && GetModInfo()->GetStringKeywordValue(pwszAddInitString, GetStaticPropDesc(DBPROP_INIT_PROVIDERSTRING), &pwszProviderString))
			SAFE_FREE(pwszAddInitString);

		//GetDataSource with the InitString
		AppendString(&pwszAddInitString, pwszInitString);
		TESTC_(GetDataSource(NULL, pwszAddInitString, riid, (IUnknown**)&pDataSource),S_OK);

		//Initialize (if we should - no dialogs or stuff...)
		if(ShouldInitialize(pDataSource))
			hr = InitializeDataSource(pDataSource, CREATEDSO_INITIALIZE);
		
		//Verify Properties after Initialization
		if(hr == S_OK)
		{
			//Since we have successfully connected, so this information into our
			//ConnectMngr, so we have it for later use of successful Connections

			
			//Its hard to verify properties after Initialization with ProviderString.
			//Since the provider may also return more information in this property
			//than was passed in.  User may pass in DSN=dsn; UID=uid and on return
			//contain those and many more provider specific values in this string...
			if(guidPropertySet == DBPROPSET_DBINIT && dwPropertyID != DBPROP_INIT_PROVIDERSTRING)
			{
				//Since GetDataSource succeeded, and Initialize returned S_OK
				//Our initial property passed into this function sure has better be
				//set on the initialized DSO.  Verify this...
				QTESTC(VerifyDataSource(pDataSource, dwPropertyID, guidPropertySet, wType, pValue));
				QTESTC(VerifyPassword(pDataSource, TRUE, pwszInitString, FALSE));

				//Might as well do some bonus testing of an Init String with 
				//this particular property
				TESTC(VerifyInitString(dwPropertyID, guidPropertySet, wType, pValue));

				//Now that we have all Properties set...
				//Not a "exact" coparision (FALSE), since we called GetDataSource
				//From the string, which could return additional properties
				TESTC(VerifyInitString(pDataSource, pwszInitString, FALSE));
			}
		}
		else
		{
			//Since we are calling Initialize, and we are trying almost every
			//combination of properties with this method, most of the time
			//Initialize will fail to connect due to property validation at
			//Initialize time...

			//TODO We currently allow E_FAIL, since some providers return this
			//becuase there unable to determine which properties were in error
			//for now we will allow this, and the properties test should flag it as an error
			//We want to continue testing here, so we will ignore it for now...
			TEST4C_(hr, DB_E_ERRORSOCCURRED, DB_SEC_E_AUTH_FAILED, E_FAIL, DB_S_ASYNCHRONOUS);
		}

		if(!ppDataSource)
		{
			//Since the DataSource is used for this function only and not returned to the user
			//we might as well do additional testing, as try to get this object into the pool
			TESTC_(ReleaseDataSource(&pDataSource), S_OK);
		}
	}
	else if(hr==DB_E_ERRORSOCCURRED || hr==E_FAIL)
	{
		//TODO We currently allow E_FAIL, since some providers return this
		//becuase there unable to determine which properties were in error
		//for now we will allow this, and the properties test should flag it as an error
		//We want to continue testing here, so we will ignore it for now...

		//On error it should be equal to the input
		TESTC(pDataSource == (ppDataSource ? *ppDataSource : NULL));

		if(SettableProperty(dwPropertyID, guidPropertySet, pDBInitSC()))
		{
			//Normally this error should only be returned if the property was not supported
			//or not settable.  If it is a supported and settable property then the only other
			//explaination is that is an unsupported value like with I4 property bits and enums...
			switch(wType)
			{
				case DBTYPE_I4:
				case DBTYPE_I2:
					if(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS)
					{
						TWARNING("Deferred checking until Initialize time for possible unsupported property '" << GetStaticPropDesc(dwPropertyID, guidPropertySet) << "' = '" << (ULONG_PTR)pValue << "'");
					}
					break;
			};
		}
	}
	else
	{
		//The only other valid Error is BadInitString, since some variations
		//Might be expecting that, return FALSE to indicate this...
		TESTC_(hr, DB_E_BADINITSTRING);
		TESTC(pDataSource == NULL);
		QTESTC(FALSE);
	}
	
CLEANUP:
	if(ppDataSource)
		*ppDataSource = pDataSource;
	else
		SAFE_RELEASE_DSO(pDataSource);
	SAFE_FREE(pwszAddInitString);
	SAFE_FREE(pwszProviderString);
	TRETURN

}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::AppendToInitString
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::AppendToInitString(WCHAR** ppwszInitString, DBPROPID dwPropertyID, GUID guidPropertySet, VARIANT* pVariant)
{
	TBEGIN
	ASSERT(ppwszInitString);
	WCHAR wszBuffer[MAX_QUERY_LEN+100];
	wszBuffer[0] = L'\0';
	WCHAR* pwszNewString = NULL;
	HRESULT hr = S_OK;

	//Now get the Keyword and Value
	WCHAR* pwszKeyword	= GetPropDesc(dwPropertyID, guidPropertySet, pDBInitSC());
	TESTC_(hr = VariantToString(pVariant, wszBuffer, MAX_QUERY_LEN+100, FALSE),S_OK);

	//Don't neccessaryly need a value, "keyword=;", but we
	//do need a keyword.  This should map to a non-NULL property description
	TESTC(pwszKeyword != NULL);

	//Add this to the String
	pwszNewString = CreateString(L"%s%s =", *ppwszInitString ? L";" : L"", pwszKeyword);
	hr = AppendString(ppwszInitString, pwszNewString);
	SAFE_FREE(pwszNewString);

	//We might need to quote the items if there is an embedded semi
	if(wcschr(wszBuffer, L';'))
		pwszNewString = CreateString(L"\"%s\"", wszBuffer);
	else
		pwszNewString = CreateString(L"%s", wszBuffer);
	hr = AppendString(ppwszInitString, pwszNewString);

CLEANUP:
	SAFE_FREE(pwszKeyword);
	SAFE_FREE(pwszNewString);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreatePropString
//
////////////////////////////////////////////////////////////////////////////
WCHAR* CDBInit::CreatePropString(ULONG cPropSets, DBPROPSET* rgPropSets, BOOL fIncludeProvider)
{
	TBEGIN
	WCHAR* pwszInitString = NULL;

	if(fIncludeProvider)
	{
		//Place the Provider ProgID in the string.  Since when running on other
		//provider without the provider keyword, it will default to MSDASQL...
		pwszInitString = CreateString(L"Provider = %s;", GetProgID());
	}
	
	//Loop through specified Properties and Formulate the String
	for(ULONG i=0; i<cPropSets; i++)
	{
		DBPROPSET* pPropSet = &rgPropSets[i];
		for(ULONG j=0; j<pPropSet->cProperties; j++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[j];
			
			//Now Add this Property
			AppendToInitString(&pwszInitString, pProp->dwPropertyID, pPropSet->guidPropertySet, &pProp->vValue);
			TESTC(pwszInitString != NULL);
		}
	}

	//Add Trailing Semi...
	//If no properties, just use an empty string, since GetDataSource fails on NULL
	AppendString(&pwszInitString, cPropSets ? L";" : L"");

CLEANUP:
	return pwszInitString;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::CreateInitPropString
//
////////////////////////////////////////////////////////////////////////////
WCHAR* CDBInit::CreateInitPropString(BOOL fIncludeProvider)
{
	TBEGIN
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszInitString = NULL;

	//Get the Initialization Properties
	TESTC(GetInitProps(&cPropSets, &rgPropSets));
	
	//Delegate
	pwszInitString = CreatePropString(cPropSets, rgPropSets, fIncludeProvider);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	return pwszInitString;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::IsUninitialized
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::IsUninitialized(IUnknown* pDataSource)
{
	TBEGIN
	ASSERT(pDataSource);
	WCHAR* pwszProviderName = NULL;

	//Should not be able to call GetProperties on anything other
	//than DBPROPSET_DBINIT if truely Uninitialized...
	QTESTC(!GetProperty(DBPROP_PROVIDERNAME, DBPROPSET_DATASOURCEINFO, pDataSource, &pwszProviderName));

CLEANUP:
	SAFE_FREE(pwszProviderName);
	TRETURN
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::IsInitialized
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::IsInitialized(IUnknown* pDataSource, BOOL fQuick)
{
	TBEGIN
	ASSERT(pDataSource);
	IDBInitialize* pIDBInitialize = NULL;
	IDBCreateSession* pIDBCreateSession = NULL;
	IOpenRowset* pIOpenRowset = NULL;

	HRESULT hr = S_OK;
	BOOL bInitialized = FALSE;

	if(fQuick || !ShouldInitialize(pDataSource))
	{
		//Try to obtain IDBCreateSession
		TEST3C_(hr = QI(pDataSource, IID_IDBCreateSession, (void**)&pIDBCreateSession), S_OK, E_NOINTERFACE, E_UNEXPECTED);

		//Make sure QI returned a valid return code...

		//Additional
		//Some providers may actually return a IDBCreateSession interface, but it may
		//not actually be useable, so we need to go one step further...
		if(SUCCEEDED(hr))
		{
			//Basically anything other than E_UNEXPECTED/E_FAIL is operational...
			hr = pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset, (IUnknown**)&pIOpenRowset);
			if(SUCCEEDED(hr) || hr==DB_E_OBJECTCREATIONLIMITREACHED)
				bInitialized = TRUE;
		}
	}
	else
	{
		//Obtain IDBInitialize interface
		TESTC_(QI(pDataSource, IID_IDBInitialize, (void**)&pIDBInitialize),S_OK);
			
		//Verify Can't Initialize...
		hr = InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE);
		bInitialized = (hr==DB_E_ALREADYINITIALIZED);

		//Restore the state of the object in case of a bug in the provider...
		if(SUCCEEDED(hr))
			pIDBInitialize->Uninitialize();
	}

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);	//Can't Initialize
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIOpenRowset);
	return bInitialized;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyInitialize
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyInitialize(IUnknown* pDataSource, BOOL fRegPropsSet)
{
	TBEGIN
	ASSERT(pDataSource);
	WCHAR* pwszInitString = NULL;

	//Verify Can Initialize...
	HRESULT hr = InitializeDataSource(pDataSource, CREATEDSO_INITIALIZE);

	//Verify Results
	if(fRegPropsSet)
	{
		TESTC_(hr, S_OK);
	}
	else
	{
		//TODO Kagera returns E_FAIL on init if unable to connect
		//it should return DB_E_ERRORSOCCURRED, but thats another bug...
		TEST4C_(hr, S_OK, DB_E_ERRORSOCCURRED, DB_SEC_E_AUTH_FAILED, E_FAIL);
	}

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::IsDPO
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::IsDPO(IUnknown* pDataSource)
{
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	//QI for a valid DataSource interface
	//QIing for IConnectionPointContainer will case ServiceComponents to create a DCM
	//since its an interface they don't support and forces the provider to be created.
	//But this only causes a DCM if the provider natively supports ICPC.
//	HRESULT hrCPC = QI(pDataSource, IID_IConnectionPointContainer, (void**)&pIUnknown2);

	//Defaults are always returned for non-DPO objects
	//Any DataSource that was not created through IDataInitialize, IDBPromptInitialize
	//we have defaults returned since SC cannot easily determine which values
	//have been changed.  The easiest way to determine weither this is a DSO
	//or a DPO is that a DPO object will have specific interfaces...
	HRESULT hrDPO = QI(pDataSource, IID_IDPOInternal, (void**)&pIUnknown);

	SAFE_RELEASE(pIUnknown2);
	SAFE_RELEASE(pIUnknown);
	return SUCCEEDED(hrDPO);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::IsDCM
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::IsDCM(IUnknown* pDataSource)
{
	IUnknown* pIUnknown = NULL;
	IUnknown* pIUnknown2 = NULL;

	//QI for a valid DataSource interface
	//QIing for IConnectionPointContainer will case ServiceComponents to create a DCM
	//since its an interface they don't support and forces the provider to be created.
	//But this only causes a DCM if the provider natively supports ICPC.
	HRESULT hrCPC = QI(pDataSource, IID_IConnectionPointContainer, (void**)&pIUnknown2);

	//Defaults are always returned for non-DPO objects
	//Any DataSource that was not created through IDataInitialize, IDBPromptInitialize
	//we have defaults returned since SC cannot easily determine which values
	//have been changed.  The easiest way to determine weither this is a DSO
	//or a DPO is that a DPO object will have specific interfaces...
	HRESULT hrDPO = QI(pDataSource, IID_IDPOInternal, (void**)&pIUnknown);

	SAFE_RELEASE(pIUnknown2);
	SAFE_RELEASE(pIUnknown);
	return SUCCEEDED(hrCPC) && SUCCEEDED(hrDPO);
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::SetHiddenInitProperties
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::SetHiddenInitProperties(IUnknown* pDataSource)
{
	TBEGIN
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	HRESULT hr = E_FAIL;

	//Basically we need to make sure that Service Components can deal with 
	//existing properties that are already set on the provider, and they
	//are not returned in GetPropertyInfo.  Example:  A provider has "hidden"
	//properties that a consumer if they know about them can set, but they
	//are not advertised generically.  

	//Its kind of difficult to set provider hidden properties since its hard
	//to know if they got set unless you know exactly what functionality they 
	//expose, since GetProperty will not return hidden properties.  And this
	//senario would only work on one particular provider.
	
	//The easier way is to set our own defined property in our own 
	//set, this way if it truely gets passed to the provider, the provider will
	//fail, and we will know immediatly that ServiceComponents is indeed
	//actually passing this property.  Since GetPRoper

	//Define our own hidden property propset and hidden property
	const GUID	DBPROPSET_HIDDENPROPSET = {0x47c30871,0x11c8,0x11d2,{0xa9,0x87,0x00,0xc0,0x4f,0x94,0xa7,0x17}};
	const DBPROPID DBPROP_HIDDENPROP	= 2;

	//Setup the hidden property
	TESTC(::SetProperty(DBPROP_HIDDENPROP, DBPROPSET_HIDDENPROPSET, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL));
	
	//SetProperties
	TESTC_(SetInitProps(pDataSource),S_OK);
	TEST2C_(hr = SetProperties(pDataSource, cPropSets, rgPropSets),S_OK,DB_E_ERRORSOCCURRED);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}

	
////////////////////////////////////////////////////////////////////////////
//  CDBInit::IsDefaultValue
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::IsDefaultValue(IUnknown* pDataSource, DBPROPID dwPropertyID, GUID guidPropertySet, DBTYPE wType, void* pValue)
{
	TBEGIN
	BOOL bDefault = FALSE;
	IPersist* pIPersist = NULL;
	IUnknown* pIUnknown = NULL;
	IGetDataSource* pIGetDataSource = NULL;
	CLSID clsidProv;

	VARIANT vVariant;
	VARIANT vVariant2;
	VariantInit(&vVariant);
	VariantInit(&vVariant2);
	HRESULT hr = S_OK;
	
	//Get the CLSID for the passed in DataSource
	TEST2C_(hr = QI(pDataSource, IID_IPersist, (void**)&pIPersist),S_OK,E_NOINTERFACE);
	if(FAILED(hr))
	{
		//Is the user interested in getting defaults for the Session Object
		TESTC_(QI(pDataSource, IID_IGetDataSource, (void**)&pIGetDataSource),S_OK);
		TESTC_(pIGetDataSource->GetDataSource(IID_IPersist, (IUnknown**)&pIPersist),S_OK);
	}

	//Obtain a new datasource, (so there are no properties set...)
	QTESTC_(pIPersist->GetClassID(&clsidProv),S_OK);
	
	//How to create the "clean" DataSource
	if(IsDPO(pDataSource))
	{
		TESTC_(CreateDBInstance(clsidProv, NULL, IID_IUnknown, &pIUnknown),S_OK);
	}
	else
	{
		TESTC_(CreateInstance(clsidProv, NULL, IID_IUnknown, &pIUnknown),S_OK);
	}
	
	//Are they interesed in a Session
	if(pIGetDataSource)
	{
		//Create a Session
		TESTC_(InitializeDataSource(pIUnknown, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
		SAFE_RELEASE(pIGetDataSource);
		TESTC_(CreateNewSession(pIUnknown, IID_IGetDataSource, (IUnknown**)&pIGetDataSource),S_OK);
	}

	//Verify this property is the default on whatever object...
	QTESTC(GetProperty(dwPropertyID, guidPropertySet, pIGetDataSource ? pIGetDataSource : pIUnknown, &vVariant));

	//Create a Variant from the Passed in Values...
	if ((NULL == pValue) && ((DBTYPE_WSTR == wType) || (VT_BSTR == wType)))
	{
		WCHAR wstrZeroLength[] = L"";
		QTESTC_(CreateVariant(&vVariant2, wType, wstrZeroLength),S_OK);
	}
	else
	{
		QTESTC_(CreateVariant(&vVariant2, wType, pValue),S_OK);
	}

	//Now Verify they are Equal
	QTESTC(CompareVariant(&vVariant, &vVariant2));

	//If we have made it this far, its a default property...
	bDefault = TRUE;

CLEANUP:
	VariantClear(&vVariant);
	VariantClear(&vVariant2);
	SAFE_RELEASE(pIPersist);
	SAFE_RELEASE(pIUnknown);
	SAFE_RELEASE(pIGetDataSource);
	return bDefault;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::AreDefaultValues
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::AreDefaultValues(IUnknown* pIUnknown, ULONG cPropSets, DBPROPSET* rgPropSets)
{
	TBEGIN

	//Pural Version of IsDefaultValue
	for(ULONG iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		for(ULONG iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];

			//Ensure Default Value...
			//NOTE: This goes to cleanup on the first none-default value
			QTESTC(IsDefaultValue(pIUnknown, pProp->dwPropertyID, pPropSet->guidPropertySet, DBTYPE_VARIANT, &pProp->vValue));
		}
	}

CLEANUP:
	TRETURN
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::VerifyPassword
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::VerifyPassword(IUnknown* pDataSource, BOOL fIncludePassword, WCHAR* pwszInitString, BOOL fReturnedString, BOOL fAllowUnitialize)
{
	TBEGIN
	ASSERT(pwszInitString);
	WCHAR* pwszInitStringValue	= NULL;
	WCHAR* pwszInitStringPWD	= NULL;
	BOOL fIsDefault = FALSE;
	
	WCHAR* pwszPassword = NULL;
	WCHAR* pwszProviderString = NULL;
	WCHAR* pwszPWD = NULL;
	IDBInitialize* pIDBInitialize = NULL;

	//Determine Default Value...
	BOOL bSupported = GetProperty(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, pDataSource, &pwszPassword);
		fIsDefault	= IsDefaultValue(pDataSource, DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszPassword);
	BOOL bAuthInfo	= !GetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, pDataSource, VARIANT_FALSE);
		
	//Try to find the Password Keyword in the passed in String
	BOOL bFound		= GetModInfo()->GetStringKeywordValue(pwszInitString, GetStaticPropDesc(DBPROP_AUTH_PASSWORD), &pwszInitStringValue);
	
	//Try to find the ODBC PWD= Keyword in the passed in PROVIDERSTRING
	BOOL bFoundPWD	= FALSE;
	if(GetModInfo()->GetStringKeywordValue(pwszInitString, GetStaticPropDesc(DBPROP_INIT_PROVIDERSTRING), &pwszProviderString))
	{
		bFoundPWD = GetModInfo()->GetStringKeywordValue(pwszProviderString, L"PWD", &pwszInitStringPWD);
		SAFE_FREE(pwszProviderString);
	}

	//Additional Testing for PROVIDERSTRING that contains the ODBC Password (PWD=)
	if(GetProperty(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, pDataSource, &pwszProviderString) && pwszProviderString)
		GetModInfo()->GetStringKeywordValue(pwszProviderString, L"PWD", &pwszPWD);

	//There are 2 senarios you would call this method.
	//1 - fReturnedString = TRUE, where you have previously called GetInitString
	//					 and want to verify all properties set on the DataSource
	//					 are indeed returned in the string
	//2 - fReturnedString = FALSE, where you have previously called GetDataSource
	//					 passing it a String, and want to make sure the
	//					 returned DataSource does indeed contain the property
	//					 But it could also contain other properties besides 
	//					 what wass passed in the InitString

	if(fReturnedString)
	{
		//Password returned in the InitString is dependent upon a couple of factors
		//1.  DBPROP_AUTH_PASSWORD - must be supported (obviously...)
		//2.  GetInitializationString with fIncludePassword = TRUE 
		//3.  DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO - must be VARIANT_TRUE
		//4.  Must not be the default value
		//5.  Password property must not be empty...

		//The format of the InitString is offical, and it should be using
		//the property descriptions indicated by the OLD DB Spec, so there should
		//be a property keyword "Password=" in the InitString...

		//#1 - DBPROP_AUTH_PASSWORD - supported
		if(!bSupported)
		{
			if(bFound)
			{
				TERROR(L"Password returned in the InitString, but not a supported property!");
				QTESTC(FALSE);
			}
		}

		//#2 - GetInitializationString with fIncludePassword = TRUE 
		//The the user didn't request the password, then it shouldn't be in the string
		//no matter wither its supported or not...
		if(!fIncludePassword)
		{
			if(bFound || bFoundPWD)
			{
				TERROR(L"fIncludePassword=FALSE, but " << (bFoundPWD ? L"PWD" : L"PASSWORD") << " is still returned in the Initialization String");
				QTESTC(FALSE);
			}
		}

		//#3 - DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO - must be VARIANT_TRUE
		//If not then the password should not be returned...
		if(bAuthInfo == FALSE)
		{
			if(bFound || bFoundPWD)
			{
				TERROR(L"DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO=FALSE, but " << (bFoundPWD ? L"PWD" : L"PASSWORD") << " is still returned in the Initialization String");
				QTESTC(FALSE);
			}
		}

		//#4 - Previously set, Must not be the default value
		//Should not have returned the password in the string if Default, or the Property was not set...
		if(fIsDefault)
		{
			if(bFound || bFoundPWD)
			{
				TERROR((bFoundPWD ? L"PWD" : L"PASSWORD") << " is returned in the Initialzation string, even though its the default value");
				QTESTC(FALSE);
			}
		}

		//5.  Password property must not be empty...
		if(pwszPassword && !fIsDefault && fIncludePassword && bAuthInfo==TRUE)
		{
			if(!bFound)
			{
				TERROR(L"Password not returned in the Initialzation string!");
				QTESTC(FALSE);
			}
		}
	}
	else
	{
		//Otherwise this function is called from GetDataSource, and we need to very properties
		//as having the correct output from the string (ie: fReturnedString==FALSE), 
		//rather than verify the string represents the properties (ie: fReturnedString==TRUE).
		
		//Password returned on the DataSource is dependent upon a couple of factors
		//1.  DBPROP_AUTH_PASSWORD - must be supported (obviously...)
		//2.  Uninitialized DataSource - password should always be returned
		//3.  Initialized DataSource DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO - must be VARIANT_TRUE
		//4.  Uninitialized DataSource (previously Initialized) DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO - must be VARIANT_TRUE before Initializartion

		//1.  DBPROP_AUTH_PASSWORD - must be supported (obviously...)
		if(!bSupported)
		{
			if(bFound)
			{
				TERROR(L"Password returned on DataSource, but not a supported property!");
				QTESTC(FALSE);
			}
		}
		
		//2.  Uninitialized DataSource - password should always be returned
		if(!IsInitialized(pDataSource))
		{
			if((bFound && !pwszPassword && pwszInitStringValue && pwszInitStringValue[0]) || 
				(bFoundPWD && !pwszPWD && pwszInitStringPWD && pwszInitStringPWD[0]))
			{
				//If password existed in InitString, it should be on Uninitialized DataSource 
				TERROR(((bFoundPWD && !pwszPWD) ? L"PWD" : L"PASSWORD") << L" existed in InitString, but not available on Uninitialized DataSource");
				QTESTC(FALSE);
			}
		}
		
		//3.  Initialized DataSource DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO - must be VARIANT_TRUE
		else if(ShouldInitialize(pDataSource))
		{
			//If the password property is available, AuthInfo MUST be true
			if(bAuthInfo==FALSE && ((pwszPassword && !fIsDefault) || pwszPWD))
			{
				TERROR((pwszPWD ? L"PWD" : L"PASSWORD") << L" returned on Initialized DataSource even when DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO=FALSE");
				QTESTC(FALSE);
			}
		

			if(fAllowUnitialize)
			{
				//4.  Uninitialized DataSource (previously Initialized) DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO - must be VARIANT_TRUE before Initializartion
				//Obtain IDBInitialize interface
				TESTC_(QI(pDataSource, IID_IDBInitialize, (void**)&pIDBInitialize),S_OK);
				TESTC_(pIDBInitialize->Uninitialize(),S_OK);
				
				//PASSWORD
				SAFE_FREE(pwszPassword);
				if(GetProperty(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, pDataSource, &pwszPassword))
				{
					if(bAuthInfo == FALSE && (pwszPassword && !fIsDefault))
					{
						TERROR(L"Password returned on Uninitialized DataSource (previsouly Initialized) even when DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO=FALSE before Initialization");
						QTESTC(FALSE);
					}
				}

				//PWD
				SAFE_FREE(pwszProviderString);
				if(GetProperty(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, pDataSource, &pwszProviderString))
				{
					if(bAuthInfo == FALSE && GetModInfo()->GetStringKeywordValue(pwszProviderString, L"PWD", &pwszPWD))
					{
						TERROR(L"PWD returned on Uninitialized DataSource (previsouly Initialized) even when DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO=FALSE before Initialization");
						QTESTC(FALSE);
					}
				}
		
				//Initialize to restore state...
				TESTC_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE),S_OK);
			}
		}
	}

	//If we have made it this far, then verify the property returned is the same as specified
	//in the InitString...
	if(pwszPassword && pwszInitStringValue)
	{
		if(wcscmp(pwszPassword, pwszInitStringValue)!=0)
		{
			TERROR("Property Values for Password don't match!  InitString: \"" << pwszInitStringValue << "\" GetProperties: \"" << pwszPassword << "\"");
			QTESTC(FALSE);
		}
	}

	//If we have made it this far, then verify the property returned is the same as specified
	//in the InitString...
	if(pwszPWD && pwszInitStringPWD)
	{
		if(wcscmp(pwszPWD, pwszInitStringPWD)!=0)
		{
			TERROR("Property Values for PWD don't match!  InitString: \"" << pwszInitStringPWD << "\" GetProperties: \"" << pwszPWD << "\"");
			QTESTC(FALSE);
		}
	}

CLEANUP:
	SAFE_FREE(pwszInitStringValue);
	SAFE_FREE(pwszInitStringPWD);
	SAFE_FREE(pwszPassword);
	SAFE_FREE(pwszProviderString);
	SAFE_FREE(pwszPWD);
	SAFE_RELEASE(pIDBInitialize);	//Can't Initialize this as the user is still using it...
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::SetInitProps
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::SetInitProps(IUnknown* pDataSource)
{
	TBEGIN
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//Obtain Required Init Properties (from the Init String)
	TESTC(GetInitProps(&cPropSets, &rgPropSets));

	//Now set those on the DataSource
	QTESTC_(hr = SetProperties(pDataSource, cPropSets, rgPropSets),S_OK);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::SetProperties
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::SetProperties(IUnknown* pDataSource, ULONG cPropSets, DBPROPSET* rgPropSets)
{
	TBEGIN
	ASSERT(pDataSource);
	HRESULT hr = S_OK;
	IDBProperties* pIDBProperites = NULL;

	//Obtain IDBProperties interface
	TESTC_(hr = QI(pDataSource, IID_IDBProperties, (void**)&pIDBProperites),S_OK);

	//SetProperties
	QTESTC_(hr = pIDBProperites->SetProperties(cPropSets, rgPropSets),S_OK);

CLEANUP:
	SAFE_RELEASE(pIDBProperites);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::GetProperties
//
////////////////////////////////////////////////////////////////////////////
HRESULT CDBInit::GetProperties(IUnknown* pDataSource, ULONG cPropertyIDSets, DBPROPIDSET* rgPropertyIDSets, ULONG* pcPropSets, DBPROPSET** prgPropSets)
{
	ASSERT(pDataSource);
	TBEGIN
	HRESULT hr = S_OK;
	IDBProperties* pIDBProperites = NULL;

	//Obtain IDBProperties interface
	TESTC_(hr = QI(pDataSource, IID_IDBProperties, (void**)&pIDBProperites),S_OK);

	//GetProperties
	QTESTC_(hr = pIDBProperites->GetProperties(cPropertyIDSets, rgPropertyIDSets, pcPropSets, prgPropSets),S_OK);

CLEANUP:
	SAFE_RELEASE(pIDBProperites);
	return hr;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CompareProperties
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::CompareProperties(IUnknown* pDataSource, IUnknown* pDataSource2, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions)
{
	ASSERT(pDataSource);
	ASSERT(pDataSource2);
	TBEGIN
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	ULONG cPropSets2 = 0;
	DBPROPSET* rgPropSets2 = NULL;

	//GetProperties for passed in DataSource
	TESTC_(hr = GetProperties(pDataSource, 0, NULL, &cPropSets, &rgPropSets), S_OK);

	//GetProperties for passed in DataSource
	TESTC_(hr = GetProperties(pDataSource2, 0, NULL, &cPropSets2, &rgPropSets2), S_OK);

	//GetProperties
	QTESTC(EqualPropSets(cPropSets, rgPropSets, cPropSets2, rgPropSets2, eCompareOptions));

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	::FreeProperties(&cPropSets2, &rgPropSets2);
	TRETURN;
}


////////////////////////////////////////////////////////////////////////////
//  CDBInit::CompareProperties
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::CompareProperties(IUnknown* pDataSource, ULONG cPropSets, DBPROPSET* rgPropSets, enum PROPSETCOMPAREOPTIONS_ENUM eCompareOptions)
{
	ASSERT(pDataSource);
	TBEGIN
	HRESULT hr = S_OK;
	ULONG cPropSets2 = 0;
	DBPROPSET* rgPropSets2 = NULL;

	//GetProperties for passed in DataSource
	TESTC_(hr = GetProperties(pDataSource, 0, NULL, &cPropSets2, &rgPropSets2), S_OK);

	//GetProperties
	QTESTC(EqualPropSets(cPropSets, rgPropSets, cPropSets2, rgPropSets2, eCompareOptions));

CLEANUP:
	::FreeProperties(&cPropSets2, &rgPropSets2);
	TRETURN;
}


///////////////////////////////////////////////////////////////////////////
//  CDBInit::Terminate
//
////////////////////////////////////////////////////////////////////////////
BOOL CDBInit::Terminate()
{
	return CDataSource::Terminate();
}




// {{ TCW_TEST_CASE_MAP(TCCreateDBInstance)
//*-----------------------------------------------------------------------
// @class Test IDataInitialize::CreateDBInstance
//
class TCCreateDBInstance : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateDBInstance,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - ClassFactory - CoGetClassObject(valid riids) - S_OK
	int Variation_1();
	// @cmember General - ClassFactory - CoGetClassObject(invalid riids) - E_NOINTERFACE
	int Variation_2();
	// @cmember General - ClassFactory - CoGetClassObject(NULL) - E_INVALIDARG
	int Variation_3();
	// @cmember General - ClassFactory - CreateInstance(valid riids) - S_OK
	int Variation_4();
	// @cmember General - ClassFactory - CreateInstance(invalid riids) - E_NOINTERFACE
	int Variation_5();
	// @cmember General - ClassFactory - CreateInstance(NULL) - E_INVALIDARG
	int Variation_6();
	// @cmember General - ClassFactory - LockServer combinations
	int Variation_7();
	// @cmember General - ClassFactory - QI combinations
	int Variation_8();
	// @cmember General - ClassFactory - AddRef/Release QI combinations
	int Variation_9();
	// @cmember General - CoCreateInstance - Aggregation - non-IUnknown
	int Variation_10();
	// @cmember General - CoCreateInstance - Aggregation - all valid combinations
	int Variation_11();
	// @cmember General - CoCreateInstance - Verify DCM is Aggregated with DataSource
	int Variation_12();
	// @cmember General - CoCreateInstance - Verify DCM is Aggregated with Aggregated DataSource
	int Variation_13();
	// @cmember General - IUnknown - QI(valid riids) - S_OK
	int Variation_14();
	// @cmember General - IUnknown - QI(invalid riids) - E_NOINTERFACE
	int Variation_15();
	// @cmember General - IUnknown - QI(NULL) - E_INVALIDARG
	int Variation_16();
	// @cmember General - IUnknown - AddRef/Release combinations
	int Variation_17();
	// @cmember Boundary - REGDB_E_CLASSNOTREG - Invalid CLSID
	int Variation_18();
	// @cmember Boundary - REGDB_E_CLASSNOTREG - CLSID_NULL
	int Variation_19();
	// @cmember Boundary - DB_E_NOAGGREGATION - Provider doesn't support Aggregation
	int Variation_20();
	// @cmember Boundary - E_INVALIDARG - Invalid CLSCTX
	int Variation_21();
	// @cmember Boundary - E_INVALIDARG - ppDataSource == NULL
	int Variation_22();
	// @cmember Boundary - E_NOINTERFACE - invalid riids
	int Variation_23();
	// @cmember Parameters - clsidProvider -  Enumerator Object - E_NOINTERFACE
	int Variation_24();
	// @cmember Parameters - pUnkOuter - Aggregation, non-IUnknown
	int Variation_25();
	// @cmember Parameters - pUnkOuter - Aggregation, verify inner
	int Variation_26();
	// @cmember Parameters - pUnkOuter - Aggregation, verify Session, Command, Rowset can obtain aggregated DataSource
	int Variation_27();
	// @cmember Parameters - dwCLSCTX - CLSCTX_INPROC_SERVER
	int Variation_28();
	// @cmember Parameters - dwCLSCTX - CLSCTX_INPROC_HANDLER
	int Variation_29();
	// @cmember Parameters - dwCLSCTX - CLSCTX_SERVER
	int Variation_30();
	// @cmember Parameters - dwCLSCTX - CLSCTX_ALL
	int Variation_31();
	// @cmember Parameters - dwCLSCTX - CLSCTX_LOCAL_SERVER
	int Variation_32();
	// @cmember Parameters - dwCLSCTX - CLSCTX_REMOTE_SERVER
	int Variation_33();
	// @cmember Parameters - pwszReserved - valid
	int Variation_34();
	// @cmember Parameters - riid - IID_IUnknown - S_OK
	int Variation_35();
	// @cmember Parameters - riid - all TDataSource interfaces - S_OK
	int Variation_36();
	// @cmember Parameters - riid - Provider Specific interfaces - S_OK
	int Variation_37();
	// @cmember Stress - 100 DataSources
	int Variation_38();
	// @cmember Stress - Verify FreeThreaded
	int Variation_39();
	// @cmember MultiUser - CreateDBInstance from 3 threads, all succeess
	int Variation_40();
	// @cmember MultiUser - CreateDBInstance from 3 threads, at least one error
	int Variation_41();
	// @cmember MultiUser - GetDataSource from 3 threads - each same provider with at least one in error
	int Variation_42();
	// @cmember Parameters - Provider hidden properties set
	int Variation_43();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateDBInstance)
#define THE_CLASS TCCreateDBInstance
BEG_TEST_CASE(TCCreateDBInstance, CDBInit, L"Test IDataInitialize::CreateDBInstance")
	TEST_VARIATION(1, 		L"General - ClassFactory - CoGetClassObject(valid riids) - S_OK")
	TEST_VARIATION(2, 		L"General - ClassFactory - CoGetClassObject(invalid riids) - E_NOINTERFACE")
	TEST_VARIATION(3, 		L"General - ClassFactory - CoGetClassObject(NULL) - E_INVALIDARG")
	TEST_VARIATION(4, 		L"General - ClassFactory - CreateInstance(valid riids) - S_OK")
	TEST_VARIATION(5, 		L"General - ClassFactory - CreateInstance(invalid riids) - E_NOINTERFACE")
	TEST_VARIATION(6, 		L"General - ClassFactory - CreateInstance(NULL) - E_INVALIDARG")
	TEST_VARIATION(7, 		L"General - ClassFactory - LockServer combinations")
	TEST_VARIATION(8, 		L"General - ClassFactory - QI combinations")
	TEST_VARIATION(9, 		L"General - ClassFactory - AddRef/Release QI combinations")
	TEST_VARIATION(10, 		L"General - CoCreateInstance - Aggregation - non-IUnknown")
	TEST_VARIATION(11, 		L"General - CoCreateInstance - Aggregation - all valid combinations")
	TEST_VARIATION(12, 		L"General - CoCreateInstance - Verify DCM is Aggregated with DataSource")
	TEST_VARIATION(13, 		L"General - CoCreateInstance - Verify DCM is Aggregated with Aggregated DataSource")
	TEST_VARIATION(14, 		L"General - IUnknown - QI(valid riids) - S_OK")
	TEST_VARIATION(15, 		L"General - IUnknown - QI(invalid riids) - E_NOINTERFACE")
	TEST_VARIATION(16, 		L"General - IUnknown - QI(NULL) - E_INVALIDARG")
	TEST_VARIATION(17, 		L"General - IUnknown - AddRef/Release combinations")
	TEST_VARIATION(18, 		L"Boundary - REGDB_E_CLASSNOTREG - Invalid CLSID")
	TEST_VARIATION(19, 		L"Boundary - REGDB_E_CLASSNOTREG - CLSID_NULL")
	TEST_VARIATION(20, 		L"Boundary - DB_E_NOAGGREGATION - Provider doesn't support Aggregation")
	TEST_VARIATION(21, 		L"Boundary - E_INVALIDARG - Invalid CLSCTX")
	TEST_VARIATION(22, 		L"Boundary - E_INVALIDARG - ppDataSource == NULL")
	TEST_VARIATION(23, 		L"Boundary - E_NOINTERFACE - invalid riids")
	TEST_VARIATION(24, 		L"Parameters - clsidProvider -  Enumerator Object - E_NOINTERFACE")
	TEST_VARIATION(25, 		L"Parameters - pUnkOuter - Aggregation, non-IUnknown")
	TEST_VARIATION(26, 		L"Parameters - pUnkOuter - Aggregation, verify inner")
	TEST_VARIATION(27, 		L"Parameters - pUnkOuter - Aggregation, verify Session, Command, Rowset can obtain aggregated DataSource")
	TEST_VARIATION(28, 		L"Parameters - dwCLSCTX - CLSCTX_INPROC_SERVER")
	TEST_VARIATION(29, 		L"Parameters - dwCLSCTX - CLSCTX_INPROC_HANDLER")
	TEST_VARIATION(30, 		L"Parameters - dwCLSCTX - CLSCTX_SERVER")
	TEST_VARIATION(31, 		L"Parameters - dwCLSCTX - CLSCTX_ALL")
	TEST_VARIATION(32, 		L"Parameters - dwCLSCTX - CLSCTX_LOCAL_SERVER")
	TEST_VARIATION(33, 		L"Parameters - dwCLSCTX - CLSCTX_REMOTE_SERVER")
	TEST_VARIATION(34, 		L"Parameters - pwszReserved - valid")
	TEST_VARIATION(35, 		L"Parameters - riid - IID_IUnknown - S_OK")
	TEST_VARIATION(36, 		L"Parameters - riid - all TDataSource interfaces - S_OK")
	TEST_VARIATION(37, 		L"Parameters - riid - Provider Specific interfaces - S_OK")
	TEST_VARIATION(38, 		L"Stress - 100 DataSources")
	TEST_VARIATION(39, 		L"Stress - Verify FreeThreaded")
	TEST_VARIATION(40, 		L"MultiUser - CreateDBInstance from 3 threads, all succeess")
	TEST_VARIATION(41, 		L"MultiUser - CreateDBInstance from 3 threads, at least one error")
	TEST_VARIATION(42, 		L"MultiUser - GetDataSource from 3 threads - each same provider with at least one in error")
	TEST_VARIATION(43, 		L"Parameters - Provider hidden properties set")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCCreateDBInstanceEx)
//*-----------------------------------------------------------------------
// @class Test IDataInitialize::CreateDBInstanceEx
//
class TCCreateDBInstanceEx : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCCreateDBInstanceEx,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Verify DCM is Aggregated with DataSource
	int Variation_1();
	// @cmember General - Verify DCM is Aggregated with Aggregated DataSource
	int Variation_2();
	// @cmember Boundary - REGDB_E_CLASSNOTREG - Invalid clsid
	int Variation_3();
	// @cmember Boundary - REGDB_E_CLASSNOTREG - CLSID_NULL
	int Variation_4();
	// @cmember Boundary - DB_E_NOAGGREGATION - Possible on a provider that doesn't support aggregation
	int Variation_5();
	// @cmember Boundary - DB_E_NOAGGREGATION - Specify out of process
	int Variation_6();
	// @cmember Boundary - E_INVALIDARG - Invalid CLSCTX
	int Variation_7();
	// @cmember Boundary - E_INVALIDARG - cmq == 0
	int Variation_8();
	// @cmember Boundary - E_INVALIDARG - rgmqResults == NULL
	int Variation_9();
	// @cmember Boundary - E_INVALIDARG - rgmqResults[0].pIID == NULL
	int Variation_10();
	// @cmember Boundary - E_INVALIDARG - Verify combinations of NULL pointers inside pServerInfo
	int Variation_11();
	// @cmember Boundary - E_INVALIDARG - Verify Authenitcation Information
	int Variation_12();
	// @cmember Boundary - CO_S_NOTALLINTERFACES - (IUnknown, IDBInitialize, IRowset)
	int Variation_13();
	// @cmember Boundary - E_NOINTERFACE - (IID_NULL, IDBInitialize, IOpenRowset)
	int Variation_14();
	// @cmember Parameters - clsidProvider -  Enumerator Object - E_NOINTERFACE
	int Variation_15();
	// @cmember Parameters - pUnkOuter - Aggregation, not asking for IID_IUnknown
	int Variation_16();
	// @cmember Parameters - pUnkOuter - Aggregation, verify inner
	int Variation_17();
	// @cmember Parameters - pUnkOuter - Aggregation, verify Session, Command, Rowset, can all get back to the aggregated DataSource
	int Variation_18();
	// @cmember Parameters - dwCLSCTX - CLSCTX_INPROC_SERVER
	int Variation_19();
	// @cmember Parameters - dwCLSCTX - CLSCTX_INPROC_HANDLER
	int Variation_20();
	// @cmember Parameters - dwCLSCTX - CLSCTX_SERVER
	int Variation_21();
	// @cmember Parameters - dwCLSCTX - CLSCTX_ALL
	int Variation_22();
	// @cmember Parameters - dwCLSCTX - CLSCTX_LOCAL_SERVER
	int Variation_23();
	// @cmember Parameters - dwCLSCTX - CLSCTX_REMOTE_SERVER
	int Variation_24();
	// @cmember Parameters - pwszReserved - ingored
	int Variation_25();
	// @cmember Parameters - cmq - Combinations
	int Variation_26();
	// @cmember Parameters - rgmqResults - IID_IUnknown - S_OK
	int Variation_27();
	// @cmember Parameters - rgmqResults - { all DataSource interfaces } - S_OK
	int Variation_28();
	// @cmember Parameters - rgmqResults - { Provider Specific Interfaces } - S_OK
	int Variation_29();
	// @cmember Stress - 100 DataSources
	int Variation_30();
	// @cmember MultiUser - CreateDBInstance from 3 threads, all success
	int Variation_31();
	// @cmember MultiUser - CreateDBInstance from 3 threads, at least one error
	int Variation_32();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCCreateDBInstanceEx)
#define THE_CLASS TCCreateDBInstanceEx
BEG_TEST_CASE(TCCreateDBInstanceEx, CDBInit, L"Test IDataInitialize::CreateDBInstanceEx")
	TEST_VARIATION(1, 		L"General - Verify DCM is Aggregated with DataSource")
	TEST_VARIATION(2, 		L"General - Verify DCM is Aggregated with Aggregated DataSource")
	TEST_VARIATION(3, 		L"Boundary - REGDB_E_CLASSNOTREG - Invalid clsid")
	TEST_VARIATION(4, 		L"Boundary - REGDB_E_CLASSNOTREG - CLSID_NULL")
	TEST_VARIATION(5, 		L"Boundary - DB_E_NOAGGREGATION - Possible on a provider that doesn't support aggregation")
	TEST_VARIATION(6, 		L"Boundary - DB_E_NOAGGREGATION - Specify out of process")
	TEST_VARIATION(7, 		L"Boundary - E_INVALIDARG - Invalid CLSCTX")
	TEST_VARIATION(8, 		L"Boundary - E_INVALIDARG - cmq == 0")
	TEST_VARIATION(9, 		L"Boundary - E_INVALIDARG - rgmqResults == NULL")
	TEST_VARIATION(10, 		L"Boundary - E_INVALIDARG - rgmqResults[0].pIID == NULL")
	TEST_VARIATION(11, 		L"Boundary - E_INVALIDARG - Verify combinations of NULL pointers inside pServerInfo")
	TEST_VARIATION(12, 		L"Boundary - E_INVALIDARG - Verify Authenitcation Information")
	TEST_VARIATION(13, 		L"Boundary - CO_S_NOTALLINTERFACES - (IUnknown, IDBInitialize, IRowset)")
	TEST_VARIATION(14, 		L"Boundary - E_NOINTERFACE - (IID_NULL, IDBInitialize, IOpenRowset)")
	TEST_VARIATION(15, 		L"Parameters - clsidProvider -  Enumerator Object - E_NOINTERFACE")
	TEST_VARIATION(16, 		L"Parameters - pUnkOuter - Aggregation, not asking for IID_IUnknown")
	TEST_VARIATION(17, 		L"Parameters - pUnkOuter - Aggregation, verify inner")
	TEST_VARIATION(18, 		L"Parameters - pUnkOuter - Aggregation, verify Session, Command, Rowset, can all get back to the aggregated DataSource")
	TEST_VARIATION(19, 		L"Parameters - dwCLSCTX - CLSCTX_INPROC_SERVER")
	TEST_VARIATION(20, 		L"Parameters - dwCLSCTX - CLSCTX_INPROC_HANDLER")
	TEST_VARIATION(21, 		L"Parameters - dwCLSCTX - CLSCTX_SERVER")
	TEST_VARIATION(22, 		L"Parameters - dwCLSCTX - CLSCTX_ALL")
	TEST_VARIATION(23, 		L"Parameters - dwCLSCTX - CLSCTX_LOCAL_SERVER")
	TEST_VARIATION(24, 		L"Parameters - dwCLSCTX - CLSCTX_REMOTE_SERVER")
	TEST_VARIATION(25, 		L"Parameters - pwszReserved - ingored")
	TEST_VARIATION(26, 		L"Parameters - cmq - Combinations")
	TEST_VARIATION(27, 		L"Parameters - rgmqResults - IID_IUnknown - S_OK")
	TEST_VARIATION(28, 		L"Parameters - rgmqResults - { all DataSource interfaces } - S_OK")
	TEST_VARIATION(29, 		L"Parameters - rgmqResults - { Provider Specific Interfaces } - S_OK")
	TEST_VARIATION(30, 		L"Stress - 100 DataSources")
	TEST_VARIATION(31, 		L"MultiUser - CreateDBInstance from 3 threads, all success")
	TEST_VARIATION(32, 		L"MultiUser - CreateDBInstance from 3 threads, at least one error")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCGetDataSource)
//*-----------------------------------------------------------------------
// @class Test IDataInitialize::GetDataSource
//
class TCGetDataSource : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetDataSource,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Verify DCM is Aggregated with DataSource
	int Variation_1();
	// @cmember General - Verify DCM is Aggregated with Aggregated DataSource
	int Variation_2();
	// @cmember Boundary - DB_E_NOAGGREGATION - Provider may not support aggregation
	int Variation_3();
	// @cmember Boundary - E_INVALIDARG - Invalid CLSCTX
	int Variation_4();
	// @cmember Boundary - E_INVALIDARG - pwszInitString==NULL
	int Variation_5();
	// @cmember Boundary - E_INVALIDARG - ppDataSource==NULL
	int Variation_6();
	// @cmember Boundary - E_NOINTERFACE - IID_INULL
	int Variation_7();
	// @cmember Boundary - E_NOINTERFACE - non-DataSource Object
	int Variation_8();
	// @cmember Boundary - E_NOINTERFACE - Enumerator Object
	int Variation_9();
	// @cmember Boundary - E_NOINTERFACE - IID_IDataInitialize
	int Variation_10();
	// @cmember Parameters - pUnkOuter - Aggregation, not passing IID_IUnknown
	int Variation_11();
	// @cmember Parameters - pUnkOuter - Aggregation, verify inner
	int Variation_12();
	// @cmember Parameters - pUnkOuter - Aggregation, verify GetDataSource returns outer
	int Variation_13();
	// @cmember Parameters - CLSCTX - CLSCTX_INPROC_SERVER
	int Variation_14();
	// @cmember Parameters - CLSCTX - CLSCTX_INPROC_HANDLER
	int Variation_15();
	// @cmember Parameters - CLSCTX - CLSCTX_SERVER
	int Variation_16();
	// @cmember Parameters - CLSCTX - CLSCTX_ALL
	int Variation_17();
	// @cmember Parameters - CLSCTX - CLSCTX_LOCAL_SERVER
	int Variation_18();
	// @cmember Parameters - CLSCTX - CLSCTX_REMOTE_SERVER
	int Variation_19();
	// @cmember Parameters - pwszInitString - Contains different ProgID
	int Variation_20();
	// @cmember Parameters - pwszInitString - Contains different ProgID, with versioning
	int Variation_21();
	// @cmember Parameters - pwszInitString - Contains same ProgID, but with versioning
	int Variation_22();
	// @cmember Parameters - pwszInitString -  Enumerator Object
	int Variation_23();
	// @cmember Parameters - riid - IID_IUnknown
	int Variation_24();
	// @cmember Parameters - riid - verify all TDataSource interfaces
	int Variation_25();
	// @cmember Parameters - riid - Provider Specific interfaces - S_OK
	int Variation_26();
	// @cmember Parameters - ppDataSource - Uninitialized DSO - complete init string
	int Variation_27();
	// @cmember Parameters - ppDataSource - Uninitialized DSO - init string with no props
	int Variation_28();
	// @cmember Parameters - ppDataSource - Uninitialized DSO - init string with another provider
	int Variation_29();
	// @cmember Parameters - ppDataSource - Uninitialized DSO - init string with no provider in the init string specified
	int Variation_30();
	// @cmember Parameters - ppDataSource - Uninitialized DSO - init string with invalid ProgID
	int Variation_31();
	// @cmember Parameters - ppDataSource - Uninitialized DSO - init string with invalid Provider String
	int Variation_32();
	// @cmember Parameters - ppDataSource - Initailized DSO - Complete init string - DB_E_ERRORSOCCURRED
	int Variation_33();
	// @cmember Parameters - ppDataSource - Initailized DSO - Complete init string with no props
	int Variation_34();
	// @cmember Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an open session
	int Variation_35();
	// @cmember Parameters - ppDataSource - Initailized DSO - Complete init string with no provider specified
	int Variation_36();
	// @cmember Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an invalid ProgID
	int Variation_37();
	// @cmember Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an invalid Provider String
	int Variation_38();
	// @cmember Strings - Keywords - DBPROP_AUTH_CACHE_AUTHINFO
	int Variation_39();
	// @cmember Strings - Keywords - DBPROP_AUTH_ENCRYPT_PASSWORD
	int Variation_40();
	// @cmember Strings - Keywords - DBPROP_AUTH_INTEGRATED
	int Variation_41();
	// @cmember Strings - Keywords - DBPROP_AUTH_MASK_PASSWORD
	int Variation_42();
	// @cmember Strings - Keywords - DBPROP_AUTH_PASSWORD
	int Variation_43();
	// @cmember Strings - Keywords - DBPROP_AUTH_PERSIST_ENCRYPTED
	int Variation_44();
	// @cmember Strings - Keywords - DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO
	int Variation_45();
	// @cmember Strings - Keywords - DBPROP_AUTH_USERID
	int Variation_46();
	// @cmember Strings - Keywords - DBPROP_INIT_ASYNCH
	int Variation_47();
	// @cmember Strings - Keywords - DBPROP_INIT_DATASOURCE
	int Variation_48();
	// @cmember Strings - Keywords - DBPROP_INIT_HWND
	int Variation_49();
	// @cmember Strings - Keywords - DBPROP_INIT_IMPERSONATION_LEVEL
	int Variation_50();
	// @cmember Strings - Keywords - DBPROP_INIT_LCID
	int Variation_51();
	// @cmember Strings - Keywords - DBPROP_INIT_LOCATION
	int Variation_52();
	// @cmember Strings - Keywords - DBPROP_INIT_MODE
	int Variation_53();
	// @cmember Strings - Keywords - DBPROP_INIT_PROMPT
	int Variation_54();
	// @cmember Strings - Keywords - DBPROP_INIT_PROTECTION_LEVEL
	int Variation_55();
	// @cmember Strings - Keywords - DBPROP_INIT_PROVIDERSTRING
	int Variation_56();
	// @cmember Strings - Keywords - DBPROP_INIT_TIMEOUT
	int Variation_57();
	// @cmember Strings - Keywords - DBPROP_INIT_GENERALTIMEOUT
	int Variation_58();
	// @cmember Strings - Keywords - All Properties (including Provider Specific) - together
	int Variation_59();
	// @cmember Strings - Keywords - All Properties (including Provider Specific) - seperate
	int Variation_60();
	// @cmember Strings - Keywords - Property specified more than once with diff values
	int Variation_61();
	// @cmember Strings - Keywords - Verify Case Insensitive
	int Variation_62();
	// @cmember Strings - Keywords - Specified more than once, second in error
	int Variation_63();
	// @cmember Strings - Provider Keyword - Just Provider Keyword
	int Variation_64();
	// @cmember Strings - Provider Keyword - Not first item in the list
	int Variation_65();
	// @cmember Strings - Provider Keyword - Versioned Provider ProgID
	int Variation_66();
	// @cmember Strings - Provider Keyword - Provider ProgID which contains a space
	int Variation_67();
	// @cmember Strings - Provider Keyword - Missing
	int Variation_68();
	// @cmember Strings - Provider Keyword - Versioned Provider ProgID that doesn't exist, should fall back
	int Variation_69();
	// @cmember Strings - Provider Keyword - Specified twice
	int Variation_70();
	// @cmember Strings - Provider Keyword - Provider=; - error
	int Variation_71();
	// @cmember Strings - Friendly Names - Allow integer equivilent
	int Variation_72();
	// @cmember Strings - Friendly Names - Numeric Value out of range
	int Variation_73();
	// @cmember Strings - Friendly Names - oring Friendly Names
	int Variation_74();
	// @cmember Strings - Friendly Names - oring same Friendly Name
	int Variation_75();
	// @cmember Strings - Friendly Names - wrong Friendly Names
	int Variation_76();
	// @cmember Strings - Friendly Names - oring values for nonMode props - error
	int Variation_77();
	// @cmember Strings - Property Values - Contains same value as keyword
	int Variation_78();
	// @cmember Strings - Reserved SemiColon - Last item doesn't contain semicolon
	int Variation_79();
	// @cmember Strings - Reserved SemiColon - Provider=progid with no trailing semi
	int Variation_80();
	// @cmember Strings - Reserved SemiColon - Double semi
	int Variation_81();
	// @cmember Strings - Reserved SemiColon - Semicolon in keyword
	int Variation_82();
	// @cmember Strings - Reserved SemiColon - Semicolon in Value - (quoted)
	int Variation_83();
	// @cmember Strings - Reserved SemiColon - Semicolon following keyword Data Source =;
	int Variation_84();
	// @cmember Strings - Reserved SemiColon - Semicolon following keyword Extended Properties;
	int Variation_85();
	// @cmember Strings - Reserved SemiColon - Middle item missing semi - error
	int Variation_86();
	// @cmember Strings - Reserved SemiColon - SemiColon in value - not qouted - error
	int Variation_87();
	// @cmember Strings - Reserved EqualSign - In Keyword - Doubled
	int Variation_88();
	// @cmember Strings - Reserved EqualSign - In Value - not Doubled
	int Variation_89();
	// @cmember Strings - Reserved EqualSign - In Value - Doubled - should count as 2
	int Variation_90();
	// @cmember Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - In Keyword - not doubled - error
	int Variation_91();
	// @cmember Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - Completly missing - error
	int Variation_92();
	// @cmember Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - Keyword==value; - error
	int Variation_93();
	// @cmember Strings - Spaces - Before/After keywords
	int Variation_94();
	// @cmember Strings - Spaces - Before/After equal sign
	int Variation_95();
	// @cmember Strings - Spaces - Before/After semi
	int Variation_96();
	// @cmember Strings - Spaces - Before/After value- ignored
	int Variation_97();
	// @cmember Strings - Spaces - Before/After value - quoted - retained
	int Variation_98();
	// @cmember Strings - Spaces - in value - retained
	int Variation_99();
	// @cmember Strings - Spaces - Before string, After string
	int Variation_100();
	// @cmember Strings - Spaces - Before/After orable token (|)
	int Variation_101();
	// @cmember Strings - Spaces - In Keyword that already has spaces
	int Variation_102();
	// @cmember Strings - Quotes - All value single quotes
	int Variation_103();
	// @cmember Strings - Quotes - All value double quotes
	int Variation_104();
	// @cmember Strings - Quotes - Keywords single and Double quoted
	int Variation_105();
	// @cmember Strings - Quotes - Embedding Single
	int Variation_106();
	// @cmember Strings - Quotes - Embedding Double
	int Variation_107();
	// @cmember Strings - Quotes - Both Embedding Single and Double starting with Single
	int Variation_108();
	// @cmember Strings - Quotes - Both Embedding Single and Double starting with Double
	int Variation_109();
	// @cmember Strings - Quotes - Just a pain!
	int Variation_110();
	// @cmember Strings - Quotes - Uneven quotes
	int Variation_111();
	// @cmember Strings - Quotes - Doubled Quotes
	int Variation_112();
	// @cmember Strings - Quotes - Tripled Quotes
	int Variation_113();
	// @cmember Strings - Special Chars - DBCS
	int Variation_114();
	// @cmember Strings - Special Chars - All Printable Chars
	int Variation_115();
	// @cmember Strings - Special Chars - non-printable
	int Variation_116();
	// @cmember Strings - Special Chars - Localized Property Descriptions
	int Variation_117();
	// @cmember MultiUser - GetDataSource from 3 threads - each asking for all diff interfaces, all dealing with the same provider
	int Variation_118();
	// @cmember MultiUser - GetDataSource from 3 threads - each asking for all diff interfaces, all same provider, with an error
	int Variation_119();
	// @cmember Keywords - DBPROP_INIT_CATALOG
	int Variation_120();
	// @cmember Keywords - DBPROP_INIT_OLEDBSERVICES
	int Variation_121();
	// @cmember Parameters - Provider hidden properties set before
	int Variation_122();
	// @cmember Parameters - Provider hidden properties set after
	int Variation_123();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGetDataSource)
#define THE_CLASS TCGetDataSource
BEG_TEST_CASE(TCGetDataSource, CDBInit, L"Test IDataInitialize::GetDataSource")
	TEST_VARIATION(1, 		L"General - Verify DCM is Aggregated with DataSource")
	TEST_VARIATION(2, 		L"General - Verify DCM is Aggregated with Aggregated DataSource")
	TEST_VARIATION(3, 		L"Boundary - DB_E_NOAGGREGATION - Provider may not support aggregation")
	TEST_VARIATION(4, 		L"Boundary - E_INVALIDARG - Invalid CLSCTX")
	TEST_VARIATION(5, 		L"Boundary - E_INVALIDARG - pwszInitString==NULL")
	TEST_VARIATION(6, 		L"Boundary - E_INVALIDARG - ppDataSource==NULL")
	TEST_VARIATION(7, 		L"Boundary - E_NOINTERFACE - IID_INULL")
	TEST_VARIATION(8, 		L"Boundary - E_NOINTERFACE - non-DataSource Object")
	TEST_VARIATION(9, 		L"Boundary - E_NOINTERFACE - Enumerator Object")
	TEST_VARIATION(10, 		L"Boundary - E_NOINTERFACE - IID_IDataInitialize")
	TEST_VARIATION(11, 		L"Parameters - pUnkOuter - Aggregation, not passing IID_IUnknown")
	TEST_VARIATION(12, 		L"Parameters - pUnkOuter - Aggregation, verify inner")
	TEST_VARIATION(13, 		L"Parameters - pUnkOuter - Aggregation, verify GetDataSource returns outer")
	TEST_VARIATION(14, 		L"Parameters - CLSCTX - CLSCTX_INPROC_SERVER")
	TEST_VARIATION(15, 		L"Parameters - CLSCTX - CLSCTX_INPROC_HANDLER")
	TEST_VARIATION(16, 		L"Parameters - CLSCTX - CLSCTX_SERVER")
	TEST_VARIATION(17, 		L"Parameters - CLSCTX - CLSCTX_ALL")
	TEST_VARIATION(18, 		L"Parameters - CLSCTX - CLSCTX_LOCAL_SERVER")
	TEST_VARIATION(19, 		L"Parameters - CLSCTX - CLSCTX_REMOTE_SERVER")
	TEST_VARIATION(20, 		L"Parameters - pwszInitString - Contains different ProgID")
	TEST_VARIATION(21, 		L"Parameters - pwszInitString - Contains different ProgID, with versioning")
	TEST_VARIATION(22, 		L"Parameters - pwszInitString - Contains same ProgID, but with versioning")
	TEST_VARIATION(23, 		L"Parameters - pwszInitString -  Enumerator Object")
	TEST_VARIATION(24, 		L"Parameters - riid - IID_IUnknown")
	TEST_VARIATION(25, 		L"Parameters - riid - verify all TDataSource interfaces")
	TEST_VARIATION(26, 		L"Parameters - riid - Provider Specific interfaces - S_OK")
	TEST_VARIATION(27, 		L"Parameters - ppDataSource - Uninitialized DSO - complete init string")
	TEST_VARIATION(28, 		L"Parameters - ppDataSource - Uninitialized DSO - init string with no props")
	TEST_VARIATION(29, 		L"Parameters - ppDataSource - Uninitialized DSO - init string with another provider")
	TEST_VARIATION(30, 		L"Parameters - ppDataSource - Uninitialized DSO - init string with no provider in the init string specified")
	TEST_VARIATION(31, 		L"Parameters - ppDataSource - Uninitialized DSO - init string with invalid ProgID")
	TEST_VARIATION(32, 		L"Parameters - ppDataSource - Uninitialized DSO - init string with invalid Provider String")
	TEST_VARIATION(33, 		L"Parameters - ppDataSource - Initailized DSO - Complete init string - DB_E_ERRORSOCCURRED")
	TEST_VARIATION(34, 		L"Parameters - ppDataSource - Initailized DSO - Complete init string with no props")
	TEST_VARIATION(35, 		L"Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an open session")
	TEST_VARIATION(36, 		L"Parameters - ppDataSource - Initailized DSO - Complete init string with no provider specified")
	TEST_VARIATION(37, 		L"Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an invalid ProgID")
	TEST_VARIATION(38, 		L"Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an invalid Provider String")
	TEST_VARIATION(39, 		L"Strings - Keywords - DBPROP_AUTH_CACHE_AUTHINFO")
	TEST_VARIATION(40, 		L"Strings - Keywords - DBPROP_AUTH_ENCRYPT_PASSWORD")
	TEST_VARIATION(41, 		L"Strings - Keywords - DBPROP_AUTH_INTEGRATED")
	TEST_VARIATION(42, 		L"Strings - Keywords - DBPROP_AUTH_MASK_PASSWORD")
	TEST_VARIATION(43, 		L"Strings - Keywords - DBPROP_AUTH_PASSWORD")
	TEST_VARIATION(44, 		L"Strings - Keywords - DBPROP_AUTH_PERSIST_ENCRYPTED")
	TEST_VARIATION(45, 		L"Strings - Keywords - DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO")
	TEST_VARIATION(46, 		L"Strings - Keywords - DBPROP_AUTH_USERID")
	TEST_VARIATION(47, 		L"Strings - Keywords - DBPROP_INIT_ASYNCH")
	TEST_VARIATION(48, 		L"Strings - Keywords - DBPROP_INIT_DATASOURCE")
	TEST_VARIATION(49, 		L"Strings - Keywords - DBPROP_INIT_HWND")
	TEST_VARIATION(50, 		L"Strings - Keywords - DBPROP_INIT_IMPERSONATION_LEVEL")
	TEST_VARIATION(51, 		L"Strings - Keywords - DBPROP_INIT_LCID")
	TEST_VARIATION(52, 		L"Strings - Keywords - DBPROP_INIT_LOCATION")
	TEST_VARIATION(53, 		L"Strings - Keywords - DBPROP_INIT_MODE")
	TEST_VARIATION(54, 		L"Strings - Keywords - DBPROP_INIT_PROMPT")
	TEST_VARIATION(55, 		L"Strings - Keywords - DBPROP_INIT_PROTECTION_LEVEL")
	TEST_VARIATION(56, 		L"Strings - Keywords - DBPROP_INIT_PROVIDERSTRING")
	TEST_VARIATION(57, 		L"Strings - Keywords - DBPROP_INIT_TIMEOUT")
	TEST_VARIATION(58, 		L"Strings - Keywords - DBPROP_INIT_GENERALTIMEOUT")
	TEST_VARIATION(59, 		L"Strings - Keywords - All Properties (including Provider Specific) - together")
	TEST_VARIATION(60, 		L"Strings - Keywords - All Properties (including Provider Specific) - seperate")
	TEST_VARIATION(61, 		L"Strings - Keywords - Property specified more than once with diff values")
	TEST_VARIATION(62, 		L"Strings - Keywords - Verify Case Insensitive")
	TEST_VARIATION(63, 		L"Strings - Keywords - Specified more than once, second in error")
	TEST_VARIATION(64, 		L"Strings - Provider Keyword - Just Provider Keyword")
	TEST_VARIATION(65, 		L"Strings - Provider Keyword - Not first item in the list")
	TEST_VARIATION(66, 		L"Strings - Provider Keyword - Versioned Provider ProgID")
	TEST_VARIATION(67, 		L"Strings - Provider Keyword - Provider ProgID which contains a space")
	TEST_VARIATION(68, 		L"Strings - Provider Keyword - Missing")
	TEST_VARIATION(69, 		L"Strings - Provider Keyword - Versioned Provider ProgID that doesn't exist, should fall back")
	TEST_VARIATION(70, 		L"Strings - Provider Keyword - Specified twice")
	TEST_VARIATION(71, 		L"Strings - Provider Keyword - Provider=; - error")
	TEST_VARIATION(72, 		L"Strings - Friendly Names - Allow integer equivilent")
	TEST_VARIATION(73, 		L"Strings - Friendly Names - Numeric Value out of range")
	TEST_VARIATION(74, 		L"Strings - Friendly Names - oring Friendly Names")
	TEST_VARIATION(75, 		L"Strings - Friendly Names - oring same Friendly Name")
	TEST_VARIATION(76, 		L"Strings - Friendly Names - wrong Friendly Names")
	TEST_VARIATION(77, 		L"Strings - Friendly Names - oring values for nonMode props - error")
	TEST_VARIATION(78, 		L"Strings - Property Values - Contains same value as keyword")
	TEST_VARIATION(79, 		L"Strings - Reserved SemiColon - Last item doesn't contain semicolon")
	TEST_VARIATION(80, 		L"Strings - Reserved SemiColon - Provider=progid with no trailing semi")
	TEST_VARIATION(81, 		L"Strings - Reserved SemiColon - Double semi")
	TEST_VARIATION(82, 		L"Strings - Reserved SemiColon - Semicolon in keyword")
	TEST_VARIATION(83, 		L"Strings - Reserved SemiColon - Semicolon in Value - (quoted)")
	TEST_VARIATION(84, 		L"Strings - Reserved SemiColon - Semicolon following keyword Data Source =;")
	TEST_VARIATION(85, 		L"Strings - Reserved SemiColon - Semicolon following keyword Extended Properties;")
	TEST_VARIATION(86, 		L"Strings - Reserved SemiColon - Middle item missing semi - error")
	TEST_VARIATION(87, 		L"Strings - Reserved SemiColon - SemiColon in value - not qouted - error")
	TEST_VARIATION(88, 		L"Strings - Reserved EqualSign - In Keyword - Doubled")
	TEST_VARIATION(89, 		L"Strings - Reserved EqualSign - In Value - not Doubled")
	TEST_VARIATION(90, 		L"Strings - Reserved EqualSign - In Value - Doubled - should count as 2")
	TEST_VARIATION(91, 		L"Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - In Keyword - not doubled - error")
	TEST_VARIATION(92, 		L"Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - Completly missing - error")
	TEST_VARIATION(93, 		L"Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - Keyword==value; - error")
	TEST_VARIATION(94, 		L"Strings - Spaces - Before/After keywords")
	TEST_VARIATION(95, 		L"Strings - Spaces - Before/After equal sign")
	TEST_VARIATION(96, 		L"Strings - Spaces - Before/After semi")
	TEST_VARIATION(97, 		L"Strings - Spaces - Before/After value- ignored")
	TEST_VARIATION(98, 		L"Strings - Spaces - Before/After value - quoted - retained")
	TEST_VARIATION(99, 		L"Strings - Spaces - in value - retained")
	TEST_VARIATION(100, 		L"Strings - Spaces - Before string, After string")
	TEST_VARIATION(101, 		L"Strings - Spaces - Before/After orable token (|)")
	TEST_VARIATION(102, 		L"Strings - Spaces - In Keyword that already has spaces")
	TEST_VARIATION(103, 		L"Strings - Quotes - All value single quotes")
	TEST_VARIATION(104, 		L"Strings - Quotes - All value double quotes")
	TEST_VARIATION(105, 		L"Strings - Quotes - Keywords single and Double quoted")
	TEST_VARIATION(106, 		L"Strings - Quotes - Embedding Single")
	TEST_VARIATION(107, 		L"Strings - Quotes - Embedding Double")
	TEST_VARIATION(108, 		L"Strings - Quotes - Both Embedding Single and Double starting with Single")
	TEST_VARIATION(109, 		L"Strings - Quotes - Both Embedding Single and Double starting with Double")
	TEST_VARIATION(110, 		L"Strings - Quotes - Just a pain!")
	TEST_VARIATION(111, 		L"Strings - Quotes - Uneven quotes")
	TEST_VARIATION(112, 		L"Strings - Quotes - Doubled Quotes")
	TEST_VARIATION(113, 		L"Strings - Quotes - Tripled Quotes")
	TEST_VARIATION(114, 		L"Strings - Special Chars - DBCS")
	TEST_VARIATION(115, 		L"Strings - Special Chars - All Printable Chars")
	TEST_VARIATION(116, 		L"Strings - Special Chars - non-printable")
	TEST_VARIATION(117, 		L"Strings - Special Chars - Localized Property Descriptions")
	TEST_VARIATION(118, 		L"MultiUser - GetDataSource from 3 threads - each asking for all diff interfaces, all dealing with the same provider")
	TEST_VARIATION(119, 		L"MultiUser - GetDataSource from 3 threads - each asking for all diff interfaces, all same provider, with an error")
	TEST_VARIATION(120, 		L"Keywords - DBPROP_INIT_CATALOG")
	TEST_VARIATION(121, 		L"Keywords - DBPROP_INIT_OLEDBSERVICES")
	TEST_VARIATION(122, 		L"Parameters - Provider hidden properties set before")
	TEST_VARIATION(123, 		L"Parameters - Provider hidden properties set after")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCGetInitializationString)
//*-----------------------------------------------------------------------
// @class Test IDataInitialize::GetInitializationString
//
class TCGetInitializationString : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCGetInitializationString,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - Verify password format
	int Variation_1();
	// @cmember Boundary - E_INVALIDARG - pDataSource == NULL
	int Variation_2();
	// @cmember Boundary - E_INVALIDARG - ppwszInitString == NULL
	int Variation_3();
	// @cmember Boundary - E_NOINTERFACE - pDataSource = non-datasource interface (ie: Session object)
	int Variation_4();
	// @cmember Boundary - E_NOINTERFACE - pDataSource = Enumerator Object
	int Variation_5();
	// @cmember Boundary - S_OK - pDataSource being different TDataSource interfaces
	int Variation_6();
	// @cmember Boundary - S_OK - fIncludePassword == 2 - assumming to be true
	int Variation_7();
	// @cmember Sequence - CreateDBInstance, GetInitString, GetDataSource, verify uninitialized and no props sets
	int Variation_8();
	// @cmember Sequence - CreateDBInstance, Set required properties, GetInitString, GetDataSource
	int Variation_9();
	// @cmember Sequence - PasswordSaved - GetInitString on Initialized DSO, GetDataSource, Initialize
	int Variation_10();
	// @cmember Sequence - PasswordSaved - GetInitString on Uninitialized DSO (with no password set), GetDataSource, Initialize
	int Variation_11();
	// @cmember Sequence - PasswordSaved - GetInitString on Uninitialized DSO (with password set), GetDataSource, Initialize
	int Variation_12();
	// @cmember Sequence - PasswordNotSaved - GetInitString on Initialized DSO, GetDataSource, Initialize
	int Variation_13();
	// @cmember Sequence - PasswordNotSaved - GetInitString on Initialized DSO, GetDataSource, Set Password, Initialize
	int Variation_14();
	// @cmember Sequence - PasswordNotSaved - GetInitString on Uninitialized DSO (with no password), GetDataSource, Initialize
	int Variation_15();
	// @cmember Sequence - PasswordNotSaved - GetInitString on Uninitialized DSO (with password), GetDataSource, Initialize
	int Variation_16();
	// @cmember Sequence - GetInitString with SENSITIVE_AUTHINFO = TRUE
	int Variation_17();
	// @cmember Sequence - GetInitString with SENSITIVE_AUTHINFO = FALSE
	int Variation_18();
	// @cmember Properties - REQUIRED Init Properties
	int Variation_19();
	// @cmember Properties - Provider Specific Init Properties
	int Variation_20();
	// @cmember Properties - OPTIONAL Init Properties
	int Variation_21();
	// @cmember Properties - COLID Properties
	int Variation_22();
	// @cmember Properties - Special Chars - DBCS
	int Variation_23();
	// @cmember MultiUser - GetInitString from 3 threads, all having the same DataSource pointer
	int Variation_24();
	// @cmember MultiUser - GetInitString from 3 threads, all having different DataSource pointers
	int Variation_25();
	// @cmember ODBC - Interesting Backward Compatible Strings
	int Variation_26();
	// @cmember ODBC - PASSWORD_INCLUDED - Verify Password Saved
	int Variation_27();
	// @cmember ODBC - PASSWORD_INCLUDED - Verify PWD parsed out
	int Variation_28();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCGetInitializationString)
#define THE_CLASS TCGetInitializationString
BEG_TEST_CASE(TCGetInitializationString, CDBInit, L"Test IDataInitialize::GetInitializationString")
	TEST_VARIATION(1, 		L"General - Verify password format")
	TEST_VARIATION(2, 		L"Boundary - E_INVALIDARG - pDataSource == NULL")
	TEST_VARIATION(3, 		L"Boundary - E_INVALIDARG - ppwszInitString == NULL")
	TEST_VARIATION(4, 		L"Boundary - E_NOINTERFACE - pDataSource = non-datasource interface (ie: Session object)")
	TEST_VARIATION(5, 		L"Boundary - E_NOINTERFACE - pDataSource = Enumerator Object")
	TEST_VARIATION(6, 		L"Boundary - S_OK - pDataSource being different TDataSource interfaces")
	TEST_VARIATION(7, 		L"Boundary - S_OK - fIncludePassword == 2 - assumming to be true")
	TEST_VARIATION(8, 		L"Sequence - CreateDBInstance, GetInitString, GetDataSource, verify uninitialized and no props sets")
	TEST_VARIATION(9, 		L"Sequence - CreateDBInstance, Set required properties, GetInitString, GetDataSource")
	TEST_VARIATION(10, 		L"Sequence - PasswordSaved - GetInitString on Initialized DSO, GetDataSource, Initialize")
	TEST_VARIATION(11, 		L"Sequence - PasswordSaved - GetInitString on Uninitialized DSO (with no password set), GetDataSource, Initialize")
	TEST_VARIATION(12, 		L"Sequence - PasswordSaved - GetInitString on Uninitialized DSO (with password set), GetDataSource, Initialize")
	TEST_VARIATION(13, 		L"Sequence - PasswordNotSaved - GetInitString on Initialized DSO, GetDataSource, Initialize")
	TEST_VARIATION(14, 		L"Sequence - PasswordNotSaved - GetInitString on Initialized DSO, GetDataSource, Set Password, Initialize")
	TEST_VARIATION(15, 		L"Sequence - PasswordNotSaved - GetInitString on Uninitialized DSO (with no password), GetDataSource, Initialize")
	TEST_VARIATION(16, 		L"Sequence - PasswordNotSaved - GetInitString on Uninitialized DSO (with password), GetDataSource, Initialize")
	TEST_VARIATION(17, 		L"Sequence - GetInitString with SENSITIVE_AUTHINFO = TRUE")
	TEST_VARIATION(18, 		L"Sequence - GetInitString with SENSITIVE_AUTHINFO = FALSE")
	TEST_VARIATION(19, 		L"Properties - REQUIRED Init Properties")
	TEST_VARIATION(20, 		L"Properties - Provider Specific Init Properties")
	TEST_VARIATION(21, 		L"Properties - OPTIONAL Init Properties")
	TEST_VARIATION(22, 		L"Properties - COLID Properties")
	TEST_VARIATION(23, 		L"Properties - Special Chars - DBCS")
	TEST_VARIATION(24, 		L"MultiUser - GetInitString from 3 threads, all having the same DataSource pointer")
	TEST_VARIATION(25, 		L"MultiUser - GetInitString from 3 threads, all having different DataSource pointers")
	TEST_VARIATION(26, 		L"ODBC - Interesting Backward Compatible Strings")
	TEST_VARIATION(27, 		L"ODBC - PASSWORD_INCLUDED - Verify Password Saved")
	TEST_VARIATION(28, 		L"ODBC - PASSWORD_INCLUDED - Verify PWD parsed out")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCLoadStringFromStorage)
//*-----------------------------------------------------------------------
// @class Test IDataInitialize::LoadStringFromStorage
//
class TCLoadStringFromStorage : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCLoadStringFromStorage,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Boundary - E_INVALIDARG - pwszFileName - NULL
	int Variation_1();
	// @cmember Boundary - E_INVALIDARG - ppwszInitString - NULL
	int Variation_2();
	// @cmember Boundary - STG_E_PATHNOTFOUND - Valid formed filename, (just non-existent on the disk)
	int Variation_3();
	// @cmember Boundary - STG_E_FILENOTFOUND - Valid formed filename, (but no access writes)
	int Variation_4();
	// @cmember Boundary - STG_E_FILENOTFOUND - Valid formed Directory, (but non existent directory)
	int Variation_5();
	// @cmember Boundary - STG_E_FILENOTFOUND [Invalid-Name] - Invalid filename
	int Variation_6();
	// @cmember Boundary - STG_E_INVALIDNAME [Invalid-Name] - Invalid file path
	int Variation_7();
	// @cmember Boundary - STG_E_PATHNOTFOUND [Invalid-Name] - just directory location
	int Variation_8();
	// @cmember Boundary - STG_E_FILENOTFOUND [Invalid-Name] - Empty String
	int Variation_9();
	// @cmember Boundary - STG_E_INVALIDNAME [Invalid-Name] - Wildcards
	int Variation_10();
	// @cmember Boundary - STG_E_ACCESSDENIED - ReadOnly Directory
	int Variation_11();
	// @cmember Boundary - STG_E_SHAREVIOLATION - File is locked
	int Variation_12();
	// @cmember Boundary - STG_E_INVALIDHEADER - Invalid File Format
	int Variation_13();
	// @cmember Boundary - STG_E_INVALIDHEADER - Empty File
	int Variation_14();
	// @cmember WinLogo - S_OK - Fake 8.3
	int Variation_15();
	// @cmember WinLogo - S_OK - Long FileName > 255 characters
	int Variation_16();
	// @cmember WinLogo - S_OK - Huge FileName > Stack Size
	int Variation_17();
	// @cmember WinLogo - S_OK - Read Only File
	int Variation_18();
	// @cmember WinLogo - S_OK - UNC Format \\server\\dir\\file.dot
	int Variation_19();
	// @cmember WinLogo - S_OK - Http format
	int Variation_20();
	// @cmember WinLogo - S_OK - Drive:\\Dir\\File.dot
	int Variation_21();
	// @cmember WinLogo - S_OK - Special Chars - DBCS
	int Variation_22();
	// @cmember WinLogo - S_OK - Special Chars - OEM
	int Variation_23();
	// @cmember WinLogo - S_OK - Special Chars, plus signs, commas, brackets, etc.
	int Variation_24();
	// @cmember WinLogo - S_OK - Not save leading and trailing spaces
	int Variation_25();
	// @cmember WinLogo - S_OK - Save embedded spaces
	int Variation_26();
	// @cmember WinLogo - S_OK - Must add an extension if not specified
	int Variation_27();
	// @cmember WinLogo - S_OK - Doesn't add an extension if already exists
	int Variation_28();
	// @cmember WinLogo - S_OK - Not save question marks
	int Variation_29();
	// @cmember WinLogo - S_OK - Valid Characters one at a time
	int Variation_30();
	// @cmember WinLogo - STG_E_INVALIDNAME - Invalid Characters one at a time
	int Variation_31();
	// @cmember Empty
	int Variation_32();
	// @cmember UDL Cache - Fill up UDL File Cache
	int Variation_33();
	// @cmember UDL Cache - Interleaved UDL Requests
	int Variation_34();
	// @cmember UDL Cache - Random UDL Requests
	int Variation_35();
	// @cmember UDL Cache - MultiThreaded Load from Cache
	int Variation_36();
	// @cmember Empty
	int Variation_37();
	// @cmember MultiUser - LoadStringFromStorage from 3 threads, all from the same file
	int Variation_38();
	// @cmember MultiUser - LoadStringFromStorage from 3 threads, all having different files
	int Variation_39();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCLoadStringFromStorage)
#define THE_CLASS TCLoadStringFromStorage
BEG_TEST_CASE(TCLoadStringFromStorage, CDBInit, L"Test IDataInitialize::LoadStringFromStorage")
	TEST_VARIATION(1, 		L"Boundary - E_INVALIDARG - pwszFileName - NULL")
	TEST_VARIATION(2, 		L"Boundary - E_INVALIDARG - ppwszInitString - NULL")
	TEST_VARIATION(3, 		L"Boundary - STG_E_PATHNOTFOUND - Valid formed filename, (just non-existent on the disk)")
	TEST_VARIATION(4, 		L"Boundary - STG_E_FILENOTFOUND - Valid formed filename, (but no access writes)")
	TEST_VARIATION(5, 		L"Boundary - STG_E_FILENOTFOUND - Valid formed Directory, (but non existent directory)")
	TEST_VARIATION(6, 		L"Boundary - STG_E_FILENOTFOUND [Invalid-Name] - Invalid filename")
	TEST_VARIATION(7, 		L"Boundary - STG_E_INVALIDNAME [Invalid-Name] - Invalid file path")
	TEST_VARIATION(8, 		L"Boundary - STG_E_PATHNOTFOUND [Invalid-Name] - just directory location")
	TEST_VARIATION(9, 		L"Boundary - STG_E_FILENOTFOUND [Invalid-Name] - Empty String")
	TEST_VARIATION(10, 		L"Boundary - STG_E_INVALIDNAME [Invalid-Name] - Wildcards")
	TEST_VARIATION(11, 		L"Boundary - STG_E_ACCESSDENIED - ReadOnly Directory")
	TEST_VARIATION(12, 		L"Boundary - STG_E_SHAREVIOLATION - File is locked")
	TEST_VARIATION(13, 		L"Boundary - STG_E_INVALIDHEADER - Invalid File Format")
	TEST_VARIATION(14, 		L"Boundary - STG_E_INVALIDHEADER - Empty File")
	TEST_VARIATION(15, 		L"WinLogo - S_OK - Fake 8.3")
	TEST_VARIATION(16, 		L"WinLogo - S_OK - Long FileName > 255 characters")
	TEST_VARIATION(17, 		L"WinLogo - S_OK - Huge FileName > Stack Size")
	TEST_VARIATION(18, 		L"WinLogo - S_OK - Read Only File")
	TEST_VARIATION(19, 		L"WinLogo - S_OK - UNC Format \\server\\dir\\file.dot")
	TEST_VARIATION(20, 		L"WinLogo - S_OK - Http format")
	TEST_VARIATION(21, 		L"WinLogo - S_OK - Drive:\\Dir\\File.dot")
	TEST_VARIATION(22, 		L"WinLogo - S_OK - Special Chars - DBCS")
	TEST_VARIATION(23, 		L"WinLogo - S_OK - Special Chars - OEM")
	TEST_VARIATION(24, 		L"WinLogo - S_OK - Special Chars, plus signs, commas, brackets, etc.")
	TEST_VARIATION(25, 		L"WinLogo - S_OK - Not save leading and trailing spaces")
	TEST_VARIATION(26, 		L"WinLogo - S_OK - Save embedded spaces")
	TEST_VARIATION(27, 		L"WinLogo - S_OK - Must add an extension if not specified")
	TEST_VARIATION(28, 		L"WinLogo - S_OK - Doesn't add an extension if already exists")
	TEST_VARIATION(29, 		L"WinLogo - S_OK - Not save question marks")
	TEST_VARIATION(30, 		L"WinLogo - S_OK - Valid Characters one at a time")
	TEST_VARIATION(31, 		L"WinLogo - STG_E_INVALIDNAME - Invalid Characters one at a time")
	TEST_VARIATION(32, 		L"Empty")
	TEST_VARIATION(33, 		L"UDL Cache - Fill up UDL File Cache")
	TEST_VARIATION(34, 		L"UDL Cache - Interleaved UDL Requests")
	TEST_VARIATION(35, 		L"UDL Cache - Random UDL Requests")
	TEST_VARIATION(36, 		L"UDL Cache - MultiThreaded Load from Cache")
	TEST_VARIATION(37, 		L"Empty")
	TEST_VARIATION(38, 		L"MultiUser - LoadStringFromStorage from 3 threads, all from the same file")
	TEST_VARIATION(39, 		L"MultiUser - LoadStringFromStorage from 3 threads, all having different files")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCWriteStringToStorage)
//*-----------------------------------------------------------------------
// @class Test IDataInitialize::WriteStringToStorage
//
class TCWriteStringToStorage : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCWriteStringToStorage,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Boundary - E_INVALIDARG - pwszFileName = NULL
	int Variation_1();
	// @cmember Boundary - E_INVALIDARG - pwszInitString = NULL
	int Variation_2();
	// @cmember Boundary - STG_E_FILEALREADYEXISTS - dwCreate = CREATE_NEW
	int Variation_3();
	// @cmember Boundary - STG_E_FILENOTFOUND - dwCreate = OPEN_EXISTING
	int Variation_4();
	// @cmember Boundary - STG_E_FILENOTFOUND - dwCreate = TRUNCATE_EXISTING
	int Variation_5();
	// @cmember Boundary - STG_E_FILENOTFOUND - just directory name
	int Variation_6();
	// @cmember Boundary - STG_E_SHAREVIOLATION - File is locked
	int Variation_7();
	// @cmember Boundary - STG_E_ACCESSDENIED - ReadOnly Directory
	int Variation_8();
	// @cmember Boundary - STG_E_INVALIDNAME [Invalid-Name] - ill-formed filename
	int Variation_9();
	// @cmember Boundary - STG_E_INVALIDNAME [Invalid-Name] - ill-formed directory name
	int Variation_10();
	// @cmember Boundary - STG_E_PATHNOTFOUND [Invalid-Name] - just directory location
	int Variation_11();
	// @cmember Boundary - E_INVALIDARG - dwCreateDisposition = invalid
	int Variation_12();
	// @cmember Parameters - dwCreateDisposition - CREATE_NEW
	int Variation_13();
	// @cmember Parameters - dwCreateDisposition - CREATE_ALWAYS
	int Variation_14();
	// @cmember Parameters - dwCreateDisposition - CREATE_ALWAYS - previsouly existing
	int Variation_15();
	// @cmember Parameters - dwCreateDisposition - OPEN_ALWAYS - not existing
	int Variation_16();
	// @cmember Parameters - dwCreateDisposition - OPEN_ALWAYS - existing
	int Variation_17();
	// @cmember Parameters - dwCreateDisposition - OPEN_EXISTING - existing
	int Variation_18();
	// @cmember Parameters - dwCreateDisposition - TRUNCATE_EXISTING - existing
	int Variation_19();
	// @cmember Sequence - WriteString(), LoadString, GetDataSource
	int Variation_20();
	// @cmember Sequence - WriteString(some garbage), LoadString, GetDataSource
	int Variation_21();
	// @cmember Sequence - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource
	int Variation_22();
	// @cmember Sequence - CreateDBInstance, Set Required Init Properties, GetInitString, WriteString, LoadString, GetDataSource
	int Variation_23();
	// @cmember Sequence - PasswordSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, Initialize
	int Variation_24();
	// @cmember Sequence - PasswordSaved - GetInitString on UnInit DSO (no password set), WriteString, LoadString, GetDataSource, Initialize
	int Variation_25();
	// @cmember Sequence - PasswordSaved - GetInitString on UnInit DSO (with password set), WriteString, LoadString, GetDataSource, Initialize
	int Variation_26();
	// @cmember Sequence - PasswordNotSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, Initialize
	int Variation_27();
	// @cmember Sequence - PasswordNotSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, SetPassword, Initialize
	int Variation_28();
	// @cmember Sequence - PasswordNotSaved - GetInitString on UnInit DSO (no password set), WriteString, LoadString, GetDataSource, Initialize
	int Variation_29();
	// @cmember Sequence - PasswordNotSaved - GetInitString on UnInit DSO (password set), WriteString, LoadString, GetDataSource, Initialize
	int Variation_30();
	// @cmember Stress - Large InitString > 255 characters
	int Variation_31();
	// @cmember Stress - InitString - Special Chars - DBCS
	int Variation_32();
	// @cmember MultiUser - WriteStringToStorage from 3 threads
	int Variation_33();
	// @cmember MultiUser - WriteStringToStorage from 3 threads, all having different files
	int Variation_34();
	// @cmember MultiUser - WriteStringToStorage while LoadStringFromStorage
	int Variation_35();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCWriteStringToStorage)
#define THE_CLASS TCWriteStringToStorage
BEG_TEST_CASE(TCWriteStringToStorage, CDBInit, L"Test IDataInitialize::WriteStringToStorage")
	TEST_VARIATION(1, 		L"Boundary - E_INVALIDARG - pwszFileName = NULL")
	TEST_VARIATION(2, 		L"Boundary - E_INVALIDARG - pwszInitString = NULL")
	TEST_VARIATION(3, 		L"Boundary - STG_E_FILEALREADYEXISTS - dwCreate = CREATE_NEW")
	TEST_VARIATION(4, 		L"Boundary - STG_E_FILENOTFOUND - dwCreate = OPEN_EXISTING")
	TEST_VARIATION(5, 		L"Boundary - STG_E_FILENOTFOUND - dwCreate = TRUNCATE_EXISTING")
	TEST_VARIATION(6, 		L"Boundary - STG_E_FILENOTFOUND - just directory name")
	TEST_VARIATION(7, 		L"Boundary - STG_E_SHAREVIOLATION - File is locked")
	TEST_VARIATION(8, 		L"Boundary - STG_E_ACCESSDENIED - ReadOnly Directory")
	TEST_VARIATION(9, 		L"Boundary - STG_E_INVALIDNAME [Invalid-Name] - ill-formed filename")
	TEST_VARIATION(10, 		L"Boundary - STG_E_INVALIDNAME [Invalid-Name] - ill-formed directory name")
	TEST_VARIATION(11, 		L"Boundary - STG_E_PATHNOTFOUND [Invalid-Name] - just directory location")
	TEST_VARIATION(12, 		L"Boundary - E_INVALIDARG - dwCreateDisposition = invalid")
	TEST_VARIATION(13, 		L"Parameters - dwCreateDisposition - CREATE_NEW")
	TEST_VARIATION(14, 		L"Parameters - dwCreateDisposition - CREATE_ALWAYS")
	TEST_VARIATION(15, 		L"Parameters - dwCreateDisposition - CREATE_ALWAYS - previsouly existing")
	TEST_VARIATION(16, 		L"Parameters - dwCreateDisposition - OPEN_ALWAYS - not existing")
	TEST_VARIATION(17, 		L"Parameters - dwCreateDisposition - OPEN_ALWAYS - existing")
	TEST_VARIATION(18, 		L"Parameters - dwCreateDisposition - OPEN_EXISTING - existing")
	TEST_VARIATION(19, 		L"Parameters - dwCreateDisposition - TRUNCATE_EXISTING - existing")
	TEST_VARIATION(20, 		L"Sequence - WriteString(), LoadString, GetDataSource")
	TEST_VARIATION(21, 		L"Sequence - WriteString(some garbage), LoadString, GetDataSource")
	TEST_VARIATION(22, 		L"Sequence - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource")
	TEST_VARIATION(23, 		L"Sequence - CreateDBInstance, Set Required Init Properties, GetInitString, WriteString, LoadString, GetDataSource")
	TEST_VARIATION(24, 		L"Sequence - PasswordSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, Initialize")
	TEST_VARIATION(25, 		L"Sequence - PasswordSaved - GetInitString on UnInit DSO (no password set), WriteString, LoadString, GetDataSource, Initialize")
	TEST_VARIATION(26, 		L"Sequence - PasswordSaved - GetInitString on UnInit DSO (with password set), WriteString, LoadString, GetDataSource, Initialize")
	TEST_VARIATION(27, 		L"Sequence - PasswordNotSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, Initialize")
	TEST_VARIATION(28, 		L"Sequence - PasswordNotSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, SetPassword, Initialize")
	TEST_VARIATION(29, 		L"Sequence - PasswordNotSaved - GetInitString on UnInit DSO (no password set), WriteString, LoadString, GetDataSource, Initialize")
	TEST_VARIATION(30, 		L"Sequence - PasswordNotSaved - GetInitString on UnInit DSO (password set), WriteString, LoadString, GetDataSource, Initialize")
	TEST_VARIATION(31, 		L"Stress - Large InitString > 255 characters")
	TEST_VARIATION(32, 		L"Stress - InitString - Special Chars - DBCS")
	TEST_VARIATION(33, 		L"MultiUser - WriteStringToStorage from 3 threads")
	TEST_VARIATION(34, 		L"MultiUser - WriteStringToStorage from 3 threads, all having different files")
	TEST_VARIATION(35, 		L"MultiUser - WriteStringToStorage while LoadStringFromStorage")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCPromptDataSource)
//*-----------------------------------------------------------------------
// @class Test IDBPromptInitialize::PromptDataSource
//
class TCPromptDataSource : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPromptDataSource,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember General - IClassFactory - CoGetClassObject (all valid riids) - S_OK
	int Variation_1();
	// @cmember General - IClassFactory - CoGetClassObject (all invalid riids) - E_NOINTERFACE
	int Variation_2();
	// @cmember General - IClassFactory - CoGetClassObject (NULL) - E_INVALIDARG
	int Variation_3();
	// @cmember General - IClassFactory - CoCreateInstance (all valid riids) - S_OK
	int Variation_4();
	// @cmember General - IClassFactory - CoCreateInstance (all invalid riids) - E_NOINTERFACE
	int Variation_5();
	// @cmember General - IClassFactory - CoCreateInstance (NULL) - E_INVALIDARG
	int Variation_6();
	// @cmember General - IClassFactory - LockServer combinations
	int Variation_7();
	// @cmember General - IClassFactory - QI combinations
	int Variation_8();
	// @cmember General - IClassFactory - AddRef/Release combinations
	int Variation_9();
	// @cmember General - CoCreateInstance - Aggregation - non-IUnknown
	int Variation_10();
	// @cmember General - CoCreateInstance - Aggregation - all valid combinations
	int Variation_11();
	// @cmember General - IUnknown - QI(valid riids) S_OK
	int Variation_12();
	// @cmember General - IUnknown - QI(in valid riids) E_NOINTERFACE
	int Variation_13();
	// @cmember General - IUnknown - QI(NULL) E_INVALIDARG
	int Variation_14();
	// @cmember General - IUnknown - AddRef/Release combinations
	int Variation_15();
	// @cmember Boundary - E_INVALIDARG - hWnd = NULL / Invalid
	int Variation_16();
	// @cmember Boundary -DBPROMPTOPTIONS_
	int Variation_17();
	// @cmember Boundary - DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION
	int Variation_18();
	// @cmember Boundary - E_INVALIDARG - cSourceType != 0 and rgSourceType == NULL
	int Variation_19();
	// @cmember Boundary - E_INVALIDARG - cSourceType == 0 and rgSourceType != NULL
	int Variation_20();
	// @cmember Boundary - E_INVALIDARG - Singularly null terminated string
	int Variation_21();
	// @cmember Boundary - E_INVALIDARG - ppDataSource = NULL
	int Variation_22();
	// @cmember Boundary - E_NOINTERFACE - riid = IID_INULL
	int Variation_23();
	// @cmember Boundary - E_NOINTERFACE - riid = IID_IDataInitalize
	int Variation_24();
	// @cmember Parameters - rgSourceType = contains both valid and invalid DBSOURCETYPE values
	int Variation_25();
	// @cmember Parameters - pwszProviderFilter - Contains both valid and invalid progIDs
	int Variation_26();
	// @cmember Parameters - pwszProviderFilter - Contains one large invalid progID
	int Variation_27();
	// @cmember Parameters - ppDataSource - verify untouched after numerous error conditions
	int Variation_28();
	// @cmember Parameters - ppDataSource - contains enumerator ojbect
	int Variation_29();
	// @cmember Parameters - ppDataSource - contains non-DataSource Object
	int Variation_30();
	// @cmember Aggregation - Verify DCM is aggregated
	int Variation_31();
	// @cmember Aggregation - Verify DCM is aggregated with Aggregated DataSource
	int Variation_32();
	// @cmember Aggregation - asking for non-IUnknown
	int Variation_33();
	// @cmember Aggregation - Verify inner and outer
	int Variation_34();
	// @cmember Stress - Verify FreeThreaded
	int Variation_35();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCPromptDataSource)
#define THE_CLASS TCPromptDataSource
BEG_TEST_CASE(TCPromptDataSource, CDBInit, L"Test IDBPromptInitialize::PromptDataSource")
	TEST_VARIATION(1, 		L"General - IClassFactory - CoGetClassObject (all valid riids) - S_OK")
	TEST_VARIATION(2, 		L"General - IClassFactory - CoGetClassObject (all invalid riids) - E_NOINTERFACE")
	TEST_VARIATION(3, 		L"General - IClassFactory - CoGetClassObject (NULL) - E_INVALIDARG")
	TEST_VARIATION(4, 		L"General - IClassFactory - CoCreateInstance (all valid riids) - S_OK")
	TEST_VARIATION(5, 		L"General - IClassFactory - CoCreateInstance (all invalid riids) - E_NOINTERFACE")
	TEST_VARIATION(6, 		L"General - IClassFactory - CoCreateInstance (NULL) - E_INVALIDARG")
	TEST_VARIATION(7, 		L"General - IClassFactory - LockServer combinations")
	TEST_VARIATION(8, 		L"General - IClassFactory - QI combinations")
	TEST_VARIATION(9, 		L"General - IClassFactory - AddRef/Release combinations")
	TEST_VARIATION(10, 		L"General - CoCreateInstance - Aggregation - non-IUnknown")
	TEST_VARIATION(11, 		L"General - CoCreateInstance - Aggregation - all valid combinations")
	TEST_VARIATION(12, 		L"General - IUnknown - QI(valid riids) S_OK")
	TEST_VARIATION(13, 		L"General - IUnknown - QI(in valid riids) E_NOINTERFACE")
	TEST_VARIATION(14, 		L"General - IUnknown - QI(NULL) E_INVALIDARG")
	TEST_VARIATION(15, 		L"General - IUnknown - AddRef/Release combinations")
	TEST_VARIATION(16, 		L"Boundary - E_INVALIDARG - hWnd = NULL / Invalid")
	TEST_VARIATION(17, 		L"Boundary -DBPROMPTOPTIONS_")
	TEST_VARIATION(18, 		L"Boundary - DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION")
	TEST_VARIATION(19, 		L"Boundary - E_INVALIDARG - cSourceType != 0 and rgSourceType == NULL")
	TEST_VARIATION(20, 		L"Boundary - E_INVALIDARG - cSourceType == 0 and rgSourceType != NULL")
	TEST_VARIATION(21, 		L"Boundary - E_INVALIDARG - Singularly null terminated string")
	TEST_VARIATION(22, 		L"Boundary - E_INVALIDARG - ppDataSource = NULL")
	TEST_VARIATION(23, 		L"Boundary - E_NOINTERFACE - riid = IID_INULL")
	TEST_VARIATION(24, 		L"Boundary - E_NOINTERFACE - riid = IID_IDataInitalize")
	TEST_VARIATION(25, 		L"Parameters - rgSourceType = contains both valid and invalid DBSOURCETYPE values")
	TEST_VARIATION(26, 		L"Parameters - pwszProviderFilter - Contains both valid and invalid progIDs")
	TEST_VARIATION(27, 		L"Parameters - pwszProviderFilter - Contains one large invalid progID")
	TEST_VARIATION(28, 		L"Parameters - ppDataSource - verify untouched after numerous error conditions")
	TEST_VARIATION(29, 		L"Parameters - ppDataSource - contains enumerator ojbect")
	TEST_VARIATION(30, 		L"Parameters - ppDataSource - contains non-DataSource Object")
	TEST_VARIATION(31, 		L"Aggregation - Verify DCM is aggregated")
	TEST_VARIATION(32, 		L"Aggregation - Verify DCM is aggregated with Aggregated DataSource")
	TEST_VARIATION(33, 		L"Aggregation - asking for non-IUnknown")
	TEST_VARIATION(34, 		L"Aggregation - Verify inner and outer")
	TEST_VARIATION(35, 		L"Stress - Verify FreeThreaded")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCPromptFileName)
//*-----------------------------------------------------------------------
// @class Test IDBPromptInitialize::PromptFileName
//
class TCPromptFileName : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPromptFileName,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember Boundary - E_INVALIDARG - hWnd == NULL
	int Variation_1();
	// @cmember Boundary - E_INVALIDARG - hWnd == invalid
	int Variation_2();
	// @cmember Boundary - E_INVALIDARG - dwPromptOptions
	int Variation_3();
	// @cmember Boundary - E_INVALIDARG - ppwszSelectedFile == NULL
	int Variation_4();
	// @cmember Parameters - pwszInitDirectory - non existent
	int Variation_5();
	// @cmember Parameters - pwszInitDirectory - invalid
	int Variation_6();
	// @cmember Parameters - pwszInitFile - non existent
	int Variation_7();
	// @cmember Parameters - pwszInitFile - invalid
	int Variation_8();
	// @cmember Stress - Verify FreeThreaded
	int Variation_9();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCPromptFileName)
#define THE_CLASS TCPromptFileName
BEG_TEST_CASE(TCPromptFileName, CDBInit, L"Test IDBPromptInitialize::PromptFileName")
	TEST_VARIATION(1, 		L"Boundary - E_INVALIDARG - hWnd == NULL")
	TEST_VARIATION(2, 		L"Boundary - E_INVALIDARG - hWnd == invalid")
	TEST_VARIATION(3, 		L"Boundary - E_INVALIDARG - dwPromptOptions")
	TEST_VARIATION(4, 		L"Boundary - E_INVALIDARG - ppwszSelectedFile == NULL")
	TEST_VARIATION(5, 		L"Parameters - pwszInitDirectory - non existent")
	TEST_VARIATION(6, 		L"Parameters - pwszInitDirectory - invalid")
	TEST_VARIATION(7, 		L"Parameters - pwszInitFile - non existent")
	TEST_VARIATION(8, 		L"Parameters - pwszInitFile - invalid")
	TEST_VARIATION(9, 		L"Stress - Verify FreeThreaded")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END

// {{ TCW_TEST_CASE_MAP(TCPooling)
//*-----------------------------------------------------------------------
// @class Test Session Pooling
//
class TCPooling : public CDBInit { 
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPooling,CDBInit);
	// }} TCW_DECLARE_FUNCS_END
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();

	// {{ TCW_TESTVARS()
	// @cmember GetDataSource - simple complete case
	int Variation_1();
	// @cmember Empty
	int Variation_2();
	// @cmember SetProperties - After GetDataSource
	int Variation_3();
	// @cmember SetProperties - More than once
	int Variation_4();
	// @cmember SetProperties - no-op
	int Variation_5();
	// @cmember Empty
	int Variation_6();
	// @cmember Stress - 1 Pool - with hundreads of requests for the same datasource
	int Variation_7();
	// @cmember Stress - Many Pools - each with interleaving requests
	int Variation_8();
	// @cmember Empty
	int Variation_9();
	// @cmember Session Properties - Set Session Properties, redraw from pool, verify reset to defaults
	int Variation_10();
	// @cmember Empty
	int Variation_11();
	// @cmember Timeout - Verify SPTimeOut - Registry Setting
	int Variation_12();
	// @cmember Timeout - Verify SPTimeOut - intervals
	int Variation_13();
	// @cmember Timeout - Verify SPTimeout - tight loop
	int Variation_14();
	// @cmember Timeout - Verify Retry Wait, ExpBackOff - Registry Setting
	int Variation_15();
	// @cmember Timeout - Verify Retry Wait, ExpBackOff
	int Variation_16();
	// @cmember Empty
	int Variation_17();
	// @cmember MultiUser - Apartment Model Pooled DSO, make sure its not reused outside that apartment
	int Variation_18();
	// }} TCW_TESTVARS_END
} ;
// {{ TCW_TESTCASE(TCPooling)
#define THE_CLASS TCPooling
BEG_TEST_CASE(TCPooling, CDBInit, L"Test Session Pooling")
	TEST_VARIATION(1, 		L"GetDataSource - simple complete case")
	TEST_VARIATION(2, 		L"Empty")
	TEST_VARIATION(3, 		L"SetProperties - After GetDataSource")
	TEST_VARIATION(4, 		L"SetProperties - More than once")
	TEST_VARIATION(5, 		L"SetProperties - no-op")
	TEST_VARIATION(6, 		L"Empty")
	TEST_VARIATION(7, 		L"Stress - 1 Pool - with hundreads of requests for the same datasource")
	TEST_VARIATION(8, 		L"Stress - Many Pools - each with interleaving requests")
	TEST_VARIATION(9, 		L"Empty")
	TEST_VARIATION(10, 		L"Session Properties - Set Session Properties, redraw from pool, verify reset to defaults")
	TEST_VARIATION(11, 		L"Empty")
	TEST_VARIATION(12, 		L"Timeout - Verify SPTimeOut - Registry Setting")
	TEST_VARIATION(13, 		L"Timeout - Verify SPTimeOut - intervals")
	TEST_VARIATION(14, 		L"Timeout - Verify SPTimeout - tight loop")
	TEST_VARIATION(15, 		L"Timeout - Verify Retry Wait, ExpBackOff - Registry Setting")
	TEST_VARIATION(16, 		L"Timeout - Verify Retry Wait, ExpBackOff")
	TEST_VARIATION(17, 		L"Empty")
	TEST_VARIATION(18, 		L"MultiUser - Apartment Model Pooled DSO, make sure its not reused outside that apartment")
END_TEST_CASE()
#undef THE_CLASS
// }} TCW_TESTCASE_END
// }} TCW_TEST_CASE_MAP_END



//*-----------------------------------------------------------------------
// Test Case Section
//*-----------------------------------------------------------------------




// }} END_DECLARE_TEST_CASES()


// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(9, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCCreateDBInstance)
	TEST_CASE(2, TCCreateDBInstanceEx)
	TEST_CASE(3, TCGetDataSource)
	TEST_CASE(4, TCGetInitializationString)
	TEST_CASE(5, TCLoadStringFromStorage)
	TEST_CASE(6, TCWriteStringToStorage)
	TEST_CASE(7, TCPromptDataSource)
	TEST_CASE(8, TCPromptFileName)
	TEST_CASE(9, TCPooling)
END_TEST_MODULE()
// }} TCW_TESTMODULE_END

// {{ TCW_TC_PROTOTYPE(TCCreateDBInstance)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateDBInstance - Test IDataInitialize::CreateDBInstance
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateDBInstance::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - CoGetClassObject(valid riids) - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_1()
{ 
	TBEGIN

	//ClassFactory - CoGetClassObject with Valid riids
	TESTC_(GetClassObject(IID_IUnknown),		S_OK);
	TESTC_(GetClassObject(IID_IClassFactory),	S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - CoGetClassObject(invalid riids) - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_2()
{ 
	TBEGIN

	//ClassFactory - CoGetClassObject with Invalid riids
	//NOTE: COM returns REGDB_E_CLASSNOTREG for this senario, even though E_NOINTERFACE
	//is spec'd in the docs for this senario.
	TEST2C_(GetClassObject(IID_IDataInitialize),		E_NOINTERFACE, REGDB_E_CLASSNOTREG);
	TEST2C_(GetClassObject(IID_IDBPromptInitialize),	E_NOINTERFACE, REGDB_E_CLASSNOTREG);
	TEST2C_(GetClassObject(IID_IDBInitialize),			E_NOINTERFACE, REGDB_E_CLASSNOTREG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - CoGetClassObject(NULL) - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_3()
{ 
	TBEGIN

	//ClassFactory - CoGetClassObject with Valid riids
	TESTC_(CoGetClassObject(CLSID_MSDAINITIALIZE, CLSCTX_INPROC_SERVER, NULL, IID_IUnknown,			NULL), E_INVALIDARG);
	TESTC_(CoGetClassObject(CLSID_MSDAINITIALIZE, CLSCTX_INPROC_SERVER, NULL, IID_IDataInitialize,  NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - CreateInstance(valid riids) - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_4()
{ 
	TBEGIN

	//CreateInstance with Valid riids
	TESTC_(CreateInstance(NULL, IID_IUnknown),				S_OK);
	TESTC_(CreateInstance(NULL, IID_IDataInitialize),		S_OK);
	TESTC_(CreateInstance(NULL, IID_IDBPromptInitialize),	E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - CreateInstance(invalid riids) - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_5()
{ 
	TBEGIN

	//CreateInstance with Invalid riids
	TESTC_(CreateInstance(NULL, IID_IClassFactory),	E_NOINTERFACE);
	TESTC_(CreateInstance(NULL, IID_IDBInitialize),	E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - CreateInstance(NULL) - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_6()
{ 
	TBEGIN

	//CreateInstance with NULL
	TESTC_(CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown,			NULL), E_INVALIDARG);
	TESTC_(CoCreateInstance(CLSID_MSDAINITIALIZE, NULL, CLSCTX_INPROC_SERVER, IID_IDataInitialize,	NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - LockServer combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_7()
{ 
	TBEGIN
	IClassFactory* pIClassFactory = NULL;
	IDataInitialize* pIDataInitialize = NULL;
	
	//Obtain ClassFactory
	TESTC_(GetClassObject(IID_IClassFactory, (IUnknown**)&pIClassFactory),	S_OK);

	//CreateInstance
	TESTC_(pIClassFactory->CreateInstance(NULL, IID_IDataInitialize, (void**)&pIDataInitialize),S_OK);

	//LockServer Combinations
	TESTC_(pIClassFactory->LockServer(TRUE),S_OK);  //Increment Count
	TESTC_(pIClassFactory->LockServer(TRUE),S_OK);  //Increment Count
	TESTC_(pIClassFactory->LockServer(FALSE),S_OK); //Decrement Count
	TESTC_(pIClassFactory->LockServer(FALSE),S_OK); //Decrement Count
	TESTC_(pIClassFactory->LockServer(TRUE),S_OK);  //Increment Count
	TESTC_(pIClassFactory->LockServer(FALSE),S_OK); //Decrement Count

CLEANUP:
	SAFE_RELEASE(pIClassFactory);
	SAFE_RELEASE_(pIDataInitialize); //Verify Reference == 0
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - QI combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_8()
{ 
	TBEGIN
	IClassFactory* pIClassFactory = NULL;
	
	//Obtain ClassFactory
	TESTC_(GetClassObject(IID_IClassFactory, (IUnknown**)&pIClassFactory),	S_OK);

	//QI Combinations - valid
	TESTC_(QI(pIClassFactory, IID_IUnknown),		S_OK);
	TESTC_(QI(pIClassFactory, IID_IClassFactory),	S_OK);

	//QI Combinations - invalid
	TESTC_(QI(pIClassFactory, IID_IDataInitialize),		E_NOINTERFACE);
	TESTC_(QI(pIClassFactory, IID_IDBPromptInitialize),	E_NOINTERFACE);

	//QI Combinations - NULL
	//S_OK if the interface is supported, E_NOINTERFACE if not.

	TEST2C_(pIClassFactory->QueryInterface(IID_IUnknown,		NULL), E_INVALIDARG, E_POINTER);
	TEST2C_(pIClassFactory->QueryInterface(IID_IClassFactory,	NULL), E_INVALIDARG, E_POINTER);

CLEANUP:
	SAFE_RELEASE(pIClassFactory);  
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - ClassFactory - AddRef/Release QI combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_9()
{ 
	TBEGIN
	IClassFactory* pIClassFactory = NULL;
	ULONG ulOrgRefCount = 0;

	//Obtain ClassFactory
	TESTC_(GetClassObject(IID_IClassFactory, (IUnknown**)&pIClassFactory),	S_OK);
	ulOrgRefCount = GetRefCount(pIClassFactory);

	//AddRef/Release Combinations
	SetRefCount(pIClassFactory, 100); // AddRef 100 times
	SetRefCount(pIClassFactory, -10); // Release 10 times
	SetRefCount(pIClassFactory,   1); // AddRef   1 time
	SetRefCount(pIClassFactory, -90); // Release 90 times
	SetRefCount(pIClassFactory,  -1); // Release  1 time

	//Make sure the RefCount is back where we started...
	TESTC(ulOrgRefCount == GetRefCount(pIClassFactory));

CLEANUP:
	SAFE_RELEASE(pIClassFactory);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc General - CoCreateInstance - Aggregation - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_10()
{ 
	TBEGIN
	IDataInitialize* pIDataInitialize = NULL;
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(&Aggregate, IID_IDataInitialize, (IUnknown**)&pIUnkInner), CLASS_E_NOAGGREGATION);
	Aggregate.SetUnkInner(pIUnkInner);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc General - CoCreateInstance - Aggregation - all valid combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_11()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//CreateInstance
	HRESULT hr = CreateInstance(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDataInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc General - CoCreateInstance - Verify DCM is Aggregated with DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_12()
{ 
	TBEGIN
	IUnknown* pDataSource = NULL;
	HRESULT hrDCM = S_OK;

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IUnknown, (IUnknown**)&pDataSource), S_OK);

	//QI Valid Combinations
	TESTC_(QI(pDataSource, IID_IUnknown),			S_OK);
	TESTC_(QI(pDataSource, IID_IDBInitialize),		S_OK);

	//Provider May not support aggregation, or may not have the OLEDB_SERVICES
	//registry key, in which the returned DSO will not have the SC aggregated.
	hrDCM = IsDCM(pDataSource) ? S_OK : E_NOINTERFACE;
	TESTC_(QI(pDataSource, IID_IService),			hrDCM);

	//QI Invalid Combinations
	TESTC_(QI(pDataSource, IID_IDataInitialize),	E_NOINTERFACE);
	TESTC_(QI(pDataSource, IID_IOpenRowset),		E_NOINTERFACE);
	TESTC_(QI(pDataSource, IID_IClassFactory),		E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_DSO(pDataSource);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc General - CoCreateInstance - Verify DCM is Aggregated with Aggregated DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_13()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	HRESULT hrDCM = S_OK;

	//CreateDBInstance
	HRESULT hr = CreateDBInstance(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize, (IUnknown**)&pIDBInitialize));
	
	//QI Valid Combinations
	TESTC_(QI(pIDBInitialize, IID_IUnknown),		S_OK);
	TESTC_(QI(pIDBInitialize, IID_IAggregate),		S_OK);
	TESTC_(QI(pIDBInitialize, IID_IDBInitialize),	S_OK);
	
	//Provider May not support aggregation, or may not have the OLEDB_SERVICES
	//registry key, in which the returned DSO will not have the SC aggregated.
	hrDCM = IsDCM(pIDBInitialize) ? S_OK : E_NOINTERFACE;
	TESTC_(QI(pIDBInitialize, IID_IService),		hrDCM);
				  
	//QI Invalid Combinations
	TESTC_(QI(pIDBInitialize, IID_IDataInitialize),	E_NOINTERFACE);
	TESTC_(QI(pIDBInitialize, IID_IOpenRowset),		E_NOINTERFACE);
	TESTC_(QI(pIDBInitialize, IID_IClassFactory),	E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - QI(valid riids) - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_14()
{ 
	TBEGIN
	IDataInitialize* pIDataInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(NULL, IID_IDataInitialize, (IUnknown**)&pIDataInitialize),	S_OK);

	//QI Invalid Combinations
	TESTC_(QI(pIDataInitialize, IID_IUnknown),			S_OK);
	TESTC_(QI(pIDataInitialize, IID_IDataInitialize),	S_OK);

CLEANUP:
	SAFE_RELEASE_(pIDataInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - QI(invalid riids) - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_15()
{ 
	TBEGIN
	IDataInitialize* pIDataInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(NULL, IID_IDataInitialize, (IUnknown**)&pIDataInitialize),	S_OK);

	//QI Invalid Combinations
	TESTC_(QI(pIDataInitialize, IID_NULL),				E_NOINTERFACE);
	TESTC_(QI(pIDataInitialize, IID_IClassFactory),		E_NOINTERFACE);
	TESTC_(QI(pIDataInitialize, IID_IDBInitialize),		E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_(pIDataInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - QI(NULL) - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_16()
{ 
	TBEGIN
	IDataInitialize* pIDataInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(NULL, IID_IDataInitialize, (IUnknown**)&pIDataInitialize),	S_OK);

	//QI Invalid Combinations
	//
	// S_OK if the interface is supported, E_NOINTERFACE if not.
	// Spec does not say that IUnknown::QI is required to return E_INVALIDARG
	// when invalid parameter is specfied. So changing code to expect both.
	TEST2C_(pIDataInitialize->QueryInterface(IID_IUnknown, NULL),		E_INVALIDARG, E_POINTER);
	TEST2C_(pIDataInitialize->QueryInterface(IID_IDataInitialize, NULL),	E_INVALIDARG, E_POINTER);


CLEANUP:
	SAFE_RELEASE_(pIDataInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - AddRef/Release combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_17()
{ 
	TBEGIN
	IDataInitialize* pIDataInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(NULL, IID_IDataInitialize, (IUnknown**)&pIDataInitialize),	S_OK);

	//AddRef/Release Combinations
	SetRefCount(pIDataInitialize, 100); // AddRef 100 times
	SetRefCount(pIDataInitialize, -10); // Release 10 times
	SetRefCount(pIDataInitialize,   1); // AddRef   1 time
	SetRefCount(pIDataInitialize, -90); // Release 90 times
	SetRefCount(pIDataInitialize,  -1); // Release  1 time

CLEANUP:
	SAFE_RELEASE_(pIDataInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Boundary - REGDB_E_CLASSNOTREG - Invalid CLSID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_18()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateDBInstance(CLSID_NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), REGDB_E_CLASSNOTREG);
	TESTC_(CreateDBInstance(IID_IAggregate, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), REGDB_E_CLASSNOTREG);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Boundary - REGDB_E_CLASSNOTREG - CLSID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_19()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	
	//CreateDBInstance
	TESTC_(CreateDBInstance(IID_IAggregate, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), REGDB_E_CLASSNOTREG);
	TESTC_(CreateDBInstance(CLSID_NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), REGDB_E_CLASSNOTREG);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_NOAGGREGATION - Provider doesn't support Aggregation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_20()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;

	//CreateDBInstance
	HRESULT hr = CreateDBInstance(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - Invalid CLSCTX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_21()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INVALID), E_INVALIDARG);

	// When passing '0' for Class Context, thid function does not return E_INVALIDARG rather
	// REGDB_E_CLASSNOTREG is returned. Bug #27213
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, 0), REGDB_E_CLASSNOTREG);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - ppDataSource == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_22()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(pDataInit()->CreateDBInstance(PROVIDER_CLSID, NULL, CLSCTX_INPROC_SERVER, NULL, IID_IDBInitialize, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - invalid riids
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_23()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDataInitialize), E_NOINTERFACE);
	TESTC_(CreateDBInstance(NULL, IID_IRowset), E_NOINTERFACE);
	TESTC_(CreateDBInstance(NULL, IID_NULL), E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - clsidProvider -  Enumerator Object - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_24()
{ 
	TBEGIN

	//CreateDBInstance
	//NOTE: Root Enumerator doesn't support IDBProperties - E_NOINTERFACE (non-DSO)
	TESTC_(CreateDBInstance(CLSID_MSDASQL_ENUMERATOR, NULL, IID_IUnknown), E_NOINTERFACE);
	
	//NOTE: Root Binder does support IDBProperties - S_OK (potential DSO interface)
	TESTC_(CreateDBInstance(CLSID_RootBinder, NULL, IID_IUnknown), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_25()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//CreateDBInstance
	TESTC_(CreateDBInstance(&Aggregate, IID_IDBInitialize, (IUnknown**)&pIUnkInner), DB_E_NOAGGREGATION);
	Aggregate.SetUnkInner(pIUnkInner);

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, verify inner
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_26()
{ 
	TBEGIN
	CAggregate Aggregate(pDataInit());
	IUnknown* pIUnkInner = NULL;
	
	//CreateDBInstance
	HRESULT hr = CreateDBInstance(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, verify Session, Command, Rowset can obtain aggregated DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_27()
{ 
	TBEGIN
	CAggregate Aggregate;
    IGetDataSource* pIGetDataSource = NULL;
    IDBCreateSession* pIDBCreateSession = NULL;
	IUnknown* pIUnkOuter	= NULL;
	IUnknown* pIAggregate	= NULL;
	IUnknown* pIUnkInner	= NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	
	//CreateDBInstance (Aggregated)
	HRESULT hr = CreateDBInstance(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation (note: DataSource is aggregated, not Session)
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

	//Now Initialize
	//Since the Service Components create a session (for session pooling)
	//Calling CreateNewDSO in on step has extra references...
	TESTC_(hr = InitializeDataSource(pIUnkInner),S_OK);

	//Obtain a new Session from that DSO
	TESTC_(hr = QI(pIUnkInner, IID_IDBCreateSession, (void**)&pIDBCreateSession),S_OK);

	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIDBCreateSession->CreateSession(NULL, IID_IGetDataSource, (IUnknown**)&pIGetDataSource),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//Verify the child correctly addref'd the parent outer.
	//The is an absolute requirement that the child keep the parent outer alive.
	//If it doesn't addref the outer, the outer can be released externally since
	//its not being used anymore due to the fact the outer controls the refcount
	//of the inner.  Many providers incorrectly addref the inner, which does nothing
	//but guareentee the inner survives, but the inner will delegate to the outer
	//and crash since it no longer exists...
	TESTC(ulRefCountAfter > ulRefCountBefore);

	//Verify we are hooked up...
	//This call we are using the Session and asking for IID_IAggregate of the DataSource, 
	//which is the outer object and should succeed!!!  Kind of cool huh!
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIDBCreateSession));

	//Now make sure the Session GetDataSource for IUnknown give me the outer
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIUnkOuter, pIDBCreateSession));

CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIUnkOuter);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_INPROC_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_28()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, NULL, CLSCTX_INPROC_SERVER), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_INPROC_HANDLER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_29()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, NULL, CLSCTX_INPROC_HANDLER), REGDB_E_CLASSNOTREG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_30()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, NULL, CLSCTX_SERVER), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_ALL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_31()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, NULL, CLSCTX_ALL), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_LOCAL_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_32()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	HRESULT hr = S_OK;

	//CreateDBInstance
	//DB_E_NOAGGREGATION - Currently COM doesn't suport aggregation (cross-process or cross-machine).  SC Internally
	//		uses Aggregation even though the consumer didn't ask for it and this will fail with
	//		DB_E_NOAGGREGATION as long as the provider supports outofproc
	//REGDB_E_CLASSNOTREG - If the provider doesn't support outofproc, COM will validate this 
	//		erro code first reporting that the object is not registered for outofproc...
	TEST3C_(hr = CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_LOCAL_SERVER), S_OK, DB_E_NOAGGREGATION, REGDB_E_CLASSNOTREG);

	if(hr == S_OK)
	{
		//S_OK - Service Components could not wrap the provider, probably does not have the OLEDB_SERVICES key...
		TESTC(pIDBInitialize != NULL);

		//Currently COM doesn't suport aggregation (cross-process or cross-machine).  SC Internally
		//uses Aggregation even though the consumer didn't ask for it and this will fail, 
		//unless the provider could not be aggregated, either OLEDB_SERVICES was missing or the provider
		//does not support aggregation in general...
		
		//Verify Service Components have not wrapped this DSO...
		TESTC(!IsDCM(pIDBInitialize));
	}

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_REMOTE_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_33()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;

	//CreateDBInstance
	//CO_E_CANT_REMOTE - but remoting may not be registered as well - REGDB_E_CLASSNOTREG
	TEST2C_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_REMOTE_SERVER), CO_E_CANT_REMOTE, REGDB_E_CLASSNOTREG);

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszReserved - valid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_34()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszReserved = L"Hello, are you ingoring me?";

	//CreateDBInstance
	TESTC_(pDataInit()->CreateDBInstance(PROVIDER_CLSID, NULL, CLSCTX_INPROC_SERVER, pwszReserved, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

CLEANUP:
	SAFE_RELEASE(pIDBInitialize);	
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - IID_IUnknown - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_35()
{ 
	TBEGIN

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IUnknown), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - all TDataSource interfaces - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_36()
{ 
	TBEGIN

	//CreateDBInstance [MANDATORY] Level - 0 Interfaces
	TESTC_(CreateDBInstance(NULL, IID_IUnknown), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_IDBProperties), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_IPersist), S_OK);

	//[MANDATORY] - Only allowed after Initialization
	TEST2C_(CreateDBInstance(NULL, IID_IDBCreateSession), E_UNEXPECTED, E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - Provider Specific interfaces - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_37()
{ 
	TBEGIN
	
	//Currently the Only provider that I know of that supports a provider 
	//specific DataSource interface is MSDASQL (IID_ISQLRequestDiagFields)
	//Since we really only interested that the DCM can return Provider Specific Interfaces
	//This should be sufficent when run against MSDASQL.
	TESTC_PROVIDER(PROVIDER_CLSID == CLSID_MSDASQL);

	//CreateDBInstance [MANDATORY] Level - 0 Interfaces
	TESTC_(CreateDBInstance(NULL, IID_IUnknown), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_IDBProperties), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_IPersist), S_OK);
	TESTC_(CreateDBInstance(NULL, IID_ISQLRequestDiagFields), S_OK);

	//[MANDATORY] - Only allowed after Initialization
	TEST2C_(CreateDBInstance(NULL, IID_IDBCreateSession), E_UNEXPECTED, E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Stress - 100 DataSources
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_38()
{ 
	TBEGIN
	const int cDataSources = 100;
	IDBInitialize* rgpDataSources[cDataSources];
	memset(rgpDataSources, 0, sizeof(IDBInitialize*)*cDataSources);

	ULONG i;
	//Create DataSources
	for(i=0; i<cDataSources; i++)
	{
		TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&rgpDataSources[i]), S_OK);
	}

CLEANUP:
	//Free DataSources
	for(i=0; i<cDataSources; i++)
		SAFE_RELEASE_DSO(rgpDataSources[i]);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Stress - Verify FreeThreaded
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_39()
{ 
	TBEGIN
	
	//Verify DCM is FreeThreaded or Both
	TESTC(VerifyThreadingModel(CLSID_MSDAINITIALIZE, L"Both") || VerifyThreadingModel(CLSID_MSDAINITIALIZE, L"Free"));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - CreateDBInstance from 3 threads, all succeess
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_40()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	//Setup Thread Arguments
	THREADARG T1Arg = { this, (void*)&IID_IDBInitialize, (void*)S_OK };

	//Create Threads
	CREATE_THREADS(Thread_CreateDBInstance, &T1Arg);

	START_THREADS();
	END_THREADS();	
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - CreateDBInstance from 3 threads, at least one error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_41()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	

	//Setup Thread Arguments
	THREADARG T1Arg = { this, (void*)&IID_IDBInitialize,	(void*)S_OK };
	THREADARG T2Arg = { this, (void*)&IID_IRowset,			(void*)E_NOINTERFACE };
	THREADARG T3Arg = { this, (void*)&IID_IDBProperties,	(void*)S_OK };
	
	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_CreateDBInstance, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_CreateDBInstance, &T2Arg);
	CREATE_THREAD(THREAD_THREE,	Thread_CreateDBInstance, &T3Arg);

	START_THREADS();
	END_THREADS();	
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - GetDataSource from 3 threads - each same provider with at least one in error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_42()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Provider hidden properties set
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstance::Variation_43()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	IUnknown* pUnkPooledDSO = NULL;

	//Create a datasource in the pool...
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, &pUnkPooledDSO),S_OK);
	SAFE_RELEASE_DSO(pUnkPooledDSO);

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Set provider hidden properties...
	TESTC_PROVIDER(SetHiddenInitProperties(pIDBInitialize)==S_OK);

	//Initialize - should fail due to hidden property
	TEST3C_(InitializeDataSource(pIDBInitialize), DB_E_ERRORSOCCURRED, E_FAIL, DB_SEC_E_AUTH_FAILED);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);	
	SAFE_RELEASE(pUnkPooledDSO);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateDBInstance::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCCreateDBInstanceEx)
//*-----------------------------------------------------------------------
//| Test Case:		TCCreateDBInstanceEx - Test IDataInitialize::CreateDBInstanceEx
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCCreateDBInstanceEx::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify DCM is Aggregated with DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_1()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	HRESULT hrDCM = S_OK;
	
	//CreateDBInstanceEx
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), S_OK);
	
	//QI Valid Combinations
	TESTC_(QI(rgMultiQI[0].pItf, IID_IUnknown),			S_OK);
	TESTC_(QI(rgMultiQI[0].pItf, IID_IDBInitialize),	S_OK);

	//Provider May not support aggregation, or may not have the OLEDB_SERVICES
	//registry key, in which the returned DSO will not have the SC aggregated.
	hrDCM = IsDCM(rgMultiQI[0].pItf) ? S_OK : E_NOINTERFACE;
	TESTC_(QI(rgMultiQI[0].pItf, IID_IService),			hrDCM);

	//QI Invalid Combinations
	TESTC_(QI(rgMultiQI[0].pItf, IID_IDataInitialize),	E_NOINTERFACE);
	TESTC_(QI(rgMultiQI[0].pItf, IID_IOpenRowset),		E_NOINTERFACE);
	TESTC_(QI(rgMultiQI[0].pItf, IID_IClassFactory),	E_NOINTERFACE);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Verify DCM is Aggregated with Aggregated DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_2()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	IDBProperties* pIDBProperties = NULL;
	CAggregate Aggregate;
	HRESULT hrDCM = S_OK;

	//CreateDBInstanceEx
	HRESULT hr = CreateDBInstanceEx(&Aggregate, cMultiQI, rgMultiQI);
	if(SUCCEEDED(hr))
	{
		TESTC_(Aggregate.SetUnkInner(rgMultiQI[0].pItf),S_OK);
		FreeMultiQI(cMultiQI, rgMultiQI);
	}

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBProperties, (IUnknown**)&pIDBProperties));
	
	//QI Valid Combinations
	TESTC_(QI(pIDBProperties, IID_IUnknown),		S_OK);
	TESTC_(QI(pIDBProperties, IID_IDBInitialize),	S_OK);

	//Provider May not support aggregation, or may not have the OLEDB_SERVICES
	//registry key, in which the returned DSO will not have the SC aggregated.
	hrDCM = IsDCM(pIDBProperties) ? S_OK : E_NOINTERFACE;
	TESTC_(QI(pIDBProperties, IID_IService),		hrDCM);
							
	//QI Invalid Combinations
	TESTC_(QI(pIDBProperties, IID_IDataInitialize),	E_NOINTERFACE);
	TESTC_(QI(pIDBProperties, IID_IOpenRowset),		E_NOINTERFACE);
	TESTC_(QI(pIDBProperties, IID_IClassFactory),	E_NOINTERFACE);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	SAFE_RELEASE_DSO(pIDBProperties);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - REGDB_E_CLASSNOTREG - Invalid clsid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_3()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
															
	//CreateDBInstanceEx (CLSID_NULL)
	TESTC_(CreateDBInstanceEx(CLSID_NULL, NULL, cMultiQI, rgMultiQI), REGDB_E_CLASSNOTREG);
	TESTC_(rgMultiQI[0].hr, REGDB_E_CLASSNOTREG);

	//CreateDBInstanceEx (IID_IAggregate)
	TESTC_(CreateDBInstanceEx(IID_IAggregate, NULL, cMultiQI, rgMultiQI), REGDB_E_CLASSNOTREG);
	TESTC_(rgMultiQI[0].hr, REGDB_E_CLASSNOTREG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - REGDB_E_CLASSNOTREG - CLSID_NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_4()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	MULTI_QI rgMultiQI2[cMultiQI] = { &IID_IDBCreateSession, NULL, S_OK};
															
	//CreateDBInstanceEx (CLSID_NULL)
	TESTC_(CreateDBInstanceEx(CLSID_NULL, NULL, cMultiQI, rgMultiQI), REGDB_E_CLASSNOTREG);
	TESTC_(rgMultiQI[0].hr, REGDB_E_CLASSNOTREG);

	//CreateDBInstanceEx (IID_IAggregate)
	TESTC_(CreateDBInstanceEx(IID_IAggregate, NULL, cMultiQI, rgMultiQI2), REGDB_E_CLASSNOTREG);
	TESTC_(rgMultiQI2[0].hr, REGDB_E_CLASSNOTREG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	FreeMultiQI(cMultiQI, rgMultiQI2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_NOAGGREGATION - Possible on a provider that doesn't support aggregation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_5()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	CAggregate Aggregate;
	
	//CreateDBInstanceEx (IID_IUnknown)
	//Aggregation postive testing is tested in other varaitions
	HRESULT hr = CreateDBInstanceEx(&Aggregate, cMultiQI, rgMultiQI);
	if(SUCCEEDED(hr))
	{
		TESTC_(Aggregate.SetUnkInner(rgMultiQI[0].pItf),S_OK);
		FreeMultiQI(cMultiQI, rgMultiQI);
	}

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_NOAGGREGATION - Specify out of process
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_6()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	CAggregate Aggregate;
	
	//CreateDBInstanceEx (IID_IUnknown)
	//Aggregation postive testing is tested in other varaitions
	//Remoting may not be registered as well - REGDB_E_CLASSNOTREG
	TEST2C_(CreateDBInstanceEx(&Aggregate, cMultiQI, rgMultiQI, NULL, CLSCTX_REMOTE_SERVER), DB_E_NOAGGREGATION, REGDB_E_CLASSNOTREG);
	TEST2C_(rgMultiQI[0].hr, DB_E_NOAGGREGATION, REGDB_E_CLASSNOTREG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - Invalid CLSCTX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_7()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_INVALID), E_INVALIDARG);

	// Ideally, this should return E_INVALIDARG, but due to Bug #27213 as Won't Fix
	// it returns REGDB_E_CLASSNOTREG
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, 0), REGDB_E_CLASSNOTREG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - cmq == 0
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_8()
{ 
	TBEGIN
	const ULONG cMultiQI = 0;
	MULTI_QI rgMultiQI[1] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), E_INVALIDARG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - rgmqResults == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_9()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - rgmqResults[0].pIID == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_10()
{ 
	TBEGIN
	const ULONG cMultiQI = 2;
	MULTI_QI rgMultiQI[cMultiQI] = {	NULL, NULL, S_OK,
										&IID_IUnknown, NULL, S_OK,	};

	const ULONG cMultiQI2 = 3;
	MULTI_QI rgMultiQI2[cMultiQI2] = {	&IID_IUnknown,			NULL, S_OK,
										NULL,					NULL, S_OK,					
										&IID_IDBCreateSession,	NULL, S_OK,	};
	
	//CreateDBInstanceEx (cMultiQI)
	TESTC_(CreateDBInstanceEx(NULL, 1, rgMultiQI, NULL, CLSCTX_INPROC_SERVER), E_NOINTERFACE);
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_INPROC_SERVER), CO_S_NOTALLINTERFACES);
	TESTC_(rgMultiQI[0].hr, E_INVALIDARG);
	TESTC_(rgMultiQI[1].hr, S_OK);

	//CreateDBInstanceEx (cMultiQI2)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI2, rgMultiQI2, NULL, CLSCTX_INPROC_SERVER), CO_S_NOTALLINTERFACES);
	TESTC_(rgMultiQI[0].hr, S_OK);
	TESTC_(rgMultiQI[1].hr, E_INVALIDARG);
	TEST2C_(rgMultiQI[2].hr, E_UNEXPECTED, E_NOINTERFACE);

	//CreateDBInstanceEx (Other Invalid Arguments)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_INVALID), E_INVALIDARG);
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, 0), E_INVALIDARG);
  
CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - Verify combinations of NULL pointers inside pServerInfo
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_11()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown,			NULL, S_OK	};

	COSERVERINFO ServerInfo = { 0, NULL, NULL, 0 };

	//CreateDBInstanceEx ( Current Machine)
	ServerInfo.pwszName = GetMachineName();
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), S_OK);
	SAFE_FREE(ServerInfo.pwszName);

	//CreateDBInstanceEx ( ServerInfo.pwszName = NULL )
	ServerInfo.pwszName = NULL;
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), E_INVALIDARG);

	//CreateDBInstanceEx ( ServerInfo.pwszName = ""	)
	ServerInfo.pwszName = wcsDuplicate(L"");
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), E_INVALIDARG);
	SAFE_FREE(ServerInfo.pwszName);

	//CreateDBInstanceEx ( ServerInfo.pwszName = "Garbage" )
	ServerInfo.pwszName = wcsDuplicate(L"Garbage");
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), E_INVALIDARG);
	SAFE_FREE(ServerInfo.pwszName);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	SAFE_FREE(ServerInfo.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - Verify Authenitcation Information
//
// @rdesc TEST_PASS or TEST_FAIL 
//			 <wtypes.h>
int TCCreateDBInstanceEx::Variation_12()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown,			NULL, S_OK	};

	COAUTHINFO		AuthInfo	= { 0, 0, NULL, 0, 0, NULL, 0 };	
	COSERVERINFO	ServerInfo	= { 0, GetMachineName(), &AuthInfo, 0 };
	COAUTHIDENTITY	AuthIdentity= { NULL, NULL, NULL, 0, 0 };

	//CreateDBInstanceEx - No Authentication Info - Shoulld use default NTS
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), S_OK);

	//CreateDBInstanceEx - No Authentication Info
	AuthInfo.pwszServerPrincName = ServerInfo.pwszName;
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), E_INVALIDARG);

	//CreateDBInstanceEx - No Authentication Info
	AuthInfo.pAuthIdentityData = &AuthIdentity;
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, &ServerInfo), E_INVALIDARG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	SAFE_FREE(ServerInfo.pwszName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - CO_S_NOTALLINTERFACES - (IUnknown, IDBInitialize, IRowset)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_13()
{ 
	TBEGIN
	const ULONG cMultiQI = 5;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown,			NULL, S_OK,  
									 &IID_IDBInitialize,	NULL, S_OK,								
									 &IID_IRowset,			NULL, S_OK,
									 &IID_IDBCreateSession,	NULL, S_OK,	
									 &IID_IDBProperties,	NULL, S_OK,	};

	//CreateDBInstanceEx
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), CO_S_NOTALLINTERFACES);
	TESTC_(rgMultiQI[0].hr, S_OK);
	TESTC_(rgMultiQI[1].hr, S_OK);
	TESTC_(rgMultiQI[2].hr, E_NOINTERFACE);
	TEST2C_(rgMultiQI[3].hr, E_NOINTERFACE, E_UNEXPECTED);
	TESTC_(rgMultiQI[4].hr, S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - (IID_NULL, IDBInitialize, IOpenRowset)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_14()
{ 
	TBEGIN
	const ULONG cMultiQI = 3;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_NULL,				NULL, S_OK,  
									 &IID_IDataInitialize,	NULL, S_OK,								
									 &IID_IRowset,			NULL, S_OK,	};

	//CreateDBInstanceEx
	//This helper will validate all arguments for error cases...
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), E_NOINTERFACE);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - clsidProvider -  Enumerator Object - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_15()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	
	//CreateDBInstanceEx - passing in clsid as an Enumerator Object
	TESTC_(CreateDBInstanceEx(CLSID_MSDASQL_ENUMERATOR, NULL, cMultiQI, rgMultiQI), E_NOINTERFACE);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, not asking for IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_16()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	CAggregate Aggregate;

	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(&Aggregate, cMultiQI, rgMultiQI), DB_E_NOAGGREGATION);
	
	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	TESTC_(rgMultiQI[0].hr, DB_E_NOAGGREGATION);
	COMPARE(Aggregate.GetRefCount(), 1);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, verify inner
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_17()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	CAggregate Aggregate;
	
	//CreateDBInstanceEx (aggregated)
	HRESULT hr = CreateDBInstanceEx(&Aggregate, cMultiQI, rgMultiQI);
	if(SUCCEEDED(hr))
	{
		TESTC_(Aggregate.SetUnkInner(rgMultiQI[0].pItf),S_OK);
		FreeMultiQI(cMultiQI, rgMultiQI);
	}

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, verify Session, Command, Rowset, can all get back to the aggregated DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_18()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	CAggregate Aggregate;
    IGetDataSource* pIGetDataSource = NULL;
    IDBCreateSession* pIDBCreateSession = NULL;
	IUnknown* pIUnkOuter	= NULL;
	IUnknown* pIAggregate	= NULL;
	IUnknown* pIUnkInner	= NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	
	//CreateDBInstanceEx (Aggregated)
	HRESULT hr = CreateDBInstanceEx(&Aggregate, cMultiQI, rgMultiQI);
	if(SUCCEEDED(hr))
	{
		pIUnkInner = rgMultiQI[0].pItf;
		TESTC_(Aggregate.SetUnkInner(pIUnkInner),S_OK);
	}

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

	//Now Initialize
	//Since the Service Components create a session (for session pooling)
	//Calling CreateNewDSO in on step has extra references...
	TESTC_(hr = InitializeDataSource(pIUnkInner),S_OK);

	//Obtain a new Session from that DSO
	TESTC_(hr = QI(pIUnkInner, IID_IDBCreateSession, (void**)&pIDBCreateSession),S_OK);
	
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIDBCreateSession->CreateSession(NULL, IID_IGetDataSource, (IUnknown**)&pIGetDataSource),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//Verify the child correctly addref'd the parent outer.
	//The is an absolute requirement that the child keep the parent outer alive.
	//If it doesn't addref the outer, the outer can be released externally since
	//its not being used anymore due to the fact the outer controls the refcount
	//of the inner.  Many providers incorrectly addref the inner, which does nothing
	//but guareentee the inner survives, but the inner will delegate to the outer
	//and crash since it no longer exists...
	TESTC(ulRefCountAfter > ulRefCountBefore);
	
	//Verify we are hooked up...
	//This call we are using the Session and asking for IID_IAggregate of the DataSource, 
	//which is the outer object and should succeed!!!  Kind of cool huh!
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIDBCreateSession));

	//Now make sure the Session GetDataSource for IUnknown give me the outer
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIUnkOuter, pIDBCreateSession));

CLEANUP:
	FreeMultiQI(cMultiQI, rgMultiQI);
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIUnkOuter);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_INPROC_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_19()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_INPROC_SERVER), S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_INPROC_HANDLER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_20()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_INPROC_HANDLER), REGDB_E_CLASSNOTREG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_21()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_SERVER), S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_ALL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_22()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_ALL), S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_LOCAL_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_23()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	HRESULT hr = S_OK;
	
	//CreateDBInstanceEx (IID_IUnknown)
	//DB_E_NOAGGREGATION - Currently COM doesn't suport aggregation (cross-process or cross-machine).  SC Internally
	//		uses Aggregation even though the consumer didn't ask for it and this will fail with
	//		DB_E_NOAGGREGATION as long as the provider supports outofproc
	//REGDB_E_CLASSNOTREG - If the provider doesn't support outofproc, COM will validate this 
	//		erro code first reporting that the object is not registered for outofproc...
	TEST3C_(hr = CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_LOCAL_SERVER), S_OK, DB_E_NOAGGREGATION, REGDB_E_CLASSNOTREG);
	TESTC_(rgMultiQI[0].hr, hr);

	if(hr == S_OK)
	{
		//S_OK - Service Components could not wrap the provider, probably does not have the OLEDB_SERVICES key...
		TESTC(rgMultiQI[0].pItf != NULL);

		//Currently COM doesn't suport aggregation (cross-process or cross-machine).  SC Internally
		//uses Aggregation even though the consumer didn't ask for it and this will fail, 
		//unless the provider could not be aggregated, either OLEDB_SERVICES was missing or the provider
		//does not support aggregation in general...
		
		//Verify Service Components have not wrapped this DSO...
		TESTC(!IsDCM(rgMultiQI[0].pItf));
	}

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCLSCTX - CLSCTX_REMOTE_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_24()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	//Remoting may not be registered as well - REGDB_E_CLASSNOTREG
	TEST2C_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI, NULL, CLSCTX_REMOTE_SERVER), CO_E_CANT_REMOTE, REGDB_E_CLASSNOTREG);
	TEST2C_(rgMultiQI[0].hr, CO_E_CANT_REMOTE, REGDB_E_CLASSNOTREG);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszReserved - ingored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_25()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IDBInitialize, NULL, S_OK};
	WCHAR* pwszReserved = L"Hello, are you ingoring me?";
	HRESULT hr;

	//CreateDBInstanceEx 
	TESTC_(hr = pDataInit()->CreateDBInstanceEx(PROVIDER_CLSID, NULL, CLSCTX_INPROC_SERVER, pwszReserved, NULL, cMultiQI, rgMultiQI), S_OK);
	TESTC(VerifyMultiQI(hr, cMultiQI, rgMultiQI));

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - cmq - Combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_26()
{ 
	TBEGIN
	const ULONG cMultiQI = 2;
	MULTI_QI rgMultiQI[cMultiQI] = {	&IID_IUnknown, NULL, S_OK,
										&IID_IUnknown, NULL, S_OK,	};
	
	//CreateDBInstanceEx (cMulti = 0, rgMulti = NULL)
	TESTC_(CreateDBInstanceEx(NULL, 0, NULL), E_INVALIDARG);
	
	//CreateDBInstanceEx (cMulti = 0, rgMulti != NULL)
	TESTC_(CreateDBInstanceEx(NULL, 0, rgMultiQI), E_INVALIDARG);

	//CreateDBInstanceEx (cMulti != 0, rgMulti == NULL)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, NULL), E_INVALIDARG);

	//CreateDBInstanceEx (cMulti != 0, rgMulti != NULL)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), S_OK);
	FreeMultiQI(cMultiQI, rgMultiQI);

	//CreateDBInstanceEx (cMulti < Array Size, rgMulti != NULL)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI-1, rgMultiQI), S_OK);

	//Make sure only 1 returned.
	TESTC(rgMultiQI[1].hr == S_OK && rgMultiQI[1].pItf == NULL);
	FreeMultiQI(cMultiQI, rgMultiQI);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - rgmqResults - IID_IUnknown - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_27()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_IUnknown, NULL, S_OK};
	
	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Parameters - rgmqResults - { all DataSource interfaces } - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_28()
{ 
	TBEGIN
	const ULONG cMultiQI = 11;
	MULTI_QI rgMultiQI[cMultiQI] = {//[MANDATORY]
									&IID_IUnknown,					NULL, S_OK, //0
									&IID_IDBCreateSession,			NULL, S_OK,	//1
									&IID_IDBInitialize,				NULL, S_OK,	//2
									&IID_IDBProperties,				NULL, S_OK,	//3
									&IID_IPersist,					NULL, S_OK,	//4
									//[OPTIONAL]
									&IID_IConnectionPointContainer,	NULL, S_OK, //5
									&IID_IDBAsynchStatus,			NULL, S_OK,	//6
									&IID_IDBDataSourceAdmin,		NULL, S_OK,	//7
									&IID_IDBInfo,					NULL, S_OK,	//8
									&IID_IPersistFile,				NULL, S_OK,	//9
									&IID_ISupportErrorInfo,			NULL, S_OK,	//10
									};
	
	//CreateDBInstanceEx (IID_IUnknown)
	HRESULT hr = CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI);
	TEST2C_(hr, S_OK, CO_S_NOTALLINTERFACES);

	//[MANDATORY] interfaces - Level 0
	TESTC_(rgMultiQI[0].hr,	S_OK);
	TESTC_(rgMultiQI[2].hr,	S_OK);
	TESTC_(rgMultiQI[3].hr,	S_OK);
	TESTC_(rgMultiQI[4].hr,	S_OK);
	
	//[MANDATORY] Only Available After Initialized
	TEST2C_(rgMultiQI[1].hr, E_NOINTERFACE, E_UNEXPECTED);

	//[OPTIONAL]
	TEST2C_(rgMultiQI[5].hr,	S_OK, E_NOINTERFACE);
	TEST2C_(rgMultiQI[6].hr,	S_OK, E_NOINTERFACE);
	TEST2C_(rgMultiQI[7].hr,	S_OK, E_NOINTERFACE);
	TEST2C_(rgMultiQI[10].hr,S_OK, E_NOINTERFACE);

	//[OPTIONAL] Only Available After Initialized
	TEST3C_(rgMultiQI[8].hr,	S_OK, E_NOINTERFACE, E_UNEXPECTED);
	TEST3C_(rgMultiQI[9].hr,	S_OK, E_NOINTERFACE, E_UNEXPECTED);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - rgmqResults - { Provider Specific Interfaces } - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_29()
{ 
	TBEGIN
	const ULONG cMultiQI = 1;
	MULTI_QI rgMultiQI[cMultiQI] = { &IID_ISQLRequestDiagFields, NULL, S_OK};
	
	//Currently the Only provider that I know of that supports a provider 
	//specific DataSource interface is MSDASQL (IID_ISQLRequestDiagFields)
	//Since we really only interested that the DCM can return Provider Specific Interfaces
	//This should be sufficent when run against MSDASQL.
	TESTC_PROVIDER(PROVIDER_CLSID == CLSID_MSDASQL);

	//CreateDBInstanceEx (IID_IUnknown)
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Stress - 100 DataSources
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_30()
{ 
	TBEGIN
	const ULONG cMultiQI = 100;
	MULTI_QI rgMultiQI[cMultiQI];
	memset(rgMultiQI, 0, sizeof(rgMultiQI));

	//Ask for IID_IDBInitalize for all of them
	for(ULONG i=0; i<cMultiQI; i++)
		rgMultiQI[i].pIID = &IID_IDBInitialize;
	
	//CreateDBInstanceEx 
	TESTC_(CreateDBInstanceEx(NULL, cMultiQI, rgMultiQI), S_OK);

CLEANUP:
	//Free Returned interfaces
	FreeMultiQI(cMultiQI, rgMultiQI);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - CreateDBInstance from 3 threads, all success
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_31()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	const ULONG cIIDs = MAX_THREADS;
	IID rgIIDs[cIIDs];
	
	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, (void*)cIIDs, rgIIDs };

	for(ULONG i=0; i<MAX_THREADS; i++)
		rgIIDs[i] = IID_IUnknown;

	//Create Threads
	CREATE_THREADS(Thread_CreateDBInstanceEx, &T1Arg);

	START_THREADS();
	END_THREADS();	

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - CreateDBInstance from 3 threads, at least one error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCCreateDBInstanceEx::Variation_32()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	
	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, (void*)1, (void*)&IID_IUnknown,	(void*)S_OK };
	THREADARG T2Arg = { this, (void*)1, (void*)&IID_IRowset,	(void*)E_NOINTERFACE };
	THREADARG T3Arg = { this, (void*)1, (void*)&IID_IUnknown,	(void*)S_OK };

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_CreateDBInstanceEx, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_CreateDBInstanceEx, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_CreateDBInstanceEx, &T3Arg);

	START_THREADS();
	END_THREADS();	

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCCreateDBInstanceEx::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCGetDataSource)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetDataSource - Test IDataInitialize::GetDataSource
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetDataSource::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify DCM is Aggregated with DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_1()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	HRESULT hrDCM =	S_OK;

	//GetDataSource
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBProperties, (IUnknown**)&pIUnknown), S_OK);

	//QI Valid Combinations
	TESTC_(QI(pIUnknown, IID_IUnknown),			S_OK);
	TESTC_(QI(pIUnknown, IID_IDBInitialize),	S_OK);

	//Provider May not support aggregation, or may not have the OLEDB_SERVICES
	//registry key, in which the returned DSO will not have the SC aggregated.
	hrDCM = IsDCM(pIUnknown) ? S_OK : E_NOINTERFACE;
	TESTC_(QI(pIUnknown, IID_IService),			hrDCM);

	//QI Invalid Combinations
	TESTC_(QI(pIUnknown, IID_IDataInitialize),	E_NOINTERFACE);
	TESTC_(QI(pIUnknown, IID_IOpenRowset),		E_NOINTERFACE);
	TESTC_(QI(pIUnknown, IID_IClassFactory),	E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - Verify DCM is Aggregated with Aggregated DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_2()
{ 
	TBEGIN
	CAggregate Aggregate;
	IDBInitialize* pIDBInitialize = NULL;
	IUnknown* pIUnkInner = NULL;
	HRESULT hrDCM =	S_OK;
	
	//GetDataSource
	HRESULT hr = GetDataSource(&Aggregate, m_pwszProperties, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize, (IUnknown**)&pIDBInitialize));

	//QI Valid Combinations
	TESTC_(QI(pIDBInitialize, IID_IUnknown),			S_OK);
	TESTC_(QI(pIDBInitialize, IID_IAggregate),			S_OK);
	TESTC_(QI(pIDBInitialize, IID_IDBInitialize),		S_OK);

	//Provider May not support aggregation, or may not have the OLEDB_SERVICES
	//registry key, in which the returned DSO will not have the SC aggregated.
	hrDCM = IsDCM(pIDBInitialize) ? S_OK : E_NOINTERFACE;
	TESTC_(QI(pIDBInitialize, IID_IService),			hrDCM);

	//QI Invalid Combinations
	TESTC_(QI(pIDBInitialize, IID_IDataInitialize),		E_NOINTERFACE);
	TESTC_(QI(pIDBInitialize, IID_IOpenRowset),			E_NOINTERFACE);
	TESTC_(QI(pIDBInitialize, IID_IClassFactory),		E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DB_E_NOAGGREGATION - Provider may not support aggregation
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_3()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//GetDataSource (use the Default InitString)
	HRESULT hr = GetDataSource(&Aggregate, m_pwszProperties, IID_IUnknown, &pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - Invalid CLSCTX
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_4()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, CLSCTX_INVALID), E_INVALIDARG);
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, 0), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - pwszInitString==NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_5()
{ 
	TBEGIN
	IPersist* pIPersist = NULL;
	CLSID clsid,
			CLSID_SQLOLEDB;

	TESTC_(CLSIDFromProgID(L"SQLOLEDB", &CLSID_SQLOLEDB), S_OK);
	
	//GetDataSource (use NULL InitString)
	//The spec allows NULL string, and default to MSDASQL.
	TESTC_(GetDataSource(NULL, NULL, IID_IPersist, (IUnknown**)&pIPersist), S_OK);

	pIPersist->GetClassID(&clsid);

#ifdef _WIN64
	TESTC(clsid == CLSID_SQLOLEDB);
#else
	TESTC(clsid == CLSID_MSDASQL);
#endif

CLEANUP:
	SAFE_RELEASE_DSO(pIPersist);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - ppDataSource==NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_6()
{ 
	TBEGIN
	WCHAR* pwszInitString = NULL;	

	//GetInitString
	TESTC_(GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource (use NULL InitString)
	//Wa have to call directly, since our helper (GetDataSource) never passed NULL...
	TESTC_(pDataInit()->GetDataSource(NULL, CLSCTX_INPROC_SERVER, pwszInitString, IID_IUnknown, NULL), E_INVALIDARG);

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - IID_INULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_7()
{ 
	TBEGIN
		
	//GetDataSource (use NULL InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_NULL), E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - non-DataSource Object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_8()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	
	//Create a Session  Object!
	TESTC_(CreateNewSession(NULL, IID_IUnknown, &pIUnknown),S_OK);
	
	//GetDataSource (using default InitString)
	//Should fail when trying to SetProperties...
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, &pIUnknown), E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - Enumerator Object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_9()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	
	//Create Uninitialized Enumerator Object!
	//May not be installed on the System?
	TESTC_PROVIDER(CoCreateInstance(CLSID_MSDASQL_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnknown)==S_OK);
	
	//GetDataSource (using a passed in InitString with the Enumerator)
	TESTC_(GetDataSource(NULL, L"Provider=MSDASQL Enumerator;", IID_IUnknown, &pIUnknown), E_NOINTERFACE);
	TESTC(pIUnknown != NULL);
	SAFE_RELEASE_DSO(pIUnknown);

	//GetDataSource
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, &pIUnknown), S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - IID_IDataInitialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_10()
{ 
	TBEGIN
	IDataInitialize* pIDataInitialize = NULL;
	
	//GetDataSource (using default InitString)
	//Should fail when trying to SetProperties...
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDataInitialize, (IUnknown**)&pIDataInitialize), E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE(pIDataInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, not passing IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_11()
{ 
	TBEGIN
	CAggregate Aggregate(pDataInit());
	IUnknown* pIUnkInner = NULL;
	
	//GetDataSource (use the Default InitString)
	TEST2C_(GetDataSource(&Aggregate, m_pwszProperties, IID_IDBInitialize, &pIUnkInner), DB_E_NOAGGREGATION, CLASS_E_NOAGGREGATION);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);
   
CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, verify inner
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_12()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//GetDataSource
	HRESULT hr = GetDataSource(&Aggregate, m_pwszProperties, IID_IUnknown, &pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pUnkOuter - Aggregation, verify GetDataSource returns outer
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_13()
{ 
	TBEGIN
	CAggregate Aggregate;
    IGetDataSource* pIGetDataSource = NULL;
    IDBCreateSession* pIDBCreateSession = NULL;
	IUnknown* pIUnkOuter	= NULL;
	IUnknown* pIAggregate	= NULL;
	IUnknown* pIUnkInner	= NULL;
	ULONG ulRefCountBefore, ulRefCountAfter;
	
	//GetDataSource (note: DataSource is aggregation not session)
	HRESULT hr = GetDataSource(&Aggregate, m_pwszProperties, IID_IUnknown, &pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//VerifyArregation
	//Indicate we are not Initialized at this point...
	TESTC_PROVIDER(hr = Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize, NULL, FALSE));

	//Now Initialize
	//Since the Service Components create a session (for session pooling)
	//Calling CreateNewDSO in on step has extra references...
	TESTC_(hr = InitializeDataSource(pIUnkInner),S_OK);

	//Obtain a new Session from that DSO
	TESTC_(hr = QI(pIUnkInner, IID_IDBCreateSession, (void**)&pIDBCreateSession),S_OK);
	
	ulRefCountBefore = Aggregate.GetRefCount();
	TESTC_(hr = pIDBCreateSession->CreateSession(NULL, IID_IGetDataSource, (IUnknown**)&pIGetDataSource),S_OK);
	ulRefCountAfter = Aggregate.GetRefCount();

	//Verify the child correctly addref'd the parent outer.
	//The is an absolute requirement that the child keep the parent outer alive.
	//If it doesn't addref the outer, the outer can be released externally since
	//its not being used anymore due to the fact the outer controls the refcount
	//of the inner.  Many providers incorrectly addref the inner, which does nothing
	//but guareentee the inner survives, but the inner will delegate to the outer
	//and crash since it no longer exists...
	TESTC(ulRefCountAfter > ulRefCountBefore);
	
	//Verify we are hooked up...
	//This call we are using the Session and asking for IID_IAggregate of the DataSource, 
	//which is the outer object and should succeed!!!  Kind of cool huh!
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IAggregate, (IUnknown**)&pIAggregate),S_OK);
	TESTC(VerifyEqualInterface(pIAggregate, pIDBCreateSession));

	//Now make sure the Session GetDataSource for IUnknown give me the outer
	TESTC_(hr = pIGetDataSource->GetDataSource(IID_IUnknown, (IUnknown**)&pIUnkOuter),S_OK);
	TESTC(VerifyEqualInterface(pIUnkOuter, pIDBCreateSession));

CLEANUP:
	SAFE_RELEASE(pIAggregate);
	SAFE_RELEASE(pIUnkOuter);
	SAFE_RELEASE(pIGetDataSource);
	SAFE_RELEASE(pIDBCreateSession);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - CLSCTX - CLSCTX_INPROC_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_14()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, CLSCTX_INPROC_SERVER), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - CLSCTX - CLSCTX_INPROC_HANDLER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_15()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, CLSCTX_INPROC_HANDLER), REGDB_E_CLASSNOTREG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - CLSCTX - CLSCTX_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_16()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, CLSCTX_SERVER), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - CLSCTX - CLSCTX_ALL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_17()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, CLSCTX_ALL), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - CLSCTX - CLSCTX_LOCAL_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_18()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	HRESULT hr = S_OK;
	
	//GetDataSource (use the Default InitString)
	//DB_E_NOAGGREGATION - Currently COM doesn't suport aggregation (cross-process or cross-machine).  SC Internally
	//		uses Aggregation even though the consumer didn't ask for it and this will fail with
	//		DB_E_NOAGGREGATION as long as the provider supports outofproc
	//REGDB_E_CLASSNOTREG - If the provider doesn't support outofproc, COM will validate this 
	//		erro code first reporting that the object is not registered for outofproc...
	TEST3C_(hr = GetDataSource(NULL, m_pwszProperties, IID_IUnknown, &pIUnknown, CLSCTX_LOCAL_SERVER), S_OK, DB_E_NOAGGREGATION, REGDB_E_CLASSNOTREG);

	if(hr == S_OK)
	{
		//S_OK - Service Components could not wrap the provider, probably does not have the OLEDB_SERVICES key...
		TESTC(pIUnknown != NULL);

		//Currently COM doesn't suport aggregation (cross-process or cross-machine).  SC Internally
		//uses Aggregation even though the consumer didn't ask for it and this will fail, 
		//unless the provider could not be aggregated, either OLEDB_SERVICES was missing or the provider
		//does not support aggregation in general...
		
		//Verify Service Components have not wrapped this DSO...
		TESTC(!IsDCM(pIUnknown));
	}

CLEANUP:
	SAFE_RELEASE(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - CLSCTX - CLSCTX_REMOTE_SERVER
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_19()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	//Remoting may not be registered - REGDB_E_CLASSNOTREG
	TEST2C_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown, NULL, CLSCTX_REMOTE_SERVER), CO_E_CANT_REMOTE, REGDB_E_CLASSNOTREG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitString - Contains different ProgID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_20()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	HRESULT hr = S_OK;
	
	//Create Uninitialized DSO - (do everything expect Initialize)
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)

	//Create Uninitialized DSO (of another provider!)
	TESTC_PROVIDER(CreateDiffProvider(NULL, IID_IUnknown, &pIUnknown, CREATEDSO_SETPROPERTIES)==S_OK);
	TESTC_(GetInitString(pIUnknown, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER), DB_E_MISMATCHEDPROVIDER);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitString - Contains different ProgID, with versioning
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_21()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	HRESULT hr = S_OK;
	
	//Create Uninitialized DSO (of another provider!)
	TESTC_PROVIDER(CreateDiffProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize)==S_OK);

	//Create InitString
	pwszInitString = CreateString(L" PROVIDER =%s.7890", GetProgID());

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER), DB_E_MISMATCHEDPROVIDER);

CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitString - Contains same ProgID, but with versioning
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_22()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	HRESULT hr = S_OK;
	
	//Create Uninitialized DSO - (do everything expect Initialize)
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)

	//Create InitString
	pwszInitString = CreateString(L" PRovider =%s.7890", GetProgID());

	//GetDataSource (and indicate provider didn't change = FALSE = default)
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER), S_OK);

CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitString -  Enumerator Object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_23()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	
	//GetDataSource (using a passed in InitString with the Enumerator)
	TESTC_(GetDataSource(NULL, L"Provider=MSDAENUM;", IID_IUnknown, &pIUnknown), E_NOINTERFACE);
	TESTC(pIUnknown == NULL);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - IID_IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_24()
{ 
	TBEGIN
		
	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - verify all TDataSource interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_25()
{ 
	TBEGIN
	HRESULT hr = S_OK;
		
	//GetDataSource (use the Default InitString)
	//[MANDATORY]
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IUnknown), S_OK);
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize), S_OK);
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBProperties), S_OK);
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IPersist), S_OK);
	
	//[MANDATORY] Only Available after Initalized
	TEST2C_(GetDataSource(NULL, m_pwszProperties, IID_IDBCreateSession), E_NOINTERFACE, E_UNEXPECTED);

	//[OPTIONAL]
	TEST2C_(GetDataSource(NULL, m_pwszProperties, IID_IConnectionPointContainer), S_OK, E_NOINTERFACE);
	TEST2C_(GetDataSource(NULL, m_pwszProperties, IID_IDBAsynchStatus), S_OK, E_NOINTERFACE);
	TEST2C_(GetDataSource(NULL, m_pwszProperties, IID_IDBDataSourceAdmin), S_OK, E_NOINTERFACE);
	TEST2C_(GetDataSource(NULL, m_pwszProperties, IID_ISupportErrorInfo), S_OK, E_NOINTERFACE);

	//[OPTIONAL] Only Available After Initialization
	TEST3C_(GetDataSource(NULL, m_pwszProperties, IID_IDBInfo), S_OK, E_NOINTERFACE, E_UNEXPECTED);	
	TEST3C_(GetDataSource(NULL, m_pwszProperties, IID_IPersistFile), S_OK, E_NOINTERFACE, E_UNEXPECTED);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - riid - Provider Specific interfaces - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_26()
{ 
	TBEGIN
	
	//Currently the Only provider that I know of that supports a provider 
	//specific DataSource interface is MSDASQL (IID_ISQLRequestDiagFields)
	//Since we really only interested that the DCM can return Provider Specific Interfaces
	//This should be sufficent when run against MSDASQL.
	TESTC_PROVIDER(PROVIDER_CLSID == CLSID_MSDASQL);

	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_ISQLRequestDiagFields), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Uninitialized DSO - complete init string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_27()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	IDBInitialize* pIDBInitialize2 = NULL;
	HRESULT hr = S_OK;

	//Create Uninitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IUnknown, (IUnknown**)&pIUnknown),S_OK)
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize2, CREATEDSO_SETPROPERTIES),S_OK)
	
	//Passing in an IUnknown interface, asking for a IDBInitialize interface
	//on return, should test if there really doing a QI...
	pIDBInitialize = (IDBInitialize*)pIUnknown;

	//GetDataSource (use the Default InitString)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Compare Properties Before Initialization
	TESTC(!IsInitialized(pIDBInitialize));
	TESTC(!IsInitialized(pIDBInitialize2));
	TESTC(CompareProperties(pIDBInitialize, pIDBInitialize2, IGNORE_OPTION_FOR_VTEMPTY));

	//Initialize should now succeed
	TESTC_(hr = InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE),S_OK);
	TESTC_(hr = InitializeDataSource(pIDBInitialize2, CREATEDSO_INITIALIZE),S_OK);

	//Compare Properties After Initialization
	TESTC(CompareProperties(pIDBInitialize, pIDBInitialize2));

CLEANUP:
	//Dump the properties on the DataSource...
	DisplayProperties(pIDBInitialize);

	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_RELEASE_DSO(pIDBInitialize2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Uninitialized DSO - init string with no props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_28()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = NULL;
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	
	//Create Uninitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IUnknown, (IUnknown**)&pIUnknown),S_OK)
	TESTC_(GetInitString(pIUnknown, PASSWORD_INCLUDED, &pwszInitString),S_OK);
	TESTC_(GetProperties(pIUnknown, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource 
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIUnknown, cPropSets, rgPropSets));
	
	//Initialize should only succeed if provider requires no init props...
	TESTC(IsUninitialized(pIUnknown));

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_FREE(pwszInitString);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Uninitialized DSO - init string with another provider
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_29()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	HRESULT hr = S_OK;
	
	//Create Uninitialized DSO - (do everything expect Initialize)
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)

	//Create Uninitialized DSO (of another provider!)
	TESTC_PROVIDER(CreateDiffProvider(NULL, IID_IUnknown, &pIUnknown, CREATEDSO_SETPROPERTIES)==S_OK);
	TESTC_(GetInitString(pIUnknown, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER), DB_E_MISMATCHEDPROVIDER);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Uninitialized DSO - init string with no provider in the init string specified
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_30()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	WCHAR* pwszValue = L"password";
	CLSID  clsidProv;
	BOOL bSettable = FALSE;

	//Create Uninitialized DSO (of another provider!)
	TESTC_PROVIDER(CreateDiffProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, 0, &clsidProv)==S_OK);

	//Test InitString
	//GetDataSource with no Provider Keyword and *ppDataSource != NULL on input
	//Normally it would default to MSDASQL if no provider keyword, but since a 
	//DataSource was passed in, it just sets the properties on the passed in 
	//DataSource.
	bSettable = SettableProperty(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, pIDBInitialize);
	if(bSettable)
	{
		pwszInitString = CreateString(L"%s = %s ;", GetStaticPropDesc(DBPROP_AUTH_PASSWORD), pwszValue);
	}
	else
	{
		pwszInitString = CreateString(L" ;");
	}
	
	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify the new DataSource is the same as our input, and has the property set...
	TESTC(VerifyDataSource(pIDBInitialize, clsidProv));
	COMPC(VerifyDataSource(pIDBInitialize, DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue), bSettable);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 Reference Count...
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Uninitialized DSO - init string with invalid ProgID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_31()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	
	//Create Uninitialized DSO - (everything expect Init)
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource 
	TESTC_(GetDataSource(NULL, L"Provider=MyCoolProv;", IID_IDBInitialize, (IUnknown**)&pIDBInitialize), REGDB_E_CLASSNOTREG);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));
	
	//Initialize should succeed 
	TESTC_(hr = InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Uninitialized DSO - init string with invalid Provider String
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_32()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	
	//Create Uninitialized DSO - (everything except Init)
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource 
	TESTC_(GetDataSource(NULL, L"Provider=;", IID_IDBInitialize, (IUnknown**)&pIDBInitialize), DB_E_BADINITSTRING);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));
	
	//Initialize should succeed 
	TESTC_(hr = InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Initailized DSO - Complete init string - DB_E_ERRORSOCCURRED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_33()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	
	//Create Initialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK)

	//GetDataSource - SetProperties should fail on an Initialized DSO
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), m_fReqInitProps ? DB_E_ERRORSOCCURRED : S_OK);

	//Initialize should fail - (already initialized) 
	TESTC(IsInitialized(pIDBInitialize));

	//Uninitialize...
	TESTC_(pIDBInitialize->Uninitialize(), S_OK);
	TESTC(IsUninitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Initailized DSO - Complete init string with no props
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_34()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszInitString = NULL;

	//Create Uninitialized DSO
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);
	TESTC_(SetInitProps(pIDBInitialize),S_OK);
	TESTC_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE),S_OK);

	//Get Currently set properties
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//Create an InitString with just the Provider keyword, no properties
	//We cant just rely upon GetInitString, since for non-DPO objects default
	//properties will be returned...
	SAFE_FREE(pwszInitString);
	pwszInitString = CreateString(L"Provider = %s", GetProgID());

	//GetDataSource - No Properties, should succeed...
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an open session
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_35()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IOpenRowset* pIOpenRowset = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	HRESULT hr = S_OK;
	
	//Create Initialized DSO - With open Session
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
	TESTC_(CreateNewSession(pIDBInitialize, IID_IOpenRowset, (IUnknown**)&pIOpenRowset),S_OK);

	//Create Initialized DSO (of another provider!)
	TESTC_PROVIDER(CreateDiffProvider(NULL, IID_IUnknown, &pIUnknown, CREATEDSO_SETPROPERTIES)==S_OK);
	TESTC_(GetInitString(pIUnknown, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource (the existing DataSource is released, even though it has an open session) 
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CLSCTX_INPROC_SERVER), DB_E_MISMATCHEDPROVIDER);

	//Verify Already Initialized
	TESTC(IsInitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE(pIOpenRowset);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Initailized DSO - Complete init string with no provider specified
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_36()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	WCHAR* pwszValue = L"HeyYou!";

	//Create Initialized DSO (of another provider!)
	TESTC_PROVIDER(CreateDiffProvider(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE)==S_OK);

	//Test InitString
	//GetDataSource with no Provider Keyword and *ppDataSource != NULL on input
	//Normally it would default to MSDASQL if no provider keyword, but since a 
	//DataSource was passed in, it just sets the properties on the passed in 
	//DataSource, but this DSO is already Initialized so SetProperties 
	//Should fail.  Also indicate that the Provider should change = TRUE
	pwszInitString = CreateString(L" %s = %s", GetStaticPropDesc(DBPROP_AUTH_PASSWORD), pwszValue);
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), DB_E_ERRORSOCCURRED);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 Reference Count...
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an invalid ProgID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_37()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	
	//Create Initialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource 
	TESTC_(GetDataSource(NULL, L"Provider=MyCoolProv;", IID_IDBInitialize, (IUnknown**)&pIDBInitialize), REGDB_E_CLASSNOTREG);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));
	
	//Initialize should fail - this is the old DataSource which is already initialized 
	TESTC(IsInitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - Initailized DSO - Complete init string of another provider with an invalid Provider String
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_38()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	HRESULT hr = S_OK;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	
	//Create Initialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource 
	TESTC_(GetDataSource(NULL, L";", IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));
	
	//Initialize should fail - this is the old DataSource which is already initialized 
	TESTC(IsInitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_CACHE_AUTHINFO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_39()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_CACHE_AUTHINFO, DBPROPSET_DBINIT, DBTYPE_BOOL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(40)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_ENCRYPT_PASSWORD
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_40()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_ENCRYPT_PASSWORD, DBPROPSET_DBINIT, DBTYPE_BOOL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(41)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_INTEGRATED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_41()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_INTEGRATED, DBPROPSET_DBINIT, DBTYPE_BSTR));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(42)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_MASK_PASSWORD
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_42()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_MASK_PASSWORD, DBPROPSET_DBINIT, DBTYPE_BOOL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(43)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_PASSWORD
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_43()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, DBTYPE_BSTR));

	//TODO - need to verify this is in an excrypted form...

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(44)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_PERSIST_ENCRYPTED
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_44()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_PERSIST_ENCRYPTED, DBPROPSET_DBINIT, DBTYPE_BOOL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(45)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_45()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, DBTYPE_BOOL));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(46)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_AUTH_USERID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_46()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_AUTH_USERID, DBPROPSET_DBINIT, DBTYPE_BSTR));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(47)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_ASYNCH
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_47()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_ASYNCH;

    if(!SettableProperty(dwPropertyID, DBPROPSET_DBINIT))
    {
        TESTB = TEST_SKIPPED;
        odtLog << "DBPROP_INIT_ASYNCH is not supported!\n";
        goto CLEANUP;
    }

	//Verify this Property
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4));

	//Verify Friendly Names
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Initialize"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"INITIALIZE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"initialize"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"  initialize  ;"));

	//Using Numeric Equivilent
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROPVAL_ASYNCH_INITIALIZE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROPVAL_ASYNCH_INITIALIZE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROPVAL_ASYNCH_INITIALIZE));

	//Oring Friendly Names
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Initialize | Initialize"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DBPROPVAL_ASYNCH_INITIALIZE, L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Initialize|Initialize"));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(48)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_DATASOURCE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_48()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, DBTYPE_BSTR));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(49)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_HWND
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_49()
{ 
	TBEGIN

	//Verify this Property (this is allowed in the InitString
	//According to spec should not be persisted...
	TESTC(VerifyGetDataSource(DBPROP_INIT_HWND, DBPROPSET_DBINIT, DBTYPE_I4));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(50)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_IMPERSONATION_LEVEL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_50()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_IMPERSONATION_LEVEL;

	//Verify this Property
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4));

	//Verify Friendly Names - Anonymous
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_ANONYMOUS,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Anonymous"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_ANONYMOUS,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" anonymous ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_ANONYMOUS,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"ANONYMOUS"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_ANONYMOUS,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_ANONYMOUS));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_ANONYMOUS,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_ANONYMOUS));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_ANONYMOUS,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_ANONYMOUS));

	//Verify Friendly Names - Identify
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IDENTIFY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Identify"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IDENTIFY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" identify ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IDENTIFY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"IDENTITY"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IDENTIFY,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_IDENTIFY));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IDENTIFY,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_IDENTIFY));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IDENTIFY,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_IDENTIFY));

	//Verify Friendly Names - Impersonate
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IMPERSONATE,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Impersonate"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IMPERSONATE,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" impersonate ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IMPERSONATE,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"IMPERSONATE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IMPERSONATE,	L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_IMPERSONATE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IMPERSONATE,	L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_IMPERSONATE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_IMPERSONATE,	L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_IMPERSONATE));

	//Verify Friendly Names - Delegate
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_DELEGATE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Delegate"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_DELEGATE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" delegate ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_DELEGATE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"DELEGATE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_DELEGATE,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_DELEGATE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_DELEGATE,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_DELEGATE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_IMP_LEVEL_DELEGATE,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_IMP_LEVEL_DELEGATE));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(51)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_LCID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_51()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_LCID, DBPROPSET_DBINIT, DBTYPE_I4));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(52)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_LOCATION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_52()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_LOCATION, DBPROPSET_DBINIT, DBTYPE_BSTR));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(53)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_MODE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_53()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_MODE;

	//Verify this Property
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4));

	//Verify Friendly Names - Read
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_READ,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Read"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_READ,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" read ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_READ,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"READ"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_READ,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_READ));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_READ,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_READ));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_READ,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_READ));

	//Verify Friendly Names - Write
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_WRITE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Write"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_WRITE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" write ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_WRITE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"WRITE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_WRITE,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_WRITE,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_MODE_WRITE,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_WRITE));

	//Verify Friendly Names - Share Deny Read | Share Exclusive
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Share Deny Read | Share Exclusive"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" share deny read  |  share exclusive ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"share DENY Read|Share EXCLUSIVE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE),		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE),		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE),		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_EXCLUSIVE));

	//Verify Friendly Names - Share Deny None | Share Deny Write
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Share Deny None | Share Deny Write"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" share deny none  |  share deny write ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"share DENY nonE|Share DENY WRITE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_WRITE));

	//Verify Friendly Names - Write | Share Deny Write
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Write | Share Deny Write"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" write  |  share deny write ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" WRITE|Share DENY WRITE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE),		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_WRITE | DB_MODE_SHARE_DENY_WRITE));

	//Verify Friendly Names - Read | ReadWrite | Write
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Read | ReadWrite | Write"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" read  |  readwrite | write; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE),		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" READ|readWRITE  |   WRITE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE),		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE),		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)(DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE),		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_MODE_READ | DB_MODE_READWRITE | DB_MODE_WRITE));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(54)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_PROMPT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_54()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_PROMPT;

	//Verify this Property
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2));

	//Verify Friendly Names - Prompt
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_PROMPT,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Prompt"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_PROMPT,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" prompt ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_PROMPT,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"PROMPT"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_PROMPT,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_PROMPT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_PROMPT,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_PROMPT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_PROMPT,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_PROMPT));

	//Verify Friendly Names - Complete
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETE,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Complete"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETE,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" completE ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETE,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"COMPLETE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETE,	L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_COMPLETE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETE,	L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_COMPLETE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETE,	L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_COMPLETE));

	//Verify Friendly Names - CompleteRequired
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETEREQUIRED,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"CompleteRequired"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETEREQUIRED,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" completERequireD ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETEREQUIRED,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"COMPLETERequired"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETEREQUIRED,	L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_COMPLETEREQUIRED));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETEREQUIRED,	L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_COMPLETEREQUIRED));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_COMPLETEREQUIRED,	L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_COMPLETEREQUIRED));

	//Verify Friendly Names - NoPrompt
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_NOPROMPT,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"NoPrompt"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_NOPROMPT,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" noprompt ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_NOPROMPT,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"noPROMPT"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_NOPROMPT,	L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_NOPROMPT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_NOPROMPT,	L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_NOPROMPT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I2,	(void*)DBPROMPT_NOPROMPT,	L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DBPROMPT_NOPROMPT));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(55)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_PROTECTION_LEVEL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_55()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_PROTECTION_LEVEL;

	//Verify this Property
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4));

	//Verify Friendly Names - None
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_NONE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"None"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_NONE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" none ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_NONE,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"NONE"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_NONE,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_NONE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_NONE,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_NONE));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_NONE,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_NONE));

	//Verify Friendly Names - Connect
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CONNECT,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Connect"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CONNECT,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" connect ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CONNECT,	L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"CONNECT"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CONNECT,	L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_CONNECT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CONNECT,	L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_CONNECT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CONNECT,	L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_CONNECT));

	//Verify Friendly Names - Call
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CALL,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Call"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CALL,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" call ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CALL,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"CALL"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CALL,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_CALL));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CALL,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_CALL));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_CALL,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_CALL));

	//Verify Friendly Names - Pkt
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Pkt"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" pkt ; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"pKt"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT));

	//Verify Friendly Names - Pkt Integrity
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_INTEGRITY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Pkt Integrity"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_INTEGRITY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" pkt integrity; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_INTEGRITY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"pKt  InTEGrity"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_INTEGRITY,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT_INTEGRITY));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_INTEGRITY,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT_INTEGRITY));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_INTEGRITY,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT_INTEGRITY));

	//Verify Friendly Names - Pkt Privacy
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_PRIVACY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Pkt Privacy"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_PRIVACY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L" pkt privacy; "));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_PRIVACY,		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"pKt  PRIVACY"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_PRIVACY,		L"Provider =%s; %s = %d", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT_PRIVACY));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_PRIVACY,		L"Provider =%s; %s = 0x%x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT_PRIVACY));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4,	(void*)DB_PROT_LEVEL_PKT_PRIVACY,		L"Provider =%s; %s = 0x%08x", GetProgID(), GetStaticPropDesc(dwPropertyID), DB_PROT_LEVEL_PKT_PRIVACY));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(56)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_PROVIDERSTRING
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_56()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	WCHAR* pwszValue = L"Driver= {Microsoft Text Driver (*.txt; *.csv)};Extenstions=txt,csv,asc,tab;DefaultDir=c:\\test";

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, DBTYPE_BSTR));

	//Interesting Provider Strings - Empty
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)L"",		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L""));
	
	//Interesting Provider Strings - ODBC Interesting Combinations
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)L"dsn=nwind; uid=odbc; pwd=odbc;",		L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"\"dsn=nwind; uid=odbc; pwd=odbc;\""));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)L"dsn=nwind",							L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L"dsn=nwind"));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)L"",										L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L""));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)L"",										L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L""));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)L"",										L"Provider =%s; %s = %s", GetProgID(), GetStaticPropDesc(dwPropertyID), L""));
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR,	(void*)pwszValue,								L"Provider =%s; %s = \"%s\"", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(57)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_TIMEOUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_57()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_TIMEOUT, DBPROPSET_DBINIT, DBTYPE_I4));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(58)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - DBPROP_INIT_GENERALTIMEOUT
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_58()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_GENERALTIMEOUT, DBPROPSET_DBINIT, DBTYPE_I4));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(59)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - All Properties (including Provider Specific) - together
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_59()
{ 
	TBEGIN
	ULONG i,cPropInfoSets = NULL;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;
	WCHAR* pwszInitString = CreateString(L" Provider = %s; ", GetProgID());
	IUnknown* pDataSource = NULL;
	HRESULT hr = S_OK;
	VARIANT VarTemp;
	VariantInit(&VarTemp);

	//GetPropertyInfo
	TESTC_(GetPropertyInfo(DBPROPSET_DBINITALL, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK);

	//Loop through all propsets, were only interested in non DBPROPSET_DBINIT sets
	for(i=0; i<cPropInfoSets; i++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[i];
		for(ULONG j=0; j<pPropInfoSet->cPropertyInfos; j++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[j];
			VARIANT* pVariant = &pPropInfo->vValues;

			//Need to Set a particualr value for this property
			//Since we are actually "saving" this in the Variant we can put whatever
			//we want and just compare it to the GetProperties call
			VariantClear(&VarTemp);
			VariantClear(pVariant);

			//Create some value, the first one is invalid, the second one is valid
			//Since the spec indicates only the second one is used...
			GetSomePropValue(&VarTemp, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, pPropInfo->vtType, FALSE);
			GetSomePropValue(pVariant, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, pPropInfo->vtType);
			
			//Add Temp Value, (duplicate testing)
			TESTC_(AppendToInitString(&pwszInitString, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, &VarTemp),S_OK);

			//Add The Final Value (the one we are verifying) to the InitString
			TESTC_(AppendToInitString(&pwszInitString, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, &pPropInfo->vValues),S_OK);
		}
	}

	//Now that we have an entire init string with all properties.
	//Get the Get the DataSource with these properties set
	TEST3C_(hr = GetDataSource(NULL, pwszInitString, IID_IUnknown, &pDataSource),S_OK, DB_E_ERRORSOCCURRED, E_FAIL);
	if(hr==S_OK)
	{
		TESTC(VerifyDataSource(pDataSource, PROVIDER_CLSID));
		
		//Now loop through all the Properties and make sure the properties are
		//set with the approaite values...
		for(i=0; i<cPropInfoSets; i++)
		{
			DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[i];
			for(ULONG j=0; j<pPropInfoSet->cPropertyInfos; j++)
			{
				DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[j];

				//Verify This property Value
				if(pPropInfo->dwFlags & DBPROPFLAGS_WRITE)
					TESTC(VerifyDataSource(pDataSource, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, DBTYPE_VARIANT, &pPropInfo->vValues));
			}
		}
	}
	else
	{
		//TODO need to verify Not Supported, Not Settable, BadValue...
		TESTC(pDataSource == NULL);
	}

CLEANUP:
	::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pDataSource);
	VariantClear(&VarTemp);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(60)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - All Properties (including Provider Specific) - seperate
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_60()
{ 
	TBEGIN
	ULONG i,cPropInfoSets = NULL;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;

	//GetPropertyInfo
	TESTC_(GetPropertyInfo(DBPROPSET_DBINITALL, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK);

	//Loop through all propsets, were only interested in non DBPROPSET_DBINIT sets
	for(i=0; i<cPropInfoSets; i++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[i];
		for(ULONG j=0; j<pPropInfoSet->cPropertyInfos; j++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[j];
			
			//Verify this Property
			if(TRUE!=VerifyGetDataSource(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, pPropInfo->vtType))
			{
				odtLog << "i="<<i<<"; j="<<j<<"; dwPropertyID: " << pPropInfo->dwPropertyID << "; DBType: " << GetDBTypeName(pPropInfo->vtType) << "\n";				                  
				TESTC(FALSE);
			}
		}
	}

CLEANUP:
	::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(61)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - Property specified more than once with diff values
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_61()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =%s; ", GetProgID());
	BOOL fDataSource	= SettableProperty(DBPROP_INIT_DATASOURCE,	DBPROPSET_DBINIT, pDBInitSC());
	BOOL fUserID		= SettableProperty(DBPROP_AUTH_USERID,		DBPROPSET_DBINIT, pDBInitSC());
	BOOL fPassword		= SettableProperty(DBPROP_AUTH_PASSWORD,	DBPROPSET_DBINIT, pDBInitSC());


	//This is pretty much covered in the seperate property sections, and is 
	//covered in the Provider Specific "all" case.  But we could come up with
	//some other interesting combination as well, like "interleived" combination 
	//of duplicates...
	
	//DBPROP_INIT_DATASOURCE
	if(fDataSource)
	{
		AppendString(&pwszInitString, L"Data Source = MSDASQL;");
		AppendString(&pwszInitString, L"Data Source = TestOSP;");
	}

	//DBPROP_AUTH_USERID
	if(fUserID)
	{
		AppendString(&pwszInitString, L"User ID = oledb1;");
		AppendString(&pwszInitString, L"User ID = oledb2;");
	}

	//DBPROP_AUTH_PASSWORD
	if(fPassword)
	{
		AppendString(&pwszInitString, L"Password = pass1;");
		AppendString(&pwszInitString, L"Password = pass2;");
	}

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	TESTC_(VerifyDataSource(pIUnknown, DBPROP_INIT_DATASOURCE,	DBPROPSET_DBINIT, DBTYPE_BSTR,	L"TestOSP"),	fDataSource);
	TESTC_(VerifyDataSource(pIUnknown, DBPROP_AUTH_USERID,		DBPROPSET_DBINIT, DBTYPE_BSTR,	L"oledb2"),		fUserID);
	TESTC_(VerifyDataSource(pIUnknown, DBPROP_AUTH_PASSWORD,	DBPROPSET_DBINIT, DBTYPE_BSTR,	L"pass2"),		fPassword);
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(62)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - Verify Case Insensitive
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_62()
{ 
	TBEGIN
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;

	//This is pretty much covered in the seperate property sections, and is 
	//covered in the Provider Specific "all" case.  But we could come up with
	//some other interesting combination as well...
	
	//Interesting Case Insensitive cases

	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(63)
//*-----------------------------------------------------------------------
// @mfunc Strings - Keywords - Specified more than once, second in error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_63()
{ 
	TBEGIN
	BOOL fPromptSettable	= SettableProperty(DBPROP_INIT_PROMPT,	DBPROPSET_DBINIT, pDBInitSC());
	BOOL fhWndSettable		= SettableProperty(DBPROP_INIT_HWND,	DBPROPSET_DBINIT, pDBInitSC());

	//This is pretty much covered in the seperate property sections, and is 
	//covered in the Provider Specific "all" case.  But we could come up with
	//some other interesting combination as well...
	
	TESTC_(VerifyGetDataSource(DBPROP_INIT_PROMPT,	DBPROPSET_DBINIT, DBTYPE_I2,	0, 	L"Provider =%s; %s = %s; %s = %s", GetProgID(), GetStaticPropDesc(DBPROP_INIT_PROMPT),	L"Complete", GetStaticPropDesc(DBPROP_INIT_PROMPT), L"HelloWorld!"),	!fPromptSettable);
	TESTC_(VerifyGetDataSource(DBPROP_INIT_HWND,	DBPROPSET_DBINIT, DBTYPE_I4, 	0, 	L"Provider =%s; %s = %s; %s = %s", GetProgID(), GetStaticPropDesc(DBPROP_INIT_HWND),		L"0", GetStaticPropDesc(DBPROP_INIT_HWND), L"Word!"),					!fhWndSettable);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(64)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Just Provider Keyword
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_64()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"   Provider = %s   ", GetProgID());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(65)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Not first item in the list
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_65()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"%s Provider =%s;", m_pwszProperties, GetProgID());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(66)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Versioned Provider ProgID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_66()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =%s.1", GetProgID());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(67)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Provider ProgID which contains a space
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_67()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IParseDisplayName* pIParseDisplayName = NULL;

	//CoCreate on MSDASQL Enumerator	
	HRESULT hr = CoCreateInstance(CLSID_MSDASQL_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_IParseDisplayName, (void**)&pIParseDisplayName);

	//Test InitString
	TESTC_(GetDataSource(NULL, L"Provider =MSDASQL Enumerator", IID_IUnknown, &pIUnknown), SUCCEEDED(hr) ? E_NOINTERFACE : REGDB_E_CLASSNOTREG);
	TESTC(pIUnknown == NULL);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE(pIParseDisplayName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(68)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Missing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_68()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	IDBInitialize* pIDBInitialize = NULL;

	//CoCreate on MSDASQL
	//Were are only CoCreating here, in case MSDASQL is not installed on the users
	//System, this way we know exactly what error to expect...
	TESTC_(CoCreateInstance(CLSID_MSDASQL, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, (void**)&pIDBInitialize), S_OK);

	//Test InitString
	//GetDataSource with no Provider Keyword and *ppDataSource = NULL on input
	TESTC_(GetDataSource(NULL, L"", IID_IUnknown, &pIUnknown), S_OK);
	if(pIUnknown)
	{
#ifdef _WIN64
		TESTC(VerifyDataSource(pIUnknown, CLSID_SQLOLEDB));
#else
		TESTC(VerifyDataSource(pIUnknown, CLSID_MSDASQL));
#endif
	}
	
CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(69)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Versioned Provider ProgID that doesn't exist, should fall back
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_69()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =%s.1", GetProgID());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(70)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Specified twice
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_70()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =SomeGarbage;Provider =%s", GetProgID());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(71)
//*-----------------------------------------------------------------------
// @mfunc Strings - Provider Keyword - Provider=; - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_71()
{ 
	TBEGIN

	//Test InitString
	TESTC_(GetDataSource(NULL, L"Provider=;"), DB_E_BADINITSTRING);
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(72)
//*-----------------------------------------------------------------------
// @mfunc Strings - Friendly Names - Allow integer equivilent
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_72()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =%s; ", GetProgID());
	
	//DBPROP_INIT_ASYNCH
	if(SettableProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Asynchronous Processing = 1;");

	//DBPROP_INIT_PROMPT
	if(SettableProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Prompt = 3;");

	//DBPROP_INIT_HWND
	if(SettableProperty(DBPROP_INIT_HWND, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Window Handle = 0;");
	
	//DBPROP_INIT_IMPERSONATION_LEVEL
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Impersonation Level = 2;");
		
	//DBPROP_INIT_PROTECTION_LEVEL
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Protection Level = 5;");

	//DBPROP_INIT_MODE
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Mode = 3;");

	//DBPROP_INIT_OLEDBSERVICES
	if(SettableProperty(DBPROP_INIT_OLEDBSERVICES, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"OLE DB Services = -1;");

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(73)
//*-----------------------------------------------------------------------
// @mfunc Strings - Friendly Names - Numeric Value out of range
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_73()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =%s; ", GetProgID());
	ULONG iSettableProps = 0;
	HRESULT hr;

	//This is throughly testing in individula properties, but just for
	//completness, and different senarios this variation is included...

	//DBPROP_INIT_ASYNCH
	if(SettableProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"Asynchronous Processing = 2;");
		iSettableProps++;
	}

	//DBPROP_INIT_PROMPT
	if(SettableProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"Prompt = 0;");
		iSettableProps++;
	}

	//DBPROP_INIT_HWND
	if(SettableProperty(DBPROP_INIT_HWND, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"Window Handle = 0;");
		iSettableProps++;
	}
	
	//DBPROP_INIT_IMPERSONATION_LEVEL
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"Impersonation Level = 100;");
		iSettableProps++;
	}
		
	//DBPROP_INIT_PROTECTION_LEVEL
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"Protection Level = 100;");
		iSettableProps++;
	}

	//DBPROP_INIT_MODE
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"Mode = 14;");
		iSettableProps++;
	}

	//DBPROP_INIT_OLEDBSERVICES
	if(SettableProperty(DBPROP_INIT_OLEDBSERVICES, DBPROPSET_DBINIT, pDBInitSC()))
	{
		AppendString(&pwszInitString, L"OLE DB Services = 1000;");
//		iSettableProps++;
	}

	//Test InitString
	hr = GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown);
	if (iSettableProps == 0)
	{
		TESTC_(hr, S_OK);
	}
	else
	{
		TEST2C_(hr, DB_E_ERRORSOCCURRED, DB_E_BADINITSTRING);
	}

CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(74)
//*-----------------------------------------------------------------------
// @mfunc Strings - Friendly Names - oring Friendly Names
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_74()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider =%s; ", GetProgID());
	
	//DBPROP_INIT_ASYNCH
	if(SettableProperty(DBPROP_INIT_ASYNCH, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Asynchronous Processing = Initialize;");

	//DBPROP_INIT_PROMPT
	if(SettableProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Prompt = CompleteRequired;");

	//DBPROP_INIT_HWND
	if(SettableProperty(DBPROP_INIT_HWND, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Window Handle = 0;");
	
	//DBPROP_INIT_IMPERSONATION_LEVEL
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Impersonation Level = Idenify;");
		
	//DBPROP_INIT_PROTECTION_LEVEL
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Protection Level = Connect;");

	//DBPROP_INIT_MODE
	if(SettableProperty(DBPROP_INIT_IMPERSONATION_LEVEL, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"Mode = Read | Write | ReadWrite;");

	//DBPROP_INIT_OLEDBSERVICES
	if(SettableProperty(DBPROP_INIT_OLEDBSERVICES, DBPROPSET_DBINIT, pDBInitSC()))
		AppendString(&pwszInitString, L"OLE DB Services = ResourcePooling | TxnEnlistment | EnableAll;");

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(75)
//*-----------------------------------------------------------------------
// @mfunc Strings - Friendly Names - oring same Friendly Name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_75()
{ 
	TBEGIN

	//DBPROP_INIT_MODE
	TESTC(VerifyGetDataSource(DBPROP_INIT_MODE, DBPROPSET_DBINIT, DBTYPE_I4, (void*)(DB_MODE_SHARE_DENY_READ | DB_MODE_SHARE_DENY_NONE | DB_MODE_SHARE_DENY_READ), 
		L"Provider =%s; Mode =Share Deny Read | Share Deny None |  Share Deny Read", GetProgID()));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(76)
//*-----------------------------------------------------------------------
// @mfunc Strings - Friendly Names - wrong Friendly Names
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_76()
{ 
	TBEGIN
	WCHAR* pwszInitString = CreateString(
		L"Provider =%s; "
		L"Prompt =Initialize; "
		L"Mode = Complete"
		,GetProgID()
		);

	//DBPROP_INIT_MODE
	BOOL fModeSettable = SettableProperty(DBPROP_INIT_MODE, DBPROPSET_DBINIT, pDBInitSC());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString), fModeSettable ? DB_E_BADINITSTRING : DB_E_ERRORSOCCURRED);
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(77)
//*-----------------------------------------------------------------------
// @mfunc Strings - Friendly Names - oring values for nonMode props - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_77()
{ 
	TBEGIN
	WCHAR* pwszInitString = CreateString(
		L"Provider =%s; "
		L"Prompt =Prompt | Prompt; "
		,GetProgID()
		);

	//DBPROP_INIT_PROMPT
	BOOL fPromptSettable = SettableProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, pDBInitSC());

	//Test InitString
	TESTC_(GetDataSource(NULL, pwszInitString), fPromptSettable ? DB_E_BADINITSTRING : DB_E_ERRORSOCCURRED);
	
CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(78)
//*-----------------------------------------------------------------------
// @mfunc Strings - Property Values - Contains same value as keyword
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_78()
{ 
	TBEGIN

	DBPROPID dwPropertyID = DBPROP_AUTH_PASSWORD;
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, GetStaticPropDesc(dwPropertyID),	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), GetStaticPropDesc(dwPropertyID)));

	dwPropertyID = DBPROP_AUTH_USERID;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, GetStaticPropDesc(dwPropertyID),	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), GetStaticPropDesc(dwPropertyID)));

	dwPropertyID = DBPROP_INIT_DATASOURCE;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, GetStaticPropDesc(dwPropertyID),	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), GetStaticPropDesc(dwPropertyID)));
	
	dwPropertyID = DBPROP_INIT_LOCATION;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, GetStaticPropDesc(dwPropertyID),	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), GetStaticPropDesc(dwPropertyID)));

	dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, GetStaticPropDesc(dwPropertyID),	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), GetStaticPropDesc(dwPropertyID)));

	dwPropertyID = DBPROP_INIT_CATALOG;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, GetStaticPropDesc(dwPropertyID),	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), GetStaticPropDesc(dwPropertyID)));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(79)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Last item doesn't contain semicolon
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_79()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROMPT;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_I2, (void*)DBPROMPT_PROMPT,	L"Provider =%s; %s = %s ", GetProgID(), GetStaticPropDesc(dwPropertyID), L"Prompt"));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(80)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Provider=progid with no trailing semi
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_80()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROMPT;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_I2, (void*)DBPROMPT_PROMPT,	L"  %s = %s ; Provider =%s ", GetStaticPropDesc(dwPropertyID), L"Prompt", GetProgID()));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(81)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Double semi
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_81()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROMPT;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_I2, (void*)DBPROMPT_PROMPT,	L"; ; ;;  %s = %s ;; Provider =%s ;;;;;; ", GetStaticPropDesc(dwPropertyID), L"Prompt", GetProgID()));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(82)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Semicolon in keyword
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_82()
{ 
	return TEST_SKIPPED;

} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(83)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Semicolon in Value - (quoted)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_83()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	WCHAR* pwszValue = L"Data Source=localAccess; User ID=odbc; Password = odbc;";
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"%s = \"%s\"; Provider =%s;", GetStaticPropDesc(dwPropertyID), pwszValue, GetProgID()));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(84)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Semicolon following keyword Data Source =;
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_84()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_EMPTY, NULL,	L"%s=; Provider =%s;", GetStaticPropDesc(dwPropertyID), GetProgID()));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(85)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Semicolon following keyword Extended Properties;
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_85()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	//Test InitString
	//Semicolon following Keyword (without equal sign)
	//"Extended Properties; Provider=MSDASQL"

	GUID guidPropertySet = DBPROPSET_DBINIT;
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	DBTYPE wType = DBTYPE_BSTR;
	WCHAR* pwszInitString = CreateString(L"%s; Provider=%s", GetStaticPropDesc(dwPropertyID), GetProgID());
	//TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, pwszInitString,	L"%s;", pwszInitString));

	//Call without an existing DSO
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify the Returned DataSource has the correct property set
	QTESTC(VerifyDataSource(pIDBInitialize, dwPropertyID, guidPropertySet, wType, pwszInitString));
	QTESTC(VerifyPassword(pIDBInitialize, TRUE, pwszInitString, FALSE));

	//Might as well do some bonus testing of an Init String with 
	//this particular property
	QTESTC(VerifyInitString(dwPropertyID, guidPropertySet, wType, pwszInitString));

	//Now that we have all Properties set...
	//Not a "exact" comparision (FALSE), since we called GetDataSource
	//From the string, which could return additional properties
	TESTC(VerifyInitString(pIDBInitialize, pwszInitString, FALSE));

	SAFE_RELEASE_DSO(pIDBInitialize);
	
	//Now call with an existing DSO
	TESTC_(CreateNewDSO(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), DB_E_ERRORSOCCURRED);
	
CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(86)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - Middle item missing semi - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_86()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;

	//Test InitString
	//Create a string:
	//"Provider =MSDASQL; Data Source = odbc = Location = server"
	//Since a semicolon ends a string the entire DataSource value should be:
	//"odbc = Location = server"
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = CreateString(L"%s %s =%s", L"odbc", GetStaticPropDesc(DBPROP_INIT_LOCATION), L"server");
	WCHAR* pwszInitString = CreateString(L" Provider =%s; %s =%s", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);

	//DBPROP_INIT_DATASOURCE
	TESTC(VerifyDataSource(pIUnknown, DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue));
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_FREE(pwszValue);
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(87)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved SemiColon - SemiColon in value - not qouted - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_87()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszDataSource = L"Driver=SQLServer";
	WCHAR* pwszProviderString = L"DataBase= pubs";
	WCHAR* pwszInitString = CreateString(L" Provider =%s; %s =%s;%s", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszDataSource, pwszProviderString);
	
	//Create String 
	//"Provider=MSDASQL; Data Souirce = Driver=SQLServer; DataBase=pubs;"
	//So Data Source should end up with a value of "Drvier=SQLServer"
	//And Extended Properties should end up with a value of "DataBase=pubs"

	BOOL fDataSource		= SettableProperty(DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, pDBInitSC());
	BOOL fProviderString	= SettableProperty(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, pDBInitSC());

	//GetDataSource
	if(fDataSource && fProviderString)
	{
		//GetDataSource
		TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);

		//DBPROP_INIT_DATASOURCE
		TESTC(VerifyDataSource(pIUnknown, DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszDataSource));

		//DBPROP_INIT_PROVIDERSTRING
		TESTC(VerifyDataSource(pIUnknown, DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszProviderString));
		TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	}
	else
	{
		//GetDataSource
		TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), DB_E_ERRORSOCCURRED);
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(88)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved EqualSign - In Keyword - Doubled
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_88()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Data Source===Driver=SQLServer ;DataBase= pubs";
	TESTC(VerifyGetDataSource(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s;%s", GetProgID(), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(89)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved EqualSign - In Value - not Doubled
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_89()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver=SQLServer";

	//DBPROP_INIT_DATASOURCE
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(90)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved EqualSign - In Value - Doubled - should count as 2
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_90()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver==SQLServer";
	
	//DBPROP_INIT_DATASOURCE
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s =%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(91)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - In Keyword - not doubled - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_91()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver==SQLServer";
	WCHAR* pwszExtValue = L"DataBase=pubs";
	WCHAR* pwszActualValue = L"Data Source ==Driver==SQLServer;DataBase=pubs";
	TESTC(VerifyGetDataSource(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszActualValue,	L"Provider =%s; %s ==%s;%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue, pwszExtValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(92)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - Completly missing - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_92()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver==SQLServer";
	WCHAR* pwszExtValue = L"DataBase=pubs";
	WCHAR* pwszActualValue = L"Data Source Driver==SQLServer;DataBase=pubs";
	TESTC(VerifyGetDataSource(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszActualValue,	L"Provider =%s; %s %s;%s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue, pwszExtValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(93)
//*-----------------------------------------------------------------------
// @mfunc Strings - Reserved EqualSign - In Keyword - not DoubledStrings - Reserved EqualSign - Keyword==value; - error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_93()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"== Driver== S QLServer;  DataBase =pubs";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s=\"%s\"; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(94)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before/After keywords
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_94()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	WCHAR* pwszValue = L"Driver=SQLServer DataBase=pubs";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(95)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before/After equal sign
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_95()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver=SQLServer DataBase=pubs";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(96)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before/After semi
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_96()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver=SQLServer DataBase=pubs";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s ; %s = %s ; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(97)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before/After value- ignored
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_97()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"Driver=SQLServer";
	WCHAR* pwszInitString = CreateString(L" Provider =%s; %s =  %s  ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue);
	
	//Create String 
	//"Provider=MSDASQL; Data Source =   Driver=SQLServer;  DataBase=pubs   ; "
	//So Data Source should end up with a value of "Drvier=SQLServer"
	//And Extended Properties should end up with a value of "  DataBase=pubs   ;"
	//Since spaces are keep arround unknown values...

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown), S_OK);

	//DBPROP_INIT_DATASOURCE
	TESTC(VerifyDataSource(pIUnknown, DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue));
	TESTC(VerifyDataSource(pIUnknown, PROVIDER_CLSID));
	
CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(98)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before/After value - quoted - retained
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_98()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"   Driver=SQLServer DataBase=pubs  ";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s = \"%s\"; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(99)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - in value - retained
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_99()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"   Driver=SQLServer    DataBase=pubs  ";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s = \"%s\"  ; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(100)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before string, After string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_100()
{ 
	TBEGIN

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"   Driver=SQLServer;DataBase=pubs  ";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s = \"%s\"  ; ", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(101)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - Before/After orable token (|)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_101()
{ 
	TBEGIN
	
	//Already covered in Individual property senarios, but will do here again
	//just for completeness...

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_MODE;
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4, (void*)(DB_MODE_READ | DB_MODE_WRITE),	L"Provider =%s; %s = %s;", GetProgID(), GetStaticPropDesc(dwPropertyID), L"   Read   |   Write  "));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(102)
//*-----------------------------------------------------------------------
// @mfunc Strings - Spaces - In Keyword that already has spaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_102()
{ 
	TBEGIN
	
	//Already covered in Individual property senarios, but will do here again
	//just for completeness...

	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_MODE;
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_I4, (void*)(DB_MODE_SHARE_DENY_NONE | DB_MODE_READ),	L"Provider =%s; %s = %s;", GetProgID(), GetStaticPropDesc(dwPropertyID), L" Share Deny None   |  Read"));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(103)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - All value single quotes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_103()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_AUTH_USERID;
	WCHAR* pwszValue = L"odbc";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider ='%s'; %s = '%s' ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(104)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - All value double quotes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_104()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_AUTH_USERID;
	WCHAR* pwszValue = L"odbc";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =\"%s\"; %s = \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(105)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Keywords single and Double quoted
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_105()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_AUTH_USERID;
	WCHAR* pwszValue = L"\"User ID\"= odbc ; 'User ID' = odbc ";
	WCHAR* pwszExpectedValue = L"\"User ID\"= odbc ;'User ID' = odbc ";
	TESTC(VerifyGetDataSource(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszExpectedValue,	L"Provider =%s; %s;", GetProgID(), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(106)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Embedding Single
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_106()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	WCHAR* pwszValue = L"DSN='datasource'; UID='odbc'; PASSWORD=ed's";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(107)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Embedding Double
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_107()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"DSN=\"\"datasource\"\"; UID='odbc'; PASSWORD=ed's";
	WCHAR* pwszActualValue = L"DSN=\"datasource\"; UID='odbc'; PASSWORD=ed's";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszActualValue,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(108)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Both Embedding Single and Double starting with Single
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_108()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"DSN=\"datasource\"; UID=''odbc''; PASSWORD=ed''s";
	WCHAR* pwszActualValue = L"DSN=\"datasource\"; UID='odbc'; PASSWORD=ed's";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszActualValue,	L"Provider =%s; %s= '%s' ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(109)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Both Embedding Single and Double starting with Double
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_109()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	WCHAR* pwszValue = L"DSN=\"\"datasource\"\"; UID='odbc'; PASSWORD=ed's";
	WCHAR* pwszActualValue = L"DSN=\"datasource\"; UID='odbc'; PASSWORD=ed's";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszActualValue,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(110)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Just a pain!
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_110()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	WCHAR* pwszValue = L"\"\"\"\"DSN=ed's 'n\"\"e\"\"w' 'w\"\"a\"\"ve'\"\"";
	WCHAR* pwszActualValue = L"\"\"DSN=ed's 'n\"e\"w' 'w\"a\"ve'\"";
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszActualValue,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(111)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Uneven quotes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_111()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"DSN=Password; \"";
	TESTC(!VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(112)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Doubled Quotes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_112()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue = L"   \"\"c:\\Cubes\\bank.cub\"\"   ";
	TESTC(!VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue,	L"Provider =%s; %s= %s ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue));
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(113)
//*-----------------------------------------------------------------------
// @mfunc Strings - Quotes - Tripled Quotes
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_113()
{ 
	TBEGIN
	
	//Test InitString
	DBPROPID dwPropertyID = DBPROP_INIT_DATASOURCE;
	WCHAR* pwszValue1 = L"   \'\'\'c:\\Cubes\\bank.cub\'\'\'   ";
	WCHAR* pwszValue2 = L"   \"\"\"c:\\Cubes\\bank.cub\"\"\"   ";

	//This should succeed (uneven number of quote - not the same as outside quote)
	TESTC(VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue1,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue1));
	
	//This should fail (uneven number of qoutes - with same outside quote)
	TESTC(!VerifyGetDataSource(dwPropertyID, DBPROPSET_DBINIT, DBTYPE_BSTR, pwszValue2,	L"Provider =%s; %s= \"%s\" ;", GetProgID(), GetStaticPropDesc(dwPropertyID), pwszValue2));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(114)
//*-----------------------------------------------------------------------
// @mfunc Strings - Special Chars - DBCS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_114()
{ 
	TBEGIN
	WCHAR wszValue[MAX_QUERY_LEN+1];
	wszValue[0] = L'\0';

	//For all "String" properties create a unique DBCS String for the property,
	//Verify GetDataSource can handle it and sets it appropiately...

	//Create Locale Object
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );
	DBPROPID dwPropertyID = DBPROP_AUTH_PASSWORD;

	//TODO - this crashes the oracle server and blocks automation, waiting for fix...
	TESTC_PROVIDER(FALSE);

	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_AUTH_USERID;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_DATASOURCE;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));
	
	dwPropertyID = DBPROP_INIT_LOCATION;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_AUTH_INTEGRATED;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_CATALOG;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(115)
//*-----------------------------------------------------------------------
// @mfunc Strings - Special Chars - All Printable Chars
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_115()
{ 
	TBEGIN
	WCHAR* wszValue = L"={*&()@`$#^<>:,.:-+=_!~}|/:/'%";

	DBPROPID dwPropertyID = DBPROP_AUTH_PASSWORD;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_AUTH_USERID;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_DATASOURCE;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));
	
	dwPropertyID = DBPROP_INIT_LOCATION;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_AUTH_INTEGRATED;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(116)
//*-----------------------------------------------------------------------
// @mfunc Strings - Special Chars - non-printable
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_116()
{ 
	TBEGIN
	WCHAR* wszValue = L"\a\b\f\n\r\t\v\'\\\x16\x17\x18\x19";

	DBPROPID dwPropertyID = DBPROP_AUTH_PASSWORD;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_AUTH_USERID;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_DATASOURCE;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));
	
	dwPropertyID = DBPROP_INIT_LOCATION;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

	dwPropertyID = DBPROP_AUTH_INTEGRATED;
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), wszValue));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(117)
//*-----------------------------------------------------------------------
// @mfunc Strings - Special Chars - Localized Property Descriptions
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_117()
{ 
	TBEGIN
	BSTR bstrValue = NULL;

	//For all "String" properties create a unique DBCS String for the property,
	//Verify GetDataSource can handle it and sets it appropiately...

	//Create Locale Object
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );

	DBPROPID dwPropertyID = DBPROP_AUTH_PASSWORD;
	TESTC_(LocaleInfo.LocalizeString(GetStaticPropDesc(dwPropertyID), &bstrValue),S_OK);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, bstrValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), bstrValue));
	SAFE_SYSFREE(bstrValue);

	dwPropertyID = DBPROP_AUTH_USERID;
	TESTC_(LocaleInfo.LocalizeString(GetStaticPropDesc(dwPropertyID), &bstrValue),S_OK);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, bstrValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), bstrValue));
	SAFE_SYSFREE(bstrValue);

	dwPropertyID = DBPROP_INIT_DATASOURCE;
	TESTC_(LocaleInfo.LocalizeString(GetStaticPropDesc(dwPropertyID), &bstrValue),S_OK);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, bstrValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), bstrValue));
	SAFE_SYSFREE(bstrValue);
	
	dwPropertyID = DBPROP_INIT_LOCATION;
	TESTC_(LocaleInfo.LocalizeString(GetStaticPropDesc(dwPropertyID), &bstrValue),S_OK);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, bstrValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), bstrValue));
	SAFE_SYSFREE(bstrValue);

	dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	TESTC_(LocaleInfo.LocalizeString(GetStaticPropDesc(dwPropertyID), &bstrValue),S_OK);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, bstrValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), bstrValue));
	SAFE_SYSFREE(bstrValue);

	dwPropertyID = DBPROP_AUTH_INTEGRATED;
	TESTC_(LocaleInfo.LocalizeString(GetStaticPropDesc(dwPropertyID), &bstrValue),S_OK);
	TESTC(VerifyGetDataSource(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, bstrValue,	L"Provider =%s; %s = %s; ", GetProgID(), GetStaticPropDesc(dwPropertyID), bstrValue));
	SAFE_SYSFREE(bstrValue);


CLEANUP:
	SAFE_SYSFREE(bstrValue);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(118)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - GetDataSource from 3 threads - each asking for all diff interfaces, all dealing with the same provider
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_118()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);
	WCHAR* pwszInitString = NULL;

	//Obtain InitString
	GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszInitString);

	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, pwszInitString, NULL, (void*)S_OK };

	//Create Threads
	CREATE_THREADS(Thread_GetDataSource, &T1Arg);

	START_THREADS();
	END_THREADS();	

	//Make sure nothing changed after the threads existed...
	//TODO

//CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(119)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - GetDataSource from 3 threads - each asking for all diff interfaces, all same provider, with an error
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_119()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	WCHAR* pwszInitString = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	
	//Create Uninitialized DSO
	CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize);

	//Obtain InitString
	GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszInitString);

	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, pwszInitString,	pIDBInitialize, (void*)S_OK };
	THREADARG T2Arg = { this, L"Provider=goo",	pIDBInitialize, (void*)REGDB_E_CLASSNOTREG};
	THREADARG T3Arg = { this, pwszInitString,	NULL,			(void*)S_OK };

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_GetDataSource, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_GetDataSource, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetDataSource, &T3Arg);

	START_THREADS();
	END_THREADS();	

	//Make sure nothing changed after the threads existed...
	//TODO

//CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(120)
//*-----------------------------------------------------------------------
// @mfunc Keywords - DBPROP_INIT_CATALOG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_120()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_CATALOG, DBPROPSET_DBINIT, DBTYPE_BSTR));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(121)
//*-----------------------------------------------------------------------
// @mfunc Keywords - DBPROP_INIT_OLEDBSERVICES
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_121()
{ 
	TBEGIN

	//Verify this Property
	TESTC(VerifyGetDataSource(DBPROP_INIT_OLEDBSERVICES, DBPROPSET_DBINIT, DBTYPE_I4));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(122)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Provider hidden properties set before
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_122()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider=%s", GetProgID());

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Set provider hidden properties...
	TESTC_PROVIDER(SetHiddenInitProperties(pIDBInitialize)==S_OK);

	//GetDataSource (with no properties)
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Initialize - should fail due to hidden property
	TEST3C_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), DB_E_ERRORSOCCURRED, E_FAIL, DB_SEC_E_AUTH_FAILED);
	SAFE_RELEASE_DSO(pIDBInitialize);

	//GetDataSource (with init properties)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);
	TESTC_(SetHiddenInitProperties(pIDBInitialize), S_OK);

	//Initialize - should fail due to hidden property
	TEST3C_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), DB_E_ERRORSOCCURRED, E_FAIL, DB_SEC_E_AUTH_FAILED);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);	
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(123)
//*-----------------------------------------------------------------------
// @mfunc Parameters - Provider hidden properties set after
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetDataSource::Variation_123()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = CreateString(L"Provider=%s", GetProgID());

	//CreateDBInstance
	TESTC_(CreateDBInstance(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//GetDataSource (with no properties)
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Set provider hidden properties...
	TESTC_PROVIDER(SetHiddenInitProperties(pIDBInitialize)==S_OK);

	//Initialize - should fail due to hidden property
	TEST3C_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), DB_E_ERRORSOCCURRED, E_FAIL, DB_SEC_E_AUTH_FAILED);
	SAFE_RELEASE_DSO(pIDBInitialize);

	//GetDataSource (with init properties)
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);
	TESTC_(SetHiddenInitProperties(pIDBInitialize), S_OK);

	//Initialize - should fail due to hidden property
	TEST3C_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), DB_E_ERRORSOCCURRED, E_FAIL, DB_SEC_E_AUTH_FAILED);

	//Set provider hidden properties...
	TESTC_PROVIDER(SetHiddenInitProperties(pIDBInitialize)==S_OK);

	//Initialize - should fail due to hidden property
	TEST3C_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE), DB_E_ERRORSOCCURRED, E_FAIL, DB_SEC_E_AUTH_FAILED);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);	
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCGetDataSource::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCGetInitializationString)
//*-----------------------------------------------------------------------
//| Test Case:		TCGetInitializationString - Test IDataInitialize::GetInitializationString
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCGetInitializationString::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 







// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - Verify password format
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_1()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - pDataSource == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_2()
{ 
	TBEGIN
	WCHAR* pwszInitString = NULL;
		
	//GetInitString 
	TESTC_(GetInitString(NULL, PASSWORD_EXCLUDED, &pwszInitString), E_INVALIDARG);

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - ppwszInitString == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_3()
{ 
	TBEGIN
		
	//GetInitString 
	TESTC_(pDataInit()->GetInitializationString(pIDBInit(), FALSE, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - pDataSource = non-datasource interface (ie: Session object)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_4()
{ 
	TBEGIN
	WCHAR* pwszInitString = NULL;
		
	//GetInitString - (non-datasource object)
	TESTC_(GetInitString(pDataInit(), PASSWORD_INCLUDED, &pwszInitString), E_NOINTERFACE);

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END








// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - pDataSource = Enumerator Object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_5()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = NULL;
	
	//Create Enumerator Object
	//May not be installed on the users system...
	TESTC_PROVIDER(CoCreateInstance(CLSID_MSDASQL_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnknown)==S_OK);

	//GetInitString - (non-datasource object)
	TESTC_(GetInitString(pIUnknown, PASSWORD_INCLUDED, &pwszInitString), E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - S_OK - pDataSource being different TDataSource interfaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_6()
{ 
	TBEGIN
	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = NULL;
	
	//Obtain the Datasource interfaces...
	ULONG cInterfaces = 0;
	INTERFACEMAP* rgInterfaces = NULL;
	GetInterfaceArray(DATASOURCE_INTERFACE, &cInterfaces, &rgInterfaces);

	//GetInitString - (all TDataSource interfaces)
	for(ULONG i=0; i<cInterfaces; i++)
	{
		//Obtain the Correct DataSource Interface...
		if(rgInterfaces[i].fMandatory)
		{
			TESTC_(QI(pIDBInit(), *(rgInterfaces[i].pIID), (void**)&pIUnknown),S_OK);
		}
		else
		{
			TEST2C_(QI(pIDBInit(), *(rgInterfaces[i].pIID), (void**)&pIUnknown),S_OK,E_NOINTERFACE);
		}
		
		if(pIUnknown)
		{
			//Try to obtain the InitString from this Interface
			TESTC_(GetInitString(pIUnknown, PASSWORD_INCLUDED, &pwszInitString), S_OK);
			SAFE_RELEASE_DSO(pIUnknown);
			SAFE_FREE(pwszInitString);
		}
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - S_OK - fIncludePassword == 2 - assumming to be true
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_7()
{ 
	TBEGIN
	WCHAR* pwszInitString = NULL;
	WCHAR* pwszInitString2 = NULL;
		
	//GetInitString - 
	TESTC_(GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszInitString), S_OK);

	//GetInitString - 
	TESTC_(GetInitString(pIDBInit(), 2, &pwszInitString2), S_OK);

	//Make sure the "2" is interpreted as "TRUE" - include password
	TESTC(wcscmp(pwszInitString, pwszInitString2)==0);

CLEANUP:
	SAFE_FREE(pwszInitString);
	SAFE_FREE(pwszInitString2);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Sequence - CreateDBInstance, GetInitString, GetDataSource, verify uninitialized and no props sets
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_8()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszInitString = NULL;

	//Create Uninitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource - No Properties, should succeed...
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));

	//Verify not-Initialized
	TESTC(IsUninitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Sequence - CreateDBInstance, Set required properties, GetInitString, GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_9()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszInitString = NULL;

	//Create Uninitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)

	//Verify not-Initialized
	TESTC(!IsInitialized(pIDBInitialize));

	//Obtain currently set properties
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);
	TESTC_(GetProperties(pIDBInitialize, 0, NULL, &cPropSets, &rgPropSets),S_OK);

	//GetDataSource - No Properties, should succeed...
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify there is no change in properties
	TESTC(CompareProperties(pIDBInitialize, cPropSets, rgPropSets));

	//Verify not-Initialized
	TESTC(!IsInitialized(pIDBInitialize));

	//Initialize
	TESTC_(InitializeDataSource(pIDBInitialize,CREATEDSO_INITIALIZE),S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordSaved - GetInitString on Initialized DSO, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_10()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;

	//GetInitString on Initialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Initialize
	TESTC_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE),S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordSaved - GetInitString on Uninitialized DSO (with no password set), GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_11()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;

	//Uninitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Initialize
	TESTC(IsUninitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordSaved - GetInitString on Uninitialized DSO (with password set), GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_12()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;

	//Uninitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Initialize
	TESTC(IsUninitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on Initialized DSO, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_13()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;

	//Initialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_EXCLUDED, &pwszInitString),S_OK);

	//Initialize
	TESTC(IsInitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on Initialized DSO, GetDataSource, Set Password, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_14()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;

	//Initialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_EXCLUDED, &pwszInitString),S_OK);

	//Initialize
	TESTC(IsInitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on Uninitialized DSO (with no password), GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_15()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;

	//UnInitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_EXCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Initialize
	TESTC(IsUninitialized(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on Uninitialized DSO (with password), GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_16()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	WCHAR* pwszValue = NULL;

	//UnInitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_EXCLUDED, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Initialize
	TESTC(IsUninitialized(pIDBInitialize));
	
	//The only time this should fail is running against a provider that
	//uses the ODBC connection string PWD for password.  This will be stripped
	//out when PASSWORD_EXCLUDED is specified.  
	if(GetModInfo()->GetStringKeywordValue(m_pwszProperties, GetStaticPropDesc(DBPROP_INIT_PROVIDERSTRING), &pwszValue) && FindSubString(pwszValue, L"PWD"))
	{
		TEST2C_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE),S_OK,DB_SEC_E_AUTH_FAILED);
	}
	else
	{
		//Otherwise this will succeed, since the PASSWORD property was
		//already set on the passed in DSO, and GetDataSource just won't set it again.
		TESTC_(InitializeDataSource(pIDBInitialize, CREATEDSO_INITIALIZE),S_OK);
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	SAFE_FREE(pwszValue);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetInitString with SENSITIVE_AUTHINFO = TRUE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_17()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	ULONG  cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//UnInitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	
	//DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO = TRUE
	::SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)VARIANT_TRUE, DBTYPE_BOOL);
	SetProperties(pIDBInitialize, cPropSets, rgPropSets);

	//Verify GetInitString - Password Included
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);
	SAFE_FREE(pwszInitString);

	//Verify GetInitString - Password Excluded
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_EXCLUDED, &pwszInitString),S_OK);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Sequence - GetInitString with SENSITIVE_AUTHINFO = FALSE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_18()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	WCHAR* pwszInitString = NULL;
	ULONG  cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//UnInitialized DSO
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK)
	
	//DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO = FALSE
	::SetProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, &cPropSets, &rgPropSets, (void*)VARIANT_FALSE, DBTYPE_BOOL);
	SetProperties(pIDBInitialize, cPropSets, rgPropSets);

	//Verify GetInitString - Password Included
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_INCLUDED, &pwszInitString),S_OK);
	SAFE_FREE(pwszInitString);

	//Verify GetInitString - Password Excluded
	TESTC_(GetInitString(pIDBInitialize, PASSWORD_EXCLUDED, &pwszInitString),S_OK);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE_DSO(pIDBInitialize);  //Verify 0 refcount
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Properties - REQUIRED Init Properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_19()
{ 
	//All individual REQUIRED DBPROPSET_DBINIT properties are covered in GetDataSource
	//That method specifies an init string with properites, verifies the resulatnat
	//datasource contains the properties.  Then call GetInitString and verifies the 
	//String returned also contains the properties on the DataSource.

	//For completeness we will test it here as well, but make it more interesting
	//and set all properties and verify they all exist in the InitString...

	TBEGIN
	ULONG i,cPropInfoSets = NULL;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;
	WCHAR* pwszInitString = NULL;
	IUnknown* pDataSource = NULL;
	HRESULT hr = S_OK;

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//Create new Datasource
	TESTC_(CreateDataSource(NULL, IID_IUnknown, &pDataSource, CREATEDSO_NONE),S_OK)
	
	//GetPropertyInfo - (just DBINIT - exclude provider spepcific)
	TESTC_(GetPropertyInfo(DBPROPSET_DBINIT, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK);

	//Loop through all propsets, were only interested in non DBPROPSET_DBINIT sets
	for(i=0; i<cPropInfoSets; i++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[i];
		for(ULONG j=0; j<pPropInfoSet->cPropertyInfos; j++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[j];
			VARIANT* pVariant = &pPropInfo->vValues;

			//Need to Set a particualr value for this property
			//Since we are actually "saving" this in the Variant we can put whatever
			//we want and just compare it to the GetProperties call
			VariantClear(pVariant);
			GetSomePropValue(pVariant, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, pPropInfo->vtType, FALSE/*fValid*/);
			
			//Set This Property
			TESTC(::SetProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, &cPropSets, &rgPropSets, (void*)pVariant, DBTYPE_VARIANT));
		}
	}

	//Set all these properties
	SetProperties(pDataSource, cPropSets, rgPropSets);

	//Get the InitString
	TESTC_(GetInitString(pDataSource, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//Now that we have all Properties set...
	//Verify that GetInitString returns them all
	TESTC(VerifyInitString(pDataSource, pwszInitString));

CLEANUP:
	::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pDataSource);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Properties - Provider Specific Init Properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_20()
{ 
	//All individual REQUIRED DBPROPSET_DBINIT properties are covered in GetDataSource
	//That method specifies an init string with properites, verifies the resulatnat
	//datasource contains the properties.  Then call GetInitString and verifies the 
	//String returned also contains the properties on the DataSource.

	//For completeness we will test it here as well, but make it more interesting
	//and set all properties and verify they all exist in the InitString...

	TBEGIN
	ULONG i,cPropInfoSets = NULL;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;
	WCHAR* pwszInitString = NULL;
	IUnknown* pDataSource = NULL;
	HRESULT hr = S_OK;

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//Create new Datasource
	TESTC_(CreateDataSource(NULL, IID_IUnknown, &pDataSource, CREATEDSO_NONE),S_OK)

	//GetPropertyInfo
	TESTC_(GetPropertyInfo(DBPROPSET_DBINITALL, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK);

	//Loop through all propsets, were only interested in non DBPROPSET_DBINIT sets
	for(i=0; i<cPropInfoSets; i++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[i];
		for(ULONG j=0; j<pPropInfoSet->cPropertyInfos; j++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[j];
			VARIANT* pVariant = &pPropInfo->vValues;

			//Need to Set a particualr value for this property
			//Since we are actually "saving" this in the Variant we can put whatever
			//we want and just compare it to the GetProperties call
			VariantClear(pVariant);
			GetSomePropValue(pVariant, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, pPropInfo->vtType, FALSE/*fValid*/);
			
			//Set This Property
			TESTC(::SetProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, &cPropSets, &rgPropSets, (void*)pVariant, DBTYPE_VARIANT));
		}
	}

	//Set all these properties
	SetProperties(pDataSource, cPropSets, rgPropSets);

	//Get the InitString
	TESTC_(GetInitString(pDataSource, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//Now that we have all Properties set...
	//Verify that GetInitString returns them all
	TESTC(VerifyInitString(pDataSource, pwszInitString));

CLEANUP:
	::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pDataSource);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Properties - OPTIONAL Init Properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_21()
{ 
	//All individual REQUIRED DBPROPSET_DBINIT properties are covered in GetDataSource
	//That method specifies an init string with properites, verifies the resulatnat
	//datasource contains the properties.  Then call GetInitString and verifies the 
	//String returned also contains the properties on the DataSource.

	//For completeness we will test it here as well, but make it more interesting
	//and set all properties and verify they all exist in the InitString...

	TBEGIN
	ULONG i,cPropInfoSets = NULL;
	DBPROPINFOSET* rgPropInfoSets = NULL;
	WCHAR* pwszStringBuffer = NULL;
	WCHAR* pwszInitString = NULL;
	IUnknown* pDataSource = NULL;
	HRESULT hr = S_OK;

	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;

	//Create new Datasource
	TESTC_(CreateDataSource(NULL, IID_IUnknown, &pDataSource, CREATEDSO_NONE),S_OK)

	//GetPropertyInfo
	TESTC_(GetPropertyInfo(DBPROPSET_DBINITALL, &cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer),S_OK);

	//Loop through all propsets, were only interested in non DBPROPSET_DBINIT sets
	for(i=0; i<cPropInfoSets; i++)
	{
		DBPROPINFOSET* pPropInfoSet = &rgPropInfoSets[i];
		for(ULONG j=0; j<pPropInfoSet->cPropertyInfos; j++)
		{
			DBPROPINFO* pPropInfo = &pPropInfoSet->rgPropertyInfos[j];
			VARIANT* pVariant = &pPropInfo->vValues;

			//Need to Set a particualr value for this property
			//Since we are actually "saving" this in the Variant we can put whatever
			//we want and just compare it to the GetProperties call
			VariantClear(pVariant);
			GetSomePropValue(pVariant, pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, pPropInfo->vtType, FALSE/*fValid*/);
			
			//Set This Property
			//Set Every other one as Optional...
			TESTC(::SetProperty(pPropInfo->dwPropertyID, pPropInfoSet->guidPropertySet, &cPropSets, &rgPropSets, (void*)pVariant, DBTYPE_VARIANT, j%2 ? DBPROPOPTIONS_REQUIRED : DBPROPOPTIONS_OPTIONAL));
		}
	}

	//Set all these properties
	SetProperties(pDataSource, cPropSets, rgPropSets);

	//Get the InitString
	TESTC_(GetInitString(pDataSource, PASSWORD_INCLUDED, &pwszInitString),S_OK);

	//Now that we have all Properties set...
	//Verify that GetInitString returns them all
	TESTC(VerifyInitString(pDataSource, pwszInitString));

CLEANUP:
	::FreeProperties(&cPropInfoSets, &rgPropInfoSets, &pwszStringBuffer);
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_FREE(pwszInitString);
	SAFE_RELEASE_DSO(pDataSource);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Properties - COLID Properties
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_22()
{ 
	//TODO
	//Currently not supported by the InitString spec...
	//And now way to represent in the InitString...
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Properties - Special Chars - DBCS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_23()
{ 
	TBEGIN
	WCHAR wszValue[MAX_QUERY_LEN+1];
	wszValue[0] = L'\0';

	//For all "String" properties create a unique DBCS String for the property,
	//Verify GetDataSource can handle it and sets it appropiately...

	//Create Locale Object
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );

	DBPROPID dwPropertyID = DBPROP_AUTH_PASSWORD;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyInitString(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue));

	dwPropertyID = DBPROP_AUTH_USERID;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyInitString(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue));

	dwPropertyID = DBPROP_INIT_DATASOURCE;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyInitString(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue));
	
	dwPropertyID = DBPROP_INIT_LOCATION;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyInitString(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue));

	dwPropertyID = DBPROP_INIT_PROVIDERSTRING;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyInitString(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue));

	dwPropertyID = DBPROP_AUTH_INTEGRATED;
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
	TESTC(VerifyInitString(dwPropertyID,	DBPROPSET_DBINIT, DBTYPE_BSTR, wszValue));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - GetInitString from 3 threads, all having the same DataSource pointer
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_24()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	
	WCHAR* pwszInitString1 = NULL;
	WCHAR* pwszInitString2 = NULL;
	WCHAR* pwszInitString3 = NULL;
	
	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, pIDBInit(), (void*)PASSWORD_INCLUDED, &pwszInitString1, (void*)S_OK };
	THREADARG T2Arg = { this, pIDBInit(), (void*)PASSWORD_INCLUDED, &pwszInitString2, (void*)S_OK };
	THREADARG T3Arg = { this, pIDBInit(), (void*)PASSWORD_INCLUDED, &pwszInitString3, (void*)S_OK };

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_GetInitString, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_GetInitString, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetInitString, &T3Arg);

	START_THREADS();
	END_THREADS();	

	//Make the Strings are equal
	TESTC(wcscmp(pwszInitString1, pwszInitString2)==0);
	TESTC(wcscmp(pwszInitString2, pwszInitString3)==0);

CLEANUP:
	SAFE_FREE(pwszInitString1);
	SAFE_FREE(pwszInitString2);
	SAFE_FREE(pwszInitString3);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - GetInitString from 3 threads, all having different DataSource pointers
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_25()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	
	WCHAR* pwszInitString1 = NULL;
	WCHAR* pwszInitString2 = NULL;
	WCHAR* pwszInitString3 = NULL;

	IDBInitialize* pIDBInitialize = NULL;
	IUnknown* pIUnknown = NULL;
	IDBProperties* pIDBProperties = NULL;

	//Create 3 different DataSources, with different combination of props...
	CreateDataSource(NULL, IID_IDBInitialize,	(IUnknown**)&pIDBInitialize);
	CreateDataSource(NULL, IID_IUnknown,		(IUnknown**)&pIUnknown, CREATEDSO_SETPROPERTIES);
	CreateDataSource(NULL, IID_IDBProperties,	(IUnknown**)&pIDBProperties, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE);

	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, pIDBInit(), (void*)PASSWORD_INCLUDED, &pwszInitString1, (void*)S_OK };
	THREADARG T2Arg = { this, pIDBInit(), (void*)PASSWORD_EXCLUDED, &pwszInitString2, (void*)S_OK };
	THREADARG T3Arg = { this, pIDBInit(), (void*)PASSWORD_INCLUDED, &pwszInitString3, (void*)S_OK };

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_GetInitString, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_GetInitString, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_GetInitString, &T3Arg);

	START_THREADS();
	END_THREADS();	

	SAFE_FREE(pwszInitString1);
	SAFE_FREE(pwszInitString2);
	SAFE_FREE(pwszInitString3);

	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE_DSO(pIDBProperties);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc ODBC - Interesting Backward Compatible Strings
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_26()
{ 
	TBEGIN
	WCHAR* pwszDSN = NULL;
	WCHAR* pwszUID = NULL;
	WCHAR* pwszPWD = NULL;
	WCHAR* pwszProvString = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	DBPROP* pProp = NULL;

	//Make sure this is running against MSDASQL (ODBC Provider)
	TESTC_PROVIDER(PROVIDER_CLSID == CLSID_MSDASQL);

	//Obtain the Initialization properties
	TESTC(GetInitProps(&cPropSets, &rgPropSets));
	
	//PROVIDERSTRING
	//NOTE: Properties could be VT_EMPTY is not explcitly set
	if(FindProperty(DBPROP_INIT_PROVIDERSTRING, DBPROPSET_DBINIT, cPropSets, rgPropSets, &pProp) && pProp->vValue.vt == VT_BSTR)
		pwszProvString = wcsDuplicate(V_BSTR(&pProp->vValue));

	//DSN 
	if(FindProperty(DBPROP_INIT_DATASOURCE, DBPROPSET_DBINIT, cPropSets, rgPropSets, &pProp) && pProp->vValue.vt == VT_BSTR)
		pwszDSN = wcsDuplicate(V_BSTR(&pProp->vValue));
	if(!pwszDSN)
		GetModInfo()->GetStringKeywordValue(pwszProvString, L"DSN", &pwszDSN);
	
	//UID
	if(FindProperty(DBPROP_AUTH_USERID, DBPROPSET_DBINIT, cPropSets, rgPropSets, &pProp) && pProp->vValue.vt == VT_BSTR)
		pwszUID = wcsDuplicate(V_BSTR(&pProp->vValue));
	if(!pwszUID)
		if(!GetModInfo()->GetStringKeywordValue(pwszProvString, L"UID", &pwszUID))
			pwszUID = wcsDuplicate(L"");

	//PWD
	if(FindProperty(DBPROP_AUTH_PASSWORD, DBPROPSET_DBINIT, cPropSets, rgPropSets, &pProp) && pProp->vValue.vt == VT_BSTR)
		pwszPWD = wcsDuplicate(V_BSTR(&pProp->vValue));
	if(!pwszPWD)
		if(!GetModInfo()->GetStringKeywordValue(pwszProvString, L"PWD", &pwszPWD))
			pwszPWD = wcsDuplicate(L"");

	//Since we are running against MSDASQL, we need a DSN to verify these senarios...
	//The only way to get coonected without a DSN or ProviderString, would be a fileDSN
	//If this variation is skipped (or FileDSNs become interesting for automation we can
	//update this senario.  But for the most part we have gone beyond the call of duty
	//to find the individual pieces of connect into se we can vary them in the connect string...
	TESTC_PROVIDER(pwszDSN && pwszUID && pwszPWD);
	
	//NOTE: All unrecognized keyword/value pairs are passed directly to the provider
	//in the ProviderString (Extended Properties).  This has been advised a little
	//to remove any leading space before the keyword, so strings like:
	//Provider=MSDASQL; DSN=dsn;UID=uid;PWD=pwd; actually removes the leading space
	//otherwise ODBC will fail th connection.  But note this is only whitespace before the
	//keyword, after is retained.  Its retained since ODBC doesn't require multi-words 
	//to be quoted, its only looking for the ending semicolon and ODBC perserves space
	//as well

	//Standard Common Connection String
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Provider=%s;DSN=%s;UID=%s;PWD=%s;", GetProgID(), pwszDSN, pwszUID, pwszPWD),S_OK);
	//Extra space before Provider String - should be stripped
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Provider=%s; DSN=%s;UID=%s;PWD=%s;", GetProgID(), pwszDSN, pwszUID, pwszPWD),S_OK);

	//Standard String (default to MSDASQL - no provider specified)
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN=%s;UID=%s;PWD=%s;", pwszDSN, pwszUID, pwszPWD),S_OK);
	//No trailing semi - not required by ODBC or DSL
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN=%s;UID=%s;PWD=%s", pwszDSN, pwszUID, pwszPWD),S_OK);
	//Leading space - removed by DSL
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L" DSN=%s;UID=%s;PWD=%s", pwszDSN, pwszUID, pwszPWD),S_OK);
	//Trailing space (after the semi) is ignored
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN=%s;UID=%s;PWD=%s;   ", pwszDSN, pwszUID, pwszPWD),S_OK);
	//Leading space before a keyword is ignored and removed by DSL (although would fail directly with ODBC)
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN=%s; UID=%s; PWD=%s;", pwszDSN, pwszUID, pwszPWD),S_OK);

	//Specifcally indicating Extended Properties (rather than unrecognized)
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Extended Properties=\"DSN=%s;UID=%s;PWD=%s;\"", pwszDSN, pwszUID, pwszPWD),S_OK);

	//Combination
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Data Source = %s ; UID=%s; PWD=%s;", pwszDSN, pwszUID, pwszPWD),S_OK);
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Data Source = %s ; UID=%s; Password=%s;", pwszDSN, pwszUID, pwszPWD),S_OK);
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Data Source = %s ;User ID = %s; PWD=%s;", pwszDSN, pwszUID, pwszPWD),S_OK);
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Data Source=%s ;User ID=%s;Password=%s;", pwszDSN, pwszUID, pwszPWD),S_OK);

	//Specifing other properties besides providerstring
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"Data Source = %s ; Extended Properties=\"UID=%s;PWD=%s;\"", pwszDSN, pwszUID, pwszPWD),S_OK);
	
	//WhiteSpace After (Failure from ODBC)
	//NOTE: Some drviers are fine without a trailing semicolon
	TEST2C_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN=%s;UID=%s;PWD=%s ", pwszDSN, pwszUID, pwszPWD), DB_SEC_E_AUTH_FAILED, S_OK);
	//Whitespace after value (before semi) is retained by DSL and failure for ODBC
	//NOTE: E_FAIL in this case since ODBC is trying to find a server with a space 
	//as the last character which does't exist.
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN=%s ;UID=%s ;PWD=%s", pwszDSN, pwszUID, pwszPWD),E_FAIL);
	//Spaces before value is retained and a failure by ODBC
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN= %s;UID= %s;PWD= %s;", pwszDSN, pwszUID, pwszPWD),E_FAIL);

	//Spaces before seperator are normally removed by DSL
	//But for unrecognized keywords they are not removed
	TESTC_(GetDataSource(NULL, IID_IDBInitialize, NULL, CREATEDSO_INITIALIZE, 
		L"DSN =%s;UID =%s;PWD =%s;", pwszDSN, pwszUID, pwszPWD),E_FAIL);
	
CLEANUP:
	SAFE_FREE(pwszDSN);
	SAFE_FREE(pwszUID);
	SAFE_FREE(pwszPWD);
	SAFE_FREE(pwszProvString);
	::FreeProperties(&cPropSets, &rgPropSets);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc ODBC - PASSWORD_INCLUDED - Verify Password Saved
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_27()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc ODBC - PASSWORD_INCLUDED - Verify PWD parsed out
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCGetInitializationString::Variation_28()
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
BOOL TCGetInitializationString::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCLoadStringFromStorage)
//*-----------------------------------------------------------------------
//| Test Case:		TCLoadStringFromStorage - Test IDataInitialize::LoadStringFromStorage
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCLoadStringFromStorage::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - pwszFileName - NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_1()
{ 
	TBEGIN

	//LoadString
	TESTC_(LoadString(NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - ppwszInitString - NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_2()
{ 
	TBEGIN

	//LoadString 
	//NOTE: must call directly - since our helper handles NULL
	TESTC_(LoadStringInternal(L"valid.udl", NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_PATHNOTFOUND - Valid formed filename, (just non-existent on the disk)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_3()
{ 
	TBEGIN
	
	//Make sure it doesn't exist...	
	RemoveFile(FILENAME_NOTFOUND);

	//LoadString
	TESTC_(LoadString(FILENAME_NOTFOUND), STG_E_FILENOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND - Valid formed filename, (but no access writes)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND - Valid formed Directory, (but non existent directory)
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_5()
{ 
	TBEGIN

	//LoadString
	TESTC_(LoadString(L"c:\\this is my non exsitent file path\\file.ext"), STG_E_PATHNOTFOUND);

	//NOTE: This filename contains control characters (\t \f), so its invalid...
	TESTC_(LoadString(L"c:\this is my non exsitent file path\file.ext"), STG_E_INVALIDNAME);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND [Invalid-Name] - Invalid filename
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_6()
{ 
	TBEGIN

	//LoadString
	//To OS returns GetLastError=2 from CreateFile = STG_E_FILENOTFOUND.
	//Seems like this should be STG_E_INVALIDNAME, but the "\" in the FILENAME_INVALID
	//cuases it to think its a directory and not file... 
	TEST2C_(LoadString(FILENAME_INVALID), STG_E_FILENOTFOUND, STG_E_INVALIDNAME);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_INVALIDNAME [Invalid-Name] - Invalid file path
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_7()
{ 
	TBEGIN

	//LoadString
	TESTC_(LoadString(FILENAME_INVALIDPATH), STG_E_INVALIDNAME );

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_PATHNOTFOUND [Invalid-Name] - just directory location
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_8()
{ 
	TBEGIN

	//LoadString
	TESTC_(LoadString(FILENAME_PATH), STG_E_PATHNOTFOUND );

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND [Invalid-Name] - Empty String
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_9()
{ 
	TBEGIN

	//LoadString
	TESTC_(LoadString(L""), STG_E_PATHNOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_INVALIDNAME [Invalid-Name] - Wildcards
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_10()
{ 
	TBEGIN

	//LoadString
	TESTC_(LoadString(L"*.*"), STG_E_INVALIDNAME );

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_ACCESSDENIED - ReadOnly Directory
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_11()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_SHAREVIOLATION - File is locked
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_12()
{ 
	TBEGIN

	//Need to save an Empty File...
	WCHAR* pwszFileName = NULL;
	HANDLE hFile = CreateFile(L"MyEmptyFile.ext", &pwszFileName);

	//LoadString
	TESTC_(LoadString(pwszFileName), STG_E_SHAREVIOLATION);

CLEANUP:
	//Don't Close the File before calling LoadString, so already open
	//in ReadOnly mode (default for CreateFile)..
	CloseHandle(hFile);

	SAFE_FREE(pwszFileName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_INVALIDHEADER - Invalid File Format
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_13()
{ 
	TBEGIN
	WCHAR* pwszFileName = NULL;
	HANDLE hFile = CreateFile(L"RICHED32.DLL", &pwszFileName, OPEN_EXISTING, GENERIC_READ);
	TESTC_PROVIDER(hFile != INVALID_HANDLE_VALUE && pwszFileName);

	//Close the File before calling LoadString, so its not already open...
	CloseHandle(hFile);
	hFile = NULL;

	//LoadString
	TESTC_(LoadString(pwszFileName), STG_E_INVALIDHEADER);

CLEANUP:
	CloseHandle(hFile);
	SAFE_FREE(pwszFileName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_INVALIDHEADER - Empty File
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_14()
{ 
	TBEGIN

	//Need to save an Empty File...
	WCHAR* pwszFileName = NULL;
	HANDLE hFile = CreateFile(L"MyEmptyFile.ext", &pwszFileName);

	//Close the File before calling LoadString, so its not already open...
	CloseHandle(hFile);
	hFile = NULL;

	//LoadString
	TESTC_(LoadString(pwszFileName), STG_E_INVALIDHEADER);

CLEANUP:
	SAFE_FREE(pwszFileName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Fake 8.3
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_15()
{ 
	TBEGIN

	//Verify - LoadString
	//Make sure we can save and load using alias
	TESTC(VerifyWriteString(L"MyFake~1.ext", L"MyString"));

	//Make sure we can save using long file name, and load using 8.3 alias...
	//NOTE: This is done automatically for all files in VerifyWriteString
	TESTC(VerifyWriteString(L"My Long File Name.udl", L"MyString"));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Long FileName > 255 characters
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_16()
{ 
	TBEGIN
	CHAR	szCurDirectory[_MAX_DIR];

	//Win2000 Logo Requirements for LFN (Long File Names)
	
	//Support for leading and trailing periods ( . ) has been dropped from the Logo requirements.
	//An LFN is 260 characters long, which generally includes: 
	//	3 bytes for "<driveletter>:\" 
	//	255 bytes for the file name and extension 
	//	2 bytes for the null terminator 
	//A Uniform Naming Convention (UNC) path has 2 bytes for "\\" instead of 3 bytes for 
	//"<driveletter>:\", and the path may not include an extension (file type).

	//MAX_PATH = 260 chars = 3 chars (driver:\) + 255 chars (filename+ext) + 2 chars (null term)
	WCHAR wszFileName[_MAX_PATH] =		L"c:\\"							// 3 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"My really cool long file "	//25 chars
										L"e.ext"						// 5 chars
										L"\0";							// 1 char

	//Make sure the drive letter is correct...
	if(GetCurrentDirectory(_MAX_DIR, szCurDirectory))
	{
		CHAR	szCurDrive[_MAX_DRIVE];
		WCHAR	wszCurDrive[_MAX_DRIVE];

		//Replace the Driver letter with the current drive...
		_splitpath(szCurDirectory, szCurDrive, NULL/*directory*/, NULL/*fname*/, NULL/*ext*/);
		ConvertToWCHAR(szCurDrive, wszCurDrive, _MAX_DRIVE);
		wszFileName[0] = wszCurDrive[0];
	}

	//InitString
	WCHAR* pwszInitString = L"MyString that contains a \t tab character and contains a \n new line and hey why not throw in a carriage return \r character as well?!  Who knows this might even find a bug saving the file in text mode?  You think?";

	//Verify - LoadString
	TESTC(VerifyWriteString(wszFileName, pwszInitString));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Huge FileName > Stack Size
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_17()
{ 
	TBEGIN

	//We need to make sure internally SC is not using the stack for conversions,
	//or is at least making sure the filename is smaller than a MAX_FNAME.
	//Many time "_alloca" will be used, which will allocate on the stack but for large
	//names (if no checking is done ahead of time) this may corrupt or overflow the stack...
	const ULONG MAX_HUGE_SIZE	= 10000;
	WCHAR* pwszHugeFileName		= NULL;
	WCHAR* pwszHugeInitString	= NULL;
	ULONG i = 0;

	SAFE_ALLOC(pwszHugeFileName,	WCHAR, MAX_HUGE_SIZE);
	SAFE_ALLOC(pwszHugeInitString,	WCHAR, MAX_HUGE_SIZE);
	
	//Dynamically Create US strings for both...
	for(i=0; i<MAX_HUGE_SIZE-1; i++)
	{
		pwszHugeFileName[i]		= L'A' + WCHAR(i % 26);
		pwszHugeInitString[i]	= L'a' + WCHAR(i % 26);
	}

	//NULL Terminators
	pwszHugeFileName[MAX_HUGE_SIZE-1]		= L'\0';
	pwszHugeInitString[MAX_HUGE_SIZE-1]		= L'\0';


	//WriteStringFromStorage
	TESTC_(WriteString(pwszHugeFileName, pwszHugeInitString), STG_E_PATHNOTFOUND);

	//LoadStringFromStorage
	TESTC_(LoadString(pwszHugeFileName), STG_E_PATHNOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Read Only File
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_18()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - UNC Format \\server\\dir\\file.dot
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_19()
{ 
	TBEGIN
	WCHAR* pwszMachineName = GetMachineName();
	HANDLE hFile = NULL;

	//Try and find a directory that we can write to using server notation
	//so we don't have to always skip this variaiton...
	WCHAR* pwszFileName	= CreateString(L"\\\\%s\\c$\\my server file name", pwszMachineName);
	hFile = CreateFile(pwszFileName, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		SAFE_FREE(pwszFileName);
		pwszFileName = CreateString(L"\\\\%s\\d$\\my server file name", pwszMachineName);
		hFile = CreateFile(pwszFileName, NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			SAFE_FREE(pwszFileName);
			WCHAR* pwszFileName	= CreateString(L"\\\\%s\\webcat\\my server file name", pwszMachineName);
			hFile = CreateFile(pwszFileName, NULL);
		}
	}

	//At this point make sure we have such a directory...
	//NOTE: on some automation machines we can't obtain the directory, or create directories, so
	//we allow this to pass - otherwise if we skip it (ie: TESTC_PROVIDER) then it will constantly
	//miscompare the baselines since its possible on some machines...
	if(hFile != INVALID_HANDLE_VALUE && pwszFileName)
	{
		CloseHandle(hFile);
		hFile = NULL;

		//Verify - LoadString
		TESTC(VerifyWriteString(pwszFileName, L"MyString"));
	}

CLEANUP:
	if(hFile)
		CloseHandle(hFile);
	SAFE_FREE(pwszMachineName);
	SAFE_FREE(pwszFileName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Http format
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_20()
{ 
	TBEGIN
	WCHAR* pwszMachineName = GetMachineName();
	
	//Need to find the Current Server Name, and valid directory name...
	WCHAR* pwszFileName		= CreateString(L"file://%s\\public\\myserverfilename.txt", pwszMachineName);
	
	//Make sure we have such a directory...
	HANDLE hFile = CreateFile(pwszFileName, NULL);
	DWORD dwLastError = GetLastError();
	TESTC_PROVIDER(hFile != INVALID_HANDLE_VALUE && pwszFileName);
	CloseHandle(hFile);
	hFile = NULL;

	//Verify - LoadString
	TESTC(VerifyWriteString(pwszFileName, L"MyString"));

CLEANUP:
	CloseHandle(hFile);
	SAFE_FREE(pwszMachineName);
	SAFE_FREE(pwszFileName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Drive:\\Dir\\File.dot
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_21()
{ 
	TBEGIN
	CHAR szBuffer[MAX_QUERY_LEN+1];
	WCHAR wszBuffer[MAX_QUERY_LEN+1];
	
	//Obtain the Current Directory
	GetCurrentDirectoryA(MAX_QUERY_LEN, szBuffer);


	ConvertToWCHAR(szBuffer, wszBuffer, MAX_QUERY_LEN);

	//Need to find the Current Server Name, and valid directory name...
	WCHAR* pwszFileName			= CreateString(L"%s\\my server filename.dot", wszBuffer);

	//Verify - LoadString
	TESTC(VerifyWriteString(pwszFileName, L"MyString"));

CLEANUP:
	SAFE_FREE(pwszFileName);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Special Chars - DBCS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_22()
{ 
	TBEGIN
	CHAR szBuffer[MAX_QUERY_LEN+1];
	WCHAR wszFileName[MAX_QUERY_LEN+1];
	wszFileName[0] = L'\0';
	WCHAR wszInitString[MAX_QUERY_LEN+1];
	wszInitString[0] = L'\0';
	
	//Path is limited by 256 characters.
	//So we need to find out the length of the current direcotry
	GetCurrentDirectoryA(MAX_QUERY_LEN, szBuffer);

	//Create Locale Object
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );
	LocaleInfo.MakeUnicodeIntlString(wszFileName, _MAX_FNAME - (INT)strlen(szBuffer));
	LocaleInfo.MakeUnicodeIntlString(wszInitString, MAX_QUERY_LEN);

	//Verify - LoadString
	TESTC(VerifyWriteString(wszFileName, wszInitString));
							   				
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Special Chars - OEM
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_23()
{ 
	TBEGIN
	WCHAR wch = 0x00A3;	// Currency symbol, English Pound (different on ANSI and OEM)
	WCHAR wszFileName[] = { wch, L'.', wch, L'\0' } ;
	WCHAR wszInitString[] = { wch, L'\0' } ;

	//Default is ANSI, (but just make sure)
	TESTC(AreFileApisANSI());

	//Make sure that the file name can actually be converted to ANSI.
	TESTC_PROVIDER( iswcharMappable(wch) );

	//ANSI -> ANSI
	//(this is the default but just for completeness will will repeat here)
	SetFileApisToANSI();
	TESTC(VerifyWriteString(wszFileName, wszInitString));

	//OEM -> OEM 
	SetFileApisToOEM();
	TESTC(VerifyWriteString(wszFileName, wszInitString));

	//OEM -> ANSI
	SetFileApisToOEM();
	TESTC_(WriteString(wszFileName, wszInitString),S_OK);
	SetFileApisToANSI();
	TESTC(VerifyLoadString(wszFileName, wszInitString));
	//NOTE: Our RemoveFile will fail if it was unable to find the file created
	//from Write, so this is how we are verifying the correct characters were used for the filename
	//(as this function correrctly uses ANSI or OEM depending upon "AreFileApisANSI")
	TESTC(RemoveFile(wszFileName));

	//ANSI -> OEM
	SetFileApisToANSI();
	TESTC_(WriteString(wszFileName, wszInitString),S_OK);
	SetFileApisToOEM();
	TESTC(VerifyLoadString(wszFileName, wszInitString));
	TESTC(DeleteFileW(wszFileName));

CLEANUP:
	//Restore to the default
	SetFileApisToANSI();
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END



// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Special Chars, plus signs, commas, brackets, etc.
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_24()
{ 
	TBEGIN

	//Special Characters that are not allowed are according to File Explorer 
	//are "\/:*?"<>|" (including the double quote character)

	//Verify - LoadString
	TESTC(VerifyWriteString(L"+This;{File}@Name=;$`Con#tains_~all, [the+%&']'(special)[-[]char[^a]acters]!.udl", 
							L"+This;{Init}@String=;$`Con#tains_~all, [the+%&']'(speical)[-[]char[^a]acters]!"));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Not save leading and trailing spaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_25()
{ 
	TBEGIN
	
	//NOTE: This is no longer a WinLogo requirement for API's.  So use this varition
	//to verify that leading and trailing spaces are preserved...

	//FileName should be saved with leading and trailing removed...
	WCHAR* pwszFileName			= L"    This sure had better ignore leading and trailing spaces, but keep embeeded ones!  ";
	WCHAR* pwszInitString		= L"        This sure had better ignore leading and trailing spaces, but keep embeeded ones!       ";

	//Verify - LoadString (only leading)
	TESTC(VerifyWriteString(L" file name", pwszInitString));

	//Verify - LoadString (only trailing)
	TESTC(VerifyWriteString(L"file name ", pwszInitString));

	//Verify - LoadString (leading and trailing)
	TESTC(VerifyWriteString(pwszFileName, pwszInitString));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Save embedded spaces
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_26()
{ 
	TBEGIN
	CHAR szBuffer[MAX_QUERY_LEN+1];
	WCHAR wszBuffer[MAX_QUERY_LEN+1];
	
	//Obtain the Current Directory
	GetCurrentDirectoryA(MAX_QUERY_LEN, szBuffer);

	ConvertToWCHAR(szBuffer, wszBuffer, MAX_QUERY_LEN);

	//Form		Directory\ file name
	//Make sure spaces are preserved 
	//(as well as between the directory and filename?)
	//NOTE: This is the example provided in the Win2000 Logo requirements 
	wcscat(wszBuffer, L"\\ test , + , = [ ]");

	//Verify - LoadString
	TESTC(VerifyWriteString(wszBuffer, L"MyInitString"));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Must add an extension if not specified
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_27()
{ 
	TBEGIN
	
	//FileName should be saved with leading and trailing removed...
	WCHAR* pwszFileName			= L"This filename does not have a default extension";
	WCHAR* pwszInitString		= L"MyInitString.udl";

	//Verify - LoadString
	TESTC(VerifyWriteString(pwszFileName, pwszInitString));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Doesn't add an extension if already exists
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_28()
{ 
	TBEGIN
	WCHAR* pwszInitString		= L"MyInitString.udl";

	//Verify - LoadString
	TESTC(VerifyWriteString(L".filename",			pwszInitString));
	TESTC(VerifyWriteString(L"filename.",			pwszInitString));
	TESTC(VerifyWriteString(L"file.name",			pwszInitString));
	TESTC(VerifyWriteString(L".f.i.l.e.n.a.m.e.",	pwszInitString));
	TESTC(VerifyWriteString(L"filename...",			pwszInitString));
	TESTC(VerifyWriteString(L"...filename",			pwszInitString));
	TESTC(VerifyWriteString(L"filename.ud",			pwszInitString));
	TESTC(VerifyWriteString(L"filename.udl",		pwszInitString));
	TESTC(VerifyWriteString(L"filename.udl.",		pwszInitString));
	TESTC(VerifyWriteString(L"filename.udl.udl",	pwszInitString));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Not save question marks
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_29()
{ 
	TBEGIN
	
	//FileName should not be saved with "?" wildcard character...
	WCHAR* pwszFileName			= L"filename?";
	WCHAR* pwszInitString		= L"filename.udl";	

	//Verify - WriteStringToStorage
	TESTC_(WriteString(pwszFileName, pwszInitString), STG_E_INVALIDNAME);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - S_OK - Valid Characters one at a time
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_30()
{ 
	TBEGIN
	
	WCHAR*	pwszValid		= L"+;{}@=$`#_~,[%&']()-^!";
	WCHAR*	pwszInitString	= L"+;{}@=$`#_~,[%&']()-^!" L"\\/*?\"<>|";
	WCHAR	wszFileName[_MAX_PATH];

	//Make sure all the valid characters are valid even if used seperatly...
	for(WCHAR* pwsz=pwszValid; pwsz && *pwsz; pwsz++)
	{
		//With extension
		swprintf(wszFileName, L"%cFileName%c.udl", *pwsz, *pwsz);
		TESTC(VerifyWriteString(wszFileName, pwszInitString));

		//Without extension
		swprintf(wszFileName, L"%cFileName%c", *pwsz, *pwsz);
		TESTC(VerifyWriteString(wszFileName, pwszInitString));
	}

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc WinLogo - STG_E_INVALIDNAME - Invalid Characters one at a time
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_31()
{ 
	TBEGIN
	
	WCHAR*	pwszillegal		= L"\\/*?\"<>|";
	WCHAR*	pwszInitString	= L"+;{}@=$`#_~,[%&']()-^!" L"\\/*?\"<>|";
	WCHAR	wszFileName[_MAX_PATH];

	//Make sure all the valid characters are valid even if used seperatly...
	for(WCHAR* pwsz=pwszillegal; pwsz && *pwsz; pwsz++)
	{
		//If there is a "\" or "/" the OS (CreateFile) seems to think this is a path and 
		//returns PATHNOTFOUND instead of INVALIDNAME...
		HRESULT hrExpected = (*pwsz == L'\\' || *pwsz == L'/') ? STG_E_PATHNOTFOUND : STG_E_INVALIDNAME;
		
		//With extension
		swprintf(wszFileName, L"%cFileName%c.udl", *pwsz, *pwsz);
		GCHECK(WriteString(wszFileName, pwszInitString), hrExpected);

		//Without extension
		swprintf(wszFileName, L"FileName%c", *pwsz);
		GCHECK(WriteString(wszFileName, pwszInitString), STG_E_INVALIDNAME);
	}

//CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_32()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc UDL Cache - Fill up UDL File Cache
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_33()
{ 
	TBEGIN
	ULONG i,cMaxCache = s_CUDLCache.GetMaxCacheSize();
	CUDLInfo* pCUDLInfo = NULL;
	CUDLInfo* pCUDLTail = NULL;

	//So we start with a clean slate.
	TESTC_PROVIDER(SetDevUDLCacheSize(0));
	TESTC(!m_fTestingHooks || GetDevUDLCacheSize() == 0);
	TESTC(SetDevUDLCacheSize(cMaxCache));
	s_CUDLCache.RemoveAll();

	//Verify that we can max out the cache
	//Our own cache will keep track of the items, (LRU)
	for(i=0; i<cMaxCache; i++)
		TESTC(VerifyWriteString(GENERATE_NAME, GENERATE_NAME));

	//Sanity check our internal cache making sure it has the right number of elements...
	TESTC(s_CUDLCache.GetCurrentSize() == cMaxCache);
	TESTC(!m_fTestingHooks || GetDevUDLCacheSize() == cMaxCache);

	//Our internal cache already verifed that the items were pulled from the 
	//cache (seperatly), but we need to make sure all items (as a whole) are still
	//in the cache, since we have put exactly the cache size number of elements in...
	for(i=0; i<cMaxCache; i++)
	{
		pCUDLInfo = s_CUDLCache[i];
		TESTC(VerifyDevUDLCache(S_OK/*cached*/, pCUDLInfo->GetFileName(), pCUDLInfo));
	}

	//Note compaction will remove the tail, so make a copy first
	pCUDLInfo = s_CUDLCache[cMaxCache-1];
	pCUDLTail = new CUDLInfo(pCUDLInfo->GetFileName(), pCUDLInfo->GetInitString(), pCUDLInfo->GetSeconds());

	//Add one more (to the cache)
	//This should (at least) remove the LRU item (last one in our list)
	TESTC(VerifyWriteString(GENERATE_NAME, GENERATE_NAME));
	TESTC(!m_fTestingHooks || GetDevUDLCacheSize() < cMaxCache);
	TESTC(!m_fTestingHooks || GetDevUDLCacheSize() == s_CUDLCache.GetCurrentSize());
	
	//Make sure the head (MRU) is cached
	TESTC(VerifyDevUDLCache(S_OK/*cached*/, s_CUDLCache[0]->GetFileName(), s_CUDLCache[0]));

	//Make sure the old file (LRU) is not cached
	TESTC(VerifyDevUDLCache(S_FALSE/*not cached*/, pCUDLTail->GetFileName(), pCUDLTail));

CLEANUP:
	SAFE_DELETE(pCUDLTail);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc UDL Cache - Interleaved UDL Requests
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_34()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc UDL Cache - Random UDL Requests
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_35()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(36)
//*-----------------------------------------------------------------------
// @mfunc UDL Cache - MultiThreaded Load from Cache
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_36()
{ 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(37)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_37()
{ 
	// TO DO:  Add your own code here 
	return TEST_SKIPPED;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(38)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - LoadStringFromStorage from 3 threads, all from the same file
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_38()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	//Make sure the cache is full, (if not already)
	FullDevUDLCache();
	CUDLInfo* pCUDLInfo = s_CUDLCache[0];

	//Setup Thread Arguments
	THREADARG T1Arg = { this, pCUDLInfo, (void*)S_OK };

	//Create Threads
	CREATE_THREADS(Thread_LoadString, &T1Arg);
	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(39)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - LoadStringFromStorage from 3 threads, all having different files
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCLoadStringFromStorage::Variation_39()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	
	//Setup Thread Arguments
	THREADARG T1Arg = { this, NULL, (void*)S_OK };

	//Make sure the cache is full, (if not already)
	TESTC(FullDevUDLCache());

	//Create Threads
	CREATE_THREADS(Thread_LoadString, &T1Arg);
	START_THREADS();
	END_THREADS();	

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCLoadStringFromStorage::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END



// {{ TCW_TC_PROTOTYPE(TCWriteStringToStorage)
//*-----------------------------------------------------------------------
//| Test Case:		TCWriteStringToStorage - Test IDataInitialize::WriteStringToStorage
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCWriteStringToStorage::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 




// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - pwszFileName = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_1()
{ 
	TBEGIN

	//WriteString
	TESTC_(WriteString(NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END





// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - pwszInitString = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_2()
{ 
	TBEGIN

	//WriteString (have to call directly since our helper handles NULL)
	TESTC_(pDataInit()->WriteStringToStorage(L"valid.udl", NULL, CREATE_ALWAYS), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILEALREADYEXISTS - dwCreate = CREATE_NEW
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_3()
{ 
	TBEGIN

	//Create exsiting file
	TESTC_(WriteString(FILENAME_VALID),S_OK);

	//WriteString (CREATE_NEW with existing file)
	TESTC_(WriteString(FILENAME_VALID, NULL, CREATE_NEW),STG_E_FILEALREADYEXISTS);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND - dwCreate = OPEN_EXISTING
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_4()
{ 
	TBEGIN

	//WriteString (OPEN_EXISTING with non-existing file)
	TESTC_(WriteString(FILENAME_NOTFOUND, NULL, OPEN_EXISTING), STG_E_FILENOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND - dwCreate = TRUNCATE_EXISTING
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_5()
{ 
	TBEGIN

	//WriteString (TRUNCATE_EXISTING with non-existing file)
	TESTC_(WriteString(FILENAME_NOTFOUND, NULL, OPEN_EXISTING), STG_E_FILENOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_FILENOTFOUND - just directory name
//						   
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_6()
{ 
	TBEGIN

	//WriteString 
	//STG_E_PATHNOTFOUND should be returned since its just the directory name
	TESTC_(WriteString(L"\\", NULL, OPEN_EXISTING), STG_E_PATHNOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_SHAREVIOLATION - File is locked
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_7()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_ACCESSDENIED - ReadOnly Directory
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_8()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_INVALIDNAME [Invalid-Name] - ill-formed filename
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_9()
{ 
	TBEGIN

	//WriteString 
	//To OS returns GetLastError=2 from CreateFile = STG_E_FILENOTFOUND.
	//Seems like this should be STG_E_INVALIDNAME, but the "\" in the FILENAME_INVALID
	//cuases it to think its a directory and not file... 
	// In some cases, oledb does return STG_E_INVALIDNAME for invalid filenames, so
	// adding check for both values.
	//TESTC_(WriteString(FILENAME_INVALID), STG_E_FILENOTFOUND);
	TEST2C_(WriteString(FILENAME_INVALID), STG_E_FILENOTFOUND, STG_E_INVALIDNAME);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_INVALIDNAME [Invalid-Name] - ill-formed directory name
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_10()
{ 
	TBEGIN

	//WriteString 
	TESTC_(WriteString(FILENAME_INVALIDPATH), STG_E_INVALIDNAME );

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Boundary - STG_E_PATHNOTFOUND [Invalid-Name] - just directory location
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_11()
{ 
	TBEGIN

	//WriteString 
	//STG_E_PATHNOTFOUND should be returned since its just the directory name
	TESTC_(WriteString(FILENAME_PATH), STG_E_PATHNOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - dwCreateDisposition = invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_12()
{ 
	TBEGIN

	//WriteString 
	TESTC_(WriteString(GENERATE_NAME, NULL, CREATE_INVALID), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - CREATE_NEW
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_13()
{ 
	TBEGIN

	//WriteString 
	TESTC(VerifyWriteString(GENERATE_NAME, NULL, CREATE_NEW));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - CREATE_ALWAYS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_14()
{ 
	TBEGIN

	//WriteString - CREATE_ALWAYS - not previsouly existing
	TESTC(VerifyWriteString(GENERATE_NAME, NULL, CREATE_ALWAYS));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - CREATE_ALWAYS - previsouly existing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_15()
{ 
	TBEGIN

	//Create previously existing file
	TESTC_(WriteString(FILENAME_VALID),S_OK);

	//WriteString - CREATE_ALWAYS - with previsouly existing
	TESTC_(WriteString(FILENAME_VALID, NULL, CREATE_ALWAYS), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - OPEN_ALWAYS - not existing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_16()
{ 
	TBEGIN

	//WriteString - OPEN_ALWAYS - not previsouly existing
	TESTC(VerifyWriteString(GENERATE_NAME, NULL, OPEN_ALWAYS));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - OPEN_ALWAYS - existing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_17()
{ 
	TBEGIN

	//Create previously existing file
	TESTC_(WriteString(FILENAME_VALID),S_OK);

	//WriteString - OPEN_ALWAYS - with previsouly existing
	TESTC_(WriteString(FILENAME_VALID, NULL, OPEN_ALWAYS), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - OPEN_EXISTING - existing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_18()
{ 
	TBEGIN

	//Create previously existing file
	TESTC_(WriteString(FILENAME_VALID),S_OK);

	//WriteString - OPEN_EXISTING - with previsouly existing
	TESTC_(WriteString(FILENAME_VALID, NULL, OPEN_EXISTING), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Parameters - dwCreateDisposition - TRUNCATE_EXISTING - existing
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_19()
{ 
	TBEGIN

	//Create previously existing file
	TESTC_(WriteString(FILENAME_VALID),S_OK);

	//WriteString - TRUNCATE_EXISTING - with previsouly existing
	TESTC_(WriteString(FILENAME_VALID, NULL, TRUNCATE_EXISTING), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Sequence - WriteString(), LoadString, GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_20()
{ 
	TBEGIN
	WCHAR* pwszInitString = NULL;

	//WriteString
	TESTC_(WriteString(FILENAME_VALID),S_OK);

	//LoadSring
	TESTC_(LoadString(FILENAME_VALID, &pwszInitString),S_OK);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown), S_OK);

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Sequence - WriteString(some garbage), LoadString, GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_21()
{ 
	TBEGIN
	WCHAR* pwszInitString = NULL;
	WCHAR* pwszInitStringInput = L"Some Garge Stuff Here";

	//WriteString
	TESTC_(WriteString(FILENAME_VALID, pwszInitStringInput),S_OK);

	//LoadSring
	TESTC_(LoadString(FILENAME_VALID, &pwszInitString),S_OK);

	//Verify strings are equal...
	TESTC(wcscmp(pwszInitStringInput, pwszInitString)==0);

	//GetDataSource
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown), DB_E_BADINITSTRING);

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Sequence - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_22()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_NONE, PASSWORD_INCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Sequence - CreateDBInstance, Set Required Init Properties, GetInitString, WriteString, LoadString, GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_23()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_SETPROPERTIES, PASSWORD_INCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_24()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE, PASSWORD_INCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordSaved - GetInitString on UnInit DSO (no password set), WriteString, LoadString, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_25()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_NONE, PASSWORD_INCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordSaved - GetInitString on UnInit DSO (with password set), WriteString, LoadString, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_26()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_SETPROPERTIES, PASSWORD_INCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_27()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE, PASSWORD_EXCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on Init DSO, WriteString, LoadString, GetDataSource, SetPassword, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_28()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE, PASSWORD_EXCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on UnInit DSO (no password set), WriteString, LoadString, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_29()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_NONE, PASSWORD_EXCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Sequence - PasswordNotSaved - GetInitString on UnInit DSO (password set), WriteString, LoadString, GetDataSource, Initialize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_30()
{ 
	TBEGIN

	//Verify Complete Sequence (All Methods) - CreateDBInstance, GetInitString, WriteString, LoadString, GetDataSource, Initialize...
	TESTC(VerifyCompleteSequence(CREATEDSO_SETPROPERTIES, PASSWORD_EXCLUDED));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Stress - Large InitString > 255 characters
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_31()
{ 
	TBEGIN
	//We have already covered the complete init string in many other cases.
	//I don't realy want to have duplicate logic here that needs maintaining, so
	//I will use the stanard init string, but duplicate it so its fairly long in
	//length...

	IUnknown* pIUnknown = NULL;
	WCHAR* pwszInitString = NULL;

	//Make the property string quite long...
	for(ULONG i=0; i<100; i++)
		AppendString(&pwszInitString, m_pwszProperties);

	//Now that we have a fairly good size string.  Try all our normal combinations...
	TESTC_(GetDataSource(NULL, pwszInitString, IID_IUnknown, &pIUnknown),S_OK);

	//Try Saving and Loading it...
	TESTC(VerifyWriteString(GENERATE_NAME, pwszInitString));

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Stress - InitString - Special Chars - DBCS
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_32()
{ 
	TBEGIN
	WCHAR wszFileName[MAX_PATH/2];
	wszFileName[0] = L'\0';
	WCHAR wszInitString[MAX_QUERY_LEN+1];
	wszInitString[0] = L'\0';
	
	//Create Locale Object
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );
	LocaleInfo.MakeUnicodeIntlString(wszFileName, MAX_PATH/2);
	LocaleInfo.MakeUnicodeIntlString(wszInitString, MAX_QUERY_LEN);

	//Verify - WriteString
	TESTC(VerifyWriteString(wszFileName, wszInitString));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - WriteStringToStorage from 3 threads
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_33()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	//Make sure the cache is full, (if not already)
	FullDevUDLCache();
	CUDLInfo* pCUDLInfo = s_CUDLCache[0];
	
	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, pCUDLInfo, (void*)STG_E_SHAREVIOLATION };

	//Create Threads
	CREATE_THREADS(Thread_WriteString, &T1Arg);
	START_THREADS();
	END_THREADS();	

	//We need to verify the file is not corrupted, and one of them should have succeed...
	s_CUDLCache.RemoveUDLInfo(pCUDLInfo->GetFileName());
	TESTC(VerifyLoadString(pCUDLInfo->GetFileName(), pCUDLInfo->GetInitString()));

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - WriteStringToStorage from 3 threads, all having different files
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_34()
{ 
	TBEGIN
	INIT_THREADS(FIVE_THREADS);	

	//Make sure the cache is full, (if not already)
	FullDevUDLCache();

	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, s_CUDLCache[0],  (void*)S_OK };
	THREADARG T2Arg = { this, s_CUDLCache[1],  (void*)S_OK };
	THREADARG T3Arg = { this, s_CUDLCache[2],  (void*)S_OK };
	THREADARG T4Arg = { this, s_CUDLCache[3],  (void*)S_OK };
	THREADARG T5Arg = { this, s_CUDLCache[4],  (void*)S_OK };

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_WriteString, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_WriteString, &T2Arg);
	CREATE_THREAD(THREAD_THREE,	Thread_WriteString, &T3Arg);
	CREATE_THREAD(THREAD_FOUR,	Thread_WriteString, &T4Arg);
	CREATE_THREAD(THREAD_FIVE,	Thread_WriteString, &T5Arg);

	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - WriteStringToStorage while LoadStringFromStorage
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCWriteStringToStorage::Variation_35()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	

	//Make sure the cache is full, (if not already)
	FullDevUDLCache();

	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, s_CUDLCache[0], (void*)STG_E_SHAREVIOLATION };
	THREADARG T2Arg = { this, s_CUDLCache[1], (void*)STG_E_SHAREVIOLATION };


	//Create Threads
	CREATE_FIRST_THREADS(Thread_WriteString, &T1Arg);
	CREATE_SECOND_THREADS(Thread_LoadString, &T2Arg);
	
	START_THREADS();
	END_THREADS();	

//CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCWriteStringToStorage::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

// {{ TCW_TC_PROTOTYPE(TCPromptDataSource)
//*-----------------------------------------------------------------------
//| Test Case:		TCPromptDataSource - Test IDBPromptInitialize::PromptDataSource
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPromptDataSource::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - CoGetClassObject (all valid riids) - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_1()
{ 
	TBEGIN

	//Class Factory - CoGetClassObject
	TESTC_(GetClassObject(CLSID_DataLinks, IID_IUnknown),S_OK);
	TESTC_(GetClassObject(CLSID_DataLinks, IID_IClassFactory),S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - CoGetClassObject (all invalid riids) - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_2()
{ 
	TBEGIN

	//Class Factory - CoGetClassObject
	//NOTE: COM returns REGDB_E_CLASSNOTREG for this senario, even though E_NOINTERFACE
	//is spec'd in the docs for this senario.
	TEST2C_(GetClassObject(CLSID_DataLinks, IID_IDBPromptInitialize),	E_NOINTERFACE, REGDB_E_CLASSNOTREG);
	TEST2C_(GetClassObject(CLSID_DataLinks, IID_IDBInitialize),			E_NOINTERFACE, REGDB_E_CLASSNOTREG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - CoGetClassObject (NULL) - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_3()
{ 
	TBEGIN

	//Class Factory - CoGetClassObject
	//Must call directly since our helper - handles NULL
	TESTC_(CoGetClassObject(CLSID_DataLinks, CLSCTX_INPROC_SERVER, NULL, IID_IUnknown, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - CoCreateInstance (all valid riids) - S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_4()
{ 
	TBEGIN

	//CreateInstance - (CoCreateInstance - helper)
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IUnknown), S_OK);
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBPromptInitialize), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - CoCreateInstance (all invalid riids) - E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_5()
{ 
	TBEGIN

	//CreateInstance - (CoCreateInstance - helper)
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBInitialize), E_NOINTERFACE);
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IClassFactory), E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - CoCreateInstance (NULL) - E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_6()
{ 
	TBEGIN

	//CoCreateInstance
	//Have to call directly since our helper handles NULL
	TESTC_(CoCreateInstance(CLSID_DataLinks, NULL, CLSCTX_INPROC_SERVER, IID_IDBPromptInitialize, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - LockServer combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_7()
{ 
	TBEGIN
	IClassFactory* pIClassFactory = NULL;
	IDBPromptInitialize* pIDBPromptInitialize = NULL;
	
	//Obtain ClassFactory
	TESTC_(GetClassObject(CLSID_DataLinks, IID_IClassFactory, (IUnknown**)&pIClassFactory),	S_OK);

	//CreateInstance
	TESTC_(pIClassFactory->CreateInstance(NULL, IID_IDBPromptInitialize, (void**)&pIDBPromptInitialize),S_OK);

	//LockServer Combinations
	TESTC_(pIClassFactory->LockServer(TRUE),S_OK);  //Increment Count
	TESTC_(pIClassFactory->LockServer(TRUE),S_OK);  //Increment Count
	TESTC_(pIClassFactory->LockServer(FALSE),S_OK); //Decrement Count
	TESTC_(pIClassFactory->LockServer(FALSE),S_OK); //Decrement Count
	TESTC_(pIClassFactory->LockServer(TRUE),S_OK);  //Increment Count
	TESTC_(pIClassFactory->LockServer(FALSE),S_OK); //Decrement Count

CLEANUP:
	SAFE_RELEASE(pIClassFactory);
	SAFE_RELEASE_(pIDBPromptInitialize); //Verify Reference == 0
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - QI combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_8()
{ 
	TBEGIN
	IClassFactory* pIClassFactory = NULL;
	
	//Obtain ClassFactory
	TESTC_(GetClassObject(CLSID_DataLinks, IID_IClassFactory, (IUnknown**)&pIClassFactory),	S_OK);

	//QI Combinations - valid
	TESTC_(QI(pIClassFactory, IID_IUnknown),		S_OK);
	TESTC_(QI(pIClassFactory, IID_IClassFactory),	S_OK);

	//QI Combinations - invalid
	TESTC_(QI(pIClassFactory, IID_IDataInitialize),		E_NOINTERFACE);
	TESTC_(QI(pIClassFactory, IID_IDBPromptInitialize),	E_NOINTERFACE);

	//QI Combinations - NULL
	//	S_OK if the interface is supported, E_NOINTERFACE if not.
	// MSDN does not specify that provider is required to return E_INVALIDARG if invalid
	// parameters are specified. So, seems like both E_INVALIDARG or E_POINTER are valid.
	TEST2C_(pIClassFactory->QueryInterface(IID_IUnknown,			NULL), E_INVALIDARG, E_POINTER);
	TEST2C_(pIClassFactory->QueryInterface(IID_IClassFactory,	NULL), E_INVALIDARG, E_POINTER);


CLEANUP:
	SAFE_RELEASE(pIClassFactory);  
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc General - IClassFactory - AddRef/Release combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_9()
{ 
	TBEGIN
	IClassFactory* pIClassFactory = NULL;
	ULONG ulOrgRefCount = 0;

	//Obtain ClassFactory
	TESTC_(GetClassObject(CLSID_DataLinks, IID_IClassFactory, (IUnknown**)&pIClassFactory),	S_OK);
	ulOrgRefCount = GetRefCount(pIClassFactory);

	//AddRef/Release Combinations
	SetRefCount(pIClassFactory, 100); // AddRef 100 times
	SetRefCount(pIClassFactory, -10); // Release 10 times
	SetRefCount(pIClassFactory,   1); // AddRef   1 time
	SetRefCount(pIClassFactory, -90); // Release 90 times
	SetRefCount(pIClassFactory,  -1); // Release  1 time

	//Make sure the RefCount is back where we started...
	TESTC(ulOrgRefCount == GetRefCount(pIClassFactory));

CLEANUP:
	SAFE_RELEASE(pIClassFactory);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END








// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc General - CoCreateInstance - Aggregation - non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_10()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(CLSID_DataLinks, &Aggregate, IID_IDBPromptInitialize, (IUnknown**)&pIUnkInner), CLASS_E_NOAGGREGATION);
	Aggregate.SetUnkInner(pIUnkInner);
	
	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);


CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc General - CoCreateInstance - Aggregation - all valid combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_11()
{ 
	TBEGIN
	CAggregate Aggregate;
	IUnknown* pIUnkInner = NULL;
	
	//CreateInstance
	HRESULT hr = CreateInstance(CLSID_DataLinks, &Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBPromptInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - QI(valid riids) S_OK
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_12()
{ 
	TBEGIN
	IDBPromptInitialize* pIDBPromptInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBPromptInitialize, (IUnknown**)&pIDBPromptInitialize),	S_OK);

	//QI Invalid Combinations
	TESTC_(QI(pIDBPromptInitialize, IID_IUnknown),				S_OK);
	TESTC_(QI(pIDBPromptInitialize, IID_IDBPromptInitialize),	S_OK);

CLEANUP:
	SAFE_RELEASE_(pIDBPromptInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - QI(in valid riids) E_NOINTERFACE
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_13()
{ 
	TBEGIN
	IDBPromptInitialize* pIDBPromptInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBPromptInitialize, (IUnknown**)&pIDBPromptInitialize),	S_OK);

	//QI Invalid Combinations
	TESTC_(QI(pIDBPromptInitialize, IID_IClassFactory),		E_NOINTERFACE);
	TESTC_(QI(pIDBPromptInitialize, IID_IDBInitialize),		E_NOINTERFACE);

CLEANUP:
	SAFE_RELEASE_(pIDBPromptInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - QI(NULL) E_INVALIDARG
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_14()
{ 
	TBEGIN
	IDBPromptInitialize* pIDBPromptInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBPromptInitialize, (IUnknown**)&pIDBPromptInitialize),	S_OK);

	//QI Invalid Combinations
	//	S_OK if the interface is supported, E_NOINTERFACE if not.
	// MSDN does not specify that provider is required to return E_INVALIDARG if invalid
	// parameters are specified. So, seems like both E_INVALIDARG or E_POINTER are valid.
	TEST2C_(pIDBPromptInitialize->QueryInterface(IID_IUnknown, NULL),			E_INVALIDARG, E_POINTER);
	TEST2C_(pIDBPromptInitialize->QueryInterface(IID_IDBPromptInitialize, NULL),	E_INVALIDARG, E_POINTER);


CLEANUP:
	SAFE_RELEASE_(pIDBPromptInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc General - IUnknown - AddRef/Release combinations
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_15()
{ 
	TBEGIN
	IDBPromptInitialize* pIDBPromptInitialize = NULL;
	
	//CreateInstance
	TESTC_(CreateInstance(CLSID_DataLinks, NULL, IID_IDBPromptInitialize, (IUnknown**)&pIDBPromptInitialize),	S_OK);

	//AddRef/Release Combinations
	SetRefCount(pIDBPromptInitialize, 100); // AddRef 100 times
	SetRefCount(pIDBPromptInitialize, -10); // Release 10 times
	SetRefCount(pIDBPromptInitialize,   1); // AddRef   1 time
	SetRefCount(pIDBPromptInitialize, -90); // Release 90 times
	SetRefCount(pIDBPromptInitialize,  -1); // Release  1 time

CLEANUP:
	SAFE_RELEASE_(pIDBPromptInitialize);  //Verify Reference == 0 
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - hWnd = NULL / Invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_16()
{ 
	TBEGIN

	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptDataSource(NULL, NULL, DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Boundary -DBPROMPTOPTIONS_
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_17()
{ 
	TBEGIN

	//PromptDataSource - 
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_NONE, 0, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_INVALID, 0, NULL, NULL), E_INVALIDARG);

	//NOTE:  These are not entered as they incorrectly produce a dialog
	// which halts automation.  Until these are fixed this will always prodcue
	//an error.   Just step into these in debugging to determine they are fixed
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//E_INVALIDARG
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET | DBPROMPTOPTIONS_BROWSEONLY, 0, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET | DBPROMPTOPTIONS_WIZARDSHEET | DBPROMPTOPTIONS_BROWSEONLY, 0, NULL, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Boundary - DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_18()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	
	//Create a datasource
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK);

	//NOTE:  These are not entered as they incorrectly produce a dialog
	// which halts automation.  Until these are fixed this will always prodcue
	//an error.   Just step into these in debugging to determine they are fixed
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION - (requires *ppDataSource on input)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET | DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION, 0, NULL, NULL), E_INVALIDARG);
	
	//DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION - (error by itself, requires property Sheet or wizard sheet)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), E_INVALIDARG);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION | DBPROMPTOPTIONS_INVALID, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), E_INVALIDARG);

	//DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION - success
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET | DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - cSourceType != 0 and rgSourceType == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_19()
{ 
	TBEGIN
	
	//PromptDataSource - 
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 5, NULL, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - cSourceType == 0 and rgSourceType != NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_20()
{ 
	TBEGIN
	DBSOURCETYPE rgFilter[1] = { DBSOURCETYPE_DATASOURCE };

	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, rgFilter, NULL), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - Singularly null terminated string
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_21()
{ 
	TBEGIN
	WCHAR* pwszFilter = L"MSDASQL\0";

	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, pwszFilter), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - ppDataSource = NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_22()
{ 
	TBEGIN
		
	//PromptDataSource - 
	//Go to have to call directly - since our helper handles NULL
	TESTC_(pDBPromptInit()->PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, NULL, IID_IUnknown, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - riid = IID_INULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_23()
{ 
	TBEGIN
		
	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//PromptDataSource - 
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, NULL, IID_NULL), E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_NOINTERFACE - riid = IID_IDataInitalize
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_24()
{ 
	TBEGIN
		
	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//This code is incorrect but throws
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, NULL, IID_IDataInitialize), E_NOINTERFACE);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, NULL, IID_IDBPromptInitialize), E_NOINTERFACE);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, NULL, IID_IRowset), E_NOINTERFACE);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc Parameters - rgSourceType = contains both valid and invalid DBSOURCETYPE values
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_25()
{ 
	TBEGIN
	static const DBSOURCETYPE rgSourceType[] = { DBSOURCETYPE_DATASOURCE };
	static const int cSourceType = NUMELEM(rgSourceType);

	static const DBSOURCETYPE rgInvalidSourceType[] = { DBSOURCETYPE_DATASOURCE, DBSOURCETYPE_ENUMERATOR };
	static const int cInvalidSourceType = NUMELEM(rgInvalidSourceType);

	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//Error Cases (provider not in the list)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"ThisProviderProgIDSureHadBetterNoAlreadyExist\0\0"), DB_E_NOPROVIDERSREGISTERED);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"\0\0"), DB_E_NOPROVIDERSREGISTERED);

	//Error Cases (enumerator filter)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cInvalidSourceType, (DBSOURCETYPE*)rgInvalidSourceType, NULL), DB_E_NOPROVIDERSREGISTERED);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cInvalidSourceType, (DBSOURCETYPE*)rgInvalidSourceType, L"MSDASQL\0\0"), DB_E_NOPROVIDERSREGISTERED);

	//Success Cases (shouldn't even display the providers dialog
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"MSDASQL\0\0"), S_OK);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"MSDASQL\0MSDASQL\0\0"), S_OK);

	//Success Cases (1 provider filter)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, L"MSDAORA\0\0"), S_OK);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, L"MSDAORA\0MSDAORA\0\0"), S_OK);

	//Success Cases (SourceType filter, no progid filter)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, NULL), S_OK);

	//Success Cases (group filter)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, L"MSDAORA\0MSDASQL\0MSDAOSP\0SQLOLEDB\0"), S_OK);
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(26)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszProviderFilter - Contains both valid and invalid progIDs
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_26()
{ 
	TBEGIN
	static const DBSOURCETYPE rgSourceType[] = { DBSOURCETYPE_DATASOURCE, DBSOURCETYPE_DATASOURCE_TDP, DBSOURCETYPE_DATASOURCE_MDP };
	static const int cSourceType = NUMELEM(rgSourceType);

	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//Error Cases (provider not in the list)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"ThisProviderProgIDSureHadBetterNoAlreadyExist\0\0"), DB_E_NOPROVIDERSREGISTERED);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"\0\0"), DB_E_NOPROVIDERSREGISTERED);

	//Success Cases (duplicate providers)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"MSDASQL\0\0"), S_OK);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, L"MSDASQL\0MSDASQL\0\0"), S_OK);

	//Success Cases (1 provider filter)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, L"MSDAORA\0\0"), S_OK);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, L"MSDAORA\0MSDAORA\0\0"), S_OK);

	//Success Cases (group filter)
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, cSourceType, (DBSOURCETYPE*)rgSourceType, L"MSDAORA\0MSDASQL\0MSDAOSP\0SQLOLEDB\0"), S_OK);
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(27)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszProviderFilter - Contains one large invalid progID
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_27()
{ 
	TBEGIN
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );
	WCHAR wszBuffer[MAX_QUERY_LEN+1];

	//PromptDataSource - 
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//LargeProvider ProgID
	LocaleInfo.MakeUnicodeIntlString(wszBuffer, MAX_QUERY_LEN);
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_WIZARDSHEET, 0, NULL, wszBuffer), DB_E_NOPROVIDERSREGISTERED);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(28)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - verify untouched after numerous error conditions
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_28()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	IDBProperties* pIDBProperties = NULL;
	
	//Create a datasource
	TESTC_(CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES),S_OK);
	pIDBProperties = (IDBProperties*)pIDBInitialize;

	//PromptDataSource
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//SuccessFul Case
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties), S_OK);
	//Make sure its the correct new interface
	TESTC_(pIDBProperties->GetProperties(0, NULL, 0, NULL),E_INVALIDARG);

	//Error Case
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDataInitialize, (IUnknown**)&pIDBProperties), E_NOINTERFACE);
	
CLEANUP:
	//NOTE:  Purposely leave this object (DataSource) open.  Service Components have issues with 
	//DataSource still in the pools when the dll is detached.  This should cover this senario.
	//We will also leave the IDataInitialize object arround as well to help improve reproducability
	//of this issue...
//	SAFE_RELEASE_DSO(pIDBProperties);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(29)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - contains enumerator ojbect
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_29()
{ 
	TBEGIN
	IUnknown* pIUnknown  = NULL;
	
	//Create an Enumerator
	TESTC_PROVIDER(CoCreateInstance(CLSID_MSDASQL_ENUMERATOR, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)&pIUnknown)==S_OK);

	//PromptDataSource
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//E_NOINTERFACE - *ppDataSource = non-DataSource object
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IUnknown, (IUnknown**)&pIUnknown), E_NOINTERFACE);
	
CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(30)
//*-----------------------------------------------------------------------
// @mfunc Parameters - ppDataSource - contains non-DataSource Object
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_30()
{ 
	TBEGIN
	CRowset RowsetA;
	IRowset* pRowset = NULL;

	//Create a datasource
	TESTC_(RowsetA.CreateRowset(),S_OK);

	//PromptDataSource
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//E_NOINTERFACE
	//The passed in *ppDataSource is not a TDataSource object...
	pRowset = RowsetA.pIRowset();
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBProperties, (IUnknown**)&pRowset), E_NOINTERFACE);
	
CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(31)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Verify DCM is aggregated
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_31()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	IUnknown* pIUnknown = NULL;
	
	//NOTE:  These are not entered as they incorrectly produce a dialog
	// which halts automation.  Until these are fixed this will always prodcue
	//an error.   Just step into these in debugging to determine they are fixed
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//DBPROMPTOPTIONS_PROPERTYSHEET - success
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Other interfaces
	TESTC_(QI(pIDBInitialize, IID_IService), IsDCM(pIDBInitialize) ? S_OK : E_NOINTERFACE);

	//Verify interfaces... (all TDataSource interfaces)
	TESTC(DefaultObjectTesting(pIDBInitialize, DATASOURCE_INTERFACE, FALSE/*fInitialized*/));

	//SetProperties, Initialize
	TESTC_(InitializeDataSource(pIDBInitialize, CREATEDSO_SETPROPERTIES | CREATEDSO_INITIALIZE),S_OK);

	//Verify interfaces... (all TDataSource interfaces)
	TESTC(DefaultObjectTesting(pIDBInitialize, DATASOURCE_INTERFACE, TRUE/*fInitialized*/));

	//PromptDataSource on an Initialized DSO
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

CLEANUP:
	SAFE_RELEASE_DSO(pIUnknown);
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(32)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Verify DCM is aggregated with Aggregated DataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_32()
{ 
	TBEGIN
	CAggregate Aggregate(pDBPromptInit());
	IDBInitialize* pIDBInitialize = NULL;
	IUnknown* pIUnkInner = NULL;
	CAggregate Aggregate2;
	IUnknown* pIUnkInner2 = NULL;

	//Create an aggregated datasource
	HRESULT hr = CreateDataSource(&Aggregate, IID_IUnknown, (IUnknown**)&pIUnkInner, CREATEDSO_SETPROPERTIES);
	Aggregate.SetUnkInner(pIUnkInner);

	//Aggregation
	TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize, (IUnknown**)&pIDBInitialize));

	//PromptDataSource
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	
	//SuccessFul Case - passing in Aggregated DSO
	TESTC_(PromptDataSource(NULL, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

	//Verify we can get the Aggregated DataSource...
	TESTC_(QI(pIDBInitialize, IID_IAggregate),S_OK);

	//PromptDataSource
	hr = PromptDataSource(&Aggregate2, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IUnknown, (IUnknown**)&pIUnkInner2);
	Aggregate2.SetUnkInner(pIUnkInner);
	
	//Aggregation
	TESTC_PROVIDER(Aggregate2.VerifyAggregationQI(hr, IID_IDBInitialize));

CLEANUP:
	SAFE_RELEASE(pIUnkInner2);
	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(33)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - asking for non-IUnknown
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_33()
{ 
	TBEGIN
	CAggregate Aggregate(pDBPromptInit());
	IUnknown* pIUnkInner = NULL;
	
	//PromptDataSource
	if(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS)
	{
		TESTC_(PromptDataSource(&Aggregate, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IDBInitialize, &pIUnkInner), DB_E_NOAGGREGATION);
		Aggregate.SetUnkInner(pIUnkInner);
	}
	else
	{
		TESTC_(PromptDataSource(&Aggregate, GetDesktopWindow(), DBPROMPTOPTIONS_NONE, 0, NULL, NULL, IID_IDBInitialize, &pIUnkInner), DB_E_NOAGGREGATION);
		Aggregate.SetUnkInner(pIUnkInner);
	}

	//Inner object cannot RefCount the outer object - COM rule for CircularRef
	COMPARE(Aggregate.GetRefCount(), 1);
	TESTC(pIUnkInner == NULL);

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(34)
//*-----------------------------------------------------------------------
// @mfunc Aggregation - Verify inner and outer
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_34()
{ 
	TBEGIN
	CAggregate Aggregate(pDBPromptInit());
	IUnknown* pIUnkInner = NULL;
	
	//PromptDataSource
	if(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS)
	{
		HRESULT hr = PromptDataSource(&Aggregate, GetDesktopWindow(), DBPROMPTOPTIONS_PROPERTYSHEET, 0, NULL, NULL, IID_IUnknown, &pIUnkInner);
		Aggregate.SetUnkInner(pIUnkInner);

		//Aggregation
		TESTC_PROVIDER(Aggregate.VerifyAggregationQI(hr, IID_IDBInitialize));
	}
	else
	{
		TESTC_(PromptDataSource(&Aggregate, GetDesktopWindow(), DBPROMPTOPTIONS_NONE, 0, NULL, NULL, IID_IUnknown, &pIUnkInner), E_INVALIDARG);
		Aggregate.SetUnkInner(pIUnkInner);

		//Inner object cannot RefCount the outer object - COM rule for CircularRef
		COMPARE(Aggregate.GetRefCount(), 1);
		TESTC(pIUnkInner == NULL);
	}

CLEANUP:
	SAFE_RELEASE(pIUnkInner);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(35)
//*-----------------------------------------------------------------------
// @mfunc Stress - Verify FreeThreaded
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptDataSource::Variation_35()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	WCHAR* pwszInitString = NULL;
	IDBInitialize* pIDBInitialize = NULL;
	
	//Create Uninitialized DSO
	CreateDataSource(NULL, IID_IDBInitialize, (IUnknown**)&pIDBInitialize, CREATEDSO_SETPROPERTIES);

	//Input DataSource - no need to AddRef, since were not using a different prov
	IUnknown* pDataSource1 = pIDBInitialize;
	IUnknown* pDataSource2 = pIDBInitialize;
	IUnknown* pDataSource3 = NULL;
	
	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, (void*)DBPROMPTOPTIONS_PROPERTYSHEET, (void*)&IID_IDBInitialize, &pDataSource1, (void*)S_OK };
	THREADARG T2Arg = { this, (void*)DBPROMPTOPTIONS_PROPERTYSHEET, (void*)&IID_IDBProperties, &pDataSource2, (void*)S_OK };
	THREADARG T3Arg = { this, (void*)DBPROMPTOPTIONS_PROPERTYSHEET, (void*)&IID_IUnknown,		&pDataSource3, (void*)S_OK };

	//Verify DCM is FreeThreaded or Both
	TESTC(VerifyThreadingModel(CLSID_DataLinks, L"Both") || VerifyThreadingModel(CLSID_DataLinks, L"Free"));

	//PromptDataSource
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_PromptDataSource, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_PromptDataSource, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_PromptDataSource, &T3Arg);

	START_THREADS();
	END_THREADS();	

	//Make sure nothing changed after the threads existed...
	TESTC(VerifyEqualInterface(pDataSource1, pDataSource2));
	TESTC(!VerifyEqualInterface(pDataSource2, pDataSource3));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_RELEASE_DSO(pDataSource3);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCPromptDataSource::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCPromptFileName)
//*-----------------------------------------------------------------------
//| Test Case:		TCPromptFileName - Test IDBPromptInitialize::PromptFileName
//| Created:  	1/14/98
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPromptFileName::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		return TRUE;
	} 
	return FALSE;
} 





// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - hWnd == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_1()
{ 
	TBEGIN
		
	//PromptFileName
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_BROWSEONLY, NULL, NULL, NULL), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - hWnd == invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_2()
{ 
	TBEGIN
		
	//PromptFileName
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_BROWSEONLY, NULL, NULL, NULL), S_OK);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - dwPromptOptions
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_3()
{ 
	TBEGIN
		
	//PromptFileName
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION, NULL, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_PROPERTYSHEET, NULL, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_WIZARDSHEET, NULL, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_WIZARDSHEET | DBPROMPTOPTIONS_BROWSEONLY, NULL, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_PROPERTYSHEET | DBPROMPTOPTIONS_BROWSEONLY, NULL, NULL, NULL), E_INVALIDARG);
	TESTC_(PromptFileName(NULL, DBPROMPTOPTIONS_INVALID, NULL, NULL, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc Boundary - E_INVALIDARG - ppwszSelectedFile == NULL
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_4()
{ 
	TBEGIN
		
	//PromptFileName - our helper handles NULL so we have to call directly
	TESTC_(pDBPromptInit()->PromptFileName(GetDesktopWindow(), DBPROMPTOPTIONS_BROWSEONLY, NULL, NULL, NULL), E_INVALIDARG);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitDirectory - non existent
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_5()
{ 
	TBEGIN
	WCHAR wszValue[MAX_QUERY_LEN+1];
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
		
	//PromptFileName - our helper handles NULL so we have to call directly
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptFileName(GetDesktopWindow(), DBPROMPTOPTIONS_BROWSEONLY, wszValue, NULL, NULL), STG_E_PATHNOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitDirectory - invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_6()
{ 
	TBEGIN
		
	//PromptFileName
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptFileName(GetDesktopWindow(), DBPROMPTOPTIONS_BROWSEONLY, FILENAME_INVALID, NULL, NULL), STG_E_INVALIDNAME);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitFile - non existent
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_7()
{ 
	TBEGIN
	WCHAR wszValue[MAX_QUERY_LEN+1];
	CLocaleInfo LocaleInfo( GetUserDefaultLCID() );
	LocaleInfo.MakeUnicodeIntlString(wszValue, MAX_QUERY_LEN);
		
	//PromptFileName
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptFileName(GetDesktopWindow(), DBPROMPTOPTIONS_BROWSEONLY, NULL, wszValue, NULL), STG_E_FILENOTFOUND);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Parameters - pwszInitFile - invalid
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_8()
{ 
	TBEGIN
		
	//PromptFileName
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);
	TESTC_(PromptFileName(GetDesktopWindow(), DBPROMPTOPTIONS_BROWSEONLY, NULL, FILENAME_INVALID, NULL), STG_E_INVALIDNAME);

CLEANUP:
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Stress - Verify FreeThreaded
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPromptFileName::Variation_9()
{ 
	TBEGIN
	INIT_THREADS(THREE_THREADS);	
	WCHAR* pwszSelectedFile1 = NULL;
	WCHAR* pwszSelectedFile2 = NULL;
	WCHAR* pwszSelectedFile3 = NULL;

	//Setup Thread Arguments
	//All threads need to have a different pItf to put the pointer,
	//Otherwsie they will all end up in the same location...
	THREADARG T1Arg = { this, (void*)DBPROMPTOPTIONS_NONE, &pwszSelectedFile1, (void*)S_OK };
	THREADARG T2Arg = { this, (void*)DBPROMPTOPTIONS_NONE, &pwszSelectedFile2, (void*)S_OK };
	THREADARG T3Arg = { this, (void*)DBPROMPTOPTIONS_NONE, &pwszSelectedFile3, (void*)S_OK };

	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//Create Threads
	CREATE_THREAD(THREAD_ONE,	Thread_PromptFileName, &T1Arg);
	CREATE_THREAD(THREAD_TWO,	Thread_PromptFileName, &T2Arg);
	CREATE_THREAD(THREAD_THREE, Thread_PromptFileName, &T3Arg);

	START_THREADS();
	END_THREADS();	

	//Verify DCM is FreeThreaded or Both
	TESTC(VerifyThreadingModel(CLSID_DataLinks, L"Both") || VerifyThreadingModel(CLSID_DataLinks, L"Free"));
	TESTC(CompareStrings(pwszSelectedFile1, pwszSelectedFile2));
	TESTC(CompareStrings(pwszSelectedFile1, pwszSelectedFile3));

CLEANUP:
	SAFE_FREE(pwszSelectedFile1);
	SAFE_FREE(pwszSelectedFile2);
	SAFE_FREE(pwszSelectedFile3);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCPromptFileName::Terminate()
{ 
// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END


// {{ TCW_TC_PROTOTYPE(TCPooling)
//*-----------------------------------------------------------------------
//| Test Case:		TCPooling - Test Session Pooling
//| Created:  	2/11/99
//*-----------------------------------------------------------------------

//*-----------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPooling::Init()
{ 
	// {{ TCW_INIT_BASECLASS_CHECK
	if(CDBInit::Init())
	// }}
	{ 
		// TO DO:  Add your own code here 
		return TRUE;
	} 
	return FALSE;
} 






// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc GetDataSource - simple complete case
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_1()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_2()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc SetProperties - After GetDataSource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_3()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ULONG i,cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	WCHAR* pwszInitString = NULL;
	DBPROPSET propSet = { NULL, 0 };

	//Obtain the Initialization properties, but only use the first property...
	TESTC(GetInitProps(&cPropSets, &rgPropSets));
	if(cPropSets && rgPropSets)
	{
		propSet.cProperties = 1;
		propSet.rgProperties = rgPropSets[0].rgProperties;
		propSet.guidPropertySet = rgPropSets[0].guidPropertySet;
	}

	//Create a string for GetDataSource with just the first property for connection
	pwszInitString = CreatePropString(1, &propSet, TRUE/*fIncludeProvider*/);

	//NOTE: Do this in a loop so we test after connections are in the pool (CPU+1)
	for(i=0; i<100; i++)
	{
		//GetDataSource
		//This should "stress" the prototype, since we are creating a prototype which is a subset of the 
		//initstring, and then adding properties after the prototype...
		TESTC_(GetDataSource(NULL, pwszInitString, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

		//Now set the additional properties (beyond the first one)
		TESTC_(SetProperties(pIDBInitialize, cPropSets, rgPropSets),S_OK);

		//Add this DataSource to our internal pool tracker
		SAFE_RELEASE_DSO(pIDBInitialize);

		//Now try to connect again, this time without specifing all properties...

	}

CLEANUP:
	SAFE_FREE(pwszInitString);
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc SetProperties - More than once
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_4()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc SetProperties - no-op
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_5()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	DBPROPSET propSet = { NULL, 0 };

	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);


	//Now set additional properties, no-op(s)
	TESTC_(SetProperties(pIDBInitialize, 0, NULL),S_OK);
	TESTC_(SetProperties(pIDBInitialize, 0, INVALID(DBPROPSET*)),S_OK);
	TESTC_(SetProperties(pIDBInitialize, 1, &propSet),S_OK);

	propSet.rgProperties = INVALID(DBPROP*);
	TESTC_(SetProperties(pIDBInitialize, 1, &propSet),S_OK);

	//Invalid Senarios
	TESTC_(SetProperties(pIDBInitialize, 1, NULL),E_INVALIDARG);
	propSet.cProperties = 1;
	propSet.rgProperties = NULL;
	TESTC_(SetProperties(pIDBInitialize, 1, &propSet),E_INVALIDARG);

	//Double check valid properties after errors have occurred...
	TESTC(GetInitProps(&cPropSets, &rgPropSets));
	TESTC_(SetProperties(pIDBInitialize, cPropSets, rgPropSets),S_OK);

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_6()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc Stress - 1 Pool - with hundreads of requests for the same datasource
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_7()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;

	for(ULONG i=0; i<100; i++)
	{
		//GetDataSource (use the Default InitString)
		TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize), S_OK);

		//Initialize so it can be pooled...
		TESTC(VerifyInitialize(pIDBInitialize));

		//Add this DataSource to our internal pool tracker
		TESTC_(ReleaseDataSource((IUnknown**)&pIDBInitialize),S_OK);
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Stress - Many Pools - each with interleaving requests
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_8()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_9()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc Session Properties - Set Session Properties, redraw from pool, verify reset to defaults
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_10()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ISessionProperties* pISessionProperties = NULL;
	ULONG iDataSource,iPropSet,iProp = 0;
	ULONG cPropSets = 0;
	DBPROPSET* rgPropSets = NULL;
	ULONG cInputPropSets = 0;
	DBPROPSET* rgInputPropSets = NULL;
	HRESULT hr = S_OK;
	ULONG cActuallySet = 0;

	//Ensure we have objects in the pool...
	TESTC_(CreatePool(m_pwszProperties),S_OK);
	
	//Now draw a DataSource from the pool...
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
	TESTC_(InitializeDataSource(pIDBInitialize),S_OK);

	//Obtain Session Properties
	TESTC_(CreateNewSession(pIDBInitialize, IID_ISessionProperties, (IUnknown**)&pISessionProperties),S_OK);
	TESTC_(pISessionProperties->GetProperties(0, NULL, &cPropSets, &rgPropSets),S_OK);
	
	//Try and set some session properties (all those that are supported), 
	//to something other than non-default values.  
	for(iPropSet=0; iPropSet<cPropSets; iPropSet++)
	{
		DBPROPSET* pPropSet = &rgPropSets[iPropSet];
		for(iProp=0; iProp<pPropSet->cProperties; iProp++)
		{
			DBPROP* pProp = &pPropSet->rgProperties[iProp];
			
			//Loop through numerous times setting properties, trying to actually
			//set some successful values...
			for(ULONG i=0; i<10; i++)
			{
				VARIANT vVariant;
				VariantInit(&vVariant);

				//If we can create a value for this property
				GetSomePropValue(&vVariant, pProp->dwPropertyID, pPropSet->guidPropertySet, V_VT(&pProp->vValue), TRUE/*fValid*/);
				
				//Create a property with this value
				::SetProperty(pProp->dwPropertyID, pPropSet->guidPropertySet, &cInputPropSets, &rgInputPropSets, &vVariant, DBTYPE_VARIANT);

				//Set the value...
				//If it succeeds, then we have a "good" testcase senario where
				//there existed a property on the session that was not the default,
				//and then we can correctly see that it got reset
				TEST3C_(hr = pISessionProperties->SetProperties(cInputPropSets, rgInputPropSets),S_OK,DB_S_ERRORSOCCURRED,DB_E_ERRORSOCCURRED);
				if(SUCCEEDED(hr))
					cActuallySet++;
				
				VariantClear(&vVariant);
				::FreeProperties(&cInputPropSets, &rgInputPropSets);
			}
		}
	}

	//Now that we have done all that release everything, and reobtain DSOs from the pool making sure
	//they have the defaults for the session properties...
	SAFE_RELEASE(pISessionProperties);
	TESTC_(ReleaseDataSource((IUnknown**)&pIDBInitialize),S_OK);

	//If none of our property values could be set, inform the user, so they are aware
	//we couldn't really verify this variation
	if(!cActuallySet)
		TOUTPUT(L"Unable to set any Session Properties, thus unable to verify reset correctly...");

	//Now ask for DataSources (drawn from the pool...)
	//NOTE: We loop through a few DataSources, since the one we get from the pool might not be
	//same one we set properties one, and it would luck out...
	for(iDataSource=0; iDataSource<10; iDataSource++)
	{
		//Obtain a DataSource
		TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
		TESTC_(InitializeDataSource(pIDBInitialize),S_OK);
			
		//Create a session
		TESTC_(CreateNewSession(pIDBInitialize, IID_ISessionProperties, (IUnknown**)&pISessionProperties),S_OK);

		//Verify all properties are the default values on the session
		TESTC(AreDefaultValues(pISessionProperties, cPropSets, rgPropSets));

		//We are done with this datasource...
		TESTC_(ReleaseDataSource((IUnknown**)&pIDBInitialize),S_OK);
		SAFE_RELEASE(pISessionProperties);
	}

CLEANUP:
	::FreeProperties(&cPropSets, &rgPropSets);
	::FreeProperties(&cInputPropSets, &rgInputPropSets);
	SAFE_RELEASE(pISessionProperties);
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_11()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END








// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Timeout - Verify SPTimeOut - Registry Setting
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_12()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	DWORD dwReg_PoolTimeout = 0;
	WCHAR wszBuffer[MAX_NAME_LEN];
	WCHAR* pwszCLSID = NULL;
	CPool cPool;
	
	//Ensure we have objects in the pool...
	TESTC_(CreatePool(m_pwszProperties),S_OK);

	//Obtain another DataSource
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
	
	//Obtain pooling interface (hooks)
	TESTC_PROVIDER(cPool.Create(pIDBInitialize)==S_OK);


	//Obtain the Registry Setting...
	//CLSID\<Provider GUID>\SPTimeOut - DWORD Value...
	TESTC_(StringFromCLSID(PROVIDER_CLSID, &pwszCLSID), S_OK);
	swprintf(wszBuffer, L"CLSID\\%s", pwszCLSID);
	if(FAILED(GetRegEntry(HKEY_CLASSES_ROOT, wszBuffer, L"SPTimeOut", &dwReg_PoolTimeout)))
		dwReg_PoolTimeout = 60; //Default	

	//Make sure the Pool Timeout matches the Registry setting...
	TESTC(cPool.GetPoolTimeout() == dwReg_PoolTimeout);

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	SAFE_FREE(pwszCLSID);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Timeout - Verify SPTimeOut - intervals
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_13()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;

	CPool cPool;
	CPerfTimer cPoolTimer;
	CPerfTimer cConnectionTime;
	DWORD dwPoolTimeout = 0;
	ULONG cSampling = 1;

	//Ensure we have objects in the pool...
	TESTC_(CreatePool(m_pwszProperties),S_OK);

	//Obtain another DataSource
	cPoolTimer.Start();
	cConnectionTime.Start();
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
	cConnectionTime.Stop();
	
	//Obtain pooling interface (hooks)
	TESTC_PROVIDER(cPool.Create(pIDBInitialize)==S_OK);
	dwPoolTimeout = cPool.GetPoolTimeout();
	SAFE_RELEASE_DSO(pIDBInitialize);
	cPoolTimer.Stop();

	while(cPoolTimer.GetSeconds() < dwPoolTimeout)
	{
		Sleep(1000);
		cPoolTimer.Stop();

		//Make sure that before timeout has expired that connections can be made.
		//This is a little tricky, as causing a new connection will end up reseting the 
		//pool timeout.  So we don't want to do this tool many times...
		if(cSampling <=4)
		{
			if(cPoolTimer.GetSeconds() >= (dwPoolTimeout - (dwPoolTimeout / cSampling)))
			{
				//This should a good "Sampling" of connections before the pool timeout at:
				//60-60/1	=	(0 seconds)
				//60-60/2	=	(30 seconds)
				//60-60/3	=	(40 seconds)
				//60-60/4	=	(45 seconds)

				//Make sure the connection succeeds and if drawn the pool
				TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
				TESTC_(InitializeDataSource(pIDBInitialize),S_OK);
				
				if(!DrawnFromPool(pIDBInitialize))
					TWARNING("Not Drawn From Pool? Sampling at " << cPoolTimer.GetSeconds() << "/" << dwPoolTimeout << " seconds");
				SAFE_RELEASE(pIDBInitialize);

				cPoolTimer.Restart();
				cSampling++;
			}
		}
	}

	Sleep((ULONG)cConnectionTime.GetSeconds() * 1000);

	//Pool Timeout has expired, 
	//make sure new connection does not come from the pool...
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
	TESTC_(InitializeDataSource(pIDBInitialize),S_OK);
//	TESTC(CreatedFromPool(pIDBInitialize));
//	TESTC(!DrawnFromPool(pIDBInitialize));

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END




// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Timeout - Verify SPTimeout - tight loop
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_14()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	ULONG i=0;

	//Ensure we have objects in the pool...
	TESTC_(CreatePool(m_pwszProperties),S_OK);

	//Loop and obtain numerous connections
	for(i=0; i<100; i++)
	{
		//Make sure the connection succeeds and if drawn the pool
		TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
		TESTC_(InitializeDataSource(pIDBInitialize),S_OK);
		
		if(!DrawnFromPool(pIDBInitialize))
			TWARNING("Not Drawn From Pool [" << i << "]");
		SAFE_RELEASE_DSO(pIDBInitialize);
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Timeout - Verify Retry Wait, ExpBackOff - Registry Setting
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_15()
{ 
	TBEGIN
	IDBInitialize* pIDBInitialize = NULL;
	DWORD dwReg_RetryWait;
	DWORD dwReg_BackOff;
	CPool cPool;
	
	//TODO: The RetryTimeout should be part of the PoolMngr not the poolinfo.
	//Quite a bit more difficult to use to first obtain/draw a DSO from the pool
	//to figure out and set global counts.

	//Ensure we have objects in the pool...
	TESTC_(CreatePool(m_pwszProperties),S_OK);

	//Obtain another DataSource
	TESTC_(GetDataSource(NULL, m_pwszProperties, IID_IDBInitialize, (IUnknown**)&pIDBInitialize),S_OK);
		
	//Initialize so it can be pooled...
	TESTC(VerifyInitialize(pIDBInitialize));

	//Obtain pooling interface (hooks)
	if(cPool.Create(pIDBInitialize)==S_OK)
	{
		//First verify that the "Retry Wait" and "ExpBackOff" pooling registry settings
		//match what is entered in the registry
		//NOTE: The keys may not exist, so defaults will be used...
		if(SUCCEEDED(GetRegEntry(HKEY_LOCAL_MACHINE, REGKEY_POOLING, REGKEY_POOLING_RETRY, &dwReg_RetryWait)))
		{
			TESTC(cPool.GetRetryTimeout()	== dwReg_RetryWait);
		}
		else
		{
			TESTC(cPool.GetRetryTimeout()	== DEFAULT_POOLING_RETRY);
		}

		//TODO: waiting for new test hook to programmically verify this...
		if(SUCCEEDED(GetRegEntry(HKEY_LOCAL_MACHINE, REGKEY_POOLING, REGKEY_POOLING_BACKOFF, &dwReg_BackOff)))
		{
	//		TESTC(cPool.GetExpBackOff()		== dwReg_BackOff);
		}
		else
		{
	//		TESTC(cPool.GetExpBackOff()		== default);
		}
	}

CLEANUP:
	SAFE_RELEASE_DSO(pIDBInitialize);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END






// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Timeout - Verify Retry Wait, ExpBackOff
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_16()
{ 
	TBEGIN
//	DWORD dwReg_RetryWait;
//	DWORD dwReg_BackOff;
	
	ULONG		i,cDSOs = 0;
	IUnknown**	rgDSOs = NULL;
	HRESULT hr = S_OK;
	const int MAX_DSO = 200;

	//In order to test the "Retry Wait" and "ExpBackOff" we have to actually cause a server error.
	//It can't just be a property/value error, since different properties or value would be a different
	//pool.  It has to be a senario where at some point with the same properties/value succeeds,
	//and at some other time fails.  The failure will trigger a "retry wait" where all attempts on
	//the pool will error imiediately, since timeouts of the actually server are extremly slow.

	//The only real way I can determine to cause a server error from the test, is to actually
	//max out the connections.  We don't normally want to run this in the automation tests
	//since other tests will fail when trying to hit this server, so only do the maxing out 
	//in the test debug mode (ie: DEBUGMODE=DIALOGS in the initstring)
	TESTC_PROVIDER(GetModInfo()->GetDebugMode() & DEBUGMODE_DIALOGS);

	//We need to ensure the following:
	//	1.	"Timeout" pool does not affect other pools.  Connections can still be made to
	//		other pools when a different pool is in a timeout state
	//  2.	The "Retry Timeout" does end (at the end of the current ExpBackOff) and a 
	//		successfuly connection can be made from that point one...
	//	3.  That the "ExpBackOff" is reset to the default on a successful connection
	//	4.  That the timeout never exceeds the max = "Retry Wait"
	//  5.  The the values in the registry are actually looked at and not just the defaults
	
	//Max out the connections
	//NOTE: Some providers may not have a connection limit, so prevent infinite loop...
	while(SUCCEEDED(hr) && cDSOs < MAX_DSO)
	{
		SAFE_REALLOC(rgDSOs, IUnknown*, cDSOs+1);
		rgDSOs[cDSOs] = NULL; //GetDataSource looks at the non-NULL input, so this MUST be initialized.

		//NOTE: DB_SEC_E_AUTH_FAILED, does not trigger the Retry Pool Logic.
		TEST2C_(hr = GetDataSource(NULL, m_pwszProperties, IID_IUnknown, (IUnknown**)&rgDSOs[cDSOs]), S_OK, E_FAIL);
	
		if(SUCCEEDED(hr))
		{
			//Initialize
			TEST2C_(hr = InitializeDataSource(rgDSOs[cDSOs]),S_OK,E_FAIL);
			if(SUCCEEDED(hr))
				cDSOs++;
		}
	}
	
	//If we were unable to cause a server error, there is nothing more we can really do...
	TESTC_PROVIDER(cDSOs < MAX_DSO);


CLEANUP:
	//Release all the DSOs
	for(i=0; i<cDSOs; i++)
		SAFE_RELEASE_DSO(rgDSOs[i]);
	SAFE_FREE(rgDSOs);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Empty
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_17()
{ 
	// TO DO:  Add your own code here 
	return TRUE;
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc MultiUser - Apartment Model Pooled DSO, make sure its not reused outside that apartment
//
// @rdesc TEST_PASS or TEST_FAIL 
//
int TCPooling::Variation_18()
{ 
	TBEGIN
	INIT_THREADS(MAX_THREADS);	
	WCHAR* pwszInitString = NULL;

	//Obtain InitString
	GetInitString(pIDBInit(), PASSWORD_INCLUDED, &pwszInitString);

	//Setup Thread Arguments
	THREADARG T1Arg = { this, (void*)FALSE/*FreeThreaded*/, pwszInitString, (void*)S_OK };
	
	//Create Threads
	CREATE_THREADS(Thread_CreatePool, &T1Arg);
	START_THREADS();
	END_THREADS();	

	//Now that we have created numerous threads in the pool
	//Try and actually draw one from the pool, making sure it doesn't use an apartment
	//model DSO within this Thread, (different than the apartment it ws created in).
	TESTC_(CreatePool(pwszInitString),S_OK);

CLEANUP:
	SAFE_FREE(pwszInitString);
	TRETURN
} 
// }} TCW_VAR_PROTOTYPE_END


// {{ TCW_TERMINATE_METHOD
//*-----------------------------------------------------------------------
// @mfunc TestCase Termination Routine 
//
// @rdesc TEST_PASS or TEST_FAIL 
//
BOOL TCPooling::Terminate()
{ 
	// TO DO:  Add your own code here 

// {{ TCW_TERM_BASECLASS_CHECK2
	return(CDBInit::Terminate());
} 	// }}
// }} TCW_TERMINATE_METHOD_END
// }} TCW_TC_PROTOTYPE_END

