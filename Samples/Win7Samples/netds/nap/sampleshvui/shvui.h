// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF

// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO

// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A

// PARTICULAR PURPOSE.

//

// Copyright (c) Microsoft Corporation. All rights reserved.

#include "resource.h"
#include <windows.h>
#include "napcommon.h"
#include "strsafe.h"

#ifndef SHVUI_H
#define SHVUI_H

static const UINT32 DEFAULT_CONFIGURATION_ID = 0;

#define DEBUGLOGRESULT(fname, result) \
                if (result != ERROR_SUCCESS) \
                { \
                    ShavLogMessageToDebugger(L"%s Returned HR 0x%x\n", fname, result); \
                } \
                else \
                {\
                    ShavLogMessageToDebugger(L"%s succeeded\n", fname); \
                }\
                if (result != ERROR_SUCCESS)\
                {\
                    goto cleanup;\
                }

class ShvUI : public INapComponentConfig3
{
    public:
        ShvUI() : m_cRef(0) {}
        ~ShvUI();

        // implement IUnknown
        STDMETHODIMP 
        QueryInterface(const IID& iid, void** ppv);

        ULONG __stdcall
        AddRef();
    
        ULONG __stdcall
        Release();


        //Implement INapComponentConfig
        STDMETHODIMP 
        IsUISupported(BOOL* isSupported);

        STDMETHODIMP 
        InvokeUI(HWND hwndParent);

        STDMETHODIMP
        GetConfig(UINT16 *bCount, BYTE** data);

        STDMETHODIMP
        SetConfig(UINT16 bCount, BYTE* data);

        //Implement INapComponentConfig2
        STDMETHODIMP 
        IsRemoteConfigSupported(BOOL* isSupported, UINT8* remoteConfigType);

        STDMETHODIMP 
        InvokeUIForMachine(HWND hwndParent, CountedString* machineName);

        STDMETHODIMP 
        InvokeUIFromConfigBlob(
                HWND hwndParent,
                UINT16 inbCount,
                BYTE* inData, 
                UINT16* outbCount, 
                BYTE** outdata, 
                BOOL *fConfigChanged
                );

        //INapComponentConfig3 methods:

        STDMETHODIMP
        NewConfig(IN UINT32 configID);

        STDMETHODIMP
        DeleteConfig(IN UINT32 configID);

        STDMETHODIMP
        DeleteAllConfig();

        STDMETHODIMP
        GetConfigFromID(IN UINT32 configID, OUT UINT16* count, OUT BYTE** outdata);

        STDMETHODIMP
        SetConfigToID(IN UINT32 configID, IN UINT16 count, IN BYTE* outdata);

    private:
        long m_cRef;
};

#define MAX_LOGFILE_LINE_LEN 1024

inline DWORD
ShavLogMessageToDebugger(
    __in PWSTR pwszFormat,
    ...)
{
    DWORD LogResult = ERROR_SUCCESS;
    va_list arglist;
    WCHAR wszOutputBuffer[1024]={0};
    size_t length = 0;
    SYSTEMTIME SystemTime = {0} ;
    
    va_start(arglist, pwszFormat);
	
    GetLocalTime( &SystemTime );

    //
    // Put the thread id and timestamp at the begining of the line.
    //    
    
    LogResult = (DWORD) StringCchPrintf
                    ( 
                        &wszOutputBuffer[0], 
                        ARRAYSIZE(wszOutputBuffer), 
                        L"[%d] %02u/%02u %02u:%02u:%02u ", 
                        GetCurrentThreadId(),
                        SystemTime.wMonth,
                        SystemTime.wDay,
                        SystemTime.wHour,
                        SystemTime.wMinute,
                        SystemTime.wSecond                         
                    );
    if ( LogResult == S_OK )
    {
        LogResult = StringCchLength (   wszOutputBuffer,
                                        MAX_LOGFILE_LINE_LEN-1,
                                        &length
                                    );
        if ( LogResult == S_OK )
        {
            LogResult = (DWORD) StringCchVPrintf(
                                            wszOutputBuffer, 
                                            (MAX_LOGFILE_LINE_LEN-1)-length,  // allow only to write upto MAX_LOGFILE_LINE_LEN-1 characters
                                            pwszFormat, 
                                            arglist);        
        }
    }
    
    va_end(arglist); 
    
    OutputDebugString(wszOutputBuffer);

    return LogResult;
}

#define SHV_CONFIG_BLOB L"ConfigBlob"
#define SHV_KEY L"Software\\Microsoft\\NapServer\\SHVs"
#define SDKSHV_KEY SHV_KEY L"\\79856"
#define SDKSHV_DEFAULT_CONFIG_KEY SDKSHV_KEY L"\\DefaultConfig"
#define SDKSHV_MULTI_CONFIG_KEY   SDKSHV_KEY L"\\MultiConfigs"

#endif //SHV_UI
