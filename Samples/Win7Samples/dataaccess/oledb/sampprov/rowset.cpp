//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module ROWSET.CPP | CRowset object implementation
//
//

// Includes ------------------------------------------------------------------

#include "headers.h"

static const int MAX_BITS = 1008;

static const int TYPE_CHAR = 1;
static const int TYPE_SLONG = 3;

// Code ----------------------------------------------------------------------

// CRowset::CRowset ----------------------------------------------------------
//
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CRowset::CRowset
    (
    LPUNKNOWN pUnkOuter         //@parm IN | Outer Unkown Pointer
    )	// invoke ctor for base class
	: CBaseObj( BOT_ROWSET )
{

    //  Initialize simple member vars
    m_cRef          = 0L;
	m_pUnkOuter		= pUnkOuter ? pUnkOuter : this;

    // Intialize buffered row count + pointers to allocated memory
    m_pFileio           = NULL;
    m_cRows             = 0;
    m_irowFilePos       = 0;
    m_irowLastFilePos   = 0;
    m_cbTotalRowSize    = 0;
    m_cbRowSize         = 0;
    m_irowMin           = 0;
    m_ulRowRefCount     = 0;
    m_pextbufferAccessor= NULL;
    m_pIBuffer          = NULL;
    m_prowbitsIBuffer   = NULL;
    m_pLastBindBase     = NULL;
    m_rgbRowData        = NULL;
    m_dwStatus          = 0;
    m_pUtilProp         = NULL;
	m_pCreator			= NULL;

	m_wszFilePath[0]		= L'\0';
	m_wszDataSourcePath[0]	= L'\0';

    //  Initially, NULL all contained interfaces
    m_pIAccessor                    = NULL;
    m_pIColumnsInfo                 = NULL;
	m_pIConvertType					= NULL;
    m_pIRowset                      = NULL;
    m_pIRowsetChange                = NULL;
    m_pIRowsetIdentity              = NULL;
    m_pIRowsetInfo                  = NULL;
	m_pIGetRow						= NULL;

    // Increment global object count.
    OBJECT_CONSTRUCTED();

    return;
}


// CRowset::~CRowset ---------------------------------------------------------
//
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CRowset:: ~CRowset
    (
    void
    )
{
    // Free pointers.
    // (Note delete is safe for NULL ptr.)
    SAFE_DELETE( m_prowbitsIBuffer );
    SAFE_DELETE( m_pUtilProp );

    if (m_pIBuffer)
        ReleaseSlotList( m_pIBuffer );

    // Free accessors.
    // Each accessor is allocated via new/delete.
    // We store an array of ptrs to each accessor (m_pextbufferAccessor).
    if (m_pextbufferAccessor)
	{
        HACCESSOR   hAccessor, hAccessorLast;
        PACCESSOR   pAccessor;

        m_pextbufferAccessor->GetFirstLastItemH(hAccessor, hAccessorLast);
        
		for (; hAccessor <= hAccessorLast; hAccessor++)
		{
            m_pextbufferAccessor->GetItemOfExtBuffer(hAccessor, &pAccessor);
            SAFE_DELETE( pAccessor );
		}
        
		SAFE_DELETE( m_pextbufferAccessor );
	}

    //  Free contained interfaces
    SAFE_DELETE( m_pIAccessor );
    SAFE_DELETE( m_pIColumnsInfo );
    SAFE_DELETE( m_pIConvertType );
    SAFE_DELETE( m_pIRowset );
    SAFE_DELETE( m_pIRowsetChange );
    SAFE_DELETE( m_pIRowsetIdentity );
    SAFE_DELETE( m_pIRowsetInfo );
	SAFE_DELETE( m_pIGetRow );

    // free CFileio object
    if (m_pFileio)
        SAFE_DELETE( m_pFileio );

	if( m_pCreator )
		m_pCreator->GetOuterUnknown()->Release();

	if( m_pParentObj )	
		m_pParentObj->GetOuterUnknown()->Release();

    // Decrement global object count.
    OBJECT_DESTRUCTED();

    return;
}

// CRowset::FInit ------------------------------------------------------------
//
// @mfunc Initialize the rowset Object
//
// @rdesc Did the Initialization Succeed
//      @flag  TRUE | Initialization succeeded
//      @flag  FALSE | Initialization failed
//
BOOL CRowset::FInit
    (
    CFileIO *   pCFileio,		//@parm IN | pointer to Fileio object
	CBaseObj *	pParentBaseObj,	//@parm IN | pointer to Base Object creating the rowset
	WCHAR *		pwszFilePath,	//@parm IN | The File Path of the csv file
	WCHAR *		pwszDataSource	//@parm IN | The datasource path
    )
{
	// Asserts
	assert(pCFileio);
	assert(pParentBaseObj);
    assert(m_pUnkOuter);
    assert(pParentBaseObj);

    LPUNKNOWN pIUnknown = m_pUnkOuter;
    m_pFileio = pCFileio;

	m_pParentObj = pParentBaseObj;
	m_pParentObj->GetOuterUnknown()->AddRef();

    //--------------------
    // Get FileInfo
    //--------------------
    // Find # of columns in the result set.
    m_cCols = m_pFileio->GetColumnCnt();
    if( m_cCols <= 0 )
        return FALSE;

	m_cbRowSize = m_pFileio->GetRowSize();
    
    if (FAILED( CreateHelperFunctions()))
        return FALSE;
    
    m_cbTotalRowSize = m_pIBuffer->cbSlot;

    //--------------------
    // Perform binding
    //--------------------
    // Bind result set columns to the first row of the internal buffer.
    // For each column bind it's data as well as length. Leave space for
    // derived status info.
    // Note that we could defer binding, but this way we can check for
    // bad errors before we begin.
    // We may need to bind again if going back and forth
    // with GetNextRows.
    assert(m_rgbRowData);
    if (FAILED( Rebind((BYTE *) GetRowBuff( m_irowMin, TRUE ))))
        return FALSE;

    // allocate utility object that manages our properties
    m_pUtilProp = new CUtilProp();
	if (!m_pUtilProp)
		return FALSE;

    // Allocate contained interface objects
    // Note that our approach is simple - we always create *all* of the Rowset interfaces
    // If our properties were read\write (i.e., could be set), we would need to
    // consult properties to known which interfaces to create.
    // Also, none of our interfaces conflict. If any did conflict, then we could
    // not just blindly create them all.
    m_pIColumnsInfo             = new CImpIColumnsInfo( this, pIUnknown );
	m_pIConvertType				= new CImpIConvertType(this, pIUnknown);
    m_pIRowset                  = new CImpIRowset( this, pIUnknown );
    m_pIRowsetIdentity          = new CImpIRowsetIdentity( this, pIUnknown );
    m_pIRowsetInfo              = new CImpIRowsetInfo( this, pIUnknown );
	m_pIAccessor                = new CImpIAccessor( this, pIUnknown );
	m_pIGetRow					= new CImpIGetRow( this, pIUnknown );

	if (!m_pFileio->IsReadOnly())
		m_pIRowsetChange		= new CImpIRowsetChange( this, pIUnknown );

	if (m_pIAccessor && FAILED(m_pIAccessor->FInit(TRUE)))
		return FALSE;

	StringCchCopyW(m_wszFilePath, sizeof(m_wszFilePath)/sizeof(WCHAR), pwszFilePath);
	StringCchCopyW(m_wszDataSourcePath, sizeof(m_wszDataSourcePath)/sizeof(WCHAR), pwszDataSource);

    // if all interfaces were created, return success
    return (BOOL) (m_pIAccessor && m_pIColumnsInfo && m_pIConvertType &&
                   m_pIRowset && m_pIRowsetIdentity && m_pIRowsetInfo &&
				   ((m_pFileio->IsReadOnly() && !m_pIRowsetChange) ||
				    (!m_pFileio->IsReadOnly() && m_pIRowsetChange)) &&
					m_pIGetRow);
}


// CRowset::QueryInterface ---------------------------------------------------
//
// @mfunc Returns a pointer to a specified interface. Callers use
// QueryInterface to determine which interfaces the called object
// supports.
//
// @rdesc HRESULT indicating the status of the method
//      @flag S_OK | Interface is supported and ppvObject is set.
//      @flag E_NOINTERFACE | Interface is not supported by the object
//      @flag E_INVALIDARG | One or more arguments are invalid.
//
STDMETHODIMP CRowset::QueryInterface
    (
    REFIID riid,
    LPVOID * ppv
    )
{
    if (NULL == ppv)
        return ResultFromScode( E_INVALIDARG );

    //  Place NULL in *ppv in case of failure
    *ppv = NULL;

    //  This is the non-delegating IUnknown implementation
    if (riid == IID_IUnknown)
        *ppv = (LPVOID) this;
    else if (riid == IID_IAccessor)
        *ppv = (LPVOID) m_pIAccessor;
    else if (riid == IID_IColumnsInfo)
        *ppv = (LPVOID) m_pIColumnsInfo;
    else if (riid == IID_IConvertType)
        *ppv = (LPVOID) m_pIConvertType;
    else if (riid == IID_IRowset)
        *ppv = (LPVOID) m_pIRowset;
    else if (riid == IID_IRowsetChange && SupportIRowsetChange() )
        *ppv = (LPVOID) m_pIRowsetChange;
    else if (riid == IID_IRowsetIdentity)
        *ppv = (LPVOID) m_pIRowsetIdentity;
    else if (riid == IID_IRowsetInfo)
        *ppv = (LPVOID) m_pIRowsetInfo;
	else if (riid == IID_IGetRow)
		*ppv = (LPVOID) m_pIGetRow;

    //  If we're going to return an interface, AddRef it first
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
        return ResultFromScode( S_OK );
        }
    else
        return ResultFromScode( E_NOINTERFACE );
}


// CRowset::AddRef -----------------------------------------------------------
//
// @mfunc Increments a persistence count for the object
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CRowset::AddRef
     (
     void
     )
{
    return ++m_cRef;
}


// CRowset::Release ----------------------------------------------------------
//
// @mfunc Decrements a persistence count for the object and if
// persistence count is 0, the object destroys itself.
//
// @rdesc Current reference count
//
STDMETHODIMP_( DBREFCOUNT ) CRowset::Release
     (
     void
     )
{
    if (!--m_cRef)
        {
        this->m_pCreator->DecrementOpenRowsets();
        delete this;
        return 0;
        }

    return m_cRef;
}

// CRowset::CreateHelperFunctions --------------------------------------------
//
// @mfunc Creates Helper classes that are needed to manage the Rowset Object
//
// @rdesc HRESULT
//      @flag S_OK | Helper classes created
//      @flag E_FAIL | Helper classes were not created
//
HRESULT CRowset::CreateHelperFunctions
    (
    void
    )
{
    //----------------------
    // Create helper objects
    //----------------------

    // Bit array to track presence/absence of rows.
    m_prowbitsIBuffer = new CBitArray;
    if( !m_prowbitsIBuffer || FAILED(m_prowbitsIBuffer->FInit(MAX_BITS, g_dwPageSize)))
        return ResultFromScode( E_FAIL );

    // List of free slots.
    // This manages the allocation of sets of contiguous rows.
    if (FAILED( InitializeSlotList( MAX_TOTAL_ROWBUFF_SIZE / (ULONG)m_cbRowSize,
                         (ULONG) m_cbRowSize, g_dwPageSize, m_prowbitsIBuffer,
                         &m_pIBuffer, &m_rgbRowData )))
        return ResultFromScode( E_FAIL );

    // Locate some free slots.
    // Should be at very beginning.
    // This tells us which row we will bind to: m_irowMin.
    if (FAILED( GetNextSlots( m_pIBuffer, 1, &m_irowMin )))
        return ResultFromScode( E_FAIL );

    ReleaseSlots( m_pIBuffer, m_irowMin, 1 );

    return ResultFromScode( S_OK );
}


// CRowset::Rebind --------------------------------------------
//
// @mfunc Establishes data offsets and type for the file
// routines to place the data
//
// @rdesc HRESULT
//      @flag S_OK | Bindings set fine
//      @flag E_FAIL | Bindings could not be set
//
HRESULT CRowset::Rebind
    (
    BYTE *pBase                 //@parm IN | Base pointer for Data Area
    )
{
    COLUMNDATA  *pColumn;

    // Bind result set columns.
    // Use established types and sizes and offsets.
    // Bind to internal row buffer, area beginning with 'pRowBuff'.
    //
    // For each column, bind it's data as well as length.
    // Offsets point to start of COLUMNDATA structure.

    assert( pBase );

    // Optimize by not doing it over again.
    if( pBase != m_pLastBindBase )
	{
        m_pLastBindBase = 0;

        for (DBORDINAL cCols=1; cCols <= m_cCols; cCols++)
		{
			pColumn = m_pFileio->GetColumnData(cCols, (ROWBUFF *)pBase);
            
			if( FAILED( m_pFileio->SetColumnBind(cCols, pColumn)) )
                return( E_FAIL );
		}

        // Remember in case we bind to same place again.
        m_pLastBindBase = pBase;
	}

    return( S_OK );
}


// CRowset::GetRowBuff--------------------------------------------
//
// @mfunc Shorthand way to get the address of a row buffer.
// Later we may extend this so that it can span several
// non-contiguous blocks of memory.
//
// @rdesc Pointer to the buffer.
//
ROWBUFF* CRowset::GetRowBuff
    (
    DBCOUNTITEM iRow,           //@parm IN | Row to get address of.
    BOOL  fDataLocation         //@parm IN | Get the Data offset.
    )
{
    // This assumes iRow is valid...
    // How can we calculate maximum row?
    // Should we return NULL if it's out of range?
    assert( m_rgbRowData );
    assert( m_cbRowSize );
    assert( iRow > 0 );

	// Get the LSTSlot address or the Data offset (We need to make sure this structure is properly aligned)
	if ( fDataLocation )
		return (ROWBUFF *) ROUND_UP ((m_rgbRowData + m_cbTotalRowSize*iRow), COLUMN_ALIGNVAL);
	else
		return (ROWBUFF *) ROUND_UP ((m_rgbRowData + m_cbRowSize + (m_cbTotalRowSize*(iRow-1))), COLUMN_ALIGNVAL);
}




//////////////////////////////////////////////////////////////////////////////
//      Helper functions    Helper functions    Helper functions
//////////////////////////////////////////////////////////////////////////////

// GetInternalTypeFromCSVType ------------------------------------------------
//
// @func This function returns the default OLE DB representation
// for a data type
//
HRESULT GetInternalTypeFromCSVType
    (
    SWORD  swDataType,  //@parm IN | Data Type
    BOOL   fIsSigned,   //@parm IN | Signed or Unsigned
    DWORD  *pdwdbType   //@parm OUT | OLE DB type for DBColumnInfo
    )
{
    static struct {
        SWORD   swCSVType;
        BOOL    fIsSigned;          // 1=signed, 0=unsigned
        BOOL    fSignedDistinction; // 1=signed matters
        DWORD   dwdbType;
    } TypeTable[] =
        {
            {TYPE_CHAR,         0, 0, DBTYPE_STR },
            {TYPE_SLONG,        1, 1, DBTYPE_I4  },
        };

    for (int j=0; j < NUMELEM( TypeTable ); j++)
        {
        if (swDataType == TypeTable[j].swCSVType        // type match
        && (!TypeTable[j].fSignedDistinction            // care about sign?
            || fIsSigned == TypeTable[j].fIsSigned))    // sign match
            {
            assert( pdwdbType );
            *pdwdbType     = TypeTable[j].dwdbType;
            return ResultFromScode( S_OK );
            }
        }

    // Should never get here, since we supposedly have
    // a table of all possible CSV types.
    assert( !"Unmatched CSV Type." );
    return ResultFromScode( E_FAIL );
}

// SupportIRowsetChange ------------------------------------------------
//
// @func This function returns if IrowsetChange is supported
//
BOOL CRowset::SupportIRowsetChange()
{
	BOOL		fIRowsetChange = FALSE;
	ULONG		cPropSets      = 0;
	DBPROPSET *	rgPropSets     = NULL;
	DBPROPID	rgPropId[1];
	DBPROPIDSET	rgPropertySets[1];

    // Get the value of the DBPROP_CANHOLDROWS property
	rgPropertySets[0].guidPropertySet = DBPROPSET_ROWSET;
	rgPropertySets[0].rgPropertyIDs	  = rgPropId;
	rgPropertySets[0].cPropertyIDs	  = 1;
	rgPropId[0]                       = DBPROP_IRowsetChange;

	// Get the IRowsetChange Property from m_pUtilProp
	GetCUtilProp()->GetProperties(PROPSET_ROWSET, 1, rgPropertySets, 
											&cPropSets, &rgPropSets);
	
	// Get the Prompt value
	if( V_BOOL(&rgPropSets->rgProperties->vValue) == VARIANT_TRUE )
		fIRowsetChange = TRUE;

	// release properties 
	FreeProperties(&cPropSets, &rgPropSets);

	return fIRowsetChange;
}
