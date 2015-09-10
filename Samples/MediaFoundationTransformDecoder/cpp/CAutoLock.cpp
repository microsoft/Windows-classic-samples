#include "CAutoLock.h"

_When_(pcs != NULL, _Acquires_lock_(*this->m_pcs) _Post_same_lock_(*pcs, *this->m_pcs)) _Post_satisfies_(this->m_pcs == pcs)
CAutoLock::CAutoLock(CRITICAL_SECTION* pcs)
{
    m_pcs = pcs;

    if(m_pcs != NULL)
    {
        EnterCriticalSection(m_pcs);
    }
}

_When_(this->m_pcs != NULL, _Releases_lock_(*this->m_pcs))
CAutoLock::~CAutoLock(void)
{
    if(m_pcs != NULL)
    {
        LeaveCriticalSection(m_pcs);

        m_pcs = NULL;
    }
}