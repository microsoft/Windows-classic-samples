/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for working with contacts using
    the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Note:
    This peer to peer application requires global IPv6 connectivity.

--********************************************************************/

#pragma warning(disable:4201)   // nameless struct/union

#include <p2p.h>
#include <stdio.h>
#include <strsafe.h>
#include "SignIn.h"
#include "Shared.h"

//-----------------------------------------------------------------------------
// Function:    SignIn
// Purpose:     Routine to sign in to an Internet (serverless presence) or subnet ("People Near me")
//              peer-to-peer collaboration network
// Parameters:  None.
//
void SignIn()
{
    WCHAR wzBuff[INPUT_BUFSIZE];
    HRESULT hr = S_OK;
    PEER_SIGNIN_FLAGS signInFlags = 0;

    // Prompt for the sign in type
    //
    wprintf(L"Sign into (1) People Near Me, (2) Internet, (3) All [1-3]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    if  (wcsncmp(wzBuff, L"1", INPUT_BUFSIZE) == 0)
    {
        signInFlags = PEER_SIGNIN_NEAR_ME;
    }
    else if  (wcsncmp(wzBuff, L"2", INPUT_BUFSIZE) == 0)
    {
        signInFlags = PEER_SIGNIN_INTERNET;
    }
    else if  (wcsncmp(wzBuff, L"3", INPUT_BUFSIZE) == 0)
    {
        signInFlags = PEER_SIGNIN_ALL;
    }
    else
    {
        wprintf(L"Invalid selection.\n");
        return;
    }

    hr = PeerCollabSignin(NULL, signInFlags);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully signed in.\n");
    }
    else
    {
        wprintf(L"Sign in failed. HRESULT=0x%x.\n", hr);
        PrintError(hr);
    }

exit:
    return;
}


//-----------------------------------------------------------------------------
// Function:    SignOut
// Purpose:     Routine to sign out of a specific type of peer collaboration network
// Parameters:  None.
//
void SignOut()
{
    WCHAR wzBuff[INPUT_BUFSIZE];
    HRESULT hr = S_OK;
    PEER_SIGNIN_FLAGS signInFlags = 0;

    // Prompt for the sign in type
    //
    wprintf(L"Sign out of (1) People Near Me, (2) Internet, (3) All [1-3]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    if  (wcsncmp(wzBuff, L"1", INPUT_BUFSIZE) == 0)
    {
        signInFlags = PEER_SIGNIN_NEAR_ME;
    }
    else if  (wcsncmp(wzBuff, L"2", INPUT_BUFSIZE) == 0)
    {
        signInFlags = PEER_SIGNIN_INTERNET;
    }
    else if  (wcsncmp(wzBuff, L"3", INPUT_BUFSIZE) == 0)
    {
        signInFlags = PEER_SIGNIN_ALL;
    }
    else
    {
        wprintf(L"Invalid selection.\n");
        return;
    }

    hr = PeerCollabSignout(signInFlags);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully signed out.\n");
    }
    else
    {
        wprintf(L"Sign our failed. HRESULT=0x%x.\n", hr);
        PrintError(hr);
    }

exit:
    return;
}

//-----------------------------------------------------------------------------
// Function:    SignInOptions
// Purpose:     Routine to obtains the peer's current signed-in peer collaboration network presence options
// Parameters:  None.
//
void SignInOptions()
{
    HRESULT hr = S_OK;
    DWORD dwSigninOptions = 0;

    hr = PeerCollabGetSigninOptions(&dwSigninOptions);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Current Sign In options are: %s\n", PublicationScope(dwSigninOptions));
    }
    else
    {
        wprintf(L"PeerCollabGetSigninOptions failed. HRESULT=0x%x.\n", hr);
        PrintError(hr);
    }

}
