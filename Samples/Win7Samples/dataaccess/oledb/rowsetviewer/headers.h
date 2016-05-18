//-----------------------------------------------------------------------------
// Microsoft OLE DB RowsetViewer
// Copyright (C) 1994 - 1999 By Microsoft Corporation.
//
// @doc
//
// @module HEADERS.H
//
//-----------------------------------------------------------------------------------

#ifndef _HEADERS_H_
#define _HEADERS_H_

///////////////////////////////////////////////////////////////
// Defines
//
///////////////////////////////////////////////////////////////
#define STRICT			//Strict type checking for Window types


///////////////////////////////////////////////////////////////
// Includes
//
///////////////////////////////////////////////////////////////
#include "oledb.h"		//OLE DB Header 
#include "oledberr.h"	//OLE DB Errors
#include "msdasc.h"		//OLE DB ServiceComponents
#include "msdaguid.h"	//CLSID_OLEDB_ENUMERATOR
#include <msdadc.h>		//DataConversion library

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>	//InitCommonControls

#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <wchar.h>
#include <ocidl.h>		//IConnectionPoint
#include <olectl.h>		//IConnectionPoints interface
#include <cguid.h>		//GUID_NULL
#include <locale.h>		//setLocale
#include <richedit.h>	//RichEdit Control Header
#include <htmlhelp.h>	//htmlhelp
#include <atlbase.h>	//ATL

#include "resource.h"	//Resources

#include "Common.h"


#include "CWinApp.h"
#include "CMainWindow.h"
#include "Spy.h"


#endif //_HEADERS_H_
