/*++

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
    ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
    PARTICULAR PURPOSE.

    Copyright (c) Microsoft Corporation. All rights reserved

Module Name:

    TdhUtil.h

Abstract:

   Definitions of the functions for rerouting the binary event data to proper 
   formatting routines based on operating system version.  Also defines the 
   decoding context structures, and some utility methods to maintain the data
   in these structures.

--*/

#pragma once
#include "common.h"
#include <Tdh.h>

#define MIN_BUFFERSIZE_INCREMENT 65535
#define MIN_TEI_BUFFERSIZE  USHORT_MAX + 1
#define MIN_EMI_BUFFERSIZE  USHORT_MAX + 1
#define MIN_PROP_BUFFERSIZE 2 * (USHORT_MAX + 1)

//
// Define a function pointer type with the same signature as
// TdhFormatProperty(), which was added to the TDH API in Windows 7.
//

typedef
ULONG
(__stdcall *FPTR_TDH_FORMATPROPERTY)(
    __in PTRACE_EVENT_INFO EventInfo,
    __in_opt PEVENT_MAP_INFO MapInfo,
    __in ULONG PointerSize,
    __in USHORT PropertyInType,
    __in USHORT PropertyOutType,
    __in USHORT PropertyLength,
    __in USHORT UserDataLength,
    __in_bcount(UserDataLength) PBYTE UserData,
    __inout PULONG BufferSize,
    __out_bcount_opt(*BufferSize) PWCHAR Buffer,
    __out PUSHORT UserDataConsumed
    );

VOID
GetFormatPropertyHandle(
    __out HMODULE* TdhHandle,
    __out FPTR_TDH_FORMATPROPERTY* FormatPropertyPtr
    );


//
// The following is a user-defined structure that can be passed to functions
// that decode event data. It isolates the parameters related to 
// the event payload data, the rendering strings needed for formatting 
// the event message, and one temporary buffer for storing the formatted
// property data.
//

typedef struct _PROCESSING_DATA_CONTEXT {
    PBYTE Buffer;
    ULONG BufferSize;
    PBYTE MapInfoBuffer;
    ULONG MapInfoBufferSize;
    PBYTE EventInfoBuffer;
    ULONG EventInfoBufferSize;
    PWSTR* RenderItems;
    ULONG RenderItemsCount;
    LONG LastTopLevelIndex;
    LONG CurrentTopLevelIndex;
    PULONG ReferenceValues;
    ULONG ReferenceValuesCount;
    USHORT BinDataConsumed;
    USHORT BinDataLeft;
    USHORT UserDataOffset;
    
    _PROCESSING_DATA_CONTEXT():
        BufferSize(MIN_PROP_BUFFERSIZE)
        ,MapInfoBufferSize(MIN_EMI_BUFFERSIZE)
        ,EventInfoBufferSize(MIN_TEI_BUFFERSIZE)
        ,RenderItems(NULL)
        ,RenderItemsCount(0)
        ,ReferenceValues(NULL)
        ,ReferenceValuesCount(0)
        ,BinDataConsumed(0)
        ,UserDataOffset(0)
    {
    }

} PROCESSING_DATA_CONTEXT, *PPROCESSING_DATA_CONTEXT;

//
// The following is a user-defined structure that can be passed to the
// ProcessTrace API. EventCallback() and BufferCallback() functions can
// retrieve a pointer to this structure from EVENT_RECORD::UserContext
// and EVENT_TRACE_LOGFILE::Context respectively. This is used to pass
// logfile-specific information between Callbacks, which is a better 
// approach than using globals. This structure also isolates the 
// DataContext for each event, and the handle to tdh.dll if the 
// operatiog system is above Vista, in order to use the new Windows 7 API.
//

typedef struct _PROCESSING_CONTEXT {
    PROCESSING_DATA_CONTEXT DataContext;
    ULONG BufferCount;
    ULONGLONG EventCount;
    ULONG TimerResolution;
    BOOLEAN IsPrivateLogger;
    ULONG PointerSize;
    BOOLEAN DumpXml;
    PBYTE PrintBuffer;
    ULONG PrintBufferSize;
    HMODULE TdhDllHandle;
    FPTR_TDH_FORMATPROPERTY FormatPropertyPtr;

    _PROCESSING_CONTEXT():
        BufferCount(0)
        ,EventCount(0)
        ,TimerResolution(1)
        ,IsPrivateLogger(FALSE)
        ,DumpXml(FALSE)
        ,PrintBufferSize(MIN_PROP_BUFFERSIZE)
        ,TdhDllHandle(NULL)
    {
    }

    ~_PROCESSING_CONTEXT()
    {
        if (DataContext.Buffer != NULL) {
            free(DataContext.Buffer);
        }
        if (DataContext.EventInfoBuffer!= NULL) {
            free(DataContext.EventInfoBuffer);
        }
        if (DataContext.MapInfoBuffer!= NULL) {
            free(DataContext.MapInfoBuffer);
        }
        if (PrintBuffer != NULL) {
            free(PrintBuffer);
        }

        if (TdhDllHandle != NULL) {
            FreeLibrary(TdhDllHandle);
        }
    }

} PROCESSING_CONTEXT, *PPROCESSING_CONTEXT;

ULONG
InitializeProcessingContext(
    __inout PPROCESSING_CONTEXT LogContext
    );

ULONG
InitializeDataContext(
    __inout PPROCESSING_DATA_CONTEXT DataContext
    );

VOID
ResetDataContext(
    __inout PPROCESSING_DATA_CONTEXT DataContext
    );

ULONG
ResizeBuffer(
    __inout PBYTE* DataContext,
    __in PULONG BufferSize,
    __in ULONG NewBufferSize
    );

ULONG
UpdateRenderItem(
    __inout PPROCESSING_DATA_CONTEXT DataContext
    );

ULONG
GetFormattedBuffer(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in USHORT Length,
    __in USHORT PointerSize,
    __in USHORT InType,
    __in USHORT OutType,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
BitMapToString(
    __in PEVENT_MAP_INFO MapInfo,
    __in ULONG Value,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
ValueMapToString(
    __in PEVENT_MAP_INFO MapInfo,
    __in ULONG Value,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
MapToString(
    __in PEVENT_MAP_INFO MapInfo,
    __in ULONG Value,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

template <typename T>
ULONG
FormatMap(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in PEVENT_MAP_INFO MapInfo,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

ULONG
GetFormattedMapValue(
    __in_bcount(BinDataLeft) PBYTE BinDataPtr,
    __in ULONG BinDataLeft,
    __in PEVENT_MAP_INFO MapInfo,
    __in USHORT InType,
    __out_bcount(BufferSize) PBYTE Buffer,
    __in ULONG BufferSize,
    __out PUSHORT BinDataConsumed
    );

FORCEINLINE
BOOLEAN
PROPERTY_IS_STRUCTURE(
    __in PEVENT_PROPERTY_INFO Property
    )
{
    return (BOOLEAN)(Property->Flags & PropertyStruct);
}

FORCEINLINE
BOOLEAN
IS_WBEM_EVENT(
    __in PTRACE_EVENT_INFO EventInfo
    )
{
    return (EventInfo->DecodingSource == DecodingSourceWbem);
}

