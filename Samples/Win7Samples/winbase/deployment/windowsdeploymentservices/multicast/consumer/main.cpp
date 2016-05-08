/*++

Copyright (c) 2007 Microsoft Corporation

Module Name:

    main.cpp

Abstract:

    Sample code demonstrating the use of the WDS multicast client apis.
    
Author:

   Blaine Young (blyoung)

Environment:

    User Mode

--*/

#include <windows.h>
#include <wdstci.h>
#include <stdio.h>
#include <time.h>

//
// Handy macros.
//

#define EXIT_ON_HR_ERROR(x) do{if(FAILED(x)){hr = x; wprintf(L"Failing with error 0x%08x on line %d\n", hr, __LINE__); goto exit;}}while(0)
#define EXIT_ON_MALLOC_ERROR(x) do{if(NULL == x){hr = E_OUTOFMEMORY;  wprintf(L"Failing with error 0x%08x on line %d\n", hr, __LINE__); goto exit;}}while(0)
#define EXIT_ON_WIN_ERROR(x) do{if(ERROR_SUCCESS != x){hr = HRESULT_FROM_WIN32(x);  wprintf(L"Failing with error 0x%08x on line %d\n", hr, __LINE__); goto exit;}}while(0)

VOID
SessionStartCallback(
    __in HANDLE hSessionKey,
    __in PVOID pCallerData,
    __in PULARGE_INTEGER FileSize
    )
/*++

Routine Description:

    The PFN_WdsTransportClientSessionStart callback is called at the start of a 
    multicast session to indicate file size and other server side information 
    about the file to the consumer.

Arguments:

    hSessionKey - The handle belonging to the session that is being started.
    pvCallerData - Pointer to the caller specific data for this session.  This 
                   data was specified in the call to 
                   WdsTransportClientStartSession.
    FileSize - The total size of the file being transferred over multicast.

--*/       
{
    printf("File transfer started\n");
}

VOID
ReceiveContentsCallback(
    __in HANDLE hSessionKey,
    __in PVOID pCallerData,
    __in_bcount(ulSize) PVOID pContents,
    __in ULONG ulSize,
    __in PULARGE_INTEGER pContentOffset
    )
/*++

Routine Description:

    The PFN_WdsTransportClientReceiveContents callback is used by the multicast 
    client to indicate that a block of data is ready to be consumed.

Arguments:

    hSessionKey - The handle belonging to the session that is being started.
    pvCallerData - Pointer to the caller specific data for this session.  This 
                   data was specified in the call to 
                   WdsTransportClientStartSession.
    pDataBlock - Buffer containing the data that the client received.  This 
                 buffer belongs to the client and should not be modified, but 
                 the consumer may extend the lifetime of this buffer by calling 
                 WdsTransportClientReferenceBuffer on it.
    ulSize - The size of the data in pContents.
    pContentOffset - The offset in the data stream where this block of data 
                     starts.

--*/      
{
    HRESULT hr = S_OK;
    BOOL bResult = FALSE;
    DWORD dwError = ERROR_SUCCESS;
    
    HANDLE hFile = (HANDLE)pCallerData;
    OVERLAPPED Overlapped = {0};

    ULONG ulBytesWritten = 0;

    //
    // Fill in the overlapped structure with the offset information for this
    // write.
    //

    Overlapped.Offset = pContentOffset->LowPart;
    Overlapped.OffsetHigh = pContentOffset->HighPart;

    //
    // Write the data to disk.
    //

    bResult = WriteFile(hFile,
                        pContents,
                        ulSize,
                        &ulBytesWritten,
                        &Overlapped);
    if (FALSE == bResult)
    {
        perror("Could not write data to disk");
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    //
    // Inform the multicast client that it can release the cache associated
    // with this write.  This function is associated with the receive throttling
    // provided by the multicast client.  
    //
    // If this function is not called, the multicast client will stall once it
    // delivers n bytes (where n is the outstanding data limit that the
    // implementor specifies in the ulCacheSize member of the 
    // WDS_TRANSPORTCLIENT_REQUEST structure).
    //

    dwError = WdsTransportClientCompleteReceive(hSessionKey, 
                                                ulSize, 
                                                pContentOffset);
    EXIT_ON_WIN_ERROR(dwError);

exit:

    if (FAILED(hr))
    {
        //
        // We failed somehow, so cancel the transfer.
        //

        WdsTransportClientCancelSession(hSessionKey);
    }

    return;
}

VOID
SessionCompleteCallback(
    __in HANDLE hSessionKey,
    __in PVOID pCallerData,
    __in DWORD dwError
    )
/*++

Routine Description:

    The PFN_WdsTransportClientSessionCompete callback is used by the client to 
    indicate that no more callbacks will be sent to the consumer and that the 
    session either completed successfully or encountered a non-recoverable 
    error.

Arguments:

    hSessionKey - The handle belonging to the session that is being started.
    pvCallerData - Pointer to the caller specific data for this session.  This 
                   data was specified in the call to 
                   WdsTransportClientStartSession.
     dwError - The overall status of the file transfer.  If the session 
               succeeded, this value will be set to ERROR_SUCCESS.  If the 
               session did not succeed, the appropriate error code for the 
               session will be set.

--*/      
{
    printf("File transfer complete with result 0x%08x\n", dwError);
}

int 
__cdecl
wmain(
    __in int argc,
    __in_ecount(argc) LPWSTR argv[]
)
{
    HRESULT hr = S_OK;
    DWORD dwError = ERROR_SUCCESS;
    ULONG ulStatus = WDS_TRANSPORTCLIENT_STATUS_IN_PROGRESS;
    ULONG ulError = ERROR_SUCCESS;

    WDS_TRANSPORTCLIENT_REQUEST RequestParams = {0};    
    HANDLE hSession = NULL;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMCClientKey = NULL;
    clock_t StartTime = 0;

    if(argc < 5)
    {
        printf("Usage: tmc <server> <namespace> <remote file> <local file>\n");
        dwError = ERROR_INVALID_PARAMETER;
        EXIT_ON_WIN_ERROR(dwError);
    }

    StartTime = clock();

    //
    // Initialize the multicast client.  This must occur before any calls into
    // the multicast client.
    //

    dwError = WdsTransportClientInitialize();
    EXIT_ON_WIN_ERROR(dwError);

    //
    // Open a handle to the file that we will be writing to.  We'll use the
    // file handle as our caller data.
    //

    hFile = CreateFile(argv[4],
                       GENERIC_WRITE | GENERIC_READ,
                       FILE_SHARE_READ,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto exit;
    }

    //
    // Fill out the request structure with the parameters that will be used
    // for this transfer.
    //

    RequestParams.ulLength = sizeof(RequestParams);
    RequestParams.ulApiVersion = WDS_TRANSPORTCLIENT_CURRENT_API_VERSION;
    RequestParams.ulAuthLevel = WDS_TRANSPORTCLIENT_AUTH;
    RequestParams.pwszServer = argv[1];
    RequestParams.pwszNamespace = argv[2];
    RequestParams.pwszObjectName = argv[3];
    RequestParams.ulProtocol = WDS_TRANSPORTCLIENT_PROTOCOL_MULTICAST;
    RequestParams.pvProtocolData = NULL;
    RequestParams.ulProtocolDataLength = 0;

    //
    // Cache sizes are fairly arbitrary.  You should test with a variety of
    // cache sizes to determine which best meets your performance and
    // application memory footprint needs.
    // 

    RequestParams.ulCacheSize = 0x2000000;
    
    //
    // Initialize the session in the mc client.  This WILL NOT start the 
    // transfer, it will only initialize it in the client.
    //
    
    dwError = WdsTransportClientInitializeSession(&RequestParams,
                                                  hFile,
                                                  &hMCClientKey);
    EXIT_ON_WIN_ERROR(dwError);

    //
    // Register our callbacks with the multicast client.  These callbacks MUST
    // be registered before we start the transfer.
    //

    //
    // Register our callbacks with the mc client.
    //

    dwError = WdsTransportClientRegisterCallback(hMCClientKey, 
                                                 WDS_TRANSPORTCLIENT_SESSION_COMPLETE, 
                                                 (PVOID)SessionCompleteCallback);
    EXIT_ON_WIN_ERROR(dwError);

    dwError = WdsTransportClientRegisterCallback(hMCClientKey, 
                                                 WDS_TRANSPORTCLIENT_SESSION_START, 
                                                 (PVOID)SessionStartCallback);
    EXIT_ON_WIN_ERROR(dwError);

    dwError = WdsTransportClientRegisterCallback(hMCClientKey, 
                                                 WDS_TRANSPORTCLIENT_RECEIVE_CONTENTS, 
                                                 (PVOID)ReceiveContentsCallback);
    EXIT_ON_WIN_ERROR(dwError);

    //
    // Start the actual transfer.
    //

    dwError = WdsTransportClientStartSession(hMCClientKey);
    EXIT_ON_WIN_ERROR(dwError);

    //
    // Wait for the session to complete.
    //

    dwError = WdsTransportClientWaitForCompletion(hMCClientKey, INFINITE);
    EXIT_ON_WIN_ERROR(dwError);   

    dwError = WdsTransportClientQueryStatus(hMCClientKey, &ulStatus, &ulError);
    EXIT_ON_WIN_ERROR(dwError); 

    hr = HRESULT_FROM_WIN32(ulError);
    
    printf("Transfer complete with 0x%08x\n", hr);

    printf("\nDownload Completed - Time:%.2f Seconds\n",((float)(clock() - StartTime) / CLOCKS_PER_SEC));

exit:

    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
    }

    if (NULL != hMCClientKey)
    {
        if (FAILED(hr))
        {
            WdsTransportClientCancelSession(hMCClientKey);
        }
        WdsTransportClientCloseSession(hMCClientKey);        
    }

    WdsTransportClientShutdown();

    return 0;
}
    

