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

#include <atlbase.h>
#include <xmllite.h>

HRESULT WriteAttributes(IXmlReader* xmlReader)
{
    PCWSTR pwszPrefix;
    PCWSTR pwszLocalName;
    PCWSTR pwszValue;
    HRESULT hr = xmlReader->MoveToFirstAttribute();

    if (S_FALSE == hr)
        return hr;
    if (S_OK != hr)
    {
        wprintf(L"Error moving to first attribute, error is %08.8lx", hr);
        return -1;
    }
    if (S_OK == hr)
    {
        for (;;)
        {
            if (!xmlReader->IsDefault())
            {
                UINT cwchPrefix;
                if (FAILED(hr = xmlReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
                {
                    wprintf(L"Error getting prefix, error is %08.8lx", hr);
                    return hr;
                }
                if (FAILED(hr = xmlReader->GetLocalName(&pwszLocalName, nullptr)))
                {
                    wprintf(L"Error getting local name, error is %08.8lx", hr);
                    return hr;
                }
                if (FAILED(hr = xmlReader->GetValue(&pwszValue, nullptr)))
                {
                    wprintf(L"Error getting value, error is %08.8lx", hr);
                    return hr;
                }
                if (cwchPrefix > 0)
                    wprintf(L"%s:%s=\"%s\" ", pwszPrefix, pwszLocalName, pwszValue);
                else
                    wprintf(L"%s=\"%s\" ", pwszLocalName, pwszValue);
            }

            if (S_OK != xmlReader->MoveToNextAttribute())
                break;
        }
    }
    return hr;
}

int wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    CComPtr<IStream> fileStream;
    CComPtr<IXmlReader> xmlReader;
    CComPtr<IXmlReaderInput> xmlReaderInput;
    XmlNodeType nodetype;
    PCWSTR pwszPrefix;
    PCWSTR pwszLocalName;
    PCWSTR pwszValue;
    UINT cwchPrefix;

    PCWSTR file = (argc == 2) ? argv[1] : L"encoded.xml";

    // Open read-only input stream
    HRESULT hr = SHCreateStreamOnFileW(file, STGM_READ, &fileStream);
    if (FAILED(hr))
    {
        wprintf(L"Error creating file reader, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = CreateXmlReader(IID_PPV_ARGS(&xmlReader), nullptr)))
    {
        wprintf(L"Error creating xml reader, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = CreateXmlReaderInputWithEncodingName(fileStream, nullptr, L"utf-16", FALSE,
        L"c:\temp", &xmlReaderInput)))
    {
        wprintf(L"Error creating xml reader with encoding code page, error is %08.8lx", hr);
        return -1;
    }

    if (FAILED(hr = xmlReader->SetInput(xmlReaderInput)))
    {
        wprintf(L"Error setting input for reader, error is %08.8lx", hr);
        return -1;
    }

    for (;;)
    {
        hr = xmlReader->Read(&nodetype);
        if (S_FALSE == hr)
            break;
        if (S_OK != hr)
        {
            wprintf(L"\nXmlLite Error: %08.8lx\n", hr);
            return -1;
        }
        switch (nodetype)
        {
        case XmlNodeType_XmlDeclaration:
            wprintf(L"<?xml ");
            if (FAILED(hr = WriteAttributes(xmlReader)))
            {
                wprintf(L"Error, Method: WriteAttributes, error is %08.8lx", hr);
                return -1;
            }
            wprintf(L"?>\r\n");
            break;
        case XmlNodeType_Element:
            if (FAILED(hr = xmlReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
            {
                wprintf(L"Error, Method: GetPrefix, error is %08.8lx", hr);
                return -1;
            }
            if (FAILED(hr = xmlReader->GetLocalName(&pwszLocalName, nullptr)))
            {
                wprintf(L"Error, Method: GetLocalName, error is %08.8lx", hr);
                return -1;
            }
            if (cwchPrefix > 0)
                wprintf(L"<%s:%s ", pwszPrefix, pwszLocalName);
            else
                wprintf(L"<%s ", pwszLocalName);

            if (FAILED(hr = WriteAttributes(xmlReader)))
            {
                wprintf(L"Error, Method: WriteAttributes, error is %08.8lx", hr);
                return -1;
            }

            if (xmlReader->IsEmptyElement())
                wprintf(L"/>");
            else
                wprintf(L">");
            break;
        case XmlNodeType_EndElement:
            if (FAILED(hr = xmlReader->GetPrefix(&pwszPrefix, &cwchPrefix)))
            {
                wprintf(L"Error, Method: GetPrefix, error is %08.8lx", hr);
                return -1;
            }
            if (FAILED(hr = xmlReader->GetLocalName(&pwszLocalName, nullptr)))
            {
                wprintf(L"Error, Method: GetLocalName, error is %08.8lx", hr);
                return -1;
            }
            if (cwchPrefix > 0)
                wprintf(L"</%s:%s>", pwszPrefix, pwszLocalName);
            else
                wprintf(L"</%s>", pwszLocalName);
            break;
        case XmlNodeType_Text:
        case XmlNodeType_Whitespace:
            if (FAILED(hr = xmlReader->GetValue(&pwszValue, nullptr)))
            {
                wprintf(L"Error, Method: GetValue, error is %08.8lx", hr);
                return -1;
            }
            wprintf(L"%s", pwszValue);
            break;
        case XmlNodeType_CDATA:
            if (FAILED(hr = xmlReader->GetValue(&pwszValue, nullptr)))
            {
                wprintf(L"Error, Method: GetValue, error is %08.8lx", hr);
                return -1;
            }
            wprintf(L"<![CDATA[%s]]>", pwszValue);
            break;
        case XmlNodeType_ProcessingInstruction:
            if (FAILED(hr = xmlReader->GetValue(&pwszValue, nullptr)))
            {
                wprintf(L"Error, Method: GetValue, error is %08.8lx", hr);
                return -1;
            }
            wprintf(L"<?%s?>", pwszValue);
            break;
        case XmlNodeType_Comment:
            if (FAILED(hr = xmlReader->GetValue(&pwszValue, nullptr)))
            {
                wprintf(L"Error, Method: GetValue, error is %08.8lx", hr);
                return -1;
            }
            wprintf(L"<!--%s-->", pwszValue);
            break;
        case XmlNodeType_DocumentType:
            wprintf(L"<!-- DOCTYPE is not printed -->\r\n");
            break;
        }
    }
    wprintf(L"\n");
    return 0;
}
