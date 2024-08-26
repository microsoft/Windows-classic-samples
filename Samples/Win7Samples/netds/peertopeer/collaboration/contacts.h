/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for working with contacts using
    the Microsoft Peer-to-Peer Collaboration APIs.

Note:
    This peer to peer application requires global IPv6 connectivity.

--********************************************************************/

//Calls PeerCollabExportContact
void ExportContact();

//Calls PeerCollabParseContact
void ParseContact();

//Calls PeerCollabAddContact
void ImportContact();

//Calls PeerCollabDeleteContact
void DeleteContact();

//Calls PeerCollabEnumContacts
void EnumContacts();

//Calls PeerCollabGetContact, PeerCollabUpdateContact
void SetWatchPermissions();
void SetWatching();

