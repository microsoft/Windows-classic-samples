//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module OPNROWST.CPP | IOpenRowset interface implementation
//

// Includes ------------------------------------------------------------------

#include "headers.h"


// Code ----------------------------------------------------------------------



// CImpIOpenRowset::OpenRowset ------------------------------------------------
//
// @mfunc Opens and returns a rowset that includes all rows from a single base table
//
// @rdesc HRESULT
//      @flag S_OK                  | The method succeeded
//      @flag E_INVALIDARG          | pTableID was NULL
//      @flag E_FAIL                | Provider-specific error
//      @flag DB_E_NOTABLE          | Specified table does not exist in current Data
//                                  | Data Source object
//      @flag E_OUTOFMEMORY         | Out of memory
//      @flag E_NOINTERFACE         | The requested interface was not available

STDMETHODIMP CImpIOpenRowset::OpenRowset
    (
    IUnknown*                   pUnkOuter,          //@parm IN    | Controlling unknown, if any
    DBID*                       pTableID,           //@parm IN    | table to open
	DBID*						pIndexID,			//@parm IN	  | DBID of the index
    REFIID                      riid,               //@parm IN    | interface to return
    ULONG                       cPropertySets,      //@parm IN    | count of properties
    DBPROPSET					rgPropertySets[],   //@parm INOUT | array of property values
    IUnknown**                  ppRowset            //@parm OUT   | where to return interface
    )
{
	CCommand*				pCCommand = NULL;
	ICommandText*			pICmdText = NULL;
	ICommandProperties*		pICmdProp = NULL;
    CRowset*    pRowset = NULL;

    HRESULT     hr;
	HRESULT		hrProp	= ResultFromScode( S_OK );
	ULONG		ul,ul2;

    assert( m_pObj );
	assert( m_pObj->m_pUtilProp );

    // NULL out-params in case of error
    if( ppRowset )
	    *ppRowset = NULL;

    // Check Arguments
	if ( !pTableID && !pIndexID )
        return ResultFromScode( E_INVALIDARG );

	if ( riid == IID_NULL)
        return ResultFromScode( E_NOINTERFACE );

	// Check Arguments for use by properties
	if( cPropertySets ) 
	{
		hr = m_pObj->m_pUtilProp->SetPropertiesArgChk(cPropertySets, 
													  rgPropertySets);	
		if( FAILED(hr) )
			return hr;
	}

	// We only accept NULL for pIndexID
	if( pIndexID )
		return ResultFromScode( DB_E_NOINDEX );

	// If the eKind is not known to use, basically it
	// means we have no table identifier
	if ( (!pTableID ) || ( pTableID->eKind != DBKIND_NAME ) ||
	   ( (pTableID->eKind == DBKIND_NAME) && (!(pTableID->uName.pwszName)) ) ||
	   ( wcslen(pTableID->uName.pwszName) == 0 ) ||
	   ( wcslen(pTableID->uName.pwszName) > _MAX_FNAME ) )
		return ResultFromScode( DB_E_NOTABLE );

	// We do not allow the riid to be anything other than IID_IUnknown for aggregation
	if ( (pUnkOuter) && (riid != IID_IUnknown) )
		return ResultFromScode( DB_E_NOAGGREGATION );
   
	// Create a Command Object
	// This is the outer unknown from the user, for the new Command,
	// not to be confused with the outer unknown of this DBSession object.
	pCCommand = new CCommand(m_pObj, NULL);
	if (pCCommand)
	{		
		hr = pCCommand->FInit();
		if( FAILED(hr) )
		{
			delete pCCommand;	// clean up.
			pCCommand = NULL;
		}
	}
	else
	{
		// Since Ctor failed, it cannot know to Release pUnkOuter, 
		// so we must do it here since we are owner.
		if (pUnkOuter)
			pUnkOuter->Release();
		hr = E_OUTOFMEMORY;
	}

	if( pCCommand )
	{
		// Since we are using the command objects interfaces to impersonate
		// doing IOpenRowset, we need to tell the command object to post errors
		// as IID_IOpenRowset
		pCCommand->SetImpersonateIID(&IID_IOpenRowset);


		// Allocate buffer for Table Name.
		if( FAILED(hr = pCCommand->QueryInterface(IID_ICommandText, (LPVOID*)&pICmdText)) )
		{
			assert(!"QI for ICmdText failed");
			goto EXIT;
		}
		assert( pICmdText ); 
		if( FAILED(hr = pICmdText->SetCommandText(DBGUID_DEFAULT, pTableID->uName.pwszName)) )
		{
			// Process Errors
			goto EXIT;				
		}				

		// Process Properties
		if( cPropertySets > 0 )
		{
			if( FAILED(hr = pCCommand->QueryInterface(IID_ICommandProperties, (LPVOID*)&pICmdProp)) )
			{
				assert(!"QI for ICmdProp failed");
				goto EXIT;
			}
			assert( pICmdProp );
			hr = (pICmdProp->SetProperties(cPropertySets, rgPropertySets));
			if( (hr == DB_E_ERRORSOCCURRED) || 
				(hr == DB_S_ERRORSOCCURRED) )
			{
				// If all the properties set were SETIFCHEAP then we can set 
				// our status to DB_S_ERRORSOCCURRED and continue.
				for(ul=0;ul<cPropertySets; ul++)
				{
					for(ul2=0;ul2<rgPropertySets[ul].cProperties; ul2++)
					{
						// Check for a required property that failed, if found, we must return
						// DB_E_ERRORSOCCURRED
						if( (rgPropertySets[ul].rgProperties[ul2].dwStatus != DBPROPSTATUS_OK) &&
							(rgPropertySets[ul].rgProperties[ul2].dwOptions != DBPROPOPTIONS_SETIFCHEAP) )
						{
							hr = ResultFromScode(DB_E_ERRORSOCCURRED);
							goto EXIT;
						}
					}
				}
				hrProp = DB_S_ERRORSOCCURRED;
			}
			else if( FAILED(hr) )
			{
				goto EXIT;
			}
		}

		// Open the rowset using ::Execute
		// ppRowset buffer.
		if( ppRowset )
		{
			hr = pICmdText->Execute(pUnkOuter, riid, NULL, NULL, ppRowset);			
								
			// Sample Provider's property handling is simplistic.
			// All property errors should have been caught by the previous
			// call to ::SetProperties.
			// Assume that a failure was caused by something other
			// than bad properties.
			// So, no need for property post-processing
		}
	}

EXIT:
	if( pICmdText )
		pICmdText->Release();
	if( pICmdProp )
		pICmdProp->Release();

	// DB_S_ERRORSOCCURRED must take precedence over DB_S_NOTSINGLETON
	return (hr == S_OK || hr == DB_S_NOTSINGLETON) ? hrProp : hr;
}
