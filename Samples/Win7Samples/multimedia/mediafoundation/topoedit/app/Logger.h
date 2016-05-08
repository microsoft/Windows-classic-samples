#pragma once

#include "stdafx.h"
#include "atlfile.h"

#include <stdio.h>

class CLogger
{
public:
    CLogger(CAtlStringW strFileName)
        : m_cRef(0)
    {
        m_file.Create(strFileName, FILE_WRITE_DATA, FILE_SHARE_READ, CREATE_ALWAYS);
    }

    void LogString(CAtlStringA str)
    {
        m_file.Write(str, str.GetLength());
    }

    void LogFormat(const char* str, ...)
    {
        va_list argptr;

        va_start(argptr, str);
        CAtlStringA strFormat;
        strFormat.FormatV(str, argptr);
        va_end(argptr);

        m_file.Write(strFormat, strFormat.GetLength());
    }

    virtual STDMETHODIMP_(ULONG) AddRef()
    {
        LONG cRef = InterlockedIncrement(&m_cRef);

        return cRef;
    }
    
    virtual STDMETHODIMP_(ULONG) Release() 
    {
        LONG cRef = InterlockedDecrement( &m_cRef );

        if( cRef == 0 )
        {
            delete this;
        }
        return( cRef );
    }
    
private:
    CAtlFile m_file;
    LONG m_cRef;
};

