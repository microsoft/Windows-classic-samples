//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright (C) 1995-2000 Microsoft Corporation
//
// @doc
//
// @module IGETSESSION.H | Header file for IGetSession test module.
//
// @rev 01 | 10-05-98 | Microsoft | Created
//

#ifndef _IGETSESSION_H_
#define _IGETSESSION_H_


//////////////////////////////////////////////////////////////////////
// Includes
//
//////////////////////////////////////////////////////////////////////
#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"
#include "privlib.h"		// Private Library

///////////////////////////////////////////////////////////////////////
//Defines
//
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
//Enumerations
//
///////////////////////////////////////////////////////////////////////

//This enumeration represents various ways of obtaining a ROW 
//object. For e.g., TC_* represents the Test Case which will
//test a GetSession on a ROW object obtained from *.
enum ETESTCASE
{
	TC_RowsetByOpenRowset = 1,	//Row from a Rowset got by OpenRowset
	TC_RowsetByCommand,			//Row from a Rowset got by executing a command
	TC_SchemaRowset,			//Row from a Schema Rowset
	TC_ColumnsRowset,			//Row from a Columns Rowset
	TC_DirectBindOnRootBinder,	//Row from direct binding thru Root Binder
	TC_DirectBindOnProvider,	//Row from direct binding thru Provider's Session
	TC_GetSourceRow,			//Row from GetSourceRow call on a Stream object
	TC_OpenRowsetDirect,		//Row directly from OpenRowset call
	TC_CommandDirect			//Row directly from command (singleton)
};

#endif 	//_IGETSESSION_H_
