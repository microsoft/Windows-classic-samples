#include "stdafx.h"
#include "CDBFile.h"

// The Constructor
CDBFile::~CDBFile()
{
	size_t nSize = m_Rows.GetCount();
	for (size_t n = 0; n < nSize; n++)
		delete m_Rows[n];
	// Close the file
	if(!m_bClosed)
		Close();
	// Delete the m_prgColInfo
	if (NULL != m_prgColInfo) 
	{
		delete [] m_prgColInfo;
		m_prgColInfo = NULL;
	}
}


bool CDBFile::Open(LPCTSTR szFileName, bool bOpenMode)  // 0 = Open Exclusive mode
{
	TCHAR pszFileName[_MAX_PATH];
    _tcscpy_s(pszFileName, _countof(pszFileName), szFileName);
	// Open schema .sxt file
	if (!GetFileSchemaInfo(pszFileName))
	{
		// An error occured opening schema file
		OUT_LINE_FILE();
		return false;
	}
	
	// Get the Data file from the schema file
	pszFileName[_tcslen(szFileName) - 3] = 't';

	if(true == bOpenMode)
	{
		m_hFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, 0 /* open file for exclusive use*/, 
		       NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	}
	else
	{
		m_hFile = CreateFile(pszFileName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ /* open file for non-exclusive use*/, 
		       NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, NULL);
	}

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		// An error occured opening the file
		OUT_LINE_FILE();
		return false;
	}

	m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (m_hMapFile == NULL)
	{
		OUT_LINE_FILE();
		return false;
	}

	// Create just one view of the memory-mapped file
	m_pFileMap = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_pFileMap == NULL)
	{
		OUT_LINE_FILE();
		return false;
	}

	m_pbEOF = (char *) m_pFileMap + GetFileSize(m_hFile, NULL) -1;
	
	m_bClosed = false;

	return true;
}

// Close the File 

bool CDBFile::Close()
{
	UnmapViewOfFile(m_pFileMap);

	CloseHandle(m_hMapFile);
	
	CloseHandle(m_hFile);

	m_bClosed = true;

	return m_bClosed;
}

// Allocate and Fill CRow objects

bool CDBFile::FillRowArray()
{
	CRow *pRow;
	m_pbCurrentPos = (char *)m_pFileMap;
	int iIndex = 0;
	int nRet;

	while (nRet = FetchRow(&pRow) == FETCH_SUCCESS)
	{
		// Row fetched successfully. Add it to the array.
		pRow->m_bmk = ++iIndex;
 		m_Rows.Add(pRow);	
	}

	if (ERROR_IN_ROW_DATA == nRet)
	{ 
		OUT_LINE_FILE();
		return false; // fatal error
	}

	return true;
}


int CDBFile::FetchRow(CRow** ppRow)
{
	if (m_pbCurrentPos > m_pbEOF)
		return LAST_ROW_FETCHED;  	// we were already at the end of the line and therefore there is no row to fetch

	CRow *pRow = new CRow;

	pRow->m_pbStartLoc = m_pbCurrentPos;

	int cbRowSize = 0;
	while (m_pbCurrentPos <= m_pbEOF && *m_pbCurrentPos != '\r')
	{
		m_pbCurrentPos++;
		cbRowSize++;
	}

	
	if (m_pbCurrentPos <= m_pbEOF)
	{
		// not EOF
		if (*m_pbCurrentPos == '\r' && *(m_pbCurrentPos+1) == '\n')
			m_pbCurrentPos+=2; 	// skip the carriage return 
		else 
		{
			if (*m_pbCurrentPos == '\r')
			{
				delete pRow;
				return ERROR_IN_ROW_DATA;  // error because there was a \r without a \n
			}
		}
	}
	
	pRow->m_cbRowSize = cbRowSize;  // cbRowSize does not include the carriage return - line feed
	
	// defer filling the CColumn elements until the row is fetched and the data is fetched using IRowset::GetData

	*ppRow = pRow;
	return FETCH_SUCCESS;

}
	

ATLCOLUMNINFO * CDBFile::GetSchemaInfo(DBORDINAL * pNumCols)
{
	// Allocate the column information only once  as the functions we are returning the 
	// data from don't free.
	*pNumCols = m_cCols;

	return m_prgColInfo;
}

bool CDBFile::GetFileSchemaInfo(LPCTSTR pszFileName)
{
//	Do a bunch of stuff to get the schema information from the file	

  USES_CONVERSION;

  TCHAR szTmpFileName[_MAX_PATH];
  if(_MAX_PATH < _tcslen(pszFileName) )
	{
		ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: File path cannot be larger than 260 characters...\n");
		return false;
	}
	_tcscpy_s(szTmpFileName, _countof(szTmpFileName), pszFileName);
	szTmpFileName[_tcslen(szTmpFileName) - 3] = 's';

	FILE *fptr;
	if( fopen_s(&fptr, szTmpFileName,"r") )
	{
		ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Schema file cannot be read...\n");
		return false;
	}
	
	// Start reading the file...
	// Expected Format of the Schema file
    // <Table_Properties> <Column_Properties>[,...n]
    // Syntax :
	// <Table_Properties> ::= <Table_Name> <Number_of_Fields> <New_Line>
	// <Table_Name> ::= <Table_Alphabet>[1,....,20]
	// <Table_Alphabet> ::= [a,...,z] | [0,...9]
	// <Number_of_Fields> ::= [0,...,255]
	// <Table_Properties> ::= <Table_Name> <Number_of_Fields> <New_Line>
	// <Column_Properties> ::= <Column_Name> <Column_Type> <Column_Precision> <Column_Size><New_Line>
	// <Column_Name> ::= <Column_Alphabet>[1,....,20]
	// <Column_Alphabet> ::= [a,...,z] | [0,...9]
	// <Column_Type> ::= DBTYPE_I4 | DBTYPE_STR
	// <Column_Presicion> ::= [1...255]
	// <Column_Size> ::= [1...255]
	// <New_Line> ::= '\n'
    
    // Get the <Table_Properties>
	char szTblNm[40],szColNm[40],szColType[20];
	ULONG ulNumFld, ulColPrec, ulColSize;
	ulNumFld=ulColPrec=ulColSize =0;

	fscanf_s(fptr,"%s %d",szTblNm, _countof(szTblNm), &ulNumFld);
	if(_tcslen(szTblNm) > 20)
	{
		ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Table Name too long...\n");
		return false;
	}
	else if(ulNumFld > 255 || ulNumFld < 1)
	{
		ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Incorrect Number of fields specified...\n");
		return false;
	}
	// Set the number of columns
	m_cCols = ulNumFld;
	// Set Table_Name  
	_tcscpy_s(m_szTblNm, _countof(m_szTblNm), szTblNm);
	// Build the ColumnInfo structure
	m_prgColInfo = new ATLCOLUMNINFO[(sizeof(ATLCOLUMNINFO) * ulNumFld)];
    // Get the <Column_Properties>
	for(ULONG ulColCnt = 0; ulColCnt < ulNumFld; ulColCnt++)
	{
		fscanf_s(fptr,"%s %s %d %d",szColNm, _countof(szColNm), szColType, _countof(szColType), &ulColPrec,&ulColSize);
		if (_tcslen(szColNm) > 20)
		{
			ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Column Name too long...\n");
			return false;
		}
		else if (_tcscmp(szColType,"DBTYPE_I4") != 0 && _tcscmp(szColType,"DBTYPE_STR") != 0 ) 
		{
			ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Only DBTYPE_I4 or DBTYPE_STR types supported...\n");
			return false;
		}
		else if( (ulColPrec < 1 || ulColPrec > 255) && (_tcscmp(szColType,"DBTYPE_I4") == 0) ) 
		{
			ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Incorrect Column Precision specified...\n");
			return false;
		}
		else if ((ulColSize < 1 || ulColSize > 255) && (_tcscmp(szColType,"DBTYPE_STR") == 0) ) 
		{
			ATLTRACE2(atlTraceDBProvider, 0, "FILE ERROR: Incorrect Column Size specified...\n");
			return false;
		}
	
		// Do the assignment into the ATLCOLUMNINFO structure
		DWORD dwFlags = DBCOLUMNFLAGS_WRITE;
		memset(&m_prgColInfo[ulColCnt], 0, sizeof(ATLCOLUMNINFO));
		m_prgColInfo[ulColCnt].iOrdinal = ulColCnt + 1;
		m_prgColInfo[ulColCnt].columnid.eKind = DBKIND_GUID_NAME; //DBKIND_NAME;
		m_prgColInfo[ulColCnt].pTypeInfo = (ITypeInfo*)NULL;
		m_prgColInfo[ulColCnt].columnid.uGuid.guid = GUID_NULL;
		m_prgColInfo[ulColCnt].dwFlags = dwFlags;
		m_prgColInfo[ulColCnt].bScale = -1;
		// User modifiable Attributes
		m_prgColInfo[ulColCnt].columnid.uName.pwszName = SysAllocString(T2OLE(szColNm));
		m_prgColInfo[ulColCnt].pwszName = SysAllocString(T2OLE(szColNm));
		if(_tcscmp(szColType,"DBTYPE_I4") == 0)
		{
			m_prgColInfo[ulColCnt].ulColumnSize = 4;
			m_prgColInfo[ulColCnt].bPrecision = (BYTE) ulColPrec;
			m_prgColInfo[ulColCnt].wType = DBTYPE_I4;
		}
		else  //DBTYPE_STR
		{
			m_prgColInfo[ulColCnt].ulColumnSize = ulColSize;
			m_prgColInfo[ulColCnt].bPrecision = -1;
			m_prgColInfo[ulColCnt].wType = DBTYPE_STR;
		}
		if(0 == ulColCnt)
			m_prgColInfo[ulColCnt].cbOffset = 0;
		else
			m_prgColInfo[ulColCnt].cbOffset = m_prgColInfo[ulColCnt-1].cbOffset + m_prgColInfo[ulColCnt-1].ulColumnSize;

	}
	return true;
}


//TODO: remove this function - should be move to rowset because rowset will have 
//      final column information
void CDBFile::AllocateProxyBuffers(bool bUseBookmarks)
{
	size_t nSize = m_Rows.GetCount();
	for (size_t i = 0; i < nSize; i++)
	{
		m_Rows[i]->AllocProxyBuffer(m_prgColInfo, m_cCols);
	}

}


// DeleteRowImmediate:
//  This function deletes the row from the file immediately.
//  The record is not marked for deletion but is immediately removed.
//  With large files, this function may be expensive and it is 
//  recommended that the consumer use deferred updates (IRowestUpdate)
//int CDBFile::DeleteRowImmediate(CRow * pRow, ULONG ulRow)
int CDBFile::DeleteRowImmediate(CRow * pRow)
{
	// TODO: could derive row number from the bookmark. Should we do this?

	// CRow contains the starting location and the the row size 
	// Special consideration: if this is the last row, there may not be 
	// a "\r\n" pair so we need to check for this in the calculations
	// If it is not the last row being deleted then we need to subtract 2 bytes from 
	// the file count.
	ULONG ulRow = pRow->m_bmk;
	ULONG ulActRow = ulRow -1;  // Actual Row is Bookmark - 1

	size_t nRows = m_Rows.GetCount();

	BYTE *  pbOldEOF = (BYTE *)m_pbEOF;

	// check for last element of the array
	//if (ulRow == (ULONG)nRows -1)
	if (ulRow == (ULONG)nRows)
	{
//		m_pbEOF = m_Rows[ulRow]->m_pbStartLoc - 1;
		m_pbEOF = m_Rows[ulActRow]->m_pbStartLoc - 1;		
		// For the last element update the bookmark too
		m_Rows[ulActRow]->m_bmk --;
	}
	else 
	{
		// Copy record over previous record
		// Row sizes will stay the same.
//		BYTE * pbDest = (BYTE *)m_Rows[ulRow]->m_pbStartLoc;
		BYTE * pbDest = (BYTE *)m_Rows[ulActRow]->m_pbStartLoc;
		BYTE * pbSrc;
		for (size_t i = ulActRow+1; i < nRows; i++)
		{
			pbSrc = (BYTE *)m_Rows[i]->m_pbStartLoc;
			m_Rows[i]->m_pbStartLoc = (char *)pbDest;
			// m_Rows[i]->m_bmk = m_Rows[i-1]->m_bmk;
			m_Rows[i]->m_bmk --;

			
			for (int c = 0; c < m_Rows[i]->m_cbRowSize; c++)
			{
				*pbDest++ = *pbSrc++;
			}

			// add carriage return - line feed
			*pbDest++ = '\r';
			*pbDest++ = '\n';

		}
		m_pbEOF = (char *)pbDest-1;  // this will be used to set the end of file when we close the file
	}

	// slide everything up  - this is OK because only ReleaseRows will ever reference the row again
//		m_Rows[ulActRow]->m_bDeleted = true;
	if(1 == m_Rows.GetCount( ))
		m_Rows.RemoveAll();
	else if (ulActRow < m_Rows.GetCount())
	{
		m_Rows.RemoveAt(ulActRow);
	}
	else
	{
		return S_FALSE; 
		
	}
	// Close and re-open the file so that the transaction is completely done and the file is completely modified
	// This is an expensive operation but is done to ensure that the file is not currupted if the application is terminated
	// and the CDBFile is not closed properly
	SaveAndReOpenFile();

	return S_OK;
}

HRESULT CDBFile::UpdateRowImmediate(CRow * pRow, ATLCOLUMNINFO * prgColInfo, DBORDINAL ulCols)
{
	// TODO: currently we are updating the lazy way  - just take and move everything to make 
	// space or shrink the row.  
	//
	//  If the updated row is smaller than it was before, we will fill the rest of the row with white 
	//  space as we will strip off any white space in the reading of the row.
	//
	//  If the updated row is larger than the existing row, we move everything to make 
	//  space.  Other algorithms, could be used which wouldn't cause the shuffling of all of the 
	//  data but this would probably mean some kind of mechanism of marking rows as deleted.
	//  TODO: need to revist this
	bool bAddEOLtoEOF= false;
	bool bModifyEOF = false;
	ULONG ulBytesNeeded;
	ULONG ulBytesToDelete = 0;
	HRESULT hr = S_OK;

 	if ((ulBytesNeeded = pRow->CalculateRowData(prgColInfo, ulCols) )> 0)
	{
		if ( !(pRow->HasEOL((BYTE*)m_pbEOF)) )
		{	
			ulBytesNeeded += 2;  // add space needed to add a \r\n which may not be there
								// if this is the last row
			bAddEOLtoEOF = true;
		}
		// Accomodate pre-existing EOL which is not part of the data
		ULONG_PTR ulBytesToMove  = 0;
		ulBytesToMove = (ULONG_PTR)m_pbEOF - (ULONG_PTR)(pRow->m_pbStartLoc + pRow->m_cbRowSize) + 1;

		// need to make space for updated row
		ExpandFileAndAdjustRowObjects(ulBytesNeeded,EXPAND_FILE);
		if(true== bAddEOLtoEOF)
		{
			*(m_pbEOF-1) = '\r';
			*m_pbEOF = '\n';
		}
		// end of current row 
		BYTE * pbEOR = NULL;
			pbEOR =  (BYTE *)pRow->m_pbStartLoc + pRow->m_cbRowSize; 
		// Move rows to make room for new row
		//Move bytes from end of row to EOF - ulBytesNeeded
		memmove(pbEOR + ulBytesNeeded, pbEOR, ulBytesToMove);  
	}
	// If the data updated is less-than previously - implode the data 
	else if (0 == ulBytesNeeded )
	{
		// This ulBytesNeeded has to be positive
		ulBytesToDelete = pRow->CalculateRowDataToDelete(prgColInfo, ulCols); 
		if(true == m_Rows[m_Rows.GetCount()-1]->HasEOL((BYTE*) m_pbEOF) ) 
			bModifyEOF = true;

		// Accomodate pre-existing EOL which is not part of the data
		ULONG_PTR ulBytesToMove  = 0;
		ulBytesToMove = (ULONG_PTR)m_pbEOF - (ULONG_PTR)(pRow->m_pbStartLoc + pRow->m_cbRowSize) + 1;

		char*pprevEOF = m_pbEOF;
		ExpandFileAndAdjustRowObjects(ulBytesToDelete, CONTRACT_FILE);

		BYTE * pbEOR = NULL;
		pbEOR =  (BYTE *)pRow->m_pbStartLoc + pRow->m_cbRowSize; 
		//Move bytes from end of row to EOF - ulBytesNeeded
		memmove(pbEOR - ulBytesToDelete, pbEOR, ulBytesToMove);  
		if(		(ULONG)( pprevEOF - m_pbEOF) == ulBytesToDelete) // The file was not reallocated....
		{  // Since no new memory was allocated for the data 
			// there could junk between the current (smaller) EOF and the prev EOF...
			// This could be picked up when writing the file
			memset(m_pbEOF+1, 0, ulBytesToDelete);
		}
	}
	 // If the Number of Bytes required are in Error !!!
	else if(ulBytesNeeded < 0)
			return DB_E_DATAOVERFLOW;
	
	// now that there is space allocated, simply dump the row's data
	pRow->Update(prgColInfo, ulCols); // write the data to the file
	// The Start Location of all the remaining Rows need to be updated...
	if(ulBytesNeeded > 0)
		for(size_t iRow = pRow->m_bmk; iRow< m_Rows.GetCount(); iRow++)
			m_Rows[iRow]->m_pbStartLoc += ulBytesNeeded;
	 else if(0 == ulBytesNeeded)
		 for(size_t iRow = pRow->m_bmk; iRow< m_Rows.GetCount(); iRow++)
			m_Rows[iRow]->m_pbStartLoc -= ulBytesToDelete;

	// commit data to disk
	BOOL bSuccess = FlushViewOfFile(m_pFileMap, m_pbEOF - (char *)m_pFileMap + 1);
	if (!bSuccess)
		return E_OUTOFMEMORY;  // This is a generic error message for the present
//		return DBFILE_FLUSH_ERROR;
	return 0;
}


int CDBFile::SaveAndReOpenFile()
{
	void * pFileMapOld = m_pFileMap;
	ULONG_PTR nFileSize = m_pbEOF - (char *)m_pFileMap + 1;

	// commit data to disk
	BOOL bSuccess = FlushViewOfFile(m_pFileMap, nFileSize);
	if (!bSuccess)
		return DBFILE_FLUSH_ERROR;

	pFileMapOld = m_pFileMap;

	UnmapViewOfFile(m_pFileMap);

	CloseHandle(m_hMapFile);
	
	SetFilePointer(m_hFile, (LONG)nFileSize, NULL, FILE_BEGIN);

	SetEndOfFile(m_hFile);
	// CloseHandle(m_hFile);  don't actually close file

	// re-open and adjust row objects based on a new adjust potentially
	m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
	if (m_hMapFile == NULL)
	{
		OUT_LINE_FILE();
		return false;
	}

	// Create just one view of the memory-mapped file
	m_pFileMap = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_pFileMap == NULL)
	{
		OUT_LINE_FILE();
		return false;
	}

	// Check to see if the mapping address has changed.  If it has, must recalculate all 
	// CRow objects.
	if (m_pFileMap != pFileMapOld)
	{
		// calculate adjustment
		ULONG_PTR nAdjustment = (ULONG_PTR)m_pFileMap - (ULONG_PTR)pFileMapOld;

		size_t nSize = m_Rows.GetCount();
		for (size_t iRow = 0; iRow < nSize; iRow++)
		{
			m_Rows[iRow]->m_pbStartLoc += nAdjustment;
		}
		
		m_pbEOF += nAdjustment;
	}

	return true;
}



int CDBFile::ExpandFileAndAdjustRowObjects(ULONG ulBytesNeeded, bool bExpOrCon)
{
	void * pFileMapOld = m_pFileMap;
	ULONG nFileSize = (ULONG)(m_pbEOF - (char *)m_pFileMap + 1);

	// commit data to disk
//	BOOL bSuccess = FlushViewOfFile(m_pFileMap, nFileSize);
//	if (!bSuccess)
//		return DBFILE_FLUSH_ERROR;

	pFileMapOld = m_pFileMap;

	UnmapViewOfFile(m_pFileMap);

	CloseHandle(m_hMapFile);
	
//	SetFilePointer(m_hFile, nFileSize, NULL, FILE_BEGIN);

//	SetEndOfFile(m_hFile);
	// CloseHandle(m_hFile);  don't actually close file

	// re-open and adjust row objects based on a new address potentially
	if(EXPAND_FILE == bExpOrCon)
		m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, nFileSize + ulBytesNeeded, NULL);
	else
		m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE, 0, nFileSize - ulBytesNeeded, NULL);

	if (m_hMapFile == NULL)
	{
		OUT_LINE_FILE();
		return false;
	}

	// Create just one view of the memory-mapped file
	m_pFileMap = MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_pFileMap == NULL)
	{
		OUT_LINE_FILE();
		return false;
	}

	// Check to see if the mapping address has changed.  If it has, must recalculate all 
	// CRow objects.
	if (m_pFileMap != pFileMapOld)
	{
		// calculate adjustment
		ULONG_PTR nAdjustment = (ULONG_PTR)m_pFileMap - (ULONG_PTR)pFileMapOld;

		size_t nSize = m_Rows.GetCount();
		for (size_t iRow = 0; iRow < nSize; iRow++)
		{
			m_Rows[iRow]->m_pbStartLoc += nAdjustment;
		}

		if(EXPAND_FILE == bExpOrCon)
			m_pbEOF += nAdjustment + ulBytesNeeded;
		else 
			m_pbEOF += nAdjustment - ulBytesNeeded;
	}
	else
	{
		if(EXPAND_FILE == bExpOrCon)
			m_pbEOF += ulBytesNeeded;
		else  
			m_pbEOF -= ulBytesNeeded;
	}

	return 0;
}

HRESULT CDBFile::InsertRowImmediate(CRow * pRow, ATLCOLUMNINFO * prgColInfo, DBORDINAL ulCols, bool bData)
{
	bool bAddEOL=false;
	ULONG ulBytesNeeded;

	if(bData == true)  // Data is contained in the Row
		ulBytesNeeded = pRow->CalculateRowData(prgColInfo, ulCols); // m_cbRowsize == 0 when this called
	else
		ulBytesNeeded = pRow->CreateDefaultRow(prgColInfo, ulCols); 
	// If the Number of Bytes required are in Error !!!
	if (ulBytesNeeded < 0) 
		return DB_E_DATAOVERFLOW;

	// To accomodate the \r \n at the end of the line
	ulBytesNeeded += 2;

	// Check if the previous line if any had any EOL
	if(pRow->m_bmk > 1)
	{
		if (!(m_Rows[(pRow->m_bmk) - 2]->HasEOL((BYTE*)m_pbEOF)))
		{	
			ulBytesNeeded += 2;  // add space needed to add a \r\n which may not be there
								// if this is the last row
			bAddEOL = true;
		}
	}
	// need to make space for updated row
	ExpandFileAndAdjustRowObjects(ulBytesNeeded);
	
	// add \r\n on end of row if necessary
	if (bAddEOL)
	{
		m_Rows[pRow->m_bmk - 1]->AddEOL();
		// adjust start location of pRow 
		pRow->AdjustStartLocationForEOL();
	}

	// now that there is space allocated, simply dump the row's data
	pRow->Update(prgColInfo, ulCols); // write the data to the file

	pRow->AddEOL();

	// commit data to disk
	BOOL bSuccess = FlushViewOfFile(m_pFileMap, m_pbEOF - (char *)m_pFileMap + 1);
	if (!bSuccess)
		return DBFILE_FLUSH_ERROR;

	return 0;

}

BOOL CDBFile::AddRow(CRow * pRow, ATLCOLUMNINFO * prgColInfo, DBORDINAL ulCols)
{
	pRow->m_cbRowSize = 0; 
	pRow->AllocProxyBuffer(prgColInfo, ulCols);
	pRow->m_pbStartLoc = m_pbEOF + 1;
	BOOL bTmp = TRUE;
	
	try
	{
		m_Rows.Add(pRow);
	}
	catch (CAtlException& )
	{
		bTmp = FALSE;
	}

	if (bTmp == TRUE)
		pRow->m_bmk = (ULONG)m_Rows.GetCount(); // Initialize it 
   return bTmp;
}

