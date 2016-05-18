//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IDBCRCMD.H | Header file for IDBCreateCommand test module.
//
// @rev 01 | 08-06-95 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IDBCRCMD_H_
#define _IDBCRCMD_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		// Private Library

#define  NUMOFCMDOBJ 11		// Number of Command Object Interfaces
#define  MAX_CMDOBJ 100		// MAX Number of Command Objects

// Used for the Zombie cases
enum ETXN {ETXN_COMMIT,ETXN_ABORT};

#endif 	//_IDBCRCMD_H_
