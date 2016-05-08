//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IDBINIT.H | Header file for IDBInitialize test module.
//
// @rev 01 | 08-06-95 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//
#ifndef _IDBINIT_H_
#define _IDBINIT_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include "privlib.h"		// Private Library

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR wszInterfaceNotImpl[] = L"IDBInitialize::GetDBInitOptions is not Implimented. \n";

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define  NUMOFOPTS	 16		// Number of Initialization Options

#endif 	//_IDBINIT_H_
