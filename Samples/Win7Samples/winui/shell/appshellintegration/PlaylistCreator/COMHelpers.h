// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
#include <objbase.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <strsafe.h>

HRESULT IStream_WriteString(IStream *pstm, PCSTR pszBuf);
HRESULT IStream_WriteString(IStream *pstm, PCWSTR pszBuf);
HRESULT IStream_WriteStringAsUTF8(IStream *pstm, PCWSTR pszBuf);
HRESULT IStream_CchPrintfAsUTF8(IStream *pstm, PCWSTR pszKeyFormatString, ...);
HRESULT IStream_ReadToBuffer(IStream *pstm, UINT uMaxSize,
                             BYTE **ppBytes, UINT *pcBytes);

HRESULT OpenFolderAndSelectItem(IShellItem *psi);
HRESULT GetUNCPathFromItem(IShellItem *psi, PWSTR *ppszUNC);
