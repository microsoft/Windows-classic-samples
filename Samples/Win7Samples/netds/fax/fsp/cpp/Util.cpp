//==========================================================================
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//--------------------------------------------------------------------------

//
//This module implements utility functions for the fax service provider
//


#include "SampleFSP.h"

//+---------------------------------------------------------------------------
//
//  function:   OpenLogFile
//
//  Synopsis:   Open the log file
//
//  Arguments:  [bLoggingEnabled] - indicates if logging is enabled
//				[lpszLoggingDirectory] - indicates the logging directory
//				[pDeviceInfo] - pointer to the virtual fax devices
//
//  Returns:     TRUE on success
//
//----------------------------------------------------------------------------

BOOL
OpenLogFile(
                BOOL    bLoggingEnabled,
                LPWSTR  lpszLoggingDirectory
           )
{
        // szLoggingFilename is the logging file name
        WCHAR  szLoggingFilename[MAX_PATH]={0};
        // cUnicodeBOM is the Unicode BOM
        WCHAR  cUnicodeBOM = 0xFEFF;
        DWORD  dwSize;
        HRESULT hr = S_OK;
        BOOL bRetVal = FALSE;

        if (bLoggingEnabled == TRUE) {
                // Set the logging file name
                if (wcslen(lpszLoggingDirectory) >= (sizeof(szLoggingFilename)/sizeof(szLoggingFilename[0]) - 2 - wcslen(NEWFSP_LOG_FILE))) { 
                        // directory path is too long, consider changing szLoggingFilename size
                        return FALSE;
                }
                hr = StringCchCopy(szLoggingFilename,MAX_PATH,lpszLoggingDirectory );
                if(hr != S_OK)
                {
                        WriteDebugString( L"StringCchCopy failed, hr = 0x%x for szLoggingFilename", hr );
                        bRetVal = FALSE;
                        goto Exit;
                }
                hr = StringCchCat(szLoggingFilename,MAX_PATH,L"\\" );
                if(hr != S_OK)
                {
                        WriteDebugString( L"StringCchCat failed, hr = 0x%x for szLoggingFilename", hr );
                        bRetVal = FALSE;
                        goto Exit;
                }
                hr = StringCchCat(szLoggingFilename,MAX_PATH,NEWFSP_LOG_FILE );
                if(hr != S_OK)
                {
                        WriteDebugString( L"StringCchCat failed, hr = 0x%x for szLoggingFilename", hr );
                        bRetVal = FALSE;
                        goto Exit;
                }

                // Create the new log file
                g_hLogFile = CreateFile(szLoggingFilename,
                                GENERIC_WRITE,
                                FILE_SHARE_READ,
                                NULL,
                                CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);
                if (g_hLogFile == INVALID_HANDLE_VALUE) {
                        goto Exit;
                }

                // Write the Unicode BOM to the log file
                WriteFile(g_hLogFile,
                                &cUnicodeBOM,
                                sizeof(WCHAR),
                                &dwSize,
                                NULL);
                bRetVal = TRUE;
        }
        else {
                g_hLogFile = INVALID_HANDLE_VALUE;
        }

Exit:
        return bRetVal;
}

//+---------------------------------------------------------------------------
//
//  function:    CloseLogFile
//
//  Synopsis:    Close the log file
//
//  Arguments:   None
//
//  Returns:     None
//
//----------------------------------------------------------------------------

VOID CloseLogFile()
{
        if (g_hLogFile != INVALID_HANDLE_VALUE) {
                CloseHandle(g_hLogFile);
                g_hLogFile = INVALID_HANDLE_VALUE;
        }
}

//+---------------------------------------------------------------------------
//
//  function:    WriteDebugString
//
//  Synopsis:    Write a debug string to the debugger and log file
//
//  Arguments:   [lpszFormatString] - pointer to the string
//
//  Returns:     None
//
//----------------------------------------------------------------------------
VOID WriteDebugString( LPWSTR  lpszFormatString, ... )     
{
        va_list     varg_ptr = NULL;
        SYSTEMTIME  SystemTime;
        HRESULT hr = S_OK;
        BOOL bRetVal = FALSE;
        // szOutputString is the output string
        WCHAR       szOutputString[1024] = {0};
        DWORD       cb;

        // Initialize the buffer
        ZeroMemory(szOutputString, sizeof(szOutputString));

        // Get the current time
        GetLocalTime(&SystemTime);
        hr = StringCchPrintf(szOutputString,1024,  L"%02d.%02d.%04d@%02d:%02d:%02d.%03d:\n", 
                        SystemTime.wMonth,
                        SystemTime.wDay,
                        SystemTime.wYear,
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond, 
                        SystemTime.wMilliseconds);
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchPrintf failed, hr = 0x%x for szOutputString", hr );                
                goto Exit;
        }       
        cb = lstrlen(szOutputString);

        va_start(varg_ptr, lpszFormatString);
        hr = StringCchVPrintf(&szOutputString[cb],(sizeof(szOutputString)/sizeof(szOutputString[0])) - cb -1,
                        lpszFormatString,
                        varg_ptr); 
        if(hr != S_OK)
        {
                WriteDebugString( L"StringCchVPrintf failed, hr = 0x%x for szOutputString", hr );                
                goto Exit;
        }

        // Write the string to the debugger
        OutputDebugString(szOutputString);
        if (g_hLogFile != INVALID_HANDLE_VALUE) {
                // Write the string to the log file
                WriteFile(g_hLogFile,
                                szOutputString,
                                lstrlen(szOutputString) * sizeof(WCHAR),
                                &cb,
                                NULL);
        }

Exit:
        if(varg_ptr)
                va_end(varg_ptr);
}

//+---------------------------------------------------------------------------
//
//  function:   PostJobStatus
//
//  Synopsis:   Post a completion packet for a fax service provider fax job status change
//
//  Arguments:  [CompletionPort] - specifies a handle to an I/O completion port
//				[CompletionKey] - specifies a completion port key value
//				[StatusId] - specifies a fax status code
//				[ErrorCode] - specifies one of the Win32 error codes that the fax service provider should use to report an error that occurs
//
//  Returns:    TRUE on success
//
//----------------------------------------------------------------------------

VOID
PostJobStatus(
                HANDLE     CompletionPort,
                ULONG_PTR  CompletionKey,
                DWORD      StatusId,
                DWORD      ErrorCode
             )
{
        // pFaxDevStatus is a pointer to the completion packet
        PFAX_DEV_STATUS  pFaxDevStatus;

        // Allocate a block of memory for the completion packet
        pFaxDevStatus = (PFAX_DEV_STATUS) MemAllocMacro(sizeof(FAX_DEV_STATUS));
        if (pFaxDevStatus != NULL) {
                // Set the completion packet's structure size
                pFaxDevStatus->SizeOfStruct = sizeof(FAX_DEV_STATUS);
                // Copy the completion packet's fax status identifier
                pFaxDevStatus->StatusId = StatusId;
                // Set the completion packet's string resource identifier to 0
                pFaxDevStatus->StringId = 0;
                // Set the completion packet's current page number to 0
                pFaxDevStatus->PageCount = 0;
                // Set the completion packet's remote fax device identifier to NULL
                pFaxDevStatus->CSI = NULL;
                // Set the completion packet's calling fax device identifier to NULL
                pFaxDevStatus->CallerId = NULL;
                // Set the completion packet's routing string to NULL
                pFaxDevStatus->RoutingInfo = NULL;
                // Copy the completion packet's Win32 error code
                pFaxDevStatus->ErrorCode = ErrorCode;

                // Post the completion packet
                PostQueuedCompletionStatus( CompletionPort,
                                sizeof(FAX_DEV_STATUS),
                                CompletionKey,
                                (LPOVERLAPPED) pFaxDevStatus);
        }
}
