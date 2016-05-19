/*++

Copyright (c) 1999 - 2000 Microsoft Corporation

Module Name:

    ipsamplerm.h

Abstract:

    The file contains type definitions and declarations for SAMPLE, the
    sample ip protocol, used by the IP Router Manager.

--*/

#ifndef _IPSAMPLERM_H_
#define _IPSAMPLERM_H_


// useful for variable sized structs
#undef  ANY_SIZE
#define ANY_SIZE 0



//----------------------------------------------------------------------------
// CONSTANT AND MACRO DECLARATIONS
//----------------------------------------------------------------------------

#define PROTO_IP_SAMPLE 200
#define MS_IP_SAMPLE    \
PROTOCOL_ID(PROTO_TYPE_UCAST, PROTO_VENDOR_MS0, PROTO_IP_SAMPLE)

#define SAMPLE_PROTOCOL_MULTICAST_GROUP ((DWORD)0x640000E0) // 224.0.0.100
    
    
//----------------------------------------------------------------------------
// constants identifying IPSAMPLE's MIB tables
//----------------------------------------------------------------------------

#define IPSAMPLE_GLOBAL_STATS_ID        0
#define IPSAMPLE_GLOBAL_CONFIG_ID       1
#define IPSAMPLE_IF_STATS_ID            2
#define IPSAMPLE_IF_CONFIG_ID           3
#define IPSAMPLE_IF_BINDING_ID          4


//----------------------------------------------------------------------------
// constants used for the field IPSAMPLE_GLOBAL_CONFIG::dwLoggingLevel
//----------------------------------------------------------------------------

#define IPSAMPLE_LOGGING_NONE           0
#define IPSAMPLE_LOGGING_ERROR          1
#define IPSAMPLE_LOGGING_WARN           2
#define IPSAMPLE_LOGGING_INFO           3



//----------------------------------------------------------------------------
// constants used for the field IPSAMPLE_IF_CONFIG::ulMetric
//----------------------------------------------------------------------------

#define IPSAMPLE_METRIC_INFINITE        16

    
    
//----------------------------------------------------------------------------
// constants used to construct the field IPSAMPLE_IF_BINDING::dwState
//----------------------------------------------------------------------------
    
#define IPSAMPLE_STATE_ACTIVE           0x00000001
#define IPSAMPLE_STATE_BOUND            0x00000002
    


//----------------------------------------------------------------------------
// STRUCTURE DEFINITIONS
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_GLOBAL_STATS
//
// This MIB entry stores global statistics for IPSAMPLE;
// There is only one instance, so this entry has no index.
//
// This structure is read-only.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_GLOBAL_STATS
{
    ULONG       ulNumInterfaces; // # interfaces added
} IPSAMPLE_GLOBAL_STATS, *PIPSAMPLE_GLOBAL_STATS;



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_GLOBAL_CONFIG
//
// This MIB entry stores global configuration for IPSAMPLE
// There is only one instance, so this entry has no index.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_GLOBAL_CONFIG
{
    DWORD       dwLoggingLevel;
} IPSAMPLE_GLOBAL_CONFIG, *PIPSAMPLE_GLOBAL_CONFIG;



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_IF_STATS
//
// This MIB entry stores per-interface statistics for IPSAMPLE.
//
// This structure is read-only.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_IF_STATS
{
    ULONG       ulNumPackets;   // # packets sent out this interface
} IPSAMPLE_IF_STATS, *PIPSAMPLE_IF_STATS;



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_IF_CONFIG
//
// This MIB entry describes per-interface configuration.
// All IP address fields must be in network order.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_IF_CONFIG
{
    ULONG       ulMetric;
} IPSAMPLE_IF_CONFIG, *PIPSAMPLE_IF_CONFIG;



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_IF_BINDING
//
// This MIB entry contains the table of IP addresses to which each
// interface is bound.  All IP addresses are in network order.
//
// THIS STRUCTURE IS VARIABLE LENGTH:
//
//  The base structure contains of the field ulCount, which gives the
//  number of IP addresses to which the indexed interface is bound.  The
//  IP addresses themselves follow the base structure, and are given as
//  IPSAMPLE_IP_ADDRESS structures.
//
// This MIB entry is read-only.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_IF_BINDING
{
    DWORD       dwState;
    ULONG       ulCount;
} IPSAMPLE_IF_BINDING, *PIPSAMPLE_IF_BINDING;




//----------------------------------------------------------------------------
// struct:      IPSAMPLE_IP_ADDRESS
//
// This structure is used for storing interface bindings.  A series of
// structures of this type follows the IPSAMPLE_IF_BINDING structure
// (described above).
//
// Both fields are IP address fields in network-order.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_IP_ADDRESS
{
    DWORD       dwAddress;
    DWORD       dwMask;
} IPSAMPLE_IP_ADDRESS, *PIPSAMPLE_IP_ADDRESS;



//----------------------------------------------------------------------------
// macros for manipulating the variable-length IPSAMPLE_IF_BINDING structure
//
// IPSAMPLE_IF_BINDING_SIZE computes the size of a binding structure.
//
// IPSAMPLE_IF_ADDRESS_TABLE computes the starting address in a binding
//      struct of the series of IPSAMPLE_IP_ADDRESS structures which are
//      the bindings for the interface in question.
//
// e.g.
//      PIPSAMPLE_IF_BINDING piibSource, piibDest;
//
//      piibDest = malloc(IPSAMPLE_IF_BINDING_SIZE(piibSource));
//      memcpy(piibDest, piibSource, IPSAMPLE_IF_BINDING_SIZE(piibSource));
//
// e.g.
//      ULONG                   i;
//      PIPSAMPLE_IF_BINDING    piib;
//      PIPSAMPLE_IP_ADDRESS    *piia;
//
//      piia = IPSAMPLE_IF_ADDRESS_TABLE(piib);
//      for (i = 0; i < piib->ulCount; i++)
//      {
//          printf("%s-", inet_ntoa(*(struct in_addr *)&piia->dwAddress));
//          printf("%s\n", inet_ntoa(*(struct in_addr *)&piia->dwMask));
//      }
//----------------------------------------------------------------------------

#define IPSAMPLE_IF_BINDING_SIZE(bind)                          \
    (sizeof(IPSAMPLE_IF_BINDING) +                              \
     (bind)->ulCount * sizeof(IPSAMPLE_IP_ADDRESS))

#define IPSAMPLE_IF_ADDRESS_TABLE(bind)                         \
    ((PIPSAMPLE_IP_ADDRESS)((bind) + 1))

        

//----------------------------------------------------------------------------
// The following structures are used to query the MIB and get back a
// response.  The TypeID field is one of the IDs #defined above.  The
// IfIndex is used to reference an interface table entry .  For sample
// it corresponds to an ip address, and hence is a single DWORD for now.
// In general the index could be a variable size array of DWORDs.
//----------------------------------------------------------------------------

        

//----------------------------------------------------------------------------
// struct:      IPSAMPLE_MIB_SET_INPUT_DATA
//
// This is passed as input data for MibSet.
// Note that only global config and interface config can be set.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_MIB_SET_INPUT_DATA
{
    DWORD       IMSID_TypeID;
    DWORD       IMSID_IfIndex;
    DWORD       IMSID_BufferSize;
    BYTE        IMSID_Buffer[ANY_SIZE];
} IPSAMPLE_MIB_SET_INPUT_DATA, *PIPSAMPLE_MIB_SET_INPUT_DATA;



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_MIB_GET_INPUT_DATA
//
// This is passed as input data for MibGet, MibGetFirst, MibGetNext.  All
// tables are readable.  All IP addresses must be in network order.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_MIB_GET_INPUT_DATA
{
    DWORD   IMGID_TypeID;
    DWORD   IMGID_IfIndex;
} IPSAMPLE_MIB_GET_INPUT_DATA, *PIPSAMPLE_MIB_GET_INPUT_DATA;



//----------------------------------------------------------------------------
// struct:      IPSAMPLE_MIB_GET_OUTPUT_DATA
//
// This is written into the output data by MibGet, MibGetFirst, MibGetNext.
//----------------------------------------------------------------------------

typedef struct _IPSAMPLE_MIB_GET_OUTPUT_DATA
{
    DWORD   IMGOD_TypeID;
    DWORD   IMGOD_IfIndex;

    BYTE    IMGOD_Buffer[ANY_SIZE];
} IPSAMPLE_MIB_GET_OUTPUT_DATA, *PIPSAMPLE_MIB_GET_OUTPUT_DATA;

#endif // _IPSAMPLERM_H_
