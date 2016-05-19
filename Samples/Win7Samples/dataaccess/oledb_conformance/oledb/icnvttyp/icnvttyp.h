//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module ICNVTTYP.H | Header file for IConvertType test module.
//
// @rev 01 | 09-06-95 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IConvertType_H_
#define _IConvertType_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"		// OLE DB 
#include "msdadc.h"			// MSDADC Data Conversion Library
#include "privlib.h"		// Private Library

// Used for the Zombie cases
enum ETXN {ETXN_COMMIT,ETXN_ABORT};

//-----------------------------------------------------------------------------
// String constants
//-----------------------------------------------------------------------------
const WCHAR CLASSID_IDataConvert[] = L"{c8b522d1-5cf3-11ce-ade5-00aa0044773d}";

#endif 	//_ICNVTTYP_H_
