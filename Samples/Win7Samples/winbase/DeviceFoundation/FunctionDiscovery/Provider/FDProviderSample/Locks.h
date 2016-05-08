// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//  Abstract:
//
//      This module define TLock, it wraps Windows SRWLOCK

#pragma once

class TLock
{
public:
    TLock() { InitializeSRWLock(&m_Lock); }
    inline void AcquireExclusive() { AcquireSRWLockExclusive(&m_Lock); }
    inline void ReleaseExclusive() { ReleaseSRWLockExclusive(&m_Lock); }
    inline void AcquireShared() { AcquireSRWLockShared(&m_Lock); }
    inline void ReleaseShared() { ReleaseSRWLockShared(&m_Lock); }

protected:
    SRWLOCK m_Lock;
};  // TLock