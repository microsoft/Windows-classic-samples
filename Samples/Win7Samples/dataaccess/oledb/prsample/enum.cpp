//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module ENUM.CPP
//
//---------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "prsample.h"		// Programmer's Reference Sample includes



/////////////////////////////////////////////////////////////////
// myCreateEnumerator
//
//	This function creates an enumerator, obtains a sources rowset
//	from it, displays the rowset to the user, and allows the user
//	to specify the ProgID of a provider. The CLSID that matches
//	this ProgID is retuned to the caller in *pCLSID.
//
/////////////////////////////////////////////////////////////////
HRESULT myCreateEnumerator
	(
	REFCLSID				clsidEnumerator,
	CLSID *					pCLSID
	)
{
	HRESULT					hr;
	IUnknown *				pIUnkEnumerator				= NULL;
	ISourcesRowset *		pISourcesRowset				= NULL;
	IRowset *				pIRowset					= NULL;
	IDBInitialize *			pIDBInitialize				= NULL;
	WCHAR					wszProgID[MAX_NAME_LEN + 1] = {0};
	
	const ULONG				cProperties					= 2;
	DBPROP					rgProperties[cProperties];
	DBPROPSET				rgPropSets[1];

	// Create the Enumerator object. We ask for IUnknown when creating
	// the enumerator because some enumerators may require initialization
	// before we can obtain a sources rowset from the enumerator. This is
	// indicated by whether the enumerator object exposes IDBInitialize
	// or not (we don't want to ask for IDBInitialize, since enumerators
	// that don't require initialization will cause the CoCreateInstance
	// to fail)
	XCHECK_HR(hr = CoCreateInstance(
				clsidEnumerator,			//clsid -- enumerator
				NULL,						//pUnkOuter
				CLSCTX_INPROC_SERVER,		//dwClsContext
				IID_IUnknown,				//riid
				(void**)&pIUnkEnumerator	//ppvObj
				));

	// If the enumerator exposes IDBInitialize, we need to initialize it
	if( SUCCEEDED(hr = pIUnkEnumerator->QueryInterface(IID_IDBInitialize, 
				(void**)&pIDBInitialize)) )
	{
		CHECK_HR(hr = myDoInitialization(pIUnkEnumerator));
	}

	// Set properties on the rowset, to request additional functionality
	myAddRowsetProperties(rgPropSets, cProperties, rgProperties);

	// Obtain a sources rowset from the enumerator. This rowset contains
	// all of the OLE DB providers that this enumerator is able to list
	XCHECK_HR(hr = pIUnkEnumerator->QueryInterface(IID_ISourcesRowset,
				(void**)&pISourcesRowset));
	XCHECK_HR(hr = pISourcesRowset->GetSourcesRowset(
				NULL,					//pUnkOuter
				IID_IRowset,			//riid
				1,						//cPropSets
				rgPropSets,				//rgPropSets
				(IUnknown**)&pIRowset	//ppRowset
				));

	// Display the rowset to the user; this will allow the user to
	// perform basic navigation of the rowset and will allow the user
	// to select a row containing a desired provider.
	CHECK_HR(hr = myDisplayRowset(pIRowset, 
				L"SOURCES_NAME", MAX_NAME_LEN, wszProgID));

	// Obtain the ProgID for the provider to use from the user;
	// the default value for this is the value of the SOURCES_NAME
	// column in the row selected by the user previously
	myGetInputFromUser(wszProgID, _countof(wszProgID),L"\nType the ProgID of a provider"
				L" to use [Enter = `%s`]: ", wszProgID);
	XCHECK_HR(hr = CLSIDFromProgID(wszProgID, pCLSID));

CLEANUP:
	if( pIUnkEnumerator )
		pIUnkEnumerator->Release();
	if( pISourcesRowset )
		pISourcesRowset->Release();
	if( pIRowset )
		pIRowset->Release();
	if( pIDBInitialize )
		pIDBInitialize->Release();
	return hr;
}
