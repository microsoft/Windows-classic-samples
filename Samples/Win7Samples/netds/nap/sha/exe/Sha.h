// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.


#ifndef __Sha_H__
#define __Sha_H__

#pragma once


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <atlbase.h>
#include "SdkShaModule.h"



class AtlExeModule : public CAtlExeModuleT<AtlExeModule>
{
};

AtlExeModule * exeModule = new AtlExeModule;

namespace SDK_SAMPLE_SHA
{
    enum ShaActionCode {NOACTION, DOREGISTER, DOUNREGISTER, DOEXECUTE};

    static const wchar_t ShaActionCode_Register[] = L"/register";
    static const wchar_t ShaActionCode_Unregister[] = L"/unregister";
    static const wchar_t ShaActionCode_Execute[] = L"/execute";
}

#endif
