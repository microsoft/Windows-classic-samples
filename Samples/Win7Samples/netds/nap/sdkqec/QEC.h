// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef __Qec_H__
#define __Qec_H__

#pragma once


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlbase.h>
#include <Naptypes.h>
#include <naperror.h>
#include "QecCallback.h"
#include "SdkQecModule.h"
#include <Napenforcementclient.h>

class AtlExeModule : public CAtlExeModuleT<AtlExeModule>
{
};

AtlExeModule * exeModule = new AtlExeModule;

namespace SDK_SAMPLE_QEC
{
    enum QecActionCode {NOACTION, DOREGISTER, DOUNREGISTER, DOEXECUTE};

    static const wchar_t QecActionCode_Register[] = L"/register";
    static const wchar_t QecActionCode_Unregister[] = L"/unregister";
    static const wchar_t QecActionCode_Execute[] = L"/execute";
}

#endif
