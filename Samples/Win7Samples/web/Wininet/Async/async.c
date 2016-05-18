/*++
 Copyright (c) 2002 - 2006 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT. 

--*/

#include "precomp.h"
#include "async.h"

//Action to perfom GET or POST
DWORD g_action;
CONST LPWSTR DEFAULTHOSTNAME = L"www.microsoft.com";
//Host to connect to
LPWSTR g_hostName;
//Resource to get from the server
LPWSTR g_resource = L"/"; //By default request the root object
//File containing data to post
LPWSTR g_inputFile;
//File to write the data received from the server
LPWSTR g_outputFile;
//Flag to indicate the use of a proxy
BOOL g_bUseProxy = FALSE;
//Name of the proxy to use
LPWSTR g_proxy = NULL;
//Flag to indicate the use of SSL
BOOL g_bSecure=FALSE;
//Callback function
INTERNET_STATUS_CALLBACK g_callback;
//Structures to be used in the Async File IO callbacks
IO_BUF *g_readIO;
IO_BUF *g_writeIO;
//Timeout for the async operations
DWORD g_userTimeout = DEFAULT_TIMEOUT;
//Indicate if we had to create a temp file
BOOL g_bCreatedTempFile = FALSE;
//Pointer to RtlNtStatusToDosError function
PRtlNtStatusToDosError g_pfnRtlNtStatusToDosError = NULL;

//Counters of callings to malloc and free
#ifdef DEBUG
LONG  allocCount = 0;
LONG  freeCount = 0;
#endif

int __cdecl wmain(int argc,
                  __in_ecount(argc) LPWSTR *argv)
{
    DWORD dwError = 0;
    DWORD dwSync= 0;
    BOOL bRequestSuccess;
    DWORD dwFileSize = 0;
    DWORD dwOpenType = INTERNET_OPEN_TYPE_PRECONFIG; //Use pre-configured options as default
    INTERNET_PORT serverPort = INTERNET_DEFAULT_HTTP_PORT;
    DWORD dwRequestFlags = 0;
    LPWSTR verb;
    INTERNET_BUFFERS buffersIn;
    MAIN_CONTEXT mainContext;
    APP_CONTEXT context;
    DWORD dwStatus = STATUS_SUCCESS;
    
    //Parse the command line arguments
    ParseArguments(argc,
                   argv);

    if((dwError = SetFunctionEntryPoint()) != ERROR_SUCCESS)
    {
              LogSysError(dwError, L"SetFunctionEntryPoint");
    }
    
    //Initialize the context for the Session and Connection handles
    InitMainContext(&mainContext);

    if(g_bUseProxy)
    {
        dwOpenType = INTERNET_OPEN_TYPE_PROXY;
    }
    //Create Session handle and specify Async Mode
    mainContext.hSession = InternetOpen(L"WinInet HTTP Async Session", //User Agent
                                        dwOpenType, //Preconfig or Proxy
                                        g_proxy,  //g_proxy name
                                        NULL, //g_proxy bypass, do not bypass any address
                                        INTERNET_FLAG_ASYNC); // 0 for Synchronous
    
    if (!mainContext.hSession)
    {
        LogInetError(GetLastError(),L"InternetOpen");
        dwStatus = STATUS_FAILURE;
        goto Exit;
    }

    //Set the dwStatus callback for the handle to the Callback function
    g_callback = InternetSetStatusCallback(mainContext.hSession, 
                                           (INTERNET_STATUS_CALLBACK)CallBack );

    if (g_callback == INTERNET_INVALID_STATUS_CALLBACK)
    {
        LogInetError(GetLastError(),L"InternetSetStatusCallback");
        dwStatus = STATUS_FAILURE;
        goto Exit;
    }

    //Set the correct server port if using SSL
    //Also set the flag for HttpOpenRequest 
    if(g_bSecure)
    {
        serverPort = INTERNET_DEFAULT_HTTPS_PORT;
        dwRequestFlags = INTERNET_FLAG_SECURE;
    }    
    
    //Create Connection handle and provide context for async operations
    mainContext.hConnect = InternetConnect(mainContext.hSession,
                                           g_hostName, //Name of the server to connect to
                                           serverPort, //HTTP (80) or HTTPS (443)
                                           NULL, //Do not provide a user name for the server
                                           NULL, //Do not provide a password for the server
                                           INTERNET_SERVICE_HTTP,
                                           0, //Do not provide any special flag
                                           (DWORD_PTR)&mainContext); //Provide the context to be
                                                                     //used during the callbacks
    //For HTTP InternetConnect returns synchronously.
    //For FTP, ERROR_IO_PENDING should be verified too                      
    if (!mainContext.hConnect )
    {
        LogInetError(GetLastError(),L"InternetConnect");
        dwStatus = STATUS_FAILURE;
        goto Exit;
    }


    //Initialize the context to be used in the asynchronous calls
    InitRequestContext(&mainContext,
                                     &context);

    //Open the file to dump the response entity body and
    //if required the file with the data to post
    OpenFiles(&context);

    //Verify if we've opened a file to post and get its size
    if (context.hFile)
    {
        dwFileSize = GetFileSize(context.hFile,
                                 NULL); 
    }
    
    //Set the initial state of the context and the verb depending on the operation to perform
    if(g_action == GET)
    {
        context.dwState = GET_REQ;
        verb=L"GET";
    }
    else
    {
        context.dwState = POST_REQ;
        verb=L"POST";
    }

    //We're overriding WinInet's default behavior.
    //Setting this flags, we make sure we get the response from the server and not the cache.
    //Also ask WinInet not to store the response in the cache.
    dwRequestFlags |= INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;
    
    //Create a Request handle
    context.hRequest = HttpOpenRequest(context.mainContext->hConnect,
                                       verb, //GET or POST
                                       g_resource, //root "/" by default
                                       NULL, //USe default HTTP/1.1 as the version
                                       NULL, //Do not provide any referrer
                                       NULL, //Do not provide Accept types
                                       dwRequestFlags, //(0 or INTERNET_FLAG_SECURE) | 
                                                       // INTERNET_FLAG_RELOAD | 
                                                       // INTERNET_FLAG_NO_CACHE_WRITE
                                       (DWORD_PTR)&context);
    
    if (!context.hRequest )
    {
        LogInetError(GetLastError(),L"HttpOpenRequest");
        dwStatus = STATUS_FAILURE;
        goto Exit;
    }
    
    //Send the request using two different options.
    //HttpSendRequest for GET and HttpSendRequestEx for POST.
    //HttpSendRequest can also be used also to post data to a server, 
    //to do so, the data should be provided using the lpOptional
    //parameter and it's size on dwOptionalLength.
    //Here we decided to depict the use of both HttpSendRequest functions.
    if (g_action == GET)
    {
        bRequestSuccess = HttpSendRequest(context.hRequest,
                                          NULL, //do not provide additional Headers
                                          0, //dwHeadersLength 
                                          NULL, //Do not send any data 
                                          0); //dwOptionalLength 
    }
    else
    {
        //Prepare the Buffers to be passed to HttpSendRequestEx
        ZeroMemory(&buffersIn,sizeof(INTERNET_BUFFERS));
        buffersIn.dwStructSize = sizeof(INTERNET_BUFFERS);
        buffersIn.lpvBuffer = NULL;
        buffersIn.dwBufferLength = 0;
        buffersIn.dwBufferTotal = dwFileSize; //content-length of data to post
        
        bRequestSuccess = HttpSendRequestEx(context.hRequest,
                                            &buffersIn,
                                            NULL, //Do not use output buffers
                                            0, //dwFlags reserved
                                            (DWORD_PTR)&context);
    }

    if (!bRequestSuccess && (dwError=GetLastError())!=ERROR_IO_PENDING)
    {
        LogInetError(dwError,L"HttpSendRequest(Ex)");
        dwStatus = STATUS_FAILURE;
        goto Exit;
    }   

    //If you're using a UI thread, this call is not required
    dwSync = WaitForSingleObject(context.hEvent,
                                 g_userTimeout); // Wait until we receive the completion
    
    switch(dwSync)
    {
        case WAIT_OBJECT_0:
            printf("Done!\n");
            break;
        case WAIT_ABANDONED:
            fprintf(stderr,
                    "The callback thread was terminated\n");
            dwStatus = STATUS_FAILURE;
            break;
        case WAIT_TIMEOUT:
            fprintf(stderr,
                    "Timeout while waiting for event\n");
            dwStatus = STATUS_FAILURE;
            break;
    }
    
    Exit:

    CleanUp(&context);
    return dwStatus;
}

VOID CALLBACK 
CallBack(HINTERNET hInternet ,
         __in DWORD_PTR dwContext,
         DWORD dwInternetStatus,
         __in_bcount(dwStatusInformationLength) LPVOID lpvStatusInformation,
         DWORD dwStatusInformationLength)
/*++

Routine Description:
    Callback routine for asynchronous WinInet operations

Arguments:
     hInternet - The handle for which the callback function is called.
     dwContext - Pointer to the application defined context.
     dwInternetStatus - Status code indicating why the callback is called.
     lpvStatusInformation - Pointer to a buffer holding callback specific data.
     dwStatusInformationLength - Specifies size of lpvStatusInformation buffer.

Return Value:
    None.

--*/
{
    DWORD dwError;
    DWORD dwBytes = 0;
    BOOL bQuit = FALSE;
    MAIN_CONTEXT *mainContext;
    APP_CONTEXT *appContext;

    //Perform the correct casting given the type of structure passed in the callback
    DWORD *dwStructType = (DWORD*)dwContext;
    
    if( *dwStructType == STRUCT_TYPE_MAIN_CONTEXT)
    {
        mainContext = (MAIN_CONTEXT*)dwContext;
    }
    else
    {
        appContext = (APP_CONTEXT*)dwContext;
    }
    
    fprintf(stderr,"Callback Received for Handle %p \t",hInternet);
    
    switch(dwInternetStatus)
    {
        case INTERNET_STATUS_COOKIE_SENT:
            fprintf(stderr,"Status: Cookie found and will be sent with request\n");
            break;
        case INTERNET_STATUS_COOKIE_RECEIVED:
            fprintf(stderr,"Status: Cookie Received\n");
            break;
        case INTERNET_STATUS_COOKIE_HISTORY:
            {
                InternetCookieHistory cookieHistory;
                fprintf(stderr,"Status: Cookie History\n");

                //Verify we've a valid pointer with the correct size
                if(lpvStatusInformation && 
                   dwStatusInformationLength == sizeof(InternetCookieHistory))
                {
                    cookieHistory = *((InternetCookieHistory*)lpvStatusInformation);
                }
                else
                {
                    fprintf(stderr,"Cookie History not valid\n");
                    goto ExitSwitch;
                }
                if(cookieHistory.fAccepted)
                {
                    fprintf(stderr,"Cookie Accepted\n");
                }
                if(cookieHistory.fLeashed)
                {
                    fprintf(stderr,"Cookie Leashed\n");
                }        
                if(cookieHistory.fDowngraded)
                {
                    fprintf(stderr,"Cookie Downgraded\n");
                }        
                if(cookieHistory.fRejected)
                {
                    fprintf(stderr,"Cookie Rejected\n");
                }
            }
        ExitSwitch:
            break;   
        case INTERNET_STATUS_CLOSING_CONNECTION:
            fprintf(stderr,"Status: Closing Connection\n");
            break;
        case INTERNET_STATUS_CONNECTED_TO_SERVER:
            fprintf(stderr,"Status: Connected to Server\n");
            break;
        case INTERNET_STATUS_CONNECTING_TO_SERVER:
            fprintf(stderr,"Status: Connecting to Server\n");
            break;
        case INTERNET_STATUS_CONNECTION_CLOSED:
            fprintf(stderr,"Status: Connection Closed\n");
            break;
        case INTERNET_STATUS_HANDLE_CLOSING:
            fprintf(stderr,"Status: Handle Closing\n");
            //Signal the event for closing the handle
            //only for the Request Handle
            if(appContext)
            {
                SetEvent(appContext->hEvent);
            }
            break;
        case INTERNET_STATUS_HANDLE_CREATED:
            //Verify we've a valid pointer
            if(lpvStatusInformation)
            {
                fprintf(stderr,
                        "Handle %x created\n", 
                        ((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwResult);
            }
            break;
        case INTERNET_STATUS_INTERMEDIATE_RESPONSE:
            fprintf(stderr,"Status: Intermediate response\n");
            break;
        case INTERNET_STATUS_RECEIVING_RESPONSE:
            fprintf(stderr,"Status: Receiving Response\n");    
            break;
        case INTERNET_STATUS_RESPONSE_RECEIVED:
            //Verify we've a valid pointer with the correct size
            if(lpvStatusInformation && 
                dwStatusInformationLength == sizeof(DWORD))
            {
                dwBytes = *((LPDWORD)lpvStatusInformation);
                fprintf(stderr,"Status: Response Received (%d Bytes)\n",dwBytes);
            }
            else
            {
                fprintf(stderr,"Response Received: lpvStatusInformation not valid\n");
            }        
            break;
        case INTERNET_STATUS_REDIRECT:
            fprintf(stderr,"Status: Redirect\n");
            break;
        case INTERNET_STATUS_REQUEST_COMPLETE:
            fprintf(stderr,"Status: Request complete\n");
            
            //check for error first            
            dwError = ((LPINTERNET_ASYNC_RESULT)lpvStatusInformation)->dwError ;
                
            if ( dwError != ERROR_SUCCESS)
            {
                LogInetError(dwError,L"Request_Complete");
                exit(1);
            }
                
            switch (appContext->dwState)
            {
                case POST_REQ:     
                    //read bytes to write   
                    if((dwError = DoReadFile(appContext))!=ERROR_SUCCESS 
                        && dwError!= ERROR_IO_PENDING)
                    {
                        LogSysError(dwError,L"DoReadFile");
                        exit(1);
                    } 
                    
                    break;
                    
                case POST_RES: //fall through 
                case GET_REQ:
                    
                    if(!appContext->dwDownloaded )
                    {
                        
                        EnterCriticalSection(&appContext->crSection);
                        {
                            appContext->bReceiveDone=TRUE;
                            
                            if (!appContext->lPendingWrites)
                            {
                                bQuit = TRUE;
                            }
                            
                        }
                        LeaveCriticalSection(&appContext->crSection);
                        
                        if(bQuit)
                        {
                            SetEvent(appContext->hEvent);  
                        }
                        
                        break;
                    }
                    else if(appContext->dwDownloaded !=INVALID_DOWNLOAD_VALUE)
                    {
                      
                        ZeroMemory(g_writeIO,sizeof(IO_BUF));
                            
                        InterlockedIncrement(&appContext->lPendingWrites);
                        
                        g_writeIO->aContext = appContext;
                        
                        g_writeIO->lpo.Offset=appContext->dwWriteOffset;
                        CopyMemory(&g_writeIO->buffer,
                                   appContext->pszOutBuffer,
                                   appContext->dwDownloaded);
                        
                        if(!WriteFile(appContext->hRes,
                                      &g_writeIO->buffer,
                                      appContext->dwDownloaded,
                                      NULL,
                                      &g_writeIO->lpo))
                        {
                            if((dwError=GetLastError())!= ERROR_IO_PENDING)
                            {
                                LogSysError(dwError,L"WriteFile");
                                exit(1);
                            }
                        }                   
                        
                        appContext->dwWriteOffset += appContext->dwDownloaded;
                        
                    }
                    else
                    {
                        //appContext->dwDownloaded ==INVALID_DOWNLOAD_VALUE
                        //We're in the initial state of the response's download
                        fprintf(stderr,"Ready to start reading the Response Entity Body\n");
                    }
                    
                    DoInternetRead(appContext);
                    
                    break;
            } 
            
            break;
        case INTERNET_STATUS_REQUEST_SENT:
            //Verify we've a valid pointer with the correct size
            if(lpvStatusInformation && 
               dwStatusInformationLength == sizeof(DWORD))
            {
                dwBytes = *((LPDWORD)lpvStatusInformation);
                fprintf(stderr,"Status: Request sent (%d Bytes)\n",dwBytes);
            }
            else
            {
                fprintf(stderr,"Request sent: lpvStatusInformation not valid\n");
            }
            break;
        case INTERNET_STATUS_DETECTING_PROXY:
            fprintf(stderr,"Status: Detecting Proxy\n");
            break;            
        case INTERNET_STATUS_RESOLVING_NAME:
            fprintf(stderr,"Status: Resolving Name\n");
            break;
        case INTERNET_STATUS_NAME_RESOLVED:
            fprintf(stderr,"Status: Name Resolved\n");
            break;
        case INTERNET_STATUS_SENDING_REQUEST:
            fprintf(stderr,"Status: Sending request\n");
            break;
        case INTERNET_STATUS_STATE_CHANGE:
            fprintf(stderr,"Status: State Change\n");
            break;
        case INTERNET_STATUS_P3P_HEADER:
            fprintf(stderr,"Status: Received P3P header\n");
            break;
        default:
            fprintf(stderr,"Status: Unknown (%d)\n",dwInternetStatus);
            break;
    }
}

DWORD 
DoReadFile(__in APP_CONTEXT* aContext)
/*++

Routine Description:
     This routine handles asynschronous file reads.

Arguments:
     aContext - Pointer to application context structure

Return Value:
    Error code for the operation.

--*/
{
    DWORD dwError = ERROR_SUCCESS;

    ZeroMemory(g_readIO,sizeof(IO_BUF));
    
    g_readIO->aContext = aContext;
    
    g_readIO->lpo.Offset = aContext->dwReadOffset;

    if ( !ReadFile(aContext->hFile,
                   &g_readIO->buffer,
                   BUFFER_LEN,
                   NULL,
                   &g_readIO->lpo) )
    {
        
        if ( (dwError=GetLastError()) == ERROR_HANDLE_EOF )
        {
            //Clear the error code since we've handled the error conditions
            dwError = DoCompleteReadFile(aContext);
        }
        else if (dwError != ERROR_IO_PENDING )
        {
            LogSysError(dwError,L"ReadFile");
            goto Exit;
        }
    }

Exit:
    return dwError;
}

DWORD DoCompleteReadFile(__in APP_CONTEXT* aContext)
/*++

Routine Description:
     This routine handles asynschronous file reads.

Arguments:
     aContext - Pointer to application context structure

Return Value:
    Error Code for the operation.

--*/
{

    DWORD dwError=ERROR_SUCCESS;

    fprintf(stderr,"Finished posting file\n");
    aContext->dwState = POST_RES;
    if( !HttpEndRequest(aContext->hRequest,NULL,0,0))
    {
        if ( (dwError = GetLastError()) == ERROR_IO_PENDING)
        {
            fprintf(stderr,"Waiting for HttpEndRequest to complete \n");
        }
        else
        {
            LogInetError(dwError,L"HttpEndRequest");
            goto Exit;
        }
    }
    else
    {
        DoInternetRead(aContext);
    }

Exit:
    return dwError;
}

VOID 
DoInternetWrite(__in APP_CONTEXT *aContext)
/*++

Routine Description:
     This routine handles WinInet Writes. If a write completes synchronously
     this routine issues a file read to gather data to post.

Arguments:
     aContext - Pointer to application context structure

Return Value:
    None.

--*/
{
    DWORD dwError = 0;

    if(InternetWriteFile(aContext->hRequest,
                         aContext->pszOutBuffer,
                         aContext->dwRead,
                         &aContext->dwWritten))
    {
        //read bytes to write   
        if((dwError=DoReadFile(aContext)) != ERROR_SUCCESS
            && dwError != ERROR_IO_PENDING)

        {
            LogSysError(dwError,L"DoReadFile");
            exit(1);
        } 
    }
    
    if (  (dwError=GetLastError()) == ERROR_IO_PENDING)
    {
        fprintf(stderr,"Waiting for InternetWriteFile to complete\n");
    }
    else if ( dwError != ERROR_SUCCESS)
    {
        LogInetError(dwError,L"InternetWriteFile");
        exit(1);
    }
}

VOID
DoInternetRead(__in APP_CONTEXT *aContext)
/*++

Routine Description:
     This routine handles WinInet/Internet reads.

Arguments:
     aContext - Pointer to application context structure

Return Value:
    None.

--*/
{
    DWORD dwError=0;

    while (InternetReadFile(aContext->hRequest,
                            aContext->pszOutBuffer,
                            BUFFER_LEN,
                            &aContext->dwDownloaded))
    {
        //completed synchronously ; callback won't be issued
        BOOL bQuit = FALSE;

        if ( !aContext->dwDownloaded )
        {
            EnterCriticalSection(&aContext->crSection);
            {
                aContext->bReceiveDone = TRUE;
                
                if (!aContext->lPendingWrites)
                {
                    bQuit = TRUE;
                }
                
            }
            LeaveCriticalSection(&aContext->crSection);
            
            if (bQuit)
            {
                SetEvent(aContext->hEvent);
            }
            return;
        }

        ZeroMemory(g_writeIO,sizeof(IO_BUF));

        InterlockedIncrement(&aContext->lPendingWrites);

        g_writeIO->aContext = aContext;

        g_writeIO->lpo.Offset = aContext->dwWriteOffset;
        CopyMemory(&g_writeIO->buffer,
                   aContext->pszOutBuffer,
                   aContext->dwDownloaded); 
        
        if (!WriteFile(aContext->hRes,
                       &g_writeIO->buffer,
                       aContext->dwDownloaded,
                       NULL,
                       &g_writeIO->lpo))
        {
            
            if ( (dwError = GetLastError()) != ERROR_IO_PENDING )
            {
                LogSysError(dwError,L"WriteFile");
                exit(1);
            }
        }

        aContext->dwWriteOffset += aContext->dwDownloaded;

    }

    if ( (dwError=GetLastError()) == ERROR_IO_PENDING)
    {
        fprintf(stderr,"Waiting for InternetReadFile to complete\n");
    }
    else
    {
        LogInetError(dwError,L"InternetReadFile");
        exit(1);
    }
}

VOID CALLBACK
WriteFileCallBack(DWORD dwErrorCode,
                  DWORD dwNumberOfBytesTransfered,
                  __in LPOVERLAPPED lpOverlapped)
/*++

Routine Description:
     Callback routine for Asynchronous file write completions. This 
     routine determines if the response is completely received and 
     all writes are completed before signalling the event to terminate
     the program. Frees the Overlapped object.

Arguments:
     dwErrorCode - I/O completion Status
     dwNumberOfBytesTransfered - Number of bytes transfered
     lpOverlapped - Pointer to the overlapped structure used in WriteFile
                    The way IO_BUF structure is designed, this pointer also
                    points to the IO_BUF structure.

Return Value:
    None.

    --*/
{
    BOOL bQuit = FALSE;
    APP_CONTEXT *aContext;
    IO_BUF *ioBuf;
    UNREFERENCED_PARAMETER(dwNumberOfBytesTransfered);

    if (dwErrorCode == ERROR_SUCCESS)
    {
        ioBuf = CONTAINING_RECORD(lpOverlapped,
                                                      IO_BUF,
                                                      lpo);
        aContext = ioBuf->aContext;

        EnterCriticalSection(&aContext->crSection);
        {
            if (!InterlockedDecrement(&aContext->lPendingWrites)
                && aContext->bReceiveDone)
            {
                bQuit = TRUE;
               
            }
        }
        LeaveCriticalSection(&aContext->crSection);
     
        if (bQuit)
        {
            SetEvent(aContext->hEvent);
        }
    }

    return;
}

VOID CALLBACK
ReadFileCallBack(DWORD dwErrorCode,
                 DWORD dwNumberOfBytesTransfered,
                 __in LPOVERLAPPED lpOverlapped)
/*++

Routine Description:
     Callback routine for Asynchronous file read completions. On successful
     file read this routine triggers a WinInet Write operation to transfer data
     to the http server.

Arguments:
     dwErrorCode - I/O completion Status
     dwNumberOfBytesTransfered - Number of bytes read
     lpOverlapped - Pointer to the overlapped structure used in WriteFile
                    The way IO_BUF structure is designed, this pointer also
                    points to the IO_BUF structure.

Return Value:
    None.

--*/
{
    LPSTR buffer;
    APP_CONTEXT *aContext;
    IO_BUF *ioBuf;

    if ( dwErrorCode == ERROR_SUCCESS || 
          g_pfnRtlNtStatusToDosError(dwErrorCode) == ERROR_HANDLE_EOF  )
    {
        ioBuf = CONTAINING_RECORD(lpOverlapped,
                                  IO_BUF,
                                  lpo);

        aContext = ioBuf->aContext;
        if ( dwErrorCode == ERROR_SUCCESS)
        {
            buffer = ioBuf->buffer;

            aContext->dwReadOffset += dwNumberOfBytesTransfered;
            aContext->dwRead = dwNumberOfBytesTransfered;

            CopyMemory(aContext->pszOutBuffer,
                       buffer,
                       aContext->dwRead);

            DoInternetWrite(aContext);
        }
        else //ERROR_HANDLE_EOF
        {
            DoCompleteReadFile(aContext);
        }
    }
    return;
}

VOID 
InitMainContext(__inout MAIN_CONTEXT *aMainContext)
/*++

Routine Description:
    This routine initializes the session and connection handles.

Arguments:
    aMainContext - Pointer to MAIN_CONTEXT structure

Return Value:
    None.

--*/
{
    aMainContext->dwStructType =  STRUCT_TYPE_MAIN_CONTEXT;
    aMainContext->hSession = NULL;
    aMainContext->hConnect = NULL;

    return;
}

VOID 
InitRequestContext(__in MAIN_CONTEXT* aMainContext,
                   __inout APP_CONTEXT *aContext)
/*++

Routine Description:
    This routine initializes application request context variables to appropriate
    values.

Arguments:
    aContext - Pointer to Application context structure
    aMainContext - Pointer to MAIN_CONTEXT structure containing the sesion and 
    connection handles.

Return Value:
    None.

--*/
{
    aContext->dwStructType =  STRUCT_TYPE_APP_CONTEXT;
    aContext->mainContext = aMainContext;
    aContext->hRequest = NULL;
    aContext->dwDownloaded = INVALID_DOWNLOAD_VALUE;
    aContext->dwRead = 0;
    aContext->dwWritten = 0;
    aContext->dwReadOffset = 0;
    aContext->dwWriteOffset = 0;
    aContext->lPendingWrites = 0;
    aContext->bReceiveDone = FALSE;
    aContext->hFile = NULL;
    aContext->hRes = NULL;

    aContext->pszOutBuffer = Malloc(BUFFER_LEN);
        
    //create event
    aContext->hEvent = CreateEvent(NULL, //Sec attrib
                                   FALSE, //Auto reset
                                   FALSE, //Initial state unsignalled
                                   L"MAIN_SYNC"); 
    
    if (!aContext->hEvent)
    {
        LogSysError(GetLastError(),L"CreateEvent");
        exit(1);
    }
    
    //initialize critical section
    InitializeCriticalSection(&aContext->crSection);

    return;
}

VOID 
CleanUp(__in APP_CONTEXT* aContext)
/*++

Routine Description:
    Used to cleanup application context before exiting.

Arguments:
    aContext - Application context structure

Return Value:
    None.

--*/
{
    DWORD dwSync = 0;

    if( aContext->hFile) 
    {
        CloseHandle(aContext->hFile);
    }
    if( aContext->hRes) 
    {
        CloseHandle(aContext->hRes);
    }
    if( aContext->hRequest ) 
    {
        InternetCloseHandle(aContext->hRequest );
        // Wait for the closing of the handle
        dwSync = WaitForSingleObject(aContext->hEvent,INFINITE); 
        if(WAIT_ABANDONED == dwSync)
        {
            fprintf(stderr,"The callback thread has terminated.\n");
        }
    }
    if( aContext->mainContext->hConnect ) 
    {
        //Remove the callback from the Connection handle.
        //Since se set the callback on the session handle previous to create the conenction and
        //request handles, they inherited the callback function.
        //Setting the callback function to null in the Connect handle, will ensure we don't get 
        //a notification when the handle is closed
        g_callback = InternetSetStatusCallback( aContext->mainContext->hConnect, NULL );

        //Call InternetCloseHandle and do not wait for the closing notification 
        //in the callback funciton
        InternetCloseHandle(aContext->mainContext->hConnect );

    }
     if( aContext->mainContext->hSession) 
    { 
        //Remove the callback from the Session handle
        g_callback = InternetSetStatusCallback( aContext->mainContext->hSession, NULL );
        //At this point the Session handle should be valid
        InternetCloseHandle(aContext->mainContext->hSession );
    }
    if( aContext->hEvent) 
    {
        CloseHandle(aContext->hEvent);
    }
    
    DeleteCriticalSection(&aContext->crSection);

    Free(aContext->pszOutBuffer);
    
    //Free the structures containing the overlapped structure
    Free(g_readIO);
    Free(g_writeIO);

    //Free the buffer for the temporary name if needed.
    if(g_bCreatedTempFile)
    {
        Free(g_outputFile);
    }

#ifdef DEBUG
    printf("Cleanup:\nAlloc count is %d\n",allocCount);
    printf("Free count is %d\n",freeCount);
#endif  
}

VOID 
OpenFiles(__inout APP_CONTEXT *aContext)
/*++

Routine Description:
    This routine opens files in async mode and binds a thread from the
    thread-pool to handle the callback for asynchronous operations. Always
    opens a file to write output to. 

Arguments:
    aContext - Pointer to Application context structure

Return Value:
    None.

--*/
{
    if ( g_action == POST)
    {
        //Open input file
        aContext->hFile = CreateFile(g_inputFile,
                                     GENERIC_READ,
                                     FILE_SHARE_READ, 
                                     NULL, // handle cannot be inherited
                                     OPEN_ALWAYS, // if file exists, open it
                                     FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                                     NULL);    //No template file

        if (!aContext->hFile)
        {
            LogSysError(GetLastError(),L"CreateFile");
            exit(1);
        }
        
        if ( ! BindIoCompletionCallback(aContext->hFile,ReadFileCallBack,0))
        {
            LogSysError(GetLastError(),L"BindIoCompletionCallback");
            exit(1);
            
        }
    }

    //Open output file
    aContext->hRes = CreateFile(g_outputFile,
                                GENERIC_WRITE,
                                0, //Open exclusively
                                NULL, //handle cannot be inherited
                                CREATE_ALWAYS, // if file exists, delete it
                                FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
                                NULL);  //No template file
    
    if (!aContext->hRes)
    {
        LogSysError(GetLastError(),L"CreateFile");
        exit(1);       
    }
    
    if ( ! BindIoCompletionCallback(aContext->hRes,WriteFileCallBack,0) )
    {
        LogSysError(GetLastError(),L"BindIoCompletionCallback");
        exit(1);        
    }

    //Allocate Memory for the IO_BUFF structures
   g_writeIO = Malloc(sizeof(IO_BUF));
   g_readIO = Malloc(sizeof(IO_BUF));

}


VOID 
ParseArguments(int argc,
               __in_ecount(argc) LPWSTR *argv)
/*++

Routine Description:
     This routine is used to Parse command line arguments. Flags are
     case sensitive.

Arguments:
     argc - Number of arguments
     argv - Pointer to the argument vector

Return Value:
    None.

--*/
{ 
    int i;
    DWORD dwError = 0;
    UINT uRetVal;

    for (i = 1; i < argc; ++i)
    {        
        if ( wcsncmp(argv[i],L"-",1))
        {
            printf("Invalid switch %ws\n",argv[i]);
            i++;
            continue;
        }
        
        switch(argv[i][1])
        {            
            case L'p':
                
                g_bUseProxy = 1;
                if (i < argc-1)
                {
                    g_proxy = argv[++i];
                }
                break;
                
            case L'h':
                
                if ( i < argc-1)
                {
                    g_hostName = argv[++i];
                }
                
                break;
                
            case L'o':
                
                if ( i < argc-1)
                {
                    g_resource = argv[++i];
                }
                
                break;
                
            case L'r':
            
                if ( i < argc-1)
                {
                    g_inputFile = argv[++i];
                }
                
                break;

            case L'w':
                
                if ( i < argc-1)
                {
                    g_outputFile = argv[++i];
                }
                
                break;
            
            case L'a':
                
                if ( i < argc-1)
                {
                    if ( !_wcsnicmp(argv[i+1],L"get",3))
                    {
                        g_action = GET;
                    }
                    else if (!_wcsnicmp(argv[i+1],L"post",4))
                    {
                        g_action = POST;
                    }
                }
                ++i;
                break;

            case L's':
                g_bSecure = TRUE;
                break;

            case L't':
                if ( i < argc-1)
                {
                    //Verify the user provided a valid number for the default time
                    if (0 != isdigit((UCHAR)argv[i+1][0]))
                    {
                        g_userTimeout = _wtoi(argv[++i]);
                    }
                }
                break;        

            default:
                ShowUsage();
                exit(1);
                break;
        }
    }

    if (!g_hostName)
    {
        printf("Defaulting hostname to: %ws\n",DEFAULTHOSTNAME);
        g_hostName = DEFAULTHOSTNAME;
    }

    if (!g_action)
    {
        printf("Defaulting action to: GET\n");
        g_action = GET;
    }

    if (!g_inputFile && g_action == POST)
    {
        printf("Error: File to post not specified\n");
        dwError++;
    }
    
    if (!g_outputFile)
    {
        g_bCreatedTempFile = TRUE;
        g_outputFile = Malloc(MAX_PATH);
        // Create a temporary file. 
        uRetVal = GetTempFileName(L".", // current directory 
                                  L"TMP",        // temp file name prefix 
                                  0,            // create unique name 
                                  g_outputFile);  // buffer for name 
        if (uRetVal == 0)
        {
            printf ("GetTempFileName failed with error %d.\n", GetLastError());
            dwError++;
        }
        else
        {          
            printf("Defaulting output file to: %ws\n", g_outputFile);
        }
           
    }
    
    if (dwError)
    {
        exit(1);
    }
}


VOID
ShowUsage(VOID)
/*++

Routine Description:
      Shows the usage of the application.

Arguments:
      None.

Return Value:
      None.

--*/
{
    printf("Usage: async [-a {get|post}] [-h <hostname>] [-o <resourcename>] [-s] ");
    printf("[-p <proxyname>] [-w <output filename>] [-r <file to post>] [-t <userTimeout>]\n");
    printf("Flag Semantics: \n");
    printf("-a : Specify action (\"get\" if omitted)\n");
    printf("-h : Specify Hostname (\"www.microsoft.com\" if omitted)\n");
    printf("-o : Specify resource name in the server (\"/\" if omitted)\n");
    printf("-s : Use secure connection - https\n");
    printf("-p : Specify Proxy\n");
    printf("-w : Specify file to write output to (generate temp file if omitted)\n");
    printf("-r : Specify file to post data from\n");
    printf("-t : Specify time to wait for completing the operation in async mode. Default 2 minutes");
}

VOID 
LogInetError(DWORD err,
                          __in LPCWSTR str)
/*++

Routine Description:
     This routine is used to log WinInet errors in human readable form.

Arguments:
     err - Error number obtained from GetLastError()
     str - String pointer holding caller-context information 

Return Value:
    None.

--*/
{
    DWORD dwResult;
    LPTSTR msgBuffer = Malloc(ERR_MSG_LEN*sizeof(TCHAR));

    ZeroMemory(msgBuffer,ERR_MSG_LEN*sizeof(TCHAR));
    
    dwResult = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE,
                             GetModuleHandle(L"wininet.dll"),
                             err,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                             msgBuffer,
                             ERR_MSG_LEN,
                             NULL);
    
    if (dwResult)
    {
        fprintf(stderr,"%ws: %ws\n",str,msgBuffer);
    }
    else
    {
        fprintf(stderr,
                "Error %d while formatting message for %d in %ws\n",
                GetLastError(),
                err,
                str);
    }
    Free(msgBuffer);
}

VOID
LogSysError(DWORD err,
            __in LPCWSTR str)
/*++

Routine Description:
     This routine is used to log System Errors in human readable form.

Arguments:
     err - Error number obtained from GetLastError()
     str - String pointer holding caller-context information 

Return Value:
    None.

--*/
{
    DWORD dwResult;
    LPTSTR msgBuffer = Malloc(ERR_MSG_LEN*sizeof(TCHAR));

    ZeroMemory(msgBuffer,ERR_MSG_LEN*sizeof(TCHAR));
    
    dwResult = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                             NULL,
                             err,
                             MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                             msgBuffer,
                             ERR_MSG_LEN,
                             NULL);
    
    if (dwResult)
    {
        fprintf(stderr,
                "%ws: %ws\n",
                str,
                msgBuffer);
    }
    else
    {
        fprintf(stderr,
                "Error %d while formatting message for %d in %ws\n",
                GetLastError(),
                err,
                str);    
    }
    Free(msgBuffer);
}

VOID *
Malloc(size_t size)
/*++

Routine Description:
      Wrapper around malloc handling errors and alloc counts.

Arguments:
      size - number of bytes to allocate

Return Value:
      VOID pointer to block of memory allocated

--*/
{
    VOID *ptr;

#ifdef DEBUG
    InterlockedIncrement(&allocCount);
#endif

    ptr = malloc(size);
    
    if (!ptr)
    {
        fprintf(stderr,"Out of Memory\n");
        exit(1);
    }

    return ptr;
}



VOID
Free(__in VOID *memblock)
/*++

Routine Description:
      Wrapper around free to keep up free counts.

Arguments:
      memblock - pointer to memblock to be freed

Return Value:
      None.

--*/
{
#ifdef DEBUG
    InterlockedIncrement(&freeCount);
#endif
    
    free(memblock);
}

DWORD 
SetFunctionEntryPoint()
/*++

Routine Description:
      Get the funciton pointer for RtlNtStatusToDosError.

Arguments:
      None

Return Value:
      Error code for the operation.

--*/
{
	DWORD dwError = ERROR_SUCCESS;

	HMODULE hModule = GetModuleHandle( L"ntdll.dll" );

       if(hModule)
       {
        	SetLastError( ERROR_SUCCESS );
                
        	g_pfnRtlNtStatusToDosError = ( PRtlNtStatusToDosError ) GetProcAddress( hModule, "RtlNtStatusToDosError" );
       }
       
	dwError = GetLastError();

	return dwError;
}
