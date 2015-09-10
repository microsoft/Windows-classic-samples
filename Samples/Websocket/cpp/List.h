#pragma once

// List entry that contain a buffer that is transmitted between peers.
class ListEntry
{
private:
    ListEntry* forward;
    ListEntry* backward;
    ULONG size;
    ULONG index;
    BYTE* data;
    friend class List;

public:
    ListEntry();

    ~ListEntry();

    void Attach(
        _In_ ULONG dataLength,
        _In_reads_bytes_(dataLength) __drv_aliasesMem BYTE* data);

    void CopyTo(
        _Inout_updates_bytes_to_(dataLength, *bytesCopied) BYTE* data,
        _In_ ULONG dataLength,
        _Out_ ULONG* bytesCopied);

    ULONG DataLeft() const;
};

// Double-linked list.
class List
{
private:
    ListEntry head;

public:
    List();

    ~List();

    ListEntry* Peek();

    void RemoveHead();

    void AppendTail(
        _In_ __drv_aliasesMem ListEntry* entry);

    bool IsEmpty();
};
