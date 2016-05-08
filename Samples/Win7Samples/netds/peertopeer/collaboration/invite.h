/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:

    This C file includes sample code for sending an invitation
    with the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

Notes:
    * This peer to peer application requires global IPv6 connectivity.

    * This sample relies on having existing contacts who are actively
      signed in.  This sample does not offer the ability to add new
      contacts.  To do this, the Contacts samples may be run.

--********************************************************************/

//Calls PeerCollabRegisterApplication
void RegisterApplication();

//Calls PeerCollabUnregisterApplication
void UnregisterApplication();

//Calls PeerCollabEnumApplicationRegistrationInfo
void DisplayApplicationRegistrations();

//Calls PeerCollabInviteContact, PeerCollabInviteEndpoint,
//      PeerCollabAsyncInviteContact, PeerCollabAsyncInviteEndpoint, PeerCollabCloseHandle,
//      PeerCollabGetInvitationResponse and PeerCollabCancelInvitation
void SendInvite();

//Calls PeerCollabGetAppLaunchInfo
void DisplayAppLauchInfo();

