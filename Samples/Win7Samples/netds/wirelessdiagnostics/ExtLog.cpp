
#include "precomp.h"
#pragma hdrstop


VOID
InitLogging(
    LPCWSTR         pwszString
    )
{
    UNREFERENCED_PARAMETER (pwszString);
}

VOID
LogInfo(
    IN  CHAR*   Format,
    ...
    )
{
    UNREFERENCED_PARAMETER (Format);

    return;
}

VOID
LogEntry(
    IN  PVOID   pPtr
    )
{
    UNREFERENCED_PARAMETER (pPtr);

    return;
}

VOID
LogExit(
    IN  DWORD   dwStatus
    )
{
    UNREFERENCED_PARAMETER (dwStatus);

    return;
}

