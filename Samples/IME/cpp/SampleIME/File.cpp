// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "File.h"
#include "SampleIMEBaseStructure.h"

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CFile::CFile(UINT codePage)
{
    _codePage = codePage;
    _fileHandle = nullptr;
    _pReadBuffer = nullptr;
    _fileSize = 0;
    _filePosPointer = 0;
    _pFileName = nullptr;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CFile::~CFile()
{
    if (_pReadBuffer)
    {
        delete [] _pReadBuffer;
        _pReadBuffer = nullptr;
    }
    if (_fileHandle)
    {
        CloseHandle(_fileHandle);
        _fileHandle = nullptr;
    }
    if (_pFileName)
    {
        delete [] _pFileName;
        _pFileName = nullptr;
    }
}

//---------------------------------------------------------------------
//
// CreateFile
//
//---------------------------------------------------------------------

BOOL CFile::CreateFile(_In_ PCWSTR pFileName, DWORD desiredAccess,
    DWORD creationDisposition,
    DWORD sharedMode, _Inout_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD flagsAndAttributes, _Inout_opt_ HANDLE templateFileHandle)
{
    size_t fullPathLen = wcslen(pFileName);
    if (!_pFileName)
    {
        _pFileName = new (std::nothrow) WCHAR[ fullPathLen + 1 ];
    }
    if (!_pFileName)
    {
        return FALSE;
    }

    StringCchCopyN(_pFileName, fullPathLen + 1, pFileName, fullPathLen);

    _fileHandle = ::CreateFile(pFileName, desiredAccess, sharedMode,
        lpSecurityAttributes, creationDisposition, flagsAndAttributes, templateFileHandle);

    if (_fileHandle == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    _fileSize = ::GetFileSize(_fileHandle, NULL);

    return TRUE;
}

//---------------------------------------------------------------------
//
// SetupReadBuffer
//
//---------------------------------------------------------------------

BOOL CFile::SetupReadBuffer()
{
    const WCHAR* pWideBuffer = nullptr;

    _pReadBuffer = (const WCHAR *) new (std::nothrow) BYTE[ _fileSize ];
    if (!_pReadBuffer)
    {
        return FALSE;
    }

    DWORD dwNumberOfByteRead = 0;
    if (!ReadFile(_fileHandle, (LPVOID)_pReadBuffer, (DWORD)_fileSize, &dwNumberOfByteRead, NULL))
    {
        delete [] _pReadBuffer;
        _pReadBuffer = nullptr;
        return FALSE;
    }

    if (!IsTextUnicode(_pReadBuffer, dwNumberOfByteRead, NULL))
    {
        // This is ASCII file.
        // Read file with Unicode conversion.
        int wideLength = 0;

        wideLength = MultiByteToWideChar(_codePage, 0, (LPCSTR)_pReadBuffer, dwNumberOfByteRead, NULL, 0);
        if (wideLength <= 0)
        {
            delete [] _pReadBuffer;
            _pReadBuffer = nullptr;
            return FALSE;
        }

        pWideBuffer = new (std::nothrow) WCHAR[ wideLength ];
        if (!pWideBuffer)
        {
            delete [] _pReadBuffer;
            _pReadBuffer = nullptr;
            return FALSE;
        }

        wideLength = MultiByteToWideChar(_codePage, 0, (LPCSTR)_pReadBuffer, (DWORD)_fileSize, (LPWSTR)pWideBuffer, wideLength);
        if (wideLength <= 0)
        {
            delete [] pWideBuffer;
            delete [] _pReadBuffer;
            _pReadBuffer = nullptr;
            return FALSE;
        }

        _fileSize = wideLength * sizeof(WCHAR);
        delete [] _pReadBuffer;
        _pReadBuffer = pWideBuffer;
    }
    else if (_fileSize > sizeof(WCHAR))
    {
        // Read file in allocated buffer
        pWideBuffer = new (std::nothrow) WCHAR[ _fileSize/sizeof(WCHAR) - 1 ];
        if (!pWideBuffer)
        {
            delete [] _pReadBuffer;
            _pReadBuffer = nullptr;
            return FALSE;
        }

        // skip unicode byte-order signature
        SetFilePointer(_fileHandle, sizeof(WCHAR), NULL, FILE_BEGIN);

        if (!ReadFile(_fileHandle, (LPVOID)pWideBuffer, (DWORD)(_fileSize - sizeof(WCHAR)), &dwNumberOfByteRead, NULL))
        {
            delete [] pWideBuffer;
            delete [] _pReadBuffer;
            _pReadBuffer = nullptr;
            return FALSE;
        }

        _fileSize -= sizeof(WCHAR);
        delete [] _pReadBuffer;
        _pReadBuffer = pWideBuffer;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// IsEndOfFile
//
//----------------------------------------------------------------------------

BOOL CFile::IsEndOfFile()
{
    return _fileSize == _filePosPointer ? TRUE : FALSE;
}

//+---------------------------------------------------------------------------
//
// NextLine
//
//----------------------------------------------------------------------------

VOID CFile::NextLine()
{
    DWORD_PTR totalBufLen = GetBufferInWCharLength();
    if (totalBufLen == 0)
    {
        goto SetEOF;
    }
    const WCHAR *pwch = GetBufferInWChar();

    DWORD_PTR indexTrace = 0;       // in char

    if (FindChar(L'\r', pwch, totalBufLen, &indexTrace) != S_OK)
    {
        goto SetEOF;
    }
    if (indexTrace >= DWORD_MAX -1)
    {
        goto SetEOF;
    }

    indexTrace++;  // skip CR
    totalBufLen -= indexTrace;
    if (totalBufLen == 0)
    {
        goto SetEOF;
    }

    if (pwch[indexTrace] != L'\n')
    {
        _filePosPointer += (indexTrace * sizeof(WCHAR));
        return;
    }

    indexTrace++;
    totalBufLen--;
    if (totalBufLen == 0)
    {
        goto SetEOF;
    }

    _filePosPointer += (indexTrace * sizeof(WCHAR));

    return;

SetEOF:
    _filePosPointer = _fileSize;
    return;
}
