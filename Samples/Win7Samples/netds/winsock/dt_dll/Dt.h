/*++
  
  Copyright (c) 1995 Intel Corp
  Copyright (c) 1996 - 2000 Microsoft Corporation
  
  Module Name:
  
    dt.h
  
  Abstract:
  
    Header file containing definitions, function prototypes, and other
    stuff for internal use of the debug/trace dll.
  
--*/


#define NO_OUTPUT       0
#define FILE_ONLY       1
#define WINDOW_ONLY     2
#define FILE_AND_WINDOW 3
#define DEBUGGER 4
#define EC_CHILD 1
#define TEXT_LEN 2048
#define MAX_FP   128

// structure to hold init data
typedef struct _INITDATA {

    SYSTEMTIME LocalTime; 
    DWORD      TID;
    DWORD      PID;   

} INITDATA, *PINITDATA;

BOOL
DTTextOut(
    IN HANDLE FileHandle,
    IN char   *String,
    IN DWORD  Style);

extern HANDLE LogFileHandle;             
extern DWORD  OutputStyle;
extern char   Buffer[TEXT_LEN];
