;/*++ BUILD Version: 0001    // Increment this if a change has global effects
;
;Copyright 1995 - 1998 Microsoft Corporation
;
;Module Name:
;
;    genctrs.h
;       (derived from genctrs.mc by the message compiler  )
;
;Abstract:
;
;   Event message definititions used by routines in PerfGen.DLL
;
;Created:
;
;   Bob Watson  28-Jul-1995
;
;Revision History:
;
;--*/
;//
;#ifndef _PERFGEN_H_
;#define _PERFGEN_H_
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
An extensible counter has opened the Event Log for PerfGen.DLL
.
;//
MessageId=1999
Severity=Informational
Facility=Application
SymbolicName=UTIL_CLOSING_LOG
Language=English
An extensible counter has closed the Event Log for PerfGen.DLL
.
;//
MessageId=2000
Severity=Error
Facility=Application
SymbolicName=GENPERF_UNABLE_OPEN_DRIVER_KEY
Language=English
Unable open "Performance" key of PerfGen driver in registy. Status code is returned in data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=GENPERF_UNABLE_READ_FIRST_COUNTER
Language=English
Unable to read the "First Counter" value under the PerfGen\Performance Key. Status codes retuened in data.
.
;//
MessageId=+1
Severity=Error
Facility=Application
SymbolicName=GENPERF_UNABLE_READ_FIRST_HELP
Language=English
Unable to read the "First Help" value under the PerfGen\Performance Key. Status codes retuened in data.
.
;//
;#endif // _PERFGEN_H_
