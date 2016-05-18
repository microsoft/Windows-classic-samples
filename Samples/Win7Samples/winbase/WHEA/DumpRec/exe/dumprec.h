// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

/*++

Module Name:

    dumprec.h

Abstract:

    This module contains the shared definitions and declarations used in the
    Dumprec SDK sample.

--*/

#pragma once

#pragma warning(push)
#pragma warning(disable:4201)
#pragma warning(disable:4214)

#include <wchar.h>
#include <windows.h>
#include <winevt.h>
#include <wheadef.h>

#pragma warning(pop)

#include <cperhlp.h>
#include "dumprecs.h"

//
// This macro evaluates to TRUE if the typed pointer refers to a structure that
// is completely contained within the specified range.
//

#define CONTAINED(RangeStart, RangeSize, Structure, StructureSize) \
    (((PUCHAR)(Structure)) >= ((PUCHAR)(RangeStart)) && \
     ((((PUCHAR)(Structure)) + (StructureSize)) <= (((PUCHAR)(RangeStart)) + (RangeSize))))

#define TYPE_CONTAINED(RangeStart, RangeSize, Structure) \
    CONTAINED(RangeStart, RangeSize, Structure, sizeof(*(Structure)))

