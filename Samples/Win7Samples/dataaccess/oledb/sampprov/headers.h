//--------------------------------------------------------------------
// Microsoft OLE DB Sample Provider 
// (C) Copyright 1991 - 1999 Microsoft Corporation. All Rights Reserved.
//
// @doc
//
// @module HEADERS.H | Precompiled headers
//
#ifndef _HEADERS_H_
#define _HEADERS_H_



//	Don't include everything from windows.h, but always bring in OLE 2 support
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define INC_OLE2

#define STRICT					//Strict type checking for Window APIs

// Basic Windows and OLE everything
#include <windows.h>
#include <windowsx.h>
#include <limits.h>				
#include <stdio.h>				// vsnprintf, etc.
#include <stddef.h>				// offsetof, etc.
#include <wchar.h>				// swprintf
#include <commdlg.h>			// GetOpenFileName

//	OLE DB headers
#include "oledb.h"
#include "oledberr.h"

//	Data conversion library header
#include "msdadc.h"

// MSDASQL Guids (for conversion library guid)
#include "msdaguid.h"


//	Sample Provider -specific general headers
#include "sampprov.h"
#include "asserts.h"
#include "utilprop.h"

// GUIDs
#include "guids.h"
#include "resource.h"

//	CDataSource object and contained interface objects
#include "datasrc.h"

// CDBSession object and contained interface objects
#include "dbsess.h"

//	CRowset object and contained interface objects
#include "rowset.h"

//	CCommand object and contained interface objects
#include "command.h"

//	CImpIAccessor implementation
#include "accessor.h"

//	CBinder object and contained interface objects
#include "binder.h"

//	CRow object and contained interface objects
#include "row.h"

//	CStream object and contained interface objects
#include "stream.h"

//	General utility functions
#include "common.h"
// safe string funcs
#include "strsafe.h"
#endif

