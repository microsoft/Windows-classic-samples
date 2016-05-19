// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// COMRTS.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__

#endif

#include "resource.h"		// main symbols
#include "COMRTS_i.h"


// CCOMRTSApp:
// See COMRTS.cpp for the implementation of this class


class CCOMRTSApp : public CWinApp
{
public:
	CCOMRTSApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
public:
	 virtual int ExitInstance(void);

};

extern CCOMRTSApp theApp;