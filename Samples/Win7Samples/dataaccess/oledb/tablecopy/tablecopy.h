//-----------------------------------------------------------------------------
// Microsoft OLE DB TABLECOPY Sample
// Copyright (C) 1991-2000 Microsoft Corporation
//
// @doc
//
// @module TABLECOPY.H
//
//-----------------------------------------------------------------------------

#ifndef _TABLECOPY_H_
#define _TABLECOPY_H_


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "oledb.h"


///////////////////////////////////////////////////////////////
// Forward Declarations
//
///////////////////////////////////////////////////////////////
class CTable;
class CWizard;


///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define MAX_QUERY_LEN			4096
#define MAX_NAME_LEN			256

#define MAX_COL_SIZE		   50000
#define MAX_BLOCK_SIZE			  20
#define MAX_STREAM_BLOCK_SIZE	2000

// Create param bitmasks describe the parameter required on create table
#define CP_PRECISION		0x0001
#define CP_SCALE			0x0002
#define CP_LENGTH			0x0004
#define CP_MAXLENGTH		0x0008

// This macro will determine if the type is numeric, and 
// if numeric -> bPrecision, if not numeric -> dwColumnSize
#define COLINFO_SIZE(ColInfo) (IsNumericType(ColInfo.wType) ? ColInfo.bPrecision : ColInfo.ulColumnSize)


/////////////////////////////////////////////////////////////////
// CTableCopy
//
/////////////////////////////////////////////////////////////////
class CTableCopy
{
public:
	//Constructors
	CTableCopy(CWizard* pCWizard);
	virtual ~CTableCopy();
	
	//Members
	virtual HRESULT MapTypes();
	virtual HRESULT CopyTables();

	//Row Options
	DWORD		m_dwRowOpt;			// Row Options
	ULONG		m_ulMaxRows;		// Maximum rows or ALL_ROWS
	
	//Insert Options
	DWORD		m_dwInsertOpt;		// Insert Options
	ULONG		m_ulParamSets;		// Number of Parameters Sets
	
	//BLOB options
	DWORD		m_dwBlobOpt;        // Blob Options
	ULONG		m_ulBlobSize;       // Maximum Size for BLOB Columns

	//Options
	BOOL		m_fShowQuery;		// TRUE to display SQL statements
	BOOL		m_fCopyTables;		// TRUE to create the table definition
	BOOL		m_fCopyIndexes;		// TRUE to create indexes on new table
	BOOL		m_fCopyPrimaryKeys;	// TRUE to copy primary keys on new table

	//Data
	CTable*		m_pCFromTable;		//Source Table
	CTable*		m_pCToTable;		//Target Table

	BOOL		m_fTranslate;		// TRUE to translate object names
	CWizard*    m_pCWizard;
};


#endif	//_TABLECOPY_H_
