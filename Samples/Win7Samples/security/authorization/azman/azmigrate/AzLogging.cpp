/****************************************************************************

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

 File:  AzLogging.cpp

Abstract:

    Routines performing the logging

 History:

****************************************************************************/
#include "AzLogging.h"

CAzLogging::CAzLogging(void)
{
}

CAzLogging::~CAzLogging(void)
{
}
/*++

Routine description:

    This method returns current system time in a formatted string

Arguments: NONE

Return Value:

    Returns Formatted string

--*/

_TCHAR *CAzLogging::getTimeBuf() {

        static _TCHAR buf[300];         

        static SYSTEMTIME SystemTime;

        GetLocalTime( &SystemTime );

        buf[0] = _TEXT('\0');

        StringCchPrintf( buf, sizeof(buf)/sizeof(&buf[0]),
                                timestamp_formatstring,
                                SystemTime.wMonth,
                                SystemTime.wDay,
                                SystemTime.wHour,
                                SystemTime.wMinute,
                                SystemTime.wSecond );

    return buf;
}

_TCHAR *CAzLogging::getMsgBuf(HRESULT hr) {

        static _TCHAR ErrorMsgBuffer[1024] ;

        ErrorMsgBuffer[0] = _TEXT('\0');

        FormatMessage( 
                FORMAT_MESSAGE_FROM_SYSTEM | 
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                hr,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                ErrorMsgBuffer,
                1024,
                NULL );

        return ErrorMsgBuffer;

}
/*++

Routine description:

    This method initializes the current log level

Arguments: pcurrentLogLevel - current log level

Return Value:

    NONE

--*/

void CAzLogging::Initialize(loglevel pcurrentLogLevel) {

    currentLogLevel=pcurrentLogLevel;

    MIGRATE_SUCCESS=true;
}

/*++

Routine description:

    This method is to be called as the first line of entering any method

Arguments: strMsg - Message to be logged

Return Value:

    NONE

--*/

void CAzLogging::Entering(__in _TCHAR *strMsg) {

 if (currentLogLevel==LOG_DEBUG) {

        wcout << getTimeBuf() << _TEXT(": ") << _TEXT("Entering ") << strMsg << endl;

    }
}

/*++

Routine description:

    This method is to be called as the last line of entering any method

Arguments: strMsg - Message to be logged

Return Value:

    NONE

--*/

void CAzLogging::Exiting(__in _TCHAR *strMsg) {

    if (currentLogLevel==LOG_DEBUG)
        wcout << getTimeBuf() << _TEXT(": ") << _TEXT("Exiting ") << 
        strMsg << endl;
}

/*++

Routine description:

    This method initializes the current log level and log file

Arguments: pcurrentLogLevel - current log level
           plogfile - Log file name

Return Value:

    NONE

--*/

void CAzLogging::Initialize(loglevel pcurrentLogLevel,
                            __in _TCHAR *plogfile) {

    MIGRATE_SUCCESS=true;

    if (pcurrentLogLevel!=LOG_LOGFILE) {

        Initialize(pcurrentLogLevel);

        return;
    }

    currentLogLevel=pcurrentLogLevel;

    // Converting back to Ansi as Open does not accept wide characters
    logfile.open(CW2A(plogfile));

}

/*++

Routine description:

    This method logs a message at a particular log level

Arguments: strMsg - Message to be logged
           LogLevel - Log level of the message

Return Value:

    NONE

--*/

void CAzLogging::Log(loglevel LogLevel,
                     __in _TCHAR *strMsg) {

    if (currentLogLevel <= LogLevel)
        wcout << getTimeBuf() << _TEXT(": ") << strMsg << endl;

    if (currentLogLevel==LOG_LOGFILE)
        logfile << getTimeBuf() << _TEXT(": ") << strMsg << endl;

}

/*++

Routine description:

    This method logs a message at a particular log level

Arguments: strMsg - Message to be logged
           hr - HRESULT of the operation which is being logged
           strEntityName - Entity name for this log message
           pPropID -  Property ID of the entity which this msg deals with

Return Value:

    NONE

--*/

void CAzLogging::Log(HRESULT hr,
                     __in _TCHAR *strMsg,
                     __in  _TCHAR *strEntityName,
                     unsigned int pPropID) {

    if (SUCCEEDED(hr)) {

        if (currentLogLevel == LOG_TRACE ) 
                wcout << getTimeBuf() << _TEXT(": ") << strMsg << pPropID << 
                    _TEXT(" for entity:")<< strEntityName << _TEXT(" SUCCESS.") << 
                    endl;

        if (currentLogLevel == LOG_LOGFILE ) 
                logfile << getTimeBuf() << _TEXT(": ") <<  strMsg << 
                    pPropID << _TEXT(" for entity:") << 
                    strEntityName << _TEXT(" SUCCESS.") << endl;

    } else {

        if (HRESULT_CODE(hr)==ERROR_NOT_SUPPORTED) {

                wcout <<getTimeBuf() << _TEXT(": ") << strMsg << pPropID << 
                    _TEXT(" for entity:")<< strEntityName << 
                    _TEXT(" WARNING.NOT SUPPORTED.") << endl;

                if (currentLogLevel==LOG_LOGFILE) {
                    logfile << getTimeBuf() << _TEXT(": ")<<  strMsg << 
                        pPropID <<_TEXT(" for entity:") << strEntityName << 
                        _TEXT(" WARNING.NOT SUPPORTED.") << endl;
                }
                
            goto lDone;
        }

        MIGRATE_SUCCESS = false;

        wcout << getTimeBuf() << _TEXT(": ") << strMsg << pPropID << 
            _TEXT(" for entity:") << strEntityName << 
            _TEXT(" FAILED.ERROR MSG:") << getMsgBuf(hr) << endl;

        if (currentLogLevel==LOG_LOGFILE)
            logfile << getTimeBuf() << _TEXT(": ") << strMsg << 
            pPropID << _TEXT(" for entity:") << strEntityName << 
            _TEXT(" FAILED.ERROR MSG: ") << getMsgBuf(hr) << endl;
lDone:
        return;
    }
}

/*++

Routine description:

    This method logs a message at a particular log level

Arguments: strMsg - Message to be logged
           hr - HRESULT of the operation which is being logged
           strEntityName - Entity name for this log message

Return Value:

    NONE

--*/

void CAzLogging::Log(HRESULT hr,
                     __in _TCHAR *strMsg,
                     __in _TCHAR *strEntityName) {
    if (SUCCEEDED(hr)) {

            if (currentLogLevel == LOG_TRACE )
                wcout << getTimeBuf() << _TEXT(": ")<< strMsg << 
                _TEXT(" for entity:")<< strEntityName << 
                _TEXT(" SUCCESS.") << endl;

            if (currentLogLevel==LOG_LOGFILE)
                logfile << getTimeBuf() << _TEXT(": ") <<  strMsg << 
                _TEXT(" for entity:")<< strEntityName << _TEXT(" SUCCESS.") << endl;

    } else {

        if (HRESULT_CODE(hr)==ERROR_NOT_SUPPORTED) {

                wcout << getTimeBuf() << _TEXT(": ") << strMsg << 
                    _TEXT(" for entity:") << strEntityName << 
                    _TEXT(" WARNING.NOT SUPPORTED.") << endl;

                if (currentLogLevel==LOG_LOGFILE)
                    logfile << getTimeBuf() << _TEXT(": ") << strMsg << 
                        _TEXT(" for entity:")<< strEntityName << 
                        _TEXT(" WARNING.NOT SUPPORTED. ") << endl;
                            
            goto lDone;
        }

        MIGRATE_SUCCESS = false;

        wcout << getTimeBuf() << _TEXT(": ") << strMsg << _TEXT(" for entity:")<< 
            strEntityName << _TEXT(" FAILED.ERROR MSG:") << getMsgBuf(hr) << endl;

        if (currentLogLevel==LOG_LOGFILE) {
            logfile << getTimeBuf() << _TEXT(": ") << strMsg << 
                _TEXT(" for entity:")<< strEntityName << _TEXT(" FAILED.ERROR MSG: ") << 
                getMsgBuf(hr) << endl;
        }
    }

lDone:
    return;
}

/*++

Routine description:

    This method logs a message at a particular log level

Arguments: strMsg - Message to be logged
           hr - HRESULT of the operation which is being logged

Return Value:

    NONE

--*/

void CAzLogging::Log(HRESULT hr,
                     __in  _TCHAR *strMsg) {

    if (SUCCEEDED(hr)) {

            if (currentLogLevel == LOG_TRACE )
                wcout <<getTimeBuf() << _TEXT(": ") << strMsg << 
                _TEXT(" SUCCESS.") << endl;

            if (currentLogLevel==LOG_LOGFILE) {
                logfile << getTimeBuf() << _TEXT(": ") <<  strMsg << 
                _TEXT(" SUCCESS.") << endl;
            }
    } else {

        if (HRESULT_CODE(hr)==ERROR_NOT_SUPPORTED) {

                wcout << getTimeBuf() << _TEXT(": ") << strMsg << 
                    _TEXT(" WARNING.NOT SUPPORTED.") << endl;

            if (currentLogLevel==LOG_LOGFILE) 
                logfile << getTimeBuf() << _TEXT(": ") << strMsg << 
                    _TEXT(" WARNING.NOT SUPPORTED. ") << endl;
                            
            goto lDone;
        }

        MIGRATE_SUCCESS = false;

        wcout << getTimeBuf() << _TEXT(": ") << strMsg << 
            _TEXT(" FAILED.ERROR MSG: ") << getMsgBuf(hr) << endl;

        if (currentLogLevel==LOG_LOGFILE)
            logfile << getTimeBuf() << _TEXT(": ") << strMsg << 
            _TEXT(" FAILED.ERROR MSG: ") << getMsgBuf(hr) << endl;
    }
lDone:
    return;
}

/*++

Routine description:

    This method closes the log file 

Arguments: NONE

Return Value:

    NONE

--*/

void CAzLogging::Close() {
    if (currentLogLevel==LOG_LOGFILE) {

        logfile.close();

    }
}


wofstream CAzLogging::logfile;

const _TCHAR *CAzLogging::timestamp_formatstring=_TEXT("%02u/%02u %02u:%02u:%02u ");

bool CAzLogging::MIGRATE_SUCCESS;

const loglevel CAzLogging::LOG_DEBUG=0x7;

const loglevel CAzLogging::LOG_TRACE=0x3;

const loglevel CAzLogging::LOG_ERROR=0x1;

const loglevel CAzLogging::LOG_LOGFILE=0x0;

loglevel CAzLogging::currentLogLevel;