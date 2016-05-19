//+---------------------------------------------------------------------------
//
//  Copyright (c) Microsoft Corporation. All rights reserved. 
//
//
//  BITS Upload sample
//  ==================
//
//  Module name: 
//  util.h
//
//  Purpose:
//  Forward prototypes for functions defined on util.cpp and
//  declaration/implementation of CSmartComPtr.
//
//----------------------------------------------------------------------------

#pragma once

#define MAX_BUFFER_SIZE             4096
#define MAX_GUID_STRING_LENGTH      40


// ----------------------------------------------------------------------------------
// Prototypes for functions defined in util.cpp
// ----------------------------------------------------------------------------------

HRESULT DisplayErrorMessage(LPCWSTR pwszAdditionalMsg, HRESULT hrCode);
LPCWSTR ConvertGuidToString(REFGUID guid);


// ----------------------------------------------------------------------------------
// Parametrized utility classes
// ----------------------------------------------------------------------------------

//
// CSmartComPtr class
// For those used to ATL, this class is very similar to CComPtr
//
template <class T> class CSmartComPtr
{
    T *m_pI;

    void AddReference()
    {
        if (m_pI)
        {
            m_pI->AddRef();
        }
    }

    void ReleaseInterface()
    {
        if (m_pI)
        {
            m_pI->Release();
            m_pI = NULL;
        }
    }

public:

    CSmartComPtr()
    {
        m_pI = NULL;
    }

    CSmartComPtr(T* pInterface)
    {
        m_pI = pInterface;
        AddReference();
    }

    CSmartComPtr(const CSmartComPtr<T>& lpAnother)
    {
        m_pI = lpAnother.m_pI;
        AddReference();
    }

    ~CSmartComPtr()
    {
        ReleaseInterface();
    }

    T* Release()
    {
        T* pTemp = m_pI;
        m_pI = NULL;
        return temp;
    }

    void Clear()
    {
        ReleaseInterface();
    }

    operator const T*() const
    {
        return m_pI;
    }

    // shouldn't be used for in/out parameters if interface pointer is
    // already initialized
    T** operator&()
    {
        _ASSERT(m_pI == NULL);
        return &m_pI;
    }

    T* operator->() const
    {
        return m_pI;
    }

    CSmartComPtr& operator=(CSmartComPtr &Other )
    {
        ReleaseInterface();
        m_pI = Other.m_pI;
        AddReference();

        return *this;
    }

    T** GrabOutPtr()
    {
        ReleaseInterface();
        return &m_pI;
    }
};

