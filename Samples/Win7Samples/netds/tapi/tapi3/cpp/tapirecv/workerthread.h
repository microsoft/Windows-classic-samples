/*

Copyright (c) 1999 - 2000  Microsoft Corporation


Module Name:

    WorkerThread.h

Abstract:

    interface for the CWorkerThread class.

*/


#if !defined(AFX_WORKERTHREAD_H__45467AAB_C5AB_43D6_B518_B81B971A859C__INCLUDED_)
#define AFX_WORKERTHREAD_H__45467AAB_C5AB_43D6_B518_B81B971A859C__INCLUDED_


//
// worker whose purpose is to asynchronously process messages received 
// from TAPI
// 

class CWorkerThread
{

public:

    //
    // post an message to be processed asyncronously
    //

    BOOL PostMessage(UINT Msg,       // message to post
                     WPARAM wParam,  // first message parameter
                     LPARAM lParam   // second message parameter
                     );

public:

    //
    // create thread
    //

    HRESULT Initialize();

    //
    // shutdown thread
    //

    HRESULT Shutdown();



    CWorkerThread()
        :m_hThreadHandle(NULL),
        m_nThreadID(0)
    {}


    virtual ~CWorkerThread();

protected:
    
    //
    // the actual thread function
    //

    static unsigned long _stdcall ThreadFunction(void *);


private:

    //
    // thread handle and thread id
    //

    HANDLE m_hThreadHandle;

    unsigned long m_nThreadID;

};

#endif // !defined(AFX_WORKERTHREAD_H__45467AAB_C5AB_43D6_B518_B81B971A859C__INCLUDED_)
