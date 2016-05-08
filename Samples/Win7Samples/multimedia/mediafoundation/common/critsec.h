#pragma once

namespace MediaFoundationSamples
{


    //////////////////////////////////////////////////////////////////////////
    //  CritSec
    //  Description: Wraps a critical section.
    //////////////////////////////////////////////////////////////////////////

    class CritSec
    {
    private:
        CRITICAL_SECTION m_criticalSection;
    public:
        CritSec()
        {
            InitializeCriticalSection(&m_criticalSection);
        }

        ~CritSec()
        {
            DeleteCriticalSection(&m_criticalSection);
        }

        void Lock()
        {
            EnterCriticalSection(&m_criticalSection);
        }

        void Unlock()
        {
            LeaveCriticalSection(&m_criticalSection);
        }
    };


    //////////////////////////////////////////////////////////////////////////
    //  AutoLock
    //  Description: Provides automatic locking and unlocking of a 
    //               of a critical section.
    //
    //  Note: The AutoLock object must go out of scope before the CritSec.
    //////////////////////////////////////////////////////////////////////////

    class AutoLock
    {
    private:
        CritSec *m_pCriticalSection;
    public:
        AutoLock(CritSec& crit)
        {
            m_pCriticalSection = &crit;
            m_pCriticalSection->Lock();
        }
        ~AutoLock()
        {
            m_pCriticalSection->Unlock();
        }
    };

}; // namespace MediaFoundationSamples

