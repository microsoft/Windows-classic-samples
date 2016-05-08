//-----------------------------------------------------------------------------
// File: GrowArray.h
// Desc: Growable array class.
//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
//  Copyright (C) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------


#pragma once


namespace MediaFoundationSamples
{

    // Class template: Re-sizable array. 

    // To grow or shrink the array, call SetSize(). 
    // To pre-allocate the array, call Allocate(). 

    // Notes:
    // Copy constructor and assignment operator are private, to avoid throwing exceptions. (One could easily modify this.)
    // It is the caller's responsibility to release the objects in the array. The array's destuctor does not release them.
    // The array does not actually shrink when SetSize is called with a smaller size. Only the reported size changes.

    template <class T>
    class GrowableArray
    {
    public:
        GrowableArray() : m_count(0), m_allocated(0), m_pArray(NULL)
        {
            
        }
        virtual ~GrowableArray()
        {
            SAFE_ARRAY_DELETE(m_pArray);
        }

        // Allocate: Reserves memory for the array, but does not increase the count.
        HRESULT Allocate(DWORD alloc)
        {
            HRESULT hr = S_OK;
            if (alloc > m_allocated)
            {
                T *pTmp = new T[alloc];
                if (pTmp)
                {
                    ZeroMemory(pTmp, alloc * sizeof(T));

                    assert(m_count <= m_allocated);

                    // Copy the elements to the re-allocated array.
                    for (DWORD i = 0; i < m_count; i++)
                    {
                        pTmp[i] = m_pArray[i];
                    }

                    delete [] m_pArray;

                    m_pArray = pTmp;
                    m_allocated = alloc;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }
            return hr;
        }

        // SetSize: Changes the count, and grows the array if needed.
        HRESULT SetSize(DWORD count)
        {
            assert (m_count <= m_allocated);

            HRESULT hr = S_OK;
            if (count > m_allocated)
            {
                hr = Allocate(count);
            }
            if (SUCCEEDED(hr))
            {
                m_count = count;
            }
            return hr;
        }
        
        DWORD GetCount() const { return m_count; }

        // Accessor.
        T& operator[](DWORD index)
        {
            assert(index < m_count);
            return m_pArray[index];
        }

        // Const accessor.
        const T& operator[](DWORD index) const
        {
            assert(index < m_count);
            return m_pArray[index];
        }

        // Return the underlying array.
        T* Ptr() { return m_pArray; }

    protected:
        GrowableArray& operator=(const GrowableArray& r);
        GrowableArray(const GrowableArray &r);

        T       *m_pArray;
        DWORD   m_count;        // Nominal count.
        DWORD   m_allocated;    // Actual allocation size.
    };

};  // namespace MediaFoundationSamples


