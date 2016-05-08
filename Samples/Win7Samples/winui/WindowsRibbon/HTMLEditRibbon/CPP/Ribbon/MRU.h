// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

#include "../stdafx.h"
#include <UIRibbon.h>
#include <UIRibbonPropertyHelpers.h>

#define RECENT_FILE_COUNT _AFX_MRU_MAX_COUNT

// PURPOSE: Given MFC's Recent Files list, produce the Recent Items list in the Application Menu.
// RETURNS: S_OK when the operation is successful, E_FAIL otherwise.
// NOTE:    The result is stored in the PROPVARIANT pointed to by pvarValue.
HRESULT PopulateRibbonRecentItems(__in CRecentFileList& recentFiles, __deref_out PROPVARIANT* pvarValue);
