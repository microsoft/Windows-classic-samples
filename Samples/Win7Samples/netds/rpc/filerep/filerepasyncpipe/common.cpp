// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.


/*
    

    File Replication Sample
    Server System Service

    FILE: common.cpp

    PURPOSE: Provides common file replication service routine definitions.

*/

#include "common.h"

#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>

HANDLE hProcessHeap = NULL;

VOID GetEEInfoText(TCHAR *Msg, UINT BuffSize, UINT *MsgSize) {
    RPC_STATUS status;
    RPC_ERROR_ENUM_HANDLE EnumHandle;

    *MsgSize +=  _sntprintf_s(&Msg[*MsgSize], BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("EEInfo:\n"));
	
    status = RpcErrorStartEnumeration(&EnumHandle);

    if (status == RPC_S_ENTRY_NOT_FOUND) {
       *MsgSize += _sntprintf_s(&Msg[*MsgSize], BuffSize - *MsgSize,  BuffSize - *MsgSize, TEXT("no EEInfo found\n"));
    }
	
    else if (status != RPC_S_OK) {
       *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("could not get EEInfo\n"));
    }
    else {
        RPC_EXTENDED_ERROR_INFO ErrorInfo;
        
        while (status == RPC_S_OK) {
            ErrorInfo.Version = RPC_EEINFO_VERSION;
            ErrorInfo.Flags = 0;
            ErrorInfo.NumberOfParameters = 4;

            status = RpcErrorGetNextRecord(&EnumHandle, FALSE, &ErrorInfo);
            if (status == RPC_S_ENTRY_NOT_FOUND) {
                break;
            }
            else if (status != RPC_S_OK) {
               *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("enumeration broke with status %d\n"), status);
                break;
            }
            else {
                INT i;

               *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("GeneratingComponent: %d\n"), ErrorInfo.GeneratingComponent);
               *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("Status: %d\n"), ErrorInfo.Status);
               *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("Detection Location: %d\n"), ErrorInfo.DetectionLocation);
               *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("Flags: %d\n"), ErrorInfo.Flags);
               *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("NumberOfParameters: %d\n"), ErrorInfo.NumberOfParameters);
                
                for (i = 0; i < ErrorInfo.NumberOfParameters; i++) {
                    switch (ErrorInfo.Parameters[i].ParameterType) {
                    case eeptAnsiString:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("AnsiString: %s\n"), ErrorInfo.Parameters[i].u.AnsiString);
                        break;
                        
                    case eeptUnicodeString:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("UnicodeString: %S\n"), ErrorInfo.Parameters[i].u.UnicodeString);                        
                        break;
                        
                    case eeptLongVal:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("LongVal: %d\n"), ErrorInfo.Parameters[i].u.LVal);
                        break;
                        
                    case eeptShortVal:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("ShortVal: %d\n"), ErrorInfo.Parameters[i].u.SVal);                        
                        break;
                        
                    case eeptPointerVal:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("PointerVal: %d\n"), ErrorInfo.Parameters[i].u.PVal);                        
                        break;
                        
                    case eeptNone:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("Invalid type of parameter %d\n"), ErrorInfo.Parameters[i].ParameterType);
                        break;
                        
                    default:
                        *MsgSize += _sntprintf_s(&Msg[*MsgSize],  BuffSize - *MsgSize, BuffSize - *MsgSize, TEXT("Invalid type of parameter %d\n"), ErrorInfo.Parameters[i].ParameterType);    
                    }
                }
            }
        }
    }
}

VOID PrintProcFailureEEInfo(LPTSTR ProcName, DWORD ErrCode) {
    TCHAR Msg[MSG_SIZE];
    UINT MsgSize = 0;

    MsgSize += _stprintf_s(&Msg[MsgSize], MSG_SIZE, TEXT("%s failed with code %d\n"), ProcName, ErrCode);
	GetEEInfoText(Msg, MSG_SIZE, &MsgSize);

	Msg[MsgSize] = 0;

    _tprintf_s(TEXT("%s"), Msg);
}

// Memory management functions.
VOID * AutoHeapAlloc(size_t dwBytes) {
    if (hProcessHeap == NULL) {
        hProcessHeap = GetProcessHeap();
    }
    return HeapAlloc(hProcessHeap, 0, dwBytes);
}

VOID AutoHeapFree(VOID * lpMem) {
    DWORD status;
    ASSERT(hProcessHeap != NULL);
    status = HeapFree(hProcessHeap, 0, lpMem);
    ASSERT(status);
}


// end common.cpp

