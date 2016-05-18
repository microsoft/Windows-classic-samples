// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//
// PropertyStore.h/cpp implement the utility functions that are used for interacting 
// with the font control. Here you can find utility functions for converting IPropertyStore interface
// to and from CHARFORMAT2 to format the text displayed in the RichEdit control.
//

#pragma once

#include <UIRibbon.h>
#include <Richedit.h>

#define TWIPS_PER_POINT 20 // For setting font size in CHARFORMAT2.

// Convert CHARFORMAT2 structure to and from IPropertyStore.
void GetCharFormat2FromIPropertyStore(__in IPropertyStore* pPropStore, __out CHARFORMAT2 *pCharFormat);
void GetIPropStoreFromCharFormat2(const __in CHARFORMAT2* pCharFormat, __in IPropertyStore *pPropStore);

