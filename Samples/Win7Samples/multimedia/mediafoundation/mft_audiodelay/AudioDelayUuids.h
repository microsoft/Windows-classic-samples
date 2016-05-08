//////////////////////////////////////////////////////////////////////////
//
// AudioDelayUuids.cpp
// Defines GUIDs that are used by the MFT. These can also be used by 
// clients of the MFT.
// 
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

// CLSIDs

// {5B91187F-3C42-409e-A8C9-7F637708D724}
DEFINE_GUID(CLSID_DelayMFT, 
0x5b91187f, 0x3c42, 0x409e, 0xa8, 0xc9, 0x7f, 0x63, 0x77, 0x8, 0xd7, 0x24);


// Attributes

// MF_AUDIODELAY_DELAY_LENGTH: {95915546-B07C-4234-A237-1AF27187DEEE}
// Type: UINT32
// Specifies the length of the delay effect, in milliseconds.
// This attribute must be set before the MFT_MESSAGE_NOTIFY_BEGIN_STREAMING
// message is sent, or before the first call to ProcessInput. 
DEFINE_GUID(MF_AUDIODELAY_DELAY_LENGTH, 
0x95915546, 0xb07c, 0x4234, 0xa2, 0x37, 0x1a, 0xf2, 0x71, 0x87, 0xde, 0xee);


// MF_AUDIODELAY_WET_DRY_MIX: {72127F43-5878-4ea8-8269-D1AF3BB11CB2}
// Type: UINT32
// Specifies the wet/dry mix. (Range: 0 - 100. 0 = no delay, 100 = all delay)
// This attribute can be set before each call to ProcessOutput()
DEFINE_GUID(MF_AUDIODELAY_WET_DRY_MIX, 
0x72127f43, 0x5878, 0x4ea8, 0x82, 0x69, 0xd1, 0xaf, 0x3b, 0xb1, 0x1c, 0xb2);

