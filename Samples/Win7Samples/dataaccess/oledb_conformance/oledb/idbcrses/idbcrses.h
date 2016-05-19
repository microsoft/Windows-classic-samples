//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IDBCRSES.H | Header file for IDBCreateSession test module.
//
// @rev 01 | 08-06-95 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IDBINFO_H_
#define _IDBINFO_H_

#include "oledb.h"			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		// Private Library

#define  NUMOFSESOBJ 13		// Number of Session Object Interfaces
#define  MAX_SESOBJ 100		// MAX Number of Session Objects

// DSO states
#define DSO_INIT	TRUE
#define DSO_UNINIT	FALSE

// Used for the Zombie cases
enum ETXN {ETXN_COMMIT,ETXN_ABORT};

#endif 	//_IDBCRSES_H_