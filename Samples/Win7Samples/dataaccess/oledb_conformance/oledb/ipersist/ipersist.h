//--------------------------------------------------------------------
// Microsoft OLE DB Test
//
// Copyright 1995-2000 Microsoft Corporation.  
//
// @doc
//
// @module IPERSIST.H | Header file for IPersist test module.
//
// @rev 01 | 02-04-96 | Microsoft | Created
// @rev 02 | 04-25-98 | Microsoft | Updated
//

#ifndef _IPERSIST_H_
#define _IPERSIST_H_

#include "oledb.h" 			// OLE DB Header Files
#include "oledberr.h"

#include <objbase.h>		// StgCreateDocFile
#include <sys/stat.h>

#include "privlib.h"		// Private Library

//--------------------------------------------------------------------
// Constant defines
//--------------------------------------------------------------------
#define	PATH_SIZE 	100
#define	PNUM_ROWS	10

//--------------------------------------------------------------------
// Constant strings
//--------------------------------------------------------------------
const WCHAR 	wszNoPrompt[]				= L"Running in No Prompt mode, prompting testcase skipped.";
const WCHAR		wszErrorDeletingFile[]		= L"Error Deleting File.";
const WCHAR 	wszErrorFindingCurrentPath[]= L"Error Finding Current Path.";
const WCHAR		wszDefaultSavePrompt[]		= L"Default Save Prompt returned by the provider is: ";

//--------------------------------------------------------------------
// Enums
//--------------------------------------------------------------------
enum EDELETE_FILE
{
	DELETE_YES,
	DELETE_NO
};

#endif 	//_IPERSIST_H_
