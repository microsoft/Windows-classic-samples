//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  cpack.cpp
//
//  Purpose:
//  This module implements the CPack class, which is 
//  the main class in this sample's code. 
//
//  CPack receives the text entered in the UI edit box, and packs it as 
//  a simple XML file. It then creates a BITS upload job, sets all 
//  the relevant properties, and kicks off the upload process (Job->Resume()).
// 
//  The main method implemented by this class is CPack::Upload(), which
//  takes care of creating the upload Job and therefore exemplifies how 
//  the BITS upload API may be used to upload a file to a web server.
//
//----------------------------------------------------------------------------



#include <windows.h>
#include <crtdbg.h>
#include <strsafe.h>
#include <wininet.h>
#include <bits.h>

#include "main.h"
#include "util.h"
#include "cpack.h"
#include "cmonitor.h"


// Forward prototype for UrlCombine(). The header shlwapi.h is installed with the IE SDK, but not with the Core SDK.
// If you have installed the IE SDK in addition to the Core SDK, you may remove the line below.
EXTERN_C HRESULT STDAPICALLTYPE UrlCombineW(LPCWSTR pszBase, LPCWSTR pszRelative, LPWSTR pszCombined, LPDWORD pcchCombined, DWORD dwFlags);
#ifndef URL_ESCAPE_UNSAFE
#define URL_ESCAPE_UNSAFE               0x20000000
#endif

CPack::CPack()
{
    m_hFile = INVALID_HANDLE_VALUE;
    memset(m_wszTempFile, 0, sizeof(m_wszTempFile));
    m_pwszFilename = NULL;
}

CPack::~CPack()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
    }
}

//----------------------------------------------------------------------------
// CPack public methods
//----------------------------------------------------------------------------


HRESULT CPack::Upload(LPCWSTR wszJobName, LPCWSTR wszUploadDir, BOOL fRequestReply)
{
    GUID         guid;
    BG_JOB_TYPE  jobtype;
    HRESULT      hr;
    CSmartComPtr<IBackgroundCopyJob> Job;
    WCHAR        wszRemoteUrl[INTERNET_MAX_URL_LENGTH] = {0};

    //
    // Prepare the remote URL name
    // No attempt is made to check the URL that was given by the user
    //
    hr = BuildRemoteUrl(wszUploadDir, m_pwszFilename, wszRemoteUrl, sizeof(wszRemoteUrl));
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to build the remote URL for the upload job. \n"
                            L"Please verify that the URL given for the upload virtual directory is valid.", hr);
        goto cleanup;
    }

    // 
    // Set the BITS job type -- we are only interested in upload job types
    //
    if (fRequestReply)
    {
        jobtype = BG_JOB_TYPE_UPLOAD_REPLY;
    }
    else
    {
        jobtype = BG_JOB_TYPE_UPLOAD;
    }

    //
    // Create an empty job with the BITS job manager
    // Note that even if the job name is not unique, we have a unique identifier (the returned guid value)
    //
    hr = g_JobManager->CreateJob(wszJobName, jobtype, &guid, &Job);
    if (FAILED(hr))
    {
        // handle the most common reason for the failure of this call
        if (hr == E_NOTIMPL)
        {
            DisplayErrorMessage(L"This application is not able to create a upload job with BITS.\n"
                                L"It is likely that the reason for that is that BITS 1.5 runtime binaries \n"
                                L"are not present in the system. Please see the file readme.htm installed \n"
                                L"with this SDK sample for instructions on how to install BITS 1.5.", hr);
        }
        else
        {
            DisplayErrorMessage(L"Failed to create the BITS upload job.", hr);
        }

        goto cleanup;
    }

    g_pDialog->AddStatusMessage(L"Created a new BITS upload job. Job ID is %s.", ConvertGuidToString(guid));

    //
    // Add a file to the job. This is the file that will be uploaded.
    //
    hr = Job->AddFile(wszRemoteUrl, m_wszTempFile);
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to add file to the BITS upload job.", hr);
        goto cleanup;
    }

    g_pDialog->AddStatusMessage(L"Added a file to the job. (Job ID %s)", ConvertGuidToString(guid));

    //
    // Set the no-progress timeout value to be 60 seconds. The default, if we don't set it, is 14 days.
    // We want to set this to a small value so that transient errors will fall back to errors quickly.
    //
    // The idea here is that we are using this app to test and experiment with the BITS API in a
    // controlled environment, so it is reasonable to assume that the server and network are readily 
    // accessible, and if not, we want to be notified of the problem quickly. We could have also
    // set NoProgressTimeout to zero.
    //
    // In a real app, it is better to leverage BITS' ability to retry jobs that are in transient error,
    // because sporadic network connectivity problems are indeed common, specially for file transfers 
    // across the Internet. For this to happen, the job timeout has to be greater than 2 min, otherwise
    // BITS will not attempt retries.
    // 
    hr = Job->SetNoProgressTimeout(60);
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to set the timeout value for transient errors.", hr);
        goto cleanup;
    }

    //
    // Kick off the job!!
    //
    hr = Job->Resume();
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to resume the BITS upload job.", hr);
        goto cleanup;
    }

    g_pDialog->AddStatusMessage(L"Upload job activated. (Job ID %s)", ConvertGuidToString(guid));

    //
    // Make sure we receive notifications for the status of the job,
    // so we can call Job->Complete() and have the job effectively uploaded
    //
    hr = Job->SetNotifyInterface(&g_NotificationReceiver);
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to set a notification callback.", hr);
        goto cleanup;
    }

    //
    // Well, ideally we just want to wait for JOB_TRANSFERRED, but errors
    // might happen so we want to act upon those as well
    //
    hr = Job->SetNotifyFlags(BG_NOTIFY_JOB_TRANSFERRED | BG_NOTIFY_JOB_ERROR | BG_NOTIFY_JOB_MODIFICATION);
    if (FAILED(hr))
    {
        DisplayErrorMessage(L"Failed to set a notification callback.", hr);
        goto cleanup;
    }

cleanup:

    return hr;
}


HRESULT CPack::PackText(LPCWSTR wszText)
{
    HRESULT hr = S_OK;
    WCHAR wszXMLTemplateStart[] = L""
        L"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
        L"<uploadsample>\r\n"
        L"  <text>";
    WCHAR wszXMLTemplateEnd[] = L""
        L"</text>\r\n"
        L"</uploadsample>\r\n";

    hr = OpenTempFile();
    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = WriteWCharAsUTF8(wszXMLTemplateStart);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // we don't need to write anything if we weren't given any text
    if (wcslen(wszText) > 0)
    {
        hr = WriteXMLString(wszText);
        if (FAILED(hr))
        {
            goto cleanup;
        }
    }

    hr = WriteWCharAsUTF8(wszXMLTemplateEnd);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    g_pDialog->AddStatusMessage(L"Packed edit box text in the temporary file %s", m_wszTempFile);

cleanup:

    CloseTempFile();

    return hr;
}

LPCWSTR CPack::GetFileName()
{
    return m_wszTempFile;
}


//----------------------------------------------------------------------------
// CPack private methods
//----------------------------------------------------------------------------


HRESULT CPack::BuildRemoteUrl(IN LPCWSTR wszUploadDir, IN LPCWSTR wszTempFile, OUT WCHAR *wszRemoteUrl, DWORD cbRemoteUrl)
{
    HRESULT hr;
    DWORD   cchRemoteUrl = (cbRemoteUrl/sizeof(WCHAR)) - 1; // this count shouldn't contain the NULL caracter
    WCHAR   wszXmlExt[]  = L".xml";
    WCHAR  *pStart       = NULL;
    WCHAR  *pEnd         = NULL;

    _ASSERT(wszRemoteUrl != NULL);
    _ASSERT((cbRemoteUrl == (INTERNET_MAX_URL_LENGTH*sizeof(WCHAR))));

    // clear the buffer
    memset(wszRemoteUrl, 0, cbRemoteUrl);

    // use the same filename as the temp file, and use the URL for the upload virtual directory
    // that was given by the user
    hr = UrlCombineW(wszUploadDir, m_pwszFilename, wszRemoteUrl, &cchRemoteUrl, URL_ESCAPE_UNSAFE);
    if (FAILED(hr))
    {
        goto cleanup;
    }

    // now change the extension
    pStart = wcsrchr(wszRemoteUrl, L'.');
    pEnd   = wszRemoteUrl + (cbRemoteUrl/sizeof(WCHAR)) - 1; // place the end at the last possible place for the \0.

    // make sure we have space to write our extension
    if (pStart && (static_cast<size_t>((pEnd - pStart)) >= wcslen(wszXmlExt)))  
    {
        hr = StringCchCopyW(pStart, (pEnd - pStart), wszXmlExt);
        if (FAILED(hr))
        {
            goto cleanup;
        }
    }         
    else
    {
        // should never happen
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
    }

cleanup:

    return hr;
}


HRESULT CPack::OpenTempFile()
{
    HRESULT hr = S_OK;
    WCHAR   wszTempDir[MAX_PATH+1] = {0};
    DWORD   dwLen = 0;


    dwLen = GetTempPathW(MAX_PATH, wszTempDir);
    if (dwLen == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    if (dwLen > MAX_PATH)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
        goto cleanup;
    }
 
    if (!GetTempFileName(
        wszTempDir,           // dir. for temp. files 
        L"upl",               // temp. file name prefix 
        0,                    // create unique name 
        m_wszTempFile))       // buffer for name 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    // Get The filename part
    m_pwszFilename = wcsrchr(m_wszTempFile, L'\\');
    if (!m_pwszFilename)
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_NAME);
        goto cleanup;
    }
    m_pwszFilename++;

    m_hFile = CreateFile(
        m_wszTempFile,                // file name 
        GENERIC_READ | GENERIC_WRITE, // open for read/write 
        0,                            // do not share 
        NULL,                         // no security 
        CREATE_ALWAYS,                // overwrite existing file
        FILE_ATTRIBUTE_NORMAL,        // normal file 
        NULL);                        // no attr. template 

    if (m_hFile == INVALID_HANDLE_VALUE) 
    { 
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
 

cleanup:

    return hr;
}

void CPack::CloseTempFile()
{
    if (m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
    }
}

HRESULT CPack::WriteWCharAsUTF8(LPCWSTR wszText, DWORD cCharsToWrite)
{
    HRESULT hr              = S_OK;
    CHAR    *szBuf          = NULL;
    INT     cchBuf          = 0;
    INT     cchWritten      = 0;
    DWORD   cbWrittenToFile = 0;

    _ASSERT(wszText != NULL);
    _ASSERT(cCharsToWrite > 0 && cCharsToWrite <= MAX_BUFFER_SIZE);

    cchBuf  = (cCharsToWrite + 1) * 3;    // UTF-8 can convert one WCHAR into 3 bytes

    szBuf = new CHAR[cchBuf];
    if (!szBuf)
    {
        hr = E_OUTOFMEMORY;
        goto cleanup;
    }

    cchWritten = WideCharToMultiByte(CP_UTF8, 0, wszText, cCharsToWrite, szBuf, cchBuf, NULL, NULL);
    if (cchWritten == 0)   // function failed
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

    // null-terminate the string
    szBuf[cchWritten] = 0;

    // write the string
    if (!WriteFile(m_hFile, szBuf, (cchWritten * sizeof(CHAR)), &cbWrittenToFile, NULL)) 
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto cleanup;
    }

cleanup:
    
    if (szBuf)
    {
        delete [] szBuf;
        szBuf = NULL;
    }

    return hr;
}

HRESULT CPack::WriteWCharAsUTF8(LPCWSTR wszText)
{
    return WriteWCharAsUTF8(wszText, static_cast<DWORD>(wcslen(wszText)));
}

HRESULT CPack::WriteXMLString(LPCWSTR wszText)
{
    WCHAR *pwszRemainder = NULL;
    WCHAR *pchSpecial    = NULL;
    WCHAR *pchEnd        = NULL;
    HRESULT hr = S_OK;

    _ASSERT(wszText != NULL);
    INT   cchText        = static_cast<INT>(wcslen(wszText));

    _ASSERT(cchText > 0 && cchText <= MAX_BUFFER_SIZE);
    
    pwszRemainder = const_cast<WCHAR *>(wszText);
    pchSpecial    = const_cast<WCHAR *>(wcsstr(wszText, L"<>&'\""));
    pchEnd        = (const_cast<WCHAR *>(wszText) + cchText);
    
    while (pchSpecial) 
    {
        if (pchSpecial > pwszRemainder)
        {
            hr = WriteWCharAsUTF8(pwszRemainder, static_cast<DWORD>((pchSpecial - pwszRemainder)));
            if (FAILED(hr))
            {
                goto cleanup;
            }
        }
        
        switch (*pchSpecial) 
        {
            case L'<':   WriteWCharAsUTF8(L"&lt;");      break;
            case L'>':   WriteWCharAsUTF8(L"&gt;");      break;
            case L'&':   WriteWCharAsUTF8(L"&amp;");     break;
            case L'\'':  WriteWCharAsUTF8(L"&apos;");    break;
            case L'"':   WriteWCharAsUTF8(L"&quot;");    break;

            default:    
                // should never get here
                _ASSERT(0);
        }

        pwszRemainder = pchSpecial + 1;
        pchSpecial = wcsstr(pwszRemainder, L"<>&'\"");
    }

    if (pwszRemainder < pchEnd)
    {
        hr = WriteWCharAsUTF8(pwszRemainder, static_cast<DWORD>((pchEnd - pwszRemainder)));
        if (FAILED(hr))
        {
            goto cleanup;
        }
    }

cleanup:
    
    return hr;
}

