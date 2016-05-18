//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc 
//
// @module Privlib.h | 	This module contains header information for 
//						private library
//
// @comm
// Special Notes...:	(OPTIONAL NOTES FOR SPECIAL CIRCUMSTANCES)
//
// <nl><nl>
// Revision History:<nl>
//	
//	[00] MM-DD-YY	EMAIL_NAME	ACTION PERFORMED... <nl>
//	[01] 06-30-95	Microsoft	Created <nl>
//	[02] 10-16-95	Microsoft	Changed to single header for all private library users <nl>
//	[03] 12-01-96	Microsoft	Updated for release <nl>
//
// @head3 PRIVLIB Elements|
//
//---------------------------------------------------------------------------

#ifndef _PRIVLIB_H_
#define _PRIVLIB_H_

//suppress warnings about calling "unsecure" string functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif	//_CRT_SECURE_NO_WARNINGS

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif	//_CRT_SECURE_NO_DEPRECATE

//suppress warnings about calling unconformanced swprintf
#ifndef _CRT_NON_CONFORMING_SWPRINTFS
#define _CRT_NON_CONFORMING_SWPRINTFS
#endif	//_CRT_NON_CONFORMING_SWPRINTFS

///////////////////////////////////////////////////////////////////
// Includes 
//
///////////////////////////////////////////////////////////////////
#include "transact.h"
#include <msdadc.h>
#include <msdasc.h>			// Service Components header

#include "privcnst.h"
#include "prvtrace.h"
#include "miscfunc.h"		// Miscellaneous Private Library Functions
#include "ctable.hpp"		// CTable class
#include "coledb.hpp"		// COLEDB class
#include "ctranstn.hpp"		// CTransaction class
#include "cexterr.hpp"		// Extended Error class
#include "cparsestrm.hpp"	// CParseInitFile class
#include "CStorage.hpp"		// CStorage class
#include "Strings.h"		// OLE DB Strings
#include "CTree.hpp"		// CTree and CSchema classes


#endif // _PRIVLIB_H_
