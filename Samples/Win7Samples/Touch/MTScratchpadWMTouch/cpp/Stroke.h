// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

// Stroke.h
//
// Definition of lightweight array template class.
// Definition of helper classes for stroke storage, CStroke and
// CStrokeCollection.

#pragma once

// C RunTime header files
#include <stdlib.h>
#include <assert.h>
#define ASSERT assert

///////////////////////////////////////////////////////////////////////////////
// TArray - lightweight array template class.
// Arguments: T = element type, AG = allocation granularity
// This template is intended to be used only with small C structs and pointers.
// For simplicity, a lot of standard array functionality is omitted.
template <class T, int AG = 32>
class TArray
{
public:
    TArray();
    ~TArray();

    // Access to the elements.
    int Count() const { return m_nCount; }
    T& operator [] (int i)
    {
        ASSERT((i >= 0) && (i <m_nCount));
        return m_arrData[i];
    }

    // Adds the element to the array.
    bool Add(const T& t);
    // Removes the element from the array.
    void Remove(int i);

protected:
    T* m_arrData;
    int m_nCount;
};

// Array constructor.
template <class T, int AG>
TArray<T,AG>::TArray()
:   m_arrData(NULL),
    m_nCount(0)
{
}

// Array destructor.
template <class T, int AG>
TArray<T,AG>::~TArray()
{
    if (m_arrData != NULL)
    {
        free(m_arrData);
    }
}

// Adds the element to the array.
// in:
//      t      new element
// returns:
//      success status
template <class T, int AG>
bool TArray<T,AG>::Add(const T& t)
{
    // If there are no more unused elements in the array, allocate one more chunk of size AG.
    if (m_nCount % AG == 0)
    {
        T* arrDataNew = (T*) realloc(m_arrData, (m_nCount + AG)*sizeof(T));
        if (arrDataNew == NULL)
        {
            if (m_arrData != NULL)
            {
                free(m_arrData);
                m_arrData = NULL;
            }
            m_nCount = 0;

            return false;
        }

        m_arrData = arrDataNew;
    }

    // Add the element to the array.
    m_arrData[m_nCount++] = t;

    return true;
}

// Removes the element from the array.
// in:
//      i       element index
// returns:
//      success status
template <class T, int AG>
void TArray<T,AG>::Remove(int i)
{
    ASSERT((i >= 0) && (i < m_nCount));

    // If the element i is not the last element in the array,
    // shift all the elements beyond i for one place backwards.
    if (m_nCount - i - 1 > 0)
    {
        // Move the memory one element backwards.
        memmove(m_arrData + i, m_arrData + i + 1, (m_nCount - i - 1) * sizeof(T));
    }

    // Decrement element count.
    --m_nCount;
}

///////////////////////////////////////////////////////////////////////////////
// CStroke object represents a single stroke, trajectory of the finger from
// touch-down to touch-up. Object has two properties: color of the stroke,
// and ID used to distinguish strokes coming from different fingers.
// Stroke points are contained in the base class.
class CStroke : public TArray<POINT>
{
public:
    CStroke();

    // Property: stroke color
    void SetColor(COLORREF clr) { m_clr = clr; }
    COLORREF GetColor() const { return m_clr; }

    // Property: stroke ID
    void SetId(int id) { m_id = id; }
    int GetId() const { return m_id; }

    // Draw the complete stroke.
    void Draw(HDC hDC) const;
    // Draw only last segment of the stroke.
    void DrawLast(HDC hDC) const;

private:
    COLORREF m_clr;     // Stroke color
    int m_id;           // Stroke ID
};

///////////////////////////////////////////////////////////////////////////////
// CStrokeCollection object represents a collection of the strokes.
// It supports add/remove stroke operations and finding a stroke by ID.
// Strokes are stored by-reference, meaning that collection does not own
// the strokes and that it is not responsible for their life time.
class CStrokeCollection : public TArray<CStroke*>
{
public:
    // Search the collection for given ID.
    int FindStrokeById(int id) const;

    // Draw the collection of the strokes.
    void Draw(HDC hDC) const;
};
