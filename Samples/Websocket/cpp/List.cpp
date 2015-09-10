#include <Windows.h>
#include <StdLib.h>
#include <Assert.h>
#include "List.h"

ListEntry::ListEntry()
{
    this->backward = NULL;
    this->forward = NULL;
    this->data = NULL;
    this->index = 0;
    this->size = 0;
}

ListEntry::~ListEntry()
{
    if (this->data != NULL)
    {
        delete[] this->data;
        this->data = NULL;
        this->index = 0;
        this->size = 0;
    }
}

void ListEntry::Attach(
    _In_ ULONG dataLength,
    _In_reads_bytes_(dataLength) __drv_aliasesMem BYTE* data)
{
    this->index = 0;
    this->size = dataLength;
    this->data = data;
}

void ListEntry::CopyTo(
    _Inout_updates_bytes_to_(dataLength, *bytesCopied) BYTE* data,
    _In_ ULONG dataLength,
    _Out_ ULONG* bytesCopied)
{
    ULONG size = __min(dataLength, this->size);
    CopyMemory(data, this->data + this->index, size);
    this->index += size;
    *bytesCopied = size;
}

ULONG ListEntry::DataLeft() const
{
    return this->size - this->index;
}

List::List()
{
    this->head.forward = &this->head;
    this->head.backward = &this->head;
}

List::~List()
{
    assert(IsEmpty());
}

ListEntry* List::Peek()
{
    assert(!IsEmpty());

    return this->head.forward;
}

void List::RemoveHead()
{
    assert(!IsEmpty());

    ListEntry* first = this->head.forward;
    ListEntry* previous = first->backward;
    previous->forward = first->forward;
    first->forward->backward = first->backward;

    delete first;
}

void List::AppendTail(
    _In_ __drv_aliasesMem ListEntry* entry)
{
    ListEntry* last = this->head.backward;
    ListEntry* next = last->forward;

    last->forward = entry;
    next->backward = entry;
    entry->forward = next;
    entry->backward = last;
}

bool List::IsEmpty()
{
    return (this->head.forward == &this->head && this->head.backward == &this->head);
}
