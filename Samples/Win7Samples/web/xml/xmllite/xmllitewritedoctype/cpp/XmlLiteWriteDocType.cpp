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

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr;
    CComPtr<IStream> pOutFileStream;
    CComPtr<IXmlWriter> pWriter;

    if (argc != 2)
    {
        wprintf(L"Usage: XmlLiteWriteDocType.exe name-of-output-file\n");
        return 0;
    }

    //Open writeable output stream
    if (FAILED(hr = FileStream::OpenFile(argv[1], &pOutFileStream, TRUE)))
    {
        wprintf(L"Error creating file writer, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = CreateXmlWriter(__uuidof(IXmlWriter), (void**) &pWriter, NULL)))
    {
        wprintf(L"Error creating xml writer, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->SetOutput(pOutFileStream)))
    {
        wprintf(L"Error, Method: SetOutput, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->SetProperty(XmlWriterProperty_Indent, TRUE)))
    {
        wprintf(L"Error, Method: SetProperty XmlWriterProperty_Indent, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartDocument(XmlStandalone_Omit)))
    {
        wprintf(L"Error, Method: WriteStartDocument, error is %08.8lx", hr);
        return -1;
    }

    const WCHAR * name = L"Employees";
    const WCHAR * pubid = NULL;
    const WCHAR * sysid = NULL;
    const WCHAR * subset =
    L"<!ELEMENT Employees (Employee)+>\n"
    L"<!ELEMENT Employee EMPTY>\n"
    L"<!ATTLIST Employee firstname CDATA #REQUIRED>\n"
    L"<!ENTITY Company 'Microsoft'>\n";

    if (FAILED(hr = pWriter->WriteDocType(name, pubid, sysid, subset)))
    {
        wprintf(L"Error, Method: WriteDocType, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteProcessingInstruction(L"xml-stylesheet",
        L"href=\"mystyle.css\" title=\"Compact\" type=\"text/css\"")))
    {
        wprintf(L"Error, Method: WriteProcessingInstruction, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"root", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"sub", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteAttributeString(NULL, L"myAttr", NULL, L"1234")))
    {
        wprintf(L"Error, Method: WriteAttributeString, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteString(L"Markup is <escaped> for this string")))
    {
        wprintf(L"Error, Method: WriteString, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteFullEndElement()))
    {
        wprintf(L"Error, Method: WriteFullEndElement, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"anotherChild", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteString(L"some text")))
    {
        wprintf(L"Error, Method: WriteString, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteFullEndElement()))
    {
        wprintf(L"Error, Method: WriteFullEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteCData(L"This is CDATA text.")))
    {
        wprintf(L"Error, Method: WriteCData, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"containsCharacterEntity", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteCharEntity(L'a')))
    {
        wprintf(L"Error, Method: WriteCharEntity, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteFullEndElement()))
    {
        wprintf(L"Error, Method: WriteFullEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"containsChars", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteChars(L"abcdefghijklm", 5)))
    {
        wprintf(L"Error, Method: WriteChars, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteFullEndElement()))
    {
        wprintf(L"Error, Method: WriteFullEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"containsEntity", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteEntityRef(L"myEntity")))
    {
        wprintf(L"Error, Method: WriteEntityRef, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteEndElement()))
    {
        wprintf(L"Error, Method: WriteEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"containsName", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteName(L"myName")))
    {
        wprintf(L"Error, Method: WriteName, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteEndElement()))
    {
        wprintf(L"Error, Method: WriteEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteStartElement(NULL, L"containsNmToken", NULL)))
    {
        wprintf(L"Error, Method: WriteStartElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteNmToken(L"myNmToken")))
    {
        wprintf(L"Error, Method: WriteNmToken, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteEndElement()))
    {
        wprintf(L"Error, Method: WriteEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteComment(L"This is a comment")))
    {
        wprintf(L"Error, Method: WriteComment, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteRaw(L"<elementWrittenRaw/>")))
    {
        wprintf(L"Error, Method: WriteRaw, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteRawChars(L"<rawCharacters/>", 16)))
    {
        wprintf(L"Error, Method: WriteRawChars, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = pWriter->WriteElementString(NULL, L"myElement", NULL, L"myValue")))
    {
        wprintf(L"Error, Method: WriteElementString, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteFullEndElement()))
    {
        wprintf(L"Error, Method: WriteFullEndElement, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->WriteWhitespace(L"\n")))
    {
        wprintf(L"Error, Method: WriteWhitespace, error is %08.8lx", hr);
        return -1;
    }

    // WriteEndDocument closes any open elements or attributes and reinitializes
    // the writer so that a new document can be written.
    if (FAILED(hr = pWriter->WriteEndDocument()))
    {
        wprintf(L"Error, Method: WriteEndDocument, error is %08.8lx", hr);
        return -1;
    }
    if (FAILED(hr = pWriter->Flush()))
    {
        wprintf(L"Error, Method: Flush, error is %08.8lx", hr);
        return -1;
    }

    return 0;

}
