#pragma once

#include "List.h"

// Abstract transport that allows writing and reading data. The goal here was to simplify sample's code,
// so it does not have to take into the account all complications associated with using sockets or pipes.
// However, it should be straightforward to replace this abstract transport with a concrete one list sockets/pipes/etc.
class Transport
{
private:
    CRITICAL_SECTION lock;
    List list;

public:
    Transport();

    ~Transport();

    HRESULT WriteData(
        _In_reads_bytes_opt_(dataLength) BYTE* data,
        _In_ ULONG dataLength);

    HRESULT ReadData(
        _In_ ULONG dataLength,
        _Inout_ ULONG* outputDataLength,
        _Inout_updates_bytes_to_opt_(dataLength, *outputDataLength) BYTE* data);
};
