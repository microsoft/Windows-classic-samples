//-----------------------------------------------------------------------
// This file is part of the Windows SDK Code Samples.
// 
// Copyright (C) Microsoft Corporation.  All rights reserved.
// 
// This source code is intended only as a supplement to Microsoft
// Development Tools and/or on-line documentation.  See these other
// materials for detailed information regarding Microsoft code samples.
// 
// THIS CODE AND INFORMATION ARE PROVIDED AS IS WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//-----------------------------------------------------------------------

#include "stdafx.h"
#include <atlbase.h>
#include "xmllite.h"

//implement filestream that derives from IStream
class FileStream : public IStream
{
    FileStream(HANDLE hFile) 
    { 
        _refcount = 1; 
        _hFile = hFile;
    }

    ~FileStream() 
    { 
        if (_hFile != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(_hFile);
        }
    }

public:
    HRESULT static OpenFile(LPCWSTR pName, IStream ** ppStream, bool fWrite)
    {
        HANDLE hFile = ::CreateFileW(pName, fWrite ? GENERIC_WRITE : GENERIC_READ, FILE_SHARE_READ,
            NULL, fWrite ? CREATE_ALWAYS : OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile == INVALID_HANDLE_VALUE)
            return HRESULT_FROM_WIN32(GetLastError());
        
        *ppStream = new FileStream(hFile);
        
        if(*ppStream == NULL)
            CloseHandle(hFile);
            
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject)
    { 
        if (iid == __uuidof(IUnknown)
            || iid == __uuidof(IStream)
            || iid == __uuidof(ISequentialStream))
        {
            *ppvObject = static_cast<IStream*>(this);
            AddRef();
            return S_OK;
        } else
            return E_NOINTERFACE; 
    }

    virtual ULONG STDMETHODCALLTYPE AddRef(void) 
    { 
        return (ULONG)InterlockedIncrement(&_refcount); 
    }

    virtual ULONG STDMETHODCALLTYPE Release(void) 
    {
        ULONG res = (ULONG) InterlockedDecrement(&_refcount);
        if (res == 0) 
            delete this;
        return res;
    }

    // ISequentialStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE Read(void* pv, ULONG cb, ULONG* pcbRead)
    {
        BOOL rc = ReadFile(_hFile, pv, cb, pcbRead, NULL);
        return (rc) ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }

    virtual HRESULT STDMETHODCALLTYPE Write(void const* pv, ULONG cb, ULONG* pcbWritten)
    {
        BOOL rc = WriteFile(_hFile, pv, cb, pcbWritten, NULL);
        return rc ? S_OK : HRESULT_FROM_WIN32(GetLastError());
    }

    // IStream Interface
public:
    virtual HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER)
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE CopyTo(IStream*, ULARGE_INTEGER, ULARGE_INTEGER*,
        ULARGE_INTEGER*) 
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Commit(DWORD)                                      
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Revert(void)                                       
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)              
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD)            
    { 
        return E_NOTIMPL;   
    }
    
    virtual HRESULT STDMETHODCALLTYPE Clone(IStream **)                                  
    { 
        return E_NOTIMPL;   
    }

    virtual HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER liDistanceToMove, DWORD dwOrigin,
        ULARGE_INTEGER* lpNewFilePointer)
    { 
        DWORD dwMoveMethod;

        switch(dwOrigin)
        {
        case STREAM_SEEK_SET:
            dwMoveMethod = FILE_BEGIN;
            break;
        case STREAM_SEEK_CUR:
            dwMoveMethod = FILE_CURRENT;
            break;
        case STREAM_SEEK_END:
            dwMoveMethod = FILE_END;
            break;
        default:   
            return STG_E_INVALIDFUNCTION;
            break;
        }

        if (SetFilePointerEx(_hFile, liDistanceToMove, (PLARGE_INTEGER) lpNewFilePointer,
                             dwMoveMethod) == 0)
            return HRESULT_FROM_WIN32(GetLastError());
        return S_OK;
    }

    virtual HRESULT STDMETHODCALLTYPE Stat(STATSTG* pStatstg, DWORD grfStatFlag) 
    {
        if (GetFileSizeEx(_hFile, (PLARGE_INTEGER) &pStatstg->cbSize) == 0)
            return HRESULT_FROM_WIN32(GetLastError());
        return S_OK;
    }

private:
    HANDLE _hFile;
    LONG _refcount;
};

void dotWhiteSpace(WCHAR* buff, UINT len)
{
    UINT cnt;

    for (cnt = 0; cnt < len; ++cnt)
        if (buff[cnt] == L'\n')
            buff[cnt] = L'.';
}

HRESULT WriteAttributes(IXmlReader* pReader)
{
    const WCHAR* pwszPrefix;
    const WCHAR* pwszLocalName;
    HRESULT hr = pReader->MoveToFirstAttribute();

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"\nXmlLite Error: %08.8lx\n", hr);
        return hr;
    }
    if (S_OK == hr)
    {
        while (TRUE)
        {
            if (!pReader->IsDefault())
            {
                const UINT buffSize = 24;
                WCHAR buff[buffSize];
                UINT charsRead;

                if (FAILED(hr = pReader->GetPrefix(&pwszPrefix, NULL)))
                {
                    wprintf(L"Error, Method: GetPrefix, error is %08.8lx", hr);
                    return -1;
                }
                if (FAILED(hr = pReader->GetLocalName(&pwszLocalName, NULL)))
                {
                    wprintf(L"Error, Method: GetLocalName, error is %08.8lx", hr);
                    return -1;
                }

                while (TRUE)
                {
                    hr = pReader->ReadValueChunk(buff, buffSize - 1, &charsRead);
                    if (S_FALSE == hr || 0 == charsRead)
                        break;
                    if (S_OK != hr)
                    {
                        wprintf(L"\nXmlLite Error: %08.8lx\n", hr);
                        return -1;
                    }
                    buff[charsRead] = L'\0';
                    dotWhiteSpace(buff, charsRead);
                    wprintf(L"attribute chunk size:%d >%s<\n", charsRead, buff);
                }
            }

            if (S_OK != pReader->MoveToNextAttribute())
                break;
        }
    }
    return hr;
}

int _tmain(int argc, WCHAR* argv[])
{
    HRESULT hr;
    CComPtr<IStream> pFileStream;
    CComPtr<IXmlReader> pReader;
    XmlNodeType nodetype;

    if (argc != 2)
    {
        wprintf(L"Usage: XmlLiteChunkReader.exe name-of-input-file\n");
        return 0;
    }
    
    //Open read-only input stream
    if (FAILED(hr = FileStream::OpenFile(argv[1], &pFileStream, FALSE)))
    {
        wprintf(L"Error creating file   reader, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = CreateXmlReader(__uuidof(IXmlReader), (void**) &pReader, NULL)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pReader->SetInput(pFileStream)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx", hr);
        return -1;
    }

    while (TRUE)
    {
        hr = pReader->Read(&nodetype);
        if (S_FALSE == hr)
            break;
        if (S_OK != hr)
        {
            wprintf(L"\nXmlLite Error: %08.8lx\n", hr);
            return -1;
        }
        switch (nodetype)
        {
        case XmlNodeType_Element:
            if (FAILED(hr = WriteAttributes(pReader)))
            {
                wprintf(L"Error, Method: WriteAttributes, error is %08.8lx", hr);
                return -1;
            }
            break;
        case XmlNodeType_Text:
        case XmlNodeType_Whitespace:
            const UINT buffSize = 24;
            WCHAR buff[buffSize];
            UINT charsRead;

            while (TRUE)
            {
                hr = pReader->ReadValueChunk(buff, buffSize - 1, &charsRead);
                if (S_FALSE == hr || 0 == charsRead)
                    break;
                if (S_OK != hr)
                {
                    wprintf(L"\nXmlLite Error: %08.8lx\n", hr);
                    return -1;
                }
                buff[charsRead] = L'\0';
                dotWhiteSpace(buff, charsRead);
                wprintf(L"element chunk size:%d >%s<\n", charsRead, buff);
            }
            break;
        }
    }
    wprintf(L"\n");
    return 0;
}
