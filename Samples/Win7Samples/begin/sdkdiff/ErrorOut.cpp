// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// simple function for outputting an error, given a string and error code.
#include "precomp.h"

void OutputError(HRESULT hr, LPCSTR operation) {
	TCHAR error_string[ERROR_MAX];
	TCHAR error_code[ERROR_LENGTH];

	memset(error_string, 0, ERROR_MAX);
	memset(error_code, 0, ERROR_LENGTH);

	switch(hr) {
		case STRSAFE_E_INSUFFICIENT_BUFFER:
			if (FAILED(StringCchCopy(error_code, ERROR_LENGTH, "insufficient buffer"))) 
				return;
			break;
		case STRSAFE_E_INVALID_PARAMETER:
			if (FAILED(StringCchCopy(error_code, ERROR_LENGTH, "invalid parameter")))
				return;
			break;
		case STRSAFE_E_END_OF_FILE:
			if (FAILED(StringCchCopy(error_code, ERROR_LENGTH, "encountering end of file")))
				return;
			break;
	}

	if (0 != error_code[0]) {
		if (FAILED(StringCchPrintf(error_string, ERROR_MAX, "Unable to perform %s due to %s.\n", operation, error_code)))
			return;
	}
	else 
		if (FAILED(StringCchPrintf(error_string, ERROR_MAX, "Unable to perform %s.\n", operation)))
			return;

	OutputDebugString(error_string);
	return;
}
