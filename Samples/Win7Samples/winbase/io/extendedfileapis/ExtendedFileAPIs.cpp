//======================================================================
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
// KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE 
// IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
// PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// This sample demonstrates how to query information about files and
// directories using the GetFileInformationByHandle API as well as how
// to open a file by ID using OpenFileById.
//
//=====================================================================

#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

VOID 
PrintUsage() {
    printf("Usage: ExtendedFileAPIs [-id] targetFile\n\n");
    printf("  Display extended information about the target file or directory\n");
    printf("  using the GetFileInformationByHandleEx API.\n\n");
    printf("  -id  If this flag is specified the target file is assumed to be a file ID\n");
    printf("       and the program will attempt to open the file using OpenFileById.\n");
    printf("       The current directory will be used to determine which volume to scope\n");
    printf("       the open to.\n");
}

VOID
PrintFileAttributes(
    ULONG FileAttributes
    )
{
    if (FileAttributes & FILE_ATTRIBUTE_ARCHIVE) {
        printf("Archive ");
        FileAttributes &= ~FILE_ATTRIBUTE_ARCHIVE;
    }
    if (FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        printf("Directory ");
        FileAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;
    }
    if (FileAttributes & FILE_ATTRIBUTE_READONLY) {
        printf("Read-Only ");
        FileAttributes &= ~FILE_ATTRIBUTE_READONLY;
    }
    if (FileAttributes & FILE_ATTRIBUTE_HIDDEN) {
        printf("Hidden ");
        FileAttributes &= ~FILE_ATTRIBUTE_HIDDEN;
    }
    if (FileAttributes & FILE_ATTRIBUTE_SYSTEM) {
        printf("System ");
        FileAttributes &= ~FILE_ATTRIBUTE_SYSTEM;
    }
    if (FileAttributes & FILE_ATTRIBUTE_NORMAL) {
        printf("Normal ");
        FileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
    }
    if (FileAttributes & FILE_ATTRIBUTE_TEMPORARY) {
        printf("Temporary ");
        FileAttributes &= ~FILE_ATTRIBUTE_TEMPORARY;
    }
    if (FileAttributes & FILE_ATTRIBUTE_COMPRESSED) {
        printf("Compressed ");
        FileAttributes &= ~FILE_ATTRIBUTE_COMPRESSED;
    }

    if (FileAttributes) {
        printf("  Additional Attributes: %x", FileAttributes);
    }

    printf("\n");
}
BOOL
PrintDate(
    LARGE_INTEGER Date
    )
{
    BOOL result;
    FILETIME fileTime;
    SYSTEMTIME systemTime;

    fileTime.dwLowDateTime = Date.LowPart;
    fileTime.dwHighDateTime = Date.HighPart;

    result = FileTimeToLocalFileTime( &fileTime,
                                      &fileTime );

    if (!result) {
        return result;
    }

    result = FileTimeToSystemTime( &fileTime,
                                  &systemTime );

    if (!result) {
        return result;
    }

    printf("%.2d/%.2d/%.4d %.2d:%.2d", 
           systemTime.wMonth, 
           systemTime.wDay, 
           systemTime.wYear, 
           systemTime.wHour, 
           systemTime.wMinute);

    return true;
}

BOOL
DisplayBasicInfo( 
    HANDLE hFile,
    BOOL * bIsDirectory
    )
{
    FILE_BASIC_INFO basicInfo;
    BOOL result;

    result = GetFileInformationByHandleEx( hFile,
                                               FileBasicInfo,
                                               &basicInfo,
                                               sizeof(basicInfo));

    if (!result) {
        printf("Failure fetching basic information: %d\n", GetLastError());
        return result;
    }

    printf("\n[Basic Information]\n\n");

    printf("  Creation Time: ");

    result = PrintDate(basicInfo.CreationTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving creation time.\n");
    }

    printf("  Change Time: ");

    result = PrintDate(basicInfo.ChangeTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving creation time.\n");
    }

    printf("  Last Access Time: ");

    result = PrintDate(basicInfo.LastAccessTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving last access time.\n");
    }

    printf("  Last Write Time: ");

    result = PrintDate(basicInfo.LastWriteTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving last write time.\n");
    }

    printf("  File Attributes: ");
    PrintFileAttributes(basicInfo.FileAttributes);

    if (basicInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        *bIsDirectory = true;
    } else {
        *bIsDirectory = false;
    }
    return result;
}

BOOL
DisplayStandardInfo(
    HANDLE hFile
    )
{
    FILE_STANDARD_INFO standardInfo;
    BOOL result;

    result = GetFileInformationByHandleEx( hFile,
                                           FileStandardInfo,
                                           &standardInfo,
                                           sizeof(standardInfo));

    if (!result) {
        printf("Failure fetching standard information: %d\n", GetLastError());
        return result;
    }

    printf("\n[Standard Information]\n\n");

    printf("  Allocation Size: %I64d\n", standardInfo.AllocationSize);
    printf("  End of File: %I64d\n", standardInfo.EndOfFile);
    printf("  Number of Links: %d\n", standardInfo.NumberOfLinks);
    printf("  Delete Pending: ");
    if (standardInfo.DeletePending) {
        printf("Yes\n");
    } else {
        printf("No\n");
    }
    printf("  Directory: ");
    if (standardInfo.Directory) {
        printf("Yes\n");
    } else {
        printf("No\n");
    }

    return true;
}

BOOL
DisplayNameInfo(
    HANDLE hFile
    )
{
    PFILE_NAME_INFO nameInfo;
    ULONG nameSize;
    BOOL result;

    //
    // Allocate an information structure that is hopefully large enough to
    // retrieve name information.
    //

    nameSize = sizeof(FILE_NAME_INFO) + (sizeof(WCHAR) * MAX_PATH);

retry:

    nameInfo = (PFILE_NAME_INFO) LocalAlloc(LMEM_ZEROINIT,
                                                nameSize);


    if (nameInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return false;
    }

    result = GetFileInformationByHandleEx( hFile,
                                               FileNameInfo,
                                               nameInfo,
                                               nameSize );

    if (!result) {
        //
        // If our buffer wasn't large enough try again with a larger one.
        //
        if (GetLastError() == ERROR_MORE_DATA) {
            nameSize *= 2;
            goto retry;
        }

        printf("Failure fetching name information: %d\n", GetLastError());
        LocalFree(nameInfo);
        return result;
    }

    printf("\n[Name Information]\n\n");

    printf("  File Name: %S\n", nameInfo->FileName);

    LocalFree(nameInfo);
    return true;
}

BOOL
DisplayStreamInfo(
    HANDLE hFile
    )
{
    PFILE_STREAM_INFO currentStreamInfo;
    BOOL result;
    PFILE_STREAM_INFO streamInfo;
    ULONG streamInfoSize;


    //
    // Allocate an information structure that is hopefully large enough to
    // retrieve stream information.
    //

    streamInfoSize = sizeof(FILE_STREAM_INFO) + (sizeof(WCHAR) * MAX_PATH);

retry:

    streamInfo = (PFILE_STREAM_INFO) LocalAlloc(LMEM_ZEROINIT,
                                                    streamInfoSize);


    if (streamInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return false;
    }

    result = GetFileInformationByHandleEx( hFile,
                                               FileStreamInfo,
                                               streamInfo,
                                               streamInfoSize );

    if (!result) {
        //
        // If our buffer wasn't large enough try again with a larger one.
        //
        if (GetLastError() == ERROR_MORE_DATA) {
            streamInfoSize *= 2;
            goto retry;
        }

        printf("Failure fetching stream information: %d\n", GetLastError());
        LocalFree(streamInfo);
        return result;
    }

    printf("\n[Stream Information]\n\n");

    currentStreamInfo = streamInfo;

    do {
        printf("  Stream Name: %S\n", currentStreamInfo->StreamName);
        printf("  Stream Size: %I64d\n", currentStreamInfo->StreamSize);
        printf("  Stream Allocation Size: %I64d\n", 
               currentStreamInfo->StreamAllocationSize);

        if (currentStreamInfo->NextEntryOffset == 0) {
            currentStreamInfo = NULL;
        } else {
            currentStreamInfo = 
                (PFILE_STREAM_INFO) ((PUCHAR)currentStreamInfo +
                                     currentStreamInfo->NextEntryOffset);
        }
    } while (currentStreamInfo != NULL);


    LocalFree(streamInfo);
    return true;
}

void
PrintDirectoryEntry(
    PFILE_ID_BOTH_DIR_INFO entry
    )
{
    WCHAR lastChar;
    BOOL result;

    //
    // The names aren't necessarily NULL terminated, so we deal with that here.
    //
    lastChar = entry->FileName[(entry->FileNameLength / sizeof(WCHAR))];
        entry->FileName[(entry->FileNameLength / sizeof(WCHAR))] = L'\0';

    printf("\n  %S ", entry->FileName);

    entry->FileName[(entry->FileNameLength / sizeof(WCHAR))] = lastChar;

    if (entry->ShortName[0] != L'\0') {
        lastChar = entry->ShortName[(entry->ShortNameLength / sizeof(WCHAR))];
        entry->ShortName[(entry->ShortNameLength / sizeof(WCHAR))] = L'\0';

        printf("[%S]\n\n", entry->ShortName);

        entry->ShortName[(entry->ShortNameLength / sizeof(WCHAR))] = lastChar;
    } else {
        printf("\n\n");
    }

    printf("    Creation Time: ");

    result = PrintDate(entry->CreationTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving creation time.\n");
    }

    printf("    Change Time: ");

    result = PrintDate(entry->ChangeTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving creation time.\n");
    }

    printf("    Last Access Time: ");

    result = PrintDate(entry->LastAccessTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving last access time.\n");
    }

    printf("    Last Write Time: ");

    result = PrintDate(entry->LastWriteTime);

    if (result) {
        printf("\n");
    } else {
        printf(" Error retrieving last write time.\n");
    }

    printf("    End of File: %I64d\n", entry->EndOfFile);
    printf("    Allocation Size: %I64d\n", entry->AllocationSize);
    printf("    File Attributes: ");
    PrintFileAttributes(entry->FileAttributes);
    printf("    File ID: %I64d\n", entry->FileId);
}

BOOL
DisplayFullDirectoryInfo(
     HANDLE hFile
     )
{
    PFILE_ID_BOTH_DIR_INFO currentDirInfo;
    PFILE_ID_BOTH_DIR_INFO dirInfo;
    ULONG dirInfoSize;
    FILE_INFO_BY_HANDLE_CLASS infoClass;
    BOOL result;

    //
    // Allocate an information structure that is hopefully large enough to
    // retrieve at least one directory entry.
    //

    dirInfoSize = sizeof(FILE_ID_BOTH_DIR_INFO) + (sizeof(WCHAR) * MAX_PATH);

    //
    // We initially want to start our enumeration from the beginning so we
    // use the restart class.
    //
    infoClass = FileIdBothDirectoryRestartInfo;

retry:

    dirInfo = (PFILE_ID_BOTH_DIR_INFO) LocalAlloc(LMEM_ZEROINIT,
                                             dirInfoSize);


    if (dirInfo == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return false;
    }

	for (;;) {
        result = GetFileInformationByHandleEx( hFile,
                                               infoClass,  
                                               dirInfo,
                                               dirInfoSize );

        if (!result) {
            //
            // If our buffer wasn't large enough try again with a larger one.
            //
            if (GetLastError() == ERROR_MORE_DATA) {
                dirInfoSize *= 2;
                goto retry;
            } else if (GetLastError() == ERROR_NO_MORE_FILES) {
                //
                // Enumeration completed successfully, we simply break out here.
                //
                break;
            }

            //
            // A real error occurred.
            //
            printf("\nFailure fetching directory information: %d\n", 
                   GetLastError());

            LocalFree(dirInfo);
            return result;
        }

        if (infoClass == FileIdBothDirectoryRestartInfo) {
            printf("\n[Full Directory Information]\n\n");
            infoClass = FileIdBothDirectoryInfo;
        }

        currentDirInfo = dirInfo;

        do {
            PrintDirectoryEntry(currentDirInfo);
            if (currentDirInfo->NextEntryOffset == 0) {
                currentDirInfo = NULL;
            } else {
                currentDirInfo = 
                    (PFILE_ID_BOTH_DIR_INFO) ((PUCHAR)currentDirInfo + 
                                              currentDirInfo->NextEntryOffset);
            }
        } while (currentDirInfo != NULL);

    }
    LocalFree(dirInfo);

    return true;
}

int __cdecl wmain( __in int argc, __in_ecount(argc) WCHAR* argv[])
{
    BOOL bIsDirectory;
    BOOL bResult;
    HANDLE hFile;
    FILE_ID_DESCRIPTOR fileId;


    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    //
    // Check for flag that indicates we should open by ID
    //
    if (CompareStringW(LOCALE_INVARIANT,
                       NORM_IGNORECASE,
                       argv[1],
                       -1,
                       L"-id", 
                       -1) == CSTR_EQUAL) {

        HANDLE hDir;
        
        if (argc < 3) {
            PrintUsage();
            return 1;
        }

        //
        // Open a handle to the current directory to use as a hint when
        // opening the file by ID.
        //
        hDir = CreateFileW(L".",
                           GENERIC_READ,
                           FILE_SHARE_READ | 
                           FILE_SHARE_WRITE | 
                           FILE_SHARE_DELETE,
                           NULL,
                           OPEN_EXISTING,
                           FILE_FLAG_BACKUP_SEMANTICS,
                           NULL);

        if (hDir == INVALID_HANDLE_VALUE) {
            printf("Couldn't open current directory.\n");
            return 1;
        }

        //
        // Capture the file ID and attempt to open by ID.
        //
        fileId.FileId.QuadPart = _wtoi64(argv[2]);
        fileId.Type = FileIdType;
        fileId.dwSize = sizeof(fileId);

        hFile = OpenFileById(hDir,
                             &fileId,
                             GENERIC_READ,
                             FILE_SHARE_READ | 
                             FILE_SHARE_WRITE | 
                             FILE_SHARE_DELETE,
                             NULL,
                             FILE_FLAG_BACKUP_SEMANTICS);

        CloseHandle(hDir);

        if (hFile == INVALID_HANDLE_VALUE) {
            printf("\nError opening file with ID %s.  Last error was %d.\n", 
                    argv[2], 
                    GetLastError());

            return 1;
        }

    } else {

        hFile = CreateFileW(argv[1],
                            GENERIC_READ,
                            FILE_SHARE_READ |
                            FILE_SHARE_WRITE | 
                            FILE_SHARE_DELETE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_BACKUP_SEMANTICS,
                            NULL);

        if (hFile == INVALID_HANDLE_VALUE) {
            printf("\nError opening file %s.  Last error was %d.\n", 
                    argv[1], 
                    GetLastError());

            return 1;
        }
    }



    //
    // Display information about the file/directory
    //
    bResult = DisplayBasicInfo(hFile, &bIsDirectory);

    if (!bResult) {

        printf("\nError displaying basic information.\n");
        return 1;
    }

    bResult = DisplayStandardInfo(hFile);

    if (!bResult) {

        printf("\nError displaying standard information.\n");
        return 1;
    }

    bResult = DisplayNameInfo(hFile);

    if (!bResult) {

        printf("\nError displaying name information.\n");
        return 1;
    }

    //
    // For directories we query for full directory information, which gives us
    // various pieces of information about each entry in the directory.
    //
    if (bIsDirectory) {

        bResult = DisplayFullDirectoryInfo(hFile);

        if (!bResult) {

            printf("\nError displaying directory information.\n");
            return 1;
        }

    //
    // Otherwise we query information about the streams associated with the 
    // file.
    //
    } else {

        bResult = DisplayStreamInfo(hFile);

        if (!bResult) {

            printf("\nError displaying stream information.\n");
            return 1;
        }
    }

    CloseHandle(hFile);


    return 0;
}

