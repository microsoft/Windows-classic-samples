///////////////////////////////////////////////////////////////////////////////
//
// DRMSampleUtils.cpp : Contains implementation of common functions for 
//  DRM samples.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
///////////////////////////////////////////////////////////////////////////////
#include "DRMSampleUtils.h"

///////////////////////////////////////////////////////////////////////////////
//
// Function: DisplayError
// Description: Displays a specified error string.
// Parameters: ErrorCode - HRESULT error code.
//             pwszMessage - Message to display.
// Notes: This function is proveded so that chages can be made to the way 
//  errors are reported without an impact on the reporting code.
//
///////////////////////////////////////////////////////////////////////////////
void DisplayError(HRESULT ErrorCode, const wchar_t* pwszMessage)
{
    wprintf(L"%s\nError Code = 0x%08X\n", pwszMessage, ErrorCode);
}