//<SnippetMusicBundleSig_hUtilWholePage>
/*****************************************************************************
*
* File: Util.h
*
* Description:
* Utility classes and function declarations for Packaging APIs.
*
* ------------------------------------
*
*  This file is part of the Microsoft Windows SDK Code Samples.
* 
*  Copyright (C) Microsoft Corporation.  All rights reserved.
* 
* This source code is intended only as a supplement to Microsoft
* Development Tools and/or on-line documentation.  See these other
* materials for detailed information regarding Microsoft code samples.
* 
* THIS CODE AND INFORMATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
* KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
* PARTICULAR PURPOSE.
* 
****************************************************************************/

#pragma once

// Array class to store and manage IOpcPart interface pointers.
class OpcPartArray
{
public:
    OpcPartArray() : m_container(NULL), m_count(0), m_bufferSize(0)
    {
    }

    ~OpcPartArray()
    {
        if (m_container)
        {
            for (UINT32 i=0; i<m_count; i++)
            {
                IOpcPart* part = m_container[i];
                
                if (part)
                {
                    part->Release();
                }
            }

            delete[] m_container;
        }
    }

    // Allocate and initialize internal buffer to store array elements.
    HRESULT
    Init(
        UINT32 numberOfElementsWantToStore
        )
    {
        HRESULT hr = S_OK;

        if (m_container)
        {
            hr = E_NOT_VALID_STATE;   // Already initialized.
        }

        if (SUCCEEDED(hr))
        {
            m_container = new(std::nothrow) IOpcPart*[numberOfElementsWantToStore];
        }

        if (SUCCEEDED(hr) && !m_container)
        {
            hr = E_OUTOFMEMORY;   // not enough memory.
        }

        if (SUCCEEDED(hr))
        {
            ZeroMemory(m_container, numberOfElementsWantToStore*sizeof(m_container[0]));
        
            m_bufferSize = numberOfElementsWantToStore;
        }

        return hr;
    }

    // Get number of elements in the array.
    UINT32
    GetCount() { return m_count; }

    HRESULT
    Append(
        IOpcPart* part
        )
    {
        HRESULT hr = S_OK;

        if (m_count >= m_bufferSize)
        {
            hr = E_NOT_SUFFICIENT_BUFFER;   // overflow
        }

        if (SUCCEEDED(hr))
        {
            m_container[m_count] = part;
            part->AddRef();
            m_count++;
        }

        return hr;
    }

    HRESULT
    GetAt(
        UINT32 index,
        IOpcPart** part
        )
    {
        HRESULT hr = S_OK;

        if (index >= m_count)
        {
            hr = E_INVALIDARG;    // out of range
        }

        if (SUCCEEDED(hr))
        {
            *part = m_container[index];

            (*part)->AddRef();
        }

        return hr;
    }

private:
    IOpcPart** m_container;
    UINT32 m_count;
    UINT32 m_bufferSize;
};

// Gets the target part of a relationship.
//
// Parameters:
//      partSet - the set of parts in the package
//      relationship    - the relationship of which we want to get the target part.
//      expectedContentType - the expected content type of the target part.
//      targetedPart - holds the target part. Method may return a valid pointer
//                     even on failure, and the caller must always release if a
//                     non-NULL value is returned.
HRESULT
GetRelationshipTargetPart(
    IOpcPartSet* partSet,
    IOpcRelationship* relationship,
    LPCWSTR expectedContentType,
    IOpcPart** targetedPart
    );

// Gets the target part of a relationship with a specific relationship type, out of the
// given set of relationships. It requires the given relationship set to have only one
// relationship with the given relationship type. Otherwise this function will report error.
//
// Parameters:
//      partSet - the set of parts in the package
//      relsSet - the relationship set from which we find a relationship with specific type.
//      relationshipType - the specific relationship type of the relationship we want to find.
//      expectedContentType - the expected content type of the target part.
//      targetedPart - holds the target part. Method may return a valid pointer
//                     even on failure, and the caller must always release if a
//                     non-NULL value is returned.
HRESULT
GetPartByRelationshipType(
    IOpcPartSet* partSet,
    IOpcRelationshipSet* relsSet,
    LPCWSTR relationshipType,
    LPCWSTR expectedContentType,
    IOpcPart** targetedPart
    );

// Reads a package from a file.
HRESULT
ReadPackageFromFile(
    LPCWSTR filename,
    IOpcFactory* opcFactory,
    IOpcPackage** opcPackage
    );

// Writes a package to a file.
HRESULT
WritePackageToFile(
    LPCWSTR filename,
    IOpcPackage* opcPackage,
    IOpcFactory* opcFactory
    );

// Saves a certificate to a file.
HRESULT
SaveCertificateToFile(
    PCCERT_CONTEXT cert,
    LPCWSTR filename
    );
//</SnippetMusicBundleSig_hUtilWholePage>