/*++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (C) 1994 - 2000.  Microsoft Corporation.  All rights reserved.


Module Name:

    unbufcp2.c

Abstract:

    Intended to demonstrate how to complete I/O in a different thread
    asynchronously than the I/O was started from.  This is useful for
    people who want a more powerful mechanism of asynchronous completion
    callbacks than Win32 provides (i.e. VMS developers who are used to ASTs).

    Two threads are used. The first thread posts overlapped reads from the
    source file. These reads complete to an I/O completion port the second
    thread is waiting on. The second thread sees the I/O completion and
    posts an overlapped write to the destination file. The write completes
    to another I/O completion port that the first thread is waiting on.
    The first thread sees the I/O completion and posts another overlapped
    read.

    Thread 1                                        Thread 2
       |                                               |
       |                                               |
    kick off a few                           -->GetQueuedCompletionStatus(ReadPort)
    overlapped reads                         |         |
       |                                     |         |
       |                                     |  read has completed,
  ->GetQueuedCompletionStatus(WritePort)     |  kick off the corresponding
  |    |                                     |  write.
  | write has completed,                     |         |
  | kick off another                         |_________|
  | read
  |    |
  |____|

--*/

#ifdef _IA64_
#pragma warning(disable:4100 4127)
#endif

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//
// File handles for the copy operation. All read operations are
// from SourceFile. All write operations are to DestFile.
//
HANDLE SourceFile;
HANDLE DestFile;

//
// I/O completion ports. All reads from the source file complete
// to ReadPort. All writes to the destination file complete to
// WritePort.
//
HANDLE ReadPort;
HANDLE WritePort;

//
//version information
//
OSVERSIONINFO     ver;

//
// Structure used to track each outstanding I/O. The maximum
// number of I/Os that will be outstanding at any time is
// controllable by the MAX_CONCURRENT_IO definition.
//

#define MAX_CONCURRENT_IO 20

typedef struct _COPY_CHUNK {
    OVERLAPPED Overlapped;
    LPVOID Buffer;
} COPY_CHUNK, *PCOPY_CHUNK;

COPY_CHUNK CopyChunk[MAX_CONCURRENT_IO];

//
// Define the size of the buffers used to do the I/O.
// 64K is a nice number.
//
#define BUFFER_SIZE (64*1024)

//
// The system's page size will always be a multiple of the
// sector size. Do all I/Os in page-size chunks.
//
DWORD PageSize;


//
// Local function prototypes
//
VOID
WINAPI
WriteLoop(
    ULARGE_INTEGER FileSize
    );

VOID
ReadLoop(
    ULARGE_INTEGER FileSize
    );

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{
    HANDLE WritingThread;
    DWORD ThreadId;
    ULARGE_INTEGER FileSize;
    ULARGE_INTEGER InitialFileSize;
    BOOL Success;
    DWORD Status;
    DWORD StartTime, EndTime;
    SYSTEM_INFO SystemInfo;
    HANDLE BufferedHandle;

    if (argc != 3) {
        fprintf(stderr, "Usage: %s SourceFile DestinationFile\n", argv[0]);
        exit(1);
    }

    //
    //confirm we are running on Windows NT 3.5 or greater, if not, display notice and
    //terminate.  Completion ports are only supported on Win32 & Win32s.  Creating a
    //Completion port with no handle specified is only supported on NT 3.51, so we need
    //to know what we're running on.  Note, Win32s does not support console apps, thats
    //why we exit here if we are not on Windows NT.
    //
    //
    //ver.dwOSVersionInfoSize needs to be set before calling GetVersionInfoEx()
    //
    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    //
    //Failure here could mean several things 1. On an NT system,
    //it indicates NT version 3.1 because GetVersionEx() is only
    //implemented on NT 3.5.  2. On Windows 3.1 system, it means
    //either Win32s version 1.1 or 1.0 is installed.
    //
    Success = GetVersionEx((LPOSVERSIONINFO) &ver);

    if ( (!Success) ||                                   //GetVersionEx() failed - see above.
         (ver.dwPlatformId != VER_PLATFORM_WIN32_NT) )   //GetVersionEx() succeeded but we are not on NT.
      {
       MessageBox(NULL,
                  "This sample application can only be run on Windows NT. 3.5 or greater\n"
                  "This application will now terminate.",
                  "UnBufCp2",
                  MB_OK           |
                  MB_ICONSTOP     |
                  MB_SETFOREGROUND );
       exit( 1 );
      }

    //
    // Get the system's page size.
    //
    GetSystemInfo(&SystemInfo);
    PageSize = SystemInfo.dwPageSize;

    //
    // Open the source file and create the destination file.
    // Use FILE_FLAG_NO_BUFFERING to avoid polluting the
    // system cache with two copies of the same data.
    //

    SourceFile = CreateFile(argv[1],
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                            NULL);
    if (SourceFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "failed to open %s, error %d\n", argv[1], GetLastError());
        exit(1);
    }
    FileSize.LowPart = GetFileSize(SourceFile, &FileSize.HighPart);
    if ((FileSize.LowPart == 0xffffffff) && (GetLastError() != NO_ERROR)) {
        fprintf(stderr, "GetFileSize failed, error %d\n", GetLastError());
        exit(1);
    }

    DestFile = CreateFile(argv[2],
                          GENERIC_READ | GENERIC_WRITE,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          CREATE_ALWAYS,
                          FILE_FLAG_NO_BUFFERING | FILE_FLAG_OVERLAPPED,
                          SourceFile);
    if (DestFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "failed to open %s, error %d\n", argv[2], GetLastError());
        exit(1);
    }

    //
    // Extend the destination file so that the filesystem does not
    // turn our asynchronous writes into synchronous ones.
    //
    InitialFileSize.QuadPart = (FileSize.QuadPart + PageSize - 1) & ~((DWORD_PTR)(PageSize - 1));
    Status = SetFilePointer(DestFile,
                            InitialFileSize.LowPart,
                            (PLONG)&InitialFileSize.HighPart,
                            FILE_BEGIN);
    if ((Status == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR)) {
        fprintf(stderr, "SetFilePointer failed, error %d\n", GetLastError());
        exit(1);
    }
    Success = SetEndOfFile(DestFile);
    if (!Success) {
        fprintf(stderr, "SetEndOfFile failed, error %d\n", GetLastError());
        exit(1);
    }

    //
    //In NT 3.51 it is not necessary to specify the FileHandle parameter
    //of CreateIoCompletionPort()--It is legal to specify the FileHandle
    //as INVALID_HANDLE_VALUE.  However, for NT 3.5 an overlapped file
    //handle is needed.
    //
    //We know already that we are running on NT, or else we wouldn't have
    //gotten this far, so lets see what version we are running on.
    //
    if (ver.dwMajorVersion == 3 && ver.dwMinorVersion == 50) 
        {
          //
          //we're running on NT 3.5 - Completion Ports exists
          //
          ReadPort = CreateIoCompletionPort(SourceFile,        //file handle to associate with I/O completion port
                                            NULL,              //optional handle to existing I/O completion port
                                            (DWORD_PTR)SourceFile, //completion key
                                            1);                //# of threads allowed to execute concurrently

          if (ReadPort == NULL) {
            fprintf(stderr, "failed to create ReadPort, error %d\n", GetLastError());
            exit(1);
          }
        }

    else   
      //
      //We are running on NT 3.51 or greater.
      //
      //Create the I/O Completion Port.
      //
        {
          ReadPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, //file handle to associate with I/O completion port
                                            NULL,                 //optional handle to existing I/O completion port
                                            (DWORD_PTR)SourceFile,    //completion key
                                            1);                   //# of threads allowed to execute concurrently




         //
         //If we need to, aka we're running on NT 3.51, let's associate a file handle with the
         //completion port.
         //
         ReadPort = CreateIoCompletionPort(SourceFile,
                                           ReadPort,
                                           (DWORD_PTR)SourceFile,  //should be the previously specified key.
                                           1);

         if (ReadPort == NULL)
          {
            fprintf(stderr,
                    "failed to create ReadPort, error %d\n",
                    GetLastError());

            exit(1);

          }
        }

    WritePort = CreateIoCompletionPort(DestFile,
                                       NULL,
                                       (DWORD_PTR)DestFile,
                                       1);
    if (WritePort == NULL) {
        fprintf(stderr, "failed to create WritePort, error %d\n", GetLastError());
        exit(1);
    }

    //
    // Start the writing thread
    //
    WritingThread = CreateThread(NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE)WriteLoop,
                                 &FileSize,
                                 0,
                                 &ThreadId);
    if (WritingThread == NULL) {
        fprintf(stderr, "failed to create write thread, error %d\n", GetLastError());
        exit(1);
    }
    CloseHandle(WritingThread);

    StartTime = GetTickCount();

    //
    // Start the reads
    //
    ReadLoop(FileSize);

    EndTime = GetTickCount();

    //
    // We need another handle to the destination file that is
    // opened without FILE_FLAG_NO_BUFFERING. This allows us to set
    // the end-of-file marker to a position that is not sector-aligned.
    //
    BufferedHandle = CreateFile(argv[2],
                                GENERIC_WRITE,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                0,
                                NULL);
    if (BufferedHandle == INVALID_HANDLE_VALUE) {
        fprintf(stderr,
                "failed to open buffered handle to %s, error %d\n",
                argv[2],
                GetLastError());
        exit(1);
    }

    //
    // Set the destination's file size to the size of the
    // source file, in case the size of the source file was
    // not a multiple of the page size.
    //
    Status = SetFilePointer(BufferedHandle,
                            FileSize.LowPart,
                            (PLONG)&FileSize.HighPart,
                            FILE_BEGIN);
    if ((Status == INVALID_SET_FILE_POINTER) && (GetLastError() != NO_ERROR)) {
        fprintf(stderr, "final SetFilePointer failed, error %d\n", GetLastError());
        exit(1);
    }
    Success = SetEndOfFile(BufferedHandle);
    if (!Success) {
        fprintf(stderr, "SetEndOfFile failed, error %d\n", GetLastError());
        exit(1);
    }
    CloseHandle(BufferedHandle);
    CloseHandle(SourceFile);
    CloseHandle(DestFile);

    printf("\n\n%d bytes copied in %.3f seconds\n",
           FileSize.LowPart,
           (float)(EndTime-StartTime)/1000.0);
    printf("%.2f MB/sec\n",
           ((LONGLONG)FileSize.QuadPart/(1024.0*1024.0)) / (((float)(EndTime-StartTime)) / 1000.0));

    return(0);

}

VOID
ReadLoop(
    ULARGE_INTEGER FileSize
    )
{
    ULARGE_INTEGER ReadPointer;
    BOOL Success;
    DWORD NumberBytes;
    LPOVERLAPPED CompletedOverlapped;
    DWORD_PTR Key;
    PCOPY_CHUNK Chunk;
    int PendingIO = 0;
    int i;

    //
    // Start reading the file. Kick off MAX_CONCURRENT_IO reads, then just
    // loop waiting for writes to complete.
    //
    ReadPointer.QuadPart = 0;

    for (i=0; i < MAX_CONCURRENT_IO; i++) {
        if (ReadPointer.QuadPart >= FileSize.QuadPart) {
            break;
        }
        //
        // Use VirtualAlloc so we get a page-aligned buffer suitable
        // for unbuffered I/O.
        //
        CopyChunk[i].Buffer = VirtualAlloc(NULL,
                                           BUFFER_SIZE,
                                           MEM_COMMIT,
                                           PAGE_READWRITE);
        if (CopyChunk[i].Buffer == NULL) {
            fprintf(stderr, "VirtualAlloc %d failed, error %d\n",i, GetLastError());
            exit(1);
        }
        CopyChunk[i].Overlapped.Offset = ReadPointer.LowPart;
        CopyChunk[i].Overlapped.OffsetHigh = ReadPointer.HighPart;
        CopyChunk[i].Overlapped.hEvent = NULL;     // not needed

        Success = ReadFile(SourceFile,
                           CopyChunk[i].Buffer,
                           BUFFER_SIZE,
                           &NumberBytes,
                           &CopyChunk[i].Overlapped);

        if (!Success && (GetLastError() != ERROR_IO_PENDING)) {
            fprintf(stderr,
                    "ReadFile at %lx failed, error %d\n",
                    ReadPointer.LowPart,
                    GetLastError());
            exit(1);
        } else {
            ReadPointer.QuadPart += BUFFER_SIZE;
            ++PendingIO;

        }
    }

    //
    // We have started the initial async. reads, enter the main loop.
    // This simply waits until a write completes, then issues the next
    // read.
    //
    while (PendingIO) {
        Success = GetQueuedCompletionStatus(WritePort,
                                            &NumberBytes,
                                            &Key,
                                            &CompletedOverlapped,
                                            INFINITE);
        if (!Success) {
            //
            // Either the function failed to dequeue a completion packet
            // (CompletedOverlapped is not NULL) or it dequeued a completion
            // packet of a failed I/O operation (CompletedOverlapped is NULL).  
            //
            fprintf(stderr,
                    "GetQueuedCompletionStatus on the IoPort failed, error %d\n",
                    GetLastError());
            exit(1);
        }
        //
        // Issue the next read using the buffer that has just completed.
        //
        if (ReadPointer.QuadPart < FileSize.QuadPart) {
            Chunk = (PCOPY_CHUNK)CompletedOverlapped;
            Chunk->Overlapped.Offset = ReadPointer.LowPart;
            Chunk->Overlapped.OffsetHigh = ReadPointer.HighPart;
            ReadPointer.QuadPart += BUFFER_SIZE;
            Success = ReadFile(SourceFile,
                               Chunk->Buffer,
                               BUFFER_SIZE,
                               &NumberBytes,
                               &Chunk->Overlapped);

            if (!Success && (GetLastError() != ERROR_IO_PENDING)) {
                fprintf(stderr,
                        "ReadFile at %lx failed, error %d\n",
                        Chunk->Overlapped.Offset,
                        GetLastError());
                exit(1);
            }
        }
        else {
            //
            // There are no more reads left to issue, just wait
            // for the pending writes to drain.
            //
            --PendingIO;

        }
    }
    //
    // All done. There is no need to call VirtualFree() to free CopyChunk 
    // buffers here. The buffers will be freed when this process exits.
    //
}

VOID
WINAPI
WriteLoop(
    ULARGE_INTEGER FileSize
    )
{
    BOOL Success;
    DWORD_PTR Key;
    LPOVERLAPPED CompletedOverlapped;
    PCOPY_CHUNK Chunk;
    DWORD NumberBytes;
    ULARGE_INTEGER TotalBytesWritten;

    TotalBytesWritten.QuadPart = 0;

    for (;;) {
        Success = GetQueuedCompletionStatus(ReadPort,
                                            &NumberBytes,
                                            &Key,
                                            &CompletedOverlapped,
                                            INFINITE);

        if (!Success) {
            //
            // Either the function failed to dequeue a completion packet
            // (CompletedOverlapped is not NULL) or it dequeued a completion
            // packet of a failed I/O operation (CompletedOverlapped is NULL).  
            //
            fprintf(stderr,
                    "GetQueuedCompletionStatus on the IoPort failed, error %d\n",
                    GetLastError());
            exit(1);
        }

        //
        // Update the total number of bytes written.
        //
        TotalBytesWritten.QuadPart += NumberBytes;

        //
        // Issue the next write using the buffer that has just been read into.
        //
        Chunk = (PCOPY_CHUNK)CompletedOverlapped;

        //
        // Round the number of bytes to write up to a sector boundary
        //
        NumberBytes = (NumberBytes + PageSize - 1) & ~(PageSize-1);

        Success = WriteFile(DestFile,
                            Chunk->Buffer,
                            NumberBytes,
                            &NumberBytes,
                            &Chunk->Overlapped);

        if (!Success && (GetLastError() != ERROR_IO_PENDING)) {
            fprintf(stderr,
                    "WriteFile at %lx failed, error %d\n",
                    Chunk->Overlapped.Offset,
                    GetLastError());
            exit(1);
        }

        //
        //Check to see if we've copied the complete file, if so return
        //
        if (TotalBytesWritten.QuadPart >= FileSize.QuadPart)
            return;
    }
}
