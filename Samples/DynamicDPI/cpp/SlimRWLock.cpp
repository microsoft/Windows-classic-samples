//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#include "pch.h"
#include "SlimRWLock.h"


CSlimRWLock::CSlimRWLock(void)
{
    InitializeSRWLock(&h);
}

CSlimRWLock::~CSlimRWLock(void)
{
}

_Acquires_lock_(h)
void CSlimRWLock::Enter()
{
    AcquireSRWLockExclusive(&h);
}

_Acquires_lock_(h)
bool CSlimRWLock::Try_Enter()
{
    return 0 != TryAcquireSRWLockExclusive(&h);
}

_Requires_lock_held_(h)
void CSlimRWLock::Exit()
{
    ReleaseSRWLockExclusive(&h);
}
