// File: CRow.h
//
//
//	This file contains the definition for the CRow class
//
//	The CRow class does the following:
//
//
//		- Contains the conversion functions for getting the 
//		  data from the string of data for the row into each
//		  CColumn class
//		- Contains the CColumn array which has an element
//		  for each column
//      - Contains the HROW associate with that column
//      - Contains bookmark information
//      - Contains pointer to the starting location of the row
//		  in the file and ending offset
//

#ifndef __CRow_H_
#define __CRow_H_

#define MAX_COLUMNSIZE 255
#define BAD_RECORD_FORMAT -1


class CRow
{
public:
	char*		m_pbStartLoc;
	int			m_cbRowSize;

	ULONG		m_bmk;

	BYTE *		m_pbProxyData;
	size_t		m_cbProxyData;

	bool		m_bRetrieved;

	CRow()
	{
		m_pbStartLoc = NULL;
		m_cbRowSize = 0;
		
		m_bmk = 0;   

		m_pbProxyData = NULL;
		m_cbProxyData = 0;

		m_bRetrieved = false;
	}

// Methods
	void AllocProxyBuffer(ATLCOLUMNINFO * prgColInfo, DBORDINAL cNumCols);
	void GetProxyData(ATLCOLUMNINFO * pColInfo, DBORDINAL cCols);
	int  FetchColumnString(char * pszColData, char ** ppPos);
	void StoreInProxy(DBORDINAL iCol, char * szColumnString, ATLCOLUMNINFO * pColInfo);
	int  Update(ATLCOLUMNINFO * prgColInfo, DBORDINAL cCols);
	int  CalculateRowData(ATLCOLUMNINFO * prgColInfo, DBORDINAL cNumCols, bool bAddEOL = false);
    int  CreateDefaultRow(ATLCOLUMNINFO *prgColInfo, DBORDINAL cNumCols, bool bAddEOL = false); 
	int  CalculateRowDataToDelete(ATLCOLUMNINFO * prgColInfo, DBORDINAL cNumCols, bool bAddEOL=false);
	bool HasEOL(BYTE * pbEOF);
	void AddEOL();
	void AdjustStartLocationForEOL() { m_pbStartLoc +=2; }  // previous line had EOL added 
	                                                       // we need to move this rows starting
	                                                       // location
};

#endif __CROW_H_
