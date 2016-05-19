//*****************************************************************************
//
// Microsoft Windows Media
// Copyright ( C) Microsoft Corporation. All rights reserved.
//
// FileName:            TSimpleList.h
//
// Abstract:            Implement a sorted list class.
//
//*****************************************************************************

#if !defined(AFX_TSimpleList_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
#define AFX_TSimpleList_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

////////////////////////////////////////////////////////////////////////////////
//
//  Element of the list template
//  Implements operators < , == and != 
//  required by CTSimpleList template
//  Template parameter class must implement method Cleanup()
//
////////////////////////////////////////////////////////////////////////////////
template< class e > class El
{
public :
    El() : m_pNext( NULL ) { }

    ~El()
    {
        m_elem.Cleanup();
    }
    
    explicit El( const e )
    {
        m_elem = e;
        m_pNext = NULL;
    }

    El< e > &operator=( const El< e > &right )
    {
        if( this != &right )
        {
            m_elem = right.m_elem;
            m_pNext = NULL;
        }
        return( *this );
    }

    bool operator<( const El< e > &right )
    {
      return( this->m_elem < right.m_elem );
    }

    bool operator==( const El< e > &right )
    {
      return( this->m_elem == right.m_elem );
    }

    bool operator!=( const El< e > &right )
    {
      return( !( this->m_elem == right.m_elem ) );
    }

    e   m_elem;
    El< e > *m_pNext;
};

////////////////////////////////////////////////////////////////////////////////
typedef enum 
{
    ITER_FIRST,
    ITER_NEXT
} IterationType;

////////////////////////////////////////////////////////////////////////////////
// List template declaration
////////////////////////////////////////////////////////////////////////////////
template< class T > class CTSimpleList  
{
public:
    CTSimpleList()
    {
        m_pStart = m_pCur = m_pIter = m_pEnd = NULL;
        m_wSize = 0;
    }
    ~CTSimpleList()
    {
        while( NULL != m_pStart )
        {
            m_pCur = m_pStart;
            m_pStart = m_pStart->m_pNext;
            delete m_pCur;
        }
    }

    El<T> *GetStart() { return m_pStart; };
    bool Append( T * pElem );
    bool Erase( El<T> * pElem );
    WORD Size() { return m_wSize; };
    T *Iterate( IterationType Iter, El<T> ** ppEl = NULL );
    T *GetMinElement( El<T> ** ppEl = NULL );

private :
    El< T > *m_pStart, *m_pEnd, *m_pCur, *m_pIter;
    WORD m_wSize;
};

////////////////////////////////////////////////////////////////////////////////
//                  IMPLEMENTATION
////////////////////////////////////////////////////////////////////////////////
template< class T >
bool CTSimpleList< T >::Append( T * pElem )
{
    if( NULL == pElem )
    {
        return FALSE;
    }

    m_pCur = m_pEnd;
    m_pEnd = new El< T >;
    if( NULL == m_pEnd)
    {
        return( FALSE );
    }

    m_pEnd->m_elem = *pElem; 
    m_pEnd->m_pNext = NULL;
    (m_pCur ? m_pCur->m_pNext : m_pStart ) = m_pEnd;
    m_wSize++;

    return( TRUE );
}

////////////////////////////////////////////////////////////////////////////////
template< class T >
bool CTSimpleList< T >::Erase( El<T> * pElem )
{
    if( NULL == pElem || NULL == m_pStart )
    {
        return FALSE;
    }

    El< T > * pTemp = NULL;

    if( m_pStart == pElem )
    {
        pTemp = m_pStart;
        m_pStart = m_pStart->m_pNext;
        delete pTemp;
        m_wSize--;
        return TRUE;
    }
    else
    {
        for( El< T > * pEl = m_pStart; NULL != pEl; pEl = pEl->m_pNext )
        {
            if( NULL != pEl->m_pNext && pEl->m_pNext == pElem )
            {
               pTemp = pEl->m_pNext;
               pEl->m_pNext = pEl->m_pNext->m_pNext;
               delete pTemp;
               m_wSize--;
               return TRUE;
            }
        }
    }

    return (FALSE);
}

////////////////////////////////////////////////////////////////////////////////
template< class T >
T * CTSimpleList< T >::Iterate( IterationType Iter, El<T> ** ppEl ) 
{
    do
    {
        if( NULL == m_pStart )
        {
            break;
        }

        if( ITER_FIRST == Iter )
        {
            m_pIter = m_pStart;
            if( NULL != ppEl )
            {
                *ppEl = m_pStart;
            }
            return &( m_pIter->m_elem );
        }

        if( NULL == m_pIter )
        {
            break;
        }

        m_pIter = m_pIter->m_pNext;

        if( NULL != m_pIter )
        {
            if( NULL != ppEl )
            {
                *ppEl = m_pIter;
            }
            return( &(m_pIter->m_elem) );
        }

    } while( FALSE );

    if( NULL != ppEl )
    {
        *ppEl = NULL;
    }
    return ( ( T *)NULL );

}

////////////////////////////////////////////////////////////////////////////////
template< class T >
T * CTSimpleList< T >::GetMinElement( El<T> ** ppEl )
{
    if( NULL == ppEl || NULL == m_pStart )
    {
        return ( ( T *)NULL );
    }

    El< T > * pMin = m_pStart;

    for( El< T > * pEl = m_pStart->m_pNext; NULL != pEl; pEl = pEl->m_pNext )
    {
        if( *pEl < *pMin )
        {
            pMin = pEl;
        }
    }

    if( NULL != ppEl )
    {
        *ppEl = pMin;
    }
    return &( pMin->m_elem );
}

////////////////////////////////////////////////////////////////////////////////
template < class T, class A >
T *Find( CTSimpleList< T > * pLst, A * pElem, El<T> ** ppEl )
{
    if( NULL == pLst || NULL == pElem )
    {
        return NULL;
    }

    El<T> * pStart = pLst->GetStart();

    if( NULL != pStart && NULL != pElem )
    {
        for( El< T > * pEl = pStart; NULL != pEl; pEl = pEl->m_pNext )
        {
            if( pEl->m_elem == *pElem )
            {                
                if( NULL != ppEl )
                {
                    *ppEl = pEl;
                }

                return( &( pEl->m_elem ) );
            }
        }
    }

    if( NULL != ppEl )
    {
        *ppEl = NULL;
    }
    return NULL;
}

#endif // !defined(AFX_TSimpleList_H__21A57EE3_8C94_4E10_8092_4C171BCBA68E__INCLUDED_)
