// WinHttpFormSubmitSample.cpp : Defines the entry point for the console application.
//
// Copyright (c) Microsoft Corporation. All rights reserved. 


#include "stdio.h"
#include "tchar.h"
#include "windows.h"
#include "winhttp.h"

#ifndef UNICODE
#error "This sample was written as a Unicode only application."
#endif

BOOL WinHttpFormSubmitSample( LPCWSTR szServer, INTERNET_PORT dwPort, LPWSTR szPath, 
                              BOOL fUseSSL, LPWSTR szPostFilename);

int _tmain(int argc, _TCHAR* argv[])
{
    URL_COMPONENTS urlComponents;
    LPWSTR szUrl = argv[1];
    LPWSTR szFilename = argv[2];
    BOOL fShowHelp = FALSE;
    LPWSTR szAllocHost = NULL;
    LPWSTR szAllocPath = NULL;

    if (argc != 3)
    {
        fShowHelp = TRUE;
        goto done;
    }

    //  We call WinHttpCrackUrl to get the server, path, and scheme of the request.
    memset( &urlComponents, 0, sizeof(urlComponents));
    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.dwHostNameLength = 1;
    urlComponents.dwUrlPathLength = 1;
    
    if (!WinHttpCrackUrl( szUrl, 0, NULL, &urlComponents))
    {
        printf("\n\nThere was a problem with the URL given, \"%S\".", szUrl);
        fShowHelp = TRUE;
        goto done;
    }

    szAllocHost = new WCHAR[urlComponents.dwHostNameLength+1];
    szAllocPath = new WCHAR[urlComponents.dwUrlPathLength+1];

    if (szAllocHost == NULL
        || szAllocPath == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        goto done;
    }

    memcpy(szAllocHost, urlComponents.lpszHostName, 
           urlComponents.dwHostNameLength*sizeof(WCHAR));
    szAllocHost[urlComponents.dwHostNameLength] = L'\0';
    memcpy(szAllocPath, urlComponents.lpszUrlPath, 
           urlComponents.dwUrlPathLength*sizeof(WCHAR));
    szAllocPath[urlComponents.dwUrlPathLength] = L'\0';

    if ((DWORD)-1 == GetFileAttributes(szFilename))
    {
        printf("\n\nThe specified file, \"%S\", does not exist.", szFilename);
        fShowHelp = TRUE;
        goto done;
    }

    if (!WinHttpFormSubmitSample(
          szAllocHost, urlComponents.nPort, szAllocPath,
          (urlComponents.nScheme == INTERNET_SCHEME_HTTPS) ? TRUE : FALSE, szFilename))
    {
        goto done;
    }

    SetLastError(NO_ERROR);
done:    
    if (GetLastError() != NO_ERROR)
    {
        DWORD dwLastError = GetLastError();
        printf("\n\nFormSubmit failed with error code %i", dwLastError);
        fShowHelp = TRUE;
    }

    if (fShowHelp)
    {
        printf(
            "\n\n  Proper usage of this example is \"FormSubmit <url> <filename>\",\n"
            "Where <url> is a target HTTP URL and <filename> is the name of a file\n"
            "with the form submit contents.  Please see Section 8.2.1 \"Forms Submission\"\n"
            "of RFC 1866 (http://www.ietf.org/rfc/rfc1866.txt) for a description of valid\n"
            "form submit contents.");
    }

    if (szAllocPath != NULL)
        delete [] szAllocPath;

    if (szAllocHost != NULL)
        delete [] szAllocHost;

    printf("\n\n");
    return 0;
}



BOOL GetFileHandleAndSize(LPWSTR szFilename, OUT HANDLE* pHandle, OUT DWORD* pdwSize)
{
    BOOL returnValue = FALSE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    LARGE_INTEGER liSize;
    liSize.LowPart = 0;
    liSize.HighPart = 0;

    hFile = CreateFile(szFilename, GENERIC_READ, FILE_SHARE_READ, NULL, 
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        goto done;

    if (!GetFileSizeEx(hFile, &liSize))
        goto done;
    
    if (liSize.HighPart != 0 
        || liSize.LowPart > 0x7FFFFFFF)
    {
        //  Lets not try to send anything larger than 2 gigs
        SetLastError(ERROR_OPEN_FAILED);
    }

    returnValue = TRUE;
done:
    
    if (returnValue)
    {
        *pHandle = hFile;
        *pdwSize = liSize.LowPart;
        return TRUE;
    }
    else
    {
        if (hFile != INVALID_HANDLE_VALUE)
            CloseHandle(hFile);

        return FALSE;
    }
}

    
BOOL WinHttpFormSubmitSample( LPCWSTR szServer, INTERNET_PORT dwPort, LPWSTR szPath, 
                              BOOL fUseSSL, LPWSTR szPostFilename)
{
    DWORD dwStatusCode = 0;
    DWORD dwSize = sizeof(DWORD);
    DWORD dwTemp, dwTemp2;
    BOOL  bResults = FALSE;
    BOOL returnValue = FALSE;

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize = 0;
    BYTE byteBuffer[2048];

    HINTERNET  hSession = NULL, 
               hConnect = NULL,
               hRequest = NULL;

    static const WCHAR szContentType[] = L"Content-Type: application/x-www-form-urlencoded\r\n";


    // First make sure the file we want to send is available.
    if (!GetFileHandleAndSize(szPostFilename, &hFile, &dwFileSize))
        goto done;

    // Use WinHttpOpen to obtain a session handle.
    hSession = WinHttpOpen( L"WinHttpFormSubmitSample",  
                            WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                            WINHTTP_NO_PROXY_NAME, 
                            WINHTTP_NO_PROXY_BYPASS, 0);
    if (hSession == NULL)
        goto done;

    // Specify an HTTP server.
    hConnect = WinHttpConnect( hSession, szServer, dwPort, 0);
    if (hConnect == NULL)
        goto done;

    // Create an HTTP request handle.
    hRequest = WinHttpOpenRequest( hConnect, L"POST", szPath,
               NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 
               fUseSSL ? WINHTTP_FLAG_SECURE : 0);
    if (hRequest == NULL)
        goto done;

    //  Add the "Content-Type" header

    if (!WinHttpAddRequestHeaders(hRequest, szContentType, (DWORD)-1, 
               WINHTTP_ADDREQ_FLAG_ADD & WINHTTP_ADDREQ_FLAG_REPLACE))
        goto done;

    bResults = WinHttpSendRequest( hRequest,
                                   WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                   NULL, 0, dwFileSize, 0);
    if (!bResults)
        goto done;

    while (dwFileSize > 0)
    {
        if (!ReadFile(hFile, byteBuffer, sizeof(byteBuffer), &dwTemp, NULL))
            goto done;

        if (dwTemp == 0)
        {
            dwFileSize = 0;
            continue;
        }
        else if (dwTemp > dwFileSize)
        {
            // unexpectedly found more data in the file than we though was there, bail out
            goto done;
        }
        else
        {
            dwFileSize -= dwTemp;  // update the amount of file left to send..
        }

        if (!WinHttpWriteData( hRequest, byteBuffer, dwTemp, &dwTemp2)
            || dwTemp != dwTemp2)
        {
            goto done;
        }
    }

    //  ensure the whole file was successfully sent
    if (dwFileSize != 0)
        goto done;

    // End the request.
    bResults = WinHttpReceiveResponse( hRequest, NULL);
    if (!bResults)
        goto done;

    // Check the status code.
    bResults = WinHttpQueryHeaders( hRequest, 
                                    WINHTTP_QUERY_STATUS_CODE|
                                    WINHTTP_QUERY_FLAG_NUMBER, NULL, 
                                    &dwStatusCode, &dwSize, NULL);
    if (!bResults)
        goto done;

    if (bResults)
    {
        switch (dwStatusCode)
        {
        case 200: 
            //  The resource was successfully retrieved.
            //  You could use WinHttpReadData to read the contents of the server's response.
            printf("\nSuccessfuly posted to %S.", szServer);
            break;
        default:
            //  The status code does not indicate success.
            printf("\nThe server responded with a status code of %i.", dwStatusCode);
            break;
        }
    }

    returnValue = TRUE;

done:
    dwTemp = GetLastError();
    
    // Close any open handles.
    if (hRequest) 
        WinHttpCloseHandle(hRequest);
    
    if (hConnect) 
        WinHttpCloseHandle(hConnect);
    
    if (hSession) 
        WinHttpCloseHandle(hSession);
    
    if (hFile != INVALID_HANDLE_VALUE) 
        CloseHandle(hFile);

    SetLastError(dwTemp);
    return returnValue;
}
