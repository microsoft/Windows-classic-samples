//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IGETSOURCEROW.H | IGETSOURCEROW header file for test modules.
//
//
// @rev 01 | 03-21-95 | Microsoft | Created
// @rev 02 | 09-06-95 | Microsoft | Updated
//

#ifndef _IGETSOURCEROW_H_
#define _IGETSOURCEROW_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

enum ETESTCASE
{
	TC_Empty = 1,
	TC_IGetRow_Open,
	TC_IGetRow_GetCol,
	TC_BindResource_Open,
	TC_BindResource_GetCol,
	TC_Execute_Open,
	TC_Execute_GetCol,
	TC_OpenRowset_Open,
	TC_OpenRowset_GetCol,
	TC_BindResource_Stream,
	TC_CreateRow_Stream,
	TC_IScopedOps_Stream,
	TC_BindSession_Stream,
	TC_CreateRowSession_Stream,
};

enum ESTREAMSOURCE
{
	EROWOPEN		= 1,
	EROWGETCOL,	
	EROWBOTH,	
};

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------




#endif 	//_IGETSOURCEROW_H_
