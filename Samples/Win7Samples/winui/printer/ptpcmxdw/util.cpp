// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


#include "stdafx.h"
#include <rpcsal.h>

#include <msxml6.h>




//
// Define the max length (MAX_TEXT_LENGTH) of the error message buffer and text. Since the
// text is in unicode, therefore the buffer (MAX_TEXT_BYTES) is twice the size.
// (Size defined here includes \0 )
//
#define ERROR_MESSAGE_NUMCHARS 1024
#define ERROR_MESSAGE_BUFSIZE  ((ERROR_MESSAGE_NUMCHARS)*2)

VOID vPrint(
    __in    LPCTSTR   szMessageText
    )
{

    // 
    // Declare the length of buffer required to store multibyte string
    // for \n.  DBCS char set, needs 2 bytes to store singe char.
    //

    CHAR  szMultiByteString[ERROR_MESSAGE_BUFSIZE];
    DWORD dwNumCharsWritten     = 0;


    if ( !szMessageText ) 
    {
        return;
    }


    //
    // Format Unicode Text to a Multibyte 
    //
    if ( WideCharToMultiByte(CP_OEMCP, 0, szMessageText, -1, szMultiByteString,
                              CCHOF(szMultiByteString), NULL, NULL) )
    {   
        WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), szMultiByteString, 
                       (DWORD) strlen(szMultiByteString),
                       &dwNumCharsWritten, NULL);
    }

}

VOID vFormatAndPrint(
        IN  DWORD dwMsgId
    )
{
    //
    // It would have been so easy to use 
    //      CString MessageText;
    //      MessageText.FormatMessage(dwMsgId);
    // But what happens if the error message is that we are out of memory.
    // CString allocates memory, so CString might fail, the message might not 
    // get printed, and the user wont know what happened to the program.
    // Therefore, we cannot use dynamic allocation of mem here.
    //

    TCHAR szMessageText[ERROR_MESSAGE_NUMCHARS];
    if ( ! LoadString( (HINSTANCE)GetModuleHandle(NULL),// handle to resource module
                    dwMsgId,                            // resource identifier
                    szMessageText,                      // resource buffer
                    ERROR_MESSAGE_NUMCHARS )            // num chars in buffer
    )
    {

        //
        // The size of 1024 is big enough. But if for some reason, the error 
        // message becomes real big, the buffer will be filled with part of the
        // message. But no crash will happen. The only other thing we need to 
        // check for is whether LoadString() could actually find the string with the 
        // corresponding dwMsgID. In this case, we cant print anything, so we
        // quietly return.
        //

        return;
    }

    vPrint(szMessageText);
    return;
}



VOID DisplayUsage(
    VOID
    )
{
   
    vFormatAndPrint(IDS_APP_SUMMARY);
    vFormatAndPrint(IDS_APP_PREREQ);
    vFormatAndPrint(IDS_APP_SYNTAX);
}





/*++
Routine Name:
    IStreamToDOMDocument

Routine Description:

    Wrapper function for GetPrinter spooler API

Arguments:

    pStream       - Stream to read the XML content from.  It should be set to the
                    beginning of the XML document.
    ppXmlDocument - Newly created DOM document. Caller should release it.
    ppbstrErr     - Error string indicating what went wrong if there was an MSXML
                    parser error.
Return Value:

    

--*/

HRESULT
IStreamToDOMDocument(
    __inout     IStream           *pStream, 
    __deref_out IXMLDOMDocument2 **ppXmlDocument
    )
{
    HRESULT           hr        = S_OK;

    if (NULL == ppXmlDocument || 
        NULL == pStream)
    {
        hr = E_INVALIDARG;
    }

    if (ppXmlDocument)
    {
        *ppXmlDocument = NULL;
    }

    if ( SUCCEEDED(hr) )
    {
        hr = CoCreateInstance(CLSID_DOMDocument60,NULL,CLSCTX_INPROC_SERVER,IID_IXMLDOMDocument2,(LPVOID *)ppXmlDocument);
    }

    if( SUCCEEDED(hr) )
    {
        LARGE_INTEGER pos = {0, 0};
        hr = pStream->Seek( pos, STREAM_SEEK_SET, NULL );
    }

    //
    // Load the XML synchronously
    //
    if (SUCCEEDED(hr))
    {
        VARIANT_BOOL loadResult = 0; // Inidicates the result of loading the XML
        VARIANT      streamVar;      // Variant wrapper for the IStream to pass to load.

        VariantInit(&streamVar);
        V_VT(&streamVar) = VT_UNKNOWN;
        V_UNKNOWN(&streamVar) = pStream;
        pStream->AddRef();

        hr = (*ppXmlDocument)->load( streamVar, &loadResult );

        //
        // Check immediately to see if there was a parse error, since this
        // comes back as S_FALSE instead of E_...  
        //
        if (hr == S_FALSE)
        {
            // We could Figure out why it failed, but for now, lets just 
            // signal failure.
            hr = E_FAIL;
            
        }

        VariantClear(&streamVar);
    }

    return hr;
}

HRESULT
DOMDocumentToIStream(
    __in    IXMLDOMDocument2 *pDocument,
    __inout IStream          *pStream
     )
{
    HRESULT hr = S_OK;

    if ( NULL == pDocument || 
         NULL == pStream
        )
    {
        return E_INVALIDARG;
    }

    //
    // Write the document to the stream using a variant.  The stream
    // will automatically grow to the correct size.
    //
    if( SUCCEEDED(hr) )
    {
        VARIANT outputStream;
        VariantInit(&outputStream);
        V_VT(&outputStream) = VT_UNKNOWN;
        pStream->AddRef();
        V_UNKNOWN(&outputStream) = pStream;

        hr = pDocument->save( outputStream );

        VariantClear(&outputStream);
    }

    return hr;
}

HRESULT
StringDupUsingHeapAlloc(
    __in        PCWSTR pszString,
    __deref_out PWSTR  *ppszNewString
    )
{
    HRESULT hr           = S_OK;
    size_t  cchString    = 0;
    PWSTR  pszNewString  = NULL;

    if ( NULL == pszString ||
         NULL == ppszNewString )
    {
        hr = E_INVALIDARG;
    }

    if ( SUCCEEDED(hr) )
    {
        *ppszNewString = NULL;
        hr = StringCchLength( pszString, STRSAFE_MAX_CCH, &cchString);
    }

    if ( SUCCEEDED(hr) )
    {
        cchString++;
        pszNewString = (PWSTR)MemAllocZ(cchString * sizeof(WCHAR) );

        if ( NULL == pszNewString )
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if ( SUCCEEDED(hr) )
    {
        hr = StringCchCopy(pszNewString, cchString, pszString);
    }

    if ( SUCCEEDED(hr) )
    {
        *ppszNewString = pszNewString;
    }
    else
    {
        if ( pszNewString )
        {
            MemFree(pszNewString);
            pszNewString = NULL;
        }
    }
    return hr;
}
