// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef _COMMON_H_
#define _COMMON_H_

#pragma once

///////////////////////////////////////////////////////////////
//
//  error handling utilities
//
///////////////////////////////////////////////////////////////
#define ERROR_LABEL error

#define WIN32_FROM_HRESULT(hr)  \
    (SUCCEEDED(hr) ? ERROR_SUCCESS : \
        (HRESULT_FACILITY(hr) == FACILITY_WIN32 ? HRESULT_CODE(hr) : (hr)))

#define BAIL() \
    { \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_ERROR(r) \
    if( ERROR_SUCCESS != (r)) \
    { \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_SUCCESS(r) \
    if( ERROR_SUCCESS == (r)) \
    { \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_APP_ERROR(r, errcode) \
    { \
        (r) = errcode; \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_LAST_ERROR(r) \
    { \
        (r) = GetLastError(); \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_HRESULT_ERROR(r, hr) \
    { \
        (r) = WIN32_FROM_HRESULT(hr); \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_WIN32_ERROR(r, hr) \
    if( ERROR_SUCCESS != (r)) \
    { \
        (hr) = HRESULT_FROM_WIN32(r); \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_LAST_WIN32_ERROR(hr) \
    { \
        (hr) = HRESULT_FROM_WIN32(GetLastError()); \
        goto ERROR_LABEL; \
    }

#define BAIL_ON_FAILURE(hr) \
    if( S_OK != (hr)) \
    { \
        goto ERROR_LABEL; \
    }

// an object that supports ref/deref
class CRefObject
{
public:
    CRefObject() 
    : m_RefCount(1) 
    {}

    virtual ~CRefObject() {};

    ULONG AddRef() 
    {
        if ( 0 != m_RefCount )
        {
            return InterlockedIncrement((volatile LONG *)&m_RefCount);
        }
        return 0;
    }

    ULONG Release()
    {
        const LONG l = InterlockedDecrement((volatile LONG *)&m_RefCount);
        if (0 == l)
        {
            delete this;
        }

        return l;
    }

private:
    volatile ULONG m_RefCount;
};

// list of referenced objects
template < class T >
class CRefObjList : public CAtlList<T>
{

public:
    CRefObjList(UINT nBlockSize = 10)
    :CAtlList<T>(nBlockSize)
    {
    }

    ~CRefObjList()
    {
        RemoveAllEntries();
    }

    HRESULT RemoveAllEntries()
    {
        T cTemp;

        while ( 0 != GetCount() )
        {
            cTemp = RemoveTail();
            if ( NULL != cTemp )
            {
                cTemp->Release();
            }
        }

        return S_OK;
    }

    T GetElement(T pCheck)
    {
        
        for (size_t i = 0; i < GetCount(); i++)
        {
            T pObj = GetAt(FindIndex(i));
            if (*pObj == *pCheck)
            {
                return pObj;
            }
        }

        return NULL;
    }
    
    BOOL IsInArray(T pCheck)
    {
        return GetElement(pCheck) != NULL;
    }
};

#endif  // _COMMON_H_