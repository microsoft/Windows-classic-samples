// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// FUTURE: Unused currently
#pragma once

class RdcThread
{
public:
    RdcThread();
    ~RdcThread();

    template<class JobTypeT, class UserDataT>
    bool AddJob ( JobTypeT &job, UserDataT *userData )
    {
        return AddJobImpl (
                   RunJobImpl<JobTypeT, UserDataT>,
                   static_cast<void *> ( &job ),
                   static_cast<void *> ( userData ) );
    }

private:
    typedef void ( *RunFnT ) ( void *jobData, void *userData );
    struct JobInfo
    {
        void ( *runFn ) ( void *jobData, void *userData );
        void *jobData;
        void *userData;
    };

    std::vector<JobInfo> m_PendingJobs;

    bool AddJobImpl (
        RunFnT runFn,   // A pointer to a RunJobImpl()
        void *jobData,
        void *userData );

    template<class JobTypeT, class UserDataT>
    static void RunJobImpl ( void *voidJob, void *voidUserData )
    {
        // Cast (void*) back to the correct data type
        // and dispatch
        JobTypeT * job = reinterpret_cast<JobTypeT *>voidJob;
        UserDataT *userData = reinterpret_cast<UserDataT *>voidUserData;

        job->Run ( userData );
    }
};



