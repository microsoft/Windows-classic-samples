/********************************************************************++
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
PARTICULAR PURPOSE.

Copyright (c) 1985-2005 Microsoft Corporation. All Rights Reserved.

Abstract:
    This H file includes sample code for ...

Feedback:
    If you have any questions or feedback, please contact us using
    any of the mechanisms below:

    Email: peerfb@microsoft.com
    Newsgroup: Microsoft.public.win32.programmer.networks
    Website: http://www.microsoft.com/p2p

--********************************************************************/

// Registration info constants
// ---------------------------
#define MAX_PAYLOAD_LENGTH         256  // Max length of registration's payload
#define MAX_COMMENT_LENGTH         40   // "           "     " comment string
#define MAX_CLOUD_NAME             256  // Max length of a cloud name

// Peer name constants
// -------------------
#define MAX_CLASSIFIER_LENGTH       150  // Max length of a classifier
#define MAX_AUTHORITY_LENGTH        40  // Max length of the authority

// NOTE: A peer name is of the form [authority].[classifier]
#define MAX_PEERNAME_LENGTH         (MAX_AUTHORITY_LENGTH + 1 + MAX_CLASSIFIER_LENGTH)

// Other global constants
// ----------------------
#define MAX_ADDR_LENGTH          256 // Max length of a string representation
                                     // of an address
#define MAX_ENDPOINTS_TO_RESOLVE 5   // Default number of endpoints to return
                                     // with each peer name resolve
#define NUM_USER_DEFINED_ADDRESSES  1  // Number of user defined addresses
                                        // allowed per registration

//Function Prototypes
//-------------------

// Menu Commands - the signature on these functions must not be changed, since
// pointers these functions are being used in the Menu routines of this sample
HRESULT RegisterPeerNameCommand();
HRESULT EnumCloudsCommand();
HRESULT ResolvePeerNameCommand();
HRESULT ShowDNSEncodedNameCommand();
HRESULT UnregisterPeerNameCommand();
HRESULT UpdateRegistrationCommand();

// Helper Functions
HRESULT SyncPeerNameResolve(PCWSTR pwzPeerName, PCWSTR pwzCloudName);
HRESULT AsyncPeerNameResolve(PCWSTR pwzPeerName, PCWSTR pwzCloudName);
HRESULT DisplayPNRPEndpoint (__in PPEER_PNRP_ENDPOINT_INFO pEndpoint);
HRESULT GetCloudName (BOOL fAllowAll, __in ULONG cchCloudName, __out_ecount(cchCloudName) PWSTR pwzCloudName);
HRESULT GetAddress(BOOL fAllowDefault, __out ULONG* pcAddresses, __out_ecount(pcAddresses) SOCKADDR ***ppRegAddrOut);
HRESULT GetComment (__in ULONG cchComment, __out_ecount(cchComment) PWSTR pwzComment);
HRESULT GetPayload (__in ULONG cbPayloadDataSize,
                    __out_bcount(cbPayloadDataSize) PBYTE pbPayloadData);
HRESULT GetIdentity(__in ULONG cchIdentity, __out_ecount(cchIdentity) PWSTR pwzIdentity);
BOOL VerifyPayloadData(__in PPEER_DATA pData);
ULONG PayloadSize(__in PWSTR pPayloadData);

void PrintMenu();
int __cdecl main(int argc, __in_ecount(argc) char *argv[]);

// Utility Macros
#define celems(a)   (sizeof(a) / sizeof(a[0]))
