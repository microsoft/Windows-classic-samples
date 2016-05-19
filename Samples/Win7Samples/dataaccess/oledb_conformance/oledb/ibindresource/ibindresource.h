//------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module IBINDRESOURCE.H | Header file for IBindResource test module.
//
// @rev 01 | 09-22-98 | Microsoft | Created
//

#ifndef _IBINDRESOURCE_H_
#define _IBINDRESOURCE_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library

///////////////////////////////////////////////////////////////
//Defines
//
///////////////////////////////////////////////////////////////
//Number of New URLs for testing CreateRow (1 to 99)
#define		NEWURLS		25

//Print the URL (from m_rgURLs) on which the test failed.
#define		OUTPUT_FAILEDURL(ulIndex)											\
				if(TESTB==TEST_FAIL)										\
					odtLog<<L"FAILED on the URL - "<<m_rgURLs[ulIndex]<<"\n";

//Print the URL (from m_rgRowsetURLs) on which the test failed.
#define		OUTPUT_FAILEDROWURL(ulIndex)												\
				if(TESTB==TEST_FAIL)										\
					odtLog<<L"FAILED on the URL - "<<m_rgRowsetURLs[ulIndex]<<"\n";

//This is used when calling CreateRow with an existing URL. Skip 
//the remaining variation if DB_E_RESOURCEEXISTS is returned. 
#define		RESOURCE_EXISTS(hr)		if(hr==DB_E_RESOURCEEXISTS) goto CLEANUP;

//Increment the new URL counter. Try to use a new url for every
//successful call to CreateRow.
#define		NEW_URL		if(g_cNewURL < NEWURLS-1) g_cNewURL++;

//Check if the IBindResource is supported on ROW object. If not
//Then some variations of CreateRow may need to be skipped.
#define		SKIPIF_NOBINDONROW		if((m_eTestCase==TC_ROW) && !m_pIBindResource) \
										TESTC_PROVIDER(m_pIBindResource);



///////////////////////////////////////////////////////////////
//Enumerations
//
///////////////////////////////////////////////////////////////
//The 5 types of objects returned by Bind. Commands are treated
//separately.
enum EOBJECTTYPE
{
	DSO				= 0,
	SESSION			= 1,
	ROWSET			= 2,
	ROW				= 3,
	STREAM			= 4,
	INVALID_OBJECT	= 5  //This should always be the last one.
};

//This is used to identify the object on which the IBindResource
//or ICreateRow interface currently being tested is implemented
//on. For e.g. TC_RBINDER identifies that the test is running
//on the interface on the Root Binder.
enum ETESTCASE
{
	TC_RBINDER = 1,		//Root Binder
	TC_PBINDER,			//Provider Binder
	TC_SESSION,			//Session object
	TC_ROW				//Row object	
};


///////////////////////////////////////////////////////////////
//CAuthenticate Class  -   Wrapper object for IAuthenticate 
//							interface.
//
///////////////////////////////////////////////////////////////
class CAuthenticate : public IAuthenticate
{
protected:
	DWORD m_cRef;

public:
	CAuthenticate()	{ m_cRef = 1; };
	virtual ~CAuthenticate();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    virtual HRESULT STDMETHODCALLTYPE Authenticate(HWND *phwnd, LPWSTR *pszUsername, LPWSTR *pszPassword);
};



#endif 	//_IBINDRESOURCE_H_
