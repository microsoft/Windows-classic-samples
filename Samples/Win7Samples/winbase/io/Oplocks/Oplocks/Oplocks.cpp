// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "targetver.h"

#include <windows.h>
#include <winioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>


/*
 * This code sample shows the usage of the new oplock levels introduced in Windows 7.
 *
 * The purpose of this program is to produce a checksum of each file in the directory,
 * while ensuring that it does not interfere with other programs writing/deleting
 * these files.
 *
 * To achieve this goal, the program takes a directory as a command line argument, 
 * enumerates the files in the directory using the FindFirstFileEx()/FindNextFile() API, 
 * opens each file, obtains the new (CACHE_OPLOCK_LEVEL_READ | CACHE_OPLOCK_LEVEL_HANDLE)
 * (shortened as RH  in the comments) oplock, and attempts to read the entire file 
 * sequentially, while holding this oplock.
 *
 * If any other process attempts to open the file for writing/deleting, the oplock would
 * break, and the open attempt by that process would be blocked until this program
 * acknowledges the break or closes the file handle. This program checks the status of 
 * the oplock and abandons the checksum operation if a break occurs.
 *
 * While holding the oplock, all I/O that we issue must be asynchronous. If we ever
 * wait for any I/O to complete, we must monitor simultaneously for oplock breaks 
 * as well. Limiting the operations that are performed while holding an oplock will
 * make it is easier to avoid deadlock scenarios.
 *
 * The key routines in this program are:
 *      SimpleCheckSumAlgorithm          
 *           Implements a simple checksum algorithm
 *
 *      PerformCheckSumOperation
 *           Reads and computes the checksum on a single file, while holding the
 *           oplock to detect when the checksum operation should be abandoned.
 *
 *      ScanAndComputeCheckSum
 *           Enumerates the contents of the directory passed to it, and calls
 *           the PerformCheckSumOperation routine to compute the checksum of
 *           each file.
 */


#define CHECKSUM_BLOCK_SIZE 512

/*
 * This is the structure used for encapsulating oplock related information
 * maintained by this program.
 *
 */
  
typedef struct _OPLOCK_COMMUNICATION {
    OVERLAPPED OverLapped;
    REQUEST_OPLOCK_INPUT_BUFFER Request;
} OPLOCK_COMMUNICATION, *POPLOCK_COMMUNICATION;


/*
 *   Extremely simple checksum routine, not intended to be
 *   used in real applications
 */
DWORD SimpleCheckSumAlgorithm(PVOID Buffer, DWORD Length, DWORD PartialCheckSum, DWORD * Counter)
{
    DWORD i;
    PUCHAR TmpBuffer = (PUCHAR) Buffer;
    for (i = 0 ; i < Length ; i++) {
        PartialCheckSum = ((PartialCheckSum << 8)|(PartialCheckSum >> 24)) ^ TmpBuffer[i];
    }
    *Counter += i;
    return PartialCheckSum;
}


/*
 *  This is the routine that performs the checksum operation on 
 *  a single file. This routine wants to ensure that the checksum that 
 *  it computes is valid, therefore it opens the file without sharing 
 *  it for DELETE or WRITE.
 *
 *  Without Oplocks, any other process attempting to open this file 
 *  for DELETE or WRITE will receive a sharing violation. By obtaining
 *  an oplock level of CACHE_OPLOCK_LEVEL_READ | CACHE_OPLOCK_LEVEL_HANDLE,
 *  it ensures that when any create that is incompatable with our 
 *  sharing mode is issued, the create request will be pended and this 
 *  routine will be notified via the breaking of the oplock.
 *  When this routine acknowledges the break of the oplock by closing
 *  the handle, the pending open operation will be allowed to proceed,
 *  and this would be completely transparent to the other process.
 */
VOID PerformCheckSumOperation(TCHAR * FilePath, PVOID Buffer, POPLOCK_COMMUNICATION OplockInfo)
{
    HANDLE FileHandle;
    REQUEST_OPLOCK_OUTPUT_BUFFER Response;
    BOOL bSuccess = FALSE;
    BOOL ResultValid = TRUE;
    DWORD PartialCheckSum = 0;
    DWORD BytesSummedCounter = 0;
    HANDLE KeyEvents[2];
    OVERLAPPED ReadOverLapped;
    LARGE_INTEGER Offset = {0};
    DWORD RetVal;


    //
    // Open the file for Asynchronous I/O
    //

    FileHandle = CreateFile( FilePath,
                             GENERIC_READ,
                             FILE_SHARE_READ, 
                             NULL,
                             OPEN_EXISTING,
                             FILE_FLAG_OVERLAPPED,
                             NULL );

    if (INVALID_HANDLE_VALUE == FileHandle) {
        _tprintf(_T("Error 0x%x Opening %s\n"), GetLastError(), FilePath );
        return;
    }   

    //
    //   Obtain an RH Oplock on the file. This Oplock will break to a level of
    //   NONE if any writes are issued, and will break to a level of "R" if a
    //   create with an incompatabile sharing mode arrives. In our case, since
    //   we are not willing to share WRITE or DELETE access, any process that
    //   opens the file we are working with for either delete or write access
    //   will trigger an oplock break.
    //

    ResetEvent( OplockInfo->OverLapped.hEvent );

    bSuccess = DeviceIoControl( FileHandle,
                                FSCTL_REQUEST_OPLOCK,
                                &OplockInfo->Request,
                                sizeof( OplockInfo->Request ),
                                &Response,
                                sizeof( Response ),
                                NULL,
                                &OplockInfo->OverLapped ); 

    //
    // If the above routine returns FALSE, and the last error is ERROR_IO_PENDING,
    // we have obtained the oplock.
    //

    if (bSuccess || (ERROR_IO_PENDING != GetLastError())) {
        _tprintf( _T("We failed to acquire the oplock, the error code was 0x%x\n"), GetLastError() );
        CloseHandle(FileHandle);
        return;
    }


    //
    // We obtained the Oplock and may safely operate on the file, secure in 
    // the knowledge that it cannot change while we are working on it.
    // We now read through the file, checksumming as we go.
    //

    ReadOverLapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if (ReadOverLapped.hEvent == INVALID_HANDLE_VALUE) {
        _tprintf(_T("Failed to create event"));
        CloseHandle(FileHandle);
        return;
    }

    //
    // Setup the event structure for the WaitForMultipleObjects Call.
    //

    KeyEvents[0] =  OplockInfo->OverLapped.hEvent;
    KeyEvents[1] =  ReadOverLapped.hEvent;

    while (ResultValid){
        
        DWORD BytesRead;

        ReadOverLapped.Offset = Offset.LowPart;
        ReadOverLapped.OffsetHigh = Offset.HighPart;

        bSuccess = ReadFile( FileHandle,
                             Buffer,
                             CHECKSUM_BLOCK_SIZE,
                             &BytesRead,
                             &ReadOverLapped );

        if (!bSuccess) {

            if (ERROR_IO_PENDING != GetLastError()) {

                if (ERROR_HANDLE_EOF == GetLastError()) {  
                    //
                    // We reached end of this file
                    //
                    break;
                } else {
                    _tprintf( _T("Error 0x%x, while reading the file\n"), GetLastError() );
                    ResultValid = FALSE;
                    break;
                }
            }

            //
            // We wait simultaneously for the read to complete and for the
            // oplock to break. If either of these events occur, our wait
            // will be satisfied.
            //
            // !!! IMPORTANT NOTE !!!
            // It is important to ensure that we do not perform any
            // operation that may result in us blocking while we hold an
            // oplock. All I/O that we perform while we hold the oplock 
            // should be asynchronous. If that is not possible, then such
            // operations should be performed in a separate thread.
            // 
            // If we are waiting for any I/O to complete in this thread
            // we must use the WaitForMultipleObjects() API to monitor
            // simultaneously for the oplock break as well.
            //

            RetVal = WaitForMultipleObjects( 2, KeyEvents, FALSE, INFINITE );

            //
            // WaitForMultipleObjects returns the lowest numbered event
            // that has entered the signalled state. If the return
            // value indicates that the lowest signalled event is NOT the
            // event associated with the Read operation, the oplock must
            // have broken.
            //
            
            if (RetVal != WAIT_OBJECT_0 + 1 ) {
                ResultValid = FALSE;
                _tprintf( _T( "Oplock Break: From 0x%x to 0x%x before we finished reading.\n"),
                               Response.OriginalOplockLevel,
                               Response.NewOplockLevel );                
                break;
            }
            
            if (!GetOverlappedResult( FileHandle,
                                      &ReadOverLapped,
                                      &BytesRead,
                                      FALSE )) {

                if (ERROR_HANDLE_EOF == GetLastError()) {  
                    //
                    // We reached end of this file
                    //
                    break;
                } else {
                    _tprintf( _T("Asynchronous Error 0x%x, while reading the file\n"), GetLastError() );
                    ResultValid = FALSE;
                    break;
                }
            }
                
        } 
                    
        //
        // Compute the checksum
        //

        PartialCheckSum = SimpleCheckSumAlgorithm( Buffer,
                                                   BytesRead,
                                                   PartialCheckSum,
                                                   &BytesSummedCounter);
                    
        //
        // Advance to the next offset in the file.
        //

        Offset.QuadPart += BytesRead;

    }


    CloseHandle( ReadOverLapped.hEvent );

    if (ResultValid) {

        _tprintf(_T("Checksum is 0x%08x, Analyzed %d bytes.\n"),PartialCheckSum,BytesSummedCounter);

    } else {

        _tprintf(_T("Checksum is unknown\n"));
    }
        
    //
    //  This Close will either:
    //   Release the oplock (If the oplock hadn't been broken)
    //   (OR)
    //   Acknowledge the oplock break (If the oplock had broken)
    //

    CloseHandle( FileHandle );
}

/*
 *  This routine will be passed the path to a directory. It will compute the 
 *  checksum of all the files in that directory by calling the 
 *  PerformChecksumOperation() routine on each file.
 *
 */
 
void ScanAndComputeCheckSum(_TCHAR * Path)
{
    HANDLE FileEnumerationHandle;
    WIN32_FIND_DATA FindFileData;
    BOOL MoreFiles = FALSE;

    WCHAR SearchString[MAX_PATH + 2];
    WCHAR FilePath[MAX_PATH + 2];

    OPLOCK_COMMUNICATION OplockInfo = {0};
    PVOID Buffer = NULL;

    //
    //  Initialize the oplock communication structure
    //

    OplockInfo.Request.StructureVersion = REQUEST_OPLOCK_CURRENT_VERSION;
    OplockInfo.Request.StructureLength = sizeof(REQUEST_OPLOCK_INPUT_BUFFER);
    OplockInfo.Request.RequestedOplockLevel = (OPLOCK_LEVEL_CACHE_READ | OPLOCK_LEVEL_CACHE_HANDLE);
    OplockInfo.Request.Flags = REQUEST_OPLOCK_INPUT_FLAG_REQUEST;

    OplockInfo.OverLapped.hEvent = CreateEvent( NULL,
                                                TRUE,
                                                FALSE,
                                                NULL );

    if (INVALID_HANDLE_VALUE == OplockInfo.OverLapped.hEvent) {

        _tprintf(_T("Failed to create oplock event\n"));
        return;
    }

    //
    // Allocate a buffer to work on the file
    //

    Buffer = malloc( CHECKSUM_BLOCK_SIZE );

    if (NULL == Buffer) {

        _tprintf(_T("Failed to allocate memory for read buffer"));
        CloseHandle( OplockInfo.OverLapped.hEvent );
        return;
    }

    //
    // Prepare the search String for the directory enumeration API
    //
    _stprintf_s( SearchString, ARRAYSIZE(SearchString), _T("%s\\*"), Path );

    ZeroMemory( &FindFileData, sizeof( FindFileData ) );

    FileEnumerationHandle = FindFirstFileEx( SearchString,
                                             FindExInfoBasic,
                                             &FindFileData,
                                             FindExSearchNameMatch,
                                             NULL,
                                             FIND_FIRST_EX_LARGE_FETCH );

    if (INVALID_HANDLE_VALUE == FileEnumerationHandle) {

        _tprintf(_T("Unable to list files under %s"),Path);
        free( Buffer );
        CloseHandle( OplockInfo.OverLapped.hEvent );
        return;
    }

    do {
        _tprintf(_T("Processing File %s\n"), FindFileData.cFileName);

        //
        // Construct full path to the file.
        //

        _stprintf_s(FilePath, ARRAYSIZE(FilePath), _T("%s\\%s"), Path , FindFileData.cFileName);

        PerformCheckSumOperation( FilePath, Buffer, &OplockInfo );
        
        MoreFiles = FindNextFile( FileEnumerationHandle, &FindFileData );
    } while ( MoreFiles );

    FindClose( FileEnumerationHandle );
    free( Buffer );
    CloseHandle( OplockInfo.OverLapped.hEvent );
}


//
// The entrypoint for this program. Takes a single command line argument,
// which is the name of the directory containing the files to be checksummed.
//

INT __cdecl _tmain(INT argc, WCHAR* argv[])
{
    _tprintf(_T("Windows-7 Oplocks SDK Sample Program.\n"));

    if (argc > 1) {

        ScanAndComputeCheckSum( argv[1] );

    } else {

        _tprintf(_T("Supply the path of the directory to scan files for generating checksum\n"));
    }

    return 0;
}

