//------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//------------------------------------------------------------

#ifndef UNICODE
#define UNICODE
#endif
#include "WebServices.h"
#include "process.h"
#include "stdio.h"
#include "string.h"

// Print out rich error info
void PrintError(
    __in HRESULT errorCode, 
    __in_opt WS_ERROR* error)
{
    wprintf(L"Failure: errorCode=0x%lx\n", errorCode);

    if (errorCode == E_INVALIDARG || errorCode == WS_E_INVALID_OPERATION)
    {
        // Correct use of the APIs should never generate these errors
        wprintf(L"The error was due to an invalid use of an API.  This is likely due to a bug in the program.\n");
        DebugBreak();
    }

    HRESULT hr = S_OK;
    if (error != NULL)
    {
        ULONG errorCount;
        hr = WsGetErrorProperty(error, WS_ERROR_PROPERTY_STRING_COUNT, &errorCount, sizeof(errorCount));
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < errorCount; i++)
        {
            WS_STRING string;
            hr = WsGetErrorString(error, i, &string);
            if (FAILED(hr))
            {
                goto Exit;
            }
            wprintf(L"%.*s\n", string.length, string.chars);
        }
    }
Exit:
    if (FAILED(hr))
    {
        wprintf(L"Could not get error string (errorCode=0x%lx)\n", hr);
    }
}

void PrintString(
    __in const WS_XML_STRING* string)
{
    if (string->length == 0)
    {
        return;
    }

    WCHAR* chars = NULL;
    int charCount = 0;

#pragma prefast(suppress:26035, "The string is not null-terminated because lenght of the string is also passed to the function.")
    charCount = MultiByteToWideChar(
        CP_UTF8, 
        MB_ERR_INVALID_CHARS, 
        (char*)string->bytes, 
        string->length,
        NULL, 
        0);
    if (charCount == 0)
    {
        printf("<error: MultiByteToWideChar failed>");
        goto Exit;
    }

    chars = (WCHAR*)HeapAlloc(
        GetProcessHeap(), 
        0, 
        charCount * sizeof(WCHAR));

    if (chars == NULL)
    {
        printf("<error: HeapAlloc failed>");
        goto Exit;
    }

    if (MultiByteToWideChar(
        CP_UTF8,
        MB_ERR_INVALID_CHARS, 
        (char*)string->bytes, string->length,
        chars, charCount) != charCount)
    {
        printf("<error: MultiByteToWideChar failed>");
        goto Exit;
    }

    printf("%.*S", charCount, chars);

Exit:
    if (chars != NULL)
    {
        HeapFree(GetProcessHeap(), 0, chars);
    }
}

void PrintText(
    __in const WS_XML_TEXT* text)
{
    switch (text->textType)
    {
        case WS_XML_TEXT_TYPE_UTF8:
            {
                const WS_XML_UTF8_TEXT* utf8Text = (const WS_XML_UTF8_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_UTF8(value='");
                PrintString(&utf8Text->value);
                printf("')");
            }
            break;
        case WS_XML_TEXT_TYPE_UTF16:
            {
                const WS_XML_UTF16_TEXT* utf16Text = (const WS_XML_UTF16_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_UTF16(value='%.*S')", utf16Text->byteCount / sizeof(WCHAR), (WCHAR*)utf16Text->bytes);
            }
            break;
        case WS_XML_TEXT_TYPE_BASE64:
            {
                const WS_XML_BASE64_TEXT* base64Text = (const WS_XML_BASE64_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_BASE64(value={ ");
                for (ULONG i = 0; i < base64Text->length; i++)
                {
                    printf("%02x", base64Text->bytes[i]);
                }
                printf(" })");
            }
            break;
        case WS_XML_TEXT_TYPE_BOOL:
            {
                const WS_XML_BOOL_TEXT* boolText = (const WS_XML_BOOL_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_BOOL(value='%s')", boolText->value ? "true" : "false");
            }
            break;
        case WS_XML_TEXT_TYPE_INT32:
            {
                const WS_XML_INT32_TEXT* int32Text = (const WS_XML_INT32_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_INT32(value='%d')", int32Text->value);
            }
            break;
        case WS_XML_TEXT_TYPE_INT64:
            {
                const WS_XML_INT64_TEXT* int64Text = (const WS_XML_INT64_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_INT64(value='%I64d')", int64Text->value);
            }
            break;
        case WS_XML_TEXT_TYPE_UINT64:
            {
                const WS_XML_UINT64_TEXT* uint64Text = (const WS_XML_UINT64_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_UINT64(value='%I64u')", uint64Text->value);
            }
            break;
        case WS_XML_TEXT_TYPE_FLOAT:
            {
                const WS_XML_FLOAT_TEXT* floatText = (const WS_XML_FLOAT_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_FLOAT(value='%f')", floatText->value);
            }
            break;
        case WS_XML_TEXT_TYPE_DOUBLE:
            {
                const WS_XML_DOUBLE_TEXT* doubleText = (const WS_XML_DOUBLE_TEXT*) text;
                printf("WS_XML_TEXT_TYPE_DOUBLE(value='%f')", doubleText->value);
            }
            break;
        case WS_XML_TEXT_TYPE_DECIMAL:
            {
                const WS_XML_DECIMAL_TEXT* decimalText = (const WS_XML_DECIMAL_TEXT*) text;
                const DECIMAL* dec = &decimalText->value;
                printf("WS_XML_TEXT_TYPE_DECIMAL(value={%x %x %x, %I64x})", dec->wReserved, dec->signscale, dec->Hi32, dec->Lo64);
            }
            break;
        case WS_XML_TEXT_TYPE_GUID:
            {
                WS_XML_GUID_TEXT* guidText = (WS_XML_GUID_TEXT*) text;
                RPC_WSTR s;
                if (UuidToString(&guidText->value, &s) == RPC_S_OK)
                {
                    printf("WS_XML_TEXT_TYPE_GUID(value='%ls')", s);
                    RpcStringFree(&s);
                }
                else
                {
                    printf("WS_XML_TEXT_TYPE_GUID(value='<error: UuidToString failed>')");
                }
            }
            break;
        case WS_XML_TEXT_TYPE_UNIQUE_ID:
            {
                WS_XML_UNIQUE_ID_TEXT* uniqueIdText = (WS_XML_UNIQUE_ID_TEXT*) text;
                RPC_WSTR s;
                if (UuidToString(&uniqueIdText->value, &s) == RPC_S_OK)
                {
                    printf("WS_XML_TEXT_TYPE_UNIQUEID(value='%ls')", s);
                    RpcStringFree(&s);
                }
                else
                {
                    printf("WS_XML_TEXT_TYPE_UNIQUEID(value='<error: UuidToString failed>')");
                }
            }
            break;
        case WS_XML_TEXT_TYPE_DATETIME:
            {
                const WS_XML_DATETIME_TEXT* dateTimeText = (const WS_XML_DATETIME_TEXT*)text;
                WS_DATETIME value = dateTimeText->value;
                printf("WS_XML_DATETIME_TEXT(ticks='%I64u',format='%d')", value.ticks, value.format);
            }
            break;
        case WS_XML_TEXT_TYPE_TIMESPAN:
            {
                const WS_XML_TIMESPAN_TEXT* timeSpanText = (const WS_XML_TIMESPAN_TEXT*)text;
                printf("WS_XML_TIMESPAN_TEXT(value='%I64u')", timeSpanText->value.ticks);
            }
            break;
        case WS_XML_TEXT_TYPE_QNAME:
            {
                const WS_XML_QNAME_TEXT* qnameText = (const WS_XML_QNAME_TEXT*)text;
                printf("WS_XML_QNAME_TEXT(prefix='"); 
                PrintString(qnameText->prefix);
                printf("', localName='"); 
                PrintString(qnameText->localName);
                printf("', ns='"); 
                PrintString(qnameText->ns);
                printf("')");
            }
            break;
        case WS_XML_TEXT_TYPE_LIST:
            {
                const WS_XML_LIST_TEXT* listText = (const WS_XML_LIST_TEXT*)text;
                printf("WS_XML_LIST_TEXT(value={");
                for (ULONG i = 0; i < listText->itemCount; i++)
                {
                    PrintText(listText->items[i]);
                    printf(" ");
                }
                printf("}");
            }
            break;
        default:
            printf("<error: Unexpected textType>");
            break;
    }
}

// Main entry point
int __cdecl wmain()
{
    
    HRESULT hr = S_OK;
    WS_ERROR* error = NULL;
    WS_XML_READER* xmlReader = NULL;
    
    // Create an error object for storing rich error information
    hr = WsCreateError(
        NULL, 
        0, 
        &error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    // Create an XML reader
    hr = WsCreateReader(
        NULL,
        0, 
        &xmlReader, 
        error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    char* xml = 
    "<?xml version='1.0' encoding='UTF-8' standalone='yes'?>"
    "<Orders xmlns='http://example.com'>"
        "<!-- Order #1 -->"
        "<PurchaseOrder id='1'>"
            "<Quantity>42</Quantity>"
            "<ProductName>Toaster</ProductName>"
        "</PurchaseOrder>"
        "<!-- Order #2 -->"
        "<PurchaseOrder id='2'>"
            "<Quantity>5</Quantity>"
            "<ProductName><![CDATA[Block&Tackle]]></ProductName>"
        "</PurchaseOrder>"
    "</Orders>";
    BYTE* bytes = (BYTE*) xml;
    ULONG byteCount = (ULONG)strlen(xml);
    
    // Setup the source input
    WS_XML_READER_BUFFER_INPUT bufferInput;
    ZeroMemory(&bufferInput, sizeof(bufferInput));
    bufferInput.input.inputType = WS_XML_READER_INPUT_TYPE_BUFFER;
    bufferInput.encodedData = bytes;
    bufferInput.encodedDataSize = byteCount;
    
    // Setup the source encoding
    WS_XML_READER_TEXT_ENCODING textEncoding;
    ZeroMemory(&textEncoding, sizeof(textEncoding));
    textEncoding.encoding.encodingType = WS_XML_READER_ENCODING_TYPE_TEXT;
    textEncoding.charSet = WS_CHARSET_AUTO;
    
    // Setup the reader
    hr = WsSetInput(xmlReader, &textEncoding.encoding, &bufferInput.input, NULL, 0, error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    WS_CHARSET charSet;
    hr = WsGetReaderProperty(xmlReader, WS_XML_READER_PROPERTY_CHARSET, &charSet, sizeof(charSet), error);
    if (FAILED(hr))
    {
        goto Exit;
    }
    
    switch (charSet)
    {
        case WS_CHARSET_UTF8:
            printf("charSet=utf-8\n");
            break;
        case WS_CHARSET_UTF16LE:
            printf("charSet=utf-16LE\n");
            break;
        case WS_CHARSET_UTF16BE:
            printf("charSet=utf-16BE\n");
            break;
        default:
            printf("<error: Unexpected charset>\n");
            hr = E_FAIL;
            goto Exit;
    }
    
    ULONG depth = 0;
    for (;;)
    {
        // Get the current node of the reader
        const WS_XML_NODE* node;
        hr = WsGetReaderNode(xmlReader, &node, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
        for (ULONG i = 0; i < depth; i++)
        {
            printf(" ");
        }
        // Print out the type of node and its contents
        switch (node->nodeType)
        {
        case WS_XML_NODE_TYPE_ELEMENT:
            {
                const WS_XML_ELEMENT_NODE* elementNode = (const WS_XML_ELEMENT_NODE*) node;
                printf("WS_XML_NODE_TYPE_ELEMENT(prefix='");
                PrintString(elementNode->prefix);
                printf("', localName='");
                PrintString(elementNode->localName);
                printf("', ns='");
                PrintString(elementNode->ns);
                printf("', attributes={");
                for (ULONG i = 0; i < elementNode->attributeCount; i++)
                {
                    if (i != 0)
                    {
                        printf(", ");
                    }
                    const WS_XML_ATTRIBUTE* attribute = elementNode->attributes[i];
                    if (attribute->isXmlNs)
                    {
                        printf("(isXmlNs=TRUE, prefix='");
                        PrintString(attribute->prefix);
                        printf("', ns='");
                        PrintString(attribute->ns);
                        printf("')");
                    }
                    else
                    {
                        printf("(isXmlNs=FALSE, prefix='");
                        PrintString(attribute->prefix);
                        printf("', localName='");
                        PrintString(attribute->localName);
                        printf("', ns='");
                        PrintString(attribute->ns);
                        printf("', value=");
                        PrintText(attribute->value);
                        printf(")");
                    }
                }
                printf("})\n");
            }
            depth++;
            break;
        case WS_XML_NODE_TYPE_END_ELEMENT:
            printf("WS_XML_NODE_TYPE_END_ELEMENT\n");
            depth--;
            break;
        case WS_XML_NODE_TYPE_TEXT:
            {
                const WS_XML_TEXT_NODE* textNode = (const WS_XML_TEXT_NODE*) node;
                printf("WS_XML_NODE_TYPE_TEXT(value=");
                PrintText(textNode->text);
                printf(")\n");
            }
            break;
        case WS_XML_NODE_TYPE_CDATA:
            printf("WS_XML_NODE_TYPE_CDATA\n");
            depth++;
            break;
        case WS_XML_NODE_TYPE_END_CDATA:
            printf("WS_XML_NODE_TYPE_END_CDATA\n");
            depth--;
            break;
        case WS_XML_NODE_TYPE_COMMENT:
            {
                const WS_XML_COMMENT_NODE* commentNode = (const WS_XML_COMMENT_NODE*) node;
                printf("WS_XML_NODE_TYPE_COMMENT(value='");
                PrintString(&commentNode->value);
                printf("')\n");
            }
            break;
        case WS_XML_NODE_TYPE_BOF:
            printf("WS_XML_NODE_TYPE_BOF\n");
            depth++;
            break;
        case WS_XML_NODE_TYPE_EOF:
            printf("WS_XML_NODE_TYPE_EOF\n");
            depth--;
            break;
        default:
            printf("<error: Unexpected nodeType>\n");
            hr = E_FAIL;
            goto Exit;
        }
        // See if we've reached the end of the document
        if (node->nodeType == WS_XML_NODE_TYPE_EOF)
        {
            break;
        }
        // Advance the reader
        hr = WsReadNode(xmlReader, error);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }
    
Exit:
    if (FAILED(hr))
    {
        // Print out the error
        PrintError(hr, error);
    }
    
    if (xmlReader != NULL)
    {
        WsFreeReader(xmlReader);
    }
    if (error != NULL)
    {
        WsFreeError(error);
    }
    fflush(stdout);
    return SUCCEEDED(hr) ? 0 : -1;
}
