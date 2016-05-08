;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
; THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
; ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
; TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
; PARTICULAR PURPOSE.
;
; Copyright (C) 1995-1999.  Microsoft Corporation.  All rights reserved.
;
;Module Name:
;
;    memctrs.h
;       (derived from memctrs.mc by the message compiler  )
;
;Abstract:
;
;   Event message definititions used by routines in MEMCTRS.DLL
;
;--*/
;//
;#ifndef _MEMCTRS_H_
;#define _MEMCTRS_H_
;//
MessageIdTypedef=DWORD
;//
;//     Perfutil messages
;//
MessageId=1900
Severity=Informational
Facility=Application
SymbolicName=UTIL_LOG_OPEN
Language=English
An extensible counter has opened the Event Log for MEMCTRS.DLL
.
;//
MessageId=1999
Severity=Informational
Facility=Application
SymbolicName=UTIL_CLOSING_LOG
Language=English
An extensible counter has closed the Event Log for MEMCTRS.DLL
.
;//
MessageId=2000
Severity=Error
Facility=Application
SymbolicName=APPMEM_OPEN_FILE_MAPPING_ERROR
Language=English
Unable to open mapped file containing Application Memory performance data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=APPMEM_UNABLE_MAP_VIEW_OF_FILE
Language=English
Unable to map to shared memory file containing Application Memory performance data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=APPMEM_UNABLE_OPEN_DRIVER_KEY
Language=English
Unable open "Performance" key of Applicationa Memory Performace Driver in registy. Status code is returned in data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=APPMEM_UNABLE_READ_FIRST_COUNTER
Language=English
Unable to read the "First Counter" value under the AppMem\Performance Key. Status codes retuened in data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=APPMEM_UNABLE_READ_FIRST_HELP
Language=English
Unable to read the "First Help" value under the AppMem\Performance Key. Status codes retuened in data.
.
;//
;#endif // _MEMCTRS_H_
