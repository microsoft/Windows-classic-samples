#include "stdafx.h"
#include "CRow.h"

// Allocating a buffer to use for storage of the row 
// This proxy will be the temporary storage area for values as they are 
// passed from file to the consumer and back.
void CRow::AllocProxyBuffer(ATLCOLUMNINFO * prgColInfo, DBORDINAL cNumCols)
{
	DBLENGTH uSize = 0;
	
	for (DBORDINAL i = 0; i < cNumCols; i++)
	{
		switch(prgColInfo[i].wType)
			{
			case DBTYPE_I4:
				uSize += 4;
				break;
			case DBTYPE_STR:
				uSize += prgColInfo[i].ulColumnSize;
				break;
	#ifdef _DEBUG				
			default: 
			// the types in the column info structures should always be correct but
			// this is just a precaution
				_ASSERTE(0);
				return;
	#endif
			}
	}
	m_pbProxyData = new BYTE[uSize];
	m_cbProxyData = uSize;
	memset(m_pbProxyData,0,uSize);
}

// GetProxyData will convert the data in the file into the native data
// specified in the .INI file.
void CRow::GetProxyData(ATLCOLUMNINFO * pColInfo, DBORDINAL cCols)
{
//	if (m_bRetrieved)
//		return;  // already have the data for the row
	
	char* pPos = m_pbStartLoc;
	char  szColumnString[MAX_COLUMNSIZE];

	// pPos will be incremented as data is fetched and can be passed into 
	// subsequent calls

	
	//TODO: optimize by passing in the proxybuffer  into FetchColumnString for string fields
	for (DBORDINAL i = 0; i < cCols; i++)
	{
//		if (!(pColInfo[i].dwFlags &= DBCOLUMNFLAGS_ISBOOKMARK))
		if (!(pColInfo[i].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK))
		{
			int nResult = FetchColumnString(szColumnString, &pPos);
			if (nResult == BAD_RECORD_FORMAT)
				return;  // TODO: handle bad file info
			StoreInProxy(i, szColumnString, pColInfo);
		}
		else 
			StoreInProxy(i, NULL, pColInfo);
		
	}	
	
	m_bRetrieved = true;
}

int CRow::FetchColumnString(char * pszColData, char ** ppPos)
{
	// TODO: need to parse out white space before and after  column data

	if (*ppPos > m_pbStartLoc + m_cbRowSize - 1)
		return BAD_RECORD_FORMAT;
	while ((*ppPos <= (m_pbStartLoc + m_cbRowSize - 1)) && **ppPos != ',')
	{
		*pszColData++ = **ppPos;
		(*ppPos)++;
	}
	if (**ppPos == ',')
		(*ppPos)++;

	*pszColData = '\0';
	return 0;	
}

void CRow::StoreInProxy(DBORDINAL iCol, char * szColumnString, ATLCOLUMNINFO * pColInfo)
{
	// TODO: Probably need to do something here with bookmarks
	// pColInfo is ordered by ordinal so we can do a simple array look-up
	// currently only supports DBTYPE_I4
	void * pvBuffer = m_pbProxyData + pColInfo[iCol].cbOffset;

	switch(pColInfo[iCol].wType)
	{
		case DBTYPE_I4:
			if (pColInfo[iCol].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK)
				*((long *)pvBuffer) = m_bmk;
			else
				*((long *)pvBuffer) = atoi(szColumnString);
			break;
		case DBTYPE_STR:
			_tcscpy_s((char*) pvBuffer, m_cbProxyData - pColInfo[iCol].cbOffset, szColumnString);
			break;
#ifdef _DEBUG				
		default:
			// Should never get here if we have all of the types supported and we 
			// validate the types at the time of reading the schema
			_ASSERTE(0);
			break;
#endif
	}
}


int CRow::CalculateRowData(ATLCOLUMNINFO * prgColInfo, DBORDINAL cNumCols, bool bAddEOL)
{
	size_t nBytesNeeded = 0;


	for (ULONG i = 0; i < cNumCols; i++)
	{

		void * pvBuffer = m_pbProxyData + prgColInfo[i].cbOffset;
		
		switch(prgColInfo[i].wType)
		{
		case DBTYPE_I4:
			if (!(prgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK))
			{
				//convert to string
				char szTemp[MAX_COLUMNSIZE];
				_itoa_s(*((long*)pvBuffer), szTemp, _countof(szTemp), 10);
				nBytesNeeded += _tcslen(szTemp);
			}	
			else 
				continue;

			break;
		case DBTYPE_STR:
			nBytesNeeded += _tcslen((char*)pvBuffer);
			if(_tcslen((char*)pvBuffer)> prgColInfo[i].ulColumnSize)
				return -1;
			break;
#ifdef _DEBUG				
		default: 
		// the types in the column info structures should always be correct but
	    // this is just a precaution
			_ASSERTE(0);
			return -1;
#endif

		}

		nBytesNeeded++; // add one byte for the comma
	}

	nBytesNeeded--;	// subtract the count which was added for the comma 

	if (bAddEOL)
		nBytesNeeded += 2;
	   
	return (int)(nBytesNeeded - m_cbRowSize) < 0 ? 0 : (int)(nBytesNeeded - m_cbRowSize);
}

int CRow::CalculateRowDataToDelete(ATLCOLUMNINFO * prgColInfo, DBORDINAL cNumCols, bool bAddEOL)
{
	DBLENGTH nBytesNeeded = 0;


	for (DBORDINAL i = 0; i < cNumCols; i++)
	{

		void * pvBuffer = m_pbProxyData + prgColInfo[i].cbOffset;
		
		switch(prgColInfo[i].wType)
		{
		case DBTYPE_I4:
			if (!(prgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK))
			{
				//convert to string
				char szTemp[MAX_COLUMNSIZE];
				_itoa_s(*((long*)pvBuffer), szTemp, _countof(szTemp), 10);
				nBytesNeeded += _tcslen(szTemp);
			}	
			else 
				continue;

			break;
		case DBTYPE_STR:
			nBytesNeeded += _tcslen((char*)pvBuffer);
			if(_tcslen((char*)pvBuffer)> prgColInfo[i].ulColumnSize)
				return -1;
			break;
#ifdef _DEBUG				
		default: 
		// the types in the column info structures should always be correct but
	    // this is just a precaution
			_ASSERTE(0);
			return -1;
#endif

		}

		nBytesNeeded++; // add one byte for the comma
	}

	nBytesNeeded--;	// subtract the count which was added for the comma 

	if (bAddEOL)
		nBytesNeeded += 2;
	   
	return (int)(m_cbRowSize - nBytesNeeded);
}

int CRow::Update(ATLCOLUMNINFO * prgColInfo, DBORDINAL cCols)
{
	// CDBFile will update the row size
	BYTE * pbCurrentPos = (BYTE *)m_pbStartLoc;


	

	// copy data from proxy to the file
	for (DBORDINAL i = 0; i < cCols; i++)
	{
		void * pvBuffer = m_pbProxyData + prgColInfo[i].cbOffset;
		switch(prgColInfo[i].wType)
		{
		case DBTYPE_I4:
			if (prgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK)
				continue;
			else
			{ 
				//convert to string
				char szTemp[MAX_COLUMNSIZE];
				_itoa_s(*((long*)pvBuffer), szTemp, _countof(szTemp), 10);
				//copy from buffer to file
				for (int n = 0; szTemp[n] != '\0'; n++)
					*pbCurrentPos++ = szTemp[n];
			}

			break;

		case DBTYPE_STR:
			{
				char* pszTemp;
				pszTemp = (char *)pvBuffer;
				for (DBLENGTH n = 0; *pszTemp != '\0'; n++)
					*pbCurrentPos++ = *pszTemp++;
			}
			break;
#ifdef _DEBUG				
		default: 
			// the types in the column info structures should always be correct but
			// this is just a precaution
			_ASSERTE(0);
			return 0;
#endif
		}

		if (i != cCols -1) // not the last column
			*pbCurrentPos++ = ',';

	}

	/*	// fill remainder of bytes with spaces
	// This can occur because we don't allocate a new row if the updated row will be 
	// smaller
	if ((pbCurrentPos  - (BYTE *)m_pbStartLoc) < m_cbRowSize)
	{
	int n = m_cbRowSize - (pbCurrentPos - (BYTE *)m_pbStartLoc);
	for (;n != 0; n--)
	*pbCurrentPos++= ' ';

	}*/

	m_cbRowSize = (int) (pbCurrentPos - (BYTE *)m_pbStartLoc);


	// add seperator
	//	*pbCurrentPos++ = '\r';
	//	*pbCurrentPos = '\n';

	return 0;
}


bool CRow::HasEOL(BYTE * pbEOF)
{
	char * pbEOR = m_pbStartLoc + m_cbRowSize;
	if (pbEOR <= (char *)pbEOF && pbEOR[0] == '\r' && pbEOR[1] == '\n')
		return true;
	else 
		return false;
}

void CRow::AddEOL()
{
	//assumes the space has already been allocated to do this
	char * pbEOR = m_pbStartLoc + m_cbRowSize;
    *pbEOR++ = '\r';
	*pbEOR = '\n';
	
}


int CRow::CreateDefaultRow(ATLCOLUMNINFO *prgColInfo, DBORDINAL cNumCols, bool bAddEOL)
{
	USES_CONVERSION;
	DBLENGTH nBytesNeeded = 0;
	for (DBORDINAL i = 0; i < cNumCols; i++)
	{

		void * pvBuffer = m_pbProxyData + prgColInfo[i].cbOffset;
		
		switch(prgColInfo[i].wType)
		{
		case DBTYPE_I4:
			if (!(prgColInfo[i].dwFlags & DBCOLUMNFLAGS_ISBOOKMARK))
			{
				//Store default '0' in the Proxy Buffer
				StoreInProxy(i, "0", prgColInfo);
				nBytesNeeded ++;
			}
			else  // Do not store if it is a bookmark
				continue;
			break;
		case DBTYPE_STR:
			// Store the name of the field 
				
			if (_tcslen(OLE2A(prgColInfo[i].pwszName)) > 0  && _tcslen(OLE2A(prgColInfo[i].pwszName)) < prgColInfo[i].ulColumnSize)
			{
				StoreInProxy(i, OLE2A(prgColInfo[i].pwszName), prgColInfo);
				nBytesNeeded += _tcslen(OLE2A(prgColInfo[i].pwszName));
			}
			else
			{
				// Store a '?' 
				StoreInProxy(i,"?", prgColInfo);
				nBytesNeeded ++;
			}
			break;
#ifdef _DEBUG				
		default: 
		// the types in the column info structures should always be correct but
	    // this is just a precaution
			_ASSERTE(0);
			return -1;
#endif

		}

		nBytesNeeded++; // add one byte for the comma
	}
	nBytesNeeded--;	// subtract the count which was added for the comma 
	if (bAddEOL)
		nBytesNeeded += 2;
	return (int)(nBytesNeeded - m_cbRowSize) < 0 ? 0 : (int)(nBytesNeeded - m_cbRowSize);
}