//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991-1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ACCESSOR.CPP | CImpIAccessor object implementation
//
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

// Code ----------------------------------------------------------------------

// CImpIAccessor::FInit--------------------------------------------------------------
//
// @mfunc Initialize the Accessor implementation object.
//
// @rdesc Did the Initialization Succeed
// 		@flag S_OK 			| initialization succeeded,
// 		@flag E_OUTOFMEMORY | initialization failed because of memory allocation problem,
//-----------------------------------------------------------------------------------

STDMETHODIMP  CImpIAccessor::FInit
	(
	BOOL	fUnderRowset		//@parm IN | Is aggregated within Rowset?   
	)
{
	// Create the Extended Buffer array.
    // This is an array of pointers to malloc'd accessors.
    m_pextbufferAccessor = (LPEXTBUFFER) new CExtBuffer;
    if (m_pextbufferAccessor == NULL || FAILED( m_pextbufferAccessor->FInit( 1, sizeof( PACCESSOR ), g_dwPageSize )))
        return ResultFromScode( E_OUTOFMEMORY );

	// If aggregated under Rowset do the following:
	if(fUnderRowset)
	{
		// For efficiency reasons put a copy of the accessors buffer
		// directly on the CRowset object.
		((CRowset *)m_pObj)->m_pextbufferAccessor = m_pextbufferAccessor;

		HRESULT			hr;
		CCommand    	*pCCommand;
		CImpIAccessor	*pIAccessorCommand;

		// See if there are any accessors created on the Command object, and if so
		// copy their handles so they can be used on the Rowset.
		pCCommand = ((CRowset *)m_pObj)->m_pCreator;
		if(pCCommand)
		{
			if(SUCCEEDED(pCCommand->QueryInterface(IID_IAccessor, (void **)&pIAccessorCommand)) && pIAccessorCommand)
			{
				// Copy handles.
				hr = pIAccessorCommand->CopyAccessors(m_pextbufferAccessor);
				pIAccessorCommand->Release();
				if(FAILED(hr))
					return hr;
			}
		}
	}

	return NOERROR;
}

// IAccessor specific methods

// CImpIAccessor::AddRefAccessor -----------------------------------------
//
// @mfunc Adds a reference count to an existing accessor
//
// @rdesc HRESULT
//      @flag S_OK                      | Method Succeeded
//      @flag E_FAIL                    | Provider specific Error
//
STDMETHODIMP CImpIAccessor::AddRefAccessor
    (
	HACCESSOR	hAccessor,		//@parm IN | Accessor Handle
	DBREFCOUNT*	pcRefCounts		//@parm OUT | Reference Count
    )
{
    // Retrieve our accessor structure from the client's hAccessor,
    // free it, then mark accessor ptr as unused.
    // We do not re-use accessor handles.  This way, we hope
    // to catch more client errors.  (Also, ExtBuffer doesn't
    // maintain a free list, so it doesn't know how to.)

    PACCESSOR   pAccessor;

    if( pcRefCounts )
		*pcRefCounts = 0;

	m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &pAccessor);
    if( !pAccessor )
        return DB_E_BADACCESSORHANDLE;

	InterlockedIncrement((LONG*)&(pAccessor->cRef));

	if( pcRefCounts )
		*pcRefCounts = (DBREFCOUNT)(pAccessor->cRef);

	return ResultFromScode( S_OK );
}


// CImpIAccessor::CreateAccessor -----------------------------------------
//
// @mfunc Creates a set of bindings that can be used to send data
// to or retrieve data from the data cache.
//
// @rdesc HRESULT
//      @flag S_OK                      | Method Succeeded
//      @flag E_FAIL                    | Provider specific Error
//      @flag E_INVALIDARG              | pHAccessor was NULL, dwAccessorFlags was
//                                        invalid, or cBindings was not 0 and
//                                        rgBindings was NULL
//      @flag E_OUTOFMEMORY             | Out of Memory
//      @flag DB_E_ERRORSOCCURRED		| dwBindPart in an rgBindings element was invalid, OR
//									    | Column number specified was out of range, OR
//										| Requested coercion is not supported.
//      @flag OTHER                     | Other HRESULTs returned by called functions
//
STDMETHODIMP CImpIAccessor::CreateAccessor
    (
    DBACCESSORFLAGS dwAccessorFlags,
    DBCOUNTITEM     cBindings,      //@parm IN | Number of Bindings
    const DBBINDING rgBindings[],   //@parm IN | Array of DBBINDINGS
    DBLENGTH        cbRowSize,      //@parm IN | Number of bytes in consumer's buffer
    HACCESSOR*      phAccessor,     //@parm OUT | Accessor Handle
	DBBINDSTATUS	rgStatus[]		//@parm OUT	| Binding status
    )
{
    PACCESSOR   pAccessor;
    HACCESSOR   hAccessor;
    DBCOUNTITEM cBind;
    DBORDINAL   cCols;
    HRESULT     hr;


    // Check Parameters
    if( (cBindings && !rgBindings) || (phAccessor == NULL) )
        return ResultFromScode( E_INVALIDARG );

    // init out params
    *phAccessor = NULL;

	// Check if we have a correct accessor type
    if ( dwAccessorFlags & DBACCESSOR_PASSBYREF )
        return ResultFromScode( DB_E_BYREFACCESSORNOTSUPPORTED );

	// Only allow DBACCESSOR_ROWDATA and DBACCESSOR_OPTIMIZED
    if ( (dwAccessorFlags & ~DBACCESSOR_OPTIMIZED ) != DBACCESSOR_ROWDATA )
        return ResultFromScode( DB_E_BADACCESSORFLAGS );

	// Check for NULL Accessor on the Command Object
	// Also check for NULL Accessor on a read only rowset
	if( (m_pObj->GetBaseObjectType() == BOT_COMMAND && !cBindings) ||
		(m_pObj->GetBaseObjectType() == BOT_ROWSET  && !cBindings && !((CRowset *)m_pObj)->SupportIRowsetChange()) )
		return ResultFromScode( DB_E_NULLACCESSORNOTSUPPORTED );

	// Check for Optimized Accessor on the Rowset Object after Fetch
	if( (dwAccessorFlags & DBACCESSOR_OPTIMIZED) && 
		m_pObj->GetBaseObjectType() == BOT_ROWSET && ((CRowset *)m_pObj)->m_cRows )
		return ResultFromScode( DB_E_BADACCESSORFLAGS );

	// Initialize the status array to DBBINDSTATUS_OK.
	if ( rgStatus )
		memset(rgStatus, 0x00, cBindings * sizeof(DBBINDSTATUS));

    // Check on the bindings the user gave us.
    for (cBind=0, hr=NOERROR; cBind < cBindings; cBind++)
    {
		// other binding problems forbidden by OLE-DB
        const DBTYPE currType = rgBindings[cBind].wType;
        const DBTYPE currTypePtr = currType &
        					(DBTYPE_BYREF|DBTYPE_ARRAY|DBTYPE_VECTOR);
		const DBTYPE currTypeBase = currType & 
							~(DBTYPE_BYREF|DBTYPE_ARRAY|DBTYPE_VECTOR);
        const DWORD  currFlags = rgBindings[cBind].dwFlags;

        cCols = rgBindings[cBind].iOrdinal;

		// Check for a Bad Ordinal
		if( m_pObj->GetBaseObjectType() == BOT_ROWSET )
		{
			// make sure column number is in range
			if ( !(0 < cCols && cCols <= ((CRowset *) m_pObj)->m_cCols) )
			{
				// Set Bind status to DBBINDSTATUS_BADORDINAL
				hr = ResultFromScode( DB_E_ERRORSOCCURRED );
				if ( rgStatus )
					rgStatus[cBind] = DBBINDSTATUS_BADORDINAL;
				continue;
			}
		}

		// At least one of these valid parts has to be set. In SetData I assume it is the case.
        if ( !(rgBindings[cBind].dwPart & (DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS)) )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// dwPart is something other than value, length, or status
		else if ( (rgBindings[cBind].dwPart & ~(DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS)) )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// wType was DBTYPE_EMPTY or DBTYPE_NULL
		else if ( (currType==DBTYPE_EMPTY || currType==DBTYPE_NULL) )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// wType was DBTYPE_BYREF or'ed with DBTYPE_EMPTY, NULL, or RESERVED
		else if ( ((currType & DBTYPE_BYREF) && 
			  (currTypeBase == DBTYPE_EMPTY || currTypeBase == DBTYPE_NULL || 
			   currType & DBTYPE_RESERVED)) )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// dwFlags was DBBINDFLAG_HTML and the type was not a String
		else if ( currFlags && (currFlags != DBBINDFLAG_HTML || 
				 (currTypeBase != DBTYPE_STR  &&
				  currTypeBase != DBTYPE_WSTR &&
				  currTypeBase != DBTYPE_BSTR)) )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// wType was used with more than one type indicators
		else if ( !(currTypePtr == 0 || currTypePtr == DBTYPE_BYREF ||
				currTypePtr == DBTYPE_ARRAY || currTypePtr == DBTYPE_VECTOR) )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// wType was a non pointer type with provider owned memory
		else if ( !currTypePtr && 
			 rgBindings[cBind].dwMemOwner==DBMEMOWNER_PROVIDEROWNED )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }

		// we only support client owned memory
		else if ( rgBindings[cBind].dwMemOwner != DBMEMOWNER_CLIENTOWNED )
		{
			// Set Bind status to DBBINDSTATUS_BADBINDINFO
            hr = ResultFromScode( DB_E_ERRORSOCCURRED );
			if ( rgStatus )
				rgStatus[cBind] = DBBINDSTATUS_BADBINDINFO;
        }
    }

    // Any errors amongst those checks?
    if (hr != NOERROR)
    {
        return hr;
    }

    // Make a copy of the client's binding array, and the type of binding.
    pAccessor = (ACCESSOR *) new BYTE[sizeof( ACCESSOR ) + (cBindings - 1) *sizeof( DBBINDING )];
    if ( pAccessor == NULL )
        return ResultFromScode( E_OUTOFMEMORY );

    // We store a ptr to the newly created variable-sized ACCESSOR.
    // We have an array of ptrs (to ACCESSOR's).
    // The handle is the index into the array of ptrs.
    // The InsertIntoExtBuffer function appends to the end of the array.
    assert( m_pextbufferAccessor );
    hr = m_pextbufferAccessor->InsertIntoExtBuffer(&pAccessor, hAccessor);
    if ( FAILED( hr ) )
	{
        SAFE_DELETE( pAccessor );
        return ResultFromScode( E_OUTOFMEMORY );
	}
    assert( hAccessor );

    // Copy the client's bindings into the ACCESSOR.
    pAccessor->dwAccessorFlags	= dwAccessorFlags;
    pAccessor->cBindings		= cBindings;
    pAccessor->cRef				= 1;		// Establish Reference count.
	
	memcpy( &(pAccessor->rgBindings[0]), &rgBindings[0], cBindings*sizeof( DBBINDING ));

    // fill out-param and return
    *phAccessor = hAccessor;
    return ResultFromScode( S_OK );
}


// CImpIAccessor::GetBindings --------------------------------------------------
//
// @mfunc Returns the bindings in an accessor
//
// @rdesc HRESULT
//      @flag S_OK                      | Method Succeeded
//      @flag E_INVALIDARG              | pdwAccessorFlags/pcBinding/prgBinding were NULL
//      @flag E_OUTOFMEMORY             | Out of Memory
//      @flag DB_E_BADACCESSORHANDLE    | Invalid Accessor given
//
STDMETHODIMP CImpIAccessor::GetBindings
    (
    HACCESSOR        hAccessor,         //@parm IN | Accessor Handle
    DBACCESSORFLAGS* pdwAccessorFlags,  //@parm OUT | Binding Type flag
    DBCOUNTITEM*     pcBindings,        //@parm OUT | Number of Bindings returned
    DBBINDING**      prgBindings        //@parm OUT | Bindings
    )
{
    // Retrieve our accessor structure from the client's hAccessor,
    // make a copy of the bindings for the user, then done.
    PACCESSOR   pAccessor;
    DBCOUNTITEM	cBindingSize;

    // init out-params
	if(	pdwAccessorFlags )
		*pdwAccessorFlags = 0;
	if( pcBindings )
		*pcBindings = 0;
	if ( prgBindings )
		*prgBindings = NULL;

    // check parameters
    if (!pdwAccessorFlags || !pcBindings || !prgBindings)
        return ResultFromScode( E_INVALIDARG );

    // Validate Accessor Handle
    m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &pAccessor);
    if( !pAccessor )
        return DB_E_BADACCESSORHANDLE;

    // Allocate and return Array of bindings
	cBindingSize = pAccessor->cBindings * sizeof( DBBINDING );
	if ( cBindingSize )
		*prgBindings = (DBBINDING *) PROVIDER_ALLOC( cBindingSize );
    
	// Check the Allocation
	if ( ( *prgBindings == NULL ) && ( cBindingSize ) )
        return ResultFromScode( E_OUTOFMEMORY );

    *pdwAccessorFlags = pAccessor->dwAccessorFlags;
    *pcBindings = pAccessor->cBindings;
    memcpy( *prgBindings, pAccessor->rgBindings, cBindingSize );

    // all went well..
    return ResultFromScode( S_OK );
}



// CImpIAccessor::ReleaseAccessor ---------------------------------------
//
// @mfunc Releases an Accessor
//
// @rdesc HRESULT
//      @flag S_OK                      | Method Succeeded
//      @flag DB_E_BADACCESSORHANDLE    | hAccessor was invalid
//
STDMETHODIMP CImpIAccessor::ReleaseAccessor
    (
    HACCESSOR	hAccessor,      //@parm IN | Accessor handle to release
	DBREFCOUNT*	pcRefCounts		//@parm OUT | Reference Count
    )
{
    // Retrieve our accessor structure from the client's hAccessor,
    // free it, then mark accessor ptr as unused.
    // We do not re-use accessor handles.  This way, we hope
    // to catch more client errors.  (Also, ExtBuffer doesn't
    // maintain a free list, so it doesn't know how to.)

    PACCESSOR   pAccessor;

	if( pcRefCounts )
		*pcRefCounts = 0;

    m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &pAccessor);
    if( !pAccessor )
        return DB_E_BADACCESSORHANDLE;

    // Free the actual structure.
	InterlockedDecrement((LONG*)&(pAccessor->cRef));
	
	if( pAccessor->cRef <= 0 )
	{
		SAFE_DELETE( pAccessor );
		if( pcRefCounts )
			*pcRefCounts = 0;

	    // Store a null in our array-of-ptrs,
	    // so we know next time that it is invalid.
	    // (operator[] returns a ptr to the space where the ptr is stored.)
	    *(PACCESSOR*) ((*m_pextbufferAccessor)[hAccessor]) = NULL;
	}
	else
	{
		if( pcRefCounts )
			*pcRefCounts = (DBREFCOUNT)(pAccessor->cRef);
	}

    return ResultFromScode( S_OK );
}



// CImpIAccessor::CopyAccessors------------------------------------------------------
//
// @mfunc Copies accessors from one instantiation of the IAccessor to another (e.g.
// from one under ICommand to the one owned by IRowset implementation object).
//
// @rdesc Returns one of the following values:
// 		@flag S_OK 						| copying of the accessors succeeded,
// 		@flag E_OUTOFMEMORY				| copying failed due to memory allocation
//										  failure for a new accessor,
// 		@flag OTHER						| other result codes returned by called
//										  functions.
//-----------------------------------------------------------------------------------

STDMETHODIMP CImpIAccessor::CopyAccessors
	(
	LPEXTBUFFER   	   pextbufferAccessor//@parm IN  | specifies extended buffer 
										  // to which accessors should be copied
	)
{
	HACCESSOR	hAccessor, hAccessorFirst, hAccessorLast, hAccessorDup;
	size_t		cbAccessor;
	PACCESSOR	paccessor=NULL, paccessorNew=NULL, paccessorNull=NULL;
	HRESULT		hr;

	assert(pextbufferAccessor);

	if(pextbufferAccessor)
	{
		// Get the number of available accessors.
		m_pextbufferAccessor->GetFirstLastItemH(hAccessorFirst, hAccessorLast);

		for(hAccessor=hAccessorFirst; hAccessor <= hAccessorLast; hAccessor++)
		{
			hr = m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &paccessor );
			if( FAILED(hr) )
				return hr;

			if (paccessor == NULL)
				paccessorNew = paccessorNull;	
			else
			{
				// Allocate space for the accessor structure.
				cbAccessor = sizeof(ACCESSOR) 
								+ (paccessor->cBindings ?
								  (paccessor->cBindings-1) : 0)
									* sizeof(DBBINDING);

				paccessorNew = (PACCESSOR) new BYTE [cbAccessor];
				if (paccessorNew == NULL)
					return ResultFromScode(E_OUTOFMEMORY);

				memcpy(	paccessorNew, paccessor, cbAccessor);

				paccessorNew->cRef = 1;
			}

			// Insert accessor ptr into the new buffer.
			hr = pextbufferAccessor->InsertIntoExtBuffer(&paccessorNew, hAccessorDup);
			
			// Can fail because of Out-Of-Memory condition.
			if(FAILED(hr))
			{
				SAFE_DELETE(paccessorNew);
				return hr;
			}
		}
	}

	return NOERROR;
}


