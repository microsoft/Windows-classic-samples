//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IROWINFO.H | The eader file for IRowsetInfo test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IROWINFO_H_
#define _IROWINFO_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		//include private library, which includes
							//the "transact.h"

// Used for the Zombie cases
enum ETXN {ETXN_COMMIT,ETXN_ABORT};

#endif 	//_IROWINFO_H_
