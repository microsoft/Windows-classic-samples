// File: CBDFile.h
//
//  This file contains the information for the CDBFile class.
//
//	CDBFile does the following:
//
//		- Opens the text file which will be read/written
//		- Gathers information from the .INI information file
//		- Contains the CRows array which contains all of the row 
//        information
//      - Does all of the reading and writing to the file
//		- Contains all of the column information
//      
#ifndef __CDBFile_H_
#define __CDBFile_H_

#include "CRow.h"
#include <comdef.h>

//#define MAX_SUPPORTED_RECORDS 10000
#define MAX_RECORD_SIZE 10000

#define LAST_ROW_FETCHED 1
#define ERROR_IN_ROW_DATA -1
#define FETCH_SUCCESS 0
#define DBFILE_FLUSH_ERROR 100
#define MAX_TABLE_NAME_SIZE 20

#define EXPAND_FILE true
#define CONTRACT_FILE false

#ifdef _DEBUG
	#define OUT_LINE_FILE()  { TCHAR szOut[500]; wsprintf(szOut, "Error: %s, Line %d\n", __FILE__, __LINE__); \
							ATLTRACE(szOut); }
#else 
	#define OUT_LINE_FILE()
#endif


class CDBFile
{
public:

	
//	Attributes
	
	HANDLE m_hFile;		// handle of the data file
	HANDLE m_hMapFile;	// handle of the file mapping object
	LPVOID m_pFileMap;  // pointer to the beginning of the mapped file
	char * m_pbEOF;		// pointer to the last character in the file
	char * m_pbCurrentPos; // used to track the current location in the file
	
	CAtlArray<CRow *> m_Rows;
	
	
	CDBFile():m_bClosed(true),m_prgColInfo(NULL)
	{
		
	}

	~CDBFile();
//	Methods
	bool Open(LPCTSTR pszFileName, bool bOpenMode= false);
	bool Close();
	bool FillRowArray();
	int FetchRow(CRow** pRow);
	int DeleteRowImmediate(CRow * pRow);
	HRESULT UpdateRowImmediate(CRow * pRow, ATLCOLUMNINFO * pColInfo, DBORDINAL ulCols);
	HRESULT InsertRowImmediate(CRow * pRow, ATLCOLUMNINFO * pColInfo, DBORDINAL ulCols,bool bData);
	int SaveAndReOpenFile();
	int ExpandFileAndAdjustRowObjects(ULONG ulBytesNeeded, bool bExpOrCon = EXPAND_FILE);
	BOOL AddRow(CRow * pRow, ATLCOLUMNINFO * prgColInfo, DBORDINAL ulCols);

	void AllocateProxyBuffers(bool bUseBookmarks);

//schema information
	bool GetFileSchemaInfo(LPCTSTR pszFileName);
	ATLCOLUMNINFO * GetSchemaInfo(DBORDINAL * pNumCols);
	ATLCOLUMNINFO * m_prgColInfo;
	DBORDINAL m_cCols;
	TCHAR m_szTblNm[MAX_TABLE_NAME_SIZE];
	bool m_bClosed;
};

#endif // __CDBFile_H_