#include "SparkleFinisher.h"

#include <iostream>
#include <sstream>

#include <wil\com.h>
#include <wil\resource.h>
#include <wil\result.h>

namespace
{
    HANDLE duplicateHandle(HANDLE const in)
    {
        HANDLE out;

        THROW_IF_WIN32_BOOL_FALSE(::DuplicateHandle(::GetCurrentProcess(),
            in,
            ::GetCurrentProcess(),
            &out,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS));

        return out;
    }
}

// This class represents the work we are doing on behalf of the client.
class SparkleWork final
{
public:
    SparkleWork(HANDLE file, HANDLE startEvent) :
        m_file(duplicateHandle(file)), m_startEvent(duplicateHandle(startEvent))
    {
    }

    ~SparkleWork()
    {
        // Let the client know that the work is done, or we tried and failed to do the work.
        m_finishedEvent.SetEvent();
    }

    HANDLE DuplicateFinishedEventHandle()
    {
        return duplicateHandle(m_finishedEvent.get());
    }

    void ScheduleWork();

private:
    static VOID CALLBACK s_WaitCallback(
        _Inout_     PTP_CALLBACK_INSTANCE /*Instance*/,
        _Inout_opt_ PVOID                 Context,
        _Inout_     PTP_WAIT              /*Wait*/,
        _In_        TP_WAIT_RESULT        WaitResult);

private:
    wil::unique_hfile m_file;
    wil::unique_event m_startEvent;

    // Auto reset, initially unset, no name, default security, throws on fail.
    wil::unique_event m_finishedEvent{ wil::EventOptions::None };

    // Use the no-wait version of the WIL threadpool wait wrapper. The plain
    // unique_threadpool_wait waits for callbacks to drain, but that would
    // result in a deadlock because we destruct from within the callback.
    wil::unique_threadpool_wait_nowait m_threadpoolWait;
};

void SparkleWork::ScheduleWork()
{
    // Create a threadpool wait that will continue the work.
    m_threadpoolWait.reset(CreateThreadpoolWait(s_WaitCallback, this, nullptr));
    THROW_LAST_ERROR_IF(!m_threadpoolWait);

    // Schedule the wait to run when the m_startEvent is signaled.
    SetThreadpoolWait(m_threadpoolWait.get(), m_startEvent.get(), nullptr); // never fails
}

void CALLBACK SparkleWork::s_WaitCallback(
    _Inout_     PTP_CALLBACK_INSTANCE /*Instance*/,
    _Inout_opt_ PVOID                 Context,
    _Inout_     PTP_WAIT              /*Wait*/,
    _In_        TP_WAIT_RESULT        WaitResult
)
try
{
    // Re-attach work pointer to a unique_ptr to ensure it is cleaned up
    // even if an error occurs.
    auto work = std::unique_ptr<SparkleWork>(static_cast<SparkleWork*>(Context));

    // If the wait ended for any reason except being signaled properly by the caller,
    // we need to abort.
    THROW_HR_IF(E_ABORT, WAIT_OBJECT_0 != WaitResult);

    // Build up our message.
    std::stringstream message;
    message << "Dispensing sparkle finisher." << std::endl;
    message << "Rinsing." << std::endl;
    message << "Drying." << std::endl;
    message << "Sparkly!" << std::endl;

    // Finalize into string.
    auto completeMessage = message.str();

    // Write out to file.
    auto bytesToWrite = completeMessage.size() * sizeof(completeMessage[0]);
    DWORD bytesWritten = 0;
    THROW_IF_WIN32_BOOL_FALSE(WriteFile(work->m_file.get(),
        completeMessage.c_str(),
        static_cast<DWORD>(bytesToWrite),
        &bytesWritten,
        nullptr));

    // On scope exit, the unique_ptr will free the SparkleWork
    // which will signal the m_finishedEvent (to tell the client that
    // we are done) and then clean up the handles.
}
// Since this is an anonymous work callback, capture and log errors as there's
// no one really listening. (A fancier version of this COM method might use
// some mechanism to report an error to the client, but that is outside the
// scope of this sample.)
CATCH_LOG()

HRESULT SparkleFinisher::AddSparkleFinishToFile(HANDLE file,
    HANDLE startEvent,
    HANDLE* finishedEvent) noexcept
try
{
    // Create a SparkleWork to record the work we are doing for this client.
    // The parameters are valid only for the lifetime of this call, so
    // the SparkleWork constructor duplicates the handles so that we can
    // continue to use them after we return to the caller.
    auto work = std::make_unique<SparkleWork>(file, startEvent);

    // As with all output parameters, COM takes ownership of the handle we return.
    // Since we want to retain access to the handle for ourselves, give the caller a duplicate.
    *finishedEvent = work->DuplicateFinishedEventHandle();

    // Schedule the work to continue on the threadpool when the start event
    // is signaled.
    work->ScheduleWork();

    // The threadpool work item owns the work now.
    work.release();

    return S_OK;
}
CATCH_RETURN()
