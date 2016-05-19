//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module DBINIT.CPP | IDBInitialize interface implementation
//
//

// Includes ------------------------------------------------------------------
#include "headers.h"
#include <msdasc.h>		//IDBPromptInitialize

// IDBInitialize specific interface methods


// CImpIDBInitialize::Initialize ---------------------------------------------
//
// @mfunc Initializes the DataSource object.. For this provider it requires
// that a valid path is given to where the file will be located.
//
// @rdesc HRESULT
//      @flag S_OK          | Path exists
//      @flag E_FAIL        | Invalid path
//      @flag E_INVALIDARG  | Invalid Parameters passed in
//		@flag DB_E_ALREADYINITIALIZED | Datasource Object already initialized
//
STDMETHODIMP CImpIDBInitialize::Initialize
    (
	)
{
    ASSERT( m_pObj );
	CHAR szDataSource[MAX_PATH + MAX_PATH + 1] = {0};
	
	HRESULT		hr;
	ULONG		cPropSets	= 0;
	DBPROPSET*	rgPropSets = NULL;

	WCHAR* pwszDataSource = NULL;
    int  nPrompt = DBPROMPT_NOPROMPT;
	HWND hWnd = NULL;

	IDBPromptInitialize* pIDBPromptInitialize = NULL;
	IDBProperties*		 pIDBProperties	      = NULL;

    if(m_pObj->m_fDSOInitialized)
        return ResultFromScode( DB_E_ALREADYINITIALIZED );

	//Setup Properties
	const ULONG	cProperties		= 3;
	DBPROPID	rgProperties[cProperties] = { DBPROP_INIT_DATASOURCE, DBPROP_INIT_PROMPT, DBPROP_INIT_HWND };

	const ULONG cPropertyIDSets = 1;
	DBPROPIDSET	rgPropertyIDSets[cPropertyIDSets];
	rgPropertyIDSets[0].guidPropertySet		= DBPROPSET_DBINIT;
	rgPropertyIDSets[0].rgPropertyIDs		= rgProperties;
	rgPropertyIDSets[0].cPropertyIDs		= cProperties;

    //Get the property values
    hr = m_pObj->m_pUtilProp->GetProperties(PROPSET_DSO, cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets );
	if(SUCCEEDED(hr))
	{
		pwszDataSource	= V_BSTR(&rgPropSets[0].rgProperties[0].vValue);
		nPrompt			= V_I2(&rgPropSets[0].rgProperties[1].vValue);

#ifdef _WIN64
		hWnd			= (HWND)V_I8(&rgPropSets[0].rgProperties[2].vValue);
#else
		hWnd			= (HWND)V_I4(&rgPropSets[0].rgProperties[2].vValue);
#endif	
		//Convert Directory to MBCS
		ConvertToMBCS(pwszDataSource, szDataSource, sizeof(szDataSource));
	}

	// if caller didn't supply a directory path, ask the user
    if ((nPrompt == DBPROMPT_PROMPT) || 
		((nPrompt == DBPROMPT_COMPLETE || nPrompt == DBPROMPT_COMPLETEREQUIRED) && (!SetCurrentDirectory(szDataSource))) )
    {
	    //Use our current DataSource as defaults for properties
		TESTC(hr = QueryInterface(IID_IDBProperties, (void**)&pIDBProperties));
		
		//Use the Data Links Dialog to prompt...
		TESTC(hr = CoCreateInstance(CLSID_DataLinks, NULL, CLSCTX_ALL, IID_IDBPromptInitialize, (void**)&pIDBPromptInitialize));
		TESTC(hr = pIDBPromptInitialize->PromptDataSource(NULL, hWnd, DBPROMPTOPTIONS_PROPERTYSHEET | DBPROMPTOPTIONS_DISABLE_PROVIDER_SELECTION, 0, NULL, NULL, IID_IDBProperties, (IUnknown**)&pIDBProperties));
		
		//Free the Previous Properties
		FreeProperties(&cPropSets, &rgPropSets);
		pwszDataSource = NULL;

		//Now Data Links has put the user choosen properties onto our data source, we just need to
		//reobtain the property values...
		hr = m_pObj->m_pUtilProp->GetProperties(PROPSET_DSO, cPropertyIDSets, rgPropertyIDSets, &cPropSets, &rgPropSets );
		if(SUCCEEDED(hr))
		{
			pwszDataSource	= V_BSTR(&rgPropSets[0].rgProperties[0].vValue);
			nPrompt			= V_I2(&rgPropSets[0].rgProperties[1].vValue);

#ifdef _WIN64
			hWnd			= (HWND)V_I8(&rgPropSets[0].rgProperties[2].vValue);
#else
			hWnd			= (HWND)V_I4(&rgPropSets[0].rgProperties[2].vValue);
#endif
			//Convert Directory to MBCS
			ConvertToMBCS(pwszDataSource, szDataSource, sizeof(szDataSource));
		}
	}

	//NOTE:  We don't require DBPROP_INIT_DATASOURCE.  If the DataSource property is passed
	//in then we will use that to represent the current directory.  This will allow the user
	//to only have to specify the filename in OpenRowset, and not the full path...
	if(szDataSource[0])
	{
		// Get the current Directory
		CHAR szCurrentDir[MAX_PATH];
		GetCurrentDirectory( MAX_PATH, szCurrentDir );
		
		//See if the directory passed is valid...
		if(!SetCurrentDirectory( szDataSource ))
		{
			hr = DB_E_ERRORSOCCURRED;
			goto CLEANUP;
		}

		// Restore to the original directory.
		SetCurrentDirectory( szCurrentDir );
		StringCchCopyW( m_pObj->m_wszPath, MAX_PATH, pwszDataSource);
	}
	else
	{
		m_pObj->m_wszPath[0] = wEOL;
	}

	//We are now initialized...
	m_pObj->m_fDSOInitialized = TRUE;
	hr = S_OK;

CLEANUP:
	FreeProperties(&cPropSets, &rgPropSets);
	SAFE_RELEASE(pIDBPromptInitialize);
	SAFE_RELEASE(pIDBProperties);
	return hr;
}



// CImpIDBInitialize::Uninitialize ---------------------------------------------
//
// @mfunc Returns the Data Source Object to an uninitialized state
//
// @rdesc HRESULT
//      @flag S_OK            | The method succeeded
//      @flag DB_E_OBJECTOPEN | A DBSession object was already created
//
STDMETHODIMP CImpIDBInitialize::Uninitialize
    (
    void
    )
{
    assert( m_pObj );

    if (!m_pObj->m_fDSOInitialized)
        {
        // data source object is not initialized; do nothing
        return ResultFromScode( S_OK );
        }
    else
        {
        if (!m_pObj->m_fDBSessionCreated)
            {
            // DSO initialized, but no DBSession has been created.
            // So, reset DSO to uninitialized state
            m_pObj->m_fDSOInitialized = FALSE;
            return ResultFromScode( S_OK );
            }
        else
            {
            // DBSession has already been created; trying to uninit
            // the DSO now is an error
            return ResultFromScode( DB_E_OBJECTOPEN );
            }
        }
}



