// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

template<class T> class RdcSmartArray
{
public:
    RdcSmartArray();
    ~RdcSmartArray();

    size_t Size() const ;
    size_t Capacity() const ;
    T &operator[] ( size_t i );
    const T &operator[] ( size_t i ) const ;

    T *Begin() const;
    T *End() const;
    // single element
    T *Append();
    bool Append ( const T &t );
    T *Insert ( size_t i );
    bool Insert ( size_t i, const T &t );
    void Remove ( size_t i );

    // multiple elements
    T *AppendItems ( size_t c );
    bool AppendItems ( const T *p, size_t c );
    T *InsertItems ( size_t i, size_t c );
    bool InsertItems ( size_t i, const T *p, size_t c );
    void RemoveItems ( size_t i, size_t c );

    // misc methods
    void Clear();
    void Truncate ( size_t i );
    bool ReserveExact ( size_t n );
    bool ReserveAtLeast ( size_t n );

private:
    // array starts with this size
    static const size_t s_cMinAlloc = 16;

    // pointer to items
    T *m_p;

    // used items
    size_t m_nUsed;

    // allocated items
    size_t m_nAllocated;

    // left unimplemented
    RdcSmartArray ( const RdcSmartArray<T> & );

    // left unimplemented
    RdcSmartArray &operator= ( const RdcSmartArray<T> & );
};

//---------------------------------------------------------------------------
// Name: RdcSmartArray::RdcSmartArray
//
// Constructor
//
// Arguments:
//   None
//---------------------------------------------------------------------------

template <class T> inline RdcSmartArray<T>::RdcSmartArray()
{
    m_p = NULL;
    m_nUsed = 0;
    m_nAllocated = 0;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::~RdcSmartArray
//
// Destructor
//---------------------------------------------------------------------------

template <class T> inline RdcSmartArray<T>::~RdcSmartArray()
{
    Clear();
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Size
//
// Request number of elements in an array
//
// Arguments:
//   None
//
// Returns:
//   number of elements in an array
//---------------------------------------------------------------------------

template <class T> inline size_t RdcSmartArray<T>::Size() const
{
    return m_nUsed;
}

template <class T> inline size_t RdcSmartArray<T>::Capacity() const
{
    return m_nAllocated;
}
//---------------------------------------------------------------------------
// Name: RdcSmartArray::operator[]
//
// Indexation operator.
// Array template is the place, where it's reasonable to define it.
//
// Arguments:
//   i                  element index
//
// Returns:
//   Reference to indexed element
//---------------------------------------------------------------------------

template <class T> T &RdcSmartArray<T>::operator[] ( size_t i )
{
    RDCAssert ( i < m_nUsed );
    return m_p[i];
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::operator[]
//
// Indexation operator for constant object.
// The same as operator[], but returns constant reference.
// Compilator must be smart enough to choose apropriate operator.
//
// Arguments:
//   i                  element index
//
// Returns:
//   Constant reference to indexed element
//---------------------------------------------------------------------------

template <class T> const T &RdcSmartArray<T>::operator[] ( size_t i ) const
{
    RDCAssert ( i < m_nUsed );
    return m_p[i];
}

template <class T> T *RdcSmartArray<T>::Begin() const
{
    RDCAssert ( m_p );
    return m_p;
}

template <class T> T *RdcSmartArray<T>::End() const
{
    RDCAssert ( m_p );
    return m_p + m_nUsed;
}
//---------------------------------------------------------------------------
// Name: RdcSmartArray::ReserveExact
//
// Extends array.
//
// Arguments:
//   n                  array size in elements
//
// Returns:
//   true if array has been successfuly extended
//---------------------------------------------------------------------------

template <class T> bool RdcSmartArray<T>::ReserveExact ( size_t n )
{
    // must fit data
    n = Maximum ( n, s_cMinAlloc );
    n = Maximum ( n, m_nUsed );

    // is changed ?
    if ( n == m_nAllocated )
        return true;

    // Check for overflow of the allocation...
    if ( n > 0x7ffffffe / sizeof ( T ) )
    {
        // We should throw an exception if exceptions
        // are being used, and just return NULL otherwise.
        return false;
    }
    // allocate
    T *p = ( T * ) new ( std::nothrow ) BYTE[n * sizeof ( T ) ];
    if ( p == NULL )
        return false;

    // copy existing data (no constructors!)
    if ( m_nUsed != 0 )
        CopyMemory ( p, m_p, m_nUsed * sizeof ( T ) );

    // free old data (no destructors!)
    if ( m_nAllocated != 0 )
        delete [] ( BYTE* ) m_p;

    // reassign
    m_p = p;
    m_nAllocated = n;
    return true;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::ReserveAtLeast
//
// Private function, extends array with extraallocation.
//
// Arguments:
//   n                  new array size in elements
//
// Returns:
//   true if array has been successfuly extended
//---------------------------------------------------------------------------

template <class T> bool RdcSmartArray<T>::ReserveAtLeast ( size_t n )
{
    if ( n > m_nAllocated )
    {
        // too large ?
        if ( int ( n ) < 0 )
            return false;

        // find power of two greater of equal to n
        size_t nAlloc = s_cMinAlloc;
        while ( nAlloc < n )
            nAlloc <<= 1;

        // allocate this size
        if ( !ReserveExact ( nAlloc ) )
            return false;
    }

    return true;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Append
//
// Append element to array
// Empty constructor is called for the new element.
//
// Arguments:
//   None
//
// Returns:
//   Pointer to a new element or NULL, if can't allocate
//---------------------------------------------------------------------------

template <class T> T *RdcSmartArray<T>::Append()
{
    if ( m_nUsed == m_nAllocated && !ReserveAtLeast ( m_nUsed + 1 ) )
        return NULL;

    // call constructor
    T *p = new ( m_p + m_nUsed ) T;

    // allocated
    m_nUsed++;

    return p;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Append
//
// Append element to array
// Copy constructor is called for the new element.
//
// Arguments:
//   t                  element to append
//
// Returns:
//   true is successfuly appended, false if can't allocate
//---------------------------------------------------------------------------

template <class T> bool RdcSmartArray<T>::Append ( const T &t )
{
    if ( m_nUsed == m_nAllocated && !ReserveAtLeast ( m_nUsed + 1 ) )
        return false;

    // call constructor
    new ( m_p + m_nUsed ) T ( t );

    // allocated
    m_nUsed++;

    return true;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Insert
//
// Insert element into array
// Empty constructor is called for the new element.
//
// Arguments:
//   nAt                index to insert before
//                      May be equal to Size(), that means Append
//
// Returns:
//   Pointer to a new element or NULL, if can't allocate
//---------------------------------------------------------------------------

template <class T> T *RdcSmartArray<T>::Insert ( size_t nAt )
{
    RDCAssert ( nAt <= m_nUsed );

    // alloc element
    if ( m_nUsed == m_nAllocated && !ReserveAtLeast ( m_nUsed + 1 ) )
        return NULL;

    // shift data after nAt
    if ( nAt < m_nUsed )
        MoveMemory ( m_p + nAt + 1, m_p + nAt, ( m_nUsed - nAt ) * sizeof ( T ) );

    m_nUsed++;

    // call constructor
    return new ( m_p + nAt ) T;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Insert
//
// Insert element into array
// Copy constructor is called for the new element.
//
// Arguments:
//   nAt                index to insert before
//                      May be equal to Size(), that means Append
//   t                  element to insert
//
// Returns:
//   true is successfuly inserted, false if can't allocate
//---------------------------------------------------------------------------

template <class T> bool RdcSmartArray<T>::Insert ( size_t nAt, const T &t )
{
    RDCAssert ( nAt <= m_nUsed );

    // alloc element
    if ( m_nUsed == m_nAllocated && !ReserveAtLeast ( m_nUsed + 1 ) )
        return false;

    // shift data after nAt
    if ( nAt < m_nUsed )
        MoveMemory ( m_p + nAt + 1, m_p + nAt, ( m_nUsed - nAt ) * sizeof ( T ) );

    m_nUsed++;

    // call constructor
    new ( m_p + nAt ) T ( t );
    return true;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Remove
//
// Remove an element from an array
//
// Arguments:
//   nAt                element index
//
// Returns:
//   None
//---------------------------------------------------------------------------

template <class T> void RdcSmartArray<T>::Remove ( size_t nAt )
{
    RDCAssert ( nAt < m_nUsed );

    // call destructor
    ( m_p + nAt ) ->T::~T();

    // shift data after nAt
    if ( nAt + 1 < m_nUsed )
        MoveMemory ( m_p + nAt, m_p + nAt + 1, ( m_nUsed - nAt - 1 ) * sizeof ( T ) );

    m_nUsed--;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::AppendItems
//
// Append c items to array
// Empty constructor is called for every new element.
//
// Arguments:
//   c                  number of elements to append
//
// Returns:
//   Pointer to the start of new elements or NULL, if can't allocate
//---------------------------------------------------------------------------

template <class T> T *RdcSmartArray<T>::AppendItems ( size_t c )
{
    size_t nNewSize = m_nUsed + c;
    if ( nNewSize > m_nAllocated && !ReserveAtLeast ( nNewSize ) )
        return NULL;

    T *ptr = m_p + m_nUsed;

    // call constructors
    while ( c != 0 )
    {
        new ( m_p + m_nUsed ) T;
        m_nUsed++;
        c--;
    }

    return ptr;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::AppendItems
//
// Append elements to array
// Copy constructor is called for every new element.
//
// Arguments:
//   p                  pointer to elements to append
//   c                  number of elements to append
//
// Returns:
//   true is successfuly appended, false if can't allocate
//---------------------------------------------------------------------------

template <class T> bool RdcSmartArray<T>::AppendItems ( const T *p, size_t c )
{
    RDCAssert ( c == 0 || p != NULL );

    size_t nNewSize = m_nUsed + c;
    if ( nNewSize > m_nAllocated && !ReserveAtLeast ( nNewSize ) )
        return false;

    // call constructors
    while ( c != 0 )
    {
        new ( m_p + m_nUsed ) T ( *p++ );
        m_nUsed++;
        c--;
    }

    return true;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::InsertItems
//
// Insert elements into array
// Empty constructor is called for every new element.
//
// Arguments:
//   nAt                index to insert before
//                      May be equal to Size(), that means Append
//   c                  number of elements to insert
//
// Returns:
//   Pointer to start of new elements or NULL, if can't allocate
//---------------------------------------------------------------------------

template <class T> T *RdcSmartArray<T>::InsertItems ( size_t nAt, size_t c )
{
    RDCAssert ( nAt <= m_nUsed );

    size_t nNewSize = m_nUsed + c;
    if ( nNewSize > m_nAllocated && !ReserveAtLeast ( nNewSize ) )
        return NULL;

    // shift data after nAt
    if ( nAt < m_nUsed )
        MoveMemory ( m_p + nAt + c, m_p + nAt, ( m_nUsed - nAt ) * sizeof ( T ) );

    m_nUsed += c;

    T *ptr = m_p + nAt;

    // call constructors
    while ( c )
    {
        new ( m_p + nAt++ ) T;
        c--;
    }

    return ptr;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::InsertItems
//
// Insert elements into array
// Copy constructor is called for every new element.
//
// Arguments:
//   nAt                index to insert before
//                      May be equal to Size(), that means Append
//   p                  pointer to elements to insert
//   c                  number of elements to insert
//
// Returns:
//   true is successfuly inserted, false if can't allocate
//---------------------------------------------------------------------------

template <class T> bool RdcSmartArray<T>::InsertItems ( size_t nAt, const T *p, size_t c )
{
    RDCAssert ( c == 0 || p != NULL );
    RDCAssert ( nAt <= m_nUsed );

    size_t nNewSize = m_nUsed + c;
    if ( nNewSize > m_nAllocated && !ReserveAtLeast ( nNewSize ) )
        return false;

    // shift data after nAt
    if ( nAt < m_nUsed )
        MoveMemory ( m_p + nAt + c, m_p + nAt, ( m_nUsed - nAt ) * sizeof ( T ) );

    m_nUsed += c;

    // call constructors
    while ( c != 0 )
    {
        new ( m_p + nAt++ ) T ( *p++ );
        c--;
    }

    return true;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::RemoveItems
//
// Remove an elements from an array
//
// Arguments:
//   nAt                element index
//   c                  elements to remove
//
// Returns:
//   None
//---------------------------------------------------------------------------

template <class T> void RdcSmartArray<T>::RemoveItems ( size_t nAt, size_t c )
{
    RDCAssert ( nAt + c <= m_nUsed );

    // call destructors
    size_t nEnd = nAt + c;
    for ( size_t i = nAt; i < nEnd; i++ )
        ( m_p + i ) ->T::~T();

    // shift data after nAt
    if ( nEnd < m_nUsed )
        MoveMemory ( m_p + nAt, m_p + nEnd, ( m_nUsed - nEnd ) * sizeof ( T ) );

    m_nUsed -= c;
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Truncate
//
// Truncate array to given size
//
// Arguments:
//   n                  new count of elements
//
// Returns:
//   None
//---------------------------------------------------------------------------

template <class T> void RdcSmartArray<T>::Truncate ( size_t n )
{
    while ( m_nUsed > n )
    {
        --m_nUsed;
        // call destructor now
        ( m_p + m_nUsed ) ->T::~T();
    }
}

//---------------------------------------------------------------------------
// Name: RdcSmartArray::Clear
//
// Remove all elements from an array.
//
// Arguments:
//   None
//
// Returns:
//   None
//---------------------------------------------------------------------------

template <class T> void RdcSmartArray<T>::Clear()
{
    Truncate ( 0 );
    if ( m_nAllocated != 0 )
    {
        // free, no destructors!
        delete [] ( BYTE* ) m_p;
        m_p = NULL;
        m_nAllocated = 0;
    }
}


