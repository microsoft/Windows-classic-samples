/*++

Copyright (c) 1995 - 2000  Microsoft Corporation

Module Name:

    perfutil.c

Abstract:

    This file implements the utility routines used to construct the
    common parts of a PERF_INSTANCE_DEFINITION (see winperf.h) and
    perform event logging functions.
                 
Created:    

    Bob Watson  28-Jul-1995

Revision History:

--*/
//
//  include files
//        
#include <windows.h>
#include <string.h>
#include <winperf.h>
#include "genctrs.h"     // error message definition
#include "perfmsg.h"
#include "perfutil.h"

//
// Global data definitions.
//

ULONG ulInfoBufferSize = 0;

HANDLE hEventLog = NULL;      // event log handle for reporting events
                              // initialized in Open... routines
DWORD  dwLogUsers = 0;        // count of functions using event log

DWORD MESSAGE_LEVEL = 0;

WCHAR GLOBAL_STRING[] = L"Global";
WCHAR FOREIGN_STRING[] = L"Foreign";
WCHAR COSTLY_STRING[] = L"Costly";

WCHAR NULL_STRING[] = L"\0";    // pointer to null string 

// test for delimiter, end of line and non-digit characters
// used by IsNumberInUnicodeList routine
//
#define DIGIT       1
#define DELIMITER   2
#define INVALID     3

#define EvalThisChar(c,d) ( \
     (c == d) ? DELIMITER : \
     (c == 0) ? DELIMITER : \
     (c < (WCHAR)'0') ? INVALID : \
     (c > (WCHAR)'9') ? INVALID : \
     DIGIT)

HANDLE
MonOpenEventLog(
)
/*++

Routine Description:

    Reads the level of event logging from the registry and opens the
        channel to the event logger for subsequent event log entries.

Arguments:

      None

Return Value:

    Handle to the event log for reporting events.
    NULL if open not successful.

--*/
{
    HKEY hAppKey;
    TCHAR LogLevelKeyName[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
    TCHAR LogLevelValueName[] = "EventLogLevel";

    LONG lStatus;

    DWORD dwLogLevel;
    DWORD dwValueType;
    DWORD dwValueSize;
   
    // if global value of the logging level not initialized or is disabled, 
    //  check the registry to see if it should be updated.

    if (!MESSAGE_LEVEL) {
       lStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                              LogLevelKeyName,
                              0,                         
                              KEY_READ,
                              & hAppKey);
       if (lStatus == ERROR_SUCCESS) {
            dwValueSize = sizeof(dwLogLevel);
            lStatus     = RegQueryValueEx(hAppKey,
                                          LogLevelValueName,
                                          (LPDWORD) NULL,           
                                          & dwValueType,
                                          (LPBYTE) & dwLogLevel,
                                          & dwValueSize);
            if (lStatus == ERROR_SUCCESS) {
               MESSAGE_LEVEL = dwLogLevel;
            } else {
               MESSAGE_LEVEL = MESSAGE_LEVEL_DEFAULT;
            }
            RegCloseKey(hAppKey);
       } else {
         MESSAGE_LEVEL = MESSAGE_LEVEL_DEFAULT;
       }
    }
       
    if (hEventLog == NULL){
         hEventLog = RegisterEventSource(
                         (LPTSTR) NULL,            // Use Local Machine
                         APP_NAME);                // event log app name to find in registry
         if (hEventLog != NULL) {
            REPORT_INFORMATION (UTIL_LOG_OPEN, LOG_DEBUG);
         }
    }
    
    if (hEventLog != NULL) {
         dwLogUsers++;           // increment count of perfctr log users
    }
    return (hEventLog);
}

VOID
MonCloseEventLog(
)
/*++

Routine Description:

      Closes the handle to the event logger if this is the last caller
      
Arguments:

      None

Return Value:

      None

--*/
{
    if (hEventLog != NULL) {
        dwLogUsers --;            // decrement usage
        if (dwLogUsers <= 0) {    // and if we're the last, then close up log
            REPORT_INFORMATION(UTIL_CLOSING_LOG, LOG_DEBUG);
            DeregisterEventSource(hEventLog);
        }
    }
}

DWORD
GetQueryType(
    IN LPWSTR lpValue
)
/*++

GetQueryType

    returns the type of query described in the lpValue string so that
    the appropriate processing method may be used

Arguments

    IN lpValue
        string passed to PerfRegQuery Value for processing

Return Value

    QUERY_GLOBAL
        if lpValue == 0 (null pointer)
           lpValue == pointer to Null string
           lpValue == pointer to "Global" string

    QUERY_FOREIGN
        if lpValue == pointer to "Foriegn" string

    QUERY_COSTLY
        if lpValue == pointer to "Costly" string

    otherwise:

    QUERY_ITEMS

--*/
{
    WCHAR   * pwcArgChar, * pwcTypeChar;
    BOOL    bFound;

    if (lpValue == 0) {
        return QUERY_GLOBAL;
    } else if (*lpValue == 0) {
        return QUERY_GLOBAL;
    }

    // check for "Global" request

    pwcArgChar  = lpValue;
    pwcTypeChar = GLOBAL_STRING;
    bFound = TRUE;  // assume found until contradicted

    // check to the length of the shortest string
    
    while ((* pwcArgChar != 0) && (* pwcTypeChar != 0)) {
        if (* pwcArgChar ++ != * pwcTypeChar ++) {
            bFound = FALSE; // no match
            break;          // bail out now
        }
    }

    if (bFound) return QUERY_GLOBAL;

    // check for "Foreign" request
    
    pwcArgChar  = lpValue;
    pwcTypeChar = FOREIGN_STRING;
    bFound      = TRUE;  // assume found until contradicted

    // check to the length of the shortest string
    
    while ((* pwcArgChar != 0) && (* pwcTypeChar != 0)) {
        if (* pwcArgChar ++ != * pwcTypeChar ++) {
            bFound = FALSE; // no match
            break;          // bail out now
        }
    }

    if (bFound) return QUERY_FOREIGN;

    // check for "Costly" request
    
    pwcArgChar  = lpValue;
    pwcTypeChar = COSTLY_STRING;
    bFound      = TRUE;  // assume found until contradicted

    // check to the length of the shortest string
    
    while ((* pwcArgChar != 0) && (* pwcTypeChar != 0)) {
        if (* pwcArgChar ++ != * pwcTypeChar ++) {
            bFound = FALSE; // no match
            break;          // bail out now
        }
    }

    if (bFound) return QUERY_COSTLY;

    // if not Global and not Foreign and not Costly, 
    // then it must be an item list
    
    return QUERY_ITEMS;

}

BOOL
IsNumberInUnicodeList(
    IN DWORD   dwNumber,
    IN LPWSTR  lpwszUnicodeList
)
/*++

IsNumberInUnicodeList

Arguments:
        
    IN dwNumber
        DWORD number to find in list

    IN lpwszUnicodeList
        Null terminated, Space delimited list of decimal numbers

Return Value:

    TRUE:
            dwNumber was found in the list of unicode number strings

    FALSE:
            dwNumber was not found in the list.

--*/
{
    DWORD   dwThisNumber;
    WCHAR   *pwcThisChar;
    BOOL    bValidNumber;
    BOOL    bNewItem;
    WCHAR   wcDelimiter;    // could be an argument to be more flexible

    if (lpwszUnicodeList == 0) return FALSE;    // null pointer, # not founde

    pwcThisChar  = lpwszUnicodeList;
    dwThisNumber = 0;
    wcDelimiter  = (WCHAR)' ';
    bValidNumber = FALSE;
    bNewItem     = TRUE;
    
    while (TRUE) {
        switch (EvalThisChar (*pwcThisChar, wcDelimiter)) {
            case DIGIT:
                // if this is the first digit after a delimiter, then 
                // set flags to start computing the new number
                if (bNewItem) {
                    bNewItem     = FALSE;
                    bValidNumber = TRUE;
                }
                if (bValidNumber) {
                    dwThisNumber *= 10;
                    dwThisNumber += (* pwcThisChar - (WCHAR) '0');
                }
                break;
            
            case DELIMITER:
                // a delimter is either the delimiter character or the 
                // end of the string ('\0') if when the delimiter has been
                // reached a valid number was found, then compare it to the
                // number from the argument list. if this is the end of the
                // string and no match was found, then return.
                //
                if (bValidNumber) {
                    if (dwThisNumber == dwNumber) return TRUE;
                    bValidNumber = FALSE;
                }
                if (* pwcThisChar == 0) {
                    return FALSE;
                } else {
                    bNewItem     = TRUE;
                    dwThisNumber = 0;
                }
                break;

            case INVALID:
                // if an invalid character was encountered, ignore all
                // characters up to the next delimiter and then start fresh.
                // the invalid number is not compared.
                bValidNumber = FALSE;
                break;

            default:
                break;

        }
        pwcThisChar++;
    }
    return FALSE;
}   // IsNumberInUnicodeList

BOOL
MonBuildInstanceDefinition(
    PERF_INSTANCE_DEFINITION * pBuffer,
    PVOID                    * pBufferNext,
    DWORD                      ParentObjectTitleIndex,
    DWORD                      ParentObjectInstance,
    DWORD                      UniqueID,
    LPWSTR                     Name
)
/*++

    MonBuildInstanceDefinition  -   Build an instance of an object

        Inputs:

            pBuffer         -   pointer to buffer where instance is to
                                be constructed

            pBufferNext     -   pointer to a pointer which will contain
                                next available location, DWORD aligned

            ParentObjectTitleIndex
                            -   Title Index of parent object type; 0 if
                                no parent object

            ParentObjectInstance
                            -   Index into instances of parent object
                                type, starting at 0, for this instances
                                parent object instance

            UniqueID        -   a unique identifier which should be used
                                instead of the Name for identifying
                                this instance

            Name            -   Name of this instance
--*/
{
    DWORD NameLength;
    LPWSTR pName;
    //
    //  Include trailing null in name size
    //

    NameLength                      = (lstrlenW(Name) + 1) * sizeof(WCHAR);
    pBuffer->ByteLength             = sizeof(PERF_INSTANCE_DEFINITION) + DWORD_MULTIPLE(NameLength);
    pBuffer->ParentObjectTitleIndex = ParentObjectTitleIndex;
    pBuffer->ParentObjectInstance   = ParentObjectInstance;
    pBuffer->UniqueID               = UniqueID;
    pBuffer->NameOffset             = sizeof(PERF_INSTANCE_DEFINITION);
    pBuffer->NameLength             = NameLength;

    // copy name to name buffer
    pName = (LPWSTR) & pBuffer[1];
    RtlMoveMemory(pName, Name, NameLength);

    // update "next byte" pointer
    * pBufferNext = (PVOID) ((PCHAR) pBuffer + pBuffer->ByteLength);

    return 0;
}
