// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#define SC_ASSERT Assert

const HRESULT E_NOTFOUND = HRESULT_FROM_WIN32(ERROR_NOT_FOUND);

//----------------------------------------------------------------------------
// NonCopyAble - Base class for objects that can't be copied using assignement
// operators or copy constructors.  Objects that drive from this class
// typically provide a separate initialization function that reurn errors
// upon failure.  Constructors and assignment operators can not perform any
// operation that can fail because they have no good way to return errors.
//----------------------------------------------------------------------------
class NonCopyAble
{
public:
    NonCopyAble(){}
    ~NonCopyAble(){}

private:
    NonCopyAble(const NonCopyAble& other);
    NonCopyAble& operator=(const NonCopyAble& other);
};

//----------------------------------------------------------------------------
// CDefaultTraits - Default helper for comparing, constructing, and
// assigning elements.
//
// Override if using complex types without operator== or operator= or copy
// constructors.
//----------------------------------------------------------------------------
template <class T>
class CDefaultTraits
{
public:
    static bool IsEqual(const T& t1, const T& t2)
    {
        return t1 == t2;
    }

    template<class X>
    static HRESULT Construct(
        T *pDest,                   // Pointer to destination element
        const X &xSrc               // Source to assign
        )
    {
        // Use placement new to call the copy constructor.  Note that since we
        // don't use exceptions, constructors are not allowed to fail. Objects
        // that require more complex initialization should have a custom traits class
        // that contruct and then initialize returning errors as appropriate.
        new(pDest) T(xSrc);
        return S_OK;
    }

    template<class X>
    static HRESULT Assign(
        T *pDest,                   // Pointer to destination element
        const X &xSrc               // Source to assign
        )
    {
        // Call the object's assignment operator.  Override this method if
        // with a custom traits class if assignment can fail.
        *pDest = xSrc;
        return S_OK;
    }
};

//----------------------------------------------------------------------------
// CArray - Dynamically sized array based off of ATL's CSimpleArray.
//----------------------------------------------------------------------------
template <class T, class TElementTraits = CDefaultTraits< T > >
class CArray : public NonCopyAble
{
public:
    static const UINT NotFound;

    // Construction/destruction
    CArray();
    ~CArray();

    // Operations
    UINT        GetCount() const;
    UINT        GetCapacity() const;
    HRESULT     RemoveAt(UINT nIndex);
    void        RemoveAll();
    const T&    operator[] (UINT nIndex) const;
    T&          operator[] (UINT nIndex);
    T*          GetData() const;
    HRESULT     Resize(UINT nNewSize);

    T&          GetFront()
    {
        Assert(GetCount() != 0);
        return (*this)[0];
    }

    T&          GetBack()
    {
        Assert(GetCount() != 0);
        return (*this)[GetCount()-1];
    }

    void        RemoveLast()
    {
        Assert(GetCount() != 0);

        // We can safely ignore the return value here because Resize will never
        // fail when reducing the size of the array (no reallocation is needed).
        Resize(GetCount()-1);
    }

    //
    // The following three functions are templated to permit the caller to
    // assign different types to our array.  The TElementTraits must support
    // the types used.  For example an array of CString objects might allow
    // both CString and PWSTR variable types to be added.
    //

    //
    // Removes the specified element from the array. All following elements are
    // shifted down.
    //
    template<typename X>
    HRESULT Remove(
        const X &x                  // Element to remove
        )
    {
        HRESULT hr = S_OK;
        UINT nIndex = Find(x);
        if (nIndex != NotFound)
        {
            hr = RemoveAt(nIndex);
        }
        else
        {
            hr = E_NOTFOUND;
        }
        return hr;
    }

    //
    // Finds an element in the array. Returns the index of the found element, or
    // NotFound if the element is not found.
    //
    template<typename X>
    UINT Find (
        const X &x                  // Element to find
        ) const
    {
        for (UINT i = 0; i < _nSize; i++)
        {
            if (TElementTraits::IsEqual(_aT[i], x))
            {
                return i;
            }
        }

        // Not found
        return NotFound;
    }

    //
    // Inserts an item at the specified index shifting following items up one slot.
    //
    template<typename X>
    HRESULT InsertAt(
        UINT nIndex,                // Index where to insert
        const X &x                  // Element to insert
        )
    {
        HRESULT hr;
        UINT nOldSize = _nSize;

        hr = MakeRoomAt(nIndex);
        if (SUCCEEDED(hr))
        {
            // If index is past the end, call default constructor
            // for all items between the old end and new index
            if (nIndex > nOldSize)
            {
                for (UINT i = nOldSize; i < nIndex; ++i)
                {
                    new(_aT + i) T;
                }
            }

            hr = TElementTraits::Construct(_aT + nIndex, x);
            if (FAILED(hr))
            {
                // Element assignment failed, remove inserted element
                RemoveAt(nIndex);
            }
        }
        return hr;
    }

    //
    // Appends element to the end of the array.
    //
    template<typename X>
    HRESULT Add(
        const X &x                  // Element to add
        )
    {
        HRESULT hr = S_OK;
        if (_nSize == _nAllocSize)
        {
            hr = EnsureCapacity(_nSize + 1);
        }
        if (SUCCEEDED(hr))
        {
            _nSize++;
            hr = TElementTraits::Construct(_aT + _nSize - 1, x);
            if (FAILED(hr))
            {
                // Element assignment failed, remove element
                RemoveAt(_nSize - 1);
            }
        }
       return hr;
    }

    // Set the specified element in the array
    template<typename X>
    HRESULT SetAt(
        UINT nIndex,                // Element index to Set
        const X &x                  // Element to assign
        )
    {
        if (nIndex < 0 || nIndex >= _nSize)
        {
            hr = E_INVALIDARG;
        }
        if (SUCCEEDED(hr))
        {
            hr = TElementTraits::Assign(_aT + nIndex, x);
        }
        return hr;
    }

    // Copy a C array of a type into the array
    template<typename X>
    HRESULT AddArray(
        const X *pxSource,          // Array to to copy
        UINT cItems                 // Count of elements to copy
        )
    {
        HRESULT hr = S_OK;
        if (cItems < 0)
        {
            hr = E_INVALIDARG;
        }
        if (SUCCEEDED(hr))
        {
            hr = EnsureCapacity(_nSize + cItems);

            for (UINT i = 0; SUCCEEDED(hr) && i < cItems; i++)
            {
                hr = Add(pxSource[i]);
            }
        }
        return hr;
    }

    template<typename X, typename Traits>
    HRESULT AddArray(
        const CArray<X, Traits> &other      // Array to add
        )
    {
        return AddArray(other.GetData(), other.GetCount());
    }

    HRESULT     EnsureCapacity(UINT cCapacityDesired);

protected:
    HRESULT     MakeRoomAt(UINT nIndex);

private:
    T*          _aT;
    UINT        _nSize;
    UINT        _nAllocSize;
};


template <class T, class TElementTraits>
const UINT CArray<T, TElementTraits>::NotFound = UINT_MAX;

//
// Constructor
//
template <class T, class TElementTraits>
CArray<T, TElementTraits>::CArray()
:
    _aT(NULL),
    _nSize(0),
    _nAllocSize(0)
{
}

//
// Destructor
//
template <class T, class TElementTraits>
CArray<T, TElementTraits>::~CArray()
{
    RemoveAll();
}

//
// Return the number of elements in the array
//
template <class T, class TElementTraits>
UINT CArray<T, TElementTraits>::GetCount() const
{
    return _nSize;
}

//
// Return the capacity of the array
//
template <class T, class TElementTraits>
UINT CArray<T, TElementTraits>::GetCapacity() const
{
    return _nAllocSize;
}


//
// Allocates sufficient memory to hold the desired number of items.
//
template <class T, class TElementTraits>
HRESULT CArray<T, TElementTraits>::EnsureCapacity(
    UINT cCapacityDesired           // Capacity count to check and allocate
    )
{
    HRESULT hr = S_OK;

    if (_nAllocSize < cCapacityDesired)
    {

#pragma prefast(push)
#pragma prefast(disable:6287 "Prefast can't determne that sizeof(T) != 1")

        if (_nSize > (UINT_MAX / 2) ||
            _nSize > (SIZE_MAX / 2 / sizeof(T)))
#pragma prefast(pop)

        {
            hr = E_FAIL;
        }
        else
        {
            const UINT nNewAllocSize = max(_nSize * 2, cCapacityDesired);

            T *aT;

            //
            // If we have already allocated the array. Note, _nSize might be zero
            // at this point if we had resized to 0.
            //
            if (_aT)
            {
                aT = static_cast<T*>(realloc(_aT, nNewAllocSize * sizeof(T)));
            }
            else
            {
                aT = static_cast<T*>(malloc(nNewAllocSize * sizeof(T)));
            }

            if (aT == NULL)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                _nAllocSize = nNewAllocSize;
                _aT = aT;
            }
        }
    }

    return hr;
}

//
// Adds an empty slot at the specified index.
//
template <class T, class TElementTraits>
HRESULT CArray<T, TElementTraits>::MakeRoomAt(
    UINT nIndex                     // Index to add an empty element
    )
{
    HRESULT hr = S_OK;
    if (nIndex < 0)
    {
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        UINT cElemGrowTo;

        if (nIndex >= _nSize)
        {
            cElemGrowTo = nIndex + 1;
        }
        else
        {
            cElemGrowTo = _nSize + 1;
        }

        if (cElemGrowTo > _nAllocSize)
        {
            hr = EnsureCapacity(cElemGrowTo);
        }
        if (SUCCEEDED(hr))
        {
            if (nIndex < _nSize)
            {
                // Move the remaining elements out so there's room
                memmove((void*)(_aT + nIndex + 1), (void*)(_aT + nIndex), (_nSize - nIndex) * sizeof(T));
            }
            _nSize = cElemGrowTo;
        }
    }
    return hr;
}

//
// Removes the element at the specified index and shift all following elements
// down an index.
//
template <class T, class TElementTraits>
HRESULT CArray<T, TElementTraits>::RemoveAt(
    UINT nIndex                     // Index of element to remove
    )
{
    HRESULT hr = S_OK;
    SC_ASSERT(nIndex >= 0 && nIndex < _nSize);
    if (nIndex < 0 || nIndex >= _nSize)
    {
        hr = E_INVALIDARG;
    }
    if (SUCCEEDED(hr))
    {
        _aT[nIndex].~T();
        if (nIndex != (_nSize - 1))
        {
            memmove((void *)(_aT + nIndex), (void *)(_aT + nIndex + 1), (_nSize - (nIndex + 1)) * sizeof(T));
        }
        _nSize--;
    }
    return hr;
}

//
// Removes all elements from the array
//
template <class T, class TElementTraits>
void CArray<T, TElementTraits>::RemoveAll()
{
    if (_aT != NULL)
    {
        for (UINT i = 0; i < _nSize; i++)
        {
            _aT[i].~T();
        }

        free(_aT);
        _aT = NULL;
    }
    _nSize = 0;
    _nAllocSize = 0;
}

//
// Array operator for accessing elements.  This version works for const arrays.
//
template <class T, class TElementTraits>
const T& CArray<T, TElementTraits>::operator[] (UINT nIndex) const
{
    SC_ASSERT(nIndex >= 0 && nIndex < _nSize);
    return _aT[nIndex];
}

//
// Array operator for accessing elements.
//
template <class T, class TElementTraits>
T& CArray<T, TElementTraits>::operator[] (UINT nIndex)
{
    SC_ASSERT(nIndex >= 0 && nIndex < _nSize);
    return _aT[nIndex];
}

//
// Returns a pointer to the data stored in the array.
//
template <class T, class TElementTraits>
T* CArray<T, TElementTraits>::GetData() const
{
    return _aT;
}

//
// If the array's size is less than the requested size, nNewSize elements
// are added to the array until it reaches the requested size. If the
// array's size is larger than the requested size, the elements closest to
// the end of the array are deleted until the array reaches the requested size
// If the present size of the array is the same as the requested size, no
// action is taken.
//
template <class T, class TElementTraits>
HRESULT CArray<T, TElementTraits>::Resize(
    UINT nNewSize                   // Count to resize to
    )
{
    HRESULT hr = S_OK;

    if (nNewSize < 0)
    {
        hr = E_INVALIDARG;
    }

    if (nNewSize > _nSize)
    {
        // Add more elements to the end of the array
        hr = EnsureCapacity(nNewSize);
        if (SUCCEEDED(hr))
        {
            // Call the NULL constructor to init each element
            for (UINT i = _nSize; i < nNewSize; ++i)
            {
                new(_aT + i) T;
            }
            _nSize = nNewSize;
        }
    }
    else if (nNewSize < _nSize)
    {
        // Remove elements from the end of the array
        for (UINT i = nNewSize; i < _nSize; ++i)
        {
            _aT[i].~T();
        }
        _nSize = nNewSize;
    }
    return hr;
}
