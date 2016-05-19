//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            ScriptList.h
//
// Abstract:            Interface & implementation for the CScript 
//                      and CScriptList classes.
//
//*****************************************************************************

#if !defined(AFX_ScriptList_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
#define AFX_ScriptList_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef SAFE_RELEASE
    #define SAFE_RELEASE( x )           \
        if ( NULL != x )                \
        {                               \
            x->Release( );              \
            x = NULL;                   \
        }
#endif // SAFE_RELEASE

#ifndef SAFE_DELETE
    #define SAFE_DELETE( x )            \
       if( x )                          \
       {                                \
           delete x;                    \
           x = NULL;                    \
       }
#endif //SAFE_DELETE

#ifndef SAFE_ARRAYDELETE
    #define SAFE_ARRAYDELETE( x )       \
       if( x )                          \
       {                                \
           delete [] x;                 \
           x = NULL;                    \
       }
#endif //SAFE_ARRAYDELETE

#ifndef SAFE_CLOSEHANDLE
    #define SAFE_CLOSEHANDLE( h )       \
        if( NULL != h )                 \
        {                               \
            CloseHandle( h );           \
            h = NULL;                   \
        }
#endif //SAFE_CLOSEHANDLE

////////////////////////////////////////////////////////////////////////////////// 
// This class stores the type, parameter and time information of the
// script entry, in a queue-style linked list 
//////////////////////////////////////////////////////////////////////////////////
class CScript
{
public:
    CScript () 
    {
        m_pwszType = NULL;
        m_pwszParameter = NULL;      
        m_pNextScript = NULL;
    };

    ~CScript( )
    {
        SAFE_ARRAYDELETE( m_pwszType );
        SAFE_ARRAYDELETE( m_pwszParameter );
    };

    HRESULT Initial( __in LPWSTR pwszType,           // Script command type
                     __in LPWSTR pwszParameter,      // Script command parameter
                     QWORD cnsTime )            // Script command time
    {
        if ( NULL == pwszType || NULL == pwszParameter )
        {
            return( E_INVALIDARG );
        }

        m_pwszType = new WCHAR[ wcslen( pwszType ) + 1 ];
        if ( NULL == m_pwszType )
        {
            return( E_OUTOFMEMORY );
        }

        m_pwszParameter = new WCHAR[ wcslen( pwszParameter ) + 1 ];
        if ( NULL == m_pwszParameter )
        {
            SAFE_ARRAYDELETE( m_pwszType );
            return( E_OUTOFMEMORY );
        }

        (void)StringCchCopyW( m_pwszType, wcslen( pwszType ) + 1, pwszType );
        (void)StringCchCopyW( m_pwszParameter, wcslen( pwszParameter) + 1, pwszParameter );
        m_cnsTime = cnsTime;

        return( S_OK );
    };

    inline WCHAR *      GetType ( ) { return m_pwszType; } ;
    inline WCHAR *      GetParameter ( ) { return m_pwszParameter; } ;
    inline QWORD        GetTime ( ) { return m_cnsTime; } ;

    CScript *m_pNextScript;

private:
    WCHAR   *m_pwszType;
    WCHAR   *m_pwszParameter;
    QWORD   m_cnsTime;
};

////////////////////////////////////////////////////////////////////////////////// 
// This class implements the linked list (queue) to store all script data       //
//////////////////////////////////////////////////////////////////////////////////
class CScriptList
{
public:
    CScriptList( )
        : m_pHead ( NULL ),
          m_pTail ( NULL )
    {
    };

    ~CScriptList( )
    {
        CScript * pScriptNode;

        while( m_pHead )
        {
            pScriptNode = m_pHead;
            m_pHead = m_pHead->m_pNextScript;
            SAFE_DELETE( pScriptNode );
        }
    };


    //////////////////////////////////////////////////////////////////////////////////
    // This function adds new script data to the link list implemented as a queue //
    //////////////////////////////////////////////////////////////////////////////////
    HRESULT AddScript( __in LPWSTR pwszType,         // Script command type
                       __in LPWSTR pwszParameter,    // Script command parameter
                       QWORD cnsTime )          // Script command time
    {
        CScript * pNewScript = new CScript();
        if ( NULL == pNewScript )
        {
            return E_OUTOFMEMORY;
        }

        HRESULT hr = pNewScript->Initial( pwszType, pwszParameter, cnsTime );
        if ( FAILED( hr ) )
        {
            SAFE_DELETE( pNewScript );
            return hr;
        }

        if ( m_pTail )
            m_pTail->m_pNextScript = pNewScript;

        if ( !m_pHead )
            m_pHead = pNewScript;

        m_pTail = pNewScript;

        return( hr );
    };

    //////////////////////////////////////////////////////////////////////////////////
    // This function returns a list item from the queue. It removes the        
    // returned item from the queue but does not free the memory. The calling 
    // function is responsible for deleting this memory.
    //////////////////////////////////////////////////////////////////////////////////
    CScript * GetScript( CScript **ppScript )
    {
        *ppScript = m_pHead;

        if ( m_pHead )
        {
            m_pHead = m_pHead->m_pNextScript;
        }
        else
        {
            m_pTail = NULL;
        }

        return *ppScript;
    };

private:
    CScript         * m_pHead;
    CScript         * m_pTail;
};

#endif // !defined(AFX_ScriptList_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)


