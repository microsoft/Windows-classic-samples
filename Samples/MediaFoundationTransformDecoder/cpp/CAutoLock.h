#pragma once

#include <windows.h>

class CAutoLock
{
public:
    _When_(pcs != NULL, _Acquires_lock_(*this->m_pcs) _Post_same_lock_(*pcs, *this->m_pcs)) _Post_satisfies_(this->m_pcs == pcs)
    CAutoLock(CRITICAL_SECTION* pcs);
    _When_(this->m_pcs != NULL, _Releases_lock_(*this->m_pcs))
    ~CAutoLock(void);

protected:
    CRITICAL_SECTION*   m_pcs;
};