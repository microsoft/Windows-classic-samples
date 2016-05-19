//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module DATASOURCE.CPP
//
//---------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "prsample.h"		// Programmer's Reference Sample includes



/////////////////////////////////////////////////////////////////
// myCreateDataSource
//
//	This function creates an OLE DB DataSource object for a
//	provider selected by the user, sets initialization properties
//	for the DataSource, and initializes the DataSource. The
//	function returns a pointer to the DataSource object's
//	IUnknown in *ppUnkDataSource.
//
/////////////////////////////////////////////////////////////////
HRESULT myCreateDataSource
	(
	IUnknown **				ppUnkDataSource
	)
{
	HRESULT					hr;
	IDataInitialize *		pIDataInitialize			= NULL;
	IDBPromptInitialize *	pIDBPromptInitialize		= NULL;
	IDBInitialize *			pIDBInitialize				= NULL;
	CLSID					clsid						= CLSID_MSDASQL;

	// Use the Microsoft Data Links UI to create the DataSource
	// object; this will allow the user to select the provider
	// to connect to and to set the initialization properties
	// for the DataSource object, which will be created by the
	// Data Links UI.
	if( g_dwFlags & USE_PROMPTDATASOURCE )
	{
		// Create the Data Links UI object and obtain the
		// IDBPromptInitialize interface from it
		XCHECK_HR(hr = CoCreateInstance(
					CLSID_DataLinks,				//clsid -- Data Links UI
					NULL,							//pUnkOuter
					CLSCTX_INPROC_SERVER,			//dwClsContext
					IID_IDBPromptInitialize,		//riid
					(void**)&pIDBPromptInitialize	//ppvObj
					));

		// Invoke the Data Links UI to allow the user to select
		// the provider and set initialization properties for
		// the DataSource object that this will create
		XCHECK_HR(hr = pIDBPromptInitialize->PromptDataSource(
					NULL,							//pUnkOuter
					GetDesktopWindow(),				//hWndParent
					DBPROMPTOPTIONS_PROPERTYSHEET,	//dwPromptOptions
					0,								//cSourceTypeFilter
					NULL,							//rgSourceTypeFilter
					NULL,							//pwszszzProviderFilter
					IID_IDBInitialize, 				//riid
					(IUnknown**)&pIDBInitialize		//ppDataSource
					));

		// We've obtained a DataSource object from the Data Links UI. This
		// object has had its initialization properties set, so all we
		// need to do is Initialize it
		XCHECK_HR(hr = pIDBInitialize->Initialize());
	}
	// We are not using the Data Links UI to create the DataSource object.
	// Instead, we will enumerate the providers installed on this system
	// through the OLE DB Enumerator and will allow the user to select
	// the ProgID of the provider for which we will create a DataSource
	// object.
	else
	{
		// Use the OLE DB Enumerator to obtain a rowset of installed providers,
		// then allow the user to select a provider from this rowset
		CHECK_HR(hr = myCreateEnumerator(CLSID_OLEDB_ENUMERATOR, &clsid));

		// We will create the DataSource object through the OLE DB service
		// component IDataInitialize interface, so we need to create an
		// instance of the data initialization object
		XCHECK_HR(hr = CoCreateInstance(
					CLSID_MSDAINITIALIZE,			//clsid -- data initialize
					NULL,							//pUnkOuter
					CLSCTX_INPROC_SERVER,			//dwClsContext
					IID_IDataInitialize,			//riid
					(void**)&pIDataInitialize		//ppvObj
					));

		// Use IDataInitialize::CreateDBInstance to create an uninitialized
		// DataSource object for the chosen provider. By using this service
		// component method, the service component manager can provide
		// additional functionality beyond what is natively supported by the
		// provider if the consumer requests that functionality
		XCHECK_HR(hr = pIDataInitialize->CreateDBInstance(
					clsid,							//clsid -- provider
					NULL,							//pUnkOuter
					CLSCTX_INPROC_SERVER,			//dwClsContext
					NULL,							//pwszReserved
					IID_IDBInitialize,				//riid
					(IUnknown**)&pIDBInitialize		//ppDataSource
					));

		// Initialize the DataSource object by setting any required
		// initialization properties and calling IDBInitialize::Initialize
		CHECK_HR(hr = myDoInitialization(pIDBInitialize));
	}

CLEANUP:
	*ppUnkDataSource = pIDBInitialize;
	if( pIDataInitialize )
		pIDataInitialize->Release();
	if( pIDBPromptInitialize )
		pIDBPromptInitialize->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////
// myDoInitialization
//
//	This function sets initialization properties that tell the
//	provider to prompt the user for any information required to
//	initialize the provider, then calls the provider's 
//	initialization function.
//
/////////////////////////////////////////////////////////////////
HRESULT myDoInitialization
	(
	IUnknown *				pIUnknown
	)
{
	HRESULT					hr;
	IDBInitialize *			pIDBInitialize				= NULL;
	IDBProperties *			pIDBProperties				= NULL;
	HWND					hWnd						= GetDesktopWindow();
	
	const ULONG				cProperties					= 2;
	DBPROP					rgProperties[cProperties];
	DBPROPSET				rgPropSets[1];

	// In order to initialize the DataSource object most providers require
	// some initialization properties to be set by the consumer. For instance,
	// these might include the data source to connect to and the user ID and
	// password to use to establish identity. We will ask the provider to
	// prompt the user for this required information by setting the following
	// properties:
	myAddProperty(&rgProperties[0],DBPROP_INIT_PROMPT,VT_I2,DBPROMPT_COMPLETE);
#ifdef _WIN64
	myAddProperty(&rgProperties[1],DBPROP_INIT_HWND,  VT_I8, (LONG_PTR)hWnd);
#else
	myAddProperty(&rgProperties[1],DBPROP_INIT_HWND,  VT_I4, (LONG_PTR)hWnd);
#endif

	rgPropSets[0].rgProperties		= rgProperties;
	rgPropSets[0].cProperties		= cProperties;
	rgPropSets[0].guidPropertySet	= DBPROPSET_DBINIT;

	// Obtain the needed interfaces
	XCHECK_HR(hr = pIUnknown->QueryInterface(IID_IDBProperties, 
				(void**)&pIDBProperties));
	XCHECK_HR(hr = pIUnknown->QueryInterface(IID_IDBInitialize, 
				(void**)&pIDBInitialize));

	// If a provider requires initialization properties, it must support the
	// properties that we are setting (_PROMPT and _HWND). However, some
	// providers do not need initialization properties and may therefore
	// not support the _PROMPT and _HWND properties. Because of this, we will
	// not check the return value from SetProperties
	hr = pIDBProperties->SetProperties(1, rgPropSets);

	// Now that we've set our properties, initialize the provider
	XCHECK_HR(hr = pIDBInitialize->Initialize());

CLEANUP:
	if( pIDBProperties )
		pIDBProperties->Release();
	if( pIDBInitialize )
		pIDBInitialize->Release();
	return hr;
}



/////////////////////////////////////////////////////////////////
// myGetProperty
//
//	This function gets the BOOL value for the specified property
//	and returns the result in *pbValue.
//
/////////////////////////////////////////////////////////////////
HRESULT myGetProperty
	(
	IUnknown *				pIUnknown, 
	REFIID					riid, 
	DBPROPID				dwPropertyID, 
	REFGUID					guidPropertySet, 
	BOOL *					pbValue
	)
{
	HRESULT					hr;
	DBPROPID				rgPropertyIDs[1];
	DBPROPIDSET				rgPropertyIDSets[1];
	
	ULONG					cPropSets					= 0;
	DBPROPSET *				rgPropSets					= NULL;

	IDBProperties *			pIDBProperties				= NULL;
	ISessionProperties *	pISesProps					= NULL;
	ICommandProperties *	pICmdProps					= NULL;
	IRowsetInfo *			pIRowsetInfo				= NULL;

	// Initialize the output value
	*pbValue = FALSE;

	// Set up the property ID array
	rgPropertyIDs[0] = dwPropertyID;
	
	// Set up the Property ID Set
	rgPropertyIDSets[0].rgPropertyIDs	= rgPropertyIDs;
	rgPropertyIDSets[0].cPropertyIDs	= 1;
	rgPropertyIDSets[0].guidPropertySet	= guidPropertySet;

	// Get the property value for this property from the provider, but
	// don't try to display extended error information, since this may
	// not be a supported property: a failure is, in fact, expected if
	// the property is not supported
	if( riid == IID_IDBProperties )
	{
		XCHECK_HR(hr = pIUnknown->QueryInterface(IID_IDBProperties, 
					(void**)&pIDBProperties));
		CHECK_HR(hr = pIDBProperties->GetProperties(
					1,					//cPropertyIDSets
					rgPropertyIDSets,	//rgPropertyIDSets
					&cPropSets,			//pcPropSets
					&rgPropSets			//prgPropSets
					));
	}
	else if( riid == IID_ISessionProperties )
	{
		XCHECK_HR(hr = pIUnknown->QueryInterface(IID_ISessionProperties, 
					(void**)&pISesProps));
		CHECK_HR(hr = pISesProps->GetProperties(
					1,					//cPropertyIDSets
					rgPropertyIDSets,	//rgPropertyIDSets
					&cPropSets,			//pcPropSets
					&rgPropSets			//prgPropSets
					));
	}
	else if( riid == IID_ICommandProperties )
	{
		XCHECK_HR(hr = pIUnknown->QueryInterface(IID_ICommandProperties, 
					(void**)&pICmdProps));
		CHECK_HR(hr = pICmdProps->GetProperties(
					1,					//cPropertyIDSets
					rgPropertyIDSets,	//rgPropertyIDSets
					&cPropSets,			//pcPropSets
					&rgPropSets			//prgPropSets
					));
	}
	else
	{
		XCHECK_HR(hr = pIUnknown->QueryInterface(IID_IRowsetInfo, 
					(void**)&pIRowsetInfo));
		CHECK_HR(hr = pIRowsetInfo->GetProperties(
					1,					//cPropertyIDSets
					rgPropertyIDSets,	//rgPropertyIDSets
					&cPropSets,			//pcPropSets
					&rgPropSets			//prgPropSets
					));
	}

	// Return the value for this property to the caller if
	// it's a VT_BOOL type value, as expected
	if( V_VT(&rgPropSets[0].rgProperties[0].vValue) == VT_BOOL )
		*pbValue = V_BOOL(&rgPropSets[0].rgProperties[0].vValue);

CLEANUP:
	if( rgPropSets )
	{
		CoTaskMemFree(rgPropSets[0].rgProperties);
		CoTaskMemFree(rgPropSets);
	}
	if( pIDBProperties )
		pIDBProperties->Release();
	if( pISesProps )
		pISesProps->Release();
	if( pICmdProps )
		pICmdProps->Release();
	if( pIRowsetInfo )
		pIRowsetInfo->Release();
	return hr;
}



/////////////////////////////////////////////////////////////////
// myAddProperty
//
//	This function initializes the property structure pProp
//
/////////////////////////////////////////////////////////////////
void myAddProperty
	(
	DBPROP *				pProp, 
	DBPROPID				dwPropertyID, 
	VARTYPE					vtType, 
	LONG_PTR				lValue, 
	DBPROPOPTIONS			dwOptions
	)
{
	// Set up the property structure
	pProp->dwPropertyID		= dwPropertyID;
	pProp->dwOptions		= dwOptions;
	pProp->dwStatus			= DBPROPSTATUS_OK;
	pProp->colid			= DB_NULLID;
	V_VT(&pProp->vValue)	= vtType;

	// Since VARIANT data is a union, we can place the value in any
	// member (except for VT_DECIMAL, which is a union with the whole
	// VARIANT structure -- but we know we're not passing VT_DECIMAL)
	if (vtType != VT_I8)
		V_I4(&pProp->vValue)	= (LONG)lValue;
	else 
		V_I8(&pProp->vValue)	= lValue;
}
