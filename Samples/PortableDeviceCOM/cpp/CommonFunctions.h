// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#pragma once

// Implemented in ContentProperties.cpp
void DisplayStringProperty(
    _In_ IPortableDeviceValues*  properties,
    _In_ REFPROPERTYKEY          key,
    _In_ PCWSTR                  keyName);

// Implemented in ContentProperties.cpp
void DisplayGuidProperty(
    _In_ IPortableDeviceValues*  properties,
    _In_ REFPROPERTYKEY          key,
    _In_ PCWSTR                  keyName);

// Determines if a device supports a particular functional category.
BOOL SupportsCommand(
    _In_ IPortableDevice*  device,
    _In_ REFPROPERTYKEY    command);

// Helper class to convert a GUID to a string
class CGuidToString
{
private:
    WCHAR m_stringGUID[64];

public:
    CGuidToString(_In_ REFGUID guid)
    {
        if (!::StringFromGUID2(guid, m_stringGUID, ARRAYSIZE(m_stringGUID)))
        {
            m_stringGUID[0] = L'\0';
        }
    }

    operator PCWSTR()
    {
        return m_stringGUID;
    }
};