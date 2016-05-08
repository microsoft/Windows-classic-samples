/////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) 2003 <company name>
//
//  Module Name:
//      PropList.cpp
//
//  Description:
//      Implementation of the CClusPropList class.
//
//  Author:
//      <name> (<e-mail name>) Mmmm DD, 2003
//
//  Revision History:
//
//  Notes:
//
/////////////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "PropList.h"
#include <StrSafe.h>

#ifdef _DEBUG
#ifdef __AFX_H__
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif // __AFX_H__
#endif

#ifndef __AFX_H__
class CMemoryException
{
public:
    void Delete( void ) { }

}; //*** class CMemoryException
#endif // __AFX_H__

/////////////////////////////////////////////////////////////////////////////
// Constant Definitions
/////////////////////////////////////////////////////////////////////////////

const int BUFFER_GROWTH_FACTOR = 256;

//////////////////////////////////////////////////////////////////////////////
// Globals
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// String comparison routines
//////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//++
//
//  WrapCompareString
//
//  Description:
//      Wraps the call to CompareString
//
//  Arguments:
//      fCaseSensitive      TRUE for a case sensitive compare
//                          FALSE for a case insensitive compare
//      pcwszString1In      The first string.
//      cch1In              The count of characters in the first string.
//                          If -1 then it's assumed to be null-terminated.
//      pcwszString2In      The second string.
//      cchIn               The count of characters in the second string.
//                          If -1 then it's assumed to be null-terminated.
//
//  Return Value:
//       0  Strings are equal.
//     > 0  String1 greater than String2.
//     < 0  String1 less than String2.
//
//--
/////////////////////////////////////////////////////////////////////////////
int
WrapCompareString(
      BOOL      fCaseSensitive
    , LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    )
{
    int nRet;

    nRet = CompareStringW(
              LOCALE_SYSTEM_DEFAULT
            , fCaseSensitive ? 0 : NORM_IGNORECASE
            , pcwszString1In
            , static_cast< DWORD >( cch1In )
            , pcwszString2In
            , static_cast< DWORD >( cch2In )
            );

    return nRet - CSTR_EQUAL; // CSTR_LT < CSTR_EQUAL < CSTR_GT

} // *** WrapCompareString

//
// Full string compares.
//

int
IBSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    )
{
    return WrapCompareString(
                  FALSE
                , static_cast< LPCWSTR >( bstrString1In )
                , SysStringLen( bstrString1In )
                , static_cast< LPCWSTR >( bstrString2In )
                , SysStringLen( bstrString2In )
                );
} //*** IBSTRCompareW

int
BSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    )
{
    return WrapCompareString(
                  TRUE
                , static_cast< LPCWSTR >( bstrString1In )
                , SysStringLen( bstrString1In )
                , static_cast< LPCWSTR >( bstrString2In )
                , SysStringLen( bstrString2In )
                );
} //*** BSTRCompareW

int
IStringCompareW(
      LPCWSTR pcwszString1In
    , LPCWSTR pcwszString2In
    )
{
    return WrapCompareString(
                  FALSE
                , pcwszString1In
                , static_cast< size_t >( -1 )
                , pcwszString2In
                , static_cast< size_t >( -1 )
            );
} //*** IStringCompareW

int
StringCompareW(
      LPCWSTR pcwszString1In
    , LPCWSTR pcwszString2In
    )
{
    return WrapCompareString(
                  TRUE
                , pcwszString1In
                , static_cast< size_t >( -1 )
                , pcwszString2In
                , static_cast< size_t >( -1 )
                );
} //*** StringCompareW

int
IStringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    )
{
    return WrapCompareString(
                  FALSE
                , pcwszString1In
                , cch1In
                , pcwszString2In
                , cch2In
                );
} //*** IStringCompareW

int
StringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    )
{
    return WrapCompareString(
                  TRUE
                , pcwszString1In
                , cch1In
                , pcwszString2In
                , cch2In
                );
} //*** StringCompareW

//
// Partial string compares.  They use whichever one's length is shortest
// if both are specified or the NBSTR routines are used.
//

int
NIBSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    )
{
    size_t cchMin;
    
    cchMin = min( SysStringLen( bstrString1In ), SysStringLen( bstrString2In ) );

    return WrapCompareString(
                  FALSE
                , static_cast< LPCWSTR >( bstrString1In )
                , cchMin
                , static_cast< LPCWSTR >( bstrString2In )
                , cchMin
                );
} //*** NIBSTRCompareW

int
NBSTRCompareW(
      BSTR  bstrString1In
    , BSTR  bstrString2In
    )
{
    size_t cchMin;
    
    cchMin = min( SysStringLen( bstrString1In ), SysStringLen( bstrString2In ) );

    return WrapCompareString(
                  TRUE
                , static_cast< LPCWSTR >( bstrString1In )
                , cchMin
                , static_cast< LPCWSTR >( bstrString2In )
                , cchMin
                );
} //*** NBSTRCompareW

int
NIStringCompareW(
      LPCWSTR   pcwszString1In
    , LPCWSTR   pcwszString2In
    , size_t    cchIn
    )
{
    return WrapCompareString(
                  FALSE
                , pcwszString1In
                , cchIn
                , pcwszString2In
                , cchIn
                );
} //*** NIStringCompareW

int
NStringCompareW(
      LPCWSTR   pcwszString1In
    , LPCWSTR   pcwszString2In
    , size_t    cchIn
    )
{
    return WrapCompareString(
                  TRUE
                , pcwszString1In
                , cchIn
                , pcwszString2In
                , cchIn
                );
} //*** NIStringCompareW

int
NIStringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    )
{
    return WrapCompareString(
                  FALSE
                , pcwszString1In
                , min( cch1In, cch2In )
                , pcwszString2In
                , min( cch1In, cch2In )
                );
} //*** NIStringCompareW

int
NStringCompareW(
      LPCWSTR   pcwszString1In
    , size_t    cch1In
    , LPCWSTR   pcwszString2In
    , size_t    cch2In
    )
{
    return WrapCompareString(
                  TRUE
                , pcwszString1In
                , min( cch1In, cch2In )
                , pcwszString2In
                , min( cch1In, cch2In )
                );
} //*** NStringCompareW

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CchMultiSz
//
//  Description:
//      Length of all of the substrings of a multisz string minus the final NULL.
//
//      (i.e., includes the nulls of the substrings, excludes the final null)
//      multiszlen( "abcd\0efgh\0\0" => 5 + 5 = 10
//
//  Arguments:
//      pszIn     The string to get the length of.
//
//  Return Value:
//      Count of characters in the multisz or 0 if empty.
//
//--
/////////////////////////////////////////////////////////////////////////////
static size_t
CchMultiSz(
    LPCWSTR pszIn
    )
{
    ASSERT( pszIn != NULL );

    size_t  cchTotal = 0;
    size_t  cchChars = 0;

    while ( *pszIn != L'\0' )
    {
        cchChars = lstrlenW( pszIn ) + 1;

        cchTotal += cchChars;
        pszIn += cchChars;
    } // while: pointer not stopped on EOS

    return cchTotal;

} //*** CchMultiSz

/////////////////////////////////////////////////////////////////////////////
//++
//
//  NCompareMultiSz
//
//  Description:
//      Compare two MULTI_SZ buffers.
//
//  Arguments:
//      pszSourceIn   The source string.
//      pszTargetIn   The target string.
//
//  Return Value:
//      If the string pointed to by pszSource is less than the string pointed
//      to by pszTarget, the return value is negative. If the string pointed
//      to by pszSource is greater than the string pointed to by pszTarget,
//      the return value is positive. If the strings are equal, the return value
//      is zero.
//
//--
/////////////////////////////////////////////////////////////////////////////
static int NCompareMultiSz(
      LPCWSTR pszSourceIn
    , LPCWSTR pszTargetIn
    )
{
    ASSERT( pszSourceIn != NULL );
    ASSERT( pszTargetIn != NULL );

    while ( ( *pszSourceIn != L'\0' ) && ( *pszTargetIn != L'\0') )
    {
        //
        // Move to end of strings.
        //

        while ( ( *pszSourceIn != L'\0' ) && ( *pszTargetIn != L'\0') && ( *pszSourceIn == *pszTargetIn ) )
        {
            ++pszSourceIn;
            ++pszTargetIn;
        } // while: pointer not stopped on EOS

        //
        // If strings are the same, skip past terminating NUL.
        // Otherwise exit the loop.

        if ( ( *pszSourceIn == L'\0' ) && ( *pszTargetIn == L'\0') )
        {
            ++pszSourceIn;
            ++pszTargetIn;
        } // if: both stopped on EOS
        else
        {
            break;
        } // else: stopped because something is not equal -- we are done.

    } // while: pointer not stopped on EOS

    return *pszSourceIn - *pszTargetIn;

} //*** NCompareMultiSz

/////////////////////////////////////////////////////////////////////////////
// CClusPropValueList class
/////////////////////////////////////////////////////////////////////////////

#ifdef __AFX_H__
IMPLEMENT_DYNAMIC( CClusPropValueList, CObject );
#endif // __AFX_H__

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropValueList::ScMoveToFirstValue
//
//  Description:
//      Move the cursor to the first value in the value list.
//
//  Arguments:
//      None.
//
//  Return Value:
//      ERROR_SUCCESS   Position moved to the first value successfully.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropValueList::ScMoveToFirstValue( void )
{
    ASSERT( m_cbhValueList.pb != NULL );

    DWORD   sc;

    m_cbhCurrentValue = m_cbhValueList;
    m_cbDataLeft = m_cbDataSize;
    m_fAtEnd = FALSE;

    if ( m_cbhCurrentValue.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK )
    {
        sc = ERROR_NO_MORE_ITEMS;
    } // if: no items in the value list
    else
    {
        sc = ERROR_SUCCESS;
    } // else: items exist in the value list

    return sc;

} //*** CClusPropValueList::ScMoveToFirstValue

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropValueList::ScMoveToNextValue
//
//  Description:
//      Move the cursor to the next value in the list.
//
//  Arguments:
//      None.
//
//  Return Value:
//      ERROR_SUCCESS       Position moved to the next value successfully.
//      ERROR_NO_MORE_ITEMS Already at the end of the list.
//      ERROR_INVALID_DATA  Not enough data in the buffer.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropValueList::ScMoveToNextValue( void )
{
    ASSERT( m_cbhCurrentValue.pb != NULL );

    DWORD                   sc     = ERROR_NO_MORE_ITEMS;
    size_t                  cbDataSize;
    CLUSPROP_BUFFER_HELPER  cbhCurrentValue;

    cbhCurrentValue = m_cbhCurrentValue;

    //
    // Don't try to move if we're already at the end.
    //

    if ( m_fAtEnd )
    {
        goto Cleanup;
    } // if: already at the end of the list

    //
    // Make sure the buffer is big enough for the value header.
    //

    if ( m_cbDataLeft < sizeof( *cbhCurrentValue.pValue ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Calculate how much to advance buffer pointer.
    //

    cbDataSize = sizeof( *cbhCurrentValue.pValue )
                + ALIGN_CLUSPROP( cbhCurrentValue.pValue->cbLength );

    //
    // Make sure the buffer is big enough for the value header,
    // the data itself, and the endmark.
    //

    if ( m_cbDataLeft < cbDataSize + sizeof( CLUSPROP_SYNTAX ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Move past the current value to the next value's syntax.
    //

    cbhCurrentValue.pb += cbDataSize;

    //
    // This test will ensure that the value is always valid since we won't
    // advance if the next thing is the endmark.
    //

    if ( cbhCurrentValue.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK )
    {
        m_cbhCurrentValue = cbhCurrentValue;
        m_cbDataLeft -= cbDataSize;
        sc = ERROR_SUCCESS;
    } // if: next value's syntax is not the endmark
    else
    {
        m_fAtEnd = TRUE;
    } // else: next value's syntax is the endmark

Cleanup:

    return sc;

} //*** CClusPropValueList::ScMoveToNextValue

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropValueList::ScCheckIfAtLastValue
//
//  Description:
//      Indicate whether we are on the last value in the list or not.
//
//  Arguments:
//      None.
//
//  Return Value:
//      ERROR_SUCCESS       Not currently at the last value in the list.
//      ERROR_NO_MORE_ITEMS Currently at the last value in the list.
//      ERROR_INVALID_DATA  Not enough data in the buffer.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropValueList::ScCheckIfAtLastValue( void )
{
    ASSERT( m_cbhCurrentValue.pb != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    CLUSPROP_BUFFER_HELPER  cbhCurrentValue;
    size_t                  cbDataSize = 0;

    cbhCurrentValue = m_cbhCurrentValue;

    //
    // Don't try to recalculate if we already know
    // we're at the end of the list.
    //

    if ( m_fAtEnd )
    {
        goto Cleanup;
    } // if: already at the end of the list

    //
    // Make sure the buffer is big enough for the value header.
    //

    if ( m_cbDataLeft < sizeof( *cbhCurrentValue.pValue ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Calculate how much to advance buffer pointer.
    //

    cbDataSize = sizeof( *cbhCurrentValue.pValue )
                + ALIGN_CLUSPROP( cbhCurrentValue.pValue->cbLength );

    //
    // Make sure the buffer is big enough for the value header,
    // the data itself, and the endmark.
    //

    if ( m_cbDataLeft < cbDataSize + sizeof( CLUSPROP_SYNTAX ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Move past the current value to the next value's syntax.
    //

    cbhCurrentValue.pb += cbDataSize;

    //
    // We are on the last value if the next thing after this value
    // is an endmark.
    //

    if ( cbhCurrentValue.pSyntax->dw == CLUSPROP_SYNTAX_ENDMARK )
    {
        sc = ERROR_NO_MORE_ITEMS;
    } // if: next value's syntax is the endmark

Cleanup:

    return sc;

} //*** CClusPropValueList::ScCheckIfAtLastValue

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropValueList::ScAllocValueList
//
//  Description:
//      Allocate a value list buffer that's big enough to hold the next
//      value.
//
//  Arguments:
//      cbMinimumIn   Minimum size of the value list.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropValueList::ScAllocValueList(
    size_t cbMinimumIn
    )
{
    ASSERT( cbMinimumIn > 0 );

    DWORD   sc = ERROR_SUCCESS;
    size_t  cbTotal = 0;

    //
    // Add the size of the item count and final endmark.
    //

    cbMinimumIn += sizeof( CLUSPROP_VALUE );
    cbTotal = m_cbDataSize + cbMinimumIn;

    if ( m_cbBufferSize < cbTotal )
    {
        PBYTE   _pbNewValuelist = NULL;

        cbMinimumIn = max( BUFFER_GROWTH_FACTOR, cbMinimumIn );
        cbTotal = m_cbDataSize + cbMinimumIn;

        //
        // Allocate and zero a new buffer.
        //

        _pbNewValuelist = new BYTE[ cbTotal ];
        if ( _pbNewValuelist != NULL )
        {
            ZeroMemory( _pbNewValuelist, cbTotal );

            //
            // If there was a previous buffer, copy it and the delete it.
            //

            if ( m_cbhValueList.pb != NULL )
            {
                if ( m_cbDataSize != 0 )
                {
                    CopyMemory( _pbNewValuelist, m_cbhValueList.pb, m_cbDataSize );
                } // if: data already exists in buffer

                delete [] m_cbhValueList.pb;
                m_cbhCurrentValue.pb = _pbNewValuelist + (m_cbhCurrentValue.pb - m_cbhValueList.pb);
            } // if: there was a previous buffer
            else
            {
                m_cbhCurrentValue.pb = _pbNewValuelist + sizeof( DWORD ); // move past prop count
            } // else: no previous buffer

            //
            // Save the new buffer.
            //

            m_cbhValueList.pb = _pbNewValuelist;
            m_cbBufferSize = cbTotal;
        } // if: allocation succeeded
        else
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
        } // else: allocation failed
    } // if: buffer isn't big enough

    return sc;

} //*** CClusPropValueList::ScAllocValueList

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropValueList::ScGetResourceValueList
//
//  Description:
//      Get value list of a resource.
//
//  Arguments:
//      hResourceIn     Handle for the resource to get properties from.
//      dwControlCodeIn Control code for the request.
//      hHostNodeIn     Handle for the node to direct this request to.
//                      Defaults to NULL.
//      lpBufferIn      Input buffer for the request.  Defaults to NULL.
//      cbBufferIn      Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropValueList::ScGetResourceValueList(
      HRESOURCE hResourceIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn
    , size_t    cbBufferIn
    )
{
    ASSERT( hResourceIn != NULL );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_RESOURCE << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cb = 512;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get values.
    //

    sc = ScAllocValueList( cb );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterResourceControl(
                        hResourceIn,
                        hHostNodeIn,
                        dwControlCodeIn,
                        lpBufferIn,
                        static_cast< DWORD >( cbBufferIn ),
                        m_cbhValueList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &cb
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocValueList( cb );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterResourceControl(
                                hResourceIn,
                                hHostNodeIn,
                                dwControlCodeIn,
                                lpBufferIn,
                                static_cast< DWORD >( cbBufferIn ),
                                m_cbhValueList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &cb
                                );
            } // if: ScAllocValueList succeeded
        } // if: buffer too small
    } // if: ScAllocValueList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeleteValueList();
    } // if: error getting properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cb );
        m_cbDataLeft = static_cast< size_t >( cb );
    } // else: no errors

    return sc;

} //*** CClusPropValueList::ScGetResourceValueList

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropValueList::ScGetResourceTypeValueList
//
//  Description:
//      Get value list of a resource type.
//
//  Arguments:
//      hClusterIn          Handle for the cluster in which the resource
//                          type resides.
//      pwszResTypeNameIn   Name of the resource type.
//      dwControlCodeIn     Control code for the request.
//      hHostNodeIn         Handle for the node to direct this request to.
//                          Defaults to NULL.
//      lpBufferIn          Input buffer for the request.  Defaults to NULL.
//      cbBufferIn          Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropValueList::ScGetResourceTypeValueList(
      HCLUSTER  hClusterIn
    , LPCWSTR   pwszResTypeNameIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn
    , size_t    cbBufferIn
    )
{
    ASSERT( hClusterIn != NULL );
    ASSERT( pwszResTypeNameIn != NULL );
    ASSERT( *pwszResTypeNameIn != L'\0' );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_RESOURCE_TYPE << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cb = 512;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get values.
    //

    sc = ScAllocValueList( cb );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterResourceTypeControl(
                        hClusterIn,
                        pwszResTypeNameIn,
                        hHostNodeIn,
                        dwControlCodeIn,
                        lpBufferIn,
                        static_cast< DWORD >( cbBufferIn ),
                        m_cbhValueList.pb,
                        static_cast< DWORD >( m_cbBufferSize ),
                        &cb
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocValueList( cb );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterResourceTypeControl(
                                hClusterIn,
                                pwszResTypeNameIn,
                                hHostNodeIn,
                                dwControlCodeIn,
                                lpBufferIn,
                                static_cast< DWORD >( cbBufferIn ),
                                m_cbhValueList.pb,
                                static_cast< DWORD >( m_cbBufferSize ),
                                &cb
                                );
            } // if: ScAllocValueList succeeded
        } // if: buffer too small
    } // if: ScAllocValueList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeleteValueList();
    } // if: error getting properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cb );
        m_cbDataLeft = static_cast< size_t >( cb );
    } // else: no errors

    return sc;

} //*** CClusPropValueList::ScGetResourceTypeValueList


//*************************************************************************//


/////////////////////////////////////////////////////////////////////////////
// CClusPropList class
/////////////////////////////////////////////////////////////////////////////

#ifdef __AFX_H__
IMPLEMENT_DYNAMIC( CClusPropList, CObject );
#endif // __AFX_H__

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScCopy
//
//  Description:
//      Copy a property list.  This function is equivalent to an assignment
//      operator.  Since this operation can fail, no assignment operator is
//      provided.
//
//  Arguments:
//      pcplPropListIn    The proplist to copy into this instance.
//      cbPropListIn      The total size of the prop list.
//
//  Return Value:
//      Win32 status code.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScCopy(
      const PCLUSPROP_LIST  pcplPropListIn
    , size_t                cbPropListIn
    )
{
    ASSERT( pcplPropListIn != NULL );

    DWORD   sc = ERROR_SUCCESS;

    //
    // Clean up any vestiges of a previous prop list.
    //

    if ( m_cbhPropList.pb != NULL )
    {
        DeletePropList();
    } // if: the current list is not empty

    //
    // Allocate the new property list buffer.  If successful,
    // copy the source list.
    //

    m_cbhPropList.pb = new BYTE[ cbPropListIn ];
    if ( m_cbhPropList.pb != NULL )
    {
        CopyMemory( m_cbhPropList.pList, pcplPropListIn, cbPropListIn );
        m_cbBufferSize = cbPropListIn;
        m_cbDataSize   = cbPropListIn;
        m_cbDataLeft   = cbPropListIn;
        sc = ScMoveToFirstProperty();
    } // if: new succeeded
    else
    {
        sc = ERROR_NOT_ENOUGH_MEMORY;
    } // else:

    return sc;

} //*** CClusPropList::ScCopy

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAppend
//
//  Description:
//      Append to a property list.
//
//  Arguments:
//      rclPropListIn    The proplist to append onto this instance.
//
//  Return Value:
//      Win32 status code.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAppend(
    const CClusPropList & rclPropListIn
    )
{
    DWORD   sc = ERROR_SUCCESS;
    size_t  cbPropertyCountOffset = 0;
    size_t  cbIncrement = 0;
    size_t  cbDataLeft = 0;
    PBYTE   pbsrc = NULL;
    PBYTE   pbdest = NULL;

    //
    //  Compute the number of bytes to get past the count of properties that
    //  is at the head of the list.  This is typically sizeof DWORD.
    //

    cbPropertyCountOffset = sizeof( m_cbhPropList.pList->nPropertyCount );

    //
    //  Compute the allocation increment.  This is used when growing our buffer
    //  and is the amount of data to copy from the passed in list.  This includes
    //  the trailing endmark.  m_cbDataSize does not include the leading property
    //  count DWORD.
    //

    cbIncrement = rclPropListIn.m_cbDataSize;

    //
    //  How much space remains in our current buffer?
    //

    cbDataLeft = m_cbBufferSize - m_cbDataSize;

    //
    //  If the size of the list to append is larger than what we have remaining
    //  then we need to grow the list.
    //

    if ( cbIncrement > cbDataLeft )
    {
        sc = ScAllocPropList( m_cbDataSize + cbIncrement );
        if ( sc != ERROR_SUCCESS )
        {
            goto Cleanup;
        } // if:
    } // if:

    pbdest = (PBYTE) &m_cbhPropList.pb[ cbPropertyCountOffset + m_cbDataSize ];

    pbsrc = (PBYTE) &rclPropListIn.m_cbhPropList.pList->PropertyName;

    CopyMemory( pbdest, pbsrc, cbIncrement );

    //
    //  Grow our data size to match our new size.
    //

    m_cbDataSize += cbIncrement;

    //
    //  Increment our property count to include the count of properties appended to the end
    //  of our buffer.
    //

    m_cbhPropList.pList->nPropertyCount += rclPropListIn.m_cbhPropList.pList->nPropertyCount;

Cleanup:

    return sc;

} //*** CClusPropList::ScAppend

////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScMoveToFirstProperty
//
//  Description:
//      Move the cursor to the first propery in the list.
//
//  Arguments:
//      None.
//
//  Return Value:
//      ERROR_SUCCESS       Position moved to the first property successfully.
//      ERROR_NO_MORE_ITEMS There are no properties in the list.
//      ERROR_INVALID_DATA  Not enough data in the buffer.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScMoveToFirstProperty( void )
{
    ASSERT( m_cbhPropList.pb != NULL );
    ASSERT( m_cbDataSize >= sizeof( m_cbhPropList.pList->nPropertyCount ) );

    DWORD                   sc;
    size_t                  cbDataLeft;
    size_t                  cbDataSize;
    CLUSPROP_BUFFER_HELPER  cbhCurrentValue;

    //
    // Make sure the buffer is big enough for the list header.
    //

    if ( m_cbDataSize < sizeof( m_cbhPropList.pList->nPropertyCount ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data

    //
    // Set the property counter to the number of properties in the list.
    //

    m_nPropsRemaining = m_cbhPropList.pList->nPropertyCount;

    //
    // Point the name pointer to the first name in the list.
    //

    m_cbhCurrentPropName.pName = &m_cbhPropList.pList->PropertyName;
    m_cbDataLeft = m_cbDataSize - sizeof( m_cbhPropList.pList->nPropertyCount );

    //
    // Check to see if there are any properties in the list.
    //

    if ( m_nPropsRemaining == 0 )
    {
        sc = ERROR_NO_MORE_ITEMS;
        goto Cleanup;
    } // if: no properties in the list

    //
    // Make sure the buffer is big enough for the first property name.
    //

    if ( m_cbDataLeft < sizeof( *m_cbhCurrentPropName.pName ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Calculate how much to advance the buffer pointer.
    //

    cbDataSize = sizeof( *m_cbhCurrentPropName.pName )
                + ALIGN_CLUSPROP( m_cbhCurrentPropName.pName->cbLength );

    //
    // Make sure the buffer is big enough for the name header
    // and the data itself.
    //

    if ( m_cbDataLeft < cbDataSize )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Point the value buffer to the first value in the list.
    //

    cbhCurrentValue.pb = m_cbhCurrentPropName.pb + cbDataSize;
    cbDataLeft = m_cbDataLeft - cbDataSize;
    m_pvlValues.Init( cbhCurrentValue, cbDataLeft );

    //
    // Indicate we are successful.
    //

    sc = ERROR_SUCCESS;

Cleanup:

    return sc;

} //*** CClusPropList::ScMoveToFirstProperty

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScMoveToNextProperty
//
//  Description:
//      Move the cursor to the next property in the list.
//
//  Arguments:
//      None.
//
//  Return Value:
//      ERROR_SUCCESS       Position moved to the next property successfully.
//      ERROR_NO_MORE_ITEMS Already at the end of the list.
//      ERROR_INVALID_DATA  Not enough data in the buffer.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScMoveToNextProperty( void )
{
    ASSERT( m_cbhPropList.pb != NULL );
    ASSERT( m_pvlValues.CbhValueList().pb != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    size_t                  cbNameSize = 0;
    size_t                  cbDataLeft = 0;
    size_t                  cbDataSize = 0;
    CLUSPROP_BUFFER_HELPER  cbhCurrentValue;
    CLUSPROP_BUFFER_HELPER  cbhPropName;

    cbhCurrentValue = m_pvlValues;
    cbDataLeft = m_pvlValues.CbDataLeft();

    //
    // If we aren't already at the last property, attempt to move to the next one.
    //

    sc = ScCheckIfAtLastProperty();
    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: already at the last property (probably)

    //
    // Make sure the buffer is big enough for the value header.
    //

    if ( cbDataLeft < sizeof( *cbhCurrentValue.pValue ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data left

    //
    // Careful!  Add offset only to cbhCurrentValue.pb.  Otherwise
    // pointer arithmetic will give undesirable results.
    //

    while ( cbhCurrentValue.pSyntax->dw != CLUSPROP_SYNTAX_ENDMARK )
    {
        //
        // Make sure the buffer is big enough for the value
        // and an endmark.
        //

        cbDataSize = sizeof( *cbhCurrentValue.pValue )
                    + ALIGN_CLUSPROP( cbhCurrentValue.pValue->cbLength );
        if ( cbDataLeft < cbDataSize + sizeof( *cbhCurrentValue.pSyntax ) )
        {
            sc = ERROR_INVALID_DATA;
            goto Cleanup;
        } // if: not enough data left

        //
        // Advance past the value.
        //

        cbhCurrentValue.pb += cbDataSize;
        cbDataLeft -= cbDataSize;
    } // while: not at endmark

    if ( sc != ERROR_SUCCESS )
    {
        goto Cleanup;
    } // if: error occurred in loop

    //
    // Advanced past the endmark.
    // Size check already performed in above loop.
    //

    cbDataSize = sizeof( *cbhCurrentValue.pSyntax );
    cbhCurrentValue.pb += cbDataSize;
    cbDataLeft -= cbDataSize;

    //
    // Point the name pointer to the next name in the list.
    //

    cbhPropName = cbhCurrentValue;
    ASSERT( cbDataLeft == m_cbDataSize - (cbhPropName.pb - m_cbhPropList.pb) );

    //
    // Calculate the size of the name with header.
    // Make sure the buffer is big enough for the name and an endmark.
    //

    if ( cbDataLeft < sizeof( *cbhPropName.pName ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data

    cbNameSize = sizeof( *cbhPropName.pName )
                + ALIGN_CLUSPROP( cbhPropName.pName->cbLength );

    if ( cbDataLeft < cbNameSize + sizeof( CLUSPROP_SYNTAX ) )
    {
        sc = ERROR_INVALID_DATA;
        goto Cleanup;
    } // if: not enough data

    //
    // Point the value buffer to the first value in the list.
    //

    cbhCurrentValue.pb = cbhPropName.pb + cbNameSize;
    m_cbhCurrentPropName = cbhPropName;
    m_cbDataLeft = cbDataLeft - cbNameSize;
    m_pvlValues.Init( cbhCurrentValue, m_cbDataLeft );

    //
    // We've successfully advanced to the next property,
    // so there is now one fewer property remaining.
    //

    --m_nPropsRemaining;
    ASSERT( m_nPropsRemaining >= 1 );

    sc = ERROR_SUCCESS;

Cleanup:

    return sc;

} //*** CClusPropList::ScMoveToNextProperty

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScMoveToPropertyByName
//
//  Description:
//      Find the passed in property name in the proplist.  Note that the
//      cursor is reset to the beginning at the beginning of the routine and
//      the current state of the cursor is lost.
//
//  Arguments:
//      pwszPropNameIn    Name of the property
//
//  Return Value:
//      ERROR_SUCCESS if the property was found, other Win32 code if not.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScMoveToPropertyByName(
    LPCWSTR pwszPropNameIn
    )
{
    ASSERT( m_cbhPropList.pb != NULL );

    DWORD   sc = ERROR_SUCCESS;

    sc = ScMoveToFirstProperty();
    if ( sc == ERROR_SUCCESS )
    {
        do
        {
            //
            // See if this is the specified property.  If so, we're done.
            //

            if ( IStringCompareW( m_cbhCurrentPropName.pName->sz, pwszPropNameIn ) == 0 )
            {
                break;
            } // if: property found

            //
            // Advance to the next property.
            //

            sc = ScMoveToNextProperty();   // No because we expect an error when at the d

        } while ( sc == ERROR_SUCCESS );   // do-while: not end of list
    } // if: successfully moved to the first property

    return sc;

} //*** ClusPropList::ScMoveToPropertyByName( LPCWSTR )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAllocPropList
//
//  Description:
//      Allocate a property list buffer that's big enough to hold the next
//      property.
//
//  Arguments:
//      cbMinimumIn   Minimum size of the property list.
//
//  Return Value:
//      ERROR_SUCCESS
//      ERROR_NOT_ENOUGH_MEMORY
//      Other Win32 error codes
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAllocPropList(
    size_t cbMinimumIn
    )
{
    ASSERT( cbMinimumIn > 0 );

    DWORD   sc = ERROR_SUCCESS;
    size_t  cbTotal = 0;

    //
    // Add the size of the item count and final endmark.
    //

    cbMinimumIn += sizeof( CLUSPROP_VALUE );
    cbTotal = m_cbDataSize + cbMinimumIn;

    if ( m_cbBufferSize < cbTotal )
    {
        PBYTE   pbNewProplist = NULL;

        cbMinimumIn = max( BUFFER_GROWTH_FACTOR, cbMinimumIn );
        cbTotal = m_cbDataSize + cbMinimumIn;

        //
        // Allocate and zero a new buffer.
        //

        pbNewProplist = new BYTE[ cbTotal ];
        if ( pbNewProplist != NULL )
        {
            ZeroMemory( pbNewProplist, cbTotal );

            //
            // If there was a previous buffer, copy it and the delete it.
            //

            if ( m_cbhPropList.pb != NULL )
            {
                if ( m_cbDataSize != 0 )
                {
                    CopyMemory( pbNewProplist, m_cbhPropList.pb, m_cbDataSize );
                } // if: data already exists in buffer

                delete [] m_cbhPropList.pb;
                m_cbhCurrentProp.pb = pbNewProplist + (m_cbhCurrentProp.pb - m_cbhPropList.pb);
            } // if: there was a previous buffer
            else
            {
                m_cbhCurrentProp.pb = pbNewProplist + sizeof( DWORD ); // move past prop count
            } // else: no previous buffer

            //
            // Save the new buffer.
            //

            m_cbhPropList.pb = pbNewProplist;
            m_cbBufferSize = cbTotal;
        } // if: allocation succeeded
        else
        {
            sc = ERROR_NOT_ENOUGH_MEMORY;
        } // else: allocation failed
    } // if: buffer isn't big enough

    return sc;

} //*** CClusPropList::ScAllocPropList

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddProp
//
//  Description:
//      Add a string property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      pwszValueIn       Value of the property to set in the list.
//      pwszPrevValueIn   Previous value of the property.
//
//  Return Value:
//      ERROR_SUCCESS or other Win32 error code.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddProp(
      LPCWSTR   pwszNameIn
    , LPCWSTR   pwszValueIn
    , LPCWSTR   pwszPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    BOOL                    fValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_SZ            pValue = NULL;

    if ( ( pwszPrevValueIn != NULL ) && ( IStringCompareW( pwszValueIn, pwszPrevValueIn ) == 0 ) )
    {
        fValuesDifferent = FALSE;
    } // if: we have a prev value and the values are the same

    //
    // If we should always add, or if the new value and the previous value
    // are not equal, add the property to the property list.
    //

    if ( m_fAlwaysAddProp || fValuesDifferent )
    {
        size_t  cbNameSize;
        size_t  cbDataSize;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbDataSize = (lstrlenW( pwszValueIn ) + 1) * sizeof( *pwszValueIn );

        cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pStringValue;
            CopyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValueIn, cbDataSize );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist

    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddProp( LPCWSTR )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddMultiSzProp
//
//  Description:
//      Add a string property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      pwszValueIn       Value of the property to set in the list.
//      pwszPrevValueIn   Previous value of the property.
//
//  Return Value:
//      ERROR_SUCCESS or other Win32 error code.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddMultiSzProp(
      LPCWSTR   pwszNameIn
    , LPCWSTR   pwszValueIn
    , LPCWSTR   pwszPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    BOOL                    fValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_MULTI_SZ      pValue = NULL;

    if ( ( pwszPrevValueIn != NULL ) && ( NCompareMultiSz( pwszValueIn, pwszPrevValueIn ) == 0 ) )
    {
        fValuesDifferent = FALSE;
    } // if: we have a prev value and the values are the same

    //
    // If we should always add, or if the new value and the previous value
    // are not equal, add the property to the property list.
    //

    if ( m_fAlwaysAddProp || fValuesDifferent )
    {
        size_t  cbNameSize;
        size_t  cbDataSize;
        size_t  cbValueSize;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbDataSize = static_cast< DWORD >( (CchMultiSz( pwszValueIn ) + 1) * sizeof( *pwszValueIn ) );

        cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pMultiSzValue;
            CopyMultiSzProp( pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValueIn, cbDataSize );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddMultiSzProp( LPCWSTR )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddExpandSzProp
//
//  Description:
//      Add an EXPAND_SZ string property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      pwszValueIn       Value of the property to set in the list.
//      pwszPrevValueIn   Previous value of the property.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddExpandSzProp(
      LPCWSTR   pwszNameIn
    , LPCWSTR   pwszValueIn
    , LPCWSTR   pwszPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    BOOL                    fValuesDifferent = TRUE;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_SZ            pValue = NULL;

    if ( ( pwszPrevValueIn != NULL ) && ( IStringCompareW( pwszValueIn, pwszPrevValueIn ) == 0 ) )
    {
        fValuesDifferent = FALSE;
    } // if: we have a prev value and the values are the same

    //
    // If we should always add, or if the new value and the previous value
    // are not equal, add the property to the property list.
    //

    if ( m_fAlwaysAddProp || fValuesDifferent )
    {
        size_t  cbNameSize = 0;
        size_t  cbDataSize = 0;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbDataSize = (lstrlenW( pwszValueIn ) + 1) * sizeof( *pwszValueIn );

        cbValueSize = sizeof( CLUSPROP_SZ )
                    + ALIGN_CLUSPROP( cbDataSize )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pStringValue;
            CopyExpandSzProp( pValue, CLUSPROP_TYPE_LIST_VALUE, pwszValueIn, cbDataSize );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddExpandSzProp( LPCWSTR )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddProp
//
//  Description:
//      Add a DWORD property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      nValueIn          Value of the property to set in the list.
//      nPrevValueIn      Previous value of the property.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddProp(
      LPCWSTR   pwszNameIn
    , DWORD     nValueIn
    , DWORD     nPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_DWORD         pValue = NULL;

    if ( m_fAlwaysAddProp || ( nValueIn != nPrevValueIn ) )
    {
        size_t  cbNameSize = 0;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbValueSize = sizeof( CLUSPROP_DWORD )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pDwordValue;
            CopyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, nValueIn );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddProp( DWORD )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddProp
//
//  Description:
//      Add a LONG property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      nValueIn          Value of the property to set in the list.
//      nPrevValueIn      Previous value of the property.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD CClusPropList::ScAddProp(
      LPCWSTR  pwszNameIn
    , LONG     nValueIn
    , LONG     nPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_LONG          pValue = NULL;

    if ( m_fAlwaysAddProp || ( nValueIn != nPrevValueIn ) )
    {
        size_t  cbNameSize = 0;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbValueSize = sizeof( CLUSPROP_LONG )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pLongValue;
            CopyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, nValueIn );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddProp( LONG )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddProp
//
//  Description:
//      Add a binary property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      pbValueIn         Value of the property to set in the list.
//      cbValueIn         Count of bytes in pbValueIn.
//      pbPrevValueIn     Previous value of the property.
//      cbPrevValueIn     Count of bytes in pbPrevValueIn.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddProp(
      LPCWSTR               pwszNameIn
    , const unsigned char * pbValueIn
    , size_t                cbValueIn
    , const unsigned char * pbPrevValueIn
    , size_t                cbPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    BOOL                    _fChanged = FALSE;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_BINARY        pValue = NULL;

    //
    // Determine if the buffer has changed.
    //

    if ( m_fAlwaysAddProp || (cbValueIn != cbPrevValueIn) )
    {
        _fChanged = TRUE;
    } // if: always adding the property or the value size changed
    else if ( ( cbValueIn != 0 ) && ( cbPrevValueIn != 0 ) )
    {
        _fChanged = memcmp( pbValueIn, pbPrevValueIn, cbValueIn ) != 0;
    } // else if: value length changed

    if ( _fChanged )
    {
        size_t  cbNameSize = 0;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbValueSize = sizeof( CLUSPROP_BINARY )
                    + ALIGN_CLUSPROP( cbValueIn )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pBinaryValue;
            CopyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, pbValueIn, cbValueIn );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddProp( const unsigned char * )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddProp
//
//  Routine Description:
//      Add a ULONGLONG property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      ullValueIn        Value of the property to set in the list.
//      ullPrevValueIn    Previous value of the property.
//
//  Return Value:
//      ERROR_SUCCESS or other Win32 error codes.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddProp(
      LPCWSTR   pwszNameIn
    , ULONGLONG ullValueIn
    , ULONGLONG ullPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                       sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME     pName = NULL;
    PCLUSPROP_ULARGE_INTEGER    pValue = NULL;

    if ( m_fAlwaysAddProp || ( ullValueIn != ullPrevValueIn ) )
    {
        size_t  cbNameSize = 0;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbValueSize = sizeof( CLUSPROP_ULARGE_INTEGER )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pULargeIntegerValue;
            CopyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, ullValueIn );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddProp( ULONGLONG )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScAddProp
//
//  Routine Description:
//      Add a LONGLONG property to a property list if it has changed.
//
//  Arguments:
//      pwszNameIn        Name of the property.
//      llValueIn         Value of the property to set in the list.
//      llPrevValueIn     Previous value of the property.
//
//  Return Value:
//      ERROR_SUCCESS or other Win32 status codes.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScAddProp(
      LPCWSTR      pwszNameIn
    , LONGLONG     llValueIn
    , LONGLONG     llPrevValueIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                       sc = ERROR_SUCCESS;
    PCLUSPROP_PROPERTY_NAME     pName = NULL;
    PCLUSPROP_LARGE_INTEGER     pValue = NULL;

    if ( m_fAlwaysAddProp || ( llValueIn != llPrevValueIn ) )
    {
        size_t  cbNameSize = 0;
        size_t  cbValueSize = 0;

        //
        // Calculate sizes and make sure we have a property list.
        //

        cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                    + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

        cbValueSize = sizeof( CLUSPROP_LARGE_INTEGER )
                    + sizeof( CLUSPROP_SYNTAX ); // value list endmark

        sc = ScAllocPropList( cbNameSize + cbValueSize );
        if ( sc == ERROR_SUCCESS )
        {
            //
            // Set the property name.
            //

            pName = m_cbhCurrentProp.pName;
            CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
            m_cbhCurrentProp.pb += cbNameSize;

            //
            // Set the property value.
            //

            pValue = m_cbhCurrentProp.pLargeIntegerValue;
            CopyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, llValueIn );
            m_cbhCurrentProp.pb += cbValueSize;

            //
            // Increment the property count and buffer size.
            //

            m_cbhPropList.pList->nPropertyCount++;
            m_cbDataSize += cbNameSize + cbValueSize;
        } // if: ScAllocPropList successfully grew the proplist
    } // if: the value has changed

    return sc;

} //*** CClusPropList::ScAddProp( LONGLONG )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScSetPropToDefault
//
//  Description:
//      Add a property to the property list so that it will revert to its
//      default value.
//
//  Arguments:
//      pwszNameIn          Name of the property.
//      cpfPropFormatIn     Format of property
//
//  Return Value:
//      ERROR_SUCCESS or other Win32 status codes.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScSetPropToDefault(
      LPCWSTR                   pwszNameIn
    , CLUSTER_PROPERTY_FORMAT   cpfPropFormatIn
    )
{
    ASSERT( pwszNameIn != NULL );

    DWORD                   sc = ERROR_SUCCESS;
    size_t                  cbNameSize = 0;
    size_t                  cbValueSize = 0;
    PCLUSPROP_PROPERTY_NAME pName = NULL;
    PCLUSPROP_VALUE         pValue = NULL;

    //
    //  Calculate sizes and make sure we have a property list.
    //

    cbNameSize = sizeof( CLUSPROP_PROPERTY_NAME )
                + ALIGN_CLUSPROP( (lstrlenW( pwszNameIn ) + 1) * sizeof( *pwszNameIn ) );

    cbValueSize = sizeof( CLUSPROP_BINARY )
                + sizeof( CLUSPROP_SYNTAX ); // value list endmark

    sc = ScAllocPropList( cbNameSize + cbValueSize );
    if ( sc == ERROR_SUCCESS )
    {
        //
        // Set the property name.
        //

        pName = m_cbhCurrentProp.pName;
        CopyProp( pName, CLUSPROP_TYPE_NAME, pwszNameIn );
        m_cbhCurrentProp.pb += cbNameSize;

        //
        // Set the property value.
        //

        pValue = m_cbhCurrentProp.pValue;
        CopyEmptyProp( pValue, CLUSPROP_TYPE_LIST_VALUE, cpfPropFormatIn );
        m_cbhCurrentProp.pb += cbValueSize;

        //
        // Increment the property count and buffer size.
        //

        m_cbhPropList.pList->nPropertyCount++;
        m_cbDataSize += cbNameSize + cbValueSize;
    } // if:

    return sc;

} //*** CClusPropList::ScSetPropToDefault

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyProp
//
//  Description:
//      Copy a string property to a property structure.
//
//  Arguments:
//      ppropOut      Property structure to fill.
//      cptPropTypeIn Type of string.
//      pszIn         String to copy.
//      cbSzIn        Count of bytes in pwsz string.  If specified as 0,
//                    the the length will be determined by a call to strlen.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyProp(
      PCLUSPROP_SZ          ppropOut
    , CLUSTER_PROPERTY_TYPE cptPropTypeIn
    , LPCWSTR               pszIn
    , size_t                cbSzIn        // = 0
    )
{
    ASSERT( ppropOut != NULL );
    ASSERT( pszIn != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;
    HRESULT                 hr = S_OK;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_SZ;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );

    if ( cbSzIn == 0 )
    {
        cbSzIn = (lstrlenW( pszIn ) + 1) * sizeof( *pszIn );
    } // if: zero size specified

    ASSERT( cbSzIn == (lstrlenW( pszIn ) + 1) * sizeof( *pszIn ) );

    ppropOut->cbLength = static_cast< DWORD >( cbSzIn );

    hr = StringCbCopyW( ppropOut->sz, cbSzIn, pszIn );
    if ( SUCCEEDED( hr ) )
    {
        //
        // Set an endmark.
        //

        cbhProps.pStringValue = ppropOut;
        cbhProps.pb += sizeof( *cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbSzIn );
        cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    } // if:

} //*** CClusPropList::CopyProp

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyMultiSzProp
//
//  Description:
//      Copy a MULTI_SZ string property to a property structure.
//
//  Arguments:
//      ppropOut      Property structure to fill.
//      cptPropTypeIn Type of string.
//      pszIn         String to copy.
//      cbSzIn        Count of bytes in pszIn string.  If specified as 0,
//                    the the length will be determined by calls to strlen.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyMultiSzProp(
      PCLUSPROP_MULTI_SZ    ppropOut
    , CLUSTER_PROPERTY_TYPE cptPropTypeIn
    , LPCWSTR               pszIn
    , size_t                cbSzIn
    )
{
    ASSERT( ppropOut != NULL );
    ASSERT( pszIn != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_MULTI_SZ;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );

    if ( cbSzIn == 0 )
    {
        cbSzIn = (CchMultiSz( pszIn ) + 1) * sizeof( *pszIn );
    } // if: zero size specified

    ASSERT( cbSzIn == (CchMultiSz( pszIn ) + 1) * sizeof( *pszIn ) );

    ppropOut->cbLength = static_cast< DWORD >( cbSzIn );
    CopyMemory( ppropOut->sz, pszIn, cbSzIn );

    //
    // Set an endmark.
    //

    cbhProps.pMultiSzValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pMultiSzValue ) + ALIGN_CLUSPROP( cbSzIn );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyMultiSzProp

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyExpandSzProp
//
//  Description:
//      Copy an EXPAND_SZ string property to a property structure.
//
//  Arguments:
//      ppropOut      Property structure to fill.
//      cptPropTypeIn Type of string.
//      pszIn         String to copy.
//      cbSzIn        Count of bytes in pszIn string.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyExpandSzProp(
      PCLUSPROP_SZ          ppropOut
    , CLUSTER_PROPERTY_TYPE cptPropTypeIn
    , LPCWSTR               pszIn
    , size_t                cbSzIn
    )
{
    ASSERT( ppropOut != NULL );
    ASSERT( pszIn != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;
    HRESULT                 hr = S_OK;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_EXPAND_SZ;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );

    if ( cbSzIn == 0 )
    {
        cbSzIn = (lstrlenW( pszIn ) + 1) * sizeof( *pszIn );
    } // if: cbSzIn == 0

    ASSERT( cbSzIn == (lstrlenW( pszIn ) + 1) * sizeof( *pszIn ) );

    ppropOut->cbLength = static_cast< DWORD >( cbSzIn );

    hr = StringCbCopyW( ppropOut->sz, cbSzIn, pszIn );
    if ( SUCCEEDED( hr ) )
    {
        //
        // Set an endmark.
        //

        cbhProps.pStringValue = ppropOut;
        cbhProps.pb += sizeof( *cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbSzIn );
        cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;
    } // if:

} //*** CClusPropList::CopyExpandSzProp

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyProp
//
//  Description:
//      Copy a DWORD property to a property structure.
//
//  Arguments:
//      ppropOut      Property structure to fill.
//      cptPropTypeIn Type of DWORD.
//      nValueIn      Property value to copy.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyProp(
      PCLUSPROP_DWORD       ppropOut
    , CLUSTER_PROPERTY_TYPE cptPropTypeIn
    , DWORD                 nValueIn
    )
{
    ASSERT( ppropOut != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_DWORD;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );
    ppropOut->cbLength = sizeof( DWORD );
    ppropOut->dw = nValueIn;

    //
    // Set an endmark.
    //

    cbhProps.pDwordValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pDwordValue );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyProp( DWORD )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyProp
//
//  Description:
//      Copy a LONG property to a property structure.
//
//  Arguments:
//      ppropOut      Property structure to fill.
//      cptPropTypeIn Type of LONG.
//      nValueIn      Property value to copy.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyProp(
      PCLUSPROP_LONG        ppropOut
    , CLUSTER_PROPERTY_TYPE cptPropTypeIn
    , LONG                  nValueIn
    )
{
    ASSERT( ppropOut != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_LONG;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );
    ppropOut->cbLength = sizeof( DWORD );
    ppropOut->l = nValueIn;

    //
    // Set an endmark.
    //

    cbhProps.pLongValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pLongValue );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyProp( LONG )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyProp
//
//  Description:
//      Copy a ULONGLONG property to a property structure.
//
//  Arguments:
//      ppropOut        Property structure to fill.
//      cptPropTypeIn   Type of LONG.
//      ullValueIn      Property value to copy.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyProp(
      PCLUSPROP_ULARGE_INTEGER  ppropOut
    , CLUSTER_PROPERTY_TYPE     cptPropTypeIn
    , ULONGLONG                 ullValueIn
    )
{
    ASSERT( ppropOut != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_ULARGE_INTEGER;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );
    ppropOut->cbLength = sizeof( ULONGLONG );

    //
    // ppropOut may not have the correct alignment for large ints; copy as two
    // DWORDs to be safe
    //

    ppropOut->li.u = ((ULARGE_INTEGER *)&ullValueIn)->u;

    //
    // Set an endmark.
    //

    cbhProps.pULargeIntegerValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pULargeIntegerValue );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyProp( ULONGLONG )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyProp
//
//  Description:
//      Copy a LONGLONG property to a property structure.
//
//  Arguments:
//      ppropOut        Property structure to fill.
//      cptPropTypeIn   Type of LONG.
//      llValueIn       Property value to copy.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyProp(
      PCLUSPROP_LARGE_INTEGER   ppropOut
    , CLUSTER_PROPERTY_TYPE     cptPropTypeIn
    , LONGLONG                  llValueIn
    )
{
    ASSERT( ppropOut != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_LARGE_INTEGER;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );
    ppropOut->cbLength = sizeof( LONGLONG );

    //
    // ppropOut may not have the correct alignment for large ints; copy as two
    // DWORDs to be safe
    //

    ppropOut->li.u = ((LARGE_INTEGER *)&llValueIn)->u;

    //
    // Set an endmark.
    //

    cbhProps.pLargeIntegerValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pLargeIntegerValue );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyProp( LONGLONG )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyProp
//
//  Description:
//      Copy a binary property to a property structure.
//
//  Arguments:
//      ppropOut        Property structure to fill.
//      cptPropTypeIn   Type of string.
//      pbValueIn       Block to copy.
//      cbValueIn       Count of bytes in pb buffer.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyProp(
      PCLUSPROP_BINARY      ppropOut
    , CLUSTER_PROPERTY_TYPE cptPropTypeIn
    , const unsigned char * pbValueIn
    , size_t                cbValueIn
    )
{
    ASSERT( ppropOut != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = CLUSPROP_FORMAT_BINARY;
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );
    ppropOut->cbLength = static_cast< DWORD >( cbValueIn );

    if ( cbValueIn > 0 )
    {
        CopyMemory( ppropOut->rgb, pbValueIn, cbValueIn );
    } // if: non-zero data length

    //
    // Set an endmark.
    //

    cbhProps.pBinaryValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pStringValue ) + ALIGN_CLUSPROP( cbValueIn );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyProp( PBYTE )

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::CopyEmptyProp
//
//  Description:
//      Copy an empty property to a property structure.
//
//  Arguments:
//      ppropOut            Property structure to fill.
//      cptPropTypeIn       Type of property.
//      cpfPropFormatIn     Format of property.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
void
CClusPropList::CopyEmptyProp(
      PCLUSPROP_VALUE           ppropOut
    , CLUSTER_PROPERTY_TYPE     cptPropTypeIn
    , CLUSTER_PROPERTY_FORMAT   cptPropFmt
    )
{
    ASSERT( ppropOut != NULL );

    CLUSPROP_BUFFER_HELPER  cbhProps;

    ppropOut->Syntax.wFormat = static_cast< WORD >( cptPropFmt );
    ppropOut->Syntax.wType = static_cast< WORD >( cptPropTypeIn );
    ppropOut->cbLength = 0;

    //
    // Set an endmark.
    //

    cbhProps.pValue = ppropOut;
    cbhProps.pb += sizeof( *cbhProps.pValue );
    cbhProps.pSyntax->dw = CLUSPROP_SYNTAX_ENDMARK;

} //*** CClusPropList::CopyEmptyProp

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetNodeProperties
//
//  Description:
//      Get properties on a node.
//
//  Arguments:
//      hNodeIn           Handle for the node to get properties from.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetNodeProperties(
      HNODE     hNodeIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn              // = NULL
    , size_t    cbBufferIn              // = 0
    )
{
    ASSERT( hNodeIn != NULL );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_NODE << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //

    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterNodeControl(
                          hNodeIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterNodeControl(
                                  hNodeIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetNodeProperties

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetGroupProperties
//
//  Description:
//      Get properties on a group.
//
//  Arguments:
//      hGroupIn          Handle for the group to get properties from.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetGroupProperties(
      HGROUP    hGroupIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn          // = NULL
    , size_t    cbBufferIn          // = 0
    )
{
    ASSERT( hGroupIn != NULL );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_GROUP << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //

    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterGroupControl(
                          hGroupIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterGroupControl(
                                  hGroupIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetGroupProperties

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetResourceProperties
//
//  Description:
//      Get properties on a resource.
//
//  Arguments:
//      hResourceIn       Handle for the resource to get properties from.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetResourceProperties(
      HRESOURCE hResourceIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn          // = NULL
    , size_t    cbBufferIn          // = 0
    )
{
    ASSERT( hResourceIn != NULL );
    ASSERT( ( dwControlCodeIn & (CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_RESOURCE << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //

    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterResourceControl(
                          hResourceIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterResourceControl(
                                  hResourceIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetResourceProperties

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetResourceTypeProperties
//
//  Description:
//      Get properties on a resource type.
//
//  Arguments:
//      hClusterIn        Handle for the cluster in which the resource
//                        type resides.
//      pwszResTypeNameIn Name of the resource type.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetResourceTypeProperties(
      HCLUSTER  hClusterIn
    , LPCWSTR   pwszResTypeNameIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn          // = NULL
    , size_t    cbBufferIn          // = 0
    )
{
    ASSERT( hClusterIn != NULL );
    ASSERT( pwszResTypeNameIn != NULL );
    ASSERT( *pwszResTypeNameIn != L'\0' );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_RESOURCE_TYPE << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //

    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterResourceTypeControl(
                          hClusterIn
                        , pwszResTypeNameIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterResourceTypeControl(
                                  hClusterIn
                                , pwszResTypeNameIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetResourceTypeProperties

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetNetworkProperties
//
//  Description:
//      Get properties on a network.
//
//  Arguments:
//      hNetworkIn        Handle for the network to get properties from.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetNetworkProperties(
      HNETWORK  hNetworkIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn              // = NULL
    , size_t    cbBufferIn              // = 0
    )
{
    ASSERT( hNetworkIn != NULL );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_NETWORK << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //
    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterNetworkControl(
                          hNetworkIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterNetworkControl(
                                  hNetworkIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting private properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetNetworkProperties

/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetNetInterfaceProperties
//
//  Description:
//      Get properties on a network interface.
//
//  Arguments:
//      hNetInterfaceIn   Handle for the network interface to get properties from.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetNetInterfaceProperties(
      HNETINTERFACE hNetInterfaceIn
    , DWORD         dwControlCodeIn
    , HNODE         hHostNodeIn
    , LPVOID        lpBufferIn              // = NULL
    , size_t        cbBufferIn              // = 0
    )
{
    ASSERT( hNetInterfaceIn != NULL );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_NETINTERFACE << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //

    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterNetInterfaceControl(
                          hNetInterfaceIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterNetInterfaceControl(
                                  hNetInterfaceIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting private properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetNetInterfaceProperties


/////////////////////////////////////////////////////////////////////////////
//++
//
//  CClusPropList::ScGetClusterProperties
//
//  Description:
//      Get properties on a cluster.
//
//  Arguments:
//      hClusterIn        Handle for the cluster to get properties from.
//      dwControlCodeIn   Control code for the request.
//      hHostNodeIn       Handle for the node to direct this request to.
//                        Defaults to NULL.
//      lpBufferIn        Input buffer for the request.  Defaults to NULL.
//      cbBufferIn        Size of the input buffer.  Defaults to 0.
//
//  Return Value:
//      None.
//
//--
/////////////////////////////////////////////////////////////////////////////
DWORD
CClusPropList::ScGetClusterProperties(
      HCLUSTER  hClusterIn
    , DWORD     dwControlCodeIn
    , HNODE     hHostNodeIn
    , LPVOID    lpBufferIn              // = NULL
    , size_t    cbBufferIn              // = 0
    )
{
    ASSERT( hClusterIn != NULL );
    ASSERT( ( dwControlCodeIn & ( CLUSCTL_OBJECT_MASK << CLUSCTL_OBJECT_SHIFT ) )
            == ( CLUS_OBJECT_CLUSTER << CLUSCTL_OBJECT_SHIFT ) );

    DWORD   sc = ERROR_SUCCESS;
    DWORD   cbProps = 256;

    //
    // Overwrite anything that may be in the buffer.
    // Allows this class instance to be reused.
    //

    m_cbDataSize = 0;

    //
    // Get properties.
    //

    sc = ScAllocPropList( cbProps );
    if ( sc == ERROR_SUCCESS )
    {
        sc = ClusterControl(
                          hClusterIn
                        , hHostNodeIn
                        , dwControlCodeIn
                        , lpBufferIn
                        , static_cast< DWORD >( cbBufferIn )
                        , m_cbhPropList.pb
                        , static_cast< DWORD >( m_cbBufferSize )
                        , &cbProps
                        );
        if ( sc == ERROR_MORE_DATA )
        {
            sc = ScAllocPropList( cbProps );
            if ( sc == ERROR_SUCCESS )
            {
                sc = ClusterControl(
                                  hClusterIn
                                , hHostNodeIn
                                , dwControlCodeIn
                                , lpBufferIn
                                , static_cast< DWORD >( cbBufferIn )
                                , m_cbhPropList.pb
                                , static_cast< DWORD >( m_cbBufferSize )
                                , &cbProps
                                );
            } // if: ScAllocPropList succeeded
        } // if: buffer too small
    } // if: ScAllocPropList succeeded

    if ( sc != ERROR_SUCCESS )
    {
        DeletePropList();
    } // if: error getting private properties.
    else
    {
        m_cbDataSize = static_cast< size_t >( cbProps );
        m_cbDataLeft = static_cast< size_t >( cbProps );
    } // else: no errors

    return sc;

} //*** CClusPropList::ScGetClusterProperties
