// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

class SmartFileHandle
{
public:
    SmartFileHandle()
            : m_Handle ( INVALID_HANDLE_VALUE )
    {}

    ~SmartFileHandle()
    {
        Close();
    }

    void Set ( HANDLE h )
    {
        RDCAssert ( !IsValid() );
        m_Handle = h;
    }

    void Close()
    {
        if ( IsValid() )
        {
            CloseHandle ( m_Handle );
        }
        m_Handle = INVALID_HANDLE_VALUE;
    }

    bool IsValid() const
    {
        return m_Handle != 0 && m_Handle != INVALID_HANDLE_VALUE;
    }

    HANDLE GetHandle() const
    {
        return m_Handle;
    }
private:
    HANDLE m_Handle;
};
