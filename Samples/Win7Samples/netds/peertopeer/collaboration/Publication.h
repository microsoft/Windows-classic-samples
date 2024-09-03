/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2007 Microsoft Corporation. All Rights Reserved.

Abstract:
    This C file includes sample code for enumerating people near me
    with the Microsoft Peer-to-Peer Collaboration APIs.

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
