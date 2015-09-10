// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "FileMapping.h"
#include "Globals.h"

//---------------------------------------------------------------------
//
// ctor
//
//---------------------------------------------------------------------

CFileMapping::CFileMapping() : CFile()
{
    _fileMappingHandle = nullptr;
    _pMapBuffer = nullptr;
}

//---------------------------------------------------------------------
//
// dtor
//
//---------------------------------------------------------------------

CFileMapping::~CFileMapping()
{
    if (_pMapBuffer)
    {
        UnmapViewOfFile(_pMapBuffer);
        _pMapBuffer = nullptr;
        _pReadBuffer = nullptr;
    }
    if (_fileMappingHandle)
    {
        CloseHandle(_fileMappingHandle);
        _fileMappingHandle = nullptr;
    }
}

//---------------------------------------------------------------------
//
// SetupReadBuffer
//
//---------------------------------------------------------------------

BOOL CFileMapping::SetupReadBuffer()
{
    if (_fileSize > sizeof(WCHAR))
    {
        //
        // Read file in file mapping
        //
        _fileMappingHandle = CreateFileMapping(_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
        if (_fileMappingHandle)
        {
            _pMapBuffer = (const WCHAR *)MapViewOfFile(_fileMappingHandle, FILE_MAP_READ, 0, 0, 0);
            if (_pMapBuffer)
            {
                if (IsTextUnicode(_pMapBuffer, (int)_fileSize, NULL))
                {
                    _pReadBuffer = (WCHAR*)_pMapBuffer;

                    // skip Unicode byte order mark
                    if (*((WCHAR*)_pMapBuffer) == Global::UnicodeByteOrderMark)
                    {
                        _pReadBuffer++;
                        _fileSize--;
                    }
                    return TRUE;
                }

                UnmapViewOfFile(_pReadBuffer);
                _pReadBuffer = nullptr;
            }

            CloseHandle(_fileMappingHandle);
            _fileMappingHandle = nullptr;
        }
    }

    return FALSE;
}
