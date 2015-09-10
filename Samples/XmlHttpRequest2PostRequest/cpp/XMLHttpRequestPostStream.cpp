//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved
//

#include "XMLHttpRequestPostStream.h"

CXMLHttpRequestPostStream::CXMLHttpRequestPostStream()
{
    m_hFile = INVALID_HANDLE_VALUE;
}

CXMLHttpRequestPostStream::~CXMLHttpRequestPostStream()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

STDMETHODIMP
CXMLHttpRequestPostStream::Open(
    _In_opt_ PCWSTR pcwszFileName
)
/*++

Routine Description:

    Initalize the post stream instance.

Arguments:

    pcwszFileName - The file name of the data to be posted to server.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    if (pcwszFileName == NULL ||
        *pcwszFileName == L'\0')
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    hFile = CreateFile(pcwszFileName,
                       GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    m_hFile = hFile;
    hFile = INVALID_HANDLE_VALUE;

Exit:

    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
        hFile = INVALID_HANDLE_VALUE;
    }

    return hr;
}

STDMETHODIMP
CXMLHttpRequestPostStream::Read(
    _Out_writes_bytes_to_(cb, *pcbRead)  void *pv,
    ULONG cb,
    _Out_opt_  ULONG *pcbRead
)
/*++

Routine Description:

    Reads a specified number of bytes from the stream object into memory,
    starting at the current seek pointer.

Arguments:

    pv - Supplies a pointer to receive the stream data.

    cb - The number of bytes of data to read from the stream object.

    pcbRead - Supplies a pointer to receive the actual number of bytes read
              from the stream object.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    BOOL fSuccess = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbRead = 0;

    if (pcbRead != NULL)
    {
        *pcbRead = 0;
    }

    if (pv == NULL ||
        cb == 0)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    fSuccess = ReadFile(m_hFile, pv, cb, &cbRead, NULL);
    if (!fSuccess)
    {
        dwError = GetLastError();

        if (dwError != ERROR_HANDLE_EOF)
        {
            hr = HRESULT_FROM_WIN32(dwError);
            goto Exit;
        }
    }

    if (cbRead < cb)
    {
        hr = S_FALSE;
    }

    if (pcbRead != NULL)
    {
        *pcbRead = cbRead;
    }

Exit:

    return hr;
}

STDMETHODIMP
CXMLHttpRequestPostStream::Write(
    _In_reads_bytes_(cb)  const void *pv,
    ULONG cb,
    _Out_opt_  ULONG *pcbWritten
)
/*++

Routine Description:

    Writes a specified number of bytes into the stream object starting at the
    current seek pointer. It's unnecessary to implement for the post stream.

Arguments:

    pv - A pointer to the buffer that contains the data that is to be

    cb - The number of bytes of data to attempt to write into the stream.

    pcbWritten - The actual number of bytes written to the stream object.

Return Value:

    HRESULT.

--*/
{
    UNREFERENCED_PARAMETER(pv);
    UNREFERENCED_PARAMETER(cb);
    UNREFERENCED_PARAMETER(pcbWritten);
    return STG_E_ACCESSDENIED;
}

STDMETHODIMP
CXMLHttpRequestPostStream::GetSize(
    _Out_ ULONGLONG *pullSize
)
/*++

Routine Description:

    Gets the size in bytes of the post data file.

Arguments:

    pullSize - The number of bytes in the file.

Return Value:

    HRESULT.

--*/
{
    HRESULT hr = S_OK;
    BOOL fSuccess = FALSE;
    LARGE_INTEGER liFileSize = {};

    if (pullSize == NULL)
    {
        hr = E_INVALIDARG;
        goto Exit;
    }

    *pullSize = 0;

    fSuccess = GetFileSizeEx(m_hFile,
                             &liFileSize);

    if (!fSuccess)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto Exit;
    }

    *pullSize = (ULONGLONG)liFileSize.QuadPart;

Exit:

    return hr;
}
