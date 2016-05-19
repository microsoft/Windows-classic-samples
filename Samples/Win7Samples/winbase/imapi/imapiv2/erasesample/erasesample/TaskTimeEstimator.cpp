/*--

Copyright (C) Microsoft Corporation, 2006

Implementation of CTaskTimeEstimator

--*/

#include "stdafx.h"

CTaskTimeEstimator::CTaskTimeEstimator( const ULONG estimatedTotalNumberOfMilliseconds, const ULONG numberOfEqualSteps) throw() :
    m_OriginalTotalMilliseconds(estimatedTotalNumberOfMilliseconds),
    m_ExpectedTotalMilliseconds(estimatedTotalNumberOfMilliseconds),
    m_ElapsedMilliseconds(0),
    m_StartTimeTickCount(0),
    m_TimerStarted(FALSE),
    m_TimerEnded(FALSE),
    m_TotalSteps(numberOfEqualSteps ? numberOfEqualSteps : 1), // avoid divide-by-zero
    m_CompletedSteps(0),
    m_PreviousCompletedSteps(0),
    m_NonZeroCompletedStepsCount(0),
    m_FirstNonZeroElapsedMilliseconds(0),
    m_FirstNonZeroCompletedSteps(0)
{
    for (ULONG i = 0; i < RTL_NUMBER_OF(m_NewEstimatesHistory); i++)
    {
        m_NewEstimatesHistory[i] = m_OriginalTotalMilliseconds;
    }
    TestInvariants();
}

CTaskTimeEstimator::CTaskTimeEstimator(const CTaskTimeEstimator& x) throw()     :
    m_OriginalTotalMilliseconds(x.m_OriginalTotalMilliseconds),
    m_ExpectedTotalMilliseconds(x.m_ExpectedTotalMilliseconds),
    m_ElapsedMilliseconds(x.m_ElapsedMilliseconds),
    m_StartTimeTickCount(x.m_StartTimeTickCount),
    m_TimerStarted(x.m_TimerStarted),
    m_TimerEnded(x.m_TimerEnded),
    m_TotalSteps(x.m_TotalSteps),
    m_CompletedSteps(x.m_CompletedSteps),
    m_PreviousCompletedSteps(x.m_PreviousCompletedSteps),
    m_NonZeroCompletedStepsCount(x.m_NonZeroCompletedStepsCount),
    m_FirstNonZeroElapsedMilliseconds(x.m_FirstNonZeroElapsedMilliseconds),
    m_FirstNonZeroCompletedSteps(x.m_FirstNonZeroCompletedSteps)
{
    RtlCopyMemory(m_NewEstimatesHistory, x.m_NewEstimatesHistory, sizeof(m_NewEstimatesHistory[0]) * RTL_NUMBER_OF(m_NewEstimatesHistory));
    TestInvariants();
}

CTaskTimeEstimator::~CTaskTimeEstimator() throw()
{
    TestInvariants();
}

VOID CTaskTimeEstimator::StartNow()
{
    TestInvariants();
    assert(!m_TimerStarted);
    if (!m_TimerStarted)
    {
        m_StartTimeTickCount = GetTickCount();
        m_TimerStarted = TRUE;
    }
    return;
}

VOID CTaskTimeEstimator::EndNow()
{
    TestInvariants();
    assert( m_TimerStarted);
    if (m_TimerStarted && (!m_TimerEnded))
    {
        put_CompletedSteps(m_TotalSteps);
    }
    return;
}

ULONG CTaskTimeEstimator::get_OriginalTotalMilliseconds() const throw()
{
    TestInvariants();
    return m_OriginalTotalMilliseconds;
}

ULONG CTaskTimeEstimator::get_TotalSteps() const throw()
{
    TestInvariants();
    return m_TotalSteps;
}

ULONG CTaskTimeEstimator::get_TotalMilliseconds() const throw()
{     
    TestInvariants();
    return m_ExpectedTotalMilliseconds;
}

ULONG CTaskTimeEstimator::get_RemainingMilliseconds() const throw()
{
    TestInvariants();
    return m_ExpectedTotalMilliseconds - m_ElapsedMilliseconds;
}

ULONG CTaskTimeEstimator::get_ElapsedMilliseconds() const throw()
{
    TestInvariants();
    return m_ElapsedMilliseconds;
}

VOID CTaskTimeEstimator::UpdateTime()
{
    TestInvariants();
    // Code flow:
    // 1) Exit unless started and not finished
    // 2) Determine elapsed tick count (in milliseconds)
    // 3) If finished with the task, just set estimate to elapsed
    // 4) Avoid case where completed == first non-zero (division by zero)
    // 5) Improve estimates for drives which report progress
    //    in very large "steps" as opposed to nice linear progression
    // 6) Else if the CompletedSteps property was set to a non-zero
    //    value more than once, update estimated time
    //    NOTE: updating only for 2nd and later non-zero prevents
    //          modifying the time estimate infinitely large, and
    //          also smooths the starting time estimates.
    // 7) If elapsed time is greater than estimate,
    //    increase estimate to elapsed.
    ;
    // 1) Exit unless started and not finished
    if (!m_TimerStarted || m_TimerEnded)
    {
        //assert(m_TimerStarted && !m_TimerEnded);
        return;
    }

    // 2) Determine elapsed tick count
    m_ElapsedMilliseconds = GetTickCount() - m_StartTimeTickCount;

    // 3) If finished with the task, just set estimate to elapsed
    if (m_CompletedSteps == m_TotalSteps)
    {
        m_ExpectedTotalMilliseconds = m_ElapsedMilliseconds;
    }
    // 4) Avoid division by zero in estimate
    else if (m_CompletedSteps == m_FirstNonZeroCompletedSteps)
    {
        // do not update the time estimate
    }
    // 5) If the current value is equal to the last value we saw,
    //    then don't update the estimate.  This is to prevent really
    //    unusual behavior for drives which report progress in very
    //    large "steps".  Also works around drives which just always
    //    report 100% completed (i.e. "LITE-ON " / "DVDRW LDW-811S" / "HS06")
    else if (m_CompletedSteps == m_PreviousCompletedSteps)
    {
        // do not update the time estimate
    }
    // 6) Else if the CompletedSteps property was set to a non-zero
    //    value more than once, update estimated time
    else if (m_NonZeroCompletedStepsCount > 2)
    {
        // New estimate is slightly more complex than before, but
        //    can use the old formula, just with "new" numbers:
        // New elapsed time is elapsed time - time of first non-zero
        // New total steps is total steps - steps at first non-zero
        // New completed steps is total steps - steps at first non-zero

        // new estimate is:
        // elapsed * (totalSteps / completed)
        // Avoiding overflow, use 64-bit temp variable
        ULONGLONG tmp = m_ElapsedMilliseconds - m_FirstNonZeroElapsedMilliseconds;

        // a ULONG * ULONG is _never_ more than a ULONGLONG,
        // so this is guaranteed to be safe (no overflow)
        tmp *= m_TotalSteps - m_FirstNonZeroCompletedSteps;
        tmp /= m_CompletedSteps - m_FirstNonZeroCompletedSteps;

        // and add back into the equation the ticks from the first non-zero
        tmp += m_FirstNonZeroElapsedMilliseconds;

        // check for overflow before storing back in ULONG
        if (tmp > 0xFFFFFFFF)
        {
            tmp = 0xFFFFFFFF;
        }

        // So, should we actually change the estimated time?
        // Being 100% exact is ok, but being smooth is more important.
        // So, use a "sliding window", with a history of four measurements,
        // each of equal importance.  No need to worry about guessing less
        // than elapsed, as that's handled in the next step.  Also, unused
        // estimates are filled with original estimate, so can just save
        // this new value into the array and run an estimate.
        ULONG index = m_NonZeroCompletedStepsCount % TimeKeeperHistoryDepth;
        m_NewEstimatesHistory[index] = (ULONG)tmp;

        // Algorithm: add the four values, divide by count.
        tmp = 0;
        for (ULONG i = 0; i < TimeKeeperHistoryDepth; i++)
        {
            tmp += m_NewEstimatesHistory[ i ];
        }

        // Give new value 2x weight.  With history length of 4, this
        // means 40% of the new value + 60% average over the last three
        //
        // This results in the following table:
        //  (0 == good, 100 == bad)       (0 == old, 100 == new good value)
        //    Effect of 1 bad value       Effect of 4 good values (w/change)
        // pass 1:       40                       40
        // pass 2:        8                       48
        // pass 3:       10                       58
        // pass 4:       12                       69
        // pass 5:        6                       75
        // pass 6:        5                       80
        // pass 7:        5                       85
        // pass 8:        3                       88
        // pass 9:        3                       91
        //

        // Give new value 3x weight.  With history length of 4, this
        // means 50% of the new value + 50% average over the last three
        //
        // This results in the following table:
        //  (0 == good, 100 == bad)       (0 == old, 100 == new good value)
        //    Effect of 1 bad value       Effect of 4 good values (w/change)
        // pass 1:       50                       50 
        // pass 2:        8                       58 
        // pass 3:       10                       68 
        // pass 4:       11                       78 
        // pass 5:        5                       79 
        // pass 6:        4                       84 
        // pass 7:        3                       89 
        // pass 8:        2                       92 
        // pass 9:        2                       94 
        //                                        

        // I also created a spreadsheet up to 6x.  It looks like a 2x weight
        // to the new number is sufficient to dampen oscillations when converging
        // upon a new target (the second column above), while minimizing the
        // disruptive effect of a single extremely incorrect value.

        // So, just add the new value one more time, and divide by HistoryDepth+1
        tmp += m_NewEstimatesHistory[index];
        tmp /= TimeKeeperHistoryDepth + 1;

        // Replace the "raw" value above with the new weighted value
        m_NewEstimatesHistory[index] = (ULONG)tmp;
        m_ExpectedTotalMilliseconds  = (ULONG)tmp;
    }

    // 7) If elapsed time is greater than estimate,
    //    increase estimate to elapsed.
    if (m_ElapsedMilliseconds >= m_ExpectedTotalMilliseconds)
    {
        m_ExpectedTotalMilliseconds = m_ElapsedMilliseconds;
    }

    return;
}

ULONG CTaskTimeEstimator::get_CompletedSteps() const throw()
{
    TestInvariants();

    return m_CompletedSteps;
}

VOID CTaskTimeEstimator::put_CompletedSteps(const ULONG completed)
{
    TestInvariants();

    ULONG result;
    if (completed >= m_TotalSteps)
    {
        result = m_TotalSteps;
    }
    else
    {
        result = completed;
    }
    assert(m_TimerStarted && !m_TimerEnded);
    m_PreviousCompletedSteps = m_CompletedSteps;
    m_CompletedSteps = result;

    if (m_CompletedSteps != 0)
    {
        if (m_NonZeroCompletedStepsCount == 0)
        {
            // save the tick count and value for the first non-zero
            // completion state -- this makes time estimates more reliable
            m_FirstNonZeroElapsedMilliseconds = GetTickCount() - m_StartTimeTickCount;
            m_FirstNonZeroCompletedSteps = m_CompletedSteps;
        }
        m_NonZeroCompletedStepsCount++;
    }
    UpdateTime();

    if (m_CompletedSteps == m_TotalSteps)
    {
        m_TimerEnded = TRUE;
    }

    return;
}

VOID  CTaskTimeEstimator::TestInvariants() const throw()
{
#if ( defined(DBG) || defined(DEBUG) )
    // avoid divide-by-zero
    assert(m_TotalSteps != 0);
    // avoid greater-than-100% completed
    assert(m_CompletedSteps <= m_TotalSteps);
    // avoid greater-than-100% completed for previous also
    assert(m_PreviousCompletedSteps <= m_TotalSteps);
    // can only end timer after its started
    assert((!m_TimerEnded) || (m_TimerStarted));
    // elapsed time always less than or equal to expected time
    assert(m_ElapsedMilliseconds <= m_ExpectedTotalMilliseconds);
#endif
}

