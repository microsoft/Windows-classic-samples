/*++

Copyright (c) 2008  Microsoft Corporation

Title:

    Express Writer sample application

Abstract:

    Helper functions implementation

--*/


#include "stdafx.h"


///////////////////////////////////////////////////////////////////////////////
//
// LoadMetadata
//
///////////////////////////////////////////////////////////////////////////////
bool LoadMetadata(
    __in        IVssExpressWriter           *pExpressWriter,
    __in        PCWSTR                      wszParameter,
    __out       PWSTR                       *pwszData
)
{
    HRESULT hr      = S_OK;
    bool    bResult = false;
    PWSTR   wszData = NULL;
    HANDLE  hFile   = INVALID_HANDLE_VALUE;
    DWORD   cbRead  = 0;
    DWORD   cbSize  = 0;

    wprintf(L"INFO: LoadMetadata called\n");

    *pwszData = NULL;

    CHECK_CONDITION(wszParameter != NULL, L"LoadMetadata not called");

    //
    // Load from file
    //
    hFile = CreateFile(
        wszParameter,                       // file to open
        GENERIC_READ,                       // open for writing
        0,                                  // don't share
        NULL,                               // default security
        OPEN_EXISTING,                      // existing file only
        FILE_ATTRIBUTE_NORMAL,              // normal file
        NULL);                              // no attr. template

    CHECK_CONDITION(
        hFile != INVALID_HANDLE_VALUE,
        L"CreateFile failed",
        (LONGLONG)GetLastError());

    cbSize = GetFileSize(hFile, NULL);
    CHECK_CONDITION(
        cbSize != INVALID_FILE_SIZE,
        L"GetFileSize failed",
        (LONGLONG)GetLastError());

    wszData = (PWSTR)malloc(cbSize + sizeof(WCHAR));
    ZeroMemory(wszData, cbSize + sizeof(WCHAR));

    CHECK_CONDITION(
        ReadFile(hFile, wszData, cbSize, &cbRead, NULL),
        L"ReadFile failed",
        (LONGLONG)GetLastError());

    CHECK_HR(
        pExpressWriter->LoadMetadata(wszData, 0),
        L"CreateVssExpressWriter failed");

    *pwszData = wszData;

    bResult = true;

_exit:
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// SetRestoreMethod
//
///////////////////////////////////////////////////////////////////////////////
bool SetRestoreMethod(
    __in        IVssCreateExpressWriterMetadata *pMetadata,
    __in        VSS_RESTOREMETHOD_ENUM          Method,
    __in_opt    LPCWSTR                         wszService,
    __in        bool                            bRebootRequired
)
{
    HRESULT hr      = S_OK;
    bool    bResult = false;

    wprintf(L"INFO: SetRestoreMethod called\n");

    CHECK_HR(pMetadata->SetRestoreMethod(
        Method,
        wszService,
        NULL,                               // Reserved, must be NULL
        VSS_WRE_NEVER,                      // Must be VSS_WRE_NEVER
        bRebootRequired),
        L"SetRestoreMethod failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// AddComponent
//
///////////////////////////////////////////////////////////////////////////////
bool AddComponent(
    __in        IVssCreateExpressWriterMetadata *pMetadata,
    __in        VSS_COMPONENT_TYPE              componentType,
    __in        PCWSTR                          wszName
)
{
    HRESULT hr      = S_OK;
    bool    bResult = false;

    wprintf(L"INFO: AddComponent called\n");

    CHECK_HR(pMetadata->AddComponent(
        componentType,
        NULL,
        wszName,
        wszName,
        NULL,
        0,
        false,
        false,
        false),
        L"AddComponent failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// AddFilesToFileGroup
//
///////////////////////////////////////////////////////////////////////////////
bool AddFilesToFileGroup(
    __in        IVssCreateExpressWriterMetadata  *pMetadata,
    __in        PCWSTR                          wszGroupName,
    __in        PCWSTR                          wszPath,
    __in        PCWSTR                          wszFilespec,
    __in        bool                            bRecursive,
    __in        DWORD                           dwBackupTypeMask
)
{
    HRESULT hr      = S_OK;
    bool    bResult = false;

    wprintf(L"INFO: AddFilesToFileGroup called\n");

    CHECK_HR(pMetadata->AddFilesToFileGroup(
        NULL,
        wszGroupName,
        wszPath,
        wszFilespec,
        bRecursive,
        NULL,
        dwBackupTypeMask),
        L"AddFilesToFileGroup failed");

    bResult = true;

_exit:
    return bResult;
}


///////////////////////////////////////////////////////////////////////////////
//
// SaveToFile
//
///////////////////////////////////////////////////////////////////////////////
bool SaveToFile(
    __in        PCWSTR  wszData,
    __in        PCWSTR  wszFileName
)
{
    bool        bResult     = false;
    HANDLE      hFile       = INVALID_HANDLE_VALUE;
    DWORD       cbWritten   = 0;

    wprintf(L"INFO: SaveToFile called\n");

    hFile = CreateFile(
        wszFileName,                        // file to open
        GENERIC_WRITE,                      // open for writing
        0,                                  // share for reading
        NULL,                               // default security
        CREATE_ALWAYS,                      // always create
        FILE_ATTRIBUTE_NORMAL,              // normal file
        NULL);                              // no attr. template

    CHECK_CONDITION(
        hFile != INVALID_HANDLE_VALUE,
        L"CreateFile failed with %d",
        (LONGLONG)GetLastError());

    CHECK_CONDITION(
        WriteFile(hFile, wszData, (DWORD)(wcslen(wszData) * sizeof(WCHAR)), &cbWritten, NULL),
        L"WriteFile failed",
        (LONGLONG)GetLastError());

    bResult = true;

_exit:
    if (hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hFile);
    }
    return bResult;
}


bool SaveToFile(
    __in        IVssCreateExpressWriterMetadata *pMetadata,
    __in        PCWSTR                          wszFileName
)
{
    HRESULT     hr          = S_OK;
    bool        bResult     = false;
    CComBSTR    bstr;

    CHECK_HR(pMetadata->SaveAsXML(&bstr), L"SaveAsXML failed");

    CHECK_CONDITION(SaveToFile(PWSTR(bstr), wszFileName), L"SaveToFile failed\n");

    bResult = true;

_exit:
    return bResult;
}

///////////////////////////////////////////////////////////////////////////////
//
// ConstructWriterDefinition
//
///////////////////////////////////////////////////////////////////////////////
bool ConstructWriterDefinition(
    __in    IVssCreateExpressWriterMetadata *pMetadata
)
{
    HRESULT hr              = S_OK;
    bool    bResult         = false;
    PCWSTR  wszComponent    = L"SampleComponent";

    wprintf(L"INFO: ConstructWriterDefinition called\n");

    //
    // Select restore method
    //
    wprintf(L"INFO: Selecting restore method...\n");
    CHECK_CONDITION(
        SetRestoreMethod(
            pMetadata,
            VSS_RME_RESTORE_IF_CAN_REPLACE,     // VSS_RESTOREMETHOD_ENUM
            NULL,                               // Service name, if method is VSS_RME_STOP_RESTORE_RESTART
            false),                             // Reboot required
        L"SetRestoreMethod failed");

    //
    // Add components and files
    //
    wprintf(L"INFO: Adding basic component...\n");

    CHECK_CONDITION(
        AddComponent(
            pMetadata,
            VSS_CT_FILEGROUP,                   // VSS_COMPONENT_TYPE
            wszComponent),                      // Component name
        L"AddComponent failed");

    CHECK_CONDITION(
        AddFilesToFileGroup(
            pMetadata,
            wszComponent,                       // Component name
            L"c:\\xwdata",                      // Directory covered
            L"*.*",                             // Files covered
            true,                               // Recursive
            VSS_FSBT_ALL_SNAPSHOT_REQUIRED |
            VSS_FSBT_FULL_BACKUP_REQUIRED),     // Backup type mask
        L"AddFilesToFileGroup failed");

    bResult = true;

_exit:
    return bResult;
}


