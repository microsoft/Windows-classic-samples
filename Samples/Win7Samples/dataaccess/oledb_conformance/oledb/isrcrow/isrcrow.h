//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ISRCROW.H | Header file for ISourcesRowset test module.
//
// @rev 01 | 09-11-96 | Microsoft | Created
// @rev 02 | 12-01-96 | Microsoft | Updated
//

#ifndef _ISRCROW_H_
#define _ISRCROW_H_

#include "initguid.h"						  
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "msdaguid.h"
#include "msdasql.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define MAXPROP				20
#define COLUMN_COUNT_ROOT	7
#define COLUMN_COUNT_STD	6
#define ROW_COUNT			100
#define PROPERTY_COUNT		20
#define DATA_SIZE			2002
#define MAX_NUM_PROV		2500
#define REG_BUFFER			256
#define FETCH_ROW_ROOT		2
#define FETCH_ROW_STD		5

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define RETURN(fResult) if(fResult) return TRUE; else return FALSE;

//-----------------------------------------------------------------------------
// ENUM
//-----------------------------------------------------------------------------
enum PROP_STATUS
    {	NOTSUPPORTED	= 0,
		SUPPORTED		= 1,
		SETTABLE		= 2
    };


#endif 	//_ISRCROW_H_
