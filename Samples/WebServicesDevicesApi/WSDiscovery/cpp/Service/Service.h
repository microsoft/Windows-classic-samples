//////////////////////////////////////////////////////////////////////////////
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <wsdapi.h>

_Success_( return == S_OK )
HRESULT ParseArguments
(   _In_ int argc
,   _In_reads_( argc ) LPWSTR *argv
,   _Outptr_result_maybenull_ LPWSTR *epr
,   _Outptr_result_maybenull_ WSD_URI_LIST **scopesList
);

void DisplayUsages();
