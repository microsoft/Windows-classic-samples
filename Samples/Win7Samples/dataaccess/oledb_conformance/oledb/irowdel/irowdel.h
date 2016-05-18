//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IROWDEL.H | Header file for IRowsetChange::Delete test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IROWDEL_H_
#define _IROWDEL_H_

#include "oledb.h"			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		// Private Library

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define	COLUMN_ALIGNVAL			8

typedef struct 
{
	ULONG ulStatus;		// for Read: indicates null, coercion, or any error state
	ULONG ulLength;		// true count of length of data pulled back, count of bytes
	BYTE bValue[1];		// data pulled back
} COLUMNDATA; 

#endif	//_IROWDEL_H_
