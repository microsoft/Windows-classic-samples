// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

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
        ShvUI();
        ~ShvUI();

        // IUnknown

        STDMETHODIMP QueryInterface(
            /* [in] */ __RPC__in const IID& iid, 
            /* [out] */ __RPC__out void** ppv);

        STDMETHODIMP_(ULONG) AddRef();
    
        STDMETHODIMP_(ULONG) Release();

        // INapComponentConfig

        STDMETHODIMP IsUISupported( 
            /* [out] */ __RPC__out BOOL *isSupported);
        
        STDMETHODIMP InvokeUI( 
            /* [unique][in] */ __RPC__in_opt HWND hwndParent);
        
        STDMETHODIMP GetConfig( 
            /* [out] */ _Out_ UINT16 *bCount,
            /* [size_is][size_is][out] */ _Outptr_result_buffer_all_(*bCount) BYTE **data);
        
        STDMETHODIMP SetConfig( 
            /* [in] */ UINT16 bCount,
            /* [size_is][in] */ __RPC__in_ecount_full(bCount) BYTE *data);

        // INapComponentConfig2

        STDMETHODIMP IsRemoteConfigSupported(
            /* [out] */ __RPC__out BOOL* isSupported, 
            /* [out] */ __RPC__out UINT8* remoteConfigType);

        STDMETHODIMP InvokeUIForMachine(
            /* [in] */ __RPC__in_opt HWND hwndParent, 
            /* [in] */ __RPC__in_opt CountedString* machineName);

        STDMETHODIMP InvokeUIFromConfigBlob(
            /* [unique][in] */ __RPC__in_opt HWND hwndParent,
            /* [in] */ __RPC__in UINT16 inbCount,
            /* [size_is][in] */ __RPC__in_ecount_full(inbCount) BYTE *inData,
            /* [out] */ __RPC__out UINT16 *outbCount,
            /* [size_is][size_is][out] */ _Outptr_result_buffer_all_(*outbCount) BYTE **outdata,
            /* [out] */ __RPC__out BOOL *fConfigChanged);

        //INapComponentConfig3

        STDMETHODIMP NewConfig(
            /* [in] */ UINT32 configID);

        STDMETHODIMP DeleteConfig(
            /* [in] */ UINT32 configID);

        STDMETHODIMP DeleteAllConfig();

        _Success_(return == 0)
        STDMETHODIMP GetConfigFromID(
            /* [in] */ _In_ UINT32 configID,
            /* [out] */ _Out_ UINT16 *count,
            /* [size_is][size_is][out] */ _Outptr_result_buffer_all_(*count) BYTE **outdata);

        STDMETHODIMP SetConfigToID(
            /* [in] */ UINT32 configID,
            /* [in] */ UINT16 count,
            /* [size_is][in] */ __RPC__in_ecount_full(count) BYTE *outdata);

    private:
        long m_cRef;
};

#define MAX_LOGFILE_LINE_LEN 1024

inline DWORD
ShavLogMessageToDebugger(
    _In_z_ PWSTR pFormat,
    ...)
{
    DWORD LogResult = ERROR_SUCCESS;
    va_list arglist;
    WCHAR outputBuffer[1024]={0};
    size_t length = 0;
    SYSTEMTIME SystemTime = {0} ;
    
    va_start(arglist, pFormat);
	
    GetLocalTime( &SystemTime );

    //
    // Put the thread id and timestamp at the beginning of the line.
    //    
    
    LogResult = (DWORD) StringCchPrintf(
                                &outputBuffer[0],
                                ARRAYSIZE(outputBuffer),
                                L"[%lu] %02u/%02u %02u:%02u:%02u ",
                                GetCurrentThreadId(),
                                SystemTime.wMonth,
                                SystemTime.wDay,
                                SystemTime.wHour,
                                SystemTime.wMinute,
                                SystemTime.wSecond);
    if ( LogResult == S_OK )
    {
        LogResult = StringCchLength (outputBuffer,
                                     MAX_LOGFILE_LINE_LEN-1,
                                     &length);
        if ( LogResult == S_OK )
        {
            LogResult = (DWORD) StringCchVPrintf(
                                            outputBuffer, 
                                            (MAX_LOGFILE_LINE_LEN-1)-length,  // allow only to write up to MAX_LOGFILE_LINE_LEN-1 characters
                                            pFormat, 
                                            arglist);        
        }
    }
    
    va_end(arglist); 
    
    OutputDebugString(outputBuffer);

    return LogResult;
}

#define SHV_CONFIG_BLOB L"ConfigBlob"
#define SHV_KEY L"Software\\Microsoft\\NapServer\\SHVs"
#define SDKSHV_KEY SHV_KEY L"\\79856"
#define SDKSHV_DEFAULT_CONFIG_KEY SDKSHV_KEY L"\\DefaultConfig"
#define SDKSHV_MULTI_CONFIG_KEY   SDKSHV_KEY L"\\MultiConfigs"

#endif //SHV_UI
