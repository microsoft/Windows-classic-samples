// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module implements a linked list

#pragma once

template<typename T>
class TList
{
// Member classes
public:

    class TIterator;

    class TListEntry
    {
        friend TList;
        friend TIterator;

        // Protected member functions
    protected:
        TListEntry():
            m_pNextListEntry(NULL),
            m_pPrevListEntry(NULL)
        {
        }

        // Protected member variables
    protected:
        TListEntry* m_pNextListEntry;
        TListEntry* m_pPrevListEntry;
    };  // TListEntry

    class TIterator
    {
        friend TList;

        // Public member functions
    public:
        TIterator():m_pEntry(NULL) {}
        inline T& operator*() const { return *(T*)m_pEntry; } 
        inline T* operator->()  const { return (T*) m_pEntry; }
        inline bool operator==(const TIterator& iter) const { return (m_pEntry == iter.m_pEntry); }
        inline bool operator!=(const TIterator& iter) const { return (m_pEntry != iter.m_pEntry); }
        inline TIterator operator++() 
        {
            m_pEntry = m_pEntry->m_pNextListEntry;
            return TIterator(m_pEntry);
        }
        inline TIterator operator++(int)
        {
            TListEntry* pEntry = m_pEntry;
            m_pEntry = m_pEntry->m_pNextListEntry
            return TIterator(pEntry);
        }

        // Protected member functions
    protected:
        TIterator(__in TListEntry* pEntry): m_pEntry(pEntry) {}

        // Protected member variables
    protected:
        TListEntry* m_pEntry;
    };  // TIterator

// Member functions
public:
    TList();
    inline bool IsEmpty() const;
    inline size_t GetCount() const;
    
    inline T* GetHead();
    inline T* GetTail();
    inline void RemoveEntry(T* pEntry);
    inline T* RemoveHead();
    inline T* RemoveTail();
    inline void InsertTail(__in T* pEntry);
    inline void InsertHead(__in T* pEntry);
    bool RemoveEntryIfInList(__in T* pEntry);
    inline TIterator Begin()
    {
        return TIterator(m_ListHead.m_pNextListEntry);
    }
    inline TIterator End()
    {
        return TIterator(&m_ListHead);
    }

// Member variables
protected:
    TListEntry m_ListHead;
    size_t m_cElementCount;

};  // TList

//---------------------------------------------------------------------------
// Begin TList implementation
//---------------------------------------------------------------------------

template<typename T>
TList<T>::TList():
    m_cElementCount(0)
{
    m_ListHead.m_pNextListEntry = m_ListHead.m_pPrevListEntry = &m_ListHead;
}  // TList<T>::TList

template<typename T>
bool TList<T>::IsEmpty() const
{
    return (m_cElementCount == 0);
}  // TList<T>::IsEmpty

template<typename T>
size_t TList<T>::GetCount() const
{
    return m_cElementCount;
}  // TList<T>::GetCount

template<typename T>
T* TList<T>::GetHead()
{
    assert (m_cElementCount != 0);

    return (T*) m_ListHead.m_pNextListEntry;
}  // TList<T>::GetHead()

template<typename T>
T* TList<T>::GetTail()
{
    assert (m_cElementCount != 0);

    return (T*) m_ListHead.m_pPrevListEntry;
}  // TList<T>::GetTail

template<typename T>
void TList<T>::RemoveEntry(T* pEntry)
{
    TListEntry* pNextListEntry;
    TListEntry* pPrevListEntry;

    pNextListEntry = pEntry->m_pNextListEntry;
    pPrevListEntry = pEntry->m_pPrevListEntry;
    pPrevListEntry->m_pNextListEntry = pNextListEntry;
    pNextListEntry->m_pPrevListEntry = pPrevListEntry;

    pEntry->m_pNextListEntry = NULL;
    pEntry->m_pPrevListEntry = NULL;

    --m_cElementCount;
}  // TList<T>::RemoveEntry

template<typename T>
T* TList<T>::RemoveHead()
{
    TListEntry* pNextListEntry;
    TListEntry* pEntry;

    assert (m_cElementCount != 0);

    pEntry = m_ListHead.m_pNextListEntry;
    pNextListEntry = pEntry->m_pNextListEntry;
    m_ListHead.m_pNextListEntry = pNextListEntry;
    pNextListEntry->m_pPrevListEntry = &m_ListHead;

    pEntry->m_pNextListEntry = NULL;
    pEntry->m_pPrevListEntry = NULL;

    --m_cElementCount;

    return (T*) pEntry;
}  // TList<T>::RemoveHead

template<typename T>
T* TList<T>::RemoveTail()
{
    TListEntry* pPrevListEntry;
    TListEntry* pEntry;

    assert (m_cElementCount != 0);

    pEntry = m_ListHead.m_pPrevListEntry;
    pPrevListEntry = pEntry->m_pPrevListEntry;
    m_ListHead.m_pPrevListEntry = pPrevListEntry;
    pPrevListEntry->m_pNextListEntry = &m_ListHead;

    pEntry->m_pNextListEntry = NULL;
    pEntry->m_pPrevListEntry = NULL;

    --m_cElementCount;

    return (T*) pEntry;
}  // TList<T>::RemoveTail

template<typename T>
void TList<T>::InsertTail(__in T* pEntry)
{
    TListEntry* pPrevListEntry;

    pPrevListEntry = m_ListHead.m_pPrevListEntry;
    pEntry->m_pNextListEntry = &m_ListHead;
    pEntry->m_pPrevListEntry = pPrevListEntry;
    pPrevListEntry->m_pNextListEntry = pEntry;
    m_ListHead.m_pPrevListEntry = pEntry;

    ++m_cElementCount;
}  // TList<T>::InsertTail

template<typename T>
void TList<T>::InsertHead(__in T* pEntry)
{
    TListEntry* pNextListEntry;

    pNextListEntry = m_ListHead.m_pNextListEntry;
    pEntry->m_pNextListEntry = pNextListEntry;
    pEntry->m_pPrevListEntry = &m_ListHead;
    pNextListEntry->m_pPrevListEntry = pEntry;
    m_ListHead.m_pNextListEntry = pEntry;

    ++m_cElementCount;
}  // TList<T>::InsertHead

template<typename T>
bool TList<T>::RemoveEntryIfInList(__in T* pEntry)
{
    bool fInList = false;

    for (
        TListEntry* pCurrentEntry = m_ListHead.m_pNextListEntry;
        !fInList && (pCurrentEntry != &m_ListHead);
        pCurrentEntry = pCurrentEntry->m_pNextListEntry)
    {
        fInList = (pCurrentEntry == pEntry);
    }

    if (fInList)
    {
        RemoveEntry(pEntry);
    }

    return fInList;
}  // TList<T>::RemoveEntryIfInList

//---------------------------------------------------------------------------
// End TList implementation
//---------------------------------------------------------------------------
