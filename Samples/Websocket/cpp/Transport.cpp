#include <Windows.h>
#include <Assert.h>
#include <stdlib.h>
#include "Transport.h"
#include <new>

Transport::Transport()
{
    InitializeCriticalSection(&this->lock);
}

Transport::~Transport()
{
    DeleteCriticalSection(&this->lock);
}

HRESULT Transport::WriteData(
    _In_reads_bytes_opt_(dataLength) BYTE* data,
    _In_ ULONG dataLength)
{
    HRESULT hr = S_OK;
    ListEntry* entry = NULL;
    BYTE* buffer = NULL;

    if (data == NULL)
    {
        goto quit;
    }

    entry = new(std::nothrow) ListEntry();
    if (entry == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto quit;
    }

    buffer = new(std::nothrow) BYTE[dataLength];
    if (buffer == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto quit;
    }

    // Copy application buffer to the transport buffer.
    CopyMemory(buffer, data, dataLength);
    entry->Attach(dataLength, buffer);
    buffer = NULL;

    // Add the entry to the list.
    EnterCriticalSection(&this->lock);
    list.AppendTail(entry);
    entry = NULL;
    LeaveCriticalSection(&this->lock);

quit:

    if (entry != NULL)
    {
        delete entry;
        entry = NULL;
    }

    if (buffer != NULL)
    {
        delete[] buffer;
        buffer = NULL;
    }

    return hr;
}

HRESULT Transport::ReadData(
    _In_ ULONG dataLength,
    _Inout_ ULONG* outputDataLength,
    _Inout_updates_bytes_to_opt_(dataLength, *outputDataLength) BYTE* data)
{
    *outputDataLength = 0;
    ULONG index = 0;

    if (data == NULL)
    {
        return E_FAIL;
    }

    EnterCriticalSection(&this->lock);

    // Read as much data as possible from the transport.
    while (index < dataLength && !this->list.IsEmpty())
    {
        ListEntry* first = list.Peek();

        // Copy data from the transport buffer to the application buffer.
        ULONG copiedBytes = 0;
        first->CopyTo(data + index, dataLength - index, &copiedBytes);
        index += copiedBytes;

        // Remove the entry if there are not more data to process.
        if (first->DataLeft() == 0)
        {
            list.RemoveHead();
        }
    }

    LeaveCriticalSection(&this->lock);

    *outputDataLength = index;
    assert(*outputDataLength != 0);
    return S_OK;
}
