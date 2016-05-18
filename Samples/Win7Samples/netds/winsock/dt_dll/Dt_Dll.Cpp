/*++
  
  Copyright (c) 1995 Intel Corp
  Copyright (c) 1996 - 2000 Microsoft Corporation  

  File Name:
  
    dt_dll.cpp
  
  Abstract:
  
    Contains main and supporting functions for a Debug/Trace
    DLL for the WinSock2 DLL.  See the design spec
    for more information.
  
--*/

//
// Include Files
//

#include "nowarn.h"  /* turn off benign warnings */
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_   /* Prevent inclusion of winsock.h in windows.h */
#endif
#include <windows.h>
#include "nowarn.h"  /* some warnings may have been turned back on */
#include <winsock2.h>
#include <stdarg.h>
#include <ws2spi.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <strsafe.h>

#include "dt_dll.h"
#include "cstack.h"
#include "dt.h"
#include "handlers.h"

//
// Forward References for Functions
//

BOOL WINAPI 
DllMain(
    HINSTANCE DllInstHandle, 
    DWORD     Reason, 
    LPVOID    Reserved);

// 
// Externally Visible Global Variables
//

HANDLE LogFileHandle=INVALID_HANDLE_VALUE;// handle to the log file
ULONG  OutputStyle=NO_OUTPUT;
char   Buffer[TEXT_LEN];                  // buffer for building output strings
char  *szDtRegKey="System\\CurrentControlSet\\Services\\Winsock2\\Parameters\\DT_DLL_App_Data";

//
// Static Global Variables
//

// name for my window class
static HINSTANCE        DllInstHandle; 		 // handle to the dll instance
static DWORD            TlsIndex=0xFFFFFFFF; // tls index for this module
static CRITICAL_SECTION CrSec;         // critical section for text output
static HANDLE           TextOutEvent;  // set when debug window is ready

static char             LogFileName[MAX_PATH+1]; // name of the log file
static char             ModuleFileName[MAX_PATH+1]; // name of the application

// function pointer tables for handler functions.  
static LPFNDTHANDLER  HdlFuncTable[MAX_DTCODE + 1];

//
// Function Definitions
//


BOOL WINAPI 
DllMain(
    HINSTANCE InstanceHandle, 
    DWORD     Reason, 
    LPVOID    Reserved)
/*++
  
  DllMain()
  
  Function Description:
  
      Please see Windows documentation for DllEntryPoint.
  
  Arguments:
  
      Please see windows documentation.
  
  Return Value:
  
      Please see windows documentation.
  
--*/
{
    
    Cstack_c   *ThreadCstack;   // points to Cstack objects in tls 
    PINITDATA   InitDataPtr;    // to pass to the window creation thread
    HKEY        hkeyAppData;
    char        szSubKeyStr[sizeof(szDtRegKey)+256],
               *ptr=NULL;
    int         ModuleLen=0;


    // OutputDebugString ("DllMain called.\n");
    switch(Reason) {

        // Determine the reason for the call and act accordingly.
        case DLL_PROCESS_ATTACH:

            if ((ModuleLen = GetModuleFileName (NULL, ModuleFileName, sizeof (ModuleFileName)))==0)
                return FALSE;


            // Allocate a TLS index.
            TlsIndex = TlsAlloc();
            if (TlsIndex==0xFFFFFFFF)
                return FALSE;

            DllInstHandle = InstanceHandle;
            try {
                InitializeCriticalSection(&CrSec);
	        }
            catch(...) {
                OutputStyle = NO_OUTPUT;
                return FALSE;
            }
            
            TextOutEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
            if (TextOutEvent==NULL)
                return FALSE;

            // Fill in the handler function table.
            DTHandlerInit(HdlFuncTable, MAX_DTCODE);

            // Build the log file name
            StringCchCopy( LogFileName, MAX_PATH+1, ModuleFileName );
            StringCchCat( LogFileName, MAX_PATH+1, ".log");

            // See if the reg key exists for this EXE
            ptr = &ModuleFileName[ModuleLen-1];
            while ( (ptr != ModuleFileName) &&  (*ptr != '\\') )
                ptr--;
            if (*ptr == '\\')
                ptr++;

            // Build the key

            StringCchPrintf(szSubKeyStr, sizeof(szDtRegKey)+255, "%s\\%s", szDtRegKey, ptr);

            OutputDebugString("Looking for key: ");
            OutputDebugString(szSubKeyStr);
            OutputDebugString("\r\n");

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        szSubKeyStr,
                        0,
                        KEY_QUERY_VALUE,
                        &hkeyAppData) == NO_ERROR)
            {
                OutputDebugString("Found the registry key\r\n");
                OutputStyle = FILE_ONLY;
  
                RegCloseKey(hkeyAppData);
            }
            else
            {
                OutputStyle = NO_OUTPUT;
            }

            if (OutputStyle == FILE_ONLY)
            {
                LogFileHandle = CreateFile(LogFileName, 
                        GENERIC_WRITE, 
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, 
                        CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, 
                        NULL);
                if (LogFileHandle == INVALID_HANDLE_VALUE) {
                    OutputDebugString("Unable to create log file!\r\n");
                    OutputStyle = NO_OUTPUT;
                }
            }

            // Get some information for later output to the debug window
            // or file -- get the time, PID, and TID of the calling
            // process and put into a INITDATA struct.  This memory will
            // be freed by the thread it is passed to.
            InitDataPtr = (PINITDATA) LocalAlloc(0, sizeof(INITDATA));
            GetLocalTime(&(InitDataPtr->LocalTime));
            InitDataPtr->TID = GetCurrentThreadId();
            InitDataPtr->PID = GetCurrentProcessId();

            // Normally the window thread does a DTTextOut of the time
            // and process info that we saved just above.  But in this
            // case,  there is no window thread so spit it out to the
            // file or debugger. 

            StringCchPrintf(Buffer, TEXT_LEN-1, "Log initiated: %d-%d-%d, %d:%d:%d\r\n", 
                    InitDataPtr->LocalTime.wMonth, 
                    InitDataPtr->LocalTime.wDay, 
                    InitDataPtr->LocalTime.wYear, 
                    InitDataPtr->LocalTime.wHour, 
                    InitDataPtr->LocalTime.wMinute, 
                    InitDataPtr->LocalTime.wSecond);
            DTTextOut(LogFileHandle, Buffer, OutputStyle);
            StringCchPrintf(Buffer, TEXT_LEN-1, "Process ID: 0x%X   Thread ID: 0x%X\r\n",
                    InitDataPtr->PID,
                    InitDataPtr->TID);
            DTTextOut(LogFileHandle, Buffer, OutputStyle);

            // Setting this event allows {Pre|Post}ApiNotify to
            // proceed.  This event isn't really needed in this case
            // (because there is only one thread, and we know the code
            // above has been executed before WSAPre|PostApiNotify).
            SetEvent(TextOutEvent);

            // flow through...

        case DLL_THREAD_ATTACH:

            // Store a pointer to a new Cstack_c in the slot for this
            // thread. 
            ThreadCstack = new Cstack_c();
            TlsSetValue(TlsIndex, (LPVOID)ThreadCstack);

            break;

        case DLL_PROCESS_DETACH:

            // Free up some resources.  This is like cleaning up your room
            // before the tornado strikes, but hey, it's good practice.
            TlsFree(TlsIndex);
            DeleteCriticalSection(&CrSec);

            CloseHandle(LogFileHandle);

            break;

        case DLL_THREAD_DETACH:

            // Get the pointer to this thread's Cstack, and delete the
            // object.
            ThreadCstack = (Cstack_c *)TlsGetValue(TlsIndex);
            delete ThreadCstack;

            break;

        default:

            break;
    } // switch (Reason)

    return TRUE;
} // DllMain()





BOOL WINAPIV
WSAPreApiNotify(
    IN  INT    NotificationCode,
    OUT LPVOID ReturnCode,
    IN  LPSTR  LibraryName,
    ...)
/*++
  
  Function Description:
  
      Builds a string for output and passes it, along with information
      about the call, to a handler function. 
  
  Arguments:
  
      NotificationCode -- specifies which API function called us.
  
      ReturnCode -- a generic pointer to the return value of the API
      function.  Can be used to change the return value in the
      case of a short-circuit (see how the return value from
      PreApiNotify works for more information on short-circuiting
      the API function).

      LibraryName -- a string pointing to the name of the library that
      called us.  
  
      ...    -- variable number argument list.  These are pointers
      to the actual parameters of the API functions.
  
  Return Value:
  
      Returns TRUE if we want to short-circuit the API function;
      in other words, returning non-zero here forces the API function
      to return immediately before any other actions take place.  
      
      Returns FALSE if we want to proceed with the API function.
  
--*/
{
    va_list          vl;            // used for variable arg-list parsing
    Cstack_c         *ThreadCstack; // the Cstack_c object for this thread
    int              Index = 0;     // index into string we are creating
    BOOL             ReturnValue;   // value to return
    LPFNDTHANDLER    HdlFunc;       // pointer to handler function
    int              Counter;       // counter popped off the cstack
    int              OriginalError; // any pending error is saved

    if (OutputStyle==NO_OUTPUT)
        return FALSE;
    
    OriginalError = GetLastError();

    EnterCriticalSection(&CrSec);

    // Wait until the debug window is ready to receive text for output.
    WaitForSingleObject(TextOutEvent, INFINITE);
    va_start(vl, LibraryName);
    
    // Get the Cstack_c object for this thread.
    ThreadCstack = (Cstack_c *)TlsGetValue(TlsIndex);
    if (!ThreadCstack){
        ThreadCstack = new Cstack_c();
        TlsSetValue(TlsIndex, (LPVOID)ThreadCstack);
        StringCchPrintf(Buffer, TEXT_LEN-1, "0x%X Foriegn thread\n",
                 GetCurrentThreadId());
        DTTextOut(LogFileHandle, Buffer, OutputStyle);
    } //if
        
    // Start building an output string with some info that's
    // independent of which API function called us.
    Index = StringCchPrintf(Buffer, TEXT_LEN-1, "Function call: %d   ", 
                      ThreadCstack->CGetCounter());
            
    // Push the counter & increment.
    ThreadCstack->CPush();

    // Reset the error to what it was when the function started.
    SetLastError(OriginalError);

    // Call the appropriate handling function, output the buffer.
    if ((NotificationCode < MAX_DTCODE) && HdlFuncTable[NotificationCode]) {
        HdlFunc = HdlFuncTable[NotificationCode];
        ReturnValue = (*HdlFunc)(vl, ReturnCode, 
                                 LibraryName, 
                                 Buffer, 
                                 Index,
                                 TEXT_LEN,
                                 TRUE);

    } else {

        StringCchPrintf(Buffer + Index, TEXT_LEN - Index - 1, "Unknown function called!\r\n");
        DTTextOut(LogFileHandle, Buffer, OutputStyle);
        ReturnValue = FALSE;
    }
    
    // If we are returning TRUE, then the API/SPI function will be
    // short-circuited.  We must pop the thread stack, since no
    // corresponding WSAPostApiNotify will be called.
    if (ReturnValue) {
        ThreadCstack->CPop(Counter);
    }

    LeaveCriticalSection(&CrSec);
    // In case the error has changed since the handler returned, we
    // want to set it back.
    SetLastError(OriginalError);
    return(ReturnValue);

} // WSAPreApiNotify()






BOOL WINAPIV
WSAPostApiNotify(
    IN  INT    NotificationCode,
    OUT LPVOID ReturnCode,
    IN  LPSTR  LibraryName,
    ...)
/*++
  
  PostApiNotify()
  
  Function Description:
  
      Like PreApiNotify, builds a string and passes it, along with
      information about the call, to a handler function. 
  
  Arguments:
  
      NotificationCode  -- specifies which API function called us.
  
      ReturnCode -- a generic pointer to the return value of the API
      function.  
  
      ...    -- variable number argument list.  These are pointers
      to the actual parameters of the API functions.
  
  Return Value:
  
      Returns value is currently meaningless.
  
--*/
{
    va_list          vl;            // used for variable arg-list parsing
    Cstack_c         *ThreadCstack; // the Cstack_c object for this thread
    int              Index = 0;     // index into string we are creating
    int              Counter;       // counter we pop off the cstack
    LPFNDTHANDLER    HdlFunc;       // pointer to handler function
    int              OriginalError; // any pending error is saved

    if (OutputStyle==NO_OUTPUT)
        return FALSE;

    OriginalError = GetLastError();

    EnterCriticalSection(&CrSec);

    // Wait until it's ok to send output.
    WaitForSingleObject(TextOutEvent, INFINITE);
	
	va_start(vl, LibraryName);

	// Get the cstack object from TLS, pop the Counter.
    ThreadCstack = (Cstack_c *) TlsGetValue(TlsIndex);
    
    if (!ThreadCstack){
        ThreadCstack = new Cstack_c();
        TlsSetValue(TlsIndex, (LPVOID)ThreadCstack);
        StringCchPrintf(Buffer, TEXT_LEN-1, "0x%X Foriegn thread\n",
                 GetCurrentThreadId());
        DTTextOut(LogFileHandle, Buffer, OutputStyle);
    } //if
    
    ThreadCstack->CPop(Counter);
    
    Index = StringCchPrintf(Buffer, TEXT_LEN-1, "Function Call: %d   ", Counter);
    
    // Set the error to what it originally was.
    SetLastError(OriginalError);

    // Call the appropriate handling function, output the buffer.
    if ((NotificationCode < MAX_DTCODE) && HdlFuncTable[NotificationCode]) {
        HdlFunc = HdlFuncTable[NotificationCode];
        (*HdlFunc)(vl, ReturnCode, 
                   LibraryName, 
                   Buffer, 
                   Index,
                   TEXT_LEN,
                   FALSE);
    } else {

        StringCchPrintf(Buffer + Index, TEXT_LEN - Index - 1, "Unknown function returned!\r\n");
        DTTextOut(LogFileHandle, Buffer, OutputStyle);
    }
    
    LeaveCriticalSection(&CrSec);
    // In case the error has changed since the handler returned, we
    // want to set it back.
    SetLastError(OriginalError);
    return(FALSE);

} // WSAPostApiNotify()


BOOL
DTTextOut(
    IN HANDLE FileHandle,
    IN char   *String,
    DWORD     Style)
/*++
  
  DTTextOut()
  
  Function Description:
  
      This function outputs a string to a debug window and/or file.
        
  Arguments:
  
      FileHandle -- handle to an open file for debug output.

      String -- the string to output.

      Style -- specifies whether the output should go to the window,
      the file, or both.
  
  Return Value:
  
      Returns TRUE if the output succeeds, FALSE otherwise.
        
--*/
{
#define SCRATCH_BUFFER  (TEXT_LEN + 60)

    DWORD NumWritten;               // WriteFile takes an address to this
    BOOL  Result;                   // result of WriteFile
    char  Output[SCRATCH_BUFFER];   // scratch buffer 

    static DWORD LineCount = 0;     // text output line number
    DWORD  BufIndex = 0;            // index into output string

    if ((Style==NO_OUTPUT) || (FileHandle == INVALID_HANDLE_VALUE))
        return TRUE;

    // Build a new string with the line-number and pid.tid in front.
    BufIndex += StringCchPrintf(Output, SCRATCH_BUFFER-1, "(%d-%X.%X) ", LineCount++,
                                    GetCurrentProcessId (),
                                    GetCurrentThreadId ());
    StringCbCatN(Output, sizeof(Output),String, SCRATCH_BUFFER - BufIndex - 1);
    
    Result = WriteFile(FileHandle, (LPCVOID)Output, (int) strlen(Output), 
                       &NumWritten, NULL);
    if (!Result) {
        
        CloseHandle(FileHandle);
        FileHandle = INVALID_HANDLE_VALUE;
        return FALSE;
    }

    return TRUE;

} // DTTextOut()
