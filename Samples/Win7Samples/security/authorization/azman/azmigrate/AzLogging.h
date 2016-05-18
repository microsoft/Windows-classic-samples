
/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzLogging.h

Abstract:

    Declaration of the CAzLogging class


 History:

****************************************************************************/

#pragma once
#include <atlbase.h>
#include <iostream>
#include <fstream>
#include <strsafe.h>
using namespace std;
typedef unsigned char loglevel;
class CAzLogging
{
public:
    CAzLogging(void);

    ~CAzLogging(void);

    static void Initialize(loglevel);

    static void Initialize(loglevel, __in _TCHAR *);

    static void Log(loglevel LogLevel,__in _TCHAR *);

    static void Log(HRESULT hr, __in _TCHAR *);

    static void Log(HRESULT hr,__in _TCHAR *,__in _TCHAR *entity);

    static void Log(HRESULT hr,__in _TCHAR *,__in _TCHAR *entity,unsigned int pPropID);

    static void Entering(__in _TCHAR *);

    static void Exiting(__in _TCHAR *);

    static void Close();

    static bool MIGRATE_SUCCESS;

    static const loglevel LOG_DEBUG;

    static const loglevel LOG_TRACE;

    static const loglevel LOG_ERROR;

    static const loglevel LOG_LOGFILE;

    static _TCHAR *getTimeBuf();

    static _TCHAR *getMsgBuf(HRESULT hr);

protected:


    static wofstream logfile;

    static loglevel currentLogLevel;

    static const _TCHAR *timestamp_formatstring;
};
