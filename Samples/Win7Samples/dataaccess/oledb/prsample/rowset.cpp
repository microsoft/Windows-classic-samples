//---------------------------------------------------------------------------
// Microsoft OLE DB Programmer's Reference Sample
// Copyright (C) 1998 By Microsoft Corporation.
//	  
// @doc
//												  
// @module ROWSET.CPP
//			
//---------------------------------------------------------------------------
						  

/////////////////////////////////////////////////////////////////
// Includes					 
//
/////////////////////////////////////////////////////////////////
#include "prsample.h"		// Programmer's Reference Sample includes



/////////////////////////////////////////////////////////////////
// myCreateRowset
//
//	This function creates an OLE DB Rowset object from the given
//	provider's Session object. It first obtains a default table
//	name from the user through the tables schema rowset, if
//	supported, then creates a Rowset object by one of two methods:
//
//	- If the user requested that the Rowset object be created
//	  from a Command object, it creates a Command object, then
//	  obtains command text from the user, sets properties and
//	  the command text, and finally executes the command to
//	  create the Rowset object
//	- Otherwise, the function obtains a table name from the user
//	  and calls IOpenRowset::OpenRowset to create a Rowset object
//	  over that table that supports the requested properties
//
/////////////////////////////////////////////////////////////////
HRESULT	myCreateRowset
	(
	IUnknown *				pUnkSession,
	IUnknown **				ppUnkRowset
	)
{
	HRESULT					hr;
	IUnknown *				pUnkCommand						= NULL;
	IOpenRowset *			pIOpenRowset					= NULL;
	WCHAR					wszTableName[MAX_NAME_LEN + 1]	= {0};

	const ULONG				cProperties						= 2;
	DBPROP					rgProperties[cProperties];
	DBPROPSET				rgPropSets[1];

	// Obtain a default table name from the user by displaying the
	// tables schema rowset if schema rowsets are supported.
	CHECK_HR(hr = myCreateSchemaRowset(DBSCHEMA_TABLES, pUnkSession, 
		MAX_NAME_LEN, wszTableName));

	// Set properties on the rowset, to request additional functionality
	myAddRowsetProperties(rgPropSets, cProperties, rgProperties);

	// If the user requested that the rowset be created from a
	// Command object, create a Command, set its properties and
	// text and execute it to create the Rowset object
	if( g_dwFlags & USE_COMMAND )
	{
		WCHAR		wszCommandText[MAX_NAME_LEN + 1];

		// Attempt to create the Command object from the provider's
		// Session object. Note that Commands are not supported by
		// all providers, and this will fail in that case
		CHECK_HR(hr = myCreateCommand(pUnkSession, &pUnkCommand));
		
		// Get the command text that we will execute from the user
		if( !myGetInputFromUser(wszCommandText, _countof(wszCommandText), L"\nType the command "
			L"to execute [Enter = `select * from %s`]: ", wszTableName) )
		{
			swprintf(wszCommandText, _countof(wszCommandText), L"select * from %s", wszTableName);
		}

		// And execute the Command the user entered
		CHECK_HR(hr = myExecuteCommand(pUnkCommand, wszCommandText,
					1, rgPropSets, ppUnkRowset));
	}
	// Otherwise, the user gets the default behavior, which is to use
	// IOpenRowset to create the Rowset object from the Session object.
	// IOpenRowset is supported by all providers; it takes a TableID
	// and creates a rowset containing all rows in that table. It is
	// similar to using SQL command text of "select * from TableID"
	else
	{
		DBID				TableID;

		// Create the TableID
		TableID.eKind			= DBKIND_NAME;
		TableID.uName.pwszName	= wszTableName;

		// Obtain the table name from the user
		myGetInputFromUser(wszTableName, _countof(wszTableName),L"\nType the name of the table "
			L"to use [Enter = `%s`]: ", wszTableName);

		// Get the IOpenRowset interface and create a Rowset object
		// over the requested table through OpenRowset
		XCHECK_HR(hr = pUnkSession->QueryInterface(
					IID_IOpenRowset, (void**)&pIOpenRowset));
		XCHECK_HR(hr = pIOpenRowset->OpenRowset(
					NULL,			//pUnkOuter
					&TableID,		//pTableID
					NULL,			//pIndexID
					IID_IRowset,	//riid
					1,				//cPropSets
					rgPropSets,		//rgPropSets
					ppUnkRowset		//ppRowset
					));
	}

CLEANUP:
	if( pIOpenRowset )
		pIOpenRowset->Release();
	if( pUnkCommand )
		pUnkCommand->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////
// mySetupBindings
//
//	This function takes an IUnknown pointer from a Rowset object
//	and creates a bindings array that describes how we want the
//	data we fetch from the Rowset to be laid out in memory. It
//	also calculates the total size of a row so that we can use
//	this to allocate memory for the rows that we will fetch
//	later.
//
//	For each column in the Rowset, there will be a corresponding
//	element in the bindings array that describes how the
//	provider should transfer the data, including length and
//	status, for that column. This element also specifies the data
//	type that the provider should return the column as. We will
//	bind all columns as DBTYPE_WSTR, with a few exceptions
//	detailed below, as providers are required to support the
//	conversion of their column data to this type in the vast
//	majority of cases. The exception to our binding as
//	DBTYPE_WSTR is if the native column data type is
//	DBTYPE_IUNKNOWN or if the user has requested that BLOB
//	columns be bound as ISequentialStream objects, in which case
//	we will bind those columns as ISequentialStream objects.
//
/////////////////////////////////////////////////////////////////
HRESULT mySetupBindings
	(
	IUnknown *				pUnkRowset, 
	DBORDINAL *				pcBindings, 
	DBBINDING **			prgBindings, 
	DBORDINAL *				pcbRowSize
	)
{
	HRESULT					hr;
	DBORDINAL				cColumns;
	DBCOLUMNINFO *			rgColumnInfo				= NULL;
	LPWSTR					pStringBuffer				= NULL;
	IColumnsInfo *			pIColumnsInfo				= NULL;

	ULONG					iCol;
	DBORDINAL				dwOffset					= 0;
	DBBINDING *				rgBindings					= NULL;
	
	ULONG					cStorageObjs				= 0;
	BOOL					fMultipleObjs				= FALSE;

	// Obtain the column information for the Rowset; from this, we can find
	// out the following information that we need to construct the bindings
	// array:
	//  - the number of columns
	//  - the ordinal of each column
	//	- the precision and scale of numeric columns
	//  - the OLE DB data type of the column
	XCHECK_HR(hr = pUnkRowset->QueryInterface(
				IID_IColumnsInfo, (void**)&pIColumnsInfo));
	XCHECK_HR(hr = pIColumnsInfo->GetColumnInfo(
				&cColumns,		//pcColumns
				&rgColumnInfo,	//prgColumnInfo
				&pStringBuffer	//ppStringBuffer
				));

	// Allocate memory for the bindings array; there is a one-to-one
	// mapping between the columns returned from GetColumnInfo and our
	// bindings
	rgBindings = (DBBINDING*)CoTaskMemAlloc(cColumns * sizeof(DBBINDING));
	CHECK_MEMORY(hr, rgBindings);
	memset(rgBindings, 0, cColumns * sizeof(DBBINDING));

	// Determine if the Rowset supports multiple storage object bindings;
	// if it does not, we will only bind the first BLOB column or IUnknown
	// column as an ISequentialStream object, and will bind the rest as
	// DBTYPE_WSTR
	myGetProperty(pUnkRowset, IID_IRowset, DBPROP_MULTIPLESTORAGEOBJECTS, 
		DBPROPSET_ROWSET, &fMultipleObjs);

	// Construct the binding array element for each column
	for( iCol = 0; iCol < cColumns; iCol++ )
	{
		// This binding applies to the ordinal of this column
		rgBindings[iCol].iOrdinal	= rgColumnInfo[iCol].iOrdinal;

		// We are asking the provider to give us the data for this column
		// (DBPART_VALUE), the length of that data (DBPART_LENGTH), and
		// the status of the column (DBPART_STATUS)
		rgBindings[iCol].dwPart		= DBPART_VALUE|DBPART_LENGTH|DBPART_STATUS;

		// The following values are the offsets to the status, length, and
		// data value that the provider will fill with the appropriate values
		// when we fetch data later. When we fetch data, we will pass a
		// pointer to a buffer that the provider will copy column data to,
		// in accordance with the binding we have provided for that column;
		// these are offsets into that future buffer
		rgBindings[iCol].obStatus	= dwOffset;
		rgBindings[iCol].obLength	= dwOffset + sizeof(DBSTATUS);
		rgBindings[iCol].obValue	= dwOffset + sizeof(DBSTATUS) + sizeof(ULONG);
		
		// Any memory allocated for the data value will be owned by us, the
		// client. Note that no data will be allocated in this case, as the
		// DBTYPE_WSTR bindings we are using will tell the provider to simply
		// copy data directly into our provided buffer
		rgBindings[iCol].dwMemOwner	= DBMEMOWNER_CLIENTOWNED;

		// This is not a parameter binding
		rgBindings[iCol].eParamIO	= DBPARAMIO_NOTPARAM;
		
		// We want to use the precision and scale of the column
		rgBindings[iCol].bPrecision	= rgColumnInfo[iCol].bPrecision;
		rgBindings[iCol].bScale		= rgColumnInfo[iCol].bScale;

		// Bind this column as DBTYPE_WSTR, which tells the provider to
		// copy a Unicode string representation of the data into our buffer,
		// converting from the native type if necessary
		rgBindings[iCol].wType		= DBTYPE_WSTR;

		// Initially, we set the length for this data in our buffer to 0;
		// the correct value for this will be calculated directly below
		rgBindings[iCol].cbMaxLen	= 0;
						
		// Determine the maximum number of bytes required in our buffer to
		// contain the Unicode string representation of the provider's native
		// data type, including room for the NULL-termination character
		switch( rgColumnInfo[iCol].wType )
		{
			case DBTYPE_NULL:
			case DBTYPE_EMPTY:
			case DBTYPE_I1:
			case DBTYPE_I2:
			case DBTYPE_I4:
			case DBTYPE_UI1:
			case DBTYPE_UI2:
			case DBTYPE_UI4:
			case DBTYPE_R4:
			case DBTYPE_BOOL:
			case DBTYPE_I8:
			case DBTYPE_UI8:
			case DBTYPE_R8:
			case DBTYPE_CY:
			case DBTYPE_ERROR:
				// When the above types are converted to a string, they
				// will all fit into 25 characters, so use that plus space
				// for the NULL-terminator
				rgBindings[iCol].cbMaxLen = (25 + 1) * sizeof(WCHAR);
				break;

			case DBTYPE_DECIMAL:
			case DBTYPE_NUMERIC:
			case DBTYPE_DATE:
			case DBTYPE_DBDATE:
			case DBTYPE_DBTIMESTAMP:
			case DBTYPE_GUID:
				// Converted to a string, the above types will all fit into
				// 50 characters, so use that plus space for the terminator
				rgBindings[iCol].cbMaxLen = (50 + 1) * sizeof(WCHAR);
				break;
			
			case DBTYPE_BYTES:
				// In converting DBTYPE_BYTES to a string, each byte
				// becomes two characters (e.g. 0xFF -> "FF"), so we
				// will use double the maximum size of the column plus
				// include space for the NULL-terminator
				rgBindings[iCol].cbMaxLen =
					(rgColumnInfo[iCol].ulColumnSize * 2 + 1) * sizeof(WCHAR);
				break;

			case DBTYPE_STR:
			case DBTYPE_WSTR:
			case DBTYPE_BSTR:
				// Going from a string to our string representation,
				// we can just take the maximum size of the column,
				// a count of characters, and include space for the
				// terminator, which is not included in the column size
				rgBindings[iCol].cbMaxLen = 
					(rgColumnInfo[iCol].ulColumnSize + 1) * sizeof(WCHAR);
				break;

			default:
				// For any other type, we will simply use our maximum
				// column buffer size, since the display size of these
				// columns may be variable (e.g. DBTYPE_VARIANT) or
				// unknown (e.g. provider-specific types)
				rgBindings[iCol].cbMaxLen = MAX_COL_SIZE;
				break;
		};
		
		// If the provider's native data type for this column is
		// DBTYPE_IUNKNOWN or this is a BLOB column and the user
		// has requested that we bind BLOB columns as ISequentialStream
		// objects, bind this column as an ISequentialStream object if
		// the provider supports our creating another ISequentialStream
		// binding
		if( (rgColumnInfo[iCol].wType == DBTYPE_IUNKNOWN ||
			 ((rgColumnInfo[iCol].dwFlags & DBCOLUMNFLAGS_ISLONG) &&
			  (g_dwFlags & USE_ISEQSTREAM))) &&
			(fMultipleObjs || !cStorageObjs) )
		{
			// To create an ISequentialStream object, we will
			// bind this column as DBTYPE_IUNKNOWN to indicate
			// that we are requesting this column as an object
			rgBindings[iCol].wType				= DBTYPE_IUNKNOWN;

			// We want to allocate enough space in our buffer for
			// the ISequentialStream pointer we will obtain from
			// the provider
			rgBindings[iCol].cbMaxLen			= sizeof(ISequentialStream *);

			// To specify the type of object that we want from the
			// provider, we need to create a DBOBJECT structure and
			// place it in our binding for this column
		    rgBindings[iCol].pObject			= 
								(DBOBJECT *)CoTaskMemAlloc(sizeof(DBOBJECT));
			CHECK_MEMORY(hr, rgBindings[iCol].pObject);

			// Direct the provider to create an ISequentialStream
			// object over the data for this column
			rgBindings[iCol].pObject->iid		= IID_ISequentialStream;

			// We want read access on the ISequentialStream
			// object that the provider will create for us
			rgBindings[iCol].pObject->dwFlags	= STGM_READ;

			// Keep track of the number of storage objects (ISequentialStream
			// is a storage interface) that we have requested, so that we
			// can avoid requesting multiple storage objects from a provider
			// that only supports a single storage object in our bindings
			cStorageObjs++;
		}	

		// Ensure that the bound maximum length is no more than the
		// maximum column size in bytes that we've defined
		rgBindings[iCol].cbMaxLen	
			= min(rgBindings[iCol].cbMaxLen, MAX_COL_SIZE);

		// Update the offset past the end of this column's data, so
		// that the next column will begin in the correct place in
		// the buffer
		dwOffset = rgBindings[iCol].cbMaxLen + rgBindings[iCol].obValue;
		
		// Ensure that the data for the next column will be correctly
		// aligned for all platforms, or, if we're done with columns,
		// that if we allocate space for multiple rows that the data
		// for every row is correctly aligned
		dwOffset = ROUNDUP(dwOffset);
	}

	// Return the row size (the current dwOffset is the size of the row),
	// the count of bindings, and the bindings array to the caller
	*pcbRowSize		= dwOffset;
	*pcBindings		= cColumns;
	*prgBindings	= rgBindings;

CLEANUP:
	CoTaskMemFree(rgColumnInfo);
	CoTaskMemFree(pStringBuffer);
	if( pIColumnsInfo )
		pIColumnsInfo->Release();
	return hr;
}



/////////////////////////////////////////////////////////////////
// myCreateAccessor
//
//	This function takes an IUnknown pointer for a Rowset object
//	and creates an Accessor that describes the layout of the
//	buffer we will use when we fetch data. The provider will fill
//	this buffer according to the description contained in the
//	Accessor that we will create here.
//
/////////////////////////////////////////////////////////////////
HRESULT myCreateAccessor
	(
	IUnknown *				pUnkRowset, 
	HACCESSOR *				phAccessor, 
	DBORDINAL *				pcBindings, 
	DBBINDING **			prgBindings, 
	DBORDINAL *				pcbRowSize
	)
{
	HRESULT					hr;
	IAccessor *				pIAccessor					= NULL;

	// An Accessor is basically a handle to a collection of bindings.
	// To create the Accessor, we need to first create an array of
	// bindings for the columns in the Rowset
	CHECK_HR(hr = mySetupBindings(pUnkRowset, pcBindings, prgBindings, 
		pcbRowSize));
	
	// Now that we have an array of bindings, tell the provider to
	// create the Accessor for those bindings. We get back a handle
	// to this Accessor, which we will use when fetching data
	XCHECK_HR(hr = pUnkRowset->QueryInterface(
				IID_IAccessor, (void**)&pIAccessor));
	XCHECK_HR(hr = pIAccessor->CreateAccessor(
				DBACCESSOR_ROWDATA,	//dwAccessorFlags
				*pcBindings,		//cBindings
				*prgBindings,		//rgBindings
				0,					//cbRowSize
				phAccessor,			//phAccessor	
				NULL				//rgStatus
				));

CLEANUP:
	if( pIAccessor )
		pIAccessor->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////
// myDisplayRowset
//
//	This function will display data from a Rowset object and will
//	allow the user to perform basic navigation of the rowset.
//
//	The function takes a pointer to a Rowset object's IUnknown
//	and, optionally, the name of a column and a buffer that will
//	receive the value of that column when the user selects a row.
//
/////////////////////////////////////////////////////////////////
HRESULT myDisplayRowset
	(
	IUnknown *				pUnkRowset,
	LPCWSTR					pwszColToReturn,
	ULONG					cchBuffer,
	LPWSTR					pwszBuffer
	)
{
	HRESULT					hr;
	IRowset *				pIRowset					= NULL;
	DBORDINAL				cBindings;
	DBBINDING *				rgBindings					= NULL;	
	HACCESSOR				hAccessor					= DB_NULL_HACCESSOR;
	DBORDINAL				cbRowSize;
	void *					pData						= NULL;
	ULONG *					rgDispSize					= NULL;
	DBCOUNTITEM				cRowsObtained;
	HROW *					rghRows						= NULL;
	ULONG					iRow;
	LONG					cRows						= MAX_ROWS;
	LONG					iRetCol						= -1;
	BOOL					fCanFetchBackwards;
	DBORDINAL				iIndex;
	void *					pCurData;

	// Obtain the IRowset interface for use in fetching rows and data
	XCHECK_HR(hr = pUnkRowset->QueryInterface(
				IID_IRowset, (void**)&pIRowset));

	// Determine whether this rowset supports fetching data backwards;
	// we use this to determine whether the rowset can support moving
	// to the previous set of rows, described in more detail below
	myGetProperty(pUnkRowset, IID_IRowset, DBPROP_CANFETCHBACKWARDS, 
		DBPROPSET_ROWSET, &fCanFetchBackwards);
	
	// If the caller wants us to return the data for a particular column
	// from a user-selected row, we need to turn the column name into a
	// column ordinal
	if( pwszColToReturn )
		CHECK_HR(hr = myFindColumn(pUnkRowset, pwszColToReturn, &iRetCol));

	// Create an Accessor. An Accessor is basically a handle to a
	// collection of bindings that describes to the provider how to
	// copy (and convert, if necessary) column data into our buffer.
	// The Accessor that this creates will bind all columns as either
	// DBTYPE_WSTR (a Unicode string) or as an ISequentialStream object
	// (used for BLOB data). This will also give us the size of the
	// row buffer that the Accessor describes to the provider
	CHECK_HR(hr = myCreateAccessor(pUnkRowset, &hAccessor, 
										&cBindings, &rgBindings, &cbRowSize));

	// Allocate enough memory to hold cRows rows of data; this is
	// where the actual row data from the provider will be placed
	pData = CoTaskMemAlloc(cbRowSize * MAX_ROWS);
	CHECK_MEMORY(hr, pData);

	// Allocate memory for an array that we will use to calculate the
	// maximum display size used by each column in the current set of
	// rows
	rgDispSize = (ULONG *)CoTaskMemAlloc(cBindings * sizeof(ULONG));
	CHECK_MEMORY(hr, rgDispSize);

	// In this loop, we perform the following process:
	//  - reset the maximum display size array
	//  - try to get cRows row handles from the provider
	//  - these handles are then used to actually get the row data from the
	//    provider copied into our allocated buffer
	//  - calculate the maximum display size for each column
	//  - release the row handles to the rows we obtained
	//  - display the column names for the rowset
	//  - display the row data for the rows that we fetched
	//  - get user input
	//  - free the provider-allocated row handle array
	//  - repeat unless the user has chosen to quit or has selected a row
	while( hr == S_OK )
	{
		// Clear the maximum display size array
		memset(rgDispSize, 0, cBindings * sizeof(ULONG));

		// Attempt to get cRows row handles from the provider
		XCHECK_HR(hr = pIRowset->GetNextRows(
					DB_NULL_HCHAPTER,	//hChapter
					0,					//lOffset
					cRows,				//cRows
					&cRowsObtained,		//pcRowsObtained
					&rghRows			//prghRows
					));			

		// Loop over the row handles obtained from GetNextRows,
		// actually fetching the data for these rows into our buffer
		for( iRow = 0; iRow < cRowsObtained; iRow++ )
		{
			// Find the location in our buffer where we want to place
			// the data for this row. Note that if we fetched rows
			// backwards (cRows < 0), the row handles obtained from the
			// provider are reversed from the order in which we want to
			// actually display the data on the screen, so we will
			// account for this. This ensures that the resulting order
			// of row data in the pData buffer matches the order we
			// wish to use to display the data
			iIndex		= cRows > 0 ? iRow : cRowsObtained - iRow - 1;
			pCurData	= (BYTE*)pData + (cbRowSize * iIndex);
			
			// Get the data for this row handle. The provider will copy
			// (and convert, if necessary) the data for each of the
			// columns that are described in our Accessor into the given
			// buffer (pCurData)
			XCHECK_HR(hr = pIRowset->GetData(
					rghRows[iRow],	//hRow 
					hAccessor,		//hAccessor
					pCurData		//pData
					));

			// Update the maximum display size array, accounting for this row
			CHECK_HR(hr = myUpdateDisplaySize(cBindings, rgBindings, 
				pCurData, rgDispSize));
		}

		// If we obtained rows, release the row handles for the retrieved rows
		// and display the names of the rowset columns before we display the data
		if( cRowsObtained )
		{
			// Release the row handles that we obtained
			XCHECK_HR(hr = pIRowset->ReleaseRows(
						cRowsObtained,	//cRows
						rghRows,		//rghRows
						NULL,			//rgRowOptions
						NULL,			//rgRefCounts
						NULL			//rgRowStatus
						));

			
			// Display the names of the rowset columns
			CHECK_HR(hr = myDisplayColumnNames(pIRowset, rgDispSize));
		}
		
		// For each row that we obtained the data for, display this data
		for( iRow = 0; iRow < cRowsObtained; iRow++ )
		{
			// Get a pointer to the data for this row
			pCurData = (BYTE*)pData + (cbRowSize* iRow);

			// And display the row data
			CHECK_HR(hr = myDisplayRow(iRow, cBindings, rgBindings,
				pCurData, rgDispSize));
		}

		// Allow the user to navigate the rowset. This displays the appropriate
		// prompts, gets the user's input, may call IRowset::RestartPosition,
		// and may copy data from a selected row to the selection buffer, if so
		// directed. This will return S_OK if the user asked for more rows,
		// S_FALSE if the user selected a row, or E_FAIL if the user quits
		hr = myInteractWithRowset(
			pIRowset,					// IRowset pointer, for RestartPosition
			&cRows,						// updated with fetch direction value
			cRowsObtained,				// to indicate selection range
			fCanFetchBackwards,			// whether [P]revious is supported
			pData,						// data pointer for copying selection
			cbRowSize,					// size of rows for copying selection
			iRetCol >= 0 ?				// bindings for the selection column,
				&rgBindings[iRetCol] :	//  or NULL if no selection column
				NULL,
			cchBuffer,					// size of the selection buffer
			pwszBuffer);				// pointer to the selection buffer

		// Since we are allowing the provider to allocate the memory for 
		// the row handle array, we will free this memory and reset the 
		// pointer to NULL. If this is not NULL on the next call to GetNextRows,
		// the provider will assume that it points to an allocated array of
		// the required size (which may not be the case if we obtained less
		// than cRows rows from this last call to GetNextRows
		CoTaskMemFree(rghRows);
		rghRows = NULL;
	}

CLEANUP:
	myFreeBindings(cBindings, rgBindings);
	CoTaskMemFree(rgDispSize);
	CoTaskMemFree(pData);
	if( pIRowset )
		pIRowset->Release();
	return hr;
}



/////////////////////////////////////////////////////////////////
// myInteractWithRowset
//
//	This function allows the user to interact with the rowset. It
//	prompts the user appropriately, gets the user's input, may
//	call IRowset::RestartPosition if the user requests a restart,
//	and will copy data from a selected row to the selection
//	buffer.
//
/////////////////////////////////////////////////////////////////
HRESULT myInteractWithRowset
	(
	IRowset *				pIRowset,
	LONG *					pcRows,
	DBCOUNTITEM				cRowsObtained,
	BOOL					fCanFetchBackwards,
	void *					pData,
	DBORDINAL				cbRowSize,
	DBBINDING *				pBinding,
	ULONG					cchBuffer,
	LPWSTR					pwszBuffer
	)
{
	HRESULT					hr							= S_OK;
	CHAR					ch;

	// Let the user know if no rows were fetched
	if( !cRowsObtained )
		printf("\n*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n" \
				 "*                                 *\n" \
				 "* No rows obtained on this fetch! *\n" \
				 "*                                 *\n" \
				 "*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*\n");

	// Print navigation options
	if( fCanFetchBackwards )
		printf("\n[P]revious; [N]ext; [R]estart; ");
	else
		printf("\n[N]ext; [R]estart; ");
	
	// Print selection options
	if( cRowsObtained && pwszBuffer && pBinding )
		printf("[0]-[%d] for a row; ", cRowsObtained - 1);

	// User can always quit the program
	printf("[Q]uit? ");

	// Get the user's input
	while( TRUE )
	{
		// Get a character from the console
		ch = myGetChar();

		// Look for one of the allowed options; if not found, go
		// back around and wait for another input from the user

		// If we're looking for a row selection, allow the user to select
		// a row that we fetched, then copy the data from the requested
		// column into the selection buffer we were passed
		if( pwszBuffer && pBinding &&
			ch >= '0' && ch < (int)('0' + cRowsObtained) )
		{
			// Save the data for the selected row
			ULONG nSelection = ch - '0';
			//pwszBuffer is of size cchBuffer+1
			_snwprintf_s(pwszBuffer, cchBuffer+1, cchBuffer, L"%s", 
				  (WCHAR *)((BYTE *)pData + cbRowSize * nSelection + 
				  pBinding->obValue));
			pwszBuffer[cchBuffer] = L'\0';
			hr = S_FALSE;
		}
		// If the provider supports fetching backwards, set *pcRows
		// to -MAX_ROWS. When GetNextRows is called with this value,
		// it will fetch rows backwards from the current position
		// until it fetches MAX_ROWS rows or hits the end of the rowset
		else if( fCanFetchBackwards && ch == 'p' )
		{
			// Fetch backwards
			*pcRows = -MAX_ROWS;
		}
		// Set *pcRows so that the next call to GetNextRows fetches
		// MAX_ROWS rows forward from the current position
		else if( ch == 'n' )
		{
			// Fetch forward
			*pcRows = MAX_ROWS;
		}
		// Call IRowset::RestartPosition and fetch the first MAX_ROWS
		// rows of the rowset forward from there
		else if( ch == 'r' )
		{
			// RestartPosition
			*pcRows = MAX_ROWS;
			XCHECK_HR(hr = pIRowset->RestartPosition(DB_NULL_HCHAPTER));
			
			// Restarting a command may return the DB_S_COMMANDREEXECUTED
			// warning. If this happens, we still want the caller to
			// continue to display data, so we will reset the result code
			// to S_OK
			hr = S_OK;
		}
		// Quit the program
		else if( ch == 'q' )
		{
			hr = E_FAIL;
		}
		// Invalid option; go back up and get another character from the user
		else
		{
			continue;
		}

		// Echo the character and stop waiting for input
		printf("%c\n", ch);
		break;
	}

CLEANUP:
	return hr;
}

		
/////////////////////////////////////////////////////////////////
// myDisplayColumnNames
//
//	This function takes an IUnknown pointer to a Rowset object
//	and displays the names of the columns of that Rowset.
//
/////////////////////////////////////////////////////////////////
HRESULT myDisplayColumnNames
	(
	IUnknown *				pUnkRowset,
	ULONG *					rgDispSize
	)
{
	HRESULT					hr;
	IColumnsInfo *			pIColumnsInfo				= NULL;
	DBORDINAL					cColumns;
	DBCOLUMNINFO *			rgColumnInfo				= NULL;
	LPOLESTR				pStringsBuffer				= NULL;
	WCHAR					wszColumn[MAX_DISPLAY_SIZE + 1];
	LPWSTR					pwszColName;
	ULONG					iCol;
	size_t					cSpaces;
	ULONG					iSpace;

	// Get the IColumnsInfo interface for the Rowset
	XCHECK_HR(hr = pUnkRowset->QueryInterface(
				IID_IColumnsInfo, (void**)&pIColumnsInfo));

	// Get the columns information
	XCHECK_HR(hr = pIColumnsInfo->GetColumnInfo(
				&cColumns,		//pcColumns
				&rgColumnInfo,	//prgColumnInfo
				&pStringsBuffer	//ppStringBuffer
				));

	// Display the title of the row index column
	wprintf(L" Row | ");

	// Display all column names
	for( iCol = 0; iCol < cColumns; iCol++ )
	{
		pwszColName = rgColumnInfo[iCol].pwszName;
		
		// If the column name is NULL, we'll use a default string
		if( !pwszColName )
		{
			// Is this the bookmark column?
			if( !rgColumnInfo[iCol].iOrdinal )
				pwszColName = L"Bmk";
			else
				pwszColName = L"(null)";
		}

		// Ensure that the name is no longer than MAX_DISPLAY_SIZE
		wcsncpy_s(wszColumn, _countof(wszColumn), pwszColName, MAX_DISPLAY_SIZE);
		wszColumn[min(rgDispSize[iCol], MAX_DISPLAY_SIZE)] = L'\0';

		// Figure out how many spaces we need to print after this column name
		cSpaces = min(rgDispSize[iCol], MAX_DISPLAY_SIZE) - wcslen(wszColumn);

		// Print the column name
		wprintf(L"%s", wszColumn);

		// Now print any spaces necessary to align this column
		for(iSpace = 0; iSpace < cSpaces; iSpace++ )
			_putch(' ');

		// Now end the column with a separator marker if necessary
		if( iCol < cColumns - 1 )
			wprintf(L" | ");
	}

	// Done with the header, so print a newline
	wprintf(L"\n");

CLEANUP:
	CoTaskMemFree(rgColumnInfo);
	CoTaskMemFree(pStringsBuffer);
	if( pIColumnsInfo )
		pIColumnsInfo->Release();
	return hr;
}


/////////////////////////////////////////////////////////////////
// myDisplayRow
//
//	This function displays the data for a row.
//
/////////////////////////////////////////////////////////////////
HRESULT myDisplayRow
	(
	ULONG					iRow, 
	DBORDINAL				cBindings, 
	DBBINDING *				rgBindings, 
	void *					pData, 
	ULONG *					rgDispSize
	)
{
	HRESULT					hr							= S_OK;
	WCHAR					wszColumn[MAX_DISPLAY_SIZE + 1];
	DBSTATUS				dwStatus;
	ULONG					ulLength;
	void *					pvValue;
	ULONG					iCol;
	ULONG					cbRead;
	ISequentialStream *		pISeqStream					= NULL;
	size_t					cSpaces;
	ULONG					iSpace;
	
	// Display the row number
	wprintf(L" [%d] | ", iRow);

	// For each column that we have bound, display the data
	for( iCol = 0; iCol < cBindings; iCol++ )
	{
		// We have bound status, length, and the data value for all
		// columns, so we know that these can all be used
		dwStatus	= *(DBSTATUS *)((BYTE *)pData + rgBindings[iCol].obStatus);
		ulLength	= *(ULONG *)((BYTE *)pData + rgBindings[iCol].obLength);
		pvValue		= (BYTE *)pData + rgBindings[iCol].obValue;

		// Check the status of this column. This decides
		// exactly what will be displayed for the column
		switch( dwStatus )
		{
			// The data is NULL, so don't try to display it
			case DBSTATUS_S_ISNULL:
				wcscpy_s(wszColumn, _countof(wszColumn), L"(null)");
				break;

			// The data was fetched, but may have been truncated.
			// Display string data for this column to the user
			case DBSTATUS_S_TRUNCATED:
			case DBSTATUS_S_OK:
			case DBSTATUS_S_DEFAULT:
			{
				// We have either bound the column as a Unicode string
				// (DBTYPE_WSTR) or as an ISequentialStream object
				// (DBTYPE_IUNKNOWN), and have to do different processing
				// for each one of these possibilities
				switch( rgBindings[iCol].wType )
				{
					case DBTYPE_WSTR:
					{	
						// Copy the string data
						wcsncpy_s(wszColumn, _countof(wszColumn),(WCHAR *)pvValue, MAX_DISPLAY_SIZE);
						wszColumn[MAX_DISPLAY_SIZE - 1] = L'\0';
						break;
					}

					case DBTYPE_IUNKNOWN:
					{
						// We've bound this as an ISequentialStream object,
						// therefore the data in our buffer is a pointer
						// to the object's ISequentialStream interface
						pISeqStream =  *(ISequentialStream**)pvValue;
						
						// We call ISequentialStream::Read to read bytes from
						// the stream blindly into our buffer, simply as a
						// demonstration of ISequentialStream. To display the
						// data properly, the native provider type of this
						// column should be accounted for; it could be
						// DBTYPE_WSTR, in which case this works, or it could
						// be DBTYPE_STR or DBTYPE_BYTES, in which case this
						// won't display the data correctly
						CHECK_HR(hr = pISeqStream->Read(
									wszColumn,			//pBuffer
									MAX_DISPLAY_SIZE,	//cBytes
									&cbRead				//pcBytesRead
									));
						
						// Since streams don't provide NULL-termination,
						// we'll NULL-terminate the resulting string ourselves
						wszColumn[cbRead / sizeof(WCHAR)] = L'\0';

						// Release the stream object, now that we're done
						pISeqStream->Release();
						pISeqStream = NULL;
						break;
					}
				}
				break;
			}

			// This is an error status, so don't try to display the data
			default:
				wcscpy_s(wszColumn, _countof(wszColumn), L"(error status)");
				break;
		}

		// Determine how many spaces we need to add after displaying this
		// data to align it with this column in other rows
		cSpaces = min(rgDispSize[iCol], MAX_DISPLAY_SIZE) - wcslen(wszColumn);

		// Print the column data
		wprintf(L"%s", wszColumn);

		// Now print any spaces necessary
		for(iSpace = 0; iSpace < cSpaces; iSpace++ )
			_putch(' ');

		// Now end the column with a separator marker if necessary
		if( iCol < cBindings - 1 )
			wprintf(L" | ");
	}
	
CLEANUP:
	if( pISeqStream )
		pISeqStream->Release();

	// Print the row separator
	wprintf(L"\n");
	return hr;
}



/////////////////////////////////////////////////////////////////
// myFreeBindings
//
//	This function frees a bindings array and any allocated
//	structures contained in that array.
//
/////////////////////////////////////////////////////////////////
void myFreeBindings
	(
	DBORDINAL				cBindings, 
	DBBINDING *				rgBindings
	)
{
	ULONG					iBind;

	// Free any memory used by DBOBJECT structures in the array
	for( iBind = 0; iBind < cBindings; iBind++ )
		CoTaskMemFree(rgBindings[iBind].pObject);

	// Now free the bindings array itself
	CoTaskMemFree(rgBindings);
}


/////////////////////////////////////////////////////////////////
// myAddRowsetProperties
//
//	This function sets up the given DBPROPSET and DBPROP
//	structures, adding two optional properties that describe
//	features that we would like to use on the Rowset created
//	with these properties applied:
//	 - DBPROP_CANFETCHBACKWARDS -- the rowset should support
//	   fetching rows backwards from our current cursor position
//	 - DBPROP_IRowsetLocate     -- the rowset should support
//	   the IRowsetLocate interface and its semantics
//
/////////////////////////////////////////////////////////////////
void myAddRowsetProperties(DBPROPSET* pPropSet, ULONG cProperties, DBPROP* rgProperties)
{
	// Initialize the property set array
	pPropSet->rgProperties		= rgProperties;
	pPropSet->cProperties		= cProperties;
	pPropSet->guidPropertySet	= DBPROPSET_ROWSET;

	// Add the following two properties (as OPTIONAL) to the property
	// array contained in the property set array in order to request
	// that they be supported by the rowset we will create. Because
	// these are optional, the rowset we obtain may or may not support
	// this functionality. We will check for the functionality that
	// we need once the rowset is created and will modify our behavior
	// appropriately
	myAddProperty(&rgProperties[0], DBPROP_CANFETCHBACKWARDS);
	myAddProperty(&rgProperties[1], DBPROP_IRowsetLocate);
}


/////////////////////////////////////////////////////////////////
// myUpdateDisplaySize
//
//	This function updates the rgDispSize array, keeping the
//	maximum of the display size needed for the given data and
//	the previous maximum size already in the array.
//
/////////////////////////////////////////////////////////////////
HRESULT myUpdateDisplaySize
	(
	DBORDINAL				cBindings, 
	DBBINDING *				rgBindings, 
	void *					pData, 
	ULONG *					rgDispSize
	)
{
	DBSTATUS				dwStatus;
	ULONG					cchLength;
	ULONG					iCol;

	// Loop through the bindings, comparing the size of each column
	// against the previously found maximum size for that column
	for( iCol = 0; iCol < cBindings; iCol++ )
	{
		dwStatus  = *(DBSTATUS *)((BYTE *)pData + rgBindings[iCol].obStatus);
		cchLength = ((*(ULONG *)((BYTE *)pData + rgBindings[iCol].obLength))
					/ sizeof(WCHAR));

		// The length that we need to display depends on the status
		// of this column and generally on the data in the column
		switch( dwStatus )
		{
			case DBSTATUS_S_ISNULL:
				cchLength = 6;						// "(null)"
				break;

			case DBSTATUS_S_TRUNCATED:
			case DBSTATUS_S_OK:
			case DBSTATUS_S_DEFAULT:
				if( rgBindings[iCol].wType == DBTYPE_IUNKNOWN )
					cchLength = 2 + 8;				// "0x%08lx"
				
				// Ensure that the length is at least the minimum display size
				cchLength = max(cchLength, MIN_DISPLAY_SIZE);
				break;

			default:
				cchLength = 14;						// "(error status)"
				break;
		}

		if( rgDispSize[iCol] < cchLength )
			rgDispSize[iCol] = cchLength;
	}

	return S_OK;
}


/////////////////////////////////////////////////////////////////
// myFindColumn
//
//	Find the index of the column described in pwszName and return
//	S_OK, or, if not found, S_FALSE.
//
/////////////////////////////////////////////////////////////////
HRESULT myFindColumn
	(
	IUnknown *				pUnkRowset,
	LPCWSTR					pwszName,
	LONG *					plIndex
	)
{
	HRESULT					hr;
	IColumnsInfo *			pIColumnsInfo				= NULL;
	DBORDINAL				cColumns;
	DBCOLUMNINFO *			rgColumnInfo				= NULL;
	OLECHAR *				pStringsBuffer				= NULL;
	ULONG					iCol;

	// Get the IColumnsInfo interface
	XCHECK_HR(hr = pUnkRowset->QueryInterface(
				IID_IColumnsInfo, (void**)&pIColumnsInfo));

	// Get the columns information
	XCHECK_HR(hr = pIColumnsInfo->GetColumnInfo(
				&cColumns,		//pcColumns
				&rgColumnInfo,	//prgColumnInfo
				&pStringsBuffer	//ppStringBuffer
				));

	// Assume that we'll find the column
	hr = S_OK;

	// Search for the column we need
	for( iCol = 0; iCol < cColumns; iCol++ )
	{
		// If the column name matches we've found the column...
		if( rgColumnInfo[iCol].pwszName && 
			!wcscmp(pwszName, rgColumnInfo[iCol].pwszName) )
		{
			*plIndex = iCol;
			goto CLEANUP;
		}
	}

	// If we didn't find the column, we'll return S_FALSE
	hr = S_FALSE;

CLEANUP:
	CoTaskMemFree(rgColumnInfo);
	CoTaskMemFree(pStringsBuffer);
	if( pIColumnsInfo )
		pIColumnsInfo->Release();
	return hr;
}


