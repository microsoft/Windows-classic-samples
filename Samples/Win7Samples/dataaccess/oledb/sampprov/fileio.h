//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module FILEIO.H | Class Definitions for CFileIO Class
//
//
#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <fstream>
#include "fileidx.h"
#include "hashtbl.h"
using namespace std;
#define MAX_INPUT_BUFFER 2048
#define MAX_COLUMNS	129		// 128 columns supported, but 1 extra
							// is needed since 1 based.

enum UPDTYPE {UPDATE, INSERT};

//--------------------------------------------------------------------
// @class CFileIO | Opens and manipulates a given CSV file.  Allows 
// deletions, reads, and updates.
// 
// @hungarian 
//
class CFileIO : public fstream		//@base public | fstream
{
private: //@access private
	//@cmember Count of rows in the table 
	DBCOUNTITEM		m_cRows;
	//@cmember Count of columns in the table
	DBORDINAL		m_cColumns;
	//@cmember Pointer to Column Names buffer
	LPTSTR			m_pColNames;
	//@cmember Array of pointers into the Column Names buffer
	LPTSTR			m_rgpColNames[MAX_COLUMNS];
	//@cmember Array of Column Data Types
	SWORD			m_rgswColType[MAX_COLUMNS];
	//@cmember Array of Precision values for columns
	UDWORD			m_rgudwColSize[MAX_COLUMNS];
	//@cmember True if columns contains signed values
	BOOL			m_rgfSigned[MAX_COLUMNS];
	//@cmember Array of pointers to Data	
	PCOLUMNDATA		m_rgpColumnData[MAX_COLUMNS];
	//@cmember Array of Lengths for the data
	SDWORD			m_rgsdwMaxLen[MAX_COLUMNS];
	//@cmember Input buffer of size MAX_INPUT_BUFFER
	LPTSTR			m_pvInput;
	//@cmember Offset into file for DataTypes
	size_t			m_ulDataTypeOffset;
	//@cmember Index Class declaration
	CFileIdx		m_FileIdx;
	//@cmember If the File is ReadOnly
	BOOL			m_FileReadOnly;
	//@cmember Array of DBCOLUMNINFO
	DBCOLUMNINFO	m_rgdbcolinfo[MAX_COLUMNS];
	//@cmember Array of column offsets used to interpret a rowbuffer
	DBLENGTH		m_rgdwDataOffsets[MAX_COLUMNS];	
	//@cmember Pointer to Info Array Heap (heap of column name strings)
	BYTE *			m_pbHeap; 
	//@cmember how many bytes of the column names heap is in use
	ULONG  			m_cbHeapUsed;
	//@cmember size of row data for this file
	DBLENGTH   		m_cbRowSize;  

private: //@access private
	//@cmember Break a stream into column names
	HRESULT ParseColumnNames(LPTSTR ptstrInput);
	//@cmember Break a stream into MetaData information
	HRESULT ParseDataTypes();
	//@cmember Control procedure to Read and parse intitial
	//information from the file
	HRESULT GenerateFileInfo();
	//@cmember Fill the COLUMNDATA structure
	HRESULT CFileIO::FillBinding(DBORDINAL cColumn, LPTSTR pvCopy);


public: //@access public
	//@cmember Constructor
	CFileIO(void);
	//@cmember Destructor
	~CFileIO(void);
	//@cmember Initialization routine
	HRESULT fInit(LPTSTR pstrFileName);	
	//@cmember Return the number of columns in the file
	inline DBORDINAL GetColumnCnt() { return m_cColumns; };
	//@cmember Return the number of rows in the file
	inline DBCOUNTITEM GetRowCnt() { return m_cRows; };
	//@cmember Determines if the rowset is ReadOnly
	inline BOOL IsReadOnly() { return m_FileReadOnly; };
	//@cmember Return the columninfo array
	inline DBCOLUMNINFO * GetColInfo() { return m_rgdbcolinfo; };
	//@cmember Return the column name heap
	inline BYTE * GetColNameHeap() { return m_pbHeap; };
	//@cmember Return the column name heap size
	inline ULONG GetColNameHeapSize() { return m_cbHeapUsed; };
	//@cmember Return row data size for this file
	inline DBLENGTH GetRowSize() { return m_cbRowSize; };
	//@cmember Retrieve pointers to the columns names
	HRESULT GetColumnName(DBORDINAL cCols, LPTSTR* pptstrName = NULL);
	//@cmember Retrieve the metadata for a particular column
	HRESULT GetDataTypes(DBORDINAL cCols, SWORD* pswType, UDWORD* pudwColDef, BOOL* pfSigned);
	//@cmember Set the Binding Areas.
	HRESULT SetColumnBind(DBORDINAL cCols, PCOLUMNDATA pColumn);
	//@cmember Extract the data from a stream
	HRESULT ParseRowValues();
	//@cmember Fetch A single rows data values
	HRESULT Fetch(DBCOUNTITEM ulRow);
	//@cmember Update the current rows values
	HRESULT UpdateRow(DBCOUNTITEM ulRow, BYTE* pbProvRow, UPDTYPE eUpdateType);
	//@cmember Remove the specified row from the file
	HRESULT DeleteRow(DBCOUNTITEM ulRow);
	//@cmember Determines if the row has already been deleted
	HRESULT IsDeleted(DBCOUNTITEM ulRow);
	//@cmember Gathers information that both the CCommand and CRowset objects use
	HRESULT GatherColumnInfo();
	//@cmember Return a ptr to a COLUMNDATA struct
	COLUMNDATA * GetColumnData(DBORDINAL cCols, ROWBUFF * pRowBuff);
};

typedef CFileIO FAR * PCFILEIO;

#endif

