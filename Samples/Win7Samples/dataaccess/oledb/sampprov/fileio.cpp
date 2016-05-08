//--------------------------------------------------------------------
// Microsoft OLE DB 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module FILEIO.CPP | This module contains the File Manipulation code
// for a Comma Seperated Value (CSV) Simple Provider.
//
//
#include "headers.h"
#include "fileio.h"

static const int ARRAY_INIT_SIZE = 1000;

// Data Types supported
static const int TYPE_CHAR = 1;
static const int TYPE_SLONG = 3;

// Data Type Parse strings and lengths
static const char CHAR_STRING[] = "CHAR";
static const int CHAR_STRING_SIZE = 4;
static const char SLONG_STRING[] = "SLONG";
static const int SLONG_STRING_SIZE = 5;


//--------------------------------------------------------------------
// @mfunc Constructor for this class
//
// @rdesc NONE
//
CFileIO::CFileIO()
{
	memset(&m_rgpColumnData, 0, sizeof(PCOLUMNDATA) * MAX_COLUMNS);

    m_pColNames        = NULL;
    m_pvInput          = NULL;
    m_ulDataTypeOffset = 0;
    m_cColumns         = 0;
    m_cRows            = 0;
	m_FileReadOnly	   = FALSE;			
	m_pbHeap	       = NULL;
	m_cbHeapUsed       = 0;
	m_cbRowSize		   = 0;
}


//--------------------------------------------------------------------
// @mfunc Destructor for this class
//
// @rdesc NONE
//
CFileIO:: ~CFileIO()
{
	// Release buffer for column names (pointed to by m_rgdbcolinfo).
    if (m_pbHeap)
        VirtualFree((VOID *) m_pbHeap, 0, MEM_RELEASE );

    // Close file
    if (is_open())
        close();

    // Delete buffers
    SAFE_DELETE( m_pColNames );
    SAFE_DELETE( m_pvInput );
}


//--------------------------------------------------------------------
// @mfunc Initialization routine, opens file specified and creates
// buffers
//
// @rdesc HRESULTs
//      @flag S_OK | Succeeded
//      @flag E_FAIL | Failed to Initialize
//		@flag DB_E_NOTABLE | File marked as read only or didn't exist
//
HRESULT CFileIO::fInit
    (
    LPSTR ptstrFileName         //@parm IN | File Name to Open
    )
{
    // Allocate Stream Buffer
    m_pvInput = new char[MAX_INPUT_BUFFER ];
    if (NULL == m_pvInput)
        return ResultFromScode( E_FAIL );

    // Open the File
	open( ptstrFileName, ios::in | ios::out | ios::_Nocreate);//, filebuf::sh_read || filebuf::sh_write );
    if( !is_open() ) {
		open( ptstrFileName, ios::in | ios::_Nocreate);//, filebuf::sh_read );
		if (!is_open())
			return ResultFromScode( DB_E_NOTABLE );

		m_FileReadOnly = TRUE;
	}

    // Obtain the Column Names, Data Types, and Indexes
    // for each of the rows
    if (FAILED( GenerateFileInfo()))
        return ResultFromScode( E_FAIL );

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Retrieve the Name associated with a particular column.  If
// names have not been read from the file yet, retrieve those names.
// If a name does not exist for a column, fabricate one.
//
// @rdesc HRESULT
//      @flag S_OK | Succeeded
//      @flag E_FAIL | Invalid Column Number
//
HRESULT CFileIO::GetColumnName
    (
    DBORDINAL cCols,         //@parm IN | Column Number
    LPSTR*    pptstrName     //@parm OUT | Pointer to Column Name
    )
{
    // Column number greater than MAX
    if (cCols > MAX_COLUMNS)
        return ResultFromScode( E_FAIL );

    // If Column Names have not been retrieved,
    // then retrieve them into the internal array
    if (!m_pColNames)
        {
        // Save Current Position and move to beginning of file
        seekg( 0L );
        clear();

        // Retrieve the column names record
        getline( m_pvInput, MAX_INPUT_BUFFER );
        if (good() && 0 < gcount())
        {
			m_pColNames = new char[gcount() ];
			memcpy( m_pColNames, m_pvInput, gcount());
        }
		else
		{
			//Invalid Table, first line does not contain 
			//column metadata.
			return ResultFromScode( E_FAIL );
		}
        
		ParseColumnNames( m_pColNames );

        return ResultFromScode( S_FALSE );
        }

    ASSERT( pptstrName );

    // If the column number is in range then return
    // the pointer
    if ((0 == cCols) || (m_cColumns < cCols))
        return ResultFromScode( E_FAIL );
    else
        {
        *pptstrName = m_rgpColNames[cCols];
        return ResultFromScode( S_OK );
        }
}


//--------------------------------------------------------------------
// @mfunc Tokenize the column names
//
// @rdesc HRESULT
//      @flag S_OK | Parsing yielded no Error
//
HRESULT CFileIO::ParseColumnNames
    (
    LPTSTR ptstrInput
    )
{
    LPTSTR  pvInput = ptstrInput;

    ASSERT( pvInput );

    // Set first column pointer
    if ('\0' != *pvInput)
        {
        m_rgpColNames[++m_cColumns] = pvInput;
        }

    // Null Terminate each column
    while ('\0' != *pvInput)
        {
        // Check for Comma
        if (0 == strncmp( ",", pvInput, sizeof(char)))
            {
            memcpy( pvInput, "", sizeof(char));

            if (0 != strncmp( "", (pvInput+1), sizeof(char)))
                {
                m_rgpColNames[++m_cColumns] = (pvInput+1);
                }
            }

        pvInput++;
        }

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Retrieves the columns data characteristics
//
// @rdesc HRESULT
//      @flag S_OK | Succeeded
//      @flag E_FAIL | Invalid Column Number
//
HRESULT CFileIO::GetDataTypes
    (
    DBORDINAL cCols,    //@parm IN | Column number
    SWORD* pswType,     //@parm OUT | Data Type
    UDWORD* pudwColDef, //@parm OUT | Precision of the column
    BOOL* pfSigned      //@parm OUT | Is the columns signed
    )
{
    HRESULT hr;

    // Column number greater than MAX
    if (cCols > MAX_COLUMNS)
        return ResultFromScode( E_FAIL );

    // If Data Types have not been retrieved,
    // then retrieve them into the internal array
    if (0 == m_ulDataTypeOffset)
        {
        seekg( 0L );
        clear();

        // To retrieve the Column Data Types, we need
        // to skip the first row and get to the 2nd row
        getline( m_pvInput, MAX_INPUT_BUFFER );
        m_ulDataTypeOffset = tellg();
        getline( m_pvInput, MAX_INPUT_BUFFER );

        // Check Stream status
        if (bad() || 0 == gcount())
            return ResultFromScode( E_FAIL );

        // Parse the datatypes from the stream.
        hr = ParseDataTypes();
        if (FAILED( hr ))
            return hr;

        return ResultFromScode( S_FALSE );
        }

    assert( pswType || pudwColDef || pfSigned );

    // If the column number is in range then return
    // the pointer
    if ((0 == cCols) || (m_cColumns < cCols))
        return ResultFromScode( E_FAIL );
    else
        {
        *pswType = m_rgswColType[cCols];
        *pudwColDef = m_rgudwColSize[cCols];
        *pfSigned = m_rgfSigned[cCols];
        return ResultFromScode( S_OK );
        }
}


//--------------------------------------------------------------------
// @mfunc Tokenize the DataTypes and Lengths
// Valid Data Types are CHAR(n), INTEGER, and LONG
//
// CHAR, SLONG
//
// @rdesc HRESULT
//      @flag S_OK | Parsing yielded no Error
//
HRESULT CFileIO::ParseDataTypes()
{
    DBORDINAL cColumn = 0;
    LPSTR     pVal, pOpen;
	LPSTR	  pchNextToken;

    assert( m_pvInput );

    pVal = strtok_s( m_pvInput, ",\0", &pchNextToken);
    if (NULL == pVal)
        return ResultFromScode( E_FAIL );

    while (NULL != pVal)
        {
        cColumn++;

        if (0 == _strnicmp( pVal, CHAR_STRING, CHAR_STRING_SIZE ))
            {
            m_rgswColType[cColumn] = TYPE_CHAR;
            pOpen = strstr( pVal, "(" );
            m_rgudwColSize[cColumn] = atol( ++pOpen );
            m_rgfSigned[cColumn] = FALSE;
            }
        else if (0 == _strnicmp( pVal, SLONG_STRING, SLONG_STRING_SIZE ))
            {
            m_rgswColType[cColumn] = TYPE_SLONG;
            m_rgudwColSize[cColumn] = 4;
            m_rgfSigned[cColumn] = TRUE;
            }
        else
            return ResultFromScode( E_FAIL );

        pVal = strtok_s( NULL, ",\0", &pchNextToken);
        }

    // should have exactly the same number of types as we have columns
    if (cColumn != m_cColumns)
        return ResultFromScode( E_FAIL );

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Obtain the offsets into the file that each row exists at.
// Ignore any deleted rows while reading the file.
//
// @rdesc HRESULT
//      @flag S_OK | Got the offsets, Column Names and Data Types
//      @flag E_FAIL | Could not obtain all the necessary info
//
HRESULT CFileIO::GenerateFileInfo()
{
    ULONG ulDex = 0;
    ULONG_PTR ulSavePos;

    // Generate Column Info, if NULL is returned, a problem
    // was encountered while reading the Column Names.
    if (S_FALSE != GetColumnName( 0, NULL ))
        return ResultFromScode( E_FAIL );

    // Generate DataType Mapping, if FALSE is returned, a problem
    // was encountered while reading the DataTypes
    if (S_FALSE != GetDataTypes( 0, NULL, NULL, NULL ))
        return ResultFromScode( E_FAIL );

    // Create and Initialize the Index Array
    if (FALSE == m_FileIdx.fInit())
        return ResultFromScode( E_FAIL );

	// Cache essentail column metadata
	if (FAILED(GatherColumnInfo()))
		return ResultFromScode( E_FAIL );

    // Obtain the starting offset for each row
    seekg( m_ulDataTypeOffset );
    ulSavePos = tellg();
    getline( m_pvInput, MAX_INPUT_BUFFER );
    while (good() && !eof())
        {
        //Ignore Deleted Lines
        if ('@' != *m_pvInput && '\0' != *m_pvInput)
            m_FileIdx.SetIndex( ulDex++, ulSavePos );
        ulSavePos = tellg();
        getline( m_pvInput, MAX_INPUT_BUFFER );
        }

    // Store the number of rows
    m_cRows = ulDex - 1;
	clear();
    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Check if the row has already been deleted
//
// @rdesc HRESULT
//      @flag S_OK | Row already deleted
//      @flag S_FALSE | Row not deleted
//
HRESULT CFileIO::IsDeleted
    (
    DBCOUNTITEM ulRow                 //@parm IN | Row to Check
    )
{
    // Already deleted
    if (TRUE == m_FileIdx.IsDeleted( ulRow ))
        return ResultFromScode( S_OK );
    else
        return ResultFromScode( S_FALSE );
}


//--------------------------------------------------------------------
// @mfunc Fill the row with '@' characters, the deletion pattern..
// And set the Deletion status flag in the index class.
//
// @rdesc HRESULT
//      @flag S_OK | Deleted Row
//      @flag E_FAIL | Row Number was invalid or problem deleting.
//
HRESULT CFileIO::DeleteRow
    (
    DBCOUNTITEM ulRow                 //@parm IN | Row to Delete
    )
{
    assert( is_open());
    assert( m_pvInput );

    // Check the Row Number
    if ((ulRow < 1) || (ulRow > m_cRows))
        return ResultFromScode( E_FAIL );

    // If already deleted, just ignore.
    if (TRUE == m_FileIdx.IsDeleted( ulRow ))
        return ResultFromScode( S_OK );

    // Set the File Pointer
    seekg( m_FileIdx.GetRowOffset( ulRow ));
    clear();

    // Delete the row in the file and mark the status
    // as deleted in the index Array
    getline( m_pvInput, MAX_INPUT_BUFFER );
    if (good())
        {
        // Set the number bytes in the stream minus
        // the null terminator to this pattern
        memset( m_pvInput, '@', gcount() - 1 );
        seekp( m_FileIdx.GetRowOffset( ulRow ));
        clear();
        write( m_pvInput, gcount() - 1 );
        if (bad())
            return ResultFromScode( E_FAIL );
        else
            flush();
        }
    else
        return ResultFromScode( E_FAIL );

    m_FileIdx.DeleteRow( ulRow );

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Establish the Binding Information for the given file
//
// @rdesc HRESULT
//      @flag S_OK | Binding set
//      @flag E_FAIL | Problem setting the binding
//
HRESULT CFileIO::SetColumnBind
    (
    DBORDINAL   cCols,      //@parm IN | Column Number
    PCOLUMNDATA pColumn		//@parm IN | Pointer to the Data Area
    )
{
    assert( is_open());
    assert( m_rgpColumnData );

    // If the column number is in range then return
    // the pointer
    if ((0 == cCols) || (m_cColumns < cCols))
        return ResultFromScode( E_FAIL );

    // Expect valid pointer
    assert( pColumn );

    m_rgpColumnData[cCols] = pColumn;

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Fetch the row data from the stream to the internal data
// buffers
//
// @rdesc HRESULT
//      @flag S_OK    | Row Retrieve successfully
//      @flag S_FALSE | End of Result Set
//      @flag E_FAIL  | Row could not be retrieved
//
HRESULT CFileIO::Fetch
    (
    DBCOUNTITEM ulRow           //@parm IN | Row to retrieve
    )
{
    assert( is_open());
    assert( m_rgpColumnData );
    assert( m_rgsdwMaxLen );

    // Check the Row Number
    if ((ulRow < 1))
        return ResultFromScode( E_FAIL );

    //Check end of Result Set
    if (ulRow > m_cRows)
        return ResultFromScode( S_FALSE );

    // Set the File Pointer to the row.
    seekg( m_FileIdx.GetRowOffset( ulRow ));
    clear();

    // If already deleted, just ignore.
    if (TRUE == m_FileIdx.IsDeleted( ulRow ))
        return ResultFromScode( S_OK );

    // Retrieve the column names record
    getline( m_pvInput, MAX_INPUT_BUFFER );

    if (good() && 0 < gcount())
	{
        //Flag a Delete from another user
        if ( strncmp(m_pvInput, "@", sizeof(char)) == 0 )
		{
			DeleteRow( ulRow );
			return ResultFromScode( S_OK );
		}

		// Parse the row
        return ParseRowValues();
	}

    return ResultFromScode( E_FAIL );
}


//--------------------------------------------------------------------
// @mfunc Tokenize the Data values and put them into the correct
// binding areas
//
// @rdesc HRESULT
//      @flag S_OK | Parsing yielded no Error
//      @flag E_FAIL | Data value could not be parsed or stored
//
HRESULT CFileIO::ParseRowValues
    (
    void
    )
{
    DBORDINAL cColumns = 0;
    DWORD     cQuotes = 0;
    LPTSTR    pvCopy,
              pvInput,
              pLastQuote;

    pLastQuote = NULL;
    pvCopy = NULL;
    pvInput = m_pvInput;

    assert( pvInput );
    assert( m_cColumns > 0 );

    while ('\0' != *pvInput)
        {

        // Check for Quotes
        if (0 == strncmp( "\"", pvInput, sizeof( char )))
            {
            pLastQuote = pvInput;
            cQuotes++;
            goto TermCheck;
            }

        // Check for Comma
        // NOTE: THIS won't handle """
        if (0 == strncmp( ",", pvInput, sizeof( char )) &&
             0 == cQuotes % 2)
            {
            if (pLastQuote)
                memcpy( pLastQuote, "", sizeof( char ));
            else
                memcpy( pvInput, "", sizeof( char ));

            // Increment Columns processed
            cColumns++;

            //          TRACE(pvCopy ? pvCopy : "<NULL>");
            if (FAILED( FillBinding( cColumns, pvCopy )))
                return ResultFromScode( E_FAIL );

            pLastQuote = NULL;
            pvCopy = NULL;
            cQuotes = 0;
            goto TermCheck;
            }

        //Valid First character for next column
        if (NULL == pvCopy)
            pvCopy = pvInput;

        TermCheck:
        // Check for Final Null Terminator
        if (0 == strncmp( "", (pvInput+1), sizeof(char)))
            {
            //If we are to the null terminator and have unbalanced "'s
            //then we fail
            if (0 != cQuotes % 2)
                return ResultFromScode( E_FAIL );

            if (pLastQuote)
                memcpy( pLastQuote, "", sizeof(char));

            // Increment Columns processed
            cColumns++;

            //          TRACE(pvCopy ? pvCopy : "<NULL>");
            if (FAILED( FillBinding( cColumns, pvCopy )))
                return ResultFromScode( E_FAIL );
            }

        pvInput++;
        }

    // Check that we returned the correct number of columns
    if (cColumns < m_cColumns)
        return ResultFromScode( E_FAIL );

    return ResultFromScode( S_OK );
}

//--------------------------------------------------------------------
// @mfunc Based on the given bindings and column data, put the data
// in the correct area, update the status and length fields
//
// @rdesc HRESULT
//      @flag S_OK | Data copied to the specified location
//
HRESULT CFileIO::FillBinding
    (
    DBORDINAL cColumn, //@parm IN | Column that value is for
    LPTSTR    pvCopy   //@parm IN | Pointer to data value to transfer
    )
{
    assert( m_rgswColType );
    assert( m_rgpColumnData );
    assert( m_rgsdwMaxLen );

    // Null Value
    if (!pvCopy)
        {
        m_rgpColumnData[cColumn]->dwStatus = DBSTATUS_S_ISNULL;
        return ResultFromScode( S_OK );
        }

    switch (m_rgswColType[cColumn])
        {
    case TYPE_CHAR:
        lstrcpyn((LPTSTR) m_rgpColumnData[cColumn]->bData, pvCopy, m_rgsdwMaxLen[cColumn] + sizeof( char ) );
        m_rgpColumnData[cColumn]->uLength = lstrlen( (LPTSTR) m_rgpColumnData[cColumn]->bData );
        m_rgpColumnData[cColumn]->dwStatus = DBSTATUS_S_OK;
        break;

    case TYPE_SLONG:
        *(ULONG*) m_rgpColumnData[cColumn]->bData = atol( pvCopy );
        m_rgpColumnData[cColumn]->uLength = 4;
        m_rgpColumnData[cColumn]->dwStatus = DBSTATUS_S_OK;
        break;

    default:
        assert( !"Unknown Data Type" );
        break;
        }

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Given a pointer to the new data and the row the the data is
// for, write the data to the file.
//
// @rdesc HRESULT
//      @flag S_OK | Record and Indexes updated
//      @flag E_FAIL | Problems updating record
//
HRESULT CFileIO::UpdateRow
    (
    DBCOUNTITEM ulRow,      //@parm IN | Row to update
    BYTE*      pbProvRow,  //@parm IN | Data to update row with.
	UPDTYPE		eUpdateType	//@parm IN | What type of update
    )
{
    LPSTR       pvInput = m_pvInput;
	CHAR		szTmpBuff[MAX_BIND_LEN+1];
    PCOLUMNDATA pColData;
    DBORDINAL   cCols;
    size_t      nCnt;
    
	assert( is_open());
    assert( m_rgdwDataOffsets );

	// Fix up Row count
	if(	eUpdateType == INSERT )
		m_cRows++;

	// Delete old Row
	if( (eUpdateType == UPDATE) && 
		(FAILED( DeleteRow( ulRow ))) )
			return ResultFromScode( E_FAIL );

	// Fix up Row offset value
	seekg( 0, ios::end );
	m_FileIdx.SetIndex( ulRow, tellg() );

    // Check the Row Number
    if ((ulRow < 0) || (ulRow > m_cRows))
        return ResultFromScode( E_FAIL );

    // Updated Rows are added to the end of the file, the row number will
    // remain the same until the rowset is closed, because the old
    // offset is deleted and the new is put in it's place.
    for (cCols = 1; cCols <= m_cColumns; cCols++)
    {
        pColData = GetColumnData(cCols, (ROWBUFF *)pbProvRow);

        // Handle NULL Data
        if (pColData->dwStatus != DBSTATUS_S_ISNULL)
        {
            switch (m_rgswColType[cCols])
			{
            case TYPE_CHAR:
				StringCchCopyNA( szTmpBuff, sizeof(szTmpBuff), (LPSTR)pColData->bData, pColData->uLength );
				StringCchPrintfExA( pvInput, MAX_INPUT_BUFFER, &pvInput, &nCnt, 0, "\"%s\"", szTmpBuff );
                break;

            case TYPE_SLONG:
                StringCchPrintfExA( pvInput, MAX_INPUT_BUFFER, &pvInput, &nCnt, 0, "%d", (signed long) *pColData->bData );
                break;

            default:
                assert( !"Unknown Data Type" );
                break;

            }
        }

        //Calculate the next append area
        //pvInput = (pvInput + (nCnt * sizeof(char)));

        if (cCols == m_cColumns)
            StringCchCopyA( pvInput, nCnt, "\n" );
        else
        {
            StringCchCopyA( pvInput, nCnt, "," );
            pvInput += 1;
        }
    }

    // Write Stream to File
	// If Update change in place or InsertRow add to the end
	seekg( m_FileIdx.GetRowOffset( ulRow ) );

    clear();
    write( m_pvInput, lstrlen( m_pvInput ));
    if (bad())
        return ResultFromScode( E_FAIL );
    else
        flush();

    return ResultFromScode( S_OK );
}


//--------------------------------------------------------------------
// @mfunc Returns ::GetColumnsInfo data
//
// @rdesc HRESULT
//      @flag S_OK | All output populated correctly
//      @flag E_FAIL | Problems gathering Column Information
//
HRESULT CFileIO::GatherColumnInfo()
{
    HRESULT         hr;
    LPWSTR          lpwstr;
    int             cchFree, cchWide;
    DBCOLUMNINFO *  pcolinfo = NULL;
    DBLENGTH        dwOffset;
    DBORDINAL       cCols;
    int             cbName;
    SWORD           swCSVType;
    UDWORD          cbColDef;
    DWORD           dwdbtype;
	BOOL			fSigned;
	SDWORD *		rgcbLen = NULL;
	DBLENGTH *		rgdwOffsets = NULL;

	// Heap for column names.
    // Commit it all, then de-commit and release once we know size.
    m_pbHeap = (BYTE *) VirtualAlloc( NULL,
                                      MAX_HEAP_SIZE,
                                      MEM_RESERVE | MEM_COMMIT,
                                      PAGE_READWRITE );

	if( !m_pbHeap )
	{
		hr = ResultFromScode(E_OUTOFMEMORY);
		goto EXIT;
	}

    //----------------------------------
    // Gather column info
    //----------------------------------

    lpwstr  = (LPWSTR) m_pbHeap;
    cchFree = MAX_HEAP_SIZE / 2;

    pcolinfo	= &(m_rgdbcolinfo[1]);
	rgcbLen		= m_rgsdwMaxLen;
	rgdwOffsets = m_rgdwDataOffsets;

    dwOffset = offsetof( ROWBUFF, cdData );

    for (cCols=1; cCols <= m_cColumns; cCols++, pcolinfo++)
        {
        LPTSTR ptstrName;

        // Get Column Names and Lengths
        hr = GetColumnName(cCols, &ptstrName);
        if (FAILED( hr ))
			{
            hr = ResultFromScode( E_FAIL );
			goto EXIT;
			}

        // Store the Column Name in the Heap
        if (cchFree)
            {
            cbName = lstrlen( ptstrName );
            if (cbName)
                cchWide = MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, ptstrName, (int) cbName, lpwstr, cchFree );
            else
                cchWide = 0;
            pcolinfo->pwszName = lpwstr;
            lpwstr += cchWide;

            cchFree -= cchWide;
            if (cchFree)
                {
                *lpwstr++ = 0;
                cchFree--;
                }
            else
                *(lpwstr - 1) = 0;
            }
        else
            pcolinfo->pwszName = NULL;

        // Get DataTypes and Precision
        hr = GetDataTypes(cCols,
                        &swCSVType, // CSV data type
                        &cbColDef,  // Precision
                        &fSigned	// Signed or Unsigned
                        );
        if (FAILED( hr ))
            return ResultFromScode( E_FAIL );

        // We use ordinal numbers for our columns
        pcolinfo->columnid.eKind          = DBKIND_GUID_PROPID;
        pcolinfo->columnid.uGuid.guid     = GUID_NULL;
        pcolinfo->columnid.uName.ulPropid = (ULONG) cCols;

        // Determine the OLE DB type, for binding.
        hr = GetInternalTypeFromCSVType( swCSVType,					// in
                                         fSigned,					// in
                                         &dwdbtype );				// out, DBTYPE to show client
        if (FAILED( hr ))
			{
            hr = ResultFromScode( E_FAIL );
			goto EXIT;
			}

        // Check for overflow of size.
        rgcbLen[cCols] = cbColDef;
        rgcbLen[cCols] = MIN( rgcbLen[cCols], MAX_BIND_LEN );

        pcolinfo->iOrdinal		= cCols;
        pcolinfo->wType			= (DBTYPE) dwdbtype;
        pcolinfo->pTypeInfo     = NULL;
        pcolinfo->ulColumnSize  = rgcbLen[cCols];
		pcolinfo->bPrecision	= (BYTE) ~0;
		pcolinfo->bScale		= (BYTE) ~0;
        pcolinfo->dwFlags       = 0;

        // Is it a string datatype
		if(pcolinfo->wType != DBTYPE_STR)
			pcolinfo->bPrecision = (BYTE)cbColDef;

        // Is it a fixed length datatype
        if(pcolinfo->wType != DBTYPE_STR)
	        pcolinfo->dwFlags |= DBCOLUMNFLAGS_ISFIXEDLENGTH;

        // We do support nulls
        pcolinfo->dwFlags |= DBCOLUMNFLAGS_ISNULLABLE;
        pcolinfo->dwFlags |= DBCOLUMNFLAGS_MAYBENULL;

        //We should always be able to write to the column
        pcolinfo->dwFlags |= DBCOLUMNFLAGS_WRITE;

        // Set the offset from the start of the row,
        // for this column, then advance past.
        dwOffset = ROUND_UP( dwOffset, COLUMN_ALIGNVAL );
        rgdwOffsets[cCols] = dwOffset;
        dwOffset += offsetof( COLUMNDATA, bData ) + rgcbLen[cCols];
        }

    m_cbRowSize = ROUND_UP( dwOffset, COLUMN_ALIGNVAL );
    m_cbHeapUsed = MAX_HEAP_SIZE - 2*cchFree;

    // Decommit unused memory in our column-name buffer.
    // We know it will never grow beyond what it is now.
    // Decommit all pages past where we currently are.
    BYTE  *pDiscardPage;
    ptrdiff_t  ulSize;
    pDiscardPage = (BYTE *) ROUND_UP( lpwstr, g_dwPageSize );
    ulSize       = MAX_HEAP_SIZE - (pDiscardPage - m_pbHeap);
    if (ulSize > 0)
        VirtualFree( pDiscardPage, ulSize, MEM_DECOMMIT );
    assert( '\0' == (*lpwstr = '\0'));  // We shouldn't generate a mem fault.

EXIT:
	if( FAILED(hr) )
	{
		if( m_pbHeap )
		{
			VirtualFree((VOID *) m_pbHeap, 0, MEM_RELEASE );
		}
		m_cbRowSize = m_cbHeapUsed = 0;
		return hr;	
	}
	else
		return ResultFromScode( S_OK );
}

//--------------------------------------------------------------------
// @mfunc Given a ROWBUFF and a column ordinal, this function 
// returns a ptr to a particular column's COLUMNDATA
//
//
COLUMNDATA * CFileIO::GetColumnData
(
	DBORDINAL	cCols,
	ROWBUFF *	pRowBuff
)
{
    assert( is_open());
    assert( m_rgdwDataOffsets );

    // If the column number is in range then return
    // the pointer
    if ((0 == cCols) || (m_cColumns < cCols))
        return NULL;

    return (COLUMNDATA *)((BYTE *)pRowBuff + m_rgdwDataOffsets[cCols]);
}