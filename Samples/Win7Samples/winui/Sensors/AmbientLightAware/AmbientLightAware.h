//-----------------------------------------------------------------------------
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation.  All rights reserved.
//
// Module:
//        AmbientLightAware.h
//
// Description:
//        Dialog for Ambient Light Aware SDK Sample
//
// Comments: 
//        Standard vc++ dialog created by VS 2005 wizard.
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __AFXWIN_H__
#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


// CAmbientLightAwareApp:
// See AmbientLightAware.cpp for the implementation of this class
//

class CAmbientLightAwareApp : public CWinApp
{
public:
    CAmbientLightAwareApp();

    // Overrides
public:
    virtual BOOL InitInstance();

    // Implementation

    DECLARE_MESSAGE_MAP()
};

extern CAmbientLightAwareApp theApp;