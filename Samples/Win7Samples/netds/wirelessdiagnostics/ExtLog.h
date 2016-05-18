
#ifndef __EXT_LOG_H__
#define __EXT_LOG_H__


VOID
InitLogging(
    LPCWSTR         pwszString
    );

VOID
LogInfo(
    IN  CHAR*   Format,
    ...
    );

VOID
LogEntry(
    IN  PVOID   pPtr
    );

VOID
LogExit(
    IN  DWORD   dwStatus
    );


#endif // __EXT_LOG_H__


