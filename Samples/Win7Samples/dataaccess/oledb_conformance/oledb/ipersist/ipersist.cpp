//--------------------------------------------------------------------
// Microsoft OLE/DB Testing
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module IPERSIST.CPP | Test Module for IPersistFile.
//

#include "modstandard.hpp"
#define  DBINITCONSTANTS	// Must be defined to initialize constants in OLEDB.H
#define  INITGUID
#include "ipersist.h"
#include <direct.h>
#include <io.h>

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Module Values
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// {{ TCW_MODULE_GLOBALS
DECLARE_MODULE_CLSID = { 0xc7e42c91, 0xbed8, 0x11ce, { 0xa9, 0xd4, 0x00, 0xaa, 0x00, 0x3e, 0x77, 0x8a }};
DECLARE_MODULE_NAME("IPersist");
DECLARE_MODULE_OWNER("Microsoft");
DECLARE_MODULE_DESCRIP("Test Module for IPersist and IPersistFile");
DECLARE_MODULE_VERSION(823308657);
// }}


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
		pThisTestModule->m_pVoid = new CTable((IUnknown *)pThisTestModule->m_pIUnknown2, 
				(LPWSTR)gwszModuleName);

		if (!pThisTestModule->m_pVoid)
		{
			odtLog << wszMemoryAllocationError;
			return FALSE;
		}

		//If we made it this far, everything has succeeded
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
	//We still own the table since all of our testcases
	//have only used it and not deleted it.
	if (pThisTestModule->m_pVoid)
	{
		// delete CTable object
		delete (CTable*)pThisTestModule->m_pVoid;
		pThisTestModule->m_pVoid = NULL;
	}
	
	return ModuleReleaseDBSession(pThisTestModule);
}	


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Base Class Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// @class TCPersist Base Class for all IPersist Testcases
class TCPersist : public CDataSourceObject
{
	public:
		// @cmember Constructor
		TCPersist(const LPWSTR strTestCaseName) : CDataSourceObject(strTestCaseName)
		{		
			m_pIPersist	= NULL;
		};
	
		// @cmember Destructor
		virtual ~TCPersist(){};	

	protected:
		// @cmember IPersistFile Interface
		IPersist * m_pIPersist;

		// @cmember Specific initialization needed for inheriting testcases		
		BOOL Init();
		// @cmember Specific clean up needed for inheriting testcases		
		BOOL Terminate();
};

// @class TCPersistFile Base Class for all IPersistFile Testcases
class TCPersistFile : public CDataSourceObject
{
	public:
		// @cmember Constructor
		TCPersistFile(const LPWSTR strTestCaseName) : CDataSourceObject(strTestCaseName)
		{		
			m_pDSOIPersistFile		= NULL;
			m_pCmdIPersistFile		= NULL;
			m_pIDBCreateSession		= NULL;
			m_pIOpenRowset			= NULL;
			m_pIDBCreateCommand		= NULL;
			m_pICommand				= NULL;
			m_pICommandText			= NULL;
			m_pIAccessor			= NULL;
			m_pTable				= NULL;
			m_pwszDefaultExt		= NULL;
			m_hr					= E_FAIL;
		};
	
		// @cmember Destructor
		virtual ~TCPersistFile(){};	

	protected:
		// @cmember IPersistFile Interface
		IPersistFile * 			m_pDSOIPersistFile;
		IPersistFile *			m_pCmdIPersistFile;
		IDBCreateSession *		m_pIDBCreateSession;
		IOpenRowset *			m_pIOpenRowset;
		IDBCreateCommand * 		m_pIDBCreateCommand;
		ICommand	*			m_pICommand;
		ICommandText *			m_pICommandText;
		IAccessor	*			m_pIAccessor;
		CTable *				m_pTable;		
		CHAR					m_szPath[PATH_SIZE];
		CHAR					m_szFile[PATH_SIZE];
		WCHAR					m_wszFile[PATH_SIZE];
		WCHAR *					m_pwszDefaultExt;
		HRESULT					m_hr;

		// @cmember Specific initialization needed for inheriting testcases		
		BOOL Init();
		// @cmember Specific clean up needed for inheriting testcases		
		BOOL Terminate();
		// @cmember Causes Initialize to be called explicitly on the DSO		
		BOOL ExplicitInit(BOOL fPersist_Sensitive = TRUE);
		// @cmember Causes Initialize to be called implicitly on the DSO		
		BOOL ImplicitInit();
		// @cmember Uninitializes the DSO
		BOOL Uninitialize();		
		// @cmember Creates an active child command on initialized DSO,
		// and places the ICommand Interface in m_pICommand.
		BOOL CreateActiveCommand();
		// @cmember	Creates a command, saves it, then changes its command tree, 
		// cost goals and property goals. 
		BOOL ChangeCommand();
		// @cmember Gets to executed command state
		BOOL ActivateRowset();
		// @cmember Cleans up everything from CreateActiveCommand
		void CleanUpActiveCommand();
		// @cmember Cleans up everything from ActivateRowset
		void CleanUpActivateRowset();
		// @cmember Cleans up everything from ChangeCommand
		void CleanUpChangeCommand();
		// @cmember Releases all interfaces on DSO 
		void ReleaseDSO();
		// @cmember Saves the DSO to a file and then deletes the file. This is
		// used to put the DSO in a saved state for further testing.
		HRESULT	QuickSave(EDELETE_FILE eDelete, WCHAR * pwszFile, 
				BOOL fRemember = TRUE, BOOL fAddExt = FALSE, BOOL fUnInit = FALSE);
		// @cmember Releases the current DSO, gets a new one, and loads 
		// the file specifed to verify that it was saved correctly.
		// If pwszFile is NULL, the current file (m_wszFile) is loaded
		HRESULT QuickLoad(LPWSTR pwszFile = NULL);
		// @cmember Delete the file that was created
		void QuickDelete(HRESULT hr, WCHAR * pwszFile, BOOL fUnInit = TRUE);
};


// @class TCPersistFile Base Class for all IPersistFile Testcases on the command object
class TCCmdPersistFile : public TCPersistFile
{
	public:
		// @cmember Constructor
		TCCmdPersistFile(const LPWSTR strTestCaseName) : TCPersistFile(strTestCaseName)
		{			
			m_pCmdIPersistFile = NULL;
		};
	
		// @cmember Destructor
		virtual ~TCCmdPersistFile(){};	

	protected:
		// @cmember IPersistFile Interface		
		IPersistFile *	m_pCmdIPersistFile;
		// @cmember Specific initialization needed for inheriting testcases		
		BOOL Init();
		// @cmember Specific clean up needed for inheriting testcases		
		BOOL Terminate();
};

//--------------------------------------------------------------------
// @mfunc  Specific initialization needed for inheriting testcases.  If
//		   this function fails, the rest of the testcase is not executed.	
//
// @rdesc TRUE or FALSE
//
BOOL TCPersist::Init()
{
	// Get a DSO object
	if( CDataSourceObject::Init() ) 
	{				
		// Instantiate Data Source Object, get the IPersistFile interface.  
		TESTC_(CreateDataSourceObject(), S_OK);

		TESTC(VerifyInterface(m_pIDBInitialize, IID_IPersist, 
							DATASOURCE_INTERFACE, (IUnknown **)&m_pIPersist));
		return TRUE;
	}

CLEANUP:

	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Specific cleanup needed for inheriting testcases	
//
// @rdesc TRUE or FALSE
//
BOOL TCPersist::Terminate()
{				
	// Release the DSO object
	SAFE_RELEASE(m_pIPersist);
	ReleaseDataSourceObject();
	
	return CDataSourceObject::Terminate();
}

//--------------------------------------------------------------------
// @mfunc  Specific initialization needed for inheriting testcases.  If
//		   this function fails, the rest of the testcase is not executed.	
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile::Init()
{
	// Get a DSO object
	if( CDataSourceObject::Init() ) 
	{				
		// Instantiate Data Source Object, get the IPersistFile interface.  
		TESTC_(CreateDataSourceObject(), S_OK);
		TESTC(ExplicitInit());

		TESTC(VerifyInterface(m_pIDBInitialize, IID_IDBCreateSession,
					DATASOURCE_INTERFACE, (IUnknown **)&m_pIDBCreateSession));

		TESTC_(UninitializeDSO(), S_OK);

		if( VerifyInterface(m_pIDBInitialize, IID_IPersistFile, 
						DATASOURCE_INTERFACE, (IUnknown **)&m_pDSOIPersistFile) )
			CHECK(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&m_pwszDefaultExt),S_FALSE);

		// Our table already exists at the module level, so point us to it
		m_pTable=(CTable *)m_pThisTestModule->m_pVoid;	
		m_pTable->MakeTableName(NULL);

		// Get current working directory to be used for creating files
		if(!_getcwd(m_szPath, PATH_SIZE)) {
			odtLog << wszErrorFindingCurrentPath;
			return FALSE;
		}

		// Build whole file path and name
		ConvertToWCHAR(m_szPath, m_wszFile, PATH_SIZE);
		wcscat(m_wszFile, L"\\");
		if( m_pTable->GetTableName() )
			wcscat(m_wszFile, m_pTable->GetTableName());
		else
			wcscat(m_wszFile, m_pTable->GetModuleName());
		wcscat(m_wszFile, L".tst");

		return TRUE;
	}

CLEANUP:

	TESTC_(UninitializeDSO(), S_OK);
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Specific cleanup needed for inheriting testcases	
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile::Terminate()
{				
	// Free the memory
	SAFE_FREE(m_pwszDefaultExt);

	// Release the DSO object
	ReleaseDSO();
	
	// Make sure our file is deleted, this may fail
	// if it was cleaned up in the test case
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(m_wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);	   		
	
	return CDataSourceObject::Terminate();
}

//--------------------------------------------------------------------
// @mfunc Causes Initialize to be called explicitly on the DSO
//
// @rdesc HRESULT of Initialize
//
BOOL TCPersistFile::ExplicitInit(BOOL fPersist_Sensitive)
{	
	IDBProperties * pIDBProperties = NULL;
	DBPROPSET  rgPropertySets;
	DBPROP     rgProperties;

	// Initialize the DBPROP
	memset(&rgProperties, 0, sizeof(DBPROP));

	// First get an interface
	if(!m_pIDBInitialize)	
		if(!VerifyInterface(m_pDSOIPersistFile, IID_IDBInitialize, 
						DATASOURCE_INTERFACE, (IUnknown **)&m_pIDBInitialize))
			return FALSE;
		
	// If we are initialized, redo it, as it may have been caused
	// by a load or implicit init
	if( m_fInitialized )
		Uninitialize();

	// Need to check and set DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO
	if(SupportedProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, m_pIDBInitialize))
		if (SettableProperty(DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO, DBPROPSET_DBINIT, m_pIDBInitialize))
		{
			rgPropertySets.guidPropertySet=DBPROPSET_DBINIT;
			rgPropertySets.cProperties=1;
 			rgPropertySets.rgProperties=&rgProperties;
			rgPropertySets.rgProperties->dwPropertyID = DBPROP_AUTH_PERSIST_SENSITIVE_AUTHINFO;
			rgPropertySets.rgProperties->dwOptions = DBPROPOPTIONS_OPTIONAL;
			rgPropertySets.rgProperties->vValue.vt = VT_BOOL;
			if(fPersist_Sensitive)
				V_BOOL(&rgPropertySets.rgProperties->vValue)=VARIANT_TRUE;
			else
				V_BOOL(&rgPropertySets.rgProperties->vValue)=VARIANT_FALSE;
			
			// Set the Property
			if(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown **)&pIDBProperties))
				CHECK(pIDBProperties->SetProperties(1,&rgPropertySets),S_OK);

			SAFE_RELEASE(pIDBProperties);
		}

	// Need to check and set DBPROP_AUTH_PERSIST_ENCRYPTED
	if(SupportedProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBPROPSET_DBINIT, m_pIDBInitialize))
		if (SettableProperty(DBPROP_AUTH_PERSIST_ENCRYPTED, DBPROPSET_DBINIT, m_pIDBInitialize))
		{
			rgPropertySets.guidPropertySet=DBPROPSET_DBINIT;
			rgPropertySets.cProperties=1;
 			rgPropertySets.rgProperties=&rgProperties;
			rgPropertySets.rgProperties->dwPropertyID = DBPROP_AUTH_PERSIST_ENCRYPTED;
			rgPropertySets.rgProperties->dwOptions = DBPROPOPTIONS_OPTIONAL;
			rgPropertySets.rgProperties->vValue.vt = VT_BOOL;
			if(fPersist_Sensitive)
				V_BOOL(&rgPropertySets.rgProperties->vValue)=VARIANT_TRUE;
			else
				V_BOOL(&rgPropertySets.rgProperties->vValue)=VARIANT_FALSE;
			
			// Set the Property
			if(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown **)&pIDBProperties))
				CHECK(pIDBProperties->SetProperties(1,&rgPropertySets),S_OK);

			SAFE_RELEASE(pIDBProperties);
		}

	// Need to check and set DBPROP_INIT_PROMPT
	if(SupportedProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, m_pIDBInitialize))
		if(SettableProperty(DBPROP_INIT_PROMPT, DBPROPSET_DBINIT, m_pIDBInitialize))
		{
			rgPropertySets.guidPropertySet=DBPROPSET_DBINIT;
			rgPropertySets.cProperties=1;
 			rgPropertySets.rgProperties=&rgProperties;
			rgPropertySets.rgProperties->dwPropertyID = DBPROP_INIT_PROMPT;
			rgPropertySets.rgProperties->dwOptions = DBPROPOPTIONS_OPTIONAL;
			rgPropertySets.rgProperties->vValue.vt = VT_I2;
			V_BOOL(&rgPropertySets.rgProperties->vValue)=DBPROMPT_NOPROMPT;
			
			// Set the Property
			if(VerifyInterface(m_pIDBInitialize, IID_IDBProperties, 
						DATASOURCE_INTERFACE, (IUnknown **)&pIDBProperties))
				CHECK(pIDBProperties->SetProperties(1,&rgPropertySets),S_OK);

			SAFE_RELEASE(pIDBProperties);
		}

	// Initialize the DSO and then Release the CDataSource ref
	if( SUCCEEDED(InitializeDSO()) )
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Causes Initialize to be called implicitly on the DSO.  This
//		  function may cause prompting, therefore caller should only
//		  invoke if the user has selected prompting when running the test.
//
// @rdesc HRESULT of call causing the implicit initialize
//
BOOL TCPersistFile::ImplicitInit()
{
	// We need completely new DSO to cause implicit initialize, 
	// so release the old DSO
	ReleaseDSO();
	
	// Create new DSO
	if(!CHECK(CreateDataSourceObject(),S_OK))
		return FALSE;

	if(!VerifyInterface(m_pIDBInitialize, IID_IPersistFile, 
				DATASOURCE_INTERFACE, (IUnknown **)&m_pDSOIPersistFile))
		return FALSE;

	m_fInitialized = FALSE;
	return TRUE;
}

//--------------------------------------------------------------------
// @mfunc Gets an IDBInitialize interface and does an uninitialize.
// Assumes that m_pDSOIPersistFile is valid.
//
// @rdesc TRUE if uninitialize was successful or not needed, else FALSE
BOOL TCPersistFile::Uninitialize()
{				
	// If no Initialize pointer get one
	if(!m_pIDBInitialize)
		if(!VerifyInterface(m_pDSOIPersistFile, IID_IDBInitialize, 
						DATASOURCE_INTERFACE, (IUnknown **)&m_pIDBInitialize))
			return FALSE;

	// Initialize the DSO and then Release the CDataSource ref
	if(CHECK(UninitializeDSO(), S_OK))
		return TRUE;
	else
		return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Creates an active child command on initialized DSO.  
//
// NOTE: Caller must call CleanUpAcitveCommand to take care
// of any necessary cleanup from this function.
//
// @rdesc Whether or not creating the command was successful
//
BOOL TCPersistFile::CreateActiveCommand()
{
	BOOL fResults = FALSE;	

	// Just return if command already exists
	if( m_pICommand )
		return TRUE;

	if( fResults=ExplicitInit() )
	{
		// Save initialized DSO here to ensure we are in a clean save state		
		if( FAILED(m_hr=QuickSave(DELETE_YES, m_wszFile)) )
			return FALSE;	

		// Get a Session pointer
		if( !m_pIDBCreateSession )
			if(!VerifyInterface(m_pDSOIPersistFile, IID_IDBCreateSession, 
						DATASOURCE_INTERFACE, (IUnknown **)&m_pIDBCreateSession))
				return FALSE;

		// Get a Command pointer
		m_hr=m_pIDBCreateSession->CreateSession(NULL, IID_IDBCreateCommand,
											(IUnknown **)&m_pIDBCreateCommand);
		if(FAILED(m_hr))
			return FALSE;
		
		CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,
											(IUnknown **)&m_pICommand),S_OK);	
	}
	
	SAFE_RELEASE(m_pIDBCreateCommand);
	return fResults;
}

//--------------------------------------------------------------------
// @mfunc Cleans up after call to CreateActiveCommand
//
void TCPersistFile::CleanUpActiveCommand()
{
	SAFE_RELEASE(m_pICommand);
	SAFE_RELEASE(m_pCmdIPersistFile);
	Uninitialize();
}

//--------------------------------------------------------------------
// @mfunc	Creates a command, saves it, then changes its command tree, 
//			and any types of goals that are supported. 
//
// NOTE:  Caller must call CleanUpChangeCommand to do necessary clean up
// from this function.
//
// @rdesc Success of creating command and changing goals
//
BOOL TCPersistFile::ChangeCommand()
{	  										
	BOOL fResults = FALSE;

	if(CreateActiveCommand())
	{
		// Save DSO here to ensure we are in a clean save state,
		// if DSO has persist interface
		if(m_pDSOIPersistFile)		
			if(FAILED(m_hr=QuickSave(DELETE_YES, m_wszFile)))
				return FALSE;
	
		// Change the command text, this should always succeed
		CHECK(m_pICommand->QueryInterface(IID_ICommandText, (void **)&m_pICommandText),S_OK);
		
		// Set text to Select * from %s, note we'll never execute, so it can be bogus table name
		// We know that at least this change occured, so can return TRUE
		if(CHECK(m_pICommandText->SetCommandText(DBGUID_DEFAULT, (LPWSTR)wszSELECT_ALLFROMTBL),S_OK))
			fResults = TRUE;	

		// Change the property goals
		SetRowsetProperty(m_pICommand, DBPROPSET_ROWSET, DBPROP_IRowsetChange);
		SAFE_RELEASE(m_pICommandText);
	}
	
	return fResults;
}

//--------------------------------------------------------------------
// @mfunc Cleans up anything done in ChangeCommand
//
void TCPersistFile::CleanUpChangeCommand()
{
	CleanUpActiveCommand();
}

//--------------------------------------------------------------------
// @mfunc Cleans up anything done in ActivateRowset
//
void TCPersistFile::CleanUpActivateRowset()
{
	// Drop table from database
	SAFE_RELEASE(m_pIOpenRowset);
	SAFE_RELEASE(m_pIAccessor);
	m_pTable->DropTable();
}

//--------------------------------------------------------------------
// @mfunc Cleans up DSO
//
void TCPersistFile::ReleaseDSO()
{
	//Release objects
	SAFE_RELEASE(m_pIDBCreateSession);
	SAFE_RELEASE(m_pDSOIPersistFile);
	ReleaseDataSourceObject();
}

//--------------------------------------------------------------------
// @mfunc Executes a query to generate an active rowset.  The CTable
//		  object is created by this method if one hasn't been created
//		  by a previous variation.  The CTable object is deleted in 
//		  the TCPersistFile destructor.
//
//	NOTE:  Caller must call CleanUpExecute to clean up the state
//  ActivateRowset introduces.
//
// @rdesc TRUE if everything went OK, else FALSE
//
BOOL TCPersistFile::ActivateRowset()
{
	// We should always be in a clean state before hand
	ASSERT(!m_pIAccessor);
	
	if(ExplicitInit())
	{
		// Only get the IOpenRowset if we haven't before
		if(!m_pIOpenRowset)
		{
			// Get the session object
			if(!m_pIDBCreateSession)
				if(!VerifyInterface(m_pDSOIPersistFile, IID_IDBCreateSession, 
								DATASOURCE_INTERFACE, (IUnknown **)&m_pIDBCreateSession))
					return FALSE;
			
			// Get a Command pointer
			if(!CHECK(m_hr=m_pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset,
												(IUnknown **)&m_pIOpenRowset), S_OK))
				return FALSE;
		}
							
		// Start with a table with PNUM_ROWS rows								 
		if(FAILED(m_hr=m_pTable->CreateTable(PNUM_ROWS)))
			return FALSE;
			
		// Get Select stmt and execute it, using the current command;
		// putting rowset's interface in m_pIAccessor
		// Now get the rowset which we'll use to populate the table
		return CHECK(m_hr=m_pTable->CreateRowset(USE_OPENROWSET, 
					IID_IAccessor, 0, NULL, (IUnknown **)&m_pIAccessor), S_OK);
	}

	return FALSE;
}


//--------------------------------------------------------------------
// @mfunc Performs a save to the file name in m_wszFile, and then deletes
//			the file.
//
// @rdesc HRESULT of Save
//
HRESULT	TCPersistFile::QuickSave(EDELETE_FILE eDelete, WCHAR * pwszFile, BOOL fRemember, BOOL fAddExt, BOOL fUnInit)
{	
	HRESULT hr=m_pDSOIPersistFile->Save(pwszFile,fRemember);

	// Don't need file for anything, so delete
	if( eDelete == DELETE_YES )
		QuickDelete(hr, pwszFile, fUnInit);

	return hr;
}

//--------------------------------------------------------------------
// @mfunc Releases the current DSO, creates a new one, and attempts
//		  a load of the file named in m_wszFile.
//
// @rdesc HRESULT of Load
//
HRESULT TCPersistFile::QuickLoad(LPWSTR pwszFile)
{		
	// Use default file if one isn't specified
	if(!pwszFile)
		pwszFile = m_wszFile;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 
	if(CHECK(CreateDataSourceObject(), S_OK))
		if(VerifyInterface(m_pIDBInitialize, IID_IPersistFile, 
							DATASOURCE_INTERFACE, (IUnknown **)&m_pDSOIPersistFile))
		{
			//Now try to load and return results
			m_hr = m_pDSOIPersistFile->Load(pwszFile,STGM_READWRITE);
			return m_hr;
		}

	return E_FAIL;
}


//--------------------------------------------------------------------
// @mfunc Delete the File that was created by IPersistFile::Save
//
// @rdesc 
//
void TCPersistFile::QuickDelete(HRESULT hr, WCHAR * pwszFile, BOOL fUnInit)
{		
	WCHAR wszFile[PATH_SIZE];

	//Uninitialize the DSO
	if( fUnInit )
		Uninitialize();

	// Create the FileName
	wcscpy(wszFile, pwszFile);

	if( SUCCEEDED(hr) )
	{
		LPWSTR wszCurFile = NULL;

		// Get the current File and compare it
		if( CHECK(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile), S_OK) )
		{
			COMPARE(wcscmp(wszCurFile, wszFile), 0);
			ConvertToMBCS(wszCurFile, m_szFile, PATH_SIZE);
			PROVIDER_FREE(wszCurFile);
		}

		COMPARE(remove(m_szFile), 0);
	}
	else
		COMPARE(remove(m_szFile), -1);
}


//--------------------------------------------------------------------
// @mfunc  Specific initialization needed for inheriting testcases.  If
//		   this function fails, the rest of the testcase is not executed.	
//
// @rdesc TRUE or FALSE
//
BOOL TCCmdPersistFile::Init()
{	
	
	if( CDataSourceObject::Init() ) 
	{				
		m_pTable=(CTable *)m_pThisTestModule->m_pVoid;	
		m_pTable->MakeTableName(NULL);

		// Get current working directory to be used for creating files
		if(!_getcwd(m_szPath,PATH_SIZE)) {
			odtLog << wszErrorFindingCurrentPath;
			return FALSE;
		}

		// Build whole file path and name
		ConvertToWCHAR(m_szPath, m_wszFile, PATH_SIZE);
		wcscat(m_wszFile, L"\\");
		if(m_pTable->GetTableName())
			wcscat(m_wszFile, m_pTable->GetTableName());
		else
			wcscat(m_wszFile, m_pTable->GetModuleName());
		wcscat(m_wszFile, L".tst");

		// Instantiate Data Source Object and get a command on it
		if(!CHECK(CreateDataSourceObject(), S_OK))
			goto CLEANUP;

		if(!ExplicitInit())
			goto CLEANUP;

		if(!CHECK(m_pIDBInitialize->QueryInterface(IID_IDBCreateSession, 
										(void **)&m_pIDBCreateSession),S_OK))
			goto CLEANUP;

		if(!CHECK(m_pIDBCreateSession->CreateSession(NULL, IID_IOpenRowset,
										(IUnknown **)&m_pIOpenRowset),S_OK))
			goto CLEANUP;

		// Check to see if Commands are supported
		if(VerifyInterface(m_pIOpenRowset, IID_IDBCreateCommand, 
						SESSION_INTERFACE, (IUnknown **)&m_pIDBCreateCommand))
			CHECK(m_pIDBCreateCommand->CreateCommand(NULL, IID_ICommand,
											(IUnknown **)&m_pICommand),S_OK);
		else
			odtLog << L"Commands are not supported by Provider." << ENDL;
					
		SAFE_RELEASE(m_pIOpenRowset);
		SAFE_RELEASE(m_pIDBCreateCommand);

		// This is optional an interface, 
		// so check up front if we need to run any testcases.
		if(m_pICommand)
		{
			if(VerifyInterface(m_pICommand, IID_IPersistFile, 
						COMMAND_INTERFACE, (IUnknown **)&m_pCmdIPersistFile))
			{	
				odtLog << L"IPersistFile should not supported in V1.5 by Provider." << ENDL;
				return TRUE;
			}
			else
				odtLog << L"IPersistFile is not supported by Provider." << ENDL;
		}
	}

CLEANUP:
	
	return FALSE;
}

//--------------------------------------------------------------------
// @mfunc Specific cleanup needed for inheriting testcases	
//
// @rdesc TRUE or FALSE
//
BOOL TCCmdPersistFile::Terminate()
{
	if(CDataSourceObject::Terminate()) 	
	{	
		// Release objects
		SAFE_RELEASE(m_pCmdIPersistFile);
		SAFE_RELEASE(m_pICommand);
		SAFE_RELEASE(m_pIDBCreateSession);
		ReleaseDataSourceObject();

		// Make sure our file is deleted, this may fail
		// if it was cleaned up in the test case
		memset(m_szFile,'\0',PATH_SIZE);
		ConvertToMBCS(m_wszFile, m_szFile, PATH_SIZE);
		remove(m_szFile);	   		

		return TRUE;
	}
	
	return FALSE;
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// Test Case Section
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -


// {{ TCW_TEST_CASE_MAP(TCPersist_GetClassID_DSO)
//--------------------------------------------------------------------
// @class IPersist::GetClassID
//
class TCPersist_GetClassID_DSO : public TCPersist {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersist_GetClassID_DSO,TCPersist);
	// }}

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Verify Correct CLSID - DSO
	int Variation_1();
	// @cmember Null pclsid Parameter - DSO
	int Variation_2();
	// }}
};

// {{ TCW_TESTCASE(TCPersist_GetClassID_DSO)
#define THE_CLASS TCPersist_GetClassID_DSO
BEG_TEST_CASE(TCPersist_GetClassID_DSO, TCPersist, L"IPersist::GetClassID")
	TEST_VARIATION(1,		L"Verify Correct CLSID - DSO")
	TEST_VARIATION(2,		L"Null pclsid Parameter - DSO")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_GetClassID_DSO)
//--------------------------------------------------------------------
// @class IPersistFile::GetClassID
//
class TCPersistFile_GetClassID_DSO : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_GetClassID_DSO,TCPersistFile);
	// }}

	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Verify Correct CLSID - DSO
	int Variation_1();
	// @cmember Null pclsid Parameter - DSO
	int Variation_2();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_GetClassID_DSO)
#define THE_CLASS TCPersistFile_GetClassID_DSO
BEG_TEST_CASE(TCPersistFile_GetClassID_DSO, TCPersistFile, L"IPersistFile::GetClassID")
	TEST_VARIATION(1,		L"Verify Correct CLSID - DSO")
	TEST_VARIATION(2,		L"Null pclsid Parameter - DSO")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_IsDirty_DSO)
//--------------------------------------------------------------------
// @class IPersistFile::IsDirty
//
class TCPersistFile_IsDirty_DSO : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:											 
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_IsDirty_DSO,TCPersistFile);
	// }}
	
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Before Initialize - DSO
	int Variation_1();
	// @cmember After Explicit Initialize - DSO
	int Variation_2();
	// @cmember After Implicit Initialize - DSO
	int Variation_3();
	// @cmember With Active Command - DSO
	int Variation_4();
	// @cmember After Changing Command - DSO
	int Variation_5();
	// @cmember After Loading valid DSO - DSO
	int Variation_6();
	// @cmember After IProvideMoniker::GetMoniker - DSO
	int Variation_7();
	// @cmember Save to second file with fRemember = TRUE - DSO
	int Variation_8();
	// @cmember Save to second file with fRemember = FALSE - DSO
	int Variation_9();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_IsDirty_DSO)
#define THE_CLASS TCPersistFile_IsDirty_DSO
BEG_TEST_CASE(TCPersistFile_IsDirty_DSO, TCPersistFile, L"IPersistFile::IsDirty")
	TEST_VARIATION(1,		L"Before Initialize - DSO")
	TEST_VARIATION(2,		L"After Explicit Initialize - DSO")
	TEST_VARIATION(3,		L"After Implicit Initialize - DSO")
	TEST_VARIATION(4,		L"With Active Command - DSO")
	TEST_VARIATION(5,		L"After Changing Command - DSO")
	TEST_VARIATION(6,		L"After Loading valid DSO - DSO")
	TEST_VARIATION(7,		L"After IProvideMoniker::GetMoniker - DSO")
	TEST_VARIATION(8,		L"Save to second file with fRemember = TRUE - DSO")
	TEST_VARIATION(9,		L"Save to second file with fRemember = FALSE - DSO")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_Save_DSO)
//--------------------------------------------------------------------
// @class IPersistFile::Save
//
class TCPersistFile_Save_DSO : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_Save_DSO,TCPersistFile);
	// }}
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Before Initialize
	int Variation_1();
	// @cmember After Explicit Initialize
	int Variation_2();
	// @cmember After Implicit  Initialize
	int Variation_3();
	// @cmember With Active Command
	int Variation_4();
	// @cmember After Changing Command
	int Variation_5();
	// @cmember With Active Rowset
	int Variation_6();
	// @cmember lpszFileName=NULL, fRemember=TRUE, no current file
	int Variation_7();
	// @cmember lpszFileName=NULL, fRemember=FALSE, no current file
	int Variation_8();
	// @cmember lpszFileName=NULL, fRemember=TRUE, current file
	int Variation_9();
	// @cmember lpszFileName=NULL, fRemember=FALSE, current file
	int Variation_10();
	// @cmember Valid lpszFileName, fRemember=TRUE, current file
	int Variation_11();
	// @cmember Valid lpszFileName, fRemember=FALSE, current file
	int Variation_12();
	// @cmember Valid lpszFileName, fRemember=TRUE, no current file
	int Variation_13();
	// @cmember Valid lpszFileName, fRemember=FALSE, no current file
	int Variation_14();
	// @cmember lpszFileName = Empty String
	int Variation_15();
	// @cmember lpszFileName with no extention
	int Variation_16();
	// @cmember lpszFileName of provider default
	int Variation_17();
	// @cmember lpszFileName with a ending period
	int Variation_18();
	// @cmember lpszFileName with leading spaces
	int Variation_19();
	// @cmember lpszFileName with trailing spaces
	int Variation_20();
	// @cmember lpszFileName with special characters
	int Variation_21();
	// @cmember lpszFileName with space in the name
	int Variation_22();
	// @cmember lpszFileName with special characters in the name
	int Variation_23();
	// @cmember lpszFileName with special characters in the name with default extension
	int Variation_24();
	// @cmember lpszFileName with special characters in the name with non default extension
	int Variation_25();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_Save_DSO)
#define THE_CLASS TCPersistFile_Save_DSO
BEG_TEST_CASE(TCPersistFile_Save_DSO, TCPersistFile, L"IPersistFile::Save")
	TEST_VARIATION(1,		L"Before Initialize")
	TEST_VARIATION(2,		L"After Explicit Initialize")
	TEST_VARIATION(3,		L"After Implicit  Initialize")
	TEST_VARIATION(4,		L"With Active Command")
	TEST_VARIATION(5,		L"After Changing Command")
	TEST_VARIATION(6,		L"With Active Rowset")
	TEST_VARIATION(7,		L"lpszFileName=NULL, fRemember=TRUE, no current file")
	TEST_VARIATION(8,		L"lpszFileName=NULL, fRemember=FALSE, no current file")
	TEST_VARIATION(9,		L"lpszFileName=NULL, fRemember=TRUE, current file")
	TEST_VARIATION(10,		L"lpszFileName=NULL, fRemember=FALSE, current file")
	TEST_VARIATION(11,		L"Valid lpszFileName, fRemember=TRUE, current file")
	TEST_VARIATION(12,		L"Valid lpszFileName, fRemember=FALSE, current file")
	TEST_VARIATION(13,		L"Valid lpszFileName, fRemember=TRUE, no current file")
	TEST_VARIATION(14,		L"Valid lpszFileName, fRemember=FALSE, no current file")
	TEST_VARIATION(15,		L"lpszFileName = Empty String")
	TEST_VARIATION(16,		L"lpszFileName with no extention")
	TEST_VARIATION(17,		L"lpszFileName of provider default")
	TEST_VARIATION(18,		L"lpszFileName with a ending period")
	TEST_VARIATION(19,		L"lpszFileName with leading spaces")
	TEST_VARIATION(20,		L"lpszFileName with trailing spaces")
	TEST_VARIATION(21,		L"lpszFileName with special characters")
	TEST_VARIATION(22,		L"lpszFileName with space in the name")
	TEST_VARIATION(23,		L"lpszFileName with special characters in the name")
	TEST_VARIATION(24,		L"lpszFileName with special characters in the name with default extension")
	TEST_VARIATION(25,		L"lpszFileName with special characters in the name with non default extension")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_Load_DSO)
//--------------------------------------------------------------------
// @class IPersistFile::Load
//
class TCPersistFile_Load_DSO : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_Load_DSO,TCPersistFile);
	// }}
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember After Explicit Initialize
	int Variation_1();
	// @cmember After Implicit Initialize
	int Variation_2();
	// @cmember With Unreleased DB Session
	int Variation_3();
	// @cmember With Active Command
	int Variation_4();
	// @cmember With Active Rowset
	int Variation_5();
	// @cmember Two Consecutive Loads
	int Variation_6();
	// @cmember File That Does Not Exist
	int Variation_7();
	// @cmember lpszFileName=Null
	int Variation_8();
	// @cmember Invalid Persisted File
	int Variation_9();
	// @cmember grfMode=0
	int Variation_10();
	// @cmember Multiple Users Loading File in Read Mode
	int Variation_11();
	// @cmember Load, Uninitialize, Save and Load
	int Variation_12();
	// @cmember Load, Save, Uninitialize and Load
	int Variation_13();
	// @cmember Load with the default extension
	int Variation_14();
	// @cmember Save with PERSIST_SENSITIVE set to FALSE
	int Variation_15();
	// @cmember lpszFileName with leading spaces
	int Variation_16();
	// @cmember lpszFileName with trailing spaces
	int Variation_17();
	// @cmember lpszFileName with special characters
	int Variation_18();
	// @cmember lpszFileName with space in the name
	int Variation_19();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_Load_DSO)
#define THE_CLASS TCPersistFile_Load_DSO
BEG_TEST_CASE(TCPersistFile_Load_DSO, TCPersistFile, L"IPersistFile::Load")
	TEST_VARIATION(1,		L"After Explicit Initialize")
	TEST_VARIATION(2,		L"After Implicit Initialize")
	TEST_VARIATION(3,		L"With Unreleased DB Session")
	TEST_VARIATION(4,		L"With Active Command")
	TEST_VARIATION(5,		L"With Active Rowset")
	TEST_VARIATION(6,		L"Two Consecutive Loads")
	TEST_VARIATION(7,		L"File That Does Not Exist")
	TEST_VARIATION(8,		L"lpszFileName=Null")
	TEST_VARIATION(9,		L"Invalid Persisted File")
	TEST_VARIATION(10,		L"grfMode=0")
	TEST_VARIATION(11,		L"Multiple Users Loading File in Read Mode")
	TEST_VARIATION(12,		L"Load, Uninitialize, Save and Load")
	TEST_VARIATION(13,		L"Load, Save, Uninitialize and Load")
	TEST_VARIATION(14,		L"Load with the default extension")
	TEST_VARIATION(15,		L"Save with PERSIST_SENSITIVE set to FALSE")
	TEST_VARIATION(16,		L"lpszFileName with leading spaces")
	TEST_VARIATION(17,		L"lpszFileName with trailing spaces")
	TEST_VARIATION(18,		L"lpszFileName with special characters")
	TEST_VARIATION(19,		L"lpszFileName with space in the name")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_SaveCompleted_DSO)
//--------------------------------------------------------------------
// @class IPersistFile::SaveCompleted
//
class TCPersistFile_SaveCompleted_DSO : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_SaveCompleted_DSO,TCPersistFile);
	// }}
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember lpszFileName=Null
	int Variation_1();
	// @cmember lpszFileName=Valid Persisted File
	int Variation_2();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_SaveCompleted_DSO)
#define THE_CLASS TCPersistFile_SaveCompleted_DSO
BEG_TEST_CASE(TCPersistFile_SaveCompleted_DSO, TCPersistFile, L"IPersistFile::SaveCompleted")
	TEST_VARIATION(1,		L"lpszFileName=Null")
	TEST_VARIATION(2,		L"lpszFileName=Valid Persisted File")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_GetClassID_Cmd)
//--------------------------------------------------------------------
// @class IPersistFile::GetClassID on Command
//
class TCPersistFile_GetClassID_Cmd : public TCCmdPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_GetClassID_Cmd,TCCmdPersistFile);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Verify Correct CLSID - Command
	int Variation_1();
	// @cmember Null pclsid Parameter - Command
	int Variation_2();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_GetClassID_Cmd)
#define THE_CLASS TCPersistFile_GetClassID_Cmd
BEG_TEST_CASE(TCPersistFile_GetClassID_Cmd, TCCmdPersistFile, L"IPersistFile::GetClassID on Command")
	TEST_VARIATION(1,		L"Verify Correct CLSID - Command")
	TEST_VARIATION(2,		L"Null pclsid Parameter - Command")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_GetCurFile_DSO)
//--------------------------------------------------------------------
// @class IPersistFile::GetCurFile on DSO Object
//
class TCPersistFile_GetCurFile_DSO : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_GetCurFile_DSO,TCPersistFile);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Unsaved DSO
	int Variation_1();
	// @cmember Uninitialize after Load and Save
	int Variation_2();
	// @cmember New Loaded File
	int Variation_3();
	// @cmember lplpszFileName = NULL
	int Variation_4();
	// }}
};

// {{ TCW_TESTCASE(TCPersistFile_GetCurFile_DSO)
#define THE_CLASS TCPersistFile_GetCurFile_DSO
BEG_TEST_CASE(TCPersistFile_GetCurFile_DSO, TCPersistFile, L"IPersistFile::GetCurFile on DSO Object")
	TEST_VARIATION(1,		L"Unsaved DSO")
	TEST_VARIATION(2,		L"Uninitialize after Load and Save")
	TEST_VARIATION(3,		L"New Loaded File")
	TEST_VARIATION(4,		L"lplpszFileName = NULL")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }} TCW_TEST_CASE_MAP_END


// {{ TCW_TEST_CASE_MAP(TCPersistFile_UseOEMCharset)
//--------------------------------------------------------------------
// @class Persisting a file using OEM Charsets active
//
class TCPersistFile_UseOEMCharset : public TCPersistFile {
private:
	// @cmember Static array of variations
	DECLARE_TEST_CASE_DATA();

	WCHAR	m_wchExtendedChar;
	
public:
	// {{ TCW_DECLARE_FUNCS
	// @cmember Execution Routine
	DECLARE_TEST_CASE_FUNCS(TCPersistFile_UseOEMCharset,TCPersistFile);
	// }}
 
	// @cmember Initialization Routine
	virtual BOOL Init();
	// @cmember Termination Routine
	virtual BOOL Terminate();
	
	// {{ TCW_TESTVARS()
	// @cmember Save file containing upper ansi characters
	int Variation_1();
	// @cmember Save File with extended chars and switch charset to ANSI
	int Variation_2();
	// }}
};
// {{ TCW_TESTCASE(TCPersistFile_UseOEMCharset)
#define THE_CLASS TCPersistFile_UseOEMCharset
BEG_TEST_CASE(TCPersistFile_UseOEMCharset, TCPersistFile, L"Persisting a file using OEM Charsets active")
	TEST_VARIATION(1,		L"Save file containing upper ansi characters")
	TEST_VARIATION(2,		L"Save File with extended chars and switch charset to ANSI")
END_TEST_CASE()
#undef THE_CLASS
// }}
// }}


// }} END_DECLARE_TEST_CASES()

// {{ TCW_TESTMODULE(ThisModule)
TEST_MODULE(9, ThisModule, gwszModuleDescrip)
	TEST_CASE(1, TCPersist_GetClassID_DSO)
	TEST_CASE(2, TCPersistFile_GetClassID_DSO)
	TEST_CASE(3, TCPersistFile_IsDirty_DSO)
	TEST_CASE(4, TCPersistFile_Save_DSO)
	TEST_CASE(5, TCPersistFile_Load_DSO)
	TEST_CASE(6, TCPersistFile_SaveCompleted_DSO)
	TEST_CASE(7, TCPersistFile_GetClassID_Cmd)
	TEST_CASE(8, TCPersistFile_GetCurFile_DSO)
	TEST_CASE(9, TCPersistFile_UseOEMCharset)
END_TEST_MODULE()
// }}


// {{ TCW_TC_PROTOTYPE(TCPersist_GetClassID_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersist_GetClassID_DSO - IPersist::GetClassID
//|	Created:		07/15/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersist_GetClassID_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersist::Init() )
	// }}
			return TRUE;	

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Verify Correct CLSID - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersist_GetClassID_DSO::Variation_1()
{	
	TBEGIN;

	CLSID clsid;

	// Verify that CLSID returned is identical to the Provider CLSID
	TESTC_(m_pIPersist->GetClassID(&clsid), S_OK);
	TESTC(clsid == m_ProviderClsid);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Null pclsid Parameter - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersist_GetClassID_DSO::Variation_2()
{
	TBEGIN;

	// Null pclsid should fail gracefully
	TEST2C_(m_pIPersist->GetClassID(NULL),E_FAIL,HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER));

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
BOOL TCPersist_GetClassID_DSO::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersist::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_GetClassID_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_GetClassID_DSO - IPersistFile::GetClassID
//|	Created:		07/15/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_GetClassID_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}
		
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Verify Correct CLSID - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetClassID_DSO::Variation_1()
{	
	TBEGIN;

	CLSID clsid;

	// Verify that CLSID returned is identical to the Provider CLSID
	TESTC_(m_pDSOIPersistFile->GetClassID(&clsid), S_OK);
	TESTC(clsid == m_ProviderClsid);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Null pclsid Parameter - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetClassID_DSO::Variation_2()
{
	TBEGIN;

	// Null pclsid should fail gracefully
	TEST2C_(m_pDSOIPersistFile->GetClassID(NULL),E_FAIL, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER));

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
BOOL TCPersistFile_GetClassID_DSO::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_IsDirty_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_IsDirty_DSO - IPersistFile::IsDirty
//|	Created:		07/19/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_IsDirty_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}
		
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Before Initialize - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_1()
{
	TBEGIN;

	// New Uninitialized DSO was created for us in Init function,
	// so just check that it is dirty
	TESTC_(m_pDSOIPersistFile->IsDirty(), S_OK);

	// Save should fail before DSO is Initialized
	TEST2C_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),E_UNEXPECTED,S_OK);

	// Since some providers allow you to SAVE before being Initialized
	if( FAILED(m_hr) ) {
		TESTC_(QuickLoad(), STG_E_FILENOTFOUND);
	}
	else {
		TESTC_(QuickLoad(), S_OK);
	}

CLEANUP:	

	// Make sure we Uninitialize the DSO
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc After Explicit Initialize - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_2()
{
	TBEGIN;

	// Initialization the DSO
	TESTC(ExplicitInit());

	// After initialization, the DSO should be dirty
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_OK);

	// Save and Load the DSO
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(),S_OK);
	
CLEANUP:	

	// Make sure we Uninitialize the DSO
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc After Implicit Initialize - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_3()
{
	TBEGIN;

	HRESULT hr = E_FAIL;

	// Delete persisted file				
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(m_wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	// After atempting to initialization
	TESTC(ImplicitInit());

	// Initialize and get the HReuslt
	hr = m_pIDBInitialize->Initialize();
	TESTC_(m_pIDBInitialize->Uninitialize(),S_OK);

	// After attempting to initialization, the DSO should be dirty
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_OK);

	// Save should fail before DSO is Initialized
	TEST2C_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK,E_FAIL);
	
	// Since some providers allow you to SAVE before being Initialized
	if( FAILED(m_hr) ) {
		TESTC_(QuickLoad(), STG_E_FILENOTFOUND);
	}
	else {
		TESTC_(QuickLoad(), S_OK);
	}

	// Initialize the DSO should fail since a file was loaded
	TESTC_(m_pIDBInitialize->Initialize(), hr);

CLEANUP:

	// Make sure we Uninitialize the DSO
	Uninitialize();
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc With Active Command - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_4()
{
	TBEGIN;

	// Creating a command on a saved DSO should NOT dirty the DSO
	if( !CreateActiveCommand() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}

	// DSO should NOT dirty the DSO
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_FALSE);

	// Save should Succeed
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Cleanup command before load is done
	CleanUpActiveCommand();

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
		
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	//Must cleanup as part of call to ActiveCommand
	CleanUpActiveCommand();
	Uninitialize();
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc After Changing Command - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_5()
{
	TBEGIN;

	// Creating a command on a saved DSO should NOT dirty the DSO
	if( !ChangeCommand() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}

	// DSO should NOT dirty the DSO
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_FALSE);

	// Save should Succeed
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Cleanup command before load is done
	SAFE_RELEASE(m_pICommand);

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
		
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
	
CLEANUP:
	
	//Must cleanup as part of call to ChangeCommand
	CleanUpChangeCommand();
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc After Loading valid DSO - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_6()
{
	TBEGIN;

	// Initialization the DSO
	TESTC(ExplicitInit());

	// Save this initialized DSO				
	TESTC_(QuickSave(DELETE_NO, m_wszFile, FALSE),S_OK);
		
	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Status should not be dirty
	TESTC_(m_pDSOIPersistFile->IsDirty(), S_FALSE);

	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	// Make sure we Uninitialize the DSO
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc After IProvideMoniker::GetMoniker - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_7()
{	
#if(0)	//TODO - IProvideMoniker is no longer part of the spec...
	IProvideMoniker * 	pIProvMon = NULL;
	IMoniker *			pIMoniker = NULL;
		
	// Verify that getting a moniker has no affect on 
	// the dirty status of the DSO

	// Dirty DSO by initializing it
	TESTC(ExplicitInit());

	// Get IProvideMoniker if supported
	QTESTC(VerifyInterface(m_pDSOIPersistFile, IID_IProvideMoniker, 
								DATASOURCE_INTERFACE,(IUnknown **)&pIProvMon);

	// Save DSO object, clearing its dirty flag
	TESTC_(pIProvMon->GetMoniker(&pIMoniker), S_OK);
	SAFE_RELEASE(pIProvMon);
	SAFE_RELEASE(pIMoniker);

	// Verify that DSO dirty flag is still set			
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_OK);
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	// Make sure we Uninitialize the DSO
	Uninitialize();

	TRETURN;
#else	//0
	return TEST_SKIPPED;
#endif	//0

}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc Save to second file with fRemember = TRUE - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_8()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];	

	// Save an initialized DSO with standard file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Dirty the DSO by reinitializing
	TESTC(ExplicitInit());

	// Build string to store path of file to save to
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\second.tst");

	// Now save and remember another file name, and check dirty status
	TESTC_(QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK);

	// Should have a clean object since a SAVE occured
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_FALSE);

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
					
CLEANUP:

	// Delete file				
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Save to second file with fRemember = FALSE - DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_IsDirty_DSO::Variation_9()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];	
	
	// Save an initialized DSO with standard file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Dirty the DSO by reinitializing
	TESTC(ExplicitInit());

	// Build string to store path of file to save to
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\second.tst");

	// Now save but don't remember another file name, and check dirty status
	TESTC_(QuickSave(DELETE_NO, m_wszFile, FALSE),S_OK);

	// Delete file		
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);
			
	TESTC_(m_pDSOIPersistFile->IsDirty(),S_OK);

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
	
CLEANUP:

	// Make sure we Uninitialize the DSO
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_IsDirty_DSO::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_Save_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_Save_DSO - IPersistFile::Save
//|	Created:		07/19/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_Save_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}
		
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Before Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_1()
{
	TBEGIN;
								 
	// Saving should fail.
	// Since some providers allow you to SAVE before being Initialized
	TEST2C_(QuickSave(DELETE_YES, m_wszFile, TRUE),E_UNEXPECTED,S_OK);

	// Saving should fail.
	// Since some providers allow you to SAVE before being Initialized
	TEST2C_(QuickSave(DELETE_YES, m_wszFile, FALSE),E_UNEXPECTED,S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc After Explicit Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_2()
{		
	TBEGIN;

	// After initialization, saving the DSO should succeed
	TESTC(ExplicitInit());

	TESTC_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK);
	TESTC_(QuickLoad(), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
	QuickDelete(m_hr, m_wszFile);

	// After initialization, saving the DSO should succeed
	TESTC(ExplicitInit());

	TESTC_(m_hr=QuickSave(DELETE_NO, m_wszFile, FALSE),S_OK);
	TESTC_(QuickLoad(), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc After Implicit  Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_3()
{
	TBEGIN;

	HRESULT hr = E_FAIL;

	// After initialization with prompting, the saving DSO should work
	TESTC(ImplicitInit());

	// Initialize and get the HReuslt
	hr = m_pIDBInitialize->Initialize();
	TESTC_(m_pIDBInitialize->Uninitialize(),S_OK);

	// Save should fail before DSO is Initialized
	// Since some providers allow you to SAVE before being Initialized
	TEST2C_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),E_FAIL,S_OK);
	TESTC_(QuickLoad(), (FAILED(m_hr) ? STG_E_FILENOTFOUND : S_OK));
	TESTC_(m_pIDBInitialize->Initialize(), hr);
	QuickDelete(m_hr, m_wszFile);

	// Save should fail before DSO is Initialized
	// Since some providers allow you to SAVE before being Initialized
	TEST2C_(m_hr=QuickSave(DELETE_NO, m_wszFile, FALSE),E_FAIL,S_OK);
	TESTC_(QuickLoad(), (FAILED(m_hr) ? STG_E_FILENOTFOUND : S_OK));
	TESTC_(m_pIDBInitialize->Initialize(), hr);

CLEANUP:				

	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc With Active Command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_4()
{
	TBEGIN;
			
	// Creating a command on a saved DSO should NOT dirty the DSO
	if( !CreateActiveCommand() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}

	TESTC_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK);

	// Cleanup command before load is done
	CleanUpActiveCommand();

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
								
CLEANUP:

	//Must cleanup as part of call to ActiveCommand
	CleanUpActiveCommand();
	
	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc After Changing Command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_5()
{		
	TBEGIN;
	
	// Saving DSO after changing command should work
	if( !ChangeCommand() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}

	TESTC_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK);

	// Cleanup command before load is done
	CleanUpChangeCommand();

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
								
CLEANUP:

	//Must cleanup as part of call to ChangeCommand
	CleanUpChangeCommand();
	
	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc With Active Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_6()
{	
	TBEGIN;

	// Saving with Active rowset should work
	if( !ActivateRowset() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}

	TESTC_(m_hr=QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK);

	// Cleanup command before load is done
	CleanUpActivateRowset();

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
								
CLEANUP:

	//Must cleanup as part of call to ActivateRowset
	CleanUpActivateRowset();
	
	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=NULL, fRemember=TRUE, no current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_7()
{
	TBEGIN;

	// Start with a new DSO so there is no current file name
	ReleaseDSO();

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);

	// Test parameters
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, NULL, TRUE),STG_E_INVALIDNAME);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=NULL, fRemember=FALSE, no current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_8()
{
	TBEGIN;

	// Start with a new DSO so there is no current file name
	ReleaseDSO();

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);

	// Test parameters
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, NULL, FALSE),STG_E_INVALIDNAME);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=NULL, fRemember=TRUE, current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_9()
{	
	TBEGIN;

	LPWSTR wszCurFile = NULL;
	
	// Make sure save occurs to the current file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, m_wszFile, TRUE), S_OK);
	TESTC_(QuickSave(DELETE_NO, NULL, TRUE), S_OK);

	// Verify current file name was set to new file name
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_OK);

	// Returns 0 when identical and free filename
	TESTC(wszCurFile != NULL);
	TESTC(!wcscmp(wszCurFile, m_wszFile));
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(QuickLoad(), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);
	
CLEANUP:

	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	//Free the memory
	PROVIDER_FREE(wszCurFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=NULL, fRemember=FALSE, current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_10()
{	
	TBEGIN;

	LPWSTR wszCurFile = NULL;
	
	// Make sure save occurs to the current file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile, TRUE), S_OK);
	TESTC_(QuickSave(DELETE_NO, NULL, FALSE), S_OK);

	// Verify current file name was set to new file name
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_OK);

	// Returns 0 when identical and free filename
	TESTC(wszCurFile != NULL);
	TESTC(!wcscmp(wszCurFile, m_wszFile));
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(QuickLoad(), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	//Free the memory
	PROVIDER_FREE(wszCurFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Valid lpszFileName, fRemember=TRUE, current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_11()
{
	TBEGIN;

	WCHAR	wszNewFile[PATH_SIZE];
	LPWSTR 	wszCurFile = NULL;
	
	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszNewFile, PATH_SIZE);
	wcscat(wszNewFile, L"\\new.tst");
		
	// Make sure save occurs to the new file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile),S_OK);
	TESTC_(QuickSave(DELETE_NO, wszNewFile, TRUE), S_OK);

	// Verify current file name was set to new file name
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_OK);

	// Returns 0 when identical and free filename
	TESTC(wszCurFile != NULL);
	TESTC(!wcscmp(wszCurFile, wszNewFile));

	// Initialize the DSO with the LOADED Properties
	TESTC_(QuickLoad(wszNewFile), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:
	
	// Cleanup file
	QuickDelete(m_hr, wszNewFile);

	//Free the memory
	PROVIDER_FREE(wszCurFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Valid lpszFileName, fRemember=FALSE, current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_12()
{
	TBEGIN;

	WCHAR  wszNewFile[PATH_SIZE];
	LPWSTR wszCurFile = NULL;
	
	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszNewFile, PATH_SIZE);
	wcscat(wszNewFile,L"\\new.tst");
		
	// Do a save to get a current file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile),S_OK);
	TESTC_(QuickSave(DELETE_NO, wszNewFile, FALSE), S_OK);

	// Verify current file name was set to new file name
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_OK);

	// Returns 0 when identical and free filename
	TESTC(wszCurFile != NULL);
	TESTC(wcscmp(wszCurFile, wszNewFile) != 0);

	// Initialize the DSO with the LOADED Properties
	TESTC_(QuickLoad(wszNewFile), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:
	
	// Cleanup file
	QuickDelete(m_hr, wszNewFile);

	//Free the memory
	PROVIDER_FREE(wszCurFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Valid lpszFileName, fRemember=TRUE, no current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_13()
{
	TBEGIN;

	LPWSTR wszFile = NULL;

	// We expect the current file name to be set to what we're saving as
	// Start with a new DSO so there is no current file name
	ReleaseDSO();

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);

	// Save to valid file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile, TRUE), S_OK);

	// Make sure current file name was set
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR*)&wszFile),S_OK);
	
	// Returns 0 if identical
	TESTC(wszFile != NULL);
	TESTC(!wcscmp(wszFile, m_wszFile));

	// Initialize the DSO with the LOADED Properties
	TESTC_(QuickLoad(), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	//Free the memory
	PROVIDER_FREE(wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Valid lpszFileName, fRemember=FALSE, no current file
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_14()
{
	TBEGIN;

	LPWSTR wszFile = NULL;	

	// We expect the current file name to NOT be set to what we're saving as
	// Start with a new DSO so there is no current file name
	ReleaseDSO();

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);

	// Save to valid file name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_NO, m_wszFile, FALSE), S_OK);
			
	// Make sure current file name was NOT set and
	// the save prompt is returned
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszFile),S_FALSE);
		
	// Returns 0 if identical, these should not be
	TESTC(wszFile != NULL);
	odtLog << wszDefaultSavePrompt << wszFile << wszNewLine;					

	// Initialize the DSO with the LOADED Properties
	TESTC_(QuickLoad(), S_OK);
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	// Delete the Persisted File
	QuickDelete(m_hr, m_wszFile);

	//Free the memory
	PROVIDER_FREE(wszFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName = Empty String
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_15()
{
	TBEGIN;

	// Empty string as file name should fail with Invalid Name
	TESTC(ExplicitInit());

	TESTC_(QuickSave(DELETE_YES, L"", TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, L"", FALSE), STG_E_INVALIDNAME);

CLEANUP:
		
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with no extention
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_16()
{
	TBEGIN;

	// Build whole file path and new file name
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\NoExtend");

	// Empty string as file name should fail with Invalid Name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, TRUE, TRUE), S_OK);

	// Empty string as file name should fail with Invalid Name
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, TRUE, TRUE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName of provider default
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_17()
{
	TBEGIN;

	// Build whole file path and new file name plus the default extension
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\Test");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, FALSE, TRUE), S_OK);

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, FALSE, TRUE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with a ending period
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_18()
{
	TBEGIN;

	// Build whole file path and new file name
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\PeriodEx.");

	// String ending with a period should get saved
	TESTC(ExplicitInit());	
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, FALSE, TRUE), S_OK);

	// String ending with a period should get saved
	TESTC(ExplicitInit());	
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, FALSE, TRUE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with leading spaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_19()
{
	TBEGIN;

	// Build whole file path and new file name
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\   LSpaces");

	// String beginning with spaces should get saved
	TESTC(ExplicitInit());		
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, TRUE, TRUE), S_OK);
	
	// String beginning with spaces should get saved
	TESTC(ExplicitInit());		
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, TRUE, TRUE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(20)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with trailing spaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_20()
{
	TBEGIN;

	// Build whole file path and new file name
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\TSpaces   ");

	// String ending with spaces should get saved
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, TRUE, TRUE), S_OK);

	// String ending with spaces should get saved
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, TRUE, TRUE), S_OK);
	
CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(21)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with special characters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_21()
{
	TBEGIN;

	// Build whole file path and new file name
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\test ; + , = [ ]");

	// String ending with a period should get saved
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, TRUE, TRUE), S_OK);

	// String ending with a period should get saved
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, TRUE, TRUE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(22)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with space in the name
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_22()
{
	TBEGIN;
	
	// Build whole file path and new file name
	WCHAR wszFile[PATH_SIZE];
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with Spaces");

	// String ending with a period should get saved
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE, TRUE, TRUE), S_OK);

	// String ending with a period should get saved
	TESTC(ExplicitInit());
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE, TRUE, TRUE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(23)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with special characters in the name
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_23()
{
	TBEGIN;

	WCHAR wszFile[PATH_SIZE];

	// String ending with a period should get saved
	TESTC(ExplicitInit());

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a \\");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a \\");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a :");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a :");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a *");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a *");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a ?");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a ?");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a \"");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a \"");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a <");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a <");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a >");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a >");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with a |");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile,L"File with a |");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(24)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with special characters in the name with default extension
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_24()
{
	TBEGIN;

	WCHAR wszFile[PATH_SIZE];

	// String ending with a period should get saved
	TESTC(ExplicitInit());

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a \\");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	// Build new file name
	wcscpy(wszFile, L"File with a \\");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a /");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	// Build new file name
	wcscpy(wszFile, L"File with a /");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	/*
	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a :");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_NO, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a :");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a *");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a *");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);
*/
	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a ?");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a ?");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a \"");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a \"");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a <");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a <");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a >");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a >");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a |");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a |");
	if( m_pwszDefaultExt )
		wcscat(wszFile,&m_pwszDefaultExt[1]);
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(25)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName with special characters in the name with non default extension
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Save_DSO::Variation_25()
{
	TBEGIN;

	WCHAR wszFile[PATH_SIZE];

	// String ending with a period should get saved
	TESTC(ExplicitInit());

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a \\");
	wcscat(wszFile, L".tst");
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	// Build new file name
	wcscpy(wszFile, L"File with a \\");
	wcscat(wszFile, L".tst");
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a /");
	wcscat(wszFile, L".tst");
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	// Build new file name
	wcscpy(wszFile, L"File with a /");
	wcscat(wszFile, L".tst");
	TEST2C_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);
	TEST2C_(QuickSave(DELETE_YES, wszFile, FALSE),STG_E_INVALIDNAME, STG_E_PATHNOTFOUND);

	/*
	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a :");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a :");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);
*/
	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a *");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a *");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a ?");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a ?");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a \"");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a \"");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a <");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a <");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a >");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a >");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile, L"\\File with a |");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

	// Build new file name
	wcscpy(wszFile, L"File with a |");
	wcscat(wszFile, L".tst");
	TESTC_(QuickSave(DELETE_YES, wszFile, TRUE), STG_E_INVALIDNAME);
	TESTC_(QuickSave(DELETE_YES, wszFile, FALSE), STG_E_INVALIDNAME);

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
BOOL TCPersistFile_Save_DSO::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_Load_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_Load_DSO - IPersistFile::Load
//|	Created:		07/20/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//			
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_Load_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}
		
		// Create a file we can always load
		TESTC(ExplicitInit());
		TESTC_(QuickSave(DELETE_NO, m_wszFile, TRUE),S_OK);
		
		return TRUE;
	}

CLEANUP:
	
	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc After Explicit Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_1()
{
	TBEGIN;

	// Reloading while initialized should succeed
	TESTC(ExplicitInit());

	// Now try loading while initialized
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE), DB_E_ALREADYINITIALIZED);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc After Implicit Initialize
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_2()
{
	TBEGIN;

	TESTC(ImplicitInit());

	// Now try loading while initialized
	if( !m_fInitialized ) {
		TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE), S_OK);
	}
	else {
		TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE), DB_E_ALREADYINITIALIZED);
	}

CLEANUP:

	// Uninitialize
	Uninitialize();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc With Unreleased DB Session
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_3()
{
	TBEGIN;
	
	IGetDataSource*	pIGetDataSource = NULL;

	// Load should fail with active DBSession
	TESTC(ExplicitInit());

	// Get a DB Session by asking for create command interface
	SAFE_RELEASE(m_pIDBCreateSession);
	TESTC(VerifyInterface(m_pDSOIPersistFile, IID_IDBCreateSession, 
					DATASOURCE_INTERFACE,(IUnknown **)&m_pIDBCreateSession));

	TESTC_(m_pIDBCreateSession->CreateSession(NULL, IID_IGetDataSource, 
											 (IUnknown **)&pIGetDataSource),S_OK);

	// Now load should fail
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READ), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup DB Session
	SAFE_RELEASE(pIGetDataSource);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc With Active Command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_4()
{
	TBEGIN;

	// Load should fail with active command object
	if( !CreateActiveCommand() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}

	TESTC_(m_pDSOIPersistFile->Load(m_wszFile, STGM_READ), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup command object
	CleanUpActiveCommand();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(5)
//*-----------------------------------------------------------------------
// @mfunc With Active Rowset
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_5()
{
	TBEGIN;
	
	// Load should fail with active rowset object
	if( !ActivateRowset() )
	{
		// Check to see if commands are not supported
		QTESTC_(m_hr, E_NOINTERFACE);
		odtLog << L"Commands are not supported by Provider." << ENDL;
		goto CLEANUP;
	}
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READ), DB_E_ALREADYINITIALIZED);

CLEANUP:

	// Cleanup command object
	CleanUpActivateRowset();

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(6)
//*-----------------------------------------------------------------------
// @mfunc Two Consecutive Loads
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_6()
{
	TBEGIN;

	// Initialize and Save
	TESTC(ExplicitInit());

	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Start over with clean DSO
	ReleaseDSO();

	// Load the file we saved on a previous DSO
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile, STGM_READWRITE), S_OK);

	// Load the file we saved on a previous DSO
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile, STGM_READWRITE), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(7)
//*-----------------------------------------------------------------------
// @mfunc File That Does Not Exist
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_7()
{
	TBEGIN;

	// Assume this filename does not exist
	WCHAR wszFile[PATH_SIZE] = L"xxzz";
	
	// Uninitialize if we need to
	Uninitialize();

	// Should get file not found attempting to load nonexistent file
	TESTC_(m_pDSOIPersistFile->Load(wszFile, STGM_READ), STG_E_FILENOTFOUND);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(8)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=Null
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_8()
{
	TBEGIN;

	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid arg attempting to load with null file argument
	TEST2C_(m_pDSOIPersistFile->Load(NULL,STGM_READ),STG_E_INVALIDNAME,HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER));

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(9)
//*-----------------------------------------------------------------------
// @mfunc Invalid Persisted File
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_9()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];
	int		iReturn;

	// Get the name of a file to create
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\bogus.tst");

	// Create a file without using IPersistFile->Save() and verify it succeeded
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	iReturn = _creat(m_szFile,_S_IWRITE);
	TESTC(iReturn != -1);
			
	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid param attempting to load a non persisted file
	TEST2C_(m_pDSOIPersistFile->Load(wszFile,STGM_READ), STG_E_FILENOTFOUND, STG_E_SHAREVIOLATION);
	
CLEANUP:

	// File needs to be Deleted
	_close(iReturn);
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(10)
//*-----------------------------------------------------------------------
// @mfunc grfMode=0
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_10()
{
	TBEGIN;

	// Uninitialize if we need to
	Uninitialize();

	// Verify that a mode of 0 is allowed
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,0), S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(11)
//*-----------------------------------------------------------------------
// @mfunc Multiple Users Loading File in Read Mode
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_11()
{
	TBEGIN;

	IPersistFile * 	pIPersistFile = NULL;

	// Uninitialize if we need to
	Uninitialize();

	// Load file with ReadWrite Mode to the DSO
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE),S_OK);

	// Now create a second DSO and attemp to load same file in Read Mode
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&pIPersistFile),S_OK);

	// Second load should also succeed
	TESTC_(pIPersistFile->Load(m_wszFile,STGM_READWRITE), S_OK);
			
CLEANUP:

	// Cleanup second DSO
	SAFE_RELEASE_(pIPersistFile);
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(12)
//*-----------------------------------------------------------------------
// @mfunc Load, Uninitialize, Save and Load
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_12()
{
	TBEGIN;

	// Get a valid persisted file to load again
	TESTC(ExplicitInit());

	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Start over with clean DSO
	ReleaseDSO();

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);

	// Load the file we saved on a previous DSO
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE),S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Save to make sure we really lost the init info after uninitializing
	TEST2C_(QuickSave(DELETE_NO, m_wszFile, TRUE),E_UNEXPECTED,S_OK);

	// Load should fail due to no connection info in persisted file
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(13)
//*-----------------------------------------------------------------------
// @mfunc Load, Save, Uninitialize and Load
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_13()
{
	TBEGIN;

	// Get a valid persisted file to load again
	TESTC(ExplicitInit());

	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Start over with clean DSO
	ReleaseDSO();

	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);

	// Load the file we saved on the previous DSO
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE),S_OK);

	//Get an IDBInitialize pointer
	SAFE_RELEASE(m_pIDBInitialize)
	TESTC_(m_pDSOIPersistFile->QueryInterface(IID_IDBInitialize, (void **)&m_pIDBInitialize),S_OK);

	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(),S_OK);

	// Now save the connection info generated strictly by the load we just did
	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Load should succeed from the file we saved
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE), S_OK);

CLEANUP:
	
	TRETURN;
}
// }}

// {{ TCW_VAR_PROTOTYPE(14)
//*-----------------------------------------------------------------------
// @mfunc Load with the default extension
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_14()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];
	LPWSTR 	wszCurFile = NULL;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 	
	// Verify current file name is default save prompt
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_FALSE);

	// Build whole file path and new file name
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\Test.");

	// Add the first three after the period
	if(wszCurFile && (wcslen(wszCurFile) > 3))
		wcscat(wszFile,&wszCurFile[abs(int(3-wcslen(wszCurFile)))]);
	else
		wcscat(wszFile,wszCurFile ? wszCurFile : L"");

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());

	TESTC_(m_pDSOIPersistFile->Save(wszFile,FALSE),S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid param attempting to load a non persisted file
	TESTC_(m_pDSOIPersistFile->Load(wszFile,STGM_READ), S_OK);
	
CLEANUP:

	// Free what the provider alloc'd
	PROVIDER_FREE(wszCurFile);

	// File needs to be Deleted
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(15)
//*-----------------------------------------------------------------------
// @mfunc Save with PERSIST_SENSITIVE set to FALSE
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_15()
{
	TBEGIN;
	IDBProperties	*pIDBProperties = NULL;
	DBPROP			rgProps[1];
	DBPROPSET		rgPropSet[1];
	ULONG			cPropSet = NUMELEM(rgPropSet);
	HRESULT			hr;

	// Initialize and Save
	TESTC(ExplicitInit(FALSE));

	TESTC_(QuickSave(DELETE_NO, m_wszFile), S_OK);

	// get rid of the password property (set it to default)
	memset(&rgProps[0], 0, sizeof(DBPROP));
	rgProps[0].dwPropertyID	= DBPROP_AUTH_PASSWORD;
	rgProps[0].dwOptions	= DBPROPOPTIONS_REQUIRED;
	VariantInit(&rgProps[0].vValue);

	rgPropSet[0].cProperties		= NUMELEM(rgProps);
	rgPropSet[0].guidPropertySet	= DBPROPSET_DBINIT;
	rgPropSet[0].rgProperties		= rgProps;
	TESTC_(m_pIDBInitialize->QueryInterface(IID_IDBProperties, (void **)&pIDBProperties),S_OK);
	
	TESTC(Uninitialize());
	TESTC_(pIDBProperties->SetProperties(cPropSet, rgPropSet), S_OK);

	hr = m_pIDBInitialize->Initialize();
	TEST2C_(hr, S_OK, DB_SEC_E_AUTH_FAILED);

	// Start over with clean DSO
	ReleaseDSO();

	// Load the file we saved on a previous DSO
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->Load(m_wszFile,STGM_READWRITE), S_OK);

	//Get an IDBInitialize pointer
	SAFE_RELEASE(m_pIDBInitialize)
	TESTC_(m_pDSOIPersistFile->QueryInterface(IID_IDBInitialize, (void **)&m_pIDBInitialize),S_OK);

	// Initialize the DSO with the LOADED Properties
	if( !FindSubString(GetModInfo()->GetInitString(), L"PASSWORD") && 
		!FindSubString(GetModInfo()->GetInitString(), L"PWD") ) {
		TESTC_(m_pIDBInitialize->Initialize(),S_OK);
	}
	else {
		TESTC_(m_pIDBInitialize->Initialize(), hr);
	}

CLEANUP:

	// Uninitialize if we need to
	Uninitialize();
	SAFE_RELEASE(pIDBProperties);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(16)
//*-----------------------------------------------------------------------
// @mfunc Load with leading spaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_16()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];
	LPWSTR 	wszCurFile = NULL;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 	
	// Verify current file name is default save prompt
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_FALSE);

	// Build whole file path and new file name
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\   Test");

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());

	TESTC_(m_pDSOIPersistFile->Save(wszFile,FALSE),S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid param attempting to load a non persisted file
	TESTC_(m_pDSOIPersistFile->Load(wszFile,STGM_READ), S_OK);
	
CLEANUP:

	// Free what the provider alloc'd
	PROVIDER_FREE(wszCurFile);

	// File needs to be Deleted
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(17)
//*-----------------------------------------------------------------------
// @mfunc Load with trailing spaces
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_17()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];
	LPWSTR 	wszCurFile = NULL;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 	
	// Verify current file name is default save prompt
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_FALSE);

	// Build whole file path and new file name
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\TSpaces   ");

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());
	
	TESTC_(m_pDSOIPersistFile->Save(wszFile,FALSE),S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid param attempting to load a non persisted file
	TESTC_(m_pDSOIPersistFile->Load(wszFile,STGM_READ), S_OK);
	
CLEANUP:

	// Free what the provider alloc'd
	PROVIDER_FREE(wszCurFile);

	// File needs to be Deleted
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(18)
//*-----------------------------------------------------------------------
// @mfunc Load with special characters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_18()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];
	LPWSTR 	wszCurFile = NULL;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 	
	// Verify current file name is default save prompt
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_FALSE);

	// Build whole file path and new file name
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\test ; + , = [ ]");

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());
	
	TESTC_(m_pDSOIPersistFile->Save(wszFile,FALSE),S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid param attempting to load a non persisted file
	TESTC_(m_pDSOIPersistFile->Load(wszFile,STGM_READ), S_OK);
	
CLEANUP:

	// Free what the provider alloc'd
	PROVIDER_FREE(wszCurFile);

	// File needs to be Deleted
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(19)
//*-----------------------------------------------------------------------
// @mfunc Load with space in the name
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_Load_DSO::Variation_19()
{
	TBEGIN;

	WCHAR	wszFile[PATH_SIZE];
	LPWSTR 	wszCurFile = NULL;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 	
	// Verify current file name is default save prompt
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_FALSE);

	// Build whole file path and new file name
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToWCHAR(m_szPath, wszFile, PATH_SIZE);
	wcscat(wszFile,L"\\File with Spaces");

	// Valid file name with the providers default extension
	TESTC(ExplicitInit());

	TESTC_(m_pDSOIPersistFile->Save(wszFile,FALSE),S_OK);

	// Uninitialize if we need to
	Uninitialize();

	// Should get invalid param attempting to load a non persisted file
	TESTC_(m_pDSOIPersistFile->Load(wszFile,STGM_READ), S_OK);
	
CLEANUP:

	// Free what the provider alloc'd
	PROVIDER_FREE(wszCurFile);

	// File needs to be Deleted
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_Load_DSO::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_SaveCompleted_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_SaveCompleted_DSO - IPersistFile::SaveCompleted
//|	Created:		07/22/95
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_SaveCompleted_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}
		
		// Create a file we can always load
		TESTC(ExplicitInit());
		
		return TRUE;
	}

CLEANUP:

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=Null
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_SaveCompleted_DSO::Variation_1()
{
	TBEGIN;

	// Save the state we are in 
	TESTC_(m_pDSOIPersistFile->Save(m_wszFile,TRUE), S_OK);

	// Verify null file name succeeds (S_OK should always be returned
	// so here are are just making sure the NULL doesn't crash the provider
	TESTC_(m_pDSOIPersistFile->SaveCompleted(NULL),S_OK);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc lpszFileName=Valid Persisted File
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_SaveCompleted_DSO::Variation_2()
{
	TBEGIN;

	// Save the state we are in 
	TESTC_(m_pDSOIPersistFile->Save(m_wszFile,TRUE), S_OK);

	// Verify the correct file name works  
	TESTC_(m_pDSOIPersistFile->SaveCompleted(m_wszFile),S_OK);

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
BOOL TCPersistFile_SaveCompleted_DSO::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_GetClassID_Cmd)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_GetClassID_Cmd - IPersistFile::GetClassID on Command
//|	Created:		01/18/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_GetClassID_Cmd::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCCmdPersistFile::Init() )
	// }}
		return TRUE;

	// Check to see if supported
	if( !m_pCmdIPersistFile || !m_pICommand )
		return TEST_SKIPPED;
	else
		return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Verify Correct CLSID - Command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetClassID_Cmd::Variation_1()
{	
	TBEGIN;

	CLSID clsid = GUID_NULL;

	// Verify that CLSID returned is identical to the Provider CLSID			
	TESTC_(m_pCmdIPersistFile->GetClassID(&clsid), S_OK);	

	TESTC(clsid == m_ProviderClsid);

CLEANUP:

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Null pclsid Parameter - Command
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetClassID_Cmd::Variation_2()
{	
	TBEGIN;

	// Null pclsid should fail gracefully
	TEST2C_(m_pCmdIPersistFile->GetClassID(NULL),E_FAIL, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER));

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
BOOL TCPersistFile_GetClassID_Cmd::Terminate()
{
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCCmdPersistFile::Terminate());

}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_GetCurFile_DSO)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_GetCurFile_DSO - IPersistFile::GetCurFile on DSO Object
//|	Created:		02/01/96
//|	Updated:		04/25/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_GetCurFile_DSO::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}
		
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Unsaved DSO
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetCurFile_DSO::Variation_1()
{
	TBEGIN;
	
	LPWSTR wszCurFile = NULL;

	// Release current DSO
	ReleaseDSO();

	// Get new DSO 	
	TESTC_(GetModInfo()->CreateProvider(NULL, IID_IPersistFile,
									   (IUnknown **)&m_pDSOIPersistFile),S_OK);
	
	// Verify current file name is default save prompt
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_FALSE);
		
	// Just print default save prompt, since its provider specific
	odtLog << wszDefaultSavePrompt << wszCurFile << wszNewLine;			

CLEANUP:
	
	// Free what the provider alloc'd
	PROVIDER_FREE(wszCurFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//*-----------------------------------------------------------------------
// @mfunc Uninitialize after Load and Save
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetCurFile_DSO::Variation_2()
{
	TBEGIN;

	LPWSTR 	wszCurFile = NULL;
			
	TESTC(ExplicitInit());
		
	// Do a save to get a current file name
	TESTC_(QuickSave(DELETE_YES, m_wszFile),S_OK);

	// Now do the uninitialize
	TESTC(Uninitialize());

	// Should definitely be dirty at this point
	TESTC_(m_pDSOIPersistFile->IsDirty(), S_OK);

	// Verify current file name is still old value, since we haven't resaved
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_OK);
				
	// Returns 0 when identical
	TESTC(!wcscmp(wszCurFile ? wszCurFile : L"",m_wszFile));

CLEANUP:

	PROVIDER_FREE(wszCurFile);
	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(3)
//*-----------------------------------------------------------------------
// @mfunc New Loaded File
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetCurFile_DSO::Variation_3()
{			
	WCHAR	wszNewFile[PATH_SIZE];
	LPWSTR	wszCurFile = NULL;

	// Build whole file path and new file name
	ConvertToWCHAR(m_szPath, wszNewFile, PATH_SIZE);
	wcscat(wszNewFile,L"\\new.tst");
	
	// First get initialized
	TESTC(ExplicitInit());

	// Save our current state with a known file name
	TESTC_(m_pDSOIPersistFile->Save(wszNewFile,TRUE),S_OK);

	// Must uninitialize before loading again
	TESTC(Uninitialize());
					
	// Load the persisted state
	TESTC_(m_pDSOIPersistFile->Load(wszNewFile, STGM_READWRITE), S_OK);

	// Now GetCurFile, it should be wszNewFile
	TESTC_(m_pDSOIPersistFile->GetCurFile((LPWSTR *)&wszCurFile),S_OK);
						
	// Returns 0 when identical
	TESTC(!wcscmp(wszCurFile ? wszCurFile : L"", wszNewFile));
	m_fInitialized = TRUE;
					
CLEANUP:

	PROVIDER_FREE(wszCurFile);

	// Delete file
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(wszNewFile, m_szFile, PATH_SIZE);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(4)
//*-----------------------------------------------------------------------
// @mfunc lplpszFileName = NULL
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_GetCurFile_DSO::Variation_4()
{
	TBEGIN;

	TEST2C_(m_pDSOIPersistFile->GetCurFile(NULL),E_FAIL, HRESULT_FROM_WIN32(RPC_X_NULL_REF_POINTER));

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
BOOL TCPersistFile_GetCurFile_DSO::Terminate()
{	
	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());
}	// }}
// }}
// }}


// {{ TCW_TC_PROTOTYPE(TCPersistFile_UseOEMCharset)
//*-----------------------------------------------------------------------
//|	Test Case:		TCPersistFile_UseOEMCharset - Persisting a file using OEM Charsets active
//|	Created:			11/03/98
//*-----------------------------------------------------------------------

//--------------------------------------------------------------------
// @mfunc TestCase Initialization Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_UseOEMCharset::Init()
{
	// {{ TCW_INIT_BASECLASS_CHECK
	if( TCPersistFile::Init() )
	// }}
	{
		// If not supported 
		if( !m_pDSOIPersistFile )
		{
			odtLog << L"IPersistFile is not supported by Provider." << ENDL;
			return TEST_SKIPPED;
		}

		m_wchExtendedChar = 0x00A3; // Currency symbol, English Pound
		// make sure that the file name 
		// can actually be converted to ANSI.
		if( !iswcharMappable(m_wchExtendedChar) )
		{
			odtLog << L"Skipping because the character" << m_wchExtendedChar << L"is not supported on this machine." << ENDL;
			return TEST_SKIPPED;
		}

		// Indicate that FILE functions should use 
		// the OEM character set
		SetFileApisToOEM();
		
		return TRUE;
	}

	return FALSE;
}


// {{ TCW_VAR_PROTOTYPE(1)
//*-----------------------------------------------------------------------
// @mfunc Save file containing upper ansi characters
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_UseOEMCharset::Variation_1()
{
	TBEGIN;

	m_wszFile[0] = m_wchExtendedChar;
	m_wszFile[1] = L'\0';

	// After initialization, saving the DSO should succeed
	TESTC(ExplicitInit());

	TESTC_(m_pDSOIPersistFile->Save(m_wszFile,TRUE),S_OK);
			
	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	Uninitialize();

	// Delete the Persisted File
	// Important to use OEM code page
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(m_wszFile, m_szFile, PATH_SIZE, CP_OEMCP);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_VAR_PROTOTYPE(2)
//--------------------------------------------------------------------
// @mfunc Save File with extended chars and switch charset to ANSI
//
// @rdesc TEST_PASS or TEST_FAIL
//
int TCPersistFile_UseOEMCharset::Variation_2()
{
	TBEGIN;

	m_wszFile[0] = m_wchExtendedChar;
	m_wszFile[1] = m_wchExtendedChar;
	m_wszFile[2] = L'\0';

	// After initialization, saving the DSO should succeed
	TESTC(ExplicitInit());

	TESTC_(m_pDSOIPersistFile->Save(m_wszFile,TRUE),S_OK);

	// Reset File characters set to ANSI code page
	SetFileApisToANSI();

	// Load the DSO
	TESTC_(QuickLoad(), S_OK);
	
	// Initialize the DSO with the LOADED Properties
	TESTC_(m_pIDBInitialize->Initialize(), S_OK);

CLEANUP:

	Uninitialize();

	// Reset for the other test variations
	SetFileApisToOEM();

	// Delete the Persisted File
	// Important to use OEM code page
	memset(m_szFile,'\0',PATH_SIZE);
	ConvertToMBCS(m_wszFile, m_szFile, PATH_SIZE, CP_OEMCP);
	remove(m_szFile);

	TRETURN;
}
// }}


// {{ TCW_TERMINATE_METHOD
//--------------------------------------------------------------------
// @mfunc TestCase Termination Routine
//
// @rdesc TRUE or FALSE
//
BOOL TCPersistFile_UseOEMCharset::Terminate()
{
	// Important to set char set back to ANSI
	SetFileApisToANSI();

	// {{ TCW_TERM_BASECLASS_CHECK2
	return(TCPersistFile::Terminate());
}	// }}
// }}
// }}
