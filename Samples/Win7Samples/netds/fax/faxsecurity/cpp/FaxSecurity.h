#ifndef __FAXSECURITY_SAMPLE
//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

#define __FAXSECURITY_SAMPLE

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


#include <lm.h>
#include <lmaccess.h>
#include <Sddl.h>
#include <objbase.h>
#include <activeds.h>
#include <ACLAPI.h>
#include <Mqoai.h>

#define VISTA 6

#define SD_LENGTH 1024
#define SD_LENGTH1 1024
#define MAXDWORD 0xffffffff


BYTE g_bSecurityDescriptor[SD_LENGTH];
PACL g_pDACL;
DWORD g_dwLength;
PSECURITY_DESCRIPTOR* g_pSecurityDescriptor;
PSECURITY_DESCRIPTOR g_pSecurityDescriptorNew;

#endif
