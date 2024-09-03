/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    SampleProvider.h

Abstract:

    This h file includes sample code for a simple nspv2 naming 
    provider.

--********************************************************************/
#pragma once

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#include "Reg.h"
#include "install.h"
#include "utils.h"

#ifndef celems
#define celems(x) (sizeof(x)/sizeof((x)[0]))
#endif
