// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef _ERROR_OUT
#define _ERROR_OUT

void OutputError(HRESULT hr, LPCSTR operation);

#define	ERROR_LENGTH	25
#define ERROR_MAX		256
#define	IDS_SAFE_PRINTF	"safe string printf"
#define IDS_SAFE_COPY	"safe string copy"
#define IDS_SAFE_CAT	"safe string concatenation"

#endif