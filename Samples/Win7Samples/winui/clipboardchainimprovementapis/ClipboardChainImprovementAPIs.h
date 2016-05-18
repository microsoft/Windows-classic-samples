///////////////////////////////////////////////////////////////////////////////
//
// ClipboardChainImprovementAPIs.h
// This declares functions and used in the clipboard API sample
//
// History:
//      2/9/2006 -- a-erical -- created
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

#define OEMRESOURCE

#include <windows.h>

//
// Shows the clipboard chain improvement APIs in action
//
VOID ClipboardSample(
    __in HWND hWindow);

//
// Adds bitmap data to the clipboard
//
BOOL AddBitmapDataToClipboard(
    __in HWND hWindow);