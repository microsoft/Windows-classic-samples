//// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//// PARTICULAR PURPOSE.
////
//// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once
class CSlimRWLock
{
    SRWLOCK h;
    CSlimRWLock(CSlimRWLock const &);
    CSlimRWLock const & operator=(CSlimRWLock const &);

public:
    CSlimRWLock(void);
    ~CSlimRWLock(void);

	_Acquires_lock_(h) void Enter();
	_Acquires_lock_(h) bool Try_Enter();
	_Requires_lock_held_(h) void Exit();
};

