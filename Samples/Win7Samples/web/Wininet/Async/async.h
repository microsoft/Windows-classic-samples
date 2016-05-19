/*++
 Copyright (c) 2002 - 2006 Microsoft Corporation.  All Rights Reserved.

 THIS CODE AND INFORMATION IS PROVIDED "AS-IS" WITHOUT WARRANTY OF
 ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
 PARTICULAR PURPOSE.

 THIS CODE IS NOT SUPPORTED BY MICROSOFT. 

--*/

#define BUFFER_LEN 4096
#define ERR_MSG_LEN 512
#define BUFSIZE 65536

#define GET 1
#define POST 2

#define GET_REQ 1
#define POST_REQ 2
#define POST_RES 3
#define DEFAULT_TIMEOUT 2 * 60 * 1000 //two minutes

#define INVALID_DOWNLOAD_VALUE (DWORD)-1

#define STATUS_SUCCESS 0
#define STATUS_FAILURE 1

#define STRUCT_TYPE_MAIN_CONTEXT 1
#define STRUCT_TYPE_APP_CONTEXT 2

//Pointer to RtlNtStatusToDosError
typedef ULONG ( NTAPI * PRtlNtStatusToDosError) (
			      IN NTSTATUS Status );

//Structure containing the Session and Connect handles
typedef struct _MAIN_CONTEXT
{
    DWORD dwStructType;
    HINTERNET hSession;
    HINTERNET hConnect;
} MAIN_CONTEXT;

//Structure used for storing the context for the asynchronous calls
//it contains the request handle and a pointer to a structure
//Containing the Session and connect handle.
//This allows having only one connection to the server and create multiple
//request handles using that connection.
typedef struct _APP_CONTEXT
{
    DWORD dwStructType;
    MAIN_CONTEXT* mainContext;
    HINTERNET hRequest;
    HANDLE hEvent;
    LPSTR pszOutBuffer;
    DWORD dwDownloaded ;
    DWORD dwRead;
    DWORD dwWritten;
    DWORD dwReadOffset;
    DWORD dwWriteOffset;
    HANDLE hFile;
    HANDLE hRes;
    DWORD dwState;
    LONG lPendingWrites;
    BOOL bReceiveDone;
    CRITICAL_SECTION crSection;
} APP_CONTEXT;

//Structure to be used in the Asynchronous IO operations
typedef struct
{
    OVERLAPPED lpo;
    APP_CONTEXT *aContext;
    char buffer[BUFFER_LEN];
}IO_BUF;

//WinInet Callback function
VOID CALLBACK CallBack( HINTERNET, 
                        __in DWORD_PTR, 
                        DWORD, 
                        __in_bcount(dwStatusInformationLength)LPVOID,
                        DWORD dwStatusInformationLength);

//File IO related functions
DWORD DoReadFile(__in APP_CONTEXT*);
DWORD DoCompleteReadFile(__in APP_CONTEXT* );
VOID CALLBACK WriteFileCallBack(DWORD ,
                                DWORD ,
                                __in LPOVERLAPPED);
VOID CALLBACK ReadFileCallBack(DWORD ,
                               DWORD ,
                               __in LPOVERLAPPED);

//Network IO related functions
VOID DoInternetWrite(__in APP_CONTEXT*);
VOID DoInternetRead(__in APP_CONTEXT*);

//Initialization functions
VOID InitMainContext(__inout MAIN_CONTEXT*);
VOID InitRequestContext(__in MAIN_CONTEXT*,
                        __inout APP_CONTEXT*);

//Cleanup function
VOID CleanUp(__in APP_CONTEXT*);

//Utility functions
VOID OpenFiles(__inout APP_CONTEXT*);
VOID ParseArguments(int argc,
                    __in_ecount(argc) LPWSTR *);
VOID ShowUsage(VOID);
VOID LogInetError(DWORD,
                  __in LPCWSTR);
VOID LogSysError(DWORD,
                 __in LPCWSTR);
VOID *Malloc(size_t);
VOID Free(__in VOID*);
DWORD SetFunctionEntryPoint();
