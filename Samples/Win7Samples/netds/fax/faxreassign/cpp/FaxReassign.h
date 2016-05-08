#ifndef __COM_REASSIGN_SAMPLE
//==========================================================================
//
//  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
//  KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
//  PURPOSE.
//
//  Copyright 1998 - 2000 Microsoft Corporation.  All Rights Reserved.
//
//--------------------------------------------------------------------------

#define __COM_REASSIGN_SAMPLE

//
//Includes
//
#include <faxcomex.h>
#include <windows.h>
#include <winbase.h>
#include <stdlib.h>
#include <objbase.h>
#include <tchar.h>
#include <assert.h>
#include <shellapi.h>
#include <strsafe.h>


#define RECIPIENT_SIZE 4096
#define NUM_MSGS 100
#define ARR_SIZE(a) (sizeof(a)/sizeof(a[0]))
#define SENDER_NAME _T("ReassignAdmin")
#define SENDER_FAXNUMBER _T("1234")
#define SUBJECT _T("Reassigned Fax")
#define VISTA 6

#endif