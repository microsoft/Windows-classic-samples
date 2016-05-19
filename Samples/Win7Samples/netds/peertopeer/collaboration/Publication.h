/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for enumerating people near me
    with the Microsoft Peer-to-Peer Collaboration APIs.

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/

#include "shared.h"

//Calls PeerCollabSubscribeEndpointData
void SubscribeEndpointData();

//Calls PeerCollabUnsubscribeEndpointData
void UnsubscribeEndpointData();

//Calls PeerCollabSetObject
void PublishEndpointObject();

//Calls PeerCollabDeleteObject
void DeleteEndpointObject();
