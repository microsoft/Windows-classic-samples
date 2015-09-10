#include <ole2.h>
#include <xmllite.h>
#include <stdio.h>
#include "FileStreamWithEPending.hpp"


HRESULT SaveFile(LPCWSTR pcwsFilename, IStream *pStream, ULONG uLength)
{
    HRESULT hr = S_OK;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    BOOL rc = FALSE;
    ULONG byteRead = 0;
    BYTE buff[255];

    if (NULL == pcwsFilename || NULL == pStream)
        HR(E_POINTER);

    hFile = ::CreateFile(pcwsFilename, GENERIC_WRITE, FILE_SHARE_READ,
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
        HR(HRESULT_FROM_WIN32(GetLastError()));

    CHKHR(pStream->Read(buff, DELIMIT_LEN + 2, &byteRead));  //omit delimit and enter character(\r\n)

    while(uLength > 0 && S_FALSE != hr)
    {
        byteRead = min(uLength, 255);
        CHKHR(pStream->Read(buff, byteRead, &byteRead));
        uLength -= byteRead;
        rc = ::WriteFile(hFile, buff, byteRead, NULL, NULL);
        if (FALSE == rc)
            HR(HRESULT_FROM_WIN32(GetLastError()));
    }

CleanUp:
    if (INVALID_HANDLE_VALUE != hFile)
        CloseHandle(hFile);
    return hr;

}

/**
 * Notice that this only works for XmlLite with non-blocking feature from Win8.
 *
 */
int __cdecl wmain(int argc, _In_reads_(argc) WCHAR* argv[])
{
    HRESULT hr = S_OK;
    IStream* pFileStream = NULL;
    IXmlReader* pReader = NULL;
    XmlNodeType nodeType;

    if (argc != 2)
    {
        wprintf( L"Usage: InterleavedXmlReader.exe name-of-input-file\n");
        HR(E_INVALIDARG);
    }

    CHKHR(FileStreamWithEPending::OpenFile(argv[1], &pFileStream, false));
    CHKHR(::CreateXmlReader(IID_IXmlReader, (void**)&pReader, NULL));
    CHKHR(pReader->SetInput(pFileStream));

    WCHAR *szFileName = NULL, *szFileLength = NULL;
    UINT uFileNameLength = 0, uFileLength = 0;
    while (S_FALSE != hr && SUCCEEDED(hr))
    {
        while(S_OK == (hr = pReader->Read(&nodeType)))
        {
            switch (nodeType) {
            case XmlNodeType_Element:
               CHKHR(pReader->MoveToAttributeByName(L"file_name", NULL));
               CHKHR(pReader->GetValue((LPCWSTR *)&szFileName, &uFileNameLength));
               if (!szFileName || 0 == uFileNameLength)
                   HR(E_UNEXPECTED);
               
               CHKHR(pReader->MoveToElement());
               CHKHR(pReader->MoveToAttributeByName(L"file_length", NULL));

               CHKHR(pReader->GetValue((LPCWSTR *)&szFileLength, &uFileLength));
               if (szFileLength && uFileLength > 0)
                   uFileLength = _wtoi(szFileLength);
               else
                   uFileLength = 0;

               CHKHR(pReader->MoveToElement());
               break;
           }
        }

        if (E_PENDING == hr)
        {
            ((FileStreamWithEPending*)pFileStream)->SetRaisePendingFlag(FALSE);
            CHKHR(SaveFile(szFileName, pFileStream, uFileLength));
            ((FileStreamWithEPending*)pFileStream)->SetRaisePendingFlag(TRUE);
        }
    }


CleanUp:
    SAFE_RELEASE(pFileStream);
    SAFE_RELEASE(pReader);

    return hr;
}
