//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ORAPROC.H | ORAPROC header file for test modules.
//
//
// @rev 01 | 03-21-95 | Microsoft | Created
// @rev 02 | 09-06-95 | Microsoft | Updated
//

#ifndef _ORAPROC_H_
#define _ORAPROC_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

//
// Definitions
//
#define PREPARE			1
#define NO_PREPARE		0

#define SETPARAMINFO	1
#define NOSETPARAMINFO	0

#define GETPARAMINFO	1
#define NOGETPARAMINFO	0

#define NO_DEFAULT		0
#define DEFAULT			1

#define NEGSCALE		0
#define NOSCALE			1

#define MAX_BINDINGS	10  // Shouldn't need more than 10 Bindings ever 
#define MAX_PARAMSETS	10	// Shouldn't need more that 10 parameter sets

#define ORAMAX_TABLELEN 30
#define ORAMAX_COLLEN	30

#define MAXTMPBUF		2000

struct ValueInfo
{
	DBSTATUS		dbsStatus;
	ULONG			cbLength;
	void			*pValue;
};


//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszINITCASEfailed[]=L"Test Case initialization failed\n";



#endif 	//_ORAPROC_H_
