#pragma once

class DebugHresult
{
public:
    DebugHresult()
            : m_HR ( S_OK )
    {}
    DebugHresult ( HRESULT hr )
            : m_HR ( hr )
    {
        RDCAssert ( hr == S_OK );
    }

    operator HRESULT() const
    {
        return m_HR;
    }
    HRESULT operator= ( HRESULT hr )
    {
        RDCAssert ( hr == S_OK );
        m_HR = hr;
        return m_HR;
    }
private:
    HRESULT m_HR;
};

