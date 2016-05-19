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
#include "Contacts.h"
#include "Shared.h"

//-----------------------------------------------------------------------------
// Function:    ExportContact
// Purpose:     Routine to export the "me" contact to a user specified file.
// Parameters:  None.
//
void ExportContact()
{
    WCHAR wzPath[MAX_PATH];
    PWSTR pwzXML = NULL;
    HRESULT hr = S_OK;
    FILE *pFile = NULL;
    errno_t err = 0;

    // Obtain a filename from the user
    //
    wprintf(L"Enter the file to export to: ");
    GET_PROMPT_RESPONSE(hr, wzPath);

    // Get information about default contact
    //
    hr = PeerCollabExportContact(NULL, &pwzXML);

    if (FAILED(hr))
    {
        wprintf(L"Export failed. HRESULT=0x%x.\n", hr);
        PrintError(hr);
        goto exit;
    }

    // Write the information out to a user specified file
    //
    err = _wfopen_s(&pFile, wzPath, L"wb");
    if (err != 0)
    {
        wprintf(L"Failed to open file.\n");
        hr = E_FAIL;
        goto exit;
    }

    if (fputws(pwzXML, pFile) == WEOF)
    {
        wprintf(L"Error writing to file.\n");
        hr = E_FAIL;
        goto exit;
    }

exit:
    if (pFile)
    {
        fclose(pFile);
    }

    SAFE_PEER_FREE_DATA(pwzXML);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Contact successfully exported to %s\n", wzPath);
    }
}

//-----------------------------------------------------------------------------
// Function:    ParseContact
// Purpose:     Routine to parse and disply a contact from a user specified file
// Parameters:  None.
//
void ParseContact()
{
    HRESULT hr = S_OK;
    WCHAR wzPath[MAX_PATH];
    WCHAR wzContactInfo[MAX_CONTACT_INFO] = {0};
    PEER_CONTACT *pContact = NULL;
    FILE *pFile = NULL;
    errno_t err = 0;

    // Obtain a filename from the user
    //
    wprintf(L"Enter the file to parse from: ");
    GET_PROMPT_RESPONSE(hr, wzPath);

    err = _wfopen_s(&pFile, wzPath, L"rb");
    if (err != 0)
    {
        wprintf(L"Failed to open file.\n");
        hr = E_FAIL;
        goto exit;
    }

    if (fread(wzContactInfo, sizeof(WCHAR), MAX_CONTACT_INFO-1, pFile) == 0)
    {
        wprintf(L"Error reading from file.\n");
        hr = E_FAIL;
    }

    // Add the contact information to the local machine
    //
    hr = PeerCollabParseContact(wzContactInfo, &pContact);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Contact successfully parsed.\n");
        wprintf(L"\n");
        PrintContact(pContact);
    }
    else
    {
        wprintf(L"Parse failed (0x%x).\n", hr);
        PrintError(hr);
    }

exit:
    if (pFile)
    {
        fclose(pFile);
    }

    SAFE_PEER_FREE_DATA(pContact);
}


//-----------------------------------------------------------------------------
// Function:    ImportContact
// Purpose:     Routine to import a contact from a user specified file
// Parameters:  None.
//
void ImportContact()
{
    HRESULT hr = S_OK;
    WCHAR wzPath[MAX_PATH];
    WCHAR wzContactInfo[MAX_CONTACT_INFO] = {0};
    FILE *pFile = NULL;
    errno_t err = 0;

    // Obtain a filename from the user
    //
    wprintf(L"Enter the file to import from: ");
    GET_PROMPT_RESPONSE(hr, wzPath);

    err = _wfopen_s(&pFile, wzPath, L"rb");
    if (err != 0)
    {
        wprintf(L"Failed to open file.\n");
        hr = E_FAIL;
        goto exit;
    }

    if (fread(wzContactInfo, sizeof(WCHAR), MAX_CONTACT_INFO-1, pFile) == 0)
    {
        wprintf(L"Error reading from file.\n");
        hr = E_FAIL;
        goto exit;
    }

    // Add the contact information to the local machine
    //
    hr = PeerCollabAddContact(wzContactInfo, NULL);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Contact successfully imported.\n");
    }
    else
    {
        wprintf(L"Import failed (0x%x).\n", hr);
        PrintError(hr);
    }

exit:
    if (pFile)
    {
        fclose(pFile);
    }
}

//-----------------------------------------------------------------------------
// Function:    DeleteContact
// Purpose:     Routine to delete a selected contact
// Parameters:  None.
//
void DeleteContact()
{
    HRESULT hr = S_OK;
    PPEER_CONTACT pContact = NULL;

    // Retrieve the list of contacts
    //
    hr = SelectContact(&pContact, TRUE);

    if (FAILED(hr))
    {
        goto exit;
    }

    // Delete the contact
    //
    hr = PeerCollabDeleteContact(pContact->pwzPeerName);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully deleted contact.\n");
    }
    else
    {
        wprintf(L"Export failed with HRESULT=0x%x.\n", hr);
        PrintError(hr);
    }

exit:
    // Free the contact
    //
    SAFE_FREE(pContact);
}

//-----------------------------------------------------------------------------
// Function:    EnumContacts
// Purpose:     Enumerates and displays all contacts
// Parameters:  None
//
void EnumContacts()
{
    HRESULT hr = S_OK;
    PPEER_CONTACT pContact = NULL;

    // Print the list of contacts
    //
    hr = SelectContact(&pContact, FALSE);

    SAFE_FREE(pContact);
}

//-----------------------------------------------------------------------------
// Function:    SetWatchPermissions
// Purpose:     Routine to grant/deny a user specified contact permission to watch
//              for our presence changes
// Parameters:  None.
//
void SetWatchPermissions()
{
    HRESULT hr = S_OK;
    PPEER_CONTACT pContact = NULL;
    WCHAR wzBuff[5];
    int nWatchPermissions = 0;

    // Retrieve the list of contacts
    //
    hr = SelectContact(&pContact, TRUE);

    if (FAILED(hr))
    {
        goto exit;
    }

    // Prompt the user for the desired watch permission
    //
    wprintf(L"Select watch permissions (1) Allowed, (2) Blocked [1-2]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    // since wtoi can't distinguish between 0 and an error, note that valid values start at 1, not 0    
    nWatchPermissions = _wtoi(wzBuff);
    if (nWatchPermissions < 1  || nWatchPermissions > 2)
    {
        printf ("Invalid input, expected a number between 1 and 2.\n");
        hr = E_FAIL;
        goto exit;

    }

    // Set the watch permissions
    //
    if (nWatchPermissions == 1)
    {
        pContact->WatcherPermissions = TRUE;
    }
    else
    {
        pContact->WatcherPermissions = FALSE;
    }

    //Save the updates to the contact
    //
    hr = PeerCollabUpdateContact(pContact);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully updated contact.\n");
    }
    else
    {
        wprintf(L"Failed to update contact. HRESULT=0x%X.\n", hr);
        PrintError(hr);
    }

exit:
    SAFE_FREE(pContact);
}

//-----------------------------------------------------------------------------
// Function:    SetWatching
// Purpose:     Routine to start watching a user specified contact
// Parameters:  None.
//
void SetWatching()
{
    HRESULT hr = S_OK;
    PPEER_CONTACT pContact = NULL;
    WCHAR wzBuff[5];
    int nWatchState = 0;


    // Retrieve the list of contacts
    //
    hr = SelectContact(&pContact, TRUE);

    if (FAILED(hr))
    {
        goto exit;
    }

    // Prompt the user for the desired watch permission
    //
    wprintf(L"Select watch state (1) Watch, (2) Do not watch [1-2]: ");
    GET_PROMPT_RESPONSE(hr, wzBuff);

    // since wtoi can't distinguish between 0 and an error, note that valid values start at 1, not 0    
    nWatchState = _wtoi(wzBuff);
    if (nWatchState < 1  || nWatchState > 2)
    {
        printf ("Invalid input, expected a number between 1 and 2.\n");
        hr = E_FAIL;
        goto exit;

    }

    // Start watching them
    //
    if (nWatchState == 1)
    {
        pContact->fWatch = TRUE;
    }
    else
    {
        pContact->fWatch = FALSE;
    }

    //Save the updates to the contact
    //
    hr = PeerCollabUpdateContact(pContact);

    if (SUCCEEDED(hr))
    {
        wprintf(L"Successfully updated contact.\n");
    }
    else
    {
        wprintf(L"Failed to update contact. HRESULT=0x%X.\n", hr);
        PrintError(hr);
    }

exit:
    SAFE_FREE(pContact);
}
