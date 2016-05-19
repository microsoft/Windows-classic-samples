/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) Microsoft Corporation. All Rights Reserved.

Module Name:

    InvitationHelpers.h

Abstract:

    This C header file declares functions for use in the
    with the invitation helpers sample code.

--********************************************************************/


#pragma once

#include <p2p.h>

//Structures
//
typedef struct endpoint_invitation_context {
    PEER_ENDPOINT *pEndpoint; 
    PEER_INVITATION *pInvite;
} ENDPOINT_INVITATION_CONTEXT, * PENDPOINT_INVITATION_CONTEXT;

HRESULT SendInviteOnThread(ENDPOINT_INVITATION_CONTEXT * pInvitationContext);
VOID FreeInvitationContext(ENDPOINT_INVITATION_CONTEXT * pInvitationContext);
