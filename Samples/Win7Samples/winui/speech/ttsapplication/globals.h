// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright © Microsoft Corporation. All rights reserved

#ifndef __TTSAPP_GLOBALS_H__
#define __TTSAPP_GLOBALS_H__

// App includes
#include "TtsApplication.h"         


// Other global variables
extern int                  g_iBmp;
extern HIMAGELIST           g_hListBmp;
extern const int            g_aMapVisemeToImage[22];

// Output formats
extern const SPSTREAMFORMAT g_aOutputFormat[NUM_OUTPUTFORMATS];
extern TCHAR*               g_aszOutputFormat[NUM_OUTPUTFORMATS];

  
#endif // __TTSAPP_GLOBALS_H__
