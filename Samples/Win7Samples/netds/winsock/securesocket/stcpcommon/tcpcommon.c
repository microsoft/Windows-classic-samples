// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

/*++

Module Name:

    Tcpcommon.c

Abstract:

    This file contains common shared routines between the sample secure TCP
    client and secure TCP server. These routines mainly contain sample code for
    managing custom IPsec policy and querying IPsec SA information using the 
    FWP API.

--*/

#ifndef UNICODE
#define UNICODE
#endif

#include <wchar.h>
#include <Winsock2.h>
#include <mstcpip.h>
#include <ws2tcpip.h>
#include "Tcpcommon.h"

DWORD
AddCustomIPsecPolicy(
   OUT HANDLE* fwpHandle,
   OUT GUID* qmPolicyKey
)
/*++

Routine Description:

    This routine creates a customized FWP IPsec quick mode policy and adds it to
    the FWP platform.

Arguments:

    fwpHandle - Pointer that will upon return contain the handle for the 
                FWP session created.

    qmPolicyKey - Pointer that will upon return contain the unique
                  identifier of the quick mode policy that was just added.

Return Value:

   Windows error code indicating the status of the operation, or NO_ERROR if 
   the operation succeeded.

--*/
{
   DWORD result = 0;
   IPSEC_TRANSPORT_POLICY0 qmPolicy = {0};
   IPSEC_PROPOSAL0 qmProposal = {0};
   IKEEXT_EM_POLICY0 emPolicy = {0};
   IKEEXT_AUTHENTICATION_METHOD0 emAuthMethod = {0};
   FWPM_PROVIDER_CONTEXT0 fwpProviderCtxt = {0};
   IPSEC_SA_TRANSFORM0 qmTransform = {0};
   IPSEC_AUTH_AND_CIPHER_TRANSFORM0 espAuthAndCipherTransform = {0};

   //-----------------------------------------
   // Construct the QM ESP algorithms (SHA1 for hash, AES_128 for encryption)
   espAuthAndCipherTransform.authTransform.authTransformId =
      IPSEC_AUTH_TRANSFORM_ID_HMAC_SHA_1_96;
   espAuthAndCipherTransform.cipherTransform.cipherTransformId =
      IPSEC_CIPHER_TRANSFORM_ID_AES_128;

   //-----------------------------------------
   // Construct the QM ESP policy transform for ESP auth + cipher
   qmTransform.ipsecTransformType = IPSEC_TRANSFORM_ESP_AUTH_AND_CIPHER;
   qmTransform.espAuthAndCipherTransform = &espAuthAndCipherTransform;

   //-----------------------------------------
   // Construct the QM policy proposal
   // QM lifetime 1 hr, 55GB,
   qmProposal.lifetime.lifetimeSeconds = 60 * 60;
   qmProposal.lifetime.lifetimeKilobytes = 55 * 1024;
   qmProposal.lifetime.lifetimePackets = 0x7fffffff;
   qmProposal.numSaTransforms = 1;
   qmProposal.saTransforms = &qmTransform;
   // No PFS
   qmProposal.pfsGroup = IPSEC_PFS_NONE;

   //-----------------------------------------
   // Construct the EM auth method as Kerberos
   emAuthMethod.authenticationMethodType = IKEEXT_KERBEROS;
   emAuthMethod.kerberosAuthentication.flags = 0;

   //-----------------------------------------
   // Construct the EM policy
   emPolicy.numAuthenticationMethods = 1;
   emPolicy.authenticationMethods = &emAuthMethod;
   // Turn on impersonation / user auth.
   emPolicy.initiatorImpersonationType = IKEEXT_IMPERSONATION_SOCKET_PRINCIPAL;

   //-----------------------------------------
   // Construct the QM policy
   qmPolicy.numIpsecProposals = 1;
   qmPolicy.ipsecProposals = &qmProposal;
   // Default value for ND clear timeout
   qmPolicy.ndAllowClearTimeoutSeconds = 10;
   // Default idle timeouts (5 min, 1 min)
   qmPolicy.saIdleTimeout.idleTimeoutSeconds = 5 * 60;
   qmPolicy.saIdleTimeout.idleTimeoutSecondsFailOver = 60;
   qmPolicy.emPolicy = &emPolicy;

   //-----------------------------------------
   // Construct the FWP provider context
   fwpProviderCtxt.displayData.name = L"Secure Socket App customized IPsec Policy";
   fwpProviderCtxt.displayData.description = L"Secure Socket App customized IPsec Policy";
   fwpProviderCtxt.type = FWPM_IPSEC_AUTHIP_QM_TRANSPORT_CONTEXT;
   fwpProviderCtxt.authipQmTransportPolicy = &qmPolicy;
   // Create a unique identifier
   result = UuidCreate(&fwpProviderCtxt.providerContextKey);
   if (result)
   {
      wprintf(L"UuidCreate returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Open a session handle to FWP
   result = FwpmEngineOpen0(
               NULL,
               RPC_C_AUTHN_WINNT,
               NULL,
               NULL,
               fwpHandle
            );
   if (result)
   {
      wprintf(L"FwpmEngineOpen0 returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Add the provider context to FWP
   result = FwpmProviderContextAdd0(
               *fwpHandle,
               &fwpProviderCtxt,
               NULL,
               NULL
            );
   if (result)
   {
      wprintf(L"FwpmProviderContextAdd0 returned error %ld\n", result);
      goto cleanup;
   }

   *qmPolicyKey = fwpProviderCtxt.providerContextKey;

cleanup:
   return result;
}

VOID
RemoveCustomIPsecPolicy(
   IN HANDLE fwpHandle,
   IN const GUID* qmPolicyKey
)
/*++

Routine Description:

     This routine removes the customized FWP IPsec quick mode policy from the 
     platform and closes the FWP session handle

Arguments:

    fwpHandle - Handle for the FWP session

    qmPolicyKey - Pointer to the unique identifier of the quick mode policy 
                  that was added to the platform

Return Value:

    None

--*/
{
   GUID zeroGuid = {0};

   if(fwpHandle)
   {
      if(memcmp(
            &zeroGuid,
            qmPolicyKey,
            sizeof(GUID)
         ) != 0)
      {
         FwpmProviderContextDeleteByKey0(
            fwpHandle,
            qmPolicyKey
            );
      }

      FwpmEngineClose0(fwpHandle);
   }
}

VOID
PrintQmSa(
   IN const IPSEC_SA_CONTEXT0* qmSa
)
/*++

Routine Description:

     This routine prints sample information from the QM SA for illustration 
     purposes

Arguments:

    qmSa - Pointer to the quick mode SA context

Return Value:

    None

--*/
{
   UINT32 i = 0;
   // Static array containing string description of various encryption 
   // algorithms
   static const wchar_t* const cipherString[] =
   {
      L"None",
      L"DES",
      L"3DES",
      L"AES-128",
      L"AES-192",
      L"AES-256"
   };
   // Static array containing string description of various hashing algorithms
   static const wchar_t* const hashString[] =
   {
      L"MD5",
      L"SHA1"
   };
   IPSEC_AUTH_TRANSFORM_ID0* authTransformId = NULL;
   IPSEC_CIPHER_TRANSFORM_ID0* cipherTransformId = NULL;
   IPSEC_SA0* sa = NULL;

   wprintf(L"QM SA context ID %I64d\n", qmSa->saContextId);

   //-----------------------------------------
   // Print all the SAs in the bundle
   for(i=0; i<qmSa->outboundSa->saBundle.numSAs; i++)
   {
      sa = &qmSa->outboundSa->saBundle.saList[i];

      // For each SA print information based on its type
      switch(sa->saTransformType)
      {
         case IPSEC_TRANSFORM_AH:
         {
            // Print AH hash algorithm
            authTransformId = 
               &sa->ahInformation->authTransform.authTransformId;
            if(authTransformId->authType < RTL_NUMBER_OF(hashString))
            {
               wprintf(L"QM SA AH: Hash %s\n", hashString[authTransformId->authType]);
            }
            break;
         }
         case IPSEC_TRANSFORM_ESP_AUTH:
         {
            // Print ESP hash algorithm
            authTransformId = 
               &sa->espAuthInformation->authTransform.authTransformId;
            if(authTransformId->authType < RTL_NUMBER_OF(hashString))
            {
               wprintf(L"QM SA ESP: Hash %s\n", hashString[authTransformId->authType]);
            }
            break;
         }
         case IPSEC_TRANSFORM_ESP_CIPHER:
         {
            // Print ESP encryption algorithm
            cipherTransformId = 
               &sa->espCipherInformation->cipherTransform.cipherTransformId;
            if(cipherTransformId->cipherType < RTL_NUMBER_OF(cipherString))
            {
               wprintf(L"QM SA ESP: Encryption %s\n", cipherString[cipherTransformId->cipherType]);
            }
            break;
         }
         case IPSEC_TRANSFORM_ESP_AUTH_AND_CIPHER:
         {
            // Print ESP hash & encryption algorithm
            authTransformId = 
               &sa->espAuthAndCipherInformation->saAuthInformation.authTransform.authTransformId;
            cipherTransformId = 
               &sa->espAuthAndCipherInformation->saCipherInformation.cipherTransform.cipherTransformId;
            if((authTransformId->authType < RTL_NUMBER_OF(hashString)) &&
               (cipherTransformId->cipherType < RTL_NUMBER_OF(cipherString)))
            {
               wprintf(
                  L"QM SA ESP: Hash %s, Encryption %s\n",
                  hashString[authTransformId->authType],
                  cipherString[cipherTransformId->cipherType]
                  );
            }
            break;
         }
         default:
         {
            break;
         }
      }
   }
}

VOID
PrintMmSa(
   IN const IKEEXT_SA_DETAILS0* mmSa
)
/*++

Routine Description:

     This routine prints sample information from the MM SA for illustration 
     purposes

Arguments:

    mmSa - Pointer to the main mode SA

Return Value:

    None

--*/
{
   // Static array containing string description of various encryption 
   // algorithms
   static const wchar_t* const cipherString[] =
   {
      L"DES",
      L"3DES",
      L"AES-128",
      L"AES-192",
      L"AES-256"
   };
   // Static array containing string description of various hashing algorithms
   static const wchar_t* const hashString[] =
   {
      L"MD5",
      L"SHA1"
   };
   // Static array containing string description of various DH algorithms
   static const wchar_t* const dhString[] =
   {
      L"None",
      L"DH_1",
      L"DH_2",
      L"DH_2048",
      L"DH_ECP_256",
      L"DH_ECP_384"
   };
   // Static array containing string description of various authentication 
   // methods
   static const wchar_t* const authString[] =
   {
      L"Preshared Key",
      L"Certificate",
      L"Kerberos",
      L"Anonymous",
      L"SSL",
      L"NTLMv2",
      L"CGA"
   };

   //-----------------------------------------
   // Print various pieces of information from the MM SA

   // Print MM SA ID
   wprintf(L"MM SA context ID %I64d\n", mmSa->saId);

   // Print MM SA hash, encryption algorithms
   if((mmSa->ikeProposal.integrityAlgorithm.algoIdentifier < 
         RTL_NUMBER_OF(hashString)) &&
      (mmSa->ikeProposal.cipherAlgorithm.algoIdentifier < 
         RTL_NUMBER_OF(cipherString)))
   {
      wprintf(
         L"MM SA Hash %s, Encryption %s\n", 
         hashString[mmSa->ikeProposal.integrityAlgorithm.algoIdentifier],
         cipherString[mmSa->ikeProposal.cipherAlgorithm.algoIdentifier]
         );
   }

   // Print MM SA DH algorithm
   if(mmSa->ikeProposal.dhGroup < RTL_NUMBER_OF(dhString))
   {
      wprintf(L"MM SA DH group %s\n", dhString[mmSa->ikeProposal.dhGroup]);
   }

   // Print MM SA auth method
   if(mmSa->ikeCredentials.credentials[0].localCredentials.authenticationMethodType < 
      RTL_NUMBER_OF(authString))
   {
      wprintf(
         L"MM SA Auth method %s\n", 
         authString[
            mmSa->ikeCredentials.credentials[0].localCredentials.authenticationMethodType]
      );
   }

   // Print EM auth method (if it exists)
   if((mmSa->ikeCredentials.numCredentials > 1) &&
      (mmSa->ikeCredentials.credentials[1].localCredentials.authenticationMethodType < 
      RTL_NUMBER_OF(authString)))
   {
      wprintf(
         L"MM SA EM Auth method %s\n", 
         authString[
            mmSa->ikeCredentials.credentials[1].localCredentials.authenticationMethodType]
      );
   }
}

BOOL
IsQmSAExactMatch(
   IN const IPSEC_SA_CONTEXT0* qmSa,
   IN const FWPM_FILTER0* matchingFwpFilter,
   IN UINT16 peerPort
)
/*++

Routine Description:

   This routine verifies if a quick mode SA matches the given paramters

Arguments:

   qmSa - Pointer to the quick mode SA context
   matchingFwpFilter - Pointer to the outbound transport layer filter that 
                       matched the application traffic
   peerPort - Port corresponding to the peer application

Return Value:

   TRUE if the QM SA matches the parameters, FALSE if it does not match.

--*/
{
   if(!qmSa->outboundSa)
   {
      return FALSE;
   }
   if(qmSa->outboundSa->transportFilter->filterId != 
      matchingFwpFilter->filterId)
   {
      // The filter IDs dont match.
      return FALSE;
   }
   if(qmSa->outboundSa->traffic.remotePort && 
      (qmSa->outboundSa->traffic.remotePort != peerPort))
   {
      // The remote ports dont match.
      return FALSE;
   }

   // SA matches our criteria
   return TRUE;
}

DWORD
MatchIPsecSAsForConnection(
   IN const SOCKADDR_STORAGE* localAddress,
   IN const SOCKADDR_STORAGE* peerAddress
)
/*++

Routine Description:

   This routine finds the IPsec SAs (MM & QM) that match a given connection 
   5-tuple (Local address, Remote address, Local port, Remote port, TCP)

Arguments:

   localAddress - Pointer to the socket address that contains local 
                  address, port information
   peerAddress - Pointer to the socket address that contains peer 
                  address, port information

Return Value:

   Windows error code indicating the status of the operation, or NO_ERROR if 
   the operation succeeded.

--*/
{
   DWORD result = 0;
   FWPM_FILTER_CONDITION0 filterConditions[6] = {0};
   HANDLE fwpHandle = NULL;
   FWPM_FILTER_ENUM_TEMPLATE0 filterEnumTemplate = {0};
   HANDLE filterEnumHandle = NULL;
   FWPM_FILTER0** matchingFwpFilter=NULL;
   UINT32 numEntriesReturned=0;
   IPSEC_SA_CONTEXT_ENUM_TEMPLATE0 saEnumTemplate = {0};
   HANDLE saEnumHandle = NULL;
   IPSEC_SA_CONTEXT0** qmSaList = NULL;
   IPSEC_SA_CONTEXT0* matchingQmSa = NULL;
   IKEEXT_SA_DETAILS0* matchingMmSa = NULL;
   UINT32 i = 0;
   const GUID* layerKey = NULL;
   UINT16 localPort = 0, remotePort = 0;

   //-----------------------------------------
   // Open a session handle to FWP
   result = FwpmEngineOpen0(
               NULL,
               RPC_C_AUTHN_WINNT,
               NULL,
               NULL,
               &fwpHandle
            );
   if (result)
   {
      wprintf(L"FwpmEngineOpen0 returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Construct the FWP outbound IPsec transport layer filter enum template
   if(localAddress->ss_family == AF_INET)
   {
      filterConditions[0].fieldKey = FWPM_CONDITION_IP_LOCAL_ADDRESS;
      filterConditions[0].matchType = FWP_MATCH_EQUAL;
      filterConditions[0].conditionValue.type = FWP_UINT32;
      filterConditions[0].conditionValue.uint32 = 
         ntohl(((struct sockaddr_in*)localAddress)->sin_addr.s_addr);
      
      filterConditions[1].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
      filterConditions[1].matchType = FWP_MATCH_EQUAL;
      filterConditions[1].conditionValue.type = FWP_UINT32;
      filterConditions[1].conditionValue.uint32 = 
         ntohl(((struct sockaddr_in*)peerAddress)->sin_addr.s_addr);

      layerKey = &FWPM_LAYER_OUTBOUND_TRANSPORT_V4;
      localPort = ntohs(((struct sockaddr_in*)localAddress)->sin_port);
      remotePort = ntohs(((struct sockaddr_in*)peerAddress)->sin_port);
   }
   else
   {
      // Assume AF_INET6
      filterConditions[0].fieldKey = FWPM_CONDITION_IP_LOCAL_ADDRESS;
      filterConditions[0].matchType = FWP_MATCH_EQUAL;
      filterConditions[0].conditionValue.type = FWP_BYTE_ARRAY16_TYPE;
      filterConditions[0].conditionValue.byteArray16 = 
         (FWP_BYTE_ARRAY16*)&(((struct sockaddr_in6*)localAddress)->sin6_addr);
      
      filterConditions[1].fieldKey = FWPM_CONDITION_IP_REMOTE_ADDRESS;
      filterConditions[1].matchType = FWP_MATCH_EQUAL;
      filterConditions[1].conditionValue.type = FWP_BYTE_ARRAY16_TYPE;
      filterConditions[1].conditionValue.byteArray16 = 
         (FWP_BYTE_ARRAY16*)&(((struct sockaddr_in6*)peerAddress)->sin6_addr);

      layerKey = &FWPM_LAYER_OUTBOUND_TRANSPORT_V6;
      localPort = ntohs(((struct sockaddr_in6*)localAddress)->sin6_port);
      remotePort = ntohs(((struct sockaddr_in6*)peerAddress)->sin6_port);
   }

   filterConditions[2].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
   filterConditions[2].matchType = FWP_MATCH_EQUAL;
   filterConditions[2].conditionValue.type = FWP_UINT16;
   filterConditions[2].conditionValue.uint16 = localPort;

   filterConditions[3].fieldKey = FWPM_CONDITION_IP_REMOTE_PORT;
   filterConditions[3].matchType = FWP_MATCH_EQUAL;
   filterConditions[3].conditionValue.type = FWP_UINT16;
   filterConditions[3].conditionValue.uint16 = remotePort;

   filterConditions[4].fieldKey = FWPM_CONDITION_IP_PROTOCOL;
   filterConditions[4].matchType = FWP_MATCH_EQUAL;
   filterConditions[4].conditionValue.type = FWP_UINT8;
   filterConditions[4].conditionValue.uint8 = IPPROTO_TCP;

   filterConditions[5].fieldKey = FWPM_CONDITION_IP_LOCAL_ADDRESS_TYPE;
   filterConditions[5].matchType = FWP_MATCH_EQUAL;
   filterConditions[5].conditionValue.type = FWP_UINT8;
   filterConditions[5].conditionValue.uint8 = NlatUnicast;

   filterEnumTemplate.layerKey = *layerKey;
   filterEnumTemplate.enumType = FWP_FILTER_ENUM_FULLY_CONTAINED;
   filterEnumTemplate.flags= FWP_FILTER_ENUM_FLAG_BEST_TERMINATING_MATCH;
   filterEnumTemplate.numFilterConditions = RTL_NUMBER_OF(filterConditions);
   filterEnumTemplate.filterCondition = filterConditions;
   filterEnumTemplate.actionMask = FWP_ACTION_FLAG_TERMINATING;

   //-----------------------------------------
   // Enumerate and get the matching FWP outbound IPsec transport layer filter
   result = FwpmFilterCreateEnumHandle0(
               fwpHandle,
               &filterEnumTemplate,
               &filterEnumHandle
               );
   if (result)
   {
      wprintf(L"FwpmFilterCreateEnumHandle0 returned error %ld\n", result);
      goto cleanup;
   }
   result = FwpmFilterEnum0(
               fwpHandle,
               filterEnumHandle,
               1,
               &matchingFwpFilter,
               &numEntriesReturned
               );
   if (result)
   {
      wprintf(L"FwpmFilterEnum0 returned error %ld\n", result);
      goto cleanup;
   }
   // Verify the matching filter
   if((numEntriesReturned < 1) || 
      !(matchingFwpFilter[0]->action.type & FWP_ACTION_FLAG_CALLOUT) ||
      !((memcmp(
            &matchingFwpFilter[0]->action.calloutKey,
            &FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V4,
            sizeof(GUID)
         ) == 0) ||
        (memcmp(
            &matchingFwpFilter[0]->action.calloutKey,
            &FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V4,
            sizeof(GUID)
         ) == 0) ||
        (memcmp(
            &matchingFwpFilter[0]->action.calloutKey,
            &FWPM_CALLOUT_IPSEC_OUTBOUND_TRANSPORT_V6,
            sizeof(GUID)
         ) == 0) ||
        (memcmp(
            &matchingFwpFilter[0]->action.calloutKey,
            &FWPM_CALLOUT_IPSEC_OUTBOUND_TUNNEL_V6,
            sizeof(GUID)
         ) == 0)))
   {
      result = ERROR_INVALID_PARAMETER;
      wprintf(L"FwpmFilterEnum0 didn't return any matching filters\n");
      goto cleanup;
   }

   //-----------------------------------------
   // Construct IPsec QM SA enum template
   if(localAddress->ss_family == AF_INET)
   {
      saEnumTemplate.localSubNet.type = FWP_UINT32;
      saEnumTemplate.localSubNet.uint32 = 
         ntohl(((struct sockaddr_in*)localAddress)->sin_addr.s_addr);
      saEnumTemplate.remoteSubNet.type = FWP_UINT32;
      saEnumTemplate.remoteSubNet.uint32 = 
         ntohl(((struct sockaddr_in*)peerAddress)->sin_addr.s_addr);
   }
   else
   {
      saEnumTemplate.localSubNet.type = FWP_BYTE_ARRAY16_TYPE;
      saEnumTemplate.localSubNet.byteArray16 = 
         (FWP_BYTE_ARRAY16*)&(((struct sockaddr_in6*)localAddress)->sin6_addr);
      saEnumTemplate.remoteSubNet.type = FWP_BYTE_ARRAY16_TYPE;
      saEnumTemplate.remoteSubNet.byteArray16 = 
         (FWP_BYTE_ARRAY16*)&(((struct sockaddr_in6*)peerAddress)->sin6_addr);
   }

   //-----------------------------------------
   // Enumerate the IPsec QM SAs
   result = IPsecSaContextCreateEnumHandle0(
               fwpHandle,
               &saEnumTemplate,
               &saEnumHandle
               );
   if (result)
   {
      wprintf(L"IPsecSaContextCreateEnumHandle0 returned error %ld\n", result);
      goto cleanup;
   }
   result = IPsecSaContextEnum0(
               fwpHandle,
               saEnumHandle,
               UINT_MAX,
               &qmSaList,
               &numEntriesReturned
               );
   if (result)
   {
      wprintf(L"IPsecSaContextEnum0 returned error %ld\n", result);
      goto cleanup;
   }
   if(numEntriesReturned < 1)
   {
      result = ERROR_INVALID_PARAMETER;
      wprintf(L"IPsecSaContextEnum0 didn't return any matching SAs\n");
      goto cleanup;
   }

   //-----------------------------------------
   // Go through the enumerated QM SAs and look for an exact match
   for(i=0; i<numEntriesReturned; i++)
   {
      if(IsQmSAExactMatch(
            qmSaList[i],
            matchingFwpFilter[0],
            remotePort
         ))
      {
         matchingQmSa = qmSaList[i];

         //-----------------------------------------
         // Get the IPsec MM SA using the MM SA ID
         result = IkeextSaGetById0(
                     fwpHandle,
                     matchingQmSa->outboundSa->saBundle.mmSaId,
                     &matchingMmSa
                     );
         if (result)
         {
            wprintf(L"IkeextSaGetById0 returned error %ld\n", result);
            goto cleanup;
         }
         
         //-----------------------------------------
         // Print the SAs
         wprintf(L"---------------------\n");
         PrintQmSa(matchingQmSa);
         PrintMmSa(matchingMmSa);

         //-----------------------------------------
         // Free the MM SA pointer
         FwpmFreeMemory0(&((void*)matchingMmSa));
      }
   }
   if(!matchingQmSa)
   {
      result = ERROR_INVALID_PARAMETER;
      wprintf(L"Didn't find any matching SAs\n");
      goto cleanup;
   }

cleanup:
   if(matchingFwpFilter)
   {
      FwpmFreeMemory0(&((void*)matchingFwpFilter));
   }
   if(qmSaList)
   {
      FwpmFreeMemory0(&((void*)qmSaList));
   }
   if(filterEnumHandle)
   {
      FwpmFilterDestroyEnumHandle0(fwpHandle, filterEnumHandle);
   }
   if(saEnumHandle)
   {
      IPsecSaContextDestroyEnumHandle0(fwpHandle, saEnumHandle);
   }
   if(fwpHandle)
   {
      FwpmEngineClose0(fwpHandle);
   }
   return result;
}

DWORD
MatchIPsecSAsForConnectedSocket(
   IN SOCKET sock
)
/*++

Routine Description:

   This routine finds the IPsec SAs (MM & QM) that match a given connected TCP
   socket (i-e after Connect() or Accept() have taken place)

Arguments:

   sock - The connected socket

Return Value:

   Windows error code indicating the status of the operation, or NO_ERROR if 
   the operation succeeded.

--*/
{
   DWORD result = 0;
   int sockErr = 0;
   SOCKADDR_STORAGE localAddress = {0};
   int localAddrLen = sizeof(SOCKADDR_STORAGE);
   SOCKADDR_STORAGE peerAddress = {0};
   int peerAddrLen = sizeof(SOCKADDR_STORAGE);

   //-----------------------------------------
   // Get local address bound to the socket   
   sockErr = getsockname(
               sock,
               (struct sockaddr*)&localAddress,
               &localAddrLen
               );
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"getsockname returned error %ld\n", result);
      goto cleanup;
   }
   if(!((localAddress.ss_family == AF_INET) ||
        (localAddress.ss_family == AF_INET6)))
   {
      result = ERROR_NOT_SUPPORTED;
      wprintf(L"Received incorrect address family %d\n", localAddress.ss_family);
      goto cleanup;
   }

   //-----------------------------------------
   // Get peer address bound to the socket   
   sockErr = getpeername(
               sock,
               (struct sockaddr*)&peerAddress,
               &peerAddrLen
               );
   if (sockErr == SOCKET_ERROR)
   {
      result = WSAGetLastError();
      wprintf(L"getpeername returned error %ld\n", result);
      goto cleanup;
   }

   //-----------------------------------------
   // Call the routine to do the match 
   result = MatchIPsecSAsForConnection(&localAddress, &peerAddress);
   if (result)
   {
      wprintf(L"MatchIPsecSAsForConnection returned error %ld\n", result);
      goto cleanup;
   }

cleanup:
   return result;
}

