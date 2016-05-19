// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// FUTURE: Unused currently
#include "stdafx.h"
#include "rdcThread.h"
#include "globals.h"

struct JobInfo
{
    void ( *runFn ) ( void *jobData, void *userData );
    void *jobData;
    void *userData;
};

CComAutoCriticalSection jobCS;
RdcSmartArray<JobInfo> jobs;

RdcThread::RdcThread()
{}

RdcThread::~RdcThread()
{}

bool RdcThread::AddJobImpl (
    RunFnT runFn,
    void *jobData,
    void *userData )
{
    CComCritSecLock<CComCriticalSection> lock ( jobCS );

    JobInfo jobInfo =
        {
            runFn,
            jobData,
            userData
        };

    if ( !jobs.Append ( jobInfo ) )
    {
        return false;
    }

    return true;
}

