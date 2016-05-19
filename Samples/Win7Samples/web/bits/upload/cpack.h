//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  cpack.h
//
//  Purpose:
//  Defines the class CPack, used to pack the user's input as an XML file
//  and upload it using BITS.
//
//----------------------------------------------------------------------------


#pragma once


class CPack
{
    WCHAR   m_wszTempFile[MAX_PATH+1];
    WCHAR  *m_pwszFilename;
    HANDLE  m_hFile;

    HRESULT OpenTempFile();
    void    CloseTempFile();

    HRESULT WriteWCharAsUTF8(LPCWSTR wszText, DWORD cCharsToWrite);
    HRESULT WriteWCharAsUTF8(LPCWSTR wszText);
    HRESULT WriteXMLString(LPCWSTR wszText);

    HRESULT BuildRemoteUrl(IN LPCWSTR wszUploadDir, IN LPCWSTR wszTempFile, OUT WCHAR *wszRemoteUrl, DWORD cbRemoteUrl);

public:

    CPack();
    ~CPack();

    HRESULT  PackText(LPCWSTR wszText);
    LPCWSTR  GetFileName();
    HRESULT  Upload(LPCWSTR wszJobName, LPCWSTR wszRemoteFile, BOOL fRequireUploadReply);
};

