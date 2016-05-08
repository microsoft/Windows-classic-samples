//+-------------------------------------------------------------------------
//
//  Microsoft Windows Media Technologies
//  Copyright (C) Microsoft Corporation. All rights reserved.
//
//  File:       AuthenticateContext.cpp
//
//  Contents:
//
//--------------------------------------------------------------------------


#include "stdafx.h"
#include "Authenticate.h"
#include "AuthenticatePlugin.h"
#include "AuthenticateContext.h"

//
// TODO: Change the realm name here
//
#define REALM_NAME      L"MyRealm"

BOOL base64Decode
(
    WCHAR*  pBufEncoded,
    DWORD   BufEncodedLen,
    BYTE*   pBufDecoded,
    DWORD*  pBufDecodedLen
);

/////////////////////////////////////////////////////////////////////////////
// CAuthenticateContext
///////////////////////////////////////////////////////////////////////////////

CAuthenticateContext::CAuthenticateContext() :
    m_hToken( NULL ),
    m_bstrUsername( L"" ),
    m_State( 0 )
{
    InitializeCriticalSection( &m_CritSec );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CAuthenticateContext::~CAuthenticateContext()
{
    DeleteCriticalSection( &m_CritSec );

    if( NULL != m_hToken )
    {
        CloseHandle( m_hToken );
    }

    if( NULL != m_pWMSAuthenticationPlugin )
    {
        m_pWMSAuthenticationPlugin->Release();
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticateContext::Initialize
(
    IWMSAuthenticationPlugin* pAuthenticator
)
{
    if( NULL == pAuthenticator)
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    EnterCriticalSection( &m_CritSec );

    m_pWMSAuthenticationPlugin = pAuthenticator;
    m_pWMSAuthenticationPlugin->AddRef();

    LeaveCriticalSection( &m_CritSec );

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticateContext::GetAuthenticationPlugin
(
    IWMSAuthenticationPlugin **ppAuthenPlugin
)
{
    if( NULL == ppAuthenPlugin)
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;

    EnterCriticalSection( &m_CritSec );

    do
    {
        if( NULL == m_pWMSAuthenticationPlugin )
        {
            hr = E_UNEXPECTED;
            break;
        }

        hr = m_pWMSAuthenticationPlugin->QueryInterface( IID_IWMSAuthenticationPlugin, (void**)ppAuthenPlugin );

    }
    while( FALSE );

    LeaveCriticalSection( &m_CritSec );

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticateContext::Authenticate
(
    VARIANT ResponseBlob,
    IWMSContext *pUserCtx,
    IWMSContext *pPresentationCtx,
    IWMSCommandContext *pCommandContext,
    IWMSAuthenticationCallback *pCallback,
    VARIANT Context
)
{

    if( NULL == pCallback )
    {
        return(E_INVALIDARG);
    }

    CSafeArrayOfBytes response( &ResponseBlob );

    if( !response.HasValidData() )
    {
        return( E_INVALIDARG );
    }

    HRESULT hr = S_OK;
    EnterCriticalSection( &m_CritSec );

    DWORD decodeLen = 0;
    DWORD dwState = WMS_AUTHENTICATION_ERROR;

    char* pszCredBuf = NULL;

    VARIANT ChallengeBlob;
    VariantInit( &ChallengeBlob );

    do
    {
        if( 0 == response.GetLength() )
        {

            //
            // empty response. set state to WMS_AUTHENTICATION_CONTINUE and send challenge
            //

            dwState = WMS_AUTHENTICATION_CONTINUE;

            WCHAR wszChallenge[ sizeof( REALM_NAME ) / sizeof( WCHAR ) + 9 ];
            _snwprintf_s( wszChallenge,sizeof( REALM_NAME ) / sizeof( WCHAR ) + 9 , sizeof( REALM_NAME ) / sizeof( WCHAR ) + 9, L"realm=\"%s\"", REALM_NAME );
            wszChallenge[ sizeof( REALM_NAME ) / sizeof( WCHAR ) + 8 ] = L'\0';
            CSafeArrayOfBytes challenge( &ChallengeBlob );
            hr = challenge.SetData( ( BYTE* ) wszChallenge, DWORD(sizeof( WCHAR ) * wcslen( wszChallenge ) ));

            break;
        }

        //
        // add 1 to store the extra '\0' at the end later
        //

        decodeLen = base64DecodeNeedLength( response.GetLength() / sizeof(WCHAR) ) + 1;
        pszCredBuf = new char[ decodeLen ];
        if( NULL == pszCredBuf)
        {
            hr = E_OUTOFMEMORY;
            break;
        }

        //
        // decode the user token
        //

        if( !base64Decode( (WCHAR*) response.GetData(), response.GetLength() / sizeof(WCHAR),
                           (BYTE*) pszCredBuf, &decodeLen ) )
        {
            hr = E_FAIL;
            break;
        }

        //
        // NULL terminate the decoded user credential
        //

        *( pszCredBuf + decodeLen ) = '\0';

        //
        // scan for the password
        //

        char* pszUserName = pszCredBuf;
        char* pszPassword = strchr( pszUserName, ':' );

        if( NULL == pszPassword )
        {
            pszPassword = pszUserName + strlen( pszUserName );
        }
        else
        {
            *pszPassword = '\0';
            pszPassword++;
        }

        //
        // now verify username and password
        //
        if( IsValidUser( pszUserName, pszPassword ) )
        {
            dwState = WMS_AUTHENTICATION_SUCCESS;
            m_bstrUsername = pszUserName;
        }
        else
        {
            dwState = WMS_AUTHENTICATION_DENIED;
            m_bstrUsername = L"";
        }

        //
        // TODO: Add code here to get the Impersonation Token
        // m_hToken = ...
        //

    } while( FALSE );

    if( SUCCEEDED( hr ) )
    {
        m_State = dwState;
    }

    LeaveCriticalSection( &m_CritSec );

    if( SUCCEEDED( hr ) )
    {
        pCallback->OnAuthenticateComplete( (WMS_AUTHENTICATION_RESULT)dwState, ChallengeBlob, Context );
    }

    if( NULL != pszCredBuf )
    {
        delete [] pszCredBuf;
        pszCredBuf = NULL;
    }

    VariantClear( &ChallengeBlob );

    return( hr );

}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL CAuthenticateContext::IsValidUser
(
    char* pszUserName,
    char* pszPassword
)
{
    BOOL fSucceeded = FALSE;

    //
    // TODO: Add code here to verify the user credentials
    //
    // fSucceeded = ...

    return( fSucceeded );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticateContext::GetLogicalUserID
(
    BSTR* bstrUserID
)
{
    if( NULL == bstrUserID )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;
    EnterCriticalSection( &m_CritSec );

    if( WMS_AUTHENTICATION_SUCCESS == m_State )
    {
        *bstrUserID = SysAllocString( m_bstrUsername.m_str );

        if( NULL == *bstrUserID )
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *bstrUserID = NULL;
    }

    LeaveCriticalSection( &m_CritSec );

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticateContext::GetImpersonationAccountName
(
    BSTR* bstrAccountName
)
{
    if( NULL == bstrAccountName )
    {
        return( E_POINTER );
    }

    HRESULT hr = E_NOTIMPL;
    EnterCriticalSection( &m_CritSec );

    //
    // TODO: Add code here to return the Impersonation Account Name
    //
    // hr = ...

    LeaveCriticalSection( &m_CritSec );

    return( hr );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
STDMETHODIMP CAuthenticateContext::GetImpersonationToken
(
    long* Token
)
{
    if( NULL == Token )
    {
        return( E_POINTER );
    }

    HRESULT hr = S_OK;

    EnterCriticalSection( &m_CritSec );

    *Token = (long) m_hToken;

    LeaveCriticalSection( &m_CritSec );

    return( hr );
}


/////////////////////////////////////////////////////////////////////////////
// CSafeArrayOfBytes
///////////////////////////////////////////////////////////////////////////////

CSafeArrayOfBytes::CSafeArrayOfBytes
(
    VARIANT* pVariant
)
{
    m_pVariant = pVariant;
    if( ( VT_ARRAY | VT_UI1 ) == m_pVariant -> vt )
    {
        m_psaBlob = V_ARRAY( m_pVariant );
    }
    else
    {
        m_psaBlob = NULL;
    }
    m_dataPtr = NULL;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
CSafeArrayOfBytes::~CSafeArrayOfBytes()
{
    if( m_dataPtr )
    {
        SafeArrayUnaccessData( m_psaBlob );
    }
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL CSafeArrayOfBytes::HasValidData()
{
    return( NULL != m_psaBlob );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
DWORD CSafeArrayOfBytes::GetLength()
{
    if( NULL == m_psaBlob )
    {
        return( NULL );
    }
    return( m_psaBlob -> rgsabound[0].cElements );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BYTE* CSafeArrayOfBytes::GetData()
{
    if( NULL == m_psaBlob )
    {
        return( NULL );
    }
    if( !m_dataPtr )
    {
        SafeArrayAccessData( m_psaBlob, (void **) &m_dataPtr );
    }
    return( m_dataPtr );
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
HRESULT CSafeArrayOfBytes::SetData
(
    BYTE* blob,
    DWORD length
)
{
    if( m_dataPtr )
    {
        SafeArrayUnaccessData( m_psaBlob );
        m_dataPtr = NULL;
    }
        if( m_psaBlob )
    {
        SafeArrayDestroy( m_psaBlob );
    }

    SAFEARRAYBOUND rgsabound[1];
    rgsabound[0].lLbound = 0;
    rgsabound[0].cElements = length;
    m_psaBlob = SafeArrayCreate( VT_UI1, 1, rgsabound );
    if( NULL == m_psaBlob )
    {
        return( E_OUTOFMEMORY );
    }
    SafeArrayAccessData( m_psaBlob, (void **) &m_dataPtr );
    memcpy( (char*)m_dataPtr, blob, length );

    VariantInit( m_pVariant );
    V_VT( m_pVariant ) = VT_ARRAY | VT_UI1;
    V_ARRAY( m_pVariant ) = m_psaBlob;

    return( S_OK );
}

///////////////////////////////////////////////////////////////////////////////
//  base64 encode/decode functions
///////////////////////////////////////////////////////////////////////////////

static const int pr2six[256] = {
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,62,64,64,64,63,
    52,53,54,55,56,57,58,59,60,61,64,64,64,64,64,64,64,0,1,2,3,4,5,6,7,8,9,
    10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,64,64,64,64,64,64,26,27,
    28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,64,
    64,64,64,64,64,64,64,64,64,64,64,64,64
};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
BOOL base64Decode
(
    WCHAR*  pBufEncoded,
    DWORD   BufEncodedLen,
    BYTE*  pBufDecoded,
    DWORD* pBufDecodedLen
)
{
    WCHAR* bufin;
    BYTE* bufout;
    DWORD nbytesdecoded;
    int nprbytes;

    //
    // Encode buffer length (minus extra '\0' character at the end )
    // should be devisable by 4
    //

    if( ( BufEncodedLen - 1 ) % 4 != 0 )
    {
        return( FALSE );
    }

    //
    // Figure out how many characters are in the input buffer.
    // Check if this would decode into the output buffer.
    //

    bufin = pBufEncoded;
    nprbytes = 0;
    while( nprbytes < (int) (BufEncodedLen - 1) )
    {
        if( *bufin >= 256 )
        {
            return( FALSE ); // Non ANSI characters not valid
        }
        if( pr2six[ *bufin ] >= 64 )
        {
            break;
        }
        bufin++;
        ++nprbytes;
    }
    nbytesdecoded = base64DecodeNeedLength( nprbytes );

    //
    // Double check all the padding characters are '='.
    // Return FALSE otherwise.
    //

    WCHAR *pBufEnd = pBufEncoded + BufEncodedLen;
    while( ( bufin < pBufEnd ) && ( NULL != *bufin ) )
    {
        if( L'=' != *bufin )
        {
            return( FALSE );
        }
        bufin++;
    }

    if( *pBufDecodedLen < nbytesdecoded )
        return FALSE;

    //
    // Perform the decoding
    //

    bufin = pBufEncoded;
    bufout = pBufDecoded;

    while( nprbytes > 0 )
    {
        *(bufout++) =
            (BYTE) (pr2six[*bufin] << 2 | pr2six[bufin[1]] >> 4);
        *(bufout++) =
            (BYTE) (pr2six[bufin[1]] << 4 | pr2six[bufin[2]] >> 2);
        *(bufout++) =
            (BYTE) (pr2six[bufin[2]] << 6 | pr2six[bufin[3]]);
        bufin += 4;
        nprbytes -= 4;
    }

    //
    // Adjust if trailing characters are in invalid
    //

    if(nprbytes & 03) {
        if(pr2six[bufin[-2]] > 63)
            nbytesdecoded -= 2;
        else
            nbytesdecoded -= 1;
    }

    //
    // fill in the final length and return
    //
    *pBufDecodedLen = nbytesdecoded;

    return TRUE;
}
