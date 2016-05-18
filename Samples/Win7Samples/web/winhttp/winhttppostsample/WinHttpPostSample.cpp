// WinHttpPostSample.cpp : Defines the entry point for the console application.
//
// Copyright (c) Microsoft Corporation. All rights reserved. 


#include <tchar.h>
#include <windows.h>
#include <stdio.h>
#include "winhttp.h"

#ifndef UNICODE
#error This sample was written as a Unicode only application."
#endif

BOOL WinHttpSamplePost( LPCWSTR szServerUrl, LPCWSTR szFile, LPCWSTR szProxyUrl);

class CQuickStringWrap
{
    LPWSTR _szAlloc;

public:
    CQuickStringWrap()
    {
        _szAlloc = NULL;
    }

    ~CQuickStringWrap()
    {
        if (_szAlloc != NULL)
            delete [] _szAlloc;
    }

    operator LPCWSTR() const { return _szAlloc;}

    BOOL Set(LPCWSTR szIn, DWORD dwLen)
    {
        LPWSTR szNew;
        
        szNew = new WCHAR[dwLen+1];

        if (szNew == NULL)
        {
            SetLastError( ERROR_OUTOFMEMORY);
            return FALSE;
        }

        memcpy(szNew, szIn, dwLen*sizeof(WCHAR));
        szNew[dwLen] = L'\0';

        if (_szAlloc != NULL)
            delete [] _szAlloc;

        _szAlloc = szNew;

        return TRUE;
    }
};


int _tmain(int argc, _TCHAR* argv[])
{
    LPWSTR szServerUrl = argv[1];
    LPWSTR szFilename = argv[2];
    LPWSTR szProxyUrl = NULL;  // optional argv[3];
    URL_COMPONENTS urlComponents;
    BOOL fShowHelp = FALSE;
    DWORD dwTemp;

    //
    //  Make sure we got three or four params 
    //(the first is the executeable name, rest are user params)
    //
    
    if (argc != 4)
    {
        if (argc == 3)
        {
            szProxyUrl = NULL;
        }
        else
        {
            fShowHelp = TRUE;
            goto done;
        }
    }
    else
    {
        szProxyUrl = argv[3];
    }

    //
    //  Do some validation on the input URLs..
    //

    memset(&urlComponents,0,sizeof(urlComponents));
    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.dwUserNameLength = 1;
    urlComponents.dwPasswordLength = 1;
    urlComponents.dwHostNameLength = 1;
    urlComponents.dwUrlPathLength = 1;

    if( !WinHttpCrackUrl(szServerUrl, 0, 0, &urlComponents))
    {
        printf("\nThere was a problem with the Server URL %S.\n", szServerUrl);
        fShowHelp = TRUE;
        goto done;
    }

    memset(&urlComponents,0,sizeof(urlComponents));
    urlComponents.dwStructSize = sizeof(urlComponents);
    urlComponents.dwUserNameLength = 1;
    urlComponents.dwPasswordLength = 1;
    urlComponents.dwHostNameLength = 1;
    urlComponents.dwUrlPathLength = 1;

    if (szProxyUrl != NULL
        && (!WinHttpCrackUrl(szProxyUrl, 0, 0, &urlComponents)
            || urlComponents.dwUrlPathLength > 1
            || urlComponents.nScheme != INTERNET_SCHEME_HTTP))
    {
        printf("\nThere was a problem with the Proxy URL %S."
               "  It should be a http:// url, and should have"
               " an empty path.\n", szProxyUrl);
        fShowHelp = TRUE;
        goto done;
    }

    //
    //  Make sure a file was passed in...
    //

    if ((DWORD)-1 == GetFileAttributes(szFilename))
    {
        printf("\nThe specified file, \"%S\", was not found.\n", szFilename);
        fShowHelp = TRUE;
        goto done;
    }

    if (!WinHttpSamplePost(szServerUrl, szFilename, szProxyUrl))
        goto done;

    SetLastError(NO_ERROR);

done:
    dwTemp = GetLastError();
    if (dwTemp != NO_ERROR)
    {
        printf( "\nWinHttpPostSample failed with error code %i.", dwTemp);
    }

    if (fShowHelp)
    {
        printf(
            "\n\n  Proper usage of this example is \"Post <url1> <filename> [url2]\",\n"
            "Where <url1> is the target HTTP URL and <filename> is the name of a file\n"
            "which will be POST'd to <url1>.  [url2] is optional and indicates the proxy\n"
            "to use.\n"
            "  Urls are of the form http://[username]:[password]@<server>[:port]/<path>.");
    }

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


DWORD ChooseAuthScheme( HINTERNET hRequest, DWORD dwSupportedSchemes)
{
    //  It is the servers responsibility to only accept authentication schemes
    //which provide the level of security needed to protect the server's
    //resource.
    //  However the client has some obligation when picking an authentication
    //scheme to ensure it provides the level of security needed to protect
    //the client's username and password from being revealed.  The Basic authentication
    //scheme is risky because it sends the username and password across the
    //wire in a format anyone can read.  This is not an issue for SSL connections though.

    if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE)
        return WINHTTP_AUTH_SCHEME_NEGOTIATE;
    else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_NTLM)
        return WINHTTP_AUTH_SCHEME_NTLM;
    else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT)
        return WINHTTP_AUTH_SCHEME_PASSPORT;
    else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST)
        return WINHTTP_AUTH_SCHEME_DIGEST;
    else if (dwSupportedSchemes & WINHTTP_AUTH_SCHEME_BASIC)
    {
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        if (WinHttpQueryOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwValue,&dwSize)
            && (dwValue & SECURITY_FLAG_SECURE))
        {
            return WINHTTP_AUTH_SCHEME_BASIC;
        }
        else
            return 0;
    }
    else
        return 0;
}


BOOL WinHttpSamplePost( LPCWSTR szServerUrl, LPCWSTR szFile, LPCWSTR szProxyUrl)
{
    BOOL returnValue = FALSE;
    DWORD dwTemp;

    URL_COMPONENTS urlServerComponents;
    URL_COMPONENTS urlProxyComponents;
    CQuickStringWrap strTargetServer;
    CQuickStringWrap strTargetPath;
    CQuickStringWrap strTargetUsername;
    CQuickStringWrap strTargetPassword;
    CQuickStringWrap strProxyServer;
    CQuickStringWrap strProxyUsername;
    CQuickStringWrap strProxyPassword;
    
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize;

    HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
    DWORD dwProxyAuthScheme = 0;
    DWORD dwStatusCode = 0;

    if (!GetFileHandleAndSize((LPWSTR)szFile, &hFile, &dwFileSize))
        goto done;

    //
    //  Its a long but straightforward chunk of code below that
    //splits szServerUrl and szProxyUrl into the various
    //strTarget*,strProxy* components.  They need to be put
    //into the separate str* variables so that they are individually
    //NULL-terminated.
    //

    //  From the server URL, we need a host, path, username and password.
    memset (&urlServerComponents, 0, sizeof(urlServerComponents));
    urlServerComponents.dwStructSize = sizeof(urlServerComponents);
    urlServerComponents.dwHostNameLength = 1;
    urlServerComponents.dwUrlPathLength = 1;
    urlServerComponents.dwUserNameLength = 1;
    urlServerComponents.dwPasswordLength = 1;

    if (!WinHttpCrackUrl(szServerUrl, 0, 0, &urlServerComponents))
        goto done;

    //
    //  An earlier version of WinHttp v5.1 has a bug where it misreports
    //the length of the username or password if they are not present.
    //

    if (urlServerComponents.lpszUserName == NULL)
        urlServerComponents.dwUserNameLength = 0;

    if (urlServerComponents.lpszPassword == NULL)
        urlServerComponents.dwPasswordLength = 0;

    if (!strTargetServer.Set(urlServerComponents.lpszHostName, urlServerComponents.dwHostNameLength)
        || !strTargetPath.Set(urlServerComponents.lpszUrlPath, urlServerComponents.dwUrlPathLength))
    {
        goto done;
    }

    //  for the username and password, if they are empty, leave the string pointers as NULL.
    //  This allows for the current process's default credentials to be used.
    if (urlServerComponents.dwUserNameLength != 0
        && !strTargetUsername.Set(urlServerComponents.lpszUserName, 
                                  urlServerComponents.dwUserNameLength))
    {
        goto done;
    }

    if (urlServerComponents.dwPasswordLength != 0
        && !strTargetPassword.Set(urlServerComponents.lpszPassword, 
                                  urlServerComponents.dwPasswordLength))
    {
        goto done;
    }

    if (szProxyUrl != NULL)
    {
        //  From the proxy URL, we need a host, username and password.
        memset (&urlProxyComponents, 0, sizeof(urlProxyComponents));
        urlProxyComponents.dwStructSize = sizeof(urlProxyComponents);
        urlProxyComponents.dwHostNameLength = 1;
        urlProxyComponents.dwUserNameLength = 1;
        urlProxyComponents.dwPasswordLength = 1;

        if (!WinHttpCrackUrl(szProxyUrl, 0, 0, &urlProxyComponents))
            goto done;

        //
        //  An earlier version of WinHttp v5.1 has a bug where it misreports
        //the length of the username or password if they are not present.
        //

        if (urlProxyComponents.lpszUserName == NULL)
            urlProxyComponents.dwUserNameLength = 0;

        if (urlProxyComponents.lpszPassword == NULL)
            urlProxyComponents.dwPasswordLength = 0;

        //  We do something tricky here, taking from the host beginning 
        //to the beginning of the path as the strProxyServer.  What this 
        //does, is if you have urls like "http://proxy","http://proxy/",
        //"http://proxy:8080" is copy them as "proxy","proxy","proxy:8080" 
        //respectively.  This makes the port available for WinHttpOpen.
        if (urlProxyComponents.lpszUrlPath == NULL)
        {
            urlProxyComponents.lpszUrlPath = wcschr(urlProxyComponents.lpszHostName, L'/');
            if(urlProxyComponents.lpszUrlPath == NULL)
            {
                urlProxyComponents.lpszUrlPath = urlProxyComponents.lpszHostName 
                                         + wcslen(urlProxyComponents.lpszHostName);
            }
        }
        if (!strProxyServer.Set(urlProxyComponents.lpszHostName, 
            (DWORD)(urlProxyComponents.lpszUrlPath - urlProxyComponents.lpszHostName)))
        {
            goto done;
        }

        //  for the username and password, if they are empty, leave the string pointers as NULL.
        //  This allows for the current process's default credentials to be used.
        if (urlProxyComponents.dwUserNameLength != 0
            && !strProxyUsername.Set(urlProxyComponents.lpszUserName, 
                                     urlProxyComponents.dwUserNameLength))
        {
            goto done;
        }

        if (urlProxyComponents.dwPasswordLength != 0
            && !strProxyPassword.Set(urlProxyComponents.lpszPassword, 
                                     urlProxyComponents.dwPasswordLength))
        {
            goto done;
        }
    }


    //
    //  whew, now we can go on and start the request.
    //

    //
    //  Open a WinHttp session using the specified proxy
    //
    if (szProxyUrl != NULL)
    {
        hSession = WinHttpOpen(L"WinHttpPostSample",WINHTTP_ACCESS_TYPE_NAMED_PROXY, 
                               strProxyServer, L"<local>", 0);
    }
    else
    {
        hSession = WinHttpOpen(L"WinHttpPostSample",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                               NULL,NULL,0);
    }
    
    if (hSession == NULL)
       goto done;

    //
    //  Open a connection to the target server
    //
    hConnect = WinHttpConnect( hSession, strTargetServer, urlServerComponents.nPort, 0);

    if (hConnect == NULL)
        goto done;

    //
    //  Open the request
    //
    hRequest = WinHttpOpenRequest(hConnect, L"POST", strTargetPath, NULL, NULL, NULL,
                urlServerComponents.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0);

    if (hRequest == NULL)
        goto done;

    //
    //  Send the request.
    //
    //  This is done in a loop so that authentication challenges can be handled.
    //
    BOOL bDone;
    DWORD dwLastStatusCode = 0;
    bDone = FALSE;
    while (!bDone)
    {
        //  If a proxy auth challenge was responded to, reset those credentials
        //before each SendRequest.  This is done because after responding to a 401
        //or perhaps a redirect the proxy may require re-authentication.  You
        //could get into a 407,401,407,401,etc loop otherwise.
        if (dwProxyAuthScheme != 0)
        {
            if( !WinHttpSetCredentials( hRequest, WINHTTP_AUTH_TARGET_PROXY, 
                                        dwProxyAuthScheme, strProxyUsername,
                                        strProxyPassword, NULL))
            {
                goto done;
            }
        }
        // Send a request.
        
        if (!WinHttpSendRequest( hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                 NULL, 0, dwFileSize, 0))
        {
            goto done;
        }

        //
        //  Now we send the contents of the file.  We may have to redo this
        //after an auth challenge, and so we will reset the file position
        //to the beginning on each loop.
        //
        if(INVALID_SET_FILE_POINTER == SetFilePointer(hFile,0,NULL,FILE_BEGIN))
            goto done;

        //  Load the file 4k at a time and write it.  fwFileLeft will track
        //how much more needs to be written.
        DWORD dwFileLeft;
        dwFileLeft = dwFileSize;
        while(dwFileLeft > 0)
        {
            DWORD dwBytesRead;
            BYTE buffer[4096];

            if (!ReadFile(hFile, buffer, sizeof(buffer), &dwBytesRead, NULL))
                goto done;

            if (dwBytesRead == 0)
            {
                dwFileLeft = 0;
                continue;
            }
            else if (dwBytesRead > dwFileLeft)
            {
                // unexpectedly read more from the file than we expected to find.. bail out
                goto done;
            }
            else
                dwFileLeft -= dwBytesRead;

            if (!WinHttpWriteData(hRequest, buffer, dwBytesRead, &dwBytesRead))
                goto done;
        }

        // End the request.
        if (!WinHttpReceiveResponse( hRequest, NULL))
        {
            //  There is a special error we can get here indicating we need to try again
            if (GetLastError() == ERROR_WINHTTP_RESEND_REQUEST)
                continue;
            else
                goto done;
        }

        // Check the status code.
        dwTemp = sizeof(dwStatusCode);
        if (!WinHttpQueryHeaders( hRequest, 
                        WINHTTP_QUERY_STATUS_CODE| WINHTTP_QUERY_FLAG_NUMBER, 
                        NULL, &dwStatusCode, &dwTemp, NULL))
        {
            goto done;
        }

        DWORD dwSupportedSchemes, dwFirstScheme, dwTarget, dwSelectedScheme;
        switch (dwStatusCode)
        {
        case 200: 
            // The resource was successfully retrieved.
            // You could use WinHttpReadData to read the contents of the server's response.
            printf("\nThe POST was successfully completed.");
            bDone = TRUE;
            break;
        case 401:
            // The server requires authentication.
            printf("\nThe server requires authentication.  Sending credentials...");

            // Obtain the supported and preferred schemes.
            if( !WinHttpQueryAuthSchemes(hRequest, &dwSupportedSchemes, &dwFirstScheme, &dwTarget))
                goto done;                                          

            // Set the credentials before resending the request.
            dwSelectedScheme = ChooseAuthScheme(hRequest, dwSupportedSchemes);

            if (dwSelectedScheme == 0)
            {
                bDone = TRUE;
            }
            else
            {
                if (!WinHttpSetCredentials( hRequest, dwTarget, dwSelectedScheme, 
                                            strTargetUsername, strTargetPassword, NULL))
                {
                    goto done;
                }
            }

            // If the same credentials are requested twice, abort the
            // request.  For simplicity, this sample does not check for
            // a repeated sequence of status codes.
            if (dwLastStatusCode==401)
            {
                printf("\nServer Authentication failed.");
                bDone = TRUE;
            }

            break;
        case 407:
            // The proxy requires authentication.
            printf("\nThe proxy requires authentication.  Sending credentials...");

            // Obtain the supported and preferred schemes.
            if (!WinHttpQueryAuthSchemes( hRequest, &dwSupportedSchemes, &dwFirstScheme, &dwTarget))
                goto done;

            // Set the credentials before resending the request.
            dwProxyAuthScheme = ChooseAuthScheme(hRequest, dwSupportedSchemes);

            // If the same credentials are requested twice, abort the
            // request.  For simplicity, this sample does not check for
            // a repeated sequence of status codes.
            if (dwLastStatusCode==407)
            {
                printf("\nProxy Authentication failed.");
                bDone = TRUE;
            }
            break;
        default:
            // The status code does not indicate success.
            printf("\nStatus code %d returned.\n", dwStatusCode);
            bDone = TRUE;
        }
   
        // Keep track of the last status code.
        dwLastStatusCode = dwStatusCode;
    }

    returnValue = TRUE;

done:
    dwTemp = GetLastError();

    if (hRequest != NULL)
        WinHttpCloseHandle(hRequest);
    
    if (hConnect != NULL)
        WinHttpCloseHandle(hConnect);
    
    if (hSession != NULL)
        WinHttpCloseHandle(hSession);

    if (hFile != INVALID_HANDLE_VALUE)
        CloseHandle(INVALID_HANDLE_VALUE);
    
    SetLastError(dwTemp);

    return returnValue;
}


