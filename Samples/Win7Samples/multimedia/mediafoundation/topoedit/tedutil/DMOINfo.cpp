// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

#include "DMOInfo.h"

#include <dmo.h>
#include <stdio.h>
#include <atlconv.h>

DMOInfo* DMOInfo::GetSingleton() 
{
    static DMOInfo singleton;

    return &singleton;
}

CAtlStringW DMOInfo::GetCLSIDName(const CLSID& clsid) 
{
    for(size_t i = 0; i < m_clsids.GetCount(); i++) 
    {
        if(m_clsids.GetAt(i) == clsid) 
        {
            return m_names.GetAt(i);
        }
    }

    return L"";
}


CLSID DMOInfo::GetCLSIDFromName(const CAtlStringW& strName) 
{
    for(size_t i = 0; i < m_names.GetCount(); i++) 
    {
        if(m_names.GetAt(i) == strName) 
        {
            return m_clsids.GetAt(i);
        }
    }

    return GUID_NULL;
}


// Protected constructor for singleton creation
DMOInfo::DMOInfo() 
{
    USES_CONVERSION;
    
    DMO_PARTIAL_MEDIATYPE mediaType;
    CComPtr<IEnumDMO> spDMOList;

    mediaType.type = GUID_NULL;
    mediaType.subtype = GUID_NULL;
    
    ::DMOEnum(GUID_NULL, DMO_ENUMF_INCLUDE_KEYED, 0, &mediaType, 0, &mediaType, &spDMOList);

    bool bContinue = true;
    while(bContinue) {
        CLSID clsids[5];
        WCHAR* pNames[5];
        DWORD dwNumItems;

        spDMOList->Next(5, clsids, pNames, &dwNumItems);

        for(unsigned int i = 0; i < dwNumItems; i++) {
            m_clsids.Add(clsids[i]);
            m_names.Add(pNames[i]);
        }

        if(5 != dwNumItems) {
            bContinue = false;
        }
    }
}
