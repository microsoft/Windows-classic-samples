/*++

Copyright (c) 2003  Microsoft Corporation

Module Name: IsapiExtension.h

Abstract:

    A class to provide basic services to an
    ISAPI extension

Author:

    ISAPI developer (Microsoft employee), October 2002

--*/

#ifndef _isapiextension_h
#define _isapiextension_h

#include <IsapiTools.h>

typedef BOOL (WINAPI * PFN_FIRST_HIT_INIT)(VOID * pContext);


class ISAPI_EXTENSION
{
public:

    ISAPI_EXTENSION();

    virtual
    ~ISAPI_EXTENSION();

    BOOL
    Initialize(
        LPSTR   szModuleName
        );

    BOOL
    InitOnce(
        PFN_FIRST_HIT_INIT          pfnInit,
        VOID *                      pContext
        );

    CHAR *
    QueryModulePath(
        VOID
        );

private:

    ISAPI_STRING        _strModuleName;
    ISAPI_STRING        _strModulePath;
    PFN_FIRST_HIT_INIT  _pfnInit;
    CRITICAL_SECTION    _csInit;
    BOOL                _fFirstHitInitDone;
    BOOL                _fInitialized;
};

#endif  // _isaextension_h
